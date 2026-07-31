#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// radiant/camwnd.cpp — the CCamWnd 3D perspective camera view (CoD4Radiant.exe).
// KISAK: the world draw is IMMEDIATE-MODE (R_AddRenderCmdDrawTris) plus the editor
// surf-cache passes; the binary routes everything through DrawGeneralWorld_/DrawGeo.
// NOTE: 0x403d30 is CamWnd_DropModelsToPlane, NOT Cam_Draw (0x407dc0).

#include "stdafx.h"
#include <csetjmp>                  // model-load asset-drop recovery guard (Stage B)
#include <universal/q_parse.h>      // Com_GetParseThreadInfo / negativeNumbers (collmap parse)
#include "mainfrm.h"                // CCamWnd, camera_s
#include "qe3.h"                    // g_qeglobals, selbrush_t, brush_t, face_t, MaterialDef, qtexture_s
#include "prefs.h"                  // g_PrefsDlg (camera_fov, enable_light_preview, preview_sun_aswell)
#include <gfx_d3d/r_gfx.h>          // GfxMatrix, GfxColor
#include <gfx_d3d/r_material.h>     // Material
#include <gfx_d3d/r_init.h>         // dx, R_SetupRendertarget_CheckDevice, R_Hwnd_Resize
#include <gfx_d3d/r_scene.h>        // R_Ed_SetSceneParms
#include <gfx_d3d/r_rendercmds.h>   // R_BeginFrame/EndFrame, clear/material-color, MaterialTechniqueType
#include <gfx_d3d/r_state.h>        // CONST_SRC_CODE_SUN_POSITION/DIFFUSE/SPECULAR
#include <universal/com_math.h>     // AngleVectors
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern selbrush_t active_brushes;                              // map.cpp (0x23F189C)
extern void       Radiant_FL_Log( const char *fmt, ... );      // mainfrm.cpp
extern int        g_nUpdateBits;                               // 0x25D5A74 (mainfrm.cpp)
// Brush_Ray (0x475fe0) headless-safe distance wrapper — select.cpp.
extern bool       Ed_BrushFloorRay( brush_t *def, const float *start,
                                     const float *dir, float *outDist );

// R_AddRenderCmdDrawTris — r_rendercmds.cpp KISAK_RADIANT editor delta (P5.4).
extern void __cdecl R_AddRenderCmdDrawTris(
    Material *material, MaterialTechniqueType techType, short indexCount,
    const uint16_t *indices, short vertexCount,
    const float (*xyzw)[4], const float (*normal)[3], float *color,
    const float (*st)[2] );

extern int  MaterialDef_11( MaterialDef *m );                 // materialdef.cpp — layer count
extern float world_orient_matrix[4][3];                       // entity.cpp (identity orientation)
extern char Byte4PackPixelColor( float *from, GfxColor *out );// 0x402ac0 (qe3/engine_stubs)
extern void Draw_PatchSelectPoints();                         // brush.cpp (0x40c250) — sel-curve-point overlay
extern void DrawBrush( selbrush_t *b, const orientation_t *orient, int viewType,
                       int technique, GfxColor *col, char width, int drawFlags,
                       const char *layerPrefix );             // brush.cpp (0x47afc0)
// Face_BuildLayerGeom + EdLayerGeom are declared in qe3.h.
extern void SunPrev_Setup();                                  // brush.cpp (Stage 2) — read worldspawn sun
extern int  SunPrev_Active();                                 // brush.cpp — sun key present
extern void SunPrev_FaceShade( const float *wn, float *out4 );// brush.cpp — per-face sun colour
extern int  Sys_Printf( const char *fmt, ... );               // win_qe3.cpp — editor console (0x499e90)

extern entity_s *world_entity;   // entity.cpp 0x25D5B30 — worldspawn (carries the sun keys)
// kisak's BSP sun-light parser/interpreter (r_bsp_load_obj.cpp) — the same pair the binary's
// R_SunPrev_SetSunConstants feeds the worldspawn text to.  Not header-declared for the editor.
extern "C++" char *__cdecl R_ParseSunLight( SunLightParseParams *params, char *text );
extern void __cdecl R_InterpretSunLightParseParamsIntoLights( SunLightParseParams *sunParse, GfxLight *sunLight );

// 0x403470  Cam_BuildMatrix — view basis from the angles.  AngleVectors wants pitch NEGATED;
// forward/right are the yaw-plane movement basis (the binary's AngleVectors_YawPlane).
void CCamWnd::Cam_BuildMatrix()
{
    float a[3] = { -camera.angles[0], camera.angles[1], camera.angles[2] };
    AngleVectors( a, camera.vpn, camera.vright, camera.vup );

    float flat[3] = { 0.0f, camera.angles[1], 0.0f };
    float up[3];
    AngleVectors( flat, camera.forward, camera.right, up );   // yaw-plane forward/right
}

// Cam_Draw's prologue: the R_SetupScene (0x506570) perspective projection + R_Ed_SetSceneParms
// with axis = { vpn, -vright, vup }.  R_SetupProjection (0x4a78e0) is inlined.
// KISAK: R_Ed_ProjectionWouldBeValid has no binary counterpart — it is a call-site guard around
// the shared r_state_utils.cpp:20 inverse-VP assert.  On far-from-origin maps (blackout at world
// X ~ -175000) the float32 MatrixInverse44 loses the sign of the inverse-VP's m[3][3], so the
// assert fires on a projection the forward render handles fine.  Returns false to drop just that
// frame (the cleared background stands) instead of crashing.
bool CCamWnd::Cam_SetupScene()
{
    Cam_BuildMatrix();

    float axis[3][3];
    axis[0][0] =  camera.vpn[0];    axis[0][1] =  camera.vpn[1];    axis[0][2] =  camera.vpn[2];
    axis[1][0] = -camera.vright[0]; axis[1][1] = -camera.vright[1]; axis[1][2] = -camera.vright[2];
    axis[2][0] =  camera.vup[0];    axis[2][1] =  camera.vup[1];    axis[2][2] =  camera.vup[2];

    const float fov  = g_PrefsDlg->camera_fov;                      // "Fov" pref (default 65)
    const int   w    = camera.width  > 0 ? camera.width  : 1;
    const int   h    = camera.height > 0 ? camera.height : 1;
    float tanY = tanf( DEG2RAD( fov ) * 0.5f ) * 0.75f;
    float tanX = tanY * (float)w / (float)h;
    // R_SetupScene 0x506570: zNear = max(r_znear, 0.01).  0.01 is the CLAMP FLOOR, not the near
    // plane — using it directly makes the depth range ~400x too coarse (coplanar tool faces Z-fight).
    float zNear = (float)Dvar_GetFloat( "r_znear" );                 // default 4.0
    if ( 0.0099999998f - zNear >= 0.0f )                            // 0x5065b0: floor at 0.01
        zNear = 0.0099999998f;

    GfxMatrix proj;
    memset( &proj, 0, sizeof( proj ) );
    proj.m[0][0] = 0.99951171875f / tanX;
    proj.m[1][1] = 0.99951171875f / tanY;
    proj.m[2][2] = 0.99951172f;
    proj.m[2][3] = 1.0f;
    proj.m[3][2] = 0.99951171875f * -zNear;

    if ( !R_Ed_ProjectionWouldBeValid( camera.origin, (const float (*)[3])axis, &proj ) )
    {
        static int s_camProjDropped = 0;
        s_camProjDropped++;
        if ( ( s_camProjDropped & ( s_camProjDropped - 1 ) ) == 0 )   // log at 1,2,4,8,... (no spam)
            Radiant_FL_Log( "Cam_SetupScene: dropped degenerate-projection frame #%d "
                            "(org=%.0f,%.0f,%.0f ang=%.2f,%.2f fov=%.1f)",
                            s_camProjDropped, camera.origin[0], camera.origin[1], camera.origin[2],
                            camera.angles[0], camera.angles[1], fov );
        return false;
    }

    R_Ed_SetSceneParms( camera.origin, (const float (*)[3])axis, &proj );
    return true;
}

// Cubic / frustum clip setup + test (Cam_Fov 0x405460, sub_405620, CullCubic 0x4056d0).
// Load-bearing on dense maps: without the cull EVERY prefab-content model is skinned and the
// r_ed_scene tempSkinBuf overflows (~1264 models), silently DROPPING later models.
extern void  Vec3Cross( const float *a, const float *b, float *out );              // 0x40A4D0 (out = a × b)
extern void  VectorRotateByAxis( float *out, const float *axisMatrix, const float *dir ); // 0x4BA6B0
extern void  OrientationWorldPosToLocalPos( float *out, const float *pos,
                                            const orientation_t *orient );          // 0x4BA610
extern float world_orient_matrix[4][3];                                             // entity.cpp 0x6DE290
extern void  MaterialDef_02( MaterialDef *m, int ( *cb )( qtexture_s * ) );         // materialdef.cpp 0x431520
extern int   MaterialDef_09( qtexture_s *radMtl );                                  // materialdef.cpp 0x431A00
extern int   dword_181F51C;                                                         // engine_stubs.cpp (realize-state)

// 0x405460  Cam_Fov — compute the two WORLD-space frustum SIDE planes from the camera
// basis + camera_fov, then set up the identity-world clip planes for CullCubic.  Each
// side plane's normal is stored (clipWorldNormal[0/1]) plus a 3-entry axis selector
// (clipWorldAxis[0/1]) picking, per component, the box corner (0/1/2 = mins, 3/4/5 =
// maxs) that is farthest along that normal — the classic Q3 axial box/plane test.
void CCamWnd::Cam_Fov()
{
    float vec[3];
    // v4 = tan(fov/2) * 0.75 * (width/height); v3 = -v4  (0x405491..0x4054b0)
    float v4 = (float)( tan( DEG2RAD( g_PrefsDlg->camera_fov ) * 0.5 ) * 0.75 );
    v4 = (float)( v4 * (double)camera.width / (double)camera.height );
    float v3 = -v4;

    // Left side plane normal = cross(vpn - v4*vright, vup)   (0x4054ca..0x4054ed)
    vec[0] = camera.vright[0] * v3 + camera.vpn[0];
    vec[1] = camera.vright[1] * v3 + camera.vpn[1];
    vec[2] = v3 * camera.vright[2] + camera.vpn[2];
    Vec3Cross( vec, camera.vup, clipWorldNormal[0] );

    // Right side plane normal = cross(vup, vpn + v4*vright)  (0x405518..0x40553b)
    vec[0] = camera.vright[0] * v4 + camera.vpn[0];
    vec[1] = camera.vright[1] * v4 + camera.vpn[1];
    vec[2] = v4 * camera.vright[2] + camera.vpn[2];
    Vec3Cross( camera.vup, vec, clipWorldNormal[1] );

    // Per-component box-corner selector: normal>0 → the max face (3+axis), else the min
    // face (axis).  (0x405551..0x405606 — x39..x44.)
    clipWorldAxis[0][0] = clipWorldNormal[0][0] > 0.0f ? 3 : 0;
    clipWorldAxis[1][0] = clipWorldNormal[1][0] > 0.0f ? 3 : 0;
    clipWorldAxis[0][1] = clipWorldNormal[0][1] > 0.0f ? 4 : 1;
    clipWorldAxis[1][1] = clipWorldNormal[1][1] > 0.0f ? 4 : 1;
    clipWorldAxis[0][2] = clipWorldNormal[0][2] > 0.0f ? 5 : 2;
    clipWorldAxis[1][2] = clipWorldNormal[1][2] > 0.0f ? 5 : 2;

    Cam_SetupClipPlanes( &world_orient_matrix[0][0] );   // 0x4055fa
}

// 0x405620  sub_405620 — transform the world-space clip setup (clipWorldNormal/Axis)
// into `orient`'s local space so CullCubic can test a brush drawn at that orientation:
//   clipBoxCenter    = camera origin in local space
//   clipPlaneNormal  = the two world normals rotated into local space
//   clipPlaneAxis    = re-derived box-corner selectors from the rotated normals' signs
void CCamWnd::Cam_SetupClipPlanes( const float *orient )
{
    const orientation_t *o = (const orientation_t *)orient;
    OrientationWorldPosToLocalPos( clipBoxCenter, camera.origin, o );          // 0x405636
    VectorRotateByAxis( clipPlaneNormal[0], orient, clipWorldNormal[0] );      // 0x40564d
    VectorRotateByAxis( clipPlaneNormal[1], orient, clipWorldNormal[1] );      // 0x405661
    for ( int p = 0; p < 2; ++p )                                             // 0x405670 loop (2 planes)
    {
        clipPlaneAxis[p][0] = clipPlaneNormal[p][0] > 0.0f ? 3 : 0;            // 0x405687
        clipPlaneAxis[p][1] = clipPlaneNormal[p][1] > 0.0f ? 4 : 1;           // 0x40569f
        clipPlaneAxis[p][2] = clipPlaneNormal[p][2] > 0.0f ? 5 : 2;           // 0x4056b1
    }
}

// 0x4056d0  CullCubic — returns 1 to CULL (skip) the brush, 0 to KEEP.  Two independent
// clips against the camera `cam`'s precomputed clip planes:
//   (1) cubic distance clip (only if g_PrefsDlg->m_bCubicClipping): reject brushes whose
//       AABB lies wholly outside a cube of half-size (m_nCubicScale<<6) centered at the
//       camera (in the target orientation's local space = clipBoxCenter).  When cull-sky
//       is off, SKY-material brushes are exempt (never distance-culled).
//   (2) frustum clip: reject brushes whose farthest AABB corner is behind either of the
//       two side planes (dot < -1.0).
// `brush` is the INSTANCE (selbrush_t); its ->def carries the world-space mins/maxs (0x20/
// 0x2c, contiguous, so def->mins[3+i] == def->maxs[i] — the axis-index trick).
char CullCubic( selbrush_t *brush, CCamWnd *cam )
{
    brush_t *def = brush->def;
    // m_bCubicClipping defaults ON with m_nCubicScale=13 (cube half-size 13<<6 = 832 units),
    // faithful to CPrefsDlg::LoadPrefs 0x44e7b9/0x44e7df.  Missing/popping distant geometry is
    // usually THIS, not a draw bug — toggle View→Cubic Clipping (Ctrl+\, cmd 32817).
    if ( g_PrefsDlg->m_bCubicClipping )
    {
        bool doDistance = false;
        if ( g_PrefsDlg->b_mCullSky )                       // 0x4056e9 — cull sky too
        {
            doDistance = true;                              // 0x4056f0 goto LABEL_5
        }
        else
        {
            // Sky test: fold the face material's surface-type bits (seed 4 = SURF_SKY);
            // if bit 2 survives (== 0 cleared? no — !=0 means sky) the brush is exempt.
            // (0x405706: brush_faces->mtldef[current_edit_layer]; MaterialDef_02 + _09.)
            MaterialDef *md = &def->faces[0].mtldef[g_qeglobals.current_edit_layer];
            dword_181F51C = 4;                              // 0x405709
            MaterialDef_02( md, MaterialDef_09 );           // 0x405713
            if ( dword_181F51C == 0 )                       // 0x405722 — NOT sky → distance-clip
                doDistance = true;
            else
                return 0;                                   // 0x4058cd — sky → keep (no distance cull)
        }
        if ( doDistance )
        {
            const float half = (float)( g_PrefsDlg->m_nCubicScale << 6 );  // 0x40572d
            const float *mins = def->mins;                  // def->mins[0..2], def->mins[3..5] == maxs
            // Reject if the AABB min corner is beyond the cube's max face on all axes,
            // OR the AABB max corner is beyond the cube's min face on all axes.  This is
            // the binary's two-pointer walk (0x40578e / 0x4057df) transcribed verbatim.
            float boxMin[3] = { cam->clipBoxCenter[0] - half,
                                cam->clipBoxCenter[1] - half,
                                cam->clipBoxCenter[2] - half };
            int i = 0;
            while ( boxMin[i] <= (double)mins[i] || boxMin[i] <= (double)mins[i + 3] )
            {
                if ( ++i >= 3 )
                {
                    float boxMax[3] = { cam->clipBoxCenter[0] + half,
                                        cam->clipBoxCenter[1] + half,
                                        cam->clipBoxCenter[2] + half };
                    int j = 0;
                    while ( boxMax[j] >= (double)mins[j] || boxMax[j] >= (double)mins[j + 3] )
                    {
                        if ( ++j >= 3 )
                            goto FRUSTUM;                    // 0x4057ee — passed cubic clip
                    }
                    return 1;                                // 0x4057df — cull
                }
            }
            return 1;                                        // 0x40578e — cull
        }
    }

FRUSTUM:   // LABEL_13 (0x4057f0) — frustum clip against the two side planes.
    {
        const float *mins = def->mins;                       // 0x4057f0
        // Plane 0: farthest corner (per-axis via clipPlaneAxis[0]) dotted with normal[0].
        float d0v[3];
        d0v[0] = mins[cam->clipPlaneAxis[0][0]] - cam->clipBoxCenter[0];      // 0x40580f
        d0v[1] = mins[cam->clipPlaneAxis[0][1]] - cam->clipBoxCenter[1];      // 0x40581c
        d0v[2] = mins[cam->clipPlaneAxis[0][2]] - cam->clipBoxCenter[2];      // 0x405829
        float d0 = cam->clipPlaneNormal[0][1] * d0v[1]
                 + cam->clipPlaneNormal[0][0] * d0v[0]
                 + cam->clipPlaneNormal[0][2] * d0v[2];                       // 0x40584b
        if ( d0 >= -1.0 )                                                     // 0x405860
        {
            float d1v[3];
            d1v[0] = mins[cam->clipPlaneAxis[1][0]] - cam->clipBoxCenter[0];  // 0x40587e
            d1v[1] = mins[cam->clipPlaneAxis[1][1]] - cam->clipBoxCenter[1];  // 0x40588b
            d1v[2] = mins[cam->clipPlaneAxis[1][2]] - cam->clipBoxCenter[2];  // 0x405898
            float d1 = cam->clipPlaneNormal[1][1] * d1v[1]
                     + cam->clipPlaneNormal[1][0] * d1v[0]
                     + cam->clipPlaneNormal[1][2] * d1v[2];                   // 0x4058ba
            if ( d1 >= -1.0 )                                                 // 0x4058c5
                return 0;                                                     // keep
        }
        return 1;                                                            // 0x4058c9 — cull
    }
}

// A face's mtldef carries a radMtl (qtexture_s) whose ->next is the engine Material* handle.
static Material *FaceMaterial( const MaterialDef *md )
{
    if ( md->radMtl )
        return md->radMtl->next;
    return nullptr;
}

// Per-face scratch (a brush face has at most MAX_POINTS_ON_WINDING verts; 64 is ample).
static const int CAM_MAXFACEVERTS = 64;

// camera.draw_mode -> MaterialTechniqueType, per Cam_Draw 0x407dc0.
// CASE_TEXTURE must stay OFF in g_useTechnique (r_material_load_obj.cpp): enabling slot 0x1B
// makes Material_LoadTechniqueSet REQUIRE case_texture for every l_sm_* world techset, which
// binds a code image kisak never sets up (R_LoadCaseTextures unported) — the whole techset load
// then fails and the world material falls back to the "2d" default.  Cam_TechAvailable demotes
// mode 4 to UNLIT as a result.
static MaterialTechniqueType Cam_TechForDrawMode( int mode )
{
    switch ( mode )
    {
    case 0:  return TECHNIQUE_WIREFRAME_SHADED;   // wireframe (filled, wireframe state)
    case 1:  return TECHNIQUE_UNLIT;              // fullbright: colormap * MATERIAL_COLOR
    case 2:  return TECHNIQUE_FAKELIGHT_NORMAL;   // textured + normal fake-light
    case 3:  return TECHNIQUE_FAKELIGHT_VIEW;     // textured + view fake-light
    case 4:  return TECHNIQUE_CASE_TEXTURE;       // case texture
    default: return TECHNIQUE_UNLIT;              // IDA initializes tech_type=4 before the switch
    }
}

// Editor flat colour for a material name: true (+ colour) for a tool/sky material, false (+
// white) for a plain world material.  The world draw splits on this — tools/sky get UNLIT +
// flat MATERIAL_COLOR, world materials go through FAKELIGHT.
static bool Cam_EditorMaterialColor( const char *name, float out[4] )
{
    out[0] = out[1] = out[2] = out[3] = 1.0f;     // default: white (textured world material)
    if ( !name ) return false;
    const char *n = name;
    const char *slash = strrchr( name, '/' );      // strip "wc/" / any path
    if ( slash ) n = slash + 1;
    static const struct { const char *key; float r, g, b; } tbl[] = {
        { "$opaque",   0.55f, 0.50f, 0.42f },       // caulk / no-texture fallback -> tan-grey
        { "sky",       0.45f, 0.62f, 0.92f },       // sky -> blue
        { "caulk",     0.55f, 0.50f, 0.42f },       // caulk -> tan-grey
        { "portal",    0.30f, 0.35f, 0.85f },
        { "hint",      0.88f, 0.85f, 0.20f },
        { "skip",      0.88f, 0.55f, 0.20f },
        { "nodraw",    0.42f, 0.42f, 0.42f },
        { "clip",      0.85f, 0.30f, 0.55f },
        { "trigger",   0.30f, 0.78f, 0.34f },
        { "origin",    0.78f, 0.30f, 0.78f },
        { "volume",    0.30f, 0.72f, 0.72f },        // lightgrid_volume / *_volume
        { "lightgrid", 0.30f, 0.72f, 0.72f },
    };
    for ( const auto &e : tbl )
        if ( strstr( n, e.key ) ) { out[0] = e.r; out[1] = e.g; out[2] = e.b; return true; }
    return false;
}

