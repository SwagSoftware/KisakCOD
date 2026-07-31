#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\common\texturevecs.cpp — the texture-projection axis/matrix TU (its assert
// strings name this file).  Carved out of brush.cpp 2026-07-30: Ed_Normal_Calc
// (TextureAxisFromPlane, 0x459A60), Ed_texturevecs (0x459BA0), Face_MoveTexture
// (0x45A1C0, texdef -> 2x4 matrix), texturevecs_02 (0x459CC0, the inverse), plus the
// two com_math leaves they use (Ed_SinCos 0x4AABC0, sub_4AAD00 snap-rounder — their
// asserts keep the com_math.cpp provenance).  Ed_SinCos is the single canonical copy
// (select.cpp's SinCosDeg duplicate was consolidated here).

#include "stdafx.h"
#include <universal/assertive.h>    // iassert/vassert (USE_ASSERTS always on)
#include <cmath>

extern void Assert( const char *file, int line, int type, const char *fmt, ... );  // 0x49CEA0

// ── Ed_baseaxis (flt_6DE31C) — TextureAxisFromPlane table, verbatim from the IDB
//    (6 entries × { xv[3], yv[3], normal[3] }; Ed_Normal_Calc reads only xv/yv). ──
static const float Ed_baseaxis[6][9] =
{
    { 1, 0, 0,   0,-1, 0,   0, 0,-1 },
    { 1, 0, 0,   0,-1, 0,   1, 0, 0 },
    { 0, 1, 0,   0, 0,-1,  -1, 0, 0 },
    { 0, 1, 0,   0, 0,-1,   0, 1, 0 },
    { 1, 0, 0,   0, 0,-1,   0,-1, 0 },
    { 1, 0, 0,   0, 0,-1,   0, 0, 0 },
};

// 0x459A60  Normal_Calc — pick the dominant ±axis of `normal` (dot-test order
// +z,-z,+x,-x,+y,-y → index 0..5) and copy its base S/T vectors into xv/yv.
// strict >/< compares, tie keeps the lower index.
void Ed_Normal_Calc( const float *normal, float *xv, float *yv )
{
    int   v3  = 0;
    float v18 = 0.0f;
    float v12 = normal[2];   if ( v12 > 0.0f )  v18 = v12;
    float v13 = -normal[2];  if ( v18 < v13 ) { v18 = v13; v3 = 1; }
    float v14 = normal[0];   if ( v18 < v14 ) { v18 = v14; v3 = 2; }
    float v15 = -normal[0];  if ( v18 < v15 ) { v18 = v15; v3 = 3; }
    float v16 = normal[1];   if ( v18 < v16 ) { v18 = v16; v3 = 4; }
    float v17 = -normal[1];  if ( v18 < v17 ) {            v3 = 5; }
    xv[0] = Ed_baseaxis[v3][0]; xv[1] = Ed_baseaxis[v3][1]; xv[2] = Ed_baseaxis[v3][2];
    yv[0] = Ed_baseaxis[v3][3]; yv[1] = Ed_baseaxis[v3][4]; yv[2] = Ed_baseaxis[v3][5];
}

// 0x459BA0  texturevecs_xv_yv_s_t_normal — base S/T vectors + the s/t axis indices.
// Normal_Calc(normal,xv,yv) + s/t axis selection; carries the 5
// texturevecs.cpp:46-50 level-0 param-null asserts.
static void Ed_texturevecs( float *yv, float *xv, const float *normal, int *s, int *t )
{
    iassert( normal );   // texturevecs.cpp:46
    iassert( xv );       // texturevecs.cpp:47
    iassert( yv );       // texturevecs.cpp:48
    iassert( s );        // texturevecs.cpp:49
    iassert( t );        // texturevecs.cpp:50
    Ed_Normal_Calc( normal, xv, yv );
    if ( 0.0f == xv[0] ) *s = ( 0.0f == xv[1] ) ? 2 : 1; else *s = 0;
    if ( 0.0f == yv[0] ) *t = ( 0.0f == yv[1] ) ? 2 : 1; else *t = 0;
}

