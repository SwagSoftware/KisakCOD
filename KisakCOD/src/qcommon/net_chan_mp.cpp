#include "qcommon.h"


const dvar_t *showpackets;
const dvar_t *fakelag_target;
const dvar_t *fakelag_packetloss;
const dvar_t *fakelag_currentjitter;
const dvar_t *fakelag_jitter;
const dvar_t *fakelag_current;
const dvar_t *msg_dumpEnts;
const dvar_t *net_profile;
const dvar_t *net_lanauthorize;
const dvar_t *packetDebug;
const dvar_t *showdrop;
const dvar_t *fakelag_jitterinterval;
const dvar_t *net_showprofile;
const dvar_t *msg_printEntityNums;
const dvar_t *msg_hudelemspew;
// 
// struct fakedLatencyPackets_t *laggedPackets 82ff2720     net_chan_mp.obj
// int g_qport              83020760     net_chan_mp.obj
// BOOL fakelagInitialized  83020768     net_chan_mp.obj

int net_iProfilingOn;
const char *WeaponStateNames[27] =
{
  "WEAPON_READY",
  "WEAPON_RAISING",
  "WEAPON_RAISING_ALTSWITCH",
  "WEAPON_DROPPING",
  "WEAPON_DROPPING_QUICK",
  "WEAPON_FIRING",
  "WEAPON_RECHAMBERING",
  "WEAPON_RELOADING",
  "WEAPON_RELOADING_INTERUPT",
  "WEAPON_RELOAD_START",
  "WEAPON_RELOAD_START_INTERUPT",
  "WEAPON_RELOAD_END",
  "WEAPON_MELEE_INIT",
  "WEAPON_MELEE_FIRE",
  "WEAPON_MELEE_END",
  "WEAPON_OFFHAND_INIT",
  "WEAPON_OFFHAND_PREPARE",
  "WEAPON_OFFHAND_HOLD",
  "WEAPON_OFFHAND_START",
  "WEAPON_OFFHAND",
  "WEAPON_OFFHAND_END",
  "WEAPON_DETONATING",
  "WEAPON_SPRINT_RAISE",
  "WEAPON_SPRINT_LOOP",
  "WEAPON_SPRINT_DROP",
  "WEAPON_NIGHTVISION_WEAR",
  "WEAPON_NIGHTVISION_REMOVE"
}; // idb
const char *netsrcString[2] =
{
    "client1",
    "server"
};
const char *NET_AdrToString(netadr_t a)
{
    static	char	s[64];

    Com_sprintf(s, sizeof(s), "unknown");

    if (a.type == NA_LOOPBACK) {
        Com_sprintf(s, sizeof(s), "loopback");
    }
    else if (a.type == NA_BOT) {
        Com_sprintf(s, sizeof(s), "bot");
    }
    else if (a.type == NA_IP) {
        Com_sprintf(s, sizeof(s), "%i.%i.%i.%i:%i",
            a.ip[0], a.ip[1], a.ip[2], a.ip[3], BigShort(a.port));
    }
    else if (a.type == NA_BAD) {
        Com_sprintf(s, sizeof(s), "BAD");
    }
    else {
        Com_sprintf(s, sizeof(s), "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%i",
            a.ipx[0], a.ipx[1], a.ipx[2], a.ipx[3], a.ipx[4], a.ipx[5], a.ipx[6], a.ipx[7], a.ipx[8], a.ipx[9],
            BigShort(a.port));
    }

    return s;
}

void __cdecl NetProf_PrepProfiling(netProfileInfo_t *prof)
{
    if (net_profile->current.integer)
    {
        if (!net_iProfilingOn)
        {
            if (!com_sv_running->current.enabled || CL_AnyLocalClientsRunning() && net_profile->current.integer == 2)
                net_iProfilingOn = 1;
            else
                net_iProfilingOn = 2;
            Com_Printf(16, "Net Profiling turned on: %s\n", WeaponStateNames[net_iProfilingOn + 26]);
            memset((unsigned __int8 *)prof, 0, sizeof(netProfileInfo_t));
        }
    }
    else if (net_iProfilingOn)
    {
        net_iProfilingOn = 0;
        Com_Printf(16, "Net Profiling turned off\n");
        memset((unsigned __int8 *)prof, 0, sizeof(netProfileInfo_t));
    }
}

void __cdecl NetProf_AddPacket(netProfileStream_t *pProfStream, int iSize, int bFragment)
{
    netProfilePacket_t *pPacket; // [esp+0h] [ebp-4h]

    if (!net_iProfilingOn)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 242, 0, "%s", "net_iProfilingOn");
    pProfStream->iCurrPacket = (pProfStream->iCurrPacket + 1) % 60;
    pPacket = &pProfStream->packets[pProfStream->iCurrPacket];
    pPacket->iTime = Sys_Milliseconds();
    pPacket->iSize = iSize;
    pPacket->bFragment = bFragment;
}

void __cdecl NetProf_NewSendPacket(netchan_t *pChan, int iSize, int bFragment)
{
    if (net_iProfilingOn)
    {
        NetProf_AddPacket(&pChan->prof.send, iSize, bFragment);
        if ((net_showprofile->current.integer & 2) != 0)
        {
            if (bFragment)
                Com_Printf(16, "[%s] send%s: %i\n", netsrcString[pChan->sock], " fragment", iSize);
            else
                Com_Printf(16, "[%s] send%s: %i\n", netsrcString[pChan->sock], "", iSize);
        }
    }
}

void __cdecl NetProf_NewRecievePacket(netchan_t *pChan, int iSize, int bFragment)
{
    if (net_iProfilingOn)
    {
        NetProf_AddPacket(&pChan->prof.recieve, iSize, bFragment);
        if ((net_showprofile->current.integer & 2) != 0)
        {
            if (bFragment)
                Com_Printf(16, "[%s] recieve%s: %i\n", netsrcString[pChan->sock], " fragment", iSize);
            else
                Com_Printf(16, "[%s] recieve%s: %i\n", netsrcString[pChan->sock], "", iSize);
        }
    }
}