// Draw one face's winding as a textured triangle fan with technique `tech`. bgra
// modulates it (0xFFFFFFFF = white = the world path); push displaces each vertex along
// the face normal (selection overlay, to sit just in front of the coplanar world face).
static void Cam_DrawFaceTinted( const face_t *f, Material *mtl, uint32_t bgra, float push,
                                MaterialTechniqueType tech )
{
    const winding_t *w = f->w;
    int nv = w->numpoints;
    if ( nv < 3 || nv > CAM_MAXFACEVERTS )
        return;

    float    xyzw[CAM_MAXFACEVERTS][4];
    float    normal[CAM_MAXFACEVERTS][3];
    float    st[CAM_MAXFACEVERTS][2];
    float    color[CAM_MAXFACEVERTS];
    uint16_t indices[3 * CAM_MAXFACEVERTS];

    // Planar texcoord: project onto the two axes least aligned with the face normal,
    // at a fixed 1/128-unit scale (faithful texdef projection = follow-up).
    const float *n = f->plane.normal;
    float an[3] = { fabsf( n[0] ), fabsf( n[1] ), fabsf( n[2] ) };
    int ai, bi;                                  // the two projection axes
    if ( an[0] >= an[1] && an[0] >= an[2] ) { ai = 1; bi = 2; }      // normal ~X
    else if ( an[1] >= an[2] )              { ai = 0; bi = 2; }      // normal ~Y
    else                                    { ai = 0; bi = 1; }      // normal ~Z
    const float texScale = 1.0f / 128.0f;

    for ( int i = 0; i < nv; ++i )
    {
        const float *p = w->p[i];
        xyzw[i][0] = p[0] + n[0]*push; xyzw[i][1] = p[1] + n[1]*push;
        xyzw[i][2] = p[2] + n[2]*push; xyzw[i][3] = 1.0f;
        normal[i][0] = n[0]; normal[i][1] = n[1]; normal[i][2] = n[2];
        st[i][0] = p[ai] * texScale;
        st[i][1] = p[bi] * texScale;
        *(uint32_t *)&color[i] = bgra;            // modulated by the technique
    }

    // Fan triangulation: (0, i, i+1).
    int ic = 0;
    for ( int i = 1; i < nv - 1; ++i )
    {
        indices[ic++] = 0;
        indices[ic++] = (uint16_t)i;
        indices[ic++] = (uint16_t)( i + 1 );
    }

    R_AddRenderCmdDrawTris( mtl, tech, (short)ic, indices, (short)nv,
                            xyzw, normal, color, st );
}

static inline void Cam_DrawFace( const face_t *f, Material *mtl, MaterialTechniqueType tech )
{
    Cam_DrawFaceTinted( f, mtl, 0xFFFFFFFFu, 0.0f, tech );   // world path: white, no push
}

// ── editor surf-cache path ────────────────────────────────────────────────────
// The faithful Cam_Draw draws the world through the editor surface cache
// (Visuals_InitFaceVis 0x46F7A0 builds per-face GfxWorldVertex into the per-material D3D9 VB
// pool; DrawGeo emits them; R_AddEditorSurfsCmd flushes one RC_DRAW_EDITOR_SKINNEDCACHED).
// KISAK: the DEFAULT world draw is the immediate path; RADIANT_SURFCACHE selects the cached
// one.  Both build geometry with brush.cpp Face_BuildLayerGeom, so they are bit-identical.
extern unsigned int Editor_VB_Upload( Material *material, int vertCount,
    const float *xyz, const float *tangent, const float *binormal,
    const float *normal, const float *texCoord, const float *color );
extern void  Editor_AddGeoFace( Material *handle, int techType, int sortKey,
                                int vertCount, int vbIndexAndOffs );
extern void *R_AddEditorSurfsCmd();

// RADIANT_SURFCACHE — operator switch: faithful surf-cache world draw instead of immediate.
static bool Cam_SurfCacheEnabled()
{
    static const bool s = ( getenv( "RADIANT_SURFCACHE" ) != nullptr );
    return s;
}
// Render decorations (entity origin boxes, trigger-radius cylinders, script-colour tokens):
// display-only overlays, default-OFF behind RADIANT_DECOR.  NON-static — brush.cpp's DrawModels
// decoration gate shares it so the whole decoration layer flips together.
bool Radiant_DecorEnabled()
{
    static const bool s = ( getenv( "RADIANT_DECOR" ) != nullptr );
    return s;
}
// The binary gates ActiveSunLightPreviewInit on enable_light_preview && preview_sun_aswell
// (defaults 1 and 0), so the sun preview is off until the user enables it in Preferences.
static bool Cam_SunPrevEnabled()
{
    return g_PrefsDlg->enable_light_preview && g_PrefsDlg->preview_sun_aswell;
}

// Brush_MakeFaceVisuals (0x477C50) uploads each face's per-layer GfxWorldVertex run to the
// editor surf-cache VB pool on every faceVis rebuild.
// KISAK: the headless gates run with NO D3D device, where that upload would FatalError — gate
// the GPU half on the device, mirroring the R_Ed_SetSceneParms device gate.  brush.cpp externs.
bool Radiant_FaceVisGpuReady()
{
    return dx.device != nullptr;
}

// 0x406760  R_SunPrev_SetSunConstants — parse the worldspawn sun keys and push the sun
// direction/colour into CONST_SRC_CODE_SUN_POSITION/DIFFUSE/SPECULAR.  The binary round-trips
// the worldspawn through a CMemFile (Entity_WriteSelected); we build the same epair text from
// world_entity->epairs and feed the SAME parser pair, so the GfxLight is bit-identical.
// Returns 1 (+ the directional sun dir, w=0) when "sundirection" is present, else 0.
// ambientMulOut: the black-world multiply colour (sub_50C470) the full-screen quad resets the
// textured world to before the SUNLIGHT_PREVIEW pass adds the sun light.
// `emit` = issue the 3 RC_SET_CUSTOM_CONSTANT commands (false = pre-compute values only).
static int Cam_SunPrev_SetSunConstants( float sunDirOut[3], float ambientMulOut[3] = nullptr,
                                        float sunColorOut[3] = nullptr, bool emit = true )
{
    if ( !world_entity )
        return 0;
    entity_s_def *wd = (entity_s_def *)world_entity->def;
    if ( !wd )
        return 0;
    // Gate on the sun key, like the binary's HasKeyValuePair(def,"sundirection").
    bool hasSun = false;
    for ( epair_t *e = wd->epairs; e; e = e->next )
        if ( e->key && !_stricmp( e->key, "sundirection" ) ) { hasSun = true; break; }
    if ( !hasSun )
        return 0;

    // Worldspawn epair text as R_ParseSunLight expects it: "{\n" + "\"k\" \"v\"\n"* + "}\n".
    // 8192 matches the binary's 0x2000 read clamp.  _snprintf returns -1 on MSVC when truncated,
    // so Append clamps n rather than letting it go negative.
    char text[8192];
    int n = 0;
    const int CAP = (int)sizeof(text) - 4;
    auto Append = [&]( const char *fmt, const char *a, const char *b ) {
        if ( n >= CAP ) return;
        int w = a ? _snprintf( text + n, CAP - n, fmt, a, b ) : _snprintf( text + n, CAP - n, "%s", fmt );
        if ( w < 0 || w > CAP - n ) n = CAP; else n += w;
    };
    Append( "{\n", nullptr, nullptr );
    for ( epair_t *e = wd->epairs; e && n < CAP - 8; e = e->next )
        if ( e->key && e->value )
            Append( "\"%s\" \"%s\"\n", e->key, e->value );
    Append( "}\n", nullptr, nullptr );
    text[sizeof(text) - 1] = 0;

    SunLightParseParams params;
    memset( &params, 0, sizeof(params) );
    GfxLight light;
    memset( &light, 0, sizeof(light) );
    char *p = text;
    R_ParseSunLight( &params, p );
    R_InterpretSunLightParseParamsIntoLights( &params, &light );

    // The 3 sun code constants sunpre_*.hlsl reads (binary order/values, 0x406896+).
    if ( emit )
    {
        R_AddCmdSetCustomShaderConstant( CONST_SRC_CODE_SUN_POSITION, light.dir[0],   light.dir[1],   light.dir[2],   0.0f );
        R_AddCmdSetCustomShaderConstant( CONST_SRC_CODE_SUN_DIFFUSE,  light.color[0], light.color[1], light.color[2], 1.0f );
        R_AddCmdSetCustomShaderConstant( CONST_SRC_CODE_SUN_SPECULAR, light.color[0], light.color[1], light.color[2], 1.0f );
    }

    sunDirOut[0] = light.dir[0];
    sunDirOut[1] = light.dir[1];
    sunDirOut[2] = light.dir[2];
    if ( sunColorOut )
    {
        sunColorOut[0] = light.color[0];
        sunColorOut[1] = light.color[1];
        sunColorOut[2] = light.color[2];
    }

    // 0x50C470  R_SunPrev_ComputeBlackWorldMultiplyColor — the colour the full-screen multiply
    // quad drops the textured world to before the SUNLIGHT_PREVIEW pass adds the sun light.
    // All x87, so hex-rays shows only fragments; both branch tests are the `fnstsw ax / test
    // ah,44h / jp` EQUALITY idiom (== 0), NOT a > test:
    //   0x50c473  if ( ambientScale == 0 ) amb = {0,0,0}
    //   0x50c48e  ColorNormalize(ambientColor, ambientColor)  [in place, RETURNS the max]
    //   0x50c493  if ( returned max == 0 ) amb = {0,0,0}   <- the RETURN value, not the output
    //             (ColorNormalize writes {1,1,1} to `out` when max==0, so testing `out` here
    //              wrongly picks the scale branch for a worldspawn with no "_color")
    //   0x50c4ce  k = (sunLight - ambientScale) * diffuseFraction
    //   0x50c4f4  out[i] = diffuseColor[i]*k + amb[i]
    if ( ambientMulOut )
    {
        float amb[3] = { 0.0f, 0.0f, 0.0f };
        if ( params.ambientScale != 0.0f )
        {
            const float maxComp = ColorNormalize( params.ambientColor, params.ambientColor );
            if ( maxComp != 0.0f )
            {
                amb[0] = params.ambientScale * params.ambientColor[0];
                amb[1] = params.ambientScale * params.ambientColor[1];
                amb[2] = params.ambientScale * params.ambientColor[2];
            }
        }
        const float sunFloor = ( params.sunLight - params.ambientScale ) * params.diffuseFraction;
        ambientMulOut[0] = params.diffuseColor[0] * sunFloor + amb[0];
        ambientMulOut[1] = params.diffuseColor[1] * sunFloor + amb[1];
        ambientMulOut[2] = params.diffuseColor[2] * sunFloor + amb[2];
    }
    return 1;
}

// Per-face editor flat colour via MATERIAL_COLOR, emitted only on CHANGE.  The dedup is
// load-bearing: one command per face overflows the editor command buffer on a big map.
static bool Cam_MaterialColorChanged( const float a[4], const float b[4] );
static void Cam_SetLastMaterialColor( float dst[4], const float src[4] );

static void Cam_EmitEditorTint( const face_t *f, int layer, float lastMC[4] )
{
    const char *mn = f->mtldef[layer].radMtl ? f->mtldef[layer].radMtl->name : nullptr;
    float ecol[4];
    Cam_EditorMaterialColor( mn, ecol );
    if ( Cam_MaterialColorChanged( ecol, lastMC ) )
    {
        R_AddCmdSetMaterialColor( ecol );
        Cam_SetLastMaterialColor( lastMC, ecol );
    }
}

// Surf-cache emit: per material layer build the real per-vertex geometry, upload it to the
// editor VB pool, queue the cached draw.  The pool is per-frame scratch (re-uploaded every
// Cam_Draw, flushed by R_AddEditorSurfsCmd), so no faceVis_s is persisted here.
static void Cam_DrawFaceCached( face_t *f )
{
    int layerCount = MaterialDef_11( &f->mtldef[g_qeglobals.current_edit_layer] );
    for ( int L = 0; L < layerCount; ++L )
    {
        EdLayerGeom g;
        if ( !Face_BuildLayerGeom( f, (const orientation_t *)world_orient_matrix, L, &g )
             || !g.material || g.vertcount < 3 )
            continue;
        unsigned int handle = Editor_VB_Upload( g.material, g.vertcount,
            (const float *)g.xyz, (const float *)g.tangent, (const float *)g.binormal,
            (const float *)g.normal, (const float *)g.st, (const float *)g.color );
        Editor_AddGeoFace( g.material, TECHNIQUE_UNLIT, 0, g.vertcount, (int)handle );
    }
}

// Draw `mtl` with the requested technique if its loaded techset carries it, else UNLIT (always
// present).  Keeps the draw safe — a material that fell to the "2d" default techset would
// otherwise hit a NULL technique.
static MaterialTechniqueType Cam_TechAvailable( Material *mtl, MaterialTechniqueType want )
{
    if ( want != TECHNIQUE_UNLIT && ( !mtl->techniqueSet || !mtl->techniqueSet->techniques[want] ) )
        return TECHNIQUE_UNLIT;
    return want;
}

static bool Cam_MaterialColorChanged( const float a[4], const float b[4] )
{
    return a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3];
}

static void Cam_SetLastMaterialColor( float dst[4], const float src[4] )
{
    dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
}

static void Cam_DrawFaceFaithfulImmediate( face_t *f, MaterialTechniqueType tech )
{
    int layerCount = MaterialDef_11( &f->mtldef[g_qeglobals.current_edit_layer] );
    for ( int L = 0; L < layerCount; ++L )
    {
        EdLayerGeom g;
        if ( !Face_BuildLayerGeom( f, (const orientation_t *)world_orient_matrix, L, &g )
             || !g.material || g.vertcount < 3 )
            continue;
        MaterialTechniqueType useTech = Cam_TechAvailable( g.material, tech );

        float    xyzw[CAM_MAXFACEVERTS][4];
        uint16_t indices[3 * CAM_MAXFACEVERTS];
        for ( int i = 0; i < g.vertcount; ++i )
        {
            xyzw[i][0] = g.xyz[i][0]; xyzw[i][1] = g.xyz[i][1];
            xyzw[i][2] = g.xyz[i][2]; xyzw[i][3] = 1.0f;
        }
        int ic = 0;
        for ( int i = 1; i < g.vertcount - 1; ++i )
        {
            indices[ic++] = 0; indices[ic++] = (uint16_t)i; indices[ic++] = (uint16_t)( i + 1 );
        }

        R_AddRenderCmdDrawTris( g.material, useTech, (short)ic, indices,
                                (short)g.vertcount, xyzw, g.normal, (float *)g.color, g.st );
    }
}

// KISAK sun-preview APPROXIMATION (used when the faithful R_SunPrev_Main path is unavailable):
// MATERIAL_COLOR = the face's directional-sun colour, then draw the textured face UNLIT (which
// multiplies its colormap by MATERIAL_COLOR).  No shadows, no per-pixel lighting.
// RC_SET_MATERIAL_COLOR is a CRITICAL command sharing a 0x2000-byte budget (~410 commands), so
// one per face overruns it on a big map.  Two guards: dedup on the QUANTIZED shade (5-bit
// channels), and a hard per-frame cap — past the cap later faces just reuse the last shade.
static int s_sunMCEmitted = 0;                         // MATERIAL_COLORs emitted this frame
static const int SUN_MC_BUDGET = 300;                  // hard cap (< 410 critical-command ceiling)
static void Cam_DrawFaceSun( face_t *f, Material *mtl, MaterialTechniqueType tech, float lastShade[4] )
{
    float shade[4];
    SunPrev_FaceShade( f->plane.normal, shade );       // world brushes: local normal = world
    // Quantize to 5 bits/channel (1/31 steps) so near-equal facings dedup to one command.
    for ( int c = 0; c < 3; ++c )
    {
        float q = shade[c]; if ( q < 0.0f ) q = 0.0f; if ( q > 4.0f ) q = 4.0f;
        shade[c] = (float)( (int)( q * 31.0f / 4.0f + 0.5f ) ) * 4.0f / 31.0f;
    }
    shade[3] = 1.0f;
    if ( ( shade[0] != lastShade[0] || shade[1] != lastShade[1] ||
           shade[2] != lastShade[2] ) && s_sunMCEmitted < SUN_MC_BUDGET )
    {
        R_AddCmdSetMaterialColor( shade );
        lastShade[0] = shade[0]; lastShade[1] = shade[1];
        lastShade[2] = shade[2]; lastShade[3] = shade[3];
        ++s_sunMCEmitted;
    }
    Cam_DrawFace( f, mtl, tech );                       // textured face, tinted by MATERIAL_COLOR
}

// ── g_SelectedFaces (select.cpp) — the Ctrl+Shift+LMB face-selection set ──────────
extern float world_orient_matrix[4][3];              // 0x6DE290 (orientation_t, world space)

// brush.cpp line-outline batcher (reused) + the deferred-line flush command.
extern int DrawShadedWireframe( int cullMode, face_t *face, const orientation_t *orient,
                                GfxColor *lineColor, char width, int vertCount,
                                int vertLimit, GfxPointVertex *verts );

