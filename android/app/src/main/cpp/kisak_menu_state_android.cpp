#include "kisak_menu_state_android.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <map>
#include <mutex>
#include <sys/stat.h>
#include <utility>
#include <vector>

#include "kisak_android_fs_runtime.h"
#include "kisak_menu_expression_android.h"

namespace {
// menuDef_t offsets.
constexpr uint32_t kMenuName = 0;
constexpr uint32_t kMenuItemCount = 164;
constexpr uint32_t kMenuOnOpen = 196;
constexpr uint32_t kMenuOnClose = 200;
constexpr uint32_t kMenuItems = 280;

// itemDef_s offsets.
constexpr uint32_t kItemName = 0;
constexpr uint32_t kItemGroup = 52;
constexpr uint32_t kItemStaticFlags = 76;
constexpr uint32_t kItemDynamicFlags = 80;
constexpr uint32_t kItemType = 180;
constexpr uint32_t kItemTextAlignMode = 196;
constexpr uint32_t kItemTextAlignX = 200;
constexpr uint32_t kItemOnFocus = 260;
constexpr uint32_t kItemLeaveFocus = 264;
constexpr uint32_t kItemDvar = 268;
constexpr uint32_t kItemTypeData = 300;
constexpr uint32_t kItemSpecial = 292;
constexpr uint32_t kItemVisibleExp = 308;

// windowDef_t offsets.
constexpr uint32_t kWinRectX = 4;
constexpr uint32_t kWinRectY = 8;
constexpr uint32_t kWinRectW = 12;
constexpr uint32_t kWinRectH = 16;
constexpr uint32_t kWinHorzAlign = 20;
constexpr uint32_t kWinStaticFlags = 76;

// listBoxDef_s offsets.
constexpr uint32_t kListElementWidth = 16;
constexpr uint32_t kListElementHeight = 20;
constexpr uint32_t kListNotSelectable = 292;

// editFieldDef_s offsets, also used by ITEM_TYPE_SLIDER.
constexpr uint32_t kEditMinVal = 0;
constexpr uint32_t kEditMaxVal = 4;
constexpr uint32_t kEditDefVal = 8;

constexpr uint32_t kWindowDecoration = 0x100000;
constexpr uint32_t kWindowHorizontalScroll = 0x200000;
constexpr uint32_t kWindowVisibleDynamic = 4;
constexpr float kCanvasWidth = 854.0f;
constexpr int kMaxExecDepth = 4;

bool ApplyExecCommand(const std::string& commands, int depth = 0);

std::mutex g_stateMutex;
// hide/show overrides for serialized items, keyed by {zone, itemRef}.
std::map<std::pair<const void*, uint32_t>, bool> g_visibleOverrides;
struct ListBoxState {
    int cursorPos = 0;
    int startPos = 0;
};
std::map<std::pair<const void*, uint32_t>, ListBoxState> g_listBoxState;
std::vector<std::shared_ptr<const KisakZoneLoadResult>> g_scriptZones;
bool g_defaultControlsSeeded = false;
std::string g_pendingLaunchCommand;

struct BindAssignment {
    std::string command;
    std::string label;
};
std::map<int, BindAssignment> g_bindings;
struct BindCaptureState {
    const void* zone = nullptr;
    uint32_t itemRef = 0;
    std::string command;
};
BindCaptureState g_bindCapture;

std::mutex g_profileMutex;
std::string g_profileStoragePath;
std::vector<std::string> g_playerProfiles{"AndroidPlayer"};

struct FocusState {
    const void* zone = nullptr;
    uint32_t menuRef = 0;
    uint32_t itemRef = 0;
};
FocusState g_focus;

bool EqualNoCase(const std::string& a, const std::string& b) {
    return a.size() == b.size() && strcasecmp(a.c_str(), b.c_str()) == 0;
}

bool FeederIs(float value, float expected) {
    return std::fabs(value - expected) < 0.01f;
}

bool ItemIsTextFieldType(uint32_t itemType) {
    return itemType == 4 || itemType == 9 || itemType == 0x10
        || itemType == 0x11 || itemType == 0x12;
}

void ApplyRectX(const KisakZoneView& view, uint32_t windowRef, float& x, float& w) {
    switch (view.U32(windowRef, kWinHorzAlign)) {
        case 2:
        case 7:
            x += kCanvasWidth * 0.5f;
            break;
        case 3:
            x += kCanvasWidth;
            break;
        case 1:
        case 5:
        case 6:
            break;
        case 4: {
            const float s = kCanvasWidth / 640.0f;
            x *= s;
            w *= s;
            break;
        }
        default:
            x += (kCanvasWidth - 640.0f) * 0.5f;
            break;
    }
}

float RectPlacementX(int alignX, float x0, float containerWidth, float selfWidth) {
    if (alignX == 1) {
        return (containerWidth - selfWidth) * 0.5f + x0;
    }
    if (alignX == 2) {
        return containerWidth - selfWidth + x0;
    }
    return x0;
}

std::string AndroidKeyLabel(int unicodeChar, int keyCode) {
    if (unicodeChar >= 33 && unicodeChar <= 126) {
        char ch = static_cast<char>(unicodeChar);
        if (ch >= 'a' && ch <= 'z') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
        return std::string(1, ch);
    }
    if (unicodeChar == ' ' || keyCode == 62) {
        return "SPACE";
    }
    if (keyCode >= 7 && keyCode <= 16) {
        return std::string(1, static_cast<char>('0' + keyCode - 7));
    }
    if (keyCode >= 29 && keyCode <= 54) {
        return std::string(1, static_cast<char>('A' + keyCode - 29));
    }
    switch (keyCode) {
        case 19: return "UPARROW";
        case 20: return "DOWNARROW";
        case 21: return "LEFTARROW";
        case 22: return "RIGHTARROW";
        case 23: return "DPAD_CENTER";
        case 56: return "ALT_LEFT";
        case 57: return "ALT_RIGHT";
        case 59: return "SHIFT_LEFT";
        case 60: return "SHIFT_RIGHT";
        case 61: return "TAB";
        case 66: return "ENTER";
        case 67: return "BACKSPACE";
        case 111: return "ESCAPE";
        default: break;
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "KEYCODE_%d", keyCode);
    return buffer;
}

std::string UpperAscii(std::string value) {
    for (char& ch : value) {
        if (ch >= 'a' && ch <= 'z') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
    }
    return value;
}

int AndroidBindKeyId(int unicodeChar, int keyCode) {
    if (keyCode > 0) {
        return keyCode;
    }
    return unicodeChar > 0 ? unicodeChar + 0x10000 : -1;
}

int BindKeyIdFromLabel(const std::string& label) {
    const std::string key = UpperAscii(label);
    if (key.empty()) {
        return -1;
    }
    if (key.size() == 1) {
        const char ch = key[0];
        if (ch >= 'A' && ch <= 'Z') {
            return 29 + (ch - 'A');
        }
        if (ch >= '0' && ch <= '9') {
            return 7 + (ch - '0');
        }
    }
    if (key == "SPACE" || key == "KEY_SPACE") return 62;
    if (key == "ENTER" || key == "KEY_ENTER") return 66;
    if (key == "TAB" || key == "KEY_TAB") return 61;
    if (key == "ESC" || key == "ESCAPE" || key == "KEY_ESCAPE") return 111;
    if (key == "BACKSPACE" || key == "DEL" || key == "KEY_BACKSPACE") return 67;
    if (key == "UPARROW" || key == "UP") return 19;
    if (key == "DOWNARROW" || key == "DOWN") return 20;
    if (key == "LEFTARROW" || key == "LEFT") return 21;
    if (key == "RIGHTARROW" || key == "RIGHT") return 22;

    uint32_t hash = 2166136261u;
    for (char ch : key) {
        hash ^= static_cast<unsigned char>(ch);
        hash *= 16777619u;
    }
    return static_cast<int>(0x20000u + (hash & 0x3FFFFFu));
}

std::vector<int> CommandBindingKeysLocked(const std::string& command) {
    std::vector<int> keys;
    for (const auto& entry : g_bindings) {
        if (EqualNoCase(entry.second.command, command)) {
            keys.push_back(entry.first);
            if (keys.size() == 2) {
                break;
            }
        }
    }
    return keys;
}

void ClearCommandBindingsLocked(const std::string& command) {
    for (auto it = g_bindings.begin(); it != g_bindings.end();) {
        if (EqualNoCase(it->second.command, command)) {
            it = g_bindings.erase(it);
        } else {
            ++it;
        }
    }
}

void SetCommandBindingLocked(const std::string& command, int keyId, const std::string& label) {
    if (command.empty() || keyId < 0 || label.empty()) {
        return;
    }
    const std::vector<int> existing = CommandBindingKeysLocked(command);
    if (existing.size() == 2) {
        for (int key : existing) {
            g_bindings.erase(key);
        }
    }
    g_bindings.erase(keyId);
    g_bindings[keyId] = {command, label};
}

void SetCommandBindingFromLabelLocked(const std::string& command, const std::string& label) {
    SetCommandBindingLocked(command, BindKeyIdFromLabel(label), UpperAscii(label));
}

std::string CommandBindingTextLocked(const std::string& command) {
    std::vector<std::string> labels;
    for (const auto& entry : g_bindings) {
        if (EqualNoCase(entry.second.command, command) && !entry.second.label.empty()) {
            labels.push_back(entry.second.label);
            if (labels.size() == 2) {
                break;
            }
        }
    }
    if (labels.empty()) {
        return "@KEY_UNBOUND";
    }
    if (labels.size() == 1) {
        return labels[0];
    }
    return labels[0] + " OR " + labels[1];
}

std::string TrimAscii(std::string value) {
    while (!value.empty() && static_cast<unsigned char>(value.front()) <= ' ') {
        value.erase(value.begin());
    }
    while (!value.empty() && static_cast<unsigned char>(value.back()) <= ' ') {
        value.pop_back();
    }
    return value;
}

std::vector<std::string> SplitExecCommands(const std::string& commands) {
    std::vector<std::string> out;
    bool inQuote = false;
    size_t start = 0;
    for (size_t i = 0; i < commands.size(); ++i) {
        if (commands[i] == '"') {
            inQuote = !inQuote;
        } else if (!inQuote && commands[i] == '/' && i + 1 < commands.size() && commands[i + 1] == '/') {
            std::string command = TrimAscii(commands.substr(start, i - start));
            if (!command.empty()) {
                out.push_back(std::move(command));
            }
            i = commands.find('\n', i + 2);
            if (i == std::string::npos) {
                return out;
            }
            start = i + 1;
        } else if (!inQuote && (commands[i] == ';' || commands[i] == '\n' || commands[i] == '\r')) {
            std::string command = TrimAscii(commands.substr(start, i - start));
            if (!command.empty()) {
                out.push_back(std::move(command));
            }
            start = i + 1;
        }
    }
    std::string command = TrimAscii(commands.substr(start));
    if (!command.empty()) {
        out.push_back(std::move(command));
    }
    return out;
}

std::vector<std::string> ExecCommandTokens(const std::string& command) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    while (pos < command.size()) {
        while (pos < command.size() && static_cast<unsigned char>(command[pos]) <= ' ') {
            ++pos;
        }
        if (pos >= command.size()) {
            break;
        }
        if (command[pos] == '"') {
            const size_t end = command.find('"', pos + 1);
            if (end == std::string::npos) {
                tokens.push_back(command.substr(pos + 1));
                break;
            }
            tokens.push_back(command.substr(pos + 1, end - pos - 1));
            pos = end + 1;
            continue;
        }
        size_t end = pos;
        while (end < command.size() && static_cast<unsigned char>(command[end]) > ' ') {
            ++end;
        }
        tokens.push_back(command.substr(pos, end - pos));
        pos = end;
    }
    return tokens;
}

