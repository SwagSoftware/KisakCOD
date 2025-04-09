#include "client_mp.h"
#include <qcommon/mem_track.h>
#include <universal/com_memory.h>
#include <client/client.h>
#include <aim_assist/aim_assist.h>
#include <win32/win_local.h>
#include <qcommon/cmd.h>


const dvar_t *cl_stanceHoldTime;
const dvar_t *cl_analog_attack_threshold;
const dvar_t *cl_yawspeed;
const dvar_t *cl_upspeed;
const dvar_t *cl_sidespeed;
const dvar_t *cl_bypassMouseInput;
const dvar_t *cl_pitchspeed;
const dvar_t *cl_forwardspeed;
const dvar_t *cl_anglespeedkey;

kbutton_t playersKb[1][30];

void __cdecl TRACK_cl_input()
{
    track_static_alloc_internal(playersKb, 600, "playersKb", 10);
}

void __cdecl CL_SetStance(int localClientNum, StanceState stance)
{
    if (!playersKb[localClientNum][24].active && !playersKb[localClientNum][11].active)
        CL_GetLocalClientGlobals(localClientNum)->stance = stance;
}

void __cdecl IN_CenterView()
{
    clientActive_t *LocalClientGlobals; // edx

    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        LocalClientGlobals->viewangles[0] = -LocalClientGlobals->snap.ps.delta_angles[0];
    }
}

bool __cdecl IN_IsTalkKeyHeld()
{
    return playersKb[0][29].active;
}

void __cdecl CL_UpdateCmdButton(int localClientNum, int *cmdButtons, int kbButton, int buttonFlag)
{
    kbutton_t *kb; // [esp+0h] [ebp-4h]

    kb = playersKb[localClientNum];
    if (kb[kbButton].active || kb[kbButton].wasPressed)
        *cmdButtons |= buttonFlag;
    kb[kbButton].wasPressed = 0;
}

void __cdecl CL_WritePacket(int localClientNum)
{
    int v1; // eax
    int v2; // eax
    unsigned __int8 *v3; // ecx
    unsigned __int8 *v4; // edx
    const char *v5; // eax
    int v6; // [esp-Ch] [ebp-8C0h]
    char *v7; // [esp+8h] [ebp-8ACh]
    const char *v8; // [esp+Ch] [ebp-8A8h]
    usercmd_s nullcmd; // [esp+2Ch] [ebp-888h] BYREF
    clientActive_t *LocalClientGlobals; // [esp+4Ch] [ebp-868h]
    usercmd_s *oldcmd; // [esp+58h] [ebp-85Ch]
    int compressedSize; // [esp+5Ch] [ebp-858h]
    connstate_t connstate; // [esp+60h] [ebp-854h]
    unsigned __int8 data[2052]; // [esp+64h] [ebp-850h] BYREF
    unsigned __int8 (*compressedBuf)[2048]; // [esp+86Ch] [ebp-48h]
    msg_t buf; // [esp+870h] [ebp-44h] BYREF
    int oldPacketNum; // [esp+898h] [ebp-1Ch]
    clientConnection_t *clc; // [esp+89Ch] [ebp-18h]
    int key; // [esp+8A0h] [ebp-14h]
    int i; // [esp+8A4h] [ebp-10h]
    usercmd_s *cmd; // [esp+8A8h] [ebp-Ch]
    int count; // [esp+8ACh] [ebp-8h]
    int packetNum; // [esp+8B0h] [ebp-4h]

    LargeLocal compressedBuf_large_local(2048);

    //compressedBuf = (unsigned __int8 (*)[2048])LargeLocal::GetBuf(&compressedBuf_large_local);
    compressedBuf = (unsigned __int8 (*)[2048])compressedBuf_large_local.GetBuf();
    clc = CL_GetLocalClientConnection(localClientNum);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    connstate = clientUIActives[0].connectionState;
    if (clc->demoplaying || connstate == CA_CINEMATIC || connstate == CA_LOGO || connstate == CA_SENDINGSTATS)
    {
        //LargeLocal::~LargeLocal(&compressedBuf_large_local);
        return;
    }
    else
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        MSG_SetDefaultUserCmd(&LocalClientGlobals->snap.ps, &nullcmd);
        oldcmd = &nullcmd;
        cmd = &nullcmd;
        memset(&buf, 0, sizeof(buf));
        MSG_Init(&buf, data, 2048);
        MSG_WriteByte(&buf, LocalClientGlobals->serverId);
        MSG_WriteLong(&buf, clc->serverMessageSequence);
        MSG_WriteLong(&buf, clc->serverCommandSequence);
        for (i = clc->reliableAcknowledge + 1; i <= clc->reliableSequence; ++i)
        {
            MSG_WriteBits(&buf, 2, 3u);
            MSG_WriteLong(&buf, i);
            MSG_WriteString(&buf, clc->reliableCommands[i & 0x7F]);
        }
        oldPacketNum = (clc->netchan.outgoingSequence - 1 - cl_packetdup->current.integer) & 0x1F;
        count = LocalClientGlobals->cmdNumber - LocalClientGlobals->outPackets[oldPacketNum].p_cmdNumber;
        if (count > 32)
        {
            count = 32;
            Com_Printf(14, "MAX_PACKET_USERCMDS\n");
        }
        if (count >= 1)
        {
            if (cl_showSend->current.enabled)
                Com_Printf(14, "(%i)", count);
            if (!clc->demowaiting
                && !cl_nodelta->current.enabled
                && LocalClientGlobals->snap.valid
                && clc->serverMessageSequence == LocalClientGlobals->snap.messageNum)
            {
                MSG_WriteBits(&buf, 0, 3u);
            }
            else
            {
                MSG_WriteBits(&buf, 1, 3u);
            }
            MSG_WriteByte(&buf, count);
            key = clc->checksumFeed;
            key ^= clc->serverMessageSequence;
            v1 = Com_HashKey(clc->serverCommands[clc->serverCommandSequence & 0x7F], 32);
            key ^= v1;
            if (cl_debugMessageKey->current.enabled)
            {
                v7 = &clc->serverCommands[clc->serverCommandSequence & 0x7F][1];
                v8 = clc->serverCommands[clc->serverCommandSequence & 0x7F];
                v6 = clc->serverCommandSequence & 0x7F;
                v2 = Com_HashKey(v8, 32);
                Com_Printf(
                    14,
                    "key:%i, checksumFeed:%i, messageAcknowledge:%i, Com_HashKey:%i, servercommand(%i):'%s', len:%i\n",
                    key,
                    clc->checksumFeed,
                    clc->serverMessageSequence,
                    v2,
                    v6,
                    v8,
                    &v8[strlen(v8) + 1] - v7);
            }
            for (i = 0; i < count; ++i)
            {
                cmd = &LocalClientGlobals->cmds[((unsigned __int8)LocalClientGlobals->cmdNumber - (_BYTE)count + (_BYTE)i + 1)
                    & 0x7F];
                MSG_WriteDeltaUsercmdKey(&buf, key, oldcmd, cmd);
                oldcmd = cmd;
            }
            MSG_WriteLong(&buf, LODWORD(LocalClientGlobals->cgameOrigin[0]));
            MSG_WriteLong(&buf, LODWORD(LocalClientGlobals->cgameOrigin[1]));
            MSG_WriteLong(&buf, LODWORD(LocalClientGlobals->cgameOrigin[2]));
            if ((unsigned int(LocalClientGlobals->cgameVelocity[0]) & 0x7F800000) == 0x7F800000
                || (unsigned int(LocalClientGlobals->cgameVelocity[1]) & 0x7F800000) == 0x7F800000
                || (unsigned int(LocalClientGlobals->cgameVelocity[2]) & 0x7F800000) == 0x7F800000)
            {
                MyAssertHandler(
                    ".\\client_mp\\cl_input.cpp",
                    2051,
                    0,
                    "%s",
                    "!IS_NAN((cl->cgameVelocity)[0]) && !IS_NAN((cl->cgameVelocity)[1]) && !IS_NAN((cl->cgameVelocity)[2])");
            }
            MSG_WriteLong(&buf, LocalClientGlobals->cgamePredictedDataServerTime);
            CL_SavePredictedOriginForServerTime(
                LocalClientGlobals,
                LocalClientGlobals->cgamePredictedDataServerTime,
                LocalClientGlobals->cgameOrigin,
                LocalClientGlobals->cgameVelocity,
                LocalClientGlobals->cgameViewangles,
                LocalClientGlobals->cgameBobCycle,
                LocalClientGlobals->cgameMovementDir);
        }
        MSG_WriteBits(&buf, 3, 3u);
        if (buf.cursize < 9)
            MyAssertHandler(".\\client_mp\\cl_input.cpp", 2063, 0, "%s", "buf.cursize >= CL_ENCODE_START");
        v3 = buf.data;
        v4 = (unsigned __int8 *)compressedBuf;
        *(unsigned int *)compressedBuf = *(unsigned int *)buf.data;
        *((unsigned int *)v4 + 1) = *((unsigned int *)v3 + 1);
        v4[8] = v3[8];
        if (buf.cursize > 0x800u)
            Com_Error(ERR_DROP, "Overflow compressed msg buf in CL_WritePacket()");
        compressedSize = MSG_WriteBitsCompress(
            0,
            (const unsigned __int8 *)buf.data + 9,
            &(*compressedBuf)[9],
            buf.cursize - 9)
            + 9;
        packetNum = clc->netchan.outgoingSequence & 0x1F;
        LocalClientGlobals->outPackets[packetNum].p_realtime = cls.realtime;
        LocalClientGlobals->outPackets[packetNum].p_serverTime = oldcmd->serverTime;
        LocalClientGlobals->outPackets[packetNum].p_cmdNumber = LocalClientGlobals->cmdNumber;
        clc->lastPacketSentTime = cls.realtime;
        if (cl_showSend->current.enabled)
        {
            v5 = NET_AdrToString(clc->netchan.remoteAddress);
            Com_Printf(14, "%i to %s\n", compressedSize, v5);
        }
        CL_Netchan_Transmit(&clc->netchan, (unsigned __int8 *)compressedBuf, compressedSize);
        while (clc->netchan.unsentFragments)
            CL_Netchan_TransmitNextFragment(&clc->netchan);
        //LargeLocal::~LargeLocal(&compressedBuf_large_local);
    }
}

