#include "../shim.h"
#include "surface_collision.h"
#include "../include/surface_terrains.h"
#include "../../load_surfaces.h"

/**
 * Iterate through the list of ceilings and find the first ceiling over a given point.
 */
static struct SM64SurfaceCollisionData *find_ceil_from_list( s32 x, s32 y, s32 z, f32 *pheight) {
    register struct SM64SurfaceCollisionData *surf;
    register s32 x1, z1, x2, z2, x3, z3;
    struct SM64SurfaceCollisionData *ceil = NULL;

    ceil = NULL;

    uint32_t groupCount = loaded_surface_iter_group_count();
    for( int i = 0; i < groupCount; ++i ) {
    uint32_t surfCount = loaded_surface_iter_group_size( i );
    for( int j = 0; j < surfCount; ++j ) {
        surf = loaded_surface_iter_get_at_index( i, j );

        // libsm64: Weed out surfaces whose triangles are actually line segs. TODO do this at surface load time
        if( !surf->isValid ) continue;

        // Do the check normally done in add_surface_to_cell
        if( surf->normal.y >= -0.01f ) continue;

        x1 = surf->vertex1[0];
        z1 = surf->vertex1[2];
        z2 = surf->vertex2[2];
        x2 = surf->vertex2[0];

        // Checking if point is in bounds of the triangle laterally.
        if ((z1 - z) * (x2 - x1) - (x1 - x) * (z2 - z1) > 0) {
            continue;
        }

        // Slight optimization by checking these later.
        x3 = surf->vertex3[0];
        z3 = surf->vertex3[2];
        if ((z2 - z) * (x3 - x2) - (x2 - x) * (z3 - z2) > 0) {
            continue;
        }
        if ((z3 - z) * (x1 - x3) - (x3 - x) * (z1 - z3) > 0) {
            continue;
        }

        {
            f32 nx = surf->normal.x;
            f32 ny = surf->normal.y;
            f32 nz = surf->normal.z;
            f32 oo = surf->originOffset;
            f32 height;

            // If a wall, ignore it. Likely a remnant, should never occur.
            if (ny == 0.0f) {
                continue;
            }

            // Find the ceil height at the specific point.
            height = -(x * nx + nz * z + oo) / ny;

            // Checks for ceiling interaction with a 78 unit buffer.
            //! (Exposed Ceilings) Because any point above a ceiling counts
            //  as interacting with a ceiling, ceilings far below can cause
            // "invisible walls" that are really just exposed ceilings.
            if (y - (height - -78.0f) > 0.0f) {
                continue;
            }

            if( height < *pheight )
            {
                *pheight = height;
                ceil = surf;
            }
        }
    }}
    return ceil;
}

/**
 * Iterate through the list of floors and find the first floor under a given point.
 */
static struct SM64SurfaceCollisionData *find_floor_from_list( s32 x, s32 y, s32 z, f32 *pheight) {
    register struct SM64SurfaceCollisionData *surf;
    register s32 x1, z1, x2, z2, x3, z3;
    f32 nx, ny, nz;
    f32 oo;
    f32 height;
    struct SM64SurfaceCollisionData *floor = NULL;

    uint32_t groupCount = loaded_surface_iter_group_count();
    for( int i = 0; i < groupCount; ++i ) {
    uint32_t surfCount = loaded_surface_iter_group_size( i );
    for( int j = 0; j < surfCount; ++j ) {
        surf = loaded_surface_iter_get_at_index( i, j );

        // libsm64: Weed out surfaces whose triangles are actually line segs. TODO do this at surface load time
        if( !surf->isValid ) continue;

        // Do the check normally done in add_surface_to_cell
        if( surf->normal.y <= 0.01f ) continue;

        x1 = surf->vertex1[0];
        z1 = surf->vertex1[2];
        x2 = surf->vertex2[0];
        z2 = surf->vertex2[2];

        // Check that the point is within the triangle bounds.
        if ((z1 - z) * (x2 - x1) - (x1 - x) * (z2 - z1) < 0) {
            continue;
        }

        // To slightly save on computation time, set this later.
        x3 = surf->vertex3[0];
        z3 = surf->vertex3[2];

        if ((z2 - z) * (x3 - x2) - (x2 - x) * (z3 - z2) < 0) {
            continue;
        }
        if ((z3 - z) * (x1 - x3) - (x3 - x) * (z1 - z3) < 0) {
            continue;
        }

        nx = surf->normal.x;
        ny = surf->normal.y;
        nz = surf->normal.z;
        oo = surf->originOffset;

        // If a wall, ignore it. Likely a remnant, should never occur.
        if (ny == 0.0f) {
            continue;
        }

        // Find the height of the floor at a given location.
        height = -(x * nx + nz * z + oo) / ny;
        // Checks for floor interaction with a 78 unit buffer.
        if (y - (height + -78.0f) < 0.0f) {
            continue;
        }

        if( height > *pheight )
        {
            *pheight = height;
            floor = surf;
        }
    }}
    return floor;
}