std::string JoinTokens(const std::vector<std::string>& tokens, size_t first) {
    std::string out;
    for (size_t i = first; i < tokens.size(); ++i) {
        if (!out.empty()) {
            out.push_back(' ');
        }
        out += tokens[i];
    }
    return out;
}

bool LooksLikeCfgPath(const std::string& value) {
    if (value.size() < 4) {
        return false;
    }
    const std::string lower = UpperAscii(value);
    return lower.size() >= 4 && lower.compare(lower.size() - 4, 4, ".CFG") == 0;
}

std::string Basename(const std::string& path) {
    const size_t slash = path.find_last_of("/\\");
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

bool ApplyExecFile(const std::string& qpath, int depth) {
    if (depth >= kMaxExecDepth || qpath.empty()) {
        return false;
    }
    std::vector<std::shared_ptr<const KisakZoneLoadResult>> zones;
    {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        zones = g_scriptZones;
    }
    const std::string qname = Basename(qpath);
    for (const auto& zone : zones) {
        if (!zone) {
            continue;
        }
        for (const KisakLoadedRawFile& rawFile : zone->rawFiles) {
            if ((EqualNoCase(rawFile.name, qpath) || EqualNoCase(Basename(rawFile.name), qname))
                && !rawFile.contents.empty()) {
                return ApplyExecCommand(rawFile.contents, depth + 1);
            }
        }
    }

    const KisakVirtualFileRead read = AndroidFsReadFile(qpath, 512 * 1024);
    if (!read.found || read.data.empty()) {
        return false;
    }
    const std::string text(
        reinterpret_cast<const char*>(read.data.data()),
        reinterpret_cast<const char*>(read.data.data()) + read.data.size()
    );
    return ApplyExecCommand(text, depth + 1);
}

std::string RawFileContents(const KisakZoneLoadResult& zone, const std::string& qpath) {
    const std::string qname = Basename(qpath);
    for (const KisakLoadedRawFile& rawFile : zone.rawFiles) {
        if ((EqualNoCase(rawFile.name, qpath) || EqualNoCase(Basename(rawFile.name), qname))
            && !rawFile.contents.empty()) {
            return rawFile.contents;
        }
    }
    return {};
}

bool ProfileExistsLocked(const std::string& name) {
    for (const std::string& profile : g_playerProfiles) {
        if (EqualNoCase(profile, name)) {
            return true;
        }
    }
    return false;
}

void EnsureDefaultProfileLocked() {
    if (g_playerProfiles.empty()) {
        g_playerProfiles.push_back("AndroidPlayer");
    }
}

void SaveProfilesLocked() {
    if (g_profileStoragePath.empty()) {
        return;
    }
    std::ofstream out(g_profileStoragePath, std::ios::out | std::ios::trunc);
    if (!out.good()) {
        return;
    }
    EnsureDefaultProfileLocked();
    for (const std::string& profile : g_playerProfiles) {
        out << profile << '\n';
    }
}

void LoadProfilesLocked() {
    g_playerProfiles.clear();
    if (!g_profileStoragePath.empty()) {
        std::ifstream in(g_profileStoragePath);
        std::string line;
        while (std::getline(in, line)) {
            const std::string profile = TrimAscii(line);
            if (!profile.empty() && !ProfileExistsLocked(profile)) {
                g_playerProfiles.push_back(profile);
            }
        }
    }
    EnsureDefaultProfileLocked();
}

std::string FocusedEditFieldValue() {
    FocusState focus;
    {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        focus = g_focus;
    }
    if (focus.zone == nullptr || focus.itemRef == 0) {
        return {};
    }

    const auto* zone = static_cast<const KisakZoneLoadResult*>(focus.zone);
    KisakZoneView view(*zone);
    if (!view.ValidRef(focus.itemRef, 372)
        || !ItemIsTextFieldType(view.U32(focus.itemRef, kItemType))) {
        return {};
    }
    const std::string dvar = view.StrAt(focus.itemRef, kItemDvar);
    return dvar.empty() ? std::string() : GetKisakUiDvar(dvar);
}

bool ApplyUiScript(const std::string& verb) {
    if (EqualNoCase(verb, "createPlayerProfile")) {
        const std::string profile = TrimAscii(FocusedEditFieldValue());
        if (profile.empty()) {
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(g_profileMutex);
            if (!ProfileExistsLocked(profile)) {
                g_playerProfiles.push_back(profile);
                SaveProfilesLocked();
            }
        }
        SetKisakUiDvar("ui_playerProfileSelected", profile);
        SetKisakUiDvar("com_playerProfile", profile);
        SetKisakUiDvar("name", profile);
        return true;
    }

    if (EqualNoCase(verb, "deletePlayerProfile")) {
        const std::string selected = GetKisakUiDvar("ui_playerProfileSelected");
        if (selected.empty()) {
            return false;
        }
        bool removed = false;
        std::string fallback;
        {
            std::lock_guard<std::mutex> lock(g_profileMutex);
            if (g_playerProfiles.size() <= 1) {
                return false;
            }
            for (auto it = g_playerProfiles.begin(); it != g_playerProfiles.end(); ++it) {
                if (EqualNoCase(*it, selected)) {
                    g_playerProfiles.erase(it);
                    removed = true;
                    SaveProfilesLocked();
                    break;
                }
            }
            fallback = g_playerProfiles.empty() ? std::string() : g_playerProfiles.front();
        }
        if (removed) {
            SetKisakUiDvar("ui_playerProfileSelected", fallback);
            return true;
        }
        return false;
    }

    if (EqualNoCase(verb, "selectPlayerProfile") || EqualNoCase(verb, "loadPlayerProfile")) {
        const std::string selected = GetKisakUiDvar("ui_playerProfileSelected");
        if (selected.empty()) {
            return false;
        }
        SetKisakUiDvar("com_playerProfile", selected);
        SetKisakUiDvar("name", selected);
        return true;
    }

    return false;
}

ListBoxState GetListBoxStateLocked(const KisakZoneLoadResult& zone, uint32_t itemRef) {
    const auto it = g_listBoxState.find(std::make_pair(static_cast<const void*>(&zone), itemRef));
    return it != g_listBoxState.end() ? it->second : ListBoxState{};
}

int ListBoxViewMax(const KisakZoneView& view, uint32_t itemRef, uint32_t listBox) {
    const bool horizontal = (view.U32(itemRef, kWinStaticFlags) & kWindowHorizontalScroll) != 0;
    const float totalSize = horizontal
        ? std::max(view.F32(itemRef, kWinRectW) - 2.0f, 0.0f)
        : std::max(view.F32(itemRef, kWinRectH) - 2.0f, 0.0f);
    const float unitSize = horizontal
        ? view.F32(listBox, kListElementWidth)
        : view.F32(listBox, kListElementHeight);
    if (unitSize <= 0.0f) {
        return 0;
    }
    return static_cast<int>(totalSize / unitSize);
}

void ClampListBoxState(
    const KisakZoneView& view,
    uint32_t itemRef,
    uint32_t listBox,
    ListBoxState* state
) {
    const int count = KisakMenuFeederCount(view.F32(itemRef, kItemSpecial));
    const int maxCursor = std::max(count - 1, 0);
    const int viewMax = std::max(ListBoxViewMax(view, itemRef, listBox), 1);
    const int maxStart = std::max(count - viewMax + 1, 0);
    state->cursorPos = std::clamp(state->cursorPos, 0, maxCursor);
    state->startPos = std::clamp(state->startPos, 0, maxStart);
    if (state->startPos > state->cursorPos) {
        state->startPos = state->cursorPos;
    }
    if (state->startPos <= state->cursorPos - viewMax) {
        state->startPos = std::max(state->cursorPos - viewMax + 1, 0);
    }
}

// Menu_ItemsMatchingGroup: an item matches by name or group, case
// insensitive; a trailing "*" makes it a prefix match.
bool NameMatches(const std::string& candidate, const std::string& pattern, size_t wildcard) {
    if (candidate.empty()) {
        return false;
    }
    if (wildcard == std::string::npos) {
        return EqualNoCase(candidate, pattern);
    }
    return candidate.size() >= wildcard
        && strncasecmp(candidate.c_str(), pattern.c_str(), wildcard) == 0;
}

void ShowItemsByName(
    const KisakZoneLoadResult& zone,
    uint32_t menuRef,
    const std::string& pattern,
    bool show,
    bool* stateChanged
) {
    KisakZoneView view(zone);
    const size_t wildcard = pattern.find('*');
    const uint32_t itemCount = view.U32(menuRef, kMenuItemCount);
    const uint32_t items = view.U32(menuRef, kMenuItems);
    for (uint32_t i = 0; i < itemCount; ++i) {
        const uint32_t itemRef = view.U32(items, i * 4);
        if (!view.ValidRef(itemRef, 372)) {
            continue;
        }
        if (!NameMatches(view.StrAt(itemRef, kItemName), pattern, wildcard)
            && !NameMatches(view.StrAt(itemRef, kItemGroup), pattern, wildcard)) {
            continue;
        }
        std::lock_guard<std::mutex> lock(g_stateMutex);
        const auto key = std::make_pair(static_cast<const void*>(&zone), itemRef);
        const auto it = g_visibleOverrides.find(key);
        if (it == g_visibleOverrides.end() || it->second != show) {
            g_visibleOverrides[key] = show;
            *stateChanged = true;
        }
    }
}

bool ItemVisibleForFocus(const KisakZoneLoadResult& zone, uint32_t itemRef) {
    if (!KisakItemDynamicVisible(zone, itemRef)) {
        return false;
    }
    KisakZoneView view(zone);
    const uint32_t visibleEntries = view.U32(itemRef + kItemVisibleExp, 0);
    if (visibleEntries == 0) {
        return true;
    }
    KisakExprEnv env; // dvars/localvars only — enough for visible-when checks
    return KisakStatementTrue(view, itemRef + kItemVisibleExp, env);
}

bool ItemFocusable(const KisakZoneLoadResult& zone, uint32_t itemRef) {
    KisakZoneView view(zone);
    if ((view.U32(itemRef, kItemStaticFlags) & kWindowDecoration) != 0) {
        return false;
    }
    if (view.U32(itemRef, kItemType) == 0) { // ITEM_TYPE_TEXT
        return false;
    }
    return ItemVisibleForFocus(zone, itemRef);
}

// setdvar-style value: menu scripts pass literals.
void SetLocalVarFromToken(const std::string& name, const std::string& value, bool asString) {
    if (asString) {
        SetKisakUiLocalVar(name, KisakExprValue::MakeString(value));
    } else if (value.find('.') != std::string::npos) {
        SetKisakUiLocalVar(
            name, KisakExprValue::MakeFloat(static_cast<float>(atof(value.c_str()))));
    } else {
        SetKisakUiLocalVar(name, KisakExprValue::MakeInt(atoi(value.c_str())));
    }
}

// Console commands inside menu exec/execnow that have Android-side UI effects.
// Map loads and engine-only verbs intentionally remain no-ops until those
// subsystems exist.
bool ApplySingleExecCommand(const std::string& command, int depth) {
    const std::vector<std::string> tokens = ExecCommandTokens(command);
    if (tokens.empty()) {
        return false;
    }
    const std::string& verb = tokens[0];
    if ((strcasecmp(verb.c_str(), "exec") == 0 || strcasecmp(verb.c_str(), "execnow") == 0)
        && tokens.size() >= 2) {
        return ApplyExecFile(tokens[1], depth);
    }
    if (strcasecmp(verb.c_str(), "set") == 0 || strcasecmp(verb.c_str(), "seta") == 0) {
        if (tokens.size() < 3) {
            return false;
        }
        SetKisakUiDvar(tokens[1], JoinTokens(tokens, 2));
        return true;
    }
    if (strcasecmp(verb.c_str(), "bind") == 0) {
        if (tokens.size() < 3) {
            return false;
        }
        std::lock_guard<std::mutex> lock(g_stateMutex);
        SetCommandBindingFromLabelLocked(JoinTokens(tokens, 2), tokens[1]);
        return true;
    }
    if (strcasecmp(verb.c_str(), "unbind") == 0) {
        if (tokens.size() < 2) {
            return false;
        }
        std::lock_guard<std::mutex> lock(g_stateMutex);
        g_bindings.erase(BindKeyIdFromLabel(tokens[1]));
        return true;
    }
    if (strcasecmp(verb.c_str(), "unbindall") == 0) {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        g_bindings.clear();
        return true;
    }
    if ((strcasecmp(verb.c_str(), "devmap") == 0 || strcasecmp(verb.c_str(), "map") == 0)
        && tokens.size() >= 2) {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        g_pendingLaunchCommand = verb + " " + tokens[1];
        return true;
    }
    if (tokens.size() == 1 && LooksLikeCfgPath(tokens[0])) {
        return ApplyExecFile(tokens[0], depth);
    }
    return false;
}

// One exec string can chain commands; semicolons inside quoted bind commands
// stay attached to that command.
bool ApplyExecCommand(const std::string& commands, int depth) {
    if (depth > kMaxExecDepth) {
        return false;
    }
    bool applied = false;
    for (const std::string& command : SplitExecCommands(commands)) {
        applied |= ApplySingleExecCommand(command, depth);
    }
    return applied;
}

// No savegame system is wired on Android yet, so SaveExists() reads false —
// savegameshow hides its target and savegamehide shows it, which is exactly
// the retail fresh-install main menu (New Game visible, Resume hidden).
constexpr bool kSaveGameExists = false;

void RunFocusScripts(
    const KisakZoneLoadResult& zone,
    uint32_t menuRef,
    uint32_t itemRef,
    bool* stateChanged
) {
    FocusState previous;
    {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        previous = g_focus;
        if (previous.zone == &zone && previous.itemRef == itemRef) {
            return;
        }
        g_focus = {static_cast<const void*>(&zone), menuRef, itemRef};
    }
    *stateChanged = true;

    KisakZoneView view(zone);
    if (previous.zone == &zone && previous.itemRef != 0
        && view.ValidRef(previous.itemRef, 372)) {
        const std::string leave = view.StrAt(previous.itemRef, kItemLeaveFocus);
        if (!leave.empty()) {
            RunKisakMenuScript(zone, previous.menuRef, leave);
        }
    }
    const std::string onFocus = view.StrAt(itemRef, kItemOnFocus);
    if (!onFocus.empty()) {
        RunKisakMenuScript(zone, menuRef, onFocus);
    }
}
} // namespace

