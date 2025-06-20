#include "client.h"

#include <client_mp/client_mp.h>

#include <gfx_d3d/r_debug.h>

void __cdecl CL_AddDebugString(
    const float *xyz,
    const float *color,
    float scale,
    const  char *text,
    int fromServer,
    int duration)
{
    clientDebugStringInfo_t *info; // [esp+10h] [ebp-4h]

    if (cls.rendererStarted && CreateDebugStringsIfNeeded())
    {
        if (fromServer)
            info = &cls.debug.svStrings;
        else
            info = &cls.debug.clStrings;
        AddDebugStringInternal(xyz, color, scale, text, duration, info);
        if (fromServer)
            cls.debug.fromServer = 1;
    }
}

bool __cdecl CreateDebugStringsIfNeeded()
{
    if (cls.debug.clStrings.strings)
    {
        if (!cls.debug.svStrings.strings)
            MyAssertHandler(".\\client\\cl_debugdata.cpp", 26, 0, "%s", "cls.debug.svStrings.strings");
        return cls.debug.clStrings.durations
            && cls.debug.svStrings.strings
            && cls.debug.svStrings.durations
            && cls.debug.svStringsBuffer.strings;
    }
    else
    {
        if (cls.debug.svStrings.strings)
            MyAssertHandler(".\\client\\cl_debugdata.cpp", 30, 0, "%s", "NULL == cls.debug.svStrings.strings");
        if (cls.debug.svStringsBuffer.strings)
            MyAssertHandler(".\\client\\cl_debugdata.cpp", 31, 0, "%s", "NULL == cls.debug.svStringsBuffer.strings");
        cls.debug.clStrings.max = 1024;
        cls.debug.svStrings.max = 1024;
        cls.debug.svStringsBuffer.max = 1024;
        R_DebugAlloc((void **)&cls.debug.clStrings.strings, 0x20000, "Client Debug Strings");
        R_DebugAlloc((void **)&cls.debug.clStrings.durations, 4 * cls.debug.clStrings.max, "Client Debug Strings");
        cls.debug.clStrings.num = 0;
        R_DebugAlloc((void **)&cls.debug.svStrings.strings, cls.debug.svStrings.max << 7, "Client Debug Strings");
        R_DebugAlloc((void **)&cls.debug.svStrings.durations, 4 * cls.debug.svStrings.max, "Client Debug Strings");
        cls.debug.svStrings.num = 0;
        R_DebugAlloc((void **)&cls.debug.svStringsBuffer.strings, cls.debug.svStrings.max << 7, "Client Debug Strings");
        cls.debug.svStringsBuffer.num = 0;
        return cls.debug.clStrings.strings
            && cls.debug.clStrings.durations
            && cls.debug.svStrings.strings
            && cls.debug.svStrings.durations
            && cls.debug.svStringsBuffer.strings;
    }
}

void __cdecl AddDebugStringInternal(
    const float *xyz,
    const float *color,
    float scale,
    const char *text,
    int duration,
    clientDebugStringInfo_t *info)
{
    trDebugString_t *string; // [esp+4h] [ebp-4h]

    if (info->num + 1 <= info->max)
    {
        string = &info->strings[info->num];
        string->xyz[0] = *xyz;
        string->xyz[1] = xyz[1];
        string->xyz[2] = xyz[2];
        string->color[0] = *color;
        string->color[1] = color[1];
        string->color[2] = color[2];
        string->color[3] = color[3];
        string->scale = scale;
        strncpy((unsigned __int8 *)string->text, (unsigned __int8 *)text, 0x5Fu);
        string->text[95] = 0;
        info->durations[info->num++] = duration;
    }
}

void __cdecl CL_AddDebugLine(
    const float *start,
    const float *end,
    const float *color,
    int depthTest,
    int duration,
    int fromServer)
{
    if (cls.rendererStarted && CreateDebugLinesIfNeeded())
    {
        if (fromServer)
            AddDebugLineInternal(start, end, color, depthTest, duration, &cls.debug.svLines);
        else
            AddDebugLineInternal(start, end, color, depthTest, duration, &cls.debug.clLines);
        if (fromServer)
            cls.debug.fromServer = 1;
    }
}

