#pragma once
#include "q_shared.h"

static const char *prof_enumNames[433] =
{
  "",
  "Probe1",
  "Probe2",
  "Probe3",
  "Probe4",
  "Probe5",
  "GProbe1",
  "GProbe2",
  "GProbe3",
  "GProbe4",
  "GProbe5",
  "CG_ProcessSnapshots",
  "CG_UpdateEntInfo",
  "CG_DrawActiveFrame",
  "R_RenderScene",
  "R_IssueRenderCommands",
  "CL_Frame",
  "CL_RunOncePerFrame",
  "SCR_UpdateFrame",
  "CL_BeginFrame",
  "CL_DrawFrame",
  "CL_EndFrame",
  "CG_DAF_UpdateEffects",
  "CG_DAF_AddLocalEntities",
  "CG_DAF_SmoothFrame",
  "CG_DrawActive",
  "CG_Pmove",
  "G_Pmove",
  "CbufExecute",
  "Com_ClientPacketEvent",
  "Com_ServerPacketEvent",
  "SV_BuildAndSendClientSnapshot",
  "NetworkRead",
  "NetworkWrite",
  "SCR_UpdateScreen",
  "CM_TerrainTrace",
  "CM_CurveTrace",
  "SV_Trace",
  "SV_TracePassed",
  "CM_Trace",
  "CM_SightTrace",
  "CM_AreaEntities",
  "CM_LinkEntity",
  "CM_PointSightTraceToEntities",
  "CM_ClipSightTraceToEntities",
  "CM_TraceBrush",
  "CM_TraceTerrain",
  "CM_SightBrush",
  "CM_SightTerrain",
  "CG_UnlinkEntity",
  "CG_LinkEntity",
  "CG_LocationalTrace",
  "CG_TraceCapsule",
  "CG_Trace",
  "CG_ClipMoveToEntities",
  "CG_ClipMoveToEntity",
  "CG_PointTraceToEntities",
  "CG_PointTraceToEntity",
  "AimTarget_ProcessEntity",
  "AimTarget_IsTargetValid",
  "AimTarget_IsTargetVisible",
  "AimTarget_CreateTarget",
  "AimAssist_UpdateGamePadInput",
  "AimAssist_UpdateMouseInput",
  "AimAssist_UpdateScreenTargets",
  "R_EmitDrawSurfList",
  "R_DrawSunShadowPartition",
  "R_BoundScene",
  "R_BoundAndPartitionScene",
  "R_AddEntitySurfaces",
  "R_DoesDrawSurfListInfoNeedFloatz",
  "R_AddWorldSurfacesDpvs",
  "R_AddShadowSurfacesDpvs",
  "GenerateSunShadow",
  "EmitSpotShadow",
  "EmitSunShadow",
  "R_SunShadowMaps",
  "R_DepthPrepass",
  "R_StreamUpdate",
  "R_StreamUpdateStatic",
  "R_StreamUpdateDynamicModels",
  "RB_StreamExecuteQueuedCommands",
  "R_StreamAlloc_Alloc",
  "R_StreamAlloc_RunOptimizationTask",
  "R_ConvexHull",
  "R_GetStaticLights",
  "R_PickFinalLights",
  "R_LightCacheTrace",
  "R_SortDrawSurfs",
  "R_SkinXModel",
  "RB_DrawTechnique",
  "RB_SetSampler",
  "RB_SetVertexShaderConstant",
  "RB_SetPixelShaderConstant",
  "RB_SetVertexDeclaration",
  "RB_SetVertexShader",
  "RB_SetPixelShader",
  "RB_SetParticleCloudConstants",
  "R_DrawSurfs",
  "rb_surfaceTable",
  "TessCodeMesh",
  "TessCloud",
  "TessXModSkin",
  "TessXModSkinCach",
  "TessXModSkinUncach",
  "TessXModRigid",
  "TessStaticModel",
  "TessStaticModelCached",
  "TessTriangles",
  "TessBModel",
  "TessSharedShadowCaster",
  "SkinXSurfaceWeight",
  "EndSurface",
  "EndSurface_Optimized",
  "EndSurface_Standard",
  "RB_ProcessPostEffects",
  "RB_ApplyPostEffects",
  "RB_UpdateAverageSceneBrightnessImage",
  "RB_GlowEffect",
  "RB_UploadWaterTexture",
  "RB_DrawSunQuerySprite",
  "R_SyncGpu",
  "FrequenciesAtTime",
  "AmplitudesFromFrequencies",
  "PixelsFromAmplitudes",
  "ProcessImage",
  "UploadImage",
  "GenerateMipMaps",
  "Present",
  "DrawIndexedPrimitive",
  "DrawIndexedPrimitiveUP",
  "SetVertexShaderConstant",
  "SetPixelShaderConstant",
  "WaitRenderer",
  "WaitFX",
  "WaitSpotLightFX",
  "WaitDepFX",
  "WaitRenderSwap",
  "WaitUpdateEvent",
  "WaitOtherThread",
  "RB_DrawProfileCmd",
  "RB_Draw3D",
  "R_SetupPass",
  "WaitWater",
  "R_Cinematic_UpdateFrame",
  "R_Cinematic_Advance",
  "BinkDoFrame",
  "BinkNextFrame",
  "BinkWait",
  "RB_LimitQueuedFrames",
  "SC_FindCasters",
  "SC_AddReceivers",
  "SC_DrawCaster",
  "SC_FindReceivers",
  "SC_DrawReceivers",
  "SC_SkinXModel",
  "RB_SetIndexData",
  "LockIndexBufferDiscard",
  "LockIndexBufferNoOverwrite",
  "LockSkinnedCache",
  "UnlockSkinnedCache",
  "RB_SetVertexData",
  "LockIndexBuffer",
  "LockVertexBuffer",
  "LockSModelBuffer",
  "BindTexture",
  "R_memcpy",
  "RB_memcpy",
  "RB_memcpy_it",
  "RB_memcpy_vt",
  "RB_memcpy_ib",
  "RB_memcpy_vb",
  "RendererSleep",
  "FrontEndSleep",
  "WorkerThread",
  "WaitForWorkerCmd",
  "WaitWorkerCmds",
  "ProcessWorkerCmds",
  "PixelShaderSurfs",
  "PixelShader",
  "StaticCull",
  "SceneEntCull",
  "DynModelCull",
  "DynBrushCull",
  "R_DrawStaticModelCmd",
  "R_DrawXModelCmd",
  "R_DrawTrianglesCmd",
  "SaveWriteThread",
  "R_VisitPortals",
  "ExecuteRenderCmds",
  "RB_memcpy_smc",
  "RB_memcpy_tris",
  "RB_memcpy_shadow",
  "FX_Update",
  "FX_UpdateOrigin",
  "FX_EndUpdate",
  "FX_GenerateVerts",
  "FX_GenElemVerts",
  "FX_EvalVisState",
  "FX_Sort",
  "FX_Draw",
  "FX_DrawElems",
  "FX_DrawSortedElems",
  "FX_Trace",
  "FX_CreateImpactMark",
  "FX_ImpactMark_World",
  "FX_ImpactMark_Models",
  "FX_ImpactMark_Generate_AddEntityModels",
  "FX_FreeLruMark",
  "FX_GenMarkVertsStaticModel",
  "FX_GenMarkVertsEnt",
  "FX_GenMarkVertsWorld",
  "FX_GetVisibility",
  "FX_StopEffect",
  "SV_FX_GetVisibility",
  "R_BoxStaticModels",
  "R_MarkFragments_SceneDObjs",
  "R_MarkUtil_GetDObjAnimMatAndHideParts",
  "R_MarkFragments_AnimatedXModel",
  "R_MarkFragments_AnimatedXModel_VertList",
  "AIThink",
  "AIPhysics",
  "AIPhysicsTrace",
  "AISightTrace",
  "AITurretSightTrace",
  "AIUpdateEyeInfo",
  "AIUpdateSight",
  "AIPathSearch",
  "AINearestNode",
  "AILookAhead",
  "G_RunFrame",
  "AITurretThink",
  "AIShoot",
  "SV_SightTrace",
  "AIGetMuzzleInfo",
  "Turret_Fire_Lead",
  "AIAnimscript",
  "AIAnimscriptNonBuiltIn",
  "G_WriteGame",
  "G_SaveFileAccess",
  "G_LoadGame",
  "G_LoadFileAccess",
  "G_FreeEntity",
  "G_FindConfigstringIndex",
  "G_ModelIndex",
  "G_EntAttach",
  "G_EntDetach",
  "G_DObjUpdate",
  "G_UpdateTagInfoOfChildren",
  "G_EntDetachAll",
  "G_EntLinkTo",
  "G_EntUnlink",
  "G_UpdateTagInfo",
  "FS_GetDataForFile",
  "SV_GetArchivedPlayerState",
  "SV_Frame",
  "SV_RunFrame",
  "SV_SendClientMessages",
  "SV_ArchiveSnapshot",
  "G_UpdateDvars",
  "G_RunFrameForEntity",
  "G_RunThink",
  "G_RunMover",
  "G_UpdateObjectiveToClients",
  "G_UpdateHudElemsToClients",
  "ClientEndFrame",
  "ClientEvents",
  "ClientImpacts",
  "IntermissionThink",
  "SpectatorThink",
  "G_UpdateClientInfo",
  "G_TriggerChecks",
  "G_TriggerCheckScripts",
  "IntermissionClientEndFrame",
  "SpectatorClientEndFrame",
  "ClientSpawn",
  "Player_UpdateCursorHints",
  "G_PlayerStateToEntityStateExtrapolate",
  "BG_PlayerStateToEntityState",
  "G_GetNonPVSPlayerInfo",
  "BG_PlayerAnimation",
  "BG_UpdatePlayerDObj",
  "SV_UserMove",
  "PmoveSingle",
  "PM_Move",
  "PM_Weapon",
  "PM_GroundTrace",
  "PM_Footsteps",
  "PM_FoliageSounds",
  "Mantle_Check",
  "SV_BuildClientSnapshot",
  "SV_PacketDataIsType",
  "WriteDeltaPlayerstate",
  "EmitPacketEntities",
  "EmitPacketClients",
  "SV_SendMessageToClient",
  "SV_TrackFieldChange",
  "SV_TrackPacketData",
  "WriteEntity",
  "SendPacket",
  "dwSendTo",
  "dwRecvFrom",
  "SV_SendClientSnapshot",
  "SV_UpdateBots",
  "SV_CalcPings",
  "SV_CheckTimeouts",
  "FakeLag_Frame",
  "xdelta",
  "xtiming",
  "xanim",
  "xskel",
  "xtrace",
  "staticModelTrace",
  "G_CalcTagParentAxis",
  "G_DObjGetLocalTagMatrix",
  "CG_DObjGetLocalTagMatrix",
  "CM_PointTraceToEntities",
  "CM_ClipMoveToEntities",
  "R_DObjCalcPose",
  "FX_GetBoneOrientation",
  "G_XAnimUpdate",
  "CG_AddPacketEntities",
  "CG_PredictPlayerState",
  "CG_DoControllers",
  "XAnimSetModel",
  "DObjCreateDuplicateParts",
  "DObjCreate",
  "DObjFree",
  "XAnimSetGoalWeight",
  "DObjGetHierarchyBits",
  "DObjCompleteHierarchyBits",
  "code_notify",
  "threads",
  "scriptVariable",
  "scriptString",
  "scriptMemory",
  "SND_init_sample",
  "SND_set_3d_sample_info",
  "SND_open_stream",
  "SND_Update",
  "SND_UpdateStream",
  "SND_spatialization",
  "SND_x3daudiocalculate",
  "SoundThread",
  "SND_entityeq",
  "DebugOverlays",
  "CG_Draw2D",
  "Actor_Grenade_GetTossPositionsFromHints",
  "Actor_Grenade_CheckGrenadeHintToss",
  "Actor_Grenade_CheckDesperateToss",
  "G_TouchTriggers",
  "G_DoTouchTriggers",
  "Actor_GetPotentialNodeList",
  "Actor_BadPlace_FindSafeNodeOutsideBadPlace",
  "VEH_VerifyPosition",
  "SwitchToLocalClient",
  "InactivePlayersSwitch",
  "CL_VoiceFrame",
  "SubmitChatData",
  "GetChatData",
  "MainThread",
  "MaxFPSSpin",
  "Live_Frame",
  "Phys_RunToTime",
  "Phys_Traces",
  "Phys_RunServerFrame",
  "Phys_Collide",
  "Phys_NearCallback",
  "Phys_WorldCollision",
  "Phys_WrldCollPt1",
  "Phys_WrldCollPt2",
  "Phys_OPCODE",
  "Phys_BoxTriColl",
  "Phys_BoxBoxColl",
  "Phys_BoxBrushColl",
  "Phys_BrushBrushColl",
  "Phys_BrushTriColl",
  "Phys_CylinderBrushColl",
  "Phys_CylinderTriColl",
  "Phys_CapsuleBrushColl",
  "Phys_CapsuleTriColl",
  "BldWndingsForBrsh",
  "Phys_Step",
  "Phys_PostStep",
  "Phys_ReduceContacts",
  "Phys_ODE_SOR_LCP",
  "DynEntCl_ProcessEntities",
  "DynEntCl_PhysRunToTime",
  "DynEntCl_DrawEntities",
  "DynEntCl_DrawModelEntity",
  "DynEntCl_DrawBrushEntity",
  "DynEntCl_PointTrace",
  "DynEntCl_ClipMoveTrace",
  "DynEntCl_AreaEntities",
  "DynEntCl_AddBreakableDrawSurfs",
  "DynEntSv_PointTrace",
  "DynEntSv_ClipMoveTrace",
  "DynEntSv_AreaEntities",
  "Ragdoll_GetDObjLocalBoneMatrix",
  "Ragdoll_UpdateRigidConstraint",
  "Ragdoll_Update",
  "ServerDemo_SaveHistory",
  "ServerDemo_CacheHistory",
  "FMOD_Release",
  "FMOD_Update",
  "FMOD_Codec",
  "FMOD_FileRead",
  "DrawDynEnt",
  "BspSurfaces",
  "BspSurfacesShadow",
  "SModelSurfaces",
  "SModelSurfacesShadow",
  "SModelLighting",
  "SceneEntSurfaces",
  "SortSceneEntSurfaces",
  "SceneEntSurfacesShadow",
  "R_GetLightSurfs",
  "FMOD_System_Lock",
  "WaitServer",
  "WaitSnapshot",
  "WaitReceiveMsg",
  "SoundCrit",
  "SoundOpen",
  "CodecOpen",
  "SoundMisc",
  "SoundReallyOpen",
  "SoundAlloc",
  "WaitBackendEvent",
  "PS3_SpeexDecode",
  "PS3_SpeexEncode",
  "PS3_VoiceDecoder",
  "PS3_VoiceMix",
  0
}; // idb

