#include "scr_debugger.h"
#include "scr_animtree.h"
#include "scr_parser.h"
#include "scr_main.h"
#include "scr_stringlist.h"
#include "scr_vm.h"

#include <database/database.h>

#include <bgame/bg_local.h>

#include <qcommon/mem_track.h>

#include <universal/com_memory.h>

#include <win32/win_net_debug.h>
#include "scr_evaluate.h"
#include <client/client.h>
#include <win32/win_input.h>
#include <qcommon/threads.h>
#include <gfx_d3d/r_rendercmds.h>
#include <qcommon/cmd.h>
#include <win32/win_net.h>
#include "scr_compiler.h"

#ifdef KISAK_MP
#include <game_mp/g_public_mp.h>
#endif

scrDebuggerGlob_t scrDebuggerGlob;
Scr_Breakpoint g_breakpoints[128];

Scr_Breakpoint *g_breakpointsHead;

void __cdecl TRACK_scr_debugger()
{
    track_static_alloc_internal(&scrDebuggerGlob, 696, "scrDebuggerGlob", 0);
    track_static_alloc_internal(g_breakpoints, 3584, "g_breakpoints", 0);
}

void __cdecl Scr_AddDebugText(char *text)
{
    if (UI_Component::g.consoleReason == 1)
    {
        I_strncpyz(UI_Component::g.findText, text, 128);
        Scr_SetSelectionComp(&scrDebuggerGlob.scriptScrollPane);
        if (scrDebuggerGlob.scriptScrollPane.comp)
        {
            //Scr_ScriptWindow::FindNext(scrDebuggerGlob.scriptScrollPane.comp);
            ((Scr_ScriptWindow *)(scrDebuggerGlob.scriptScrollPane.comp))->FindNext();
        }
    }
    else
    {
        UI_Component::selectionComp->AddText(text);
    }
}

void __thiscall Scr_ScriptWindow::FindPrev()
{
    int i; // eax
    int numLines; // [esp+0h] [ebp-24h]
    int currentLine; // [esp+18h] [ebp-Ch]
    unsigned int len; // [esp+1Ch] [ebp-8h]
    const char *s; // [esp+20h] [ebp-4h]

    len = strlen(UI_Component::g.findText);
    if (len)
    {
        if (this->selectedLine && this->numLines)
        {
            if (this->selectedLine <= 0)
                numLines = this->numLines;
            else
                numLines = this->selectedLine;
            currentLine = numLines - 1;
            Scr_ScriptWindow::SetCurrentLine(numLines - 1);
            s = this->currentBufPos;
        LABEL_9:
            for (i = I_strnicmp(s, UI_Component::g.findText, len); ; i = I_strnicmp(s, UI_Component::g.findText, len))
            {
                if (!i)
                {
                    this->SetSelectedLineFocus(currentLine, 0);
                    return;
                }
                if (*s)
                {
                    ++s;
                    goto LABEL_9;
                }
                if (!currentLine)
                    break;
                Scr_ScriptWindow::SetCurrentLine(--currentLine);
                s = this->currentBufPos;
            }
            this->selectedLine = -1;
        }
        else
        {
            this->selectedLine = -1;
        }
    }
}

void __thiscall Scr_ScriptWindow::FindNext()
{
    int v1; // kr00_4
    int currentLine; // [esp+14h] [ebp-Ch]
    const char *s; // [esp+1Ch] [ebp-4h]

    v1 = strlen(UI_Component::g.findText);
    if (v1)
    {
        currentLine = this->selectedLine + 1;
        if (currentLine < this->numLines)
        {
            this->SetCurrentLine(currentLine);
            for (s = this->currentBufPos; ; ++s)
            {
                if (!I_strnicmp(s, UI_Component::g.findText, v1))
                {
                    this->SetSelectedLineFocus(currentLine, 0);
                    return;
                }
                if (!*s && ++currentLine >= this->numLines)
                    break;
            }
            this->selectedLine = -1;
        }
        else
        {
            this->selectedLine = -1;
        }
    }
}

void __thiscall Scr_ScriptWindow::GetSourcePos(unsigned int *start, unsigned int *end)
{
    SourceBufferInfo *sourceBufData; // [esp+4h] [ebp-Ch]
    const char *s; // [esp+8h] [ebp-8h]
    int line; // [esp+Ch] [ebp-4h]

    if (this->selectedLine < 0)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1077, 0, "%s", "selectedLine >= 0");
    sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
    s = sourceBufData->sourceBuf;
    for (line = 0; ; ++line)
    {
        if (s - sourceBufData->sourceBuf > sourceBufData->len)
            MyAssertHandler(
                ".\\script\\scr_debugger.cpp",
                1086,
                0,
                "%s",
                "s - sourceBufData->sourceBuf <= sourceBufData->len");
        if (line == this->selectedLine)
            break;
        while (*s)
            ++s;
        ++s;
    }
    *start = s - sourceBufData->sourceBuf;
    while (*s)
        ++s;
    *end = s - sourceBufData->sourceBuf;
}

bool __thiscall Scr_ScriptWindow::AddBreakpointAtSourcePos(
    Scr_WatchElement_s *element,
    unsigned __int8 breakpointType,
    bool user,
    Scr_Breakpoint **pBreakpoint,
    unsigned int startSourcePos,
    unsigned int endSourcePos)
{
    const char *v8; // eax
    Scr_WatchElement_s *BreakpointElement; // [esp-8h] [ebp-2Ch]
    unsigned __int16 v11; // [esp+8h] [ebp-1Ch]
    int builtinIndex; // [esp+Ch] [ebp-18h]
    bool success; // [esp+1Bh] [ebp-9h]
    char *codePos; // [esp+1Ch] [ebp-8h]
    unsigned __int8 *codePosa; // [esp+1Ch] [ebp-8h]
    unsigned int sourcePos; // [esp+20h] [ebp-4h] BYREF

    builtinIndex = -1;
    if (Sys_IsRemoteDebugClient())
    {
        if (!Sys_ReadDebugSocketInt())
            return 0;
        this->bufferIndex = Sys_ReadDebugSocketInt();
        sourcePos = Sys_ReadDebugSocketInt();
        codePos = 0;
    }
    else
    {
        success = 0;
        switch (breakpointType)
        {
        case 4u:
        case 5u:
            codePos = (char*)Scr_GetOpcodePosOfType(this->bufferIndex, startSourcePos, endSourcePos, 1, &sourcePos);
            if (codePos && (*Scr_FindBreakpointInfo(codePos) != 127 || *codePos != 135))
                success = 1;
            break;
        case 6u:
        case 7u:
            goto $LN6_54;
        default:
            if (!alwaysfails)
            {
                v8 = va("unreachable: %d", breakpointType);
                MyAssertHandler(".\\script\\scr_debugger.cpp", 1152, 0, v8);
            }
        $LN6_54:
            codePosa = (unsigned char*)Scr_GetOpcodePosOfType(this->bufferIndex, startSourcePos, endSourcePos, 8, &sourcePos);
            if (codePosa)
            {
                success = 1;
                v11 = *(codePosa + 1);
                builtinIndex = v11;
                if (v11 >= scrCompilePub.func_table_size)
                    MyAssertHandler(
                        ".\\script\\scr_debugger.cpp",
                        1164,
                        0,
                        "%s\n\t(builtinIndex) = %i",
                        "(builtinIndex >= 0 && builtinIndex < scrCompilePub.func_table_size)",
                        v11);
                if (breakpointType == 6)
                    ++scrVmDebugPub.func_table[v11].breakpointCount;
            }
            codePos = 0;
            break;
        }
        if (!success)
            return 0;
    }
    //BreakpointElement = Scr_ScriptWatch::CreateBreakpointElement(
    //    &scrDebuggerGlob.scriptWatch,
    //    element,
    //    this->bufferIndex,
    //    sourcePos,
    //    user);
    BreakpointElement = scrDebuggerGlob.scriptWatch.CreateBreakpointElement(element, this->bufferIndex, sourcePos, user);
    //Scr_ScriptWindow::AddBreakpoint(this, pBreakpoint, codePos, builtinIndex, BreakpointElement, breakpointType);
    this->AddBreakpoint(pBreakpoint, codePos, builtinIndex, BreakpointElement, breakpointType);
    return 1;
}
void __thiscall Scr_ScriptWindow::AddBreakpoint(
    Scr_Breakpoint **pBreakpoint,
    char *codePos,
    int builtinIndex,
    Scr_WatchElement_s *element,
    unsigned __int8 type)
{
    Scr_Breakpoint *breakpoint; // [esp+8h] [ebp-8h]
    Scr_Breakpoint *newBreakpoint; // [esp+Ch] [ebp-4h]

    breakpoint = *pBreakpoint;
    newBreakpoint = Scr_AllocBreakpoint();
    *pBreakpoint = newBreakpoint;
    newBreakpoint->prev = pBreakpoint;
    newBreakpoint->next = breakpoint;
    if (breakpoint)
        breakpoint->prev = &newBreakpoint->next;
    newBreakpoint->line = this->selectedLine;
    newBreakpoint->codePos = codePos;
    newBreakpoint->builtinIndex = builtinIndex;
    newBreakpoint->bufferIndex = this->bufferIndex;
    newBreakpoint->element = element;
    if (element->breakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 973, 0, "%s", "!element->breakpoint");
    element->breakpoint = newBreakpoint;
    element->breakpointType = type;
    if (type >= 4u && type <= 5u && !Sys_IsRemoteDebugClient())
    {
        if (!codePos)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 985, 0, "%s", "codePos");
        Scr_AddManualBreakpoint((unsigned char*)codePos);
    }
}

void __thiscall Scr_ScriptWindow::ToggleBreakpointInternal(
    Scr_WatchElement_s *element,
    bool force,
    bool overwrite,
    unsigned __int8 breakpointType,
    bool user)
{
    Scr_Breakpoint *breakpoint; // [esp+8h] [ebp-24h]
    unsigned int startSourcePos; // [esp+Ch] [ebp-20h] BYREF
    bool movedSelectedLine; // [esp+13h] [ebp-19h]
    SourceBufferInfo *sourceBufData; // [esp+14h] [ebp-18h]
    const char *s; // [esp+18h] [ebp-14h]
    Scr_WatchElement_s *breakpointElement; // [esp+1Ch] [ebp-10h]
    int line; // [esp+20h] [ebp-Ch]
    Scr_Breakpoint **pBreakpoint; // [esp+24h] [ebp-8h]
    unsigned int endSourcePos; // [esp+28h] [ebp-4h] BYREF

    if (this->selectedLine < 0)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1214, 0, "%s", "selectedLine >= 0");
    sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
    if (breakpointType >= 6u && breakpointType <= 7u)
    {
        //Scr_ScriptWindow::GetSourcePos(&startSourcePos, &endSourcePos);
        this->GetSourcePos(&startSourcePos, &endSourcePos);
        //Scr_ScriptWindow::AddBreakpointAtSourcePos(
        //    this,
        //    element,
        //    breakpointType,
        //    user,
        //    &this->builtinHead,
        //    startSourcePos,
        //    endSourcePos);
        this->AddBreakpointAtSourcePos(element, breakpointType, user, &this->builtinHead, startSourcePos, endSourcePos);
        return;
    }
    if (overwrite && element)
    {
        //element = Scr_ScriptWatch::DeleteElementInternal(&scrDebuggerGlob.scriptWatch, element);
        scrDebuggerGlob.scriptWatch.DeleteElementInternal(element);
    }
    s = sourceBufData->sourceBuf;
    line = 0;
    pBreakpoint = &this->breakpointHead;
    movedSelectedLine = 0;
    while (1)
    {
        while (1)
        {
            breakpoint = *pBreakpoint;
            if (!*pBreakpoint)
                goto LABEL_25;
            if (breakpoint->line >= this->selectedLine)
                break;
            pBreakpoint = &breakpoint->next;
        }
        if (breakpoint->line == this->selectedLine)
            break;
    LABEL_25:
        if (!force)
            element = 0;
        while (1)
        {
            if (s - sourceBufData->sourceBuf > sourceBufData->len)
                MyAssertHandler(
                    ".\\script\\scr_debugger.cpp",
                    1294,
                    0,
                    "%s",
                    "s - sourceBufData->sourceBuf <= sourceBufData->len");
            if (line == this->selectedLine)
                break;
            while (*s)
                ++s;
            ++s;
            ++line;
        }
        startSourcePos = s - sourceBufData->sourceBuf;
        while (*s)
            ++s;
        endSourcePos = s - sourceBufData->sourceBuf;
        if (this->AddBreakpointAtSourcePos(
            element,
            breakpointType,
            user,
            pBreakpoint,
            startSourcePos,
            endSourcePos)
            || this->selectedLine >= this->numLines - 1)
        {
            return;
        }
        movedSelectedLine = 1;
        UI_LinesComponent::IncSelectedLineFocus(0);
        if (*s)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 1323, 0, "%s", "!(*s)");
        ++s;
        ++line;
    }
    if (force)
    {
        if (element == breakpoint->element)
            element = element->next;
        Scr_FreeLineBreakpoint(breakpoint, 1);
        goto LABEL_25;
    }
    breakpointElement = breakpoint->element;
    if (!breakpointElement)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1274, 0, "%s", "breakpointElement");
    if (breakpointElement->breakpointType == breakpointType)
    {
        if (!movedSelectedLine)
            Scr_FreeLineBreakpoint(breakpoint, 1);
    }
    else
    {
        breakpointElement->breakpointType = breakpointType;
    }
}

void __thiscall Scr_ScriptWindow::SetCurrentLine(int line)
{
    const char *endPos; // [esp+4h] [ebp-Ch]
    SourceBufferInfo *sourceBufData; // [esp+8h] [ebp-8h]
    const char *startPos; // [esp+Ch] [ebp-4h]

    if (this->bufferIndex >= scrParserPub.sourceBufferLookupLen)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            721,
            0,
            "bufferIndex doesn't index scrParserPub.sourceBufferLookupLen\n\t%i not in [0, %i)",
            this->bufferIndex,
            scrParserPub.sourceBufferLookupLen);
    sourceBufData = &scrParserPub.sourceBufferLookup[this->bufferIndex];
    if (line >= this->currentTopLine)
    {
        if (line > this->currentTopLine)
        {
            endPos = &sourceBufData->sourceBuf[sourceBufData->len];
            while (1)
            {
                while (*this->currentBufPos)
                    ++this->currentBufPos;
                ++this->currentBufPos;
                ++this->currentTopLine;
                if (this->currentBufPos >= endPos)
                    break;
                if (line <= this->currentTopLine)
                    return;
            }
            //Scr_ScriptWindow::SetCurrentLine(this, this->currentTopLine - 1);
            this->SetCurrentLine(this->currentTopLine - 1);
        }
    }
    else
    {
        startPos = sourceBufData->sourceBuf;
        while (this->currentBufPos != startPos)
        {
            if (*--this->currentBufPos)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 737, 0, "%s", "!(*currentBufPos)");
            do
                --this->currentBufPos;
            while (*this->currentBufPos);
            ++this->currentBufPos;
            if (line >= --this->currentTopLine)
                return;
        }
        this->currentTopLine = 0;
    }
}

void __cdecl Scr_SetMiscScrollPaneComp(struct UI_LinesComponent *comp)
{
    if (!comp)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7762, 0, "%s", "comp");
    scrDebuggerGlob.miscScrollPane.comp = comp;
    if (comp->selectionParent != &scrDebuggerGlob.miscScrollPane)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            7765,
            0,
            "%s",
            "comp->selectionParent == &scrDebuggerGlob.miscScrollPane");
    Scr_SetSelectionComp(&scrDebuggerGlob.miscScrollPane);
    comp->SetSelectedLineFocus(comp->selectedLine, 0);
}

