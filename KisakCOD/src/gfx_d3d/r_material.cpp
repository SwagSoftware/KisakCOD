#include "r_material.h"
#include "r_init.h"
#include <universal/com_memory.h>
#include <qcommon/mem_track.h>
#include <qcommon/cmd.h>
#include <database/database.h>
#include "rb_logfile.h"
#include "r_utils.h"
#include "r_dvars.h"

MaterialGlobals materialGlobals;

const stream_source_info_t s_streamSourceInfo[16][9];
const stream_dest_info_t s_streamDestInfo[12];
const BuiltInMaterialTable s_builtInMaterials[50];

bool g_generateOverrideTechniques;

void __cdecl TRACK_r_material()
{
    track_static_alloc_internal(&materialGlobals, 4100, "materialGlobals", 18);
    track_static_alloc_internal((void *)s_streamSourceInfo, 432, "s_streamSourceInfo", 18);
    track_static_alloc_internal((void *)s_streamDestInfo, 24, "s_streamDestInfo", 18);
    track_static_alloc_internal((void *)s_builtInMaterials, 400, "s_builtInMaterials", 18);
}

unsigned __int8 *__cdecl Material_Alloc(unsigned int size)
{
    return Hunk_Alloc(size, "Material_Alloc", 22);
}

void __cdecl Load_CreateMaterialPixelShader(GfxPixelShaderLoadDef *loadDef, MaterialPixelShader *mtlShader)
{
    if (loadDef != &mtlShader->prog.loadDef)
        MyAssertHandler(".\\r_material.cpp", 623, 0, "%s", "loadDef == &mtlShader->prog.loadDef");
    if (r_loadForRenderer->current.enabled && loadDef->loadForRenderer == r_rendererInUse->current.integer)
    {
        ProfLoad_Begin("Create pixel shader");
        dx.device->CreatePixelShader(loadDef->program, (IDirect3DPixelShader9 **)&mtlShader->prog);
        ProfLoad_End();
    }
    else
    {
        mtlShader->prog.ps = 0;
    }
}

void __cdecl Load_CreateMaterialVertexShader(GfxVertexShaderLoadDef *loadDef, MaterialVertexShader *mtlShader)
{
    if (loadDef != &mtlShader->prog.loadDef)
        MyAssertHandler(".\\r_material.cpp", 643, 0, "%s", "loadDef == &mtlShader->prog.loadDef");
    if (r_loadForRenderer->current.enabled && loadDef->loadForRenderer == r_rendererInUse->current.integer)
    {
        ProfLoad_Begin("Create vertex shader");
        dx.device->CreateVertexShader(loadDef->program, (IDirect3DVertexShader9 **)&mtlShader->prog);
        ProfLoad_End();
    }
    else
    {
        mtlShader->prog.vs = 0;
    }
}

void __cdecl AssertValidVertexDeclOffsets(const stream_source_info_t *streamTable)
{
    int thisIndex; // [esp+0h] [ebp-10h]
    unsigned int stream; // [esp+4h] [ebp-Ch]
    int lastIndex[2]; // [esp+8h] [ebp-8h]

    lastIndex[0] = -1;
    lastIndex[1] = -1;
    for (thisIndex = 0; thisIndex < 9; ++thisIndex)
    {
        stream = streamTable[thisIndex].Stream;
        if (stream != 255)
        {
            if (stream >= 2)
                MyAssertHandler(
                    ".\\r_material.cpp",
                    686,
                    0,
                    "%s\n\t(stream) = %i",
                    "(stream >= 0 && stream < (sizeof( lastIndex ) / (sizeof( lastIndex[0] ) * (sizeof( lastIndex ) != 4 || sizeof("
                    " lastIndex[0] ) <= 4))))",
                    stream);
            if (lastIndex[stream] >= 0 && streamTable[lastIndex[stream]].Offset > (int)streamTable[thisIndex].Offset)
                MyAssertHandler(
                    ".\\r_material.cpp",
                    688,
                    0,
                    "%s",
                    "streamTable[lastIndex[stream]].Offset <= streamTable[thisIndex].Offset");
            lastIndex[stream] = thisIndex;
        }
    }
}

