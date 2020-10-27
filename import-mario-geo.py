#!/usr/bin/env python3
import os
import shutil
import urllib.request

GEO_URL = "https://raw.githubusercontent.com/n64decomp/sm64/06ec56df7f951f88da05f468cdcacecba496145a/actors/mario/geo.inc.c"
MODEL_URL = "https://raw.githubusercontent.com/n64decomp/sm64/06ec56df7f951f88da05f468cdcacecba496145a/actors/mario/model.inc.c"

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

model_inc_h = """
#pragma once

#include "../include/types.h"
#include "../include/PR/gbi.h"
"""

def main():
    global model_inc_h

    shutil.rmtree("src/decomp/mario", ignore_errors=True)
    os.makedirs("src/decomp/mario", exist_ok=True)

    print("Downloading " + GEO_URL)
    geo_inc_c = urllib.request.urlopen(GEO_URL).read().decode('utf8')
    print("Downloading " + MODEL_URL)
    model_inc_c = urllib.request.urlopen(MODEL_URL).read().decode('utf8')

    lines = model_inc_c.splitlines()

    skip = 0
    for i in range(len(lines)):
        if skip > 0:
            skip = skip - 1
            lines[i] = "//" + lines[i]
        elif lines[i].startswith("ALIGNED8 static const u8 mario_"):
            skip = 2
            lines[i] = "//" + lines[i]
        elif lines[i].startswith("const "):
            model_inc_h += "\nextern " + lines[i].replace(" = {", ";")

    lines.insert(0, "#include \"../../gfx_macros.h\"")
    lines.insert(0, "#include \"../../load_tex_data.h\"")
    model_inc_c = "\n".join(lines)


    with open("src/decomp/mario/geo.inc.c", "w") as file:
        file.write(geo_inc_c_header + geo_inc_c + geo_inc_c_footer)

    with open("src/decomp/mario/model.inc.c", "w") as file:
        file.write(model_inc_c)

    with open("src/decomp/mario/model.inc.h", "w") as file:
        file.write(model_inc_h)

    with open("src/decomp/mario/geo.inc.h", "w") as file:
        file.write(geo_inc_h)

if __name__ == "__main__":
    main()