void __cdecl NetProf_UpdateStatistics(netProfileStream_t *pStream)
{
    int v1; // esi
    int iTimeSpan; // [esp+4h] [ebp-2Ch]
    int iTotalBytes; // [esp+8h] [ebp-28h]
    int iLargestSize; // [esp+Ch] [ebp-24h]
    int iOldestPacket; // [esp+10h] [ebp-20h]
    int iSmallestSize; // [esp+14h] [ebp-1Ch]
    int iNumFragments; // [esp+18h] [ebp-18h]
    int i; // [esp+1Ch] [ebp-14h]
    int iNumPackets; // [esp+20h] [ebp-10h]
    int iOldestPacketTime; // [esp+2Ch] [ebp-4h]

    if (!pStream)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 313, 0, "%s", "pStream");
    if (!net_iProfilingOn)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 314, 0, "%s", "net_iProfilingOn");
    iNumPackets = 0;
    iNumFragments = 0;
    iOldestPacket = -1;
    iOldestPacketTime = Sys_Milliseconds();
    iTotalBytes = 0;
    iSmallestSize = 9999;
    iLargestSize = 0;
    for (i = 0; i < 60; ++i)
    {
        if (pStream->packets[i].iTime && (int)Sys_Milliseconds() <= pStream->packets[i].iTime + 1000)
        {
            ++iNumPackets;
            if (pStream->packets[i].bFragment)
                ++iNumFragments;
            if (pStream->packets[i].iTime < iOldestPacketTime)
            {
                iOldestPacket = i;
                iOldestPacketTime = pStream->packets[i].iTime;
            }
            iTotalBytes += pStream->packets[i].iSize;
            if (pStream->packets[i].iSize < iSmallestSize)
                iSmallestSize = pStream->packets[i].iSize;
            if (pStream->packets[i].iSize > iLargestSize)
                iLargestSize = pStream->packets[i].iSize;
        }
    }
    if (iNumPackets)
    {
        if (iNumFragments)
            pStream->iFragmentPercentage = 100 * iNumFragments / iNumPackets;
        else
            pStream->iFragmentPercentage = 0;
        pStream->iLargestPacket = iLargestSize;
        pStream->iSmallestPacket = iSmallestSize;
        v1 = pStream->iLastBPSCalcTime + 100;
        if (v1 < (int)Sys_Milliseconds())
        {
            iTimeSpan = Sys_Milliseconds() - iOldestPacketTime;
            if (iOldestPacket != -1)
            {
                iTotalBytes -= pStream->packets[iOldestPacket].iSize;
                --iNumPackets;
                if (pStream->packets[iOldestPacket].bFragment)
                    --iNumFragments;
            }
            if (iTimeSpan >= 1 && iNumPackets)
            {
                if (iTotalBytes)
                    pStream->iBytesPerSecond = (int)((double)iTotalBytes / ((double)iTimeSpan * 0.001000000047497451));
                else
                    pStream->iBytesPerSecond = 0;
                pStream->iLastBPSCalcTime = Sys_Milliseconds();
            }
            else
            {
                pStream->iBytesPerSecond = 0;
            }
        }
        pStream->iCountedPackets = iNumPackets;
        pStream->iCountedFragments = iNumFragments;
    }
    else
    {
        pStream->iBytesPerSecond = 0;
        pStream->iLastBPSCalcTime = 0;
        pStream->iCountedPackets = 0;
        pStream->iCountedFragments = 0;
        pStream->iFragmentPercentage = 0;
        pStream->iLargestPacket = 0;
        pStream->iSmallestPacket = 0;
    }
}

void __cdecl Net_DisplayProfile(int localClientNum)
{
    if (!net_profile->current.integer)
        Dvar_SetInt((dvar_s *)net_profile, 2 - com_sv_running->current.enabled);
    if (net_iProfilingOn)
    {
        if (net_iProfilingOn == 1)
            CL_Netchan_PrintProfileStats(localClientNum, 0);
        else
            SV_Netchan_PrintProfileStats(0);
    }
}

char __cdecl FakeLag_DestroyPacket(unsigned int packet)
{
    Z_VirtualFree((void *)dword_2321360[20 * packet]);
    dword_2321360[20 * packet] = 0;
    dword_2321370[20 * packet] = 0;
    dword_232135C[20 * packet] = 0;
    dword_232137C[20 * packet] = 0;
    return 1;
}

void __cdecl FakeLag_SendPacket_Real(unsigned int packet)
{
    if (packet >= 0x200)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 541, 0, "%s", "packet < FAKELATENCY_MAX_PACKETS_HELD");
    if (!laggedPackets[packet].outbound)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 542, 0, "%s", "laggedPackets[ packet ].outbound");
    if (!dword_2321360[20 * packet])
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 543, 0, "%s", "laggedPackets[ packet ].data");
    NET_SendPacket(
        (netsrc_t)dword_2321344[20 * packet],
        dword_232135C[20 * packet],
        (unsigned __int8 *)dword_2321360[20 * packet],
        *((netadr_t *)&unk_2321348 + 4 * packet));
    FakeLag_DestroyPacket(packet);
}

void __cdecl FakeLag_Init()
{
    if (!fakelagInitialized)
    {
        memset((unsigned __int8 *)laggedPackets, 0, sizeof(laggedPackets));
        fakelagInitialized = 1;
    }
}

unsigned int __cdecl FakeLag_GetFreeSlot()
{
    int packet; // [esp+0h] [ebp-Ch]
    int packeta; // [esp+0h] [ebp-Ch]
    unsigned int oldest; // [esp+4h] [ebp-8h]
    signed int oldestTime; // [esp+8h] [ebp-4h]

    for (packet = 0; packet < 512; ++packet)
    {
        if (!dword_232135C[20 * packet])
            return packet;
    }
    oldest = 0;
    oldestTime = Sys_Milliseconds();
    for (packeta = 0; packeta < 512; ++packeta)
    {
        if (laggedPackets[packeta].outbound && dword_2321364[20 * packeta] < oldestTime)
        {
            oldest = packeta;
            oldestTime = dword_2321364[20 * packeta];
        }
    }
    Com_Printf(16, "fake lag buffer is full, you should increase FAKELATENCY_MAX_PACKETS_HELD or reduce your latency\n");
    if (laggedPackets[oldest].outbound)
        FakeLag_SendPacket_Real(oldest);
    else
        FakeLag_DestroyPacket(oldest);
    return oldest;
}

bool __cdecl FakeLag_HostingGameOrParty()
{
    return com_sv_running->current.enabled;
}

