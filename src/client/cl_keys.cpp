#include <universal/q_shared.h>
#include "client.h"
#include <universal/assertive.h>
#include <qcommon/mem_track.h>
#include <qcommon/qcommon.h>
#include <qcommon/cmd.h>
#include <ui/keycodes.h>
#include <universal/com_memory.h>
#include <cgame/cg_local.h>
#include <stringed/stringed_hooks.h>
#include <win32/win_local.h>
#include <universal/com_files.h>
#include <devgui/devgui.h>
#include <script/scr_debugger.h>

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#include <cgame_mp/cg_local_mp.h>
#include <server_mp/server_mp.h>
#elif KISAK_SP
#include <cgame/cg_main.h>
#include <cgame/cg_modelpreviewer.h>
#include "cl_ui.h"
#include "cl_scrn.h"
#endif
#include <universal/profile.h>


keyname_t keynames[96] =
{
  { "TAB", K_TAB },
  { "ENTER", K_ENTER },
  { "ESCAPE", K_ESCAPE },
  { "SPACE", K_SPACE },
  { "BACKSPACE", K_BACKSPACE },
  { "UPARROW", K_UPARROW },
  { "DOWNARROW", K_DOWNARROW },
  { "LEFTARROW", K_LEFTARROW },
  { "RIGHTARROW", K_RIGHTARROW },
  { "ALT", K_ALT },
  { "CTRL", K_CTRL },
  { "SHIFT", K_SHIFT },
  { "CAPSLOCK", K_CAPSLOCK },
  { "F1", K_F1 },
  { "F2", K_F2 },
  { "F3", K_F3 },
  { "F4", K_F4 },
  { "F5", K_F5 },
  { "F6", K_F6 },
  { "F7", K_F7 },
  { "F8", K_F8 },
  { "F9", K_F9 },
  { "F10", K_F10 },
  { "F11", K_F11 },
  { "F12", K_F12 },
  { "INS", K_INS },
  { "DEL", K_DEL },
  { "PGDN", K_PGDN },
  { "PGUP", K_PGUP },
  { "HOME", K_HOME },
  { "END", K_END },
  { "MOUSE1", K_MOUSE1 },
  { "MOUSE2", K_MOUSE2 },
  { "MOUSE3", K_MOUSE3 },
  { "MOUSE4", K_MOUSE4 },
  { "MOUSE5", K_MOUSE5 },
  { "MWHEELUP", K_MWHEELUP },
  { "MWHEELDOWN", K_MWHEELDOWN },
  { "AUX1", K_AUX1 },
  { "AUX2", K_AUX2 },
  { "AUX3", K_AUX3 },
  { "AUX4", K_AUX4 },
  { "AUX5", K_AUX5 },
  { "AUX6", K_AUX6 },
  { "AUX7", K_AUX7 },
  { "AUX8", K_AUX8 },
  { "AUX9", K_AUX9 },
  { "AUX10", K_AUX10 },
  { "AUX11", K_AUX11 },
  { "AUX12", K_AUX12 },
  { "AUX13", K_AUX13 },
  { "AUX14", K_AUX14 },
  { "AUX15", K_AUX15 },
  { "AUX16", K_AUX16 },
  { "KP_HOME", K_KP_HOME },
  { "KP_UPARROW", K_KP_UPARROW },
  { "KP_PGUP", K_KP_PGUP },
  { "KP_LEFTARROW", K_KP_LEFTARROW },
  { "KP_5", K_KP_5 },
  { "KP_RIGHTARROW", K_KP_RIGHTARROW },
  { "KP_END", K_KP_END },
  { "KP_DOWNARROW", K_KP_DOWNARROW },
  { "KP_PGDN", K_KP_PGDN },
  { "KP_ENTER", K_KP_ENTER },
  { "KP_INS", K_KP_INS },
  { "KP_DEL", K_KP_DEL },
  { "KP_SLASH", K_KP_SLASH },
  { "KP_MINUS", K_KP_MINUS },
  { "KP_PLUS", K_KP_PLUS },
  { "KP_NUMLOCK", K_KP_NUMLOCK },
  { "KP_STAR", K_KP_STAR },
  { "KP_EQUALS", K_KP_EQUALS },
  { "PAUSE", K_PAUSE },
  { "SEMICOLON", ';' },
  { "COMMAND", K_COMMAND },
  { "181", K_ASCII_181 },
  { "191", K_ASCII_191 },
  { "223", K_ASCII_223 },
  { "224", K_ASCII_224 },
  { "225", K_ASCII_225 },
  { "228", K_ASCII_228 },
  { "229", K_ASCII_229 },
  { "230", K_ASCII_230 },
  { "231", K_ASCII_231 },
  { "232", K_ASCII_232 },
  { "233", K_ASCII_233 },
  { "236", K_ASCII_236 },
  { "241", K_ASCII_241 },
  { "242", K_ASCII_242 },
  { "243", K_ASCII_243 },
  { "246", K_ASCII_246 },
  { "248", K_ASCII_248 },
  { "249", K_ASCII_249 },
  { "250", K_ASCII_250 },
  { "252", K_ASCII_252 },
  { NULL, K_NONE }
}; // idb
keyname_t keynames_localized[96] =
{
  { "KEY_TAB", K_TAB },
  { "KEY_ENTER", K_ENTER },
  { "KEY_ESCAPE", K_ESCAPE },
  { "KEY_SPACE", K_SPACE },
  { "KEY_BACKSPACE", K_BACKSPACE },
  { "KEY_UPARROW", K_UPARROW },
  { "KEY_DOWNARROW", K_DOWNARROW },
  { "KEY_LEFTARROW", K_LEFTARROW },
  { "KEY_RIGHTARROW", K_RIGHTARROW },
  { "KEY_ALT", K_ALT },
  { "KEY_CTRL", K_CTRL },
  { "KEY_SHIFT", K_SHIFT },
  { "KEY_CAPSLOCK", K_CAPSLOCK },
  { "KEY_F1", K_F1 },
  { "KEY_F2", K_F2 },
  { "KEY_F3", K_F3 },
  { "KEY_F4", K_F4 },
  { "KEY_F5", K_F5 },
  { "KEY_F6", K_F6 },
  { "KEY_F7", K_F7 },
  { "KEY_F8", K_F8 },
  { "KEY_F9", K_F9 },
  { "KEY_F10", K_F10 },
  { "KEY_F11", K_F11 },
  { "KEY_F12", K_F12 },
  { "KEY_INS", K_INS },
  { "KEY_DEL", K_DEL },
  { "KEY_PGDN", K_PGDN },
  { "KEY_PGUP", K_PGUP },
  { "KEY_HOME", K_HOME },
  { "KEY_END", K_END },
  { "KEY_MOUSE1", K_MOUSE1 },
  { "KEY_MOUSE2", K_MOUSE2 },
  { "KEY_MOUSE3", K_MOUSE3 },
  { "KEY_MOUSE4", K_MOUSE4 },
  { "KEY_MOUSE5", K_MOUSE5 },
  { "KEY_MWHEELUP", K_MWHEELUP },
  { "KEY_MWHEELDOWN", K_MWHEELDOWN },
  { "KEY_AUX1", K_AUX1 },
  { "KEY_AUX2", K_AUX2 },
  { "KEY_AUX3", K_AUX3 },
  { "KEY_AUX4", K_AUX4 },
  { "KEY_AUX5", K_AUX5 },
  { "KEY_AUX6", K_AUX6 },
  { "KEY_AUX7", K_AUX7 },
  { "KEY_AUX8", K_AUX8 },
  { "KEY_AUX9", K_AUX9 },
  { "KEY_AUX10", K_AUX10 },
  { "KEY_AUX11", K_AUX11 },
  { "KEY_AUX12", K_AUX12 },
  { "KEY_AUX13", K_AUX13 },
  { "KEY_AUX14", K_AUX14 },
  { "KEY_AUX15", K_AUX15 },
  { "KEY_AUX16", K_AUX16 },
  { "KEY_KP_HOME", K_KP_HOME },
  { "KEY_KP_UPARROW", K_KP_UPARROW },
  { "KEY_KP_PGUP", K_KP_PGUP },
  { "KEY_KP_LEFTARROW", K_KP_LEFTARROW },
  { "KEY_KP_5", K_KP_5 },
  { "KEY_KP_RIGHTARROW", K_KP_RIGHTARROW },
  { "KEY_KP_END", K_KP_END },
  { "KEY_KP_DOWNARROW", K_KP_DOWNARROW },
  { "KEY_KP_PGDN", K_KP_PGDN },
  { "KEY_KP_ENTER", K_KP_ENTER },
  { "KEY_KP_INS", K_KP_INS },
  { "KEY_KP_DEL", K_KP_DEL },
  { "KEY_KP_SLASH", K_KP_SLASH },
  { "KEY_KP_MINUS", K_KP_MINUS },
  { "KEY_KP_PLUS", K_KP_PLUS },
  { "KEY_KP_NUMLOCK", K_KP_NUMLOCK },
  { "KEY_KP_STAR", K_KP_STAR },
  { "KEY_KP_EQUALS", K_KP_EQUALS },
  { "KEY_PAUSE", K_PAUSE },
  { "KEY_SEMICOLON", ';' },
  { "KEY_COMMAND", K_COMMAND },
  { "�", K_ASCII_181 },
  { "KISAK", K_ASCII_191},
  { "�", K_ASCII_223 },
  { "�", K_ASCII_224 },
  { "�", K_ASCII_225 },
  { "�", K_ASCII_228 },
  { "�", K_ASCII_229 },
  { "�", K_ASCII_230 },
  { "�", K_ASCII_231 },
  { "�", K_ASCII_232 },
  { "�", K_ASCII_233 },
  { "�", K_ASCII_236 },
  { "�", K_ASCII_241 },
  { "�", K_ASCII_242 },
  { "�", K_ASCII_243 },
  { "�", K_ASCII_246 },
  { "�", K_ASCII_248 },
  { "�", K_ASCII_249 },
  { "�", K_ASCII_250 },
  { "�", K_ASCII_252 },
  { NULL, K_NONE }
}; // idb