// KISAK: the selected-FACE highlight.  The binary tints picked faces through DrawGeo's editor
// surf-cache; the port outlines each picked winding in magenta via DrawShadedWireframe ->
// R_AddCmd_Line3D, because the UNLIT triangle shader carries no colour for immediate tris.
static void Cam_DrawSelectedFaces()
{
    int count = g_SelectedFaces.GetSize();
    if ( count <= 0 || !g_SelectedFaces.m_pData )
        return;

    static GfxPointVertex s_hlVerts[4096];
    unsigned int magenta = 0xFFFF00FFu;              // packed line colour (the $line material
                                                     // colours from CODE_MATERIAL_COLOR, set below)
    int vc = 0;
    for ( int i = 0; i < count; ++i )
    {
        selbrush_t *b = g_SelectedFaces.GetAt( i ).brush;
        if ( !b || !b->def )
            continue;
        int idx = g_SelectedFaces.GetAt( i ).index;
        if ( (unsigned)idx >= (unsigned)b->def->faceCount )
            continue;
        face_t *f = &b->def->faces[idx];
        if ( !f->w )
            continue;
        vc = DrawShadedWireframe( -1, f, (const orientation_t *)world_orient_matrix,
                                  (GfxColor *)&magenta, 3, vc, 4096, s_hlVerts );
    }
    if ( vc )
    {
        // The $line material colours from CONST_SRC_CODE_MATERIAL_COLOR, not the per-vertex
        // colour — drive the tint through R_AddCmdSetMaterialColor, then reset to white.
        static const float s_magenta[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
        static const float s_white[4]   = { 1.0f, 1.0f, 1.0f, 1.0f };
        R_AddCmdSetMaterialColor( s_magenta );
        R_AddCmd_Line3D( (short)( vc / 2 ), 3, s_hlVerts );
        R_AddCmdSetMaterialColor( s_white );
    }
}

// ── xmodel MESH draw ──────────────────────────────────────────────────────────
// Cam_Draw reaches misc_model meshes through DrawBrush -> DrawModels -> SetupModelInst
// (Entity_UpdateModelInst registers the xmodel in the editor model-inst buffer) + SkinModelInst
// (builds GfxModelSkinnedSurfaces, queued as ED_SURF_MODEL surfs); R_AddEditorSurfsCmd flushes.
extern bool  Model_SetModel( entity_brush_s *b, int orientMatrix );        // brush.cpp 0x478780
extern void  SkinModelInst( int instanceHandle, Material *checkhandle, int techType,
                            const int *colorPtr, int drawFlags );          // r_ed_scene.cpp 0x4FE2E0
extern void Radiant_FL_Log( const char *fmt, ... );                       // mainfrm.cpp

// Editor model-load asset-drop recovery guard (engine_stubs.cpp).
extern int     g_radiantAssetLoadGuard;
extern jmp_buf g_radiantAssetLoadJmp;

// Operator escape hatch: RADIANT_MODELS=0 disables the xmodel mesh draw (default ON).
// Non-static — the 2D views share this one gate.
bool Editor_ModelsEnabled()
{
    static const bool s = []{
        const char *v = getenv( "RADIANT_MODELS" );
        return !( v && v[0] == '0' && v[1] == '\0' );
    }();
    return s;
}

// Per-entity model instance + skin under SEH, so a model whose load derefs a NULL handle is
// skipped rather than crashing.  Returns 1 iff the entity has a renderable model/prefab
// (Entity_HasRenderableModel 0x479610 = the binary's DrawBrush gate; a bare-prefab entity
// returns 1 with no modelInst and the caller draws the prefab CONTENTS).
// Its own function because MSVC forbids setjmp and __try in one function (C2713).
extern float world_orient_matrix[4][3];                                   // entity.cpp
extern char  Entity_HasRenderableModel( brush_t_with_custom_def *b, int orient );  // brush.cpp 0x479610
static int Cam_SkinModelSEH( selbrush_t *b, const orientation_t *orient, int meshTech,
                             GfxColor *col, int drawFlags )
{
    int renderable = 0;
    __try
    {
        if ( Entity_HasRenderableModel( (brush_t_with_custom_def *)b, (int)(intptr_t)orient ) )
        {
            renderable = 1;
            if ( b->owner->modelInst )
            {
                // DrawModels 0x479735: SkinModelInst(inst, checkhandle, draw_meth2,
                // draw_meth2 != 29 ? 0 : color, drawFlags) — the per-vert colour override
                // only rides the WIREFRAME technique.
                SkinModelInst( b->owner->modelInst, nullptr, meshTech,
                               meshTech != 29 ? nullptr : (const int *)col, drawFlags );
            }
        }
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        renderable = 0;   // a model that AV'd while loading
    }
    return renderable;
}

// KISAK: install the ERR_DROP recovery frame, then instance + skin one entity's model.  A
// CoD4-format asset the CoD3-sized parser chokes on raises Com_Error(ERR_DROP) deep in the
// load; engine_stubs.cpp longjmps back here and Model_SetModel's modelFailed flag stops that
// model retrying.  The guarded entry DrawBrush (0x47b102) calls per fixedsize entity.
int Editor_InstanceAndSkinModel( selbrush_t *b, const orientation_t *orient, int meshTech,
                                 GfxColor *col, int drawFlags )
{
    // PARSE-STATE PRECONDITION: XModel_LoadPhysicsCollMap parses phys_collmaps/<name>.map at
    // parseInfoNum==0 (no map session during a draw).  Collmap geom holds negative floats; with
    // parseInfo[0] left at spaceDelimited==0 && negativeNumbers==0 by an earlier editor parse,
    // Com_ParseExt splits "-0.000000" into "-" + "0.000000" and every later read shifts by one.
    // Enable negativeNumbers for the load (identical result in either tokenizer mode).
    ParseThreadInfo *parse = Com_GetParseThreadInfo();
    parseInfo_t     *pi    = &parse->parseInfo[parse->parseInfoNum];
    int              savedNeg = pi->negativeNumbers;
    pi->negativeNumbers = 1;

    if ( setjmp( g_radiantAssetLoadJmp ) != 0 )
    {
        g_radiantAssetLoadGuard = 0;   // unwound from an ERR_DROP — frame reset, skip
        return 0;
    }
    ++g_radiantAssetLoadGuard;
    int skinned = Cam_SkinModelSEH( b, orient, meshTech, col, drawFlags );
    --g_radiantAssetLoadGuard;
    pi->negativeNumbers = savedNeg;
    return skinned;
}

// SunLightPreview_DrawBrushShadow's misc_model arm (0x47b2e1) drives a real model load on
// entities the base pass may never have touched, so it needs the SAME bracket
// Editor_InstanceAndSkinModel installs.  KISAK: the bracket only — the arm is binary code.
extern char Radiant_ShadVol_ModelShadowArm( selbrush_t *sb, orientation_t *orient,
                                            const float *light );          // shadowvolume.cpp
static int Cam_ShadowModelSEH( selbrush_t *b, orientation_t *orient, const float *light )
{
    int built = 0;
    __try   { built = Radiant_ShadVol_ModelShadowArm( b, orient, light ) ? 1 : 0; }
    __except( EXCEPTION_EXECUTE_HANDLER ) { built = 0; }   // a model that AV'd while loading
    return built;
}

int Editor_ModelShadowGuarded( selbrush_t *b, orientation_t *orient, const float *light )
{
    ParseThreadInfo *parse = Com_GetParseThreadInfo();
    parseInfo_t     *pi    = &parse->parseInfo[parse->parseInfoNum];
    int              savedNeg = pi->negativeNumbers;
    pi->negativeNumbers = 1;

    if ( setjmp( g_radiantAssetLoadJmp ) != 0 )
    {
        g_radiantAssetLoadGuard = 0;   // unwound from an ERR_DROP — frame reset, skip
        return 0;
    }
    ++g_radiantAssetLoadGuard;
    int built = Cam_ShadowModelSEH( b, orient, light );
    --g_radiantAssetLoadGuard;
    pi->negativeNumbers = savedNeg;
    return built;
}

// ── LIGHT-PREVIEW GLOW SPHERE ─────────────────────────────────────────────────
// 0x4058e0  LightPreview_DrawLight2 — DrawLightsMain's (0x407180) fallback when the per-pixel
// GPU light path (LightPreview_DrawLight 0x406fb0, unported) is unavailable: a 32x16 UV-sphere
// of `radius` at the light origin, packed with the light colour, drawn additively through
// R_AddRenderCmdDrawTris($additive, TECHNIQUE_UNLIT).
// Colour per DrawLightsMain: _color (default white), saturated to the max component because
// g_qeglobals.preview_at_max_intensity defaults 0 (the binary's 1e7 intensity path).
// Topology is the binary's: 32 longitude x 16 latitude -> 15 rings of 32 (480) + 2 poles = 482,
// quad-pairs per ring + two tri pole caps.  Rebuilt from intent, not transcribed (the IDB's
// interleaved stack-array juggling is trap-dense); vertex count and index pattern are exact.
extern selbrush_t selected_brushes;                                // map.cpp (0x23F1864)
extern void  OrientationPosToWorldPos( float *out, const float *localPos,
                                       const orientation_t *orient );   // brush.cpp (0x4BA430)
extern float Entity_GetFloatValueForKey( int e, const char *key );      // entity.cpp (0x4837C0)

static void Cam_BuildLightGlowSphere( const float center[3], float radius,
                                      const float rgb[3] )
{
    const int sideCount = 32;             // longitudinal segments (binary: 32)
    const int halfSide  = sideCount / 2;  // 16 latitude bands
    const int ringCount = halfSide - 1;   // 15 full rings of `sideCount` verts
    const int ringVerts = ringCount * sideCount;        // 480
    const int vertCount = ringVerts + 2;                // + 2 poles = 482

    // index count: ring quad band (ringCount-1 bands) ×6 + 2 pole caps ×(sideCount×3)
    // (matches the binary's two index loops: 14 bands of 32 quads + top/bottom 32 tris)
    static float    xyzw  [482][4];
    static float    normal[482][3];
    static float    color [482];
    static float    st    [482][2];       // R_AddRenderCmdDrawTris memcpy's st unconditionally
    static uint16_t indices[ (32 - 1) * 32 * 6 + 2 * 32 * 6 ];   // generous upper bound

    GfxColor packed;
    Byte4PackPixelColor( const_cast<float *>( rgb ), &packed );
    const float fcol = *(const float *)&packed;          // BGRA bit-pattern as float

    // ── ring vertices: row r ∈ [1..ringCount], col c ∈ [0..sideCount) ──
    int vc = 0;
    for ( int r = 1; r <= ringCount; ++r )
    {
        // latitude angle θ = r * (2π / sideCount)  (binary: row*0.19634954 = row*2π/32)
        const float theta = (float)r * ( 6.2831853f / (float)sideCount );
        const float sinT = sinf( theta ), cosT = cosf( theta );
        for ( int c = 0; c < sideCount; ++c )
        {
            const float phi = (float)c * ( 6.2831853f / (float)sideCount );
            const float nx = cosf( phi ) * sinT;
            const float ny = sinf( phi ) * sinT;
            const float nz = cosT;
            normal[vc][0] = nx; normal[vc][1] = ny; normal[vc][2] = nz;
            xyzw[vc][0] = nx * radius + center[0];
            xyzw[vc][1] = ny * radius + center[1];
            xyzw[vc][2] = nz * radius + center[2];
            xyzw[vc][3] = 1.0f;
            color[vc] = fcol;
            st[vc][0] = (float)c / (float)sideCount;     // polar UV (binary maps the same)
            st[vc][1] = (float)r / (float)halfSide;
            ++vc;
        }
    }
    // top pole (+Z) and bottom pole (−Z)
    const int topPole = vc, botPole = vc + 1;
    normal[topPole][0] = 0.0f; normal[topPole][1] = 0.0f; normal[topPole][2] = 1.0f;
    xyzw[topPole][0] = center[0]; xyzw[topPole][1] = center[1];
    xyzw[topPole][2] = radius + center[2]; xyzw[topPole][3] = 1.0f;
    color[topPole] = fcol; st[topPole][0] = 0.0f; st[topPole][1] = 0.0f;
    normal[botPole][0] = 0.0f; normal[botPole][1] = 0.0f; normal[botPole][2] = -1.0f;
    xyzw[botPole][0] = center[0]; xyzw[botPole][1] = center[1];
    xyzw[botPole][2] = -radius + center[2]; xyzw[botPole][3] = 1.0f;
    color[botPole] = fcol; st[botPole][0] = 0.0f; st[botPole][1] = 1.0f;

    // ── indices: 14 quad bands between adjacent rings ──
    int ic = 0;
    for ( int r = 0; r < ringCount - 1; ++r )
    {
        const int base = r * sideCount, next = ( r + 1 ) * sideCount;
        for ( int c = 0; c < sideCount; ++c )
        {
            const int c1 = ( c + 1 ) % sideCount;
            indices[ic++] = (uint16_t)( base + c  );
            indices[ic++] = (uint16_t)( base + c1 );
            indices[ic++] = (uint16_t)( next + c  );
            indices[ic++] = (uint16_t)( next + c  );
            indices[ic++] = (uint16_t)( base + c1 );
            indices[ic++] = (uint16_t)( next + c1 );
        }
    }
    // top cap: ring 0 → top pole
    for ( int c = 0; c < sideCount; ++c )
    {
        const int c1 = ( c + 1 ) % sideCount;
        indices[ic++] = (uint16_t)topPole;
        indices[ic++] = (uint16_t)( 0 + c1 );
        indices[ic++] = (uint16_t)( 0 + c  );
    }
    // bottom cap: last ring → bottom pole
    const int last = ( ringCount - 1 ) * sideCount;
    for ( int c = 0; c < sideCount; ++c )
    {
        const int c1 = ( c + 1 ) % sideCount;
        indices[ic++] = (uint16_t)botPole;
        indices[ic++] = (uint16_t)( last + c  );
        indices[ic++] = (uint16_t)( last + c1 );
    }

    // d_additive may be null if the matsys default-material init hasn't run — guard so a
    // light entity never crashes the camera before assets are registered.
    Material *addMat = g_qeglobals.d_additive;
    if ( !addMat )
        return;
    R_AddRenderCmdDrawTris( addMat, TECHNIQUE_UNLIT, (short)ic, indices,
                            (short)vertCount, xyzw, normal, color, st );
}

// One light instance's glow sphere.  All gates (owner/eclass/CLASS_LIGHT/hidden/radius) live
// here so either preview source below can pass any selbrush_t.
static void Cam_DrawOneLightPreview( selbrush_t *b )
{
    {
        {
            entity_s *owner = b ? b->owner : nullptr;
            if ( !owner )
                return;
            entity_s_def *eDef = (entity_s_def *)owner->def;
            if ( !eDef || !eDef->eclass )
                return;
            if ( ( eDef->eclass->classtype & 1 ) == 0 )      // CLASS_LIGHT only
                return;
            // FilterBrush gate (the binary skips filtered lights); brushFlags&2 = hidden.
            // (The binary's list loop also sign-tests a per-record byte, 0x406727 — the
            // pending-remove flag; RemoveLightPreview already drops freed instances here.)
            if ( ( b->brushFlags & 2 ) != 0 )
                return;
            brush_t *def = b->def;
            if ( !def )
                return;
            // radius key — binary returns early if radius <= 0 (LightPreview draws nothing).
            float radius = Entity_GetFloatValueForKey( (int)(intptr_t)eDef, "radius" );
            if ( radius <= 0.0f )
                return;

            // world position = DEF-bbox centre, transformed through the world orientation
            // (identity for the editor world) — matches DrawLightsMain's OrientationPosToWorldPos.
            float localCenter[3] = { ( def->mins[0] + def->maxs[0] ) * 0.5f,
                                     ( def->mins[1] + def->maxs[1] ) * 0.5f,
                                     ( def->mins[2] + def->maxs[2] ) * 0.5f };
            float worldCenter[4];
            OrientationPosToWorldPos( worldCenter,
                                      localCenter,
                                      (const orientation_t *)world_orient_matrix );

            // colour = _color (default white), saturated to the max component (binary's
            // preview_at_max_intensity==0 → intensity 1e7 path → hue at full brightness).
            float rgb[3] = { 1.0f, 1.0f, 1.0f };
            for ( epair_t *ep = eDef->epairs; ep; ep = ep->next )
                if ( !_stricmp( ep->key, "_color" ) )
                {
                    float r, g, bl;
                    if ( sscanf( ep->value, "%f %f %f", &r, &g, &bl ) == 3 )
                    { rgb[0] = r; rgb[1] = g; rgb[2] = bl; }
                    break;
                }
            float maxC = rgb[0];
            if ( rgb[1] > maxC ) maxC = rgb[1];
            if ( rgb[2] > maxC ) maxC = rgb[2];
            if ( maxC > 0.0f ) { rgb[0] /= maxC; rgb[1] /= maxC; rgb[2] /= maxC; }

            Cam_BuildLightGlowSphere( worldCenter, radius, rgb );
        }
    }
}

// Two preview sources, no dedupe (the binary double-draws a pinned+selected light too):
//   (a) ActiveSunLightPreviewInit 0x406719 — the camera's PINNED light_preview_arr (8-slot
//       FIFO, filled by Light Preview→"Start preview on selected", cmd 33951);
//   (b) Cam_Draw 0x408266 — every SELECTED light.
// With nothing pinned and nothing selected the toggle shows nothing; that is retail behaviour.
static void Cam_DrawLightPreviews( CCamWnd *cam )
{
    for ( int i = 0; i < cam->light_preview_count; ++i )                      // (a) pinned list
        Cam_DrawOneLightPreview( (selbrush_t *)(intptr_t)cam->light_preview_arr[i].inst );
    for ( selbrush_t *b = selected_brushes.next; b && b != &selected_brushes; b = b->next )
        Cam_DrawOneLightPreview( b );                                         // (b) selected lights
}

// Camera decorations drawn after the world pass: DrawTriggerRadius (0x407410) and
// CamWnd_Tokens (0x4076C0), both on the immediate RC_DRAW_TRIANGLES (d_white, UNLIT) path.
extern bool  HasKeyValuePair( entity_s_def *e, const char *key );                    // entity.cpp 0x4838B0
extern char *ValueForKey2( int e, const char *key );                                 // entity.cpp 0x4825C0
extern int   ScriptGroup_Unreachable( const char *a1 );                              // scriptgroup.cpp 0x451170
extern char  FilterBrush( selbrush_t *b, int updateFilters );                        // filters.cpp 0x46A1F0
extern void  Ed_DrawScriptColorQuad( int entDef, const float *color );               // brush.cpp 0x46AE10

// flt_73B098 — the 7 token colours (r/b/y/c/g/p/o), shared with brush.cpp.
static const float kCamTokenColors[7][4] = {
    { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.4f, 0.0f, 1.0f },
};

// 0x405D00  Ed_DrawTriggerCylinder - a 32-sided trigger volume: top end-cap disc (verts
// 0..31), bottom end-cap disc (32..63), side wall (64..95 top / 96..127 bottom); 128 verts,
// 372 indices, UNLIT + per-vertex colour.  center[3] is the trigger base; topZ/botZ are ring
// z-offsets above center.z.  Rebuilt from intent (the binary's 4-array stack juggling is
// trap-dense); vertex roles, angle step 2pi/32 and both index loops are exact.
static void Ed_DrawTriggerCylinder( const GfxColor *col, const float center[3],
                                    float radius, float topZ, float botZ )
{
    const int sideCount = 32;
    static float    xyzw  [128][4];
    static float    normal[128][3];
    static float    color [128];
    static uint16_t indices[372];

    const float fcol = *(const float *)&col->packed;
    const float zTop = center[2] + topZ;
    const float zBot = center[2] + botZ;
    const float kStep = 6.2831853f / (float)sideCount;     // dbl_6F4500 = 2π/32

    for ( int i = 0; i < sideCount; ++i )
    {
        const float ang = (float)i * kStep;
        const float c = cosf( ang ), s = sinf( ang );
        const float rx = c * radius + center[0];
        const float ry = s * radius + center[1];
        // group0 (0..31): top cap ring; normal (0,0,-1) (binary flt_6F40C4=-1).
        xyzw[i][0]=rx; xyzw[i][1]=ry; xyzw[i][2]=zTop; xyzw[i][3]=1.0f;
        normal[i][0]=0; normal[i][1]=0; normal[i][2]=-1.0f; color[i]=fcol;
        // group1 (32..63): bottom cap ring.
        xyzw[i+32][0]=rx; xyzw[i+32][1]=ry; xyzw[i+32][2]=zBot; xyzw[i+32][3]=1.0f;
        normal[i+32][0]=0; normal[i+32][1]=0; normal[i+32][2]=1.0f; color[i+32]=fcol;
        // group2 (64..95): wall-top ring (radial normal).
        xyzw[i+64][0]=rx; xyzw[i+64][1]=ry; xyzw[i+64][2]=zTop; xyzw[i+64][3]=1.0f;
        normal[i+64][0]=c; normal[i+64][1]=s; normal[i+64][2]=0; color[i+64]=fcol;
        // group3 (96..127): wall-bottom ring.
        xyzw[i+96][0]=rx; xyzw[i+96][1]=ry; xyzw[i+96][2]=zBot; xyzw[i+96][3]=1.0f;
        normal[i+96][0]=c; normal[i+96][1]=s; normal[i+96][2]=0; color[i+96]=fcol;
    }

    // index loop 1 (i=2..31): top-cap fan (0,i-1,i) + bottom-cap fan (32,i+32,i+31).
    int ic = 0;
    for ( int i = 2; i < sideCount; ++i )
    {
        indices[ic+0]=0;            indices[ic+1]=(uint16_t)(i-1);    indices[ic+2]=(uint16_t)i;
        indices[ic+3]=32;           indices[ic+4]=(uint16_t)(i+32);   indices[ic+5]=(uint16_t)(i+31);
        ic += 6;
    }
    // index loop 2 (i=0..31): cylinder wall (v+64, v+96, n+96) + (n+96, n+64, v+64),
    // where n=(i+1)%32 (binary masks with 0x8000001F → modulo-32 wraparound).
    for ( int i = 0; i < sideCount; ++i )
    {
        const int n = ( i + 1 ) % sideCount;
        indices[ic+0]=(uint16_t)(i+64); indices[ic+1]=(uint16_t)(i+96); indices[ic+2]=(uint16_t)(n+96);
        indices[ic+3]=(uint16_t)(n+96); indices[ic+4]=(uint16_t)(n+64); indices[ic+5]=(uint16_t)(i+64);
        ic += 6;
    }
    iassert( ic == 372 );

    if ( !g_qeglobals.d_white )
        return;
    static float st[128][2] = { { 0.0f, 0.0f } };          // memcpy'd unconditionally
    R_AddRenderCmdDrawTris( g_qeglobals.d_white, TECHNIQUE_UNLIT, (short)ic, indices,
                            128, xyzw, normal, color, st );
}

// 0x46CA10  Brush_GetColor2d - eclass colour for non-worldspawn, else d_savedinfo.colors[9].
static void Cam_BrushColor2d( selbrush_t *b, GfxColor *out )
{
    float rgba[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    entity_s     *owner = b->owner;
    entity_s_def *eDef  = owner ? (entity_s_def *)owner->def : nullptr;
    eclass_t     *ec    = eDef ? eDef->eclass : nullptr;
    if ( ec && ec->name && _stricmp( ec->name, "worldspawn" ) != 0 )
    { rgba[0] = ec->color[0]; rgba[1] = ec->color[1]; rgba[2] = ec->color[2]; }
    else
    { rgba[0] = g_qeglobals.d_savedinfo.colors[9][0];
      rgba[1] = g_qeglobals.d_savedinfo.colors[9][1];
      rgba[2] = g_qeglobals.d_savedinfo.colors[9][2]; }
    rgba[3] = 1.0f;
    Byte4PackPixelColor( rgba, out );
}

// 0x407410  DrawTriggerRadius - for a trigger entity with a "radius" or "fixedNodeSafeRadius"
// key, two stacked cylinders per volume ("radius" in the brush 2D colour @alpha 64,
// fixedNodeSafeRadius in cyan 1090453504).  Height: disk trigger (classtype&0x80) 32, else
// the "height" key, else 80.
static void Cam_DrawTriggerRadius( selbrush_t *b )
{
    iassert( b->owner->def == b->def->owner );
    entity_s_def *eDef = (entity_s_def *)b->owner->def;
    const float radius   = Entity_GetFloatValueForKey( (int)(intptr_t)eDef, "radius" );
    const float safeRad  = Entity_GetFloatValueForKey( (int)(intptr_t)eDef, "fixedNodeSafeRadius" );
    if ( !( radius > 0.0f || safeRad > 0.0f ) )
        return;
    if ( ( b->brushFlags & 2 ) != 0 )            // hidden (brushFlags bit 1)
        return;

    // height: disk trigger → 32; else "height" key (>0) else 80.
    float height;
    eclass_t *eclass = eDef->eclass;
    if ( ( eclass->classtype & 0x80 /*CLASS_TRIGGER_DISC*/ ) != 0 )
        height = 32.0f;
    else
    {
        height = Entity_GetFloatValueForKey( (int)(intptr_t)eDef, "height" );
        if ( !( height > 0.0f ) )
            height = 80.0f;
    }

    // center = (origin.x, origin.y, eclass.mins[2] + origin.z)  [disasm: eclass+0x14 = mins.z]
    float center[3];
    center[0] = eDef->origin[0];
    center[1] = eDef->origin[1];
    center[2] = eclass->mins[2] + eDef->origin[2];

    if ( radius > 0.0f )
    {
        GfxColor c;
        Cam_BrushColor2d( b, &c );
        c.array[3] = 64;                          // alpha 64
        Ed_DrawTriggerCylinder( &c, center, radius, height, 0.0f );
        Ed_DrawTriggerCylinder( &c, center, radius, 0.0f, height );
    }
    if ( safeRad > 0.0f )
    {
        GfxColor c;
        c.packed = 1090453504u;                   // binary literal (cyan-ish, a=64)
        Ed_DrawTriggerCylinder( &c, center, safeRad, height, 0.0f );
        Ed_DrawTriggerCylinder( &c, center, safeRad, 0.0f, height );
    }
}

// 0x4076C0  CamWnd_Tokens - the script-colour name-token overlay: parse the space-separated
// ScriptColorTeamKey value of a selected trigger, colour each token via
// ScriptGroup_Unreachable, move the ScriptColorKey-matching token to the front, then draw a
// billboard per matching token on every gated entity.
//   sub_46AAE0 (entity gate): HasKeyValuePair(ScriptColorTeamKey) && classname is one of
//                             actor/node/info_volume.
//   sub_46AA80 (token match): the entity's ScriptColorTeamKey value strstr-contains the token.
static bool Cam_Token_EntityGate( selbrush_t *b )
{
    entity_s_def *e = (entity_s_def *)b->owner->def;
    if ( !HasKeyValuePair( e, g_PrefsDlg->ScriptColorTeamKey ) )
        return false;
    const char *classname = "";
    for ( epair_t *ep = e->epairs; ep; ep = ep->next )
        if ( !_stricmp( ep->key, "classname" ) ) { classname = ep->value; break; }
    return strstr( classname, "actor" ) || strstr( classname, "node" )
        || strstr( classname, "info_volume" ) != nullptr;
}
static bool Cam_Token_Match( selbrush_t *b, const char *token )
{
    entity_s_def *e = (entity_s_def *)b->owner->def;
    const char *teamVal = ValueForKey2( (int)(intptr_t)e, g_PrefsDlg->ScriptColorTeamKey );
    return strstr( teamVal, token ) != nullptr;
}

static void Cam_DrawTokens( const char *teamKeyValue )
{
    char buf[1008];
    strncpy( buf, teamKeyValue, sizeof( buf ) - 1 );
    buf[sizeof( buf ) - 1] = 0;
    if ( !buf[0] )
        return;

    // parse the space-separated tokens; per-token colour from ScriptGroup_Unreachable.
    const int MAX_COLORENTREES = 32;
    char  tokenStr[MAX_COLORENTREES][16];     // v38 (4-dword stride per token, +space)
    float tokenCol[MAX_COLORENTREES][4];      // v28
    int   tokens = 0;
    for ( char *t = strtok( buf, " " ); t; t = strtok( nullptr, " " ) )
    {
        iassert( t[0] );
        if ( tokens >= MAX_COLORENTREES )
        {
            iassert( tokens < MAX_COLORENTREES );
            break;
        }
        _snprintf( tokenStr[tokens], sizeof( tokenStr[tokens] ), "%s ", t );
        tokenStr[tokens][sizeof( tokenStr[tokens] ) - 1] = 0;   // _snprintf may not null-terminate
        // ScriptGroup_Unreachable returns 0..6 (r/b/y/c/g/p/o) or -1 (no match; the binary
        // would index flt_73B098[-1]). Guard the never-valid case (defensive divergence).
        int ci = ScriptGroup_Unreachable( t );
        if ( ci < 0 || ci > 6 )
            continue;
        const float *c = kCamTokenColors[ci];
        tokenCol[tokens][0] = c[0]; tokenCol[tokens][1] = c[1];
        tokenCol[tokens][2] = c[2]; tokenCol[tokens][3] = c[3];
        ++tokens;
    }
    if ( tokens <= 0 )
    {
        iassert( tokens > 0 );
        return;
    }

    // reorder: bring the token whose text contains the ScriptColorKey to the front (the
    // binary swaps the first match into slot [i]==[last]; here move-to-front of slot 0).
    const char *colorKey = (const char *)g_PrefsDlg->ScriptColorKey;
    for ( int i = 0; i + 1 < tokens; ++i )
    {
        if ( strstr( tokenStr[i], colorKey ) )
        {
            if ( i != 0 )
            {
                char  ts[16];   float tc[4];
                memcpy( ts, tokenStr[i], sizeof( ts ) );
                memcpy( tc, tokenCol[i], sizeof( tc ) );
                for ( int k = i; k > 0; --k )
                {
                    memcpy( tokenStr[k], tokenStr[k-1], sizeof( ts ) );
                    memcpy( tokenCol[k], tokenCol[k-1], sizeof( tc ) );
                }
                memcpy( tokenStr[0], ts, sizeof( ts ) );
                memcpy( tokenCol[0], tc, sizeof( tc ) );
            }
            break;
        }
    }

    // for every active + selected gated entity, draw a billboard per matching token.
    extern selbrush_t selected_brushes;
    for ( int pass = 0; pass < 2; ++pass )
    {
        selbrush_t *head = pass ? &selected_brushes : &active_brushes;
        for ( selbrush_t *b = head->next; b && b != head; b = b->next )
        {
            if ( FilterBrush( b, 0 ) )
                continue;
            if ( !Cam_Token_EntityGate( b ) )
                continue;
            for ( int ti = 0; ti < tokens; ++ti )
                if ( Cam_Token_Match( b, tokenStr[ti] ) )
                {
                    Ed_DrawScriptColorQuad( (int)(intptr_t)b->owner->def, tokenCol[ti] );
                }
        }
    }
}

// Defined below (after Cam_Draw); the Cam_Draw tail's 3D-marquee box needs them.
static void CameraCalcRayDir( int y, float *dir, CCamWnd *cam, int x );
void Camera_GetRectSelection3D( int x1, int y1, int x2, int y2, float *outPlanes );

// 0x441240  DrawAdvancedTerrainEditCircle - the terrain-paint brush-radius cursor ring in the
// 3D view: a 16-segment inner+outer ring around the cursor world pos, radii from sub_401BB0 /
// sub_401C00, cyan.  Clips both rings against the ACTIVE patches (only when CurvEditDlg's
// "apply to active" box is set) and the SELECTED patches (PMESH_19_Radius -> 3D lines), then
// emits falloff-coloured control-point dots for selected patches (PMESH_20_Radius_2 -> points).
extern selbrush_t selected_brushes;                          // map.cpp (0x23F1864)
extern float sub_401BB0();                                   // patchdialog/pmesh.cpp — inner radius
extern float sub_401C00();                                   // patchdialog/pmesh.cpp — outer radius
extern int   CurvEditDlg_OnSomeSetting();                    // patchdialog.cpp — "apply to active" (dword_25D6570)
extern void  R_AddCmd_Line3D( short count, char width, GfxPointVertex *verts );  // 0x4FD1A0
extern void  R_AddCmd_Line3DNoDepth( short count, char width, GfxPointVertex *verts );
extern GfxCmdDrawPoints *R_AddPointCmd_W( short pointCount, char size, const GfxPointVertex *verts ); // 0x4FD080
extern int   PMESH_19_Radius( patchMesh_t *patch, const float *cursor, float innerR, float outerR,
                              const float *innerRing, const float *outerRing,
                              const unsigned int *color, int count, GfxPointVertex *outVerts ); // pmesh.cpp 0x43F230
extern int   PMESH_20_Radius_2( int count, patchMesh_t *patch, const float *cursor,
                                float innerR, float outerR, GfxPointVertex *outVerts );          // pmesh.cpp 0x43F580

static GfxCmdDrawPoints *DrawAdvancedTerrainEditCircle( const float *a1 )
{
    extern bool g_radiantTerrainOverlayNoDepth;
    const float innerR = sub_401BB0();        // v19
    const float outerR = sub_401C00();        // v20
    const float ringZ  = a1[2] + 1.0;         // v23 (dbl_6F4098 = 1.0)

    // 16-segment inner + outer rings (3 floats per vertex).
    float innerRing[48];                       // v25 (a5) — radius innerR
    float outerRing[48];                       // v24 (a6) — radius outerR
    for ( int k = 0; k < 16; ++k )             // v21 angle index
    {
        const float ang  = (float)( (double)k * 0.3926990926265717 );   // k * (pi/8)
        const float sinA = (float)sin( ang );  // v16/v22
        const float cosA = (float)cos( ang );  // v17
        innerRing[3 * k + 0] = cosA * innerR + a1[0];
        innerRing[3 * k + 1] = sinA * innerR + a1[1];
        innerRing[3 * k + 2] = ringZ;
        outerRing[3 * k + 0] = cosA * outerR + a1[0];
        outerRing[3 * k + 1] = sinA * outerR + a1[1];
        outerRing[3 * k + 2] = ringZ;
    }

    static const float s_ringColor[4] = { 0.0f, 1.0f, 1.0f, 1.0f };  // flt_6DE1C0 cyan
    GfxColor color;
    Byte4PackPixelColor( const_cast<float *>( s_ringColor ), &color );

    GfxPointVertex verts[1362];                // v26 — on-stack batch
    int lineCount = 0;                         // v1

    const bool oldNoDepth = g_radiantTerrainOverlayNoDepth;
    g_radiantTerrainOverlayNoDepth = true;

    // ACTIVE patches: clipped only when the "apply to active" checkbox is set.
    if ( CurvEditDlg_OnSomeSetting() )
    {
        for ( selbrush_t *i = active_brushes.next; i; i = i->next )
        {
            if ( i == &active_brushes )
                break;
            if ( i->patch )
                lineCount = PMESH_19_Radius( i->patch->def, a1, innerR, outerR,
                                             innerRing, outerRing,
                                             (const unsigned int *)&color, lineCount, verts );
        }
    }
    // SELECTED patches: always clipped (lines).
    for ( selbrush_t *j = selected_brushes.next; j; j = j->next )
    {
        if ( j == &selected_brushes )
            break;
        if ( j->patch )
            lineCount = PMESH_19_Radius( j->patch->def, a1, innerR, outerR,
                                         innerRing, outerRing,
                                         (const unsigned int *)&color, lineCount, verts );
    }
    if ( lineCount )
        R_AddCmd_Line3DNoDepth( (short)( lineCount / 2 ), 1, verts );
    g_radiantTerrainOverlayNoDepth = oldNoDepth;

    // SELECTED patches: falloff-coloured control-point dots.
    int ptCount = 0;
    if ( selected_brushes.next )
    {
        for ( selbrush_t *k = selected_brushes.next; k; k = k->next )
        {
            if ( k == &selected_brushes )
                break;
            if ( k->patch )
                ptCount = PMESH_20_Radius_2( ptCount, k->patch->def, a1, innerR, outerR, verts );
        }
        if ( ptCount )
            return R_AddPointCmd_W( (short)ptCount, 6, verts );
    }
    return (GfxCmdDrawPoints *)(intptr_t)ptCount;
}

// 0x406960  DrawBrush_SunPreview  — the sun-preview LIT re-add of one brush list.
//   for ( i = head->[0]; i != head; i = i->[0] )
//       if ( byte[i+0x26] )                      // selbrush_t::cullFlag
//           Brush_GetColor2d(i,&col),
//           DrawBrush(i, world_orient_matrix, -1, 0, 26, 0, 26, &col, 1, 0, "")
// The retail call passes technique 26 in BOTH of DrawBrush's technique slots (draw_meth1 =
// prefab-content/geo, draw_meth2 = xmodel mesh); the port's DrawBrush reduction carries one
// `technique` filling both roles, so a single 26 is exact.
// It must go through DrawBrush, NOT a per-face loop: the technique's state (sunpre_*.tech ->
// stateMap "additive_stencil" -> depthTest EQUAL, depthWrite off, blend Add/InvDestAlpha/One)
// only lights pixels at the EXACT depth the base pass wrote, so the re-add must reuse the same
// faceVis/prefab-content/patch/model surf-cache entries.
// KISAK: the CullCubic call is an addition - the port's entity pass culls (buffer pressure), so
// the re-add culls identically.  A brush the base pass culled has nothing on screen to re-light.

static int Cam_DrawBrushList_SunPreview( selbrush_t *head, CCamWnd *cam )
{
    extern void DrawBrush( selbrush_t *b, const orientation_t *orient, int viewType,
                           int technique, GfxColor *col, char width, int drawFlags,
                           const char *layerPrefix );                       // brush.cpp 0x47afc0
    extern char CullCubic( selbrush_t *brush, CCamWnd *cam );               // camwnd.cpp
    int drawn = 0;
    // The re-add walks the list BACKWARD: 0x406965 `mov esi,[edi]` seeds from head->prev and
    // 0x4069a9 `mov esi,[esi]` advances by prev (offset 0), unlike DrawGeneralWorld_'s +4 walk.
    for ( selbrush_t *b = head->prev; b && b != head; b = b->prev )
    {
        if ( !b->cullFlag )                       // 0x406970: byte[i+0x26]
            continue;
        if ( cam && CullCubic( b, cam ) )         // KISAK (see above)
            continue;
        GfxColor col;
        Cam_BrushColor2d( b, &col );              // 0x40697c Brush_GetColor2d
        DrawBrush( b, (const orientation_t *)world_orient_matrix, /*viewType*/ -1,
                   /*technique*/ TECHNIQUE_SUNLIGHT_PREVIEW, &col,
                   /*width*/ 1, /*drawFlags*/ 0, /*layerPrefix*/ "" );      // 0x4069a1
        ++drawn;
    }
    return drawn;
}

// 0x4069C0  R_SunPrev_Main - the sun-light preview sequence, at the binary's own position
// (Cam_Draw tail 0x408261, after the main / patch / selected-entity surf flushes):
//   R_SunPrev_SetSunConstants(dir, ambientMul)
//   SetProjection2D
//   R_AddCmdDrawFullScreenColoredQuad(0,0,1,1, ambientMul, mat_white_multiply)
//   R_AddCmdDrawFullScreenColoredQuad(0,0,1,1, colorWhite, rgp.clearAlphaStencilMaterial)
//   shadVol_frontCapIndices = shadVol_quadsPerEdge = (dir.w != 0 ? 6 : 3)   // sun -> 3
//   SunLightPreview_BrushShadow(active) / (selected) + ..._PolyOffsetShadows
//   R_SortMaterials ; DrawBrush_SunPreview(selected) ; DrawBrush_SunPreview(active)
//   R_AddEditorSurfsCmd
// The pass re-draws the world depth-EQUAL and ADDS sun light scaled by INVERSE dest alpha
// (raw/techniques/sunpre_r0c0n0s0.tech -> statemap additive_stencil: blend Add/InvDestAlpha/
// One, depthTest Equal, depthWrite Disable, stencil OneSided Equal Keep/Keep/Keep).  The
// shadow volumes mark shadowed pixels in dest alpha + stencil; the clear_alpha_stencil quad
// zeroes both first.  ORDER IS LOAD-BEARING: everything that should darken must already be
// RASTERISED (not merely queued in the surf cache) before the multiply quad, and nothing may
// write depth or alpha between the clear quad and the re-add.
static void Cam_SunPrev_Main( CCamWnd *cam, bool faithfulSun, Material *sunMultiplyMat,
                              const float litSunDir[3], const float litAmbientMul[3],
                              bool litHaveSun )
{
    if ( !faithfulSun )
        return;

    // 0x4069cf/0x4069d9 - R_SunPrev_Main's FIRST statement is R_SunPrev_SetSunConstants and the
    // WHOLE sequence is gated on its non-zero return: no sun key means no quads, no volumes, no
    // re-add.  Setting the constants here rather than earlier matters — emitted before the world
    // draw they re-shade every material whose technique samples them, and additive blending can
    // never bring those surfaces back up.
    if ( !litHaveSun )
        return;
    {
        float d[3], a[3], c[3];
        Cam_SunPrev_SetSunConstants( d, a, c, /*emit=*/true );
    }

    // 0x4069e3 sets sundir.w = 0 unconditionally, so the shadVol counts are always the
    // directional 3 (`if (0.0 != v4) v1 = 6` can never fire).
    float sun[4]        = { litSunDir[0], litSunDir[1], litSunDir[2], 0.0f };
    float ambientMul[3] = { litAmbientMul[0], litAmbientMul[1], litAmbientMul[2] };

    // 0x4069eb..0x406a3a - the two full-screen quads framing the lit pass: black-world multiply
    // (mat_white_multiply * the worldspawn ambient, dropping the rasterised world to its
    // ambient/diffuse floor) then clear-stencil (zeroes dest alpha + stencil for the volumes).
    // SetProjection2D is for the quads; the 3D projection is restored immediately after.
    R_AddCmdProjectionSet2D();
    if ( sunMultiplyMat )
    {
        float blackCol[4] = { ambientMul[0], ambientMul[1], ambientMul[2], 1.0f };
        R_AddCmdDrawFullScreenColoredQuad( 0.0f, 0.0f, 1.0f, 1.0f, blackCol, sunMultiplyMat );
    }
    if ( rgp.clearAlphaStencilMaterial )
    {
        static const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        R_AddCmdDrawFullScreenColoredQuad( 0.0f, 0.0f, 1.0f, 1.0f, white, rgp.clearAlphaStencilMaterial );
    }
    R_AddCmdProjectionSet3D();

    // 0x406a4c..0x406a8e - the shadow volumes.  frontCapIndices = quadsPerEdge = 3: the
    // directional (w==0) case, where every extruded vertex is the same point at infinity, so
    // the back cap degenerates and one front-cap tri per silhouette tri is all that is kept.
    // BrushShadow recurses into PREFAB CONTENTS with the composed placement orientation;
    // PolyOffsetShadows emits the silhouette side quads from the edge hash and draws the volume
    // through rgp.stencilShadowMaterial (colorWrite off, depthWrite off, depthTest LESS, cull
    // NONE, stencil two-sided z-fail).  Shadowed pixels end with stencil != 0, which the
    // additive_stencil re-add (stencil EQUAL ref 0) then skips.
    extern bool Radiant_ShadVol_Begin( int frontCapPerTri );
    extern void Radiant_ShadVol_BrushShadow( selbrush_t *listHead, const orientation_t *orient,
                                             const float *light );
    extern void SunLightPreview_PolyOffsetShadows();
    extern float world_orient_matrix[4][3];                                 // entity.cpp 0x6DE290
    extern selbrush_t selected_brushes;                                     // map.cpp 0x23F1864
    if ( Radiant_ShadVol_Begin( 3 ) )                                       // 0x406a4c/0x406a66
    {
        const orientation_t *worldOr = (const orientation_t *)world_orient_matrix;
        Radiant_ShadVol_BrushShadow( &active_brushes,   worldOr, sun );     // 0x406a70
        Radiant_ShadVol_BrushShadow( &selected_brushes, worldOr, sun );     // 0x406a86
        SunLightPreview_PolyOffsetShadows();                                // 0x406a8e
    }

    // The LIT re-add (0x406a93..0x406aac).  R_SortMaterials demarcates the flush so it carries
    // ONLY the tech-26 surfs; re-flushing an earlier pass would re-rasterise it on top of the
    // darkened frame.
    {
        extern void R_SortMaterials();                                      // r_ed_scene.cpp
        R_SortMaterials();                                                  // 0x406a93
        Cam_DrawBrushList_SunPreview( &selected_brushes, cam );             // 0x406a9d
        Cam_DrawBrushList_SunPreview( &active_brushes,   cam );             // 0x406aa7
        R_AddEditorSurfsCmd();                                              // 0x406aac
    }
}

void CCamWnd::Cam_Draw()
{
    if ( !active_brushes.next )      // brush lists not bootstrapped → no map loaded
        return;

    if ( !Cam_SetupScene() )         // degenerate projection: skip geometry this frame, the
        return;                      // cleared background stands

    // 0x407ee3 - the camera's WORLD-space cubic/frustum clip planes, so the entity and
    // prefab-content passes below can CullCubic against them.  The binary calls this right
    // after R_SetupScene; without it the entity pass skins every off-screen model.
    Cam_Fov();

    const int layer = g_qeglobals.current_edit_layer;

    // Two sun-preview paths: the FAITHFUL R_SunPrev_Main sequence (black-world multiply quad +
    // clear-stencil quad + stencil shadow volumes + the cached SUNLIGHT_PREVIEW(26) lit draw),
    // or, when the "white_multiply" material is missing (without the black-world reset the lit
    // pass would double-brighten), a KISAK APPROXIMATION: each world face flat-tinted by
    // suncolor*(ambient + diffuse*N.L) through MATERIAL_COLOR under UNLIT - directional shading
    // with no shadows and no per-pixel lighting.  Both are opt-in (preview_sun_aswell = 0).
    const bool sunPrev = Cam_SunPrevEnabled();
    if ( sunPrev )
    {
        // Per frame, NOT one-shot: a newly loaded map must re-read its own worldspawn sun, and
        // live sundirection/suncolor edits must take.  Setup fully rewrites g_edSun each call.
        SunPrev_Setup();
    }
    const bool sunActive = sunPrev && SunPrev_Active();    // sun key present in this map
    const bool useCache  = Cam_SurfCacheEnabled();

    extern Material *Radiant_GetWhiteMultiplyMaterial();   // shadowvolume.cpp
    Material *sunMultiplyMat = sunActive ? Radiant_GetWhiteMultiplyMaterial() : nullptr;
    const bool faithfulSun   = sunActive && sunMultiplyMat != nullptr;
    // On the faithful path the world loop draws TEXTURED (not the N.L tint): the multiply quad
    // darkens that textured world and the cached lit pass then adds the sun on top.
    const bool sunApprox = sunActive && !faithfulSun;      // the per-face N.L MATERIAL_COLOR tint
    // Tell the shared geometry builder whether the per-vertex sun bake applies.
    // Face_BuildLayerGeom multiplies every cached face colour by suncolour*(ambient +
    // diffuse*N.L) when g_edSun.active.  That bake belongs to the APPROXIMATION path alone: on
    // the faithful path it pre-darkens the world before the multiply quad, and the additive
    // tech-26 re-add can never brighten it back.
    {
        extern int g_edSunBakeVertColor;                   // brush.cpp
        const int bake = sunApprox ? 1 : 0;
        // The bake lives IN the built per-vertex colours, so a mode change leaves STALE baked
        // verts in every realized face cache; force the display rebuild (sub_47D060 - the same
        // def-version bump + patch re-tess every layer change makes) on a transition.
        static int s_prevBake = -1;
        if ( bake != s_prevBake )
        {
            s_prevBake = bake;
            extern void sub_47D060( int listHead );        // brush.cpp (0x47D060)
            extern selbrush_t filtered_brushes;            // map.cpp display lists
            sub_47D060( (int)(intptr_t)&active_brushes );
            sub_47D060( (int)(intptr_t)&selected_brushes );
            sub_47D060( (int)(intptr_t)&filtered_brushes );
        }
        g_edSunBakeVertColor = bake;
    }
    // Sun state pre-computed here and carried to the shadow-vol / quad / lit-pass block below.
    float  s_litSunDir[3]    = { 0.0f, 0.0f, 0.0f };
    float  s_litSunColor[3]  = { 1.0f, 1.0f, 1.0f };
    float  s_litAmbientMul[3]= { 0.0f, 0.0f, 0.0f };
    bool   s_litHaveSun      = false;
    static bool s_camSunMsg  = false;   // one-shot faithful-vs-approximation console message
    if ( !s_camSunMsg )
    {
        s_camSunMsg = true;
        if ( faithfulSun )
            Sys_Printf( "Sun light preview: faithful (R_SunPrev_Main).\n" );
        else if ( sunApprox )
            Sys_Printf( "Sun light preview: approximation (per-face directional N.L shading, no "
                        "shadows) - no \"white_multiply\" material for the faithful path.\n" );
    }

    // Values only - the EMISSION happens in Cam_SunPrev_Main, at the binary's own position
    // (after the world draw).  emit=false touches no render state.
    if ( faithfulSun )
    {
        if ( sunActive )
            s_litHaveSun = ( Cam_SunPrev_SetSunConstants( s_litSunDir, s_litAmbientMul,
                                                          s_litSunColor, /*emit=*/false ) != 0 );
        if ( !s_litHaveSun )
        {
            // No sun key: a fixed high-afternoon sun + white light.
            s_litSunDir[0] = -0.30f; s_litSunDir[1] = -0.40f; s_litSunDir[2] = -0.866f;
            s_litSunColor[0] = s_litSunColor[1] = s_litSunColor[2] = 1.0f;
        }
        // NOTE: exactly THREE constants here (0x406896/0x4068c3/0x4068f0) - the binary never
        // touches CONST_SRC_CODE_ENVMAP_PARMS on this path.
    }
    // World technique from camera.draw_mode; default draw_mode 1 -> TECHNIQUE_UNLIT.
    const MaterialTechniqueType worldTech = Cam_TechForDrawMode( camera.draw_mode );

    // Cam_Draw 0x407fb0 tints SELECTED brushes (R_SetMaterialColor(colors[11]) + DrawBrush,
    // gated on !dontDrawSelectedTint) and outlines them in white (the !dontDrawSelectedOutlines
    // tech-29 loop).  Both run as immediate-mode passes below.
    float lastMC[4] = { -1.0f, -1.0f, -1.0f, -1.0f };  // emit MATERIAL_COLOR only on change
    s_sunMCEmitted = 0;                                // reset the sun-preview critical-cmd budget
    // World fill = active_brushes ONLY, matching DrawGeneralWorld_: SELECTED brushes are drawn
    // separately by the red-tint pass below, so drawing them here too would z-fight that fill.
    extern selbrush_t selected_brushes;
    // 0x407ab9 - R_SortMaterials opens the world accumulation so the main R_AddEditorSurfsCmd
    // flush is demarcated and the later selected/white flushes do not re-draw these surfs.
    { extern void R_SortMaterials(); R_SortMaterials(); }
    selbrush_t *bhead = &active_brushes;
    for ( selbrush_t *b = bhead->next; b != bhead; b = b->next )
    {
        brush_t *def = b->def;
        if ( !def || def->patch )                       // convex brushes only
            continue;
        // 0x407af0 - DrawGeneralWorld_ gates every world brush on !CullCubic && !FilterBrush.
        // KISAK: CullCubic stays elided here (the port draws the full map); FilterBrush is
        // load-bearing - without it the map's own lightgrid_volume worldspawn brush renders as
        // opaque teal planes z-fighting the coplanar floor.
        if ( FilterBrush( b, 0 ) )
            continue;
        // FIXEDSIZE POINT ENTITIES ARE NOT WORLD FACES.  DrawBrush 0x47b0bd routes any brush
        // whose eclass is fixedsize down DrawModels (model/prefab contents, no bbox) or, for a
        // model-less one, DrawGeo at the entity-local INVERSE orientation - never as filled
        // world faces at the world orientation.  The entity pass below handles them; drawing
        // their placeholder-bbox faces here renders an opaque box straddling the xmodel.
        {
            entity_s_def *eDef = b->owner ? (entity_s_def *)b->owner->def : nullptr;
            eclass_t     *ec   = eDef ? eDef->eclass : nullptr;
            if ( ec && *(int *)&ec->fixedsize )
                continue;
        }
        for ( int fi = 0; fi < def->faceCount; ++fi )
        {
            face_t   *f   = &def->faces[fi];
            if ( !f->w )
                continue;
            Material *mtl = FaceMaterial( &f->mtldef[layer] );
            if ( !mtl )
                continue;
            if ( sunApprox )
            {
                // UNLIT * per-face directional-sun MATERIAL_COLOR, deduped via lastMC.
                Cam_DrawFaceSun( f, mtl, TECHNIQUE_UNLIT, lastMC );
            }
            // faithfulSun draws the world TEXTURED through the DEFAULT branch below; the
            // multiply quad then darkens it before the cached lit draw adds the sun.
            else if ( useCache )
                Cam_DrawFaceCached( f );                          // RADIANT_SURFCACHE
            else
            {
                // DEFAULT: split world vs tool/sky surfaces.
                //  - WORLD materials -> real texdef texcoords + world normals, drawn with the
                //    draw_mode technique where the material's techset carries it, else UNLIT.
                //  - TOOL/SKY materials -> UNLIT + flat MATERIAL_COLOR.  Tool materials have no
                //    editor colormap and the FAKELIGHT pixel shader (vertcol_shaded) ignores
                //    MATERIAL_COLOR, so a uniform FAKELIGHT would render them flat grey.
                const char *mn = f->mtldef[layer].radMtl ? f->mtldef[layer].radMtl->name : nullptr;
                float ecol[4];
                if ( Cam_EditorMaterialColor( mn, ecol ) )      // tool/sky/volume -> UNLIT + flat colour
                {
                    if ( Cam_MaterialColorChanged( ecol, lastMC ) )
                    {
                        R_AddCmdSetMaterialColor( ecol );
                        Cam_SetLastMaterialColor( lastMC, ecol );
                    }
                    // Tool materials draw with the FLAT WHITE material, not their own: some DO
                    // carry a colormap (light_grid_volume bakes its name into one) and sampling it
                    // at the placeholder 1/128 planar scale stretches that text over the brush.
                    // EXCEPT sky, whose colormap is real content the editor should show.
                    const char *sl = mn ? strrchr( mn, '/' ) : nullptr;
                    const bool isSky = mn && strstr( sl ? sl + 1 : mn, "sky" ) != nullptr;
                    Material *flat = ( !isSky && g_qeglobals.d_white ) ? g_qeglobals.d_white : mtl;
                    Cam_DrawFace( f, flat, TECHNIQUE_UNLIT );
                }
                else                                            // world -> faithful + FAKELIGHT
                {
                    // FAKELIGHT (vertcol_shaded) LERPS by materialColor.w, so the neutral that
                    // shows the texture is w=0 (the binary's {0,0,0,0}) - a float[3] here would
                    // have its .w read OUT OF BOUNDS by R_AddCmdSetMaterialColor.
                    static const float neutral[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
                    if ( Cam_MaterialColorChanged( neutral, lastMC ) )
                    {
                        R_AddCmdSetMaterialColor( neutral );
                        Cam_SetLastMaterialColor( lastMC, neutral );
                    }
                    Cam_DrawFaceFaithfulImmediate( f, worldTech );
                }
            }
        }
    }

    // The default and sun world draws set MATERIAL_COLOR per face; reset to white so the
    // selection overlay and next frame are not tinted.  The cached UNLIT path does not.
    if ( !useCache )
    {
        static const float s_white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        R_AddCmdSetMaterialColor( s_white );
    }

    // 0x407fb0  SELECTED-BRUSH RED TINT: re-draw every selected convex brush tinted by
    // d_savedinfo.colors[11].  Only the COLOUR is gated on !dontDrawSelectedTint - the brushes
    // draw either way, so a selected brush is never invisible.  UNLIT is the technique that
    // honours MATERIAL_COLOR (FAKELIGHT's vertcol_shaded ignores it).  Lights go through
    // DrawLightsMain and patches through the patch loop below, so both are skipped here.
    {
        static const float s_white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        if ( !g_qeglobals.dontDrawSelectedTint )
            R_AddCmdSetMaterialColor( g_qeglobals.d_savedinfo.colors[11] );  // red {1,0.25,0.25}
        else
            R_AddCmdSetMaterialColor( s_white );
        for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        {
            brush_t *def = b->def;
            if ( !def || def->patch )                       // patches → patch-wireframe loop
                continue;
            entity_s_def *eDef = b->owner ? (entity_s_def *)b->owner->def : nullptr;
            if ( eDef && eDef->eclass && ( eDef->eclass->classtype & 1 ) )   // CLASS_LIGHT → skip
                continue;
            // Fixedsize point entities are not world faces - same 0x47b0bd gate as the world
            // fill.  The selected-entity pass below redraws them via DrawBrush -> DrawModels.
            if ( eDef && eDef->eclass && *(int *)&eDef->eclass->fixedsize )
                continue;
            for ( int fi = 0; fi < def->faceCount; ++fi )
            {
                face_t *f = &def->faces[fi];
                if ( f->w )
                    Cam_DrawFaceFaithfulImmediate( f, TECHNIQUE_UNLIT );     // textured * red
            }
        }
        R_AddCmdSetMaterialColor( s_white );                // reset for overlays / next frame
    }

    // ENTITY DRAW (ACTIVE list): bboxes + MODELS + PREFAB CONTENTS.  The world fill above only
    // draws convex WORLD faces, which fixedsize POINT entities do not have, so without this they
    // are invisible in the 3D view.  KISAK: the binary sends every brush through
    // DrawGeneralWorld_ -> DrawBrush at the camera technique; this pass keeps plain point
    // entities (info_*, triggers, path nodes, lights) as tech-29 wireframe bboxes but gives
    // MODEL/PREFAB classes (classtype & 0x18) the camera technique, so DrawModels renders the
    // xmodel mesh and the prefab contents textured.  Identity view orientation (DrawBrush
    // derives the entity inverse itself), viewType -1 so DrawShadedWireframe draws every edge.
    // SELECTED entities get their own tinted pass after the main flush (0x40809d).
    {
        extern selbrush_t selected_brushes;
        {
            selbrush_t *head = &active_brushes;
            for ( selbrush_t *b = head->next; b && b != head; b = b->next )
            {
                brush_t *def = b->def;
                if ( !def || def->patch )                       // patches: own pass below
                    continue;
                entity_s     *owner = b->owner;
                entity_s_def *eDef  = owner ? (entity_s_def *)owner->def : nullptr;
                eclass_t     *ec    = eDef ? eDef->eclass : nullptr;
                if ( !ec || !*(int *)&ec->fixedsize )           // only fixedsize point entities
                    continue;

                // 0x407af0 - CullCubic per world-oriented entity brush.  Without it every
                // off-screen misc_model / misc_prefab is skinned and the skinned-surf buffers
                // overflow, dropping later on-screen models.  Cam_Fov set the planes in WORLD
                // space above, so `this` is the right cam.  Live editor only.
                extern char CullCubic( selbrush_t *brush, CCamWnd *cam );
                if ( CullCubic( b, this ) )
                    continue;

                GfxColor ecol;
                Cam_BrushColor2d( b, &ecol );

                // Model/prefab classes take the camera technique (meshes/contents render lit
                // through the surf-cache); plain point entities keep the wireframe bbox.
                const int entTech = ( ec->classtype & 0x18 /*CLASS_MODEL|CLASS_PREFAB*/ )
                                    ? (int)worldTech : 29;
                // KISAK drawFlags 0 = draw ALL layers in one pass.  The binary runs TWO passes,
                // DrawGeneralWorld_(tech, 8=SKIP_MULTIPLY) @0x407f3b then (tech, 4=ONLY_MULTIPLY)
                // @0x4082f3; this pass runs ONCE, so 8 would drop every additive/effect
                // prefab-content layer.  drawFlags 0 short-circuits Editor_SurfFilter
                // ((drawFlags&0xC)==0), letting all layers draw ordered by material sortKey.
                DrawBrush( b, (orientation_t *)world_orient_matrix, /*viewType (no cull)*/ -1,
                           entTech, &ecol, /*width*/ 1, /*drawFlags*/ 0, /*layerPrefix*/ "" );
            }
        }
    }

    // Curve-point candidate markers (binary DrawGeneralWorld_ → Draw_PatchSelectPoints at
    // 0x407bb4): the GREEN UNSELECTED-candidates overlay, self-gated on sel_curvepoint/
    // sel_area.  The SELECTED set draws light blue via Draw_PatchSelectPointsSelected
    // (0x40c360) from the DrawConnectionLinks position at the Cam_Draw tail.
    Draw_PatchSelectPoints();

    // Light preview: pinned list + SELECTED lights only (ActiveSunLightPreviewInit 0x4066d0's
    // list loop + Cam_Draw 0x408266's selected loop).
    if ( g_PrefsDlg->enable_light_preview )
        Cam_DrawLightPreviews( this );

    // Flush the accumulated surf cache as one RC_DRAW_EDITOR_SKINNEDCACHED: ED_SURF_MESH
    // (brush faces from the cached world fill and the entity pass's prefab contents) plus
    // ED_SURF_MODEL (xmodel meshes).  Unconditional - a no-op when nothing is queued.
    // MATERIAL_COLOR must be the binary's NEUTRAL {0,0,0,0} first.  The lit editor pixel
    // shader family (vertcol_shaded: fakelight_normal/view 24/25 and wireframe_shaded 29) does
    // not MULTIPLY by MATERIAL_COLOR, it LERPS:
    //   result.rgb = lerp( sample(colorMap)*vColor, materialColor.rgb, materialColor.w )
    // so .w is a FLAT-COLOUR OVERRIDE factor - w==1 replaces the texture entirely.  The binary
    // resets via R_SetMaterialColor(NULL) (0x4fc2c0's NULL arm = all zeros) at 0x4080f7 and
    // 0x408115.  KISAK: the $line adaptation pushes eclass colours at w=1, so the reset here is
    // mandatory - and to zeros, NOT white.
    {
        static const float s_flushNeutral[4] = { 0.0f, 0.0f, 0.0f, 0.0f };   // binary R_SetMaterialColor(NULL)
        R_AddCmdSetMaterialColor( s_flushNeutral );
    }
    R_AddEditorSurfsCmd();

    // PATCH fill/wireframe.  DrawGeneralWorld_ -> DrawBrush routes patch brushes to the camera
    // (viewType>2) tech-29 branch, sub_4415D0 - the FILLED per-material-layer patch draw
    // (PMESH_25 instance rebuild + Editor_AddMeshCmd); DrawBrush may still pick
    // DrawPatchesWireframeGrid when the patch-wireframe pref is on.  Self-bracketed: opens with
    // R_SortMaterials and flushes its own surfs, so it neither discards nor re-flushes the main
    // pass's range.  It must NOT straddle the main accumulation - doing so discards every surf
    // the world fill queued and makes the main flush re-draw the patch range.
    {
        extern void DrawBrush( selbrush_t *b, const orientation_t *orient, int viewType,
                               int technique, GfxColor *col, char width, int drawFlags,
                               const char *layerPrefix );
        extern selbrush_t selected_brushes;
        const orientation_t *orient = (const orientation_t *)world_orient_matrix;

        R_SortMaterials();
        for ( int pass = 0; pass < 2; ++pass )
        {
            selbrush_t *head = pass ? &selected_brushes : &active_brushes;
            if ( pass == 1 && !g_qeglobals.dontDrawSelectedTint )
                R_AddCmdSetMaterialColor( g_qeglobals.d_savedinfo.colors[11] );
            for ( selbrush_t *b = head->next; b && b != head; b = b->next )
            {
                if ( !b->def || !b->def->patch )
                    continue;
                GfxColor pcol;
                Cam_BrushColor2d( b, &pcol );
                DrawBrush( b, orient, /*viewType*/ -1, (int)worldTech, &pcol,
                           /*width*/ 1, /*drawFlags*/ pass ? 1 : 0, /*layerPrefix*/ "" );
            }
            if ( pass == 1 && !g_qeglobals.dontDrawSelectedTint )
            {
                static const float s_neutralPatch[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                R_AddCmdSetMaterialColor( s_neutralPatch );
            }
        }
        R_AddEditorSurfsCmd();
    }

    // 0x407fb0 -> 0x40809d  SELECTED-ENTITY textured + red tint: every selected non-light
    // non-patch brush drawn TEXTURED at tech_type, preceded by R_SetMaterialColor(colors[11])
    // (0x407fb7, red {1,0.25,0.25,0.25}, gated on !dontDrawSelectedTint), then its OWN
    // R_AddEditorSurfsCmd (0x4080e3) so only these surfs carry the tint.  vertcol_shaded LERPs,
    // so the result is 0.75*texture + 0.25*red - the textured base under the white wireframe.
    {
        extern void DrawBrush( selbrush_t *b, const orientation_t *orient, int viewType,
                               int technique, GfxColor *col, char width, int drawFlags,
                               const char *layerPrefix );                                // brush.cpp
        extern selbrush_t selected_brushes;
        extern void R_SortMaterials();                                                   // r_ed_scene.cpp
        // 0x407fbf - R_SortMaterials advances sceneSurfCount_saved so the flush below carries
        // ONLY this pass's surfs.  Without it every flush re-draws from surf 0.  The binary
        // calls it before EACH pass: 0x407ab9 world, 0x407fbf tint, 0x4084f0 white.
        R_SortMaterials();
        // 0x407fb7: the red tint, or neutral when the pref is off (the model keeps its texture).
        if ( !g_qeglobals.dontDrawSelectedTint )
            R_AddCmdSetMaterialColor( g_qeglobals.d_savedinfo.colors[11] );   // red {1,0.25,0.25,0.25}
        for ( selbrush_t *b = selected_brushes.next; b && b != &selected_brushes; b = b->next )
        {
            brush_t *def = b->def;
            if ( !def || def->patch )                       // 0x408011: skip CONTENTS_PATCH
                continue;
            entity_s     *owner = b->owner;
            entity_s_def *eDef  = owner ? (entity_s_def *)owner->def : nullptr;
            eclass_t     *ec    = eDef ? eDef->eclass : nullptr;
            if ( !ec || !*(int *)&ec->fixedsize )           // fixedsize point entities (models/prefabs)
                continue;
            if ( ec->classtype & 1 )                        // 0x408020: skip CLASS_LIGHT (DrawLightsMain)
                continue;
            const int entTech = ( ec->classtype & 0x18 /*MODEL|PREFAB*/ ) ? (int)worldTech : 29;
            GfxColor ecol; Cam_BrushColor2d( b, &ecol );     // decoration/bbox colour (col arg)
            // 0x40809d: DrawBrush(b, world_orient, 0xFFFFFFFF, 0, tech_type, 0, tech_type, &col, w, 1, zero)
            DrawBrush( b, (orientation_t *)world_orient_matrix, /*viewType*/ -1,
                       entTech, &ecol, /*width*/ 1, /*drawFlags*/ 1, /*layerPrefix*/ "" );
        }
        // 0x4080e3: flush the selected surfs WHILE matColor is the red tint (separate from the
        // w=0 active flush above), then reset to neutral for the passes that follow.
        R_AddEditorSurfsCmd();
        static const float s_neutral[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        R_AddCmdSetMaterialColor( s_neutral );             // 0x4080f7: R_SetMaterialColor(NULL)
    }

    // 0x408261 - ActiveSunLightPreviewInit, i.e. AFTER the world (0x407f46), patch and
    // selected-entity (0x4080e3) flushes and BEFORE the decoration/white-outline tail.  The
    // position is load-bearing: the multiply quad can only darken ALREADY-RASTERISED pixels and
    // the depth-EQUAL re-add can only land on depths the base pass wrote.
    Cam_SunPrev_Main( this, faithfulSun, sunMultiplyMat, s_litSunDir, s_litAmbientMul, s_litHaveSun );

    // TRIGGER-RADIUS volumes + SCRIPT-COLOUR TOKENS.  Cam_Draw draws these in its
    // selected/active brush overlay loops after the surf flush: DrawTriggerRadius (0x40834E
    // selected-loop non-light + 0x408448 active-loop trigger-class) and CamWnd_Tokens
    // (0x4080DE, gated on sub_4560F0).  KISAK: one combined pass over both lists (they are
    // display-only overlays), default-OFF behind RADIANT_DECOR.
    if ( Radiant_DecorEnabled() )
    {
        extern selbrush_t selected_brushes;
        // DrawTriggerRadius — every trigger_radius/_disk entity (classtype & 0xC0), in
        // both lists (binary: selected-loop else-branch + active-loop trigger-class).
        for ( int pass = 0; pass < 2; ++pass )
        {
            selbrush_t *head = pass ? &selected_brushes : &active_brushes;
            for ( selbrush_t *b = head->next; b && b != head; b = b->next )
            {
                if ( pass == 0 && !b->cullFlag )         // active-loop honours cullFlag (binary)
                    continue;
                entity_s *owner = b->owner;
                if ( !owner )
                    continue;
                entity_s_def *eDef = (entity_s_def *)owner->def;
                if ( !eDef || !eDef->eclass )
                    continue;
                if ( ( eDef->eclass->classtype & 0xC0 /*TRIGGER_RADIUS|TRIGGER_DISC*/ ) != 0 )
                    Cam_DrawTriggerRadius( b );
            }
        }

        // CamWnd_Tokens — sub_4560F0 gate, then the last selected script-trigger's
        // ScriptColorTeamKey value drives the token billboards.
        const bool tokensOn = strcmp( (const char *)g_PrefsDlg->ScriptGroupKey, "token" ) != 0
            && strcmp( (const char *)g_PrefsDlg->ScriptGroupKey, (const char *)g_PrefsDlg->ScriptColorTeamKey ) == 0;
        if ( tokensOn )
        {
            extern bool ScriptGroup_BrushIsTrigger( selbrush_t *b );   // scriptgroup.cpp 0x453FD0
            const char *teamVal = nullptr;
            for ( selbrush_t *b = selected_brushes.next; b && b != &selected_brushes; b = b->next )
                if ( ScriptGroup_BrushIsTrigger( b ) )
                    teamVal = ValueForKey2( (int)(intptr_t)b->owner->def, g_PrefsDlg->ScriptColorTeamKey );
            if ( teamVal && teamVal[0] )
                Cam_DrawTokens( teamVal );
        }

    }

    // 0x4084bd  SELECTED-BRUSH WHITE OUTLINE (!dontDrawSelectedOutlines), 1:1 with
    // 0x4084c3..0x408535: R_AddClearCmd(6 = depth+stencil) -> DrawRegions -> pack colorWhite ->
    // each selected non-patch brush at WIREFRAME tech 29 through the SAME DrawBrush the world
    // pass uses -> ONE R_AddEditorSurfsCmd.  For a fixedsize MODEL/PREFAB that dispatches
    // SkinModelInst at draw_meth2==29, so the model's triangle MESH is queued as an
    // ED_SURF_MODEL at tech 29 and draws as a white wireframe; for a convex brush the same call
    // bottoms out in DrawGeo's tech-29 line outline.
    if ( !g_qeglobals.dontDrawSelectedOutlines )
    {
        // 0x4084d2 - clear DEPTH|STENCIL before the outline draws so the selected wireframe
        // passes the depth test against the coplanar geometry and shows THROUGH.  whichToClear=6
        // means the colour arg is unused.
        extern void R_AddCmdClearScreen( int whichToClear, const float *color, float depth, uint8_t stencil );
        R_AddCmdClearScreen( 6, colorWhite, 1.0f, 0 );
        // KISAK: 0x4084d7 DrawRegions (the region-border line overlay) is not ported - it only
        // draws while a region is active, and is unrelated to the model wireframe.
        // 0x4084f0 - demarcate this WHITE pass so its flush carries ONLY the tech-29 wireframe.
        // Re-flushing the earlier textured surfs after the depth clear re-draws the UNSELECTED
        // prefab's model over a cleared depth buffer.
        extern void R_SortMaterials();                                             // r_ed_scene.cpp
        R_SortMaterials();
        GfxColor whiteCol;
        Byte4PackPixelColor( const_cast<float *>( colorWhite ), &whiteCol );   // 0x4084e8

        extern void DrawBrush( selbrush_t *b, const orientation_t *orient, int viewType,
                               int technique, GfxColor *col, char width, int drawFlags,
                               const char *layerPrefix );                      // brush.cpp 0x47afc0
        for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
        {
            brush_t *def = b->def;
            if ( !def || def->patch )                       // binary skips CONTENTS_PATCH (brushflags&0x100)
                continue;
            // drawFlags=1 (force-draw) matches the binary's a10=1; viewType -1 = no 2D cull.
            DrawBrush( b, (const orientation_t *)world_orient_matrix, /*viewType*/ -1,
                       /*technique*/ 29, &whiteCol, /*width*/ 1, /*drawFlags*/ 1, /*layerPrefix*/ "" );
        }
        R_AddEditorSurfsCmd();                              // 0x408535 — flush the model wireframe surfs
    }

    // Selection overlay: highlight Ctrl+Shift+LMB-picked faces (TASK 2 / P5.7).
    Cam_DrawSelectedFaces();

    // 0x40c9f0  DrawConnectionLinks, at the Cam_Draw tail exactly as in XY_Draw.  Two parts in
    // this order: the sel_vertex/sel_edge grab handles, then the target/targetname +
    // script_linkTo connection lines.  Both are world-space and view-agnostic.
    extern void Ed_DrawVertexHandles();     // xywnd.cpp — DrawConnectionLinks handle prefix
    extern void Ed_DrawConnectionLines();   // xywnd.cpp (Lines_AddLinkTo + Lines_AddLinkToScript)
    extern void Draw_PatchSelectPointsSelected();  // brush.cpp (0x40c360) — SELECTED curve pts, light blue
    Draw_PatchSelectPointsSelected();       // DrawConnectionLinks prefix 0x40ca0f — before the handles
    Ed_DrawVertexHandles();
    Ed_DrawConnectionLines();

    // 0x408680  3D-marquee selection box: with a box-drag active in the 3D view (the drag
    // callback is Camera_GetRectSelection3D and d_select_mode is a marquee/point-rect mode),
    // project the four drag-rect corners to a fixed near depth and draw the translucent blue
    // quad.  Verbatim: rect normalise, 4 CameraCalcRayDir corners, scale = 4.001/(ray.vpn),
    // corner = ray*scale + camera.origin, w=1.
    extern void Ed_DrawSelectionBoxQuad( const float (*verts)[4] );           // xywnd.cpp 0x40CC50
    const select_t sm = g_qeglobals.d_select_mode;
    if ( g_qeglobals.camera_fov_setup == (void *)Camera_GetRectSelection3D
      && ( sm == sel_area || sm == sel_areapoint_vertex || sm == sel_areabrush
        || sm == sel_areabrush_sub || sm == sel_areapoint_curve || sm == sel_areapoint
        || sm == sel_editpoint ) )
    {
        // Normalise the drag rect: x_1/x_2 = press, y_1/y_2 = current (IDA's min/max swap).
        int a4, halfx_i, drag_y2, a1_i;
        if ( g_qeglobals.drag_selectionbox_x_1 >= g_qeglobals.drag_selectionbox_y_1 )
        { a4 = g_qeglobals.drag_selectionbox_y_1; halfx_i = g_qeglobals.drag_selectionbox_x_1; }
        else
        { a4 = g_qeglobals.drag_selectionbox_x_1; halfx_i = g_qeglobals.drag_selectionbox_y_1; }
        if ( g_qeglobals.drag_selectionbox_x_2 >= g_qeglobals.drag_selectionbox_y_2 )
        { drag_y2 = g_qeglobals.drag_selectionbox_y_2; a1_i = g_qeglobals.drag_selectionbox_x_2; }
        else
        { drag_y2 = g_qeglobals.drag_selectionbox_x_2; a1_i = g_qeglobals.drag_selectionbox_y_2; }

        const float depth = 4.000999927520752f;
        const float *o    = this->camera.origin;
        float rays[4][3];
        // 4 corner rays in the IDA's exact order (drag_y2,a4)/(a1,a4)/(a1,halfx)/(drag_y2,halfx).
        CameraCalcRayDir( a4,      rays[0], this, drag_y2 );
        CameraCalcRayDir( a4,      rays[1], this, a1_i );
        CameraCalcRayDir( halfx_i, rays[2], this, a1_i );
        CameraCalcRayDir( halfx_i, rays[3], this, drag_y2 );
        float verts[4][4];
        for ( int i = 0; i < 4; ++i )
        {
            float dn = rays[i][2] * this->camera.vpn[2] + rays[i][1] * this->camera.vpn[1]
                     + rays[i][0] * this->camera.vpn[0];
            float s  = depth / dn;
            verts[i][0] = rays[i][0] * s + o[0];
            verts[i][1] = rays[i][1] * s + o[1];
            verts[i][2] = rays[i][2] * s + o[2];
            verts[i][3] = 1.0f;
        }
        Ed_DrawSelectionBoxQuad( verts );
    }

    // 0x408953-0x408aa5  terrain-paint brush-radius CURSOR RING, gated on cursor_visible AND
    // the terrain-paint mode (sub_401D50 = AdvPatchEditDlg visible + a paint mode + outer>inner,
    // the gate the binary inlines here).  Pick the terrain cell under the cursor, ring it.
    extern int  sub_401D50();                                            // patchdialog.cpp 0x401D50
    extern char sub_43DD50( const float *dir, unsigned char *colorOut,
                            const float *cam_origin, float *origin_out ); // pmesh.cpp 0x43DD50
    if ( this->cursor_visible && sub_401D50() )
    {
        POINT pt;
        ::GetCursorPos( &pt );
        ::ScreenToClient( m_hWnd, &pt );
        RECT rc;
        ::GetClientRect( m_hWnd, &rc );
        if ( pt.x >= rc.left && pt.x < rc.right && pt.y >= rc.top && pt.y < rc.bottom )
        {
            float dir[3];
            CameraCalcRayDir( rc.bottom - pt.y, dir, this, pt.x );   // (a4=bottom-y, dir, this, x)
            float cursorWorld[3];                                    // mouse_origin (vec3)
            if ( sub_43DD50( dir, nullptr, camera.origin, cursorWorld ) )
            {
                // The binary's terrain tail rides after the selected-outline prelude's
                // R_AddClearCmd(6); the port's filled patch pass can leave depth under the
                // cursor, so re-clear to preserve that overlay relationship.
                extern void R_AddCmdClearScreen( int whichToClear, const float *color, float depth, uint8_t stencil );
                R_AddCmdClearScreen( 6, colorWhite, 1.0f, 0 );
                DrawAdvancedTerrainEditCircle( cursorWorld );
            }
        }
    }
}

// CCamWnd MFC window (same skeleton as CXYWnd/CZWnd), rendering into d_hwndCamera, plus the
// 3D mouse interaction: CamWnd_DropModelsToPlane (0x403d30), CameraCalcRayDir (0x403b30),
// Camera_GetRectSelection3D (0x403c10), Cam_MouseControl (0x403950), Cam_MouseUp (0x404f70)
// and the OnLButton handlers (0x403160/0x4031d0).  The alt+ctrl DROP-MODEL / duplicate /
// curve-point-drag sub-paths (prefs-gated, off by default) are an unported TODO in the
// dispatcher.
extern void  Drag_Begin( void *pressFunc, unsigned int buttons, int viewz,
                         int px, int py, float *xvec, float *yvec,
                         float *trace_start, float *trace_dir );          // drag.cpp 0x47E890
extern void  Drag_MouseUp( unsigned int buttons );                       // drag.cpp 0x4802A0
extern void  Vec3Cross( const float *a, const float *b, float *out );    // 0x40A4D0
extern float Vec3Normalize_R( float *v );                                // 0x40A5E0
extern int   g_nPatchClickedView;                                        // 0x73B108
extern char  g_bXYViewIsLastPatchClick;                                  // 0x25D5A6A
extern CMainFrame *g_pParentWnd;                                         // 0x25D5A70
// Cam_MouseMoved (0x404fc0) drag-select / texture rotate-shift dispatch deps:
extern void  Drag_MouseMoved( int x, int y, int buttons, float *origin, float *dir ); // drag.cpp 0x47FF30
extern void  Brush_ShiftTexture( float ds, float dt );                   // select.cpp 0x491F20
extern void  Brush_RotateTexture( int deg );                             // select.cpp 0x4929F0
extern int   sub_401D50();                                               // patchdialog.cpp 0x401D50
extern void  Sys_GetCursorPos( int *x, int *y );                         // win_qe3.cpp 0x499C90 (GetCursorPos wrapper)

// 0x403b30  CameraCalcRayDir - screen (x,y) -> normalised world pick ray.
static void CameraCalcRayDir( int y, float *dir, CCamWnd *cam, int x )
{
    int    height = cam->camera.height;
    double t      = tan( DEG2RAD( g_PrefsDlg->camera_fov ) * 0.5 );
    float  s      = (float)( ( t * 0.75 + t * 0.75 ) / (double)height );
    float  yf     = (float)( (double)( y - height / 2 ) * s );
    float  xf     = (float)( s * (double)( x - cam->camera.width / 2 ) );
    dir[0] = cam->camera.vup[0] * yf + cam->camera.vright[0] * xf + cam->camera.vpn[0];
    dir[1] = cam->camera.vright[1] * xf + cam->camera.vpn[1] + cam->camera.vup[1] * yf;
    dir[2] = yf * cam->camera.vup[2] + xf * cam->camera.vright[2] + cam->camera.vpn[2];
    Vec3Normalize_R( dir );
}

// 0x403c10  Camera_GetRectSelection3D - Drag_Begin's 3D marquee-frustum callback: 4 corner rays
// + the camera origin -> 4 side planes, normal = cross(ray[i], ray[i-1]).
// PLANE STRIDE IS 32 BYTES: normal{x,y,z}@0/4/8, dist as an 8-byte DOUBLE @16, type(int)=-1
// @24.  The double matters - disasm 0x403d16 is `fstp qword ptr [esi-18h]` and
// Patch_SelectAreaPoints_sub reads it back as *(double*)(plane+16).
void Camera_GetRectSelection3D( int x1, int y1, int x2, int y2, float *outPlanes )
{
    int xlo = ( x1 < x2 ) ? x1 : x2;
    int xhi = x2 + x1 - xlo;
    int ylo = ( y1 < y2 ) ? y1 : y2;
    int yhi = y2 + y1 - ylo;
    CCamWnd *cam = g_pParentWnd->m_pCamWnd;
    float rays[4][3];
    CameraCalcRayDir( ylo, rays[0], cam, xlo );
    CameraCalcRayDir( yhi, rays[1], cam, xlo );
    CameraCalcRayDir( yhi, rays[2], cam, xhi );
    CameraCalcRayDir( ylo, rays[3], cam, xhi );
    const float *o = cam->camera.origin;
    for ( int i = 0; i < 4; ++i )
    {
        float *plane = outPlanes + 8 * i;                 // 32-byte plane stride
        Vec3Cross( rays[i], rays[( i - 1 ) & 3], plane );
        // dist (double @ +16): float dot widened to double, matching `fstp qword`.
        *(double *)( plane + 4 ) = (double)( plane[0]*o[0] + plane[1]*o[1] + plane[2]*o[2] );
        *(int *)( plane + 6 ) = -1;                       // type marker @ +24
    }
}

// 0x403950  Cam_MouseControl - RMB-hold cursor-joystick free-look / forward-back fly.
// Reached one-shot from CamWnd_DropModelsToPlane (plain RMB, no Alt) and re-driven every idle
// by CMainFrame::RoutineProcessing (its PostMessage(WM_TIMER) re-pokes OnIdle), hence
// non-static.
void Cam_MouseControl( CCamWnd *cam, float dtime )
{
    if ( g_PrefsDlg->m_nMouseButtons == 2 )
    {
        if ( cam->m_nCambuttonstate != 6 )
            return;
    }
    else if ( !( cam->m_nCambuttonstate == 2 && ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) == 0 ) )
    {
        return;
    }
    if ( g_PrefsDlg->camera_mode )
        return;

    float vert  = (float)( (double)( cam->m_ptButton.y - cam->camera.height / 2 ) / (double)( cam->camera.height / 2 ) );
    float horiz = (float)( (double)( cam->m_ptButton.x - cam->camera.width  / 2 ) / (double)( cam->camera.width  / 2 ) );
    float lateral = (float)( ( 1.0 - fabs( vert ) ) * horiz );
    float turn;
    if ( lateral >= 0.0f )
    {
        turn = lateral - 0.1f;
        if      ( turn < 0.0f )          turn = 0.0f;
        else if ( turn > 0.33000001f )   turn = 0.33000001f;
    }
    else
    {
        turn = lateral + 0.1f;
        if      ( turn > 0.0f )          turn = 0.0f;
        else if ( turn < -0.33000001f )  turn = -0.33000001f;
    }
    g_nUpdateBits |= 4 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
    float speed = (float)( vert * dtime * ( (double)g_PrefsDlg->m_nMoveSpeed * 6.0 ) );
    cam->camera.origin[0] += cam->camera.forward[0] * speed;
    cam->camera.origin[1] += cam->camera.forward[1] * speed;
    cam->camera.origin[2] += cam->camera.forward[2] * speed;
    cam->camera.angles[1] -= dtime * turn * 1250.0f;
    PostMessageA( g_pParentWnd->m_hWnd, WM_TIMER, 0, 0 );
}

// 0x403d30  CamWnd_DropModelsToPlane - the camera button dispatcher (the 3D analogue of
// XY_MouseDown): a selection-button combo opens a Drag_Begin pick/drag with the perspective
// ray; plain RMB without Alt runs the free-look fly.  The alt+ctrl DROP-MODEL
// (m_bDropModel/m_bOrientModel, off by default), curve-point-drag and alt+shift duplicate-drop
// branches at 0x403da7..0x404a4c are unported; plain select never reaches them.
void CamWnd_DropModelsToPlane( CCamWnd *cam, long x, long y, unsigned int nFlags )
{
    // 0x403d6b: m_ptCursor is the free-look pivot that Cam_PositionDrag / Cam_Rotate /
    // Cam_Rotate2 / Cam_PositionPan read and pin the cursor back to on each move.
    GetCursorPos( &cam->m_ptCursor );
    cam->m_nCambuttonstate = nFlags;
    cam->m_ptButton.x = x;
    cam->m_ptButton.y = y;
    int v6 = ( g_PrefsDlg->m_nMouseButtons != 2 ) ? 16 : 2;
    if ( nFlags & 2 )
        cam->cam_was_not_dragged = true;

    bool isSelect = ( nFlags == 1 || nFlags == 5 || nFlags == 9 || nFlags == 13
                   || nFlags == (unsigned)v6 || nFlags == (unsigned)( v6 | 4 )
                   || nFlags == (unsigned)( v6 | 8 ) || nFlags == (unsigned)( v6 | 0xC ) );
    if ( !isSelect )
    {
        if ( nFlags != 2 )
            return;
        if ( GetAsyncKeyState( VK_MENU ) >= 0 )      // plain RMB (no Alt) → free-look fly
        {
            Cam_MouseControl( cam, g_qeglobals.g_oldtime );
            return;
        }
        // Alt+RMB falls through to a marquee drag.
    }

    bool sameView = ( g_nPatchClickedView == 1 );
    g_nPatchClickedView      = 1;
    g_bXYViewIsLastPatchClick = sameView ? 1 : 0;
    float dir[3];
    CameraCalcRayDir( (int)y, dir, cam, (int)x );
    Drag_Begin( (void *)Camera_GetRectSelection3D, nFlags, 2, (int)x, (int)y,
                cam->camera.vright, cam->camera.vup, cam->camera.origin, dir );
}

// 0x404f70  Cam_MouseUp.
static int Cam_MouseUp( unsigned int flags, CCamWnd *cam )
{
    cam->m_nCambuttonstate = 0;
    Drag_MouseUp( flags );
    cam->prob_some_cursor = 0;
    cam->x47 = 0;
    cam->cursor_visible = 1;
    int r;
    do { r = ShowCursor( TRUE ); } while ( r < 0 );
    return r;
}

// 0x403160 / 0x4031d0  CCamWnd::OnLButtonDown / OnLButtonUp.
void CCamWnd::OnLButtonDown( UINT nFlags, CPoint point )
{
    CRect rc;
    GetClientRect( &rc );
    SetFocus();
    SetCapture();
    // The 3D pick uses a bottom-left origin (flip Y), faithful to the binary.
    CamWnd_DropModelsToPlane( this, point.x, rc.bottom - point.y - 1, nFlags );
}

void CCamWnd::OnLButtonUp( UINT nFlags, CPoint point )
{
    Cam_MouseUp( nFlags, this );
    if ( ( nFlags & ( MK_LBUTTON | MK_RBUTTON | MK_MBUTTON ) ) == 0 )
        ReleaseCapture();
    CWnd::OnLButtonUp( nFlags, point );
}

BEGIN_MESSAGE_MAP( CCamWnd, CWnd )
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_KEYDOWN()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_DESTROY()
    // Right-click context-menu commands (IDs from Cam_ContextMenu's AppendMenuA calls).
    ON_COMMAND_RANGE( 0x8CA0, 0x8CB3, OnContextMenuBrushLayer )   // per-face toggle (0x404c20)
    ON_COMMAND( 0x8CB4, OnContextMenuSelectAll )                  // Select all      (0x404cd0)
    ON_COMMAND( 0x8CB5, OnContextMenuDeselectAll )               // Deselect all    (0x404d10)
END_MESSAGE_MAP()

// 0x402f10  CCamWnd::OnDestroy - persist the window placement.
extern BOOL SaveRegistryInfo( const char *pszName, void *pvBuf, int lSize );   // win_qe3.cpp 0x499940
void CCamWnd::OnDestroy()
{
    CWnd::OnDestroy();                                       // 0x402f19
    WINDOWPLACEMENT wndpl;
    wndpl.length = sizeof( wndpl );                          // 0x402f2a (44)
    if ( GetWindowPlacement( &wndpl ) )                      // 0x402f31
        SaveRegistryInfo( "Radiant::CameraWindowPlace", &wndpl, 0x2C );   // 0x402f47
}

CCamWnd::CCamWnd()
{
    memset( &camera, 0, sizeof( camera ) );
    camera.angles[1] = 0.0f;          // yaw → look +X
    // CMainFrame::CreateQEChildren 0x4219cb spawns the camera at (0, 20, 46).
    camera.origin[0] =  0.0f;
    camera.origin[1] = 20.0f;
    camera.origin[2] = 46.0f;
    camera.draw_mode = 1;             // Cam_Draw 0x407dc0 maps mode 1 to TECHNIQUE_UNLIT
}

BOOL CCamWnd::PreCreateWindow( CREATESTRUCT& cs )
{
    cs.lpszClass = AfxRegisterWndClass(
        CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor( NULL, IDC_ARROW ), NULL, NULL );
    cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    return CWnd::PreCreateWindow( cs );
}

int CCamWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
    if ( CWnd::OnCreate( lpCreateStruct ) == -1 )
        return -1;
    CRect rc;
    GetClientRect( &rc );
    m_nWidth  = rc.Width();
    m_nHeight = rc.Height();
    camera.width  = m_nWidth;
    camera.height = m_nHeight;
    return 0;
}

void CCamWnd::OnSize( UINT nType, int cx, int cy )
{
    CWnd::OnSize( nType, cx, cy );
    m_nWidth  = cx;
    m_nHeight = cy;
    camera.width  = cx;
    camera.height = cy;
    if ( dx.device && cx > 0 && cy > 0 )
        R_Hwnd_Resize( (HWND__ *)GetSafeHwnd(), cx, cy );
}

BOOL CCamWnd::OnEraseBkgnd( CDC* /*pDC*/ )
{
    return TRUE;   // D3D present owns the client area
}

void CCamWnd::OnPaint()
{
    CPaintDC dc( this );
    if ( !dx.device )
        return;

    camera.width  = m_nWidth;
    camera.height = m_nHeight;

    HWND__ *hwnd = (HWND__ *)GetSafeHwnd();
    // The floating Dynamic-Lighting popup (CMainFrame::OnDynamicLighting 0x429960) is a CCamWnd
    // whose hwnd the renderer never registered (R_MAX_WINDOWS=5 is full at startup); like the
    // binary it renders nothing.  Skip cleanly so R_SetupRendertarget_CheckDevice's invalid-hwnd
    // assert never fires for that dead window.
    if ( !R_IsRegisteredRenderWindow( hwnd ) )
        return;
    if ( !R_SetupRendertarget_CheckDevice( hwnd ) )
        return;

    R_BeginFrame();
    R_BeginSharedCmdList();
    R_AddCmdClearScreen( 7, g_qeglobals.d_savedinfo.colors[4], 1.0f, 0 );   // COLOR_CAMERABACK
    static const float s_white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    R_AddCmdSetMaterialColor( s_white );

    Cam_Draw();

    R_EndFrame();
    R_IssueRenderCommands( (uint32_t)-1 );
    R_SortMaterials();
    R_CheckTargetWindow( hwnd );
}

// 0x402f60  CCamWnd::OnKeyDown - a thin forward to CMainFrame::OnKeyDown.
extern CMainFrame *g_pParentWnd;     // mainfrm.cpp (0x25D5A70)

// Camera movement in the binary is purely the RMB cursor-joystick fly plus the command-map
// camera-nudge WM_COMMANDs; there are NO built-in WASD/arrow fly keys.
void CCamWnd::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
#if 0 // DISABLED (kept on operator request): local WASD/arrows/QE keyboard fly, a kisak
      // addition the binary lacks.
    bool anyMod = ( GetKeyState( VK_CONTROL ) < 0 ) || ( GetKeyState( VK_SHIFT ) < 0 ) || ( GetKeyState( VK_MENU ) < 0 );
    if ( !anyMod )
    {
        const float step = 32.0f;   // world units per key press
        Cam_BuildMatrix();          // ensure forward/right current
        float *o = camera.origin;
        switch ( nChar )
        {
            case 'W': case VK_UP:
                o[0] += camera.forward[0]*step; o[1] += camera.forward[1]*step; o[2] += camera.forward[2]*step; Invalidate( FALSE ); return;
            case 'S': case VK_DOWN:
                o[0] -= camera.forward[0]*step; o[1] -= camera.forward[1]*step; o[2] -= camera.forward[2]*step; Invalidate( FALSE ); return;
            case 'A': case VK_LEFT:
                o[0] -= camera.right[0]*step;   o[1] -= camera.right[1]*step;   o[2] -= camera.right[2]*step;   Invalidate( FALSE ); return;
            case 'D': case VK_RIGHT:
                o[0] += camera.right[0]*step;   o[1] += camera.right[1]*step;   o[2] += camera.right[2]*step;   Invalidate( FALSE ); return;
            case 'E': case VK_PRIOR: o[2] += step; Invalidate( FALSE ); return;
            case 'Q': case VK_NEXT:  o[2] -= step; Invalidate( FALSE ); return;
            default: break;
        }
    }
#endif
    if ( g_pParentWnd )
        g_pParentWnd->OnKeyDown( nChar, nRepCnt, nFlags );
}

// The binary's camera-control scheme: cursor-JOYSTICK fly + camera_mode view control + 3D LMB
// drag-select + alt texture rotate/shift.
//   OnRButtonDown 0x4032b0 / OnLButtonDown 0x403160 / OnMButtonDown 0x403220 -> the shared
//     dispatcher CamWnd_DropModelsToPlane (y flipped to a bottom-left origin).  Plain RMB (no
//     Alt, camera_mode 0) runs Cam_MouseControl once, then the WM_TIMER/OnIdle pump re-drives.
//   OnMouseMove 0x403100 -> Cam_MouseMoved 0x404fc0 (deduped on m_ptLastCursor).
//   OnRButtonUp 0x403310 -> Cam_ContextMenu -> Cam_MouseUp + ReleaseCapture.
//   The Cam_MouseControl pump lives in CMainFrame::RoutineProcessing (0x421a90).

// 0x4035f0  Cam_PositionDrag - camera_mode 1, RMB: yaw by cursor-dx, fly along the view forward
// (vpn flattened to the XY plane) by cursor-dy.  Cursor re-centred + hidden.
void CCamWnd::Cam_PositionDrag( CCamWnd *cam )
{
    POINT pt;
    GetCursorPos( &pt );
    if ( pt.x == cam->m_ptCursor.x && pt.y == cam->m_ptCursor.y )
        return;
    int dx = pt.x - cam->m_ptCursor.x;
    int dy = pt.y - cam->m_ptCursor.y;
    cam->camera.angles[1] -= (float)( (double)g_PrefsDlg->m_nMoveSpeed / 500.0 * (double)dx );
    float fwd[3] = { cam->camera.vpn[0], cam->camera.vpn[1], 0.0f };
    Vec3Normalize_R( fwd );
    float move = (float)( (double)g_PrefsDlg->m_nMoveSpeed / -250.0 * (double)dy );
    cam->camera.origin[0] += fwd[0] * move;
    cam->camera.origin[1] += fwd[1] * move;
    cam->camera.origin[2] += fwd[2] * move;       // fwd[2]==0 → no Z drift (matches binary)
    SetCursorPos( cam->m_ptCursor.x, cam->m_ptCursor.y );
    cam->cursor_visible = 0;
    ShowCursor( FALSE );
    g_nUpdateBits |= 4 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
}

// 0x403700  Cam_Rotate - RMB+Shift+Ctrl free-look: yaw by cursor-dx, pitch by cursor-dy, both
// scaled by m_nMoveSpeed/500.  Cursor re-centred + hidden.
void CCamWnd::Cam_Rotate( CCamWnd *cam )
{
    POINT pt;
    GetCursorPos( &pt );
    if ( pt.x == cam->m_ptCursor.x && pt.y == cam->m_ptCursor.y )
        return;
    int dx = pt.x - cam->m_ptCursor.x;
    int dy = pt.y - cam->m_ptCursor.y;
    cam->camera.angles[1] -= (float)( (double)g_PrefsDlg->m_nMoveSpeed / 500.0 * (double)dx );
    cam->camera.angles[0] -= (float)( (double)g_PrefsDlg->m_nMoveSpeed / 500.0 * (double)dy );
    SetCursorPos( cam->m_ptCursor.x, cam->m_ptCursor.y );
    cam->cursor_visible = 0;
    ShowCursor( FALSE );
    g_nUpdateBits |= 4 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
}

// 0x4037c0  Cam_Rotate2 - camera_mode 2, RMB: free-look at a fixed 0.35 deg/pixel.
void CCamWnd::Cam_Rotate2( CCamWnd *cam )
{
    POINT pt;
    GetCursorPos( &pt );
    if ( pt.x == cam->m_ptCursor.x && pt.y == cam->m_ptCursor.y )
        return;
    int dx = pt.x - cam->m_ptCursor.x;
    int dy = pt.y - cam->m_ptCursor.y;
    cam->camera.angles[1] -= (float)( (double)dx * 0.3499999940395355 );
    cam->camera.angles[0] -= (float)( 0.3499999940395355 * (double)dy );
    SetCursorPos( cam->m_ptCursor.x, cam->m_ptCursor.y );
    cam->cursor_visible = 0;
    ShowCursor( FALSE );
    g_nUpdateBits |= 4 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
}

// 0x403870  Cam_PositionPan - RMB+Ctrl: strafe along view-right by cursor-dx, move world-Z by
// cursor-dy (m_nMoveSpeed/300).
void CCamWnd::Cam_PositionPan( CCamWnd *cam )
{
    POINT pt;
    GetCursorPos( &pt );
    if ( pt.x == cam->m_ptCursor.x && pt.y == cam->m_ptCursor.y )
        return;
    int dx = pt.x - cam->m_ptCursor.x;
    int dy = pt.y - cam->m_ptCursor.y;
    float strafe = (float)( (double)g_PrefsDlg->m_nMoveSpeed / 300.0 * (double)dx );
    cam->camera.origin[0] += strafe * cam->camera.vright[0];
    cam->camera.origin[1] += strafe * cam->camera.vright[1];
    cam->camera.origin[2] += strafe * cam->camera.vright[2];
    cam->camera.origin[2] -= (float)( (double)g_PrefsDlg->m_nMoveSpeed / 300.0 * (double)dy );
    SetCursorPos( cam->m_ptCursor.x, cam->m_ptCursor.y );
    cam->cursor_visible = 0;
    ShowCursor( FALSE );
    g_nUpdateBits |= 4 * ( g_PrefsDlg->m_bCamXYUpdate != 0 ) + 1;
}

// 0x404fc0  Cam_MouseMoved - the central 3D-view mouse handler, button-mask gated: texture
// ROTATE (Ctrl+RMB+Alt) / texture SHIFT (RMB+Alt) / camera_mode view control (RMB combos) /
// 3D LMB drag-select / shift-ctrl LMB drop-to-plane.
// The 4th arg is an INT y (bottom-left-origin client coord), NOT a float: hex-rays types it
// `float y` from the __userpurge prototype and renders the int reads as LODWORD/SLODWORD, but
// the disasm `fild [ebp+y]` everywhere proves it is an int.
extern "C" int ClampGridSize();                                          // drag.cpp (0x463a80)
extern float   grid_sizes[];                                             // engine_stubs (0x6dde5c)
static double Cam_MaxF( float a, float b ) { return ( a - b >= 0.0f ) ? a : b; } // sub_40A1D0 (max)
void CCamWnd::Cam_MouseMoved( CCamWnd *cam, unsigned int buttons, int x, int y )
{
    cam->m_nCambuttonstate = buttons;
    if ( !buttons )
    {
        if ( sub_401D50() )                      // terrain-paint mode active → just repaint
            g_nUpdateBits |= W_CAMERA;
        return;
    }
    if ( cam->m_ptButton.x != x || cam->m_ptButton.y != y )
        cam->cam_was_not_dragged = false;
    cam->m_ptButton.x = x;
    cam->m_ptButton.y = y;

    const bool altDown = ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) != 0;

    if ( buttons == MK_RBUTTON )
    {
        if ( altDown )
        {
            // ── RMB+Alt → texture SHIFT (drag the texture in S/T by grid-snapped cursor delta) ──
            if ( g_qeglobals.d_select_mode == sel_addpoint )
            {
                float dir[3];
                CameraCalcRayDir( y, dir, cam, x );
                Drag_MouseMoved( x, y, 2, cam->camera.origin, dir );
                g_nUpdateBits |= W_CAMERA;
                return;
            }
            int cx, cy;
            Sys_GetCursorPos( &cx, &cy );
            if ( cx == cam->m_ptCursor.x && cy == cam->m_ptCursor.y )
                return;
            cam->prob_some_cursor += cx - cam->m_ptCursor.x;
            cam->x47             += cy - cam->m_ptCursor.y;
            float accX = (float)cam->prob_some_cursor;
            float gs   = grid_sizes[g_qeglobals.d_gridsize];
            float remX = (float)fmod( accX, gs );
            float accY = (float)cam->x47;
            float remY = (float)fmod( accY, gs );
            if ( remX != accX || remY != accY )
            {
                int snapX = (int)( accX - remX );
                cam->prob_some_cursor -= snapX;
                int snapY = (int)( accY - remY );
                cam->x47             -= snapY;
                Brush_ShiftTexture( (float)snapX, (float)snapY );
            }
            SetCursorPos( cam->m_ptCursor.x, cam->m_ptCursor.y );
            ShowCursor( FALSE );
            cam->cursor_visible = 0;
            return;
        }
        // RMB, no Alt → fall through to the camera_mode / drag-select dispatch.
    }
    else if ( buttons == ( MK_RBUTTON | MK_CONTROL ) && altDown )
    {
        // ── Ctrl+RMB+Alt → texture ROTATE (grid-snapped, ClampGridSize degrees) ──
        POINT pt;
        GetCursorPos( &pt );
        if ( pt.x == cam->m_ptCursor.x && pt.y == cam->m_ptCursor.y )
            return;
        cam->prob_some_cursor += pt.x - cam->m_ptCursor.x;
        cam->x47             += pt.y - cam->m_ptCursor.y;
        int accX = cam->prob_some_cursor;
        int accY = cam->x47;
        int step = (int)Cam_MaxF( 1.0f, grid_sizes[g_qeglobals.d_gridsize] );   // sub_40A1D0(1, gs)
        int remX = accX % step;
        bool snap = ( accX != remX );
        if ( !snap && ( accY % step ) != accY )      // (accX snapped) OR (accY snapped)
            snap = true;
        if ( !snap )
        {
            // no whole grid-step accumulated yet → just recentre, no rotate
            SetCursorPos( cam->m_ptCursor.x, cam->m_ptCursor.y );
            ShowCursor( FALSE );
            cam->cursor_visible = 0;
            return;
        }
        int snappedX = accX - remX;
        cam->prob_some_cursor = remX;
        int remY = accY % step;
        cam->x47 = remY;
        int snappedY = accY - remY;
        int deg = ClampGridSize();
        Brush_RotateTexture( ( snappedX / step + snappedY / step ) * deg );
        SetCursorPos( cam->m_ptCursor.x, cam->m_ptCursor.y );
        ShowCursor( FALSE );
        cam->cursor_visible = 0;
        return;
    }

    // ── LABEL_27: camera_mode 1/2 view control, then the LMB drag-select / drop-to-plane ──
    if ( ( buttons & MK_RBUTTON ) != 0 && !altDown && g_PrefsDlg->camera_mode == 2 )
    {
        if ( buttons & MK_CONTROL ) Cam_PositionPan( cam );
        else                        Cam_Rotate2( cam );
        g_nUpdateBits |= W_CAMERA;
        return;
    }
    if ( buttons == MK_RBUTTON )
    {
        if ( !altDown && g_PrefsDlg->camera_mode == 1 )
        {
            Cam_PositionDrag( cam );
            g_nUpdateBits |= W_CAMERA;
            return;
        }
    }
    else if ( buttons == ( MK_RBUTTON | MK_SHIFT | MK_CONTROL ) )
    {
        if ( !altDown )
        {
            Cam_Rotate( cam );
            g_nUpdateBits |= W_CAMERA;
            return;
        }
    }
    else if ( buttons == ( MK_RBUTTON | MK_CONTROL ) && !altDown )
    {
        Cam_PositionPan( cam );
        g_nUpdateBits |= W_CAMERA;
        return;
    }

    GetCursorPos( &cam->m_ptCursor );
    if ( ( buttons & ( MK_LBUTTON | MK_MBUTTON ) ) == 0 )
        return;

    if ( !g_qeglobals.toggle_unk03_mousedrag_state1 && !g_qeglobals.toggle_unk04_mousedrag_state2 )
    {
        // ── plain LMB/MMB drag: continue the 3D ray drag-select (the marquee gap is now closed) ──
        float dir[3];
        CameraCalcRayDir( y, dir, cam, x );
        Drag_MouseMoved( x, y, buttons, cam->camera.origin, dir );
        if ( ( buttons & MK_LBUTTON ) != 0 && ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) != 0 )
            g_nUpdateBits |= W_CAMERA;            // Alt+LMB terrain paint → camera only
        else
            g_nUpdateBits |= ( W_CAMERA | W_XY | W_Z );   // 0x0B
        return;
    }

    // ── a marquee/point-rect drag is in progress (toggle_unk03/04): re-dispatch or finalize ──
    if ( g_qeglobals.d_select_mode == sel_areabrush || g_qeglobals.d_select_mode == sel_areabrush_sub
      || g_qeglobals.d_select_mode == sel_areapoint_curve || g_qeglobals.d_select_mode == sel_areapoint )
    {
        g_qeglobals.toggle_unk03_mousedrag_state1 = 0;
        g_qeglobals.toggle_unk04_mousedrag_state2 = 0;
        return;
    }
    if ( buttons == ( MK_LBUTTON | MK_SHIFT )                          // 5
      || ( buttons == ( MK_LBUTTON | MK_CONTROL ) && !altDown )        // 9, no Alt
      || buttons == ( MK_LBUTTON | MK_SHIFT | MK_CONTROL ) )           // 0x0D
    {
        CamWnd_DropModelsToPlane( cam, x, y, buttons );
        return;
    }
    g_qeglobals.toggle_unk03_mousedrag_state1 = 0;
    g_qeglobals.toggle_unk04_mousedrag_state2 = 0;
}