unsigned int __cdecl FakeLag_SendPacket(netsrc_t sock, int length, unsigned __int8 *data, netadr_t to)
{
    __int64 v4; // rax
    const char *v6; // [esp+Ch] [ebp-28h]
    const char *v7; // [esp+10h] [ebp-24h]
    int v8; // [esp+14h] [ebp-20h]
    int jitter; // [esp+1Ch] [ebp-18h]
    unsigned int slot; // [esp+20h] [ebp-14h]
    unsigned intnow; // [esp+24h] [ebp-10h]
    int change; // [esp+28h] [ebp-Ch]

    if (length <= 0)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 676, 0, "%s", "length > 0");
    if (!data)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 677, 0, "%s", "data != NULL");
    now = Sys_Milliseconds();
    if (fakelag_jitter->current.integer + fakelag_target->current.integer != fakelag_current->current.integer)
    {
        v4 = fakelag_current->current.integer - (fakelag_currentjitter->current.integer + fakelag_target->current.integer);
        if ((int)((HIDWORD(v4) ^ v4) - HIDWORD(v4)) < (int)(now - lastCall))
            v8 = (HIDWORD(v4) ^ v4) - HIDWORD(v4);
        else
            v8 = now - lastCall;
        change = v8;
        if (fakelag_currentjitter->current.integer + fakelag_target->current.integer < fakelag_current->current.integer)
            change = -v8;
        Dvar_SetInt((dvar_s *)fakelag_current, change + fakelag_current->current.integer);
    }
    if (fakelag_packetloss->current.value > 0.0 && fakelag_packetloss->current.value >= flrand(0.0, 1.0))
        return -1;
    if (fakelagInitialized
        && (fakelag_current->current.integer || fakelag_currentjitter->current.integer)
        && (!FakeLag_HostingGameOrParty() || to.type != NA_IP))
    {
        slot = FakeLag_GetFreeSlot();
        laggedPackets[slot].outbound = 1;
        dword_2321344[20 * slot] = sock;
        byte_2321341[80 * slot] = to.type == NA_LOOPBACK;
        *((netadr_t *)&unk_2321348 + 4 * slot) = to;
        dword_232135C[20 * slot] = length;
        dword_2321360[20 * slot] = (int)Z_VirtualAlloc(length, "FakeLag_SendPacket", 0);
        dword_2321370[20 * slot] = dword_2321360[20 * slot];
        memcpy((unsigned __int8 *)dword_2321360[20 * slot], data, length);
        if (fakelag_jitter->current.integer)
            jitter = irand(0, lastCall - now);
        else
            jitter = 0;
        dword_2321364[20 * slot] = jitter + Sys_Milliseconds();
        if (showpackets->current.integer && (showpackets->current.integer > 1 || !byte_2321341[80 * slot]))
        {
            if (sock == NS_SERVER)
            {
                v7 = "server";
            }
            else
            {
                if (sock >= NS_SERVER)
                    v6 = "";
                else
                    v6 = "client";
                v7 = v6;
            }
            if (byte_2321341[80 * slot])
                Com_Printf(16, "[%i] adding outbound %s packet for %s\n", now, "loopback", v7);
            else
                Com_Printf(16, "[%i] adding outbound %s packet for %s\n", now, "network", v7);
        }
        lastCall = now;
        return slot;
    }
    else if (NET_SendPacket(sock, length, data, to))
    {
        return -1;
    }
    else
    {
        return -2;
    }
}

unsigned int __cdecl FakeLag_QueueIncomingPacket(bool loopback, netsrc_t sock, netadr_t *from, msg_t *msg)
{
    __int64 v4; // rax
    const char *v6; // [esp+14h] [ebp-24h]
    const char *v7; // [esp+18h] [ebp-20h]
    int v8; // [esp+1Ch] [ebp-1Ch]
    int jitter; // [esp+24h] [ebp-14h]
    unsigned int slot; // [esp+28h] [ebp-10h]
    unsigned intnow; // [esp+2Ch] [ebp-Ch]
    int change; // [esp+30h] [ebp-8h]

    if (msg->cursize <= 0)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 775, 0, "%s", "msg->cursize > 0");
    if (!msg->data)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 776, 0, "%s", "msg->data != NULL");
    now = Sys_Milliseconds();
    if (fakelag_jitter->current.integer + fakelag_target->current.integer != fakelag_current->current.integer)
    {
        v4 = fakelag_current->current.integer - (fakelag_currentjitter->current.integer + fakelag_target->current.integer);
        if ((int)((HIDWORD(v4) ^ v4) - HIDWORD(v4)) < (int)(now - lastCall_0))
            v8 = (HIDWORD(v4) ^ v4) - HIDWORD(v4);
        else
            v8 = now - lastCall_0;
        change = v8;
        if (fakelag_currentjitter->current.integer + fakelag_target->current.integer < fakelag_current->current.integer)
            change = -v8;
        Dvar_SetInt((dvar_s *)fakelag_current, change + fakelag_current->current.integer);
    }
    if (fakelag_packetloss->current.value > 0.0 && fakelag_packetloss->current.value >= flrand(0.0, 1.0))
        return -1;
    slot = FakeLag_GetFreeSlot();
    laggedPackets[slot].outbound = 0;
    byte_2321341[80 * slot] = loopback;
    dword_2321344[20 * slot] = sock;
    *((netadr_t *)&unk_2321348 + 4 * slot) = *from;
    dword_232135C[20 * slot] = msg->cursize;
    dword_2321360[20 * slot] = (int)Z_VirtualAlloc(msg->cursize, "FakeLag_SendPacket", 0);
    if (!dword_2321360[20 * slot])
        return -1;
    memcpy((unsigned __int8 *)dword_2321360[20 * slot], msg->data, msg->cursize);
    memcpy(&dword_2321368[20 * slot], msg, 0x28u);
    dword_2321370[20 * slot] = dword_2321360[20 * slot];
    if (fakelag_jitter->current.integer)
        jitter = irand(0, lastCall_0 - now);
    else
        jitter = 0;
    dword_2321364[20 * slot] = jitter + Sys_Milliseconds();
    if (showpackets->current.integer && (showpackets->current.integer > 1 || !byte_2321341[80 * slot]))
    {
        if (sock == NS_SERVER)
        {
            v7 = "server";
        }
        else
        {
            if (sock >= NS_SERVER)
                v6 = "";
            else
                v6 = "client";
            v7 = v6;
        }
        if (loopback)
            Com_Printf(16, "[%i] adding incoming %s packet for %s\n", now, "loopback", v7);
        else
            Com_Printf(16, "[%i] adding incoming %s packet for %s\n", now, "network", v7);
    }
    lastCall_0 = now;
    return slot;
}

