/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
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

#ifndef _ODE_ODE_H_
#define _ODE_ODE_H_

/* include *everything* here */

#include <ode/config.h>
#include <ode/compatibility.h>
#include <ode/common.h>
#include <ode/contact.h>
#include <ode/error.h>
#include <ode/memory.h>
#include <ode/odemath.h>
#include <ode/matrix.h>
#include <ode/timer.h>
#include <ode/rotation.h>
#include <ode/mass.h>
#include <ode/misc.h>
#include <ode/objects.h>
#include <ode/odecpp.h>
#include <ode/collision_space.h>
#include <ode/collision.h>
#include <ode/odecpp_collision.h>
#include <ode/export-dif.h>

// LWSS ADD - Custom COD4
#include <physics/ode/collision_kernel.h>
#include <universal/pool_allocator.h>
void __cdecl ODE_Init();


struct dxSimpleSpace : public dxSpace {
    dxSimpleSpace(dSpaceID _space);
    void cleanGeoms();
    void collide(void *data, dNearCallback *callback);
    void collide2(void *data, dxGeom *geom, dNearCallback *callback);
};
struct dxJointGroup : public dBase {
    int num;		// number of joints on the stack
    dObStack stack;	// a stack of (possibly differently sized) dxJoint
};			// objects.
struct dxUserGeom : public dxGeom {
    void *user_data;

    dxUserGeom(int class_num);
    ~dxUserGeom();
    void computeAABB();
    int AABBTest(dxGeom *o, dReal aabb[6]);
};
struct odeGlob_t // sizeof=0x2C64E0
{                                       // ...
    dxWorld world[3];
    dxSimpleSpace space[3];             // ...
    dxJointGroup contactsGroup[3];      // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    dxBody bodies[512];                 // ...
    pooldata_t bodyPool;                // ...
    unsigned __int8 geoms[425984];      // ...
    pooldata_t geomPool;                // ...
    dxUserGeom worldGeom;               // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
extern odeGlob_t odeGlob;

// LWSS END
#endif
