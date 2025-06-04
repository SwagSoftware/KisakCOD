#include "sv_msg_write_mp.h"

#include "qcommon.h"
#include "mem_track.h"
#include "huffman.h"

#include "net_chan_mp.h"
#include <server_mp/server.h>

netFieldOrderInfo_t orderInfo;

huffman_t msgHuff;

#define	NETF(s) NETF_BASE(entityState_s, s)

const NetField eventEntityStateFields[59] =
{
  { NETF(eType), 8, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(un1), 8, 0u },
  { NETF(lerp.u.anonymous.data[0]), 32, 0u },
  { NETF(lerp.u.anonymous.data[1]), 32, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(lerp.u.anonymous.data[2]), 32, 0u },
  { NETF(index), 10, 0u },
  { NETF(solid), 24, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(lerp.u.anonymous.data[4]), 32, 0u },
  { NETF(lerp.u.anonymous.data[5]), 32, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(lerp.apos.trTime), 32, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(time2), -97, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(un2), 32, 0u },
  { NETF(torsoAnim), 10, 1u },
  { NETF(legsAnim), 10, 1u },
  { NETF(fWaistPitch), 0, 1u },
  { NETF(fTorsoPitch), 0, 1u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.apos.trDuration), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
}; // idb

const NetField playerEntityStateFields[59] =
{
  { NETF(eType), 8, 1u },
  { NETF(lerp.pos.trBase[0]), -92, 2u },
  { NETF(lerp.pos.trBase[1]), -91, 2u },
  { NETF(lerp.u.player.movementDir), -8, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(legsAnim), 10, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(fTorsoPitch), 0, 0u },
  { NETF(fWaistPitch), 0, 0u },
  { NETF(solid), 24, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(lerp.u.player.leanf), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 1u },
  { NETF(lerp.pos.trDelta[0]), 0, 1u },
  { NETF(lerp.pos.trDuration), 32, 1u },
  { NETF(lerp.pos.trTime), -97, 1u },
  { NETF(lerp.pos.trDelta[2]), 0, 1u },
  { NETF(surfType), 8, 1u },
  { NETF(un1), 8, 1u },
  { NETF(index), 10, 1u },
  { NETF(lerp.apos.trDelta[0]), 0, 1u },
  { NETF(lerp.apos.trDelta[1]), 0, 1u },
  { NETF(lerp.apos.trDelta[2]), 0, 1u },
  { NETF(time2), -97, 1u },
  { NETF(loopSound), 8, 1u },
  { NETF(attackerEntityNum), 10, 1u },
  { NETF(lerp.apos.trTime), 32, 1u },
  { NETF(lerp.apos.trDuration), 32, 1u },
  { NETF(lerp.u.anonymous.data[2]), 32, 1u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.u.anonymous.data[4]), 32, 1u },
  { NETF(lerp.u.anonymous.data[5]), 32, 1u },
  { NETF(un2), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
}; // idb

const NetField corpseEntityStateFields[59] =
{
  { NETF(eType), 8, 1u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(legsAnim), 10, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(lerp.u.player.movementDir), -8, 1u },
  { NETF(eventSequence), 8, 1u },
  { NETF(events[0]), -94, 1u },
  { NETF(events[1]), -94, 1u },
  { NETF(events[2]), -94, 1u },
  { NETF(events[3]), -94, 1u },
  { NETF(fTorsoPitch), 0, 1u },
  { NETF(eventParms[1]), -93, 1u },
  { NETF(eventParms[0]), -93, 1u },
  { NETF(eventParms[2]), -93, 1u },
  { NETF(weapon), 7, 1u },
  { NETF(weaponModel), 4, 1u },
  { NETF(eventParms[3]), -93, 1u },
  { NETF(solid), 24, 1u },
  { NETF(lerp.pos.trDuration), 32, 1u },
  { NETF(fWaistPitch), 0, 1u },
  { NETF(eventParm), -93, 1u },
  { NETF(iHeadIcon), 4, 1u },
  { NETF(iHeadIconTeam), 2, 1u },
  { NETF(surfType), 8, 1u },
  { NETF(un1), 8, 1u },
  { NETF(otherEntityNum), 10, 1u },
  { NETF(index), 10, 1u },
  { NETF(lerp.apos.trDelta[0]), 0, 1u },
  { NETF(lerp.apos.trDelta[1]), 0, 1u },
  { NETF(lerp.apos.trDelta[2]), 0, 1u },
  { NETF(time2), -97, 1u },
  { NETF(loopSound), 8, 1u },
  { NETF(attackerEntityNum), 10, 1u },
  { NETF(lerp.apos.trTime), 32, 1u },
  { NETF(lerp.u.player.leanf), 0, 1u },
  { NETF(lerp.apos.trDuration), 32, 1u },
  { NETF(un2), 32, 1u },
  { NETF(lerp.u.anonymous.data[2]), 32, 1u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.u.anonymous.data[4]), 32, 1u },
  { NETF(lerp.u.anonymous.data[5]), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
}; // idb
const NetField itemEntityStateFields[59] =
{
  { NETF(eType), 8, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(lerp.pos.trType), 8, 2u },
  { NETF(lerp.pos.trDelta[2]), 0, 2u },
  { NETF(lerp.pos.trDelta[0]), 0, 2u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(clientNum), 7, 2u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.apos.trType), 8, 2u },
  { NETF(lerp.apos.trTime), -97, 2u },
  { NETF(lerp.apos.trDelta[0]), 0, 2u },
  { NETF(lerp.apos.trDelta[1]), 0, 2u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(index), 10, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(solid), 24, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(lerp.u.anonymous.data[0]), 32, 0u },
  { NETF(lerp.u.anonymous.data[1]), 32, 0u },
  { NETF(lerp.u.anonymous.data[2]), 32, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(un1), 8, 0u },
  { NETF(time2), -97, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(un2), 32, 0u },
  { NETF(legsAnim), 10, 1u },
  { NETF(torsoAnim), 10, 1u },
  { NETF(fTorsoPitch), 0, 1u },
  { NETF(fWaistPitch), 0, 1u },
  { NETF(iHeadIcon), 4, 1u },
  { NETF(iHeadIconTeam), 2, 1u },
  { NETF(lerp.apos.trDuration), 32, 1u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.u.anonymous.data[4]), 32, 1u },
  { NETF(lerp.u.anonymous.data[5]), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
}; // idb
const NetField soundBlendEntityStateFields[59] =
{
  { NETF(eType), 8, 1u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
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
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.apos.trTime), 32, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(solid), 24, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(un2), 32, 0u },
  { NETF(time2), -97, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(fWaistPitch), 0, 0u },
  { NETF(fTorsoPitch), 0, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(lerp.u.soundBlend.lerp), 0, 0u },
  { NETF(lerp.apos.trDuration), 32, 0u },
  { NETF(lerp.u.anonymous.data[1]), 32, 1u },
  { NETF(lerp.u.anonymous.data[2]), 32, 1u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.u.anonymous.data[4]), 32, 1u },
  { NETF(lerp.u.anonymous.data[5]), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
};
const NetField scriptMoverStateFields[59] =
{
  { NETF(eType), 8, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(index), 10, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(partBits[0]), 32, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(lerp.apos.trTime), 32, 0u },
  { NETF(partBits[1]), 32, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(lerp.apos.trDuration), 32, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(solid), 24, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(lerp.u.anonymous.data[0]), 32, 0u },
  { NETF(lerp.u.anonymous.data[1]), 32, 0u },
  { NETF(lerp.u.anonymous.data[2]), 32, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(time2), -97, 0u },
  { NETF(un2), 32, 0u },
  { NETF(un1), 8, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(fWaistPitch), 0, 0u },
  { NETF(fTorsoPitch), 0, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(legsAnim), 10, 0u },
  { NETF(lerp.u.anonymous.data[3]), 32, 0u },
  { NETF(lerp.u.anonymous.data[4]), 32, 0u },
  { NETF(lerp.u.anonymous.data[5]), 32, 0u },
  { NETF(partBits[2]), 32, 0u },
  { NETF(partBits[3]), 32, 0u }
}; // idb
const NetField loopFxEntityStateFields[59] =
{
  { NETF(eType), 8, 1u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(lerp.u.loopFx.cullDist), 0, 0u },
  { NETF(lerp.u.loopFx.period), 32, 0u },
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
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.apos.trTime), 32, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(torsoAnim), 10, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(solid), 24, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(un2), 32, 0u },
  { NETF(time2), -97, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(fWaistPitch), 0, 0u },
  { NETF(fTorsoPitch), 0, 0u },
  { NETF(iHeadIcon), 4, 0u },
  { NETF(iHeadIconTeam), 2, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(lerp.apos.trDuration), 32, 0u },
  { NETF(lerp.u.anonymous.data[2]), 32, 1u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.u.anonymous.data[4]), 32, 1u },
  { NETF(lerp.u.anonymous.data[5]), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
};