// Cam_ContextMenu deps (select.cpp / material).
extern void Test_Ray( float *start, float *dir, int contents,
                      edTrace_t *t, int num_traces );                 // select.cpp 0x48D7C0
extern void Select_Brush( selbrush_t *b, char some_overwrite,
                          char bStatus, char center );                // select.cpp 0x48DCC0
extern void Deselect_Brush( selbrush_t *b );                         // select.cpp 0x48DC60

// The pick-ray hit list Cam_ContextMenu fills and the WM_COMMAND handlers read.  IDB
// camera_trace @ 0x1808e00 is `edTrace_t camera_trace[20]`, 88-byte stride (the build loop
// ends at &camera_trace[20] = byte_18094E0).
#define CAM_TRACE_COUNT 20
static edTrace_t camera_trace[CAM_TRACE_COUNT];

// Popup command IDs (hardcoded in the binary's AppendMenuA / message map @ 0x6d50e0).
#define ID_BRUSH_LAYER_BASE 0x8CA0   // 36000 — first per-face toggle entry (base + trace index)
#define ID_BRUSH_LAYER_MAX  0x8CB3   // 36019 — last per-face entry (20 slots, base..base+19)
#define ID_CAM_SELECT_ALL   0x8CB4
#define ID_CAM_DESELECT_ALL 0x8CB5

