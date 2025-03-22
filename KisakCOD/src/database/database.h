#pragma once
#include <xanim/xanim.h>
#include <win32/win_local.h>
#include <cstddef>

struct XModelPiece // sizeof=0x10
{
    XModel *model;
    float offset[3];
};
struct XModelPieces // sizeof=0xC
{                                       // ...
    const char *name;
    int numpieces;
    XModelPiece *pieces;
};

union XAssetHeader // sizeof=0x4
{                                       // ...
    XAssetHeader() { data = NULL; }
    // LWSS: This is used for lots of places in db_registry
    XAssetHeader(XModelPieces *arg) { xmodelPieces = arg; }
    XAssetHeader(PhysPreset *arg) { physPreset = arg; }
    XAssetHeader(XAnimParts *arg) { parts = arg; }
    XAssetHeader(XModel *arg) { model = arg; }
    XAssetHeader(Material *arg) { material = arg; }
    XAssetHeader(MaterialPixelShader *arg) { pixelShader = arg; }
    XAssetHeader(MaterialVertexShader *arg) { vertexShader = arg; }
    XAssetHeader(MaterialTechniqueSet *arg) { techniqueSet = arg; }
    XAssetHeader(GfxImage *arg) { image = arg; }
    XAssetHeader(snd_alias_list_t *arg) { sound = arg; }
    XAssetHeader(SndCurve *arg) { sndCurve = arg; }
    XAssetHeader(LoadedSound *arg) { loadSnd = arg; }
    XAssetHeader(clipMap_t *arg) { clipMap = arg; }
    XAssetHeader(ComWorld *arg) { comWorld = arg; }
    XAssetHeader(GameWorldSp *arg) { gameWorldSp = arg; }
    XAssetHeader(GameWorldMp *arg) { gameWorldMp = arg; }
    XAssetHeader(MapEnts *arg) { mapEnts = arg; }
    XAssetHeader(GfxWorld *arg) { gfxWorld = arg; }
    XAssetHeader(GfxLightDef *arg) { lightDef = arg; }
    XAssetHeader(Font_s *arg) { font = arg; }
    XAssetHeader(MenuList *arg) { menuList = arg; }
    XAssetHeader(menuDef_t *arg) { menu = arg; }
    XAssetHeader(LocalizeEntry *arg) { localize = arg; }
    XAssetHeader(WeaponDef *arg) { weapon = arg; }
    XAssetHeader(SndDriverGlobals *arg) { sndDriverGlobals = arg; }
    XAssetHeader(const FxEffectDef *arg) { fx = arg; }
    XAssetHeader(FxImpactTable *arg) { impactFx = arg; }
    XAssetHeader(RawFile *arg) { rawfile = arg; }
    XAssetHeader(StringTable *arg) { stringTable = arg; }
    XAssetHeader(void *arg) { data = arg; }

    XModelPieces *xmodelPieces;
    PhysPreset *physPreset;
    XAnimParts *parts;
    XModel *model;
    Material *material;
    MaterialPixelShader *pixelShader;
    MaterialVertexShader *vertexShader;
    MaterialTechniqueSet *techniqueSet;
    GfxImage *image;
    snd_alias_list_t *sound;
    SndCurve *sndCurve;
    LoadedSound *loadSnd;
    clipMap_t *clipMap;
    ComWorld *comWorld;
    GameWorldSp *gameWorldSp;
    GameWorldMp *gameWorldMp;
    MapEnts *mapEnts;
    GfxWorld *gfxWorld;
    GfxLightDef *lightDef;
    Font_s *font;
    MenuList *menuList;
    menuDef_t *menu;
    LocalizeEntry *localize;
    WeaponDef *weapon;
    SndDriverGlobals *sndDriverGlobals;
    const FxEffectDef *fx;
    FxImpactTable *impactFx;
    RawFile *rawfile;
    StringTable *stringTable;
    void *data;
};

struct XAsset // sizeof=0x8
{                                       // ...
    XAssetType type;                    // ...
    XAssetHeader header;                // ...
};

template <typename T>
union XAssetPoolEntry // sizeof=0x10
{                                       // ...
    StringTable entry;
    XAssetPoolEntry<T> *next;
};

template <typename T, int LEN>
struct XAssetPool
{
    XAssetPoolEntry<T> *freeHead;
    XAssetPoolEntry<T> entries[LEN];
};

struct XAssetEntry // sizeof=0x10
{                                       // ...
    XAsset asset;                       // ...
    unsigned __int8 zoneIndex;
    bool inuse;
    unsigned __int16 nextHash;
    unsigned __int16 nextOverride;
    unsigned __int16 usageFrame;
};

union XAssetEntryPoolEntry // sizeof=0x10
{                                       // ...
    XAssetEntry entry;
    XAssetEntryPoolEntry *next;
};

struct XZoneInfo // sizeof=0xC
{                                       // ...
    const char *name;                   // ...
    int allocFlags;                     // ...
    int freeFlags;                      // ...
};

struct XBlock // sizeof=0x8
{                                       // ...
    unsigned __int8 *data;
    unsigned int size;
};
struct XZoneMemory // sizeof=0x58
{                                       // ...
    XBlock blocks[9];
    unsigned __int8 *lockedVertexData;
    unsigned __int8 *lockedIndexData;
    void *vertexBuffer;
    void *indexBuffer;
};