void __cdecl CL_SendCmd(int localClientNum)
{
    if (CL_ReadyToSendPacket(localClientNum))
        CL_WritePacket(localClientNum);
}

bool __cdecl Sys_IsLANAddress(netadr_t adr);
bool __cdecl CL_ReadyToSendPacket(int localClientNum)
{
    int oldPacketNum; // [esp+Ch] [ebp-8h]
    clientConnection_t *clc; // [esp+10h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    if (clc->demoplaying)
        return 0;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == CA_CINEMATIC || clientUIActives[0].connectionState == CA_LOGO)
        return 0;
    if (cls.downloadTempName[0] && cls.realtime - clc->lastPacketSentTime < 50)
        return 0;
    if (clientUIActives[0].connectionState != CA_ACTIVE
        && clientUIActives[0].connectionState != CA_PRIMED
        && !cls.downloadTempName[0]
        && cls.realtime - clc->lastPacketSentTime < 1000)
    {
        return 0;
    }
    if (clc->netchan.remoteAddress.type == NA_LOOPBACK)
        return 1;
    if (Sys_IsLANAddress(clc->netchan.remoteAddress))
        return 1;
    oldPacketNum = ((unsigned __int8)clc->netchan.outgoingSequence - 1) & 0x1F;
    return cls.realtime - CL_GetLocalClientGlobals(localClientNum)->outPackets[oldPacketNum].p_realtime >= 1000 / cl_maxpackets->current.integer;
}

void __cdecl CL_CreateCmdsDuringConnection(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= CA_CONNECTED && clientUIActives[0].connectionState != CA_ACTIVE)
        CL_CreateNewCommands(localClientNum);
}

void __cdecl CL_CreateNewCommands(int localClientNum)
{
    usercmd_s result; // [esp+8h] [ebp-48h] BYREF
    _BYTE v2[32]; // [esp+28h] [ebp-28h] BYREF
    clientActive_t *LocalClientGlobals; // [esp+48h] [ebp-8h]
    int cmdNum; // [esp+4Ch] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= CA_PRIMED)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        cmdNum = ++LocalClientGlobals->cmdNumber & 0x7F;
        memcpy(v2, CL_CreateCmd(&result, localClientNum), sizeof(v2));
        memcpy(&LocalClientGlobals->cmds[cmdNum], v2, sizeof(LocalClientGlobals->cmds[cmdNum]));
    }
}

usercmd_s *__cdecl CL_CreateCmd(usercmd_s *result, int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+Ch] [ebp-34h]
    float oldAngles; // [esp+10h] [ebp-30h]
    usercmd_s cmd; // [esp+1Ch] [ebp-24h] BYREF

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    oldAngles = LocalClientGlobals->viewangles[0];
    CL_AdjustAngles(localClientNum);
    memset(&cmd, 0, sizeof(cmd));
    if (!Key_IsCatcherActive(localClientNum, 8) || !CG_HandleLocationSelectionInput(localClientNum, &cmd))
    {
        CL_CmdButtons(localClientNum, &cmd);
        CL_KeyMove(localClientNum, &cmd);
        CL_MouseMove(localClientNum, &cmd);
        if (LocalClientGlobals->viewangles[0] - oldAngles <= 90.0)
        {
            if (oldAngles - LocalClientGlobals->viewangles[0] > 90.0)
                LocalClientGlobals->viewangles[0] = oldAngles - 90.0;
        }
        else
        {
            LocalClientGlobals->viewangles[0] = oldAngles + 90.0;
        }
        if (CG_VehLocalClientDriving(localClientNum))
        {
            cmd.buttons &= ~1u;
            cmd.buttons &= ~0x800u;
            cmd.buttons &= 0xFFFF3FFF;
            cmd.buttons &= ~0x8000u;
        }
    }
    CL_FinishMove(localClientNum, &cmd);
    memcpy(result, &cmd, sizeof(usercmd_s));
    return result;
}

void __cdecl CL_AdjustAngles(int localClientNum)
{
    double v1; // st7
    float v2; // [esp+10h] [ebp-44h]
    float v3; // [esp+14h] [ebp-40h]
    float v4; // [esp+28h] [ebp-2Ch]
    float v5; // [esp+2Ch] [ebp-28h]
    float v6; // [esp+30h] [ebp-24h]
    float cgameMaxPitchSpeed; // [esp+34h] [ebp-20h]
    float value; // [esp+38h] [ebp-1Ch]
    float cgameMaxYawSpeed; // [esp+3Ch] [ebp-18h]
    clientActive_t *LocalClientGlobals; // [esp+40h] [ebp-14h]
    float max; // [esp+44h] [ebp-10h]
    float maxa; // [esp+44h] [ebp-10h]
    kbutton_t *kb; // [esp+48h] [ebp-Ch]
    float speed; // [esp+50h] [ebp-4h]

    kb = playersKb[localClientNum];
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ((LocalClientGlobals->snap.ps.pm_flags & 0x800) == 0)
    {
        if (kb[9].active)
            v1 = (double)cls.frametime * 0.001000000047497451 * cl_anglespeedkey->current.value;
        else
            v1 = (double)cls.frametime * 0.001000000047497451;
        speed = v1;
        if (!kb[8].active)
        {
            if (LocalClientGlobals->cgameMaxYawSpeed <= 0.0)
            {
                max = cl_yawspeed->current.value;
            }
            else
            {
                value = cl_yawspeed->current.value;
                cgameMaxYawSpeed = LocalClientGlobals->cgameMaxYawSpeed;
                v5 = cgameMaxYawSpeed - value;
                if (v5 < 0.0)
                    v4 = cgameMaxYawSpeed;
                else
                    v4 = value;
                max = v4;
            }
            LocalClientGlobals->viewangles[1] = LocalClientGlobals->viewangles[1] - CL_KeyState(kb + 1) * (speed * max);
            LocalClientGlobals->viewangles[1] = CL_KeyState(kb) * (speed * max) + LocalClientGlobals->viewangles[1];
        }
        if (LocalClientGlobals->cgameMaxPitchSpeed <= 0.0)
        {
            maxa = cl_pitchspeed->current.value;
        }
        else
        {
            v6 = cl_pitchspeed->current.value;
            cgameMaxPitchSpeed = LocalClientGlobals->cgameMaxPitchSpeed;
            v3 = cgameMaxPitchSpeed - v6;
            if (v3 < 0.0)
                v2 = cgameMaxPitchSpeed;
            else
                v2 = v6;
            maxa = v2;
        }
        LocalClientGlobals->viewangles[0] = LocalClientGlobals->viewangles[0] - CL_KeyState(kb + 4) * (speed * maxa);
        LocalClientGlobals->viewangles[0] = CL_KeyState(kb + 5) * (speed * maxa) + LocalClientGlobals->viewangles[0];
    }
}

