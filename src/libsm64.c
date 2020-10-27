#define SM64_LIB_EXPORT
#include "libsm64.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "decomp/include/PR/os_cont.h"
#include "decomp/engine/math_util.h"
#include "decomp/include/sm64.h"
#include "decomp/shim.h"
#include "decomp/memory.h"
#include "decomp/global_state.h"
#include "decomp/game/mario.h"
#include "decomp/game/object_stuff.h"
#include "decomp/engine/surface_collision.h"
#include "decomp/engine/graph_node.h"
#include "decomp/engine/geo_layout.h"
#include "decomp/game/rendering_graph_node.h"
#include "decomp/mario/geo.inc.h"
#include "decomp/game/platform_displacement.h"

#include "debug_print.h"
#include "load_surfaces.h"
#include "gfx_adapter.h"
#include "load_anim_data.h"
#include "load_tex_data.h"

static struct AllocOnlyPool *s_mario_geo_pool;
static struct GraphNode *s_mario_graph_node;
static uint32_t s_last_colors_hash;

static struct GlobalState *s_global_state;

static void update_button( bool on, u16 button )
{
    gController.buttonPressed &= ~button;

    if( on )
    {
        if(( gController.buttonDown & button ) == 0 )
            gController.buttonPressed |= button;

        gController.buttonDown |= button;
    }
    else 
    {
        gController.buttonDown &= ~button;
    }
}

static struct Area *allocate_area( void )
{
    struct Area *result = malloc( sizeof( struct Area ));
    memset( result, 0, sizeof( struct Area ));

    result->flags = 1;
    result->camera = malloc( sizeof( struct Camera ));
    memset( result->camera, 0, sizeof( struct Camera ));

    return result;
}

static void free_area( struct Area *area )
{
    free( area->camera );
    free( area );
}

SM64_LIB_FN void sm64_global_init( uint8_t *rom, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction )
{
    s_last_colors_hash = 0;
    g_debug_print_func = debugPrintFunction;

    load_mario_textures_from_rom( rom, outTexture );
    load_mario_anims_from_rom( rom );

    memory_init();
    s_global_state = global_state_create();
    global_state_bind( s_global_state );

    gCurrSaveFileNum = 1;
    gMarioObject = hack_allocate_mario();
    gCurrentArea = allocate_area();
    gCurrentObject = gMarioObject;

    s_mario_geo_pool = alloc_only_pool_init();
    s_mario_graph_node = process_geo_layout( s_mario_geo_pool, mario_geo_ptr );

    D_80339D10.animDmaTable = NULL;
    D_80339D10.currentAnimAddr = NULL;
    D_80339D10.targetAnim = NULL;
}

SM64_LIB_FN void sm64_load_surfaces( uint16_t terrainType, const struct SM64Surface *surfaceArray, uint32_t numSurfaces )
{
    surfaces_load_static_libsm64( surfaceArray, numSurfaces );
    gCurrentArea->terrainType = terrainType;
}

SM64_LIB_FN void sm64_mario_reset( int16_t marioX, int16_t marioY, int16_t marioZ )
{
    gMarioSpawnInfoVal.startPos[0] = marioX;
    gMarioSpawnInfoVal.startPos[1] = marioY;
    gMarioSpawnInfoVal.startPos[2] = marioZ;

    gMarioSpawnInfoVal.startAngle[0] = 0;
    gMarioSpawnInfoVal.startAngle[1] = 0;
    gMarioSpawnInfoVal.startAngle[2] = 0;

    gMarioSpawnInfoVal.areaIndex = 0;
    gMarioSpawnInfoVal.activeAreaIndex = 0;
    gMarioSpawnInfoVal.behaviorArg = 0;
    gMarioSpawnInfoVal.behaviorScript = NULL;
    gMarioSpawnInfoVal.unk18 = NULL;
    gMarioSpawnInfoVal.next = NULL;

    init_mario_from_save_file();
    init_mario();
    set_mario_action( gMarioState, ACT_SPAWN_SPIN_AIRBORNE, 0);
    find_floor( marioX, marioY, marioZ, &gMarioState->floor );
}

static void update_terrain_objects( void )
{
    update_dynamic_surface_list(); 
}

static void update_non_terrain_objects( void )
{
    bhv_mario_update();
}

static void update_objects( void )
{
    //clear_dynamic_surfaces();
    update_terrain_objects();
    apply_mario_platform_displacement();
    //detect_object_collisions();
    update_non_terrain_objects();
    update_mario_platform();
}

SM64_LIB_FN void sm64_mario_tick( const struct SM64MarioInputs *inputs, struct SM64MarioState *outState, struct SM64MarioGeometryBuffers *outBuffers )
{
    update_button( inputs->buttonA, A_BUTTON );
    update_button( inputs->buttonB, B_BUTTON );
    update_button( inputs->buttonZ, Z_TRIG );

    gMarioState->area->camera->yaw = atan2s( inputs->camLookZ, inputs->camLookX );

    gController.stickX = -64.0f * inputs->stickX;
    gController.stickY = 64.0f * inputs->stickY;
    gController.stickMag = sqrtf( gController.stickX*gController.stickX + gController.stickY*gController.stickY );

    update_objects();

    gfx_adapter_bind_output_buffers( outBuffers );

    geo_process_root_hack_single_node( s_mario_graph_node );

    gAreaUpdateCounter++;

    outState->health = gMarioState->health;
    vec3f_copy( outState->position, gMarioState->pos );
    vec3f_copy( outState->velocity, gMarioState->vel );
    outState->faceAngle = (float)gMarioState->faceAngle[1] / 32768.0f * 3.14159f;
}

SM64_LIB_FN void sm64_global_terminate( void )
{
    free( gMarioObject );
    free_area( gCurrentArea );

    global_state_bind( NULL );
    global_state_destroy( s_global_state );
    s_global_state = NULL;

    surfaces_unload_all();
    unload_mario_anims();
    memory_terminate();
}

SM64_LIB_FN uint32_t sm64_load_surface_object( const struct SM64SurfaceObject *surfaceObject )
{
    return surfaces_load_object( surfaceObject );
}

SM64_LIB_FN void sm64_move_object( uint32_t id, const struct SM64ObjectTransform *transform )
{
    surface_object_update_transform( id, transform );
}

SM64_LIB_FN void sm64_unload_object( uint32_t id )
{
    surfaces_unload_object( id );
}