const NetField fxStateFields[59] =
{
  { NETF(eType), 8, 0u },
  { NETF(time2), -97, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(un1), 8, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(lerp.apos.trTime), -97, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(index), 10, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(solid), 24, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(lerp.u.anonymous.data[0]), 32, 0u },
  { NETF(lerp.u.anonymous.data[1]), 32, 0u },
  { NETF(lerp.u.anonymous.data[2]), 32, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(un2), 32, 0u },
  { NETF(legsAnim), 10, 1u },
  { NETF(torsoAnim), 10, 1u },
  { NETF(fTorsoPitch), 0, 1u },
  { NETF(fWaistPitch), 0, 1u },
  { NETF(iHeadIcon), 4, 1u },
  { NETF(iHeadIconTeam), 2, 1u },
  { NETF(lerp.apos.trDuration), 32, 1u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.u.anonymous.data[4]), 32, 1u },
  { NETF(lerp.u.anonymous.data[5]), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
}; // idb
const NetField missileEntityStateFields[59] =
{
  { NETF(eType), 8, 0u },
  { NETF(lerp.pos.trBase[1]), -91, 0u },
  { NETF(lerp.pos.trBase[0]), -92, 0u },
  { NETF(lerp.pos.trDelta[0]), 0, 0u },
  { NETF(lerp.pos.trDelta[1]), 0, 0u },
  { NETF(lerp.pos.trDelta[2]), 0, 0u },
  { NETF(lerp.apos.trBase[2]), -100, 0u },
  { NETF(lerp.apos.trBase[0]), -100, 0u },
  { NETF(lerp.pos.trBase[2]), -90, 0u },
  { NETF(lerp.pos.trTime), -97, 0u },
  { NETF(lerp.apos.trTime), -97, 0u },
  { NETF(lerp.apos.trDelta[0]), 0, 0u },
  { NETF(eventSequence), 8, 0u },
  { NETF(groundEntityNum), -96, 0u },
  { NETF(lerp.apos.trBase[1]), -100, 0u },
  { NETF(lerp.pos.trType), 8, 0u },
  { NETF(lerp.apos.trType), 8, 0u },
  { NETF(lerp.eFlags), -98, 0u },
  { NETF(surfType), 8, 0u },
  { NETF(lerp.apos.trDelta[2]), 0, 0u },
  { NETF(lerp.u.missile.launchTime), -97, 0u },
  { NETF(weapon), 7, 0u },
  { NETF(eventParms[0]), -93, 0u },
  { NETF(events[0]), -94, 0u },
  { NETF(events[1]), -94, 0u },
  { NETF(index), 10, 0u },
  { NETF(clientNum), 7, 0u },
  { NETF(eventParms[1]), -93, 0u },
  { NETF(events[2]), -94, 0u },
  { NETF(eventParms[2]), -93, 0u },
  { NETF(un2), 1, 0u },
  { NETF(events[3]), -94, 0u },
  { NETF(eventParms[3]), -93, 0u },
  { NETF(weaponModel), 4, 0u },
  { NETF(eventParm), -93, 0u },
  { NETF(un1), 8, 0u },
  { NETF(otherEntityNum), 10, 0u },
  { NETF(lerp.apos.trDelta[1]), 0, 0u },
  { NETF(attackerEntityNum), 10, 0u },
  { NETF(lerp.pos.trDuration), 32, 0u },
  { NETF(loopSound), 8, 0u },
  { NETF(lerp.u.anonymous.data[1]), 32, 0u },
  { NETF(lerp.u.anonymous.data[2]), 32, 0u },
  { NETF(solid), 24, 0u },
  { NETF(time2), -97, 0u },
  { NETF(legsAnim), 10, 1u },
  { NETF(torsoAnim), 10, 1u },
  { NETF(fTorsoPitch), 0, 1u },
  { NETF(fWaistPitch), 0, 1u },
  { NETF(iHeadIcon), 4, 1u },
  { NETF(iHeadIconTeam), 2, 1u },
  { NETF(lerp.apos.trDuration), 32, 1u },
  { NETF(lerp.u.anonymous.data[3]), 32, 1u },
  { NETF(lerp.u.anonymous.data[4]), 32, 1u },
  { NETF(lerp.u.anonymous.data[5]), 32, 1u },
  { NETF(partBits[0]), 32, 1u },
  { NETF(partBits[1]), 32, 1u },
  { NETF(partBits[2]), 32, 1u },
  { NETF(partBits[3]), 32, 1u }
}; // idb

#define NETF_OBJ(x) NETF_BASE(objective_t, x)

static const NetField objectiveFields[6] =
{
  { NETF_OBJ(origin[0]), 0, 0u },
  { NETF_OBJ(origin[1]), 0, 0u },
  { NETF_OBJ(origin[2]), 0, 0u },
  { NETF_OBJ(icon), 12, 0u },
  { NETF_OBJ(entNum), 10, 0u },
  { NETF_OBJ(teamNum), 4, 0u }
}; // idb

const int msg_hData[256] =
{
  274054,
  68777,
  40460,
  40266,
  48059,
  39006,
  48630,
  27692,
  17712,
  15439,
  12386,
  10758,
  9420,
  9979,
  9346,
  15256,
  13184,
  14319,
  7750,
  7221,
  6095,
  5666,
  12606,
  7263,
  7322,
  5807,
  11628,
  6199,
  7826,
  6349,
  7698,
  9656,
  28968,
  5164,
  13629,
  6058,
  4745,
  4519,
  5199,
  4807,
  5323,
  3433,
  3455,
  3563,
  6979,
  5229,
  5002,
  4423,
  14108,
  13631,
  11908,
  11801,
  10261,
  7635,
  7215,
  7218,
  9353,
  6161,
  5689,
  4649,
  5026,
  5866,
  8002,
  10534,
  15381,
  8874,
  11798,
  7199,
  12814,
  6103,
  4982,
  5972,
  6779,
  4929,
  5333,
  3503,
  4345,
  6098,
  14117,
  16440,
  6446,
  3062,
  4695,
  3085,
  4198,
  4013,
  3878,
  3414,
  5514,
  4092,
  3261,
  4740,
  4544,
  3127,
  3385,
  7688,
  11126,
  6417,
  5297,
  4529,
  6333,
  4210,
  7056,
  4658,
  6190,
  3512,
  2843,
  3479,
  9369,
  5203,
  4980,
  5881,
  7509,
  4292,
  6097,
  5492,
  4648,
  2996,
  4988,
  4163,
  6534,
  4001,
  4342,
  4488,
  6039,
  4827,
  7112,
  8654,
  26712,
  8688,
  9677,
  9368,
  7209,
  3399,
  4473,
  4677,
  11087,
  4094,
  3404,
  4176,
  6733,
  3702,
  11420,
  4867,
  5968,
  3475,
  3722,
  3560,
  4571,
  2720,
  3189,
  3099,
  4595,
  4044,
  4402,
  3889,
  4989,
  3186,
  3153,
  5387,
  8020,
  3322,
  3775,
  2886,
  4191,
  2879,
  3110,
  2576,
  3693,
  2436,
  4935,
  3017,
  3538,
  5688,
  3444,
  3410,
  9170,
  4708,
  3425,
  3273,
  3684,
  4564,
  6957,
  4817,
  5224,
  3285,
  3143,
  4227,
  5630,
  6053,
  5851,
  6507,
  13692,
  8270,
  8260,
  5583,
  7568,
  4082,
  3984,
  4574,
  6440,
  3533,
  2992,
  2708,
  5190,
  3889,
  3799,
  4582,
  6020,
  3464,
  4431,
  3495,
  2906,
  2243,
  3856,
  3321,
  8759,
  3928,
  2905,
  3875,
  4382,
  3885,
  5869,
  6235,
  10685,
  4433,
  4639,
  4305,
  4683,
  2849,
  3379,
  4683,
  5477,
  4127,
  3853,
  3515,
  4913,
  3601,
  5237,
  6617,
  9019,
  4857,
  4112,
  5180,
  5998,
  4925,
  4986,
  6365,
  7930,
  5948,
  8085,
  7732,
  8643,
  8901,
  9653,
  32647
}; // idb

const NetFieldList s_netFieldList[18] =
{
  { &entityStateFields[0], 59u},
  { &playerEntityStateFields[0], 59u },
  { &corpseEntityStateFields[0], 59u },
  { &itemEntityStateFields[0], 59u },
  { &missileEntityStateFields[0], 59u },
  { &entityStateFields[0], 59u },
  { &scriptMoverStateFields[0], 59u },
  { &soundBlendEntityStateFields[0], 59u },
  { &fxStateFields[0], 59u },
  { &loopFxEntityStateFields[0], 59u },
  { &entityStateFields[0], 59u },
  { &entityStateFields[0], 59u },
  { &helicopterEntityStateFields[0], 58u },
  { &planeStateFields[0], 60u },
  { &vehicleEntityStateFields[0], 59u },
  { &entityStateFields[0], 59u },
  { &entityStateFields[0], 59u },
  { &eventEntityStateFields[0], 59u }
};

void __cdecl TRACK_msg()
{
    track_static_alloc_internal(&msgHuff, 19476, "msgHuff", 9);
    //track_static_alloc_internal(kbitmask, 132, "kbitmask", 9);
    track_static_alloc_internal((void *)entityStateFields, 944, "entityStateFields", 9);
    track_static_alloc_internal((void *)eventEntityStateFields, 944, "eventEntityStateFields", 9);
    track_static_alloc_internal((void *)playerEntityStateFields, 944, "playerEntityStateFields", 9);
    track_static_alloc_internal((void *)corpseEntityStateFields, 944, "corpseEntityStateFields", 9);
    track_static_alloc_internal((void *)missileEntityStateFields, 944, "missileEntityStateFields", 9);
    track_static_alloc_internal((void *)itemEntityStateFields, 944, "itemEntityStateFields", 9);
    track_static_alloc_internal((void *)playerStateFields, 2256, "playerStateFields", 9);
    track_static_alloc_internal((void *)fxStateFields, 944, "fxStateFields", 9);
    track_static_alloc_internal((void *)scriptMoverStateFields, 944, "scriptMoverStateFields", 9);
    track_static_alloc_internal((void *)msg_hData, 1024, "msg_hData", 9);
}

const NetFieldList *__cdecl MSG_GetStateFieldListForEntityType(int eType)
{
    int v2; // [esp+0h] [ebp-4h]

    if (eType > 17)
        v2 = 17;
    else
        v2 = eType;
    return &s_netFieldList[v2];
}

void __cdecl MSG_WriteReliableCommandToBuffer(const char *pszCommand, char *pszBuffer, int iBufferSize)
{
    signed int v3; // ecx
    char *v4; // [esp+10h] [ebp-Ch]
    int iCommandLength; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    v3 = strlen(pszCommand);
    iCommandLength = v3;
    if (v3 >= iBufferSize)
        Com_PrintWarning(
            16,
            "WARNING: Reliable command is too long (%i/%i) and will be truncated: '%s'\n",
            v3,
            iBufferSize,
            pszCommand);
    if (!iCommandLength)
        Com_PrintWarning(16, "WARNING: Empty reliable command\n");
    v4 = pszBuffer;
    for (i = 0; i < iBufferSize && pszCommand[i]; ++i)
    {
        *v4 = I_CleanChar(pszCommand[i]);
        if (*v4 == 37)
            *v4 = 46;
        ++v4;
    }
    if (i >= iBufferSize)
        pszBuffer[iBufferSize - 1] = 0;
    else
        pszBuffer[i] = 0;
}