void __cdecl Scr_KeyEvent(int key)
{
    float v1; // [esp+0h] [ebp-34h]
    float v2; // [esp+4h] [ebp-30h]
    float v3; // [esp+1Ch] [ebp-18h]
    float v4; // [esp+20h] [ebp-14h]
    UI_VerticalDivider *comp; // [esp+24h] [ebp-10h]
    DWORD newMouseTime; // [esp+28h] [ebp-Ch]
    float point[2]; // [esp+2Ch] [ebp-8h] BYREF

    if (!scrDebuggerGlob.debugger_inited_system)
        return;
    if (UI_Component::g.hideCursor)
        IN_ActivateMouse(1);
    if (!Key_IsCatcherActive(0, 2))
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            7788,
            0,
            "%s",
            "Key_IsCatcherActive( ONLY_LOCAL_CLIENT_NUM, KEYCATCH_SCRIPT )");
    if (!Key_IsDown(0, 158) && !Key_IsDown(0, 159) && !Key_IsDown(0, 160))
    {
        switch (key)
        {
        case 49:
            goto $LN41_4;
        case 50:
            Scr_SetMiscScrollPaneComp(&scrDebuggerGlob.scriptWatch);
            return;
        case 51:
            Scr_SetMiscScrollPaneComp(&scrDebuggerGlob.scriptList);
            return;
        case 52:
            Scr_SetMiscScrollPaneComp(&scrDebuggerGlob.scriptCallStack);
            return;
        case 53:
            Scr_SetMiscScrollPaneComp(&scrDebuggerGlob.openScriptList);
            return;
        case 153:
            if (Sys_IsRemoteDebugClient())
            {
                Sys_WriteDebugSocketMessageType(0x2Cu);
                Sys_EndWriteDebugSocket();
            }
            return;
        case 167:
            //UI_LinesComponent::SetSelectedLineFocus(&scrDebuggerGlob.scriptList, 0, 0);
            scrDebuggerGlob.scriptList.SetSelectedLineFocus(0, 0);
            //UI_LinesComponent::ClearFocus(&scrDebuggerGlob.scriptList);
            scrDebuggerGlob.scriptList.ClearFocus();
        $LN41_4:
            Scr_SetSelectionComp(&scrDebuggerGlob.scriptScrollPane);
            if (scrDebuggerGlob.scriptScrollPane.comp)
            {
                //scrDebuggerGlob.scriptScrollPane.comp->SetSelectedLineFocus(scrDebuggerGlob.scriptScrollPane.comp, scrDebuggerGlob.scriptScrollPane.comp->selectedLine, 0);
                scrDebuggerGlob.scriptScrollPane.comp->SetSelectedLineFocus(scrDebuggerGlob.scriptScrollPane.comp->selectedLine, 0);
            }
            break;
        case 169:
            Scr_SetSelectionComp(&scrDebuggerGlob.scriptScrollPane);
            if (scrDebuggerGlob.scriptScrollPane.comp)
            {
                //Scr_ScriptWindow::FindNext(scrDebuggerGlob.scriptScrollPane.comp);
                ((Scr_ScriptWindow *)(scrDebuggerGlob.scriptScrollPane.comp))->FindNext();
            }
            break;
        case 171:
            scrDebuggerGlob.step_mode = 0;
            Scr_Step();
            break;
        case 176:
            scrDebuggerGlob.step_mode = 1;
            Scr_Step();
            break;
        case 177:
            scrDebuggerGlob.step_mode = 2;
            Scr_Step();
            break;
        default:
            goto LABEL_50;
        }
        return;
    }
    if (Key_IsDown(0, 158) || Key_IsDown(0, 159) || !Key_IsDown(0, 160))
    {
        if (Key_IsDown(0, 158) || !Key_IsDown(0, 159) || Key_IsDown(0, 160))
        {
            if (!Key_IsDown(0, 158) && Key_IsDown(0, 159) && Key_IsDown(0, 160) && key == 9)
            {
                //UI_LinesComponent::IncSelectedLineFocus(&scrDebuggerGlob.openScriptList, 1);
                scrDebuggerGlob.openScriptList.IncSelectedLineFocus(true);
                return;
            }
        }
        else
        {
            if (key == 9)
            {
                //UI_LinesComponent::DecSelectedLineFocus(&scrDebuggerGlob.openScriptList, 1);
                scrDebuggerGlob.openScriptList.DecSelectedLineFocus(true);
                return;
            }
            if (key == 102)
            {
                UI_Component::g.consoleReason = 1;
                Con_OpenConsole(0);
                return;
            }
        }
        goto LABEL_50;
    }
    if (key != 169)
    {
        if (key == 177)
        {
            scrDebuggerGlob.step_mode = 3;
            Scr_Step();
            return;
        }
    LABEL_50:
        point[0] = UI_Component::g.cursorPos[0];
        point[1] = UI_Component::g.cursorPos[1];
        if (key == 200)
        {
            if (!UI_Component::g.hideCursor)
            {
                //comp = UI_VerticalDivider::GetCompAtLocation(&scrDebuggerGlob.mainWindow, point);
                comp = (UI_VerticalDivider*)scrDebuggerGlob.mainWindow.GetCompAtLocation(point);
                if (comp)
                {
                    if (comp->selectionParent)
                        Scr_SetSelectionComp(comp->selectionParent);
                    if (comp->KeyEvent(point, 200))
                    {
                        scrDebuggerGlob.prevMouseTime = 0;
                    }
                    else
                    {
                        newMouseTime = Sys_Milliseconds();
                        if ((newMouseTime - scrDebuggerGlob.prevMouseTime) > 300
                            || (v4 = UI_Component::g.cursorPos[0] - scrDebuggerGlob.prevMousePos[0], v2 = I_fabs(v4), v2 >= 5.0)
                            || (v3 = UI_Component::g.cursorPos[1] - scrDebuggerGlob.prevMousePos[1], v1 = I_fabs(v3), v1 >= 5.0))
                        {
                            scrDebuggerGlob.prevMouseTime = newMouseTime;
                            scrDebuggerGlob.prevMousePos[0] = UI_Component::g.cursorPos[0];
                            scrDebuggerGlob.prevMousePos[1] = UI_Component::g.cursorPos[1];
                        }
                        else
                        {
                            comp->KeyEvent(point, 223);
                            scrDebuggerGlob.prevMouseTime = 0;
                        }
                    }
                }
            }
        }
        else if (!UI_Component::selectionComp->KeyEvent(point, key))
        {
            //UI_VerticalDivider::KeyEvent(&scrDebuggerGlob.mainWindow, UI_Component::g.cursorPos, key);
            scrDebuggerGlob.mainWindow.KeyEvent(UI_Component::g.cursorPos, key);
        }
        return;
    }
    Scr_SetSelectionComp(&scrDebuggerGlob.scriptScrollPane);
    if (scrDebuggerGlob.scriptScrollPane.comp)
    {
        //Scr_ScriptWindow::FindPrev(scrDebuggerGlob.scriptScrollPane.comp);
        ((Scr_ScriptWindow *)(scrDebuggerGlob.scriptScrollPane.comp))->FindPrev();
    }
}

void __cdecl Scr_AddManualBreakpoint(unsigned __int8 *codePos)
{
    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 506, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (!codePos)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 509, 0, "%s", "codePos");
    if (*codePos == 135 || *codePos == 137)
    {
        if ((unsigned __int8 *)scrDebuggerGlob.nextBreakpointCodePos != codePos)
            MyAssertHandler(
                ".\\script\\scr_debugger.cpp",
                513,
                0,
                "%s",
                "(byte *)scrDebuggerGlob.nextBreakpointCodePos == codePos");
        if (scrDebuggerGlob.nextBreakpointCodePosMasked)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 514, 0, "%s", "!scrDebuggerGlob.nextBreakpointCodePosMasked");
        scrDebuggerGlob.nextBreakpointCodePosMasked = 1;
    }
    else if (*codePos == 136)
    {
        *codePos = -119;
    }
    else
    {
        Scr_AddBreakpoint(codePos);
        *codePos = -121;
    }
}

void __cdecl Scr_AddBreakpoint(const unsigned __int8 *codePos)
{
    unsigned __int8 *breakpoint; // [esp+0h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 457, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (*codePos == 127)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 460, 0, "%s", "*codePos != OP_NOP");
    breakpoint = (unsigned __int8 *)Scr_FindBreakpointInfo((const char *)codePos);
    if (*breakpoint != 127)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 462, 0, "%s", "*breakpoint == OP_NOP");
    *breakpoint = *codePos;
    ++scrDebuggerGlob.breakpointCount;
}

char *__cdecl Scr_FindBreakpointInfo(const char *codePos)
{
    unsigned int index; // [esp+0h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 434, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (!codePos)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 437, 0, "%s", "codePos");
    index = codePos - scrVarPub.programBuffer;
    if (codePos - scrVarPub.programBuffer >= scrCompilePub.programLen)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 440, 0, "%s", "index < scrCompilePub.programLen");
    return &scrDebuggerGlob.breakpoints[index];
}

Scr_Breakpoint *__cdecl Scr_AllocBreakpoint()
{
    Scr_Breakpoint *breakpoint; // [esp+0h] [ebp-4h]

    breakpoint = g_breakpointsHead;
    if (!g_breakpointsHead)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 633, 0, "%s", "breakpoint");
    g_breakpointsHead = g_breakpointsHead->next;
    return breakpoint;
}

void __cdecl Scr_FreeBreakpoint(Scr_Breakpoint *breakpoint)
{
    breakpoint->next = g_breakpointsHead;
    g_breakpointsHead = breakpoint;
}

Scr_WatchElement_s *__cdecl Scr_ReadElement()
{
    int id; // [esp+4h] [ebp-4h]

    id = Sys_ReadDebugSocketInt();
    if (id)
        return scrDebuggerGlob.scriptWatch.GetElementWithId(id);
        //return Scr_ScriptWatch::GetElementWithId(&scrDebuggerGlob.scriptWatch, id);
    else
        return 0;
}

void __cdecl Scr_FreeLineBreakpoint(Scr_Breakpoint *breakpoint, bool deleteElement)
{
    unsigned __int8 breakpointType; // [esp+7h] [ebp-9h]
    Scr_Breakpoint **pBreakpoint; // [esp+8h] [ebp-8h]
    Scr_WatchElement_s *element; // [esp+Ch] [ebp-4h]

    pBreakpoint = breakpoint->prev;
    element = breakpoint->element;
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1036, 0, "%s", "element");
    breakpointType = element->breakpointType;
    breakpoint->element = 0;
    if (deleteElement)
    {
        //Scr_ScriptWatch::DeleteElementInternal(&scrDebuggerGlob.scriptWatch, element);
        scrDebuggerGlob.scriptWatch.DeleteElementInternal(element);
    }
    if (breakpointType >= 4u && breakpointType <= 5u && !Sys_IsRemoteDebugClient())
    {
        if (!breakpoint->codePos)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 1051, 0, "%s", "breakpoint->codePos");
        Scr_RemoveManualBreakpoint((unsigned __int8 *)breakpoint->codePos);
    }
    if (pBreakpoint != breakpoint->prev)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1057, 0, "%s", "pBreakpoint == breakpoint->prev");
    if (breakpoint->next)
        breakpoint->next->prev = pBreakpoint;
    *pBreakpoint = breakpoint->next;
    Scr_FreeBreakpoint(breakpoint);
}

void __cdecl Scr_RemoveManualBreakpoint(unsigned __int8 *codePos)
{
    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 540, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if ((unsigned __int8 *)scrDebuggerGlob.nextBreakpointCodePos == codePos
        && scrDebuggerGlob.nextBreakpointCodePosMasked)
    {
        scrDebuggerGlob.nextBreakpointCodePosMasked = 0;
    }
    else if (*codePos == 135)
    {
        Scr_RemoveBreakpoint(codePos);
    }
    else
    {
        if (*codePos != 137)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 558, 0, "%s", "*codePos == OP_manualAndAssignmentBreakpoint");
        *codePos = -120;
    }
}

void __cdecl Scr_RemoveBreakpoint(unsigned __int8 *codePos)
{
    unsigned __int8 *breakpoint; // [esp+0h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 482, 0, "%s", "!Sys_IsRemoteDebugClient()");
    breakpoint = (unsigned __int8 *)Scr_FindBreakpointInfo((const char *)codePos);
    if (*breakpoint == 127)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 486, 0, "%s", "*breakpoint != OP_NOP");
    *codePos = *breakpoint;
    *breakpoint = 127;
    if (!scrDebuggerGlob.breakpointCount)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 491, 0, "%s", "scrDebuggerGlob.breakpointCount");
    --scrDebuggerGlob.breakpointCount;
}

void __cdecl Scr_WriteElement(Scr_WatchElement_s *element)
{
    if (element)
        Sys_WriteDebugSocketInt(element->id);
    else
        Sys_WriteDebugSocketInt(0);
}

void __cdecl Scr_MonitorCommand(const char *text)
{
    if (!text)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 1540, 0, "%s", "text");
}

Scr_WatchElement_s *Scr_ResumeBreakpoints()
{
    Scr_WatchElement_s *result; // eax
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    scrDebuggerGlob.atBreakpoint = 0;
    scrDebuggerGlob.breakpointPos.bufferIndex = -1;
    result = scrDebuggerGlob.scriptWatch.elementHead;
    for (element = scrDebuggerGlob.scriptWatch.elementHead; element; element = element->next)
    {
        result = element;
        element->changed = 0;
        element->hitBreakpoint = 0;
    }
    return result;
}

void __cdecl Scr_SetTempBreakpoint(char *codePos, unsigned int threadId)
{
    if (codePos)
    {
        if (scrDebuggerGlob.killThreadCodePos)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 1560, 0, "%s", "!scrDebuggerGlob.killThreadCodePos");
        scrDebuggerGlob.nextBreakpointCodePos = codePos;
        Scr_AddManualBreakpoint((unsigned __int8 *)codePos);
        scrDebuggerGlob.prevBreakpointLineNum = scrDebuggerGlob.breakpointPos.lineNum;
        scrDebuggerGlob.nextBreakpointThreadId = threadId;
    }
}

void __cdecl Scr_FreeDebugMem(void *ptr)
{
    Z_Free(ptr, 0);
}

unsigned int *__cdecl Scr_AllocDebugMem(int size, const char *name)
{
    return (unsigned int*)Z_Malloc(size, name, 0);
}

Scr_WatchElement_s *__cdecl Scr_GetElementRoot(Scr_WatchElement_s *element)
{
    while (element->parent)
        element = element->parent;
    return element;
}

void __cdecl Scr_FreeWatchElementChildrenStrict(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s *childElement; // [esp+0h] [ebp-8h]
    Scr_WatchElement_s *nextChildElement; // [esp+4h] [ebp-4h]

    childElement = element->childHead;
    if (childElement)
    {
        while (childElement)
        {
            nextChildElement = childElement->next;
            Scr_FreeWatchElementChildren(childElement);
            childElement = nextChildElement;
        }
        if (!element->childArrayHead)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 4950, 0, "%s", "element->childArrayHead");
        Scr_FreeDebugMem(element->childArrayHead);
        element->childCount = 0;
        element->childArrayHead = 0;
        element->childHead = 0;
        //Scr_ScriptWatch::UpdateHeight(&scrDebuggerGlob.scriptWatch);
        scrDebuggerGlob.scriptWatch.UpdateHeight();
    }
}

void __cdecl Scr_FreeWatchElementChildren(Scr_WatchElement_s *element)
{
    Scr_FreeWatchElementText(element);
    if (!Sys_IsRemoteDebugClient())
        Scr_RemoveValue(element);
    Scr_FreeWatchElementChildrenStrict(element);
}

void __cdecl Scr_RemoveValue(Scr_WatchElement_s *element)
{
    if (element->valueDefined)
    {
        element->valueDefined = 0;
        RemoveRefToValue(element->value.type, element->value.u);
    }
}