struct __declspec(align(4)) XZone // sizeof=0xA8
{                                       // ...
    char name[64];                      // ...
    int flags;                          // ...
    int allocType;
    XZoneMemory mem;                    // ...
    int fileSize;                       // ...
    bool modZone;                       // ...
    // padding byte
    // padding byte
    // padding byte
};

enum inflate_block_mode : __int32
{                                       // ...
    TYPE = 0x0,
    LENS = 0x1,
    STORED = 0x2,
    TABLE = 0x3,
    BTREE = 0x4,
    DTREE = 0x5,
    CODES = 0x6,
    DRY = 0x7,
    DONE = 0x8,
    BAD = 0x9,
};
typedef inflate_block_mode inflate_mode; // ...


struct inflate_blocks_state // sizeof=0x4
{
    int dummy;
};
struct internal_state // sizeof=0x18
{
    inflate_mode mode;
    //$1F0878CB33AB691FEEFD309FEF732A16 sub;
    union //$1F0878CB33AB691FEEFD309FEF732A16 // sizeof=0x8
    {                                       // ...
        unsigned int method;
        //$A1AD60A7CB977A1DBF6E2660B1E40DE7 check;
        struct //$A1AD60A7CB977A1DBF6E2660B1E40DE7
        {
            unsigned int was;
            unsigned int need;
        };
        unsigned int marker;
    };

    int nowrap;
    unsigned int wbits;
    inflate_blocks_state *blocks;
};

struct z_stream_s // sizeof=0x34
{                                       // ...
    unsigned __int8 *next_in;           // ...
    unsigned int avail_in;              // ...
    unsigned int total_in;
    unsigned __int8 *next_out;          // ...
    unsigned int avail_out;             // ...
    unsigned int total_out;
    char *msg;
    internal_state *state;
    unsigned __int8 *(__cdecl *zalloc)(unsigned __int8 *, unsigned int, unsigned int);
    void(__cdecl *zfree)(unsigned __int8 *, unsigned __int8 *);
    unsigned __int8 *opaque;
    int data_type;
    unsigned int adler;
};

struct ScriptStringList // sizeof=0x8
{                                       // ...
    int count;
    const char **strings;
};
struct XAssetList // sizeof=0x10
{                                       // ...
    ScriptStringList stringList;
    int assetCount;
    XAsset *assets;
};

// db_registry
void __cdecl TRACK_db_registry();
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
void __cdecl DB_RemoveClipMap();
void __cdecl Load_ComWorldAsset(XAssetHeader *comWorld);
void __cdecl Mark_ComWorldAsset(ComWorld *comWorld);
void __cdecl DB_RemoveComWorld();
void __cdecl Load_GameWorldSpAsset(XAssetHeader *gameWorldSp);
void __cdecl Mark_GameWorldSpAsset(GameWorldSp *gameWorldSp);
void __cdecl Load_GameWorldMpAsset(XAssetHeader *gameWorldMp);
void __cdecl Mark_GameWorldMpAsset(GameWorldMp *gameWorldMp);
void __cdecl Load_MapEntsAsset(XAssetHeader *mapEnts);
void __cdecl Mark_MapEntsAsset(MapEnts *mapEnts);
void __cdecl Load_GfxWorldAsset(XAssetHeader *gfxWorld);
void __cdecl Mark_GfxWorldAsset(GfxWorld *gfxWorld);
void __cdecl DB_RemoveGfxWorld();
void __cdecl Load_LightDefAsset(XAssetHeader *lightDef);
void __cdecl Mark_LightDefAsset(GfxLightDef *lightDef);
void __cdecl Load_FontAsset(XAssetHeader *font);
void __cdecl Mark_FontAsset(Font_s *font);
void __cdecl Load_MenuListAsset(XAssetHeader *menuList);
void __cdecl Mark_MenuListAsset(MenuList *menuList);
void __cdecl Load_MenuAsset(XAssetHeader *menu);
void __cdecl Mark_MenuAsset(menuDef_t *menu);
void __cdecl DB_DynamicCloneMenu(XAssetHeader from, XAssetHeader to);
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
void __cdecl Sys_LockWrite(FastCriticalSection *critSect);
void __cdecl Sys_UnlockWrite(FastCriticalSection *critSect);
void __cdecl DB_Sleep(unsigned int msec);
void __cdecl DB_LogMissingAsset(XAssetType type, const char *name);
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



struct DB_LoadData // sizeof=0x68
{                                       // ...
    void *f;                            // ...
    const char *filename;               // ...
    XZoneMemory *zoneMem;               // ...
    int outstandingReads;               // ...
    _OVERLAPPED overlapped;             // ...
    z_stream_s stream;                  // ...
    unsigned __int8 *compressBufferStart; // ...
    unsigned __int8 *compressBufferEnd; // ...
    void(__cdecl *interrupt)();        // ...
    int allocType;                      // ...
};
struct XFile // sizeof=0x2C
{                                       // ...
    unsigned int size;
    unsigned int externalSize;          // ...
    unsigned int blockSize[9];          // ...
};

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



extern XAssetList *varXAssetList;