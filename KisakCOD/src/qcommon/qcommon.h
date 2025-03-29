#pragma once

#include "../universal/q_shared.h"
#include <gfx_d3d/r_scene.h>
#include <server_mp/server.h>

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

extern int marker_common;

extern int com_skelTimeStamp;
extern unsigned int com_errorPrintsCount;

extern float com_timescaleValue;

extern int com_fixedConsolePosition;
extern int com_errorEntered;
extern int com_frameNumber;
extern int com_consoleLogOpenFailed;
extern int com_missingAssetOpenFailed;

extern const dvar_t *com_dedicated;
extern const dvar_t *com_hiDef;
extern const dvar_t *com_animCheck;
extern const dvar_t *com_developer_script;
extern const dvar_t *dev_timescale;
extern const dvar_t *cl_useMapPreloading;
extern const dvar_t *com_maxfps;
extern const dvar_t *com_frameTime;
extern const dvar_t *com_recommendedSet;
extern const dvar_t *sv_useMapPreloading;
extern const dvar_t *ui_errorMessage;
extern const dvar_t *com_introPlayed;
extern const dvar_t *com_wideScreen;
extern const dvar_t *ui_errorTitle;
extern const dvar_t *shortversion;
extern const dvar_t *com_attractmodeduration;
extern const dvar_t *com_attractmode;
extern const dvar_t *cl_paused_simple;
extern const dvar_t *sv_paused;
extern const dvar_t *com_fixedtime;
extern const dvar_t *fastfile_allowNoAuth;
extern const dvar_t *com_logfile;
extern const dvar_t *cl_paused;
extern const dvar_t *com_timescale;
extern const dvar_t *nextmap;
extern const dvar_t *version;
extern const dvar_t *com_sv_running;
extern const dvar_t *com_useConfig;
extern const dvar_t *com_maxFrameTime;
extern const dvar_t *com_statmon;
extern const dvar_t *com_filter_output;
extern const dvar_t *com_developer;

extern const dvar_t *sys_lockThreads;
extern const dvar_t *sys_smp_allowed;
extern const dvar_t *com_masterServerName;
extern const dvar_t *com_authServerName;
extern const dvar_t *com_masterPort;
extern const dvar_t *com_authPort;


enum errorParm_t : __int32
{                                       // ...
    ERR_FATAL = 0x0,
    ERR_DROP = 0x1,
    ERR_SERVERDISCONNECT = 0x2,
    ERR_DISCONNECT = 0x3,
    ERR_SCRIPT = 0x4,
    ERR_SCRIPT_DROP = 0x5,
    ERR_LOCALIZATION = 0x6,
    ERR_MAPLOADERRORSUMMARY = 0x7,
};

void QDECL Com_Printf(int channel, const char* fmt, ...);
void QDECL Com_Error(errorParm_t code, const char* fmt, ...);

// commandLine should not include the executable name (argv[0])
void Com_Init(char* commandLine);
void Com_Frame(void);

void __cdecl Com_ShutdownInternal(const char* finalmsg);
void Com_Shutdown(const char* finalmsg);

void __cdecl Com_ShutdownWorld();
void __cdecl Com_InitPlayerProfiles(int localClientNum);
void __cdecl Com_PrintMessage(int channel, const char* msg, int error);
void __cdecl Com_LogPrintMessage(int channel, char* msg);
void Com_OpenLogFile();
void Com_DPrintf(int channel, const char* fmt, ...);
void Com_PrintError(int channel, const char* fmt, ...);
void Com_PrintWarning(int channel, const char* fmt, ...);
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

struct SndCurve;
struct SpeakerMapInfo;
struct XModelPiece;

void __cdecl Com_InitDefaultSoundAliasVolumeFalloffCurve(SndCurve* sndCurve);
void __cdecl Com_InitDefaultSoundAliasSpeakerMap(SpeakerMapInfo* info);
void __cdecl Com_InitThreadData(int threadContext);
void __cdecl Com_FreeEvent(char* ptr);

// com_loadutils.cpp
char *__cdecl Com_LoadInfoString(char *fileName, const char *fileDesc, const char *ident, char *loadBuffer);
const char *__cdecl Com_LoadInfoString_FastFile(const char *fileName, const char *fileDesc, const char *ident);
char *__cdecl Com_LoadInfoString_LoadObj(char *fileName, const char *fileDesc, const char *ident, char *loadBuffer);
char *__cdecl Com_LoadRawTextFile(const char *fullpath);
XModelPiece *__cdecl Com_LoadRawTextFile_FastFile(const char *fullpath);
char *__cdecl Com_LoadRawTextFile_LoadObj(const char *fullpath);
void __cdecl Com_UnloadRawTextFile(char *filebuf);



