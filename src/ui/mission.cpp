#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include <qcommon/qcommon.h>

const dvar_t *mis_cheat;
const dvar_t *mis_01;
const dvar_t *mis_difficulty;

void Campaign_UnlockAll()
{
	Dvar_SetBool(mis_cheat, 1);
}

void Campaign_RegisterDvars()
{
    mis_01 = Dvar_RegisterInt("mis_01", 0, 0, 50, 0, "Mission 1");
    mis_difficulty = Dvar_RegisterString(
        "mis_difficulty",
        "00000000000000000000000000000000000000000000000000",
        0,
        "Mission difficulty");
    mis_cheat = Dvar_RegisterBool("mis_cheat", 0, 0, "Set when level unlock cheat is performed");
}