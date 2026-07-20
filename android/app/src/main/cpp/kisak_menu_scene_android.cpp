#include "kisak_menu_scene_android.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstring>
#include <mutex>
#include <sstream>

#include "kisak_menu_expression_android.h"
#include "kisak_menu_state_android.h"

namespace {
// windowDef_t field offsets (32-bit layout, ui_shared.h).
constexpr uint32_t kWinName = 0;
constexpr uint32_t kWinRectX = 4;
constexpr uint32_t kWinRectY = 8;
constexpr uint32_t kWinRectW = 12;
constexpr uint32_t kWinRectH = 16;
constexpr uint32_t kWinStyle = 56;
constexpr uint32_t kWinStaticFlags = 76;
constexpr uint32_t kWinDynamicFlags = 80;
constexpr uint32_t kWinForeColor = 88;
constexpr uint32_t kWinBackColor = 104;
constexpr uint32_t kWinBackground = 152;

// menuDef_t offsets.
constexpr uint32_t kMenuItemCount = 164;
constexpr uint32_t kMenuItems = 280;

// itemDef_s offsets.
constexpr uint32_t kItemTextAlignMode = 196;
constexpr uint32_t kItemDvarTest = 272;
constexpr uint32_t kItemEnableDvar = 280;
constexpr uint32_t kItemDvarFlags = 284;
constexpr uint32_t kItemVisibleExp = 308;
constexpr uint32_t kItemTextExp = 316;
constexpr uint32_t kItemMaterialExp = 324;
constexpr uint32_t kItemRectXExp = 332;
constexpr uint32_t kItemRectYExp = 340;
constexpr uint32_t kItemRectWExp = 348;
constexpr uint32_t kItemRectHExp = 356;
constexpr uint32_t kItemForeColorAExp = 364;
constexpr uint32_t kItemType = 180;
constexpr uint32_t kItemTextAlignX = 200;
constexpr uint32_t kItemTextAlignY = 204;
constexpr uint32_t kItemTextScale = 208;
constexpr uint32_t kItemText = 224;
constexpr uint32_t kItemMouseEnterText = 236;
constexpr uint32_t kItemMouseEnter = 244;
constexpr uint32_t kItemAction = 252;
constexpr uint32_t kItemDvar = 268;
constexpr uint32_t kItemOnFocus = 260;
constexpr uint32_t kItemLeaveFocus = 264;
constexpr uint32_t kItemTypeData = 300;
constexpr uint32_t kItemSpecial = 292;

// editFieldDef_s offsets, also used by ITEM_TYPE_SLIDER.
constexpr uint32_t kEditMinVal = 0;
constexpr uint32_t kEditMaxVal = 4;
constexpr uint32_t kEditDefVal = 8;

// listBoxDef_s offsets.
constexpr uint32_t kListElementWidth = 16;
constexpr uint32_t kListElementHeight = 20;
constexpr uint32_t kListNumColumns = 28;
constexpr uint32_t kListColumns = 32;
constexpr uint32_t kListNoScrollBars = 296;

// Font_s offsets.
constexpr uint32_t kFontPixelHeight = 4;
constexpr uint32_t kFontGlyphCount = 8;
constexpr uint32_t kFontMaterial = 12;
constexpr uint32_t kFontGlyphs = 20;

constexpr uint32_t kStyleShader = 3;
constexpr uint32_t kWindowHorizontalScroll = 0x200000;
constexpr uint32_t kWindowAutoWrapped = 0x800000;

std::mutex g_menuZoneMutex;
std::vector<std::shared_ptr<const KisakZoneLoadResult>> g_menuZones;
uint32_t g_menuZoneGeneration = 0;

struct SceneBuilder {
    const std::vector<std::shared_ptr<const KisakZoneLoadResult>>& zones;
    KisakMenuScene& scene;

    // Zone the menu lives in and zone the chosen font lives in.
    const KisakZoneLoadResult* menuZone = nullptr;
    const KisakZoneLoadResult* fontZone = nullptr;
    uint32_t fontRef = 0;
    int fontTexture = -1;

    SceneBuilder(
        const std::vector<std::shared_ptr<const KisakZoneLoadResult>>& z,
        KisakMenuScene& s
    ) : zones(z), scene(s) {}

    int TextureFor(const std::string& imageName) {
        if (imageName.empty()) {
            return -1;
        }
        for (size_t i = 0; i < scene.textureImages.size(); ++i) {
            if (scene.textureImages[i] == imageName) {
                return static_cast<int>(i);
            }
        }
        scene.textureImages.push_back(imageName);
        return static_cast<int>(scene.textureImages.size() - 1);
    }

    // material ref -> first texture def's image name, within one zone.
    static std::string MaterialImage(const KisakZoneLoadResult& zone, uint32_t materialRef) {
        KisakZoneView view(zone);
        if (!view.ValidRef(materialRef, 80)) {
            return {};
        }
        const uint32_t textureTable = view.U32(materialRef, 68);
        const uint8_t textureCount = view.U8(materialRef, 58);
        if (textureCount == 0 || !view.ValidRef(textureTable, 12)) {
            return {};
        }
        const uint32_t imageRef = view.U32(textureTable, 8);
        if (!view.ValidRef(imageRef, 36)) {
            return {};
        }
        return view.StrAt(imageRef, 32);
    }

