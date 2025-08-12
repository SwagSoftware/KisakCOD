#include "ui_shared.h"
#include <win32/win_net_debug.h>
#include <script/scr_parser.h>
#include <script/scr_main.h>
#include <script/scr_evaluate.h>
#include <script/scr_parsetree.h>
#include <script/scr_compiler.h>
#include <script/scr_vm.h>
#include <universal/com_files.h>
#include <client/client.h>

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#elif KISAK_SP
#include "ui.h"
// LWSS: helper function (SP lacks the 1st arg)
static void CL_LookupColor(int localClientNum, int num, float *color)
{
    CL_LookupColor(num, color);
}
#endif

#undef DrawText

UI_Component_data_t UI_Component::g;
UI_Component* UI_Component::selectionComp = NULL;


bool __cdecl Scr_ElementChildrenExist(Scr_WatchElement_s *element)
{
    if (element->threadList || element->endonList)
        return 1;
    return element->childHead && element->expand && element->objectType && element->objectType < 0x16u;
}

void __cdecl UI_Component::InitAssets()
{
    memset(&UI_Component::g, 0, sizeof(UI_Component::g));
    UI_Component::g.screenWidth = cls.vidConfig.displayWidth;
    UI_Component::g.screenHeight = cls.vidConfig.displayHeight;
    UI_Component::g.charWidth = 12.0;
    UI_Component::g.charHeight = 16.0;
    UI_Component::g.scrollBarSize = 16.0;
    if (UI_Component::g.screenWidth == 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 59, 0, "%s", "g.screenWidth");
    if (UI_Component::g.screenHeight == 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 60, 0, "%s", "g.screenHeight");
    UI_Component::g.cursor = UI_Component::RegisterMaterialNoMip((char*)"ui_cursor", 1);
    UI_Component::g.filledCircle = UI_Component::RegisterMaterialNoMip((char*)"ui_sliderbutt_1", 1);
}

void UI_Component::DrawText(float x, float y, float width, int fontEnum, const float *color, char *text)
{
    float v6; // [esp+1Ch] [ebp-10h]
    float v7; // [esp+20h] [ebp-Ch]
    float v8; // [esp+24h] [ebp-8h]
    int maxChars; // [esp+28h] [ebp-4h]

    if (UI_Component::g.charWidth == 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 132, 0, "%s", "g.charWidth");
    maxChars = (width / UI_Component::g.charWidth);
    v7 = floor(x);
    v8 = UI_Component::g.charHeight + y;
    v6 = floor(v8);
    R_AddCmdDrawText(text, maxChars, cls.consoleFont, v7, v6, 1.0, 1.0, 0.0, color, 0);
}

Material *__cdecl UI_Component::RegisterMaterialNoMip(char *name, int imageTrack)
{
    return Material_RegisterHandle(name, imageTrack);
}

void UI_Component::MouseEvent(int x, int y)
{
    UI_Component::g.hideCursor = 0;
    if (x < 0 || UI_Component::g.screenWidth <= x)
        UI_Component::g.hideCursor = 1;
    if (y < 0 || UI_Component::g.screenHeight <= y)
        UI_Component::g.hideCursor = 1;
    if (!UI_Component::g.hideCursor)
    {
        UI_Component::g.cursorPos[0] = x;
        UI_Component::g.cursorPos[1] = y;
    }
}

void __cdecl UI_Component_Init()
{
	UI_Component::InitAssets();
}

char *__cdecl Scr_GetElementArchiveText(Scr_WatchElement_s *element)
{
    char *result; // eax

    switch (element->breakpointType)
    {
    case 1u:
        result = va("%s%s", "@", element->refText);
        break;
    case 2u:
        result = va("%s%s", "?", element->refText);
        break;
    case 3u:
        result = va("%s%s", "#", element->refText);
        break;
    case 4u:
        result = va("%s%s", "##", element->refText);
        break;
    case 5u:
        result = va("%s%s", "#@", element->refText);
        break;
    case 6u:
        result = va("%s%s", "+", element->refText);
        break;
    case 7u:
        result = va("%s%s", "-", element->refText);
        break;
    default:
        result = va("%s%s", "", element->refText);
        break;
    }
    return result;
}

void Scr_ScriptWatch::Shutdown()
{
    Scr_WatchElement_s *next; // [esp+14h] [ebp-Ch]
    int f; // [esp+18h] [ebp-8h]
    char *text; // [esp+1Ch] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
    {
        Scr_ScriptWatch::UpdateBreakpoints(0);
        Scr_UnbreakAllAssignmentPos();
    }
    f = FS_FOpenTextFileWrite("watch_window.txt");
    while (this->elementHead)
    {
        next = this->elementHead->next;
        if (f)
        {
            text = Scr_GetElementArchiveText(this->elementHead);
            FS_Write(text, strlen(text), f);
            FS_Write("\n", 1u, f);
        }
        Scr_ScriptWatch::FreeWatchElement(this->elementHead);
        this->elementHead = next;
    }
    if (f)
        FS_FCloseFile(f);
}

void Scr_ScriptWatch::Draw(
    float x,
    float y,
    float width,
    float height,
    float compX,
    float compY)
{
    int currentLine; // [esp+30h] [ebp-14h] BYREF
    int startLine; // [esp+34h] [ebp-10h]
    float startLineFrac; // [esp+38h] [ebp-Ch]
    float currentY; // [esp+3Ch] [ebp-8h] BYREF
    float lastHeight; // [esp+40h] [ebp-4h]

    UI_Component::DrawPic(x, y, width, height, 0, cls.consoleMaterial);
    startLineFrac = compY / UI_Component::g.charHeight;
    startLine = startLineFrac;
    currentY = -startLineFrac;
    currentY = currentY * UI_Component::g.charHeight;
    currentLine = 0;
    lastHeight = height - UI_Component::g.charHeight;
    Scr_ScriptWatch::Draw_r(
        this->elementHead,
        x,
        y,
        width,
        lastHeight,
        startLine,
        0,
        0,
        &currentLine,
        &currentY,
        compX,
        compY);
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::CloneElement(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s *ElementRoot; // eax
    Scr_WatchElement_s **ElementRef; // eax
    Scr_WatchElement_s *newElement; // [esp+4h] [ebp-4h]

    ElementRoot = Scr_GetElementRoot(element);
    ElementRef = Scr_ScriptWatch::GetElementRef(ElementRoot);
    newElement = Scr_ScriptWatch::CreateWatchElement((char*)element->refText, ElementRef, "Scr_ScriptWatch::CloneElement");
    if (!Sys_IsRemoteDebugClient())
        Scr_CompileText(element->refText, &newElement->expr);
    return newElement;
}

void __cdecl Scr_PrintElementText(Scr_WatchElement_s *element, int bufLen, int depth, char *buf)
{
    unsigned __int8 objectType; // [esp+30h] [ebp-10h]
    int threadIdSize; // [esp+34h] [ebp-Ch]
    int len; // [esp+38h] [ebp-8h]
    int lena; // [esp+38h] [ebp-8h]
    signed int lenb; // [esp+38h] [ebp-8h]
    int entIdSize; // [esp+3Ch] [ebp-4h]

    if (element->breakpoint)
    {
        if (depth)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 3331, 0, "%s", "!depth");
        I_strncpyz(buf + 1, element->valueText, bufLen - 1);
        return;
    }
    if (element->threadList || element->endonList)
    {
        I_strncpyz(&buf[depth + 1], element->refText, bufLen - (depth + 1));
        return;
    }
    if (!element->directObject)
    {
    LABEL_19:
        lenb = strlen(element->refText);
        if (lenb >= 32 - depth)
        {
            Com_sprintf(&buf[depth + 1], bufLen - (depth + 1), "%s = %s", element->refText, element->valueText);
        }
        else
        {
            if (bufLen < 33)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 3377, 0, "%s", "1 + SCRIPT_WATCH_TEXT_MIN_DISPLAY_LEN <= bufLen");
            if (depth + lenb + 1 > bufLen)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 3378, 0, "%s", "depth + 1 + len <= bufLen");
            memset(&buf[depth + 1], 0x20u, 32 - depth);
            memcpy(&buf[depth + 1], element->refText, lenb);
            Com_sprintf(buf + 33, bufLen - 33, " = %s", element->valueText);
        }
        return;
    }
    objectType = element->objectType;
    if (objectType == 14)
        goto LABEL_12;
    if (objectType != 20)
    {
        if (objectType == 22)
        {
        LABEL_12:
            I_strncpyz(&buf[depth + 1], element->refText, bufLen - (depth + 1));
            if (element->bufferIndex != -1)
            {
                threadIdSize = 8;
                len = strlen(&buf[depth + 1]);
                if (len >= 8)
                    threadIdSize = len + 1;
                memset(&buf[depth + 1 + len], 0x20u, threadIdSize - len);
                Scr_GetSourcePos(
                    element->bufferIndex,
                    element->sourcePos,
                    &buf[depth + 1 + threadIdSize],
                    bufLen - (depth + threadIdSize + 1));
                buf[bufLen - 1] = 0;
            }
            return;
        }
        goto LABEL_19;
    }
    I_strncpyz(&buf[depth + 1], element->refText, bufLen - (depth + 1));
    entIdSize = 7;
    lena = strlen(&buf[depth + 1]);
    if (lena >= 7)
        entIdSize = lena + 1;
    memset(&buf[depth + 1 + lena], 0x20u, entIdSize - lena);
    I_strncpyz(&buf[depth + 1 + entIdSize], element->valueText, bufLen - (depth + entIdSize + 1));
    buf[bufLen - 1] = 0;
}

void Scr_ScriptWatch::Draw_r(Scr_WatchElement_s *element,
    float x,
    float y,
    float width,
    float lastHeight,
    int startLine,
    unsigned int depth,
    bool isArray,
    int *currentLine,
    float *currentY,
    float compX,
    float compY)
{
    float v13; // [esp+2Ch] [ebp-280h]
    float v14; // [esp+30h] [ebp-27Ch]
    float v15; // [esp+44h] [ebp-268h]
    float v16; // [esp+48h] [ebp-264h]
    float v17; // [esp+4Ch] [ebp-260h]
    float v18; // [esp+50h] [ebp-25Ch]
    float v19; // [esp+54h] [ebp-258h]
    float v20; // [esp+58h] [ebp-254h]
    float v21; // [esp+5Ch] [ebp-250h]
    float v22; // [esp+60h] [ebp-24Ch]
    float v23; // [esp+64h] [ebp-248h]
    float v24; // [esp+68h] [ebp-244h]
    float v25; // [esp+6Ch] [ebp-240h]
    float v26; // [esp+70h] [ebp-23Ch]
    float v27; // [esp+74h] [ebp-238h]
    float height; // [esp+78h] [ebp-234h]
    float v29; // [esp+80h] [ebp-22Ch]
    float v30; // [esp+84h] [ebp-228h]
    float selectColor[4]; // [esp+9Ch] [ebp-210h] BYREF
    float colorYellow[4]; // [esp+ACh] [ebp-200h] BYREF
    float colorWhite[4]; // [esp+BCh] [ebp-1F0h] BYREF
    float colorBlue[4]; // [esp+CCh] [ebp-1E0h] BYREF
    float colorRed[4]; // [esp+DCh] [ebp-1D0h] BYREF
    char buf[396]; // [esp+ECh] [ebp-1C0h] BYREF
    float fraction; // [esp+27Ch] [ebp-30h]
    float innerWidth; // [esp+280h] [ebp-2Ch]
    float color[4]; // [esp+284h] [ebp-28h] BYREF
    float colorDelta[4]; // [esp+294h] [ebp-18h] BYREF
    int deltaTime; // [esp+2A4h] [ebp-8h]
    unsigned int startCol; // [esp+2A8h] [ebp-4h]

    CL_LookupColor(0, 0x37u, colorWhite);
    colorRed[0] = 1.0;
    colorRed[1] = 0.0;
    colorRed[2] = 0.0;
    colorRed[3] = 1.0;
    CL_LookupColor(0, 0x33u, colorYellow);
    CL_LookupColor(0, 0x34u, colorBlue);
    selectColor[0] = 0.5;
    selectColor[1] = 0.5;
    selectColor[2] = 0.5;
    selectColor[3] = 1.0;
    innerWidth = width - UI_Component::g.charHeight;
    startCol = (compX / UI_Component::g.charWidth);
    while (element && lastHeight >= *currentY)
    {
        if (*currentLine >= startLine && *currentY >= 0.0)
        {
            if (depth >= 0x186)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 3433, 0, "%s", "depth < sizeof( buf )");
            memset(buf, 0x20u, depth);
            if (element->objectType)
                buf[depth] = 2 * element->expand + 43;
            else
                buf[depth] = 32;
            if (*currentLine == this->selectedLine)
            {
                v30 = y + *currentY;
                v29 = x + UI_Component::g.charHeight;
                UI_Component::DrawPic(
                    v29,
                    v30,
                    innerWidth,
                    UI_Component::g.charHeight,
                    selectColor,
                    sharedUiInfo.assets.whiteMaterial);
            }
            switch (element->breakpointType)
            {
            case 1u:
            case 5u:
                height = UI_Component::g.charHeight + UI_Component::g.charHeight;
                v27 = y + *currentY - UI_Component::g.charHeight * 0.5;
                UI_Component::DrawPic(x, v27, UI_Component::g.charHeight, height, colorRed, UI_Component::g.filledCircle);
                if (element->hitBreakpoint)
                {
                    v26 = y + *currentY;
                    v25 = UI_Component::g.charWidth * 0.5 + x;
                    UI_Component::DrawText(v25, v26, UI_Component::g.charHeight, 5, colorYellow, (char*)">");
                }
                break;
            case 2u:
                v18 = y + *currentY;
                v17 = UI_Component::g.charWidth * 0.5 + x;
                UI_Component::DrawText(v17, v18, UI_Component::g.charHeight, 5, colorRed, (char *)"?");
                break;
            case 3u:
            case 4u:
                v16 = y + *currentY;
                v15 = UI_Component::g.charWidth * 0.5 + x;
                UI_Component::DrawText(v15, v16, UI_Component::g.charHeight, 5, colorRed, (char *)"O");
                break;
            case 6u:
                v24 = UI_Component::g.charHeight + UI_Component::g.charHeight;
                v23 = y + *currentY - UI_Component::g.charHeight * 0.5;
                UI_Component::DrawPic(x, v23, UI_Component::g.charHeight, v24, colorBlue, UI_Component::g.filledCircle);
                if (element->hitBreakpoint)
                {
                    v22 = y + *currentY;
                    v21 = UI_Component::g.charWidth * 0.5 + x;
                    UI_Component::DrawText(v21, v22, UI_Component::g.charHeight, 5, colorYellow, (char *)">");
                }
                break;
            case 7u:
                v20 = y + *currentY;
                v19 = UI_Component::g.charWidth * 0.5 + x;
                UI_Component::DrawText(v19, v20, UI_Component::g.charHeight, 5, colorBlue, (char *)"O");
                break;
            default:
                break;
            }
            Scr_PrintElementText(element, 390, depth, buf);
            if (&buf[strlen(buf) + 1] - &buf[1] > startCol)
            {
                color[0] = colorWhite[0];
                color[1] = colorWhite[1];
                color[2] = colorWhite[2];
                color[3] = colorWhite[3];
                if (element->changed)
                {
                    if (element->changedTime)
                    {
                        deltaTime = Sys_Milliseconds() - element->changedTime;
                        if (deltaTime >= 1000)
                        {
                            element->changed = 0;
                        }
                        else
                        {
                            fraction = deltaTime / 1000.0;
                            Vec4Sub(colorWhite, colorYellow, colorDelta);
                            Vec4Mad(colorYellow, fraction, colorDelta, color);
                        }
                    }
                    else
                    {
                        color[0] = colorYellow[0];
                        color[1] = colorYellow[1];
                        color[2] = colorYellow[2];
                        color[3] = colorYellow[3];
                    }
                }
                if (Sys_IsRemoteDebugClient() && !scrDebuggerGlob.atBreakpoint && !element->changedTime)
                    element->changedTime = Sys_Milliseconds();
                v14 = y + *currentY;
                v13 = x + UI_Component::g.charHeight;
                UI_Component::DrawText(v13, v14, innerWidth, 5, color, &buf[startCol]);
            }
        }
        ++ * currentLine;
        *currentY = *currentY + UI_Component::g.charHeight;
        if (Scr_ElementChildrenExist(element))
            Scr_ScriptWatch::Draw_r(
                element->childHead,
                x,
                y,
                width,
                lastHeight,
                startLine,
                depth + 1,
                element->objectType == 21,
                currentLine,
                currentY,
                compX,
                compY);
        element = element->next;
    }
}

