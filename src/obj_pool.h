#pragma once

#include <stddef.h>
#include <stdint.h>

struct ObjPool
{
    size_t size;
    void **objects;
};

extern uint32_t obj_pool_alloc_index( struct ObjPool *pool, size_t size );
extern void obj_pool_free_index( struct ObjPool *pool, uint32_t index );
extern void obj_pool_free_all( struct ObjPool *pool );