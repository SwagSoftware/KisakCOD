#include "devgui.h"
#include <qcommon/mem_track.h>
#include <qcommon/qcommon.h>
#include <qcommon/cmd.h>
#include <client_mp/client_mp.h>
#include <client/client.h>
#include <cgame/cg_local.h>


const dvar_t *devgui_colorTextGray;
const dvar_t *devgui_colorBgndGray;
const dvar_t *devgui_colorGraphKnotSelected;
const dvar_t *devgui_colorSliderKnob;
const dvar_t *devgui_colorBgnd;
const dvar_t *devgui_colorText;
const dvar_t *devgui_bevelShade;
const dvar_t *devgui_colorBgndSel;
const dvar_t *devgui_colorGraphKnotEditing;
const dvar_t *devgui_colorTextGraySel;
const dvar_t *devgui_colorSliderBgnd;
const dvar_t *devgui_colorSliderKnobSel;
const dvar_t *devgui_colorTextSel;
const dvar_t *devgui_colorBgndGraySel;
const dvar_t *devgui_colorGraphKnotNormal;

devguiGlob_t devguiGlob;

void __cdecl TRACK_devgui()
{
    track_static_alloc_internal(&devguiGlob, 24080, "devguiGlob", 0);
}

void __cdecl DevGui_AddDvar(const char *path, const dvar_s *dvar)
{
    unsigned __int16 handle; // [esp+0h] [ebp-8h]
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    if (!path)
        MyAssertHandler(".\\devgui\\devgui.cpp", 443, 0, "%s", "path");
    if (!dvar)
        MyAssertHandler(".\\devgui\\devgui.cpp", 444, 0, "%s", "dvar");
    if (DevGui_IsValidPath(path))
    {
        handle = DevGui_ConstructPath_r(0, path);
        menu = DevGui_GetMenu(handle);
        if (!menu)
            MyAssertHandler(".\\devgui\\devgui.cpp", 451, 0, "%s", "menu");
        if (!menu->menus[0].childType && !menu->menus[0].child.menu
            || menu->menus[0].childType == 1 && menu->menus[0].child.dvar == dvar)
        {
            menu->menus[0].childType = 1;
            menu->menus[0].child.command = (const char *)dvar;
        }
        else
        {
            Com_Printf(
                11,
                "Path '%s' can't be used for dvar '%s' because it is already used for something else.\n",
                path,
                (const char *)dvar);
        }
    }
}

devguiGlob_t *__cdecl DevGui_GetMenu(unsigned __int16 handle)
{
    if (!handle || handle > 0x258u)
        MyAssertHandler(
            ".\\devgui\\devgui.cpp",
            118,
            0,
            "handle not in [1, ARRAY_COUNT( devguiGlob.menus )]\n\t%i not in [%i, %i]",
            handle,
            1,
            600);
    return (devguiGlob_t *)((char *)&devguiGlob + 40 * handle - 40);
}

unsigned __int16 __cdecl DevGui_ConstructPath_r(unsigned __int16 parent, const char *path)
{
    char label[28]; // [esp+0h] [ebp-28h] BYREF
    DevGuiTokenResult tokResult; // [esp+20h] [ebp-8h]
    __int16 sortKey; // [esp+24h] [ebp-4h] BYREF

    do
    {
        tokResult = (DevGuiTokenResult)DevGui_PathToken(&path, label, &sortKey);
        if (tokResult == DEVGUI_TOKEN_ERROR)
            MyAssertHandler(".\\devgui\\devgui.cpp", 393, 0, "%s", "tokResult != DEVGUI_TOKEN_ERROR");
        parent = DevGui_RegisterMenu(parent, label, sortKey);
    } while (tokResult != DEVGUI_TOKEN_LAST);
    return parent;
}

unsigned __int16 __cdecl DevGui_RegisterMenu(unsigned __int16 parentHandle, const char *label, __int16 sortKey)
{
    unsigned __int16 childHandle; // [esp+0h] [ebp-4h]

    childHandle = DevGui_FindMenu(parentHandle, label);
    if (!childHandle)
        return DevGui_CreateMenu(parentHandle, label, sortKey);
    return childHandle;
}

unsigned __int16 __cdecl DevGui_CreateMenu(unsigned __int16 parentHandle, const char *label, __int16 sortKey)
{
    char v4; // [esp+3h] [ebp-25h]
    DevMenuItem *v5; // [esp+8h] [ebp-20h]
    unsigned __int16 handle; // [esp+10h] [ebp-18h]
    unsigned __int16 *prevNext; // [esp+14h] [ebp-14h]
    DevMenuItem *menu; // [esp+18h] [ebp-10h]
    unsigned __int16 prev; // [esp+1Ch] [ebp-Ch]
    devguiGlob_t *nextMenu; // [esp+20h] [ebp-8h]
    devguiGlob_t *parentMenu; // [esp+24h] [ebp-4h]

    menu = devguiGlob.nextFreeMenu;
    if (!devguiGlob.nextFreeMenu)
        Com_Error(ERR_DROP, "Too many devgui entries (more than %i)", 600);
    devguiGlob.nextFreeMenu = *(DevMenuItem **)menu->label;
    handle = DevGui_GetMenuHandle(menu);
    v5 = menu;
    do
    {
        v4 = *label;
        v5->label[0] = *label++;
        v5 = (DevMenuItem *)((char *)v5 + 1);
    } while (v4);
    menu->childType = 0;
    menu->childMenuMemory = 0;
    menu->sortKey = sortKey;
    menu->parent = parentHandle;
    menu->child.menu = 0;
    if (parentHandle)
        parentMenu = DevGui_GetMenu(parentHandle);
    else
        parentMenu = (devguiGlob_t *)&devguiGlob.topmostMenu;
    prev = 0;
    for (prevNext = (unsigned __int16 *)&parentMenu->menus[0].child; *prevNext; prevNext = &nextMenu->menus[0].nextSibling)
    {
        nextMenu = DevGui_GetMenu(*prevNext);
        if (!DevGui_CompareMenus(nextMenu->menus, menu))
            MyAssertHandler(".\\devgui\\devgui.cpp", 251, 0, "%s", "DevGui_CompareMenus( nextMenu, menu ) != 0");
        if (DevGui_CompareMenus(nextMenu->menus, menu) > 0)
            break;
        prev = *prevNext;
    }
    menu->nextSibling = *prevNext;
    menu->prevSibling = prev;
    *prevNext = handle;
    if (menu->nextSibling)
        DevGui_GetMenu(menu->nextSibling)->menus[0].prevSibling = handle;
    return handle;
}

unsigned __int16 __cdecl DevGui_GetMenuHandle(DevMenuItem *menu)
{
    unsigned __int16 handle; // [esp+0h] [ebp-4h]

    handle = ((char *)menu - (char *)&devguiGlob) / 40 + 1;
    if ((unsigned __int16)(((char *)menu - (char *)&devguiGlob) / 40) == 0xFFFF || handle > 0x258u)
        MyAssertHandler(
            ".\\devgui\\devgui.cpp",
            137,
            0,
            "handle not in [1, ARRAY_COUNT( devguiGlob.menus )]\n\t%i not in [%i, %i]",
            handle,
            1,
            600);
    return ((char *)menu - (char *)&devguiGlob) / 40 + 1;
}

int __cdecl DevGui_CompareMenus(const DevMenuItem *menu0, const DevMenuItem *menu1)
{
    if (!menu0)
        MyAssertHandler(".\\devgui\\devgui.cpp", 175, 0, "%s", "menu0");
    if (!menu1)
        MyAssertHandler(".\\devgui\\devgui.cpp", 176, 0, "%s", "menu1");
    if (menu0->sortKey == menu1->sortKey)
        return I_stricmp(menu0->label, menu1->label);
    else
        return menu0->sortKey - menu1->sortKey;
}

unsigned __int16 __cdecl DevGui_FindMenu(unsigned __int16 parentHandle, const char *label)
{
    devguiGlob_t *childMenu; // [esp+0h] [ebp-Ch]
    unsigned __int16 childHandle; // [esp+4h] [ebp-8h]
    devguiGlob_t *parentMenu; // [esp+8h] [ebp-4h]

    if (parentHandle)
        parentMenu = DevGui_GetMenu(parentHandle);
    else
        parentMenu = (devguiGlob_t *)&devguiGlob.topmostMenu;
    if (parentMenu->menus[0].childType)
        MyAssertHandler(
            ".\\devgui\\devgui.cpp",
            278,
            0,
            "%s\n\t(parentMenu->childType) = %i",
            "(parentMenu->childType == DEV_CHILD_MENU)",
            parentMenu->menus[0].childType);
    for (childHandle = parentMenu->menus[0].child.menu; childHandle; childHandle = childMenu->menus[0].nextSibling)
    {
        childMenu = DevGui_GetMenu(childHandle);
        if (!childMenu)
            MyAssertHandler(".\\devgui\\devgui.cpp", 283, 0, "%s", "childMenu");
        if (!I_stricmp(label, (const char *)childMenu))
            return childHandle;
    }
    return 0;
}

DevGuiTokenResult __cdecl DevGui_PathToken(const char **pathInOut, char *label, __int16 *sortKeyOut)
{
    const char *path; // [esp+0h] [ebp-10h]
    __int16 sign; // [esp+4h] [ebp-Ch]
    int labelLen; // [esp+8h] [ebp-8h]
    __int16 sortKey; // [esp+Ch] [ebp-4h]

    if (!pathInOut)
        MyAssertHandler(".\\devgui\\devgui.cpp", 310, 0, "%s", "pathInOut");
    if (!*pathInOut)
        MyAssertHandler(".\\devgui\\devgui.cpp", 311, 0, "%s", "*pathInOut");
    if (!label)
        MyAssertHandler(".\\devgui\\devgui.cpp", 312, 0, "%s", "label");
    if (!sortKeyOut)
        MyAssertHandler(".\\devgui\\devgui.cpp", 313, 0, "%s", "sortKeyOut");
    path = *pathInOut;
    labelLen = 0;
    *sortKeyOut = 0x7FFF;
    while (*path)
    {
        if (*path == 58)
        {
            label[labelLen] = 0;
            ++path;
            sortKey = 0;
            sign = 1;
            if (*path == 45)
            {
                sign = -1;
                ++path;
            }
            else if (*path == 43)
            {
                ++path;
            }
            if (*path < 48 || *path > 57)
                return (DevGuiTokenResult)0;
            do
                sortKey = 10 * sortKey + *path++ - 48;
            while (*path >= 48 && *path <= 57);
            *sortKeyOut = sign * sortKey;
            if (*path && (*path != 47 || path[1] == 47))
                return (DevGuiTokenResult)0;
        }
        else
        {
            if (*path == 47 && *++path != 47)
            {
                label[labelLen] = 0;
                *pathInOut = path;
                return (DevGuiTokenResult)1;
            }
            label[labelLen] = *path++;
            if (++labelLen == 26)
                return (DevGuiTokenResult)0;
        }
    }
    if (!labelLen)
        return (DevGuiTokenResult)0;
    *pathInOut = path;
    label[labelLen] = 0;
    return (DevGuiTokenResult)2;
}

