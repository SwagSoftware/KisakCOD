#pragma once
#include <qcommon/ent.h>
#include <qcommon/net_chan_mp.h>
#include <qcommon/sv_msg_write_mp.h>
#include <client_mp/client_mp.h>

enum svscmd_type : __int32
{                                       // ...
    SV_CMD_CAN_IGNORE = 0x0,
    SV_CMD_RELIABLE = 0x1,
};

struct svEntity_s // sizeof=0x178
{                                       // ...
    unsigned __int16 worldSector;
    unsigned __int16 nextEntityInWorldSector; // ...
    archivedEntity_s baseline;          // ...
    int numClusters;
    int clusternums[16];                // ...
    int lastCluster;
    int linkcontents;
    float linkmin[2];
    float linkmax[2];
};

struct svscmd_info_t // sizeof=0x408
{                                       // ...
    char cmd[1024];                     // ...
    int time;
    int type;
};
struct clientSnapshot_t // sizeof=0x2F84
{                                       // ...
    playerState_s ps;                   // ...
    int num_entities;
    int num_clients;                    // ...
    int first_entity;
    int first_client;
    int messageSent;
    int messageAcked;
    int messageSize;
    int serverTime;
};

struct __declspec(align(2)) client_t // sizeof=0xA5638
{                                       // ...
    clientHeader_t header;              // ...
    const char *dropReason;
    char userinfo[1024];                // ...
    svscmd_info_t reliableCommandInfo[128]; // ...
    int reliableSequence;
    int reliableAcknowledge;
    int reliableSent;
    int messageAcknowledge;
    int gamestateMessageNum;
    int challenge;
    usercmd_s lastUsercmd;              // ...
    int lastClientCommand;
    char lastClientCommandString[1024];
    gentity_s *gentity;                 // ...
    char name[16];                      // ...
    int downloading;
    char downloadName[64];
    int download;
    int downloadSize;
    int downloadCount;
    int downloadClientBlock;
    int downloadCurrentBlock;
    int downloadXmitBlock;
    unsigned __int8 *downloadBlocks[8];
    int downloadBlockSize[8];
    int downloadEOF;
    int downloadSendTime;
    char downloadURL[256];
    int wwwOk;
    int downloadingWWW;
    int clientDownloadingWWW;
    int wwwFallback;
    int nextReliableTime;
    int lastPacketTime;
    int lastConnectTime;
    int nextSnapshotTime;
    int timeoutCount;
    clientSnapshot_t frames[32];        // ...
    int ping;                           // ...
    int rate;
    int snapshotMsec;
    int snapshotBackoffCount;
    int pureAuthentic;
    char netchanOutgoingBuffer[131072]; // ...
    char netchanIncomingBuffer[2048];
    char cdkeyHash[33];                 // ...
    // padding byte
    unsigned __int16 scriptId;          // ...
    int bIsTestClient;
    int serverId;
    VoicePacket_t voicePackets[40];     // ...
    int voicePacketCount;
    bool muteList[64];                  // ...
    bool sendVoice;                     // ...
    unsigned __int8 stats[8192];        // ...
    unsigned __int8 statPacketsReceived; // ...
    bool tempPacketDebugging;
    // padding byte
};

//sv_init_mp
void __cdecl SV_SetConfigstring(int index, char *val);
void __cdecl SV_GetConfigstring(unsigned int index, char *buffer, int bufferSize);
unsigned int __cdecl SV_GetConfigstringConst(unsigned int index);
void __cdecl SV_SetConfigValueForKey(int start, int max, char *key, char *value);
void __cdecl SV_SetUserinfo(int index, char *val);
void __cdecl SV_GetUserinfo(int index, char *buffer, int bufferSize);
void __cdecl SV_CreateBaseline();
void __cdecl SV_BoundMaxClients(int minimum);
void __cdecl SV_Startup();
void __cdecl SV_ClearServer();
void __cdecl SV_InitArchivedSnapshot();
void __cdecl SV_InitDvar();
void __cdecl SV_SpawnServer(char *server);
void SV_SaveSystemInfo();
bool __cdecl SV_Loaded();
void __cdecl SV_Init();
void __cdecl SV_DropAllClients();
void __cdecl SV_Shutdown(const char *finalmsg);
void __cdecl SV_FinalMessage(const char *message);
void __cdecl SV_CheckThread();



