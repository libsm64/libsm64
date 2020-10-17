#include "load_surfaces.h"

#include <stdlib.h>
#include <string.h>

#include "include/types.h"
#include "include/surface_terrains.h"
#include "shim.h"

static uint32_t s_static_surface_count = 0;
static struct Surface *s_static_surface_list = NULL;

static uint32_t s_surface_object_count = 0;
static struct SM64SurfaceObject *s_surface_object_list = NULL;

static uint32_t s_dynamic_surface_count = 0;
static struct Surface *s_dynamic_surface_list = NULL;

/**
 * Returns whether a surface has exertion/moves Mario
 * based on the surface type.
 */
static s32 surface_has_force(s16 surfaceType) {
    s32 hasForce = FALSE;

    switch (surfaceType) {
        case SURFACE_0004: // Unused
        case SURFACE_FLOWING_WATER:
        case SURFACE_DEEP_MOVING_QUICKSAND:
        case SURFACE_SHALLOW_MOVING_QUICKSAND:
        case SURFACE_MOVING_QUICKSAND:
        case SURFACE_HORIZONTAL_WIND:
        case SURFACE_INSTANT_MOVING_QUICKSAND:
            hasForce = TRUE;
            break;

        default:
            break;
    }
    return hasForce;
}

static void engine_surface_from_lib_surface( struct Surface *surface, const struct SM64Surface *libSurf, const struct SM64ObjectTransform *transform )
{
    int16_t type = libSurf->type;
    int16_t force = libSurf->force;
    s32 x1 = libSurf->vertices[0][0];
    s32 y1 = libSurf->vertices[0][1];
    s32 z1 = libSurf->vertices[0][2];
    s32 x2 = libSurf->vertices[1][0];
    s32 y2 = libSurf->vertices[1][1];
    s32 z2 = libSurf->vertices[1][2];
    s32 x3 = libSurf->vertices[2][0];
    s32 y3 = libSurf->vertices[2][1];
    s32 z3 = libSurf->vertices[2][2];

    s32 maxY, minY;
    f32 nx, ny, nz;
    f32 mag;

    // (v2 - v1) x (v3 - v2)
    nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
    ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
    nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
    mag = sqrtf(nx * nx + ny * ny + nz * nz);

    // Could have used min_3 and max_3 for this...
    minY = y1;
    if (y2 < minY) {
        minY = y2;
    }
    if (y3 < minY) {
        minY = y3;
    }

    maxY = y1;
    if (y2 > maxY) {
        maxY = y2;
    }
    if (y3 > maxY) {
        maxY = y3;
    }

    if (mag < 0.0001)
        DEBUG_LOG("ERROR: normal magnitude is very close to zero");

    mag = (f32)(1.0 / mag);
    nx *= mag;
    ny *= mag;
    nz *= mag;

    // TODO apply tranform

    surface->vertex1[0] = x1;
    surface->vertex2[0] = x2;
    surface->vertex3[0] = x3;

    surface->vertex1[1] = y1;
    surface->vertex2[1] = y2;
    surface->vertex3[1] = y3;

    surface->vertex1[2] = z1;
    surface->vertex2[2] = z2;
    surface->vertex3[2] = z3;

    surface->normal.x = nx;
    surface->normal.y = ny;
    surface->normal.z = nz;

    surface->originOffset = -(nx * x1 + ny * y1 + nz * z1);

    surface->lowerY = minY - 5;
    surface->upperY = maxY + 5;

    s16 hasForce = surface_has_force(type);
    s16 flags = 0; // surf_has_no_cam_collision(type);

    surface->room = 0;
    surface->type = type;
    surface->flags = (s8) flags;

    if (hasForce) {
        surface->force = force;
    } else {
        surface->force = 0;
    }

    return surface;
}

static void update_dynamic_surface_list( void )
{
    if( s_dynamic_surface_list != NULL )
        free( s_dynamic_surface_list );

    s_dynamic_surface_count = 0;
    s_dynamic_surface_list = NULL;

    for( int i = 0; i < s_surface_object_count; ++i )
    {
        for( int j = 0; j < s_surface_object_list[i].surfaceCount; ++j )
        {
            s_dynamic_surface_count++;
            s_dynamic_surface_list = realloc( s_dynamic_surface_list, s_dynamic_surface_count * sizeof( struct Surface ));
            engine_surface_from_lib_surface( &s_dynamic_surface_list[s_dynamic_surface_count - 1], &s_surface_object_list[i].surfaces[j], &s_surface_object_list[i].transform );
        }
    }
}


struct Surface *loaded_surface_get_at_index( uint32_t index )
{
    if( index < s_static_surface_count )
        return &s_static_surface_list[ index ];
    
    return &s_dynamic_surface_list[ index - s_static_surface_count ];
}

uint32_t loaded_surface_get_count()
{
    return s_static_surface_count + s_dynamic_surface_count;
}

void surfaces_load_static_libsm64( const struct SM64Surface *surfaceArray, uint32_t numSurfaces )
{
    s_static_surface_count = numSurfaces;
    s_static_surface_list = malloc( sizeof( struct Surface ) * numSurfaces );

    for( int i = 0; i < numSurfaces; ++i )
        engine_surface_from_lib_surface( &s_static_surface_list[i], &surfaceArray[i], NULL );
}

uint32_t surfaces_load_object( const struct SM64SurfaceObject *surfaceObject )
{
    uint32_t idx = s_surface_object_count;
    s_surface_object_count++;
    s_surface_object_list = realloc( s_surface_object_list, s_surface_object_count * sizeof( struct SM64SurfaceObject ));

    struct SM64SurfaceObject *obj = &s_surface_object_list[idx];
    obj->transform = surfaceObject->transform;
    obj->surfaceCount = surfaceObject->surfaceCount;
    obj->surfaces = malloc( obj->surfaceCount * sizeof( struct SM64Surface ));
    memcpy( obj->surfaces, surfaceObject->surfaces, obj->surfaceCount * sizeof( struct SM64Surface ));

    update_dynamic_surface_list();

    return idx;
}