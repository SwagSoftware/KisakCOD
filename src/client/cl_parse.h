#pragma once
#include <qcommon/msg_mp.h>

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "client.h"

void __fastcall TRACK_cl_parse();
void __fastcall SHOWNET(msg_t *msg, char *s);
void __fastcall CL_ParsePacketEntities(clientActive_t *cl, msg_t *msg, clSnapshot_t *newframe);
void __fastcall CL_ParseSnapshot(msg_t *msg);
void __fastcall CL_ParseGamestate(char *configstrings);
void __fastcall CL_ParseServerCommands(msg_t *msg);
void __fastcall CL_RecordServerCommands(serverCommands_s *serverCommands);
void __fastcall CL_ParseCommandString(serverCommands_s *serverCommands);
void __fastcall CL_ParseServerMessage(msg_t *msg);
