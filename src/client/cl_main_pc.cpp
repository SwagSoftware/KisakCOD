#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "client.h"

//      int __cdecl Cmd_LocalControllerIndex(void) 8217b9c8 f i cl_main_xenon.obj
//      void __cdecl CL_ControllerInserted(int) 8217ba30 f   cl_main_xenon.obj
//      void __cdecl CL_ControllerRemoved(int) 8217bad8 f   cl_main_xenon.obj
//      void __cdecl CL_Live_EnableSplitscreenControls(void) 8217bc78 f   cl_main_xenon.obj
//      void __cdecl CL_Live_DisableSplitscreenControls(void) 8217bc88 f   cl_main_xenon.obj
//      void __cdecl CL_Xenon_RegisterCommands(void) 8217bf50 f   cl_main_xenon.obj
//      void __cdecl CL_Xenon_RegisterDvars(void) 8217c050 f   cl_main_xenon.obj
//      void __cdecl CL_Live_ShowFriendsList(void) 8217bb90 f   cl_main_xenon.obj
//      void __cdecl CL_Live_SignOut(void)    8217bc00 f   cl_main_xenon.obj
//      void __cdecl CL_Live_SignIn(void)     8217bc08 f   cl_main_xenon.obj
//      void __cdecl CL_UpdateGamerProfile(void) 8217bdc0 f   cl_main_xenon.obj
//      void __cdecl CL_UpdateDvarsFromProfile(void) 8217be50 f   cl_main_xenon.obj
//      void __cdecl CL_Live_SignInLive(void) 8217bee0 f   cl_main_xenon.obj

int cl_multi_gamepads_enabled;
int cl_controller_in_use;
int cl_last_controller_input;