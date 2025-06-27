#pragma once

#include "msg_mp.h"

// number of old messages that must be kept on client and
// server for delta comrpession and ping estimation
#define	PACKET_BACKUP	32
#define	PACKET_MASK		(PACKET_BACKUP-1)

#define	MAX_PACKET_USERCMDS		32		// max number of usercmd_t in a packet

#define	PORT_ANY			-1

#define	MAX_RELIABLE_COMMANDS	128			// max string commands buffered for restransmit

enum netadrtype_t {
    NA_BOT,
    NA_BAD,					// an address lookup failed
    NA_LOOPBACK,
    NA_BROADCAST,
    NA_IP,
    NA_IPX,
    NA_BROADCAST_IPX
};

enum netsrc_t : __int32
{                                       // ...
    NS_CLIENT1 = 0x0,
    NS_SERVER = 0x1,
    NS_MAXCLIENTS = 0x1,
    NS_PACKET = 0x2,
};
inline netsrc_t &operator++(netsrc_t &e) {
    e = static_cast<netsrc_t>(static_cast<int>(e) + 1);
    return e;
}
inline netsrc_t &operator++(netsrc_t &e, int i)
{
    ++e;
    return e;
}

struct netadr_t {
    netadrtype_t	type;

    unsigned char  ip[4];
    unsigned short port;
    unsigned char  ipx[10];
};

struct ClientSnapshotData // sizeof=0x44
{
    int snapshotSize[8];
    int compressedSize[8];
    int index;
};

struct netProfilePacket_t // sizeof=0xC
{                                       // ...
    int iTime;
    int iSize;
    int bFragment;
};

struct netProfileStream_t // sizeof=0x2F0
{                                       // ...
    netProfilePacket_t packets[60];
    int iCurrPacket;
    int iBytesPerSecond;
    int iLastBPSCalcTime;
    int iCountedPackets;
    int iCountedFragments;
    int iFragmentPercentage;
    int iLargestPacket;
    int iSmallestPacket;
};

struct netProfileInfo_t // sizeof=0x5E0
{                                       // ...
    netProfileStream_t send;
    netProfileStream_t recieve;         // ...
};

struct netchan_t // sizeof=0x62C
{                                       // ...
    int outgoingSequence;
    netsrc_t sock;
    int dropped;
    int incomingSequence;
    netadr_t remoteAddress;             // ...
    int qport;
    int fragmentSequence;
    int fragmentLength;
    unsigned __int8* fragmentBuffer;
    int fragmentBufferSize;
    int unsentFragments;
    int unsentFragmentStart;
    int unsentLength;
    unsigned __int8* unsentBuffer;
    int unsentBufferSize;
    netProfileInfo_t prof;
};

struct fakedLatencyPackets_t // sizeof=0x50
{
    bool outbound;
    bool loopback;
    // padding byte
    // padding byte
    netsrc_t sock;
    netadr_t addr;
    unsigned int length;
    unsigned __int8 *data;
    int startTime;
    msg_t msg;
};

struct loopmsg_t // sizeof=0x580
{                                       // ...
    unsigned __int8 data[1400];
    int datalen;
    int port;
};

struct loopback_t // sizeof=0x5808
{                                       // ...
    loopmsg_t msgs[16];
    volatile unsigned int get;
    volatile unsigned int send;
};

struct clientHeader_t // sizeof=0x64C
{                                       // ...
    int state;                          // ...
    int sendAsActive;
    int deltaMessage;
    int rateDelayed;
    netchan_t netchan;                  // ...
    float predictedOrigin[3];
    int predictedOriginServerTime;      // ...
};

//
//qboolean	NET_CompareAdr(netadr_t a, netadr_t b);
//qboolean	NET_CompareBaseAdr(netadr_t a, netadr_t b);
//qboolean	NET_IsLocalAddress(netadr_t adr);
const char* NET_AdrToString(netadr_t a);
//qboolean	NET_StringToAdr(const char* s, netadr_t* a);
//qboolean	NET_GetLoopPacket(netsrc_t sock, netadr_t* net_from, msg_t* net_message);
//

struct netProfileInfo_t;
struct netProfileStream_t;