//sv_main_mp
struct cachedSnapshot_t // sizeof=0x1C
{                                       // ...
    int archivedFrame;
    int time;
    int num_entities;
    int first_entity;
    int num_clients;
    int first_client;
    int usesDelta;
};
struct archivedSnapshot_s // sizeof=0x8
{                                       // ...
    int start;
    int size;
};
struct cachedClient_s // sizeof=0x2FCC
{                                       // ...
    int playerStateExists;
    clientState_s cs;
    playerState_s ps;                   // ...
};
struct __declspec(align(4)) challenge_t // sizeof=0x50
{                                       // ...
    netadr_t adr;
    int challenge;                      // ...
    int time;
    int pingTime;                       // ...
    int firstTime;
    int firstPing;                      // ...
    int connected;                      // ...
    char cdkeyHash[33];                 // ...
    // padding byte
    // padding byte
    // padding byte
};
struct tempBanSlot_t // sizeof=0x24
{                                       // ...
    char cdkeyHash[32];                 // ...
    int banTime;                        // ...
};
struct __declspec(align(128)) serverStatic_t // sizeof=0xB227480
{                                       // ...
    cachedSnapshot_t cachedSnapshotFrames[512];
    archivedEntity_s cachedSnapshotEntities[16384]; // ...
    int initialized;                    // ...
    int time;                           // ...
    int snapFlagServerBit;              // ...
    client_t clients[64];               // ...
    int numSnapshotEntities;            // ...
    int numSnapshotClients;             // ...
    int nextSnapshotEntities;           // ...
    int nextSnapshotClients;            // ...
    entityState_s snapshotEntities[172032]; // ...
    clientState_s snapshotClients[131072]; // ...
    int nextArchivedSnapshotFrames;     // ...
    archivedSnapshot_s archivedSnapshotFrames[1200]; // ...
    unsigned __int8 archivedSnapshotBuffer[33554432]; // ...
    int nextArchivedSnapshotBuffer;     // ...
    int nextCachedSnapshotEntities;     // ...
    int nextCachedSnapshotClients;      // ...
    int nextCachedSnapshotFrames;       // ...
    cachedClient_s cachedSnapshotClients[4096]; // ...
    int nextHeartbeatTime;              // ...
    int nextStatusResponseTime;
    challenge_t challenges[1024];       // ...
    netadr_t redirectAddress;           // ...
    netadr_t authorizeAddress;          // ...
    int sv_lastTimeMasterServerCommunicated; // ...
    netProfileInfo_t OOBProf;           // ...
    tempBanSlot_t tempBans[16];         // ...
    float mapCenter[3];                 // ...
};
enum serverState_t : __int32
{                                       // ...
    SS_DEAD = 0x0,
    SS_LOADING = 0x1,
    SS_GAME = 0x2,
};
struct ServerProfileTimes // sizeof=0x8
{                                       // ...
    float frameTime;                    // ...
    float wallClockTime;                // ...
};
struct server_t // sizeof=0x5FC60
{                                       // ...
    serverState_t state;                // ...
    int timeResidual;                   // ...
    bool inFrame;                       // ...
    // padding byte
    // padding byte
    // padding byte
    int restarting;                     // ...
    int start_frameTime;                // ...
    int checksumFeed;                   // ...
    cmodel_t *models[512];
    unsigned __int16 emptyConfigString; // ...
    unsigned __int16 configstrings[2442]; // ...
    // padding byte
    // padding byte
    svEntity_s svEntities[1024];        // ...
    gentity_s *gentities;               // ...
    int gentitySize;                    // ...
    int num_entities;                   // ...
    playerState_s *gameClients;         // ...
    int gameClientSize;                 // ...
    int skelTimeStamp;                  // ...
    int skelMemPos;                     // ...
    int bpsWindow[20];                  // ...
    int bpsWindowSteps;                 // ...
    int bpsTotalBytes;                  // ...
    int bpsMaxBytes;                    // ...
    int ubpsWindow[20];                 // ...
    int ubpsTotalBytes;                 // ...
    int ubpsMaxBytes;                   // ...
    float ucompAve;                     // ...
    int ucompNum;                       // ...
    volatile ServerProfileTimes profile; // ...
    volatile float serverFrameTimeMin;  // ...
    volatile float serverFrameTimeMax;  // ...
    char gametype[64];                  // ...
    bool killServer;                    // ...
    // padding byte
    // padding byte
    // padding byte
    const char *killReason;             // ...
};
void __cdecl TRACK_sv_main();
char *__cdecl SV_ExpandNewlines(char *in);
void __cdecl SV_AddServerCommand(client_t *client, svscmd_type type, char *cmd);
int __cdecl SV_CanReplaceServerCommand(client_t *client, const char *cmd);
bool __cdecl SV_IsFirstTokenEqual(const char *str1, const char *str2);
void __cdecl SV_CullIgnorableServerCommands(client_t *client);
void SV_SendServerCommand(client_t *cl, svscmd_type type, const char *fmt, ...);
client_t *__cdecl SV_FindClientByAddress(netadr_t from, int qport);
void __cdecl SVC_Status(netadr_t from);
void __cdecl SVC_GameCompleteStatus(netadr_t from);
void __cdecl SVC_Info(netadr_t from);
void __cdecl SV_ConnectionlessPacket(netadr_t from, msg_t *msg);
void __cdecl SV_PacketEvent(netadr_t from, msg_t *msg);
void __cdecl SV_CalcPings();
void __cdecl SV_FreeClientScriptId(client_t *cl);
void __cdecl SV_CheckTimeouts();
int __cdecl SV_CheckPaused();
void __cdecl SV_RunFrame();
void __cdecl SV_UpdatePerformanceFrame(float time);
void __cdecl SV_BotUserMove(client_t *cl);
void __cdecl SV_UpdateBots();
void __cdecl SV_WaitServer();
void __cdecl SV_InitSnapshot();
void __cdecl SV_KillLocalServer();
void __cdecl SV_PreFrame();
int __cdecl SV_Frame(int msec);
void __cdecl SV_FrameInternal(int msec);
int SV_PostFrame();
char __cdecl SV_CheckOverflow();