// 0x511070  handle -> display name (r_material.cpp:1484).
static const char *Cam_MaterialHandleName( Material *handle )
{
    iassert( handle );   // r_material.cpp:1484
    return Material_FromHandle( handle )->info.name;
}

// 0x404BA0  the sub_408CA0 (std::sort) comparator: order two hit faces by their base material's
// sort layer DESCENDING; a face-less hit sorts last.  The layer is the 12-bit field at drawSurf
// bits 29..40 (`>> 29 & 0xFFF`).  qsort wants a tri-state int, so derive it from two `<`.
static int __cdecl Cam_TraceMtlLess( const edTrace_t *a, const edTrace_t *b )
{
    faceVis_s *fa = a->hit.face;
    faceVis_s *fb = b->hit.face;
    // less(a,b):
    bool ab;
    if ( !fa )                ab = false;
    else if ( !fb )           ab = true;
    else {
        unsigned la = (unsigned)((Material_FromHandle( fa->visArray->mtlHandle )->info.drawSurf.packed >> 29) & 0xFFF);
        unsigned lb = (unsigned)((Material_FromHandle( fb->visArray->mtlHandle )->info.drawSurf.packed >> 29) & 0xFFF);
        ab = la > lb;
    }
    if ( ab ) return -1;
    // less(b,a):
    bool ba;
    if ( !fb )                ba = false;
    else if ( !fa )           ba = true;
    else {
        unsigned la = (unsigned)((Material_FromHandle( fa->visArray->mtlHandle )->info.drawSurf.packed >> 29) & 0xFFF);
        unsigned lb = (unsigned)((Material_FromHandle( fb->visArray->mtlHandle )->info.drawSurf.packed >> 29) & 0xFFF);
        ba = lb > la;
    }
    return ba ? 1 : 0;
}

