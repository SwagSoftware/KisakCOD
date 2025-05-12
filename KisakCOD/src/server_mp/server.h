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

const NetField hudElemFields[] =
{
  { "color.rgba", 36, -85, 0u },
  { "fadeStartTime", 44, -97, 0u },
  { "fromColor.rgba", 40, -85, 0u },
  { "y", 8, -91, 0u },
  { "type", 0, 4, 0u },
  { "materialIndex", 64, 8, 0u },
  { "height", 60, 10, 0u },
  { "width", 56, 10, 0u },
  { "x", 4, -92, 0u },
  { "fadeTime", 48, 16, 0u },
  { "z", 12, -90, 0u },
  { "value", 120, 0, 0u },
  { "alignScreen", 32, 6, 0u },
  { "sort", 128, 0, 0u },
  { "alignOrg", 28, 4, 0u },
  { "offscreenMaterialIdx", 68, 8, 0u },
  { "fontScale", 20, -86, 0u },
  { "text", 124, 9, 0u },
  { "font", 24, 4, 0u },
  { "scaleStartTime", 80, -97, 0u },
  { "scaleTime", 84, 16, 0u },
  { "fromWidth", 72, 10, 0u },
  { "fromHeight", 76, 10, 0u },
  { "targetEntNum", 16, 10, 0u },
  { "glowColor.rgba", 132, -85, 0u },
  { "fxBirthTime", 136, -97, 0u },
  { "soundID", 152, 5, 0u },
  { "fxLetterTime", 140, 12, 0u },
  { "fxDecayStartTime", 144, 16, 0u },
  { "fxDecayDuration", 148, 16, 0u },
  { "flags", 156, 3, 0u },
  { "label", 52, 9, 0u },
  { "time", 112, -97, 0u },
  { "moveStartTime", 104, -97, 0u },
  { "moveTime", 108, 16, 0u },
  { "fromX", 88, -99, 0u },
  { "fromY", 92, -99, 0u },
  { "fromAlignScreen", 100, 6, 0u },
  { "fromAlignOrg", 96, 4, 0u },
  { "duration", 116, 32, 0u }
}; // idb

const NetField vehicleEntityStateFields[59] =
{
  { "eType", 4, 8, 1u },
  { "lerp.pos.trTime", 16, -97, 0u },
  { "lerp.pos.trBase[0]", 24, -92, 0u },
  { "lerp.pos.trBase[1]", 28, -91, 0u },
  { "lerp.pos.trDelta[0]", 36, 0, 0u },
  { "lerp.pos.trDelta[1]", 40, 0, 0u },
  { "lerp.u.vehicle.gunYaw", 104, 0, 0u },
  { "lerp.apos.trBase[1]", 64, -100, 0u },
  { "lerp.pos.trBase[2]", 32, -90, 0u },
  { "lerp.pos.trDelta[2]", 44, 0, 0u },
  { "lerp.apos.trBase[0]", 60, -100, 0u },
  { "eventSequence", 160, 8, 0u },
  { "legsAnim", 204, 10, 0u },
  { "surfType", 132, 8, 0u },
  { "otherEntityNum", 116, 10, 0u },
  { "un1", 212, 8, 0u },
  { "lerp.eFlags", 8, -98, 0u },
  { "groundEntityNum", 124, -96, 0u },
  { "clientNum", 140, 7, 0u },
  { "events[0]", 164, -94, 0u },
  { "events[1]", 168, -94, 0u },
  { "events[2]", 172, -94, 0u },
  { "weapon", 196, 7, 0u },
  { "weaponModel", 200, 4, 0u },
  { "eventParms[1]", 184, -93, 0u },
  { "eventParms[0]", 180, -93, 0u },
  { "eventParms[2]", 188, -93, 0u },
  { "index", 136, 10, 0u },
  { "lerp.u.vehicle.materialTime", 96, -97, 0u },
  { "lerp.pos.trType", 12, 8, 0u },
  { "lerp.apos.trType", 48, 8, 0u },
  { "events[3]", 176, -94, 0u },
  { "lerp.apos.trBase[2]", 68, -100, 0u },
  { "lerp.apos.trTime", 52, 32, 0u },
  { "lerp.apos.trDelta[0]", 72, 0, 0u },
  { "lerp.apos.trDelta[2]", 80, 0, 0u },
  { "torsoAnim", 208, 10, 0u },
  { "eventParms[3]", 192, -93, 0u },
  { "lerp.u.vehicle.gunPitch", 100, 0, 0u },
  { "solid", 152, 24, 0u },
  { "lerp.pos.trDuration", 20, 32, 0u },
  { "lerp.apos.trDelta[1]", 76, 0, 0u },
  { "un2", 216, 32, 0u },
  { "time2", 112, -97, 0u },
  { "loopSound", 128, 8, 0u },
  { "attackerEntityNum", 120, 10, 0u },
  { "fWaistPitch", 224, 0, 0u },
  { "fTorsoPitch", 220, 0, 0u },
  { "lerp.u.vehicle.bodyPitch", 84, -100, 0u },
  { "lerp.u.vehicle.bodyRoll", 88, -100, 0u },
  { "iHeadIcon", 144, 4, 0u },
  { "iHeadIconTeam", 148, 2, 0u },
  { "eventParm", 156, -93, 0u },
  { "lerp.u.vehicle.steerYaw", 92, 0, 0u },
  { "lerp.apos.trDuration", 56, 32, 0u },
  { "partBits[0]", 228, 32, 1u },
  { "partBits[1]", 232, 32, 1u },
  { "partBits[2]", 236, 32, 1u },
  { "partBits[3]", 240, 32, 1u }
};