void __cdecl Load_BuildVertexDecl(MaterialVertexDeclaration **mtlVertDecl)
{
    IDirect3DVertexDeclaration9 *v1; // eax
    MaterialStreamRouting data[16]; // [esp+8h] [ebp-28h] BYREF
    int vertDeclType; // [esp+2Ch] [ebp-4h]

    memcpy(data, &(*mtlVertDecl)->routing, sizeof(data));
    for (vertDeclType = 0; vertDeclType < 16; ++vertDeclType)
    {
        if (r_loadForRenderer->current.enabled)
        {
            v1 = Material_BuildVertexDecl(data, (*mtlVertDecl)->streamCount, s_streamSourceInfo[vertDeclType]);
            (*mtlVertDecl)->routing.decl[vertDeclType] = v1;
        }
        else
        {
            (*mtlVertDecl)->routing.decl[vertDeclType] = 0;
        }
    }
    (*mtlVertDecl)->isLoaded = 1;
}

IDirect3DVertexDeclaration9 *__cdecl Material_BuildVertexDecl(
    const MaterialStreamRouting *routingData,
    int streamCount,
    const stream_source_info_t *sourceTable)
{
    int v4; // ecx
    int v5; // ecx
    const char *v6; // eax
    int hr; // [esp+0h] [ebp-824h]
    int elemIndexInsert; // [esp+4h] [ebp-820h]
    const stream_source_info_t *sourceInfo; // [esp+8h] [ebp-81Ch]
    _D3DVERTEXELEMENT9 declEnd; // [esp+Ch] [ebp-818h]
    _D3DVERTEXELEMENT9 elemTable[256]; // [esp+14h] [ebp-810h] BYREF
    IDirect3DVertexDeclaration9 *decl; // [esp+818h] [ebp-Ch] BYREF
    const stream_dest_info_t *destInfo; // [esp+81Ch] [ebp-8h]
    int elemIndex; // [esp+820h] [ebp-4h]

    decl = 0;
    declEnd.Stream = 255;
    declEnd.Offset = 0;
    declEnd.Type = 17;
    declEnd.Method = 0;
    declEnd.Usage = 0;
    declEnd.UsageIndex = 0;
    AssertValidVertexDeclOffsets(sourceTable);
    elemIndex = 0;
    while (streamCount)
    {
        if (routingData->source >= 9u)
            MyAssertHandler(
                ".\\r_material.cpp",
                712,
                0,
                "routingData->source doesn't index STREAM_SRC_COUNT\n\t%i not in [0, %i)",
                routingData->source,
                9);
        sourceInfo = &sourceTable[routingData->source];
        if (sourceInfo->Stream == 255)
            return 0;
        destInfo = (const stream_dest_info_t *)(2 * routingData->dest + 9358400);
        for (elemIndexInsert = elemIndex;
            elemIndexInsert > 0 && *(&declEnd.Stream + 4 * elemIndexInsert) > (int)sourceInfo->Stream;
            --elemIndexInsert)
        {
            v4 = *((unsigned int *)&declEnd.Type + 2 * elemIndexInsert);
            *(unsigned int *)&elemTable[elemIndexInsert].Stream = *((unsigned int *)&declEnd.Stream + 2 * elemIndexInsert);
            *(unsigned int *)&elemTable[elemIndexInsert].Type = v4;
        }
        elemTable[elemIndexInsert].Stream = sourceInfo->Stream;
        elemTable[elemIndexInsert].Offset = sourceInfo->Offset;
        elemTable[elemIndexInsert].Type = sourceInfo->Type;
        elemTable[elemIndexInsert].Method = 0;
        elemTable[elemIndexInsert].Usage = destInfo->Usage;
        elemTable[elemIndexInsert].UsageIndex = destInfo->UsageIndex;
        ++elemIndex;
        ++routingData;
        --streamCount;
    }
    v5 = elemIndex;
    *(unsigned int *)&elemTable[elemIndex].Stream = *(unsigned int *)&declEnd.Stream;
    *(unsigned int *)&elemTable[v5].Type = *(unsigned int *)&declEnd.Type;
    do
    {
        if (r_logFile && r_logFile->current.integer)
            RB_LogPrint("dx.device->CreateVertexDeclaration( elemTable, &decl )\n");
        hr = dx.device->CreateVertexDeclaration(dx. elemTable, &decl);
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v6 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_material.cpp (%i) dx.device->CreateVertexDeclaration( elemTable, &decl ) failed: %s\n",
                    734,
                    v6);
            } while (alwaysfails);
        }
    } while (alwaysfails);
    if (!decl)
        MyAssertHandler(".\\r_material.cpp", 736, 0, "%s", "decl");
    return decl;
}