double __cdecl CL_KeyState(kbutton_t *key)
{
    signed int msec; // [esp+Ch] [ebp-4h]

    msec = key->msec;
    key->msec = 0;
    if (key->active)
    {
        if (key->downtime)
            msec += com_frameTime - key->downtime;
        else
            msec = com_frameTime;
        key->downtime = com_frameTime;
    }
    if (msec <= 0)
        return 0.0;
    if (msec >= frame_msec)
        return 1.0;
    if (!frame_msec)
        MyAssertHandler(".\\client_mp\\cl_input.cpp", 270, 0, "%s", "frame_msec");
    return (float)((double)msec / (double)frame_msec);
}

void __cdecl CL_KeyMove(int localClientNum, usercmd_s *cmd)
{
    unsigned int v2; // ecx
    int v3; // ecx
    int sidea; // [esp+3Ch] [ebp-10h]
    int side; // [esp+3Ch] [ebp-10h]
    int sideb; // [esp+3Ch] [ebp-10h]
    kbutton_t *kb; // [esp+44h] [ebp-8h]
    int forwarda; // [esp+48h] [ebp-4h]
    int forward; // [esp+48h] [ebp-4h]

    CL_GetLocalClientGlobals(localClientNum);
    if (playersKb[localClientNum][24].active || playersKb[localClientNum][11].active)
    {
        if (playersKb[localClientNum][24].active)
        {
            cmd->buttons |= 0x100u;
            v2 = cmd->buttons & 0xFFFFFDFF;
        }
        else
        {
            cmd->buttons |= 0x200u;
            v2 = cmd->buttons & 0xFFFFFEFF;
        }
        cmd->buttons = v2;
        cmd->buttons |= 0x1000u;
    }
    else
    {
        CL_StanceButtonUpdate(localClientNum);
        CL_AddCurrentStanceToCmd(localClientNum, cmd);
    }
    kb = playersKb[localClientNum];
    if (kb[9].active == !CL_GetLocalClientGlobals(localClientNum)->usingAds)
        v3 = cmd->buttons | 0x800;
    else
        v3 = cmd->buttons & 0xFFFFF7FF;
    cmd->buttons = v3;
    sidea = (int)(CL_KeyState(kb + 7) * (double)127);
    side = sidea - (int)(CL_KeyState(kb + 6) * (double)127);
    forwarda = (int)(CL_KeyState(kb + 2) * (double)127);
    forward = forwarda - (int)(CL_KeyState(kb + 3) * (double)127);
    if (!kb[3].active)
    {
        if (kb[27].active || kb[27].wasPressed)
        {
            cmd->buttons |= 2u;
            kb[27].wasPressed = 0;
        }
        else
        {
            cmd->buttons &= ~2u;
        }
    }
    if (kb[8].active && (cmd->buttons & 2) == 0)
    {
        sideb = side + (int)(CL_KeyState(kb + 1) * (double)127);
        side = sideb - (int)(CL_KeyState(kb) * (double)127);
    }
    cmd->forwardmove = ClampChar(forward);
    cmd->rightmove = ClampChar(side);
}

void __cdecl CL_StanceButtonUpdate(int localClientNum)
{
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-4h]

    if (playersKb[localClientNum][24].active || playersKb[localClientNum][11].active)
        MyAssertHandler(".\\client_mp\\cl_input.cpp", 1116, 0, "%s", "!IN_IsTempStanceKeyActive( localClientNum )");
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if (LocalClientGlobals->stanceHeld
        && com_frameTime - LocalClientGlobals->stanceTime >= cl_stanceHoldTime->current.integer)
    {
        if (LocalClientGlobals->stancePosition == CL_STANCE_PRONE)
            LocalClientGlobals->stance = CL_STANCE_STAND;
        else
            LocalClientGlobals->stance = CL_STANCE_PRONE;
        LocalClientGlobals->stanceHeld = 0;
    }
}

void __cdecl CL_AddCurrentStanceToCmd(int localClientNum, usercmd_s *cmd)
{
    StanceState stance; // [esp+0h] [ebp-8h]
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    stance = LocalClientGlobals->stance;
    if (stance == CL_STANCE_CROUCH)
    {
        cmd->buttons |= 0x200u;
        cmd->buttons &= ~0x100u;
    }
    else if (stance == CL_STANCE_PRONE)
    {
        cmd->buttons |= 0x100u;
        cmd->buttons &= ~0x200u;
    }
    else
    {
        if (LocalClientGlobals->stance)
            MyAssertHandler(".\\client_mp\\cl_input.cpp", 1150, 0, "%s", "cl->stance == CL_STANCE_STAND");
        cmd->buttons &= ~0x100u;
        cmd->buttons &= ~0x200u;
    }
    cmd->buttons &= ~0x1000u;
}

void __cdecl CL_MouseMove(int localClientNum, usercmd_s *cmd)
{
    float v2; // [esp+10h] [ebp-D4h]
    float v3; // [esp+14h] [ebp-D0h]
    float v4; // [esp+18h] [ebp-CCh]
    float v5; // [esp+24h] [ebp-C0h]
    float v6; // [esp+28h] [ebp-BCh]
    float v7; // [esp+2Ch] [ebp-B8h]
    float v8; // [esp+40h] [ebp-A4h]
    float v9; // [esp+48h] [ebp-9Ch]
    float v10; // [esp+58h] [ebp-8Ch]
    float v11; // [esp+5Ch] [ebp-88h]
    float v12; // [esp+60h] [ebp-84h]
    float v13; // [esp+64h] [ebp-80h]
    float v14; // [esp+6Ch] [ebp-78h]
    float v15; // [esp+80h] [ebp-64h]
    clientActive_t *LocalClientGlobals; // [esp+84h] [ebp-60h]
    float rate; // [esp+88h] [ebp-5Ch]
    float delta; // [esp+8Ch] [ebp-58h]
    float deltaa; // [esp+8Ch] [ebp-58h]
    AimInput aimInput; // [esp+90h] [ebp-54h] BYREF
    kbutton_t *kb; // [esp+C0h] [ebp-24h]
    float mx; // [esp+C4h] [ebp-20h] BYREF
    float my; // [esp+C8h] [ebp-1Ch] BYREF
    AimOutput aimOutput; // [esp+CCh] [ebp-18h] BYREF
    float cap; // [esp+DCh] [ebp-8h]
    float accelSensitivity; // [esp+E0h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    CL_GetMouseMovement(LocalClientGlobals, &mx, &my);
    if (frame_msec)
    {
        v15 = my * my + mx * mx;
        v8 = sqrt(v15);
        rate = v8 / (double)frame_msec;
        accelSensitivity = rate * cl_mouseAccel->current.value + cl_sensitivity->current.value;
        accelSensitivity = accelSensitivity * LocalClientGlobals->cgameFOVSensitivityScale;
        if (rate != 0.0 && cl_showMouseRate->current.enabled)
            Com_Printf(14, "%f : %f\n", rate, accelSensitivity);
        if ((LocalClientGlobals->snap.ps.pm_flags & 0x800) == 0)
        {
            mx = mx * accelSensitivity;
            my = my * accelSensitivity;
            if (mx != 0.0 || my != 0.0)
            {
                kb = playersKb[localClientNum];
                if (kb[8].active)
                {
                    v14 = mx * m_side->current.value;
                    cmd->rightmove = ClampChar((int)(v14 + 9.313225746154785e-10) + cmd->rightmove);
                }
                else
                {
                    delta = m_yaw->current.value * mx;
                    if (LocalClientGlobals->cgameMaxYawSpeed > 0.0)
                    {
                        cap = (double)frame_msec * LocalClientGlobals->cgameMaxYawSpeed * 0.001000000047497451;
                        v7 = delta - cap;
                        if (v7 < 0.0)
                            v13 = delta;
                        else
                            v13 = cap;
                        v12 = -cap;
                        v6 = v12 - v13;
                        if (v6 < 0.0)
                            v5 = v13;
                        else
                            v5 = -cap;
                        delta = v5;
                    }
                    LocalClientGlobals->viewangles[1] = LocalClientGlobals->viewangles[1] - delta;
                }
                if ((kb[13].active || cl_freelook->current.enabled) && !kb[8].active)
                {
                    deltaa = m_pitch->current.value * my;
                    if (LocalClientGlobals->cgameMaxPitchSpeed > 0.0)
                    {
                        cap = (double)frame_msec * LocalClientGlobals->cgameMaxPitchSpeed * 0.001000000047497451;
                        v4 = deltaa - cap;
                        if (v4 < 0.0)
                            v11 = deltaa;
                        else
                            v11 = cap;
                        v10 = -cap;
                        v3 = v10 - v11;
                        if (v3 < 0.0)
                            v2 = v11;
                        else
                            v2 = -cap;
                        deltaa = v2;
                    }
                    LocalClientGlobals->viewangles[0] = LocalClientGlobals->viewangles[0] + deltaa;
                }
                else
                {
                    v9 = my * m_forward->current.value;
                    cmd->forwardmove = ClampChar(cmd->forwardmove - (int)(v9 + 9.313225746154785e-10));
                }
            }
            aimInput.deltaTime = (double)cls.frametime * 0.001000000047497451;
            aimInput.pitch = LocalClientGlobals->viewangles[0];
            aimInput.pitchAxis = 0.0;
            aimInput.pitchMax = 0.0;
            aimInput.yaw = LocalClientGlobals->viewangles[1];
            aimInput.yawAxis = 0.0;
            aimInput.yawMax = 0.0;
            aimInput.forwardAxis = 0.0;
            aimInput.rightAxis = 0.0;
            aimInput.buttons = cmd->buttons;
            aimInput.localClientNum = 0;
            aimInput.ps = CG_GetPredictedPlayerState(0);
            AimAssist_UpdateMouseInput(&aimInput, &aimOutput);
            LocalClientGlobals->viewangles[0] = aimOutput.pitch;
            LocalClientGlobals->viewangles[1] = aimOutput.yaw;
            cmd->meleeChargeYaw = aimOutput.meleeChargeYaw;
            cmd->meleeChargeDist = aimOutput.meleeChargeDist;
        }
    }
}