/*
==============================================================

DVAR

==============================================================
*/

enum DvarSetSource : __int32
{                                       // ...
    DVAR_SOURCE_INTERNAL = 0x0,
    DVAR_SOURCE_EXTERNAL = 0x1,
    DVAR_SOURCE_SCRIPT = 0x2,
    DVAR_SOURCE_DEVGUI = 0x3,
};

int __cdecl Dvar_Command();
void __cdecl Dvar_GetCombinedString(char *combined, int first);
void __cdecl Dvar_Toggle_f();
bool __cdecl Dvar_ToggleInternal();
bool __cdecl Dvar_ToggleSimple(dvar_s *dvar);
void __cdecl Dvar_TogglePrint_f();
void __cdecl Dvar_Set_f();
void __cdecl Dvar_SetU_f();
void __cdecl Dvar_SetS_f();
void __cdecl Dvar_SetA_f();
void __cdecl Dvar_SetFromDvar_f();
void __cdecl Dvar_Reset_f();
void __cdecl Dvar_WriteVariables(int f);
void __cdecl Dvar_WriteSingleVariable(const dvar_s *dvar, int *userData);
void __cdecl Dvar_WriteDefaults(int f);
void __cdecl Dvar_WriteSingleDefault(const dvar_s *dvar, int *userData);
void __cdecl Dvar_List_f();
void __cdecl Dvar_ListSingle(const dvar_s *dvar, const char *userData);
void __cdecl Dvar_Dump_f();
void __cdecl PBdvar_set(const char *var_name, char *value);
char *__cdecl Dvar_InfoString(int localClientNum, char bit);
void __cdecl Dvar_InfoStringSingle(const dvar_s *dvar, unsigned int *userData);
char *__cdecl Dvar_InfoString_Big(int bit);
void __cdecl Dvar_InfoStringSingle_Big(const dvar_s *dvar, unsigned int *userData);
void __cdecl Dvar_AddCommands();
void __cdecl Dvar_RegisterBool_f();
void __cdecl Dvar_RegisterInt_f();
void __cdecl Dvar_RegisterFloat_f();
void __cdecl Dvar_SetFromLocalizedStr_f();
void __cdecl Dvar_SetToTime_f();
void __cdecl CL_SelectStringTableEntryInDvar_f();
void __cdecl Dvar_ForEach(void(__cdecl *callback)(const dvar_s *, void *), void *userData);
void Dvar_Sort();
void __cdecl Dvar_ForEachName(void(__cdecl *callback)(const char *));
const dvar_s *__cdecl Dvar_GetAtIndex(unsigned int index);
void __cdecl Dvar_SetInAutoExec(bool inAutoExec);
bool __cdecl Dvar_IsSystemActive();
char __cdecl Dvar_IsValidName(const char *dvarName);
const char *__cdecl Dvar_EnumToString(const dvar_s *dvar);
const char *__cdecl Dvar_IndexStringToEnumString(const dvar_s *dvar, const char *indexString);
const char *__cdecl Dvar_DisplayableValue(const dvar_s *dvar);
const char *__cdecl Dvar_ValueToString(const dvar_s *dvar, DvarValue value);
const char *__cdecl Dvar_DisplayableResetValue(const dvar_s *dvar);
const char *__cdecl Dvar_DisplayableLatchedValue(const dvar_s *dvar);
char __cdecl Dvar_ValueInDomain(unsigned __int8 type, DvarValue value, DvarLimits domain);
char __cdecl Dvar_VectorInDomain(const float *vector, int components, float min, float max);
const char *__cdecl Dvar_DomainToString(unsigned __int8 type, DvarLimits domain, __int64 outBuffer);
const char *__cdecl Dvar_DomainToString_Internal(
    unsigned __int8 type,
    DvarLimits domain,
    __int64 outBuffer,
    int *outLineCount);
void __cdecl Dvar_VectorDomainToString(int components, DvarLimits domain, __int64 outBuffer);
const char *__cdecl Dvar_DomainToString_GetLines(
    unsigned __int8 type,
    DvarLimits domain,
    __int64 outBuffer,
    int *outLineCount);
