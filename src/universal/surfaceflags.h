#pragma once

#define SURF_INVALID -1
#define SURF_NONE           0x00000000
#define SURF_NODAMAGE       0x00000001
#define SURF_SLICK          0x00000002
#define SURF_SKY            0x00000004
#define SURF_LADDER         0x00000008
#define SURF_NOIMPACT       0x00000010
#define SURF_NOMARKS        0x00000020
#define SURF_HDRPORTAL      0x00000040
#define SURF_NODRAW         0x00000080
#define SURF_NOPENETRATE    0x00000100
#define SURF_NOLIGHTMAP     0x00000400
#define SURF_NOSTEPS        0x00002000
#define SURF_NONSOLID       0x00004000
#define SURF_NODLIGHT       0x00020000
#define SURF_NOCASTSHADOW   0x00040000
#define SURF_MANTLEON       0x02000000
#define SURF_MANTLEOVER     0x04000000
#define SURF_PORTAL         ((int)0x80000000u)

#define SURF_TYPE_SHIFT 20
#define SURF_TYPE_MASK  0x01F00000
#define SURF_TYPECOUNT  29

#define SURF_TYPE_DEFAULT       0x00000000
#define SURF_TYPE_BARK          0x00100000
#define SURF_TYPE_BRICK         0x00200000
#define SURF_TYPE_CARPET        0x00300000
#define SURF_TYPE_CLOTH         0x00400000
#define SURF_TYPE_CONCRETE      0x00500000
#define SURF_TYPE_DIRT          0x00600000
#define SURF_TYPE_FLESH         0x00700000
#define SURF_TYPE_FOLIAGE       0x00800000
#define SURF_TYPE_GLASS         0x00900000
#define SURF_TYPE_GRASS         0x00A00000
#define SURF_TYPE_GRAVEL        0x00B00000
#define SURF_TYPE_ICE           0x00C00000
#define SURF_TYPE_METAL         0x00D00000
#define SURF_TYPE_MUD           0x00E00000
#define SURF_TYPE_PAPER         0x00F00000
#define SURF_TYPE_PLASTER       0x01000000
#define SURF_TYPE_ROCK          0x01100000
#define SURF_TYPE_SAND          0x01200000
#define SURF_TYPE_SNOW          0x01300000
#define SURF_TYPE_WATER         0x01400000
#define SURF_TYPE_WOOD          0x01500000
#define SURF_TYPE_ASPHALT       0x01600000
#define SURF_TYPE_CERAMIC       0x01700000
#define SURF_TYPE_PLASTIC       0x01800000
#define SURF_TYPE_RUBBER        0x01900000
#define SURF_TYPE_CUSHION       0x01A00000
#define SURF_TYPE_FRUIT         0x01B00000
#define SURF_TYPE_PAINTEDMETAL  0x01C00000
#define SURF_TYPE_OPAQUEGLASS   SURF_TYPE_GLASS

#define SURF_TYPEINDEX(flags) \
    (((flags) & SURF_TYPE_MASK) >> SURF_TYPE_SHIFT)

struct infoParm_t // sizeof=0x14
{
    const char *name;
    int clearSolid;
    int surfaceFlags;
    int contents;
    int toolFlags;
};

extern const infoParm_t infoParms[60];

const char *__cdecl Com_SurfaceTypeToName(int iTypeIndex);
int __cdecl Com_SurfaceTypeFromName(const char *name);