void __cdecl CL_GetMouseMovement(clientActive_t *cl, float *mx, float *my)
{
    double v3; // st7

    if (!mx)
        MyAssertHandler(".\\client_mp\\cl_input.cpp", 1463, 0, "%s", "mx");
    if (!my)
        MyAssertHandler(".\\client_mp\\cl_input.cpp", 1464, 0, "%s", "my");
    if (m_filter->current.enabled)
    {
        *mx = (double)(cl->mouseDx[1] + cl->mouseDx[0]) * 0.5;
        v3 = (double)(cl->mouseDy[1] + cl->mouseDy[0]) * 0.5;
    }
    else
    {
        *mx = (float)cl->mouseDx[cl->mouseIndex];
        v3 = (double)cl->mouseDy[cl->mouseIndex];
    }
    *my = v3;
    cl->mouseIndex ^= 1u;
    cl->mouseDx[cl->mouseIndex] = 0;
    cl->mouseDy[cl->mouseIndex] = 0;
}

void __cdecl CL_CmdButtons(int localClientNum, usercmd_s *cmd)
{
    bool v3; // [esp+4h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-Ch]

    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 14, 1);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 15, 0x2000);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 16, 0x4000);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 17, 0x8000);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 18, 4);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 19, 8);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 20, 16);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 21, 32);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 22, 64);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 23, 128);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 24, 256);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 25, 512);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 10, 1024);
    CL_UpdateCmdButton(localClientNum, &cmd->buttons, 26, 0x80000);
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    v3 = clientUIActives[0].keyCatchers && !cl_bypassMouseInput->current.enabled;
    if (v3 && UI_GetActiveMenu(localClientNum) != 10)
        cmd->buttons |= 0x100000u;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState > CA_CONNECTED)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        if (LocalClientGlobals->snap.ps.pm_type == 2
            || LocalClientGlobals->snap.ps.pm_type == 3
            || LocalClientGlobals->snap.ps.pm_type == 4)
        {
            CL_UpdateCmdButton(localClientNum, &cmd->buttons, 12, 1024);
        }
    }
    if (cmd->buttons >= 0x200000)
        MyAssertHandler(".\\client_mp\\cl_input.cpp", 1633, 0, "%s", "cmd->buttons < (1 << BUTTON_BIT_COUNT)");
}

void __cdecl CL_FinishMove(int localClientNum, usercmd_s *cmd)
{
    int serverTime; // [esp+0h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    cmd->weapon = LocalClientGlobals->cgameUserCmdWeapon;
    cmd->offHandIndex = LocalClientGlobals->cgameUserCmdOffHandIndex;
    if (LocalClientGlobals->serverTime - LocalClientGlobals->snap.serverTime > 5000)
        serverTime = LocalClientGlobals->snap.serverTime + 5000;
    else
        serverTime = LocalClientGlobals->serverTime;
    cmd->serverTime = serverTime;
    for (i = 0; i < 3; ++i)
        cmd->angles[i] = (unsigned __int16)(int)((LocalClientGlobals->viewangles[i] + LocalClientGlobals->cgameKickAngles[i])
            * 182.0444488525391);
    cmd->buttons |= LocalClientGlobals->cgameExtraButtons;
    LocalClientGlobals->cgameExtraButtons = 0;
    if (CG_VehLocalClientDriving(localClientNum))
    {
        cmd->angles[1] = (unsigned __int16)(int)(LocalClientGlobals->vehicleViewYaw * 182.0444488525391);
        cmd->angles[0] = (unsigned __int16)(int)(LocalClientGlobals->vehicleViewPitch * 182.0444488525391);
        cmd->angles[2] = 0;
    }
}

char __cdecl CG_HandleLocationSelectionInput(int localClientNum, usercmd_s *cmd)
{
    float v3; // [esp+4h] [ebp-54h]
    float v4; // [esp+18h] [ebp-40h]
    float v5; // [esp+28h] [ebp-30h]
    float v6; // [esp+2Ch] [ebp-2Ch]
    float v7; // [esp+30h] [ebp-28h]
    float v8; // [esp+34h] [ebp-24h]
    clientActive_t *LocalClientGlobals; // [esp+38h] [ebp-20h]
    float mapAspectRatio; // [esp+40h] [ebp-18h]
    float mx; // [esp+48h] [ebp-10h] BYREF
    float my; // [esp+4Ch] [ebp-Ch] BYREF
    float frametime; // [esp+50h] [ebp-8h]
    LocSelInputState locSelInputState; // [esp+54h] [ebp-4h]

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\../cgame_mp/cg_local_mp.h",
            1071,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.locationSelectionInfo)
    {
        CL_AddCurrentStanceToCmd(localClientNum, cmd);
        cmd->buttons |= 0x100000u;
        frametime = (double)cgArray[0].frametime * 0.001000000047497451;
        mapAspectRatio = cgArray[0].compassMapWorldSize[0] / cgArray[0].compassMapWorldSize[1];
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        CL_GetMouseMovement(LocalClientGlobals, &mx, &my);
        cgArray[0].selectedLocation[0] = mx * cg_mapLocationSelectionCursorSpeed->current.value * frametime * (float)0.1
            + cgArray[0].selectedLocation[0];
        cgArray[0].selectedLocation[1] = my
            * mapAspectRatio
            * cg_mapLocationSelectionCursorSpeed->current.value
            * frametime
            * (float)0.1
            + cgArray[0].selectedLocation[1];
        v8 = cgArray[0].selectedLocation[0];
        if (0.0 >= 1.0)
            MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 533, 0, "%s", "min < max");
        if (v8 >= 0.0)
        {
            if (v8 <= 1.0)
                v7 = v8;
            else
                v7 = 1.0;
        }
        else
        {
            v7 = 0.0;
        }
        cgArray[0].selectedLocation[0] = v7;
        v6 = cgArray[0].selectedLocation[1];
        if (0.0 >= 1.0)
            MyAssertHandler("c:\\trees\\cod3\\src\\universal\\com_math.h", 533, 0, "%s", "min < max");
        if (v6 >= 0.0)
        {
            if (v6 <= 1.0)
                v5 = v6;
            else
                v5 = 1.0;
        }
        else
        {
            v5 = 0.0;
        }
        cgArray[0].selectedLocation[1] = v5;
        locSelInputState = playerKeys[localClientNum].locSelInputState;
        if (locSelInputState == LOC_SEL_INPUT_CONFIRM)
        {
            cmd->buttons |= 0x10000u;
            v4 = cgArray[0].selectedLocation[0] * 255.0;
            cmd->selectedLocation[0] = (int)(v4 + 9.313225746154785e-10) + 0x80;
            v3 = cgArray[0].selectedLocation[1] * 255.0;
            cmd->selectedLocation[1] = (int)(v3 + 9.313225746154785e-10) + 0x80;
        }
        else if (locSelInputState == LOC_SEL_INPUT_CANCEL)
        {
            cmd->buttons |= 0x20000u;
        }
        return 1;
    }
    else
    {
        Key_RemoveCatcher(localClientNum, -9);
        return 0;
    }
}