void __cdecl Scr_FreeWatchElementText(Scr_WatchElement_s *element)
{
    if (!element->valueText)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 4970, 0, "%s", "element->valueText");
    FreeString(element->valueText);
    element->valueText = 0;
    if (!element->refText)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 4974, 0, "%s", "element->refText");
    FreeString(element->refText);
    element->refText = 0;
}

bool __cdecl Scr_IsSortWatchElement(Scr_WatchElement_s *element)
{
    return element->threadList;
}

int __cdecl CompareArrayIndices(unsigned int *arg1, unsigned int *arg2)
{
    int v2; // ecx
    VariableValue ArrayIndexValue; // [esp+14h] [ebp-24h]
    unsigned int name[2]; // [esp+1Ch] [ebp-1Ch]
    int i; // [esp+24h] [ebp-14h]
    VariableValue value[2]; // [esp+28h] [ebp-10h]

    name[0] = *arg1;
    name[1] = *arg2;
    for (i = 0; i < 2; ++i)
    {
        ArrayIndexValue = Scr_GetArrayIndexValue(name[i]);
        v2 = i;
        value[i].u.intValue = ArrayIndexValue.u.intValue;
        value[v2].type = ArrayIndexValue.type;
    }
    if (value[0].type != value[1].type)
        return value[0].type - value[1].type;
    if (value[0].type == 2)
    {
        return strcmp(SL_ConvertToString(value[0].u.stringValue), SL_ConvertToString(value[1].u.stringValue));
    }
    else
    {
        if (value[0].type != 6)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 5120, 0, "%s", "value[0].type == VAR_INTEGER");
        return value[0].u.intValue - value[1].u.intValue;
    }
}

void __cdecl Scr_DeltaElementValueText(Scr_WatchElement_s *element, const char *oldValueText)
{
    if (strcmp(element->valueText, oldValueText))
    {
        if (*oldValueText)
        {
            element->changed = 1;
            element->changedTime = 0;
        }
    }
}

void __cdecl Scr_SetNonFieldElementRefText(Scr_WatchElement_s *element)
{
    char refText[128]; // [esp+18h] [ebp-88h] BYREF
    Scr_WatchElement_s *parentElement; // [esp+9Ch] [ebp-4h]

    parentElement = element->parent;
    switch (element->fieldName)
    {
    case 0u:
        Com_sprintf(refText, 0x80u, "%s.size", parentElement->refText);
        ReplaceString(&element->refText, refText);
        break;
    case 1u:
        ReplaceString(&element->refText, (char *)parentElement->valueText);
        break;
    case 2u:
        ReplaceString(&element->refText, "<endons>");
        element->endonList = 1;
        break;
    case 3u:
        if (!strcmp(parentElement->refText, "<locals>"))
        {
            ReplaceString(&element->refText, "self");
        }
        else
        {
            Com_sprintf(refText, 0x80u, "%s.self", parentElement->refText);
            ReplaceString(&element->refText, refText);
        }
        break;
    case 4u:
        ReplaceString(&element->refText, "<threads>");
        element->threadList = 1;
        break;
    default:
        Com_sprintf(refText, 0x80u, "$t%i", element->fieldName - 5);
        ReplaceString(&element->refText, refText);
        break;
    }
    if (!Sys_IsRemoteDebugClient())
        Scr_PostSetText(element);
}

void __cdecl Scr_PostSetText(Scr_WatchElement_s *element)
{
    int v1; // [esp+18h] [ebp-14Ch]
    int v2; // [esp+2Ch] [ebp-138h]
    unsigned __int8 ObjectType; // [esp+44h] [ebp-120h]
    unsigned int bufferIndex; // [esp+48h] [ebp-11Ch]
    char valueText[264]; // [esp+4Ch] [ebp-118h] BYREF
    bool directObject; // [esp+15Ah] [ebp-Ah]
    unsigned __int8 type; // [esp+15Bh] [ebp-9h]
    const char *codePos; // [esp+15Ch] [ebp-8h]
    unsigned int sourcePos; // [esp+160h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5179, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (element->threadList)
    {
        type = 24;
    }
    else if (element->endonList)
    {
        type = 25;
    }
    else
    {
        if (element->objectId)
            ObjectType = GetObjectType(element->objectId);
        else
            ObjectType = 0;
        type = ObjectType;
        if (ObjectType >= 0xEu && ObjectType <= 0x11u)
            type = Scr_IsEndonThread(element->objectId) ? 0 : 14;
    }
    directObject = 0;
    switch (type)
    {
    case 0xEu:
        v2 = strcmp(element->refText, element->valueText);
        directObject = v2 == 0;
        if (!v2)
            ReplaceString(&element->valueText, (char *)"");
        break;
    case 0x12u:
    case 0x13u:
        directObject = strcmp(element->refText, element->valueText) == 0;
        break;
    case 0x14u:
        v1 = strcmp(element->refText, element->valueText);
        directObject = v1 == 0;
        if (!v1)
        {
            Scr_GetFieldValue(element->objectId, "classname", 257, valueText);
            ReplaceString(&element->valueText, valueText);
        }
        break;
    case 0x15u:
        directObject = 1;
        break;
    default:
        break;
    }
    if (element->objectType != type || element->directObject != directObject)
    {
        element->objectType = type;
        element->directObject = directObject;
        if ((type == 14 || type == 22) && element->oldObjectType != 14 && element->oldObjectType != 22)
        {
            codePos = Scr_GetElementThreadPos(element);
            if (codePos)
            {
                bufferIndex = Scr_GetSourceBuffer(codePos - 1);
                sourcePos = Scr_GetPrevSourcePos(codePos - 1, 0);
                element->bufferIndex = bufferIndex;
                element->sourcePos = Scr_GetClosestSourcePosOfType(bufferIndex, sourcePos, 4);
            }
            else
            {
                element->bufferIndex = -1;
                element->sourcePos = 0;
            }
        }
    }
}

const char *__cdecl Scr_GetElementThreadPos(Scr_WatchElement_s *element)
{
    const char *codePos; // [esp+0h] [ebp-4h]

    if (element->objectType == 14 && (codePos = Scr_GetThreadPos(element->objectId)) != 0)
        return codePos;
    else
        return element->deadCodePos;
}

void __cdecl Scr_SetElementRefText(Scr_WatchElement_s *element, char *fieldText)
{
    char refText[128]; // [esp+18h] [ebp-88h] BYREF
    Scr_WatchElement_s *parentElement; // [esp+9Ch] [ebp-4h]

    parentElement = element->parent;
    switch (parentElement->objectType)
    {
    case 0xEu:
        if (strcmp(parentElement->refText, "<locals>"))
            goto $LN7_47;
        goto LABEL_3;
    case 0x12u:
    case 0x13u:
    case 0x14u:
    $LN7_47:
        Com_sprintf(refText, 0x80u, "%s.%s", parentElement->refText, fieldText);
        ReplaceString(&element->refText, refText);
        break;
    case 0x15u:
        Com_sprintf(refText, 0x80u, "%s[%s]", parentElement->refText, fieldText);
        ReplaceString(&element->refText, refText);
        break;
    case 0x18u:
    case 0x19u:
    LABEL_3:
        ReplaceString(&element->refText, fieldText);
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 5521, 0, "unreachable");
        break;
    }
    if (!Sys_IsRemoteDebugClient())
        Scr_PostSetText(element);
}

void __cdecl Scr_ConnectElementChildren(Scr_WatchElement_s *parentElement)
{
    Scr_WatchElement_s *v1; // [esp+0h] [ebp-10h]
    int newIndex; // [esp+4h] [ebp-Ch]
    Scr_WatchElement_s *newElements; // [esp+8h] [ebp-8h]
    signed int count; // [esp+Ch] [ebp-4h]

    count = parentElement->childCount;
    newElements = parentElement->childArrayHead;
    for (newIndex = 0; newIndex < count; ++newIndex)
    {
        if (newIndex >= count - 1)
            v1 = 0;
        else
            v1 = &newElements[newIndex + 1];
        newElements[newIndex].next = v1;
    }
    parentElement->childHead = newElements;
}

void __cdecl Scr_SortElementChildren(Scr_WatchElement_s *parentElement)
{
    unsigned int v1; // [esp+0h] [ebp-14h]
    int newIndex; // [esp+4h] [ebp-10h]
    int newIndexa; // [esp+4h] [ebp-10h]
    Scr_WatchElement_s *newElements; // [esp+8h] [ebp-Ch]
    unsigned int *elementList; // [esp+Ch] [ebp-8h]
    int count; // [esp+10h] [ebp-4h]

    if (!scrDebuggerGlob.debugger_inited_system)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5634, 0, "%s", "scrDebuggerGlob.debugger_inited_system");
    if (!Scr_IsSortWatchElement(parentElement))
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5635, 0, "%s", "Scr_IsSortWatchElement( parentElement )");
    count = parentElement->childCount;
    newElements = parentElement->childArrayHead;
    elementList = Scr_AllocDebugMem(4 * count, "Scr_SortElementChildren");
    for (newIndex = 0; newIndex < count; ++newIndex)
        elementList[newIndex] = (unsigned int)&newElements[newIndex];
    qsort(elementList, count, 4u, (int(__cdecl *)(const void *, const void *))CompareThreadElements);
    for (newIndexa = 0; newIndexa < count; ++newIndexa)
    {
        if (newIndexa >= count - 1)
            v1 = 0;
        else
            v1 = elementList[newIndexa + 1];
        *(unsigned int *)(elementList[newIndexa] + 96) = v1;
    }
    parentElement->childHead = (Scr_WatchElement_s *)*elementList;
    Scr_FreeDebugMem(elementList);
}

int __cdecl CompareThreadElements(int *arg1, int *arg2)
{
    int elements; // [esp+8h] [ebp-8h]
    int elements_4; // [esp+Ch] [ebp-4h]

    elements = *arg1;
    elements_4 = *arg2;
    if (scrParserPub.sourceBufferLookup[*(unsigned int *)(*arg1 + 72)].sortedIndex != scrParserPub.sourceBufferLookup[*(unsigned int *)(*arg2 + 72)].sortedIndex)
        return scrParserPub.sourceBufferLookup[*(unsigned int *)(*arg1 + 72)].sortedIndex
        - scrParserPub.sourceBufferLookup[*(unsigned int *)(*arg2 + 72)].sortedIndex;
    if (*(unsigned int *)(elements + 76) == *(unsigned int *)(elements_4 + 76))
        return *(unsigned int *)(elements + 48) - *(unsigned int *)(elements_4 + 48);
    return *(unsigned int *)(elements + 76) - *(unsigned int *)(elements_4 + 76);
}

Scr_WatchElement_s *__cdecl Scr_CreateWatchElement(char *text, Scr_WatchElement_s **prevElem, const char *name)
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    element = (Scr_WatchElement_s *)Scr_AllocDebugMem(100, name);
    memset((unsigned __int8 *)element, 0, sizeof(Scr_WatchElement_s));
    element->valueText = CopyString((char *)"");
    element->refText = CopyString(text);
    element->next = *prevElem;
    *prevElem = element;
    return element;
}

void __cdecl Scr_Evaluate()
{
    //Scr_ScriptWatch::Evaluate(&scrDebuggerGlob.scriptWatch);
    scrDebuggerGlob.scriptWatch.Evaluate();
}

void __cdecl Scr_CheckBreakonNotify(
    unsigned int notifyListOwnerId,
    unsigned int stringValue,
    VariableValue *top,
    char *pos,
    unsigned int localId)
{
    int hitBreakpoint; // [esp+0h] [ebp-18h]
    bool updateBreakpoints; // [esp+7h] [ebp-11h]
    Scr_WatchElement_s *element; // [esp+Ch] [ebp-Ch]
    VariableValue newValue; // [esp+10h] [ebp-8h] BYREF

    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6976, 0, "%s", "!scrVmPub.inparamcount");
    scrVmPub.top = top;
    scrDebuggerGlob.scriptWatch.localId = 0;
    g_breakonObject = notifyListOwnerId;
    g_breakonString = stringValue;
    if (scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6984, 0, "%s", "!scrVarPub.evaluate");
    scrVarPub.evaluate = 1;
    scrDebuggerGlob.scriptWatch.localId = 0;
    updateBreakpoints = 0;
    hitBreakpoint = 0;
retry_13:
    for (element = scrDebuggerGlob.scriptWatch.elementHead; element; element = element->next)
    {
        if (!element->expr.breakonExpr)
            continue;
        if (element->breakpoint)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 6997, 0, "%s", "!element->breakpoint");
        if (!element->expr.exprHead)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 6999, 0, "%s", "expr->exprHead");
        g_breakonHit = 0;
        if (Scr_EvalScriptExpression(&element->expr, 0, &newValue, 1, 1) && !updateBreakpoints)
        {
            Scr_ClearErrorMessage();
            RemoveRefToValue(newValue.type, newValue.u);
        retry2:
            updateBreakpoints = 1;
            //Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 0);
            scrDebuggerGlob.scriptWatch.UpdateBreakpoints(0);
            goto retry_13;
        }
        if (scrVarPub.error_message)
        {
            Scr_ClearErrorMessage();
            RemoveRefToValue(newValue.type, newValue.u);
            if (!element->valueDefined)
                continue;
            if (!updateBreakpoints)
                goto retry2;
        }
        else if (!g_breakonHit)
        {
            continue;
        }
        if (pos)
        {
            if (element->breakpointType == 1)
            {
                Scr_WatchElementHitBreakpoint(element, 1);
                hitBreakpoint = 1;
            }
        }
    }
    if (updateBreakpoints)
    {
        //Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 1);
        scrDebuggerGlob.scriptWatch.UpdateBreakpoints(1);
    }
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7042, 0, "%s", "scrVarPub.evaluate");
    scrVarPub.evaluate = 0;
    g_breakonObject = 0;
    g_breakonString = 0;
    if (hitBreakpoint)
        Scr_SpecialBreakpoint(top, pos, localId, 121, 16);
}

void __cdecl Scr_SpecialBreakpoint(VariableValue *top, char *pos, unsigned int localId, int opcode, int type)
{
    if (!pos)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6939, 0, "%s", "pos");
    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6940, 0, "%s", "!scrVmPub.inparamcount");
    scrVmPub.outparamcount = 0;
    if (scrDebuggerGlob.nextBreakpointCodePos)
    {
        Scr_RemoveManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.nextBreakpointCodePos);
        scrDebuggerGlob.nextBreakpointCodePos = 0;
    }
    if (Scr_AllowBreakpoint(pos))
    {
        scrDebuggerGlob.breakpointTop = top;
        scrDebuggerGlob.breakpointCodePos = pos - 1;
        scrDebuggerGlob.breakpointOpcode = opcode;
        scrDebuggerGlob.scriptWatch.localId = localId;
        if (!Scr_GetSourcePosOfType(pos - 1, type, &scrDebuggerGlob.breakpointPos))
            MyAssertHandler(".\\script\\scr_debugger.cpp", 6961, 0, "%s", "hasSourcePos");
        Scr_HitBreakpointInternal();
    }
}

char __cdecl Scr_AllowBreakpoint(char *pos)
{
    if (scrDebuggerGlob.disableBreakpoints)
        return 0;
    if (cls.uiStarted && !cls.quit)
        return 1;
    if (pos)
    {
        Com_PrintWarning(23, "script runtime warning: ignored breakpoint.\n");
        Scr_PrintPrevCodePos(23, pos, 0);
    }
    return 0;
}