// 0x404d40  Cam_ContextMenu - the RMB-release brush-face picker popup.  Fires only when
// m_bRightClick && cam_was_not_dragged && no Alt/Ctrl held (Shift also blocks it outside
// camera_mode 0), so a camera fly or drag never opens it.  Pick ray (up to 20 hits) sorted by
// material layer, then one Select/Deselect toggle per face within 1 unit of the nearest hit,
// a separator, and Select-all / Deselect-all.
void CCamWnd::Cam_ContextMenu( CCamWnd *cam, int x, int y )
{
    if ( !g_PrefsDlg->m_bRightClick )                    // 0x404d4e
        return;
    if ( !cam->cam_was_not_dragged )                     // 0x404d5d — a drag happened; no menu
        return;
    // Alt or Ctrl held blocks the menu; Shift blocks it ONLY when camera_mode != 0 (in mode 0
    // Shift is a modifier for the RMB drag combos).  Disasm 0x404d70-0x404da0: JS-exit on
    // VK_MENU/VK_CONTROL, then `cmp camera_mode,0 / jz skip` around the VK_SHIFT test.
    if ( GetKeyState( VK_MENU ) < 0 || GetKeyState( VK_CONTROL ) < 0 )                   // 0x404d74/0x404d84
        return;
    if ( g_PrefsDlg->camera_mode != 0 && GetKeyState( VK_SHIFT ) < 0 )                   // 0x404d90-0x404da0
        return;

    if ( cam->m_contextMenu.m_hMenu )                    // 0x404db4 — rebuild each time
        cam->m_contextMenu.DestroyMenu();
    cam->m_contextMenu.Attach( CreatePopupMenu() );      // 0x404dc1/0x404dca

    float dir[3];
    CameraCalcRayDir( y, dir, cam, x );                  // 0x404ddc
    Test_Ray( cam->camera.origin, dir, 0, camera_trace, CAM_TRACE_COUNT );   // 0x404df2

    if ( !camera_trace[0].hit.brush )                        // 0x404e00 — nothing hit
        return;

    float nearDist = camera_trace[0].dist;               // 0x404e14
    int   nSelected = 0;                                 // var_24 — hits already selected
    int   nUnselected = 0;                               // var_28 — hits not yet selected

    // std::sort the 20-slot list by material layer (descending). Face-less trailing slots go last.
    qsort( camera_trace, CAM_TRACE_COUNT, sizeof( edTrace_t ), (int(__cdecl*)(const void*,const void*))Cam_TraceMtlLess );

    for ( int i = 0; i < CAM_TRACE_COUNT; ++i )          // 0x404e31..0x404ed7
    {
        edTrace_t *tr = &camera_trace[i];
        float d = fabsf( tr->dist - nearDist );          // 0x404e38..0x404e42
        if ( d > 1.0f )                                  // 0x404e55 — beyond the near cluster
        {
            tr->hit.brush = nullptr;                         // 0x404e57 — drop it from the list
            continue;
        }
        UINT flags;
        if ( tr->selected )                              // 0x404e5f
        {
            ++nSelected;                                 // 0x404e65
            flags = MF_CHECKED;                          // 8 — already-selected entries are checked
        }
        else
        {
            flags = MF_UNCHECKED;                        // 0
            ++nUnselected;                               // 0x404e73
        }
        iassert( tr->hit.face->visCount == 1 );              // 0x404e7f (CamWnd.cpp:1108)
        const char *name = Cam_MaterialHandleName( tr->hit.face->visArray->mtlHandle );   // 0x404ea8
        cam->m_contextMenu.AppendMenuA( flags, ID_BRUSH_LAYER_BASE + i, name );       // 0x404ec3
    }

    cam->m_contextMenu.AppendMenuA( MF_SEPARATOR, 0, (LPCSTR)nullptr );   // 0x404ef3 — flag 0x800
    // Select-all: enabled (flag 0) only when at least one hit is unselected, else grayed (flag 1).
    cam->m_contextMenu.AppendMenuA( nUnselected ? MF_ENABLED : MF_GRAYED,
                                    ID_CAM_SELECT_ALL, "Select all" );     // 0x404f0f/0x404f1a
    // Deselect-all: enabled only when at least one hit is already selected, else grayed.
    cam->m_contextMenu.AppendMenuA( nSelected ? MF_ENABLED : MF_GRAYED,
                                    ID_CAM_DESELECT_ALL, "Deselect all" ); // 0x404f36/0x404f41

    POINT pt;
    GetCursorPos( &pt );                                  // 0x404f48
    cam->m_contextMenu.TrackPopupMenu( TPM_RIGHTBUTTON, pt.x, pt.y, cam, nullptr );   // 0x404f61
}

