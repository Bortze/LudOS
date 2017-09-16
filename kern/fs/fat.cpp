/*
fat.cpp

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

#include "fat.hpp"

#include "diskinterface.hpp"

#include "utils/logging.hpp"
#include "utils/vector.hpp"
#include "utils/string.hpp"

fat::FATInfo fat::read_fat_fs(size_t drive)
{
    vector<uint8_t> first_sector(512);

    DiskInterface::read(drive, 0, 1, first_sector.data());

    BS bootsector = *reinterpret_cast<BS*>(first_sector.data());

    FATInfo info;
    info.bootsector = bootsector;
    info.ext16 = *reinterpret_cast<extBS_16*>(bootsector.extended_section);
    info.ext32 = *reinterpret_cast<extBS_32*>(bootsector.extended_section);

    if (bootsector.bootjmp[0] != 0xEB && bootsector.bootjmp[0] != 0xE9)
    {
        warn("Drive %zd is not at FAT drive\n");
        info.valid = false;

        return info;
    }

    info.drive = drive;

    info.total_sectors = (bootsector.total_sectors_16 == 0) ? bootsector.total_sectors_32 : bootsector.total_sectors_16;
    info.fat_size = (bootsector.table_size_16 == 0)? info.ext32.table_size_32 : bootsector.table_size_16;
    info.root_dir_sectors = ((bootsector.root_entry_count * 32) + (bootsector.bytes_per_sector - 1)) / bootsector.bytes_per_sector;
    info.first_data_sector = bootsector.reserved_sector_count + (bootsector.table_count * info.fat_size) + info.root_dir_sectors;
    info.first_fat_sector = bootsector.reserved_sector_count;
    info.data_sectors = info.total_sectors - (bootsector.reserved_sector_count + (bootsector.table_count * info.fat_size) + info.root_dir_sectors);
    info.total_clusters = info.data_sectors / bootsector.sectors_per_cluster;

    info.valid = true;

    if (info.total_clusters < 4085)
    {
        info.type = FATType::FAT12;
    }
    else if (info.total_clusters < 65525)
    {
        info.type = FATType::FAT16;
    }
    else if (info.total_clusters < 268435445)
    {
        info.type = FATType::FAT32;
    }
    else
    {
        info.type = FATType::ExFAT;
        info.valid = false;
        err("ExFAT is not supported !\n");

        return info;
    }

    return info;
}

size_t fat::detail::first_sector_of_cluster(size_t cluster, const fat::FATInfo &info)
{
    return ((cluster - 2) * info.bootsector.sectors_per_cluster) + info.first_data_sector;
}

void fat::detail::read_FAT_sector(vector<uint8_t> &FAT, size_t sector, size_t drive)
{
    DiskInterface::read(drive, sector, 1, FAT.data());
}

uint32_t fat::detail::next_cluster(size_t cluster, const fat::FATInfo &info)
{
    uint32_t fat_offset;
    if (info.type == FATType::FAT12)
    {
        fat_offset = cluster + cluster / 2;
    }
    else if (info.type == FATType::FAT16)
    {
        fat_offset = cluster * 2;
    }
    else if (info.type == FATType::FAT32)
    {
        fat_offset = cluster * 4;
    }

    uint32_t fat_sector = info.first_fat_sector + (fat_offset / 512);
    uint32_t ent_offset = fat_offset % 512;

    vector<uint8_t> FAT(512);

    read_FAT_sector(FAT, fat_sector, info.drive);

    uint32_t table_value = *reinterpret_cast<uint32_t*>(&FAT[ent_offset]) & 0x0FFFFFFF;

    if (info.type == FATType::FAT12)
    {
        if(cluster & 0x0001)
            table_value = table_value >> 4;
        else
            table_value = table_value & 0x0FFF;
    }

    return table_value;
}

vector<fat::Entry> fat::root_entries(const fat::FATInfo &info)
{
    if (info.type == FATType::FAT32)
    {
        return detail::read_cluster_entries(detail::first_sector_of_cluster(info.ext32.root_cluster, info), info);
    }
    else
    {
        return detail::read_cluster_entries(info.first_data_sector - info.root_dir_sectors, info);
    }
}

vector<fat::Entry> fat::detail::read_cluster_entries(size_t first_sector, const fat::FATInfo &info)
{
    vector<uint8_t> data(512 * info.bootsector.sectors_per_cluster);
    DiskInterface::read(info.drive, first_sector, info.bootsector.sectors_per_cluster, data.data());

    vector<Entry> entries;

    Entry* entry = reinterpret_cast<Entry*>(data.data());
    Entry* entry_end = reinterpret_cast<Entry*>(data.end());

    string longname_buf;

    while (entry < entry_end && entry->filename[0] != '\0')
    {
        if (entry->filename[0] == 0xE5)
        {
            ++entry;
            continue;
        }

        if (entry->attributes == 0x0F)
        {
            string vec;
            // Don't forget we are dealing with UCS2 encoding (16-bit)
            for (size_t i { 0 }; i < sizeof(entry->name1)/2; ++i)
            {
                vec += entry->name1[i*2];
            }
            for (size_t i { 0 }; i < sizeof(entry->name2)/2; ++i)
            {
                vec += entry->name2[i*2];
            }
            for (size_t i { 0 }; i < sizeof(entry->name3)/2; ++i)
            {
                vec += entry->name3[i*2];
            }

            longname_buf = vec + longname_buf;

            ++entry;
            continue;
        }
        else
        {
            log("Long name : %s\n", longname_buf.data());

            entries.push_back(*entry);

            longname_buf.clear();
        }

        ++entry;
    }
    return entries;
}

vector<uint8_t> fat::detail::read_cluster(size_t first_sector, const fat::FATInfo &info)
{
    vector<uint8_t> data(512 * info.bootsector.sectors_per_cluster);
    DiskInterface::read(info.drive, first_sector, info.bootsector.sectors_per_cluster, data.data());

    return data;
}

vector<uint8_t> fat::detail::read_cluster_chain(size_t cluster, const fat::FATInfo& info)
{
    uint32_t table_entry = next_cluster(cluster, info);

    vector<uint8_t> next_entries;
    if (info.type == FATType::FAT12 && table_entry >= 0xFF7)
    {
        next_entries = {};
    }
    else if (info.type == FATType::FAT16 && table_entry >= 0xFFF7)
    {
        next_entries = {};
    }
    else if (info.type == FATType::FAT32 && table_entry >= 0x0FFFFFF7)
    {
        next_entries = {};
    }
    else
    {
        next_entries = read_cluster_chain(table_entry, info);
    }

    return read_cluster(first_sector_of_cluster(cluster, info), info) + next_entries;
}

vector<uint8_t> fat::read(const fat::Entry &entry, const FATInfo &info)
{
    return detail::read_cluster_chain(entry.low_cluster_bits | (entry.high_cluster_bits << 16), info);
}