extern server_t sv;

extern const dvar_t *sv_allowedClan2;
extern const dvar_t *sv_maxPing;
extern const dvar_t *sv_debugPacketContentsForClientThisFrame;
extern const dvar_t *sv_privateClients;
extern const dvar_t *sv_maxclients;
extern const dvar_t *sv_hostname;
extern const dvar_t *sv_allowedClan1;
extern const dvar_t *sv_smp;
extern const dvar_t *sv_debugReliableCmds;
extern const dvar_t *sv_clientSideBullets;
extern const dvar_t *sv_privateClientsForClients;
extern const dvar_t *sv_reconnectlimit;
extern const dvar_t *sv_kickBanTime;
extern const dvar_t *sv_floodProtect;
extern const dvar_t *sv_gametype;
extern const dvar_t *sv_mapname;
extern const dvar_t *sv_cheats;
extern const dvar_t *sv_maxRate;
extern const dvar_t *sv_showCommands;
extern const dvar_t *sv_packet_info;
extern const dvar_t *sv_mapRotationCurrent;
extern const dvar_t *sv_connectTimeout;
extern const dvar_t *sv_disableClientConsole;
extern const dvar_t *sv_network_fps;
extern const dvar_t *sv_minPing;
extern const dvar_t *sv_mapcrc;
extern const dvar_t *sv_debugPacketContents;
extern const dvar_t *sv_zombietime;
extern const dvar_t *sv_debugRate;
extern const dvar_t *sv_showAverageBPS;
extern const dvar_t *sv_timeout;
extern const dvar_t *sv_padPackets;
extern const dvar_t *sv_debugPlayerstate;
extern const dvar_t *sv_maxHappyPingTime;
extern const dvar_t *sv_endGameIfISuck;
extern const dvar_t *sv_debugMessageKey;
extern const dvar_t *sv_fps;
extern const dvar_t *sv_botsPressAttackBtn;
extern const dvar_t *sv_serverid;
extern const dvar_t *sv_mapRotation;

