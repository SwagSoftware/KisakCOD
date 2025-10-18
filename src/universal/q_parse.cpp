#include "q_parse.h"
#include <math.h>
#include <stdlib.h>
#include <qcommon/mem_track.h>
#include <qcommon/qcommon.h>
#include <qcommon/threads.h>

ParseThreadInfo g_parse[4]; // LWSS: increase from 3 to 4 for SP's SERVER thread

void __cdecl TRACK_q_parse()
{
    //track_static_alloc_internal(g_parse, 53796, "g_parse", 10);
}

void __cdecl Com_InitParse()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < 4; ++i)
        Com_InitParseInfo(g_parse[i].parseInfo);
}

void __cdecl Com_InitParseInfo(parseInfo_t *pi)
{
    pi->lines = 1;
    pi->ungetToken = 0;
    pi->spaceDelimited = 1;
    pi->keepStringQuotes = 0;
    pi->csv = 0;
    pi->negativeNumbers = 0;
    pi->errorPrefix = "";
    pi->warningPrefix = "";
    pi->backup_lines = 0;
    pi->backup_text = 0;
}

void __cdecl Com_BeginParseSession(const char *filename)
{
    parseInfo_t *pi; // [esp+0h] [ebp-Ch]
    ParseThreadInfo *parse; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    if (parse->parseInfoNum == 15)
    {
        Com_Printf(23, "Already parsing:\n");
        for (i = 0; i < parse->parseInfoNum; ++i)
            Com_Printf(23, "%i. %s\n", i, parse->parseInfo[i].parseFile);
        Com_Error(ERR_FATAL, "Com_BeginParseSession: session overflow trying to parse %s", filename);
    }
    pi = &parse->parseInfo[++parse->parseInfoNum];
    Com_InitParseInfo(pi);
    pi->parseFile = filename;
}

ParseThreadInfo *__cdecl Com_GetParseThreadInfo()
{
    if (Sys_IsMainThread())
        return &g_parse[0];
    if (Sys_IsRenderThread())
        return &g_parse[1];
    if (Sys_IsDatabaseThread())
        return &g_parse[2];
#ifdef KISAK_SP
    if (Sys_IsServerThread())
    {
        return &g_parse[3];
    }
#endif
    if (!alwaysfails)
        MyAssertHandler(".\\universal\\q_parse.cpp", 117, 0, "No parse context for current thread.");
    return 0;
}

void __cdecl Com_EndParseSession()
{
    ParseThreadInfo *parse; // [esp+0h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    if (!parse->parseInfoNum)
        Com_Error(ERR_FATAL, "Com_EndParseSession: session underflow");
    --parse->parseInfoNum;
}

void __cdecl Com_ResetParseSessions()
{
    Com_GetParseThreadInfo()->parseInfoNum = 0;
}

void __cdecl Com_SetSpaceDelimited(int spaceDelimited)
{
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    parse->parseInfo[parse->parseInfoNum].spaceDelimited = spaceDelimited != 0;
}

void __cdecl Com_SetKeepStringQuotes(int keepStringQuotes)
{
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    parse->parseInfo[parse->parseInfoNum].keepStringQuotes = keepStringQuotes != 0;
}

void __cdecl Com_SetCSV(int csv)
{
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    parse->parseInfo[parse->parseInfoNum].csv = csv != 0;
}

void __cdecl Com_SetParseNegativeNumbers(int negativeNumbers)
{
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    parse->parseInfo[parse->parseInfoNum].negativeNumbers = negativeNumbers != 0;
}

int __cdecl Com_GetCurrentParseLine()
{
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    return parse->parseInfo[parse->parseInfoNum].lines;
}

void __cdecl Com_SetScriptWarningPrefix(const char *prefix)
{
    parseInfo_t *pi; // [esp+0h] [ebp-8h]
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    if (!prefix)
        MyAssertHandler(".\\universal\\q_parse.cpp", 302, 0, "%s", "prefix");
    if (parse->parseInfoNum <= 0)
        MyAssertHandler(
            ".\\universal\\q_parse.cpp",
            303,
            0,
            "%s\n\t(parse->parseInfoNum) = %i",
            "(parse->parseInfoNum > 0)",
            parse->parseInfoNum);
    pi->warningPrefix = prefix;
}