Scr_OpcodeList_s *Scr_UnbreakAllAssignmentPos()
{
    Scr_OpcodeList_s *result; // eax
    Scr_OpcodeList_s *opcodeElement; // [esp+0h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7173, 0, "%s", "!Sys_IsRemoteDebugClient()");
    result = (Scr_OpcodeList_s *)scrDebuggerGlob.assignBreakpointSet;
    if (scrDebuggerGlob.assignBreakpointSet)
    {
        scrDebuggerGlob.assignBreakpointSet = 0;
        for (opcodeElement = scrDebuggerGlob.assignHead; opcodeElement; opcodeElement = result)
        {
            Scr_RemoveAssignmentBreakpoint((unsigned __int8 *)opcodeElement->codePos);
            result = opcodeElement->next;
        }
    }
    return result;
}

void __cdecl Scr_RemoveAssignmentBreakpoint(unsigned __int8 *codePos)
{
    if (*codePos == 136)
    {
        Scr_RemoveBreakpoint(codePos);
    }
    else
    {
        if (*codePos != 137)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 598, 0, "%s", "*codePos == OP_manualAndAssignmentBreakpoint");
        *codePos = -121;
    }
}

bool __cdecl Scr_RefToVariable(unsigned int id, int isObject)
{
    Scr_WatchElementNode_s **pElementNode; // [esp+0h] [ebp-1Ch]
    Scr_WatchElementNode_s *elementNodeNext; // [esp+4h] [ebp-18h]
    Scr_WatchElementDoubleNode_t *breakpoints; // [esp+8h] [ebp-14h]
    unsigned int *elementNodec; // [esp+Ch] [ebp-10h]
    Scr_WatchElementNode_s *elementNode; // [esp+Ch] [ebp-10h]
    Scr_WatchElementNode_s *elementNodea; // [esp+Ch] [ebp-10h]
    Scr_WatchElementNode_s *elementNodeb; // [esp+Ch] [ebp-10h]
    Scr_WatchElement_s *element; // [esp+10h] [ebp-Ch]
    VariableValue value; // [esp+14h] [ebp-8h] BYREF
    unsigned int ida; // [esp+24h] [ebp+8h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7252, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (!id)
        return 0;
    if (isObject)
        ida = id + 1;
    else
        ida = id + VARIABLELIST_CHILD_BEGIN;
    if (scrDebuggerGlob.removeId)
        return scrDebuggerGlob.removeId == ida;
    if (!scrDebuggerGlob.currentElement)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7266, 0, "%s", "scrDebuggerGlob.currentElement");
    if (!scrDebuggerGlob.variableBreakpoints)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7268, 0, "%s", "scrDebuggerGlob.variableBreakpoints");
    breakpoints = scrDebuggerGlob.variableBreakpoints[ida];
    if (!breakpoints)
    {
        if (!scrDebuggerGlob.add)
            return 0;
        breakpoints = (Scr_WatchElementDoubleNode_t *)Scr_AllocDebugMem(8, "Scr_RefToVariable1");
        breakpoints->list = 0;
        breakpoints->removedList = 0;
        scrDebuggerGlob.variableBreakpoints[ida] = breakpoints;
        if (isObject)
            AddRefToObject(ida - 1);
    }
    for (pElementNode = &breakpoints->list;
        *pElementNode && (*pElementNode)->element != scrDebuggerGlob.currentElement;
        pElementNode = &(*pElementNode)->next)
    {
        ;
    }
    if (scrDebuggerGlob.add)
    {
        if (*pElementNode)
            return 0;
        elementNodec = Scr_AllocDebugMem(8, "Scr_RefToVariable2");
        *elementNodec = (unsigned int)scrDebuggerGlob.currentElement;
        elementNodec[1] = (unsigned int)breakpoints->list;
        breakpoints->list = (Scr_WatchElementNode_s *)elementNodec;
    }
    else
    {
        elementNode = *pElementNode;
        if (!*pElementNode)
            return 0;
        if (elementNode->element != scrDebuggerGlob.currentElement)
            MyAssertHandler(
                ".\\script\\scr_debugger.cpp",
                7306,
                0,
                "%s",
                "elementNode->element == scrDebuggerGlob.currentElement");
        *pElementNode = elementNode->next;
        elementNode->next = breakpoints->removedList;
        breakpoints->removedList = elementNode;
        if (!breakpoints->list)
        {
            if (isObject)
            {
                if (scrVarPub.evaluate && (!Scr_GetRefCountToObject(ida - 1) || !IsFieldObject(ida - 1)))
                {
                    for (elementNodea = breakpoints->removedList; elementNodea; elementNodea = elementNodea->next)
                    {
                        scrDebuggerGlob.removeId = ida;
                        element = elementNodea->element;
                        if (Scr_RefScriptExpression(&elementNodea->element->expr))
                        {
                            //Scr_ScriptWatch::EvaluateWatchElementExpression(&scrDebuggerGlob.scriptWatch, element, &value);
                            scrDebuggerGlob.scriptWatch.EvaluateWatchElementExpression(element, &value);
                            if (scrVarPub.error_message)
                                Scr_ClearErrorMessage();
                            RemoveRefToValue(value.type, value.u);
                        }
                        scrDebuggerGlob.removeId = 0;
                    }
                }
                RemoveRefToObject(ida - 1);
            }
            for (elementNodeb = breakpoints->removedList; elementNodeb; elementNodeb = elementNodeNext)
            {
                elementNodeNext = elementNodeb->next;
                Scr_FreeDebugMem(elementNodeb);
            }
            Scr_FreeDebugMem(breakpoints);
            scrDebuggerGlob.variableBreakpoints[ida] = 0;
        }
    }
    return 0;
}

Scr_OpcodeList_s *Scr_BreakOnAllAssignmentPos()
{
    Scr_OpcodeList_s *result; // eax
    Scr_OpcodeList_s *opcodeElement; // [esp+0h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 7155, 0, "%s", "!Sys_IsRemoteDebugClient()");
    result = (Scr_OpcodeList_s *)scrDebuggerGlob.assignBreakpointSet;
    if (!scrDebuggerGlob.assignBreakpointSet)
    {
        scrDebuggerGlob.assignBreakpointSet = 1;
        scrDebuggerGlob.objectId = 1;
        for (opcodeElement = scrDebuggerGlob.assignHead; opcodeElement; opcodeElement = result)
        {
            Scr_AddAssignmentBreakpoint((unsigned __int8 *)opcodeElement->codePos);
            result = opcodeElement->next;
        }
    }
    return result;
}

void __cdecl Scr_AddAssignmentBreakpoint(unsigned __int8 *codePos)
{
    if (*codePos == 135)
    {
        *codePos = -119;
    }
    else
    {
        Scr_AddBreakpoint(codePos);
        *codePos = -120;
    }
}

void Scr_Step()
{
    bool evaluate; // [esp+3h] [ebp-9h]
    unsigned int localId; // [esp+4h] [ebp-8h] BYREF
    const char *codePos; // [esp+8h] [ebp-4h]

    if (Sys_IsRemoteDebugClient())
    {
        if (scrDebuggerGlob.atBreakpoint)
        {
            Sys_WriteDebugSocketMessageType(0x18u);
            Sys_WriteDebugSocketInt(scrDebuggerGlob.step_mode);
            Sys_EndWriteDebugSocket();
            Scr_ResumeBreakpoints();
        }
    }
    else
    {
        clientUIActives[0].keyCatchers &= ~2u;
        if (scrDebuggerGlob.step_mode && scrVmPub.function_count)
        {
            if (scrDebuggerGlob.step_mode == 3)
            {
                scrDebuggerGlob.step_mode = 1;
                codePos = Scr_GetReturnPos(&localId);
            }
            else
            {
                evaluate = scrVarPub.evaluate;
                scrVarPub.evaluate = 1;
                if (scrDebuggerGlob.breakpointOpcode < 0)
                    MyAssertHandler(
                        ".\\script\\scr_debugger.cpp",
                        1628,
                        0,
                        "%s\n\t(scrDebuggerGlob.breakpointOpcode) = %i",
                        "(scrDebuggerGlob.breakpointOpcode >= 0)",
                        scrDebuggerGlob.breakpointOpcode);
                codePos = Scr_GetNextCodepos(scrDebuggerGlob.breakpointTop, scrDebuggerGlob.breakpointCodePos, scrDebuggerGlob.breakpointOpcode, scrDebuggerGlob.step_mode, &localId);
                scrVarPub.evaluate = evaluate;
            }
            Scr_SetTempBreakpoint((char*)codePos, localId);
        }
    }
}

void __cdecl Scr_InitDebuggerMain()
{
    if (scrVarPub.developer)
    {
        if (scrDebuggerGlob.debugger_inited_main)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 7941, 0, "%s", "!scrDebuggerGlob.debugger_inited_main");
        if (!Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.variableBreakpoints = (Scr_WatchElementDoubleNode_t **)Hunk_AllocDebugMem(393216);// , "scrDebuggerGlob.variableBreakpoints");
            memset((unsigned __int8 *)scrDebuggerGlob.variableBreakpoints, 0, 0x60000u);
            scrDebuggerGlob.assignHead = 0;
            scrDebuggerGlob.assignHeadCodePos = 0;
            scrDebuggerGlob.disableBreakpoints = 0;
        }
        //UI_ScrollPane::Init(&scrDebuggerGlob.scriptScrollPane);
        scrDebuggerGlob.scriptScrollPane.Init();
        //UI_ScrollPane::Init(&scrDebuggerGlob.miscScrollPane);
        scrDebuggerGlob.miscScrollPane.Init();
        //UI_VerticalDivider::Init(&scrDebuggerGlob.mainWindow);
        scrDebuggerGlob.mainWindow.Init();
        scrDebuggerGlob.debugger_inited_main = 1;
    }
}

void __cdecl Scr_ShutdownDebuggerMain()
{
    int j; // [esp+0h] [ebp-8h]
    Scr_OpcodeList_s *opcodeElement; // [esp+4h] [ebp-4h]

    if (scrVarPub.developer && scrDebuggerGlob.debugger_inited_main)
    {
        scrDebuggerGlob.debugger_inited_main = 0;
        if (!Sys_IsRemoteDebugClient())
        {
            if (scrDebuggerGlob.variableBreakpoints)
            {
                for (j = 0; j < 98304; ++j)
                {
                    if (scrDebuggerGlob.variableBreakpoints[j])
                        MyAssertHandler(".\\script\\scr_debugger.cpp", 8003, 0, "%s", "!scrDebuggerGlob.variableBreakpoints[j]");
                }
                Hunk_FreeDebugMem(scrDebuggerGlob.variableBreakpoints);
                scrDebuggerGlob.variableBreakpoints = 0;
            }
            while (scrDebuggerGlob.assignHead)
            {
                opcodeElement = scrDebuggerGlob.assignHead->next;
                Hunk_FreeDebugMem(scrDebuggerGlob.assignHead);
                scrDebuggerGlob.assignHead = opcodeElement;
            }
        }
    }
}

void __cdecl Scr_InitDebugger()
{
    if (scrVarPub.developer && scrCompilePub.script_loading)
    {
        if (scrDebuggerGlob.debugger_inited)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8028, 0, "%s", "!scrDebuggerGlob.debugger_inited");
        if (!Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.breakpoints = (char *)Hunk_AllocDebugMem(scrCompilePub.programLen);// , "scrDebuggerGlob.breakpoints");
            memset((unsigned __int8 *)scrDebuggerGlob.breakpoints, 0x7Fu, scrCompilePub.programLen);
        }
        //Scr_ScriptList::Init(&scrDebuggerGlob.scriptList);
        scrDebuggerGlob.scriptList.Init();
        //Scr_OpenScriptList::Init(&scrDebuggerGlob.openScriptList);
        scrDebuggerGlob.openScriptList.Init();
        scrDebuggerGlob.debugger_inited = 1;
        if (cls.uiStarted)
            UI_Component_Init();
    }
}

void __cdecl Scr_ShutdownDebugger()
{
    if (scrVarPub.developer && scrDebuggerGlob.debugger_inited)
    {
        scrDebuggerGlob.debugger_inited = 0;
        //Scr_OpenScriptList::Shutdown(&scrDebuggerGlob.openScriptList);
        scrDebuggerGlob.openScriptList.Shutdown();
        //Scr_ScriptList::Shutdown(&scrDebuggerGlob.scriptList);
        scrDebuggerGlob.scriptList.Shutdown();
        if (!Sys_IsRemoteDebugClient())
        {
            if (scrDebuggerGlob.breakpoints)
            {
                Hunk_FreeDebugMem(scrDebuggerGlob.breakpoints);
                scrDebuggerGlob.breakpoints = 0;
            }
        }
        scrDebuggerGlob.debugger_inited = 0;
    }
}

void __cdecl Scr_SetSelectionComp(UI_Component *comp)
{
    UI_Component::selectionComp = comp;
    if (comp == &scrDebuggerGlob.scriptScrollPane && scrDebuggerGlob.scriptList.selectedLine >= 0)
    {
        //Scr_AbstractScriptList::AddEntry(&scrDebuggerGlob.openScriptList, scrDebuggerGlob.scriptList.scriptWindows[scrDebuggerGlob.scriptList.selectedLine], 0);
        scrDebuggerGlob.openScriptList.AddEntry(scrDebuggerGlob.scriptList.scriptWindows[scrDebuggerGlob.scriptList.selectedLine], 0);
    }
}

void __cdecl Scr_InitDebuggerSystem()
{
    if (scrVarPub.developer)
    {
        if (scrDebuggerGlob.debugger_inited_system)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8088, 0, "%s", "!scrDebuggerGlob.debugger_inited_system");
        Scr_InitBreakpoints();
        if (!Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.nextBreakpointCodePos = 0;
            scrDebuggerGlob.killThreadCodePos = 0;
            scrDebuggerGlob.breakpointCount = 0;
            if (scrVarPub.evaluate)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 8103, 0, "%s", "!scrVarPub.evaluate");
            scrVarPub.evaluate = 1;
        }
        scrDebuggerGlob.assignBreakpointSet = 0;
        scrDebuggerGlob.breakpointPos.bufferIndex = -1;
        scrDebuggerGlob.atBreakpoint = 0;
        scrDebuggerGlob.run_debugger = 0;
        scrDebuggerGlob.scriptWatch.Init();
        scrDebuggerGlob.gainFocusTime = 0;
        scrDebuggerGlob.scriptList.LoadScriptPos();
        scrDebuggerGlob.scriptCallStack.Init();
        if (Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.atBreakpoint = 1;
            scrDebuggerGlob.step_mode = 0;
            Scr_Step();
        }
        else
        {
            if (!scrVarPub.evaluate)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 8128, 0, "%s", "scrVarPub.evaluate");
            scrVarPub.evaluate = 0;
        }
        scrDebuggerGlob.debugger_inited_system = 1;
        if (scrDebuggerGlob.mainWindow.posY == 0.0)
            scrDebuggerGlob.mainWindow.posY = UI_Component::g.screenHeight - 120.0;
        scrDebuggerGlob.miscScrollPane.comp = &scrDebuggerGlob.scriptWatch;
        scrDebuggerGlob.scriptList.selectionParent = &scrDebuggerGlob.miscScrollPane;
        scrDebuggerGlob.openScriptList.selectionParent = &scrDebuggerGlob.miscScrollPane;
        scrDebuggerGlob.scriptWatch.selectionParent = &scrDebuggerGlob.miscScrollPane;
        scrDebuggerGlob.scriptCallStack.selectionParent = &scrDebuggerGlob.miscScrollPane;
        Scr_SetSelectionComp(&scrDebuggerGlob.miscScrollPane);
        if (!Sys_IsRemoteDebugClient())
        {
            scrDebuggerGlob.scriptWatch.UpdateBreakpoints(1);
        }
    }
}

void Scr_InitBreakpoints()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 0x7F; ++i)
        g_breakpoints[i].next = &g_breakpoints[i + 1];
    if (g_breakpoints[127].next)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            617,
            0,
            "%s",
            "!g_breakpoints[ARRAY_COUNT( g_breakpoints ) - 1].next");
    g_breakpointsHead = g_breakpoints;
}

