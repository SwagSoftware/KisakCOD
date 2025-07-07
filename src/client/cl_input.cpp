#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cl_input.h"
#include <qcommon/mem_track.h>


//struct dvar_s const *const cl_stanceHoldTime 827f4fc8     cl_input.obj
//struct dvar_s const *const cl_analog_attack_threshold 827f4fcc     cl_input.obj
//struct dvar_s const *const cl_yawspeed 827f4fd0     cl_input.obj
//int marker_cl_input      827f4fd4     cl_input.obj
//struct dvar_s const *const cl_upspeed 827f4fd8     cl_input.obj
//struct dvar_s const *const cl_sidespeed 827f4fdc     cl_input.obj
//int old_com_frameTime    827f4fe0     cl_input.obj
//unsigned int frame_msec           827f4fe4     cl_input.obj
//struct dvar_s const *const cl_pitchspeed 827f522c     cl_input.obj
//struct dvar_s const *const cl_forwardspeed 827f5230     cl_input.obj
//struct dvar_s const *const cl_anglespeedkey 827f5234     cl_input.obj

kbutton_t kb[29];

void __cdecl TRACK_cl_input()
{
    track_static_alloc_internal(kb, 580, "kb", 10);
}

bool __cdecl IN_IsTempProneKeyActive()
{
    return kb[24].active;
}

int __cdecl IN_IsTempStanceKeyActive()
{
    unsigned __int8 v0; // r11

    if (kb[24].active)
        return 1;
    v0 = 0;
    if (kb[11].active)
        return 1;
    return v0;
}

void IN_MLookDown()
{
    kb[13].active = 1;
}

void __cdecl IN_KeyDown(kbutton_t *b)
{
    int nesting; // r7
    const char *v3; // r3
    int v4; // r3
    int v5; // r11
    const char *v6; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 1)
        v3 = byte_82003CDD;
    else
        v3 = (const char *)*((unsigned int *)cmd_args.argv[nesting] + 1);
    if (*v3)
        v4 = atol(v3);
    else
        v4 = -1;
    if (v4 != b->down[0])
    {
        v5 = b->down[1];
        if (v4 != v5)
        {
            if (b->down[0])
            {
                if (v5)
                {
                    Com_Printf(14, "Three keys down for a button!\n");
                    return;
                }
                b->down[1] = v4;
            }
            else
            {
                b->down[0] = v4;
            }
            if (!b->active)
            {
                v6 = Cmd_Argv(2);
                b->downtime = atol(v6);
                b->active = 1;
                b->wasPressed = 1;
            }
        }
    }
}

void __cdecl IN_KeyUp(kbutton_t *b)
{
    int nesting; // r7
    char *v3; // r3
    int v4; // r3
    int v5; // r11
    const char *v6; // r3
    int v7; // r3
    unsigned int msec; // r10
    unsigned int v9; // r11

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 1)
        v3 = byte_82003CDD;
    else
        v3 = (char *)*((unsigned int *)cmd_args.argv[nesting] + 1);
    if (!*v3)
    {
        b->down[1] = 0;
        b->down[0] = 0;
    LABEL_17:
        b->active = 0;
        return;
    }
    v4 = atol(v3);
    v5 = b->down[0];
    if (b->down[0] == v4)
    {
        b->down[0] = 0;
    LABEL_11:
        if (b->down[1])
            return;
        b->active = 0;
        v6 = Cmd_Argv(2);
        v7 = atol(v6);
        msec = b->msec;
        if (v7)
            v9 = v7 - b->downtime + msec;
        else
            v9 = (frame_msec >> 1) + msec;
        b->msec = v9;
        goto LABEL_17;
    }
    if (b->down[1] == v4)
    {
        b->down[1] = 0;
        if (!v5)
            goto LABEL_11;
    }
}

// local variable allocation has failed, the output may be wrong!
float __cdecl CL_KeyState(kbutton_t *key)
{
    __int64 v1; // r11 OVERLAPPED
    bool active; // r10
    unsigned int msec; // r30
    unsigned int downtime; // r10
    double v5; // fp1

    active = key->active;
    msec = key->msec;
    key->msec = 0;
    if (active)
    {
        downtime = key->downtime;
        if (downtime)
            msec += com_frameTime - downtime;
        else
            msec = com_frameTime;
        key->downtime = com_frameTime;
    }
    if (msec)
    {
        LODWORD(v1) = frame_msec;
        if (msec < frame_msec)
        {
            if (!frame_msec)
            {
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp", 229, 0, "%s", "frame_msec");
                LODWORD(v1) = frame_msec;
            }
            HIDWORD(v1) = msec;
            v5 = (float)((float)*(__int64 *)((char *)&v1 + 4) / (float)v1);
        }
        else
        {
            v5 = 1.0;
        }
    }
    else
    {
        v5 = 0.0;
    }
    return *((float *)&v5 + 1);
}

void __cdecl CL_SetStance(int localClientNum, StanceState stance)
{
    char v3; // r11

    if (kb[24].active || (v3 = 0, kb[11].active))
        v3 = 1;
    if (!v3)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\client.h",
                555,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        clients[0].stance = stance;
    }
}

void __cdecl CL_ToggleStance(StanceState preferredStance)
{
    char v1; // r11
    StanceState v2; // r11

    if (kb[24].active || (v1 = 0, kb[11].active))
        v1 = 1;
    if (!v1)
    {
        v2 = CL_STANCE_STAND;
        if (clients[0].stance != preferredStance)
            v2 = preferredStance;
        clients[0].stance = v2;
    }
}

void IN_UpDown()
{
    char v0; // r11
    StanceState v1; // r10

    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            191,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
    IN_KeyDown(&kb[12]);
    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
    {
        if (clients[0].stance == CL_STANCE_CROUCH)
        {
            v1 = CL_STANCE_STAND;
        }
        else
        {
            if (clients[0].stance != CL_STANCE_PRONE)
            {
                if (clients[0].stance)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp",
                        279,
                        1,
                        "%s",
                        "cl->stance == CL_STANCE_STAND");
                IN_KeyDown(&kb[10]);
                return;
            }
            v1 = CL_STANCE_CROUCH;
        }
        clients[0].stance = v1;
    }
}

void IN_UpUp()
{
    IN_KeyUp(&kb[12]);
    IN_KeyUp(&kb[10]);
}