//struct keyname_t *keynames 827b29b0     cl_keys.obj
//struct keyname_t *keynames_localized 827b2d50     cl_keys.obj
//char **frenchNumberKeysMap 827b30f0     cl_keys.obj
//int32_t historyLine          82874d18     cl_keys.obj
//int32_t nextHistoryLine      82875128     cl_keys.obj
//int32_t marker_cl_keys       8287512c     cl_keys.obj
//struct field_t g_consoleField 82875130     cl_keys.obj
//struct PlayerKeyState *playerKeys 82875250     cl_keys.obj
//struct field_t *historyEditLines 828786f0     cl_keys.obj

PlayerKeyState playerKeys[1];
field_t g_consoleField;
field_t historyEditLines[32];
char s_shortestMatch[1024];

int32_t historyLine;
int32_t nextHistoryLine;
bool s_shouldCompleteCmd;
const char *s_completionString;
int32_t s_matchCount;
int32_t s_prefixMatchCount;
bool s_hasExactMatch;

void __cdecl TRACK_cl_keys()
{
    track_static_alloc_internal(playerKeys, 3368, "playerKeys", 10);
    track_static_alloc_internal(&g_consoleField, 280, "g_consoleField", 10);
    track_static_alloc_internal(historyEditLines, 8960, "historyEditLines", 10);
    track_static_alloc_internal(keynames, 768, "keynames", 10);
    track_static_alloc_internal(keynames_localized, 768, "keynames_localized", 10);
    track_static_alloc_internal(s_shortestMatch, 1024, "s_shortestMatch", 3);
}

void __cdecl Field_DrawTextOverride(
    int32_t localClientNum,
    const field_t *edit,
    int32_t x,
    int32_t y,
    int32_t horzAlign,
    int32_t vertAlign,
    char *str,
    int32_t drawLen,
    int32_t cursorPos)
{
    Font_s *font; // [esp+30h] [ebp-24h]
    float xScale; // [esp+34h] [ebp-20h]
    float yAdj; // [esp+38h] [ebp-1Ch]
    const ScreenPlacement *scrPlace; // [esp+3Ch] [ebp-18h]
    float xAdj; // [esp+40h] [ebp-14h]
    float fontScalea; // [esp+44h] [ebp-10h]
    float fontScale; // [esp+44h] [ebp-10h]
    int32_t fontStyle; // [esp+48h] [ebp-Ch]
    float yScale; // [esp+4Ch] [ebp-8h]
    char cursorChar; // [esp+53h] [ebp-1h]

    if (drawLen <= 0)
        MyAssertHandler(".\\client\\cl_keys.cpp", 401, 0, "%s", "drawLen > 0");

    scrPlace = &scrPlaceView[localClientNum];
    if (edit->fixedSize)
    {
        font = cls.consoleFont;
        fontStyle = 0;
        xScale = 1.0;
        yScale = 1.0;
        if (Key_GetOverstrikeMode(localClientNum))
            cursorChar = 95;
        else
            cursorChar = 124;
    }
    else
    {
        fontScalea = edit->charHeight / 48.0;
        font = UI_GetFontHandle(scrPlace, 0, fontScalea);
        fontScale = R_NormalizedTextScale(font, fontScalea);
        if (vertAlign == 5)
            xScale = scrPlace->scaleVirtualToReal[0] * fontScale;
        else
            xScale = fontScale;
        if (vertAlign == 5)
            yScale = scrPlace->scaleVirtualToReal[1] * fontScale;
        else
            yScale = fontScale;
        fontStyle = 3;
        if (Key_GetOverstrikeMode(localClientNum))
            cursorChar = 95;
        else
            cursorChar = 124;
    }
    xAdj = (float)x;
    yAdj = (float)(y + (int)((double)R_TextHeight(font) * yScale));
    CL_DrawTextWithCursor(
        scrPlace,
        str,
        drawLen,
        font,
        xAdj,
        yAdj,
        horzAlign,
        vertAlign,
        xScale,
        yScale,
        colorWhite,
        fontStyle,
        cursorPos,
        cursorChar);
}

void __cdecl Field_Draw(int32_t localClientNum, field_t *edit, int32_t x, int32_t y, int32_t horzAlign, int32_t vertAlign)
{
    char str[1024]; // [esp+0h] [ebp-408h] BYREF

    if (!edit->drawWidth)
        edit->drawWidth = 256;
    if (edit->scroll < 0)
        MyAssertHandler(".\\client\\cl_keys.cpp", 463, 0, "%s\n\t(edit->scroll) = %i", "(edit->scroll >= 0)", edit->scroll);
    I_strncpyz(str, &edit->buffer[edit->scroll], 256 - edit->scroll);
    Field_DrawTextOverride(
        localClientNum,
        edit,
        x,
        y,
        horzAlign,
        vertAlign,
        str,
        edit->drawWidth,
        edit->cursor - edit->scroll);
}

void __cdecl Field_AdjustScroll(const ScreenPlacement *scrPlace, field_t *edit)
{
    double v2; // st7
    char *v3; // [esp+10h] [ebp-34h]
    char *v4; // [esp+14h] [ebp-30h]
    Font_s *font; // [esp+28h] [ebp-1Ch]
    float len; // [esp+2Ch] [ebp-18h]
    float lena; // [esp+2Ch] [ebp-18h]
    float lenb; // [esp+2Ch] [ebp-18h]
    float actualScale; // [esp+30h] [ebp-14h]
    float lineWidth; // [esp+38h] [ebp-Ch]
    float fullLen; // [esp+3Ch] [ebp-8h]
    float fullLena; // [esp+3Ch] [ebp-8h]
    float fontScale; // [esp+40h] [ebp-4h]
    float fontScalea; // [esp+40h] [ebp-4h]

    fontScale = edit->charHeight / 48.0;
    lineWidth = (float)edit->widthInPixels;
    if (edit->fixedSize)
    {
        fontScalea = scrPlace->scaleRealToVirtual[0] * fontScale;
        lineWidth = scrPlace->scaleRealToVirtual[1] * lineWidth;
        font = cls.consoleFont;
        v2 = R_NormalizedTextScale(cls.consoleFont, fontScalea);
    }
    else
    {
        font = UI_GetFontHandle(scrPlace, 0, fontScale);
        v2 = R_NormalizedTextScale(font, fontScale);
    }
    actualScale = v2;
    fullLen = (double)R_TextWidth(edit->buffer, 0, font) * actualScale;
    if (lineWidth <= (double)fullLen)
    {
        len = 0.0;
        while (lineWidth > (double)len)
        {
            if (edit->scroll <= 0)
                break;
            len = (double)R_TextWidth((const char *)&edit->fixedSize + edit->scroll + 3, 0, font) * actualScale;
            if (lineWidth <= (double)len)
                break;
            --edit->scroll;
        }
        do
        {
            fullLena = (double)R_TextWidth(&edit->buffer[edit->scroll], 0, font) * actualScale;
            lena = fullLena - (double)R_TextWidth(&edit->buffer[edit->cursor], 0, font) * actualScale;
            if (lena < 0.0 || lineWidth <= (double)lena)
            {
                if (lena >= 0.0)
                {
                    ++edit->scroll;
                }
                else if (edit->scroll)
                {
                    --edit->scroll;
                }
                else
                {
                    lena = 0.0;
                }
            }
        } while (lena < 0.0 || lineWidth <= (double)lena);
        v3 = &edit->buffer[edit->scroll + 1];
        v4 = &edit->buffer[edit->scroll + 1 + strlen(&edit->buffer[edit->scroll])];
        edit->drawWidth = edit->cursor - edit->scroll;
        lenb = 0.0;
        while (lineWidth > (double)lenb && edit->drawWidth < v4 - v3)
        {
            lenb = (double)R_TextWidth(&edit->buffer[edit->scroll], edit->drawWidth + 1, font) * actualScale;
            if (lineWidth > (double)lenb)
                ++edit->drawWidth;
        }
    }
    else
    {
        edit->scroll = 0;
        edit->drawWidth = SEH_PrintStrlen(edit->buffer);
    }
}

