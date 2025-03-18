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

// commandLine should not include the executable name (argv[0])
void Com_Init(char* commandLine);
void Com_Frame(void);
void Com_Shutdown(void);

struct cmd_function_s
{
    cmd_function_s *next;
    const char *name;
    const char *autoCompleteDir;
    const char *autoCompleteExt;
    void (__cdecl *function)();
};

void Cmd_Init();

void Cmd_AddCommandInternal(const char* cmdName, void(__cdecl* function)(), cmd_function_s* allocedCmd);

cmd_function_s* Cmd_FindCommand(const char* cmdName);


/*
==============================================================

DVAR

==============================================================
*/

const dvar_s* Dvar_FindVar(const char* dvarName);
const dvar_s* Dvar_RegisterBool(
	const char* dvarName,
	bool value,
	word flags,
	const char* description);
const dvar_s* Dvar_RegisterVariant(
	const char* dvarName,
	byte type,
	word flags,
	DvarValue value,
	DvarLimits domain,
	const char* description);
/*
==============================================================

NET

==============================================================
*/

#define	PACKET_BACKUP	32	// number of old messages that must be kept on client and
// server for delta comrpession and ping estimation
#define	PACKET_MASK		(PACKET_BACKUP-1)

#define	MAX_PACKET_USERCMDS		32		// max number of usercmd_t in a packet

#define	PORT_ANY			-1

#define	MAX_RELIABLE_COMMANDS	128			// max string commands buffered for restransmit

typedef enum {
	NA_BOT,
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX
} netadrtype_t;

typedef enum {
	NS_CLIENT,
	NS_SERVER
} netsrc_t;

typedef struct {
	netadrtype_t	type;

	byte	ip[4];
	byte	ipx[10];

	unsigned short	port;
} netadr_t;

void		NET_Init(void);
void		NET_Shutdown(void);
void		NET_Restart(void);
void		NET_Config(qboolean enableNetworking);

void		NET_SendPacket(netsrc_t sock, int length, const void* data, netadr_t to);
void		QDECL NET_OutOfBandPrint(netsrc_t net_socket, netadr_t adr, const char* format, ...);
void		QDECL NET_OutOfBandData(netsrc_t sock, netadr_t adr, byte* format, int len);

qboolean	NET_CompareAdr(netadr_t a, netadr_t b);
qboolean	NET_CompareBaseAdr(netadr_t a, netadr_t b);
qboolean	NET_IsLocalAddress(netadr_t adr);
const char* NET_AdrToString(netadr_t a);
qboolean	NET_StringToAdr(const char* s, netadr_t* a);
qboolean	NET_GetLoopPacket(netsrc_t sock, netadr_t* net_from, msg_t* net_message);
void		NET_Sleep(int msec);

/*
==============================================================

MISC

==============================================================
*/

#define RoundUp(N, M) ((N) + ((unsigned int)(M)) - (((unsigned int)(N)) % ((unsigned int)(M))))
#define RoundDown(N, M) ((N) - (((unsigned int)(N)) % ((unsigned int)(M))))