std::vector<std::string> KisakMenuScriptTokens(const std::string& script) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    while (pos < script.size()) {
        const char c = script[pos];
        if (c == '"') {
            const size_t end = script.find('"', pos + 1);
            if (end == std::string::npos) {
                break;
            }
            tokens.push_back(script.substr(pos + 1, end - pos - 1));
            pos = end + 1;
        } else if (c == ';') {
            tokens.emplace_back(";");
            ++pos;
        } else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            ++pos;
        } else {
            size_t end = pos;
            while (end < script.size() && script[end] != ' ' && script[end] != '\t'
                   && script[end] != ';' && script[end] != '"'
                   && script[end] != '\r' && script[end] != '\n') {
                ++end;
            }
            tokens.push_back(script.substr(pos, end - pos));
            pos = end;
        }
    }
    return tokens;
}

void SetKisakMenuStorageRoot(const std::string& rootPath) {
    if (rootPath.empty()) {
        return;
    }
    const std::string playersDir = rootPath + "/players";
    mkdir(playersDir.c_str(), 0775);
    std::lock_guard<std::mutex> lock(g_profileMutex);
    g_profileStoragePath = playersDir + "/profiles_android.txt";
    LoadProfilesLocked();
    SaveProfilesLocked();
}

void ClearKisakMenuScriptZones() {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_scriptZones.clear();
    g_defaultControlsSeeded = false;
    g_pendingLaunchCommand.clear();
}

