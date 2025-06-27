/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001-2003 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#include "objects.h"
#include "joint.h"
#include <ode/config.h>
#include <ode/odemath.h>
#include <ode/rotation.h>
#include <ode/timer.h>
#include <ode/error.h>
#include <ode/matrix.h>
#include <ode/misc.h>
// #include "lcp.h"
#include "util.h"
#include <universal/profile.h>
#include <universal/com_math.h>
#include <qcommon/qcommon.h>

#define ALLOCA dALLOCA16

typedef const dReal *dRealPtr;
typedef dReal *dRealMutablePtr;
#define dRealArray(name,n) dReal name[n];
#define dRealAllocaArray(name,n) dReal *name = (dReal*) ALLOCA ((n)*sizeof(dReal));

//***************************************************************************
// configuration

// for the SOR and CG methods:
// uncomment the following line to use warm starting. this definitely
// help for motor-driven joints. unfortunately it appears to hurt
// with high-friction contacts using the SOR method. use with care

// #define WARM_STARTING 1


// for the SOR method:
// uncomment the following line to determine a new constraint-solving
// order for each iteration. however, the qsort per iteration is expensive,
// and the optimal order is somewhat problem dependent. 
// @@@ try the leaf->root ordering.

//#define REORDER_CONSTRAINTS 1


// for the SOR method:
// uncomment the following line to randomly reorder constraint rows
// during the solution. depending on the situation, this can help a lot
// or hardly at all, but it doesn't seem to hurt.

#define RANDOMLY_REORDER_CONSTRAINTS 1

//***************************************************************************
// testing stuff

#ifdef TIMING
#define IFTIMING(x) x
#else
#define IFTIMING(x) /* */
#endif

//***************************************************************************
// various common computations involving the matrix J

// compute iMJ = inv(M)*J'

static void compute_invM_JT (int constraintRowCount,
	unsigned int bodyCount,
	ConstraintRowData *rows,
	dxBody *const *body,
	const float *invI)
{
	unsigned int body1; // [esp+Ch] [ebp-10h]
	int body2; // [esp+10h] [ebp-Ch]
	int rowIndex; // [esp+14h] [ebp-8h]
	ConstraintRowData *row_ptr; // [esp+18h] [ebp-4h]

	iassert(rows);
	iassert(body);
	iassert(invI);

	row_ptr = rows;
	for (rowIndex = 0; rowIndex < constraintRowCount; ++rowIndex)
	{
		body1 = row_ptr->body1;
		body2 = row_ptr->body2;
		bcassert(body1, bodyCount);

		Vec3Scale(row_ptr->J_body1Linear, body[body1]->invMass, row_ptr->iMJ_body1Linear);
		dMULTIPLY0_331(row_ptr->iMJ_body1Angular, &invI[12 * body1], row_ptr->J_body1Angular);
		if (body2 >= 0)
		{
			bcassert(body2, bodyCount);
			Vec3Scale(row_ptr->J_body2Linear, body[body2]->invMass, row_ptr->iMJ_body2Linear);
			dMULTIPLY0_331(row_ptr->iMJ_body2Angular, &invI[12 * body2], row_ptr->J_body2Angular);
		}
		++row_ptr;
	}
}


// compute out = inv(M)*J'*in.

//static void multiply_invM_JT (int m, int nb, dRealMutablePtr iMJ, int *jb,
//	dRealMutablePtr in, dRealMutablePtr out)
//{
//	int i,j;
//	dSetZero (out,6*nb);
//	dRealPtr iMJ_ptr = iMJ;
//	for (i=0; i<m; i++) {
//		int b1 = jb[i*2];	
//		int b2 = jb[i*2+1];
//		dRealMutablePtr out_ptr = out + b1*6;
//		for (j=0; j<6; j++) out_ptr[j] += iMJ_ptr[j] * in[i];
//		iMJ_ptr += 6;
//		if (b2 >= 0) {
//			out_ptr = out + b2*6;
//			for (j=0; j<6; j++) out_ptr[j] += iMJ_ptr[j] * in[i];
//		}
//		iMJ_ptr += 6;
//	}
//}


// compute out = J*in.

//static void multiply_J (int m, dRealMutablePtr J, int *jb,
//	dRealMutablePtr in, dRealMutablePtr out)
//{
//	int i,j;
//	dRealPtr J_ptr = J;
//	for (i=0; i<m; i++) {
//		int b1 = jb[i*2];	
//		int b2 = jb[i*2+1];
//		dReal sum = 0;
//		dRealMutablePtr in_ptr = in + b1*6;
//		for (j=0; j<6; j++) sum += J_ptr[j] * in_ptr[j];				
//		J_ptr += 6;
//		if (b2 >= 0) {
//			in_ptr = in + b2*6;
//			for (j=0; j<6; j++) sum += J_ptr[j] * in_ptr[j];				
//		}
//		J_ptr += 6;
//		out[i] = sum;
//	}
//}

static void __cdecl multiply_J(int constraintRowCount, ConstraintRowData *rows, float *in)
{
	float suma; // [esp+0h] [ebp-18h]
	float sum; // [esp+0h] [ebp-18h]
	float sumb; // [esp+0h] [ebp-18h]
	int i; // [esp+8h] [ebp-10h]
	int body2; // [esp+Ch] [ebp-Ch]
	float *in_ptr; // [esp+10h] [ebp-8h]

	for (i = 0; i < constraintRowCount; ++i)
	{
		body2 = rows->body2;
		in_ptr = &in[6 * rows->body1];
		suma = Vec3Dot(rows->J_body1Linear, in_ptr) + 0.0;
		sum = Vec3Dot(rows->J_body1Angular, in_ptr + 3) + suma;
		if (body2 >= 0)
		{
			sumb = Vec3Dot(rows->J_body2Linear, &in[6 * body2]) + sum;
			sum = Vec3Dot(rows->J_body2Angular, &in[6 * body2 + 3]) + sumb;
		}
		iassert(isfinite(sum));
		rows->rhs = sum;
		++rows;
	}
}


// compute out = (J*inv(M)*J' + cfm)*in.
// use z as an nb*6 temporary.

//static void multiply_J_invM_JT (int m, int nb, dRealMutablePtr J, dRealMutablePtr iMJ, int *jb,
//	dRealPtr cfm, dRealMutablePtr z, dRealMutablePtr in, dRealMutablePtr out)
//{
//	multiply_invM_JT (m,nb,iMJ,jb,in,z);
//	multiply_J (m,J,jb,z,out);
//
//	// add cfm
//	for (int i=0; i<m; i++) out[i] += cfm[i] * in[i];
//}

