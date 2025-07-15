#pragma once

#include <game/pathnode.h>

struct GameWorldSp // sizeof=0x2C
{
    const char *name;
    PathData path;
};
struct GameWorldMp // sizeof=0x4
{                                       // ...
    const char *name;
};

#ifdef KISAK_MP
extern GameWorldMp gameWorldMp;
#elif KISAK_SP
extern GameWorldSp gameWorldSp;
#endif