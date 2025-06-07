#include "database.h"

#include <xanim/xanim.h>
#include <xanim/xmodel.h>

#include <sound/snd_local.h>

#include <gfx_d3d/fxprimitives.h>
#include <gfx_d3d/r_material.h>
#include <gfx_d3d/r_gfx.h>
#include <xanim/dobj.h>
#include <gfx_d3d/r_buffers.h>

#include <DynEntity/DynEntity_client.h>
#include <gfx_d3d/r_water.h>
#include <gfx_d3d/r_image.h>
#include <universal/com_sndalias.h>
#include <gfx_d3d/r_sky.h>
#include <gfx_d3d/r_primarylights.h>

struct DynEntityServer // sizeof=0x24
{
    GfxPlacement pose;
    unsigned __int16 flags;
    // padding byte
    // padding byte
    int health;
};

void *varint;
void *varuint;
GfxVertex *varGfxVertex;
unsigned __int64 *varuint64_t           ;
int *varexpressionEntryType     ;
float *varfloat               ;
ComWorld **varComWorldPtr     ;
enum weapInventoryType_t *varweapInventoryType_t     ;
RawFile **varRawFilePtr     ;
GfxLightDef **varGfxLightDefPtr     ;
GfxLightmapArray *varGfxLightmapArray     ;
itemDef_s **varitemDef_ptr     ;
GameWorldSp **varGameWorldSpPtr     ;
XAnimDeltaPartQuat *varXAnimDeltaPartQuat     ;
clipMap_t *varclipMap_t     ;
MaterialPixelShaderProgram *varMaterialPixelShaderProgram     ;
GfxWorldStreamInfo *varGfxWorldStreamInfo     ;
char const * varConstChar           ;
unsigned short *varr_index16_t         ;
MenuList *varMenuList     ;
listBoxDef_s *varlistBoxDef_t     ;
Operand *varOperand     ;
DObjAnimMat *varDObjAnimMat     ;
unsigned long *varXAUDIOSAMPLERATE     ;
mnode_t *varmnode_t     ;
union FxElemDefVisuals *varFxElemDefVisuals     ;
XModelCollSurf_s *varXModelCollSurf     ;
XModelCollTri_s *varXModelCollTri;
DynEntityServer *varDynEntityServer     ;
MaterialStreamRouting *varMaterialStreamRouting     ;
GfxScaledPlacement *varGfxScaledPlacement     ;
FxFloatRange *varFxFloatRange     ;
short *varint16_t             ;
GfxWorld *varGfxWorld     ;
GfxPortal *varGfxPortal     ;
CardMemory *varCardMemory     ;
//XAUDIOFXDATPARAM *varXAUDIOFXDATAPARAM     ;
LocalizeEntry *varLocalizeEntry     ;
MenuList **varMenuListPtr     ;
unsigned int *varunsigned            ;
//XAUDIOCHANNELMAPENTRY *varXAUDIOCHANNELMAPENTRY     ;
MaterialTechnique *varMaterialTechnique     ;
enum MapType *varMapType     ;
sunflare_t *varsunflare_t     ;
PhysPreset *varPhysPreset     ;
//D3DCubeTexture *varIDirect3DCubeTexture9     ;
unsigned __int8 *varXQuat2           ;
//unsigned short (*)[3] varedgeCount_t      ;
Material **varMaterialHandle     ;
//XAUDIOREVERBSETTINGS *varXAUDIOREVERBSETTINGS     ;
pathnode_t *varpathnode_t     ;
unsigned char *varbyte16              ;
StreamFileName *varStreamFileName     ;
XAnimPartTrans *varXAnimPartTrans     ;
enum weapOverlayReticle_t *varweapOverlayReticle_t     ;
unsigned short *varushort              ;
float *varraw_float           ;
unsigned char *varbyte4096            ;
unsigned short *varDynEntityId         ;
clipMap_t **varclipMap_ptr     ;
GfxLightRegionHull *varGfxLightRegionHull     ;
unsigned char *varbyte128             ;
XModel **varXModelPtr     ;
enum XAssetType *varXAssetType     ;
enum weapType_t *varweapType_t     ;
MaterialPass *varMaterialPass     ;
GfxCell *varGfxCell     ;
enum weapPositionAnimNum_t *varweapPositionAnimNum_t     ;
pathlink_s *varpathlink_t     ;
FxElemMarkVisuals *varFxElemMarkVisuals     ;
unsigned char *varXAUDIOSAMPLETYPE     ;
union XAnimPartTransData *varXAnimPartTransData     ;
Font_s *varFont        ;
SndCurve *varSndCurve     ;
editFieldDef_s *vareditFieldDef_t     ;
XSurfaceCollisionNode *varXSurfaceCollisionNode     ;
GfxSceneDynBrush *varGfxSceneDynBrush     ;
pathnode_constant_t *varpathnode_constant_t     ;
cmodel_t *varcmodel_t     ;
unsigned char *varFxElemType          ;
XBoneInfo *varXBoneInfo     ;
FxImpactTable **varFxImpactTablePtr     ;
float *varXAUDIOVOLUME        ;
//XaReverbSettings *varXaReverbSettings     ;
unsigned __int8 *varvec2_           ;
float (*varvec2_t)[2];
FxElemAtlas *varFxElemAtlas     ;
MaterialVertexStreamRouting *varMaterialVertexStreamRouting     ;
CollisionPartition *varCollisionPartition     ;
union XAnimIndices *varXAnimIndices     ;
XAsset *varXAsset      ;
snd_alias_list_t **varsnd_alias_list_ptr     ;
unsigned int *varuint32_t            ;
unsigned char **varGfxImagePixels     ;
enum weapStance_t *varweapStance_t     ;
pathnode_tree_t **varpathnode_tree_ptr     ;
MaterialShaderArgument *varMaterialShaderArgument     ;
WeaponDef *varWeaponDef     ;
enum expDataType *varoperandDataType     ;
//int (*)[4] varXPartBits        ;
ComPrimaryLight *varComPrimaryLight     ;
MaterialTextureDef *varMaterialTextureDef     ;
BOOL * varbool               ;
unsigned short *varUnsignedShort       ;
union MaterialArgumentDef *varMaterialArgumentDef     ;
Glyph *varGlyph        ;
//StreamFileNamePacked *varStreamFileNamePacked     ;
XModelLodInfo *varXModelLodInfo     ;
enum ammoCounterClipType_t *varammoCounterClipType_t     ;
unsigned short *varLeafBrush           ;
//XAUDIOCHANNELMAP *varXAUDIOCHANNELMAP     ;
enum nodeType *varnodeType     ;
columnInfo_s *varcolumnInfo_t     ;
enum snd_alias_type_t *varsnd_alias_type_t     ;
enum activeReticleType_t *varactiveReticleType_t     ;
GfxLightGridEntry *varGfxLightGridEntry     ;
ItemKeyHandler *varItemKeyHandler     ;
union XAUDIOFXPARAM *varXAUDIOFXPARAM     ;
union StreamFileInfo *varStreamFileInfo     ;
GfxPackedVertex *varGfxPackedVertex     ;
cLeaf_t *varcLeaf_t     ;
union FxEffectDefRef *varFxEffectDefRef     ;
unsigned char *varbyteShader          ;
enum WeapAccuracyType *varWeapAccuracyType     ;
unsigned char *varbyte                ;
FxTrailVertex *varFxTrailVertex     ;
//XAUDIOXMAFORMAT *varXAUDIOXMAFORMAT     ;
char const **varTempString        ;
StringTable **varStringTablePtr     ;
//float (*)[4] varraw_vec4_t       ;
unsigned __int8 *varUShortVec        ;
statement_s *varstatement     ;
//D3DVolumeTexture *varIDirect3DVolumeTexture9     ;
GfxLightGridColors *varGfxLightGridColors     ;
enum operationEnum *varOperator     ;
cLeafBrushNodeLeaf_t *varcLeafBrushNodeLeaf_t     ;
multiDef_s **varmultiDef_ptr     ;
XRigidVertList *varXRigidVertList     ;
DpvsPlane *varDpvsPlane     ;
//short (*)[3] varAxialMaterialNum     ;
XModelPiece *varXModelPiece     ;
XModelPieces **varXModelPiecesPtr     ;
union XAssetHeader *varXAssetHeader     ;
CollisionAabbTree *varCollisionAabbTree     ;
cplane_s *varcplane_t     ;
union operandInternalDataUnion *varoperandInternalDataUnion     ;
short *varXQuat[4]            ;
expressionEntry *varexpressionEntry     ;
XAssetList *varXAssetList     ;
enum weapClass_t *varweapClass_t     ;
enum MaterialWorldVertexFormat *varMaterialWorldVertexFormat     ;
MaterialPixelShader **varMaterialPixelShaderPtr     ;
unsigned __int8 * varvec4_t           ;
char *varchar                ;
FxEffectDef const **varFxEffectDefHandle     ;
unsigned short *varXBlendInfo          ;
GfxImageLoadDef *varGfxImageLoadDef     ;
GfxLightRegion *varGfxLightRegion     ;
GfxPackedPlacement *varGfxPackedPlacement     ;
SoundFile *varSoundFile     ;
DynEntityColl *varDynEntityColl     ;
unsigned char *varuint8_t             ;
GfxShadowGeometry *varGfxShadowGeometry     ;
union SoundFileRef *varSoundFileRef     ;
XModelPieces *varXModelPieces     ;
//unsigned __int8 *varvec3_t           ;
float (*varvec3_t)[3];
Font_s **varFontHandle     ;
GfxImage *varGfxImage     ;
//union MaterialTextureDefInfo *varMaterialTextureDefInfo     ;
water_t **varMaterialTextureDefInfo; // KISAKTODO: this is really the above union
MaterialInfo *varMaterialInfo     ;
union FxSpawnDef *varFxSpawnDef     ;
union FxElemVisuals *varFxElemVisuals     ;
enum weapAnimFiles_t *varweapAnimFiles_t     ;
SunLightParseParams *varSunLightParseParams     ;
FxEffectDef *varFxEffectDef     ;
enum GfxLightType *varGfxLightType     ;
XAnimParts **varXAnimPartsPtr     ;
PathData *varPathData     ;
float *varvec_t               ;
GfxBrushModel *varGfxBrushModel     ;
itemDef_s *varitemDef_t     ;
XAnimPartTransFrames *varXAnimPartTransFrames     ;
//short (*)[3] varShort3           ;
XSurfaceCollisionLeaf *varXSurfaceCollisionLeaf     ;
//D3DBaseTexture *varIDirect3DBaseTexture9     ;
XAnimDeltaPartQuatDataFrames *varXAnimDeltaPartQuatDataFrames     ;
union GfxTexture *varGfxRawTexture     ;
GfxPlacement *varGfxPlacement     ;
GfxLightImage *varGfxLightImage     ;
XModel *varXModel      ;
ComWorld *varComWorld     ;
int *varqboolean            ;
rectDef_s *varrectDef_t     ;
char *varint8_t              ;
GfxAabbTree *varGfxAabbTree     ;
FxElemVec3Range *varFxElemVec3Range     ;
PhysMass *varPhysMass     ;
SndDriverGlobals *varSndDriverGlobals     ;
menuDef_t **varmenuDef_ptr     ;
water_t *varwater_t     ;
GfxWorldVertex *varGfxWorldVertex     ;
GfxLightGrid *varGfxLightGrid     ;
MaterialTechniqueSet *varMaterialTechniqueSet     ;
enum OffhandClass *varOffhandClass     ;
FxIntRange *varFxIntRange     ;
unsigned int *varraw_uint            ;
unsigned long *varDWORD               ;
GameWorldSp *varGameWorldSp     ;
XSurfaceVertexInfo *varXSurfaceVertexInfo     ;
enum DynEntityType *varDynEntityType     ;
union GfxColor *varGfxColor     ;
MaterialTechnique **varMaterialTechniquePtr     ;
union GfxTexture *varGfxTexture     ;
GfxWorldDpvsPlanes *varGfxWorldDpvsPlanes     ;
MaterialPixelShader *varMaterialPixelShader     ;
Picmip *varPicmip      ;
int *varint32_t             ;
Material *varMaterial     ;
//XModelHighMipBounds *varXModelHighMipBounds     ;
snd_alias_list_t **varsnd_alias_list_name     ;
DynEntityDef *varDynEntityDef     ;
union XAnimDeltaPartQuatData *varXAnimDeltaPartQuatData     ;
srfTriangles_t *varsrfTriangles_t     ;
XAnimNotifyInfo *varXAnimNotifyInfo     ;
union itemDefData_t *varitemDefData_t     ;
//D3DTexture *varIDirect3DTexture9     ;
GfxLight *varGfxLight     ;
FxImpactEntry *varFxImpactEntry     ;
FxElemVisualState *varFxElemVisualState     ;
windowDef_t *varwindowDef_t     ;
XSurfaceCollisionTree *varXSurfaceCollisionTree     ;
union CollisionAabbTreeIndex *varCollisionAabbTreeIndex     ;
XSurfaceCollisionAabb *varXSurfaceCollisionAabb     ;
XSurface *varXSurface     ;
//union PackedTexCoords *varPackedTexCoords     ;
enum weaponIconRatioType_t *varweaponIconRatioType_t     ;
GfxVertexShaderLoadDef *varGfxVertexShaderLoadDef     ;
enum WeapOverlayInteface_t *varWeapOverlayInteface_t     ;
editFieldDef_s **vareditFieldDef_ptr     ;
MaterialMemory *varMaterialMemory     ;
//XaIwXmaDataInfo *varXaIwXmaDataInfo     ;
FxElemDef *varFxElemDef     ;
union cLeafBrushNodeData_t *varcLeafBrushNodeData_t     ;
PhysGeomList *varPhysGeomList     ;
//GfxStreamingAabbTree *varGfxStreamingAabbTree     ;
LoadedSound **varLoadedSoundPtr     ;
unsigned int *varraw_uint128         ;
StringTable *varStringTable     ;
union GfxDrawSurf *varGfxDrawSurf     ;
DynEntityClient *varDynEntityClient     ;
dmaterial_t *vardmaterial_t     ;
RawFile *varRawFile     ;
SndCurve **varSndCurvePtr     ;
ItemKeyHandler *varItemKeyHandlerNext     ;
MaterialTechniqueSet **varMaterialTechniqueSetPtr     ;
GfxWorldVertexData *varGfxWorldVertexData     ;
MaterialVertexDeclaration *varMaterialVertexDeclaration     ;
enum guidedMissileType_t *varguidedMissileType_t     ;
GfxPixelShaderLoadDef *varGfxPixelShaderLoadDef     ;
union PackedLightingCoords *varPackedLightingCoords     ;
MaterialVertexShader **varMaterialVertexShaderPtr     ;
union XAnimDynamicIndices *varXAnimDynamicIndicesTrans     ;
unsigned short *varStaticModelIndex     ;
GfxStaticModelDrawInst *varGfxStaticModelDrawInst     ;
enum PenetrateType *varPenetrateType     ;
int marker_db_load           ;
GfxLightDef *varGfxLightDef     ;
//union MaterialVertexShaderProgram *varMaterialVertexShaderProgram     ;
SndDriverGlobals **varSndDriverGlobalsPtr     ;
cStaticModel_s *varcStaticModel_t     ;
menuDef_t *varmenuDef_t     ;
expressionEntry **varexpressionEntry_ptr     ;
unsigned char *varbyte4               ;
unsigned long *varraw_DWORD           ;
pathnode_tree_t *varpathnode_tree_t     ;
char const ***varXStringPtr      ;
union pathnode_tree_info_t *varpathnode_tree_info_t     ;
cLeafBrushNode_s *varcLeafBrushNode_t     ;
complex_s *varcomplex_t     ;
WeaponDef **varWeaponDefPtr     ;
LoadedSound *varLoadedSound     ;
//XaSeekTable *varXaSeekTable     ;
//XAUDIOSOURCEFORMAT *varXAUDIOSOURCEFORMAT     ;
unsigned char *varGfxImageCategory     ;
unsigned char *varXAUDIOXMASTREAMCOUNT     ;
GfxSceneDynModel *varGfxSceneDynModel     ;
FxSpawnDefOneShot *varFxSpawnDefOneShot     ;
ScriptStringList *varScriptStringList     ;
union XAnimDynamicIndices *varXAnimDynamicIndicesDeltaQuat     ;
GfxLightRegionAxis *varGfxLightRegionAxis     ;
unsigned char *varraw_byte            ;
void *varvoid                ;
cNode_t *varcNode_t     ;
GfxSurface *varGfxSurface     ;
multiDef_s *varmultiDef_t     ;
union GfxTexture *varGfxTextureLoad;
GameWorldMp **varGameWorldMpPtr     ;
enum WeapStickinessType *varWeapStickinessType     ;
GfxWorld **varGfxWorldPtr     ;
enum weapProjExposion_t *varweapProjExposion_t     ;
snd_alias_t *varsnd_alias_t     ;
unsigned char *varraw_byte16          ;
SpeakerMap *varSpeakerMap     ;
//D3DIndexBuffer *varGfxIndexBuffer     ;
unsigned char *varGfxSamplerState     ;
unsigned short *varraw_ushort          ;
MaterialArgumentCodeConst *varMaterialArgumentCodeConst     ;
union XAnimDynamicFrames *varXAnimDynamicFrames     ;
pathnode_tree_nodes_t *varpathnode_tree_nodes_t     ;
StreamedSound *varStreamedSound     ;
XModelStreamInfo *varXModelStreamInfo     ;
FxElemVelStateInFrame *varFxElemVelStateInFrame     ;
unsigned char *varcbrushedge_t        ;
pathbasenode_t *varpathbasenode_t     ;
GfxStateBits *varGfxStateBits     ;
union PackedUnitVec *varPackedUnitVec     ;
GfxPosTexVertex *varGfxPosTexVertex     ;
unsigned short *varr_index_t           ;
BrushWrapper *varBrushWrapper     ;
GfxPackedVertex *varGfxPackedVertex0     ;
int *varFxElemDefFlags      ;
FxTrailDef *varFxTrailDef     ;
GfxReflectionProbe *varGfxReflectionProbe     ;
GfxStaticModelInst *varGfxStaticModelInst     ;
union entryInternalData *varentryInternalData     ;
GameWorldMp *varGameWorldMp     ;
MaterialVertexShader *varMaterialVertexShader     ;
cbrushside_t *varcbrushside_t     ;
char const **varXString           ;
unsigned char *varBYTE                ;
GfxWorldDpvsDynamic *varGfxWorldDpvsDynamic     ;
FxSpawnDefLooping *varFxSpawnDefLooping     ;
MaterialConstantDef *varMaterialConstantDef     ;
StreamFileNameRaw *varStreamFileNameRaw     ;
GfxCullGroup *varGfxCullGroup     ;
PhysPreset **varPhysPresetPtr     ;
rectDef_s *varUiRectangle     ;
DynEntityPose *varDynEntityPose     ;
MapEnts **varMapEntsPtr     ;
enum ImpactType *varImpactType     ;
FxImpactTable *varFxImpactTable     ;
cbrush_t *varcbrush_t     ;
//D3DVertexBuffer *varGfxVertexBuffer     ;
GfxWorldVertexLayerData *varGfxWorldVertexLayerData     ;
MapEnts *varMapEnts     ;
unsigned char *varXAUDIOCHANNEL       ;
char *varchar2048            ;
//XAUDIOPACKET_ALIGNED *varXAUDIOPACKET_ALIGNED     ;
unsigned short *varScriptString        ;
windowDef_t *varWindow     ;
CollisionBorder *varCollisionBorder     ;
FxElemVelStateSample *varFxElemVelStateSample     ;
XAnimDeltaPart *varXAnimDeltaPart     ;
GfxWorldVertex *varGfxWorldVertex0     ;
//float (*)[3] varshared_vec3_t     ;
listBoxDef_s **varlistBoxDef_ptr     ;
PhysGeomInfo *varPhysGeomInfo     ;
//unsigned char (*)[3] varByteVec          ;
unsigned short *varuint16_t            ;
enum weapFireType_t *varweapFireType_t     ;
enum weaponAltModel_t *varweaponAltModel_t     ;
FxElemVisStateSample *varFxElemVisStateSample     ;
GfxWorldDpvsStatic *varGfxWorldDpvsStatic     ;
XAnimParts *varXAnimParts     ;
short *varshort               ;
GfxImage **varGfxImagePtr     ;
snd_alias_list_t *varsnd_alias_list_t     ;
cLeafBrushNodeChildren_t *varcLeafBrushNodeChildren_t     ;
LocalizeEntry **varLocalizeEntryPtr     ;
unsigned __int8 (*varByteVec)[3];
//MssSound *varMssSound;
MssSoundCOD4 *varMssSound;
IDirect3DVertexBuffer9 **varGfxVertexBuffer;
unsigned __int8 *varXZoneHandle;
MaterialVertexShaderProgram *varMaterialVertexShaderProgram;

void __cdecl Load_byte(bool atStreamStart)
{
    Load_Stream(atStreamStart, varbyte, 1);
}

void __cdecl Load_byteArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varbyte, count);
}

void __cdecl Load_charArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varchar, count);
}

void __cdecl Load_int(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varint, 4);
}

void __cdecl Load_intArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varint, 4 * count);
}

void __cdecl Load_uintArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned char*)varuint, 4 * count);
}

void __cdecl Load_uint(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varuint, 4);
}

void __cdecl Load_float(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varfloat, 4);
}

void __cdecl Load_floatArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varfloat, 4 * count);
}

void __cdecl Load_raw_uintArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varraw_uint, 4 * count);
}

unsigned __int8 *__cdecl AllocLoad_raw_uint128()
{
    return DB_AllocStreamPos(127);
}

void __cdecl Load_raw_uint128Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varraw_uint128, 4 * count);
}

void __cdecl Load_raw_byteArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varraw_byte, count);
}

void __cdecl Load_raw_byte16Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varraw_byte16, count);
}

void __cdecl Load_vec2_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varvec2_t, 8 * count);
}

void __cdecl Load_vec3_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varvec3_t, 12);
}

void __cdecl Load_vec3_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varvec3_t, 12 * count);
}

void __cdecl Load_shortArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varshort, 2 * count);
}

void __cdecl Load_ushortArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varushort, 2 * count);
}

void __cdecl Load_XQuat2(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXQuat2, 4);
}

void __cdecl Load_XQuat2Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXQuat2, 4 * count);
}

unsigned __int8 *__cdecl AllocLoad_XBlendInfo()
{
    return DB_AllocStreamPos(1);
}

void __cdecl Load_UnsignedShortArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varUnsignedShort, 2 * count);
}

void __cdecl Load_ScriptString(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varScriptString, 2);
    Load_ScriptStringCustom(varScriptString);
}

void __cdecl Load_ScriptStringArray(bool atStreamStart, int count)
{
    unsigned __int16 *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varScriptString, 2 * count);
    var = varScriptString;
    for (i = 0; i < count; ++i)
    {
        varScriptString = var;
        Load_ScriptString(0);
        ++var;
    }
}

unsigned __int8 *__cdecl AllocLoad_raw_byte()
{
    return DB_AllocStreamPos(0);
}

void __cdecl Load_ConstCharArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varConstChar, count);
}

void __cdecl Load_TempString(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varTempString, 4);
    if (*varTempString)
    {
        if (*varTempString == (const char *)-1)
        {
            *varTempString = (const char *)AllocLoad_raw_byte();
            varConstChar = *varTempString;
            Load_TempStringCustom((char **)varTempString);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varTempString);
        }
    }
}

void __cdecl Load_TempStringArray(bool atStreamStart, int count)
{
    const char **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varTempString, 4 * count);
    var = varTempString;
    for (i = 0; i < count; ++i)
    {
        varTempString = var;
        Load_TempString(0);
        ++var;
    }
}

void __cdecl Load_XString(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXString, 4);
    if (*varXString)
    {
        if (*varXString == (const char *)-1)
        {
            *varXString = (const char *)AllocLoad_raw_byte();
            varConstChar = *varXString;
            Load_XStringCustom((char **)varXString);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varXString);
        }
    }
}

void __cdecl Load_XStringArray(bool atStreamStart, int count)
{
    const char **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXString, 4 * count);
    var = varXString;
    for (i = 0; i < count; ++i)
    {
        varXString = var;
        Load_XString(0);
        ++var;
    }
}

void __cdecl Load_XStringPtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXStringPtr, 4);
    if (*varXStringPtr)
    {
        if (*varXStringPtr == (const char **)-1)
        {
            *varXStringPtr = (const char **)AllocLoad_FxElemVisStateSample();
            varXString = *varXStringPtr;
            Load_XString(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varXStringPtr);
        }
    }
}

void __cdecl Load_ScriptStringList(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varScriptStringList, 8);
    DB_PushStreamPos(4u);
    if (varScriptStringList->strings)
    {
        varScriptStringList->strings = (const char **)AllocLoad_FxElemVisStateSample();
        varTempString = varScriptStringList->strings;
        Load_TempStringArray(1, varScriptStringList->count);
    }
    DB_PopStreamPos();
}

void __cdecl Load_complex_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcomplex_t, 8 * count);
}

void __cdecl Load_dmaterial_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vardmaterial_t, 72 * count);
}

void __cdecl Mark_ScriptString()
{
    Mark_ScriptStringCustom(varScriptString);
}

void __cdecl Mark_ScriptStringArray(int count)
{
    unsigned __int16 *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varScriptString;
    for (i = 0; i < count; ++i)
    {
        varScriptString = var;
        Mark_ScriptString();
        ++var;
    }
}

void __cdecl Load_XAnimIndices()
{
    if (varXAnimParts->numframes >= 0x100u)
    {
        if (varXAnimIndices->_1)
        {
            varXAnimIndices->_1 = AllocLoad_XBlendInfo();
            varushort = (unsigned __int16 *)varXAnimIndices->_1;
            Load_ushortArray(1, varXAnimParts->indexCount);
        }
    }
    else if (varXAnimIndices->_1)
    {
        varXAnimIndices->_1 = AllocLoad_raw_byte();
        varbyte = varXAnimIndices->_1;
        Load_byteArray(1, varXAnimParts->indexCount);
    }
}

