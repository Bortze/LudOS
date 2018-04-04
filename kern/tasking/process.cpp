/*
process.cpp

Copyright (c) 06 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "process.hpp"

#include "fs/vfs.hpp"

#include "syscalls/syscalls.hpp"

#include "utils/messagebus.hpp"

#include <sys/wait.h>

Process::Process()
{
    init_default_fds();
}

void Process::reset(gsl::span<const uint8_t> code_to_copy, size_t allocated_size)
{
    if (arch_data) cleanup(); // if arch_data == nullptr then the process isn't in a ready state yet, so don't clean it up
    arch_init(code_to_copy, allocated_size);
}

void Process::init_default_fds()
{
    assert(vfs::find("/dev/stdout"));
    assert(vfs::find("/dev/stdin"));
    assert(vfs::find("/dev/stderr"));

    assert(add_fd({vfs::find("/dev/stdin" ), .read = true,  .write = false}) == 0);
    assert(add_fd({vfs::find("/dev/stdout"), .read = false, .write = true }) == 1);
    assert(add_fd({vfs::find("/dev/stderr"), .read = false, .write = true }) == 2);
}

void Process::release_all_pages()
{
    std::vector<uintptr_t> addr_list;
    // First copy the address list because release_pages will remove elements while iterating
    for (const auto& pair : allocated_pages)
    {
        addr_list.emplace_back(pair.first);
    }
    for (auto addr : addr_list)
    {
        release_pages(addr, 1);
    }

    assert(allocated_pages.empty());
}

pid_t Process::find_free_pid()
{
    for (size_t i { 0 }; i < m_processes.size(); ++i)
    {
        if (!m_processes[i]) return i;
    }

    return m_processes.size();
}

size_t Process::add_fd(const FDInfo &info)
{
    // Search for an empty entry in the table
    for (size_t i { 0 }; i < fd_table.size(); ++i)
    {
        if (fd_table[i].node == nullptr)
        {
            fd_table[i] = info;
            return i;
        }
    }

    // If nothing is found, we append an entry to the table
    fd_table.emplace_back(info);
    return fd_table.size() - 1;
}

Process::FDInfo *Process::get_fd(size_t fd)
{
    if (fd >= fd_table.size() || fd_table[fd].node == nullptr)
    {
        return nullptr;
    }

    return &fd_table[fd];
}

void Process::close_fd(size_t fd)
{
    assert(get_fd(fd));

    fd_table[fd].node = nullptr;
}

bool Process::is_waiting() const
{
    return waiting_pid.has_value();
}

void Process::wait_for(pid_t pid, int *wstatus)
{
    waiting_pid = pid;
    this->wstatus = wstatus;
    assert(this->wstatus);
}

bool Process::check_perms(uint16_t perms, uint16_t tgt_uid, uint16_t tgt_gid, AccessRequestPerm type)
{
    uint16_t user_flag  = (type == AccessRequestPerm::Read ? vfs::UserRead :
                                                             type == AccessRequestPerm::Write? vfs::UserWrite:
                                                                                               vfs::UserExec);
    uint16_t group_flag = (type == AccessRequestPerm::Read ? vfs::GroupRead :
                                                             type == AccessRequestPerm::Write? vfs::GroupWrite:
                                                                                               vfs::GroupExec);
    uint16_t other_flag = (type == AccessRequestPerm::Read ? vfs::OtherRead :
                                                             type == AccessRequestPerm::Write? vfs::OtherWrite:
                                                                                               vfs::OtherExec);

    if (this->uid == Process::root_uid)
    {
        return true;
    }

    if ((tgt_uid == this->uid && perms & user_flag) ||
            (tgt_gid == this->gid && perms & group_flag)||
            perms & other_flag)
    {
        return true;
    }

    return false;
}

Process::~Process()
{
    unswitch();
    cleanup();
    log_serial("PID destroyed : %d\n", pid);
    kfree(arch_data);
}

bool Process::enabled()
{
    return m_current_process != nullptr;
}

Process &Process::current()
{
    assert(enabled());

    return *m_current_process;
}

size_t Process::count()
{
    return m_process_count;
}

void Process::kill(pid_t pid, int err_code)
{
    if (pid == 0)
    {
        panic("Tried to kill the master process !\n");
    }

    assert(by_pid(pid));
    assert(by_pid(by_pid(pid)->parent));
    auto& parent = *by_pid(by_pid(pid)->parent);
    // erase pid from parent children list

    assert(std::find(parent.children.begin(), parent.children.end(), pid) != parent.children.end());
    parent.children.erase(std::remove(parent.children.begin(), parent.children.end(), pid), parent.children.end());

    log_serial("Waiting : %d, pid: %d\n", *parent.waiting_pid, pid);

    if (parent.is_waiting() && (parent.waiting_pid.value() == -1 || parent.waiting_pid.value() == pid))
    {
        log_serial("Waking up PID %d with PID %d\n", parent.pid, pid);
        parent.wake_up(pid, err_code);
    }

    if (Process::current().pid == pid)
    {
        m_current_process = nullptr;
    }

    m_processes[pid].reset();
    --m_process_count;
    assert(!by_pid(pid));

    MessageBus::send(ProcessDestroyedEvent{pid, err_code});
}

Process *Process::by_pid(pid_t pid)
{
    if (pid >= (int)m_processes.size())
    {
        return nullptr;
    }

    return m_processes[pid].get();
}

Process *Process::create(const std::vector<std::string>& args)
{
    pid_t free_idx = find_free_pid();
    if (free_idx == (int)m_processes.size())
    { // expand the process list
        m_processes.emplace_back(new Process);
    }
    else
    { // reuse a hole in the list
        m_processes[free_idx].reset(new Process);
    }

    if (m_processes[free_idx] == nullptr) return nullptr;

    m_processes[free_idx]->pid = free_idx;
    m_processes[free_idx]->waiting_pid.reset();
    m_processes[free_idx]->args = args;

    ++m_process_count;

    MessageBus::send(ProcessCreatedEvent{free_idx});

    return m_processes[free_idx].get();
}
