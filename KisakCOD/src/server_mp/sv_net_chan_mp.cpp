#include "server.h"

#include <cstring>
#include <qcommon/net_chan_mp.h>

void __cdecl SV_Netchan_Decode(client_t *client, unsigned __int8 *data, int size)
{
    int i, index;
    byte key, * string;

    iassert(client->reliableSequence - client->reliableAcknowledge < MAX_RELIABLE_COMMANDS);

    string = (byte*)client->reliableCommandInfo[client->reliableAcknowledge & (MAX_RELIABLE_COMMANDS - 1)].cmd;
    key = client->challenge ^ (byte)client->serverId ^ client->messageAcknowledge;

    for (i = 0, index = 0; i < size; i++)
    {
        if (!string[index])
        {
            index = 0;
        }

        iassert(string[index] != '%');

        // modify the key with the last sent and acknowledged server command
        key ^= string[index] << (i & 1);
        data[i] ^= key;

        index++;
    }
}

void __cdecl SV_Netchan_Encode(client_t *client, unsigned __int8 *data, int size)
{
    int i, index;
    byte key, * string;

    string = (byte*)client->lastClientCommandString;
    key = client->challenge ^ client->header.netchan.outgoingSequence;

    for (i = 0, index = 0; i < size; i++)
    {
        if (!string[index])
        {
            index = 0;
        }

        iassert(string[index] != '%');

        // modify the key with the last sent and acknowledged server command
        key ^= string[index] << (i & 1);
        data[i] ^= key;

        index++;
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

