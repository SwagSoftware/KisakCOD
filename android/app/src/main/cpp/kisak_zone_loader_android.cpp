#include "kisak_zone_loader_android.h"
#include <map>

#include <algorithm>
#include <cstring>
#include <sstream>

namespace {
constexpr size_t kXFileHeaderBytes = 44;
constexpr size_t kBlockCount = 9;
constexpr uint32_t kPtrInline = 0xffffffffu;   // -1: data follows in stream
constexpr uint32_t kPtrInsert = 0xfffffffeu;   // -2: data follows, pointer recorded
constexpr uint64_t kMaxTotalBlockBytes = 768ull * 1024 * 1024;

constexpr uint32_t kAssetTypeMaterial = 0x4;
constexpr uint32_t kAssetTypeTechniqueSet = 0x5;
constexpr uint32_t kAssetTypeImage = 0x6;
constexpr uint32_t kAssetTypeSound = 0x7;
constexpr uint32_t kAssetTypeSoundCurve = 0x8;
constexpr uint32_t kAssetTypeLoadedSound = 0x9;
constexpr uint32_t kAssetTypeFont = 0x13;
constexpr uint32_t kAssetTypeLocalize = 0x16;
constexpr uint32_t kAssetTypeRawFile = 0x1f;

constexpr uint32_t kTechniqueCountPerSet = 34;
constexpr uint8_t kTextureSemanticWater = 11;

const char* kAssetTypeNames[] = {
    "xmodelpieces", "physpreset", "xanimparts", "xmodel", "material",
    "techniqueset", "image", "sound", "soundcurve", "loadedsound",
    "clipmap", "clipmap_pvs", "comworld", "gameworld_sp", "gameworld_mp",
    "map_ents", "gfxworld", "lightdef", "uimap", "font", "menulist", "menu",
    "localize", "weapon", "snddriverglobals", "fx", "impactfx", "aitype",
    "mptype", "character", "xmodelalias", "rawfile", "stringtable",
};

std::string AssetTypeName(uint32_t type) {
    if (type < std::size(kAssetTypeNames)) {
        return kAssetTypeNames[type];
    }
    return "type_" + std::to_string(type);
}

// Port of the db_stream.cpp globals: the file cursor, the nine block
// heaps, the per-block stream positions, and the push/pop stack.
class ZoneLoader {
public:
    explicit ZoneLoader(const std::vector<uint8_t>& zone, KisakZoneLoadResult& result)
        : zone_(zone), result_(result) {}

    void Run();

private:
    struct StackEntry {
        uint32_t index;
        uint32_t pos;
    };

    // --- db_file_load.cpp: DB_LoadXFileData ----------------------------
    bool ReadFileData(uint8_t* dest, size_t size) {
        if (failed_) {
            return false;
        }
        if (cursor_ + size > zone_.size()) {
            Fail("lecture au-dela de la fin de zone (cursor=" + std::to_string(cursor_)
                 + " taille=" + std::to_string(size) + ")");
            return false;
        }
        std::memcpy(dest, zone_.data() + cursor_, size);
        cursor_ += size;
        return true;
    }

    // --- db_stream.cpp -------------------------------------------------
    uint8_t* BlockAt(uint32_t index, uint32_t offset) {
        return blocks_[index].data() + offset;
    }

    uint32_t EncodeRef(uint32_t index, uint32_t offset) const {
        return ((index << 28) | offset) + 1;
    }

    bool DecodeRef(uint32_t ref, uint32_t& index, uint32_t& offset) {
        if (ref == 0) {
            return false;
        }
        index = (ref - 1) >> 28;
        offset = (ref - 1) & 0xfffffff;
        if (index >= kBlockCount || offset >= blocks_[index].size()) {
            Fail("reference de bloc invalide: " + std::to_string(ref));
            return false;
        }
        return true;
    }

    void PushStreamPos(uint32_t index) {
        if (stackDepth_ >= 64) {
            Fail("pile de stream saturee");
            return;
        }
        stack_[stackDepth_].index = posIndex_;
        SetStreamIndex(index);
        stack_[stackDepth_++].pos = streamPos_;
    }

    void PopStreamPos() {
        if (stackDepth_ == 0) {
            Fail("PopStreamPos sur pile vide");
            return;
        }
        --stackDepth_;
        if (posIndex_ == 0) {
            streamPos_ = stack_[stackDepth_].pos;
        }
        SetStreamIndex(stack_[stackDepth_].index);
    }

    void SetStreamIndex(uint32_t index) {
        if (index != posIndex_) {
            posArray_[posIndex_] = streamPos_;
            posIndex_ = index;
            streamPos_ = posArray_[index];
        }
    }

    uint32_t AllocStreamPos(uint32_t alignMask) {
        streamPos_ = (streamPos_ + alignMask) & ~alignMask;
        return streamPos_;
    }

    void IncStreamPos(uint32_t size) {
        if (streamPos_ + size > blocks_[posIndex_].size()) {
            Fail("depassement du bloc " + std::to_string(posIndex_)
                 + " (pos=" + std::to_string(streamPos_) + " taille=" + std::to_string(size)
                 + " bloc=" + std::to_string(blocks_[posIndex_].size()) + ")");
            return;
        }
        streamPos_ += size;
        result_.blockUsed[posIndex_] = std::max(result_.blockUsed[posIndex_], streamPos_);
    }

    // --- db_stream_load.cpp: Load_Stream -------------------------------
    // Block 1 holds runtime-only data (zero-filled, absent from the file);
    // blocks 2 and 3 are delay-streamed and read at the end of the walk.
    void LoadStream(bool atStreamStart, uint8_t* ptr, uint32_t size) {
        if (!atStreamStart || size == 0 || failed_) {
            return;
        }
        if (posIndex_ - 1 < 3) {
            if (posIndex_ == 1) {
                std::memset(ptr, 0, size);
            } else {
                delays_.push_back({ptr, size});
            }
        } else {
            ReadFileData(ptr, size);
        }
        IncStreamPos(size);
    }

    void LoadDelayStream() {
        for (const DelayEntry& entry : delays_) {
            if (!ReadFileData(entry.ptr, entry.size)) {
                return;
            }
        }
        delays_.clear();
    }

    // --- db_load.cpp string loading ------------------------------------
    // Load_XStringCustom: copies the inline NUL-terminated string from the
    // file into the active block at the current stream position.
    uint32_t LoadXStringCustom() {
        const uint32_t start = streamPos_;
        while (!failed_) {
            uint8_t ch = 0;
            if (!ReadFileData(&ch, 1)) {
                return 0;
            }
            if (streamPos_ >= blocks_[posIndex_].size()) {
                Fail("string inline depasse le bloc " + std::to_string(posIndex_));
                return 0;
            }
            *BlockAt(posIndex_, streamPos_) = ch;
            IncStreamPos(1);
            if (ch == 0) {
                break;
            }
        }
        return EncodeRef(posIndex_, start);
    }

    // Load_XString on a 4-byte slot living in block memory.
    void LoadXString(uint32_t* slot) {
        if (failed_ || *slot == 0) {
            return;
        }
        if (*slot == kPtrInline) {
            AllocStreamPos(0);
            *slot = LoadXStringCustom();
        }
        // Any other value is already the offset encoding used by
        // DB_ConvertOffsetToPointer; it stays as-is in this port.
    }

    std::string ResolveString(uint32_t ref) {
        uint32_t index = 0;
        uint32_t offset = 0;
        if (!DecodeRef(ref, index, offset)) {
            return {};
        }
        const uint8_t* begin = BlockAt(index, offset);
        const uint8_t* end = blocks_[index].data() + blocks_[index].size();
        const uint8_t* terminator = std::find(begin, end, uint8_t{0});
        return std::string(reinterpret_cast<const char*>(begin), terminator - begin);
    }

    uint32_t ReadSlot(uint32_t block, uint32_t offset) {
        uint32_t value = 0;
        std::memcpy(&value, BlockAt(block, offset), 4);
        return value;
    }

    void WriteSlot(uint32_t block, uint32_t offset, uint32_t value) {
        std::memcpy(BlockAt(block, offset), &value, 4);
    }

    // Load_<T>Ptr without block push: -1 allocates inline data in the
    // current block, anything else stays as the engine's offset encoding
    // (DB_ConvertOffsetToPointer). Returns true when inline data follows.
    bool HandleInlinePtr(uint32_t block, uint32_t slotOffset, uint32_t alignMask, uint32_t& outOffset) {
        const uint32_t value = ReadSlot(block, slotOffset);
        if (value != kPtrInline) {
            return false;
        }
        outOffset = AllocStreamPos(alignMask);
        WriteSlot(block, slotOffset, EncodeRef(posIndex_, outOffset));
        return true;
    }

    // Pattern `if (ptr) { ptr = Alloc...; Load...(1, n); }` — the engine
    // allocates for ANY non-zero value without a -1 check.
    uint32_t AllocForSlot(uint32_t block, uint32_t slotOffset, uint32_t alignMask) {
        const uint32_t outOffset = AllocStreamPos(alignMask);
        WriteSlot(block, slotOffset, EncodeRef(posIndex_, outOffset));
        return outOffset;
    }

    uint16_t ReadU16(uint32_t block, uint32_t offset) {
        uint16_t value = 0;
        std::memcpy(&value, BlockAt(block, offset), 2);
        return value;
    }

    // DB_AddXAsset role: asset structs deserialize into the rewinding temp
    // block, so persist a copy in the asset pool (pseudo-block 15) and
    // reference that from the pointer slot.
    uint32_t PoolAsset(uint32_t block, uint32_t structOffset, uint32_t size) {
        const uint32_t poolOffset = static_cast<uint32_t>(result_.assetPool.size());
        result_.assetPool.insert(
            result_.assetPool.end(),
            BlockAt(block, structOffset),
            BlockAt(block, structOffset) + size
        );
        return ((15u << 28) | poolOffset) + 1;
    }

    // Shared shape of every Load_<T>Ptr/Handle: push block 0, then -1/-2
    // deserializes inline (with DB_InsertPointer for -2) and anything else
    // is an alias to an already-loaded asset (DB_ConvertOffsetToAlias).
    void HandleAssetSlot(uint32_t block, uint32_t slotOffset, void (ZoneLoader::*load)(uint32_t), uint32_t structSize) {
        PushStreamPos(0);
        const uint32_t value = ReadSlot(block, slotOffset);
        if (value != 0) {
            if (value == kPtrInline || value == kPtrInsert) {
                uint32_t insertRef = 0;
                if (value == kPtrInsert) {
                    PushStreamPos(4);
                    const uint32_t insertOffset = AllocStreamPos(3);
                    IncStreamPos(4);
                    insertRef = EncodeRef(4, insertOffset);
                    PopStreamPos();
                }
                const uint32_t structOffset = AllocStreamPos(3);
                WriteSlot(block, slotOffset, EncodeRef(posIndex_, structOffset));
                (this->*load)(structOffset);
                if (!failed_) {
                    WriteSlot(block, slotOffset, PoolAsset(posIndex_, structOffset, structSize));
                }
                if (insertRef != 0) {
                    uint32_t index = 0;
                    uint32_t offset = 0;
                    if (DecodeRef(insertRef, index, offset)) {
                        const uint32_t slotValue = ReadSlot(block, slotOffset);
                        std::memcpy(BlockAt(index, offset), &slotValue, 4);
                    }
                }
            } else {
                uint32_t index = 0;
                uint32_t offset = 0;
                if (DecodeRef(value, index, offset)) {
                    WriteSlot(block, slotOffset, ReadSlot(index, offset));
                }
            }
        }
        PopStreamPos();
    }

    // --- db_load.cpp / db_file_load.cpp asset walk ---------------------
    void LoadScriptStringList(uint32_t count, uint32_t stringsRef);
    void LoadLocalizeEntry(uint32_t structOffset);
    void LoadRawFile(uint32_t structOffset);
    void LoadGfxTextureLoad(uint32_t block, uint32_t slotOffset, uint16_t width, uint16_t height, uint32_t nameRef);
    void LoadGfxImage(uint32_t structOffset);
    void LoadGfxImagePtr(uint32_t block, uint32_t slotOffset);
    void LoadSndCurve(uint32_t structOffset);
    void LoadSpeakerMap(uint32_t structOffset);
    void LoadLoadedSound(uint32_t structOffset);
    void LoadSoundFile(uint32_t structOffset);
    void LoadSndAlias(uint32_t block, uint32_t aliasOffset);
    void LoadSndAliasList(uint32_t structOffset);
    void LoadFont(uint32_t structOffset);
    void LoadGfxLightDef(uint32_t structOffset);
    void LoadFxEffectDefRefSlot(uint32_t block, uint32_t slotOffset);
    void LoadFxElemVisualSlot(uint32_t block, uint32_t slotOffset, uint8_t elemType);
    void LoadFxTrailDef(uint32_t structOffset);
    void LoadFxElemDef(uint32_t block, uint32_t elemOffset);
    void LoadFxEffectDef(uint32_t structOffset);
    void LoadFxImpactTable(uint32_t structOffset);
    void LoadPhysPreset(uint32_t structOffset);
    void LoadStringTable(uint32_t structOffset);
    void LoadXAnimPartTrans(uint32_t structOffset, uint16_t numframes);
    void LoadXAnimDeltaPartQuat(uint32_t structOffset, uint16_t numframes);
    void LoadXAnimParts(uint32_t structOffset);
    void LoadXSurfaceCollisionTree(uint32_t structOffset);
    void LoadXSurface(uint32_t block, uint32_t surfOffset);
    void LoadBrushWrapper(uint32_t structOffset);
    void LoadPhysGeomList(uint32_t structOffset);
    void LoadXModel(uint32_t structOffset);
    void LoadXStringPtrSlot(uint32_t block, uint32_t slotOffset);
    void LoadWeaponDef(uint32_t structOffset);
    void LoadStatement(uint32_t block, uint32_t stmtOffset);
    void LoadItemKeyHandlerChain(uint32_t block, uint32_t slotOffset);
    void LoadWindowDef(uint32_t block, uint32_t windowOffset);
    void LoadItemDef(uint32_t structOffset);
    void LoadMenuDef(uint32_t structOffset);
    void LoadMenuList(uint32_t structOffset);
    void LoadWater(uint32_t structOffset);
    void LoadShader(uint32_t structOffset);
    void LoadMaterialShaderArgumentArray(uint32_t arrayOffset, uint32_t count);
    void LoadMaterialPass(uint32_t block, uint32_t passOffset);
    void LoadMaterialTechnique(uint32_t structOffset);
    void LoadMaterialTechniqueSet(uint32_t structOffset);
    void LoadMaterialTextureDefArray(uint32_t arrayOffset, uint32_t count);
    void LoadMaterial(uint32_t structOffset);
    void LoadComWorld(uint32_t structOffset);
    void LoadPathNodeTreeNode(uint32_t block, uint32_t nodeOffset);
    void LoadGameWorldSp(uint32_t structOffset);
    void LoadGameWorldMp(uint32_t structOffset);
    void LoadMapEnts(uint32_t structOffset);
    void LoadCPlaneSlot(uint32_t block, uint32_t slotOffset, uint32_t count);
    void LoadCBrushSideAt(uint32_t block, uint32_t sideOffset);
    void LoadCBrushAt(uint32_t block, uint32_t brushOffset);
    void LoadXModelPiecesSlot(uint32_t block, uint32_t slotOffset);
    void LoadClipMap(uint32_t structOffset);
    void LoadGfxAabbTreeAt(uint32_t block, uint32_t treeOffset);
    void LoadGfxPortalAt(uint32_t block, uint32_t portalOffset);
    void LoadGfxCellAt(uint32_t block, uint32_t cellOffset);
    void LoadGfxWorld(uint32_t structOffset);
    bool LoadAssetHeader(uint32_t type, uint32_t* headerSlot);