const NetField planeStateFields[60] =
{
  { "eType", 4, 8, 1u },
  { "lerp.pos.trBase[0]", 24, 0, 2u },
  { "lerp.pos.trBase[1]", 28, 0, 2u },
  { "lerp.pos.trBase[2]", 32, 0, 2u },
  { "index", 136, 10, 2u },
  { "lerp.pos.trDelta[0]", 36, 0, 2u },
  { "lerp.pos.trDelta[1]", 40, 0, 2u },
  { "lerp.pos.trTime", 16, -97, 2u },
  { "lerp.pos.trType", 12, 8, 2u },
  { "lerp.pos.trDuration", 20, 32, 2u },
  { "lerp.u.vehicle.teamAndOwnerIndex", 108, 8, 2u },
  { "lerp.apos.trBase[1]", 64, -100, 2u },
  { "lerp.pos.trDelta[2]", 44, 0, 0u },
  { "events[0]", 164, -94, 0u },
  { "eventSequence", 160, 8, 0u },
  { "eventParms[0]", 180, -93, 0u },
  { "events[1]", 168, -94, 0u },
  { "eventParms[1]", 184, -93, 0u },
  { "loopSound", 128, 8, 0u },
  { "lerp.apos.trType", 48, 8, 0u },
  { "eventParm", 156, -93, 0u },
  { "weapon", 196, 7, 0u },
  { "weaponModel", 200, 4, 0u },
  { "surfType", 132, 8, 0u },
  { "lerp.u.anonymous.data[0]", 84, 32, 0u },
  { "time2", 112, -97, 0u },
  { "solid", 152, 24, 0u },
  { "un2", 216, 32, 0u },
  { "groundEntityNum", 124, -96, 0u },
  { "un1", 212, 8, 0u },
  { "lerp.apos.trBase[0]", 60, -100, 0u },
  { "clientNum", 140, 7, 0u },
  { "events[2]", 172, -94, 0u },
  { "eventParms[2]", 188, -93, 0u },
  { "events[3]", 176, -94, 0u },
  { "lerp.apos.trBase[2]", 68, -100, 0u },
  { "lerp.apos.trTime", 52, 32, 0u },
  { "lerp.apos.trDelta[0]", 72, 0, 0u },
  { "lerp.apos.trDelta[2]", 80, 0, 0u },
  { "eventParms[3]", 192, -93, 0u },
  { "lerp.apos.trDelta[1]", 76, 0, 0u },
  { "attackerEntityNum", 120, 10, 0u },
  { "fWaistPitch", 224, 0, 0u },
  { "fTorsoPitch", 220, 0, 0u },
  { "iHeadIcon", 144, 4, 0u },
  { "iHeadIconTeam", 148, 2, 0u },
  { "lerp.apos.trDuration", 56, 32, 0u },
  { "torsoAnim", 208, 10, 0u },
  { "lerp.eFlags", 8, -98, 0u },
  { "legsAnim", 204, 10, 0u },
  { "otherEntityNum", 116, 10, 0u },
  { "lerp.u.anonymous.data[1]", 88, 32, 0u },
  { "lerp.u.anonymous.data[2]", 92, 32, 0u },
  { "lerp.u.anonymous.data[3]", 96, 32, 1u },
  { "lerp.u.anonymous.data[4]", 100, 32, 1u },
  { "lerp.u.anonymous.data[5]", 104, 32, 1u },
  { "partBits[0]", 228, 32, 1u },
  { "partBits[1]", 232, 32, 1u },
  { "partBits[2]", 236, 32, 1u },
  { "partBits[3]", 240, 32, 1u }
};


