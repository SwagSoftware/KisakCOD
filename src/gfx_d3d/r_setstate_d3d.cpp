#include "r_setstate_d3d.h"
#include "r_init.h"


bool __cdecl RB_IsGpuFinished()
{
    const char *v1; // eax
    int hr; // [esp+0h] [ebp-8h]

	// KISAKGPUFENCE: Comment asserts out for now. Sometimes goes off when alt-tabbing.
    //if (!dx.flushGpuQuery)
    //    MyAssertHandler(".\\r_setstate_d3d.cpp", 16, 0, "%s", "dx.flushGpuQuery");
    //if (dx.flushGpuQueryCount != 1)
    //    MyAssertHandler(".\\r_setstate_d3d.cpp", 17, 0, "%s", "dx.flushGpuQueryCount == 1");
    hr = dx.flushGpuQuery->GetData(0, 0, 1);
    if (hr == -2005530520)
    {
        --dx.flushGpuQueryCount;
        return 1;
    }
    else
    {
        if (hr < 0)
        {
            do
            {
                ++g_disableRendering;
                v1 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    ".\\r_setstate_d3d.cpp (%i) dx.flushGpuQuery->GetData( 0, 0, (1 << 0) ) failed: %s\n",
                    28,
                    v1);
            } while (alwaysfails);
        }
        if ((unsigned int)hr > 1)
            MyAssertHandler(
                ".\\r_setstate_d3d.cpp",
                30,
                0,
                "%s\n\t(hr) = %08x",
                "(hr == ((HRESULT)0L) || hr == ((HRESULT)1L))",
                hr);
        if (hr != 1)
            --dx.flushGpuQueryCount;
        return hr != 1;
    }
}

bool __cdecl RB_IsGpuFenceFinished()
{
    if (dx.flushGpuQueryIssued)
    {
    	// KISAKGPUFENCE: Comment asserts out for now. Sometimes goes off when alt-tabbing.
        //if (dx.flushGpuQueryCount != 1)
        //    MyAssertHandler(".\\r_setstate_d3d.cpp", 48, 0, "%s", "dx.flushGpuQueryCount == 1");
        if (RB_IsGpuFinished())
        {
            dx.flushGpuQueryIssued = 0;
            // KISAKGPUFENCE: Comment asserts out for now. Sometimes goes off when alt-tabbing.
            //if (dx.flushGpuQueryCount)
            //    MyAssertHandler(".\\r_setstate_d3d.cpp", 54, 0, "%s", "!dx.flushGpuQueryCount");
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
    	// KISAKGPUFENCE: Comment asserts out for now. Sometimes goes off when alt-tabbing.
        //if (dx.flushGpuQueryCount)
        //    MyAssertHandler(".\\r_setstate_d3d.cpp", 44, 0, "%s", "!dx.flushGpuQueryCount");
        return 1;
    }
}