    void Fail(std::string message) {
        if (!failed_) {
            failed_ = true;
            result_.error = std::move(message) + " (asset " + std::to_string(currentAssetIndex_)
                + " type " + AssetTypeName(currentAssetType_) + ", cursor " + std::to_string(cursor_) + ")";
        }
    }

    struct DelayEntry {
        uint8_t* ptr;
        uint32_t size;
    };

    const std::vector<uint8_t>& zone_;
    KisakZoneLoadResult& result_;
    size_t cursor_ = kXFileHeaderBytes;
    std::array<std::vector<uint8_t>, kBlockCount> blocks_;
    std::array<uint32_t, kBlockCount> posArray_{};
    uint32_t posIndex_ = 0;
    uint32_t streamPos_ = 0;
    std::array<StackEntry, 64> stack_{};
    uint32_t stackDepth_ = 0;
    std::vector<DelayEntry> delays_;
    uint32_t currentAssetIndex_ = 0;
    uint32_t currentAssetType_ = 0;
    bool failed_ = false;
};

// Load_ScriptStringList + Load_TempStringArray: the pointer array lands in
// the active block, each -1 entry is followed by its inline string.
void ZoneLoader::LoadScriptStringList(uint32_t count, uint32_t stringsRef) {
    if (count == 0 || stringsRef == 0) {
        return;
    }
    if (stringsRef != kPtrInline) {
        Fail("ScriptStringList.strings n'est pas inline");
        return;
    }

    PushStreamPos(4);
    const uint32_t arrayOffset = AllocStreamPos(3);
    const uint32_t arrayBytes = count * 4;
    if (arrayOffset + arrayBytes > blocks_[posIndex_].size()) {
        Fail("tableau de script strings depasse le bloc");
        return;
    }
    LoadStream(true, BlockAt(posIndex_, arrayOffset), arrayBytes);

    for (uint32_t i = 0; i < count && !failed_; ++i) {
        auto* slot = reinterpret_cast<uint32_t*>(BlockAt(posIndex_, arrayOffset + i * 4));
        if (*slot == kPtrInline) {
            AllocStreamPos(0);
            *slot = LoadXStringCustom();
            result_.scriptStrings.push_back(ResolveString(*slot));
        }
    }
    PopStreamPos();
}

// Load_LocalizeEntry: 8-byte struct {value, name} in the current (temp)
// block; both XStrings land in block 4.
void ZoneLoader::LoadLocalizeEntry(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 8);
    PushStreamPos(4);
    auto* value = reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset));
    auto* name = reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 4));
    LoadXString(value);
    LoadXString(name);
    PopStreamPos();

    if (!failed_) {
        result_.localizeEntries.push_back({ResolveString(*name), ResolveString(*value)});
    }
}

// Load_RawFile: 12-byte struct {name, len, buffer} in the current (temp)
// block; the buffer is always inline in block 4.
void ZoneLoader::LoadRawFile(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 12);
    PushStreamPos(4);
    auto* name = reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset));
    uint32_t length = 0;
    std::string contents;
    std::memcpy(&length, BlockAt(structBlock, structOffset + 4), 4);
    auto* buffer = reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 8));
    LoadXString(name);
    if (*buffer != 0) {
        const uint32_t contentOffset = AllocStreamPos(0);
        if (length + 1 > blocks_[posIndex_].size() - contentOffset) {
            Fail("contenu rawfile depasse le bloc");
        } else {
            LoadStream(true, BlockAt(posIndex_, contentOffset), length + 1);
            const auto* content = reinterpret_cast<const char*>(BlockAt(posIndex_, contentOffset));
            contents.assign(content, strnlen(content, length));
            *buffer = EncodeRef(posIndex_, contentOffset);
        }
    }
    PopStreamPos();

    if (!failed_) {
        result_.rawFiles.push_back({ResolveString(*name), length, std::move(contents)});
    }
}

// Load_GfxTextureLoad: 4-byte union slot; -1/-2 allocates the
// GfxImageLoadDef (16-byte header + resourceSize inline pixel bytes) in the
// current (temp) block. The D3D texture creation hook is skipped.
void ZoneLoader::LoadGfxTextureLoad(
    uint32_t block,
    uint32_t slotOffset,
    uint16_t width,
    uint16_t height,
    uint32_t nameRef
) {
    PushStreamPos(0);
    const uint32_t value = ReadSlot(block, slotOffset);
    if (value != 0) {
        if (value == kPtrInline || value == kPtrInsert) {
            uint32_t insertRef = 0;
            if (value == kPtrInsert) {
                PushStreamPos(4);
                const uint32_t insertOffset = AllocStreamPos(3);
                IncStreamPos(4);
                insertRef = EncodeRef(4, insertOffset);
                PopStreamPos();
            }

            const uint32_t defOffset = AllocStreamPos(3);
            WriteSlot(block, slotOffset, EncodeRef(posIndex_, defOffset));
            const uint32_t defBlock = posIndex_;
            LoadStream(true, BlockAt(defBlock, defOffset), 16);

            uint32_t resourceSize = 0;
            std::memcpy(&resourceSize, BlockAt(defBlock, defOffset + 12), 4);
            const uint32_t pixelOffset = streamPos_;
            if (!failed_ && resourceSize > 0) {
                if (pixelOffset + resourceSize > blocks_[defBlock].size()) {
                    Fail("pixels d'image depassent le bloc " + std::to_string(defBlock));
                } else {
                    LoadStream(true, BlockAt(defBlock, pixelOffset), resourceSize);
                }
            }

            if (!failed_) {
                KisakLoadedImage image;
                image.name = ResolveString(nameRef);
                image.width = width;
                image.height = height;
                image.levelCount = *BlockAt(defBlock, defOffset);
                std::memcpy(&image.format, BlockAt(defBlock, defOffset + 8), 4);
                image.resourceSize = resourceSize;
                image.hasInlinePixels = resourceSize > 0;
                if (resourceSize > 0) {
                    // The payload lives in the rewinding temp block; keep a
                    // copy so consumers (lightmaps) survive the walk.
                    image.pixels.assign(
                        BlockAt(defBlock, pixelOffset),
                        BlockAt(defBlock, pixelOffset) + resourceSize
                    );
                }
                result_.images.push_back(std::move(image));
            }
            if (insertRef != 0) {
                uint32_t index = 0;
                uint32_t offset = 0;
                if (DecodeRef(insertRef, index, offset)) {
                    const uint32_t slotValue = ReadSlot(block, slotOffset);
                    std::memcpy(BlockAt(index, offset), &slotValue, 4);
                }
            }
        }
        // Other values reference an already-created texture; kept as-is.
    }
    PopStreamPos();
}

// Load_GfxImage: 36-byte struct; name at +32, texture union slot at +4,
// width/height at +24/+26.
void ZoneLoader::LoadGfxImage(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 36);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 32)));
    uint16_t width = 0;
    uint16_t height = 0;
    std::memcpy(&width, BlockAt(structBlock, structOffset + 24), 2);
    std::memcpy(&height, BlockAt(structBlock, structOffset + 26), 2);
    const uint32_t nameRef = ReadSlot(structBlock, structOffset + 32);
    LoadGfxTextureLoad(structBlock, structOffset + 4, width, height, nameRef);
    PopStreamPos();
}

// Load_GfxImagePtr: asset-style slot (push 0, -1/-2 inline, alias).
void ZoneLoader::LoadGfxImagePtr(uint32_t block, uint32_t slotOffset) {
    HandleAssetSlot(block, slotOffset, &ZoneLoader::LoadGfxImage, 36);
}

// Load_water_t: 68-byte struct; H0 at +4 (N*M complex), wTerm at +8
// (N*M floats), M at +12, N at +16, image ptr at +64.
void ZoneLoader::LoadWater(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 68);
    uint32_t m = 0;
    uint32_t n = 0;
    std::memcpy(&m, BlockAt(structBlock, structOffset + 12), 4);
    std::memcpy(&n, BlockAt(structBlock, structOffset + 16), 4);

    uint32_t dataOffset = 0;
    if (ReadSlot(structBlock, structOffset + 4) != 0
        && HandleInlinePtr(structBlock, structOffset + 4, 3, dataOffset)) {
        LoadStream(true, BlockAt(posIndex_, dataOffset), n * m * 8);
    }
    if (ReadSlot(structBlock, structOffset + 8) != 0
        && HandleInlinePtr(structBlock, structOffset + 8, 3, dataOffset)) {
        LoadStream(true, BlockAt(posIndex_, dataOffset), n * m * 4);
    }
    LoadGfxImagePtr(structBlock, structOffset + 64);
}

// Load_MaterialVertexShader / Load_MaterialPixelShader: identical 16-byte
// layout {name, shader, program ptr, programSize, loadForRenderer}; the
// D3D shader-create hooks are skipped.
void ZoneLoader::LoadShader(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 16);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    if (ReadSlot(structBlock, structOffset + 8) != 0) {
        uint16_t programSize = 0;
        std::memcpy(&programSize, BlockAt(structBlock, structOffset + 12), 2);
        const uint32_t programOffset = AllocStreamPos(3);
        WriteSlot(structBlock, structOffset + 8, EncodeRef(posIndex_, programOffset));
        LoadStream(true, BlockAt(posIndex_, programOffset), programSize * 4u);
    }
}

// Load_MaterialShaderArgumentArray: 8-byte args already bulk-read; only
// literal constants (type 1/7) pull an inline float[4].
void ZoneLoader::LoadMaterialShaderArgumentArray(uint32_t arrayOffset, uint32_t count) {
    const uint32_t block = posIndex_;
    for (uint32_t i = 0; i < count && !failed_; ++i) {
        const uint32_t argOffset = arrayOffset + i * 8;
        uint16_t type = 0;
        std::memcpy(&type, BlockAt(block, argOffset), 2);
        if (type == 1 || type == 7) {
            uint32_t literalOffset = 0;
            if (ReadSlot(block, argOffset + 4) != 0
                && HandleInlinePtr(block, argOffset + 4, 3, literalOffset)) {
                LoadStream(true, BlockAt(posIndex_, literalOffset), 16);
            }
        }
    }
}

// Load_MaterialPass: 20-byte pass already bulk-read; fixes up vertexDecl
// (100-byte inline), both shaders, and the argument array.
void ZoneLoader::LoadMaterialPass(uint32_t block, uint32_t passOffset) {
    uint32_t declOffset = 0;
    if (ReadSlot(block, passOffset) != 0
        && HandleInlinePtr(block, passOffset, 3, declOffset)) {
        LoadStream(true, BlockAt(posIndex_, declOffset), 100);
    }

    for (uint32_t slot = 4; slot <= 8 && !failed_; slot += 4) {
        uint32_t shaderOffset = 0;
        if (ReadSlot(block, passOffset + slot) != 0
            && HandleInlinePtr(block, passOffset + slot, 3, shaderOffset)) {
            LoadShader(shaderOffset);
        }
    }

    if (ReadSlot(block, passOffset + 16) != 0 && !failed_) {
        uint32_t argCount = 0;
        for (uint32_t i = 12; i < 15; ++i) {
            argCount += *BlockAt(block, passOffset + i);
        }
        const uint32_t argsOffset = AllocStreamPos(3);
        WriteSlot(block, passOffset + 16, EncodeRef(posIndex_, argsOffset));
        LoadStream(true, BlockAt(posIndex_, argsOffset), argCount * 8);
        LoadMaterialShaderArgumentArray(argsOffset, argCount);
    }
}

// Load_MaterialTechnique: variable-length struct — 8-byte header, then
// passCount * 20-byte passes contiguously, then the technique name.
void ZoneLoader::LoadMaterialTechnique(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 8);
    uint16_t passCount = 0;
    std::memcpy(&passCount, BlockAt(structBlock, structOffset + 6), 2);

    const uint32_t passArrayOffset = streamPos_;
    LoadStream(true, BlockAt(structBlock, passArrayOffset), passCount * 20u);
    for (uint16_t i = 0; i < passCount && !failed_; ++i) {
        LoadMaterialPass(structBlock, passArrayOffset + i * 20u);
    }
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
}

// Load_MaterialTechniqueSet: 148-byte struct, name at +0, 34 technique
// slots at +12.
void ZoneLoader::LoadMaterialTechniqueSet(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 148);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    KisakLoadedTechniqueSet techniqueSet;
    techniqueSet.name = ResolveString(ReadSlot(structBlock, structOffset));

    for (uint32_t i = 0; i < kTechniqueCountPerSet && !failed_; ++i) {
        const uint32_t slotOffset = structOffset + 12 + i * 4;
        uint32_t techniqueOffset = 0;
        if (ReadSlot(structBlock, slotOffset) != 0) {
            ++techniqueSet.techniqueCount;
            if (HandleInlinePtr(structBlock, slotOffset, 3, techniqueOffset)) {
                LoadMaterialTechnique(techniqueOffset);
                uint16_t passCount = 0;
                std::memcpy(&passCount, BlockAt(posIndex_, techniqueOffset + 6), 2);
                techniqueSet.passCount += passCount;
            }
        }
    }
    PopStreamPos();

    if (!failed_) {
        result_.techniqueSets.push_back(std::move(techniqueSet));
    }
}

// Load_MaterialTextureDefArray: 12-byte defs already bulk-read; semantic 11
// resolves to a water_t, everything else to a GfxImage.
void ZoneLoader::LoadMaterialTextureDefArray(uint32_t arrayOffset, uint32_t count) {
    const uint32_t block = posIndex_;
    for (uint32_t i = 0; i < count && !failed_; ++i) {
        const uint32_t defOffset = arrayOffset + i * 12;
        const uint8_t semantic = *BlockAt(block, defOffset + 7);
        if (semantic == kTextureSemanticWater) {
            uint32_t waterOffset = 0;
            if (ReadSlot(block, defOffset + 8) != 0
                && HandleInlinePtr(block, defOffset + 8, 3, waterOffset)) {
                LoadWater(waterOffset);
            }
        } else {
            LoadGfxImagePtr(block, defOffset + 8);
        }
    }
}

// Load_Material: 80-byte struct — info (name at +0), techniqueSet slot at
// +64, textureTable at +68 (count at +58), constantTable at +72 (count at
// +59, 16-byte aligned), stateBitsTable at +76 (count at +60).
void ZoneLoader::LoadMaterial(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 80);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    KisakLoadedMaterial material;
    material.name = ResolveString(ReadSlot(structBlock, structOffset));
    material.textureCount = *BlockAt(structBlock, structOffset + 58);

    // Load_MaterialTechniqueSetPtr: asset-style slot.
    HandleAssetSlot(structBlock, structOffset + 64, &ZoneLoader::LoadMaterialTechniqueSet, 148);

    uint32_t tableOffset = 0;
    if (ReadSlot(structBlock, structOffset + 68) != 0
        && HandleInlinePtr(structBlock, structOffset + 68, 3, tableOffset)) {
        const uint32_t count = *BlockAt(structBlock, structOffset + 58);
        LoadStream(true, BlockAt(posIndex_, tableOffset), count * 12u);
        LoadMaterialTextureDefArray(tableOffset, count);
    }
    if (ReadSlot(structBlock, structOffset + 72) != 0
        && HandleInlinePtr(structBlock, structOffset + 72, 15, tableOffset)) {
        const uint32_t count = *BlockAt(structBlock, structOffset + 59);
        LoadStream(true, BlockAt(posIndex_, tableOffset), count * 32u);
    }
    if (ReadSlot(structBlock, structOffset + 76) != 0
        && HandleInlinePtr(structBlock, structOffset + 76, 3, tableOffset)) {
        const uint32_t count = *BlockAt(structBlock, structOffset + 60);
        LoadStream(true, BlockAt(posIndex_, tableOffset), count * 8u);
    }
    PopStreamPos();

    if (!failed_) {
        result_.materials.push_back(std::move(material));
    }
}