void __cdecl Console_Key(int32_t localClientNum, int32_t key)
{
    bool v2; // [esp+8h] [ebp-1Ch]
    bool v3; // [esp+Ch] [ebp-18h]
    char v4; // [esp+12h] [ebp-12h]
    char v5; // [esp+13h] [ebp-11h]
    int32_t isShiftDown; // [esp+14h] [ebp-10h]
    int32_t isCtrlDown; // [esp+18h] [ebp-Ch]
    int32_t shouldCompleteCmd; // [esp+1Ch] [ebp-8h]
    int32_t isAltDown; // [esp+20h] [ebp-4h]

    shouldCompleteCmd = s_shouldCompleteCmd;
    s_shouldCompleteCmd = 1;
    isShiftDown = playerKeys[localClientNum].keys[K_SHIFT].down;
    isCtrlDown = playerKeys[localClientNum].keys[K_CTRL].down;
    isAltDown = playerKeys[localClientNum].keys[K_ALT].down;
    if (key == 'l' && isCtrlDown)
    {
        Cbuf_AddText(localClientNum, "clear\n");
        return;
    }
    if (key == K_ENTER || key == K_KP_ENTER)
    {
        if (Con_CommitToAutoComplete())
            return;
        Com_Printf(CON_CHANNEL_DONT_FILTER, "]%s\n", g_consoleField.buffer);
        if (Key_IsCatcherActive(localClientNum, 2))
        {
            Scr_AddDebugText(g_consoleField.buffer);
        }
        else if (g_consoleField.buffer[0] == 92 || g_consoleField.buffer[0] == 47)
        {
            Cbuf_AddText(localClientNum, &g_consoleField.buffer[1]);
            Cbuf_AddText(localClientNum, "\n");
        }
        else if (Console_IsClientDisconnected()
            && I_strnicmp(g_consoleField.buffer, "quit", 4)
            && I_strnicmp(g_consoleField.buffer, "kill", 4))
        {
            Cbuf_AddText(localClientNum, g_consoleField.buffer);
            Cbuf_AddText(localClientNum, "\n");
        }
        else
        {
            if (!g_consoleField.buffer[0])
                return;

#ifdef KISAK_MP
            if (!Console_IsRconCmd(g_consoleField.buffer))
            {
                Cbuf_AddText(localClientNum, "cmd say ");
#endif
                Cbuf_AddText(localClientNum, g_consoleField.buffer);
#ifdef KISAK_MP
                Cbuf_AddText(localClientNum, "\n");
            }
#endif
        }
        if (g_consoleField.buffer[0])
        {
            memcpy(&historyEditLines[nextHistoryLine % 32], &g_consoleField, sizeof(field_t));
            historyLine = ++nextHistoryLine;
            if (Key_IsCatcherActive(localClientNum, 2))
                Con_ToggleConsole();
        }
        Field_Clear(&g_consoleField);
        g_consoleField.widthInPixels = g_console_field_width;
        g_consoleField.charHeight = g_console_char_height;
        g_consoleField.fixedSize = 1;
        if (Console_IsClientDisconnected())
            SCR_UpdateScreen();
    }
    else if (Key_IsCatcherActive(localClientNum, 2) || key != K_TAB)
    {
        if (key == K_UPARROW && isCtrlDown)
        {
            Con_CycleAutoComplete(-1);
            return;
        }
        if (key == K_DOWNARROW && isCtrlDown)
        {
            Con_CycleAutoComplete(1);
            return;
        }
        if (key == K_MWHEELUP && isShiftDown)
        {
            v5 = 1;
        }
        else
        {
            v3 = key == K_UPARROW || key == K_KP_UPARROW || tolower(key) == 'p' && isCtrlDown;
            v5 = v3;
        }
        if (v5)
        {
            if (nextHistoryLine - historyLine < 32 && historyLine > 0)
                --historyLine;
            memcpy(&g_consoleField, &historyEditLines[historyLine % 32], sizeof(g_consoleField));
            Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
            Con_AllowAutoCompleteCycling(0);
        }
        else
        {
            if (key == K_MWHEELDOWN && isShiftDown)
            {
                v4 = 1;
            }
            else
            {
                v2 = key == K_DOWNARROW || key == K_KP_DOWNARROW || tolower(key) == 'n' && isCtrlDown;
                v4 = v2;
            }
            if (v4)
            {
                if (!Con_CycleAutoComplete(1) && historyLine != nextHistoryLine)
                {
                    memcpy(&g_consoleField, &historyEditLines[++historyLine % 32], sizeof(g_consoleField));
                    Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
                }
            }
            else
            {
                switch (key)
                {
                case K_PGUP:
                    Con_PageUp();
                    return;
                case K_PGDN:
                    Con_PageDown();
                    return;
                case K_MWHEELUP:
                    Con_PageUp();
                    if (isCtrlDown)
                    {
                        Con_PageUp();
                        Con_PageUp();
                    }
                    break;
                case K_MWHEELDOWN:
                    Con_PageDown();
                    if (isCtrlDown)
                    {
                        Con_PageDown();
                        Con_PageDown();
                    }
                    break;
                default:
                    if (key == K_HOME && isCtrlDown)
                    {
                        Con_Top();
                        return;
                    }
                    if (key == K_END && isCtrlDown)
                    {
                        Con_Bottom();
                        return;
                    }
                    if (key == K_DEL || key == K_ESCAPE)
                    {
                        if (Con_CancelAutoComplete())
                            return;
                    }
                    else if (key == K_RIGHTARROW
                        || key == K_KP_RIGHTARROW
                        || key == K_LEFTARROW
                        || key == K_KP_LEFTARROW
                        || key != K_BACKSPACE && !isCtrlDown && !isAltDown && !isShiftDown)
                    {
                        Con_CommitToAutoComplete();
                    }
                    if (Field_KeyDownEvent(localClientNum, &scrPlaceFull, &g_consoleField, key))
                        Con_AllowAutoCompleteCycling(1);
                    break;
                }
            }
        }
    }
    else
    {
        if (shouldCompleteCmd)
            CompleteCommand();
        else
            Con_CycleAutoComplete(2 * (isShiftDown == 0) - 1);
        s_shouldCompleteCmd = 0;
    }
}

char __cdecl Field_KeyDownEvent(int32_t localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int32_t key)
{
    int32_t OverstrikeMode; // eax
    int32_t len; // [esp+14h] [ebp-Ch]
    int32_t isCtrlDown; // [esp+18h] [ebp-8h]
    char isModified; // [esp+1Fh] [ebp-1h]

    isCtrlDown = playerKeys[localClientNum].keys[K_CTRL].down;
    isModified = 0;
    len = strlen(edit->buffer);
    if ((key == K_INS || key == K_KP_INS) && playerKeys[localClientNum].keys[K_SHIFT].down)
    {
        isModified = Field_Paste(localClientNum, scrPlace, edit);
    }
    else
    {
        switch (key)
        {
        case K_DEL:
            if (edit->cursor < len)
                memmove(
                    (uint8_t *)&edit->buffer[edit->cursor],
                    (uint8_t *)&edit->buffer[edit->cursor + 1],
                    len - edit->cursor);
            break;
        case K_RIGHTARROW:
            if (edit->cursor < len)
                ++edit->cursor;
            if (isCtrlDown)
            {
                while (edit->cursor < len && isalnum(edit->buffer[edit->cursor]))
                    ++edit->cursor;
                while (edit->cursor < len && !isalnum(edit->buffer[edit->cursor]))
                    ++edit->cursor;
            }
            break;
        case K_LEFTARROW:
            if (edit->cursor > 0)
                --edit->cursor;
            if (isCtrlDown)
            {
                while (edit->cursor > 0 && isalnum(*((char *)&edit->fixedSize + edit->cursor + 3)))
                    --edit->cursor;
            }
            if (edit->cursor < edit->scroll)
                edit->scroll = edit->cursor;
            break;
        default:
            if (key == K_HOME || tolower(key) == 'a' && isCtrlDown)
            {
                edit->cursor = 0;
            }
            else if (key == K_END || tolower(key) == 'e' && isCtrlDown)
            {
                edit->cursor = len;
            }
            else if (key == K_INS)
            {
                OverstrikeMode = Key_GetOverstrikeMode(localClientNum);
                Key_SetOverstrikeMode(localClientNum, OverstrikeMode == 0);
            }
            break;
        }
    }
    if (cls.uiStarted)
        Field_AdjustScroll(scrPlace, edit);
    return isModified;
}

char __cdecl Field_Paste(int32_t localClientNum, const ScreenPlacement *scrPlace, field_t *edit)
{
    int32_t v4; // [esp+0h] [ebp-1Ch]
    int32_t i; // [esp+14h] [ebp-8h]
    char *cbd; // [esp+18h] [ebp-4h]

    cbd = Sys_GetClipboardData();
    if (!cbd)
        return 0;
    v4 = strlen(cbd);
    for (i = 0; i < v4; ++i)
        Field_CharEvent(localClientNum, scrPlace, edit, cbd[i]);
    Com_FreeEvent(cbd);
    return 1;
}

bool __cdecl Field_CharEvent(int32_t localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int32_t ch)
{
    uint32_t len; // [esp+10h] [ebp-8h]
    bool isModified; // [esp+17h] [ebp-1h]

    len = strlen(edit->buffer);
    switch (ch)
    {
    case 22:
        isModified = Field_Paste(localClientNum, scrPlace, edit);
        break;
    case 3:
        Field_Clear(edit);
        isModified = 1;
        break;
    case 8:
        isModified = edit->cursor > 0;
        if (edit->cursor > 0)
        {
            memmove(
                (uint8_t *)&edit->fixedSize + edit->cursor + 3,
                (uint8_t *)&edit->buffer[edit->cursor],
                len + 1 - edit->cursor);
            --edit->cursor;
        }
        break;
    case 1:
        edit->cursor = 0;
        edit->scroll = 0;
        return 0;
    case 5:
        edit->cursor = len;
        isModified = 0;
        break;
    default:
        if (ch < 32)
            return 0;
        if (Key_GetOverstrikeMode(localClientNum))
        {
            if (edit->cursor == 255)
                return 0;
            edit->buffer[edit->cursor++] = ch;
        }
        else
        {
            if (len == 255)
                return 0;
            memmove(
                (uint8_t *)&edit->buffer[edit->cursor + 1],
                (uint8_t *)&edit->buffer[edit->cursor],
                len + 1 - edit->cursor);
            edit->buffer[edit->cursor++] = ch;
        }
        if (edit->cursor == len + 1)
            edit->buffer[edit->cursor] = 0;
        isModified = 1;
        break;
    }
    Field_AdjustScroll(scrPlace, edit);
    return isModified;
}

