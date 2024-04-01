#pragma once

#include "../include/types.h"

/* activeFlags */
#define ACTIVE_FLAG_DEACTIVATED            0         // 0x0000
#define ACTIVE_FLAG_ACTIVE                 (1 <<  0) // 0x0001
#define ACTIVE_FLAG_FAR_AWAY               (1 <<  1) // 0x0002
#define ACTIVE_FLAG_UNK2                   (1 <<  2) // 0x0004
#define ACTIVE_FLAG_IN_DIFFERENT_ROOM      (1 <<  3) // 0x0008
#define ACTIVE_FLAG_UNIMPORTANT            (1 <<  4) // 0x0010
#define ACTIVE_FLAG_INITIATED_TIME_STOP    (1 <<  5) // 0x0020
#define ACTIVE_FLAG_MOVE_THROUGH_GRATE     (1 <<  6) // 0x0040
#define ACTIVE_FLAG_DITHERED_ALPHA         (1 <<  7) // 0x0080
#define ACTIVE_FLAG_UNK8                   (1 <<  8) // 0x0100
#define ACTIVE_FLAG_UNK9                   (1 <<  9) // 0x0200
#define ACTIVE_FLAG_UNK10                  (1 << 10) // 0x0400

// The discriminant for different types of geo nodes
#define GRAPH_NODE_TYPE_ROOT                  0x001
#define GRAPH_NODE_TYPE_ORTHO_PROJECTION      0x002
#define GRAPH_NODE_TYPE_PERSPECTIVE          (0x003 | GRAPH_NODE_TYPE_FUNCTIONAL)
#define GRAPH_NODE_TYPE_MASTER_LIST           0x004
#define GRAPH_NODE_TYPE_START                 0x00A
#define GRAPH_NODE_TYPE_LEVEL_OF_DETAIL       0x00B
#define GRAPH_NODE_TYPE_SWITCH_CASE          (0x00C | GRAPH_NODE_TYPE_FUNCTIONAL)
#define GRAPH_NODE_TYPE_CAMERA               (0x014 | GRAPH_NODE_TYPE_FUNCTIONAL)
#define GRAPH_NODE_TYPE_TRANSLATION_ROTATION  0x015
#define GRAPH_NODE_TYPE_TRANSLATION           0x016
#define GRAPH_NODE_TYPE_ROTATION              0x017
#define GRAPH_NODE_TYPE_OBJECT                0x018
#define GRAPH_NODE_TYPE_ANIMATED_PART         0x019
#define GRAPH_NODE_TYPE_BILLBOARD             0x01A
#define GRAPH_NODE_TYPE_DISPLAY_LIST          0x01B
#define GRAPH_NODE_TYPE_SCALE                 0x01C
#define GRAPH_NODE_TYPE_SHADOW                0x028
#define GRAPH_NODE_TYPE_OBJECT_PARENT         0x029
#define GRAPH_NODE_TYPE_GENERATED_LIST       (0x02A | GRAPH_NODE_TYPE_FUNCTIONAL)
#define GRAPH_NODE_TYPE_BACKGROUND           (0x02C | GRAPH_NODE_TYPE_FUNCTIONAL)
#define GRAPH_NODE_TYPE_HELD_OBJ             (0x02E | GRAPH_NODE_TYPE_FUNCTIONAL)
#define GRAPH_NODE_TYPE_CULLING_RADIUS        0x02F

/* respawnInfoType */
#define RESPAWN_INFO_TYPE_NULL 0
#define RESPAWN_INFO_TYPE_32   1
#define RESPAWN_INFO_TYPE_16   2

#define GRAPH_RENDER_ACTIVE         (1 << 0)
#define GRAPH_RENDER_CHILDREN_FIRST (1 << 1)
#define GRAPH_RENDER_BILLBOARD      (1 << 2)
#define GRAPH_RENDER_Z_BUFFER       (1 << 3)
#define GRAPH_RENDER_INVISIBLE      (1 << 4)
#define GRAPH_RENDER_HAS_ANIMATION  (1 << 5)

struct Object *hack_allocate_mario(void);
void bhv_mario_update(void);
void create_transformation_from_matrices(Mat4 a0, Mat4 a1, Mat4 a2);
void obj_update_pos_from_parent_transformation(Mat4 a0, struct Object *a1);
void obj_set_gfx_pos_from_pos(struct Object *obj);