const NetField helicopterEntityStateFields[58] =
{
  { "eType", 4, 8, 1u },
  { "lerp.pos.trBase[0]", 24, -92, 0u },
  { "lerp.apos.trBase[0]", 60, -100, 0u },
  { "lerp.apos.trBase[2]", 68, -100, 0u },
  { "lerp.u.vehicle.gunPitch", 100, 0, 0u },
  { "lerp.pos.trBase[1]", 28, -91, 0u },
  { "lerp.apos.trBase[1]", 64, -100, 0u },
  { "lerp.pos.trBase[2]", 32, -90, 0u },
  { "lerp.u.vehicle.gunYaw", 104, 0, 0u },
  { "eventSequence", 160, 8, 0u },
  { "un1.helicopterStage", 212, 3, 0u },
  { "time2", 112, -97, 0u },
  { "events[0]", 164, -94, 0u },
  { "events[1]", 168, -94, 0u },
  { "events[2]", 172, -94, 0u },
  { "events[3]", 176, -94, 0u },
  { "eventParms[1]", 184, -93, 0u },
  { "eventParms[0]", 180, -93, 0u },
  { "eventParms[2]", 188, -93, 0u },
  { "eventParms[3]", 192, -93, 0u },
  { "loopSound", 128, 8, 0u },
  { "lerp.pos.trType", 12, 8, 0u },
  { "lerp.apos.trType", 48, 8, 0u },
  { "un2", 216, 32, 0u },
  { "weapon", 196, 7, 0u },
  { "lerp.u.vehicle.teamAndOwnerIndex", 108, 8, 0u },
  { "weaponModel", 200, 4, 0u },
  { "groundEntityNum", 124, -96, 0u },
  { "eventParm", 156, -93, 0u },
  { "lerp.pos.trDelta[0]", 36, 0, 0u },
  { "lerp.pos.trDelta[1]", 40, 0, 0u },
  { "lerp.pos.trDelta[2]", 44, 0, 0u },
  { "lerp.pos.trTime", 16, -97, 0u },
  { "legsAnim", 204, 10, 0u },
  { "surfType", 132, 8, 0u },
  { "otherEntityNum", 116, 10, 0u },
  { "lerp.eFlags", 8, -98, 0u },
  { "index", 136, 10, 0u },
  { "lerp.u.vehicle.materialTime", 96, -97, 0u },
  { "lerp.apos.trTime", 52, 32, 0u },
  { "lerp.apos.trDelta[0]", 72, 0, 0u },
  { "lerp.apos.trDelta[2]", 80, 0, 0u },
  { "torsoAnim", 208, 10, 0u },
  { "solid", 152, 24, 0u },
  { "lerp.pos.trDuration", 20, 32, 0u },
  { "lerp.apos.trDelta[1]", 76, 0, 0u },
  { "attackerEntityNum", 120, 10, 0u },
  { "fWaistPitch", 224, 0, 0u },
  { "fTorsoPitch", 220, 0, 0u },
  { "lerp.u.vehicle.bodyPitch", 84, -100, 0u },
  { "lerp.u.vehicle.bodyRoll", 88, -100, 0u },
  { "iHeadIcon", 144, 4, 0u },
  { "iHeadIconTeam", 148, 2, 0u },
  { "lerp.apos.trDuration", 56, 32, 0u },
  { "partBits[0]", 228, 32, 1u },
  { "partBits[1]", 232, 32, 1u },
  { "partBits[2]", 236, 32, 1u },
  { "partBits[3]", 240, 32, 1u }
};

