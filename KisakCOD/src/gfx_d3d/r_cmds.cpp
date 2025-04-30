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

void __cdecl R_ImageList_f()
{
    const char *v0; // eax
    _D3DFORMAT v1; // eax
    const char *v2; // eax
    const char *fmt; // [esp+Ch] [ebp-2110h]
    GfxImage *image; // [esp+A0h] [ebp-207Ch]
    int v5; // [esp+A4h] [ebp-2078h]
    bool v6; // [esp+ABh] [ebp-2071h]
    unsigned __int8 dst[80]; // [esp+ACh] [ebp-2070h] BYREF
    _DWORD v8[2]; // [esp+FCh] [ebp-2020h]
    _D3DFORMAT v9; // [esp+104h] [ebp-2018h]
    unsigned int i; // [esp+108h] [ebp-2014h]
    ImageList imageList; // [esp+10Ch] [ebp-2010h] BYREF
    int j; // [esp+2114h] [ebp-8h]
    float v13; // [esp+2118h] [ebp-4h]

    v6 = 0;
    if (Cmd_Argc() == 2)
    {
        v0 = Cmd_Argv(1);
        v6 = I_stricmp(v0, "all") == 0;
    }
    v8[0] = 0;
    v8[1] = 0;
    memset(dst, 0, sizeof(dst));
    R_GetImageList(&imageList);
    if (v6)
    {
        for (i = 0; i < 0xE && imageList.count < 0x800; ++i)
        {
            if (g_imageProgs[i].mapType)
                imageList.image[imageList.count++] = &g_imageProgs[i];
        }
    }
    //std::_Sort<GfxImage **, int, int(__cdecl *)(GfxImage *, GfxImage *)>(
    //    imageList.image,
    //    &imageList.image[imageList.count],
    //    (signed int)(4 * imageList.count) >> 2,
    //    imagecompare);
    std::sort((GfxImage **)&imageList.image[0], (GfxImage **)&imageList.image[imageList.count], imagecompare);
    Com_Printf(8, "\n-fmt- -dimension-");
    for (j = 0; j < 2; ++j)
        Com_Printf(8, "%s", g_platform_name[j]);
    Com_Printf(8, "  --name-------\n");
    for (i = 0; i < imageList.count; ++i)
    {
        image = imageList.image[i];
        Com_Printf(8, "%4i x %-4i ", image->width, image->height);
        v1 = R_ImagePixelFormat(image);
        v9 = v1;
        if (v1 > D3DFMT_A8L8)
        {
            if (v1 > D3DFMT_DXT3)
            {
                if (v1 == D3DFMT_DXT5)
                {
                    Com_Printf(8, "DXT5  ");
                    goto LABEL_36;
                }
            }
            else
            {
                switch (v1)
                {
                case D3DFMT_DXT3:
                    Com_Printf(8, "DXT3  ");
                    goto LABEL_36;
                case D3DFMT_R32F:
                    Com_Printf(8, "R32F  ");
                    goto LABEL_36;
                case D3DFMT_DXT1:
                    Com_Printf(8, "DXT1  ");
                    goto LABEL_36;
                }
            }
        LABEL_34:
            if (!alwaysfails)
            {
                v2 = va("unhandled case: %d", v9);
                MyAssertHandler(".\\r_image.cpp", 1539, 1, v2);
            }
        }
        else if (v1 == D3DFMT_A8L8)
        {
            Com_Printf(8, "AL16  ");
        }
        else
        {
            switch (v1)
            {
            case D3DFMT_A8R8G8B8:
                Com_Printf(8, "RGBA32");
                break;
            case D3DFMT_X8R8G8B8:
                Com_Printf(8, "RGB32 ");
                break;
            case D3DFMT_A8:
                Com_Printf(8, "A8    ");
                break;
            case D3DFMT_L8:
                Com_Printf(8, "L8    ");
                break;
            default:
                goto LABEL_34;
            }
        }
    LABEL_36:
        Com_Printf(8, "  %s", imageTypeName[image->track]);
        for (j = 0; j < 2; ++j)
        {
            v13 = (double)image->cardMemory.platform[j] / 1024.0;
            if (v13 >= 10.0)
                fmt = "%7.0fk";
            else
                fmt = "%7.1fk";
            Com_Printf(8, fmt, v13);
            v5 = image->cardMemory.platform[j];
            if (!useFastFile->current.enabled)
            {
                *(_DWORD *)&dst[8 * image->track + 4 * j] += v5;
                if (!v6 && Image_IsCodeImage(image->track))
                    continue;
            }
            v8[j] += v5;
        }
        Com_Printf(8, "  %s\n", image->name);
    }
    Com_Printf(8, " ---------\n");
    Com_Printf(8, " %i total images\n", imageList.count);
    for (j = 0; j < 2; ++j)
        Com_Printf(8, " %5.1f MB %s total image size\n", (double)(int)v8[j] / 1048576.0, g_platform_name[j]);
    if (!useFastFile->current.enabled)
    {
        Com_Printf(8, "\n");
        Com_Printf(8, "       ");
        for (j = 0; j < 2; ++j)
            Com_Printf(8, "%s", g_platform_name[j]);
        Com_Printf(8, "\n");
        for (i = 0; i < 0xA; ++i)
        {
            Com_Printf(8, "%s:", imageTypeName[i]);
            for (j = 0; j < 2; ++j)
                Com_Printf(8, "  %5.1f", (double)*(int *)&dst[8 * i + 4 * j] / 1048576.0);
            Com_Printf(8, "  MB\n");
        }
    }
    Com_Printf(8, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

void __cdecl R_Cmd_LoadSun()
{
    const char *v0; // eax
    sunflare_t *p_sun; // [esp-4h] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        if (sv_cheats->current.enabled)
        {
            if (rgp.world)
            {
                p_sun = &rgp.world->sun;
                v0 = Cmd_Argv(1);
                R_LoadSunThroughDvars(v0, p_sun);
            }
            else
            {
                Com_Printf(8, "You can't r_loadsun while a map isn't loaded\n");
            }
        }
        else
        {
            Com_Printf(8, "You must have cheats enabled to use r_loadsun\n");
        }
    }
    else
    {
        Com_Printf(8, "USAGE: r_loadsun <sunname>\n  sunname must not have an extension\n");
    }
}

void __cdecl R_ModelList_f()
{
    const char *Name; // eax
    const char *v1; // [esp+Ch] [ebp-212Ch]
    const char *fmt; // [esp+10h] [ebp-2128h]
    int inData; // [esp+108h] [ebp-2030h] BYREF
    XModel *v4[2050]; // [esp+10Ch] [ebp-202Ch] BYREF
    XModel *model; // [esp+2114h] [ebp-24h]
    int v6; // [esp+2118h] [ebp-20h]
    int v7; // [esp+211Ch] [ebp-1Ch]
    int XModelUsageCount; // [esp+2120h] [ebp-18h]
    int MemoryUsage; // [esp+2124h] [ebp-14h]
    int MemUsage; // [esp+2128h] [ebp-10h]
    int i; // [esp+212Ch] [ebp-Ch]
    float v12; // [esp+2130h] [ebp-8h]
    int modelCount; // [esp+2134h] [ebp-4h] BYREF

    v7 = 0;
    v6 = 0;
    inData = 0;
    DB_EnumXAssets(ASSET_TYPE_XMODEL, (void(__cdecl *)(XAssetHeader, void *))R_GetModelList, &inData, 1);
    //std::_Sort<XModel **, int, bool(__cdecl *)(XModel *&, XModel *&)>(v4, &v4[inData], (4 * inData) >> 2, R_ModelSort);
    std::sort((XModel ***)&v4[0], (XModel ***)&v4[inData], R_ModelSort);
    Com_Printf(8, "---------------------------\n");
    Com_Printf(8, "SM# is the number of static model instances\n");
    Com_Printf(8, "instKB is static model instance usage\n");
    Com_Printf(8, "DE# is the number of dyn entity instances\n");
    Com_Printf(8, "   SM#  instKB   DE#   geoKB  name\n");
    for (i = 0; i < inData; ++i)
    {
        model = v4[i];
        MemUsage = XModelGetMemUsage(model);
        v7 += MemUsage;
        MemoryUsage = R_StaticModelGetMemoryUsage(model, &modelCount);
        if (MemoryUsage)
        {
            v6 += MemoryUsage;
            v12 = (double)MemoryUsage / 1024.0;
            Com_Printf(8, "  %4i  ", modelCount);
            if (v12 >= 10.0)
                fmt = "%6.0f";
            else
                fmt = "%6.1f";
            Com_Printf(8, fmt, v12);
        }
        else
        {
            Com_Printf(8, "              ");
        }
        XModelUsageCount = DynEnt_GetXModelUsageCount(model);
        if (XModelUsageCount)
            Com_Printf(8, "  %4i  ", XModelUsageCount);
        else
            Com_Printf(8, "        ");
        v12 = (double)MemUsage / 1024.0;
        if (v12 >= 10.0)
            v1 = "%6.0f";
        else
            v1 = "%6.1f";
        Com_Printf(8, v1, v12);
        Name = XModelGetName(model);
        Com_Printf(8, "  %s\n", Name);
    }
    Com_Printf(8, "---------------------------\n");
    Com_Printf(8, "current inst total  %4.1f MB\n", (double)v6 / 1048576.0);
    Com_Printf(8, "current geo total   %4.1f MB\n", (double)v7 / 1048576.0);
    Com_Printf(8, "current total       %4.1f MB\n", (double)(v7 + v6) / 1048576.0);
    Com_Printf(8, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

void __cdecl RB_Stats_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax

    if (Cmd_Argc() > 2)
        goto LABEL_2;
    R_SyncRenderThread();
    if (Cmd_Argc() == 2)
    {
        v1 = Cmd_Argv(1);
        if (!I_stricmp(v1, "max"))
        {
            RB_Stats_Display(&backEnd.frameStatsMax);
            return;
        }
        v2 = Cmd_Argv(1);
        if (I_stricmp(v2, "cur"))
        {
        LABEL_2:
            v0 = Cmd_Argv(0);
            Com_Printf(8, "USAGE: %s [cur|max]\n", v0);
            return;
        }
    }
    RB_Stats_Display(&g_frameStatsCur);
}


void __cdecl R_StaticModelCacheStats_f()
{
    unsigned int usedCount; // [esp+24h] [ebp-24h]
    unsigned int lodIter; // [esp+28h] [ebp-20h]
    unsigned int allocCount; // [esp+2Ch] [ebp-1Ch]
    unsigned int surfCount; // [esp+30h] [ebp-18h]
    unsigned int smodelIter; // [esp+34h] [ebp-14h]
    unsigned int lodCount; // [esp+38h] [ebp-10h]
    const GfxStaticModelDrawInst *drawInst; // [esp+3Ch] [ebp-Ch]
    unsigned int surfIter; // [esp+40h] [ebp-8h]
    XSurface *surfs; // [esp+44h] [ebp-4h] BYREF

    if (rgp.world)
    {
        allocCount = 0;
        usedCount = 0;
        for (smodelIter = 0; smodelIter < rgp.world->dpvs.smodelCount; ++smodelIter)
        {
            drawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIter];
            lodCount = XModelGetNumLods(drawInst->model);
            for (lodIter = 0; lodIter < lodCount; ++lodIter)
            {
                if (drawInst->smodelCacheIndex[lodIter])
                {
                    allocCount += 1 << XModelGetLodInfo(drawInst->model, lodIter)->smcAllocBits;
                    surfCount = XModelGetSurfCount(drawInst->model, lodIter);
                    XModelGetSurfaces(drawInst->model, &surfs, lodIter);
                    for (surfIter = 0; surfIter < surfCount; ++surfIter)
                        usedCount += XSurfaceGetNumVerts(&surfs[surfIter]);
                }
            }
        }
        Com_Printf(8, "%.2f%% of cache is currently allocated.\n", (double)allocCount * 100.0 / 262144.0);
        if (allocCount)
            Com_Printf(8, "%.2f%% allocated cache vertices are used.\n", (double)usedCount * 100.0 / (double)allocCount);
    }
}

void __cdecl R_StaticModelCacheFlush_f()
{
    R_SyncRenderThread();
    R_ClearAllStaticModelCacheRefs();
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

