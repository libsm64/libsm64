#include <stdlib.h>
#include <stdbool.h>

#include "memory.h"

// TODO don't handle every individual allocation with malloc, it sucks on windows

struct AllocOnlyPool 
{
    size_t allocatedCount;
    void **allocatedBlocks;
};

struct AllocOnlyPool *s_display_list_pool;

void memory_init(void)
{
    s_display_list_pool = alloc_only_pool_init();
}

void memory_terminate(void)
{
    alloc_only_pool_free( s_display_list_pool );
}

struct AllocOnlyPool *alloc_only_pool_init(void)
{
    struct AllocOnlyPool *newPool = malloc( sizeof( struct AllocOnlyPool ));
    newPool->allocatedCount = 0;
    newPool->allocatedBlocks = NULL;
    return newPool;
}

void *alloc_only_pool_alloc(struct AllocOnlyPool *pool, s32 size)
{
    pool->allocatedCount++;
    pool->allocatedBlocks = realloc( pool->allocatedBlocks, pool->allocatedCount * sizeof( void * ));
    pool->allocatedBlocks[ pool->allocatedCount - 1 ] = malloc( size );
    return pool->allocatedBlocks[ pool->allocatedCount - 1 ];
}

void alloc_only_pool_free(struct AllocOnlyPool *pool)
{
    for( int i = 0; i < pool->allocatedCount; ++i )
        free( pool->allocatedBlocks[i] );
    free( pool->allocatedBlocks );
    free( pool );
}

void display_list_pool_reset(void)
{
    alloc_only_pool_free( s_display_list_pool );
    s_display_list_pool = alloc_only_pool_init();
}

void *alloc_display_list(u32 size)
{
    return alloc_only_pool_alloc( s_display_list_pool, (s32)size );
}