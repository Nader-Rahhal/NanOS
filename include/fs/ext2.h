#pragma once

#include "drivers/ata.h"
#include "util.h"
#include "panic.h"

#define SUPERBLOCK_BYTE_OFFSET 1024
#define BGD_TABLE_BYTE_OFFSET 2048
#define MAX_BLOCK_GROUPS 32

namespace fs {
    uint32_t bytes_to_sector(uint32_t byte_offset){
        return byte_offset / BYTES_PER_SECTOR;
    }

    uint32_t block_to_sector(uint32_t block_offset){
        return block_offset * 2;
    }
}


namespace fs::ext2 {

    enum class FS_STATE {
        CLEAN,
        CONTAINS_ERRORS
    };

    enum class ERROR_HANDLING_METHODS {
        IGNORE,
        REMOUNT_AS_READ_ONLY,
        KERNEL_PANIC
    };

    enum class OS_ID {
        LINUX,
        GNU_HURD,
        MASIX,
        FREE_BSD,
        OTHER
    };



    struct __attribute__((packed)) superblock {
        uint32_t total_inodes;
        uint32_t total_blocks;
        uint32_t reserved_superuser_blocks;
        uint32_t free_blocks;
        uint32_t free_inodes;
        uint32_t block_containing_superblock;
        uint32_t block_size;
        uint32_t fragment_size;
        uint32_t blocks_per_group;
        uint32_t fragments_per_group;
        uint32_t inodes_per_group;
        uint32_t last_mnt_time;
        uint32_t last_write_time;
        uint16_t mnts_since_last_cc;
        uint16_t max_mnt_count;
        uint16_t signature;
        uint16_t state;
        uint16_t error_response;
        uint16_t version_minor;
        uint32_t time_last_cc;
        uint32_t time_btwn_forced_ccs;
        uint32_t os_id;
        uint32_t version_major;
        uint16_t user_id_for_reserved_blocks;
        uint16_t group_id_for_reserved_blocks;

        // extended sb
        uint32_t first_non_reserved_inode;
        uint16_t inode_size;
        uint16_t block_group_containing_superblock;
        uint32_t optional_features;
        uint32_t required_features;
        uint32_t unsupported_features;
        uint8_t fs_id[16];
        uint8_t volume_name[16];
        uint8_t path_volume_last_mounted_to[64];
        uint32_t compression_algo_used;
        uint8_t num_blocks_preallocated_for_files;
        uint8_t num_blocks_preallocated_for_dirs;
        uint16_t reserved;
        uint8_t journal_id[16];
        uint32_t journal_inode;
        uint32_t journal_device;
        uint32_t head_orphan_inode_list;
    };

    struct __attribute__((packed)) bgd {
        uint32_t block_usage_bitmap;
        uint32_t inode_usage_bitmap;
        uint32_t first_block_inode_table;
        uint16_t free_blocks;
        uint16_t free_inodes;
        uint16_t dirs_count;
        uint8_t reserved[14];
    };

    struct __attribute__((packed)) inode {
        uint16_t type_permissions;
        uint16_t used_id;
        uint32_t size_low;
        uint32_t last_access_time;
        uint32_t creation_time;
        uint32_t last_mod_time;
        uint32_t deletion_time;
        uint16_t gid;
        uint16_t count_hard_links;
        uint32_t count_disk_sectors;
        uint32_t flags;
        uint32_t os_val;
        uint32_t direct_block_ptrs[12];
        uint32_t singly_indirect_block_ptr;
        uint32_t doubly_indirect_block_ptr;
        uint32_t triply_indirect_block_ptr;
        uint32_t generation_number;
        uint32_t ext_attributes;
        uint32_t size_upper;
        uint32_t block_addr_fragment;
        uint32_t os_val2[3];
    };

    struct __attribute__((packed)) directory_entry {
        uint32_t inode;
        uint16_t size;
        uint16_t name_length;
        char name[8];
    };

    uint32_t get_block_group(uint32_t inode_number, uint32_t inodes_per_group){
        return (inode_number - 1) / inodes_per_group;
    }

    uint32_t inode_index_within_bg(uint32_t inode_number, uint32_t inodes_per_group){
        return (inode_number - 1) % inodes_per_group;
    }


    struct superblock sb;
    struct bgd bgd_table[MAX_BLOCK_GROUPS];

    struct inode get_inode(uint32_t inode_number){
        uint32_t block_group = get_block_group(inode_number, sb.inodes_per_group);
        struct bgd* block_group_desc = &bgd_table[block_group];

        uint32_t inode_table_block = block_group_desc->first_block_inode_table;
        uint32_t index_into_inode_table = inode_index_within_bg(inode_number, sb.inodes_per_group);

        uint32_t block_size = 1024u << sb.block_size;
        uint32_t byte_offset = (inode_table_block * block_size) + (index_into_inode_table * sb.inode_size);

        uint8_t sector_buffer[BYTES_PER_SECTOR];

        uint32_t inode_sector = bytes_to_sector(byte_offset);
        uint32_t offset_in_sector = byte_offset % BYTES_PER_SECTOR;

        ata::read_sector(inode_sector, sector_buffer);

        struct inode inode_info;

        util::memcpy(&inode_info, sector_buffer + offset_in_sector, sizeof(inode));