    // Material asset lookup by name across all loaded zones (materialExp
    // results are material names like "gradient_fadein" or
    // "levelshot_killhouse"). 2D UI materials that only ship as loose IWI
    // images (gradient_fadein, button_highlight_end...) fall back to the
    // same-named image, which the renderer streams from the IWDs.
    std::string MaterialImageByName(const std::string& materialName) {
        if (materialName.empty()) {
            return {};
        }
        for (const auto& zone : zones) {
            KisakZoneView view(*zone);
            for (const auto& [type, ref] : zone->assetRefs) {
                if (type == 0x4 && view.ValidRef(ref, 80)
                    && view.StrAt(ref, 0) == materialName) {
                    return MaterialImage(*zone, ref);
                }
            }
        }
        return materialName;
    }

    std::string Localize(const std::string& key) {
        for (const auto& zone : zones) {
            for (const KisakLoadedLocalize& entry : zone->localizeEntries) {
                if (entry.name == key) {
                    return entry.value;
                }
            }
        }
        return key;
    }

    std::string ResolveText(const std::string& raw) {
        if (!raw.empty() && raw.front() == '@') {
            return Localize(raw.substr(1));
        }
        return raw;
    }

    std::string YesNoText(const KisakZoneView& view, uint32_t itemRef) {
        const std::string dvar = view.StrAt(itemRef, kItemDvar);
        const float value = dvar.empty() ? 0.0f : static_cast<float>(atof(GetKisakUiDvar(dvar).c_str()));
        return Localize(value == 0.0f ? "EXE_NO" : "EXE_YES");
    }

    std::string MultiText(const KisakZoneView& view, uint32_t itemRef) {
        const std::string dvar = view.StrAt(itemRef, kItemDvar);
        const uint32_t multi = view.U32(itemRef, kItemTypeData);
        if (dvar.empty() || !view.ValidRef(multi, 392)) {
            return "<dvarStrList or dvarFloatList not set>";
        }
        const int count = static_cast<int>(view.U32(multi, 384));
        if (count <= 0 || count > 32) {
            return "";
        }
        const bool strDef = view.U32(multi, 388) != 0;
        const std::string currentValue = GetKisakUiDvar(dvar);
        if (strDef) {
            for (int i = 0; i < count; ++i) {
                if (strcasecmp(currentValue.c_str(), view.StrAt(multi, 128 + i * 4).c_str()) == 0) {
                    return ResolveText(view.StrAt(multi, i * 4));
                }
            }
        } else {
            const float value = static_cast<float>(atof(currentValue.c_str()));
            for (int i = 0; i < count; ++i) {
                if (value == view.F32(multi, 256 + i * 4)) {
                    return ResolveText(view.StrAt(multi, i * 4));
                }
            }
        }
        return "";
    }

    KisakExprEnv ExprEnv() const {
        KisakExprEnv env;
        env.zones = &zones;
        env.milliseconds = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
        return env;
    }

    void TrackTimeDependency(const KisakZoneView& view, uint32_t stmtRef) {
        if (KisakStatementUsesMilliseconds(view, stmtRef)) {
            scene.timeDependentExpressions = true;
        }
    }

    bool StatementFloat(const KisakZoneView& view, uint32_t stmtRef, float* out) const {
        KisakExprValue value;
        if (!EvaluateKisakStatement(view, stmtRef, ExprEnv(), &value)) {
            return false;
        }
        *out = value.AsFloat();
        return true;
    }

    std::array<float, 4> EffectiveForeColor(const KisakZoneView& view, uint32_t itemRef) const {
        std::array<float, 4> color{};
        for (int i = 0; i < 4; ++i) {
            color[i] = view.F32(itemRef, kWinForeColor + i * 4);
        }
        float alpha = 0.0f;
        const bool alphaFromExp = StatementFloat(view, itemRef + kItemForeColorAExp, &alpha);
        if (alphaFromExp) {
            color[3] = std::clamp(alpha, 0.0f, 1.0f);
        } else if (color[3] <= 0.01f) {
            color = {1.0f, 1.0f, 1.0f, 1.0f};
        }
        return color;
    }

    // Item_EnableShowViaDvar (ui_utils.cpp:106): enableDvar lists values;
    // when the tested dvar matches one, the item shows/enables iff `flag` is
    // set in dvarFlags, otherwise iff it is clear.
    bool ItemEnabledViaDvar(const KisakZoneView& view, uint32_t itemRef, uint32_t flag) {
        const std::string enableDvar = view.StrAt(itemRef, kItemEnableDvar);
        const std::string dvarTest = view.StrAt(itemRef, kItemDvarTest);
        if (enableDvar.empty() || dvarTest.empty()) {
            return true;
        }
        const uint32_t dvarFlags = view.U32(itemRef, kItemDvarFlags);
        const std::string testValue = GetKisakUiDvar(dvarTest);
        std::istringstream in(enableDvar);
        std::string token;
        while (in >> token) {
            if (token == ";") {
                continue;
            }
            if (token.size() >= 2 && token.front() == '"' && token.back() == '"') {
                token = token.substr(1, token.size() - 2);
            }
            if (strcasecmp(token.c_str(), testValue.c_str()) == 0) {
                return (flag & dvarFlags) != 0;
            }
        }
        return (flag & dvarFlags) == 0;
    }

