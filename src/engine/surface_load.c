#include <stdlib.h>
#include <math.h>
#include "../include/surface_terrains.h"
#include "../include/surface_terrains.h"
#include "../shim.h"
#include "surface_load.h"
#include "surface_collision.h"

struct Surface **s_surface_list = NULL;
size_t s_surface_list_count = 0;
struct SurfaceNode *s_surface_node_list = NULL;

static void add_surface_ex(struct Surface *surface, s32 dynamic)
{
    s_surface_list_count++;

    if( s_surface_list == NULL )
        s_surface_list = malloc( s_surface_list_count * sizeof( struct Surface * ));
    else
        s_surface_list = realloc( s_surface_list, s_surface_list_count * sizeof( struct Surface * ));

    s_surface_list[s_surface_list_count - 1] = surface;
}

static struct Surface *alloc_surface(void) {
    struct Surface *surface = malloc(sizeof(struct Surface));  // PATCH - just use malloc instead of pool
//  struct Surface *surface = &sSurfacePool[gSurfacesAllocated];
//  gSurfacesAllocated++;

    //! A bounds check! If there's more surfaces than the 2300 allowed,
    //  we, um...
    // Perhaps originally just debug feedback?
//  if (gSurfacesAllocated >= sSurfacePoolSize) {
//  }

    surface->type = 0;
    surface->force = 0;
    surface->flags = 0;
    surface->room = 0;
    surface->object = NULL;

    return surface;
}

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

/**
 * Returns whether a surface should have the
 * SURFACE_FLAG_NO_CAM_COLLISION flag.
 */
static s32 surf_has_no_cam_collision(s16 surfaceType) {
    s32 flags = 0;

    switch (surfaceType) {
        case SURFACE_NO_CAM_COLLISION:
        case SURFACE_NO_CAM_COLLISION_77: // Unused
        case SURFACE_NO_CAM_COL_VERY_SLIPPERY:
        case SURFACE_SWITCH:
            flags = SURFACE_FLAG_NO_CAM_COLLISION;
            break;

        default:
            break;
    }

    return flags;
}


static struct Surface *read_surface_data_ex( s32 x1, s32 y1, s32 z1, s32 x2, s32 y2, s32 z2, s32 x3, s32 y3, s32 z3) {
    struct Surface *surface;
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

    // Checking to make sure no DIV/0
    if (mag < 0.0001) {
        return NULL;
    }
    mag = (f32)(1.0 / mag);
    nx *= mag;
    ny *= mag;
    nz *= mag;

    surface = alloc_surface();

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

    return surface;
}

/**
 * Initializes a Surface struct using the given vertex data
 * @param vertexData The raw data containing vertex positions
 * @param vertexIndices Helper which tells positions in vertexData to start reading vertices
 */
static struct Surface *read_surface_data(s16 *vertexData, s16 **vertexIndices) {
    register s32 x1, y1, z1;
    register s32 x2, y2, z2;
    register s32 x3, y3, z3;
    s16 offset1, offset2, offset3;

    offset1 = 3 * (*vertexIndices)[0];
    offset2 = 3 * (*vertexIndices)[1];
    offset3 = 3 * (*vertexIndices)[2];

    x1 = *(vertexData + offset1 + 0);
    y1 = *(vertexData + offset1 + 1);
    z1 = *(vertexData + offset1 + 2);

    x2 = *(vertexData + offset2 + 0);
    y2 = *(vertexData + offset2 + 1);
    z2 = *(vertexData + offset2 + 2);

    x3 = *(vertexData + offset3 + 0);
    y3 = *(vertexData + offset3 + 1);
    z3 = *(vertexData + offset3 + 2);

    return read_surface_data_ex( x1, y1, z1, x2, y2, z2, x3, y3, z3 );
}

/**
 * Read the data for vertices for reference by triangles.
 */
static s16 *read_vertex_data(s16 **data) {
    s32 numVertices;
    UNUSED s16 unused1[3];
    UNUSED s16 unused2[3];
    s16 *vertexData;

    numVertices = *(*data);
    (*data)++;

    vertexData = *data;
    *data += 3 * numVertices;

    return vertexData;
}

/**
 * Load in the surfaces for a given surface type. This includes setting the flags,
 * exertion, and room.
 */
static void load_static_surfaces(s16 **data, s16 *vertexData, s16 surfaceType, s8 **surfaceRooms) {
    s32 i;
    s32 numSurfaces;
    struct Surface *surface;
    s8 room = 0;
    s16 hasForce = surface_has_force(surfaceType);
    s16 flags = surf_has_no_cam_collision(surfaceType);

    numSurfaces = *(*data);
    *data += 1;

    for (i = 0; i < numSurfaces; i++) {
        if (*surfaceRooms != NULL) {
            room = *(*surfaceRooms);
            *surfaceRooms += 1;
        }

        surface = read_surface_data(vertexData, data);
        if (surface != NULL) {
            surface->room = room;
            surface->type = surfaceType;
            surface->flags = (s8) flags;

            if (hasForce) {
                surface->force = *(*data + 3);
            } else {
                surface->force = 0;
            }

            add_surface_ex(surface, FALSE);
        }

        *data += 3;
        if (hasForce) {
            *data += 1;
        }
    }
}

