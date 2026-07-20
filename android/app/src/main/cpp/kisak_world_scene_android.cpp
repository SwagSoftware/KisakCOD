#include "kisak_world_scene_android.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <sstream>

#include "kisak_iwi_texture_android.h"

namespace {
constexpr uint32_t kAssetTypeClipMapSp = 0x0a;
constexpr uint32_t kAssetTypeMapEnts = 0x0f;
constexpr uint32_t kAssetTypeGfxWorld = 0x10;
constexpr uint8_t kTextureSemanticColorMap = 2;

// GfxWorld field offsets (32-bit serialized layout, see r_bsp.h).
constexpr uint32_t kWorldIndexCount = 16;
constexpr uint32_t kWorldIndices = 20;
constexpr uint32_t kWorldSurfaceCount = 24;
constexpr uint32_t kWorldVertexCount = 48;
constexpr uint32_t kWorldVertices = 52;
constexpr uint32_t kWorldMins = 344;
constexpr uint32_t kWorldMaxs = 356;
constexpr uint32_t kWorldDpvsSurfaces = 580 + 80;

constexpr uint32_t kVertexStride = 44;   // GfxWorldVertex
constexpr uint32_t kSurfaceStride = 48;  // GfxSurface

// IEEE 754 half -> float (packed XModel texcoords).
float HalfToFloat(uint16_t half) {
    const uint32_t sign = static_cast<uint32_t>(half & 0x8000u) << 16;
    uint32_t exponent = (half >> 10) & 0x1f;
    uint32_t mantissa = half & 0x3ffu;
    uint32_t bits;
    if (exponent == 0) {
        if (mantissa == 0) {
            bits = sign;
        } else {
            exponent = 113;
            while ((mantissa & 0x400u) == 0) {
                mantissa <<= 1;
                --exponent;
            }
            bits = sign | (exponent << 23) | ((mantissa & 0x3ffu) << 13);
        }
    } else if (exponent == 31) {
        bits = sign | 0x7f800000u | (mantissa << 13);
    } else {
        bits = sign | ((exponent + 112) << 23) | (mantissa << 13);
    }
    float value = 0.0f;
    std::memcpy(&value, &bits, 4);
    return value;
}

uint32_t FindAssetRef(const KisakZoneLoadResult& zone, uint32_t type) {
    for (const auto& [assetType, ref] : zone.assetRefs) {
        if (assetType == type && ref != 0) {
            return ref;
        }
    }
    return 0;
}

// Material -> colormap image name (semantic 2), falling back to the first
// texture def like the 2D path does.
std::string MaterialColorMapImage(const KisakZoneView& view, uint32_t materialRef) {
    if (!view.ValidRef(materialRef, 80)) {
        return {};
    }
    const uint32_t textureTable = view.U32(materialRef, 68);
    const uint8_t textureCount = view.U8(materialRef, 58);
    if (textureCount == 0 || !view.ValidRef(textureTable, 12u * textureCount)) {
        return {};
    }
    uint32_t chosenImage = view.U32(textureTable, 8);
    for (uint8_t i = 0; i < textureCount; ++i) {
        if (view.U8(textureTable, i * 12 + 7) == kTextureSemanticColorMap) {
            chosenImage = view.U32(textureTable, i * 12 + 8);
            break;
        }
    }
    if (!view.ValidRef(chosenImage, 36)) {
        return {};
    }
    std::string name = view.StrAt(chosenImage, 32);
    // Cross-zone asset references serialize with a leading comma; the IWI
    // file on disk uses the bare name.
    if (!name.empty() && name.front() == ',') {
        name.erase(name.begin());
    }
    return name;
}

// Minimal entity-string scan: find the info_player_start block and read its
// origin/angles. Entity strings are sequences of { "key" "value" ... } blocks.
void ParseSpawnPoint(const std::string& entities, KisakWorldScene& scene) {
    size_t cursor = 0;
    const auto nextQuoted = [&](size_t from, size_t end, std::string& out) -> size_t {
        const size_t open = entities.find('"', from);
        if (open == std::string::npos || open >= end) {
            return std::string::npos;
        }
        const size_t close = entities.find('"', open + 1);
        if (close == std::string::npos || close > end) {
            return std::string::npos;
        }
        out.assign(entities, open + 1, close - open - 1);
        return close + 1;
    };
    const char* preferredClasses[] = {"info_player_start", "script_origin"};
    for (const char* wantedClass : preferredClasses) {
        cursor = 0;
        while (true) {
            const size_t open = entities.find('{', cursor);
            if (open == std::string::npos) {
                break;
            }
            const size_t close = entities.find('}', open);
            if (close == std::string::npos) {
                break;
            }
            cursor = close + 1;

            std::string classname;
            std::string origin;
            std::string angles;
            size_t at = open;
            while (at < close) {
                std::string key;
                std::string value;
                at = nextQuoted(at, close, key);
                if (at == std::string::npos) {
                    break;
                }
                at = nextQuoted(at, close, value);
                if (at == std::string::npos) {
                    break;
                }
                if (key == "classname") {
                    classname = value;
                } else if (key == "origin") {
                    origin = value;
                } else if (key == "angles") {
                    angles = value;
                }
            }
            if (classname != wantedClass || origin.empty()) {
                continue;
            }
            std::istringstream originIn(origin);
            originIn >> scene.spawnOrigin[0] >> scene.spawnOrigin[1] >> scene.spawnOrigin[2];
            if (!angles.empty()) {
                float pitch = 0.0f;
                float yaw = 0.0f;
                std::istringstream anglesIn(angles);
                anglesIn >> pitch >> yaw;
                scene.spawnYaw = yaw;
            }
            scene.hasSpawn = true;
            return;
        }
    }
}
}  // namespace