//***************************************************************************
// conjugate gradient method with jacobi preconditioner
// THIS IS EXPERIMENTAL CODE that doesn't work too well, so it is ifdefed out.
//
// adding CFM seems to be critically important to this method.

#if 0

static inline dReal dot (int n, dRealPtr x, dRealPtr y)
{
	dReal sum=0;
	for (int i=0; i<n; i++) sum += x[i]*y[i];
	return sum;
}


// x = y + z*alpha

static inline void add (int n, dRealMutablePtr x, dRealPtr y, dRealPtr z, dReal alpha)
{
	for (int i=0; i<n; i++) x[i] = y[i] + z[i]*alpha;
}


static void CG_LCP (int m, int nb, dRealMutablePtr J, int *jb, dxBody * const *body,
	dRealPtr invI, dRealMutablePtr lambda, dRealMutablePtr fc, dRealMutablePtr b,
	dRealMutablePtr lo, dRealMutablePtr hi, dRealPtr cfm, int *findex,
	dxQuickStepParameters *qs)
{
	int i,j;
	const int num_iterations = qs->num_iterations;

	// precompute iMJ = inv(M)*J'
	dRealAllocaArray (iMJ,m*12);
	compute_invM_JT (m,J,iMJ,jb,body,invI);

	dReal last_rho = 0;
	dRealAllocaArray (r,m);
	dRealAllocaArray (z,m);
	dRealAllocaArray (p,m);
	dRealAllocaArray (q,m);

	// precompute 1 / diagonals of A
	dRealAllocaArray (Ad,m);
	dRealPtr iMJ_ptr = iMJ;
	dRealPtr J_ptr = J;
	for (i=0; i<m; i++) {
		dReal sum = 0;
		for (j=0; j<6; j++) sum += iMJ_ptr[j] * J_ptr[j];
		if (jb[i*2+1] >= 0) {
			for (j=6; j<12; j++) sum += iMJ_ptr[j] * J_ptr[j];
		}
		iMJ_ptr += 12;
		J_ptr += 12;
		Ad[i] = REAL(1.0) / (sum + cfm[i]);
	}

#ifdef WARM_STARTING
	// compute residual r = b - A*lambda
	multiply_J_invM_JT (m,nb,J,iMJ,jb,cfm,fc,lambda,r);
	for (i=0; i<m; i++) r[i] = b[i] - r[i];
#else
	dSetZero (lambda,m);
	memcpy (r,b,m*sizeof(dReal));		// residual r = b - A*lambda
#endif
	
	for (int iteration=0; iteration < num_iterations; iteration++) {
		for (i=0; i<m; i++) z[i] = r[i]*Ad[i];	// z = inv(M)*r
		dReal rho = dot (m,r,z);		// rho = r'*z
		
		// @@@
		// we must check for convergence, otherwise rho will go to 0 if
		// we get an exact solution, which will introduce NaNs into the equations.
		if (rho < 1e-10) {
			printf ("CG returned at iteration %d\n",iteration);
			break;
		}
		
		if (iteration==0) {
			memcpy (p,z,m*sizeof(dReal));	// p = z
		}
		else {
			add (m,p,z,p,rho/last_rho);	// p = z + (rho/last_rho)*p
		}
		
		// compute q = (J*inv(M)*J')*p
		multiply_J_invM_JT (m,nb,J,iMJ,jb,cfm,fc,p,q);
	
		dReal alpha = rho/dot (m,p,q);		// alpha = rho/(p'*q)
		add (m,lambda,lambda,p,alpha);		// lambda = lambda + alpha*p
		add (m,r,r,q,-alpha);			// r = r - alpha*q
		last_rho = rho;
	}

	// compute fc = inv(M)*J'*lambda
	multiply_invM_JT (m,nb,iMJ,jb,lambda,fc);

#if 0
	// measure solution error
	multiply_J_invM_JT (m,nb,J,iMJ,jb,cfm,fc,lambda,r);
	dReal error = 0;
	for (i=0; i<m; i++) error += dFabs(r[i] - b[i]);
	printf ("lambda error = %10.6e\n",error);
#endif
}

#endif

//***************************************************************************
// SOR-LCP method

// nb is the number of bodies in the body array.
// J is an m*12 matrix of constraint rows
// jb is an array of first and second body numbers for each constraint row
// invI is the global frame inverse inertia for each body (stacked 3x3 matrices)
//
// this returns lambda and fc (the constraint force).
// note: fc is returned as inv(M)*J'*lambda, the constraint force is actually J'*lambda
//
// b, lo and hi are modified on exit


struct IndexError {
	dReal error;		// error to sort on
	int findex;
	int index;		// row index
};


#ifdef REORDER_CONSTRAINTS

static int compare_index_error (const void *a, const void *b)
{
	const IndexError *i1 = (IndexError*) a;
	const IndexError *i2 = (IndexError*) b;
	if (i1->findex < 0 && i2->findex >= 0) return -1;
	if (i1->findex >= 0 && i2->findex < 0) return 1;
	if (i1->error < i2->error) return -1;
	if (i1->error > i2->error) return 1;
	return 0;
}

#endif

unsigned int *g_holdrand;
__int64 __cdecl SOR_LCP_irand(int max)
{
	unsigned int holdrand; // [esp+0h] [ebp-Ch]

	if (!g_holdrand)
		MyAssertHandler(".\\physics\\ode\\src\\quickstep.cpp", 626, 0, "%s", "g_holdrand");
	holdrand = 214013 * *g_holdrand + 2531011;
	*g_holdrand = holdrand;
	return ((holdrand >> 17) * max) >> 15;
}

