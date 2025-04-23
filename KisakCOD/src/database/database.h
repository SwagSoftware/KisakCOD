#pragma once

#include <zlib/zlib.h>

#include <cstddef>

#include <xanim/xanim.h>
#include <xanim/xmodel.h>
#include <win32/win_local.h>

struct StreamDelayInfo // sizeof=0x8
{
    const void *ptr;
    int size;
};

struct StreamPosInfo // sizeof=0x8
{                                       // ...
    unsigned __int8 *pos;               // ...
    unsigned int index;                 // ...
};

struct AssetList // sizeof=0xC
{                                       // ...
    int assetCount;                     // ...
    int maxCount;                       // ...
    XAssetHeader *assets;               // ...
};

// db_registry
void __cdecl TRACK_db_registry();
char *__cdecl DB_ReferencedFFChecksums();
char *__cdecl DB_ReferencedFFNameList();
void __cdecl Hunk_OverrideDataForFile(int type, const char *name, void *data);
void __cdecl DB_GetIndexBufferAndBase(unsigned __int8 zoneHandle, void *indices, void **ib, int *baseIndex);
void __cdecl DB_GetVertexBufferAndOffset(unsigned __int8 zoneHandle, _BYTE *verts, void **vb, int *vertexOffset);
void __cdecl DB_EndRecoverLostDevice();
void __cdecl DB_BeginRecoverLostDevice();
void __cdecl DB_InitSingleton(void *pool, int size);
void __cdecl Load_PhysPresetAsset(XAssetHeader *physPreset);
void __cdecl Mark_PhysPresetAsset(PhysPreset *physPreset);
void __cdecl Load_XAnimPartsAsset(XAssetHeader *parts);
void __cdecl Mark_XAnimPartsAsset(XAnimParts *parts);
void __cdecl Load_XModelAsset(XAssetHeader *model);
void __cdecl Mark_XModelAsset(XModel *model);
void __cdecl Load_MaterialAsset(XAssetHeader *material);
void __cdecl Mark_MaterialAsset(Material *material);
void __cdecl Load_MaterialTechniqueSetAsset(XAssetHeader *techniqueSet);
void __cdecl Mark_MaterialTechniqueSetAsset(MaterialTechniqueSet *techniqueSet);
void __cdecl Load_GfxImageAsset(XAssetHeader *image);
void __cdecl Mark_GfxImageAsset(GfxImage *image);
void __cdecl Load_snd_alias_list_Asset(XAssetHeader *sound);
void __cdecl Mark_snd_alias_list_Asset(snd_alias_list_t *sound);
void __cdecl Load_SndCurveAsset(XAssetHeader *sndCurve);
void __cdecl Mark_SndCurveAsset(SndCurve *sndCurve);
void __cdecl Load_LoadedSoundAsset(XAssetHeader *loadSnd);
void __cdecl Mark_LoadedSoundAsset(LoadedSound *loadSnd);
void __cdecl Load_ClipMapAsset(XAssetHeader *clipMap);
void __cdecl Mark_ClipMapAsset(clipMap_t *clipMap);
void __cdecl DB_RemoveClipMap(XAssetHeader ass);
void __cdecl Load_ComWorldAsset(XAssetHeader *comWorld);
void __cdecl Mark_ComWorldAsset(ComWorld *comWorld);
void __cdecl DB_RemoveComWorld(XAssetHeader ass);
void __cdecl Load_GameWorldSpAsset(XAssetHeader *gameWorldSp);
void __cdecl Mark_GameWorldSpAsset(GameWorldSp *gameWorldSp);
void __cdecl Load_GameWorldMpAsset(XAssetHeader *gameWorldMp);
void __cdecl Mark_GameWorldMpAsset(GameWorldMp *gameWorldMp);
void __cdecl Load_MapEntsAsset(XAssetHeader *mapEnts);
void __cdecl Mark_MapEntsAsset(MapEnts *mapEnts);
void __cdecl Load_GfxWorldAsset(XAssetHeader *gfxWorld);
void __cdecl Mark_GfxWorldAsset(GfxWorld *gfxWorld);
void __cdecl DB_RemoveGfxWorld(XAssetHeader ass);
void __cdecl Load_LightDefAsset(XAssetHeader *lightDef);
void __cdecl Mark_LightDefAsset(GfxLightDef *lightDef);
void __cdecl Load_FontAsset(XAssetHeader *font);
void __cdecl Mark_FontAsset(Font_s *font);
void __cdecl Load_MenuListAsset(XAssetHeader *menuList);
void __cdecl Mark_MenuListAsset(MenuList *menuList);
void __cdecl Load_MenuAsset(XAssetHeader *menu);
void __cdecl Mark_MenuAsset(menuDef_t *menu);
void __cdecl DB_DynamicCloneMenu(XAssetHeader from, XAssetHeader to, int swag = 0);
void __cdecl DB_RemoveWindowFocus(windowDef_t *window);
void __cdecl Load_LocalizeEntryAsset(XAssetHeader *localize);
void __cdecl Mark_LocalizeEntryAsset(LocalizeEntry *localize);
void __cdecl Load_WeaponDefAsset(XAssetHeader *weapon);
void __cdecl Mark_WeaponDefAsset(WeaponDef *weapon);
void __cdecl Load_FxEffectDefAsset(XAssetHeader *fx);
void __cdecl Mark_FxEffectDefAsset(FxEffectDef *fx);
void __cdecl Load_FxEffectDefFromName(const char **name);
void __cdecl Load_FxImpactTableAsset(XAssetHeader *impactFx);
void __cdecl Mark_FxImpactTableAsset(FxImpactTable *impactFx);
void __cdecl Load_RawFileAsset(XAssetHeader *rawfile);
void __cdecl Mark_RawFileAsset(RawFile *rawfile);
void __cdecl Load_StringTableAsset(XAssetHeader *stringTable);
void __cdecl Mark_StringTableAsset(StringTable *stringTable);
XAssetHeader __cdecl DB_AllocMaterial(XAssetHeader *pool);
void __cdecl DB_FreeMaterial(XAssetPoolEntry<StringTable> **pool, XAssetHeader header);
void __cdecl DB_EnumXAssets_FastFile(
    XAssetType type,
    void(__cdecl *func)(XAssetHeader, void *),
    void *inData,
    bool includeOverride);