void Com_ScriptErrorDrop(const char *msg, ...)
{
    char string[4096]; // [esp+0h] [ebp-1010h] BYREF
    char *ap; // [esp+1004h] [ebp-Ch]
    parseInfo_t *v3; // [esp+1008h] [ebp-8h]
    ParseThreadInfo *ParseThreadInfo; // [esp+100Ch] [ebp-4h]
    va_list va; // [esp+101Ch] [ebp+Ch] BYREF

    va_start(va, msg);
    ParseThreadInfo = Com_GetParseThreadInfo();
    v3 = &ParseThreadInfo->parseInfo[ParseThreadInfo->parseInfoNum];
    va_copy(ap, va);
    _vsnprintf(string, 0x1000u, msg, va);
    ap = 0;
    if (ParseThreadInfo->parseInfoNum)
        Com_Error(ERR_DROP, "%sFile %s, line %i: %s", v3->errorPrefix, v3->parseFile, v3->lines, string);
    else
        Com_Error(ERR_DROP, "%s", string);
}

void Com_ScriptError(const char *msg, ...)
{
    char string[4096]; // [esp+0h] [ebp-1010h] BYREF
    char *ap; // [esp+1004h] [ebp-Ch]
    parseInfo_t *v3; // [esp+1008h] [ebp-8h]
    ParseThreadInfo *ParseThreadInfo; // [esp+100Ch] [ebp-4h]
    va_list va; // [esp+101Ch] [ebp+Ch] BYREF

    va_start(va, msg);
    ParseThreadInfo = Com_GetParseThreadInfo();
    v3 = &ParseThreadInfo->parseInfo[ParseThreadInfo->parseInfoNum];
    va_copy(ap, va);
    _vsnprintf(string, 0x1000u, msg, va);
    ap = 0;
    if (ParseThreadInfo->parseInfoNum)
        Com_PrintError(23, "%sFile %s, line %i: %s", v3->warningPrefix, v3->parseFile, v3->lines, string);
    else
        Com_PrintError(23, "%s", string);
}

void __cdecl Com_UngetToken()
{
    parseInfo_t *pi; // [esp+0h] [ebp-8h]
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    if (pi->ungetToken)
        Com_ScriptErrorDrop("UngetToken called twice");
    pi->ungetToken = 1;
    parse->tokenPos = parse->prevTokenPos;
}

void __cdecl Com_ParseSetMark(const char **text, com_parse_mark_t *mark)
{
    parseInfo_t *pi; // [esp+0h] [ebp-8h]
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    if (!text)
        MyAssertHandler(".\\universal\\q_parse.cpp", 386, 0, "%s", "text");
    if (!mark)
        MyAssertHandler(".\\universal\\q_parse.cpp", 387, 0, "%s", "mark");
    mark->lines = pi->lines;
    mark->text = *text;
    mark->ungetToken = pi->ungetToken;
    mark->backup_lines = pi->backup_lines;
    mark->backup_text = pi->backup_text;
}

void __cdecl Com_ParseReturnToMark(const char **text, com_parse_mark_t *mark)
{
    parseInfo_t *pi; // [esp+0h] [ebp-8h]
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    if (!text)
        MyAssertHandler(".\\universal\\q_parse.cpp", 410, 0, "%s", "text");
    if (!mark)
        MyAssertHandler(".\\universal\\q_parse.cpp", 411, 0, "%s", "mark");
    pi->lines = mark->lines;
    *text = mark->text;
    pi->ungetToken = mark->ungetToken != 0;
    pi->backup_lines = mark->backup_lines;
    pi->backup_text = mark->backup_text;
}