extern serverStatic_t svs;
extern int com_inServerFrame;


// sv_net_chan_mp
bool __cdecl SV_Netchan_TransmitNextFragment(client_t *client, netchan_t *chan);
void __cdecl SV_Netchan_OutgoingSequenceIncremented(client_t *client, netchan_t *chan);
bool __cdecl SV_Netchan_Transmit(client_t *client, unsigned __int8 *data, int length);
void __cdecl SV_Netchan_AddOOBProfilePacket(int iLength);
void __cdecl SV_Netchan_UpdateProfileStats();
void __cdecl SV_Netchan_PrintProfileStats(int bPrintToConsole);



// sv_archive_mp
void __cdecl SV_ArchiveSnapshot(msg_t *msg);
gentity_s *__cdecl SV_GentityNumLocal(int num);
const clientState_s *__cdecl G_GetClientStateLocal(int clientNum);
int __cdecl GetFollowPlayerStateLocal(int clientNum, playerState_s *ps);


// sv_snapshot_mp
struct snapshotEntityNumbers_t // sizeof=0x1004
{                                       // ...
    int numSnapshotEntities;            // ...
    int snapshotEntities[1024];         // ...
};
void __cdecl SV_WriteSnapshotToClient(client_t *client, msg_t *msg);
void __cdecl SV_EmitPacketEntities(
    SnapshotInfo_s *snapInfo,
    int from_num_entities,
    int from_first_entity,
    int to_num_entities,
    int to_first_entity,
    msg_t *msg);
void __cdecl SV_EmitPacketClients(
    SnapshotInfo_s *snapInfo,
    int from_num_clients,
    int from_first_client,
    int to_num_clients,
    int to_first_client,
    msg_t *msg);
void __cdecl SV_UpdateServerCommandsToClient(client_t *client, msg_t *msg);
void __cdecl SV_UpdateServerCommandsToClient_PreventOverflow(client_t *client, msg_t *msg, int iMsgSize);
char __cdecl SV_GetClientPositionAtTime(int client, int gametime, float *pos);
int __cdecl SV_GetArchivedClientInfo(int clientNum, int *pArchiveTime, playerState_s *ps, clientState_s *cs);
cachedSnapshot_t *__cdecl SV_GetCachedSnapshot(int *pArchiveTime);
cachedSnapshot_t *__cdecl SV_GetCachedSnapshotInternal(int archivedFrame);
int __cdecl SV_GetCurrentClientInfo(int clientNum, playerState_s *ps, clientState_s *cs);
void __cdecl SV_BuildClientSnapshot(client_t *client);
void __cdecl SV_AddEntitiesVisibleFromPoint(float *org, int clientNum, snapshotEntityNumbers_t *eNums);
void __cdecl SV_AddCachedEntitiesVisibleFromPoint(
    int from_num_entities,
    int from_first_entity,
    float *org,
    int clientNum,
    snapshotEntityNumbers_t *eNums);
void __cdecl SV_AddArchivedEntToSnapshot(int e, snapshotEntityNumbers_t *eNums);
void __cdecl SV_SendMessageToClient(msg_t *msg, client_t *client);
int __cdecl SV_RateMsec(client_t *client, int messageSize);
void __cdecl SV_BeginClientSnapshot(client_t *client, msg_t *msg);
void __cdecl SV_EndClientSnapshot(client_t *client, msg_t *msg);
void __cdecl SV_PrintServerCommandsForClient(client_t *client);
void __cdecl SV_SetServerStaticHeader();
void __cdecl SV_GetServerStaticHeader();
void __cdecl SV_SendClientMessages();