XAssetHeader __cdecl DB_FindXAssetHeader(XAssetType type, const char *name);
void __cdecl DB_Sleep(unsigned int msec);
void __cdecl DB_LogMissingAsset(XAssetType type, const char *name);
void __cdecl DB_RegisteredReorderAsset(int type, const char *assetName, XAssetEntry *assetEntry);
XAssetEntryPoolEntry *__cdecl DB_FindXAssetEntry(XAssetType type, const char *name);
unsigned __int32 __cdecl DB_HashForName(const char *name, XAssetType type);
XAssetEntry *__cdecl DB_CreateDefaultEntry(XAssetType type, char *name);
XAssetEntryPoolEntry *__cdecl DB_AllocXAssetEntry(XAssetType type, unsigned __int8 zoneIndex);
XAssetHeader __cdecl DB_AllocXAssetHeader(XAssetType type);
void __cdecl DB_PrintAssetName(XAssetHeader header, int *data);
void __cdecl DB_CloneXAssetInternal(const XAsset *from, XAsset *to);
XAssetHeader __cdecl DB_FindXAssetDefaultHeaderInternal(XAssetType type);
void __cdecl PrintWaitedError(XAssetType type, char *name, int waitedMsec);
void __cdecl DB_Update();
void __cdecl DB_SetInitializing(bool inUse);
bool __cdecl DB_GetInitializing();
bool __cdecl DB_IsXAssetDefault(XAssetType type, const char *name);
int __cdecl DB_GetAllXAssetOfType_FastFile(XAssetType type, XAssetHeader *assets, int maxCount);
XAssetHeader __cdecl DB_AddXAsset(XAssetType type, XAssetHeader header);
XAssetEntryPoolEntry *__cdecl DB_LinkXAssetEntry(XAssetEntryPoolEntry *newEntry, int allowOverride);
void __cdecl DB_FreeXAssetEntry(XAssetEntryPoolEntry *assetEntry);
void __cdecl DB_FreeXAssetHeader(XAssetType type, XAssetHeader header);
void __cdecl DB_CloneXAssetEntry(const XAssetEntry *from, XAssetEntry *to);
void __cdecl DB_DynamicCloneXAsset(XAssetHeader from, XAssetHeader to, XAssetType type, int fromDefault);
void __cdecl DB_DelayedCloneXAsset(XAssetEntry *newEntry);
bool __cdecl DB_OverrideAsset(unsigned int newZoneIndex, unsigned int existingZoneIndex);
void __cdecl DB_GetXAsset(XAssetType type, XAssetHeader header);
void DB_PostLoadXZone();
void __cdecl DB_UpdateDebugZone();
void __cdecl DB_SyncXAssets();
void __cdecl DB_LoadXAssets(XZoneInfo *zoneInfo, unsigned int zoneCount, int sync);
int DB_Init();
void __cdecl DB_InitPoolHeader(XAssetType type);
void __cdecl DB_LoadXZone(XZoneInfo *zoneInfo, unsigned int zoneCount);
void __cdecl DB_LoadZone_f();
void __cdecl DB_InitThread();
void __cdecl  DB_Thread(unsigned int threadContext);
void DB_TryLoadXFile();
int __cdecl DB_TryLoadXFileInternal(char *zoneName, int zoneFlags);
void __cdecl DB_BuildOSPath(const char *zoneName, unsigned int size, char *filename);
int __cdecl DB_GetZoneAllocType(int zoneFlags);
void __cdecl DB_UnloadXZone(unsigned int zoneIndex, bool createDefault);
void __cdecl DB_RemoveXAsset(XAsset *asset);
void __cdecl DB_ReleaseXAssets();
void __cdecl DB_ShutdownXAssets();
void __cdecl DB_UnloadXZoneMemory(XZone *zone);
void DB_FreeDefaultEntries();
void __cdecl DB_UnloadXAssetsMemoryForZone(int zoneFreeFlags, int zoneFreeBit);
void __cdecl DB_UnloadXAssetsMemory(XZone *zone, int sortedIndex);
void __cdecl DB_ReplaceModel(const char *original, const char *replacement);
void __cdecl DB_ReplaceXAsset(XAssetType type, const char *original, const char *replacement);
void __cdecl DB_CloneXAsset(const XAsset *from, XAsset *to);
void DB_SyncExternalAssets();
void DB_ArchiveAssets();
void DB_FreeUnusedResources();
void DB_ExternalInitAssets();
void DB_UnarchiveAssets();
void __cdecl DB_Cleanup();

void __cdecl DB_EnumXAssets_FastFile(
    XAssetType type,
    void(__cdecl* func)(XAssetHeader, void*),
    void* inData,
    bool includeOverride);