// Load_SndCurve: 72-byte struct, only the filename at +0 is a pointer.
void ZoneLoader::LoadSndCurve(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 72);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    PopStreamPos();
    if (!failed_) {
        ++result_.soundCurveCount;
    }
}

// Load_SpeakerMap: 408-byte struct, name at +4; no block push in the engine.
void ZoneLoader::LoadSpeakerMap(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 408);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 4)));
}

// Load_LoadedSound: 44-byte struct {name, MssSound}; the MSS audio bytes at
// +40 land in the temp block (data_len at +12); SND_SetData is skipped.
void ZoneLoader::LoadLoadedSound(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 44);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    PushStreamPos(0);
    const uint32_t dataValue = ReadSlot(structBlock, structOffset + 40);
    if (dataValue != 0) {
        if (dataValue < kPtrInsert) {
            uint32_t index = 0;
            uint32_t offset = 0;
            if (DecodeRef(dataValue, index, offset)) {
                WriteSlot(structBlock, structOffset + 40, ReadSlot(index, offset));
            }
        } else {
            uint32_t insertRef = 0;
            if (dataValue == kPtrInsert) {
                PushStreamPos(4);
                const uint32_t insertOffset = AllocStreamPos(3);
                IncStreamPos(4);
                insertRef = EncodeRef(4, insertOffset);
                PopStreamPos();
            }
            uint32_t dataLen = 0;
            std::memcpy(&dataLen, BlockAt(structBlock, structOffset + 12), 4);
            const uint32_t dataOffset = AllocStreamPos(0);
            WriteSlot(structBlock, structOffset + 40, EncodeRef(posIndex_, dataOffset));
            if (dataOffset + dataLen > blocks_[posIndex_].size()) {
                Fail("donnees audio depassent le bloc " + std::to_string(posIndex_));
            } else {
                LoadStream(true, BlockAt(posIndex_, dataOffset), dataLen);
            }
            if (insertRef != 0) {
                uint32_t index = 0;
                uint32_t offset = 0;
                if (DecodeRef(insertRef, index, offset)) {
                    const uint32_t slotValue = ReadSlot(structBlock, structOffset + 40);
                    std::memcpy(BlockAt(index, offset), &slotValue, 4);
                }
            }
        }
    }
    PopStreamPos();
    PopStreamPos();

    if (!failed_) {
        ++result_.loadedSoundCount;
    }
}

// Load_SoundFile: 12-byte struct {type, exists, u}; type 1 is a loaded
// sound (asset-style slot), everything else a streamed dir/name pair.
void ZoneLoader::LoadSoundFile(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 12);
    const uint8_t type = *BlockAt(structBlock, structOffset);
    if (type == 1) {
        HandleAssetSlot(structBlock, structOffset + 4, &ZoneLoader::LoadLoadedSound, 44);
    } else {
        LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 4)));
        LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 8)));
    }
}

// Load_snd_alias_t: 92-byte alias already bulk-read; four name strings,
// then the sound file, falloff curve (asset-style) and speaker map.
void ZoneLoader::LoadSndAlias(uint32_t block, uint32_t aliasOffset) {
    for (uint32_t slot = 0; slot <= 12 && !failed_; slot += 4) {
        LoadXString(reinterpret_cast<uint32_t*>(BlockAt(block, aliasOffset + slot)));
    }

    uint32_t inlineOffset = 0;
    if (ReadSlot(block, aliasOffset + 16) != 0
        && HandleInlinePtr(block, aliasOffset + 16, 3, inlineOffset)) {
        LoadSoundFile(inlineOffset);
    }
    HandleAssetSlot(block, aliasOffset + 72, &ZoneLoader::LoadSndCurve, 72);
    if (ReadSlot(block, aliasOffset + 88) != 0
        && HandleInlinePtr(block, aliasOffset + 88, 3, inlineOffset)) {
        LoadSpeakerMap(inlineOffset);
    }
}

// Load_snd_alias_list_t: 12-byte struct {aliasName, head, count}; head is
// an inline array of 92-byte aliases.
void ZoneLoader::LoadSndAliasList(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 12);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    KisakLoadedSoundAlias alias;
    alias.name = ResolveString(ReadSlot(structBlock, structOffset));
    std::memcpy(&alias.aliasCount, BlockAt(structBlock, structOffset + 8), 4);

    uint32_t headOffset = 0;
    if (ReadSlot(structBlock, structOffset + 4) != 0
        && HandleInlinePtr(structBlock, structOffset + 4, 3, headOffset)) {
        const uint32_t headBlock = posIndex_;
        LoadStream(true, BlockAt(headBlock, headOffset), alias.aliasCount * 92u);
        for (uint32_t i = 0; i < alias.aliasCount && !failed_; ++i) {
            LoadSndAlias(headBlock, headOffset + i * 92u);
        }
    }
    PopStreamPos();

    if (!failed_) {
        result_.soundAliases.push_back(std::move(alias));
    }
}

// Load_Font: 24-byte struct {fontName, pixelHeight, glyphCount, material,
// glowMaterial, glyphs}; both materials are asset-style, glyphs are a raw
// 24-byte-per-entry array.
void ZoneLoader::LoadFont(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 24);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    KisakLoadedFont font;
    font.name = ResolveString(ReadSlot(structBlock, structOffset));
    std::memcpy(&font.glyphCount, BlockAt(structBlock, structOffset + 8), 4);

    HandleAssetSlot(structBlock, structOffset + 12, &ZoneLoader::LoadMaterial, 80);
    HandleAssetSlot(structBlock, structOffset + 16, &ZoneLoader::LoadMaterial, 80);

    uint32_t glyphOffset = 0;
    if (ReadSlot(structBlock, structOffset + 20) != 0
        && HandleInlinePtr(structBlock, structOffset + 20, 3, glyphOffset)) {
        LoadStream(true, BlockAt(posIndex_, glyphOffset), font.glyphCount * 24u);
    }
    PopStreamPos();

    if (!failed_) {
        result_.fonts.push_back(std::move(font));
    }
}

// Load_GfxLightDef: 16 bytes {name, GfxLightImage attenuation, lookup}.
void ZoneLoader::LoadGfxLightDef(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 16);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    LoadGfxImagePtr(structBlock, structOffset + 4);
    PopStreamPos();
    if (!failed_) {
        ++result_.lightDefCount;
    }
}

// Load_FxEffectDefRef: the slot is an effect NAME string; the runtime
// FromName lookup is skipped.
void ZoneLoader::LoadFxEffectDefRefSlot(uint32_t block, uint32_t slotOffset) {
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(block, slotOffset)));
}

// Load_FxElemVisuals: 4-byte union dispatched on the element type.
void ZoneLoader::LoadFxElemVisualSlot(uint32_t block, uint32_t slotOffset, uint8_t elemType) {
    switch (elemType) {
        case 5:
            HandleAssetSlot(block, slotOffset, &ZoneLoader::LoadXModel, 220);
            break;
        case 10:
            LoadFxEffectDefRefSlot(block, slotOffset);
            break;
        case 8:
            LoadXString(reinterpret_cast<uint32_t*>(BlockAt(block, slotOffset)));
            break;
        case 6:
        case 7:
            break;
        default:
            HandleAssetSlot(block, slotOffset, &ZoneLoader::LoadMaterial, 80);
            break;
    }
}

// Load_FxTrailDef: 28 bytes {scrollTime, repeatDist, splitDist, vertCount,
// verts, indCount, inds}.
void ZoneLoader::LoadFxTrailDef(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 28);
    uint32_t vertCount = 0;
    uint32_t indCount = 0;
    std::memcpy(&vertCount, BlockAt(structBlock, structOffset + 12), 4);
    std::memcpy(&indCount, BlockAt(structBlock, structOffset + 20), 4);
    if (ReadSlot(structBlock, structOffset + 16) != 0) {
        const uint32_t verts = AllocForSlot(structBlock, structOffset + 16, 3);
        LoadStream(true, BlockAt(posIndex_, verts), vertCount * 20u);
    }
    if (ReadSlot(structBlock, structOffset + 24) != 0) {
        const uint32_t inds = AllocForSlot(structBlock, structOffset + 24, 1);
        LoadStream(true, BlockAt(posIndex_, inds), indCount * 2u);
    }
}

// Load_FxElemDef: 252-byte element already bulk-read; velocity/visual
// samples, visuals union, three effect refs and the trail def follow.
void ZoneLoader::LoadFxElemDef(uint32_t block, uint32_t elemOffset) {
    const uint8_t elemType = *BlockAt(block, elemOffset + 176);
    const uint8_t visualCount = *BlockAt(block, elemOffset + 177);
    const uint8_t velIntervalCount = *BlockAt(block, elemOffset + 178);
    const uint8_t visStateIntervalCount = *BlockAt(block, elemOffset + 179);

    if (ReadSlot(block, elemOffset + 180) != 0) {
        const uint32_t samples = AllocForSlot(block, elemOffset + 180, 3);
        LoadStream(true, BlockAt(posIndex_, samples), (velIntervalCount + 1u) * 96u);
    }
    if (ReadSlot(block, elemOffset + 184) != 0) {
        const uint32_t samples = AllocForSlot(block, elemOffset + 184, 3);
        LoadStream(true, BlockAt(posIndex_, samples), (visStateIntervalCount + 1u) * 48u);
    }

    // Load_FxElemDefVisuals on the 4-byte union at +188.
    if (elemType == 9) {
        if (ReadSlot(block, elemOffset + 188) != 0) {
            const uint32_t marks = AllocForSlot(block, elemOffset + 188, 3);
            const uint32_t markBlock = posIndex_;
            LoadStream(true, BlockAt(markBlock, marks), visualCount * 8u);
            for (uint8_t i = 0; i < visualCount && !failed_; ++i) {
                HandleAssetSlot(markBlock, marks + i * 8, &ZoneLoader::LoadMaterial, 80);
                HandleAssetSlot(markBlock, marks + i * 8 + 4, &ZoneLoader::LoadMaterial, 80);
            }
        }
    } else if (visualCount > 1) {
        if (ReadSlot(block, elemOffset + 188) != 0) {
            const uint32_t visuals = AllocForSlot(block, elemOffset + 188, 3);
            const uint32_t visualBlock = posIndex_;
            LoadStream(true, BlockAt(visualBlock, visuals), visualCount * 4u);
            for (uint8_t i = 0; i < visualCount && !failed_; ++i) {
                LoadFxElemVisualSlot(visualBlock, visuals + i * 4, elemType);
            }
        }
    } else {
        LoadFxElemVisualSlot(block, elemOffset + 188, elemType);
    }

    LoadFxEffectDefRefSlot(block, elemOffset + 216);
    LoadFxEffectDefRefSlot(block, elemOffset + 220);
    LoadFxEffectDefRefSlot(block, elemOffset + 224);

    if (ReadSlot(block, elemOffset + 244) != 0) {
        const uint32_t trail = AllocForSlot(block, elemOffset + 244, 3);
        LoadFxTrailDef(trail);
    }
}

// Load_FxEffectDef: 32 bytes {name, flags, totalSize, msecLoopingLife,
// three elemDef counts, elemDefs}.
void ZoneLoader::LoadFxEffectDef(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 32);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    uint32_t elemCount = 0;
    for (uint32_t field = 16; field <= 24; field += 4) {
        uint32_t value = 0;
        std::memcpy(&value, BlockAt(structBlock, structOffset + field), 4);
        elemCount += value;
    }
    if (ReadSlot(structBlock, structOffset + 28) != 0) {
        const uint32_t elems = AllocForSlot(structBlock, structOffset + 28, 3);
        const uint32_t elemBlock = posIndex_;
        LoadStream(true, BlockAt(elemBlock, elems), elemCount * 252u);
        for (uint32_t i = 0; i < elemCount && !failed_; ++i) {
            LoadFxElemDef(elemBlock, elems + i * 252u);
        }
    }
    PopStreamPos();
    if (!failed_) {
        ++result_.fxCount;
    }
}

// Load_FxImpactTable: 8 bytes {name, table of 12 entries x 33 fx handles}.
void ZoneLoader::LoadFxImpactTable(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 8);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    if (ReadSlot(structBlock, structOffset + 4) != 0) {
        const uint32_t table = AllocForSlot(structBlock, structOffset + 4, 3);
        const uint32_t tableBlock = posIndex_;
        LoadStream(true, BlockAt(tableBlock, table), 12u * 132u);
        for (uint32_t entry = 0; entry < 12 && !failed_; ++entry) {
            for (uint32_t slot = 0; slot < 33 && !failed_; ++slot) {
                HandleAssetSlot(tableBlock, table + entry * 132 + slot * 4, &ZoneLoader::LoadFxEffectDef, 32);
            }
        }
    }
    PopStreamPos();
    if (!failed_) {
        ++result_.impactFxCount;
    }
}

// Load_PhysPreset: 44 bytes, strings at +0 and +28.
void ZoneLoader::LoadPhysPreset(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 44);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 28)));
    PopStreamPos();
    if (!failed_) {
        ++result_.physPresetCount;
    }
}

// Load_StringTable: 16 bytes {name, columnCount, rowCount, values} with no
// block push in the engine.
void ZoneLoader::LoadStringTable(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 16);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    uint32_t columnCount = 0;
    uint32_t rowCount = 0;
    std::memcpy(&columnCount, BlockAt(structBlock, structOffset + 4), 4);
    std::memcpy(&rowCount, BlockAt(structBlock, structOffset + 8), 4);

    if (ReadSlot(structBlock, structOffset + 12) != 0) {
        const uint32_t values = AllocForSlot(structBlock, structOffset + 12, 3);
        const uint32_t valueBlock = posIndex_;
        const uint32_t cellCount = columnCount * rowCount;
        LoadStream(true, BlockAt(valueBlock, values), cellCount * 4u);
        for (uint32_t i = 0; i < cellCount && !failed_; ++i) {
            LoadXString(reinterpret_cast<uint32_t*>(BlockAt(valueBlock, values + i * 4)));
        }
    }
    if (!failed_) {
        result_.stringTables.push_back(ResolveString(ReadSlot(structBlock, structOffset)));
    }
}

// Load_XAnimPartTrans: 4-byte header {size, smallTrans} then either a
// vec3 (size==0) or a 28-byte frame header, inline indices and frame data.
void ZoneLoader::LoadXAnimPartTrans(uint32_t structOffset, uint16_t numframes) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 4);
    const uint16_t size = ReadU16(structBlock, structOffset);
    const uint8_t smallTrans = *BlockAt(structBlock, structOffset + 2);

    if (size != 0) {
        const uint32_t frames = structOffset + 4;
        LoadStream(true, BlockAt(structBlock, frames), 28);
        const uint32_t indices = streamPos_;
        const uint32_t indexBytes = (numframes >= 0x100) ? (size + 1u) * 2u : (size + 1u);
        LoadStream(true, BlockAt(structBlock, indices), indexBytes);
        if (ReadSlot(structBlock, frames + 24) != 0 && !failed_) {
            const uint32_t count = size ? size + 1u : 0u;
            if (smallTrans != 0) {
                const uint32_t data = AllocForSlot(structBlock, frames + 24, 0);
                LoadStream(true, BlockAt(posIndex_, data), count * 3u);
            } else {
                const uint32_t data = AllocForSlot(structBlock, frames + 24, 3);
                LoadStream(true, BlockAt(posIndex_, data), count * 6u);
            }
        }
    } else {
        LoadStream(true, BlockAt(structBlock, structOffset + 4), 12);
    }
}