void __cdecl FakeLag_ReceivePackets()
{
    netadr_t from; // [esp+0h] [ebp-44h] BYREF
    msg_t msg; // [esp+18h] [ebp-2Ch] BYREF
    int clientNum; // [esp+40h] [ebp-4h]

    if (fakelag_current->current.integer)
    {
        MSG_Init(&msg, tempNetchanPacketBuf, 0x20000);
        while (NET_GetLoopPacket_Real(NS_SERVER, &from, &msg))
            FakeLag_QueueIncomingPacket(1, NS_SERVER, &from, &msg);
        MSG_Init(&msg, tempNetchanPacketBuf, 0x20000);
        for (clientNum = 0; clientNum < 1; ++clientNum)
        {
            while (NET_GetLoopPacket_Real((netsrc_t)clientNum, &from, &msg))
                FakeLag_QueueIncomingPacket(1, (netsrc_t)clientNum, &from, &msg);
        }
        MSG_Init(&msg, tempNetchanPacketBuf, 0x20000);
        while (Sys_GetPacket(&from, &msg))
            FakeLag_QueueIncomingPacket(0, NS_PACKET, &from, &msg);
    }
}

int __cdecl FakeLag_GetPacket(bool loopback, netsrc_t sock, netadr_t *net_from, msg_t *net_message)
{
    signed int packet; // [esp+0h] [ebp-8h]
    signed int now; // [esp+4h] [ebp-4h]

    now = Sys_Milliseconds();
    for (packet = 0; ; ++packet)
    {
        if (packet >= 512)
            return 0;
        if (dword_232135C[20 * packet]
            && !laggedPackets[packet].outbound
            && dword_2321344[20 * packet] == sock
            && byte_2321341[80 * packet] == loopback
            && (FakeLag_HostingGameOrParty() && !byte_2321341[80 * packet]
                || dword_2321364[20 * packet] + fakelag_current->current.integer / 2 < now))
        {
            break;
        }
    }
    if (showpackets->current.integer && (showpackets->current.integer > 1 || !byte_2321341[80 * packet]))
        Com_Printf(
            16,
            "[%i] delivering incoming packet from %i (time: %i) (%ims latency)\n",
            now,
            dword_2321364[20 * packet],
            dword_2321364[20 * packet],
            now - dword_2321364[20 * packet]);
    *net_from = *(netadr_t *)((_BYTE *)&unk_2321348 + 4 * packet);
    net_message->bit = dword_2321388[20 * packet];
    net_message->cursize = dword_232137C[20 * packet];
    net_message->maxsize = dword_2321378[20 * packet];
    net_message->overflowed = dword_2321368[20 * packet];
    net_message->readcount = dword_2321384[20 * packet];
    memcpy(net_message->data, (unsigned __int8 *)dword_2321360[20 * packet], dword_232135C[20 * packet]);
    FakeLag_DestroyPacket(packet);
    return 1;
}

void __cdecl FakeLag_Frame()
{
    double v0; // st7
    int v1; // esi
    double v2; // [esp+Ch] [ebp-18h]
    double integer; // [esp+18h] [ebp-Ch]
    signed int now; // [esp+20h] [ebp-4h]

    now = Sys_Milliseconds();
    if (fakelag_jitter->current.integer > 0 && now > nextChangeTime)
    {
        integer = (double)fakelag_jitter->current.integer;
        v0 = flrand(0.0, 1.0) * integer;
        Dvar_SetInt((dvar_s *)fakelag_currentjitter, (int)v0);
        v1 = fakelag_jitterinterval->current.integer + now;
        v2 = (double)(2 * fakelag_jitterinterval->current.integer);
        nextChangeTime = (int)(flrand(0.0, 1.0) * v2) + v1;
    }
    FakeLag_ReceivePackets();
    FakeLag_SendLaggedPackets();
}

int __cdecl FakeLag_SendLaggedPackets()
{
    unsigned intv1; // eax
    int v2; // [esp-8h] [ebp-24h]
    unsigned intv3; // [esp-4h] [ebp-20h]
    const char *v4; // [esp+0h] [ebp-1Ch]
    const char *v5; // [esp+4h] [ebp-18h]
    const char *v6; // [esp+8h] [ebp-14h]
    signed int packet; // [esp+Ch] [ebp-10h]
    int loopbackPacketTime; // [esp+10h] [ebp-Ch]
    signed int networkPacketTime; // [esp+14h] [ebp-8h]
    int numSent; // [esp+18h] [ebp-4h]

    if (!fakelagInitialized)
        return 0;
    loopbackPacketTime = Sys_Milliseconds() - fakelag_current->current.integer / 2;
    if (FakeLag_HostingGameOrParty())
        networkPacketTime = Sys_Milliseconds();
    else
        networkPacketTime = Sys_Milliseconds() - fakelag_current->current.integer / 2;
    numSent = 0;
    for (packet = 0; packet < 512; ++packet)
    {
        if (dword_232135C[20 * packet]
            && laggedPackets[packet].outbound
            && (byte_2321341[80 * packet] && dword_2321364[20 * packet] <= loopbackPacketTime
                || !byte_2321341[80 * packet] && dword_2321364[20 * packet] <= networkPacketTime))
        {
            if (showpackets->current.integer && (showpackets->current.integer > 1 || !byte_2321341[80 * packet]))
            {
                if (dword_2321344[20 * packet] == 1)
                {
                    v6 = "server";
                }
                else
                {
                    if (dword_2321344[20 * packet] >= 1)
                        v5 = "";
                    else
                        v5 = "client";
                    v6 = v5;
                }
                if (byte_2321341[80 * packet])
                    v4 = "loopback";
                else
                    v4 = "network";
                v3 = Sys_Milliseconds() - dword_2321364[20 * packet];
                v2 = dword_2321364[20 * packet];
                v1 = Sys_Milliseconds();
                Com_Printf(16, "[%i] delivering outbound %s packet for %s (from %i) (%ims delay)\n", v1, v4, v6, v2, v3);
            }
            FakeLag_SendPacket_Real(packet);
            ++numSent;
        }
    }
    return numSent;
}

