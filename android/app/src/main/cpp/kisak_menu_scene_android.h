#pragma once

#include <memory>
#include <string>
#include <vector>

#include "kisak_zone_loader_android.h"

// Flat draw list built from a loaded menuDef_t: solid and textured quads in
// the engine's virtual 640x480 space, ready for any 2D backend.
struct KisakMenuQuad {
    float x = 0;
    float y = 0;
    float w = 0;
    float h = 0;
    float u0 = 0;
    float v0 = 0;
    float u1 = 1;
    float v1 = 1;
    float color[4] = {1, 1, 1, 1};
    int textureIndex = -1; // -1: solid fill
};

struct KisakMenuHitbox {
    std::string name;
    std::string text;
    std::string action;
    std::string onFocus;
    std::string leaveFocus;
    uint32_t itemRef = 0; // serialized itemDef_s ref inside the menu's zone
    float x = 0;
    float y = 0;
    float w = 0;
    float h = 0;
};

struct KisakMenuScene {
    std::string menuName;
    std::vector<std::string> textureImages; // GL textures to create, by image name
    std::vector<KisakMenuQuad> quads;
    std::vector<KisakMenuHitbox> hitboxes;
    // Zone the menu was found in and its menuDef ref, for running item
    // scripts against the scene (zone lifetime is owned by the menu-zone
    // registry below).
    const KisakZoneLoadResult* menuZone = nullptr;
    uint32_t menuRef = 0;
    float canvasWidth = 854.0f;  // widescreen virtual canvas
    float canvasHeight = 480.0f;
    uint32_t itemCount = 0;
    uint32_t hiddenItemCount = 0;
    uint32_t textGlyphCount = 0;
    uint32_t listBoxItemCount = 0;
    uint32_t listBoxRowCount = 0;
    std::vector<float> listBoxFeeders;
    bool timeDependentExpressions = false;
    std::string error;
};

// Builds the scene for menuName ("" = first menu found). Menus, fonts and
// localize strings may come from different zones (fonts live in
// code_post_gfx, the main menu in ui.ff).
KisakMenuScene BuildMenuScene(
    const std::vector<std::shared_ptr<const KisakZoneLoadResult>>& zones,
    const std::string& menuName
);

std::string DescribeMenuScene(const KisakMenuScene& scene);

// Hand-off point between the startup zone probes and the render thread.
void ClearKisakMenuZones();
void AddKisakMenuZone(std::shared_ptr<const KisakZoneLoadResult> zone);
std::vector<std::shared_ptr<const KisakZoneLoadResult>> GetKisakMenuZones();
uint32_t GetKisakMenuZoneGeneration();
