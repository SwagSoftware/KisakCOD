#include "ui_shared.h"
#include <universal/q_parse.h>
#include <client/client.h>
#include <qcommon/cmd.h>
#include <stringed/stringed_hooks.h>
#include <win32/win_storage.h>
#include <qcommon/mem_track.h>
#include <database/database.h>
#include <cgame/cg_local.h>

#include <algorithm>
#include <universal/profile.h>

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include "ui.h"
#include <game/savedevice.h>
#include <game/savememory.h>
#include <game/g_local.h>
#endif


int g_waitingForKey;
int g_editingField;
struct itemDef_s *g_editItem;
int g_debugMode;
void(__cdecl *captureFunc)(UiContext *, void *);
void *captureData;

struct commandDef_t // sizeof=0x8
{                                       // ...
    const char *name;                   // ...
    void(__cdecl *handler)(UiContext *, itemDef_s *, const char **); // ...
};

#ifdef KISAK_MP
const commandDef_t commandList[42] =
{
  { "fadein", Script_FadeIn },
  { "fadeout", Script_FadeOut },
  { "show", Script_Show },
  { "hide", Script_Hide },
  { "showMenu", Script_ShowMenu },
  { "hideMenu", Script_HideMenu },
  { "setcolor", Script_SetColor },
  { "open", Script_Open },
  { "close", Script_Close },
  { "ingameopen", Script_InGameOpen },
  { "ingameclose", Script_InGameClose },
  { "setbackground", Script_SetBackground },
  { "setitemcolor", Script_SetItemColor },
  { "focusfirst", Script_FocusFirstInMenu},
  { "setfocus", Script_SetFocus },
  { "setfocusbydvar", Script_SetFocusByDvar },
  { "setdvar", Script_SetDvar },
  { "exec", Script_Exec },
  { "execnow", Script_ExecNow },
  { "execOnDvarStringValue", Script_ExecOnDvarStringValue },
  { "execOnDvarIntValue", Script_ExecOnDvarIntValue },
  { "execOnDvarFloatValue", Script_ExecOnDvarFloatValue },
  { "execNowOnDvarStringValue", Script_ExecNowOnDvarStringValue },
  { "execNowOnDvarIntValue", Script_ExecNowOnDvarIntValue },
  { "execNowOnDvarFloatValue", Script_ExecNowOnDvarFloatValue },
  { "play", Script_Play },
  { "scriptmenuresponse", Script_ScriptMenuResponse },
  { "scriptMenuRespondOnDvarStringValue", Script_RespondOnDvarStringValue },
  { "scriptMenuRespondOnDvarIntValue", Script_RespondOnDvarIntValue },
  { "scriptMenuRespondOnDvarFloatValue", Script_RespondOnDvarFloatValue },
  { "setLocalVarBool", Script_SetLocalVarBool },
  { "setLocalVarInt", Script_SetLocalVarInt },
  { "setLocalVarFloat", Script_SetLocalVarFloat },
  { "setLocalVarString", Script_SetLocalVarString },
  { "feederTop",  Script_FeederTop },
  { "feederBottom",  Script_FeederBottom },
  { "openforgametype", Script_OpenForGameType },
  { "closeforgametype", Script_CloseForGameType },
  { "statclearperknew", Script_StatClearPerkNew },
  { "statsetusingtable", Script_StatSetUsingStatsTable },
  { "statclearbitmask", Script_StatClearBitMask },
  { "getautoupdate", Script_GetAutoUpdate }
}; // idb
#elif KISAK_SP
void Script_SaveGameHide(UiContext *dc, itemDef_s *item, const char **args)
{
    //const char **v6; // r3
    //unsigned int v7; // r11
    //int v8; // r31
    //char v9[1056]; // [sp+50h] [-420h] BYREF
    //
    //v7 = _cntlzw(SaveExists(CONSOLE_DEFAULT_SAVE_NAME));
    //v6 = args;
    //v8 = (v7 >> 5)  1;
    //if (String_Parse(v6, v9, 1024))
    //    Menu_ShowItemByName(dc->localClientNum, item->parent, v9, (unsigned __int8)v8);

    char parsedName[1024];
    int saveExists = SaveExists(CONSOLE_DEFAULT_SAVE_NAME);
    int shouldHide = !saveExists;  // 1 if save does not exist, 0 otherwise

    if (String_Parse(args, parsedName, sizeof(parsedName))) {
        Menu_ShowItemByName(dc->localClientNum, item->parent, parsedName, (unsigned char)shouldHide);
    }
}
void Script_SaveGameShow(UiContext *dc, itemDef_s *item, const char **args)
{
    const char **v6; // r3
    int v7; // r11
    bool v8; // r31
    char v9[1056]; // [sp+50h] [-420h] BYREF

    v7 = SaveExists(CONSOLE_DEFAULT_SAVE_NAME) - 1;
    v6 = args;
    v8 = v7 == 0;
    if (String_Parse(v6, v9, 1024))
        Menu_ShowItemByName(dc->localClientNum, item->parent, v9, v8);
}
void Script_ProfileHide(UiContext *dc, itemDef_s *item, const char **args)
{
    //int v6; // r3
    //const char **v7; // r3
    //unsigned int v8; // r11
    //int v9; // r30
    //char v10[1056]; // [sp+50h] [-420h] BYREF
    //
    //v6 = CL_ControllerIndexFromClientNum(dc->localClientNum);
    //v8 = _cntlzw(GamerProfile_IsProfileLoggedIn(v6));
    //v7 = args;
    //v9 = (v8 >> 5) & 1;
    //if (String_Parse(v7, v10, 1024))
    //    Menu_ShowItemByName(dc->localClientNum, item->parent, v10, (unsigned __int8)v9);
}
void Script_ProfileShow(UiContext *dc, itemDef_s *item, const char **args)
{
    //int v6; // r3
    //bool IsProfileLoggedIn; // r28
    //char v8[1072]; // [sp+50h] [-430h] BYREF
    //
    //v6 = CL_ControllerIndexFromClientNum(dc->localClientNum);
    //IsProfileLoggedIn = GamerProfile_IsProfileLoggedIn(v6);
    //if (String_Parse(args, v8, 1024))
    //    Menu_ShowItemByName(dc->localClientNum, item->parent, v8, IsProfileLoggedIn);
}

void Script_NoSaveHide(UiContext *dc, itemDef_s *item, const char **args)
{
    int localClientNum; // r30
    bool IsProfileLoggedIn; // r30
    int v8; // r3
    char v9[1072]; // [sp+50h] [-430h] BYREF

    localClientNum = dc->localClientNum;
    //if (SaveMemory_IsCurrentCommittedSaveValid())
    //{
    //    v8 = CL_ControllerIndexFromClientNum(localClientNum);
    //    IsProfileLoggedIn = GamerProfile_IsProfileLoggedIn(v8);
    //}
    //else
    {
        IsProfileLoggedIn = 0;
    }
    if (String_Parse(args, v9, 1024))
        Menu_ShowItemByName(dc->localClientNum, item->parent, v9, IsProfileLoggedIn);
}

void Script_SaveAvailableHide(UiContext *dc, itemDef_s *item, const char **args)
{
    int localClientNum; // r30
    bool IsProfileLoggedIn; // r30
    int v8; // r3
    char v9[1072]; // [sp+50h] [-430h] BYREF

    localClientNum = dc->localClientNum;
    //if (SaveMemory_IsCurrentCommittedSaveValid())
    //{
    //    v8 = CL_ControllerIndexFromClientNum(localClientNum);
    //    IsProfileLoggedIn = GamerProfile_IsProfileLoggedIn(v8);
    //}
    //else
    {
        IsProfileLoggedIn = 0;
    }
    if (String_Parse(args, v9, 1024))
        Menu_ShowItemByName(dc->localClientNum, item->parent, v9, !IsProfileLoggedIn);
}

void Script_DisplaySaveMessage(UiContext *dc, itemDef_s *item, const char **args)
{
    iassert(item);
    iassert(item->parent);
    iassert(item->parent->window.name);

    Dvar_SetBool(ui_isSaving, 1);
    ui_saveTimeGlob.isSaving = 1;
    ui_saveTimeGlob.saveTime = Sys_Milliseconds();
    ui_saveTimeGlob.hasfirstFrameShown = 0;
    ui_saveTimeGlob.saveMenuName = item->parent->window.name;
    Com_Printf(13, "Save message opened: %i\n", ui_saveTimeGlob.saveTime);
}

void Script_WriteSave(UiContext *dc, itemDef_s *item, const char **args)
{
    ui_saveTimeGlob.callWrite = 1;
}

void Script_SetSaveExecOnSuccess(UiContext *dc, itemDef_s *item, const char **args)
{
    ui_saveTimeGlob.hasExecOnSuccess = 1;
    String_Parse(args, ui_saveTimeGlob.execOnSuccess, 256);
}

void Script_ScriptNextLevel(UiContext *dc, itemDef_s *item, const char **args)
{
    G_LoadNextMap();
}

const commandDef_t commandList[46] =
{
  { "fadein", Script_FadeIn },
  { "fadeout", Script_FadeOut },
  { "show", Script_Show },
  { "hide", Script_Hide },
  { "showMenu", Script_ShowMenu },
  { "hideMenu", Script_HideMenu },
  { "setcolor", Script_SetColor },
  { "open", Script_Open },
  { "close", Script_Close },
  { "ingameopen", Script_InGameOpen },
  { "ingameclose", Script_InGameClose },
  { "setbackground", Script_SetBackground },
  { "setitemcolor", Script_SetItemColor },
  { "focusfirst", Script_FocusFirstInMenu },
  { "setfocus", Script_SetFocus },
  { "setfocusbydvar", Script_SetFocusByDvar },
  { "setdvar", Script_SetDvar },
  { "exec", Script_Exec },
  { "execnow", Script_ExecNow },
  { "execOnDvarStringValue", Script_ExecOnDvarStringValue },
  { "execOnDvarIntValue", Script_ExecOnDvarIntValue },
  { "execOnDvarFloatValue", Script_ExecOnDvarFloatValue },
  { "execNowOnDvarStringValue", Script_ExecNowOnDvarStringValue },
  { "execNowOnDvarIntValue", Script_ExecNowOnDvarIntValue },
  { "execNowOnDvarFloatValue", Script_ExecNowOnDvarFloatValue },
  { "play", Script_Play },
  { "scriptmenuresponse", Script_ScriptMenuResponse },
  { "scriptMenuRespondOnDvarStringValue", Script_RespondOnDvarStringValue },
  { "scriptMenuRespondOnDvarIntValue", Script_RespondOnDvarIntValue },
  { "scriptMenuRespondOnDvarFloatValue", Script_RespondOnDvarFloatValue },
  { "setLocalVarBool", Script_SetLocalVarBool },
  { "setLocalVarInt", Script_SetLocalVarInt },
  { "setLocalVarFloat", Script_SetLocalVarFloat },
  { "setLocalVarString", Script_SetLocalVarString },
  { "feederTop", Script_FeederTop },
  { "feederBottom", Script_FeederBottom },
  { "savegamehide", Script_SaveGameHide },
  { "savegameshow", Script_SaveGameShow },
  { "profilehide", Script_ProfileHide },
  { "profileshow", Script_ProfileShow },
  { "nosavehide", Script_NoSaveHide },
  { "saveAvailableHide", Script_SaveAvailableHide },
  { "saveDelay", Script_DisplaySaveMessage },
  { "writeSave", Script_WriteSave },
  { "setSaveExecOnSuccess", Script_SetSaveExecOnSuccess },
  { "nextlevel", Script_ScriptNextLevel }
};
#endif

bool __cdecl Window_IsVisible(int localClientNum, const windowDef_t *w)
{
    if (!w)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 60, 0, "%s", "w");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            23,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    return (w->dynamicFlags[localClientNum] & 4) != 0;
}

#ifdef KISAK_MP
void __cdecl Script_GetAutoUpdate(UiContext *dc, itemDef_s *item, const char **args)
{
    CL_GetAutoUpdate();
}
#endif

void __cdecl Script_StatClearPerkGetArg(
    UiContext *dc,
    itemDef_s *item,
    const char **args,
    char *refString,
    int refStringLen)
{
    char arg[1028]; // [esp+0h] [ebp-408h] BYREF

    if (!refString)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 1257, 0, "%s", "refString");
    String_Parse(args, arg, 1024);
    if (!I_stricmp(arg, "("))
        String_Parse(args, arg, 1024);
    if (!I_stricmp(arg, "\""))
        String_Parse(args, arg, 1024);
    I_strncpyz(refString, arg, refStringLen);
    if (!I_stricmp(arg, "\""))
        String_Parse(args, arg, 1024);
    if (!I_stricmp(arg, ")"))
        Com_UngetToken();
}

void __cdecl Script_StatClearPerkNew(UiContext *dc, itemDef_s *item, const char **args)
{
    const char *v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // [esp-8h] [ebp-420h]
    int v7; // [esp-4h] [ebp-41Ch]
    unsigned int v8; // [esp-4h] [ebp-41Ch]
    StringTable *table; // [esp+4h] [ebp-414h] BYREF
    int perkIndex; // [esp+8h] [ebp-410h]
    int statValue; // [esp+Ch] [ebp-40Ch]
    char refString[1028]; // [esp+10h] [ebp-408h] BYREF

#ifdef KISAK_NO_FASTFILES
    if (true)
#else
    if (IsFastFileLoad())
#endif
    {
        Script_StatClearPerkGetArg(dc, item, args, refString, 1024);
        StringTable_GetAsset("mp/statstable.csv", &table);
        v3 = StringTable_Lookup(table, 4, refString, 1);
        perkIndex = atoi(v3);
        if (perkIndex >= 150 && perkIndex <= 298)
        {
            v7 = perkIndex;
            v4 = CL_ControllerIndexFromClientNum(dc->localClientNum);
            statValue = LiveStorage_GetStat(v4, v7);
            if ((statValue & 3) != 0)
            {
                statValue &= ~2u;
                statValue |= 1u;
                v8 = statValue;
                v6 = perkIndex;
                v5 = CL_ControllerIndexFromClientNum(dc->localClientNum);
                LiveStorage_SetStat(v5, v6, v8);
            }
            else
            {
                Com_Error(ERR_DROP, "statClearPerkNew: perk %s[%d] isn't unlocked.\n", refString, perkIndex);
            }
        }
        else
        {
            Com_Error(ERR_DROP, "statClearPerkNew: Invalid perk index %d for %s\n", perkIndex, refString);
        }
    }
    else
    {
        Com_PrintWarning(13, "You can only do table lookups when using fastfiles.\n");
    }
}

void __cdecl Menu_Setup(UiContext *dc)
{
    dc->menuCount = 0;
    dc->openMenuCount = 0;
    Item_SetupKeywordHash();
    Menu_SetupKeywordHash();
    UILocalVar_Init(&dc->localVars);
}

void __cdecl Menu_FreeMemory(menuDef_t *menu)
{
    int item; // [esp+0h] [ebp-4h]

    for (item = 0; item < menu->itemCount; ++item)
        Menu_FreeItemMemory(menu->items[item]);
    free_expression(&menu->visibleExp);
    free_expression(&menu->rectXExp);
    free_expression(&menu->rectYExp);
}

void __cdecl Menus_FreeAllMemory(UiContext *dc)
{
    int menu; // [esp+0h] [ebp-4h]

    for (menu = 0; menu < dc->menuCount; ++menu)
        Menu_FreeMemory(dc->Menus[menu]);
}

void __cdecl LerpColor(float *a, float *b, float *c, float t)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        c[i] = (b[i] - a[i]) * t + a[i];
        if (c[i] >= 0.0)
        {
            if (c[i] > 1.0)
                c[i] = 1.0;
        }
        else
        {
            c[i] = 0.0;
        }
    }
}

int __cdecl Color_Parse(const char **p, float (*c)[4])
{
    float f; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        if (!Float_Parse(p, &f))
            return 0;
        (*c)[i] = f;
    }
    return 1;
}

int __cdecl String_Parse(const char **p, char *out, int len)
{
    char *pszTranslated; // [esp+0h] [ebp-8h]
    parseInfo_t *token; // [esp+4h] [ebp-4h]

    token = Com_ParseOnLine(p);
    if (!token)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 275, 0, "%s", "token");
    if (!*p)
        return 0;
    if (token->token[0] == 64)
    {
        pszTranslated = (char *)SEH_StringEd_GetString(&token->token[1]);
        if (pszTranslated)
        {
            I_strncpyz(out, pszTranslated, len);
            return 1;
        }
        if (Dvar_GetBool("loc_warnings"))
        {
            if (Dvar_GetBool("loc_warningsAsErrors"))
                Com_Error(ERR_LOCALIZATION, "Could not translate menu string reference %s", token->token);
            else
                Com_PrintWarning(13, "WARNING: Could not translate menu string reference %s\n", token->token);
        }
    }
    I_strncpyz(out, token->token, len);
    return 1;
}

void __cdecl Script_SetColor(UiContext *dc, itemDef_s *item, const char **args)
{
    float *out; // [esp+0h] [ebp-414h]
    char name[1028]; // [esp+4h] [ebp-410h] BYREF
    float f; // [esp+40Ch] [ebp-8h] BYREF
    int i; // [esp+410h] [ebp-4h]

    if (String_Parse(args, name, 1024))
    {
        out = 0;
        if (I_stricmp(name, "backcolor"))
        {
            if (I_stricmp(name, "forecolor"))
            {
                if (!I_stricmp(name, "bordercolor"))
                    out = item->window.borderColor;
            }
            else
            {
                out = item->window.foreColor;
                Window_AddDynamicFlags(dc->localClientNum, &item->window, 0x10000);
            }
        }
        else
        {
            out = item->window.backColor;
            Window_AddDynamicFlags(dc->localClientNum, &item->window, 0x8000);
        }
        if (out)
        {
            for (i = 0; i < 4 && Float_Parse(args, &f); ++i)
                out[i] = f;
        }
    }
}

void __cdecl Script_SetBackground(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
        item->window.background = Material_RegisterHandle(name, item->imageTrack);
}

void __cdecl Script_SetItemColor(UiContext *dc, itemDef_s *item, const char **args)
{
    int j; // [esp+0h] [ebp-828h]
    itemDef_s *item2; // [esp+4h] [ebp-824h]
    int count; // [esp+8h] [ebp-820h]
    float *out; // [esp+Ch] [ebp-81Ch]
    char itemname[1024]; // [esp+10h] [ebp-818h] BYREF
    char name[1024]; // [esp+410h] [ebp-418h] BYREF
    int i; // [esp+814h] [ebp-14h]
    float color[4]; // [esp+818h] [ebp-10h] BYREF

    if (String_Parse(args, itemname, 1024))
    {
        if (String_Parse(args, name, 1024))
        {
            count = Menu_ItemsMatchingGroup(item->parent, itemname);
            if (Color_Parse(args, (float (*)[4])color))
            {
                for (j = 0; j < count; ++j)
                {
                    item2 = Menu_GetMatchingItemByNumber(item->parent, j, itemname);
                    if (item2)
                    {
                        out = 0;
                        if (I_stricmp(name, "backcolor"))
                        {
                            if (I_stricmp(name, "forecolor"))
                            {
                                if (!I_stricmp(name, "bordercolor"))
                                    out = item2->window.borderColor;
                            }
                            else
                            {
                                out = item2->window.foreColor;
                                Window_AddDynamicFlags(dc->localClientNum, &item2->window, 0x10000);
                            }
                        }
                        else
                        {
                            out = item2->window.backColor;
                        }
                        if (out)
                        {
                            for (i = 0; i < 4; ++i)
                                out[i] = color[i];
                        }
                    }
                }
            }
        }
    }
}

int __cdecl Menu_ItemsMatchingGroup(menuDef_t *menu, char *name)
{
    int v2; // eax
    int wildcard; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    count = 0;
    wildcard = -1;
    v2 = (int)strstr(name, "*");
    if (v2)
        wildcard = v2 - (unsigned int)name;
    for (i = 0; i < menu->itemCount; ++i)
    {
        if (wildcard == -1)
        {
            if (menu->items[i]->window.name && !I_stricmp(menu->items[i]->window.name, name)
                || menu->items[i]->window.group && !I_stricmp(menu->items[i]->window.group, name))
            {
                ++count;
            }
        }
        else if (menu->items[i]->window.name && !I_strncmp(menu->items[i]->window.name, name, wildcard)
            || menu->items[i]->window.group && !I_strncmp(menu->items[i]->window.group, name, wildcard))
        {
            ++count;
        }
    }
    return count;
}

itemDef_s *__cdecl Menu_GetMatchingItemByNumber(menuDef_t *menu, int index, char *name)
{
    int v3; // eax
    int wildcard; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    count = 0;
    wildcard = -1;
    v3 = (int)strstr(name, "*");
    if (v3)
        wildcard = v3 - (unsigned int)name;
    for (i = 0; i < menu->itemCount; ++i)
    {
        if (wildcard == -1)
        {
            if (menu->items[i]->window.name && !I_stricmp(menu->items[i]->window.name, name)
                || menu->items[i]->window.group && !I_stricmp(menu->items[i]->window.group, name))
            {
                if (count == index)
                    return menu->items[i];
                ++count;
            }
        }
        else if (menu->items[i]->window.name && !I_strncmp(menu->items[i]->window.name, name, wildcard)
            || menu->items[i]->window.group && !I_strncmp(menu->items[i]->window.group, name, wildcard))
        {
            if (count == index)
                return menu->items[i];
            ++count;
        }
    }
    return 0;
}

int __cdecl Menus_MenuIsInStack(UiContext *dc, menuDef_t *menu)
{
    PROF_SCOPED("Menus_MenuIsInStack");

    int menuIndex; // [esp+0h] [ebp-4h]

    iassert(dc);
    iassert(menu);

    for (menuIndex = dc->openMenuCount - 1; menuIndex >= 0; --menuIndex)
    {
        if (dc->menuStack[menuIndex] == menu)
            return 1;
    }
    return 0;
}

menuDef_t *__cdecl Menus_FindByName(const UiContext *dc, const char *p)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < dc->menuCount; ++i)
    {
        if (!I_stricmp(dc->Menus[i]->window.name, p))
            return dc->Menus[i];
    }
    return 0;
}

void __cdecl Menus_HideByName(const UiContext *dc, const char *menuName)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    if (!dc)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 820, 0, "%s", "dc");
    if (!menuName)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 821, 0, "%s", "menuName");
    menu = Menus_FindByName(dc, menuName);
    if (menu)
        Window_RemoveDynamicFlags(dc->localClientNum, &menu->window, 4);
}

void __cdecl Menus_ShowByName(const UiContext *dc, const char *windowName)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    if (!dc)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 833, 0, "%s", "dc");
    if (!windowName)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 834, 0, "%s", "windowName");
    menu = Menus_FindByName(dc, windowName);
    if (menu)
        Window_AddDynamicFlags(dc->localClientNum, &menu->window, 4);
}

void __cdecl Menus_CloseByName(UiContext *dc, const char *p)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    menu = Menus_FindByName(dc, p);
    if (menu)
        Menus_Close(dc, menu);
}

void __cdecl Menus_Close(UiContext *dc, menuDef_t *menu)
{
    bool noFocus; // [esp+1Eh] [ebp-Ah]
    bool hadFocus; // [esp+1Fh] [ebp-9h]
    int menuNum; // [esp+20h] [ebp-8h]
    int menuNuma; // [esp+20h] [ebp-8h]
    int menuNumb; // [esp+20h] [ebp-8h]
    menuDef_t *menuDef; // [esp+24h] [ebp-4h]
    menuDef_t *menuDefa; // [esp+24h] [ebp-4h]

    if (!dc)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 934, 0, "%s", "dc");
    if (!menu)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 935, 0, "%s", "menu");
    if (Menus_MenuIsInStack(dc, menu))
    {
        Menu_RunCloseScript(dc, menu);
        hadFocus = Window_HasFocus(dc->localClientNum, &menu->window);
        Menus_RemoveFromStack(dc, menu);
        if (hadFocus)
        {
            for (menuNum = dc->openMenuCount - 1; menuNum >= 0; --menuNum)
            {
                menuDef = dc->menuStack[menuNum];
                if (Window_IsVisible(dc->localClientNum, &menuDef->window))
                {
                    Menu_GainFocusDueToClose(dc, menuDef);
                    break;
                }
            }
        }
        if (dc->openMenuCount > 0)
        {
            noFocus = 1;
            for (menuNuma = 0; menuNuma < dc->openMenuCount; ++menuNuma)
            {
                menuDefa = dc->menuStack[menuNuma];
                if (Window_IsVisible(dc->localClientNum, &menuDefa->window)
                    && Window_HasFocus(dc->localClientNum, &menuDefa->window))
                {
                    noFocus = 0;
                    break;
                }
            }
            if (noFocus)
            {
                if (!menu->window.name)
                    MyAssertHandler(".\\ui\\ui_shared.cpp", 976, 0, "%s", "menu->window.name");
                Com_PrintWarning(13, "WARNING: No menu has focus after closing %s.\n Active menus: \n", menu->window.name);
                for (menuNumb = 0; menuNumb < dc->openMenuCount; ++menuNumb)
                    Com_PrintWarning(13, "  %d:  %s\n", menuNumb, dc->menuStack[menuNumb]->window.name);
            }
        }
    }
    Window_RemoveDynamicFlags(dc->localClientNum, &menu->window, 6);
}

bool __cdecl Window_HasFocus(int localClientNum, const windowDef_t *w)
{
    if (!w)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 70, 0, "%s", "w");
    if (Window_IsVisible(localClientNum, w))
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                23,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        return (w->dynamicFlags[localClientNum] & 2) != 0;
    }
    else
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                23,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        if ((w->dynamicFlags[localClientNum] & 2) != 0 && !alwaysfails)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 74, 0, "Hidden window has focus!");
        return 0;
    }
}

int __cdecl Menus_RemoveFromStack(UiContext *dc, menuDef_t *pMenu)
{
    int i; // [esp+4h] [ebp-4h]

    for (i = dc->openMenuCount - 1; ; --i)
    {
        if (i < 0)
            return 0;
        if (dc->menuStack[i] == pMenu)
            break;
    }
    --dc->openMenuCount;
    while (i < dc->openMenuCount)
    {
        dc->menuStack[i] = dc->menuStack[i + 1];
        ++i;
    }
    return 1;
}

void __cdecl Menu_GainFocusDueToClose(UiContext *dc, menuDef_t *menu)
{
    if (!dc)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 904, 0, "%s", "dc");
    if (!menu)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 905, 0, "%s", "menu");
    if (Window_HasFocus(dc->localClientNum, &menu->window))
        MyAssertHandler(".\\ui\\ui_shared.cpp", 907, 0, "%s", "!Window_HasFocus( dc->localClientNum, &menu->window )");
    Window_AddDynamicFlags(dc->localClientNum, &menu->window, 2);
    Menu_CallOnFocusDueToOpen(dc, menu);
}

void __cdecl Menu_CallOnFocusDueToOpen(UiContext *dc, menuDef_t *menu)
{
    bool anyFound; // [esp+Fh] [ebp-5h]
    int i; // [esp+10h] [ebp-4h]

    anyFound = 0;
    for (i = 0; i < menu->itemCount; ++i)
    {
        if (Window_HasFocus(dc->localClientNum, &menu->items[i]->window))
        {
            if (menu->items[i]->onFocus)
                Item_RunScript(dc, menu->items[i], (char *)menu->items[i]->onFocus);
            if (anyFound)
                MyAssertHandler(".\\ui\\ui_shared.cpp", 893, 0, "%s", "!anyFound");
            anyFound = 1;
        }
    }
}