Scr_WatchElement_s *Scr_ScriptWatch::GetElementWithId_r(Scr_WatchElement_s *element, int id)
{
    Scr_WatchElement_s *childElement; // [esp+4h] [ebp-4h]

    while (element)
    {
        if (element->id == id)
            return element;
        if (element->childHead)
        {
            childElement = GetElementWithId_r(element->childHead, id);
            if (childElement)
                return childElement;
        }
        element = element->next;
    }
    return 0;
}

void Scr_ScriptWatch::AddElement(Scr_WatchElement_s *element, char *text)
{
    int v3; // [esp+0h] [ebp-18h]
    ScriptExpression_t scriptExpr; // [esp+8h] [ebp-10h] BYREF
    ScriptExpression_t *expr; // [esp+14h] [ebp-4h]

    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6608, 0, "%s", "element");
    if (!element->breakpoint)
    {
        scriptExpr.exprHead = 0;
        if (Sys_IsRemoteDebugClient())
        {
        LABEL_13:
            if (element->parent)
            {
                if (!Sys_IsRemoteDebugClient())
                {
                    Com_Printf(23, "Cannot change child element\n");
                    Scr_FreeDebugExpr(&scriptExpr);
                }
            }
            else
            {
                if (!Sys_IsRemoteDebugClient())
                {
                    expr = &element->expr;
                    Scr_FreeDebugExpr(&element->expr);
                }
                ReplaceString(&element->refText, text);
                if (!Sys_IsRemoteDebugClient())
                {
                    element->expr = scriptExpr;
                    Scr_RemoveValue(element);
                    Scr_ScriptWatch::EvaluateWatchElement(element);
                    Scr_ScriptWatch::UpdateHeight();
                }
            }
            return;
        }
        Scr_CompileText(text, &scriptExpr);
        v3 = *(const char *)scriptExpr.parseData.type;
        if (*(const char *)scriptExpr.parseData.type != 83)
        {
            if (v3 > 83 && v3 <= 85)
            {
                Scr_FreeDebugExpr(&scriptExpr);
                return;
            }
            goto LABEL_13;
        }
        if (!scrVarPub.evaluate)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 6623, 0, "%s", "scrVarPub.evaluate");
        scrVarPub.evaluate = 0;
        Scr_ExecCode(*(const char**)(scriptExpr.parseData.type + 4), this->localId);
        scrVarPub.evaluate = 1;
        SL_ShutdownSystem(2);
        Scr_FreeDebugExpr(&scriptExpr);
        Scr_ScriptWatch::Evaluate();
    }
}

void Scr_ScriptWatch::DeleteElement()
{
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    element = Scr_ScriptWatch::GetSelectedElement();
    if (element)
    {
        // KISAKTODO: meh
        //if (Sys_IsRemoteDebugClient())
        //{
        //    Sys_WriteDebugSocketMessageType(0xAu);
        //    Scr_WriteElement(element);
        //    Sys_EndWriteDebugSocket();
        //}
        //else
        {
            Scr_ScriptWatch::DeleteElementInternal(element);
        }
    }
}

int __cdecl Scr_GetWatchElementSize(Scr_WatchElement_s *element)
{
    int size; // [esp+0h] [ebp-4h]

    size = 0;
    while (element)
    {
        ++size;
        if (Scr_ElementChildrenExist(element))
            size += Scr_GetWatchElementSize(element->childHead);
        element = element->next;
    }
    return size;
}

void Scr_ScriptWatch::DeleteElementInternal(Scr_WatchElement_s *element)
{
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 4396, 0, "%s", "element");
    if (!element->parent)
    {
        *Scr_ScriptWatch::GetElementRef(element) = element->next;
        Scr_ScriptWatch::UpdateHeight();
        Scr_ScriptWatch::SetSelectedElement(element->next, 1);
        Scr_ScriptWatch::FreeWatchElement(element);
    }
}

void __thiscall Scr_ScriptWatch::ToggleWatchElementBreakpoint(
    Scr_WatchElement_s *element,
    unsigned __int8 type)
{
    Scr_WatchElement_s *ElementRoot; // eax
    Scr_WatchElement_s *elementa; // [esp+10h] [ebp+8h]

    if (element->breakpointType == type)
    {
        elementa = Scr_ScriptWatch::RemoveBreakpoint(element);
        if (!Sys_IsRemoteDebugClient())
        {
            Scr_FreeDebugExpr(&elementa->expr);
            Scr_CompileText(elementa->refText, &elementa->expr);
        }
    }
    else
    {
        elementa = Scr_ScriptWatch::AddBreakpoint(element, type);
    }
    if (!Sys_IsRemoteDebugClient())
    {
        ElementRoot = Scr_GetElementRoot(elementa);
        Scr_ScriptWatch::EvaluateWatchElement(ElementRoot);
    }
}

void __thiscall Scr_ScriptWatch::ToggleBreakpointInternal(
    Scr_WatchElement_s *element,
    unsigned __int8 type)
{
    unsigned __int8 breakpointType; // [esp+0h] [ebp-Ch]
    Scr_Breakpoint *breakpoint; // [esp+8h] [ebp-4h]

    if (!element->threadList && !element->endonList)
    {
        breakpoint = element->breakpoint;
        if (breakpoint)
        {
            Scr_SelectScriptLine(breakpoint->bufferIndex, breakpoint->line);
            breakpointType = element->breakpointType;
            switch (breakpointType)
            {
            case 4u:
                if (type == 1)
                {
                    element->breakpointType = 5;
                }
                else if (type == 3)
                {
                    Scr_ScriptWatch::DeleteElementInternal(element);
                }
                break;
            case 6u:
                if (type == 7)
                {
                    element->breakpointType = 7;
                    if (!Sys_IsRemoteDebugClient())
                    {
                        if (breakpoint->builtinIndex < 0 || breakpoint->builtinIndex >= scrCompilePub.func_table_size)
                            MyAssertHandler(
                                ".\\script\\scr_debugger.cpp",
                                7510,
                                0,
                                "%s\n\t(breakpoint->builtinIndex) = %i",
                                "(breakpoint->builtinIndex >= 0 && breakpoint->builtinIndex < scrCompilePub.func_table_size)",
                                breakpoint->builtinIndex);
                        --scrVmDebugPub.func_table[breakpoint->builtinIndex].breakpointCount;
                    }
                }
                break;
            case 7u:
                if (type == 6)
                {
                    element->breakpointType = 6;
                    if (!Sys_IsRemoteDebugClient())
                    {
                        if (breakpoint->builtinIndex < 0 || breakpoint->builtinIndex >= scrCompilePub.func_table_size)
                            MyAssertHandler(
                                ".\\script\\scr_debugger.cpp",
                                7493,
                                0,
                                "%s\n\t(breakpoint->builtinIndex) = %i",
                                "(breakpoint->builtinIndex >= 0 && breakpoint->builtinIndex < scrCompilePub.func_table_size)",
                                breakpoint->builtinIndex);
                        ++scrVmDebugPub.func_table[breakpoint->builtinIndex].breakpointCount;
                    }
                }
                break;
            default:
                if (element->breakpointType != 5)
                    MyAssertHandler(
                        ".\\script\\scr_debugger.cpp",
                        7526,
                        0,
                        "%s\n\t(element->breakpointType) = %i",
                        "(element->breakpointType == SCR_BREAKPOINT_LINE_NORMAL)",
                        element->breakpointType);
                if (type == 3)
                {
                    element->breakpointType = 4;
                }
                else if (type == 1)
                {
                    Scr_ScriptWatch::DeleteElementInternal(element);
                }
                break;
            }
        }
        else if (type != 6 && type != 7)
        {
            Scr_ScriptWatch::ToggleWatchElementBreakpoint(element, type);
        }
    }
}

Scr_WatchElement_s *Scr_ScriptWatch::GetSelectedElement_r(Scr_WatchElement_s *element, int *currentLine)
{
    Scr_WatchElement_s *childElement; // [esp+4h] [ebp-4h]

    while (element)
    {
        if (*currentLine == this->selectedLine)
            return element;
        ++*currentLine;
        if (Scr_ElementChildrenExist(element))
        {
            //childElement = Scr_ScriptWatch::GetSelectedElement_r(this, element->childHead, currentLine);
            childElement = GetSelectedElement_r(element->childHead, currentLine);
            if (childElement)
                return childElement;
        }
        element = element->next;
    }
    return 0;
}

Scr_WatchElement_s *Scr_ScriptWatch::GetSelectedElement()
{
    int currentLine; // [esp+4h] [ebp-4h] BYREF

    currentLine = 0;
    return Scr_ScriptWatch::GetSelectedElement_r(this->elementHead, &currentLine);
}

Scr_WatchElement_s **Scr_ScriptWatch::GetElementRef(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s **pElement; // [esp+4h] [ebp-4h]

    for (pElement = &this->elementHead; *pElement != element; pElement = &(*pElement)->next)
    {
        if (!*pElement)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 3677, 0, "%s", "*pElement");
    }
    return pElement;
}

Scr_WatchElement_s *Scr_ScriptWatch::GetElementPrev(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s *prevElement; // [esp+4h] [ebp-4h]

    if (this->elementHead == element)
        return 0;
    for (prevElement = this->elementHead; prevElement; prevElement = prevElement->next)
    {
        if (prevElement->next == element)
            return prevElement;
    }
    if (!alwaysfails)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 3701, 0, "unreachable");
    return 0;
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::BackspaceElementInternal(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s **pElement; // [esp+4h] [ebp-8h]
    Scr_WatchElement_s *prevElement; // [esp+8h] [ebp-4h]

    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 4486, 0, "%s", "element");
    if (element->parent)
        return element;
    pElement = Scr_ScriptWatch::GetElementRef(element);
    prevElement = Scr_ScriptWatch::GetElementPrev(element);
    Scr_ScriptWatch::SetSelectedElement(prevElement, 1);
    *pElement = element->next;
    Scr_ScriptWatch::UpdateHeight();
    Scr_ScriptWatch::FreeWatchElement(element);
    return prevElement;
}

void __thiscall Scr_ScriptWatch::ToggleExpandElement(Scr_WatchElement_s *element)
{
    element->expand = !element->expand;
    if (element->expand)
    {
        if (element->objectType)
        {
            if (!Sys_IsRemoteDebugClient())
            {
                //Scr_ScriptWatch::EvaluateWatchChildren(element);
                this->EvaluateWatchChildren(element);
            }
        }
        else
        {
            element->expand = 0;
        }
    }
    if (!element->expand)
        Scr_FreeWatchElementChildrenStrict(element);
    if (!Sys_IsRemoteDebugClient())
    {
        //Scr_ScriptWatch::UpdateHeight();
        this->UpdateHeight();
    }
}

void Scr_ScriptWatch::UpdateHeight()
{
    this->numLines = Scr_GetWatchElementSize(this->elementHead);
    UI_LinesComponent::UpdateHeight();
}

void UI_LinesComponent::UpdateHeight()
{
    this->size[1] = this->numLines * UI_Component::g.charHeight;
}

void UI_LinesComponent::ClearFocus()
{
    this->focusOnSelectedLine = 0;
}

void __thiscall UI_LinesComponent::IncSelectedLineFocus(bool wrap)
{
    if (this->selectedLine < 0 || wrap && this->selectedLine >= this->numLines - 1)
        this->SetSelectedLineFocus(0, 0);
    else
        this->SetSelectedLineFocus(this->selectedLine + 1, 0);
}

void __thiscall UI_LinesComponent::DecSelectedLineFocus(bool wrap)
{
    if (this->selectedLine < 0 || wrap && this->selectedLine <= 0)
        this->SetSelectedLineFocus(this->numLines - 1, 0);
    else
        this->SetSelectedLineFocus(this->selectedLine - 1, 0);
}

void Scr_ScriptWatch::SetSelectedElement(Scr_WatchElement_s *selElement, bool user)
{
    int currentLine; // [esp+4h] [ebp-4h] BYREF

    if (selElement)
    {
        currentLine = 0;
        Scr_ScriptWatch::SetSelectedElement_r(selElement, this->elementHead, &currentLine, user);
    }
    else
    {
        this->selectedLine = -1;
    }
}

