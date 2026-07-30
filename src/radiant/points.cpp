#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\radiant\points.cpp
// Pointfile = the .lin file the compiler writes for a leaked map: a list of 3D
// positions along the leak path, drawn as a line and walked with Next/Prev.
// 0x48ACB0 Pointfile_Check  0x48AE20 Pointfile_Draw  0x410600 Pointfile_Clear
// 0x48AA60 Pointfile_Next   0x48AB90 Pointfile_Prev
// s_pointFile base 0x23F1CE0 (stride 12, max 0x2000); s_num_points 0x23F1CD8;
// s_startpoint 0x23F18D4.  The IDB calls 0x1814CE8 g_qeglobals.d_pointfile_display_list
// but the assert string names it s_errLogCount and Pointfile_Clear uses it as the
// error-log entry COUNT — it is not a GL display list.

#include "stdafx.h"
#include "mainfrm.h"
#include "qe3.h"
#include <gfx_d3d/r_gfx.h>  // GfxPointVertex, GfxColor

// ── externs from other radiant TUs ───────────────────────────────────────────
extern void  Assert( const char *file, int line, int type, const char *fmt, ... );
extern int   Sys_Printf( const char *fmt, ... );
extern char  currentmap[];            // map.cpp 0x23F18D8
extern void  StripExtension( char *path );  // cmdlib.cpp 0x40AE90
extern CMainFrame *g_pParentWnd;     // engine_stubs.cpp

void Pointfile_Next();
void Pointfile_Prev();

// ── renderer / math deps ─────────────────────────────────────────────────────
struct orientation_t;                                    // gfx_d3d/fxprimitives.h (pointer use only)
extern int   g_nUpdateBits;                              // mainfrm.cpp 0x25D5A74
extern float Vec3Normalize_R( float *v );                // engine_stubs.cpp 0x40A5E0 (length; ignored here)
extern float world_orient_matrix[4][3];                  // entity.cpp 0x6DE290 (identity world orient)
extern char  Byte4PackPixelColor( float *from, GfxColor *out );                 // 0x402AC0
extern int   R_Add3DLine( GfxPointVertex *verts, const orientation_t *orient,
                          const float *p1, const float *p2, const unsigned int *color,
                          char width, int vertCount, int maxVertCount );        // 0x40C110
extern void  R_AddCmd_Line3D( short count, char width, GfxPointVertex *verts ); // 0x4FD1A0 (r_rendercmds)

// ── module globals (IDB addresses above) ─────────────────────────────────────
static float s_pointFile[0x2000][3];

static int s_num_points = 0;

static int s_startpoint = 0;

// Error-log entry count; errorfile.cpp shares it via extern.
int s_errLogCount = 0; // IDB 0x1814CE8

// 0x48ACB0  Pointfile_Check — read <currentmap>.lin into s_pointFile[].
// KISAK: the binary's last statement is `return (FILE*)fclose(f)`, i.e. it returns
// NULL on success AND on failure; this port returns the FILE* per the intent.  The
// return has no caller today; callers key off s_num_points.
FILE *Pointfile_Check()
{
    char      path[1024];
    FILE     *f;
    float     x, y, z;
    int       i;

    i = 0;
    do {
        path[i] = currentmap[i];
    } while ( currentmap[i++] );
    // IDA: StripExtension(0, path) — the unused first arg is a __usercall artifact.
    StripExtension( path );
    strcat( path, ".lin" );

    f = fopen( path, "r" );
    if ( !f )
        return NULL;

    Sys_Printf( "Reading pointfile %s\n", path );

    iassert(s_num_points == 0);

    while ( fscanf( f, "%f %f %f\n", &x, &y, &z ) == 3 )
    {
        if ( s_num_points < 0x2000 )
        {
            s_pointFile[s_num_points][0] = x;
            s_pointFile[s_num_points][1] = y;
            s_pointFile[s_num_points][2] = z;
            s_num_points++;
        }
    }

    s_startpoint = 0;
    fclose( f );

    return f;
}

// 0x410600  Pointfile_Clear — free every error-log entry's two heap strings and
// reset the count.  The binary walks from dword_180ACEC (= &s_errLog[0].matname)
// with a 40-byte stride, freeing *(p-1) (mapfile) and *p (matname).
// ── s_errLog entry (mirrors errorfile.cpp; 40 bytes) ─────────────────────────
struct s_errLogEntry_t
{
    char  *mapfile;      // +0   heap string
    char  *matname;      // +4   heap string
    int    enum1;        // +8
    int    enum2;        // +12
    float  origin[3];    // +16
    float  dir[3];       // +28
};
// IDB 0x180ACE8: s_errLog[N]; errorfile.cpp declares the same array extern.
extern s_errLogEntry_t s_errLog[];   // defined in errorfile.cpp

void Pointfile_Clear()
{
    size_t count = (size_t)(int)s_errLogCount;

    if ( (int)s_errLogCount > 0 )
    {
        for ( size_t i = 0; i < count; ++i )
        {
            free( s_errLog[i].mapfile );
            free( s_errLog[i].matname );
        }
    }

    s_errLogCount = 0;
}

// 0x423B40 CMainFrame::OnErrorFile and Map_NewMap/Map_SaveFile zero s_num_points.
void Pointfile_ResetPoints() { s_num_points = 0; }