void __cdecl MSG_WriteEntityIndex(SnapshotInfo_s *snapInfo, msg_t *msg, int index, int indexBits)
{
    const char *v4; // eax

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 419, 0, "%s", "!msg->readOnly");
    if (msg_printEntityNums->current.enabled && SV_IsPacketDataNetworkData())
        Com_Printf(15, "Writing entity num %i\n", index);
    if (index - msg->lastEntityRef <= 0)
    {
        v4 = va("lastEntityReferenced is %i, index is %i", msg->lastEntityRef, index);
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 426, 0, "%s\n\t%s", "index - msg->lastEntityRef > 0", v4);
    }
    if (index - msg->lastEntityRef == 1)
    {
        if (msg_printEntityNums->current.enabled && SV_IsPacketDataNetworkData())
            Com_Printf(16, "Wrote entity num: 1 bit (inc)\n");
        MSG_WriteBit1(msg);
    }
    else
    {
        MSG_WriteBit0(msg);
        if (indexBits == 10 && index - msg->lastEntityRef < 16)
        {
            if (msg_printEntityNums->current.enabled && SV_IsPacketDataNetworkData())
                Com_Printf(16, "Wrote entity num: %i bits (delta)\n", 6);
            if (index - msg->lastEntityRef <= 0)
            {
                const char* someError = va("index was %i, lastEntityRef is %i", index, msg->lastEntityRef);
                MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 6969, 0, "%s", someError);
            }
            MSG_WriteBit0(msg);
            MSG_WriteBits(msg, index - msg->lastEntityRef, 4u);
        }
        else
        {
            if (msg_printEntityNums->current.enabled && SV_IsPacketDataNetworkData())
                Com_Printf(16, "Wrote entity num: %i bits (full)\n", indexBits + 2);
            if (indexBits == 10)
                MSG_WriteBit1(msg);
            MSG_WriteBits(msg, index, indexBits);
        }
    }
    msg->lastEntityRef = index;
}

void __cdecl MSG_WriteOriginFloat(const int clientNum, msg_t *msg, int bits, float value, float oldValue)
{
    int v5; // eax
    int MinBitCountForNum; // [esp+58h] [ebp-20h]
    int roundedValue; // [esp+60h] [ebp-18h]
    unsigned int roundedValuea; // [esp+60h] [ebp-18h]
    int truncDelta; // [esp+64h] [ebp-14h]
    int roundedOldValue; // [esp+68h] [ebp-10h]
    int roundedCenter; // [esp+6Ch] [ebp-Ch]
    int roundedCentera; // [esp+6Ch] [ebp-Ch]
    int index; // [esp+70h] [ebp-8h]
    int indexa; // [esp+70h] [ebp-8h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 186, 0, "%s", "!msg->readOnly");
    roundedValue = (int)value;
    roundedOldValue = (int)oldValue;
    truncDelta = roundedValue - roundedOldValue;
    SV_PacketDataIsOverhead(clientNum, msg);
    if ((unsigned int)(roundedValue - roundedOldValue + 64) >= 0x80)
    {
        MSG_WriteBit1(msg);
        if (bits == -92)
        {
            indexa = 0;
        }
        else
        {
            if (bits != -91)
                MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 249, 0, "%s", "bits == MSG_FIELD_ORIGINY");
            indexa = 1;
        }
        if (!svsHeaderValid)
            MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 253, 0, "%s", "svsHeaderValid");
        roundedCentera = (svsHeader.mapCenter[indexa] + 0.5);
        roundedValuea = (roundedOldValue + 0x8000 - roundedCentera) ^ (roundedValue - roundedCentera + 0x8000);
        SV_PacketDataIsOrigin(clientNum, msg);
        MinBitCountForNum = GetMinBitCountForNum(roundedValuea);
        if (MinBitCountForNum > 16)
        {
            if (indexa)
                Com_Error(
                    ERR_DROP,
                    "Entity with %s coordinate of %f is too far outside the playable area of the map.  The playable area goes from "
                    "( %f, %f, %f ) to ( %f, %f, %f )\n",
                    "Y",
                    value,
                    svsHeader.mapCenter[0] - 32768.0,
                    svsHeader.mapCenter[1] - 32768.0,
                    svsHeader.mapCenter[2] - 32768.0,
                    svsHeader.mapCenter[0] + 32768.0 - 1.0,
                    svsHeader.mapCenter[1] + 32768.0 - 1.0,
                    svsHeader.mapCenter[2] + 32768.0 - 1.0);
            else
                Com_Error(
                    ERR_DROP,
                    "Entity with %s coordinate of %f is too far outside the playable area of the map.  The playable area goes from "
                    "( %f, %f, %f ) to ( %f, %f, %f )\n",
                    "X",
                    value,
                    svsHeader.mapCenter[0] - 32768.0,
                    svsHeader.mapCenter[1] - 32768.0,
                    svsHeader.mapCenter[2] - 32768.0,
                    svsHeader.mapCenter[0] + 32768.0 - 1.0,
                    svsHeader.mapCenter[1] + 32768.0 - 1.0,
                    svsHeader.mapCenter[2] + 32768.0 - 1.0);
        }
        SV_TrackOriginFullBits(MinBitCountForNum);
        MSG_WriteBits(msg, roundedValuea, 0x10u);
    }
    else
    {
        MSG_WriteBit0(msg);
        SV_PacketDataIsOriginDelta(clientNum, msg);
        v5 = GetMinBitCountForNum(truncDelta + 64);
        SV_TrackOriginDeltaBits(v5);
        MSG_WriteBits(msg, truncDelta + 64, 7u);
        if (bits == -92)
        {
            index = 0;
        }
        else
        {
            if (bits != -91)
                MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 217, 0, "%s", "bits == MSG_FIELD_ORIGINY");
            index = 1;
        }
        if (!svsHeaderValid)
            MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 221, 0, "%s", "svsHeaderValid");
        roundedCenter = (svsHeader.mapCenter[index] + 0.5);
        if (GetMinBitCountForNum((roundedOldValue + 0x8000 - roundedCenter) ^ (roundedValue - roundedCenter + 0x8000)) > 16)
        {
            if (index)
                Com_Error(
                    ERR_DROP,
                    "Entity with %s coordinate of %f is too far outside the playable area of the map.  The playable area goes from "
                    "( %f, %f, %f ) to ( %f, %f, %f )\n",
                    "Y",
                    value,
                    svsHeader.mapCenter[0] - 32768.0,
                    svsHeader.mapCenter[1] - 32768.0,
                    svsHeader.mapCenter[2] - 32768.0,
                    svsHeader.mapCenter[0] + 32768.0 - 1.0,
                    svsHeader.mapCenter[1] + 32768.0 - 1.0,
                    svsHeader.mapCenter[2] + 32768.0 - 1.0);
            else
                Com_Error(
                    ERR_DROP,
                    "Entity with %s coordinate of %f is too far outside the playable area of the map.  The playable area goes from "
                    "( %f, %f, %f ) to ( %f, %f, %f )\n",
                    "X",
                    value,
                    svsHeader.mapCenter[0] - 32768.0,
                    svsHeader.mapCenter[1] - 32768.0,
                    svsHeader.mapCenter[2] - 32768.0,
                    svsHeader.mapCenter[0] + 32768.0 - 1.0,
                    svsHeader.mapCenter[1] + 32768.0 - 1.0,
                    svsHeader.mapCenter[2] + 32768.0 - 1.0);
        }
    }
}

void __cdecl MSG_WriteOriginZFloat(const int clientNum, msg_t *msg, float value, float oldValue)
{
    int v4; // eax
    int MinBitCountForNum; // eax
    int roundedValue; // [esp+58h] [ebp-14h]
    unsigned int roundedValuea; // [esp+58h] [ebp-14h]
    int truncDelta; // [esp+5Ch] [ebp-10h]
    int roundedOldValue; // [esp+60h] [ebp-Ch]
    int roundedCenter; // [esp+64h] [ebp-8h]
    int roundedCentera; // [esp+64h] [ebp-8h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 286, 0, "%s", "!msg->readOnly");
    roundedValue = (int)value;
    roundedOldValue = (int)oldValue;
    truncDelta = roundedValue - roundedOldValue;
    SV_PacketDataIsOverhead(clientNum, msg);
    if ((unsigned int)(roundedValue - roundedOldValue + 64) >= 0x80)
    {
        MSG_WriteBit1(msg);
        if (!svsHeaderValid)
            MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 330, 0, "%s", "svsHeaderValid");
        roundedCentera = (svsHeader.mapCenter[2] + 0.5);
        roundedValuea = (roundedOldValue + 0x8000 - roundedCentera) ^ (roundedValue - roundedCentera + 0x8000);
        SV_PacketDataIsOrigin(clientNum, msg);
        if (GetMinBitCountForNum(roundedValuea) > 16)
            Com_Error(
                ERR_DROP,
                "Entity with Z coordinate of %f is too far outside the playable area of the map.  The playable area goes from ( %"
                "f, %f, %f ) to ( %f, %f, %f )\n",
                value,
                svsHeader.mapCenter[0] - 32768.0,
                svsHeader.mapCenter[1] - 32768.0,
                svsHeader.mapCenter[2] - 32768.0,
                svsHeader.mapCenter[0] + 32768.0 - 1.0,
                svsHeader.mapCenter[1] + 32768.0 - 1.0,
                svsHeader.mapCenter[2] + 32768.0 - 1.0);
        MinBitCountForNum = GetMinBitCountForNum(roundedValuea);
        SV_TrackOriginZFullBits(MinBitCountForNum);
        MSG_WriteBits(msg, roundedValuea, 0x10u);
    }
    else
    {
        MSG_WriteBit0(msg);
        SV_PacketDataIsOriginDelta(clientNum, msg);
        if (!svsHeaderValid)
            MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 304, 0, "%s", "svsHeaderValid");
        roundedCenter = (svsHeader.mapCenter[2] + 0.5);
        if (GetMinBitCountForNum((roundedOldValue + 0x8000 - roundedCenter) ^ (roundedValue - roundedCenter + 0x8000)) > 16)
            Com_Error(
                ERR_DROP,
                "Entity with Z coordinate of %f is too far outside the playable area of the map.  The playable area goes from ( %"
                "f, %f, %f ) to ( %f, %f, %f )\n",
                value,
                svsHeader.mapCenter[0] - 32768.0,
                svsHeader.mapCenter[1] - 32768.0,
                svsHeader.mapCenter[2] - 32768.0,
                svsHeader.mapCenter[0] + 32768.0 - 1.0,
                svsHeader.mapCenter[1] + 32768.0 - 1.0,
                svsHeader.mapCenter[2] + 32768.0 - 1.0);
        v4 = GetMinBitCountForNum(truncDelta + 64);
        SV_TrackOriginZDeltaBits(v4);
        MSG_WriteBits(msg, truncDelta + 64, 7u);
    }
}

