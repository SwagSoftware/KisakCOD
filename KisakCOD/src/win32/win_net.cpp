// net_wins.c
//Anything above this #include will be ignored by the compiler
//#include "../qcommon/exe_headers.h"

#include "win_local.h"
#include <qcommon/net_chan_mp.h>
#include "win_net.h"
#include "win_net_debug.h"
#include <qcommon/mem_track.h>

static WSADATA	winsockdata;
static qboolean	winsockInitialized = qfalse;
static qboolean usingSocks = qfalse;
static qboolean networkingEnabled = qfalse;

static const dvar_t	*net_noudp;
static const dvar_t	*net_noipx;
static const dvar_t	*net_forcenonlocal;
static const dvar_t	*net_socksEnabled;
static const dvar_t	*net_socksServer;
static const dvar_t	*net_socksPort;
static const dvar_t	*net_socksUsername;
static const dvar_t	*net_socksPassword;
static struct sockaddr	socksRelayAddr;

#ifdef _XBOX
SOCKET	v_socket = INVALID_SOCKET;
#endif
static SOCKET	ip_socket = INVALID_SOCKET;
static SOCKET	socks_socket = INVALID_SOCKET;
static SOCKET	ipx_socket = INVALID_SOCKET;

#define	MAX_IPS		16
static	int		numIP;
static	byte	localIP[MAX_IPS][4];

//=============================================================================

/*
====================
NET_ErrorString
====================
*/
const char *NET_ErrorString( void ) {
	int		code;

	code = WSAGetLastError();
	switch( code ) {
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSWSAECONNABORTEDAEINTR";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	case WSAEHOSTUNREACH: return "WSAEHOSTUNREACH";
	default: return "NO ERROR";
	}
}

