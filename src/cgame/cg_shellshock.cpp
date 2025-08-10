#include "cg_local.h"
#include "cg_public.h"

#include <gfx_d3d/r_rendercmds.h>
#include <sound/snd_public.h>

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#include <cgame_mp/cg_local_mp.h>
#elif KISAK_SP
#include <cgame/cg_main.h>
#include "cg_view.h"
#endif

const float cg_perturbations[131][2] =
{
  { -0.56355101f, -0.0044300002f },
  { -0.28206199f, -0.75793302f },
  { 0.41304699f, 0.24424601f },
  { 0.52789497f, -0.72389698f },
  { -0.329777f, 0.66979998f },
  { -0.39424801f, -0.76309001f },
  { 0.12620699f, 0.497769f },
  { 0.0049859998f, -0.01413f },
  { 0.55991298f, 0.112825f },
  { -0.33308899f, -0.57328302f },
  { 0.33540401f, -0.107176f },
  { -0.56906003f, -0.21314099f },
  { -0.166676f, 0.78508401f },
  { 0.29959199f, 0.037593f },
  { -0.51686698f, 0.510759f },
  { 0.138009f, 0.034823f },
  { -0.156167f, 0.82904798f },
  { -0.99945801f, 0.020316999f },
  { 0.30002901f, 0.25294399f },
  { 0.030215001f, -0.29573199f },
  { -0.91736197f, -0.050710998f },
  { 0.044176999f, -0.26928899f },
  { 0.58842403f, 0.36257699f },
  { -0.379913f, 0.619214f },
  { 0.204432f, -0.019423001f },
  { 0.018499f, 0.468079f },
  { 0.91618699f, -0.247878f },
  { 0.0037990001f, 0.10821f },
  { 0.057363f, 0.60623997f },
  { 0.324595f, 0.158733f },
  { -0.130529f, -0.18338799f },
  { 0.71567202f, -0.36385801f },
  { 0.984258f, 0.106096f },
  { -0.0033130001f, 0.34553501f },
  { -0.320351f, -0.57393599f },
  { 0.063455001f, -0.003239f },
  { -0.57017303f, -0.75931299f },
  { 0.106456f, 0.28372601f },
  { -0.668163f, 0.142388f },
  { -0.50111902f, -0.72000599f },
  { -0.253281f, 0.524032f },
  { -0.064084001f, -0.165943f },
  { -0.194672f, 0.43355f },
  { -0.2818f, -0.41774401f },
  { 0.045786001f, 0.40298599f },
  { 0.105064f, -0.55893701f },
  { 0.312244f, 0.68831801f },
  { -0.26329401f, -0.25681099f },
  { 0.65918601f, 0.070671998f },
  { 0.093625002f, -0.046812002f },
  { -0.87502003f, 0.28850901f },
  { 0.32935899f, 0.105941f },
  { -0.181309f, 0.25986499f },
  { 0.26159701f, -0.074069999f },
  { -0.29608199f, 0.031858001f },
  { 0.038584001f, 0.565947f },
  { -0.253445f, -0.71786499f },
  { -0.211836f, 0.336521f },
  { 0.89012301f, 0.00495f },
  { -0.97982502f, -0.17079f },
  { 0.045345999f, 0.02224f },
  { -0.34579599f, 0.52271199f },
  { 0.108525f, 0.165424f },
  { -0.57279599f, -0.47339901f },
  { 0.36860499f, -0.86584401f },
  { 0.075571001f, -0.327703f },
  { -0.466353f, -0.56559402f },
  { -0.35883701f, 0.61030197f },
  { 0.60388398f, 0.44002301f },
  { 0.0024649999f, -0.144449f },
  { -0.29491499f, 0.79996997f },
  { -0.028347f, -0.112071f },
  { -0.0094720004f, 0.68606102f },
  { 0.071149997f, 0.01991f },
  { 0.96269f, 0.024925999f },
  { 0.30920801f, 0.87154901f },
  { -0.123782f, -0.31230101f },
  { -0.43305501f, -0.89598101f },
  { 0.96249503f, -0.26377699f },
  { -0.51146001f, -0.359478f },
  { -0.044013001f, 0.02021f },
  { -0.10934f, -0.76122999f },
  { 0.171003f, -0.107461f },
  { 0.41891199f, 0.435294f },
  { 0.44494f, -0.139643f },
  { 0.518574f, 0.36596501f },
  { -0.50699699f, 0.65559697f },
  { 0.51052499f, 0.50896102f },
  { -0.29617301f, -0.67583698f },
  { 0.85133201f, 0.307192f },
  { -0.0084739998f, -0.18874399f },
  { 0.55270302f, 0.427086f },
  { 0.080334f, -0.0028049999f },
  { 0.035656001f, 0.610991f },
  { 0.77059299f, 0.39887401f },
  { -0.52213699f, 0.32436201f },
  { 0.0060450002f, 0.042787999f },
  { 0.482456f, 0.84899402f },
  { 0.22605801f, -0.522367f },
  { -0.67460603f, -0.54781401f },
  { -0.441998f, 0.59884f },
  { -0.183957f, -0.27023399f },
  { 0.51885003f, 0.63494599f },
  { 0.43038601f, 0.125257f },
  { -0.185496f, -0.26445901f },
  { 0.02369f, 0.312978f },
  { -0.444287f, 0.84992802f },
  { 0.291978f, -0.89767897f },
  { -0.045825999f, -0.047127999f },
  { -0.114246f, 0.51197499f },
  { 0.73813301f, 0.60766703f },
  { -0.78688902f, -0.38405699f },
  { 0.18299299f, 0.265086f },
  { -0.39945f, -0.30903101f },
  { -0.48289499f, 0.26566201f },
  { 0.059671f, 0.097759999f },
  { 0.79317403f, -0.015972f },
  { 0.201658f, 0.49244499f },
  { -0.707371f, -0.02619f },
  { -0.32088199f, 0.37228f },
  { 0.57281297f, -0.53725499f },
  { 0.33761901f, 0.116293f },
  { -0.60653698f, 0.173373f },
  { -0.166593f, -0.33511201f },
  { -0.58399302f, 0.182916f },
  { -0.57351899f, -0.623348f },
  { -0.39270699f, 0.44947401f },
  { 0.151474f, 0.84040099f },
  { -0.56355101f, -0.0044300002f },
  { -0.28206199f, -0.75793302f },
  { 0.41304699f, 0.24424601f }
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

int32_t __cdecl CG_DrawShellShockSavedScreenBlendBlurred(
    int32_t localClientNum,
    const shellshock_parms_t* parms,
    int32_t start,
    int32_t duration)
{
    float v5; // [esp+14h] [ebp-20h]
    int32_t dt; // [esp+24h] [ebp-10h]
    const ClientViewParams* view; // [esp+2Ch] [ebp-8h]
    int32_t screenBlendTime; // [esp+30h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (start && duration > 0)
    {
        dt = duration + start - cgameGlob->time;
        if (dt > 0)
        {
            screenBlendTime = parms->screenBlend.blurredEffectTime;
            if (dt < parms->screenBlend.blurredFadeTime)
            {
                screenBlendTime = (int)(dt / parms->screenBlend.blurredFadeTime * screenBlendTime);
            }
            if (cgameGlob->shellshock.hasSavedScreen)
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
            cgameGlob->shellshock.hasSavedScreen = 1;
            return 1;
        }
        else
        {
            cgameGlob->shellshock.hasSavedScreen = 0;
            return 0;
        }
    }
    else
    {
        cgameGlob->shellshock.hasSavedScreen = 0;
        return 0;
    }
}

void __cdecl SaveScreenToBuffer(int32_t localClientNum)
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
int32_t __cdecl CG_DrawShellShockSavedScreenBlendFlashed(
    int32_t localClientNum,
    const shellshock_parms_t* parms,
    int32_t start,
    int32_t duration)
{
    int32_t dt; // [esp+18h] [ebp-14h]
    float whiteFactor; // [esp+20h] [ebp-Ch]
    float whiteFactora; // [esp+20h] [ebp-Ch]
    float whiteFactorb; // [esp+20h] [ebp-Ch]
    const ClientViewParams* view; // [esp+24h] [ebp-8h]
    float grabFactor; // [esp+28h] [ebp-4h]
    float grabFactora; // [esp+28h] [ebp-4h]
    float grabFactorb; // [esp+28h] [ebp-4h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (start && duration > 0)
    {
        dt = duration + start - cgameGlob->time;
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
            if (cgameGlob->shellshock.hasSavedScreen)
            {
                view = CG_GetLocalClientViewParams(localClientNum);
                R_AddCmdBlendSavedScreenShockFlashed(whiteFactorb, grabFactorb, view->x, view->y, view->width, view->height);
            }
            else
            {
                SaveScreenToBuffer(localClientNum);
            }
            cgameGlob->shellshock.hasSavedScreen = 1;
            return 1;
        }
        else
        {
            cgameGlob->shellshock.hasSavedScreen = 0;
            return 0;
        }
    }
    else
    {
        cgameGlob->shellshock.hasSavedScreen = 0;
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

void __cdecl CG_UpdateShellShock(int32_t localClientNum, const shellshock_parms_t *parms, int32_t start, int32_t duration)
{
    int32_t time;

    time = CG_GetLocalClientGlobals(localClientNum)->time - start;

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

void __cdecl EndShellShock(int32_t localClientNum)
{
    EndShellShockSound(localClientNum);
    EndShellShockLookControl(localClientNum);
    EndShellShockCamera(localClientNum);
    EndShellShockScreen(localClientNum);
}

void __cdecl EndShellShockSound(int32_t localClientNum)
{
    snd_alias_t *alias; // [esp+10h] [ebp-4h]
    cg_s *cgameGlob;

    SND_DeactivateChannelVolumes(3, 0);
    SND_DeactivateEnvironmentEffects(2, 0);

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->shellshock.loopEndTime)
    {
        cgameGlob->shellshock.loopEndTime = 0;
        alias = CL_PickSoundAlias("shellshock_end_abort");
        SND_PlaySoundAlias(alias, (SndEntHandle)ENTITYNUM_NONE, vec3_origin, 0, SASYS_CGAME);
    }
}

void __cdecl EndShellShockLookControl(int32_t localClientNum)
{
    CG_GetLocalClientGlobals(localClientNum)->shellshock.sensitivity = 1.0f;
    CL_CapTurnRate(localClientNum, 0.0f, 0.0f);
}

void __cdecl EndShellShockCamera(int32_t localClientNum)
{
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    cgameGlob->shellshock.viewDelta[0] = 0.0;
    cgameGlob->shellshock.viewDelta[1] = 0.0;
}

void __cdecl EndShellShockScreen(int32_t localClientNum)
{
    CG_GetLocalClientGlobals(localClientNum)->shellshock.hasSavedScreen = 0;
}

void __cdecl UpdateShellShockSound(int32_t localClientNum, const shellshock_parms_t* parms, int32_t time, int32_t duration)
{
    const snd_alias_t* v4; // eax
    const snd_alias_t* v5; // eax
    int32_t wetlevel; // [esp+10h] [ebp-3Ch]
    snd_alias_t* alias1; // [esp+30h] [ebp-1Ch]
    snd_alias_t* alias0; // [esp+34h] [ebp-18h]
    int32_t dt; // [esp+38h] [ebp-14h]
    int32_t dta; // [esp+38h] [ebp-14h]
    float fade; // [esp+3Ch] [ebp-10h]
    int32_t end; // [esp+44h] [ebp-8h]
    cg_s *cgameGlob;

    iassert(parms);
    iassert(time >= 0);
    iassert(duration >= 0);

    if (parms->sound.affect)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
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
            SND_PlayBlendedSoundAliases(alias0, alias1, fade, 1.0, (SndEntHandle)ENTITYNUM_NONE, vec3_origin, 0, SASYS_CGAME);
        }
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\cgame\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        end = parms->sound.loopEndDelay + duration + cgameGlob->time - time;
        if (cgameGlob->time >= end)
        {
            if (end != cgameGlob->shellshock.loopEndTime)
            {
                cgameGlob->shellshock.loopEndTime = end;
                wetlevel = cgameGlob->time - end;
                v5 = CL_PickSoundAlias(parms->sound.end);
                SND_PlaySoundAlias(v5, (SndEntHandle)ENTITYNUM_NONE, vec3_origin, wetlevel, SASYS_CGAME);
            }
        }
        else if (cgameGlob->shellshock.loopEndTime)
        {
            cgameGlob->shellshock.loopEndTime = 0;
            v4 = CL_PickSoundAlias(parms->sound.endAbort);
            SND_PlaySoundAlias(v4, (SndEntHandle)ENTITYNUM_NONE, vec3_origin, 0, SASYS_CGAME);
        }
    }
    else
    {
        EndShellShockSound(localClientNum);
    }
}

