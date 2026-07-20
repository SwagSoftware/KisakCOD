# Android port bootstrap

This repository currently builds a Windows Direct3D 9 executable. The Android
port is staged so the project can keep the Windows build intact while Android
support is brought up piece by piece.

## Current milestone

- Android Gradle project under `android/app`.
- Native C++ library built with the Android NDK.
- Java launcher activity in forced landscape mode.
- JNI bootstrap proves native code loads on device.
- Android `SurfaceView` with native surface lifecycle callbacks.
- Native data probe creates and checks the user-provided COD4 data directory.
- Android COD4 filesystem bootstrap mounts the user data folder, indexes IWD
  ZIP entries, lists FastFiles, and resolves representative loose/IWD assets.
- Engine filesystem bridge derives the `fs_basepath`, `fs_homepath`,
  `fs_game`, FastFile root, and `FS_Startup("main")` call plan from the Android
  mount, including the `fileSysCheck.cfg` readiness check used by
  `FS_InitFilesystem()`.
- Android VFS probe can read loose files and inflate Deflate-compressed files
  from IWD archives, proving the next `FS_FOpenFileRead` bridge can return real
  bytes from retail COD4 data.
- Android FS runtime shim exposes `AndroidFsInitialized()` and
  `AndroidFsReadFile(qpath)` as the temporary native backend that can be wired
  into `FS_FOpenFileRead` while the full `com_files.cpp` dependency chain is
  being made portable.
- Native Android renderer bootstrap owns the `SurfaceView` buffer via
  `ANativeWindow_lock` and draws a diagnostic frame from C++, giving the port a
  real NDK rendering hook before replacing D3D9 with Vulkan/OpenGL ES.
- Native render loop now owns the surface lifetime on a C++ thread, redraws
  continuously, and stops cleanly when Android destroys the `SurfaceView`.
- EGL/OpenGL ES backend now initializes on that render thread and draws the
  diagnostic frame with GPU clears/scissor passes, with the previous
  `ANativeWindow_lock` software path retained as fallback.
- OpenGL ES 3 shader pipeline is initialized on the render thread and draws a
  simple animated triangle, proving programmable GPU draw calls are live.
- Renderer now reads `images/$white.iwi` through the Android VFS and creates an
  OpenGL texture from those retail asset bytes, proving the data path from IWD
  archive to GPU resource creation.
- Real IWI decoder (`kisak_iwi_texture_android.cpp`) parses the COD4 IWi
  version 6 header and software-decodes DXT1/DXT3/DXT5 plus raw bitmap formats
  to RGBA8. The renderer picks a recognizable retail texture
  (`images/compassface.iwi`, 256x256 DXT3 from `iw_01.iwd`), uploads the top
  mip, and draws it alpha-blended on an aspect-correct fullscreen quad —
  verified on device with a screenshot showing the COD4 compass face.
- FastFile decompression (`kisak_fastfile_android.cpp`) validates the
  `IWffu100` version 5 header, streams the zlib zone payload with bounded
  memory, and parses the decompressed `XFile` header (size, externalSize,
  block sizes). Verified on device against `zone/english/code_post_gfx.ff`
  (872,586 compressed -> 1,378,309 zone bytes, XFile size consistent with the
  stream), which is the first zone the real engine loads.
- Zone asset-directory parser (`kisak_zone_parser_android.cpp`) reads the
  decompressed zone using the engine's own layout (`XFile` with 9 block sizes,
  `XAssetList`, inline-pointer convention): it extracts the ScriptStringList
  strings and the full XAsset directory as a per-type histogram, without yet
  deserializing asset payloads. Logic verified with a host-compiled unit test
  and on device against `code_post_gfx.ff`: 107 script strings (real skeleton
  joint names such as `j_mainroot`) and 1,639 assets (1,351 localize, 90
  techniqueset, 72 material, 76 rawfile, 9 font, 1 weapon, ...).
- RawFile extraction (`kisak_zone_rawfile_android.cpp`) recovers RawFile
  assets from the decompressed zone by scanning for the serialized inline
  pattern ({name=-1, len, buffer=-1} from `Load_RawFile`), validates names and
  NUL-terminated buffers, and dumps them under
  `<data>/cod4/dump/rawfiles/<zone>/` with path-traversal protection. On
  device it recovers 74 of the 76 rawfiles in `code_post_gfx.ff` (the other 2
  reference shared data and need the full loader) — real readable configs
  such as `enable_art.cfg` and `soundaliases/channels.def`.
