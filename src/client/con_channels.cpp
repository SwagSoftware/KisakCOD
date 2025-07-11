#include "client.h"
#include <bgame/bg_local.h>
#include <universal/com_files.h>
#include <qcommon/cmd.h>

struct PrintChannel // sizeof=0x21
{                                       // ...
    char name[32];                      // ...
    bool allowScript;                   // ...
};
struct PrintChannelGlob // sizeof=0x21E0
{                                       // ...
    PrintChannel openChannels[256];     // ...
    uint32_t filters[7][8];         // ...
};

// char const **defaultGameWindowFilters 827b3bcc     con_channels.obj
// char (*)[25] con_gameMsgWindowNFilter_Names 828a7160     con_channels.obj
// struct dvar_s const **con_gameMsgWindowNFilter 828a71c4     con_channels.obj
// char (*)[91] con_gameMsgWindowNFilter_Descs 828a71d8     con_channels.obj

PrintChannelGlob pcGlob;

char __cdecl Con_OpenChannel(char *name, bool allowScript)
{
    int32_t index; // [esp+0h] [ebp-8h]
    bool alreadyExists; // [esp+7h] [ebp-1h]

    alreadyExists = 0;
    for (index = 0; index < 256 && pcGlob.openChannels[index].name[0]; ++index)
    {
        if (!I_strnicmp(pcGlob.openChannels[index].name, name, 32))
        {
            alreadyExists = 1;
            break;
        }
    }
    if (index >= 256)
        return 0;
    if (!alreadyExists)
    {
        I_strncpyz(pcGlob.openChannels[index].name, name, 32);
        *((_BYTE *)pcGlob.filters[-263] + 33 * index) = allowScript;
    }
    return 1;
}

bool __cdecl Con_ScriptHasPermission(uint32_t channel)
{
    if (channel >= 0x100)
        return 0;
    if (pcGlob.openChannels[channel].name[0])
        return *((_BYTE *)pcGlob.filters[-263] + 33 * channel);
    return 0;
}

bool __cdecl Con_GetChannel(const char *name, int32_t *channel_result)
{
    int32_t channel; // [esp+0h] [ebp-4h]

    for (channel = 0; channel < 256; ++channel)
    {
        if (pcGlob.openChannels[channel].name[0] && !I_stricmp(name, pcGlob.openChannels[channel].name))
        {
            *channel_result = channel;
            return channel < 256;
        }
    }
    return channel < 256;
}

bool __cdecl Con_IsChannelOpen(uint32_t channel)
{
    return channel < 0x100 && pcGlob.openChannels[channel].name[0] != 0;
}

bool __cdecl Con_IsChannelVisible(print_msg_dest_t dest, uint32_t channel, int32_t errorflags)
{
    int32_t error; // [esp+4h] [ebp-4h]

    if (channel >= 0x100)
        MyAssertHandler(
            ".\\client\\con_channels.cpp",
            209,
            0,
            "%s\n\t(channel) = %i",
            "(channel >= 0 && channel < CON_MAX_CHANNELS)",
            channel);
    if (!pcGlob.openChannels[channel].name[0])
        return 0;
    if (dest == CON_DEST_MINICON)
    {
        if (channel == 2 || channel == 3 || channel == 4)
            return 0;
        dest = CON_DEST_CONSOLE;
    }
    if (dest == CON_DEST_CONSOLE && !channel)
        return 1;
    if (Com_BitCheckAssert(pcGlob.filters[dest], channel, 32))
        return 1;
    error = (errorflags >> 5) & 0x1F;
    return (error == 3 || error == 2) && Com_BitCheckAssert(pcGlob.filters[dest], 1, 32);
}

void __cdecl Con_WriteFilterConfigString(int32_t f)
{
    int32_t channel; // [esp+0h] [ebp-4h]

    FS_Printf(f, "con_hidechannel *; con_showchannel");
    for (channel = 0; channel < 256; ++channel)
    {
        if (Con_IsChannelVisible(CON_DEST_CONSOLE, channel, 0))
            FS_Printf(f, " %s", pcGlob.openChannels[channel].name);
    }
    FS_Printf(f, "\n");
}

