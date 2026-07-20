#pragma once

#include <memory>
#include <string>
#include <vector>

#include "kisak_zone_loader_android.h"

// Menu script runtime: a faithful subset of Item_RunScript
// (src/ui/ui_shared.cpp commandList) working on serialized menuDef_t /
// itemDef_s data. Handles the state-side commands (setdvar, setLocalVar*,
// hide/show groups, focus) and reports navigation (open/close) back to the
// caller instead of executing it.

struct KisakMenuScriptResult {
    std::string openTarget; // "open X" — caller drives navigation
    bool closeSelf = false; // "close self" (or close <this menu>)
    bool stateChanged = false; // dvar/localvar/visibility/focus changed
};

// Splits a serialized menu script into its quoted tokens (";" separators
// come through as tokens and are skipped by the executor).
std::vector<std::string> KisakMenuScriptTokens(const std::string& script);

// Runs one item/menu script (action, onFocus, onOpen, ...).
KisakMenuScriptResult RunKisakMenuScript(
    const KisakZoneLoadResult& zone,
    uint32_t menuRef,
    const std::string& script
);

// Gives menu-side Android state a writable root for lightweight profile
// persistence while the full COD4 profile filesystem is still being ported.
void SetKisakMenuStorageRoot(const std::string& rootPath);

// Lets exec <rawfile.cfg> resolve RawFile assets from any loaded menu zone
// (not only loose/IWD filesystem files).
void ClearKisakMenuScriptZones();
void AddKisakMenuScriptZone(std::shared_ptr<const KisakZoneLoadResult> zone);

// Map/game launch commands requested by menu scripts (`devmap`, `map`) are
// captured here until the native gameplay loader exists.
std::string ConsumeKisakMenuLaunchCommand();

// Menu open protocol: runs the menu's onOpen script; its "focusFirst" gives
// focus to the first focusable visible item (running that item's onFocus).
void ResetKisakMenuForOpen(const KisakZoneLoadResult& zone, uint32_t menuRef);
void RunKisakMenuOnOpen(const KisakZoneLoadResult& zone, uint32_t menuRef);
KisakMenuScriptResult RunKisakMenuOnClose(const KisakZoneLoadResult& zone, uint32_t menuRef);

// Menu navigation starts a fresh focus pass. Without this, reopening a menu
// whose first item was already focused can skip its onFocus show/hide scripts.
void ClearKisakMenuFocus();

// Gives focus to an item (touch down on its hitbox): runs the previous focus
// item's leaveFocus and the new item's onFocus. Returns true when UI state
// changed (scene should be rebuilt).
bool KisakMenuFocusItem(
    const KisakZoneLoadResult& zone,
    uint32_t menuRef,
    uint32_t itemRef
);

// Dynamic visibility: the serialized windowDef dynamicFlags visible bit,
// overridden by any hide/show script that ran since zone load.
bool KisakItemDynamicVisible(const KisakZoneLoadResult& zone, uint32_t itemRef);

// WINDOW_DECORATION check — decoration items draw but never take part in
// hit-testing or focus.
bool KisakItemIsDecoration(const KisakZoneLoadResult& zone, uint32_t itemRef);

// Engine-side effect of activating an interactive item (before its action
// script runs): multi (0xC) cycles its dvar to the next entry, yesno (0xB)
// toggles it, listbox (0x6) selects the row under the touch point. Returns
// true when UI state changed (scene should rebuild).
bool KisakMenuItemActivate(
    const KisakZoneLoadResult& zone,
    uint32_t itemRef,
    float canvasX,
    float canvasY
);

// Minimal Android-side UI feeders for serialized listBox items. The retail
// front end asks feeder 16 for savegames and feeder 24 for player profiles.
int KisakMenuFeederCount(float feederId);
std::string KisakMenuFeederItemText(float feederId, int row, int column);
int KisakMenuListBoxCursor(const KisakZoneLoadResult& zone, uint32_t itemRef);
int KisakMenuListBoxStart(const KisakZoneLoadResult& zone, uint32_t itemRef);

// Minimal ITEM_TYPE_BIND state. Android keeps the same UI-level key->command
// ownership semantics as the PC menu while the full input system is still out.
std::string KisakMenuCommandBindingText(const std::string& command);
std::string KisakMenuBindText(const KisakZoneLoadResult& zone, uint32_t itemRef);

// Applies keyboard text to the currently focused editfield. keyCode follows
// Android KeyEvent constants for backspace/enter; unicodeChar is the printable
// character when available. Also completes pending ITEM_TYPE_BIND capture.
bool KisakMenuTextInput(int unicodeChar, int keyCode);