- Zone loader port (`kisak_zone_loader_android.cpp`): compiling the
  decompiled engine directly is impossible on modern arm64-only devices (the
  decomp assumes 32-bit pointers throughout, including in-place zone
  deserialization), so the real `DB_LoadXFile` walk from
  `src/database/db_stream.cpp` / `db_load.cpp` is ported as 64-bit-clean code
  that keeps the exact 32-bit serialized layout inside the nine zone memory
  blocks: pointer slots stay 4 bytes and hold the engine's own
  `(block << 28 | offset) + 1` encoding instead of native pointers. Stream
  mechanics are faithful (temp-block rewind on pop, zero-filled runtime
  block 1, delay-streamed blocks 2-3, `-1`/`-2`/alias pointer conventions,
  `DB_InsertPointer`). Per-type deserializers so far: LocalizeEntry, RawFile,
  MaterialTechniqueSet (34 technique slots, variable-length techniques with
  passes, vertex declarations, vertex/pixel shaders with inline D3D bytecode,
  shader arguments with literal constants), Material (info, texture defs
  including `water_t`, constant table with 16-byte alignment, state bits),
  and GfxImage (GfxTextureLoad slot plus GfxImageLoadDef with inline pixel
  payload). D3D runtime hooks (texture/shader/vertex-decl creation) are
  skipped at their exact call sites. On device it loads 1,200 of the 1,639
  assets of `code_post_gfx.ff` — 1,116 localize (real menu text such as
  `MENU_MULTIPLAYER="Multiplayer"`), 74 rawfiles, 2 techsets, 1 material
  (`ui_cursor`), 8 images — and stops cleanly at the first soundcurve.
  Comma-prefixed empty techsets in localized zones are the IW "asset
  reference" convention, not a parse error.
- Sound and font deserializers: SndCurve (72 bytes), snd_alias_list_t with
  its 92-byte alias array (four name strings, SoundFile with loaded/streamed
  variants, falloff-curve asset slot, 408-byte SpeakerMap), LoadedSound with
  inline MSS audio bytes in the temp block, and Font_s (materials as asset
  slots plus the 24-byte-per-entry glyph array). Asset types absent from the
  engine's own `Load_XAssetHeader` switch (xmodelpieces, uimap,
  snddriverglobals, aitype, mptype, character, xmodelalias) are skipped with
  no stream consumption, exactly like the PC engine — snddriverglobals slots
  hold raw non-pointer values that must not be interpreted. With these,
  `localized_code_post_gfx_mp.ff` (3,605 assets) and `localized_common_mp.ff`
  (3,160 assets, 2,769 sound aliases, 703 loaded sounds, 322 materials) load
  to 100% with zone blocks filled to within alignment padding of their exact
  declared sizes; `code_post_gfx.ff` reaches 1,223/1,639 and stops at fx.
- Remaining asset deserializers: GfxLightDef, FxEffectDef (variable-length
  elemDefs with per-type visuals, trails, nested model/material/effect
  refs), FxImpactTable (12x33 effect handles), PhysPreset, StringTable (no
  block push, cell strings), XAnimParts (delta trans/quat with dynamic
  index/frame widths keyed on numframes), XModel (bone tables, XSurfaces
  with packed vertices in block 7 and triangle indices in block 8, 16-byte
  aligned; collision trees, brush wrappers, phys geoms), WeaponDef (2,168
  bytes — the 98-operation pointer walk is generated from the engine source
  into `kisak_weapondef_fields_android.inc`), and the menu chain (MenuList,
  menuDef_t, itemDef_s with type-dispatched data, expression statements with
  string operands, ItemKeyHandler chains). With these `code_post_gfx.ff`
  loads completely on device: 1,638/1,639 assets (the one remainder is the
  engine-skipped snddriverglobals) — 14 menus, the `none` weapon, the `void`
  xmodel, 11 xanims, 90 techsets, 82 materials, 33 images — with blocks 4,
  7 and 8 filled to their exact declared sizes (407,412/407,412, 4,224/4,224,
  480/480).