void __cdecl Load_XAnimDynamicIndicesDeltaQuat(bool atStreamStart)
{
    if (varXAnimParts->numframes >= 0x100u)
    {
        if (!atStreamStart)
            MyAssertHandler("c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h", 1229, 0, "%s", "atStreamStart");
        Load_Stream(1, varXAnimDynamicIndicesDeltaQuat->_1, 0);
        if (DB_GetStreamPos() != (unsigned __int8 *)varXAnimDynamicIndicesDeltaQuat)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h",
                1231,
                0,
                "%s",
                "DB_GetStreamPos() == reinterpret_cast< byte * >( varXAnimDynamicIndicesDeltaQuat->_2 )");
        varUnsignedShort = (unsigned __int16 *)varXAnimDynamicIndicesDeltaQuat;
        Load_UnsignedShortArray(1, varXAnimDeltaPartQuat->size + 1);
    }
    else
    {
        if (!atStreamStart)
            MyAssertHandler("c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h", 1221, 0, "%s", "atStreamStart");
        Load_Stream(1, varXAnimDynamicIndicesDeltaQuat->_1, 0);
        if (DB_GetStreamPos() != (unsigned __int8 *)varXAnimDynamicIndicesDeltaQuat)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h",
                1223,
                0,
                "%s",
                "DB_GetStreamPos() == reinterpret_cast< byte * >( varXAnimDynamicIndicesDeltaQuat->_1 )");
        varbyte = (unsigned __int8 *)varXAnimDynamicIndicesDeltaQuat;
        Load_byteArray(1, varXAnimDeltaPartQuat->size + 1);
    }
}

void __cdecl Load_XAnimDeltaPartQuatDataFrames(bool atStreamStart)
{
    if (!atStreamStart)
        MyAssertHandler("c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h", 1284, 0, "%s", "atStreamStart");
    Load_Stream(1, (unsigned __int8 *)varXAnimDeltaPartQuatDataFrames, 4);
    if (DB_GetStreamPos() != (unsigned __int8 *)&varXAnimDeltaPartQuatDataFrames->indices)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h",
            1286,
            0,
            "%s",
            "DB_GetStreamPos() == reinterpret_cast< byte * >( &varXAnimDeltaPartQuatDataFrames->indices )");
    varXAnimDynamicIndicesDeltaQuat = &varXAnimDeltaPartQuatDataFrames->indices;
    Load_XAnimDynamicIndicesDeltaQuat(1);
    if (varXAnimDeltaPartQuatDataFrames->frames)
    {
        varXAnimDeltaPartQuatDataFrames->frames = (__int16 (*)[2])AllocLoad_FxElemVisStateSample();
        varXQuat2 = (unsigned char*)varXAnimDeltaPartQuatDataFrames->frames;
        if (varXAnimDeltaPartQuat->size)
            Load_XQuat2Array(1, varXAnimDeltaPartQuat->size + 1);
        else
            Load_XQuat2Array(1, 0);
    }
}

void __cdecl Load_XAnimDeltaPartQuatData(bool atStreamStart)
{
    if (varXAnimDeltaPartQuat->size)
    {
        varXAnimDeltaPartQuatDataFrames = &varXAnimDeltaPartQuatData->frames;
        Load_XAnimDeltaPartQuatDataFrames(atStreamStart);
    }
    else if (atStreamStart)
    {
        varXQuat2 = (unsigned char*)varXAnimDeltaPartQuatData;
        Load_XQuat2(atStreamStart);
    }
}

void __cdecl Load_XAnimDeltaPartQuat(bool atStreamStart)
{
    if (!atStreamStart)
        MyAssertHandler("c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h", 1415, 0, "%s", "atStreamStart");
    Load_Stream(1, (unsigned __int8 *)varXAnimDeltaPartQuat, 4);
    if (DB_GetStreamPos() != (unsigned __int8 *)&varXAnimDeltaPartQuat->u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h",
            1417,
            0,
            "%s",
            "DB_GetStreamPos() == reinterpret_cast< byte * >( &varXAnimDeltaPartQuat->u )");
    varXAnimDeltaPartQuatData = &varXAnimDeltaPartQuat->u;
    Load_XAnimDeltaPartQuatData(1);
}

void __cdecl Load_XAnimDeltaPart(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimDeltaPart, 8);
    if (varXAnimDeltaPart->trans)
    {
        varXAnimDeltaPart->trans = (XAnimPartTrans *)AllocLoad_FxElemVisStateSample();
        varXAnimPartTrans = varXAnimDeltaPart->trans;
        Load_XAnimPartTrans(1);
    }
    if (varXAnimDeltaPart->quat)
    {
        varXAnimDeltaPart->quat = (XAnimDeltaPartQuat *)AllocLoad_FxElemVisStateSample();
        varXAnimDeltaPartQuat = varXAnimDeltaPart->quat;
        Load_XAnimDeltaPartQuat(1);
    }
}

void __cdecl Load_XAnimDynamicIndicesTrans(bool atStreamStart)
{
    if (varXAnimParts->numframes >= 0x100u)
    {
        if (!atStreamStart)
            MyAssertHandler("c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h", 1550, 0, "%s", "atStreamStart");
        Load_Stream(1, varXAnimDynamicIndicesTrans->_1, 0);
        if (DB_GetStreamPos() != (unsigned __int8 *)varXAnimDynamicIndicesTrans)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h",
                1552,
                0,
                "%s",
                "DB_GetStreamPos() == reinterpret_cast< byte * >( varXAnimDynamicIndicesTrans->_2 )");
        varUnsignedShort = (unsigned __int16 *)varXAnimDynamicIndicesTrans;
        Load_UnsignedShortArray(1, varXAnimPartTrans->size + 1);
    }
    else
    {
        if (!atStreamStart)
            MyAssertHandler("c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h", 1542, 0, "%s", "atStreamStart");
        Load_Stream(1, varXAnimDynamicIndicesTrans->_1, 0);
        if (DB_GetStreamPos() != (unsigned __int8 *)varXAnimDynamicIndicesTrans)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h",
                1544,
                0,
                "%s",
                "DB_GetStreamPos() == reinterpret_cast< byte * >( varXAnimDynamicIndicesTrans->_1 )");
        varbyte = (unsigned __int8 *)varXAnimDynamicIndicesTrans;
        Load_byteArray(1, varXAnimPartTrans->size + 1);
    }
}

void __cdecl Load_ByteVecArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varByteVec, 3 * count);
}

void __cdecl Load_UShortVecArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varUShortVec, 6 * count);
}

void __cdecl Load_XAnimDynamicFrames()
{
    if (varXAnimPartTrans->smallTrans)
    {
        if (varXAnimDynamicFrames->_1)
        {
            varXAnimDynamicFrames->_1 = (unsigned __int8 (*)[3])AllocLoad_raw_byte();
            varByteVec = varXAnimDynamicFrames->_1;
            if (varXAnimPartTrans->size)
                Load_ByteVecArray(1, varXAnimPartTrans->size + 1);
            else
                Load_ByteVecArray(1, 0);
        }
    }
    else if (varXAnimDynamicFrames->_1)
    {
        varXAnimDynamicFrames->_1 = (unsigned __int8 (*)[3])AllocLoad_FxElemVisStateSample();
        varUShortVec = (unsigned char*)varXAnimDynamicFrames->_1;
        if (varXAnimPartTrans->size)
            Load_UShortVecArray(1, varXAnimPartTrans->size + 1);
        else
            Load_UShortVecArray(1, 0);
    }
}

void __cdecl Load_XAnimPartTransFrames(bool atStreamStart)
{
    if (!atStreamStart)
        MyAssertHandler("c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h", 1784, 0, "%s", "atStreamStart");
    Load_Stream(1, (unsigned __int8 *)varXAnimPartTransFrames, 28);
    if (DB_GetStreamPos() != (unsigned __int8 *)&varXAnimPartTransFrames->indices)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h",
            1786,
            0,
            "%s",
            "DB_GetStreamPos() == reinterpret_cast< byte * >( &varXAnimPartTransFrames->indices )");
    varXAnimDynamicIndicesTrans = &varXAnimPartTransFrames->indices;
    Load_XAnimDynamicIndicesTrans(1);
    varXAnimDynamicFrames = &varXAnimPartTransFrames->frames;
    Load_XAnimDynamicFrames();
}

void __cdecl Load_XAnimPartTransData(bool atStreamStart)
{
    if (varXAnimPartTrans->size)
    {
        varXAnimPartTransFrames = &varXAnimPartTransData->frames;
        Load_XAnimPartTransFrames(atStreamStart);
    }
    else if (atStreamStart)
    {
        varvec3_t = (float (*)[3])varXAnimPartTransData;
        Load_vec3_t(atStreamStart);
    }
}

void __cdecl Load_XAnimPartTrans(bool atStreamStart)
{
    if (!atStreamStart)
        MyAssertHandler("c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h", 1923, 0, "%s", "atStreamStart");
    Load_Stream(1, (unsigned __int8 *)varXAnimPartTrans, 4);
    if (DB_GetStreamPos() != (unsigned __int8 *)&varXAnimPartTrans->u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\database\\../xanim/xanim_load_db.h",
            1925,
            0,
            "%s",
            "DB_GetStreamPos() == reinterpret_cast< byte * >( &varXAnimPartTrans->u )");
    varXAnimPartTransData = &varXAnimPartTrans->u;
    Load_XAnimPartTransData(1);
}

void __cdecl Load_XAnimNotifyInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimNotifyInfo, 8);
    varScriptString = &varXAnimNotifyInfo->name;
    Load_ScriptString(0);
}

void __cdecl Load_XAnimNotifyInfoArray(bool atStreamStart, int count)
{
    XAnimNotifyInfo *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimNotifyInfo, 8 * count);
    var = varXAnimNotifyInfo;
    for (i = 0; i < count; ++i)
    {
        varXAnimNotifyInfo = var;
        Load_XAnimNotifyInfo(0);
        ++var;
    }
}

void __cdecl Load_XAnimParts(bool atStreamStart)
{
    static_assert(sizeof(XAnimParts) == 88);
    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimParts, 88);
    DB_PushStreamPos(4u);
    varXString = &varXAnimParts->name;
    Load_XString(0);
    if (varXAnimParts->names)
    {
        varXAnimParts->names = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varScriptString = varXAnimParts->names;
        Load_ScriptStringArray(1, varXAnimParts->boneCount[9]);
    }
    if (varXAnimParts->notify)
    {
        varXAnimParts->notify = (XAnimNotifyInfo *)AllocLoad_FxElemVisStateSample();
        varXAnimNotifyInfo = varXAnimParts->notify;
        Load_XAnimNotifyInfoArray(1, varXAnimParts->notifyCount);
    }
    if (varXAnimParts->deltaPart)
    {
        varXAnimParts->deltaPart = (XAnimDeltaPart *)AllocLoad_FxElemVisStateSample();
        varXAnimDeltaPart = varXAnimParts->deltaPart;
        Load_XAnimDeltaPart(1);
    }
    if (varXAnimParts->dataByte)
    {
        varXAnimParts->dataByte = AllocLoad_raw_byte();
        varbyte = varXAnimParts->dataByte;
        Load_byteArray(1, varXAnimParts->dataByteCount);
    }
    if (varXAnimParts->dataShort)
    {
        varXAnimParts->dataShort = (__int16 *)AllocLoad_XBlendInfo();
        varshort = varXAnimParts->dataShort;
        Load_shortArray(1, varXAnimParts->dataShortCount);
    }
    if (varXAnimParts->dataInt)
    {
        varXAnimParts->dataInt = (int *)AllocLoad_FxElemVisStateSample();
        varint = varXAnimParts->dataInt;
        Load_intArray(1, varXAnimParts->dataIntCount);
    }
    if (varXAnimParts->randomDataShort)
    {
        varXAnimParts->randomDataShort = (__int16 *)AllocLoad_XBlendInfo();
        varshort = varXAnimParts->randomDataShort;
        Load_shortArray(1, varXAnimParts->randomDataShortCount);
    }
    if (varXAnimParts->randomDataByte)
    {
        varXAnimParts->randomDataByte = AllocLoad_raw_byte();
        varbyte = varXAnimParts->randomDataByte;
        Load_byteArray(1, varXAnimParts->randomDataByteCount);
    }
    if (varXAnimParts->randomDataInt)
    {
        varXAnimParts->randomDataInt = (int *)AllocLoad_FxElemVisStateSample();
        varint = varXAnimParts->randomDataInt;
        Load_intArray(1, varXAnimParts->randomDataIntCount);
    }
    varXAnimIndices = &varXAnimParts->indices;
    Load_XAnimIndices();
    DB_PopStreamPos();
}

void __cdecl Load_XAnimPartsPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXAnimPartsPtr, 4);
    DB_PushStreamPos(0);
    if (*varXAnimPartsPtr)
    {
        value = (unsigned int)*varXAnimPartsPtr;
        if (*varXAnimPartsPtr == (XAnimParts *)-1 || value == -2)
        {
            *varXAnimPartsPtr = (XAnimParts *)AllocLoad_FxElemVisStateSample();
            varXAnimParts = *varXAnimPartsPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_XAnimParts(1);
            Load_XAnimPartsAsset((XAssetHeader *)varXAnimPartsPtr);
            if (inserted)
                *inserted = *varXAnimPartsPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varXAnimPartsPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_XAnimNotifyInfo()
{
    varScriptString = &varXAnimNotifyInfo->name;
    Mark_ScriptString();
}

void __cdecl Mark_XAnimNotifyInfoArray(int count)
{
    XAnimNotifyInfo *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varXAnimNotifyInfo;
    for (i = 0; i < count; ++i)
    {
        varXAnimNotifyInfo = var;
        Mark_XAnimNotifyInfo();
        ++var;
    }
}

void __cdecl Mark_XAnimParts()
{
    if (varXAnimParts->names)
    {
        varScriptString = varXAnimParts->names;
        Mark_ScriptStringArray(varXAnimParts->boneCount[9]);
    }
    if (varXAnimParts->notify)
    {
        varXAnimNotifyInfo = varXAnimParts->notify;
        Mark_XAnimNotifyInfoArray(varXAnimParts->notifyCount);
    }
}

void __cdecl Mark_XAnimPartsPtr()
{
    if (*varXAnimPartsPtr)
    {
        varXAnimParts = *varXAnimPartsPtr;
        Mark_XAnimPartsAsset(varXAnimParts);
        Mark_XAnimParts();
    }
}

void __cdecl Load_XBoneInfoArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXBoneInfo, 40 * count);
}

void __cdecl Load_DObjAnimMatArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDObjAnimMat, 32 * count);
}

void __cdecl Load_StreamFileNameRaw(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStreamFileNameRaw, 8);
    varXString = &varStreamFileNameRaw->dir;
    Load_XString(0);
    varXString = &varStreamFileNameRaw->name;
    Load_XString(0);
}

void __cdecl Load_StreamFileInfo(bool atStreamStart)
{
    varStreamFileNameRaw = &varStreamFileInfo->raw;
    Load_StreamFileNameRaw(atStreamStart);
}

void __cdecl Load_StreamFileName(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStreamFileName, 8);
    varStreamFileInfo = &varStreamFileName->info;
    Load_StreamFileInfo(0);
}

void __cdecl Load_SetSoundData(unsigned __int8 **data, MssSoundCOD4 *mssSound)
{
    SND_SetData(mssSound, *data);
}

void __cdecl Load_MssSound(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned char*)varMssSound, 40);
    DB_PushStreamPos(0);
    if (varMssSound->data)
    {
        value = (unsigned int)varMssSound->data;
        if (value < 0xFFFFFFFE)
        {
            DB_ConvertOffsetToAlias((unsigned int*)&varMssSound->data);
        }
        else
        {
            varMssSound->data = AllocLoad_raw_byte();
            varbyte = varMssSound->data;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_byteArray(1, varMssSound->info.data_len);
            Load_SetSoundData(&varMssSound->data, varMssSound);
            if (inserted)
                *inserted = varMssSound->data;
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_LoadedSound(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varLoadedSound, 44);
    DB_PushStreamPos(4u);
    varXString = &varLoadedSound->name;
    Load_XString(0);
    varMssSound = &varLoadedSound->sound;
    Load_MssSound(0);
    DB_PopStreamPos();
}

void __cdecl Load_LoadedSoundPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varLoadedSoundPtr, 4);
    DB_PushStreamPos(0);
    if (*varLoadedSoundPtr)
    {
        value = (unsigned int)*varLoadedSoundPtr;
        if (*varLoadedSoundPtr == (LoadedSound *)-1 || value == -2)
        {
            *varLoadedSoundPtr = (LoadedSound *)AllocLoad_FxElemVisStateSample();
            varLoadedSound = *varLoadedSoundPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_LoadedSound(1);
            Load_LoadedSoundAsset((XAssetHeader *)varLoadedSoundPtr);
            if (inserted)
                *inserted = *varLoadedSoundPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varLoadedSoundPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_StreamedSound(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStreamedSound, 8);
    varStreamFileName = &varStreamedSound->filename;
    Load_StreamFileName(0);
}

void __cdecl Load_SoundFileRef(bool atStreamStart)
{
    if (varSoundFile->type == 1)
    {
        varLoadedSoundPtr = &varSoundFileRef->loadSnd;
        Load_LoadedSoundPtr(atStreamStart);
    }
    else
    {
        varStreamedSound = (StreamedSound *)varSoundFileRef;
        Load_StreamedSound(atStreamStart);
    }
}

void __cdecl Load_SoundFile(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varSoundFile->type, 12);
    varSoundFileRef = &varSoundFile->u;
    Load_SoundFileRef(0);
}

void __cdecl Load_SndCurve(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varSndCurve, 72);
    DB_PushStreamPos(4u);
    varXString = &varSndCurve->filename;
    Load_XString(0);
    DB_PopStreamPos();
}

void __cdecl Load_SndCurvePtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varSndCurvePtr, 4);
    DB_PushStreamPos(0);
    if (*varSndCurvePtr)
    {
        value = (unsigned int)*varSndCurvePtr;
        if (*varSndCurvePtr == (SndCurve *)-1 || value == -2)
        {
            *varSndCurvePtr = (SndCurve *)AllocLoad_FxElemVisStateSample();
            varSndCurve = *varSndCurvePtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_SndCurve(1);
            Load_SndCurveAsset((XAssetHeader *)varSndCurvePtr);
            if (inserted)
                *inserted = *varSndCurvePtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varSndCurvePtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_SpeakerMap(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varSpeakerMap, 408);
    varXString = &varSpeakerMap->name;
    Load_XString(0);
}

void __cdecl Load_snd_alias_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_t, 92);
    varXString = &varsnd_alias_t->aliasName;
    Load_XString(0);
    varXString = &varsnd_alias_t->subtitle;
    Load_XString(0);
    varXString = &varsnd_alias_t->secondaryAliasName;
    Load_XString(0);
    varXString = &varsnd_alias_t->chainAliasName;
    Load_XString(0);
    if (varsnd_alias_t->soundFile)
    {
        if (varsnd_alias_t->soundFile == (SoundFile *)-1)
        {
            varsnd_alias_t->soundFile = (SoundFile *)AllocLoad_FxElemVisStateSample();
            varSoundFile = varsnd_alias_t->soundFile;
            Load_SoundFile(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varsnd_alias_t->soundFile);
        }
    }
    varSndCurvePtr = &varsnd_alias_t->volumeFalloffCurve;
    Load_SndCurvePtr(0);
    if (varsnd_alias_t->speakerMap)
    {
        if (varsnd_alias_t->speakerMap == (SpeakerMap *)-1)
        {
            varsnd_alias_t->speakerMap = (SpeakerMap *)AllocLoad_FxElemVisStateSample();
            varSpeakerMap = varsnd_alias_t->speakerMap;
            Load_SpeakerMap(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varsnd_alias_t->speakerMap);
        }
    }
}

void __cdecl Load_snd_alias_tArray(bool atStreamStart, int count)
{
    snd_alias_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_t, 92 * count);
    var = varsnd_alias_t;
    for (i = 0; i < count; ++i)
    {
        varsnd_alias_t = var;
        Load_snd_alias_t(0);
        ++var;
    }
}

void __cdecl Load_snd_alias_list_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_list_t, 12);
    DB_PushStreamPos(4u);
    varXString = &varsnd_alias_list_t->aliasName;
    Load_XString(0);
    if (varsnd_alias_list_t->head)
    {
        if (varsnd_alias_list_t->head == (snd_alias_t *)-1)
        {
            varsnd_alias_list_t->head = (snd_alias_t *)AllocLoad_FxElemVisStateSample();
            varsnd_alias_t = varsnd_alias_list_t->head;
            Load_snd_alias_tArray(1, varsnd_alias_list_t->count);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varsnd_alias_list_t->head);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_snd_alias_list_ptr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned char*)varsnd_alias_list_ptr, 4);
    DB_PushStreamPos(0);
    if (*varsnd_alias_list_ptr)
    {
        value = (unsigned int)*varsnd_alias_list_ptr;
        if ((unsigned int)*varsnd_alias_list_ptr == -1 || value == -2)
        {
            *varsnd_alias_list_ptr = (snd_alias_list_t*)AllocLoad_FxElemVisStateSample();
            varsnd_alias_list_t = *varsnd_alias_list_ptr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_snd_alias_list_t(1);
            Load_snd_alias_list_Asset((XAssetHeader*)varsnd_alias_list_ptr);
            if (inserted)
                *inserted = *varsnd_alias_list_ptr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int*)varsnd_alias_list_ptr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_snd_alias_list_name(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_list_name, 4);
    Load_SndAliasCustom(varsnd_alias_list_name);
}

void __cdecl Load_snd_alias_list_nameArray(bool atStreamStart, int count)
{
    snd_alias_list_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varsnd_alias_list_name, 4 * count);
    var = varsnd_alias_list_name;
    for (i = 0; i < count; ++i)
    {
        varsnd_alias_list_name = var;
        Load_snd_alias_list_name(0);
        ++var;
    }
}

void __cdecl Mark_LoadedSoundPtr()
{
    if (*varLoadedSoundPtr)
    {
        varLoadedSound = *varLoadedSoundPtr;
        Mark_LoadedSoundAsset(varLoadedSound);
    }
}

void __cdecl Mark_SoundFileRef()
{
    if (varSoundFile->type == 1)
    {
        varLoadedSoundPtr = &varSoundFileRef->loadSnd;
        Mark_LoadedSoundPtr();
    }
}

void __cdecl Mark_SoundFile()
{
    varSoundFileRef = &varSoundFile->u;
    Mark_SoundFileRef();
}

void __cdecl Mark_SndCurvePtr()
{
    if (*varSndCurvePtr)
    {
        varSndCurve = *varSndCurvePtr;
        Mark_SndCurveAsset(varSndCurve);
    }
}

void __cdecl Mark_snd_alias_t()
{
    if (varsnd_alias_t->soundFile)
    {
        varSoundFile = varsnd_alias_t->soundFile;
        Mark_SoundFile();
    }
    varSndCurvePtr = &varsnd_alias_t->volumeFalloffCurve;
    Mark_SndCurvePtr();
}

void __cdecl Mark_snd_alias_tArray(int count)
{
    snd_alias_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varsnd_alias_t;
    for (i = 0; i < count; ++i)
    {
        varsnd_alias_t = var;
        Mark_snd_alias_t();
        ++var;
    }
}

void __cdecl Mark_snd_alias_list_t()
{
    if (varsnd_alias_list_t->head)
    {
        varsnd_alias_t = varsnd_alias_list_t->head;
        Mark_snd_alias_tArray(varsnd_alias_list_t->count);
    }
}

void __cdecl Mark_snd_alias_list_ptr()
{
    if (*varsnd_alias_list_ptr)
    {
        varsnd_alias_list_t = *varsnd_alias_list_ptr;
        Mark_snd_alias_list_Asset(varsnd_alias_list_t);
        Mark_snd_alias_list_t();
    }
}

void __cdecl Mark_snd_alias_list_name()
{
    Mark_SndAliasCustom(varsnd_alias_list_name);
}

void __cdecl Mark_snd_alias_list_nameArray(int count)
{
    snd_alias_list_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varsnd_alias_list_name;
    for (i = 0; i < count; ++i)
    {
        varsnd_alias_list_name = var;
        Mark_snd_alias_list_name();
        ++var;
    }
}

void __cdecl Load_MaterialInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialInfo, 24);
    varXString = &varMaterialInfo->name;
    Load_XString(0);
}

void __cdecl Load_GfxWorldVertex0Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldVertex0, 44 * count);
}

void __cdecl Load_GfxPackedVertex0Array(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxPackedVertex0, 32 * count);
}

void __cdecl Load_GfxBrushModelArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxBrushModel, 56 * count);
}

void __cdecl Load_XSurfaceCollisionLeafArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXSurfaceCollisionLeaf, 2 * count);
}

cbrush_t *__cdecl AllocLoad_GfxPackedVertex0()
{
    return (cbrush_t *)DB_AllocStreamPos(15);
}

void __cdecl Load_XSurfaceCollisionNodeArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXSurfaceCollisionNode, 16 * count);
}

void __cdecl Load_XSurfaceCollisionTree(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXSurfaceCollisionTree, 40);
    if (varXSurfaceCollisionTree->nodes)
    {
        varXSurfaceCollisionTree->nodes = (XSurfaceCollisionNode *)AllocLoad_GfxPackedVertex0();
        varXSurfaceCollisionNode = varXSurfaceCollisionTree->nodes;
        Load_XSurfaceCollisionNodeArray(1, varXSurfaceCollisionTree->nodeCount);
    }
    if (varXSurfaceCollisionTree->leafs)
    {
        varXSurfaceCollisionTree->leafs = (XSurfaceCollisionLeaf *)AllocLoad_XBlendInfo();
        varXSurfaceCollisionLeaf = varXSurfaceCollisionTree->leafs;
        Load_XSurfaceCollisionLeafArray(1, varXSurfaceCollisionTree->leafCount);
    }
}

void __cdecl Load_XRigidVertList(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXRigidVertList, 12);
    if (varXRigidVertList->collisionTree)
    {
        if (varXRigidVertList->collisionTree == (XSurfaceCollisionTree *)-1)
        {
            varXRigidVertList->collisionTree = (XSurfaceCollisionTree *)AllocLoad_FxElemVisStateSample();
            varXSurfaceCollisionTree = varXRigidVertList->collisionTree;
            Load_XSurfaceCollisionTree(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXRigidVertList->collisionTree);
        }
    }
}