// mainfrm.cpp's pointfile commands branch on whether a pointfile is loaded.
int Pointfile_GetNumPoints() { return s_num_points; }

// 0x48AE20  Pointfile_Draw — the leak path as connected 3D segments.
// Colour is flt_6DE130 = {1,0,0,1} (RED, read from .data); the batch is a 1362-vertex
// stack array, flushed once with R_AddCmd_Line3D(vertCount/2, ...) — a LINE count.
void Pointfile_Draw()
{
    static const float s_pointfileColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };   // IDB flt_6DE130
    GfxColor packed;
    Byte4PackPixelColor( const_cast<float *>( s_pointfileColor ), &packed );

    if ( s_num_points > 1 )
    {
        GfxPointVertex verts[1362];          // IDB v4[1362] — on-stack batch
        int vc = 0;
        for ( int i = 1; i < s_num_points; ++i )
            vc = R_Add3DLine( verts, (const orientation_t *)world_orient_matrix,
                              &s_pointFile[i - 1][0], &s_pointFile[i][0],
                              (const unsigned int *)&packed, 4, vc, 1362 );
        if ( vc )
            R_AddCmd_Line3D( (short)( vc / 2 ), 4, verts );
    }
}

// 0x48AA60  Pointfile_Next — step forward one leak-path node: snap camera + XY view
// to point[startpoint+1] and aim the camera at point[startpoint+2].
// The hex-rays atan2(a1,v12) / _CIasin(v12) args are x87-return artifacts; the disasm
// FPU operands are the NORMALIZED dir (yaw = atan2(n[1],n[0]), pitch = asin(n[2])).
// Degrees use the binary's EXACT constants *180.0 / 3.14159 (dbl_6F42A8 / dbl_6F4428 —
// a TRUNCATED pi, not M_PI).  cam.origin @+0x64, angles @+0x70 pitch / +0x74 yaw.
void Pointfile_Next()
{
    if ( s_startpoint < s_num_points - 2 )
    {
        CCamWnd *cam = g_pParentWnd->m_pCamWnd;
        CXYWnd  *xy  = g_pParentWnd->m_pXYWnd;
        int      idx = s_startpoint + 1;          // IDB 6*(v1+1) then *2 = point index v1+1
        ++s_startpoint;

        cam->camera.origin[0] = s_pointFile[idx][0];
        cam->camera.origin[1] = s_pointFile[idx][1];
        cam->camera.origin[2] = s_pointFile[idx][2];
        xy->m_vOrigin[0] = s_pointFile[idx][0];
        xy->m_vOrigin[1] = s_pointFile[idx][1];
        xy->m_vOrigin[2] = s_pointFile[idx][2];

        float normal[3];
        normal[0] = s_pointFile[idx + 1][0] - cam->camera.origin[0];
        normal[1] = s_pointFile[idx + 1][1] - cam->camera.origin[1];
        normal[2] = s_pointFile[idx + 1][2] - cam->camera.origin[2];
        Vec3Normalize_R( normal );

        cam->camera.angles[1] = (float)( atan2( (double)normal[1], (double)normal[0] ) * 180.0 / 3.14159 );
        g_nUpdateBits = -1;
        cam->camera.angles[0] = (float)( asin( (double)normal[2] ) * 180.0 / 3.14159 );
    }
    else
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0, (LPARAM)"End of pointfile" );
    }
}

// 0x48AB90  Pointfile_Prev — symmetric to Pointfile_Next (same FPU artifacts, same
// exact degree constants); the guard is `if (s_startpoint)` and the aim target is
// point[startpoint], i.e. still the path's forward direction.
void Pointfile_Prev()
{
    if ( s_startpoint )
    {
        CCamWnd *cam = g_pParentWnd->m_pCamWnd;
        CXYWnd  *xy  = g_pParentWnd->m_pXYWnd;
        int      idx = s_startpoint - 1;          // IDB 6*(v1-1) then *2 = point index v1-1
        --s_startpoint;

        cam->camera.origin[0] = s_pointFile[idx][0];
        cam->camera.origin[1] = s_pointFile[idx][1];
        cam->camera.origin[2] = s_pointFile[idx][2];
        xy->m_vOrigin[0] = s_pointFile[idx][0];
        xy->m_vOrigin[1] = s_pointFile[idx][1];
        xy->m_vOrigin[2] = s_pointFile[idx][2];

        float normal[3];
        normal[0] = s_pointFile[idx + 1][0] - cam->camera.origin[0];
        normal[1] = s_pointFile[idx + 1][1] - cam->camera.origin[1];
        normal[2] = s_pointFile[idx + 1][2] - cam->camera.origin[2];
        Vec3Normalize_R( normal );

        cam->camera.angles[1] = (float)( atan2( (double)normal[1], (double)normal[0] ) * 180.0 / 3.14159 );
        g_nUpdateBits = -1;
        cam->camera.angles[0] = (float)( asin( (double)normal[2] ) * 180.0 / 3.14159 );
    }
    else
    {
        SendMessageA( g_qeglobals.d_hwndStatus, WM_USER | WM_CREATE, 0, (LPARAM)"Start of pointfile" );
    }
}