void __cdecl Scr_ShutdownDebuggerSystem(int restart)
{
    if (scrVarPub.developer)
    {
        if (!restart && Key_IsCatcherActive(0, 2))
        {
            Key_RemoveCatcher(0, -3);
            IN_ActivateMouse(1);
        }
        if (scrDebuggerGlob.debugger_inited_system)
        {
            scrDebuggerGlob.debugger_inited_system = 0;
            scrVarPub.evaluate = 0;
            //Scr_ScriptWatch::Shutdown(&scrDebuggerGlob.scriptWatch);
            scrDebuggerGlob.scriptWatch.Shutdown();
            if (!Sys_IsRemoteDebugClient())
            {
                if (scrDebuggerGlob.nextBreakpointCodePos)
                {
                    Scr_RemoveManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.nextBreakpointCodePos);
                    scrDebuggerGlob.nextBreakpointCodePos = 0;
                }
                if (scrDebuggerGlob.killThreadCodePos)
                {
                    Scr_RemoveManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.killThreadCodePos);
                    scrDebuggerGlob.killThreadCodePos = 0;
                }
                if (scrDebuggerGlob.breakpointCount)
                    MyAssertHandler(".\\script\\scr_debugger.cpp", 8219, 0, "%s", "!scrDebuggerGlob.breakpointCount");
            }
        }
    }
}

void __cdecl Scr_AddAssignmentPos(char *codePos)
{
    Scr_OpcodeList_s *v1; // eax

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8258, 0, "%s", "scrVarPub.developer");
    if (scrCompilePub.developer_statement != 2 && scrDebuggerGlob.assignHeadCodePos != codePos)
    {
        scrDebuggerGlob.assignHeadCodePos = codePos;
        v1 = (Scr_OpcodeList_s *)Hunk_AllocDebugMem(8);
        v1->codePos = codePos;
        v1->next = scrDebuggerGlob.assignHead;
        scrDebuggerGlob.assignHead = v1;
    }
}

void __cdecl Scr_RunDebuggerRemote()
{
    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8623, 0, "%s", "Sys_IsRemoteDebugClient()");
    if (Key_IsCatcherActive(0, 2))
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            8624,
            0,
            "%s",
            "!Key_IsCatcherActive( ONLY_LOCAL_CLIENT_NUM, KEYCATCH_SCRIPT )");
    Con_CloseConsole(0);
    Key_AddCatcher(0, 2);
    IN_ActivateMouse(1);
    while (Key_IsCatcherActive(0, 2))
        Debug_Frame(0);
    IN_ActivateMouse(1);
}

void __cdecl Scr_RunDebugger()
{
    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8647, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (scrVarPub.developer)
    {
        if (!Scr_IsStackClear())
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8657, 0, "%s", "Scr_IsStackClear()");
        if (scrVmPub.function_count)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8659, 0, "%s", "!scrVmPub.function_count");
        scrDebuggerGlob.scriptWatch.localId = 0;
        Scr_DisplayDebugger();
        if (!Scr_IsStackClear())
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8663, 0, "%s", "Scr_IsStackClear()");
    }
}

// KISAKTODO: move in client somewhere
void __cdecl CL_EndScriptDebugger(int timeSpentInDebugger)
{
#ifdef KISAK_MP
    if (clientUIActives[0].connectionState == CA_ACTIVE)
    {
        Con_TimeNudged(0, timeSpentInDebugger);
        CL_AdjustTimeDelta(0);
    }
#endif
}

void __cdecl Scr_ShutdownRemoteClient(int restart)
{
    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8244, 0, "%s", "Sys_IsRemoteDebugClient()");
    Scr_ShutdownDebuggerSystem(restart);
    Scr_ShutdownDebugger();
    Scr_ShutdownDebuggerMain();
    Scr_ShutdownOpcodeLookup();
}

Scr_WatchElement_s *Scr_DisplayDebugger()
{
    const char *varUsagePos; // [esp+0h] [ebp-14h]
    unsigned int keyCatchers; // [esp+4h] [ebp-10h]
    int startTime; // [esp+Ch] [ebp-8h]
    int remoteScreenUpdateNesting; // [esp+10h] [ebp-4h]

    iassert(!Sys_IsRemoteDebugClient());

    if (scrDebuggerGlob.atBreakpoint)
    {
        scrDebuggerGlob.scriptCallStack.UpdateStack();
    }
    varUsagePos = scrVarPub.varUsagePos;
    scrVarPub.varUsagePos = "<script debugger variable>";

    iassert(!scrVmPub.outparamcount);
    iassert(!scrVmPub.inparamcount);
    iassert(!scrVarPub.evaluate);

    scrVarPub.evaluate = 1;
    
    scrDebuggerGlob.scriptWatch.UpdateBreakpoints(0);

    Scr_Evaluate();
    
    scrDebuggerGlob.scriptWatch.UpdateHeight();
    scrDebuggerGlob.scriptCallStack.UpdateHeight();

    if (scrDebuggerGlob.showConsole)
        scrDebuggerGlob.showConsole = 0;
    else
        Con_CloseConsole(0);

#ifndef KISAK_SP // on SP, we are in the SERVER thread
    iassert(Sys_IsMainThread());
#endif
    iassert(!Key_IsCatcherActive(ONLY_LOCAL_CLIENT_NUM, KEYCATCH_SCRIPT));

    Key_AddCatcher(ONLY_LOCAL_CLIENT_NUM, KEYCATCH_SCRIPT);

    if ((clientUIActives[0].keyCatchers & KEYCATCH_SCRIPT) != 0)
    {
        startTime = cls.realtime;
        keyCatchers = clientUIActives[0].keyCatchers & 0xFFFFFFFC;
        clientUIActives[0].keyCatchers &= 3u;
        IN_ActivateMouse(1);
        remoteScreenUpdateNesting = R_PopRemoteScreenUpdate();

        while ((clientUIActives[0].keyCatchers & KEYCATCH_SCRIPT) != 0)
            Debug_Frame(0);

        R_PushRemoteScreenUpdate(remoteScreenUpdateNesting);
        IN_ActivateMouse(1);
        clientUIActives[0].keyCatchers = keyCatchers | clientUIActives[0].keyCatchers & 3;
        CL_EndScriptDebugger(cls.realtime - startTime);
    }

    scrDebuggerGlob.scriptWatch.UpdateBreakpoints(1);

    iassert(scrVarPub.evaluate);

    scrVarPub.evaluate = 0;
    scrVarPub.varUsagePos = varUsagePos;
    return Scr_ResumeBreakpoints();
}

void __cdecl Scr_WatchElementHitBreakpoint(Scr_WatchElement_s *element, bool enabled)
{
    if (enabled)
    {
        element->hitBreakpoint = 1;
    }
    else
    {
        element->changed = 1;
        element->changedTime = 0;
    }
}

void __cdecl Scr_ShowConsole()
{
    Con_OpenConsole(0);
    Con_OpenConsoleOutput(0);
    scrDebuggerGlob.showConsole = 1;
}

void __cdecl Scr_ClearElementChanged(Scr_WatchElement_s *element)
{
    Scr_WatchElement_s *childElement; // [esp+0h] [ebp-4h]

    if (!element->breakpoint)
    {
        element->changed = 0;
        for (childElement = element->childHead; childElement; childElement = childElement->next)
            Scr_ClearElementChanged(childElement);
    }
}

void Scr_ClearElementsChanged()
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    for (element = scrDebuggerGlob.scriptWatch.elementHead; element; element = element->next)
        Scr_ClearElementChanged(element);
}

void Scr_DisplayHitBreakpoint()
{
    if (!scrDebuggerGlob.atBreakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8674, 0, "%s", "scrDebuggerGlob.atBreakpoint");
    Scr_SelectScriptLine(scrDebuggerGlob.breakpointPos.bufferIndex, scrDebuggerGlob.breakpointPos.lineNum);
    scrDebuggerGlob.scriptCallStack.selectedLine = 0;
    Scr_ClearElementsChanged();
}

void Scr_HitBreakpointInternal()
{
    //Scr_ScriptWatch::SortHitBreakpointsTop(&scrDebuggerGlob.scriptWatch);
    scrDebuggerGlob.scriptWatch.SortHitBreakpointsTop();

    Scr_DisplayHitBreakpoint();
    Scr_DisplayDebugger();
    Scr_ResetTimeout();
}

int __cdecl Scr_HitBreakpoint(VariableValue *top, char *pos, unsigned int localId, int hitBreakpoint)
{
    Scr_Breakpoint *breakpoint; // [esp+0h] [ebp-24h]
    bool hitStepBreakpoint; // [esp+Bh] [ebp-19h]
    const char *codePos; // [esp+10h] [ebp-14h]
    bool kill_thread; // [esp+17h] [ebp-Dh]
    int opcode; // [esp+18h] [ebp-Ch]
    Scr_WatchElement_s *element; // [esp+1Ch] [ebp-8h]
    bool existsBreakpoint; // [esp+23h] [ebp-1h]

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8830, 0, "%s", "scrVarPub.developer");
    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8832, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (!scrVmPub.function_count)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8834, 0, "%s", "scrVmPub.function_count");
    if (scrDebuggerGlob.breakpointPos.bufferIndex != -1)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            8835,
            0,
            "%s",
            "scrDebuggerGlob.breakpointPos.bufferIndex == NO_BUFFER_INDEX");
    if (scrDebuggerGlob.atBreakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8836, 0, "%s", "!scrDebuggerGlob.atBreakpoint");
    codePos = pos - 1;
    opcode = (unsigned __int8)*Scr_FindBreakpointInfo(pos - 1);
    hitStepBreakpoint = 0;
    existsBreakpoint = 0;
    if (scrDebuggerGlob.nextBreakpointCodePos == pos - 1)
    {
        if (scrDebuggerGlob.nextBreakpointThreadId && localId != scrDebuggerGlob.nextBreakpointThreadId)
            existsBreakpoint = 1;
        else
            hitStepBreakpoint = 1;
    }
    kill_thread = scrDebuggerGlob.kill_thread;
    if (scrDebuggerGlob.kill_thread)
    {
        scrDebuggerGlob.kill_thread = 0;
        opcode = 0;
        if (scrDebuggerGlob.killThreadCodePos)
        {
            if (scrDebuggerGlob.killThreadCodePos != codePos)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 8861, 0, "%s", "scrDebuggerGlob.killThreadCodePos == codePos");
            Scr_RemoveManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.killThreadCodePos);
            scrDebuggerGlob.killThreadCodePos = 0;
            existsBreakpoint = 1;
        }
    }
    else if (scrDebuggerGlob.killThreadCodePos)
    {
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8870, 0, "%s", "!scrDebuggerGlob.killThreadCodePos");
    }
    scrVmPub.top = top;
    if (scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8875, 0, "%s", "!scrVarPub.evaluate");
    scrVarPub.evaluate = 1;
    for (element = scrDebuggerGlob.scriptWatch.elementHead; element; element = element->next)
    {
        breakpoint = element->breakpoint;
        if (breakpoint && breakpoint->codePos == codePos)
        {
            existsBreakpoint = 1;
            if (Scr_ConditionalExpression(element, localId))
                hitBreakpoint = 1;
        }
    }
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8892, 0, "%s", "scrVarPub.evaluate");
    scrVarPub.evaluate = 0;
    if (!hitBreakpoint && !hitStepBreakpoint && existsBreakpoint)
        return opcode;
    if (scrDebuggerGlob.nextBreakpointCodePos)
    {
        Scr_RemoveManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.nextBreakpointCodePos);
        scrDebuggerGlob.nextBreakpointCodePos = 0;
    }
    if (!Scr_AllowBreakpoint(pos))
        return opcode;
    scrDebuggerGlob.breakpointTop = top;
    scrDebuggerGlob.breakpointCodePos = pos - 1;
    scrDebuggerGlob.breakpointOpcode = opcode;
    if (Scr_GetSourcePosOfType(codePos, !kill_thread, &scrDebuggerGlob.breakpointPos)
        && (!hitStepBreakpoint
            || scrDebuggerGlob.step_mode == 4
            || scrDebuggerGlob.prevBreakpointLineNum != scrDebuggerGlob.breakpointPos.lineNum))
    {
        scrVmPub.function_frame->fs.pos = pos;
        scrDebuggerGlob.scriptWatch.localId = localId;
        Scr_HitBreakpointInternal();
        return opcode;
    }
    else
    {
        if (!hitStepBreakpoint)
            scrDebuggerGlob.step_mode = 4;
        Scr_Step();
        scrDebuggerGlob.breakpointPos.bufferIndex = -1;
        return opcode;
    }
}

bool __cdecl Scr_ConditionalExpression(Scr_WatchElement_s *element, unsigned int localId)
{
    unsigned int Self; // eax
    bool v4; // [esp+0h] [ebp-20h]
    Scr_WatchElement_s *conditionalElement; // [esp+8h] [ebp-18h]
    Scr_WatchElement_s *conditionalElementa; // [esp+8h] [ebp-18h]
    VariableValue newValue; // [esp+18h] [ebp-8h] BYREF

    for (conditionalElement = element->next; ; conditionalElement = conditionalElement->next)
    {
        if (!conditionalElement || conditionalElement->breakpointType != 2)
        {
            v4 = element->breakpointType == 1 || element->breakpointType == 5 || element->breakpointType == 6;
            Scr_WatchElementHitBreakpoint(element, v4);
            for (conditionalElementa = element->next;
                conditionalElementa && conditionalElementa->breakpointType == 2;
                conditionalElementa = conditionalElementa->next)
            {
                Scr_WatchElementHitBreakpoint(conditionalElementa, v4);
            }
            return v4;
        }
        if (conditionalElement->breakpoint)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8732, 0, "%s", "!conditionalElement->breakpoint");
        if (!conditionalElement->expr.exprHead)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8734, 0, "%s", "expr->exprHead");
        Scr_EvalScriptExpression(&conditionalElement->expr, localId, &newValue, 0, 1);
        if (newValue.type == 1)
            break;
        Scr_CastBool(&newValue);
        if (scrVarPub.error_message)
        {
            Scr_ClearErrorMessage();
            return 0;
        }
        if (newValue.type != 6)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 8770, 0, "%s", "newValue.type == VAR_INTEGER");
        if (!newValue.u.intValue)
            return 0;
    LABEL_2:
        ;
    }
    RemoveRefToObject(newValue.u.stringValue);
    switch (GetObjectType(newValue.u.stringValue))
    {
    case 0xEu:
    case 0xFu:
    case 0x10u:
    case 0x11u:
        if (newValue.u.intValue == localId)
            goto LABEL_2;
        break;
    case 0x12u:
    case 0x13u:
    case 0x14u:
        if (localId)
        {
            Self = Scr_GetSelf(localId);
            if (newValue.u.intValue == Self)
                goto LABEL_2;
        }
        break;
    default:
        return 0;
    }
    return 0;
}