void __cdecl Menu_RunCloseScript(UiContext *dc, menuDef_t *menu)
{
    itemDef_s item; // [esp+4h] [ebp-178h] BYREF

    if (!menu)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 916, 0, "%s", "menu");
    if (Window_IsVisible(dc->localClientNum, &menu->window))
    {
        if (menu->onClose)
        {
            item.parent = menu;
            Item_RunScript(dc, &item, (char*)menu->onClose);
        }
    }
}

itemDef_s *__cdecl Menu_FindItemByName(menuDef_t *menu, const char *p)
{
    int i; // [esp+0h] [ebp-4h]

    if (!menu || !p)
        return 0;
    for (i = 0; i < menu->itemCount; ++i)
    {
        if (menu->items[i]->window.name && !I_stricmp(p, menu->items[i]->window.name))
            return menu->items[i];
    }
    return 0;
}

void __cdecl Menus_CloseAll(UiContext *dc)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < dc->menuCount; ++i)
        Menus_Close(dc, dc->Menus[i]);
}

void __cdecl Script_Show(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
        Menu_ShowItemByName(dc->localClientNum, item->parent, name, 1);
}

void __cdecl Menu_ShowItemByName(int localClientNum, menuDef_t *menu, char *p, int bShow)
{
    itemDef_s *item; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    count = Menu_ItemsMatchingGroup(menu, p);
    for (i = 0; i < count; ++i)
    {
        item = Menu_GetMatchingItemByNumber(menu, i, p);
        if (item)
        {
            if (bShow)
                Window_AddDynamicFlags(localClientNum, &item->window, 4);
            else
                Window_RemoveDynamicFlags(localClientNum, &item->window, 4);
        }
    }
}

void __cdecl Script_Hide(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
        Menu_ShowItemByName(dc->localClientNum, item->parent, name, 0);
}

void __cdecl Script_FadeIn(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
        Menu_FadeItemByName(dc->localClientNum, item->parent, name, 0);
}

void __cdecl Menu_FadeItemByName(int localClientNum, menuDef_t *menu, char *p, int fadeOut)
{
    itemDef_s *item; // [esp+0h] [ebp-Ch]
    int i; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    count = Menu_ItemsMatchingGroup(menu, p);
    for (i = 0; i < count; ++i)
    {
        item = Menu_GetMatchingItemByNumber(menu, i, p);
        if (item)
        {
            if (fadeOut)
            {
                Window_AddDynamicFlags(localClientNum, &item->window, 20);
                Window_RemoveDynamicFlags(localClientNum, &item->window, 32);
            }
            else
            {
                Window_AddDynamicFlags(localClientNum, &item->window, 36);
                Window_RemoveDynamicFlags(localClientNum, &item->window, 16);
            }
        }
    }
}

void __cdecl Script_FadeOut(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
        Menu_FadeItemByName(dc->localClientNum, item->parent, name, 1);
}

void __cdecl Script_ShowMenu(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
        Menus_ShowByName(dc, name);
}

void __cdecl Script_HideMenu(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
        Menus_HideByName(dc, name);
}

void __cdecl Script_Open(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
        Menus_OpenByName(dc, name);
}

void __cdecl Script_OpenForGameType(UiContext *dc, itemDef_s *item, const char **args)
{
    const char *String; // eax
    const char *v4; // eax
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
    {
        String = Dvar_GetString(item->dvar);
        v4 = va(name, String);
        Menus_OpenByName(dc, v4);
    }
}

void __cdecl Script_CloseForGameType(UiContext *dc, itemDef_s *item, const char **args)
{
    const char *String; // eax
    const char *v4; // eax
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
    {
        String = Dvar_GetString(item->dvar);
        v4 = va(name, String);
        Menus_CloseByName(dc, v4);
    }
}

void __cdecl Script_ValidateStat(int controllerIndex, int index, int value)
{
    if (index < 2400 || index > 2402)
    {
        if (index < 200 || index >= 250)
            Com_Error(ERR_DROP, "statsetusingtable should only be used for create-a-class.  You can't set stat %d\n", index);
        LiveStorage_ValidateCaCStat(controllerIndex, index, value);
    }
}

void __cdecl Script_StatSetUsingStatsTable(UiContext *dc, itemDef_s *item, const char **args)
{
    const char *v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // [esp-8h] [ebp-824h]
    int v7; // [esp-8h] [ebp-824h]
    int v8; // [esp-4h] [ebp-820h]
    unsigned int v9; // [esp-4h] [ebp-820h]
    char searchValue[1024]; // [esp+4h] [ebp-818h] BYREF
    int statNum; // [esp+404h] [ebp-418h]
    StringTable *table; // [esp+408h] [ebp-414h] BYREF
    int returnValueColumn; // [esp+40Ch] [ebp-410h]
    int comparisonColumn; // [esp+410h] [ebp-40Ch]
    char arg[1024]; // [esp+414h] [ebp-408h] BYREF
    int newStatValue; // [esp+818h] [ebp-4h]

#ifdef KISAK_NO_FASTFILES
    if (true)
#else
    if (IsFastFileLoad())
#endif
    {
        String_Parse(args, arg, 1024);
        if (!I_stricmp(arg, "("))
            String_Parse(args, arg, 1024);
        statNum = atoi(arg);
        String_Parse(args, arg, 1024);
        while (!I_stricmp(arg, "tablelookup") || !I_stricmp(arg, ",") || !I_stricmp(arg, "("))
            String_Parse(args, arg, 1024);
        StringTable_GetAsset(arg, &table);
        String_Parse(args, arg, 1024);
        while (!I_stricmp(arg, ","))
            String_Parse(args, arg, 1024);
        comparisonColumn = atoi(arg);
        String_Parse(args, arg, 1024);
        while (!I_stricmp(arg, ","))
            String_Parse(args, arg, 1024);
        I_strncpyz(searchValue, arg, 1024);
        String_Parse(args, arg, 1024);
        while (!I_stricmp(arg, ","))
            String_Parse(args, arg, 1024);
        returnValueColumn = atoi(arg);
        v3 = StringTable_Lookup(table, comparisonColumn, searchValue, returnValueColumn);
        newStatValue = atoi(v3);
        v8 = newStatValue;
        v6 = statNum;
        v4 = CL_ControllerIndexFromClientNum(dc->localClientNum);
        Script_ValidateStat(v4, v6, v8);
        v9 = newStatValue;
        v7 = statNum;
        v5 = CL_ControllerIndexFromClientNum(dc->localClientNum);
        LiveStorage_SetStat(v5, v7, v9);
        String_Parse(args, arg, 1024);
        while (!I_stricmp(arg, ")"))
            String_Parse(args, arg, 1024);
        Com_UngetToken();
    }
    else
    {
        Com_PrintWarning(13, "You can only do table lookups when using fastfiles.\n");
    }
}

void __cdecl Script_StatClearBitMask(UiContext *dc, itemDef_s *item, const char **args)
{
    int bitMask; // [esp+0h] [ebp-14h] BYREF
    int oldStatValue; // [esp+4h] [ebp-10h]
    int controllerIndex; // [esp+8h] [ebp-Ch]
    int statNum; // [esp+Ch] [ebp-8h] BYREF
    int newStatValue; // [esp+10h] [ebp-4h]

    Script_StatBitMaskGetArgs(dc, item, args, &statNum, &bitMask);
    controllerIndex = CL_ControllerIndexFromClientNum(dc->localClientNum);
    oldStatValue = LiveStorage_GetStat(controllerIndex, statNum);
    newStatValue = oldStatValue & ~bitMask;
    LiveStorage_SetStat(controllerIndex, statNum, newStatValue);
}

void __cdecl Script_StatBitMaskGetArgs(UiContext *dc, itemDef_s *item, const char **args, int *statNum, int *bitMask)
{
    char arg[1028]; // [esp+0h] [ebp-408h] BYREF

    if (!statNum)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 1229, 0, "%s", "statNum");
    if (!bitMask)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 1230, 0, "%s", "bitMask");
    String_Parse(args, arg, 1024);
    if (!I_stricmp(arg, "("))
        String_Parse(args, arg, 1024);
    *statNum = Dvar_GetInt(arg);
    String_Parse(args, arg, 1024);
    while (!I_stricmp(arg, ","))
        String_Parse(args, arg, 1024);
    *bitMask = Dvar_GetInt(arg);
    String_Parse(args, arg, 1024);
    while (!I_stricmp(arg, ")"))
        String_Parse(args, arg, 1024);
    Com_UngetToken();
}

void __cdecl Script_Close(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+14h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
    {
        if (!strcmp(name, "self"))
            Menus_Close(dc, item->parent);
        else
            Menus_CloseByName(dc, name);
    }
}

void __cdecl Script_InGameOpen(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
    {
        if (CL_IsLocalClientInGame(dc->localClientNum))
            Menus_OpenByName(dc, name);
    }
}

void __cdecl Script_InGameClose(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, name, 1024))
    {
        if (CL_IsLocalClientInGame(dc->localClientNum))
            Menus_CloseByName(dc, name);
    }
}

void __cdecl Script_FocusFirstInMenu(UiContext *dc, itemDef_s *item, const char** args)
{
    const char *name; // [esp+0h] [ebp-Ch]
    itemDef_s *focusItem; // [esp+8h] [ebp-4h]

    focusItem = Menu_FocusFirstSelectableItem(dc, item->parent);
    if (focusItem)
    {
        if (Item_IsTextField(focusItem))
            Item_TextField_BeginEdit(dc->localClientNum, focusItem);
    }
    else if (item->parent->window.name)
    {
        Com_Printf(13, "focusFirst: no itemDefs in %s were selectable\n", item->parent->window.name);
    }
    else
    {
        if (item->window.name)
            name = item->window.name;
        else
            name = "itemDef's unnamed menu";
        Com_Printf(13, "focusFirst: no itemDefs in %s were selectable\n", name);
    }
}

BOOL __cdecl IsVisible(char flags)
{
    return (flags & 4) != 0 && (flags & 0x10) == 0;
}

int __cdecl Item_ListBox_OverLB(int localClientNum, itemDef_s *item, float x, float y)
{
    float thumbstart; // [esp+Ch] [ebp-28h]
    float thumbstarta; // [esp+Ch] [ebp-28h]
    rectDef_s r; // [esp+10h] [ebp-24h] BYREF
    const rectDef_s *rect; // [esp+2Ch] [ebp-8h]
    int count; // [esp+30h] [ebp-4h]

    if (!item)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2467, 0, "%s", "item");
    count = UI_FeederCount(localClientNum, item->special);
    if (!Item_GetListBoxDef(item))
        return 0;
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    rect = &item->window.rect;
    r.horzAlign = item->window.rect.horzAlign;
    r.vertAlign = item->window.rect.vertAlign;
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 53, 0, "%s", "w");
    if ((item->window.staticFlags & 0x200000) != 0)
    {
        r.x = rect->x;
        r.y = rect->y + rect->h - 16.0;
        r.w = 16.0;
        r.h = 16.0;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 256;
        r.x = rect->x + rect->w - 16.0;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 512;
        thumbstart = Item_ListBox_ThumbPosition(localClientNum, item);
        r.x = thumbstart;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 1024;
        r.x = rect->x + 16.0;
        r.w = thumbstart - r.x;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 2048;
        r.x = thumbstart + 16.0;
        r.w = rect->x + rect->w - 16.0;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 4096;
    }
    else
    {
        r.x = rect->x + rect->w - 16.0;
        r.y = rect->y;
        r.w = 16.0;
        r.h = 16.0;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 256;
        r.y = rect->y + rect->h - 16.0;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 512;
        thumbstarta = Item_ListBox_ThumbPosition(localClientNum, item);
        r.y = thumbstarta;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 1024;
        r.y = rect->y + 16.0;
        r.h = thumbstarta - r.y;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 2048;
        r.y = thumbstarta + 16.0;
        r.h = rect->y + rect->h - 16.0;
        if (Rect_ContainsPoint(localClientNum, &r, x, y))
            return 4096;
    }
    return 0;
}

void __cdecl Item_ListBox_MouseEnter(int localClientNum, itemDef_s *item, float x, float y)
{
    int v4; // eax
    int v5; // [esp+8h] [ebp-68h]
    int v6; // [esp+Ch] [ebp-64h]
    float v7; // [esp+18h] [ebp-58h]
    float v8; // [esp+34h] [ebp-3Ch]
    rectDef_s r; // [esp+4Ch] [ebp-24h] BYREF
    listBoxDef_s *listPtr; // [esp+64h] [ebp-Ch]
    int mousePos; // [esp+68h] [ebp-8h]
    const rectDef_s *rect; // [esp+6Ch] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if (listPtr)
    {
        Window_RemoveDynamicFlags(localClientNum, &item->window, 7936);
        v4 = Item_ListBox_OverLB(localClientNum, item, x, y);
        Window_AddDynamicFlags(localClientNum, &item->window, v4);
        if (!item)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
        rect = &item->window.rect;
        r.horzAlign = item->window.rect.horzAlign;
        r.vertAlign = item->window.rect.vertAlign;
        if (!item)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 53, 0, "%s", "w");
        if ((item->window.staticFlags & 0x200000) != 0)
        {
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                    23,
                    0,
                    "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1);
            if ((item->window.dynamicFlags[localClientNum] & 0x1F00) == 0 && listPtr->elementStyle == 1)
            {
                r.x = rect->x;
                r.y = rect->y;
                r.h = rect->h - 16.0;
                r.w = rect->w - (double)listPtr->drawPadding;
                if (Rect_ContainsPoint(localClientNum, &r, x, y))
                {
                    v8 = (x - r.x) / listPtr->elementWidth;
                    mousePos = (int)(v8 - 0.4999999990686774) + listPtr->startPos[localClientNum];
                    if (listPtr->endPos[localClientNum] < mousePos)
                        v6 = listPtr->endPos[localClientNum];
                    else
                        v6 = mousePos;
                    listPtr->mousePos = v6;
                }
            }
        }
        else
        {
            if (localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                    23,
                    0,
                    "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1);
            if ((item->window.dynamicFlags[localClientNum] & 0x1F00) == 0)
            {
                r.x = rect->x;
                r.y = rect->y + 2.0;
                r.w = rect->w - 16.0;
                r.h = rect->h - (double)listPtr->drawPadding;
                if (Rect_ContainsPoint(localClientNum, &r, x, y))
                {
                    v7 = (y - r.y) / listPtr->elementHeight;
                    mousePos = (int)(v7 - 0.4999999990686774) + listPtr->startPos[localClientNum];
                    if (listPtr->endPos[localClientNum] < mousePos)
                        v5 = listPtr->endPos[localClientNum];
                    else
                        v5 = mousePos;
                    listPtr->mousePos = v5;
                }
            }
        }
    }
}

void __cdecl Item_MouseEnter(UiContext *dc, itemDef_s *item, float x, float y)
{
    int localClientNum; // [esp+8h] [ebp-24h]
    rectDef_s r; // [esp+Ch] [ebp-20h] BYREF
    int flags; // [esp+24h] [ebp-8h]
    const rectDef_s *textRect; // [esp+28h] [ebp-4h]

    if (item)
    {
        textRect = Item_GetTextRect(dc->localClientNum, item);
        r = *textRect;
        r.y = r.y - r.h;
        r.horzAlign = textRect->horzAlign;
        r.vertAlign = textRect->vertAlign;
        if ((item->dvarFlags & 3) == 0 || Item_EnableShowViaDvar(item, 1))
        {
            if (Item_IsVisible(dc->localClientNum, item))
            {
                localClientNum = dc->localClientNum;
                if (dc->localClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                        23,
                        0,
                        "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1);
                flags = item->window.dynamicFlags[localClientNum];
                if (Rect_ContainsPoint(dc->localClientNum, &r, x, y))
                {
                    if ((flags & 0x40) == 0)
                    {
                        Item_RunScript(dc, item, (char*)item->mouseEnterText);
                        Window_AddDynamicFlags(dc->localClientNum, &item->window, 64);
                    }
                    if ((flags & 1) == 0)
                    {
                        Item_RunScript(dc, item, (char *)item->mouseEnter);
                        Window_AddDynamicFlags(dc->localClientNum, &item->window, 1);
                    }
                }
                else
                {
                    if ((flags & 0x40) != 0)
                    {
                        Item_RunScript(dc, item, (char *)item->mouseExitText);
                        Window_RemoveDynamicFlags(dc->localClientNum, &item->window, 64);
                    }
                    if ((flags & 1) == 0)
                    {
                        Item_RunScript(dc, item, (char *)item->mouseEnter);
                        Window_AddDynamicFlags(dc->localClientNum, &item->window, 1);
                    }
                    if (item->type == 6)
                        Item_ListBox_MouseEnter(dc->localClientNum, item, x, y);
                }
            }
        }
    }
}

void __cdecl Item_MouseLeave(UiContext *dc, itemDef_s *item)
{
    int localClientNum; // [esp+4h] [ebp-4h]

    if (item)
    {
        localClientNum = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                23,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        if ((item->window.dynamicFlags[localClientNum] & 0x40) != 0)
        {
            Item_RunScript(dc, item, (char *)item->mouseExitText);
            Window_RemoveDynamicFlags(dc->localClientNum, &item->window, 64);
        }
        Item_RunScript(dc, item, (char *)item->mouseExit);
        Window_RemoveDynamicFlags(dc->localClientNum, &item->window, 768);
    }
    else
    {
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2674, 0, "%s", "item");
    }
}

void __cdecl Item_SetMouseOver(const UiContext *dc, itemDef_s *item, int focus)
{
    if (item)
    {
        if (focus)
            Window_AddDynamicFlags(dc->localClientNum, &item->window, 1);
        else
            Window_RemoveDynamicFlags(dc->localClientNum, &item->window, 1);
    }
    else
    {
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2692, 0, "%s", "item");
    }
}

itemDef_s *itemCapture;
BOOL __cdecl Menu_HandleMouseMove(UiContext *dc, menuDef_t *menu)
{
    const rectDef_s *v3; // eax
    itemDef_s *v4; // [esp+Ch] [ebp-4Ch]
    int v5; // [esp+10h] [ebp-48h]
    int v6; // [esp+18h] [ebp-40h]
    itemDef_s *v7; // [esp+1Ch] [ebp-3Ch]
    itemDef_s *v8; // [esp+2Ch] [ebp-2Ch]
    int v9; // [esp+30h] [ebp-28h]
    int localClientNum; // [esp+38h] [ebp-20h]
    int pass; // [esp+3Ch] [ebp-1Ch]
    itemDef_s *focusItem; // [esp+40h] [ebp-18h]
    int i; // [esp+44h] [ebp-14h]
    float x; // [esp+48h] [ebp-10h]
    float y; // [esp+4Ch] [ebp-Ch]
    int focusSet; // [esp+50h] [ebp-8h]
    itemDef_s *overItem; // [esp+54h] [ebp-4h]

    focusSet = 0;
    focusItem = 0;
    if (!dc->isCursorVisible)
        return 0;
    if (!menu)
        return 0;
    localClientNum = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            23,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if ((menu->window.dynamicFlags[localClientNum] & 0x4004) == 0)
        return 0;
    if (itemCapture)
        return 0;
    if (g_waitingForKey)
        return 0;
    x = dc->cursor.x;
    y = dc->cursor.y;
    for (pass = 0; pass < 2; ++pass)
    {
        for (i = menu->itemCount - 1; i >= 0; --i)
        {
            v8 = menu->items[i];
            v9 = dc->localClientNum;
            if (dc->localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                    23,
                    0,
                    "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    v9,
                    1);
            if ((v8->window.dynamicFlags[v9] & 0x4004) != 0
                && ((menu->items[i]->dvarFlags & 3) == 0 || Item_EnableShowViaDvar(menu->items[i], 1))
                && ((menu->items[i]->dvarFlags & 0xC) == 0 || Item_EnableShowViaDvar(menu->items[i], 4))
                && Item_IsVisible(dc->localClientNum, menu->items[i]))
            {
                if (Window_HasFocus(dc->localClientNum, &menu->items[i]->window) && !focusItem)
                    focusItem = menu->items[i];
                v7 = menu->items[i];
                if (!v7)
                    MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
                if (Rect_ContainsPoint(dc->localClientNum, &v7->window.rect, x, y))
                {
                    if (pass == 1)
                    {
                        overItem = menu->items[i];
                        if (overItem->type
                            || !overItem->text
                            || (v3 = Item_CorrectedTextRect(dc->localClientNum, overItem),
                                Rect_ContainsPoint(dc->localClientNum, v3, x, y)))
                        {
                            v6 = dc->localClientNum;
                            if (dc->localClientNum)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                                    23,
                                    0,
                                    "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                                    v6,
                                    1);
                            if (IsVisible(overItem->window.dynamicFlags[v6]))
                            {
                                Item_MouseEnter(dc, overItem, x, y);
                                if (!focusSet)
                                {
                                    focusSet = Item_SetFocus(dc, overItem, x, y);
                                    if (focusSet)
                                        focusItem = overItem;
                                }
                            }
                        }
                    }
                }
                else
                {
                    v4 = menu->items[i];
                    v5 = dc->localClientNum;
                    if (dc->localClientNum)
                        MyAssertHandler(
                            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                            23,
                            0,
                            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                            v5,
                            1);
                    if ((v4->window.dynamicFlags[v5] & 1) != 0)
                    {
                        Item_MouseLeave(dc, menu->items[i]);
                        Item_SetMouseOver(dc, menu->items[i], 0);
                    }
                }
            }
        }
    }
    if (!focusSet && focusItem && !Rect_ContainsPoint(dc->localClientNum, &focusItem->window.rect, x, y))
        Menu_ClearFocus(dc, menu);
    return focusSet != 0;
}

itemDef_s *__cdecl Menu_FocusFirstSelectableItem(UiContext *dc, menuDef_t *menu)
{
    int localClientNum; // [esp+8h] [ebp-8h]
    unsigned int cursor; // [esp+Ch] [ebp-4h]
    int cursora; // [esp+Ch] [ebp-4h]

    if (Menu_HandleMouseMove(dc, menu))
    {
        localClientNum = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        cursor = menu->cursorItem[localClientNum];
        if (cursor >= menu->itemCount)
            MyAssertHandler(
                ".\\ui\\ui_shared.cpp",
                1387,
                0,
                "cursor doesn't index menu->itemCount\n\t%i not in [0, %i)",
                cursor,
                menu->itemCount);
        return menu->items[cursor];
    }
    else
    {
        cursora = 0;
        Menu_SetCursorItem(dc->localClientNum, menu, 0);
        while (cursora < menu->itemCount)
        {
            if (Item_SetFocus(dc, menu->items[cursora], dc->cursor.x, dc->cursor.y))
                return menu->items[cursora];
            Menu_SetCursorItem(dc->localClientNum, menu, ++cursora);
        }
        Menu_SetCursorItem(dc->localClientNum, menu, 0);
        return 0;
    }
}

void __cdecl Script_SetFocus(UiContext *dc, itemDef_s *item, const char **args)
{
    char name[1028]; // [esp+8h] [ebp-410h] BYREF
    itemDef_s *focusItem; // [esp+410h] [ebp-8h]
    const rectDef_s *rect; // [esp+414h] [ebp-4h]

    if (String_Parse(args, name, 1024))
    {
        focusItem = Menu_FindItemByName(item->parent, name);
        if (focusItem)
        {
            rect = &focusItem->window.rect;
            if (Item_SetFocus(dc, focusItem, focusItem->window.rect.x, focusItem->window.rect.y))
            {
                if (Item_IsTextField(focusItem))
                    Item_TextField_BeginEdit(dc->localClientNum, focusItem);
            }
            else
            {
                Com_PrintError(13, "setFocus: error focusing widget '%s' (widget was found but could not accept focus)\n", name);
            }
        }
        else
        {
            Com_Printf(13, "setFocus: could not find widget named '%s'\n", name);
        }
    }
}

void __cdecl Script_SetFocusByDvar(UiContext *dc, itemDef_s *item, const char **args)
{
    itemDef_s *focusItem; // [esp+8h] [ebp-418h]
    menuDef_t *parent; // [esp+Ch] [ebp-414h]
    char dvarName[1028]; // [esp+10h] [ebp-410h] BYREF
    int i; // [esp+418h] [ebp-8h]
    const rectDef_s *rect; // [esp+41Ch] [ebp-4h]

    if (String_Parse(args, dvarName, 1024))
    {
        parent = item->parent;
        if (!parent)
            MyAssertHandler(".\\ui\\ui_shared.cpp", 1463, 0, "%s", "parent");
        for (i = 0; i < parent->itemCount; ++i)
        {
            focusItem = parent->items[i];
            if ((focusItem->dvarFlags & 0x10) != 0)
            {
                if (!focusItem->dvarTest)
                    Com_Error(ERR_DROP, "cript_SetFocusByDvar: Item's dvarTest field is empty.");
                if (!I_stricmp(focusItem->dvarTest, dvarName) && Item_EnableShowViaDvar(focusItem, 16))
                {
                    if (!focusItem)
                        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
                    rect = &focusItem->window.rect;
                    if (Item_SetFocus(dc, focusItem, focusItem->window.rect.x, focusItem->window.rect.y))
                        break;
                }
            }
        }
    }
}

void __cdecl Script_SetDvar(UiContext *dc, itemDef_s *item, const char **args)
{
    char val[1024]; // [esp+0h] [ebp-808h] BYREF
    char dvarName[1028]; // [esp+400h] [ebp-408h] BYREF

    if (String_Parse(args, dvarName, 1024))
    {
        if (String_Parse(args, val, 1024))
            Dvar_SetFromStringByName(dvarName, val);
    }
}

void __cdecl Script_Exec(UiContext *dc, itemDef_s *item, const char **args)
{
    int v3; // eax

    v3 = CL_ControllerIndexFromClientNum(dc->localClientNum);
    Script_ExecHandler(dc->localClientNum, v3, item, args, Script_AddTextWrapper);
}

void __cdecl Script_ExecHandler(
    int localClientNum,
    int controllerIndex,
    itemDef_s *item,
    const char **args,
    void(__cdecl *textCallback)(int, int, const char *))
{
    char val[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, val, 1023))
    {
        I_strncat(val, 1024, "\n");
        textCallback(localClientNum, controllerIndex, val);
    }
}

void __cdecl Script_AddTextWrapper(int clientNum, int controllerIndex, const char *text)
{
    Cbuf_AddText(clientNum, text);
}

void __cdecl Script_ExecNow(UiContext *dc, itemDef_s *item, const char **args)
{
    int v3; // eax

    v3 = CL_ControllerIndexFromClientNum(dc->localClientNum);
    Script_ExecHandler(dc->localClientNum, v3, item, args, (void(__cdecl *)(int, int, const char *))Cbuf_ExecuteBuffer);
}

void __cdecl Script_ExecOnDvarStringValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(dc->localClientNum, item, args, Script_ExecIfStringsEqual, Script_AddTextWrapper);
}

