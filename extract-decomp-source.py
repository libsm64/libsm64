#!/usr/bin/env python3
import sys
import os
import subprocess
import shutil

geo_inc_c_header = """
#include "../include/sm64.h"
#include "../include/types.h"
#include "../include/geo_commands.h"
#include "../game/rendering_graph_node.h"
#include "../shim.h"
#include "../game/object_stuff.h"
#include "../game/behavior_actions.h"
#include "model.inc.h"

#define SHADOW_CIRCLE_PLAYER 99
"""

geo_inc_c_footer = """
const GeoLayout mario_geo_libsm64[] = {
   GEO_SHADOW(SHADOW_CIRCLE_PLAYER, 0xB4, 100),
   GEO_OPEN_NODE(),
      GEO_ZBUFFER(1),
      GEO_OPEN_NODE(),
         GEO_SCALE(0x00, 16384),
         GEO_OPEN_NODE(),
            GEO_ASM(0, geo_mirror_mario_backface_culling),
            GEO_ASM(0, geo_mirror_mario_set_alpha),
            GEO_BRANCH(1, mario_geo_load_body),
            GEO_ASM(1, geo_mirror_mario_backface_culling),
         GEO_CLOSE_NODE(),
      GEO_CLOSE_NODE(),
   GEO_CLOSE_NODE(),
   GEO_END(),
};

void *mario_geo_ptr = (void*)mario_geo_libsm64;
"""

geo_inc_h = """
#pragma once

extern void *mario_geo_ptr;
"""

anim_data_h = """
#pragma once

extern void *mario_anims_ptr;
"""

anim_data_c_footer = """
void *mario_anims_ptr = &gMarioAnims;
"""

def main():
    os.chdir(os.path.dirname(sys.argv[0]))
    shutil.rmtree("src/mario", ignore_errors=True)
    os.makedirs("src/mario", exist_ok=True)

    geo_inc_c = ""
    model_inc_c = ""
    model_inc_h_lines = [
        "#pragma once",
        '#include "../include/types.h"',
        '#include "../include/PR/gbi.h"'
    ]

    with open("sm64-port/actors/mario/geo.inc.c", "r") as file:
        geo_inc_c = file.read()

    with open("sm64-port/actors/mario/model.inc.c", "r") as file:
        lines = file.read().splitlines()

        for line in lines:
            if line.startswith("const "):
                model_inc_h_lines.append("extern " + line.replace(" = {", ";"))

        lines = [ x.replace("#include", "//#include") for x in lines ]
        lines.insert(0, "#include \"../model_hack.h\"")
        model_inc_c = "\n".join(lines)

    with open("src/mario/geo.inc.c", "w") as file:
        file.write(geo_inc_c_header + geo_inc_c + geo_inc_c_footer)

    with open("src/mario/model.inc.c", "w") as file:
        file.write(model_inc_c)

    with open("src/mario/model.inc.h", "w") as file:
        file.write("\n".join(model_inc_h_lines))

    with open("src/mario/geo.inc.h", "w") as file:
        file.write(geo_inc_h)

main()