void Scr_ScriptWatch::FreeWatchElement(Scr_WatchElement_s *element)
{
    Scr_Breakpoint *breakpoint; // [esp+4h] [ebp-8h]

    if (element->parent)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5030, 0, "%s", "!element->parent");
    breakpoint = element->breakpoint;
    if (breakpoint)
    {
        Scr_FreeWatchElementText(element);
        if (breakpoint->element)
            Scr_FreeLineBreakpoint(breakpoint, 0);
        if (!Sys_IsRemoteDebugClient() && element->breakpointType == 6)
        {
            if (breakpoint->builtinIndex < 0 || breakpoint->builtinIndex >= scrCompilePub.func_table_size)
                MyAssertHandler(
                    ".\\script\\scr_debugger.cpp",
                    5046,
                    0,
                    "%s\n\t(breakpoint->builtinIndex) = %i",
                    "(breakpoint->builtinIndex >= 0 && breakpoint->builtinIndex < scrCompilePub.func_table_size)",
                    breakpoint->builtinIndex);
            --scrVmDebugPub.func_table[breakpoint->builtinIndex].breakpointCount;
        }
    }
    else
    {
        Scr_FreeWatchElementChildren(element);
        if (!Sys_IsRemoteDebugClient())
        {
            if (element->breakpointType)
            {
                Scr_ScriptWatch::RemoveBreakpoint(element);
                //Scr_ScriptWatch::RemoveBreakpoint(this, element);
            }
            Scr_FreeDebugExpr(&element->expr);
        }
    }
    Scr_FreeDebugMem(element);
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::AddBreakpoint(Scr_WatchElement_s *element, unsigned __int8 type)
{
    if (element->breakpointType == type)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7396, 0, "%s", "element->breakpointType != type");
    if (element->parent)
        element = Scr_ScriptWatch::CloneElement(element);
    element->breakpointType = type;
    Scr_ScriptWatch::SetSelectedElement(element, 0);
    return element;
}

Scr_WatchElement_s * Scr_ScriptWatch::RemoveBreakpoint(Scr_WatchElement_s *element)
{
    if (!element->breakpointType)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7414, 0, "%s", "element->breakpointType != SCR_BREAKPOINT_NONE");
    element->breakpointType = 0;
    if (!Sys_IsRemoteDebugClient())
        Scr_RemoveValue(element);
    return element;
}

bool __thiscall Scr_ScriptWatch::SetSelectedElement_r(
    Scr_WatchElement_s *selElement,
    Scr_WatchElement_s *element,
    int *currentLine,
    bool user)
{
    while (element)
    {
        if (element == selElement)
        {
            SetSelectedLineFocus(*currentLine, user);
            return 1;
        }
        ++ * currentLine;
        if (Scr_ElementChildrenExist(element)
            && Scr_ScriptWatch::SetSelectedElement_r(selElement, element->childHead, currentLine, user))
        {
            return 1;
        }
        element = element->next;
    }
    return 0;
}

bool UI_LinesComponent::SetSelectedLineFocus(int newSelectedLine, bool user)
{
    if (newSelectedLine >= user - 1 && newSelectedLine < this->numLines)
    {
        this->selectedLine = newSelectedLine;
        this->focusOnSelectedLine = 1;
        this->focusOnSelectedLineUser = user;
        return 1;
    }
    else
    {
        this->selectedLine = -1;
        return 0;
    }
}

void __thiscall Scr_ScriptWatch::DisplayThreadPos(Scr_WatchElement_s *element)
{
    unsigned int bufferIndex; // [esp+4h] [ebp-10h]
    unsigned int lineNum; // [esp+8h] [ebp-Ch]
    const char *codePos; // [esp+Ch] [ebp-8h]
    unsigned int sourcePos; // [esp+10h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7063, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (scrVarPub.evaluate && (element->objectType == 22 || element->objectType == 14 && element->directObject))
    {
        codePos = Scr_GetElementThreadPos(element);
        if (codePos)
        {
            bufferIndex = Scr_GetSourceBuffer(codePos - 1);
            sourcePos = Scr_GetPrevSourcePos(codePos - 1, 0);
            lineNum = Scr_GetLineNum(bufferIndex, sourcePos);
            Scr_SelectScriptLine(bufferIndex, lineNum);
        }
    }
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::CreateWatchElement(
    char *text,
    Scr_WatchElement_s **prevElem,
    const char *name)
{
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    element = Scr_CreateWatchElement(text, prevElem, name);
    if (!++this->elementId)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6513, 0, "%s", "elementId");
    element->id = this->elementId;
    //Scr_ScriptWatch::UpdateHeight(this);
    this->UpdateHeight();
    return element;
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::CreateBreakpointElement(
    Scr_WatchElement_s *element,
    unsigned int bufferIndex,
    unsigned int sourcePos,
    bool user)
{
    Scr_WatchElement_s **ElementRef; // eax
    Scr_WatchElement_s *ElementRoot; // [esp+0h] [ebp-1ACh]
    Scr_WatchElement_s **pElement; // [esp+8h] [ebp-1A4h]
    char refText[136]; // [esp+Ch] [ebp-1A0h] BYREF
    char valueText[268]; // [esp+94h] [ebp-118h] BYREF
    Scr_WatchElement_s *newElement; // [esp+1A4h] [ebp-8h]
    int lineNum; // [esp+1A8h] [ebp-4h]

    if (element)
    {
        ElementRoot = Scr_GetElementRoot(element);
        ElementRef = Scr_ScriptWatch::GetElementRef(ElementRoot);
    }
    else
    {
        ElementRef = Scr_ScriptWatch::GetElementRef(0);
    }
    pElement = ElementRef;
    lineNum = Scr_GetSourcePos(bufferIndex, sourcePos, valueText, 0x101u) + 1;
    Com_sprintf(refText, 0x81u, "%i %s", lineNum, scrParserPub.sourceBufferLookup[bufferIndex].buf);
    newElement = Scr_ScriptWatch::CreateWatchElement(refText, pElement, "Scr_ScriptWatch::CreateBreakpointElement");
    ReplaceString(&newElement->valueText, valueText);
    Scr_ScriptWatch::SetSelectedElement(newElement, 1);
    return newElement;
}

void __thiscall Scr_ScriptWatch::SortHitBreakpointsTop()
{
    Scr_WatchElement_s **pElement; // [esp+8h] [ebp-14h]
    Scr_WatchElement_s **pInsertPoint; // [esp+Ch] [ebp-10h]
    int hitBreakpoint; // [esp+10h] [ebp-Ch]
    Scr_WatchElement_s *element; // [esp+18h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
    {
        scrDebuggerGlob.gainFocusTime = Sys_Milliseconds() + 500;
        SetForegroundWindow(g_wv.hWnd);
    }
    scrDebuggerGlob.atBreakpoint = 1;
    hitBreakpoint = 0;
    for (pInsertPoint = &this->elementHead;
        *pInsertPoint && (*pInsertPoint)->hitBreakpoint;
        pInsertPoint = &(*pInsertPoint)->next)
    {
        hitBreakpoint = 1;
    }
    pElement = pInsertPoint;
    while (*pElement)
    {
        element = *pElement;
        if ((*pElement)->hitBreakpoint)
        {
            hitBreakpoint = 1;
            *pElement = (*pElement)->next;
            element->next = *pInsertPoint;
            *pInsertPoint = element;
            pInsertPoint = &element->next;
        }
        else
        {
            pElement = &(*pElement)->next;
        }
    }
    if (hitBreakpoint)
    {
        if (this->elementHead && this->elementHead->breakpoint)
            this->SetSelectedLineFocus( 0, 0);
        else
            this->SetSelectedLineFocus( -1, 0);

        Scr_SetMiscScrollPaneComp(this);
    }
}


bool __thiscall Scr_ScriptWatch::SetSelectedLineFocus(int newSelectedLine, bool user)
{
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    if (!UI_LinesComponent::SetSelectedLineFocus(newSelectedLine, user))
        return 0;
    element = Scr_ScriptWatch::GetSelectedElement();
    if (!element)
        return 1;
    if (element->objectType != 14 && element->objectType != 22)
        return 1;
    // KISAKTODO: meh
    //if (Sys_IsRemoteDebugClient())
    //{
    //    Sys_WriteDebugSocketMessageType(0x27u);
    //    Scr_WriteElement(element);
    //    Sys_EndWriteDebugSocket();
    //}
    //else
    {
        //Scr_ScriptWatch::DisplayThreadPos(this, element);
        DisplayThreadPos(element);
    }
    return 1;
}

bool __thiscall Scr_ScriptCallStack::SetSelectedLineFocus(int newSelectedLine, bool user)
{
    unsigned int LineNum; // eax
    unsigned int bufferIndex; // [esp+4h] [ebp-8h]

    if (!UI_LinesComponent::SetSelectedLineFocus(newSelectedLine, user))
        return 0;
    if (newSelectedLine < 0)
        return 1;
    bufferIndex = this->stack[newSelectedLine].bufferIndex;
    if (bufferIndex != -1)
    {
        LineNum = Scr_GetLineNum(bufferIndex, this->stack[newSelectedLine].sourcePos);
        Scr_SelectScriptLine(bufferIndex, LineNum);
    }
    return 1;
}

void Scr_ScriptCallStack::Draw(
    float x,
    float y,
    float width,
    float height,
    float compX,
    float compY)
{
    float v7; // [esp+18h] [ebp-F8h]
    float v8; // [esp+2Ch] [ebp-E4h]
    int currentLine; // [esp+44h] [ebp-CCh]
    float selectColor[4]; // [esp+48h] [ebp-C8h] BYREF
    unsigned int bufferIndex; // [esp+58h] [ebp-B8h]
    int startLine; // [esp+5Ch] [ebp-B4h]
    float startLineFrac; // [esp+60h] [ebp-B0h]
    float currentY; // [esp+64h] [ebp-ACh]
    int index; // [esp+68h] [ebp-A8h]
    float lastHeight; // [esp+6Ch] [ebp-A4h]
    float color[4]; // [esp+70h] [ebp-A0h] BYREF
    char text[136]; // [esp+80h] [ebp-90h] BYREF
    unsigned int startCol; // [esp+10Ch] [ebp-4h]

    UI_Component::DrawPic(x, y, width, height, 0, cls.consoleMaterial);
    CL_LookupColor(0, 0x37u, color);
    lastHeight = height - UI_Component::g.charHeight;
    startLineFrac = compY / UI_Component::g.charHeight;
    startLine = startLineFrac;
    currentY = startLine - startLineFrac;
    if (currentY < 0.0)
    {
        currentY = currentY + 1.0;
        ++startLine;
    }
    currentY = currentY * UI_Component::g.charHeight;
    currentLine = startLine;
    startCol = (compX / UI_Component::g.charWidth);
    selectColor[0] = 0.5;
    selectColor[1] = 0.5;
    selectColor[2] = 0.5;
    selectColor[3] = 1.0;
    while (lastHeight >= currentY && currentLine < this->numLines)
    {
        if (currentLine == this->selectedLine)
        {
            v8 = y + currentY;
            UI_Component::DrawPic(x, v8, width, UI_Component::g.charHeight, selectColor, sharedUiInfo.assets.whiteMaterial);
        }
        index = currentLine == scrVmPub.function_count;
        bufferIndex = this->stack[currentLine].bufferIndex;
        if (bufferIndex == -1)
            I_strncpyz(text, "<removed thread>", 129);
        else
            Scr_GetSourcePos(this->stack[currentLine].bufferIndex, this->stack[currentLine].sourcePos, text, 0x81u);
        ++currentLine;
        if (&text[strlen(text) + 1] - &text[1] > startCol)
        {
            v7 = y + currentY;
            UI_Component::DrawText(x, v7, width, 5, color, &text[startCol]);
        }
        currentY = currentY + UI_Component::g.charHeight;
    }
}

void __thiscall Scr_ScriptCallStack::UpdateStack()
{
    Scr_SourcePos2_t *pos; // [esp+4h] [ebp-14h]
    unsigned int index; // [esp+Ch] [ebp-Ch]
    int i; // [esp+10h] [ebp-8h]
    char *codePos; // [esp+14h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 3193, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (scrVmPub.function_count)
    {
        this->numLines = scrVmPub.function_count + 1;
        for (i = 0; i <= scrVmPub.function_count; ++i)
        {
            if (i)
            {
                codePos = (char*)scrVmPub.stack[3 * (scrVmPub.function_count - i) - 96].u.intValue;
                index = scrVmPub.function_frame_start[scrVmPub.function_count - i].fs.localId == 0;
            }
            else
            {
                codePos = (char*)(scrDebuggerGlob.breakpointCodePos + 1);
                index = 0;
            }
            pos = &this->stack[i];
            if (codePos == &g_EndPos)
            {
                pos->bufferIndex = -1;
                this->stack[i].sourcePos = 0;
            }
            else
            {
                pos->bufferIndex = Scr_GetSourceBuffer(codePos - 1);
                this->stack[i].sourcePos = Scr_GetPrevSourcePos(codePos - 1, index);
            }
        }
    }
    else
    {
        this->numLines = 0;
    }
}

void Scr_OpenScriptList::Shutdown()
{
    Scr_StringNode_s *node; // [esp+34h] [ebp-10h]
    char *filenamea; // [esp+38h] [ebp-Ch]
    char *filename; // [esp+38h] [ebp-Ch]
    int f; // [esp+3Ch] [ebp-8h]
    int i; // [esp+40h] [ebp-4h]

    f = FS_FOpenTextFileWrite("open_scripts.txt");
    if (f)
    {
        if (scrDebuggerGlob.scriptList.scriptWindows && scrDebuggerGlob.scriptList.selectedLine >= 0)
        {
            //filenamea = Scr_ScriptWindow::GetFilename(scrDebuggerGlob.scriptList.scriptWindows[scrDebuggerGlob.scriptList.selectedLine]);
            filenamea = scrDebuggerGlob.scriptList.scriptWindows[scrDebuggerGlob.scriptList.selectedLine]->GetFilename();
            FS_Write(filenamea, strlen(filenamea), f);
        }
        FS_Write("\n", 1u, f);
        for (i = 0; i < this->numLines; ++i)
        {
            //filename = Scr_ScriptWindow::GetFilename(this->scriptWindows[i]);
            filename = this->scriptWindows[i]->GetFilename();
            if (*filename)
            {
                FS_Write(filename, strlen(filename), f);
                FS_Write("\n", 1u, f);
            }
        }
        while (this->usedHead)
        {
            FS_Write(this->usedHead->text, strlen(this->usedHead->text), f);
            FS_Write("\n", 1u, f);
            Hunk_FreeDebugMem();
            node = this->usedHead->next;
            Hunk_FreeDebugMem();
            this->usedHead = node;
        }
        FS_FCloseFile(f);
    }
    Scr_AbstractScriptList::Shutdown();
}

bool Scr_OpenScriptList::SetSelectedLineFocus(
    int newSelectedLine,
    bool user)
{
    unsigned int sortedIndex; // [esp+4h] [ebp-Ch]
    unsigned int bufferIndex; // [esp+8h] [ebp-8h]

    if (!UI_LinesComponent::SetSelectedLineFocus(newSelectedLine, user))
        return 0;
    if ((newSelectedLine & 0x80000000) != 0)
        return 1;
    if (newSelectedLine >= scrParserPub.sourceBufferLookupLen)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            2875,
            0,
            "newSelectedLine doesn't index scrParserPub.sourceBufferLookupLen\n\t%i not in [0, %i)",
            newSelectedLine,
            scrParserPub.sourceBufferLookupLen);
    bufferIndex = this->scriptWindows[newSelectedLine]->bufferIndex;
    if (bufferIndex == -1)
        return 1;
    if (bufferIndex >= scrParserPub.sourceBufferLookupLen)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            2881,
            0,
            "bufferIndex doesn't index scrParserPub.sourceBufferLookupLen\n\t%i not in [0, %i)",
            bufferIndex,
            scrParserPub.sourceBufferLookupLen);
    sortedIndex = scrParserPub.sourceBufferLookup[bufferIndex].sortedIndex;
    if (sortedIndex >= scrParserPub.sourceBufferLookupLen)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            2886,
            0,
            "sortedIndex doesn't index scrParserPub.sourceBufferLookupLen\n\t%i not in [0, %i)",
            sortedIndex,
            scrParserPub.sourceBufferLookupLen);
    //UI_LinesComponent::SetSelectedLineFocus(&scrDebuggerGlob.scriptList, sortedIndex, 1);
   //((UI_LinesComponent*)&scrDebuggerGlob.scriptList).SetSelectedLineFocus(sortedIndex, 1);
    scrDebuggerGlob.scriptList.SetSelectedLineFocus(sortedIndex, 1);
    return 1;
}


