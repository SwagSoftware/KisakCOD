#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "client.h"

#include <qcommon/msg.h>

void __cdecl TRACK_cl_parse();
void __cdecl SHOWNET(msg_t *msg, char *s);
void __cdecl CL_ParsePacketEntities(clientActive_t *cl, msg_t *msg, clSnapshot_t *newframe);
void __cdecl CL_ParseSnapshot(msg_t *msg);
void __cdecl CL_ParseGamestate(char *configstrings);
void __cdecl CL_ParseServerCommands(msg_t *msg);
void __cdecl CL_RecordServerCommands(serverCommands_s *serverCommands);
void __cdecl CL_ParseCommandString(serverCommands_s *serverCommands);
void __cdecl CL_ParseServerMessage(msg_t *msg);