KisakWorldScene BuildWorldScene(const KisakZoneLoadResult& zone) {
    KisakWorldScene scene;
    const KisakZoneView view(zone);

    const uint32_t world = FindAssetRef(zone, kAssetTypeGfxWorld);
    if (world == 0 || !view.ValidRef(world, 732)) {
        scene.error = "gfxworld absent de la zone";
        return scene;
    }
    scene.name = view.StrAt(world, 0);
    for (int axis = 0; axis < 3; ++axis) {
        scene.mins[axis] = view.F32(world, kWorldMins + axis * 4);
        scene.maxs[axis] = view.F32(world, kWorldMaxs + axis * 4);
    }

    const uint32_t vertexCount = view.U32(world, kWorldVertexCount);
    const uint32_t vertices = view.U32(world, kWorldVertices);
    const uint32_t indexCount = view.U32(world, kWorldIndexCount);
    const uint32_t indices = view.U32(world, kWorldIndices);
    const uint32_t surfaceCount = view.U32(world, kWorldSurfaceCount);
    const uint32_t surfaces = view.U32(world, kWorldDpvsSurfaces);
    if (vertexCount == 0 || !view.ValidRef(vertices, vertexCount * kVertexStride)) {
        scene.error = "vertex pool du monde invalide";
        return scene;
    }
    if (indexCount == 0 || !view.ValidRef(indices, indexCount * 2)) {
        scene.error = "index pool du monde invalide";
        return scene;
    }
    if (surfaceCount == 0 || !view.ValidRef(surfaces, surfaceCount * kSurfaceStride)) {
        scene.error = "surfaces du monde invalides";
        return scene;
    }

    // Vertex stream: xyz@0, color@16 (BGRA byte order), texCoord@20,
    // lmapCoord@28.
    const uint8_t* vertexData = view.Ptr(vertices);
    scene.vertices.reserve(static_cast<size_t>(vertexCount) * 11);
    for (uint32_t i = 0; i < vertexCount; ++i) {
        const uint8_t* vertex = vertexData + static_cast<size_t>(i) * kVertexStride;
        float position[3];
        float texCoord[2];
        float lmapCoord[2];
        std::memcpy(position, vertex, 12);
        std::memcpy(texCoord, vertex + 20, 8);
        std::memcpy(lmapCoord, vertex + 28, 8);
        const uint8_t* color = vertex + 16;
        scene.vertices.insert(scene.vertices.end(), {
            position[0], position[1], position[2],
            texCoord[0], texCoord[1],
            color[2] / 255.0f, color[1] / 255.0f, color[0] / 255.0f, color[3] / 255.0f,
            lmapCoord[0], lmapCoord[1],
        });
    }

    // Lightmap pairs: GfxWorld lightmapCount@264, GfxLightmapArray@268
    // (primary image@0, secondary@4). The pixel payloads were copied out of
    // the rewinding temp block into zone.images; match them by name.
    const auto decodeZoneImage = [&](uint32_t imageRef) -> KisakIwiTexture {
        KisakIwiTexture decoded;
        if (!view.ValidRef(imageRef, 36)) {
            return decoded;
        }
        const std::string imageName = view.StrAt(imageRef, 32);
        for (const KisakLoadedImage& image : zone.images) {
            if (image.name == imageName && image.hasInlinePixels) {
                decoded = DecodeD3DTopMip(
                    image.format, image.width, image.height,
                    image.pixels.data(), image.pixels.size());
                break;
            }
        }
        return decoded;
    };
    const uint32_t lightmapCount = view.U32(world, 264);
    const uint32_t lightmapArray = view.U32(world, 268);
    if (lightmapCount != 0 && view.ValidRef(lightmapArray, lightmapCount * 8)) {
        for (uint32_t i = 0; i < lightmapCount; ++i) {
            KisakWorldLightmap page;
            page.primary = decodeZoneImage(view.U32(lightmapArray, i * 8));
            page.secondary = decodeZoneImage(view.U32(lightmapArray, i * 8 + 4));
            scene.lightmaps.push_back(std::move(page));
        }
    }
    // Sun color from the GfxWorld sunLight (GfxLight color@4).
    const uint32_t sunLight = view.U32(world, 200);
    if (view.ValidRef(sunLight, 64)) {
        for (int channel = 0; channel < 3; ++channel) {
            scene.sunColor[channel] = view.F32(sunLight, 4 + channel * 4);
        }
    }

    const uint8_t* indexData = view.Ptr(indices);
    const uint8_t* surfaceData = view.Ptr(surfaces);
    std::map<std::string, int> textureIndexByImage;
    struct SurfaceState {
        int textureIndex;
        float alphaTestRef;
        bool blended;
        uint8_t srcBlend;
        uint8_t dstBlend;
        bool cullNone;
        bool sky;
    };
    // Material -> texture slot + draw state. The material's GfxStateBits
    // table has one entry per used technique; pick the entry the world pass
    // would use (stateBitsEntry[34]@24 indexed by the first technique the
    // techset provides among lit_sun(8) > lit(7) > emissive(5) > unlit(4);
    // techniques[34] live at techset+12). loadBits[0]: blend factors in
    // nibbles 0/1 (D3DBLEND values), ATEST field bits 12-13 (1 = >0,
    // 3 = >=128), cull field bits 14-15 (1 = none, 2 = back).
    const auto applyStateBits = [](SurfaceState& state, uint32_t loadBits0) {
        const uint32_t alphaTestField = (loadBits0 >> 12) & 3;
        if (alphaTestField == 1) {
            state.alphaTestRef = std::max(state.alphaTestRef, 0.02f);
        } else if (alphaTestField == 3) {
            state.alphaTestRef = std::max(state.alphaTestRef, 0.5f);
        }
        if (((loadBits0 >> 14) & 3) != 2) {
            state.cullNone = true;
        }
        const uint8_t srcBlend = loadBits0 & 0xF;
        const uint8_t dstBlend = (loadBits0 >> 4) & 0xF;
        if (srcBlend != 0 && !(srcBlend == 2 && dstBlend == 1)) {
            state.blended = true;
            state.srcBlend = srcBlend;
            state.dstBlend = dstBlend;
        }
    };
    const auto resolveSurfaceState = [&](uint32_t materialRef) -> SurfaceState {
        SurfaceState state{-1, -1.0f, false, 0, 0, false, false};
        const std::string image = MaterialColorMapImage(view, materialRef);
        if (!image.empty()) {
            const auto [it, inserted] =
                textureIndexByImage.try_emplace(image, static_cast<int>(scene.textureImages.size()));
            if (inserted) {
                scene.textureImages.push_back(image);
            }
            state.textureIndex = it->second;
        }
        const uint8_t stateBitsCount = view.U8(materialRef, 60);
        const uint32_t stateBitsTable = view.U32(materialRef, 76);
        if (stateBitsCount == 0 || !view.ValidRef(stateBitsTable, 8u * stateBitsCount)) {
            return state;
        }
        int entryIndex = -1;
        const uint32_t techset = view.U32(materialRef, 64);
        if (view.ValidRef(techset, 148)) {
            for (const uint32_t technique : {8u, 7u, 5u, 4u}) {
                if (view.U32(techset, 12 + technique * 4) != 0) {
                    entryIndex = view.U8(materialRef, 24 + technique);
                    break;
                }
            }
        }
        if (entryIndex >= 0 && entryIndex < stateBitsCount) {
            applyStateBits(state, view.U32(stateBitsTable, entryIndex * 8));
        } else {
            // Reference techsets (empty shells) leave no technique to pick:
            // merge every entry like the engine would never do, as a lenient
            // fallback.
            for (uint8_t bits = 0; bits < stateBitsCount; ++bits) {
                applyStateBits(state, view.U32(stateBitsTable, bits * 8));
            }
        }
        return state;
    };
    struct PendingSurface {
        uint32_t surfaceIndex;
        int textureIndex;
        int lightmapIndex;
        SurfaceState state;
    };
    std::vector<PendingSurface> pending;
    pending.reserve(surfaceCount);
    for (uint32_t i = 0; i < surfaceCount; ++i) {
        const uint8_t* surface = surfaceData + static_cast<size_t>(i) * kSurfaceStride;
        uint32_t materialRef = 0;
        std::memcpy(&materialRef, surface + 16, 4);
        SurfaceState state = resolveSurfaceState(materialRef);
        // Sky materials carry a cubemap colormap and render through the
        // dedicated sky pass.
        if (view.StrAt(materialRef, 0).rfind("wc/sky", 0) == 0) {
            state.sky = true;
            state.blended = false;
            if (state.textureIndex >= 0
                && static_cast<size_t>(state.textureIndex) < scene.textureImages.size()) {
                scene.skyImage = scene.textureImages[state.textureIndex];
            }
            state.textureIndex = -1;
        }
        const uint8_t lightmapIndex = surface[20];
        pending.push_back({
            i, state.textureIndex,
            lightmapIndex < scene.lightmaps.size() ? lightmapIndex : -1,
            state,
        });
    }
    // Opaque surfaces first, then blended ones; both grouped by texture and
    // lightmap page.
    std::stable_sort(pending.begin(), pending.end(),
                     [](const PendingSurface& a, const PendingSurface& b) {
                         if (a.state.sky != b.state.sky) {
                             return !a.state.sky;
                         }
                         if (a.state.blended != b.state.blended) {
                             return !a.state.blended;
                         }
                         if (a.textureIndex != b.textureIndex) {
                             return a.textureIndex < b.textureIndex;
                         }
                         return a.lightmapIndex < b.lightmapIndex;
                     });

    for (const PendingSurface& entry : pending) {
        const uint8_t* surface = surfaceData + static_cast<size_t>(entry.surfaceIndex) * kSurfaceStride;
        uint32_t firstVertex = 0;
        uint16_t triCount = 0;
        uint32_t baseIndex = 0;
        std::memcpy(&firstVertex, surface + 4, 4);
        std::memcpy(&triCount, surface + 10, 2);
        std::memcpy(&baseIndex, surface + 12, 4);
        const uint32_t surfIndexCount = 3u * triCount;
        if (baseIndex + surfIndexCount > indexCount) {
            continue;
        }
        KisakWorldDrawSurface draw;
        draw.firstIndex = static_cast<uint32_t>(scene.indices.size());
        draw.indexCount = surfIndexCount;
        draw.textureIndex = entry.textureIndex;
        draw.lightmapIndex = entry.lightmapIndex;
        draw.alphaTestRef = entry.state.alphaTestRef;
        draw.blended = entry.state.blended;
        draw.srcBlend = entry.state.srcBlend;
        draw.dstBlend = entry.state.dstBlend;
        draw.cullNone = entry.state.cullNone;
        draw.sky = entry.state.sky;
        for (uint32_t k = 0; k < surfIndexCount; ++k) {
            uint16_t index = 0;
            std::memcpy(&index, indexData + static_cast<size_t>(baseIndex + k) * 2, 2);
            scene.indices.push_back(firstVertex + index);
        }
        scene.surfaces.push_back(draw);
    }

    // Static models: unique LOD0 meshes shared through GLES3 instancing.
    // dpvs@580: smodelCount@+0, smodelDrawInsts@+88 (76 bytes per instance:
    // placement origin@4 / axis[3][3]@16 / scale@52, XModel pool ref@56).
    const uint32_t smodelCount = view.U32(world, 580);
    const uint32_t drawInsts = view.U32(world, 580 + 88);
    if (smodelCount != 0 && view.ValidRef(drawInsts, smodelCount * 76)) {
        const uint8_t* instData = view.Ptr(drawInsts);
        std::map<uint32_t, std::vector<uint32_t>> instancesByModel;
        for (uint32_t i = 0; i < smodelCount; ++i) {
            uint32_t model = 0;
            std::memcpy(&model, instData + static_cast<size_t>(i) * 76 + 56, 4);
            if (view.ValidRef(model, 220)) {
                instancesByModel[model].push_back(i);
            }
        }
        for (const auto& [model, instanceIndices] : instancesByModel) {
            // XModel: surfs@32, materialHandles@36, lodInfo[0] numsurfs@44 /
            // surfIndex@46, numLods@196.
            if (static_cast<int16_t>(view.U16(model, 196)) < 1) {
                continue;
            }
            const uint16_t numsurfs = view.U16(model, 44);
            const uint16_t surfIndex = view.U16(model, 46);
            const uint32_t surfs = view.U32(model, 32);
            const uint32_t materials = view.U32(model, 36);
            if (numsurfs == 0 || !view.ValidRef(surfs, (surfIndex + numsurfs) * 56u)) {
                continue;
            }
            KisakWorldModelMesh mesh;
            mesh.modelName = view.StrAt(model, 0);
            for (uint16_t s = 0; s < numsurfs; ++s) {
                // XSurface 56o: vertCount@2, triCount@4, triIndices@12,
                // verts0@28 (GfxPackedVertex 32o: xyz@0, color@16 BGRA,
                // texCoord@20 as two half floats).
                const uint32_t surf = (surfIndex + s) * 56u;
                const uint16_t vertCount = view.U16(surfs, surf + 2);
                const uint16_t triCount = view.U16(surfs, surf + 4);
                const uint32_t triIndices = view.U32(surfs, surf + 12);
                const uint32_t verts0 = view.U32(surfs, surf + 28);
                if (vertCount == 0 || triCount == 0
                    || !view.ValidRef(verts0, vertCount * 32u)
                    || !view.ValidRef(triIndices, triCount * 6u)) {
                    continue;
                }
                const uint32_t baseVertex = static_cast<uint32_t>(scene.modelVertices.size() / 9);
                const uint8_t* vertexData = view.Ptr(verts0);
                for (uint32_t vertex = 0; vertex < vertCount; ++vertex) {
                    const uint8_t* packed = vertexData + static_cast<size_t>(vertex) * 32;
                    float position[3];
                    std::memcpy(position, packed, 12);
                    const uint8_t* color = packed + 16;
                    uint16_t texCoord[2];
                    std::memcpy(texCoord, packed + 20, 4);
                    scene.modelVertices.insert(scene.modelVertices.end(), {
                        position[0], position[1], position[2],
                        HalfToFloat(texCoord[0]), HalfToFloat(texCoord[1]),
                        color[2] / 255.0f, color[1] / 255.0f, color[0] / 255.0f, color[3] / 255.0f,
                    });
                }
                KisakWorldDrawSurface draw;
                draw.firstIndex = static_cast<uint32_t>(scene.modelIndices.size());
                draw.indexCount = 3u * triCount;
                uint32_t materialRef = 0;
                if (view.ValidRef(materials, (surfIndex + s + 1u) * 4u)) {
                    materialRef = view.U32(materials, (surfIndex + s) * 4u);
                }
                const SurfaceState state = resolveSurfaceState(materialRef);
                draw.textureIndex = state.textureIndex;
                draw.alphaTestRef = state.alphaTestRef;
                draw.blended = state.blended;
                draw.srcBlend = state.srcBlend;
                draw.dstBlend = state.dstBlend;
                draw.cullNone = state.cullNone;
                const uint8_t* localIndices = view.Ptr(triIndices);
                for (uint32_t k = 0; k < 3u * triCount; ++k) {
                    uint16_t index = 0;
                    std::memcpy(&index, localIndices + static_cast<size_t>(k) * 2, 2);
                    scene.modelIndices.push_back(baseVertex + index);
                }
                mesh.surfaces.push_back(draw);
            }
            if (mesh.surfaces.empty()) {
                continue;
            }
            mesh.firstInstance = static_cast<uint32_t>(scene.modelInstances.size() / 12);
            mesh.instanceCount = static_cast<uint32_t>(instanceIndices.size());
            for (uint32_t instance : instanceIndices) {
                const uint8_t* inst = instData + static_cast<size_t>(instance) * 76;
                float origin[3];
                float axis[9];
                float scale = 1.0f;
                std::memcpy(origin, inst + 4, 12);
                std::memcpy(axis, inst + 16, 36);
                std::memcpy(&scale, inst + 52, 4);
                for (int row = 0; row < 3; ++row) {
                    for (int component = 0; component < 3; ++component) {
                        scene.modelInstances.push_back(axis[row * 3 + component] * scale);
                    }
                }
                scene.modelInstances.insert(scene.modelInstances.end(),
                                            {origin[0], origin[1], origin[2]});
            }
            scene.models.push_back(std::move(mesh));
        }
    }

    // Spawn point from the map_ents entity string. map_ents rarely has its
    // own directory entry: it deserializes through the clipmap's mapEnts
    // slot (offset 164), so fall back to that path.
    uint32_t mapEnts = FindAssetRef(zone, kAssetTypeMapEnts);
    if (mapEnts == 0) {
        uint32_t clipMap = FindAssetRef(zone, kAssetTypeClipMapSp);
        if (clipMap == 0) {
            clipMap = FindAssetRef(zone, kAssetTypeClipMapSp + 1);
        }
        if (clipMap != 0 && view.ValidRef(clipMap, 284)) {
            mapEnts = view.U32(clipMap, 164);
        }
    }
    if (mapEnts != 0 && view.ValidRef(mapEnts, 12)) {
        const uint32_t entityString = view.U32(mapEnts, 4);
        const uint32_t entityChars = view.U32(mapEnts, 8);
        if (entityChars > 1 && view.ValidRef(entityString, entityChars)) {
            const std::string entities(
                reinterpret_cast<const char*>(view.Ptr(entityString)), entityChars - 1);
            ParseSpawnPoint(entities, scene);
        }
    }
    if (!scene.hasSpawn) {
        for (int axis = 0; axis < 3; ++axis) {
            scene.spawnOrigin[axis] = (scene.mins[axis] + scene.maxs[axis]) * 0.5f;
        }
    }

    // Collision brushes from the clipmap: numBrushes u16@140, brushes@144
    // (80 bytes: mins@0, contents@12, maxs@16, numsides@28, sides@32 —
    // cbrushside 12o with plane ref@0 -> cplane 20o {normal, dist}). The 6
    // AABB planes are implicit; sides hold only the non-axial ones.
    {
        uint32_t clipMap = FindAssetRef(zone, kAssetTypeClipMapSp);
        if (clipMap == 0) {
            clipMap = FindAssetRef(zone, kAssetTypeClipMapSp + 1);
        }
        if (clipMap != 0 && view.ValidRef(clipMap, 284)) {
            const uint32_t brushCount = view.U16(clipMap, 140);
            const uint32_t brushArray = view.U32(clipMap, 144);
            if (brushCount != 0 && view.ValidRef(brushArray, brushCount * 80u)) {
                constexpr uint32_t kPlayerSolidMask = 0x1 | 0x10000; // solid | playerclip
                for (uint32_t i = 0; i < brushCount; ++i) {
                    const uint32_t brush = i * 80u;
                    const uint32_t contents = view.U32(brushArray, brush + 12);
                    if ((contents & kPlayerSolidMask) == 0) {
                        continue;
                    }
                    KisakWorldBrush out;
                    for (int axis = 0; axis < 3; ++axis) {
                        out.mins[axis] = view.F32(brushArray, brush + axis * 4);
                        out.maxs[axis] = view.F32(brushArray, brush + 16 + axis * 4);
                    }
                    out.firstPlane = static_cast<uint32_t>(scene.brushPlanes.size() / 4);
                    const uint32_t numsides = view.U32(brushArray, brush + 28);
                    const uint32_t sides = view.U32(brushArray, brush + 32);
                    if (numsides != 0 && view.ValidRef(sides, numsides * 12u)) {
                        for (uint32_t side = 0; side < numsides; ++side) {
                            const uint32_t plane = view.U32(sides, side * 12);
                            if (!view.ValidRef(plane, 20)) {
                                continue;
                            }
                            scene.brushPlanes.insert(scene.brushPlanes.end(), {
                                view.F32(plane, 0), view.F32(plane, 4),
                                view.F32(plane, 8), view.F32(plane, 12),
                            });
                        }
                    }
                    out.planeCount = static_cast<uint32_t>(scene.brushPlanes.size() / 4) - out.firstPlane;
                    scene.brushes.push_back(out);
                }
            }
        }
    }

    // Walkable heightfield: rasterize upward-facing world triangles into a
    // coarse grid so the camera can follow the floor.
    {
        KisakWorldHeightfield& field = scene.ground;
        field.originX = scene.mins[0];
        field.originY = scene.mins[1];
        const float spanX = std::max(scene.maxs[0] - scene.mins[0], field.cellSize);
        const float spanY = std::max(scene.maxs[1] - scene.mins[1], field.cellSize);
        field.width = std::min<uint32_t>(1536, static_cast<uint32_t>(spanX / field.cellSize) + 1);
        field.height = std::min<uint32_t>(1536, static_cast<uint32_t>(spanY / field.cellSize) + 1);
        std::vector<std::pair<uint32_t, float>> entries;
        entries.reserve(static_cast<size_t>(field.width) * field.height);
        const size_t vertexStride = 11;
        for (const KisakWorldDrawSurface& surface : scene.surfaces) {
            if (surface.sky || surface.blended) {
                continue;
            }
            for (uint32_t k = 0; k + 2 < surface.indexCount; k += 3) {
                const float* v0 = &scene.vertices[scene.indices[surface.firstIndex + k] * vertexStride];
                const float* v1 = &scene.vertices[scene.indices[surface.firstIndex + k + 1] * vertexStride];
                const float* v2 = &scene.vertices[scene.indices[surface.firstIndex + k + 2] * vertexStride];
                const float e1x = v1[0] - v0[0];
                const float e1y = v1[1] - v0[1];
                const float e1z = v1[2] - v0[2];
                const float e2x = v2[0] - v0[0];
                const float e2y = v2[1] - v0[1];
                const float e2z = v2[2] - v0[2];
                const float nx = e1y * e2z - e1z * e2y;
                const float ny = e1z * e2x - e1x * e2z;
                const float nz = e1x * e2y - e1y * e2x;
                const float lengthSq = nx * nx + ny * ny + nz * nz;
                if (lengthSq < 1e-6f || nz * nz < 0.30f * lengthSq) {
                    continue; // steeper than ~57 degrees
                }
                const float area2 = e1x * e2y - e1y * e2x; // signed, in XY
                if (area2 > -1e-3f && area2 < 1e-3f) {
                    continue;
                }
                const float minX = std::min({v0[0], v1[0], v2[0]});
                const float maxX = std::max({v0[0], v1[0], v2[0]});
                const float minY = std::min({v0[1], v1[1], v2[1]});
                const float maxY = std::max({v0[1], v1[1], v2[1]});
                const int cellX0 = std::max(0, static_cast<int>((minX - field.originX) / field.cellSize));
                const int cellX1 = std::min<int>(field.width - 1,
                                                 static_cast<int>((maxX - field.originX) / field.cellSize));
                const int cellY0 = std::max(0, static_cast<int>((minY - field.originY) / field.cellSize));
                const int cellY1 = std::min<int>(field.height - 1,
                                                 static_cast<int>((maxY - field.originY) / field.cellSize));
                for (int cellY = cellY0; cellY <= cellY1; ++cellY) {
                    for (int cellX = cellX0; cellX <= cellX1; ++cellX) {
                        const float px = field.originX + (cellX + 0.5f) * field.cellSize;
                        const float py = field.originY + (cellY + 0.5f) * field.cellSize;
                        const float w0 = ((v1[0] - px) * (v2[1] - py) - (v1[1] - py) * (v2[0] - px)) / area2;
                        const float w1 = ((v2[0] - px) * (v0[1] - py) - (v2[1] - py) * (v0[0] - px)) / area2;
                        const float w2 = 1.0f - w0 - w1;
                        if (w0 < -0.02f || w1 < -0.02f || w2 < -0.02f) {
                            continue;
                        }
                        const float z = w0 * v0[2] + w1 * v1[2] + w2 * v2[2];
                        entries.push_back({static_cast<uint32_t>(cellY) * field.width + cellX, z});
                    }
                }
            }
        }
        std::sort(entries.begin(), entries.end());
        field.cellStart.assign(static_cast<size_t>(field.width) * field.height + 1, 0);
        field.heights.reserve(entries.size());
        size_t cursor = 0;
        for (uint32_t cellIndex = 0; cellIndex < field.width * field.height; ++cellIndex) {
            field.cellStart[cellIndex] = static_cast<uint32_t>(field.heights.size());
            float lastHeight = -1e30f;
            while (cursor < entries.size() && entries[cursor].first == cellIndex) {
                const float z = entries[cursor].second;
                // Entries within a cell arrive height-sorted (pair ordering);
                // collapse near-duplicates from adjacent triangles.
                if (z - lastHeight > 4.0f) {
                    field.heights.push_back(z);
                    lastHeight = z;
                }
                ++cursor;
            }
        }
        field.cellStart[static_cast<size_t>(field.width) * field.height] =
            static_cast<uint32_t>(field.heights.size());
        field.valid = !field.heights.empty();
    }
    return scene;
}

