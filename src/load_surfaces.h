#pragma once

#include "libsm64.h"

extern struct Surface *loaded_surface_get_at_index( uint32_t index );
extern uint32_t loaded_surface_get_count();

extern void surfaces_load_static_libsm64( const struct SM64Surface *surfaceArray, uint32_t numSurfaces );
extern uint32_t surfaces_load_object( const struct SM64SurfaceObject *surfaceObject );