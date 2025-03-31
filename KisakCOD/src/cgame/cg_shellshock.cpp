#include "cg_local.h"
#include "cg_public.h"


void __cdecl CG_PerturbCamera(cg_s *cgameGlob)
{
    float rot[3][3]; // [esp+18h] [ebp-48h] BYREF
    float axis[3][3]; // [esp+3Ch] [ebp-24h] BYREF

    if (cgameGlob->shellshock.viewDelta[0] != 0.0 || cgameGlob->shellshock.viewDelta[1] != 0.0)
    {
        rot[0][0] = 1.0;
        rot[0][1] = cgameGlob->shellshock.viewDelta[0];
        rot[0][2] = cgameGlob->shellshock.viewDelta[1];
        rot[2][0] = 0.0;
        rot[2][1] = 0.0;
        rot[2][2] = 1.0;
        Vec3Normalize(rot[0]);
        Vec3Cross(rot[2], rot[0], rot[1]);
        Vec3Normalize(rot[1]);
        Vec3Cross(rot[0], rot[1], rot[2]);
        AxisCopy(cgameGlob->refdef.viewaxis, axis);
        MatrixMultiply(rot, axis, cgameGlob->refdef.viewaxis);
    }
}

int __cdecl CG_DrawShellShockSavedScreenBlendBlurred(
    int localClientNum,
    const shellshock_parms_t *parms,
    int start,
    int duration)
{
    float v5; // [esp+14h] [ebp-20h]
    int dt; // [esp+24h] [ebp-10h]
    const ClientViewParams *view; // [esp+2Ch] [ebp-8h]
    int screenBlendTime; // [esp+30h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (start && duration > 0)
    {
        dt = duration + start - MEMORY[0x9D5560];
        if (dt > 0)
        {
            screenBlendTime = parms->screenBlend.blurredEffectTime;
            if (dt < parms->screenBlend.blurredFadeTime)
            {
                v5 = (double)dt / (double)parms->screenBlend.blurredFadeTime * (double)screenBlendTime;
                screenBlendTime = (int)(v5 + 9.313225746154785e-10);
            }
            if (MEMORY[0x9DF924])
            {
                view = CG_GetLocalClientViewParams(localClientNum);
                R_AddCmdBlendSavedScreenShockBlurred(
                    screenBlendTime,
                    view->x,
                    view->y,
                    view->width,
                    view->height,
                    localClientNum);
            }
            SaveScreenToBuffer(localClientNum);
            MEMORY[0x9DF924] = 1;
            return 1;
        }
        else
        {
            MEMORY[0x9DF924] = 0;
            return 0;
        }
    }
    else
    {
        MEMORY[0x9DF924] = 0;
        return 0;
    }
}

void __cdecl SaveScreenToBuffer(int localClientNum)
{
    const ClientViewParams *view; // [esp+14h] [ebp-8h]

    if (CL_GetLocalClientActiveCount() > 1)
    {
        view = CG_GetLocalClientViewParams(localClientNum);
        R_AddCmdSaveScreenSection(view->x, view->y, view->width, view->height, localClientNum);
    }
    else
    {
        R_AddCmdSaveScreen(localClientNum);
    }
}