char con_gameMsgWindowNFilter_Descs[4][91];
char con_gameMsgWindowNFilter_Names[4][25];
const dvar_s *con_gameMsgWindowNFilter[4];
const char *defaultGameWindowFilters[4] =
{
    "gamenotify obituary",
    "boldgame",
    "subtitle",
    ""
};
void __cdecl Con_InitGameMsgChannels()
{
    uint32_t gameWindowIndex; // [esp+24h] [ebp-8h]
    uint32_t gameWindowIndexa; // [esp+24h] [ebp-8h]
    char *dvarDesc; // [esp+28h] [ebp-4h]

    for (gameWindowIndex = 0; gameWindowIndex < 4; ++gameWindowIndex)
    {
        dvarDesc = con_gameMsgWindowNFilter_Descs[gameWindowIndex];
        sprintf(con_gameMsgWindowNFilter_Names[gameWindowIndex], "con_gameMsgWindow%dFilter", gameWindowIndex);
        if (strlen(con_gameMsgWindowNFilter_Names[gameWindowIndex]) != 24)
            MyAssertHandler(
                ".\\client\\con_channels.cpp",
                371,
                0,
                "%s",
                "strlen( dvarName ) == sizeof( con_gameMsgWindowNFilter_Names[gameWindowIndex] ) - 1");
        sprintf(
            dvarDesc,
            "Space-separated list of console channels that should be displayed in game message window %d",
            gameWindowIndex);
        if (strlen(con_gameMsgWindowNFilter_Descs[gameWindowIndex]) != 90)
            MyAssertHandler(
                ".\\client\\con_channels.cpp",
                374,
                0,
                "%s",
                "strlen( dvarDesc ) == sizeof( con_gameMsgWindowNFilter_Descs[gameWindowIndex] ) - 1");
        con_gameMsgWindowNFilter[gameWindowIndex] = Dvar_RegisterString(
            con_gameMsgWindowNFilter_Names[gameWindowIndex],
            (char *)defaultGameWindowFilters[gameWindowIndex],
            DVAR_ARCHIVE | DVAR_SYSTEMINFO | DVAR_LATCH,
            dvarDesc);
    }
    for (gameWindowIndexa = 0; gameWindowIndexa < 4; ++gameWindowIndexa)
        Con_InitChannelsForDestFromList(
            (print_msg_dest_t)(gameWindowIndexa + 3),
            con_gameMsgWindowNFilter[gameWindowIndexa]->current.string);
}

void __cdecl Con_InitChannelsForDestFromList(print_msg_dest_t dest, const char *channelNames)
{
    char channelName[256]; // [esp+10h] [ebp-118h] BYREF
    uint32_t channelNamesLen; // [esp+114h] [ebp-14h]
    uint32_t charIndex; // [esp+118h] [ebp-10h]
    uint32_t channelNameLength; // [esp+11Ch] [ebp-Ch]
    bool foundChannelName; // [esp+123h] [ebp-5h]
    uint32_t channelNameStart; // [esp+124h] [ebp-4h]

    if (!channelNames)
        MyAssertHandler(".\\client\\con_channels.cpp", 319, 0, "%s", "channelNames");
    channelNamesLen = strlen(channelNames);
    foundChannelName = 0;
    channelNameStart = 0;
    Con_FilterShowChannel(dest, "*", 0);
    for (charIndex = 0; charIndex <= channelNamesLen; ++charIndex)
    {
        if (channelNames[charIndex] == 32 || channelNames[charIndex] == 44 || !channelNames[charIndex])
        {
            if (foundChannelName)
            {
                channelNameLength = charIndex - channelNameStart;
                if (charIndex - channelNameStart + 1 > 0x100)
                    Com_Error(ERR_DROP, "Channel name too long in specified list: \"%s\"\n", channelNames);
                memcpy((uint8_t *)channelName, (uint8_t *)&channelNames[channelNameStart], channelNameLength);
                channelName[channelNameLength] = 0;
                Con_FilterShowChannel(dest, channelName, 1);
                foundChannelName = 0;
            }
        }
        else if (!foundChannelName)
        {
            channelNameStart = charIndex;
            foundChannelName = 1;
        }
    }
}

