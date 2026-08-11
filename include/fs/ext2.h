#pragma once

#include "drivers/ata.h"
#include "util.h"
#include "panic.h"

#define SUPERBLOCK_BYTE_OFFSET 1024
#define BGD_TABLE_BYTE_OFFSET 2048
#define MAX_BLOCK_GROUPS 32
#define ROOT_INODE 2

namespace fs::ext2 {
namespace detail {

    uint32_t bytes_to_sector(uint32_t byte_offset){
        return byte_offset / BYTES_PER_SECTOR;
    }

    uint32_t block_to_sector(uint32_t block_offset){
        return block_offset * 2;
    }

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
        uint16_t rec_len;
        uint8_t  name_len;
        uint8_t  file_type;
        char     name[255];   // max possible; on-disk size is name_len, 4-byte aligned
    };

    enum class Filetype : uint8_t {
        UNKNOWN  = 0,
        REG_FILE = 1,   // regular file
        DIR      = 2,   // directory
        CHR_DEV  = 3,   // character device
        BLK_DEV  = 4,   // block device
        FIFO     = 5,   // named pipe
        SOCKET   = 6,   // unix socket
        SYMLINK  = 7
    };

    uint32_t get_block_group(uint32_t inode_number, uint32_t inodes_per_group){
        return (inode_number - 1) / inodes_per_group;
    }

    uint32_t inode_index_within_bg(uint32_t inode_number, uint32_t inodes_per_group){
        return (inode_number - 1) % inodes_per_group;
    }

    // on-disk footprint of a directory entry: 8-byte header + name, 4-byte aligned
    uint32_t dirent_actual_size(uint8_t name_len){
        return (8u + name_len + 3u) & ~3u;
    }