struct ProfileAtom // sizeof=0x4
{                                       // ...
    unsigned int value[1];              // ...
};
struct ProfileWritable // sizeof=0x1C
{                                       // ...
    int nesting;
    unsigned int hits;
    ProfileAtom start[3];
    ProfileAtom total;
    ProfileAtom child;
};
volatile struct ProfileReadable // sizeof=0xC
{                                       // ...
    unsigned int hits;
    ProfileAtom total;                  // ...
    ProfileAtom self;                   // ...
};

struct ProfileDrawInfo // sizeof=0x8
{                                       // ...
    int probeIndex;
    int indentation;
};

struct ProfileSettings // sizeof=0x20
{
    const char *name;
    const ProfileDrawInfo *profDrawInfo;
    int infoCount;
    int defaultProbeIndex[5];
};

struct ProfileProbe // sizeof=0x5A8
{                                       // ...
    int parity;                         // ...
    int historyCount;                   // ...
    ProfileReadable history[120];       // ...
};

struct __declspec(align(8)) ProfileReadableGlobal // sizeof=0x38
{                                       // ...
    int sequence;                       // ...
    unsigned int hits;
    long double totalClks;
    long double selfClks;
    ProfileAtom maxSelf;                // ...
    unsigned int maxHits;
    ProfileAtom min;
    ProfileAtom max;                    // ...
    ProfileReadable read;               // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
struct profile_t // sizeof=0x28
{                                       // ...
    ProfileWritable write;
    volatile ProfileReadable read;
};
struct profile_guard_t // sizeof=0x8
{                                       // ...
    int id;
    profile_t **ppStack;
};

struct ProfileStack // sizeof=0x144BC
{                                       // ...
    profile_t prof_root;
    profile_t *prof_pStack[16384];
    profile_t **prof_ppStack;
    profile_t prof_array[432];
    ProfileAtom prof_overhead_internal;
    ProfileAtom prof_overhead_external;
    profile_guard_t prof_guardstack[32];
    int prof_guardpos;
    float prof_timescale;
};

struct ProfileScriptWritable // sizeof=0xC
{                                       // ...
    int refCount;
    unsigned int startTime;
    unsigned int totalTime;             // ...
};
struct ProfileScript // sizeof=0x80C
{                                       // ...
    ProfileScriptWritable write[40];    // ...
    volatile unsigned int totalTime[40]; // ...
    volatile unsigned int avgTime[40];  // ...
    volatile unsigned int maxTime[40];  // ...
    volatile float cumulative[40];      // ...
    char profileScriptNames[40][20];    // ...
    int scriptSrcBufferIndex[32];
    unsigned int srcTotal;
    unsigned int srcAvgTime;
    unsigned int srcMaxTime;
};

static const ProfileDrawInfo s_profileMain[18] =
{
  { 255, 0 },
  { 230, 0 },
  { 27, 1 },
  { 332, 1 },
  { 320, 1 },
  { 260, 1 },
  { 261, 2 },
  { 11, 1 },
  { 12, 1 },
  { 34, 0 },
  { 13, 1 },
  { 26, 2 },
  { 14, 2 },
  { 71, 3 },
  { 66, 3 },
  { 15, 1 },
  { 339, 0 },
  { 340, 0 }
}; // idb
static const ProfileDrawInfo s_profileGenericCGame[12] =
{
  { 34, 0 },
  { 129, 1 },
  { 130, 1 },
  { 128, 1 },
  { 161, 1 },
  { 71, 1 },
  { 0, 0 },
  { 230, 0 },
  { 0, 0 },
  { 346, 0 },
  { 345, 0 },
  { 322, 0 }
}; // idb
static const ProfileDrawInfo s_profileGenericGame[16] =
{
  { 238, 1 },
  { 239, 2 },
  { 240, 1 },
  { 241, 2 },
  { 242, 0 },
  { 243, 0 },
  { 244, 0 },
  { 245, 0 },
  { 246, 0 },
  { 247, 0 },
  { 248, 0 },
  { 249, 0 },
  { 250, 0 },
  { 251, 0 },
  { 252, 0 },
  { 253, 0 }
}; // idb
static const ProfileDrawInfo s_profileProbes[12] =
{
  { 1, 0 },
  { 2, 0 },
  { 3, 0 },
  { 4, 0 },
  { 5, 0 },
  { 0, 0 },
  { 6, 0 },
  { 7, 0 },
  { 8, 0 },
  { 9, 0 },
  { 10, 0 },
  { 0, 0 }
}; // idb
static const ProfileDrawInfo s_profileFrontEnd[9] =
{
  { 34, 0 },
  { 71, 1 },
  { 188, 2 },
  { 85, 1 },
  { 86, 1 },
  { 87, 2 },
  { 88, 1 },
  { 89, 2 },
  { 70, 0 }
}; // idb
static const ProfileDrawInfo s_profileRenderer[16] =
{
  { 34, 0 },
  { 129, 0 },
  { 130, 0 },
  { 98, 0 },
  { 111, 0 },
  { 100, 0 },
  { 101, 0 },
  { 103, 0 },
  { 104, 0 },
  { 105, 0 },
  { 107, 0 },
  { 108, 0 },
  { 110, 0 },
  { 112, 0 },
  { 113, 1 },
  { 114, 1 }
}; // idb
static const ProfileDrawInfo s_profileRenderer2[9] =
{
  { 166, 0 },
  { 168, 1 },
  { 169, 1 },
  { 171, 1 },
  { 190, 1 },
  { 191, 1 },
  { 192, 1 },
  { 90, 0 },
  { 189, 0 }
}; // idb
static const ProfileDrawInfo s_profileRenderer3[22] =
{
  { 128, 0 },
  { 156, 0 },
  { 157, 1 },
  { 158, 1 },
  { 160, 0 },
  { 161, 0 },
  { 163, 1 },
  { 164, 0 },
  { 118, 1 },
  { 116, 0 },
  { 115, 1 },
  { 117, 2 },
  { 120, 0 },
  { 121, 0 },
  { 82, 0 },
  { 78, 0 },
  { 79, 1 },
  { 80, 1 },
  { 83, 1 },
  { 81, 0 },
  { 65, 0 },
  { 144, 0 }
}; // idb
static const ProfileDrawInfo s_profileCinematics[5] = { { 144, 0 }, { 145, 0 }, { 146, 1 }, { 147, 1 }, { 148, 1 } }; // idb
static const ProfileDrawInfo s_profileWater[7] =
{
  { 119, 0 },
  { 122, 1 },
  { 123, 1 },
  { 124, 1 },
  { 125, 1 },
  { 126, 1 },
  { 127, 1 }
}; // idb
static const ProfileDrawInfo s_profileSnd[4] = { { 336, 0 }, { 337, 0 }, { 338, 0 }, { 344, 0 } }; // idb
static const ProfileDrawInfo s_profileXanim[12] =
{
  { 313, 0 },
  { 314, 0 },
  { 315, 0 },
  { 318, 0 },
  { 319, 0 },
  { 320, 0 },
  { 321, 0 },
  { 307, 0 },
  { 308, 0 },
  { 309, 0 },
  { 310, 0 },
  { 311, 0 }
}; // idb
static const ProfileDrawInfo s_profileXanim2[8] =
{
  { 324, 0 },
  { 325, 0 },
  { 326, 0 },
  { 327, 0 },
  { 328, 0 },
  { 329, 0 },
  { 330, 0 },
  { 323, 0 }
}; // idb
static const ProfileDrawInfo s_profileScript[4] = { { 331, 0 }, { 332, 0 }, { 334, 0 }, { 335, 0 } }; // idb
static const ProfileDrawInfo s_profileFx[30] =
{
  { 193, 0 },
  { 194, 1 },
  { 203, 2 },
  { 204, 1 },
  { 205, 2 },
  { 206, 2 },
  { 207, 3 },
  { 215, 3 },
  { 216, 3 },
  { 217, 4 },
  { 218, 4 },
  { 219, 5 },
  { 208, 2 },
  { 195, 0 },
  { 209, 0 },
  { 210, 0 },
  { 211, 0 },
  { 196, 0 },
  { 197, 1 },
  { 198, 2 },
  { 199, 1 },
  { 200, 0 },
  { 201, 1 },
  { 202, 2 },
  { 212, 0 },
  { 213, 0 },
  { 97, 0 },
  { 134, 0 },
  { 136, 0 },
  { 135, 0 }
}; // idb
static const ProfileDrawInfo s_profilePrediction[2] = { { 34, 0 }, { 322, 1 } }; // idb
static const ProfileDrawInfo s_profileTrace[18] =
{
  { 37, 0 },
  { 38, 0 },
  { 39, 0 },
  { 40, 0 },
  { 35, 0 },
  { 36, 0 },
  { 41, 0 },
  { 316, 0 },
  { 312, 0 },
  { 42, 0 },
  { 43, 0 },
  { 44, 0 },
  { 45, 0 },
  { 46, 0 },
  { 47, 0 },
  { 48, 0 },
  { 317, 0 },
  { 233, 0 }
}; // idb
static const ProfileDrawInfo s_profileServer[37] =
{
  { 254, 0 },
  { 255, 0 },
  { 257, 0 },
  { 302, 1 },
  { 290, 2 },
  { 292, 2 },
  { 293, 2 },
  { 298, 3 },
  { 296, 4 },
  { 294, 2 },
  { 295, 2 },
  { 296, 3 },
  { 297, 3 },
  { 299, 2 },
  { 300, 3 },
  { 301, 0 },
  { 258, 0 },
  { 230, 0 },
  { 259, 1 },
  { 320, 1 },
  { 260, 1 },
  { 263, 1 },
  { 264, 1 },
  { 265, 1 },
  { 214, 3 },
  { 276, 2 },
  { 279, 2 },
  { 272, 2 },
  { 266, 1 },
  { 267, 1 },
  { 303, 0 },
  { 282, 0 },
  { 27, 0 },
  { 283, 1 },
  { 289, 2 },
  { 284, 2 },
  { 286, 2 }
}; // idb
static const ProfileDrawInfo s_profileTrigger[1] = { { 350, 0 } }; // idb

static const ProfileDrawInfo s_profileUpdateScreen[13] =
{
                {0x22, 0}    ,
                {0x12, 1}    ,
                {0x13, 2}    ,
                {0x14, 2}    ,
                {0x0D, 3}    ,
                {0x16, 4}    ,
                {0x17, 4}    ,
                {0x18, 4}    ,
                {0x15A, 4}   ,
                {0x19, 4}    ,
                {0x0E, 5}    ,
                {0x15, 2}    ,
                { 0x0F, 3 }  ,
};
static const ProfileDrawInfo s_profileThread[6] =
{
    {0x168, 0},
    {0xBD, 0},
    {0x9F, 0},
    {0x86, 0},
    {0x88, 0},
    {0x87, 0}
};

static const ProfileDrawInfo s_profilePhys[22] =
{
    { 0x16B, 0},
    { 0x16C, 1},
    { 0x16D, 1},
    { 0x16E, 2},
    { 0x16F, 3},
    { 0x173, 4},
    { 0x170, 4},
    { 0x171, 5},
    { 0x172, 5},
    { 0x174, 6},
    { 0x176, 6},
    { 0x177, 6},
    { 0x178, 6},
    { 0x179, 6},
    { 0x17A, 6},
    { 0x17B, 6},
    { 0x17C, 6},
    { 0x17D, 7},
    { 0x175, 4},
    { 0x180, 4},
    { 0x17E, 2},
    { 0x181, 3 },
};

static const ProfileDrawInfo s_shadowCookie[6] =
{
    {0x96, 0},
    {0x97, 0},
    {0x98, 1},
    {0x9B, 2},
    {0x99, 2},
    {0x9A, 0}
};

static const ProfileDrawInfo s_profileCgTrace[9] =
{
    {0x31, 0},
    {0x32, 0},
    {0x33, 0},
    {0x34, 0},
    {0x35, 0},
    {0x36, 1},
    {0x37, 2},
    {0x38, 1},
    {0x39, 2}
};

static const ProfileDrawInfo s_profileDynEnts[11] =
{
    {0x182, 0},
    {0x183, 2},
    {0x184, 1},
    {0x185, 2},
    {0x186, 2},
    {0x187, 0},
    {0x188, 0},
    {0x189, 0},
    {0x18B, 0},
    {0x18C, 0},
    {0x18D, 0},
};

static const ProfileDrawInfo s_profileAimAssist[7] =
{
    {0x3A, 0}  ,
    {0x3B, 1}  ,
    {0x3C, 1}  ,
    {0x3D, 1}  ,
    {0x3E, 0}  ,
    {0x3F, 0}  ,
    { 0x40, 0 },
};

static const ProfileSettings s_profileArrays[29] =
{
  { "main", s_profileMain, 18, {230, 34, 339, 0, 0}},
  { "updateScreen", s_profileUpdateScreen, 13, { 0, 0, 0, 0, 0 } },
  { "thread", s_profileThread, 6, { 360, 189, 0, 0, 0 } },
  { "generic", s_profileGenericCGame, 12, { 34, 230, 0, 0, 0 } },
  { "generic2", s_profileGenericGame, 16, { 243, 245, 246, 247, 253 } },
  { "probes", s_profileProbes, 12, { 1, 6, 0, 0, 0 } },
  { "probe", s_profileProbes, 12, { 1, 2, 3, 4, 5 } },
  { "gprobe", s_profileProbes, 12, { 6, 7, 8, 9, 10 } },
  { "frontend", s_profileFrontEnd, 9, { 34, 71, 89, 87, 88 } },
  { "renderer", s_profileRenderer, 16, { 34, 98, 112, 0, 0 } },
  { "renderer2", s_profileRenderer2, 9, { 166, 169, 171, 0, 0 } },
  { "renderer3", s_profileRenderer3, 22, { 121, 0, 0, 0, 0 } },
  { "cinematics", s_profileCinematics, 5, { 144, 145, 146, 147, 0 } },
  { "phys", s_profilePhys, 22, { 0, 0, 0, 0, 0 } },
  { "shadowcookie", s_shadowCookie, 6, { 0, 0, 0, 0, 0 } },
  { "water", s_profileWater, 7, { 119, 122, 123, 124, 0 } },
  { "snd", s_profileSnd, 4, { 336, 337, 338, 0, 0 } },
  { "xanim", s_profileXanim, 12, { 314, 315, 318, 320, 321 } },
  { "xanim2", s_profileXanim2, 8, { 326, 325, 324, 0, 0 } },
  { "script", s_profileScript, 4, { 332, 334, 0, 0, 0 } },
  { "fx", s_profileFx, 30, { 0, 0, 0, 0, 0 } },
  { "prediction", s_profilePrediction, 2, { 34, 322, 0, 0, 0 } },
  { "trace", s_profileTrace, 18, { 39, 47, 45, 48, 316 } },
  { "cgtrace", s_profileCgTrace, 9, { 49, 50, 51, 52, 0 } },
  { "server", s_profileServer, 37, { 0, 0, 0, 0, 0 } },
  { "trigger", s_profileTrigger, 1, { 350, 0, 0, 0, 0 } },
  { "dynent", s_profileDynEnts, 11, { 0, 0, 0, 0, 0 } },
  { "aimassist", s_profileAimAssist, 7, { 0, 0, 0, 0, 0 } },
  { NULL, NULL, 0, { 0, 0, 0, 0, 0 } }
}; // idb

void Profile_Init();
void __cdecl Profile_Guard(int id);
void __cdecl Profile_Unguard(int id);
void __cdecl Profile_SetTotal(int index, int total);
void Profile_ResetScriptCounters();
void __cdecl Profile_ResetCounters(int system);
void __cdecl Profile_Recover(int id);
void __cdecl Profile_InitContext(int profileContext);
void __cdecl Profile_InitContext(int profileContext);
ProfileStack *__cdecl Profile_GetStackForContext(int profileContext);
ProfileScript *__cdecl Profile_GetScript();
int __cdecl Profile_GetEnumParity(unsigned int profEnum);
int __cdecl Profile_GetDisplayThread();
void __cdecl Profile_EndScripts(unsigned int profileFlags);
void __cdecl Profile_EndScript(int profileIndex);
int __cdecl Profile_EndInternal(long double *duration);
void __cdecl Profile_BeginScripts(unsigned int profileFlags);
void __cdecl Profile_BeginScript(int profileIndex);
void __cdecl Profile_Begin(int index);
int __cdecl Profile_AddScriptName(char *profileName);
void __cdecl Profile_ResetCountersForContext(int profileContext, int system);
const char *__cdecl Profile_MissingEnd();

extern const dvar_t *profile;
extern const dvar_t *profile_thread;
extern const dvar_t *profile_rowcount;

extern ProfileScript profileScript;