void Scr_ScriptWatch::Evaluate()
{
    Scr_SelectedLineInfo info; // [esp+4h] [ebp-10h] BYREF
    Scr_WatchElement_s *element; // [esp+10h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6860, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6863, 0, "%s", "scrVarPub.evaluate");
    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6864, 0, "%s", "!scrVmPub.outparamcount");
    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6865, 0, "%s", "!scrVmPub.inparamcount");

    Scr_ScriptWatch::SaveSelectedLine(&info);

    for (element = this->elementHead; element; element = element->next)
    {
        if (!element->breakpoint)
            Scr_ScriptWatch::EvaluateWatchElement( element);
    }

    Scr_ScriptWatch::LoadSelectedLine(&info);
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::PasteNonBreakpointElement(
    Scr_WatchElement_s *element,
    char *text,
    bool user)
{
    Scr_WatchElement_s **ElementRef; // eax
    Scr_WatchElement_s *ElementRoot; // [esp+0h] [ebp-10h]
    Scr_WatchElement_s *newElement; // [esp+Ch] [ebp-4h]

    if (element)
    {
        ElementRoot = Scr_GetElementRoot(element);
        ElementRef = Scr_ScriptWatch::GetElementRef(ElementRoot);
    }
    else
    {
        ElementRef = Scr_ScriptWatch::GetElementRef(0);
    }
    newElement = Scr_ScriptWatch::CreateWatchElement(text, ElementRef, "Scr_ScriptWatch::PasteNonBreakpointElement");
    if (!Sys_IsRemoteDebugClient())
    {
        if (newElement->breakpoint)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 4031, 0, "%s", "!newElement->breakpoint");
        Scr_CompileText(text, &newElement->expr);
        Scr_ScriptWatch::EvaluateWatchElement(newElement);
    }
    Scr_ScriptWatch::SetSelectedElement(newElement, 1);
    return newElement;
}

void Scr_ScriptWatch::SaveSelectedLine(Scr_SelectedLineInfo *info)
{
    int id; // [esp+0h] [ebp-Ch]
    Scr_WatchElement_s *selectedElement; // [esp+8h] [ebp-4h]

    selectedElement = Scr_ScriptWatch::GetSelectedElement();
    if (selectedElement)
        id = selectedElement->id;
    else
        id = 0;
    info->selectedId = id;
    info->oldSelectedLine = this->selectedLine;
    info->oldFocusOnSelectedLine = this->focusOnSelectedLine;
}

void __thiscall Scr_ScriptWatch::LoadSelectedLine(Scr_SelectedLineInfo *info)
{
    Scr_WatchElement_s *selectedElement; // [esp+8h] [ebp-4h]

    selectedElement = Scr_ScriptWatch::GetElementWithId(info->selectedId);
    if (selectedElement)
    {
        Scr_ScriptWatch::SetSelectedElement(selectedElement, 1);
        this->pos[1] = (this->selectedLine - info->oldSelectedLine) * UI_Component::g.charHeight + this->pos[1];
        this->focusOnSelectedLine = info->oldFocusOnSelectedLine;
    }
}

bool Scr_ScriptWatch::EvaluateWatchChildElement(
    Scr_WatchElement_s *element,
    unsigned int fieldName,
    Scr_WatchElement_s *childElement,
    bool hardcodedField)
{
    unsigned __int8 objectType; // [esp+4h] [ebp-18h]
    VariableValue value; // [esp+14h] [ebp-8h] BYREF

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6354, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (element->breakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6357, 0, "%s", "!element->breakpoint");
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6359, 0, "%s", "scrVarPub.evaluate");
    childElement->fieldName = fieldName;
    if (hardcodedField)
    {
        value.type = VAR_POINTER;
        value.u.intValue = element->objectId;
        switch (fieldName)
        {
        case 0u:
            AddRefToObject(value.u.stringValue);
            Scr_EvalSizeValue(&value);
            break;
        case 1u:
            break;
        case 2u:
        case 4u:
            value.type = VAR_UNDEFINED;
            break;
        case 3u:
            value.u.intValue = Scr_GetSelf(value.u.stringValue);
            break;
        default:
            value.u.intValue = fieldName - 5;
            if (fieldName == 5)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 6415, 0, "%s", "value.u.pointerValue");
            break;
        }
        AddRefToValue(value.type, value.u);
    }
    else
    {
        objectType = element->objectType;
        switch (objectType)
        {
        case 0x15u:
            value = Scr_GetArrayIndexValue(fieldName);
            AddRefToValue(value.type, value.u);
            Scr_EvalArrayVariable(element->objectId, &value);
            break;
        case 0x18u:
            value.u.intValue = fieldName;
            value.type = VAR_POINTER;
            AddRefToObject(fieldName);
            break;
        case 0x19u:
            value.u.intValue = fieldName;
            value.type = VAR_STRING;
            SL_AddRefToString(fieldName);
            break;
        default:
            Scr_EvalFieldVariable(fieldName, &value, element->objectId);
            break;
        }
    }
    return Scr_ScriptWatch::PostEvaluateWatchElement(childElement, &value);
}

int __cdecl CompareThreadIndices(unsigned int *arg1, unsigned int *arg2);

void Scr_ScriptWatch::EvaluateWatchChildren(Scr_WatchElement_s *parentElement)
{
    unsigned int AllVariableField_DONE; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *CanonicalString; // eax
    unsigned __int8 v6; // [esp+4h] [ebp-12Ch]
    bool v7; // [esp+8h] [ebp-128h]
    int v8; // [esp+Ch] [ebp-124h]
    bool v9; // [esp+10h] [ebp-120h]
    int(__cdecl * v10)(unsigned int *, unsigned int *); // [esp+14h] [ebp-11Ch]
    unsigned __int8 v11; // [esp+18h] [ebp-118h]
    unsigned __int8 objectType; // [esp+1Ch] [ebp-114h]
    bool oldHardcodedField; // [esp+2Fh] [ebp-101h]
    char fieldText[136]; // [esp+30h] [ebp-100h] BYREF
    unsigned int hardcodedNames[5]; // [esp+BCh] [ebp-74h] BYREF
    Scr_WatchElement_s *childElement; // [esp+D0h] [ebp-60h]
    unsigned int newIndex; // [esp+D4h] [ebp-5Ch]
    unsigned int oldChildCount; // [esp+D8h] [ebp-58h]
    int(__cdecl * compare)(unsigned int*, unsigned int*); // [esp+DCh] [ebp-54h]
    unsigned __int8 oldObjectType; // [esp+E2h] [ebp-4Eh]
    bool isArray; // [esp+E3h] [ebp-4Dh]
    Scr_WatchElement_s *newElements; // [esp+E4h] [ebp-4Ch]
    Scr_WatchElement_s *oldElements; // [esp+E8h] [ebp-48h]
    unsigned int hardcodedCount; // [esp+ECh] [ebp-44h]
    Scr_WatchElement_s *newElement; // [esp+F0h] [ebp-40h]
    int function_count; // [esp+F4h] [ebp-3Ch]
    unsigned int objectId; // [esp+F8h] [ebp-38h]
    Scr_WatchElement_s *oldElement; // [esp+FCh] [ebp-34h]
    Scr_WatchElement_s **newElementOldRef; // [esp+100h] [ebp-30h]
    int compareResult; // [esp+104h] [ebp-2Ch]
    bool elementChanged; // [esp+10Bh] [ebp-25h]
    unsigned int threadId; // [esp+10Ch] [ebp-24h]
    unsigned int oldIndex; // [esp+110h] [ebp-20h]
    unsigned int *names; // [esp+114h] [ebp-1Ch]
    bool hardcodedField; // [esp+11Bh] [ebp-15h]
    unsigned int nameIndex; // [esp+11Ch] [ebp-14h]
    unsigned int count; // [esp+120h] [ebp-10h]
    VariableValue value; // [esp+124h] [ebp-Ch] BYREF
    bool setChildCount; // [esp+12Eh] [ebp-2h]
    bool sameType; // [esp+12Fh] [ebp-1h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5768, 0, "%s", "!Sys_IsRemoteDebugClient()");
    oldObjectType = parentElement->oldObjectType;
    parentElement->oldObjectType = parentElement->objectType;
    if (parentElement->expand && parentElement->objectType)
    {
        if (!scrVarPub.evaluate)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 5780, 0, "%s", "scrVarPub.evaluate");
        isArray = parentElement->objectType == 21;
        hardcodedCount = 0;
        if (parentElement->objectType == 24)
        {
            if (!parentElement->parent)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 5787, 0, "%s", "parentElement->parent");
            objectId = parentElement->parent->objectId;
            if (!objectId)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 5789, 0, "%s", "objectId");
            count = Scr_FindAllThreads(objectId, 0, this->localId);
        }
        else if (parentElement->objectType == 25)
        {
            if (!parentElement->parent)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 5795, 0, "%s", "parentElement->parent");
            objectId = parentElement->parent->objectId;
            if (!objectId)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 5797, 0, "%s", "objectId");
            count = Scr_FindAllEndons(objectId, 0);
        }
        else
        {
            objectId = parentElement->objectId;
            if (!objectId)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 5804, 0, "%s", "objectId");
            if (parentElement->directObject)
            {
                objectType = parentElement->objectType;
                if (objectType == 14)
                {
                    threadId = GetSafeParentLocalId(parentElement->objectId);
                    if (!threadId && GetObjectType(parentElement->objectId) == 14)
                    {
                        for (function_count = scrVmPub.function_count; ; --function_count)
                        {
                            if (!function_count)
                                MyAssertHandler(".\\script\\scr_debugger.cpp", 5824, 0, "%s", "function_count");
                            if (parentElement->objectId == scrVmPub.function_frame_start[function_count].fs.localId)
                                break;
                        }
                        do
                            threadId = scrVmPub.function_frame_start[--function_count].fs.localId;
                        while (!threadId && function_count);
                    }
                    if (threadId)
                        hardcodedNames[hardcodedCount++] = threadId + 5;
                }
                else if (objectType > 0x11u && objectType <= 0x14u)
                {
                    hardcodedNames[hardcodedCount++] = 4;
                }
            }
            else
            {
                hardcodedNames[hardcodedCount++] = 1;
            }
            v11 = parentElement->objectType;
            if (v11 == 14)
            {
                hardcodedNames[hardcodedCount++] = 2;
                hardcodedNames[hardcodedCount++] = 3;
            }
            else if (v11 == 21)
            {
                hardcodedNames[hardcodedCount++] = 0;
            }
            if (hardcodedCount > 5)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 5872, 0, "%s", "hardcodedCount <= ARRAY_COUNT( hardcodedNames )");
            AllVariableField_DONE = Scr_FindAllVariableField(objectId, 0);
            count = hardcodedCount + AllVariableField_DONE;
        }
        if (count)
        {
            names = Scr_AllocDebugMem(4 * count, "Scr_ScriptWatch::EvaluateWatchChildren");
            memcpy(names, hardcodedNames, 4 * hardcodedCount);
            if (parentElement->objectType == 24)
            {
                Scr_FindAllThreads(objectId, names, this->localId);
                compare = CompareThreadIndices;
            }
            else if (parentElement->objectType == 25)
            {
                Scr_FindAllEndons(objectId, names);
                compare = CompareArrayIndices;
            }
            else
            {
                Scr_FindAllVariableField(objectId, &names[hardcodedCount]);
                if (isArray)
                    v10 = CompareArrayIndices;
                else
                    v10 = Scr_CompareCanonicalStrings;
                compare = v10;
            }
            qsort(&names[hardcodedCount], count - hardcodedCount, 4u, (int(__cdecl *)(void const *, void const *))compare);
            oldElements = parentElement->childArrayHead;
            oldChildCount = parentElement->childCount;
            newElements = (Scr_WatchElement_s*)Scr_AllocDebugMem(100 * count, "Scr_ScriptWatch::EvaluateWatchChildren3");
            memset(newElements, 0, 100 * count);
            newElementOldRef = (Scr_WatchElement_s**)Scr_AllocDebugMem(4 * count, "Scr_ScriptWatch::EvaluateWatchChildren");
            v9 = oldElements && parentElement->objectType == oldObjectType;
            sameType = v9;
            elementChanged = 0;
            oldIndex = 0;
            newIndex = 0;
            for (nameIndex = 0; nameIndex < count; ++nameIndex)
            {
                newElement = &newElements[newIndex];
                v3 = CopyString("");
                newElement->valueText = v3;
                v4 = CopyString("");
                newElement->refText = v4;
                hardcodedField = newIndex < hardcodedCount;
                if (Scr_ScriptWatch::EvaluateWatchChildElement(
                    parentElement,
                    names[nameIndex],
                    newElement,
                    newIndex < hardcodedCount))
                {
                    newElement->parent = parentElement;
                    if (!++this->elementId)
                        MyAssertHandler(".\\script\\scr_debugger.cpp", 5943, 0, "%s", "elementId");
                    newElement->id = this->elementId;
                    newElementOldRef[newIndex] = 0;
                    if (sameType)
                    {
                        while (oldIndex < oldChildCount)
                        {
                            oldElement = &oldElements[oldIndex];
                            oldHardcodedField = oldIndex < parentElement->hardcodedCount;
                            if (oldHardcodedField == hardcodedField)
                            {
                                if (hardcodedField)
                                    v8 = oldElement->fieldName - newElement->fieldName;
                                else
                                    v8 = compare(&oldElement->fieldName, &newElement->fieldName);
                                compareResult = v8;
                            }
                            else
                            {
                                compareResult = oldHardcodedField - hardcodedField;
                            }
                            if (compareResult >= 0)
                            {
                                Scr_RemoveValue(oldElement);
                                if (compareResult)
                                {
                                    elementChanged = 1;
                                }
                                else
                                {
                                    if (!this->elementId)
                                        MyAssertHandler(".\\script\\scr_debugger.cpp", 5977, 0, "%s", "elementId");
                                    --this->elementId;
                                    newElement->expand = oldElement->expand;
                                    newElement->childArrayHead = oldElement->childArrayHead;
                                    newElement->childHead = oldElement->childHead;
                                    newElement->childCount = oldElement->childCount;
                                    newElement->hardcodedCount = oldElement->hardcodedCount;
                                    newElement->objectType = oldElement->objectType;
                                    newElement->oldObjectType = oldElement->oldObjectType;
                                    newElement->directObject = oldElement->directObject;
                                    newElement->bufferIndex = oldElement->bufferIndex;
                                    newElement->sourcePos = oldElement->sourcePos;
                                    newElement->changed = oldElement->changed;
                                    newElement->changedTime = oldElement->changedTime;
                                    if (!oldElement->id)
                                        MyAssertHandler(".\\script\\scr_debugger.cpp", 5991, 0, "%s", "oldElement->id");
                                    newElement->id = oldElement->id;
                                    for (childElement = oldElement->childHead; childElement; childElement = childElement->next)
                                        childElement->parent = newElement;
                                    newElementOldRef[newIndex] = oldElement;
                                    ++oldIndex;
                                }
                                break;
                            }
                            elementChanged = 1;
                            Scr_FreeWatchElementChildren(oldElement);
                            ++oldIndex;
                        }
                    }
                    ++newIndex;
                }
                else
                {
                    Scr_FreeWatchElementText(newElement);
                }
            }
            Scr_FreeDebugMem(names);
            while (oldIndex < oldChildCount)
            {
                oldElement = &oldElements[oldIndex];
                elementChanged = 1;
                Scr_FreeWatchElementChildren(oldElement);
                ++oldIndex;
            }
            count = newIndex;
            v7 = newIndex && (!sameType || elementChanged || count != oldChildCount);
            setChildCount = v7;
            for (newIndex = 0; newIndex < count; ++newIndex)
            {
                newElement = &newElements[newIndex];
                oldElement = newElementOldRef[newIndex];
                hardcodedField = newIndex < hardcodedCount;
                if (newIndex >= hardcodedCount)
                {
                    v6 = parentElement->objectType;
                    if (v6 == 21)
                    {
                        value = Scr_GetArrayIndexValue(newElement->fieldName);
                        Scr_GetValueString(0, &value, 129, fieldText);
                    }
                    else if (v6 > 0x17u && v6 <= 0x19u)
                    {
                        I_strncpyz(fieldText, newElement->valueText, 129);
                    }
                    else
                    {
                        CanonicalString = Scr_GetCanonicalString(newElement->fieldName);
                        I_strncpyz(fieldText, CanonicalString, 129);
                    }
                    Scr_SetElementRefText(newElement, fieldText);
                }
                else
                {
                    Scr_SetNonFieldElementRefText(newElement);
                }
                if (oldElement)
                    Scr_DeltaElementValueText(newElement, oldElement->valueText);
                else
                    Scr_DeltaElementValueText(newElement, "");
                if (oldElement)
                    Scr_FreeWatchElementText(oldElement);
            }
            Scr_FreeDebugMem(newElementOldRef);
            if (oldElements)
                Scr_FreeDebugMem(oldElements);
            if (count)
            {
                parentElement->childCount = count;
                parentElement->hardcodedCount = hardcodedCount;
                parentElement->childArrayHead = newElements;
                Scr_ConnectElementChildren(parentElement);
                if (Scr_IsSortWatchElement(parentElement))
                    Scr_SortElementChildren(parentElement);
                for (newElement = parentElement->childHead; newElement; newElement = newElement->next)
                    Scr_ScriptWatch::EvaluateWatchChildren(newElement);
            }
            else
            {
                Scr_FreeDebugMem(newElements);
                if (parentElement->childCount)
                    Scr_FreeWatchElementChildren(parentElement);
            }
        }
        else if (parentElement->childCount)
        {
            Scr_FreeWatchElementChildren(parentElement);
        }
    }
    else
    {
        Scr_FreeWatchElementChildrenStrict(parentElement);
    }
}

