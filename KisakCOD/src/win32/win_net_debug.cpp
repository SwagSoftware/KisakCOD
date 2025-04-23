#include "win_net.h"
#include "win_net_debug.h"
#include <script/scr_vm.h>

static int g_debugClient;

static int g_debugPacketPos[1];
static int sys_debugMessageType[1];

unsigned __int8 g_debugPacket[1][8192];

static int g_debugReadBytesRemote;
static int g_debugReadBytesSent;
static int g_debugWriteBytes;

unsigned int ip_debugSocket[2];
unsigned int ip_debugServerSocket[2];

char *g_debugReadBytes;

// KISAKTODO
// track_static_alloc_internal(g_debugPacket, 0x2000, "g_debugPacket", 9);

int __cdecl Sys_IsRemoteDebugClient()
{
	return g_debugClient;
}

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
			ip_debugServerSocket[i] = NET_TCPIPSocket((char*)"localhost", i + 28970, 0);
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

void Sys_SendDebugReadBytesInternal()
{
	const char *v0; // eax
	const char *v1; // eax

	if (*g_debugReadBytes != g_debugReadBytesSent)
	{
		g_debugReadBytesSent = *g_debugReadBytes;
		if (!ip_debugSocket[1])
			MyAssertHandler(".\\win32\\win_net.cpp", 1728, 0, "%s", "ip_debugSocket[DEBUG_SOCKET_MSG_REPLY_CHANNEL]");
		if (send(ip_debugSocket[1], g_debugReadBytes, 4, 0) == -1)
		{
			v0 = NET_ErrorString();
			v1 = va("Sys_SendDebugReadBytes: %s", v0);
			Sys_DebugSocketError(v1);
		}
	}
}

void __cdecl Sys_SendDebugReadBytes(int read)
{
	*g_debugReadBytes += read;
	if (*g_debugReadBytes - g_debugReadBytesSent >= 0x2000)
		Sys_SendDebugReadBytesInternal();
}

int __cdecl Sys_ReadDebugSocketData(char *buffer, int len, int blocking)
{
	const char *v4; // eax
	const char *v5; // eax
	int err; // [esp+0h] [ebp-10h]
	int read; // [esp+Ch] [ebp-4h]

	while (1)
	{
		while (1)
		{
			if (!len)
				return 1;
			if (!ip_debugSocket[0])
				MyAssertHandler(".\\win32\\win_net.cpp", 1764, 0, "%s", "ip_debugSocket[0]");
			read = recvfrom(ip_debugSocket[0], buffer, len, 0, 0, 0);
			if (read)
				break;
		block:
			if (!blocking)
				return 0;
			Sys_SendDebugReadBytesInternal();
			NET_Sleep(1u);
		}
		if (read == -1)
			break;
		Sys_SendDebugReadBytes(read);
		if (read > len)
			MyAssertHandler(".\\win32\\win_net.cpp", 1793, 0, "%s", "read <= remaining");
		len -= read;
		buffer += read;
	}
	err = WSAGetLastError();
	if (err == 10035)
		goto block;
	if (err == 10054)
	{
		Sys_DebugSocketError("Sys_ReadDebugSocketData: Socket closed");
	}
	else
	{
		v4 = NET_ErrorString();
		v5 = va("Sys_ReadDebugSocketData: %s", v4);
		Sys_DebugSocketError(v5);
	}
	return 0;
}

int __cdecl Sys_ReadDebugSocketInt()
{
	int value; // [esp+0h] [ebp-4h] BYREF

	Sys_ReadDebugSocketData((char*)&value, 4, 1);
	return value;
}