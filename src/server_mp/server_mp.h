#pragma once
#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include <qcommon/ent.h>
#include <qcommon/net_chan_mp.h>
#include <qcommon/sv_msg_write_mp.h>
#include <client_mp/client_mp.h>

enum svscmd_type : __int32
{                                       // ...
    SV_CMD_CAN_IGNORE = 0x0,
    SV_CMD_RELIABLE = 0x1,
};

#define NETF_HUD(x) NETF_BASE(hudelem_s, x)

const NetField hudElemFields[40] =
{
  { NETF_HUD(color.rgba), -85, 0u },
  { NETF_HUD(fadeStartTime), -97, 0u },
  { NETF_HUD(fromColor.rgba), -85, 0u },
  { NETF_HUD(y), -91, 0u },
  { NETF_HUD(type), 4, 0u },
  { NETF_HUD(materialIndex), 8, 0u },
  { NETF_HUD(height), 10, 0u },
  { NETF_HUD(width), 10, 0u },
  { NETF_HUD(x), -92, 0u },
  { NETF_HUD(fadeTime), 16, 0u },
  { NETF_HUD(z), -90, 0u },
  { NETF_HUD(value), 0, 0u },
  { NETF_HUD(alignScreen), 6, 0u },
  { NETF_HUD(sort), 0, 0u },
  { NETF_HUD(alignOrg), 4, 0u },
  { NETF_HUD(offscreenMaterialIdx), 8, 0u },
  { NETF_HUD(fontScale), -86, 0u },
  { NETF_HUD(text), 9, 0u },
  { NETF_HUD(font), 4, 0u },
  { NETF_HUD(scaleStartTime), -97, 0u },
  { NETF_HUD(scaleTime), 16, 0u },
  { NETF_HUD(fromWidth), 10, 0u },
  { NETF_HUD(fromHeight), 10, 0u },
  { NETF_HUD(targetEntNum), 10, 0u },
  { NETF_HUD(glowColor.rgba), -85, 0u },
  { NETF_HUD(fxBirthTime), -97, 0u },
  { NETF_HUD(soundID), 5, 0u },
  { NETF_HUD(fxLetterTime), 12, 0u },
  { NETF_HUD(fxDecayStartTime), 16, 0u },
  { NETF_HUD(fxDecayDuration), 16, 0u },
  { NETF_HUD(flags), 3, 0u },
  { NETF_HUD(label), 9, 0u },
  { NETF_HUD(time), -97, 0u },
  { NETF_HUD(moveStartTime), -97, 0u },
  { NETF_HUD(moveTime), 16, 0u },
  { NETF_HUD(fromX), -99, 0u },
  { NETF_HUD(fromY), -99, 0u },
  { NETF_HUD(fromAlignScreen), 6, 0u },
  { NETF_HUD(fromAlignOrg), 4, 0u },
  { NETF_HUD(duration), 32, 0u }
}; // idb

#define NETF(x) NETF_BASE(entityState_s, x)

const NetField vehicleEntityStateFields[59] =
{
  { NETF(eType), 8, 1u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(lerp.u.vehicle.gunYaw), 0, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(legsAnim), 10, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(un1), 8, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(index), 10, 0u },
  { NETF(lerp.u.vehicle.materialTime), -97, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.apos.trTime), 32, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(lerp.u.vehicle.gunPitch), 0, 0u },
  { NETF(solid), 24, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(un2), 32, 0u },
  { NETF(time2), -97, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(fWaistPitch), 0, 0u },
  { NETF(fTorsoPitch), 0, 0u },
  { NETF(lerp.u.vehicle.bodyPitch), -100, 0u },
  { NETF(lerp.u.vehicle.bodyRoll), -100, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(lerp.u.vehicle.steerYaw), 0, 0u },
  { NETF(lerp.apos.trDuration), 32, 0u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
};