void __cdecl Load_XRigidVertListArray(bool atStreamStart, int count)
{
    XRigidVertList *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXRigidVertList, 12 * count);
    var = varXRigidVertList;
    for (i = 0; i < count; ++i)
    {
        varXRigidVertList = var;
        Load_XRigidVertList(0);
        ++var;
    }
}

void __cdecl Load_GfxVertexBuffer(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxVertexBuffer, 4);
}

void __cdecl Load_XBlendInfoArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXBlendInfo, 2 * count);
}

void __cdecl Load_XSurfaceVertexInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXSurfaceVertexInfo, 12);
    if (varXSurfaceVertexInfo->vertsBlend)
    {
        if (varXSurfaceVertexInfo->vertsBlend == (unsigned __int16 *)-1)
        {
            varXSurfaceVertexInfo->vertsBlend = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varXBlendInfo = varXSurfaceVertexInfo->vertsBlend;
            Load_XBlendInfoArray(
                1,
                7 * varXSurfaceVertexInfo->vertCount[3]
                + 5 * varXSurfaceVertexInfo->vertCount[2]
                + 3 * varXSurfaceVertexInfo->vertCount[1]
                + varXSurfaceVertexInfo->vertCount[0]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXSurfaceVertexInfo->vertsBlend);
        }
    }
}

void __cdecl Load_r_index_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varr_index_t, 2 * count);
}

void __cdecl Load_r_index16_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varr_index16_t, 2 * count);
}

void __cdecl Load_XZoneHandle(bool atStreamStart)
{
    Load_Stream(atStreamStart, varXZoneHandle, 1);
    varbyte = varXZoneHandle;
    Load_byte(0);
    Load_GetCurrentZoneHandle(varXZoneHandle);
}

void __cdecl Load_XSurface(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varXSurface->tileMode, 56);
    varXZoneHandle = &varXSurface->zoneHandle;
    Load_XZoneHandle(0);
    varXSurfaceVertexInfo = &varXSurface->vertInfo;
    Load_XSurfaceVertexInfo(0);
    DB_PushStreamPos(7u);
    if (varXSurface->verts0)
    {
        if (varXSurface->verts0 == (GfxPackedVertex *)-1)
        {
            varXSurface->verts0 = (GfxPackedVertex *)AllocLoad_GfxPackedVertex0();
            varGfxPackedVertex0 = varXSurface->verts0;
            Load_GfxPackedVertex0Array(1, varXSurface->vertCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXSurface->verts0);
        }
    }
    DB_PopStreamPos();
    if (varXSurface->vertList)
    {
        if (varXSurface->vertList == (XRigidVertList *)-1)
        {
            varXSurface->vertList = (XRigidVertList *)AllocLoad_FxElemVisStateSample();
            varXRigidVertList = varXSurface->vertList;
            Load_XRigidVertListArray(1, varXSurface->vertListCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXSurface->vertList);
        }
    }
    DB_PushStreamPos(8u);
    if (varXSurface->triIndices)
    {
        if (varXSurface->triIndices == (unsigned __int16 *)-1)
        {
            varXSurface->triIndices = (unsigned __int16 *)AllocLoad_GfxPackedVertex0();
            varr_index16_t = varXSurface->triIndices;
            Load_r_index16_tArray(1, 3 * varXSurface->triCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXSurface->triIndices);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_XSurfaceArray(bool atStreamStart, int count)
{
    XSurface *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, &varXSurface->tileMode, 56 * count);
    var = varXSurface;
    for (i = 0; i < count; ++i)
    {
        varXSurface = var;
        Load_XSurface(0);
        ++var;
    }
}

void __cdecl Load_GfxTextureLoad(bool atStreamStart)
{
    GfxTexture *inserted; // [esp+0h] [ebp-Ch]
    IDirect3DBaseTexture9 *value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned char*)varGfxTextureLoad, 4);
    DB_PushStreamPos(0);
    if (varGfxTextureLoad->basemap)
    {
        // LWSS: union abuse here
        value = varGfxTextureLoad->basemap;
        if (value == (IDirect3DBaseTexture9 *)-1 || value == (IDirect3DBaseTexture9*)-2)
        {
            varGfxTextureLoad->basemap = (IDirect3DBaseTexture9*)AllocLoad_FxElemVisStateSample();
            varGfxImageLoadDef = varGfxTextureLoad->loadDef;
            if (value == (IDirect3DBaseTexture9*)-2)
                inserted = (GfxTexture*)DB_InsertPointer();
            else
                inserted = 0;
            Load_GfxImageLoadDef(1);
            Load_Texture(varGfxTextureLoad, varGfxImage);
            if (inserted)
                inserted->basemap = varGfxTextureLoad->basemap;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int*)varGfxTextureLoad);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxRawTextureArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxRawTexture, 4 * count);
}

void __cdecl Load_GfxImageLoadDef(bool atStreamStart)
{
    if (!atStreamStart)
        MyAssertHandler("c:\\trees\\cod3\\src\\database\\../gfx_d3d/r_image_load_db.h", 2614, 0, "%s", "atStreamStart");
    iassert(OFFSET_TO_GfxImageLoadDef_DATA == 16);
    Load_Stream(1, (unsigned char*)varGfxImageLoadDef, 16);
    if (DB_GetStreamPos() != varGfxImageLoadDef->data)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\database\\../gfx_d3d/r_image_load_db.h",
            2616,
            0,
            "%s",
            "DB_GetStreamPos() == reinterpret_cast< byte * >( varGfxImageLoadDef->data )");
    varbyte = &varGfxImageLoadDef->data[0];
    Load_byteArray(1, varGfxImageLoadDef->resourceSize);
}

void __cdecl Load_GfxImage(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxImage, 36);
    DB_PushStreamPos(4u);
    varXString = &varGfxImage->name;
    Load_XString(0);
    varGfxTextureLoad = &varGfxImage->texture;
    Load_GfxTextureLoad(0);
    DB_PopStreamPos();
}

void __cdecl Load_GfxImagePtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxImagePtr, 4);
    DB_PushStreamPos(0);
    if (*varGfxImagePtr)
    {
        value = (unsigned int)*varGfxImagePtr;
        if (*varGfxImagePtr == (GfxImage *)-1 || value == -2)
        {
            *varGfxImagePtr = (GfxImage *)AllocLoad_FxElemVisStateSample();
            varGfxImage = *varGfxImagePtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GfxImage(1);
            Load_GfxImageAsset((XAssetHeader *)varGfxImagePtr);
            if (inserted)
                *inserted = *varGfxImagePtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGfxImagePtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_GfxImagePtr()
{
    if (*varGfxImagePtr)
    {
        varGfxImage = *varGfxImagePtr;
        Mark_GfxImageAsset(varGfxImage);
    }
}

void __cdecl Load_water_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varwater_t, 68);
    if (varwater_t->H0)
    {
        varwater_t->H0 = (complex_s *)AllocLoad_FxElemVisStateSample();
        varcomplex_t = varwater_t->H0;
        Load_complex_tArray(1, varwater_t->N * varwater_t->M);
    }
    if (varwater_t->wTerm)
    {
        varwater_t->wTerm = (float *)AllocLoad_FxElemVisStateSample();
        varfloat = varwater_t->wTerm;
        Load_floatArray(1, varwater_t->N * varwater_t->M);
    }
    varGfxImagePtr = &varwater_t->image;
    Load_GfxImagePtr(0);
}

void __cdecl Mark_water_t()
{
    varGfxImagePtr = &varwater_t->image;
    Mark_GfxImagePtr();
}

void __cdecl Load_GfxVertexShaderLoadDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxVertexShaderLoadDef, 8);
    if (varGfxVertexShaderLoadDef->program)
    {
        varGfxVertexShaderLoadDef->program = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varuint = varGfxVertexShaderLoadDef->program;
        Load_uintArray(1, varGfxVertexShaderLoadDef->programSize);
    }
}

void __cdecl Load_GfxPixelShaderLoadDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxPixelShaderLoadDef, 8);
    if (varGfxPixelShaderLoadDef->program)
    {
        varGfxPixelShaderLoadDef->program = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varuint = varGfxPixelShaderLoadDef->program;
        Load_uintArray(1, varGfxPixelShaderLoadDef->programSize);
    }
}

void __cdecl Load_MaterialVertexShaderProgram(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialVertexShaderProgram, 12);
    varGfxVertexShaderLoadDef = &varMaterialVertexShaderProgram->loadDef;
    Load_GfxVertexShaderLoadDef(0);
    Load_CreateMaterialVertexShader(&varMaterialVertexShaderProgram->loadDef, varMaterialVertexShader);
}

void __cdecl Load_MaterialPixelShaderProgram(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPixelShaderProgram, 12);
    varGfxPixelShaderLoadDef = &varMaterialPixelShaderProgram->loadDef;
    Load_GfxPixelShaderLoadDef(0);
    Load_CreateMaterialPixelShader(&varMaterialPixelShaderProgram->loadDef, varMaterialPixelShader);
}

void __cdecl Load_MaterialVertexShader(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialVertexShader, 16);
    varXString = &varMaterialVertexShader->name;
    Load_XString(0);
    varMaterialVertexShaderProgram = &varMaterialVertexShader->prog;
    Load_MaterialVertexShaderProgram(0);
}

void __cdecl Load_MaterialVertexShaderPtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialVertexShaderPtr, 4);
    if (*varMaterialVertexShaderPtr)
    {
        if (*varMaterialVertexShaderPtr == (MaterialVertexShader *)-1)
        {
            *varMaterialVertexShaderPtr = (MaterialVertexShader *)AllocLoad_FxElemVisStateSample();
            varMaterialVertexShader = *varMaterialVertexShaderPtr;
            Load_MaterialVertexShader(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varMaterialVertexShaderPtr);
        }
    }
}

void __cdecl Load_MaterialPixelShader(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPixelShader, 16);
    varXString = &varMaterialPixelShader->name;
    Load_XString(0);
    varMaterialPixelShaderProgram = &varMaterialPixelShader->prog;
    Load_MaterialPixelShaderProgram(0);
}

void __cdecl Load_MaterialPixelShaderPtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPixelShaderPtr, 4);
    if (*varMaterialPixelShaderPtr)
    {
        if (*varMaterialPixelShaderPtr == (MaterialPixelShader *)-1)
        {
            *varMaterialPixelShaderPtr = (MaterialPixelShader *)AllocLoad_FxElemVisStateSample();
            varMaterialPixelShader = *varMaterialPixelShaderPtr;
            Load_MaterialPixelShader(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varMaterialPixelShaderPtr);
        }
    }
}

void __cdecl Load_MaterialVertexDeclaration(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varMaterialVertexDeclaration->streamCount, 100);
}

void __cdecl Load_MaterialArgumentCodeConst(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialArgumentCodeConst, 4);
}

void __cdecl Load_MaterialArgumentDef(bool atStreamStart)
{
    switch (varMaterialShaderArgument->type)
    {
    case 1u:
    case 7u:
        if (varMaterialArgumentDef->codeSampler)
        {
            if (varMaterialArgumentDef->codeSampler == -1)
            {
                varMaterialArgumentDef->codeSampler = (unsigned int)AllocLoad_FxElemVisStateSample();
                varfloat = (float *)varMaterialArgumentDef->codeSampler;
                Load_floatArray(1, 4);
            }
            else
            {
                DB_ConvertOffsetToPointer((unsigned int*)varMaterialArgumentDef);
            }
        }
        break;
    case 3u:
    case 5u:
        if (atStreamStart)
        {
            varMaterialArgumentCodeConst = (MaterialArgumentCodeConst *)varMaterialArgumentDef;
            Load_MaterialArgumentCodeConst(atStreamStart);
        }
        break;
    case 4u:
        if (atStreamStart)
        {
            varuint = varMaterialArgumentDef;
            Load_uint(atStreamStart);
        }
        break;
    default:
        if (atStreamStart)
        {
            varuint = varMaterialArgumentDef;
            Load_uint(atStreamStart);
        }
        break;
    }
}

void __cdecl Load_MaterialShaderArgument(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialShaderArgument, 8);
    varMaterialArgumentDef = &varMaterialShaderArgument->u;
    Load_MaterialArgumentDef(0);
}

void __cdecl Load_MaterialShaderArgumentArray(bool atStreamStart, int count)
{
    MaterialShaderArgument *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialShaderArgument, 8 * count);
    var = varMaterialShaderArgument;
    for (i = 0; i < count; ++i)
    {
        varMaterialShaderArgument = var;
        Load_MaterialShaderArgument(0);
        ++var;
    }
}

void __cdecl Load_GfxStateBitsArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStateBits, 8 * count);
}

void __cdecl Load_MaterialPass(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned char*)varMaterialPass, 20);
    if (varMaterialPass->vertexDecl)
    {
        if (varMaterialPass->vertexDecl == (MaterialVertexDeclaration*)-1)
        {
            varMaterialPass->vertexDecl = (MaterialVertexDeclaration*)AllocLoad_FxElemVisStateSample();
            varMaterialVertexDeclaration = varMaterialPass->vertexDecl;
            Load_MaterialVertexDeclaration(1);
            Load_BuildVertexDecl(&varMaterialPass->vertexDecl);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varMaterialPass);
        }
    }
    varMaterialVertexShaderPtr = &varMaterialPass->vertexShader;
    Load_MaterialVertexShaderPtr(0);
    varMaterialPixelShaderPtr = &varMaterialPass->pixelShader;
    Load_MaterialPixelShaderPtr(0);
    if (varMaterialPass->args)
    {
        varMaterialPass->args = (MaterialShaderArgument*)AllocLoad_FxElemVisStateSample();
        varMaterialShaderArgument = varMaterialPass->args;
        Load_MaterialShaderArgumentArray(
            1,
            varMaterialPass->stableArgCount + varMaterialPass->perObjArgCount + varMaterialPass->perPrimArgCount);
    }
}

void __cdecl Load_MaterialPassArray(bool atStreamStart, int count)
{
    MaterialPass *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialPass, 20 * count);
    var = (MaterialPass *)varMaterialPass;
    for (i = 0; i < count; ++i)
    {
        varMaterialPass = (MaterialPass*)&var->vertexDecl;
        Load_MaterialPass(0);
        ++var;
    }
}

void __cdecl Load_MaterialTechnique(bool atStreamStart)
{
    if (!atStreamStart)
        MyAssertHandler("c:\\trees\\cod3\\src\\database\\../gfx_d3d/r_material_load_db.h", 5470, 0, "%s", "atStreamStart");
    Load_Stream(1, (unsigned __int8 *)varMaterialTechnique, 8); // 0x2668
    if (DB_GetStreamPos() != (unsigned __int8 *)varMaterialTechnique->passArray)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\database\\../gfx_d3d/r_material_load_db.h",
            5472,
            0,
            "%s",
            "DB_GetStreamPos() == reinterpret_cast< byte * >( varMaterialTechnique->passArray )");
    varMaterialPass = (MaterialPass*)&varMaterialTechnique->passArray[0].vertexDecl;
    Load_MaterialPassArray(1, varMaterialTechnique->passCount); // 0x2990
    varXString = &varMaterialTechnique->name;
    Load_XString(0); // 0x29A1
}

void __cdecl Load_MaterialTextureDefInfo(bool atStreamStart)
{
    if (varMaterialTextureDef->semantic == 11)
    {
        if (*varMaterialTextureDefInfo)
        {
            if (*varMaterialTextureDefInfo == (water_t*)-1)
            {
                *varMaterialTextureDefInfo = (water_t*)AllocLoad_FxElemVisStateSample();
                varwater_t = *varMaterialTextureDefInfo;
                Load_water_t(1);
                Load_PicmipWater(varMaterialTextureDefInfo);
            }
            else
            {
                DB_ConvertOffsetToPointer((unsigned int*)varMaterialTextureDefInfo);
            }
        }
    }
    else
    {
        varGfxImagePtr = (GfxImage **)varMaterialTextureDefInfo;
        Load_GfxImagePtr(atStreamStart);
    }
}

void __cdecl Load_MaterialTextureDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTextureDef, 12);
    varMaterialTextureDefInfo = (water_t**)&varMaterialTextureDef->u;
    Load_MaterialTextureDefInfo(0);
}

void __cdecl Load_MaterialTextureDefArray(bool atStreamStart, int count)
{
    MaterialTextureDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTextureDef, 12 * count);
    var = varMaterialTextureDef;
    for (i = 0; i < count; ++i)
    {
        varMaterialTextureDef = var;
        Load_MaterialTextureDef(0);
        ++var;
    }
}

void __cdecl Load_MaterialConstantDefArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialConstantDef, 32 * count);
}

void __cdecl Load_MaterialTechniquePtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTechniquePtr, 4);
    if (*varMaterialTechniquePtr)
    {
        if (*varMaterialTechniquePtr == (MaterialTechnique *)-1)
        {
            *varMaterialTechniquePtr = (MaterialTechnique *)AllocLoad_FxElemVisStateSample();
            varMaterialTechnique = *varMaterialTechniquePtr;
            Load_MaterialTechnique(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varMaterialTechniquePtr);
        }
    }
}

void __cdecl Load_MaterialTechniquePtrArray(bool atStreamStart, int count)
{
    MaterialTechnique **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTechniquePtr, 4 * count);
    var = varMaterialTechniquePtr;
    for (i = 0; i < count; ++i)
    {
        varMaterialTechniquePtr = var;
        Load_MaterialTechniquePtr(0);
        ++var;
    }
}

void __cdecl Load_MaterialTechniqueSet(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTechniqueSet, 148);
    DB_PushStreamPos(4u);
    varXString = &varMaterialTechniqueSet->name;
    Load_XString(0);
    varMaterialTechniquePtr = varMaterialTechniqueSet->techniques;
    Load_MaterialTechniquePtrArray(0, 34);
    DB_PopStreamPos();
}

void __cdecl Load_MaterialTechniqueSetPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialTechniqueSetPtr, 4);
    DB_PushStreamPos(0);
    if (*varMaterialTechniqueSetPtr)
    {
        value = (unsigned int)*varMaterialTechniqueSetPtr;
        if (*varMaterialTechniqueSetPtr == (MaterialTechniqueSet *)-1 || value == -2)
        {
            *varMaterialTechniqueSetPtr = (MaterialTechniqueSet *)AllocLoad_FxElemVisStateSample();
            varMaterialTechniqueSet = *varMaterialTechniqueSetPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_MaterialTechniqueSet(1);
            Load_MaterialTechniqueSetAsset((XAssetHeader *)varMaterialTechniqueSetPtr);
            if (inserted)
                *inserted = *varMaterialTechniqueSetPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varMaterialTechniqueSetPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_Material(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterial, 80);
    DB_PushStreamPos(4u);
    varMaterialInfo = &varMaterial->info;
    Load_MaterialInfo(0);
    varMaterialTechniqueSetPtr = &varMaterial->techniqueSet;
    Load_MaterialTechniqueSetPtr(0);
    if (varMaterial->textureTable)
    {
        if (varMaterial->textureTable == (MaterialTextureDef *)-1)
        {
            varMaterial->textureTable = (MaterialTextureDef *)AllocLoad_FxElemVisStateSample();
            varMaterialTextureDef = varMaterial->textureTable;
            Load_MaterialTextureDefArray(1, varMaterial->textureCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varMaterial->textureTable);
        }
    }
    if (varMaterial->constantTable)
    {
        if (varMaterial->constantTable == (MaterialConstantDef *)-1)
        {
            varMaterial->constantTable = (MaterialConstantDef *)AllocLoad_GfxPackedVertex0();
            varMaterialConstantDef = varMaterial->constantTable;
            Load_MaterialConstantDefArray(1, varMaterial->constantCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varMaterial->constantTable);
        }
    }
    if (varMaterial->stateBitsTable)
    {
        if (varMaterial->stateBitsTable == (GfxStateBits *)-1)
        {
            varMaterial->stateBitsTable = (GfxStateBits *)AllocLoad_FxElemVisStateSample();
            varGfxStateBits = varMaterial->stateBitsTable;
            Load_GfxStateBitsArray(1, varMaterial->stateBitsCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varMaterial->stateBitsTable);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_MaterialHandle(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialHandle, 4);
    DB_PushStreamPos(0);
    if (*varMaterialHandle)
    {
        value = (unsigned int)*varMaterialHandle;
        if (*varMaterialHandle == (Material *)-1 || value == -2)
        {
            *varMaterialHandle = (Material *)AllocLoad_FxElemVisStateSample();
            varMaterial = *varMaterialHandle;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_Material(1);
            Load_MaterialAsset((XAssetHeader *)varMaterialHandle);
            if (inserted)
                *inserted = *varMaterialHandle;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varMaterialHandle);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_MaterialHandleArray(bool atStreamStart, int count)
{
    Material **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialHandle, 4 * count);
    var = varMaterialHandle;
    for (i = 0; i < count; ++i)
    {
        varMaterialHandle = var;
        Load_MaterialHandle(0);
        ++var;
    }
}

void __cdecl Mark_MaterialTextureDefInfo()
{
    if (varMaterialTextureDef->semantic == 11)
    {
        if (varMaterialTextureDefInfo)
        {
            varwater_t = *(water_t**)varMaterialTextureDefInfo;
            Mark_water_t();
        }
    }
    else
    {
        varGfxImagePtr = (GfxImage **)varMaterialTextureDefInfo;
        Mark_GfxImagePtr();
    }
}

void __cdecl Mark_MaterialTextureDef()
{
    varMaterialTextureDefInfo = (water_t**)&varMaterialTextureDef->u;
    Mark_MaterialTextureDefInfo();
}

void __cdecl Mark_MaterialTextureDefArray(int count)
{
    MaterialTextureDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varMaterialTextureDef;
    for (i = 0; i < count; ++i)
    {
        varMaterialTextureDef = var;
        Mark_MaterialTextureDef();
        ++var;
    }
}

void __cdecl Mark_MaterialTechniqueSetPtr()
{
    if (*varMaterialTechniqueSetPtr)
    {
        varMaterialTechniqueSet = *varMaterialTechniqueSetPtr;
        Mark_MaterialTechniqueSetAsset(varMaterialTechniqueSet);
    }
}

void __cdecl Mark_Material()
{
    varMaterialTechniqueSetPtr = &varMaterial->techniqueSet;
    Mark_MaterialTechniqueSetPtr();
    if (varMaterial->textureTable)
    {
        varMaterialTextureDef = varMaterial->textureTable;
        Mark_MaterialTextureDefArray(varMaterial->textureCount);
    }
}

void __cdecl Mark_MaterialHandle()
{
    if (*varMaterialHandle)
    {
        varMaterial = *varMaterialHandle;
        Mark_MaterialAsset(varMaterial);
        Mark_Material();
    }
}

void __cdecl Mark_MaterialHandleArray(int count)
{
    Material **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varMaterialHandle;
    for (i = 0; i < count; ++i)
    {
        varMaterialHandle = var;
        Mark_MaterialHandle();
        ++var;
    }
}

void __cdecl Load_GfxLightImage(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightImage, 8);
    varGfxImagePtr = &varGfxLightImage->image;
    Load_GfxImagePtr(0);
}

void __cdecl Load_GfxLightDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightDef, 16);
    DB_PushStreamPos(4u);
    varXString = &varGfxLightDef->name;
    Load_XString(0);
    varGfxLightImage = &varGfxLightDef->attenuation;
    Load_GfxLightImage(0);
    DB_PopStreamPos();
}

void __cdecl Load_GfxLightDefPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightDefPtr, 4);
    DB_PushStreamPos(0);
    if (*varGfxLightDefPtr)
    {
        value = (unsigned int)*varGfxLightDefPtr;
        if (*varGfxLightDefPtr == (GfxLightDef *)-1 || value == -2)
        {
            *varGfxLightDefPtr = (GfxLightDef *)AllocLoad_FxElemVisStateSample();
            varGfxLightDef = *varGfxLightDefPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GfxLightDef(1);
            Load_LightDefAsset((XAssetHeader *)varGfxLightDefPtr);
            if (inserted)
                *inserted = *varGfxLightDefPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGfxLightDefPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxLight(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varGfxLight->type, 64);
    varGfxLightDefPtr = &varGfxLight->def;
    Load_GfxLightDefPtr(0);
}

void __cdecl Mark_GfxLightImage()
{
    varGfxImagePtr = &varGfxLightImage->image;
    Mark_GfxImagePtr();
}

void __cdecl Mark_GfxLightDef()
{
    varGfxLightImage = &varGfxLightDef->attenuation;
    Mark_GfxLightImage();
}

void __cdecl Mark_GfxLightDefPtr()
{
    if (*varGfxLightDefPtr)
    {
        varGfxLightDef = *varGfxLightDefPtr;
        Mark_LightDefAsset(varGfxLightDef);
        Mark_GfxLightDef();
    }
}

void __cdecl Mark_GfxLight()
{
    varGfxLightDefPtr = &varGfxLight->def;
    Mark_GfxLightDefPtr();
}

void __cdecl Load_GfxSurface(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxSurface, 48);
    varMaterialHandle = &varGfxSurface->material;
    Load_MaterialHandle(0);
}

void __cdecl Load_GfxSurfaceArray(bool atStreamStart, int count)
{
    GfxSurface *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxSurface, 48 * count);
    var = varGfxSurface;
    for (i = 0; i < count; ++i)
    {
        varGfxSurface = var;
        Load_GfxSurface(0);
        ++var;
    }
}

void __cdecl Load_GfxLightmapArray(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightmapArray, 8);
    varGfxImagePtr = &varGfxLightmapArray->primary;
    Load_GfxImagePtr(0);
    varGfxImagePtr = &varGfxLightmapArray->secondary;
    Load_GfxImagePtr(0);
}

void __cdecl Load_GfxLightmapArrayArray(bool atStreamStart, int count)
{
    GfxLightmapArray *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightmapArray, 8 * count);
    var = varGfxLightmapArray;
    for (i = 0; i < count; ++i)
    {
        varGfxLightmapArray = var;
        Load_GfxLightmapArray(0);
        ++var;
    }
}

void __cdecl Mark_GfxSurface()
{
    varMaterialHandle = &varGfxSurface->material;
    Mark_MaterialHandle();
}

void __cdecl Mark_GfxSurfaceArray(int count)
{
    GfxSurface *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxSurface;
    for (i = 0; i < count; ++i)
    {
        varGfxSurface = var;
        Mark_GfxSurface();
        ++var;
    }
}

