#include "r_cmds.h"
#include <qcommon/cmd.h>

#include "r_screenshot.h"
#include "r_sky.h"
#include "r_dvars.h"
#include "r_init.h"
#include "r_rendercmds.h"
#include "r_image.h"
#include "r_model.h"
#include <universal/com_memory.h>
#include "r_staticmodel.h"
#include <xanim/xmodel.h>
#include "r_xsurface.h"
#include "rb_stats.h"
#include <DynEntity/DynEntity_client.h>
#include <database/database.h>
#include "r_staticmodelcache.h"

#include <algorithm>

void __cdecl R_Cmd_ScreenshotJpeg()
{
	R_ScreenshotCommand(R_SCREENSHOT_JPG);
}
void __cdecl R_Cmd_Screenshot()
{
	R_ScreenshotCommand(R_SCREENSHOT_TGA);
}

void __cdecl R_Cmd_ApplyPicmip()
{
    Material_UpdatePicmipAll();
}

cmd_function_s R_Cmd_Screenshot_VAR;
cmd_function_s R_Cmd_ScreenshotJpeg_VAR;
cmd_function_s R_ImageList_f_VAR;
cmd_function_s R_Cmd_ApplyPicmip_VAR;
cmd_function_s R_Cmd_ReloadMaterialTextures_VAR;
cmd_function_s R_Cmd_LoadSun_VAR;
cmd_function_s R_Cmd_SaveSun_VAR;
cmd_function_s R_MaterialList_f_VAR;
cmd_function_s R_ModelList_f_VAR;
cmd_function_s RB_Stats_f_VAR;
cmd_function_s R_StaticModelCacheStats_f_VAR;
cmd_function_s R_StaticModelCacheFlush_f_VAR;

void __cdecl R_RegisterCmds()
{
	Cmd_AddCommandInternal("screenshot", R_Cmd_Screenshot, &R_Cmd_Screenshot_VAR);
	Cmd_AddCommandInternal("screenshotJpeg", R_Cmd_ScreenshotJpeg, &R_Cmd_ScreenshotJpeg_VAR);
	Cmd_AddCommandInternal("imagelist", R_ImageList_f, &R_ImageList_f_VAR);
	Cmd_AddCommandInternal("r_applyPicmip", R_Cmd_ApplyPicmip, &R_Cmd_ApplyPicmip_VAR);
	Cmd_AddCommandInternal("reloadmaterialtextures", R_Cmd_ReloadMaterialTextures, &R_Cmd_ReloadMaterialTextures_VAR);
	Cmd_AddCommandInternal("r_loadsun", R_Cmd_LoadSun, &R_Cmd_LoadSun_VAR);
	Cmd_AddCommandInternal("r_savesun", R_Cmd_SaveSun, &R_Cmd_SaveSun_VAR);
	Cmd_AddCommandInternal("gfx_world", R_MaterialList_f, &R_MaterialList_f_VAR);
	Cmd_AddCommandInternal("gfx_model", R_ModelList_f, &R_ModelList_f_VAR);
	Cmd_AddCommandInternal("gfx_stats", RB_Stats_f, &RB_Stats_f_VAR);
	Cmd_AddCommandInternal("r_smc_stats", R_StaticModelCacheStats_f, &R_StaticModelCacheStats_f_VAR);
	Cmd_AddCommandInternal("r_smc_flush", R_StaticModelCacheFlush_f, &R_StaticModelCacheFlush_f_VAR);
}

void __cdecl R_UnregisterCmds()
{
	Cmd_RemoveCommand("r_applyPicmip");
	Cmd_RemoveCommand("screenshot");
	Cmd_RemoveCommand("screenshotJpeg");
	Cmd_RemoveCommand("imagelist");
	Cmd_RemoveCommand("reloadmaterialtextures");
	Cmd_RemoveCommand("r_loadsun");
	Cmd_RemoveCommand("r_savesun");
	Cmd_RemoveCommand("gfx_world");
	Cmd_RemoveCommand("gfx_model");
	Cmd_RemoveCommand("gfx_stats");
	Cmd_RemoveCommand("r_smc_stats");
	Cmd_RemoveCommand("r_smc_flush");
}

