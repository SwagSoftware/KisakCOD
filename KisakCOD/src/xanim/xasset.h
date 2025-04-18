#pragma once

#include "xanim.h"

extern "C" {
    // win32
    struct _OVERLAPPED;
}

// todo: this sucks

struct cbrush_t;
struct clipMap_t;
struct menuDef_t;
struct snd_alias_list_t;
struct windowDef_t;

struct ComWorld;

struct Font_s;
struct FxEffectDef;
struct FxImpactTable;

struct GameWorldSp;
struct GameWorldMp;

struct PhysPreset;

struct LoadedSound;
struct LocalizeEntry;

struct GfxPackedVertex;
struct GfxImage;
struct GfxLightDef;
struct GfxWorld;

struct MapEnts;

struct Material;
struct MaterialPixelShader;
struct MaterialVertexShader;
struct MaterialTechniqueSet;

struct MenuList;

struct RawFile;

struct StringTable;
struct SndCurve;
struct SndDriverGlobals;

struct WeaponDef;

struct XAnimParts;
struct XModel;
struct XModelPiece;
struct XModelPieces;

union XAssetHeader // sizeof=0x4
{                                       // ...
    XAssetHeader() { data = NULL; }
    // LWSS: This is used for lots of places in db_registry
    // XAssetHeader(XModelPieces *arg) { xmodelPieces = arg; }
    // XAssetHeader(PhysPreset *arg) { physPreset = arg; }
    // XAssetHeader(XAnimParts *arg) { parts = arg; }
    // XAssetHeader(XModel *arg) { model = arg; }
    // XAssetHeader(Material *arg) { material = arg; }
    // XAssetHeader(MaterialPixelShader *arg) { pixelShader = arg; }
    // XAssetHeader(MaterialVertexShader *arg) { vertexShader = arg; }
    // XAssetHeader(MaterialTechniqueSet *arg) { techniqueSet = arg; }
    // XAssetHeader(GfxImage *arg) { image = arg; }
    // XAssetHeader(snd_alias_list_t *arg) { sound = arg; }
    // XAssetHeader(SndCurve *arg) { sndCurve = arg; }
    // XAssetHeader(LoadedSound *arg) { loadSnd = arg; }
    // XAssetHeader(clipMap_t *arg) { clipMap = arg; }
    // XAssetHeader(ComWorld *arg) { comWorld = arg; }
    // XAssetHeader(GameWorldSp *arg) { gameWorldSp = arg; }
    // XAssetHeader(GameWorldMp *arg) { gameWorldMp = arg; }
    // XAssetHeader(MapEnts *arg) { mapEnts = arg; }
    // XAssetHeader(GfxWorld *arg) { gfxWorld = arg; }
    // XAssetHeader(GfxLightDef *arg) { lightDef = arg; }
    // XAssetHeader(Font_s *arg) { font = arg; }
    // XAssetHeader(MenuList *arg) { menuList = arg; }
    // XAssetHeader(menuDef_t *arg) { menu = arg; }
    // XAssetHeader(LocalizeEntry *arg) { localize = arg; }
    // XAssetHeader(WeaponDef *arg) { weapon = arg; }
    // XAssetHeader(SndDriverGlobals *arg) { sndDriverGlobals = arg; }
    // XAssetHeader(const FxEffectDef *arg) { fx = arg; }
    // XAssetHeader(FxImpactTable *arg) { impactFx = arg; }
    // XAssetHeader(RawFile *arg) { rawfile = arg; }
    // XAssetHeader(StringTable *arg) { stringTable = arg; }
    XAssetHeader(void* arg) { data = arg; }

    XModelPieces* xmodelPieces;
    PhysPreset* physPreset;
    XAnimParts* parts;
    XModel* model;
    Material* material;
    MaterialPixelShader* pixelShader;
    MaterialVertexShader* vertexShader;
    MaterialTechniqueSet* techniqueSet;
    GfxImage* image;
    snd_alias_list_t* sound;
    SndCurve* sndCurve;
    LoadedSound* loadSnd;
    clipMap_t* clipMap;
    ComWorld* comWorld;
    GameWorldSp* gameWorldSp;
    GameWorldMp* gameWorldMp;
    MapEnts* mapEnts;
    GfxWorld* gfxWorld;
    GfxLightDef* lightDef;
    Font_s* font;
    MenuList* menuList;
    menuDef_t* menu;
    LocalizeEntry* localize;
    WeaponDef* weapon;
    SndDriverGlobals* sndDriverGlobals;
    const FxEffectDef* fx;
    FxImpactTable* impactFx;
    RawFile* rawfile;
    StringTable* stringTable;
    void* data;
};