void IN_DownDown()
{
    IN_KeyDown(&kb[11]);
}

void IN_DownUp()
{
    IN_KeyUp(&kb[11]);
}

void IN_LeftDown()
{
    IN_KeyDown(kb);
}

void IN_LeftUp()
{
    IN_KeyUp(kb);
}

void IN_RightDown()
{
    IN_KeyDown(&kb[1]);
}

void IN_RightUp()
{
    IN_KeyUp(&kb[1]);
}

void IN_ForwardDown()
{
    IN_KeyDown(&kb[2]);
}

void IN_ForwardUp()
{
    IN_KeyUp(&kb[2]);
}

void IN_BackDown()
{
    IN_KeyDown(&kb[3]);
}

void IN_BackUp()
{
    IN_KeyUp(&kb[3]);
}

void IN_LookupDown()
{
    IN_KeyDown(&kb[4]);
}

void IN_LookupUp()
{
    IN_KeyUp(&kb[4]);
}

void IN_LookdownDown()
{
    IN_KeyDown(&kb[5]);
}

void IN_LookdownUp()
{
    IN_KeyUp(&kb[5]);
}

void IN_MoveleftDown()
{
    IN_KeyDown(&kb[6]);
}

void IN_MoveleftUp()
{
    IN_KeyUp(&kb[6]);
}

void IN_MoverightDown()
{
    IN_KeyDown(&kb[7]);
}

void IN_MoverightUp()
{
    IN_KeyUp(&kb[7]);
}

void IN_SpeedDown()
{
    IN_KeyDown(&kb[9]);
}

void IN_SpeedUp()
{
    IN_KeyUp(&kb[9]);
}

void IN_StrafeDown()
{
    IN_KeyDown(&kb[8]);
}

void IN_StrafeUp()
{
    IN_KeyUp(&kb[8]);
}

void IN_Attack_Down()
{
    IN_KeyDown(&kb[14]);
}

void IN_Attack_Up()
{
    IN_KeyUp(&kb[14]);
}

void IN_Breath_Down()
{
    IN_KeyDown(&kb[15]);
}

void IN_Breath_Up()
{
    IN_KeyUp(&kb[15]);
}

void IN_MeleeBreath_Down()
{
    IN_KeyDown(&kb[18]);
    IN_KeyDown(&kb[15]);
}

void IN_MeleeBreath_Up()
{
    IN_KeyUp(&kb[18]);
    IN_KeyUp(&kb[15]);
}

void IN_Frag_Down()
{
    IN_KeyDown(&kb[16]);
}

void IN_Frag_Up()
{
    IN_KeyUp(&kb[16]);
}

void IN_Smoke_Down()
{
    IN_KeyDown(&kb[17]);
}

void IN_Smoke_Up()
{
    IN_KeyUp(&kb[17]);
}

void __cdecl IN_BreathSprint_Down()
{
    IN_KeyDown(&kb[15]);
    IN_KeyDown(&kb[27]);
}

void __cdecl IN_BreathSprint_Up()
{
    IN_KeyUp(&kb[15]);
    IN_KeyUp(&kb[27]);
}

void IN_Melee_Down()
{
    IN_KeyDown(&kb[18]);
}

void IN_Melee_Up()
{
    IN_KeyUp(&kb[18]);
}

void IN_Activate_Down()
{
    IN_KeyDown(&kb[19]);
}

void IN_Activate_Up()
{
    IN_KeyUp(&kb[19]);
}

void IN_Reload_Down()
{
    IN_KeyDown(&kb[20]);
}

void IN_Reload_Up()
{
    IN_KeyUp(&kb[20]);
}

void IN_UseReload_Down()
{
    IN_KeyDown(&kb[21]);
}

void IN_UseReload_Up()
{
    IN_KeyUp(&kb[21]);
}

void IN_LeanLeft_Down()
{
    IN_KeyDown(&kb[22]);
}

void IN_LeanLeft_Up()
{
    IN_KeyUp(&kb[22]);
}

void IN_LeanRight_Down()
{
    IN_KeyDown(&kb[23]);
}

void IN_LeanRight_Up()
{
    IN_KeyUp(&kb[23]);
}

void IN_Prone_Down()
{
    IN_KeyDown(&kb[24]);
}

void IN_Prone_Up()
{
    IN_KeyUp(&kb[24]);
}

void IN_Stance_Down()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
    {
        clients[0].stanceHeld = 1;
        clients[0].stancePosition = clients[0].stance;
        clients[0].stanceTime = com_frameTime;
        if (clients[0].stance != CL_STANCE_CROUCH)
            clients[0].stance = CL_STANCE_CROUCH;
    }
}

void IN_Stance_Up()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
    {
        if (clients[0].stanceHeld && clients[0].stancePosition == CL_STANCE_CROUCH)
            clients[0].stance = CL_STANCE_STAND;
        clients[0].stanceHeld = 0;
    }
}

void __cdecl IN_CenterView()
{
    clients[0].viewangles[0] = -clients[0].snap.ps.delta_angles[0];
}

void __cdecl IN_RemoteKeyboard()
{
    int nesting; // r7
    const char *v1; // r3
    int v2; // r31
    const char *v3; // r3
    int v4; // r29
    const char *v5; // r3
    int v6; // r30
    unsigned int v7; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] >= 3)
    {
        v1 = Cmd_Argv(1);
        v2 = atol(v1);
        v3 = Cmd_Argv(2);
        v4 = atol(v3);
        v5 = Cmd_Argv(3);
        v6 = atol(v5);
        if (v2)
        {
            v7 = Sys_Milliseconds();
            CL_KeyEvent(0, v2, v6, v7);
        }
        if (v4)
        {
            if (v6)
                CL_CharEvent(0, v4);
        }
    }
    else
    {
        Com_Printf(0, "USAGE: remoteKey <value> <value> <value>\n");
    }
}

void __cdecl IN_RemoteMouseMove()
{
    const char *v0; // r3
    int v1; // r31
    const char *v2; // r3
    int v3; // r3

    if (CG_IsValidRemoteInputState(0))
    {
        if (Cmd_Argc() >= 2)
        {
            v0 = Cmd_Argv(1);
            v1 = atol(v0);
            v2 = Cmd_Argv(2);
            v3 = atol(v2);
            clients[0].mouseDx[clients[0].mouseIndex] = v1;
            clients[0].mouseDy[clients[0].mouseIndex] = v3;
        }
        else
        {
            Com_Printf(0, "USAGE: mouseMove <value> <value>\n");
        }
    }
    else
    {
        Com_Printf(0, " Invalid State for using remote Mouse input : Must be in God Mode, UFO, or NOCLIP\n");
    }
}