- First real menu rendered from game data: the loader now persists asset
  structs into an asset pool (the `DB_AddXAsset` role — temp-block structs
  are copied out before the rewind, referenced via pseudo-block 15), retains
  the zone blocks, and exposes a `KisakZoneView` accessor.
  `kisak_menu_scene_android.cpp` builds a flat quad list from a loaded
  `menuDef_t` (window/item rects, back colors, shader-style backgrounds via
  material -> texture def -> image name, localized item text laid out with
  real Glyph metrics), and the GLES renderer draws it on a letterboxed
  640x480 virtual canvas with per-texture batched runs. On device,
  `error_popmenu` renders with the `popups_alpha` gradient panel, the
  translucent overlay, and "Exit" drawn from the `devfonts` DXT5 atlas
  (localized from `@MENU_EXIT`). Textures stream from the IWDs through the
  IWI decoder at scene build time.
- Real single-player front end: the startup probe loads the engine's boot
  zones (`code_post_gfx.ff`, `common.ff`, `ui.ff` — 8,175 assets total) and
  the renderer shows the retail `main_text` menu with the layered smoke
  background (`bg_back`/`bg_fogscroll`/`bg_front` widescreen quads), the
  CoD4 logo, and the full item column. Touch input maps screen taps to the
  virtual 640x480 canvas hitboxes; item actions are parsed for
  `open <menu>` / `close self` script tokens and drive a menu navigation
  stack, with the Android Back key popping to the previous menu
  (`main_text` -> `levels` -> `main_text` validated on device).
- UI expression evaluator (`kisak_menu_expression_android.cpp`): a faithful
  64-bit port of `ui_expressions.cpp` — the shunting-yard
  `EvaluateExpression`/`RunOp` machinery with the exact
  `s_operatorPrecedence` table, operand lists, comma merging, and the
  function-call precedence class. Arithmetic, comparison, logic, dvar reads
  (small in-process dvar store), `localizestring` (with the leading-char
  escape convention), `tablelookup` over loaded StringTable assets,
  `secondsastime`, and safe zero defaults for game-state operators. The
  scene builder now evaluates `itemDef_s.visibleExp` plus the
  show/hide-dvar flags (`Item_EnableShowViaDvar` port) to hide
  conditional items, and `textExp` for dynamic labels. `main_text` drops
  from 75 to 42 rendered items (matching retail exactly) and `levels` hides
  its 31 locked-mission entries, leaving Back/MISSIONS/Prologue and the
  dimmed act headers.
- Menu script runtime (`kisak_menu_state_android.cpp`): the state-side subset
  of `Item_RunScript` — `setdvar`, `exec "set ..."` (with `;`-chained
  commands), `setLocalVarInt/Bool/Float/String`, `hide`/`show` (name or
  group match, case-insensitive, `*` wildcard, dynamic-visible overrides on
  top of the serialized `dynamicFlags` bit 4), `savegameshow`/`savegamehide`
  (SaveExists()=false for now, which yields the exact retail fresh-install
  main menu: New Game visible, Resume hidden), `open` reported to the caller,
  `close self` running the current menu's `onClose` before navigation,
  `setfocus`/`focusFirst` with the engine's focusable rules.
  Focus changes run `leaveFocus`/`onFocus` scripts; localvars feed the
  `localvar*` expression operators (highlight bars via
  `ui_highlight`/`ui_choicegroup` work as authored).
- Faithful `ScrPlace_ApplyRect` alignment (additive center/right/bottom
  anchors, fullscreen stretch), `materialExp` evaluation (with a fallback
  from material name to same-named IWI for 2D materials that only ship as
  loose images, e.g. `gradient_fadein`), and `Window_Paint`-accurate
  background tinting (SHADER styles use foreColor gated on dynamic flag
  0x10000, FILLED uses backColor) — the authored panel alphas finally
  render as designed.
- Renderer integration: menu onOpen runs once per navigation before the
  scene builds; touch-down gives the touched item focus and rebuilds the
  scene (decoded IWI textures now live in a persistent per-context cache so
  rebuilds are cheap); touch-up runs the full action script. Validated on
  device end-to-end: `main_text` -> `levels` -> `popmenu_act_pre` (F.N.G.
  entry with highlight bar, killhouse levelshot, mission description via
  the hide/show group machinery) -> `popmenu_difficulty`
  (Recruit/Regular/Hardened/Veteran) and back, no crash.
