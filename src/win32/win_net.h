#pragma once

#ifdef KISAK_MP
#include <qcommon/net_chan_mp.h>
#elif KISAK_SP
#include <qcommon/net_chan.h>
#elif defined(KISAK_RADIANT)
enum netsrc_t : int { NS_CLIENT1 = 0, NS_SERVER = 1, NS_MAXCLIENTS = 1, NS_PACKET = 2 };
struct netchan_t  { int outgoingSequence; };
#ifndef KISAK_RADIANT_NETADR_DEFINED
#define KISAK_RADIANT_NETADR_DEFINED
struct netadr_t { int type; unsigned char ip[4]; unsigned short port; unsigned char ipx[10]; };
#endif
#endif


void		NET_Init(void);
void		NET_Shutdown(void);
void		NET_Restart(void);
void		NET_Config(bool enableNetworking);
void		NET_SendPacket(netsrc_t sock, int length, const void* data, netadr_t to);
const char* NET_ErrorString(void);
void		NET_Sleep(int msec);

uint32_t __cdecl NET_TCPIPSocket(const char *net_interface, int port, int type);

qboolean Sys_StringToAdr(const char *s, netadr_t *a);
char __cdecl Sys_SendPacket(int length, unsigned __int8 *data, netadr_t to);