void __cdecl SOR_LCP_MainLoop(
	unsigned int constraintRowCount,
	ConstraintRowData *rows,
	int *findex,
	ConstraintForce *fc,
	dxQuickStepParameters *qs,
	SorLcpData *sd)
{
	int v6; // eax
	float v7; // [esp+10h] [ebp-6Ch]
	float v8; // [esp+14h] [ebp-68h]
	float v9; // [esp+18h] [ebp-64h]
	double v10; // [esp+1Ch] [ebp-60h]
	double v11; // [esp+24h] [ebp-58h]
	float v12; // [esp+2Ch] [ebp-50h]
	int v13; // [esp+30h] [ebp-4Ch]
	float v14; // [esp+34h] [ebp-48h]
	float v15; // [esp+38h] [ebp-44h]
	float v16; // [esp+3Ch] [ebp-40h]
	float v17; // [esp+3Ch] [ebp-40h]
	float scale; // [esp+3Ch] [ebp-40h]
	float v19; // [esp+40h] [ebp-3Ch]
	ConstraintForce *a; // [esp+48h] [ebp-34h]
	float lo; // [esp+4Ch] [ebp-30h]
	float hi; // [esp+50h] [ebp-2Ch]
	ConstraintForce *start; // [esp+54h] [ebp-28h]
	int body2; // [esp+58h] [ebp-24h]
	ConstraintRowData *b; // [esp+5Ch] [ebp-20h]
	int index; // [esp+60h] [ebp-1Ch]
	int tmp; // [esp+68h] [ebp-14h]
	int swapi; // [esp+6Ch] [ebp-10h]
	unsigned int iteration; // [esp+70h] [ebp-Ch]
	unsigned int num_iterations; // [esp+74h] [ebp-8h]
	unsigned int i; // [esp+78h] [ebp-4h]
	unsigned int ia; // [esp+78h] [ebp-4h]

	num_iterations = qs->num_iterations;
	for (iteration = 0; iteration < num_iterations; ++iteration)
	{
		if ((iteration & 7) == 7)
		{
			for (i = 0; i < constraintRowCount; ++i)
			{
				swapi = SOR_LCP_irand(constraintRowCount);
				v6 = SOR_LCP_irand(constraintRowCount);
				tmp = sd->order[swapi];
				sd->order[swapi] = sd->order[v6];
				sd->order[v6] = tmp;
			}
		}
		for (ia = 0; ia < constraintRowCount; ++ia)
		{
			index = sd->order[ia];
			v13 = findex[index];
			b = &rows[index];
			if (v13 < 0)
			{
				hi = b->hi;
				lo = b->lo;
			}
			else
			{
				v15 = rows[v13].lambda * b->hi;
				v12 = fabs(v15);
				hi = v12;
				lo = -v12;
			}
			body2 = b->body2;
			a = &fc[b->body1];
			v16 = b->rhs - rows[index].lambda * b->Ad;
			v11 = Vec3Dot(a->linear, b->J_body1Linear);
			v17 = v16 - (Vec3Dot(a->angular, b->J_body1Angular) + v11);
			start = 0;
			if (body2 >= 0)
			{
				start = &fc[body2];
				v10 = Vec3Dot(start->linear, b->J_body2Linear);
				v17 = v17 - (Vec3Dot(start->angular, b->J_body2Angular) + v10);
			}
			v19 = rows[index].lambda + v17;
			v9 = v19 - hi;
			if (v9 < 0.0)
				v14 = rows[index].lambda + v17;
			else
				v14 = hi;
			v8 = lo - v19;
			if (v8 < 0.0)
				v7 = v14;
			else
				v7 = lo;
			scale = v7 - rows[index].lambda;
			rows[index].lambda = v7;
			Vec3Mad(a->linear, scale, b->iMJ_body1Linear, a->linear);
			Vec3Mad(a->angular, scale, b->iMJ_body1Angular, a->angular);

			assert(isfinite(a->linear[0]));
			assert(isfinite(a->linear[1]));
			assert(isfinite(a->linear[2]));

			assert(isfinite(a->angular[0]));
			assert(isfinite(a->angular[1]));
			assert(isfinite(a->angular[2]));

			if (body2 >= 0)
			{
				Vec3Mad(start->linear, scale, b->iMJ_body2Linear, start->linear);
				Vec3Mad(start->angular, scale, b->iMJ_body2Angular, start->angular);

				assert(isfinite(start->linear[0]));
				assert(isfinite(start->linear[1]));
				assert(isfinite(start->linear[2]));

				assert(isfinite(start->angular[0]));
				assert(isfinite(start->angular[1]));
				assert(isfinite(start->angular[2]));
			}
		}
	}
}

void __cdecl SOR_LCP_MainLoop_OneBody(
	unsigned int constraintRowCount,
	ConstraintRowData *rows,
	int *findex,
	ConstraintForce *fc,
	dxQuickStepParameters *qs,
	SorLcpData *sd)
{
	float v6; // [esp+10h] [ebp-64h]
	float v7; // [esp+14h] [ebp-60h]
	float v8; // [esp+18h] [ebp-5Ch]
	double v9; // [esp+1Ch] [ebp-58h]
	float v10; // [esp+24h] [ebp-50h]
	int v11; // [esp+28h] [ebp-4Ch]
	float v12; // [esp+2Ch] [ebp-48h]
	float v13; // [esp+30h] [ebp-44h]
	float v14; // [esp+34h] [ebp-40h]
	float v15; // [esp+34h] [ebp-40h]
	float scale; // [esp+34h] [ebp-40h]
	float v17; // [esp+38h] [ebp-3Ch]
	float lo; // [esp+3Ch] [ebp-38h]
	float hi; // [esp+40h] [ebp-34h]
	ConstraintRowData *b; // [esp+44h] [ebp-30h]
	int index; // [esp+48h] [ebp-2Ch]
	int swapj; // [esp+4Ch] [ebp-28h]
	int tmp; // [esp+50h] [ebp-24h]
	int swapi; // [esp+54h] [ebp-20h]
	ConstraintForce *fc_ptr; // [esp+58h] [ebp-1Ch]
	int body; // [esp+5Ch] [ebp-18h]
	unsigned int iteration; // [esp+60h] [ebp-14h]
	unsigned int num_iterations; // [esp+68h] [ebp-Ch]
	unsigned int i; // [esp+70h] [ebp-4h]
	unsigned int ia; // [esp+70h] [ebp-4h]

	num_iterations = qs->num_iterations;
	body = rows->body1;
	fc_ptr = &fc[body];
	for (iteration = 0; iteration < num_iterations; ++iteration)
	{
		if ((iteration & 7) == 7)
		{
			for (i = 0; i < constraintRowCount; ++i)
			{
				swapi = SOR_LCP_irand(constraintRowCount);
				swapj = SOR_LCP_irand(constraintRowCount);
				tmp = sd->order[swapi];
				sd->order[swapi] = sd->order[swapj];
				sd->order[swapj] = tmp;
			}
		}
		for (ia = 0; ia < constraintRowCount; ++ia)
		{
			index = sd->order[ia];
			if (body != rows[index].body1)
				MyAssertHandler(".\\physics\\ode\\src\\quickstep.cpp", 727, 0, "%s", "body == rows[index].body1");
			if (rows[index].body2 >= 0)
				MyAssertHandler(".\\physics\\ode\\src\\quickstep.cpp", 728, 0, "%s", "rows[index].body2 < 0");
			v11 = findex[index];
			b = &rows[index];
			if (v11 < 0)
			{
				hi = b->hi;
				lo = b->lo;
			}
			else
			{
				v13 = rows[v11].lambda * b->hi;
				v10 = fabs(v13);
				hi = v10;
				lo = -v10;
			}
			v14 = b->rhs - rows[index].lambda * b->Ad;
			v9 = Vec3Dot(fc_ptr->linear, b->J_body1Linear);
			v15 = v14 - (Vec3Dot(fc_ptr->angular, b->J_body1Angular) + v9);
			v17 = rows[index].lambda + v15;
			v8 = v17 - hi;
			if (v8 < 0.0)
				v12 = rows[index].lambda + v15;
			else
				v12 = hi;
			v7 = lo - v17;
			if (v7 < 0.0)
				v6 = v12;
			else
				v6 = lo;
			scale = v6 - rows[index].lambda;
			rows[index].lambda = v6;
			Vec3Mad(fc_ptr->linear, scale, b->iMJ_body1Linear, fc_ptr->linear);
			Vec3Mad(fc_ptr->angular, scale, b->iMJ_body1Angular, fc_ptr->angular);

			assert(!IS_NAN(fc_ptr->linear[0]));
			assert(!IS_NAN(fc_ptr->linear[1]));
			assert(!IS_NAN(fc_ptr->linear[2]));

			assert(!IS_NAN(fc_ptr->angular[0]));
			assert(!IS_NAN(fc_ptr->angular[1]));
			assert(!IS_NAN(fc_ptr->angular[2]));
		}
	}
}