void CompleteCommand()
{
    bool v0; // [esp+28h] [ebp-268h]
    bool v1; // [esp+2Ch] [ebp-264h]
    bool v2; // [esp+40h] [ebp-250h]
    int32_t matchLenAfterCmds; // [esp+44h] [ebp-24Ch] BYREF
    field_t savedField; // [esp+158h] [ebp-138h] BYREF
    bool isDvarCommand; // [esp+27Bh] [ebp-15h]
    int32_t offset; // [esp+27Ch] [ebp-14h]
    const char *originalCommand; // [esp+280h] [ebp-10h]
    int32_t matchLenAfterDvars; // [esp+284h] [ebp-Ch] BYREF
    bool useExactMatch; // [esp+28Bh] [ebp-5h]
    field_t *edit; // [esp+28Ch] [ebp-4h]

    v2 = g_consoleField.buffer[0] == 47 || g_consoleField.buffer[0] == 92;
    offset = v2;
    // LWSS: Remove Punkbuster crap
    //if (!strnicmp(&g_consoleField.buffer[v2], "pb_", 3u))
    //{
    //    strncpy((uint8_t *)pbbuf, (uint8_t *)&g_consoleField.buffer[offset], 0xFFu);
    //    pbbuf[255] = 0;
    //    if (!strnicmp(pbbuf, "pb_sv_", 6u))
    //        PbServerCompleteCommand(pbbuf, 255);
    //    else
    //        PbClientCompleteCommand(pbbuf, 255);
    //    Com_sprintf(g_consoleField.buffer, 0x100u, "\\%s", pbbuf);
    //    g_consoleField.cursor = strlen(g_consoleField.buffer);
    //    Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
    //}
    //else
    {
        s_completionString = Con_TokenizeInput();
        s_matchCount = 0;
        s_prefixMatchCount = 0;
        s_shortestMatch[0] = 0;
        if (*s_completionString)
        {
            originalCommand = s_completionString;
            v1 = Cmd_Argc() > 1 && Con_IsDvarCommand(s_completionString);
            isDvarCommand = v1;
            if (v1)
                s_completionString = Cmd_Argv(1);
            matchLenAfterCmds = 0;
            matchLenAfterDvars = 0;
            if (con_matchPrefixOnly->current.enabled)
            {
                con_ignoreMatchPrefixOnly = 1;
                UpdateMatches(!isDvarCommand, &matchLenAfterCmds, &matchLenAfterDvars);
                if (s_matchCount > con_inputMaxMatchesShown)
                {
                    con_ignoreMatchPrefixOnly = 0;
                    UpdateMatches(!isDvarCommand, &matchLenAfterCmds, &matchLenAfterDvars);
                    if (!s_matchCount)
                    {
                        con_ignoreMatchPrefixOnly = 1;
                        UpdateMatches(!isDvarCommand, &matchLenAfterCmds, &matchLenAfterDvars);
                    }
                }
            }
            else
            {
                con_ignoreMatchPrefixOnly = 0;
                UpdateMatches(!isDvarCommand, &matchLenAfterCmds, &matchLenAfterDvars);
            }
            if (s_matchCount)
            {
                edit = &g_consoleField;
                memcpy(&savedField, &g_consoleField, sizeof(savedField));
                v0 = isDvarCommand || s_matchCount == 1 || s_hasExactMatch && Con_AnySpaceAfterCommand();
                useExactMatch = v0;
                if (isDvarCommand)
                    Com_sprintf(edit->buffer, 0x100u, "\\%s %s", originalCommand, s_shortestMatch);
                else
                    Com_sprintf(edit->buffer, 0x100u, "\\%s", s_shortestMatch);
                edit->cursor = strlen(edit->buffer);
                ConcatRemaining(savedField.buffer, (char *)s_completionString);
                if (useExactMatch)
                {
                    if (!isDvarCommand)
                    {
                        if (Cmd_Argc() == 1)
                        {
                            I_strncat(g_consoleField.buffer, 256, " ");
                        }
                        else if (Cmd_Argc() == 2)
                        {
                            if (matchLenAfterCmds == matchLenAfterDvars)
                                CompleteCmdArgument();
                            else
                                CompleteDvarArgument();
                        }
                    }
                    edit->cursor = strlen(edit->buffer);
                }
                else if (Con_HasTooManyMatchesToShow())
                {
                    Com_Printf(CON_CHANNEL_DONT_FILTER, "]%s\n", g_consoleField.buffer);
                    Cmd_ForEach(PrintMatches);
                    Dvar_ForEachName(PrintMatches);
                }
                Cmd_EndTokenizedString();
                Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
            }
            else
            {
                Cmd_EndTokenizedString();
            }
        }
        else
        {
            Cmd_EndTokenizedString();
        }
    }
}

void __cdecl PrintMatches(const char *s)
{
    if (con_ignoreMatchPrefixOnly && con_matchPrefixOnly->current.enabled
        || !I_strnicmp(s, s_shortestMatch, strlen(s_shortestMatch)))
    {
        if (I_stristr(s, s_shortestMatch))
            Com_Printf(CON_CHANNEL_DONT_FILTER, "    %s\n", s);
    }
}

void __cdecl ConcatRemaining(char *src, char *start)
{
    char* v2; // eax

    v2 = strstr((char *)src, (char *)start);
    if (v2)
        I_strncat(g_consoleField.buffer, 256, (char *)(strlen(start) + v2));
    else
        keyConcatArgs();
}

int32_t keyConcatArgs()
{
    int32_t result; // eax
    char *v1; // eax
    int32_t i; // [esp+0h] [ebp-8h]
    const char *arg; // [esp+4h] [ebp-4h]

    for (i = 1; ; ++i)
    {
        result = Cmd_Argc();
        if (i >= result)
            break;
        I_strncat(g_consoleField.buffer, 256, " ");
        for (arg = Cmd_Argv(i); *arg; ++arg)
        {
            if (*arg == 32)
            {
                I_strncat(g_consoleField.buffer, 256, "\"");
                break;
            }
        }
        v1 = (char *)Cmd_Argv(i);
        I_strncat(g_consoleField.buffer, 256, v1);
        if (*arg == 32)
            I_strncat(g_consoleField.buffer, 256, "\"");
    }
    return result;
}

void CompleteCmdArgument()
{
    const char *cmdArgPrefix; // [esp+10h] [ebp-114h]
    char arg[256]; // [esp+14h] [ebp-110h] BYREF
    const char *cmdName; // [esp+118h] [ebp-Ch]
    const char **files; // [esp+11Ch] [ebp-8h]
    int32_t fileCount; // [esp+120h] [ebp-4h] BYREF

    cmdName = Con_TokenizeInput();
    cmdArgPrefix = Cmd_Argv(1);
    if (*cmdArgPrefix)
    {
        files = Cmd_GetAutoCompleteFileList(cmdName, &fileCount);
        Cmd_EndTokenizedString();
        if (fileCount)
        {
            Con_AutoCompleteFromList(files, fileCount, cmdArgPrefix, arg, 0x100u);
            FS_FreeFileList(files);
            Com_StripExtension(arg, arg);
            ReplaceConsoleInputArgument(strlen(cmdArgPrefix), arg);
        }
    }
    else
    {
        Cmd_EndTokenizedString();
    }
}

void __cdecl ReplaceConsoleInputArgument(int32_t replaceCount, char *replacement)
{
    const char *v2; // eax
    int32_t cmdLineLen; // [esp+10h] [ebp-8h]

    if (!replacement)
        MyAssertHandler(".\\client\\cl_keys.cpp", 845, 0, "%s", "replacement");
    if (*replacement)
    {
        //for (cmdLineLen = strlen(g_consoleField.buffer); cmdLineLen && isspace(*(char *)(cmdLineLen + 11748111)); --cmdLineLen);
        for (cmdLineLen = strlen(g_consoleField.buffer); cmdLineLen && isspace(g_consoleField.buffer[cmdLineLen]); --cmdLineLen);

        if (replaceCount >= cmdLineLen)
        {
            v2 = va("%s: %i, %i", g_consoleField.buffer, replaceCount, cmdLineLen);
            MyAssertHandler(".\\client\\cl_keys.cpp", 852, 0, "%s\n\t%s", "replaceCount < cmdLineLen", v2);
        }
        I_strncpyz(&g_consoleField.buffer[cmdLineLen - replaceCount], replacement, 256 - (cmdLineLen - replaceCount));
    }
}

void CompleteDvarArgument()
{
    const char *dvarName; // [esp+10h] [ebp-114h]
    const dvar_s *dvar; // [esp+14h] [ebp-110h]
    const char *dvarValuePrefix; // [esp+18h] [ebp-10Ch]
    char dvarValue[260]; // [esp+1Ch] [ebp-108h] BYREF

    dvarName = Con_TokenizeInput();
    dvar = Dvar_FindVar(dvarName);
    if (!dvar)
        MyAssertHandler(".\\client\\cl_keys.cpp", 898, 0, "%s", "dvar");
    if (dvar->type == DVAR_TYPE_ENUM)
    {
        dvarValuePrefix = Cmd_Argv(1);
        if (*dvarValuePrefix)
        {
            Con_AutoCompleteFromList(
                dvar->domain.enumeration.strings,
                dvar->domain.enumeration.stringCount,
                dvarValuePrefix,
                dvarValue,
                0x100u);
            ReplaceConsoleInputArgument(strlen(dvarValuePrefix), dvarValue);
        }
        Cmd_EndTokenizedString();
    }
    else
    {
        Cmd_EndTokenizedString();
    }
}