void AddKisakMenuScriptZone(std::shared_ptr<const KisakZoneLoadResult> zone) {
    if (!zone) {
        return;
    }
    const std::string defaultCfg = RawFileContents(*zone, "default.cfg");
    bool seedDefaults = false;
    {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        g_scriptZones.push_back(std::move(zone));
        if (!g_defaultControlsSeeded && g_bindings.empty() && !defaultCfg.empty()) {
            g_defaultControlsSeeded = true;
            seedDefaults = true;
        }
    }
    if (seedDefaults) {
        ApplyExecCommand(defaultCfg);
    }
}

void ResetKisakMenuForOpen(const KisakZoneLoadResult& zone, uint32_t menuRef) {
    KisakZoneView view(zone);
    const uint32_t itemCount = view.U32(menuRef, kMenuItemCount);
    const uint32_t items = view.U32(menuRef, kMenuItems);
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_focus = {};
    for (uint32_t i = 0; i < itemCount; ++i) {
        const uint32_t itemRef = view.U32(items, i * 4);
        if (view.ValidRef(itemRef, 372)) {
            g_visibleOverrides.erase(std::make_pair(static_cast<const void*>(&zone), itemRef));
        }
    }
}

std::string ConsumeKisakMenuLaunchCommand() {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    std::string command = std::move(g_pendingLaunchCommand);
    g_pendingLaunchCommand.clear();
    return command;
}