// KISAKTODO they fuck with this somehow
static void SOR_LCP (int constraintRowCount,
	unsigned int bodyCount,
	ConstraintRowData *rows,
	dxBody *const *body,
	const float *invI,
	ConstraintForce *fc,
	const float *cfm,
	int *findex,
	dxQuickStepParameters *qs,
	SorLcpData *sd)
{
	float suma; // [esp+38h] [ebp-18h]
	float sum; // [esp+38h] [ebp-18h]
	float sumb; // [esp+38h] [ebp-18h]
	int j; // [esp+3Ch] [ebp-14h]
	float sor_w; // [esp+40h] [ebp-10h]
	int k; // [esp+44h] [ebp-Ch]
	int i; // [esp+48h] [ebp-8h]
	int ib; // [esp+48h] [ebp-8h]
	ConstraintRowData *row_ptr; // [esp+4Ch] [ebp-4h]

	sor_w = qs->w;
	if (constraintRowCount > 444)
		MyAssertHandler(
			".\\physics\\ode\\src\\quickstep.cpp",
			769,
			0,
			"%s\n\t(constraintRowCount) = %i",
			"(constraintRowCount <= ( 74 * 6 ))",
			constraintRowCount);

	iassert(rows);
	iassert(body);
	iassert(qs);
	iassert(sd);

	PROF_SCOPED("Phys_ODE_SOR_LCP");

	compute_invM_JT(constraintRowCount, bodyCount, rows, body, invI);
	memset(fc, 0, sizeof(ConstraintForce) * bodyCount);
	for (i = 0; i < constraintRowCount; ++i)
	{
		row_ptr = &rows[i];
		suma = Vec3Dot(row_ptr->J_body1Linear, row_ptr->iMJ_body1Linear) + 0.0;
		sum = Vec3Dot(row_ptr->J_body1Angular, row_ptr->iMJ_body1Angular) + suma;
		if (row_ptr->body2 >= 0)
		{
			sumb = Vec3Dot(row_ptr->J_body2Linear, row_ptr->iMJ_body2Linear) + sum;
			sum = Vec3Dot(row_ptr->J_body2Angular, row_ptr->iMJ_body2Angular) + sumb;
		}
		iassert(sum + cfm[i] != 0);

		row_ptr->Ad = sor_w / (sum + cfm[i]);
	}
	for (i = 0; i < constraintRowCount; ++i)
	{
		row_ptr = &rows[i];
		Vec3Scale(row_ptr->J_body1Linear, row_ptr->Ad, row_ptr->J_body1Linear);
		Vec3Scale(row_ptr->J_body1Angular, row_ptr->Ad, row_ptr->J_body1Angular);
		Vec3Scale(row_ptr->J_body2Linear, row_ptr->Ad, row_ptr->J_body2Linear);
		Vec3Scale(row_ptr->J_body2Angular, row_ptr->Ad, row_ptr->J_body2Angular);

		iassert(isfinite(row_ptr->rhs));
		// fprintf(stderr, "math is hard; %f * %f = %f\n", row_ptr->rhs, row_ptr->Ad, row_ptr->rhs * row_ptr->Ad);
		row_ptr->rhs = row_ptr->rhs * row_ptr->Ad;
		iassert(isfinite(row_ptr->rhs));

		row_ptr->Ad = row_ptr->Ad * cfm[i];
	}
	j = 0;
	k = 1;
	for (ib = 0; ib < constraintRowCount; ++ib)
	{
		if (findex[ib] >= 0)
			sd->order[constraintRowCount - k++] = ib;
		else
			sd->order[j++] = ib;
	}
	if (j + k - 1 != constraintRowCount)
		MyAssertHandler(".\\physics\\ode\\src\\quickstep.cpp", 861, 0, "%s", "(j + k - 1) == constraintRowCount");
	if (bodyCount == 1)
		SOR_LCP_MainLoop_OneBody(constraintRowCount, rows, findex, fc, qs, sd);
	else
		SOR_LCP_MainLoop(constraintRowCount, rows, findex, fc, qs, sd);
}

const char *spaces =
"                                        ";

void __cdecl pr_0(const char *msg, int indent)
{
	int v2; // [esp+0h] [ebp-8h]
	int v3; // [esp+4h] [ebp-4h]

	if (indent < 40)
		v3 = indent;
	else
		v3 = 40;
	if (v3 > 0)
		v2 = v3;
	else
		v2 = 0;
	Com_Printf(20, "%s%s", &spaces[40 - v2], msg);
}

const char *__cdecl jointKind(const dxJoint *joint)
{
	switch (joint->typenum)
	{
	case 4:
		return "Contact";
	case 8:
		return "AMotor";
	case 2:
		return "Hinge";
	case 1:
		return "Ball";
	}
	return "Unknown";
}

int __cdecl ptrIdx(void **ptrs, void *ptr, int numPtrs)
{
	int i; // [esp+0h] [ebp-4h]

	for (i = 0; i < numPtrs; ++i)
	{
		if (ptrs[i] == ptr)
			return i;
	}
	return -1;
}