void __cdecl Con_FilterShowChannel(print_msg_dest_t dest, const char *channelName, bool show)
{
    int32_t channel; // [esp+0h] [ebp-8h]
    int32_t count; // [esp+4h] [ebp-4h]

    count = 0;
    for (channel = 1; channel < 256; ++channel)
    {
        if (pcGlob.openChannels[channel].name[0] && !I_stricmpwild(channelName, pcGlob.openChannels[channel].name))
        {
            if (show)
            {
                if (!Com_BitCheckAssert(pcGlob.filters[dest], channel, 32))
                {
                    Com_BitSetAssert(pcGlob.filters[dest], channel, 32);
                    Com_Printf(0, "Adding channel: %s\n", pcGlob.openChannels[channel].name);
                    ++count;
                }
            }
            else if (Com_BitCheckAssert(pcGlob.filters[dest], channel, 32))
            {
                Com_BitClearAssert(pcGlob.filters[dest], channel, 32);
                Com_Printf(0, "Hiding channel: %s\n", pcGlob.openChannels[channel].name);
                ++count;
            }
        }
    }
    if (!count)
        Com_Printf(0, "No channels added or hidden\n");
}

const char *builtinChannels[25] =
{
  "dontfilter",
  "error",
  "gamenotify",
  "boldgame",
  "subtitle",
  "obituary",
  "logfile_only",
  "console_only",
  "gfx",
  "sound",
  "files",
  "devgui",
  "profile",
  "ui",
  "client",
  "server",
  "system",
  "playerweap",
  "ai",
  "anim",
  "physics",
  "fx",
  "leaderboards",
  "parserscript",
  "script"
}; // idb

cmd_function_s Con_ChannelList_f_VAR;
cmd_function_s Con_FilterAdd_f_VAR;
cmd_function_s Con_FilterRemove_f_VAR;
cmd_function_s Con_FilterList_f_VAR;

const dvar_t *con_default_console_filter;
void __cdecl Con_InitChannels()
{
    uint32_t channel; // [esp+0h] [ebp-4h]

    memset((uint8_t *)&pcGlob, 0, 0x2100u);
    for (channel = 0; channel < 0x19; ++channel)
        Con_OpenChannel((char *)builtinChannels[channel], 0);
    pcGlob.openChannels[24].allowScript = 1;
    con_default_console_filter = Dvar_RegisterString(
        "con_default_console_filter",
        "*",
        DVAR_NOFLAG,
        "Default channel filter for the console destination.");
    Con_FilterShowChannel(CON_DEST_CONSOLE, con_default_console_filter->current.string, 1);
    Con_FilterShowChannel(CON_DEST_MINICON, "std", 1);
    Con_FilterShowChannel(CON_DEST_MINICON, "error", 1);
    Con_FilterShowChannel(CON_DEST_ERROR, "error", 1);
    Con_InitGameMsgChannels();
    Cmd_AddCommandInternal("con_channellist", Con_ChannelList_f, &Con_ChannelList_f_VAR);
    Cmd_AddCommandInternal("con_showchannel", Con_FilterAdd_f, &Con_FilterAdd_f_VAR);
    Cmd_AddCommandInternal("con_hidechannel", Con_FilterRemove_f, &Con_FilterRemove_f_VAR);
    Cmd_AddCommandInternal("con_visiblechannellist", Con_FilterList_f, &Con_FilterList_f_VAR);
}

void __cdecl Con_ChannelList_f()
{
    int32_t channel; // [esp+0h] [ebp-4h]

    for (channel = 0; channel < 256; ++channel)
    {
        if (channel)
        {
            if (pcGlob.openChannels[channel].name[0])
                Com_Printf(0, "%s\n", pcGlob.openChannels[channel].name);
        }
    }
}

void __cdecl Con_FilterAdd_f()
{
    Con_FilterAdd(1);
}

void __cdecl Con_FilterAdd(bool show)
{
    const char *v1; // eax
    const char *v2; // eax
    int32_t arg; // [esp+0h] [ebp-8h]
    int32_t argc; // [esp+4h] [ebp-4h]

    argc = Cmd_Argc();
    if (argc >= 2)
    {
        for (arg = 1; arg < argc; ++arg)
        {
            v2 = Cmd_Argv(arg);
            Con_FilterShowChannel(CON_DEST_CONSOLE, v2, show);
        }
        dvar_modifiedFlags |= 1u;
    }
    else
    {
        v1 = Cmd_Argv(0);
        Com_Printf(0, "USAGE: %s <channel>\n<channel> may include wildcards */?\n", v1);
    }
}

