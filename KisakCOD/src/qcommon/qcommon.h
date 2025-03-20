#pragma once

#include "../universal/q_shared.h"
#include <xanim/xanim.h> // KISAKTODO: move SndCurve

typedef enum
{
    SE_NONE = 0x0,
    SE_KEY = 0x1,
    SE_CHAR = 0x2,
    SE_CONSOLE = 0x3,
} sysEventType_t;

struct msg_t // sizeof=0x28
{                                       // ...
	int overflowed;                     // ...
	int readOnly;                       // ...
	unsigned __int8* data;              // ...
	unsigned __int8* splitData;         // ...
	int maxsize;                        // ...
	int cursize;                        // ...
	int splitSize;                      // ...
	int readcount;                      // ...
	int bit;                            // ...
	int lastEntityRef;                  // ...
};
struct field_t // sizeof=0x118
{                                       // ...
	int cursor;                         // ...
	int scroll;
	int drawWidth;                      // ...
	int widthInPixels;                  // ...
	float charHeight;                   // ...
	int fixedSize;                      // ...
	char buffer[256];                   // ...
};
struct parseInfo_t // sizeof=0x420
{                                       // ...
	char token[1024];
	int lines;
	bool ungetToken;
	bool spaceDelimited;
	bool keepStringQuotes;
	bool csv;
	bool negativeNumbers;
	// padding byte
	// padding byte
	// padding byte
	const char* errorPrefix;
	const char* warningPrefix;
	int backup_lines;
	const char* backup_text;
	const char* parseFile;
};

extern dvar_t *com_developer;
extern int marker_common;
extern dvar_t *com_maxFrameTime;
extern dvar_t *com_statmon;
extern dvar_t *com_filter_output;
extern int com_skelTimeStamp;
extern dvar_t *com_useConfig;
extern unsigned int com_errorPrintsCount;
extern dvar_t *com_fixedtime;
extern dvar_t *fastfile_allowNoAuth;
extern dvar_t *com_logfile;
extern dvar_t *cl_paused;
extern dvar_t *com_timescale;
extern dvar_t *nextmap;
extern dvar_t *version;
extern dvar_t *com_sv_running;
extern float com_timescaleValue;
extern dvar_t *ui_errorTitle;
extern dvar_t *shortversion;
extern dvar_t *com_attractmodeduration;
extern dvar_t *com_attractmode;
extern dvar_t *cl_paused_simple;
extern dvar_t *sv_paused;
extern int com_fixedConsolePosition;
extern dvar_t *com_hiDef;
extern dvar_t *com_animCheck;
extern dvar_t *com_developer_script;
extern dvar_t *dev_timescale;
extern dvar_t *cl_useMapPreloading;
extern dvar_t *com_maxfps;
extern dvar_t *com_frameTime;
extern dvar_t *com_recommendedSet;
extern dvar_t *sv_useMapPreloading;
extern dvar_t *ui_errorMessage;
extern dvar_t *com_introPlayed;
extern dvar_t *com_wideScreen;
extern int com_errorEntered;
extern int com_frameNumber;
extern int com_consoleLogOpenFailed;
extern int com_missingAssetOpenFailed;
extern dvar_t *com_dedicated;

void QDECL Com_Printf(int channel, const char* fmt, ...);
void QDECL Com_Error(errorParm_t code, const char* fmt, ...);

// commandLine should not include the executable name (argv[0])
void Com_Init(char* commandLine);
void Com_Frame(void);

void __cdecl Com_ShutdownInternal(const char* finalmsg);
void Com_Shutdown(const char* finalmsg);

void __cdecl Com_ShutdownWorld();
void __cdecl Com_InitPlayerProfiles(int localClientNum);
void __cdecl Com_PrintMessage(int channel, char* msg, int error);
void __cdecl Com_LogPrintMessage(int channel, char* msg);
void Com_OpenLogFile();
void Com_DPrintf(int channel, char* fmt, ...);
void Com_PrintError(int channel, const char* fmt, ...);
void Com_PrintWarning(int channel, char* fmt, ...);
void __cdecl Com_SetLocalizedErrorMessage(char* localizedErrorMessage, const char* titleToken);
void __cdecl Com_SetErrorMessage(char* errorMessage);
char __cdecl Com_ErrorIsNotice(const char* errorMessage);
void __cdecl Com_PrintStackTrace();
void __cdecl Com_ErrorAbort();
void __cdecl Com_Quit_f();
void Com_ClearTempMemory();
void __cdecl Com_ParseCommandLine(char* commandLine);
int __cdecl Com_SafeMode();
void __cdecl Com_ForceSafeMode();
void __cdecl Com_StartupVariable(const char* match);
void __cdecl Info_Print(const char* s);
unsigned int* __cdecl Com_AllocEvent(int size);
void __cdecl Com_ClientPacketEvent();