const NetField entityStateFields[] =
{
  { "eType", 4, 8, 0u },
  { "lerp.eFlags", 8, -98, 0u },
  { "lerp.pos.trBase[0]", 24, -92, 0u },
  { "lerp.pos.trBase[1]", 28, -91, 0u },
  { "lerp.pos.trBase[2]", 32, -90, 0u },
  { "events[0]", 164, -94, 0u },
  { "eventSequence", 160, 8, 0u },
  { "weapon", 196, 7, 0u },
  { "weaponModel", 200, 4, 0u },
  { "eventParms[0]", 180, -93, 0u },
  { "surfType", 132, 8, 0u },
  { "lerp.u.anonymous.data[0]", 84, 32, 0u },
  { "time2", 112, -97, 0u },
  { "index", 136, 10, 0u },
  { "solid", 152, 24, 0u },
  { "un2", 216, 32, 0u },
  { "groundEntityNum", 124, -96, 0u },
  { "un1", 212, 8, 0u },
  { "lerp.apos.trBase[1]", 64, -100, 0u },
  { "lerp.apos.trBase[0]", 60, -100, 0u },
  { "clientNum", 140, 7, 0u },
  { "lerp.pos.trDelta[0]", 36, 0, 0u },
  { "lerp.pos.trDelta[1]", 40, 0, 0u },
  { "lerp.pos.trDelta[2]", 44, 0, 0u },
  { "events[1]", 168, -94, 0u },
  { "events[2]", 172, -94, 0u },
  { "eventParms[1]", 184, -93, 0u },
  { "eventParms[2]", 188, -93, 0u },
  { "lerp.pos.trTime", 16, -97, 0u },
  { "lerp.pos.trType", 12, 8, 0u },
  { "eventParm", 156, -93, 0u },
  { "lerp.apos.trType", 48, 8, 0u },
  { "events[3]", 176, -94, 0u },
  { "lerp.apos.trBase[2]", 68, -100, 0u },
  { "lerp.apos.trTime", 52, 32, 0u },
  { "lerp.apos.trDelta[0]", 72, 0, 0u },
  { "lerp.apos.trDelta[2]", 80, 0, 0u },
  { "eventParms[3]", 192, -93, 0u },
  { "lerp.pos.trDuration", 20, 32, 0u },
  { "lerp.apos.trDelta[1]", 76, 0, 0u },
  { "attackerEntityNum", 120, 10, 0u },
  { "fWaistPitch", 224, 0, 0u },
  { "fTorsoPitch", 220, 0, 0u },
  { "iHeadIcon", 144, 4, 0u },
  { "iHeadIconTeam", 148, 2, 0u },
  { "lerp.apos.trDuration", 56, 32, 0u },
  { "torsoAnim", 208, 10, 0u },
  { "legsAnim", 204, 10, 0u },
  { "loopSound", 128, 8, 0u },
  { "otherEntityNum", 116, 10, 0u },
  { "lerp.u.anonymous.data[1]", 88, 32, 0u },
  { "lerp.u.anonymous.data[2]", 92, 32, 0u },
  { "lerp.u.anonymous.data[3]", 96, 32, 0u },
  { "lerp.u.anonymous.data[4]", 100, 32, 0u },
  { "lerp.u.anonymous.data[5]", 104, 32, 0u },
  { "partBits[0]", 228, 32, 0u },
  { "partBits[1]", 232, 32, 0u },
  { "partBits[2]", 236, 32, 0u },
  { "partBits[3]", 240, 32, 0u }
}; // idb
const NetField clientStateFields[] =
{
  { "modelindex", 8, 9, 0u },
  { "name[0]", 60, 32, 0u },
  { "rank", 80, 8, 0u },
  { "prestige", 84, 8, 0u },
  { "team", 4, 2, 0u },
  { "attachedVehEntNum", 92, 10, 0u },
  { "name[4]", 64, 32, 0u },
  { "attachModelIndex[0]", 12, 9, 0u },
  { "name[8]", 68, 32, 0u },
  { "perks", 88, 32, 0u },
  { "name[12]", 72, 32, 0u },
  { "attachModelIndex[1]", 16, 9, 0u },
  { "maxSprintTimeMultiplier", 76, 0, 0u },
  { "attachedVehSlotIndex", 96, 2, 0u },
  { "attachTagIndex[5]", 56, 5, 0u },
  { "attachTagIndex[0]", 36, 5, 0u },
  { "attachTagIndex[1]", 40, 5, 0u },
  { "attachTagIndex[2]", 44, 5, 0u },
  { "attachTagIndex[3]", 48, 5, 0u },
  { "attachTagIndex[4]", 52, 5, 0u },
  { "attachModelIndex[2]", 20, 9, 0u },
  { "attachModelIndex[3]", 24, 9, 0u },
  { "attachModelIndex[4]", 28, 9, 0u },
  { "attachModelIndex[5]", 32, 9, 0u }
}; // idb
const NetField playerStateFields[141] =
{
  { "commandTime", 0, -97, 0u },
  { "viewangles[1]", 268, -87, 0u },
  { "viewangles[0]", 264, -87, 0u },
  { "viewangles[2]", 272, -87, 0u },
  { "origin[0]", 28, -88, 3u },
  { "origin[1]", 32, -88, 3u },
  { "bobCycle", 8, 8, 3u },
  { "velocity[1]", 44, -88, 3u },
  { "velocity[0]", 40, -88, 3u },
  { "movementDir", 172, -8, 3u },
  { "eventSequence", 180, 8, 0u },
  { "legsAnim", 140, 10, 0u },
  { "origin[2]", 36, -88, 3u },
  { "weaponTime", 60, -16, 0u },
  { "aimSpreadScale", 1576, -88, 0u },
  { "torsoTimer", 144, 16, 0u },
  { "pm_flags", 12, 21, 0u },
  { "weapAnim", 1572, 10, 0u },
  { "weaponstate", 236, 5, 0u },
  { "velocity[2]", 48, -88, 3u },
  { "events[0]", 184, 8, 0u },
  { "events[1]", 188, 8, 0u },
  { "events[2]", 192, 8, 0u },
  { "events[3]", 196, 8, 0u },
  { "eventParms[0]", 200, 8, 0u },
  { "eventParms[1]", 204, 8, 0u },
  { "eventParms[2]", 208, 8, 0u },
  { "eventParms[3]", 212, 8, 0u },
  { "torsoAnim", 148, 10, 0u },
  { "holdBreathScale", 1492, -88, 0u },
  { "eFlags", 176, -98, 0u },
  { "viewHeightCurrent", 280, -88, 0u },
  { "fWeaponPosFrac", 244, -88, 0u },
  { "legsTimer", 136, 16, 0u },
  { "viewHeightTarget", 276, -8, 0u },
  { "sprintState.lastSprintStart", 1472, -97, 0u },
  { "sprintState.lastSprintEnd", 1476, -97, 0u },
  { "weapon", 232, 7, 0u },
  { "weaponDelay", 64, -16, 0u },
  { "sprintState.sprintStartMaxLength", 1480, 14, 0u },
  { "weapFlags", 16, 9, 0u },
  { "groundEntityNum", 112, 10, 0u },
  { "damageTimer", 160, 10, 0u },
  { "weapons[0]", 1372, 32, 0u },
  { "weapons[1]", 1376, 32, 0u },
  { "weaponold[0]", 1388, 32, 0u },
  { "delta_angles[1]", 104, -100, 0u },
  { "offHandIndex", 224, 7, 0u },
  { "pm_time", 24, -16, 0u },
  { "otherFlags", 20, 5, 0u },
  { "moveSpeedScaleMultiplier", 1500, 0, 0u },
  { "perks", 1532, 32, 0u },
  { "killCamEntity", 2208, 10, 0u },
  { "throwBackGrenadeOwner", 72, 10, 0u },
  { "actionSlotType[2]", 1544, 2, 0u },
  { "delta_angles[0]", 100, -100, 0u },
  { "speed", 96, 16, 0u },
  { "viewlocked_entNum", 1436, 16, 0u },
  { "gravity", 88, 16, 0u },
  { "actionSlotType[0]", 1536, 2, 0u },
  { "dofNearBlur", 1608, 0, 0u },
  { "dofFarBlur", 1612, 0, 0u },
  { "clientNum", 220, 8, 0u },
  { "damageEvent", 312, 8, 0u },
  { "viewHeightLerpTarget", 288, -8, 0u },
  { "damageYaw", 316, 8, 0u },
  { "viewmodelIndex", 260, 9, 0u },
  { "damageDuration", 164, 16, 0u },
  { "damagePitch", 320, 8, 0u },
  { "flinchYawAnim", 168, 2, 0u },
  { "weaponShotCount", 240, 3, 0u },
  { "viewHeightLerpDown", 292, 1, 2u },
  { "cursorHint", 1440, 8, 0u },
  { "cursorHintString", 1444, -8, 0u },
  { "cursorHintEntIndex", 1448, 10, 0u },
  { "viewHeightLerpTime", 284, 32, 0u },
  { "offhandSecondary", 228, 1, 2u },
  { "radarEnabled", 1456, 1, 2u },
  { "pm_type", 4, 8, 0u },
  { "fTorsoPitch", 1484, 0, 0u },
  { "holdBreathTimer", 1496, 16, 0u },
  { "actionSlotParam[2]", 1560, 7, 0u },
  { "jumpTime", 128, 32, 0u },
  { "mantleState.flags", 1516, 5, 0u },
  { "fWaistPitch", 1488, 0, 0u },
  { "grenadeTimeLeft", 68, -16, 0u },
  { "proneDirection", 1420, 0, 0u },
  { "mantleState.timer", 1508, 32, 0u },
  { "damageCount", 324, 7, 0u },
  { "shellshockTime", 1584, -97, 0u },
  { "shellshockDuration", 1588, 16, 2u },
  { "sprintState.sprintButtonUpRequired", 1464, 1, 2u },
  { "shellshockIndex", 1580, 4, 0u },
  { "proneTorsoPitch", 1428, 0, 0u },
  { "sprintState.sprintDelay", 1468, 1, 2u },
  { "actionSlotParam[3]", 1564, 7, 0u },
  { "weapons[3]", 1384, 32, 0u },
  { "actionSlotType[3]", 1548, 2, 0u },
  { "proneDirectionPitch", 1424, 0, 0u },
  { "jumpOriginZ", 132, 0, 0u },
  { "mantleState.yaw", 1504, 0, 0u },
  { "mantleState.transIndex", 1512, 4, 0u },
  { "weaponrechamber[0]", 1404, 32, 0u },
  { "throwBackGrenadeTimeLeft", 76, -16, 0u },
  { "weaponold[3]", 1400, 32, 0u },
  { "weaponold[1]", 1392, 32, 0u },
  { "foliageSoundTime", 84, -97, 0u },
  { "vLadderVec[0]", 116, 0, 0u },
  { "viewlocked", 1432, 2, 0u },
  { "deltaTime", 2204, 32, 0u },
  { "viewAngleClampRange[1]", 308, 0, 0u },
  { "viewAngleClampBase[1]", 300, 0, 0u },
  { "viewAngleClampRange[0]", 304, 0, 0u },
  { "vLadderVec[1]", 120, 0, 0u },
  { "locationSelectionInfo", 1460, 8, 0u },
  { "meleeChargeTime", 1528, -97, 0u },
  { "meleeChargeYaw", 1520, -100, 0u },
  { "meleeChargeDist", 1524, 8, 0u },
  { "iCompassPlayerInfo", 1452, 32, 0u },
  { "weapons[2]", 1380, 32, 0u },
  { "actionSlotType[1]", 1540, 2, 0u },
  { "weaponold[2]", 1396, 32, 0u },
  { "vLadderVec[2]", 124, 0, 0u },
  { "weaponRestrictKickTime", 80, -16, 0u },
  { "delta_angles[2]", 108, -100, 0u },
  { "spreadOverride", 252, 6, 0u },
  { "spreadOverrideState", 256, 2, 0u },
  { "actionSlotParam[0]", 1552, 7, 0u },
  { "actionSlotParam[1]", 1556, 7, 0u },
  { "dofNearStart", 1592, 0, 0u },
  { "dofNearEnd", 1596, 0, 0u },
  { "dofFarStart", 1600, 0, 0u },
  { "dofFarEnd", 1604, 0, 0u },
  { "dofViewmodelStart", 1616, 0, 0u },
  { "dofViewmodelEnd", 1620, 0, 0u },
  { "viewAngleClampBase[0]", 296, 0, 0u },
  { "weaponrechamber[1]", 1408, 32, 0u },
  { "weaponrechamber[2]", 1412, 32, 0u },
  { "weaponrechamber[3]", 1416, 32, 0u },
  { "leanf", 92, 0, 0u },
  { "adsDelayTime", 248, 32, 1u }
};

