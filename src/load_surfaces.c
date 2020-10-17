#include "load_surfaces.h"

#include "include/types.h"
#include "include/surface_terrains.h"


static struct Surface *s_loaded_static_surface_list = NULL;
static size_t s_loaded_static_surface_count = 0;


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

static void read_surface_data( struct Surface *surface, int16_t type, int16_t force, s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2, s32 x3, s32 y3, s32 z3)
{
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

    surface = malloc(sizeof(struct Surface)); 

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


struct Surface *loaded_surface_get_at_index( uint32_t index )
{
    return &s_loaded_static_surface_list[ index ];
}

uint32_t loaded_surface_get_count()
{
    return s_loaded_static_surface_count;
}

void surfaces_load_static_libsm64( const struct SM64Surface *surfaceArray, uint32_t numSurfaces )
{
    s_loaded_static_surface_count = numSurfaces;
    s_loaded_static_surface_list = malloc( sizeof( struct Surface ) * numSurfaces );

    for( int i = 0; i < numSurfaces; ++i )
        read_surface_data(
            &s_loaded_static_surface_list[i], surfaceArray[i].type, surfaceArray[i].force,
            surfaceArray[i].vertices[0][0], surfaceArray[i].vertices[0][1], surfaceArray[i].vertices[0][2],
            surfaceArray[i].vertices[1][0], surfaceArray[i].vertices[1][1], surfaceArray[i].vertices[1][2],
            surfaceArray[i].vertices[2][0], surfaceArray[i].vertices[2][1], surfaceArray[i].vertices[2][2]
        );
}

uint32_t surfaces_load_object( const struct SM64SurfaceObject *surfaceObject )
{
    return 0;
}