bool __cdecl MSG_ValuesAreEqual(const SnapshotInfo_s *snapInfo, int bits, const int *fromF, const int *toF)
{
    bool result; // al

    if (*fromF == *toF)
        return 1;
    switch (bits)
    {
    case -100:
    case -87:
        result = (unsigned __int16)(int)(*(float *)fromF * 182.0444488525391) == (unsigned __int16)(int)(*(float *)toF * 182.0444488525391);
        break;
    case -95:
        result = *fromF / 100 == *toF / 100;
        break;
    case -92:
    case -91:
    case -90:
        result = (int)(*(float *)fromF + 9.313225746154785e-10) == (int)(*(float *)toF + 9.313225746154785e-10);
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

void __cdecl MSG_WriteLastChangedField(msg_t *msg, int lastChangedFieldNum, unsigned int numFields)
{
    unsigned int idealBits; // [esp+0h] [ebp-4h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 471, 0, "%s", "!msg->readOnly");
    idealBits = GetMinBitCountForNum(numFields);
    MSG_WriteBits(msg, lastChangedFieldNum, idealBits);
}

void __cdecl MSG_WriteEventNum(int clientNum, msg_t *msg, unsigned __int8 eventNum)
{
    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 651, 0, "%s", "!msg->readOnly");
    SV_PacketDataIsData(clientNum, msg);
    MSG_WriteByte(msg, eventNum);
    SV_PacketDataIsUnknown(clientNum, msg);
}

void __cdecl MSG_WriteEventParam(int clientNum, msg_t *msg, unsigned __int8 eventParam)
{
    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 665, 0, "%s", "!msg->readOnly");
    SV_PacketDataIsData(clientNum, msg);
    MSG_WriteByte(msg, eventParam);
    SV_PacketDataIsUnknown(clientNum, msg);
}

PacketEntityType __cdecl MSG_GetPacketEntityTypeForEType(int eType)
{
    PacketEntityType result; // eax

    switch (eType)
    {
    case 0:
        result = ANALYZE_DATATYPE_ENTITYTYPE_GENERALENTITY;
        break;
    case 1:
        result = ANALYZE_DATATYPE_ENTITYTYPE_PLAYERENTITY;
        break;
    case 2:
        result = ANALYZE_DATATYPE_ENTITYTYPE_PLAYERCORPSEENTITY;
        break;
    case 3:
        result = ANALYZE_DATATYPE_ENTITYTYPE_ITEMENTITY;
        break;
    case 4:
        result = ANALYZE_DATATYPE_ENTITYTYPE_MISSILEENTITY;
        break;
    case 5:
        result = ANALYZE_DATATYPE_ENTITYTYPE_INVISIBLEENTITY;
        break;
    case 6:
        result = ANALYZE_DATATYPE_ENTITYTYPE_SCRIPTMOVERENTITY;
        break;
    case 7:
        result = ANALYZE_DATATYPE_ENTITYTYPE_SOUNDBLENDENTITY;
        break;
    case 8:
        result = ANALYZE_DATATYPE_ENTITYTYPE_FXENTITY;
        break;
    case 9:
        result = ANALYZE_DATATYPE_ENTITYTYPE_LOOPFXENTITY;
        break;
    case 11:
        result = ANALYZE_DATATYPE_ENTITYTYPE_MG42ENTITY;
        break;
    case 12:
        result = ANALYZE_DATATYPE_ENTITYTYPE_HELICOPTER;
        break;
    case 13:
        result = ANALYZE_DATATYPE_ENTITYTYPE_PLANE;
        break;
    case 14:
        result = ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE;
        break;
    case 15:
        result = ANALYZE_DATATYPE_ENTITYTYPE_VEHICLE_COLLMAP;
        break;
    default:
        result = ANALYZE_DATATYPE_ENTITYTYPE_TEMPENTITY;
        break;
    }
    return result;
}

unsigned int __cdecl MSG_GetBitCount(int bits, bool *estimate, int from, int to)
{
    const char *v5; // eax

    if (bits)
    {
        switch (bits)
        {
        case -89:
            *estimate = 1;
            return 33;
        case -88:
            *estimate = 0;
            return 32;
        case -99:
            *estimate = 1;
            return 33;
        case -100:
            *estimate = 0;
            return 17;
        case -87:
            *estimate = 0;
            return 16;
        case -86:
            *estimate = 0;
            return 5;
        case -85:
            *estimate = 0;
            if ((from & 0xFFFFFF00) == (to & 0xFFFFFF00))
            {
                if (((_BYTE)from || (unsigned __int8)to != 255) && ((unsigned __int8)from != 255 || (_BYTE)to))
                    return 7;
                else
                    return 2;
            }
            else
            {
                return 34;
            }
        case -97:
            *estimate = 1;
            return 9;
        case -98:
            *estimate = 0;
            return 6;
        case -96:
            *estimate = 1;
            return 3;
        case -94:
            *estimate = 0;
            return 8;
        case -93:
            *estimate = 0;
            return 8;
        case -92:
        case -91:
            *estimate = 1;
            return 17;
        case -90:
            *estimate = 1;
            return 17;
        case -95:
            *estimate = 0;
            return 7;
        default:
            if (bits < -50 && !alwaysfails)
            {
                v5 = va("Missed a MSG_ case in MSG_WriteDeltaField - value is %i", bits);
                MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 844, 0, v5);
            }
            *estimate = 0;
            return abs(bits) + 1;
        }
    }
    else
    {
        *estimate = 1;
        return 33;
    }
}

void __cdecl MSG_WriteEntity(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    entityState_s *from,
    const entityState_s *to,
    int force)
{
    char *EntityTypeName; // eax
    int eType; // [esp+4Ch] [ebp-4h]

    //Profile_Begin(298);
    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1633, 0, "%s", "!msg->readOnly");
    if (from->eventParm > 0xFF)
        MyAssertHandler(
            ".\\qcommon\\sv_msg_write_mp.cpp",
            1634,
            0,
            "%s\n\t(from->eventParm) = %i",
            "(from->eventParm <= 0xff)",
            from->eventParm);
    if (to)
    {
        eType = to->eType;
        snapInfo->packetEntityType = MSG_GetPacketEntityTypeForEType(eType);
        MSG_WriteEntityDeltaForEType(snapInfo, msg, time, eType, from, to, force);
        //Profile_EndInternal(0);
    }
    else
    {
        if (!from)
            MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1638, 0, "%s", "from");
        if (sv_debugPacketContents->current.enabled)
        {
            EntityTypeName = BG_GetEntityTypeName(from->eType);
            Com_Printf(15, "Removing entity %i - object is type %i (%s)\n", from->number, from->eType, EntityTypeName);
        }
        snapInfo->packetEntityType = MSG_GetPacketEntityTypeForEType(from->eType);
        MSG_WriteEntityRemoval(snapInfo, msg, (unsigned __int8 *)from, 10, 0);
        //Profile_EndInternal(0);
    }
}

void __cdecl MSG_WriteEntityRemoval(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    unsigned __int8 *from,
    int indexBits,
    bool changeBit)
{
    if (!from)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1557, 0, "%s", "from");
    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1558, 0, "%s", "!msg->readOnly");
    if (cl_shownet && (cl_shownet->current.integer >= 2 || cl_shownet->current.integer == -1))
        Com_Printf(16, "W|%3i: #%-3i remove\n", msg->cursize, *(unsigned int *)from);
    if (sv_debugPacketContents->current.enabled)
        Com_Printf(16, "Entity was removed\n");
    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
    if (changeBit)
        MSG_WriteBit1(msg);
    SV_PacketDataIsEntityNum(snapInfo->clientNum, msg);
    MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)from, indexBits);
    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
    MSG_WriteBit1(msg);
    SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
}

void __cdecl MSG_WriteEntityDeltaForEType(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    int eType,
    const entityState_s *from,
    const entityState_s *to,
    int force)
{
    char *EntityTypeName; // eax
    int bits; // [esp+0h] [ebp-8h]
    const NetFieldList *fieldList; // [esp+4h] [ebp-4h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1594, 0, "%s", "!msg->readOnly");
    fieldList = MSG_GetStateFieldListForEntityType(eType);
    snapInfo->fieldChanges = (int *)&orderInfo;
    bits = MSG_WriteEntityDelta(
        snapInfo,
        msg,
        time,
        (const unsigned __int8 *)from,
        (const unsigned __int8 *)to,
        force,
        fieldList->count,
        10,
        fieldList->array);
    SV_TrackETypeBytes(eType, bits);
    if (bits)
    {
        if (sv_debugPacketContents->current.enabled)
        {
            EntityTypeName = BG_GetEntityTypeName(to->eType);
            Com_Printf(
                15,
                "^^ Entity delta entnum %i - object is type %i (%s) - took %i bits\n",
                to->number,
                to->eType,
                EntityTypeName,
                bits);
        }
    }
}

int __cdecl MSG_WriteEntityDelta(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    const unsigned __int8 *from,
    const unsigned __int8 *to,
    int force,
    int numFields,
    int indexBits,
    const NetField *stateFields)
{
    const char *v9; // eax
    int startBits; // [esp+0h] [ebp-18h]
    const NetField *field; // [esp+4h] [ebp-14h]
    const NetField *fielda; // [esp+4h] [ebp-14h]
    int lc; // [esp+8h] [ebp-10h]
    int *toF; // [esp+Ch] [ebp-Ch]
    int *fromF; // [esp+10h] [ebp-8h]
    signed int i; // [esp+14h] [ebp-4h]
    int ia; // [esp+14h] [ebp-4h]

    startBits = MSG_GetUsedBitCount(msg);
    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1446, 0, "%s", "!msg->readOnly");
    if (!to)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1447, 0, "%s", "to");
    if (*(unsigned int *)to >= (unsigned int)(1 << indexBits))
    {
        v9 = va("to = %i, bits = %i", *(unsigned int *)to, indexBits);
        MyAssertHandler(
            ".\\qcommon\\sv_msg_write_mp.cpp",
            1449,
            0,
            "%s\n\t%s",
            "*reinterpret_cast< const uint * >( to ) < (1u << indexBits)",
            v9);
    }
    lc = 0;
    i = 0;
    field = stateFields;
    while (i < numFields)
    {
        fromF = (int *)&from[field->offset];
        toF = (int *)&to[field->offset];
        if (!MSG_ValuesAreEqual(snapInfo, field->bits, fromF, toF))
        {
            if (*fromF == *toF)
                MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1459, 0, "%s", "*fromF != *toF");
            SV_TrackFieldChange(snapInfo->clientNum, snapInfo->packetEntityType, i);
            lc = i + 1;
        }
        ++i;
        ++field;
    }
    if (lc < 0 || lc > numFields)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1466, 0, "%s\n\t(lc) = %i", "(lc >= 0 && lc <= numFields)", lc);
    if (lc)
    {
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Entity had a delta\n");
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Writing index number %i\n", *(unsigned int *)to);
        SV_PacketDataIsEntityNum(snapInfo->clientNum, msg);
        MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)to, indexBits);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Writing 0,1 to say it's not removed and we have a delta\n");
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        MSG_WriteBit0(msg);
        MSG_WriteBit1(msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Writing byte for how many fields changed (%i)\n", lc);
        SV_PacketDataIsLastFieldChanged(snapInfo->clientNum, msg);
        MSG_WriteLastChangedField(msg, lc, numFields);
        ia = 0;
        fielda = stateFields;
        while (ia < lc)
        {
            SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "Writing delta for field %i (%s)\n", ia, fielda->name);
            MSG_WriteDeltaField(snapInfo, msg, time, from, to, fielda++, ia++, 0);
        }
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else if (force)
    {
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Entity did not change, but we're forcing a send to say this\n");
        SV_PacketDataIsEntityNum(snapInfo->clientNum, msg);
        MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)to, indexBits);
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        MSG_WriteBit0(msg);
        MSG_WriteBit0(msg);
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else
    {
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        return 0;
    }
}

