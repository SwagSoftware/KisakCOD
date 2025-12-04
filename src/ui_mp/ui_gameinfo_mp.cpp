#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "ui_mp.h"
#include <universal/q_parse.h>
#include <database/database.h>
#include <universal/com_files.h>

int ui_numArenas;
char *ui_arenaInfos[64];

int __cdecl UI_ParseInfos(const char *buf, int max, char **infos)
{
    unsigned __int8 *v3; // eax
    char v5; // [esp+3h] [ebp-865h]
    char *v6; // [esp+8h] [ebp-860h]
    char *v7; // [esp+Ch] [ebp-85Ch]
    unsigned int v8; // [esp+10h] [ebp-858h]
    char info[1024]; // [esp+58h] [ebp-810h] BYREF
    char key[1028]; // [esp+458h] [ebp-410h] BYREF
    const char *token; // [esp+860h] [ebp-8h]
    int count; // [esp+864h] [ebp-4h]

    count = 0;
    while (1)
    {
        token = (const char *)Com_Parse(&buf);
        if (!*token)
            return count;
        if (strcmp(token, "{"))
        {
            Com_Printf(13, "Missing { in info file\n");
            return count;
        }
        if (count == max)
        {
            Com_Printf(13, "Max infos exceeded\n");
            return count;
        }
        info[0] = 0;
        while (1)
        {
            token = (const char *)Com_Parse(&buf);
            if (!*token)
                break;
            if (!strcmp(token, "}"))
                goto LABEL_14;
            I_strncpyz(key, (char *)token, 1024);
            token = (const char *)Com_ParseOnLine(&buf);
            if (!*token)
                token = "<NULL>";
            Info_SetValueForKey(info, key, token);
        }
        Com_Printf(13, "Unexpected end of info file\n");
    LABEL_14:
        v8 = strlen(va("%d", 64));
        v3 = UI_Alloc(strlen(info) + v8 + 6, 1);
        infos[count] = (char *)v3;
        if (infos[count])
        {
            v7 = info;
            v6 = infos[count];
            do
            {
                v5 = *v7;
                *v6++ = *v7++;
            } while (v5);
            ++count;
        }
    }
}

void __cdecl UI_LoadArenas()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    const char *pszText; // [esp+0h] [ebp-14h] BYREF
    const char *pszGameTypes; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    int n; // [esp+Ch] [ebp-8h]
    const char *pszToken; // [esp+10h] [ebp-4h]

    sharedUiInfo.mapCount = 0;
    UI_LoadArenasFromFile();
    for (n = 0; n < ui_numArenas; ++n)
    {
        v0 = Info_ValueForKey(ui_arenaInfos[n], "map");
        sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5119] = (Material *)String_Alloc(v0);
        v1 = Info_ValueForKey(ui_arenaInfos[n], "longname");
        sharedUiInfo.mapList[sharedUiInfo.mapCount].mapName = String_Alloc(v1);
        sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5082] = 0;
        v2 = va("loadscreen_%s", (const char *)sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5119]);
        sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5118] = (Material *)String_Alloc(v2);
        sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5082] = Material_RegisterHandle(
            (char *)sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5118],
            3);
        pszGameTypes = Info_ValueForKey(ui_arenaInfos[n], "gametype");
        if (!pszGameTypes)
            goto LABEL_15;
        if (*pszGameTypes)
        {
            sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5115] = 0;
            v3 = va(".arena files : %s", (const char *)sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5119]);
            Com_BeginParseSession(v3);
            pszText = pszGameTypes;
            while (1)
            {
                pszToken = (const char *)Com_Parse(&pszText);
                if (!pszToken || !*pszToken)
                    break;
                for (i = 0; i < sharedUiInfo.numGameTypes; ++i)
                {
                    if (!I_stricmp(pszToken, sharedUiInfo.gameTypes[i].gameType))
                        sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5115] = (Material *)((int)sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5115]
                            | (1 << i));
                }
            }
            Com_EndParseSession();
        }
        else
        {
        LABEL_15:
            sharedUiInfo.serverHardwareIconList[40 * sharedUiInfo.mapCount - 5115] = (Material *)-1;
        }
        if (++sharedUiInfo.mapCount >= 128)
            break;
    }
}

const char *UI_LoadArenasFromFile_LoadObj()
{
    char *result; // eax
    const char *v1; // [esp+14h] [ebp-24A4h]
    char string[132]; // [esp+18h] [ebp-24A0h] BYREF
    char *v3; // [esp+9Ch] [ebp-241Ch]
    char listbuf[1024]; // [esp+A0h] [ebp-2418h] BYREF
    char buffer[8196]; // [esp+4A0h] [ebp-2018h] BYREF
    int len; // [esp+24A8h] [ebp-10h]
    int f; // [esp+24ACh] [ebp-Ch] BYREF
    int v8; // [esp+24B0h] [ebp-8h]
    unsigned int v9; // [esp+24B4h] [ebp-4h]

    ui_numArenas = 0;
    result = (char *)FS_GetFileList("mp", "arena", FS_LIST_PURE_ONLY, listbuf, 1024);
    v1 = result;
    v3 = listbuf;
    v8 = 0;
    while (v8 < (int)v1)
    {
        v9 = strlen(v3);
        snprintf(string, ARRAYSIZE(string), "%s/%s", "mp", v3);
        len = FS_FOpenFileByMode(string, &f, FS_READ);
        if (f)
        {
            if (len < 0x2000)
            {
                FS_Read((unsigned __int8 *)buffer, len, f);
                buffer[len] = 0;
                FS_FCloseFile(f);
                ui_numArenas += UI_ParseInfos(buffer, 64 - ui_numArenas, &ui_arenaInfos[ui_numArenas]);
            }
            else
            {
                Com_PrintError(13, "file too large: %s is %i, max allowed is %i", string, len, 0x2000);
                FS_FCloseFile(f);
            }
        }
        else
        {
            Com_PrintError(13, "file not found: %s\n", string);
        }
        ++v8;
        result = &v3[v9 + 1];
        v3 = result;
    }
    return result;
}

void UI_LoadArenasFromFile()
{
    if (IsFastFileLoad())
        UI_LoadArenasFromFile_FastFile();
    else
        UI_LoadArenasFromFile_LoadObj();
}

void UI_LoadArenasFromFile_FastFile()
{
    RawFile *rawfile; // [esp+8h] [ebp-8h]

    rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, "mp/cod2maps.arena").rawfile;
    if (rawfile)
        ui_numArenas = UI_ParseInfos(rawfile->buffer, 64 - ui_numArenas, &ui_arenaInfos[ui_numArenas]);
    else
        Com_PrintError(13, "file not found: %s\n", "mp/cod2maps.arena");
}

