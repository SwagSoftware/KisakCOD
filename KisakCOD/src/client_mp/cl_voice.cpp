#include "client_mp.h"
#include <cgame_mp/cg_local_mp.h>
#include <win32/win_local.h>
#include <server_mp/server.h>

unsigned __int8 tempVoicePacketBuf[2048];
voiceCommunication_t cl_voiceCommunication;

void __cdecl CL_WriteVoicePacket(int localClientNum)
{
    int voicePacket; // [esp+8h] [ebp-34h]
    msg_t msg; // [esp+10h] [ebp-2Ch] BYREF
    const clientConnection_t *clc; // [esp+38h] [ebp-4h]

    clc = CL_GetLocalClientConnection(localClientNum);
    CL_GetLocalClientGlobals(localClientNum);
    if (localClientNum)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1072,
            0,
            "localClientNum doesn't index 1\n\t%i not in [0, %i)",
            localClientNum,
            1);
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client_mp\\client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    }
    if (!clc->demoplaying
        && (clientUIActives[0].connectionState == CA_ACTIVE
            || clientUIActives[0].connectionState == CA_LOADING
            || clientUIActives[0].connectionState == CA_PRIMED))
    {
        MSG_Init(&msg, tempVoicePacketBuf, 2048);
        MSG_WriteString(&msg, "v");
        MSG_WriteShort(&msg, clc->qport);
        MSG_WriteByte(&msg, cl_voiceCommunication.voicePacketCount);
        for (voicePacket = 0; voicePacket < cl_voiceCommunication.voicePacketCount; ++voicePacket)
        {
            if (cl_voiceCommunication.voicePackets[voicePacket].dataSize <= 0)
                MyAssertHandler(".\\client_mp\\cl_voice.cpp", 883, 0, "%s", "vc->voicePackets[voicePacket].dataSize > 0");
            if (cl_voiceCommunication.voicePackets[voicePacket].dataSize >= 0x10000)
                MyAssertHandler(".\\client_mp\\cl_voice.cpp", 884, 0, "%s", "vc->voicePackets[voicePacket].dataSize < (2<<15)");
            MSG_WriteByte(&msg, cl_voiceCommunication.voicePackets[voicePacket].dataSize);
            MSG_WriteData(
                &msg,
                cl_voiceCommunication.voicePackets[voicePacket].data,
                cl_voiceCommunication.voicePackets[voicePacket].dataSize);
        }
        NET_OutOfBandVoiceData(clc->netchan.sock, clc->serverAddress, msg.data, msg.cursize);
        if (cl_showSend->current.enabled)
            Com_Printf(14, "voice: %i\n", msg.cursize);
    }
}

void __cdecl CL_VoicePacket(int localClientNum, msg_t *msg)
{
    int packet; // [esp+0h] [ebp-114h]
    VoicePacket_t voicePacket; // [esp+4h] [ebp-110h] BYREF
    int numPackets; // [esp+110h] [ebp-4h]

    numPackets = MSG_ReadByte(msg);
    if ((unsigned int)numPackets <= 0x28)
    {
        for (packet = 0; packet < numPackets; ++packet)
        {
            voicePacket.talker = MSG_ReadByte(msg);
            voicePacket.dataSize = MSG_ReadByte(msg);
            if (voicePacket.dataSize <= 0 || voicePacket.dataSize > 256)
            {
                Com_Printf(14, "Invalid server voice packet of %i bytes\n", voicePacket.dataSize);
                return;
            }
            MSG_ReadData(msg, voicePacket.data, voicePacket.dataSize);
            if (voicePacket.talker >= 0x40u)
            {
                Com_Printf(14, "Invalid voice packet - talker was %i\n", voicePacket.talker);
                return;
            }
            if (!CL_IsPlayerMuted(localClientNum, voicePacket.talker) && cl_voice->current.enabled)
                Voice_IncomingVoiceData(voicePacket.talker, voicePacket.data, voicePacket.dataSize);
        }
    }
}

bool __cdecl CL_IsPlayerTalking(int localClientNum, int talkingClientIndex)
{
    if (CL_IsClientLocal(talkingClientIndex) && (sv_voice->current.enabled || cl_voice->current.enabled))
        return IN_IsTalkKeyHeld();
    else
        return Voice_IsClientTalking(talkingClientIndex);
}