    bool ItemVisible(const KisakZoneView& view, uint32_t itemRef) {
        if (!KisakItemDynamicVisible(*menuZone, itemRef)) {
            return false;
        }
        if ((view.U32(itemRef, kItemDvarFlags) & 3) != 0
            && !ItemEnabledViaDvar(view, itemRef, 1)) {
            return false;
        }
        const uint32_t visibleEntries = view.U32(itemRef + kItemVisibleExp, 0);
        TrackTimeDependency(view, itemRef + kItemVisibleExp);
        if (visibleEntries != 0
            && !KisakStatementTrue(view, itemRef + kItemVisibleExp, ExprEnv())) {
            return false;
        }
        return true;
    }

    void PickFont() {
        const char* preferred[] = {"fonts/normalFont", "fonts/boldFont", "fonts/bigDevFont"};
        for (const char* want : preferred) {
            for (const auto& zone : zones) {
                KisakZoneView view(*zone);
                for (const auto& [type, ref] : zone->assetRefs) {
                    if (type != 0x13 || !view.ValidRef(ref, 24)) {
                        continue;
                    }
                    if (view.StrAt(ref, 0) == want) {
                        fontZone = zone.get();
                        fontRef = ref;
                        fontTexture = TextureFor(MaterialImage(*zone, view.U32(ref, kFontMaterial)));
                        return;
                    }
                }
            }
        }
        // Any font at all.
        for (const auto& zone : zones) {
            KisakZoneView view(*zone);
            for (const auto& [type, ref] : zone->assetRefs) {
                if (type == 0x13 && view.ValidRef(ref, 24)) {
                    fontZone = zone.get();
                    fontRef = ref;
                    fontTexture = TextureFor(MaterialImage(*zone, view.U32(ref, kFontMaterial)));
                    return;
                }
            }
        }
    }

    // Negative extents are authored intentionally (mirrored gradients on the
    // act popups' folder art): the engine emits the quad from x+w to x with
    // the texture flipped, so normalize and mirror the UVs.
    void EmitRect(float x, float y, float w, float h, const float* rgba, int texture) {
        KisakMenuQuad quad;
        if (w < 0) {
            x += w;
            w = -w;
            std::swap(quad.u0, quad.u1);
        }
        if (h < 0) {
            y += h;
            h = -h;
            std::swap(quad.v0, quad.v1);
        }
        quad.x = x;
        quad.y = y;
        quad.w = w;
        quad.h = h;
        std::copy_n(rgba, 4, quad.color);
        quad.textureIndex = texture;
        scene.quads.push_back(quad);
    }

    // ScrPlace_ApplyRect (client/screen_placement.cpp:406) on a 854x480
    // canvas whose virtual-to-real scale is 1: 0=subscreen (the centered
    // 640-wide 4:3 area), 1=left/top edge, 2=center anchor (additive),
    // 3=right/bottom edge, 4=stretch to fullscreen, 5/6=no scale.
    void ApplyRectX(const KisakZoneView& view, uint32_t windowRef, float& x, float& w) const {
        switch (view.U32(windowRef, 20)) { // horzAlign
            case 2:
            case 7:
                x += scene.canvasWidth * 0.5f;
                break;
            case 3:
                x += scene.canvasWidth;
                break;
            case 1:
            case 5:
            case 6:
                break;
            case 4: {
                const float s = scene.canvasWidth / 640.0f;
                x *= s;
                w *= s;
                break;
            }
            default: // 0: subscreen
                x += (scene.canvasWidth - 640.0f) * 0.5f;
                break;
        }
    }

    float AlignOffsetY(const KisakZoneView& view, uint32_t windowRef) const {
        switch (view.U32(windowRef, 24)) { // vertAlign
            case 2:
            case 7:
                return scene.canvasHeight * 0.5f;
            case 3:
                return scene.canvasHeight;
            default: // 0/1: top (the 480-high canvas has no vertical letterbox)
                return 0.0f;
        }
    }

    void EmitWindowBackgroundAt(
        const KisakZoneView& view,
        uint32_t windowRef,
        float x,
        float y,
        float w,
        float h,
        const float* foreColorOverride,
        const std::string& materialOverride = {}
    ) {
        if (w == 0 || h == 0) {
            return;
        }

        float backColor[4];
        for (int i = 0; i < 4; ++i) {
            backColor[i] = view.F32(windowRef, kWinBackColor + i * 4);
        }
        const uint32_t style = view.U32(windowRef, kWinStyle);
        const std::string image = materialOverride.empty()
            ? MaterialImage(*menuZone, view.U32(windowRef, kWinBackground))
            : MaterialImageByName(materialOverride);

        // Window_Paint (ui_shared.cpp:4711): SHADER styles tint the material
        // with foreColor when the 0x10000 dynamic flag is set (white
        // otherwise); FILLED (1) uses backColor for both image and fill.
        if (style >= kStyleShader) {
            if (image.empty()) {
                return;
            }
            float tint[4] = {1, 1, 1, 1};
            if ((view.U32(windowRef, kWinDynamicFlags) & 0x10000) != 0) {
                if (foreColorOverride != nullptr) {
                    std::copy_n(foreColorOverride, 4, tint);
                } else {
                    for (int i = 0; i < 4; ++i) {
                        tint[i] = view.F32(windowRef, kWinForeColor + i * 4);
                    }
                }
            }
            if (tint[3] > 0.004f) {
                EmitRect(x, y, w, h, tint, TextureFor(image));
            }
        } else if (style == 1) {
            if (!image.empty()) {
                EmitRect(x, y, w, h, backColor, TextureFor(image));
            } else if (backColor[3] > 0.01f) {
                EmitRect(x, y, w, h, backColor, -1);
            }
        } else if (backColor[3] > 0.01f && style == 0 && windowRef == scene.menuRef) {
            // menuDef windows keep their plain fill even with style 0 (the
            // engine clears fullscreen menus through a different path).
            EmitRect(x, y, w, h, backColor, -1);
        }
    }

