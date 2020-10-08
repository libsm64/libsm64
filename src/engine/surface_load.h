#pragma once

#include "../libsm64.h"
#include "../include/types.h"

struct Surface **surface_load_from_collision_data( const s16 *data, size_t *numSurfaces );

void surface_load_for_libsm64( const struct SM64Surface *surfaceArray, size_t numSurfaces );