enum XAssetType : __int32
{
    ASSET_TYPE_XMODELPIECES = 0x0,
    ASSET_TYPE_PHYSPRESET = 0x1,
    ASSET_TYPE_XANIMPARTS = 0x2,
    ASSET_TYPE_XMODEL = 0x3,
    ASSET_TYPE_MATERIAL = 0x4,
    ASSET_TYPE_TECHNIQUE_SET = 0x5,
    ASSET_TYPE_IMAGE = 0x6,
    ASSET_TYPE_SOUND = 0x7,
    ASSET_TYPE_SOUND_CURVE = 0x8,
    ASSET_TYPE_LOADED_SOUND = 0x9,
    ASSET_TYPE_CLIPMAP = 0xA,
    ASSET_TYPE_CLIPMAP_PVS = 0xB,
    ASSET_TYPE_COMWORLD = 0xC,
    ASSET_TYPE_GAMEWORLD_SP = 0xD,
    ASSET_TYPE_GAMEWORLD_MP = 0xE,
    ASSET_TYPE_MAP_ENTS = 0xF,
    ASSET_TYPE_GFXWORLD = 0x10,
    ASSET_TYPE_LIGHT_DEF = 0x11,
    ASSET_TYPE_UI_MAP = 0x12,
    ASSET_TYPE_FONT = 0x13,
    ASSET_TYPE_MENULIST = 0x14,
    ASSET_TYPE_MENU = 0x15,
    ASSET_TYPE_LOCALIZE_ENTRY = 0x16,
    ASSET_TYPE_WEAPON = 0x17,
    ASSET_TYPE_SNDDRIVER_GLOBALS = 0x18,
    ASSET_TYPE_FX = 0x19,
    ASSET_TYPE_IMPACT_FX = 0x1A,
    ASSET_TYPE_AITYPE = 0x1B,
    ASSET_TYPE_MPTYPE = 0x1C,
    ASSET_TYPE_CHARACTER = 0x1D,
    ASSET_TYPE_XMODELALIAS = 0x1E,
    ASSET_TYPE_RAWFILE = 0x1F,
    ASSET_TYPE_STRINGTABLE = 0x20,
    ASSET_TYPE_COUNT = 0x21,
    ASSET_TYPE_STRING = 0x21,
    ASSET_TYPE_ASSETLIST = 0x22,
};
XAssetType &operator++(XAssetType &e) {
    static_cast<XAssetType>(static_cast<int>(e) + 1);
    return e;
}
XAssetType &operator++(XAssetType &e, int i)
{
    XAssetType temp = e;
    ++e;
    return temp;
}

struct XAsset // sizeof=0x8
{                                       // ...
    XAssetType type;                    // ...
    XAssetHeader header;                // ...
};

union XAssetSize // sizeof=0x878
{                                       // ...
    XAssetSize()
    {
        fx = NULL;
    }
    XAnimParts parts;
    XModel model;
    Material material;
    MaterialPixelShader pixelShader;
    MaterialVertexShader vertexShader;
    MaterialTechniqueSet techniqueSet;
    GfxImage image;
    snd_alias_list_t sound;
    SndCurve sndCurve;
    clipMap_t clipMap;
    ComWorld comWorld;
    MapEnts mapEnts;
    GfxWorld gfxWorld;
    GfxLightDef lightDef;
    Font_s font;
    MenuList menuList;
    menuDef_t menu;
    LocalizeEntry localize;
    WeaponDef weapon;
    SndDriverGlobals sndDriverGlobals;
    const FxEffectDef *fx;
    FxImpactTable impactFx;
    RawFile rawfile;
    StringTable stringTable;
};

template <typename T>
union XAssetPoolEntry // sizeof=0x10
{                                       // ...
    StringTable entry;
    XAssetPoolEntry<T>* next;
};

template <typename T, int LEN>
struct XAssetPool
{
    XAssetPoolEntry<T>* freeHead;
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
    XAssetEntryPoolEntry* next;
};

struct XZoneInfo // sizeof=0xC
{                                       // ...
    const char* name;                   // ...
    int allocFlags;                     // ...
    int freeFlags;                      // ...
};

struct XBlock // sizeof=0x8
{                                       // ...
    unsigned __int8* data;
    unsigned int size;
};

struct XZoneMemory // sizeof=0x58
{                                       // ...
    XBlock blocks[9];
    unsigned __int8* lockedVertexData;
    unsigned __int8* lockedIndexData;
    void* vertexBuffer;
    void* indexBuffer;
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

struct ScriptStringList // sizeof=0x8
{                                       // ...
    int count;
    const char** strings;
};

struct XAssetList // sizeof=0x10
{                                       // ...
    ScriptStringList stringList;
    int assetCount;
    XAsset* assets;
};

struct XFile // sizeof=0x2C
{                                       // ...
    unsigned int size;
    unsigned int externalSize;          // ...
    unsigned int blockSize[9];          // ...
};

struct XSurfaceCollisionAabb // sizeof=0xC
{                                       // ...
    unsigned __int16 mins[3];
    unsigned __int16 maxs[3];
};

struct XSurfaceCollisionNode // sizeof=0x10
{
    XSurfaceCollisionAabb aabb;
    unsigned __int16 childBeginIndex;
    unsigned __int16 childCount;
};

struct XSurfaceCollisionLeaf // sizeof=0x2
{
    unsigned __int16 triangleBeginIndex;
};

struct XSurfaceCollisionTree // sizeof=0x28
{
    float trans[3];
    float scale[3];
    unsigned int nodeCount;
    XSurfaceCollisionNode* nodes;
    unsigned int leafCount;
    XSurfaceCollisionLeaf* leafs;
};
struct XRigidVertList // sizeof=0xC
{                                       // ...
    unsigned __int16 boneOffset;        // ...
    unsigned __int16 vertCount;         // ...
    unsigned __int16 triOffset;         // ...
    unsigned __int16 triCount;          // ...
    XSurfaceCollisionTree* collisionTree;
};

struct XSurfaceVertexInfo // sizeof=0xC
{                                       // ...
    __int16 vertCount[4];
    unsigned __int16* vertsBlend;
};

struct XSurface // sizeof=0x38
{
    unsigned __int8 tileMode;
    bool deformed;
    unsigned __int16 vertCount;
    unsigned __int16 triCount;
    unsigned __int8 zoneHandle;
    // padding byte
    unsigned __int16 baseTriIndex;
    unsigned __int16 baseVertIndex;
    unsigned __int16* triIndices;
    XSurfaceVertexInfo vertInfo;
    GfxPackedVertex* verts0;
    unsigned int vertListCount;
    XRigidVertList* vertList;
    int partBits[4];
};