// 0x4AABC0  SinCos (com_math) — degrees → (*s, *c), exact at 0/90/180/270.
// radians/wrap narrow to FLOAT before fsincos (see inline); carries the 2
// com_math.cpp:3864-3865 param-null asserts.
void Ed_SinCos( float deg, float *s, float *c )
{
    // com_math.cpp:3864-3865 param-null asserts (L0) — RESTORED (the math-only pass dropped them).
    if ( !s ) Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp", 3864, 0, "%s", "s" );
    if ( !c ) Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp", 3865, 0, "%s", "c" );
    double v = deg;
    if ( deg < 0.0 ) v = (float)( v + 360.0 );   // IDA 0x4aac21: fstp/fld [deg] narrows the +360 wrap to float
    if      ( 0.0   == v ) { *c = 1.0f;  *s = 0.0f; }
    else if ( 90.0  == v ) { *c = 0.0f;  *s = 1.0f; }
    else if ( 180.0 == v ) { *c = -1.0f; *s = 0.0f; }
    else if ( 270.0 == v ) { *c = 0.0f;  *s = -1.0f; }
    // IDA 0x4aac9d: fstp/fld [deg] narrows radians to FLOAT before fsincos (deg is a 4-byte slot).
    else { float r = (float)( DEG2RAD( v ) ); *c = (float)cos( (double)r ); *s = (float)sin( (double)r ); }
}

// 0x4A47D0  Vec3Normalize (out-of-place) — normalize inVecs[0..2] into outS[0..2].
// lensq narrows to float; zero-guard (-len >= 0 -> 1.0).
void sub_4A47D0( int outS, int inVecs )
{
    float       *a1 = (float *)outS;
    const float *a2 = (const float *)inVecs;
    float len = (float)sqrt( (double)( a2[1]*a2[1] + a2[0]*a2[0] + a2[2]*a2[2] ) );
    if ( -len >= 0.0f ) len = 1.0f;             // length <= 0 ⇒ divide by 1
    float inv = 1.0f / len;
    a1[0] = a2[0] * inv;
    a1[1] = inv * a2[1];
    a1[2] = inv * a2[2];
}

// 0x45A1C0  Face_MoveTexture — build the 2×4 texture-projection matrix `outVecs`
// from a texdef (scale@surfDef[0,1], shift@uvBase[0,1], rotate, crossterm) and the
// face normal. surfDef/outVecs/uvBase are byte-pointers (IDA usercall convention,
// shared with Brush_ApplyTextureProjection's caller).
// full 2x4 matrix chain, pure x87; bit-exact rotation depends on Ed_SinCos.
// Carries the 7 texturevecs.cpp:157,160-166 level-1 output-contract asserts (s!=t +
// xv/yv unit-vector properties).
void Face_MoveTexture( int surfDef, const float *normal, int outVecs,
                       int uvBase, float rotate, float crossterm )
{
    const float *scale = (const float *)surfDef;   // [0]=sizeX [1]=sizeY
    const float *shift = (const float *)uvBase;     // [0]=shiftX [1]=shiftY
    float       *a3    = (float *)outVecs;          // out matrix [0..7]

    float sx = scale[0]; if ( 0.0f == sx ) sx = 128.0f;
    float sy = scale[1]; if ( 0.0f == sy ) sy = 128.0f;

    float pvecs[2][3];                             // the binary's array (assert strings)
    float *xv = pvecs[0], *yv = pvecs[1];
    int   s, t;
    Ed_texturevecs( yv, xv, normal, &s, &t );
    // texturevecs output-contract validation (common/texturevecs.cpp:157,160-166; ALL LEVEL 1) —
    // RESTORED (the prior port dropped these 7 asserts; the strings keep the original `pvecs` names).
    vassert( (s != t), "(s) = %i", s );   // texturevecs.cpp:157
    int r = 3 - s - t;
    vassert( (pvecs[0][s] == 1 || pvecs[0][s] == -1), "(pvecs[0][s]) = %g", pvecs[0][s] );   // :160
    vassert( (pvecs[0][t] == 0), "(pvecs[0][t]) = %g", pvecs[0][t] );   // :161
    vassert( (pvecs[0][r] == 0), "(pvecs[0][r]) = %g", pvecs[0][r] );   // :162
    vassert( (pvecs[1][s] == 0), "(pvecs[1][s]) = %g", pvecs[1][s] );   // :164
    vassert( (pvecs[1][t] == 1 || pvecs[1][t] == -1), "(pvecs[1][t]) = %g", pvecs[1][t] );   // :165
    vassert( (pvecs[1][r] == 0), "(pvecs[1][r]) = %g", pvecs[1][r] );   // :166

    float sinv, cosv;
    Ed_SinCos( rotate, &sinv, &cosv );

    float v10 = xv[s] / sx;
    a3[s] = cosv * v10;
    a3[t] = v10 * sinv;
    a3[r] = 0.0f;
    float v12 = yv[t] / sy;
    a3[s + 4] = -sinv * v12;
    a3[t + 4] = v12 * cosv;
    a3[r + 4] = 0.0f;
    a3[0] = a3[4] * crossterm + a3[0];
    a3[1] = a3[5] * crossterm + a3[1];
    a3[2] = crossterm * a3[6] + a3[2];
    a3[3] = -shift[0] / sx;
    a3[7] = -shift[1] / sy;
}