    void EmitWindowBackground(
        const KisakZoneView& view,
        uint32_t windowRef,
        const std::string& materialOverride = {}
    ) {
        float x = view.F32(windowRef, kWinRectX);
        float w = view.F32(windowRef, kWinRectW);
        ApplyRectX(view, windowRef, x, w);
        const float y = view.F32(windowRef, kWinRectY) + AlignOffsetY(view, windowRef);
        const float h = view.F32(windowRef, kWinRectH);
        EmitWindowBackgroundAt(view, windowRef, x, y, w, h, nullptr, materialOverride);
    }

    void EmitText(const std::string& text, float x, float baseline, float scale, const float* rgba) {
        if (fontRef == 0 || text.empty()) {
            return;
        }
        KisakZoneView view(*fontZone);
        const uint32_t glyphs = view.U32(fontRef, kFontGlyphs);
        const uint32_t glyphCount = view.U32(fontRef, kFontGlyphCount);
        const uint32_t pixelHeight = view.U32(fontRef, kFontPixelHeight);
        if (!view.ValidRef(glyphs, glyphCount * 24) || pixelHeight == 0) {
            return;
        }

        const float glyphScale = scale * 48.0f / static_cast<float>(pixelHeight);
        float penX = x;
        for (unsigned char ch : text) {
            uint32_t found = 0;
            for (uint32_t g = 0; g < glyphCount; ++g) {
                if (view.U16(glyphs, g * 24) == ch) {
                    found = glyphs + g * 24;
                    break;
                }
            }
            if (found == 0) {
                penX += 6.0f * glyphScale;
                continue;
            }
            const auto x0 = static_cast<int8_t>(view.U8(found, 2));
            const auto y0 = static_cast<int8_t>(view.U8(found, 3));
            const uint8_t dx = view.U8(found, 4);
            const uint8_t pixelWidth = view.U8(found, 5);
            const uint8_t glyphHeight = view.U8(found, 6);

            KisakMenuQuad quad;
            quad.x = penX + x0 * glyphScale;
            quad.y = baseline + y0 * glyphScale;
            quad.w = pixelWidth * glyphScale;
            quad.h = glyphHeight * glyphScale;
            quad.u0 = view.F32(found, 8);
            quad.v0 = view.F32(found, 12);
            quad.u1 = view.F32(found, 16);
            quad.v1 = view.F32(found, 20);
            std::copy_n(rgba, 4, quad.color);
            quad.textureIndex = fontTexture;
            scene.quads.push_back(quad);
            ++scene.textGlyphCount;
            penX += dx * glyphScale;
        }
    }

    float MeasureTextWidth(const std::string& text, float scale) const {
        if (fontRef == 0 || text.empty()) {
            return 0.0f;
        }
        KisakZoneView view(*fontZone);
        const uint32_t glyphs = view.U32(fontRef, kFontGlyphs);
        const uint32_t glyphCount = view.U32(fontRef, kFontGlyphCount);
        const uint32_t pixelHeight = view.U32(fontRef, kFontPixelHeight);
        if (!view.ValidRef(glyphs, glyphCount * 24) || pixelHeight == 0) {
            return text.size() * scale * 24.0f;
        }

        const float glyphScale = scale * 48.0f / static_cast<float>(pixelHeight);
        float width = 0.0f;
        for (unsigned char ch : text) {
            uint32_t found = 0;
            for (uint32_t g = 0; g < glyphCount; ++g) {
                if (view.U16(glyphs, g * 24) == ch) {
                    found = glyphs + g * 24;
                    break;
                }
            }
            width += found == 0 ? 6.0f * glyphScale : view.U8(found, 4) * glyphScale;
        }
        return width;
    }