void __cdecl Com_ServerPacketEvent();
void __cdecl Com_EventLoop();
void __cdecl Com_SetScriptSettings();
void __cdecl Com_RunAutoExec(int localClientNum, int controllerIndex);
void __cdecl Com_ExecStartupConfigs(int localClientNum, const char* configFile);
void Com_ErrorCleanup();
void Com_AddStartupCommands();
void __cdecl Com_Init_Try_Block_Function(char* commandLine);
void __cdecl Com_Error_f();
void __cdecl Com_Freeze_f();
void __cdecl Com_Crash_f();
void __cdecl Com_Assert_f();
void COM_PlayIntroMovies();
void Com_InitDvars();
void __cdecl Com_StartupConfigs(int localClientNum);
void Com_InitXAssets();
void __cdecl Com_WriteDefaultsToFile(char* filename);
void __cdecl Com_WriteConfig_f();
void __cdecl Com_WriteConfigToFile(int localClientNum, char* filename);
void __cdecl Com_WriteDefaults_f();
double __cdecl Com_GetTimescaleForSnd();
void __cdecl Com_AdjustMaxFPS(int* maxFPS);
void __cdecl Com_Frame_Try_Block_Function();
void __cdecl Com_WriteConfiguration(int localClientNum);
int __cdecl Com_ModifyMsec(int msec);
void Com_Statmon();
int Com_UpdateMenu();
void __cdecl Com_AssetLoadUI();
void __cdecl Com_CheckSyncFrame();
void Com_StartHunkUsers();
void __cdecl Com_CloseLogfiles();
bool __cdecl Com_LogFileOpen();
void __cdecl Com_Close();
void __cdecl Field_Clear(field_t* edit);
void __cdecl Com_Restart();
void __cdecl Com_SetWeaponInfoMemory(int source);
void __cdecl Com_FreeWeaponInfoMemory(int source);
int __cdecl Com_AddToString(const char* add, char* msg, int len, int maxlen, int mayAddQuotes);
char __cdecl Com_GetDecimalDelimiter();
void __cdecl Com_LocalizedFloatToString(float f, char* buffer, unsigned int maxlen, unsigned int numDecimalPlaces);
void __cdecl Com_SyncThreads();
void __cdecl Com_InitDObj();
void __cdecl Com_ShutdownDObj();
void __cdecl Com_InitialHull(
	const float (*points)[2],
	unsigned int* pointOrder,
	unsigned int pointCount,
	unsigned int* hullOrder);
void Com_InitHunkMemory();
void __cdecl Com_InitSoundDevGuiGraphs();
void Com_InitSoundDevGuiGraphs_LoadObj();
void Com_InitSoundDevGuiGraphs_FastFile();
void __cdecl Com_InitSoundAliasHash(unsigned int aliasCount);
void Com_InitSpeakerMaps();
void Com_InitDefaultSpeakerMap();
void Com_InitCurves();
void __cdecl Com_InitSoundAlias();
void Com_InitEntChannels(char* file);
void __cdecl Com_InitDefaultSoundAliasVolumeFalloffCurve(SndCurve* sndCurve);
void __cdecl Com_InitDefaultSoundAliasSpeakerMap(SpeakerMapInfo* info);
void __cdecl Com_InitParse();
void __cdecl Com_InitParseInfo(parseInfo_t* pi);
void __cdecl Com_InitThreadData(int threadContext);
void __cdecl Com_FreeEvent(char* ptr);


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


void __cdecl Com_PacketEventLoop(netsrc_t client, msg_t *netmsg);
void __cdecl Com_DispatchClientPacketEvent(netadr_t adr, msg_t *netmsg);
/*
==============================================================

MISC

==============================================================
*/

#define RoundUp(N, M) ((N) + ((unsigned int)(M)) - (((unsigned int)(N)) % ((unsigned int)(M))))
#define RoundDown(N, M) ((N) - (((unsigned int)(N)) % ((unsigned int)(M))))