void __cdecl Mark_GfxLightmapArray()
{
    varGfxImagePtr = &varGfxLightmapArray->primary;
    Mark_GfxImagePtr();
    varGfxImagePtr = &varGfxLightmapArray->secondary;
    Mark_GfxImagePtr();
}

void __cdecl Mark_GfxLightmapArrayArray(int count)
{
    GfxLightmapArray *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxLightmapArray;
    for (i = 0; i < count; ++i)
    {
        varGfxLightmapArray = var;
        Mark_GfxLightmapArray();
        ++var;
    }
}

void __cdecl Load_PhysPreset(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysPreset, 44);
    DB_PushStreamPos(4u);
    varXString = &varPhysPreset->name;
    Load_XString(0);
    varXString = &varPhysPreset->sndAliasPrefix;
    Load_XString(0);
    DB_PopStreamPos();
}

void __cdecl Load_PhysPresetPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysPresetPtr, 4);
    DB_PushStreamPos(0);
    if (*varPhysPresetPtr)
    {
        value = (unsigned int)*varPhysPresetPtr;
        if (*varPhysPresetPtr == (PhysPreset *)-1 || value == -2)
        {
            *varPhysPresetPtr = (PhysPreset *)AllocLoad_FxElemVisStateSample();
            varPhysPreset = *varPhysPresetPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_PhysPreset(1);
            Load_PhysPresetAsset((XAssetHeader *)varPhysPresetPtr);
            if (inserted)
                *inserted = *varPhysPresetPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varPhysPresetPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_PhysPresetPtr()
{
    if (*varPhysPresetPtr)
    {
        varPhysPreset = *varPhysPresetPtr;
        Mark_PhysPresetAsset(varPhysPreset);
    }
}

void __cdecl Load_cplane_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcplane_t, 20);
}

void __cdecl Load_cplane_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcplane_t, 20 * count);
}

void __cdecl Load_cbrushside_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcbrushside_t, 12);
    if (varcbrushside_t->plane)
    {
        if (varcbrushside_t->plane == (cplane_s *)-1)
        {
            varcbrushside_t->plane = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varcbrushside_t->plane;
            Load_cplane_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)(unsigned int*)varcbrushside_t);
        }
    }
}

XAsset *__cdecl AllocLoad_FxElemVisStateSample()
{
    return (XAsset *)DB_AllocStreamPos(3);
}

void __cdecl Load_cbrushside_tArray(bool atStreamStart, int count)
{
    cbrushside_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varcbrushside_t, 12 * count);
    var = varcbrushside_t;
    for (i = 0; i < count; ++i)
    {
        varcbrushside_t = var;
        Load_cbrushside_t(0);
        ++var;
    }
}

void __cdecl Load_cbrushedge_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, varcbrushedge_t, 1);
}

void __cdecl Load_cbrushedge_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varcbrushedge_t, count);
}

void __cdecl Load_XModelCollTriArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned char*)varXModelCollTri, 48 * count);
}

void __cdecl Load_XModelCollSurf(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelCollSurf, 44);
    if (varXModelCollSurf->collTris)
    {
        varXModelCollSurf->collTris = (XModelCollTri_s *)AllocLoad_FxElemVisStateSample();
        varXModelCollTri = varXModelCollSurf->collTris;
        Load_XModelCollTriArray(1, varXModelCollSurf->numCollTris);
    }
}

void __cdecl Load_XModelCollSurfArray(bool atStreamStart, int count)
{
    XModelCollSurf_s *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelCollSurf, 44 * count);
    var = varXModelCollSurf;
    for (i = 0; i < count; ++i)
    {
        varXModelCollSurf = var;
        Load_XModelCollSurf(0);
        ++var;
    }
}

void __cdecl Load_BrushWrapper(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varBrushWrapper, 80);
    if (varBrushWrapper->sides)
    {
        varBrushWrapper->sides = (cbrushside_t *)AllocLoad_FxElemVisStateSample();
        varcbrushside_t = varBrushWrapper->sides;
        Load_cbrushside_tArray(1, varBrushWrapper->numsides);
    }
    if (varBrushWrapper->baseAdjacentSide)
    {
        varBrushWrapper->baseAdjacentSide = AllocLoad_raw_byte();
        varcbrushedge_t = varBrushWrapper->baseAdjacentSide;
        Load_cbrushedge_tArray(1, varBrushWrapper->totalEdgeCount);
    }
    if (varBrushWrapper->planes)
    {
        if (varBrushWrapper->planes == (cplane_s *)-1)
        {
            varBrushWrapper->planes = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varBrushWrapper->planes;
            Load_cplane_tArray(1, varBrushWrapper->numsides);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varBrushWrapper->planes);
        }
    }
}

void __cdecl Load_PhysGeomInfo(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysGeomInfo, 68);
    if (varPhysGeomInfo->brush)
    {
        if (varPhysGeomInfo->brush == (BrushWrapper *)-1)
        {
            varPhysGeomInfo->brush = (BrushWrapper *)AllocLoad_FxElemVisStateSample();
            varBrushWrapper = varPhysGeomInfo->brush;
            Load_BrushWrapper(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varPhysGeomInfo);
        }
    }
}

void __cdecl Load_PhysGeomInfoArray(bool atStreamStart, int count)
{
    PhysGeomInfo *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysGeomInfo, 68 * count);
    var = varPhysGeomInfo;
    for (i = 0; i < count; ++i)
    {
        varPhysGeomInfo = var;
        Load_PhysGeomInfo(0);
        ++var;
    }
}

void __cdecl Load_PhysGeomList(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPhysGeomList, 44);
    if (varPhysGeomList->geoms)
    {
        varPhysGeomList->geoms = (PhysGeomInfo *)AllocLoad_FxElemVisStateSample();
        varPhysGeomInfo = varPhysGeomList->geoms;
        Load_PhysGeomInfoArray(1, varPhysGeomList->count);
    }
}

void __cdecl Load_XModel(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModel, 220);
    DB_PushStreamPos(4u);
    varXString = &varXModel->name;
    Load_XString(0);
    if (varXModel->boneNames)
    {
        if (varXModel->boneNames == (unsigned __int16 *)-1)
        {
            varXModel->boneNames = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varScriptString = varXModel->boneNames;
            Load_ScriptStringArray(1, varXModel->numBones);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXModel->boneNames);
        }
    }
    if (varXModel->parentList)
    {
        if (varXModel->parentList == (unsigned __int8 *)-1)
        {
            varXModel->parentList = AllocLoad_raw_byte();
            varbyte = varXModel->parentList;
            Load_byteArray(1, varXModel->numBones - varXModel->numRootBones);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXModel->parentList);
        }
    }
    if (varXModel->quats)
    {
        if (varXModel->quats == (__int16 *)-1)
        {
            varXModel->quats = (__int16 *)AllocLoad_XBlendInfo();
            varshort = varXModel->quats;
            Load_shortArray(1, 4 * (varXModel->numBones - varXModel->numRootBones));
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXModel->quats);
        }
    }
    if (varXModel->trans)
    {
        if (varXModel->trans == (float *)-1)
        {
            varXModel->trans = (float *)AllocLoad_FxElemVisStateSample();
            varfloat = varXModel->trans;
            Load_floatArray(1, 4 * (varXModel->numBones - varXModel->numRootBones));
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXModel->trans);
        }
    }
    if (varXModel->partClassification)
    {
        if (varXModel->partClassification == (unsigned __int8 *)-1)
        {
            varXModel->partClassification = AllocLoad_raw_byte();
            varbyte = varXModel->partClassification;
            Load_byteArray(1, varXModel->numBones);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXModel->partClassification);
        }
    }
    if (varXModel->baseMat)
    {
        if (varXModel->baseMat == (DObjAnimMat *)-1)
        {
            varXModel->baseMat = (DObjAnimMat *)AllocLoad_FxElemVisStateSample();
            varDObjAnimMat = varXModel->baseMat;
            Load_DObjAnimMatArray(1, varXModel->numBones);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXModel->baseMat);
        }
    }
    if (varXModel->surfs)
    {
        varXModel->surfs = (XSurface *)AllocLoad_FxElemVisStateSample();
        varXSurface = varXModel->surfs;
        Load_XSurfaceArray(1, varXModel->numsurfs);
    }
    if (varXModel->materialHandles)
    {
        varXModel->materialHandles = (Material **)AllocLoad_FxElemVisStateSample();
        varMaterialHandle = varXModel->materialHandles;
        Load_MaterialHandleArray(1, varXModel->numsurfs);
    }
    if (varXModel->collSurfs)
    {
        varXModel->collSurfs = (XModelCollSurf_s *)AllocLoad_FxElemVisStateSample();
        varXModelCollSurf = varXModel->collSurfs;
        Load_XModelCollSurfArray(1, varXModel->numCollSurfs);
    }
    if (varXModel->boneInfo)
    {
        varXModel->boneInfo = (XBoneInfo *)AllocLoad_FxElemVisStateSample();
        varXBoneInfo = varXModel->boneInfo;
        Load_XBoneInfoArray(1, varXModel->numBones);
    }
    varPhysPresetPtr = &varXModel->physPreset;
    Load_PhysPresetPtr(0);
    if (varXModel->physGeoms)
    {
        if (varXModel->physGeoms == (PhysGeomList *)-1)
        {
            varXModel->physGeoms = (PhysGeomList *)AllocLoad_FxElemVisStateSample();
            varPhysGeomList = varXModel->physGeoms;
            Load_PhysGeomList(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varXModel->physGeoms);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_XModelPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPtr, 4);
    DB_PushStreamPos(0);
    if (*varXModelPtr)
    {
        value = (unsigned int)*varXModelPtr;
        if (*varXModelPtr == (XModel *)-1 || value == -2)
        {
            *varXModelPtr = (XModel *)AllocLoad_FxElemVisStateSample();
            varXModel = *varXModelPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_XModel(1);
            Load_XModelAsset((XAssetHeader *)varXModelPtr);
            if (inserted)
                *inserted = *varXModelPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varXModelPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_XModelPtrArray(bool atStreamStart, int count)
{
    XModel **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPtr, 4 * count);
    var = varXModelPtr;
    for (i = 0; i < count; ++i)
    {
        varXModelPtr = var;
        Load_XModelPtr(0);
        ++var;
    }
}

void __cdecl Load_XModelPiece(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPiece, 16);
    varXModelPtr = &varXModelPiece->model;
    Load_XModelPtr(0);
}

void __cdecl Load_XModelPieceArray(bool atStreamStart, int count)
{
    XModelPiece *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPiece, 16 * count);
    var = varXModelPiece;
    for (i = 0; i < count; ++i)
    {
        varXModelPiece = var;
        Load_XModelPiece(0);
        ++var;
    }
}

void __cdecl Load_XModelPieces(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPieces, 12);
    varXString = &varXModelPieces->name;
    Load_XString(0);
    if (varXModelPieces->pieces)
    {
        varXModelPieces->pieces = (XModelPiece *)AllocLoad_FxElemVisStateSample();
        varXModelPiece = varXModelPieces->pieces;
        Load_XModelPieceArray(1, varXModelPieces->numpieces);
    }
}

void __cdecl Load_XModelPiecesPtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXModelPiecesPtr, 4);
    if (*varXModelPiecesPtr)
    {
        if (*varXModelPiecesPtr == (XModelPieces *)-1)
        {
            *varXModelPiecesPtr = (XModelPieces *)AllocLoad_FxElemVisStateSample();
            varXModelPieces = *varXModelPiecesPtr;
            Load_XModelPieces(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varXModelPiecesPtr);
        }
    }
}

void __cdecl Mark_XModel()
{
    if (varXModel->boneNames)
    {
        varScriptString = varXModel->boneNames;
        Mark_ScriptStringArray(varXModel->numBones);
    }
    if (varXModel->materialHandles)
    {
        varMaterialHandle = varXModel->materialHandles;
        Mark_MaterialHandleArray(varXModel->numsurfs);
    }
    varPhysPresetPtr = &varXModel->physPreset;
    Mark_PhysPresetPtr();
}

void __cdecl Mark_XModelPtr()
{
    if (*varXModelPtr)
    {
        varXModel = *varXModelPtr;
        Mark_XModelAsset(varXModel);
        Mark_XModel();
    }
}

void __cdecl Mark_XModelPtrArray(int count)
{
    XModel **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varXModelPtr;
    for (i = 0; i < count; ++i)
    {
        varXModelPtr = var;
        Mark_XModelPtr();
        ++var;
    }
}

void __cdecl Mark_XModelPiece()
{
    varXModelPtr = &varXModelPiece->model;
    Mark_XModelPtr();
}

void __cdecl Mark_XModelPieceArray(int count)
{
    XModelPiece *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varXModelPiece;
    for (i = 0; i < count; ++i)
    {
        varXModelPiece = var;
        Mark_XModelPiece();
        ++var;
    }
}

void __cdecl Mark_XModelPieces()
{
    if (varXModelPieces->pieces)
    {
        varXModelPiece = varXModelPieces->pieces;
        Mark_XModelPieceArray(varXModelPieces->numpieces);
    }
}

void __cdecl Mark_XModelPiecesPtr()
{
    if (*varXModelPiecesPtr)
    {
        varXModelPieces = *varXModelPiecesPtr;
        Mark_XModelPieces();
    }
}

void __cdecl Load_pathlink_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathlink_t, 12 * count);
}

void __cdecl Load_pathnode_constant_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_constant_t, 68);
    varScriptString = &varpathnode_constant_t->targetname;
    Load_ScriptString(0);
    varScriptString = &varpathnode_constant_t->script_linkName;
    Load_ScriptString(0);
    varScriptString = &varpathnode_constant_t->script_noteworthy;
    Load_ScriptString(0);
    varScriptString = &varpathnode_constant_t->target;
    Load_ScriptString(0);
    varScriptString = &varpathnode_constant_t->animscript;
    Load_ScriptString(0);
    if (varpathnode_constant_t->Links)
    {
        varpathnode_constant_t->Links = (pathlink_s *)AllocLoad_FxElemVisStateSample();
        varpathlink_t = varpathnode_constant_t->Links;
        Load_pathlink_tArray(1, varpathnode_constant_t->totalLinkCount);
    }
}

void __cdecl Load_pathnode_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_t, 128);
    varpathnode_constant_t = &varpathnode_t->constant;
    Load_pathnode_constant_t(0);
}

void __cdecl Load_pathnode_tArray(bool atStreamStart, int count)
{
    pathnode_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_t, count << 7);
    var = varpathnode_t;
    for (i = 0; i < count; ++i)
    {
        varpathnode_t = var;
        Load_pathnode_t(0);
        ++var;
    }
}

void __cdecl Load_pathbasenode_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathbasenode_t, 16 * count);
}

void __cdecl Load_pathnode_tree_nodes_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_nodes_t, 8);
    if (varpathnode_tree_nodes_t->nodes)
    {
        varpathnode_tree_nodes_t->nodes = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varpathnode_tree_nodes_t->nodes;
        Load_ushortArray(1, varpathnode_tree_nodes_t->nodeCount);
    }
}

void __cdecl Load_pathnode_tree_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_ptr, 4);
    if (*varpathnode_tree_ptr)
    {
        if (*varpathnode_tree_ptr == (pathnode_tree_t *)-1)
        {
            *varpathnode_tree_ptr = (pathnode_tree_t *)AllocLoad_FxElemVisStateSample();
            varpathnode_tree_t = *varpathnode_tree_ptr;
            Load_pathnode_tree_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varpathnode_tree_ptr);
        }
    }
}

void __cdecl Load_pathnode_tree_ptrArray(bool atStreamStart, int count)
{
    pathnode_tree_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_ptr, 4 * count);
    var = varpathnode_tree_ptr;
    for (i = 0; i < count; ++i)
    {
        varpathnode_tree_ptr = var;
        Load_pathnode_tree_ptr(0);
        ++var;
    }
}

void __cdecl Load_pathnode_tree_info_t(bool atStreamStart)
{
    if (varpathnode_tree_t->axis < 0)
    {
        varpathnode_tree_nodes_t = (pathnode_tree_nodes_t *)varpathnode_tree_info_t;
        Load_pathnode_tree_nodes_t(atStreamStart);
    }
    else
    {
        varpathnode_tree_ptr = (pathnode_tree_t **)varpathnode_tree_info_t;
        Load_pathnode_tree_ptrArray(atStreamStart, 2);
    }
}

void __cdecl Load_pathnode_tree_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_t, 16);
    varpathnode_tree_info_t = &varpathnode_tree_t->u;
    Load_pathnode_tree_info_t(0);
}

void __cdecl Load_pathnode_tree_tArray(bool atStreamStart, int count)
{
    pathnode_tree_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varpathnode_tree_t, 16 * count);
    var = varpathnode_tree_t;
    for (i = 0; i < count; ++i)
    {
        varpathnode_tree_t = var;
        Load_pathnode_tree_t(0);
        ++var;
    }
}

void __cdecl Mark_pathnode_constant_t()
{
    varScriptString = &varpathnode_constant_t->targetname;
    Mark_ScriptString();
    varScriptString = &varpathnode_constant_t->script_linkName;
    Mark_ScriptString();
    varScriptString = &varpathnode_constant_t->script_noteworthy;
    Mark_ScriptString();
    varScriptString = &varpathnode_constant_t->target;
    Mark_ScriptString();
    varScriptString = &varpathnode_constant_t->animscript;
    Mark_ScriptString();
}

void __cdecl Mark_pathnode_t()
{
    varpathnode_constant_t = &varpathnode_t->constant;
    Mark_pathnode_constant_t();
}

void __cdecl Mark_pathnode_tArray(int count)
{
    pathnode_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varpathnode_t;
    for (i = 0; i < count; ++i)
    {
        varpathnode_t = var;
        Mark_pathnode_t();
        ++var;
    }
}

void __cdecl Load_PathData(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varPathData, 40);
    if (varPathData->nodes)
    {
        varPathData->nodes = (pathnode_t *)AllocLoad_FxElemVisStateSample();
        varpathnode_t = varPathData->nodes;
        Load_pathnode_tArray(1, varPathData->nodeCount);
    }
    DB_PushStreamPos(1u);
    if (varPathData->basenodes)
    {
        varPathData->basenodes = (pathbasenode_t *)AllocLoad_GfxPackedVertex0();
        varpathbasenode_t = varPathData->basenodes;
        Load_pathbasenode_tArray(1, varPathData->nodeCount);
    }
    DB_PopStreamPos();
    if (varPathData->chainNodeForNode)
    {
        varPathData->chainNodeForNode = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varUnsignedShort = varPathData->chainNodeForNode;
        Load_UnsignedShortArray(1, varPathData->nodeCount);
    }
    if (varPathData->nodeForChainNode)
    {
        varPathData->nodeForChainNode = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varUnsignedShort = varPathData->nodeForChainNode;
        Load_UnsignedShortArray(1, varPathData->nodeCount);
    }
    if (varPathData->pathVis)
    {
        varPathData->pathVis = AllocLoad_raw_byte();
        varbyte = varPathData->pathVis;
        Load_byteArray(1, varPathData->visBytes);
    }
    if (varPathData->nodeTree)
    {
        varPathData->nodeTree = (pathnode_tree_t *)AllocLoad_FxElemVisStateSample();
        varpathnode_tree_t = varPathData->nodeTree;
        Load_pathnode_tree_tArray(1, varPathData->nodeTreeCount);
    }
}

void __cdecl Load_GameWorldSp(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGameWorldSp, 44);
    DB_PushStreamPos(4u);
    varXString = &varGameWorldSp->name;
    Load_XString(0);
    varPathData = &varGameWorldSp->path;
    Load_PathData(0);
    DB_PopStreamPos();
}

void __cdecl Load_GameWorldMp(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGameWorldMp, 4);
    DB_PushStreamPos(4u);
    varXString = &varGameWorldMp->name;
    Load_XString(0);
    DB_PopStreamPos();
}

void __cdecl Load_GameWorldSpPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGameWorldSpPtr, 4);
    DB_PushStreamPos(0);
    if (*varGameWorldSpPtr)
    {
        value = (unsigned int)*varGameWorldSpPtr;
        if (*varGameWorldSpPtr == (GameWorldSp *)-1 || value == -2)
        {
            *varGameWorldSpPtr = (GameWorldSp *)AllocLoad_FxElemVisStateSample();
            varGameWorldSp = *varGameWorldSpPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GameWorldSp(1);
            Load_GameWorldSpAsset((XAssetHeader *)varGameWorldSpPtr);
            if (inserted)
                *inserted = *varGameWorldSpPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGameWorldSpPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_GameWorldMpPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGameWorldMpPtr, 4);
    DB_PushStreamPos(0);
    if (*varGameWorldMpPtr)
    {
        value = (unsigned int)*varGameWorldMpPtr;
        if (*varGameWorldMpPtr == (GameWorldMp *)-1 || value == -2)
        {
            *varGameWorldMpPtr = (GameWorldMp *)AllocLoad_FxElemVisStateSample();
            varGameWorldMp = *varGameWorldMpPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GameWorldMp(1);
            Load_GameWorldMpAsset((XAssetHeader *)varGameWorldMpPtr);
            if (inserted)
                *inserted = *varGameWorldMpPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGameWorldMpPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_PathData()
{
    if (varPathData->nodes)
    {
        varpathnode_t = varPathData->nodes;
        Mark_pathnode_tArray(varPathData->nodeCount);
    }
}

void __cdecl Mark_GameWorldSp()
{
    varPathData = &varGameWorldSp->path;
    Mark_PathData();
}

void __cdecl Mark_GameWorldSpPtr()
{
    if (*varGameWorldSpPtr)
    {
        varGameWorldSp = *varGameWorldSpPtr;
        Mark_GameWorldSpAsset(varGameWorldSp);
        Mark_GameWorldSp();
    }
}

void __cdecl Mark_GameWorldMpPtr()
{
    if (*varGameWorldMpPtr)
    {
        varGameWorldMp = *varGameWorldMpPtr;
        Mark_GameWorldMpAsset(varGameWorldMp);
    }
}

void __cdecl Load_FxEffectDefHandle(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxEffectDefHandle, 4);
    DB_PushStreamPos(0);
    if (*varFxEffectDefHandle)
    {
        value = (unsigned int)*varFxEffectDefHandle;
        if (*varFxEffectDefHandle == (const FxEffectDef *)-1 || value == -2)
        {
            *varFxEffectDefHandle = (const FxEffectDef *)AllocLoad_FxElemVisStateSample();
            varFxEffectDef = (FxEffectDef *)*varFxEffectDefHandle;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_FxEffectDef(1);
            Load_FxEffectDefAsset((XAssetHeader *)varFxEffectDefHandle);
            if (inserted)
                *inserted = *varFxEffectDefHandle;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varFxEffectDefHandle);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_FxEffectDefHandleArray(bool atStreamStart, int count)
{
    const FxEffectDef **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxEffectDefHandle, 4 * count);
    var = varFxEffectDefHandle;
    for (i = 0; i < count; ++i)
    {
        varFxEffectDefHandle = var;
        Load_FxEffectDefHandle(0);
        ++var;
    }
}

void __cdecl Load_FxEffectDefRef(bool atStreamStart)
{
    varXString = (const char **)varFxEffectDefRef;
    Load_XString(atStreamStart);
    Load_FxEffectDefFromName((const char **)varFxEffectDefRef);
}

void __cdecl Load_FxElemMarkVisuals(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemMarkVisuals, 8);
    varMaterialHandle = (Material **)varFxElemMarkVisuals;
    Load_MaterialHandleArray(0, 2);
}

void __cdecl Load_FxElemMarkVisualsArray(bool atStreamStart, int count)
{
    FxElemMarkVisuals *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemMarkVisuals, 8 * count);
    var = varFxElemMarkVisuals;
    for (i = 0; i < count; ++i)
    {
        varFxElemMarkVisuals = var;
        Load_FxElemMarkVisuals(0);
        ++var;
    }
}

void __cdecl Load_FxElemVisuals(bool atStreamStart)
{
    switch (varFxElemDef->elemType)
    {
    case 5u:
        varXModelPtr = (XModel **)varFxElemVisuals;
        Load_XModelPtr(atStreamStart);
        break;
    case 0xAu:
        varFxEffectDefRef = (FxEffectDefRef *)varFxElemVisuals;
        Load_FxEffectDefRef(atStreamStart);
        break;
    case 8u:
        varXString = (const char **)varFxElemVisuals;
        Load_XString(atStreamStart);
        break;
    default:
        if (varFxElemDef->elemType != 6 && varFxElemDef->elemType != 7)
        {
            varMaterialHandle = (Material **)varFxElemVisuals;
            Load_MaterialHandle(atStreamStart);
        }
        break;
    }
}

void __cdecl Load_FxElemVisualsArray(bool atStreamStart, int count)
{
    FxElemVisuals *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemVisuals, 4 * count);
    var = varFxElemVisuals;
    for (i = 0; i < count; ++i)
    {
        varFxElemVisuals = var;
        Load_FxElemVisuals(0);
        ++var;
    }
}

void __cdecl Load_FxElemVisStateSampleArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, varFxElemVisStateSample->base.color, 48 * count);
}

void __cdecl Load_FxElemVelStateSampleArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemVelStateSample, 96 * count);
}

void __cdecl Load_FxElemDefVisuals(bool atStreamStart)
{
    if (varFxElemDef->elemType == 9)
    {
        if (varFxElemDefVisuals->markArray)
        {
            varFxElemDefVisuals->markArray = (FxElemMarkVisuals *)AllocLoad_FxElemVisStateSample();
            varFxElemMarkVisuals = varFxElemDefVisuals->markArray;
            Load_FxElemMarkVisualsArray(1, varFxElemDef->visualCount);
        }
    }
    else if (varFxElemDef->visualCount > 1u)
    {
        if (varFxElemDefVisuals->markArray)
        {
            varFxElemDefVisuals->markArray = (FxElemMarkVisuals *)AllocLoad_FxElemVisStateSample();
            varFxElemVisuals = (FxElemVisuals *)varFxElemDefVisuals->markArray;
            Load_FxElemVisualsArray(1, varFxElemDef->visualCount);
        }
    }
    else
    {
        varFxElemVisuals = (FxElemVisuals *)varFxElemDefVisuals;
        Load_FxElemVisuals(atStreamStart);
    }
}

void __cdecl Load_FxTrailVertexArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxTrailVertex, 20 * count);
}