void __cdecl CL_Input(int localClientNum)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == CA_ACTIVE)
    {
        IN_Frame();
        CL_CreateNewCommands(localClientNum);
    }
}

cmd_function_s IN_CenterView_VAR;
cmd_function_s IN_UpDown_VAR;
cmd_function_s IN_UpUp_VAR;
cmd_function_s IN_DownDown_VAR;
cmd_function_s IN_DownUp_VAR;
cmd_function_s IN_LeftDown_VAR;
cmd_function_s IN_LeftUp_VAR;
cmd_function_s IN_RightDown_VAR;
cmd_function_s IN_RightUp_VAR;
cmd_function_s IN_ForwardDown_VAR;
cmd_function_s IN_ForwardUp_VAR;
cmd_function_s IN_BackDown_VAR;
cmd_function_s IN_BackUp_VAR;
cmd_function_s IN_LookupDown_VAR;
cmd_function_s IN_LookupUp_VAR;
cmd_function_s IN_LookdownDown_VAR;
cmd_function_s IN_LookdownUp_VAR;
cmd_function_s IN_StrafeDown_VAR;
cmd_function_s IN_StrafeUp_VAR;
cmd_function_s IN_MoveleftDown_VAR;
cmd_function_s IN_MoveleftUp_VAR;
cmd_function_s IN_MoverightDown_VAR;
cmd_function_s IN_MoverightUp_VAR;
cmd_function_s IN_SpeedDown_VAR;
cmd_function_s IN_SpeedUp_VAR;
cmd_function_s IN_Attack_Down_VAR;
cmd_function_s IN_Attack_Up_VAR;
cmd_function_s IN_CenterView_VAR;
cmd_function_s IN_Melee_Down_VAR;
cmd_function_s IN_Melee_Up_VAR;
cmd_function_s IN_Breath_Down_VAR;
cmd_function_s IN_Breath_Up_VAR;
cmd_function_s IN_MeleeBreath_Down_VAR;
cmd_function_s IN_MeleeBreath_Up_VAR;
cmd_function_s IN_Frag_Down_VAR;
cmd_function_s IN_Frag_Up_VAR;
cmd_function_s IN_Smoke_Down_VAR;
cmd_function_s IN_Smoke_Up_VAR;
cmd_function_s IN_BreathSprint_Down_VAR;
cmd_function_s IN_BreathSprint_Up_VAR;
cmd_function_s IN_Activate_Down_VAR;
cmd_function_s IN_Activate_Up_VAR;
cmd_function_s IN_Reload_Down_VAR;
cmd_function_s IN_Reload_Up_VAR;
cmd_function_s IN_UseReload_Down_VAR;
cmd_function_s IN_UseReload_Up_VAR;
cmd_function_s IN_LeanLeft_Down_VAR;
cmd_function_s IN_LeanLeft_Up_VAR;
cmd_function_s IN_LeanRight_Down_VAR;
cmd_function_s IN_LeanRight_Up_VAR;
cmd_function_s IN_Prone_Down_VAR;
cmd_function_s IN_Prone_Up_VAR;
cmd_function_s IN_Stance_Down_VAR;
cmd_function_s IN_Stance_Up_VAR;
cmd_function_s IN_MLookDown_VAR;
cmd_function_s IN_MLookUp_VAR;
cmd_function_s IN_ToggleADS_VAR;
cmd_function_s IN_LeaveADS_VAR;
cmd_function_s IN_Throw_Down_VAR;
cmd_function_s IN_Throw_Up_VAR;
cmd_function_s IN_Speed_Throw_Down_VAR;
cmd_function_s IN_Speed_Throw_Up_VAR;
cmd_function_s IN_ToggleADS_Throw_Down_VAR;
cmd_function_s IN_ToggleADS_Throw_Up_VAR;
cmd_function_s IN_LowerStance_VAR;
cmd_function_s IN_RaiseStance_VAR;
cmd_function_s IN_ToggleCrouch_VAR;
cmd_function_s IN_ToggleProne_VAR;
cmd_function_s IN_GoProne_VAR;
cmd_function_s IN_GoCrouch_VAR;
cmd_function_s IN_GoStandDown_VAR;
cmd_function_s IN_GoStandUp_VAR;
cmd_function_s IN_TalkDown_VAR;
cmd_function_s IN_TalkUp_VAR;
cmd_function_s IN_SprintDown_VAR;
cmd_function_s IN_SprintUp_VAR;


