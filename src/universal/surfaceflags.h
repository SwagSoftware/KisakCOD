#pragma once

struct infoParm_t // sizeof=0x14
{
    const char *name;
    int clearSolid;
    int surfaceFlags;
    int contents;
    int toolFlags;
};

const infoParm_t infoParms[60] =
{
    { "bark", 0, 0x100000, 0, 0},
    { "brick", 0, 0x200000, 0, 0},
    { "carpet", 0, 0x300000, 0, 0},
    { "cloth", 0, 0x400000, 0, 0},
    { "concrete", 0, 0x500000, 0, 0},
    { "dirt" , 0, 0x600000, 0, 0},
    { "flesh", 0, 0x700000, 0, 0},
    { "foilage", 1, 0x800000, 2, 0},
    { "glass", 1, 0x900000, 0x10, 0},
    { "grass", 0, 0x0A00000, 0, 0},
    { "gravel", 0, 0x0B00000, 0, 0},
    { "ice", 0, 0x0C00000, 0, 0},
    { "metal", 0, 0x0D00000, 0, 0},
    { "mud", 0, 0x0E00000, 0, 0},
    { "paper", 0, 0x0F00000, 0, 0},
    { "plaster", 0, 0x1000000, 0, 0},
    { "rock", 0, 0x1100000, 0, 0},
    { "sand", 0, 0x1200000, 0, 0},
    { "snow", 0, 0x1300000, 0, 0},
    { "water", 1, 0x1400000, 0x20, 0},
    { "wood", 0, 0x1500000, 0, 0},
    { "asphalt", 0, 0x1600000, 0, 0},
    { "ceramic", 0, 0x1700000, 0, 0},
    { "plastic", 0, 0x1800000, 0, 0},
    { "rubber", 0, 0x1900000, 0, 0},
    { "cushion", 0, 0x1A00000, 0, 0},
    { "fruit", 0, 0x1B00000, 0, 0},
    { "paintedmetal", 0, 0x1C00000, 0, 0},
    { "opaqueglass", 0, 0x900000, 0, 0},
    { "clipmissile", 1, 0, 0x80, 0},
    { "ai_nosight", 1, 0, 0x1000, 0},
    { "clipshot", 1, 0, 0x2000, 0},
    { "playerclip", 1, 0, 0x10000, 0},
    { "monsterclip", 1, 0, 0x20000, 0},
    { "vehicleclip", 1, 0, 0x200, 0},
    { "itemclip", 1, 0, 0x400, 0},
    { "nodrop", 1, 0, (int)0x80000000, 0},
    { "nonsolid", 1, 0x4000, 0, 0},
    { "detail", 0, 0, 0x8000000, 0},
    { "structural", 0, 0, 0x10000000, 0},
    { "portal", 1, (int)0x80000000, 0, 0},
    { "canshootclip", 0, 0, 0x40, 0},
    { "origin", 1, 0, 0, 4},
    { "sky", 0, 4, 0x800, 0},
    { "nocastshadow", 0, 0x40000, 0, 0},
    { "physicsGeom", 0, 0, 0, 0x400},
    { "lightPortal", 0, 0, 0, 0x2000},
    { "slick", 0, 2, 0, 0},
    { "noimpact", 0, 0x10, 0, 0},
    { "nomarks", 0, 0x20, 0, 0},
    { "nopenetrate", 0, 0x100, 0, 0},
    { "ladder", 0, 8, 0, 0},
    { "nodamage", 0, 1, 0, 0},
    { "mantleOn", 0, 0x2000000, 0x1000000, 0},
    { "mantleOver", 0, 0x4000000, 0x1000000, 0},
    { "nosteps", 0, 0x2000, 0, 0},
    { "nodraw", 0, 0x80, 0, 0},
    { "nolightmap", 0, 0x400, 0, 0},
    { "nodlight", 0, 0x20000, 0, 0},
};

const char *__cdecl Com_SurfaceTypeToName(int iTypeIndex);
int __cdecl Com_SurfaceTypeFromName(const char *name);