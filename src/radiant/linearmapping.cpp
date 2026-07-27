#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// Double-precision 3x3 LU factor / back-substitute / iterative-refine solver
// specialized to 3x3 matrices for texture/lightmap-lock reprojection.

#include "stdafx.h"
#include "linearmapping.h"
#include <string.h>   // memcpy

// The binary uses the verbose Radiant Assert(file,line,0,"%s",cond) form for these
// (NaN output-validation + the null-dir guard); kept byte-faithful.
extern void Assert( const char *file, int line, int type, const char *fmt, ... );

// ─────────────────────────────────────────────────────────────────────────────
//  0x4A45D0  Vec3_MajorAxis (com_math.cpp:690) — index of the component of `dir`
//  with the largest squared magnitude.  Kept radiant-local (com_math.cpp is shared
//  with SP/MP/dedi; this symbol does not exist there and is only used here).
//
//  IDA: squares each component, then  result = 2;
//       if ( v[ v0<v1 ] >= v2 )  return (v0 < v1);   else  return 2;
//  i.e. picks max(v0,v1) via the (v0<v1) index, compares that to v2.
// ─────────────────────────────────────────────────────────────────────────────
int Vec3_MajorAxis( const float *dir )
{
    if ( !dir )
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\com_math.cpp",
                690, 0, "%s", "dir" );

    float v[2];
    v[0] = dir[0] * dir[0];
    v[1] = dir[1] * dir[1];
    float v2 = dir[2] * dir[2];

    int hi01 = ( v[0] < (double)v[1] ) ? 1 : 0;   // index of max(v0,v1)
    if ( v[hi01] >= (double)v2 )
        return hi01;
    return 2;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B6DC0  LinearMapping_LUFactor — Numerical-Recipes ludcmp (scaled partial
//  pivoting) over the 3x3 `a` (row-major).  `indx` receives the row permutation.
//  The parity sign `d` is computed but discarded (the binary keeps no *d arg).
//  Returns false if a row is all-zero (singular) or a pivot is zero, OR if any of
//  the three resulting diagonal elements a[0][0]/a[1][1]/a[2][2] is exactly zero.
//
//  disasm @0x4b6dc0: identical to the canonical n=3 ludcmp.  vv[i]=1/max|row i|
//  (returns false if max==0); the j-column loop with the i<j upper / i>=j lower
//  split + dum=vv[i]*|sum| pivot search; row-swap of a[imax] <-> a[j]; pivot==0
//  test; divide the sub-diagonal column by the pivot.
// ─────────────────────────────────────────────────────────────────────────────
bool LinearMapping_LUFactor( double a[3][3], int indx[3] )
{
    const int n = 3;
    double vv[3];

    // implicit row scaling
    for ( int i = 0; i < n; ++i )
    {
        double big = 0.0;
        for ( int j = 0; j < n; ++j )
        {
            double tmp = a[i][j];
            if ( tmp < 0.0 ) tmp = -tmp;          // fabs
            if ( tmp > big ) big = tmp;
        }
        if ( big == 0.0 )
            return false;                          // singular row
        vv[i] = 1.0 / big;
    }

    int imax = 0;   // IDA: var_20 init to 0 before the j-loop; carries across j.
    for ( int j = 0; j < n; ++j )
    {
        // upper triangle: i < j
        for ( int i = 0; i < j; ++i )
        {
            double sum = a[i][j];
            for ( int k = 0; k < i; ++k )
                sum -= a[i][k] * a[k][j];
            a[i][j] = sum;
        }

        // diagonal + lower triangle: i >= j, with pivot search
        double big = 0.0;   // IDA resets big each j-iteration (imax does NOT reset)
        for ( int i = j; i < n; ++i )
        {
            double sum = a[i][j];
            for ( int k = 0; k < j; ++k )
                sum -= a[i][k] * a[k][j];
            a[i][j] = sum;

            // IDA pivot test is STRICT '>' (fcom; test ah,41h; jnz-skip ⇒ update
            // only when dum > big), NOT the canonical NR '>='.  imax carries over
            // across j-iterations (the binary resets big to 0 each j but NOT imax);
            // i=j always sets it whenever dum>0 (sum!=0), so a degenerate dum==0 at
            // i=j keeps the prior imax — replicate exactly.
            double dum = vv[i] * ( sum < 0.0 ? -sum : sum );
            if ( dum > big )
            {
                big  = dum;
                imax = i;
            }
        }

        if ( a[imax][j] == 0.0 )
            return false;                          // singular pivot

        if ( j != imax )
        {
            // swap rows imax and j
            for ( int k = 0; k < n; ++k )
            {
                double dum = a[imax][k];
                a[imax][k] = a[j][k];
                a[j][k]    = dum;
            }
            vv[imax] = vv[j];
        }
        indx[j] = imax;

        // divide the column below the pivot
        if ( j != n - 1 )
        {
            double dum = 1.0 / a[j][j];
            for ( int i = j + 1; i < n; ++i )
                a[i][j] *= dum;
        }
    }

    // IDA tail: success iff every diagonal element is non-zero.
    return a[0][0] != 0.0 && a[1][1] != 0.0 && a[2][2] != 0.0;
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B7120  LinearMapping_BackSub — Numerical-Recipes lubksb.  Solves a*x = b in
//  place: forward substitution (with the ludcmp permutation `indx`) then back
//  substitution.  `b` is overwritten with the solution.  disasm @0x4b7120: the
//  classic two-loop lubksb (ii-tracking forward pass, /a[i][i] back pass).
// ─────────────────────────────────────────────────────────────────────────────
void LinearMapping_BackSub( const int indx[3], const double a[3][3], double b[3] )
{
    const int n = 3;
    int ii = -1;

    for ( int i = 0; i < n; ++i )
    {
        int    ip  = indx[i];
        double sum = b[ip];
        b[ip] = b[i];
        if ( ii >= 0 )
        {
            for ( int j = ii; j <= i - 1; ++j )
                sum -= a[i][j] * b[j];
        }
        else if ( sum != 0.0 )
        {
            ii = i;
        }
        b[i] = sum;
    }

    for ( int i = n - 1; i >= 0; --i )
    {
        double sum = b[i];
        for ( int j = i + 1; j < n; ++j )
            sum -= a[i][j] * b[j];
        b[i] = sum / a[i][i];
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B72D0  LinearMapping_Refine — Numerical-Recipes mprove (one pass).  Computes
//  the residual r = m_orig*x - b_orig (note the SIGN: the binary computes
//  m_orig*x - b_orig, not b_orig - m_orig*x), solves m_lu*dx = r via BackSub, and
//  corrects x -= dx.  disasm @0x4b72d0 (verbatim): r[k] = m_orig[k][0]*x[0]
//  - b_orig[k] + m_orig[k][1]*x[1] + m_orig[k][2]*x[2]; BackSub(indx, m_lu, r);
//  x[0..2] -= r[0..2].
// ─────────────────────────────────────────────────────────────────────────────
void LinearMapping_Refine( const double m_orig[3][3], const double a_lu[3][3],
                           double x[3], const int indx[3], double b_orig[3] )
{
    double r[3];
    for ( int k = 0; k < 3; ++k )
        r[k] = m_orig[k][0] * x[0] - b_orig[k]
             + m_orig[k][1] * x[1]
             + m_orig[k][2] * x[2];

    LinearMapping_BackSub( indx, a_lu, r );

    x[0] -= r[0];
    x[1] -= r[1];
    x[2] -= r[2];
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B7430  LinearMapping_Setup — seed the 3x3 system from the 3 plane points
//  projected onto the (i,j) plane perpendicular to the normal's major axis (k),
//  with a homogeneous "1" column, then LU-factor it.
//
//  IDA arg map (usercall): a1@eax=lm, a2@edx=normal, a3@ecx=p2, a4=p0, a5=p1.
//  axisK = Vec3_MajorAxis(normal); axisI = ((axisK & 1) == 0);
//  axisJ = (~axisK & 2).  (k=0 -> i=1,j=2 ; k=1 -> i=0,j=2 ; k=2 -> i=1,j=0.)
//  M row r = ( pr[axisI], pr[axisJ], 1 ).  m_lu = copy of M; LUFactor(m_lu, indx).
// ─────────────────────────────────────────────────────────────────────────────
bool LinearMapping_Setup( LinearMapping *lm, const float *normal,
                          const float *p2, const float *p0, const float *p1 )
{
    int axisK = Vec3_MajorAxis( normal );
    lm->axisK = axisK;
    int axisI = ( ( axisK & 1 ) == 0 ) ? 1 : 0;   // (BOOL)((v7&1)==0)
    int axisJ = ( ~axisK ) & 2;

    lm->m_orig[0][0] = p0[axisI];
    lm->axisI = axisI;
    lm->m_orig[0][1] = p0[axisJ];
    lm->axisJ = axisJ;
    lm->m_orig[0][2] = 1.0;

    lm->m_orig[1][0] = p1[axisI];
    lm->m_orig[1][1] = p1[axisJ];
    lm->m_orig[1][2] = 1.0;

    lm->m_orig[2][0] = p2[axisI];
    lm->m_orig[2][1] = p2[axisJ];
    lm->m_orig[2][2] = 1.0;

    memcpy( lm->m_lu, lm->m_orig, sizeof( lm->m_orig ) );   // qmemcpy 0x48
    return LinearMapping_LUFactor( lm->m_lu, lm->indx );
}

// ─────────────────────────────────────────────────────────────────────────────
//  0x4B7340  LinearMapping_Apply — solve the system for the per-point scalar coord
//  triple (c0,c1,c2) and scatter the recovered values into a 4-float texvec.
//
//  IDA arg map (usercall): a1@eax=indx, a2@ebx=m_lu, a3@edi=out, a4=lm(m_orig),
//  a5..a7=c0,c1,c2, a8..a10=axisI,axisJ,axisK.
//  b = {c0,c1,c2}; bsave = b (for refine).  BackSub(indx, m_lu, b);
//  Refine(m_orig, m_lu, b, indx, bsave).
//  out[axisI]=b[0]; out[axisJ]=b[1]; out[axisK]=0; out[3]=b[2].  Asserts no NaNs.
//
//  out[3] is the SOLVED 3rd component (the affine constant), NOT the original c2 —
//  the IDA `v14`/`a3[3]=v15` reads the same stack slot that BackSub+Refine overwrote
//  with sol[2] (disasm: fld [var_C] AFTER the solve; var_C is b[2]'s slot).  The
//  hex-rays variable name `v14`(=a7 at entry) is the misleading register-reuse alias.
//
//  We pass the LinearMapping* directly (it holds indx / m_lu / m_orig / axes).
// ─────────────────────────────────────────────────────────────────────────────
void LinearMapping_Apply( LinearMapping *lm, float *out,
                          float c0, float c1, float c2 )
{
    double b[3]     = { c0, c1, c2 };
    double bsave[3] = { c0, c1, c2 };

    LinearMapping_BackSub( lm->indx, lm->m_lu, b );
    LinearMapping_Refine( lm->m_orig, lm->m_lu, b, lm->indx, bsave );

    out[lm->axisI] = (float)b[0];
    out[lm->axisJ] = (float)b[1];
    out[lm->axisK] = 0.0f;
    out[3]         = (float)b[2];   // IDA: out[3] = solved sol[2] (the affine constant)

    if ( ( *(unsigned int *)&out[0] & 0x7F800000 ) == 0x7F800000
      || ( *(unsigned int *)&out[1] & 0x7F800000 ) == 0x7F800000
      || ( *(unsigned int *)&out[2] & 0x7F800000 ) == 0x7F800000
      || ( *(unsigned int *)&out[3] & 0x7F800000 ) == 0x7F800000 )
    {
        Assert( "C:\\trees\\cod3-pc\\cod3-modtools\\cod3src\\src\\universal\\linearmapping.cpp",
                159, 0, "%s",
                "!IS_NAN((vec)[0]) && !IS_NAN((vec)[1]) && !IS_NAN((vec)[2]) && !IS_NAN((vec)[3])" );
    }
}