void __cdecl CL_InitInput()
{
    DvarLimits min; // [esp+4h] [ebp-10h]

    Cmd_AddCommandInternal("centerview", IN_CenterView, &IN_CenterView_VAR);
    Cmd_AddCommandInternal("+moveup", IN_UpDown, &IN_UpDown_VAR);
    Cmd_AddCommandInternal("-moveup", IN_UpUp, &IN_UpUp_VAR);
    Cmd_AddCommandInternal("+movedown", IN_DownDown, &IN_DownDown_VAR);
    Cmd_AddCommandInternal("-movedown", IN_DownUp, &IN_DownUp_VAR);
    Cmd_AddCommandInternal("+left", IN_LeftDown, &IN_LeftDown_VAR);
    Cmd_AddCommandInternal("-left", IN_LeftUp, &IN_LeftUp_VAR);
    Cmd_AddCommandInternal("+right", IN_RightDown, &IN_RightDown_VAR);
    Cmd_AddCommandInternal("-right", IN_RightUp, &IN_RightUp_VAR);
    Cmd_AddCommandInternal("+forward", IN_ForwardDown, &IN_ForwardDown_VAR);
    Cmd_AddCommandInternal("-forward", IN_ForwardUp, &IN_ForwardUp_VAR);
    Cmd_AddCommandInternal("+back", IN_BackDown, &IN_BackDown_VAR);
    Cmd_AddCommandInternal("-back", IN_BackUp, &IN_BackUp_VAR);
    Cmd_AddCommandInternal("+lookup", IN_LookupDown, &IN_LookupDown_VAR);
    Cmd_AddCommandInternal("-lookup", IN_LookupUp, &IN_LookupUp_VAR);
    Cmd_AddCommandInternal("+lookdown", IN_LookdownDown, &IN_LookdownDown_VAR);
    Cmd_AddCommandInternal("-lookdown", IN_LookdownUp, &IN_LookdownUp_VAR);
    Cmd_AddCommandInternal("+strafe", IN_StrafeDown, &IN_StrafeDown_VAR);
    Cmd_AddCommandInternal("-strafe", IN_StrafeUp, &IN_StrafeUp_VAR);
    Cmd_AddCommandInternal("+moveleft", IN_MoveleftDown, &IN_MoveleftDown_VAR);
    Cmd_AddCommandInternal("-moveleft", IN_MoveleftUp, &IN_MoveleftUp_VAR);
    Cmd_AddCommandInternal("+moveright", IN_MoverightDown, &IN_MoverightDown_VAR);
    Cmd_AddCommandInternal("-moveright", IN_MoverightUp, &IN_MoverightUp_VAR);
    Cmd_AddCommandInternal("+speed", IN_SpeedDown, &IN_SpeedDown_VAR);
    Cmd_AddCommandInternal("-speed", IN_SpeedUp, &IN_SpeedUp_VAR);
    Cmd_AddCommandInternal("+attack", IN_Attack_Down, &IN_Attack_Down_VAR);
    Cmd_AddCommandInternal("-attack", IN_Attack_Up, &IN_Attack_Up_VAR);
    Cmd_AddCommandInternal("+melee", IN_Melee_Down, &IN_Melee_Down_VAR);
    Cmd_AddCommandInternal("-melee", IN_Melee_Up, &IN_Melee_Up_VAR);
    Cmd_AddCommandInternal("+holdbreath", IN_Breath_Down, &IN_Breath_Down_VAR);
    Cmd_AddCommandInternal("-holdbreath", IN_Breath_Up, &IN_Breath_Up_VAR);
    Cmd_AddCommandInternal("+melee_breath", IN_MeleeBreath_Down, &IN_MeleeBreath_Down_VAR);
    Cmd_AddCommandInternal("-melee_breath", IN_MeleeBreath_Up, &IN_MeleeBreath_Up_VAR);
    Cmd_AddCommandInternal("+frag", IN_Frag_Down, &IN_Frag_Down_VAR);
    Cmd_AddCommandInternal("-frag", IN_Frag_Up, &IN_Frag_Up_VAR);
    Cmd_AddCommandInternal("+smoke", IN_Smoke_Down, &IN_Smoke_Down_VAR);
    Cmd_AddCommandInternal("-smoke", IN_Smoke_Up, &IN_Smoke_Up_VAR);
    Cmd_AddCommandInternal("+breath_sprint", IN_BreathSprint_Down, &IN_BreathSprint_Down_VAR);
    Cmd_AddCommandInternal("-breath_sprint", IN_BreathSprint_Up, &IN_BreathSprint_Up_VAR);
    Cmd_AddCommandInternal("+activate", IN_Activate_Down, &IN_Activate_Down_VAR);
    Cmd_AddCommandInternal("-activate", IN_Activate_Up, &IN_Activate_Up_VAR);
    Cmd_AddCommandInternal("+reload", IN_Reload_Down, &IN_Reload_Down_VAR);
    Cmd_AddCommandInternal("-reload", IN_Reload_Up, &IN_Reload_Up_VAR);
    Cmd_AddCommandInternal("+usereload", IN_UseReload_Down, &IN_UseReload_Down_VAR);
    Cmd_AddCommandInternal("-usereload", IN_UseReload_Up, &IN_UseReload_Up_VAR);
    Cmd_AddCommandInternal("+leanleft", IN_LeanLeft_Down, &IN_LeanLeft_Down_VAR);
    Cmd_AddCommandInternal("-leanleft", IN_LeanLeft_Up, &IN_LeanLeft_Up_VAR);
    Cmd_AddCommandInternal("+leanright", IN_LeanRight_Down, &IN_LeanRight_Down_VAR);
    Cmd_AddCommandInternal("-leanright", IN_LeanRight_Up, &IN_LeanRight_Up_VAR);
    Cmd_AddCommandInternal("+prone", IN_Prone_Down, &IN_Prone_Down_VAR);
    Cmd_AddCommandInternal("-prone", IN_Prone_Up, &IN_Prone_Up_VAR);
    Cmd_AddCommandInternal("+stance", IN_Stance_Down, &IN_Stance_Down_VAR);
    Cmd_AddCommandInternal("-stance", IN_Stance_Up, &IN_Stance_Up_VAR);
    Cmd_AddCommandInternal("+mlook", IN_MLookDown, &IN_MLookDown_VAR);
    Cmd_AddCommandInternal("-mlook", IN_MLookUp, &IN_MLookUp_VAR);
    Cmd_AddCommandInternal("toggleads", IN_ToggleADS, &IN_ToggleADS_VAR);
    Cmd_AddCommandInternal("leaveads", IN_LeaveADS, &IN_LeaveADS_VAR);
    Cmd_AddCommandInternal("+throw", IN_Throw_Down, &IN_Throw_Down_VAR);
    Cmd_AddCommandInternal("-throw", IN_Throw_Up, &IN_Throw_Up_VAR);
    Cmd_AddCommandInternal("+speed_throw", IN_Speed_Throw_Down, &IN_Speed_Throw_Down_VAR);
    Cmd_AddCommandInternal("-speed_throw", IN_Speed_Throw_Up, &IN_Speed_Throw_Up_VAR);
    Cmd_AddCommandInternal("+toggleads_throw", IN_ToggleADS_Throw_Down, &IN_ToggleADS_Throw_Down_VAR);
    Cmd_AddCommandInternal("-toggleads_throw", IN_ToggleADS_Throw_Up, &IN_ToggleADS_Throw_Up_VAR);
    Cmd_AddCommandInternal("lowerstance", IN_LowerStance, &IN_LowerStance_VAR);
    Cmd_AddCommandInternal("raisestance", IN_RaiseStance, &IN_RaiseStance_VAR);
    Cmd_AddCommandInternal("togglecrouch", IN_ToggleCrouch, &IN_ToggleCrouch_VAR);
    Cmd_AddCommandInternal("toggleprone", IN_ToggleProne, &IN_ToggleProne_VAR);
    Cmd_AddCommandInternal("goprone", IN_GoProne, &IN_GoProne_VAR);
    Cmd_AddCommandInternal("gocrouch", IN_GoCrouch, &IN_GoCrouch_VAR);
    Cmd_AddCommandInternal("+gostand", IN_GoStandDown, &IN_GoStandDown_VAR);
    Cmd_AddCommandInternal("-gostand", IN_GoStandUp, &IN_GoStandUp_VAR);
    Cmd_AddCommandInternal("+talk", IN_TalkDown, &IN_TalkDown_VAR);
    Cmd_AddCommandInternal("-talk", IN_TalkUp, &IN_TalkUp_VAR);
    Cmd_AddCommandInternal("+sprint", IN_SprintDown, &IN_SprintDown_VAR);
    Cmd_AddCommandInternal("-sprint", IN_SprintUp, &IN_SprintUp_VAR);
    min.value.max = 1.0;
    min.value.min = 0.000099999997;
    cl_analog_attack_threshold = Dvar_RegisterFloat(
        "cl_analog_attack_threshold",
        0.80000001,
        min,
        0,
        "The threshold before firing");
    cl_stanceHoldTime = Dvar_RegisterInt(
        "cl_stanceHoldTime",
        300,
        (DvarLimits)0x3E800000000LL,
        0,
        "The time to hold the stance button before the player goes prone");
    cl_nodelta = Dvar_RegisterBool("cl_nodelta", 0, 0, "The server does not send snapshot deltas");
}

void __cdecl IN_MLookDown()
{
    playersKb[0][13].active = 1;
}

void __cdecl IN_MLookUp()
{
    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        playersKb[0][13].active = 0;
        if (!cl_freelook->current.enabled)
            IN_CenterView();
    }
}

void __cdecl IN_UpDown()
{
    StanceState stance; // [esp+0h] [ebp-10h]
    bool v1; // [esp+4h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-8h]

    IN_KeyDown(&playersKb[0][12]);
    v1 = playersKb[0][24].active || playersKb[0][11].active;
    if (!v1 && clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        stance = LocalClientGlobals->stance;
        if (stance == CL_STANCE_CROUCH)
        {
            LocalClientGlobals->stance = CL_STANCE_STAND;
        }
        else if (stance == CL_STANCE_PRONE)
        {
            LocalClientGlobals->stance = CL_STANCE_CROUCH;
        }
        else
        {
            if (LocalClientGlobals->stance)
                MyAssertHandler(".\\client_mp\\cl_input.cpp", 324, 1, "%s", "cl->stance == CL_STANCE_STAND");
            IN_KeyDown(&playersKb[0][10]);
        }
    }
}

void __cdecl IN_KeyDown(kbutton_t *b)
{
    const char *c; // [esp+0h] [ebp-8h]
    const char *ca; // [esp+0h] [ebp-8h]
    int k; // [esp+4h] [ebp-4h]

    c = Cmd_Argv(1);
    if (*c)
        k = atoi(c);
    else
        k = -1;
    if (k != b->down[0] && k != b->down[1])
    {
        if (b->down[0])
        {
            if (b->down[1])
            {
                Com_Printf(14, "Three keys down for a button!\n");
                return;
            }
            b->down[1] = k;
        }
        else
        {
            b->down[0] = k;
        }
        if (!b->active)
        {
            ca = Cmd_Argv(2);
            b->downtime = atoi(ca);
            b->active = 1;
            b->wasPressed = 1;
        }
    }
}

