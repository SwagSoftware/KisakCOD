#include "client_mp.h"

#include <qcommon/qcommon.h>

static void __cdecl CL_Netchan_Encode(unsigned __int8 *data, int size)
{
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-18h]
    unsigned __int8 *string; // [esp+4h] [ebp-14h]
    clientConnection_t *clc; // [esp+8h] [ebp-10h]
    _BYTE key[5]; // [esp+Fh] [ebp-9h]
    int i; // [esp+14h] [ebp-4h]

    LocalClientGlobals = CL_GetLocalClientGlobals(0);
    clc = CL_GetLocalClientConnection(0);
    string = (unsigned __int8 *)clc->serverCommands[clc->serverCommandSequence & 0x7F];
    key[4] = 0;
    *(_DWORD *)key = (unsigned __int8)(LOBYTE(clc->serverMessageSequence)
        ^ LOBYTE(LocalClientGlobals->serverId)
        ^ LOBYTE(clc->challenge));
    for (i = 0; i < size; ++i)
    {
        if (!string[*(_DWORD *)&key[1]])
            *(_DWORD *)&key[1] = 0;
        if (string[*(_DWORD *)&key[1]] == 37)
            MyAssertHandler(
                ".\\client_mp\\cl_net_chan_mp.cpp",
                48,
                0,
                "%s\n\t(clc->serverCommands[clc->serverCommandSequence & (128 - 1)]) = %s",
                "(string[index] != '%')",
                clc->serverCommands[clc->serverCommandSequence & 0x7F]);
        key[0] ^= string[(*(_DWORD *)&key[1])++] << (i & 1);
        *data++ ^= key[0];
    }
}

void __cdecl CL_Netchan_TransmitNextFragment(netchan_t *chan)
{
    Netchan_TransmitNextFragment(chan);
}

void __cdecl CL_Netchan_Transmit(netchan_t *chan, unsigned __int8 *data, int length)
{
    CL_Netchan_Encode(data + 9, length - 9);
    Netchan_Transmit(chan, length, (char *)data);
}

void __cdecl CL_Netchan_AddOOBProfilePacket(int localClientNum, int iLength)
{
    clientConnection_t *clc; // [esp+0h] [ebp-4h]

    if (net_profile->current.integer)
    {
        if (clientConnections)
        {
            clc = CL_GetLocalClientConnection(localClientNum);
            NetProf_PrepProfiling(&clc->OOBProf);
            NetProf_AddPacket(&clc->OOBProf.send, iLength, 0);
        }
    }
}

void __cdecl CL_Netchan_PrintProfileStats(int localClientNum, int bPrintToConsole)
{
    char szLine[1024]; // [esp+0h] [ebp-420h] BYREF
    int iYPos; // [esp+404h] [ebp-1Ch]
    int iTotalBPSRecieved; // [esp+408h] [ebp-18h]
    int iTotalBPSSent; // [esp+40Ch] [ebp-14h]
    clientConnection_t *clc; // [esp+410h] [ebp-10h]
    int iTotalBPS; // [esp+414h] [ebp-Ch]
    int iYStep; // [esp+418h] [ebp-8h]
    netProfileInfo_t *pStream; // [esp+41Ch] [ebp-4h]

    iTotalBPSSent = 0;
    iTotalBPSRecieved = 0;
    iTotalBPS = 0;
    iYPos = 80;
    iYStep = 10;
    if (!net_profile->current.integer)
        MyAssertHandler(".\\client_mp\\cl_net_chan_mp.cpp", 193, 0, "%s", "net_profile->current.integer");
    CL_Netchan_UpdateProfileStats(localClientNum);
    if (bPrintToConsole)
        Com_Printf(14, "\n\n");
    Com_sprintf(szLine, 0x400u, "====================");
    if (bPrintToConsole)
    {
        Com_Printf(14, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        CL_ProfDraw(iYPos, szLine);
    }
    Com_sprintf(szLine, 0x400u, "Client Network Profile:");
    if (bPrintToConsole)
    {
        Com_Printf(14, "%s\n\n", szLine);
    }
    else
    {
        iYPos += 10;
        CL_ProfDraw(iYPos, szLine);
        iYPos += 10;
    }
    Com_sprintf(szLine, 0x400u, "      Source    bps   max   min frag%%");
    if (bPrintToConsole)
    {
        Com_Printf(14, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        CL_ProfDraw(iYPos, szLine);
    }
    clc = CL_GetLocalClientConnection(localClientNum);
    pStream = &clc->OOBProf;
    iTotalBPSSent += clc->OOBProf.send.iBytesPerSecond;
    iTotalBPSRecieved += clc->OOBProf.recieve.iBytesPerSecond;
    Com_sprintf(
        szLine,
        0x400u,
        "    OOB Sent: %5i %5i %5i    -",
        clc->OOBProf.send.iBytesPerSecond,
        clc->OOBProf.send.iLargestPacket,
        clc->OOBProf.send.iSmallestPacket);
    if (bPrintToConsole)
    {
        Com_Printf(14, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        CL_ProfDraw(iYPos, szLine);
    }
    Com_sprintf(
        szLine,
        0x400u,
        "OOB Recieved: %5i %5i %5i    -",
        pStream->recieve.iBytesPerSecond,
        pStream->recieve.iLargestPacket,
        pStream->recieve.iSmallestPacket);
    if (bPrintToConsole)
    {
        Com_Printf(14, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        CL_ProfDraw(iYPos, szLine);
    }
    pStream = &clc->netchan.prof;
    iTotalBPSSent += clc->netchan.prof.send.iBytesPerSecond;
    iTotalBPSRecieved += clc->netchan.prof.recieve.iBytesPerSecond;
    Com_sprintf(
        szLine,
        0x400u,
        "        Sent: %5i %5i %5i  %3i%%",
        clc->netchan.prof.send.iBytesPerSecond,
        clc->netchan.prof.send.iLargestPacket,
        clc->netchan.prof.send.iSmallestPacket,
        clc->netchan.prof.send.iFragmentPercentage);
    if (bPrintToConsole)
    {
        Com_Printf(14, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        CL_ProfDraw(iYPos, szLine);
    }
    Com_sprintf(
        szLine,
        0x400u,
        "    Recieved: %5i %5i %5i  %3i%%",
        pStream->recieve.iBytesPerSecond,
        pStream->recieve.iLargestPacket,
        pStream->recieve.iSmallestPacket,
        pStream->recieve.iFragmentPercentage);
    if (bPrintToConsole)
    {
        Com_Printf(14, "%s\n", szLine);
    }
    else
    {
        iYPos += 10;
        CL_ProfDraw(iYPos, szLine);
    }
    iTotalBPS = iTotalBPSRecieved + iTotalBPSSent;
    Com_sprintf(szLine, 0x400u, "       Total: %5i", iTotalBPSRecieved + iTotalBPSSent);
    if (bPrintToConsole)
        Com_Printf(14, "%s\n", szLine);
    else
        CL_ProfDraw(iYPos + 10, szLine);
}

void __cdecl CL_Netchan_UpdateProfileStats(int localClientNum)
{
    clientConnection_t *clc; // [esp+0h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    NetProf_UpdateStatistics(&clc->netchan.prof.send);
    NetProf_UpdateStatistics(&clc->netchan.prof.recieve);
    NetProf_UpdateStatistics(&clc->OOBProf.send);
    NetProf_UpdateStatistics(&clc->OOBProf.recieve);
}

void __cdecl CL_ProfDraw(int y, char *pszString)
{
    CL_DrawString(32, y, pszString, 0, cl_profileTextHeight->current.integer);
}