void __cdecl FakeLag_Shutdown()
{
    signed int packet; // [esp+0h] [ebp-4h]

    if (fakelagInitialized)
    {
        for (packet = 0; packet < 512; ++packet)
        {
            if (dword_232135C[20 * packet])
            {
                if (laggedPackets[packet].outbound)
                    FakeLag_SendPacket_Real(packet);
                else
                    FakeLag_DestroyPacket(packet);
            }
        }
        fakelagInitialized = 0;
    }
}

void __cdecl Netchan_Init(__int16 port)
{
    DvarLimits min; // [esp+4h] [ebp-10h]

    showpackets = Dvar_RegisterInt("showpackets", 0, (DvarLimits)0x200000000LL, 0, "Show packets");
    showdrop = Dvar_RegisterBool("showdrop", 0, 0, "Show dropped packets");
    packetDebug = Dvar_RegisterBool("packetDebug", 0, 0, "Enable packet debugging information");
    g_qport = port;
    net_profile = Dvar_RegisterInt("net_profile", 0, (DvarLimits)0x200000000LL, 0, "Profile network performance");
    net_showprofile = Dvar_RegisterInt(
        "net_showprofile",
        0,
        (DvarLimits)0x300000000LL,
        0,
        "Show network profiling display");
    net_lanauthorize = Dvar_RegisterBool("net_lanauthorize", 0, 0, "Authorise CD keys when using a LAN");
    msg_printEntityNums = Dvar_RegisterBool("msg_printEntityNums", 0, 0, "Print entity numbers");
    msg_dumpEnts = Dvar_RegisterBool("msg_dumpEnts", 0, 0, "Print snapshot entity info");
    msg_hudelemspew = Dvar_RegisterBool("msg_hudelemspew", 0, 0, "Debug hudelem fields changing");
    fakelag_current = Dvar_RegisterInt("fakelag_current", 0, (DvarLimits)0x3E700000000LL, 0, "Current fake lag value");
    fakelag_target = Dvar_RegisterInt(
        "fakelag_target",
        0,
        (DvarLimits)0x3E700000000LL,
        0,
        "Target value for lag debugging");
    fakelag_currentjitter = Dvar_RegisterInt(
        "fakelag_currentjitter",
        0,
        (DvarLimits)0x3E700000000LL,
        0,
        "Current jitter amount for lag debugging");
    fakelag_jitter = Dvar_RegisterInt(
        "fakelag_jitter",
        0,
        (DvarLimits)0x3E700000000LL,
        0,
        "Amount of jitter for lag debugging");
    fakelag_jitterinterval = Dvar_RegisterInt(
        "fakelag_jitterinterval",
        2000,
        (DvarLimits)0xEA6000000000LL,
        0,
        "jitter interval for lag debugging");
    min.value.max = 1.0;
    min.value.min = 0.0;
    fakelag_packetloss = Dvar_RegisterFloat("fakelag_packetloss", 0.0, min, 0, "Packet loss for lag debugging");
    FakeLag_Init();
    Cmd_AddCommandInternal("net_dumpprofile", Net_Dump//Profile_f, &Net_Dump//Profile_f_VAR);
        Cmd_AddCommandInternal("net_dumpnetfieldchanges", MSG_DumpNetFieldChanges_f, &MSG_DumpNetFieldChanges_f_VAR);
    Cmd_AddCommandInternal("getqport", Net_GetQPort_f, &Net_GetQPort_f_VAR);
    Cmd_AddCommandInternal("setqport", Net_SetQPort_f, &Net_SetQPort_f_VAR);
}

void __cdecl Net_Dump//Profile_f()
{
  if (net_iProfilingOn)
  {
    if (net_iProfilingOn == 1)
      CL_Netchan_PrintProfileStats(0, 1);
    else
      SV_Netchan_PrintProfileStats(1);
  }
  else
  {
    Com_Printf(0, "Network profiling is not on. Set net_profile to turn on network profiling\n");
  }
}

void __cdecl Netchan_Setup(
    netsrc_t sock,
    netchan_t *chan,
    netadr_t adr,
    int qport,
    char *outgoingBuffer,
    int outgoingBufferSize,
    char *incomingBuffer,
    int incomingBufferSize)
{
    memset((unsigned __int8 *)chan, 0, sizeof(netchan_t));
    chan->sock = sock;
    chan->remoteAddress = adr;
    chan->qport = qport;
    if (adr.type && !qport)
        MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 1096, 0, "%s", "adr.type == NA_BOT || qport != 0");
    chan->incomingSequence = 0;
    chan->outgoingSequence = 1;
    chan->unsentBuffer = (unsigned __int8 *)outgoingBuffer;
    chan->unsentBufferSize = outgoingBufferSize;
    chan->fragmentBuffer = (unsigned __int8 *)incomingBuffer;
    chan->fragmentBufferSize = incomingBufferSize;
    NetProf_PrepProfiling(&chan->prof);
}

bool __cdecl Netchan_TransmitNextFragment(netchan_t *chan)
{
    int fragmentLength; // [esp+30h] [ebp-5ACh]
    msg_t send; // [esp+34h] [ebp-5A8h] BYREF
    unsigned __int8 send_buf[1400]; // [esp+5Ch] [ebp-580h] BYREF
    int res; // [esp+5D8h] [ebp-4h]

    //Profile_Begin(299);
    NetProf_PrepProfiling(&chan->prof);
    MSG_Init(&send, send_buf, 1400);
    MSG_WriteLong(&send, chan->outgoingSequence | 0x80000000);
    if (chan->sock < NS_SERVER)
        MSG_WriteShort(&send, chan->qport);
    fragmentLength = 1300;
    if (chan->unsentFragmentStart + 1300 > chan->unsentLength)
        fragmentLength = chan->unsentLength - chan->unsentFragmentStart;
    MSG_WriteLong(&send, chan->unsentFragmentStart);
    MSG_WriteShort(&send, fragmentLength);
    MSG_WriteData(&send, &chan->unsentBuffer[chan->unsentFragmentStart], fragmentLength);
    res = (int)FakeLag_SendPacket(chan->sock, send.cursize, send.data, chan->remoteAddress) >= -1;
    NetProf_NewSendPacket(chan, send.cursize, 1);
    if (showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK))
        Com_Printf(
            16,
            "[%s] send %4i : s=%i fragment=%i,%i\n",
            netsrcString[chan->sock],
            send.cursize,
            chan->outgoingSequence - 1,
            chan->unsentFragmentStart,
            fragmentLength);
    chan->unsentFragmentStart += fragmentLength;
    if (chan->unsentFragmentStart == chan->unsentLength && fragmentLength != 1300)
    {
        ++chan->outgoingSequence;
        chan->unsentFragments = 0;
    }
    //Profile_EndInternal(0);
    return res > 0;
}

