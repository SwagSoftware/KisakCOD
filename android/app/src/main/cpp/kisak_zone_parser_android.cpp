#include "kisak_zone_parser_android.h"

#include <algorithm>
#include <iterator>
#include <sstream>

namespace {
constexpr size_t kBlockSizeCount = 9;
constexpr size_t kXFileBytes = (2 + kBlockSizeCount) * 4;
constexpr size_t kXAssetListBytes = 16;
constexpr uint32_t kInlinePointer = 0xffffffffu;

// XAssetType from src/xanim/xanim.h (Win32 SP/MP layout).
constexpr const char* kAssetTypeNames[] = {
    "xmodelpieces",   // 0x00
    "physpreset",     // 0x01
    "xanimparts",     // 0x02
    "xmodel",         // 0x03
    "material",       // 0x04
    "techniqueset",   // 0x05
    "image",          // 0x06
    "sound",          // 0x07
    "soundcurve",     // 0x08
    "loadedsound",    // 0x09
    "clipmap",        // 0x0a
    "clipmap_pvs",    // 0x0b
    "comworld",       // 0x0c
    "gameworld_sp",   // 0x0d
    "gameworld_mp",   // 0x0e
    "map_ents",       // 0x0f
    "gfxworld",       // 0x10
    "lightdef",       // 0x11
    "uimap",          // 0x12
    "font",           // 0x13
    "menulist",       // 0x14
    "menu",           // 0x15
    "localize",       // 0x16
    "weapon",         // 0x17
    "snddriverglobals", // 0x18
    "fx",             // 0x19
    "impactfx",       // 0x1a
    "aitype",         // 0x1b
    "mptype",         // 0x1c
    "character",      // 0x1d
    "xmodelalias",    // 0x1e
    "rawfile",        // 0x1f
    "stringtable",    // 0x20
};

std::string AssetTypeName(uint32_t type) {
    if (type < std::size(kAssetTypeNames)) {
        return kAssetTypeNames[type];
    }
    return "type_" + std::to_string(type);
}

struct ZoneCursor {
    const uint8_t* data;
    size_t size;
    size_t offset = 0;

    bool CanRead(size_t bytes) const {
        return offset + bytes <= size;
    }

    uint32_t ReadU32() {
        const uint8_t* p = data + offset;
        offset += 4;
        return static_cast<uint32_t>(p[0])
            | (static_cast<uint32_t>(p[1]) << 8)
            | (static_cast<uint32_t>(p[2]) << 16)
            | (static_cast<uint32_t>(p[3]) << 24);
    }

    bool ReadString(std::string& out, size_t maxLength) {
        out.clear();
        while (offset < size && out.size() <= maxLength) {
            const char ch = static_cast<char>(data[offset++]);
            if (ch == '\0') {
                return true;
            }
            out.push_back(ch);
        }
        return false;
    }
};
KisakZoneParse ParseZoneAssetDirectoryInternal(const std::vector<uint8_t>& zone, bool requireFullZone) {
    KisakZoneParse parse;
    if (zone.size() < kXFileBytes + kXAssetListBytes) {
        parse.error = "zone trop petite pour XFile + XAssetList";
        parse.needsMore = true;
        return parse;
    }

    ZoneCursor cursor{zone.data(), zone.size()};
    parse.xfileSize = cursor.ReadU32();
    parse.xfileExternalSize = cursor.ReadU32();
    for (size_t i = 0; i < kBlockSizeCount; ++i) {
        parse.blockSizes.push_back(cursor.ReadU32());
    }
    const uint64_t expectedZoneBytes = static_cast<uint64_t>(parse.xfileSize) + kXFileBytes;
    if (requireFullZone && expectedZoneBytes != zone.size()) {
        parse.error = "XFile.size incoherent avec la taille de zone";
        return parse;
    }
    if (!requireFullZone && expectedZoneBytes < zone.size()) {
        parse.error = "prefixe plus grand que la taille XFile";
        return parse;
    }

    const uint32_t stringCount = cursor.ReadU32();
    const uint32_t stringsPointer = cursor.ReadU32();
    const uint32_t assetCount = cursor.ReadU32();
    const uint32_t assetsPointer = cursor.ReadU32();
    parse.scriptStringCount = stringCount;
    parse.assetCount = assetCount;

    if (stringCount > 0) {
        if (stringsPointer != kInlinePointer) {
            parse.error = "ScriptStringList.strings n'est pas inline";
            return parse;
        }
        if (!cursor.CanRead(static_cast<size_t>(stringCount) * 4)) {
            parse.error = "tableau de pointeurs de strings tronque";
            parse.needsMore = true;
            return parse;
        }
        std::vector<bool> inlineString(stringCount);
        for (uint32_t i = 0; i < stringCount; ++i) {
            inlineString[i] = cursor.ReadU32() == kInlinePointer;
        }
        for (uint32_t i = 0; i < stringCount; ++i) {
            if (!inlineString[i]) {
                continue;
            }
            std::string value;
            if (!cursor.ReadString(value, 4096)) {
                parse.error = "script string tronquee a l'index " + std::to_string(i);
                parse.needsMore = true;
                return parse;
            }
            parse.scriptStrings.push_back(std::move(value));
        }
    }

    if (assetCount > 0) {
        if (assetsPointer != kInlinePointer) {
            parse.error = "XAssetList.assets n'est pas inline";
            return parse;
        }
        if (!cursor.CanRead(static_cast<size_t>(assetCount) * 8)) {
            parse.error = "repertoire d'assets tronque";
            parse.needsMore = true;
            return parse;
        }
        for (uint32_t i = 0; i < assetCount; ++i) {
            const uint32_t type = cursor.ReadU32();
            cursor.ReadU32(); // asset header pointer, payload follows later
            parse.assetTypes.push_back(type);
            ++parse.assetTypeHistogram[AssetTypeName(type)];
        }
    }

    parse.parsedBytes = cursor.offset;
    parse.valid = true;
    return parse;
}
} // namespace

KisakZoneParse ParseZoneAssetDirectory(const std::vector<uint8_t>& zone) {
    return ParseZoneAssetDirectoryInternal(zone, true);
}

KisakZoneParse ParseZoneAssetDirectoryPrefix(const std::vector<uint8_t>& zonePrefix) {
    return ParseZoneAssetDirectoryInternal(zonePrefix, false);
}

std::string DescribeZoneParse(const KisakZoneParse& parse) {
    std::ostringstream out;
    if (!parse.valid) {
        out << (parse.needsMore ? "zone incomplete: " : "zone invalide: ") << parse.error;
        return out.str();
    }

    out << "XFile size=" << parse.xfileSize
        << " externalSize=" << parse.xfileExternalSize
        << ", scriptStrings=" << parse.scriptStringCount
        << ", assets=" << parse.assetCount
        << ", parsedBytes=" << parse.parsedBytes << " {";
    bool first = true;
    for (const auto& [name, count] : parse.assetTypeHistogram) {
        out << (first ? "" : ", ") << name << ':' << count;
        first = false;
    }
    out << '}';

    if (!parse.scriptStrings.empty()) {
        out << ", premieres strings=[";
        const size_t sampleCount = std::min<size_t>(parse.scriptStrings.size(), 8);
        for (size_t i = 0; i < sampleCount; ++i) {
            out << (i ? "," : "") << parse.scriptStrings[i];
        }
        out << ']';
    }
    return out.str();
}