void __cdecl MSG_WriteDeltaField(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    const unsigned __int8 *from,
    const unsigned __int8 *to,
    const NetField *field,
    int fieldNum,
    bool forceSend)
{
    const char *EntityTypeString; // eax
    int v9; // eax
    int v10; // eax
    int v11; // eax
    const char *v12; // eax
    __int64 v13; // [esp-Ch] [ebp-88h]
    __int64 v14; // [esp-8h] [ebp-84h]
    double f; // [esp+4h] [ebp-78h]
    float v16; // [esp+20h] [ebp-5Ch]
    const hudelem_color_t *fromColor; // [esp+44h] [ebp-38h]
    const hudelem_color_t *toColor; // [esp+48h] [ebp-34h]
    int zeroVal; // [esp+4Ch] [ebp-30h] BYREF
    int trunc; // [esp+50h] [ebp-2Ch]
    int oldTrunc; // [esp+54h] [ebp-28h]
    const int *toF; // [esp+58h] [ebp-24h]
    unsigned __int8 *b; // [esp+5Ch] [ebp-20h]
    float oldFloat; // [esp+60h] [ebp-1Ch]
    int bits; // [esp+64h] [ebp-18h]
    const int *fromF; // [esp+68h] [ebp-14h]
    float fullFloat; // [esp+6Ch] [ebp-10h]
    float oldValue; // [esp+70h] [ebp-Ch]
    int partialBits; // [esp+74h] [ebp-8h]
    int value; // [esp+78h] [ebp-4h] BYREF

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 878, 0, "%s", "!msg->readOnly");
    fromF = (const int *)&from[field->offset];
    toF = (const int *)&to[field->offset];
    if (forceSend)
    {
        zeroVal = 0;
        fromF = &zeroVal;
    }
    if (field->changeHints != 2)
    {
        if (!forceSend && MSG_ValuesAreEqual(snapInfo, field->bits, fromF, toF))
        {
            SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
            MSG_WriteBit0(msg);
            SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
            return;
        }
        if (!forceSend && *fromF == *toF)
            MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 904, 0, "%s", "forceSend || *fromF != *toF");
        if (field->changeHints == 1 && (!snapInfo->fromBaseline || fieldNum))
        {
            EntityTypeString = SV_GetEntityTypeString(snapInfo->packetEntityType);
            Com_PrintError(
                15,
                "Field %s changed for eType %s when we thought it never would\n",
                field->name,
                EntityTypeString);
        }
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        MSG_WriteBit1(msg);
    }
    ++snapInfo->fieldChanges[fieldNum];
    if (field->bits)
    {
        switch (field->bits)
        {
        case 0xFFFFFFA7:
            fullFloat = *(float *)toF;
            trunc = (int)fullFloat;
            oldFloat = *(float *)fromF;
            oldTrunc = (int)oldFloat;
            SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
            if (fullFloat != (double)trunc || LODWORD(fullFloat) == 0x80000000 || (unsigned int)(trunc + 4096) >= 0x2000)
            {
                MSG_WriteBit1(msg);
                SV_PacketDataIsLargeFloat(snapInfo->clientNum, msg);
                MSG_WriteLong(msg, *fromF ^ *toF);
            }
            else
            {
                MSG_WriteBit0(msg);
                SV_PacketDataIsSmallFloat(snapInfo->clientNum, msg);
                trunc += 4096;
                trunc ^= oldTrunc + 4096;
                MSG_WriteBits(msg, trunc, 5u);
                MSG_WriteByte(msg, trunc >> 5);
            }
            goto LABEL_103;
        case 0xFFFFFFA8:
            SV_PacketDataIsLargeFloat(snapInfo->clientNum, msg);
            MSG_WriteLong(msg, *fromF ^ *toF);
            goto LABEL_103;
        case 0xFFFFFF9D:
            fullFloat = *(float *)toF;
            trunc = (int)fullFloat;
            oldFloat = *(float *)fromF;
            oldTrunc = (int)oldFloat;
            if (fullFloat != 0.0 || LODWORD(fullFloat) == 0x80000000)
            {
                SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
                MSG_WriteBit1(msg);
                if (LODWORD(fullFloat) == 0x80000000 || fullFloat != (double)trunc || (unsigned int)(trunc + 2048) >= 0x1000)
                {
                    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
                    MSG_WriteBit1(msg);
                    SV_PacketDataIsLargeFloat(snapInfo->clientNum, msg);
                    MSG_WriteLong(msg, *fromF ^ *toF);
                }
                else
                {
                    MSG_WriteBit0(msg);
                    SV_PacketDataIsSmallFloat(snapInfo->clientNum, msg);
                    trunc += 2048;
                    trunc ^= oldTrunc + 2048;
                    MSG_WriteBits(msg, trunc, 4u);
                    MSG_WriteByte(msg, trunc >> 4);
                }
            }
            else
            {
                SV_PacketDataIsZeroFloat(snapInfo->clientNum, msg);
                MSG_WriteBit0(msg);
            }
            if ((unsigned int)(__int64)(*(float *)toF + 2048.0) >= 0x1000)
                MyAssertHandler(
                    ".\\qcommon\\sv_msg_write_mp.cpp",
                    1068,
                    0,
                    "*(float *)toF + HUDELEM_COORD_BIAS doesn't index 1 << HUDELEM_COORD_BITS\n\t%i not in [0, %i)",
                    (int)(*(float *)toF + 2048.0),
                    4096);
            goto LABEL_103;
        case 0xFFFFFF9C:
            if (*toF)
            {
                SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
                MSG_WriteBit1(msg);
                fullFloat = *(float *)toF;
                SV_PacketDataIsSmallAngle(snapInfo->clientNum, msg);
                MSG_WriteAngle16(msg, fullFloat);
            }
            else
            {
                SV_PacketDataIsZeroAngle(snapInfo->clientNum, msg);
                MSG_WriteBit0(msg);
            }
            goto LABEL_103;
        case 0xFFFFFFA9:
            SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
            fullFloat = *(float *)toF;
            SV_PacketDataIsSmallAngle(snapInfo->clientNum, msg);
            MSG_WriteAngle16(msg, fullFloat);
            goto LABEL_103;
        case 0xFFFFFFAA:
            fullFloat = *(float *)toF;
            v16 = (fullFloat - 1.399999976158142) * 10.0;
            trunc = (int)(v16 + 9.313225746154785e-10);
            if (!MSG_CheckWritingEnoughBits(trunc, 5u))
            {
                LODWORD(f) = *toF;
                Com_PrintError(15, "Not enough bits written for fontScale %f\n", f);
            }
            MSG_WriteBits(msg, trunc, 5u);
            goto LABEL_103;
        }
        if (field->bits != -85)
        {
            switch (field->bits)
            {
            case 0xFFFFFF9F:
                MSG_WriteDeltaTime(snapInfo->clientNum, msg, time, *toF);
                break;
            case 0xFFFFFF9E:
                MSG_Write24BitFlag(snapInfo->clientNum, msg, *fromF, *toF);
                break;
            case 0xFFFFFFA0:
                MSG_WriteGroundEntityNum(snapInfo->clientNum, msg, *toF);
                break;
            case 0xFFFFFFA2:
                MSG_WriteEventNum(snapInfo->clientNum, msg, *toF);
                break;
            case 0xFFFFFFA3:
                MSG_WriteEventParam(snapInfo->clientNum, msg, *toF);
                break;
            case 0xFFFFFFA4:
            case 0xFFFFFFA5:
                fullFloat = *(float *)toF;
                oldValue = *(float *)fromF;
                //HIDWORD(v13) = msg;
                //LODWORD(v13) = snapInfo->clientNum;
                //MSG_WriteOriginFloat(v13, field->bits, fullFloat, oldValue);
                MSG_WriteOriginFloat(snapInfo->clientNum, msg, field->bits, fullFloat, oldValue);
                break;
            case 0xFFFFFFA6:
                fullFloat = *(float *)toF;
                oldValue = *(float *)fromF;
                //HIDWORD(v14) = msg;
                //LODWORD(v14) = snapInfo->clientNum;
                //MSG_WriteOriginZFloat(v14, fullFloat, oldValue);
                MSG_WriteOriginZFloat(snapInfo->clientNum, msg, fullFloat, oldValue);
                break;
            case 0xFFFFFFA1:
                value = *toF;
                if (sv_debugPacketContents->current.enabled)
                    Com_Printf(16, "Sending %i as playerstate timer value (%ims granularity)\n", value, 100);
                MSG_WriteBits(msg, value / 100, 7u);
                break;
            default:
                if (field->bits < -50 && !alwaysfails)
                {
                    v12 = va("Missed a MSG_ case in MSG_WriteDeltaField - value is %i", field->bits);
                    MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1190, 0, v12);
                }
                if (*toF)
                {
                    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
                    MSG_WriteBit1(msg);
                    bits = abs(field->bits);
                    value = *toF;
                    value ^= *fromF;
                    if (!MSG_CheckWritingEnoughBits(value, bits))
                        Com_PrintError(1, "Not enough bits written: %d for %s (%d)\n", value, field->name, bits);
                    partialBits = bits & 7;
                    SV_PacketDataIsData(snapInfo->clientNum, msg);
                    if (partialBits)
                    {
                        MSG_WriteBits(msg, value, partialBits);
                        bits -= partialBits;
                        value >>= partialBits;
                    }
                    while (bits)
                    {
                        MSG_WriteByte(msg, value);
                        value >>= 8;
                        bits -= 8;
                    }
                }
                else
                {
                    SV_PacketDataIsZeroInt(snapInfo->clientNum, msg);
                    MSG_WriteBit0(msg);
                }
                break;
            }
            goto LABEL_103;
        }
        toColor = (const hudelem_color_t *)toF;
        fromColor = (const hudelem_color_t *)fromF;
        if ((*((unsigned __int8 *)fromF + 3) != 255 || *((_BYTE *)toF + 3))
            && (*((_BYTE *)fromF + 3) || *((unsigned __int8 *)toF + 3) != 255)
            || memcmp(fromF, toF, 3u))
        {
            MSG_WriteBit0(msg);
            if (fromColor->r == toColor->r && fromColor->g == toColor->g && fromColor->b == toColor->b)
            {
                MSG_WriteBit1(msg);
            }
            else
            {
                MSG_WriteBit0(msg);
                MSG_WriteByte(msg, toColor->r);
                MSG_WriteByte(msg, toColor->g);
                MSG_WriteByte(msg, toColor->b);
            }
            MSG_WriteBits(msg, (int)toColor->a >> 3, 5u);
            goto LABEL_103;
        }
    }
    else
    {
        fullFloat = *(float *)toF;
        trunc = (int)fullFloat;
        oldFloat = *(float *)fromF;
        oldTrunc = (int)oldFloat;
        if (fullFloat != 0.0)
        {
            SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
            MSG_WriteBit1(msg);
            if (LODWORD(fullFloat) == 0x80000000
                || fullFloat != (double)trunc
                || (unsigned int)(trunc + 4096) >= 0x2000
                || (unsigned int)(oldTrunc + 4096) >= 0x2000)
            {
                MSG_WriteBit1(msg);
                SV_PacketDataIsLargeFloat(snapInfo->clientNum, msg);
                value = *fromF ^ *toF;
                MSG_WriteLong(msg, value);
                b = (unsigned __int8 *)&value;
                bits = Huff_bitCount(&msgHuff.compressDecompress, (unsigned __int8)value);
                v9 = Huff_bitCount(&msgHuff.compressDecompress, b[1]);
                bits += v9;
                v10 = Huff_bitCount(&msgHuff.compressDecompress, b[2]);
                bits += v10;
                v11 = Huff_bitCount(&msgHuff.compressDecompress, b[3]);
                bits += v11;
                SV_TrackFloatCompressedBits(bits);
            }
            else
            {
                MSG_WriteBit0(msg);
                SV_PacketDataIsSmallFloat(snapInfo->clientNum, msg);
                trunc += 4096;
                trunc ^= oldTrunc + 4096;
                if ((unsigned int)trunc >= 0x2000)
                    MyAssertHandler(
                        ".\\qcommon\\sv_msg_write_mp.cpp",
                        953,
                        0,
                        "trunc not in [0, (1 << FLOAT_INT_BITS) - 1]\n\t%i not in [%i, %i]",
                        trunc,
                        0,
                        0x1FFF);
                MSG_WriteBits(msg, trunc, 5u);
                MSG_WriteByte(msg, trunc >> 5);
            }
            goto LABEL_103;
        }
        SV_PacketDataIsZeroFloat(snapInfo->clientNum, msg);
        MSG_WriteBit0(msg);
        if (LODWORD(fullFloat) != 0x80000000)
        {
            MSG_WriteBit0(msg);
            goto LABEL_103;
        }
    }
    MSG_WriteBit1(msg);
