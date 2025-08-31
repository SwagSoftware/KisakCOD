#include "r_debug.h"
#include "r_init.h"

const dvar_s *r_warningRepeatDelay;
unsigned int s_warnCount[41];


void R_WarnOncePerFrame(GfxWarningType warnType, ...)
{
    char message[1028]; // [esp+0h] [ebp-410h] BYREF
    float frameRate; // [esp+408h] [ebp-8h]
    char *vargs; // [esp+40Ch] [ebp-4h]
    va_list va; // [esp+41Ch] [ebp+Ch] BYREF

    va_start(va, warnType);
    iassert( r_warningRepeatDelay );
    frameRate = R_UpdateFrameRate();
    if (s_warnCount[warnType] < rg.frontEndFrameCount)
    {
        s_warnCount[warnType] = rg.frontEndFrameCount + (int)(frameRate * r_warningRepeatDelay->current.value);
        va_copy(vargs, va);
        _vsnprintf(message, 0x400u, s_warnFormat[warnType], va);
        vargs = 0;
        Com_PrintWarning(8, "%s", message);
    }
}

unsigned int frameCount;
int previous_0;
float frameRate;
double __cdecl R_UpdateFrameRate()
{
    int frameTime; // [esp+0h] [ebp-8h]
    unsigned int current; // [esp+4h] [ebp-4h]

    if (frameCount != rg.frontEndFrameCount)
    {
        if (frameCount)
        {
            if (frameCount + 1 == rg.frontEndFrameCount)
            {
                current = Sys_Milliseconds();
                frameTime = current - previous_0;
                previous_0 = current;
                if (!frameTime)
                    frameTime = 1;
                if (frameTime >= 0)
                    frameRate = 1000.0 / (double)frameTime;
                else
                    frameRate = 0.0;
            }
            else
            {
                frameRate = 0.0;
            }
        }
        else
        {
            previous_0 = Sys_Milliseconds();
        }
        frameCount = rg.frontEndFrameCount;
    }
    return frameRate;
}

void __cdecl R_WarnInitDvars()
{
    DvarLimits min; // [esp+4h] [ebp-10h]

    min.value.max = 30.0;
    min.value.min = 0.0;
    r_warningRepeatDelay = Dvar_RegisterFloat(
        "r_warningRepeatDelay",
        5.0,
        min,
        DVAR_NOFLAG,
        "Number of seconds after displaying a \"per-frame\" warning before it will display again");
}