void __cdecl Scr_HitBuiltinBreakpoint(
    VariableValue *top,
    const char *pos,
    unsigned int localId,
    int opcode,
    int builtinIndex,
    unsigned int outparamcount)
{
    Scr_Breakpoint *breakpoint; // [esp+0h] [ebp-10h]
    bool hitBreakpoint; // [esp+7h] [ebp-9h]
    Scr_WatchElement_s *element; // [esp+8h] [ebp-8h]
    bool existsBreakpoint; // [esp+Fh] [ebp-1h]

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8937, 0, "%s", "scrVarPub.developer");
    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8939, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (!scrVmPub.function_count)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8941, 0, "%s", "scrVmPub.function_count");
    if (scrDebuggerGlob.breakpointPos.bufferIndex != -1)
        MyAssertHandler(
            ".\\script\\scr_debugger.cpp",
            8942,
            0,
            "%s",
            "scrDebuggerGlob.breakpointPos.bufferIndex == NO_BUFFER_INDEX");
    if (scrDebuggerGlob.atBreakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8943, 0, "%s", "!scrDebuggerGlob.atBreakpoint");
    hitBreakpoint = 0;
    existsBreakpoint = 0;
    scrVmPub.top = top;
    scrVmPub.outparamcount = 0;
    scrVmPub.breakpointOutparamcount = outparamcount;
    if (scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8952, 0, "%s", "!scrVarPub.evaluate");
    scrVarPub.evaluate = 1;
    for (element = scrDebuggerGlob.scriptWatch.elementHead; element; element = element->next)
    {
        breakpoint = element->breakpoint;
        if (breakpoint && !breakpoint->codePos && breakpoint->builtinIndex == builtinIndex)
        {
            existsBreakpoint = 1;
            if (Scr_ConditionalExpression(element, localId))
                hitBreakpoint = 1;
        }
    }
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8971, 0, "%s", "scrVarPub.evaluate");
    scrVarPub.evaluate = 0;
    if (!existsBreakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8974, 0, "%s", "existsBreakpoint");
    if (hitBreakpoint)
        Scr_SpecialBreakpoint(top, (char*)pos, localId, opcode, 8);
}

void __cdecl Scr_DebugKillThread(unsigned int threadId, const char *codePos)
{
    bool enabled; // [esp+3h] [ebp-11h]
    int hitBreakpoint; // [esp+4h] [ebp-10h]
    Scr_WatchElementDoubleNode_t *breakpoints; // [esp+8h] [ebp-Ch]
    Scr_WatchElementNode_s *elementNode; // [esp+Ch] [ebp-8h]
    Scr_WatchElement_s *element; // [esp+10h] [ebp-4h]

    if (!scrDebuggerGlob.variableBreakpoints)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8990, 0, "%s", "scrDebuggerGlob.variableBreakpoints");
    breakpoints = scrDebuggerGlob.variableBreakpoints[threadId + 1];
    if (breakpoints && scrVarPub.evaluate)
    {
        hitBreakpoint = 0;
        for (elementNode = breakpoints->list; elementNode; elementNode = elementNode->next)
        {
            element = elementNode->element;
            if (!elementNode->element->breakpointType)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9003, 0, "%s", "element->breakpointType != SCR_BREAKPOINT_NONE");
            if ((element->breakpointType == 1 || element->breakpointType == 3) && element->objectId == threadId)
            {
                enabled = element->breakpointType == 1;
                Scr_WatchElementHitBreakpoint(element, enabled);
                element->deadCodePos = codePos;
                if (enabled)
                    hitBreakpoint = 1;
            }
        }
        if (hitBreakpoint)
        {
            if (scrVmPub.function_count)
            {
                if (scrDebuggerGlob.killThreadCodePos)
                {
                    if (scrDebuggerGlob.killThreadCodePos != scrVmPub.function_frame->fs.pos)
                        MyAssertHandler(
                            ".\\script\\scr_debugger.cpp",
                            9029,
                            0,
                            "%s",
                            "scrDebuggerGlob.killThreadCodePos == scrVmPub.function_frame->fs.pos");
                    Scr_RemoveManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.killThreadCodePos);
                    scrDebuggerGlob.killThreadCodePos = 0;
                }
                if (scrDebuggerGlob.nextBreakpointCodePos)
                    Scr_RemoveManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.nextBreakpointCodePos);
                scrDebuggerGlob.nextBreakpointCodePos = (char *)scrVmPub.function_frame->fs.pos;
                Scr_AddManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.nextBreakpointCodePos);
                scrDebuggerGlob.prevBreakpointLineNum = -1;
                scrDebuggerGlob.nextBreakpointThreadId = scrVmPub.function_frame->fs.localId;
            }
            else
            {
                scrDebuggerGlob.run_debugger = 1;
            }
        }
    }
}

void __cdecl Scr_DebugTerminateThread(int topThread)
{
    Scr_DebugKillThread(
        scrVmPub.function_frame_start[topThread].fs.localId,
        scrVmPub.stack[3 * topThread - 96].u.codePosValue);
    if (topThread == scrVmPub.function_count)
    {
        if (!scrDebuggerGlob.kill_thread)
        {
            scrDebuggerGlob.kill_thread = 1;
            if (scrDebuggerGlob.killThreadCodePos)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9058, 0, "%s", "!scrDebuggerGlob.killThreadCodePos");
            if (*scrVmPub.function_frame->fs.pos != 135 && *scrVmPub.function_frame->fs.pos != 137)
            {
                scrDebuggerGlob.killThreadCodePos = (char *)scrVmPub.function_frame->fs.pos;
                Scr_AddManualBreakpoint((unsigned __int8 *)scrDebuggerGlob.killThreadCodePos);
            }
        }
    }
    else
    {
        scrVmPub.stack[3 * topThread - 96].u.intValue = (int)&g_EndPos;
    }
}

void __cdecl Scr_ReadRemoteFile()
{
    char v0; // [esp+13h] [ebp-9Dh]
    char *v1; // [esp+18h] [ebp-98h]
    char *v2; // [esp+1Ch] [ebp-94h]
    char extFilename[64]; // [esp+20h] [ebp-90h] BYREF
    char filename[64]; // [esp+60h] [ebp-50h] BYREF
    int len; // [esp+A4h] [ebp-Ch]
    unsigned int name; // [esp+A8h] [ebp-8h]
    char *sourceBuf; // [esp+ACh] [ebp-4h]

    Sys_ReadDebugSocketStringBuffer(extFilename, 64);
    v2 = extFilename;
    v1 = filename;
    do
    {
        v0 = *v2;
        *v1++ = *v2++;
    } while (v0);
    extFilename[&filename[strlen(filename) + 1] - &filename[1] + 60] = 0;
    len = Sys_ReadDebugSocketInt();
    if (len >= 0)
    {
        name = SL_GetString_(filename, 0, 7);
        if (FindVariable(scrCompilePub.loadedscripts, name))
            MyAssertHandler(".\\script\\scr_parser.cpp", 832, 0, "%s", "!FindVariable( scrCompilePub.loadedscripts, name )");
        GetNewVariable(scrCompilePub.loadedscripts, name);
        SL_RemoveRefToString(name);
        Hunk_CheckTempMemoryHighClear();
        sourceBuf = (char*)Hunk_AllocateTempMemoryHigh(len + 1, "Scr_ReadRemoteFile");
        Sys_ReadDebugSocketData(sourceBuf, len, 1);
        sourceBuf[len] = 0;
        Scr_AddSourceBufferInternal(extFilename, 0, sourceBuf, len, 0, 0);
        Hunk_ClearTempMemoryHigh();
    }
    else
    {
        Scr_AddSourceBufferInternal(extFilename, 0, 0, -1, 0, 0);
    }
}

int __cdecl Scr_UpdateDebugSocket()
{
    int newEvent; // [esp+4h] [ebp-Ch]
    int dirty; // [esp+8h] [ebp-8h]

    dirty = scrDebuggerGlob.scriptWatch.dirty;
    newEvent = 0;
    while (2)
    {
        switch (Sys_UpdateDebugSocket())
        {
        case 1:
            if (!Sys_IsRemoteDebugClient())
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9264, 0, "%s", "Sys_IsRemoteDebugClient()");
            Scr_ReadRemoteFile();
            goto LABEL_46;
        case 2:
            if (!Sys_IsRemoteDebugClient())
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9271, 0, "%s", "Sys_IsRemoteDebugClient()");
            Scr_ShutdownRemoteClient(1);
            Scr_BeginLoadScripts();
            goto LABEL_46;
        case 3:
            if (!Sys_IsRemoteDebugClient())
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9279, 0, "%s", "Sys_IsRemoteDebugClient()");
            Scr_EndLoadScripts();
            Scr_InitDebuggerSystem();
            goto LABEL_46;
        case 5:
            Scr_AddTextRemote();
            goto LABEL_46;
        case 7:
            Scr_PasteElementRemote();
            goto LABEL_46;
        case 9:
            Scr_InsertElementRemote();
            goto LABEL_46;
        case 11:
            Scr_DeleteElementRemote();
            goto LABEL_46;
        case 13:
            Scr_BackspaceElementRemote();
            goto LABEL_46;
        case 15:
            Scr_CloneElementRemote();
            goto LABEL_46;
        case 17:
            Scr_SelectElementRemote();
            goto LABEL_46;
        case 18:
            Scr_ToggleExpandElementRemote();
            goto LABEL_46;
        case 21:
            Scr_ToggleBreakpointRemote();
            goto LABEL_46;
        case 22:
            Scr_HitBreakpointRemote();
            goto LABEL_46;
        case 23:
            Scr_SortHitBreakpointsTopRemote();
            goto LABEL_46;
        case 27:
            Scr_ToggleWatchElementBreakpointRemote();
            goto LABEL_46;
        case 28:
            Scr_WatchElementHitBreakpointRemote();
            goto LABEL_46;
        case 29:
            Scr_FreeWatchElementChildrenRemote();
            goto LABEL_46;
        case 30:
            Scr_SetElementObjectTypeRemote();
            goto LABEL_46;
        case 31:
            Scr_SetElementThreadStartRemote();
            goto LABEL_46;
        case 32:
            Scr_SetElementValueTextRemote();
            goto LABEL_46;
        case 33:
            Scr_SetNonFieldRefTextRemote();
            goto LABEL_46;
        case 34:
            Scr_SetElementRefTextRemote();
            goto LABEL_46;
        case 35:
            Scr_SetChildCountRemote();
            goto LABEL_46;
        case 36:
            Scr_UpdateRemote();
            goto LABEL_46;
        case 37:
            Scr_UpdateWatchHeightRemote();
            goto LABEL_46;
        case 38:
            Scr_KeepAliveRemote();
            goto LABEL_46;
        case 40:
            Scr_SelectScriptLineRemote();
            goto LABEL_46;
        case 41:
            Scr_SortElementChildrenRemote();
            if (!dirty || scrDebuggerGlob.scriptWatch.dirty)
                goto LABEL_46;
            newEvent = 1;
            Sys_AckDebugSocket();
            goto LABEL_43;
        case 42:
            Sys_ConsolePrintRemote(0);
            goto LABEL_46;
        case 43:
            CL_ConsoleFixPosition();
            goto LABEL_46;
        case 44:
            Cbuf_AddText(0, "toggle cl_paused\n");
        LABEL_46:
            newEvent = 1;
            Sys_AckDebugSocket();
            continue;
        default:
        LABEL_43:
            Sys_FlushDebugSocketData();
            if (Sys_IsRemoteDebugClient())
                NET_Sleep(1);
            return newEvent;
        }
    }
}

void Scr_ToggleBreakpointRemote()
{
    bool overwrite; // [esp+5h] [ebp-Fh]
    unsigned __int8 breakpointType; // [esp+6h] [ebp-Eh]
    bool force; // [esp+7h] [ebp-Dh]
    Scr_ScriptWindow *scriptWindow; // [esp+8h] [ebp-Ch]
    bool user; // [esp+Fh] [ebp-5h]
    Scr_WatchElement_s *element; // [esp+10h] [ebp-4h]

    scriptWindow = scrDebuggerGlob.scriptList.scriptWindows[Sys_ReadDebugSocketInt()];
    scriptWindow->selectedLine = Sys_ReadDebugSocketInt();
    element = Scr_ReadElement();
    force = Sys_ReadDebugSocketInt() != 0;
    overwrite = Sys_ReadDebugSocketInt() != 0;
    breakpointType = Sys_ReadDebugSocketInt();
    user = Sys_ReadDebugSocketInt() != 0;
    //Scr_ScriptWindow::ToggleBreakpointInternal(scriptWindow, element, force, overwrite, breakpointType, user);
    scriptWindow->ToggleBreakpointInternal(element, force, overwrite, breakpointType, user);
}

void Scr_SelectScriptLineRemote()
{
    int bufferIndex; // [esp+0h] [ebp-8h]
    int lineNum; // [esp+4h] [ebp-4h]

    bufferIndex = Sys_ReadDebugSocketInt();
    lineNum = Sys_ReadDebugSocketInt();
    Scr_SelectScriptLine(bufferIndex, lineNum);
}

void Scr_UpdateWatchHeightRemote()
{
    //Scr_ScriptWatch::UpdateHeight(&scrDebuggerGlob.scriptWatch);
    scrDebuggerGlob.scriptWatch.UpdateHeight();
}

void Scr_SelectElementRemote()
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    element = Scr_ReadElement();
    //Scr_ScriptWatch::SetSelectedElement(&scrDebuggerGlob.scriptWatch, element, 0);
    scrDebuggerGlob.scriptWatch.SetSelectedElement(element, 0);
}

void Scr_ToggleExpandElementRemote()
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 3972, 0, "%s", "Sys_IsRemoteDebugClient()");
    element = Scr_ReadElement();
    if (element)
    {
        //Scr_ScriptWatch::ToggleExpandElement(&scrDebuggerGlob.scriptWatch, element);
        scrDebuggerGlob.scriptWatch.ToggleExpandElement(element);
    }
}

void Scr_PasteElementRemote()
{
    bool user; // [esp+6h] [ebp-Ah]
    Scr_WatchElement_s *element; // [esp+8h] [ebp-8h]
    char *text; // [esp+Ch] [ebp-4h]

    element = Scr_ReadElement();
    text = Sys_ReadDebugSocketString();
    user = Sys_ReadDebugSocketInt() != 0;
    //Scr_ScriptWatch::PasteNonBreakpointElement(&scrDebuggerGlob.scriptWatch, element, text, user);
    scrDebuggerGlob.scriptWatch.PasteNonBreakpointElement(element, text, user);
    FreeString(text);
}

Scr_WatchElement_s *Scr_InsertElementRemote()
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    element = Scr_ReadElement();
    //return Scr_ScriptWatch::PasteNonBreakpointElement(&scrDebuggerGlob.scriptWatch, element, (char *)"", 1);
    return scrDebuggerGlob.scriptWatch.PasteNonBreakpointElement(element, (char *)"", 1);
}

void Scr_DeleteElementRemote()
{
    Scr_WatchElement_s *result; // eax

    result = Scr_ReadElement();
    if (result)
    {
        //return Scr_ScriptWatch::DeleteElementInternal(&scrDebuggerGlob.scriptWatch, result);
        scrDebuggerGlob.scriptWatch.DeleteElementInternal(result);
    }
}

Scr_WatchElement_s *Scr_BackspaceElementRemote()
{
    Scr_WatchElement_s *result; // eax

    result = Scr_ReadElement();
    if (result)
    {
        //return Scr_ScriptWatch::BackspaceElementInternal(&scrDebuggerGlob.scriptWatch, result);
        return scrDebuggerGlob.scriptWatch.BackspaceElementInternal(result);
    }
    return result;
}

void Scr_FreeWatchElementChildrenRemote()
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5010, 0, "%s", "Sys_IsRemoteDebugClient()");
    element = Scr_ReadElement();
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5013, 0, "%s", "element");
    Scr_FreeWatchElementChildren(element);
}

bool Scr_SetElementObjectTypeRemote()
{
    bool result; // eax
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5295, 0, "%s", "Sys_IsRemoteDebugClient()");
    element = Scr_ReadElement();
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5298, 0, "%s", "element");
    element->objectType = Sys_ReadDebugSocketInt();
    result = Sys_ReadDebugSocketInt() != 0;
    element->directObject = result;
    return result;
}

int Scr_SetElementThreadStartRemote()
{
    int result; // eax
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5316, 0, "%s", "Sys_IsRemoteDebugClient()");
    element = Scr_ReadElement();
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5319, 0, "%s", "element");
    element->bufferIndex = Sys_ReadDebugSocketInt();
    result = Sys_ReadDebugSocketInt();
    element->sourcePos = result;
    return result;
}