void __cdecl dumpBodyJoints(dxWorld *world, dxBody **bodies, int bodyCount, dxJoint *const *joints, int jointCount)
{
	// KISAKTODOLATER: Physics debug dumping
#if 0
	const char *v5; // eax
	const char *v6; // eax
	int v7; // eax
	const char *v8; // eax
	const char *v9; // eax
	int v10; // [esp+30h] [ebp-34h]
	const char *v11; // [esp+34h] [ebp-30h]
	int bodyIdx; // [esp+3Ch] [ebp-28h]
	float normal; // [esp+40h] [ebp-24h]
	float normal_4; // [esp+44h] [ebp-20h]
	float normal_8; // [esp+48h] [ebp-1Ch]
	int jointIdx; // [esp+50h] [ebp-14h]
	const dxJoint *joint; // [esp+54h] [ebp-10h]
	const dxBody *body; // [esp+58h] [ebp-Ch]
	dxJointNode *jointNode; // [esp+5Ch] [ebp-8h]

	v5 = va("Dumping island - %d bodies, %d joints\n", bodyCount, jointCount);
	pr_0(v5, 0);
	for (bodyIdx = 0; bodyIdx < bodyCount; ++bodyIdx)
	{
		body = bodies[bodyIdx];
		v6 = va(
			"\n%d: %0x8p  Mass: %f  Vel: %f %f %f  AVel: %f %f %f\n",
			bodyIdx,
			body,
			body->mass.mass,
			body->info.lvel[0],
			body->info.lvel[1],
			body->info.lvel[2],
			body->info.avel[0],
			body->info.avel[1],
			body->info.avel[2]);
		pr_0(v6, 0);
		jointNode = body->firstjoint;
		jointIdx = 0;
		while (jointNode)
		{
			joint = jointNode->joint;
			v11 = jointKind(jointNode->joint);
			v10 = ptrIdx((void**)bodies, joint->node[1].body, bodyCount);
			v7 = ptrIdx((void **)bodies, joint->node[0].body, bodyCount);
			v8 = va("%d: %0x8p  Bodies: %d %d  Kind: %s  ", jointIdx, joint, v7, v10, v11);
			pr_0(v8, 4);
			if (joint->typenum == 4)
			{
				if ((joint->flags & 2) != 0)
				{
					normal = -*&joint[2].world;
					normal_4 = -*&joint[2].next;
					normal_8 = -*&joint[2].tome;
				}
				else
				{
					normal = *&joint[2].world;
					normal_4 = *&joint[2].next;
					normal_8 = *&joint[2].tome;
				}
				v9 = va(
					"Pos: %f %f %f  Normal: %f %f %f  Depth: %f\n",
					*&joint[1].node[1].body,
					*&joint[1].node[1].bodyTag,
					*&joint[1].node[1].next,
					normal,
					normal_4,
					normal_8,
					*&joint[2].tag);
				pr_0(v9, 0);
			}
			else
			{
				pr_0("\n", 0);
			}
			jointNode = jointNode->next;
			++jointIdx;
		}
	}
#endif
}

#define ISLAND_MAX_JOINT_COUNT 74
#define ISLAND_MAX_BODY_COUNT 14

