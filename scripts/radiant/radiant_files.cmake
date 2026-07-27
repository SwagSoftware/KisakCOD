# radiant_files.cmake — editor source files for KisakCOD-Radiant
# Phase 1: scaffold + stubs.
# Phase 2: engine subset appended at the bottom.
# NOTE: UNIVERSAL/XANIM/COMMON come from common_files.cmake (included in CMakeLists.txt).

set(RADIANT_SRCS

    # ── App scaffold ─────────────────────────────────────────────────────────
    "${SRC_DIR}/radiant/radiantapp.cpp"
    "${SRC_DIR}/radiant/prefs.cpp"          # CPrefsDlg settings object + registry persistence (P6)
    "${SRC_DIR}/radiant/prefs.h"
    "${SRC_DIR}/radiant/stdafx.h"
    "${SRC_DIR}/radiant/mainfrm.h"
    "${SRC_DIR}/radiant/res/resource.h"
    "${SRC_DIR}/radiant/res/radiant.rc"

    # ── Tier 1 — math/geometry foundation ────────────────────────────────────
    "${SRC_DIR}/radiant/mainfrm.cpp"        # CMainFrame (387 methods; stub for P1)
    "${SRC_DIR}/radiant/cmdlib.cpp"         # TODO_RADIANT Phase 2 (libs/cmdlib reference)
    "${SRC_DIR}/radiant/winding.cpp"
    "${SRC_DIR}/radiant/linearmapping.cpp"  # universal/linearmapping.cpp — double 3x3 LU solver (texture-lock reproject)
    "${SRC_DIR}/radiant/linearmapping.h"
    "${SRC_DIR}/radiant/primarylights_region.cpp"  # light-region CSG (common/primarylights_region.cpp)
    "${SRC_DIR}/radiant/eclass.cpp"
    "${SRC_DIR}/radiant/brush.cpp"
    "${SRC_DIR}/radiant/csg.cpp"
    "${SRC_DIR}/radiant/entity.cpp"

    # ── Tier 2 — map model & operations ──────────────────────────────────────
    "${SRC_DIR}/radiant/map.cpp"
    "${SRC_DIR}/radiant/select.cpp"
    "${SRC_DIR}/radiant/drag.cpp"
    "${SRC_DIR}/radiant/undo.cpp"
    "${SRC_DIR}/radiant/pmesh.cpp"
    "${SRC_DIR}/radiant/points.cpp"
    "${SRC_DIR}/radiant/errorfile.cpp"
    "${SRC_DIR}/radiant/mapinfo.cpp"

    # ── Tier 3 — CoD-specific editor systems ─────────────────────────────────
    "${SRC_DIR}/radiant/materialdef.cpp"
    "${SRC_DIR}/radiant/scriptgroup.cpp"
    "${SRC_DIR}/radiant/layers.cpp"
    "${SRC_DIR}/radiant/filters.cpp"
    "${SRC_DIR}/radiant/filtersettings.cpp"
    "${SRC_DIR}/radiant/shadowvolume.cpp"
    "${SRC_DIR}/radiant/layeredmaterials.cpp"
    "${SRC_DIR}/radiant/layeredmaterialwnd.cpp"

    # ── Tier 4 — render / glue ────────────────────────────────────────────────
    "${SRC_DIR}/radiant/qedefs.h"          # P3: editor primitive types & enums
    "${SRC_DIR}/radiant/qe3.h"             # P3: editor object model + layout static_asserts
    "${SRC_DIR}/radiant/qe3.cpp"
    "${SRC_DIR}/radiant/win_qe3.cpp"
    "${SRC_DIR}/radiant/gfxwrapper.cpp"
    "${SRC_DIR}/radiant/draw.cpp"

    # ── Phase 5 — MFC windows ────────────────────────────────────────────────
    "${SRC_DIR}/radiant/xywnd.cpp"
    "${SRC_DIR}/radiant/camwnd.cpp"
    "${SRC_DIR}/radiant/z.cpp"
    "${SRC_DIR}/radiant/texwnd.cpp"
    "${SRC_DIR}/radiant/texturebar.cpp"
    "${SRC_DIR}/radiant/win_ent.cpp"

    # ── Phase 6 — dialogs ────────────────────────────────────────────────────
    "${SRC_DIR}/radiant/surfacedlg.cpp"
    "${SRC_DIR}/radiant/entitylist.cpp"     # CEntityListDlg — Edit→Entity Info entity browser (tree + K/V)
    "${SRC_DIR}/radiant/findtexture.cpp"    # CFindTextureDlg + FindReplaceTextures (find/replace texture)
    "${SRC_DIR}/radiant/patchdialog.cpp"
    "${SRC_DIR}/radiant/win_dlg.cpp"
    "${SRC_DIR}/radiant/verteditdlg.cpp"
    "${SRC_DIR}/radiant/layersdlg.cpp"
    "${SRC_DIR}/radiant/dynentitydlg.cpp"
    "${SRC_DIR}/radiant/vehicledlg.cpp"
    "${SRC_DIR}/radiant/modeldlg.cpp"
    "${SRC_DIR}/radiant/mayaexport.cpp"     # Misc->Maya Export — the .mel MEL-script geometry exporter

    # ── Phase 2 — editor-only renderer files (OPUS stubs) ────────────────────
    # These have no kisak equivalents; fresh decompile + OPUS queue items.
    "${SRC_DIR}/radiant/r_ed_scene.cpp"
    "${SRC_DIR}/radiant/r_ed_vertbuf.cpp"

    # ── Build 10 — engine stubs (globals + function stubs for excluded files) ─
    "${SRC_DIR}/radiant/engine_stubs.cpp"
)