int __cdecl DB_GetAllXAssetOfType_FastFile(XAssetType type, XAssetHeader* assets, int maxCount);
int __cdecl DB_GetAllXAssetOfType(XAssetType type, XAssetHeader* assets, int maxCount);
int __cdecl DB_GetAllXAssetOfType_LoadObj(XAssetType type, XAssetHeader* assets, int maxCount);

struct fileData_s;
void __cdecl DB_EnumXAssets(
    XAssetType type,
    void(__cdecl* func)(XAssetHeader, void*),
    void* inData,
    bool includeOverride);
void __cdecl DB_EnumXAssets_LoadObj(XAssetType type, void(* func)(void*, void*), void* inData);

void __cdecl DB_EnumXAssetsFor(
    fileData_s* fileData,
    int fileDataType,
    void(* func)(void*, void*),
    void* inData);

int __cdecl DB_FileSize(const char *zoneName, int isMod);
bool __cdecl DB_ModFileExists();

void __cdecl Load_GetCurrentZoneHandle(unsigned __int8 *handle);


// db_assetnames
void __cdecl DB_StringTableSetName(XAssetHeader *header, const char *name);
XModelPiece *__cdecl DB_ImageGetName(const XAssetHeader *header);
void __cdecl DB_ImageSetName(XAssetHeader *header, XModelPiece *name);
const char *__cdecl DB_StringTableGetName(const XAssetHeader *header);
const char *__cdecl DB_LocalizeEntryGetName(const XAssetHeader *header);
void __cdecl DB_LocalizeEntrySetName(XAssetHeader *header, const char *name);
const char *__cdecl DB_GetXAssetHeaderName(int type, const XAssetHeader *header);
const char *__cdecl DB_GetXAssetName(const XAsset *asset);
void __cdecl DB_SetXAssetName(XAsset *asset, const char *name);
int __cdecl DB_GetXAssetTypeSize(int type);
const char *__cdecl DB_GetXAssetTypeName(unsigned int type);


// db_auth
int __cdecl DB_AuthLoad_InflateInit(z_stream_s *stream, bool isSecure);
void __cdecl DB_AuthLoad_InflateEnd(z_stream_s *stream);
unsigned int __cdecl DB_AuthLoad_Inflate(z_stream_s *stream, int flush);


// db_file_load
void __cdecl DB_CancelLoadXFile();
int DB_WaitXFileStage();
void __cdecl DB_LoadedExternalData(int size);
double __cdecl DB_GetLoadedFraction();
void __cdecl DB_LoadXFileData(unsigned __int8 *pos, unsigned int size);
void DB_ReadXFileStage();
int __cdecl DB_ReadData();
void __stdcall DB_FileReadCompletion(
    unsigned int dwErrorCode,
    unsigned int dwNumberOfBytesTransfered,
    _OVERLAPPED *lpOverlapped);
void __cdecl DB_LoadXFileInternal();
void Load_XAssetListCustom();
void __cdecl Load_XAssetArrayCustom(int count);
void __cdecl DB_ResetZoneSize(int trackLoadProgress);
void __cdecl DB_LoadXFile(
    const char *path,
    void *f,
    const char *filename,
    XZoneMemory *zoneMem,
    void(__cdecl *interrupt)(),
    unsigned __int8 *buf,
    int allocType);

// db_memory
void __cdecl DB_RecoverGeometryBuffers(XZoneMemory *zoneMem);
void __cdecl DB_ReleaseGeometryBuffers(XZoneMemory *zoneMem);
void __cdecl DB_AllocXZoneMemory(
    unsigned int *blockSize,
    const char *filename,
    XZoneMemory *zoneMem,
    unsigned int allocType);
unsigned __int8 *__cdecl DB_MemAlloc(unsigned int size, unsigned int type, unsigned int allocType);


// db_stream
void __cdecl DB_InitStreams(XZoneMemory *zoneMem);
void __cdecl DB_PushStreamPos(unsigned int index);
void __cdecl DB_SetStreamIndex(unsigned int index);
void __cdecl DB_PopStreamPos();
unsigned __int8 *__cdecl DB_GetStreamPos();
unsigned __int8 *__cdecl DB_AllocStreamPos(int alignment);
void __cdecl DB_IncStreamPos(int size);
const void **__cdecl DB_InsertPointer();

// db_stream_load
void __cdecl Load_Stream(bool atStreamStart, unsigned __int8 *ptr, int size);
void __cdecl Load_DelayStream();
void __cdecl DB_ConvertOffsetToAlias(unsigned int *data);
void __cdecl DB_ConvertOffsetToPointer(unsigned int *data);
void __cdecl Load_XStringCustom(char **str);
void __cdecl Load_TempStringCustom(char **str);

// db_stringtable_load
void __cdecl Load_ScriptStringCustom(unsigned __int16 *var);
void __cdecl Mark_ScriptStringCustom(unsigned __int16 *var);