// Load_XAnimDeltaPartQuat: 4-byte header {size} then either a single XQuat2
// or a frame-pointer header, inline indices and quat frames.
void ZoneLoader::LoadXAnimDeltaPartQuat(uint32_t structOffset, uint16_t numframes) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 4);
    const uint16_t size = ReadU16(structBlock, structOffset);

    if (size != 0) {
        const uint32_t frames = structOffset + 4;
        LoadStream(true, BlockAt(structBlock, frames), 4);
        const uint32_t indices = streamPos_;
        const uint32_t indexBytes = (numframes >= 0x100) ? (size + 1u) * 2u : (size + 1u);
        LoadStream(true, BlockAt(structBlock, indices), indexBytes);
        if (ReadSlot(structBlock, frames) != 0 && !failed_) {
            const uint32_t count = size ? size + 1u : 0u;
            const uint32_t data = AllocForSlot(structBlock, frames, 3);
            LoadStream(true, BlockAt(posIndex_, data), count * 4u);
        }
    } else {
        LoadStream(true, BlockAt(structBlock, structOffset + 4), 4);
    }
}

// Load_XAnimParts: 88-byte struct with bone names, notifies, delta parts
// and six data arrays; indices width depends on numframes.
void ZoneLoader::LoadXAnimParts(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 88);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    const uint16_t numframes = ReadU16(structBlock, structOffset + 14);
    const uint8_t totalBones = *BlockAt(structBlock, structOffset + 27);
    const uint8_t notifyCount = *BlockAt(structBlock, structOffset + 28);

    if (ReadSlot(structBlock, structOffset + 48) != 0) {
        const uint32_t names = AllocForSlot(structBlock, structOffset + 48, 1);
        LoadStream(true, BlockAt(posIndex_, names), totalBones * 2u);
    }
    if (ReadSlot(structBlock, structOffset + 80) != 0) {
        const uint32_t notify = AllocForSlot(structBlock, structOffset + 80, 3);
        LoadStream(true, BlockAt(posIndex_, notify), notifyCount * 8u);
    }
    if (ReadSlot(structBlock, structOffset + 84) != 0) {
        const uint32_t delta = AllocForSlot(structBlock, structOffset + 84, 3);
        const uint32_t deltaBlock = posIndex_;
        LoadStream(true, BlockAt(deltaBlock, delta), 8);
        if (ReadSlot(deltaBlock, delta) != 0 && !failed_) {
            const uint32_t trans = AllocForSlot(deltaBlock, delta, 3);
            LoadXAnimPartTrans(trans, numframes);
        }
        if (ReadSlot(deltaBlock, delta + 4) != 0 && !failed_) {
            const uint32_t quat = AllocForSlot(deltaBlock, delta + 4, 3);
            LoadXAnimDeltaPartQuat(quat, numframes);
        }
    }

    struct DataArray {
        uint32_t slot;
        uint32_t countOffset;
        bool countIsU32;
        uint32_t elementSize;
        uint32_t alignMask;
    };
    const DataArray arrays[] = {
        {52, 4, false, 1, 0},   // dataByte
        {56, 6, false, 2, 1},   // dataShort
        {60, 8, false, 4, 3},   // dataInt
        {64, 32, true, 2, 1},   // randomDataShort
        {68, 10, false, 1, 0},  // randomDataByte
        {72, 12, false, 4, 3},  // randomDataInt
    };
    for (const DataArray& array : arrays) {
        if (failed_ || ReadSlot(structBlock, structOffset + array.slot) == 0) {
            continue;
        }
        uint32_t count = 0;
        if (array.countIsU32) {
            std::memcpy(&count, BlockAt(structBlock, structOffset + array.countOffset), 4);
        } else {
            count = ReadU16(structBlock, structOffset + array.countOffset);
        }
        const uint32_t data = AllocForSlot(structBlock, structOffset + array.slot, array.alignMask);
        LoadStream(true, BlockAt(posIndex_, data), count * array.elementSize);
    }

    // Load_XAnimIndices on the union at +76.
    uint32_t indexCount = 0;
    std::memcpy(&indexCount, BlockAt(structBlock, structOffset + 36), 4);
    if (ReadSlot(structBlock, structOffset + 76) != 0 && !failed_) {
        if (numframes >= 0x100) {
            const uint32_t data = AllocForSlot(structBlock, structOffset + 76, 1);
            LoadStream(true, BlockAt(posIndex_, data), indexCount * 2u);
        } else {
            const uint32_t data = AllocForSlot(structBlock, structOffset + 76, 0);
            LoadStream(true, BlockAt(posIndex_, data), indexCount);
        }
    }
    PopStreamPos();
    if (!failed_) {
        ++result_.xanimCount;
    }
}

// Load_XSurfaceCollisionTree: 40 bytes; nodes are 16-byte aligned, leafs
// 2-byte aligned.
void ZoneLoader::LoadXSurfaceCollisionTree(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 40);
    uint32_t nodeCount = 0;
    uint32_t leafCount = 0;
    std::memcpy(&nodeCount, BlockAt(structBlock, structOffset + 24), 4);
    std::memcpy(&leafCount, BlockAt(structBlock, structOffset + 32), 4);
    if (ReadSlot(structBlock, structOffset + 28) != 0) {
        const uint32_t nodes = AllocForSlot(structBlock, structOffset + 28, 15);
        LoadStream(true, BlockAt(posIndex_, nodes), nodeCount * 16u);
    }
    if (ReadSlot(structBlock, structOffset + 36) != 0) {
        const uint32_t leafs = AllocForSlot(structBlock, structOffset + 36, 1);
        LoadStream(true, BlockAt(posIndex_, leafs), leafCount * 2u);
    }
}

// Load_XSurface: 56-byte surface already bulk-read; blend info, packed
// vertices in block 7, rigid vert lists, and triangle indices in block 8.
void ZoneLoader::LoadXSurface(uint32_t block, uint32_t surfOffset) {
    const uint16_t vertCount = ReadU16(block, surfOffset + 2);
    const uint16_t triCount = ReadU16(block, surfOffset + 4);

    // vertInfo at +16: four int16 counts then the vertsBlend pointer.
    if (ReadSlot(block, surfOffset + 24) != 0) {
        uint32_t blendEntries = 0;
        const uint32_t weights[4] = {1, 3, 5, 7};
        for (int i = 0; i < 4; ++i) {
            blendEntries += weights[i] * ReadU16(block, surfOffset + 16 + i * 2);
        }
        uint32_t blendOffset = 0;
        if (HandleInlinePtr(block, surfOffset + 24, 1, blendOffset)) {
            LoadStream(true, BlockAt(posIndex_, blendOffset), blendEntries * 2u);
        }
    }

    PushStreamPos(7);
    {
        uint32_t verts = 0;
        if (ReadSlot(block, surfOffset + 28) != 0
            && HandleInlinePtr(block, surfOffset + 28, 15, verts)) {
            LoadStream(true, BlockAt(posIndex_, verts), vertCount * 32u);
        }
    }
    PopStreamPos();

    if (ReadSlot(block, surfOffset + 36) != 0) {
        uint32_t vertListCount = 0;
        std::memcpy(&vertListCount, BlockAt(block, surfOffset + 32), 4);
        uint32_t lists = 0;
        if (HandleInlinePtr(block, surfOffset + 36, 3, lists)) {
            const uint32_t listBlock = posIndex_;
            LoadStream(true, BlockAt(listBlock, lists), vertListCount * 12u);
            for (uint32_t i = 0; i < vertListCount && !failed_; ++i) {
                uint32_t tree = 0;
                if (ReadSlot(listBlock, lists + i * 12 + 8) != 0
                    && HandleInlinePtr(listBlock, lists + i * 12 + 8, 3, tree)) {
                    LoadXSurfaceCollisionTree(tree);
                }
            }
        }
    }

    PushStreamPos(8);
    {
        uint32_t indices = 0;
        if (ReadSlot(block, surfOffset + 12) != 0
            && HandleInlinePtr(block, surfOffset + 12, 15, indices)) {
            LoadStream(true, BlockAt(posIndex_, indices), triCount * 6u);
        }
    }
    PopStreamPos();
}

// Load_BrushWrapper: 80 bytes with sides, adjacency bytes and planes.
void ZoneLoader::LoadBrushWrapper(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 80);
    uint32_t numsides = 0;
    std::memcpy(&numsides, BlockAt(structBlock, structOffset + 28), 4);

    if (ReadSlot(structBlock, structOffset + 32) != 0) {
        const uint32_t sides = AllocForSlot(structBlock, structOffset + 32, 3);
        const uint32_t sideBlock = posIndex_;
        LoadStream(true, BlockAt(sideBlock, sides), numsides * 12u);
        for (uint32_t i = 0; i < numsides && !failed_; ++i) {
            uint32_t plane = 0;
            if (ReadSlot(sideBlock, sides + i * 12) != 0
                && HandleInlinePtr(sideBlock, sides + i * 12, 3, plane)) {
                LoadStream(true, BlockAt(posIndex_, plane), 20);
            }
        }
    }
    if (ReadSlot(structBlock, structOffset + 48) != 0) {
        uint32_t totalEdgeCount = 0;
        std::memcpy(&totalEdgeCount, BlockAt(structBlock, structOffset + 72), 4);
        const uint32_t edges = AllocForSlot(structBlock, structOffset + 48, 0);
        LoadStream(true, BlockAt(posIndex_, edges), totalEdgeCount);
    }
    {
        uint32_t planes = 0;
        if (ReadSlot(structBlock, structOffset + 76) != 0
            && HandleInlinePtr(structBlock, structOffset + 76, 3, planes)) {
            LoadStream(true, BlockAt(posIndex_, planes), numsides * 20u);
        }
    }
}

// Load_PhysGeomList: 44 bytes {count, geoms, mass}.
void ZoneLoader::LoadPhysGeomList(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 44);
    uint32_t count = 0;
    std::memcpy(&count, BlockAt(structBlock, structOffset), 4);
    if (ReadSlot(structBlock, structOffset + 4) != 0) {
        const uint32_t geoms = AllocForSlot(structBlock, structOffset + 4, 3);
        const uint32_t geomBlock = posIndex_;
        LoadStream(true, BlockAt(geomBlock, geoms), count * 68u);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            uint32_t brush = 0;
            if (ReadSlot(geomBlock, geoms + i * 68) != 0
                && HandleInlinePtr(geomBlock, geoms + i * 68, 3, brush)) {
                LoadBrushWrapper(brush);
            }
        }
    }
}

// Load_XModel: 220-byte model with bone tables, surfaces (blocks 7/8),
// per-surface materials, collision surfaces and physics.
void ZoneLoader::LoadXModel(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 220);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    const uint8_t numBones = *BlockAt(structBlock, structOffset + 4);
    const uint8_t numRootBones = *BlockAt(structBlock, structOffset + 5);
    const uint8_t numsurfs = *BlockAt(structBlock, structOffset + 6);
    const uint32_t animatedBones = numBones - numRootBones;

    struct BoneArray {
        uint32_t slot;
        uint32_t byteCount;
        uint32_t alignMask;
    };
    const BoneArray boneArrays[] = {
        {8, numBones * 2u, 1},          // boneNames (script strings)
        {12, animatedBones, 0},         // parentList
        {16, animatedBones * 8u, 1},    // quats (4 shorts each)
        {20, animatedBones * 16u, 3},   // trans (4 floats each)
        {24, numBones, 0},              // partClassification
        {28, numBones * 32u, 3},        // baseMat
    };
    for (const BoneArray& array : boneArrays) {
        if (failed_ || ReadSlot(structBlock, structOffset + array.slot) == 0) {
            continue;
        }
        uint32_t data = 0;
        if (HandleInlinePtr(structBlock, structOffset + array.slot, array.alignMask, data)) {
            LoadStream(true, BlockAt(posIndex_, data), array.byteCount);
        }
    }

    if (ReadSlot(structBlock, structOffset + 32) != 0 && !failed_) {
        const uint32_t surfs = AllocForSlot(structBlock, structOffset + 32, 3);
        const uint32_t surfBlock = posIndex_;
        LoadStream(true, BlockAt(surfBlock, surfs), numsurfs * 56u);
        for (uint8_t i = 0; i < numsurfs && !failed_; ++i) {
            LoadXSurface(surfBlock, surfs + i * 56u);
        }
    }
    if (ReadSlot(structBlock, structOffset + 36) != 0 && !failed_) {
        const uint32_t materials = AllocForSlot(structBlock, structOffset + 36, 3);
        const uint32_t materialBlock = posIndex_;
        LoadStream(true, BlockAt(materialBlock, materials), numsurfs * 4u);
        for (uint8_t i = 0; i < numsurfs && !failed_; ++i) {
            HandleAssetSlot(materialBlock, materials + i * 4, &ZoneLoader::LoadMaterial, 80);
        }
    }
    if (ReadSlot(structBlock, structOffset + 152) != 0 && !failed_) {
        uint32_t numCollSurfs = 0;
        std::memcpy(&numCollSurfs, BlockAt(structBlock, structOffset + 156), 4);
        const uint32_t collSurfs = AllocForSlot(structBlock, structOffset + 152, 3);
        const uint32_t collBlock = posIndex_;
        LoadStream(true, BlockAt(collBlock, collSurfs), numCollSurfs * 44u);
        for (uint32_t i = 0; i < numCollSurfs && !failed_; ++i) {
            if (ReadSlot(collBlock, collSurfs + i * 44) != 0) {
                uint32_t numCollTris = 0;
                std::memcpy(&numCollTris, BlockAt(collBlock, collSurfs + i * 44 + 4), 4);
                const uint32_t tris = AllocForSlot(collBlock, collSurfs + i * 44, 3);
                LoadStream(true, BlockAt(posIndex_, tris), numCollTris * 48u);
            }
        }
    }
    if (ReadSlot(structBlock, structOffset + 164) != 0 && !failed_) {
        const uint32_t boneInfo = AllocForSlot(structBlock, structOffset + 164, 3);
        LoadStream(true, BlockAt(posIndex_, boneInfo), numBones * 40u);
    }
    HandleAssetSlot(structBlock, structOffset + 212, &ZoneLoader::LoadPhysPreset, 44);
    {
        uint32_t geoms = 0;
        if (ReadSlot(structBlock, structOffset + 216) != 0
            && HandleInlinePtr(structBlock, structOffset + 216, 3, geoms)) {
            LoadPhysGeomList(geoms);
        }
    }
    PopStreamPos();

    if (!failed_) {
        result_.xmodels.push_back(ResolveString(ReadSlot(structBlock, structOffset)));
    }
}

// Load_XStringPtr / Load_snd_alias_list_name: pointer to a string pointer;
// -1 allocates a 4-byte slot which is then handled as an XString.
void ZoneLoader::LoadXStringPtrSlot(uint32_t block, uint32_t slotOffset) {
    if (ReadSlot(block, slotOffset) == kPtrInline) {
        const uint32_t inner = AllocForSlot(block, slotOffset, 3);
        const uint32_t innerBlock = posIndex_;
        LoadStream(true, BlockAt(innerBlock, inner), 4);
        LoadXString(reinterpret_cast<uint32_t*>(BlockAt(innerBlock, inner)));
    }
}