# ─────────────────────────────────────────────────────────────────────────────
# Phase 2: engine file subsets
# UNIVERSAL, XANIM, COMMON defined in common_files.cmake.
# ─────────────────────────────────────────────────────────────────────────────

# script/ — only the two utility files Radiant uses (not the full VM)
set(RADIANT_SCRIPT
    "${SRC_DIR}/script/scr_memorytree.cpp"
    "${SRC_DIR}/script/scr_memorytree.h"
    "${SRC_DIR}/script/scr_stringlist.cpp"
    "${SRC_DIR}/script/scr_stringlist.h"
)

# physics/ — load-obj + ODE mass/matrix + ODE math/error support
set(RADIANT_PHYSICS
    "${SRC_DIR}/physics/physpreset_load_obj.cpp"
    "${SRC_DIR}/physics/ode/mass.cpp"
    "${SRC_DIR}/physics/ode/matrix.cpp"
    "${SRC_DIR}/physics/ode/error.cpp"
    "${SRC_DIR}/physics/ode/fastdot.c"
    "${SRC_DIR}/physics/ode/fastlsolve.c"
    "${SRC_DIR}/physics/ode/fastltsolve.c"
)

# qcommon/ — BSP API + console command layer + file access + pack funcs + mem track
set(RADIANT_QCOMMON
    "${SRC_DIR}/qcommon/com_bsp.cpp"
    "${SRC_DIR}/qcommon/com_bsp.h"
    "${SRC_DIR}/qcommon/com_bsp_load_obj.cpp"
    "${SRC_DIR}/qcommon/cmd.cpp"
    "${SRC_DIR}/qcommon/cmd.h"
    "${SRC_DIR}/qcommon/com_fileaccess.cpp"
    "${SRC_DIR}/qcommon/com_fileaccess.h"
    "${SRC_DIR}/qcommon/com_pack.cpp"
    "${SRC_DIR}/qcommon/com_pack.h"
    "${SRC_DIR}/qcommon/mem_track.cpp"
    "${SRC_DIR}/qcommon/mem_track.h"
    # iwd (ZIP) reader — stock images live in iw_*.iwd; the editor must read them
    # (materials/shaders are loose in raw/, but .iwi images are only in iwds).
    # Replaces the no-op unz* stubs in engine_stubs.cpp.
    "${SRC_DIR}/qcommon/unzip.cpp"
    "${SRC_DIR}/qcommon/unzip.h"
)