bool ResolveWorldCollision(
    const std::vector<KisakWorldBrush>& brushes,
    const std::vector<float>& brushPlanes,
    float position[3],
    float radius
) {
    bool moved = false;
    for (int iteration = 0; iteration < 3; ++iteration) {
        bool pushed = false;
        for (const KisakWorldBrush& brush : brushes) {
            if (position[0] < brush.mins[0] - radius || position[0] > brush.maxs[0] + radius
                || position[1] < brush.mins[1] - radius || position[1] > brush.maxs[1] + radius
                || position[2] < brush.mins[2] - radius || position[2] > brush.maxs[2] + radius) {
                continue;
            }
            // Deepest (least negative) signed distance over every plane of
            // the expanded convex brush; positive means outside.
            float bestDistance = -1e30f;
            float bestNormal[3] = {0.0f, 0.0f, 1.0f};
            bool outside = false;
            for (int axis = 0; axis < 3 && !outside; ++axis) {
                const float distanceMax = position[axis] - (brush.maxs[axis] + radius);
                const float distanceMin = (brush.mins[axis] - radius) - position[axis];
                if (distanceMax > 0.0f || distanceMin > 0.0f) {
                    outside = true;
                    break;
                }
                if (distanceMax > bestDistance) {
                    bestDistance = distanceMax;
                    bestNormal[0] = axis == 0 ? 1.0f : 0.0f;
                    bestNormal[1] = axis == 1 ? 1.0f : 0.0f;
                    bestNormal[2] = axis == 2 ? 1.0f : 0.0f;
                }
                if (distanceMin > bestDistance) {
                    bestDistance = distanceMin;
                    bestNormal[0] = axis == 0 ? -1.0f : 0.0f;
                    bestNormal[1] = axis == 1 ? -1.0f : 0.0f;
                    bestNormal[2] = axis == 2 ? -1.0f : 0.0f;
                }
            }
            for (uint32_t p = 0; p < brush.planeCount && !outside; ++p) {
                const float* plane = &brushPlanes[(brush.firstPlane + p) * 4];
                const float distance = plane[0] * position[0] + plane[1] * position[1]
                    + plane[2] * position[2] - plane[3] - radius;
                if (distance > 0.0f) {
                    outside = true;
                    break;
                }
                if (distance > bestDistance) {
                    bestDistance = distance;
                    bestNormal[0] = plane[0];
                    bestNormal[1] = plane[1];
                    bestNormal[2] = plane[2];
                }
            }
            if (!outside && bestDistance > -1e29f) {
                for (int axis = 0; axis < 3; ++axis) {
                    position[axis] -= bestNormal[axis] * bestDistance;
                }
                pushed = true;
                moved = true;
            }
        }
        if (!pushed) {
            break;
        }
    }
    return moved;
}