// 0x404c20  ON_COMMAND_RANGE(0x8CA0..0x8CB3): toggle select/deselect of camera_trace[i].hit.brush
// and flip the cached selected flag so the next popup shows the new state.
void CCamWnd::OnContextMenuBrushLayer( UINT nID )
{
    // 0x404c35 (CamWnd.cpp:995): nID >= ID_BRUSH_LAYER_BASE && nID <= ID_BRUSH_LAYER_MAX
    iassert( nID >= ID_BRUSH_LAYER_BASE && nID <= ID_BRUSH_LAYER_MAX );
    int i = nID - ID_BRUSH_LAYER_BASE;                                   // 0x404c5b
    iassert( camera_trace[i].hit.brush );                                    // 0x404c5e (CamWnd.cpp:999 g_traces[nID].hit.brush)
    if ( camera_trace[i].selected )                                      // 0x404c85
    {
        Deselect_Brush( camera_trace[i].hit.brush );                        // 0x404cb5
        camera_trace[i].selected = false;
    }
    else
    {
        Select_Brush( camera_trace[i].hit.brush, 0, 0, 0 );                 // 0x404c9a
        camera_trace[i].selected = true;
    }
}

// 0x404cd0  ON_COMMAND(0x8CB4): select every listed hit brush.
void CCamWnd::OnContextMenuSelectAll()
{
    for ( int i = 0; i < CAM_TRACE_COUNT; ++i )                         // 0x404cd8..0x404d06
    {
        if ( camera_trace[i].hit.brush && !camera_trace[i].selected )
        {
            Select_Brush( camera_trace[i].hit.brush, 0, 0, 0 );            // 0x404cf2
            camera_trace[i].selected = true;
        }
    }
}

// 0x404d10  ON_COMMAND(0x8CB5): deselect every listed hit brush.
void CCamWnd::OnContextMenuDeselectAll()
{
    for ( int i = 0; i < CAM_TRACE_COUNT; ++i )                         // 0x404d12..0x404d34
    {
        if ( camera_trace[i].hit.brush && camera_trace[i].selected )
        {
            Deselect_Brush( camera_trace[i].hit.brush );                   // 0x404d23
            camera_trace[i].selected = false;
        }
    }
}

void CCamWnd::OnRButtonDown( UINT nFlags, CPoint point )
{
    CRect rc;
    GetClientRect( &rc );
    SetFocus();
    SetCapture();
    // Shared button dispatcher (LMB/MMB/RMB all route here), bottom-left origin (flip Y).
    CamWnd_DropModelsToPlane( this, point.x, rc.bottom - point.y - 1, nFlags );
}