void IN_ToggleADS()
{
    clients[0].usingAds = !clients[0].usingAds;
}

void IN_LeaveADS()
{
    clients[0].usingAds = 0;
}

void IN_Throw_Down()
{
    IN_KeyDown(&kb[26]);
}

void IN_Throw_Up()
{
    IN_KeyUp(&kb[26]);
}

void IN_ToggleADS_Throw_Down()
{
    clients[0].usingAds = (_cntlzw(clients[0].usingAds) & 0x20) != 0;
    IN_KeyDown(&kb[26]);
}

void IN_ToggleADS_Throw_Up()
{
    IN_KeyUp(&kb[26]);
}

void IN_Speed_Throw_Down()
{
    IN_KeyDown(&kb[9]);
    IN_KeyDown(&kb[26]);
}

void IN_Speed_Throw_Up()
{
    IN_KeyUp(&kb[9]);
    IN_KeyUp(&kb[26]);
}

void IN_LowerStance()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
    {
        if (clients[0].stance)
        {
            if (clients[0].stance == CL_STANCE_CROUCH)
            {
                clients[0].stance = CL_STANCE_PRONE;
            }
            else if (clients[0].stance != CL_STANCE_PRONE)
            {
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp",
                    754,
                    1,
                    "%s",
                    "cl->stance == CL_STANCE_PRONE");
            }
        }
        else
        {
            clients[0].stance = CL_STANCE_CROUCH;
        }
    }
}

void IN_RaiseStance()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
    {
        if (clients[0].stance == CL_STANCE_CROUCH)
        {
            clients[0].stance = CL_STANCE_STAND;
        }
        else if (clients[0].stance == CL_STANCE_PRONE)
        {
            clients[0].stance = CL_STANCE_CROUCH;
        }
        else if (clients[0].stance)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp",
                777,
                1,
                "%s",
                "cl->stance == CL_STANCE_STAND");
        }
    }
}

void IN_ToggleCrouch()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
        clients[0].stance = (_cntlzw(clients[0].stance - 1) & 0x20) == 0;
}

void IN_ToggleProne()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
        clients[0].stance = (_cntlzw(clients[0].stance - 2) >> 4) & 2 ^ 2;
}

void IN_GoProne()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
        clients[0].stance = CL_STANCE_PRONE;
}

void IN_GoCrouch()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (!v0)
        clients[0].stance = CL_STANCE_CROUCH;
}

void IN_GoStandDown()
{
    char v0; // r11

    IN_KeyDown(&kb[12]);
    if (clients[0].stance)
    {
        if (kb[24].active || (v0 = 0, kb[11].active))
            v0 = 1;
        if (!v0)
            clients[0].stance = CL_STANCE_STAND;
    }
    else
    {
        IN_KeyDown(&kb[10]);
    }
}

void IN_GoStandUp()
{
    IN_KeyUp(&kb[12]);
    IN_KeyUp(&kb[10]);
}

void __cdecl IN_SprintDown()
{
    IN_KeyDown(&kb[27]);
}

void __cdecl IN_SprintUp()
{
    IN_KeyUp(&kb[27]);
}

void __cdecl IN_NightVisionDown()
{
    IN_KeyDown(&kb[28]);
}

void __cdecl IN_NightVisionUp()
{
    IN_KeyUp(&kb[28]);
}

void __cdecl CL_AdjustAngles(int a1, int a2, int a3, int a4, int a5, int a6, __int64 a7)
{
    __int64 v7; // r11
    double v8; // fp13
    double v9; // fp31

    HIDWORD(v7) = kb[9].active;
    LODWORD(v7) = cls.frametime;
    if (kb[9].active)
    {
        LODWORD(a7) = cls.frametime;
        v8 = (float)(cl_anglespeedkey->current.value * (float)a7);
    }
    else
    {
        v8 = (float)v7;
    }
    v9 = (float)((float)v8 * (float)0.001);
    if (!kb[8].active)
    {
        clients[0].viewangles[1] = -(float)((float)((float)(CL_KeyState(&kb[1]) * cl_yawspeed->current.value)
            * (float)((float)v8 * (float)0.001))
            - clients[0].viewangles[1]);
        clients[0].viewangles[1] = (float)((float)(CL_KeyState(kb) * cl_yawspeed->current.value) * (float)v9)
            + clients[0].viewangles[1];
    }
    clients[0].viewangles[0] = -(float)((float)((float)(CL_KeyState(&kb[4]) * cl_pitchspeed->current.value) * (float)v9)
        - clients[0].viewangles[0]);
    clients[0].viewangles[0] = (float)((float)(CL_KeyState(&kb[5]) * cl_pitchspeed->current.value) * (float)v9)
        + clients[0].viewangles[0];
}

void CL_StanceButtonUpdate()
{
    char v0; // r11

    if (kb[24].active || (v0 = 0, kb[11].active))
        v0 = 1;
    if (v0)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp", 891, 0, "%s", "!IN_IsTempStanceKeyActive()");
    if (clients[0].stanceHeld && com_frameTime - clients[0].stanceTime >= cl_stanceHoldTime->current.integer)
    {
        clients[0].stanceHeld = 0;
        clients[0].stance = (_cntlzw(clients[0].stancePosition - 2) >> 4) & 2 ^ 2;
    }
}

void __cdecl CL_AddCurrentStanceToCmd(usercmd_s *cmd)
{
    unsigned int v2; // r11

    if (clients[0].stance == CL_STANCE_CROUCH)
    {
        v2 = __ROL4__(1, 9) & 0x300 | cmd->buttons & 0xFFFFFCFF;
    }
    else if (clients[0].stance == CL_STANCE_PRONE)
    {
        v2 = cmd->buttons & 0xFFFFFCFF | 0x100;
    }
    else
    {
        if (clients[0].stance)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp",
                925,
                0,
                "%s",
                "cl->stance == CL_STANCE_STAND");
        v2 = cmd->buttons & 0xFFFFFCFF;
    }
    cmd->buttons = v2;
    cmd->buttons = v2 & 0xFFFFEFFF;
}