int __cdecl Com_Compress(char *data_p)
{
    char c; // [esp+7h] [ebp-Dh]
    char *datai; // [esp+8h] [ebp-Ch]
    int size; // [esp+Ch] [ebp-8h]
    char *datao; // [esp+10h] [ebp-4h]

    size = 0;
    datao = data_p;
    datai = data_p;
    if (data_p)
    {
        while (1)
        {
            c = *datai;
            if (!*datai)
                break;
            if (c == 13 || c == 10)
            {
                *datao++ = c;
                ++size;
                ++datai;
            }
            else if (c == 47 && datai[1] == 47)
            {
                while (*datai && *datai != 10)
                    ++datai;
            }
            else if (c == 47 && datai[1] == 42)
            {
                while (*datai && (*datai != 42 || datai[1] != 47))
                {
                    if (*datai == 10)
                    {
                        *datao++ = 10;
                        ++size;
                    }
                    ++datai;
                }
                if (*datai)
                    datai += 2;
            }
            else
            {
                *datao++ = c;
                ++size;
                ++datai;
            }
        }
    }
    *datao = 0;
    return size;
}

const char *__cdecl Com_GetLastTokenPos()
{
    return Com_GetParseThreadInfo()->tokenPos;
}

parseInfo_t *__cdecl Com_Parse(const char **data_p)
{
    parseInfo_t *pi; // [esp+0h] [ebp-8h]
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    if (pi->ungetToken)
    {
        pi->ungetToken = 0;
        *data_p = pi->backup_text;
        pi->lines = pi->backup_lines;
    }
    return Com_ParseExt(data_p, 1);
}

const char *punctuation[15] =
{
    "+=",
    "-=",
    "*=",
    "/=",
    "&=",
    "|=",
    "++",
    "--",
    "&&",
    "||",
    "<=",
    ">=",
    "==",
    "!=",
    NULL // 14
};
parseInfo_t *__cdecl Com_ParseExt(const char **data_p, int allowLineBreaks)
{
    int j; // [esp+14h] [ebp-24h]
    signed int l; // [esp+18h] [ebp-20h]
    char c; // [esp+1Fh] [ebp-19h]
    char ca; // [esp+1Fh] [ebp-19h]
    char cb; // [esp+1Fh] [ebp-19h]
    int hasNewLines; // [esp+20h] [ebp-18h] BYREF
    const char *data; // [esp+24h] [ebp-14h]
    const char **punc; // [esp+28h] [ebp-10h]
    int len; // [esp+2Ch] [ebp-Ch]
    parseInfo_t *pi; // [esp+30h] [ebp-8h]
    ParseThreadInfo *parse; // [esp+34h] [ebp-4h]

    hasNewLines = 0;
    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    if (!data_p)
        MyAssertHandler(".\\universal\\q_parse.cpp", 658, 0, "%s", "data_p");
    data = *data_p;
    len = 0;
    pi->token[0] = 0;
    if (!data)
    {
        *data_p = 0;
        return pi;
    }
    pi->backup_lines = pi->lines;
    pi->backup_text = *data_p;
    if (pi->csv)
        return Com_ParseCSV(data_p, allowLineBreaks);
    while (1)
    {
        while (1)
        {
            data = SkipWhitespace(data, &hasNewLines);
            if (!data)
            {
                *data_p = 0;
                return pi;
            }
            if (hasNewLines && !allowLineBreaks)
                return pi;
            c = *data;
            if (*data != 47 || data[1] != 47)
                break;
            while (*data && *data != 10)
                ++data;
        }
        if (c != 47 || data[1] != 42)
            break;
        while (*data && (*data != 42 || data[1] != 47))
        {
            if (*data == 10)
                ++pi->lines;
            ++data;
        }
        if (*data)
            data += 2;
    }
    parse->prevTokenPos = parse->tokenPos;
    parse->tokenPos = data;
    if (c == 34)
    {
        if (pi->keepStringQuotes)
            pi->token[len++] = 34;
        ++data;
        while (1)
        {
            ca = *data++;
            if (ca == 92 && (*data == 34 || *data == 92))
            {
                ca = *data++;
            }
            else
            {
                if (ca == 34 || !ca)
                {
                    if (pi->keepStringQuotes)
                        pi->token[len++] = 34;
                    pi->token[len] = 0;
                    *data_p = data;
                    return pi;
                }
                if (*data == 10)
                    ++pi->lines;
            }
            if (len < 1023)
                pi->token[len++] = ca;
        }
    }
    if (pi->spaceDelimited)
    {
        do
        {
            if (len < 1023)
                pi->token[len++] = c;
            c = *++data;
        } while (*data > 32);
        if (len == 1024)
            len = 0;
        pi->token[len] = 0;
        *data_p = data;
        return pi;
    }
    else if (c >= 48 && c <= 57
        || pi->negativeNumbers && c == 45 && data[1] >= 48 && data[1] <= 57
        || c == 46 && data[1] >= 48 && data[1] <= 57)
    {
        do
        {
            if (len < 1023)
                pi->token[len++] = c;
            c = *++data;
        } while (*data >= 48 && c <= 57 || c == 46);
        if (c == 101 || c == 69)
        {
            if (len < 1023)
                pi->token[len++] = c;
            cb = *++data;
            if (*data == 45 || cb == 43)
            {
                if (len < 1023)
                    pi->token[len++] = cb;
                cb = *++data;
            }
            do
            {
                if (len < 1023)
                    pi->token[len++] = cb;
                cb = *++data;
            } while (*data >= 48 && cb <= 57);
        }
        if (len == 1024)
            len = 0;
        pi->token[len] = 0;
        *data_p = data;
        return pi;
    }
    else if (c >= 97 && c <= 122 || c >= 65 && c <= 90 || c == 95 || c == 47 || c == 92)
    {
        do
        {
            if (len < 1023)
                pi->token[len++] = c;
            c = *++data;
        } while (*data >= 97 && c <= 122 || c >= 65 && c <= 90 || c == 95 || c >= 48 && c <= 57);
        if (len == 1024)
            len = 0;
        pi->token[len] = 0;
        *data_p = data;
        return pi;
    }
    else
    {
        for (punc = punctuation; *punc; ++punc)
        {
            l = strlen(*punc);
            for (j = 0; j < l && data[j] == (*punc)[j]; ++j)
                ;
            if (j == l)
            {
                memcpy((unsigned __int8 *)pi, (unsigned __int8 *)*punc, l);
                pi->token[l] = 0;
                data += l;
                *data_p = data;
                return pi;
            }
        }
        pi->token[0] = *data;
        pi->token[1] = 0;
        *data_p = ++data;
        return pi;
    }
}