void __cdecl Script_ConditionalExecHandler(
    int localClientNum,
    itemDef_s *item,
    const char **args,
    bool(__cdecl *shouldExec)(const char *, const char *),
    void(__cdecl *textCallback)(int, int, const char *))
{
    int v5; // eax
    char testValue[1024]; // [esp+0h] [ebp-C08h] BYREF
    char dvarName[1024]; // [esp+400h] [ebp-808h] BYREF
    char command[1024]; // [esp+800h] [ebp-408h] BYREF
    const char *dvarValue; // [esp+C04h] [ebp-4h]

    if (String_Parse(args, dvarName, 1024) && String_Parse(args, testValue, 1024) && String_Parse(args, command, 1023))
    {
        dvarValue = Dvar_GetVariantString(dvarName);
        if (shouldExec(dvarValue, testValue))
        {
            I_strncat(command, 1024, "\n");
            v5 = CL_ControllerIndexFromClientNum(localClientNum);
            textCallback(localClientNum, v5, command);
        }
    }
}

bool __cdecl Script_ExecIfStringsEqual(const char *dvarValue, const char *testValue)
{
    return I_stricmp(dvarValue, testValue) == 0;
}

void __cdecl Script_ExecOnDvarIntValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(dc->localClientNum, item, args, Script_ExecIfIntsEqual, Script_AddTextWrapper);
}

bool __cdecl Script_ExecIfIntsEqual(const char *dvarValue, const char *testValue)
{
    int v2; // esi

    v2 = atoi(dvarValue);
    return v2 == atoi(testValue);
}

void __cdecl Script_ExecOnDvarFloatValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(dc->localClientNum, item, args, Script_ExecIfFloatsEqual, Script_AddTextWrapper);
}

bool __cdecl Script_ExecIfFloatsEqual(const char *dvarValue, const char *testValue)
{
    float v3; // [esp+4h] [ebp-10h]
    long double v4; // [esp+8h] [ebp-Ch]
    float v5; // [esp+10h] [ebp-4h]

    v4 = atof(testValue);
    v5 = atof(dvarValue) - v4;
    v3 = I_fabs(v5);
    return v3 < 0.000009999999747378752;
}

void __cdecl Script_ExecNowOnDvarStringValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(
        dc->localClientNum,
        item,
        args,
        Script_ExecIfStringsEqual,
        (void(__cdecl *)(int, int, const char *))Cbuf_ExecuteBuffer);
}

void __cdecl Script_ExecNowOnDvarIntValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(
        dc->localClientNum,
        item,
        args,
        Script_ExecIfIntsEqual,
        (void(__cdecl *)(int, int, const char *))Cbuf_ExecuteBuffer);
}

void __cdecl Script_ExecNowOnDvarFloatValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalExecHandler(
        dc->localClientNum,
        item,
        args,
        Script_ExecIfFloatsEqual,
        (void(__cdecl *)(int, int, const char *))Cbuf_ExecuteBuffer);
}

void __cdecl Script_RespondOnDvarStringValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalResponseHandler(dc->localClientNum, item, args, Script_ExecIfStringsEqual);
}

void __cdecl Script_ConditionalResponseHandler(
    int localClientNum,
    itemDef_s *item,
    const char **args,
    bool(__cdecl *shouldRespond)(const char *, const char *))
{
    int Int; // eax
    const char *v5; // eax
    int iIndex; // [esp+0h] [ebp-C10h]
    const char *pszName; // [esp+4h] [ebp-C0Ch]
    char testValue[1024]; // [esp+8h] [ebp-C08h] BYREF
    char dvarName[1024]; // [esp+408h] [ebp-808h] BYREF
    char command[1024]; // [esp+808h] [ebp-408h] BYREF
    const char *dvarValue; // [esp+C0Ch] [ebp-4h]

    if (String_Parse(args, dvarName, 1024))
    {
        if (String_Parse(args, testValue, 1024))
        {
            if (String_Parse(args, command, 1024))
            {
                dvarValue = Dvar_GetVariantString(dvarName);
                if (shouldRespond(dvarValue, testValue))
                {
                    for (iIndex = 0; iIndex < 32; ++iIndex)
                    {
                        pszName = CL_GetConfigString(localClientNum, iIndex + 1970);
                        if (*pszName)
                        {
                            if (!I_stricmp(item->parent->window.name, pszName))
                                break;
                        }
                    }
                    if (iIndex == 32)
                        iIndex = -1;
                    Int = Dvar_GetInt("sv_serverId");
                    v5 = va("cmd mr %i %i %s\n", Int, iIndex, command);
                    Cbuf_AddText(localClientNum, v5);
                }
            }
        }
    }
}

void __cdecl Script_RespondOnDvarIntValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalResponseHandler(dc->localClientNum, item, args, Script_ExecIfIntsEqual);
}

void __cdecl Script_RespondOnDvarFloatValue(UiContext *dc, itemDef_s *item, const char **args)
{
    Script_ConditionalResponseHandler(dc->localClientNum, item, args, Script_ExecIfFloatsEqual);
}

void __cdecl Script_SetLocalVarBool(UiContext *dc, itemDef_s *item, const char **args)
{
    int v3; // eax
    UILocalVarContext *var; // [esp+0h] [ebp-10Ch]
    char string[260]; // [esp+4h] [ebp-108h] BYREF

    var = Script_ParseLocalVar(dc, args);
    if (String_Parse(args, string, 256))
    {
        if (var)
        {
            v3 = atoi(string);
            UILocalVar_SetBool(var->table, v3 != 0);
        }
    }
}

UILocalVarContext *__cdecl Script_ParseLocalVar(UiContext *dc, const char **args)
{
    char varName[260]; // [esp+0h] [ebp-108h] BYREF

    if (String_Parse(args, varName, 256))
        return UILocalVar_FindOrCreate(&dc->localVars, varName);
    else
        return 0;
}

void __cdecl Script_SetLocalVarInt(UiContext *dc, itemDef_s *item, const char **args)
{
    int v3; // eax
    UILocalVarContext *var; // [esp+0h] [ebp-10Ch]
    char string[260]; // [esp+4h] [ebp-108h] BYREF

    var = Script_ParseLocalVar(dc, args);
    if (String_Parse(args, string, 256))
    {
        if (var)
        {
            v3 = atoi(string);
            UILocalVar_SetInt(var->table, v3);
        }
    }
}

void __cdecl Script_SetLocalVarFloat(UiContext *dc, itemDef_s *item, const char **args)
{
    float f; // [esp+4h] [ebp-110h]
    UILocalVarContext *var; // [esp+8h] [ebp-10Ch]
    char string[260]; // [esp+Ch] [ebp-108h] BYREF

    var = Script_ParseLocalVar(dc, args);
    if (String_Parse(args, string, 256))
    {
        if (var)
        {
            f = atof(string);
            UILocalVar_SetFloat(var->table, f);
        }
    }
}

void __cdecl Script_SetLocalVarString(UiContext *dc, itemDef_s *item, const char **args)
{
    UILocalVarContext *var; // [esp+0h] [ebp-10Ch]
    char string[260]; // [esp+4h] [ebp-108h] BYREF

    var = Script_ParseLocalVar(dc, args);
    if (String_Parse(args, string, 256))
    {
        if (var)
            UILocalVar_SetString(var->table, string);
    }
}

void __cdecl Script_FeederTop(UiContext *dc, itemDef_s *item, const char **args)
{
    listBoxDef_s *listPtr; // [esp+8h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if (listPtr)
    {
        if (listPtr->notselectable)
        {
            listPtr->startPos[dc->localClientNum] = 0;
        }
        else
        {
            item->cursorPos[dc->localClientNum] = 0;
            listPtr->startPos[dc->localClientNum] = 0;
            UI_FeederSelection(dc->localClientNum, item->special, 0);
        }
    }
}

void __cdecl Script_FeederBottom(UiContext *dc, itemDef_s *item, const char **args)
{
    int v2; // [esp+4h] [ebp-14h]
    int v3; // [esp+8h] [ebp-10h]
    int max; // [esp+Ch] [ebp-Ch]
    listBoxDef_s *listPtr; // [esp+10h] [ebp-8h]

    listPtr = Item_GetListBoxDef(item);
    if (listPtr)
    {
        if (listPtr->notselectable)
        {
            listPtr->startPos[dc->localClientNum] = Item_ListBox_MaxScroll(dc->localClientNum, item);
        }
        else
        {
            max = Item_ListBox_Viewmax(item);
            v3 = UI_FeederCount(dc->localClientNum, item->special) - 1;
            if (v3 > 0)
                v2 = v3;
            else
                v2 = 0;
            Item_ListBox_SetCursorPos(dc->localClientNum, item, max, v2);
        }
    }
}

void __cdecl Script_Play(UiContext *dc, itemDef_s *item, const char **args)
{
    char val[1028]; // [esp+0h] [ebp-408h] BYREF

    if (String_Parse(args, val, 1024))
        UI_PlayLocalSoundAliasByName(dc->localClientNum, val);
}

void __cdecl Script_ScriptMenuResponse(UiContext *dc, itemDef_s *item, const char **args)
{
    int Int; // eax
    const char *v4; // eax
    int iIndex; // [esp+0h] [ebp-410h]
    const char *pszName; // [esp+4h] [ebp-40Ch]
    char val[1028]; // [esp+8h] [ebp-408h] BYREF

    if (UI_AllowScriptMenuResponse(dc->localClientNum) && String_Parse(args, val, 1024))
    {
        for (iIndex = 0; iIndex < 32; ++iIndex)
        {
            pszName = CL_GetConfigString(dc->localClientNum, iIndex + 1970);
            if (*pszName)
            {
                if (!I_stricmp(item->parent->window.name, pszName))
                    break;
            }
        }
        if (iIndex == 32)
            iIndex = -1;
        Int = Dvar_GetInt("sv_serverId");
        v4 = va("cmd mr %i %i %s\n", Int, iIndex, val);
        Cbuf_AddText(dc->localClientNum, v4);
    }
}

void __cdecl Item_RunScript(UiContext *dc, itemDef_s *item, char *s)
{
    int v3; // [esp+0h] [ebp-1814h]
    unsigned __int8 dst[5120]; // [esp+4h] [ebp-1810h] BYREF
    char out[1028]; // [esp+1404h] [ebp-410h] BYREF
    unsigned int i; // [esp+180Ch] [ebp-8h]
    const char *p; // [esp+1810h] [ebp-4h] BYREF

    memset(dst, 0, sizeof(dst));
    if (item && s && *s)
    {
        I_strncat((char *)dst, 5120, s);
        p = (char *)dst;
        while (String_Parse((const char **)&p, out, 1024))
        {
            if (out[0] != 59 || out[1])
            {
                v3 = 0;
                for (i = 0; i < 0x2A; ++i)
                {
                    if (!I_stricmp(out, commandList[i].name))
                    {
                        commandList[i].handler(dc, item, (const char **)&p);
                        v3 = 1;
                        break;
                    }
                }
                if (!v3)
                    UI_RunMenuScript(dc->localClientNum, &p, s);
            }
        }
    }
}

int __cdecl Item_SetFocus(UiContext *dc, itemDef_s *item, float x, float y)
{
    rectDef_s r; // [esp+28h] [ebp-2Ch] BYREF
    const rectDef_s *textRect; // [esp+40h] [ebp-14h]
    itemDef_s *oldFocus; // [esp+44h] [ebp-10h]
    menuDef_t *focusedMenu; // [esp+48h] [ebp-Ch]
    menuDef_t *parent; // [esp+4Ch] [ebp-8h]
    int i; // [esp+50h] [ebp-4h]

    if (!item)
    {
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2183, 0, "%s", "item != NULL");
        return 0;
    }
    if ((item->window.staticFlags & 0x100000) != 0 || !Window_IsVisible(dc->localClientNum, &item->window))
        return 0;
    if (Window_HasFocus(dc->localClientNum, &item->window) && Item_IsVisible(dc->localClientNum, item))
        return 1;
    parent = item->parent;
    if (parent)
    {
        if (!Window_HasFocus(dc->localClientNum, &parent->window))
        {
            focusedMenu = Menu_GetFocused(dc);
            if (focusedMenu)
            {
                if (Rect_ContainsPoint(dc->localClientNum, &focusedMenu->window.rect, x, y)
                    && Rect_ContainsPoint(dc->localClientNum, &parent->window.rect, x, y))
                {
                    return 0;
                }
            }
        }
    }
    if ((item->dvarFlags & 3) != 0 && !Item_EnableShowViaDvar(item, 1))
        return 0;
    if (!Item_IsVisible(dc->localClientNum, item))
        return 0;
    oldFocus = Menu_ClearFocus(dc, item->parent);
    if (item->type)
    {
        Window_AddDynamicFlags(dc->localClientNum, &item->window, 2);
        if (item->onFocus)
            Item_RunScript(dc, item, (char*)item->onFocus);
    }
    else
    {
        textRect = Item_GetTextRect(dc->localClientNum, item);
        r = *textRect;
        r.y = r.y - r.h;
        r.horzAlign = textRect->horzAlign;
        r.vertAlign = textRect->vertAlign;
        if (Rect_ContainsPoint(dc->localClientNum, &r, x, y))
        {
            Window_AddDynamicFlags(dc->localClientNum, &item->window, 2);
        }
        else if (oldFocus)
        {
            Window_AddDynamicFlags(dc->localClientNum, &oldFocus->window, 2);
            if (oldFocus->onFocus)
                Item_RunScript(dc, oldFocus, (char*)oldFocus->onFocus);
        }
    }
    for (i = 0; i < parent->itemCount; ++i)
    {
        if (parent->items[i] == item && !Item_IsTextField(item))
        {
            Menu_SetCursorItem(dc->localClientNum, parent, i);
            return 1;
        }
    }
    return 1;
}

const rectDef_s *__cdecl Item_GetTextRect(int localClientNum, const itemDef_s *item)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            43,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 44, 0, "%s", "item");
    return &item->textRect[localClientNum];
}

itemDef_s *__cdecl Menu_ClearFocus(UiContext *dc, menuDef_t *menu)
{
    itemDef_s *ret; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if (!menu)
        return 0;
    ret = 0;
    for (i = 0; i < menu->itemCount; ++i)
    {
        if (Window_HasFocus(dc->localClientNum, &menu->items[i]->window))
        {
            if (ret)
                MyAssertHandler(".\\ui\\ui_shared.cpp", 471, 0, "%s", "ret == NULL");
            ret = menu->items[i];
            Window_RemoveDynamicFlags(dc->localClientNum, &ret->window, 2);
            if (menu->items[i]->leaveFocus)
                Item_RunScript(dc, menu->items[i], (char*)menu->items[i]->leaveFocus);
        }
    }
    return ret;
}

bool __cdecl Rect_ContainsPoint(int localClientNum, const rectDef_s *rect, float x, float y)
{
    float compareY; // [esp+8h] [ebp-24h]
    rectDef_s compareRect; // [esp+Ch] [ebp-20h] BYREF
    const ScreenPlacement *scrPlace; // [esp+24h] [ebp-8h]
    float compareX; // [esp+28h] [ebp-4h]

    if (!rect)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 492, 0, "%s", "rect");
    compareRect.x = rect->x;
    compareRect.y = rect->y;
    compareRect.w = rect->w;
    compareRect.h = rect->h;
    scrPlace = &scrPlaceView[localClientNum];
    compareX = ScrPlace_ApplyX(scrPlace, x, 4);
    compareY = ScrPlace_ApplyY(scrPlace, y, 4);
    ScrPlace_ApplyRect(
        scrPlace,
        &compareRect.x,
        &compareRect.y,
        &compareRect.w,
        &compareRect.h,
        rect->horzAlign,
        rect->vertAlign);
    return compareRect.x <= (double)compareX
        && compareX <= compareRect.x + compareRect.w
        && compareRect.y <= (double)compareY
        && compareY <= compareRect.y + compareRect.h;
}

int __cdecl Item_ListBox_MaxScroll(int localClientNum, itemDef_s *item)
{
    int v2; // esi
    int v5; // [esp+Ch] [ebp-4h]

    v2 = Item_ListBox_Viewmax(item);
    v5 = UI_FeederCount(localClientNum, item->special) - v2 + 1;
    if (v5 > 0)
        return v5;
    else
        return 0;
}

int __cdecl Item_ListBox_Viewmax(itemDef_s *item)
{
    float v2; // [esp+8h] [ebp-2Ch]
    float v3; // [esp+Ch] [ebp-28h]
    float v4; // [esp+10h] [ebp-24h]
    float v5; // [esp+14h] [ebp-20h]
    float v6; // [esp+18h] [ebp-1Ch]
    float v7; // [esp+1Ch] [ebp-18h]
    listBoxDef_s *listPtr; // [esp+24h] [ebp-10h]
    float unitSize; // [esp+28h] [ebp-Ch]
    float totalSize; // [esp+30h] [ebp-4h]

    if (!item)
    {
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2307, 0, "%s", "item");
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    }
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2311, 0, "%s", "listPtr");
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 53, 0, "%s", "w");
    if ((item->window.staticFlags & 0x200000) != 0)
    {
        v7 = item->window.rect.w - 2.0;
        v5 = 0.0 - v7;
        if (v5 < 0.0)
            v4 = item->window.rect.w - 2.0;
        else
            v4 = 0.0;
        totalSize = v4;
        unitSize = listPtr->elementWidth;
    }
    else
    {
        v6 = item->window.rect.h - 2.0;
        v3 = 0.0 - v6;
        if (v3 < 0.0)
            v2 = item->window.rect.h - 2.0;
        else
            v2 = 0.0;
        totalSize = v2;
        unitSize = listPtr->elementHeight;
    }
    if (totalSize < 0.0)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2324, 0, "%s\n\t(totalSize) = %g", "(totalSize >= 0)", totalSize);
    if (unitSize < 0.0)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2325, 0, "%s\n\t(unitSize) = %g", "(unitSize >= 0)", unitSize);
    return SnapFloatToInt(totalSize / unitSize);
}

void __cdecl Item_ListBox_SetCursorPos(int localClientNum, itemDef_s *item, int viewmax, int newCursorPos)
{
    listBoxDef_s *listPtr; // [esp+8h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    item->cursorPos[localClientNum] = newCursorPos;
    if (listPtr->startPos[localClientNum] > newCursorPos)
        listPtr->startPos[localClientNum] = newCursorPos;
    if (listPtr->startPos[localClientNum] <= newCursorPos - viewmax)
        listPtr->startPos[localClientNum] = newCursorPos - viewmax + 1;
    UI_FeederSelection(localClientNum, item->special, newCursorPos);
}

bool __cdecl Item_IsTextField(const itemDef_s *item)
{
    bool result; // al

    switch (item->type)
    {
    case 4:
    case 9:
    case 0x10:
    case 0x11:
    case 0x12:
        result = 1;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

int __cdecl Menu_OverActiveItem(int localClientNum, menuDef_t *menu, float x, float y)
{
    const rectDef_s *v4; // eax
    itemDef_s *v6; // [esp+8h] [ebp-1Ch]
    itemDef_s *v7; // [esp+10h] [ebp-14h]
    itemDef_s *overItem; // [esp+18h] [ebp-Ch]
    int i; // [esp+1Ch] [ebp-8h]

    if (menu)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                23,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        if ((menu->window.dynamicFlags[localClientNum] & 0x4004) != 0
            && Rect_ContainsPoint(localClientNum, &menu->window.rect, x, y))
        {
            for (i = 0; i < menu->itemCount; ++i)
            {
                v7 = menu->items[i];
                if (localClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                        23,
                        0,
                        "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1);
                if ((v7->window.dynamicFlags[localClientNum] & 0x4004) != 0
                    && (menu->items[i]->window.staticFlags & 0x100000) == 0)
                {
                    v6 = menu->items[i];
                    if (!v6)
                        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
                    if (Rect_ContainsPoint(localClientNum, &v6->window.rect, x, y))
                    {
                        overItem = menu->items[i];
                        if (overItem->type || !overItem->text)
                            return 1;
                        v4 = Item_CorrectedTextRect(localClientNum, overItem);
                        if (Rect_ContainsPoint(localClientNum, v4, x, y))
                            return 1;
                    }
                }
            }
        }
    }
    else
    {
        MyAssertHandler(".\\ui\\ui_shared.cpp", 6219, 0, "%s", "menu");
    }
    return 0;
}

int __cdecl Display_VisibleMenuCount(UiContext *dc)
{
    menuDef_t *v2; // [esp+4h] [ebp-10h]
    int localClientNum; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int count; // [esp+10h] [ebp-4h]

    count = 0;
    for (i = 0; i < dc->menuCount; ++i)
    {
        v2 = dc->Menus[i];
        localClientNum = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                23,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        if ((v2->window.dynamicFlags[localClientNum] & 0x4004) != 0)
            ++count;
    }
    return count;
}

void __cdecl Menus_HandleOOBClick(UiContext *dc, menuDef_t *menu, int key, int down)
{
    int j; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if (menu)
    {
        if (down && (menu->window.staticFlags & 0x2000000) != 0)
        {
            Menu_RunCloseScript(dc, menu);
            Window_RemoveDynamicFlags(dc->localClientNum, &menu->window, 6);
        }
        for (i = dc->openMenuCount - 1; i >= 0; --i)
        {
            if (Menu_OverActiveItem(dc->localClientNum, dc->menuStack[i], dc->cursor.x, dc->cursor.y))
            {
                for (j = dc->openMenuCount - 1; j >= 0; --j)
                    Window_RemoveDynamicFlags(dc->localClientNum, &dc->menuStack[j]->window, 2);
                Window_AddDynamicFlags(dc->localClientNum, &dc->menuStack[i]->window, 6);
                Display_MouseMove(dc);
                Menu_HandleMouseMove(dc, dc->menuStack[i]);
                Menu_HandleKey(dc, dc->menuStack[i], key, down);
                break;
            }
        }
        if (!Display_VisibleMenuCount(dc))
            UI_Pause(dc->localClientNum, 0);
    }
    else
    {
        MyAssertHandler(".\\ui\\ui_shared.cpp", 4093, 0, "%s", "menu");
    }
}

void __cdecl Item_TextField_BeginEdit(int localClientNum, itemDef_s *item)
{
    unsigned int v2; // [esp+0h] [ebp-20h]
    editFieldDef_s *editPtr; // [esp+18h] [ebp-8h]
    int i; // [esp+1Ch] [ebp-4h]

    if (!item)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3290, 0, "%s", "item");
    editPtr = Item_GetEditFieldDef(item);
    if (editPtr)
        editPtr->paintOffset = 0;
    if (item->dvar)
        v2 = strlen(Dvar_GetVariantString(item->dvar));
    else
        v2 = 0;
    item->cursorPos[localClientNum] = v2;
    g_editingField = 1;
    g_editItem = item;
    Key_SetOverstrikeMode(localClientNum, 1);
    for (i = 0; i < item->parent->itemCount; ++i)
    {
        if (item->parent->items[i] == item)
        {
            Menu_SetCursorItem(localClientNum, item->parent, i);
            return;
        }
    }
}

void __cdecl Menus_Open(UiContext *dc, menuDef_t *menu)
{
    menuDef_t *v2; // [esp+4h] [ebp-188h]
    int localClientNum; // [esp+8h] [ebp-184h]
    itemDef_s item; // [esp+Ch] [ebp-180h] BYREF
    int i; // [esp+188h] [ebp-4h]

    for (i = dc->openMenuCount - 1; i >= 0; --i)
        Menu_LoseFocusDueToOpen(dc, dc->menuStack[i]);
    for (i = 0; i < dc->menuCount; ++i)
    {
        v2 = dc->Menus[i];
        localClientNum = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                23,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                localClientNum,
                1);
        if ((v2->window.dynamicFlags[localClientNum] & 2) != 0)
            MyAssertHandler(
                ".\\ui\\ui_shared.cpp",
                4007,
                0,
                "%s",
                "!(Window_GetDynamicFlags( dc->localClientNum, &dc->Menus[i]->window ) & WINDOWDYNAMIC_HASFOCUS)");
    }
    Menus_AddToStack(dc, menu);
    Window_AddDynamicFlags(dc->localClientNum, &menu->window, 6);
    Menu_CallOnFocusDueToOpen(dc, menu);
    if (dc->isCursorVisible)
        Menu_HandleMouseMove(dc, menu);
    if (menu->onOpen)
    {
        item.parent = menu;
        Item_RunScript(dc, &item, (char*)menu->onOpen);
    }
    if (menu->soundName)
        UI_PlayLocalSoundAliasByName(dc->localClientNum, menu->soundName);
}

void __cdecl Menus_AddToStack(UiContext *dc, menuDef_t *pMenu)
{
    Menus_RemoveFromStack(dc, pMenu);
    if (dc->openMenuCount == 16)
        Com_Error(ERR_DROP, "Too many menus opened");
    dc->menuStack[dc->openMenuCount++] = pMenu;
}

void __cdecl Menu_LoseFocusDueToOpen(UiContext *dc, menuDef_t *menu)
{
    bool anyFound; // [esp+1Bh] [ebp-5h]
    int i; // [esp+1Ch] [ebp-4h]

    if (!dc)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 849, 0, "%s", "dc");
    if (!menu)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 850, 0, "%s", "menu");
    if (Window_HasFocus(dc->localClientNum, &menu->window))
    {
        Window_RemoveDynamicFlags(dc->localClientNum, &menu->window, 2);
        anyFound = 0;
        for (i = 0; i < menu->itemCount; ++i)
        {
            if (Window_HasFocus(dc->localClientNum, &menu->items[i]->window))
            {
                if (menu->items[i]->leaveFocus)
                    Item_RunScript(dc, menu->items[i], (char*)menu->items[i]->leaveFocus);
                if (anyFound)
                    MyAssertHandler(".\\ui\\ui_shared.cpp", 868, 0, "%s", "!anyFound");
                anyFound = 1;
            }
        }
    }
}

int __cdecl Menus_OpenByName(UiContext *dc, const char *p)
{
    menuDef_t *pMenu; // [esp+0h] [ebp-4h]

    pMenu = Menus_FindByName(dc, p);
    if (pMenu)
    {
        Menus_Open(dc, pMenu);
        return 1;
    }
    else
    {
        Com_PrintWarning(13, "Could not find menu '%s'\n", p);
        return 0;
    }
}

void __cdecl Menus_PrintAllLoadedMenus(UiContext *dc)
{
    int i; // [esp+0h] [ebp-4h]

    Com_Printf(16, "Currently loaded UI menus (CG menus not included):\n");
    for (i = 0; i < dc->menuCount; ++i)
        Com_Printf(16, "%i. %s\n", i, dc->Menus[i]->window.name);
    Com_Printf(16, "\n%i menus total\n", dc->menuCount);
}

int __cdecl Display_MouseMove(UiContext *dc)
{
    menuDef_t *menu; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    menu = Menu_GetFocused(dc);
    if (menu && (menu->window.staticFlags & 0x1000000) != 0)
    {
        Menu_HandleMouseMove(dc, menu);
        return 1;
    }
    else
    {
        for (i = dc->openMenuCount - 1;
            i >= 0 && !Menu_HandleMouseMove(dc, dc->menuStack[i]) && !dc->menuStack[i]->fullScreen;
            --i)
        {
            ;
        }
        return 1;
    }
}