char __cdecl DevGui_IsValidPath(const char *path)
{
    char label[28]; // [esp+20h] [ebp-2Ch] BYREF
    const char *originalPath; // [esp+40h] [ebp-Ch]
    DevGuiTokenResult tokResult; // [esp+44h] [ebp-8h]
    __int16 sortKey; // [esp+48h] [ebp-4h] BYREF

    if (!path)
        MyAssertHandler(".\\devgui\\devgui.cpp", 408, 0, "%s", "path");
    if (strlen(path) <= 0x78)
    {
        originalPath = path;
        tokResult = DevGui_PathToken(&path, label, &sortKey);
        if (tokResult == DEVGUI_TOKEN_LAST)
        {
            Com_Printf(11, "Path '%s' must have at least one menu separator ('/' character).\n", originalPath);
            return 0;
        }
        else
        {
            do
            {
                if (tokResult == DEVGUI_TOKEN_ERROR)
                {
                    Com_Printf(
                        11,
                        "path '%s' is invalid.  Format is 'menu name:sortkey/submenu/...', where 'sortkey' is any (possibly signed) integer.\n",
                        originalPath);
                    return 0;
                }
                tokResult = DevGui_PathToken(&path, label, &sortKey);
            } while (tokResult != DEVGUI_TOKEN_LAST);
            return 1;
        }
    }
    else
    {
        Com_Printf(11, "Path '%s' must be no longer than %i characters (currently %i).\n", path, 120, strlen(path));
        return 0;
    }
}

void __cdecl DevGui_AddCommand(const char *path, char *command)
{
    unsigned __int16 handle; // [esp+0h] [ebp-8h]
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    if (!path)
        MyAssertHandler(".\\devgui\\devgui.cpp", 468, 0, "%s", "path");
    if (!command)
        MyAssertHandler(".\\devgui\\devgui.cpp", 469, 0, "%s", "command");
    if (DevGui_IsValidPath(path))
    {
        handle = DevGui_ConstructPath_r(0, path);
        menu = DevGui_GetMenu(handle);
        if (!menu)
            MyAssertHandler(".\\devgui\\devgui.cpp", 476, 0, "%s", "menu");
        if (!menu->menus[0].childType && !menu->menus[0].child.menu
            || menu->menus[0].childType == 2 && menu->menus[0].child.command == command)
        {
            menu->menus[0].childType = 2;
            menu->menus[0].child.command = CopyString(command);
        }
        else
        {
            Com_Printf(
                11,
                "Path '%s' can't be used for command '%s' because it is already used for something else.\n",
                path,
                command);
        }
    }
}

void __cdecl DevGui_AddGraph(const char *path, DevGraph *graph)
{
    unsigned __int16 handle; // [esp+0h] [ebp-8h]
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    if (!graph)
        MyAssertHandler(".\\devgui\\devgui.cpp", 493, 0, "%s", "graph");
    if (!graph->knots)
        MyAssertHandler(".\\devgui\\devgui.cpp", 493, 0, "%s", "graph->knots");
    if (!graph->knotCount)
        MyAssertHandler(".\\devgui\\devgui.cpp", 493, 0, "%s", "graph->knotCount");
    if (graph->knotCountMax <= 0)
        MyAssertHandler(".\\devgui\\devgui.cpp", 493, 0, "%s", "graph->knotCountMax > 0");
    if (!path)
        MyAssertHandler(".\\devgui\\devgui.cpp", 494, 0, "%s", "path");
    if (*graph->knotCount < 2 || *graph->knotCount > graph->knotCountMax)
        MyAssertHandler(
            ".\\devgui\\devgui.cpp",
            496,
            0,
            "%s",
            "*graph->knotCount >= 2 && *graph->knotCount <= graph->knotCountMax");
    if (DevGui_IsValidPath(path))
    {
        handle = DevGui_ConstructPath_r(0, path);
        menu = DevGui_GetMenu(handle);
        if (!menu)
            MyAssertHandler(".\\devgui\\devgui.cpp", 503, 0, "%s", "menu");
        if (!menu->menus[0].childType && !menu->menus[0].child.menu
            || menu->menus[0].childType == 3 && menu->menus[0].child.graph == graph)
        {
            menu->menus[0].childType = 3;
            menu->menus[0].child.command = (const char *)graph;
        }
        else
        {
            Com_Printf(11, "Path '%s' can't be added for this graph because it is already used for something else.\n", path);
        }
    }
}

void __cdecl DevGui_RemoveMenu(const char *path)
{
    const char *v1; // eax
    unsigned __int16 handle; // [esp+0h] [ebp-38h]
    char label[28]; // [esp+4h] [ebp-34h] BYREF
    DevMenuItem *parent; // [esp+24h] [ebp-14h]
    DevMenuItem *menu; // [esp+28h] [ebp-10h]
    DevMenuItem *sibling; // [esp+2Ch] [ebp-Ch]
    DevGuiTokenResult tokResult; // [esp+30h] [ebp-8h]
    __int16 sortKey; // [esp+34h] [ebp-4h] BYREF

    if (DevGui_IsValidPath(path))
    {
        handle = 0;
        while (1)
        {
            tokResult = DevGui_PathToken(&path, label, &sortKey);
            if (tokResult == DEVGUI_TOKEN_ERROR)
                MyAssertHandler(".\\devgui\\devgui.cpp", 587, 0, "%s", "tokResult != DEVGUI_TOKEN_ERROR");
            handle = DevGui_FindMenu(handle, label);
            if (!handle)
                break;
            if (tokResult == DEVGUI_TOKEN_LAST)
            {
                menu = (DevMenuItem *)DevGui_GetMenu(handle);
                if (!menu)
                    MyAssertHandler(".\\devgui\\devgui.cpp", 597, 0, "%s", "menu");
                if (menu->nextSibling)
                {
                    sibling = (DevMenuItem *)DevGui_GetMenu(menu->nextSibling);
                    sibling->prevSibling = menu->prevSibling;
                }
                if (menu->prevSibling)
                {
                    sibling = (DevMenuItem *)DevGui_GetMenu(menu->prevSibling);
                    sibling->nextSibling = menu->nextSibling;
                }
                else
                {
                    if (!menu->parent)
                        MyAssertHandler(".\\devgui\\devgui.cpp", 610, 0, "%s", "menu->parent");
                    parent = (DevMenuItem *)DevGui_GetMenu(menu->parent);
                    if (!parent)
                        MyAssertHandler(".\\devgui\\devgui.cpp", 612, 0, "%s", "parent");
                    if (parent->childType)
                    {
                        v1 = va("menu %s type %i", parent->label, parent->childType);
                        MyAssertHandler(".\\devgui\\devgui.cpp", 613, 0, "%s\n\t%s", "parent->childType == DEV_CHILD_MENU", v1);
                    }
                    parent->child.menu = menu->nextSibling;
                }
                menu->nextSibling = 0;
                menu->prevSibling = 0;
                DevGui_FreeMenu_r(handle);
                return;
            }
        }
    }
}

void __cdecl DevGui_FreeMenu_r(unsigned __int16 handle)
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if (handle)
    {
        if (devguiGlob.selectedMenu == handle)
        {
            devguiGlob.selectedMenu = 0;
            devguiGlob.isActive = 0;
            devguiGlob.editingMenuItem = 0;
        }
        menu = DevGui_GetMenu(handle);
        if (!menu)
            MyAssertHandler(".\\devgui\\devgui.cpp", 199, 0, "%s", "menu");
        if (menu->menus[0].childType == 2)
        {
            FreeString(menu->menus[0].child.command);
            menu->menus[0].child.command = 0;
        }
        else if (!menu->menus[0].childType)
        {
            DevGui_FreeMenu_r(menu->menus[0].child.menu);
        }
        DevGui_FreeMenu_r(menu->menus[0].nextSibling);
        *(unsigned int*)menu->menus[0].label = (unsigned int)devguiGlob.nextFreeMenu;
        devguiGlob.nextFreeMenu = (DevMenuItem *)menu;
    }
}

void __cdecl DevGui_OpenMenu(const char *path)
{
    unsigned __int16 handle; // [esp+0h] [ebp-30h]
    char label[28]; // [esp+4h] [ebp-2Ch] BYREF
    DevMenuItem *menu; // [esp+24h] [ebp-Ch]
    DevGuiTokenResult tokResult; // [esp+28h] [ebp-8h]
    __int16 sortKey; // [esp+2Ch] [ebp-4h] BYREF

    if (DevGui_IsValidPath(path))
    {
        handle = 0;
        while (1)
        {
            tokResult = DevGui_PathToken(&path, label, &sortKey);
            if (tokResult == DEVGUI_TOKEN_ERROR)
                MyAssertHandler(".\\devgui\\devgui.cpp", 638, 0, "%s", "tokResult != DEVGUI_TOKEN_ERROR");
            handle = DevGui_FindMenu(handle, label);
            if (!handle)
                break;
            if (tokResult == DEVGUI_TOKEN_LAST)
            {
                menu = (DevMenuItem *)DevGui_GetMenu(handle);
                if (menu->childType == 2)
                {
                    Cbuf_InsertText(0, (char *)menu->child.command);
                }
                else
                {
                    devguiGlob.isActive = 1;
                    devguiGlob.selectedMenu = handle;
                    devguiGlob.editingMenuItem = DevGui_EditableMenuItem(menu);
                    if (!devguiGlob.editingMenuItem && !menu->childType)
                        devguiGlob.selectedMenu = menu->child.menu;
                    devguiGlob.selRow = 0;
                }
                return;
            }
        }
    }
}

bool __cdecl DevGui_EditableMenuItem(const DevMenuItem *menu)
{
    if (menu->childType == 3)
        return 1;
    if (menu->childType != 1)
        return 0;
    if (*((_BYTE *)menu->child.command + 10) == 7)
        return 0;
    return *((_BYTE *)menu->child.command + 10) != 6 || *((unsigned int *)menu->child.command + 15);
}

void __cdecl DevGui_Draw(int localClientNum)
{
    int origin[2]; // [esp+0h] [ebp-10h] BYREF
    DevMenuItem *menuItem; // [esp+8h] [ebp-8h]
    unsigned __int16 parent; // [esp+Ch] [ebp-4h]

    if (devguiGlob.isActive)
    {
        menuItem = (DevMenuItem *)DevGui_GetMenu(devguiGlob.selectedMenu);
        if (devguiGlob.editingMenuItem)
        {
            if (menuItem->childType != 3 && menuItem->childType != 1)
                MyAssertHandler(
                    ".\\devgui\\devgui.cpp",
                    1225,
                    0,
                    "%s\n\t(menuItem->childType) = %i",
                    "(menuItem->childType == DEV_CHILD_GRAPH || menuItem->childType == DEV_CHILD_DVAR)",
                    menuItem->childType);
            if (menuItem->childType == 1)
                DevGui_DrawSliders(menuItem);
            else
                DevGui_DrawGraph(menuItem, localClientNum);
        }
        else
        {
            origin[0] = devguiGlob.left;
            origin[1] = devguiGlob.top;
            parent = DevGui_GetMenuParent(devguiGlob.selectedMenu);
            DevGui_ChooseOrigin(origin);
            DevGui_DrawMenu(parent, devguiGlob.selectedMenu, origin);
        }
        if (devguiGlob.bindNextKey)
            DevGui_DrawBindNextKey();
    }
}