static s32 find_wall_collisions_from_list( struct SM64WallCollisionData *data) {
    register struct SM64SurfaceCollisionData *surf;
    register f32 offset;
    register f32 radius = data->radius;
    register f32 x = data->x;
    register f32 y = data->y + data->offsetY;
    register f32 z = data->z;
    register f32 px, pz;
    register f32 w1, w2, w3;
    register f32 y1, y2, y3;
    s32 numCols = 0;

    // Max collision radius = 200
    if (radius > 200.0f) {
        radius = 200.0f;
    }

    uint32_t groupCount = loaded_surface_iter_group_count();
    for( int i = 0; i < groupCount; ++i ) {
    uint32_t surfCount = loaded_surface_iter_group_size( i );
    for( int j = 0; j < surfCount; ++j ) {
        surf = loaded_surface_iter_get_at_index( i, j );

        // libsm64: Weed out surfaces whose triangles are actually line segs. TODO do this at surface load time
        if( !surf->isValid ) continue;

        // Do the check normally done in add_surface_to_cell
        if( surf->normal.y < -0.01f || surf->normal.y > 0.01f ) continue;

        if( surf->normal.x < -0.707f || surf->normal.x > 0.707f ) {
            surf->flags |= SURFACE_FLAG_X_PROJECTION;
        }

        // Exclude a large number of walls immediately to optimize.
        if (y < surf->lowerY || y > surf->upperY) {
            continue;
        }

        offset = surf->normal.x * x + surf->normal.y * y + surf->normal.z * z + surf->originOffset;

        if (offset < -radius || offset > radius) {
            continue;
        }

        px = x;
        pz = z;

        //! (Quantum Tunneling) Due to issues with the vertices walls choose and
        //  the fact they are floating point, certain floating point positions
        //  along the seam of two walls may collide with neither wall or both walls.
        if (surf->flags & SURFACE_FLAG_X_PROJECTION) {
            w1 = -surf->vertex1[2];            w2 = -surf->vertex2[2];            w3 = -surf->vertex3[2];
            y1 = surf->vertex1[1];            y2 = surf->vertex2[1];            y3 = surf->vertex3[1];

            if (surf->normal.x > 0.0f) {
                if ((y1 - y) * (w2 - w1) - (w1 - -pz) * (y2 - y1) > 0.0f) {
                    continue;
                }
                if ((y2 - y) * (w3 - w2) - (w2 - -pz) * (y3 - y2) > 0.0f) {
                    continue;
                }
                if ((y3 - y) * (w1 - w3) - (w3 - -pz) * (y1 - y3) > 0.0f) {
                    continue;
                }
            } else {
                if ((y1 - y) * (w2 - w1) - (w1 - -pz) * (y2 - y1) < 0.0f) {
                    continue;
                }
                if ((y2 - y) * (w3 - w2) - (w2 - -pz) * (y3 - y2) < 0.0f) {
                    continue;
                }
                if ((y3 - y) * (w1 - w3) - (w3 - -pz) * (y1 - y3) < 0.0f) {
                    continue;
                }
            }
        } else {
            w1 = surf->vertex1[0];            w2 = surf->vertex2[0];            w3 = surf->vertex3[0];
            y1 = surf->vertex1[1];            y2 = surf->vertex2[1];            y3 = surf->vertex3[1];

            if (surf->normal.z > 0.0f) {
                if ((y1 - y) * (w2 - w1) - (w1 - px) * (y2 - y1) > 0.0f) {
                    continue;
                }
                if ((y2 - y) * (w3 - w2) - (w2 - px) * (y3 - y2) > 0.0f) {
                    continue;
                }
                if ((y3 - y) * (w1 - w3) - (w3 - px) * (y1 - y3) > 0.0f) {
                    continue;
                }
            } else {
                if ((y1 - y) * (w2 - w1) - (w1 - px) * (y2 - y1) < 0.0f) {
                    continue;
                }
                if ((y2 - y) * (w3 - w2) - (w2 - px) * (y3 - y2) < 0.0f) {
                    continue;
                }
                if ((y3 - y) * (w1 - w3) - (w3 - px) * (y1 - y3) < 0.0f) {
                    continue;
                }
            }
        }

        //! (Wall Overlaps) Because this doesn't update the x and z local variables,
        //  multiple walls can push mario more than is required.
        data->x += surf->normal.x * (radius - offset);
        data->z += surf->normal.z * (radius - offset);

        //! (Unreferenced Walls) Since this only returns the first four walls,
        //  this can lead to wall interaction being missed. Typically unreferenced walls
        //  come from only using one wall, however.
        if (data->numWalls < 4) {
            data->walls[data->numWalls++] = surf;
        }

        numCols++;
    }}

    return numCols;
}