void __cdecl UpdateMatches(bool searchCmds, int32_t *matchLenAfterCmds, int32_t *matchLenAfterDvars)
{
    s_matchCount = 0;
    s_prefixMatchCount = 0;
    s_shortestMatch[0] = 0;
    if (searchCmds)
    {
        Cmd_ForEach((void(__cdecl *)(const char *))FindMatches);
        *matchLenAfterCmds = strlen(s_shortestMatch);
    }
    else
    {
        *matchLenAfterCmds = 0;
    }
    Dvar_ForEachName((void(__cdecl *)(const char *))FindMatches);
    *matchLenAfterDvars = strlen(s_shortestMatch);
}

void __cdecl FindMatches(char *s)
{
    int32_t v1; // esi
    bool v2; // [esp+4h] [ebp-28h]
    bool v3; // [esp+8h] [ebp-24h]
    int32_t v4; // [esp+Ch] [ebp-20h]
    int32_t i; // [esp+28h] [ebp-4h]

    v4 = strlen(s_completionString);
    if (Con_IsAutoCompleteMatch(s, s_completionString, v4))
    {
        v3 = !con_ignoreMatchPrefixOnly && con_matchPrefixOnly->current.enabled || !I_strnicmp(s, s_completionString, v4);
        if (v3)
            ++s_prefixMatchCount;
        if (++s_matchCount == 1 || v3 && s_prefixMatchCount == 1)
        {
            I_strncpyz(s_shortestMatch, s, 1024);
            s_hasExactMatch = 1;
        }
        else if (s_prefixMatchCount)
        {
            if (v3)
            {
                for (i = 0; s[i]; ++i)
                {
                    v1 = tolower(s_shortestMatch[i]);
                    if (v1 != tolower(s[i]))
                        break;
                }
                v2 = !s[i] || s_hasExactMatch && !s_shortestMatch[i];
                s_hasExactMatch = v2;
                s_shortestMatch[i] = 0;
            }
        }
        else
        {
            I_strncpyz(s_shortestMatch, (char *)s_completionString, 1024);
        }
    }
}

bool __cdecl Console_IsRconCmd(const char *commandString)
{
    return I_strncmp(commandString, "rcon", strlen("rcon")) == 0;
}

bool __cdecl Console_IsClientDisconnected()
{
#ifdef KISAK_MP
    return CL_AllLocalClientsDisconnected();
#elif KISAK_SP
    return clientUIActives[0].connectionState == CA_DISCONNECTED;
#endif
}

int32_t __cdecl Key_GetOverstrikeMode(int32_t localClientNum)
{
    return playerKeys[localClientNum].overstrikeMode;
}

void __cdecl Key_SetOverstrikeMode(int32_t localClientNum, int32_t state)
{
    playerKeys[localClientNum].overstrikeMode = state;
}

int32_t __cdecl Key_IsDown(int32_t localClientNum, int32_t keynum)
{
    if (keynum == -1)
        return 0;
    else
        return playerKeys[localClientNum].keys[keynum].down;
}

char tinystr[5];
const char *__cdecl Key_KeynumToString(int32_t keynum, int32_t translate)
{
    char v3; // [esp+0h] [ebp-14h]
    char v4; // [esp+4h] [ebp-10h]
    char j; // [esp+8h] [ebp-Ch]
    keyname_t *kn; // [esp+Ch] [ebp-8h]
    int32_t i; // [esp+10h] [ebp-4h]

    if (keynum == -1)
        return "<KEY NOT FOUND>";
    if (keynum >= 0x100)
        return "<OUT OF RANGE>";
    if (translate && SEH_GetCurrentLanguage() == 1 && keynum >= '0' && keynum <= '9')
        return *(&keynames_localized[72].name + keynum);
    if (keynum > ' ' && keynum < K_BACKSPACE && keynum != '"')
    {
        tinystr[0] = toupper(keynum);
        tinystr[1] = 0;
        if (keynum != ';' || translate)
            return tinystr;
    }
    if (translate)
        kn = keynames_localized;
    else
        kn = keynames;
    while (kn->name)
    {
        if (keynum == kn->keynum)
            return kn->name;
        ++kn;
    }
    i = keynum >> 4;
    j = keynum & 0xF;
    qmemcpy(tinystr, "0x", 2);
    if (keynum >> 4 <= 9)
        v4 = i + 48;
    else
        v4 = i + 87;
    tinystr[2] = v4;
    if ((keynum & 0xFu) <= 9)
        v3 = j + 48;
    else
        v3 = j + 87;
    tinystr[3] = v3;
    tinystr[4] = 0;
    return tinystr;
}

void __cdecl Key_SetBinding(int32_t localClientNum, int32_t keynum, char *binding)
{
    if (keynum != -1)
    {
        ReplaceString(&playerKeys[localClientNum].keys[keynum].binding, binding);
        dvar_modifiedFlags |= 1u;
    }
}

const char *__cdecl Key_GetBinding(int32_t localClientNum, uint32_t keynum)
{
    if (localClientNum)
        MyAssertHandler(
            ".\\client\\cl_keys.cpp",
            1560,
            0,
            "localClientNum doesn't index STATIC_MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1);
    if (keynum >= 0x100)
        MyAssertHandler(
            ".\\client\\cl_keys.cpp",
            1561,
            0,
            "keynum doesn't index MAX_KEYS\n\t%i not in [0, %i)",
            keynum,
            256);
    return playerKeys[localClientNum].keys[keynum].binding;
}

int32_t __cdecl Key_GetCommandAssignment(int32_t localClientNum, const char *command, int32_t *twokeys)
{
    return Key_GetCommandAssignmentInternal(localClientNum, command, twokeys);
}

int32_t __cdecl Key_GetCommandAssignmentInternal(int32_t localClientNum, const char *command, int32_t *twokeys)
{
    int32_t keynum; // [esp+0h] [ebp-Ch]
    const char *binding; // [esp+4h] [ebp-8h]
    int32_t count; // [esp+8h] [ebp-4h]

    twokeys[1] = -1;
    *twokeys = -1;
    count = 0;
    for (keynum = 0; keynum < 256; ++keynum)
    {
        binding = Key_GetBinding(localClientNum, keynum);
        if (binding)
        {
            if (!I_stricmp(binding, command))
            {
                twokeys[count++] = keynum;
                if (count == 2)
                    break;
            }
        }
    }
    return count;
}

bool __cdecl Key_IsCommandBound(int32_t localClientNum, const char *command)
{
    int32_t keys[2]; // [esp+0h] [ebp-Ch] BYREF

    return Key_GetCommandAssignment(localClientNum, command, keys) > 0;
}

void __cdecl Key_Unbind_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int32_t b; // [esp+0h] [ebp-4h]

    if (Cmd_Argc() == 2)
    {
        v0 = Cmd_Argv(1);
        b = Key_StringToKeynum(v0);
        if (b == -1)
        {
            v1 = Cmd_Argv(1);
            Com_Printf(CON_CHANNEL_DONT_FILTER, "\"%s\" isn't a valid key\n", v1);
        }
        else
        {
            Key_SetBinding(0, b, (char *)"");
        }
    }
    else
    {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "unbind <key> : remove commands from a key\n");
    }
}

int32_t __cdecl Key_StringToKeynum(const char *str)
{
    int32_t n2; // [esp+10h] [ebp-Ch]
    int32_t n2a; // [esp+10h] [ebp-Ch]
    int32_t n1; // [esp+14h] [ebp-8h]
    int32_t n1a; // [esp+14h] [ebp-8h]
    keyname_t *kn; // [esp+18h] [ebp-4h]

    if (!str || !*str)
        return -1;
    if (!str[1])
        return *str;
    if (*str == 48 && str[1] == 120 && strlen(str) == 4)
    {
        n1 = str[2];
        if (I_isdigit(n1))
        {
            n1a = n1 - 48;
        }
        else if (n1 < 97 || n1 > 102)
        {
            n1a = 0;
        }
        else
        {
            n1a = n1 - 87;
        }
        n2 = str[3];
        if (I_isdigit(n2))
        {
            n2a = n2 - 48;
        }
        else if (n2 < 97 || n2 > 102)
        {
            n2a = 0;
        }
        else
        {
            n2a = n2 - 87;
        }
        return n2a + 16 * n1a;
    }
    else
    {
        for (kn = keynames; kn->name; ++kn)
        {
            if (!I_stricmp(str, kn->name))
                return kn->keynum;
        }
        return -1;
    }
}

void __cdecl Key_Unbindall_f()
{
    int32_t keynum; // [esp+0h] [ebp-Ch]

    for (keynum = 0; keynum < 256; ++keynum)
    {
        if (playerKeys[0].keys[keynum].binding)
            Key_SetBinding(0, keynum, (char *)"");
    }
}