unsigned __int16 __cdecl DevGui_GetMenuParent(unsigned __int16 handle)
{
    return DevGui_GetMenu(handle)->menus[0].parent;
}

void __cdecl DevGui_DrawMenu(unsigned __int16 menuHandle, unsigned __int16 activeChild, int *origin)
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if (!origin)
        MyAssertHandler(".\\devgui\\devgui.cpp", 860, 0, "%s", "origin");
    menu = DevGui_GetMenu(menuHandle);
    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 863, 0, "%s", "menu");
    if (menu->menus[0].parent)
        DevGui_DrawMenu(menu->menus[0].parent, menuHandle, origin);
    else
        DevGui_DrawMenuHorizontally(&devguiGlob.topmostMenu, menuHandle, origin);
    DevGui_DrawMenuVertically(menu->menus, activeChild, origin);
}

void __cdecl DevGui_DrawMenuVertically(const DevMenuItem *menu, unsigned __int16 activeChild, int *origin)
{
    devguiGlob_t *childMenu; // [esp+Ch] [ebp-38h]
    devguiGlob_t *childMenua; // [esp+Ch] [ebp-38h]
    int activeChildIndex; // [esp+10h] [ebp-34h]
    unsigned __int8 bgndColor[4]; // [esp+18h] [ebp-2Ch] BYREF
    unsigned __int8 textColor[4]; // [esp+1Ch] [ebp-28h] BYREF
    int subMenuStringPos; // [esp+20h] [ebp-24h]
    int visibleMenuCount; // [esp+24h] [ebp-20h]
    __int64 x; // [esp+28h] [ebp-1Ch]
    int h; // [esp+30h] [ebp-14h]
    float shade; // [esp+34h] [ebp-10h]
    int childCount; // [esp+38h] [ebp-Ch]
    unsigned __int16 childHandle; // [esp+3Ch] [ebp-8h]
    int w; // [esp+40h] [ebp-4h]

    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 721, 0, "%s", "menu");
    x = *(_QWORD *)origin;
    w = DevGui_MaxChildMenuWidth(menu);
    h = R_TextHeight(cls.consoleFont) + 8;
    subMenuStringPos = x + w - 4 - DevGui_SubMenuTextWidth();
    childCount = 0;
    activeChildIndex = 0;
    for (childHandle = menu->child.menu; childHandle; childHandle = childMenu->menus[0].nextSibling)
    {
        childMenu = DevGui_GetMenu(childHandle);
        if (childHandle == activeChild)
            activeChildIndex = childCount;
        ++childCount;
    }
    visibleMenuCount = (devguiGlob.bottom - 2 * h - HIDWORD(x)) / h;
    if (visibleMenuCount < childCount && activeChildIndex > visibleMenuCount)
    {
        *(unsigned int *)bgndColor = devgui_colorBgnd->current.integer;
        *(unsigned int *)textColor = devgui_colorText->current.integer;
        shade = devgui_bevelShade->current.value;
        DevGui_DrawBevelBox(x, w, h, shade, bgndColor);
        DevGui_DrawFont(x + 4, HIDWORD(x) + 4, textColor, (char*)"...");
        HIDWORD(x) += h;
    }
    childCount = 0;
    for (childHandle = menu->child.menu; childHandle; childHandle = childMenua->menus[0].nextSibling)
    {
        childMenua = DevGui_GetMenu(childHandle);
        if (activeChildIndex - visibleMenuCount <= childCount)
        {
            if (childHandle == activeChild)
            {
                *origin = w + x;
                origin[1] = HIDWORD(x);
                if (DevGui_MenuItemDisabled(childMenua->menus))
                {
                    *(unsigned int *)bgndColor = devgui_colorBgndGraySel->current.integer;
                    *(unsigned int *)textColor = devgui_colorTextGraySel->current.integer;
                }
                else
                {
                    *(unsigned int *)bgndColor = devgui_colorBgndSel->current.integer;
                    *(unsigned int *)textColor = devgui_colorTextSel->current.integer;
                }
                if (childHandle == devguiGlob.selectedMenu && DevGui_IsButtonDown(INPUT_ACCEPT))
                    shade = 2.0 - devgui_bevelShade->current.value;
                else
                    shade = devgui_bevelShade->current.value;
            }
            else
            {
                if (DevGui_MenuItemDisabled(childMenua->menus))
                {
                    *(unsigned int *)bgndColor = devgui_colorBgndGray->current.integer;
                    *(unsigned int *)textColor = devgui_colorTextGray->current.integer;
                }
                else
                {
                    *(unsigned int *)bgndColor = devgui_colorBgnd->current.integer;
                    *(unsigned int *)textColor = devgui_colorText->current.integer;
                }
                shade = devgui_bevelShade->current.value;
            }
            DevGui_DrawBevelBox(x, w, h, shade, bgndColor);
            DevGui_DrawFont(x + 4, HIDWORD(x) + 4, textColor, (char *)childMenua);
            if (!childMenua->menus[0].childType && childMenua->menus[0].child.menu)
                DevGui_DrawFont(subMenuStringPos, HIDWORD(x) + 4, textColor, (char*)" >");
            HIDWORD(x) += h;
            ++childCount;
        }
        else
        {
            ++childCount;
        }
    }
}

bool __cdecl DevGui_MenuItemDisabled(const DevMenuItem *menu)
{
    return menu->childType == 1 && !DevGui_EditableMenuItem(menu);
}

int __cdecl DevGui_SubMenuTextWidth()
{
    return R_TextWidth(" >", 0, cls.consoleFont);
}

int __cdecl DevGui_MaxChildMenuWidth(const DevMenuItem *menu)
{
    devguiGlob_t *childMenu; // [esp+0h] [ebp-10h]
    int widthCur; // [esp+4h] [ebp-Ch]
    int widthMax; // [esp+8h] [ebp-8h]
    unsigned __int16 childHandle; // [esp+Ch] [ebp-4h]

    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 689, 0, "%s", "menu");
    if (menu->childType)
        MyAssertHandler(".\\devgui\\devgui.cpp", 690, 0, "%s", "menu->childType == DEV_CHILD_MENU");
    widthMax = DevGui_MenuItemWidth(menu);
    for (childHandle = menu->child.menu; childHandle; childHandle = childMenu->menus[0].nextSibling)
    {
        childMenu = DevGui_GetMenu(childHandle);
        widthCur = DevGui_MenuItemWidth(childMenu->menus);
        if (widthMax < widthCur)
            widthMax = widthCur;
    }
    return widthMax;
}

int __cdecl DevGui_MenuItemWidth(const DevMenuItem *menu)
{
    int width; // [esp+0h] [ebp-4h]

    width = R_TextWidth(menu->label, 0, cls.consoleFont) + 8;
    if (!menu->childType && menu->child.menu)
        width += DevGui_SubMenuTextWidth();
    return width;
}

void __cdecl DevGui_DrawMenuHorizontally(const DevMenuItem *menu, unsigned __int16 activeChild, int *origin)
{
    devguiGlob_t *childMenu; // [esp+8h] [ebp-20h]
    unsigned __int8 bgndColor[4]; // [esp+Ch] [ebp-1Ch] BYREF
    unsigned __int8 textColor[4]; // [esp+10h] [ebp-18h] BYREF
    __int64 x; // [esp+14h] [ebp-14h]
    int h; // [esp+1Ch] [ebp-Ch]
    unsigned __int16 childHandle; // [esp+20h] [ebp-8h]
    int w; // [esp+24h] [ebp-4h]

    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 825, 0, "%s", "menu");
    x = *(_QWORD *)origin;
    h = R_TextHeight(cls.consoleFont) + 8;
    for (childHandle = menu->child.menu; childHandle; childHandle = childMenu->menus[0].nextSibling)
    {
        childMenu = DevGui_GetMenu(childHandle);
        w = DevGui_MenuItemWidth(childMenu->menus);
        if (childHandle == activeChild)
        {
            *origin = x;
            origin[1] = h + HIDWORD(x);
            *(unsigned int *)bgndColor = devgui_colorBgndSel->current.integer;
            *(unsigned int *)textColor = devgui_colorTextSel->current.integer;
        }
        else
        {
            *(unsigned int *)bgndColor = devgui_colorBgnd->current.integer;
            *(unsigned int *)textColor = devgui_colorText->current.integer;
        }
        DevGui_DrawBevelBox(x, w, h, devgui_bevelShade->current.value, bgndColor);
        DevGui_DrawFont(x + 4, HIDWORD(x) + 4, textColor, (char *)childMenu);
        LODWORD(x) = w + x;
    }
}

void __cdecl DevGui_ChooseOrigin(int *origin)
{
    unsigned __int16 handle; // [esp+0h] [ebp-18h]
    devguiGlob_t *childMenu; // [esp+4h] [ebp-14h]
    devguiGlob_t *menu; // [esp+8h] [ebp-10h]
    unsigned __int16 activeChild; // [esp+Ch] [ebp-Ch]
    unsigned __int16 childHandle; // [esp+10h] [ebp-8h]
    int w; // [esp+14h] [ebp-4h]

    w = 0;
    activeChild = devguiGlob.selectedMenu;
    for (handle = DevGui_GetMenuParent(devguiGlob.selectedMenu); handle; handle = menu->menus[0].parent)
    {
        menu = DevGui_GetMenu(handle);
        if (!menu)
            MyAssertHandler(".\\devgui\\devgui.cpp", 887, 0, "%s", "menu");
        w += DevGui_MaxChildMenuWidth(menu->menus);
        activeChild = handle;
    }
    for (childHandle = devguiGlob.topmostMenu.child.menu;
        childHandle != activeChild;
        childHandle = childMenu->menus[0].nextSibling)
    {
        childMenu = DevGui_GetMenu(childHandle);
        w += DevGui_MenuItemWidth(childMenu->menus);
    }
    *origin = devguiGlob.left;
    origin[1] = devguiGlob.top;
    if (w > devguiGlob.right - devguiGlob.left)
        *origin -= w - (devguiGlob.right - devguiGlob.left);
}