    std::vector<std::string> WrapTextToWidth(const std::string& text, float scale, float maxWidth) const {
        std::vector<std::string> lines;
        if (text.empty()) {
            return lines;
        }
        if (maxWidth <= 1.0f) {
            lines.push_back(text);
            return lines;
        }

        std::string line;
        size_t pos = 0;
        while (pos < text.size()) {
            while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos]))) {
                ++pos;
            }
            if (pos >= text.size()) {
                break;
            }
            size_t end = pos;
            while (end < text.size() && !std::isspace(static_cast<unsigned char>(text[end]))) {
                ++end;
            }
            const std::string word = text.substr(pos, end - pos);
            const std::string candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && MeasureTextWidth(candidate, scale) > maxWidth) {
                lines.push_back(line);
                line = word;
            } else {
                line = candidate;
            }
            pos = end;
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
        if (lines.empty()) {
            lines.push_back(text);
        }
        return lines;
    }

    std::string ClampTextToChars(const std::string& text, int maxChars) const {
        if (maxChars <= 0 || maxChars == 0x7FFFFFFF
            || text.size() <= static_cast<size_t>(maxChars)) {
            return text;
        }
        return text.substr(0, static_cast<size_t>(maxChars));
    }

    float TextAlignAdjustment(int alignment, float width, float textWidth) const {
        if (alignment == 1) {
            return std::max((width - textWidth) * 0.5f, 0.0f);
        }
        if (alignment == 2) {
            return std::max(width - textWidth, 0.0f);
        }
        return 0.0f;
    }

    float RectPlacementX(int alignX, float x0, float containerWidth, float selfWidth) const {
        if (alignX == 1) {
            return (containerWidth - selfWidth) * 0.5f + x0;
        }
        if (alignX == 2) {
            return containerWidth - selfWidth + x0;
        }
        return x0;
    }

    float RectPlacementY(int alignY, float y0, float containerHeight, float selfHeight) const {
        if (alignY == 12) {
            return containerHeight - selfHeight + y0;
        }
        if (alignY == 8) {
            return (containerHeight - selfHeight) * 0.5f + y0;
        }
        return y0;
    }

    bool ItemIsTextFieldType(uint32_t itemType) const {
        return itemType == 4 || itemType == 9 || itemType == 0x10
            || itemType == 0x11 || itemType == 0x12;
    }

    void EmitSlider(
        const KisakZoneView& view,
        uint32_t itemRef,
        float x,
        float y,
        float w,
        float h,
        const float* foreColor
    ) {
        const uint32_t editDef = view.U32(itemRef, kItemTypeData);
        if (!view.ValidRef(editDef, 32)) {
            return;
        }
        const uint32_t alignMode = view.U32(itemRef, kItemTextAlignMode);
        const float sliderX = RectPlacementX(
            static_cast<int>(alignMode & 3),
            x + view.F32(itemRef, kItemTextAlignX),
            w,
            96.0f
        );
        const float sliderY = RectPlacementY(
            static_cast<int>(alignMode & 0xC),
            y + view.F32(itemRef, kItemTextAlignY),
            h,
            16.0f
        );

        const float minVal = view.F32(editDef, kEditMinVal);
        const float maxVal = view.F32(editDef, kEditMaxVal);
        const float defVal = view.F32(editDef, kEditDefVal);
        const std::string dvar = view.StrAt(itemRef, kItemDvar);
        const std::string current = dvar.empty() ? std::string() : GetKisakUiDvar(dvar);
        float value = current.empty() ? defVal : static_cast<float>(atof(current.c_str()));
        if (maxVal > minVal) {
            value = std::clamp(value, minVal, maxVal);
        }
        const float t = maxVal > minVal ? (value - minVal) / (maxVal - minVal) : 0.0f;
        const float thumbCenter = sliderX + 6.0f + std::clamp(t, 0.0f, 1.0f) * 84.0f;

        EmitRect(sliderX, sliderY, 96.0f, 16.0f, foreColor, TextureFor(MaterialImageByName("ui_slider2")));
        EmitRect(thumbCenter - 5.0f, sliderY - 2.0f, 10.0f, 20.0f,
                 foreColor, TextureFor(MaterialImageByName("ui_sliderbutt_1")));
    }

    void EmitListBoxText(
        const KisakZoneView& view,
        uint32_t itemRef,
        float feederId,
        int row,
        int column,
        int maxChars,
        int alignment,
        float x,
        float y,
        float w,
        float h,
        const float* color
    ) {
        std::string text = ClampTextToChars(
            ResolveText(KisakMenuFeederItemText(feederId, row, column)),
            maxChars
        );
        if (text.empty()) {
            return;
        }
        const float scale = view.F32(itemRef, kItemTextScale);
        const float tx = x + TextAlignAdjustment(alignment, w, MeasureTextWidth(text, scale))
            + view.F32(itemRef, kItemTextAlignX);
        const float baseline = y + h + view.F32(itemRef, kItemTextAlignY);
        EmitText(text, tx, baseline, scale, color);
    }

    uint32_t EmitListBox(
        const KisakZoneView& view,
        uint32_t itemRef,
        float x,
        float y,
        float w,
        float h,
        const float* foreColor
    ) {
        const uint32_t listBox = view.U32(itemRef, kItemTypeData);
        if (!view.ValidRef(listBox, 340)) {
            return 0;
        }
        const float feederId = view.F32(itemRef, kItemSpecial);
        const int count = KisakMenuFeederCount(feederId);
        if (count <= 0) {
            return 0;
        }
        const bool horizontal = (view.U32(itemRef, kWinStaticFlags) & kWindowHorizontalScroll) != 0;
        const bool noScrollBars = view.U32(listBox, kListNoScrollBars) != 0;
        const float elementW = std::max(view.F32(listBox, kListElementWidth), 1.0f);
        const float elementH = std::max(view.F32(listBox, kListElementHeight), 1.0f);
        const int cursor = KisakMenuListBoxCursor(*menuZone, itemRef);
        const int start = KisakMenuListBoxStart(*menuZone, itemRef);

        const float rowAreaW = horizontal || noScrollBars ? w - 2.0f : w - 20.0f;
        const float rowAreaH = horizontal || noScrollBars ? h - 2.0f : h - 2.0f;
        const int visibleRows = horizontal
            ? std::max(static_cast<int>(rowAreaW / elementW), 1)
            : std::max(static_cast<int>(rowAreaH / elementH), 1);

        float backColor[4];
        float highlightColor[4];
        for (int i = 0; i < 4; ++i) {
            backColor[i] = view.F32(itemRef, kWinBackColor + i * 4);
            highlightColor[i] = foreColor[i];
        }
        if (backColor[3] <= 0.01f) {
            backColor[0] = backColor[1] = backColor[2] = 0.05f;
            backColor[3] = 0.18f;
        }
        highlightColor[3] = std::max(highlightColor[3], 0.28f);

        uint32_t emittedRows = 0;
        for (int visual = 0; visual < visibleRows && start + visual < count; ++visual) {
            const int row = start + visual;
            ++emittedRows;
            const float rowX = x + 1.0f + (horizontal ? visual * elementW : 0.0f);
            const float rowY = y + 1.0f + (horizontal ? 0.0f : visual * elementH);
            const float drawW = horizontal ? elementW - 2.0f : rowAreaW;
            const float drawH = elementH;
            EmitRect(rowX + 1.0f, rowY + 2.0f, drawW, drawH, backColor, -1);
            if (row == cursor) {
                EmitRect(rowX + 2.0f, rowY + 3.0f, std::max(drawW - 2.0f, 1.0f),
                         std::max(drawH - 1.0f, 1.0f), highlightColor, -1);
            }

            const int numColumns = static_cast<int>(view.U32(listBox, kListNumColumns));
            if (numColumns <= 0) {
                EmitListBoxText(view, itemRef, feederId, row, 0, 0x7FFFFFFF, 0,
                                rowX, rowY, drawW, drawH, foreColor);
            } else {
                for (int col = 0; col < numColumns && col < 16; ++col) {
                    const uint32_t columnRef = listBox + kListColumns + static_cast<uint32_t>(col) * 16;
                    EmitListBoxText(
                        view,
                        itemRef,
                        feederId,
                        row,
                        col,
                        static_cast<int>(view.U32(columnRef, 8)),
                        static_cast<int>(view.U32(columnRef, 12)),
                        rowX + static_cast<float>(view.U32(columnRef, 0)),
                        rowY,
                        static_cast<float>(view.U32(columnRef, 4)),
                        drawH,
                        foreColor
                    );
                }
            }
        }

        if (!noScrollBars && !horizontal) {
            const float railColor[4] = {0.0f, 0.0f, 0.0f, 0.35f};
            const float thumbColor[4] = {0.78f, 0.72f, 0.58f, 0.75f};
            const float sx = x + w - 17.0f;
            EmitRect(sx, y + 17.0f, 16.0f, std::max(h - 34.0f, 1.0f), railColor, -1);
            const float maxStart = static_cast<float>(std::max(count - visibleRows + 1, 1));
            const float thumbY = y + 17.0f + (h - 50.0f) * (static_cast<float>(start) / maxStart);
            EmitRect(sx + 2.0f, thumbY, 12.0f, 16.0f, thumbColor, -1);
        }
        return emittedRows;
    }

    void PushItemHitbox(
        const KisakZoneView& view,
        uint32_t itemRef,
        const std::string& text,
        float x,
        float y,
        float w,
        float h
    ) {
        if (KisakItemIsDecoration(*menuZone, itemRef)) {
            return;
        }
        KisakMenuHitbox hitbox;
        hitbox.name = view.StrAt(itemRef, kWinName);
        hitbox.text = text;
        hitbox.action = view.StrAt(itemRef, kItemAction);
        hitbox.onFocus = view.StrAt(itemRef, kItemOnFocus);
        hitbox.leaveFocus = view.StrAt(itemRef, kItemLeaveFocus);
        if (hitbox.onFocus.empty()) {
            hitbox.onFocus = view.StrAt(itemRef, kItemMouseEnter);
        }
        if (hitbox.onFocus.empty()) {
            hitbox.onFocus = view.StrAt(itemRef, kItemMouseEnterText);
        }
        const uint32_t itemType = view.U32(itemRef, kItemType);
        if (hitbox.action.empty() && hitbox.onFocus.empty()
            && itemType != 0x6 && itemType != 0xA && itemType != 0xE
            && !ItemIsTextFieldType(itemType)) {
            return;
        }
        hitbox.itemRef = itemRef;
        hitbox.x = std::max(x, 0.0f);
        hitbox.y = std::max(y, 0.0f);
        hitbox.w = std::max(w, 48.0f);
        hitbox.h = std::max(h, 18.0f);
        scene.hitboxes.push_back(hitbox);
    }

    void EmitItem(const KisakZoneView& view, uint32_t itemRef) {
        if (!ItemVisible(view, itemRef)) {
            ++scene.hiddenItemCount;
            return;
        }

        float rawX = view.F32(itemRef, kWinRectX);
        float rawY = view.F32(itemRef, kWinRectY);
        float rawW = view.F32(itemRef, kWinRectW);
        float rawH = view.F32(itemRef, kWinRectH);
        float exprValue = 0.0f;
        TrackTimeDependency(view, itemRef + kItemRectXExp);
        if (StatementFloat(view, itemRef + kItemRectXExp, &exprValue)) {
            rawX = exprValue + view.F32(scene.menuRef, kWinRectX);
        }
        TrackTimeDependency(view, itemRef + kItemRectYExp);
        if (StatementFloat(view, itemRef + kItemRectYExp, &exprValue)) {
            rawY = exprValue + view.F32(scene.menuRef, kWinRectY);
        }
        TrackTimeDependency(view, itemRef + kItemRectWExp);
        if (StatementFloat(view, itemRef + kItemRectWExp, &exprValue)) {
            rawW = exprValue;
        }
        TrackTimeDependency(view, itemRef + kItemRectHExp);
        if (StatementFloat(view, itemRef + kItemRectHExp, &exprValue)) {
            rawH = exprValue;
        }
        TrackTimeDependency(view, itemRef + kItemForeColorAExp);
        TrackTimeDependency(view, itemRef + kItemMaterialExp);
        TrackTimeDependency(view, itemRef + kItemTextExp);

        float x = rawX;
        float w = rawW;
        ApplyRectX(view, itemRef, x, w);
        const float y = rawY + AlignOffsetY(view, itemRef);
        const float h = rawH;
        const std::array<float, 4> foreColor = EffectiveForeColor(view, itemRef);
        EmitWindowBackgroundAt(
            view,
            itemRef,
            x,
            y,
            w,
            h,
            foreColor.data(),
            KisakStatementString(view, itemRef + kItemMaterialExp, ExprEnv())
        );

        const uint32_t itemType = view.U32(itemRef, kItemType);
        if (itemType == 0x6) {
            ++scene.listBoxItemCount;
            scene.listBoxFeeders.push_back(view.F32(itemRef, kItemSpecial));
            scene.listBoxRowCount += EmitListBox(view, itemRef, x, y, w, h, foreColor.data());
            PushItemHitbox(view, itemRef, "", x, y, w, h);
            return;
        }
        if (itemType == 0xA) {
            EmitSlider(view, itemRef, x, y, w, h, foreColor.data());
            PushItemHitbox(view, itemRef, "", x, y, w, h);
            return;
        }

        std::string text;
        if (ItemIsTextFieldType(itemType)) {
            text = GetKisakUiDvar(view.StrAt(itemRef, kItemDvar));
        } else if (itemType == 0xB) {
            text = YesNoText(view, itemRef);
        } else if (itemType == 0xC) {
            text = MultiText(view, itemRef);
        } else if (itemType == 0xE) {
            text = ResolveText(KisakMenuBindText(*menuZone, itemRef));
        } else {
            text = ResolveText(view.StrAt(itemRef, kItemText));
        }
        if (text.empty()) {
            text = ResolveText(KisakStatementString(view, itemRef + kItemTextExp, ExprEnv()));
        }
        if (text.empty()) {
            // Invisible click zones (image buttons, list rows) still hit-test
            // on their window rect.
            PushItemHitbox(view, itemRef, text, x, y, w, h);
            return;
        }

        // Item_SetTextExtents (ui_shared.cpp:5232): textalignx/y offset the
        // text inside the window rect; textAlignMode & 3 aligns horizontally
        // against the text width (1=center, 2=right) and & 0xC picks the
        // baseline placement (0=raw, 4=+height, 8=centered, 12=bottom).
        // UI_TextHeight is scale * 48 for every font.
        const float scale = view.F32(itemRef, kItemTextScale);
        const float alignX = view.F32(itemRef, kItemTextAlignX);
        const float alignY = view.F32(itemRef, kItemTextAlignY);
        const uint32_t alignMode = view.U32(itemRef, kItemTextAlignMode);
        const float textHeight = scale * 48.0f;
        const float textWidth = MeasureTextWidth(text, scale);

        if ((view.U32(itemRef, kWinStaticFlags) & kWindowAutoWrapped) != 0) {
            const std::vector<std::string> lines = WrapTextToWidth(text, scale, w);
            const float lineStep = textHeight + 5.0f;
            float startX = x + alignX;
            if ((alignMode & 3) == 1) {
                startX = x + alignX + w * 0.5f;
            } else if ((alignMode & 3) == 2) {
                startX = x + alignX + w;
            }
            float firstBaseline = y + alignY;
            switch (alignMode & 0xC) {
                case 4: firstBaseline += textHeight; break;
                case 8: firstBaseline += (h + textHeight) * 0.5f; break;
                case 12: firstBaseline += h; break;
                default: break;
            }
            for (size_t i = 0; i < lines.size(); ++i) {
                const std::string& line = lines[i];
                float lineX = startX;
                const float lineWidth = MeasureTextWidth(line, scale);
                if ((alignMode & 3) == 1) {
                    lineX -= lineWidth * 0.5f;
                } else if ((alignMode & 3) == 2) {
                    lineX -= lineWidth;
                }
                EmitText(line, lineX, firstBaseline + static_cast<float>(i) * lineStep,
                         scale, foreColor.data());
            }
            PushItemHitbox(view, itemRef, text, x, y, w, h);
            return;
        }

        float tx = alignX;
        if ((alignMode & 3) == 1) {
            tx += (w - textWidth) * 0.5f;
        } else if ((alignMode & 3) == 2) {
            tx += w - textWidth;
        }
        float ty = alignY;
        switch (alignMode & 0xC) {
            case 4: ty += textHeight; break;
            case 8: ty += (h + textHeight) * 0.5f; break;
            case 12: ty += h; break;
            default: break;
        }
        const float baseline = y + ty;
        const float textX = x + tx;
        EmitText(text, textX, baseline, scale, foreColor.data());

        // The engine hit-tests the window rect (Rect_ContainsPoint), not the
        // text extents.
        PushItemHitbox(view, itemRef, text, x, y, w, h);
    }

    void Build(const KisakZoneLoadResult& zone, uint32_t menuRef) {
        menuZone = &zone;
        scene.menuZone = &zone;
        scene.menuRef = menuRef;
        KisakZoneView view(zone);
        scene.menuName = view.StrAt(menuRef, kWinName);
        PickFont();
        EmitWindowBackground(view, menuRef);

        const uint32_t itemCount = view.U32(menuRef, kMenuItemCount);
        const uint32_t items = view.U32(menuRef, kMenuItems);
        scene.itemCount = itemCount;
        for (uint32_t i = 0; i < itemCount; ++i) {
            const uint32_t itemRef = view.U32(items, i * 4);
            if (view.ValidRef(itemRef, 372)) {
                EmitItem(view, itemRef);
            }
        }
    }
};
} // namespace

