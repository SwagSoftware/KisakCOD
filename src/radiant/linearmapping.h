#pragma once
#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// A double-precision 3x3 LU factor / back-substitute / iterative-refine solver
// specialized to 3x3 matrices, plus the affine texture-lock mapping setup.

// ── LinearMapping (IDB layout — 168 bytes used) ──────────────────────────────
// Built by LinearMapping_Setup; consumed by LinearMapping_Apply.
//   m_orig : the original 3x3 system (kept for iterative refinement)
//   m_lu   : a copy of m_orig that LinearMapping_LUFactor overwrites in place
//   indx   : the LUFactor row-permutation
//   axisI/axisJ/axisK : the (i,j,k) world-axis permutation chosen by Vec3_MajorAxis
//                       (k = major/plane normal axis; i,j span the projection plane).
struct LinearMapping
{
    double m_orig[3][3];   // 0x00  original matrix (refine residual)
    double m_lu[3][3];     // 0x48  LU-factored copy
    int    indx[3];        // 0x90  row permutation from LUFactor
    int    axisI;          // 0x9C
    int    axisJ;          // 0xA0
    int    axisK;          // 0xA4
};
static_assert(sizeof(LinearMapping) == 168, "LinearMapping");

// 0x4A45D0  com_math.cpp:690 — index (0/1/2) of the largest |component| of dir.
int  Vec3_MajorAxis( const float *dir );

// 0x4B6DC0  in-place LU factor of the 3x3 `a` (row-major, double[3][3]); `indx`
// receives the row permutation.  Returns false on a singular system.
bool LinearMapping_LUFactor( double a[3][3], int indx[3] );

// 0x4B7120  solve a*x = b in place (b is overwritten with the solution) using the
// LU factors `a` + permutation `indx`.
void LinearMapping_BackSub( const int indx[3], const double a[3][3], double b[3] );

// 0x4B72D0  one iterative-refinement pass: x -= A^-1*(m_orig*x - b_orig).
void LinearMapping_Refine( const double m_orig[3][3], const double a_lu[3][3],
                           double x[3], const int indx[3], double b_orig[3] );

// 0x4B7430  seed `lm` from the 3 plane points (p0,p1,p2) projected onto the major
// axis of `normal`, LU-factor it.  Returns false if the system is singular.
bool LinearMapping_Setup( LinearMapping *lm, const float *normal,
                          const float *p2, const float *p0, const float *p1 );

// 0x4B7340  solve the system for the scalar coords (c0,c1,c2) and scatter the
// recovered (s,t) into out[axisI]/out[axisJ], out[axisK]=0, out[3]=c2.
void LinearMapping_Apply( LinearMapping *lm, float *out,
                          float c0, float c1, float c2 );