static void load_area_terrain(s16 index, s16 *data, s8 *surfaceRooms, s16 *macroObjects) {
    s16 terrainLoadType;
    s16 *vertexData;
    UNUSED s32 unused;

    // Initialize the data for this.
//  gEnvironmentRegions = NULL;
//  unused8038BE90 = 0;
//  gSurfaceNodesAllocated = 0;
//  gSurfacesAllocated = 0;

//  clear_static_surfaces();

    // A while loop iterating through each section of the level data. Sections of data
    // are prefixed by a terrain "type." This type is reused for surfaces as the surface
    // type.
    while (TRUE) {
        terrainLoadType = *data;
        data++;

        if (TERRAIN_LOAD_IS_SURFACE_TYPE_LOW(terrainLoadType)) {
            load_static_surfaces(&data, vertexData, terrainLoadType, &surfaceRooms);
        } else if (terrainLoadType == TERRAIN_LOAD_VERTICES) {
            vertexData = read_vertex_data(&data);
//      } else if (terrainLoadType == TERRAIN_LOAD_OBJECTS) {
//          spawn_special_objects(index, &data);
//      } else if (terrainLoadType == TERRAIN_LOAD_ENVIRONMENT) {
//          load_environmental_regions(&data);
        } else if (terrainLoadType == TERRAIN_LOAD_CONTINUE) {
//          continue;
            break; // PATCH
        } else if (terrainLoadType == TERRAIN_LOAD_END) {
            break;
        } else if (TERRAIN_LOAD_IS_SURFACE_TYPE_HIGH(terrainLoadType)) {
            load_static_surfaces(&data, vertexData, terrainLoadType, &surfaceRooms);
            continue;
        }
    }

//  if (macroObjects != NULL && *macroObjects != -1) {
//      // If the first macro object presetID is within the range [0, 29].
//      // Generally an early spawning method, every object is in BBH (the first level).
//      if (0 <= *macroObjects && *macroObjects < 30) {
//          spawn_macro_objects_hardcoded(index, macroObjects);
//      }
//      // A more general version that can spawn more objects.
//      else {
//          spawn_macro_objects(index, macroObjects);
//      }
//  }

//  gNumStaticSurfaceNodes = gSurfaceNodesAllocated;
//  gNumStaticSurfaces = gSurfacesAllocated;
}




// Unused, loads from raw sm64 collision data
// struct Surface **surface_load_from_collision_data( const s16 *data, size_t *numSurfaces )
// {
//     s8 *rooms = NULL;
//     s16 *macroObjects = NULL;
//     load_area_terrain( 0, (s16*)data, rooms, macroObjects );
//     *numSurfaces = s_surface_list_count;
//     return s_surface_list;
// }


void surface_load_for_libsm64( const struct SM64Surface *surfaceArray, uint32_t numSurfaces )
{
    while( s_surface_node_list )
    {
        struct SurfaceNode *node = s_surface_node_list;
        s_surface_node_list = node->next;
        free( node );
    }

    if( s_surface_list )
    {
        for( int i = 0; i < s_surface_list_count; ++i )
            free( s_surface_list[i] );
        
        free( s_surface_list );

        s_surface_list = NULL;
        s_surface_list_count = 0;
    }

    for( int i = 0; i < numSurfaces; ++i )
    {
        struct Surface *surface = read_surface_data_ex(
            surfaceArray[i].vertices[0][0], surfaceArray[i].vertices[0][1], surfaceArray[i].vertices[0][2],
            surfaceArray[i].vertices[1][0], surfaceArray[i].vertices[1][1], surfaceArray[i].vertices[1][2],
            surfaceArray[i].vertices[2][0], surfaceArray[i].vertices[2][1], surfaceArray[i].vertices[2][2]
        );

        if (surface != NULL) {
            s16 hasForce = surface_has_force(surfaceArray[i].type);
            s16 flags = surf_has_no_cam_collision(surfaceArray[i].type);

            surface->room = 0;
            surface->type = surfaceArray[i].type;
            surface->flags = (s8) flags;

            if (hasForce) {
                surface->force = surfaceArray[i].force;
            } else {
                surface->force = 0;
            }

            add_surface_ex(surface, FALSE);
        }
    }

    struct Surface **ptr = s_surface_list;
    struct SurfaceNode *node = malloc( sizeof( struct SurfaceNode ));
    node->surface = *ptr;
    s_surface_node_list = node;

    for( int i = 1; i < s_surface_list_count; ++i )
    {
        ptr++;
        struct SurfaceNode *next = malloc( sizeof( struct SurfaceNode ));
        next->surface = *ptr;
        next->next = NULL;
        node->next = next;
        node = next;
    }

    hack_load_surface_list( s_surface_node_list );
}