void __cdecl DevGui_DrawSliders(const DevMenuItem *menu)
{
    const char *v1; // eax
    const char *v2; // eax
    __int64 v3; // [esp-10h] [ebp-60h]
    float v4; // [esp+14h] [ebp-3Ch]
    DvarValue *v5; // [esp+18h] [ebp-38h]
    DvarValue *p_current; // [esp+1Ch] [ebp-34h]
    int rowHeight; // [esp+24h] [ebp-2Ch]
    int width; // [esp+2Ch] [ebp-24h]
    int height; // [esp+30h] [ebp-20h]
    float fractiona; // [esp+34h] [ebp-1Ch]
    float fractionb; // [esp+34h] [ebp-1Ch]
    float fraction; // [esp+34h] [ebp-1Ch]
    int rowCount; // [esp+38h] [ebp-18h]
    int row; // [esp+3Ch] [ebp-14h]
    int rowa; // [esp+3Ch] [ebp-14h]
    const dvar_s *dvar; // [esp+40h] [ebp-10h]
    int xa; // [esp+44h] [ebp-Ch]
    __int64 x; // [esp+44h] [ebp-Ch]
    int x_4; // [esp+48h] [ebp-8h]
    int rowWidth; // [esp+4Ch] [ebp-4h]

    if (menu->childType != 1)
    {
        v1 = va("menu %s type %i", menu->label, menu->childType);
        MyAssertHandler(".\\devgui\\devgui.cpp", 998, 0, "%s\n\t%s", "menu->childType == DEV_CHILD_DVAR", v1);
    }
    dvar = menu->child.dvar;
    if (!dvar)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1000, 0, "%s\n\t(menu->label) = %s", "(dvar)", menu->label);
    rowWidth = devguiGlob.sliderWidth;
    rowHeight = DevGui_GetFontHeight();
    rowCount = DevGui_DvarRowCount(dvar);
    width = rowWidth + 8;
    height = rowHeight * (rowCount + 2) + 2 * rowCount + 14;
    if (dvar->type == 8)
        height += rowHeight + 2;
    xa = devguiGlob.left + (devguiGlob.right - devguiGlob.left - width) / 2;
    x_4 = devguiGlob.bottom - height;
    HIDWORD(v3) = devguiGlob.bottom - height;
    LODWORD(v3) = xa;
    DevGui_DrawBevelBox(
        v3,
        width,
        height,
        devgui_bevelShade->current.value,
        (const unsigned __int8 *)&devgui_colorBgnd->current);
    LODWORD(x) = xa + 4;
    HIDWORD(x) = x_4 + 6;
    DevGui_DrawSliderPath(x, SHIDWORD(x));
    if (dvar->type == 8)
    {
        HIDWORD(x) += rowHeight + 2;
        DevGui_DrawBox(x, SHIDWORD(x), rowWidth, rowHeight, (const unsigned __int8 *)&dvar->latched);
        for (row = 0; row < rowCount; ++row)
        {
            HIDWORD(x) += rowHeight + 2;
            if (row == devguiGlob.selRow)
                p_current = (DvarValue *)&devgui_colorSliderKnobSel->current;
            else
                p_current = (DvarValue *)&devgui_colorSliderKnob->current;
            fractiona = (double)dvar->latched.color[row] * 1.0 / 255.0;
            DevGui_DrawSingleSlider(x, rowWidth, rowHeight, fractiona, (const unsigned __int8 *)p_current);
        }
    }
    else if (dvar->type == 2 || dvar->type == 3 || dvar->type == 4)
    {
        for (rowa = 0; rowa < rowCount; ++rowa)
        {
            HIDWORD(x) += rowHeight + 2;
            if (rowa == devguiGlob.selRow)
                v5 = (DvarValue *)&devgui_colorSliderKnobSel->current;
            else
                v5 = (DvarValue *)&devgui_colorSliderKnob->current;
            fractionb = (dvar->latched.vector[rowa] - dvar->domain.value.min)
                / (dvar->domain.value.max - dvar->domain.value.min);
            DevGui_DrawSingleSlider(x, rowWidth, rowHeight, fractionb, (const unsigned __int8 *)v5);
        }
    }
    else
    {
        HIDWORD(x) += rowHeight + 2;
        if (dvar->type)
        {
            switch (dvar->type)
            {
            case 5u:
                if (dvar->domain.integer.max == dvar->domain.enumeration.stringCount)
                    fraction = 0.5;
                else
                    fraction = (double)(dvar->latched.integer - dvar->domain.enumeration.stringCount)
                    / (double)(dvar->domain.integer.max - dvar->domain.enumeration.stringCount);
                break;
            case 6u:
                if (dvar->domain.enumeration.stringCount > 1)
                    fraction = (double)dvar->latched.integer / (double)(dvar->domain.enumeration.stringCount - 1);
                else
                    fraction = 0.5;
                break;
            case 1u:
                fraction = (dvar->latched.value - dvar->domain.value.min) / (dvar->domain.value.max - dvar->domain.value.min);
                break;
            default:
                if (!alwaysfails)
                {
                    v2 = va("unhandled dvar type %i", dvar->type);
                    MyAssertHandler(".\\devgui\\devgui.cpp", 1073, 1, v2);
                }
                fraction = 0.5;
                break;
            }
        }
        else
        {
            if (dvar->latched.enabled)
                v4 = 1.0;
            else
                v4 = 0.0;
            fraction = v4;
        }
        DevGui_DrawSingleSlider(
            x,
            rowWidth,
            rowHeight,
            fraction,
            (const unsigned __int8 *)&devgui_colorSliderKnobSel->current);
    }
    DevGui_DrawDvarValue(x, HIDWORD(x) + rowHeight + 2, dvar);
}

void __cdecl DevGui_DrawSliderPath(int x, int y)
{
    char path[132]; // [esp+0h] [ebp-88h] BYREF

    DevGui_GetSliderPath(devguiGlob.selectedMenu, path, 0);
    DevGui_DrawFont(x, y, (const unsigned __int8 *)&devgui_colorText->current, path);
}

int __cdecl DevGui_GetSliderPath(unsigned __int16 menuHandle, char *path, int pathLen)
{
    int SliderPath; // eax
    unsigned int v5; // [esp+0h] [ebp-18h]
    devguiGlob_t *menu; // [esp+10h] [ebp-8h]

    menu = DevGui_GetMenu(menuHandle);
    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 912, 0, "%s", "menu");
    if (menu->menus[0].parent)
    {
        SliderPath = DevGui_GetSliderPath(menu->menus[0].parent, path, pathLen);
        path[SliderPath] = 47;
        pathLen = SliderPath + 1;
    }
    v5 = strlen((const char *)menu);
    if ((int)(v5 + pathLen) > 120)
        MyAssertHandler(
            ".\\devgui\\devgui.cpp",
            921,
            0,
            "%s\n\t(path + pathLen) = %s",
            "(pathLen + labelLen <= 120)",
            &path[pathLen]);
    memcpy((unsigned __int8 *)&path[pathLen], (unsigned __int8 *)menu, v5 + 1);
    return v5 + pathLen;
}

void __cdecl DevGui_DrawSingleSlider(
    __int64 x,
    int rowWidth,
    int rowHeight,
    float fraction,
    const unsigned __int8 *knobColor)
{
    __int64 v5; // [esp-10h] [ebp-30h]
    float v6; // [esp+Ch] [ebp-14h]

    DevGui_DrawBevelBox(
        x,
        rowWidth,
        rowHeight,
        devgui_bevelShade->current.value,
        (const unsigned __int8 *)&devgui_colorSliderBgnd->current);
    v6 = fraction * (double)(rowWidth - 8) + (double)(int)x;
    HIDWORD(v5) = HIDWORD(x);
    LODWORD(v5) = (int)(v6 + 9.313225746154785e-10);
    DevGui_DrawBevelBox(v5, 8, rowHeight, devgui_bevelShade->current.value, knobColor);
}

void __cdecl DevGui_DrawDvarValue(int x, int y, const dvar_s *dvar)
{
    const char *v3; // eax
    const char *v4; // [esp+0h] [ebp-8h]
    char *text; // [esp+4h] [ebp-4h]

    if (dvar->type)
    {
        if (dvar->type == 6)
        {
            v3 = Dvar_DisplayableLatchedValue(dvar);
            text = va("%i: %s", dvar->latched.integer, v3);
        }
        else
        {
            text = (char *)Dvar_DisplayableLatchedValue(dvar);
        }
        DevGui_DrawFont(x, y, (const unsigned __int8 *)&devgui_colorText->current, text);
    }
    else
    {
        if (dvar->latched.enabled)
            v4 = "On";
        else
            v4 = "Off";
        DevGui_DrawFont(x, y, (const unsigned __int8 *)&devgui_colorText->current, (char*)v4);
    }
}

int __cdecl DevGui_DvarRowCount(const dvar_s *dvar)
{
    int result; // eax

    switch (dvar->type)
    {
    case 2u:
        result = 2;
        break;
    case 3u:
        result = 3;
        break;
    case 4u:
    case 8u:
        result = 4;
        break;
    default:
        result = 1;
        break;
    }
    return result;
}

void DevGui_DrawBindNextKey()
{
    unsigned __int8 fadeColor[4]; // [esp+8h] [ebp-1Ch] BYREF
    unsigned __int8 textColor[4]; // [esp+Ch] [ebp-18h] BYREF
    int x; // [esp+10h] [ebp-14h]
    int y; // [esp+14h] [ebp-10h]
    int h; // [esp+18h] [ebp-Ch]
    const char *text; // [esp+1Ch] [ebp-8h]
    int w; // [esp+20h] [ebp-4h]

    *(unsigned int *)fadeColor = -1442840576;
    DevGui_DrawBox(
        devguiGlob.left,
        devguiGlob.top,
        devguiGlob.right - devguiGlob.left,
        devguiGlob.bottom - devguiGlob.top,
        fadeColor);
    text = "Press key to bind (ESC to cancel)...";
    w = DevGui_GetFontWidth("Press key to bind (ESC to cancel)...");
    h = DevGui_GetFontHeight();
    x = (devguiGlob.right + devguiGlob.left - w) / 2;
    y = (devguiGlob.bottom + devguiGlob.top - h) / 2;
    *(unsigned int *)textColor = -1;
    DevGui_DrawFont(x, y, textColor, (char *)text);
}

const char *MYINSTRUCTIONS = "<Y> Edits node,  <LB> Adds new node,  <RB> Removes node,  <START> Saves to disk"; // idb

