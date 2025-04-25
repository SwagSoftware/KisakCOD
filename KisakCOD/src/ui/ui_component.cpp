#include "ui.h"
#include <client_mp/client_mp.h>
#include <win32/win_net_debug.h>
#include <script/scr_parser.h>
#include <script/scr_main.h>
#include <script/scr_evaluate.h>
#include <script/scr_parsetree.h>
#include <script/scr_compiler.h>

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

void __thiscall Scr_ScriptWatch::AddElement(Scr_WatchElement_s *element, char *text)
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
        SL_ShutdownSystem(2u);
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

bool __cdecl Scr_ElementChildrenExist(Scr_WatchElement_s *element)
{
    if (element->threadList || element->endonList)
        return 1;
    return element->childHead && element->expand && element->objectType && element->objectType < 0x16u;
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
   ((UI_LinesComponent)scrDebuggerGlob.scriptList).SetSelectedLineFocus(sortedIndex, 1);
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
        value.type = 1;
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
            value.type = 0;
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
            value.type = 1;
            AddRefToObject(fieldName);
            break;
        case 0x19u:
            value.u.intValue = fieldName;
            value.type = 2;
            SL_AddRefToString(fieldName);
            break;
        default:
            Scr_EvalFieldVariable(fieldName, &value, element->objectId);
            break;
        }
    }
    return Scr_ScriptWatch::PostEvaluateWatchElement(childElement, &value);
}

int __cdecl CompareThreadIndices(unsigned int *arg1, unsigned int *arg2)
{
    return *arg1 - *arg2;
}

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
    int(__cdecl * compare)(unsigned int *, unsigned int *); // [esp+DCh] [ebp-54h]
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
            element->value.type = type;
        }
        else
        {
            RemoveRefToValue(value->type, value->u);
        }
        return 1;
    }
}

void Scr_ScriptList::LoadScriptPos()
{
    Scr_StringNode_s *node; // [esp+1Ch] [ebp-7Ch]
    __int64 pos; // [esp+20h] [ebp-78h] BYREF
    char filename[88]; // [esp+28h] [ebp-70h] BYREF
    Scr_StringNode_s *head; // [esp+84h] [ebp-14h]
    char *buf; // [esp+88h] [ebp-10h] BYREF
    Scr_ScriptWindow *comp; // [esp+8Ch] [ebp-Ch]
    int selectedLine; // [esp+90h] [ebp-8h] BYREF
    int i; // [esp+94h] [ebp-4h]

    for (head = Scr_GetStringList("script_pos.txt", &buf); head; head = node)
    {
        node = head->next;
        sscanf(head->text, "%80s %d %f %f", filename, &selectedLine, &pos, &pos + 4);
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
                *comp->pos = pos;
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