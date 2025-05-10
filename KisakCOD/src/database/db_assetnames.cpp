#include "database.h"

//int marker_db_assetnames 828ddeec     db_assetnames.obj

const char *(__cdecl *DB_XAssetGetNameHandler[33])(const XAssetHeader *) =
{
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_ImageGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    0,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_StringTableGetName,
    DB_LocalizeEntryGetName,
    DB_StringTableGetName,
    0,
    DB_StringTableGetName,
    DB_StringTableGetName,
    0,
    0,
    0,
    0,
    DB_StringTableGetName,
    DB_StringTableGetName
};

void(__cdecl *DB_XAssetSetNameHandler[33])(XAssetHeader *, const char *) =
{
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_ImageSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    0,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_StringTableSetName,
    DB_LocalizeEntrySetName,
    DB_StringTableSetName,
    0,
    DB_StringTableSetName,
    DB_StringTableSetName,
    0,
    0,
    0,
    0,
    DB_StringTableSetName,
    DB_StringTableSetName
};

// KISAKTODO: make these non-fixed
int __cdecl DB_SizeofXAsset_RawFile_()
{
    return sizeof(RawFile);
}
int __cdecl DB_SizeofXAsset_GameWorldSp_()
{
    return sizeof(GameWorldSp);
}
int __cdecl DB_SizeofXAsset_XAnimParts_()
{
    return sizeof(XAnimParts);
}
int __cdecl DB_SizeofXAsset_XModel_()
{
    return sizeof(XModel);
}
int __cdecl DB_SizeofXAsset_Material_()
{
    return sizeof(Material);
}
int __cdecl DB_SizeofXAsset_MaterialTechniqueSet_()
{
    return sizeof(MaterialTechniqueSet);
}
int __cdecl DB_SizeofXAsset_GfxImage_()
{
    return sizeof(GfxImage);
}
int __cdecl DB_SizeofXAsset_SndCurve_()
{
    return sizeof(SndCurve);
}
int __cdecl DB_SizeofXAsset_menuDef_t_()
{
    return sizeof(menuDef_t);
}
int __cdecl DB_SizeofXAsset_StringTable_()
{
    return sizeof(StringTable);
}
int __cdecl DB_SizeofXAsset_GameWorldMp_()
{
    return sizeof(GameWorldMp);
}
int __cdecl DB_SizeofXAsset_GfxWorld_()
{
    return sizeof(GfxWorld);
}
int __cdecl DB_SizeofXAsset_Font_s_()
{
    return sizeof(Font_s);
}
int __cdecl DB_SizeofXAsset_FxImpactTable_()
{
    return sizeof(FxImpactTable);
}
int __cdecl DB_SizeofXAsset_WeaponDef_()
{
    return sizeof(WeaponDef);
}
int __cdecl DB_SizeofXAsset_FxEffectDef_()
{
    return sizeof(FxEffectDef);
}
int(__cdecl *DB_GetXAssetSizeHandler[33])() =
{
    DB_SizeofXAsset_RawFile_,
    DB_SizeofXAsset_GameWorldSp_,
    DB_SizeofXAsset_XAnimParts_,
    DB_SizeofXAsset_XModel_,
    DB_SizeofXAsset_Material_,
    DB_SizeofXAsset_MaterialTechniqueSet_,
    DB_SizeofXAsset_GfxImage_,
    DB_SizeofXAsset_RawFile_,
    DB_SizeofXAsset_SndCurve_,
    DB_SizeofXAsset_GameWorldSp_,
    DB_SizeofXAsset_menuDef_t_,
    DB_SizeofXAsset_menuDef_t_,
    DB_SizeofXAsset_StringTable_,
    DB_SizeofXAsset_GameWorldSp_,
    DB_SizeofXAsset_GameWorldMp_,
    DB_SizeofXAsset_RawFile_,
    DB_SizeofXAsset_GfxWorld_,
    DB_SizeofXAsset_StringTable_,
    0,
    DB_SizeofXAsset_Font_s_,
    DB_SizeofXAsset_RawFile_,
    DB_SizeofXAsset_menuDef_t_,
    DB_SizeofXAsset_FxImpactTable_,
    DB_SizeofXAsset_WeaponDef_,
    0,
    DB_SizeofXAsset_FxEffectDef_,
    DB_SizeofXAsset_FxImpactTable_,
    0,
    0,
    0,
    0,
    DB_SizeofXAsset_RawFile_,
    DB_SizeofXAsset_StringTable_,
};

void __cdecl DB_StringTableSetName(XAssetHeader *header, const char *name)
{
    header->xmodelPieces->name = name;
}

const char *__cdecl DB_ImageGetName(const XAssetHeader *header)
{
    //return header->xmodelPieces[2].pieces;
    //return header->xmodelPieces->name; // KISAKTODO: this functionality is custom and probably different
    return header->image->name;
}

void __cdecl DB_ImageSetName(XAssetHeader *header, const char *name)
{
    //header->xmodelPieces[2].pieces = name;
    //header->xmodelPieces[2].name = name;
    header->image->name = name;
}

const char *__cdecl DB_StringTableGetName(const XAssetHeader *header)
{
    return header->stringTable->name;
}

const char *__cdecl DB_LocalizeEntryGetName(const XAssetHeader *header)
{
    return header->localize->name;
}

void __cdecl DB_LocalizeEntrySetName(XAssetHeader *header, const char *name)
{
    header->localize->name = name;
}

const char *__cdecl DB_GetXAssetHeaderName(int type, const XAssetHeader *header)
{
    const char *v2; // eax
    const char *name; // [esp+0h] [ebp-4h]

    if (!header)
        MyAssertHandler(".\\database\\db_assetnames.cpp", 590, 0, "%s", "header");
    if (!DB_XAssetGetNameHandler[type])
        MyAssertHandler(".\\database\\db_assetnames.cpp", 591, 0, "%s", "DB_XAssetGetNameHandler[type]");
    if (!header->data)
        MyAssertHandler(".\\database\\db_assetnames.cpp", 592, 0, "%s", "header->data");
    name = DB_XAssetGetNameHandler[type](header);
    if (!name)
    {
        v2 = va("Name not found for asset type %s\n", g_assetNames[type]);
        MyAssertHandler(".\\database\\db_assetnames.cpp", 594, 0, "%s\n\t%s", "name", v2);
    }
    return name;
}

const char *__cdecl DB_GetXAssetName(const XAsset *asset)
{
    iassert(asset);
    return DB_GetXAssetHeaderName(asset->type, &asset->header);
}

void __cdecl DB_SetXAssetName(XAsset *asset, const char *name)
{
    if (!DB_XAssetSetNameHandler[asset->type])
        MyAssertHandler(".\\database\\db_assetnames.cpp", 608, 0, "%s", "DB_XAssetSetNameHandler[asset->type]");
    DB_XAssetSetNameHandler[asset->type](&asset->header, name);
}

int __cdecl DB_GetXAssetTypeSize(int type)
{
    if (!DB_GetXAssetSizeHandler[type])
        MyAssertHandler(".\\database\\db_assetnames.cpp", 615, 0, "%s", "DB_GetXAssetSizeHandler[type]");
    return DB_GetXAssetSizeHandler[type]();
}

const char *__cdecl DB_GetXAssetTypeName(unsigned int type)
{
    if (type > 0x20)
        MyAssertHandler(".\\database\\db_assetnames.cpp", 621, 0, "%s", "type >= 0 && type < ASSET_TYPE_COUNT");
    return g_assetNames[type];
}