void __cdecl DevGui_DrawGraph(const DevMenuItem *menu, int localClientNum)
{
    const char *v2; // eax
    float *v3; // ecx
    float *v4; // [esp+24h] [ebp-15Ch]
    float *v5; // [esp+28h] [ebp-158h]
    int rowHeight; // [esp+2Ch] [ebp-154h]
    int graphBottomY; // [esp+30h] [ebp-150h]
    float nextKnot; // [esp+34h] [ebp-14Ch]
    float nextKnot_4; // [esp+38h] [ebp-148h]
    int width; // [esp+3Ch] [ebp-144h]
    DevGraph *graph; // [esp+44h] [ebp-13Ch]
    int knotX; // [esp+48h] [ebp-138h]
    float endKnotPos[2]; // [esp+4Ch] [ebp-134h] BYREF
    int knotY; // [esp+54h] [ebp-12Ch]
    __int64 x; // [esp+58h] [ebp-128h]
    int rowWidth; // [esp+60h] [ebp-120h]
    float startKnotPos[2]; // [esp+64h] [ebp-11Ch] BYREF
    float knot[2]; // [esp+6Ch] [ebp-114h]
    int knotIndex; // [esp+74h] [ebp-10Ch]
    char text[260]; // [esp+78h] [ebp-108h] BYREF

    if (menu->childType != 3)
    {
        v2 = va("menu %s type %i", menu->label, menu->childType);
        MyAssertHandler(".\\devgui\\devgui.cpp", 1129, 0, "%s\n\t%s", "menu->childType == DEV_CHILD_GRAPH", v2);
    }
    graph = menu->child.graph;
    if (!graph)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1132, 0, "%s", "graph");
    if (!graph->knots)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1132, 0, "%s", "graph->knots");
    if (!graph->knotCount)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1132, 0, "%s", "graph->knotCount");
    if (graph->knotCountMax <= 0)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1132, 0, "%s", "graph->knotCountMax > 0");
    if (!graph)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1134, 0, "%s\n\t(menu->label) = %s", "(graph)", menu->label);
    rowWidth = devguiGlob.sliderWidth;
    rowHeight = DevGui_GetFontHeight();
    width = rowWidth + 8;
    LODWORD(x) = devguiGlob.left + (devguiGlob.right - devguiGlob.left - (rowWidth + 8)) / 2;
    HIDWORD(x) = devguiGlob.bottom - (3 * rowHeight + 16);
    DevGui_DrawBevelBox(
        x,
        rowWidth + 8,
        3 * rowHeight + 16,
        devgui_bevelShade->current.value,
        (const unsigned __int8 *)&devgui_colorBgnd->current);
    graphBottomY = (int)((double)SHIDWORD(x) * 0.949999988079071);
    DevGui_DrawBox(x, graphBottomY, rowWidth + 8, 2, (const unsigned __int8 *)&devgui_colorBgndSel->current);
    LODWORD(x) = x + 4;
    HIDWORD(x) += 6;
    DevGui_DrawSliderPath(x, SHIDWORD(x));
    for (knotIndex = 0; knotIndex < *graph->knotCount; ++knotIndex)
    {
        v5 = graph->knots[knotIndex];
        knot[0] = *v5;
        knot[1] = v5[1];
        knotX = x + (int)((double)width * knot[0]);
        knotY = graphBottomY - (int)((double)(graphBottomY - 48) * knot[1]);
        if (knotIndex == graph->selectedKnot)
        {
            if (devguiGlob.editingKnot)
                DevGui_DrawBoxCentered(knotX, knotY, 19, 19, (const unsigned __int8 *)&devgui_colorGraphKnotEditing->current);
            else
                DevGui_DrawBoxCentered(knotX, knotY, 16, 16, (const unsigned __int8 *)&devgui_colorGraphKnotSelected->current);
        }
        else
        {
            DevGui_DrawBoxCentered(knotX, knotY, 12, 12, (const unsigned __int8 *)&devgui_colorGraphKnotNormal->current);
        }
    }
    for (knotIndex = 0; knotIndex < *graph->knotCount - 1; ++knotIndex)
    {
        v4 = graph->knots[knotIndex];
        knot[0] = *v4;
        knot[1] = v4[1];
        nextKnot = graph->knots[knotIndex + 1][0];
        nextKnot_4 = graph->knots[knotIndex + 1][1];
        startKnotPos[0] = (double)width * knot[0] + (double)(int)x;
        startKnotPos[1] = (double)graphBottomY - (double)(graphBottomY - 48) * knot[1];
        endKnotPos[0] = (double)width * nextKnot + (double)(int)x;
        endKnotPos[1] = (double)graphBottomY - (double)(graphBottomY - 48) * nextKnot_4;
        DevGui_DrawLine(startKnotPos, endKnotPos, 2, (const unsigned __int8 *)&devgui_colorGraphKnotNormal->current);
    }
    HIDWORD(x) += rowHeight + 2;
    v3 = graph->knots[graph->selectedKnot];
    knot[0] = *v3;
    knot[1] = v3[1];
    if (graph->textCallback)
        ((void(__cdecl *)(DevGraph *, unsigned int, unsigned int, char *, int))graph->textCallback)(
            graph,
            LODWORD(knot[0]),
            LODWORD(knot[1]),
            text,
            256);
    else
        sprintf(text, "X: %.4f, Y: %.4f", knot[0], knot[1]);
    DevGui_DrawFont(x, SHIDWORD(x), (const unsigned __int8 *)&devgui_colorText->current, text);
    HIDWORD(x) += rowHeight + 2;
    DevGui_DrawFont(x, SHIDWORD(x), (const unsigned __int8 *)&devgui_colorText->current, (char *)MYINSTRUCTIONS);
    if (graph->eventCallback)
        graph->eventCallback(graph, EVENT_DRAW, localClientNum);
}

void __cdecl DevGui_Init()
{
    unsigned int menuIndex; // [esp+0h] [ebp-Ch]
    int screen_yPad; // [esp+4h] [ebp-8h]
    int screen_xPad; // [esp+8h] [ebp-4h]

    DevGui_RegisterDvars();
    screen_xPad = jpeg_mem_init();
    screen_yPad = jpeg_mem_init();
    for (menuIndex = 0; menuIndex < 0x257; ++menuIndex)
        *(unsigned int *)devguiGlob.menus[menuIndex].label = (unsigned int)&devguiGlob.menus[menuIndex + 1];
    *(unsigned int *)devguiGlob.menus[menuIndex].label = 0;
    devguiGlob.nextFreeMenu = (DevMenuItem *)&devguiGlob;
    devguiGlob.topmostMenu.childType = 0;
    devguiGlob.topmostMenu.childMenuMemory = 0;
    devguiGlob.topmostMenu.child.menu = 0;
    devguiGlob.topmostMenu.nextSibling = 0;
    devguiGlob.topmostMenu.prevSibling = 0;
    devguiGlob.topmostMenu.parent = 0;
    devguiGlob.isActive = 0;
    devguiGlob.editingMenuItem = 0;
    devguiGlob.selRow = -1;
    devguiGlob.editingKnot = 0;
    devguiGlob.left = screen_xPad;
    devguiGlob.right = DevGui_GetScreenWidth() - screen_xPad;
    devguiGlob.top = screen_yPad;
    devguiGlob.bottom = DevGui_GetScreenHeight() - screen_yPad;
    devguiGlob.sliderWidth = 3 * (devguiGlob.right - devguiGlob.left) / 4;
    DevGui_InputInit();
}

const dvar_s *DevGui_RegisterDvars()
{
    const dvar_s *result; // eax
    DvarLimits b; // [esp+8h] [ebp-10h]

    devgui_colorBgnd = Dvar_RegisterColor(
        "devgui_colorBgnd",
        0.0,
        0.40000001,
        0.0,
        0.75,
        0,
        "Color background for the devgui");
    devgui_colorText = Dvar_RegisterColor("devgui_colorText", 1.0, 1.0, 1.0, 1.0, 0, "Text color for the devgui");
    devgui_colorBgndSel = Dvar_RegisterColor(
        "devgui_colorBgndSel",
        0.0,
        0.69999999,
        0.0,
        0.75,
        0,
        "Selection background color for the devgui");
    devgui_colorTextSel = Dvar_RegisterColor(
        "devgui_colorTextSel",
        1.0,
        1.0,
        0.0,
        1.0,
        0,
        "Selection text color for the devgui");
    devgui_colorBgndGray = Dvar_RegisterColor(
        "devgui_colorBgndGray",
        0.2,
        0.2,
        0.2,
        0.89999998,
        0,
        "Grayed out background color for the devgui");
    devgui_colorTextGray = Dvar_RegisterColor(
        "devgui_colorTextGray",
        0.69999999,
        0.69999999,
        0.69999999,
        1.0,
        0,
        "Greyed out text color for the devgui");
    devgui_colorBgndGraySel = Dvar_RegisterColor(
        "devgui_colorBgndGraySel",
        0.40000001,
        0.40000001,
        0.40000001,
        0.89999998,
        0,
        "Greyed out, selected background color for the devgui");
    devgui_colorTextGraySel = Dvar_RegisterColor(
        "devgui_colorTextGraySel",
        1.0,
        1.0,
        0.0,
        1.0,
        0,
        "Greyed out, selected text color for the devgui");
    devgui_colorSliderBgnd = Dvar_RegisterColor(
        "devgui_colorSliderBgnd",
        1.0,
        1.0,
        1.0,
        0.75,
        0,
        "Color slider background for the devgui");
    devgui_colorSliderKnob = Dvar_RegisterColor(
        "devgui_colorSliderKnob",
        1.0,
        1.0,
        1.0,
        1.0,
        0,
        "Knob color for the devgui");
    devgui_colorSliderKnobSel = Dvar_RegisterColor(
        "devgui_colorSliderKnobSel",
        1.0,
        1.0,
        0.0,
        1.0,
        0,
        "Selected knob color for the devgui");
    b.value.max = 1.0;
    b.value.min = 0.0;
    devgui_bevelShade = Dvar_RegisterFloat("devgui_bevelShade", 0.69999999, b, 0, "Bevel shade for the devgui");
    devgui_colorGraphKnotNormal = Dvar_RegisterColor(
        "devgui_colorGraphKnotNormal",
        0.0,
        1.0,
        1.0,
        0.69999999,
        0,
        "Devgiu Color graph knot normal color");
    devgui_colorGraphKnotSelected = Dvar_RegisterColor(
        "devgui_colorGraphKnotSelected",
        1.0,
        0.0,
        0.0,
        0.69999999,
        0,
        "Devgui color graph knot selected color");
    result = Dvar_RegisterColor(
        "devgui_colorGraphKnotEditing",
        1.0,
        0.0,
        1.0,
        1.0,
        0,
        "Devgui color graph knot editing color");
    devgui_colorGraphKnotEditing = result;
    return result;
}

void __cdecl DevGui_Shutdown()
{
    DevGui_InputShutdown();
    DevGui_MenuShutdown();
}

void DevGui_MenuShutdown()
{
    DevGui_FreeMenu_r(devguiGlob.topmostMenu.child.menu);
}

void __cdecl DevGui_KeyPressed(int key)
{
    char *v1; // eax
    unsigned __int16 handle; // [esp+0h] [ebp-8Ch]
    char path[128]; // [esp+4h] [ebp-88h] BYREF
    DevMenuItem *menu; // [esp+88h] [ebp-4h]

    if (devguiGlob.bindNextKey)
    {
        devguiGlob.bindNextKey = 0;
        if (key != 27)
        {
            if (key == 9 || key == 167)
            {
                Com_Printf(11, "Can't rebind 'tab' or 'F1'\n");
            }
            else
            {
                handle = devguiGlob.selectedMenu;
                if (!devguiGlob.selectedMenu)
                    MyAssertHandler(".\\devgui\\devgui.cpp", 2035, 0, "%s", "handle");
                menu = (DevMenuItem *)DevGui_GetMenu(handle);
                if (!menu)
                    MyAssertHandler(".\\devgui\\devgui.cpp", 2037, 0, "%s", "menu");
                if (menu->parent && (!menu->childType || menu->childType == 1 && !devguiGlob.editingMenuItem))
                    handle = menu->parent;
                DevGui_GetSliderPath(handle, path, 0);
                v1 = va("devgui_open \"%s\"", path);
                Key_SetBinding(0, key, v1);
            }
        }
    }
}