void Scr_ScriptWatch::EvaluateWatchElement(Scr_WatchElement_s *element)
{
    char v2; // [esp+3h] [ebp-129h]
    char *v3; // [esp+8h] [ebp-124h]
    const char *valueText; // [esp+Ch] [ebp-120h]
    VariableValue value; // [esp+14h] [ebp-118h] BYREF
    char oldValueText[268]; // [esp+1Ch] [ebp-110h] BYREF

    Scr_ScriptWatch::EvaluateWatchElementExpression(element, &value);
    valueText = element->valueText;
    v3 = oldValueText;
    do
    {
        v2 = *valueText;
        *v3++ = *valueText++;
    } while (v2);
    Scr_ScriptWatch::PostEvaluateWatchElement(element, &value);
    Scr_PostSetText(element);
    Scr_DeltaElementValueText(element, oldValueText);
    Scr_ScriptWatch::EvaluateWatchChildren(element);
}

void  Scr_ScriptWatch::EvaluateWatchElementExpression(
    Scr_WatchElement_s *element,
    VariableValue *value)
{
    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6441, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (element->breakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6444, 0, "%s", "!element->breakpoint");
    if (!element->expr.exprHead)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6447, 0, "%s", "expr->exprHead");
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6449, 0, "%s", "scrVarPub.evaluate");
    if (element->valueDefined && (element->breakpointType == 1 || element->breakpointType == 3))
        Scr_EvalScriptExpression(&element->expr, this->localId, value, 1, 0);
    else
        Scr_EvalScriptExpression(&element->expr, this->localId, value, 0, 0);
}


bool __thiscall Scr_ScriptWatch::PostEvaluateWatchElement(
    Scr_WatchElement_s *element,
    VariableValue *value)
{
    int type; // eax
    unsigned int intValue; // [esp+0h] [ebp-118h]
    char valueText[268]; // [esp+8h] [ebp-110h] BYREF

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6307, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6310, 0, "%s", "scrVarPub.evaluate");
    Scr_RemoveValue(element);
    if (scrVarPub.error_message)
    {
        Com_sprintf(valueText, 0x101u, "<%s>", scrVarPub.error_message);
        ReplaceString(&element->valueText, valueText);
        element->objectId = 0;
        Scr_ClearErrorMessage();
        RemoveRefToValue(value->type, value->u);
        return 0;
    }
    else
    {
        Scr_GetValueString(this->localId, value, 257, valueText);
        ReplaceString(&element->valueText, valueText);
        if (value->type == 1)
            intValue = value->u.intValue;
        else
            intValue = 0;
        element->objectId = intValue;
        if (element->objectId || element->breakpointType == 1 || element->breakpointType == 3)
        {
            element->valueDefined = 1;
            type = value->type;
            element->value.u.intValue = value->u.intValue;
            element->value.type = (Vartype_t)type;
        }
        else
        {
            RemoveRefToValue(value->type, value->u);
        }
        return 1;
    }
}

void Scr_ScriptList::Shutdown()
{
    Scr_ScriptWindow *v2; // [esp+2Ch] [ebp-18h]
    char *filename; // [esp+30h] [ebp-14h]
    Scr_ScriptWindow *comp; // [esp+34h] [ebp-10h]
    int f; // [esp+38h] [ebp-Ch]
    char *lineString; // [esp+3Ch] [ebp-8h]
    int i; // [esp+40h] [ebp-4h]
    int ia; // [esp+40h] [ebp-4h]

    f = FS_FOpenTextFileWrite("script_pos.txt");
    if (f)
    {
        for (i = 0; i < this->numLines; ++i)
        {
            comp = this->scriptWindows[i];
            //filename = Scr_ScriptWindow::GetFilename(comp);
            filename = comp->GetFilename();
            if (*filename)
            {
                lineString = va("%s %d %f %f\n", filename, comp->selectedLine, comp->pos[0], comp->pos[1]);
                FS_Write(lineString, strlen(lineString), f);
            }
        }
        FS_FCloseFile(f);
    }
    //if (!scrDebuggerGlob.colBuf)
    //    MyAssertHandler(".\\script\\scr_debugger.cpp", 2844, 0, "%s", "scrDebuggerGlob.colBuf");
    Hunk_FreeDebugMem();
    for (ia = 0; ia < this->numLines; ++ia)
    {
        v2 = this->scriptWindows[ia];
        //if (v2)
        //    (v2->~UI_Component)(v2, 1);
        delete v2;
    }
    Scr_AbstractScriptList::Shutdown();
}

void Scr_ScriptList::LoadScriptPos()
{
    Scr_StringNode_s *node; // [esp+1Ch] [ebp-7Ch]
    // __int64 pos; // [esp+20h] [ebp-78h] BYREF
    float pos[2];
    char filename[88]; // [esp+28h] [ebp-70h] BYREF
    Scr_StringNode_s *head; // [esp+84h] [ebp-14h]
    char *buf; // [esp+88h] [ebp-10h] BYREF
    Scr_ScriptWindow *comp; // [esp+8Ch] [ebp-Ch]
    int selectedLine; // [esp+90h] [ebp-8h] BYREF
    int i; // [esp+94h] [ebp-4h]

    for (head = Scr_GetStringList("script_pos.txt", &buf); head; head = node)
    {
        node = head->next;
        sscanf(head->text, "%80s %d %f %f", filename, &selectedLine, &pos[0], &pos[1]);
        for (i = 0; i < this->numLines; ++i)
        {
            comp = this->scriptWindows[i];
            //if (!strcmp(Scr_ScriptWindow::GetFilename(comp), filename))
            if (!strcmp(comp->GetFilename(), filename))
            {
                if (selectedLine < -1 || selectedLine >= comp->numLines)
                    selectedLine = -1;
                comp->selectedLine = selectedLine;
                comp->focusOnSelectedLine = 0;
                comp->focusOnSelectedLineUser = 0;
                comp->pos[0] = pos[0];
                comp->pos[1] = pos[1];
            }
        }
        Hunk_FreeDebugMem();
    }
    if (buf)
        Hunk_FreeDebugMem();
}

char * Scr_ScriptWindow::GetFilename()
{
    if (this->bufferIndex == -1)
        return (char*)"";
    else
        return scrParserPub.sourceBufferLookup[this->bufferIndex].buf;
}

void Scr_ScriptWatch::UpdateBreakpoints(bool add)
{
    Scr_WatchElement_s *element; // [esp+8h] [ebp-4h]
    Scr_WatchElement_s *elementa; // [esp+8h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7357, 0, "%s", "!Sys_IsRemoteDebugClient()");
    for (element = this->elementHead; element; element = element->next)
    {
        if (element->breakpointType == 1 || element->breakpointType == 3)
        {
            if (element->breakpoint)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 7365, 0, "%s", "!element->breakpoint");
            if (!element->expr.exprHead)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 7367, 0, "%s", "expr->exprHead");
            scrDebuggerGlob.currentElement = element;
            scrDebuggerGlob.removeId = 0;
            scrDebuggerGlob.add = add;
            if (element->valueDefined && element->value.type == 1)
                Scr_RefToVariable(element->value.u.stringValue, 1);
            Scr_RefScriptExpression(&element->expr);
        }
    }
    if (add)
    {
        for (elementa = this->elementHead; elementa; elementa = elementa->next)
        {
            if (elementa->breakpointType == 1)
            {
                Scr_BreakOnAllAssignmentPos();
                return;
            }
        }
        Scr_UnbreakAllAssignmentPos();
    }
}

void Scr_AbstractScriptList::Shutdown()
{
    if (this->scriptWindows)
    {
        Scr_FreeDebugMem(this->scriptWindows);
        this->scriptWindows = 0;
    }
}

void Scr_AbstractScriptList::Draw(
    float x,
    float y,
    float width,
    float height,
    float compX,
    float compY)
{
    float v7; // [esp+18h] [ebp-4Ch]
    float v8; // [esp+1Ch] [ebp-48h]
    int currentLine; // [esp+24h] [ebp-40h]
    float selectColor[4]; // [esp+28h] [ebp-3Ch] BYREF
    int startLine; // [esp+38h] [ebp-2Ch]
    float startLineFrac; // [esp+3Ch] [ebp-28h]
    float currentY; // [esp+40h] [ebp-24h]
    const char *s; // [esp+44h] [ebp-20h]
    float lastHeight; // [esp+48h] [ebp-1Ch]
    float color[4]; // [esp+4Ch] [ebp-18h] BYREF
    int startCol; // [esp+5Ch] [ebp-8h]
    int col; // [esp+60h] [ebp-4h]

    UI_Component::DrawPic(x, y, width, height, 0, cls.consoleMaterial);
    CL_LookupColor(0, 0x37u, color);
    lastHeight = height - UI_Component::g.charHeight;
    startLineFrac = compY / UI_Component::g.charHeight;
    startLine = startLineFrac;
    currentY = startLine - startLineFrac;
    if (currentY < 0.0)
    {
        currentY = currentY + 1.0;
        ++startLine;
    }
    currentY = currentY * UI_Component::g.charHeight;
    currentLine = startLine;
    startCol = (compX / UI_Component::g.charWidth);
    selectColor[0] = 0.5;
    selectColor[1] = 0.5;
    selectColor[2] = 0.5;
    selectColor[3] = 1.0;
    while (lastHeight >= currentY && currentLine < this->numLines)
    {
        if (currentLine == this->selectedLine)
        {
            v8 = y + currentY;
            UI_Component::DrawPic(x, v8, width, UI_Component::g.charHeight, selectColor, sharedUiInfo.assets.whiteMaterial);
        }
        col = 0;
        //for (s = Scr_ScriptWindow::GetFilename(this->scriptWindows[currentLine++]); *s && col < startCol; ++s)
        for (s = this->scriptWindows[currentLine++]->GetFilename(); *s && col < startCol; ++s)
            ++col;
        if (*s)
        {
            v7 = y + currentY;
            UI_Component::DrawText(x, v7, width, 5, color, (char *)s);
        }
        currentY = currentY + UI_Component::g.charHeight;
    }
}

bool Scr_AbstractScriptList::AddEntryName(const char *filename, bool select)
{
    char *v3; // eax
    Scr_ScriptWindow *scriptWindow; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < scrDebuggerGlob.scriptList.numLines; ++i)
    {
        scriptWindow = scrDebuggerGlob.scriptList.scriptWindows[i];
        //v3 = Scr_ScriptWindow::GetFilename(scriptWindow);
        v3 = scriptWindow->GetFilename();
        if (!I_stricmp(v3, filename))
        {
            //Scr_AbstractScriptList::AddEntry(this, scriptWindow, select);
            AddEntry(scriptWindow, select);
            return 1;
        }
    }
    return 0;
}