void __cdecl Load_FxTrailDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxTrailDef, 28);
    if (varFxTrailDef->verts)
    {
        varFxTrailDef->verts = (FxTrailVertex *)AllocLoad_FxElemVisStateSample();
        varFxTrailVertex = varFxTrailDef->verts;
        Load_FxTrailVertexArray(1, varFxTrailDef->vertCount);
    }
    if (varFxTrailDef->inds)
    {
        varFxTrailDef->inds = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varFxTrailDef->inds;
        Load_ushortArray(1, varFxTrailDef->indCount);
    }
}

void __cdecl Load_FxElemDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemDef, 252);
    if (varFxElemDef->velSamples)
    {
        varFxElemDef->velSamples = (FxElemVelStateSample *)AllocLoad_FxElemVisStateSample();
        varFxElemVelStateSample = varFxElemDef->velSamples;
        Load_FxElemVelStateSampleArray(1, varFxElemDef->velIntervalCount + 1);
    }
    if (varFxElemDef->visSamples)
    {
        varFxElemDef->visSamples = (FxElemVisStateSample *)AllocLoad_FxElemVisStateSample();
        varFxElemVisStateSample = varFxElemDef->visSamples;
        Load_FxElemVisStateSampleArray(1, varFxElemDef->visStateIntervalCount + 1);
    }
    varFxElemDefVisuals = &varFxElemDef->visuals;
    Load_FxElemDefVisuals(0);
    varFxEffectDefRef = &varFxElemDef->effectOnImpact;
    Load_FxEffectDefRef(0);
    varFxEffectDefRef = &varFxElemDef->effectOnDeath;
    Load_FxEffectDefRef(0);
    varFxEffectDefRef = &varFxElemDef->effectEmitted;
    Load_FxEffectDefRef(0);
    if (varFxElemDef->trailDef)
    {
        varFxElemDef->trailDef = (FxTrailDef *)AllocLoad_FxElemVisStateSample();
        varFxTrailDef = varFxElemDef->trailDef;
        Load_FxTrailDef(1);
    }
}

void __cdecl Load_FxElemDefArray(bool atStreamStart, int count)
{
    FxElemDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxElemDef, 252 * count);
    var = varFxElemDef;
    for (i = 0; i < count; ++i)
    {
        varFxElemDef = var;
        Load_FxElemDef(0);
        ++var;
    }
}

void __cdecl Load_FxEffectDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxEffectDef, 32);
    DB_PushStreamPos(4u);
    varXString = &varFxEffectDef->name;
    Load_XString(0);
    if (varFxEffectDef->elemDefs)
    {
        varFxEffectDef->elemDefs = (const FxElemDef *)AllocLoad_FxElemVisStateSample();
        varFxElemDef = (FxElemDef*)varFxEffectDef->elemDefs;
        Load_FxElemDefArray(
            1,
            varFxEffectDef->elemDefCountEmission + varFxEffectDef->elemDefCountOneShot + varFxEffectDef->elemDefCountLooping);
    }
    DB_PopStreamPos();
}

void __cdecl Mark_FxEffectDefHandle()
{
    if (*varFxEffectDefHandle)
    {
        varFxEffectDef = (FxEffectDef *)*varFxEffectDefHandle;
        Mark_FxEffectDefAsset(varFxEffectDef);
        Mark_FxEffectDef();
    }
}

void __cdecl Mark_FxEffectDefHandleArray(int count)
{
    const FxEffectDef **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxEffectDefHandle;
    for (i = 0; i < count; ++i)
    {
        varFxEffectDefHandle = var;
        Mark_FxEffectDefHandle();
        ++var;
    }
}

void __cdecl Mark_FxElemMarkVisuals()
{
    varMaterialHandle = (Material **)varFxElemMarkVisuals;
    Mark_MaterialHandleArray(2);
}

void __cdecl Mark_FxElemMarkVisualsArray(int count)
{
    FxElemMarkVisuals *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxElemMarkVisuals;
    for (i = 0; i < count; ++i)
    {
        varFxElemMarkVisuals = var;
        Mark_FxElemMarkVisuals();
        ++var;
    }
}

void __cdecl Mark_FxElemVisuals()
{
    if (varFxElemDef->elemType == 5)
    {
        varXModelPtr = (XModel **)varFxElemVisuals;
        Mark_XModelPtr();
    }
    else if (varFxElemDef->elemType != 10
        && varFxElemDef->elemType != 8
        && varFxElemDef->elemType != 6
        && varFxElemDef->elemType != 7)
    {
        varMaterialHandle = (Material **)varFxElemVisuals;
        Mark_MaterialHandle();
    }
}

void __cdecl Mark_FxElemVisualsArray(int count)
{
    FxElemVisuals *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxElemVisuals;
    for (i = 0; i < count; ++i)
    {
        varFxElemVisuals = var;
        Mark_FxElemVisuals();
        ++var;
    }
}

void __cdecl Mark_FxElemDefVisuals()
{
    if (varFxElemDef->elemType == 9)
    {
        if (varFxElemDefVisuals->markArray)
        {
            varFxElemMarkVisuals = varFxElemDefVisuals->markArray;
            Mark_FxElemMarkVisualsArray(varFxElemDef->visualCount);
        }
    }
    else if (varFxElemDef->visualCount > 1u)
    {
        if (varFxElemDefVisuals->markArray)
        {
            varFxElemVisuals = (FxElemVisuals *)varFxElemDefVisuals->markArray;
            Mark_FxElemVisualsArray(varFxElemDef->visualCount);
        }
    }
    else
    {
        varFxElemVisuals = (FxElemVisuals *)varFxElemDefVisuals;
        Mark_FxElemVisuals();
    }
}

void __cdecl Mark_FxElemDef()
{
    varFxElemDefVisuals = &varFxElemDef->visuals;
    Mark_FxElemDefVisuals();
}

void __cdecl Mark_FxElemDefArray(int count)
{
    FxElemDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxElemDef;
    for (i = 0; i < count; ++i)
    {
        varFxElemDef = var;
        Mark_FxElemDef();
        ++var;
    }
}

void __cdecl Mark_FxEffectDef()
{
    if (varFxEffectDef->elemDefs)
    {
        varFxElemDef = (FxElemDef*)varFxEffectDef->elemDefs;
        Mark_FxElemDefArray(varFxEffectDef->elemDefCountEmission + varFxEffectDef->elemDefCountOneShot
            + varFxEffectDef->elemDefCountLooping);
    }
}

void __cdecl Load_DynEntityDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityDef, 96);
    varXModelPtr = &varDynEntityDef->xModel;
    Load_XModelPtr(0);
    varFxEffectDefHandle = &varDynEntityDef->destroyFx;
    Load_FxEffectDefHandle(0);
    varXModelPiecesPtr = &varDynEntityDef->destroyPieces;
    Load_XModelPiecesPtr(0);
    varPhysPresetPtr = &varDynEntityDef->physPreset;
    Load_PhysPresetPtr(0);
}

void __cdecl Load_DynEntityDefArray(bool atStreamStart, int count)
{
    DynEntityDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityDef, 96 * count);
    var = varDynEntityDef;
    for (i = 0; i < count; ++i)
    {
        varDynEntityDef = var;
        Load_DynEntityDef(0);
        ++var;
    }
}

void __cdecl Load_DynEntityCollArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityColl, 20 * count);
}

void __cdecl Load_DynEntityPoseArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityPose, 32 * count);
}

void __cdecl Load_DynEntityClientArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varDynEntityClient, 12 * count);
}

void __cdecl Mark_DynEntityDef()
{
    varXModelPtr = &varDynEntityDef->xModel;
    Mark_XModelPtr();
    varFxEffectDefHandle = &varDynEntityDef->destroyFx;
    Mark_FxEffectDefHandle();
    varXModelPiecesPtr = &varDynEntityDef->destroyPieces;
    Mark_XModelPiecesPtr();
    varPhysPresetPtr = &varDynEntityDef->physPreset;
    Mark_PhysPresetPtr();
}

void __cdecl Mark_DynEntityDefArray(int count)
{
    DynEntityDef *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varDynEntityDef;
    for (i = 0; i < count; ++i)
    {
        varDynEntityDef = var;
        Mark_DynEntityDef();
        ++var;
    }
}

void __cdecl Load_MapEnts(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMapEnts, 12);
    DB_PushStreamPos(4u);
    varXString = &varMapEnts->name;
    Load_XString(0);
    if (varMapEnts->entityString)
    {
        varMapEnts->entityString = (char *)AllocLoad_raw_byte();
        varchar = varMapEnts->entityString;
        Load_charArray(1, varMapEnts->numEntityChars);
    }
    DB_PopStreamPos();
}

void __cdecl Load_MapEntsPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMapEntsPtr, 4);
    DB_PushStreamPos(0);
    if (*varMapEntsPtr)
    {
        value = (unsigned int)*varMapEntsPtr;
        if (*varMapEntsPtr == (MapEnts *)-1 || value == -2)
        {
            *varMapEntsPtr = (MapEnts *)AllocLoad_FxElemVisStateSample();
            varMapEnts = *varMapEntsPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_MapEnts(1);
            Load_MapEntsAsset((XAssetHeader *)varMapEntsPtr);
            if (inserted)
                *inserted = *varMapEntsPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varMapEntsPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_MapEntsPtr()
{
    if (*varMapEntsPtr)
    {
        varMapEnts = *varMapEntsPtr;
        Mark_MapEntsAsset(varMapEnts);
    }
}

void __cdecl Load_cStaticModel_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcStaticModel_t, 80);
    varXModelPtr = &varcStaticModel_t->xmodel;
    Load_XModelPtr(0);
}

void __cdecl Load_cStaticModel_tArray(bool atStreamStart, int count)
{
    cStaticModel_s *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varcStaticModel_t, 80 * count);
    var = varcStaticModel_t;
    for (i = 0; i < count; ++i)
    {
        varcStaticModel_t = var;
        Load_cStaticModel_t(0);
        ++var;
    }
}

void __cdecl Load_cNode_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcNode_t, 8);
    if (varcNode_t->plane)
    {
        if (varcNode_t->plane == (cplane_s *)-1)
        {
            varcNode_t->plane = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varcNode_t->plane;
            Load_cplane_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varcNode_t);
        }
    }
}

void __cdecl Load_cNode_tArray(bool atStreamStart, int count)
{
    cNode_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varcNode_t, 8 * count);
    var = varcNode_t;
    for (i = 0; i < count; ++i)
    {
        varcNode_t = var;
        Load_cNode_t(0);
        ++var;
    }
}

void __cdecl Load_cLeaf_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcLeaf_t, 44 * count);
}

void __cdecl Load_cLeafBrushNodeLeaf_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcLeafBrushNodeLeaf_t, 4);
    if (varcLeafBrushNodeLeaf_t->brushes)
    {
        if (varcLeafBrushNodeLeaf_t->brushes == (unsigned __int16 *)-1)
        {
            varcLeafBrushNodeLeaf_t->brushes = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varLeafBrush = varcLeafBrushNodeLeaf_t->brushes;
            Load_LeafBrushArray(1, varcLeafBrushNode_t->leafBrushCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varcLeafBrushNodeLeaf_t);
        }
    }
}

void __cdecl Load_cLeafBrushNodeChildren_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcLeafBrushNodeChildren_t, 12);
}

void __cdecl Load_cLeafBrushNodeData_t(bool atStreamStart)
{
    if (varcLeafBrushNode_t->leafBrushCount <= 0)
    {
        if (atStreamStart)
        {
            varcLeafBrushNodeChildren_t = (cLeafBrushNodeChildren_t *)varcLeafBrushNodeData_t;
            Load_cLeafBrushNodeChildren_t(atStreamStart);
        }
    }
    else
    {
        varcLeafBrushNodeLeaf_t = &varcLeafBrushNodeData_t->leaf;
        Load_cLeafBrushNodeLeaf_t(atStreamStart);
    }
}

void __cdecl Load_cLeafBrushNode_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varcLeafBrushNode_t->axis, 20);
    varcLeafBrushNodeData_t = &varcLeafBrushNode_t->data;
    Load_cLeafBrushNodeData_t(0);
}

void __cdecl Load_cLeafBrushNode_tArray(bool atStreamStart, int count)
{
    cLeafBrushNode_s *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, &varcLeafBrushNode_t->axis, 20 * count);
    var = varcLeafBrushNode_t;
    for (i = 0; i < count; ++i)
    {
        varcLeafBrushNode_t = var;
        Load_cLeafBrushNode_t(0);
        ++var;
    }
}

void __cdecl Load_CollisionBorder(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varCollisionBorder, 28);
}

void __cdecl Load_CollisionBorderArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varCollisionBorder, 28 * count);
}

void __cdecl Load_CollisionPartition(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varCollisionPartition->triCount, 12);
    if (varCollisionPartition->borders)
    {
        if (varCollisionPartition->borders == (CollisionBorder *)-1)
        {
            varCollisionPartition->borders = (CollisionBorder *)AllocLoad_FxElemVisStateSample();
            varCollisionBorder = varCollisionPartition->borders;
            Load_CollisionBorder(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varCollisionPartition->borders);
        }
    }
}

void __cdecl Load_CollisionPartitionArray(bool atStreamStart, int count)
{
    CollisionPartition *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, &varCollisionPartition->triCount, 12 * count);
    var = varCollisionPartition;
    for (i = 0; i < count; ++i)
    {
        varCollisionPartition = var;
        Load_CollisionPartition(0);
        ++var;
    }
}

void __cdecl Load_CollisionAabbTreeArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varCollisionAabbTree, 32 * count);
}

void __cdecl Load_cmodel_tArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcmodel_t, 72 * count);
}

void __cdecl Load_cbrush_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varcbrush_t, 80);
    if (varcbrush_t->sides)
    {
        if (varcbrush_t->sides == (cbrushside_t *)-1)
        {
            varcbrush_t->sides = (cbrushside_t *)AllocLoad_FxElemVisStateSample();
            varcbrushside_t = varcbrush_t->sides;
            Load_cbrushside_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varcbrush_t->sides);
        }
    }
    if (varcbrush_t->baseAdjacentSide)
    {
        if (varcbrush_t->baseAdjacentSide == (unsigned __int8 *)-1)
        {
            varcbrush_t->baseAdjacentSide = AllocLoad_raw_byte();
            varcbrushedge_t = varcbrush_t->baseAdjacentSide;
            Load_cbrushedge_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varcbrush_t->baseAdjacentSide);
        }
    }
}

void __cdecl Load_cbrush_tArray(bool atStreamStart, int count)
{
    cbrush_t *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varcbrush_t, 80 * count);
    var = varcbrush_t;
    for (i = 0; i < count; ++i)
    {
        varcbrush_t = var;
        Load_cbrush_t(0);
        ++var;
    }
}

void __cdecl Load_LeafBrushArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varLeafBrush, 2 * count);
}

void __cdecl Load_clipMap_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varclipMap_t, 284);
    DB_PushStreamPos(4u);
    varXString = &varclipMap_t->name;
    Load_XString(0);
    if (varclipMap_t->planes)
    {
        if (varclipMap_t->planes == (cplane_s *)-1)
        {
            varclipMap_t->planes = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varclipMap_t->planes;
            Load_cplane_tArray(1, varclipMap_t->planeCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varclipMap_t->planes);
        }
    }
    if (varclipMap_t->staticModelList)
    {
        varclipMap_t->staticModelList = (cStaticModel_s *)AllocLoad_FxElemVisStateSample();
        varcStaticModel_t = varclipMap_t->staticModelList;
        Load_cStaticModel_tArray(1, varclipMap_t->numStaticModels);
    }
    if (varclipMap_t->materials)
    {
        varclipMap_t->materials = (dmaterial_t *)AllocLoad_FxElemVisStateSample();
        vardmaterial_t = varclipMap_t->materials;
        Load_dmaterial_tArray(1, varclipMap_t->numMaterials);
    }
    if (varclipMap_t->brushsides)
    {
        varclipMap_t->brushsides = (cbrushside_t *)AllocLoad_FxElemVisStateSample();
        varcbrushside_t = varclipMap_t->brushsides;
        Load_cbrushside_tArray(1, varclipMap_t->numBrushSides);
    }
    if (varclipMap_t->brushEdges)
    {
        varclipMap_t->brushEdges = AllocLoad_raw_byte();
        varcbrushedge_t = varclipMap_t->brushEdges;
        Load_cbrushedge_tArray(1, varclipMap_t->numBrushEdges);
    }
    if (varclipMap_t->nodes)
    {
        varclipMap_t->nodes = (cNode_t *)AllocLoad_FxElemVisStateSample();
        varcNode_t = varclipMap_t->nodes;
        Load_cNode_tArray(1, varclipMap_t->numNodes);
    }
    if (varclipMap_t->leafs)
    {
        varclipMap_t->leafs = (cLeaf_t *)AllocLoad_FxElemVisStateSample();
        varcLeaf_t = varclipMap_t->leafs;
        Load_cLeaf_tArray(1, varclipMap_t->numLeafs);
    }
    if (varclipMap_t->leafbrushes)
    {
        varclipMap_t->leafbrushes = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varLeafBrush = varclipMap_t->leafbrushes;
        Load_LeafBrushArray(1, varclipMap_t->numLeafBrushes);
    }
    if (varclipMap_t->leafbrushNodes)
    {
        varclipMap_t->leafbrushNodes = (cLeafBrushNode_s *)AllocLoad_FxElemVisStateSample();
        varcLeafBrushNode_t = varclipMap_t->leafbrushNodes;
        Load_cLeafBrushNode_tArray(1, varclipMap_t->leafbrushNodesCount);
    }
    if (varclipMap_t->leafsurfaces)
    {
        varclipMap_t->leafsurfaces = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varuint = varclipMap_t->leafsurfaces;
        Load_uintArray(1, varclipMap_t->numLeafSurfaces);
    }
    if (varclipMap_t->verts)
    {
        varclipMap_t->verts = (float (*)[3])AllocLoad_FxElemVisStateSample();
        varvec3_t = varclipMap_t->verts;
        Load_vec3_tArray(1, varclipMap_t->vertCount);
    }
    if (varclipMap_t->triIndices)
    {
        varclipMap_t->triIndices = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varUnsignedShort = varclipMap_t->triIndices;
        Load_UnsignedShortArray(1, 3 * varclipMap_t->triCount);
    }
    if (varclipMap_t->triEdgeIsWalkable)
    {
        varclipMap_t->triEdgeIsWalkable = AllocLoad_raw_byte();
        varbyte = varclipMap_t->triEdgeIsWalkable;
        Load_byteArray(1, 4 * ((3 * varclipMap_t->triCount + 31) >> 5));
    }
    if (varclipMap_t->borders)
    {
        varclipMap_t->borders = (CollisionBorder *)AllocLoad_FxElemVisStateSample();
        varCollisionBorder = varclipMap_t->borders;
        Load_CollisionBorderArray(1, varclipMap_t->borderCount);
    }
    if (varclipMap_t->partitions)
    {
        varclipMap_t->partitions = (CollisionPartition *)AllocLoad_FxElemVisStateSample();
        varCollisionPartition = varclipMap_t->partitions;
        Load_CollisionPartitionArray(1, varclipMap_t->partitionCount);
    }
    if (varclipMap_t->aabbTrees)
    {
        varclipMap_t->aabbTrees = (CollisionAabbTree *)AllocLoad_FxElemVisStateSample();
        varCollisionAabbTree = varclipMap_t->aabbTrees;
        Load_CollisionAabbTreeArray(1, varclipMap_t->aabbTreeCount);
    }
    if (varclipMap_t->cmodels)
    {
        varclipMap_t->cmodels = (cmodel_t *)AllocLoad_FxElemVisStateSample();
        varcmodel_t = varclipMap_t->cmodels;
        Load_cmodel_tArray(1, varclipMap_t->numSubModels);
    }
    if (varclipMap_t->brushes)
    {
        varclipMap_t->brushes = AllocLoad_GfxPackedVertex0();
        varcbrush_t = varclipMap_t->brushes;
        Load_cbrush_tArray(1, varclipMap_t->numBrushes);
    }
    if (varclipMap_t->visibility)
    {
        varclipMap_t->visibility = AllocLoad_raw_byte();
        varbyte = varclipMap_t->visibility;
        Load_byteArray(1, varclipMap_t->numClusters * varclipMap_t->clusterBytes);
    }
    varMapEntsPtr = &varclipMap_t->mapEnts;
    Load_MapEntsPtr(0);
    if (varclipMap_t->box_brush)
    {
        if (varclipMap_t->box_brush == (cbrush_t *)-1)
        {
            varclipMap_t->box_brush = AllocLoad_GfxPackedVertex0();
            varcbrush_t = varclipMap_t->box_brush;
            Load_cbrush_t(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varclipMap_t->box_brush);
        }
    }
    if (varclipMap_t->dynEntDefList[0])
    {
        varclipMap_t->dynEntDefList[0] = (DynEntityDef *)AllocLoad_FxElemVisStateSample();
        varDynEntityDef = varclipMap_t->dynEntDefList[0];
        Load_DynEntityDefArray(1, varclipMap_t->dynEntCount[0]);
    }
    if (varclipMap_t->dynEntDefList[1])
    {
        varclipMap_t->dynEntDefList[1] = (DynEntityDef *)AllocLoad_FxElemVisStateSample();
        varDynEntityDef = varclipMap_t->dynEntDefList[1];
        Load_DynEntityDefArray(1, varclipMap_t->dynEntCount[1]);
    }
    DB_PushStreamPos(1u);
    if (varclipMap_t->dynEntPoseList[0])
    {
        varclipMap_t->dynEntPoseList[0] = (DynEntityPose *)AllocLoad_FxElemVisStateSample();
        varDynEntityPose = varclipMap_t->dynEntPoseList[0];
        Load_DynEntityPoseArray(1, varclipMap_t->dynEntCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varclipMap_t->dynEntPoseList[1])
    {
        varclipMap_t->dynEntPoseList[1] = (DynEntityPose *)AllocLoad_FxElemVisStateSample();
        varDynEntityPose = varclipMap_t->dynEntPoseList[1];
        Load_DynEntityPoseArray(1, varclipMap_t->dynEntCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varclipMap_t->dynEntClientList[0])
    {
        varclipMap_t->dynEntClientList[0] = (DynEntityClient *)AllocLoad_FxElemVisStateSample();
        varDynEntityClient = varclipMap_t->dynEntClientList[0];
        Load_DynEntityClientArray(1, varclipMap_t->dynEntCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varclipMap_t->dynEntClientList[1])
    {
        varclipMap_t->dynEntClientList[1] = (DynEntityClient *)AllocLoad_FxElemVisStateSample();
        varDynEntityClient = varclipMap_t->dynEntClientList[1];
        Load_DynEntityClientArray(1, varclipMap_t->dynEntCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varclipMap_t->dynEntCollList[0])
    {
        varclipMap_t->dynEntCollList[0] = (DynEntityColl *)AllocLoad_FxElemVisStateSample();
        varDynEntityColl = varclipMap_t->dynEntCollList[0];
        Load_DynEntityCollArray(1, varclipMap_t->dynEntCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varclipMap_t->dynEntCollList[1])
    {
        varclipMap_t->dynEntCollList[1] = (DynEntityColl *)AllocLoad_FxElemVisStateSample();
        varDynEntityColl = varclipMap_t->dynEntCollList[1];
        Load_DynEntityCollArray(1, varclipMap_t->dynEntCount[1]);
    }
    DB_PopStreamPos();
    DB_PopStreamPos();
}

void __cdecl Load_clipMap_ptr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varclipMap_ptr, 4);
    DB_PushStreamPos(0);
    if (*varclipMap_ptr)
    {
        value = (unsigned int)*varclipMap_ptr;
        if (*varclipMap_ptr == (clipMap_t *)-1 || value == -2)
        {
            *varclipMap_ptr = (clipMap_t *)AllocLoad_FxElemVisStateSample();
            varclipMap_t = *varclipMap_ptr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_clipMap_t(1);
            Load_ClipMapAsset((XAssetHeader *)varclipMap_ptr);
            if (inserted)
                *inserted = *varclipMap_ptr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varclipMap_ptr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_cStaticModel_t()
{
    varXModelPtr = &varcStaticModel_t->xmodel;
    Mark_XModelPtr();
}

void __cdecl Mark_cStaticModel_tArray(int count)
{
    cStaticModel_s *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varcStaticModel_t;
    for (i = 0; i < count; ++i)
    {
        varcStaticModel_t = var;
        Mark_cStaticModel_t();
        ++var;
    }
}

void __cdecl Mark_clipMap_t()
{
    if (varclipMap_t->staticModelList)
    {
        varcStaticModel_t = varclipMap_t->staticModelList;
        Mark_cStaticModel_tArray(varclipMap_t->numStaticModels);
    }
    varMapEntsPtr = &varclipMap_t->mapEnts;
    Mark_MapEntsPtr();
    if (varclipMap_t->dynEntDefList[0])
    {
        varDynEntityDef = varclipMap_t->dynEntDefList[0];
        Mark_DynEntityDefArray(varclipMap_t->dynEntCount[0]);
    }
    if (varclipMap_t->dynEntDefList[1])
    {
        varDynEntityDef = varclipMap_t->dynEntDefList[1];
        Mark_DynEntityDefArray(varclipMap_t->dynEntCount[1]);
    }
}

void __cdecl Mark_clipMap_ptr()
{
    if (*varclipMap_ptr)
    {
        varclipMap_t = *varclipMap_ptr;
        Mark_ClipMapAsset(varclipMap_t);
        Mark_clipMap_t();
    }
}

void __cdecl Load_ComPrimaryLight(bool atStreamStart)
{
    Load_Stream(atStreamStart, &varComPrimaryLight->type, 68);
    varXString = &varComPrimaryLight->defName;
    Load_XString(0);
}

void __cdecl Load_ComPrimaryLightArray(bool atStreamStart, int count)
{
    ComPrimaryLight *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, &varComPrimaryLight->type, 68 * count);
    var = varComPrimaryLight;
    for (i = 0; i < count; ++i)
    {
        varComPrimaryLight = var;
        Load_ComPrimaryLight(0);
        ++var;
    }
}

void __cdecl Load_ComWorld(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varComWorld, 16);
    DB_PushStreamPos(4u);
    varXString = &varComWorld->name;
    Load_XString(0);
    if (varComWorld->primaryLights)
    {
        varComWorld->primaryLights = (ComPrimaryLight *)AllocLoad_FxElemVisStateSample();
        varComPrimaryLight = varComWorld->primaryLights;
        Load_ComPrimaryLightArray(1, varComWorld->primaryLightCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_ComWorldPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varComWorldPtr, 4);
    DB_PushStreamPos(0);
    if (*varComWorldPtr)
    {
        value = (unsigned int)*varComWorldPtr;
        if (*varComWorldPtr == (ComWorld *)-1 || value == -2)
        {
            *varComWorldPtr = (ComWorld *)AllocLoad_FxElemVisStateSample();
            varComWorld = *varComWorldPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_ComWorld(1);
            Load_ComWorldAsset((XAssetHeader *)varComWorldPtr);
            if (inserted)
                *inserted = *varComWorldPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varComWorldPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_ComWorldPtr()
{
    if (*varComWorldPtr)
    {
        varComWorld = *varComWorldPtr;
        Mark_ComWorldAsset(varComWorld);
    }
}

void __cdecl Load_operandInternalDataUnion(bool atStreamStart)
{
    if (varOperand->dataType)
    {
        if (varOperand->dataType == VAL_FLOAT)
        {
            if (atStreamStart)
            {
                varfloat = &varoperandInternalDataUnion->floatVal;
                Load_float(atStreamStart);
            }
        }
        else if (varOperand->dataType == VAL_STRING)
        {
            varXString = (const char **)varoperandInternalDataUnion;
            Load_XString(atStreamStart);
        }
    }
    else if (atStreamStart)
    {
        varint = varoperandInternalDataUnion;
        Load_int(atStreamStart);
    }
}

void __cdecl Load_Operand(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varOperand, 8);
    varoperandInternalDataUnion = &varOperand->internals;
    Load_operandInternalDataUnion(0);
}

void __cdecl Load_Operator(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varOperator, 4);
}

void __cdecl Load_entryInternalData(bool atStreamStart)
{
    if (varexpressionEntry->type)
    {
        varOperand = (Operand *)varentryInternalData;
        Load_Operand(atStreamStart);
    }
    else if (atStreamStart)
    {
        varOperator = &varentryInternalData->op;
        Load_Operator(atStreamStart);
    }
}

void __cdecl Load_expressionEntry(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varexpressionEntry, 12);
    varentryInternalData = &varexpressionEntry->data;
    Load_entryInternalData(0);
}

void __cdecl Load_expressionEntry_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varexpressionEntry_ptr, 4);
    if (*varexpressionEntry_ptr)
    {
        *varexpressionEntry_ptr = (expressionEntry *)AllocLoad_FxElemVisStateSample();
        varexpressionEntry = *varexpressionEntry_ptr;
        Load_expressionEntry(1);
    }
}

void __cdecl Load_expressionEntry_ptrArray(bool atStreamStart, int count)
{
    expressionEntry **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varexpressionEntry_ptr, 4 * count);
    var = varexpressionEntry_ptr;
    for (i = 0; i < count; ++i)
    {
        varexpressionEntry_ptr = var;
        Load_expressionEntry_ptr(0);
        ++var;
    }
}

void __cdecl Load_statement(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varstatement, 8);
    if (varstatement->entries)
    {
        varstatement->entries = (expressionEntry **)AllocLoad_FxElemVisStateSample();
        varexpressionEntry_ptr = varstatement->entries;
        Load_expressionEntry_ptrArray(1, varstatement->numEntries);
    }
}