int __cdecl CG_DrawShellShockSavedScreenBlendFlashed(
    int localClientNum,
    const shellshock_parms_t *parms,
    int start,
    int duration)
{
    int dt; // [esp+18h] [ebp-14h]
    float whiteFactor; // [esp+20h] [ebp-Ch]
    float whiteFactora; // [esp+20h] [ebp-Ch]
    float whiteFactorb; // [esp+20h] [ebp-Ch]
    const ClientViewParams *view; // [esp+24h] [ebp-8h]
    float grabFactor; // [esp+28h] [ebp-4h]
    float grabFactora; // [esp+28h] [ebp-4h]
    float grabFactorb; // [esp+28h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (start && duration > 0)
    {
        dt = duration + start - MEMORY[0x9D5560];
        if (dt > 0)
        {
            whiteFactor = (float)parms->screenBlend.flashWhiteFadeTime;
            grabFactor = (float)parms->screenBlend.flashShotFadeTime;
            if (whiteFactor <= (double)dt)
                whiteFactora = 1.0;
            else
                whiteFactora = (double)dt / whiteFactor;
            if (grabFactor <= (double)dt)
                grabFactora = 1.0;
            else
                grabFactora = (double)dt / grabFactor;
            whiteFactorb = BlendSmooth(whiteFactora);
            grabFactorb = BlendSmooth(grabFactora);
            if (MEMORY[0x9DF924])
            {
                view = CG_GetLocalClientViewParams(localClientNum);
                R_AddCmdBlendSavedScreenShockFlashed(whiteFactorb, grabFactorb, view->x, view->y, view->width, view->height);
            }
            else
            {
                SaveScreenToBuffer(localClientNum);
            }
            MEMORY[0x9DF924] = 1;
            return 1;
        }
        else
        {
            MEMORY[0x9DF924] = 0;
            return 0;
        }
    }
    else
    {
        MEMORY[0x9DF924] = 0;
        return 0;
    }
}

double __cdecl BlendSmooth(float percent)
{
    float v3; // [esp+Ch] [ebp-Ch]
    float sin; // [esp+14h] [ebp-4h]

    v3 = (percent - 0.5) * 3.141592741012573;
    sin = sin(v3);
    return (float)((sin + 1.0) * 0.5);
}

void __cdecl CG_UpdateShellShock(int localClientNum, const shellshock_parms_t *parms, int start, int duration)
{
    int time; // [esp+14h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    time = MEMORY[0x9D5560] - start;
    if (start && time >= 0)
    {
        UpdateShellShockSound(localClientNum, parms, time, duration);
        UpdateShellShockLookControl(localClientNum, parms, time, duration);
        UpdateShellShockCamera(localClientNum, parms, time, duration);
    }
    else
    {
        EndShellShock(localClientNum);
    }
}

void __cdecl EndShellShock(int localClientNum)
{
    EndShellShockSound(localClientNum);
    EndShellShockLookControl(localClientNum);
    EndShellShockCamera(localClientNum);
    EndShellShockScreen(localClientNum);
}

void __cdecl EndShellShockSound(int localClientNum)
{
    snd_alias_t *alias; // [esp+10h] [ebp-4h]

    SND_DeactivateChannelVolumes(3, 0);
    SND_DeactivateEnvironmentEffects(2, 0);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (MEMORY[0x9DF914])
    {
        MEMORY[0x9DF914] = 0;
        alias = CL_PickSoundAlias("shellshock_end_abort");
        SND_PlaySoundAlias(alias, (SndEntHandle)1023, vec3_origin, 0, SASYS_CGAME);
    }
}

void __cdecl EndShellShockLookControl(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    MEMORY[0x9DF918] = 1.0;
    CL_CapTurnRate(localClientNum, 0.0, 0.0);
}

void __cdecl EndShellShockCamera(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    MEMORY[0x9DF91C] = 0.0;
    MEMORY[0x9DF920] = 0.0;
}

void __cdecl EndShellShockScreen(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    MEMORY[0x9DF924] = 0;
}