KisakMenuScene BuildMenuScene(
    const std::vector<std::shared_ptr<const KisakZoneLoadResult>>& zones,
    const std::string& menuName
) {
    KisakMenuScene scene;

    const KisakZoneLoadResult* targetZone = nullptr;
    uint32_t target = 0;
    const KisakZoneLoadResult* firstZone = nullptr;
    uint32_t firstMenu = 0;
    for (const auto& zone : zones) {
        KisakZoneView view(*zone);
        for (const auto& [type, ref] : zone->assetRefs) {
            if (type != 0x14) { // menulist
                continue;
            }
            const uint32_t count = view.U32(ref, 4);
            const uint32_t menus = view.U32(ref, 8);
            for (uint32_t i = 0; i < count; ++i) {
                const uint32_t menu = view.U32(menus, i * 4);
                if (!view.ValidRef(menu, 284)) {
                    continue;
                }
                if (firstMenu == 0) {
                    firstZone = zone.get();
                    firstMenu = menu;
                }
                if (!menuName.empty() && view.StrAt(menu, 0) == menuName) {
                    targetZone = zone.get();
                    target = menu;
                }
            }
        }
    }
    if (target == 0) {
        targetZone = firstZone;
        target = firstMenu;
    }
    if (target == 0 || targetZone == nullptr) {
        scene.error = "aucun menu dans les zones chargees";
        return scene;
    }

    SceneBuilder builder(zones, scene);
    builder.Build(*targetZone, target);
    return scene;
}

