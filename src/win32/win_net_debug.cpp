#include "win_net.h"
#include "win_net_debug.h"
#include <script/scr_vm.h>
#include "win_local.h"

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

void __cdecl Sys_DebugSend(int channel, const char *buf, int len, const char *name);

void __cdecl Sys_WriteDebugSocketData(unsigned __int8 *buffer, int len)
{
	int pos; // [esp+0h] [ebp-10h]
	unsigned int copyLen; // [esp+8h] [ebp-8h]

	if (!com_errorEntered && Sys_IsRemoteDebugClient())
	{
		pos = g_debugPacketPos[0];
		while (len)
		{
			if (len <= 0)
				MyAssertHandler(".\\win32\\win_net.cpp", 1956, 0, "%s", "len > 0");
			copyLen = len;
			if (len > 0x2000 - pos)
				copyLen = 0x2000 - pos;
			memcpy(&g_debugPacket[0][pos], buffer, copyLen);
			buffer += copyLen;
			len -= copyLen;
			pos += copyLen;
			g_debugPacketPos[0] = pos;
			if (pos > 0x2000)
				MyAssertHandler(".\\win32\\win_net.cpp", 1967, 0, "%s", "pos <= MAX_DEBUG_PACKETLEN");
			if (pos == 0x2000)
			{
				pos = 0;
				g_debugPacketPos[0] = 0;
				Sys_DebugSend(0, (const char*)g_debugPacket, 0x2000, "Sys_WriteDebugSocketData");
			}
		}
	}
}

void __cdecl Sys_WriteDebugSocketInt(int value)
{
	Sys_WriteDebugSocketData((unsigned char*)&value, 4);
}

void __cdecl Sys_WriteDebugSocketMessageType(unsigned __int8 type)
{
	Sys_EnterCriticalSection(CRITSECT_DEBUG_SOCKET);
	Sys_WriteDebugSocketData(&type, 1);
}

void __cdecl Sys_EndWriteDebugSocket()
{
	Sys_LeaveCriticalSection(CRITSECT_DEBUG_SOCKET);
}

char *__cdecl Sys_ReadDebugSocketString()
{
	unsigned int buffer; // [esp+0h] [ebp-100Ch] BYREF
	char in[4100]; // [esp+4h] [ebp-1008h] BYREF

	Sys_ReadDebugSocketData((char*)&buffer, 4, 1);
	if (buffer >= 0x1000)
		MyAssertHandler(".\\win32\\win_net.cpp", 1830, 0, "%s\n\t(len) = %i", "(len < sizeof( text ))", buffer);
	Sys_ReadDebugSocketData(in, buffer, 1);
	in[buffer] = 0;
	return (char*)CopyString(in);
}

void __cdecl Sys_ReadDebugSocketStringBuffer(char *buffer, int len)
{
	char *text; // [esp+0h] [ebp-4h]

	text = Sys_ReadDebugSocketString();
	I_strncpyz(buffer, text, len);
	FreeString(text);
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

int __cdecl Sys_UpdateDebugSocket()
{
	const char *v1; // eax
	const char *v2; // eax
	unsigned __int8 type; // [esp+7h] [ebp-5h] BYREF
	int i; // [esp+8h] [ebp-4h]

	if (sys_debugMessageType[0])
		return sys_debugMessageType[0];
	for (i = 0; i < 2; ++i)
	{
		if (!ip_debugSocket[i])
		{
			if (!ip_debugServerSocket[i])
				return 0;
			ip_debugSocket[i] = accept(ip_debugServerSocket[i], 0, 0);
			if (ip_debugSocket[i] == -1)
			{
				ip_debugSocket[i] = 0;
				if (WSAGetLastError() != 10035)
				{
					v1 = NET_ErrorString();
					v2 = va("Sys_UpdateDebugSocket: %s\n", v1);
					Sys_DebugSocketError(v2);
				}
				return 0;
			}
		}
	}
	if (Sys_ReadDebugSocketMessageType(&type, 0))
	{
		if (type >= 0x2Eu)
			MyAssertHandler(".\\win32\\win_net.cpp", 2117, 0, "%s", "(unsigned)type < DEBUG_MSG_COUNT");
		sys_debugMessageType[0] = type;
	}
	return sys_debugMessageType[0];
}

int __cdecl Sys_ReadDebugSocketMessageType(unsigned __int8 *type, int blocking)
{
	return Sys_ReadDebugSocketData((char*)type, 1, blocking);
}

BOOL __cdecl Sys_DebugCanSend()
{
	return g_debugWriteBytes - g_debugReadBytesRemote <= 40960;
}

void __cdecl Sys_DebugSend(int channel, const char *buf, int len, const char *name)
{
	const char *v4; // eax
	const char *v5; // eax
	const char *v6; // eax
	const char *v7; // eax
	int err; // [esp+0h] [ebp-Ch]
	int debugReadBytesRemote; // [esp+4h] [ebp-8h] BYREF
	int read; // [esp+8h] [ebp-4h]

	if (ip_debugSocket[channel])
	{
		g_debugWriteBytes += len;
		while (!Sys_DebugCanSend())
		{
			while (1)
			{
				if (!ip_debugSocket[1])
					MyAssertHandler(".\\win32\\win_net.cpp", 1887, 0, "%s", "ip_debugSocket[DEBUG_SOCKET_MSG_REPLY_CHANNEL]");
				read = recvfrom(ip_debugSocket[1], (char*)&debugReadBytesRemote, 4, 0, 0, 0);
				if (read == -1)
					break;
				g_debugReadBytesRemote = debugReadBytesRemote;
			}
			err = WSAGetLastError();
			if (err != 10035)
			{
				if (err == 10054)
				{
					Sys_DebugSocketError("Sys_DebugSend: Socket closed");
				}
				else
				{
					v4 = NET_ErrorString();
					v5 = va("Sys_DebugSend: %s", v4);
					Sys_DebugSocketError(v5);
				}
				return;
			}
			if (!Sys_DebugCanSend())
				NET_Sleep(1u);
		}
		if (!ip_debugSocket[channel])
			MyAssertHandler(".\\win32\\win_net.cpp", 1911, 0, "%s", "ip_debugSocket[channel]");
		while (send(ip_debugSocket[channel], buf, len, 0) == -1)
		{
			if (WSAGetLastError() != 10035)
			{
				v6 = NET_ErrorString();
				v7 = va("%s: %s", name, v6);
				Sys_DebugSocketError(v7);
				return;
			}
			NET_Sleep(1u);
		}
	}
	else
	{
		Sys_DebugSocketError("Sys_DebugSend: Socket closed");
	}
}

void __cdecl Sys_FlushDebugSocketData()
{
	int pos; // [esp+0h] [ebp-8h]
	int channel; // [esp+4h] [ebp-4h]

	for (channel = 0; channel < 1; ++channel)
	{
		pos = g_debugPacketPos[channel];
		if (pos)
		{
			g_debugPacketPos[channel] = 0;
			Sys_DebugSend(channel, (const char*)g_debugPacket[channel], pos, "Sys_FlushDebugSocketData");
		}
	}
}

void __cdecl Sys_AckDebugSocket()
{
	sys_debugMessageType[0] = 0;
}