void __cdecl CL_KeyMove(usercmd_s *cmd)
{
    char v2; // r10
    int buttons; // r11
    unsigned int v4; // r11
    int v5; // r11
    int v6; // r11
    int v7; // r30
    int v8; // r30
    int v9; // r30
    int v10; // r29
    int v11; // r28
    int v12; // r31
    int v13; // [sp+54h] [-4Ch]
    int v14; // [sp+54h] [-4Ch]
    int v15; // [sp+54h] [-4Ch]

    if (kb[24].active || (v2 = 0, kb[11].active))
        v2 = 1;
    if (v2)
    {
        buttons = cmd->buttons;
        if (kb[24].active)
            v4 = buttons & 0xFFFFFCFF | 0x100;
        else
            v4 = __ROL4__(1, 9) & 0x300 | buttons & 0xFFFFFCFF;
        cmd->buttons = v4;
        cmd->buttons = v4 | 0x1000;
    }
    else
    {
        CL_StanceButtonUpdate();
        CL_AddCurrentStanceToCmd(cmd);
    }
    v5 = cmd->buttons;
    if (kb[9].active == !clients[0].usingAds)
        v6 = v5 | 0x800;
    else
        v6 = v5 & 0xFFFFF7FF;
    cmd->buttons = v6;
    v7 = 0;
    if (kb[8].active)
    {
        v13 = (int)(float)(CL_KeyState(&kb[1]) * (float)127.0);
        v7 = v13 - (int)(float)(CL_KeyState(kb) * (float)127.0);
    }
    v8 = v7 - (int)(float)(CL_KeyState(&kb[7]) * (float)-127.0);
    v9 = v8 - (int)(float)(CL_KeyState(&kb[6]) * (float)127.0);
    v14 = (int)(float)(CL_KeyState(&kb[2]) * (float)127.0);
    v10 = v14 - (int)(float)(CL_KeyState(&kb[3]) * (float)127.0);
    v15 = (int)(float)(CL_KeyState(&kb[23]) * (float)127.0);
    v11 = v15 - (int)(float)(CL_KeyState(&kb[22]) * (float)127.0);
    if (kb[8].active && (cmd->buttons & 2) == 0)
    {
        v12 = v9 - (int)(float)(CL_KeyState(&kb[1]) * (float)-127.0);
        v9 = v12 - (int)(float)(CL_KeyState(kb) * (float)127.0);
    }
    cmd->forwardmove = ClampChar(v10);
    cmd->rightmove = ClampChar(v9);
    cmd->upmove = ClampChar(v11);
}

int __cdecl CL_AllowInput()
{
    int integer; // r11
    int result; // r3
    bool v2; // zf

    if (!com_sv_running->current.enabled)
        return 1;
    integer = cl_paused->current.integer;
    if (integer == 1)
        return 0;
    if (integer)
        return 1;
    if (cl_freemove->current.integer)
        return 1;
    v2 = G_DemoPlaying() != 0;
    result = 0;
    if (!v2)
        return 1;
    return result;
}

void __cdecl CL_GamepadMove(usercmd_s *cmd)
{
    double v2; // fp27
    double v3; // fp28
    double v4; // fp29
    double v5; // fp30
    int v6; // r3
    double Button; // fp26
    int v8; // r3
    double v9; // fp25
    double v10; // fp24
    double v11; // fp31
    double v12; // fp0
    char v13; // r3
    char forwardmove; // r10
    char v15; // r3
    char upmove; // r10
    char v17; // r3
    char pitchmove; // r10
    char v19; // r3
    char yawmove; // r10
    int v21; // r3
    GamerSettingState *ProfileSettings; // r3
    __int64 v23; // r11
    double v24; // fp31
    int buttons; // r10
    AimOutput v26; // [sp+60h] [-A0h] BYREF
    AimInput v27; // [sp+70h] [-90h] BYREF

    if (cl_paused->current.integer == 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp",
            1127,
            0,
            "%s",
            "cl_paused->current.integer != 1");
    if ((clients[0].snap.ps.pm_flags & 0x800) == 0 || cl_freemove->current.integer == 2)
    {
        v2 = CL_GamepadAxisValue(0, 4);
        v3 = -CL_GamepadAxisValue(0, 3);
        v4 = CL_GamepadAxisValue(0, 1);
        v5 = CL_GamepadAxisValue(0, 0);
        v6 = CL_ControllerIndexFromClientNum(0);
        Button = GPad_GetButton(v6, GPAD_R_TRIG);
        v8 = CL_ControllerIndexFromClientNum(0);
        v9 = GPad_GetButton(v8, GPAD_L_TRIG);
        v10 = CL_GamepadAxisValue(0, 5);
        v11 = 127.0;
        if (__fabs(v5) > 0.0 || __fabs(v4) > 0.0)
        {
            if (__fabs(v5) <= __fabs(v4))
                v12 = (float)((float)v5 / (float)v4);
            else
                v12 = (float)((float)v4 / (float)v5);
            v11 = (float)((float)__fsqrts((float)((float)((float)v12 * (float)v12) + (float)1.0)) * (float)127.0);
        }
        v13 = ClampChar((int)(float)((float)v11 * (float)v5) + cmd->rightmove);
        forwardmove = cmd->forwardmove;
        cmd->rightmove = v13;
        v15 = ClampChar((int)(float)((float)v11 * (float)v4) + forwardmove);
        upmove = cmd->upmove;
        cmd->forwardmove = v15;
        v17 = ClampChar((int)(float)((float)((float)v11 * (float)Button) - (float)((float)v11 * (float)v9)) + upmove);
        pitchmove = cmd->pitchmove;
        cmd->upmove = v17;
        v19 = ClampChar((int)(float)((float)v11 * (float)v2) + pitchmove);
        yawmove = cmd->yawmove;
        cmd->pitchmove = v19;
        cmd->yawmove = ClampChar((int)(float)((float)v11 * (float)v3) + yawmove);
        v21 = CL_ControllerIndexFromClientNum(0);
        ProfileSettings = Gamer//Profile_GetProfileSettings(v21);
            HIDWORD(v23) = _cntlzw(clients[0].usingAds);
        LODWORD(v23) = !ProfileSettings->invertPitch ? -1 : 1;
        v24 = (float)((float)v23 * (float)v2);
        if (kb[9].active == ((HIDWORD(v23) >> 5) & 1))
            cmd->buttons |= 0x800u;
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
        if (v10 >= cl_analog_attack_threshold->current.value)
            cmd->buttons |= 1u;
        buttons = cmd->buttons;
        v27.pitchAxis = v24;
        v27.yawAxis = v3;
        v27.forwardAxis = v4;
        v27.rightAxis = v5;
        v27.buttons = buttons;
        v27.pitch = clients[0].viewangles[0];
        v27.localClientNum = 0;
        v27.pitchMax = clients[0].cgameMaxPitchSpeed;
        v27.yaw = clients[0].viewangles[1];
        v27.yawMax = clients[0].cgameMaxYawSpeed;
        v27.deltaTime = (float)(unsigned int)cls.frametime * (float)0.001;
        v27.ps = CG_GetPredictedPlayerState(0);
        AimAssist_UpdateGamePadInput(&v27, &v26);
        clients[0].viewangles[0] = v26.pitch;
        clients[0].viewangles[1] = v26.yaw;
        cmd->meleeChargeDist = v26.meleeChargeDist;
        cmd->meleeChargeYaw = v26.meleeChargeYaw;
        CG_ModelPreviewerHandleGamepadEvents(0, v4, v5, v24, v3);
    }
}

