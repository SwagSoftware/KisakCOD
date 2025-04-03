#pragma once
#include <client_mp/client_mp.h>


struct __declspec(align(4)) pmove_t // sizeof=0x110
{                                       // ...
    playerState_s *ps;                  // ...
    usercmd_s cmd;                      // ...
    usercmd_s oldcmd;                   // ...
    int tracemask;                      // ...
    int numtouch;
    int touchents[32];
    float mins[3];                      // ...
    float maxs[3];                      // ...
    float xyspeed;
    int proneChange;
    float maxSprintTimeMultiplier;
    bool mantleStarted;                 // ...
    // padding byte
    // padding byte
    // padding byte
    float mantleEndPos[3];              // ...
    int mantleDuration;                 // ...
    int viewChangeTime;                 // ...
    float viewChange;
    unsigned __int8 handler;            // ...
    // padding byte
    // padding byte
    // padding byte
};

// bg_jump
extern const dvar_t *jump_height;
extern const dvar_t *jump_stepSize;
extern const dvar_t *jump_slowdownEnable;
extern const dvar_t *jump_ladderPushVel;
extern const dvar_t *jump_spreadAdd;

// bg_weapons
extern unsigned int bg_lastParsedWeaponIndex;