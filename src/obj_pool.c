#include "obj_pool.h"

#include <stdlib.h>

uint32_t obj_pool_alloc_index( struct ObjPool *pool, size_t size )
{
    for( uint32_t i = 0; i < pool->size; ++i )
    {
        if( pool->objects[i] == NULL )
        {
            pool->objects[i] = malloc( size );
            return i;
        }
    }

    uint32_t i = pool->size;
    pool->size++;
    pool->objects = realloc( pool->objects, pool->size * sizeof( void * ));
    pool->objects[i] = malloc( size );
    return i;
}

void obj_pool_free_index( struct ObjPool *pool, uint32_t index )
{
    free( pool->objects[index] );
    pool->objects[index] = NULL;
}

void obj_pool_free_all( struct ObjPool *pool )
{
    for( uint32_t i = 0; i < pool->size; ++i )
        free( pool->objects[i] );
    free( pool->objects );

    pool->size = 0;
    pool->objects = NULL;
}