bool __cdecl Netchan_Transmit(netchan_t *chan, int length, char *data)
{
    int file; // [esp+4Ch] [ebp-5ACh]
    msg_t send; // [esp+50h] [ebp-5A8h] BYREF
    unsigned __int8 send_buf[1400]; // [esp+78h] [ebp-580h] BYREF
    int res; // [esp+5F4h] [ebp-4h]

    //Profile_Begin(299);
    if (length > 0x20000)
    {
        file = FS_FOpenFileWrite("badpacket.dat");
        if (file)
        {
            FS_Write(data, length, file);
            FS_FCloseFile(file);
        }
        Com_Error(ERR_DROP, &byte_89AA90, length);
    }
    chan->unsentFragmentStart = 0;
    if (length < 1300)
    {
        NetProf_PrepProfiling(&chan->prof);
        MSG_Init(&send, send_buf, 1400);
        MSG_WriteLong(&send, chan->outgoingSequence);
        ++chan->outgoingSequence;
        if (chan->sock < NS_SERVER)
            MSG_WriteShort(&send, chan->qport);
        if (packetDebug->current.enabled)
            Com_Printf(16, "Adding %i byte payload to packet\n", length);
        MSG_WriteData(&send, (unsigned __int8 *)data, length);
        if (packetDebug->current.enabled)
            Com_Printf(16, "Sending %i byte packet\n", send.cursize);
        res = (int)FakeLag_SendPacket(chan->sock, send.cursize, send.data, chan->remoteAddress) >= -1;
        NetProf_NewSendPacket(chan, send.cursize, 0);
        if (showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK))
            Com_Printf(
                16,
                "[%s] send->%u.%u.%u.%u (%4i bytes) : s=%i ack=%i\n",
                netsrcString[chan->sock],
                chan->remoteAddress.ip[0],
                chan->remoteAddress.ip[1],
                chan->remoteAddress.ip[2],
                chan->remoteAddress.ip[3],
                send.cursize,
                chan->outgoingSequence - 1,
                chan->incomingSequence);
        //Profile_EndInternal(0);
        return res > 0;
    }
    else
    {
        chan->unsentFragments = 1;
        chan->unsentLength = length;
        if (chan->unsentBufferSize <= length)
            MyAssertHandler(
                ".\\qcommon\\net_chan_mp.cpp",
                1228,
                0,
                "%s\n\t(length) = %i",
                "(chan->unsentBufferSize > length)",
                length);
        Com_Memcpy((char *)chan->unsentBuffer, data, length);
        Netchan_TransmitNextFragment(chan);
        //Profile_EndInternal(0);
        return 1;
    }
}

int __cdecl Netchan_Process(netchan_t *chan, msg_t *msg)
{
    char *v2; // eax
    char *v4; // eax
    char *v5; // eax
    char *v6; // eax
    char *v7; // eax
    int dropped; // [esp-8h] [ebp-1Ch]
    int incomingSequence; // [esp-4h] [ebp-18h]
    int v10; // [esp-4h] [ebp-18h]
    signed int fragmentLength; // [esp+0h] [ebp-14h]
    int fragmented; // [esp+4h] [ebp-10h]
    int sequence; // [esp+8h] [ebp-Ch]
    int fragmentStart; // [esp+10h] [ebp-4h]

    NetProf_PrepProfiling(&chan->prof);
    MSG_BeginReading(msg);
    sequence = MSG_ReadLong(msg);
    if (sequence < 0)
    {
        sequence &= ~0x80000000;
        fragmented = 1;
    }
    else
    {
        fragmented = 0;
    }
    if (chan->sock == NS_SERVER)
        MSG_ReadShort(msg);
    if (fragmented)
    {
        fragmentStart = MSG_ReadLong(msg);
        fragmentLength = MSG_ReadShort(msg);
    }
    else
    {
        fragmentStart = 0;
        fragmentLength = 0;
    }
    NetProf_NewRecievePacket(chan, msg->cursize, fragmented);
    if (showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK))
    {
        if (fragmented)
            Com_Printf(
                16,
                "[%s] recv %4i : s=%i fragment=%i,%i\n",
                netsrcString[chan->sock],
                msg->cursize,
                sequence,
                fragmentStart,
                fragmentLength);
        else
            Com_Printf(16, "[%s] recv %4i : s=%i\n", netsrcString[chan->sock], msg->cursize, sequence);
    }
    if (sequence <= chan->incomingSequence)
    {
        if (showdrop->current.enabled
            || showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK))
        {
            incomingSequence = chan->incomingSequence;
            v2 = NET_AdrToString(chan->remoteAddress);
            Com_Printf(
                16,
                "[%s] %s: Out of order packet %i at %i\n",
                netsrcString[chan->sock],
                v2,
                sequence,
                incomingSequence);
        }
        return 0;
    }
    chan->dropped = sequence - (chan->incomingSequence + 1);
    if (chan->dropped > 0
        && (showdrop->current.enabled
            || showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK)))
    {
        dropped = chan->dropped;
        v4 = NET_AdrToString(chan->remoteAddress);
        Com_Printf(16, "[%s] %s: Dropped %i packets at %i\n", netsrcString[chan->sock], v4, dropped, sequence);
    }
    if (!fragmented)
    {
    LABEL_52:
        chan->incomingSequence = sequence;
        return 1;
    }
    if (sequence != chan->fragmentSequence)
    {
        chan->fragmentSequence = sequence;
        chan->fragmentLength = 0;
    }
    if (fragmentStart != chan->fragmentLength)
    {
        if (showdrop->current.enabled
            || showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK))
        {
            v5 = NET_AdrToString(chan->remoteAddress);
            Com_Printf(16, "%s:Dropped a message fragment\n", v5);
        }
        return 0;
    }
    if (fragmentLength >= 0
        && fragmentLength + msg->readcount <= msg->cursize
        && fragmentLength + chan->fragmentLength <= chan->fragmentBufferSize)
    {
        memcpy(&chan->fragmentBuffer[chan->fragmentLength], &msg->data[msg->readcount], fragmentLength);
        chan->fragmentLength += fragmentLength;
        if (fragmentLength == 1300)
            return 0;
        if (chan->fragmentLength > msg->maxsize)
        {
            v10 = chan->fragmentLength;
            v7 = NET_AdrToString(chan->remoteAddress);
            Com_Printf(16, "%s:fragmentLength %i > msg->maxsize\n", v7, v10);
            return 0;
        }
        *(unsigned int *)msg->data = sequence;
        memcpy(msg->data + 4, chan->fragmentBuffer, chan->fragmentLength);
        msg->cursize = chan->fragmentLength + 4;
        chan->fragmentLength = 0;
        MSG_BeginReading(msg);
        MSG_ReadLong(msg);
        goto LABEL_52;
    }
    if (showdrop->current.enabled
        || showpackets->current.integer && (showpackets->current.integer > 1 || chan->remoteAddress.type != NA_LOOPBACK))
    {
        v6 = NET_AdrToString(chan->remoteAddress);
        Com_Printf(16, "%s:illegal fragment length\n", v6);
    }
    return 0;
}

