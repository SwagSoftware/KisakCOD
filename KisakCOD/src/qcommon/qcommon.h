#pragma once

#include "../universal/q_shared.h"

typedef enum
{
    SE_NONE = 0x0,
    SE_KEY = 0x1,
    SE_CHAR = 0x2,
    SE_CONSOLE = 0x3,
} sysEventType_t;

void QDECL Com_Printf(int channel, const char* fmt, ...);
void QDECL Com_Error(int code, const char* fmt, ...);