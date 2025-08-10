#pragma once

#ifndef KISAK_SP
#error This File is SinglePlayer Only
#endif

enum netadrtype_t : __int32
{
	NA_BOT = 0x0,
	NA_BAD = 0x1,
	NA_LOOPBACK = 0x2,
	NA_BROADCAST = 0x3,
	NA_IP = 0x4,
};

enum netsrc_t : __int32
{
	NS_CLIENT1 = 0x0,
	NS_SERVER = 0x1,
	NS_MAXCLIENTS = 0x1,
	NS_PACKET = 0x2,
};

struct __declspec(align(4)) netadr_t
{
	netadrtype_t type;
	unsigned __int8 ip[4];
	unsigned __int16 port;
};

struct netchan_t
{
	int outgoingSequence;
};