void __cdecl CL_GetMouseMovement(clientActive_t *cl, float *mx, float *my, int a4, int a5, __int64 a6)
{
    __int64 v9; // r11
    double v10; // fp0
    int v11; // r10

    if (!mx)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp", 1240, 0, "%s", "mx");
    if (!my)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp", 1241, 0, "%s", "my");
    HIDWORD(v9) = 51536;
    if (m_filter->current.enabled)
    {
        LODWORD(v9) = cl->mouseDx[1] + cl->mouseDx[0];
        *mx = (float)v9 * (float)0.5;
        LODWORD(v9) = cl->mouseDy[1] + cl->mouseDy[0];
        v10 = (float)((float)v9 * (float)0.5);
    }
    else
    {
        LODWORD(a6) = cl->mouseDx[cl->mouseIndex];
        *mx = (float)a6;
        LODWORD(v9) = cl->mouseDy[cl->mouseIndex];
        v10 = (float)v9;
    }
    *my = v10;
    v11 = cl->mouseIndex ^ 1;
    cl->mouseIndex = v11;
    cl->mouseDx[v11] = 0;
    cl->mouseDy[cl->mouseIndex] = 0;
}

void __cdecl CL_MouseMove(usercmd_s *cmd, int a2, int a3, int a4, int a5, int a6, __int64 a7)
{
    __int64 v8; // r9
    long double v9; // fp2
    __int64 v10; // r11
    double v11; // fp29
    double v12; // fp30
    double v13; // fp12
    double v14; // fp12
    double v15; // fp31
    double v16; // r5
    __int64 v17; // r9
    long double v18; // fp2
    int rightmove; // r11
    long double v28; // fp2
    int forwardmove; // r10
    float v30[2]; // [sp+50h] [-60h] BYREF
    __int64 v31[6]; // [sp+58h] [-58h] BYREF

    CL_GetMouseMovement(clients, v30, (float *)v31, a4, a5, a7);
    HIDWORD(v10) = frame_msec;
    if (frame_msec)
    {
        v11 = *(float *)v31;
        LODWORD(v8) = frame_msec;
        v12 = v30[0];
        v13 = (float)((float)(v30[0] * v30[0]) + (float)(*(float *)v31 * *(float *)v31));
        v31[0] = v8;
        v14 = __fsqrts(v13);
        v15 = (float)((float)((float)(cl_mouseAccel->current.value * (float)((float)v14 / (float)v8))
            + cl_sensitivity->current.value)
            * clients[0].cgameFOVSensitivityScale);
        if ((float)((float)v14 / (float)v8) != 0.0 && cl_showMouseRate->current.enabled)
        {
            v16 = (float)((float)v14 / (float)v8);
            Com_Printf(14, (const char *)HIDWORD(v16), LODWORD(v16), LODWORD(v15));
            HIDWORD(v10) = frame_msec;
        }
        if ((clients[0].snap.ps.pm_flags & 0x800) == 0)
        {
            if ((float)((float)v15 * (float)v12) != 0.0 || (float)((float)v15 * (float)v11) != 0.0)
            {
                HIDWORD(v17) = 0x82000000;
                LOBYTE(v10) = kb[8].active;
                if (kb[8].active)
                {
                    *(double *)&v9 = (float)((float)(m_side->current.value * (float)((float)v15 * (float)v12)) + (float)0.5);
                    v18 = floor(v9);
                    rightmove = cmd->rightmove;
                    HIDWORD(v31[0]) = (int)(float)*(double *)&v18;
                    cmd->rightmove = ClampChar(rightmove + HIDWORD(v31[0]));
                    HIDWORD(v10) = frame_msec;
                    LOBYTE(v10) = kb[8].active;
                }
                else
                {
                    _FP0 = (float)(m_yaw->current.value * (float)((float)v15 * (float)v12));
                    if (clients[0].cgameMaxYawSpeed != 0.0)
                    {
                        LODWORD(v17) = HIDWORD(v10);
                        v31[0] = v17;
                        _FP11 = (float)((float)_FP0 - (float)((float)((float)v17 * clients[0].cgameMaxYawSpeed) * (float)0.001));
                        __asm { fsel      f0, f11, f13, f0 }
                        _FP13 = (float)((float)-(float)((float)((float)v17 * clients[0].cgameMaxYawSpeed) * (float)0.001)
                            - (float)_FP0);
                        __asm { fsel      f0, f13, f12, f0 }
                    }
                    clients[0].viewangles[1] = clients[0].viewangles[1] - (float)_FP0;
                }
                if ((kb[13].active || cl_freelook->current.enabled) && !(_BYTE)v10)
                {
                    _FP0 = (float)(m_pitch->current.value * (float)((float)v15 * (float)v11));
                    if (clients[0].cgameMaxPitchSpeed != 0.0)
                    {
                        LODWORD(v10) = HIDWORD(v10);
                        v31[0] = v10;
                        _FP11 = (float)((float)_FP0 - (float)((float)((float)v10 * clients[0].cgameMaxPitchSpeed) * (float)0.001));
                        __asm { fsel      f0, f11, f13, f0 }
                        _FP13 = (float)((float)-(float)((float)((float)v10 * clients[0].cgameMaxPitchSpeed) * (float)0.001)
                            - (float)_FP0);
                        __asm { fsel      f0, f13, f12, f0 }
                    }
                    clients[0].viewangles[0] = clients[0].viewangles[0] + (float)_FP0;
                }
                else
                {
                    *(double *)&v9 = (float)((float)(m_forward->current.value * (float)((float)v15 * (float)v11)) + (float)0.5);
                    v28 = floor(v9);
                    forwardmove = cmd->forwardmove;
                    HIDWORD(v31[0]) = (int)(float)*(double *)&v28;
                    cmd->forwardmove = ClampChar(forwardmove - HIDWORD(v31[0]));
                }
            }
            cmd->meleeChargeYaw = 0.0;
            cmd->meleeChargeDist = 0;
        }
    }
}