void __cdecl NetProf_PrepProfiling(netProfileInfo_t* prof);
void __cdecl NetProf_AddPacket(netProfileStream_t* pProfStream, int iSize, int bFragment);
void __cdecl NetProf_NewSendPacket(netchan_t* pChan, int iSize, int bFragment);
void __cdecl NetProf_NewRecievePacket(netchan_t* pChan, int iSize, int bFragment);
void __cdecl NetProf_UpdateStatistics(netProfileStream_t* pStream);
void __cdecl Net_DisplayProfile(int localClientNum);
char __cdecl FakeLag_DestroyPacket(unsigned int packet);
void __cdecl FakeLag_SendPacket_Real(unsigned int packet);
void __cdecl FakeLag_Init();
unsigned int __cdecl FakeLag_GetFreeSlot();
bool __cdecl FakeLag_HostingGameOrParty();
unsigned int __cdecl FakeLag_SendPacket(netsrc_t sock, int length, unsigned __int8* data, netadr_t to);
unsigned int __cdecl FakeLag_QueueIncomingPacket(bool loopback, netsrc_t sock, netadr_t* from, msg_t* msg);
void __cdecl FakeLag_ReceivePackets();
int __cdecl FakeLag_GetPacket(bool loopback, netsrc_t sock, netadr_t* net_from, msg_t* net_message);
void __cdecl FakeLag_Frame();
int __cdecl FakeLag_SendLaggedPackets();
void __cdecl FakeLag_Shutdown();
void __cdecl Netchan_Init(__int16 port);
void __cdecl Net_DumpProfile_f();
void __cdecl Netchan_Setup(
    netsrc_t sock,
    netchan_t* chan,
    netadr_t adr,
    int qport,
    char* outgoingBuffer,
    int outgoingBufferSize,
    char* incomingBuffer,
    int incomingBufferSize);
bool __cdecl Netchan_TransmitNextFragment(netchan_t* chan);
bool __cdecl Netchan_Transmit(netchan_t* chan, int length, char* data);
int __cdecl Netchan_Process(netchan_t* chan, msg_t* msg);
int __cdecl NET_CompareBaseAdrSigned(netadr_t* a, netadr_t* b);
bool __cdecl NET_CompareBaseAdr(netadr_t a, netadr_t b);
int __cdecl NET_CompareAdrSigned(netadr_t* a, netadr_t* b);
bool __cdecl NET_CompareAdr(netadr_t a, netadr_t b);
bool __cdecl NET_IsLocalAddress(netadr_t adr);
int __cdecl NET_GetClientPacket(netadr_t* net_from, msg_t* net_message);
int __cdecl NET_GetServerPacket(netadr_t* net_from, msg_t* net_message);
int __cdecl NET_GetLoopPacket_Real(netsrc_t sock, netadr_t* net_from, msg_t* net_message);
int __cdecl NET_GetLoopPacket(netsrc_t sock, netadr_t* net_from, msg_t* net_message);
void __cdecl NET_SendLoopPacket(netsrc_t sock, unsigned int length, unsigned __int8* data, netadr_t to);
char __cdecl NET_SendPacket(netsrc_t sock, int length, unsigned __int8* data, netadr_t to);
bool __cdecl NET_OutOfBandPrint(netsrc_t sock, netadr_t adr, const char* data);
bool __cdecl NET_OutOfBandData(netsrc_t sock, netadr_t adr, const unsigned __int8* format, int len);
bool __cdecl NET_OutOfBandVoiceData(netsrc_t sock, netadr_t adr, unsigned __int8* format, unsigned int len);
int __cdecl NET_StringToAdr(char* s, netadr_t* a);

extern const dvar_t* showpackets;
extern const dvar_t* fakelag_target;
extern const dvar_t* fakelag_packetloss;
extern const dvar_t* fakelag_currentjitter;
extern const dvar_t* fakelag_jitter;
extern const dvar_t* fakelag_current;
extern const dvar_t* msg_dumpEnts;
extern const dvar_t* net_profile;
extern const dvar_t* net_lanauthorize;
extern const dvar_t* packetDebug;
extern const dvar_t* showdrop;
extern const dvar_t* fakelag_jitterinterval;
extern const dvar_t* net_showprofile;
extern const dvar_t* msg_printEntityNums;
extern const dvar_t* msg_hudelemspew;

extern int g_qport;

void __cdecl Com_PacketEventLoop(netsrc_t client, msg_t* netmsg);
void __cdecl Com_DispatchClientPacketEvent(netadr_t adr, msg_t* netmsg);


extern ClientSnapshotData s_clientSnapshotData[64];