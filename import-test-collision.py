#!/usr/bin/env python3
import os
import shutil
import urllib.request

BOB_COLLISION_URL = "https://raw.githubusercontent.com/n64decomp/sm64/06ec56df7f951f88da05f468cdcacecba496145a/levels/bob/areas/1/collision.inc.c"

LEVEL_H = """#pragma once

#include "../src/libsm64.h"

extern const struct SM64Surface surfaces[];
extern const size_t surfaces_count;
"""

def main():
    print("Downloading " + BOB_COLLISION_URL)
    in_lines = urllib.request.urlopen(BOB_COLLISION_URL).read().decode('utf8').splitlines()

    verts = []
    tris = []
    mode = ""

    for line in in_lines:
        if not line.strip().startswith("COL_"):
            continue;

        tokens = line.strip().replace("(", ",").replace(")", "").split(",")

        if tokens[0] == "COL_VERTEX":
            verts.append([ int(tokens[1]), int(tokens[2]), int(tokens[3]) ])
        elif tokens[0] == "COL_TRI_INIT":
            mode = tokens[1]
        elif tokens[0] == "COL_TRI":
            tris.append([ int(tokens[1]), int(tokens[2]), int(tokens[3]), mode ])

    out_lines = []

    for tri in tris:
        out_lines.append("{%s,0,TERRAIN_SNOW,{{%s,%s,%s},{%s,%s,%s},{%s,%s,%s}}}"%(tri[3], \
            verts[tri[0]][0], verts[tri[0]][1], verts[tri[0]][2], \
            verts[tri[1]][0], verts[tri[1]][1], verts[tri[1]][2], \
            verts[tri[2]][0], verts[tri[2]][1], verts[tri[2]][2]))

    out_str = ",\n".join(out_lines)
    out_str = "const struct SM64Surface surfaces[] = {\n" + out_str + "};\n\n"
    out_str += "const size_t surfaces_count = sizeof( surfaces ) / sizeof( surfaces[0] );"
    out_str = '#include "../src/decomp/include/surface_terrains.h"\n' + out_str
    out_str = '#include "level.h"\n' + out_str

    with open("test/level.c", "w") as file:
        file.write(out_str)

    with open("test/level.h", "w") as file:
        file.write(LEVEL_H)

if __name__ == "__main__":
    main()