void Scr_AbstractScriptList::AddEntry(Scr_ScriptWindow *scriptWindow, bool select)
{
    int selectedLine; // [esp+1Ch] [ebp-1Ch]
    int newNumLines; // [esp+24h] [ebp-14h]
    int newIndex; // [esp+28h] [ebp-10h]
    float newWidth; // [esp+2Ch] [ebp-Ch]
    int i; // [esp+30h] [ebp-8h]
    unsigned __int8 *newScriptWindows; // [esp+34h] [ebp-4h]

    if (select && this->selectedLine >= 0)
        selectedLine = this->selectedLine;
    else
        selectedLine = this->numLines;
    newIndex = selectedLine;
    for (i = 0; i < this->numLines; ++i)
    {
        if (this->scriptWindows[i] == scriptWindow)
        {
            if (selectedLine <= i)
            {
                memmove(&this->scriptWindows[selectedLine + 1], &this->scriptWindows[selectedLine], 4 * (i - selectedLine));
            }
            else
            {
                newIndex = selectedLine - 1;
                memmove(&this->scriptWindows[i], &this->scriptWindows[i + 1], 4 * (selectedLine - 1 - i));
            }
            goto found_0;
        }
    }
    newNumLines = this->numLines + 1;
    newScriptWindows = (unsigned char*)Scr_AllocDebugMem(4 * newNumLines, "Scr_AbstractScriptList::AddEntry");
    if (this->scriptWindows)
    {
        memcpy(newScriptWindows, this->scriptWindows, 4 * selectedLine);
        memcpy(
            &newScriptWindows[4 * selectedLine + 4],
            &this->scriptWindows[selectedLine],
            4 * (this->numLines - selectedLine));
        Scr_FreeDebugMem(this->scriptWindows);
    }
    this->scriptWindows = (Scr_ScriptWindow**)newScriptWindows;
    this->numLines = newNumLines;
    //newWidth = strlen(Scr_ScriptWindow::GetFilename(scriptWindow)) * UI_Component::g.charWidth;
    newWidth = strlen(scriptWindow->GetFilename()) * UI_Component::g.charWidth;
    if (this->size[0] < newWidth)
        this->size[0] = newWidth;
    UI_LinesComponent::UpdateHeight();
found_0:
    this->scriptWindows[newIndex] = scriptWindow;
    if (select)
        this->SetSelectedLineFocus(newIndex, 1);
    else
        this->SetSelectedLineFocus(-1, 1);
}

void UI_VerticalDivider::DrawTop(float x, float y, float width, float topHeight)
{
    if (!this->topComp)
        MyAssertHandler(".\\ui\\ui_component.cpp", 961, 0, "%s", "topComp");
    //(this->topComp->Draw)(this->topComp, LODWORD(x), LODWORD(y), LODWORD(width), LODWORD(topHeight), 0.0, 0.0);
    this->topComp->Draw(x, y, width, topHeight, 0.0f, 0.0f);
    if (this->topComp->mouseHeldScale[0] != 0.0)
        this->posY = UI_Component::g.cursorPos[1] + UI_Component::g.charHeight;
}

void UI_VerticalDivider::Draw(
    float x,
    float y,
    float width,
    float height,
    float compX,
    float compY)
{
    float v7; // [esp+28h] [ebp-14h]
    float v8; // [esp+2Ch] [ebp-10h]
    float topHeight; // [esp+38h] [ebp-4h]

    if (compX != 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 982, 0, "%s", "!compX");
    if (compY != 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 983, 0, "%s", "!compY");
    this->size[0] = width;
    this->size[1] = height;
    this->posY = (this->posY / UI_Component::g.charHeight) * UI_Component::g.charHeight;
    if (UI_Component::g.charHeight <= this->posY)
    {
        if (height < this->posY)
            this->posY = height;
    }
    else
    {
        this->posY = UI_Component::g.charHeight;
    }
    R_AddCmdProjectionSet2D();
    if (!this->topComp)
    {
        if (!this->bottomComp)
            return;
        goto LABEL_11;
    }
    if (!this->bottomComp)
        goto LABEL_13;
    if (compY >= this->posY)
    {
    LABEL_11:
        //(this->bottomComp->Draw)(this->bottomComp, LODWORD(x), LODWORD(y), LODWORD(width), LODWORD(height), 0.0, 0.0);
        this->bottomComp->Draw(x, y, width, height, 0.0f, 0.0f);
        return;
    }
    topHeight = this->posY - compY;
    if (height > topHeight)
    {
        v8 = height - topHeight;
        v7 = y + this->posY;
        //(this->bottomComp->Draw)(this->bottomComp, LODWORD(x), LODWORD(v7), LODWORD(width), LODWORD(v8), 0.0, 0.0);
        this->bottomComp->Draw(x, v7, width, v8, 0.0f, 0.0f);
        UI_VerticalDivider::DrawTop(x, y, width, topHeight);
    }
    else
    {
    LABEL_13:
        UI_VerticalDivider::DrawTop(x, y, width, height);
    }
}

void UI_ScrollPane::SetPos()
{
    float maxPos; // [esp+4h] [ebp-10h]
    float innerSize[2]; // [esp+8h] [ebp-Ch] BYREF
    int i; // [esp+10h] [ebp-4h]

    if (!this->comp)
        MyAssertHandler(".\\ui\\ui_component.cpp", 496, 0, "%s", "comp");
    for (i = 0; i < 2; ++i)
    {
        if (this->comp->pos[i] >= 0.0)
        {
            UI_ScrollPane::GetInnerSize(innerSize);
            maxPos = this->comp->size[i] - innerSize[i];
            if (maxPos < 0.0)
                maxPos = 0.0;
            if (maxPos < this->comp->pos[i])
                this->comp->pos[i] = maxPos;
        }
        else
        {
            this->comp->pos[i] = 0.0;
        }
    }
    //UI_LinesComponent::ClearFocus(this->comp);
    this->comp->ClearFocus();
}

void UI_ScrollPane::Draw(
    float x,
    float y,
    float width,
    float height,
    float compX,
    float compY)
{
    double v7; // st7
    float v8; // [esp+30h] [ebp-68h]
    float v9; // [esp+34h] [ebp-64h]
    int selectedLine; // [esp+38h] [ebp-60h]
    float scrollbarY; // [esp+40h] [ebp-58h]
    float scrollbarYb; // [esp+40h] [ebp-58h]
    float scrollbarYa; // [esp+40h] [ebp-58h]
    float focusRegionStart2; // [esp+44h] [ebp-54h]
    float thumbColor[4]; // [esp+48h] [ebp-50h] BYREF
    float thumbMaxSize; // [esp+58h] [ebp-40h]
    float thumbSize[2]; // [esp+5Ch] [ebp-3Ch] BYREF
    float thumbPos[2]; // [esp+64h] [ebp-34h] BYREF
    float thumbEnd; // [esp+6Ch] [ebp-2Ch]
    float focusRegionEnd; // [esp+70h] [ebp-28h]
    float innerSize[3]; // [esp+74h] [ebp-24h] BYREF
    float scrollbarX; // [esp+80h] [ebp-18h]
    float scrollBarSize; // [esp+84h] [ebp-14h]
    float thumbStart; // [esp+88h] [ebp-10h]
    float focusRegionStart; // [esp+8Ch] [ebp-Ch]
    float thumbFiller; // [esp+90h] [ebp-8h]
    bool horScroll; // [esp+97h] [ebp-1h]

    if (compX != 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 346, 0, "%s", "!compX");
    if (compY != 0.0)
        MyAssertHandler(".\\ui\\ui_component.cpp", 347, 0, "%s", "!compY");
    this->size[0] = width;
    this->size[1] = height;
    if (!this->comp || this->comp->size[0] == 0.0 || this->comp->size[1] == 0.0)
    {
        UI_Component::DrawPic(x, y, width, height, 0, cls.consoleMaterial);
    }
    else
    {
        if (this->comp->size[0] < 0.0)
            MyAssertHandler(".\\ui\\ui_component.cpp", 358, 0, "%s", "comp->size[0] >= 0");
        if (this->comp->size[1] < 0.0)
            MyAssertHandler(".\\ui\\ui_component.cpp", 359, 0, "%s", "comp->size[1] >= 0");
        horScroll = UI_ScrollPane::GetInnerSize(innerSize);
        if (this->comp->focusOnSelectedLine)
        {
            this->comp->focusOnSelectedLine = 0;
            if (this->comp->selectedLine < 0)
                selectedLine = 0;
            else
                selectedLine = this->comp->selectedLine;
            LODWORD(innerSize[2]) = selectedLine;
            focusRegionStart2 = selectedLine * UI_Component::g.charHeight;
            if (this->comp->focusOnSelectedLineUser)
            {
                focusRegionStart = selectedLine * UI_Component::g.charHeight;
                v7 = focusRegionStart2 + UI_Component::g.charHeight;
            }
            else
            {
                focusRegionStart = focusRegionStart2 - UI_Component::g.charHeight * 4.0;
                v7 = UI_Component::g.charHeight * 5.0 + focusRegionStart2;
            }
            focusRegionEnd = v7;
            if (this->comp->pos[1] <= focusRegionStart)
            {
                if (focusRegionEnd > this->comp->pos[1] + innerSize[1])
                {
                    if (focusRegionStart > this->comp->pos[1] + innerSize[1])
                    {
                        this->comp->pos[1] = focusRegionStart2 - innerSize[1] / 2.0;
                    }
                    else
                    {
                        this->comp->pos[1] = focusRegionEnd - innerSize[1];
                        if (this->comp->pos[1] > focusRegionStart2 - innerSize[1] / 2.0)
                            this->comp->pos[1] = focusRegionStart2 - innerSize[1] / 2.0;
                    }
                }
            }
            else if (this->comp->pos[1] > focusRegionEnd)
            {
                this->comp->pos[1] = focusRegionStart2 - innerSize[1] / 2.0;
            }
            else
            {
                this->comp->pos[1] = focusRegionStart;
                if (this->comp->pos[1] < focusRegionStart2 - innerSize[1] / 2.0)
                    this->comp->pos[1] = focusRegionStart2 - innerSize[1] / 2.0;
            }
        }
        UI_ScrollPane::SetPos();
        thumbColor[0] = 0.5;
        thumbColor[1] = 0.5;
        thumbColor[2] = 0.5;
        thumbColor[3] = 1.0;
        //(this->comp->Draw)(
        //    this->comp,
        //    LODWORD(x),
        //    LODWORD(y),
        //    LODWORD(innerSize[0]),
        //    LODWORD(innerSize[1]),
        //    this->comp->pos[0],
        //    this->comp->pos[1]);
        this->comp->Draw(x, y, innerSize[0], innerSize[1], this->comp->pos[0], this->comp->pos[1]);
        scrollbarX = x + innerSize[0];
        UI_Component::DrawPic(scrollbarX, y, UI_Component::g.scrollBarSize, height, 0, cls.consoleMaterial);
        UI_Component::DrawPic(
            scrollbarX,
            y,
            UI_Component::g.scrollBarSize,
            UI_Component::g.scrollBarSize,
            0,
            sharedUiInfo.assets.scrollBarArrowUp);
        scrollbarY = y + UI_Component::g.scrollBarSize;
        scrollBarSize = innerSize[1] - (UI_Component::g.scrollBarSize + UI_Component::g.scrollBarSize);
        if (UI_Component::g.scrollBarSize >= scrollBarSize)
            v9 = scrollBarSize;
        else
            v9 = UI_Component::g.scrollBarSize;
        thumbFiller = v9;
        UI_Component::DrawPic(
            scrollbarX,
            scrollbarY,
            UI_Component::g.scrollBarSize,
            scrollBarSize,
            0,
            sharedUiInfo.assets.scrollBar);
        scrollbarYb = scrollbarY + scrollBarSize;
        UI_Component::DrawPic(
            scrollbarX,
            scrollbarYb,
            UI_Component::g.scrollBarSize,
            UI_Component::g.scrollBarSize,
            0,
            sharedUiInfo.assets.scrollBarArrowDown);
        thumbMaxSize = scrollBarSize - thumbFiller;
        thumbStart = this->comp->pos[1] / this->comp->size[1] * thumbMaxSize;
        thumbEnd = (this->comp->pos[1] + innerSize[1]) / this->comp->size[1] * thumbMaxSize + thumbFiller;
        if (scrollBarSize < thumbEnd)
            thumbEnd = scrollBarSize;
        thumbPos[0] = scrollbarX;
        thumbPos[1] = y + UI_Component::g.scrollBarSize + thumbStart;
        thumbSize[0] = UI_Component::g.scrollBarSize;
        thumbSize[1] = thumbEnd - thumbStart;
        UI_Component::DrawPic(
            scrollbarX,
            thumbPos[1],
            UI_Component::g.scrollBarSize,
            thumbSize[1],
            thumbColor,
            sharedUiInfo.assets.whiteMaterial);
        UI_ScrollPane::CheckMouseScroll(1, thumbPos, thumbSize, thumbMaxSize);
        if (horScroll)
        {
            scrollbarYa = y + innerSize[1];
            UI_Component::DrawPic(x, scrollbarYa, innerSize[0], UI_Component::g.scrollBarSize, 0, cls.consoleMaterial);
            scrollbarX = x;
            UI_Component::DrawPicRotate(
                x,
                scrollbarYa,
                UI_Component::g.scrollBarSize,
                UI_Component::g.scrollBarSize,
                0,
                sharedUiInfo.assets.scrollBarArrowUp);
            scrollbarX = scrollbarX + UI_Component::g.scrollBarSize;
            scrollBarSize = innerSize[0] - (UI_Component::g.scrollBarSize + UI_Component::g.scrollBarSize);
            if (UI_Component::g.scrollBarSize >= scrollBarSize)
                v8 = scrollBarSize;
            else
                v8 = UI_Component::g.scrollBarSize;
            thumbFiller = v8;
            UI_Component::DrawPicRotate(
                scrollbarX,
                scrollbarYa,
                scrollBarSize,
                UI_Component::g.scrollBarSize,
                0,
                sharedUiInfo.assets.scrollBar);
            scrollbarX = scrollbarX + scrollBarSize;
            UI_Component::DrawPicRotate(
                scrollbarX,
                scrollbarYa,
                UI_Component::g.scrollBarSize,
                UI_Component::g.scrollBarSize,
                0,
                sharedUiInfo.assets.scrollBarArrowDown);
            thumbMaxSize = scrollBarSize - thumbFiller;
            thumbStart = this->comp->pos[0] / this->comp->size[0] * thumbMaxSize;
            thumbEnd = (this->comp->pos[0] + innerSize[0]) / this->comp->size[0] * thumbMaxSize + thumbFiller;
            if (scrollBarSize < thumbEnd)
                thumbEnd = scrollBarSize;
            thumbPos[0] = x + UI_Component::g.scrollBarSize + thumbStart;
            thumbPos[1] = scrollbarYa;
            thumbSize[0] = thumbEnd - thumbStart;
            thumbSize[1] = UI_Component::g.scrollBarSize;
            UI_Component::DrawPicRotate(
                thumbPos[0],
                scrollbarYa,
                thumbSize[0],
                UI_Component::g.scrollBarSize,
                thumbColor,
                sharedUiInfo.assets.whiteMaterial);
            UI_ScrollPane::CheckMouseScroll(0, thumbPos, thumbSize, thumbMaxSize);
        }
    }
}

