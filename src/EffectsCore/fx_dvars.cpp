#include "fx_system.h"

const dvar_t *fx_mark_profile;
const dvar_t *fx_marks_smodels;
const dvar_t *fx_enable ;
const dvar_t *fx_cull_effect_spawn;
const dvar_t *fx_cull_elem_draw;
const dvar_t *fx_freeze ;
const dvar_t *fx_debugBolt;
const dvar_t *fx_count  ;
const dvar_t *fx_visMinTraceDist;
const dvar_t *fx_drawClouds;
const dvar_t *fx_marks  ;
const dvar_t *fx_draw   ;
const dvar_t *fx_profile;
const dvar_t *fx_cull_elem_spawn;
const dvar_t *fx_marks_ents;

void __cdecl FX_RegisterDvars(void)
{
    DvarLimits min; // [esp+4h] [ebp-10h]
    DvarLimits mina; // [esp+4h] [ebp-10h]

    fx_enable = Dvar_RegisterBool("fx_enable", true, DVAR_CHEAT, "Toggles all effects processing");
    fx_draw = Dvar_RegisterBool("fx_draw", true, DVAR_CHEAT, "Toggles drawing of effects after processing");
    fx_cull_elem_spawn = Dvar_RegisterBool("fx_cull_elem_spawn", true, DVAR_NOFLAG, "Culls effect elems for spawning");
    fx_cull_elem_draw = Dvar_RegisterBool("fx_cull_elem_draw", true, DVAR_NOFLAG, "Culls effect elems for drawing");
    fx_cull_effect_spawn = Dvar_RegisterBool("fx_cull_effect_spawn", false, DVAR_NOFLAG, "Culls entire effects for spawning");
    fx_marks = Dvar_RegisterBool("fx_marks", true, DVAR_ARCHIVE, "Toggles whether bullet hits leave marks");
    fx_marks_smodels = Dvar_RegisterBool(
        "fx_marks_smodels",
        true,
        DVAR_ARCHIVE,
        "Toggles whether bullet hits leave marks on static models");
	fx_marks_ents = Dvar_RegisterBool("fx_marks_ents", true, DVAR_ARCHIVE, "Toggles whether bullet hits leave marks on entities");
    fx_freeze = Dvar_RegisterBool("fx_freeze", false, DVAR_CHEAT, "Freeze effects");
    min.value.max = 100.0;
    min.value.min = 0.0;
    fx_debugBolt = Dvar_RegisterFloat("fx_debugBolt", 0.0, min, DVAR_CHEAT, "Debug effects bolt");
    fx_count = Dvar_RegisterBool("fx_count", false, DVAR_CHEAT, "Debug effects count");
    mina.value.max = 1000.0;
    mina.value.min = 0.0;
    fx_visMinTraceDist = Dvar_RegisterFloat("fx_visMinTraceDist", 80.0, mina, DVAR_CHEAT, "Minimum visibility trace size");
    fx_profile = Dvar_RegisterInt(
        "fx_profile",
        0,
        (DvarLimits)0x100000000LL,
        DVAR_CHEAT,
        "Turn on FX profiling (specify which local client, with '1' being the first.)");
    fx_mark_profile = Dvar_RegisterInt(
        "fx_mark_profile",
        0,
        (DvarLimits)0x100000000LL,
        DVAR_CHEAT,
        "Turn on FX profiling for marks (specify which local client, with '1' being the first.)");
    fx_drawClouds = Dvar_RegisterBool("fx_drawClouds", true, DVAR_CHEAT, "Toggles the drawing of particle clouds");
}