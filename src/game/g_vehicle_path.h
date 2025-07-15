#pragma once

#include <universal/q_shared.h>

struct VehicleRideSlot_t // sizeof=0xC
{                                       // ...
    uint32_t tagName;
    int32_t boneIdx;
    int32_t entNum;
};
static_assert(sizeof(VehicleRideSlot_t) == 0xC);

struct vehicle_node_t // sizeof=0x44
{                                       // ...
    uint16_t name;
    uint16_t target;
    uint16_t script_linkname;
    uint16_t script_noteworthy;
    int16_t index;
    // padding byte
    // padding byte
    int32_t rotated;
    float speed;
    float lookAhead;
    float origin[3];
    float dir[3];
    float angles[3];
    float length;
    int16_t nextIdx;
    int16_t prevIdx;
};
static_assert(sizeof(vehicle_node_t) == 0x44);

struct vehicle_pathpos_t // sizeof=0xC0
{                                       // ...
    int16_t nodeIdx;
    int16_t endOfPath;
    float frac;
    float speed;
    float lookAhead;
    float slide;
    float origin[3];
    float angles[3];
    float lookPos[3];
    vehicle_node_t switchNode[2];
};
static_assert(sizeof(vehicle_pathpos_t) == 0xC0);

struct vehicle_physic_t // sizeof=0xF8
{                                       // ...
    float origin[3];
    float prevOrigin[3];
    float angles[3];
    float prevAngles[3];
    float maxAngleVel[3];
    float yawAccel;
    float yawDecel;
    char inputAccelerationOLD;
    char inputTurning;
    // padding byte
    // padding byte
    float driverPedal;
    float driverSteer;
    int32_t onGround;
    float colVelDelta[3];
    float mins[3];
    float maxs[3];
    float vel[3];
    float bodyVel[3];
    float rotVel[3];
    float accel[3];
    float maxPitchAngle;
    float maxRollAngle;
    float wheelZVel[4];
    float wheelZPos[4];
    int32_t wheelSurfType[4];
    float worldTilt[3];
    float worldTiltVel[3];
};
static_assert(sizeof(vehicle_physic_t) == 0xF8);

struct VehicleTags // sizeof=0x60
{                                       // ...
    VehicleRideSlot_t riderSlots[3];
    int32_t detach;
    int32_t popout;
    int32_t body;
    int32_t turret;
    int32_t turret_base;
    int32_t barrel;
    int32_t flash[5];
    int32_t wheel[4];
};
static_assert(sizeof(VehicleTags) == 0x60);

enum VehicleMoveState : __int32
{                                       // ...
    VEH_MOVESTATE_STOP = 0x0,
    VEH_MOVESTATE_MOVE = 0x1,
    VEH_MOVESTATE_HOVER = 0x2,
};

enum VehicleTurretState : __int32
{                                       // ...
    VEH_TURRET_STOPPED = 0x0,
    VEH_TURRET_STOPPING = 0x1,
    VEH_TURRET_MOVING = 0x2,
};

struct VehicleTurret // sizeof=0x14
{                                       // ...
    int32_t fireTime;
    int32_t fireBarrel;
    float barrelOffset;
    int32_t barrelBlocked;
    VehicleTurretState turretState;
};
static_assert(sizeof(VehicleTurret) == 0x14);

struct VehicleJitter // sizeof=0x3C
{                                       // ...
    int32_t jitterPeriodMin;
    int32_t jitterPeriodMax;
    int32_t jitterEndTime;
    float jitterOffsetRange[3];
    float jitterDeltaAccel[3];
    float jitterAccel[3];
    float jitterPos[3];
};
static_assert(sizeof(VehicleJitter) == 0x3C);

struct VehicleHover // sizeof=0x1C
{                                       // ...
    float hoverRadius;
    float hoverSpeed;
    float hoverAccel;
    float hoverGoalPos[3];
    int32_t useHoverAccelForAngles;
};
static_assert(sizeof(VehicleHover) == 0x1C);

#ifndef KISAK_MP
void __fastcall VP_AddDebugLine(float *start, float *end, int forceDraw);
void __fastcall VP_SetScriptVariable(const char *key, const char *value, vehicle_node_t *node);
void __fastcall VP_ParseField(const char *key, const char *value, vehicle_node_t *node);
void __fastcall VP_ParseFields(vehicle_node_t *node);
void __fastcall VP_ZeroNode(vehicle_node_t *node);
void __fastcall VP_ClearNode(vehicle_node_t *node);
void __fastcall VP_InitNode(vehicle_node_t *node, __int16 nodeIdx);
void __fastcall VP_CopyNode(const vehicle_node_t *src, vehicle_node_t *dst);
int __fastcall VP_GetNodeIndex(unsigned __int16 name, float *origin);
float __fastcall VP_CalcNodeSpeed(__int16 nodeIdx);
float __fastcall VP_CalcNodeLookAhead(__int16 nodeIdx);
void __fastcall VP_CalcNodeAngles(__int16 nodeIdx, float *angles);
float __fastcall VP_GetSpeed(const vehicle_pathpos_t *vpp);
float __fastcall VP_GetLookAhead(const vehicle_pathpos_t *vpp);
float __fastcall VP_GetSlide(const vehicle_pathpos_t *vpp);
void __fastcall VP_GetAngles(const vehicle_pathpos_t *vpp, float *angles);
void __fastcall VP_GetLookAheadXYZ(const vehicle_pathpos_t *vpp, float *lookXYZ);
int __fastcall VP_UpdatePathPos(vehicle_pathpos_t *vpp, const float *dir, __int16 nodeTest);
void __fastcall VP_BeginSwitchNode(const vehicle_pathpos_t *vpp);
void __fastcall VP_EndSwitchNode(const vehicle_pathpos_t *vpp);
void __fastcall G_InitVehiclePaths();
void __fastcall G_FreeVehiclePaths();
void __fastcall G_FreeVehiclePathsScriptInfo();
void __fastcall G_SetupVehiclePaths();
void __fastcall G_VehInitPathPos(vehicle_pathpos_t *vpp);
void __fastcall G_VehFreePathPos(vehicle_pathpos_t *vpp);
void __fastcall G_VehSetUpPathPos(vehicle_pathpos_t *vpp, __int16 nodeIdx);
int __fastcall G_VehUpdatePathPos(vehicle_pathpos_t *vpp, __int16 testNode);
void __fastcall G_VehSetSwitchNode(vehicle_pathpos_t *vpp, __int16 srcNodeIdx, __int16 dstNodeIdx);
void __fastcall TRACK_g_vehicle_path();
void __fastcall SP_info_vehicle_node(int rotated);
int __fastcall GScr_GetVehicleNodeIndex(scr_entref_t *index, unsigned int a2);
void __fastcall GScr_AddFieldsForVehicleNode();
void __fastcall GScr_GetVehicleNodeField(unsigned int entnum, unsigned int offset);
void __fastcall GScr_GetVehicleNode();
void __fastcall GScr_GetVehicleNodeArray();
void __fastcall GScr_GetAllVehicleNodes();
void __fastcall VP_DrawPath(const vehicle_pathpos_t *vpp);
void __fastcall G_DrawVehiclePaths();
#endif