const NetField planeStateFields[60] =
{
  { NETF(eType), 8, 1u },
  { NETF(lerp.pos.trBase[0]), 0, 2u },
  { NETF(lerp.pos.trBase[1]), 0, 2u },
  { NETF(lerp.pos.trBase[2]), 0, 2u },
  { NETF(index), 10, 2u },
  { NETF(lerp.pos.trDelta[0]), 0, 2u },
  { NETF(lerp.pos.trDelta[1]), 0, 2u },
  { NETF(lerp.pos.trTime), -97, 2u },
  { NETF(lerp.pos.trType), 8, 2u },
  { NETF(lerp.pos.trDuration), 32, 2u },
  { NETF(lerp.u.vehicle.teamAndOwnerIndex), 8, 2u },
  { NETF(lerp.apos.trBase[1]), -100, 2u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(lerp.u.anonymous.data[0]), 32, 0u },
  { NETF(time2), -97, 0u },
  { NETF(solid), 24, 0u },
  { NETF(un2), 32, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(un1), 8, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.apos.trTime), 32, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(fWaistPitch), 0, 0u },
  { NETF(fTorsoPitch), 0, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(lerp.apos.trDuration), 32, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(legsAnim), 10, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(lerp.u.anonymous.data[1]), 32, 0u },
  { NETF(lerp.u.anonymous.data[2]), 32, 0u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.u.anonymous.data[4]), 32, 1u },
  { NETF(lerp.u.anonymous.data[5]), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
};


const NetField helicopterEntityStateFields[58] =
{
  { NETF(eType), 8, 1u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.u.vehicle.gunPitch), 0, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(lerp.u.vehicle.gunYaw), 0, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(un1.helicopterStage), 3, 0u },
  { NETF(time2), -97, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(un2), 32, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(lerp.u.vehicle.teamAndOwnerIndex), 8, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(legsAnim), 10, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(index), 10, 0u },
  { NETF(lerp.u.vehicle.materialTime), -97, 0u },
  { NETF(lerp.apos.trTime), 32, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(solid), 24, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(fWaistPitch), 0, 0u },
  { NETF(fTorsoPitch), 0, 0u },
  { NETF(lerp.u.vehicle.bodyPitch), -100, 0u },
  { NETF(lerp.u.vehicle.bodyRoll), -100, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(lerp.apos.trDuration), 32, 0u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
};

const NetField entityStateFields[59] =
{
  { NETF(eType), 8, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(lerp.u.anonymous.data[0]), 32, 0u },
  { NETF(time2), -97, 0u },
  { NETF(index), 10, 0u },
  { NETF(solid), 24, 0u },
  { NETF(un2), 32, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(un1), 8, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.apos.trTime), 32, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(fWaistPitch), 0, 0u },
  { NETF(fTorsoPitch), 0, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(lerp.apos.trDuration), 32, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(legsAnim), 10, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(lerp.u.anonymous.data[1]), 32, 0u },
  { NETF(lerp.u.anonymous.data[2]), 32, 0u },
  { NETF(lerp.u.anonymous.data[3]), 32, 0u },
  { NETF(lerp.u.anonymous.data[4]), 32, 0u },
  { NETF(lerp.u.anonymous.data[5]), 32, 0u },
  { NETF(partBits[0]), 32, 0u },
  { NETF(partBits[1]), 32, 0u },
  { NETF(partBits[2]), 32, 0u },
  { NETF(partBits[3]), 32, 0u }
}; // idb