void __cdecl UpdateShellShockLookControl(int32_t localClientNum, const shellshock_parms_t *parms, int32_t time, int32_t duration)
{
    float maxPitchSpeed; // [esp+8h] [ebp-14h]
    float maxYawSpeed; // [esp+Ch] [ebp-10h]
    float fade; // [esp+14h] [ebp-8h]
    cg_s *cgameGlob;

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

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (fade == 1.0f)
    {
        cgameGlob->shellshock.sensitivity = parms->lookControl.mouseSensitivity;
        CL_CapTurnRate(localClientNum, parms->lookControl.maxPitchSpeed, parms->lookControl.maxYawSpeed);
    }
    else
    {
        cgameGlob->shellshock.sensitivity = (parms->lookControl.mouseSensitivity - 1.0) * fade + 1.0;
        maxYawSpeed = parms->lookControl.maxYawSpeed / fade;
        maxPitchSpeed = parms->lookControl.maxPitchSpeed / fade;
        CL_CapTurnRate(localClientNum, maxPitchSpeed, maxYawSpeed);
    }
}

void __cdecl UpdateShellShockCamera(int32_t localClientNum, const shellshock_parms_t *parms, int32_t time, int32_t duration)
{
    int32_t dt; // [esp+20h] [ebp-20h]
    float ta; // [esp+24h] [ebp-1Ch]
    float t; // [esp+24h] [ebp-1Ch]
    float radius; // [esp+2Ch] [ebp-14h]
    const float *perturb; // [esp+30h] [ebp-10h]
    int32_t base; // [esp+38h] [ebp-8h]
    float scale; // [esp+3Ch] [ebp-4h]
    float scalea; // [esp+3Ch] [ebp-4h]
    cg_s *cgameGlob;

    dt = duration - time;
    if (duration - time > 0)
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        iassert(parms);
        scale = 1.0;
        if (dt < parms->view.fadeTime)
            scale = (double)dt / (double)parms->view.fadeTime;
        scalea = (3.0 - scale * 2.0) * scale * scale;
        radius = parms->view.kickRadius * scalea;
        ta = (double)time * parms->view.kickRate;
        base = (int)(ta - 0.4999999990686774);
        t = ta - (double)base;
        perturb = cg_perturbations[(base + 61 * duration) & 0x7F];

        cgameGlob->shellshock.viewDelta[0] = CubicInterpolate(t, *perturb, perturb[2], perturb[4], perturb[6]) * radius;
        cgameGlob->shellshock.viewDelta[1] = CubicInterpolate(t, perturb[1], perturb[3], perturb[5], perturb[7]) * radius;
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

void __cdecl CG_StartShellShock(cg_s *cgameGlob, const shellshock_parms_t *parms, int32_t start, int32_t duration)
{
    cgameGlob->shellshock.parms = parms;
    cgameGlob->shellshock.startTime = start;
    cgameGlob->shellshock.duration = duration;
}

bool __cdecl CG_Flashbanged(int32_t localClientNum)
{
    const cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    return cgameGlob->shellshock.duration + cgameGlob->shellshock.startTime - cgameGlob->time > 0
        && cgameGlob->shellshock.parms->screenBlend.type != SHELLSHOCK_VIEWTYPE_BLURRED;
}