// sv_client_mp
void __cdecl SV_GetChallenge(netadr_t from);
int __cdecl SV_IsBannedGuid(const char *cdkeyHash);
void __cdecl SV_ReceiveStats(netadr_t from, msg_t *msg);
void __cdecl SV_SetClientStat(int clientNum, int index, unsigned int value);
int __cdecl SV_GetClientStat(int clientNum, int index);
void __cdecl SV_BanGuidBriefly(const char *cdkeyHash);
unsigned int __cdecl SV_FindFreeTempBanSlot();
void __cdecl SV_BanClient(client_t *cl);
void __cdecl SV_UnbanClient(char *name);
void __cdecl SV_FreeClient(client_t *cl);
void __cdecl SV_FreeClients();
void __cdecl SV_DirectConnect(netadr_t from);
void __cdecl SV_FreeClientScriptPers();
void __cdecl SV_SendDisconnect(
    client_t *client,
    int state,
    const char *reason,
    bool translationForReason,
    const char *clientName);
void __cdecl SV_DropClient(client_t *drop, const char *reason, bool tellThem);
void __cdecl SV_DelayDropClient(client_t *drop, const char *reason);
void __cdecl SV_SendClientGameState(client_t *client);
void __cdecl SV_ClientEnterWorld(client_t *client, usercmd_s *cmd);
void __cdecl SV_Disconnect_f(client_t *cl);
void __cdecl SV_UserinfoChanged(client_t *cl);
void __cdecl SV_UpdateUserinfo_f(client_t *cl);
void __cdecl SV_ClientThink(client_t *cl, usercmd_s *cmd);
void __cdecl SV_UserMove(client_t *cl, msg_t *msg, int delta);
void __cdecl SV_ExecuteClientMessage(client_t *cl, msg_t *msg);
int __cdecl SV_ClientCommand(client_t *cl, msg_t *msg, int fromOldServer);
void __cdecl SV_ExecuteClientCommand(client_t *cl, char *s, int clientOK, int fromOldServer);
gentity_s *__cdecl SV_AddTestClient();


// sv_ccmds_mp
char *__cdecl SV_GetMapBaseName(char *mapname);
void __cdecl SV_Heartbeat_f();
void __cdecl SV_GameCompleteStatus_f();
void __cdecl SV_AddOperatorCommands();
void __cdecl SV_Map_f();
void __cdecl ShowLoadErrorsSummary(const char *mapName, unsigned int count);
void __cdecl SV_MapRestart_f();
void __cdecl SV_MapRestart(int fast_restart);
void __cdecl SV_FastRestart_f();
void __cdecl SV_MapRotate_f();
void __cdecl SV_TempBan_f();
int __cdecl SV_KickUser_f(char *playerName, int maxPlayerNameLen, char *cdkeyHash);
client_t *__cdecl SV_GetPlayerByName();
int __cdecl SV_KickClient(client_t *cl, char *playerName, int maxPlayerNameLen, char *cdkeyHash);
void __cdecl SV_Ban_f();
void __cdecl SV_BanNum_f();
client_t *__cdecl SV_GetPlayerByNum();
void __cdecl SV_Unban_f();
void __cdecl SV_Drop_f();
void __cdecl SV_DropNum_f();
int __cdecl SV_KickClient_f(char *playerName, int maxPlayerNameLen, char *cdkeyHash);
void __cdecl SV_TempBanNum_f();
void __cdecl SV_Status_f();
void __cdecl SV_Serverinfo_f();
void __cdecl SV_Systeminfo_f();
void __cdecl SV_DumpUser_f();
void __cdecl SV_KillServer_f();
void __cdecl SV_ScriptUsage_f();
void __cdecl SV_ScriptVarUsage_f();
void __cdecl SV_ScriptProfile_f();
void __cdecl SV_ScriptBuiltin_f();
void __cdecl SV_StringUsage_f();
void __cdecl SV_SetPerk_f();
void __cdecl SV_AddDedicatedCommands();
void __cdecl SV_ConSay_f();
void __cdecl SV_AssembleConSayMessage(int firstArg, char *text, int sizeofText);
void __cdecl SV_ConTell_f();
void __cdecl SV_RemoveDedicatedCommands();



// sv_main_pc_mp
void __cdecl SV_MasterGameCompleteStatus();
void __cdecl SV_MasterHeartbeat(const char *hbname);
void __cdecl SV_MasterShutdown();