char __cdecl InRect(float *point, float *pos, float *size)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 2; ++i)
    {
        if (pos[i] > point[i])
            return 0;
        if (point[i] >= pos[i] + size[i])
            return 0;
    }
    return 1;
}

void UI_ScrollPane::CheckMouseScroll(int index, float *thumbPos, float *thumbSize, float thumbMaxSize)
{
    float *pos; // eax

    if (Key_IsDown(0, 200))
    {
        if (this->mouseHeldScale[index] == 0.0)
        {
            if (!UI_Component::g.hideCursor && InRect(UI_Component::g.cursorPos, thumbPos, thumbSize))
            {
                if (!this->mouseWasDown[index])
                {
                    if (this->mouseHeldScale[0] != 0.0)
                        MyAssertHandler(".\\ui\\ui_component.cpp", 310, 0, "%s", "!mouseHeldScale[0]");
                    if (this->mouseHeldScale[1] != 0.0)
                        MyAssertHandler(".\\ui\\ui_component.cpp", 311, 0, "%s", "!mouseHeldScale[1]");
                    if (thumbMaxSize == 0.0)
                        MyAssertHandler(".\\ui\\ui_component.cpp", 312, 0, "%s", "thumbMaxSize");
                    this->mouseHeldScale[index] = this->comp->size[index] / thumbMaxSize;
                    this->mouseHeldPos[0] = UI_Component::g.cursorPos[0];
                    this->mouseHeldPos[1] = UI_Component::g.cursorPos[1];
                    pos = this->comp->pos;
                    this->mouseHeldCompPos[0] = *pos;
                    this->mouseHeldCompPos[1] = pos[1];
                }
            }
            else
            {
                this->mouseWasDown[index] = 1;
            }
        }
        else
        {
            this->comp->pos[index] = (UI_Component::g.cursorPos[index] - this->mouseHeldPos[index])
                * this->mouseHeldScale[index]
                + this->mouseHeldCompPos[index];
                UI_ScrollPane::SetPos();
        }
    }
    else
    {
        this->mouseWasDown[index] = 0;
        this->mouseHeldScale[0] = 0.0;
        this->mouseHeldScale[1] = 0.0;
    }
}

void UI_Component::Shutdown()
{
}

UI_Component::~UI_Component()
{
}

void UI_Component::Draw(float one, float two, float three, float four, float five, float six)
{
}

bool UI_Component::KeyEvent(float *pos, int key)
{
    return false;
}

void UI_Component::DrawPicRotate(
    float x,
    float y,
    float width,
    float height,
    const float *color,
    Material *material)
{
    float v6; // [esp+8h] [ebp-28h]
    float v7; // [esp+Ch] [ebp-24h]
    float verts[4][2]; // [esp+10h] [ebp-20h] BYREF

    v7 = floor(x);
    verts[1][0] = v7;
    v6 = floor(y);
    verts[1][1] = v6;
    verts[2][0] = v7 + width;
    verts[2][1] = v6;
    verts[3][0] = verts[2][0];
    verts[3][1] = v6 + height;
    verts[0][0] = v7;
    verts[0][1] = verts[3][1];
    R_AddCmdDrawQuadPic(verts, color, material);
}

void UI_Component::DrawPic(float x, float y, float width, float height, const float *color, Material *material)
{
    float v6; // [esp+2Ch] [ebp-Ch]
    float v7; // [esp+30h] [ebp-8h]

    v7 = floor(y);
    v6 = floor(x);
    R_AddCmdDrawStretchPic(v6, v7, width, height, 0.0, 0.0, 1.0, 1.0, color, material);
}

void Scr_ScriptWindow::Draw(float x,
    float y,
    float width,
    float height,
    float compX,
    float compY)
{
    float v7; // [esp+18h] [ebp-BCh]
    float v8; // [esp+1Ch] [ebp-B8h]
    float v9; // [esp+20h] [ebp-B4h]
    float v10; // [esp+24h] [ebp-B0h]
    float v11; // [esp+28h] [ebp-ACh]
    float v12; // [esp+2Ch] [ebp-A8h]
    float v13; // [esp+30h] [ebp-A4h]
    float v14; // [esp+34h] [ebp-A0h]
    float v15; // [esp+38h] [ebp-9Ch]
    float v16; // [esp+3Ch] [ebp-98h]
    float colorYellow[4]; // [esp+44h] [ebp-90h] BYREF
    float selectColor[4]; // [esp+54h] [ebp-80h] BYREF
    int currentLine; // [esp+64h] [ebp-70h]
    Scr_Breakpoint *breakpoint; // [esp+68h] [ebp-6Ch]
    float colorWhite[4]; // [esp+6Ch] [ebp-68h] BYREF
    int startLine; // [esp+7Ch] [ebp-58h]
    const char *endPos; // [esp+80h] [ebp-54h]
    float colorBlue[4]; // [esp+84h] [ebp-50h] BYREF
    float colorRed[4]; // [esp+94h] [ebp-40h] BYREF
    float startLineFrac; // [esp+A4h] [ebp-30h]
    SourceBufferInfo *sourceBufData; // [esp+A8h] [ebp-2Ch]
    float currentY; // [esp+ACh] [ebp-28h]
    float innerWidth; // [esp+B0h] [ebp-24h]
    const char *s; // [esp+B4h] [ebp-20h]
    float lastHeight; // [esp+B8h] [ebp-1Ch]
    char *to; // [esp+BCh] [ebp-18h]
    int i; // [esp+C0h] [ebp-14h]
    Scr_WatchElement_s *element; // [esp+C4h] [ebp-10h]
    int startCol; // [esp+C8h] [ebp-Ch]
    int col; // [esp+CCh] [ebp-8h]
    bool existsBreakpoint; // [esp+D3h] [ebp-1h]

    colorBlue[0] = 0.2f;
    colorBlue[1] = 0.2f;
    colorBlue[2] = 0.40000001f;
    colorBlue[3] = 1.0f;
    if (scrDebuggerGlob.atBreakpoint)
        UI_Component::DrawPic(x, y, width, height, colorBlue, cls.whiteMaterial);
    else
        UI_Component::DrawPic(x, y, width, height, 0, cls.consoleMaterial);
    if (this->bufferIndex != -1)
    {
        if (this->bufferIndex >= scrParserPub.sourceBufferLookupLen)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 828, 0, "%s", "bufferIndex < scrParserPub.sourceBufferLookupLen");
        sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
        CL_LookupColor(0, 0x37u, colorWhite);
        colorRed[0] = 1.0;
        colorRed[1] = 0.0;
        colorRed[2] = 0.0;
        colorRed[3] = 1.0;
        CL_LookupColor(0, 0x33u, colorYellow);
        innerWidth = width - UI_Component::g.charHeight;
        lastHeight = height - UI_Component::g.charHeight;
        startLineFrac = compY / UI_Component::g.charHeight;
        startLine = startLineFrac;
        currentY = startLine - startLineFrac;
        if (currentY < 0.0)
        {
            currentY = currentY + 1.0;
            ++startLine;
        }
        currentY = currentY * UI_Component::g.charHeight;
        Scr_ScriptWindow::SetCurrentLine(startLine);
        currentLine = this->currentTopLine;
        s = this->currentBufPos;
        endPos = &sourceBufData->sourceBuf[sourceBufData->len];
        startCol = (compX / UI_Component::g.charWidth);
        selectColor[0] = 0.5;
        selectColor[1] = 0.5;
        selectColor[2] = 0.5;
        selectColor[3] = 1.0;
        for (breakpoint = this->breakpointHead; breakpoint && breakpoint->line < currentLine; breakpoint = breakpoint->next)
            ;
        existsBreakpoint = this->bufferIndex == scrDebuggerGlob.breakpointPos.bufferIndex;
        while (lastHeight >= currentY && s < endPos)
        {
            if (currentLine == this->selectedLine)
            {
                v16 = y + currentY;
                v15 = x + UI_Component::g.charHeight;
                UI_Component::DrawPic(
                    v15,
                    v16,
                    innerWidth,
                    UI_Component::g.charHeight,
                    selectColor,
                    sharedUiInfo.assets.whiteMaterial);
            }
            if (breakpoint && breakpoint->line == currentLine)
            {
                element = breakpoint->element;
                if (!element)
                    MyAssertHandler(".\\script\\scr_debugger.cpp", 872, 0, "%s", "element");
                if (element->breakpointType == 5)
                {
                    v14 = UI_Component::g.charHeight + UI_Component::g.charHeight;
                    v13 = y + currentY - UI_Component::g.charHeight * 0.5;
                    UI_Component::DrawPic(x, v13, UI_Component::g.charHeight, v14, colorRed, UI_Component::g.filledCircle);
                }
                else
                {
                    if (element->breakpointType != 4)
                        MyAssertHandler(
                            ".\\script\\scr_debugger.cpp",
                            879,
                            0,
                            "%s",
                            "element->breakpointType == SCR_BREAKPOINT_LINE_DISABLED");
                    v12 = y + currentY;
                    v11 = UI_Component::g.charWidth * 0.5 + x;
                    UI_Component::DrawText(v11, v12, UI_Component::g.charHeight, 5, colorRed, (char*)"O");
                }
                do
                    breakpoint = breakpoint->next;
                while (breakpoint && breakpoint->line == currentLine);
            }
            if (existsBreakpoint && currentLine == scrDebuggerGlob.breakpointPos.lineNum)
            {
                v10 = y + currentY;
                v9 = UI_Component::g.charWidth * 0.5 + x;
                UI_Component::DrawText(v9, v10, UI_Component::g.charHeight, 5, colorYellow, (char*)">");
            }
            ++currentLine;
            col = 0;
            while (*s && col < startCol)
            {
                if (*s == 9)
                    col += 4 - col % 4;
                else
                    ++col;
                ++s;
            }
            if (*s)
            {
                to = scrDebuggerGlob.colBuf;
                i = col - startCol;
                memset(scrDebuggerGlob.colBuf, 32, col - startCol);
                to += i;
                i = 0;
                while (*s)
                {
                    if (*s == 9)
                    {
                        for (i = col % 4; i < 4; ++i)
                        {
                            *to++ = 32;
                            ++col;
                        }
                    }
                    else
                    {
                        *to++ = *s;
                        ++col;
                    }
                    ++s;
                }
                *to = 0;
                v8 = y + currentY;
                v7 = x + UI_Component::g.charHeight;
                UI_Component::DrawText(v7, v8, innerWidth, 5, colorWhite, scrDebuggerGlob.colBuf);
            }
            ++s;
            currentY = currentY + UI_Component::g.charHeight;
        }
    }
}

UI_Component *UI_VerticalDivider::GetCompAtLocation(float *point)
{
    if (this->topComp)
    {
        if (this->bottomComp && this->posY <= point[1])
        {
            point[1] = point[1] - this->posY;
            return this->bottomComp->GetCompAtLocation(point);
        }
        else
        {
            return this->topComp->GetCompAtLocation(point);
        }
    }
    else if (this->bottomComp)
    {
        return this->bottomComp->GetCompAtLocation(point);
    }
    else
    {
        return this;
    }
}

bool UI_VerticalDivider::KeyEvent(float *point, int key)
{
    if (!Key_IsDown(0, 158) || Key_IsDown(0, 159) || Key_IsDown(0, 160))
        return 0;
    if (key == 154)
    {
        this->posY = this->posY - UI_Component::g.charHeight;
        return 1;
    }
    if (key != 155)
        return 0;
    this->posY = this->posY + UI_Component::g.charHeight;
    return 1;
}

bool UI_ScrollPane::KeyEvent(float *point, int key)
{
    bool result; // al
    UI_LinesComponent *comp; // eax
    float innerSize[2]; // [esp+14h] [ebp-1Ch] BYREF
    float innerPoint[2]; // [esp+1Ch] [ebp-14h] BYREF
    int linesCount; // [esp+24h] [ebp-Ch]
    float vec2_origin[2]; // [esp+28h] [ebp-8h] BYREF

    if (!this->comp)
        return 0;
    if (Key_IsDown(0, 158) || Key_IsDown(0, 159) || Key_IsDown(0, 160))
    {
        if (Key_IsDown(0, 158) || !Key_IsDown(0, 159) || Key_IsDown(0, 160))
        {
        LABEL_23:
            if (key != 200
                || (vec2_origin[0] = 0.0,
                    vec2_origin[1] = 0.0,
                    UI_ScrollPane::GetInnerSize(innerSize),
                    InRect(point, vec2_origin, innerSize)))
            {
                comp = this->comp;
                innerPoint[0] = *point + comp->pos[0];
                innerPoint[1] = point[1] + comp->pos[1];
                return this->comp->KeyEvent(innerPoint, key);
            }
            else
            {
                return 0;
            }
        }
        else
        {
            switch (key)
            {
            case 154:
                this->comp->pos[1] = this->comp->pos[1] - UI_Component::g.charHeight;
                UI_ScrollPane::SetPos();
                result = 1;
                break;
            case 155:
                this->comp->pos[1] = this->comp->pos[1] + UI_Component::g.charHeight;
                UI_ScrollPane::SetPos();
                result = 1;
                break;
            case 156:
                this->comp->pos[0] = this->comp->pos[0] - UI_Component::g.charWidth;
                UI_ScrollPane::SetPos();
                result = 1;
                break;
            case 157:
                this->comp->pos[0] = this->comp->pos[0] + UI_Component::g.charWidth;
                UI_ScrollPane::SetPos();
                result = 1;
                break;
            case 163:
                this->comp->selectedLine = 0x7FFFFFFF;
                UI_ScrollPane::DisplaySelectedLine();
                result = 1;
                break;
            case 164:
                this->comp->selectedLine = 0;
                UI_ScrollPane::DisplaySelectedLine();
                result = 1;
                break;
            case 165:
                this->comp->pos[1] = 0.0;
                this->comp->selectedLine = 0;
                UI_ScrollPane::SetPos();
                UI_ScrollPane::DisplaySelectedLine();
                result = 1;
                break;
            case 166:
                this->comp->pos[1] = FLT_MAX;
                this->comp->selectedLine = 0x7FFFFFFF;
                UI_ScrollPane::SetPos();
                UI_ScrollPane::DisplaySelectedLine();
                result = 1;
                break;
            default:
                goto LABEL_23;
            }
        }
    }
    else
    {
        switch (key)
        {
        case 163:
            linesCount = UI_ScrollPane::GetInnerLinesCount();
            this->comp->pos[1] = linesCount * UI_Component::g.charHeight + this->comp->pos[1];
            this->comp->selectedLine += linesCount;
            UI_ScrollPane::SetPos();
            UI_ScrollPane::DisplaySelectedLine();
            result = 1;
            break;
        case 164:
            linesCount = UI_ScrollPane::GetInnerLinesCount();
            this->comp->pos[1] = this->comp->pos[1] - linesCount * UI_Component::g.charHeight;
            this->comp->selectedLine -= linesCount;
            UI_ScrollPane::SetPos();
            UI_ScrollPane::DisplaySelectedLine();
            result = 1;
            break;
        case 205:
            this->comp->pos[1] = UI_Component::g.charHeight * 3.0 + this->comp->pos[1];
            UI_ScrollPane::SetPos();
            result = 1;
            break;
        case 206:
            this->comp->pos[1] = this->comp->pos[1] - UI_Component::g.charHeight * 3.0;
            UI_ScrollPane::SetPos();
            result = 1;
            break;
        default:
            goto LABEL_23;
        }
    }
    return result;
}