- Faithful text placement: `DrawWrappedText` / `Item_GetTextPlacementX/Y`
  ported — `textalignx/y` offsets inside the window rect, `textAlignMode & 3`
  horizontal anchoring (left/center/right against the text width),
  `& 0xC` baseline placement (raw/+height/centered/bottom), with
  `UI_TextHeight = scale * 48`. The serialized `textRect` ships empty, so
  placement is computed at scene build. Hit-testing uses the window rect
  like the engine's `Rect_ContainsPoint`. This separates the difficulty
  popup's title/subtitle and revealed that the retail SP menu text is
  right-aligned (alignMode 0xA, right edge at x=210, selection arrow at
  x=218) — the earlier left-aligned look was a port bug.
- Autowrapped text items (`window.staticFlags & 0x800000`) now wrap on word
  boundaries inside the authored item rect, advancing each line by
  `UI_TextHeight + 5` like `DrawWrappedText`. This keeps long Options help text
  inside its left help panel instead of drawing across the settings list.
- Interactive options items: `ITEM_TYPE_YESNO` now paints from its backing
  dvar as localized YES/NO, and `ITEM_TYPE_MULTI` paints/cycles its serialized
  `multiDef_s` string or float list before running the item's action script.
  `ITEM_TYPE_SLIDER` now renders the retail slider bar/thumb materials
  (`slider2`, `sliderbutt_1`) from its `editFieldDef_s` min/max range and maps
  touch X back into the backing dvar. Touch activation marks the scene dirty
  when the dvar changes, so Options controls rebuild immediately and reveal
  Apply-state UI. The menu navigation queue clears the onOpen sentinel per
  activation and the menu runtime resets per-menu visibility overrides before
  replaying `onOpen`; returning to `main_text` from Controls now runs the
  Options `onClose` (`ui_hideBack=0`) and restores the full main menu instead of
  leaving only background quads.
- Item expression rendering: the scene builder now applies `rectXExp`,
  `rectYExp`, `rectWExp`, `rectHExp`, and `forecolorAExp` before painting item
  backgrounds, text, listbox rows, and hitboxes. This covers dvar/localvar-driven
  item position, size, and alpha states at scene rebuild time. Statements using
  the `milliseconds` operator are detected while building the scene and mark the
  menu as time-dependent; the renderer then refreshes those scenes periodically
  without spamming logcat. This covers authored fade/alpha cycles such as the
  main-menu panels while leaving static menus untouched.
- ListBox/feeders: visible `ITEM_TYPE_LISTBOX` rows now render through an
  Android-side subset of `UI_FeederCount` / `UI_FeederItemText` with retained
  cursor/start state and row selection by touch. Feeder 24 is backed by an
  Android runtime profile list seeded with `AndroidPlayer`; `uiScript`
  `createPlayerProfile` adds the focused editfield value, `loadPlayerProfile`
  applies the selected profile dvars, `deletePlayerProfile` removes the selected
  row, and tapping `profilelist` updates `ui_playerProfileSelected`. The
  temporary profile list persists to `players/profiles_android.txt` under the
  Android COD4 data directory, so create/delete survives process restarts until
  the full COD4 profile filesystem is wired.
  Feeder 16 intentionally reports no savegames until real Android savegame
  scanning is wired.
- Editfields: profile creation now has focusable `ITEM_TYPE_EDITFIELD`-style
  controls backed by the item dvar. Android `KeyEvent`s are intercepted at the
  activity level and forwarded to native text input, so `adb shell input text`
  and hardware keyboards update the focused field and mark the menu scene dirty.
  Hit-testing now walks zones from topmost to backmost, matching draw order and
  preventing popup background/close layers from stealing taps from editfields.
- Key binds: `ITEM_TYPE_BIND` now paints current command assignments in Controls
  menus, enters the retail pending state (`@MENU_BIND_KEY_PENDING`) when tapped,
  and consumes the next Android key event. The temporary Android table mirrors
  the PC UI semantics: Escape cancels, Delete clears the command, a command can
  own at most two keys, and reusing a key moves it to the new command. This is
  also wired into the UI expression operator `keybinding(...)`, so dynamic menu
  strings and bind rows resolve through the same assignment table. This is still
  UI-side state only; gameplay input consumption will use the table once the real
  client input path is portable.