itemDef_s *g_bindItem;
int inHandleKey;
void __cdecl Menu_HandleKey(UiContext *dc, menuDef_t *menu, int key, int down)
{
    int v4; // eax
    const rectDef_s *v5; // eax
    float x; // [esp+0h] [ebp-1A4h]
    float y; // [esp+4h] [ebp-1A0h]
    itemDef_s it; // [esp+1Ch] [ebp-188h] BYREF
    itemDef_s *item; // [esp+194h] [ebp-10h]
    int inHandler; // [esp+198h] [ebp-Ch]
    int i; // [esp+19Ch] [ebp-8h]
    const char *binding; // [esp+1A0h] [ebp-4h]

    item = 0;
    inHandler = 1;
    if (g_waitingForKey && down)
    {
        Item_Bind_HandleKey(dc, g_bindItem, key, down);
        inHandler = 0;
        return;
    }
    if (!g_editingField || !down)
        goto LABEL_13;
    if (!Item_TextField_HandleKey(dc, g_editItem, key))
    {
        g_editingField = 0;
        g_editItem = 0;
        inHandler = 0;
        return;
    }
    if (key == 200 || key == 201 || key == 202)
    {
        g_editingField = 0;
        g_editItem = 0;
        Display_MouseMove(dc);
    LABEL_13:
        if (menu)
        {
            if (down
                && (key & 0x400) == 0
                && menu->allowedBinding
                && (binding = Key_GetBinding(dc->localClientNum, key)) != 0
                && !I_stricmp(binding, menu->allowedBinding))
            {
                v4 = CL_ControllerIndexFromClientNum(dc->localClientNum);
                Cbuf_ExecuteBuffer(dc->localClientNum, v4, (char *)binding);
            }
            else if (!down
                || (menu->window.staticFlags & 0x1000000) != 0
                || menu->fullScreen
                || Rect_ContainsPoint(dc->localClientNum, &menu->window.rect, dc->cursor.x, dc->cursor.y)
                || inHandleKey
                || key != 200 && key != 201 && key != 202)
            {
                for (i = 0; i < menu->itemCount; ++i)
                {
                    if (Item_IsVisible(dc->localClientNum, menu->items[i]))
                    {
                        if (Window_HasFocus(dc->localClientNum, &menu->items[i]->window))
                            item = menu->items[i];
                    }
                }
                if (key != 205 && key != 206 || item && item->type == 6)
                {
                    if (item && Item_HandleKey(dc, item, key, down))
                    {
                        Item_Action(dc, item);
                        inHandler = 0;
                    }
                    else if (down)
                    {
                        if (key <= 0 || key > 255 || !Menu_CheckOnKey(dc, menu, key))
                        {
                            switch (key)
                            {
                            case 9:
                            case 155:
                            case 157:
                            case 189:
                            case 205:
                                Menu_SetNextCursorItem(dc, menu);
                                break;
                            case 13:
                            case 191:
                            case 202:
                                if (item)
                                {
                                    if (Item_IsTextField(item))
                                    {
                                        item->cursorPos[dc->localClientNum] = 0;
                                        g_editingField = 1;
                                        g_editItem = item;
                                        Key_SetOverstrikeMode(dc->localClientNum, 1);
                                    }
                                    else
                                    {
                                        Item_Action(dc, item);
                                    }
                                }
                                break;
                            case 27:
                                if (!g_waitingForKey && menu->onESC)
                                {
                                    it.parent = menu;
                                    Item_RunScript(dc, &it, (char*)menu->onESC);
                                }
                                break;
                            case 154:
                            case 156:
                            case 183:
                            case 206:
                                Menu_SetPrevCursorItem(dc, menu);
                                break;
                            case 177:
                                if (Dvar_GetInt("developer"))
                                    g_debugMode ^= 1u;
                                break;
                            case 178:
                                if (Dvar_GetInt("developer"))
                                    Cbuf_AddText(dc->localClientNum, "screenshot\n");
                                break;
                            case 200:
                            case 201:
                                if (item)
                                {
                                    if (item->type)
                                    {
                                        if (Rect_ContainsPoint(dc->localClientNum, &item->window.rect, dc->cursor.x, dc->cursor.y))
                                        {
                                            if (Item_IsTextField(item))
                                                Item_TextField_BeginEdit(dc->localClientNum, item);
                                            else
                                                Item_Action(dc, item);
                                        }
                                    }
                                    else
                                    {
                                        y = dc->cursor.y;
                                        x = dc->cursor.x;
                                        v5 = Item_CorrectedTextRect(dc->localClientNum, item);
                                        if (Rect_ContainsPoint(dc->localClientNum, v5, x, y))
                                            Item_Action(dc, item);
                                    }
                                }
                                break;
                            default:
                                return;
                            }
                        }
                    }
                    else
                    {
                        inHandler = 0;
                    }
                }
            }
            else
            {
                inHandleKey = 1;
                Menus_HandleOOBClick(dc, menu, key, down);
                inHandleKey = 0;
                inHandler = 0;
            }
        }
        else
        {
            inHandler = 0;
        }
    }
}

bool __cdecl Item_TextField_HandleKey(UiContext *dc, itemDef_s *item, int key)
{
    char *VariantString; // eax
    char *v5; // eax
    char *v6; // eax
    int OverstrikeMode; // eax
    bool v8; // [esp+10h] [ebp-430h]
    editFieldDef_s *editPtr; // [esp+24h] [ebp-41Ch]
    int len; // [esp+28h] [ebp-418h]
    itemDef_s *newItem; // [esp+2Ch] [ebp-414h]
    itemDef_s *newItema; // [esp+2Ch] [ebp-414h]
    itemDef_s *newItemb; // [esp+2Ch] [ebp-414h]
    itemDef_s *newItemc; // [esp+2Ch] [ebp-414h]
    int cursorPos; // [esp+30h] [ebp-410h]
    int cursorPosa; // [esp+30h] [ebp-410h]
    int cursorPosb; // [esp+30h] [ebp-410h]
    bool validInput; // [esp+37h] [ebp-409h] BYREF
    char buff[1024]; // [esp+38h] [ebp-408h] BYREF
    int memMoveCount; // [esp+43Ch] [ebp-4h]

    editPtr = Item_GetEditFieldDef(item);
    if (!editPtr)
        return 0;
    if (!item->dvar)
        return 0;
    memset((unsigned __int8 *)buff, 0, sizeof(buff));
    VariantString = (char *)Dvar_GetVariantString(item->dvar);
    I_strncpyz(buff, VariantString, 1024);
    len = &buff[strlen(buff) + 1] - &buff[1];
    if (editPtr->maxChars)
    {
        if (len > editPtr->maxChars)
        {
            len = editPtr->maxChars;
            buff[len] = 0;
            if (item->cursorPos[dc->localClientNum] > editPtr->maxChars)
                item->cursorPos[dc->localClientNum] = editPtr->maxChars;
        }
    }
    if ((key & 0x400) != 0)
    {
        key &= ~0x400u;
        if (key == 8)
        {
            if (item->cursorPos[dc->localClientNum] > 0)
            {
                cursorPos = item->cursorPos[dc->localClientNum];
                memMoveCount = len + 1 - cursorPos;
                if (memMoveCount <= 0 || memMoveCount > len)
                    MyAssertHandler(
                        ".\\ui\\ui_shared.cpp",
                        3375,
                        0,
                        "%s\n\t(memMoveCount) = %i",
                        "(memMoveCount > 0 && memMoveCount <= len)",
                        memMoveCount);
                memmove((unsigned __int8 *)&buff[cursorPos - 1], (unsigned __int8 *)&buff[cursorPos], memMoveCount);
            }
            Dvar_SetFromStringByName(item->dvar, buff);
            v5 = (char *)Dvar_GetVariantString(item->dvar);
            I_strncpyz(buff, v5, 1024);
            item->cursorPos[dc->localClientNum] = Item_GetCursorPosOffset(dc->localClientNum, item, buff, -1);
            Item_TextField_EnsureCursorVisible(dc->localClientNum, item, buff);
            return 1;
        }
        if (item->type == 16 && !I_isforfilename(key))
            return 1;
        if (key < 32 || !item->dvar)
            return 1;
        if (key == 64)
            return 1;
        if (item->type == 9 && !I_isdigit(key))
            return 0;
        if (item->type == 17)
        {
            v8 = I_isdigit(key) || key == Com_GetDecimalDelimiter();
            validInput = v8;
            if (!v8)
                return 0;
            if (key == Com_GetDecimalDelimiter())
                key = 46;
        }
        if (item->type == 18)
            key = toupper(key);
        if (!Key_GetOverstrikeMode(dc->localClientNum))
        {
            if (len == 255 || editPtr->maxChars && len >= editPtr->maxChars)
                return 1;
            cursorPosa = item->cursorPos[dc->localClientNum];
            memMoveCount = len + 1 - cursorPosa;
            if (memMoveCount <= 0 || memMoveCount > len + 1)
                MyAssertHandler(
                    ".\\ui\\ui_shared.cpp",
                    3439,
                    0,
                    "%s\n\t(memMoveCount) = %i",
                    "(memMoveCount > 0 && memMoveCount <= (len + 1))",
                    memMoveCount);
            memmove((unsigned __int8 *)&buff[cursorPosa + 1], (unsigned __int8 *)&buff[cursorPosa], memMoveCount);
            goto LABEL_54;
        }
        if (!editPtr->maxChars || item->cursorPos[dc->localClientNum] < editPtr->maxChars)
        {
        LABEL_54:
            buff[item->cursorPos[dc->localClientNum]] = key;
            Dvar_SetFromStringByName(item->dvar, buff);
            v6 = (char *)Dvar_GetVariantString(item->dvar);
            I_strncpyz(buff, v6, 1024);
            strlen(buff);
            item->cursorPos[dc->localClientNum] = Item_GetCursorPosOffset(dc->localClientNum, item, buff, 1);
            Item_TextField_EnsureCursorVisible(dc->localClientNum, item, buff);
            if (editPtr->maxChars)
            {
                if (item->cursorPos[dc->localClientNum] >= editPtr->maxChars)
                {
                    if (editPtr->maxCharsGotoNext)
                    {
                        newItema = Menu_SetNextCursorItem(dc, item->parent);
                        newItema->cursorPos[dc->localClientNum] = 0;
                        if (newItema)
                        {
                            if (Item_IsTextField(newItema))
                                g_editItem = newItema;
                        }
                    }
                }
            }
        LABEL_84:
            if (key == 9 || key == 155 || key == 189)
            {
                newItemb = Menu_SetNextCursorItem(dc, item->parent);
                if (newItemb)
                {
                    if (Item_IsTextField(newItemb))
                        g_editItem = newItemb;
                }
            }
            if (key == 154 || key == 183)
            {
                newItemc = Menu_SetPrevCursorItem(dc, item->parent);
                if (newItemc)
                {
                    if (Item_IsTextField(newItemc))
                        g_editItem = newItemc;
                }
            }
            if ((key == 13 || key == 191) && item->onAccept)
                Item_RunScript(dc, item, (char*)item->onAccept);
            return key != 13 && key != 191 && key != 27;
        }
        if (editPtr->maxCharsGotoNext)
        {
            newItem = Menu_SetNextCursorItem(dc, item->parent);
            if (newItem)
            {
                if (Item_IsTextField(newItem))
                    g_editItem = newItem;
            }
        }
        return 1;
    }
    else
    {
        if (key != 162 && key != 193)
        {
            switch (key)
            {
            case 157:
            case 187:
                item->cursorPos[dc->localClientNum] = Item_GetCursorPosOffset(dc->localClientNum, item, buff, 1);
                Item_TextField_EnsureCursorVisible(dc->localClientNum, item, buff);
                return 1;
            case 156:
            case 185:
                item->cursorPos[dc->localClientNum] = Item_GetCursorPosOffset(dc->localClientNum, item, buff, -1);
                Item_TextField_EnsureCursorVisible(dc->localClientNum, item, buff);
                return 1;
            case 165:
            case 182:
                item->cursorPos[dc->localClientNum] = 0;
                editPtr->paintOffset = 0;
                return 1;
            case 166:
            case 188:
                item->cursorPos[dc->localClientNum] = len;
                Item_TextField_EnsureCursorVisible(dc->localClientNum, item, buff);
                return 1;
            case 161:
            case 192:
                OverstrikeMode = Key_GetOverstrikeMode(dc->localClientNum);
                Key_SetOverstrikeMode(dc->localClientNum, OverstrikeMode == 0);
                return 1;
            }
            goto LABEL_84;
        }
        if (item->cursorPos[dc->localClientNum] < len)
        {
            cursorPosb = item->cursorPos[dc->localClientNum];
            memMoveCount = len - cursorPosb;
            if (len - cursorPosb <= 0 || memMoveCount > len)
                MyAssertHandler(
                    ".\\ui\\ui_shared.cpp",
                    3490,
                    0,
                    "%s\n\t(memMoveCount) = %i",
                    "(memMoveCount > 0 && memMoveCount <= len)",
                    memMoveCount);
            memmove((unsigned __int8 *)&buff[cursorPosb], (unsigned __int8 *)&buff[cursorPosb + 1], memMoveCount);
            Dvar_SetFromStringByName(item->dvar, buff);
        }
        return 1;
    }
}

void __cdecl Item_TextField_EnsureCursorVisible(int localClientNum, itemDef_s *item, const char *text)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-Ch]
    int paintOffsetMin; // [esp+4h] [ebp-8h]
    int cursorPos; // [esp+8h] [ebp-4h]

    editPtr = Item_GetEditFieldDef(item);
    cursorPos = item->cursorPos[localClientNum];
    if (editPtr->paintOffset <= cursorPos)
    {
        if (editPtr->maxPaintChars)
        {
            paintOffsetMin = Item_GetCursorPosOffset(localClientNum, item, text, -editPtr->maxPaintChars);
            if (editPtr->paintOffset < paintOffsetMin)
                editPtr->paintOffset = paintOffsetMin;
        }
    }
    else
    {
        editPtr->paintOffset = cursorPos;
    }
}

void __cdecl Scroll_ListBox_AutoFunc(UiContext *dc, void *p)
{
    if (dc->realTime > *(_DWORD *)p)
    {
        Item_ListBox_HandleKey(dc, *((itemDef_s **)p + 6), *((_DWORD *)p + 3), 1, 0);
        *(_DWORD *)p = *((_DWORD *)p + 2) + dc->realTime;
    }
    if (dc->realTime > *((_DWORD *)p + 1))
    {
        *((_DWORD *)p + 1) = dc->realTime + 150;
        if (*((int *)p + 2) > 20)
            *((_DWORD *)p + 2) -= 40;
    }
}

void __cdecl Scroll_ListBox_ThumbFunc(UiContext *dc, void *p)
{
    int v2; // [esp+0h] [ebp-3Ch]
    int v3; // [esp+4h] [ebp-38h]
    int v4; // [esp+8h] [ebp-34h]
    int pos; // [esp+10h] [ebp-2Ch]
    int posa; // [esp+10h] [ebp-2Ch]
    int max; // [esp+14h] [ebp-28h]
    int maxa; // [esp+14h] [ebp-28h]
    float r; // [esp+18h] [ebp-24h]
    float r_4; // [esp+1Ch] [ebp-20h]
    float r_8; // [esp+20h] [ebp-1Ch]
    float r_12; // [esp+24h] [ebp-18h]
    listBoxDef_s *listPtr; // [esp+34h] [ebp-8h]

    if (dc->isCursorVisible)
    {
        listPtr = Item_GetListBoxDef(*((itemDef_s **)p + 6));
        if (listPtr)
        {
            v4 = *((_DWORD *)p + 6);
            if (!v4)
                MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 53, 0, "%s", "w");
            if ((*(_DWORD *)(v4 + 76) & 0x200000) != 0)
            {
                if (*((float *)p + 4) == dc->cursor.x)
                    return;
                v3 = *((_DWORD *)p + 6);
                if (!v3)
                    MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
                r = *(float *)(v3 + 4) + 16.0 + 1.0;
                r_8 = *(float *)(v3 + 12) - 32.0 - 2.0;
                max = Item_ListBox_MaxScroll(dc->localClientNum, *((itemDef_s **)p + 6));
                pos = SnapFloatToInt((dc->cursor.x - r - 8.0) * (double)max / (r_8 - 16.0));
                if (pos >= 0)
                {
                    if (pos > max)
                        pos = max;
                }
                else
                {
                    pos = 0;
                }
                listPtr->startPos[dc->localClientNum] = pos;
                *((float *)p + 4) = dc->cursor.x;
            }
            else if (*((float *)p + 5) != dc->cursor.y)
            {
                v2 = *((_DWORD *)p + 6);
                if (!v2)
                    MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
                r_4 = *(float *)(v2 + 8) + 16.0 + 1.0;
                r_12 = *(float *)(v2 + 16) - 32.0 - 2.0;
                maxa = Item_ListBox_MaxScroll(dc->localClientNum, *((itemDef_s **)p + 6));
                posa = SnapFloatToInt((dc->cursor.y - r_4 - 8.0) * (double)maxa / (r_12 - 16.0));
                if (posa >= 0)
                {
                    if (posa > maxa)
                        posa = maxa;
                }
                else
                {
                    posa = 0;
                }
                listPtr->startPos[dc->localClientNum] = posa;
                *((float *)p + 5) = dc->cursor.y;
            }
            if (dc->realTime > *(_DWORD *)p)
            {
                Item_ListBox_HandleKey(dc, *((itemDef_s **)p + 6), *((_DWORD *)p + 3), 1, 0);
                *(_DWORD *)p = *((_DWORD *)p + 2) + dc->realTime;
            }
            if (dc->realTime > *((_DWORD *)p + 1))
            {
                *((_DWORD *)p + 1) = dc->realTime + 150;
                if (*((int *)p + 2) > 20)
                    *((_DWORD *)p + 2) -= 40;
            }
        }
    }
}

int __cdecl Item_Slider_OverSlider(int localClientNum, itemDef_s *item, float x, float y)
{
    rectDef_s r; // [esp+8h] [ebp-1Ch] BYREF
    const rectDef_s *rect; // [esp+20h] [ebp-4h]

    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    rect = &item->window.rect;
    r.x = Item_Slider_ThumbPosition(localClientNum, item) - 5.0;
    r.y = item->window.rect.y - 2.0;
    r.w = 10.0;
    r.h = 20.0;
    r.horzAlign = item->window.rect.horzAlign;
    r.vertAlign = item->window.rect.vertAlign;
    if (Rect_ContainsPoint(localClientNum, &r, x, y))
        return 1024;
    else
        return 0;
}

void __cdecl Scroll_Slider_SetThumbPos(UiContext *dc, itemDef_s *item);

void __cdecl Scroll_Slider_ThumbFunc(UiContext *dc, itemDef_s **p)
{
    Scroll_Slider_SetThumbPos(dc, p[6]);
}

struct scrollInfo_s // sizeof=0x20
{                                       // ...
    int nextScrollTime;                 // ...
    int nextAdjustTime;                 // ...
    int adjustValue;                    // ...
    int scrollKey;                      // ...
    float xStart;                       // ...
    float yStart;                       // ...
    itemDef_s *item;                    // ...
    int scrollDir;                      // ...
};
scrollInfo_s scrollInfo;
void __cdecl Item_StartCapture(UiContext *dc, itemDef_s *item, int key)
{
    int type; // [esp+8h] [ebp-8h]
    __int16 flags; // [esp+Ch] [ebp-4h]

    type = item->type;
    if (type == 6)
    {
        flags = Item_ListBox_OverLB(dc->localClientNum, item, dc->cursor.x, dc->cursor.y);
        if ((flags & 0x300) != 0)
        {
            scrollInfo.nextScrollTime = dc->realTime + 500;
            scrollInfo.nextAdjustTime = dc->realTime + 150;
            scrollInfo.adjustValue = 500;
            scrollInfo.scrollKey = key;
            scrollInfo.scrollDir = (flags & 0x100) != 0;
            scrollInfo.item = item;
            captureData = &scrollInfo;
            captureFunc = Scroll_ListBox_AutoFunc;
            itemCapture = item;
        }
        else if ((flags & 0x400) != 0)
        {
            scrollInfo.scrollKey = key;
            scrollInfo.item = item;
            scrollInfo.xStart = dc->cursor.x;
            scrollInfo.yStart = dc->cursor.y;
            captureData = &scrollInfo;
            captureFunc = Scroll_ListBox_ThumbFunc;
            itemCapture = item;
        }
    }
    else if (type == 10 && (Item_Slider_OverSlider(dc->localClientNum, item, dc->cursor.x, dc->cursor.y) & 0x400) != 0)
    {
        scrollInfo.scrollKey = key;
        scrollInfo.item = item;
        scrollInfo.xStart = dc->cursor.x;
        scrollInfo.yStart = dc->cursor.y;
        captureData = &scrollInfo;
        captureFunc = (void(__cdecl *)(UiContext *, void *))Scroll_Slider_ThumbFunc;
        itemCapture = item;
    }
}

int Item_HandleKey(UiContext *dc, itemDef_s *item, int key, int down)
{
    int result; // r3

    if (itemCapture)
    {
        itemCapture = 0;
        captureFunc = 0;
        captureData = 0;
    }
    else
    {
        if (!down)
            return 0;
        if (key == 200 || key == 201 || key == 202)
            Item_StartCapture(dc, item, key);
    }
    if (!down)
        return 0;
    switch (item->type)
    {
    case 6:
        result = Item_ListBox_HandleKey(dc, item, key, down, 0);
        break;
    case 8:
        result = UI_OwnerDrawHandleKey(item->window.ownerDraw, item->window.ownerDrawFlags, &item->special, key);
        break;
    case 0xA:
        result = Item_Slider_HandleKey(dc, item, key, down);
        break;
    case 0xB:
        result = Item_YesNo_HandleKey(dc, item, key);
        break;
    case 0xC:
        result = Item_Multi_HandleKey(dc, item, key);
        break;
    case 0xD:
        result = Item_DvarEnum_HandleKey(dc, item, key);
        break;
    case 0xE:
        result = Item_Bind_HandleKey(dc, item, key, down);
        break;
    default:
        return 0;
    }
    return result;
}

int __cdecl Item_OwnerDraw_HandleKey(itemDef_s *item, int key)
{
    return UI_OwnerDrawHandleKey(item->window.ownerDraw, item->window.ownerDrawFlags, &item->special, key);
}

int lastListBoxClickTime;
int __cdecl Item_ListBox_HandleKey(UiContext *dc, itemDef_s *item, int key, int down, int force)
{
    int result; // eax
    int v6; // [esp+8h] [ebp-84h]
    int v7; // [esp+Ch] [ebp-80h]
    int v8; // [esp+10h] [ebp-7Ch]
    int v9; // [esp+14h] [ebp-78h]
    int v10; // [esp+1Ch] [ebp-70h]
    int v11; // [esp+24h] [ebp-68h]
    int v12; // [esp+2Ch] [ebp-60h]
    bool v13; // [esp+30h] [ebp-5Ch]
    int v15; // [esp+4Ch] [ebp-40h]
    int v16; // [esp+54h] [ebp-38h]
    int localClientNum; // [esp+5Ch] [ebp-30h]
    int v18; // [esp+60h] [ebp-2Ch]
    int max; // [esp+64h] [ebp-28h]
    listBoxDef_s *listPtr; // [esp+70h] [ebp-1Ch]
    int viewmax; // [esp+74h] [ebp-18h]
    int flags; // [esp+78h] [ebp-14h]
    ItemKeyHandler *handler; // [esp+84h] [ebp-8h]

    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    v13 = dc->isCursorVisible && Rect_ContainsPoint(dc->localClientNum, &item->window.rect, dc->cursor.x, dc->cursor.y);
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        return 0;
    v18 = UI_FeederCount(dc->localClientNum, item->special) - 1;
    if (v18 > 0)
        v12 = v18;
    else
        v12 = 0;
    localClientNum = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            23,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    flags = item->window.dynamicFlags[localClientNum];
    if (!force && (!v13 || (flags & 2) == 0))
        return 0;
    UI_OverrideCursorPos(dc->localClientNum, item);
    max = Item_ListBox_MaxScroll(dc->localClientNum, item);
    viewmax = Item_ListBox_Viewmax(item);
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 53, 0, "%s", "w");
    if ((item->window.staticFlags & 0x200000) != 0)
    {
        if (key == 156 || key == 185)
        {
            if (listPtr->notselectable)
            {
                if (listPtr->startPos[dc->localClientNum] - 1 > 0)
                    v11 = listPtr->startPos[dc->localClientNum] - 1;
                else
                    v11 = 0;
                listPtr->startPos[dc->localClientNum] = v11;
            }
            else
            {
                v16 = item->cursorPos[dc->localClientNum] - 1;
                if (v16 > 0)
                    Item_ListBox_SetCursorPos(dc->localClientNum, item, viewmax, v16);
                else
                    Item_ListBox_SetCursorPos(dc->localClientNum, item, viewmax, 0);
            }
            return 1;
        }
        if (key == 157 || key == 187)
        {
            if (listPtr->notselectable)
            {
                if (listPtr->startPos[dc->localClientNum] + 1 < max)
                    v10 = listPtr->startPos[dc->localClientNum] + 1;
                else
                    v10 = max;
                listPtr->startPos[dc->localClientNum] = v10;
            }
            else
            {
                v15 = item->cursorPos[dc->localClientNum] + 1;
                if (v15 < v12)
                    Item_ListBox_SetCursorPos(dc->localClientNum, item, viewmax, v15);
                else
                    Item_ListBox_SetCursorPos(dc->localClientNum, item, viewmax, v12);
            }
            return 1;
        }
    LABEL_48:
        switch (key)
        {
        case 200:
        case 201:
            if ((flags & 0x100) != 0)
            {
                if (listPtr->startPos[dc->localClientNum] - 1 > 0)
                    v9 = listPtr->startPos[dc->localClientNum] - 1;
                else
                    v9 = 0;
                listPtr->startPos[dc->localClientNum] = v9;
            }
            else if ((flags & 0x200) != 0)
            {
                if (listPtr->startPos[dc->localClientNum] + 1 < max)
                    v8 = listPtr->startPos[dc->localClientNum] + 1;
                else
                    v8 = max;
                listPtr->startPos[dc->localClientNum] = v8;
            }
            else if ((flags & 0x800) != 0)
            {
                if (listPtr->startPos[dc->localClientNum] - viewmax > 0)
                    v7 = listPtr->startPos[dc->localClientNum] - viewmax;
                else
                    v7 = 0;
                listPtr->startPos[dc->localClientNum] = v7;
            }
            else if ((flags & 0x1000) != 0)
            {
                if (viewmax + listPtr->startPos[dc->localClientNum] < max)
                    v6 = viewmax + listPtr->startPos[dc->localClientNum];
                else
                    v6 = max;
                listPtr->startPos[dc->localClientNum] = v6;
            }
            else if ((flags & 0x400) == 0)
            {
                if (item->special == 25.0)
                {
                    UI_OverrideCursorPos(dc->localClientNum, item);
                    UI_FeederSelection(dc->localClientNum, item->special, item->cursorPos[dc->localClientNum]);
                }
                if (CL_GetLocalClientActiveCount())
                {
                    if (dc->realTime < lastListBoxClickTime
                        && listPtr->doubleClick
                        && item->cursorPos[dc->localClientNum] == listPtr->mousePos
                        && ListBox_HasValidCursorPos(dc->localClientNum, item))
                    {
                        Item_RunScript(dc, item, (char*)listPtr->doubleClick);
                    }
                    lastListBoxClickTime = dc->realTime + 300;
                    if (item->cursorPos[dc->localClientNum] != listPtr->mousePos)
                    {
                        if (item->cursorPos[dc->localClientNum] < 0
                            || listPtr->mousePos < UI_FeederCount(dc->localClientNum, item->special))
                        {
                            item->cursorPos[dc->localClientNum] = listPtr->mousePos;
                        }
                        UI_FeederSelection(dc->localClientNum, item->special, item->cursorPos[dc->localClientNum]);
                    }
                }
            }
            return 1;
        case 165:
        case 182:
            Script_FeederTop(dc, item);
            return 1;
        case 166:
        case 188:
            Script_FeederBottom(dc, item);
            return 1;
        case 164:
        case 184:
            Item_ListBox_Page(dc->localClientNum, item, v12, max, viewmax, -viewmax);
            return 1;
        case 163:
        case 190:
            Item_ListBox_Page(dc->localClientNum, item, v12, max, viewmax, viewmax);
            return 1;
        }
        for (handler = item->onKey; handler; handler = handler->next)
        {
            if (handler->key == key)
            {
                Item_RunScript(dc, item, (char*)handler->action);
                return 1;
            }
        }
        return 0;
    }
    switch (key)
    {
    case 154:
    case 183:
    case 206:
        Item_ListBox_Scroll(dc->localClientNum, item, v12, max, viewmax, -1);
        result = 1;
        break;
    case 155:
    case 189:
    case 205:
        Item_ListBox_Scroll(dc->localClientNum, item, v12, max, viewmax, 1);
        result = 1;
        break;
    default:
        goto LABEL_48;
    }
    return result;
}