// Load_WeaponDef: 2168-byte struct; the pointer-field walk is generated
// from the engine's Load_WeaponDef and the WeaponDef layout.
void ZoneLoader::LoadWeaponDef(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 2168);
    PushStreamPos(4);

#define WD_STR(off) LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + (off))))
#define WD_STR_ARR(off, count) \
    for (uint32_t wdI = 0; wdI < (count) && !failed_; ++wdI) { WD_STR((off) + wdI * 4); }
#define WD_MODEL(off) HandleAssetSlot(structBlock, structOffset + (off), &ZoneLoader::LoadXModel, 220)
#define WD_MODEL_ARR(off, count) \
    for (uint32_t wdI = 0; wdI < (count) && !failed_; ++wdI) { WD_MODEL((off) + wdI * 4); }
#define WD_FX(off) HandleAssetSlot(structBlock, structOffset + (off), &ZoneLoader::LoadFxEffectDef, 32)
#define WD_SND(off) LoadXStringPtrSlot(structBlock, structOffset + (off))
#define WD_MAT(off) HandleAssetSlot(structBlock, structOffset + (off), &ZoneLoader::LoadMaterial, 80)
#define WD_SND_PTR_ARR(off, count) \
    do { \
        uint32_t wdArr = 0; \
        if (ReadSlot(structBlock, structOffset + (off)) != 0 \
            && HandleInlinePtr(structBlock, structOffset + (off), 3, wdArr)) { \
            const uint32_t wdBlock = posIndex_; \
            LoadStream(true, BlockAt(wdBlock, wdArr), (count) * 4u); \
            for (uint32_t wdI = 0; wdI < (count) && !failed_; ++wdI) { \
                LoadXStringPtrSlot(wdBlock, wdArr + wdI * 4); \
            } \
        } \
    } while (0)
#define WD_KNOTS(off, countOff) \
    do { \
        uint32_t wdKnots = 0; \
        if (ReadSlot(structBlock, structOffset + (off)) != 0 \
            && HandleInlinePtr(structBlock, structOffset + (off), 3, wdKnots)) { \
            uint32_t wdCount = 0; \
            std::memcpy(&wdCount, BlockAt(structBlock, structOffset + (countOff)), 4); \
            LoadStream(true, BlockAt(posIndex_, wdKnots), wdCount * 8u); \
        } \
    } while (0)

#include "kisak_weapondef_fields_android.inc"

#undef WD_STR
#undef WD_STR_ARR
#undef WD_MODEL
#undef WD_MODEL_ARR
#undef WD_FX
#undef WD_SND
#undef WD_MAT
#undef WD_SND_PTR_ARR
#undef WD_KNOTS

    PopStreamPos();
    if (!failed_) {
        result_.weapons.push_back(ResolveString(ReadSlot(structBlock, structOffset)));
    }
}

// Load_statement: {numEntries, entries}; each entry pointer allocates a
// 12-byte expressionEntry whose operand may carry an inline string.
void ZoneLoader::LoadStatement(uint32_t block, uint32_t stmtOffset) {
    uint32_t numEntries = 0;
    std::memcpy(&numEntries, BlockAt(block, stmtOffset), 4);
    if (ReadSlot(block, stmtOffset + 4) == 0) {
        return;
    }
    const uint32_t entries = AllocForSlot(block, stmtOffset + 4, 3);
    const uint32_t entryBlock = posIndex_;
    LoadStream(true, BlockAt(entryBlock, entries), numEntries * 4u);
    for (uint32_t i = 0; i < numEntries && !failed_; ++i) {
        if (ReadSlot(entryBlock, entries + i * 4) == 0) {
            continue;
        }
        const uint32_t entry = AllocForSlot(entryBlock, entries + i * 4, 3);
        const uint32_t innerBlock = posIndex_;
        LoadStream(true, BlockAt(innerBlock, entry), 12);
        uint32_t entryType = 0;
        std::memcpy(&entryType, BlockAt(innerBlock, entry), 4);
        if (entryType != 0) {
            uint32_t dataType = 0;
            std::memcpy(&dataType, BlockAt(innerBlock, entry + 4), 4);
            if (dataType == 2) { // VAL_STRING
                LoadXString(reinterpret_cast<uint32_t*>(BlockAt(innerBlock, entry + 8)));
            }
        }
    }
}

// Load_ItemKeyHandler chain: {key, action, next} linked list, all inline.
void ZoneLoader::LoadItemKeyHandlerChain(uint32_t block, uint32_t slotOffset) {
    uint32_t currentBlock = block;
    uint32_t currentSlot = slotOffset;
    while (!failed_ && ReadSlot(currentBlock, currentSlot) != 0) {
        const uint32_t handler = AllocForSlot(currentBlock, currentSlot, 3);
        const uint32_t handlerBlock = posIndex_;
        LoadStream(true, BlockAt(handlerBlock, handler), 12);
        LoadXString(reinterpret_cast<uint32_t*>(BlockAt(handlerBlock, handler + 4)));
        currentBlock = handlerBlock;
        currentSlot = handler + 8;
    }
}

// Load_windowDef_t: 156 bytes {name, rects, group, ..., background}.
void ZoneLoader::LoadWindowDef(uint32_t block, uint32_t windowOffset) {
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(block, windowOffset)));
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(block, windowOffset + 52)));
    HandleAssetSlot(block, windowOffset + 152, &ZoneLoader::LoadMaterial, 80);
}

// Load_itemDef_t: 372 bytes; strings, key handlers, focus sound,
// type-specific data and eight expression statements.
void ZoneLoader::LoadItemDef(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 372);
    LoadWindowDef(structBlock, structOffset);

    const uint32_t stringSlots[] = {224, 236, 240, 244, 248, 252, 256, 260, 264, 268, 272};
    for (uint32_t slot : stringSlots) {
        if (failed_) {
            return;
        }
        LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + slot)));
    }
    LoadItemKeyHandlerChain(structBlock, structOffset + 276);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 280)));
    HandleAssetSlot(structBlock, structOffset + 288, &ZoneLoader::LoadSndAliasList, 12);

    // Load_itemDefData_t dispatch on the item type at +180.
    uint32_t itemType = 0;
    std::memcpy(&itemType, BlockAt(structBlock, structOffset + 180), 4);
    const uint32_t typeSlot = structOffset + 300;
    switch (itemType) {
        case 6: // listBox
            if (ReadSlot(structBlock, typeSlot) != 0) {
                const uint32_t listBox = AllocForSlot(structBlock, typeSlot, 3);
                const uint32_t listBlock = posIndex_;
                LoadStream(true, BlockAt(listBlock, listBox), 340);
                LoadXString(reinterpret_cast<uint32_t*>(BlockAt(listBlock, listBox + 288)));
                HandleAssetSlot(listBlock, listBox + 336, &ZoneLoader::LoadMaterial, 80);
            }
            break;
        case 0: case 4: case 9: case 0xA: case 0xB: case 0xE:
        case 0x10: case 0x11: case 0x12: // editField
            if (ReadSlot(structBlock, typeSlot) != 0) {
                const uint32_t editField = AllocForSlot(structBlock, typeSlot, 3);
                LoadStream(true, BlockAt(posIndex_, editField), 32);
            }
            break;
        case 0xC: // multiDef
            if (ReadSlot(structBlock, typeSlot) != 0) {
                const uint32_t multi = AllocForSlot(structBlock, typeSlot, 3);
                const uint32_t multiBlock = posIndex_;
                LoadStream(true, BlockAt(multiBlock, multi), 392);
                for (uint32_t i = 0; i < 32 && !failed_; ++i) {
                    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(multiBlock, multi + i * 4)));
                }
                for (uint32_t i = 0; i < 32 && !failed_; ++i) {
                    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(multiBlock, multi + 128 + i * 4)));
                }
            }
            break;
        case 0xD: // enum dvar name
            LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, typeSlot)));
            break;
        default:
            break;
    }

    const uint32_t statementOffsets[] = {308, 316, 324, 332, 340, 348, 356, 364};
    for (uint32_t stmt : statementOffsets) {
        if (failed_) {
            return;
        }
        LoadStatement(structBlock, structOffset + stmt);
    }
}

// Load_menuDef_t: 284 bytes; window, event strings, key handlers,
// expressions and the item array.
void ZoneLoader::LoadMenuDef(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 284);
    PushStreamPos(4);
    LoadWindowDef(structBlock, structOffset);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 156)));
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 196)));
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 200)));
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 204)));
    LoadItemKeyHandlerChain(structBlock, structOffset + 208);
    LoadStatement(structBlock, structOffset + 212);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 220)));
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 224)));
    LoadStatement(structBlock, structOffset + 264);
    LoadStatement(structBlock, structOffset + 272);

    uint32_t itemCount = 0;
    std::memcpy(&itemCount, BlockAt(structBlock, structOffset + 164), 4);
    if (ReadSlot(structBlock, structOffset + 280) != 0 && !failed_) {
        const uint32_t items = AllocForSlot(structBlock, structOffset + 280, 3);
        const uint32_t itemBlock = posIndex_;
        LoadStream(true, BlockAt(itemBlock, items), itemCount * 4u);
        for (uint32_t i = 0; i < itemCount && !failed_; ++i) {
            if (ReadSlot(itemBlock, items + i * 4) != 0) {
                const uint32_t item = AllocForSlot(itemBlock, items + i * 4, 3);
                LoadItemDef(item);
            }
        }
    }
    PopStreamPos();

    if (!failed_) {
        result_.menus.push_back(ResolveString(ReadSlot(structBlock, structOffset)));
    }
}

// Load_MenuList: 12 bytes {name, menuCount, menus}; each menu slot is an
// asset-style pointer.
void ZoneLoader::LoadMenuList(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 12);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    uint32_t menuCount = 0;
    std::memcpy(&menuCount, BlockAt(structBlock, structOffset + 4), 4);
    if (ReadSlot(structBlock, structOffset + 8) != 0) {
        const uint32_t menus = AllocForSlot(structBlock, structOffset + 8, 3);
        const uint32_t menuBlock = posIndex_;
        LoadStream(true, BlockAt(menuBlock, menus), menuCount * 4u);
        for (uint32_t i = 0; i < menuCount && !failed_; ++i) {
            HandleAssetSlot(menuBlock, menus + i * 4, &ZoneLoader::LoadMenuDef, 284);
        }
    }
    PopStreamPos();
    if (!failed_) {
        ++result_.menuListCount;
    }
}

// Load_XAssetHeader dispatch: -1/-2 payloads deserialize inline, any other
// value is an alias to an already-loaded asset (DB_ConvertOffsetToAlias).
// Load_ComWorld: 16-byte struct, name + primary light array (defName strings).
void ZoneLoader::LoadComWorld(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 16);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    if (!failed_ && ReadSlot(structBlock, structOffset + 12) != 0) {
        const uint32_t count = ReadSlot(structBlock, structOffset + 8);
        const uint32_t lights = AllocForSlot(structBlock, structOffset + 12, 3);
        const uint32_t lightBlock = posIndex_;
        LoadStream(true, BlockAt(lightBlock, lights), 68 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            LoadXString(reinterpret_cast<uint32_t*>(BlockAt(lightBlock, lights + i * 68 + 64)));
        }
    }
    result_.worlds.push_back("comworld:" + ResolveString(ReadSlot(structBlock, structOffset)));
    PopStreamPos();
}

// Load_pathnode_tree_t recursion: axis<0 holds a leaf ushort list, otherwise
// two child pointers (-1 inline, offset otherwise).
void ZoneLoader::LoadPathNodeTreeNode(uint32_t block, uint32_t nodeOffset) {
    if (failed_) {
        return;
    }
    const int32_t axis = static_cast<int32_t>(ReadSlot(block, nodeOffset));
    if (axis < 0) {
        if (ReadSlot(block, nodeOffset + 12) != 0) {
            const uint32_t count = ReadSlot(block, nodeOffset + 8);
            const uint32_t data = AllocForSlot(block, nodeOffset + 12, 1);
            LoadStream(true, BlockAt(posIndex_, data), 2 * count);
        }
        return;
    }
    for (uint32_t child = 0; child < 2 && !failed_; ++child) {
        uint32_t childOffset = 0;
        if (HandleInlinePtr(block, nodeOffset + 8 + child * 4, 3, childOffset)) {
            const uint32_t childBlock = posIndex_;
            LoadStream(true, BlockAt(childBlock, childOffset), 16);
            LoadPathNodeTreeNode(childBlock, childOffset);
        }
    }
}

// Load_GameWorldSp: name + PathData (AI pathfinding graph).
void ZoneLoader::LoadGameWorldSp(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 44);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    const uint32_t path = structOffset + 4;
    const uint32_t nodeCount = ReadSlot(structBlock, path);
    if (!failed_ && ReadSlot(structBlock, path + 4) != 0) {
        // pathnode_t is 128 bytes; constant.totalLinkCount@62, Links@64.
        const uint32_t nodes = AllocForSlot(structBlock, path + 4, 3);
        const uint32_t nodeBlock = posIndex_;
        LoadStream(true, BlockAt(nodeBlock, nodes), 128 * nodeCount);
        for (uint32_t i = 0; i < nodeCount && !failed_; ++i) {
            const uint32_t node = nodes + i * 128;
            if (ReadSlot(nodeBlock, node + 64) != 0) {
                const uint32_t linkCount = ReadU16(nodeBlock, node + 62);
                const uint32_t links = AllocForSlot(nodeBlock, node + 64, 3);
                LoadStream(true, BlockAt(posIndex_, links), 12 * linkCount);
            }
        }
    }
    PushStreamPos(1);
    if (!failed_ && ReadSlot(structBlock, path + 8) != 0) {
        // pathbasenode_t array is runtime-only (block 1, 16-byte aligned).
        const uint32_t base = AllocForSlot(structBlock, path + 8, 15);
        LoadStream(true, BlockAt(posIndex_, base), 16 * nodeCount);
    }
    PopStreamPos();
    for (uint32_t slot : {path + 16u, path + 20u}) { // chainNodeForNode / nodeForChainNode
        if (!failed_ && ReadSlot(structBlock, slot) != 0) {
            const uint32_t data = AllocForSlot(structBlock, slot, 1);
            LoadStream(true, BlockAt(posIndex_, data), 2 * nodeCount);
        }
    }
    if (!failed_ && ReadSlot(structBlock, path + 28) != 0) {
        const uint32_t visBytes = ReadSlot(structBlock, path + 24);
        const uint32_t data = AllocForSlot(structBlock, path + 28, 0);
        LoadStream(true, BlockAt(posIndex_, data), visBytes);
    }
    if (!failed_ && ReadSlot(structBlock, path + 36) != 0) {
        const uint32_t treeCount = ReadSlot(structBlock, path + 32);
        const uint32_t trees = AllocForSlot(structBlock, path + 36, 3);
        const uint32_t treeBlock = posIndex_;
        LoadStream(true, BlockAt(treeBlock, trees), 16 * treeCount);
        for (uint32_t i = 0; i < treeCount && !failed_; ++i) {
            LoadPathNodeTreeNode(treeBlock, trees + i * 16);
        }
    }
    result_.worlds.push_back("gameworld_sp:" + ResolveString(ReadSlot(structBlock, structOffset)));
    PopStreamPos();
}