const NetField archivedEntityFields[] =
{
  { "absmin[1]", 260, 0, 0u },
  { "absmax[1]", 272, 0, 0u },
  { "absmin[0]", 256, 0, 0u },
  { "absmax[0]", 268, 0, 0u },
  { "lerp.pos.trBase[1]", 28, 0, 0u },
  { "lerp.pos.trBase[0]", 24, 0, 0u },
  { "absmax[2]", 276, 0, 0u },
  { "lerp.pos.trBase[2]", 32, 0, 0u },
  { "absmin[2]", 264, 0, 0u },
  { "groundEntityNum", 124, 10, 0u },
  { "lerp.apos.trBase[1]", 64, 0, 0u },
  { "eType", 4, 8, 0u },
  { "lerp.apos.trBase[0]", 60, 0, 0u },
  { "clientNum", 140, 7, 0u },
  { "lerp.apos.trBase[2]", 68, 0, 0u },
  { "lerp.eFlags", 8, -98, 0u },
  { "svFlags", 244, 32, 0u },
  { "events[0]", 164, 8, 0u },
  { "eventSequence", 160, 8, 0u },
  { "index", 136, 10, 0u },
  { "legsAnim", 204, 10, 0u },
  { "events[1]", 168, -94, 0u },
  { "events[2]", 172, -94, 0u },
  { "events[3]", 176, -94, 0u },
  { "weapon", 196, 7, 0u },
  { "weaponModel", 200, 4, 0u },
  { "lerp.pos.trType", 12, 8, 0u },
  { "lerp.apos.trType", 48, 8, 0u },
  { "iHeadIcon", 144, 4, 0u },
  { "iHeadIconTeam", 148, 2, 0u },
  { "solid", 152, 24, 0u },
  { "eventParms[0]", 180, -93, 0u },
  { "torsoAnim", 208, 10, 0u },
  { "lerp.u.anonymous.data[1]", 88, 32, 0u },
  { "lerp.pos.trTime", 16, -97, 0u },
  { "lerp.pos.trDelta[0]", 36, 0, 0u },
  { "lerp.pos.trDelta[1]", 40, 0, 0u },
  { "lerp.pos.trDelta[2]", 44, 0, 0u },
  { "otherEntityNum", 116, 10, 0u },
  { "eventParms[1]", 184, -93, 0u },
  { "surfType", 132, 8, 0u },
  { "eventParm", 156, -93, 0u },
  { "eventParms[2]", 188, -93, 0u },
  { "un1", 212, 8, 0u },
  { "eventParms[3]", 192, 8, 0u },
  { "lerp.pos.trDuration", 20, 32, 0u },
  { "fWaistPitch", 224, 0, 0u },
  { "fTorsoPitch", 220, 0, 0u },
  { "lerp.apos.trTime", 52, -97, 0u },
  { "lerp.apos.trDelta[0]", 72, 0, 0u },
  { "lerp.apos.trDelta[2]", 80, 0, 0u },
  { "clientMask[0]", 248, 32, 0u },
  { "clientMask[1]", 252, 32, 0u },
  { "lerp.apos.trDelta[1]", 76, 0, 0u },
  { "lerp.u.anonymous.data[0]", 84, 32, 0u },
  { "attackerEntityNum", 120, 10, 0u },
  { "time2", 112, -97, 0u },
  { "lerp.u.anonymous.data[2]", 92, 32, 0u },
  { "un2", 216, 32, 0u },
  { "lerp.apos.trDuration", 56, 32, 0u },
  { "loopSound", 128, 8, 0u },
  { "lerp.u.anonymous.data[3]", 96, 32, 0u },
  { "lerp.u.anonymous.data[4]", 100, 32, 0u },
  { "lerp.u.anonymous.data[5]", 104, 32, 0u },
  { "lerp.u.anonymous.data[6]", 108, 32, 0u },
  { "partBits[0]", 228, 32, 0u },
  { "partBits[1]", 232, 32, 0u },
  { "partBits[2]", 236, 32, 0u },
  { "partBits[3]", 240, 32, 0u }
}; // idb

