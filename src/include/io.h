#ifndef KVS_IO_H
#define KVS_IO_H

#include <stdint.h>
#include "spdk/queue.h"
#include "uthash.h"
#include "pool.h"
#include "hashmap.h"

#include "spdk/blob.h"
struct page_io{
    uint64_t key;

    /**
     * @brief When performing the loading, the io_link is used to link the last
     * page io of a multi-pages loading if the last page is not in loading state. 
     * 
     * When performing the storing, the io_link is used to indicate whether current
     * page IO needs to do phase-2 writing. If it is NULL, I needn't do phase-2
     * writing. Ortherwise, the phase-2 writing need performing.
     */
    struct page_io *io_link;

    struct cache_io *cache_io;
    struct iomgr *imgr;

    uint64_t start_page;
    uint64_t len;
    struct spdk_blob* blob;
    uint8_t *buf;

    TAILQ_HEAD(, page_io) pio_head;
    TAILQ_ENTRY(page_io) link;
};

struct cache_io{
    uint64_t key[2];
    struct iomgr *imgr;

    //cnt means how many page_ios for the cache_io
    uint32_t cnt;
    uint32_t nb_segments;
    uint32_t nb_pages;
    uint64_t start_page;
    uint8_t* buf;
    struct spdk_blob *blob;

    int kverrno;
    void(*cb)(void*ctx, int kverrno);
    void* ctx;

    TAILQ_HEAD(, cache_io) cio_head;
    TAILQ_ENTRY(cache_io) link;
};

struct pending_io_hash{
    map_t page_hash;
    map_t cache_hash;
};

struct iomgr{
    struct spdk_io_channel *channel;
    //When I resize blob, I shall send such operation to the 
    //the thread that initializing the blobstore.
    struct spdk_thread *meta_thread;
    struct spdk_blob_store *target;
    
    uint32_t max_pending_io; 
    uint32_t nb_pending_io;

    TAILQ_HEAD(,cache_io) pending_read_head;
    TAILQ_HEAD(,cache_io) pending_write_head;

    struct pending_io_hash read_hash;
    struct pending_io_hash write_hash;

    struct object_cache_pool *cache_io_pool;
    struct object_cache_pool *page_io_pool;
};

#endif