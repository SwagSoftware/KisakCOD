#include "cg_local.h"
#include <sound/snd_public.h>
#include <cgame_mp/cg_local_mp.h>
#include <client/client.h>
#include <database/database.h>
#include <client_mp/client_mp.h>

void __cdecl CG_LoadingString(int32_t localClientNum, const char *s)
{
    CG_GetLocalClientGlobals(localClientNum)->isLoading = *s != 0;
    if (s && *s)
    {
        Com_Printf(14, va("LOADING... %s\n", s));
    }
    SCR_UpdateLoadScreen();
}

BOOL __cdecl CG_IsShowingProgress_LoadObj()
{
    return com_expectedHunkUsage > 0;
}

void __cdecl CG_DrawInformation(int32_t localClientNum)
{
    int32_t v1; // [esp+20h] [ebp-30h]
    uint8_t (*v2)(void); // [esp+24h] [ebp-2Ch]
    bool serverLoading; // [esp+2Bh] [ebp-25h]
    Font_s *font; // [esp+30h] [ebp-20h]
    float width; // [esp+34h] [ebp-1Ch]
    char *s; // [esp+3Ch] [ebp-14h]
    char *sa; // [esp+3Ch] [ebp-14h]
    const char *dots; // [esp+40h] [ebp-10h]
    float x; // [esp+44h] [ebp-Ch]
    float fontScale; // [esp+4Ch] [ebp-4h]

    fontScale = 0.5;
    serverLoading = CL_IsServerLoadingMap();
    if (serverLoading)
    {
        if (!CL_IsWaitingOnServerToLoadMap(localClientNum))
        {
            CG_CloseScriptMenu(localClientNum, 0);
            UI_CloseAllMenus(localClientNum);
            SND_StopSounds(SND_STOP_ALL);
            CL_SetWaitingOnServerToLoadMap(localClientNum, 1);
        }
    }
    else
    {
        CL_SetWaitingOnServerToLoadMap(localClientNum, 0);
    }
    CL_GetConfigString(localClientNum, 0);
    UI_DrawMapLevelshot(localClientNum);
    if (useFastFile->current.enabled)
        v2 = (uint8_t (*)(void))CG_IsShowingProgress_FastFile;
    else
        v2 = (uint8_t (*)(void))CG_IsShowingProgress_LoadObj;
    if (!v2() && serverLoading && g_waitingForServer)
    {
        font = UI_GetFontHandle(&scrPlaceFull, 0, fontScale);
        v1 = ((int)Sys_Milliseconds() / 500) & 3;
        switch (v1)
        {
        case 1:
            dots = ".";
            break;
        case 2:
            dots = "..";
            break;
        case 3:
            dots = "...";
            break;
        default:
            dots = "";
            break;
        }
        s = UI_SafeTranslateString("CGAME_WAITINGFORSERVERLOAD");
        width = (float)UI_TextWidth(s, 0, font, fontScale);
        sa = va("%s%s", s, dots);
        x = (640.0 - width) * 0.5;
        UI_DrawText(&scrPlaceView[localClientNum], sa, 0x7FFFFFFF, font, x, 439.0, 0, 0, fontScale, colorWhite, 3);
    }
}

bool __cdecl CG_IsShowingProgress_FastFile()
{
    return DB_GetLoadedFraction() > 0.0;
}