struct serverStaticHeader_t // sizeof=0x84
{                                       // ...
    struct client_t *clients;                  // ...
    int time;                           // ...
    int snapFlagServerBit;              // ...
    int numSnapshotEntities;            // ...
    int numSnapshotClients;             // ...
    int nextSnapshotEntities;           // ...
    int nextSnapshotClients;            // ...
    entityState_s *snapshotEntities;    // ...
    clientState_s *snapshotClients;     // ...
    svEntity_s *svEntities;             // ...
    float mapCenter[3];                 // ...
    archivedEntity_s *cachedSnapshotEntities; // ...
    struct cachedClient_s *cachedSnapshotClients; // ...
    unsigned __int8 *archivedSnapshotBuffer; // ...
    struct cachedSnapshot_t *cachedSnapshotFrames; // ...
    int nextCachedSnapshotFrames;       // ...
    int nextArchivedSnapshotFrames;     // ...
    int nextCachedSnapshotEntities;     // ...
    int nextCachedSnapshotClients;      // ...
    int num_entities;                   // ...
    int maxclients;                     // ...
    int fps;                            // ...
    int clientArchive;                  // ...
    gentity_s *gentities;               // ...
    int gentitySize;                    // ...
    clientState_s *firstClientState;    // ...
    playerState_s *firstPlayerState;    // ...
    int clientSize;                     // ...
    unsigned int pad[3];
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

struct client_t // sizeof=0xA5638
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
void __cdecl SV_SetConfigstring(int index, const char *val);
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
struct challenge_t // sizeof=0x50
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
void __cdecl SV_SetSystemInfoConfig();
void __cdecl SV_PreFrame();
int __cdecl SV_Frame(int msec);
void __cdecl SV_FrameInternal(int msec);
void SV_PostFrame();
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

extern const dvar_t *sv_punkbuster;
extern const dvar_t *sv_allowAnonymous;
extern const dvar_t *sv_privatePassword;
extern const dvar_t *sv_allowDownload;
extern const dvar_t *sv_iwds;
extern const dvar_t *sv_iwdNames;
extern const dvar_t *sv_referencedIwds;
extern const dvar_t *sv_referencedIwdNames;
extern const dvar_t *sv_FFCheckSums;
extern const dvar_t *sv_FFNames;
extern const dvar_t *sv_referencedFFCheckSums;
extern const dvar_t *sv_referencedFFNames;
extern const dvar_t *sv_voice;
extern const dvar_t *sv_voiceQuality;
extern const dvar_t *sv_pure;
extern const dvar_t *rcon_password;
extern const dvar_t *sv_wwwDownload;
extern const dvar_t *sv_wwwBaseURL;
extern const dvar_t *sv_wwwDlDisconnected;
extern const dvar_t *sv_loadMyChanges;
extern const dvar_t *sv_clientArchive;


// sv_net_chan_mp
bool __cdecl SV_Netchan_TransmitNextFragment(client_t *client, netchan_t *chan);
void __cdecl SV_Netchan_OutgoingSequenceIncremented(client_t *client, netchan_t *chan);
bool __cdecl SV_Netchan_Transmit(client_t *client, unsigned __int8 *data, int length);
void __cdecl SV_Netchan_AddOOBProfilePacket(int iLength);
void __cdecl SV_Netchan_UpdateProfileStats();
void __cdecl SV_Netchan_PrintProfileStats(int bPrintToConsole);
void __cdecl SV_Netchan_Decode(client_t *client, unsigned __int8 *data, int size);



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
void __cdecl SV_Download_Clear(client_t *cl);
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
void __cdecl SV_CloseDownload(client_t *cl);

extern serverStaticHeader_t svsHeader;


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

extern int sv_serverId_value;

// sv_main_pc_mp
const netadr_t *__cdecl SV_MasterAddress();
void __cdecl SV_MasterGameCompleteStatus();
void __cdecl SV_MasterHeartbeat(const char *hbname);
void __cdecl SV_MasterShutdown();
void __cdecl SV_UpdateLastTimeMasterServerCommunicated(netadr_t from);
void __cdecl SV_AuthorizeIpPacket(netadr_t from);
void __cdecl SVC_RemoteCommand(netadr_t from);
void __cdecl SV_VoicePacket(netadr_t from, msg_t *msg);


// sv_snapshot_profile_mp
enum packetModeList : __int32
{                                       // ...
    PACKETDATA_FIRST = 0x0,
    PACKETDATA_UNDEFINED = 0x0,
    PACKETDATA_HEADER = 0x1,
    PACKETDATA_OVERHEAD = 0x2,
    PACKETDATA_DATA = 0x3,
    PACKETDATA_RELIABLEDATA = 0x4,
    PACKETDATA_ZEROFLOAT = 0x5,
    PACKETDATA_SMALLFLOAT = 0x6,
    PACKETDATA_LARGEFLOAT = 0x7,
    PACKETDATA_ZEROINT = 0x8,
    PACKETDATA_SMALLANGLE = 0x9,
    PACKETDATA_ZEROANGLE = 0xA,
    PACKETDATA_TIMEDELTA = 0xB,
    PACKETDATA_TIME = 0xC,
    PACKETDATA_24BITFLAGINDEX = 0xD,
    PACKETDATA_GROUNDENTITY = 0xE,
    PACKETDATA_ENTITYNUM = 0xF,
    PACKETDATA_LASTFIELDCHANGED = 0x10,
    PACKETDATA_NOTNETWORKDATA = 0x11,
    PACKETDATA_ORIGINDELTA = 0x12,
    PACKETDATA_ORIGIN = 0x13,
    NUM_PACKETDATA_MODES = 0x14,
};
enum PacketDataType : __int32
{                                       // ...
    ANALYZE_SNAPSHOT_DELTAENTITY = 0x0,
    ANALYZE_SNAPSHOT_NEWENTITY = 0x1,
    ANALYZE_SNAPSHOT_REMOVEDENTITY = 0x2,
    ANALYZE_SNAPSHOT_ALLENTITIES = 0x3,
    ANALYZE_SNAPSHOT_TEMPENTITY = 0x4,
    ANALYZE_SNAPSHOT_ALLTEMPENTITIES = 0x5,
    ANALYZE_SNAPSHOT_DELTACLIENT = 0x6,
    ANALYZE_SNAPSHOT_NEWCLIENT = 0x7,
    ANALYZE_SNAPSHOT_REMOVEDCLIENT = 0x8,
    ANALYZE_SNAPSHOT_ALLCLIENTS = 0x9,
    ANALYZE_SNAPSHOT_DELTAPLAYERSTATE = 0xA,
    ANALYZE_SNAPSHOT_NODELTAPLAYERSTATE = 0xB,
    ANALYZE_SNAPSHOT_SERVERCMDS = 0xC,
    ANALYZE_SNAPSHOT_DATATYPE_COUNT = 0xD,
};
void __cdecl SV_ClearPacketAnalysis();
void __cdecl SV_TrackETypeBytes(unsigned int eType, int bits);
void __cdecl SV_TrackPSBits(int bits);
void __cdecl SV_TrackPSFieldDeltasBits(int bits);
void __cdecl SV_TrackPSHudelemBits(int bits);
void __cdecl SV_TrackPSStatsBits(int bits);
void __cdecl SV_TrackPSAmmoBits(int bits);
void __cdecl SV_TrackPSObjectivesBits(int bits);
void __cdecl SV_TrackPSWeaponModelBits(int bits);
void __cdecl SV_TrackFieldsChanged(int lc);
void __cdecl SV_DisablePacketData();
void __cdecl SV_EnablePacketData();
void __cdecl SV_ResetPacketData(int clientNum, const msg_t *msg);
bool __cdecl SV_IsPacketDataNetworkData();
void __cdecl SV_PacketDataIsGroundEntity(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsType(int clientNum, const msg_t *msg, packetModeList mode);
const char *__cdecl SV_GetPacketDataTypeName(int dataType);
void __cdecl SV_PacketDataIsEntityNum(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsLastFieldChanged(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsUnknown(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsHeader(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsNotNetworkData(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsOverhead(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIs24BitFlagIndex(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsTimeDelta(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsTime(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsReliableData(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsData(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsZeroFloat(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsSmallFloat(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsLargeFloat(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsOriginDelta(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsOrigin(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsZeroAngle(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsSmallAngle(int clientNum, const msg_t *msg);
void __cdecl SV_PacketDataIsZeroInt(int clientNum, const msg_t *msg);
void __cdecl SV_TrackFloatCompressedBits(unsigned int bits);
void __cdecl SV_TrackOriginDeltaBits(int bits);
void __cdecl SV_TrackOriginZDeltaBits(int bits);
void __cdecl SV_TrackOriginZFullBits(int bits);
void __cdecl SV_TrackOriginFullBits(int bits);
const char *__cdecl SV_GetEntityTypeString(unsigned int packetEntityType);
void __cdecl SV_AnalyzePacketData(int clientNum, const msg_t *msg);
int __cdecl SV_TrackPacketData(
    unsigned int clientNum,
    PacketDataType datatype,
    int eType,
    int entNum,
    int bitsUsedPrev,
    const msg_t *msg);
void __cdecl SV_SetNextEntityStart(int eType, int entNum);
bool __cdecl SV_NewPacketAnalysisReady();
void __cdecl SV_TrackFieldChange(int clientNum, int entityType, unsigned int field);
void __cdecl SV_WriteEntityFieldNumbers();
void __cdecl SV_GetAnalyzeEntityFields(int analyzeEntityType, NetFieldList *stateFields, unsigned int *numFields);
int __cdecl SV_GetClientSnapshotPing(int clientNum, char snapshotNum);
void __cdecl SV_TrackSnapshotSize(int size);
void __cdecl SV_TrackPacketCompression(unsigned int clientNum, int originalSize, int compressedSize);
int __cdecl SV_GetPacketCompressionForClient(int clientNum);
void __cdecl SV_Netchan_PrintProfileStats(int bPrintToConsole);
void __cdecl SV_ProfDraw(int y, char *string, bool showHighlight);


//sv_voice_mp
void __cdecl G_BroadcastVoice(gentity_s *talker, VoicePacket_t *voicePacket);
bool __cdecl SV_ClientHasClientMuted(unsigned int listener, unsigned int talker);
bool __cdecl SV_ClientWantsVoiceData(unsigned int clientNum);
void __cdecl SV_UserVoice(client_t *cl, msg_t *msg);
void __cdecl SV_QueueVoicePacket(int talkerNum, int clientNum, VoicePacket_t *voicePacket);
void __cdecl SV_PreGameUserVoice(client_t *cl, msg_t *msg);


// ucmds
void __cdecl SV_UnmutePlayer_f(client_t *cl);
void __cdecl SV_MutePlayer_f(client_t *cl);
void __cdecl SV_WWWDownLoad_Clear(client_t *cl);
void __cdecl SV_WWWDownload_f(client_t *cl);
void __cdecl SV_RetransmitDownload_f(client_t *cl);
void __cdecl SV_DoneDownload_f(client_t *cl);
void __cdecl SV_StopDownload_f(client_t *cl);
void __cdecl SV_ResetPureClient_f(client_t *cl);
void __cdecl SV_NextDownload_f(client_t *cl);
void __cdecl SV_BeginDownload_f(client_t *cl);
void __cdecl SV_VerifyIwds_f(client_t *cl);



extern int svsHeaderValid;
extern int g_bitsSent[64][13];
extern int s_totalPacketDataSizes[20];
extern int s_packetMetaDataSize[64][20];
extern unsigned int s_packetModeStart[64];
extern packetModeList s_packetMode[64];
extern int g_currentSnapshotPerEntity[64][1024];
extern unsigned __int8 g_currentSnapshotFieldsPerEntity[64][1024];
extern unsigned __int8 g_currentSnapshotPlayerStateFields[64];
extern bool newDataReady;
extern unsigned int bitsUsedPerEType[256];
extern unsigned int bitsUsedForPlayerstates[7];
extern int playerStateFieldsChanged[161];
extern bool s_packetDataEnabled;
extern bool g_archivingSnapshot;
extern int s_floatBitsCompressed[60];
extern int s_originDeltaBits[8];
extern int s_originZDeltaBits[8];
extern int s_originZFullBits[17];
extern int s_originFullBits[17];
extern unsigned int networkEntityFieldsChanged[23][160];
extern unsigned int currentSnapshotNetworkEntityFieldsChanged[23][160];
extern unsigned int bitsUsedForServerCommands;
extern int s_currentEntType;
extern int s_currentEntNum;
extern int originsSentDueToPredicitonError;
extern int originsSentDueToServerTimeMismatch;
extern float s_stdSnapshotDeviation;
extern int s_maxSnapshotSize;
extern unsigned int huffBytesSeen[256];
extern int s_maxSnapshotSize;
extern int s_numSnapshotSamples;
extern int s_numSnapshotsBuiltSinceLastPoll;
extern int s_uncompressedDataSinceLastPoll;
extern int s_compressedDataSinceLastPoll;
extern int s_numSnapshotsSentSinceLastPoll;

extern unsigned __int8 tempServerMsgBuf[131072];