void __cdecl Key_Bind_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    uint8_t *v4; // eax
    char *v6; // eax
    bool v7; // [esp+0h] [ebp-424h]
    int32_t keynum; // [esp+4h] [ebp-420h]
    int32_t keynuma; // [esp+4h] [ebp-420h]
    int32_t i; // [esp+10h] [ebp-414h]
    const char *binding; // [esp+14h] [ebp-410h]
    int32_t argc; // [esp+18h] [ebp-40Ch]
    char cmd[1028]; // [esp+1Ch] [ebp-408h] BYREF

    argc = Cmd_Argc();
    if (argc >= 2)
    {
        v0 = Cmd_Argv(1);
        keynum = Key_StringToKeynum(v0);
        if (keynum == -1)
        {
            v1 = Cmd_Argv(1);
            Com_Printf(CON_CHANNEL_DONT_FILTER, "\"%s\" isn't a valid key\n", v1);
        }
        else
        {
            keynuma = tolower(keynum);
            if (argc == 2)
            {
                binding = Key_GetBinding(0, keynuma);
                if (binding)
                {
                    v2 = Cmd_Argv(1);
                    Com_Printf(CON_CHANNEL_DONT_FILTER, "\"%s\" = \"%s\"\n", v2, binding);
                }
                else
                {
                    v3 = Cmd_Argv(1);
                    Com_Printf(CON_CHANNEL_DONT_FILTER, "\"%s\" is not bound\n", v3);
                }
            }
            else
            {
                cmd[0] = 0;
                for (i = 2; i < argc; ++i)
                {
                    v7 = 0;
                    if (argc > 3)
                    {
                        v4 = (uint8_t *)Cmd_Argv(i);
                        if (strchr((char *)v4, 0x20u))
                            v7 = 1;
                    }
                    if (v7)
                        I_strncat(cmd, 1024, "\"");
                    v6 = (char *)Cmd_Argv(i);
                    I_strncat(cmd, 1024, v6);
                    if (v7)
                        I_strncat(cmd, 1024, "\"");
                    if (i != argc - 1)
                        I_strncat(cmd, 1024, " ");
                }
                Key_SetBinding(0, keynuma, cmd);
            }
        }
    }
    else
    {
        Com_Printf(CON_CHANNEL_DONT_FILTER, "bind <key> [command] : attach a command to a key\n");
    }
}

void __cdecl Key_WriteBindings(int32_t localClientNum, int32_t f)
{
    char buffer[8196]; // [esp+0h] [ebp-2008h] BYREF

    Key_WriteBindingsToBuffer(localClientNum, buffer, 0x2000);
    FS_Printf(f, "%s", buffer);
}

int32_t __cdecl Key_WriteBindingsToBuffer(int32_t localClientNum, char *buffer, int32_t bufferSize)
{
    const char *v3; // eax
    int32_t bytesUsed; // [esp+0h] [ebp-14h]
    int32_t bytesUseda; // [esp+0h] [ebp-14h]
    const char *bind; // [esp+4h] [ebp-10h]
    KeyState *keys; // [esp+8h] [ebp-Ch]
    int32_t len; // [esp+Ch] [ebp-8h]
    int32_t keyIndex; // [esp+10h] [ebp-4h]
    int32_t bufferSizea; // [esp+24h] [ebp+10h]

    keys = playerKeys[localClientNum].keys;
    bufferSizea = bufferSize - 4;
    bytesUsed = 0;
    for (keyIndex = 0; keyIndex < 256; ++keyIndex)
    {
        if (keys[keyIndex].binding && *keys[keyIndex].binding)
        {
            v3 = Key_KeynumToString(keyIndex, 0);
            len = Com_sprintf(&buffer[bytesUsed], bufferSizea - bytesUsed, "bind %s \"", v3);
            if (len < 0)
                return bytesUsed;
            bytesUseda = len + bytesUsed;
            for (bind = keys[keyIndex].binding; *bind && bytesUseda < bufferSizea; ++bind)
            {
                if (*bind == 34)
                    buffer[bytesUseda++] = 92;
                buffer[bytesUseda++] = *bind;
            }
            buffer[bytesUseda] = 34;
            buffer[bytesUseda + 1] = 10;
            bytesUsed = bytesUseda + 2;
        }
    }
    buffer[bytesUsed] = 0;
    return bytesUsed;
}

void __cdecl Key_Bindlist_f()
{
    const char *v0; // eax
    int32_t keynum; // [esp+0h] [ebp-Ch]
    const char *binding; // [esp+8h] [ebp-4h]

    for (keynum = 0; keynum < 256; ++keynum)
    {
        binding = Key_GetBinding(0, keynum);
        if (binding)
        {
            if (*binding)
            {
                v0 = Key_KeynumToString(keynum, 0);
                Com_Printf(CON_CHANNEL_DONT_FILTER, "%s \"%s\"\n", v0, binding);
            }
        }
    }
}

cmd_function_s Key_Bind_f_VAR;
cmd_function_s Key_Unbind_f_VAR;
cmd_function_s Key_Unbindall_f_VAR;
cmd_function_s Key_Bindlist_f_VAR;
void __cdecl CL_InitKeyCommands()
{
    Cmd_AddCommandInternal("bind", Key_Bind_f, &Key_Bind_f_VAR);
    Cmd_AddCommandInternal("unbind", Key_Unbind_f, &Key_Unbind_f_VAR);
    Cmd_AddCommandInternal("unbindall", Key_Unbindall_f, &Key_Unbindall_f_VAR);
    Cmd_AddCommandInternal("bindlist", Key_Bindlist_f, &Key_Bindlist_f_VAR);
}

bool __cdecl CL_IsConsoleKey(int32_t key)
{
    // K_KP_PGUP isn't apart of ASCII
    return (key == '`' || key == '^' || key == K_KP_PGUP || key == '~');
}