void __cdecl UpdateShellShockSound(int localClientNum, const shellshock_parms_t *parms, int time, int duration)
{
    const snd_alias_t *v4; // eax
    const snd_alias_t *v5; // eax
    int wetlevel; // [esp+10h] [ebp-3Ch]
    snd_alias_t *alias1; // [esp+30h] [ebp-1Ch]
    snd_alias_t *alias0; // [esp+34h] [ebp-18h]
    int dt; // [esp+38h] [ebp-14h]
    int dta; // [esp+38h] [ebp-14h]
    float fade; // [esp+3Ch] [ebp-10h]
    int end; // [esp+44h] [ebp-8h]

    if (!parms)
        MyAssertHandler(".\\cgame\\cg_shellshock.cpp", 443, 0, "%s", "parms");
    if (time < 0)
        MyAssertHandler(".\\cgame\\cg_shellshock.cpp", 444, 0, "%s", "time >= 0");
    if (duration < 0)
        MyAssertHandler(".\\cgame\\cg_shellshock.cpp", 445, 0, "%s", "duration >= 0");
    if (parms->sound.affect)
    {
        dt = parms->sound.fadeOutTime + parms->sound.modEndDelay + duration - time;
        if (time >= parms->sound.fadeInTime)
        {
            if (dt <= parms->sound.fadeOutTime)
            {
                if (dt >= 0 && dt < parms->sound.fadeOutTime)
                {
                    SND_DeactivateEnvironmentEffects(2, dt);
                    SND_DeactivateChannelVolumes(3, dt);
                }
            }
            else
            {
                SND_SetEnvironmentEffects(2, parms->sound.roomtype, parms->sound.drylevel, parms->sound.wetlevel, 0);
                SND_SetChannelVolumes(3, parms->sound.channelvolume, 0);
            }
        }
        else
        {
            SND_SetEnvironmentEffects(
                2,
                parms->sound.roomtype,
                parms->sound.drylevel,
                parms->sound.wetlevel,
                parms->sound.fadeInTime - time);
            SND_SetChannelVolumes(3, parms->sound.channelvolume, parms->sound.fadeInTime - time);
        }
        dta = parms->sound.loopFadeTime + parms->sound.loopEndDelay + duration - time;
        if (dta > 0)
        {
            alias0 = CL_PickSoundAlias(parms->sound.loop);
            alias1 = CL_PickSoundAlias(parms->sound.loopSilent);
            if (parms->sound.loopFadeTime && dta <= parms->sound.loopFadeTime)
                fade = 1.0 - (double)dta * 1.0 / (double)parms->sound.loopFadeTime;
            else
                fade = 0.0;
            SND_PlayBlendedSoundAliases(alias0, alias1, fade, 1.0, (SndEntHandle)1023, vec3_origin, 0, SASYS_CGAME);
        }
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        end = parms->sound.loopEndDelay + duration + MEMORY[0x9D5560] - time;
        if (MEMORY[0x9D5560] >= end)
        {
            if (end != MEMORY[0x9DF914])
            {
                MEMORY[0x9DF914] = parms->sound.loopEndDelay + duration + MEMORY[0x9D5560] - time;
                wetlevel = MEMORY[0x9D5560] - end;
                v5 = CL_PickSoundAlias(parms->sound.end);
                SND_PlaySoundAlias(v5, (SndEntHandle)1023, vec3_origin, wetlevel, SASYS_CGAME);
            }
        }
        else if (MEMORY[0x9DF914])
        {
            MEMORY[0x9DF914] = 0;
            v4 = CL_PickSoundAlias(parms->sound.endAbort);
            SND_PlaySoundAlias(v4, (SndEntHandle)1023, vec3_origin, 0, SASYS_CGAME);
        }
    }
    else
    {
        EndShellShockSound(localClientNum);
    }
}