void __cdecl DevGui_Update(int localClientNum, float deltaTime)
{
    devguiGlob_t *selMenuItem; // [esp+4h] [ebp-4h]

    if (devguiGlob.isActive && !devguiGlob.bindNextKey && DevGui_InputUpdate(localClientNum, deltaTime))
    {
        if (DevGui_IsButtonReleased(INPUT_BIND) && devguiGlob.selectedMenu)
        {
            devguiGlob.bindNextKey = 1;
        }
        else
        {
            selMenuItem = DevGui_GetMenu(devguiGlob.selectedMenu);
            if (!selMenuItem)
                MyAssertHandler(".\\devgui\\devgui.cpp", 2071, 0, "%s", "selMenuItem");
            if (devguiGlob.editingMenuItem && selMenuItem->menus[0].childType == 3)
            {
                DevGui_UpdateGraph(localClientNum, deltaTime);
            }
            else
            {
                if (devguiGlob.editingMenuItem)
                {
                    if (selMenuItem->menus[0].childType != 1)
                        MyAssertHandler(".\\devgui\\devgui.cpp", 2082, 0, "%s", "selMenuItem->childType == DEV_CHILD_DVAR");
                    DevGui_UpdateDvar(deltaTime);
                    DevGui_UpdateSelection();
                }
                else
                {
                    DevGui_MoveSelectionHorizontally();
                    DevGui_MoveSelectionVertically();
                }
                if (DevGui_IsButtonReleased(INPUT_ACCEPT))
                    DevGui_Accept(localClientNum);
                if (DevGui_IsButtonReleased(INPUT_REJECT))
                    DevGui_Reject();
            }
        }
    }
}

void DevGui_MoveSelectionHorizontally()
{
    int scroll; // [esp+0h] [ebp-4h]

    for (scroll = DevGui_GetMenuScroll(SCROLL_XAXIS); scroll < 0; ++scroll)
        DevGui_MoveLeft();
    while (scroll > 0)
    {
        DevGui_MoveRight();
        --scroll;
    }
}

void DevGui_MoveLeft()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if (!devguiGlob.selectedMenu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1358, 0, "%s", "devguiGlob.selectedMenu");
    for (menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        menu->menus[0].parent;
        menu = DevGui_GetMenu(devguiGlob.selectedMenu))
    {
        devguiGlob.selectedMenu = menu->menus[0].parent;
    }
    DevGui_SelectPrevMenuItem();
}

void DevGui_SelectPrevMenuItem()
{
    devguiGlob_t *v0; // [esp+0h] [ebp-Ch]
    devguiGlob_t *menu; // [esp+4h] [ebp-8h]

    if (!devguiGlob.selectedMenu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1287, 0, "%s", "devguiGlob.selectedMenu");
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (menu->menus[0].parent)
        v0 = DevGui_GetMenu(menu->menus[0].parent);
    else
        v0 = 0;
    if (menu->menus[0].prevSibling)
    {
        if (v0)
            --v0->menus[0].childMenuMemory;
        devguiGlob.selectedMenu = menu->menus[0].prevSibling;
    }
    else
    {
        if (v0)
            v0->menus[0].childMenuMemory = 0;
        while (menu->menus[0].nextSibling)
        {
            if (v0)
                ++v0->menus[0].childMenuMemory;
            devguiGlob.selectedMenu = menu->menus[0].nextSibling;
            menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        }
    }
    DevGui_SelectTopLevelChild();
}

void DevGui_SelectTopLevelChild()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1270, 0, "%s", "menu");
    if (!menu->menus[0].parent)
    {
        if (menu->menus[0].childType)
            MyAssertHandler(
                ".\\devgui\\devgui.cpp",
                1274,
                0,
                "%s\n\t(menu->childType) = %i",
                "(menu->childType == DEV_CHILD_MENU)",
                menu->menus[0].childType);
        if (!menu->menus[0].child.menu)
            MyAssertHandler(".\\devgui\\devgui.cpp", 1275, 0, "%s", "menu->child.menu");
        devguiGlob.selectedMenu = menu->menus[0].child.menu;
        DevGui_AdvanceChildNum(menu->menus[0].childMenuMemory);
    }
}

void __cdecl DevGui_AdvanceChildNum(int numberToAdvance)
{
    devguiGlob_t *menu; // [esp+0h] [ebp-8h]
    int numberIter; // [esp+4h] [ebp-4h]

    for (numberIter = 0; numberIter != numberToAdvance; ++numberIter)
    {
        menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        if (!menu->menus[0].nextSibling)
            break;
        devguiGlob.selectedMenu = menu->menus[0].nextSibling;
        DevGui_GetMenu(devguiGlob.selectedMenu);
    }
}

void DevGui_MoveRight()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if (!devguiGlob.selectedMenu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1373, 0, "%s", "devguiGlob.selectedMenu");
    for (menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        menu->menus[0].parent;
        menu = DevGui_GetMenu(devguiGlob.selectedMenu))
    {
        devguiGlob.selectedMenu = menu->menus[0].parent;
    }
    DevGui_SelectNextMenuItem();
}

void DevGui_SelectNextMenuItem()
{
    devguiGlob_t *menuParent; // [esp+0h] [ebp-8h]
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    if (!devguiGlob.selectedMenu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1322, 0, "%s", "devguiGlob.selectedMenu");
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (menu->menus[0].parent)
    {
        menuParent = DevGui_GetMenu(menu->menus[0].parent);
        if (menu->menus[0].nextSibling)
            ++menuParent->menus[0].childMenuMemory;
        else
            menuParent->menus[0].childMenuMemory = 0;
    }
    if (menu->menus[0].nextSibling)
    {
        devguiGlob.selectedMenu = menu->menus[0].nextSibling;
    }
    else
    {
        while (menu->menus[0].prevSibling)
        {
            devguiGlob.selectedMenu = menu->menus[0].prevSibling;
            menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        }
    }
    DevGui_SelectTopLevelChild();
}

void DevGui_MoveSelectionVertically()
{
    int scroll; // [esp+0h] [ebp-4h]

    for (scroll = DevGui_GetMenuScroll(SCROLL_YAXIS); scroll < 0; ++scroll)
        DevGui_MoveDown();
    while (scroll > 0)
    {
        DevGui_MoveUp();
        --scroll;
    }
}

void DevGui_MoveUp()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if (!devguiGlob.selectedMenu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1392, 0, "%s", "devguiGlob.selectedMenu");
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (!menu->menus[0].parent)
    {
        if (menu->menus[0].childType)
            MyAssertHandler(
                ".\\devgui\\devgui.cpp",
                1396,
                0,
                "%s\n\t(menu->childType) = %i",
                "(menu->childType == DEV_CHILD_MENU)",
                menu->menus[0].childType);
        devguiGlob.selectedMenu = menu->menus[0].child.menu;
        DevGui_GetMenu(devguiGlob.selectedMenu);
    }
    DevGui_SelectPrevMenuItem();
}

void DevGui_MoveDown()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if (!devguiGlob.selectedMenu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1409, 0, "%s", "devguiGlob.selectedMenu");
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (!menu->menus[0].parent)
    {
        if (menu->menus[0].childType)
            MyAssertHandler(
                ".\\devgui\\devgui.cpp",
                1413,
                0,
                "%s\n\t(menu->childType) = %i",
                "(menu->childType == DEV_CHILD_MENU)",
                menu->menus[0].childType);
        devguiGlob.selectedMenu = menu->menus[0].child.menu;
        DevGui_GetMenu(devguiGlob.selectedMenu);
    }
    DevGui_SelectNextMenuItem();
}

void __cdecl DevGui_Accept(int localClientNum)
{
    devguiGlob_t *menu; // [esp+4h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1463, 0, "%s", "menu");
    switch (menu->menus[0].childType)
    {
    case 0u:
        devguiGlob.selectedMenu = menu->menus[0].child.menu;
        DevGui_AdvanceChildNum(menu->menus[0].childMenuMemory);
        break;
    case 1u:
        if (DevGui_EditableMenuItem(menu->menus))
        {
            if (devguiGlob.editingMenuItem)
            {
                Dvar_MakeLatchedValueCurrent((dvar_s *)menu->menus[0].child.command);
                Dvar_SetModified((dvar_s*)menu->menus[0].child.dvar);
            }
            devguiGlob.editingMenuItem = !devguiGlob.editingMenuItem;
            devguiGlob.selRow = 0;
        }
        break;
    case 2u:
        Cbuf_InsertText(0, (char *)menu->menus[0].child.command);
        break;
    case 3u:
        devguiGlob.editingMenuItem = !devguiGlob.editingMenuItem;
        devguiGlob.selRow = 0;
        if (menu->menus[0].child.command && *((unsigned int *)menu->menus[0].child.command + 4))
            (*((void(__cdecl **)(DevMenuChild, unsigned int, int))menu->menus[0].child.command + 4))(
                menu->menus[0].child,
                0,
                localClientNum);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\devgui\\devgui.cpp", 1497, 1, "unhandled case");
        break;
    }
}

void DevGui_Reject()
{
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    if (!devguiGlob.selectedMenu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1507, 0, "%s", "devguiGlob.selectedMenu");
    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (devguiGlob.editingMenuItem)
    {
        Dvar_ClearLatchedValue((dvar_s *)menu->menus[0].child.command);
        devguiGlob.editingMenuItem = 0;
    }
    else if (menu->menus[0].parent)
    {
        if (DevGui_GetMenu(menu->menus[0].parent)->menus[0].parent)
            devguiGlob.selectedMenu = menu->menus[0].parent;
    }
}

int DevGui_UpdateSelection()
{
    int result; // eax
    int scroll; // [esp+0h] [ebp-4h]

    result = DevGui_GetMenuScroll(SCROLL_YAXIS);
    for (scroll = (__int16)result; scroll < 0; ++scroll)
        result = DevGui_ScrollDown();
    while (scroll > 0)
    {
        result = DevGui_ScrollUp();
        --scroll;
    }
    return result;
}

int DevGui_ScrollUp()
{
    int result; // eax
    int rowCount; // [esp+0h] [ebp-Ch]
    devguiGlob_t *menu; // [esp+4h] [ebp-8h]
    const dvar_s *dvar; // [esp+8h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (menu->menus[0].childType == 3)
        return DevGui_ScrollUpInternal();
    if (menu->menus[0].childType != 1)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1599, 0, "%s", "menu->childType == DEV_CHILD_DVAR");
    dvar = DevGui_SelectedDvar();
    if (!dvar)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1602, 0, "%s", "dvar");
    result = DevGui_DvarRowCount(dvar);
    rowCount = result;
    if (result == 1)
        return DevGui_ScrollUpInternal();
    if (devguiGlob.selRow)
    {
        --devguiGlob.selRow;
    }
    else
    {
        --result;
        devguiGlob.selRow = rowCount - 1;
    }
    return result;
}

const dvar_s *__cdecl DevGui_SelectedDvar()
{
    const char *v0; // eax
    devguiGlob_t *menu; // [esp+0h] [ebp-8h]
    const dvar_s *dvar; // [esp+4h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1565, 0, "%s", "menu");
    if (menu->menus[0].childType != 1)
    {
        v0 = va("menu %s type %i", (const char *)menu, menu->menus[0].childType);
        MyAssertHandler(".\\devgui\\devgui.cpp", 1566, 0, "%s\n\t%s", "menu->childType == DEV_CHILD_DVAR", v0);
    }
    dvar = menu->menus[0].child.dvar;
    if (!dvar)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1568, 0, "%s\n\t(menu->label) = %s", "(dvar)", (const char *)menu);
    return dvar;
}