const char *__cdecl SkipWhitespace(const char *data, int *hasNewLines)
{
    int c; // [esp+0h] [ebp-Ch]
    parseInfo_t *pi; // [esp+4h] [ebp-8h]
    ParseThreadInfo *parse; // [esp+8h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    while (1)
    {
        c = *data;
        if (c > 32)
            break;
        if (!*data)
            return 0;
        if (c == 10)
        {
            ++pi->lines;
            *hasNewLines = 1;
        }
        ++data;
    }
    return data;
}

parseInfo_t *__cdecl Com_ParseCSV(const char **data_p, int allowLineBreaks)
{
    const char *v3; // [esp+0h] [ebp-14h]
    const char *data; // [esp+4h] [ebp-10h]
    const char *dataa; // [esp+4h] [ebp-10h]
    unsigned int len; // [esp+8h] [ebp-Ch]
    parseInfo_t *pi; // [esp+Ch] [ebp-8h]
    ParseThreadInfo *parse; // [esp+10h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    data = *data_p;
    len = 0;
    pi->token[0] = 0;
    if (allowLineBreaks)
    {
        while (*data == 13 || *data == 10)
            ++data;
    }
    else if (*data == 13 || *data == 10)
    {
        return pi;
    }
    parse->prevTokenPos = parse->tokenPos;
    parse->tokenPos = data;
    while (*data && *data != 44 && *data != 10)
    {
        if (*data == 13)
        {
            ++data;
        }
        else if (*data == 34)
        {
            for (dataa = data + 1; ; dataa += 2)
            {
                while (*dataa != 34)
                {
                    if (len < 0x3FF)
                        pi->token[len++] = *dataa;
                    ++dataa;
                }
                if (dataa[1] != 34)
                    break;
                if (len < 0x3FF)
                    pi->token[len++] = 34;
            }
            data = dataa + 1;
        }
        else
        {
            if (len < 0x3FF)
                pi->token[len++] = *data;
            ++data;
        }
    }
    if (*data && *data != 10)
        ++data;
    if (*data || len)
        v3 = data;
    else
        v3 = 0;
    *data_p = v3;
    pi->token[len] = 0;
    return pi;
}