void __cdecl Item_ListBox_Page(int localClientNum, itemDef_s *item, int max, int scrollmax, int viewmax, int delta)
{
    int v6; // [esp+4h] [ebp-18h]
    int v7; // [esp+Ch] [ebp-10h]
    int v8; // [esp+14h] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+18h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2735, 0, "%s", "listPtr");
    if (delta + listPtr->startPos[localClientNum] < scrollmax)
        v8 = delta + listPtr->startPos[localClientNum];
    else
        v8 = scrollmax;
    if (v8 > 0)
        v6 = v8;
    else
        v6 = 0;
    listPtr->startPos[localClientNum] = v6;
    if (!listPtr->notselectable)
    {
        if (delta + item->cursorPos[localClientNum] < max)
            v7 = delta + item->cursorPos[localClientNum];
        else
            v7 = max;
        if (v7 > 0)
            Item_ListBox_SetCursorPos(localClientNum, item, viewmax, v7);
        else
            Item_ListBox_SetCursorPos(localClientNum, item, viewmax, 0);
    }
}

void __cdecl Item_ListBox_Scroll(int localClientNum, itemDef_s *item, int max, int scrollmax, int viewmax, int delta)
{
    int v6; // [esp+0h] [ebp-1Ch]
    int v7; // [esp+Ch] [ebp-10h]
    int v8; // [esp+14h] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+18h] [ebp-4h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 2748, 0, "%s", "listPtr");
    if (listPtr->notselectable)
    {
        if (delta + listPtr->startPos[localClientNum] < scrollmax)
            v7 = delta + listPtr->startPos[localClientNum];
        else
            v7 = scrollmax;
        if (v7 > 0)
            v6 = v7;
        else
            v6 = 0;
        listPtr->startPos[localClientNum] = v6;
    }
    else
    {
        if (delta + item->cursorPos[localClientNum] < max)
            v8 = delta + item->cursorPos[localClientNum];
        else
            v8 = max;
        if (v8 > 0)
            Item_ListBox_SetCursorPos(localClientNum, item, viewmax, v8);
        else
            Item_ListBox_SetCursorPos(localClientNum, item, viewmax, 0);
    }
}

int __cdecl Item_YesNo_HandleKey(UiContext *dc, itemDef_s *item, int key)
{
    char *VariantString; // eax
    int v5; // eax
    char *v6; // eax
    bool v7; // [esp+0h] [ebp-28h]
    char v8; // [esp+7h] [ebp-21h]
    char dvarString[28]; // [esp+8h] [ebp-20h] BYREF

    if (!item->dvar)
        return 0;
    if (!Item_ShouldHandleKey(dc, item, key))
        return 0;
    if (key == 200 || key == 201 || key == 202)
    {
        v8 = 1;
    }
    else
    {
        v7 = key == 13 || key == 156 || key == 157 || key == 164 || key == 163;
        v8 = v7;
    }
    if (!v8)
        return 0;
    VariantString = (char *)Dvar_GetVariantString(item->dvar);
    I_strncpyz(dvarString, VariantString, 25);
    v5 = atoi(dvarString);
    v6 = va("%i", v5 == 0);
    Dvar_SetFromStringByName(item->dvar, v6);
    return 1;
}

BOOL __cdecl Item_ContainsMouse(UiContext *dc, itemDef_s *item)
{
    if (!dc->isCursorVisible)
        return 0;
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    return Rect_ContainsPoint(dc->localClientNum, &item->window.rect, dc->cursor.x, dc->cursor.y);
}

bool __cdecl Item_ShouldHandleKey(UiContext *dc, itemDef_s *item, int key)
{
    if (!Window_HasFocus(dc->localClientNum, &item->window))
        return 0;
    return key != 200 && key != 201 && key != 202 || Item_ContainsMouse(dc, item);
}

int __cdecl Item_Multi_HandleKey(UiContext *dc, itemDef_s *item, int key)
{
    char *v4; // eax
    multiDef_s *multiPtr; // [esp+8h] [ebp-10h]
    int next; // [esp+Ch] [ebp-Ch]
    int count; // [esp+10h] [ebp-8h]
    int current; // [esp+14h] [ebp-4h]

    if (!dc)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3219, 0, "%s", "dc");
    if (!item)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3220, 0, "%s", "item");
    if (!item->dvar)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3221, 0, "%s", "item->dvar");
    multiPtr = Item_GetMultiDef(item);
    if (!multiPtr)
        return 0;
    if (!Item_ShouldHandleKey(dc, item, key))
        return 0;
    current = Item_Multi_FindDvarByValue(item);
    count = Item_Multi_CountSettings(item);
    next = Item_List_NextEntryForKey(key, current, count);
    if (next == current)
        return 0;
    if (multiPtr->strDef)
    {
        Dvar_SetFromStringByName(item->dvar, (char*)multiPtr->dvarStr[next]);
    }
    else
    {
        v4 = va("%g", multiPtr->dvarValue[next]);
        Dvar_SetFromStringByName(item->dvar, v4);
    }
    return 1;
}

int __cdecl Item_Multi_CountSettings(itemDef_s *item)
{
    multiDef_s *multiPtr; // [esp+0h] [ebp-4h]

    multiPtr = Item_GetMultiDef(item);
    if (multiPtr)
        return multiPtr->count;
    else
        return 0;
}

int __cdecl Item_Multi_FindDvarByValue(itemDef_s *item)
{
    const char *VariantString; // eax
    multiDef_s *multiPtr; // [esp+0h] [ebp-10h]
    const char *string; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int ia; // [esp+8h] [ebp-8h]
    float value; // [esp+Ch] [ebp-4h]

    multiPtr = Item_GetMultiDef(item);
    if (!multiPtr)
        return 0;
    if (multiPtr->strDef)
    {
        string = Dvar_GetVariantString(item->dvar);
        for (i = 0; i < multiPtr->count; ++i)
        {
            if (!I_stricmp(string, multiPtr->dvarStr[i]))
                return i;
        }
    }
    else
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        value = atof(VariantString);
        for (ia = 0; ia < multiPtr->count; ++ia)
        {
            if (value == multiPtr->dvarValue[ia])
                return ia;
        }
    }
    return 0;
}

int __cdecl Item_List_NextEntryForKey(int key, int current, int count)
{
    bool v4; // [esp+0h] [ebp-Ch]
    bool v5; // [esp+4h] [ebp-8h]
    char v6; // [esp+Ah] [ebp-2h]
    char v7; // [esp+Bh] [ebp-1h]

    if (count < 0)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3199, 0, "%s\n\t(count) = %i", "(count >= 0)", count);
    if (current < 0 || current >= count && count)
        MyAssertHandler(
            ".\\ui\\ui_shared.cpp",
            3200,
            0,
            "%s\n\t(current) = %i",
            "(current >= 0 && (current < count || count == 0))",
            current);
    if (!count)
        return 0;
    if (key == 200 || key == 202)
    {
        v7 = 1;
    }
    else
    {
        v5 = key == 13 || key == 163 || key == 157;
        v7 = v5;
    }
    if (v7)
        return (current + 1) % count;
    if (key == 201)
    {
        v6 = 1;
    }
    else
    {
        v4 = key == 164 || key == 156;
        v6 = v4;
    }
    if (v6)
        return (current + count - 1) % count;
    else
        return current;
}

int __cdecl Item_DvarEnum_HandleKey(UiContext *dc, itemDef_s *item, int key)
{
    char *v4; // eax
    int next; // [esp+0h] [ebp-Ch]
    int count; // [esp+4h] [ebp-8h]
    int current; // [esp+8h] [ebp-4h]

    if (!dc)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3250, 0, "%s", "dc");
    if (!item)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3251, 0, "%s", "item");
    if (!item->dvar)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3252, 0, "%s", "item->dvar");
    if (!Item_ShouldHandleKey(dc, item, key))
        return 0;
    current = Item_DvarEnum_EnumIndex(item);
    count = Item_DvarEnum_CountSettings(item);
    next = Item_List_NextEntryForKey(key, current, count);
    if (next == current)
        return 0;
    v4 = va("%i", next);
    Dvar_SetFromStringByName(item->dvar, v4);
    return 1;
}

int __cdecl Item_DvarEnum_CountSettings(itemDef_s *item)
{
    const dvar_s *enumDvar; // [esp+0h] [ebp-4h]

    if (!item)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3052, 0, "%s", "item");
    if (item->type != 13)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3053, 0, "%s\n\t(item->type) = %i", "(item->type == 13)", item->type);
    if (!item->typeData.listBox)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3054, 0, "%s", "item->typeData.enumDvarName");
    enumDvar = Dvar_FindVar(item->typeData.enumDvarName);
    if (enumDvar->type == 6)
        return enumDvar->domain.enumeration.stringCount;
    else
        return 0;
}

int __cdecl Item_DvarEnum_EnumIndex(itemDef_s *item)
{
    int enumIndex; // [esp+0h] [ebp-Ch]
    int enumIndexa; // [esp+0h] [ebp-Ch]
    const dvar_s *enumDvar; // [esp+4h] [ebp-8h]
    const char *enumString; // [esp+8h] [ebp-4h]

    enumDvar = Dvar_FindVar(item->typeData.enumDvarName);
    if (enumDvar->type != 6)
        return 0;
    enumString = Dvar_GetVariantString(item->dvar);
    enumIndex = atoi(enumString);
    if (enumIndex >= 0 && enumIndex < enumDvar->domain.enumeration.stringCount)
        return enumIndex;
    for (enumIndexa = 0; enumIndexa < enumDvar->domain.enumeration.stringCount; ++enumIndexa)
    {
        if (!I_stricmp(enumString, *(const char **)(enumDvar->domain.integer.max + 4 * enumIndexa)))
            return enumIndexa;
    }
    return 0;
}

double __cdecl Item_Slider_ThumbPosition(int localClientNum, itemDef_s *item)
{
    const char *VariantString; // eax
    float x0; // [esp+Ch] [ebp-18h]
    float range; // [esp+10h] [ebp-14h]
    editFieldDef_s *editDef; // [esp+14h] [ebp-10h]
    float x; // [esp+18h] [ebp-Ch]
    float value; // [esp+20h] [ebp-4h]
    float valuea; // [esp+20h] [ebp-4h]
    float valueb; // [esp+20h] [ebp-4h]
    float valuec; // [esp+20h] [ebp-4h]

    editDef = Item_GetEditFieldDef(item);
    if (!editDef)
        return 0.0;
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    x0 = item->window.rect.x + item->textalignx;
    x = Item_GetRectPlacementX(item->textAlignMode & 3, x0, item->window.rect.w, 96.0);
    if (!editDef && item->dvar)
        return x;
    VariantString = Dvar_GetVariantString(item->dvar);
    value = atof(VariantString);
    if (editDef->minVal <= (double)value)
    {
        if (editDef->maxVal < (double)value)
            value = editDef->maxVal;
    }
    else
    {
        value = editDef->minVal;
    }
    range = editDef->maxVal - editDef->minVal;
    valuea = value - editDef->minVal;
    valueb = valuea / range;
    valuec = valueb * 84.0;
    return (float)(valuec + 5.0 + 1.0 + x);
}

double __cdecl Item_GetRectPlacementX(int alignX, float x0, float containerWidth, float selfWidth)
{
    if (!alignX)
        return x0;
    if (alignX == 1)
    {
        return (float)((containerWidth - selfWidth) * 0.5 + x0);
    }
    else
    {
        if (alignX != 2)
            MyAssertHandler(".\\ui\\ui_shared.cpp", 180, 0, "alignX == ITEM_ALIGN_RIGHT\n\t%i, %i", alignX, 2);
        return (float)(containerWidth - selfWidth + x0);
    }
}

double __cdecl Item_ListBox_ThumbPosition(int localClientNum, itemDef_s *item)
{
    int max; // [esp+Ch] [ebp-14h]
    float pos; // [esp+10h] [ebp-10h]
    float posb; // [esp+10h] [ebp-10h]
    float posa; // [esp+10h] [ebp-10h]
    float posc; // [esp+10h] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+14h] [ebp-Ch]
    float size; // [esp+18h] [ebp-8h]
    float sizea; // [esp+18h] [ebp-8h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        return 0.0;
    max = Item_ListBox_MaxScroll(localClientNum, item);
    if (!item)
    {
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 53, 0, "%s", "w");
    }
    if ((item->window.staticFlags & 0x200000) != 0)
    {
        if (max <= 0)
        {
            pos = 0.0;
        }
        else
        {
            size = item->window.rect.w - 32.0 - 2.0;
            pos = (size - 16.0) / (double)max;
        }
        posb = (double)listPtr->startPos[localClientNum] * pos;
        return (float)(item->window.rect.x + 1.0 + 16.0 + posb);
    }
    else
    {
        if (max <= 0)
        {
            posa = 0.0;
        }
        else
        {
            sizea = item->window.rect.h - 32.0 - 2.0;
            posa = (sizea - 16.0) / (double)max;
        }
        posc = (double)listPtr->startPos[localClientNum] * posa;
        return (float)(item->window.rect.y + 1.0 + 16.0 + posc);
    }
}

void __cdecl Scroll_Slider_SetThumbPos(UiContext *dc, itemDef_s *item)
{
    char *v2; // eax
    float v3; // [esp+Ch] [ebp-40h]
    float v4; // [esp+10h] [ebp-3Ch]
    float v5; // [esp+14h] [ebp-38h]
    float x0; // [esp+18h] [ebp-34h]
    float v7; // [esp+1Ch] [ebp-30h]
    float v8; // [esp+20h] [ebp-2Ch]
    float hIgnored; // [esp+24h] [ebp-28h] BYREF
    float yIgnored; // [esp+28h] [ebp-24h] BYREF
    const ScreenPlacement *scrPlace; // [esp+2Ch] [ebp-20h]
    float usableStart; // [esp+30h] [ebp-1Ch] BYREF
    editFieldDef_s *editDef; // [esp+34h] [ebp-18h]
    float cursorx; // [esp+38h] [ebp-14h]
    float usableWidth; // [esp+3Ch] [ebp-10h] BYREF
    float x; // [esp+40h] [ebp-Ch]
    const rectDef_s *rect; // [esp+44h] [ebp-8h]
    float value; // [esp+48h] [ebp-4h]

    editDef = Item_GetEditFieldDef(item);
    if (editDef)
    {
        if (!item)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
        rect = &item->window.rect;
        x0 = item->window.rect.x + item->textalignx;
        x = Item_GetRectPlacementX(item->textAlignMode & 3, x0, item->window.rect.w, 96.0);
        scrPlace = &scrPlaceView[dc->localClientNum];
        cursorx = ScrPlace_ApplyX(scrPlace, dc->cursor.x, 4);
        usableStart = x + 5.0 + 1.0;
        usableWidth = 84.0;
        yIgnored = 0.0;
        hIgnored = 0.0;
        ScrPlace_ApplyRect(
            scrPlace,
            &usableStart,
            &yIgnored,
            &usableWidth,
            &hIgnored,
            item->window.rect.horzAlign,
            item->window.rect.vertAlign);
        v7 = cursorx - usableStart;
        v5 = v7 - usableWidth;
        if (v5 < 0.0)
            v8 = cursorx - usableStart;
        else
            v8 = usableWidth;
        v4 = 0.0 - v7;
        if (v4 < 0.0)
            v3 = v8;
        else
            v3 = 0.0;
        value = v3 / usableWidth;
        value = (editDef->maxVal - editDef->minVal) * value;
        value = value + editDef->minVal;
        v2 = va("%g", value);
        Dvar_SetFromStringByName(item->dvar, v2);
    }
}

int __cdecl Item_Slider_HandleKey(UiContext *dc, itemDef_s *item, int key, int down)
{
    const char *VariantString; // eax
    char *v6; // eax
    char *v7; // [esp+4h] [ebp-38h]
    float v8; // [esp+8h] [ebp-34h]
    float v9; // [esp+Ch] [ebp-30h]
    float v11; // [esp+14h] [ebp-28h]
    float v12; // [esp+18h] [ebp-24h]
    float v14; // [esp+20h] [ebp-1Ch]
    float maxVal; // [esp+24h] [ebp-18h]
    float v16; // [esp+28h] [ebp-14h]
    float minVal; // [esp+2Ch] [ebp-10h]
    editFieldDef_s *editDef; // [esp+30h] [ebp-Ch]
    float step; // [esp+34h] [ebp-8h]
    float value; // [esp+38h] [ebp-4h]

    if (!item->dvar)
        return 0;
    if (!Item_ShouldHandleKey(dc, item, key))
        return 0;
    if (key == 200 || key == 201 || key == 202)
    {
        Scroll_Slider_SetThumbPos(dc, item);
        return 1;
    }
    else
    {
        editDef = Item_GetEditFieldDef(item);
        if (editDef)
        {
            step = (editDef->maxVal - editDef->minVal) * 0.05000000074505806;
            VariantString = Dvar_GetVariantString(item->dvar);
            value = atof(VariantString);
            if (key == 156 || key == 164)
            {
                v16 = value - step;
                minVal = editDef->minVal;
                v12 = v16 - minVal;
                if (v12 < 0.0)
                    v11 = minVal;
                else
                    v11 = value - step;
                v6 = va("%g", v11);
                Dvar_SetFromStringByName(item->dvar, v6);
                return 1;
            }
            else if (key == 157 || key == 163)
            {
                v14 = step + value;
                maxVal = editDef->maxVal;
                v9 = maxVal - v14;
                if (v9 < 0.0)
                    v8 = maxVal;
                else
                    v8 = step + value;
                v7 = va("%g", v8);
                Dvar_SetFromStringByName(item->dvar, v7);
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
}

void __cdecl Item_Action(UiContext *dc, itemDef_s *item)
{
    if (item)
        Item_RunScript(dc, item, (char*)item->action);
}

itemDef_s *__cdecl Menu_SetPrevCursorItem(UiContext *dc, menuDef_t *menu)
{
    int v3; // [esp+Ch] [ebp-40h]
    int v4; // [esp+14h] [ebp-38h]
    int v5; // [esp+1Ch] [ebp-30h]
    int v6; // [esp+24h] [ebp-28h]
    int v7; // [esp+2Ch] [ebp-20h]
    int v8; // [esp+34h] [ebp-18h]
    int v9; // [esp+3Ch] [ebp-10h]
    int localClientNum; // [esp+40h] [ebp-Ch]
    int oldCursor; // [esp+44h] [ebp-8h]
    int wrapped; // [esp+48h] [ebp-4h]

    wrapped = 0;
    localClientNum = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            36,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    oldCursor = menu->cursorItem[localClientNum];
    v9 = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            36,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            v9,
            1);
    if (menu->cursorItem[v9] < 0)
    {
        Menu_SetCursorItem(dc->localClientNum, menu, menu->itemCount - 1);
        wrapped = 1;
    }
    do
    {
        v8 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v8,
                1);
        if (menu->cursorItem[v8] <= -1)
            goto LABEL_27;
        v7 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v7,
                1);
        Menu_SetCursorItem(dc->localClientNum, menu, menu->cursorItem[v7] - 1);
        v6 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v6,
                1);
        if (menu->cursorItem[v6] < 0 && !wrapped)
        {
            wrapped = 1;
            Menu_SetCursorItem(dc->localClientNum, menu, menu->itemCount - 1);
        }
        v5 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v5,
                1);
        if (menu->cursorItem[v5] < 0)
        {
        LABEL_27:
            Menu_SetCursorItem(dc->localClientNum, menu, oldCursor);
            return 0;
        }
        v4 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v4,
                1);
    } while (!Item_SetFocus(dc, menu->items[menu->cursorItem[v4]], dc->cursor.x, dc->cursor.y));
    v3 = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            36,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            v3,
            1);
    return menu->items[menu->cursorItem[v3]];
}

itemDef_s *__cdecl Menu_SetNextCursorItem(UiContext *dc, menuDef_t *menu)
{
    int v3; // [esp+Ch] [ebp-38h]
    int v4; // [esp+14h] [ebp-30h]
    int v5; // [esp+1Ch] [ebp-28h]
    int v6; // [esp+24h] [ebp-20h]
    int v7; // [esp+2Ch] [ebp-18h]
    int v8; // [esp+34h] [ebp-10h]
    int localClientNum; // [esp+38h] [ebp-Ch]
    int oldCursor; // [esp+3Ch] [ebp-8h]
    int wrapped; // [esp+40h] [ebp-4h]

    wrapped = 0;
    localClientNum = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            36,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    oldCursor = menu->cursorItem[localClientNum];
    v8 = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            36,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            v8,
            1);
    if (menu->cursorItem[v8] == -1)
    {
        Menu_SetCursorItem(dc->localClientNum, menu, 0);
        wrapped = 1;
    }
    do
    {
        v7 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v7,
                1);
        if (menu->cursorItem[v7] >= menu->itemCount)
        {
            Menu_SetCursorItem(dc->localClientNum, menu, oldCursor);
            return 0;
        }
        v6 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v6,
                1);
        Menu_SetCursorItem(dc->localClientNum, menu, menu->cursorItem[v6] + 1);
        v5 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v5,
                1);
        if (menu->cursorItem[v5] >= menu->itemCount)
        {
            if (wrapped)
                return menu->items[oldCursor];
            wrapped = 1;
            Menu_SetCursorItem(dc->localClientNum, menu, 0);
        }
        v4 = dc->localClientNum;
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                36,
                0,
                "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                v4,
                1);
    } while (!Item_SetFocus(dc, menu->items[menu->cursorItem[v4]], dc->cursor.x, dc->cursor.y));
    v3 = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            36,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            v3,
            1);
    return menu->items[menu->cursorItem[v3]];
}

rectDef_s rect;
rectDef_s *__cdecl Item_CorrectedTextRect(int localClientNum, itemDef_s *item)
{
    rect.x = 0.0;
    rect.y = 0.0;
    rect.w = 0.0;
    rect.h = 0.0;
    rect.horzAlign = 0;
    rect.vertAlign = 0;
    if (item)
    {
        rect = *Item_GetTextRect(localClientNum, item);
        if (rect.w != 0.0)
            rect.y = rect.y - rect.h;
    }
    return &rect;
}

int __cdecl Menu_CheckOnKey(UiContext *dc, menuDef_t *menu, int key)
{
    itemDef_s *item; // [esp+14h] [ebp-184h]
    int i; // [esp+18h] [ebp-180h]
    ItemKeyHandler *handler; // [esp+1Ch] [ebp-17Ch]
    itemDef_s it; // [esp+20h] [ebp-178h] BYREF

    for (handler = menu->onKey; handler; handler = handler->next)
    {
        if (handler->key == key)
        {
        LABEL_5:
            it.parent = menu;
            Item_RunScript(dc, &it, (char*)handler->action);
            return 1;
        }
    }
    for (i = 0; i < menu->itemCount; ++i)
    {
        item = menu->items[i];
        if (Window_IsVisible(dc->localClientNum, &item->window)
            && (Window_HasFocus(dc->localClientNum, &item->window) || (item->window.staticFlags & 0x100000) != 0)
            && Item_IsVisible(dc->localClientNum, item)
            && ((item->dvarFlags & 3) == 0 || Item_EnableShowViaDvar(item, 1)))
        {
            for (handler = item->onKey; handler; handler = handler->next)
            {
                if (handler->key == key)
                    goto LABEL_5;
            }
        }
    }
    return 0;
}

void __cdecl UI_DrawWrappedText(
    const ScreenPlacement *scrPlace,
    const char *text,
    const rectDef_s *rect,
    Font_s *font,
    float x,
    float y,
    float scale,
    const float *color,
    int style,
    char textAlignMode,
    rectDef_s *textRect)
{
    DrawWrappedText(scrPlace, text, rect, font, x, y, scale, color, style, textAlignMode, textRect, 0, 0, 0);
}

void __cdecl DrawWrappedText(
    const ScreenPlacement *scrPlace,
    const char *text,
    const rectDef_s *rect,
    Font_s *font,
    float x,
    float y,
    float scale,
    const float *color,
    int style,
    char textAlignMode,
    rectDef_s *textRect,
    bool subtitle,
    const float *subtitleGlowColor,
    bool cinematic)
{
    float v14; // [esp+2Ch] [ebp-44Ch]
    float v15; // [esp+30h] [ebp-448h]
    float v16; // [esp+34h] [ebp-444h]
    float v17; // [esp+40h] [ebp-438h]
    float lineX; // [esp+44h] [ebp-434h]
    float height; // [esp+4Ch] [ebp-42Ch]
    float normalizedScale; // [esp+50h] [ebp-428h]
    float textWidth; // [esp+54h] [ebp-424h]
    int len; // [esp+58h] [ebp-420h]
    int targetLineWidth; // [esp+5Ch] [ebp-41Ch]
    const char *wrapPosition; // [esp+60h] [ebp-418h]
    float lineWidth; // [esp+64h] [ebp-414h]
    char buff[1028]; // [esp+68h] [ebp-410h] BYREF
    int xAlignMode; // [esp+470h] [ebp-8h]
    const char *p; // [esp+474h] [ebp-4h]
    float xa; // [esp+490h] [ebp+18h]
    float ya; // [esp+494h] [ebp+1Ch]

    if (rect == textRect)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 4656, 0, "%s", "rect != textRect");
    textRect->x = x + rect->w;
    textRect->y = y;
    textRect->w = 0.0;
    textRect->h = 0.0;
    normalizedScale = R_NormalizedTextScale(font, scale);
    height = (float)UI_TextHeight(font, scale);
    textWidth = (float)UI_TextWidth(text, 0, font, scale);
    xAlignMode = textAlignMode & 3;
    xa = Item_GetTextPlacementX(xAlignMode, x, rect->w, 0.0);
    ya = Item_GetTextPlacementY(textAlignMode & 0xC, y, rect->h, height);
    if (rect->w == 0.0 || rect->w >= (double)textWidth || xAlignMode != 1)
    {
        targetLineWidth = (int)rect->w;
    }
    else
    {
        v17 = textWidth / rect->w;
        v16 = ceil(v17);
        v15 = textWidth / SnapFloat(v16);
        v14 = ceil(v15);
        targetLineWidth = UI_PickWordWrapLineWidth(text, 1024, font, normalizedScale, SnapFloatToInt(v16), SnapFloatToInt(v14), SnapFloatToInt(rect->w));
    }
    p = text;
    while (*p)
    {
        wrapPosition = R_TextLineWrapPosition(p, 1024, targetLineWidth, font, normalizedScale);
        len = wrapPosition - p;
        memcpy((unsigned __int8 *)buff, (unsigned __int8 *)p, wrapPosition - p);
        buff[len] = 0;
        lineWidth = (float)UI_TextWidth(buff, 0, font, scale);
        if (xAlignMode == 1)
        {
            lineX = xa - lineWidth * 0.5;
        }
        else if (xAlignMode == 2)
        {
            lineX = xa - lineWidth;
        }
        else
        {
            lineX = xa;
        }
        if (subtitle)
        {
            if (!subtitleGlowColor)
                MyAssertHandler(".\\ui\\ui_shared.cpp", 4702, 0, "%s", "subtitleGlowColor");
            UI_DrawTextWithGlow(
                scrPlace,
                buff,
                0x7FFFFFFF,
                font,
                lineX,
                ya,
                rect->horzAlign,
                rect->vertAlign,
                scale,
                color,
                style,
                subtitleGlowColor,
                1,
                cinematic);
        }
        else
        {
            UI_DrawText(scrPlace, buff, 0x7FFFFFFF, font, lineX, ya, rect->horzAlign, rect->vertAlign, scale, color, style);
        }
        ya = height + 5.0 + ya;
        if (lineWidth > (double)textRect->w)
        {
            if (lineX > (double)textRect->x)
                MyAssertHandler(".\\ui\\ui_shared.cpp", 4713, 0, "textRect->x >= lineX\n\t%g, %g", textRect->x, lineX);
            textRect->x = lineX;
            textRect->w = lineWidth;
        }
        for (p += len; isspace(*p); ++p)
            ;
    }
    textRect->h = ya - textRect->y;
}