KisakMenuScriptResult RunKisakMenuScript(
    const KisakZoneLoadResult& zone,
    uint32_t menuRef,
    const std::string& script
) {
    KisakMenuScriptResult result;
    KisakZoneView view(zone);
    const std::string menuName = view.StrAt(menuRef, kMenuName);
    const std::vector<std::string> tokens = KisakMenuScriptTokens(script);

    for (size_t i = 0; i < tokens.size(); ++i) {
        const char* cmd = tokens[i].c_str();
        auto arg = [&](size_t n) -> std::string {
            return i + n < tokens.size() ? tokens[i + n] : std::string();
        };
        if (tokens[i] == ";") {
            continue;
        }
        if (strcasecmp(cmd, "play") == 0) {
            ++i; // sound alias — no UI-state effect
        } else if (strcasecmp(cmd, "uiscript") == 0) {
            if (ApplyUiScript(arg(1))) {
                result.stateChanged = true;
            }
            ++i;
        } else if (strcasecmp(cmd, "setdvar") == 0) {
            SetKisakUiDvar(arg(1), arg(2));
            result.stateChanged = true;
            i += 2;
        } else if (strcasecmp(cmd, "exec") == 0 || strcasecmp(cmd, "execnow") == 0) {
            if (ApplyExecCommand(arg(1))) {
                result.stateChanged = true;
            }
            ++i;
        } else if (strcasecmp(cmd, "execondvarstringvalue") == 0) {
            if (GetKisakUiDvar(arg(1)) == arg(2) && ApplyExecCommand(arg(3))) {
                result.stateChanged = true;
            }
            i += 3;
        } else if (strcasecmp(cmd, "setlocalvarint") == 0
                   || strcasecmp(cmd, "setlocalvarbool") == 0
                   || strcasecmp(cmd, "setlocalvarfloat") == 0) {
            SetLocalVarFromToken(arg(1), arg(2), false);
            result.stateChanged = true;
            i += 2;
        } else if (strcasecmp(cmd, "setlocalvarstring") == 0) {
            SetLocalVarFromToken(arg(1), arg(2), true);
            result.stateChanged = true;
            i += 2;
        } else if (strcasecmp(cmd, "hide") == 0 || strcasecmp(cmd, "show") == 0) {
            ShowItemsByName(zone, menuRef, arg(1), strcasecmp(cmd, "show") == 0,
                            &result.stateChanged);
            ++i;
        } else if (strcasecmp(cmd, "savegameshow") == 0) {
            ShowItemsByName(zone, menuRef, arg(1), kSaveGameExists, &result.stateChanged);
            ++i;
        } else if (strcasecmp(cmd, "savegamehide") == 0) {
            ShowItemsByName(zone, menuRef, arg(1), !kSaveGameExists, &result.stateChanged);
            ++i;
        } else if (strcasecmp(cmd, "open") == 0) {
            result.openTarget = arg(1);
            ++i;
        } else if (strcasecmp(cmd, "close") == 0) {
            const std::string target = arg(1);
            if (EqualNoCase(target, "self") || EqualNoCase(target, menuName)) {
                const KisakMenuScriptResult closeResult = RunKisakMenuOnClose(zone, menuRef);
                result.stateChanged |= closeResult.stateChanged;
                result.closeSelf = true;
            }
            ++i;
        } else if (strcasecmp(cmd, "setfocus") == 0) {
            const std::string name = arg(1);
            const uint32_t itemCount = view.U32(menuRef, kMenuItemCount);
            const uint32_t items = view.U32(menuRef, kMenuItems);
            for (uint32_t j = 0; j < itemCount; ++j) {
                const uint32_t itemRef = view.U32(items, j * 4);
                if (view.ValidRef(itemRef, 372)
                    && EqualNoCase(view.StrAt(itemRef, kItemName), name)) {
                    RunFocusScripts(zone, menuRef, itemRef, &result.stateChanged);
                    break;
                }
            }
            ++i;
        } else if (strcasecmp(cmd, "focusfirst") == 0) {
            const uint32_t itemCount = view.U32(menuRef, kMenuItemCount);
            const uint32_t items = view.U32(menuRef, kMenuItems);
            for (uint32_t j = 0; j < itemCount; ++j) {
                const uint32_t itemRef = view.U32(items, j * 4);
                if (view.ValidRef(itemRef, 372) && ItemFocusable(zone, itemRef)) {
                    RunFocusScripts(zone, menuRef, itemRef, &result.stateChanged);
                    break;
                }
            }
        }
        // Unknown commands (fadein, showMenu, ingame-only verbs...) are
        // skipped one token at a time, like the engine's unmatched lookups.
    }
    return result;
}

