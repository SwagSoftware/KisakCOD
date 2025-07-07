#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "server.h"

void __cdecl SV_DirectConnect()
{
    client_t *clients; // r31

    Com_DPrintf(15, "SVC_DirectConnect ()\n");
    clients = svs.clients;
    memset(svs.clients, 0, sizeof(client_t));
    clients->gentity = SV_GentityNum(0);
    clients->netchan.outgoingSequence = 1;
    SV_InitReliableCommandsForClient(clients);
    clients->state = 1;
    clients->lastUsercmd.serverTime = G_GetTime();
}

void __cdecl SV_SendClientGameState(client_t *client)
{
    if (client != svs.clients)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_client.cpp", 64, 0, "%s", "client == svs.clients");
    CL_ParseGamestate((char *)sv.configstrings);
}

void __cdecl SV_SendGameState()
{
    CL_ParseGamestate((char *)sv.configstrings);
}

void __cdecl SV_ClientEnterWorld(client_t *client)
{
    if (client->state != 1)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_client.cpp", 87, 0, "%s", "client->state == CS_ACTIVE");
    ClientConnect(0);
    client->gentity = SV_GentityNum(client - svs.clients);
}

float __cdecl SV_FX_GetVisibility(const float *start, const float *end)
{
    double ServerVisibility; // fp31
    double v3; // fp1

    if (sv.demo.playing)
    {
        v3 = SV_DemoFxVisibility();
    }
    else
    {
        ServerVisibility = FX_GetServerVisibility(start, end);
        SV_RecordFxVisibility(ServerVisibility);
        v3 = ServerVisibility;
    }
    return *((float *)&v3 + 1);
}

void __cdecl SV_ExecuteClientCommand(const char *s)
{
    if (!SV_Loaded())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_client.cpp", 140, 0, "%s", "SV_Loaded()");
    if (svs.clients->state != 1)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_client.cpp",
            141,
            0,
            "%s",
            "svs.clients[0].state == CS_ACTIVE");
    ClientCommand(0, s);
}

void __cdecl SV_ClientThink(usercmd_s *cmd)
{
    client_t *clients; // r11

    if (!SV_Loaded())
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\server\\sv_client.cpp", 155, 0, "%s", "SV_Loaded()");
    clients = svs.clients;
    if (svs.clients->state != 1)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\server\\sv_client.cpp",
            156,
            0,
            "%s",
            "svs.clients[0].state == CS_ACTIVE");
        clients = svs.clients;
    }
    memcpy(&clients->lastUsercmd, cmd, sizeof(clients->lastUsercmd));
    ClientThink(0);
}

// attributes: thunk
gentity_s *__cdecl SV_GetEntityState(int entnum)
{
    return SV_GentityNum(entnum);
}

void __cdecl SV_TrackPlayerDied()
{
    Live_SetDeaths(++svs.playerDeaths);
}

void __cdecl SV_AddToPlayerScore(int amount)
{
    svs.playerScore += amount;
    Live_SetScore(svs.playerScore);
}

