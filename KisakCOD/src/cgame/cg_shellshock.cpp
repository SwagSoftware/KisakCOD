#include "cg_local.h"
#include "cg_public.h"

#include <client_mp/client_mp.h>
#include <cgame_mp/cg_local_mp.h>

#include <gfx_d3d/r_rendercmds.h>
#include <sound/snd_public.h>

const float cg_perturbations[131][2] =
{
  { -0.56355101, -0.0044300002 },
  { -0.28206199, -0.75793302 },
  { 0.41304699, 0.24424601 },
  { 0.52789497, -0.72389698 },
  { -0.329777, 0.66979998 },
  { -0.39424801, -0.76309001 },
  { 0.12620699, 0.497769 },
  { 0.0049859998, -0.01413 },
  { 0.55991298, 0.112825 },
  { -0.33308899, -0.57328302 },
  { 0.33540401, -0.107176 },
  { -0.56906003, -0.21314099 },
  { -0.166676, 0.78508401 },
  { 0.29959199, 0.037593 },
  { -0.51686698, 0.510759 },
  { 0.138009, 0.034823 },
  { -0.156167, 0.82904798 },
  { -0.99945801, 0.020316999 },
  { 0.30002901, 0.25294399 },
  { 0.030215001, -0.29573199 },
  { -0.91736197, -0.050710998 },
  { 0.044176999, -0.26928899 },
  { 0.58842403, 0.36257699 },
  { -0.379913, 0.619214 },
  { 0.204432, -0.019423001 },
  { 0.018499, 0.468079 },
  { 0.91618699, -0.247878 },
  { 0.0037990001, 0.10821 },
  { 0.057363, 0.60623997 },
  { 0.324595, 0.158733 },
  { -0.130529, -0.18338799 },
  { 0.71567202, -0.36385801 },
  { 0.984258, 0.106096 },
  { -0.0033130001, 0.34553501 },
  { -0.320351, -0.57393599 },
  { 0.063455001, -0.003239 },
  { -0.57017303, -0.75931299 },
  { 0.106456, 0.28372601 },
  { -0.668163, 0.142388 },
  { -0.50111902, -0.72000599 },
  { -0.253281, 0.524032 },
  { -0.064084001, -0.165943 },
  { -0.194672, 0.43355 },
  { -0.2818, -0.41774401 },
  { 0.045786001, 0.40298599 },
  { 0.105064, -0.55893701 },
  { 0.312244, 0.68831801 },
  { -0.26329401, -0.25681099 },
  { 0.65918601, 0.070671998 },
  { 0.093625002, -0.046812002 },
  { -0.87502003, 0.28850901 },
  { 0.32935899, 0.105941 },
  { -0.181309, 0.25986499 },
  { 0.26159701, -0.074069999 },
  { -0.29608199, 0.031858001 },
  { 0.038584001, 0.565947 },
  { -0.253445, -0.71786499 },
  { -0.211836, 0.336521 },
  { 0.89012301, 0.00495 },
  { -0.97982502, -0.17079 },
  { 0.045345999, 0.02224 },
  { -0.34579599, 0.52271199 },
  { 0.108525, 0.165424 },
  { -0.57279599, -0.47339901 },
  { 0.36860499, -0.86584401 },
  { 0.075571001, -0.327703 },
  { -0.466353, -0.56559402 },
  { -0.35883701, 0.61030197 },
  { 0.60388398, 0.44002301 },
  { 0.0024649999, -0.144449 },
  { -0.29491499, 0.79996997 },
  { -0.028347, -0.112071 },
  { -0.0094720004, 0.68606102 },
  { 0.071149997, 0.01991 },
  { 0.96269, 0.024925999 },
  { 0.30920801, 0.87154901 },
  { -0.123782, -0.31230101 },
  { -0.43305501, -0.89598101 },
  { 0.96249503, -0.26377699 },
  { -0.51146001, -0.359478 },
  { -0.044013001, 0.02021 },
  { -0.10934, -0.76122999 },
  { 0.171003, -0.107461 },
  { 0.41891199, 0.435294 },
  { 0.44494, -0.139643 },
  { 0.518574, 0.36596501 },
  { -0.50699699, 0.65559697 },
  { 0.51052499, 0.50896102 },
  { -0.29617301, -0.67583698 },
  { 0.85133201, 0.307192 },
  { -0.0084739998, -0.18874399 },
  { 0.55270302, 0.427086 },
  { 0.080334, -0.0028049999 },
  { 0.035656001, 0.610991 },
  { 0.77059299, 0.39887401 },
  { -0.52213699, 0.32436201 },
  { 0.0060450002, 0.042787999 },
  { 0.482456, 0.84899402 },
  { 0.22605801, -0.522367 },
  { -0.67460603, -0.54781401 },
  { -0.441998, 0.59884 },
  { -0.183957, -0.27023399 },
  { 0.51885003, 0.63494599 },
  { 0.43038601, 0.125257 },
  { -0.185496, -0.26445901 },
  { 0.02369, 0.312978 },
  { -0.444287, 0.84992802 },
  { 0.291978, -0.89767897 },
  { -0.045825999, -0.047127999 },
  { -0.114246, 0.51197499 },
  { 0.73813301, 0.60766703 },
  { -0.78688902, -0.38405699 },
  { 0.18299299, 0.265086 },
  { -0.39945, -0.30903101 },
  { -0.48289499, 0.26566201 },
  { 0.059671, 0.097759999 },
  { 0.79317403, -0.015972 },
  { 0.201658, 0.49244499 },
  { -0.707371, -0.02619 },
  { -0.32088199, 0.37228 },
  { 0.57281297, -0.53725499 },
  { 0.33761901, 0.116293 },
  { -0.60653698, 0.173373 },
  { -0.166593, -0.33511201 },
  { -0.58399302, 0.182916 },
  { -0.57351899, -0.623348 },
  { -0.39270699, 0.44947401 },
  { 0.151474, 0.84040099 },
  { -0.56355101, -0.0044300002 },
  { -0.28206199, -0.75793302 },
  { 0.41304699, 0.24424601 }
};



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
    const shellshock_parms_t* parms,
    int start,
    int duration)
{
    float v5; // [esp+14h] [ebp-20h]
    int dt; // [esp+24h] [ebp-10h]
    const ClientViewParams* view; // [esp+2Ch] [ebp-8h]
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
        dt = duration + start - cgArray[0].time;
        if (dt > 0)
        {
            screenBlendTime = parms->screenBlend.blurredEffectTime;
            if (dt < parms->screenBlend.blurredFadeTime)
            {
                v5 = (double)dt / (double)parms->screenBlend.blurredFadeTime * (double)screenBlendTime;
                screenBlendTime = (int)(v5 + 9.313225746154785e-10);
            }
            if (cgArray[0].shellshock.hasSavedScreen)
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
            cgArray[0].shellshock.hasSavedScreen = 1;
            return 1;
        }
        else
        {
            cgArray[0].shellshock.hasSavedScreen = 0;
            return 0;
        }
    }
    else
    {
        cgArray[0].shellshock.hasSavedScreen = 0;
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
    const shellshock_parms_t* parms,
    int start,
    int duration)
{
    int dt; // [esp+18h] [ebp-14h]
    float whiteFactor; // [esp+20h] [ebp-Ch]
    float whiteFactora; // [esp+20h] [ebp-Ch]
    float whiteFactorb; // [esp+20h] [ebp-Ch]
    const ClientViewParams* view; // [esp+24h] [ebp-8h]
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
        dt = duration + start - cgArray[0].time;
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
            if (cgArray[0].shellshock.hasSavedScreen)
            {
                view = CG_GetLocalClientViewParams(localClientNum);
                R_AddCmdBlendSavedScreenShockFlashed(whiteFactorb, grabFactorb, view->x, view->y, view->width, view->height);
            }
            else
            {
                SaveScreenToBuffer(localClientNum);
            }
            cgArray[0].shellshock.hasSavedScreen = 1;
            return 1;
        }
        else
        {
            cgArray[0].shellshock.hasSavedScreen = 0;
            return 0;
        }
    }
    else
    {
        cgArray[0].shellshock.hasSavedScreen = 0;
        return 0;
    }
}

