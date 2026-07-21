#include <universal/q_shared.h>
#include "g_bsp.h"

#include "game_public.h"

#ifdef KISAK_MP
GameWorldMp gameWorldMp;
#endif

#ifdef KISAK_SP
GameWorldSp gameWorldSp;
#endif