// db_load
void __cdecl Load_byte(bool atStreamStart);
void __cdecl Load_byteArray(bool atStreamStart, int count);
void __cdecl Load_charArray(bool atStreamStart, int count);
void __cdecl Load_int(bool atStreamStart);
void __cdecl Load_intArray(bool atStreamStart, int count);
void __cdecl Load_uintArray(bool atStreamStart, int count);
void __cdecl Load_uint(bool atStreamStart);
void __cdecl Load_uintArray(bool atStreamStart, int count);
void __cdecl Load_float(bool atStreamStart);
void __cdecl Load_floatArray(bool atStreamStart, int count);
void __cdecl Load_raw_uintArray(bool atStreamStart, int count);
unsigned __int8 *__cdecl AllocLoad_raw_uint128();
void __cdecl Load_raw_uint128Array(bool atStreamStart, int count);
void __cdecl Load_raw_byteArray(bool atStreamStart, int count);
void __cdecl Load_raw_byte16Array(bool atStreamStart, int count);
void __cdecl Load_vec2_tArray(bool atStreamStart, int count);
void __cdecl Load_vec3_t(bool atStreamStart);
void __cdecl Load_vec3_tArray(bool atStreamStart, int count);
void __cdecl Load_shortArray(bool atStreamStart, int count);
void __cdecl Load_ushortArray(bool atStreamStart, int count);
void __cdecl Load_XQuat2(bool atStreamStart);
void __cdecl Load_XQuat2Array(bool atStreamStart, int count);
unsigned __int8 *__cdecl AllocLoad_XBlendInfo();
void __cdecl Load_UnsignedShortArray(bool atStreamStart, int count);
void __cdecl Load_ScriptString(bool atStreamStart);
void __cdecl Load_ScriptStringArray(bool atStreamStart, int count);
unsigned __int8 *__cdecl AllocLoad_raw_byte();
void __cdecl Load_ConstCharArray(bool atStreamStart, int count);
void __cdecl Load_TempString(bool atStreamStart);
void __cdecl Load_TempStringArray(bool atStreamStart, int count);
void __cdecl Load_XString(bool atStreamStart);
void __cdecl Load_XStringArray(bool atStreamStart, int count);
void __cdecl Load_XStringPtr(bool atStreamStart);
void __cdecl Load_ScriptStringList(bool atStreamStart);
void __cdecl Load_complex_tArray(bool atStreamStart, int count);
void __cdecl Load_dmaterial_tArray(bool atStreamStart, int count);
void __cdecl Mark_ScriptString();
void __cdecl Mark_ScriptStringArray(int count);
void __cdecl Load_XAnimIndices();
void __cdecl Load_XAnimDynamicIndicesDeltaQuat(bool atStreamStart);
void __cdecl Load_XAnimDeltaPartQuatDataFrames(bool atStreamStart);
void __cdecl Load_XAnimDeltaPartQuatData(bool atStreamStart);
void __cdecl Load_XAnimDeltaPartQuat(bool atStreamStart);
void __cdecl Load_XAnimDeltaPart(bool atStreamStart);
void __cdecl Load_XAnimDynamicIndicesTrans(bool atStreamStart);
void __cdecl Load_ByteVecArray(bool atStreamStart, int count);
void __cdecl Load_UShortVecArray(bool atStreamStart, int count);
void __cdecl Load_XAnimDynamicFrames();
void __cdecl Load_XAnimPartTransFrames(bool atStreamStart);
void __cdecl Load_XAnimPartTransData(bool atStreamStart);
void __cdecl Load_XAnimPartTrans(bool atStreamStart);
void __cdecl Load_XAnimNotifyInfo(bool atStreamStart);
void __cdecl Load_XAnimNotifyInfoArray(bool atStreamStart, int count);
void __cdecl Load_XAnimParts(bool atStreamStart);
void __cdecl Load_XAnimPartsPtr(bool atStreamStart);
void __cdecl Mark_XAnimNotifyInfo();
void __cdecl Mark_XAnimNotifyInfoArray(int count);
void __cdecl Mark_XAnimParts();
void __cdecl Mark_XAnimPartsPtr();
void __cdecl Load_XBoneInfoArray(bool atStreamStart, int count);
void __cdecl Load_DObjAnimMatArray(bool atStreamStart, int count);
void __cdecl Load_StreamFileNameRaw(bool atStreamStart);
void __cdecl Load_StreamFileInfo(bool atStreamStart);
void __cdecl Load_StreamFileName(bool atStreamStart);
void __cdecl Load_LoadedSound(bool atStreamStart);
void __cdecl Load_LoadedSoundPtr(bool atStreamStart);
void __cdecl Load_StreamedSound(bool atStreamStart);
void __cdecl Load_SoundFileRef(bool atStreamStart);
void __cdecl Load_SoundFile(bool atStreamStart);
void __cdecl Load_SndCurve(bool atStreamStart);
void __cdecl Load_SndCurvePtr(bool atStreamStart);
void __cdecl Load_SpeakerMap(bool atStreamStart);
void __cdecl Load_snd_alias_t(bool atStreamStart);
void __cdecl Load_snd_alias_tArray(bool atStreamStart, int count);
void __cdecl Load_snd_alias_list_t(bool atStreamStart);
void __cdecl Load_snd_alias_list_ptr(bool atStreamStart);
void __cdecl Load_snd_alias_list_name(bool atStreamStart);
void __cdecl Load_snd_alias_list_nameArray(bool atStreamStart, int count);
void __cdecl Mark_LoadedSoundPtr();
void __cdecl Mark_SoundFileRef();
void __cdecl Mark_SoundFile();
void __cdecl Mark_SndCurvePtr();
void __cdecl Mark_snd_alias_t();
void __cdecl Mark_snd_alias_tArray(int count);
void __cdecl Mark_snd_alias_list_t();
void __cdecl Mark_snd_alias_list_ptr();
void __cdecl Mark_snd_alias_list_name();
void __cdecl Mark_snd_alias_list_nameArray(int count);
void __cdecl Load_MaterialInfo(bool atStreamStart);
void __cdecl Load_GfxWorldVertex0Array(bool atStreamStart, int count);
void __cdecl Load_GfxPackedVertex0Array(bool atStreamStart, int count);
void __cdecl Load_GfxBrushModelArray(bool atStreamStart, int count);
void __cdecl Load_XSurfaceCollisionLeafArray(bool atStreamStart, int count);
cbrush_t *__cdecl AllocLoad_GfxPackedVertex0();
void __cdecl Load_XSurfaceCollisionNodeArray(bool atStreamStart, int count);
void __cdecl Load_XSurfaceCollisionTree(bool atStreamStart);
void __cdecl Load_XRigidVertList(bool atStreamStart);
void __cdecl Load_XRigidVertListArray(bool atStreamStart, int count);
void __cdecl Load_GfxVertexBuffer(bool atStreamStart);
void __cdecl Load_XBlendInfoArray(bool atStreamStart, int count);
void __cdecl Load_XSurfaceVertexInfo(bool atStreamStart);
void __cdecl Load_r_index_tArray(bool atStreamStart, int count);
void __cdecl Load_r_index16_tArray(bool atStreamStart, int count);
void __cdecl Load_XSurface(bool atStreamStart);
void __cdecl Load_XSurfaceArray(bool atStreamStart, int count);
void __cdecl Load_GfxTextureLoad(bool atStreamStart);
void __cdecl Load_GfxRawTextureArray(bool atStreamStart, int count);
void __cdecl Load_GfxImageLoadDef(bool atStreamStart);
void __cdecl Load_GfxImage(bool atStreamStart);
void __cdecl Load_GfxImagePtr(bool atStreamStart);
void __cdecl Mark_GfxImagePtr();
void __cdecl Load_water_t(bool atStreamStart);
void __cdecl Mark_water_t();
void __cdecl Load_GfxVertexShaderLoadDef(bool atStreamStart);
void __cdecl Load_GfxPixelShaderLoadDef(bool atStreamStart);
void __cdecl Load_MaterialVertexShaderProgram(bool atStreamStart);
void __cdecl Load_MaterialPixelShaderProgram(bool atStreamStart);
void __cdecl Load_MaterialVertexShader(bool atStreamStart);
void __cdecl Load_MaterialVertexShaderPtr(bool atStreamStart);
void __cdecl Load_MaterialPixelShader(bool atStreamStart);
void __cdecl Load_MaterialPixelShaderPtr(bool atStreamStart);
void __cdecl Load_MaterialVertexDeclaration(bool atStreamStart);
void __cdecl Load_MaterialArgumentCodeConst(bool atStreamStart);
void __cdecl Load_MaterialArgumentDef(bool atStreamStart);
void __cdecl Load_MaterialShaderArgument(bool atStreamStart);
void __cdecl Load_MaterialShaderArgumentArray(bool atStreamStart, int count);
void __cdecl Load_GfxStateBitsArray(bool atStreamStart, int count);
void __cdecl Load_MaterialPass(bool atStreamStart);
void __cdecl Load_MaterialPassArray(bool atStreamStart, int count);
void __cdecl Load_MaterialTechnique(bool atStreamStart);
void __cdecl Load_MaterialTextureDefInfo(bool atStreamStart);
void __cdecl Load_MaterialTextureDef(bool atStreamStart);
void __cdecl Load_MaterialTextureDefArray(bool atStreamStart, int count);
void __cdecl Load_MaterialConstantDefArray(bool atStreamStart, int count);
void __cdecl Load_MaterialTechniquePtr(bool atStreamStart);
void __cdecl Load_MaterialTechniquePtrArray(bool atStreamStart, int count);
void __cdecl Load_MaterialTechniqueSet(bool atStreamStart);
void __cdecl Load_MaterialTechniqueSetPtr(bool atStreamStart);
void __cdecl Load_Material(bool atStreamStart);
void __cdecl Load_MaterialHandle(bool atStreamStart);
void __cdecl Load_MaterialHandleArray(bool atStreamStart, int count);
void __cdecl Mark_MaterialTextureDefInfo();
void __cdecl Mark_MaterialTextureDef();
void __cdecl Mark_MaterialTextureDefArray(int count);
void __cdecl Mark_MaterialTechniqueSetPtr();
void __cdecl Mark_Material();
void __cdecl Mark_MaterialHandle();
void __cdecl Mark_MaterialHandleArray(int count);
void __cdecl Load_GfxLightImage(bool atStreamStart);
void __cdecl Load_GfxLightDef(bool atStreamStart);
void __cdecl Load_GfxLightDefPtr(bool atStreamStart);
void __cdecl Load_GfxLight(bool atStreamStart);
void __cdecl Mark_GfxLightImage();
void __cdecl Mark_GfxLightDef();
void __cdecl Mark_GfxLightDefPtr();
void __cdecl Mark_GfxLight();
void __cdecl Load_GfxSurface(bool atStreamStart);
void __cdecl Load_GfxSurfaceArray(bool atStreamStart, int count);
void __cdecl Load_GfxLightmapArray(bool atStreamStart);
void __cdecl Load_GfxLightmapArrayArray(bool atStreamStart, int count);
void __cdecl Mark_GfxSurface();
void __cdecl Mark_GfxSurfaceArray(int count);
void __cdecl Mark_GfxLightmapArray();
void __cdecl Mark_GfxLightmapArrayArray(int count);
void __cdecl Load_PhysPreset(bool atStreamStart);
void __cdecl Load_PhysPresetPtr(bool atStreamStart);
void __cdecl Mark_PhysPresetPtr();
void __cdecl Load_cplane_t(bool atStreamStart);
void __cdecl Load_cplane_tArray(bool atStreamStart, int count);
void __cdecl Load_cbrushside_t(bool atStreamStart);
XAsset *__cdecl AllocLoad_FxElemVisStateSample();
void __cdecl Load_cbrushside_tArray(bool atStreamStart, int count);
void __cdecl Load_cbrushedge_t(bool atStreamStart);
void __cdecl Load_cbrushedge_tArray(bool atStreamStart, int count);
void __cdecl Load_XModelCollSurf(bool atStreamStart);
void __cdecl Load_XModelCollSurfArray(bool atStreamStart, int count);
void __cdecl Load_BrushWrapper(bool atStreamStart);
void __cdecl Load_PhysGeomInfo(bool atStreamStart);
void __cdecl Load_PhysGeomInfoArray(bool atStreamStart, int count);
void __cdecl Load_PhysGeomList(bool atStreamStart);
void __cdecl Load_XModel(bool atStreamStart);
void __cdecl Load_XModelPtr(bool atStreamStart);
void __cdecl Load_XModelPtrArray(bool atStreamStart, int count);
void __cdecl Load_XModelPiece(bool atStreamStart);
void __cdecl Load_XModelPieceArray(bool atStreamStart, int count);
void __cdecl Load_XModelPieces(bool atStreamStart);
void __cdecl Load_XModelPiecesPtr(bool atStreamStart);
void __cdecl Mark_XModel();
void __cdecl Mark_XModelPtr();
void __cdecl Mark_XModelPtrArray(int count);
void __cdecl Mark_XModelPiece();
void __cdecl Mark_XModelPieceArray(int count);
void __cdecl Mark_XModelPieces();
void __cdecl Mark_XModelPiecesPtr();
void __cdecl Load_pathlink_tArray(bool atStreamStart, int count);
void __cdecl Load_pathnode_constant_t(bool atStreamStart);
void __cdecl Load_pathnode_t(bool atStreamStart);
void __cdecl Load_pathnode_tArray(bool atStreamStart, int count);
void __cdecl Load_pathbasenode_tArray(bool atStreamStart, int count);
void __cdecl Load_pathnode_tree_nodes_t(bool atStreamStart);
void __cdecl Load_pathnode_tree_ptr(bool atStreamStart);
void __cdecl Load_pathnode_tree_ptrArray(bool atStreamStart, int count);
void __cdecl Load_pathnode_tree_info_t(bool atStreamStart);
void __cdecl Load_pathnode_tree_t(bool atStreamStart);
void __cdecl Load_pathnode_tree_tArray(bool atStreamStart, int count);
void __cdecl Mark_pathnode_constant_t();
void __cdecl Mark_pathnode_t();
void __cdecl Mark_pathnode_tArray(int count);
void __cdecl Load_PathData(bool atStreamStart);
void __cdecl Load_GameWorldSp(bool atStreamStart);
void __cdecl Load_GameWorldMp(bool atStreamStart);
void __cdecl Load_GameWorldSpPtr(bool atStreamStart);
void __cdecl Load_GameWorldMpPtr(bool atStreamStart);
void __cdecl Mark_PathData();
void __cdecl Mark_GameWorldSp();
void __cdecl Mark_GameWorldSpPtr();
void __cdecl Mark_GameWorldMpPtr();
void __cdecl Load_FxEffectDefHandle(bool atStreamStart);
void __cdecl Load_FxEffectDefHandleArray(bool atStreamStart, int count);
void __cdecl Load_FxEffectDefRef(bool atStreamStart);
void __cdecl Load_FxElemMarkVisuals(bool atStreamStart);
void __cdecl Load_FxElemMarkVisualsArray(bool atStreamStart, int count);
void __cdecl Load_FxElemVisuals(bool atStreamStart);
void __cdecl Load_FxElemVisualsArray(bool atStreamStart, int count);
void __cdecl Load_FxElemVisStateSampleArray(bool atStreamStart, int count);
void __cdecl Load_FxElemVelStateSampleArray(bool atStreamStart, int count);
void __cdecl Load_FxElemDefVisuals(bool atStreamStart);
void __cdecl Load_FxTrailVertexArray(bool atStreamStart, int count);
void __cdecl Load_FxTrailDef(bool atStreamStart);
void __cdecl Load_FxElemDef(bool atStreamStart);
void __cdecl Load_FxElemDefArray(bool atStreamStart, int count);
void __cdecl Load_FxEffectDef(bool atStreamStart);
void __cdecl Mark_FxEffectDefHandle();
void __cdecl Mark_FxEffectDefHandleArray(int count);
void __cdecl Mark_FxElemMarkVisuals();
void __cdecl Mark_FxElemMarkVisualsArray(int count);
void __cdecl Mark_FxElemVisuals();
void __cdecl Mark_FxElemVisualsArray(int count);
void __cdecl Mark_FxElemDefVisuals();
void __cdecl Mark_FxElemDef();
void __cdecl Mark_FxElemDefArray(int count);
void __cdecl Mark_FxEffectDef();
void __cdecl Load_DynEntityDef(bool atStreamStart);
void __cdecl Load_DynEntityDefArray(bool atStreamStart, int count);
void __cdecl Load_DynEntityCollArray(bool atStreamStart, int count);
void __cdecl Load_DynEntityPoseArray(bool atStreamStart, int count);
void __cdecl Load_DynEntityClientArray(bool atStreamStart, int count);
void __cdecl Mark_DynEntityDef();
void __cdecl Mark_DynEntityDefArray(int count);
void __cdecl Load_MapEnts(bool atStreamStart);
void __cdecl Load_MapEntsPtr(bool atStreamStart);
void __cdecl Mark_MapEntsPtr();
void __cdecl Load_cStaticModel_t(bool atStreamStart);
void __cdecl Load_cStaticModel_tArray(bool atStreamStart, int count);
void __cdecl Load_cNode_t(bool atStreamStart);
void __cdecl Load_cNode_tArray(bool atStreamStart, int count);
void __cdecl Load_cLeaf_tArray(bool atStreamStart, int count);
void __cdecl Load_cLeafBrushNodeLeaf_t(bool atStreamStart);
void __cdecl Load_cLeafBrushNodeChildren_t(bool atStreamStart);
void __cdecl Load_cLeafBrushNodeData_t(bool atStreamStart);
void __cdecl Load_cLeafBrushNode_t(bool atStreamStart);
void __cdecl Load_cLeafBrushNode_tArray(bool atStreamStart, int count);
void __cdecl Load_CollisionBorder(bool atStreamStart);
void __cdecl Load_CollisionBorderArray(bool atStreamStart, int count);
void __cdecl Load_CollisionPartition(bool atStreamStart);
void __cdecl Load_CollisionPartitionArray(bool atStreamStart, int count);
void __cdecl Load_CollisionAabbTreeArray(bool atStreamStart, int count);
void __cdecl Load_cmodel_tArray(bool atStreamStart, int count);
void __cdecl Load_cbrush_t(bool atStreamStart);
void __cdecl Load_cbrush_tArray(bool atStreamStart, int count);
void __cdecl Load_LeafBrushArray(bool atStreamStart, int count);
void __cdecl Load_clipMap_t(bool atStreamStart);
void __cdecl Load_clipMap_ptr(bool atStreamStart);
void __cdecl Mark_cStaticModel_t();
void __cdecl Mark_cStaticModel_tArray(int count);
void __cdecl Mark_clipMap_t();
void __cdecl Mark_clipMap_ptr();
void __cdecl Load_ComPrimaryLight(bool atStreamStart);
void __cdecl Load_ComPrimaryLightArray(bool atStreamStart, int count);
void __cdecl Load_ComWorld(bool atStreamStart);
void __cdecl Load_ComWorldPtr(bool atStreamStart);
void __cdecl Mark_ComWorldPtr();
void __cdecl Load_operandInternalDataUnion(bool atStreamStart);
void __cdecl Load_Operand(bool atStreamStart);
void __cdecl Load_Operator(bool atStreamStart);
void __cdecl Load_entryInternalData(bool atStreamStart);
void __cdecl Load_expressionEntry(bool atStreamStart);
void __cdecl Load_expressionEntry_ptr(bool atStreamStart);
void __cdecl Load_expressionEntry_ptrArray(bool atStreamStart, int count);
void __cdecl Load_statement(bool atStreamStart);
void __cdecl Load_listBoxDef_t(bool atStreamStart);
void __cdecl Load_listBoxDef_ptr(bool atStreamStart);
void __cdecl Load_editFieldDef_t(bool atStreamStart);
void __cdecl Load_editFieldDef_ptr(bool atStreamStart);
void __cdecl Load_multiDef_t(bool atStreamStart);
void __cdecl Load_multiDef_ptr(bool atStreamStart);
void __cdecl Load_windowDef_t(bool atStreamStart);
void __cdecl Load_Window(bool atStreamStart);
void __cdecl Load_ItemKeyHandler(bool atStreamStart);
void __cdecl Load_ItemKeyHandlerNext(bool atStreamStart);
void __cdecl Load_itemDefData_t(bool atStreamStart);
void __cdecl Load_itemDef_t(bool atStreamStart);
void __cdecl Load_itemDef_ptr(bool atStreamStart);
void __cdecl Load_itemDef_ptrArray(bool atStreamStart, int count);
void __cdecl Load_menuDef_t(bool atStreamStart);
void __cdecl Load_menuDef_ptr(bool atStreamStart);
void __cdecl Load_menuDef_ptrArray(bool atStreamStart, int count);
void __cdecl Load_MenuList(bool atStreamStart);
void __cdecl Load_MenuListPtr(bool atStreamStart);
void __cdecl Mark_listBoxDef_t();
void __cdecl Mark_listBoxDef_ptr();
void __cdecl Mark_windowDef_t();
void __cdecl Mark_Window();
void __cdecl Mark_itemDefData_t();
void __cdecl Mark_itemDef_t();
void __cdecl Mark_itemDef_ptr();
void __cdecl Mark_itemDef_ptrArray(int count);
void __cdecl Mark_menuDef_t();
void __cdecl Mark_menuDef_ptr();
void __cdecl Mark_menuDef_ptrArray(int count);
void __cdecl Mark_MenuList();
void __cdecl Mark_MenuListPtr();
void __cdecl Load_LocalizeEntry(bool atStreamStart);
void __cdecl Load_LocalizeEntryPtr(bool atStreamStart);
void __cdecl Mark_LocalizeEntryPtr();
void __cdecl Load_FxImpactEntry(bool atStreamStart);
void __cdecl Load_FxImpactEntryArray(bool atStreamStart, int count);
void __cdecl Load_FxImpactTable(bool atStreamStart);
void __cdecl Load_FxImpactTablePtr(bool atStreamStart);
void __cdecl Mark_FxImpactEntry();
void __cdecl Mark_FxImpactEntryArray(int count);
void __cdecl Mark_FxImpactTable();
void __cdecl Mark_FxImpactTablePtr();
void __cdecl Load_WeaponDef(bool atStreamStart);
void __cdecl Load_WeaponDefPtr(bool atStreamStart);
void __cdecl Mark_WeaponDef();
void __cdecl Mark_WeaponDefPtr();
void __cdecl Load_RawFile(bool atStreamStart);
void __cdecl Load_RawFilePtr(bool atStreamStart);
void __cdecl Mark_RawFilePtr();
void __cdecl Load_StringTable(bool atStreamStart);
void __cdecl Load_StringTablePtr(bool atStreamStart);
void __cdecl Mark_StringTablePtr();
void __cdecl Load_GfxStaticModelDrawInst(bool atStreamStart);
void __cdecl Load_GfxStaticModelDrawInstArray(bool atStreamStart, int count);
void __cdecl Load_GfxStaticModelInstArray(bool atStreamStart, int count);
void __cdecl Mark_GfxStaticModelDrawInst();
void __cdecl Mark_GfxStaticModelDrawInstArray(int count);
void __cdecl Load_sunflare_t(bool atStreamStart);
void __cdecl Mark_sunflare_t();
void __cdecl Load_GfxReflectionProbe(bool atStreamStart);
void __cdecl Load_GfxReflectionProbeArray(bool atStreamStart, int count);
void __cdecl Mark_GfxReflectionProbe();
void __cdecl Mark_GfxReflectionProbeArray(int count);
void __cdecl Load_StaticModelIndexArray(bool atStreamStart, int count);
void __cdecl Load_GfxAabbTree(bool atStreamStart);
void __cdecl Load_GfxAabbTreeArray(bool atStreamStart, int count);
void __cdecl Load_GfxCell(bool atStreamStart);
void __cdecl Load_GfxCellArray(bool atStreamStart, int count);
void __cdecl Load_GfxPortal(bool atStreamStart);
void __cdecl Load_GfxPortalArray(bool atStreamStart, int count);
void __cdecl Load_GfxCullGroupArray(bool atStreamStart, int count);
void __cdecl Load_GfxLightGridEntryArray(bool atStreamStart, int count);
void __cdecl Load_GfxLightGridColorsArray(bool atStreamStart, int count);
void __cdecl Load_MaterialMemory(bool atStreamStart);
void __cdecl Load_MaterialMemoryArray(bool atStreamStart, int count);
void __cdecl Load_GfxWorldVertexData(bool atStreamStart);
void __cdecl Load_GfxWorldVertexLayerData(bool atStreamStart);
void __cdecl Load_GfxLightGrid(bool atStreamStart);
void __cdecl Load_GfxSceneDynModelArray(bool atStreamStart, int count);
void __cdecl Load_GfxSceneDynBrushArray(bool atStreamStart, int count);
void __cdecl Load_GfxDrawSurfArray(bool atStreamStart, int count);
void __cdecl Load_GfxShadowGeometry(bool atStreamStart);
void __cdecl Load_GfxShadowGeometryArray(bool atStreamStart, int count);
void __cdecl Load_GfxLightRegionAxisArray(bool atStreamStart, int count);
void __cdecl Load_GfxLightRegionHull(bool atStreamStart);
void __cdecl Load_GfxLightRegionHullArray(bool atStreamStart, int count);
void __cdecl Load_GfxLightRegion(bool atStreamStart);
void __cdecl Load_GfxLightRegionArray(bool atStreamStart, int count);
void __cdecl Load_GfxWorldDpvsDynamic(bool atStreamStart);
void __cdecl Load_GfxWorldDpvsStatic(bool atStreamStart);
void __cdecl Load_GfxWorldDpvsPlanes(bool atStreamStart);
void __cdecl Load_GfxWorld(bool atStreamStart);
void __cdecl Load_GfxWorldPtr(bool atStreamStart);
void __cdecl Mark_MaterialMemory();
void __cdecl Mark_MaterialMemoryArray(int count);
void __cdecl Mark_GfxWorldDpvsStatic();
void __cdecl Mark_GfxWorld();
void __cdecl Mark_GfxWorldPtr();
void __cdecl Load_GlyphArray(bool atStreamStart, int count);
void __cdecl Load_Font(bool atStreamStart);
void __cdecl Load_FontHandle(bool atStreamStart);
void __cdecl Mark_Font();
void __cdecl Mark_FontHandle();
void __cdecl Load_XAssetHeader(bool atStreamStart);
void __cdecl Load_XAsset(bool atStreamStart);
void __cdecl Mark_XAssetHeader();
void __cdecl Mark_XAsset();
void __cdecl Mark_SndAliasCustom(snd_alias_list_t **var);
void __cdecl DB_LoadDObjs();

extern XAssetEntry *g_copyInfo[2048];
extern unsigned int g_copyInfoCount;

extern volatile unsigned int g_loadingAssets;

extern XAssetList *varXAssetList;

fileData_s *com_fileDataHashTable[1024];

extern unsigned int volatile g_mainThreadBlocked;

extern unsigned int g_streamDelayIndex;
extern XBlock *g_streamBlocks;
extern unsigned __int8 *g_streamPosArray[9];
extern StreamDelayInfo g_streamDelayArray[4096];
extern unsigned int g_streamPosIndex;
extern StreamPosInfo g_streamPosStack[64];
extern XZoneMemory *g_streamZoneMem;
extern unsigned __int8 *g_streamPos;
extern unsigned int g_streamPosStackIndex;

extern XAsset *varXAsset;

extern FastCriticalSection db_hashCritSect;