void ZoneLoader::LoadGameWorldMp(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 4);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    result_.worlds.push_back("gameworld_mp:" + ResolveString(ReadSlot(structBlock, structOffset)));
    PopStreamPos();
}

// Load_MapEnts: name + raw entity string.
void ZoneLoader::LoadMapEnts(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 12);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    if (!failed_ && ReadSlot(structBlock, structOffset + 4) != 0) {
        const uint32_t length = ReadSlot(structBlock, structOffset + 8);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 4, 0);
        LoadStream(true, BlockAt(posIndex_, data), length);
    }
    result_.worlds.push_back("map_ents:" + ResolveString(ReadSlot(structBlock, structOffset)));
    PopStreamPos();
}

// cplane_s pointer slot: -1 inline (count planes of 20 bytes), offset kept
// otherwise (DB_ConvertOffsetToPointer).
void ZoneLoader::LoadCPlaneSlot(uint32_t block, uint32_t slotOffset, uint32_t count) {
    uint32_t planes = 0;
    if (HandleInlinePtr(block, slotOffset, 3, planes)) {
        LoadStream(true, BlockAt(posIndex_, planes), 20 * count);
    }
}

// Load_cbrushside_t pointer handling on an already-streamed 12-byte side.
void ZoneLoader::LoadCBrushSideAt(uint32_t block, uint32_t sideOffset) {
    LoadCPlaneSlot(block, sideOffset, 1);
}

// Load_cbrush_t pointer handling on an already-streamed 80-byte brush.
void ZoneLoader::LoadCBrushAt(uint32_t block, uint32_t brushOffset) {
    uint32_t sides = 0;
    if (HandleInlinePtr(block, brushOffset + 32, 3, sides)) {
        const uint32_t sideBlock = posIndex_;
        LoadStream(true, BlockAt(sideBlock, sides), 12);
        LoadCBrushSideAt(sideBlock, sides);
    }
    uint32_t edge = 0;
    if (HandleInlinePtr(block, brushOffset + 48, 0, edge)) {
        LoadStream(true, BlockAt(posIndex_, edge), 1);
    }
}

// Load_XModelPiecesPtr: no block push, -1 inline only.
void ZoneLoader::LoadXModelPiecesSlot(uint32_t block, uint32_t slotOffset) {
    uint32_t pieces = 0;
    if (!HandleInlinePtr(block, slotOffset, 3, pieces)) {
        return;
    }
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, pieces), 12);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, pieces)));
    if (!failed_ && ReadSlot(structBlock, pieces + 8) != 0) {
        const uint32_t count = ReadSlot(structBlock, pieces + 4);
        const uint32_t array = AllocForSlot(structBlock, pieces + 8, 3);
        const uint32_t arrayBlock = posIndex_;
        LoadStream(true, BlockAt(arrayBlock, array), 16 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            HandleAssetSlot(arrayBlock, array + i * 16, &ZoneLoader::LoadXModel, 220);
        }
    }
}

// Load_clipMap_t: the 284-byte collision world.
void ZoneLoader::LoadClipMap(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 284);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));

    LoadCPlaneSlot(structBlock, structOffset + 12, ReadSlot(structBlock, structOffset + 8));

    if (!failed_ && ReadSlot(structBlock, structOffset + 20) != 0) { // staticModelList
        const uint32_t count = ReadSlot(structBlock, structOffset + 16);
        const uint32_t models = AllocForSlot(structBlock, structOffset + 20, 3);
        const uint32_t modelBlock = posIndex_;
        LoadStream(true, BlockAt(modelBlock, models), 80 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            HandleAssetSlot(modelBlock, models + i * 80 + 4, &ZoneLoader::LoadXModel, 220);
        }
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 28) != 0) { // materials
        const uint32_t count = ReadSlot(structBlock, structOffset + 24);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 28, 3);
        LoadStream(true, BlockAt(posIndex_, data), 72 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 36) != 0) { // brushsides
        const uint32_t count = ReadSlot(structBlock, structOffset + 32);
        const uint32_t sides = AllocForSlot(structBlock, structOffset + 36, 3);
        const uint32_t sideBlock = posIndex_;
        LoadStream(true, BlockAt(sideBlock, sides), 12 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            LoadCBrushSideAt(sideBlock, sides + i * 12);
        }
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 44) != 0) { // brushEdges
        const uint32_t count = ReadSlot(structBlock, structOffset + 40);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 44, 0);
        LoadStream(true, BlockAt(posIndex_, data), count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 52) != 0) { // nodes
        const uint32_t count = ReadSlot(structBlock, structOffset + 48);
        const uint32_t nodes = AllocForSlot(structBlock, structOffset + 52, 3);
        const uint32_t nodeBlock = posIndex_;
        LoadStream(true, BlockAt(nodeBlock, nodes), 8 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            LoadCPlaneSlot(nodeBlock, nodes + i * 8, 1);
        }
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 60) != 0) { // leafs
        const uint32_t count = ReadSlot(structBlock, structOffset + 56);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 60, 3);
        LoadStream(true, BlockAt(posIndex_, data), 44 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 76) != 0) { // leafbrushes
        const uint32_t count = ReadSlot(structBlock, structOffset + 72);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 76, 1);
        LoadStream(true, BlockAt(posIndex_, data), 2 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 68) != 0) { // leafbrushNodes
        const uint32_t count = ReadSlot(structBlock, structOffset + 64);
        const uint32_t nodes = AllocForSlot(structBlock, structOffset + 68, 3);
        const uint32_t nodeBlock = posIndex_;
        LoadStream(true, BlockAt(nodeBlock, nodes), 20 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            const uint32_t node = nodes + i * 20;
            const int16_t leafBrushCount = static_cast<int16_t>(ReadU16(nodeBlock, node + 2));
            if (leafBrushCount > 0) {
                uint32_t brushes = 0;
                if (HandleInlinePtr(nodeBlock, node + 8, 1, brushes)) {
                    LoadStream(true, BlockAt(posIndex_, brushes),
                               2 * static_cast<uint32_t>(leafBrushCount));
                }
            }
        }
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 84) != 0) { // leafsurfaces
        const uint32_t count = ReadSlot(structBlock, structOffset + 80);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 84, 3);
        LoadStream(true, BlockAt(posIndex_, data), 4 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 92) != 0) { // verts
        const uint32_t count = ReadSlot(structBlock, structOffset + 88);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 92, 3);
        LoadStream(true, BlockAt(posIndex_, data), 12 * count);
    }
    const uint32_t triCount = ReadSlot(structBlock, structOffset + 96);
    if (!failed_ && ReadSlot(structBlock, structOffset + 100) != 0) { // triIndices
        const uint32_t data = AllocForSlot(structBlock, structOffset + 100, 1);
        LoadStream(true, BlockAt(posIndex_, data), 2 * (3 * triCount));
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 104) != 0) { // triEdgeIsWalkable
        const uint32_t data = AllocForSlot(structBlock, structOffset + 104, 0);
        LoadStream(true, BlockAt(posIndex_, data), 4 * ((3 * triCount + 31) >> 5));
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 112) != 0) { // borders
        const uint32_t count = ReadSlot(structBlock, structOffset + 108);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 112, 3);
        LoadStream(true, BlockAt(posIndex_, data), 28 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 120) != 0) { // partitions
        const uint32_t count = ReadSlot(structBlock, structOffset + 116);
        const uint32_t parts = AllocForSlot(structBlock, structOffset + 120, 3);
        const uint32_t partBlock = posIndex_;
        LoadStream(true, BlockAt(partBlock, parts), 12 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            uint32_t border = 0;
            if (HandleInlinePtr(partBlock, parts + i * 12 + 8, 3, border)) {
                LoadStream(true, BlockAt(posIndex_, border), 28);
            }
        }
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 128) != 0) { // aabbTrees
        const uint32_t count = ReadSlot(structBlock, structOffset + 124);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 128, 3);
        LoadStream(true, BlockAt(posIndex_, data), 32 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 136) != 0) { // cmodels
        const uint32_t count = ReadSlot(structBlock, structOffset + 132);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 136, 3);
        LoadStream(true, BlockAt(posIndex_, data), 72 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 144) != 0) { // brushes
        const uint32_t count = ReadU16(structBlock, structOffset + 140);
        const uint32_t brushes = AllocForSlot(structBlock, structOffset + 144, 15);
        const uint32_t brushBlock = posIndex_;
        LoadStream(true, BlockAt(brushBlock, brushes), 80 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            LoadCBrushAt(brushBlock, brushes + i * 80);
        }
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 156) != 0) { // visibility
        const uint32_t bytes = ReadSlot(structBlock, structOffset + 148)
            * ReadSlot(structBlock, structOffset + 152);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 156, 0);
        LoadStream(true, BlockAt(posIndex_, data), bytes);
    }
    HandleAssetSlot(structBlock, structOffset + 164, &ZoneLoader::LoadMapEnts, 12);
    uint32_t boxBrush = 0;
    if (!failed_ && HandleInlinePtr(structBlock, structOffset + 168, 15, boxBrush)) {
        const uint32_t brushBlock = posIndex_;
        LoadStream(true, BlockAt(brushBlock, boxBrush), 80);
        LoadCBrushAt(brushBlock, boxBrush);
    }
    const uint32_t dynEntCount[2] = {
        ReadU16(structBlock, structOffset + 244),
        ReadU16(structBlock, structOffset + 246),
    };
    for (uint32_t list = 0; list < 2; ++list) { // dynEntDefList
        const uint32_t slot = structOffset + 248 + list * 4;
        if (failed_ || ReadSlot(structBlock, slot) == 0) {
            continue;
        }
        const uint32_t defs = AllocForSlot(structBlock, slot, 3);
        const uint32_t defBlock = posIndex_;
        LoadStream(true, BlockAt(defBlock, defs), 96 * dynEntCount[list]);
        for (uint32_t i = 0; i < dynEntCount[list] && !failed_; ++i) {
            const uint32_t def = defs + i * 96;
            HandleAssetSlot(defBlock, def + 32, &ZoneLoader::LoadXModel, 220);
            HandleAssetSlot(defBlock, def + 40, &ZoneLoader::LoadFxEffectDef, 32);
            LoadXModelPiecesSlot(defBlock, def + 44);
            HandleAssetSlot(defBlock, def + 48, &ZoneLoader::LoadPhysPreset, 44);
        }
    }
    struct DynList {
        uint32_t slot;
        uint32_t stride;
    };
    const DynList runtimeLists[] = {
        {256, 32}, {260, 32}, // dynEntPoseList
        {264, 12}, {268, 12}, // dynEntClientList
        {272, 20}, {276, 20}, // dynEntCollList
    };
    for (uint32_t list = 0; list < std::size(runtimeLists); ++list) {
        PushStreamPos(1);
        const uint32_t slot = structOffset + runtimeLists[list].slot;
        if (!failed_ && ReadSlot(structBlock, slot) != 0) {
            const uint32_t data = AllocForSlot(structBlock, slot, 3);
            LoadStream(true, BlockAt(posIndex_, data),
                       runtimeLists[list].stride * dynEntCount[list & 1]);
        }
        PopStreamPos();
    }
    result_.worlds.push_back("clipmap:" + ResolveString(ReadSlot(structBlock, structOffset)));
    PopStreamPos();
}

// Load_GfxAabbTree pointer handling on an already-streamed 44-byte tree.
void ZoneLoader::LoadGfxAabbTreeAt(uint32_t block, uint32_t treeOffset) {
    uint32_t indexes = 0;
    if (HandleInlinePtr(block, treeOffset + 36, 1, indexes)) {
        LoadStream(true, BlockAt(posIndex_, indexes), 2u * ReadU16(block, treeOffset + 34));
    }
}

// Load_GfxPortal pointer handling on an already-streamed 68-byte portal.
void ZoneLoader::LoadGfxPortalAt(uint32_t block, uint32_t portalOffset) {
    uint32_t cell = 0;
    if (HandleInlinePtr(block, portalOffset + 32, 3, cell)) {
        const uint32_t cellBlock = posIndex_;
        LoadStream(true, BlockAt(cellBlock, cell), 56);
        LoadGfxCellAt(cellBlock, cell);
    }
    if (!failed_ && ReadSlot(block, portalOffset + 36) != 0) {
        const uint32_t vertexCount = *BlockAt(block, portalOffset + 40);
        const uint32_t verts = AllocForSlot(block, portalOffset + 36, 3);
        LoadStream(true, BlockAt(posIndex_, verts), 12 * vertexCount);
    }
}

// Load_GfxCell pointer handling on an already-streamed 56-byte cell.
void ZoneLoader::LoadGfxCellAt(uint32_t block, uint32_t cellOffset) {
    if (!failed_ && ReadSlot(block, cellOffset + 28) != 0) { // aabbTree
        const uint32_t count = ReadSlot(block, cellOffset + 24);
        const uint32_t trees = AllocForSlot(block, cellOffset + 28, 3);
        const uint32_t treeBlock = posIndex_;
        LoadStream(true, BlockAt(treeBlock, trees), 44 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            LoadGfxAabbTreeAt(treeBlock, trees + i * 44);
        }
    }
    if (!failed_ && ReadSlot(block, cellOffset + 36) != 0) { // portals
        const uint32_t count = ReadSlot(block, cellOffset + 32);
        const uint32_t portals = AllocForSlot(block, cellOffset + 36, 3);
        const uint32_t portalBlock = posIndex_;
        LoadStream(true, BlockAt(portalBlock, portals), 68 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            LoadGfxPortalAt(portalBlock, portals + i * 68);
        }
    }
    if (!failed_ && ReadSlot(block, cellOffset + 44) != 0) { // cullGroups
        const uint32_t count = ReadSlot(block, cellOffset + 40);
        const uint32_t data = AllocForSlot(block, cellOffset + 44, 3);
        LoadStream(true, BlockAt(posIndex_, data), 4 * count);
    }
    if (!failed_ && ReadSlot(block, cellOffset + 52) != 0) { // reflectionProbes
        const uint32_t count = *BlockAt(block, cellOffset + 48);
        const uint32_t data = AllocForSlot(block, cellOffset + 52, 0);
        LoadStream(true, BlockAt(posIndex_, data), count);
    }
}