void __cdecl UpdateShellShockLookControl(int localClientNum, const shellshock_parms_t *parms, int time, int duration)
{
    float maxPitchSpeed; // [esp+8h] [ebp-14h]
    float maxYawSpeed; // [esp+Ch] [ebp-10h]
    float fade; // [esp+14h] [ebp-8h]

    if (!parms)
        MyAssertHandler(".\\cgame\\cg_shellshock.cpp", 530, 0, "%s", "parms");
    if (time < 0)
        MyAssertHandler(".\\cgame\\cg_shellshock.cpp", 531, 0, "%s", "time >= 0");
    if (duration < 0)
        MyAssertHandler(".\\cgame\\cg_shellshock.cpp", 532, 0, "%s", "duration >= 0");
    if (!parms->lookControl.affect)
        goto LABEL_8;
    if (duration - time < parms->lookControl.fadeTime)
    {
        if (duration - time <= 0)
        {
        LABEL_8:
            EndShellShockLookControl(localClientNum);
            return;
        }
        fade = (double)(duration - time) * 1.0 / (double)parms->lookControl.fadeTime;
    }
    else
    {
        fade = 1.0;
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (fade == 1.0)
    {
        MEMORY[0x9DF918] = parms->lookControl.mouseSensitivity;
        CL_CapTurnRate(localClientNum, parms->lookControl.maxPitchSpeed, parms->lookControl.maxYawSpeed);
    }
    else
    {
        MEMORY[0x9DF918] = (parms->lookControl.mouseSensitivity - 1.0) * fade + 1.0;
        maxYawSpeed = parms->lookControl.maxYawSpeed / fade;
        maxPitchSpeed = parms->lookControl.maxPitchSpeed / fade;
        CL_CapTurnRate(localClientNum, maxPitchSpeed, maxYawSpeed);
    }
}

void __cdecl UpdateShellShockCamera(int localClientNum, const shellshock_parms_t *parms, int time, int duration)
{
    int dt; // [esp+20h] [ebp-20h]
    float ta; // [esp+24h] [ebp-1Ch]
    float t; // [esp+24h] [ebp-1Ch]
    float radius; // [esp+2Ch] [ebp-14h]
    const float *perturb; // [esp+30h] [ebp-10h]
    int base; // [esp+38h] [ebp-8h]
    float scale; // [esp+3Ch] [ebp-4h]
    float scalea; // [esp+3Ch] [ebp-4h]

    dt = duration - time;
    if (duration - time > 0)
    {
        if (!parms)
            MyAssertHandler(".\\cgame\\cg_shellshock.cpp", 593, 0, "%s", "parms");
        scale = 1.0;
        if (dt < parms->view.fadeTime)
            scale = (double)dt / (double)parms->view.fadeTime;
        scalea = (3.0 - scale * 2.0) * scale * scale;
        radius = parms->view.kickRadius * scalea;
        ta = (double)time * parms->view.kickRate;
        base = (int)(ta - 0.4999999990686774);
        t = ta - (double)base;
        perturb = (const float *)(8 * (((_BYTE)base + 61 * (_BYTE)duration) & 0x7F) + 8839944);
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        MEMORY[0x9DF91C] = CubicInterpolate(t, *perturb, perturb[2], perturb[4], perturb[6]) * radius;
        MEMORY[0x9DF920] = CubicInterpolate(t, perturb[1], perturb[3], perturb[5], perturb[7]) * radius;
    }
    else
    {
        EndShellShockCamera(localClientNum);
    }
}

double __cdecl CubicInterpolate(float t, float x0, float x1, float x2, float x3)
{
    float c; // [esp+4h] [ebp-10h]
    float b; // [esp+Ch] [ebp-8h]
    float a; // [esp+10h] [ebp-4h]

    a = x3 - x2 + x1 - x0;
    b = x0 - x1 - a;
    c = x2 - x0;
    return (float)(((t * a + b) * t + c) * t + x1);
}

void __cdecl CG_StartShellShock(cg_s *cgameGlob, const shellshock_parms_t *parms, int start, int duration)
{
    cgameGlob->shellshock.parms = parms;
    cgameGlob->shellshock.startTime = start;
    cgameGlob->shellshock.duration = duration;
}

bool __cdecl CG_Flashbanged(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    return MEMORY[0x9DF910] + MEMORY[0x9DF90C] - MEMORY[0x9D5560] > 0 && *(unsigned int *)(MEMORY[0x9DF908] + 16) != 0;
}

