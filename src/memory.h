#pragma once

#include "include/types.h"

struct AllocOnlyPool;

extern struct AllocOnlyPool *alloc_only_pool_init(void);
extern void *alloc_only_pool_alloc(struct AllocOnlyPool *pool, s32 size);
extern void *alloc_display_list(u32 size);
extern void main_pool_free(struct AllocOnlyPool *pool);