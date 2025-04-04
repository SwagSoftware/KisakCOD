#include "win_net.h"

static int g_debugClient;

static int g_debugPacketPos[1];
static int sys_debugMessageType[1];

static unsigned char g_debugPacket[0x2000];

static int g_debugReadBytesRemote;
static int g_debugReadBytesSent;
static int g_debugWriteBytes;
static int g_debugReadBytes;

// KISAKTODO
// track_static_alloc_internal(g_debugPacket, 0x2000, "g_debugPacket", 9);

void __cdecl NET_ShutdownDebug()
{
	int i; // [esp+0h] [ebp-4h]

	for (i = 0; i < 2; ++i)
	{
		if (ip_debugSocket[i])
		{
			closesocket(ip_debugSocket[i]);
			ip_debugSocket[i] = 0;
		}
		if (ip_debugServerSocket[i])
		{
			closesocket(ip_debugServerSocket[i]);
			ip_debugServerSocket[i] = 0;
		}
	}
	if (Sys_IsRemoteDebugClient())
		Scr_ShutdownRemoteClient(0);
}

int NET_InitDebugStreams()
{
	int result; // eax
	int i; // [esp+0h] [ebp-4h]
	int ia; // [esp+0h] [ebp-4h]

	for (i = 0; i < 2; ++i)
	{
		ip_debugServerSocket[i] = 0;
		ip_debugSocket[i] = 0;
		result = i + 1;
	}
	for (ia = 0; ia < 1; ++ia)
	{
		sys_debugMessageType[ia] = 0;
		g_debugPacketPos[ia] = 0;
		result = ia + 1;
	}
	g_debugReadBytes = 0;
	g_debugReadBytesSent = 0;
	g_debugWriteBytes = 0;
	g_debugReadBytesRemote = 0;
	return result;
}

void NET_InitDebug()
{
	g_debugClient = 0;
	NET_InitDebugStreams();
}

void NET_RestartDebug()
{
	NET_ShutdownDebug();
	NET_InitDebug();
}

void Sys_DebugSocketError(const char* message)
{
	NET_RestartDebug();
	Com_Printf(16, "%s\n", message);
}

void __cdecl Sys_Listen_f()
{
	const char* v0; // eax
	const char* v1; // eax
	int i; // [esp+0h] [ebp-4h]

	if (!com_sv_running->current.enabled)
	{
		if (g_debugClient)
			Com_Error(ERR_DROP, "Net connection already started");
		for (i = 0; i < 2; ++i)
		{
			ip_debugServerSocket[i] = NET_TCPIPSocket("localhost", i + 28970, 0);
			if (!ip_debugServerSocket[i])
			{
				Sys_DebugSocketError("Sys_Listen_f: failed to connect");
				return;
			}
			if (listen(ip_debugServerSocket[i], 5) == -1)
			{
				v0 = NET_ErrorString();
				v1 = va("Sys_Listen_f: listen: %s\n", v0);
				Sys_DebugSocketError(v1);
				return;
			}
		}
		g_debugClient = 1;
		SystemParametersInfoA(0x2001u, 0, 0, 3u);
	}
}