#define NETF_CL(x) NETF_BASE(clientState_s, x)
const NetField clientStateFields[24] = // LWSS: edit SV_GetAnalyzeEntityFields() if you change this
{
  { NETF_CL(modelindex), 9, 0u },
  { NETF_CL(name[0]), 32, 0u },
  { NETF_CL(rank), 8, 0u },
  { NETF_CL(prestige), 8, 0u },
  { NETF_CL(team), 2, 0u },
  { NETF_CL(attachedVehEntNum), 10, 0u },
  { NETF_CL(name[4]), 32, 0u },
  { NETF_CL(attachModelIndex[0]), 9, 0u },
  { NETF_CL(name[8]), 32, 0u },
  { NETF_CL(perks), 32, 0u },
  { NETF_CL(name[12]), 32, 0u },
  { NETF_CL(attachModelIndex[1]), 9, 0u },
  { NETF_CL(maxSprintTimeMultiplier), 0, 0u },
  { NETF_CL(attachedVehSlotIndex), 2, 0u },
  { NETF_CL(attachTagIndex[5]), 5, 0u },
  { NETF_CL(attachTagIndex[0]), 5, 0u },
  { NETF_CL(attachTagIndex[1]), 5, 0u },
  { NETF_CL(attachTagIndex[2]), 5, 0u },
  { NETF_CL(attachTagIndex[3]), 5, 0u },
  { NETF_CL(attachTagIndex[4]), 5, 0u },
  { NETF_CL(attachModelIndex[2]), 9, 0u },
  { NETF_CL(attachModelIndex[3]), 9, 0u },
  { NETF_CL(attachModelIndex[4]), 9, 0u },
  { NETF_CL(attachModelIndex[5]), 9, 0u }
}; // idb