extern char *va( const char *fmt, ... );    // q_shared.cpp (0x4B9850) — sub_4AAD00 assert message

// ════════════════════════════════════════════════════════════════════════════
//  0x4AAD00  sub_4AAD00 (com_math.cpp:3929) — snap-to-granularity rounder.
//
//  Faithful port from DISASM (2026-06-27): cleans the low 12 mantissa bits of
//  `value`, then snaps to a `granularity` grid via the fistp-rounding idiom
//  (the +2^-30 epsilon is what makes the (int) cast ROUND rather than truncate —
//  it is LOAD-BEARING; do not drop it).  If the snapped value drifts from the
//  cleaned value by more than `epsilon`, the cleaned value is returned instead.
//  Used ONLY by texturevecs_02 (6 sites: size×2, shift×2, rotate, crossterm).
//  Asserts (L0): granularity>0, epsilon>0, epsilon<0.5/granularity.
// ════════════════════════════════════════════════════════════════════════════
static double sub_4AAD00( float value, float granularity, float epsilon )
{
    // KEEP_VERBOSE (×3): com_math.cpp:3929-3931 strings from a CoD3 engine fn kisak
    // doesn't carry; this file is their canonical port home.
    if ( granularity <= 0.0f )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp",
                3929, 0, "%s\n\t(granularity) = %g", "(granularity > 0)", granularity );
    if ( epsilon <= 0.0f )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp",
                3930, 0, "%s\n\t(epsilon) = %g", "(epsilon > 0)", epsilon );
    if ( 0.5 / granularity <= epsilon )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp",
                3931, 0, "%s\n\t%s", "epsilon < 0.5f / granularity",
                va( "%g, %g", epsilon, granularity ) );

    // clean the low 12 mantissa bits of `value` (carried in a float slot, bit-tweaked).
    float cleaned = value;
    unsigned int low12 = *(unsigned int *)&cleaned & 0xFFF;
    if ( (int)low12 > 4 )
    {
        if ( (int)( 4096 - low12 ) <= 4 )
            *(unsigned int *)&cleaned = ( 4096 - low12 ) + *(unsigned int *)&value;
    }
    else
    {
        *(unsigned int *)&cleaned = *(unsigned int *)&value - low12;
    }

    // snap to the granularity grid with the fistp-rounding epsilon (KEEP the +2^-30).
    double snapped = (double)(int)( (double)( granularity * cleaned ) + 9.313225746154785e-10 )
                     / granularity;
    double drift   = (double)snapped - (double)cleaned;
    if ( drift < 0.0 ) drift = -drift;     // fabs (IDA: fabs)
    if ( epsilon < drift )
        return cleaned;
    return snapped;
}