void RunKisakMenuOnOpen(const KisakZoneLoadResult& zone, uint32_t menuRef) {
    KisakZoneView view(zone);
    const std::string onOpen = view.StrAt(menuRef, kMenuOnOpen);
    if (!onOpen.empty()) {
        RunKisakMenuScript(zone, menuRef, onOpen);
    }
}

KisakMenuScriptResult RunKisakMenuOnClose(const KisakZoneLoadResult& zone, uint32_t menuRef) {
    KisakZoneView view(zone);
    const std::string onClose = view.StrAt(menuRef, kMenuOnClose);
    if (onClose.empty()) {
        return {};
    }
    return RunKisakMenuScript(zone, menuRef, onClose);
}

void ClearKisakMenuFocus() {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    g_focus = {};
}

bool KisakMenuFocusItem(
    const KisakZoneLoadResult& zone,
    uint32_t menuRef,
    uint32_t itemRef
) {
    if (!ItemFocusable(zone, itemRef)) {
        return false;
    }
    bool stateChanged = false;
    RunFocusScripts(zone, menuRef, itemRef, &stateChanged);
    return stateChanged;
}

bool KisakItemDynamicVisible(const KisakZoneLoadResult& zone, uint32_t itemRef) {
    {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        const auto it = g_visibleOverrides.find(
            std::make_pair(static_cast<const void*>(&zone), itemRef));
        if (it != g_visibleOverrides.end()) {
            return it->second;
        }
    }
    KisakZoneView view(zone);
    return (view.U32(itemRef, kItemDynamicFlags) & kWindowVisibleDynamic) != 0;
}