std::string DescribeMenuScene(const KisakMenuScene& scene) {
    std::ostringstream out;
    if (!scene.error.empty()) {
        out << "scene menu invalide: " << scene.error;
        return out.str();
    }
    out << "menu '" << scene.menuName << "': " << scene.quads.size() << " quads ("
        << scene.textGlyphCount << " glyphes), " << scene.itemCount << " items ("
        << scene.hiddenItemCount << " caches), " << scene.hitboxes.size()
        << " zones tactiles, listboxes=" << scene.listBoxItemCount
        << " rows=" << scene.listBoxRowCount
        << ", timeExpr=" << (scene.timeDependentExpressions ? 1 : 0)
        << " feeders=[";
    for (size_t i = 0; i < scene.listBoxFeeders.size(); ++i) {
        out << (i ? "," : "") << scene.listBoxFeeders[i];
    }
    out << "], textures=[";
    for (size_t i = 0; i < scene.textureImages.size(); ++i) {
        out << (i ? "," : "") << scene.textureImages[i];
    }
    out << ']';
    return out.str();
}

void ClearKisakMenuZones() {
    std::lock_guard<std::mutex> lock(g_menuZoneMutex);
    g_menuZones.clear();
    ClearKisakMenuScriptZones();
    ++g_menuZoneGeneration;
}

void AddKisakMenuZone(std::shared_ptr<const KisakZoneLoadResult> zone) {
    std::lock_guard<std::mutex> lock(g_menuZoneMutex);
    AddKisakMenuScriptZone(zone);
    g_menuZones.push_back(std::move(zone));
    ++g_menuZoneGeneration;
}

std::vector<std::shared_ptr<const KisakZoneLoadResult>> GetKisakMenuZones() {
    std::lock_guard<std::mutex> lock(g_menuZoneMutex);
    return g_menuZones;
}

uint32_t GetKisakMenuZoneGeneration() {
    std::lock_guard<std::mutex> lock(g_menuZoneMutex);
    return g_menuZoneGeneration;
}
