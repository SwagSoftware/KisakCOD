#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "kisak_iwi_texture_android.h"
#include "kisak_zone_loader_android.h"

// Extracts a drawable static-world scene from a loaded map zone's GfxWorld
// asset: the shared vertex/index pools plus one draw range per world surface,
// grouped by colormap image for texture batching. Spawn position/orientation
// come from the map_ents entity string (info_player_start).

struct KisakWorldDrawSurface {
    uint32_t firstIndex = 0;  // offset into KisakWorldScene::indices
    uint32_t indexCount = 0;
    int textureIndex = -1;    // into textureImages, -1 = untextured
    int lightmapIndex = -1;   // into lightmaps, -1 = unlit (world only)
    // From the material's GfxStateBits (entry chosen through the technique
    // set, lit_sun > lit > emissive > unlit): alpha-test reference (negative
    // = disabled), D3DBLEND src/dst factors when blending, cull disable.
    float alphaTestRef = -1.0f;
    bool blended = false;
    uint8_t srcBlend = 0; // D3DBLEND (5 = SRCALPHA, 9 = DESTCOLOR, ...)
    uint8_t dstBlend = 0;
    bool cullNone = false;
    bool sky = false; // wc/sky* material: drawn by the cubemap sky pass
};

// Walkable-height grid built from the world triangles (normals pointing
// up); per-cell floor heights sorted ascending.
struct KisakWorldHeightfield {
    bool valid = false;
    float originX = 0.0f;
    float originY = 0.0f;
    float cellSize = 40.0f;
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint32_t> cellStart; // width*height + 1 entries
    std::vector<float> heights;
};

// Highest floor at (x, y) that is not above maxZ; returns -1e30f when the
// cell has no candidate.
float QueryWorldGround(const KisakWorldHeightfield& field, float x, float y, float maxZ);

// Convex collision brush from the clipmap: AABB planes plus the non-axial
// sides, 4 floats per plane (normal xyz, dist) in brushPlanes.
struct KisakWorldBrush {
    float mins[3];
    float maxs[3];
    uint32_t firstPlane = 0;
    uint32_t planeCount = 0;
};

// Pushes a sphere at `position` with `radius` out of the solid brushes
// (a few Gauss-Seidel iterations); returns true when the position moved.
bool ResolveWorldCollision(
    const std::vector<KisakWorldBrush>& brushes,
    const std::vector<float>& brushPlanes,
    float position[3],
    float radius);

// Decoded lightmap pair (zone-embedded images): primary is the L8 sun
// visibility mask, secondary the baked radiosity color.
struct KisakWorldLightmap {
    KisakIwiTexture primary;
    KisakIwiTexture secondary;
};

// One unique XModel's LOD0 mesh: draw ranges into the model vertex/index
// pools plus the contiguous instance range that uses it.
struct KisakWorldModelMesh {
    std::string modelName;
    uint32_t firstInstance = 0;
    uint32_t instanceCount = 0;
    std::vector<KisakWorldDrawSurface> surfaces;
};

struct KisakWorldScene {
    std::string name;
    // Interleaved world vertex stream: position xyz, texCoord uv, color
    // rgba, lightmap uv — 11 floats per vertex.
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    // Sorted by textureIndex so consecutive surfaces share texture binds.
    std::vector<KisakWorldDrawSurface> surfaces;
    std::vector<std::string> textureImages;
    // Optional pre-decoded RGBA for textureImages (filled off the render
    // thread so GL init only uploads); empty or invalid entries fall back to
    // on-thread IWI decoding.
    std::vector<KisakIwiTexture> decodedTextures;
    std::vector<KisakWorldLightmap> lightmaps;
    // Sky: colormap image of the wc/sky* material (a cubemap .iwi) plus its
    // decode (filled off the render thread with decodedTextures).
    std::string skyImage;
    KisakIwiCubemap skyCubemap;
    KisakWorldHeightfield ground;
    // Player-blocking clipmap brushes (contents solid | playerclip).
    std::vector<KisakWorldBrush> brushes;
    std::vector<float> brushPlanes;
    // Static models: unique LOD0 meshes (same 9-float vertex layout, model
    // space) plus 12 floats per instance — scaled basis rows X/Y/Z, origin.
    std::vector<float> modelVertices;
    std::vector<uint32_t> modelIndices;
    std::vector<float> modelInstances;
    std::vector<KisakWorldModelMesh> models;
    float mins[3] = {0.0f, 0.0f, 0.0f};
    float maxs[3] = {0.0f, 0.0f, 0.0f};
    float sunColor[3] = {1.0f, 1.0f, 0.9f};
    bool hasSpawn = false;
    float spawnOrigin[3] = {0.0f, 0.0f, 0.0f};
    float spawnYaw = 0.0f;
    std::string error;
};

KisakWorldScene BuildWorldScene(const KisakZoneLoadResult& zone);

std::string DescribeWorldScene(const KisakWorldScene& scene);