- Menu `exec` scripts now apply more of the console subset needed by Controls:
  newline/semicolon-separated cfg text, `//` comments, `set`/`seta`,
  `bind`/`unbind`/`unbindall`, nested `exec`, and RawFile-backed cfg lookup from
  loaded zones before falling back to loose/IWD VFS reads. The first loaded zone
  containing `default.cfg` seeds the temporary Android control table at startup,
  and the Set Default Controls confirmation reruns the same RawFile. On device
  this applies `unbindall`, restores dvars such as `cl_freelook`, and shows
  Lean Left/Right as `Q`/`E`.
- Menu launch commands: `devmap` and `map` commands reached through menu
  `exec` chains are now captured as pending launch requests and consumed by the
  renderer. Until the real gameplay/map loader is wired, they are logged instead
  of silently disappearing. The script runtime also handles the retail
  `execondvarstringvalue` branch used by the difficulty popup, so both direct
  New Game and Mission Select -> Prologue -> F.N.G. -> Recruit capture
  `devmap killhouse` from shipped menu data.
- Fastfile launch probe: pending `map`/`devmap` requests now probe
  `zone/english/<map>.ff` directly from the Android COD4 base path and inflate
  only enough zlib stream data to parse the `XFile` header, script string list,
  and `XAsset` directory. This keeps validation fast on device while the
  gameplay loader is still pending. On the current retail data set,
  `killhouse_load.ff` is absent and `killhouse.ff` reaches the full asset
  directory after one 256 KiB decompression chunk: `IWffu100 v5`, compressed
  size `70391800`, `XFile size=135666188`, `externalSize=379079576`, 892 script
  strings, and 1,684 assets (`xmodel:382`, `xanimparts:270`, `techniqueset:238`,
  `sound:466`, `rawfile:31`, `weapon:12`, plus map world assets).
- Negative-extent quads: the act popups' "classified folder" art (film
  perforation strip, mirrored gradient tab) is authored with negative rect
  widths that the engine draws mirrored; `EmitRect` normalizes the extent
  and flips the UVs. The `show name_*` onFocus commands reference items
  that don't exist in the shipped menus — the name bar is pure decoration.
- SP map zones now deserialize completely: the zone loader gained the four
  world asset types — `comworld` (primary lights), `gameworld_sp` (AI path
  graph incl. the recursive `pathnode_tree`), `clipmap` (collision world,
  which also owns the `map_ents` entity string through its own asset slot),
  and `gfxworld` (render world). `killhouse.ff` reaches 1,684/1,684 assets
  with byte-exact block usage, and `hunted`/`ac130`/`cargoship`/`coup` load
  100% on host as well. World materials (`wc/*`) never appear in the asset
  directory; they stream in through `GfxSurface::material` slots.
- Mission launch renders the world: activating `New Game` runs the menu's
  `devmap killhouse` launch command, a background thread decompresses and
  deserializes the map zone, and `kisak_world_scene_android.cpp` extracts the
  static GfxWorld draw scene (shared vertex/index pools, one draw range per
  surface, colormap picked by texture-def semantic 2, spawn point parsed from
  the `info_player_start` entity). The GLES renderer uploads a u32-indexed
  VBO/IBO, decodes the surfaces' IWI colormaps with REPEAT wrapping and
  mipmaps, and draws depth-tested runs with alpha test and src/inv-src alpha
  blending derived from each material's `GfxStateBits` (`GFXS0_ATEST_*`,
  blend nibbles). A one-finger camera walks the map: left half of the screen
  is a virtual move stick at COD run speed, right half looks, and the Android
  back button returns to the front-end. Verified on device in killhouse: the
  shooting range, target silhouettes, banners, and gravel paths render with
  correct perspective at interactive framerates.
- Static models render through GLES3 instancing: the scene builder groups
  `GfxWorldDpvsStatic::smodelDrawInsts` by XModel, extracts each unique
  model's LOD0 `XSurface` geometry once (packed 32-byte vertices with
  half-float texcoords decoded on extraction), and stores 12 floats per
  instance (scaled basis rows + origin). A second shader program consumes the
  instance stream through divisor-1 attributes and issues one
  `glDrawElementsInstanced` per (model, surface), with materials resolved to
  colormap/alpha-test/blend state exactly like world surfaces (cross-zone
  image references drop their leading-comma name prefix before the IWI
  lookup). killhouse: 238 unique meshes, 12,188 instances, 300k pooled
  vertices — trees, telegraph poles, army trucks, firing-range furniture, and
  signage all appear on device, drawn in four phases (opaque world → opaque
  models → blended world → blended models).