#ifdef KISAK_MP
void __cdecl CL_KeyEvent(int32_t localClientNum, int32_t key, int32_t down, uint32_t time)
{
    PROF_SCOPED("CL_KeyEvent");

    const char *v4; // eax
    bool v6; // [esp+2Fh] [ebp-421h]
    KeyState *keys; // [esp+34h] [ebp-41Ch]
    const char *kb; // [esp+38h] [ebp-418h]
    const char *kba; // [esp+38h] [ebp-418h]
    const char *kbb; // [esp+38h] [ebp-418h]
    connstate_t clcState; // [esp+40h] [ebp-410h]
    LocSelInputState *locSelInputState; // [esp+44h] [ebp-40Ch]
    char cmd[1028]; // [esp+48h] [ebp-408h] BYREF

    keys = playerKeys[localClientNum].keys;
    keys[key].down = down;
    if (down)
    {
        if (++keys[key].repeats == 1)
            ++playerKeys[localClientNum].anyKeyDown;
    }
    else
    {
        keys[key].repeats = 0;
        if (--playerKeys[localClientNum].anyKeyDown < 0)
            playerKeys[localClientNum].anyKeyDown = 0;
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (CL_IsConsoleKey(key) || (clientUIActives[0].keyCatchers & 3) != 0)
    {
        if (DevGui_IsActive())
            DevGui_Toggle();
    }
    else
    {
        kb = keys[key].binding;
        if (kb && !I_stricmp(kb, "devgui") && down && keys[key].repeats == 1)
        {
            Cbuf_AddText(localClientNum, keys[key].binding);
            return;
        }
        if (DevGui_IsActive())
        {
            if (down)
            {
                if (keys[key].repeats == 1)
                    DevGui_KeyPressed(key);
            }
            return;
        }
    }
    if (!down || keys[key].repeats <= 1)
    {
    LABEL_38:
        if ((clientUIActives[0].keyCatchers & 2) == 0 || (clientUIActives[0].keyCatchers & 1) != 0)
        {
            if (!con_restricted->current.enabled || (clientUIActives[0].keyCatchers & 1) != 0)
            {
                if (CL_IsConsoleKey(key))
                {
                    if (!down)
                        return;
                    if ((clientUIActives[0].keyCatchers & 1) == 0
                        && !com_sv_running->current.enabled
                        && sv_disableClientConsole->current.enabled)
                    {
                        return;
                    }
                    if (keys[K_SHIFT].down)
                    {
                        if (!Con_IsActive(localClientNum))
                            Con_ToggleConsole();
                        Con_ToggleConsoleOutput();
                        return;
                    }
                LABEL_45:
                    Con_ToggleConsole();
                    return;
                }
            }
            else
            {
                if (key == K_HOME && down && keys[K_BACKSPACE].down)
                    goto LABEL_45;
                if (CL_IsConsoleKey(key))
                    return;
            }
        }
        locSelInputState = &playerKeys[localClientNum].locSelInputState;
        if ((clientUIActives[0].keyCatchers & 8) != 0 && down > 0)
        {
            if (key == K_ESCAPE)
            {
                *locSelInputState = LOC_SEL_INPUT_CANCEL;
            }
            else if (keys[key].binding && !strcmp(keys[key].binding, "+attack"))
            {
                *locSelInputState = LOC_SEL_INPUT_CONFIRM;
            }
            return;
        }
        *locSelInputState = LOC_SEL_INPUT_NONE;
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
                1112,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        clcState = clientUIActives[0].connectionState;
        if (down)
        {
            v6 = key == K_MOUSE1 || key < K_ASCII_FIRST;
            if (v6
                && (CL_GetLocalClientConnection(localClientNum)->demoplaying || clcState == CA_CINEMATIC || clcState == CA_LOGO)
                && !clientUIActives[0].keyCatchers)
            {
                Dvar_SetString(nextdemo, (char*)"");
                key = K_ESCAPE;
            }
        }
        if (key == K_ESCAPE && down)
        {
            if ((clientUIActives[0].keyCatchers & 2) != 0)
            {
                if ((clientUIActives[0].keyCatchers & 1) != 0)
                    Con_ToggleConsole();
                return;
            }
            if ((clientUIActives[0].keyCatchers & 0x20) == 0)
            {
                if ((clientUIActives[0].keyCatchers & 1) != 0)
                    Con_CancelAutoComplete();
                if ((clientUIActives[0].keyCatchers & 0x10) != 0)
                {
                    UI_KeyEvent(localClientNum, K_ESCAPE, down);
                }
                else
                {
                    switch (clcState)
                    {
                    case CA_CINEMATIC:
                    case CA_LOGO:
                        CL_StopLogoOrCinematic(localClientNum);
                        break;
                    case CA_CONNECTING:
                    case CA_CHALLENGING:
                    case CA_CONNECTED:
                        return;
                    case CA_ACTIVE:
                        if (CL_GetLocalClientConnection(localClientNum)->demoplaying
                            || cl_waitingOnServerToLoadMap[localClientNum])
                        {
                            UI_SetActiveMenu(localClientNum, UIMENU_MAIN);
                        }
                        else
                        {
                            UI_SetActiveMenu(localClientNum, UIMENU_INGAME);
                        }
                        break;
                    default:
                        if (cls.uiStarted)
                            UI_SetActiveMenu(localClientNum, UIMENU_MAIN);
                        break;
                    }
                }
                return;
            }
            goto LABEL_91;
        }
        if (clientUIActives[0].cgameInitialized
            && CG_IsScoreboardDisplayed(localClientNum)
            && down
            && Scoreboard_HandleInput(localClientNum, key))
        {
            return;
        }
        if (!down)
        {
            kba = keys[key].binding;
            if (kba && *kba == '+')
            {
                Com_sprintf(cmd, 0x400u, "-%s %i %i\n", kba + 1, key, time);
                Cbuf_AddText(localClientNum, cmd);
            }
            if ((clientUIActives[0].keyCatchers & 0x10) != 0 && cls.uiStarted)
                UI_KeyEvent(localClientNum, key, 0);
            return;
        }
        if ((clientUIActives[0].keyCatchers & 1) == 0)
        {
            if ((clientUIActives[0].keyCatchers & 2) != 0)
            {
                Scr_KeyEvent(key);
                return;
            }
            if ((clientUIActives[0].keyCatchers & 0x10) != 0 && !CL_MouseInputShouldBypassMenus(localClientNum, key))
            {
                if (cls.uiStarted)
                    UI_KeyEvent(localClientNum, key, down);
                return;
            }
            if ((clientUIActives[0].keyCatchers & 0x20) != 0)
            {
            LABEL_91:
                Message_Key(localClientNum, key);
                return;
            }
            if (clcState)
            {
                kbb = keys[key].binding;
                if (kbb)
                {
                    if (*kbb == '+')
                    {
                        Com_sprintf(cmd, 0x400u, "%s %i %i\n", kbb, key, time);
                        Cbuf_AddText(localClientNum, cmd);
                    }
                    else
                    {
                        Cbuf_AddText(localClientNum, kbb);
                        Cbuf_AddText(localClientNum, "\n");
                    }
                }
                else if (key >= K_AUX1)
                {
                    v4 = Key_KeynumToString(key, 0);
                    Com_Printf(CON_CHANNEL_CLIENT, "%s is unbound, use controls menu to set.\n", v4);
                }
                return;
            }
        }
        Console_Key(localClientNum, key);
        return;
    }
    if ((clientUIActives[0].keyCatchers & 0x21) != 0)
    {
    LABEL_34:
        if (key == '`' || key == '~' || key == K_ESCAPE)
            return;
        goto LABEL_38;
    }
    if ((clientUIActives[0].keyCatchers & 0x12) != 0)
    {
        if ((clientUIActives[0].keyCatchers & 2) != 0)
        {
            switch (key)
            {
            case K_UPARROW:
            case K_DOWNARROW:
            case K_LEFTARROW:
            case K_RIGHTARROW:
            case K_PGDN:
            case K_PGUP:
            case K_F3:
            case K_F10:
                goto LABEL_34;
            default:
                return;
            }
        }
        else
        {
            if ((clientUIActives[0].keyCatchers & 0x10) == 0)
                MyAssertHandler(".\\client\\cl_keys.cpp", 1942, 0, "%s", "cl->keyCatchers & KEYCATCH_UI");
            switch (key)
            {
            case K_UPARROW:
            case K_DOWNARROW:
            case K_PGDN:
            case K_PGUP:
                goto LABEL_34;
            default:
                return;
            }
        }
    }
}
#elif KISAK_SP
void __cdecl CL_KeyEvent(int32_t localClientNum, int32_t key, int32_t down, uint32_t time)
{
    PROF_SCOPED("CL_KeyEvent");

    const char *v4; // eax
    KeyState *keys; // [esp+34h] [ebp-41Ch]
    const char *kb; // [esp+38h] [ebp-418h]
    const char *kba; // [esp+38h] [ebp-418h]
    const char *kbb; // [esp+38h] [ebp-418h]
    connstate_t clcState; // [esp+40h] [ebp-410h]
    LocSelInputState *locSelInputState; // [esp+44h] [ebp-40Ch]
    char cmd[1028]; // [esp+48h] [ebp-408h] BYREF

    keys = playerKeys[localClientNum].keys;
    keys[key].down = down;
    if (down)
    {
        if (++keys[key].repeats == 1)
            ++playerKeys[localClientNum].anyKeyDown;
    }
    else
    {
        keys[key].repeats = 0;
        if (--playerKeys[localClientNum].anyKeyDown < 0)
            playerKeys[localClientNum].anyKeyDown = 0;
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (CL_IsConsoleKey(key) || (clientUIActives[0].keyCatchers & 3) != 0)
    {
        if (DevGui_IsActive())
            DevGui_Toggle();
    }
    else
    {
        kb = keys[key].binding;
        if (kb && !I_stricmp(kb, "devgui") && down && keys[key].repeats == 1)
        {
            Cbuf_AddText(localClientNum, keys[key].binding);
            return;
        }
        if (DevGui_IsActive())
        {
            if (down)
            {
                if (keys[key].repeats == 1)
                    DevGui_KeyPressed(key);
            }
            return;
        }
    }
    if (!down || keys[key].repeats <= 1)
    {
    LABEL_38:
        if ((clientUIActives[0].keyCatchers & 2) == 0 || (clientUIActives[0].keyCatchers & 1) != 0)
        {
            if (!con_restricted->current.enabled || (clientUIActives[0].keyCatchers & 1) != 0)
            {
                if (CL_IsConsoleKey(key))
                {
                    if (!down)
                        return;
                    //if ((clientUIActives[0].keyCatchers & 1) == 0
                    //    && !com_sv_running->current.enabled)
                    //{
                    //    return;
                    //}
                    if (keys[K_SHIFT].down)
                    {
                        if (!Con_IsActive(localClientNum))
                            Con_ToggleConsole();
                        Con_ToggleConsoleOutput();
                        return;
                    }
                LABEL_45:
                    Con_ToggleConsole();
                    return;
                }
            }
            else
            {
                if (key == K_HOME && down && keys[K_BACKSPACE].down)
                    goto LABEL_45;
                if (CL_IsConsoleKey(key))
                    return;
            }
        }
        locSelInputState = &playerKeys[localClientNum].locSelInputState;
        if ((clientUIActives[0].keyCatchers & 8) != 0 && down > 0)
        {
            if (key == K_ESCAPE)
            {
                *locSelInputState = LOC_SEL_INPUT_CANCEL;
            }
            else if (keys[key].binding && !strcmp(keys[key].binding, "+attack"))
            {
                *locSelInputState = LOC_SEL_INPUT_CONFIRM;
            }
            return;
        }
        *locSelInputState = LOC_SEL_INPUT_NONE;
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
                1112,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        clcState = CL_GetLocalClientConnectionState(localClientNum);
        if (down)
        {
            if ((key == K_MOUSE1 || key < K_ASCII_FIRST)
                && (cls.demoplaying || clcState == CA_CINEMATIC || clcState == CA_LOGO)
                && !clientUIActives[0].keyCatchers)
            {
                Dvar_SetString(nextdemo, (char *)"");
                key = K_ESCAPE;
            }
        }
        if (key == K_ESCAPE && down)
        {
            if ((clientUIActives[0].keyCatchers & 2) != 0)
            {
                if ((clientUIActives[0].keyCatchers & 1) != 0)
                    Con_ToggleConsole();
                return;
            }
            if ((clientUIActives[0].keyCatchers & 0x20) == 0)
            {
                if ((clientUIActives[0].keyCatchers & 1) != 0)
                    Con_CancelAutoComplete();
                if ((clientUIActives[0].keyCatchers & 0x10) != 0)
                {
                    UI_KeyEvent(localClientNum, K_ESCAPE, down);
                }
                else
                {
                    switch (clcState)
                    {
                    case CA_CINEMATIC:
                    case CA_LOGO:
                        CL_StopLogoOrCinematic(localClientNum);
                        break;
                    case CA_ACTIVE:
                        {
                            UI_SetActiveMenu(localClientNum, UIMENU_INGAME);
                        }
                        break;
                    default:
                        if (cls.uiStarted)
                            UI_SetActiveMenu(localClientNum, UIMENU_MAIN);
                        break;
                    }
                }
                return;
            }
            goto LABEL_91;
        }

        if (!down)
        {
            kba = keys[key].binding;
            if (kba && *kba == '+')
            {
                Com_sprintf(cmd, 0x400u, "-%s %i %i\n", kba + 1, key, time);
                Cbuf_AddText(localClientNum, cmd);
            }
            if ((clientUIActives[0].keyCatchers & 0x10) != 0 && cls.uiStarted)
                UI_KeyEvent(localClientNum, key, 0);
            return;
        }
        if ((clientUIActives[0].keyCatchers & 1) == 0)
        {
            if ((clientUIActives[0].keyCatchers & 2) != 0)
            {
                Scr_KeyEvent(key);
                return;
            }
            if ((clientUIActives[0].keyCatchers & 0x10) != 0 && !CL_MouseInputShouldBypassMenus(localClientNum, key))
            {
                if (cls.uiStarted)
                    UI_KeyEvent(localClientNum, key, down);
                return;
            }
            if ((clientUIActives[0].keyCatchers & 0x20) != 0)
            {
            LABEL_91:
                Message_Key(localClientNum, key);
                return;
            }
            if (clcState)
            {
                kbb = keys[key].binding;
                if (kbb)
                {
                    if (*kbb == '+')
                    {
                        Com_sprintf(cmd, 0x400u, "%s %i %i\n", kbb, key, time);
                        Cbuf_AddText(localClientNum, cmd);
                    }
                    else
                    {
                        Cbuf_AddText(localClientNum, kbb);
                        Cbuf_AddText(localClientNum, "\n");
                    }
                }
                else if (key >= K_AUX1)
                {
                    v4 = Key_KeynumToString(key, 0);
                    Com_Printf(CON_CHANNEL_CLIENT, "%s is unbound, use controls menu to set.\n", v4);
                }
                return;
            }
        }
        Console_Key(localClientNum, key);
        return;
    }
    if ((clientUIActives[0].keyCatchers & 0x21) != 0)
    {
    LABEL_34:
        if (key == '`' || key == '~' || key == K_ESCAPE)
            return;
        goto LABEL_38;
    }
    if ((clientUIActives[0].keyCatchers & 0x12) != 0)
    {
        if ((clientUIActives[0].keyCatchers & 2) != 0)
        {
            switch (key)
            {
            case K_UPARROW:
            case K_DOWNARROW:
            case K_LEFTARROW:
            case K_RIGHTARROW:
            case K_PGDN:
            case K_PGUP:
            case K_F3:
            case K_F10:
                goto LABEL_34;
            default:
                return;
            }
        }
        else
        {
            if ((clientUIActives[0].keyCatchers & 0x10) == 0)
                MyAssertHandler(".\\client\\cl_keys.cpp", 1942, 0, "%s", "cl->keyCatchers & KEYCATCH_UI");
            switch (key)
            {
            case K_UPARROW:
            case K_DOWNARROW:
            case K_PGDN:
            case K_PGUP:
                goto LABEL_34;
            default:
                return;
            }
        }
    }
}
#endif

void __cdecl Message_Key(int32_t localClientNum, int32_t key)
{
    char buffer[1028]; // [esp+4h] [ebp-410h] BYREF
    field_t *chatField; // [esp+40Ch] [ebp-8h]
    connstate_t clcState; // [esp+410h] [ebp-4h]

    chatField = &playerKeys[localClientNum].chatField;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (key == K_ESCAPE)
    {
        clientUIActives[0].keyCatchers &= ~0x20u;
        Field_Clear(chatField);
    }
    else if (key == K_ENTER || key == K_KP_ENTER)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
                1112,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        clcState = clientUIActives[0].connectionState;
        if (chatField->buffer[0] && clcState == CA_ACTIVE)
        {
            if (playerKeys[localClientNum].chat_team)
                Com_sprintf(buffer, 0x400u, "say_team %s", chatField->buffer);
            else
                Com_sprintf(buffer, 0x400u, "say %s", chatField->buffer);
            CL_AddReliableCommand(localClientNum, buffer);
        }
        clientUIActives[0].keyCatchers &= ~0x20u;
        Field_Clear(chatField);
    }
    else
    {
        Field_KeyDownEvent(localClientNum, &scrPlaceView[localClientNum], chatField, key);
    }
}

bool __cdecl CL_MouseInputShouldBypassMenus(int32_t localClientNum, int32_t key)
{
#ifdef KISAK_SP
    return false; //KISAKTODO: Is this proper for PC?
#elif KISAK_MP
    if (UI_GetActiveMenu(localClientNum) == 10)
        return 1;
    if (!cl_bypassMouseInput || !cl_bypassMouseInput->current.enabled)
        return 0;
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3)
        return 1;
    return cls.uiStarted && !UI_CheckExecKey(localClientNum, key);
#endif
}

