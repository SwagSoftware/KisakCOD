#include "server.h"

netadr_t adr;

const netadr_t *__cdecl SV_MasterAddress()
{
    __int16 v1; // ax

    if (adr.type == NA_BOT)
    {
        Com_Printf(15, "Resolving %s\n", com_masterServerName->current.string);
        if (NET_StringToAdr((char *)com_masterServerName->current.integer, &adr))
        {
            const char* result = strstr(":", (char*)com_masterServerName->current.integer);
            if (!result)
                adr.port = BigShort(com_masterPort->current.integer);
            v1 = BigShort(adr.port);
            Com_Printf(
                15,
                "%s resolved to %i.%i.%i.%i:%i\n",
                com_masterServerName->current.string,
                adr.ip[0],
                adr.ip[1],
                adr.ip[2],
                adr.ip[3],
                v1);
            if (adr.type == NA_BOT)
                MyAssertHandler(".\\server_mp\\sv_main_pc_mp.cpp", 50, 0, "%s", "adr.type != 0");
        }
        else
        {
            if (adr.type != NA_BAD)
                MyAssertHandler(".\\server_mp\\sv_main_pc_mp.cpp", 42, 0, "%s", "adr.type == NA_BAD");
            Com_Printf(15, "Couldn't resolve address: %s\n", com_masterServerName->current.string);
        }
    }
    return &adr;
}

void __cdecl SV_MasterGameCompleteStatus()
{
    netadr_t *adr; // [esp+0h] [ebp-4h]

    if (com_dedicated && com_dedicated->current.integer == 2)
    {
        adr = (netadr_t *)SV_MasterAddress();
        if (!adr)
            MyAssertHandler(".\\server_mp\\sv_main_pc_mp.cpp", 125, 0, "%s", "adr");
        if (adr->type != NA_BAD)
        {
            Com_Printf(15, "Sending gameCompleteStatus to %s\n", com_masterServerName->current.string);
            SVC_GameCompleteStatus(*adr);
        }
    }
}

void __cdecl SV_MasterHeartbeat(const char *hbname)
{
    const char *v1; // eax
    netadr_t *adr; // [esp+0h] [ebp-4h]

    if (com_dedicated && com_dedicated->current.integer == 2 && svs.time >= svs.nextHeartbeatTime)
    {
        svs.nextHeartbeatTime = svs.time + 180000;
        adr = (netadr_t *)SV_MasterAddress();
        if (!adr)
            MyAssertHandler(".\\server_mp\\sv_main_pc_mp.cpp", 94, 0, "%s", "adr");
        if (adr->type != NA_BAD)
        {
            Com_Printf(15, "Sending heartbeat to %s\n", com_masterServerName->current.string);
            v1 = va("heartbeat %s\n", hbname);
            NET_OutOfBandPrint(NS_SERVER, *adr, v1);
        }
    }
}

void __cdecl SV_MasterShutdown()
{
    svs.nextHeartbeatTime = 0x80000000;
    SV_MasterHeartbeat("flatline");
}