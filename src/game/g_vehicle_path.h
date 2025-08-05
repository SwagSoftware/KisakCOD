#pragma once

#include <universal/q_shared.h>

#ifndef KISAK_SP
#error This file is Single-Player only
#endif

struct vehicle_node_t;
struct vehicle_pathpos_t;

void __cdecl VP_AddDebugLine(float *start, float *end, int forceDraw);
void __cdecl VP_SetScriptVariable(const char *key, const char *value, vehicle_node_t *node);
void __cdecl VP_ParseField(const char *key, const char *value, vehicle_node_t *node);
void __cdecl VP_ParseFields(vehicle_node_t *node);
void __cdecl VP_ZeroNode(vehicle_node_t *node);
void __cdecl VP_ClearNode(vehicle_node_t *node);
void __cdecl VP_InitNode(vehicle_node_t *node, short nodeIdx);
void __cdecl VP_CopyNode(const vehicle_node_t *src, vehicle_node_t *dst);
int __cdecl VP_GetNodeIndex(unsigned short name, float *origin);
float __cdecl VP_CalcNodeSpeed(short nodeIdx);
float __cdecl VP_CalcNodeLookAhead(short nodeIdx);
void __cdecl VP_CalcNodeAngles(short nodeIdx, float *angles);
float __cdecl VP_GetSpeed(const vehicle_pathpos_t *vpp);
float __cdecl VP_GetLookAhead(const vehicle_pathpos_t *vpp);
float __cdecl VP_GetSlide(const vehicle_pathpos_t *vpp);
void __cdecl VP_GetAngles(const vehicle_pathpos_t *vpp, float *angles);
void __cdecl VP_GetLookAheadXYZ(const vehicle_pathpos_t *vpp, float *lookXYZ);
int __cdecl VP_UpdatePathPos(vehicle_pathpos_t *vpp, const float *dir, short nodeTest);
void __cdecl VP_BeginSwitchNode(const vehicle_pathpos_t *vpp);
void __cdecl VP_EndSwitchNode(const vehicle_pathpos_t *vpp);
void __cdecl G_InitVehiclePaths();
void __cdecl G_FreeVehiclePaths();
void __cdecl G_FreeVehiclePathsScriptInfo();
void __cdecl G_SetupVehiclePaths();
void __cdecl G_VehInitPathPos(vehicle_pathpos_t *vpp);
void __cdecl G_VehFreePathPos(vehicle_pathpos_t *vpp);
void __cdecl G_VehSetUpPathPos(vehicle_pathpos_t *vpp, short nodeIdx);
int __cdecl G_VehUpdatePathPos(vehicle_pathpos_t *vpp, short testNode);
void __cdecl G_VehSetSwitchNode(vehicle_pathpos_t *vpp, short srcNodeIdx, short dstNodeIdx);
void __cdecl TRACK_g_vehicle_path();
void __cdecl SP_info_vehicle_node(int rotated);
int __cdecl GScr_GetVehicleNodeIndex(int index);
void __cdecl GScr_AddFieldsForVehicleNode();
void __cdecl GScr_GetVehicleNodeField(unsigned int entnum, unsigned int offset);
void __cdecl GScr_GetVehicleNode();
void __cdecl GScr_GetVehicleNodeArray();
void __cdecl GScr_GetAllVehicleNodes();
void __cdecl VP_DrawPath(const vehicle_pathpos_t *vpp);
void __cdecl G_DrawVehiclePaths();