void __cdecl Dvar_PrintDomain(unsigned __int8 type, DvarLimits domain);
bool __cdecl Dvar_HasLatchedValue(const dvar_s *dvar);
int __cdecl Dvar_ValuesEqual(unsigned __int8 type, DvarValue val0, DvarValue val1);
const dvar_s *__cdecl Dvar_FindVar(const char *dvarName);
void __cdecl Dvar_ClearModified(dvar_s *dvar);
void __cdecl Dvar_SetModified(dvar_s *dvar);
void __cdecl Dvar_UpdateEnumDomain(dvar_s *dvar, const char **stringTable);
DvarValue *__cdecl Dvar_ClampValueToDomain(
    DvarValue *result,
    unsigned __int8 type,
    DvarValue value,
    DvarValue resetValue,
    DvarLimits domain);
void __cdecl Dvar_ClampVectorToDomain(float *vector, int components, float min, float max);
bool __cdecl Dvar_GetBool(const char *dvarName);
bool __cdecl Dvar_StringToBool(const char *string);
int __cdecl Dvar_GetInt(const char *dvarName);
int __cdecl Dvar_StringToInt(const char *string);
double __cdecl Dvar_GetFloat(const char *dvarName);
double __cdecl Dvar_StringToFloat(const char *string);
const char *__cdecl Dvar_GetString(const char *dvarName);
const char *__cdecl Dvar_GetVariantString(const char *dvarName);
void __cdecl Dvar_GetUnpackedColor(const dvar_s *dvar, float *expandedColor);
void __cdecl Dvar_StringToColor(const char *string, unsigned __int8 *color);
void __cdecl Dvar_GetUnpackedColorByName(const char *dvarName, float *expandedColor);
void __cdecl Dvar_Shutdown();
void __cdecl Dvar_FreeNameString(const char *name);
bool __cdecl Dvar_ShouldFreeCurrentString(dvar_s *dvar);
bool __cdecl Dvar_ShouldFreeLatchedString(dvar_s *dvar);
bool __cdecl Dvar_ShouldFreeResetString(dvar_s *dvar);
void __cdecl Dvar_FreeString(DvarValue *value);
void __cdecl Dvar_ChangeResetValue(dvar_s *dvar, DvarValue value);
void __cdecl Dvar_UpdateResetValue(dvar_s *dvar, DvarValue value);
void __cdecl Dvar_AssignResetStringValue(dvar_s *dvar, DvarValue *dest, const char *string);
void __cdecl Dvar_CopyString(const char *string, DvarValue *value);
void __cdecl Dvar_WeakCopyString(const char *string, DvarValue *value);
void __cdecl Dvar_MakeLatchedValueCurrent(dvar_s *dvar);
void __cdecl Dvar_SetVariant(dvar_s *dvar, DvarValue value, DvarSetSource source);
void __cdecl Dvar_AssignCurrentStringValue(dvar_s *dvar, DvarValue *dest, char *string);
void __cdecl Dvar_SetLatchedValue(dvar_s *dvar, DvarValue value);
void __cdecl Dvar_AssignLatchedStringValue(dvar_s *dvar, DvarValue *dest, char *string);
void __cdecl Dvar_ClearLatchedValue(dvar_s *dvar);
void __cdecl Dvar_ReinterpretDvar(
    dvar_s *dvar,
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue value,
    DvarLimits domain);
const dvar_s *__cdecl Dvar_RegisterNew(
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue value,
    DvarLimits domain,
    const char *description);