void __cdecl CL_CharEvent(int32_t localClientNum, int32_t key)
{
    PROF_SCOPED("CL_CharEvent");

    if (DevGui_IsActive() || key == '`' || key == '~')
        return;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((clientUIActives[0].keyCatchers & 1) != 0)
    {
        if (key == '\b' && Con_CancelAutoComplete())
            return;
    LABEL_18:
        CL_ConsoleCharEvent(localClientNum, key);
        return;
    }
    if ((clientUIActives[0].keyCatchers & 0x20) != 0)
    {
        Field_CharEvent(localClientNum, &scrPlaceView[localClientNum], &playerKeys[localClientNum].chatField, key);
        return;
    }
    if ((clientUIActives[0].keyCatchers & 0x10) != 0)
    {
        UI_KeyEvent(localClientNum, key | K_CHAR_FLAG, 1);
        return;
    }
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState == CA_DISCONNECTED)
        goto LABEL_18;
}

void __cdecl CL_ConsoleCharEvent(int32_t localClientNum, int32_t key)
{
    if (Field_CharEvent(localClientNum, &scrPlaceFull, &g_consoleField, key))
        Con_AllowAutoCompleteCycling(1);
}

void __cdecl Key_ClearStates(int32_t localClientNum)
{
    int32_t keynum; // [esp+0h] [ebp-8h]
    KeyState *keys; // [esp+4h] [ebp-4h]

    playerKeys[localClientNum].anyKeyDown = 0;
    keys = playerKeys[localClientNum].keys;
    for (keynum = 0; keynum < 256; ++keynum)
    {
        if (keys[keynum].down)
            CL_KeyEvent(localClientNum, keynum, 0, 0);
        keys[keynum].down = 0;
        keys[keynum].repeats = 0;
    }
}

int32_t __cdecl CL_GetKeyBinding(int32_t localClientNum, const char *command, char (*keyNames)[128])
{
    return CL_GetKeyBindingInternal(localClientNum, command, keyNames);
}

int32_t __cdecl CL_GetKeyBindingInternal(int32_t localClientNum, const char *command, char (*keyNames)[128])
{
    int32_t keys[2]; // [esp+0h] [ebp-Ch] BYREF
    int32_t bindCount; // [esp+8h] [ebp-4h]

    (*keyNames)[128] = 0;
    bindCount = Key_GetCommandAssignmentInternal(localClientNum, command, keys);
    if ((uint32_t)bindCount > 2)
        MyAssertHandler(".\\client\\cl_keys.cpp", 2347, 0, "bindCount not in [0, 2]\n\t%i not in [%i, %i]", bindCount, 0, 2);
    if (bindCount)
    {
        Key_KeynumToStringBuf(keys[0], (char *)keyNames, 128);
        if (bindCount == 2)
            Key_KeynumToStringBuf(keys[1], &(*keyNames)[128], 128);
    }
    else
    {
        strcpy((char *)keyNames, "KEY_UNBOUND");
    }
    return bindCount;
}

void __cdecl Key_Shutdown()
{
    int32_t client; // [esp+0h] [ebp-8h]
    int32_t keynum; // [esp+4h] [ebp-4h]

    for (client = 0; client < 1; ++client)
    {
        for (keynum = 0; keynum < 256; ++keynum)
        {
            if (playerKeys[client].keys[keynum].binding)
            {
                FreeString(playerKeys[client].keys[keynum].binding);
                playerKeys[client].keys[keynum].binding = 0;
            }
        }
    }
}

bool __cdecl Key_IsCatcherActive(int32_t localClientNum, int32_t mask)
{
    iassert(localClientNum == 0);
    return (mask & clientUIActives[0].keyCatchers) != 0;
}

void __cdecl Key_AddCatcher(int32_t localClientNum, int32_t orMask)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clientUIActives[0].keyCatchers |= orMask;
}

void __cdecl Key_RemoveCatcher(int32_t localClientNum, int32_t andMask)
{
    if ((andMask & (andMask - 1)) == 0)
        MyAssertHandler(".\\client\\cl_keys.cpp", 2448, 0, "%s", "!IsPowerOf2( andMask )");
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    clientUIActives[0].keyCatchers &= andMask;
    if ((clientUIActives[0].keyCatchers & 0x10) == 0)
        clientUIActives[0].displayHUDWithKeycatchUI = 0;
}

void __cdecl Key_SetCatcher(int32_t localClientNum, int32_t catcher)
{
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\client\\../client_mp/client_mp.h",
            1063,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if ((clientUIActives[0].keyCatchers & 1) != 0)
        clientUIActives[0].keyCatchers = catcher | 1;
    else
        clientUIActives[0].keyCatchers = catcher;
    if ((clientUIActives[0].keyCatchers & 0x10) == 0)
        clientUIActives[0].displayHUDWithKeycatchUI = 0;
}


int CL_IsKeyPressed(int localClientNum, const char *keyName)
{
    int keynum; // r30

    keynum = Key_StringToKeynum(keyName);

    if (keynum < 0)
        return 0;

    iassert(localClientNum >= 0 && localClientNum < 1);
    
    return playerKeys[localClientNum].keys[keynum].down;
}

bool Key_IsValidGamePadChar(const char key)
{
    return key >= 1 && key <= 6 || key >= 14 && key <= 23 || key >= 28 && key <= 31;
}

const char *CL_GetCommandFromKey(const char *keyName)
{
    int v1; // r3

    v1 = Key_StringToKeynum(keyName);
    if (v1 >= 0)
        return playerKeys[0].keys[v1].binding;
    else
        return 0;
}