void Scr_SetElementValueTextRemote()
{
    char valueText[256]; // [esp+0h] [ebp-108h] BYREF
    Scr_WatchElement_s *element; // [esp+104h] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5370, 0, "%s", "Sys_IsRemoteDebugClient()");
    element = Scr_ReadElement();
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5373, 0, "%s", "element");
    if (*element->valueText)
    {
        element->changed = 1;
        element->changedTime = 0;
    }
    Sys_ReadDebugSocketStringBuffer(valueText, 256);
    ReplaceString(&element->valueText, valueText);
}

void Scr_SetNonFieldRefTextRemote()
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5471, 0, "%s", "Sys_IsRemoteDebugClient()");
    element = Scr_ReadElement();
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5474, 0, "%s", "element");
    element->fieldName = Sys_ReadDebugSocketInt();
    Scr_SetNonFieldElementRefText(element);
}

void Scr_SetElementRefTextRemote()
{
    char fieldText[136]; // [esp+0h] [ebp-90h] BYREF
    Scr_WatchElement_s *element; // [esp+8Ch] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5565, 0, "%s", "Sys_IsRemoteDebugClient()");
    element = Scr_ReadElement();
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5568, 0, "%s", "element");
    Sys_ReadDebugSocketStringBuffer(fieldText, 129);
    Scr_SetElementRefText(element, fieldText);
}

void Scr_SortElementChildrenRemote()
{
    Scr_SelectedLineInfo info; // [esp+0h] [ebp-10h] BYREF
    Scr_WatchElement_s *parentElement; // [esp+Ch] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5708, 0, "%s", "Sys_IsRemoteDebugClient()");
    //Scr_ScriptWatch::SaveSelectedLine(&scrDebuggerGlob.scriptWatch, &info);
    scrDebuggerGlob.scriptWatch.SaveSelectedLine(&info);
    parentElement = Scr_ReadElement();
    if (!parentElement)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5713, 0, "%s", "parentElement");
    if (!Scr_IsSortWatchElement(parentElement))
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5715, 0, "%s", "Scr_IsSortWatchElement( parentElement )");
    if (!scrDebuggerGlob.scriptWatch.dirty)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 5717, 0, "%s", "scrDebuggerGlob.scriptWatch.dirty");
    --scrDebuggerGlob.scriptWatch.dirty;
    Scr_SortElementChildren(parentElement);
    //Scr_ScriptWatch::UpdateHeight(&scrDebuggerGlob.scriptWatch);
    scrDebuggerGlob.scriptWatch.UpdateHeight();
    //Scr_ScriptWatch::LoadSelectedLine(&scrDebuggerGlob.scriptWatch, &info);
    scrDebuggerGlob.scriptWatch.LoadSelectedLine(&info);
}

void Scr_SetChildCountRemote()
{
    const char *v0; // eax
    const char *v1; // eax
    Scr_WatchElement_s *childElement; // [esp+0h] [ebp-40h]
    int newIndex; // [esp+4h] [ebp-3Ch]
    signed int oldChildCount; // [esp+8h] [ebp-38h]
    Scr_SelectedLineInfo info; // [esp+Ch] [ebp-34h] BYREF
    Scr_WatchElement_s *newElements; // [esp+18h] [ebp-28h]
    Scr_WatchElement_s *parentElement; // [esp+1Ch] [ebp-24h]
    Scr_WatchElement_s *oldElements; // [esp+20h] [ebp-20h]
    Scr_WatchElement_s *newElement; // [esp+24h] [ebp-1Ch]
    Scr_WatchElement_s *oldElement; // [esp+28h] [ebp-18h]
    int compareResult; // [esp+2Ch] [ebp-14h]
    int oldIndex; // [esp+30h] [ebp-10h]
    int nameIndex; // [esp+34h] [ebp-Ch]
    int count; // [esp+38h] [ebp-8h]
    bool sameType; // [esp+3Fh] [ebp-1h]

    if (!scrDebuggerGlob.debugger_inited_system)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6200, 0, "%s", "scrDebuggerGlob.debugger_inited_system");
    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6201, 0, "%s", "Sys_IsRemoteDebugClient()");
    //Scr_ScriptWatch::SaveSelectedLine(&scrDebuggerGlob.scriptWatch, &info);
    scrDebuggerGlob.scriptWatch.SaveSelectedLine(&info);
    parentElement = Scr_ReadElement();
    if (!parentElement)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 6206, 0, "%s", "parentElement");
    if (Scr_IsSortWatchElement(parentElement))
    {
        if (!++scrDebuggerGlob.scriptWatch.dirty)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 6211, 0, "%s", "scrDebuggerGlob.scriptWatch.dirty");
    }
    count = Sys_ReadDebugSocketInt();
    sameType = Sys_ReadDebugSocketInt() != 0;
    oldElements = parentElement->childArrayHead;
    oldChildCount = parentElement->childCount;
    newElements = (Scr_WatchElement_s *)Scr_AllocDebugMem(100 * count, "Scr_SetChildCountRemote");
    memset((unsigned __int8 *)newElements, 0, 100 * count);
    oldIndex = 0;
    newIndex = 0;
    for (nameIndex = 0; nameIndex < count; ++nameIndex)
    {
        newElement = &newElements[newIndex];
        newElement->parent = parentElement;
        if (!++scrDebuggerGlob.scriptWatch.elementId)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 6229, 0, "%s", "scrDebuggerGlob.scriptWatch.elementId");
        newElement->id = scrDebuggerGlob.scriptWatch.elementId;
        v0 = CopyString((char *)"");
        newElement->valueText = v0;
        v1 = CopyString((char *)"");
        newElement->refText = v1;
        if (sameType)
        {
            while (oldIndex < oldChildCount)
            {
                oldElement = &oldElements[oldIndex];
                compareResult = Sys_ReadDebugSocketInt();
                if (!compareResult)
                {
                    if (!scrDebuggerGlob.scriptWatch.elementId)
                        MyAssertHandler(".\\script\\scr_debugger.cpp", 6245, 0, "%s", "scrDebuggerGlob.scriptWatch.elementId");
                    --scrDebuggerGlob.scriptWatch.elementId;
                    newElement->expand = oldElement->expand;
                    newElement->childArrayHead = oldElement->childArrayHead;
                    newElement->childHead = oldElement->childHead;
                    newElement->childCount = oldElement->childCount;
                    newElement->objectType = oldElement->objectType;
                    newElement->oldObjectType = oldElement->oldObjectType;
                    newElement->directObject = oldElement->directObject;
                    newElement->bufferIndex = oldElement->bufferIndex;
                    newElement->sourcePos = oldElement->sourcePos;
                    newElement->changed = oldElement->changed;
                    newElement->changedTime = oldElement->changedTime;
                    if (!oldElement->id)
                        MyAssertHandler(".\\script\\scr_debugger.cpp", 6258, 0, "%s", "oldElement->id");
                    newElement->id = oldElement->id;
                    ReplaceString(&newElement->valueText, (char *)oldElement->valueText);
                    ReplaceString(&newElement->refText, (char *)oldElement->refText);
                    for (childElement = oldElement->childHead; childElement; childElement = childElement->next)
                        childElement->parent = newElement;
                    ++oldIndex;
                    break;
                }
                if (compareResult > 0)
                    break;
                Scr_FreeWatchElementChildren(oldElement);
                ++oldIndex;
            }
        }
        ++newIndex;
    }
    while (oldIndex < oldChildCount)
    {
        oldElement = &oldElements[oldIndex];
        Scr_FreeWatchElementChildren(oldElement);
        ++oldIndex;
    }
    if (oldElements)
        Scr_FreeDebugMem(oldElements);
    parentElement->childCount = count;
    parentElement->childArrayHead = newElements;
    Scr_ConnectElementChildren(parentElement);
    //Scr_ScriptWatch::LoadSelectedLine(&scrDebuggerGlob.scriptWatch, &info);
    scrDebuggerGlob.scriptWatch.LoadSelectedLine(&info);
}

void Scr_AddTextRemote()
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-8h]
    char *text; // [esp+4h] [ebp-4h]

    element = Scr_ReadElement();
    if (element)
    {
        text = Sys_ReadDebugSocketString();
        //Scr_ScriptWatch::AddElement(&scrDebuggerGlob.scriptWatch, element, text);
        scrDebuggerGlob.scriptWatch.AddElement(element, text);
        FreeString(text);
    }
}

void Scr_CloneElementRemote()
{
    Scr_WatchElement_s *element; // [esp+0h] [ebp-4h]
    Scr_WatchElement_s *elementa; // [esp+0h] [ebp-4h]

    element = Scr_ReadElement();
    if (element)
    {
        //elementa = Scr_ScriptWatch::CloneElement(&scrDebuggerGlob.scriptWatch, element);
        elementa = scrDebuggerGlob.scriptWatch.CloneElement(element);
        //Scr_ScriptWatch::SetSelectedElement(&scrDebuggerGlob.scriptWatch, elementa, 1);
        scrDebuggerGlob.scriptWatch.SetSelectedElement(elementa, 1);
    }
}

void Scr_ToggleWatchElementBreakpointRemote()
{
    unsigned __int8 type; // [esp+3h] [ebp-5h]
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    element = Scr_ReadElement();
    type = Sys_ReadDebugSocketInt();
    if (element)
    {
        //Scr_ScriptWatch::ToggleBreakpointInternal(&scrDebuggerGlob.scriptWatch, element, type);
        scrDebuggerGlob.scriptWatch.ToggleBreakpointInternal(element, type);
    }
}

void Scr_UpdateRemote()
{
    int line; // [esp+0h] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 8424, 0, "%s", "Sys_IsRemoteDebugClient()");
    scrDebuggerGlob.scriptCallStack.numLines = Sys_ReadDebugSocketInt();
    for (line = 0; line < scrDebuggerGlob.scriptCallStack.numLines; ++line)
    {
        scrDebuggerGlob.scriptCallStack.stack[line].bufferIndex = Sys_ReadDebugSocketInt();
        scrDebuggerGlob.scriptCallStack.stack[line].sourcePos = Sys_ReadDebugSocketInt();
    }
    //Scr_ScriptWatch::UpdateHeight(&scrDebuggerGlob.scriptWatch);
    scrDebuggerGlob.scriptWatch.UpdateHeight();
    //UI_LinesComponent::UpdateHeight(&scrDebuggerGlob.scriptCallStack);
    scrDebuggerGlob.scriptCallStack.UpdateHeight();
    Sys_WriteDebugSocketMessageType(0x26u);
    Sys_EndWriteDebugSocket();
}

void Scr_HitBreakpointRemote()
{
    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9071, 0, "%s", "Sys_IsRemoteDebugClient()");
    scrDebuggerGlob.gainFocusTime = Sys_Milliseconds() + 500;
    scrDebuggerGlob.atBreakpoint = 1;
    scrDebuggerGlob.breakpointPos.bufferIndex = Sys_ReadDebugSocketInt();
    scrDebuggerGlob.breakpointPos.lineNum = Sys_ReadDebugSocketInt();
    Scr_DisplayHitBreakpoint();
}

void Scr_WatchElementHitBreakpointRemote()
{
    bool enabled; // [esp+3h] [ebp-5h]
    Scr_WatchElement_s *element; // [esp+4h] [ebp-4h]

    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9122, 0, "%s", "Sys_IsRemoteDebugClient()");
    element = Scr_ReadElement();
    enabled = Sys_ReadDebugSocketInt() != 0;
    if (!element)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9126, 0, "%s", "element");
    Scr_WatchElementHitBreakpoint(element, enabled);
}

void Scr_KeepAliveRemote()
{
    if (scrDebuggerGlob.atBreakpoint)
    {
        Sys_WriteDebugSocketMessageType(0x26u);
        Sys_EndWriteDebugSocket();
    }
}

void Scr_SortHitBreakpointsTopRemote()
{
    if (!Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9145, 0, "%s", "Sys_IsRemoteDebugClient()");
    //Scr_ScriptWatch::SortHitBreakpointsTop(&scrDebuggerGlob.scriptWatch);
    scrDebuggerGlob.scriptWatch.SortHitBreakpointsTop();
}

void __cdecl Sys_ConsolePrintRemote(int localClientNum)
{
    char *msg; // [esp+0h] [ebp-4h]

    msg = Sys_ReadDebugSocketString();
    CL_ConsolePrint(localClientNum, 23, msg, 0, 0, 0);
    FreeString(msg);
}

void __cdecl Scr_UpdateDebugger()
{
    int hitBreakpoint; // [esp+0h] [ebp-18h]
    bool updateBreakpoints; // [esp+7h] [ebp-11h]
    Scr_WatchElement_s *element; // [esp+Ch] [ebp-Ch]
    VariableValue newValue; // [esp+10h] [ebp-8h] BYREF

    if (!scrVarPub.developer || !scrDebuggerGlob.debugger_inited_system)
        return;
    if (Sys_IsRemoteDebugClient())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9460, 0, "%s", "!Sys_IsRemoteDebugClient()");
    if (scrDebuggerGlob.atBreakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9462, 0, "%s", "!scrDebuggerGlob.atBreakpoint");
    if (!Scr_IsStackClear())
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9463, 0, "%s", "Scr_IsStackClear()");
    if (!Scr_AllowBreakpoint(0))
        return;
    if (scrDebuggerGlob.gainFocusTime)
    {
        IN_SetForegroundWindow();
        IN_ActivateMouse(1);
        if (scrDebuggerGlob.atBreakpoint || (int)(Sys_Milliseconds() - scrDebuggerGlob.gainFocusTime) >= 0)
            scrDebuggerGlob.gainFocusTime = 0;
    }
    if (scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9483, 0, "%s", "!scrVarPub.evaluate");
    scrVarPub.evaluate = 1;
    scrDebuggerGlob.scriptWatch.localId = 0;
    updateBreakpoints = 0;
    hitBreakpoint = 0;
retry_14:
    for (element = scrDebuggerGlob.scriptWatch.elementHead; element; element = element->next)
    {
        if (element->breakpointType == 1 && element->objectType != 14 && element->objectType != 22)
        {
            if (element->breakpoint)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9498, 0, "%s", "!element->breakpoint");
            if (!element->expr.exprHead)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9500, 0, "%s", "expr->exprHead");
            if (Scr_EvalScriptExpression(&element->expr, 0, &newValue, 1, 1) && !updateBreakpoints)
            {
                Scr_ClearErrorMessage();
                RemoveRefToValue(newValue.type, newValue.u);
            retry2_0:
                updateBreakpoints = 1;
                //Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 0);
                scrDebuggerGlob.scriptWatch.UpdateBreakpoints(0);
                goto retry_14;
            }
            if (scrVarPub.error_message)
            {
                Scr_ClearErrorMessage();
                RemoveRefToValue(newValue.type, newValue.u);
                if (!element->valueDefined)
                    continue;
                if (!updateBreakpoints)
                    goto retry2_0;
            }
            else if (Scr_WatchElementHasSameValue(element, &newValue))
            {
                continue;
            }
            if (Scr_ConditionalExpression(element, 0))
                hitBreakpoint = 1;
        }
    }
    if (updateBreakpoints)
    {
        //Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 1);
        scrDebuggerGlob.scriptWatch.UpdateBreakpoints(true);
    }
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9554, 0, "%s", "scrVarPub.evaluate");
    scrVarPub.evaluate = 0;
    if (hitBreakpoint)
    {
    LABEL_46:
        Scr_StackClear();
        //Scr_ScriptWatch::SortHitBreakpointsTop(&scrDebuggerGlob.scriptWatch);
        scrDebuggerGlob.scriptWatch.SortHitBreakpointsTop();
        Scr_RunDebugger();
    }
    else if (scrDebuggerGlob.run_debugger)
    {
        scrDebuggerGlob.run_debugger = 0;
        goto LABEL_46;
    }
}

