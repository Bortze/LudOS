/*
devfs.cpp

Copyright (c) 05 Yann BOUCHER (yann)

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

#include "devfs.hpp"

#include "fs/vfs.hpp"

#include "utils/messagebus.hpp"
#include "drivers/kbd/keyboard.hpp"
#include "utils/nop.hpp"
#include "drivers/storage/disk.hpp"

namespace devfs
{

struct stdout_file : public vfs::node
{
    using node::node;

protected:
    [[nodiscard]] virtual bool write_impl(size_t offset, gsl::span<const uint8_t> data) override
    {
        auto str = std::string((char*)data.data(), data.size());
        kprintf("%s", str.c_str());
        return true;
    }
};
struct stderr_file : public vfs::node
{
    using node::node;
protected:
    [[nodiscard]] virtual bool write_impl(size_t offset, gsl::span<const uint8_t> data) override
    {
        auto str = std::string((char*)data.data(), data.size());
        err("%s", str.c_str());
        return true;
    }
};

struct stdin_file : public vfs::node
{
    using node::node;
protected:
    [[nodiscard]] virtual MemBuffer read_impl(size_t offset, size_t size) const override
    {
        MemBuffer buf;

        auto handl = MessageBus::register_handler<kbd::TextEnteredEvent>([&buf](const kbd::TextEnteredEvent& e)
        {
            buf.push_back(e.c);
        });

        while (buf.size() < size) { wait_for_interrupts(); };

        MessageBus::remove_handler(handl);

        return buf;
    }
};

struct disk_file : public vfs::node
{
    disk_file(Disk& disk, const std::string& node_name)
        : m_disk(disk)
    {
        m_name = node_name;
    }

    virtual size_t size() const override { return m_disk.disk_size(); }
    virtual bool is_dir() const override { return false; }

protected:
    [[nodiscard]] virtual MemBuffer read_impl(size_t offset, size_t size) const override
    {
        return m_disk.read(offset, size);
    }
    [[nodiscard]] virtual bool write_impl(size_t offset, gsl::span<const uint8_t> data) override
    {
        m_disk.write(offset, data);
        return true;
    }

public:
    Disk& m_disk;
};

struct devfs_root : public vfs::node
{
    using node::node;
public:
    virtual bool is_dir() const override { return true; }

    std::vector<int> children;
};

void init()
{
    auto root = std::make_shared<devfs_root>(vfs::root.get());
    vfs::mount(root, "/dev");

    auto stdin_node = std::make_shared<stdin_file>(root.get());
    stdin_node->rename("stdin");

    root->children.emplace_back(6);

    vfs::new_descriptor(*stdin_node);
    if (!mount(stdin_node, "/dev"))
    {
        warn("Can't mount '/dev/stdin'\n");
    }

    auto stdout_node = std::make_shared<stdout_file>(root.get());
    stdout_node->rename("stdout");

    vfs::new_descriptor(*stdout_node);
    if (!mount(stdout_node, "/dev"))
    {
        warn("Can't mount '/dev/stdout'\n");
    }

    auto stderr_node = std::make_shared<stderr_file>(root.get());
    stderr_node->rename("stderr");

    vfs::new_descriptor(*stderr_node);
    if (!mount(stderr_node, "/dev"))
    {
        warn("Can't mount '/dev/stderr'\n");
    }

    // add existing drives
    for (Disk& disk : Disk::disks())
    {
        detail::add_drive(disk);
    }

    MessageBus::register_handler<DiskFoundEvent>([](const DiskFoundEvent& ev)
    {
        detail::add_drive(ev.disk);
    });
}

namespace detail
{

std::string drive_label(const Disk& disk)
{
    static std::map<std::string, std::string> label_mappings;
    static std::map<std::string, size_t> disk_partitions;
    static std::array<std::string, Disk::Unknown + 1> suffixes;
    if (suffixes[0].empty()) suffixes.fill("a");

    std::string prefix;
    std::string name;

    switch (disk.media_type())
    {
        case Disk::Floppy:
            prefix = "fd"; break;
        case Disk::OpticalDisk:
            prefix = "od"; break;
        case Disk::NetworkDrive:
            prefix = "nd"; break;
        case Disk::MemoryCard:
            prefix = "mmc"; break;
        case Disk::RamDrive:
            prefix = "md"; break;
        case Disk::HardDrive:
        default:
            prefix = "sd"; break;
    }

    if (disk.is_partition())
    {
        // TODO : GUID
        auto parent_name = static_cast<const DiskSlice*>(&disk)->parent().drive_name();

        name = label_mappings.at(parent_name) + std::to_string(++disk_partitions[parent_name]);
    }
    else
    {
        name = prefix + suffixes[disk.media_type()];

        label_mappings[disk.drive_name()] = name;
    }

    if (suffixes[disk.media_type()].back() != 'z')
    {
        ++suffixes[disk.media_type()].back();
    }
    else
    {
        suffixes[disk.media_type()] += "a";
    }

    return name;
}

void add_drive(Disk &disk)
{
    std::string name = drive_label(disk);

    auto node = std::make_shared<disk_file>(disk, name);
    vfs::mount(node, "/dev");
}
}

}