s32 f32_find_wall_collision(f32 *xPtr, f32 *yPtr, f32 *zPtr, f32 offsetY, f32 radius)
{
    struct SM64WallCollisionData collision;
    s32 numCollisions = 0;

    collision.offsetY = offsetY;
    collision.radius = radius;

    collision.x = *xPtr;
    collision.y = *yPtr;
    collision.z = *zPtr;

    collision.numWalls = 0;

    numCollisions = find_wall_collisions(&collision);

    *xPtr = collision.x;
    *yPtr = collision.y;
    *zPtr = collision.z;

    return numCollisions;
}

s32 find_wall_collisions(struct SM64WallCollisionData *colData)
{
    s32 numCollisions = 0;
    colData->numWalls = 0;

    // libsm64: Don't care about level boundaries with 32-bit ints for vertex positions
    // s16 x = colData->x;
    // s16 z = colData->z;
    // if (x <= -LEVEL_BOUNDARY_MAX || x >= LEVEL_BOUNDARY_MAX) {
    //     return numCollisions;
    // }
    // if (z <= -LEVEL_BOUNDARY_MAX || z >= LEVEL_BOUNDARY_MAX) {
    //     return numCollisions;
    // }

    numCollisions += find_wall_collisions_from_list(colData);
    return numCollisions;
}

f32 find_ceil(f32 posX, f32 posY, f32 posZ, struct SM64SurfaceCollisionData **pceil)
{
    f32 height = CELL_HEIGHT_LIMIT;
	*pceil = find_ceil_from_list( posX, posY, posZ, &height );
	return height;
}

struct SM64FloorCollisionData sFloorGeo;

f32 find_floor_height_and_data(f32 xPos, f32 yPos, f32 zPos, struct SM64FloorCollisionData **floorGeo)
{
    struct SM64SurfaceCollisionData *floor;
    f32 floorHeight = find_floor(xPos, yPos, zPos, &floor);

    *floorGeo = NULL;

    if (floor != NULL) {
        sFloorGeo.normalX = floor->normal.x;
        sFloorGeo.normalY = floor->normal.y;
        sFloorGeo.normalZ = floor->normal.z;
        sFloorGeo.originOffset = floor->originOffset;

        *floorGeo = &sFloorGeo;
    }
    return floorHeight;
}

f32 find_floor_height(f32 x, f32 y, f32 z)
{
    f32 height = FLOOR_LOWER_LIMIT;
	find_floor_from_list( x, y, z, &height );
	return height;
}

f32 find_floor(f32 xPos, f32 yPos, f32 zPos, struct SM64SurfaceCollisionData **pfloor)
{
    f32 height = FLOOR_LOWER_LIMIT;
	*pfloor = find_floor_from_list( xPos, yPos, zPos, &height );
	return height;
}

f32 find_water_level(f32 x, f32 z)
{
	return -10000.0f;
}

f32 find_poison_gas_level(f32 x, f32 z)
{
	return -10000.0f;
}