int DevGui_ScrollUpInternal()
{
    int result; // eax
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    do
    {
        DevGui_MoveUp();
        menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        result = DevGui_EditableMenuItem(menu->menus);
    } while (!result);
    return result;
}

int DevGui_ScrollDown()
{
    int result; // eax
    int rowCount; // [esp+0h] [ebp-Ch]
    devguiGlob_t *menu; // [esp+4h] [ebp-8h]
    const dvar_s *dvar; // [esp+8h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (menu->menus[0].childType == 3)
        return DevGui_ScrollDownInternal();
    if (menu->menus[0].childType != 1)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1642, 0, "%s", "menu->childType == DEV_CHILD_DVAR");
    dvar = DevGui_SelectedDvar();
    if (!dvar)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1645, 0, "%s", "dvar");
    rowCount = DevGui_DvarRowCount(dvar);
    if (rowCount == 1)
        return DevGui_ScrollDownInternal();
    result = ++devguiGlob.selRow;
    if (devguiGlob.selRow == rowCount)
        devguiGlob.selRow = 0;
    return result;
}

int DevGui_ScrollDownInternal()
{
    int result; // eax
    devguiGlob_t *menu; // [esp+0h] [ebp-4h]

    do
    {
        DevGui_MoveDown();
        menu = DevGui_GetMenu(devguiGlob.selectedMenu);
        result = DevGui_EditableMenuItem(menu->menus);
    } while (!result);
    return result;
}

void __cdecl DevGui_UpdateDvar(float deltaTime)
{
    const char *v1; // eax
    float floatValue; // [esp+38h] [ebp-34h]
    float floatValuea; // [esp+38h] [ebp-34h]
    bool boolValue; // [esp+3Fh] [ebp-2Dh]
    float step; // [esp+40h] [ebp-2Ch]
    float stepa; // [esp+40h] [ebp-2Ch]
    float stepb; // [esp+40h] [ebp-2Ch]
    float stepc; // [esp+40h] [ebp-2Ch]
    float vector[4]; // [esp+44h] [ebp-28h] BYREF
    const dvar_s *dvar; // [esp+54h] [ebp-18h]
    float color[4]; // [esp+58h] [ebp-14h] BYREF
    int intValue; // [esp+68h] [ebp-4h]

    dvar = DevGui_SelectedDvar();
    if (!dvar)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1715, 0, "%s", "dvar");
    switch (dvar->type)
    {
    case 0u:
        boolValue = DevGui_UpdateIntScroll(deltaTime, dvar->latched.color[0] != 0, 0, 1, SCROLL_XAXIS) != 0;
        if (boolValue != dvar->latched.enabled)
            Dvar_SetBoolFromSource((dvar_s *)dvar, boolValue, DVAR_SOURCE_DEVGUI);
        break;
    case 1u:
        step = DevGui_PickFloatScrollStep(dvar->domain.value.min, dvar->domain.value.max);
        floatValue = DevGui_UpdateFloatScroll(
            deltaTime,
            dvar->latched.value,
            dvar->domain.value.min,
            dvar->domain.value.max,
            step,
            SCROLL_XAXIS);
        if (dvar->latched.value != floatValue)
            Dvar_SetFloatFromSource((dvar_s *)dvar, floatValue, DVAR_SOURCE_DEVGUI);
        break;
    case 2u:
        stepa = DevGui_PickFloatScrollStep(dvar->domain.value.min, dvar->domain.value.max);
        vector[0] = dvar->latched.value;
        vector[1] = dvar->latched.vector[1];
        vector[devguiGlob.selRow] = DevGui_UpdateFloatScroll(
            deltaTime,
            vector[devguiGlob.selRow],
            dvar->domain.value.min,
            dvar->domain.value.max,
            stepa,
            SCROLL_XAXIS);
        if (vector[0] != dvar->latched.value || vector[1] != dvar->latched.vector[1])
            Dvar_SetVec2FromSource((dvar_s *)dvar, vector[0], vector[1], DVAR_SOURCE_DEVGUI);
        break;
    case 3u:
        stepb = DevGui_PickFloatScrollStep(dvar->domain.value.min, dvar->domain.value.max);
        vector[0] = dvar->latched.value;
        vector[1] = dvar->latched.vector[1];
        vector[2] = dvar->latched.vector[2];
        vector[devguiGlob.selRow] = DevGui_UpdateFloatScroll(
            deltaTime,
            vector[devguiGlob.selRow],
            dvar->domain.value.min,
            dvar->domain.value.max,
            stepb,
            SCROLL_XAXIS);
        if (vector[0] != dvar->latched.value
            || vector[1] != dvar->latched.vector[1]
            || vector[2] != dvar->latched.vector[2])
        {
            Dvar_SetVec3FromSource((dvar_s *)dvar, vector[0], vector[1], vector[2], DVAR_SOURCE_DEVGUI);
        }
        break;
    case 4u:
        stepc = DevGui_PickFloatScrollStep(dvar->domain.value.min, dvar->domain.value.max);
        vector[0] = dvar->latched.value;
        vector[1] = dvar->latched.vector[1];
        vector[2] = dvar->latched.vector[2];
        vector[3] = dvar->latched.vector[3];
        vector[devguiGlob.selRow] = DevGui_UpdateFloatScroll(
            deltaTime,
            vector[devguiGlob.selRow],
            dvar->domain.value.min,
            dvar->domain.value.max,
            stepc,
            SCROLL_XAXIS);
        if (!Vec4Compare(&dvar->latched.value, vector))
            Dvar_SetVec4FromSource((dvar_s *)dvar, vector[0], vector[1], vector[2], vector[3], DVAR_SOURCE_DEVGUI);
        break;
    case 5u:
        intValue = DevGui_UpdateIntScroll(
            deltaTime,
            dvar->latched.integer,
            dvar->domain.enumeration.stringCount,
            dvar->domain.integer.max,
            SCROLL_XAXIS);
        if (intValue != dvar->latched.integer)
            Dvar_SetIntFromSource((dvar_s *)dvar, intValue, DVAR_SOURCE_DEVGUI);
        break;
    case 6u:
        intValue = DevGui_UpdateIntScroll(
            deltaTime,
            dvar->latched.integer,
            0,
            dvar->domain.enumeration.stringCount - 1,
            SCROLL_XAXIS);
        if (intValue != dvar->latched.integer)
            Dvar_SetIntFromSource((dvar_s *)dvar, intValue, DVAR_SOURCE_DEVGUI);
        break;
    case 8u:
        Byte4UnpackRgba((const unsigned __int8 *)&dvar->latched, color);
        floatValuea = color[devguiGlob.selRow];
        color[devguiGlob.selRow] = DevGui_UpdateFloatScroll(deltaTime, floatValuea, 0.0, 1.0, 0.019607844, SCROLL_XAXIS);
        if (color[devguiGlob.selRow] != floatValuea)
            Dvar_SetColorFromSource((dvar_s *)dvar, color[0], color[1], color[2], color[3], DVAR_SOURCE_DEVGUI);
        break;
    default:
        if (!alwaysfails)
        {
            v1 = va("invalid dvar type %i", dvar->type);
            MyAssertHandler(".\\devgui\\devgui.cpp", 1780, 1, v1);
        }
        break;
    }
}

bool __cdecl Vec4Compare(const float *a, const float *b)
{
    return *b == *a && b[1] == a[1] && b[2] == a[2] && b[3] == a[3];
}

double __cdecl DevGui_PickFloatScrollStep(float min, float max)
{
    float v3; // [esp+0h] [ebp-44h]
    float v4; // [esp+4h] [ebp-40h]
    float range; // [esp+38h] [ebp-Ch]
    float step; // [esp+3Ch] [ebp-8h]
    float roundedStep; // [esp+40h] [ebp-4h]

    range = max - min;
    if (max == (double)(int)(max + 9.313225746154785e-10) && min == (double)(int)(min + 9.313225746154785e-10))
    {
        for (step = 1.0; range > step * 100.0; step = step + step)
            ;
        while (range < step * 100.0)
            step = step * 0.5;
    }
    else
    {
        step = range * 0.009999999776482582;
        roundedStep = (float)(int)(step + 9.313225746154785e-10);
        if (roundedStep != 0.0)
        {
            v4 = roundedStep - step;
            v3 = fabs(v4);
            if (v3 < 0.1000000014901161)
                return (float)(int)(step + 9.313225746154785e-10);
        }
    }
    return step;
}

