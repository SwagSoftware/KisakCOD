#include "server.h"
#include <game_mp/g_main_mp.h>
#include <cgame_mp/cg_local_mp.h>


void __cdecl G_BroadcastVoice(gentity_s *talker, VoicePacket_t *voicePacket)
{
    float v2; // [esp+0h] [ebp-44h]
    float v3; // [esp+8h] [ebp-3Ch]
    float delta[3]; // [esp+1Ch] [ebp-28h] BYREF
    float dist; // [esp+28h] [ebp-1Ch]
    float forward[3]; // [esp+2Ch] [ebp-18h] BYREF
    gclient_s *client; // [esp+38h] [ebp-Ch]
    int otherPlayer; // [esp+3Ch] [ebp-8h]
    gentity_s *ent; // [esp+40h] [ebp-4h]

    talker->client->lastVoiceTime = level.time;
    for (otherPlayer = 0; otherPlayer < 64; ++otherPlayer)
    {
        ent = &g_entities[otherPlayer];
        client = ent->client;
        if (ent->r.inuse)
        {
            if (client)
            {
                if (client->sess.sessionState == SESS_STATE_INTERMISSION
                    || voice_global->current.enabled
                    || OnSameTeam(talker, ent)
                    || talker->client->sess.cs.team == TEAM_FREE
                    || (client->ps.perks & 0x200) != 0
                    && (AngleVectors(level_bgs.clientinfo[otherPlayer].playerAngles, forward, 0, 0),
                        Vec3Sub(talker->client->ps.origin, client->ps.origin, delta),
                        dist = Vec3Normalize(delta),
                        v3 = perk_parabolicAngle->current.value * 0.01745329238474369,
                        v2 = cos(v3),
                        v2 <= Vec3Dot(delta, forward))
                    && perk_parabolicRadius->current.value >= dist)
                {
                    if ((ent->client->sess.sessionState == talker->client->sess.sessionState
                        || (ent->client->sess.sessionState == SESS_STATE_DEAD
                            || talker->client->sess.sessionState == SESS_STATE_DEAD)
                        && voice_deadChat->current.enabled)
                        && (talker != ent || voice_localEcho->current.enabled)
                        && !SV_ClientHasClientMuted(otherPlayer, talker->s.number)
                        && SV_ClientWantsVoiceData(otherPlayer))
                    {
                        SV_QueueVoicePacket(talker->s.number, otherPlayer, voicePacket);
                    }
                }
            }
        }
    }
}

bool __cdecl SV_ClientHasClientMuted(unsigned int listener, unsigned int talker)
{
    if (listener >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_voice_mp.cpp",
            110,
            0,
            "%s\n\t(listener) = %i",
            "(listener >= 0 && listener < 64)",
            listener);
    if (talker >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_voice_mp.cpp",
            111,
            0,
            "%s\n\t(talker) = %i",
            "(talker >= 0 && talker < 64)",
            talker);
    return svs.clients[listener].muteList[talker];
}

bool __cdecl SV_ClientWantsVoiceData(unsigned int clientNum)
{
    if (clientNum >= 0x40)
        MyAssertHandler(
            ".\\server_mp\\sv_voice_mp.cpp",
            101,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 64)",
            clientNum);
    return svs.clients[clientNum].sendVoice;
}

void __cdecl SV_UserVoice(client_t *cl, msg_t *msg)
{
    int packet; // [esp+0h] [ebp-11Ch]
    VoicePacket_t voicePacket; // [esp+4h] [ebp-118h] BYREF
    int packetCount; // [esp+114h] [ebp-8h]
    int totalBytes; // [esp+118h] [ebp-4h]

    totalBytes = 0;
    if (sv_voice->current.enabled)
    {
        packetCount = MSG_ReadByte(msg);
        if (!cl->gentity)
            MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 177, 0, "%s", "cl->gentity");
        for (packet = 0; packet < packetCount; ++packet)
        {
            voicePacket.dataSize = MSG_ReadByte(msg);
            if (voicePacket.dataSize <= 0 || voicePacket.dataSize > 256)
            {
                Com_Printf(15, "Received invalid voice packet of size %i from %s\n", voicePacket.dataSize, cl->name);
                return;
            }
            if (!msg->data)
                MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 189, 0, "%s", "msg->data");
            if (&voicePacket == (VoicePacket_t *)-1)
                MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 190, 0, "%s", "voicePacket.data");
            MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);
            G_BroadcastVoice(cl->gentity, &voicePacket);
        }
    }
}

void __cdecl SV_QueueVoicePacket(int talkerNum, int clientNum, VoicePacket_t *voicePacket)
{
    client_t *client; // [esp+0h] [ebp-8h]

    if (talkerNum < 0)
        MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 126, 0, "%s", "talkerNum >= 0");
    if (clientNum < 0)
        MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 127, 0, "%s", "clientNum >= 0");
    if (talkerNum >= sv_maxclients->current.integer)
        MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 128, 0, "%s", "talkerNum < sv_maxclients->current.integer");
    if (clientNum >= sv_maxclients->current.integer)
        MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 129, 0, "%s", "clientNum < sv_maxclients->current.integer");
    client = &svs.clients[clientNum];
    if (client->voicePacketCount < 40)
    {
        client->voicePackets[client->voicePacketCount].dataSize = voicePacket->dataSize;
        memcpy(client->voicePackets[client->voicePacketCount].data, voicePacket->data, voicePacket->dataSize);
        if (talkerNum != talkerNum)
            MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 149, 0, "%s", "talkerNum == static_cast<byte>(talkerNum)");
        client->voicePackets[client->voicePacketCount++].talker = talkerNum;
    }
}

void __cdecl SV_PreGameUserVoice(client_t *cl, msg_t *msg)
{
    int packet; // [esp+0h] [ebp-124h]
    VoicePacket_t voicePacket; // [esp+4h] [ebp-120h] BYREF
    int otherPlayer; // [esp+114h] [ebp-10h]
    int talker; // [esp+118h] [ebp-Ch]
    int packetCount; // [esp+11Ch] [ebp-8h]
    int totalBytes; // [esp+120h] [ebp-4h]

    totalBytes = 0;
    if (sv_voice->current.enabled)
    {
        talker = cl - svs.clients;
        packetCount = MSG_ReadByte(msg);
        for (packet = 0; packet < packetCount; ++packet)
        {
            voicePacket.dataSize = MSG_ReadShort(msg);
            if (voicePacket.dataSize <= 0 || voicePacket.dataSize > 256)
            {
                Com_Printf(15, "Received invalid voice packet of size %i from %s\n", voicePacket.dataSize, cl->name);
                return;
            }
            if (!msg->data)
                MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 240, 0, "%s", "msg->data");
            if (&voicePacket == (VoicePacket_t *)-1)
                MyAssertHandler(".\\server_mp\\sv_voice_mp.cpp", 241, 0, "%s", "voicePacket.data");
            MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);
            for (otherPlayer = 0; otherPlayer < 64; ++otherPlayer)
            {
                if (otherPlayer != talker
                    && svs.clients[otherPlayer].header.state >= 2
                    && !SV_ClientHasClientMuted(otherPlayer, talker)
                    && SV_ClientWantsVoiceData(otherPlayer))
                {
                    SV_QueueVoicePacket(talker, otherPlayer, &voicePacket);
                }
            }
        }
    }
}