#define NETF_PL(x) NETF_BASE(playerState_s, x)
const NetField playerStateFields[141] = // LWSS: edit SV_GetAnalyzeEntityFields() if you change this
{
  { NETF_PL(commandTime), -97, 0u },
  { NETF_PL(viewangles[1]), -87, 0u },
  { NETF_PL(viewangles[0]), -87, 0u },
  { NETF_PL(viewangles[2]), -87, 0u },
  { NETF_PL(origin[0]), -88, 3u },
  { NETF_PL(origin[1]), -88, 3u },
  { NETF_PL(bobCycle), 8, 3u },
  { NETF_PL(velocity[1]), -88, 3u },
  { NETF_PL(velocity[0]), -88, 3u },
  { NETF_PL(movementDir), -8, 3u },
  { NETF_PL(eventSequence), 8, 0u },
  { NETF_PL(legsAnim), 10, 0u },
  { NETF_PL(origin[2]), -88, 3u },
  { NETF_PL(weaponTime), -16, 0u },
  { NETF_PL(aimSpreadScale), -88, 0u },
  { NETF_PL(torsoTimer), 16, 0u },
  { NETF_PL(pm_flags), 21, 0u },
  { NETF_PL(weapAnim), 10, 0u },
  { NETF_PL(weaponstate), 5, 0u },
  { NETF_PL(velocity[2]), -88, 3u },
  { NETF_PL(events[0]), 8, 0u },
  { NETF_PL(events[1]), 8, 0u },
  { NETF_PL(events[2]), 8, 0u },
  { NETF_PL(events[3]), 8, 0u },
  { NETF_PL(eventParms[0]), 8, 0u },
  { NETF_PL(eventParms[1]), 8, 0u },
  { NETF_PL(eventParms[2]), 8, 0u },
  { NETF_PL(eventParms[3]), 8, 0u },
  { NETF_PL(torsoAnim), 10, 0u },
  { NETF_PL(holdBreathScale), -88, 0u },
  { NETF_PL(eFlags), -98, 0u },
  { NETF_PL(viewHeightCurrent), -88, 0u },
  { NETF_PL(fWeaponPosFrac), -88, 0u },
  { NETF_PL(legsTimer), 16, 0u },
  { NETF_PL(viewHeightTarget), -8, 0u },
  { NETF_PL(sprintState.lastSprintStart), -97, 0u },
  { NETF_PL(sprintState.lastSprintEnd), -97, 0u },
  { NETF_PL(weapon), 7, 0u },
  { NETF_PL(weaponDelay), -16, 0u },
  { NETF_PL(sprintState.sprintStartMaxLength), 14, 0u },
  { NETF_PL(weapFlags), 9, 0u },
  { NETF_PL(groundEntityNum), 10, 0u },
  { NETF_PL(damageTimer), 10, 0u },
  { NETF_PL(weapons[0]), 32, 0u },
  { NETF_PL(weapons[1]), 32, 0u },
  { NETF_PL(weaponold[0]), 32, 0u },
  { NETF_PL(delta_angles[1]), -100, 0u },
  { NETF_PL(offHandIndex), 7, 0u },
  { NETF_PL(pm_time), -16, 0u },
  { NETF_PL(otherFlags), 5, 0u },
  { NETF_PL(moveSpeedScaleMultiplier), 0, 0u },
  { NETF_PL(perks), 32, 0u },
  { NETF_PL(killCamEntity), 10, 0u },
  { NETF_PL(throwBackGrenadeOwner), 10, 0u },
  { NETF_PL(actionSlotType[2]), 2, 0u },
  { NETF_PL(delta_angles[0]), -100, 0u },
  { NETF_PL(speed), 16, 0u },
  { NETF_PL(viewlocked_entNum), 16, 0u },
  { NETF_PL(gravity), 16, 0u },
  { NETF_PL(actionSlotType[0]), 2, 0u },
  { NETF_PL(dofNearBlur), 0, 0u },
  { NETF_PL(dofFarBlur), 0, 0u },
  { NETF_PL(clientNum), 8, 0u },
  { NETF_PL(damageEvent), 8, 0u },
  { NETF_PL(viewHeightLerpTarget), -8, 0u },
  { NETF_PL(damageYaw), 8, 0u },
  { NETF_PL(viewmodelIndex), 9, 0u },
  { NETF_PL(damageDuration), 16, 0u },
  { NETF_PL(damagePitch), 8, 0u },
  { NETF_PL(flinchYawAnim), 2, 0u },
  { NETF_PL(weaponShotCount), 3, 0u },
  { NETF_PL(viewHeightLerpDown), 1, 2u },
  { NETF_PL(cursorHint), 8, 0u },
  { NETF_PL(cursorHintString), -8, 0u },
  { NETF_PL(cursorHintEntIndex), 10, 0u },
  { NETF_PL(viewHeightLerpTime), 32, 0u },
  { NETF_PL(offhandSecondary), 1, 2u },
  { NETF_PL(radarEnabled), 1, 2u },
  { NETF_PL(pm_type), 8, 0u },
  { NETF_PL(fTorsoPitch), 0, 0u },
  { NETF_PL(holdBreathTimer), 16, 0u },
  { NETF_PL(actionSlotParam[2]), 7, 0u },
  { NETF_PL(jumpTime), 32, 0u },
  { NETF_PL(mantleState.flags), 5, 0u },
  { NETF_PL(fWaistPitch), 0, 0u },
  { NETF_PL(grenadeTimeLeft), -16, 0u },
  { NETF_PL(proneDirection), 0, 0u },
  { NETF_PL(mantleState.timer), 32, 0u },
  { NETF_PL(damageCount), 7, 0u },
  { NETF_PL(shellshockTime), -97, 0u },
  { NETF_PL(shellshockDuration), 16, 2u },
  { NETF_PL(sprintState.sprintButtonUpRequired), 1, 2u },
  { NETF_PL(shellshockIndex), 4, 0u },
  { NETF_PL(proneTorsoPitch), 0, 0u },
  { NETF_PL(sprintState.sprintDelay), 1, 2u },
  { NETF_PL(actionSlotParam[3]), 7, 0u },
  { NETF_PL(weapons[3]), 32, 0u },
  { NETF_PL(actionSlotType[3]), 2, 0u },
  { NETF_PL(proneDirectionPitch), 0, 0u },
  { NETF_PL(jumpOriginZ), 0, 0u },
  { NETF_PL(mantleState.yaw), 0, 0u },
  { NETF_PL(mantleState.transIndex), 4, 0u },
  { NETF_PL(weaponrechamber[0]), 32, 0u },
  { NETF_PL(throwBackGrenadeTimeLeft), -16, 0u },
  { NETF_PL(weaponold[3]), 32, 0u },
  { NETF_PL(weaponold[1]), 32, 0u },
  { NETF_PL(foliageSoundTime), -97, 0u },
  { NETF_PL(vLadderVec[0]), 0, 0u },
  { NETF_PL(viewlocked), 2, 0u },
  { NETF_PL(deltaTime), 32, 0u },
  { NETF_PL(viewAngleClampRange[1]), 0, 0u },
  { NETF_PL(viewAngleClampBase[1]), 0, 0u },
  { NETF_PL(viewAngleClampRange[0]), 0, 0u },
  { NETF_PL(vLadderVec[1]), 0, 0u },
  { NETF_PL(locationSelectionInfo), 8, 0u },
  { NETF_PL(meleeChargeTime), -97, 0u },
  { NETF_PL(meleeChargeYaw), -100, 0u },
  { NETF_PL(meleeChargeDist), 8, 0u },
  { NETF_PL(iCompassPlayerInfo), 32, 0u },
  { NETF_PL(weapons[2]), 32, 0u },
  { NETF_PL(actionSlotType[1]), 2, 0u },
  { NETF_PL(weaponold[2]), 32, 0u },
  { NETF_PL(vLadderVec[2]), 0, 0u },
  { NETF_PL(weaponRestrictKickTime), -16, 0u },
  { NETF_PL(delta_angles[2]), -100, 0u },
  { NETF_PL(spreadOverride), 6, 0u },
  { NETF_PL(spreadOverrideState), 2, 0u },
  { NETF_PL(actionSlotParam[0]), 7, 0u },
  { NETF_PL(actionSlotParam[1]), 7, 0u },
  { NETF_PL(dofNearStart), 0, 0u },
  { NETF_PL(dofNearEnd), 0, 0u },
  { NETF_PL(dofFarStart), 0, 0u },
  { NETF_PL(dofFarEnd), 0, 0u },
  { NETF_PL(dofViewmodelStart), 0, 0u },
  { NETF_PL(dofViewmodelEnd), 0, 0u },
  { NETF_PL(viewAngleClampBase[0]), 0, 0u },
  { NETF_PL(weaponrechamber[1]), 32, 0u },
  { NETF_PL(weaponrechamber[2]), 32, 0u },
  { NETF_PL(weaponrechamber[3]), 32, 0u },
  { NETF_PL(leanf), 0, 0u },
  { NETF_PL(adsDelayTime), 32, 1u }
};