void NetadrToSockadr( netadr_t *a, struct sockaddr *s ) {
	memset( s, 0, sizeof(*s) );

	if( a->type == NA_BROADCAST ) {
		((struct sockaddr_in *)s)->sin_family = AF_INET;
		((struct sockaddr_in *)s)->sin_port = a->port;
		((struct sockaddr_in *)s)->sin_addr.s_addr = INADDR_BROADCAST;
	}
	else if( a->type == NA_IP ) {
		((struct sockaddr_in *)s)->sin_family = AF_INET;
		((struct sockaddr_in *)s)->sin_addr.s_addr = *(int *)&a->ip;
		((struct sockaddr_in *)s)->sin_port = a->port;
	}
#ifndef _XBOX
	else if( a->type == NA_IPX ) {
		((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
		memcpy( ((struct sockaddr_ipx *)s)->sa_netnum, &a->ipx[0], 4 );
		memcpy( ((struct sockaddr_ipx *)s)->sa_nodenum, &a->ipx[4], 6 );
		((struct sockaddr_ipx *)s)->sa_socket = a->port;
	}
	else if( a->type == NA_BROADCAST_IPX ) {
		((struct sockaddr_ipx *)s)->sa_family = AF_IPX;
		memset( ((struct sockaddr_ipx *)s)->sa_netnum, 0, 4 );
		memset( ((struct sockaddr_ipx *)s)->sa_nodenum, 0xff, 6 );
		((struct sockaddr_ipx *)s)->sa_socket = a->port;
	}
#endif //_XBOX
}


void SockadrToNetadr( struct sockaddr *s, netadr_t *a ) {
	if (s->sa_family == AF_INET) {
		a->type = NA_IP;
		*(int *)&a->ip = ((struct sockaddr_in *)s)->sin_addr.s_addr;
		a->port = ((struct sockaddr_in *)s)->sin_port;
	}
#ifndef _XBOX
	else if( s->sa_family == AF_IPX ) {
		a->type = NA_IPX;
		memcpy( &a->ipx[0], ((struct sockaddr_ipx *)s)->sa_netnum, 4 );
		memcpy( &a->ipx[4], ((struct sockaddr_ipx *)s)->sa_nodenum, 6 );
		a->port = ((struct sockaddr_ipx *)s)->sa_socket;
	}
#endif //_XBOX
}


/*
=============
Sys_StringToAdr

idnewt
192.246.40.70
12121212.121212121212
=============
*/
#define DO(src,dest)	\
	copy[0] = s[src];	\
	copy[1] = s[src + 1];	\
	sscanf (copy, "%x", &val);	\
	((struct sockaddr_ipx *)sadr)->dest = val


qboolean Sys_StringToSockaddr( const char *s, struct sockaddr *sadr )
{
#ifndef _XBOX
	struct hostent	*h;
	int		val;
	char	copy[MAX_STRING_CHARS];
#endif

	
	memset( sadr, 0, sizeof( *sadr ) );

	// check for an IPX address
	if( ( strlen( s ) == 21 ) && ( s[8] == '.' ) )
	{
#ifdef _XBOX
		assert(!"IPX not supported on XBox");
#else
		((struct sockaddr_ipx *)sadr)->sa_family = AF_IPX;
		((struct sockaddr_ipx *)sadr)->sa_socket = 0;
		copy[2] = 0;
		DO(0, sa_netnum[0]);
		DO(2, sa_netnum[1]);
		DO(4, sa_netnum[2]);
		DO(6, sa_netnum[3]);
		DO(9, sa_nodenum[0]);
		DO(11, sa_nodenum[1]);
		DO(13, sa_nodenum[2]);
		DO(15, sa_nodenum[3]);
		DO(17, sa_nodenum[4]);
		DO(19, sa_nodenum[5]);
#endif
	}
	else
	{
		((struct sockaddr_in *)sadr)->sin_family = AF_INET;
		((struct sockaddr_in *)sadr)->sin_port = 0;

		if( s[0] >= '0' && s[0] <= '9' )
		{
			*(int *)&((struct sockaddr_in *)sadr)->sin_addr = inet_addr(s);
		}
		else
		{
#ifdef _XBOX
			assert(!"gethostbyname() - unsupported on XBox");
#else
			if( ( h = gethostbyname( s ) ) == 0 ) {	// NOT SUPPORTED ON XBOX!
				return (qboolean)0;

			}
			*(int *)&((struct sockaddr_in *)sadr)->sin_addr = *(int *)h->h_addr_list[0];
#endif
		}
	}
	
	return qtrue;
}

#undef DO

/*
=============
Sys_StringToAdr

idnewt
192.246.40.70
=============
*/
qboolean Sys_StringToAdr( const char *s, netadr_t *a ) {
	struct sockaddr sadr;
	
	if ( !Sys_StringToSockaddr( s, &sadr ) ) {
		return qfalse;
	}
	
	SockadrToNetadr( &sadr, a );
	return qtrue;
}

//=============================================================================

/*
==================
Sys_GetPacket

Never called by the game logic, just the system event queing
==================
*/
int __cdecl Sys_GetPacket(netadr_t *net_from, msg_t *net_message)
{
	const char *v2; // eax
	const char *v3; // eax
	sockaddr from; // [esp+8h] [ebp-28h] BYREF
	int err; // [esp+1Ch] [ebp-14h]
	int ret; // [esp+20h] [ebp-10h]
	int protocol; // [esp+24h] [ebp-Ch]
	int fromlen; // [esp+28h] [ebp-8h] BYREF
	unsigned int net_socket; // [esp+2Ch] [ebp-4h]

	for (protocol = 0; protocol < 2; ++protocol)
	{
		if (protocol)
			net_socket = ipx_socket;
		else
			net_socket = ip_socket;
		if (net_socket)
		{
			fromlen = 16;
			ret = recvfrom(net_socket, (char*)net_message->data, net_message->maxsize, 0, &from, &fromlen);
			if (ret == -1)
			{
				err = WSAGetLastError();
				if (err != 10035 && err != 10054)
				{
					v2 = NET_ErrorString();
					Com_PrintError(16, "NET_GetPacket: %s\n", v2);
				}
			}
			else
			{
				if (net_socket == ip_socket)
				{
					*&from.sa_data[6] = 0;
					*&from.sa_data[10] = 0;
				}
				if (usingSocks && net_socket == ip_socket && !memcmp(&from, &socksRelayAddr, fromlen))
				{
					if (ret < 10
						|| *net_message->data
						|| net_message->data[1]
						|| net_message->data[2]
						|| net_message->data[3] != 1)
					{
						continue;
					}
					net_from->type = NA_IP;
					*net_from->ip = *(net_message->data + 1);
					net_from->port = *(net_message->data + 4);
					net_message->readcount = 10;
				}
				else
				{
					SockadrToNetadr(&from, net_from);
					net_message->readcount = 0;
				}
				if (ret != net_message->maxsize)
				{
					net_message->cursize = ret;
					return 1;
				}
				v3 = NET_AdrToString(*net_from);
				Com_Printf(16, "Oversize packet from %s\n", v3);
			}
		}
	}
	return 0;
}

//=============================================================================

#ifdef _XBOX
/*
==================
Sys_SendVoicePacket
==================
*/
void Sys_SendVoicePacket( int length, const void *data, netadr_t to ) {
	int				ret;
	struct sockaddr	addr;

	// check for valid packet intentions (direct send or broadcast)
	//
	if( to.type != NA_IP && to.type != NA_BROADCAST ) 
	{
		Com_Error( ERR_FATAL, "Sys_SendVoicePacket: bad address type" );
		return;
	}
	
	// check we have our voice socket set up
	//
	if( v_socket == INVALID_SOCKET) {
		return;
	}

	NetadrToSockadr( &to, &addr );
#ifdef SOF2_METRICS
	gXBL_NumberVoicePacketsSent++;
	gXBL_SizeVoicePacketsSent += length;
#endif
	/*if( usingSocks && to.type == NA_IP ) {
		vsocksBuf[0] = 0;	// reserved
		vsocksBuf[1] = 0;
		vsocksBuf[2] = 0;	// fragment (not fragmented)
		vsocksBuf[3] = 1;	// address type: IPV4
		*(int *)&vsocksBuf[4] = ((struct sockaddr_in *)&addr)->sin_addr.s_addr;
		*(short *)&vsocksBuf[8] = ((struct sockaddr_in *)&addr)->sin_port;
		memcpy( &vsocksBuf[10], data, length );
		ret = sendto( v_socket, vsocksBuf, length+10, 0, &socksRelayAddr, sizeof(socksRelayAddr) );
	}
	else {*/
		ret = sendto( v_socket, (const char *)data, length, 0, &addr, sizeof(addr) );
	//}

	if( ret == SOCKET_ERROR ) {
		int err = WSAGetLastError();

		// wouldblock is silent
		if( err == WSAEWOULDBLOCK ) {
			return;
		}

		// some PPP links do not allow broadcasts and return an error
		if( ( err == WSAEADDRNOTAVAIL ) && ( ( to.type == NA_BROADCAST ) || ( to.type == NA_BROADCAST_IPX ) ) ) {
			return;
		}

		Com_DPrintf( "NET_SendVoicePacket: %s\n", NET_ErrorString() );
	}
}
#endif

static char socksBuf[4096];

/*
==================
Sys_SendPacket
==================
*/
char __cdecl Sys_SendPacket(int length, unsigned __int8 *data, netadr_t to)
{
	const char *v4; // eax
	int err; // [esp+0h] [ebp-20h]
	sockaddr addr; // [esp+4h] [ebp-1Ch] BYREF
	int ret; // [esp+14h] [ebp-Ch]
	unsigned int net_socket; // [esp+18h] [ebp-8h]

	net_socket = 0;
	switch (to.type)
	{
	case NA_BROADCAST:
		net_socket = ip_socket;
		break;
	case NA_IP:
		net_socket = ip_socket;
		break;
	case NA_IPX:
		net_socket = ipx_socket;
		break;
	case NA_BROADCAST_IPX:
		net_socket = ipx_socket;
		break;
	default:
		Com_Error(ERR_FATAL, "Sys_SendPacket: bad address type");
		break;
	}
	if (!net_socket)
		return 1;
	NetadrToSockadr(&to, &addr);
	if (usingSocks && to.type == NA_IP)
	{
		socksBuf[0] = 0;
		socksBuf[1] = 0;
		socksBuf[2] = 0;
		socksBuf[3] = 1;
		*(_DWORD *)&socksBuf[4] = *(_DWORD *)&addr.sa_data[2];
		*(_WORD *)&socksBuf[8] = *(_WORD *)addr.sa_data;
		memcpy((unsigned __int8 *)&socksBuf[10], data, length);
		ret = sendto(net_socket, socksBuf, length + 10, 0, &socksRelayAddr, 16);
	}
	else
	{
		ret = sendto(net_socket, (const char *)data, length, 0, &addr, 16);
	}
	if (ret != -1)
		return 1;
	err = WSAGetLastError();
	if (err == 10035)
		return 1;
	if (err == 10049 && (to.type == NA_BROADCAST || to.type == NA_BROADCAST_IPX))
		return 1;
	v4 = NET_ErrorString();
	Com_PrintError(16, "Sys_SendPacket: %s\n", v4);
	return 0;
}


//=============================================================================

bool __cdecl Sys_IsLANAddress_IgnoreSubnet(netadr_t adr)
{
	switch (adr.type)
	{
	case NA_LOOPBACK:
		return 1;
	case NA_BOT:
		return 1;
	case NA_IPX:
		return 1;
	}
	if (adr.type != NA_IP)
		return 0;
	if (adr.ip[0] == 10)
		return 1;
	if (adr.ip[0] == 127)
		return 1;
	if (adr.ip[0] == 169 && adr.ip[1] == 254)
		return 1;
	if (adr.ip[0] == 172 && (adr.ip[1] & 0xF0) == 0x10)
		return 1;
	return adr.ip[0] == 192 && adr.ip[1] == 168;
}

/*
==================
Sys_IsLANAddress

LAN clients will have their rate var ignored
==================
*/
bool __cdecl Sys_IsLANAddress(netadr_t adr)
{
	int i; // [esp+0h] [ebp-8h]

	if (Sys_IsLANAddress_IgnoreSubnet(adr))
		return 1;
	for (i = 0; i < numIP; ++i)
	{
		if (adr.ip[0] == localIP[i][0] && adr.ip[1] == localIP[i][1] && adr.ip[2] == localIP[i][2])
			return 1;
	}
	return 0;
}

/*
==================
Sys_ShowIP
==================
*/
void Sys_ShowIP(void) {
	int i;

	for (i = 0; i < numIP; i++) {
		Com_Printf( 16, "IP: %i.%i.%i.%i\n", localIP[i][0], localIP[i][1], localIP[i][2], localIP[i][3] );
	}
}


//=============================================================================


/*
====================
NET_IPSocket
====================
*/
unsigned int __cdecl NET_IPSocket(const char *net_interface, int port)
{
	const char *v2; // eax
	const char *v4; // eax
	const char *v5; // eax
	const char *v6; // eax
	sockaddr address; // [esp+0h] [ebp-24h] BYREF
	int _true; // [esp+18h] [ebp-Ch] BYREF
	int i; // [esp+1Ch] [ebp-8h] BYREF
	unsigned int newsocket; // [esp+20h] [ebp-4h]

	_true = 1;
	i = 1;
	if (net_interface)
		Com_Printf(16, "Opening IP socket: %s:%i\n", net_interface, port);
	else
		Com_Printf(16, "Opening IP socket: localhost:%i\n", port);
	newsocket = socket(2, 2, 17);
	if (newsocket == -1)
	{
		if (WSAGetLastError() != 10047)
		{
			v2 = NET_ErrorString();
			Com_PrintWarning(16, "WARNING: UDP_OpenSocket: socket: %s\n", v2);
		}
		return 0;
	}
	else if (ioctlsocket(newsocket, 0x8004667E, (unsigned long*)&_true) == -1)
	{
		v4 = NET_ErrorString();
		Com_PrintWarning(16, "WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", v4);
		return 0;
	}
	else if (setsockopt(newsocket, 0xFFFF, 32, (const char*)&i, 4) == -1)
	{
		v5 = NET_ErrorString();
		Com_PrintWarning(16, "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", v5);
		return 0;
	}
	else
	{
		if (net_interface && *net_interface && I_stricmp(net_interface, "localhost"))
			Sys_StringToSockaddr(net_interface, &address);
		else
			*&address.sa_data[2] = 0;
		if (port == -1)
			*address.sa_data = 0;
		else
			*address.sa_data = htons(port);
		address.sa_family = 2;
		if (bind(newsocket, &address, 16) == -1)
		{
			v6 = NET_ErrorString();
			Com_PrintWarning(16, "WARNING: UDP_OpenSocket: bind: %s\n", v6);
			closesocket(newsocket);
			return 0;
		}
		else
		{
			return newsocket;
		}
	}
}


/*
====================
NET_OpenSocks
====================
*/
void __cdecl NET_OpenSocks(u_short port)
{
	const char *v1; // eax
	const char *v2; // eax
	const char *v3; // eax
	const char *v4; // eax
	const char *v5; // eax
	const char *v6; // eax
	unsigned int v7; // [esp+0h] [ebp-8Ch]
	unsigned int v8; // [esp+10h] [ebp-7Ch]
	sockaddr address; // [esp+2Ch] [ebp-60h] BYREF
	unsigned __int8 buf[64]; // [esp+3Ch] [ebp-50h] BYREF
	int len; // [esp+80h] [ebp-Ch]
	hostent *h; // [esp+84h] [ebp-8h]
	int rfc1929; // [esp+88h] [ebp-4h]

	usingSocks = 0;
	Com_Printf(16, "Opening connection to SOCKS server.\n");
	socks_socket = socket(2, 1, 6);
	if (socks_socket == -1)
	{
		WSAGetLastError();
		v1 = NET_ErrorString();
		Com_PrintWarning(16, "WARNING: NET_OpenSocks: socket: %s\n", v1);
		return;
	}
	h = gethostbyname(net_socksServer->current.string);
	if (!h)
	{
		WSAGetLastError();
		v2 = NET_ErrorString();
		Com_PrintWarning(16, "WARNING: NET_OpenSocks: gethostbyname: %s\n", v2);
		return;
	}
	if (h->h_addrtype != 2)
	{
		Com_PrintWarning(16, "WARNING: NET_OpenSocks: gethostbyname: address type was not AF_INET\n");
		return;
	}
	address.sa_family = 2;
	*&address.sa_data[2] = **h->h_addr_list;
	*address.sa_data = htons(net_socksPort->current.unsignedInt);
	if (connect(socks_socket, &address, 16) == -1)
	{
		WSAGetLastError();
		v3 = NET_ErrorString();
		Com_PrintError(16, "NET_OpenSocks: connect: %s\n", v3);
		return;
	}
	rfc1929 = net_socksUsername->current.integer || net_socksPassword->current.integer;
	buf[0] = 5;
	if (rfc1929)
	{
		buf[1] = 2;
		len = 4;
	}
	else
	{
		buf[1] = 1;
		len = 3;
	}
	buf[2] = 0;
	if (rfc1929)
		buf[2] = 2;
	if (send(socks_socket, (const char*)buf, len, 0) == -1)
		goto LABEL_19;
	len = recv(socks_socket, (char*)buf, 64, 0);
	if (len == -1)
		goto LABEL_43;
	if (len != 2 || buf[0] != 5)
		goto LABEL_46;
	if (buf[1] && buf[1] != 2)
	{
		Com_Printf(16, "NET_OpenSocks: request denied\n");
		return;
	}
	if (buf[1] == 2)
	{
		v8 = strlen(net_socksUsername->current.string);
		v7 = strlen(net_socksPassword->current.string);
		buf[0] = 1;
		buf[1] = v8;
		if (v8)
			memcpy(&buf[2], (const void*)net_socksUsername->current.integer, v8);
		buf[v8 + 2] = v7;
		if (v7)
			memcpy(&buf[v8 + 3], (const void*)net_socksPassword->current.integer, v7);
		if (send(socks_socket, (const char*)buf, v8 + v7 + 3, 0) == -1)
		{
		LABEL_19:
			WSAGetLastError();
			v4 = NET_ErrorString();
			Com_PrintError(16, "NET_OpenSocks: send: %s\n", v4);
			return;
		}
		len = recv(socks_socket, (char*)buf, 64, 0);
		if (len == -1)
			goto LABEL_43;
		if (len != 2 || buf[0] != 1)
		{
		LABEL_46:
			Com_Printf(16, "NET_OpenSocks: bad response\n");
			return;
		}
		if (buf[1])
		{
			Com_Printf(16, "NET_OpenSocks: authentication failed\n");
			return;
		}
	}
	buf[0] = 5;
	buf[1] = 3;
	buf[2] = 0;
	buf[3] = 1;
	*&buf[4] = 0;
	*&buf[8] = htons(port);
	if (send(socks_socket, (const char*)buf, 10, 0) == -1)
	{
		WSAGetLastError();
		v5 = NET_ErrorString();
		Com_PrintError(16, "NET_OpenSocks: send: %s\n", v5);
	}
	len = recv(socks_socket, (char*)buf, 64, 0);
	if (len == -1)
	{
	LABEL_43:
		WSAGetLastError();
		v6 = NET_ErrorString();
		Com_PrintError(16, "NET_OpenSocks: recv: %s\n", v6);
		return;
	}
	if (len < 2 || buf[0] != 5)
		goto LABEL_46;
	if (buf[1])
	{
		Com_Printf(16, "NET_OpenSocks: request denied: %i\n", buf[1]);
	}
	else if (buf[3] == 1)
	{
		socksRelayAddr.sa_family = 2;
		*&socksRelayAddr.sa_data[2] = *&buf[4];
		*socksRelayAddr.sa_data = *&buf[8];
		*&socksRelayAddr.sa_data[6] = 0;
		*&socksRelayAddr.sa_data[10] = 0;
		usingSocks = 1;
	}
	else
	{
		Com_Printf(16, "NET_OpenSocks: relay address is not IPV4: %i\n", buf[3]);
	}
}


/*
=====================
NET_GetLocalAddress
=====================
*/
int numIP;
void __cdecl Sys_ShowIP()
{
	int i; // [esp+0h] [ebp-4h]

	for (i = 0; i < numIP; ++i)
		Com_Printf(16, "IP: %i.%i.%i.%i\n", localIP[i][0], localIP[i][1], localIP[i][2], localIP[i][3]);
}

#ifndef _XBOX

int NET_GetLocalAddress()
{
	int result; // eax
	u_long v1; // [esp+0h] [ebp-114h]
	char hostname[256]; // [esp+4h] [ebp-110h] BYREF
	hostent *hostInfo; // [esp+108h] [ebp-Ch]
	int n; // [esp+10Ch] [ebp-8h]
	char *p; // [esp+110h] [ebp-4h]

	if (gethostname(hostname, 256) == -1)
		return WSAGetLastError();
	hostInfo = gethostbyname(hostname);
	if (!hostInfo)
		return WSAGetLastError();
	Com_Printf(16, "Hostname: %s\n", hostInfo->h_name);
	n = 0;
	while (1)
	{
		p = hostInfo->h_aliases[n++];
		if (!p)
			break;
		Com_Printf(16, "Alias: %s\n", p);
	}
	result = hostInfo->h_addrtype;
	if (result == 2)
	{
		for (numIP = 0; ; ++numIP)
		{
			result = numIP;
			p = hostInfo->h_addr_list[numIP];
			if (!p || numIP >= 16)
				break;
			v1 = ntohl(*(_DWORD *)p);
			localIP[numIP][0] = *p;
			localIP[numIP][1] = p[1];
			localIP[numIP][2] = p[2];
			localIP[numIP][3] = p[3];
			Com_Printf(16, "IP: %i.%i.%i.%i\n", HIBYTE(v1), BYTE2(v1), BYTE1(v1), (unsigned __int8)v1);
		}
	}
	return result;
}

#else

// Xbox version supports the force option, so we can prime the
// system and hopefully be getting an IP while Com_Init() is running.
void NET_GetLocalAddress( bool force )
{
	XNADDR xnMyAddr;
	DWORD dwStatus;
	do
	{
	   // Repeat while pending; OK to do other work in this loop
	   dwStatus = XNetGetTitleXnAddr( &xnMyAddr );
	} while( dwStatus == XNET_GET_XNADDR_PENDING && force );

	// Error checking
	if( dwStatus == XNET_GET_XNADDR_NONE )
	{
		// If this wasn't the final (necessary) call, then don't worry
		if( force )
			assert(!"Error getting XBox title address.");
		return;
	}

	*(u_long*)&localIP[0] = xnMyAddr.ina.S_un.S_addr;
	*(u_long*)localIP[1] = 0;
	*(u_long*)localIP[2] = 0;
	*(u_long*)localIP[3] = 0;

	Com_Printf( "IP: %i.%i.%i.%i\n", localIP[0], localIP[1], localIP[2], localIP[3] );
}
#endif


/*
====================
NET_OpenIP
====================
*/
void __cdecl NET_OpenIP()
{
	const dvar_s *v0; // [esp+0h] [ebp-Ch]
	int i; // [esp+4h] [ebp-8h]
	const dvar_s *port; // [esp+8h] [ebp-4h]

	v0 = Dvar_RegisterString("net_ip", "localhost", 0x20u, "Network IP Address");
	port = Dvar_RegisterInt("net_port", 28960, 0xFFFF00000000LL, 0x20u, "Network port");
	for (i = 0; ; ++i)
	{
		if (i >= 10)
		{
			Com_PrintWarning(16, "WARNING: Couldn't allocate IP port\n");
			return;
		}
		ip_socket = NET_IPSocket(v0->current.string, i + port->current.integer);
		if (ip_socket)
			break;
	}
	Dvar_SetInt(port, i + port->current.integer);
	if (net_socksEnabled->current.enabled)
		NET_OpenSocks(i + port->current.integer);
	NET_GetLocalAddress();
}


/*
====================
NET_IPXSocket
====================
*/
unsigned int __cdecl NET_IPXSocket(int port)
{
	const char *v1; // eax
	const char *v3; // eax
	const char *v4; // eax
	const char *v5; // eax
	struct sockaddr address; // [esp+0h] [ebp-20h] BYREF
	int _true; // [esp+18h] [ebp-8h] BYREF
	unsigned int newsocket; // [esp+1Ch] [ebp-4h]

	_true = 1;
	newsocket = socket(6, 2, 1000);
	if (newsocket == -1)
	{
		if (WSAGetLastError() != 10047)
		{
			v1 = NET_ErrorString();
			Com_PrintWarning(16, "WARNING: IPX_Socket: socket: %s\n", v1);
		}
		return 0;
	}
	else if (ioctlsocket(newsocket, -2147195266, (unsigned long*)&_true) == -1)
	{
		v3 = NET_ErrorString();
		Com_PrintWarning(16, "WARNING: IPX_Socket: ioctl FIONBIO: %s\n", v3);
		return 0;
	}
	else if (setsockopt(newsocket, 0xFFFF, 32, (char*)&_true, 4) == -1)
	{
		v4 = NET_ErrorString();
		Com_PrintWarning(16, "WARNING: IPX_Socket: setsockopt SO_BROADCAST: %s\n", v4);
		return 0;
	}
	else
	{
		address.sa_family = 6;
		memset(address.sa_data, 0, 10);
		if (port == -1)
			*&address.sa_data[10] = 0;
		else
			*&address.sa_data[10] = htons(port);
		if (bind(newsocket, &address, 14) == -1)
		{
			v5 = NET_ErrorString();
			Com_PrintWarning(16, "WARNING: IPX_Socket: bind: %s\n", v5);
			closesocket(newsocket);
			return 0;
		}
		else
		{
			return newsocket;
		}
	}
}


/*
====================
NET_OpenIPX
====================
*/
void NET_OpenIPX( void ) {
	const dvar_s *v0; // eax

	v0 = Dvar_RegisterInt("net_port", 28960, 0xFFFF00000000LL, 0x20u, "Network Port");
	ipx_socket = NET_IPXSocket(v0->current.integer);
}



//===================================================================


BOOL __cdecl NET_GetDvars()
{
	BOOL modified; // [esp+0h] [ebp-4h]

	modified = 0;
	if (net_noudp)
		modified = net_noudp->modified;
	net_noudp = Dvar_RegisterBool("net_noudp", 0, 0x21u, "Disable UDP");
	if (net_noipx && net_noipx->modified)
		modified = 1;
	net_noipx = Dvar_RegisterBool("net_noipx", 0, 0x21u, "Disable IPX");
	if (net_socksEnabled && net_socksEnabled->modified)
		modified = 1;
	net_socksEnabled = Dvar_RegisterBool("net_socksEnabled", 0, 0x21u, "Enable network sockets");
	if (net_socksServer && net_socksServer->modified)
		modified = 1;
	net_socksServer = Dvar_RegisterString("net_socksServer", "", 0x21u, "Network socket server");
	if (net_socksPort && net_socksPort->modified)
		modified = 1;
	net_socksPort = Dvar_RegisterInt("net_socksPort", 1080, 0xFFFF00000000LL, 0x21u, "Network socket port");
	if (net_socksUsername && net_socksUsername->modified)
		modified = 1;
	net_socksUsername = Dvar_RegisterString("net_socksUsername", "", 0x21u, "Network socket username");
	if (net_socksPassword && net_socksPassword->modified)
		modified = 1;
	net_socksPassword = Dvar_RegisterString("net_socksPassword", "", 0x21u, "Network socket password");
	return modified;
}

/*
====================
NET_Config
====================
*/
void __cdecl NET_Config(int enableNetworking)
{
	int start; // [esp+0h] [ebp-Ch]
	int stop; // [esp+4h] [ebp-8h]
	BOOL modified; // [esp+8h] [ebp-4h]

	modified = NET_GetDvars();
	if (net_noudp->current.enabled && net_noipx->current.enabled)
		enableNetworking = 0;
	if (enableNetworking != networkingEnabled || modified)
	{
		if (enableNetworking == networkingEnabled)
		{
			if (enableNetworking)
			{
				stop = 1;
				start = 1;
			}
			else
			{
				stop = 0;
				start = 0;
			}
		}
		else
		{
			if (enableNetworking)
			{
				stop = 0;
				start = 1;
			}
			else
			{
				stop = 1;
				start = 0;
			}
			networkingEnabled = enableNetworking;
		}
		if (stop)
		{
			if (ip_socket && ip_socket != -1)
			{
				closesocket(ip_socket);
				ip_socket = 0;
			}
			if (socks_socket && socks_socket != -1)
			{
				closesocket(socks_socket);
				socks_socket = 0;
			}
			if (ipx_socket && ipx_socket != -1)
			{
				closesocket(ipx_socket);
				ipx_socket = 0;
			}
		}
		if (start)
		{
			if (!net_noudp->current.enabled)
				NET_OpenIP();
			if (!net_noipx->current.enabled)
				NET_OpenIPX();
		}
	}
}


/*
====================
NET_Init
====================
*/
void __cdecl NET_Init()
{
	int r; // [esp+0h] [ebp-4h]

	r = WSAStartup(0x101u, &winsockdata);
	if (r)
	{
		Com_PrintWarning(16, "WARNING: Winsock initialization failed, returned %d\n", r);
	}
	else
	{
		winsockInitialized = 1;
		Com_Printf(16, "Winsock Initialized\n");
		NET_GetDvars();
		NET_Config(1);
		NET_InitDebug();
	}
}


/*
====================
NET_Shutdown
====================
*/
void NET_Shutdown( void ) {
	if ( !winsockInitialized ) {
		return;
	}

	NET_Config( qfalse );
	WSACleanup();
	winsockInitialized = qfalse;
}


/*
====================
NET_Sleep

sleeps msec or until net socket is ready
====================
*/
// LWSS: Done
void NET_Sleep( int msec ) 
{
	Sleep(msec);
}


/*
====================
NET_Restart_f
====================
*/
void NET_Restart( void ) {
	NET_Config( networkingEnabled );
}


void __cdecl TRACK_win_net()
{
	track_static_alloc_internal(&winsockdata, 400, "winsockdata", 9);
}

int __cdecl NET_Select(unsigned int socket)
{
	const char* v2; // eax
	fd_set readfds; // [esp+0h] [ebp-220h] BYREF
	int err; // [esp+10Ch] [ebp-114h]
	fd_set writefds; // [esp+110h] [ebp-110h] BYREF
	timeval time; // [esp+218h] [ebp-8h] BYREF

	readfds.fd_count = 1;
	readfds.fd_array[0] = socket;
	writefds.fd_count = 1;
	writefds.fd_array[0] = socket;
	time.tv_sec = 5;
	time.tv_usec = 0;
	err = select(0, &readfds, &writefds, 0, &time);
	if (err)
	{
		if (err == -1)
		{
			v2 = NET_ErrorString();
			Com_PrintWarning(16, "WARNING: NET_Select: connect: %s\n", v2);
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		Com_Printf(16, "NET_Select: NET_Select: timeout\n");
		return 0;
	}
}

unsigned int __cdecl NET_TCPIPSocket(const char* net_interface, int port, int type)
{
	const char* v3; // eax
	const char* v5; // eax
	const char* v6; // eax
	const char* v7; // eax
	sockaddr_in address; // [esp+4h] [ebp-20h] BYREF
	int err; // [esp+18h] [ebp-Ch]
	int _true; // [esp+1Ch] [ebp-8h] BYREF
	unsigned int newsocket; // [esp+20h] [ebp-4h]

	_true = 1;
	if (net_interface)
		Com_Printf(16, "Opening IP socket: %s:%i\n", net_interface, port);
	else
		Com_Printf(16, "Opening IP socket: localhost:%i\n", port);
	newsocket = socket(2, 1, 6);
	if (newsocket == -1)
	{
		if (WSAGetLastError() != 10047)
		{
			v3 = NET_ErrorString();
			Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: socket: %s\n", v3);
		}
		return 0;
	}
	else
	{
		if (ioctlsocket(newsocket, 0x8004667E, (u_long*)&_true) == -1)
		{
			v5 = NET_ErrorString();
			Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: ioctl FIONBIO: %s\n", v5);
			return 0;
		}
		if (net_interface && *net_interface && I_stricmp(net_interface, "localhost"))
			Sys_StringToSockaddr(net_interface, (sockaddr*)&address);
		else
			address.sin_addr.S_un.S_addr = 0;
		if (port == -1)
			address.sin_port = 0;
		else
			address.sin_port = htons(port);
		address.sin_family = 2;
		if (type)
		{
			if (type == 1 && connect(newsocket, (const struct sockaddr*)&address, 16) == -1)
			{
				err = WSAGetLastError();
				if (err != 10035)
				{
					v7 = NET_ErrorString();
					Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: connect: %s\n", v7);
					closesocket(newsocket);
					return 0;
				}
				if (!NET_Select(newsocket))
				{
					Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: connect failed\n");
					closesocket(newsocket);
					return 0;
				}
			}
		}
		else if (bind(newsocket, (const struct sockaddr*)&address, 16) == -1)
		{
			v6 = NET_ErrorString();
			Com_PrintWarning(16, "WARNING: NET_TCPIPSocket: bind: %s\n", v6);
			closesocket(newsocket);
			return 0;
		}
		return newsocket;
	}
}