#pragma once

#ifndef KISAK_SP
#error This file is Single-Player only
#endif
#include <universal/q_parse.h>
#include "g_scr_main.h"

enum loading_t : __int32
{
	LOADING_DONE = 0x0,
	LOADING_LEVEL = 0x1,
	LOADING_SAVEGAME = 0x2,
};

struct trigger_info_t
{
	unsigned __int16 entnum;
	unsigned __int16 otherEntnum;
	int useCount;
	int otherUseCount;
};

struct SpawnVar
{
	bool spawnVarsValid;
	int numSpawnVars;
	char *spawnVars[64][2];
	int numSpawnVarChars;
	char spawnVarChars[2048];
};

struct level_locals_t
{
	gclient_s *clients;
	gentity_s *gentities;
	int gentitySize;
	int num_entities;
	gentity_s *firstFreeEnt;
	gentity_s *lastFreeEnt;
	sentient_s *sentients;
	actor_s *actors;
	scr_vehicle_s *vehicles;
	TurretInfo *turrets;
	int initializing;
	int clientIsSpawning;
	int maxclients;
	int framenum;
	int time;
	int previousTime;
	int snapTime;
	int newSession;
	int actorCorpseCount;
	SpawnVar spawnVar;
	int reloadDelayTime;
	int absoluteReloadDelayTime;
	EntHandle droppedWeaponCue[32];
	int changelevel;
	int bMissionSuccess;
	int bMissionFailed;
	int exitTime;
	int savepersist;
	char cinematic[64];
	float fFogOpaqueDist;
	float fFogOpaqueDistSqrd;
	unsigned int grenadeHintCount;
	int remapCount;
	int iSearchFrame;
	loading_t loading;
	int actorPredictDepth;
	int bDrawCompassFriendlies;
	int bPlayerIgnoreRadiusDamage;
	int bPlayerIgnoreRadiusDamageLatched;
	unsigned __int8 triggerIndex;
	int currentEntityThink;
	int currentIndex;
	bool checkAnimChange;
	int bRegisterItems;
	int framePos;
	bool demoplaying;
	int mpviewer;
	cached_tag_mat_t cachedTagMat;
	cached_tag_mat_t cachedEntTargetTagMat;
	unsigned __int16 soundAliasFirst;
	unsigned __int16 soundAliasLast;
	trigger_info_t pendingTriggerList[256];
	trigger_info_t currentTriggerList[256];
	int pendingTriggerListSize;
	int currentTriggerListSize;
	unsigned __int8 entTriggerIndex[2176];
	unsigned __int8 specialIndex[2176];
	actor_prone_info_s cgData_actorProneInfo[48];
	unsigned __int8 cgData_actorOnCompass[32];
	unsigned __int8 cgData_actorTeam[32];
	unsigned __int16 modelMap[512];
	float priorityNodeBias;
	//void *openScriptIOFileHandles[1];
	int openScriptIOFileHandles[1];
	char *openScriptIOFileBuffers[1];
	com_parse_mark_t currentScriptIOLineMark[1];
	int scriptPrintChannel;
	float compassMapUpperLeft[2];
	float compassMapWorldSize[2];
	float compassNorth[2];
	float mapSunColor[3];
	float mapSunDirection[3];
};