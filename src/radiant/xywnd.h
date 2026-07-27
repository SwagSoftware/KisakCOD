#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Editor XY-view math: free functions taking an explicit view-state struct, which the MFC
// CXYWnd fills from its members (m_nViewType / m_fScale / m_vOrigin / m_nWidth / m_nHeight).
// The originals are CXYWnd methods in the CoD4Radiant binary (IW3xRadiant.i64).

struct GfxMatrix;

// CoD4Radiant view-type values, recovered from CXYWnd::XY_DrawGrid's label logic
// (@0x4686a0: m_nViewType==2 -> "XY Top", ==1 -> "XZ Front", else "YZ Side").
enum { ED_VIEW_YZ = 0, ED_VIEW_XZ = 1, ED_VIEW_XY = 2 };

struct XYViewState
{
    int   viewType;      // m_nViewType (ED_VIEW_*)
    float scale;         // m_fScale (pixels per world unit)
    float origin[3];     // m_vOrigin (world-space view centre)
    int   width;         // m_nWidth  (client width,  px)
    int   height;        // m_nHeight (client height, px)
    bool  active;        // m_bActive (brighter view-name label when focused)
};

void XY_SetupProjectionMtx(GfxMatrix *mtx, float width, float height, float depth); // IDB 0x4a7980
bool XY_SetupScene(const XYViewState *v);                                           // IDB 0x5064c0 (false = degenerate-projection frame dropped)
void XY_DrawGrid(const XYViewState *v);                                             // IDB 0x4686a0 (grid lines only)
void XY_DrawBlockGrid(const XYViewState *v);                                        // IDB 0x4690f0 (1024-unit block grid + labels)
void XY_DrawBrushes(const XYViewState *v);                                          // IDB 0x46CE20 (brush + overlay body)