void __cdecl CL_UpdateCmdButton(int *cmdButtons, int kbButton, int buttonFlag)
{
    int v3; // r10

    v3 = kbButton;
    if (kb[kbButton].active || kb[v3].wasPressed)
        *cmdButtons |= buttonFlag;
    kb[v3].wasPressed = 0;
}

void __cdecl CL_CmdButtons(usercmd_s *cmd)
{
    if (kb[14].active || kb[14].wasPressed)
        cmd->buttons |= 1u;
    kb[14].wasPressed = 0;
    if (kb[15].active || kb[15].wasPressed)
        cmd->buttons |= 0x2000u;
    kb[15].wasPressed = 0;
    if (kb[16].active || kb[16].wasPressed)
        cmd->buttons |= 0x4000u;
    kb[16].wasPressed = 0;
    if (kb[17].active || kb[17].wasPressed)
        cmd->buttons |= 0x8000u;
    kb[17].wasPressed = 0;
    if (kb[18].active || kb[18].wasPressed)
        cmd->buttons |= 4u;
    kb[18].wasPressed = 0;
    if (kb[19].active || kb[19].wasPressed)
        cmd->buttons |= 8u;
    kb[19].wasPressed = 0;
    if (kb[20].active || kb[20].wasPressed)
        cmd->buttons |= 0x10u;
    kb[20].wasPressed = 0;
    if (kb[21].active || kb[21].wasPressed)
        cmd->buttons |= 0x20u;
    kb[21].wasPressed = 0;
    if (kb[22].active || kb[22].wasPressed)
        cmd->buttons |= 0x40u;
    kb[22].wasPressed = 0;
    if (kb[23].active || kb[23].wasPressed)
        cmd->buttons |= 0x80u;
    kb[23].wasPressed = 0;
    if (kb[24].active || kb[24].wasPressed)
        cmd->buttons |= 0x100u;
    kb[24].wasPressed = 0;
    if (kb[25].active || kb[25].wasPressed)
        cmd->buttons |= 0x200u;
    kb[25].wasPressed = 0;
    if (kb[10].active || kb[10].wasPressed)
        cmd->buttons |= 0x400u;
    kb[10].wasPressed = 0;
    if (kb[28].active || kb[28].wasPressed)
        cmd->buttons |= 0x40000u;
    kb[28].wasPressed = 0;
    if (kb[26].active || kb[26].wasPressed)
        cmd->buttons |= 0x80000u;
    kb[26].wasPressed = 0;
    if (clients[0].snap.ps.pm_type == 2
        || clients[0].snap.ps.pm_type == 3
        || (clients[0].snap.ps.eFlags & 0x20000) != 0 && (clients[0].snap.ps.eFlags & 0x80000) == 0)
    {
        if (kb[12].active || kb[12].wasPressed)
            cmd->buttons |= 0x400u;
        kb[12].wasPressed = 0;
    }
    if (cmd->buttons >= 0x100000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp",
            1394,
            0,
            "%s",
            "cmd->buttons < (1 << BUTTON_BIT_COUNT)");
}

void __cdecl CL_SetUsercmdButtonsWeapons(int buttons, int weapon, int offhand)
{
    if (clientUIActives[0].isRunning)
    {
        clients[0].bCmdForceValues = 1;
        clients[0].iForceButtons = buttons;
        clients[0].iForceWeapon = weapon;
        clients[0].forceOffhand = offhand;
    }
}

void __cdecl CL_FinishMove(usercmd_s *cmd)
{
    int buttons; // r9

    cmd->weapon = clients[0].cgameUserCmdWeapon;
    cmd->offHandIndex = clients[0].cgameUserCmdOffHandIndex;
    cmd->serverTime = clients[0].serverTime;
    cmd->angles[0] = (unsigned __int16)(int)(float)(clients[0].viewangles[0] * (float)182.04445);
    cmd->angles[1] = (unsigned __int16)(int)(float)(clients[0].viewangles[1] * (float)182.04445);
    cmd->angles[2] = (unsigned __int16)(int)(float)(clients[0].viewangles[2] * (float)182.04445);
    buttons = cmd->buttons;
    cmd->gunPitch = clients[0].cgameUserCmdGunPitch;
    cmd->gunYaw = clients[0].cgameUserCmdGunYaw;
    cmd->gunXOfs = clients[0].cgameUserCmdGunXOfs;
    cmd->gunYOfs = clients[0].cgameUserCmdGunYOfs;
    cmd->gunZOfs = clients[0].cgameUserCmdGunZOfs;
    cmd->buttons = buttons | clients[0].cgameExtraButtons;
    clients[0].cgameExtraButtons = 0;
    if (clients[0].bCmdForceValues)
    {
        cmd->buttons = clients[0].iForceButtons;
        cmd->weapon = clients[0].iForceWeapon;
        cmd->offHandIndex = clients[0].forceOffhand;
        clients[0].bCmdForceValues = 0;
    }
}