# gfx_d3d/ — filtered: game-only files excluded per file_map.json cross-reference.
# Excluded (unsafe includes): r_cinematic (mss.h+snd_local.h),
#   r_devgui, rb_drawprofile (scr_parser.h), r_draw_pixelshader (no header).
# r_model_skin_sse RE-INCLUDED 2026-07-25: its includes are clean now (q_shared/
#   r_model_skin.h/xanim/profile/intrinsics) and the master merge (66b37ca5) made
#   r_model_skin.cpp call R_SkinXSurfaceSkinnedSse unconditionally at link time;
#   sys_SSE is registered by the engine_stubs GROUP-C shim.
# Added in Build 10: r_cmds, r_workercmds_common, rb_stats, rb_uploadshaders,
#   rb_logfile, rb_shadowcookie, rb_showcollision, r_draw_lit, r_draw_sunshadow,
#   r_dpvs_dynmodel, r_fog, r_screenshot, r_texturemem.
set(RADIANT_GFX_D3D
    "${SRC_DIR}/gfx_d3d/fxprimitives.h"
    "${SRC_DIR}/gfx_d3d/rb_backend.cpp"
    "${SRC_DIR}/gfx_d3d/rb_backend.h"
    "${SRC_DIR}/gfx_d3d/rb_debug.cpp"
    "${SRC_DIR}/gfx_d3d/rb_debug.h"
    "${SRC_DIR}/gfx_d3d/rb_depthprepass.cpp"
    "${SRC_DIR}/gfx_d3d/rb_depthprepass.h"
    "${SRC_DIR}/gfx_d3d/rb_draw3d.cpp"
    "${SRC_DIR}/gfx_d3d/rb_draw3d.h"
    "${SRC_DIR}/gfx_d3d/rb_fog.cpp"
    "${SRC_DIR}/gfx_d3d/rb_fog.h"
    "${SRC_DIR}/gfx_d3d/rb_imagefilter.cpp"
    "${SRC_DIR}/gfx_d3d/rb_imagefilter.h"
    "${SRC_DIR}/gfx_d3d/rb_imagetouch.cpp"
    "${SRC_DIR}/gfx_d3d/rb_light.cpp"
    "${SRC_DIR}/gfx_d3d/rb_light.h"
    "${SRC_DIR}/gfx_d3d/rb_pixelcost.cpp"
    "${SRC_DIR}/gfx_d3d/rb_pixelcost.h"
    "${SRC_DIR}/gfx_d3d/rb_postfx.cpp"
    "${SRC_DIR}/gfx_d3d/rb_postfx.h"
    "${SRC_DIR}/gfx_d3d/rb_shade.cpp"
    "${SRC_DIR}/gfx_d3d/rb_shade.h"
    "${SRC_DIR}/gfx_d3d/rb_sky.cpp"
    "${SRC_DIR}/gfx_d3d/rb_sky.h"
    "${SRC_DIR}/gfx_d3d/rb_spotshadow.cpp"
    "${SRC_DIR}/gfx_d3d/rb_spotshadow.h"
    "${SRC_DIR}/gfx_d3d/rb_state.cpp"
    "${SRC_DIR}/gfx_d3d/rb_state.h"
    "${SRC_DIR}/gfx_d3d/rb_sunshadow.cpp"
    "${SRC_DIR}/gfx_d3d/rb_sunshadow.h"
    "${SRC_DIR}/gfx_d3d/rb_tess.cpp"
    "${SRC_DIR}/gfx_d3d/rb_tess.h"
    "${SRC_DIR}/gfx_d3d/r_add_bsp.cpp"
    "${SRC_DIR}/gfx_d3d/r_add_cmdbuf.cpp"
    "${SRC_DIR}/gfx_d3d/r_add_staticmodel.cpp"
    "${SRC_DIR}/gfx_d3d/r_add_staticmodel.h"
    "${SRC_DIR}/gfx_d3d/r_bsp.cpp"
    "${SRC_DIR}/gfx_d3d/r_bsp.h"
    "${SRC_DIR}/gfx_d3d/r_bsp_load_obj.cpp"
    "${SRC_DIR}/gfx_d3d/r_buffers.cpp"
    "${SRC_DIR}/gfx_d3d/r_buffers.h"
    "${SRC_DIR}/gfx_d3d/r_cmdbuf.cpp"
    "${SRC_DIR}/gfx_d3d/r_cmdbuf.h"
    "${SRC_DIR}/gfx_d3d/r_debug.cpp"
    "${SRC_DIR}/gfx_d3d/r_debug.h"
    "${SRC_DIR}/gfx_d3d/r_debug_alloc.cpp"
    "${SRC_DIR}/gfx_d3d/r_dobj_skin.cpp"
    "${SRC_DIR}/gfx_d3d/r_dobj_skin.h"
    "${SRC_DIR}/gfx_d3d/r_dpvs.cpp"
    "${SRC_DIR}/gfx_d3d/r_dpvs.h"
    "${SRC_DIR}/gfx_d3d/r_dpvs_entity.cpp"
    "${SRC_DIR}/gfx_d3d/r_dpvs_sceneent.cpp"
    "${SRC_DIR}/gfx_d3d/r_dpvs_static.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_bsp.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_bsp.h"
    "${SRC_DIR}/gfx_d3d/r_draw_material.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_material.h"
    "${SRC_DIR}/gfx_d3d/r_draw_method.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_method.h"
    "${SRC_DIR}/gfx_d3d/r_draw_shadowable_light.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_shadowable_light.h"
    "${SRC_DIR}/gfx_d3d/r_draw_staticmodel.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_staticmodel.h"
    "${SRC_DIR}/gfx_d3d/r_draw_xmodel.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_xmodel.h"
    "${SRC_DIR}/gfx_d3d/r_drawsurf.cpp"
    "${SRC_DIR}/gfx_d3d/r_drawsurf.h"
    "${SRC_DIR}/gfx_d3d/r_dvars.cpp"
    "${SRC_DIR}/gfx_d3d/r_dvars.h"
    "${SRC_DIR}/gfx_d3d/r_font.cpp"
    "${SRC_DIR}/gfx_d3d/r_font.h"
    "${SRC_DIR}/gfx_d3d/r_gfx.h"
    "${SRC_DIR}/gfx_d3d/r_image.cpp"
    "${SRC_DIR}/gfx_d3d/r_image.h"
    "${SRC_DIR}/gfx_d3d/r_imagedecode.cpp"
    "${SRC_DIR}/gfx_d3d/r_image_load_common.cpp"
    "${SRC_DIR}/gfx_d3d/r_image_load_db.h"
    "${SRC_DIR}/gfx_d3d/r_image_load_obj.cpp"
    "${SRC_DIR}/gfx_d3d/r_image_utils.cpp"
    "${SRC_DIR}/gfx_d3d/r_image_wavelet.cpp"
    "${SRC_DIR}/gfx_d3d/r_init.cpp"
    "${SRC_DIR}/gfx_d3d/r_init.h"
    "${SRC_DIR}/gfx_d3d/r_light.cpp"
    "${SRC_DIR}/gfx_d3d/r_light.h"
    "${SRC_DIR}/gfx_d3d/r_light_load_obj.cpp"
    "${SRC_DIR}/gfx_d3d/r_marks.cpp"
    "${SRC_DIR}/gfx_d3d/r_marks.h"
    "${SRC_DIR}/gfx_d3d/r_material.cpp"
    "${SRC_DIR}/gfx_d3d/r_material.h"
    "${SRC_DIR}/gfx_d3d/r_material_load_db.h"
    "${SRC_DIR}/gfx_d3d/r_material_load_obj.cpp"
    "${SRC_DIR}/gfx_d3d/r_material_override.cpp"
    "${SRC_DIR}/gfx_d3d/r_meshdata.cpp"
    "${SRC_DIR}/gfx_d3d/r_meshdata.h"
    "${SRC_DIR}/gfx_d3d/r_model.cpp"
    "${SRC_DIR}/gfx_d3d/r_model.h"
    "${SRC_DIR}/gfx_d3d/r_model_lighting.cpp"
    "${SRC_DIR}/gfx_d3d/r_model_lighting.h"
    "${SRC_DIR}/gfx_d3d/r_model_pose.cpp"
    "${SRC_DIR}/gfx_d3d/r_model_pose.h"
    "${SRC_DIR}/gfx_d3d/r_model_skin.cpp"
    "${SRC_DIR}/gfx_d3d/r_model_skin.h"
    "${SRC_DIR}/gfx_d3d/r_model_skin_sse.cpp"
    "${SRC_DIR}/gfx_d3d/r_outdoor.cpp"
    "${SRC_DIR}/gfx_d3d/r_outdoor.h"
    "${SRC_DIR}/gfx_d3d/r_pixelcost_load_obj.cpp"
    "${SRC_DIR}/gfx_d3d/r_pixelcost_load_obj.h"
    "${SRC_DIR}/gfx_d3d/r_pretess.cpp"
    "${SRC_DIR}/gfx_d3d/r_pretess.h"
    "${SRC_DIR}/gfx_d3d/r_primarylights.cpp"
    "${SRC_DIR}/gfx_d3d/r_primarylights.h"
    "${SRC_DIR}/gfx_d3d/r_reflection_probe.cpp"
    "${SRC_DIR}/gfx_d3d/r_reflection_probe.h"
    "${SRC_DIR}/gfx_d3d/r_reflection_probe_load_obj.cpp"
    "${SRC_DIR}/gfx_d3d/r_rendercmds.cpp"
    "${SRC_DIR}/gfx_d3d/r_rendercmds.h"
    "${SRC_DIR}/gfx_d3d/r_rendertarget.cpp"
    "${SRC_DIR}/gfx_d3d/r_rendertarget.h"
    "${SRC_DIR}/gfx_d3d/r_scene.cpp"
    "${SRC_DIR}/gfx_d3d/r_scene.h"
    "${SRC_DIR}/gfx_d3d/r_setstate_d3d.cpp"
    "${SRC_DIR}/gfx_d3d/r_setstate_d3d.h"
    "${SRC_DIR}/gfx_d3d/r_shade.cpp"
    "${SRC_DIR}/gfx_d3d/r_shade.h"
    "${SRC_DIR}/gfx_d3d/r_shadowcookie.cpp"
    "${SRC_DIR}/gfx_d3d/r_shadowcookie.h"
    "${SRC_DIR}/gfx_d3d/r_sky.cpp"
    "${SRC_DIR}/gfx_d3d/r_sky.h"
    # r_sky_load_obj.cpp folded into r_sky.cpp in KisakCOD
    "${SRC_DIR}/gfx_d3d/r_spotshadow.cpp"
    "${SRC_DIR}/gfx_d3d/r_spotshadow.h"
    "${SRC_DIR}/gfx_d3d/r_state.cpp"
    "${SRC_DIR}/gfx_d3d/r_state.h"
    "${SRC_DIR}/gfx_d3d/r_state_utils.cpp"
    # r_staticmodel_load_obj.cpp folded into r_staticmodel.cpp in KisakCOD; use r_staticmodel.cpp
    "${SRC_DIR}/gfx_d3d/r_staticmodel.cpp"
    "${SRC_DIR}/gfx_d3d/r_staticmodelcache.cpp"
    "${SRC_DIR}/gfx_d3d/r_sunshadow.cpp"
    "${SRC_DIR}/gfx_d3d/r_sunshadow.h"
    "${SRC_DIR}/gfx_d3d/r_utils.cpp"
    "${SRC_DIR}/gfx_d3d/r_utils.h"
    "${SRC_DIR}/gfx_d3d/r_warn.cpp"
    "${SRC_DIR}/gfx_d3d/r_water.cpp"
    "${SRC_DIR}/gfx_d3d/r_water.h"
    "${SRC_DIR}/gfx_d3d/r_water_load_obj.cpp"
    "${SRC_DIR}/gfx_d3d/r_workercmds.cpp"
    "${SRC_DIR}/gfx_d3d/r_workercmds.h"
    "${SRC_DIR}/gfx_d3d/r_xsurface.cpp"
    "${SRC_DIR}/gfx_d3d/r_xsurface.h"
    # r_xsurface_load_obj.cpp folded into r_xsurface.cpp in KisakCOD

    # ── Build 10 additions ─────────────────────────────────────────────────────
    "${SRC_DIR}/gfx_d3d/r_cmds.cpp"
    "${SRC_DIR}/gfx_d3d/r_cmds.h"
    "${SRC_DIR}/gfx_d3d/r_draw_lit.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_lit.h"
    "${SRC_DIR}/gfx_d3d/r_draw_pixelshader.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_sunshadow.cpp"
    "${SRC_DIR}/gfx_d3d/r_draw_sunshadow.h"
    "${SRC_DIR}/gfx_d3d/r_dpvs_dynmodel.cpp"
    "${SRC_DIR}/gfx_d3d/r_fog.cpp"
    "${SRC_DIR}/gfx_d3d/r_fog.h"
    "${SRC_DIR}/gfx_d3d/r_screenshot.cpp"
    "${SRC_DIR}/gfx_d3d/r_screenshot.h"
    "${SRC_DIR}/gfx_d3d/r_texturemem.cpp"
    "${SRC_DIR}/gfx_d3d/r_texturemem.h"
    "${SRC_DIR}/gfx_d3d/r_workercmds_common.cpp"
    "${SRC_DIR}/gfx_d3d/r_workercmds_common.h"
    "${SRC_DIR}/gfx_d3d/rb_logfile.cpp"
    "${SRC_DIR}/gfx_d3d/rb_logfile.h"
    "${SRC_DIR}/gfx_d3d/rb_shadowcookie.cpp"
    "${SRC_DIR}/gfx_d3d/rb_shadowcookie.h"
    "${SRC_DIR}/gfx_d3d/rb_showcollision.cpp"
    "${SRC_DIR}/gfx_d3d/rb_showcollision.h"
    "${SRC_DIR}/gfx_d3d/rb_stats.cpp"
    "${SRC_DIR}/gfx_d3d/rb_stats.h"
    "${SRC_DIR}/gfx_d3d/rb_uploadshaders.cpp"
    "${SRC_DIR}/gfx_d3d/rb_uploadshaders.h"
)