MaterialTechniqueSet *__cdecl Material_FindTechniqueSet(const char *name, MtlTechSetNotFoundBehavior notFoundBehavior)
{
    if (useFastFile->current.enabled)
        return Material_FindTechniqueSet_FastFile(name, notFoundBehavior);
    else
        return Material_FindTechniqueSet_LoadObj(name, notFoundBehavior);
}

MaterialTechniqueSet *__cdecl Material_FindTechniqueSet_FastFile(
    const char *name,
    MtlTechSetNotFoundBehavior notFoundBehavior)
{
    XAssetHeader header; // [esp+4h] [ebp-4h]

    header.xmodelPieces = DB_FindXAssetHeader(ASSET_TYPE_TECHNIQUE_SET, name).xmodelPieces;
    if (notFoundBehavior || !DB_IsXAssetDefault(ASSET_TYPE_TECHNIQUE_SET, name))
        return (MaterialTechniqueSet *)header.xmodelPieces;
    else
        return 0;
}

void __cdecl Material_DirtySort()
{
    rgp.needSortMaterials = 1;
}

bool __cdecl Material_IsDefault(const Material *material)
{
    if (!material)
        MyAssertHandler(".\\r_material.cpp", 1175, 0, "%s", "material");
    if (!rgp.defaultMaterial)
        MyAssertHandler(".\\r_material.cpp", 1176, 0, "%s", "rgp.defaultMaterial");
    if (material->textureTable != rgp.defaultMaterial->textureTable)
        return 0;
    if (material->constantTable == rgp.defaultMaterial->constantTable)
        return material->techniqueSet == rgp.defaultMaterial->techniqueSet;
    return 0;
}

Material *__cdecl Material_Register_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_MATERIAL, name).material;
}

Material *__cdecl Material_Register(const char *name, int imageTrack)
{
    if (useFastFile->current.enabled)
        return Material_Register_FastFile(name);
    else
        return Material_Register_LoadObj(name, imageTrack);
}

Material *__cdecl Material_RegisterHandle(const char *name, int imageTrack)
{
    if (!name)
        MyAssertHandler(".\\r_material.cpp", 1324, 0, "%s", "name");
    if (!rgp.defaultMaterial)
        MyAssertHandler(".\\r_material.cpp", 1325, 0, "%s", "rgp.defaultMaterial");
    if (g_alwaysUseDefaultMaterial && rgp.defaultMaterial)
        return rgp.defaultMaterial;
    if (*name)
        return Material_Register(name, imageTrack);
    else
        return rgp.defaultMaterial;
}