double __cdecl BlendSmooth(float percent)
{
    float v3; // [esp+Ch] [ebp-Ch]
    float sin; // [esp+14h] [ebp-4h]

    v3 = (percent - 0.5) * 3.141592741012573;
    sin = sinf(v3);
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
    time = cgArray[0].time - start;
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
    if (cgArray[0].shellshock.loopEndTime)
    {
        cgArray[0].shellshock.loopEndTime = 0;
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
    cgArray[0].shellshock.sensitivity = 1.0;
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

    cgArray[0].shellshock.viewDelta[0] = 0.0;
    cgArray[0].shellshock.viewDelta[1] = 0.0;
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
    cgArray[0].shellshock.hasSavedScreen = 0;
}

void __cdecl UpdateShellShockSound(int localClientNum, const shellshock_parms_t* parms, int time, int duration)
{
    const snd_alias_t* v4; // eax
    const snd_alias_t* v5; // eax
    int wetlevel; // [esp+10h] [ebp-3Ch]
    snd_alias_t* alias1; // [esp+30h] [ebp-1Ch]
    snd_alias_t* alias0; // [esp+34h] [ebp-18h]
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
        end = parms->sound.loopEndDelay + duration + cgArray[0].time - time;
        if (cgArray[0].time >= end)
        {
            if (end != cgArray[0].shellshock.loopEndTime)
            {
                cgArray[0].shellshock.loopEndTime = end;
                wetlevel = cgArray[0].time - end;
                v5 = CL_PickSoundAlias(parms->sound.end);
                SND_PlaySoundAlias(v5, (SndEntHandle)1023, vec3_origin, wetlevel, SASYS_CGAME);
            }
        }
        else if (cgArray[0].shellshock.loopEndTime)
        {
            cgArray[0].shellshock.loopEndTime = 0;
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
        cgArray[0].shellshock.sensitivity = parms->lookControl.mouseSensitivity;
        CL_CapTurnRate(localClientNum, parms->lookControl.maxPitchSpeed, parms->lookControl.maxYawSpeed);
    }
    else
    {
        cgArray[0].shellshock.sensitivity = (parms->lookControl.mouseSensitivity - 1.0) * fade + 1.0;
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
        perturb = cg_perturbations[(base + 61 * duration) & 0x7F];
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);

        cgArray[0].shellshock.viewDelta[0] = CubicInterpolate(t, *perturb, perturb[2], perturb[4], perturb[6]) * radius;
        cgArray[0].shellshock.viewDelta[1] = CubicInterpolate(t, perturb[1], perturb[3], perturb[5], perturb[7]) * radius;
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
    return cgArray[0].shellshock.duration + cgArray[0].shellshock.startTime - cgArray[0].time > 0
        && cgArray[0].shellshock.parms->screenBlend.type != SHELLSHOCK_VIEWTYPE_BLURRED;
}