void dxQuickStepper (dxWorld *world,
	dxBody **body,
	int bodyCount,
	dxJoint **joint,
	int jointCount,
	float stepsize)
{
	float v6; // [esp+Ch] [ebp-20CCh]
	float v7; // [esp+10h] [ebp-20C8h]
	float inv_stepsize; // [esp+14h] [ebp-20C4h]
	float A[12]; // [esp+18h] [ebp-20C0h] BYREF
	QuickstepData *dst; // [esp+48h] [ebp-2090h]
	int j; // [esp+4Ch] [ebp-208Ch]
	float invI[168]; // [esp+50h] [ebp-2088h] BYREF
	_DWORD cr[74]; // [esp+2F0h] [ebp-1DE8h]
	ConstraintForce fc[14]; // [esp+418h] [ebp-1CC0h] BYREF
	int m; // [esp+568h] [ebp-1B70h]
	int constraintRowCount; // [esp+56Ch] [ebp-1B6Ch]
	float B[168]; // [esp+570h] [ebp-1B68h] BYREF
	dxJoint::Info1 info[74]; // [esp+810h] [ebp-18C8h] BYREF
	float a[444]; // [esp+A60h] [ebp-1678h] BYREF
	float cfm[444]; // [esp+1150h] [ebp-F88h] BYREF
	int findex[444]; // [esp+1840h] [ebp-898h] BYREF
	float in[84]; // [esp+1F30h] [ebp-1A8h] BYREF
	int bodyTag; // [esp+2080h] [ebp-58h]
	dxJoint::Info2 info2; // [esp+2084h] [ebp-54h] BYREF
	int rowIdx; // [esp+20B4h] [ebp-24h]
	float fps; // [esp+20B8h] [ebp-20h]
	float invMass; // [esp+20BCh] [ebp-1Ch]
	ConstraintRowData *rowPtr; // [esp+20C0h] [ebp-18h]
	SorLcpData *sd; // [esp+20C4h] [ebp-14h]
	int i; // [esp+20C8h] [ebp-10h]
	float v31; // [esp+20CCh] [ebp-Ch]
	dxWorldStepInfo *stepInfo; // [esp+20D0h] [ebp-8h]
	int bodyTag2; // [esp+20D4h] [ebp-4h]


	iassert(world);
	iassert(body);
	iassert(joint);

	if (jointCount >= 74 || bodyCount >= 14)
		dumpBodyJoints(world, body, bodyCount, joint, jointCount);

	bcassert(jointCount, ISLAND_MAX_JOINT_COUNT);
	bcassert(bodyCount, ISLAND_MAX_BODY_COUNT);

	inv_stepsize = 1.0 / stepsize;
	fps = inv_stepsize;
	stepInfo = &world->stepInfo;

	g_holdrand = &stepInfo->holdrand;
	dst = &world->qd;
	sd = &world->sd;
	for (i = 0; i < bodyCount; ++i)
	{
		dMULTIPLY2_333(A, body[i]->mass.I, body[i]->info.R);
		dMULTIPLY0_333(&B[12 * i], body[i]->info.R, A);
		dMULTIPLY2_333(A, body[i]->invI, body[i]->info.R);
		dMULTIPLY0_333(&invI[12 * i], body[i]->info.R, A);
		dMULTIPLY0_331(A, &B[12 * i], body[i]->info.avel);
		body[i]->tacc[0] = body[i]->tacc[0] - (body[i]->info.avel[1] * A[2] - body[i]->info.avel[2] * A[1]);
		body[i]->tacc[1] = body[i]->tacc[1] - (body[i]->info.avel[2] * A[0] - body[i]->info.avel[0] * A[2]);
		body[i]->tacc[2] = body[i]->tacc[2] - (body[i]->info.avel[0] * A[1] - body[i]->info.avel[1] * A[0]);
	}
	for (i = 0; i < bodyCount; ++i)
	{
		if ((body[i]->flags & 8) == 0)
		{
			body[i]->facc[0] = body[i]->mass.mass * stepInfo->gravity[0] + body[i]->facc[0];
			body[i]->facc[1] = body[i]->mass.mass * stepInfo->gravity[1] + body[i]->facc[1];
			body[i]->facc[2] = body[i]->mass.mass * stepInfo->gravity[2] + body[i]->facc[2];
		}
	}
	constraintRowCount = 0;
	for (i = 0; i < jointCount; ++i)
	{
		jointGetInfo1(joint[i], &info[i]);
		iassert(info[i].m >= 0 && info[i].m <= 6 && info[i].nub >= 0 && info[i].nub <= info[i].m);
		cr[i] = constraintRowCount;
		constraintRowCount += info[i].m;
	}

	vassert(constraintRowCount < (ISLAND_MAX_JOINT_COUNT * ISLAND_MAX_BODY_COUNT), "constraintRowCount = %d", constraintRowCount);
	if (constraintRowCount > 0)
	{
		dSetZero(a, constraintRowCount);
		dSetValue(cfm, constraintRowCount, stepInfo->global_cfm);
		memset(dst->rowData, 0, sizeof(ConstraintRowData) * constraintRowCount);
		for (i = 0; i < constraintRowCount; ++i)
		{
			dst->rowData[i].lo = -dInfinity;
			dst->rowData[i].hi = dInfinity;
			findex[i] = -1;
		}
		info2.rowskip = sizeof(ConstraintRowData) / sizeof(float);
		info2.fps = fps;
		info2.erp = stepInfo->global_erp;
		for (i = 0; i < jointCount; ++i)
		{
			m = info[i].m;
			if (m)
			{
				rowIdx = cr[i];
				rowPtr = &dst->rowData[rowIdx];
				info2.J1l = rowPtr->J_body1Linear;
				info2.J1a = rowPtr->J_body1Angular;
				info2.J2l = rowPtr->J_body2Linear;
				info2.J2a = rowPtr->J_body2Angular;
				info2.c = &a[rowIdx];
				info2.cfm = &cfm[rowIdx];
				info2.lo = &rowPtr->lo;
				info2.hi = &rowPtr->hi;
				info2.findex = &findex[rowIdx];
				jointGetInfo2(joint[i], stepInfo, &info2);
				
				for (j = 0; j < m; ++j)
				{
					if (findex[j + rowIdx] >= 0)
						findex[j + rowIdx] += rowIdx;
				}

				// iassert(rowPtr->J_body1Linear[0] > -99999);
				// iassert(rowPtr->J_body1Linear[1] > -99999);
				// iassert(rowPtr->J_body1Linear[2] > -99999);
				// 
				// iassert(rowPtr->J_body1Linear[0] < 99999);
				// iassert(rowPtr->J_body1Linear[1] < 99999);
				// iassert(rowPtr->J_body1Linear[2] < 99999);
			}
		}
		rowPtr = dst->rowData;
		for (i = 0; i < jointCount; ++i)
		{
			bodyTag = joint[i]->node[0].bodyTag;
			bodyTag2 = joint[i]->node[1].bodyTag;
			for (j = 0; j < info[i].m; ++j)
			{
				rowPtr->body1 = bodyTag;
				rowPtr->body2 = bodyTag2;

				// iassert(rowPtr->J_body1Linear[0] > -99999);
				// iassert(rowPtr->J_body1Linear[1] > -99999);
				// iassert(rowPtr->J_body1Linear[2] > -99999);
				// 
				// iassert(rowPtr->J_body1Linear[0] < 99999);
				// iassert(rowPtr->J_body1Linear[1] < 99999);
				// iassert(rowPtr->J_body1Linear[2] < 99999);

				++rowPtr;
			}
		}
		iassert(rowPtr == dst->rowData + constraintRowCount);
		for (i = 0; i < bodyCount; ++i)
		{
			invMass = body[i]->invMass;
			for (j = 0; j < 3; ++j)
				in[6 * i + j] = body[i]->facc[j] * invMass + body[i]->info.lvel[j] * fps;
			dMULTIPLY0_331(&in[6 * i + 3], &invI[12 * i], body[i]->tacc);
			for (j = 0; j < 3; ++j)
				in[6 * i + 3 + j] = body[i]->info.avel[j] * fps + in[6 * i + 3 + j];
		}
		multiply_J(constraintRowCount, dst->rowData, in);
		for (i = 0; i < constraintRowCount; ++i)
		{
			iassert(isfinite(a[i]));
			iassert(isfinite(fps));
			iassert(isfinite(dst->rowData[i].rhs));

			dst->rowData[i].rhs = a[i] * fps - dst->rowData[i].rhs;
			// iassert(isfinite(dst->rowData[i].rhs));
			
			cfm[i] = cfm[i] * fps;
		}
		SOR_LCP(constraintRowCount, bodyCount, dst->rowData, body, invI, fc, cfm, findex, &stepInfo->qs, sd);
		for (i = 0; i < bodyCount; ++i)
		{
			Vec3Mad(body[i]->info.lvel, stepsize, fc[i].linear, body[i]->info.lvel);
			Vec3Mad(body[i]->info.avel, stepsize, fc[i].angular, body[i]->info.avel);

			// fprintf(stderr, "BODY %d NOW WITH LVEL (%f, %f, %f)\n", i, body[i]->info.lvel[0], body[i]->info.lvel[1], body[i]->info.lvel[2]);
		}
	}
	for (i = 0; i < bodyCount; ++i)
	{
		invMass = body[i]->invMass;
		for (j = 0; j < 3; ++j)
			body[i]->info.lvel[j] = stepsize * invMass * body[i]->facc[j] + body[i]->info.lvel[j];
		for (j = 0; j < 3; ++j)
			body[i]->tacc[j] = body[i]->tacc[j] * stepsize;
		dMULTIPLYADD0_331(body[i]->info.avel, &invI[12 * i], body[i]->tacc);
	}
	for (i = 0; i < bodyCount; ++i)
	{
		v31 = Vec3LengthSq(body[i]->info.avel);
		if (v31 > 1600.0)
		{
			v7 = sqrt(v31);
			v31 = 39.0 / v7;
			iassert(!IS_NAN(v31));
			Vec3Scale(body[i]->info.avel, v31, body[i]->info.avel);
		}
		v31 = Vec3LengthSq(body[i]->info.lvel);
		if (v31 > 9.99999995904e11)
		{
			v6 = sqrt(v31);
			v31 = 999900.0 / v6;
			iassert(!IS_NAN(v31));
			Vec3Scale(body[i]->info.lvel, v31, body[i]->info.lvel);
		}
	}
	for (i = 0; i < bodyCount; ++i)
	{
		dxStepBody(body[i], stepsize);
		body[i]->flags &= ~4u;
	}
	for (j = 0; j < jointCount; ++j)
		joint[j]->tag = 1;

#if 0
	int i,j;
	IFTIMING(dTimerStart("preprocessing");)

	dReal stepsize1 = dRecip(stepsize);

	// number all bodies in the body list - set their tag values
	for (i=0; i<nb; i++) body[i]->tag = i;
	
	// make a local copy of the joint array, because we might want to modify it.
	// (the "dxJoint *const*" declaration says we're allowed to modify the joints
	// but not the joint array, because the caller might need it unchanged).
	//@@@ do we really need to do this? we'll be sorting constraint rows individually, not joints
	dxJoint **joint = (dxJoint**) alloca (nj * sizeof(dxJoint*));
	memcpy (joint,_joint,nj * sizeof(dxJoint*));
	
	// for all bodies, compute the inertia tensor and its inverse in the global
	// frame, and compute the rotational force and add it to the torque
	// accumulator. I and invI are a vertical stack of 3x4 matrices, one per body.
	// REM dRealAllocaArray (I,3*4*nb);	// need to remember all I's for feedback purposes only
	dRealAllocaArray (invI,3*4*nb);
	for (i=0; i<nb; i++) {
		dMatrix3 tmp;
		// compute inertia tensor in global frame
		dMULTIPLY2_333 (tmp,body[i]->mass.I,body[i]->info.R);
		// REM dMULTIPLY0_333 (I+i*12,body[i]->info.R,tmp);
		// compute inverse inertia tensor in global frame
		dMULTIPLY2_333 (tmp,body[i]->invI,body[i]->info.R);
		dMULTIPLY0_333 (invI+i*12,body[i]->info.R,tmp);
		// compute rotational force
		// REM dMULTIPLY0_331 (tmp,I+i*12,body[i]->info.avel);
		dCROSS (body[i]->tacc,-=,body[i]->info.avel,tmp);
	}

	// add the gravity force to all bodies
	for (i=0; i<nb; i++) {
		if ((body[i]->flags & dxBodyNoGravity)==0) {
			body[i]->facc[0] += body[i]->mass.mass * world->stepInfo.gravity[0];
			body[i]->facc[1] += body[i]->mass.mass * world->stepInfo.gravity[1];
			body[i]->facc[2] += body[i]->mass.mass * world->stepInfo.gravity[2];
		}
	}

	// get joint information (m = total constraint dimension, nub = number of unbounded variables).
	// joints with m=0 are inactive and are removed from the joints array
	// entirely, so that the code that follows does not consider them.
	//@@@ do we really need to save all the info1's
	dxJoint::Info1 *info = (dxJoint::Info1*) alloca (nj*sizeof(dxJoint::Info1));
	for (i=0, j=0; j<nj; j++) {	// i=dest, j=src
		jointGetInfo1(joint[j], info + i);
		dIASSERT (info[i].m >= 0 && info[i].m <= 6 && info[i].nub >= 0 && info[i].nub <= info[i].m);
		if (info[i].m > 0) {
			joint[i] = joint[j];
			i++;
		}
	}
	nj = i;

	// create the row offset array
	int m = 0;
	int *ofs = (int*) alloca (nj*sizeof(int));
	for (i=0; i<nj; i++) {
		ofs[i] = m;
		m += info[i].m;
	}

	// if there are constraints, compute the constraint force
	dRealAllocaArray (J,m*12);
	int *jb = (int*) alloca (m*2*sizeof(int));
	if (m > 0) {
		// create a constraint equation right hand side vector `c', a constraint
		// force mixing vector `cfm', and LCP low and high bound vectors, and an
		// 'findex' vector.
		dRealAllocaArray (c,m);
		dRealAllocaArray (cfm,m);
		dRealAllocaArray (lo,m);
		dRealAllocaArray (hi,m);
		int *findex = (int*) alloca (m*sizeof(int));
		dSetZero (c,m);
		dSetValue (cfm,m,world->stepInfo.global_cfm);
		dSetValue (lo,m,-dInfinity);
		dSetValue (hi,m, dInfinity);
		for (i=0; i<m; i++) findex[i] = -1;
		
		// get jacobian data from constraints. an m*12 matrix will be created
		// to store the two jacobian blocks from each constraint. it has this
		// format:
		//
		//   l1 l1 l1 a1 a1 a1 l2 l2 l2 a2 a2 a2 \    .
		//   l1 l1 l1 a1 a1 a1 l2 l2 l2 a2 a2 a2  }-- jacobian for joint 0, body 1 and body 2 (3 rows)
		//   l1 l1 l1 a1 a1 a1 l2 l2 l2 a2 a2 a2 /
		//   l1 l1 l1 a1 a1 a1 l2 l2 l2 a2 a2 a2 }--- jacobian for joint 1, body 1 and body 2 (3 rows)
		//   etc...
		//
		//   (lll) = linear jacobian data
		//   (aaa) = angular jacobian data
		//
		IFTIMING (dTimerNow ("create J");)
		dSetZero (J,m*12);
		dxJoint::Info2 Jinfo;
		Jinfo.rowskip = 12;
		Jinfo.fps = stepsize1;
		Jinfo.erp = world->stepInfo.global_erp;
		for (i=0; i<nj; i++) {
			Jinfo.J1l = J + ofs[i]*12;
			Jinfo.J1a = Jinfo.J1l + 3;
			Jinfo.J2l = Jinfo.J1l + 6;
			Jinfo.J2a = Jinfo.J1l + 9;
			Jinfo.c = c + ofs[i];
			Jinfo.cfm = cfm + ofs[i];
			Jinfo.lo = lo + ofs[i];
			Jinfo.hi = hi + ofs[i];
			Jinfo.findex = findex + ofs[i];
			jointGetInfo2(joint[i], &world->stepInfo, &Jinfo);
			// adjust returned findex values for global index numbering
			for (j=0; j<info[i].m; j++) {
				if (findex[ofs[i] + j] >= 0) findex[ofs[i] + j] += ofs[i];
			}
		}

		// create an array of body numbers for each joint row
		int *jb_ptr = jb;
		for (i=0; i<nj; i++) {
			int b1 = (joint[i]->node[0].body) ? (joint[i]->node[0].body->tag) : -1;
			int b2 = (joint[i]->node[1].body) ? (joint[i]->node[1].body->tag) : -1;
			for (j=0; j<info[i].m; j++) {
				jb_ptr[0] = b1;
				jb_ptr[1] = b2;
				jb_ptr += 2;
			}
		}
		dIASSERT (jb_ptr == jb+2*m);

		// compute the right hand side `rhs'
		IFTIMING (dTimerNow ("compute rhs");)
		dRealAllocaArray (tmp1,nb*6);
		// put v/h + invM*fe into tmp1
		for (i=0; i<nb; i++) {
			dReal body_invMass = body[i]->invMass;
			for (j=0; j<3; j++) tmp1[i*6+j] = body[i]->facc[j] * body_invMass + body[i]->info.lvel[j] * stepsize1;
			dMULTIPLY0_331 (tmp1 + i*6 + 3,invI + i*12,body[i]->tacc);
			for (j=0; j<3; j++) tmp1[i*6+3+j] += body[i]->info.avel[j] * stepsize1;
		}

		// put J*tmp1 into rhs
		dRealAllocaArray (rhs,m);
		multiply_J (m,J,jb,tmp1,rhs);

		// complete rhs
		for (i=0; i<m; i++) rhs[i] = c[i]*stepsize1 - rhs[i];

		// scale CFM
		for (i=0; i<m; i++) cfm[i] *= stepsize1;

		// load lambda from the value saved on the previous iteration
		dRealAllocaArray (lambda,m);
#ifdef WARM_STARTING
		dSetZero (lambda,m);	//@@@ shouldn't be necessary
		for (i=0; i<nj; i++) {
			memcpy (lambda+ofs[i],joint[i]->lambda,info[i].m * sizeof(dReal));
		}
#endif

		// solve the LCP problem and get lambda and invM*constraint_force
		IFTIMING (dTimerNow ("solving LCP problem");)
		dRealAllocaArray (cforce,nb*6);
		SOR_LCP (m,nb,J,jb,body,invI,lambda,cforce,rhs,lo,hi,cfm,findex,&world->stepInfo.qs);

#ifdef WARM_STARTING
		// save lambda for the next iteration
		//@@@ note that this doesn't work for contact joints yet, as they are
		// recreated every iteration
		for (i=0; i<nj; i++) {
			memcpy (joint[i]->lambda,lambda+ofs[i],info[i].m * sizeof(dReal));
		}
#endif

		// note that the SOR method overwrites rhs and J at this point, so
		// they should not be used again.
		
		// add stepsize * cforce to the body velocity
		for (i=0; i<nb; i++) {
			for (j=0; j<3; j++) body[i]->info.lvel[j] += stepsize * cforce[i*6+j];
			for (j=0; j<3; j++) body[i]->info.avel[j] += stepsize * cforce[i*6+3+j];
		}

		// if joint feedback is requested, compute the constraint force.
		// BUT: cforce is inv(M)*J'*lambda, whereas we want just J'*lambda,
		// so we must compute M*cforce.
		// @@@ if any joint has a feedback request we compute the entire
		//     adjusted cforce, which is not the most efficient way to do it.
#if 0 // REM? KISAKTODO
		for (j=0; j<nj; j++) {
			if (joint[j]->feedback) {
				// compute adjusted cforce
				for (i=0; i<nb; i++) {
					dReal k = body[i]->mass.mass;
					cforce [i*6+0] *= k;
					cforce [i*6+1] *= k;
					cforce [i*6+2] *= k;
					dVector3 tmp;
					dMULTIPLY0_331 (tmp, I + 12*i, cforce + i*6 + 3);
					cforce [i*6+3] = tmp[0];
					cforce [i*6+4] = tmp[1];
					cforce [i*6+5] = tmp[2];
				}
				// compute feedback for this and all remaining joints
				for (; j<nj; j++) {
					dJointFeedback *fb = joint[j]->feedback;
					if (fb) {
						int b1 = joint[j]->node[0].body->tag;
						memcpy (fb->f1,cforce+b1*6,3*sizeof(dReal));
						memcpy (fb->t1,cforce+b1*6+3,3*sizeof(dReal));
						if (joint[j]->node[1].body) {
							int b2 = joint[j]->node[1].body->tag;
							memcpy (fb->f2,cforce+b2*6,3*sizeof(dReal));
							memcpy (fb->t2,cforce+b2*6+3,3*sizeof(dReal));
						}
					}
				}
			}
		}
#endif
	}

	// compute the velocity update:
	// add stepsize * invM * fe to the body velocity

	IFTIMING (dTimerNow ("compute velocity update");)
	for (i=0; i<nb; i++) {
		dReal body_invMass = body[i]->invMass;
		for (j=0; j<3; j++) body[i]->info.lvel[j] += stepsize * body_invMass * body[i]->facc[j];
		for (j=0; j<3; j++) body[i]->tacc[j] *= stepsize;	
		dMULTIPLYADD0_331 (body[i]->info.avel,invI + i*12,body[i]->tacc);
	}

#if 0
	// check that the updated velocity obeys the constraint (this check needs unmodified J)
	dRealAllocaArray (vel,nb*6);
	for (i=0; i<nb; i++) {
		for (j=0; j<3; j++) vel[i*6+j] = body[i]->info.lvel[j];
		for (j=0; j<3; j++) vel[i*6+3+j] = body[i]->info.avel[j];
	}
	dRealAllocaArray (tmp,m);
	multiply_J (m,J,jb,vel,tmp);
	dReal error = 0;
	for (i=0; i<m; i++) error += dFabs(tmp[i]);
	printf ("velocity error = %10.6e\n",error);
#endif

	// update the position and orientation from the new linear/angular velocity
	// (over the given timestep)
	IFTIMING (dTimerNow ("update position");)
	for (i=0; i<nb; i++) dxStepBody (body[i],stepsize);

	IFTIMING (dTimerNow ("tidy up");)

	// zero all force accumulators
	for (i=0; i<nb; i++) {
		dSetZero (body[i]->facc,3);
		dSetZero (body[i]->tacc,3);
	}

	IFTIMING (dTimerEnd();)
	IFTIMING (if (m > 0) dTimerReport (stdout,1);)
#endif
}