int __cdecl NET_CompareBaseAdrSigned(netadr_t *a, netadr_t *b)
{
    if (a->type != b->type)
        return a->type - b->type;
    switch (a->type)
    {
    case NA_LOOPBACK:
        return a->port - b->port;
    case NA_BOT:
        return a->port - b->port;
    case NA_IP:
        return memcmp((const char *)a->ip, (const char *)b->ip, 4);
    case NA_IPX:
        return memcmp((const char *)a->ipx, (const char *)b->ipx, 10);
    }
    Com_Printf(16, "NET_CompareBaseAdrSigned: bad address type\n");
    return 0;
}

bool __cdecl NET_CompareBaseAdr(netadr_t a, netadr_t b)
{
    return NET_CompareBaseAdrSigned(&a, &b) == 0;
}

int __cdecl NET_CompareAdrSigned(netadr_t *a, netadr_t *b)
{
    if (a->type != b->type)
        return a->type - b->type;
    switch (a->type)
    {
    case NA_LOOPBACK:
        return 0;
    case NA_IP:
        if (a->port == b->port)
            return memcmp((const char *)a->ip, (const char *)b->ip, 4);
        else
            return a->port - b->port;
    case NA_IPX:
        if (a->port == b->port)
            return memcmp((const char *)a->ipx, (const char *)b->ipx, 10);
        else
            return a->port - b->port;
    default:
        Com_Printf(16, "NET_CompareAdrSigned: bad address type\n");
        return 0;
    }
}

bool __cdecl NET_CompareAdr(netadr_t a, netadr_t b)
{
    return NET_CompareAdrSigned(&a, &b) == 0;
}

bool __cdecl NET_IsLocalAddress(netadr_t adr)
{
    return adr.type == NA_LOOPBACK || adr.type == NA_BOT;
}

int __cdecl NET_GetClientPacket(netadr_t *net_from, msg_t *net_message)
{
    if (fakelagInitialized && fakelag_current->current.integer)
        return FakeLag_GetPacket(0, NS_PACKET, net_from, net_message);
    else
        return Sys_GetPacket(net_from, net_message);
}

int __cdecl NET_GetServerPacket(netadr_t *net_from, msg_t *net_message)
{
    return Sys_GetPacket(net_from, net_message);
}

int __cdecl NET_GetLoopPacket_Real(netsrc_t sock, netadr_t *net_from, msg_t *net_message)
{
    loopback_t *loop; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    loop = &loopbacks[sock];
    if (loop->send - loop->get > 16)
        loop->get = loop->send - 16;
    if (loop->get >= loop->send)
        return 0;
    i = loop->get & 0xF;
    InterlockedIncrement(&loop->get);
    memcpy(net_message->data, loop->msgs[i].data, loop->msgs[i].datalen);
    net_message->cursize = loop->msgs[i].datalen;
    net_from->type = NA_BOT;
    *(unsigned int *)net_from->ip = 0;
    *(unsigned int *)&net_from->port = 0;
    *(unsigned int *)&net_from->ipx[2] = 0;
    *(unsigned int *)&net_from->ipx[6] = 0;
    net_from->type = NA_LOOPBACK;
    net_from->port = loop->msgs[i].port;
    return 1;
}

int __cdecl NET_GetLoopPacket(netsrc_t sock, netadr_t *net_from, msg_t *net_message)
{
    if (fakelagInitialized && fakelag_current->current.integer)
        return FakeLag_GetPacket(1, sock, net_from, net_message);
    else
        return NET_GetLoopPacket_Real(sock, net_from, net_message);
}

void __cdecl NET_SendLoopPacket(netsrc_t sock, unsigned int length, unsigned __int8 *data, netadr_t to)
{
    loopback_t *loop; // [esp+0h] [ebp-10h]
    int i; // [esp+4h] [ebp-Ch]
    netsrc_t port; // [esp+8h] [ebp-8h]

    port = NS_CLIENT1;
    if (sock >= NS_SERVER)
    {
        if (sock == NS_SERVER)
            sock = to.port;
    }
    else
    {
        port = sock;
        sock = NS_SERVER;
    }
    loop = &loopbacks[sock];
    i = loop->send & 0xF;
    memcpy(loop->msgs[i].data, data, length);
    loop->msgs[i].datalen = length;
    loop->msgs[i].port = port;
    InterlockedIncrement(&loop->send);
}

