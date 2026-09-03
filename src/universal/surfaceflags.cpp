#include <universal/q_shared.h>
#include "surfaceflags.h"

const infoParm_t infoParms[60] =
{
    { "bark", 0, SURF_TYPE_BARK, 0, 0},
    { "brick", 0, SURF_TYPE_BRICK, 0, 0},
    { "carpet", 0, SURF_TYPE_CARPET, 0, 0},
    { "cloth", 0, SURF_TYPE_CLOTH, 0, 0},
    { "concrete", 0, SURF_TYPE_CONCRETE, 0, 0},
    { "dirt" , 0, SURF_TYPE_DIRT, 0, 0},
    { "flesh", 0, SURF_TYPE_FLESH, 0, 0},
    { "foilage", 1, SURF_TYPE_FOLIAGE, 0x2, 0},
    { "glass", 1, SURF_TYPE_GLASS, 0x10, 0},
    { "grass", 0, SURF_TYPE_GRASS, 0, 0},
    { "gravel", 0, SURF_TYPE_GRAVEL, 0, 0},
    { "ice", 0, SURF_TYPE_ICE, 0, 0},
    { "metal", 0, SURF_TYPE_METAL, 0, 0},
    { "mud", 0, SURF_TYPE_MUD, 0, 0},
    { "paper", 0, SURF_TYPE_PAPER, 0, 0},
    { "plaster", 0, SURF_TYPE_PLASTER, 0, 0},
    { "rock", 0, SURF_TYPE_ROCK, 0, 0},
    { "sand", 0, SURF_TYPE_SAND, 0, 0},
    { "snow", 0, SURF_TYPE_SNOW, 0, 0},
    { "water", 1, SURF_TYPE_WATER, 0x20, 0},
    { "wood", 0, SURF_TYPE_WOOD, 0, 0},
    { "asphalt", 0, SURF_TYPE_ASPHALT, 0, 0},
    { "ceramic", 0, SURF_TYPE_CERAMIC, 0, 0},
    { "plastic", 0, SURF_TYPE_PLASTIC, 0, 0},
    { "rubber", 0, SURF_TYPE_RUBBER, 0, 0},
    { "cushion", 0, SURF_TYPE_CUSHION, 0, 0},
    { "fruit", 0, SURF_TYPE_FRUIT, 0, 0},
    { "paintedmetal", 0, SURF_TYPE_PAINTEDMETAL, 0, 0},
    { "opaqueglass", 0, SURF_TYPE_OPAQUEGLASS, 0, 0},
    { "clipmissile", 1, 0, 0x80, 0},
    { "ai_nosight", 1, 0, 0x1000, 0},
    { "clipshot", 1, 0, 0x2000, 0},
    { "playerclip", 1, 0, 0x10000, 0},
    { "monsterclip", 1, 0, 0x20000, 0},
    { "vehicleclip", 1, 0, 0x200, 0},
    { "itemclip", 1, 0, 0x400, 0},
    { "nodrop", 1, 0, (int)0x80000000u, 0},
    { "nonsolid", 1, SURF_NONSOLID, 0, 0},
    { "detail", 0, 0, 0x8000000, 0},
    { "structural", 0, 0, 0x10000000, 0},
    { "portal", 1, SURF_PORTAL, 0, 0},
    { "canshootclip", 0, 0, 0x40, 0},
    { "origin", 1, 0, 0, 4},
    { "sky", 0, SURF_SKY, 0x800, 0},
    { "nocastshadow", 0, SURF_NOCASTSHADOW, 0, 0},
    { "physicsGeom", 0, 0, 0, 0x400},
    { "lightPortal", 0, 0, 0, 0x2000},
    { "slick", 0, SURF_SLICK, 0, 0},
    { "noimpact", 0, SURF_NOIMPACT, 0, 0},
    { "nomarks", 0, SURF_NOMARKS, 0, 0},
    { "nopenetrate", 0, SURF_NOPENETRATE, 0, 0},
    { "ladder", 0, SURF_LADDER, 0, 0},
    { "nodamage", 0, SURF_NODAMAGE, 0, 0},
    { "mantleOn", 0, SURF_MANTLEON, 0x1000000, 0},
    { "mantleOver", 0, SURF_MANTLEOVER, 0x1000000, 0},
    { "nosteps", 0, SURF_NOSTEPS, 0, 0},
    { "nodraw", 0, SURF_NODRAW, 0, 0},
    { "nolightmap", 0, SURF_NOLIGHTMAP, 0, 0},
    { "nodlight", 0, SURF_NODLIGHT, 0, 0},
};


int __cdecl Com_SurfaceTypeFromName(const char *name)
{
    int i; // [esp+0h] [ebp-4h]

    if (!I_stricmp(name, "default"))
        return 0;

    // LWSS ADD - hacky fix for special devs who spelled 'foilage' wrong
    if (!I_stricmp(name, "foliage"))
    {
        name = "foilage";
    }
    // LWSS END

    for (i = 0; i < 28; ++i)
    {
        if (!I_stricmp(name, infoParms[i].name))
            return SURF_TYPEINDEX(infoParms[i].surfaceFlags);
    }

    return -1;
}


const char *__cdecl Com_SurfaceTypeToName(int iTypeIndex)
{
    if (iTypeIndex <= 0 || iTypeIndex >= 29)
        return "default";
    if (SURF_TYPEINDEX(infoParms[iTypeIndex - 1].surfaceFlags) != iTypeIndex)
        MyAssertHandler(
            ".\\universal\\surfaceflags.cpp",
            139,
            0,
            "%s",
            "SURF_TYPEINDEX( infoParms[iTypeIndex - 1].surfaceFlags ) == iTypeIndex");
    return infoParms[iTypeIndex - 1].name;
}