bool __cdecl CreateDebugLinesIfNeeded()
{
    if (cls.debug.clLines.lines)
    {
        return cls.debug.clLines.durations
            && cls.debug.svLines.lines
            && cls.debug.svLines.durations
            && cls.debug.svLinesBuffer.lines;
    }
    else
    {
        if (cls.debug.svLines.lines)
            MyAssertHandler(".\\client\\cl_debugdata.cpp", 101, 0, "%s", "NULL == cls.debug.svLines.lines");
        if (cls.debug.svLinesBuffer.lines)
            MyAssertHandler(".\\client\\cl_debugdata.cpp", 102, 0, "%s", "NULL == cls.debug.svLinesBuffer.lines");
        cls.debug.clLines.max = 2048;
        cls.debug.svLines.max = 2048;
        cls.debug.svLinesBuffer.max = 2048;
        R_DebugAlloc((void **)&cls.debug.clLines.lines, 90112, "Client Debug Lines");
        R_DebugAlloc((void **)&cls.debug.clLines.durations, 0x2000, "Client Debug Lines");
        cls.debug.clLines.num = 0;
        R_DebugAlloc((void **)&cls.debug.svLines.lines, 90112, "Client Debug Lines");
        R_DebugAlloc((void **)&cls.debug.svLines.durations, 0x2000, "Client Debug Lines");
        cls.debug.svLines.num = 0;
        R_DebugAlloc((void **)&cls.debug.svLinesBuffer.lines, 90112, "Client Debug Lines");
        cls.debug.svLinesBuffer.num = 0;
        return cls.debug.clLines.lines
            && cls.debug.clLines.durations
            && cls.debug.svLines.lines
            && cls.debug.svLines.durations
            && cls.debug.svLinesBuffer.lines;
    }
}

void __cdecl AddDebugLineInternal(
    const float *start,
    const float *end,
    const float *color,
    int depthTest,
    int duration,
    clientDebugLineInfo_t *info)
{
    trDebugLine_t *line; // [esp+8h] [ebp-4h]

    if (info->num + 1 <= info->max)
    {
        line = &info->lines[info->num];
        line->start[0] = *start;
        line->start[1] = start[1];
        line->start[2] = start[2];
        line->end[0] = *end;
        line->end[1] = end[1];
        line->end[2] = end[2];
        line->color[0] = *color;
        line->color[1] = color[1];
        line->color[2] = color[2];
        line->color[3] = color[3];
        line->depthTest = depthTest;
        info->durations[info->num++] = duration;
    }
}

void __cdecl CL_AddDebugStarWithText(
    const float *point,
    const float *starColor,
    const float *textColor,
    const char *string,
    float fontsize,
    int duration,
    int fromServer)
{
    float lineEnd[3]; // [esp+10h] [ebp-1Ch] BYREF
    float lineStart[3]; // [esp+1Ch] [ebp-10h] BYREF
    float starsize; // [esp+28h] [ebp-4h]

    starsize = 5.0 * fontsize;
    lineStart[0] = *point;
    lineStart[1] = point[1];
    lineStart[2] = point[2];
    lineEnd[0] = *point;
    lineEnd[1] = point[1];
    lineEnd[2] = point[2];
    lineStart[0] = lineStart[0] + starsize;
    lineEnd[0] = lineEnd[0] - starsize;
    CL_AddDebugLine(lineStart, lineEnd, starColor, 0, duration, fromServer);
    lineStart[0] = lineStart[0] - starsize;
    lineEnd[0] = lineEnd[0] + starsize;
    lineStart[1] = lineStart[1] + starsize;
    lineEnd[1] = lineEnd[1] - starsize;
    CL_AddDebugLine(lineStart, lineEnd, starColor, 0, duration, fromServer);
    lineStart[1] = lineStart[1] - starsize;
    lineEnd[1] = lineEnd[1] + starsize;
    lineStart[2] = lineStart[2] + starsize;
    lineEnd[2] = lineEnd[2] - starsize;
    CL_AddDebugLine(lineStart, lineEnd, starColor, 0, duration, fromServer);
    if (string)
    {
        if (*string)
            CL_AddDebugString(point, textColor, fontsize, string, fromServer, duration);
    }
}

void __cdecl CL_AddDebugStar(const float *point, const float *color, int duration, int fromServer)
{
    const float black[] = { 0, 0, 0, 0 };
    CL_AddDebugStarWithText(
        point,
        color,
        black,
        0,
        1.0,
        duration,
        fromServer);
}

