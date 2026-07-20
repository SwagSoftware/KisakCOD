#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

// Faithful 64-bit port of the IW3 zone loader stream mechanics
// (src/database/db_stream.cpp + db_load.cpp). Block memory keeps the exact
// 32-bit serialized layout: pointer slots stay 4 bytes and hold the engine's
// own offset encoding ((block << 28) | offset) + 1 instead of native
// pointers, so struct sizes and stream byte counts match the retail data.
// Asset payload deserializers are ported per type; unsupported types stop the
// walk cleanly.

struct KisakLoadedLocalize {
    std::string name;
    std::string value;
};

struct KisakLoadedRawFile {
    std::string name;
    uint32_t length = 0;
    std::string contents;
};

struct KisakLoadedImage {
    std::string name;
    uint16_t width = 0;
    uint16_t height = 0;
    uint8_t levelCount = 0;
    uint32_t format = 0;
    uint32_t resourceSize = 0;
    bool hasInlinePixels = false;
    // Inline payload (mip 0 first) copied out of the rewinding temp block —
    // lightmaps and other zone-embedded textures live here.
    std::vector<uint8_t> pixels;
};

struct KisakLoadedTechniqueSet {
    std::string name;
    uint32_t techniqueCount = 0;
    uint32_t passCount = 0;
};

struct KisakLoadedMaterial {
    std::string name;
    uint8_t textureCount = 0;
};

struct KisakLoadedSoundAlias {
    std::string name;
    uint32_t aliasCount = 0;
};

struct KisakLoadedFont {
    std::string name;
    uint32_t glyphCount = 0;
};

struct KisakZoneLoadResult {
    bool started = false;
    uint32_t scriptStringCount = 0;
    std::vector<std::string> scriptStrings;
    uint32_t assetCount = 0;
    uint32_t assetsLoaded = 0;
    uint32_t assetsAliased = 0;
    std::array<uint32_t, 9> blockSizes{};
    std::array<uint32_t, 9> blockUsed{};
    std::vector<KisakLoadedLocalize> localizeEntries;
    std::vector<KisakLoadedRawFile> rawFiles;
    std::vector<KisakLoadedImage> images;
    std::vector<KisakLoadedTechniqueSet> techniqueSets;
    std::vector<KisakLoadedMaterial> materials;
    std::vector<KisakLoadedSoundAlias> soundAliases;
    std::vector<KisakLoadedFont> fonts;
    uint32_t soundCurveCount = 0;
    uint32_t loadedSoundCount = 0;
    std::vector<std::string> menus;
    std::vector<std::string> weapons;
    std::vector<std::string> xmodels;
    std::vector<std::string> stringTables;
    uint32_t xanimCount = 0;
    uint32_t fxCount = 0;
    uint32_t lightDefCount = 0;
    uint32_t impactFxCount = 0;
    uint32_t physPresetCount = 0;
    uint32_t menuListCount = 0;
    // World assets ("type:name"): comworld, gameworld_sp, map_ents, clipmap,
    // gfxworld.
    std::vector<std::string> worlds;

    // Retained zone memory and the processed asset directory, so consumers
    // (renderer, tools) can walk the loaded structs after the walk. Asset
    // structs live in the temp block during the walk and are copied into
    // assetPool (the DB_AddXAsset role); pool refs use pseudo-block 15.
    std::array<std::vector<uint8_t>, 9> blocks;
    std::vector<uint8_t> assetPool;
    std::vector<std::pair<uint32_t, uint32_t>> assetRefs; // {type, headerRef}
    std::string stopReason;
    std::string error;
};

// Runs the ported DB_LoadXFile walk over a fully decompressed zone
// (including the 44-byte XFile header at the start).
KisakZoneLoadResult LoadZoneAssets(const std::vector<uint8_t>& zone);

std::string DescribeZoneLoad(const KisakZoneLoadResult& result);

// Read-only accessor over a loaded zone's blocks using the engine's
// ((block << 28) | offset) + 1 reference encoding.
class KisakZoneView {
public:
    explicit KisakZoneView(const KisakZoneLoadResult& result) : result_(result) {}

    bool ValidRef(uint32_t ref, uint32_t bytes = 1) const;
    const uint8_t* Ptr(uint32_t ref) const;
    uint32_t U32(uint32_t ref, uint32_t offset) const;
    uint16_t U16(uint32_t ref, uint32_t offset) const;
    uint8_t U8(uint32_t ref, uint32_t offset) const;
    float F32(uint32_t ref, uint32_t offset) const;
    // Resolves the string a slot at ref+offset points to ("" when null).
    std::string StrAt(uint32_t ref, uint32_t offset) const;

private:
    const KisakZoneLoadResult& result_;
};
