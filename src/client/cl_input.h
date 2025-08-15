#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "client.h"
#include <qcommon/msg.h>

struct kbutton_t // sizeof=0x14
{                                       // ...
    int down[2];                        // ...
    unsigned int downtime;
    unsigned int msec;
    bool active;                        // ...
    bool wasPressed;
    // padding byte
    // padding byte
};

void __cdecl TRACK_cl_input();
bool __cdecl IN_IsTempProneKeyActive();
int __cdecl IN_IsTempStanceKeyActive();
void IN_MLookDown();
void __cdecl IN_KeyDown(kbutton_t *b);
void __cdecl IN_KeyUp(kbutton_t *b);
// local variable allocation has failed, the output may be wrong!
float __cdecl CL_KeyState(kbutton_t *key);
void __cdecl CL_SetStance(int localClientNum, StanceState stance);
void __cdecl CL_ToggleStance(StanceState preferredStance);
void IN_UpDown();
void IN_UpUp();
void IN_DownDown();
void IN_DownUp();
void IN_LeftDown();
void IN_LeftUp();
void IN_RightDown();
void IN_RightUp();
void IN_ForwardDown();
void IN_ForwardUp();
void IN_BackDown();
void IN_BackUp();
void IN_LookupDown();
void IN_LookupUp();
void IN_LookdownDown();
void IN_LookdownUp();
void IN_MoveleftDown();
void IN_MoveleftUp();
void IN_MoverightDown();
void IN_MoverightUp();
void IN_SpeedDown();
void IN_SpeedUp();
void IN_StrafeDown();
void IN_StrafeUp();
void IN_Attack_Down();
void IN_Attack_Up();
void IN_Breath_Down();
void IN_Breath_Up();
void IN_MeleeBreath_Down();
void IN_MeleeBreath_Up();
void IN_Frag_Down();
void IN_Frag_Up();
void IN_Smoke_Down();
void IN_Smoke_Up();
void __cdecl IN_BreathSprint_Down();
void __cdecl IN_BreathSprint_Up();
void IN_Melee_Down();
void IN_Melee_Up();
void IN_Activate_Down();
void IN_Activate_Up();
void IN_Reload_Down();
void IN_Reload_Up();
void IN_UseReload_Down();
void IN_UseReload_Up();
void IN_LeanLeft_Down();
void IN_LeanLeft_Up();
void IN_LeanRight_Down();
void IN_LeanRight_Up();
void IN_Prone_Down();
void IN_Prone_Up();
void IN_Stance_Down();
void IN_Stance_Up();
void __cdecl IN_CenterView();
void __cdecl IN_RemoteKeyboard();
void __cdecl IN_RemoteMouseMove();
void IN_ToggleADS();
void IN_LeaveADS();
void IN_Throw_Down();
void IN_Throw_Up();
void IN_ToggleADS_Throw_Down();
void IN_ToggleADS_Throw_Up();
void IN_Speed_Throw_Down();
void IN_Speed_Throw_Up();
void IN_LowerStance();
void IN_RaiseStance();
void IN_ToggleCrouch();
void IN_ToggleProne();
void IN_GoProne();
void IN_GoCrouch();
void IN_GoStandDown();
void IN_GoStandUp();
void __cdecl IN_SprintDown();
void __cdecl IN_SprintUp();
void __cdecl IN_NightVisionDown();
void __cdecl IN_NightVisionUp();
void __cdecl CL_AdjustAngles();
void CL_StanceButtonUpdate();
void __cdecl CL_AddCurrentStanceToCmd(usercmd_s *cmd);
void __cdecl CL_KeyMove(usercmd_s *cmd);
int __cdecl CL_AllowInput();
void __cdecl CL_GamepadMove(usercmd_s *cmd);
void __cdecl CL_GetMouseMovement(clientActive_t *cl, float *mx, float *my);
void __cdecl CL_MouseMove(usercmd_s *cmd);
void __cdecl CL_UpdateCmdButton(int *cmdButtons, int kbButton, int buttonFlag);
void __cdecl CL_CmdButtons(usercmd_s *cmd);
void __cdecl CL_SetUsercmdButtonsWeapons(int buttons, int weapon, int offhand);
void __cdecl CL_FinishMove(usercmd_s *cmd);
int __cdecl CG_HandleLocationSelectionInput(int localClientNum, usercmd_s *cmd);
void __cdecl CL_CreateCmd(usercmd_s *result);
void __cdecl CL_CreateNewCommands();
void __cdecl CL_WritePacket();
void __cdecl CL_ClearClientCommandPacket();
void __cdecl CL_ClearClientThinkPacket();
void PausedModelPreviewerGamepad();
void __cdecl CL_Input(int localClientNum);
void __cdecl CL_ShutdownInput();
void __cdecl CL_ClearKeys(int localClientNum);
void IN_MLookUp();
void __cdecl CL_InitInput();

// LWSS ADD - Mouse from PC (MP)
void __cdecl CL_ShowSystemCursor(bool show);
int __cdecl CL_MouseEvent(int x, int y, int dx, int dy);
// LWSS END

extern const dvar_t *cl_stanceHoldTime;
extern const dvar_t *cl_analog_attack_threshold;
extern const dvar_t *cl_yawspeed;
extern const dvar_t *cl_upspeed;
extern const dvar_t *cl_sidespeed;
extern const dvar_t *cl_pitchspeed;
extern const dvar_t *cl_forwardspeed;
extern const dvar_t *cl_anglespeedkey;