char __cdecl NET_SendPacket(netsrc_t sock, int length, unsigned __int8 *data, netadr_t to)
{
    netadr_t v5; // [esp-14h] [ebp-18h]

    if (showpackets->current.integer && *(unsigned int *)data == -1)
        Com_Printf(16, "[%s] send packet %4i\n", netsrcString[sock], length);
    if (to.type == NA_LOOPBACK)
    {
        *(_QWORD *)&v5.type = __PAIR64__(*(unsigned int *)to.ip, 2);
        *(unsigned int *)&v5.port = *(unsigned int *)&to.port;
        *(_QWORD *)&v5.ipx[2] = *(_QWORD *)&to.ipx[2];
        NET_SendLoopPacket(sock, length, data, v5);
        return 1;
    }
    else if (to.type == NA_BAD)
    {
        return 0;
    }
    else if (to.type)
    {
        return Sys_SendPacket(length, data, to);
    }
    else
    {
        return 0;
    }
}

bool __cdecl NET_OutOfBandPrint(netsrc_t sock, netadr_t adr, const char *data)
{
    char *v3; // eax
    const char *v4; // eax
    int v6; // [esp+0h] [ebp-5Ch]
    int res; // [esp+54h] [ebp-8h]

    tempNetchanPacketBuf[0] = -1;
    tempNetchanPacketBuf[1] = -1;
    tempNetchanPacketBuf[2] = -1;
    tempNetchanPacketBuf[3] = -1;
    if (showpackets->current.integer && (showpackets->current.integer > 1 || adr.type != NA_LOOPBACK))
    {
        v3 = NET_AdrToString(adr);
        Com_DPrintf(16, "OOB Print->%s: %s\n", v3, data);
    }
    if (strlen(data) + 1 <= 0x1FFFC)
    {
        strcpy((char *)&tempNetchanPacketBuf[4], data);
        v6 = strlen((const char *)tempNetchanPacketBuf);
        res = (int)FakeLag_SendPacket(sock, v6, tempNetchanPacketBuf, adr) >= -1;
        if (sock == NS_SERVER)
            SV_Netchan_AddOOBProfilePacket(v6);
        else
            CL_Netchan_AddOOBProfilePacket(sock, v6);
        return res > 0;
    }
    else
    {
        Com_DPrintf(16, "OOB Packet is %i bytes - too large to send\n", strlen(data));
        if (!alwaysfails)
        {
            v4 = va("OOB Packet is %i bytes - too large to send\n", strlen(data));
            MyAssertHandler(".\\qcommon\\net_chan_mp.cpp", 1818, 0, v4);
        }
        return 0;
    }
}

bool __cdecl NET_OutOfBandData(netsrc_t sock, netadr_t adr, const unsigned __int8 *format, int len)
{
    int mbuf_20; // [esp+14h] [ebp-20h]
    int i; // [esp+28h] [ebp-Ch]
    int res; // [esp+2Ch] [ebp-8h]

    tempNetchanPacketBuf[0] = -1;
    tempNetchanPacketBuf[1] = -1;
    tempNetchanPacketBuf[2] = -1;
    tempNetchanPacketBuf[3] = -1;
    for (i = 0; i < len; ++i)
        *(_BYTE *)(i + 36877148) = format[i];
    mbuf_20 = len + 4;
    if (showpackets->current.integer && (showpackets->current.integer > 1 || adr.type != NA_LOOPBACK))
        Com_DPrintf(16, "OOB Data->%u.%u.%u.%u: %i bytes\n", adr.ip[0], adr.ip[1], adr.ip[2], adr.ip[3], mbuf_20);
    res = (int)FakeLag_SendPacket(sock, mbuf_20, tempNetchanPacketBuf, adr) >= -1;
    if (sock == NS_SERVER)
        SV_Netchan_AddOOBProfilePacket(mbuf_20);
    else
        CL_Netchan_AddOOBProfilePacket(sock, mbuf_20);
    return res > 0;
}

bool __cdecl NET_OutOfBandVoiceData(netsrc_t sock, netadr_t adr, unsigned __int8 *format, unsigned int len)
{
    int mbuf_20; // [esp+14h] [ebp-1Ch]
    int res; // [esp+28h] [ebp-8h]

    tempNetchanPacketBuf[0] = -1;
    tempNetchanPacketBuf[1] = -1;
    tempNetchanPacketBuf[2] = -1;
    tempNetchanPacketBuf[3] = -1;
    if ((int)(len + 4) >= 0x20000)
        MyAssertHandler(
            ".\\qcommon\\net_chan_mp.cpp",
            1952,
            0,
            "%s",
            "len + 4 < static_cast<int>( sizeof( tempNetchanPacketBuf ) )");
    memcpy(&tempNetchanPacketBuf[4], format, len);
    mbuf_20 = len + 4;
    res = (int)FakeLag_SendPacket(sock, len + 4, tempNetchanPacketBuf, adr) >= -1;
    if (sock == NS_SERVER)
        SV_Netchan_AddOOBProfilePacket(mbuf_20);
    else
        CL_Netchan_AddOOBProfilePacket(sock, mbuf_20);
    return res > 0;
}

int __cdecl NET_StringToAdr(char *s, netadr_t *a)
{
    char *v3; // eax
    __int16 v4; // ax
    unsigned __int16 v5; // ax
    char base[1024]; // [esp+18h] [ebp-408h] BYREF
    char *port; // [esp+41Ch] [ebp-4h]

    if (!strcmp(s, "localhost"))
    {
        a->type = NA_BOT;
        *(unsigned int *)a->ip = 0;
        *(unsigned int *)&a->port = 0;
        *(unsigned int *)&a->ipx[2] = 0;
        *(unsigned int *)&a->ipx[6] = 0;
        a->type = NA_LOOPBACK;
        return 1;
    }
    else
    {
        I_strncpyz(base, s, 1024);
        strstr((unsigned __int8 *)base, ":");
        port = v3;
        if (v3)
            *port++ = 0;
        if (Sys_StringToAdr(base, a))
        {
            if (a->ip[0] == 255 && a->ip[1] == 255 && a->ip[2] == 255 && a->ip[3] == 255)
            {
                a->type = NA_BAD;
                return 0;
            }
            else
            {
                if (port)
                {
                    v4 = atoi(port);
                    v5 = BigShort(v4);
                }
                else
                {
                    v5 = BigShort(28960);
                }
                a->port = v5;
                return 1;
            }
        }
        else
        {
            a->type = NA_BAD;
            return 0;
        }
    }
}