    // dirent names have no NUL terminator, so compare length + bytes
    bool dirent_name_matches(const directory_entry* e, const char* name, uint8_t name_len){
        if (e->name_len != name_len) return false;
        for (uint8_t k = 0; k < name_len; k++){
            if (e->name[k] != name[k]) return false;
        }
        return true;
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

    // write an inode struct back into the inode table (read-modify-write the
    // sector containing it, since inodes don't align to sector boundaries).
    // assumes an inode never straddles a sector boundary, which holds because
    // inode_size (128 or 256) divides BYTES_PER_SECTOR (512).
    void write_inode(uint32_t inode_number, const struct inode* in){
        uint32_t block_group = get_block_group(inode_number, sb.inodes_per_group);
        struct bgd* block_group_desc = &bgd_table[block_group];

        uint32_t block_size = 1024u << sb.block_size;
        uint32_t byte_offset = (block_group_desc->first_block_inode_table * block_size)
                             + (inode_index_within_bg(inode_number, sb.inodes_per_group) * sb.inode_size);

        uint32_t inode_sector = bytes_to_sector(byte_offset);
        uint32_t offset_in_sector = byte_offset % BYTES_PER_SECTOR;

        uint8_t sector_buffer[BYTES_PER_SECTOR];
        ata::read_sector(inode_sector, sector_buffer);
        util::memcpy(sector_buffer + offset_in_sector, in, sizeof(struct inode));
        ata::write_sector(inode_sector, sector_buffer);
    }

    void read_block(uint32_t lba, uint8_t* buffer){
        ata::read_sector(lba, buffer);
        ata::read_sector(lba + 1, buffer + 512);
    }

    void write_block(uint32_t sector, uint8_t* buffer){
        ata::write_sector(sector, buffer);
        ata::write_sector(sector + 1, buffer + 512);
    }

    // flush the in-memory superblock back to disk.
    // read-modify-write so the trailing (reserved) bytes of the 1024-byte
    // superblock area aren't clobbered with stack garbage.
    void flush_superblock(){
        uint32_t sector = bytes_to_sector(SUPERBLOCK_BYTE_OFFSET);
        uint8_t buffer[1024];
        read_block(sector, buffer);
        util::memcpy(buffer, &sb, sizeof(struct superblock));
        write_block(sector, buffer);
    }

    // flush the in-memory bgd table back to disk (same read-modify-write idea).
    // 32 groups x 32 bytes = 1024 bytes max, so one block always suffices
    // given the MAX_BLOCK_GROUPS panic in parse_superblock.
    void flush_bgdt(){
        uint32_t number_bgds = (sb.total_blocks + sb.blocks_per_group - 1) / sb.blocks_per_group;
        uint32_t bgdt_bytes = number_bgds * sizeof(bgd);
        uint32_t sector = bytes_to_sector(BGD_TABLE_BYTE_OFFSET);
        uint8_t buffer[1024];
        read_block(sector, buffer);
        util::memcpy(buffer, bgd_table, bgdt_bytes);
        write_block(sector, buffer);
    }

    // inode 0 is invalid in ext2
    uint32_t find_and_use_free_inode(){
        uint32_t number_bgds = (sb.total_blocks + sb.blocks_per_group - 1) / sb.blocks_per_group;
        uint32_t block_size = 1024u << sb.block_size;
        uint32_t bitmap_bytes = sb.inodes_per_group / 8;

        for (uint32_t g = 0; g < number_bgds; g++){
            struct bgd* block_group_desc = &bgd_table[g];
            if (block_group_desc->free_inodes == 0) continue;

            uint8_t bitmap_buffer[1024];
            uint32_t byte_offset = block_group_desc->inode_usage_bitmap * block_size;
            read_block(bytes_to_sector(byte_offset), bitmap_buffer);

            for (uint32_t i = 0; i < bitmap_bytes; i++){
                uint8_t byte = bitmap_buffer[i];
                if (byte == 0xFF) continue;

                for (int j = 0; j < 8; j++){
                    if (byte & (1u << j)) continue;

                    // claim the bit
                    bitmap_buffer[i] = byte | (1u << j);
                    write_block(bytes_to_sector(byte_offset), bitmap_buffer);

                    // update counters + flush
                    sb.free_inodes--;
                    bgd_table[g].free_inodes--;
                    flush_superblock();
                    flush_bgdt();

                    return g * sb.inodes_per_group + (i * 8) + j + 1;
                }
            }
        }
        return 0;
    }

    // inverse of find_and_use_free_inode: clear the bitmap bit and
    // give the inode back to the counters.
    void free_inode(uint32_t inode_number){
        uint32_t g = get_block_group(inode_number, sb.inodes_per_group);
        uint32_t index = inode_index_within_bg(inode_number, sb.inodes_per_group);
        uint32_t block_size = 1024u << sb.block_size;

        uint8_t bitmap_buffer[1024];
        uint32_t byte_offset = bgd_table[g].inode_usage_bitmap * block_size;
        read_block(bytes_to_sector(byte_offset), bitmap_buffer);

        bitmap_buffer[index / 8] &= (uint8_t)~(1u << (index % 8));
        write_block(bytes_to_sector(byte_offset), bitmap_buffer);

        sb.free_inodes++;
        bgd_table[g].free_inodes++;
        flush_superblock();
        flush_bgdt();
    }

    void free_block(uint32_t block_num){
        uint32_t first_data_block = sb.block_containing_superblock;
        uint32_t g = (block_num - first_data_block) / sb.blocks_per_group;
        uint32_t index = (block_num - first_data_block) % sb.blocks_per_group;
        uint32_t block_size = 1024u << sb.block_size;

        uint8_t bitmap_buffer[1024];
        uint32_t byte_offset = bgd_table[g].block_usage_bitmap * block_size;
        read_block(bytes_to_sector(byte_offset), bitmap_buffer);

        bitmap_buffer[index / 8] &= (uint8_t)~(1u << (index % 8));
        write_block(bytes_to_sector(byte_offset), bitmap_buffer);

        sb.free_blocks++;
        bgd_table[g].free_blocks++;
        flush_superblock();
        flush_bgdt();
    }



    uint32_t find_free_inode(){
        uint32_t number_bgds = (sb.total_blocks + sb.blocks_per_group - 1) / sb.blocks_per_group;
        uint32_t block_size = 1024u << sb.block_size;
        uint32_t bitmap_bytes = sb.inodes_per_group / 8;

        for (uint32_t g = 0; g < number_bgds; g++){
            struct bgd* block_group_desc = &bgd_table[g];
            if (block_group_desc->free_inodes == 0) continue;

            uint8_t bitmap_buffer[1024];
            uint32_t byte_offset = block_group_desc->inode_usage_bitmap * block_size;
            read_block(bytes_to_sector(byte_offset), bitmap_buffer);

            for (uint32_t i = 0; i < bitmap_bytes; i++){
                uint8_t byte = bitmap_buffer[i];
                if (byte == 0xFF) continue;

                for (int j = 0; j < 8; j++){
                    if (byte & (1u << j)) continue;
                    // g * inodes_per_group + n + 1  (inodes are 1-indexed)
                    return g * sb.inodes_per_group + (i * 8) + j + 1;
                }
            }
        }
        return 0;
    }

    // insert a directory entry into a parent directory's data blocks.
    // walks entries via rec_len looking for slack space; splits the entry
    // that owns the gap. returns false if all allocated blocks are full
    // (growing the directory needs a block allocator, which we don't have yet).
    bool insert_dirent(uint32_t parent_inode_num, uint32_t new_inode_num, const char* name, uint8_t file_type){
        struct inode parent = get_inode(parent_inode_num);
        uint32_t block_size = 1024u << sb.block_size;

        uint8_t name_len = (uint8_t)util::strlen(name);
        uint32_t needed = dirent_actual_size(name_len);

        for (int b = 0; b < 12; b++){
            uint32_t block_num = parent.direct_block_ptrs[b];
            if (block_num == 0) break;   // no more allocated dir blocks

            uint8_t block[1024];
            read_block(block_to_sector(block_num), block);

            uint32_t offset = 0;
            while (offset < block_size){
                auto* e = reinterpret_cast<directory_entry*>(block + offset);
                if (e->rec_len == 0) break;   // corrupt block, bail

                // space this entry actually occupies (0 if it's a free slot)
                uint32_t used = (e->inode == 0) ? 0 : dirent_actual_size(e->name_len);

                if (e->rec_len >= used + needed){
                    directory_entry* n;
                    uint16_t new_rec_len;

                    if (e->inode == 0){
                        // reuse the free slot in place
                        n = e;
                        new_rec_len = e->rec_len;
                    } else {
                        // shrink existing entry to its true size,
                        // new entry inherits the remainder
                        uint16_t old_rec_len = e->rec_len;
                        e->rec_len = (uint16_t)used;
                        n = reinterpret_cast<directory_entry*>(block + offset + used);
                        new_rec_len = old_rec_len - (uint16_t)used;
                    }

                    n->inode = new_inode_num;
                    n->rec_len = new_rec_len;
                    n->name_len = name_len;
                    n->file_type = file_type;
                    util::memcpy(n->name, name, name_len);

                    write_block(block_to_sector(block_num), block);
                    return true;
                }

                offset += e->rec_len;
            }
        }
        return false;
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


} // namespace detail


    // high level APIs

    int create_file(const char* name){
        uint32_t inode_number = detail::find_and_use_free_inode();
        if (inode_number == 0){
            util::serial_puts("ext2: create_file failed, no free inodes\r\n");
            return -1;
        }

        detail::inode new_inode = {};
        new_inode.type_permissions = 0x8000 | 0644;
        new_inode.count_hard_links = 1;
        detail::write_inode(inode_number, &new_inode);

        if (!detail::insert_dirent(ROOT_INODE, inode_number, name, (uint8_t)detail::Filetype::REG_FILE)){
            util::serial_puts("ext2: create_file failed, root dir is full\r\n");
            detail::free_inode(inode_number);
            return -1;
        }

        util::serial_puts("ext2: created file '");
        util::serial_puts(name);
        util::serial_puts("' with inode ");
        util::serial_putdec(inode_number);
        util::serial_puts("\r\n");

        return 0;
    }

    // unlink a name from the root directory. if that was the last hard link
    // release the inode back to the bitmap + counters

    
    bool delete_file(const char* name){
        detail::inode parent = detail::get_inode(ROOT_INODE);
        uint32_t block_size = 1024u << detail::sb.block_size;
        uint8_t name_len = (uint8_t)util::strlen(name);

        for (int b = 0; b < 12; b++){
            uint32_t block_num = parent.direct_block_ptrs[b];
            if (block_num == 0) break;

            uint8_t block[1024];
            detail::read_block(detail::block_to_sector(block_num), block);

            detail::directory_entry* prev = nullptr;
            uint32_t offset = 0;
            while (offset < block_size){
                auto* e = reinterpret_cast<detail::directory_entry*>(block + offset);
                if (e->rec_len == 0) break;

                if (e->inode != 0 && detail::dirent_name_matches(e, name, name_len)){
                    uint32_t target_inode_num = e->inode;

                    if (prev != nullptr){
                        prev->rec_len += e->rec_len;
                    } else {
                        e->inode = 0;
                    }
                    detail::write_block(detail::block_to_sector(block_num), block);

                    detail::inode target = detail::get_inode(target_inode_num);
                    target.count_hard_links--;

                    if (target.count_hard_links == 0){
                        detail::write_inode(target_inode_num, &target);
                        detail::free_inode(target_inode_num);
                        for (int b = 0; b < 12; b++){
                            uint32_t block_num = target.direct_block_ptrs[b];
                            if (block_num == 0) break;
                            // dont wanna deal with indirect ones yet
                            detail::free_block(block_num);
                        }
                    } else {
                        detail::write_inode(target_inode_num, &target);
                    }

                    util::serial_puts("ext2: deleted '");
                    util::serial_puts(name);
                    util::serial_puts("' (inode ");
                    util::serial_putdec(target_inode_num);
                    util::serial_puts(")\r\n");
                    return 0;
                }

                prev = e;
                offset += e->rec_len;
            }
        }

        util::serial_puts("ext2: delete_file: '");
        util::serial_puts(name);
        util::serial_puts("' not found\r\n");
        return -1;
    }

    // search a single directory (by inode number) for an entry named
    // name[0..name_len). returns its inode number, or 0 if not present.
    // name need not be NUL-terminated (only name_len bytes are compared).
    uint32_t lookup_in_dir(uint32_t dir_inode_num, const char* name, uint8_t name_len){
        detail::inode dir = detail::get_inode(dir_inode_num);
        uint32_t block_size = 1024u << detail::sb.block_size;

        for (int b = 0; b < 12; b++){
            uint32_t block_num = dir.direct_block_ptrs[b];
            if (block_num == 0) break;

            uint8_t block[1024];
            detail::read_block(detail::block_to_sector(block_num), block);

            uint32_t offset = 0;
            while (offset < block_size){
                auto* e = reinterpret_cast<detail::directory_entry*>(block + offset);
                if (e->rec_len == 0) break;

                if (e->inode != 0 && detail::dirent_name_matches(e, name, name_len)){
                    return e->inode;
                }
                offset += e->rec_len;
            }
        }
        return 0;
    }

    // resolve a '/'-separated path from the root directory, one component at a
    // time. leading and duplicate slashes are ignored. returns the target
    // inode number, or 0 if any component is missing. (empty path -> root.)
    uint32_t lookup(const char* path){
        uint32_t current = ROOT_INODE;
        const char* p = path;

        while (*p == '/') p++;              // skip leading slashes

        while (*p){
            const char* start = p;
            while (*p && *p != '/') p++;    // span one path component
            uint8_t len = (uint8_t)(p - start);

            current = lookup_in_dir(current, start, len);
            if (current == 0) return 0;

            while (*p == '/') p++;          // skip separators to next component
        }
        return current;
    }

    // list the entries of the root directory. each entry's name is written,
    // NUL-terminated, into names[i]; at most max_entries are produced.
    // returns the number of names written.
    // (only the root directory is supported for now, like lookup/delete_file.)
    uint32_t list_entries(char names[][256], uint32_t max_entries){
        detail::inode dir = detail::get_inode(ROOT_INODE);
        uint32_t block_size = 1024u << detail::sb.block_size;
        uint32_t count = 0;

        for (int b = 0; b < 12; b++){
            uint32_t block_num = dir.direct_block_ptrs[b];
            if (block_num == 0) break;

            uint8_t block[1024];
            detail::read_block(detail::block_to_sector(block_num), block);

            uint32_t offset = 0;
            while (offset < block_size){
                auto* e = reinterpret_cast<detail::directory_entry*>(block + offset);
                if (e->rec_len == 0) break;   // corrupt block, bail

                if (e->inode != 0){           // 0 == free/deleted slot
                    if (count >= max_entries) return count;

                    uint8_t n = e->name_len;  // name_len <= 255, fits names[i][256]
                    util::memcpy(names[count], e->name, n);
                    names[count][n] = '\0';
                    count++;
                }
                offset += e->rec_len;
            }
        }
        return count;
    }

    // copy a file's contents into out. returns bytes read, or:
    //   -1  not found
    //   -2  file bigger than max_len (caller's buffer too small)
    //   -3  file needs indirect blocks (not implemented yet)
    int64_t read_file(const char* name, uint8_t* out, uint32_t max_len){
        uint32_t inode_num = lookup(name);
        if (inode_num == 0) return -1;

        detail::inode in = detail::get_inode(inode_num);
        uint32_t block_size = 1024u << detail::sb.block_size;

        if (in.size_low > max_len) return -2;
        if (in.size_low > 12u * block_size) return -3;   // would truncate silently otherwise

        uint32_t remaining = in.size_low;
        uint32_t copied = 0;

        for (int b = 0; b < 12 && remaining > 0; b++){
            uint32_t block_num = in.direct_block_ptrs[b];
            uint32_t chunk = (remaining < block_size) ? remaining : block_size;

            if (block_num == 0){
                for (uint32_t k = 0; k < chunk; k++) out[copied + k] = 0;
            } else {
                uint8_t block[1024];
                detail::read_block(detail::block_to_sector(block_num), block);
                util::memcpy(out + copied, block, chunk);
            }

            copied += chunk;
            remaining -= chunk;
        }

        return (int64_t)copied;
    }

    void parse_superblock(){
        uint32_t sb_sector_offset = detail::bytes_to_sector(SUPERBLOCK_BYTE_OFFSET);
        uint8_t buffer[1024];
        detail::read_block(sb_sector_offset, buffer);

        util::memcpy(&detail::sb, buffer, sizeof(detail::superblock));

        util::serial_puts("Ext2 Superblock:\r\n");
        util::serial_puts("Total inodes: ");
        util::serial_putdec(detail::sb.total_inodes);
        util::serial_puts("\r\nTotal blocks: ");
        util::serial_putdec(detail::sb.total_blocks);
        util::serial_puts("\r\nFree blocks: ");
        util::serial_putdec(detail::sb.free_blocks);
        util::serial_puts("\r\nFree inodes: ");
        util::serial_putdec(detail::sb.free_inodes);
        util::serial_puts("\r\nBlock size: ");
        util::serial_putdec(1024u << detail::sb.block_size);
        util::serial_puts("\r\nBlocks per group: ");
        util::serial_putdec(detail::sb.blocks_per_group);
        util::serial_puts("\r\nInodes per group: ");
        util::serial_putdec(detail::sb.inodes_per_group);
        util::serial_puts("\r\nSignature: ");
        util::serial_puthex(detail::sb.signature);
        util::serial_puts("\r\nInode size: ");
        util::serial_putdec(detail::sb.inode_size);
        util::serial_puts("\r\n");

        uint32_t number_bgds = detail::sb.total_blocks / detail::sb.blocks_per_group;
        if (number_bgds > MAX_BLOCK_GROUPS) {
            KPANIC("ext2: block group descriptor table exceeds MAX_BLOCK_GROUPS");
        }

        uint32_t bgdt_sector_offset = detail::bytes_to_sector(BGD_TABLE_BYTE_OFFSET);
        uint32_t bgdt_bytes = number_bgds * sizeof(detail::bgd);
        uint32_t bgdt_sectors = (bgdt_bytes + BYTES_PER_SECTOR - 1) / BYTES_PER_SECTOR;

        uint8_t bgdt_buffer[bgdt_sectors * BYTES_PER_SECTOR];

        for (uint32_t i = 0; i < bgdt_sectors; i++) {
            ata::read_sector(bgdt_sector_offset + i, bgdt_buffer + i * BYTES_PER_SECTOR);
        }

        util::memcpy(detail::bgd_table, bgdt_buffer, bgdt_bytes);

        util::serial_puts("\r\nBlock Group Descriptor Table:\r\n");
        for (uint32_t i = 0; i < number_bgds; i++) {
            detail::bgd& group = detail::bgd_table[i];
            util::serial_puts("Group ");
            util::serial_putdec(i);
            util::serial_puts(":\r\n  Block usage bitmap addr: ");
            util::serial_puthex(group.block_usage_bitmap);
            util::serial_puts("\r\n  Inode usage bitmap addr: ");
            util::serial_puthex(group.inode_usage_bitmap);
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

} // namespace fs::ext2