double __cdecl Item_GetTextPlacementX(int alignX, float x0, float containerWidth, float selfWidth)
{
    if (!alignX)
        return x0;
    if (alignX == 1)
    {
        return (float)((containerWidth - selfWidth) * 0.5 + x0);
    }
    else
    {
        if (alignX != 2)
            MyAssertHandler(".\\ui\\ui_shared.cpp", 191, 0, "alignX == ITEM_ALIGN_RIGHT\n\t%i, %i", alignX, 2);
        return (float)(containerWidth - selfWidth + x0);
    }
}

double __cdecl Item_GetTextPlacementY(int alignY, float y0, float containerHeight, float selfHeight)
{
    switch (alignY)
    {
    case 0:
        return y0;
    case 4:
        return (float)(y0 + selfHeight);
    case 8:
        return (float)((containerHeight + selfHeight) * 0.5 + y0);
    default:
        if (alignY != 12)
            MyAssertHandler(".\\ui\\ui_shared.cpp", 215, 0, "%s\n\t(alignY) = %i", "(alignY == 12)", alignY);
        return (float)(y0 + containerHeight);
    }
}

int __cdecl UI_PickWordWrapLineWidth(
    const char *text,
    int bufferSize,
    Font_s *font,
    float normalizedScale,
    int targetLineCount,
    int widthGuess,
    int widthLimit)
{
    int lineCount; // [esp+4h] [ebp-8h]
    const char *p; // [esp+8h] [ebp-4h]

    while (widthGuess < widthLimit)
    {
        p = text;
        lineCount = 0;
        while (*p)
        {
            for (p = R_TextLineWrapPosition(p, bufferSize, widthGuess, font, normalizedScale); isspace(*p); ++p)
                ;
            ++lineCount;
        }
        if (lineCount == targetLineCount)
            return widthGuess;
        widthGuess += 8;
    }
    return widthLimit;
}

void __cdecl UI_DrawWrappedTextSubtitled(
    const ScreenPlacement *scrPlace,
    const char *text,
    const rectDef_s *rect,
    Font_s *font,
    float x,
    float y,
    float scale,
    const float *color,
    int style,
    char textAlignMode,
    rectDef_s *textRect,
    const float *subtitleGlowColor,
    bool cinematic)
{
    DrawWrappedText(
        scrPlace,
        text,
        rect,
        font,
        x,
        y,
        scale,
        color,
        style,
        textAlignMode,
        textRect,
        1,
        subtitleGlowColor,
        cinematic);
}

int __cdecl UI_GetKeyBindingLocalizedString(int localClientNum, const char *command, char *keys)
{
    return GetKeyBindingLocalizedString(localClientNum, command, keys, 0);
}

int __cdecl Display_KeyBindPending()
{
    return g_waitingForKey;
}

int __cdecl Item_Bind_HandleKey(UiContext *dc, itemDef_s *item, int key, int down)
{
    bool v5; // [esp+3h] [ebp-Dh]
    int bindCount; // [esp+4h] [ebp-Ch]
    int boundKeys[2]; // [esp+8h] [ebp-8h] BYREF

    if (g_waitingForKey)
    {
        if (g_bindItem)
        {
            if ((key & 0x400) != 0)
            {
                return 1;
            }
            else if (key == 27)
            {
                g_waitingForKey = 0;
                return 1;
            }
            else if (key == 96)
            {
                return 1;
            }
            else
            {
                bindCount = Key_GetCommandAssignment(dc->localClientNum, item->dvar, boundKeys);
                if (key == 127 || bindCount == 2)
                {
                    Key_SetBinding(dc->localClientNum, boundKeys[0], (char *)"");
                    Key_SetBinding(dc->localClientNum, boundKeys[1], (char *)"");
                }
                if (key != 127)
                    Key_SetBinding(dc->localClientNum, key, (char *)item->dvar);
                g_waitingForKey = 0;
                return 1;
            }
        }
        else
        {
            return 0;
        }
    }
    else if (down && (key != 200 || !Item_ContainsMouse(dc, item) ? (v5 = key == 13) : (v5 = 1), v5))
    {
        g_waitingForKey = 1;
        g_bindItem = item;
        return 1;
    }
    else
    {
        return 0;
    }
}

menuDef_t *__cdecl Menu_GetFocused(UiContext *dc)
{
    int i; // [esp+10h] [ebp-4h]

    for (i = dc->openMenuCount - 1; i >= 0; --i)
    {
        if (Window_HasFocus(dc->localClientNum, &dc->menuStack[i]->window)
            && Window_IsVisible(dc->localClientNum, &dc->menuStack[i]->window))
        {
            return dc->menuStack[i];
        }
    }
    return 0;
}

void __cdecl Menu_SetFeederSelection(UiContext *dc, menuDef_t *menu, int feeder, int index, const char *name)
{
    int i; // [esp+8h] [ebp-Ch]
    itemDef_s *item; // [esp+Ch] [ebp-8h]
    listBoxDef_s *listPtr; // [esp+10h] [ebp-4h]

    if (!menu)
    {
        if (name)
            menu = Menus_FindByName(dc, name);
        else
            menu = Menu_GetFocused(dc);
    }
    if (menu)
    {
        for (i = 0; i < menu->itemCount; ++i)
        {
            item = menu->items[i];
            if (item->special == (double)feeder)
            {
                item->cursorPos[dc->localClientNum] = index;
                UI_FeederSelection(dc->localClientNum, item->special, index);
                listPtr = Item_GetListBoxDef(item);
                if (listPtr)
                {
                    if (index)
                    {
                        if (listPtr->startPos[dc->localClientNum] > index)
                            listPtr->startPos[dc->localClientNum] = index;
                    }
                    else
                    {
                        item->cursorPos[dc->localClientNum] = 0;
                        listPtr->startPos[dc->localClientNum] = 0;
                    }
                }
            }
        }
    }
}

int __cdecl Menus_AnyFullScreenVisible(UiContext *dc)
{
    int i; // [esp+4h] [ebp-4h]

    for (i = dc->openMenuCount - 1; i >= 0; --i)
    {
        if (Window_IsVisible(dc->localClientNum, &dc->menuStack[i]->window) && dc->menuStack[i]->fullScreen)
            return 1;
    }
    return 0;
}

char __cdecl Menu_IsVisible(UiContext *dc, menuDef_t *menu)
{
    if (!Window_IsVisible(dc->localClientNum, &menu->window))
        return 0;
    if (menu->window.ownerDrawFlags && !UI_OwnerDrawVisible(menu->window.ownerDrawFlags))
        return 0;
    if ((menu->window.staticFlags & 0x20000000) != 0
        && CL_IsCgameInitialized(dc->localClientNum)
        && CG_ScopeIsOverlayed(dc->localClientNum))
    {
        return 0;
    }
    if ((menu->window.staticFlags & 0x10000000) != 0
        && CL_IsCgameInitialized(dc->localClientNum)
        && CG_Flashbanged(dc->localClientNum))
    {
        return 0;
    }
    if ((menu->window.staticFlags & 0x40000000) != 0)
    {
        if (dc->localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
                1063,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                dc->localClientNum);
        if ((clientUIActives[0].keyCatchers & 0x10) != 0)
            return 0;
    }
    if (!menu->visibleExp.numEntries || IsExpressionTrue(dc->localClientNum, &menu->visibleExp))
        return 1;
    if (uiscript_debug->current.integer)
    {
        if (menu->window.name)
            Com_Printf(13, "hiding the %s menu becuase the 'visible when' expression was false\n", menu->window.name);
        else
            Com_Printf(13, "hiding the %s menu becuase the 'visible when' expression was false\n", "unnamed");
    }
    return 0;
}

char __cdecl Menu_Paint(UiContext *dc, menuDef_t *menu)
{
    float fadeCycle; // [esp+1Ch] [ebp-14h]
    float v4; // [esp+20h] [ebp-10h]
    float v5; // [esp+24h] [ebp-Ch]
    int i; // [esp+28h] [ebp-8h]

    PROF_SCOPED("Menu_Paint");

    ZoneText(menu->window.name, strlen(menu->window.name));

    iassert(menu);

    if (*(_BYTE *)ui_showMenuOnly->current.integer
        && menu->window.name
        && I_stricmp(menu->window.name, ui_showMenuOnly->current.string))
    {
        return 0;
    }

    if (!Menu_IsVisible(dc, menu))
        return 0;

    if (menu->soundName)
        UI_PlayLocalSoundAliasByName(dc->localClientNum, menu->soundName);

    if (menu->blurRadius != 0.0)
    {
        v5 = dc->blurRadiusOut * dc->blurRadiusOut + menu->blurRadius * menu->blurRadius;
        v4 = sqrt(v5);
        dc->blurRadiusOut = v4;
    }

    if (menu->rectXExp.numEntries)
        menu->window.rect.x = GetExpressionFloat(dc->localClientNum, &menu->rectXExp);

    if (menu->rectYExp.numEntries)
        menu->window.rect.y = GetExpressionFloat(dc->localClientNum, &menu->rectYExp);

    Menu_UpdatePosition(dc->localClientNum, menu);
    if (menu->fullScreen && menu->window.background)
    {
        if (!menu)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
        UI_DrawHandlePic(
            &scrPlaceView[dc->localClientNum],
            0.0,
            0.0,
            640.0,
            480.0,
            menu->window.rect.horzAlign,
            menu->window.rect.vertAlign,
            0,
            menu->window.background);
    }
    fadeCycle = (float)menu->fadeCycle;
    Window_Paint(dc, &menu->window, menu->fadeAmount, menu->fadeInAmount, menu->fadeClamp, fadeCycle);

    for (i = 0; i < menu->itemCount; ++i)
        Item_Paint(dc, menu->items[i]);

    if (g_debugMode)
    {
        if (!menu)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
        UI_DrawRect(
            &scrPlaceView[dc->localClientNum],
            menu->window.rect.x,
            menu->window.rect.y,
            menu->window.rect.w,
            menu->window.rect.h,
            menu->window.rect.horzAlign,
            menu->window.rect.vertAlign,
            1.0,
            colorMagenta);
    }

    return 1;
}

void __cdecl Window_Paint(
    UiContext *dc,
    windowDef_t *w,
    float fadeAmount,
    float fadeInAmount,
    float fadeClamp,
    float fadeCycle)
{
    PROF_SCOPED("Window_Paint");

    float *v6; // [esp+24h] [ebp-80h]
    float *v7; // [esp+28h] [ebp-7Ch]
    float *v8; // [esp+2Ch] [ebp-78h]
    int v9; // [esp+38h] [ebp-6Ch]
    int v10; // [esp+40h] [ebp-64h]
    int v11; // [esp+48h] [ebp-5Ch]
    int localClientNum; // [esp+50h] [ebp-54h]
    float fillRect; // [esp+6Ch] [ebp-38h]
    float fillRect_4; // [esp+70h] [ebp-34h]
    float fillRect_8; // [esp+74h] [ebp-30h]
    float fillRect_12; // [esp+78h] [ebp-2Ch]
    float lowColor[4]; // [esp+84h] [ebp-20h] BYREF
    const ScreenPlacement *scrPlace; // [esp+94h] [ebp-10h]
    const float *foreColor; // [esp+98h] [ebp-Ch]
    int flags; // [esp+9Ch] [ebp-8h] BYREF
    const rectDef_s *origRect; // [esp+A0h] [ebp-4h]

    if (!w)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    origRect = &w->rect;
    fillRect = w->rect.x;
    fillRect_4 = w->rect.y;
    fillRect_8 = w->rect.w;
    fillRect_12 = w->rect.h;
    scrPlace = &scrPlaceView[dc->localClientNum];
    if (g_debugMode)
        UI_DrawRect(
            scrPlace,
            origRect->x,
            origRect->y,
            origRect->w,
            origRect->h,
            origRect->horzAlign,
            origRect->vertAlign,
            1.0,
            colorWhite);
    if (w && (w->style || w->border))
    {
        if (w->border)
        {
            fillRect = fillRect + w->borderSize;
            fillRect_4 = fillRect_4 + w->borderSize;
            fillRect_8 = fillRect_8 - (w->borderSize + 1.0);
            fillRect_12 = fillRect_12 - (w->borderSize + 1.0);
        }
        switch (w->style)
        {
        case 1:
            if (w->background)
            {
                localClientNum = dc->localClientNum;
                if (dc->localClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                        23,
                        0,
                        "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        localClientNum,
                        1);
                flags = w->dynamicFlags[localClientNum];
                Fade(&flags, &w->backColor[3], fadeClamp, &w->nextTime, SnapFloatToInt(fadeCycle), 1, fadeAmount, fadeInAmount, dc);
                Window_SetDynamicFlags(dc->localClientNum, w, flags);
                UI_DrawHandlePic(
                    scrPlace,
                    fillRect,
                    fillRect_4,
                    fillRect_8,
                    fillRect_12,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    w->backColor,
                    w->background);
            }
            else
            {
                UI_FillRect(
                    scrPlace,
                    fillRect,
                    fillRect_4,
                    fillRect_8,
                    fillRect_12,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    w->backColor);
            }
            break;
        case 2:
            KISAK_NULLSUB();
            break;
        case 3:
            v11 = dc->localClientNum;
            if (dc->localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                    23,
                    0,
                    "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    v11,
                    1);
            if ((w->dynamicFlags[v11] & 0x10000) != 0)
                v8 = w->foreColor;
            else
                v8 = 0;
            foreColor = v8;
            UI_DrawHandlePic(
                scrPlace,
                fillRect,
                fillRect_4,
                fillRect_8,
                fillRect_12,
                origRect->horzAlign,
                origRect->vertAlign,
                v8,
                w->background);
            break;
        case 5:
            if (w->background)
            {
                v9 = dc->localClientNum;
                if (dc->localClientNum)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                        23,
                        0,
                        "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        v9,
                        1);
                if ((w->dynamicFlags[v9] & 0x10000) != 0)
                    v6 = w->foreColor;
                else
                    v6 = 0;
                foreColor = v6;
                UI_DrawHandlePic(
                    scrPlace,
                    fillRect,
                    fillRect_4,
                    fillRect_8,
                    fillRect_12,
                    origRect->horzAlign,
                    origRect->vertAlign,
                    v6,
                    w->background);
            }
            break;
        case 6:
            v10 = dc->localClientNum;
            if (dc->localClientNum)
                MyAssertHandler(
                    "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
                    23,
                    0,
                    "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    v10,
                    1);
            if ((w->dynamicFlags[v10] & 0x10000) != 0)
                v7 = w->foreColor;
            else
                v7 = 0;
            foreColor = v7;
            UI_DrawLoadBar(
                scrPlace,
                fillRect,
                fillRect_4,
                fillRect_8,
                fillRect_12,
                origRect->horzAlign,
                origRect->vertAlign,
                v7,
                w->background);
            break;
        default:
            break;
        }
        switch (w->border)
        {
        case 1:
            UI_DrawRect(
                scrPlace,
                origRect->x,
                origRect->y,
                origRect->w,
                origRect->h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                w->borderColor);
            break;
        case 2:
            UI_DrawTopBottom(
                scrPlace,
                origRect->x,
                origRect->y,
                origRect->w,
                origRect->h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                w->borderColor);
            break;
        case 3:
            UI_DrawSides(
                scrPlace,
                origRect->x,
                origRect->y,
                origRect->w,
                origRect->h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                w->borderColor);
            break;
        case 4:
            KISAK_NULLSUB();
            KISAK_NULLSUB();
            break;
        case 5:
            Vec3Scale(w->borderColor, ui_borderLowLightScale->current.value, lowColor);
            lowColor[3] = w->borderColor[3];
            UI_DrawHighlightRect(
                scrPlace,
                origRect->x,
                origRect->y,
                origRect->w,
                origRect->h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                w->borderColor,
                lowColor);
            break;
        case 6:
            Vec3Scale(w->borderColor, ui_borderLowLightScale->current.value, lowColor);
            lowColor[3] = w->borderColor[3];
            UI_DrawHighlightRect(
                scrPlace,
                origRect->x,
                origRect->y,
                origRect->w,
                origRect->h,
                origRect->horzAlign,
                origRect->vertAlign,
                w->borderSize,
                lowColor,
                w->borderColor);
            break;
        }
    }
}

void __cdecl Fade(
    int *flags,
    float *f,
    float clamp,
    int *nextTime,
    int offsetTime,
    int bFlags,
    float fadeAmount,
    float fadeInAmount,
    UiContext *dc)
{
    if ((*flags & 0x30) != 0 && dc->realTime > *nextTime)
    {
        *nextTime = offsetTime + dc->realTime;
        if ((*flags & 0x10) != 0)
        {
            *f = *f - fadeAmount;
            if (bFlags)
            {
                if (*f <= 0.0)
                    *flags &= 0xFFFFFFEB;
            }
        }
        else
        {
            *f = *f + fadeInAmount;
            if (clamp <= (double)*f)
            {
                *f = clamp;
                if (bFlags)
                    *flags &= ~0x20u;
            }
        }
    }
}

void __cdecl Item_Paint(UiContext *dc, itemDef_s *item)
{
    PROF_SCOPED("Item_Paint");

    //ZoneText(item->na)
    menuDef_t *v2; // esi
    menuDef_t *v3; // esi
    char *String; // eax
    char *ExpressionResultString; // eax
    float fadeCycle; // [esp+1Ch] [ebp-78h]
    rectDef_s *r; // [esp+34h] [ebp-60h]
    menuDef_t *parent; // [esp+48h] [ebp-4Ch]
    char lowerCaseName[68]; // [esp+4Ch] [ebp-48h] BYREF

    parent = item->parent;
    if (item)
    {
        if (item->window.ownerDrawFlags)
        {
            if (UI_OwnerDrawVisible(item->window.ownerDrawFlags))
                Window_AddDynamicFlags(dc->localClientNum, &item->window, 4);
            else
                Window_RemoveDynamicFlags(dc->localClientNum, &item->window, 4);
        }
        if ((item->dvarFlags & 0xC) == 0 || Item_EnableShowViaDvar(item, 4))
        {
            if (item->forecolorAExp.numEntries)
                item->window.foreColor[3] = GetExpressionFloat(dc->localClientNum, &item->forecolorAExp);
            if (Item_IsVisible(dc->localClientNum, item))
            {
                if (item->rectXExp.numEntries)
                {
                    v2 = item->parent;
                    item->window.rect.x = GetExpressionFloat(dc->localClientNum, &item->rectXExp) + v2->window.rect.x;
                }
                if (item->rectYExp.numEntries)
                {
                    v3 = item->parent;
                    item->window.rect.y = GetExpressionFloat(dc->localClientNum, &item->rectYExp) + v3->window.rect.y;
                }
                if (item->rectWExp.numEntries)
                    item->window.rect.w = GetExpressionFloat(dc->localClientNum, &item->rectWExp);
                if (item->rectHExp.numEntries)
                    item->window.rect.h = GetExpressionFloat(dc->localClientNum, &item->rectHExp);
                if (item->window.style == 5)
                {
                    String = (char *)Dvar_GetString(item->dvar);
                    I_strncpyz(lowerCaseName, String, 64);
                    I_strlwr(lowerCaseName);
                    item->window.background = Material_RegisterHandle(lowerCaseName, item->imageTrack);
                }
                else if (item->materialExp.numEntries)
                {
                    ExpressionResultString = GetExpressionResultString(dc->localClientNum, &item->materialExp);
                    I_strncpyz(lowerCaseName, ExpressionResultString, 64);
                    I_strlwr(lowerCaseName);
                    item->window.background = Material_RegisterHandle(lowerCaseName, item->imageTrack);
                }
                fadeCycle = (float)parent->fadeCycle;
                Window_Paint(dc, &item->window, parent->fadeAmount, parent->fadeInAmount, parent->fadeClamp, fadeCycle);
                if (g_debugMode)
                {
                    r = Item_CorrectedTextRect(dc->localClientNum, item);
                    UI_DrawRect(
                        &scrPlaceView[dc->localClientNum],
                        r->x,
                        r->y,
                        r->w,
                        r->h,
                        r->horzAlign,
                        r->vertAlign,
                        1.0,
                        colorGreen);
                }
                if (item->window.style != 5)
                {
                    switch (item->type)
                    {
                    case 0:
                    case 1:
                        Item_Text_Paint(dc, item);
                        break;
                    case 4:
                    case 9:
                    case 0x10:
                    case 0x11:
                    case 0x12:
                        Item_TextField_Paint(dc, item);
                        break;
                    case 6:
                        Item_ListBox_Paint(dc, item);
                        break;
                    case 8:
                        Item_OwnerDraw_Paint(dc, item);
                        break;
                    case 0xA:
                        Item_Slider_Paint(dc, item);
                        break;
                    case 0xB:
                        Item_YesNo_Paint(dc, item);
                        break;
                    case 0xC:
                        Item_Multi_Paint(dc, item);
                        break;
                    case 0xD:
                        Item_DvarEnum_Paint(dc, item);
                        break;
                    case 0xE:
                        Item_Bind_Paint(dc, item);
                        break;
                    case 0x13:
                        Item_GameMsgWindow_Paint(dc, item);
                        break;
                    default:
                        return;
                    }
                }
            }
        }
        else if (Window_HasFocus(dc->localClientNum, &item->window))
        {
            Menu_HandleKey(dc, parent, 155, 1);
            Menu_HandleKey(dc, parent, 155, 0);
        }
    }
}

const float MY_SUBTITLE_GLOWCOLOR[4] = { 0.0f, 0.3f, 0.0f, 1.0f };
void __cdecl Item_Text_Paint(UiContext *dc, itemDef_s *item)
{
    PROF_SCOPED("Item_Text_Paint");

    char *VariantString; // eax
    Font_s *font; // [esp+2Ch] [ebp-434h]
    bool subtitle; // [esp+33h] [ebp-42Dh]
    char *textPtr; // [esp+34h] [ebp-42Ch]
    float color[4]; // [esp+3Ch] [ebp-424h] BYREF
    const rectDef_s *rect; // [esp+4Ch] [ebp-414h]
    char text[1028]; // [esp+50h] [ebp-410h] BYREF
    const rectDef_s *textRect; // [esp+458h] [ebp-8h]
    bool cinematic; // [esp+45Fh] [ebp-1h]

    cinematic = (item->itemFlags & 2) != 0;
    subtitle = cinematic;
    if (cinematic)
        item->text = "NOT USING CINEMATIC_SUBTITLES";
    if (item->text)
    {
        textPtr = (char *)item->text;
    }
    else if (item->textExp.numEntries)
    {
        textPtr = GetExpressionResultString(dc->localClientNum, &item->textExp);
    }
    else
    {
        if (!item->dvar)
            return;
        VariantString = (char *)Dvar_GetVariantString(item->dvar);
        I_strncpyz(text, VariantString, 1024);
        textPtr = text;
    }
    if (*textPtr == 64)
        textPtr = UI_SafeTranslateString(textPtr + 1);
    if (*textPtr)
    {
        Item_TextColor(dc, item, (float (*)[4])color);
        Item_SetTextExtents(dc->localClientNum, item, textPtr);
        if ((item->window.staticFlags & 0x800000) != 0)
        {
            Item_Text_AutoWrapped_Paint(dc->localClientNum, item, textPtr, color, subtitle, MY_SUBTITLE_GLOWCOLOR, cinematic);
        }
        else
        {
            textRect = Item_GetTextRect(dc->localClientNum, item);
            if (!item)
                MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
            rect = &item->window.rect;
            font = UI_GetFontHandle(&scrPlaceView[dc->localClientNum], item->fontEnum, item->textscale);
            if (subtitle)
                UI_DrawTextWithGlow(
                    &scrPlaceView[dc->localClientNum],
                    textPtr,
                    0x7FFFFFFF,
                    font,
                    textRect->x,
                    textRect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    item->textscale,
                    color,
                    item->textStyle,
                    MY_SUBTITLE_GLOWCOLOR,
                    1,
                    cinematic);
            else
                UI_DrawText(
                    &scrPlaceView[dc->localClientNum],
                    textPtr,
                    0x7FFFFFFF,
                    font,
                    textRect->x,
                    textRect->y,
                    rect->horzAlign,
                    rect->vertAlign,
                    item->textscale,
                    color,
                    item->textStyle);
        }
    }
}