void __cdecl Load_listBoxDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varlistBoxDef_t, 340);
    varXString = &varlistBoxDef_t->doubleClick;
    Load_XString(0);
    varMaterialHandle = &varlistBoxDef_t->selectIcon;
    Load_MaterialHandle(0);
}

void __cdecl Load_listBoxDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varlistBoxDef_ptr, 4);
    if (*varlistBoxDef_ptr)
    {
        *varlistBoxDef_ptr = (listBoxDef_s *)AllocLoad_FxElemVisStateSample();
        varlistBoxDef_t = *varlistBoxDef_ptr;
        Load_listBoxDef_t(1);
    }
}

void __cdecl Load_editFieldDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vareditFieldDef_t, 32);
}

void __cdecl Load_editFieldDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)vareditFieldDef_ptr, 4);
    if (*vareditFieldDef_ptr)
    {
        *vareditFieldDef_ptr = (editFieldDef_s *)AllocLoad_FxElemVisStateSample();
        vareditFieldDef_t = *vareditFieldDef_ptr;
        Load_editFieldDef_t(1);
    }
}

void __cdecl Load_multiDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varmultiDef_t, 392);
    varXString = (const char **)varmultiDef_t;
    Load_XStringArray(0, 32);
    varXString = varmultiDef_t->dvarStr;
    Load_XStringArray(0, 32);
}

void __cdecl Load_multiDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varmultiDef_ptr, 4);
    if (*varmultiDef_ptr)
    {
        *varmultiDef_ptr = (multiDef_s *)AllocLoad_FxElemVisStateSample();
        varmultiDef_t = *varmultiDef_ptr;
        Load_multiDef_t(1);
    }
}

void __cdecl Load_windowDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varwindowDef_t, 156);
    varXString = &varwindowDef_t->name;
    Load_XString(0);
    varXString = &varwindowDef_t->group;
    Load_XString(0);
    varMaterialHandle = &varwindowDef_t->background;
    Load_MaterialHandle(0);
}

void __cdecl Load_Window(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varWindow, 156);
    varwindowDef_t = varWindow;
    Load_windowDef_t(0);
}

void __cdecl Load_ItemKeyHandler(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varItemKeyHandler, 12);
    varXString = &varItemKeyHandler->action;
    Load_XString(0);
    if (varItemKeyHandler->next)
    {
        varItemKeyHandler->next = (ItemKeyHandler *)AllocLoad_FxElemVisStateSample();
        varItemKeyHandlerNext = varItemKeyHandler->next;
        Load_ItemKeyHandlerNext(1);
    }
}

void __cdecl Load_ItemKeyHandlerNext(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varItemKeyHandlerNext, 12);
    varItemKeyHandler = varItemKeyHandlerNext;
    Load_ItemKeyHandler(0);
}

void __cdecl Load_itemDefData_t(bool atStreamStart)
{
    switch (varitemDef_t->type)
    {
    case 6:
        varlistBoxDef_ptr = &varitemDefData_t->listBox;
        Load_listBoxDef_ptr(atStreamStart);
        break;
    case 4:
    case 9:
    case 0x10:
    case 0x12:
    case 0xB:
    case 0xE:
    case 0xA:
    case 0:
    case 0x11:
        vareditFieldDef_ptr = (editFieldDef_s **)varitemDefData_t;
        Load_editFieldDef_ptr(atStreamStart);
        break;
    case 0xC:
        varmultiDef_ptr = (multiDef_s **)varitemDefData_t;
        Load_multiDef_ptr(atStreamStart);
        break;
    case 0xD:
        varXString = (const char **)varitemDefData_t;
        Load_XString(atStreamStart);
        break;
    }
}

void __cdecl Load_itemDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varitemDef_t, 372);
    varWindow = &varitemDef_t->window;
    Load_Window(0);
    varXString = &varitemDef_t->text;
    Load_XString(0);
    varXString = &varitemDef_t->mouseEnterText;
    Load_XString(0);
    varXString = &varitemDef_t->mouseExitText;
    Load_XString(0);
    varXString = &varitemDef_t->mouseEnter;
    Load_XString(0);
    varXString = &varitemDef_t->mouseExit;
    Load_XString(0);
    varXString = &varitemDef_t->action;
    Load_XString(0);
    varXString = &varitemDef_t->onAccept;
    Load_XString(0);
    varXString = &varitemDef_t->onFocus;
    Load_XString(0);
    varXString = &varitemDef_t->leaveFocus;
    Load_XString(0);
    varXString = &varitemDef_t->dvar;
    Load_XString(0);
    varXString = &varitemDef_t->dvarTest;
    Load_XString(0);
    if (varitemDef_t->onKey)
    {
        varitemDef_t->onKey = (ItemKeyHandler *)AllocLoad_FxElemVisStateSample();
        varItemKeyHandler = varitemDef_t->onKey;
        Load_ItemKeyHandler(1);
    }
    varXString = &varitemDef_t->enableDvar;
    Load_XString(0);
    varsnd_alias_list_ptr = &varitemDef_t->focusSound;
    Load_snd_alias_list_ptr(0);
    varitemDefData_t = &varitemDef_t->typeData;
    Load_itemDefData_t(0);
    varstatement = &varitemDef_t->visibleExp;
    Load_statement(0);
    varstatement = &varitemDef_t->textExp;
    Load_statement(0);
    varstatement = &varitemDef_t->materialExp;
    Load_statement(0);
    varstatement = &varitemDef_t->rectXExp;
    Load_statement(0);
    varstatement = &varitemDef_t->rectYExp;
    Load_statement(0);
    varstatement = &varitemDef_t->rectWExp;
    Load_statement(0);
    varstatement = &varitemDef_t->rectHExp;
    Load_statement(0);
    varstatement = &varitemDef_t->forecolorAExp;
    Load_statement(0);
}

void __cdecl Load_itemDef_ptr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varitemDef_ptr, 4);
    if (*varitemDef_ptr)
    {
        *varitemDef_ptr = (itemDef_s *)AllocLoad_FxElemVisStateSample();
        varitemDef_t = *varitemDef_ptr;
        Load_itemDef_t(1);
    }
}

void __cdecl Load_itemDef_ptrArray(bool atStreamStart, int count)
{
    itemDef_s **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varitemDef_ptr, 4 * count);
    var = varitemDef_ptr;
    for (i = 0; i < count; ++i)
    {
        varitemDef_ptr = var;
        Load_itemDef_ptr(0);
        ++var;
    }
}

void __cdecl Load_menuDef_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varmenuDef_t, 284);
    DB_PushStreamPos(4u);
    varWindow = &varmenuDef_t->window;
    Load_Window(0);
    varXString = &varmenuDef_t->font;
    Load_XString(0);
    varXString = &varmenuDef_t->onOpen;
    Load_XString(0);
    varXString = &varmenuDef_t->onClose;
    Load_XString(0);
    varXString = &varmenuDef_t->onESC;
    Load_XString(0);
    if (varmenuDef_t->onKey)
    {
        varmenuDef_t->onKey = (ItemKeyHandler *)AllocLoad_FxElemVisStateSample();
        varItemKeyHandler = varmenuDef_t->onKey;
        Load_ItemKeyHandler(1);
    }
    varstatement = &varmenuDef_t->visibleExp;
    Load_statement(0);
    varXString = &varmenuDef_t->allowedBinding;
    Load_XString(0);
    varXString = &varmenuDef_t->soundName;
    Load_XString(0);
    varstatement = &varmenuDef_t->rectXExp;
    Load_statement(0);
    varstatement = &varmenuDef_t->rectYExp;
    Load_statement(0);
    if (varmenuDef_t->items)
    {
        varmenuDef_t->items = (itemDef_s **)AllocLoad_FxElemVisStateSample();
        varitemDef_ptr = varmenuDef_t->items;
        Load_itemDef_ptrArray(1, varmenuDef_t->itemCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_menuDef_ptr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varmenuDef_ptr, 4);
    DB_PushStreamPos(0);
    if (*varmenuDef_ptr)
    {
        value = (unsigned int)*varmenuDef_ptr;
        if (*varmenuDef_ptr == (menuDef_t *)-1 || value == -2)
        {
            *varmenuDef_ptr = (menuDef_t *)AllocLoad_FxElemVisStateSample();
            varmenuDef_t = *varmenuDef_ptr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_menuDef_t(1);
            Load_MenuAsset((XAssetHeader *)varmenuDef_ptr);
            if (inserted)
                *inserted = *varmenuDef_ptr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varmenuDef_ptr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_menuDef_ptrArray(bool atStreamStart, int count)
{
    menuDef_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varmenuDef_ptr, 4 * count);
    var = varmenuDef_ptr;
    for (i = 0; i < count; ++i)
    {
        varmenuDef_ptr = var;
        Load_menuDef_ptr(0);
        ++var;
    }
}

void __cdecl Load_MenuList(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMenuList, 12);
    DB_PushStreamPos(4u);
    varXString = &varMenuList->name;
    Load_XString(0);
    if (varMenuList->menus)
    {
        varMenuList->menus = (menuDef_t **)AllocLoad_FxElemVisStateSample();
        varmenuDef_ptr = varMenuList->menus;
        Load_menuDef_ptrArray(1, varMenuList->menuCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_MenuListPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMenuListPtr, 4);
    DB_PushStreamPos(0);
    if (*varMenuListPtr)
    {
        value = (unsigned int)*varMenuListPtr;
        if (*varMenuListPtr == (MenuList *)-1 || value == -2)
        {
            *varMenuListPtr = (MenuList *)AllocLoad_FxElemVisStateSample();
            varMenuList = *varMenuListPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_MenuList(1);
            Load_MenuListAsset((XAssetHeader *)varMenuListPtr);
            if (inserted)
                *inserted = *varMenuListPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varMenuListPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_listBoxDef_t()
{
    varMaterialHandle = &varlistBoxDef_t->selectIcon;
    Mark_MaterialHandle();
}

void __cdecl Mark_listBoxDef_ptr()
{
    if (*varlistBoxDef_ptr)
    {
        varlistBoxDef_t = *varlistBoxDef_ptr;
        Mark_listBoxDef_t();
    }
}

void __cdecl Mark_windowDef_t()
{
    varMaterialHandle = &varwindowDef_t->background;
    Mark_MaterialHandle();
}

void __cdecl Mark_Window()
{
    varwindowDef_t = varWindow;
    Mark_windowDef_t();
}

void __cdecl Mark_itemDefData_t()
{
    if (varitemDef_t->type == 6)
    {
        varlistBoxDef_ptr = &varitemDefData_t->listBox;
        Mark_listBoxDef_ptr();
    }
}

void __cdecl Mark_itemDef_t()
{
    varWindow = &varitemDef_t->window;
    Mark_Window();
    varsnd_alias_list_ptr = &varitemDef_t->focusSound;
    Mark_snd_alias_list_ptr();
    varitemDefData_t = &varitemDef_t->typeData;
    Mark_itemDefData_t();
}

void __cdecl Mark_itemDef_ptr()
{
    if (*varitemDef_ptr)
    {
        varitemDef_t = *varitemDef_ptr;
        Mark_itemDef_t();
    }
}

void __cdecl Mark_itemDef_ptrArray(int count)
{
    itemDef_s **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varitemDef_ptr;
    for (i = 0; i < count; ++i)
    {
        varitemDef_ptr = var;
        Mark_itemDef_ptr();
        ++var;
    }
}

void __cdecl Mark_menuDef_t()
{
    varWindow = &varmenuDef_t->window;
    Mark_Window();
    if (varmenuDef_t->items)
    {
        varitemDef_ptr = varmenuDef_t->items;
        Mark_itemDef_ptrArray(varmenuDef_t->itemCount);
    }
}

void __cdecl Mark_menuDef_ptr()
{
    if (*varmenuDef_ptr)
    {
        varmenuDef_t = *varmenuDef_ptr;
        Mark_MenuAsset(varmenuDef_t);
        Mark_menuDef_t();
    }
}

void __cdecl Mark_menuDef_ptrArray(int count)
{
    menuDef_t **var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varmenuDef_ptr;
    for (i = 0; i < count; ++i)
    {
        varmenuDef_ptr = var;
        Mark_menuDef_ptr();
        ++var;
    }
}

void __cdecl Mark_MenuList()
{
    if (varMenuList->menus)
    {
        varmenuDef_ptr = varMenuList->menus;
        Mark_menuDef_ptrArray(varMenuList->menuCount);
    }
}

void __cdecl Mark_MenuListPtr()
{
    if (*varMenuListPtr)
    {
        varMenuList = *varMenuListPtr;
        Mark_MenuListAsset(varMenuList);
        Mark_MenuList();
    }
}

void __cdecl Load_LocalizeEntry(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varLocalizeEntry, 8);
    DB_PushStreamPos(4u);
    varXString = &varLocalizeEntry->value;
    Load_XString(0);
    varXString = &varLocalizeEntry->name;
    Load_XString(0);
    DB_PopStreamPos();
}

void __cdecl Load_LocalizeEntryPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varLocalizeEntryPtr, 4);
    DB_PushStreamPos(0);
    if (*varLocalizeEntryPtr)
    {
        value = (unsigned int)*varLocalizeEntryPtr;
        if (*varLocalizeEntryPtr == (LocalizeEntry *)-1 || value == -2)
        {
            *varLocalizeEntryPtr = (LocalizeEntry *)AllocLoad_FxElemVisStateSample();
            varLocalizeEntry = *varLocalizeEntryPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_LocalizeEntry(1);
            Load_LocalizeEntryAsset((XAssetHeader *)varLocalizeEntryPtr);
            if (inserted)
                *inserted = *varLocalizeEntryPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varLocalizeEntryPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_LocalizeEntryPtr()
{
    if (*varLocalizeEntryPtr)
    {
        varLocalizeEntry = *varLocalizeEntryPtr;
        Mark_LocalizeEntryAsset(varLocalizeEntry);
    }
}

void __cdecl Load_FxImpactEntry(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxImpactEntry, 132);
    varFxEffectDefHandle = (const FxEffectDef **)varFxImpactEntry;
    Load_FxEffectDefHandleArray(0, 29);
    varFxEffectDefHandle = varFxImpactEntry->flesh;
    Load_FxEffectDefHandleArray(0, 4);
}

void __cdecl Load_FxImpactEntryArray(bool atStreamStart, int count)
{
    FxImpactEntry *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxImpactEntry, 132 * count);
    var = varFxImpactEntry;
    for (i = 0; i < count; ++i)
    {
        varFxImpactEntry = var;
        Load_FxImpactEntry(0);
        ++var;
    }
}

void __cdecl Load_FxImpactTable(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFxImpactTable, 8);
    DB_PushStreamPos(4u);
    varXString = &varFxImpactTable->name;
    Load_XString(0);
    if (varFxImpactTable->table)
    {
        varFxImpactTable->table = (FxImpactEntry *)AllocLoad_FxElemVisStateSample();
        varFxImpactEntry = varFxImpactTable->table;
        Load_FxImpactEntryArray(1, 12);
    }
    DB_PopStreamPos();
}

void __cdecl Load_FxImpactTablePtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFxImpactTablePtr, 4);
    DB_PushStreamPos(0);
    if (*varFxImpactTablePtr)
    {
        value = (unsigned int)*varFxImpactTablePtr;
        if (*varFxImpactTablePtr == (FxImpactTable *)-1 || value == -2)
        {
            *varFxImpactTablePtr = (FxImpactTable *)AllocLoad_FxElemVisStateSample();
            varFxImpactTable = *varFxImpactTablePtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_FxImpactTable(1);
            Load_FxImpactTableAsset((XAssetHeader *)varFxImpactTablePtr);
            if (inserted)
                *inserted = *varFxImpactTablePtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varFxImpactTablePtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_FxImpactEntry()
{
    varFxEffectDefHandle = (const FxEffectDef **)varFxImpactEntry;
    Mark_FxEffectDefHandleArray(29);
    varFxEffectDefHandle = varFxImpactEntry->flesh;
    Mark_FxEffectDefHandleArray(4);
}

void __cdecl Mark_FxImpactEntryArray(int count)
{
    FxImpactEntry *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varFxImpactEntry;
    for (i = 0; i < count; ++i)
    {
        varFxImpactEntry = var;
        Mark_FxImpactEntry();
        ++var;
    }
}

void __cdecl Mark_FxImpactTable()
{
    if (varFxImpactTable->table)
    {
        varFxImpactEntry = varFxImpactTable->table;
        Mark_FxImpactEntryArray(12);
    }
}

void __cdecl Mark_FxImpactTablePtr()
{
    if (*varFxImpactTablePtr)
    {
        varFxImpactTable = *varFxImpactTablePtr;
        Mark_FxImpactTableAsset(varFxImpactTable);
        Mark_FxImpactTable();
    }
}

void __cdecl Load_WeaponDef(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varWeaponDef, 2168);
    DB_PushStreamPos(4u);
    varXString = &varWeaponDef->szInternalName;
    Load_XString(0);
    varXString = &varWeaponDef->szDisplayName;
    Load_XString(0);
    varXString = &varWeaponDef->szOverlayName;
    Load_XString(0);
    varXModelPtr = varWeaponDef->gunXModel;
    Load_XModelPtrArray(0, 16);
    varXModelPtr = &varWeaponDef->handXModel;
    Load_XModelPtr(0);
    varXString = varWeaponDef->szXAnims;
    Load_XStringArray(0, 33);
    varXString = &varWeaponDef->szModeName;
    Load_XString(0);
    varScriptString = varWeaponDef->hideTags;
    Load_ScriptStringArray(0, 8);
    varScriptString = varWeaponDef->notetrackSoundMapKeys;
    Load_ScriptStringArray(0, 16);
    varScriptString = varWeaponDef->notetrackSoundMapValues;
    Load_ScriptStringArray(0, 16);
    varFxEffectDefHandle = &varWeaponDef->viewFlashEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->worldFlashEffect;
    Load_FxEffectDefHandle(0);
    varsnd_alias_list_name = &varWeaponDef->pickupSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->pickupSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->ammoPickupSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->ammoPickupSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->projectileSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->pullbackSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->pullbackSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->fireSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->fireSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->fireLoopSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->fireLoopSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->fireStopSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->fireStopSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->fireLastSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->fireLastSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->emptyFireSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->emptyFireSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->meleeSwipeSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->meleeSwipeSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->meleeHitSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->meleeMissSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->rechamberSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->rechamberSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->reloadSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->reloadSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->reloadEmptySound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->reloadEmptySoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->reloadStartSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->reloadStartSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->reloadEndSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->reloadEndSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->detonateSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->detonateSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->nightVisionWearSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->nightVisionWearSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->nightVisionRemoveSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->nightVisionRemoveSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->altSwitchSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->altSwitchSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->raiseSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->raiseSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->firstRaiseSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->firstRaiseSoundPlayer;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->putawaySound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->putawaySoundPlayer;
    Load_snd_alias_list_name(0);
    if (varWeaponDef->bounceSound)
    {
        if (varWeaponDef->bounceSound == (snd_alias_list_t **)-1)
        {
            varWeaponDef->bounceSound = (snd_alias_list_t **)AllocLoad_FxElemVisStateSample();
            varsnd_alias_list_name = varWeaponDef->bounceSound;
            Load_snd_alias_list_nameArray(1, 29);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varWeaponDef->bounceSound);
        }
    }
    varFxEffectDefHandle = &varWeaponDef->viewShellEjectEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->worldShellEjectEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->viewLastShotEjectEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->worldLastShotEjectEffect;
    Load_FxEffectDefHandle(0);
    varMaterialHandle = &varWeaponDef->reticleCenter;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponDef->reticleSide;
    Load_MaterialHandle(0);
    varXModelPtr = varWeaponDef->worldModel;
    Load_XModelPtrArray(0, 16);
    varXModelPtr = &varWeaponDef->worldClipModel;
    Load_XModelPtr(0);
    varXModelPtr = &varWeaponDef->rocketModel;
    Load_XModelPtr(0);
    varXModelPtr = &varWeaponDef->knifeModel;
    Load_XModelPtr(0);
    varXModelPtr = &varWeaponDef->worldKnifeModel;
    Load_XModelPtr(0);
    varMaterialHandle = &varWeaponDef->hudIcon;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponDef->ammoCounterIcon;
    Load_MaterialHandle(0);
    varXString = &varWeaponDef->szAmmoName;
    Load_XString(0);
    varXString = &varWeaponDef->szClipName;
    Load_XString(0);
    varXString = &varWeaponDef->szSharedAmmoCapName;
    Load_XString(0);
    varMaterialHandle = &varWeaponDef->overlayMaterial;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponDef->overlayMaterialLowRes;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponDef->killIcon;
    Load_MaterialHandle(0);
    varMaterialHandle = &varWeaponDef->dpadIcon;
    Load_MaterialHandle(0);
    varXString = &varWeaponDef->szAltWeaponName;
    Load_XString(0);
    varXModelPtr = &varWeaponDef->projectileModel;
    Load_XModelPtr(0);
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->projDudEffect;
    Load_FxEffectDefHandle(0);
    varsnd_alias_list_name = &varWeaponDef->projExplosionSound;
    Load_snd_alias_list_name(0);
    varsnd_alias_list_name = &varWeaponDef->projDudSound;
    Load_snd_alias_list_name(0);
    varFxEffectDefHandle = &varWeaponDef->projTrailEffect;
    Load_FxEffectDefHandle(0);
    varFxEffectDefHandle = &varWeaponDef->projIgnitionEffect;
    Load_FxEffectDefHandle(0);
    varsnd_alias_list_name = &varWeaponDef->projIgnitionSound;
    Load_snd_alias_list_name(0);
    varXString = varWeaponDef->accuracyGraphName;
    Load_XString(0);
    if (varWeaponDef->accuracyGraphKnots[0])
    {
        if (varWeaponDef->accuracyGraphKnots[0] == (float (*)[2]) - 1)
        {
            varWeaponDef->accuracyGraphKnots[0] = (float (*)[2])AllocLoad_FxElemVisStateSample();
            varvec2_t = varWeaponDef->accuracyGraphKnots[0];
            Load_vec2_tArray(1, varWeaponDef->accuracyGraphKnotCount[0]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varWeaponDef->accuracyGraphKnots);
        }
    }
    if (varWeaponDef->originalAccuracyGraphKnots[0])
    {
        if (varWeaponDef->originalAccuracyGraphKnots[0] == (float (*)[2]) - 1)
        {
            varWeaponDef->originalAccuracyGraphKnots[0] = (float (*)[2])AllocLoad_FxElemVisStateSample();
            varvec2_t = varWeaponDef->originalAccuracyGraphKnots[0];
            Load_vec2_tArray(1, varWeaponDef->accuracyGraphKnotCount[0]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varWeaponDef->originalAccuracyGraphKnots);
        }
    }
    varXString = &varWeaponDef->accuracyGraphName[1];
    Load_XString(0);
    if (varWeaponDef->accuracyGraphKnots[1])
    {
        if (varWeaponDef->accuracyGraphKnots[1] == (float (*)[2]) - 1)
        {
            varWeaponDef->accuracyGraphKnots[1] = (float (*)[2])AllocLoad_FxElemVisStateSample();
            varvec2_t = varWeaponDef->accuracyGraphKnots[1];
            Load_vec2_tArray(1, varWeaponDef->accuracyGraphKnotCount[1]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varWeaponDef->accuracyGraphKnots[1]);
        }
    }
    if (varWeaponDef->originalAccuracyGraphKnots[1])
    {
        if (varWeaponDef->originalAccuracyGraphKnots[1] == (float (*)[2]) - 1)
        {
            varWeaponDef->originalAccuracyGraphKnots[1] = (float (*)[2])AllocLoad_FxElemVisStateSample();
            varvec2_t = varWeaponDef->originalAccuracyGraphKnots[1];
            Load_vec2_tArray(1, varWeaponDef->accuracyGraphKnotCount[1]);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varWeaponDef->originalAccuracyGraphKnots[1]);
        }
    }
    varXString = &varWeaponDef->szUseHintString;
    Load_XString(0);
    varXString = &varWeaponDef->dropHintString;
    Load_XString(0);
    varXString = &varWeaponDef->szScript;
    Load_XString(0);
    varXString = &varWeaponDef->fireRumble;
    Load_XString(0);
    varXString = &varWeaponDef->meleeImpactRumble;
    Load_XString(0);
    DB_PopStreamPos();
}