bool KisakItemIsDecoration(const KisakZoneLoadResult& zone, uint32_t itemRef) {
    KisakZoneView view(zone);
    return (view.U32(itemRef, kItemStaticFlags) & kWindowDecoration) != 0;
}

bool KisakMenuItemActivate(
    const KisakZoneLoadResult& zone,
    uint32_t itemRef,
    float canvasX,
    float canvasY
) {
    KisakZoneView view(zone);
    const uint32_t itemType = view.U32(itemRef, kItemType);
    if (ItemIsTextFieldType(itemType)) {
        const std::string dvar = view.StrAt(itemRef, kItemDvar);
        if (dvar.empty()) {
            return false;
        }
        if (GetKisakUiDvar(dvar).empty()) {
            SetKisakUiDvar(dvar, "");
        }
        return true;
    }

    if (itemType == 0x6) { // ITEM_TYPE_LISTBOX
        const uint32_t listBox = view.U32(itemRef, kItemTypeData);
        if (!view.ValidRef(listBox, 340) || view.U32(listBox, kListNotSelectable) != 0) {
            return false;
        }
        const float feederId = view.F32(itemRef, kItemSpecial);
        const int count = KisakMenuFeederCount(feederId);
        if (count <= 0) {
            return false;
        }
        const bool horizontal = (view.U32(itemRef, kWinStaticFlags) & kWindowHorizontalScroll) != 0;
        const float x = view.F32(itemRef, kWinRectX);
        const float y = view.F32(itemRef, kWinRectY);
        const float unitSize = horizontal
            ? view.F32(listBox, kListElementWidth)
            : view.F32(listBox, kListElementHeight);
        if (unitSize <= 0.0f) {
            return false;
        }

        ListBoxState state;
        {
            std::lock_guard<std::mutex> lock(g_stateMutex);
            state = GetListBoxStateLocked(zone, itemRef);
        }
        const int touchedOffset = horizontal
            ? static_cast<int>((canvasX - x - 1.0f) / unitSize)
            : static_cast<int>((canvasY - y - 2.0f) / unitSize);
        if (touchedOffset < 0) {
            return false;
        }
        state.cursorPos = state.startPos + touchedOffset;
        ClampListBoxState(view, itemRef, listBox, &state);
        bool feederChanged = false;
        if (FeederIs(feederId, 16.0f)) {
            const std::string value = KisakMenuFeederItemText(feederId, state.cursorPos, 0);
            feederChanged = GetKisakUiDvar("ui_savegame") != value;
            SetKisakUiDvar("ui_savegame", value);
        } else if (FeederIs(feederId, 24.0f)) {
            const std::string value = KisakMenuFeederItemText(feederId, state.cursorPos, 0);
            feederChanged = GetKisakUiDvar("ui_playerProfileSelected") != value;
            SetKisakUiDvar("ui_playerProfileSelected", value);
        }
        std::lock_guard<std::mutex> lock(g_stateMutex);
        const auto key = std::make_pair(static_cast<const void*>(&zone), itemRef);
        const ListBoxState previous = GetListBoxStateLocked(zone, itemRef);
        g_listBoxState[key] = state;
        return feederChanged || previous.cursorPos != state.cursorPos || previous.startPos != state.startPos;
    }

    const std::string dvar = view.StrAt(itemRef, kItemDvar);
    if (dvar.empty()) {
        return false;
    }

    if (itemType == 0xA) { // ITEM_TYPE_SLIDER
        const uint32_t editDef = view.U32(itemRef, kItemTypeData);
        if (!view.ValidRef(editDef, 32)) {
            return false;
        }
        float x = view.F32(itemRef, kWinRectX);
        float w = view.F32(itemRef, kWinRectW);
        ApplyRectX(view, itemRef, x, w);
        const float sliderX = RectPlacementX(
            static_cast<int>(view.U32(itemRef, kItemTextAlignMode) & 3),
            x + view.F32(itemRef, kItemTextAlignX),
            w,
            96.0f
        );
        float t = (canvasX - (sliderX + 6.0f)) / 84.0f;
        t = std::clamp(t, 0.0f, 1.0f);
        const float minVal = view.F32(editDef, kEditMinVal);
        const float maxVal = view.F32(editDef, kEditMaxVal);
        const float defVal = view.F32(editDef, kEditDefVal);
        const float value = maxVal > minVal ? minVal + (maxVal - minVal) * t : defVal;
        char buffer[48];
        snprintf(buffer, sizeof(buffer), "%g", value);
        const std::string previous = GetKisakUiDvar(dvar);
        SetKisakUiDvar(dvar, buffer);
        return previous != buffer;
    }

    if (itemType == 0xB) { // ITEM_TYPE_YESNO
        const int current = atoi(GetKisakUiDvar(dvar).c_str());
        SetKisakUiDvar(dvar, current == 0 ? "1" : "0");
        return true;
    }

    if (itemType == 0xE) { // ITEM_TYPE_BIND
        std::lock_guard<std::mutex> lock(g_stateMutex);
        g_bindCapture.zone = &zone;
        g_bindCapture.itemRef = itemRef;
        g_bindCapture.command = dvar;
        return true;
    }

    if (itemType != 0xC) { // ITEM_TYPE_MULTI
        return false;
    }

    const uint32_t multi = view.U32(itemRef, kItemTypeData);
    if (!view.ValidRef(multi, 392)) {
        return false;
    }
    const int count = static_cast<int>(view.U32(multi, 384));
    if (count <= 0 || count > 32) {
        return false;
    }
    const bool strDef = view.U32(multi, 388) != 0;
    const std::string currentValue = GetKisakUiDvar(dvar);

    int current = 0;
    if (strDef) {
        for (int i = 0; i < count; ++i) {
            if (strcasecmp(currentValue.c_str(), view.StrAt(multi, 128 + i * 4).c_str()) == 0) {
                current = i;
                break;
            }
        }
    } else {
        const float numeric = static_cast<float>(atof(currentValue.c_str()));
        for (int i = 0; i < count; ++i) {
            if (numeric == view.F32(multi, 256 + i * 4)) {
                current = i;
                break;
            }
        }
    }

    const int next = (current + 1) % count;
    if (strDef) {
        SetKisakUiDvar(dvar, view.StrAt(multi, 128 + next * 4));
    } else {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%g", static_cast<double>(view.F32(multi, 256 + next * 4)));
        SetKisakUiDvar(dvar, buffer);
    }
    return true;
}