void __cdecl IN_UpUp()
{
    IN_KeyUp(&playersKb[0][12]);
    IN_KeyUp(&playersKb[0][10]);
}

void __cdecl IN_KeyUp(kbutton_t *b)
{
    unsigned int v1; // edx
    const char *c; // [esp+0h] [ebp-Ch]
    const char *ca; // [esp+0h] [ebp-Ch]
    unsigned int uptime; // [esp+4h] [ebp-8h]
    int k; // [esp+8h] [ebp-4h]

    c = Cmd_Argv(1);
    if (!*c)
    {
        b->down[1] = 0;
        b->down[0] = 0;
        b->active = 0;
        return;
    }
    k = atoi(c);
    if (b->down[0] == k)
    {
        b->down[0] = 0;
    }
    else
    {
        if (b->down[1] != k)
            return;
        b->down[1] = 0;
    }
    if (!b->down[0] && !b->down[1])
    {
        b->active = 0;
        ca = Cmd_Argv(2);
        uptime = atoi(ca);
        if (uptime)
            v1 = b->msec + uptime - b->downtime;
        else
            v1 = b->msec + (frame_msec >> 1);
        b->msec = v1;
        b->active = 0;
    }
}

void __cdecl IN_DownDown()
{
    IN_KeyDown(&playersKb[0][11]);
}

void __cdecl IN_DownUp()
{
    IN_KeyUp(&playersKb[0][11]);
}

void __cdecl IN_LeftDown()
{
    IN_KeyDown(playersKb[0]);
}

void __cdecl IN_LeftUp()
{
    IN_KeyUp(playersKb[0]);
}

void __cdecl IN_RightDown()
{
    IN_KeyDown(&playersKb[0][1]);
}

void __cdecl IN_RightUp()
{
    IN_KeyUp(&playersKb[0][1]);
}

void __cdecl IN_ForwardDown()
{
    IN_KeyDown(&playersKb[0][2]);
}

void __cdecl IN_ForwardUp()
{
    IN_KeyUp(&playersKb[0][2]);
}

void __cdecl IN_BackDown()
{
    IN_KeyDown(&playersKb[0][3]);
}

void __cdecl IN_BackUp()
{
    IN_KeyUp(&playersKb[0][3]);
}

void __cdecl IN_LookupDown()
{
    IN_KeyDown(&playersKb[0][4]);
}

void __cdecl IN_LookupUp()
{
    IN_KeyUp(&playersKb[0][4]);
}

void __cdecl IN_LookdownDown()
{
    IN_KeyDown(&playersKb[0][5]);
}

void __cdecl IN_LookdownUp()
{
    IN_KeyUp(&playersKb[0][5]);
}

void __cdecl IN_MoveleftDown()
{
    IN_KeyDown(&playersKb[0][6]);
}

void __cdecl IN_MoveleftUp()
{
    IN_KeyUp(&playersKb[0][6]);
}

void __cdecl IN_MoverightDown()
{
    IN_KeyDown(&playersKb[0][7]);
}

void __cdecl IN_MoverightUp()
{
    IN_KeyUp(&playersKb[0][7]);
}

void __cdecl IN_SpeedDown()
{
    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        CL_GetLocalClientGlobals(0)->usingAds = 0;
        IN_KeyDown(&playersKb[0][9]);
    }
}

void __cdecl IN_SpeedUp()
{
    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        CL_GetLocalClientGlobals(0)->usingAds = 0;
        IN_KeyUp(&playersKb[0][9]);
    }
}

void __cdecl IN_StrafeDown()
{
    IN_KeyDown(&playersKb[0][8]);
}

void __cdecl IN_StrafeUp()
{
    IN_KeyUp(&playersKb[0][8]);
}

void __cdecl IN_Attack_Down()
{
    IN_KeyDown(&playersKb[0][14]);
}

void __cdecl IN_Attack_Up()
{
    IN_KeyUp(&playersKb[0][14]);
}

void __cdecl IN_Breath_Down()
{
    IN_KeyDown(&playersKb[0][15]);
}

void __cdecl IN_Breath_Up()
{
    IN_KeyUp(&playersKb[0][15]);
}

void __cdecl IN_MeleeBreath_Down()
{
    IN_KeyDown(&playersKb[0][18]);
    IN_KeyDown(&playersKb[0][15]);
}

void __cdecl IN_MeleeBreath_Up()
{
    IN_KeyUp(&playersKb[0][18]);
    IN_KeyUp(&playersKb[0][15]);
}

void __cdecl IN_Frag_Down()
{
    IN_KeyDown(&playersKb[0][16]);
}

void __cdecl IN_Frag_Up()
{
    IN_KeyUp(&playersKb[0][16]);
}

void __cdecl IN_Smoke_Down()
{
    IN_KeyDown(&playersKb[0][17]);
}

void __cdecl IN_Smoke_Up()
{
    IN_KeyUp(&playersKb[0][17]);
}

void __cdecl IN_BreathSprint_Down()
{
    IN_KeyDown(&playersKb[0][15]);
    IN_KeyDown(&playersKb[0][27]);
}

void __cdecl IN_BreathSprint_Up()
{
    IN_KeyUp(&playersKb[0][15]);
    IN_KeyUp(&playersKb[0][27]);
}

void __cdecl IN_Melee_Down()
{
    IN_KeyDown(&playersKb[0][18]);
}

void __cdecl IN_Melee_Up()
{
    IN_KeyUp(&playersKb[0][18]);
}

void __cdecl IN_Activate_Down()
{
    IN_KeyDown(&playersKb[0][19]);
}

void __cdecl IN_Activate_Up()
{
    IN_KeyUp(&playersKb[0][19]);
}

void __cdecl IN_Reload_Down()
{
    IN_KeyDown(&playersKb[0][20]);
}

void __cdecl IN_Reload_Up()
{
    IN_KeyUp(&playersKb[0][20]);
}

void __cdecl IN_UseReload_Down()
{
    IN_KeyDown(&playersKb[0][21]);
}

void __cdecl IN_UseReload_Up()
{
    IN_KeyUp(&playersKb[0][21]);
}

void __cdecl IN_LeanLeft_Down()
{
    IN_KeyDown(&playersKb[0][22]);
}

void __cdecl IN_LeanLeft_Up()
{
    IN_KeyUp(&playersKb[0][22]);
}

void __cdecl IN_LeanRight_Down()
{
    IN_KeyDown(&playersKb[0][23]);
}

void __cdecl IN_LeanRight_Up()
{
    IN_KeyUp(&playersKb[0][23]);
}

void __cdecl IN_Prone_Down()
{
    IN_KeyDown(&playersKb[0][24]);
}

void __cdecl IN_Prone_Up()
{
    IN_KeyUp(&playersKb[0][24]);
}

void __cdecl IN_Stance_Down()
{
    bool v0; // [esp+0h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-8h]

    v0 = playersKb[0][24].active || playersKb[0][11].active;
    if (!v0 && clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        LocalClientGlobals->stanceHeld = 1;
        LocalClientGlobals->stancePosition = LocalClientGlobals->stance;
        LocalClientGlobals->stanceTime = com_frameTime;
        if (LocalClientGlobals->stancePosition != CL_STANCE_CROUCH)
            LocalClientGlobals->stance = CL_STANCE_CROUCH;
    }
}

void __cdecl IN_Stance_Up()
{
    bool v0; // [esp+0h] [ebp-Ch]
    clientActive_t *LocalClientGlobals; // [esp+4h] [ebp-8h]

    v0 = playersKb[0][24].active || playersKb[0][11].active;
    if (!v0 && clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        if (LocalClientGlobals->stanceHeld && LocalClientGlobals->stancePosition == CL_STANCE_CROUCH)
            LocalClientGlobals->stance = CL_STANCE_STAND;
        LocalClientGlobals->stanceHeld = 0;
    }
}

void __cdecl IN_ToggleADS()
{
    clientActive_t *LocalClientGlobals; // edx

    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        LocalClientGlobals->usingAds = !LocalClientGlobals->usingAds;
    }
}

void __cdecl IN_LeaveADS()
{
    if (clientUIActives[0].connectionState >= CA_CONNECTED)
        CL_GetLocalClientGlobals(0)->usingAds = 0;
}