int __cdecl CG_HandleLocationSelectionInput(int localClientNum, usercmd_s *cmd)
{
    int result; // r3
    double v5; // fp27
    double v6; // fp28
    double v7; // fp31
    double v8; // fp31
    double v9; // fp30
    double v10; // fp1
    double v11; // fp0
    double v12; // fp13
    double v13; // fp13
    double v14; // fp1
    long double v15; // fp2
    LocSelInputState locSelInputState; // r11
    long double v17; // fp2
    long double v18; // fp2

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\../cgame/cg_local.h",
            910,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (cgArray[0].predictedPlayerState.locationSelectionInfo)
    {
        CL_AddCurrentStanceToCmd(cmd);
        v5 = (float)(cgArray[0].compassMapWorldSize[0] / cgArray[0].compassMapWorldSize[1]);
        v6 = (float)((float)(cgArray[0].frametime | 0x29DAC00000000uLL) * (float)0.001);
        v7 = CL_GamepadAxisValue(localClientNum, 4);
        v8 = (float)((float)v7 + CL_GamepadAxisValue(localClientNum, 1));
        v9 = CL_GamepadAxisValue(localClientNum, 3);
        v10 = CL_GamepadAxisValue(localClientNum, 0);
        v11 = (float)((float)v9 + (float)v10);
        v12 = (float)((float)((float)((float)v9 + (float)v10) * (float)((float)v9 + (float)v10))
            + (float)((float)v8 * (float)v8));
        if (v12 > 1.0)
        {
            v13 = (float)((float)1.0 / (float)__fsqrts(v12));
            v8 = (float)((float)v13 * (float)v8);
            v11 = (float)((float)v13 * (float)((float)v9 + (float)v10));
        }
        v14 = (float)((float)((float)(cg_mapLocationSelectionCursorSpeed->current.value * (float)v11) * (float)v6)
            + cgArray[0].selectedLocation[0]);
        cgArray[0].selectedLocation[0] = (float)((float)(cg_mapLocationSelectionCursorSpeed->current.value * (float)v11)
            * (float)v6)
            + cgArray[0].selectedLocation[0];
        cgArray[0].selectedLocation[1] = -(float)((float)((float)((float)(cg_mapLocationSelectionCursorSpeed->current.value
            * (float)v8)
            * (float)v5)
            * (float)v6)
            - cgArray[0].selectedLocation[1]);
        cgArray[0].selectedLocation[0] = ClampFloat(v14, 0.0, 1.0);
        cgArray[0].selectedLocation[1] = ClampFloat(cgArray[0].selectedLocation[1], 0.0, 1.0);
        locSelInputState = playerKeys[localClientNum].locSelInputState;
        if (locSelInputState == LOC_SEL_INPUT_CONFIRM)
        {
            cmd->buttons |= 0x10000u;
            *(double *)&v15 = (float)((float)(cgArray[0].selectedLocation[0] * (float)255.0) + (float)0.5);
            v17 = floor(v15);
            cmd->selectedLocation[0] = (int)(float)*(double *)&v17 + 0x80;
            *(double *)&v17 = (float)((float)(cgArray[0].selectedLocation[1] * (float)255.0) + (float)0.5);
            v18 = floor(v17);
            result = 1;
            cmd->selectedLocation[1] = (int)(float)*(double *)&v18 + 0x80;
        }
        else
        {
            if (locSelInputState == LOC_SEL_INPUT_CANCEL)
                cmd->buttons |= 0x20000u;
            return 1;
        }
    }
    else
    {
        Key_RemoveCatcher(localClientNum, -9);
        return 0;
    }
    return result;
}

void __cdecl CL_CreateCmd(usercmd_s *result, int a2, int a3, int a4, int a5, int a6, __int64 a7)
{
    double v8; // fp31
    __int64 v9; // r10
    int v10; // r8
    int v11; // r7
    int v12; // r6
    int v13; // r5
    int v14; // r4
    int v15; // r3
    double v16; // fp0

    v8 = clients[0].viewangles[0];
    CL_AdjustAngles((int)result, a2, a3, a4, a5, a6, a7);
    memset(result, 0, sizeof(usercmd_s));
    if (!Key_IsCatcherActive(0, 8) || !(unsigned __int8)CG_HandleLocationSelectionInput(0, result))
    {
        CL_CmdButtons(result);
        CL_KeyMove(result);
        CL_MouseMove(result, v14, v13, v12, v11, v10, v9);
        v15 = CL_ControllerIndexFromClientNum(0);
        if (GPad_IsActive(v15))
            CL_GamepadMove(result);
        if ((float)(clients[0].viewangles[0] - (float)v8) > 90.0)
        {
            v16 = (float)((float)v8 + (float)90.0);
        LABEL_9:
            clients[0].viewangles[0] = v16;
            goto LABEL_10;
        }
        if ((float)((float)v8 - clients[0].viewangles[0]) > 90.0)
        {
            v16 = (float)((float)v8 - (float)90.0);
            goto LABEL_9;
        }
    }
LABEL_10:
    CL_FinishMove(result);
}

void __cdecl CL_CreateNewCommands(int a1, int a2, int a3, int a4, int a5)
{
    __int64 v5; // r10
    const void *v6; // r3
    _BYTE v7[56]; // [sp+50h] [-80h] BYREF
    usercmd_s v8; // [sp+90h] [-40h] BYREF

    if (clientUIActives[0].connectionState != CA_ACTIVE)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp",
            1588,
            0,
            "%s",
            "CL_GetLocalClientConnectionState( ONLY_LOCAL_CLIENT_NUM ) == CA_ACTIVE");
    LODWORD(v5) = &con.consoleText[31992];
    HIDWORD(v5) = com_frameTime - old_com_frameTime;
    frame_msec = com_frameTime - old_com_frameTime;
    if ((unsigned int)(com_frameTime - old_com_frameTime) > 0xC8)
    {
        HIDWORD(v5) = 200;
        frame_msec = 200;
    }
    old_com_frameTime = com_frameTime;
    CL_CreateCmd(&v8, a2, a3, a4, a5, (int)&con.consoleText[31992], v5);
    memcpy(v7, v6, sizeof(v7));
    Sys_EnterCriticalSection(CRITSECT_CLIENT_CMD);
    memcpy(&clients[0].cmds[++clients[0].cmdNumber & 0x3F], v7, sizeof(clients[0].cmds[++clients[0].cmdNumber & 0x3F]));
    Sys_LeaveCriticalSection(CRITSECT_CLIENT_CMD);
}

