#pragma once

// LWSS ADD - Custom COD4
#include <physics/ode/collision_kernel.h>
#include <universal/pool_allocator.h>

#include "joint.h"

dxUserGeom *__cdecl Phys_GetWorldGeom();
void __cdecl ODE_LeakCheck();

void __cdecl ODE_Init();

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
    unsigned int geoms[425984];      // ...
    pooldata_t geomPool;                // ...
    dxUserGeom worldGeom;               // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

extern odeGlob_t odeGlob;

// LWSS END