- Baked lighting is live: zone-embedded image payloads persist out of the
  rewinding temp block, `DecodeD3DTopMip` decodes them (mip 0 first, unlike
  .iwi files), and the world shader implements the IW3 split — the secondary
  lightmap page is 2x-overbright radiosity color and the primary page an L8
  sun-visibility mask gating the GfxLight sun color. Draw state now comes
  from the material's technique-indexed GfxStateBits entry (lit_sun > lit >
  emissive > unlit) with real D3DBLEND factors per run, which fixed the
  tarmac puddle decals rendering opaque white. The EGL config gained an
  actual depth buffer (it was 0-bit — depth testing had silently been a
  no-op), world IWI decoding moved off the render thread (GL init dropped
  from 145 s, which got the app frozen by MIUI, to under 2 s), and static
  models cull back faces while world geometry stays double-sided for now
  (brush and terrain surfaces wind in opposite directions in the raw data).
  killhouse renders with baked tree/building shadows on the tarmac at 26-29
  fps on the Adreno 610 test device.
- The sky renders from the map's real cubemap: `DecodeIwiCubemap` unpacks
  the 6 faces of a cubemap .iwi (flag 0x4, faces consecutive per mip level,
  top mip last), which are laid out on world axes (+X,-X,+Y,-Y,+Z=up,-Z) —
  the sky shader therefore samples with the raw world-space view direction,
  no Y-up swizzle. Sky surfaces (any `wc/sky*` material) skip the normal
  passes and draw pinned to the far plane after opaque geometry.
- The camera walks instead of flying: scene extraction rasterizes
  upward-facing world triangles into a 40-unit heightfield (per-cell sorted
  floor heights), and the camera moves horizontally with smoothed ground
  follow (45-unit step-up, 60-unit eye height), spawning on the floor at
  info_player_start. Verified on device by walking across the training camp
  and through the hangar door into the building interior at a steady ~29
  fps.
- Walls block movement: the scene builder extracts the clipmap's
  player-solid brushes (contents solid|playerclip; the six axial planes are
  implicit in the brush AABB, `sides` only holds non-axial planes) and the
  camera resolves as a 16-unit sphere pushed out of every overlapping convex
  brush each frame (three Gauss-Seidel iterations, brute force with AABB
  early-out over killhouse's 13,751 brushes). Verified on device: the hangar
  facade stops the player and the camera slides along the wall like a real
  FPS, still at ~29 fps.

## Data policy

Users must provide their own legally owned COD4 installation files. The app will
look for them in the app external files directory shown on screen, inside a
`cod4` folder. Do not commit game data, copied retail files, Steam files, Bink
videos, or Miles runtime binaries.

## Porting order

1. Add a portable platform layer for process lifecycle, filesystem, logging,
   timers, networking, and threading.
2. Replace `src/win32` entry/input/storage/network code with Android/SDL
   equivalents.
3. Stub Steam for local/offline bring-up, then add an optional legal auth path.
4. Replace Miles/DirectSound with SDL audio or AAudio.
5. Replace Bink playback with a disabled path first, then a legal pluggable
   decoder if available.
6. Replace Direct3D 9 renderer with Vulkan or OpenGL ES.
7. Load the real COD4 filesystem from user-provided storage and reach menu.
8. Bring up map load, gameplay, input mapping, audio, and performance tuning.

## Build

```bash
./gradlew :android:app:assembleDebug
```

## Device check

```bash
./gradlew :android:app:installDebug
adb shell am start -n com.kisakcod.android/.MainActivity
```

The first screen reports the exact folder to populate, scans for `main/`,
`zone/`, `.iwd`, localized `.iwd`, and `.ff` files, then builds a lightweight
Android filesystem mount. A valid mount proves that loose files, FastFiles, and
assets inside IWD ZIP archives can be resolved from app external storage. It
also prints the exact engine filesystem startup plan that should be wired into
`com_files.cpp` once the broader engine runtime is portable enough to call it,
and it reads representative IWD-backed files such as `fileSysCheck.cfg`.
