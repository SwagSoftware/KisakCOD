#include "server.h"

#include <cstring>
#include <qcommon/net_chan_mp.h>

void __cdecl SV_Netchan_Decode(client_t *client, unsigned __int8 *data, int size)
{
    svscmd_info_t *string; // [esp+0h] [ebp-10h]
    _BYTE key[5]; // [esp+7h] [ebp-9h]
    int i; // [esp+Ch] [ebp-4h]

    if (client->reliableSequence - client->reliableAcknowledge >= 128)
        MyAssertHandler(
            ".\\server_mp\\sv_net_chan_mp.cpp",
            62,
            0,
            "%s",
            "client->reliableSequence - client->reliableAcknowledge < MAX_RELIABLE_COMMANDS");
    string = &client->reliableCommandInfo[client->reliableAcknowledge & 0x7F];
    key[4] = 0;
    *key = (LOBYTE(client->messageAcknowledge) ^ LOBYTE(client->serverId) ^ LOBYTE(client->challenge));
    for (i = 0; i < size; ++i)
    {
        if (!string->cmd[*&key[1]])
            *&key[1] = 0;
        if (string->cmd[*&key[1]] == 37)
            MyAssertHandler(
                ".\\server_mp\\sv_net_chan_mp.cpp",
                73,
                0,
                "%s\n\t(client->reliableCommandInfo[client->reliableAcknowledge & (128 - 1)].cmd) = %s",
                "(string[index] != '%')",
                client->reliableCommandInfo[client->reliableAcknowledge & 0x7F].cmd);
        key[0] ^= string->cmd[(*&key[1])++] << (i & 1);
        *data++ ^= key[0];
    }
}

void __cdecl SV_Netchan_Encode(client_t *client, unsigned __int8 *data, int size)
{
    unsigned __int8 *string; // [esp+0h] [ebp-10h]
    _BYTE key[5]; // [esp+7h] [ebp-9h]
    int i; // [esp+Ch] [ebp-4h]

    string = (unsigned __int8 *)client->lastClientCommandString;
    key[4] = 0;
    *(_DWORD *)key = (unsigned __int8)(LOBYTE(client->header.netchan.outgoingSequence) ^ LOBYTE(client->challenge));
    for (i = 0; i < size; ++i)
    {
        if (!string[*(_DWORD *)&key[1]])
            *(_DWORD *)&key[1] = 0;
        if (string[*(_DWORD *)&key[1]] == 37)
            MyAssertHandler(
                ".\\server_mp\\sv_net_chan_mp.cpp",
                34,
                0,
                "%s\n\t(client->lastClientCommandString) = %s",
                "(string[index] != '%')",
                client->lastClientCommandString);
        key[0] ^= string[(*(_DWORD *)&key[1])++] << (i & 1);
        *data++ ^= key[0];
    }
}

void __cdecl SV_Netchan_OutgoingSequenceIncremented(client_t *client, netchan_t *chan)
{
    clientSnapshot_t *frame; // [esp+0h] [ebp-4h]

    frame = &client->frames[chan->outgoingSequence & 0x1F];
    memset(frame, 0, sizeof(clientSnapshot_t));
    frame->first_entity = svs.nextSnapshotEntities;
    frame->first_client = svs.nextSnapshotClients;
}

bool __cdecl SV_Netchan_TransmitNextFragment(client_t *client, netchan_t *chan)
{
    bool res; // [esp+3h] [ebp-1h]

    res = Netchan_TransmitNextFragment(chan);
    if (!chan->unsentFragments)
        SV_Netchan_OutgoingSequenceIncremented(client, chan);
    return res;
}

bool __cdecl SV_Netchan_Transmit(client_t *client, unsigned __int8 *data, int length)
{
    bool res; // [esp+3h] [ebp-1h]

    SV_Netchan_Encode(client, data + 4, length - 4);
    res = Netchan_Transmit(&client->header.netchan, length, (char *)data);
    if (!client->header.netchan.unsentFragments)
        SV_Netchan_OutgoingSequenceIncremented(client, &client->header.netchan);
    return res;
}

void __cdecl SV_Netchan_AddOOBProfilePacket(int iLength)
{
    if (net_profile->current.integer)
    {
        NetProf_PrepProfiling(&svs.OOBProf);
        NetProf_AddPacket(&svs.OOBProf.send, iLength, 0);
    }
}

void __cdecl SV_Netchan_UpdateProfileStats()
{
    client_t *pClient; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if (&svs != (serverStatic_t *)-4601868 && net_profile->current.integer)
    {
        NetProf_UpdateStatistics(&svs.OOBProf.send);
        NetProf_UpdateStatistics(&svs.OOBProf.recieve);
        i = 0;
        pClient = svs.clients;
        while (i < sv_maxclients->current.integer)
        {
            if (pClient->header.state)
            {
                NetProf_UpdateStatistics(&pClient->header.netchan.prof.send);
                NetProf_UpdateStatistics(&pClient->header.netchan.prof.recieve);
            }
            ++i;
            ++pClient;
        }
    }
}