void __cdecl Load_WeaponDefPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varWeaponDefPtr, 4);
    DB_PushStreamPos(0);
    if (*varWeaponDefPtr)
    {
        value = (unsigned int)*varWeaponDefPtr;
        if (*varWeaponDefPtr == (WeaponDef *)-1 || value == -2)
        {
            *varWeaponDefPtr = (WeaponDef *)AllocLoad_FxElemVisStateSample();
            varWeaponDef = *varWeaponDefPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_WeaponDef(1);
            Load_WeaponDefAsset((XAssetHeader *)varWeaponDefPtr);
            if (inserted)
                *inserted = *varWeaponDefPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varWeaponDefPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_WeaponDef()
{
    varXModelPtr = varWeaponDef->gunXModel;
    Mark_XModelPtrArray(16);
    varXModelPtr = &varWeaponDef->handXModel;
    Mark_XModelPtr();
    varScriptString = varWeaponDef->hideTags;
    Mark_ScriptStringArray(8);
    varScriptString = varWeaponDef->notetrackSoundMapKeys;
    Mark_ScriptStringArray(16);
    varScriptString = varWeaponDef->notetrackSoundMapValues;
    Mark_ScriptStringArray(16);
    varFxEffectDefHandle = &varWeaponDef->viewFlashEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->worldFlashEffect;
    Mark_FxEffectDefHandle();
    varsnd_alias_list_name = &varWeaponDef->pickupSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->pickupSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->ammoPickupSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->ammoPickupSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->projectileSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->pullbackSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->pullbackSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->fireSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->fireSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->fireLoopSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->fireLoopSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->fireStopSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->fireStopSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->fireLastSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->fireLastSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->emptyFireSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->emptyFireSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->meleeSwipeSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->meleeSwipeSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->meleeHitSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->meleeMissSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->rechamberSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->rechamberSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->reloadSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->reloadSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->reloadEmptySound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->reloadEmptySoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->reloadStartSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->reloadStartSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->reloadEndSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->reloadEndSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->detonateSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->detonateSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->nightVisionWearSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->nightVisionWearSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->nightVisionRemoveSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->nightVisionRemoveSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->altSwitchSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->altSwitchSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->raiseSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->raiseSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->firstRaiseSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->firstRaiseSoundPlayer;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->putawaySound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->putawaySoundPlayer;
    Mark_snd_alias_list_name();
    if (varWeaponDef->bounceSound)
    {
        varsnd_alias_list_name = varWeaponDef->bounceSound;
        Mark_snd_alias_list_nameArray(29);
    }
    varFxEffectDefHandle = &varWeaponDef->viewShellEjectEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->worldShellEjectEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->viewLastShotEjectEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->worldLastShotEjectEffect;
    Mark_FxEffectDefHandle();
    varMaterialHandle = &varWeaponDef->reticleCenter;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->reticleSide;
    Mark_MaterialHandle();
    varXModelPtr = varWeaponDef->worldModel;
    Mark_XModelPtrArray(16);
    varXModelPtr = &varWeaponDef->worldClipModel;
    Mark_XModelPtr();
    varXModelPtr = &varWeaponDef->rocketModel;
    Mark_XModelPtr();
    varXModelPtr = &varWeaponDef->knifeModel;
    Mark_XModelPtr();
    varXModelPtr = &varWeaponDef->worldKnifeModel;
    Mark_XModelPtr();
    varMaterialHandle = &varWeaponDef->hudIcon;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->ammoCounterIcon;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->overlayMaterial;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->overlayMaterialLowRes;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->killIcon;
    Mark_MaterialHandle();
    varMaterialHandle = &varWeaponDef->dpadIcon;
    Mark_MaterialHandle();
    varXModelPtr = &varWeaponDef->projectileModel;
    Mark_XModelPtr();
    varFxEffectDefHandle = &varWeaponDef->projExplosionEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projDudEffect;
    Mark_FxEffectDefHandle();
    varsnd_alias_list_name = &varWeaponDef->projExplosionSound;
    Mark_snd_alias_list_name();
    varsnd_alias_list_name = &varWeaponDef->projDudSound;
    Mark_snd_alias_list_name();
    varFxEffectDefHandle = &varWeaponDef->projTrailEffect;
    Mark_FxEffectDefHandle();
    varFxEffectDefHandle = &varWeaponDef->projIgnitionEffect;
    Mark_FxEffectDefHandle();
    varsnd_alias_list_name = &varWeaponDef->projIgnitionSound;
    Mark_snd_alias_list_name();
}

void __cdecl Mark_WeaponDefPtr()
{
    if (*varWeaponDefPtr)
    {
        varWeaponDef = *varWeaponDefPtr;
        Mark_WeaponDefAsset(varWeaponDef);
        Mark_WeaponDef();
    }
}

void __cdecl Load_RawFile(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varRawFile, 12);
    DB_PushStreamPos(4u);
    varXString = &varRawFile->name;
    Load_XString(0);
    if (varRawFile->buffer)
    {
        varRawFile->buffer = (const char *)AllocLoad_raw_byte();
        varConstChar = (const char*)varRawFile->buffer;
        Load_ConstCharArray(1, varRawFile->len + 1);
    }
    DB_PopStreamPos();
}

void __cdecl Load_RawFilePtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varRawFilePtr, 4);
    DB_PushStreamPos(0);
    if (*varRawFilePtr)
    {
        value = (unsigned int)*varRawFilePtr;
        if (*varRawFilePtr == (RawFile *)-1 || value == -2)
        {
            *varRawFilePtr = (RawFile *)AllocLoad_FxElemVisStateSample();
            varRawFile = *varRawFilePtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_RawFile(1);
            Load_RawFileAsset((XAssetHeader *)varRawFilePtr);
            if (inserted)
                *inserted = *varRawFilePtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varRawFilePtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_RawFilePtr()
{
    if (*varRawFilePtr)
    {
        varRawFile = *varRawFilePtr;
        Mark_RawFileAsset(varRawFile);
    }
}

void __cdecl Load_StringTable(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStringTable, 16);
    varXString = &varStringTable->name;
    Load_XString(0);
    if (varStringTable->values)
    {
        varStringTable->values = (const char **)AllocLoad_FxElemVisStateSample();
        varXString = varStringTable->values;
        Load_XStringArray(1, varStringTable->rowCount * varStringTable->columnCount);
    }
}

void __cdecl Load_StringTablePtr(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStringTablePtr, 4);
    if (*varStringTablePtr)
    {
        if (*varStringTablePtr == (StringTable *)-1)
        {
            *varStringTablePtr = (StringTable *)AllocLoad_FxElemVisStateSample();
            varStringTable = *varStringTablePtr;
            Load_StringTable(1);
            Load_StringTableAsset((XAssetHeader *)varStringTablePtr);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)varStringTablePtr);
        }
    }
}

void __cdecl Mark_StringTablePtr()
{
    if (*varStringTablePtr)
    {
        varStringTable = *varStringTablePtr;
        Mark_StringTableAsset(varStringTable);
    }
}

void __cdecl Load_GfxStaticModelDrawInst(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStaticModelDrawInst, 76);
    varXModelPtr = &varGfxStaticModelDrawInst->model;
    Load_XModelPtr(0);
}

void __cdecl Load_GfxStaticModelDrawInstArray(bool atStreamStart, int count)
{
    GfxStaticModelDrawInst *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStaticModelDrawInst, 76 * count);
    var = varGfxStaticModelDrawInst;
    for (i = 0; i < count; ++i)
    {
        varGfxStaticModelDrawInst = var;
        Load_GfxStaticModelDrawInst(0);
        ++var;
    }
}

void __cdecl Load_GfxStaticModelInstArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxStaticModelInst, 28 * count);
}

void __cdecl Mark_GfxStaticModelDrawInst()
{
    varXModelPtr = &varGfxStaticModelDrawInst->model;
    Mark_XModelPtr();
}

void __cdecl Mark_GfxStaticModelDrawInstArray(int count)
{
    GfxStaticModelDrawInst *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxStaticModelDrawInst;
    for (i = 0; i < count; ++i)
    {
        varGfxStaticModelDrawInst = var;
        Mark_GfxStaticModelDrawInst();
        ++var;
    }
}

void __cdecl Load_sunflare_t(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varsunflare_t, 96);
    varMaterialHandle = &varsunflare_t->spriteMaterial;
    Load_MaterialHandle(0);
    varMaterialHandle = &varsunflare_t->flareMaterial;
    Load_MaterialHandle(0);
}

void __cdecl Mark_sunflare_t()
{
    varMaterialHandle = &varsunflare_t->spriteMaterial;
    Mark_MaterialHandle();
    varMaterialHandle = &varsunflare_t->flareMaterial;
    Mark_MaterialHandle();
}

void __cdecl Load_GfxReflectionProbe(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxReflectionProbe, 16);
    varGfxImagePtr = &varGfxReflectionProbe->reflectionImage;
    Load_GfxImagePtr(0);
}

void __cdecl Load_GfxReflectionProbeArray(bool atStreamStart, int count)
{
    GfxReflectionProbe *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxReflectionProbe, 16 * count);
    var = varGfxReflectionProbe;
    for (i = 0; i < count; ++i)
    {
        varGfxReflectionProbe = var;
        Load_GfxReflectionProbe(0);
        ++var;
    }
}

void __cdecl Mark_GfxReflectionProbe()
{
    varGfxImagePtr = &varGfxReflectionProbe->reflectionImage;
    Mark_GfxImagePtr();
}

void __cdecl Mark_GfxReflectionProbeArray(int count)
{
    GfxReflectionProbe *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varGfxReflectionProbe;
    for (i = 0; i < count; ++i)
    {
        varGfxReflectionProbe = var;
        Mark_GfxReflectionProbe();
        ++var;
    }
}

void __cdecl Load_StaticModelIndexArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varStaticModelIndex, 2 * count);
}

void __cdecl Load_GfxAabbTree(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxAabbTree, 44);
    if (varGfxAabbTree->smodelIndexes)
    {
        if (varGfxAabbTree->smodelIndexes == (unsigned __int16 *)-1)
        {
            varGfxAabbTree->smodelIndexes = (unsigned __int16 *)AllocLoad_XBlendInfo();
            varStaticModelIndex = varGfxAabbTree->smodelIndexes;
            Load_StaticModelIndexArray(1, varGfxAabbTree->smodelIndexCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varGfxAabbTree->smodelIndexes);
        }
    }
}

void __cdecl Load_GfxAabbTreeArray(bool atStreamStart, int count)
{
    GfxAabbTree *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxAabbTree, 44 * count);
    var = varGfxAabbTree;
    for (i = 0; i < count; ++i)
    {
        varGfxAabbTree = var;
        Load_GfxAabbTree(0);
        ++var;
    }
}

void __cdecl Load_GfxCell(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxCell, 56);
    if (varGfxCell->aabbTree)
    {
        varGfxCell->aabbTree = (GfxAabbTree *)AllocLoad_FxElemVisStateSample();
        varGfxAabbTree = varGfxCell->aabbTree;
        Load_GfxAabbTreeArray(1, varGfxCell->aabbTreeCount);
    }
    if (varGfxCell->portals)
    {
        varGfxCell->portals = (GfxPortal *)AllocLoad_FxElemVisStateSample();
        varGfxPortal = varGfxCell->portals;
        Load_GfxPortalArray(1, varGfxCell->portalCount);
    }
    if (varGfxCell->cullGroups)
    {
        varGfxCell->cullGroups = (int *)AllocLoad_FxElemVisStateSample();
        varint = varGfxCell->cullGroups;
        Load_intArray(1, varGfxCell->cullGroupCount);
    }
    if (varGfxCell->reflectionProbes)
    {
        varGfxCell->reflectionProbes = AllocLoad_raw_byte();
        varbyte = varGfxCell->reflectionProbes;
        Load_byteArray(1, varGfxCell->reflectionProbeCount);
    }
}

void __cdecl Load_GfxCellArray(bool atStreamStart, int count)
{
    GfxCell *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxCell, 56 * count);
    var = varGfxCell;
    for (i = 0; i < count; ++i)
    {
        varGfxCell = var;
        Load_GfxCell(0);
        ++var;
    }
}

void __cdecl Load_GfxPortal(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxPortal, 68);
    if (varGfxPortal->cell)
    {
        if (varGfxPortal->cell == (GfxCell *)-1)
        {
            varGfxPortal->cell = (GfxCell *)AllocLoad_FxElemVisStateSample();
            varGfxCell = varGfxPortal->cell;
            Load_GfxCell(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varGfxPortal->cell);
        }
    }
    if (varGfxPortal->vertices)
    {
        varGfxPortal->vertices = (float (*)[3])AllocLoad_FxElemVisStateSample();
        varvec3_t = varGfxPortal->vertices;
        Load_vec3_tArray(1, varGfxPortal->vertexCount);
    }
}

void __cdecl Load_GfxPortalArray(bool atStreamStart, int count)
{
    GfxPortal *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxPortal, 68 * count);
    var = varGfxPortal;
    for (i = 0; i < count; ++i)
    {
        varGfxPortal = var;
        Load_GfxPortal(0);
        ++var;
    }
}

void __cdecl Load_GfxCullGroupArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxCullGroup, 32 * count);
}

void __cdecl Load_GfxLightGridEntryArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightGridEntry, 4 * count);
}

void __cdecl Load_GfxLightGridColorsArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightGridColors, 168 * count);
}

void __cdecl Load_MaterialMemory(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialMemory, 8);
    varMaterialHandle = &varMaterialMemory->material;
    Load_MaterialHandle(0);
}

void __cdecl Load_MaterialMemoryArray(bool atStreamStart, int count)
{
    MaterialMemory *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varMaterialMemory, 8 * count);
    var = varMaterialMemory;
    for (i = 0; i < count; ++i)
    {
        varMaterialMemory = var;
        Load_MaterialMemory(0);
        ++var;
    }
}

void __cdecl Load_GfxWorldVertexData(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldVertexData, 8);
    if (varGfxWorldVertexData->vertices)
    {
        varGfxWorldVertexData->vertices = (GfxWorldVertex *)AllocLoad_FxElemVisStateSample();
        varGfxWorldVertex0 = varGfxWorldVertexData->vertices;
        Load_GfxWorldVertex0Array(1, varGfxWorld->vertexCount);
    }
    varGfxVertexBuffer = &varGfxWorldVertexData->worldVb;
    Load_GfxVertexBuffer(0);
    Load_VertexBuffer(
        &varGfxWorldVertexData->worldVb,
        (unsigned __int8 *)varGfxWorld->vd.vertices,
        44 * varGfxWorld->vertexCount);
}

void __cdecl Load_GfxWorldVertexLayerData(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldVertexLayerData, 8);
    if (varGfxWorldVertexLayerData->data)
    {
        varGfxWorldVertexLayerData->data = AllocLoad_raw_byte();
        varbyte = varGfxWorldVertexLayerData->data;
        Load_byteArray(1, varGfxWorld->vertexLayerDataSize);
    }
    varGfxVertexBuffer = &varGfxWorldVertexLayerData->layerVb;
    Load_GfxVertexBuffer(0);
    Load_VertexBuffer(&varGfxWorldVertexLayerData->layerVb, varGfxWorld->vld.data, varGfxWorld->vertexLayerDataSize);
}

void __cdecl Load_GfxLightGrid(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightGrid, 56);
    if (varGfxLightGrid->rowDataStart)
    {
        varGfxLightGrid->rowDataStart = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxLightGrid->rowDataStart;
        Load_ushortArray(
            1,
            varGfxLightGrid->maxs[varGfxLightGrid->rowAxis] - varGfxLightGrid->mins[varGfxLightGrid->rowAxis] + 1);
    }
    if (varGfxLightGrid->rawRowData)
    {
        varGfxLightGrid->rawRowData = AllocLoad_raw_byte();
        varbyte = varGfxLightGrid->rawRowData;
        Load_byteArray(1, varGfxLightGrid->rawRowDataSize);
    }
    if (varGfxLightGrid->entries)
    {
        varGfxLightGrid->entries = (GfxLightGridEntry *)AllocLoad_FxElemVisStateSample();
        varGfxLightGridEntry = varGfxLightGrid->entries;
        Load_GfxLightGridEntryArray(1, varGfxLightGrid->entryCount);
    }
    if (varGfxLightGrid->colors)
    {
        varGfxLightGrid->colors = (GfxLightGridColors *)AllocLoad_FxElemVisStateSample();
        varGfxLightGridColors = varGfxLightGrid->colors;
        Load_GfxLightGridColorsArray(1, varGfxLightGrid->colorCount);
    }
}

void __cdecl Load_GfxSceneDynModelArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxSceneDynModel, 6 * count);
}

void __cdecl Load_GfxSceneDynBrushArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxSceneDynBrush, 4 * count);
}

void __cdecl Load_GfxDrawSurfArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxDrawSurf, 8 * count);
}

void __cdecl Load_GfxShadowGeometry(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxShadowGeometry, 12);
    if (varGfxShadowGeometry->sortedSurfIndex)
    {
        varGfxShadowGeometry->sortedSurfIndex = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxShadowGeometry->sortedSurfIndex;
        Load_ushortArray(1, varGfxShadowGeometry->surfaceCount);
    }
    if (varGfxShadowGeometry->smodelIndex)
    {
        varGfxShadowGeometry->smodelIndex = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxShadowGeometry->smodelIndex;
        Load_ushortArray(1, varGfxShadowGeometry->smodelCount);
    }
}

void __cdecl Load_GfxShadowGeometryArray(bool atStreamStart, int count)
{
    GfxShadowGeometry *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxShadowGeometry, 12 * count);
    var = varGfxShadowGeometry;
    for (i = 0; i < count; ++i)
    {
        varGfxShadowGeometry = var;
        Load_GfxShadowGeometry(0);
        ++var;
    }
}

void __cdecl Load_GfxLightRegionAxisArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegionAxis, 20 * count);
}

void __cdecl Load_GfxLightRegionHull(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegionHull, 80);
    if (varGfxLightRegionHull->axis)
    {
        varGfxLightRegionHull->axis = (GfxLightRegionAxis *)AllocLoad_FxElemVisStateSample();
        varGfxLightRegionAxis = varGfxLightRegionHull->axis;
        Load_GfxLightRegionAxisArray(1, varGfxLightRegionHull->axisCount);
    }
}

void __cdecl Load_GfxLightRegionHullArray(bool atStreamStart, int count)
{
    GfxLightRegionHull *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegionHull, 80 * count);
    var = varGfxLightRegionHull;
    for (i = 0; i < count; ++i)
    {
        varGfxLightRegionHull = var;
        Load_GfxLightRegionHull(0);
        ++var;
    }
}

void __cdecl Load_GfxLightRegion(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegion, 8);
    if (varGfxLightRegion->hulls)
    {
        varGfxLightRegion->hulls = (GfxLightRegionHull *)AllocLoad_FxElemVisStateSample();
        varGfxLightRegionHull = varGfxLightRegion->hulls;
        Load_GfxLightRegionHullArray(1, varGfxLightRegion->hullCount);
    }
}

void __cdecl Load_GfxLightRegionArray(bool atStreamStart, int count)
{
    GfxLightRegion *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxLightRegion, 8 * count);
    var = varGfxLightRegion;
    for (i = 0; i < count; ++i)
    {
        varGfxLightRegion = var;
        Load_GfxLightRegion(0);
        ++var;
    }
}

