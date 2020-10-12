#pragma once

enum GFXAdapterCommands
{
    GFXCMD_None = 0,
    GFXCMD_VertexData,
    GFXCMD_Triangle,
    GFXCMD_Light,
    GFXCMD_Texture,
    GFXCMD_SetTileSize,
    GFXCMD_SetTextureImage,
    GFXCMD_SubDisplayList,
    GFXCMD_EndDisplayList,
};