void __cdecl DevGui_UpdateGraph(int localClientNum, float deltaTime)
{
    float v2; // [esp+18h] [ebp-A0h]
    float v3; // [esp+1Ch] [ebp-9Ch]
    float v4; // [esp+20h] [ebp-98h]
    float v5; // [esp+28h] [ebp-90h]
    float v6; // [esp+2Ch] [ebp-8Ch]
    float v7; // [esp+30h] [ebp-88h]
    float v8; // [esp+34h] [ebp-84h]
    float v9; // [esp+38h] [ebp-80h]
    float v10; // [esp+3Ch] [ebp-7Ch]
    float v11; // [esp+40h] [ebp-78h]
    bool v12; // [esp+44h] [ebp-74h]
    float v13; // [esp+48h] [ebp-70h]
    float v14; // [esp+4Ch] [ebp-6Ch]
    float v15; // [esp+50h] [ebp-68h]
    float v16; // [esp+54h] [ebp-64h]
    float v17; // [esp+58h] [ebp-60h]
    float v18; // [esp+5Ch] [ebp-5Ch]
    float v19; // [esp+60h] [ebp-58h]
    float v20; // [esp+64h] [ebp-54h]
    float v21; // [esp+68h] [ebp-50h]
    float v22; // [esp+6Ch] [ebp-4Ch]
    float *v23; // [esp+74h] [ebp-44h]
    float *v24; // [esp+78h] [ebp-40h]
    __int16 xAxisDelta; // [esp+7Ch] [ebp-3Ch]
    bool graphUpdated; // [esp+83h] [ebp-35h]
    int currentKnotCount; // [esp+84h] [ebp-34h]
    DevGraph *graph; // [esp+90h] [ebp-28h]
    float deltaX; // [esp+94h] [ebp-24h]
    devguiGlob_t *menu; // [esp+98h] [ebp-20h]
    float updatedY; // [esp+9Ch] [ebp-1Ch]
    float updatedX; // [esp+A8h] [ebp-10h]
    float knot; // [esp+ACh] [ebp-Ch]
    float knot_4; // [esp+B0h] [ebp-8h]
    float deltaY; // [esp+B4h] [ebp-4h]
    float deltaYa; // [esp+B4h] [ebp-4h]

    menu = DevGui_GetMenu(devguiGlob.selectedMenu);
    if (!menu)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1882, 0, "%s", "menu");
    if (menu->menus[0].childType != 3)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1883, 0, "%s", "menu->childType == DEV_CHILD_GRAPH");
    if (!menu->menus[0].child.command)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1884, 0, "%s", "menu->child.graph");
    graph = menu->menus[0].child.graph;
    if (!graph)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1888, 0, "%s", "graph");
    if (!graph->knots)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1888, 0, "%s", "graph->knots");
    if (!graph->knotCount)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1888, 0, "%s", "graph->knotCount");
    if (graph->knotCountMax <= 0)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1888, 0, "%s", "graph->knotCountMax > 0");
    currentKnotCount = *graph->knotCount;
    if (currentKnotCount < 2 || currentKnotCount > graph->knotCountMax)
        MyAssertHandler(
            ".\\devgui\\devgui.cpp",
            1891,
            0,
            "%s",
            "currentKnotCount >= 2 && currentKnotCount <= graph->knotCountMax");
    if (graph->selectedKnot < 0 || graph->selectedKnot >= currentKnotCount)
        graph->selectedKnot = 0;
    if (DevGui_IsButtonPressed(INPUT_GRAPH_EDIT))
        devguiGlob.editingKnot = !devguiGlob.editingKnot;
    if (DevGui_IsButtonPressed(INPUT_GRAPH_ADD))
    {
        DevGui_AddGraphKnot(graph, localClientNum);
        devguiGlob.editingKnot = 0;
    }
    else if (DevGui_IsButtonPressed(INPUT_GRAPH_REMOVE))
    {
        DevGui_RemoveGraphKnot(graph, localClientNum);
        devguiGlob.editingKnot = 0;
    }
    else if (DevGui_IsButtonPressed(INPUT_GRAPH_SAVE))
    {
        if (graph->eventCallback)
            graph->eventCallback(graph, EVENT_SAVE, localClientNum);
    }
    else if (DevGui_IsButtonPressed(INPUT_ACCEPT))
    {
        if (graph->eventCallback)
            graph->eventCallback(graph, EVENT_ACCEPT, localClientNum);
    }
    else if (DevGui_IsButtonPressed(INPUT_REJECT))
    {
        if (graph->eventCallback)
            graph->eventCallback(graph, EVENT_DEACTIVATE, localClientNum);
        devguiGlob.editingMenuItem = 0;
    }
    else if (devguiGlob.editingKnot)
    {
        v24 = graph->knots[graph->selectedKnot];
        knot = *v24;
        knot_4 = v24[1];
        updatedX = DevGui_UpdateFloatScroll(deltaTime, knot, 0.0, 1.0, 0.029999999, SCROLL_XAXIS);
        updatedY = DevGui_UpdateFloatScroll(deltaTime, knot_4, 0.0, 1.0, 0.029999999, SCROLL_YAXIS);
        if (graph->selectedKnot <= 0 || graph->selectedKnot + 1 >= currentKnotCount)
        {
            if (graph->disableEditingEndPoints)
            {
                graphUpdated = 0;
            }
            else
            {
                deltaYa = updatedY - knot_4;
                v5 = fabs(deltaYa);
                graphUpdated = v5 > 0.0000009999999974752427;
                graph->knots[graph->selectedKnot][1] = deltaYa * 4.0 + graph->knots[graph->selectedKnot][1];
                v15 = graph->knots[graph->selectedKnot][1];
                v4 = v15 - 1.0;
                if (v4 < 0.0)
                    v16 = v15;
                else
                    v16 = 1.0;
                v3 = 0.0 - v15;
                if (v3 < 0.0)
                    v2 = v16;
                else
                    v2 = 0.0;
                graph->knots[graph->selectedKnot][1] = v2;
            }
        }
        else
        {
            deltaX = updatedX - knot;
            deltaY = updatedY - knot_4;
            v14 = fabs(deltaX);
            v12 = 1;
            if (v14 <= 0.0000009999999974752427)
            {
                v13 = fabs(deltaY);
                if (v13 <= 0.0000009999999974752427)
                    v12 = 0;
            }
            graphUpdated = v12;
            graph->knots[graph->selectedKnot][0] = deltaX * 3.0 + graph->knots[graph->selectedKnot][0];
            graph->knots[graph->selectedKnot][1] = deltaY * 4.0 + graph->knots[graph->selectedKnot][1];
            v23 = graph->knots[graph->selectedKnot - 1];
            v19 = graph->knots[graph->selectedKnot][0];
            v21 = graph->knots[graph->selectedKnot + 1][0] - 0.004999999888241291;
            v11 = v19 - v21;
            if (v11 < 0.0)
                v22 = v19;
            else
                v22 = graph->knots[graph->selectedKnot + 1][0] - 0.004999999888241291;
            v20 = *v23 + 0.004999999888241291;
            v10 = v20 - v19;
            if (v10 < 0.0)
                v9 = v22;
            else
                v9 = *v23 + 0.004999999888241291;
            graph->knots[graph->selectedKnot][0] = v9;
            v17 = graph->knots[graph->selectedKnot][1];
            v8 = v17 - 1.0;
            if (v8 < 0.0)
                v18 = v17;
            else
                v18 = 1.0;
            v7 = 0.0 - v17;
            if (v7 < 0.0)
                v6 = v18;
            else
                v6 = 0.0;
            graph->knots[graph->selectedKnot][1] = v6;
        }
        if (graphUpdated && graph->eventCallback)
            graph->eventCallback(graph, EVENT_UPDATE, localClientNum);
    }
    else
    {
        xAxisDelta = DevGui_GetMenuScroll(SCROLL_XAXIS);
        if (xAxisDelta >= 0 || graph->selectedKnot <= 0)
        {
            if (xAxisDelta > 0 && currentKnotCount > graph->selectedKnot + 1)
                ++graph->selectedKnot;
        }
        else
        {
            --graph->selectedKnot;
        }
        DevGui_UpdateSelection();
    }
}

void __cdecl DevGui_AddGraphKnot(DevGraph *graph, int localClientNum)
{
    float *v2; // [esp+4h] [ebp-20h]
    float *v3; // [esp+Ch] [ebp-18h]
    float *v4; // [esp+10h] [ebp-14h]
    float averageX; // [esp+14h] [ebp-10h]
    float averageXa; // [esp+14h] [ebp-10h]
    int currentKnotCount; // [esp+18h] [ebp-Ch]
    int knotIndex; // [esp+1Ch] [ebp-8h]
    float averageY; // [esp+20h] [ebp-4h]
    float averageYa; // [esp+20h] [ebp-4h]

    if (!graph)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1793, 0, "%s", "graph");
    if (!graph->knots)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1793, 0, "%s", "graph->knots");
    if (!graph->knotCount)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1793, 0, "%s", "graph->knotCount");
    if (graph->knotCountMax <= 0)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1793, 0, "%s", "graph->knotCountMax > 0");
    if (graph->selectedKnot < 0)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1794, 0, "%s", "graph->selectedKnot >= 0");
    if (graph->selectedKnot >= *graph->knotCount)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1795, 0, "%s", "graph->selectedKnot < *graph->knotCount");
    if (*graph->knotCount == graph->knotCountMax)
    {
        Com_Printf(11, "^3Maximum number of knots have reached for this graph\n");
    }
    else
    {
        currentKnotCount = *graph->knotCount;
        if (graph->selectedKnot + 1 >= currentKnotCount)
        {
            averageXa = (graph->knots[graph->selectedKnot][0] + graph->knots[graph->selectedKnot - 1][0]) * 0.5;
            averageYa = (graph->knots[graph->selectedKnot][1] + graph->knots[graph->selectedKnot - 1][1]) * 0.5;
            v2 = graph->knots[currentKnotCount];
            *v2 = *(v2 - 2);
            v2[1] = *(v2 - 1);
            graph->knots[currentKnotCount - 1][0] = averageXa;
            graph->knots[currentKnotCount - 1][1] = averageYa;
            ++*graph->knotCount;
        }
        else
        {
            averageX = (graph->knots[graph->selectedKnot][0] + graph->knots[graph->selectedKnot + 1][0]) * 0.5;
            averageY = (graph->knots[graph->selectedKnot][1] + graph->knots[graph->selectedKnot + 1][1]) * 0.5;
            for (knotIndex = *graph->knotCount - 1; knotIndex >= graph->selectedKnot; --knotIndex)
            {
                v3 = graph->knots[knotIndex + 1];
                v4 = graph->knots[knotIndex];
                *v3 = *v4;
                v3[1] = v4[1];
            }
            graph->knots[graph->selectedKnot + 1][0] = averageX;
            graph->knots[++graph->selectedKnot][1] = averageY;
            ++*graph->knotCount;
        }
        if (graph->eventCallback)
            graph->eventCallback(graph, EVENT_UPDATE, localClientNum);
    }
}

void __cdecl DevGui_RemoveGraphKnot(DevGraph *graph, int localClientNum)
{
    float *v2; // [esp+0h] [ebp-10h]
    int currentKnotCount; // [esp+8h] [ebp-8h]
    int knotIndex; // [esp+Ch] [ebp-4h]

    if (!graph)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1843, 0, "%s", "graph");
    if (!graph->knots)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1843, 0, "%s", "graph->knots");
    if (!graph->knotCount)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1843, 0, "%s", "graph->knotCount");
    if (graph->knotCountMax <= 0)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1843, 0, "%s", "graph->knotCountMax > 0");
    if (graph->selectedKnot < 0)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1845, 0, "%s", "graph->selectedKnot >= 0");
    if (graph->selectedKnot >= *graph->knotCount)
        MyAssertHandler(".\\devgui\\devgui.cpp", 1846, 0, "%s", "graph->selectedKnot < *graph->knotCount");
    currentKnotCount = *graph->knotCount;
    if (graph->selectedKnot && graph->selectedKnot + 1 != currentKnotCount)
    {
        for (knotIndex = graph->selectedKnot; knotIndex < currentKnotCount - 1; ++knotIndex)
        {
            v2 = graph->knots[knotIndex];
            *v2 = v2[2];
            v2[1] = v2[3];
        }
        graph->knots[currentKnotCount - 1][0] = -1.0;
        graph->knots[currentKnotCount - 1][1] = -1.0;
        --*graph->knotCount;
        if (graph->eventCallback)
            graph->eventCallback(graph, EVENT_UPDATE, localClientNum);
    }
}

void __cdecl DevGui_Toggle()
{
    if (devguiGlob.topmostMenu.child.menu)
    {
        if (devguiGlob.selectedMenu)
            goto LABEL_10;
        if (devguiGlob.isActive)
            MyAssertHandler(".\\devgui\\devgui.cpp", 2113, 0, "%s", "!devguiGlob.isActive");
        devguiGlob.selectedMenu = devguiGlob.topmostMenu.child.menu;
        if (devguiGlob.topmostMenu.child.menu)
        {
            DevGui_SelectTopLevelChild();
            if (devguiGlob.selectedMenu)
                LABEL_10:
            devguiGlob.isActive = !devguiGlob.isActive;
        }
    }
    else if (devguiGlob.isActive)
    {
        MyAssertHandler(".\\devgui\\devgui.cpp", 2107, 0, "%s", "!devguiGlob.isActive");
    }
}

bool __cdecl DevGui_IsActive()
{
    return devguiGlob.isActive;
}