parseInfo_t *__cdecl Com_ParseOnLine(const char **data_p)
{
    parseInfo_t *pi; // [esp+0h] [ebp-8h]
    ParseThreadInfo *parse; // [esp+4h] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    pi = &parse->parseInfo[parse->parseInfoNum];
    if (pi->ungetToken)
    {
        pi->ungetToken = 0;
        if (!pi->spaceDelimited)
            return pi;
        *data_p = pi->backup_text;
        pi->lines = pi->backup_lines;
    }
    return Com_ParseExt(data_p, 0);
}

int __cdecl Com_MatchToken(const char **buf_p, const char *match, int warning)
{
    parseInfo_t *token; // [esp+14h] [ebp-4h]

    token = Com_Parse(buf_p);
    if (!strcmp(token->token, match))
        return 1;
    if (warning)
        Com_ScriptError("MatchToken: got '%s', expected '%s'\n", token->token, match);
    else
        Com_ScriptErrorDrop("MatchToken: got '%s', expected '%s'\n", token->token, match);
    return 0;
}

int __cdecl Com_SkipBracedSection(const char **program, unsigned int startDepth, int iMaxNesting)
{
    int bNestingExceeded; // [esp+4h] [ebp-8h]
    parseInfo_t *token; // [esp+8h] [ebp-4h]

    bNestingExceeded = 0;
    do
    {
        token = Com_Parse(program);
        if (!token->token[1])
        {
            if (token->token[0] == 123)
            {
                if (startDepth == iMaxNesting)
                    bNestingExceeded = 1;
                else
                    ++startDepth;
            }
            else if (token->token[0] == 125)
            {
                --startDepth;
            }
        }
    } while (startDepth && *program);
    return bNestingExceeded;
}

void __cdecl Com_SkipRestOfLine(const char **data)
{
    int c; // [esp+0h] [ebp-10h]
    ParseThreadInfo *parse; // [esp+8h] [ebp-8h]
    const char *p; // [esp+Ch] [ebp-4h]

    parse = Com_GetParseThreadInfo();
    p = *data;
    if (*data)
    {
        while (1)
        {
            c = *p;
            if (!*p)
                break;
            ++p;
            if (c == 10)
            {
                ++parse->parseInfo[parse->parseInfoNum].lines;
                break;
            }
        }
        *data = p;
    }
}

int __cdecl Com_GetArgCountOnLine(const char **data_p)
{
    com_parse_mark_t mark; // [esp+0h] [ebp-1Ch] BYREF
    const char *token; // [esp+14h] [ebp-8h]
    int count; // [esp+18h] [ebp-4h]

    Com_ParseSetMark(data_p, &mark);
    for (count = 0; ; ++count)
    {
        token = (const char *)Com_ParseOnLine(data_p);
        if (!*token)
            break;
    }
    Com_ParseReturnToMark(data_p, &mark);
    return count;
}

double __cdecl Com_ParseFloat(const char **buf_p)
{
    parseInfo_t *token; // [esp+4h] [ebp-4h]

    token = Com_Parse(buf_p);
    return (float)atof(token->token);
}

double __cdecl Com_ParseFloatOnLine(const char **buf_p)
{
    parseInfo_t *token; // [esp+4h] [ebp-4h]

    token = Com_ParseOnLine(buf_p);
    return (float)atof(token->token);
}

int __cdecl Com_ParseInt(const char **buf_p)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    token = Com_Parse(buf_p);
    return atoi(token->token);
}

void __cdecl Com_Parse1DMatrix(const char **buf_p, int x, float *m)
{
    parseInfo_t *token; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    Com_MatchToken(buf_p, "(", 0);
    for (i = 0; i < x; ++i)
    {
        token = Com_Parse(buf_p);
        m[i] = atof(token->token);
    }
    Com_MatchToken(buf_p, ")", 0);
}