void __cdecl Item_SetTextExtents(int localClientNum, itemDef_s *item, const char *text)
{
    bool v3; // [esp+20h] [ebp-40h]
    bool v4; // [esp+24h] [ebp-3Ch]
    Font_s *font; // [esp+28h] [ebp-38h]
    bool isOwnerDraw; // [esp+2Fh] [ebp-31h]
    float xAdj; // [esp+30h] [ebp-30h]
    rectDef_s newRect; // [esp+3Ch] [ebp-24h] BYREF
    const char *dvarString; // [esp+54h] [ebp-Ch]
    bool isDvarField; // [esp+5Bh] [ebp-5h]
    int xAlignMode; // [esp+5Ch] [ebp-4h]

    if (!text)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 4525, 0, "%s", "text");
    xAlignMode = item->textAlignMode & 3;
    isOwnerDraw = item->type == 8;
    v4 = item->type != 8 && item->dvar;
    isDvarField = v4;
    v3 = xAlignMode && (isOwnerDraw || isDvarField);
    if (v3 || Item_GetTextRect(localClientNum, item)->w == 0.0)
    {
        font = UI_GetFontHandle(&scrPlaceView[localClientNum], item->fontEnum, item->textscale);
        newRect.x = item->textalignx;
        newRect.y = item->textaligny;
        newRect.w = (float)UI_TextWidth(text, 0, font, item->textscale);
        newRect.h = (float)UI_TextHeight(font, item->textscale);
        newRect.horzAlign = item->window.rect.horzAlign;
        newRect.vertAlign = item->window.rect.vertAlign;
        if (xAlignMode)
        {
            if (xAlignMode != 2 && xAlignMode != 1)
                MyAssertHandler(
                    ".\\ui\\ui_shared.cpp",
                    4550,
                    0,
                    "%s\n\t(xAlignMode) = %i",
                    "(xAlignMode == 2 || xAlignMode == 1)",
                    xAlignMode);
            xAdj = item->window.rect.w - (double)UI_TextWidth(text, 0, font, item->textscale);
            if (isOwnerDraw)
            {
                xAdj = xAdj - (double)UI_OwnerDrawWidth(item->window.ownerDraw, font, item->textscale);
            }
            else if (isDvarField && Item_IsTextField(item))
            {
                dvarString = Dvar_GetVariantString(item->dvar);
                xAdj = xAdj - (double)UI_TextWidth(dvarString, 0, font, item->textscale);
            }
            if (xAlignMode == 1)
                xAdj = xAdj * 0.5;
            newRect.x = newRect.x + xAdj;
        }
        newRect.y = Item_GetTextPlacementY(item->textAlignMode & 0xC, newRect.y, item->window.rect.h, newRect.h);
        ToWindowCoords(&newRect.x, &newRect.y, &item->window);
        Item_SetTextRect(localClientNum, item, &newRect);
    }
}

void __cdecl ToWindowCoords(float *x, float *y, const windowDef_t *window)
{
    if (window->border)
    {
        *x = *x + window->borderSize;
        *y = *y + window->borderSize;
    }
    if (!window)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    *x = *x + window->rect.x;
    *y = *y + window->rect.y;
}

void __cdecl Item_TextColor(UiContext *dc, itemDef_s *item, float (*newColor)[4])
{
    float *disableColor; // edx
    float v4; // [esp+1Ch] [ebp-4Ch]
    float v5; // [esp+20h] [ebp-48h]
    float t; // [esp+28h] [ebp-40h]
    float v7; // [esp+2Ch] [ebp-3Ch]
    float v8; // [esp+34h] [ebp-34h]
    float v9; // [esp+38h] [ebp-30h]
    int localClientNum; // [esp+4Ch] [ebp-1Ch]
    float lowLight[4]; // [esp+50h] [ebp-18h] BYREF
    menuDef_t *parent; // [esp+60h] [ebp-8h]
    int flags; // [esp+64h] [ebp-4h] BYREF

    parent = item->parent;
    localClientNum = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            23,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    flags = item->window.dynamicFlags[localClientNum];
    Fade(
        &flags,
        &item->window.foreColor[3],
        parent->fadeClamp,
        &item->window.nextTime,
        parent->fadeCycle,
        1,
        parent->fadeAmount,
        parent->fadeInAmount,
        dc);
    Window_SetDynamicFlags(dc->localClientNum, &item->window, flags);
    if (Window_HasFocus(dc->localClientNum, &item->window))
    {
        lowLight[0] = parent->focusColor[0] * 0.800000011920929;
        lowLight[1] = parent->focusColor[1] * 0.800000011920929;
        lowLight[2] = parent->focusColor[2] * 0.800000011920929;
        lowLight[3] = parent->focusColor[3] * 0.800000011920929;
        v9 = (float)(dc->realTime / 75);
        v7 = sin(v9);
        t = v7 * 0.5 + 0.5;
        LerpColor(parent->focusColor, lowLight, (float *)newColor, t);
    }
    else if (item->textStyle != 1 || ((dc->realTime / 256) & 1) != 0)
    {
        (*newColor)[0] = item->window.foreColor[0];
        (*newColor)[1] = item->window.foreColor[1];
        (*newColor)[2] = item->window.foreColor[2];
        (*newColor)[3] = item->window.foreColor[3];
    }
    else
    {
        lowLight[0] = item->window.foreColor[0] * 0.800000011920929;
        lowLight[1] = item->window.foreColor[1] * 0.800000011920929;
        lowLight[2] = item->window.foreColor[2] * 0.800000011920929;
        lowLight[3] = item->window.foreColor[3] * 0.800000011920929;
        v8 = (float)(dc->realTime / 75);
        v5 = sin(v8);
        v4 = v5 * 0.5 + 0.5;
        LerpColor(item->window.foreColor, lowLight, (float *)newColor, v4);
    }
    if (item->enableDvar
        && *item->enableDvar
        && item->dvarTest
        && *item->dvarTest
        && (item->dvarFlags & 3) != 0
        && !Item_EnableShowViaDvar(item, 1))
    {
        disableColor = parent->disableColor;
        (*newColor)[0] = parent->disableColor[0];
        (*newColor)[1] = disableColor[1];
        (*newColor)[2] = disableColor[2];
        (*newColor)[3] = disableColor[3];
    }
}

void __cdecl Item_Text_AutoWrapped_Paint(
    int localClientNum,
    itemDef_s *item,
    const char *text,
    const float *color,
    bool subtitle,
    const float *subtitleGlowColor,
    bool cinematic)
{
    Font_s *font; // [esp+24h] [ebp-10h]
    float x; // [esp+28h] [ebp-Ch] BYREF
    float y; // [esp+2Ch] [ebp-8h] BYREF
    const rectDef_s *rect; // [esp+30h] [ebp-4h]

    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    rect = &item->window.rect;
    font = UI_GetFontHandle(&scrPlaceView[localClientNum], item->fontEnum, item->textscale);
    x = item->textalignx;
    y = item->textaligny;
    ToWindowCoords(&x, &y, &item->window);
    if (subtitle)
    {
        if (!subtitleGlowColor)
            MyAssertHandler(".\\ui\\ui_shared.cpp", 4756, 0, "%s", "subtitleGlowColor");
        UI_DrawWrappedTextSubtitled(
            &scrPlaceView[localClientNum],
            text,
            rect,
            font,
            x,
            y,
            item->textscale,
            color,
            item->textStyle,
            item->textAlignMode,
            &item->textRect[localClientNum],
            subtitleGlowColor,
            cinematic);
    }
    else
    {
        UI_DrawWrappedText(
            &scrPlaceView[localClientNum],
            text,
            rect,
            font,
            x,
            y,
            item->textscale,
            color,
            item->textStyle,
            item->textAlignMode,
            &item->textRect[localClientNum]);
    }
}

void __cdecl Item_TextField_Paint(UiContext *dc, itemDef_s *item)
{
    char *VariantString; // eax
    int maxPaintChars; // [esp+24h] [ebp-478h]
    int v4; // [esp+28h] [ebp-474h]
    float t; // [esp+2Ch] [ebp-470h]
    float v6; // [esp+30h] [ebp-46Ch]
    float v7; // [esp+3Ch] [ebp-460h]
    char cursor; // [esp+4Bh] [ebp-451h]
    editFieldDef_s *editPtr; // [esp+4Ch] [ebp-450h]
    float lowLight[4]; // [esp+50h] [ebp-44Ch] BYREF
    Font_s *font; // [esp+60h] [ebp-43Ch]
    int offset; // [esp+64h] [ebp-438h]
    menuDef_t *parent; // [esp+68h] [ebp-434h]
    char buff[1028]; // [esp+6Ch] [ebp-430h] BYREF
    int maxChars; // [esp+474h] [ebp-28h]
    float x; // [esp+478h] [ebp-24h]
    const rectDef_s *rect; // [esp+47Ch] [ebp-20h]
    const dvar_s *dvar; // [esp+480h] [ebp-1Ch]
    const char *text; // [esp+484h] [ebp-18h]
    const rectDef_s *textRect; // [esp+488h] [ebp-14h]
    float newColor[4]; // [esp+48Ch] [ebp-10h] BYREF

    PROF_SCOPED("Item_TextField_Paint");

    parent = item->parent;
    editPtr = Item_GetEditFieldDef(item);
    if (editPtr)
    {
        Item_Text_Paint(dc, item);
        buff[0] = 0;
        if (item->dvar)
        {
            if (item->type == 17)
            {
                dvar = Dvar_FindVar(item->dvar);
                if (dvar && dvar->type == 1)
                    Com_LocalizedFloatToString(dvar->current.value, buff, 0x400u, 2u);
            }
            else
            {
                VariantString = (char *)Dvar_GetVariantString(item->dvar);
                I_strncpyz(buff, VariantString, 1024);
            }
        }
        Item_SetTextExtents(dc->localClientNum, item, buff);
        parent = item->parent;
        if (Window_HasFocus(dc->localClientNum, &item->window))
        {
            lowLight[0] = parent->focusColor[0] * 0.800000011920929;
            lowLight[1] = parent->focusColor[1] * 0.800000011920929;
            lowLight[2] = parent->focusColor[2] * 0.800000011920929;
            lowLight[3] = parent->focusColor[3] * 0.800000011920929;
            v7 = (float)(dc->realTime / 75);
            v6 = sin(v7);
            t = v6 * 0.5 + 0.5;
            LerpColor(parent->focusColor, lowLight, newColor, t);
        }
        else
        {
            newColor[0] = item->window.foreColor[0];
            newColor[1] = item->window.foreColor[1];
            newColor[2] = item->window.foreColor[2];
            newColor[3] = item->window.foreColor[3];
        }
        if (item->text && *item->text)
            v4 = 8;
        else
            v4 = 0;
        offset = v4;
        textRect = Item_GetTextRect(dc->localClientNum, item);
        x = textRect->x + textRect->w + (double)offset;
        text = &buff[editPtr->paintOffset];
        if (editPtr->maxPaintChars)
            maxPaintChars = editPtr->maxPaintChars;
        else
            maxPaintChars = 0x7FFFFFFF;
        maxChars = maxPaintChars;
        font = UI_GetFontHandle(&scrPlaceView[dc->localClientNum], item->fontEnum, item->textscale);
        if (item == g_editItem && g_editingField)
        {
            cursor = Key_GetOverstrikeMode(dc->localClientNum) != 0 ? 95 : 124;
            if (!item)
                MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
            rect = &item->window.rect;
            UI_DrawTextWithCursor(
                &scrPlaceView[dc->localClientNum],
                (char *)text,
                maxChars,
                font,
                x,
                textRect->y,
                item->window.rect.horzAlign,
                item->window.rect.vertAlign,
                item->textscale,
                newColor,
                item->textStyle,
                item->cursorPos[dc->localClientNum] - editPtr->paintOffset,
                cursor);
        }
        else
        {
            UI_DrawText(
                &scrPlaceView[dc->localClientNum],
                (char *)text,
                maxChars,
                font,
                x,
                textRect->y,
                textRect->horzAlign,
                textRect->vertAlign,
                item->textscale,
                newColor,
                item->textStyle);
        }
    }
}

void __cdecl Item_YesNo_Paint(UiContext *dc, itemDef_s *item)
{
    const char *VariantString; // eax
    char *v3; // eax
    float v4; // [esp+0h] [ebp-8h]

    PROF_SCOPED("Item_YesNo_Paint");

    if (item->dvar)
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        v4 = atof(VariantString);
    }
    else
    {
        v4 = 0.0;
    }
    if (v4 == 0.0)
        v3 = UI_SafeTranslateString("EXE_NO");
    else
        v3 = UI_SafeTranslateString("EXE_YES");
    item->text = v3;
    Item_Text_Paint(dc, item);
}

void __cdecl Item_Multi_Paint(UiContext *dc, itemDef_s *item)
{
    PROF_SCOPED("Item_Multi_Paint");
    item->text = Item_Multi_Setting(item);
    Item_Text_Paint(dc, item);
}

const char *__cdecl Item_Multi_Setting(itemDef_s *item)
{
    const char *VariantString; // eax
    multiDef_s *multiPtr; // [esp+0h] [ebp-10h]
    const char *string; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int ia; // [esp+8h] [ebp-8h]
    float value; // [esp+Ch] [ebp-4h]

    multiPtr = Item_GetMultiDef(item);
    if (!multiPtr)
        return "<dvarStrList or dvarFloatList not set>";
    if (multiPtr->strDef)
    {
        string = Dvar_GetVariantString(item->dvar);
        for (i = 0; i < multiPtr->count; ++i)
        {
            if (!I_stricmp(string, multiPtr->dvarStr[i]))
                return multiPtr->dvarList[i];
        }
    }
    else
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        value = atof(VariantString);
        for (ia = 0; ia < multiPtr->count; ++ia)
        {
            if (value == multiPtr->dvarValue[ia])
                return multiPtr->dvarList[ia];
        }
    }
    return "";
}

void __cdecl Item_DvarEnum_Paint(UiContext *dc, itemDef_s *item)
{
    PROF_SCOPED("Item_DvarEnum_Paint");
    item->text = Item_DvarEnum_Setting(item);
    Item_Text_Paint(dc, item);
}

const char *__cdecl Item_DvarEnum_Setting(itemDef_s *item)
{
    const char *v2; // eax
    int enumIndex; // [esp+0h] [ebp-8h]
    const dvar_s *enumDvar; // [esp+4h] [ebp-4h]

    if (!item)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3159, 0, "%s", "item");
    if (item->type != 13)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 3160, 0, "%s\n\t(item->type) = %i", "(item->type == 13)", item->type);
    if (!item->typeData.listBox)
        return "<dvarEnumList not set>";
    enumDvar = Dvar_FindVar(item->typeData.enumDvarName);
    if (enumDvar->type != 6)
        return "<not an enum dvar>";
    if (!enumDvar->domain.enumeration.stringCount)
        return "";
    enumIndex = Item_DvarEnum_EnumIndex(item);
    if (enumIndex < 0 || enumIndex >= enumDvar->domain.enumeration.stringCount)
    {
        v2 = va("%s %i:%i", item->typeData.enumDvarName, enumDvar->domain.enumeration.stringCount, enumIndex);
        MyAssertHandler(
            ".\\ui\\ui_shared.cpp",
            3172,
            0,
            "%s\n\t%s",
            "enumIndex >= 0 && enumIndex < enumDvar->domain.enumeration.stringCount",
            v2);
    }
    return *(const char **)(enumDvar->domain.integer.max + 4 * enumIndex);
}

void __cdecl Item_Slider_Paint(UiContext *dc, itemDef_s *item)
{
    const char *VariantString; // eax
    float v3; // [esp+20h] [ebp-64h]
    float v4; // [esp+24h] [ebp-60h]
    float v5; // [esp+28h] [ebp-5Ch]
    float x0; // [esp+2Ch] [ebp-58h]
    float t; // [esp+30h] [ebp-54h]
    float v8; // [esp+34h] [ebp-50h]
    float v9; // [esp+3Ch] [ebp-48h]
    float v10; // [esp+40h] [ebp-44h]
    float lowLight[4]; // [esp+4Ch] [ebp-38h] BYREF
    const ScreenPlacement *scrPlace; // [esp+5Ch] [ebp-28h]
    menuDef_t *parent; // [esp+60h] [ebp-24h]
    float x; // [esp+64h] [ebp-20h]
    float y; // [esp+68h] [ebp-1Ch]
    const rectDef_s *rect; // [esp+6Ch] [ebp-18h]
    float value; // [esp+70h] [ebp-14h]
    float newColor[4]; // [esp+74h] [ebp-10h] BYREF

    PROF_SCOPED("Item_Slider_Paint");

    parent = item->parent;
    if (item->dvar)
    {
        VariantString = Dvar_GetVariantString(item->dvar);
        v9 = atof(VariantString);
    }
    else
    {
        v9 = 0.0;
    }
    value = v9;
    if (Window_HasFocus(dc->localClientNum, &item->window))
    {
        lowLight[0] = parent->focusColor[0] * 0.800000011920929;
        lowLight[1] = parent->focusColor[1] * 0.800000011920929;
        lowLight[2] = parent->focusColor[2] * 0.800000011920929;
        lowLight[3] = parent->focusColor[3] * 0.800000011920929;
        v10 = (float)(dc->realTime / 75);
        v8 = sin(v10);
        t = v8 * 0.5 + 0.5;
        LerpColor(parent->focusColor, lowLight, newColor, t);
    }
    else
    {
        newColor[0] = item->window.foreColor[0];
        newColor[1] = item->window.foreColor[1];
        newColor[2] = item->window.foreColor[2];
        newColor[3] = item->window.foreColor[3];
    }
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    rect = &item->window.rect;
    x0 = item->window.rect.x + item->textalignx;
    x = Item_GetRectPlacementX(item->textAlignMode & 3, x0, item->window.rect.w, 96.0);
    v5 = rect->y + item->textaligny;
    y = Item_GetRectPlacementY(item->textAlignMode & 0xC, v5, rect->h, 16.0);
    scrPlace = &scrPlaceView[dc->localClientNum];
    UI_DrawHandlePic(
        scrPlace,
        x,
        y,
        96.0,
        16.0,
        rect->horzAlign,
        rect->vertAlign,
        newColor,
        sharedUiInfo.assets.sliderBar);
    x = Item_Slider_ThumbPosition(dc->localClientNum, item);
    v4 = y - 2.0;
    v3 = x - 5.0;
    UI_DrawHandlePic(
        scrPlace,
        v3,
        v4,
        10.0,
        20.0,
        rect->horzAlign,
        rect->vertAlign,
        newColor,
        sharedUiInfo.assets.sliderThumb);
}

double __cdecl Item_GetRectPlacementY(int alignY, float y0, float containerHeight, float selfHeight)
{
    if (alignY == 12)
    {
        return (float)(containerHeight - selfHeight + y0);
    }
    else if (alignY == 8)
    {
        return (float)((containerHeight - selfHeight) * 0.5 + y0);
    }
    else
    {
        if (alignY != 4)
        {
            if (alignY)
                MyAssertHandler(".\\ui\\ui_shared.cpp", 202, 0, "%s\n\t(alignY) = %i", "(alignY == 4 || alignY == 0)", alignY);
        }
        return y0;
    }
}

void __cdecl Item_Bind_Paint(UiContext *dc, itemDef_s *item)
{
    PROF_SCOPED("Item_Bind_Paint");
    char nameBind[260]; // [esp+0h] [ebp-108h] BYREF

    if (g_waitingForKey && g_bindItem == item)
    {
        item->text = "@MENU_BIND_KEY_PENDING";
    }
    else
    {
        UI_GetKeyBindingLocalizedString(dc->localClientNum, item->dvar, nameBind);
        item->text = nameBind;
    }
    Item_Text_Paint(dc, item);
    item->text = "";
}

void __cdecl Item_ListBox_Paint(UiContext *dc, itemDef_s *item)
{
    float width; // [esp+18h] [ebp-78h]
    float v3; // [esp+24h] [ebp-6Ch]
    float v4; // [esp+28h] [ebp-68h]
    float v5; // [esp+2Ch] [ebp-64h]
    float v6; // [esp+30h] [ebp-60h]
    float v7; // [esp+34h] [ebp-5Ch]
    float v8; // [esp+38h] [ebp-58h]
    float v9; // [esp+3Ch] [ebp-54h]
    float v10; // [esp+40h] [ebp-50h]
    float v11; // [esp+44h] [ebp-4Ch]
    float v12; // [esp+48h] [ebp-48h]
    float v13; // [esp+4Ch] [ebp-44h]
    float v14; // [esp+50h] [ebp-40h]
    float v15; // [esp+54h] [ebp-3Ch]
    float h; // [esp+58h] [ebp-38h]
    float w; // [esp+5Ch] [ebp-34h]
    Material *image; // [esp+64h] [ebp-2Ch]
    Material *imagea; // [esp+64h] [ebp-2Ch]
    int j; // [esp+68h] [ebp-28h]
    ScreenPlacement *scrPlace; // [esp+6Ch] [ebp-24h]
    float thumb; // [esp+70h] [ebp-20h]
    float thumba; // [esp+70h] [ebp-20h]
    listBoxDef_s *listPtr; // [esp+74h] [ebp-1Ch]
    float sizeb; // [esp+78h] [ebp-18h]
    float size; // [esp+78h] [ebp-18h]
    float sizec; // [esp+78h] [ebp-18h]
    float sizea; // [esp+78h] [ebp-18h]
    int i; // [esp+7Ch] [ebp-14h]
    int ia; // [esp+7Ch] [ebp-14h]
    int ib; // [esp+7Ch] [ebp-14h]
    rectDef_s *rect; // [esp+80h] [ebp-10h]
    float xe; // [esp+84h] [ebp-Ch]
    float xf; // [esp+84h] [ebp-Ch]
    float x; // [esp+84h] [ebp-Ch]
    float xa; // [esp+84h] [ebp-Ch]
    float xb; // [esp+84h] [ebp-Ch]
    float xc; // [esp+84h] [ebp-Ch]
    float xd; // [esp+84h] [ebp-Ch]
    float y; // [esp+88h] [ebp-8h]
    float ya; // [esp+88h] [ebp-8h]
    float ye; // [esp+88h] [ebp-8h]
    float yf; // [esp+88h] [ebp-8h]
    float yb; // [esp+88h] [ebp-8h]
    float yc; // [esp+88h] [ebp-8h]
    float yd; // [esp+88h] [ebp-8h]
    int count; // [esp+8Ch] [ebp-4h]

    PROF_SCOPED("Item_ListBox_Paint");

    listPtr = Item_GetListBoxDef(item);
    if (listPtr)
    {
        if (!item)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
        rect = &item->window.rect;
        scrPlace = &scrPlaceView[dc->localClientNum];
        count = UI_FeederCount(dc->localClientNum, item->special);
        if (!item)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 53, 0, "%s", "w");
        if ((item->window.staticFlags & 0x200000) != 0)
        {
            xe = rect->x + 1.0;
            y = item->window.rect.y + item->window.rect.h - 16.0 - 1.0;
            UI_DrawHandlePic(
                scrPlace,
                xe,
                y,
                16.0,
                16.0,
                item->window.rect.horzAlign,
                item->window.rect.vertAlign,
                0,
                sharedUiInfo.assets.scrollBarArrowLeft);
            xf = xe + 15.0;
            sizeb = item->window.rect.w - 32.0;
            w = sizeb + 1.0;
            UI_DrawHandlePic(
                scrPlace,
                xf,
                y,
                w,
                16.0,
                item->window.rect.horzAlign,
                item->window.rect.vertAlign,
                0,
                sharedUiInfo.assets.scrollBar);
            x = sizeb - 1.0 + xf;
            UI_DrawHandlePic(
                scrPlace,
                x,
                y,
                16.0,
                16.0,
                item->window.rect.horzAlign,
                item->window.rect.vertAlign,
                0,
                sharedUiInfo.assets.scrollBarArrowRight);
            thumb = Item_ListBox_ThumbDrawPosition(dc, item);
            if (thumb > x - 16.0 - 1.0)
                thumb = x - 16.0 - 1.0;
            UI_DrawHandlePic(
                scrPlace,
                thumb,
                y,
                16.0,
                16.0,
                item->window.rect.horzAlign,
                item->window.rect.vertAlign,
                0,
                sharedUiInfo.assets.scrollBarThumb);
            listPtr->endPos[dc->localClientNum] = listPtr->startPos[dc->localClientNum];
            size = item->window.rect.w - 2.0;
            if (listPtr->elementStyle == 1)
            {
                xa = rect->x + 1.0;
                ya = item->window.rect.y + 1.0;
                for (i = listPtr->startPos[dc->localClientNum]; i < count; ++i)
                {
                    image = UI_FeederItemImage(item->special, i);
                    if (image)
                    {
                        h = listPtr->elementHeight - 2.0;
                        v15 = listPtr->elementWidth - 2.0;
                        v14 = ya + 1.0;
                        v13 = xa + 1.0;
                        UI_DrawHandlePic(
                            scrPlace,
                            v13,
                            v14,
                            v15,
                            h,
                            item->window.rect.horzAlign,
                            item->window.rect.vertAlign,
                            0,
                            image);
                    }
                    if (i == item->cursorPos[dc->localClientNum])
                    {
                        v12 = listPtr->elementHeight - 1.0;
                        v11 = listPtr->elementWidth - 1.0;
                        UI_DrawRect(
                            scrPlace,
                            xa,
                            ya,
                            v11,
                            v12,
                            item->window.rect.horzAlign,
                            item->window.rect.vertAlign,
                            item->window.borderSize,
                            item->window.borderColor);
                    }
                    size = size - listPtr->elementWidth;
                    if (listPtr->elementWidth > (double)size)
                    {
                        listPtr->drawPadding = SnapFloatToInt(size);
                        return;
                    }
                    xa = xa + listPtr->elementWidth;
                    ++listPtr->endPos[dc->localClientNum];
                }
            }
        }
        else
        {
            UI_OverrideCursorPos(dc->localClientNum, item);
            if (!listPtr->noScrollBars)
            {
                xb = item->window.rect.x + item->window.rect.w - 16.0 - 1.0;
                ye = item->window.rect.y + 1.0;
                UI_DrawHandlePic(
                    scrPlace,
                    xb,
                    ye,
                    16.0,
                    16.0,
                    item->window.rect.horzAlign,
                    item->window.rect.vertAlign,
                    0,
                    sharedUiInfo.assets.scrollBarArrowUp);
                yf = ye + 15.0;
                listPtr->endPos[dc->localClientNum] = listPtr->startPos[dc->localClientNum];
                sizec = item->window.rect.h - 32.0;
                v10 = sizec + 1.0;
                UI_DrawHandlePic(
                    scrPlace,
                    xb,
                    yf,
                    16.0,
                    v10,
                    item->window.rect.horzAlign,
                    item->window.rect.vertAlign,
                    0,
                    sharedUiInfo.assets.scrollBar);
                yb = sizec - 1.0 + yf;
                UI_DrawHandlePic(
                    scrPlace,
                    xb,
                    yb,
                    16.0,
                    16.0,
                    item->window.rect.horzAlign,
                    item->window.rect.vertAlign,
                    0,
                    sharedUiInfo.assets.scrollBarArrowDown);
                thumba = Item_ListBox_ThumbDrawPosition(dc, item);
                if (thumba > yb - 16.0 - 1.0)
                    thumba = yb - 16.0 - 1.0;
                UI_DrawHandlePic(
                    scrPlace,
                    xb,
                    thumba,
                    16.0,
                    16.0,
                    item->window.rect.horzAlign,
                    item->window.rect.vertAlign,
                    0,
                    sharedUiInfo.assets.scrollBarThumb);
            }
            sizea = item->window.rect.h - 2.0;
            if (listPtr->elementStyle == 1)
            {
                xc = rect->x + 1.0;
                yc = item->window.rect.y + 1.0;
                listPtr->endPos[dc->localClientNum] = listPtr->startPos[dc->localClientNum];
                for (ia = listPtr->startPos[dc->localClientNum]; ia < count; ++ia)
                {
                    imagea = UI_FeederItemImage(item->special, ia);
                    if (imagea)
                    {
                        v9 = listPtr->elementHeight - 2.0;
                        v8 = listPtr->elementWidth - 2.0;
                        v7 = yc + 1.0;
                        v6 = xc + 1.0;
                        UI_DrawHandlePic(
                            scrPlace,
                            v6,
                            v7,
                            v8,
                            v9,
                            item->window.rect.horzAlign,
                            item->window.rect.vertAlign,
                            0,
                            imagea);
                    }
                    if (ia == item->cursorPos[dc->localClientNum])
                    {
                        v5 = listPtr->elementHeight - 1.0;
                        v4 = listPtr->elementWidth - 1.0;
                        UI_DrawRect(
                            scrPlace,
                            xc,
                            yc,
                            v4,
                            v5,
                            item->window.rect.horzAlign,
                            item->window.rect.vertAlign,
                            item->window.borderSize,
                            item->window.borderColor);
                    }
                    ++listPtr->endPos[dc->localClientNum];
                    sizea = sizea - listPtr->elementHeight;
                    if (listPtr->elementHeight > (double)sizea)
                    {
                        listPtr->drawPadding = SnapFloatToInt(listPtr->elementHeight - sizea);
                        return;
                    }
                    yc = yc + listPtr->elementHeight;
                }
            }
            else
            {
                xd = rect->x + 1.0;
                yd = item->window.rect.y + 1.0;
                listPtr->endPos[dc->localClientNum] = listPtr->startPos[dc->localClientNum];
                for (ib = listPtr->startPos[dc->localClientNum]; ib < count; ++ib)
                {
                    Item_ListBox_PaintBackground(dc->localClientNum, item, xd, yd);
                    if (ib == item->cursorPos[dc->localClientNum])
                        Item_ListBox_PaintHighlight(dc->localClientNum, item, xd, yd);
                    if (listPtr->numColumns <= 0)
                    {
                        Item_ListBox_PaintTextElem(
                            dc->localClientNum,
                            item,
                            ib,
                            0,
                            0x7FFFFFFF,
                            0,
                            xd,
                            yd,
                            listPtr->elementWidth,
                            listPtr->elementHeight);
                    }
                    else
                    {
                        for (j = 0; j < listPtr->numColumns; ++j)
                        {
                            width = (float)listPtr->columnInfo[j].width;
                            v3 = (double)listPtr->columnInfo[j].pos + xd;
                            Item_ListBox_PaintTextElem(
                                dc->localClientNum,
                                item,
                                ib,
                                j,
                                listPtr->columnInfo[j].maxChars,
                                listPtr->columnInfo[j].alignment,
                                v3,
                                yd,
                                width,
                                listPtr->elementHeight);
                        }
                    }
                    sizea = sizea - listPtr->elementHeight;
                    if (listPtr->elementHeight > (double)sizea)
                    {
                        listPtr->drawPadding = SnapFloatToInt(listPtr->elementHeight - sizea);
                        return;
                    }
                    ++listPtr->endPos[dc->localClientNum];
                    yd = yd + listPtr->elementHeight;
                }
            }
        }
    }
}