void __cdecl IN_Throw_Down()
{
    IN_KeyDown(&playersKb[0][26]);
}

void __cdecl IN_Throw_Up()
{
    IN_KeyUp(&playersKb[0][26]);
}

void __cdecl IN_ToggleADS_Throw_Down()
{
    IN_ToggleADS();
    IN_Throw_Down();
}

void __cdecl IN_ToggleADS_Throw_Up()
{
    IN_Throw_Up();
}

void __cdecl IN_Speed_Throw_Down()
{
    IN_SpeedDown();
    IN_Throw_Down();
}

void __cdecl IN_Speed_Throw_Up()
{
    IN_SpeedUp();
    IN_Throw_Up();
}

void __cdecl IN_LowerStance()
{
    StanceState stance; // [esp+0h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-8h]

    if (clientUIActives[0].connectionState >= CA_CONNECTED && !playersKb[0][24].active && !playersKb[0][11].active)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        stance = LocalClientGlobals->stance;
        if (stance)
        {
            if (stance == CL_STANCE_CROUCH)
            {
                LocalClientGlobals->stance = CL_STANCE_PRONE;
            }
            else if (LocalClientGlobals->stance != CL_STANCE_PRONE)
            {
                MyAssertHandler(".\\client_mp\\cl_input.cpp", 881, 1, "%s", "cl->stance == CL_STANCE_PRONE");
            }
        }
        else
        {
            LocalClientGlobals->stance = CL_STANCE_CROUCH;
        }
    }
}

void __cdecl IN_RaiseStance()
{
    StanceState stance; // [esp+0h] [ebp-10h]
    clientActive_t *LocalClientGlobals; // [esp+8h] [ebp-8h]

    if (clientUIActives[0].connectionState >= CA_CONNECTED && !playersKb[0][24].active && !playersKb[0][11].active)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(0);
        stance = LocalClientGlobals->stance;
        if (stance == CL_STANCE_CROUCH)
        {
            LocalClientGlobals->stance = CL_STANCE_STAND;
        }
        else if (stance == CL_STANCE_PRONE)
        {
            LocalClientGlobals->stance = CL_STANCE_CROUCH;
        }
        else if (LocalClientGlobals->stance)
        {
            MyAssertHandler(".\\client_mp\\cl_input.cpp", 910, 1, "%s", "cl->stance == CL_STANCE_STAND");
        }
    }
}

void __cdecl IN_ToggleCrouch()
{
    CL_ToggleStance(0, CL_STANCE_CROUCH);
}

void __cdecl CL_ToggleStance(int localClientNum, StanceState preferredStance)
{
    clientActive_t *LocalClientGlobals; // edx

    if (!playersKb[localClientNum][24].active && !playersKb[localClientNum][11].active)
    {
        LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
        LocalClientGlobals->stance = preferredStance != LocalClientGlobals->stance ? preferredStance : CL_STANCE_STAND;
    }
}

void __cdecl IN_ToggleProne()
{
    CL_ToggleStance(0, CL_STANCE_PRONE);
}

void __cdecl IN_GoProne()
{
    CL_SetStance(0, CL_STANCE_PRONE);
}

void __cdecl IN_GoCrouch()
{
    CL_SetStance(0, CL_STANCE_CROUCH);
}

void __cdecl IN_GoStandDown()
{
    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        IN_KeyDown(&playersKb[0][12]);
        if (CL_GetLocalClientGlobals(0)->stance)
            CL_SetStance(0, CL_STANCE_STAND);
        else
            IN_KeyDown(&playersKb[0][10]);
    }
}

void __cdecl IN_GoStandUp()
{
    if (clientUIActives[0].connectionState >= CA_CONNECTED)
    {
        IN_KeyUp(&playersKb[0][12]);
        IN_KeyUp(&playersKb[0][10]);
    }
}

void __cdecl IN_TalkDown()
{
    IN_KeyDown(&playersKb[0][29]);
}

void __cdecl IN_TalkUp()
{
    IN_KeyUp(&playersKb[0][29]);
}

void __cdecl IN_SprintDown()
{
    IN_KeyDown(&playersKb[0][27]);
}

void __cdecl IN_SprintUp()
{
    IN_KeyUp(&playersKb[0][27]);
}

void __cdecl CL_ShutdownInput()
{
    Cmd_RemoveCommand("mouseMove");
    Cmd_RemoveCommand("remoteKey");
    Cmd_RemoveCommand("centerview");
    Cmd_RemoveCommand("+moveup");
    Cmd_RemoveCommand("-moveup");
    Cmd_RemoveCommand("+movedown");
    Cmd_RemoveCommand("-movedown");
    Cmd_RemoveCommand("+left");
    Cmd_RemoveCommand("-left");
    Cmd_RemoveCommand("+right");
    Cmd_RemoveCommand("-right");
    Cmd_RemoveCommand("+forward");
    Cmd_RemoveCommand("-forward");
    Cmd_RemoveCommand("+back");
    Cmd_RemoveCommand("-back");
    Cmd_RemoveCommand("+lookup");
    Cmd_RemoveCommand("-lookup");
    Cmd_RemoveCommand("+lookdown");
    Cmd_RemoveCommand("-lookdown");
    Cmd_RemoveCommand("+strafe");
    Cmd_RemoveCommand("-strafe");
    Cmd_RemoveCommand("+moveleft");
    Cmd_RemoveCommand("-moveleft");
    Cmd_RemoveCommand("+moveright");
    Cmd_RemoveCommand("-moveright");
    Cmd_RemoveCommand("+speed");
    Cmd_RemoveCommand("-speed");
    Cmd_RemoveCommand("+attack");
    Cmd_RemoveCommand("-attack");
    Cmd_RemoveCommand("+melee");
    Cmd_RemoveCommand("-melee");
    Cmd_RemoveCommand("+holdbreath");
    Cmd_RemoveCommand("-holdbreath");
    Cmd_RemoveCommand("+melee_breath");
    Cmd_RemoveCommand("-melee_breath");
    Cmd_RemoveCommand("+frag");
    Cmd_RemoveCommand("-frag");
    Cmd_RemoveCommand("+smoke");
    Cmd_RemoveCommand("-smoke");
    Cmd_RemoveCommand("+breath_sprint");
    Cmd_RemoveCommand("-breath_sprint");
    Cmd_RemoveCommand("+activate");
    Cmd_RemoveCommand("-activate");
    Cmd_RemoveCommand("+reload");
    Cmd_RemoveCommand("-reload");
    Cmd_RemoveCommand("+usereload");
    Cmd_RemoveCommand("-usereload");
    Cmd_RemoveCommand("+leanleft");
    Cmd_RemoveCommand("-leanleft");
    Cmd_RemoveCommand("+leanright");
    Cmd_RemoveCommand("-leanright");
    Cmd_RemoveCommand("+prone");
    Cmd_RemoveCommand("-prone");
    Cmd_RemoveCommand("+stance");
    Cmd_RemoveCommand("-stance");
    Cmd_RemoveCommand("+mlook");
    Cmd_RemoveCommand("-mlook");
    Cmd_RemoveCommand("toggleads");
    Cmd_RemoveCommand("leaveads");
    Cmd_RemoveCommand("+throw");
    Cmd_RemoveCommand("-throw");
    Cmd_RemoveCommand("+speed_throw");
    Cmd_RemoveCommand("-speed_throw");
    Cmd_RemoveCommand("+toggleads_throw");
    Cmd_RemoveCommand("-toggleads_throw");
    Cmd_RemoveCommand("lowerstance");
    Cmd_RemoveCommand("raisestance");
    Cmd_RemoveCommand("togglecrouch");
    Cmd_RemoveCommand("toggleprone");
    Cmd_RemoveCommand("goprone");
    Cmd_RemoveCommand("gocrouch");
    Cmd_RemoveCommand("+gostand");
    Cmd_RemoveCommand("-gostand");
    Cmd_RemoveCommand("+sprint");
    Cmd_RemoveCommand("-sprint");
    Cmd_RemoveCommand("+talk");
    Cmd_RemoveCommand("-talk");
}

void __cdecl CL_ClearKeys(int localClientNum)
{
    memset((unsigned __int8 *)playersKb[localClientNum], 0, sizeof(kbutton_t[30]));
}

