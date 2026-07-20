#include "kisak_zone_rawfile_android.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

namespace fs = std::filesystem;

namespace {
constexpr uint32_t kInlinePointer = 0xffffffffu;
constexpr uint32_t kMaxRawFileBytes = 16 * 1024 * 1024;
constexpr size_t kMaxNameLength = 200;
constexpr size_t kMinNameLength = 3;

uint32_t ReadU32(const uint8_t* data) {
    return static_cast<uint32_t>(data[0])
        | (static_cast<uint32_t>(data[1]) << 8)
        | (static_cast<uint32_t>(data[2]) << 16)
        | (static_cast<uint32_t>(data[3]) << 24);
}

bool IsPlausibleName(const uint8_t* begin, const uint8_t* end) {
    const size_t length = static_cast<size_t>(end - begin);
    if (length < kMinNameLength || length > kMaxNameLength) {
        return false;
    }
    return std::all_of(begin, end, [](uint8_t ch) {
        return ch >= 0x20 && ch <= 0x7e;
    });
}

// A dumped rawfile path must stay inside the dump root even if a zone
// contains hostile names ("../", absolute paths, drive letters).
bool IsSafeRelativePath(const std::string& name) {
    if (name.empty() || name.front() == '/' || name.find(':') != std::string::npos) {
        return false;
    }
    std::istringstream stream(name);
    std::string part;
    while (std::getline(stream, part, '/')) {
        if (part.empty() || part == "." || part == "..") {
            return false;
        }
    }
    return true;
}
} // namespace

std::vector<KisakZoneRawFile> ScanZoneRawFiles(const std::vector<uint8_t>& zone) {
    std::vector<KisakZoneRawFile> results;
    if (zone.size() < 16) {
        return results;
    }

    size_t offset = 0;
    const size_t scanEnd = zone.size() - 12;
    while (offset < scanEnd) {
        if (ReadU32(zone.data() + offset) != kInlinePointer
            || ReadU32(zone.data() + offset + 8) != kInlinePointer) {
            ++offset;
            continue;
        }

        const uint32_t length = ReadU32(zone.data() + offset + 4);
        if (length == 0 || length > kMaxRawFileBytes) {
            ++offset;
            continue;
        }

        const size_t nameStart = offset + 12;
        const size_t nameSearchEnd = std::min(zone.size(), nameStart + kMaxNameLength + 1);
        const auto nameTerminator = std::find(
            zone.begin() + nameStart,
            zone.begin() + nameSearchEnd,
            uint8_t{0}
        );
        if (nameTerminator == zone.begin() + nameSearchEnd) {
            ++offset;
            continue;
        }

        const size_t nameEnd = static_cast<size_t>(nameTerminator - zone.begin());
        const size_t contentStart = nameEnd + 1;
        const size_t contentTerminator = contentStart + length;
        if (contentTerminator >= zone.size()
            || zone[contentTerminator] != 0
            || !IsPlausibleName(zone.data() + nameStart, zone.data() + nameEnd)) {
            ++offset;
            continue;
        }

        KisakZoneRawFile rawFile;
        rawFile.name.assign(
            reinterpret_cast<const char*>(zone.data() + nameStart),
            nameEnd - nameStart
        );
        rawFile.length = length;
        rawFile.zoneOffset = offset;
        rawFile.contentOffset = contentStart;
        results.push_back(std::move(rawFile));

        offset = contentTerminator + 1;
    }
    return results;
}

uint32_t DumpZoneRawFiles(
    const std::vector<uint8_t>& zone,
    const std::vector<KisakZoneRawFile>& rawFiles,
    const std::string& dumpRoot,
    std::string& errorOut
) {
    uint32_t written = 0;
    std::error_code error;
    const fs::path root(dumpRoot);

    for (const KisakZoneRawFile& rawFile : rawFiles) {
        if (!IsSafeRelativePath(rawFile.name)) {
            errorOut += "nom rejete: " + rawFile.name + "; ";
            continue;
        }

        const fs::path target = root / rawFile.name;
        fs::create_directories(target.parent_path(), error);
        if (error) {
            errorOut += "mkdir a echoue: " + rawFile.name + "; ";
            continue;
        }

        std::ofstream out(target, std::ios::binary | std::ios::trunc);
        out.write(
            reinterpret_cast<const char*>(zone.data() + rawFile.contentOffset),
            rawFile.length
        );
        if (!out) {
            errorOut += "ecriture a echoue: " + rawFile.name + "; ";
            continue;
        }
        ++written;
    }
    return written;
}

std::string DescribeZoneRawFiles(const std::vector<KisakZoneRawFile>& rawFiles) {
    std::ostringstream out;
    uint64_t totalBytes = 0;
    for (const KisakZoneRawFile& rawFile : rawFiles) {
        totalBytes += rawFile.length;
    }
    out << rawFiles.size() << " rawfiles (" << totalBytes << " octets)";
    const size_t sampleCount = std::min<size_t>(rawFiles.size(), 6);
    if (sampleCount > 0) {
        out << " ex=[";
        for (size_t i = 0; i < sampleCount; ++i) {
            out << (i ? "," : "") << rawFiles[i].name << ':' << rawFiles[i].length;
        }
        out << ']';
    }
    return out.str();
}
