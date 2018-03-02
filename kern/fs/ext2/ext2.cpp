/*
ext2.cpp

Copyright (c) 19 Yann BOUCHER (yann)

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

#include "ext2.hpp"

#include "utils/memutils.hpp"
#include "time/time.hpp"
#include "utils/bitops.hpp"
#include "utils/vecutils.hpp"
#include "utils/mathutils.hpp"

Ext2FS::Ext2FS(Disk &disk) : FSImpl<Ext2FS>(disk)
{
    m_superblock = *read_superblock(disk);

    // these are the only supported features
    m_superblock.optional_features = (int)ext2::OptFeatureFlags::InodeExtendedAttributes | (int)ext2::OptFeatureFlags::InodeResize;

    disk.enable_caching(false);

    check_superblock_backups();

    m_superblock.last_mount_time = Time::epoch();
    ++m_superblock.mounts_since_fsck;
    m_superblock.fs_state = (int)ext2::FSState::HasErrors;

    update_superblock();
}

bool Ext2FS::accept(const Disk &disk)
{
    auto superblock = read_superblock(disk);
    if (!superblock) return false;

    return check_superblock(*superblock);
}

std::shared_ptr<vfs::node> Ext2FS::root() const
{
    return std::make_shared<ext2_node>(*(Ext2FS*)this, nullptr, "", 2);
}

std::optional<const ext2::Superblock> Ext2FS::read_superblock(const Disk &disk)
{
    if (disk.disk_size() < 1024*2)
    {
        return {};
    }

    auto data = disk.read(1024, 1024);
    return *reinterpret_cast<const ext2::Superblock*>(data.data());
}

bool Ext2FS::check_superblock(const ext2::Superblock &superblock)
{
    if (superblock.ext2_signature != ext2::signature)
    {
        return false;
    }

    log(Notice, "Ext2Fs revision %d.%d\n", superblock.version_major, superblock.version_minor);

    if (superblock.mounts_since_fsck + 1 >= superblock.mounts_before_fsck) // not updated yet, add one
    {
        warn("Mounts : fsck should be run ! (mounted %d/%d times)\n", superblock.mounts_since_fsck+1, superblock.mounts_before_fsck);
    }
    if (superblock.fs_state == (int)ext2::FSState::HasErrors)
    {
        warn("Filesystem is not clean, fsck should be run !\n");
    }

    if (superblock.forced_fsck_interval && Time::epoch() - superblock.last_fsck_time >= superblock.forced_fsck_interval)
    {
        warn("Time : fsck should be run ! (last check time : %s, should have run check on %s)\n", Time::to_string(Time::to_local_time(Time::from_unix(superblock.last_fsck_time))),
             Time::to_string(Time::to_local_time(Time::from_unix(superblock.last_fsck_time + superblock.forced_fsck_interval))));
    }

    if (superblock.version_major >= 1)
    {
        if (superblock.required_features & (int)ext2::RequiredFeatureFlags::Compression ||
                superblock.required_features & (int)ext2::RequiredFeatureFlags::FSNeedsJournalReplay ||
                superblock.required_features & (int)ext2::RequiredFeatureFlags::FSUsesJournal)
        {
            warn("Filesystem requires unsupported features : 0x%x\n", superblock.required_features);
            return false;
        }

        if (superblock.ro_required_features & (int)ext2::ReadOnlyFeatureFlags::BinaryTreeDirContents)
        {
            warn("Filesystem requires unsupported features : 0x%x (ro)\n", superblock.ro_required_features);
            return false;
        }
    }

    log(Notice, "Ext2FS optional features : 0x%x\n", superblock.optional_features);

    return true;
}

bool Ext2FS::check_superblock_backups() const
{
    const size_t last_group = m_superblock.block_count / m_superblock.blocks_in_block_group;

    bool okay = true;

    for (size_t i { 1 }; i <= last_group; ++i)
    {
        bool check = false;
        if (m_superblock.version_major==0) check = true; // revision 0 puts a backup at every group
        if (m_superblock.version_major>=1)
        {
            if (i == 1) check = true;
            if (i % 3 == 0 || i % 5 == 0 || i % 7 == 0) check = true;
        }

        if (check)
        {
            auto data = read_block(i * m_superblock.blocks_in_block_group + 1);

            log(Debug, "block : %d, offset 0x%x (%d)\n", i * m_superblock.blocks_in_block_group + 1,
                (i * m_superblock.blocks_in_block_group + 1) * block_size(),
                (i * m_superblock.blocks_in_block_group + 1) * block_size());

            if (((ext2::Superblock*)data.data())->ext2_signature != ext2::signature)
            {
                warn("Superblock backup at group %d has an invalid signature\n", i);
                okay = false;
            }
        }
    }

    return okay;
}

size_t Ext2FS::block_group_table_block() const
{
    return m_superblock.block_size == 0 ? 2 : 1;
}

const ext2::BlockGroupDescriptor Ext2FS::get_block_group(size_t inode) const
{
    const size_t group_desc_per_block = block_size()/sizeof(ext2::BlockGroupDescriptor);

    const size_t block_group_idx = (inode - 1) / m_superblock.inodes_in_block_group;
    const size_t block_to_read = block_group_table_block() + (block_group_idx / group_desc_per_block);

    auto data = read_block(block_to_read);

    return ((ext2::BlockGroupDescriptor*)data.data())[block_group_idx % group_desc_per_block];
}

MemBuffer Ext2FS::read_block(size_t number) const
{
    assert(number < m_superblock.block_count);
    auto vec = m_disk.read(number * block_size(), block_size());

    return vec;
}

void Ext2FS::write_block(size_t number, gsl::span<const uint8_t> data)
{
    assert(number < m_superblock.block_count);

    m_disk.write(number * block_size(), data);
}

const ext2::Inode Ext2FS::read_inode(size_t inode) const
{
    if (!check_inode_presence(inode))
    {
        error("Inode " + std::to_string(inode) + " is marked as free\n");
    }

    auto block_group = get_block_group(inode);

    size_t index = (inode - 1) % m_superblock.inodes_in_block_group;
    size_t block_idx = (index * inode_size()) / block_size();
    size_t offset = index % (block_size() / inode_size());

    auto block = read_block(block_group.inode_table + block_idx);

    return ((ext2::Inode*)block.data())[offset];
}

std::vector<const ext2::DirectoryEntry> Ext2FS::read_directory_entries(size_t inode) const
{
    std::vector<const ext2::DirectoryEntry> vec;

    auto inode_struct = read_inode(inode);

    size_t blocks = inode_struct.blocks_512 / (block_size()/512) + (inode_struct.blocks_512%(block_size()/512)?1:0);

#if 1
    auto entries = read_directory(read_data(inode_struct, 0, blocks));
    ((Ext2FS*)this)->write_directory_entries(inode, entries);
#endif
    return read_directory(read_data(inode_struct, 0, blocks));
}

bool Ext2FS::check_inode_presence(size_t inode) const
{
    auto block_group = get_block_group(inode);

    size_t index = (inode - 1) % m_superblock.inodes_in_block_group;
    size_t block_idx = index / block_size();
    size_t offset = index % block_size();

    auto block = read_block(block_group.inode_bitmap + block_idx);

    return bit_check(block[offset / 8], offset % 8);
}

MemBuffer Ext2FS::read_data_block(const ext2::Inode &inode, size_t blk_id) const
{
    size_t entries_per_block = block_size()/sizeof(uint32_t);

    if (blk_id < 12)
    {
        return read_block(inode.block_ptr[blk_id]);
    }
    blk_id -= 12;

    if (blk_id < entries_per_block)
    {
        return read_indirected(inode.block_ptr[12], blk_id, 1);
    }
    blk_id -= entries_per_block;

    if (blk_id < entries_per_block*entries_per_block)
    {
        return read_indirected(inode.block_ptr[13], blk_id, 2);
    }
    blk_id -= entries_per_block*entries_per_block;

    return read_indirected(inode.block_ptr[14], blk_id, 3);
}

MemBuffer Ext2FS::read_indirected(size_t indirected_block, size_t blk_id, size_t depth) const
{
    MemBuffer data;
    size_t entries = ipow<size_t>(block_size()/sizeof(uint32_t), depth-1);
    auto vec = read_block(indirected_block);
    uint32_t* block = (uint32_t*)vec.data();

    if (depth <= 1) return read_block(block[blk_id]);
    else
    {
        size_t tgt_block_idx = blk_id / entries;
        size_t offset = blk_id % entries;

        return read_indirected(block[tgt_block_idx], offset, depth - 1);
    }
}

MemBuffer Ext2FS::read_data(const ext2::Inode &inode, size_t offset, size_t size) const
{
    size_t blocks = inode.blocks_512 / (block_size()/512) + (inode.blocks_512%(block_size()/512)?1:0);

    assert(offset + size <= blocks);

    MemBuffer data;
    data.reserve(blocks);

    for (size_t i { offset }; i < offset + size; ++i)
    {
        merge(data, read_data_block(inode, i));
    }

    return data;
}

std::vector<const ext2::DirectoryEntry> Ext2FS::read_directory(gsl::span<const uint8_t> data) const
{
    std::vector<const ext2::DirectoryEntry> entries;
    entries.reserve(255);
    const uint8_t* ptr = (const uint8_t*)data.data();

    while (ptr < data.data() + data.size())
    {
        if (((const ext2::DirectoryEntry*)ptr)->inode) entries.emplace_back(*(const ext2::DirectoryEntry*)ptr);

        if (((const ext2::DirectoryEntry*)ptr)->record_len == 0)
        {
            error("Invalid directory record length!\n");
            return entries;
        }

        ptr += ((const ext2::DirectoryEntry*)ptr)->record_len;
    }

    return entries;
}

uint16_t Ext2FS::inode_size() const
{
    return m_superblock.version_major>=1?m_superblock.inode_size:128;
}

uint32_t Ext2FS::block_size() const
{
    return 1024<<m_superblock.block_size;
}

void Ext2FS::error(const std::string &message) const
{
    if (m_superblock.error_handling == (int)ext2::ErrorHandling::Panic)
    {
        panic("%s", message.c_str());
    }
    else
    {
        warn("%s", message.c_str());
    }
}

MemBuffer ext2_node::read_impl(size_t offset, size_t size) const
{
    size_t block_off = offset/fs.block_size();
    size_t block_size = size/fs.block_size() + (size%fs.block_size()?1:0);

    size_t byte_off = offset % fs.block_size();

    auto data = fs.read_data(inode_struct, block_off, block_size);

    if (byte_off == 0) { data.resize(size); return data; }
    else { return MemBuffer(data.begin() + byte_off, data.begin() + offset + size); }
}

std::vector<std::shared_ptr<vfs::node>> ext2_node::readdir_impl()
{
    std::vector<std::shared_ptr<vfs::node>> vec;

    for (const auto& entry : fs.read_directory_entries(inode))
    {
        auto entry_name = std::string(entry.name, entry.name_len);
        if (entry_name != "." &&
            entry_name != "..")
        {
            vec.push_back(std::static_pointer_cast<vfs::node>(
                              std::make_shared<ext2_node>(fs, this, entry_name, entry.inode)));
        }

    }

    return vec;
}

vfs::node::Stat ext2_node::stat() const
{
    Stat stat;
    stat.perms = inode_struct.type & 0x0FFF;
    stat.access_time = inode_struct.access_time;
    stat.modification_time = inode_struct.modification_time;
    stat.creation_time = inode_struct.creation_time;
    stat.uid = inode_struct.uid;
    stat.gid = inode_struct.gid;
    stat.flags = inode_struct.flags;

    return stat;
}

std::string ext2_node::name() const
{
#if 0
    if (!parent() || !dynamic_cast<ext2_node*>(parent())) return vfs::node::name();

    for (const auto& entry : fs.read_directory_entries(((ext2_node*)parent())->inode))
    {
        if (entry.inode == inode)
        {
            return std::string(entry.name, entry.name_len);
        }
    }

    assert(false); // should'nt reach this point
    return "<no_parent>";
#else
    return filename;
#endif
}

ADD_FS(Ext2FS)