void __cdecl R_MaterialList_f()
{
    char *fmt; // [esp+8h] [ebp-4150h]
    unsigned int i; // [esp+138h] [ebp-4020h]
    const char **sceneEntIndex; // [esp+13Ch] [ebp-401Ch]
    int v3; // [esp+140h] [ebp-4018h]
    ShadowCandidate *v4; // [esp+144h] [ebp-4014h]
    unsigned int inData; // [esp+148h] [ebp-4010h] BYREF
    ShadowCandidate v6[2049]; // [esp+14Ch] [ebp-400Ch] BYREF
    float v7; // [esp+4154h] [ebp-4h]

    v3 = 0;
    Com_Printf(8, "-----------------------\n");
    inData = 0;
    DB_EnumXAssets(ASSET_TYPE_MATERIAL, (void(__cdecl *)(XAssetHeader, void *))R_GetMaterialList, &inData, 0);
    std::_Sort<ShadowCandidate *, int, bool(__cdecl *)(ShadowCandidate const &, ShadowCandidate const &)>(
        v6,
        &v6[inData],
        (int)(8 * inData) >> 3,
        (bool(__cdecl *)(const ShadowCandidate *, const ShadowCandidate *))R_MaterialCompare);
    Com_Printf(8, "geo KB   name\n");
    for (i = 0; i < inData; ++i)
    {
        v4 = &v6[i];
        sceneEntIndex = (const char **)v4->sceneEntIndex;
        if (!v4->sceneEntIndex)
            MyAssertHandler(".\\r_material.cpp", 1431, 0, "%s", "material");
        v3 += LOunsigned int(v4->weight);
        v7 = (double)SLOunsigned int(v4->weight) / 1024.0;
        if (v7 >= 10.0)
            fmt = "%6.0f";
        else
            fmt = "%6.1f";
        Com_Printf(8, fmt, v7);
        Com_Printf(8, "   %s\n", *sceneEntIndex);
    }
    Com_Printf(8, "-----------------------\n");
    Com_Printf(8, "current total  %5.1f MB\n", (double)v3 / 1048576.0);
    Com_Printf(8, "%i total geometry materials\n", inData);
    Com_Printf(8, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

void __cdecl R_GetMaterialList(XAssetHeader header, char *data)
{
    int memory; // [esp+0h] [ebp-Ch]
    XAssetHeader *materialMemory; // [esp+4h] [ebp-8h]

    memory = R_GetMaterialMemory(header.material);
    if (memory)
    {
        if (*(unsigned int *)data >= 0x800u)
            MyAssertHandler(".\\r_material.cpp", 1393, 0, "%s", "materialList->count < ARRAY_COUNT( materialList->sorted )");
        materialMemory = (XAssetHeader *)&data[8 * *(unsigned int *)data + 4];
        materialMemory->xmodelPieces = header.xmodelPieces;
        materialMemory[1].xmodelPieces = (XModelPieces *)memory;
        ++*(unsigned int *)data;
    }
}

int __cdecl R_GetMaterialMemory(Material *material)
{
    int i; // [esp+0h] [ebp-4h]

    if (!rgp.world)
        return 0;
    for (i = 0; i < rgp.world->materialMemoryCount; ++i)
    {
        if (material == rgp.world->materialMemory[i].material)
            return rgp.world->materialMemory[i].memory;
    }
    return 0;
}

bool __cdecl R_MaterialCompare(const MaterialMemory *material0, const MaterialMemory *material1)
{
    return material0->memory < material1->memory;
}

const char *__cdecl Material_GetName(Material *handle)
{
    if (!handle)
        MyAssertHandler(".\\r_material.cpp", 1484, 0, "%s", "handle");
    return Material_FromHandle(handle)->info.name;
}

void __cdecl Material_ReleasePassResources(MaterialPass *pass)
{
    IDirect3DSurface9 *v1; // [esp+0h] [ebp-10h]
    IDirect3DSurface9 *var; // [esp+4h] [ebp-Ch]
    IDirect3DPixelShader9 *varCopy; // [esp+8h] [ebp-8h]
    int declIndex; // [esp+Ch] [ebp-4h]

    if (!pass->pixelShader)
        MyAssertHandler(".\\r_material.cpp", 933, 0, "%s", "pass->pixelShader");
    if (pass->pixelShader->prog.ps)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("pass->pixelShader->prog.ps->Release()\n");
            varCopy = pass->pixelShader->prog.ps;
            pass->pixelShader->prog.ps = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(
                (IDirect3DSurface9 *)varCopy,
                "pass->pixelShader->prog.ps",
                ".\\r_material.cpp",
                935);
        } while (alwaysfails);
    }
    if (!pass->vertexShader)
        MyAssertHandler(".\\r_material.cpp", 937, 0, "%s", "pass->vertexShader");
    if (pass->vertexShader->prog.vs)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("pass->vertexShader->prog.vs->Release()\n");
            var = (IDirect3DSurface9 *)pass->vertexShader->prog.vs;
            pass->vertexShader->prog.vs = 0;
            R_ReleaseAndSetNULL<IDirect3DDevice9>(var, "pass->vertexShader->prog.vs", ".\\r_material.cpp", 939);
        } while (alwaysfails);
    }
    if (!pass->vertexDecl)
        MyAssertHandler(".\\r_material.cpp", 941, 0, "%s", "pass->vertexDecl");
    if (pass->vertexDecl->isLoaded)
    {
        pass->vertexDecl->isLoaded = 0;
        for (declIndex = 0; declIndex < 16; ++declIndex)
        {
            if (pass->vertexDecl->routing.decl[declIndex])
            {
                do
                {
                    if (r_logFile)
                    {
                        if (r_logFile->current.integer)
                            RB_LogPrint("pass->vertexDecl->routing.decl[declIndex]->Release()\n");
                    }
                    v1 = (IDirect3DSurface9 *)pass->vertexDecl->routing.decl[declIndex];
                    pass->vertexDecl->routing.decl[declIndex] = 0;
                    R_ReleaseAndSetNULL<IDirect3DDevice9>(
                        v1,
                        "pass->vertexDecl->routing.decl[declIndex]",
                        ".\\r_material.cpp",
                        948);
                } while (alwaysfails);
            }
        }
    }
}