double __cdecl Item_ListBox_ThumbDrawPosition(UiContext *dc, itemDef_s *item)
{
    float max; // [esp+Ch] [ebp-Ch]
    float maxa; // [esp+Ch] [ebp-Ch]
    float min; // [esp+10h] [ebp-8h]
    float mina; // [esp+10h] [ebp-8h]

    if (itemCapture == item)
    {
        if (!item)
        {
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h", 53, 0, "%s", "w");
        }
        if ((item->window.staticFlags & 0x200000) != 0)
        {
            min = item->window.rect.x + 16.0 + 1.0;
            if (dc->cursor.x >= min + 8.0)
            {
                max = item->window.rect.x + item->window.rect.w - 32.0 - 1.0;
                if (dc->cursor.x <= max + 8.0)
                    return (float)(dc->cursor.x - 8.0);
            }
        }
        else
        {
            mina = item->window.rect.y + 16.0 + 1.0;
            if (dc->cursor.y >= mina + 8.0)
            {
                maxa = item->window.rect.y + item->window.rect.h - 32.0 - 1.0;
                if (dc->cursor.y <= maxa + 8.0)
                    return (float)(dc->cursor.y - 8.0);
            }
        }
    }
    return Item_ListBox_ThumbPosition(dc->localClientNum, item);
}

void __cdecl Item_ListBox_PaintTextElem(
    int localClientNum,
    itemDef_s *item,
    int row,
    int col,
    int maxChars,
    int alignment,
    float x,
    float y,
    float w,
    float h)
{
    float scale; // [esp+1Ch] [ebp-48h]
    float v11; // [esp+20h] [ebp-44h]
    float v12; // [esp+24h] [ebp-40h]
    float imageColor[4]; // [esp+2Ch] [ebp-38h] BYREF
    Font_s *font; // [esp+3Ch] [ebp-28h]
    const ScreenPlacement *scrPlace; // [esp+40h] [ebp-24h]
    Material *optionalImage; // [esp+44h] [ebp-20h] BYREF
    float xadj; // [esp+48h] [ebp-1Ch]
    const rectDef_s *rect; // [esp+4Ch] [ebp-18h]
    float color[4]; // [esp+50h] [ebp-14h] BYREF
    const char *text; // [esp+60h] [ebp-4h]

    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    rect = &item->window.rect;
    scrPlace = &scrPlaceView[localClientNum];
    text = UI_FeederItemText(localClientNum, item, item->special, row, col, &optionalImage);
    UI_FeederItemColor(localClientNum, item, item->special, row, col, color);
    if (optionalImage)
    {
        imageColor[0] = 1.0;
        imageColor[1] = 1.0;
        imageColor[2] = 1.0;
        imageColor[3] = color[3];
        UI_DrawHandlePic(scrPlace, x, y, w, h, rect->horzAlign, rect->vertAlign, imageColor, optionalImage);
    }
    else if (text)
    {
        font = UI_GetFontHandle(&scrPlaceView[localClientNum], item->fontEnum, item->textscale);
        scale = (float)UI_TextWidth(text, maxChars, font, item->textscale);
        xadj = Item_GetTextAlignAdj(alignment, w, scale);
        v12 = y + h + item->textaligny;
        v11 = x + xadj + item->textalignx;
        UI_DrawText(
            scrPlace,
            (char *)text,
            maxChars,
            font,
            v11,
            v12,
            rect->horzAlign,
            rect->vertAlign,
            item->textscale,
            color,
            item->textStyle);
    }
}

double __cdecl Item_GetTextAlignAdj(int alignment, float width, float textWidth)
{
    float v5; // [esp+4h] [ebp-18h]
    float v7; // [esp+Ch] [ebp-10h]
    float v8; // [esp+14h] [ebp-8h]
    float v9; // [esp+18h] [ebp-4h]

    if (alignment == 1)
    {
        v9 = (width - textWidth) * 0.5;
        v7 = 0.0 - v9;
        if (v7 < 0.0)
            return (float)((width - textWidth) * 0.5);
        else
            return (float)0.0;
    }
    else if (alignment == 2)
    {
        v8 = width - textWidth;
        v5 = 0.0 - v8;
        if (v5 < 0.0)
            return (float)(width - textWidth);
        else
            return (float)0.0;
    }
    else
    {
        if (alignment)
            MyAssertHandler(".\\ui\\ui_shared.cpp", 5243, 0, "%s\n\t(alignment) = %i", "(alignment == 0)", alignment);
        return 0.0;
    }
}

void __cdecl Item_ListBox_PaintBackground(int localClientNum, itemDef_s *item, float x, float y)
{
    float v4; // [esp+1Ch] [ebp-18h]
    float v5; // [esp+20h] [ebp-14h]
    float width; // [esp+24h] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+2Ch] [ebp-8h]

    if (!item->window.style)
    {
        listPtr = Item_GetListBoxDef(item);
        if (!listPtr)
            MyAssertHandler(".\\ui\\ui_shared.cpp", 5301, 0, "%s", "listPtr");
        if (!item)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
        width = item->window.rect.w - 16.0 - 4.0;
        v5 = y + 2.0;
        v4 = x + 2.0;
        UI_FillRect(
            &scrPlaceView[localClientNum],
            v4,
            v5,
            width,
            listPtr->elementHeight,
            item->window.rect.horzAlign,
            item->window.rect.vertAlign,
            item->window.backColor);
    }
}

void __cdecl Item_ListBox_PaintHighlight(int localClientNum, itemDef_s *item, float x, float y)
{
    float v4; // [esp+20h] [ebp-3Ch]
    float v5; // [esp+24h] [ebp-38h]
    float w; // [esp+2Ch] [ebp-30h]
    float v7; // [esp+30h] [ebp-2Ch]
    float v8; // [esp+34h] [ebp-28h]
    float v9; // [esp+38h] [ebp-24h]
    float v10; // [esp+3Ch] [ebp-20h]
    float v11; // [esp+40h] [ebp-1Ch]
    float v12; // [esp+44h] [ebp-18h]
    float width; // [esp+48h] [ebp-14h]
    ScreenPlacement *scrPlace; // [esp+4Ch] [ebp-10h]
    listBoxDef_s *listPtr; // [esp+50h] [ebp-Ch]
    Material *optionalImage; // [esp+54h] [ebp-8h]

    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 5318, 0, "%s", "listPtr");
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    scrPlace = &scrPlaceView[localClientNum];
    width = item->window.rect.w - 16.0 - 4.0;
    v12 = y + 2.0;
    v11 = x + 2.0;
    UI_FillRect(
        scrPlace,
        v11,
        v12,
        width,
        listPtr->elementHeight,
        item->window.rect.horzAlign,
        item->window.rect.vertAlign,
        item->window.outlineColor);
    v10 = listPtr->elementHeight + 1.0;
    v9 = item->window.rect.w - 16.0 - 3.0;
    v8 = y + 1.0;
    v7 = x + 1.0;
    UI_DrawRect(
        scrPlace,
        v7,
        v8,
        v9,
        v10,
        item->window.rect.horzAlign,
        item->window.rect.vertAlign,
        1.0,
        listPtr->selectBorder);
    optionalImage = listPtr->selectIcon;
    if (optionalImage)
    {
        if (!listPtr->notselectable)
        {
            w = listPtr->elementHeight - 4.0;
            v5 = y + 4.0;
            v4 = x + 4.0;
            UI_DrawHandlePic(
                scrPlace,
                v4,
                v5,
                w,
                w,
                item->window.rect.horzAlign,
                item->window.rect.vertAlign,
                0,
                optionalImage);
        }
    }
}

void __cdecl Item_OwnerDraw_Paint(UiContext *dc, itemDef_s *item)
{
    rectDef_s clamp; // [esp+44h] [ebp-90h]
    float v3; // [esp+60h] [ebp-74h]
    float x; // [esp+64h] [ebp-70h]
    float v5; // [esp+68h] [ebp-6Ch]
    float v6; // [esp+6Ch] [ebp-68h]
    float t; // [esp+74h] [ebp-60h]
    float v8; // [esp+78h] [ebp-5Ch]
    float v9; // [esp+84h] [ebp-50h]
    float v10; // [esp+88h] [ebp-4Ch]
    int localClientNum; // [esp+9Ch] [ebp-38h]
    float lowLight[4]; // [esp+A0h] [ebp-34h] BYREF
    Font_s *font; // [esp+B0h] [ebp-24h]
    menuDef_t *parent; // [esp+B4h] [ebp-20h]
    float color[4]; // [esp+B8h] [ebp-1Ch] BYREF
    int flags; // [esp+C8h] [ebp-Ch] BYREF
    const rectDef_s *rect; // [esp+CCh] [ebp-8h]
    const rectDef_s *textRect; // [esp+D0h] [ebp-4h]

    PROF_SCOPED("Item_OwnerDraw_Paint");

    if (!item)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 5521, 0, "%s", "item");
    parent = item->parent;
    localClientNum = dc->localClientNum;
    if (dc->localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../ui/ui_utils.h",
            23,
            0,
            "localClientNum doesn't index MAX_POSSIBLE_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    flags = item->window.dynamicFlags[localClientNum];
    Fade(
        &flags,
        &item->window.foreColor[3],
        parent->fadeClamp,
        &item->window.nextTime,
        parent->fadeCycle,
        1,
        parent->fadeAmount,
        parent->fadeInAmount,
        dc);
    Window_SetDynamicFlags(dc->localClientNum, &item->window, flags);
    color[0] = item->window.foreColor[0];
    color[1] = item->window.foreColor[1];
    color[2] = item->window.foreColor[2];
    color[3] = item->window.foreColor[3];
    if (Window_HasFocus(dc->localClientNum, &item->window))
    {
        lowLight[0] = parent->focusColor[0] * 0.800000011920929;
        lowLight[1] = parent->focusColor[1] * 0.800000011920929;
        lowLight[2] = parent->focusColor[2] * 0.800000011920929;
        lowLight[3] = parent->focusColor[3] * 0.800000011920929;
        v10 = (float)(dc->realTime / 75);
        v8 = sin(v10);
        t = v8 * 0.5 + 0.5;
        LerpColor(parent->focusColor, lowLight, color, t);
    }
    else if (item->textStyle == 1 && ((dc->realTime / 256) & 1) == 0)
    {
        lowLight[0] = item->window.foreColor[0] * 0.800000011920929;
        lowLight[1] = item->window.foreColor[1] * 0.800000011920929;
        lowLight[2] = item->window.foreColor[2] * 0.800000011920929;
        lowLight[3] = item->window.foreColor[3] * 0.800000011920929;
        v9 = (float)(dc->realTime / 75);
        v6 = sin(v9);
        v5 = v6 * 0.5 + 0.5;
        LerpColor(item->window.foreColor, lowLight, color, v5);
    }
    if ((item->dvarFlags & 3) != 0 && !Item_EnableShowViaDvar(item, 1))
    {
        color[0] = parent->disableColor[0];
        color[1] = parent->disableColor[1];
        color[2] = parent->disableColor[2];
        color[3] = parent->disableColor[3];
    }
    if (!item)
        MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
    rect = &item->window.rect;
    font = UI_GetFontHandle(&scrPlaceView[dc->localClientNum], item->fontEnum, item->textscale);
    if (item->text)
    {
        Item_Text_Paint(dc, item);
        textRect = Item_GetTextRect(dc->localClientNum, item);
        clamp = item->parent->window.rect;
        if (*item->text)
        {
            x = textRect->x + textRect->w + 8.0;
            UI_OwnerDraw(
                dc->localClientNum,
                x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                item->textaligny,
                item->window.ownerDraw,
                item->window.ownerDrawFlags,
                item->alignment,
                item->special,
                font,
                item->textscale,
                color,
                item->window.background,
                item->textStyle,
                clamp,
                item->textAlignMode);
        }
        else
        {
            v3 = textRect->x + textRect->w;
            UI_OwnerDraw(
                dc->localClientNum,
                v3,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                0.0,
                item->textaligny,
                item->window.ownerDraw,
                item->window.ownerDrawFlags,
                item->alignment,
                item->special,
                font,
                item->textscale,
                color,
                item->window.background,
                item->textStyle,
                clamp,
                item->textAlignMode);
        }
    }
    else
    {
        UI_OwnerDraw(
            dc->localClientNum,
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            item->textalignx,
            item->textaligny,
            item->window.ownerDraw,
            item->window.ownerDrawFlags,
            item->alignment,
            item->special,
            font,
            item->textscale,
            color,
            item->window.background,
            item->textStyle,
            item->parent->window.rect,
            item->textAlignMode);
    }
}

void __cdecl Item_GameMsgWindow_Paint(UiContext *dc, itemDef_s *item)
{
    const char *v2; // eax
    const char *v3; // eax
    Font_s *font; // [esp+1Ch] [ebp-18h]
    float color[5]; // [esp+20h] [ebp-14h] BYREF

    PROF_SCOPED("Item_GameMsgWindow_Paint");

    if (!dc)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 5580, 0, "%s", "dc");
    if (!item)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 5581, 0, "%s", "item");
    if (CL_IsCgameInitialized(dc->localClientNum))
    {
        if (item->gameMsgWindowIndex >= 4u)
        {
            v2 = va("Game message window index %d is not valid. Must be in [0, %d).", item->gameMsgWindowIndex, 4);
            Com_Error(ERR_DROP, v2);
        }
        if (item->gameMsgWindowMode >= 2u && item->gameMsgWindowMode != 3 && item->gameMsgWindowMode != 2)
        {
            v3 = va("Game message window mode %d is not valid.", item->gameMsgWindowMode);
            Com_Error(ERR_DROP, v3);
        }
        color[0] = item->window.foreColor[0];
        color[1] = item->window.foreColor[1];
        color[2] = item->window.foreColor[2];
        color[3] = item->window.foreColor[3];
        if (!item)
            MyAssertHandler("c:\\trees\\cod3\\src\\ui\\ui_utils_api.h", 36, 0, "%s", "w");
        //LODWORD(color[4]) = &item->window.rect;
        font = UI_GetFontHandle(&scrPlaceView[dc->localClientNum], item->fontEnum, item->textscale);
        Con_DrawGameMessageWindow(
            dc->localClientNum,
            item->gameMsgWindowIndex,
            SnapFloatToInt(item->window.rect.x),
            SnapFloatToInt(item->window.rect.y),
            item->window.rect.horzAlign,
            item->window.rect.vertAlign,
            font,
            item->textscale,
            color,
            item->textStyle,
            item->textAlignMode,
            (msgwnd_mode_t)item->gameMsgWindowMode);
    }
}

int __cdecl Menu_Count(UiContext *dc)
{
    return dc->menuCount;
}

void __cdecl Menu_PaintAll_BeginVisibleList(char *stringBegin, unsigned int stringSize)
{
    PROF_SCOPED("Menu_PaintAll_BeginVisibleList");

    char VISIBLE_LIST_PREFIX[14]; // [esp+0h] [ebp-14h] BYREF

    strcpy(VISIBLE_LIST_PREFIX, "ui_showlist: ");
    if (stringSize < 0xE)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 6048, 0, "%s", "stringSize >= sizeof( VISIBLE_LIST_PREFIX )");
    *(unsigned int *)stringBegin = *(unsigned int *)VISIBLE_LIST_PREFIX;
    *((unsigned int *)stringBegin + 1) = *(unsigned int *)&VISIBLE_LIST_PREFIX[4];
    *((unsigned int *)stringBegin + 2) = *(unsigned int *)&VISIBLE_LIST_PREFIX[8];
    *((_WORD *)stringBegin + 6) = *(_WORD *)&VISIBLE_LIST_PREFIX[12];
}

void __cdecl Menu_PaintAll_AppendToVisibleList(char *stringBegin, unsigned int stringSize, char *stringToAppend)
{
    PROF_SCOPED("Menu_PaintAll_AppendToVisibleList");

    unsigned int v3; // [esp+0h] [ebp-64h]
    std::reverse_iterator<char *> result; // [esp+44h] [ebp-20h] BYREF
    char _Val; // [esp+53h] [ebp-11h] BYREF
    const char *lastNewline; // [esp+54h] [ebp-10h]
    int VISIBLE_LIST_LINE_LENGTH; // [esp+58h] [ebp-Ch]
    char *stringEnd; // [esp+5Ch] [ebp-8h]
    const char *terminus; // [esp+60h] [ebp-4h]

    VISIBLE_LIST_LINE_LENGTH = 80;
    v3 = strlen(stringBegin);
    stringEnd = &stringBegin[v3];
    I_strncat(stringBegin, stringSize, stringToAppend);
    _Val = 10;

    std::reverse_iterator<char *> _First = std::reverse_iterator<char *>(stringBegin); // [esp+48h] [ebp-1Ch]
    std::reverse_iterator<char *> _Last = std::reverse_iterator<char *>(&stringBegin[v3]); // [esp+4Ch] [ebp-18h]

    std::string kisak(stringBegin);

    //_Last.current = stringBegin;
    //_First.current = &stringBegin[v3];
    //lastNewline = std::find<std::reverse_iterator<char *>, char>(
    //    &result,
    //    _Last,
    //    _First,
    //    //(std::reverse_iterator<char *>) & stringBegin[v3],
    //    //(std::reverse_iterator<char *>)stringBegin,
    //    &_Val)->current - 1;
    auto it = std::find<std::reverse_iterator<char *>, char>(_Last, _First, _Val); // KISAKTODO: i'd be surprised if this works.
    lastNewline = it._Get_current() - 1;

    if (stringEnd - lastNewline <= 80)
        terminus = ", ";
    else
        terminus = ",\n  ";
    I_strncat(stringBegin, stringSize, (char *)terminus);
}

void __cdecl Menu_PaintAll_DrawVisibleList(char *stringBegin, UiContext *dc)
{
    PROF_SCOPED("Menu_PaintAll_DrawVisibleList");

    float color[4]; // [esp+28h] [ebp-18h] BYREF
    float y; // [esp+38h] [ebp-8h]

    float MESSAGE_SCALE = 0.2f;
    color[0] = 0.75;
    color[1] = 1.0;
    color[2] = 0.5;
    color[3] = 1.0;
    if (dc->FPS == 0.0)
        y = 320.0;
    else
        y = 400.0;
    UI_DrawText(
        &scrPlaceFull,
        stringBegin,
        0x7FFFFFFF,
        sharedUiInfo.assets.smallFont,
        0.0,
        y,
        0,
        0,
        MESSAGE_SCALE,
        color,
        0);
}

void __cdecl Menu_PaintAll(UiContext *dc)
{
    PROF_SCOPED("Menu_PaintAll");

    char *v1; // eax
    Font_s *font; // [esp+2Ch] [ebp-41Ch]
    bool showVisibleList; // [esp+33h] [ebp-415h]
    int menuIndex; // [esp+34h] [ebp-414h]
    int menuIndexa; // [esp+34h] [ebp-414h]
    int menuIndexb; // [esp+34h] [ebp-414h]
    menuDef_t *menu; // [esp+38h] [ebp-410h]
    menuDef_t *menua; // [esp+38h] [ebp-410h]
    menuDef_t *menub; // [esp+38h] [ebp-410h]
    bool anyFullscreen; // [esp+3Eh] [ebp-40Ah]
    char visibleList[1024]; // [esp+40h] [ebp-408h] BYREF
    int drawStart; // [esp+444h] [ebp-4h]

    showVisibleList = ui_showList->current.enabled;
    KISAK_NULLSUB();
    if (showVisibleList)
        Menu_PaintAll_BeginVisibleList(visibleList, 0x400u);
    dc->blurRadiusOut = 0.0;
    if (captureFunc)
        captureFunc(dc, captureData);
    anyFullscreen = 0;
    drawStart = 0;
    for (menuIndex = dc->openMenuCount - 1; menuIndex >= 0; --menuIndex)
    {
        menu = dc->menuStack[menuIndex];
        if (!menu)
            MyAssertHandler(".\\ui\\ui_shared.cpp", 6124, 0, "%s", "menu");
        if (menu->fullScreen)
        {
            drawStart = menuIndex;
            anyFullscreen = 1;
            break;
        }
    }

    {
        PROF_SCOPED("Menu_PaintAll_PaintMenus");
        if (!anyFullscreen)
        {
            for (menuIndexa = 0; menuIndexa < Menu_Count(dc); ++menuIndexa)
            {
                menua = dc->Menus[menuIndexa];
                if (!menua)
                    MyAssertHandler(".\\ui\\ui_shared.cpp", 6139, 0, "%s", "menu");
                if (!Menus_MenuIsInStack(dc, menua) && Menu_Paint(dc, menua) && showVisibleList)
                    Menu_PaintAll_AppendToVisibleList(visibleList, 0x400u, (char *)menua->window.name);
            }
        }
    }

    {
        PROF_SCOPED("Menu_PaintAll_PaintOpenMenus");
        for (menuIndexb = drawStart; menuIndexb < dc->openMenuCount; ++menuIndexb)
        {
            menub = dc->menuStack[menuIndexb];
            if (!menub)
                MyAssertHandler(".\\ui\\ui_shared.cpp", 6153, 0, "%s", "menu");
            if (Menu_Paint(dc, menub) && showVisibleList)
                Menu_PaintAll_AppendToVisibleList(visibleList, 0x400u, (char *)menub->window.name);
        }
    }

    {
        PROF_SCOPED("Menu_PaintAll_Debug");

        if (g_debugMode)
        {
            font = UI_GetFontHandle(&scrPlaceView[dc->localClientNum], 0, 0.5);
            v1 = va("fps: %f", dc->FPS);
            UI_DrawText(&scrPlaceView[dc->localClientNum], v1, 0x7FFFFFFF, font, 5.0, 25.0, 0, 0, 0.5, colorWhite, 0);
        }
    }

    {
        PROF_SCOPED("Menu_PaintAll_DrawVisibleList");

        if (showVisibleList)
            Menu_PaintAll_DrawVisibleList(visibleList, dc);
    }
    
}

void __cdecl TRACK_ui_shared()
{
    track_static_alloc_internal(&scrollInfo, 32, "scrollInfo", 34);
    track_static_alloc_internal((void *)commandList, 336, "commandList", 34);
}

void __cdecl UI_AddMenuList(UiContext *dc, MenuList *menuList)
{
    int i; // [esp+0h] [ebp-4h]

    if (menuList)
    {
        for (i = 0; i < menuList->menuCount; ++i)
            UI_AddMenu(dc, menuList->menus[i]);
    }
}

void __cdecl UI_AddMenu(UiContext *dc, menuDef_t *menu)
{
    if (dc->menuCount >= 640)
        Com_Error(ERR_DROP, "UI_AddMenu: EXE_ERR_OUT_OF_MEMORY");
    if (!menu)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 6297, 0, "%s", "menu");
    if (dc->menuCount >= 0x280u)
        MyAssertHandler(
            ".\\ui\\ui_shared.cpp",
            6298,
            0,
            "dc->menuCount doesn't index ARRAY_COUNT( dc->Menus )\n\t%i not in [0, %i)",
            dc->menuCount,
            640);
    if (IsFastFileLoad() && DB_FindXAssetHeader(ASSET_TYPE_MENU, menu->window.name).menu != menu)
        MyAssertHandler(".\\ui\\ui_shared.cpp", 6304, 0, "%s", "touchMenu == menu");
    dc->Menus[dc->menuCount++] = menu;
}

int __cdecl UI_PlayLocalSoundAliasByName(unsigned int localClientNum, const char *aliasname)
{
    return SND_PlayLocalSoundAliasByName(localClientNum, aliasname, SASYS_UI);
}

int __cdecl UI_GetMenuScreen()
{
    int menu; // [esp+0h] [ebp-4h]

    menu = UI_GetForcedMenuScreen();
    if (menu >= 0)
        return menu;
    else
        return 1;
}

int __cdecl UI_GetForcedMenuScreen()
{
    return -1;
}

int __cdecl UI_GetMenuScreenForError()
{
    int menu; // [esp+0h] [ebp-4h]

    menu = UI_GetForcedMenuScreen();
    if (menu >= 0)
        return menu;
    else
        return 0;
}

MenuList *__cdecl UI_LoadMenu(char *menuFile, int imageTrack)
{
    if (IsFastFileLoad())
        return UI_LoadMenus_FastFile(menuFile);
    else
        return UI_LoadMenu_LoadObj(menuFile, imageTrack);
}

MenuList *__cdecl UI_LoadMenus(char *menuFile, int imageTrack)
{
    if (IsFastFileLoad())
        return UI_LoadMenus_FastFile(menuFile);
    else
        return UI_LoadMenus_LoadObj(menuFile, imageTrack);
}

MenuList *__cdecl UI_LoadMenus_FastFile(const char *menuFile)
{
    return DB_FindXAssetHeader(ASSET_TYPE_MENULIST, menuFile).menuList;
}