void CCamWnd::OnRButtonUp( UINT nFlags, CPoint point )
{
    CRect rc;
    GetClientRect( &rc );
    // KISAK ORDER DEVIATION from 0x403310: the binary does ContextMenu -> Cam_MouseUp ->
    // ReleaseCapture, popping the menu while the RMB-down SetCapture is STILL HELD.  In this
    // build's Win32/MFC runtime TrackPopupMenu with the mouse captured by our own window
    // no-shows / instantly dismisses - the port's own 2D path (CXYWnd::OnRButtonUp) had to
    // release first too.  Cam_MouseUp only resets button/cursor state and does NOT touch
    // cam_was_not_dragged, so the menu's drag gate survives the reorder.
    Cam_MouseUp( nFlags, this );
    if ( ( nFlags & ( MK_LBUTTON | MK_RBUTTON | MK_MBUTTON ) ) == 0 )
        ReleaseCapture();
    Cam_ContextMenu( this, point.x, rc.bottom - point.y - 1 );   // no-drag right-click popup
    // 0x403367: the binary does NOT chain to CWnd::OnRButtonUp (which would DefWindowProc ->
    // WM_CONTEXTMENU); omit it to match.
}

void CCamWnd::OnMouseMove( UINT nFlags, CPoint point )
{
    CRect rc;
    GetClientRect( &rc );
    // Dedup: the binary skips Cam_MouseMoved when the cursor hasn't moved (m_ptLastCursor).
    if ( m_ptLastCursor.x != point.x || m_ptLastCursor.y != point.y )
        Cam_MouseMoved( this, nFlags, point.x, rc.bottom - point.y - 1 );
    m_ptLastCursor = point;
}

#if 0 // DISABLED (kept on operator request): the FPS mouse-look camera that replaced the
      // binary's scheme.  To re-enable, flip this to #if 1, disable the faithful
      // OnRButtonDown/OnRButtonUp/OnMouseMove above, restore m_bLooking/m_ptLook in mainfrm.h.
void CCamWnd::OnRButtonDown( UINT nFlags, CPoint point )
{
    m_bLooking = true;
    m_ptLook   = point;
    SetCapture();
    SetFocus();                     // so WASD keys come here
    CWnd::OnRButtonDown( nFlags, point );
}

void CCamWnd::OnRButtonUp( UINT nFlags, CPoint point )
{
    m_bLooking = false;
    ReleaseCapture();
    CWnd::OnRButtonUp( nFlags, point );
}

void CCamWnd::OnMouseMove( UINT nFlags, CPoint point )
{
    if ( m_bLooking )
    {
        const float sens = 0.25f;   // degrees per pixel
        camera.angles[1] -= (float)( point.x - m_ptLook.x ) * sens;   // yaw
        camera.angles[0] += (float)( point.y - m_ptLook.y ) * sens;   // pitch
        if ( camera.angles[0] >  85.0f ) camera.angles[0] =  85.0f;
        if ( camera.angles[0] < -85.0f ) camera.angles[0] = -85.0f;
        m_ptLook = point;
        Invalidate( FALSE );
    }
    CWnd::OnMouseMove( nFlags, point );
}
#endif

// KISAK, no binary counterpart: frame the loaded map's brush bounds (the binary just spawns the
// camera at the fixed ctor origin).  Sit back on -X, slightly above, aimed at the centre.
void Cam_CenterOnMap( CCamWnd *cam )
{
    if ( !cam ) return;
    float mins[3] = {  1e30f,  1e30f,  1e30f };
    float maxs[3] = { -1e30f, -1e30f, -1e30f };
    bool any = false;
    for ( selbrush_t *b = active_brushes.next; b && b != &active_brushes; b = b->next )
    {
        brush_t *def = b->def;
        if ( !def ) continue;
        for ( int i = 0; i < 3; ++i )
        {
            if ( def->mins[i] < mins[i] ) mins[i] = def->mins[i];
            if ( def->maxs[i] > maxs[i] ) maxs[i] = def->maxs[i];
        }
        any = true;
    }
    if ( !any ) return;

    float c[3]  = { 0.5f*(mins[0]+maxs[0]), 0.5f*(mins[1]+maxs[1]), 0.5f*(mins[2]+maxs[2]) };
    float ext   = maxs[0]-mins[0];
    if ( maxs[1]-mins[1] > ext ) ext = maxs[1]-mins[1];
    if ( ext < 64.0f ) ext = 64.0f;

    // The look angles must account for Cam_BuildMatrix negating pitch: AngleVectors(p) gives
    // forward.z = -sin(p) with p = -camera.angles[0], so for L = normalize(centre-origin),
    // angles[0] = deg(asin(L.z)) and angles[1] = deg(atan2(L.y, L.x)).
    cam->camera.origin[0] = c[0] - ext * 1.05f;   // well back on -X
    cam->camera.origin[1] = c[1] - ext * 0.10f;
    cam->camera.origin[2] = c[2] + ext * 0.15f;   // only slightly above centre → near-horizontal

    float L[3] = { c[0]-cam->camera.origin[0], c[1]-cam->camera.origin[1], c[2]-cam->camera.origin[2] };
    float len  = sqrtf( L[0]*L[0] + L[1]*L[1] + L[2]*L[2] );
    if ( len < 1e-4f ) len = 1.0f;
    L[0]/=len; L[1]/=len; L[2]/=len;
    cam->camera.angles[0] = RAD2DEG( asinf( L[2] ) );          // pitch (look up/down)
    cam->camera.angles[1] = RAD2DEG( atan2f( L[1], L[0] ) );   // yaw
    cam->camera.angles[2] = 0.0f;

    Radiant_FL_Log( "cam fit: origin=(%g %g %g) angles=(%g %g) ext=%g",
        cam->camera.origin[0], cam->camera.origin[1], cam->camera.origin[2],
        cam->camera.angles[0], cam->camera.angles[1], ext );
}

// 0x4034E0  Cam_ChangeFloor - View->Up Floor / Down Floor: drop or raise the camera so its
// "feet" (eye - 48) rest on the nearest brush surface above / below.  Casts a vertical ray DOWN
// from z = 131072 through every active brush.
//   `current` = distance from that high start down to the feet (131072 - (origin.z - 48)); a
//   hit distance t < current is ABOVE the feet, t > current is BELOW.
//   up   (a2!=0): best starts 0,      keep the LARGEST t still < current; best==0 means none.
//   down (a2==0): best starts 262144, keep the SMALLEST t still > current; 262144 means none.
//   On a hit: origin.z += (current - best) and g_nUpdateBits |= 0x21.  0x21 is the raw
//   immediate at 0x4035d1 = W_CAMERA|W_Z_OVERLAY: CMainFrame::UpdateWindows 0x427090 tests
//   `bl & 28h` for the Z window and `bl & 10h` for the texture window, so W_Z_OVERLAY==0x20.
// The comparison directions are read from the FPU status-word tests, not simplified.
void CCamWnd::Cam_ChangeFloor( CCamWnd *cam, int a2 )
{
    if ( !cam ) return;

    float start[3] = { cam->camera.origin[0], cam->camera.origin[1], 131072.0f };
    float dir[3]   = { 0.0f, 0.0f, -1.0f };
    float current  = 131072.0f - ( cam->camera.origin[2] - 48.0f );
    float best     = a2 ? 0.0f : 262144.0f;

    for ( selbrush_t *b = active_brushes.next; b != &active_brushes; b = b->next )
    {
        brush_t *def = b->def;
        if ( !def ) continue;
        float t;
        if ( !Ed_BrushFloorRay( def, start, dir, &t ) )
            continue;
        if ( a2 )
        {
            if ( current > (double)t && best < (double)t )
                best = t;
        }
        else if ( current < (double)t && best > (double)t )
        {
            best = t;
        }
    }

    if ( best != 0.0f && best != 262144.0f )
    {
        g_nUpdateBits |= 0x21;   // W_CAMERA|W_Z_OVERLAY (raw `or g_nUpdateBits,21h`)
        cam->camera.origin[2] = current - best + cam->camera.origin[2];
    }
}

// Light-region "build regions for selected lights": CMainFrame::OnShowRegionsForSelected ->
// Regions_ForSelected (0x406F10) -> sub_406E00 -> sub_406CE0 ->
// { Brush_DrawSubmitFaceWindings / PMESH_29_Winding } -> the primarylights_region CSG -> the
// global region-hull array.
// The `float *a3` the face submitters take points at the light descriptor
// (lightDesc_t {int cls; float p[8]}): a3[1..3] = cone centre, a3[7] = radius, a3[8] =
// cosHalfFov - i.e. a3 == (float*)&desc with cls reinterpreted at a3[0].
#include "primarylights_region.h"
extern char  *ValueForKey2( int e, const char *key );                      // entity.cpp 0x4825C0
extern int    Entity_GetIntValueForKey( int e, const char *key );          // entity.cpp 0x483820
extern bool   Entity_HasEpairMatch( entity_s *e, const char *key, const char *val ); // entity.cpp
extern void   Entity_GetOrientation( entity_s_def *ent, orientation_t *orParent, orientation_t *orOut ); // entity.cpp
extern char   FilterBrush( selbrush_t *b, int a2 );                        // filters.cpp 0x46A1F0
extern entity_s entities;                                                   // entity.cpp 0x23F17A0
extern int    g_windingAlloc;
extern char  *va( const char *fmt, ... );
extern void   Brush_DrawSubmitFaceWindings( selbrush_t *inst, const orientation_t *orient,
                                            float *a3, rface_t **outList ); // brush.cpp 0x47B380

// d_lightRegionHulls (dword_1807E00 / dword_25D5A4C) - the global hull sink the unported
// per-pixel light preview (LightPreview_DrawLight 0x406fb0) would consume.
void *d_lightRegionHulls[1024];
int   d_lightRegionHullCount = 0;

// sub_4A56B0 (0x4A56B0) — squared distance from a point to an AABB (0 when inside).
static float Region_DistSqFromBox( const float *p, const float *mins, const float *maxs )
{
    float r = 0.0f, d;
    d = maxs[0] - p[0];
    if ( d <= 0.0f ) { d = p[0] - mins[0]; if ( d > 0.0f ) r += d * d; } else r += d * d;
    d = maxs[1] - p[1];
    if ( d <= 0.0f ) { d = p[1] - mins[1]; if ( d > 0.0f ) r += d * d; } else r += d * d;
    d = maxs[2] - p[2];
    if ( d > 0.0f )  return r + d * d;
    d = p[2] - mins[2];
    if ( d > 0.0f )  return r + d * d;
    return r;
}

// sub_4852E0 (0x4852E0) — find the entity DEF whose "targetname" == name (top-level
// search; the prefab-scoped branch is unused for editor light region builds).
static entity_s_def *Region_FindTargetEntity( const char *name )
{
    entity_s *cur = entities.next;
    while ( cur != &entities )
    {
        if ( Entity_HasEpairMatch( cur, "targetname", name ) )
            return (entity_s_def *)cur;
        cur = cur->next;
    }
    return 0;
}

static float Region_CosSum( float a1, float a2 )
{
    float v = ( 1.0f - a2 * a2 ) * ( 1.0f - a1 * a1 );
    return (float)( a2 * a1 - sqrtf( v ) );
}

// 0x4063A0  Entity_Light — classify a light + derive its cone.  Returns 2 (cone) / 3.
static int Entity_Light( const float *worldPos, int defPtr, const orientation_t *orient,
                         float *outDir, float *outCosInner, float *outCosOuter, float *outCosHalfFov )
{
    if ( ( Entity_GetIntValueForKey( defPtr, "spawnflags" ) & 1 ) != 0 )
        return 3;

    char *ep = *(char **)( defPtr + 116 );        // entity_s.epairs @0x74
    const char *targetName = "";
    bool found = false;
    while ( ep )
    {
        const char *key = *(const char **)( (char *)ep + 4 );
        if ( !strcmp( key, "target" ) )
        {
            targetName = *(const char **)( (char *)ep + 8 );
            if ( !targetName )
                return 3;
            found = true;
            break;
        }
        ep = *(char **)ep;
    }
    (void)found;

    entity_s_def *tgt = Region_FindTargetEntity( targetName );
    if ( !tgt )
        return 3;

    float tWorld[3];
    OrientationPosToWorldPos( tWorld, tgt->origin, orient );
    float dir[3] = { worldPos[0] - tWorld[0], worldPos[1] - tWorld[1], worldPos[2] - tWorld[2] };
    float len = Vec3Normalize_R( dir );

    float cosOuter;
    float fovOuter = Entity_GetFloatValueForKey( defPtr, "fov_outer" );
    if ( fovOuter == 0.0f )
        cosOuter = len / sqrtf( len * len + 4096.0f );
    else
        cosOuter = (float)cos( DEG2RAD( fovOuter ) * 0.5f );

    float cosInner = (float)cos( DEG2RAD( Entity_GetFloatValueForKey( defPtr, "fov_inner" ) ) * 0.5f );
    if ( cosOuter >= cosInner )
        return 3;

    outDir[0] = dir[0]; outDir[1] = dir[1]; outDir[2] = dir[2];

    float maxturn = Entity_GetFloatValueForKey( defPtr, "maxturn" );
    if ( maxturn == 0.0f )
    {
        *outCosHalfFov = cosOuter;
    }
    else
    {
        float c = (float)cos( DEG2RAD( maxturn ) );
        *outCosHalfFov = ( -cosOuter <= c ) ? Region_CosSum( cosOuter, c ) : -1.0f;
    }
    *outCosInner = cosInner;
    *outCosOuter = cosOuter;
    return 2;
}

// 0x47D180  recursive shadow-caster gatherer.  Each record = { orientation_t(0x30);
// pad; selbrush_t*@+48 } = 52 bytes.
static int Region_GatherShadowBrushes( const float *light, float radSq, selbrush_t *listHead,
                                       const orientation_t *orient, char *out, int cap )
{
    int n = 0;
    for ( selbrush_t *b = listHead->next; b != listHead; b = b->next )
    {
        if ( FilterBrush( b, 0 ) )
            continue;
        entity_s *owner = b->owner;
        if ( owner->prefab )
        {
            orientation_t childOr;
            Entity_GetOrientation( (entity_s_def *)owner->def, (orientation_t *)orient, &childOr );
            // recurse into the prefab's active brush list.  The prefab sentinel head is
            // the prefab's embedded brush-list; reached by the binary as
            // &owner->prefab->active_brushlist (the v18->owner->prefab->active_brushlist).
            selbrush_t *pfHead = (selbrush_t *)( (char *)owner->prefab );
            n += Region_GatherShadowBrushes( light, radSq, pfHead, &childOr, out + 52 * n, cap - n );
            if ( n == cap )
                return n;
            continue;
        }
        if ( radSq < Region_DistSqFromBox( light, b->def->mins, b->def->maxs ) )
            continue;
        char *rec = out + 52 * n;
        memcpy( rec, orient, 0x30 );
        *(selbrush_t **)( rec + 48 ) = b;
        n++;
        if ( n == cap )
            return n;
    }
    return n;
}

// 0x47D2A0  LightPreview_GatherShadowBrushes — active + selected lists.
static int LightPreview_GatherShadowBrushes( char *out, const float *light, float radius )
{
    float radSq = radius * radius;
    int n = Region_GatherShadowBrushes( light, radSq, &active_brushes,
                                        (const orientation_t *)world_orient_matrix, out, 0x8000 );
    n += Region_GatherShadowBrushes( light, radSq, &selected_brushes,
                                     (const orientation_t *)world_orient_matrix, out + 52 * n, 0x8000 - n );
    return n;
}

// 0x406CE0  sub_406CE0 — submit the shadow-caster faces, add the light cube, run the
// merge driver, append the produced hulls to the global array.
static void Region_BuildForLight( int cls, const float *coneCenter, const float *coneDir,
                                  float radius, float cosHalfFov, int casterCount, char *recs )
{
    lightDesc_t desc;
    desc.cls  = cls;
    desc.p[0] = coneCenter[0]; desc.p[1] = coneCenter[1]; desc.p[2] = coneCenter[2];
    desc.p[3] = coneDir[0];    desc.p[4] = coneDir[1];    desc.p[5] = coneDir[2];
    desc.p[6] = radius;
    desc.p[7] = cosHalfFov;
    float *a3 = (float *)&desc;        // a3[1..3]=center, a3[7]=radius, a3[8]=cosHalfFov

    rface_t *faceList = 0;
    for ( int i = 0; i < casterCount; i++ )
    {
        char *rec = recs + 52 * i;
        Brush_DrawSubmitFaceWindings( *(selbrush_t **)( rec + 48 ),
                                      (const orientation_t *)rec, a3, &faceList );
    }
    sub_4DAD20( &faceList, a3 );        // light-cube faces (reads a3[+28] = radius)

    void *hulls[35];
    unsigned int count = (unsigned int)sub_4DD260( &faceList, &desc, hulls );

    while ( faceList )
    {
        rface_t *node = faceList;
        winding_t *w = *(winding_t **)( (char *)faceList + 0x24 );
        faceList = *(rface_t **)( (char *)faceList + 0x2C );
        if ( w ) { --g_windingAlloc; free( w ); }
        free( node );
    }

    if ( count > 8 )
        MessageBoxA( 0, va( "Cannot have more than %i regions for a light", 8 ),
                     "Too many regions", MB_ICONERROR );

    for ( unsigned int i = 0; i < count; i++ )
    {
        unsigned int idx = d_lightRegionHullCount;
        if ( idx < 0x400 ) { d_lightRegionHulls[idx] = hulls[i]; d_lightRegionHullCount = idx + 1; }
    }
}

// 0x406E00  sub_406E00 — for one light brush, build its region if radius>0 & spawnflags&3.
static void Region_ForOneLight( selbrush_t *inst, const orientation_t *orient )
{
    int defPtr = (int)(intptr_t)inst->owner->def;
    float radius = Entity_GetFloatValueForKey( defPtr, "radius" );
    if ( radius <= 0.0f )
        return;

    brush_t *def = inst->def;
    float boxMid[3], coneCenter[3];
    boxMid[0] = ( def->mins[0] + def->maxs[0] ) * 0.5f;
    boxMid[1] = ( def->mins[1] + def->maxs[1] ) * 0.5f;
    boxMid[2] = 0.5f * ( def->mins[2] + def->maxs[2] );
    OrientationPosToWorldPos( coneCenter, boxMid, orient );

    if ( ( Entity_GetIntValueForKey( defPtr, "spawnflags" ) & 3 ) == 0 )
        return;

    char *recs = (char *)operator new( 0x1A0000u );
    if ( !recs )
        return;
    int casterCount = LightPreview_GatherShadowBrushes( recs, coneCenter, radius );

    float dir[3], cosInner, cosOuter, cosHalfFov;
    int cls = Entity_Light( coneCenter, defPtr, orient, dir, &cosInner, &cosOuter, &cosHalfFov );
    Region_BuildForLight( cls, coneCenter, dir, radius, cosHalfFov, casterCount, recs );

    free( recs );
}

// 0x406200  CCamWnd_AddLightPreview - append { inst, arg2, orient } to the camera's
// light_preview_arr[] (LightPreviewRec, 56-byte stride); no-op if `inst` is already present.
// When the 8-slot ring is full it drops the OLDEST record first (FIFO shift, count=7).
int CCamWnd_AddLightPreview( CCamWnd *cam, selbrush_t *inst, int arg2, const orientation_t *orient )
{
    int count = cam->light_preview_count;
    // Already present? (linear search on inst)
    if ( count > 0 )
    {
        for ( int i = 0; i < count; ++i )
            if ( (selbrush_t *)(intptr_t)cam->light_preview_arr[i].inst == inst )
                return (int)(intptr_t)cam;   // already queued — no-op (binary returns `result`)
    }
    // FIFO evict the oldest when the 8-slot ring is full.
    if ( cam->light_preview_count == 8 )
    {
        cam->light_preview_count = 7;
        for ( int i = 0; i < cam->light_preview_count; ++i )
            cam->light_preview_arr[i] = cam->light_preview_arr[i + 1];
    }
    int slot = cam->light_preview_count;
    cam->light_preview_arr[slot].inst  = (int)(intptr_t)inst;
    cam->light_preview_arr[slot].arg2  = arg2;
    memcpy( &cam->light_preview_arr[slot].orient, orient, 0x30u );
    ++cam->light_preview_count;
    return (int)(intptr_t)cam;
}

// 0x4062d0  CCamWnd light-preview-record removal, called from Brush_Free when a brush instance
// dies: linear search by inst, then shift the tail down (56-byte LightPreviewRec stride) and
// decrement.  Returns the removed index, or light_preview_count when not present.
int CCamWnd_RemoveLightPreview( selbrush_t *removed, CCamWnd *cam )
{
    int count = cam->light_preview_count;
    int idx = 0;
    if ( count <= 0 )
        return 0;

    // find the record whose inst == removed
    while ( (selbrush_t *)(intptr_t)cam->light_preview_arr[idx].inst != removed )
    {
        if ( ++idx >= count )
            return idx;                       // not found
    }

    cam->light_preview_count = count - 1;
    // shift the tail down over the removed slot
    for ( int i = idx; i < count - 1; ++i )
        cam->light_preview_arr[i] = cam->light_preview_arr[i + 1];

    return idx;
}

// 0x406F10  Regions_ForSelected - clear the old hulls, then build regions for every selected
// light brush and every camera light-preview record.
void Regions_ForSelected( CCamWnd *cam )
{
    // clear the global region-hull array (sub_406C00)
    for ( int i = d_lightRegionHullCount; i > 0; )
        free( d_lightRegionHulls[--i] );
    d_lightRegionHullCount = 0;

    for ( selbrush_t *b = selected_brushes.next; b != &selected_brushes; b = b->next )
    {
        entity_s *owner = b->owner;
        eclass_t *eclass = ((entity_s_def *)owner->def)->eclass;
        if ( ( eclass->classtype & 1 ) != 0 )        // light eclass bit
            Region_ForOneLight( b, (const orientation_t *)world_orient_matrix );
    }
    for ( int i = 0; i < cam->light_preview_count; i++ )
    {
        CCamWnd::LightPreviewRec *r = &cam->light_preview_arr[i];
        // disasm gate: *(char*)(rec+52) >= 0 (the per-record flag byte).
        if ( *( (char *)r + 52 ) >= 0 )
            Region_ForOneLight( (selbrush_t *)(intptr_t)r->inst, &r->orient );
    }
}