void __cdecl Material_ReleaseTechniqueSetResources(MaterialTechniqueSet *techniqueSet)
{
    MaterialTechnique *technique; // [esp+10h] [ebp-Ch]
    int techType; // [esp+14h] [ebp-8h]
    int passIndex; // [esp+18h] [ebp-4h]

    for (techType = 0; techType < 34; ++techType)
    {
        technique = techniqueSet->techniques[techType];
        if (technique)
        {
            for (passIndex = 0; passIndex < technique->passCount; ++passIndex)
                Material_ReleasePassResources(&technique->passArray[passIndex]);
        }
    }
}

void __cdecl Material_ReloadPassResources(MaterialPass *pass)
{
    if (!pass->pixelShader)
        MyAssertHandler(".\\r_material.cpp", 958, 0, "%s", "pass->pixelShader");
    if (!pass->pixelShader->prog.ps)
        Load_CreateMaterialPixelShader(&pass->pixelShader->prog.loadDef, pass->pixelShader);
    if (!pass->vertexShader)
        MyAssertHandler(".\\r_material.cpp", 962, 0, "%s", "pass->vertexShader");
    if (!pass->vertexShader->prog.vs)
        Load_CreateMaterialVertexShader(&pass->vertexShader->prog.loadDef, pass->vertexShader);
    if (!pass->vertexDecl)
        MyAssertHandler(".\\r_material.cpp", 966, 0, "%s", "pass->vertexDecl");
    if (!pass->vertexDecl->isLoaded)
    {
        Load_BuildVertexDecl(&pass->vertexDecl);
        if (!pass->vertexDecl->isLoaded)
            MyAssertHandler(".\\r_material.cpp", 970, 0, "%s", "pass->vertexDecl->isLoaded");
    }
}

void __cdecl Material_ReloadTechniqueSetResources(MaterialTechniqueSet *techniqueSet)
{
    MaterialTechnique *technique; // [esp+0h] [ebp-Ch]
    int techType; // [esp+4h] [ebp-8h]
    int passIndex; // [esp+8h] [ebp-4h]

    for (techType = 0; techType < 34; ++techType)
    {
        technique = techniqueSet->techniques[techType];
        if (technique)
        {
            for (passIndex = 0; passIndex < technique->passCount; ++passIndex)
                Material_ReloadPassResources(&technique->passArray[passIndex]);
        }
    }
}

void __cdecl Material_ReloadTechniqueSet(XAssetHeader header)
{
    Material_ReloadTechniqueSetResources(header.techniqueSet);
}

void __cdecl Material_ReleaseTechniqueSet(XAssetHeader header)
{
    Material_ReleaseTechniqueSetResources(header.techniqueSet);
}

void __cdecl Material_ReloadAll()
{
    DB_EnumXAssets(ASSET_TYPE_TECHNIQUE_SET, (void(__cdecl *)(XAssetHeader, void *))Material_ReloadTechniqueSet, 0, 1);
}

void __cdecl Material_ReleaseAll()
{
    DB_EnumXAssets(ASSET_TYPE_TECHNIQUE_SET, (void(__cdecl *)(XAssetHeader, void *))Material_ReleaseTechniqueSet, 0, 1);
}

const Material *__cdecl Material_FromHandle(Material *handle)
{
    if (!handle)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_material.h", 285, 0, "%s", "handle");
    if (!handle->info.name)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_material.h", 286, 0, "%s", "handle->info.name");
    if (!*handle->info.name)
        MyAssertHandler("c:\\trees\\cod3\\src\\gfx_d3d\\r_material.h", 287, 0, "%s", "handle->info.name[0]");
    return handle;
}

