namespace fs::bcache {

enum class BCACHE_ENTRY_STATUS {
    MODIFIED,
    UNMODIFIED,
    // in future add BUSY to handle multiple processes
};

struct bcache_entry {
    uint32_t block_num;
    BCACHE_ENTRY_STATUS status;
    uint8_t[1024] data;

    bcache_entry() : block_num(-1), status(BCACHE_ENTRY_STATUS::UNMODIFIED) {}
};
}