        util::serial_puts("\r\nRead inode ");
        util::serial_putdec(inode_number);
        util::serial_puts(":\r\n  Block group: ");
        util::serial_putdec(block_group);
        util::serial_puts("\r\n  Index within group: ");
        util::serial_putdec(index_into_inode_table);
        util::serial_puts("\r\n  Inode table block: ");
        util::serial_puthex(inode_table_block);
        util::serial_puts("\r\n  Byte offset: ");
        util::serial_puthex(byte_offset);
        util::serial_puts("\r\n  Sector: ");
        util::serial_puthex(inode_sector);
        util::serial_puts("\r\n  Type/permissions: ");
        util::serial_puthex(inode_info.type_permissions);
        util::serial_puts("\r\n  Size (low): ");
        util::serial_putdec(inode_info.size_low);
        util::serial_puts("\r\n  Hard links: ");
        util::serial_putdec(inode_info.count_hard_links);
        util::serial_puts("\r\n  First direct block ptr: ");
        util::serial_puthex(inode_info.direct_block_ptrs[0]);
        util::serial_puts("\r\n");

        return inode_info;
    }


    // returns a raw inode number to be used
    uint32_t find_free_inode(){
        uint32_t number_bgds = sb.total_blocks / sb.blocks_per_group;
        for (int i = 0; i < number_bgds; i++){
            struct bgd* block_group_desc = &bgd_table[i];

            uint32_t inode_usage_bitmap = block_group_desc->inode_usage_bitmap;

            uint8_t bitmap_buffer[1024];

            uint32_t block_size = 1024u << sb.block_size;
            uint32_t byte_offset = inode_usage_bitmap * block_size;

            uint32_t bitmap_sector = bytes_to_sector(byte_offset);

            ata::read_sector
        }
    }

    // functions we need
    
    // read file
        // we search directory for name match
        // when we find name match, we take inode number to find block group (call get_block_group)
        // use this to index into bgd_table
        // go to inode table in block group
        // read from whatever index inode we are
        // use inode struct to get file data

    // write file
        // we search directory for name match
        // when we find, name match we take inode number to find block group

    // create file
    // delete file


    void parse_superblock(){
        uint32_t sb_sector_offset = bytes_to_sector(SUPERBLOCK_BYTE_OFFSET);
        uint8_t buffer[1024];
        ata::read_sector(sb_sector_offset, buffer);

        util::memcpy(&sb, buffer, sizeof(struct superblock));

        util::serial_puts("Ext2 Superblock:\r\n");
        util::serial_puts("Total inodes: ");
        util::serial_putdec(sb.total_inodes);
        util::serial_puts("\r\nTotal blocks: ");
        util::serial_putdec(sb.total_blocks);
        util::serial_puts("\r\nFree blocks: ");
        util::serial_putdec(sb.free_blocks);
        util::serial_puts("\r\nFree inodes: ");
        util::serial_putdec(sb.free_inodes);
        util::serial_puts("\r\nBlock size: ");
        util::serial_putdec(1024u << sb.block_size);
        util::serial_puts("\r\nBlocks per group: ");
        util::serial_putdec(sb.blocks_per_group);
        util::serial_puts("\r\nInodes per group: ");
        util::serial_putdec(sb.inodes_per_group);
        util::serial_puts("\r\nSignature: ");
        util::serial_puthex(sb.signature);
        util::serial_puts("\r\nInode size: ");
        util::serial_putdec(sb.inode_size);
        util::serial_puts("\r\n");

        uint32_t number_bgds = sb.total_blocks / sb.blocks_per_group;
        if (number_bgds > MAX_BLOCK_GROUPS) {
            KPANIC("ext2: block group descriptor table exceeds MAX_BLOCK_GROUPS");
        }

        uint32_t bgdt_sector_offset = bytes_to_sector(BGD_TABLE_BYTE_OFFSET);
        uint32_t bgdt_bytes = number_bgds * sizeof(bgd);
        uint32_t bgdt_sectors = (bgdt_bytes + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR;

        uint8_t bgdt_buffer[bgdt_sectors * BYTES_PER_SECTOR];

        for (uint32_t i = 0; i < bgdt_sectors; i++) {
            ata::read_sector(bgdt_sector_offset + i, bgdt_buffer + i * BYTES_PER_SECTOR);
        }

        util::memcpy(bgd_table, bgdt_buffer, bgdt_bytes);

        util::serial_puts("\r\nBlock Group Descriptor Table:\r\n");
        for (uint32_t i = 0; i < number_bgds; i++) {
            struct bgd& group = bgd_table[i];
            util::serial_puts("Group ");
            util::serial_putdec(i);
            util::serial_puts(":\r\n  Block usage bitmap addr: ");
            util::serial_puthex(group.block_usage_bitmap_addr);
            util::serial_puts("\r\n  Inode usage bitmap addr: ");
            util::serial_puthex(group.inode_usage_bitmap_addr);
            util::serial_puts("\r\n  First block of inode table: ");
            util::serial_puthex(group.first_block_inode_table);
            util::serial_puts("\r\n  Free blocks: ");
            util::serial_putdec(group.free_blocks);
            util::serial_puts("\r\n  Free inodes: ");
            util::serial_putdec(group.free_inodes);
            util::serial_puts("\r\n  Directories count: ");
            util::serial_putdec(group.dirs_count);
            util::serial_puts("\r\n");
        }
    }
}