void __cdecl Load_GfxWorldDpvsDynamic(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldDpvsDynamic, 48);
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsDynamic->dynEntCellBits[0])
    {
        varGfxWorldDpvsDynamic->dynEntCellBits[0] = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorldDpvsDynamic->dynEntCellBits[0];
        Load_raw_uintArray(1, varGfxWorld->dpvsPlanes.cellCount * varGfxWorldDpvsDynamic->dynEntClientWordCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsDynamic->dynEntCellBits[1])
    {
        varGfxWorldDpvsDynamic->dynEntCellBits[1] = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorldDpvsDynamic->dynEntCellBits[1];
        Load_raw_uintArray(1, varGfxWorld->dpvsPlanes.cellCount * varGfxWorldDpvsDynamic->dynEntClientWordCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsDynamic->dynEntVisData[0][0])
    {
        varGfxWorldDpvsDynamic->dynEntVisData[0][0] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[0][0];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsDynamic->dynEntVisData[1][0])
    {
        varGfxWorldDpvsDynamic->dynEntVisData[1][0] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[1][0];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsDynamic->dynEntVisData[0][1])
    {
        varGfxWorldDpvsDynamic->dynEntVisData[0][1] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[0][1];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsDynamic->dynEntVisData[1][1])
    {
        varGfxWorldDpvsDynamic->dynEntVisData[1][1] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[1][1];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsDynamic->dynEntVisData[0][2])
    {
        varGfxWorldDpvsDynamic->dynEntVisData[0][2] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[0][2];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsDynamic->dynEntVisData[1][2])
    {
        varGfxWorldDpvsDynamic->dynEntVisData[1][2] = (unsigned __int8 *)AllocLoad_GfxPackedVertex0();
        varraw_byte16 = varGfxWorldDpvsDynamic->dynEntVisData[1][2];
        Load_raw_byte16Array(1, 32 * varGfxWorldDpvsDynamic->dynEntClientWordCount[1]);
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxWorldDpvsStatic(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldDpvsStatic, 104);
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->smodelVisData[0])
    {
        varGfxWorldDpvsStatic->smodelVisData[0] = AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->smodelVisData[0];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->smodelVisData[1])
    {
        varGfxWorldDpvsStatic->smodelVisData[1] = AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->smodelVisData[1];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->smodelVisData[2])
    {
        varGfxWorldDpvsStatic->smodelVisData[2] = AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->smodelVisData[2];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->surfaceVisData[0])
    {
        varGfxWorldDpvsStatic->surfaceVisData[0] = AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->surfaceVisData[0];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->surfaceVisData[1])
    {
        varGfxWorldDpvsStatic->surfaceVisData[1] = AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->surfaceVisData[1];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->surfaceVisData[2])
    {
        varGfxWorldDpvsStatic->surfaceVisData[2] = AllocLoad_raw_byte();
        varraw_byte = varGfxWorldDpvsStatic->surfaceVisData[2];
        Load_raw_byteArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->lodData)
    {
        varGfxWorldDpvsStatic->lodData = (unsigned int *)AllocLoad_raw_uint128();
        varraw_uint128 = varGfxWorldDpvsStatic->lodData;
        Load_raw_uint128Array(1, 2 * varGfxWorldDpvsStatic->smodelVisDataCount);
    }
    DB_PopStreamPos();
    if (varGfxWorldDpvsStatic->sortedSurfIndex)
    {
        varGfxWorldDpvsStatic->sortedSurfIndex = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxWorldDpvsStatic->sortedSurfIndex;
        Load_ushortArray(1, varGfxWorldDpvsStatic->staticSurfaceCountNoDecal + varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    if (varGfxWorldDpvsStatic->smodelInsts)
    {
        varGfxWorldDpvsStatic->smodelInsts = (GfxStaticModelInst *)AllocLoad_FxElemVisStateSample();
        varGfxStaticModelInst = varGfxWorldDpvsStatic->smodelInsts;
        Load_GfxStaticModelInstArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    if (varGfxWorldDpvsStatic->surfaces)
    {
        varGfxWorldDpvsStatic->surfaces = (GfxSurface *)AllocLoad_FxElemVisStateSample();
        varGfxSurface = varGfxWorldDpvsStatic->surfaces;
        Load_GfxSurfaceArray(1, varGfxWorld->surfaceCount);
    }
    if (varGfxWorldDpvsStatic->cullGroups)
    {
        varGfxWorldDpvsStatic->cullGroups = (GfxCullGroup *)AllocLoad_FxElemVisStateSample();
        varGfxCullGroup = varGfxWorldDpvsStatic->cullGroups;
        Load_GfxCullGroupArray(1, varGfxWorld->cullGroupCount);
    }
    if (varGfxWorldDpvsStatic->smodelDrawInsts)
    {
        varGfxWorldDpvsStatic->smodelDrawInsts = (GfxStaticModelDrawInst *)AllocLoad_FxElemVisStateSample();
        varGfxStaticModelDrawInst = varGfxWorldDpvsStatic->smodelDrawInsts;
        Load_GfxStaticModelDrawInstArray(1, varGfxWorldDpvsStatic->smodelCount);
    }
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->surfaceMaterials)
    {
        varGfxWorldDpvsStatic->surfaceMaterials = (GfxDrawSurf *)AllocLoad_FxElemVisStateSample();
        varGfxDrawSurf = varGfxWorldDpvsStatic->surfaceMaterials;
        Load_GfxDrawSurfArray(1, varGfxWorldDpvsStatic->staticSurfaceCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsStatic->surfaceCastsSunShadow)
    {
        varGfxWorldDpvsStatic->surfaceCastsSunShadow = (unsigned int *)AllocLoad_raw_uint128();
        varraw_uint128 = varGfxWorldDpvsStatic->surfaceCastsSunShadow;
        Load_raw_uint128Array(1, varGfxWorldDpvsStatic->surfaceVisDataCount);
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxWorldDpvsPlanes(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldDpvsPlanes, 16);
    if (varGfxWorldDpvsPlanes->planes)
    {
        if (varGfxWorldDpvsPlanes->planes == (cplane_s *)-1)
        {
            varGfxWorldDpvsPlanes->planes = (cplane_s *)AllocLoad_FxElemVisStateSample();
            varcplane_t = varGfxWorldDpvsPlanes->planes;
            Load_cplane_tArray(1, varGfxWorld->planeCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varGfxWorldDpvsPlanes->planes);
        }
    }
    if (varGfxWorldDpvsPlanes->nodes)
    {
        varGfxWorldDpvsPlanes->nodes = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varushort = varGfxWorldDpvsPlanes->nodes;
        Load_ushortArray(1, varGfxWorld->nodeCount);
    }
    DB_PushStreamPos(1u);
    if (varGfxWorldDpvsPlanes->sceneEntCellBits)
    {
        varGfxWorldDpvsPlanes->sceneEntCellBits = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorldDpvsPlanes->sceneEntCellBits;
        Load_raw_uintArray(1, varGfxWorldDpvsPlanes->cellCount << 8);
    }
    DB_PopStreamPos();
}

void __cdecl Load_GfxWorld(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorld, 732);
    DB_PushStreamPos(4u);
    varXString = &varGfxWorld->name;
    Load_XString(0);
    varXString = &varGfxWorld->baseName;
    Load_XString(0);
    if (varGfxWorld->indices)
    {
        varGfxWorld->indices = (unsigned __int16 *)AllocLoad_XBlendInfo();
        varr_index_t = varGfxWorld->indices;
        Load_r_index_tArray(1, varGfxWorld->indexCount);
    }
    if (varGfxWorld->skyStartSurfs)
    {
        varGfxWorld->skyStartSurfs = (int *)AllocLoad_FxElemVisStateSample();
        varint = varGfxWorld->skyStartSurfs;
        Load_intArray(1, varGfxWorld->skySurfCount);
    }
    varGfxImagePtr = &varGfxWorld->skyImage;
    Load_GfxImagePtr(0);
    if (varGfxWorld->sunLight)
    {
        if (varGfxWorld->sunLight == (GfxLight *)-1)
        {
            varGfxWorld->sunLight = (GfxLight *)AllocLoad_FxElemVisStateSample();
            varGfxLight = varGfxWorld->sunLight;
            Load_GfxLight(1);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varGfxWorld->sunLight);
        }
    }
    if (varGfxWorld->reflectionProbes)
    {
        varGfxWorld->reflectionProbes = (GfxReflectionProbe *)AllocLoad_FxElemVisStateSample();
        varGfxReflectionProbe = varGfxWorld->reflectionProbes;
        Load_GfxReflectionProbeArray(1, varGfxWorld->reflectionProbeCount);
    }
    DB_PushStreamPos(1u);
    if (varGfxWorld->reflectionProbeTextures)
    {
        varGfxWorld->reflectionProbeTextures = (GfxTexture *)AllocLoad_FxElemVisStateSample();
        varGfxRawTexture = varGfxWorld->reflectionProbeTextures;
        Load_GfxRawTextureArray(1, varGfxWorld->reflectionProbeCount);
    }
    DB_PopStreamPos();
    varGfxWorldDpvsPlanes = &varGfxWorld->dpvsPlanes;
    Load_GfxWorldDpvsPlanes(0);
    if (varGfxWorld->cells)
    {
        varGfxWorld->cells = (GfxCell *)AllocLoad_FxElemVisStateSample();
        varGfxCell = varGfxWorld->cells;
        Load_GfxCellArray(1, varGfxWorld->dpvsPlanes.cellCount);
    }
    if (varGfxWorld->lightmaps)
    {
        varGfxWorld->lightmaps = (GfxLightmapArray *)AllocLoad_FxElemVisStateSample();
        varGfxLightmapArray = varGfxWorld->lightmaps;
        Load_GfxLightmapArrayArray(1, varGfxWorld->lightmapCount);
    }
    varGfxLightGrid = &varGfxWorld->lightGrid;
    Load_GfxLightGrid(0);
    DB_PushStreamPos(1u);
    if (varGfxWorld->lightmapPrimaryTextures)
    {
        varGfxWorld->lightmapPrimaryTextures = (GfxTexture *)AllocLoad_FxElemVisStateSample();
        varGfxRawTexture = varGfxWorld->lightmapPrimaryTextures;
        Load_GfxRawTextureArray(1, varGfxWorld->lightmapCount);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorld->lightmapSecondaryTextures)
    {
        varGfxWorld->lightmapSecondaryTextures = (GfxTexture *)AllocLoad_FxElemVisStateSample();
        varGfxRawTexture = varGfxWorld->lightmapSecondaryTextures;
        Load_GfxRawTextureArray(1, varGfxWorld->lightmapCount);
    }
    DB_PopStreamPos();
    if (varGfxWorld->models)
    {
        varGfxWorld->models = (GfxBrushModel *)AllocLoad_FxElemVisStateSample();
        varGfxBrushModel = varGfxWorld->models;
        Load_GfxBrushModelArray(1, varGfxWorld->modelCount);
    }
    if (varGfxWorld->materialMemory)
    {
        varGfxWorld->materialMemory = (MaterialMemory *)AllocLoad_FxElemVisStateSample();
        varMaterialMemory = varGfxWorld->materialMemory;
        Load_MaterialMemoryArray(1, varGfxWorld->materialMemoryCount);
    }
    varGfxWorldVertexData = &varGfxWorld->vd;
    Load_GfxWorldVertexData(0);
    varGfxWorldVertexLayerData = &varGfxWorld->vld;
    Load_GfxWorldVertexLayerData(0);
    varsunflare_t = &varGfxWorld->sun;
    Load_sunflare_t(0);
    varGfxImagePtr = &varGfxWorld->outdoorImage;
    Load_GfxImagePtr(0);
    DB_PushStreamPos(1u);
    if (varGfxWorld->cellCasterBits)
    {
        varGfxWorld->cellCasterBits = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorld->cellCasterBits;
        Load_raw_uintArray(1, varGfxWorld->dpvsPlanes.cellCount * ((varGfxWorld->dpvsPlanes.cellCount + 31) >> 5));
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorld->sceneDynModel)
    {
        varGfxWorld->sceneDynModel = (GfxSceneDynModel *)AllocLoad_FxElemVisStateSample();
        varGfxSceneDynModel = varGfxWorld->sceneDynModel;
        Load_GfxSceneDynModelArray(1, varGfxWorld->dpvsDyn.dynEntClientCount[0]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorld->sceneDynBrush)
    {
        varGfxWorld->sceneDynBrush = (GfxSceneDynBrush *)AllocLoad_FxElemVisStateSample();
        varGfxSceneDynBrush = varGfxWorld->sceneDynBrush;
        Load_GfxSceneDynBrushArray(1, varGfxWorld->dpvsDyn.dynEntClientCount[1]);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorld->primaryLightEntityShadowVis)
    {
        varGfxWorld->primaryLightEntityShadowVis = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorld->primaryLightEntityShadowVis;
        Load_raw_uintArray(1, (varGfxWorld->primaryLightCount - (varGfxWorld->sunPrimaryLightIndex + 1)) << 12);
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorld->primaryLightDynEntShadowVis[0])
    {
        varGfxWorld->primaryLightDynEntShadowVis[0] = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorld->primaryLightDynEntShadowVis[0];
        Load_raw_uintArray(
            1,
            varGfxWorld->dpvsDyn.dynEntClientCount[0]
            * (varGfxWorld->primaryLightCount - (varGfxWorld->sunPrimaryLightIndex + 1)));
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorld->primaryLightDynEntShadowVis[1])
    {
        varGfxWorld->primaryLightDynEntShadowVis[1] = (unsigned int *)AllocLoad_FxElemVisStateSample();
        varraw_uint = varGfxWorld->primaryLightDynEntShadowVis[1];
        Load_raw_uintArray(
            1,
            varGfxWorld->dpvsDyn.dynEntClientCount[1]
            * (varGfxWorld->primaryLightCount - (varGfxWorld->sunPrimaryLightIndex + 1)));
    }
    DB_PopStreamPos();
    DB_PushStreamPos(1u);
    if (varGfxWorld->nonSunPrimaryLightForModelDynEnt)
    {
        varGfxWorld->nonSunPrimaryLightForModelDynEnt = AllocLoad_raw_byte();
        varraw_byte = varGfxWorld->nonSunPrimaryLightForModelDynEnt;
        Load_raw_byteArray(1, varGfxWorld->dpvsDyn.dynEntClientCount[0]);
    }
    DB_PopStreamPos();
    if (varGfxWorld->shadowGeom)
    {
        varGfxWorld->shadowGeom = (GfxShadowGeometry *)AllocLoad_FxElemVisStateSample();
        varGfxShadowGeometry = varGfxWorld->shadowGeom;
        Load_GfxShadowGeometryArray(1, varGfxWorld->primaryLightCount);
    }
    if (varGfxWorld->lightRegion)
    {
        varGfxWorld->lightRegion = (GfxLightRegion *)AllocLoad_FxElemVisStateSample();
        varGfxLightRegion = varGfxWorld->lightRegion;
        Load_GfxLightRegionArray(1, varGfxWorld->primaryLightCount);
    }
    varGfxWorldDpvsStatic = &varGfxWorld->dpvs;
    Load_GfxWorldDpvsStatic(0);
    varGfxWorldDpvsDynamic = &varGfxWorld->dpvsDyn;
    Load_GfxWorldDpvsDynamic(0);
    DB_PopStreamPos();
}

void __cdecl Load_GfxWorldPtr(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varGfxWorldPtr, 4);
    DB_PushStreamPos(0);
    if (*varGfxWorldPtr)
    {
        value = (unsigned int)*varGfxWorldPtr;
        if (*varGfxWorldPtr == (GfxWorld *)-1 || value == -2)
        {
            *varGfxWorldPtr = (GfxWorld *)AllocLoad_FxElemVisStateSample();
            varGfxWorld = *varGfxWorldPtr;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_GfxWorld(1);
            Load_GfxWorldAsset((XAssetHeader *)varGfxWorldPtr);
            if (inserted)
                *inserted = *varGfxWorldPtr;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varGfxWorldPtr);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_MaterialMemory()
{
    varMaterialHandle = &varMaterialMemory->material;
    Mark_MaterialHandle();
}

void __cdecl Mark_MaterialMemoryArray(int count)
{
    MaterialMemory *var; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    var = varMaterialMemory;
    for (i = 0; i < count; ++i)
    {
        varMaterialMemory = var;
        Mark_MaterialMemory();
        ++var;
    }
}

void __cdecl Mark_GfxWorldDpvsStatic()
{
    if (varGfxWorldDpvsStatic->surfaces)
    {
        varGfxSurface = varGfxWorldDpvsStatic->surfaces;
        Mark_GfxSurfaceArray(varGfxWorld->surfaceCount);
    }
    if (varGfxWorldDpvsStatic->smodelDrawInsts)
    {
        varGfxStaticModelDrawInst = varGfxWorldDpvsStatic->smodelDrawInsts;
        Mark_GfxStaticModelDrawInstArray(varGfxWorldDpvsStatic->smodelCount);
    }
}

void __cdecl Mark_GfxWorld()
{
    varGfxImagePtr = &varGfxWorld->skyImage;
    Mark_GfxImagePtr();
    if (varGfxWorld->sunLight)
    {
        varGfxLight = varGfxWorld->sunLight;
        Mark_GfxLight();
    }
    if (varGfxWorld->reflectionProbes)
    {
        varGfxReflectionProbe = varGfxWorld->reflectionProbes;
        Mark_GfxReflectionProbeArray(varGfxWorld->reflectionProbeCount);
    }
    if (varGfxWorld->lightmaps)
    {
        varGfxLightmapArray = varGfxWorld->lightmaps;
        Mark_GfxLightmapArrayArray(varGfxWorld->lightmapCount);
    }
    if (varGfxWorld->materialMemory)
    {
        varMaterialMemory = varGfxWorld->materialMemory;
        Mark_MaterialMemoryArray(varGfxWorld->materialMemoryCount);
    }
    varsunflare_t = &varGfxWorld->sun;
    Mark_sunflare_t();
    varGfxImagePtr = &varGfxWorld->outdoorImage;
    Mark_GfxImagePtr();
    varGfxWorldDpvsStatic = &varGfxWorld->dpvs;
    Mark_GfxWorldDpvsStatic();
}

void __cdecl Mark_GfxWorldPtr()
{
    if (*varGfxWorldPtr)
    {
        varGfxWorld = *varGfxWorldPtr;
        Mark_GfxWorldAsset(varGfxWorld);
        Mark_GfxWorld();
    }
}

void __cdecl Load_GlyphArray(bool atStreamStart, int count)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varGlyph, 24 * count);
}

void __cdecl Load_Font(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varFont, 24);
    DB_PushStreamPos(4u);
    varXString = &varFont->fontName;
    Load_XString(0);
    varMaterialHandle = &varFont->material;
    Load_MaterialHandle(0);
    varMaterialHandle = &varFont->glowMaterial;
    Load_MaterialHandle(0);
    if (varFont->glyphs)
    {
        if (varFont->glyphs == (Glyph *)-1)
        {
            varFont->glyphs = (Glyph *)AllocLoad_FxElemVisStateSample();
            varGlyph = varFont->glyphs;
            Load_GlyphArray(1, varFont->glyphCount);
        }
        else
        {
            DB_ConvertOffsetToPointer((unsigned int*)&varFont->glyphs);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Load_FontHandle(bool atStreamStart)
{
    const void **inserted; // [esp+0h] [ebp-Ch]
    unsigned int value; // [esp+4h] [ebp-8h]

    Load_Stream(atStreamStart, (unsigned __int8 *)varFontHandle, 4);
    DB_PushStreamPos(0);
    if (*varFontHandle)
    {
        value = (unsigned int)*varFontHandle;
        if (*varFontHandle == (Font_s *)-1 || value == -2)
        {
            *varFontHandle = (Font_s *)AllocLoad_FxElemVisStateSample();
            varFont = *varFontHandle;
            if (value == -2)
                inserted = DB_InsertPointer();
            else
                inserted = 0;
            Load_Font(1);
            Load_FontAsset((XAssetHeader *)varFontHandle);
            if (inserted)
                *inserted = *varFontHandle;
        }
        else
        {
            DB_ConvertOffsetToAlias((unsigned int *)varFontHandle);
        }
    }
    DB_PopStreamPos();
}

void __cdecl Mark_Font()
{
    varMaterialHandle = &varFont->material;
    Mark_MaterialHandle();
    varMaterialHandle = &varFont->glowMaterial;
    Mark_MaterialHandle();
}

void __cdecl Mark_FontHandle()
{
    if (*varFontHandle)
    {
        varFont = *varFontHandle;
        Mark_FontAsset(varFont);
        Mark_Font();
    }
}

void __cdecl Load_XAssetHeader(bool atStreamStart)
{
    switch (varXAsset->type)
    {
    case ASSET_TYPE_PHYSPRESET:
        varPhysPresetPtr = (PhysPreset **)varXAssetHeader;
        Load_PhysPresetPtr(atStreamStart);
        break;
    case ASSET_TYPE_XANIMPARTS:
        varXAnimPartsPtr = (XAnimParts **)varXAssetHeader;
        Load_XAnimPartsPtr(atStreamStart);
        break;
    case ASSET_TYPE_XMODEL:
        varXModelPtr = (XModel **)varXAssetHeader;
        Load_XModelPtr(atStreamStart);
        break;
    case ASSET_TYPE_MATERIAL:
        varMaterialHandle = (Material **)varXAssetHeader;
        Load_MaterialHandle(atStreamStart);
        break;
    case ASSET_TYPE_TECHNIQUE_SET:
        varMaterialTechniqueSetPtr = (MaterialTechniqueSet **)varXAssetHeader;
        Load_MaterialTechniqueSetPtr(atStreamStart);
        break;
    case ASSET_TYPE_IMAGE:
        varGfxImagePtr = (GfxImage **)varXAssetHeader;
        Load_GfxImagePtr(atStreamStart);
        break;
    case ASSET_TYPE_SOUND:
        varsnd_alias_list_ptr = (snd_alias_list_t **)varXAssetHeader;
        Load_snd_alias_list_ptr(atStreamStart);
        break;
    case ASSET_TYPE_SOUND_CURVE:
        varSndCurvePtr = (SndCurve **)varXAssetHeader;
        Load_SndCurvePtr(atStreamStart);
        break;
    case ASSET_TYPE_LOADED_SOUND:
        varLoadedSoundPtr = (LoadedSound **)varXAssetHeader;
        Load_LoadedSoundPtr(atStreamStart);
        break;
    case ASSET_TYPE_CLIPMAP:
    case ASSET_TYPE_CLIPMAP_PVS:
        varclipMap_ptr = (clipMap_t **)varXAssetHeader;
        Load_clipMap_ptr(atStreamStart);
        break;
    case ASSET_TYPE_COMWORLD:
        varComWorldPtr = (ComWorld **)varXAssetHeader;
        Load_ComWorldPtr(atStreamStart);
        break;
    case ASSET_TYPE_GAMEWORLD_SP:
        varGameWorldSpPtr = (GameWorldSp **)varXAssetHeader;
        Load_GameWorldSpPtr(atStreamStart);
        break;
    case ASSET_TYPE_GAMEWORLD_MP:
        varGameWorldMpPtr = (GameWorldMp **)varXAssetHeader;
        Load_GameWorldMpPtr(atStreamStart);
        break;
    case ASSET_TYPE_MAP_ENTS:
        varMapEntsPtr = (MapEnts **)varXAssetHeader;
        Load_MapEntsPtr(atStreamStart);
        break;
    case ASSET_TYPE_GFXWORLD:
        varGfxWorldPtr = (GfxWorld **)varXAssetHeader;
        Load_GfxWorldPtr(atStreamStart);
        break;
    case ASSET_TYPE_LIGHT_DEF:
        varGfxLightDefPtr = (GfxLightDef **)varXAssetHeader;
        Load_GfxLightDefPtr(atStreamStart);
        break;
    case ASSET_TYPE_FONT:
        varFontHandle = (Font_s **)varXAssetHeader;
        Load_FontHandle(atStreamStart);
        break;
    case ASSET_TYPE_MENULIST:
        varMenuListPtr = (MenuList **)varXAssetHeader;
        Load_MenuListPtr(atStreamStart);
        break;
    case ASSET_TYPE_MENU:
        varmenuDef_ptr = (menuDef_t **)varXAssetHeader;
        Load_menuDef_ptr(atStreamStart);
        break;
    case ASSET_TYPE_LOCALIZE_ENTRY:
        varLocalizeEntryPtr = (LocalizeEntry **)varXAssetHeader;
        Load_LocalizeEntryPtr(atStreamStart);
        break;
    case ASSET_TYPE_WEAPON:
        varWeaponDefPtr = (WeaponDef **)varXAssetHeader;
        Load_WeaponDefPtr(atStreamStart);
        break;
    case ASSET_TYPE_FX:
        varFxEffectDefHandle = (const FxEffectDef **)varXAssetHeader;
        Load_FxEffectDefHandle(atStreamStart);
        break;
    case ASSET_TYPE_IMPACT_FX:
        varFxImpactTablePtr = (FxImpactTable **)varXAssetHeader;
        Load_FxImpactTablePtr(atStreamStart);
        break;
    case ASSET_TYPE_RAWFILE:
        varRawFilePtr = (RawFile **)varXAssetHeader;
        Load_RawFilePtr(atStreamStart);
        break;
    case ASSET_TYPE_STRINGTABLE:
        varStringTablePtr = (StringTable **)varXAssetHeader;
        Load_StringTablePtr(atStreamStart);
        break;
    }
}

void __cdecl Load_XAsset(bool atStreamStart)
{
    Load_Stream(atStreamStart, (unsigned __int8 *)varXAsset, 8);
    varXAssetHeader = &varXAsset->header;
    Load_XAssetHeader(0);
}

void __cdecl Mark_XAssetHeader()
{
    switch (varXAsset->type)
    {
    case ASSET_TYPE_PHYSPRESET:
        varPhysPresetPtr = (PhysPreset **)varXAssetHeader;
        Mark_PhysPresetPtr();
        break;
    case ASSET_TYPE_XANIMPARTS:
        varXAnimPartsPtr = (XAnimParts **)varXAssetHeader;
        Mark_XAnimPartsPtr();
        break;
    case ASSET_TYPE_XMODEL:
        varXModelPtr = (XModel **)varXAssetHeader;
        Mark_XModelPtr();
        break;
    case ASSET_TYPE_MATERIAL:
        varMaterialHandle = (Material **)varXAssetHeader;
        Mark_MaterialHandle();
        break;
    case ASSET_TYPE_TECHNIQUE_SET:
        varMaterialTechniqueSetPtr = (MaterialTechniqueSet **)varXAssetHeader;
        Mark_MaterialTechniqueSetPtr();
        break;
    case ASSET_TYPE_IMAGE:
        varGfxImagePtr = (GfxImage **)varXAssetHeader;
        Mark_GfxImagePtr();
        break;
    case ASSET_TYPE_SOUND:
        varsnd_alias_list_ptr = (snd_alias_list_t **)varXAssetHeader;
        Mark_snd_alias_list_ptr();
        break;
    case ASSET_TYPE_SOUND_CURVE:
        varSndCurvePtr = (SndCurve **)varXAssetHeader;
        Mark_SndCurvePtr();
        break;
    case ASSET_TYPE_LOADED_SOUND:
        varLoadedSoundPtr = (LoadedSound **)varXAssetHeader;
        Mark_LoadedSoundPtr();
        break;
    case ASSET_TYPE_CLIPMAP:
    case ASSET_TYPE_CLIPMAP_PVS:
        varclipMap_ptr = (clipMap_t **)varXAssetHeader;
        Mark_clipMap_ptr();
        break;
    case ASSET_TYPE_COMWORLD:
        varComWorldPtr = (ComWorld **)varXAssetHeader;
        Mark_ComWorldPtr();
        break;
    case ASSET_TYPE_GAMEWORLD_SP:
        varGameWorldSpPtr = (GameWorldSp **)varXAssetHeader;
        Mark_GameWorldSpPtr();
        break;
    case ASSET_TYPE_GAMEWORLD_MP:
        varGameWorldMpPtr = (GameWorldMp **)varXAssetHeader;
        Mark_GameWorldMpPtr();
        break;
    case ASSET_TYPE_MAP_ENTS:
        varMapEntsPtr = (MapEnts **)varXAssetHeader;
        Mark_MapEntsPtr();
        break;
    case ASSET_TYPE_GFXWORLD:
        varGfxWorldPtr = (GfxWorld **)varXAssetHeader;
        Mark_GfxWorldPtr();
        break;
    case ASSET_TYPE_LIGHT_DEF:
        varGfxLightDefPtr = (GfxLightDef **)varXAssetHeader;
        Mark_GfxLightDefPtr();
        break;
    case ASSET_TYPE_FONT:
        varFontHandle = (Font_s **)varXAssetHeader;
        Mark_FontHandle();
        break;
    case ASSET_TYPE_MENULIST:
        varMenuListPtr = (MenuList **)varXAssetHeader;
        Mark_MenuListPtr();
        break;
    case ASSET_TYPE_MENU:
        varmenuDef_ptr = (menuDef_t **)varXAssetHeader;
        Mark_menuDef_ptr();
        break;
    case ASSET_TYPE_LOCALIZE_ENTRY:
        varLocalizeEntryPtr = (LocalizeEntry **)varXAssetHeader;
        Mark_LocalizeEntryPtr();
        break;
    case ASSET_TYPE_WEAPON:
        varWeaponDefPtr = (WeaponDef **)varXAssetHeader;
        Mark_WeaponDefPtr();
        break;
    case ASSET_TYPE_FX:
        varFxEffectDefHandle = (const FxEffectDef **)varXAssetHeader;
        Mark_FxEffectDefHandle();
        break;
    case ASSET_TYPE_IMPACT_FX:
        varFxImpactTablePtr = (FxImpactTable **)varXAssetHeader;
        Mark_FxImpactTablePtr();
        break;
    case ASSET_TYPE_RAWFILE:
        varRawFilePtr = (RawFile **)varXAssetHeader;
        Mark_RawFilePtr();
        break;
    case ASSET_TYPE_STRINGTABLE:
        varStringTablePtr = (StringTable **)varXAssetHeader;
        Mark_StringTablePtr();
        break;
    }
}

void __cdecl Mark_XAsset()
{
    varXAssetHeader = &varXAsset->header;
    Mark_XAssetHeader();
}

void __cdecl Mark_SndAliasCustom(snd_alias_list_t **var)
{
    varsnd_alias_list_ptr = var;
    Mark_snd_alias_list_ptr();
}

void __cdecl DB_SaveDObjs()
{
    signed int handle; // [esp+0h] [ebp-Ch]
    signed int handlea; // [esp+0h] [ebp-Ch]
    DObj_s *obj; // [esp+4h] [ebp-8h]
    DObj_s *obja; // [esp+4h] [ebp-8h]
    int localClientNum; // [esp+8h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        for (handle = 0; handle < 1152; ++handle)
        {
            obj = Com_GetClientDObj(handle, localClientNum);
            if (obj)
                DObjArchive(obj);
        }
    }
    for (handlea = 0; handlea < 1024; ++handlea)
    {
        obja = Com_GetServerDObj(handlea);
        if (obja)
            DObjArchive(obja);
    }
}

void __cdecl DB_LoadDObjs()
{
    signed int handle; // [esp+0h] [ebp-Ch]
    signed int handlea; // [esp+0h] [ebp-Ch]
    DObj_s *obj; // [esp+4h] [ebp-8h]
    DObj_s *obja; // [esp+4h] [ebp-8h]
    int localClientNum; // [esp+8h] [ebp-4h]

    for (localClientNum = 0; localClientNum < 1; ++localClientNum)
    {
        for (handle = 0; handle < 1152; ++handle)
        {
            obj = Com_GetClientDObj(handle, localClientNum);
            if (obj)
                DObjUnarchive(obj);
        }
    }
    for (handlea = 0; handlea < 1024; ++handlea)
    {
        obja = Com_GetServerDObj(handlea);
        if (obja)
            DObjUnarchive(obja);
    }
}