int KisakMenuFeederCount(float feederId) {
    if (FeederIs(feederId, 16.0f)) {
        // Fresh Android bring-up has no savegame scanner yet.
        return 0;
    }
    if (FeederIs(feederId, 24.0f)) {
        std::lock_guard<std::mutex> lock(g_profileMutex);
        return static_cast<int>(g_playerProfiles.size());
    }
    return 0;
}

std::string KisakMenuFeederItemText(float feederId, int row, int column) {
    if (row < 0) {
        return {};
    }
    if (FeederIs(feederId, 24.0f)) {
        if (column != 0) {
            return {};
        }
        std::lock_guard<std::mutex> lock(g_profileMutex);
        return row < static_cast<int>(g_playerProfiles.size()) ? g_playerProfiles[static_cast<size_t>(row)] : std::string();
    }
    return {};
}

int KisakMenuListBoxCursor(const KisakZoneLoadResult& zone, uint32_t itemRef) {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    return GetListBoxStateLocked(zone, itemRef).cursorPos;
}

int KisakMenuListBoxStart(const KisakZoneLoadResult& zone, uint32_t itemRef) {
    std::lock_guard<std::mutex> lock(g_stateMutex);
    KisakZoneView view(zone);
    const uint32_t listBox = view.U32(itemRef, kItemTypeData);
    ListBoxState state = GetListBoxStateLocked(zone, itemRef);
    if (view.ValidRef(listBox, 340)) {
        ClampListBoxState(view, itemRef, listBox, &state);
    }
    return state.startPos;
}

std::string KisakMenuCommandBindingText(const std::string& command) {
    if (command.empty()) {
        return "@KEY_UNBOUND";
    }
    std::lock_guard<std::mutex> lock(g_stateMutex);
    return CommandBindingTextLocked(command);
}

std::string KisakMenuBindText(const KisakZoneLoadResult& zone, uint32_t itemRef) {
    KisakZoneView view(zone);
    if (!view.ValidRef(itemRef, 372) || view.U32(itemRef, kItemType) != 0xE) {
        return {};
    }
    const std::string command = view.StrAt(itemRef, kItemDvar);
    if (command.empty()) {
        return "@KEY_UNBOUND";
    }
    std::lock_guard<std::mutex> lock(g_stateMutex);
    if (g_bindCapture.zone == &zone && g_bindCapture.itemRef == itemRef) {
        return "@MENU_BIND_KEY_PENDING";
    }
    return CommandBindingTextLocked(command);
}

bool KisakMenuTextInput(int unicodeChar, int keyCode) {
    {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        if (g_bindCapture.zone != nullptr && !g_bindCapture.command.empty()) {
            if (keyCode == 111) { // KEYCODE_ESCAPE
                g_bindCapture = {};
                return true;
            }
            if (keyCode == 67) { // KEYCODE_DEL clears the command binding.
                ClearCommandBindingsLocked(g_bindCapture.command);
                g_bindCapture = {};
                return true;
            }
            const int keyId = AndroidBindKeyId(unicodeChar, keyCode);
            SetCommandBindingLocked(g_bindCapture.command, keyId, AndroidKeyLabel(unicodeChar, keyCode));
            g_bindCapture = {};
            return true;
        }
    }

    FocusState focus;
    {
        std::lock_guard<std::mutex> lock(g_stateMutex);
        focus = g_focus;
    }
    if (focus.zone == nullptr || focus.itemRef == 0) {
        return false;
    }

    const auto* zone = static_cast<const KisakZoneLoadResult*>(focus.zone);
    KisakZoneView view(*zone);
    if (!view.ValidRef(focus.itemRef, 372)
        || !ItemIsTextFieldType(view.U32(focus.itemRef, kItemType))) {
        return false;
    }
    const std::string dvar = view.StrAt(focus.itemRef, kItemDvar);
    if (dvar.empty()) {
        return false;
    }

    std::string value = GetKisakUiDvar(dvar);
    if (keyCode == 67) { // KEYCODE_DEL
        if (!value.empty()) {
            value.pop_back();
            SetKisakUiDvar(dvar, value);
            return true;
        }
        return false;
    }
    if (keyCode == 66 || keyCode == 160) { // ENTER / NUMPAD_ENTER
        return false;
    }
    if (unicodeChar < 32 || unicodeChar > 126) {
        return false;
    }
    if (value.size() >= 64) {
        return false;
    }
    value.push_back(static_cast<char>(unicodeChar));
    SetKisakUiDvar(dvar, value);
    return true;
}