void __cdecl CL_WritePacket()
{
    int reliableAcknowledge; // r11
    bool v1; // cr58
    const char *v2; // r30
    int cmdNumber; // r26
    int v4; // r31
    int v5; // r27
    char v6; // r31
    int v7; // r11
    usercmd_s *v8; // r30

    Sys_EnterCriticalSection(CRITSECT_CLIENT_CMD);
    while (1)
    {
        reliableAcknowledge = clientConnections[0].reliableAcknowledge;
        v1 = clientConnections[0].reliableAcknowledge == clientConnections[0].reliableSequence;
        if (clientConnections[0].reliableAcknowledge > clientConnections[0].reliableSequence)
        {
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\client\\cl_input.cpp",
                1638,
                0,
                "%s",
                "clc->reliableAcknowledge <= clc->reliableSequence");
            reliableAcknowledge = clientConnections[0].reliableAcknowledge;
            v1 = clientConnections[0].reliableAcknowledge == clientConnections[0].reliableSequence;
        }
        if (v1)
            break;
        clientConnections[0].reliableAcknowledge = reliableAcknowledge + 1;
        Sys_LeaveCriticalSection(CRITSECT_CLIENT_CMD);
        v2 = &clientConnections[0].reliableCommands[0][(clientConnections[0].reliableAcknowledge << 10) & 0x3FC00];
        SV_RecordClientCommand(v2);
        SV_ExecuteClientCommand(v2);
        Sys_EnterCriticalSection(CRITSECT_CLIENT_CMD);
    }
    cmdNumber = clients[0].cmdNumber;
    v4 = clients[0].cmdNumber - clients[0].cmdNumberAcknowledge;
    if (clients[0].cmdNumber - clients[0].cmdNumberAcknowledge > 32)
    {
        v4 = 32;
        Com_Printf(14, "MAX_PACKET_USERCMDS\n");
    }
    Sys_LeaveCriticalSection(CRITSECT_CLIENT_CMD);
    if (v4 > 0)
    {
        v5 = v4;
        v6 = cmdNumber - v4 + 1;
        do
        {
            v7 = v6 & 0x3F;
            v8 = &clients[0].cmds[v7];
            if (!cl_freemove->current.integer)
            {
                SV_RecordClientThink(&clients[0].cmds[v7]);
                SV_ClientThink(v8);
            }
            --v5;
            ++v6;
        } while (v5);
    }
    Sys_EnterCriticalSection(CRITSECT_CLIENT_CMD);
    clients[0].cmdNumberAcknowledge = cmdNumber;
    Sys_LeaveCriticalSection(CRITSECT_CLIENT_CMD);
}

void __cdecl CL_ClearClientCommandPacket()
{
    Sys_EnterCriticalSection(CRITSECT_CLIENT_CMD);
    clientConnections[0].reliableAcknowledge = clientConnections[0].reliableSequence;
    Sys_LeaveCriticalSection(CRITSECT_CLIENT_CMD);
}

void __cdecl CL_ClearClientThinkPacket()
{
    Sys_EnterCriticalSection(CRITSECT_CLIENT_CMD);
    clients[0].cmdNumberAcknowledge = clients[0].cmdNumber;
    Sys_LeaveCriticalSection(CRITSECT_CLIENT_CMD);
}

void PausedModelPreviewerGamepad()
{
    double v0; // fp31
    int v1; // r3
    __int64 v2; // r11
    double v3; // fp31
    double v4; // fp30
    double v5; // fp29
    double v6; // fp1

    v0 = CL_GamepadAxisValue(0, 4);
    v1 = CL_ControllerIndexFromClientNum(0);
    LODWORD(v2) = !Gamer//Profile_GetProfileSettings(v1)->invertPitch ? -1 : 1;
        v3 = (float)((float)v2 * (float)v0);
    v4 = -CL_GamepadAxisValue(0, 3);
    v5 = CL_GamepadAxisValue(0, 1);
    v6 = CL_GamepadAxisValue(0, 0);
    CG_ModelPreviewerHandleGamepadEvents(0, v5, v6, v3, v4);
}

void __cdecl CL_Input(int localClientNum)
{
    int v1; // r7
    int v2; // r6
    int v3; // r5
    int v4; // r4
    int v5; // r3

    if ((unsigned __int8)CL_AllowInput())
    {
        IN_Frame();
        v5 = CL_AllowInput();
        if ((_BYTE)v5)
            CL_CreateNewCommands(v5, v4, v3, v2, v1);
    }
    else
    {
        PausedModelPreviewerGamepad();
    }
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
}

void __cdecl CL_ClearKeys(int localClientNum)
{
    memset(kb, 0, sizeof(kb));
}

void IN_MLookUp()
{
    kb[13].active = 0;
    if (!cl_freelook->current.enabled)
        clients[0].viewangles[0] = -clients[0].snap.ps.delta_angles[0];
}

void __cdecl CL_InitInput()
{
    const char *v0; // r5
    unsigned __int16 v1; // r4
    const char *v2; // r5
    unsigned __int16 v3; // r4

    Cmd_AddCommandInternal("mouseMove", IN_RemoteMouseMove, &IN_RemoteMouseMove_VAR);
    Cmd_AddCommandInternal("remoteKey", IN_RemoteKeyboard, &IN_RemoteKeyboard_VAR);
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
    Cmd_AddCommandInternal("+sprint", IN_SprintDown, &IN_SprintDown_VAR);
    Cmd_AddCommandInternal("-sprint", IN_SprintUp, &IN_SprintUp_VAR);
    Cmd_AddCommandInternal("+nightvision", IN_NightVisionDown, &IN_NightVisionDown_VAR);
    Cmd_AddCommandInternal("-nightvision", IN_NightVisionUp, &IN_NightVisionUp_VAR);
    cl_analog_attack_threshold = Dvar_RegisterFloat("cl_analog_attack_threshold", 0.80000001, 0.000099999997, 1.0, v1, v0);
    cl_stanceHoldTime = Dvar_RegisterInt(
        "cl_stanceHoldTime",
        300,
        0,
        1000,
        0,
        "The time to hold the stance button before the player goes prone");
    cl_freemove = Dvar_RegisterInt("cl_freemove", 0, 0, 3, 0x80u, "Fly about the level");
    cl_freemoveScale = Dvar_RegisterFloat("cl_freemoveScale", 1.0, 0.0, 5.0, v3, v2);
}