#define NETF_ARC(x) NETF_BASE(archivedEntity_s, s.x)
#define NETF_ARC_A(x) NETF_BASE(archivedEntity_s, r.x)
const NetField archivedEntityFields[69] = // LWSS: change SV_GetAnalyzeEntityFields() if you edit this
{
  { NETF_ARC_A(absmin[1]), 0, 0u },
  { NETF_ARC_A(absmax[1]), 0, 0u },
  { NETF_ARC_A(absmin[0]), 0, 0u },
  { NETF_ARC_A(absmax[0]), 0, 0u },
  { NETF_ARC(lerp.pos.trBase[1]), 0, 0u },
  { NETF_ARC(lerp.pos.trBase[0]), 0, 0u },
  { NETF_ARC_A(absmax[2]), 0, 0u },
  { NETF_ARC(lerp.pos.trBase[2]), 0, 0u },
  { NETF_ARC_A(absmin[2]), 0, 0u },
  { NETF_ARC(groundEntityNum), 10, 0u },
  { NETF_ARC(lerp.apos.trBase[1]), 0, 0u },
  { NETF_ARC(eType), 8, 0u },
  { NETF_ARC(lerp.apos.trBase[0]), 0, 0u },
  { NETF_ARC(clientNum), 7, 0u },
  { NETF_ARC(lerp.apos.trBase[2]), 0, 0u },
  { NETF_ARC(lerp.eFlags), -98, 0u },
  { NETF_ARC_A(svFlags), 32, 0u },
  { NETF_ARC(events[0]), 8, 0u },
  { NETF_ARC(eventSequence), 8, 0u },
  { NETF_ARC(index), 10, 0u },
  { NETF_ARC(legsAnim), 10, 0u },
  { NETF_ARC(events[1]), -94, 0u },
  { NETF_ARC(events[2]), -94, 0u },
  { NETF_ARC(events[3]), -94, 0u },
  { NETF_ARC(weapon), 7, 0u },
  { NETF_ARC(weaponModel), 4, 0u },
  { NETF_ARC(lerp.pos.trType), 8, 0u },
  { NETF_ARC(lerp.apos.trType), 8, 0u },
  { NETF_ARC(iHeadIcon), 4, 0u },
  { NETF_ARC(iHeadIconTeam), 2, 0u },
  { NETF_ARC(solid), 24, 0u },
  { NETF_ARC(eventParms[0]), -93, 0u },
  { NETF_ARC(torsoAnim), 10, 0u },
  { NETF_ARC(lerp.u.anonymous.data[1]), 32, 0u },
  { NETF_ARC(lerp.pos.trTime), -97, 0u },
  { NETF_ARC(lerp.pos.trDelta[0]), 0, 0u },
  { NETF_ARC(lerp.pos.trDelta[1]), 0, 0u },
  { NETF_ARC(lerp.pos.trDelta[2]), 0, 0u },
  { NETF_ARC(otherEntityNum), 10, 0u },
  { NETF_ARC(eventParms[1]), -93, 0u },
  { NETF_ARC(surfType), 8, 0u },
  { NETF_ARC(eventParm), -93, 0u },
  { NETF_ARC(eventParms[2]), -93, 0u },
  { NETF_ARC(un1), 8, 0u },
  { NETF_ARC(eventParms[3]), 8, 0u },
  { NETF_ARC(lerp.pos.trDuration), 32, 0u },
  { NETF_ARC(fWaistPitch), 0, 0u },
  { NETF_ARC(fTorsoPitch), 0, 0u },
  { NETF_ARC(lerp.apos.trTime), -97, 0u },
  { NETF_ARC(lerp.apos.trDelta[0]), 0, 0u },
  { NETF_ARC(lerp.apos.trDelta[2]), 0, 0u },
  { NETF_ARC_A(clientMask[0]), 32, 0u },
  { NETF_ARC_A(clientMask[1]), 32, 0u },
  { NETF_ARC(lerp.apos.trDelta[1]), 0, 0u },
  { NETF_ARC(lerp.u.anonymous.data[0]), 32, 0u },
  { NETF_ARC(attackerEntityNum), 10, 0u },
  { NETF_ARC(time2), -97, 0u },
  { NETF_ARC(lerp.u.anonymous.data[2]), 32, 0u },
  { NETF_ARC(un2), 32, 0u },
  { NETF_ARC(lerp.apos.trDuration), 32, 0u },
  { NETF_ARC(loopSound), 8, 0u },
  { NETF_ARC(lerp.u.anonymous.data[3]), 32, 0u },
  { NETF_ARC(lerp.u.anonymous.data[4]), 32, 0u },
  { NETF_ARC(lerp.u.anonymous.data[5]), 32, 0u },
  { NETF_ARC(lerp.u.anonymous.data[6]), 32, 0u },
  { NETF_ARC(partBits[0]), 32, 0u },
  { NETF_ARC(partBits[1]), 32, 0u },
  { NETF_ARC(partBits[2]), 32, 0u },
  { NETF_ARC(partBits[3]), 32, 0u }
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

#define MAX_CONFIGSTRINGS 2442

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
    unsigned __int16 configstrings[MAX_CONFIGSTRINGS]; // ...
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