/*
fat.hpp

Copyright (c) 16 Yann BOUCHER (yann)

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
#ifndef FAT_HPP
#define FAT_HPP

#include <stdint.h>

#include <vector.hpp>
#include "string.hpp"

#include "vfs.hpp"

#include "utils/stlutils.hpp"
#include "utils/logging.hpp"

#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LFN (ATTR_READ_ONLY|ATTR_HIDDEN|ATTR_SYSTEM|ATTR_VOLUME_ID)

namespace fat
{

union Entry
{
    struct [[gnu::packed]]
    {
        uint8_t filename[8];
        uint8_t extension[3];

        uint8_t attributes;
        uint8_t nt_reserved;

        uint8_t creation_time_ts;

        uint8_t hour : 5;
        uint8_t min : 6;
        uint8_t sec : 5;

        uint8_t year : 7;
        uint8_t month : 4;
        uint8_t day : 5;

        uint8_t last_access_year : 7;
        uint8_t last_access_month : 4;
        uint8_t last_access_day : 5;

        uint16_t high_cluster_bits;

        uint8_t last_modif_hour : 5;
        uint8_t last_modif_min : 6;
        uint8_t last_modif_sec : 5;

        uint8_t last_modif_year : 7;
        uint8_t last_modif_month : 4;
        uint8_t last_modif_day : 5;

        uint16_t low_cluster_bits;

        uint32_t size;
    };
    struct [[gnu::packed]]
    {
        uint8_t ord;
        uint8_t name1[10];
        uint8_t attr;
        uint8_t type;
        uint8_t chksum;
        uint8_t name2[12];
        uint16_t fstcluslo;
        uint8_t name3[4];
    };

};

static_assert(sizeof(Entry) == 32);


enum class FATType
{
    FAT12 = 12,
    FAT16 = 16,
    FAT32 = 32,
    ExFAT = 64
};

struct [[gnu::packed]] BS
{
    uint8_t 		bootjmp[3];
    uint8_t 		oem_name[8];
    uint16_t 	        bytes_per_sector;
    uint8_t		sectors_per_cluster;
    uint16_t		reserved_sector_count;
    uint8_t		table_count;
    uint16_t		root_entry_count;
    uint16_t		total_sectors_16;
    uint8_t		media_type;
    uint16_t		table_size_16;
    uint16_t		sectors_per_track;
    uint16_t		head_side_count;
    uint32_t 		hidden_sector_count;
    uint32_t 		total_sectors_32;

    //this will be cast to it's specific type once the driver actually knows what type of FAT this is.
    uint8_t		extended_section[54];

};

struct [[gnu::packed]] extBS_32
{
    //extended fat32 stuff
    uint32_t		table_size_32;
    uint16_t		extended_flags;
    uint16_t		fat_version;
    uint32_t		root_cluster;
    uint16_t		fat_info;
    uint16_t		backup_BS_sector;
    uint8_t 		reserved_0[12];
    uint8_t		drive_number;
    uint8_t 		reserved_1;
    uint8_t		boot_signature;
    uint32_t 		volume_id;
    uint8_t		volume_label[11];
    uint8_t		fat_type_label[8];

};

struct [[gnu::packed]] extBS_16
{
    //extended fat12 and fat16 stuff
    uint8_t		bios_drive_num;
    uint8_t		reserved1;
    uint8_t		boot_signature;
    uint32_t		volume_id;
    uint8_t		volume_label[11];
    uint8_t		fat_type_label[8];

};

struct FATInfo
{
    bool valid;
    size_t base_sector;
    BS bootsector;
    extBS_16 ext16;
    extBS_32 ext32;
    FATType type;
    size_t drive;
    size_t total_sectors;
    size_t fat_size;
    size_t root_dir_sectors;
    size_t first_data_sector;
    size_t first_fat_sector;
    size_t data_sectors;
    size_t total_clusters;
};

namespace detail
{

std::vector<uint8_t> read(const Entry& entry, const fat::FATInfo& info);
std::vector<uint8_t> read(const Entry& entry, const fat::FATInfo& info, size_t nbytes);

bool write(const fat::Entry &entry, const FATInfo &info, const std::vector<uint8_t>& data);

bool write_cluster(const FATInfo& info, size_t cluster, const std::vector<uint8_t>& data);

void read_FAT_sector(std::vector<uint8_t>& FAT, size_t sector, size_t drive);

size_t first_sector_of_cluster(size_t cluster, const FATInfo& info);

uint32_t next_cluster(size_t cluster, const FATInfo& info);

std::vector<vfs::node> read_cluster_entries(size_t first_sector, const FATInfo& info);

std::vector<uint8_t> read_cluster_chain(size_t cluster, const FATInfo& info);

std::vector<uint8_t> read_cluster(size_t first_sector, const fat::FATInfo &info);

std::vector<size_t> get_cluster_chain(size_t first_cluster, const fat::FATInfo& info);

vfs::node entry_to_vfs_node(const Entry& entry, const FATInfo &info, const std::string &long_name);

std::vector<uint8_t> get_FAT(const FATInfo& info);
uint32_t FAT_entry(const std::vector<uint8_t>& FAT, const FATInfo &info, size_t cluster);

std::vector<uint32_t> find_free_clusters(const FATInfo& info, size_t clusters);

void add_entry(const FATInfo& info, size_t cluster, Entry entry);

struct fat_file : public vfs::file
{
    virtual size_t read(void* buf, size_t bytes) const override
    {
        if (is_dir)
        {
            return 0;
        }

        auto data = fat::detail::read(entry, info, bytes);
        for (size_t i { 0 }; i < data.size(); ++i)
        {
            reinterpret_cast<uint8_t*>(buf)[i] = data[i];
        }

        update_entry();

        return data.size();
    }

    virtual size_t write(const void* buf, size_t n) override
    {
        if (is_dir)
        {
            return 0;
        }

        fat::detail::write(entry, info, std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(buf), reinterpret_cast<const uint8_t*>(buf) + n));

        update_entry();

        return n;
    }

    virtual std::vector<vfs::node> readdir_impl() const override
    {
        if (!is_dir)
        {
            return {};
        }

        update_entry();

        if (is_root)
        {
            return fat_children;
        }
        else
        {
            return merge(detail::read_cluster_entries(detail::first_sector_of_cluster(cluster, info), info), fat_children);
        }
    }

    virtual vfs::node* mkdir(const std::string& str) override
    {
        if (!is_dir)
        {
            return nullptr;
        }

        fat_children.emplace_back();
        auto& back = fat_children.back();
        back.get_file() = fat_file();
        // TODO : allocate
        back.name = str;
        back->is_dir = true;

        update_entry();

        return &back;
    }

    virtual vfs::node* touch(const std::string& str) override
    {
        if (!is_dir)
        {
            return nullptr;
        }

        fat_children.emplace_back();
        auto& back = fat_children.back();
        back.get_file() = fat_file();
        // TODO : allocate
        back.name = str;
        back->is_dir = false;

        update_entry();

        return &back;
    }

    size_t cluster;
    bool is_root : 1;
    std::vector<vfs::node> fat_children;
    fat::Entry entry;
    FATInfo info;

private:
    void update_entry() const;
};
}

FATInfo read_fat_fs(size_t drive, size_t base_sector);

vfs::node root_dir(const FATInfo& info);

}

#endif // FAT_HPP