LABEL_103:
    SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
}

void __cdecl MSG_WriteDeltaTime(int clientNum, msg_t *msg, int timeBase, int time)
{
    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 489, 0, "%s", "!msg->readOnly");
    if (time - timeBase > 0 || time - timeBase <= -256)
    {
        SV_PacketDataIsOverhead(clientNum, msg);
        MSG_WriteBit1(msg);
        SV_PacketDataIsTime(clientNum, msg);
        MSG_WriteLong(msg, time);
        SV_PacketDataIsUnknown(clientNum, msg);
    }
    else
    {
        SV_PacketDataIsOverhead(clientNum, msg);
        MSG_WriteBit0(msg);
        SV_PacketDataIsTimeDelta(clientNum, msg);
        MSG_WriteBits(msg, timeBase - time, 8u);
        SV_PacketDataIsUnknown(clientNum, msg);
    }
}

void __cdecl MSG_Write24BitFlag(int clientNum, msg_t *msg, int oldFlags, int newFlags)
{
    int bits; // [esp+0h] [ebp-10h]
    unsigned int changedBitIndex; // [esp+4h] [ebp-Ch]
    int flagDiff; // [esp+8h] [ebp-8h]
    int value; // [esp+Ch] [ebp-4h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 532, 0, "%s", "!msg->readOnly");
    flagDiff = newFlags ^ oldFlags;
    if (newFlags == oldFlags)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 535, 0, "%s", "flagDiff");
    if ((flagDiff & (flagDiff - 1)) != 0)
    {
        SV_PacketDataIsOverhead(clientNum, msg);
        MSG_WriteBit1(msg);
        value = newFlags;
        bits = 24;
        SV_PacketDataIsData(clientNum, msg);
        while (bits)
        {
            MSG_WriteByte(msg, value);
            value >>= 8;
            bits -= 8;
        }
    }
    else
    {
        changedBitIndex = 0;
        while ((flagDiff & 1) == 0)
        {
            ++changedBitIndex;
            flagDiff >>= 1;
        }
        if (changedBitIndex > 0x18)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                567,
                0,
                "%s\n\t(changedBitIndex) = %i",
                "(changedBitIndex >= 0 && changedBitIndex <= 24)",
                changedBitIndex);
        if ((1 << changedBitIndex) ^ newFlags ^ oldFlags)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                568,
                0,
                "%s",
                "( ( oldFlags ^ newFlags ) ^ ( 1 << changedBitIndex ) ) == 0");
        SV_PacketDataIsOverhead(clientNum, msg);
        MSG_WriteBit0(msg);
        SV_PacketDataIs24BitFlagIndex(clientNum, msg);
        MSG_WriteBits(msg, changedBitIndex, 5u);
        if (msg->overflowed)
            MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 581, 0, "%s", "!msg->overflowed");
    }
    SV_PacketDataIsUnknown(clientNum, msg);
}

void __cdecl MSG_WriteGroundEntityNum(int clientNum, msg_t *msg, int groundEntityNum)
{
    int bits; // [esp+0h] [ebp-Ch]
    int value; // [esp+8h] [ebp-4h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 596, 0, "%s", "!msg->readOnly");
    if (groundEntityNum == 1022 || (SV_PacketDataIsOverhead(clientNum, msg), MSG_WriteBit0(msg), !groundEntityNum))
    {
        SV_PacketDataIsGroundEntity(clientNum, msg);
        MSG_WriteBit1(msg);
    }
    else
    {
        MSG_WriteBit0(msg);
        SV_PacketDataIsGroundEntity(clientNum, msg);
        MSG_WriteBits(msg, groundEntityNum, 2u);
        bits = 8;
        value = groundEntityNum >> 2;
        while (bits)
        {
            MSG_WriteByte(msg, value);
            value >>= 8;
            bits -= 8;
        }
    }
    SV_PacketDataIsUnknown(clientNum, msg);
}

bool __cdecl MSG_CheckWritingEnoughBits(int value, unsigned int bits)
{
    DWORD v3; // eax
    unsigned int checkBits; // [esp+4h] [ebp-8h]
    unsigned int checkValue; // [esp+8h] [ebp-4h]

    if (value < 0)
    {
        checkValue = -value - 1;
        checkBits = bits - 1;
    }
    else
    {
        checkValue = value;
        checkBits = bits;
    }
    if (!_BitScanReverse(&v3, checkValue))
        v3 = 63; // `CountLeadingZeros'::`2': : notFound;
    return checkBits >= 32 - (v3 ^ 0x1Fu);
}

void __cdecl MSG_WriteDeltaArchivedEntity(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    archivedEntity_s *from,
    archivedEntity_s *to,
    int force)
{
    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1679, 0, "%s", "!msg->readOnly");
    snapInfo->packetEntityType = ANALYZE_DATATYPE_ENTITYTYPE_ARCHIVEDENTITY;
    snapInfo->fieldChanges = orderInfo.arcEntState;
    MSG_WriteDeltaStruct(
        snapInfo,
        msg,
        time,
        (unsigned __int8 *)from,
        (unsigned __int8 *)to,
        force,
        69,
        10,
        archivedEntityFields,
        0);
}

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
    int bChangeBit)
{
    const char *v10; // eax
    int startBits; // [esp+0h] [ebp-18h]
    const NetField *field; // [esp+4h] [ebp-14h]
    const NetField *fielda; // [esp+4h] [ebp-14h]
    int lc; // [esp+8h] [ebp-10h]
    signed int i; // [esp+14h] [ebp-4h]
    int ia; // [esp+14h] [ebp-4h]

    startBits = MSG_GetUsedBitCount(msg);
    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1313, 0, "%s", "!msg->readOnly");
    if (!to)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1314, 0, "%s", "to");
    if (*(unsigned int *)to >= (unsigned int)(1 << indexBits))
    {
        v10 = va("to = %i, bits = %i", *(unsigned int *)to, indexBits);
        MyAssertHandler(
            ".\\qcommon\\sv_msg_write_mp.cpp",
            1316,
            0,
            "%s\n\t%s",
            "*reinterpret_cast< unsigned * >( to ) < (1u << indexBits)",
            v10);
    }
    lc = 0;
    i = 0;
    field = stateFields;
    while (i < numFields)
    {
        if (!MSG_ValuesAreEqual(snapInfo, field->bits, (const int *)&from[field->offset], (const int *)&to[field->offset]))
        {
            SV_TrackFieldChange(snapInfo->clientNum, snapInfo->packetEntityType, i);
            lc = i + 1;
        }
        ++i;
        ++field;
    }
    if (lc < 0 || lc > numFields)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1332, 0, "%s\n\t(lc) = %i", "(lc >= 0 && lc <= numFields)", lc);
    if (lc)
    {
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Entity had a delta\n");
        if (bChangeBit)
        {
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "Writing 1 for bChangeBit\n");
            MSG_WriteBit1(msg);
        }
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Writing index number %i\n", *(unsigned int *)to);
        SV_PacketDataIsEntityNum(snapInfo->clientNum, msg);
        MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)to, indexBits);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Writing 0,1 to say it's not removed and we have a delta\n");
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        MSG_WriteBit0(msg);
        MSG_WriteBit1(msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Writing byte for how many fields changed (%i)\n", lc);
        SV_PacketDataIsLastFieldChanged(snapInfo->clientNum, msg);
        MSG_WriteLastChangedField(msg, lc, numFields);
        ia = 0;
        fielda = stateFields;
        while (ia < lc)
        {
            SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "Writing delta for field %i (%s)\n", ia, fielda->name);
            MSG_WriteDeltaField(snapInfo, msg, time, from, to, fielda++, ia++, 0);
        }
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else if (force)
    {
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Entity %u did not change, but we're forcing a send to say this\n", *(unsigned int *)to);
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        if (bChangeBit)
            MSG_WriteBit1(msg);
        SV_PacketDataIsEntityNum(snapInfo->clientNum, msg);
        MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)to, indexBits);
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        MSG_WriteBit0(msg);
        MSG_WriteBit0(msg);
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else
    {
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        return 0;
    }
}