void __cdecl Dvar_Reregister(
    dvar_s *dvar,
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue resetValue,
    DvarLimits domain,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterBool(
    const char *dvarName,
    bool value,
    unsigned __int16 flags,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterVariant(
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue value,
    DvarLimits domain,
    const char *description);
void __cdecl Dvar_MakeExplicitType(
    dvar_s *dvar,
    const char *dvarName,
    unsigned __int8 type,
    unsigned __int16 flags,
    DvarValue resetValue,
    DvarLimits domain);
DvarValue *__cdecl Dvar_StringToValue(DvarValue *result, unsigned __int8 type, DvarLimits domain, const char *string);
void __cdecl Dvar_StringToVec2(const char *string, float *vector);
void __cdecl Dvar_StringToVec3(const char *string, float *vector);
void __cdecl Dvar_StringToVec4(const char *string, float *vector);
int __cdecl Dvar_StringToEnum(const DvarLimits *domain, const char *string);
void __cdecl Dvar_UpdateValue(dvar_s *dvar, DvarValue value);
char *__cdecl Dvar_AllocNameString(const char *name);
const dvar_s *__cdecl Dvar_RegisterInt(
    const char *dvarName,
    int value,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterFloat(
    const char *dvarName,
    float value,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterVec2(
    const char *dvarName,
    float x,
    float y,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterVec3(
    const char *dvarName,
    float x,
    float y,
    float z,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterVec4(
    const char *dvarName,
    float x,
    float y,
    float z,
    float w,
    DvarLimits min,
    unsigned __int16 flags,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterString(
    const char *dvarName,
    const char *value,
    unsigned __int16 flags,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterEnum(
    const char *dvarName,
    const char **valueList,
    int defaultIndex,
    unsigned __int16 flags,
    const char *description);
const dvar_s *__cdecl Dvar_RegisterColor(
    const char *dvarName,
    float r,
    float g,
    float b,
    float a,
    unsigned __int16 flags,
    const char *description);
void __cdecl Dvar_SetBoolFromSource(dvar_s *dvar, bool value, DvarSetSource source);
void __cdecl Dvar_SetIntFromSource(dvar_s *dvar, int value, DvarSetSource source);
void __cdecl Dvar_SetFloatFromSource(dvar_s *dvar, float value, DvarSetSource source);
void __cdecl Dvar_SetVec2FromSource(dvar_s *dvar, float x, float y, DvarSetSource source);
void __cdecl Dvar_SetVec3FromSource(dvar_s *dvar, float x, float y, float z, DvarSetSource source);
void __cdecl Dvar_SetVec4FromSource(dvar_s *dvar, float x, float y, float z, float w, DvarSetSource source);
void __cdecl Dvar_SetColorFromSource(dvar_s *dvar, float r, float g, float b, float a, DvarSetSource source);
void __cdecl Dvar_SetBool(dvar_s *dvar, bool value);
void __cdecl Dvar_SetInt(dvar_s *dvar, int value);
void __cdecl Dvar_SetFloat(dvar_s *dvar, float value);
void __cdecl Dvar_SetVec3(dvar_s *dvar, float x, float y, float z);
void __cdecl Dvar_SetString(dvar_s *dvar, char *value);
void __cdecl Dvar_SetStringFromSource(dvar_s *dvar, char *string, DvarSetSource source);
void __cdecl Dvar_SetColor(dvar_s *dvar, float r, float g, float b, float a);
void __cdecl Dvar_SetFromString(dvar_s *dvar, char *string);
void __cdecl Dvar_SetFromStringFromSource(dvar_s *dvar, char *string, DvarSetSource source);
void __cdecl Dvar_SetBoolByName(const char *dvarName, bool value);
void __cdecl Dvar_SetIntByName(const char *dvarName, int value);
void __cdecl Dvar_SetFloatByName(const char *dvarName, float value);
void __cdecl Dvar_SetVec3ByName(const char *dvarName, float x, float y, float z);
void __cdecl Dvar_SetStringByName(const char *dvarName, char *value);
const dvar_s *__cdecl Dvar_SetFromStringByNameFromSource(const char *dvarName, char *string, DvarSetSource source);
void __cdecl Dvar_SetFromStringByName(const char *dvarName, char *string);
void __cdecl Dvar_SetCommand(const char *dvarName, char *string);
void __cdecl Dvar_SetDomainFunc(dvar_s *dvar, bool(__cdecl *customFunc)(dvar_s *, DvarValue));
void __cdecl Dvar_AddFlags(const dvar_s *dvar, int flags);
void __cdecl Dvar_Reset(dvar_s *dvar, DvarSetSource setSource);
void __cdecl Dvar_SetCheatState();
void __cdecl Dvar_Init();
void __cdecl Dvar_ResetScriptInfo();
char __cdecl Dvar_AnyLatchedValues();
void __cdecl Dvar_ResetDvars(unsigned __int16 filter, DvarSetSource setSource);
int __cdecl Com_LoadDvarsFromBuffer(const char **dvarnames, unsigned int numDvars, char *buffer, char *filename);
int __cdecl Com_SaveDvarsToBuffer(const char **dvarnames, unsigned int numDvars, char *buffer, unsigned int bufsize);

// dvar_cmds
void __cdecl TRACK_dvar_cmds();
int __cdecl Dvar_Command();
void __cdecl Dvar_GetCombinedString(char *combined, int first);
void __cdecl Dvar_Toggle_f();
bool __cdecl Dvar_ToggleInternal();
bool __cdecl Dvar_ToggleSimple(dvar_s *dvar);
void __cdecl Dvar_TogglePrint_f();
void __cdecl Dvar_Set_f();
void __cdecl Dvar_SetU_f();
void __cdecl Dvar_SetS_f();
void __cdecl Dvar_SetA_f();
void __cdecl Dvar_SetFromDvar_f();
void __cdecl Dvar_Reset_f();
void __cdecl Dvar_WriteVariables(int f);
void __cdecl Dvar_WriteSingleVariable(const dvar_s *dvar, int *userData);
void __cdecl Dvar_WriteDefaults(int f);
void __cdecl Dvar_WriteSingleDefault(const dvar_s *dvar, int *userData);
void __cdecl Dvar_List_f();
void __cdecl Dvar_ListSingle(const dvar_s *dvar, const char *userData);
void __cdecl Com_DvarDump(int channel, const char *match);
void __cdecl Com_DvarDumpSingle(const dvar_s *dvar, void *userData);
void __cdecl Dvar_Dump_f();
void __cdecl SV_SetConfig(int start, int max, int bit);
void __cdecl SV_SetConfigDvar(const dvar_s *dvar, int *userData);
char *__cdecl Dvar_InfoString(int localClientNum, char bit);
void __cdecl Dvar_InfoStringSingle(const dvar_s *dvar, unsigned int *userData);
char *__cdecl Dvar_InfoString_Big(int bit);
void __cdecl Dvar_InfoStringSingle_Big(const dvar_s *dvar, unsigned int *userData);
void __cdecl Dvar_AddCommands();
void __cdecl Dvar_RegisterBool_f();
void __cdecl Dvar_RegisterInt_f();
void __cdecl Dvar_RegisterFloat_f();
void __cdecl Dvar_SetFromLocalizedStr_f();
void __cdecl Dvar_SetToTime_f();

extern char info1[1024];
extern char info2[8192];

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

struct netadr_t  {
	netadrtype_t	type;

	byte	ip[4];
	byte	ipx[10];

	unsigned short	port;
};

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

void _copyDWord(unsigned int *dest, const unsigned int constant, const unsigned int count);

/*
==============================================================

DOBJ MANAGEMENT

==============================================================
*/

struct __declspec(align(2)) DObjModel_s // sizeof=0x8
{                                       // ...
	XModel *model;                      // ...
	unsigned __int16 boneName;          // ...
	bool ignoreCollision;               // ...
	// padding byte
};

void __cdecl TRACK_dobj_management();
DObj_s *__cdecl Com_GetClientDObj(unsigned int handle, int localClientNum);
DObj_s *__cdecl Com_GetServerDObj(unsigned int handle);
DObj_s *__cdecl Com_ClientDObjCreate(
	DObjModel_s *dobjModels,
	unsigned __int16 numModels,
	XAnimTree_s *tree,
	unsigned int handle,
	int localClientNum);
int __cdecl Com_GetFreeDObjIndex();
void __cdecl Com_ClientDObjClearAllSkel();
DObj_s *__cdecl Com_ServerDObjCreate(
	DObjModel_s *dobjModels,
	unsigned __int16 numModels,
	XAnimTree_s *tree,
	unsigned int handle);
void __cdecl Com_SafeClientDObjFree(unsigned int handle, int localClientNum);
void __cdecl Com_SafeServerDObjFree(unsigned int handle);
void __cdecl Com_InitDObj();
void __cdecl Com_ShutdownDObj();
void __cdecl DB_SaveDObjs();
void __cdecl DB_LoadDObjs();


/*
==============================================================

TRACES

==============================================================
*/
// cm_trace
struct TraceExtents // sizeof=0x24
{                                       // ...
    float start[3];                     // ...
    float end[3];                       // ...
    float invDelta[3];
};
struct locTraceWork_t // sizeof=0x28
{                                       // ...
    int contents;                       // ...
    TraceExtents extents;               // ...
};
struct traceWork_t // sizeof=0xB0
{                                       // ...
    TraceExtents extents;               // ...
    float delta[3];                     // ...
    float deltaLen;                     // ...
    float deltaLenSq;                   // ...
    float midpoint[3];                  // ...
    float halfDelta[3];                 // ...
    float halfDeltaAbs[3];              // ...
    float size[3];                      // ...
    float bounds[2][3];                 // ...
    int contents;                       // ...
    bool isPoint;                       // ...
    bool axialCullOnly;
    // padding byte
    // padding byte
    float radius;                       // ...
    float offsetZ;                      // ...
    float radiusOffset[3];              // ...
    float boundingRadius;               // ...
    TraceThreadInfo threadInfo;         // ...
};
struct IgnoreEntParams // sizeof=0xC
{                                       // ...
    int baseEntity;                     // ...
    int parentEntity;                   // ...
    bool ignoreSelf;                    // ...
    bool ignoreParent;                  // ...
    bool ignoreSiblings;                // ...
    bool ignoreChildren;                // ...
};
struct pointtrace_t // sizeof=0x34
{                                       // ...
    TraceExtents extents;               // ...
    const IgnoreEntParams *ignoreEntParams; // ...
    int contentmask;                    // ...
    int bLocational;                    // ...
    unsigned __int8 *priorityMap;       // ...
};
struct moveclip_t // sizeof=0x54
{
    float mins[3];
    float maxs[3];
    float outerSize[3];
    TraceExtents extents;
    int passEntityNum;
    int passOwnerNum;
    int contentmask;
};
unsigned __int16 __cdecl Trace_GetEntityHitId(const trace_t *trace);
unsigned __int16 __cdecl Trace_GetDynEntHitId(const trace_t *trace, DynEntityDrawType *drawType);
unsigned int __cdecl CM_TempBoxModel(const float *mins, const float *maxs, int contents);
void __cdecl CM_GetBox(cbrush_t **box_brush, cmodel_t **box_model);
bool __cdecl CM_ClipHandleIsValid(unsigned int handle);
cmodel_t *__cdecl CM_ClipHandleToModel(unsigned int handle);
int __cdecl CM_ContentsOfModel(unsigned int handle);
void __cdecl CM_BoxTrace(
    trace_t *results,
    const float *start,
    float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask);
void __cdecl CM_Trace(
    trace_t *results,
    const float *start,
    float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask);
void __cdecl CM_GetTraceThreadInfo(TraceThreadInfo *threadInfo);
void __cdecl CM_TestInLeaf(traceWork_t *tw, cLeaf_t *leaf, trace_t *trace);
bool __cdecl CM_TestInLeafBrushNode(traceWork_t *tw, cLeaf_t *leaf, trace_t *trace);
void __cdecl CM_TestInLeafBrushNode_r(const traceWork_t *tw, cLeafBrushNode_s *node, trace_t *trace);
void __cdecl CM_TestBoxInBrush(const traceWork_t *tw, cbrush_t *brush, trace_t *trace);
void __cdecl CM_TestCapsuleInCapsule(const traceWork_t *tw, trace_t *trace);
void __cdecl CM_PositionTest(traceWork_t *tw, trace_t *trace);
void __cdecl CM_TraceThroughLeaf(const traceWork_t *tw, cLeaf_t *leaf, trace_t *trace);
bool __cdecl CM_TraceThroughLeafBrushNode(const traceWork_t *tw, cLeaf_t *leaf, trace_t *trace);
void __cdecl CM_TraceThroughLeafBrushNode_r(
    const traceWork_t *tw,
    cLeafBrushNode_s *node,
    const float *p1_,
    __int64 p2);
void __cdecl CM_TraceThroughBrush(const traceWork_t *tw, cbrush_t *brush, trace_t *trace);
void __cdecl CM_TraceCapsuleThroughCapsule(const traceWork_t *tw, trace_t *trace);
int __cdecl CM_TraceSphereThroughSphere(
    const traceWork_t *tw,
    const float *vStart,
    const float *vEnd,
    const float *vStationary,
    float radius,
    trace_t *trace);
int __cdecl CM_TraceCylinderThroughCylinder(
    const traceWork_t *tw,
    const float *vStationary,
    float fStationaryHalfHeight,
    float radius,
    trace_t *trace);
void __cdecl CM_TraceThroughTree(const traceWork_t *tw, int num, const float *p1_, const float *p2, trace_t *trace);
void __cdecl CM_SetAxialCullOnly(traceWork_t *tw);
void __cdecl CM_TransformedBoxTraceRotated(
    trace_t *results,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask,
    const float *origin,
    float (*matrix)[3]);
void __cdecl CM_TransformedBoxTrace(
    trace_t *results,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    __int64 model,
    const float *origin,
    const float *angles);
void __cdecl CM_TransformedBoxTraceExternal(
    trace_t *results,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    __int64 model,
    const float *origin,
    const float *angles);
int __cdecl CM_BoxSightTrace(
    int oldHitNum,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask);
int __cdecl CM_SightTraceThroughBrush(const traceWork_t *tw, cbrush_t *brush);
int __cdecl CM_SightTraceThroughLeaf(const traceWork_t *tw, cLeaf_t *leaf, trace_t *trace);
int __cdecl CM_SightTraceThroughLeafBrushNode(const traceWork_t *tw, cLeaf_t *leaf);
int __cdecl CM_SightTraceThroughLeafBrushNode_r(
    const traceWork_t *tw,
    cLeafBrushNode_s *node,
    const float *p1_,
    const float *p2);
int __cdecl CM_SightTraceCapsuleThroughCapsule(const traceWork_t *tw, trace_t *trace);
bool __cdecl CM_SightTraceSphereThroughSphere(
    const traceWork_t *tw,
    const float *vStart,
    const float *vEnd,
    const float *vStationary,
    float radius,
    trace_t *trace);
bool __cdecl CM_SightTraceCylinderThroughCylinder(
    const traceWork_t *tw,
    const float *vStationary,
    float fStationaryHalfHeight,
    float radius,
    trace_t *trace);
int __cdecl CM_SightTraceThroughTree(const traceWork_t *tw, int num, const float *p1_, const float *p2, trace_t *trace);
int __cdecl CM_TransformedBoxSightTrace(
    int hitNum,
    const float *start,
    const float *end,
    const float *mins,
    const float *maxs,
    unsigned int model,
    int brushmask,
    const float *origin,
    const float *angles);

// cm_load
void __cdecl TRACK_cm_load();
void __cdecl CM_LoadMap(const char *name, int *checksum);
void CM_InitAllThreadData();
void __cdecl CM_InitThreadData(unsigned int threadContext);
void __cdecl CM_LoadMapData(const char *name);
void __cdecl CM_LoadMapData_FastFile(const char *name);
void __cdecl CM_LoadMapFromBsp(const char *name, bool usePvs);
void __cdecl CM_Shutdown();
void __cdecl CM_Unload();
int __cdecl CM_LeafCluster(unsigned int leafnum);
void __cdecl CM_ModelBounds(unsigned int model, float *mins, float *maxs);

extern clipMap_t cm;

// cm_load_obj
void __cdecl CM_LoadMapData_LoadObj(const char *name);



/*
==============================================================

Profiler

==============================================================
*/
enum MapProfileTrackedValue : __int32
{                                       // ...
    MAP_PROFILE_FILE_OPEN = 0x0,
    MAP_PROFILE_FILE_SEEK = 0x1,
    MAP_PROFILE_FILE_READ = 0x2,
    MAP_PROFILE_VALUE_MAX = 0x3,
};
struct MapProfileElement // sizeof=0x18
{                                       // ...
    unsigned __int64 ticksStart;
    unsigned __int64 ticksTotal;
    unsigned __int64 ticksSelf;
};
struct MapProfileEntry // sizeof=0x70
{                                       // ...
    const char *label;
    int accessCount;
    unsigned __int64 ticksStart;
    unsigned __int64 ticksTotal;
    unsigned __int64 ticksSelf;
    int indent;
    MapProfileEntry *parent;
    MapProfileElement elements[3];
};
struct MapProfileHotSpot // sizeof=0x18
{                                       // ...
    const char *label;                  // ...
    int accessCount;                    // ...
    __int64 ticksSelf;                  // ...
    __int64 ticksFile;                  // ...
};

void __cdecl TRACK_com_profilemapload();
bool __cdecl ProfLoad_IsActive();
void __cdecl ProfLoad_BeginTrackedValue(MapProfileTrackedValue type);
void __cdecl ProfLoad_BeginTrackedValueTicks(MapProfileElement *value, unsigned __int64 ticks);
void __cdecl ProfLoad_EndTrackedValue(MapProfileTrackedValue type);
void __cdecl ProfLoad_EndTrackedValueTicks(MapProfileElement *value, unsigned __int64 ticks);
void __cdecl ProfLoad_Init();
void __cdecl ProfLoad_Activate();
void __cdecl ProfLoad_Deactivate();
void  ProfLoad_Print();
void ProfLoad_CalculateSelfTicks();
int ProfLoad_PrintTree();
void __cdecl ProfLoad_GetEntryRowText(const MapProfileEntry *entry, char *rowText, int sizeofRowText);
void ProfLoad_PrintHotSpots();
bool __cdecl ProfLoad_CompareHotSpotNames(const MapProfileHotSpot *hotSpot0, const MapProfileHotSpot *hotSpot1);
bool __cdecl ProfLoad_CompareHotSpotTicks(const MapProfileHotSpot *hotSpot0, const MapProfileHotSpot *hotSpot1);
void __cdecl ProfLoad_Begin(const char *label);
MapProfileEntry *__cdecl Com_GetEntryForNewLabel(const char *label);
void __cdecl ProfLoad_End();
void __cdecl ProfLoad_DrawOverlay(rectDef_s *rect);
int ProfLoad_DrawTree();

extern const dvar_t *com_profileLoading;

// statmonitor
struct statmonitor_s // sizeof=0x8
{                                       // ...
    int endtime;                        // ...
    Material *material;                 // ...
};
void __cdecl TRACK_statmonitor();
void __cdecl StatMon_Warning(int type, int duration, const char *materialName);
void __cdecl StatMon_GetStatsArray(const statmonitor_s **array, int *count);
void __cdecl StatMon_Reset();



// sv_msg_write_mp
struct nodetype // sizeof=0x14
{                                       // ...
    nodetype *left;
    nodetype *right;
    nodetype *parent;
    int weight;
    int symbol;
};
struct huff_t // sizeof=0x4C14
{                                       // ...
    int blocNode;
    int blocPtrs;
    nodetype *tree;                     // ...
    nodetype *loc[257];
    nodetype **freelist;
    nodetype nodeList[768];
    nodetype *nodePtrs[768];
};
struct huffman_t // sizeof=0x4C14
{                                       // ...
    huff_t compressDecompress;          // ...
};
struct __declspec(align(4)) NetField // sizeof=0x10
{                                       // ...
    const char *name;
    int offset;
    int bits;
    unsigned __int8 changeHints;
    // padding byte
    // padding byte
    // padding byte
};
struct NetFieldList // sizeof=0x8
{                                       // ...
    const NetField *array;
    unsigned int count;
};
void __cdecl TRACK_msg();
const NetFieldList *__cdecl MSG_GetStateFieldListForEntityType(int eType);
void __cdecl MSG_WriteReliableCommandToBuffer(const char *pszCommand, char *pszBuffer, int iBufferSize);
void __cdecl MSG_WriteOriginFloat(__int64 clientNum, int bits, float value, float oldValue);
void __cdecl MSG_WriteOriginZFloat(__int64 clientNum, float value, float oldValue);
bool __cdecl MSG_ValuesAreEqual(const SnapshotInfo_s *snapInfo, int bits, const int *fromF, const int *toF);
void __cdecl MSG_WriteLastChangedField(msg_t *msg, int lastChangedFieldNum, unsigned int numFields);
void __cdecl MSG_WriteEventNum(int clientNum, msg_t *msg, unsigned __int8 eventNum);
void __cdecl MSG_WriteEventParam(int clientNum, msg_t *msg, unsigned __int8 eventParam);
PacketEntityType __cdecl MSG_GetPacketEntityTypeForEType(int eType);
unsigned int __cdecl MSG_GetBitCount(int bits, bool *estimate, int from, int to);
void __cdecl MSG_WriteEntity(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    entityState_s *from,
    const entityState_s *to,
    int force);
void __cdecl MSG_WriteEntityRemoval(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    unsigned __int8 *from,
    int indexBits,
    bool changeBit);
void __cdecl MSG_WriteEntityDeltaForEType(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    int eType,
    const entityState_s *from,
    const entityState_s *to,
    int force);
int __cdecl MSG_WriteEntityDelta(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    const unsigned __int8 *from,
    const unsigned __int8 *to,
    int force,
    int numFields,
    int indexBits,
    const NetField *stateFields);
void __cdecl MSG_WriteDeltaField(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    const unsigned __int8 *from,
    const unsigned __int8 *to,
    const NetField *field,
    int fieldNum,
    bool forceSend);
void __cdecl MSG_WriteDeltaTime(int clientNum, msg_t *msg, int timeBase, int time);
void __cdecl MSG_Write24BitFlag(int clientNum, msg_t *msg, int oldFlags, int newFlags);
void __cdecl MSG_WriteGroundEntityNum(int clientNum, msg_t *msg, int groundEntityNum);
bool __cdecl MSG_CheckWritingEnoughBits(int value, unsigned int bits);
void __cdecl MSG_WriteDeltaArchivedEntity(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    archivedEntity_s *from,
    archivedEntity_s *to,
    int force);
int __cdecl MSG_WriteDeltaStruct(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    unsigned __int8 *from,
    unsigned __int8 *to,
    int force,
    int numFields,
    int indexBits,
    const NetField *stateFields,
    int bChangeBit);
void __cdecl MSG_WriteDeltaClient(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    clientState_s *from,
    clientState_s *to,
    int force);
void __cdecl MSG_WriteDeltaPlayerstate(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    const playerState_s *from,
    const playerState_s *to);
bool __cdecl MSG_ShouldSendPSField(
    const SnapshotInfo_s *snapInfo,
    bool sendOriginAndVel,
    const playerState_s *ps,
    const playerState_s *oldPs,
    const NetField *field);
void __cdecl MSG_WriteDeltaFields(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    unsigned __int8 *from,
    unsigned __int8 *to,
    int force,
    int numFields,
    const NetField *stateFields);
void __cdecl MSG_WriteDeltaHudElems(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    const hudelem_s *from,
    const hudelem_s *to,
    unsigned int count);