void __cdecl Con_FilterRemove_f()
{
    Con_FilterAdd(0);
}

void __cdecl Con_FilterList_f()
{
    int32_t channel; // [esp+0h] [ebp-4h]

    for (channel = 0; channel < 256; ++channel)
    {
        if (channel)
        {
            if (Con_IsChannelVisible(CON_DEST_CONSOLE, channel, 0))
                Com_Printf(0, "%s\n", pcGlob.openChannels[channel].name);
        }
    }
}

void __cdecl Con_ShutdownChannels()
{
    int32_t channel; // [esp+0h] [ebp-4h]

    for (channel = 0; channel < 256; ++channel)
        Con_CloseChannelInternal(channel);
}

void __cdecl Con_CloseChannelInternal(uint32_t channel)
{
    uint32_t filter; // [esp+0h] [ebp-4h]

    if (channel >= 0x100)
        MyAssertHandler(
            ".\\client\\con_channels.cpp",
            118,
            0,
            "channel doesn't index CON_MAX_CHANNELS\n\t%i not in [0, %i)",
            channel,
            256);
    if (pcGlob.openChannels[channel].name[0])
    {
        pcGlob.openChannels[channel].name[0] = 0;
        for (filter = 0; filter < 7; ++filter)
            Com_BitClearAssert(pcGlob.filters[filter], channel, 32);
    }
}

#ifdef KISAK_SP
void Con_SaveChannels(MemoryFile *memFile)
{
    int v2; // r10
    PrintChannel *v3; // r11
    int v4; // r29
    PrintChannelGlob *v5; // r31
    PrintChannelGlob *v6; // r11
    int v7; // r9
    int v8; // r30
    _DWORD v9[16]; // [sp+50h] [-40h] BYREF

    v2 = 0;
    v3 = &pcGlob.openChannels[1];
    do
    {
        if (v3[-1].name[0])
            ++v2;
        if (v3->name[0])
            ++v2;
        if (v3[1].name[0])
            ++v2;
        if (v3[2].name[0])
            ++v2;
        v3 += 4;
    } while ((int)v3 < (int)((char *)&pcGlob.filters[1][0] + 1));
    v9[0] = v2;
    MemFile_WriteData(memFile, 4, v9);
    v4 = 0;
    v5 = &pcGlob;
    do
    {
        if (v5->openChannels[0].name[0])
        {
            v6 = v5;
            do
            {
                v7 = (unsigned __int8)v6->openChannels[0].name[0];
                v6 = (PrintChannelGlob *)((char *)v6 + 1);
            } while (v7);
            v9[0] = v4;
            v8 = (char *)v6 - (char *)v5 - 1;
            MemFile_WriteData(memFile, 4, v9);
            v9[0] = v8;
            MemFile_WriteData(memFile, 4, v9);
            MemFile_WriteData(memFile, v8, v5);
        }
        v5 = (PrintChannelGlob *)((char *)v5 + 33);
        ++v4;
    } while ((int)v5 < (int)pcGlob.filters);
}

void Con_RestoreChannels(MemoryFile *memFile)
{
    unsigned int v2; // r25
    int v3; // r30
    int v4; // r31
    unsigned int v5; // [sp+50h] [-70h] BYREF
    int v6[3]; // [sp+54h] [-6Ch] BYREF
    char v7[96]; // [sp+60h] [-60h] BYREF

    MemFile_ReadData(memFile, 4, (unsigned char*)&v5);
    v2 = v5;
    if (v5)
    {
        while (1)
        {
            MemFile_ReadData(memFile, 4, (unsigned char *)&v5);
            v3 = v5;
            if (v5 >= 0x100)
                Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
            MemFile_ReadData(memFile, 4, (unsigned char *)v6);
            v4 = v6[0];
            if (!v6[0] || v6[0] >= 31)
                Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
            MemFile_ReadData(memFile, v4, (unsigned char *)v7);
            v7[v4] = 0;
            if (!pcGlob.openChannels[v3].name[0])
                goto LABEL_10;
            if (I_stricmp(pcGlob.openChannels[v3].name, v7))
                break;
        LABEL_11:
            if (!--v2)
                return;
        }
        Con_CloseChannelInternal(v3);
    LABEL_10:
        I_strncpyz(pcGlob.openChannels[v3].name, v7, 32);
        goto LABEL_11;
    }
}
#endif