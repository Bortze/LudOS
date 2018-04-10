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

#include "mem/memmap.hpp"
#include "utils/messagebus.hpp"
#include "utils/stlutils.hpp"
#include "utils/memutils.hpp"

#include "mem/meminfo.hpp"

#ifdef LUDOS_HAS_SHM
#include "shared_memory.hpp"
#endif

#include "i686/tasking/process.hpp"

#include <sys/wait.h>

// Let the upper page free for argv/argc
constexpr uintptr_t argv_virt_page = KERNEL_VIRTUAL_BASE - (1*Memory::page_size());

Process::Process()
{
    data.pwd = std::make_shared<std::string>("/");
    init_default_fds();
}

void Process::reset(gsl::span<const uint8_t> code_to_copy, size_t allocated_size)
{
    if (arch_context)
    {
        cleanup(); // if arch_data == nullptr then the process isn't in a ready state yet, so don't clean it up
        assert(arch_context == nullptr);
    }
    arch_init(code_to_copy, allocated_size);
}

void Process::init_default_fds()
{
    assert(data.fd_table = std::make_shared<std::vector<tasking::FDInfo>>());

    assert(vfs::find("/dev/stdout"));
    assert(vfs::find("/dev/stdin"));
    assert(vfs::find("/dev/stderr"));

    assert(add_fd({vfs::find("/dev/stdin" ), .read = true,  .write = false}) == 0);
    assert(add_fd({vfs::find("/dev/stdout"), .read = false, .write = true }) == 1);
    assert(add_fd({vfs::find("/dev/stderr"), .read = false, .write = true }) == 2);
}

pid_t Process::find_free_pid()
{
    for (size_t i { 0 }; i < m_processes.size(); ++i)
    {
        if (!m_processes[i]) return i;
    }

    return m_processes.size();
}

size_t Process::add_fd(const tasking::FDInfo &info)
{
    // Search for an empty entry in the table
    for (size_t i { 0 }; i < data.fd_table->size(); ++i)
    {
        if ((*data.fd_table)[i].node == nullptr)
        {
            (*data.fd_table)[i] = info;
            return i;
        }
    }

    // If nothing is found, we append an entry to the table
    data.fd_table->emplace_back(info);
    return data.fd_table->size() - 1;
}

tasking::FDInfo *Process::get_fd(size_t fd)
{
    if (fd >= data.fd_table->size() || (*data.fd_table)[fd].node == nullptr)
    {
        return nullptr;
    }

    return &(*data.fd_table)[fd];
}

void Process::close_fd(size_t fd)
{
    assert(get_fd(fd));

    (*data.fd_table)[fd].node = nullptr;
}

bool Process::is_waiting() const
{
    return data.waiting_pid.has_value();
}

void Process::wait_for(pid_t pid, int *wstatus)
{
    data.waiting_pid = pid;
    data.wstatus = wstatus;
    data.waitstatus_phys = Memory::physical_address(wstatus);
    assert(data.wstatus);
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

    if (data.uid == Process::root_uid)
    {
        return true;
    }

    if ((tgt_uid == data.uid && perms & user_flag) ||
            (tgt_gid == data.gid && perms & group_flag)||
            perms & other_flag)
    {
        return true;
    }

    return false;
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
    assert(find(parent.data.children, pid));
    erase(parent.data.children, pid);

    if (parent.is_waiting() && (parent.data.waiting_pid.value() == -1 || parent.data.waiting_pid.value() == pid))
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

    static size_t last_size = 0;
    size_t curr = MemoryInfo::free();
    size_t diff = last_size - curr;

    //log_serial("PID destroyed : %d delta : %d (0x%x) total %d, allocated pages : %zd\n", pid, diff, diff, curr, Memory::allocated_physical_pages());

    last_size = curr;

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

    m_processes[free_idx]->tgid = free_idx;
    m_processes[free_idx]->pid  = free_idx;
    m_processes[free_idx]->data.args = args;

    ++m_process_count;

    MessageBus::send(ProcessCreatedEvent{free_idx});

    return m_processes[free_idx].get();
}

#ifdef LUDOS_HAS_SHM
void Process::map_shm()
{
    for (auto pair : data.shm_list)
    {
        if (pair.second.v_addr) pair.second.shm->map(pair.second.v_addr);
    }
}
#endif