int UI_ScrollPane::GetInnerLinesCount()
{
    float innerSize[2]; // [esp+4h] [ebp-8h] BYREF

    UI_ScrollPane::GetInnerSize(innerSize);
    return (innerSize[1] / UI_Component::g.charHeight);
}

int UI_ScrollPane::GetFirstDisplayedLine()
{
    float startLineFrac; // [esp+Ch] [ebp-8h]
    float startY; // [esp+10h] [ebp-4h]

    startLineFrac = this->comp->pos[1] / UI_Component::g.charHeight;
    startY = startLineFrac - startLineFrac;
    if (startY >= 0.0)
        return startLineFrac;
    else
        return startLineFrac + 1;
}

int UI_ScrollPane::GetLastDisplayedLine()
{
    int lastLine; // [esp+8h] [ebp-Ch]
    float innerSize[2]; // [esp+Ch] [ebp-8h] BYREF

    if (!this->comp)
        MyAssertHandler(".\\ui\\ui_component.cpp", 548, 0, "%s", "comp");
    UI_ScrollPane::GetInnerSize(innerSize);
    lastLine = ((this->comp->pos[1] + innerSize[1]) / UI_Component::g.charHeight);
    if (innerSize[1] < (lastLine + 1) * UI_Component::g.charHeight)
        --lastLine;
    if (lastLine >= this->comp->numLines)
        return this->comp->numLines - 1;
    return lastLine;
}

void UI_ScrollPane::DisplaySelectedLine()
{
    int lastLine; // [esp+4h] [ebp-8h]
    int firstLine; // [esp+8h] [ebp-4h]

    if (!this->comp)
        MyAssertHandler(".\\ui\\ui_component.cpp", 571, 0, "%s", "comp");
    firstLine = UI_ScrollPane::GetFirstDisplayedLine();
    if (this->comp->selectedLine >= firstLine)
    {
        lastLine = UI_ScrollPane::GetLastDisplayedLine();
        if (this->comp->selectedLine > lastLine)
            this->comp->selectedLine = lastLine;
    }
    else
    {
        this->comp->selectedLine = firstLine;
    }
}

bool UI_LinesComponent::KeyEvent(float *point, int key)
{
    bool result; // al

    if (Key_IsDown(0, 158) || Key_IsDown(0, 159) || Key_IsDown(0, 160))
    {
        if (Key_IsDown(0, 158) || !Key_IsDown(0, 159) || Key_IsDown(0, 160))
            return 0;
        if (key == 103)
        {
            UI_Component::g.consoleReason = 0;
            Con_ToggleConsole();
            return 1;
        }
        if (key == 200)
        {
            if (this->selectedLine == (point[1] / UI_Component::g.charHeight))
                this->selectedLine = -1;
            return 0;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        switch (key)
        {
        case 45:
            this->selectedLine = -1;
            result = 1;
            break;
        case 154:
            UI_LinesComponent::DecSelectedLineFocus(0);
            result = 1;
            break;
        case 155:
            UI_LinesComponent::IncSelectedLineFocus(0);
            result = 1;
            break;
        case 200:
            this->SetSelectedLineFocus((point[1] / UI_Component::g.charHeight), 1);
            result = 0;
            break;
        default:
            return 0;
        }
    }
    return result;
}

void Scr_ScriptWindow::CopySelectedText()
{
    int currentLine; // [esp+4h] [ebp-10h]
    const char *endPos; // [esp+8h] [ebp-Ch]
    SourceBufferInfo *sourceBufData; // [esp+Ch] [ebp-8h]
    const char *s; // [esp+10h] [ebp-4h]

    if (this->bufferIndex != -1 && this->selectedLine >= 0)
    {
        if (this->bufferIndex >= scrParserPub.sourceBufferLookupLen)
            MyAssertHandler(
                ".\\script\\scr_debugger.cpp",
                2032,
                0,
                "bufferIndex doesn't index scrParserPub.sourceBufferLookupLen\n\t%i not in [0, %i)",
                this->bufferIndex,
                scrParserPub.sourceBufferLookupLen);
        sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
        s = sourceBufData->sourceBuf;
        endPos = &s[sourceBufData->len];
        currentLine = 0;
        while (s < endPos)
        {
            if (currentLine == this->selectedLine)
            {
                while (*s == 9 || *s == 32)
                    ++s;
                Sys_SetClipboardData(s);
                return;
            }
            ++currentLine;
            while (*s)
                ++s;
            ++s;
        }
    }
}

void Scr_ScriptWindow::ToggleBreakpoint(
    Scr_WatchElement_s *element,
    bool force,
    bool overwrite,
    unsigned __int8 breakpointType,
    bool user)
{
    SourceBufferInfo *sourceBufData; // [esp+4h] [ebp-4h]

    if (this->selectedLine >= 0)
    {
        sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
        if (Sys_IsRemoteDebugClient())
        {
            Sys_WriteDebugSocketMessageType(0x14u);
            Sys_WriteDebugSocketInt(sourceBufData->sortedIndex);
            Sys_WriteDebugSocketInt(this->selectedLine);
            Scr_WriteElement(element);
            Sys_WriteDebugSocketInt(force);
            Sys_WriteDebugSocketInt(overwrite);
            Sys_WriteDebugSocketInt(breakpointType);
            Sys_WriteDebugSocketInt(user);
            Sys_EndWriteDebugSocket();
        }
        else
        {
            Scr_ScriptWindow::ToggleBreakpointInternal(element, force, overwrite, breakpointType, user);
        }
    }
}

char *__thiscall Scr_ScriptWindow::GetBreakpointCodePos()
{
    unsigned int startSourcePos; // [esp+4h] [ebp-1Ch]
    SourceBufferInfo *sourceBufData; // [esp+8h] [ebp-18h]
    const char *s; // [esp+Ch] [ebp-14h]
    int line; // [esp+10h] [ebp-10h]
    char *codePos; // [esp+14h] [ebp-Ch]
    unsigned int sourcePos; // [esp+18h] [ebp-8h] BYREF
    unsigned int endSourcePos; // [esp+1Ch] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1355, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (this->selectedLine < 0)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1358, 0, "%s", "selectedLine >= 0");
    sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
    s = sourceBufData->sourceBuf;
    for (line = 0; ; ++line)
    {
        if (s - sourceBufData->sourceBuf > sourceBufData->len)
            MyAssertHandler(
                ".\\script\\scr_debugger.cpp",
                1366,
                0,
                "%s",
                "s - sourceBufData->sourceBuf <= sourceBufData->len");
        if (line == this->selectedLine)
            break;
    LABEL_18:
        while (*s)
            ++s;
        ++s;
    }
    startSourcePos = s - sourceBufData->sourceBuf;
    while (*s)
        ++s;
    endSourcePos = s - sourceBufData->sourceBuf;
    codePos = (char*)Scr_GetOpcodePosOfType(this->bufferIndex, startSourcePos, endSourcePos, 1, &sourcePos);
    if (codePos)
        return codePos;
    if (this->selectedLine < this->numLines - 1)
    {
        ++this->selectedLine;
        if (*s)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 1383, 0, "%s", "!(*s)");
        goto LABEL_18;
    }
    return 0;
}

void Scr_ScriptWindow::RunToCursor()
{
    SourceBufferInfo *sourceBufData; // [esp+4h] [ebp-8h]
    char *codePos; // [esp+8h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
    {
        if (scrDebuggerGlob.atBreakpoint)
        {
            if (this->selectedLine < 0)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 1669, 0, "%s", "selectedLine >= 0");
            sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
            Sys_WriteDebugSocketMessageType(0x19u);
            Sys_WriteDebugSocketInt(sourceBufData->sortedIndex);
            Sys_WriteDebugSocketInt(this->selectedLine);
            Sys_EndWriteDebugSocket();
            Scr_ResumeBreakpoints();
        }
    }
    else
    {
        clientUIActives[0].keyCatchers &= ~2u;
        if (scrVmPub.function_count)
        {
            codePos = Scr_ScriptWindow::GetBreakpointCodePos();
            Scr_SetTempBreakpoint(codePos, 0);
        }
    }
}

Scr_WatchElement_s *__thiscall Scr_ScriptWatch::GetSelectedNonConditionalElement()
{
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    element = Scr_ScriptWatch::GetSelectedElement();
    if (element)
    {
        while (element->next && element->next->breakpointType == 2)
            element = element->next;
    }
    return element;
}

void Scr_ScriptWindow::EnterCallInternal()
{
    const char *v2; // [esp+4h] [ebp-30h]
    Scr_SourcePos_t pos; // [esp+Ch] [ebp-28h] BYREF
    Scr_Breakpoint *breakpoint; // [esp+18h] [ebp-1Ch]
    unsigned int startSourcePos; // [esp+1Ch] [ebp-18h] BYREF
    const char *codePos; // [esp+20h] [ebp-14h]
    unsigned int sourcePos; // [esp+24h] [ebp-10h] BYREF
    Scr_Breakpoint **pBreakpoint; // [esp+28h] [ebp-Ch]
    const char *destCodePos; // [esp+2Ch] [ebp-8h]
    unsigned int endSourcePos; // [esp+30h] [ebp-4h] BYREF

    if (this->selectedLine < 0)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1774, 0, "%s", "selectedLine >= 0");
    Scr_ScriptWindow::GetSourcePos(&startSourcePos, &endSourcePos);
    codePos = Scr_GetOpcodePosOfType(this->bufferIndex, startSourcePos, endSourcePos, 2, &sourcePos);
    if (codePos)
    {
        for (pBreakpoint = &this->breakpointHead; ; pBreakpoint = &breakpoint->next)
        {
            breakpoint = *pBreakpoint;
            if (!breakpoint)
                break;
            if (breakpoint->line >= this->selectedLine)
            {
                if (breakpoint->line != this->selectedLine)
                    breakpoint = 0;
                break;
            }
        }
        if (breakpoint)
        {
            //Scr_ScriptWatch::SetSelectedElement(&scrDebuggerGlob.scriptWatch, breakpoint->element, 0);
            scrDebuggerGlob.scriptWatch.SetSelectedElement(breakpoint->element, 0);
        }
        else
        {
            //Scr_ScriptWindow::ToggleBreakpointInternal(this, 0, 0, 0, 4u, 1);
            Scr_ScriptWindow::ToggleBreakpointInternal(0, 0, 0, 4u, 1);
        }
        v2 = (const char*)*++codePos;
        codePos += 4;
        destCodePos = v2;
        Scr_GetSourcePosOfType(v2 - 1, 4, &pos);
        Scr_SelectScriptLine(pos.bufferIndex, pos.lineNum);
    }
}

void Scr_ScriptWindow::EnterCall()
{
    SourceBufferInfo *sourceBufData; // [esp+4h] [ebp-4h]

    if (this->selectedLine >= 0)
    {
        sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
        if (Sys_IsRemoteDebugClient())
        {
            Sys_WriteDebugSocketMessageType(0x2Du);
            Sys_WriteDebugSocketInt(sourceBufData->sortedIndex);
            Sys_WriteDebugSocketInt(this->selectedLine);
            Sys_EndWriteDebugSocket();
        }
        else
        {
            Scr_ScriptWindow::EnterCallInternal();
        }
    }
}

bool Scr_ScriptWindow::KeyEvent(float *point, int key)
{
    bool result; // al
    Scr_WatchElement_s *v4; // eax
    Scr_WatchElement_s *v5; // eax
    Scr_WatchElement_s *SelectedNonConditionalElement; // eax
    Scr_WatchElement_s *v7; // eax

    if (Key_IsDown(0, 158) || Key_IsDown(0, 159) || Key_IsDown(0, 160))
    {
        if (Key_IsDown(0, 158) || !Key_IsDown(0, 159) || Key_IsDown(0, 160))
        {
            return UI_LinesComponent::KeyEvent(point, key);
        }
        else
        {
            switch (key)
            {
            case 99:
            case 161:
                Scr_ScriptWindow::CopySelectedText();
                result = 1;
                break;
            case 173:
                //SelectedNonConditionalElement = Scr_ScriptWatch::GetSelectedNonConditionalElement(&scrDebuggerGlob.scriptWatch);
                SelectedNonConditionalElement = scrDebuggerGlob.scriptWatch.GetSelectedNonConditionalElement();
                Scr_ScriptWindow::ToggleBreakpoint(SelectedNonConditionalElement, 0, 0, 7u, 1);
                result = 1;
                break;
            case 175:
                //v7 = Scr_ScriptWatch::GetSelectedNonConditionalElement(&scrDebuggerGlob.scriptWatch);
                v7 = scrDebuggerGlob.scriptWatch.GetSelectedNonConditionalElement();
                Scr_ScriptWindow::ToggleBreakpoint(v7, 0, 0, 4u, 1);
                result = 1;
                break;
            case 176:
                Scr_ScriptWindow::RunToCursor();
                result = 1;
                break;
            default:
                return UI_LinesComponent::KeyEvent(point, key);
            }
        }
    }
    else
    {
        switch (key)
        {
        case 13:
        case 191:
        case 223:
            Scr_ScriptWindow::EnterCall();
            result = 1;
            break;
        case 173:
            //v4 = Scr_ScriptWatch::GetSelectedNonConditionalElement(&scrDebuggerGlob.scriptWatch);
            v4 = scrDebuggerGlob.scriptWatch.GetSelectedNonConditionalElement();
            Scr_ScriptWindow::ToggleBreakpoint(v4, 0, 0, 6u, 1);
            result = 1;
            break;
        case 175:
            //v5 = Scr_ScriptWatch::GetSelectedNonConditionalElement(&scrDebuggerGlob.scriptWatch);
            v5 = scrDebuggerGlob.scriptWatch.GetSelectedNonConditionalElement();
            Scr_ScriptWindow::ToggleBreakpoint(v5, 0, 0, 5u, 1);
            result = 1;
            break;
        default:
            return UI_LinesComponent::KeyEvent(point, key);
        }
    }
    return result;
}