// ════════════════════════════════════════════════════════════════════════════
//  0x459CC0  texturevecs_02 (common/texturevecs.cpp) — the INVERSE of
//  Face_MoveTexture: decompose a world texture matrix `texMat` (8 floats =
//  two 4-vec rows) back into a texdef (size/shift/rotate/crossterm).  Consumed
//  by Brush_ApplyTextureProjection (0x476A30, Drag_FaceAlign).
//
//  __usercall in the binary:
//    texturevecs_02(_EDI=outSize[2], _ESI=texMat[8] (in+scratch), <st1 phantom>,
//                   a4=planeNormal[3], a5=planeDist, a6=outShift[2],
//                   a7=outRotate, a8=outCrossterm)
//  The `st1`/"st6_0" arg in hex-rays is an x87-stack artifact (it is the
//  planeDist already on the FPU stack from the row-orthogonalize phase) — it is
//  not a real parameter; the C signature drops it.
//
//  Ported from DISASM, x87 stack traced op-by-op (hex-rays renders this almost entirely
//  as raw __asm with _ET1/phantom stack slots).  Output-contract asserts
//  (texturevecs.cpp:85-97, all LEVEL 1) preserved verbatim.  FP constants from the IDB:
//    flt_6F40C4 = -1.0          (pvecs ==-1 assert)
//    dbl_6F40B8 =  0.0          (size[1] sign compare)
//    flt_6F4580 =  1e-5,  flt_6F4308 = 1000.0   (crossterm snap: gran=1000, eps=1e-5)
//    flt_6F44BC =  8.0,   flt_6F43E0 = 0.001    (size & shift snap: gran=8,  eps=0.001)
//    flt_6F4300 =  4.0,   flt_6F4640 = 0.005    (rotate snap:       gran=4,  eps=0.005)
//    dbl_6F4578 = 57.2957763671875              (radians -> degrees, 180/pi)
// ════════════════════════════════════════════════════════════════════════════
void texturevecs_02( int outSize, int texMatPtr, float /*st1_phantom*/,
                     int planeNormalPtr, float planeDist,
                     int outShift, int outRotate, int outCrossterm )
{
    float       *edi  = (float *)outSize;          // out size[2]
    float       *esi  = (float *)texMatPtr;        // texMat[0..7] (in/scratch)
    const float *pn   = (const float *)planeNormalPtr;  // a4 planeNormal

    float pvecs[2][3];                             // var_18/var_C (the binary's array)
    float *xv = pvecs[0], *yv = pvecs[1];
    int   s, t;                                    // var_20 = s, var_24 = t
    Ed_texturevecs( yv, xv, pn, &s, &t );

    // ── texturevecs output-contract validation (texturevecs.cpp:85-97, ALL LEVEL 1) ──
    vassert( (s != t), "(s) = %i", s );   // texturevecs.cpp:85
    int r = 3 - s - t;
    vassert( (pvecs[0][s] == 1 || pvecs[0][s] == -1), "(pvecs[0][s]) = %g", pvecs[0][s] );   // :88
    vassert( (pvecs[0][t] == 0), "(pvecs[0][t]) = %g", pvecs[0][t] );   // :89
    vassert( (pvecs[0][r] == 0), "(pvecs[0][r]) = %g", pvecs[0][r] );   // :90
    vassert( (pvecs[1][s] == 0), "(pvecs[1][s]) = %g", pvecs[1][s] );   // :92
    vassert( (pvecs[1][t] == 1 || pvecs[1][t] == -1), "(pvecs[1][t]) = %g", pvecs[1][t] );   // :93
    vassert( (pvecs[1][r] == 0), "(pvecs[1][r]) = %g", pvecs[1][r] );   // :94
    {
        const float *planeNormal = pn;              // the binary's param name
        iassert( planeNormal[r] != 0 );   // texturevecs.cpp:97
    }

    // ── Phase B: orthogonalize each texMat row against the r-axis using the plane.
    //    row0 (esi[0..3]) — only if texMat[r] != 0 (0x459EA7).
    if ( esi[r] != 0.0f )
    {
        float ratio = esi[r] / pn[r];
        esi[0] = esi[0] + (-ratio) * pn[0];
        esi[1] = esi[1] + (-ratio) * pn[1];
        esi[2] = esi[2] + (-ratio) * pn[2];
        esi[3] = esi[3] + ratio * planeDist;
    }
    //    row1 (esi[4..7]) — only if texMat[4+r] != 0 (0x459EFA).
    if ( esi[4 + r] != 0.0f )
    {
        float ratio = esi[4 + r] / pn[r];
        esi[4] = esi[4] + (-ratio) * pn[0];
        esi[5] = esi[5] + (-ratio) * pn[1];
        esi[6] = esi[6] + (-ratio) * pn[2];
        esi[7] = esi[7] + ratio * planeDist;
    }

    // ── Phase C: crossterm = dot(row0,row1) / |row1|^2, snapped (gran=1000, eps=1e-5).
    {
        float dot01    = esi[4] * esi[0] + esi[5] * esi[1] + esi[6] * esi[2];
        float lenSqR1  = esi[4] * esi[4] + esi[5] * esi[5] + esi[6] * esi[6];
        float crossRaw = dot01 / lenSqR1;
        *(float *)outCrossterm = crossRaw;     // raw store (IDA fst), overwritten by snap below
        float crossterm = (float)sub_4AAD00( crossRaw, 1000.0f, 1e-5f );
        *(float *)outCrossterm = crossterm;

        // ── Phase D: de-skew row0 by crossterm: row0 -= crossterm * row1 (m3 untouched).
        esi[0] = esi[0] + (-crossterm) * esi[4];
        esi[1] = esi[1] + (-crossterm) * esi[5];
        esi[2] = esi[2] + (-crossterm) * esi[6];
    }

    // ── Phase E: size[0] = 1 / |row0| (de-skewed).
    {
        float lenSqR0 = esi[0] * esi[0] + esi[1] * esi[1] + esi[2] * esi[2];
        float lenR0   = (float)sqrt( (double)lenSqR0 );      // _CIsqrt
        edi[0] = 1.0f / lenR0;
    }

    // ── Phase F: size[1] = 1 / |row1|.
    {
        float lenSqR1 = esi[4] * esi[4] + esi[5] * esi[5] + esi[6] * esi[6];
        float lenR1   = (float)sqrt( (double)lenSqR1 );      // _CIsqrt
        edi[1] = 1.0f / lenR1;
    }

    // ── Phase G: sign of size[1] — negate when the orientation cross-term is < 0.
    //    term = row1[t]*row0[s]*xv[s]*yv[t] - row1[s]*row0[t]*xv[s]*yv[t]; if (term<0) size[1] = -size[1].
    {
        float term1 = esi[4 + t] * esi[s] * xv[s] * yv[t];
        float term2 = esi[4 + s] * esi[t] * xv[s] * yv[t];
        float cross = term1 - term2;
        if ( cross < 0.0f )                                  // fcomp vs dbl_6F40B8(0.0); >=0 keeps sign
            edi[1] = -edi[1];
    }

    // ── Phase H: snap size[0], size[1] (gran=8, eps=0.001).
    edi[0] = (float)sub_4AAD00( edi[0], 8.0f, 0.001f );
    edi[1] = (float)sub_4AAD00( edi[1], 8.0f, 0.001f );

    // ── Phase I: rotate = atan2(row0[t]*xv[s], row0[s]*xv[s]) * (180/pi).
    {
        float y = esi[t] * xv[s];
        float x = esi[s] * xv[s];
        float angleDeg = (float)( RAD2DEG( atan2( (double)y, (double)x ) ) );
        *(float *)outRotate = angleDeg;        // raw store (IDA fst), overwritten by snap below
        // ── Phase J: snap rotate (gran=4, eps=0.005).
        *(float *)outRotate = (float)sub_4AAD00( angleDeg, 4.0f, 0.005f );
    }

    // ── Phase K: shift[0] = -row0[3]*size[0]; shift[1] = -row1[3]*size[1].
    {
        float *shift = (float *)outShift;
        shift[0] = -esi[3]  * edi[0];
        shift[1] = -esi[7]  * edi[1];
        // ── Phase L: snap shift (gran=8, eps=0.001).
        shift[0] = (float)sub_4AAD00( shift[0], 8.0f, 0.001f );
        shift[1] = (float)sub_4AAD00( shift[1], 8.0f, 0.001f );
    }
}