void __cdecl MSG_WriteDeltaClient(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    clientState_s *from,
    clientState_s *to,
    int force)
{
    clientState_s dummy; // [esp+4h] [ebp-70h] BYREF
    int bits; // [esp+70h] [ebp-4h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1718, 0, "%s", "!msg->readOnly");
    if (!from)
    {
        from = &dummy;
        memset((unsigned __int8 *)&dummy, 0, sizeof(dummy));
    }
    if (to)
    {
        snapInfo->packetEntityType = ANALYZE_DATATYPE_ENTITYTYPE_CLIENTSTATE;
        snapInfo->fieldChanges = orderInfo.clientState;
        bits = MSG_WriteDeltaStruct(
            snapInfo,
            msg,
            time,
            (unsigned __int8 *)from,
            (unsigned __int8 *)to,
            force,
            24,
            6,
            clientStateFields,
            1);
        if (bits)
        {
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(15, "^^ client state delta for client %i - %i bits\n", to->clientIndex, bits);
        }
    }
    else
    {
        snapInfo->packetEntityType = ANALYZE_DATATYPE_ENTITYTYPE_CLIENTSTATE;
        MSG_WriteEntityRemoval(snapInfo, msg, (unsigned __int8 *)from, 6, 1);
    }
}

void __cdecl MSG_WriteDeltaPlayerstate(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    const playerState_s *from,
    const playerState_s *to)
{
    int v5; // eax
    int v6; // eax
    float diff[8]; // [esp+2Ch] [ebp-2FD8h] BYREF
    int i; // [esp+4Ch] [ebp-2FB8h]
    int v9; // [esp+50h] [ebp-2FB4h]
    int v10; // [esp+54h] [ebp-2FB0h]
    int numFields; // [esp+58h] [ebp-2FACh]
    NetField *field; // [esp+5Ch] [ebp-2FA8h]
    float v13; // [esp+60h] [ebp-2FA4h]
    int lastChangedFieldNum; // [esp+64h] [ebp-2FA0h]
    int v15; // [esp+68h] [ebp-2F9Ch]
    unsigned __int8 dst[sizeof(playerState_s)]; // [esp+6Ch] [ebp-2F98h] BYREF
    int value; // [esp+2FD8h] [ebp-2Ch]
    int c[4]; // [esp+2FDCh] [ebp-28h]
    int v19; // [esp+2FECh] [ebp-18h]
    int fieldNum; // [esp+2FF0h] [ebp-14h]
    int v21; // [esp+2FF4h] [ebp-10h]
    int v22; // [esp+2FF8h] [ebp-Ch]
    bool v23; // [esp+2FFFh] [ebp-5h]
    int UsedBitCount; // [esp+3000h] [ebp-4h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1874, 0, "%s", "!msg->readOnly");
    //Profile_Begin(292);
    UsedBitCount = MSG_GetUsedBitCount(msg);
    if (sv_debugPacketContents->current.enabled)
        Com_Printf(16, "Writing playerstate for client #%i\n", snapInfo->clientNum);
    snapInfo->packetEntityType = ANALYZE_DATATYPE_ENTITYTYPE_PLAYERSTATE;
    if (!from)
    {
        from = (const playerState_s *)dst;
        memset(dst, 0, sizeof(playerState_s));
    }
    if (snapInfo->archived)
    {
        v23 = 1;
        MSG_WriteBit1(msg);
    }
    else
    {
        Vec3Sub(to->origin, snapInfo->client->predictedOrigin, diff);
        v13 = Vec3LengthSq(diff);
        if (!svsHeaderValid)
            MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1900, 0, "%s", "svsHeaderValid");
        if (from
            && svsHeader.clientArchive
            && v13 <= 0.009999999776482582
            && snapInfo->client->predictedOriginServerTime == to->commandTime)
        {
            v23 = 0;
            MSG_WriteBit0(msg);
        }
        else
        {
            if (v13 <= 0.009999999776482582)
            {
                if (svs.clients[snapInfo->clientNum].header.predictedOriginServerTime != to->commandTime)
                    ++originsSentDueToServerTimeMismatch;
            }
            else
            {
                ++originsSentDueToPredicitonError;
            }
            v23 = 1;
            MSG_WriteBit1(msg);
        }
    }
    numFields = 141;
    lastChangedFieldNum = 0;
    fieldNum = 0;
    field = (NetField *)playerStateFields;
    while (fieldNum < numFields)
    {
        if (MSG_ShouldSendPSField(snapInfo, v23, to, from, field))
        {
            SV_TrackFieldChange(snapInfo->clientNum, 20, fieldNum);
            if (sv_debugPlayerstate->current.enabled && !snapInfo->archived)
            {
                if (I_strcmp(field->name, "commandTime"))
                    Com_Printf(16, "PS field %s changed\n", field->name);
            }
            lastChangedFieldNum = fieldNum + 1;
        }
        ++fieldNum;
        ++field;
    }
    snapInfo->fieldChanges = orderInfo.playerState;
    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
    if (sv_debugPacketContents->current.enabled)
        Com_Printf(16, "Writing byte for number of fields changed (%i)\n", lastChangedFieldNum);
    MSG_WriteLastChangedField(msg, lastChangedFieldNum, numFields);
    fieldNum = 0;
    field = (NetField *)playerStateFields;
    while (fieldNum < lastChangedFieldNum)
    {
        if (field->changeHints == 2 || MSG_ShouldSendPSField(snapInfo, v23, to, from, field))
        {
            if (snapInfo->archived || field->changeHints != 3)
                MSG_WriteDeltaField(
                    snapInfo,
                    msg,
                    time,
                    (const unsigned __int8 *)from,
                    (const unsigned __int8 *)to,
                    field,
                    fieldNum,
                    0);
            else
                MSG_WriteDeltaField(
                    snapInfo,
                    msg,
                    time,
                    (const unsigned __int8 *)from,
                    (const unsigned __int8 *)to,
                    field,
                    fieldNum,
                    1);
        }
        else
        {
            SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
            MSG_WriteBit0(msg);
            SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        }
        ++fieldNum;
        ++field;
    }
    v9 = MSG_GetUsedBitCount(msg);
    SV_TrackPSFieldDeltasBits(v9 - UsedBitCount);
    value = 0;
    for (fieldNum = 0; fieldNum < 5; ++fieldNum)
    {
        if (to->stats[fieldNum] != from->stats[fieldNum])
            value |= 1 << fieldNum;
    }
    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
    if (value)
    {
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Sending player stats changes - bit 1 to say it changed, %i bits for which changed\n", 5);
        MSG_WriteBit1(msg);
        MSG_WriteBits(msg, value, 5u);
        SV_PacketDataIsData(snapInfo->clientNum, msg);
        if ((value & 1) != 0)
        {
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "Sending player health stat (value is %i)\n", to->stats[0]);
            MSG_WriteShort(msg, to->stats[0]);
        }
        if ((value & 2) != 0)
        {
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "Sending player dead yaw stat (value is %i)\n", to->stats[1]);
            MSG_WriteShort(msg, to->stats[1]);
        }
        if ((value & 4) != 0)
        {
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "Sending player maximum health stat (value is %i)\n", to->stats[2]);
            MSG_WriteShort(msg, to->stats[2]);
        }
        if ((value & 8) != 0)
        {
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "Sending player crosshair client stat (value is %i)\n", to->stats[3]);
            MSG_WriteBits(msg, to->stats[3], 6u);
        }
        if ((value & 0x10) != 0)
        {
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "Sending player spawn count stat (value is %i)\n", to->stats[4]);
            MSG_WriteByte(msg, to->stats[4]);
        }
    }
    else
    {
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Writing 0 to say no player stats changed\n");
        MSG_WriteBit0(msg);
    }
    v10 = MSG_GetUsedBitCount(msg);
    SV_TrackPSStatsBits(v10 - v9);
    for (i = 0; i < 4; ++i)
    {
        c[i] = 0;
        for (fieldNum = 0; fieldNum < 16; ++fieldNum)
        {
            if (to->ammo[16 * i + fieldNum] != from->ammo[16 * i + fieldNum])
                c[i] |= 1 << fieldNum;
        }
    }
    if (c[0] || c[1] || c[2] || c[3])
    {
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "Player ammo bits changed\n");
        MSG_WriteBit1(msg);
        for (i = 0; i < 4; ++i)
        {
            if (c[i])
            {
                MSG_WriteBit1(msg);
                SV_PacketDataIsData(snapInfo->clientNum, msg);
                if (sv_debugPacketContents->current.enabled)
                    Com_Printf(
                        16,
                        "ammobits[%i] changed, sending bits as short (value is %i) followed by the ammo values as shorts\n",
                        i,
                        c[i]);
                MSG_WriteShort(msg, c[i]);
                for (fieldNum = 0; fieldNum < 16; ++fieldNum)
                {
                    if ((c[i] & (1 << fieldNum)) != 0)
                        MSG_WriteShort(msg, to->ammo[16 * i + fieldNum]);
                }
            }
            else
            {
                SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
                if (sv_debugPacketContents->current.enabled)
                    Com_Printf(16, "ammobits[%i] did not change\n", i);
                MSG_WriteBit0(msg);
            }
        }
    }
    else
    {
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "ammobits did not change\n", i);
        MSG_WriteBit0(msg);
    }
    for (i = 0; i < 8; ++i)
    {
        v19 = 0;
        for (fieldNum = 0; fieldNum < 16; ++fieldNum)
        {
            if (to->ammoclip[16 * i + fieldNum] != from->ammoclip[16 * i + fieldNum])
                v19 |= 1 << fieldNum;
        }
        if (v19)
        {
            SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "sending clip ammo\n", i);
            MSG_WriteBit1(msg);
            SV_PacketDataIsData(snapInfo->clientNum, msg);
            MSG_WriteShort(msg, v19);
            for (fieldNum = 0; fieldNum < 16; ++fieldNum)
            {
                if ((v19 & (1 << fieldNum)) != 0)
                    MSG_WriteShort(msg, to->ammoclip[16 * i + fieldNum]);
            }
        }
        else
        {
            SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "clip ammo did not change\n", i);
            MSG_WriteBit0(msg);
        }
    }
    v21 = MSG_GetUsedBitCount(msg);
    SV_TrackPSAmmoBits(v21 - v10);
    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
    if (sv_debugPacketContents->current.enabled)
        Com_Printf(16, "sending objectives\n", i);
    if (!memcmp(from->objective, to->objective, sizeof(playerState_s::objective)))
    {
        MSG_WriteBit0(msg);
    }
    else
    {
        MSG_WriteBit1(msg);
        for (fieldNum = 0; fieldNum < 16; ++fieldNum)
        {
            SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
            if (sv_debugPacketContents->current.enabled)
                Com_Printf(16, "sending objective %i\n", fieldNum);
            MSG_WriteBits(msg, to->objective[fieldNum].state, 3u);
            snapInfo->fieldChanges = orderInfo.objective;
            MSG_WriteDeltaFields(
                snapInfo,
                msg,
                time,
                (unsigned __int8 *)&from->objective[fieldNum],
                (unsigned __int8 *)&to->objective[fieldNum],
                0,
                6,
                objectiveFields);
        }
    }
    v15 = MSG_GetUsedBitCount(msg);
    SV_TrackPSObjectivesBits(v15 - v21);
    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
    if (!memcmp(&from->hud, &to->hud, sizeof(playerState_s_hud)))
    {
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "no hudelems changed\n");
        MSG_WriteBit0(msg);
    }
    else
    {
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "hudelems changed\n");
        MSG_WriteBit1(msg);
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "sending archived hudelems\n");
        MSG_WriteDeltaHudElems(snapInfo, msg, time, from->hud.archival, to->hud.archival, 0x1Fu);
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "sending current hudelems\n");
        MSG_WriteDeltaHudElems(snapInfo, msg, time, from->hud.current, to->hud.current, 0x1Fu);
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
    }
    v22 = MSG_GetUsedBitCount(msg);
    SV_TrackPSHudelemBits(v22 - v15);
    if (!memcmp(from->weaponmodels, to->weaponmodels, sizeof(playerState_s::weaponmodels)))
    {
        MSG_WriteBit0(msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "no weaponmodels changed\n");
    }
    else
    {
        MSG_WriteBit1(msg);
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
        if (sv_debugPacketContents->current.enabled)
            Com_Printf(16, "%s", "sending weaponmodels\n");
        for (fieldNum = 0; fieldNum < 128; ++fieldNum)
            MSG_WriteByte(msg, to->weaponmodels[fieldNum]);
    }
    v5 = MSG_GetUsedBitCount(msg);
    SV_TrackPSWeaponModelBits(v5 - v22);
    SV_TrackFieldsChanged(lastChangedFieldNum);
    v6 = MSG_GetUsedBitCount(msg);
    SV_TrackPSBits(v6 - UsedBitCount);
    //Profile_EndInternal(0);
}