// Load_GfxWorld: the 732-byte render world (vertex/index buffers, cells,
// lightmaps, static models, dpvs data).
void ZoneLoader::LoadGfxWorld(uint32_t structOffset) {
    const uint32_t structBlock = posIndex_;
    LoadStream(true, BlockAt(structBlock, structOffset), 732);
    PushStreamPos(4);
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset)));
    LoadXString(reinterpret_cast<uint32_t*>(BlockAt(structBlock, structOffset + 4)));

    if (!failed_ && ReadSlot(structBlock, structOffset + 20) != 0) { // indices
        const uint32_t count = ReadSlot(structBlock, structOffset + 16);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 20, 1);
        LoadStream(true, BlockAt(posIndex_, data), 2 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 36) != 0) { // skyStartSurfs
        const uint32_t count = ReadSlot(structBlock, structOffset + 32);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 36, 3);
        LoadStream(true, BlockAt(posIndex_, data), 4 * count);
    }
    HandleAssetSlot(structBlock, structOffset + 40, &ZoneLoader::LoadGfxImage, 36); // skyImage
    uint32_t sunLight = 0;
    if (!failed_ && HandleInlinePtr(structBlock, structOffset + 200, 3, sunLight)) {
        const uint32_t lightBlock = posIndex_;
        LoadStream(true, BlockAt(lightBlock, sunLight), 64);
        HandleAssetSlot(lightBlock, sunLight + 60, &ZoneLoader::LoadGfxLightDef, 16);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 232) != 0) { // reflectionProbes
        const uint32_t count = ReadSlot(structBlock, structOffset + 228);
        const uint32_t probes = AllocForSlot(structBlock, structOffset + 232, 3);
        const uint32_t probeBlock = posIndex_;
        LoadStream(true, BlockAt(probeBlock, probes), 16 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            HandleAssetSlot(probeBlock, probes + i * 16 + 12, &ZoneLoader::LoadGfxImage, 36);
        }
    }
    PushStreamPos(1);
    if (!failed_ && ReadSlot(structBlock, structOffset + 236) != 0) { // reflectionProbeTextures
        const uint32_t count = ReadSlot(structBlock, structOffset + 228);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 236, 3);
        LoadStream(true, BlockAt(posIndex_, data), 4 * count);
    }
    PopStreamPos();

    // dpvsPlanes @240 (16 bytes): planes, nodes, sceneEntCellBits.
    const uint32_t cellCount = ReadSlot(structBlock, structOffset + 240);
    LoadCPlaneSlot(structBlock, structOffset + 244, ReadSlot(structBlock, structOffset + 8));
    if (!failed_ && ReadSlot(structBlock, structOffset + 248) != 0) {
        const uint32_t count = ReadSlot(structBlock, structOffset + 12);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 248, 1);
        LoadStream(true, BlockAt(posIndex_, data), 2 * count);
    }
    PushStreamPos(1);
    if (!failed_ && ReadSlot(structBlock, structOffset + 252) != 0) {
        const uint32_t data = AllocForSlot(structBlock, structOffset + 252, 3);
        LoadStream(true, BlockAt(posIndex_, data), 4 * (cellCount << 8));
    }
    PopStreamPos();

    if (!failed_ && ReadSlot(structBlock, structOffset + 260) != 0) { // cells
        const uint32_t cells = AllocForSlot(structBlock, structOffset + 260, 3);
        const uint32_t cellBlock = posIndex_;
        LoadStream(true, BlockAt(cellBlock, cells), 56 * cellCount);
        for (uint32_t i = 0; i < cellCount && !failed_; ++i) {
            LoadGfxCellAt(cellBlock, cells + i * 56);
        }
    }
    const uint32_t lightmapCount = ReadSlot(structBlock, structOffset + 264);
    if (!failed_ && ReadSlot(structBlock, structOffset + 268) != 0) { // lightmaps
        const uint32_t maps = AllocForSlot(structBlock, structOffset + 268, 3);
        const uint32_t mapBlock = posIndex_;
        LoadStream(true, BlockAt(mapBlock, maps), 8 * lightmapCount);
        for (uint32_t i = 0; i < lightmapCount && !failed_; ++i) {
            HandleAssetSlot(mapBlock, maps + i * 8, &ZoneLoader::LoadGfxImage, 36);
            HandleAssetSlot(mapBlock, maps + i * 8 + 4, &ZoneLoader::LoadGfxImage, 36);
        }
    }
    // lightGrid @272 (56 bytes), loaded in place.
    {
        const uint32_t grid = structOffset + 272;
        if (!failed_ && ReadSlot(structBlock, grid + 28) != 0) { // rowDataStart
            const uint32_t rowAxis = ReadSlot(structBlock, grid + 20);
            const uint32_t lo = ReadU16(structBlock, grid + 8 + rowAxis * 2);
            const uint32_t hi = ReadU16(structBlock, grid + 14 + rowAxis * 2);
            const uint32_t data = AllocForSlot(structBlock, grid + 28, 1);
            LoadStream(true, BlockAt(posIndex_, data), 2 * (hi - lo + 1));
        }
        if (!failed_ && ReadSlot(structBlock, grid + 36) != 0) { // rawRowData
            const uint32_t bytes = ReadSlot(structBlock, grid + 32);
            const uint32_t data = AllocForSlot(structBlock, grid + 36, 0);
            LoadStream(true, BlockAt(posIndex_, data), bytes);
        }
        if (!failed_ && ReadSlot(structBlock, grid + 44) != 0) { // entries
            const uint32_t count = ReadSlot(structBlock, grid + 40);
            const uint32_t data = AllocForSlot(structBlock, grid + 44, 3);
            LoadStream(true, BlockAt(posIndex_, data), 4 * count);
        }
        if (!failed_ && ReadSlot(structBlock, grid + 52) != 0) { // colors
            const uint32_t count = ReadSlot(structBlock, grid + 48);
            const uint32_t data = AllocForSlot(structBlock, grid + 52, 3);
            LoadStream(true, BlockAt(posIndex_, data), 168 * count);
        }
    }
    for (uint32_t slot : {structOffset + 328u, structOffset + 332u}) { // lightmap textures
        PushStreamPos(1);
        if (!failed_ && ReadSlot(structBlock, slot) != 0) {
            const uint32_t data = AllocForSlot(structBlock, slot, 3);
            LoadStream(true, BlockAt(posIndex_, data), 4 * lightmapCount);
        }
        PopStreamPos();
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 340) != 0) { // models
        const uint32_t count = ReadSlot(structBlock, structOffset + 336);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 340, 3);
        LoadStream(true, BlockAt(posIndex_, data), 56 * count);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 376) != 0) { // materialMemory
        const uint32_t count = ReadSlot(structBlock, structOffset + 372);
        const uint32_t memory = AllocForSlot(structBlock, structOffset + 376, 3);
        const uint32_t memoryBlock = posIndex_;
        LoadStream(true, BlockAt(memoryBlock, memory), 8 * count);
        for (uint32_t i = 0; i < count && !failed_; ++i) {
            HandleAssetSlot(memoryBlock, memory + i * 8, &ZoneLoader::LoadMaterial, 80);
        }
    }
    const uint32_t vertexCount = ReadSlot(structBlock, structOffset + 48);
    if (!failed_ && ReadSlot(structBlock, structOffset + 52) != 0) { // vd.vertices
        const uint32_t data = AllocForSlot(structBlock, structOffset + 52, 3);
        LoadStream(true, BlockAt(posIndex_, data), 44 * vertexCount);
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 64) != 0) { // vld.data
        const uint32_t bytes = ReadSlot(structBlock, structOffset + 60);
        const uint32_t data = AllocForSlot(structBlock, structOffset + 64, 0);
        LoadStream(true, BlockAt(posIndex_, data), bytes);
    }
    // sun @380: spriteMaterial/flareMaterial handles.
    HandleAssetSlot(structBlock, structOffset + 384, &ZoneLoader::LoadMaterial, 80);
    HandleAssetSlot(structBlock, structOffset + 388, &ZoneLoader::LoadMaterial, 80);
    HandleAssetSlot(structBlock, structOffset + 540, &ZoneLoader::LoadGfxImage, 36); // outdoorImage

    const uint32_t sunPrimaryLightIndex = ReadSlot(structBlock, structOffset + 216);
    const uint32_t primaryLightCount = ReadSlot(structBlock, structOffset + 220);
    const uint32_t nonSunLightCount = primaryLightCount - (sunPrimaryLightIndex + 1);
    const uint32_t dynEntClientCount[2] = {
        ReadSlot(structBlock, structOffset + 692),
        ReadSlot(structBlock, structOffset + 696),
    };
    struct RuntimeArray {
        uint32_t slot;
        uint32_t bytes;
        uint32_t alignMask;
    };
    const RuntimeArray runtimeArrays[] = {
        {544, 4 * (cellCount * ((cellCount + 31) >> 5)), 3}, // cellCasterBits
        {548, 6 * dynEntClientCount[0], 3},                  // sceneDynModel
        {552, 4 * dynEntClientCount[1], 3},                  // sceneDynBrush
        {556, 4 * (nonSunLightCount << 12), 3},              // primaryLightEntityShadowVis
        {560, 4 * (dynEntClientCount[0] * nonSunLightCount), 3},
        {564, 4 * (dynEntClientCount[1] * nonSunLightCount), 3},
        {568, dynEntClientCount[0], 0},                      // nonSunPrimaryLightForModelDynEnt
    };
    for (const RuntimeArray& array : runtimeArrays) {
        PushStreamPos(1);
        if (!failed_ && ReadSlot(structBlock, structOffset + array.slot) != 0) {
            const uint32_t data = AllocForSlot(structBlock, structOffset + array.slot, array.alignMask);
            LoadStream(true, BlockAt(posIndex_, data), array.bytes);
        }
        PopStreamPos();
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 572) != 0) { // shadowGeom
        const uint32_t geoms = AllocForSlot(structBlock, structOffset + 572, 3);
        const uint32_t geomBlock = posIndex_;
        LoadStream(true, BlockAt(geomBlock, geoms), 12 * primaryLightCount);
        for (uint32_t i = 0; i < primaryLightCount && !failed_; ++i) {
            const uint32_t geom = geoms + i * 12;
            if (ReadSlot(geomBlock, geom + 4) != 0) {
                const uint32_t data = AllocForSlot(geomBlock, geom + 4, 1);
                LoadStream(true, BlockAt(posIndex_, data), 2u * ReadU16(geomBlock, geom));
            }
            if (ReadSlot(geomBlock, geom + 8) != 0) {
                const uint32_t data = AllocForSlot(geomBlock, geom + 8, 1);
                LoadStream(true, BlockAt(posIndex_, data), 2u * ReadU16(geomBlock, geom + 2));
            }
        }
    }
    if (!failed_ && ReadSlot(structBlock, structOffset + 576) != 0) { // lightRegion
        const uint32_t regions = AllocForSlot(structBlock, structOffset + 576, 3);
        const uint32_t regionBlock = posIndex_;
        LoadStream(true, BlockAt(regionBlock, regions), 8 * primaryLightCount);
        for (uint32_t i = 0; i < primaryLightCount && !failed_; ++i) {
            const uint32_t region = regions + i * 8;
            if (ReadSlot(regionBlock, region + 4) == 0) {
                continue;
            }
            const uint32_t hullCount = ReadSlot(regionBlock, region);
            const uint32_t hulls = AllocForSlot(regionBlock, region + 4, 3);
            const uint32_t hullBlock = posIndex_;
            LoadStream(true, BlockAt(hullBlock, hulls), 80 * hullCount);
            for (uint32_t h = 0; h < hullCount && !failed_; ++h) {
                const uint32_t hull = hulls + h * 80;
                if (ReadSlot(hullBlock, hull + 76) != 0) {
                    const uint32_t axisCount = ReadSlot(hullBlock, hull + 72);
                    const uint32_t axis = AllocForSlot(hullBlock, hull + 76, 3);
                    LoadStream(true, BlockAt(posIndex_, axis), 20 * axisCount);
                }
            }
        }
    }

    // dpvs @580 (104 bytes).
    {
        const uint32_t dpvs = structOffset + 580;
        const uint32_t smodelCount = ReadSlot(structBlock, dpvs);
        const uint32_t staticSurfaceCount = ReadSlot(structBlock, dpvs + 4);
        const uint32_t staticSurfaceCountNoDecal = ReadSlot(structBlock, dpvs + 8);
        const uint32_t smodelVisDataCount = ReadSlot(structBlock, dpvs + 36);
        const uint32_t surfaceVisDataCount = ReadSlot(structBlock, dpvs + 40);
        const RuntimeArray visArrays[] = {
            {44, smodelCount, 0}, {48, smodelCount, 0}, {52, smodelCount, 0},
            {56, staticSurfaceCount, 0}, {60, staticSurfaceCount, 0}, {64, staticSurfaceCount, 0},
            {68, 4 * (2 * smodelVisDataCount), 127}, // lodData
        };
        for (const RuntimeArray& array : visArrays) {
            PushStreamPos(1);
            if (!failed_ && ReadSlot(structBlock, dpvs + array.slot) != 0) {
                const uint32_t data = AllocForSlot(structBlock, dpvs + array.slot, array.alignMask);
                LoadStream(true, BlockAt(posIndex_, data), array.bytes);
            }
            PopStreamPos();
        }
        if (!failed_ && ReadSlot(structBlock, dpvs + 72) != 0) { // sortedSurfIndex
            const uint32_t data = AllocForSlot(structBlock, dpvs + 72, 1);
            LoadStream(true, BlockAt(posIndex_, data),
                       2 * (staticSurfaceCountNoDecal + staticSurfaceCount));
        }
        if (!failed_ && ReadSlot(structBlock, dpvs + 76) != 0) { // smodelInsts
            const uint32_t data = AllocForSlot(structBlock, dpvs + 76, 3);
            LoadStream(true, BlockAt(posIndex_, data), 28 * smodelCount);
        }
        if (!failed_ && ReadSlot(structBlock, dpvs + 80) != 0) { // surfaces
            const uint32_t surfaceCount = ReadSlot(structBlock, structOffset + 24);
            const uint32_t surfaces = AllocForSlot(structBlock, dpvs + 80, 3);
            const uint32_t surfaceBlock = posIndex_;
            LoadStream(true, BlockAt(surfaceBlock, surfaces), 48 * surfaceCount);
            for (uint32_t i = 0; i < surfaceCount && !failed_; ++i) {
                HandleAssetSlot(surfaceBlock, surfaces + i * 48 + 16, &ZoneLoader::LoadMaterial, 80);
            }
        }
        if (!failed_ && ReadSlot(structBlock, dpvs + 84) != 0) { // cullGroups
            const uint32_t count = ReadSlot(structBlock, structOffset + 224);
            const uint32_t data = AllocForSlot(structBlock, dpvs + 84, 3);
            LoadStream(true, BlockAt(posIndex_, data), 32 * count);
        }
        if (!failed_ && ReadSlot(structBlock, dpvs + 88) != 0) { // smodelDrawInsts
            const uint32_t insts = AllocForSlot(structBlock, dpvs + 88, 3);
            const uint32_t instBlock = posIndex_;
            LoadStream(true, BlockAt(instBlock, insts), 76 * smodelCount);
            for (uint32_t i = 0; i < smodelCount && !failed_; ++i) {
                HandleAssetSlot(instBlock, insts + i * 76 + 56, &ZoneLoader::LoadXModel, 220);
            }
        }
        const RuntimeArray tailArrays[] = {
            {92, 8 * staticSurfaceCount, 3},      // surfaceMaterials
            {96, 4 * surfaceVisDataCount, 127},   // surfaceCastsSunShadow
        };
        for (const RuntimeArray& array : tailArrays) {
            PushStreamPos(1);
            if (!failed_ && ReadSlot(structBlock, dpvs + array.slot) != 0) {
                const uint32_t data = AllocForSlot(structBlock, dpvs + array.slot, array.alignMask);
                LoadStream(true, BlockAt(posIndex_, data), array.bytes);
            }
            PopStreamPos();
        }
    }

    // dpvsDyn @684 (48 bytes).
    {
        const uint32_t dpvsDyn = structOffset + 684;
        const uint32_t wordCount[2] = {
            ReadSlot(structBlock, dpvsDyn),
            ReadSlot(structBlock, dpvsDyn + 4),
        };
        for (uint32_t list = 0; list < 2; ++list) { // dynEntCellBits
            PushStreamPos(1);
            const uint32_t slot = dpvsDyn + 16 + list * 4;
            if (!failed_ && ReadSlot(structBlock, slot) != 0) {
                const uint32_t data = AllocForSlot(structBlock, slot, 3);
                LoadStream(true, BlockAt(posIndex_, data), 4 * (cellCount * wordCount[list]));
            }
            PopStreamPos();
        }
        // dynEntVisData in engine order [0][0], [1][0], [0][1], [1][1], [0][2], [1][2].
        for (uint32_t pass = 0; pass < 3; ++pass) {
            for (uint32_t list = 0; list < 2; ++list) {
                PushStreamPos(1);
                const uint32_t slot = dpvsDyn + 24 + list * 12 + pass * 4;
                if (!failed_ && ReadSlot(structBlock, slot) != 0) {
                    const uint32_t data = AllocForSlot(structBlock, slot, 15);
                    LoadStream(true, BlockAt(posIndex_, data), 32 * wordCount[list]);
                }
                PopStreamPos();
            }
        }
    }
    result_.worlds.push_back("gfxworld:" + ResolveString(ReadSlot(structBlock, structOffset)));
    PopStreamPos();
}