float QueryWorldGround(const KisakWorldHeightfield& field, float x, float y, float maxZ) {
    if (!field.valid) {
        return -1e30f;
    }
    const int cellX = static_cast<int>((x - field.originX) / field.cellSize);
    const int cellY = static_cast<int>((y - field.originY) / field.cellSize);
    if (cellX < 0 || cellY < 0
        || cellX >= static_cast<int>(field.width) || cellY >= static_cast<int>(field.height)) {
        return -1e30f;
    }
    const uint32_t cellIndex = static_cast<uint32_t>(cellY) * field.width + cellX;
    float best = -1e30f;
    for (uint32_t at = field.cellStart[cellIndex]; at < field.cellStart[cellIndex + 1]; ++at) {
        const float height = field.heights[at];
        if (height <= maxZ && height > best) {
            best = height;
        }
    }
    return best;
}

std::string DescribeWorldScene(const KisakWorldScene& scene) {
    std::ostringstream out;
    if (!scene.error.empty()) {
        out << "monde invalide: " << scene.error;
        return out.str();
    }
    size_t decodedLightmaps = 0;
    for (const KisakWorldLightmap& page : scene.lightmaps) {
        if (page.primary.valid && page.secondary.valid) {
            ++decodedLightmaps;
        }
    }
    out << "monde '" << scene.name << "': "
        << (scene.skyImage.empty() ? "sans ciel" : ("ciel=" + scene.skyImage)) << ", "
        << "sol=" << scene.ground.heights.size() << " hauteurs, "
        << scene.brushes.size() << " brushes de collision, "
        << scene.vertices.size() / 11 << " sommets, "
        << scene.indices.size() / 3 << " triangles, "
        << scene.surfaces.size() << " surfaces, "
        << scene.models.size() << " modeles ("
        << scene.modelInstances.size() / 12 << " instances, "
        << scene.modelVertices.size() / 9 << " sommets, "
        << scene.modelIndices.size() / 3 << " tris), "
        << scene.textureImages.size() << " textures, "
        << decodedLightmaps << "/" << scene.lightmaps.size() << " lightmaps"
        << ", bounds=(" << scene.mins[0] << "," << scene.mins[1] << "," << scene.mins[2]
        << ")-(" << scene.maxs[0] << "," << scene.maxs[1] << "," << scene.maxs[2] << ")";
    if (scene.hasSpawn) {
        out << ", spawn=(" << scene.spawnOrigin[0] << "," << scene.spawnOrigin[1]
            << "," << scene.spawnOrigin[2] << ") yaw=" << scene.spawnYaw;
    }
    return out.str();
}
