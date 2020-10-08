#pragma once

#include "../include/types.h"

struct Object *hack_allocate_mario(void);
void bhv_mario_update(void);
void create_transformation_from_matrices(Mat4 a0, Mat4 a1, Mat4 a2);
void obj_update_pos_from_parent_transformation(Mat4 a0, struct Object *a1);
void obj_set_gfx_pos_from_pos(struct Object *obj);