bool __cdecl MSG_ShouldSendPSField(
    const SnapshotInfo_s *snapInfo,
    bool sendOriginAndVel,
    const playerState_s *ps,
    const playerState_s *oldPs,
    const NetField *field)
{
    if (field->bits == -87)
    {
        if (snapInfo->archived)
        {
            return 1;
        }
        else if ((ps->otherFlags & 2) != 0)
        {
            return 1;
        }
        else
        {
            return ((oldPs->eFlags ^ ps->eFlags) & 2) != 0 || ps->viewlocked_entNum != 1023 || ps->pm_type == 5;
        }
    }
    else if (field->changeHints != 3 || snapInfo->archived)
    {
        return !MSG_ValuesAreEqual(
            snapInfo,
            field->bits,
            (const int *)((char *)&oldPs->commandTime + field->offset),
            (const int *)((char *)&ps->commandTime + field->offset));
    }
    else
    {
        return sendOriginAndVel;
    }
}

void __cdecl MSG_WriteDeltaFields(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    unsigned __int8 *from,
    unsigned __int8 *to,
    int force,
    int numFields,
    const NetField *stateFields)
{
    int i; // [esp+Ch] [ebp-4h]
    int ia; // [esp+Ch] [ebp-4h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1261, 0, "%s", "!msg->readOnly");
    if (force)
    {
    any_different:
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        MSG_WriteBit1(msg);
        for (ia = 0; ia < numFields; ++ia)
        {
            SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
            MSG_WriteDeltaField(snapInfo, msg, time, from, to, &stateFields[ia], ia, 0);
        }
        SV_PacketDataIsUnknown(snapInfo->clientNum, msg);
    }
    else
    {
        for (i = 0; i < numFields; ++i)
        {
            if (!MSG_ValuesAreEqual(
                snapInfo,
                stateFields[i].bits,
                (const int *)&from[stateFields[i].offset],
                (const int *)&to[stateFields[i].offset]))
                goto any_different;
        }
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        MSG_WriteBit0(msg);
    }
}

void __cdecl MSG_WriteDeltaHudElems(
    SnapshotInfo_s *snapInfo,
    msg_t *msg,
    int time,
    const hudelem_s *from,
    const hudelem_s *to,
    unsigned int count)
{
    unsigned int bits; // [esp+4h] [ebp-28h]
    bool est; // [esp+Bh] [ebp-21h] BYREF
    int alignY; // [esp+Ch] [ebp-20h]
    int alignX; // [esp+10h] [ebp-1Ch]
    unsigned int j; // [esp+14h] [ebp-18h]
    unsigned int lc; // [esp+18h] [ebp-14h]
    int *toF; // [esp+1Ch] [ebp-10h]
    int *fromF; // [esp+20h] [ebp-Ch]
    unsigned int i; // [esp+24h] [ebp-8h]
    unsigned int inuse; // [esp+28h] [ebp-4h]

    if (msg->readOnly)
        MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1773, 0, "%s", "!msg->readOnly");
    if (count != 31)
        MyAssertHandler(
            ".\\qcommon\\sv_msg_write_mp.cpp",
            1774,
            0,
            "%s",
            "count == MAX_HUDELEMS_ARCHIVAL || count == MAX_HUDELEMS_CURRENT");
    for (inuse = 0; inuse < count && to[inuse].type; ++inuse)
        ;
    SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
    MSG_WriteBits(msg, inuse, 5u);
    for (i = 0; i < inuse; ++i)
    {
        if ((from[i].alignOrg & 0xFFFFFFF0) != 0)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                1792,
                0,
                "%s\n\t(from[i].alignOrg) = %i",
                "(!(from[i].alignOrg & ~15))",
                from[i].alignOrg);
        if ((to[i].alignOrg & 0xFFFFFFF0) != 0)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                1793,
                0,
                "%s\n\t(from[i].alignOrg) = %i",
                "(!(to[i].alignOrg & ~15))",
                from[i].alignOrg);
        alignX = (from[i].alignOrg >> 2) & 3;
        if ((unsigned int)alignX > 2)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                1796,
                0,
                "%s\n\t(from[i].alignOrg) = %i",
                "(alignX == 0 || alignX == 1 || alignX == 2)",
                from[i].alignOrg);
        alignY = from[i].alignOrg & 3;
        if ((unsigned int)alignY > 2)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                1799,
                0,
                "%s\n\t(from[i].alignOrg) = %i",
                "(alignY == 0 || alignY == 1 || alignY == 2)",
                from[i].alignOrg);
        alignX = (to[i].alignOrg >> 2) & 3;
        if ((unsigned int)alignX > 2)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                1802,
                0,
                "%s\n\t(to[i].alignOrg) = %i",
                "(alignX == 0 || alignX == 1 || alignX == 2)",
                to[i].alignOrg);
        alignY = to[i].alignOrg & 3;
        if ((unsigned int)alignY > 2)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                1805,
                0,
                "%s\n\t(to[i].alignOrg) = %i",
                "(alignY == 0 || alignY == 1 || alignY == 2)",
                to[i].alignOrg);
        lc = 0;
        for (j = 0; j < 0x28; ++j)
        {
            fromF = (int*)((char*)&from[i] + hudElemFields[j].offset);
            toF = (int*)((char*)&to[i] + hudElemFields[j].offset);
            if (!MSG_ValuesAreEqual(snapInfo, hudElemFields[j].bits, fromF, toF))
            {
                if (*fromF == *toF)
                    MyAssertHandler(".\\qcommon\\sv_msg_write_mp.cpp", 1815, 0, "%s", "*fromF != *toF");
                if (!snapInfo->archived && msg_hudelemspew->current.enabled)
                {
                    bits = MSG_GetBitCount(hudElemFields[j].bits, &est, *fromF, *toF);
                    if (est)
                        Com_Printf(
                            15,
                            "Hudelem #%i field '%s' changed from %i to %i, which will take %i bits%s\n",
                            i,
                            hudElemFields[j].name,
                            *fromF,
                            *toF,
                            bits,
                            "(est)");
                    else
                        Com_Printf(
                            15,
                            "Hudelem #%i field '%s' changed from %i to %i, which will take %i bits%s\n",
                            i,
                            hudElemFields[j].name,
                            *fromF,
                            *toF,
                            bits,
                            "");
                }
                SV_TrackFieldChange(snapInfo->clientNum, 21, j);
                lc = j;
            }
        }
        if (lc >= 0x28)
            MyAssertHandler(
                ".\\qcommon\\sv_msg_write_mp.cpp",
                1830,
                0,
                "%s\n\t(lc) = %i",
                "(lc >= 0 && lc < (sizeof( hudElemFields ) / (sizeof( hudElemFields[0] ) * (sizeof( hudElemFields ) != 4 || sizeo"
                "f( hudElemFields[0] ) <= 4))))",
                lc);
        SV_PacketDataIsOverhead(snapInfo->clientNum, msg);
        MSG_WriteBits(msg, lc, 6u);
        for (j = 0; j <= lc; ++j)
        {
            snapInfo->fieldChanges = orderInfo.hudElem;
            MSG_WriteDeltaField(
                snapInfo,
                msg,
                time,
                (const unsigned __int8 *)&from[i],
                (const unsigned __int8 *)&to[i],
                &hudElemFields[j],
                j,
                0);
        }
    }
}