bool ZoneLoader::LoadAssetHeader(uint32_t type, uint32_t* headerSlot) {
    // Types absent from the engine's Load_XAssetHeader switch are skipped
    // entirely on PC: no stream data, no pointer handling (e.g. the Xbox-era
    // snddriverglobals whose slot holds a raw non-pointer value).
    switch (type) {
        case 0x00: // xmodelpieces
        case 0x12: // uimap
        case 0x18: // snddriverglobals
        case 0x1b: // aitype
        case 0x1c: // mptype
        case 0x1d: // character
        case 0x1e: // xmodelalias
            return true;
        case 0x20: { // stringtable: no block push, no -2 handling
            const uint32_t value = *headerSlot;
            if (value == kPtrInline) {
                const uint32_t structOffset = AllocStreamPos(3);
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadStringTable(structOffset);
                ++result_.assetsLoaded;
            } else if (value != 0) {
                ++result_.assetsAliased;
            }
            return true;
        }
        default:
            break;
    }

    PushStreamPos(0);
    bool handled = true;
    const uint32_t value = *headerSlot;

    if (value == 0) {
        // Null asset header: nothing follows.
    } else if (value == kPtrInline || value == kPtrInsert) {
        uint32_t insertRef = 0;
        if (value == kPtrInsert) {
            // DB_InsertPointer: reserve a pointer slot in block 4.
            PushStreamPos(4);
            const uint32_t slotOffset = AllocStreamPos(3);
            IncStreamPos(4);
            insertRef = EncodeRef(4, slotOffset);
            PopStreamPos();
        }

        // AllocLoad_*: the asset struct itself lives in the current block
        // (temp block 0, rewound when this scope pops).
        const uint32_t structOffset = AllocStreamPos(3);
        switch (type) {
            case kAssetTypeLocalize:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadLocalizeEntry(structOffset);
                break;
            case kAssetTypeRawFile:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadRawFile(structOffset);
                break;
            case kAssetTypeTechniqueSet:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadMaterialTechniqueSet(structOffset);
                break;
            case kAssetTypeMaterial:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadMaterial(structOffset);
                break;
            case kAssetTypeImage:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadGfxImage(structOffset);
                break;
            case kAssetTypeSound:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadSndAliasList(structOffset);
                break;
            case kAssetTypeSoundCurve:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadSndCurve(structOffset);
                break;
            case kAssetTypeLoadedSound:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadLoadedSound(structOffset);
                break;
            case kAssetTypeFont:
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadFont(structOffset);
                break;
            case 0x01: // physpreset
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadPhysPreset(structOffset);
                break;
            case 0x02: // xanimparts
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadXAnimParts(structOffset);
                break;
            case 0x03: // xmodel
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadXModel(structOffset);
                break;
            case 0x0a: // clipmap
            case 0x0b: // clipmap_pvs
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadClipMap(structOffset);
                break;
            case 0x0c: // comworld
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadComWorld(structOffset);
                break;
            case 0x0d: // gameworld_sp
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadGameWorldSp(structOffset);
                break;
            case 0x0e: // gameworld_mp
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadGameWorldMp(structOffset);
                break;
            case 0x0f: // map_ents
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadMapEnts(structOffset);
                break;
            case 0x10: // gfxworld
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadGfxWorld(structOffset);
                break;
            case 0x11: // lightdef
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadGfxLightDef(structOffset);
                break;
            case 0x14: // menulist
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadMenuList(structOffset);
                break;
            case 0x15: // menu
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadMenuDef(structOffset);
                break;
            case 0x17: // weapon
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadWeaponDef(structOffset);
                break;
            case 0x19: // fx
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadFxEffectDef(structOffset);
                break;
            case 0x1a: // impactfx
                *headerSlot = EncodeRef(posIndex_, structOffset);
                LoadFxImpactTable(structOffset);
                break;
            default:
                handled = false;
                break;
        }

        if (handled && !failed_) {
            // DB_AddXAsset: persist the temp-block struct in the pool.
            static const std::map<uint32_t, uint32_t> kAssetStructSizes = {
                {kAssetTypeLocalize, 8}, {kAssetTypeRawFile, 12},
                {kAssetTypeTechniqueSet, 148}, {kAssetTypeMaterial, 80},
                {kAssetTypeImage, 36}, {kAssetTypeSound, 12},
                {kAssetTypeSoundCurve, 72}, {kAssetTypeLoadedSound, 44},
                {kAssetTypeFont, 24}, {0x01, 44}, {0x02, 88}, {0x03, 220},
                {0x0a, 284}, {0x0b, 284}, {0x0c, 16}, {0x0d, 44}, {0x0e, 4},
                {0x0f, 12}, {0x10, 732},
                {0x11, 16}, {0x14, 12}, {0x15, 284}, {0x17, 2168},
                {0x19, 32}, {0x1a, 8},
            };
            *headerSlot = PoolAsset(posIndex_, structOffset, kAssetStructSizes.at(type));
        }

        if (handled && insertRef != 0) {
            uint32_t index = 0;
            uint32_t offset = 0;
            if (DecodeRef(insertRef, index, offset)) {
                std::memcpy(BlockAt(index, offset), headerSlot, 4);
            }
        }
        if (handled) {
            ++result_.assetsLoaded;
        }
    } else {
        // Alias to an already-loaded asset: copy the target slot's value.
        uint32_t index = 0;
        uint32_t offset = 0;
        if (DecodeRef(value, index, offset)) {
            std::memcpy(headerSlot, BlockAt(index, offset), 4);
        }
        ++result_.assetsAliased;
    }

    PopStreamPos();
    return handled;
}

void ZoneLoader::Run() {
    if (zone_.size() < kXFileHeaderBytes + 16) {
        Fail("zone trop petite");
        return;
    }

    // XFile header: size, externalSize, blockSize[9].
    uint64_t totalBlockBytes = 0;
    for (size_t i = 0; i < kBlockCount; ++i) {
        uint32_t blockSize = 0;
        std::memcpy(&blockSize, zone_.data() + 8 + i * 4, 4);
        result_.blockSizes[i] = blockSize;
        totalBlockBytes += blockSize;
    }
    if (totalBlockBytes > kMaxTotalBlockBytes) {
        Fail("blocs de zone au-dela de la limite memoire");
        return;
    }
    for (size_t i = 0; i < kBlockCount; ++i) {
        blocks_[i].assign(result_.blockSizes[i], 0);
    }
    result_.started = true;

    // Load_XAssetListCustom: the XAssetList itself lives outside block
    // memory; only its pointer targets land in the blocks.
    uint8_t assetList[16] = {};
    if (!ReadFileData(assetList, sizeof(assetList))) {
        return;
    }
    uint32_t stringCount = 0;
    uint32_t stringsRef = 0;
    uint32_t assetCount = 0;
    uint32_t assetsRef = 0;
    std::memcpy(&stringCount, assetList, 4);
    std::memcpy(&stringsRef, assetList + 4, 4);
    std::memcpy(&assetCount, assetList + 8, 4);
    std::memcpy(&assetsRef, assetList + 12, 4);
    result_.scriptStringCount = stringCount;
    result_.assetCount = assetCount;

    PushStreamPos(4);
    LoadScriptStringList(stringCount, stringsRef);
    PopStreamPos();
    if (failed_) {
        return;
    }

    // Load_XAssetArrayCustom: the XAsset directory is a contiguous array in
    // block 4; each entry's header is deserialized in turn.
    if (assetCount == 0 || assetsRef == 0) {
        return;
    }
    if (assetsRef != kPtrInline) {
        Fail("XAssetList.assets n'est pas inline");
        return;
    }
    PushStreamPos(4);
    const uint32_t directoryOffset = AllocStreamPos(3);
    const uint64_t directoryBytes = static_cast<uint64_t>(assetCount) * 8;
    if (directoryOffset + directoryBytes > blocks_[posIndex_].size()) {
        Fail("repertoire d'assets depasse le bloc");
        PopStreamPos();
        return;
    }
    LoadStream(true, BlockAt(posIndex_, directoryOffset), static_cast<uint32_t>(directoryBytes));

    for (uint32_t i = 0; i < assetCount && !failed_; ++i) {
        uint32_t type = 0;
        std::memcpy(&type, BlockAt(4, directoryOffset + i * 8), 4);
        currentAssetIndex_ = i;
        currentAssetType_ = type;
        auto* headerSlot = reinterpret_cast<uint32_t*>(BlockAt(4, directoryOffset + i * 8 + 4));
        if (!LoadAssetHeader(type, headerSlot)) {
            result_.stopReason = "type non supporte a l'index " + std::to_string(i)
                + ": " + AssetTypeName(type);
            break;
        }
        result_.assetRefs.push_back({type, *headerSlot});
    }
    PopStreamPos();

    if (!failed_ && result_.stopReason.empty()) {
        LoadDelayStream();
    }
    for (size_t i = 0; i < blocks_.size(); ++i) {
        result_.blocks[i] = std::move(blocks_[i]);
    }
}
} // namespace

KisakZoneLoadResult LoadZoneAssets(const std::vector<uint8_t>& zone) {
    KisakZoneLoadResult result;
    ZoneLoader loader(zone, result);
    loader.Run();
    return result;
}

namespace {
const std::vector<uint8_t>* ViewBlock(const KisakZoneLoadResult& result, uint32_t ref) {
    const uint32_t block = (ref - 1) >> 28;
    if (block == 15) {
        return &result.assetPool;
    }
    if (block < result.blocks.size()) {
        return &result.blocks[block];
    }
    return nullptr;
}
} // namespace

bool KisakZoneView::ValidRef(uint32_t ref, uint32_t bytes) const {
    if (ref == 0) {
        return false;
    }
    const std::vector<uint8_t>* block = ViewBlock(result_, ref);
    const uint32_t offset = (ref - 1) & 0xfffffff;
    return block != nullptr && static_cast<uint64_t>(offset) + bytes <= block->size();
}

const uint8_t* KisakZoneView::Ptr(uint32_t ref) const {
    return ViewBlock(result_, ref)->data() + ((ref - 1) & 0xfffffff);
}

uint32_t KisakZoneView::U32(uint32_t ref, uint32_t offset) const {
    uint32_t value = 0;
    if (ValidRef(ref, offset + 4)) {
        std::memcpy(&value, Ptr(ref) + offset, 4);
    }
    return value;
}

uint16_t KisakZoneView::U16(uint32_t ref, uint32_t offset) const {
    uint16_t value = 0;
    if (ValidRef(ref, offset + 2)) {
        std::memcpy(&value, Ptr(ref) + offset, 2);
    }
    return value;
}

uint8_t KisakZoneView::U8(uint32_t ref, uint32_t offset) const {
    return ValidRef(ref, offset + 1) ? Ptr(ref)[offset] : 0;
}

float KisakZoneView::F32(uint32_t ref, uint32_t offset) const {
    float value = 0.0f;
    if (ValidRef(ref, offset + 4)) {
        std::memcpy(&value, Ptr(ref) + offset, 4);
    }
    return value;
}

std::string KisakZoneView::StrAt(uint32_t ref, uint32_t offset) const {
    const uint32_t strRef = U32(ref, offset);
    if (!ValidRef(strRef)) {
        return {};
    }
    const std::vector<uint8_t>& data = *ViewBlock(result_, strRef);
    const uint32_t strOffset = (strRef - 1) & 0xfffffff;
    const auto terminator = std::find(data.begin() + strOffset, data.end(), uint8_t{0});
    return std::string(data.begin() + strOffset, terminator);
}

std::string DescribeZoneLoad(const KisakZoneLoadResult& result) {
    std::ostringstream out;
    if (!result.started) {
        out << "zone loader non demarre: " << result.error;
        return out.str();
    }

    out << result.assetsLoaded << "/" << result.assetCount << " assets charges";
    if (result.assetsAliased > 0) {
        out << " (+" << result.assetsAliased << " alias)";
    }
    out << ", localize=" << result.localizeEntries.size()
        << ", rawfiles=" << result.rawFiles.size()
        << ", techsets=" << result.techniqueSets.size()
        << ", materials=" << result.materials.size()
        << ", images=" << result.images.size()
        << ", sounds=" << result.soundAliases.size()
        << ", sndcurves=" << result.soundCurveCount
        << ", loadedsounds=" << result.loadedSoundCount
        << ", fonts=" << result.fonts.size()
        << ", menus=" << result.menus.size()
        << ", weapons=" << result.weapons.size()
        << ", xanims=" << result.xanimCount
        << ", xmodels=" << result.xmodels.size()
        << ", fx=" << result.fxCount
        << ", stringtables=" << result.stringTables.size()
        << ", scriptStrings=" << result.scriptStrings.size();
    if (!result.worlds.empty()) {
        out << ", worlds=[";
        for (size_t i = 0; i < result.worlds.size(); ++i) {
            out << (i ? "," : "") << result.worlds[i];
        }
        out << ']';
    }

    out << ", blocs=[";
    for (size_t i = 0; i < result.blockUsed.size(); ++i) {
        out << (i ? "," : "") << result.blockUsed[i] << "/" << result.blockSizes[i];
    }
    out << ']';

    if (!result.localizeEntries.empty()) {
        const KisakLoadedLocalize& first = result.localizeEntries.front();
        out << ", 1er localize " << first.name << "=\"" << first.value << '"';
    }
    if (!result.techniqueSets.empty()) {
        const KisakLoadedTechniqueSet& first = result.techniqueSets.front();
        out << ", 1er techset " << first.name
            << " (" << first.techniqueCount << " techniques, " << first.passCount << " passes)";
    }
    if (!result.materials.empty()) {
        out << ", 1er material " << result.materials.front().name;
    }
    if (!result.fonts.empty()) {
        const KisakLoadedFont& first = result.fonts.front();
        out << ", 1ere font " << first.name << " (" << first.glyphCount << " glyphes)";
    }
    if (!result.soundAliases.empty()) {
        out << ", 1er son " << result.soundAliases.front().name;
    }
    if (!result.menus.empty()) {
        out << ", 1er menu " << result.menus.front();
    }
    if (!result.weapons.empty()) {
        out << ", 1ere arme " << result.weapons.front();
    }
    if (!result.xmodels.empty()) {
        out << ", 1er xmodel " << result.xmodels.front();
    }
    if (!result.images.empty()) {
        const KisakLoadedImage& first = result.images.front();
        out << ", 1ere image " << first.name << " " << first.width << 'x' << first.height
            << " fmt=0x" << std::hex << first.format << std::dec
            << " (" << first.resourceSize << " octets)";
    }
    if (!result.stopReason.empty()) {
        out << ", arret: " << result.stopReason;
    }
    if (!result.error.empty()) {
        out << ", erreur: " << result.error;
    }
    return out.str();
}