void Process::map_code()
{
    size_t code_page_amnt = data.code->size() / Memory::page_size() +
            (data.code->size()%Memory::page_size()?1:0);

    for (size_t i { 0 }; i < code_page_amnt; ++i)
    {
        uintptr_t phys_addr = Memory::physical_address((uint8_t*)data.code->data() + i*Memory::page_size());
        uint8_t* virt_addr = (uint8_t*)(i * Memory::page_size());

        assert(phys_addr);
        data.mappings[(uintptr_t)virt_addr] = {phys_addr, Memory::Read|Memory::Write|Memory::User, false};
    }
}

void Process::map_stack()
{
    size_t code_page_amnt = data.stack.size() / Memory::page_size() +
            (data.stack.size()%Memory::page_size()?1:0);

    for (size_t i { 0 }; i < code_page_amnt; ++i)
    {
        uintptr_t phys_addr = Memory::physical_address((uint8_t*)data.stack.data() + i*Memory::page_size());
        uint8_t* virt_addr = (uint8_t*)(user_stack_top - (i * Memory::page_size()));

        assert(phys_addr);
        data.mappings[(uintptr_t)virt_addr] = {phys_addr, Memory::Read|Memory::Write|Memory::User, false};
    }
}

void Process::create_mappings()
{
    map_code();
    map_stack();
    if (!data.mappings.count(argv_virt_page))
    {
        data.mappings[argv_virt_page] = {Memory::allocate_physical_page(), Memory::Read|Memory::Write|Memory::User, true};
    }
}

void Process::release_mappings()
{
    for (const auto& pair : data.mappings)
    {
        if (pair.second.owned)
        {
            Memory::release_physical_page(pair.second.paddr);
        }
    }

    data.mappings.clear();
}

void Process::map_address_space()
{
    for (const auto& pair : data.mappings)
    {
        Memory::map_page(pair.second.paddr, (void*)pair.first, pair.second.flags);
    }
}

uintptr_t Process::allocate_pages(size_t pages)
{
    uint8_t* addr = reinterpret_cast<uint8_t*>(Memory::allocate_virtual_page(pages, true));
    for (size_t i { 0 }; i < pages; ++i)
    {
        void* virtual_page  = (uint8_t*)addr + i*Memory::page_size();
        uintptr_t physical_page = Memory::allocate_physical_page();
        Memory::map_page(physical_page, virtual_page, Memory::Read|Memory::Write|Memory::User);

        assert(!data.mappings.count((uintptr_t)virtual_page));
        data.mappings[(uintptr_t)virtual_page] = {(uintptr_t)physical_page, Memory::Read|Memory::Write|Memory::User, true};
    }

    return (uintptr_t)addr;
}

bool Process::release_pages(uintptr_t ptr, size_t pages)
{
    assert(ptr % Memory::page_size() == 0);

    for (size_t i { 0 }; i < pages; ++i)
    {
        void* virtual_page  = (uint8_t*)ptr + i*Memory::page_size();
        assert(data.mappings.count((uintptr_t)virtual_page));

        uintptr_t physical_page = data.mappings.at((uintptr_t)virtual_page).paddr;

        Memory::release_physical_page(physical_page);
        if (Memory::is_mapped(virtual_page)) Memory::unmap_page(virtual_page);

        assert(data.mappings.at((uintptr_t)virtual_page).owned);
        data.mappings.erase((uintptr_t)virtual_page);
    }

    return true;
}

void Process::copy_allocated_pages(Process &target)
{
    for (const auto& pair : data.mappings)
    {
        if (pair.second.owned)
        {
            target.data.mappings[pair.first] = pair.second;
            assert(target.data.mappings[pair.first].owned);
            target.data.mappings[pair.first].paddr = Memory::allocate_physical_page();

            auto src_ptr = Memory::mmap(pair.second.paddr, Memory::page_size());
            auto dest_ptr = Memory::mmap(target.data.mappings[pair.first].paddr, Memory::page_size());

            memcpy(dest_ptr, src_ptr, Memory::page_size());

            Memory::unmap(src_ptr, Memory::page_size());
            Memory::unmap(dest_ptr, Memory::page_size());
        }
    }
}

Process::~Process()
{
    unswitch();
    cleanup();
}
