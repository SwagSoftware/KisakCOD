// LWSS: This file is inspired by Jedi Academy, unsure if it's accurate to COD4
// Basically just to hold the `team_t` in a file that can be easily included without recursion

#pragma once


enum team_t : __int32
{
    TEAM_FREE = 0x0,
    TEAM_BAD = 0x0,
    TEAM_AXIS = 0x1,
    TEAM_ALLIES = 0x2,
#ifdef KISAK_MP
    TEAM_SPECTATOR = 0x3,
    TEAM_NUM_TEAMS = 0x4,
#elif KISAK_SP
    TEAM_NEUTRAL = 0x3,
    TEAM_DEAD = 0x4,
    TEAM_NUM_TEAMS = 0x5,
#endif
};