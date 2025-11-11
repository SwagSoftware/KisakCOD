#include "q_shared.h"

#include <cmath>

#include <Windows.h>
#include <qcommon/qcommon.h>

int initialized_1 = 0;
int sys_timeBase;

unsigned int __cdecl Sys_Milliseconds()
{
    if (!initialized_1)
    {
        sys_timeBase = timeGetTime();
        initialized_1 = 1;
    }
    return timeGetTime() - sys_timeBase;
}

unsigned int __cdecl Sys_MillisecondsRaw()
{
    return timeGetTime();
}

void __cdecl Sys_SnapVector(float *v)
{
    v[0] = SnapFloat(v[0]);
    v[1] = SnapFloat(v[1]);
    v[2] = SnapFloat(v[2]);

#ifdef _DEBUG
    for (size_t i = 0; i < 3; ++i)
    {
        const float input = *v;
        int32_t output{};

        __asm fld input
        __asm fistp output

        iassert(*v == output);
        //*v = static_cast<float>(output);
        ++v;
    }
#endif
}