void __cdecl CL_FlushDebugClientData()
{
    if (cls.rendererStarted)
    {
        FlushDebugStrings(&cls.debug.clStrings, 0);
        FlushDebugLines(&cls.debug.clLines, 0);
    }
}

void __cdecl FlushDebugStrings(clientDebugStringInfo_t *info, int fromServer)
{
    int idx; // [esp+8h] [ebp-4h]

    if (info->strings)
    {
        idx = 0;
        while (idx < info->num)
        {
            if (--info->durations[idx] > 0)
            {
                if (fromServer)
                    cls.debug.fromServer = 1;
                ++idx;
            }
            else
            {
                info->durations[idx] = info->durations[--info->num];
                memcpy(&info->strings[idx], &info->strings[info->num], sizeof(info->strings[idx]));
            }
        }
    }
}

void __cdecl FlushDebugLines(clientDebugLineInfo_t *info, int fromServer)
{
    int idx; // [esp+8h] [ebp-4h]

    if (info->lines)
    {
        idx = 0;
        while (idx < info->num)
        {
            if (--info->durations[idx] > 0)
            {
                if (fromServer)
                    cls.debug.fromServer = 1;
                ++idx;
            }
            else
            {
                info->durations[idx] = info->durations[--info->num];
                memcpy(&info->lines[idx], &info->lines[info->num], sizeof(info->lines[idx]));
            }
        }
    }
}

clientDebugLineInfo_t *clLine = &cls.debug.clLines;
clientDebugStringInfo_t *clStr = &cls.debug.clStrings;

clientDebugLineInfo_t *svLine = &cls.debug.svLines;
clientDebugStringInfo_t *svStr = &cls.debug.svStrings;

clientDebugLineInfo_t *svLineBuff = &cls.debug.svLinesBuffer;
clientDebugStringInfo_t *svStrBuff = &cls.debug.svStringsBuffer;

void __cdecl CL_UpdateDebugClientData()
{
    if (cls.rendererStarted)
    {
        R_CopyDebugStrings(clStr->strings, clStr->num, svStrBuff->strings, svStrBuff->num, svStrBuff->max + clStr->max);
        R_CopyDebugLines(clLine->lines, clLine->num, svLineBuff->lines, svLineBuff->num, svLineBuff->max + clLine->max);
    }
}

void __cdecl CL_FlushDebugServerData()
{
    if (cls.rendererStarted)
    {
        FlushDebugStrings(&cls.debug.svStrings, 1);
        FlushDebugLines(&cls.debug.svLines, 1);
    }
}

void __cdecl CL_UpdateDebugServerData()
{
    int copySize; // [esp+0h] [ebp-4h]
    int copySizea; // [esp+0h] [ebp-4h]

    if (cls.rendererStarted)
    {
        copySize = svStr->num << 7;
        svStrBuff->num = svStr->num;
        memcpy((unsigned __int8 *)svStrBuff->strings, (unsigned __int8 *)svStr->strings, copySize);
        copySizea = 44 * svLine->num;
        svLineBuff->num = svLine->num;
        memcpy((unsigned __int8 *)svLineBuff->lines, (unsigned __int8 *)svLine->lines, copySizea);
    }
}

void __cdecl CL_ShutdownDebugData()
{
    R_DebugFree((void **)&cls.debug.clLines.lines);
    R_DebugFree((void **)&cls.debug.clLines.durations);
    R_DebugFree((void **)&cls.debug.svLines.lines);
    R_DebugFree((void **)&cls.debug.svLines.durations);
    R_DebugFree((void **)&cls.debug.svLinesBuffer.lines);
    R_DebugFree((void **)&cls.debug.svLinesBuffer.durations);
    R_DebugFree((void **)&cls.debug.clStrings.strings);
    R_DebugFree((void **)&cls.debug.clStrings.durations);
    R_DebugFree((void **)&cls.debug.svStrings.strings);
    R_DebugFree((void **)&cls.debug.svStrings.durations);
    R_DebugFree((void **)&cls.debug.svStringsBuffer.strings);
    R_DebugFree((void **)&cls.debug.svStringsBuffer.durations);
    memset((unsigned __int8 *)&cls.debug, 0, sizeof(cls.debug));
    R_ShutdownDebug();
    KISAK_NULLSUB();
}