char __cdecl Scr_WatchElementHasSameValue(Scr_WatchElement_s *element, VariableValue *newValue)
{
    Vartype_t type; // edx
    VariableValue oldValue; // [esp+0h] [ebp-8h] BYREF

    if (!element->valueDefined)
    {
        RemoveRefToValue(newValue->type, newValue->u);
        return 0;
    }
    type = element->value.type;
    oldValue.u.intValue = element->value.u.intValue;
    oldValue.type = type;
    AddRefToValue(type, oldValue.u);
    Scr_EvalEquality(&oldValue, newValue);
    if (scrVarPub.error_message)
    {
        Scr_ClearErrorMessage();
    }
    else
    {
        if (oldValue.type != 6)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 9097, 0, "%s", "oldValue.type == VAR_INTEGER");
        if (oldValue.u.intValue)
        {
            if (element->value.type != 1)
                return 1;
            if (newValue->type != 1)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9102, 0, "%s", "newValue->type == VAR_POINTER");
            if (GetObjectType(newValue->u.intValue) == element->objectType)
                return 1;
        }
    }
    return 0;
}

int __cdecl Scr_HitAssignmentBreakpoint(VariableValue *top, char *pos, unsigned int localId, int forceBreak)
{
    VariableUnion v5; // [esp+0h] [ebp-40h]
    bool enabled; // [esp+13h] [ebp-2Dh]
    int hitBreakpoint; // [esp+14h] [ebp-2Ch]
    Scr_WatchElementDoubleNode_t *breakpoints; // [esp+1Ch] [ebp-24h]
    Scr_WatchElementDoubleNode_t *breakpointsa; // [esp+1Ch] [ebp-24h]
    bool updateBreakpoints; // [esp+23h] [ebp-1Dh]
    Scr_WatchElementNode_s *elementNode; // [esp+24h] [ebp-1Ch]
    Scr_WatchElementNode_s *elementNodea; // [esp+24h] [ebp-1Ch]
    int opcode; // [esp+30h] [ebp-10h]
    Scr_WatchElement_s *element; // [esp+34h] [ebp-Ch]
    Scr_WatchElement_s *elementa; // [esp+34h] [ebp-Ch]
    VariableValue newValue; // [esp+38h] [ebp-8h] BYREF

    if (!scrVarPub.developer)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9587, 0, "%s", "scrVarPub.developer");
    if (scrDebuggerGlob.atBreakpoint)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9588, 0, "%s", "!scrDebuggerGlob.atBreakpoint");
    scrDebuggerGlob.scriptWatch.localId = 0;
    updateBreakpoints = 0;
    hitBreakpoint = 0;
retry_15:
    if (!scrDebuggerGlob.variableBreakpoints)
        MyAssertHandler(".\\script\\scr_debugger.cpp", 9595, 0, "%s", "scrDebuggerGlob.variableBreakpoints");
    breakpoints = scrDebuggerGlob.variableBreakpoints[scrDebuggerGlob.objectId];
    if (breakpoints)
    {
        if (scrVarPub.evaluate)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 9599, 0, "%s", "!scrVarPub.evaluate");
        scrVarPub.evaluate = 1;
        scrVmPub.top = top;
        for (elementNode = breakpoints->list; ; elementNode = elementNode->next)
        {
            if (!elementNode)
            {
                if (!scrVarPub.evaluate)
                    MyAssertHandler(".\\script\\scr_debugger.cpp", 9658, 0, "%s", "scrVarPub.evaluate");
                scrVarPub.evaluate = 0;
                break;
            }
            element = elementNode->element;
            if (!elementNode->element->breakpointType)
                MyAssertHandler(".\\script\\scr_debugger.cpp", 9607, 0, "%s", "element->breakpointType != SCR_BREAKPOINT_NONE");
            if (element->breakpointType == 1
                && element->objectType != 14
                && element->objectType != 22
                && !element->expr.breakonExpr)
            {
                if (element->breakpoint)
                    MyAssertHandler(".\\script\\scr_debugger.cpp", 9615, 0, "%s", "!element->breakpoint");
                if (!element->expr.exprHead)
                    MyAssertHandler(".\\script\\scr_debugger.cpp", 9617, 0, "%s", "expr->exprHead");
                if (Scr_EvalScriptExpression(&element->expr, 0, &newValue, 1, 1) && !updateBreakpoints)
                {
                    Scr_ClearErrorMessage();
                    RemoveRefToValue(newValue.type, newValue.u);
                retry2_1:
                    updateBreakpoints = 1;
                    scrDebuggerGlob.scriptWatch.UpdateBreakpoints(false);
                    scrDebuggerGlob.scriptWatch.UpdateBreakpoints(true);
                    //Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 0);
                    //Scr_ScriptWatch::UpdateBreakpoints(&scrDebuggerGlob.scriptWatch, 1);
                    if (!scrVarPub.evaluate)
                        MyAssertHandler(".\\script\\scr_debugger.cpp", 9630, 0, "%s", "scrVarPub.evaluate");
                    scrVarPub.evaluate = 0;
                    goto retry_15;
                }
                if (scrVarPub.error_message)
                {
                    Scr_ClearErrorMessage();
                    RemoveRefToValue(newValue.type, newValue.u);
                    if (!element->valueDefined)
                        continue;
                    if (!updateBreakpoints)
                        goto retry2_1;
                }
                else if (Scr_WatchElementHasSameValue(element, &newValue))
                {
                    continue;
                }
                if (Scr_ConditionalExpression(element, localId))
                    hitBreakpoint = 1;
            }
        }
    }
    opcode = (unsigned __int8)*Scr_FindBreakpointInfo(pos - 1);
    switch (opcode)
    {
    case 0:
    case 1:
        scrDebuggerGlob.objectId = 1;
        if (!scrDebuggerGlob.variableBreakpoints)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 9671, 0, "%s", "scrDebuggerGlob.variableBreakpoints");
        breakpointsa = scrDebuggerGlob.variableBreakpoints[localId + 1];
        if (breakpointsa)
        {
            for (elementNodea = breakpointsa->list; elementNodea; elementNodea = elementNodea->next)
            {
                elementa = elementNodea->element;
                if (!elementNodea->element->breakpointType)
                    MyAssertHandler(
                        ".\\script\\scr_debugger.cpp",
                        9678,
                        0,
                        "%s",
                        "element->breakpointType != SCR_BREAKPOINT_NONE");
                if ((elementa->breakpointType == 1 || elementa->breakpointType == 3) && elementa->objectId == localId)
                {
                    enabled = elementa->breakpointType == 1;
                    Scr_WatchElementHitBreakpoint(elementa, enabled);
                    elementa->deadCodePos = pos;
                    if (enabled)
                        hitBreakpoint = 1;
                }
            }
        }
        break;
    case 13:
    case 43:
    case 56:
        scrDebuggerGlob.objectId = scrVarPub.levelId + 1;
        break;
    case 14:
    case 44:
    case 58:
        scrDebuggerGlob.objectId = scrVarPub.animId + 1;
        break;
    case 20:
        scrDebuggerGlob.objectId = scrVarPub.gameId + VARIABLELIST_CHILD_BEGIN;
        break;
    case 33:
    case 34:
    case 48:
    case 49:
    case 50:
    case 51:
    case 54:
    case 55:
    case 60:
    case 61:
        scrDebuggerGlob.objectId = localId + 1;
        break;
    case 35:
    case 36:
    case 46:
    case 47:
        break;
    case 38:
    case 45:
    case 59:
        scrDebuggerGlob.objectId = Scr_GetSelf(localId) + 1;
        break;
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 89:
        if (top->type == 1)
            v5.intValue = top->u.intValue;
        else
            v5.intValue = 0;
        scrDebuggerGlob.objectId = v5.intValue + 1;
        break;
    case 90:
        if (scrVarPub.evaluate)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 9731, 0, "%s", "!scrVarPub.evaluate");
        scrVarPub.evaluate = 1;
        //scrDebuggerGlob.objectId = *(unsigned int *)&Scr_EvalVariableObject(scrVmPub.localVars[-(unsigned __int8)*pos]) + 1;
        //scrDebuggerGlob.objectId = Scr_EvalVariableObject(scrVmPub.localVars[-(unsigned __int8)*pos]).next + 1; // KISAKTODO: shitty
        scrDebuggerGlob.objectId = Scr_EvalVariableObject(scrVmPub.localVars[-*pos]) + 1;
        if (!scrVarPub.evaluate)
            MyAssertHandler(".\\script\\scr_debugger.cpp", 9734, 0, "%s", "scrVarPub.evaluate");
        scrVarPub.evaluate = 0;
        break;
    default:
        scrDebuggerGlob.objectId = 1;
        break;
    }
    if (hitBreakpoint || forceBreak)
        return Scr_HitBreakpoint(top, pos, localId, hitBreakpoint);
    else
        return opcode;
}

bool __cdecl Scr_IgnoreErrors()
{
    return scrDebuggerGlob.disableBreakpoints;
}

void __cdecl Scr_SelectScriptLine(unsigned int bufferIndex, int lineNum)
{
    unsigned int sortedIndex; // [esp+0h] [ebp-8h]

    iassert(bufferIndex < scrParserPub.sourceBufferLookupLen);

    sortedIndex = scrParserPub.sourceBufferLookup[bufferIndex].sortedIndex;

    iassert(sortedIndex < scrParserPub.sourceBufferLookupLen);

    //UI_LinesComponent::SetSelectedLineFocus(&scrDebuggerGlob.scriptList, sortedIndex, 1);
    scrDebuggerGlob.scriptList.SetSelectedLineFocus(sortedIndex, 1);

    scrDebuggerGlob.scriptList.scriptWindows[sortedIndex]->SetSelectedLineFocus(lineNum, 0);

    //Scr_AbstractScriptList::AddEntry(&scrDebuggerGlob.openScriptList, scrDebuggerGlob.scriptList.scriptWindows[sortedIndex], 0);
    scrDebuggerGlob.openScriptList.AddEntry(scrDebuggerGlob.scriptList.scriptWindows[sortedIndex], 0);
}

void Scr_EnableBreakpoints(bool enable)
{
    scrDebuggerGlob.disableBreakpoints = !enable;
}

bool __cdecl Scr_CanDrawScript()
{
    return scrDebuggerGlob.debugger_inited_system && scrDebuggerGlob.scriptWatch.dirty == 0;
}

// LWSS HACK (dont see thisptr for these... but it's just a wrapper for R_*** functions, so who cares)
void __cdecl UI_Component__DrawText(float x, float y, float width, int fontEnum, const float *color, char *text)
{
    float v6; // [esp+1Ch] [ebp-10h]
    float v7; // [esp+20h] [ebp-Ch]
    float v8; // [esp+24h] [ebp-8h]
    int maxChars; // [esp+28h] [ebp-4h]

    if (UI_Component::g.charWidth == 0.0)
        MyAssertHandler((char *)".\\ui\\ui_component.cpp", 132, 0, "%s", "g.charWidth");
    maxChars = (int)(width / UI_Component::g.charWidth);
    v7 = floor(x);
    v8 = UI_Component::g.charHeight + y;
    v6 = floor(v8);
    R_AddCmdDrawText(text, maxChars, cls.consoleFont, v7, v6, 1.0, 1.0, 0.0, color, 0);
}

void __cdecl UI_Component__DrawPic(float x, float y, float width, float height, const float *color, Material *material)
{
    float v6; // [esp+2Ch] [ebp-Ch]
    float v7; // [esp+30h] [ebp-8h]

    v7 = floor(y);
    v6 = floor(x);
    R_AddCmdDrawStretchPic(v6, v7, width, height, 0.0, 0.0, 1.0, 1.0, color, material);
}

void Scr_DrawCurrentFilename()
{
    char *v0; // eax
    char *v1; // eax
    __int64 v2; // [esp+10h] [ebp-C8h]
    int v3; // [esp+14h] [ebp-C4h]
    float v4; // [esp+18h] [ebp-C0h]
    float v5; // [esp+1Ch] [ebp-BCh]
    float colorYellow[4]; // [esp+38h] [ebp-A0h] BYREF
    char filename[128]; // [esp+48h] [ebp-90h] BYREF
    float width; // [esp+CCh] [ebp-Ch]
    float x; // [esp+D0h] [ebp-8h]
    Scr_ScriptWindow *window; // [esp+D4h] [ebp-4h]

    if (scrDebuggerGlob.scriptList.selectedLine >= 0)
    {
        window = scrDebuggerGlob.scriptList.scriptWindows[scrDebuggerGlob.scriptList.selectedLine];
        if (window->selectedLine)
        {
            if (window->selectedLine < 0)
            {
                //v1 = Scr_ScriptWindow::GetFilename(window);
                I_strncpyz(filename, window->GetFilename(), 128);
            }
            else
            {
                //v3 = window->selectedLine + 1;
                //v0 = Scr_ScriptWindow::GetFilename(window);
                Com_sprintf(filename, 0x80u, "%s (%i)", window->GetFilename(), window->selectedLine + 1);
            }
            width = (double)(unsigned int)(&filename[strlen(filename) + 1] - &filename[1]) * UI_Component::g.charWidth;
#ifdef KISAK_MP
            CL_LookupColor(0, 0x33u, colorYellow);
#elif KISAK_SP
            CL_LookupColor(0x33u, colorYellow);
#endif
            x = UI_Component::g.screenWidth - UI_Component::g.scrollBarSize - width;
            v4 = width + UI_Component::g.charWidth;
            v3 = x - UI_Component::g.charWidth;
            //UI_Component::DrawPic(v3, 0.0, v4, UI_Component::g.charHeight, 0, cls.consoleMaterial);
            //UI_Component::DrawText(x, 0.0, width, 5, colorYellow, filename);
            UI_Component__DrawPic(v3, 0.0, v4, UI_Component::g.charHeight, 0, cls.consoleMaterial);
            UI_Component__DrawText(x, 0.0, width, 5, colorYellow, filename);
        }
    }
}

void __cdecl Scr_DrawScript()
{
    BOOL isForegroundWindow; // [esp+18h] [ebp-4h]

    iassert(scrDebuggerGlob.debugger_inited_system);
    iassert(!scrDebuggerGlob.scriptWatch.dirty);

    if (scrDebuggerGlob.gainFocusTime)
    {
        isForegroundWindow = IN_IsForegroundWindow();
        IN_SetForegroundWindow();
        if (!isForegroundWindow)
            IN_ActivateMouse(1);
        if (!scrDebuggerGlob.atBreakpoint || (int)(Sys_Milliseconds() - scrDebuggerGlob.gainFocusTime) >= 0)
            scrDebuggerGlob.gainFocusTime = 0;
    }
    if (!scrDebuggerGlob.scriptList.scriptWindows)
        MyAssertHandler((char *)".\\script\\scr_debugger.cpp", 7727, 0, "%s", "scrDebuggerGlob.scriptList.scriptWindows");
    if (scrDebuggerGlob.scriptList.selectedLine < 0)
    {
        scrDebuggerGlob.mainWindow.topComp = 0;
    }
    else
    {
        scrDebuggerGlob.mainWindow.topComp = &scrDebuggerGlob.scriptScrollPane;
        scrDebuggerGlob.scriptScrollPane.comp = scrDebuggerGlob.scriptList.scriptWindows[scrDebuggerGlob.scriptList.selectedLine];
        scrDebuggerGlob.scriptScrollPane.forceHorScoll = 1;
    }
    scrDebuggerGlob.mainWindow.bottomComp = &scrDebuggerGlob.miscScrollPane;
    scrDebuggerGlob.mainWindow.Draw(0.0f, 0.0f,
        UI_Component::g.screenWidth,
        UI_Component::g.screenHeight,
        0.0f,
        0.0f);
    Scr_DrawCurrentFilename();
    Con_DrawConsole(0);
}