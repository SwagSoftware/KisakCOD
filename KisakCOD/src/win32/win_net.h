#pragma once

#include <qcommon/net_chan_mp.h>

void		NET_Init(void);
void		NET_Shutdown(void);
void		NET_Restart(void);
void		NET_Config(bool enableNetworking);
void		NET_SendPacket(netsrc_t sock, int length, const void* data, netadr_t to);
const char* NET_ErrorString(void);

unsigned int __cdecl NET_TCPIPSocket(char* net_interface, int port, int type);