void __cdecl Material_Init()
{
    if (!useFastFile->current.enabled)
    {
        memset((unsigned __int8 *)&materialGlobals, 0, sizeof(materialGlobals));
        Material_PreLoadAllShaderText();
    }
    Material_LoadBuiltIn(s_builtInMaterials, 50);
    Material_Register("statmon_warning_tris", 1);
}

void __cdecl Material_Shutdown()
{
    Material_FreeAll();
    memset((unsigned __int8 *)&materialGlobals, 0, sizeof(materialGlobals));
    memset((unsigned __int8 *)rg.materialHashTable, 0, sizeof(rg.materialHashTable));
    rgp.materialCount = 0;
}

void __cdecl Material_LoadBuiltIn(const BuiltInMaterialTable *mtlTable, int mtlTableCount)
{
    int builtInMtlIndex; // [esp+0h] [ebp-4h]

    for (builtInMtlIndex = 0; builtInMtlIndex < mtlTableCount; ++builtInMtlIndex)
    {
        if (*mtlTable[builtInMtlIndex].material)
            MyAssertHandler(".\\r_material.cpp", 1512, 0, "%s", "!*mtlTable[builtInMtlIndex].material");
        *mtlTable[builtInMtlIndex].material = Material_Register((char *)mtlTable[builtInMtlIndex].name, 0);
        if (!*mtlTable[builtInMtlIndex].material)
            Com_Error(ERR_FATAL, "Could not find material '%s'", mtlTable[builtInMtlIndex].name);
    }
}

void __cdecl Material_ForEachTechniqueSet_FastFile(void(__cdecl *callback)(MaterialTechniqueSet *))
{
    unsigned int inData[1024]; // [esp+0h] [ebp-1008h] BYREF
    int v2; // [esp+1000h] [ebp-8h]

    v2 = 0;
    DB_EnumXAssets(
        ASSET_TYPE_TECHNIQUE_SET,
        (void(__cdecl *)(XAssetHeader, void *))Material_CollateTechniqueSets,
        inData,
        0);
    while (v2)
        callback((MaterialTechniqueSet *)inData[--v2]);
}

void __cdecl Material_CollateTechniqueSets(XAssetHeader header, XAssetHeader *userData)
{
    if (userData[1024].xmodelPieces >= (XModelPieces *)0x400)
        MyAssertHandler(
            ".\\r_material.cpp",
            1744,
            0,
            "techSetList->count doesn't index MAX_TECHNIQUE_SETS\n\t%i not in [0, %i)",
            userData[1024].xmodelPieces,
            1024);
    userData[(int)userData[1024].xmodelPieces++] = header;
}

bool __cdecl IsValidMaterialHandle(Material *const handle)
{
    if (((unsigned __int8)handle & 3) != 0)
        MyAssertHandler(".\\r_material.cpp", 1798, 0, "%s", "( 0x0003 & reinterpret_cast<int>( handle ) ) == 0x0");
    return handle && handle->info.name && *handle->info.name;
}

void __cdecl Material_PreventOverrideTechniqueGeneration()
{
    g_generateOverrideTechniques = 0;
}

void __cdecl Material_UpdatePicmipAll()
{
    R_SyncRenderThread();
    R_SetPicmip();
    DB_EnumXAssets(ASSET_TYPE_MATERIAL, (void(__cdecl *)(XAssetHeader, void *))Material_UpdatePicmipSingle, 0, 1);
}

void __cdecl R_Cmd_ReloadMaterialTextures()
{
    char *v0; // eax
    Material *material; // [esp+0h] [ebp-8h]
    const char *name; // [esp+4h] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        name = Cmd_Argv(1);
        if (!name)
            MyAssertHandler(".\\r_material.cpp", 1702, 0, "%s", "name");
        material = Material_Find(name);
        if (material)
        {
            RB_BindDefaultImages();
            Material_ReloadTextures(material);
        }
        else
        {
            v0 = va("ReloadMaterialTextures: Material '%s' is not currently loaded.\n", name);
            Com_Printf(8, v0);
        }
    }
    else
    {
        Com_Printf(8, "Usage: reloadmaterialtextures <materialname>\n");
    }
}