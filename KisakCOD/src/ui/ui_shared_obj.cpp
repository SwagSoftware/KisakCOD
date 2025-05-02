#include "ui.h"
#include <universal/com_files.h>
#include <universal/q_parse.h>
#include <universal/com_sndalias.h>

$F99A9AECA2B60514CA5C8024B8EAC369 g_load_0;
char menuBuf1[4096];

const KeywordHashEntry<menuDef_t, 128, 128> *menuParseKeywordHash[128];
const KeywordHashEntry<itemDef_s, 256, 3855> *itemParseKeywordHash[256];

int s_consumedOperandCount[26] =
{
  1,
  -1,
  2,
  0,
  1,
  1,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
}; // idb

punctuation_s default_punctuations[53] =
{
  { (char*)">>=", 1, NULL },
  { (char*)"<<=", 2, NULL },
  { (char*)"...", 3, NULL },
  { (char*)"##", 4, NULL },
  { (char*)"&", 5, NULL },
  { (char*)"||", 6, NULL },
  { (char*)">=", 7, NULL },
  { (char*)"<=", 8, NULL },
  { (char*)"==", 9, NULL },
  { (char*)"!=", 10, NULL },
  { (char*)"*=", 11, NULL },
  { (char*)"/=", 12, NULL },
  { (char*)"%=", 13, NULL },
  { (char*)"+=", 14, NULL },
  { (char*)"-=", 15, NULL },
  { (char*)"++", 16, NULL },
  { (char*)"--", 17, NULL },
  { (char*)"&=", 18, NULL },
  { (char*)"|=", 19, NULL },
  { (char*)"^=", 20, NULL },
  { (char*)">>", 21, NULL },
  { (char*)"<<", 22, NULL },
  { (char*)"->", 23, NULL },
  { (char*)"::", 24, NULL },
  { (char*)".*", 25, NULL },
  { (char*)"*", 26, NULL },
  { (char*)"/", 27, NULL },
  { (char*)"%", 28, NULL },
  { (char*)"+", 29, NULL },
  { (char*)"-", 30, NULL },
  { (char*)"=", 31, NULL },
  { (char*)"&", 32, NULL },
  { (char*)"|", 33, NULL },
  { (char*)"^", 34, NULL },
  { (char*)"~", 35, NULL },
  { (char*)"!", 36, NULL },
  { (char*)">", 37, NULL },
  { (char*)"<", 38, NULL },
  { (char*)".", 39, NULL },
  { (char*)",", 40, NULL },
  { (char*)";", 41, NULL },
  { (char*)":", 42, NULL },
  { (char*)"?", 43, NULL },
  { (char*)"(", 44, NULL },
  { (char*)")", 45, NULL },
  { (char*)"{", 46, NULL },
  { (char*)"}", 47, NULL },
  { (char*)"[", 48, NULL },
  { (char*)"]", 49, NULL },
  { (char*)"\\", 50, NULL },
  { (char*)"#", 51, NULL },
  { (char*)"$", 52, NULL },
  { NULL, 0, NULL }
}; // idb

int __cdecl PC_Directive_include(source_s *source);
int __cdecl PC_Directive_undef(source_s *source);
int __cdecl PC_Directive_define(source_s *source);
int __cdecl PC_Directive_if_def(source_s *source, int type);
int __cdecl PC_Directive_ifdef(source_s *source);
int __cdecl PC_Directive_ifndef(source_s *source);
int __cdecl PC_Directive_else(source_s *source);
int __cdecl PC_Directive_endif(source_s *source);
int __cdecl PC_Directive_elif(source_s *source);
int __cdecl PC_Directive_if(source_s *source);
int __cdecl PC_Directive_line(source_s *source);
int __cdecl PC_Directive_error(source_s *source);
int __cdecl PC_Directive_pragma(source_s *source);
int __cdecl PC_Directive_evalfloat(source_s *source);
int __cdecl PC_Directive_eval(source_s *source);
int __cdecl PC_Evaluate(source_s *source, int *intvalue, double *floatvalue, int integer);
int __cdecl PC_ExpandDefineIntoSource(source_s *source, token_s *deftoken, define_s *define);
int __cdecl PC_Float_Expression_Parse(int handle, float *f);

int __cdecl PC_DollarDirective_evalfloat(source_s *source);
int __cdecl PC_DollarDirective_evalint(source_s *source);

const directive_s directives[15] =
{
  { (char*)"if", &PC_Directive_if },
  { (char*)"ifdef", &PC_Directive_ifdef },
  { (char*)"ifndef", &PC_Directive_ifndef },
  { (char*)"elif", &PC_Directive_elif },
  { (char*)"else", &PC_Directive_else },
  { (char*)"endif", &PC_Directive_endif },
  { (char*)"include", &PC_Directive_include },
  { (char*)"define", &PC_Directive_define },
  { (char*)"undef", &PC_Directive_undef },
  { (char*)"line", &PC_Directive_line },
  { (char*)"error", &PC_Directive_error },
  { (char*)"pragma", &PC_Directive_pragma },
  { (char*)"eval", &PC_Directive_eval },
  { (char*)"evalfloat", &PC_Directive_evalfloat },
  { NULL, NULL }
}; // idb

directive_s dollardirectives[20] =
{
  { (char*)"evalint", &PC_DollarDirective_evalint },
  { (char*)"evalfloat", &PC_DollarDirective_evalfloat },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL },
  { NULL, NULL }
}; // idb

void __cdecl PrintSourceStack(const script_s *scriptstack)
{
    script_s *scriptstacka; // [esp+8h] [ebp+8h]

    for (scriptstacka = scriptstack->next; scriptstacka; scriptstacka = scriptstacka->next)
        Com_PrintWarning(23, "  From file %s, line %d\n", scriptstacka->filename, scriptstacka->line);
}

void SourceError(source_s *source, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+41Ch] [ebp+10h] BYREF

    va_start(va, str);
    _vsnprintf(text, 0x400u, str, va);
    Com_PrintError(23, "Error: file %s, line %d: %s\n", source->scriptstack->filename, source->scriptstack->line, text);
    PrintSourceStack(source->scriptstack);
}

void __cdecl PC_PushScript(source_s *source, script_s *script)
{
    script_s *s; // [esp+0h] [ebp-4h]

    for (s = source->scriptstack; s; s = s->next)
    {
        if (!I_stricmp(s->filename, script->filename))
        {
            SourceError(source, "%s recursively included", script->filename);
            return;
        }
    }
    script->next = source->scriptstack;
    source->scriptstack = script;
}

unsigned int *__cdecl GetMemory(unsigned int size)
{
    unsigned int *ptr; // [esp+4h] [ebp-4h]

    ptr = (unsigned int *)Z_Malloc(size + 4, "GetMemory", 10);
    if (!ptr)
        return 0;
    *ptr = 0x12345678;
    return ptr + 1;
}

void __cdecl FreeMemory(char *ptr)
{
    if (*((_DWORD *)ptr - 1) == 0x12345678)
        Z_Free(ptr - 4, 10);
}

int numtokens;
token_s *__cdecl PC_CopyToken(token_s *token)
{
    unsigned int *t; // [esp+8h] [ebp-4h]

    t = GetMemory(0x430u);
    if (t)
    {
        memcpy(t, token, 0x430u);
        t[266] = 0;
        ++numtokens;
        return (token_s *)t;
    }
    else
    {
        Com_Error(ERR_FATAL, "EXE_ERR_OUT_OF_MEMORY");
        return 0;
    }
}

void __cdecl PC_FreeToken(token_s *token)
{
    FreeMemory(token->string);
    --numtokens;
}

int __cdecl PS_ReadWhiteSpace(script_s *script)
{
    while (1)
    {
        while (*script->script_p <= 32)
        {
            if (!*script->script_p)
                return 0;
            if (*script->script_p == 10)
                ++script->line;
            ++script->script_p;
        }
        if (*script->script_p != 47)
            break;
        if (script->script_p[1] == 47)
        {
            ++script->script_p;
            do
            {
                if (!*++script->script_p)
                    return 0;
            } while (*script->script_p != 10);
            ++script->line;
            if (!*++script->script_p)
                return 0;
        }
        else
        {
            if (script->script_p[1] != 42)
                return 1;
            ++script->script_p;
            do
            {
                if (!*++script->script_p)
                    return 0;
                if (*script->script_p == 10)
                    ++script->line;
            } while (*script->script_p != 42 || script->script_p[1] != 47);
            if (!*++script->script_p)
                return 0;
            if (!*++script->script_p)
                return 0;
        }
    }
    return 1;
}

void ScriptError(script_s *script, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+41Ch] [ebp+10h] BYREF

    va_start(va, str);
    if ((script->flags & 1) == 0)
    {
        _vsnprintf(text, 0x400u, str, va);
        Com_PrintError(23, "Error: file %s, line %d: %s\n", script->filename, script->line, text);
    }
}

void ScriptWarning(script_s *script, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+41Ch] [ebp+10h] BYREF

    va_start(va, str);
    if ((script->flags & 2) == 0)
    {
        _vsnprintf(text, 0x400u, str, va);
        Com_PrintWarning(23, "Warning: file %s, line %d: %s\n", script->filename, script->line, text);
    }
}

int __cdecl PS_ReadEscapeCharacter(script_s *script, char *ch)
{
    char c; // [esp+4h] [ebp-Ch]
    int ca; // [esp+4h] [ebp-Ch]
    int cb; // [esp+4h] [ebp-Ch]
    int cc; // [esp+4h] [ebp-Ch]
    int val; // [esp+8h] [ebp-8h]
    int vala; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]
    int ia; // [esp+Ch] [ebp-4h]

    switch (*++script->script_p)
    {
    case '"':
        c = 34;
        goto LABEL_38;
    case '\'':
        c = 39;
        goto LABEL_38;
    case '?':
        c = 63;
        goto LABEL_38;
    case '\\':
        c = 92;
        goto LABEL_38;
    case 'a':
        c = 7;
        goto LABEL_38;
    case 'b':
        c = 8;
        goto LABEL_38;
    case 'f':
        c = 12;
        goto LABEL_38;
    case 'n':
        c = 10;
        goto LABEL_38;
    case 'r':
        c = 13;
        goto LABEL_38;
    case 't':
        c = 9;
        goto LABEL_38;
    case 'v':
        c = 11;
        goto LABEL_38;
    case 'x':
        ++script->script_p;
        i = 0;
        val = 0;
        break;
    default:
        if (*script->script_p < 48 || *script->script_p > 57)
            ScriptError(script, "unknown escape char");
        ia = 0;
        vala = 0;
        while (1)
        {
            cc = *script->script_p;
            if (cc < 48 || cc > 57)
                break;
            vala = cc - 48 + 10 * vala;
            ++ia;
            ++script->script_p;
        }
        --script->script_p;
        if (vala > 255)
        {
            ScriptWarning(script, "too large value in escape character");
            LOBYTE(vala) = -1;
        }
        c = vala;
        goto LABEL_38;
    }
    while (1)
    {
        ca = *script->script_p;
        if (ca >= 48 && ca <= 57)
        {
            cb = ca - 48;
            goto LABEL_24;
        }
        if (ca >= 65 && ca <= 90)
        {
            cb = ca - 55;
            goto LABEL_24;
        }
        if (ca < 97 || ca > 122)
            break;
        cb = ca - 87;
    LABEL_24:
        val = cb + 16 * val;
        ++i;
        ++script->script_p;
    }
    --script->script_p;
    if (val > 255)
    {
        ScriptWarning(script, "too large value in escape character");
        LOBYTE(val) = -1;
    }
    c = val;
LABEL_38:
    ++script->script_p;
    *ch = c;
    return 1;
}

int __cdecl PS_ReadString(script_s *script, token_s *token, int quote)
{
    int tmpline; // [esp+0h] [ebp-Ch]
    int len; // [esp+4h] [ebp-8h]
    int lena; // [esp+4h] [ebp-8h]
    char *tmpscript_p; // [esp+8h] [ebp-4h]

    if (quote == 34)
        token->type = 1;
    else
        token->type = 2;
    token->string[0] = *script->script_p;
    len = 1;
    ++script->script_p;
    while (1)
    {
        while (1)
        {
            while (1)
            {
                if (len >= 1022)
                {
                    ScriptError(script, "string longer than MAX_TOKEN = %d", 1024);
                    return 0;
                }
                if (*script->script_p != 92 || (script->flags & 8) != 0)
                    break;
                if (!PS_ReadEscapeCharacter(script, &token->string[len]))
                {
                    token->string[len] = 0;
                    return 0;
                }
                ++len;
            }
            if (*script->script_p == quote)
                break;
            if (!*script->script_p)
            {
                token->string[len] = 0;
                ScriptError(script, "missing trailing quote");
                return 0;
            }
            if (*script->script_p == 10)
            {
                token->string[len] = 0;
                ScriptError(script, "newline inside string %s", token->string);
                return 0;
            }
            token->string[len++] = *script->script_p++;
        }
        ++script->script_p;
        if ((script->flags & 4) != 0)
            break;
        tmpscript_p = script->script_p;
        tmpline = script->line;
        if (!PS_ReadWhiteSpace(script))
        {
            script->script_p = tmpscript_p;
            script->line = tmpline;
            break;
        }
        if (*script->script_p != quote)
        {
            script->script_p = tmpscript_p;
            script->line = tmpline;
            break;
        }
        ++script->script_p;
    }
    token->string[len] = quote;
    lena = len + 1;
    token->string[lena] = 0;
    token->subtype = lena;
    return 1;
}

void __cdecl NumberValue(char *string, __int16 subtype, unsigned int *intvalue, long double *floatvalue)
{
    unsigned int dotfound; // [esp+40h] [ebp-4h]
    char *stringa; // [esp+4Ch] [ebp+8h]
    char *stringb; // [esp+4Ch] [ebp+8h]
    char *stringc; // [esp+4Ch] [ebp+8h]

    dotfound = 0;
    *intvalue = 0;
    *floatvalue = 0.0;
    if ((subtype & 0x800) != 0)
    {
        while (*string)
        {
            if (*string == 46)
            {
                if (dotfound)
                    return;
                dotfound = 10;
                ++string;
            }
            if (dotfound)
            {
                *floatvalue = (double)(*string - 48) / (double)dotfound + *floatvalue;
                dotfound *= 10;
            }
            else
            {
                *floatvalue = *floatvalue * 10.0 + (double)(*string - 48);
            }
            ++string;
        }
        *intvalue = (__int64)*floatvalue;
    }
    else if ((subtype & 8) != 0)
    {
        while (*string)
            *intvalue = 10 * *intvalue + *string++ - 48;
        *floatvalue = (double)*intvalue;
    }
    else if ((subtype & 0x100) != 0)
    {
        for (stringa = string + 2; *stringa; ++stringa)
        {
            *intvalue *= 16;
            if (*stringa < 97 || *stringa > 102)
            {
                if (*stringa < 65 || *stringa > 70)
                    *intvalue = *stringa + *intvalue - 48;
                else
                    *intvalue = *stringa + *intvalue - 55;
            }
            else
            {
                *intvalue = *stringa + *intvalue - 87;
            }
        }
        *floatvalue = (double)*intvalue;
    }
    else if ((subtype & 0x200) != 0)
    {
        for (stringb = string + 1; *stringb; ++stringb)
            *intvalue = *stringb + 8 * *intvalue - 48;
        *floatvalue = (double)*intvalue;
    }
    else if ((subtype & 0x400) != 0)
    {
        for (stringc = string + 2; *stringc; ++stringc)
            *intvalue = *stringc + 2 * *intvalue - 48;
        *floatvalue = (double)*intvalue;
    }
}

int __cdecl PS_ReadNumber(script_s *script, token_s *token)
{
    int v3; // edx
    BOOL octal; // [esp+0h] [ebp-14h]
    char c; // [esp+7h] [ebp-Dh]
    char ca; // [esp+7h] [ebp-Dh]
    char cb; // [esp+7h] [ebp-Dh]
    char cc; // [esp+7h] [ebp-Dh]
    int len; // [esp+8h] [ebp-Ch]
    int i; // [esp+Ch] [ebp-8h]
    int dot; // [esp+10h] [ebp-4h]

    len = 0;
    token->type = 3;
    if (*script->script_p == 48 && (script->script_p[1] == 120 || script->script_p[1] == 88))
    {
        token->string[0] = *script->script_p++;
        token->string[1] = *script->script_p;
        len = 2;
        for (c = *++script->script_p; c >= 48 && c <= 57 || c >= 97 && c <= 102 || c == 65; c = *script->script_p)
        {
            token->string[len++] = *script->script_p++;
            if (len >= 1024)
            {
                ScriptError(script, "hexadecimal number longer than MAX_TOKEN = %d", 1024);
                return 0;
            }
        }
        token->subtype |= 0x100u;
        goto LABEL_41;
    }
    if (*script->script_p == 48 && (script->script_p[1] == 98 || script->script_p[1] == 66))
    {
        token->string[0] = *script->script_p++;
        token->string[1] = *script->script_p;
        len = 2;
        for (ca = *++script->script_p; ca == 48 || ca == 49; ca = *script->script_p)
        {
            token->string[len++] = *script->script_p++;
            if (len >= 1024)
            {
                ScriptError(script, "binary number longer than MAX_TOKEN = %d", 1024);
                return 0;
            }
        }
        token->subtype |= 0x400u;
        goto LABEL_41;
    }
    dot = 0;
    octal = *script->script_p == 48;
    while (1)
    {
        cb = *script->script_p;
        if (cb != 46)
            break;
        dot = 1;
    LABEL_33:
        token->string[len++] = *script->script_p++;
        if (len >= 1023)
        {
            ScriptError(script, "number longer than MAX_TOKEN = %d", 1024);
            return 0;
        }
    }
    if (cb == 56 || cb == 57)
    {
        octal = 0;
        goto LABEL_33;
    }
    if (cb >= 48 && cb <= 57)
        goto LABEL_33;
    if (octal)
        v3 = token->subtype | 0x200;
    else
        v3 = token->subtype | 8;
    token->subtype = v3;
    if (dot)
        token->subtype |= 0x800u;
LABEL_41:
    for (i = 0; i < 2; ++i)
    {
        cc = *script->script_p;
        if ((cc == 108 || cc == 76) && (token->subtype & 0x2000) == 0)
        {
            ++script->script_p;
            token->subtype |= 0x2000u;
        }
        else if ((cc == 117 || cc == 85) && (token->subtype & 0x4800) == 0)
        {
            ++script->script_p;
            token->subtype |= 0x4000u;
        }
    }
    token->string[len] = 0;
    NumberValue(token->string, token->subtype, &token->intvalue, &token->floatvalue);
    if ((token->subtype & 0x800) == 0)
        token->subtype |= 0x1000u;
    return 1;
}

int __cdecl PS_ReadPrimitive(script_s *script, token_s *token)
{
    int len; // [esp+8h] [ebp-4h]

    len = 0;
    while (*script->script_p > 32 && *script->script_p != 59)
    {
        if (len >= 1024)
        {
            ScriptError(script, "primitive token longer than MAX_TOKEN = %d", 1024);
            return 0;
        }
        token->string[len++] = *script->script_p++;
    }
    token->string[len] = 0;
    memcpy(&script->token, token, sizeof(script->token));
    return 1;
}

int __cdecl PS_ReadName(script_s *script, token_s *token)
{
    char c; // [esp+3h] [ebp-5h]
    int len; // [esp+4h] [ebp-4h]

    len = 0;
    token->type = 4;
    do
    {
        token->string[len++] = *script->script_p++;
        if (len >= 1024)
        {
            ScriptError(script, "name longer than MAX_TOKEN = %d", 1024);
            return 0;
        }
        c = *script->script_p;
    } while (c >= 97 && c <= 122 || c >= 65 && c <= 90 || c >= 48 && c <= 57 || c == 95);
    token->string[len] = 0;
    token->subtype = len;
    return 1;
}

int __cdecl PS_ReadPunctuation(script_s *script, token_s *token)
{
    punctuation_s *punc; // [esp+10h] [ebp-Ch]
    unsigned int len; // [esp+14h] [ebp-8h]
    char *p; // [esp+18h] [ebp-4h]

    for (punc = script->punctuationtable[*script->script_p]; punc; punc = punc->next)
    {
        p = punc->p;
        len = strlen(punc->p);
        if (&script->script_p[len] <= script->end_p && !strncmp(script->script_p, p, len))
        {
            strncpy((unsigned __int8 *)token, (unsigned __int8 *)p, 0x400u);
            script->script_p += len;
            token->type = 5;
            token->subtype = punc->n;
            return 1;
        }
    }
    return 0;
}

int __cdecl PS_ReadToken(script_s *script, token_s *token)
{
    if (script->tokenavailable)
    {
        script->tokenavailable = 0;
        memcpy(token, &script->token, sizeof(token_s));
        return 1;
    }
    script->lastscript_p = script->script_p;
    script->lastline = script->line;
    memset((unsigned __int8 *)token, 0, sizeof(token_s));
    script->whitespace_p = script->script_p;
    token->whitespace_p = script->script_p;
    if (!PS_ReadWhiteSpace(script))
        return 0;
    script->endwhitespace_p = script->script_p;
    token->endwhitespace_p = script->script_p;
    token->line = script->line;
    token->linescrossed = script->line - script->lastline;
    if (*script->script_p == 34)
    {
        if (!PS_ReadString(script, token, 34))
            return 0;
    }
    else if (*script->script_p == 39)
    {
        if (!PS_ReadString(script, token, 39))
            return 0;
    }
    else if (*script->script_p >= 48 && *script->script_p <= 57
        || *script->script_p == 46 && script->script_p[1] >= 48 && script->script_p[1] <= 57)
    {
        if (!PS_ReadNumber(script, token))
            return 0;
    }
    else
    {
        if ((script->flags & 0x10) != 0)
            return PS_ReadPrimitive(script, token);
        if (*script->script_p >= 97 && *script->script_p <= 122
            || *script->script_p >= 65 && *script->script_p <= 90
            || *script->script_p == 95)
        {
            if (!PS_ReadName(script, token))
                return 0;
        }
        else if (!PS_ReadPunctuation(script, token))
        {
            ScriptError(script, "can't read token");
            return 0;
        }
    }
    memcpy(&script->token, token, sizeof(script->token));
    return 1;
}

BOOL __cdecl EndOfScript(script_s *script)
{
    return script->script_p >= script->end_p;
}

void SourceWarning(source_s *source, const char *str, ...)
{
    char text[1028]; // [esp+4h] [ebp-408h] BYREF
    va_list va; // [esp+41Ch] [ebp+10h] BYREF

    va_start(va, str);
    _vsnprintf(text, 0x400u, str, va);
    Com_PrintWarning(
        23,
        "Warning: file %s, line %d: %s\n",
        source->scriptstack->filename,
        source->scriptstack->line,
        text);
    PrintSourceStack(source->scriptstack);
}

void __cdecl FreeScript(script_s *script)
{
    if (script->punctuationtable)
        FreeMemory((char *)script->punctuationtable);
    FreeMemory(script->filename);
}

int __cdecl PC_ReadSourceToken(source_s *source, token_s *token)
{
    token_s *t; // [esp+8h] [ebp-10h]
    script_s *script; // [esp+Ch] [ebp-Ch]
    parseSkip_t skip; // [esp+10h] [ebp-8h] BYREF
    int type; // [esp+14h] [ebp-4h] BYREF

    while (!source->tokens)
    {
        if (PS_ReadToken(source->scriptstack, token))
            return 1;
        if (EndOfScript(source->scriptstack))
        {
            while (source->indentstack && source->indentstack->script == source->scriptstack)
            {
                SourceWarning(source, "missing #endif");
                PC_PopIndent(source, &type, &skip);
            }
        }
        if (!source->scriptstack->next)
            return 0;
        script = source->scriptstack;
        source->scriptstack = script->next;
        FreeScript(script);
    }
    memcpy(token, source->tokens, sizeof(token_s));
    t = source->tokens;
    source->tokens = t->next;
    PC_FreeToken(t);
    return 1;
}

void __cdecl PC_PopIndent(source_s *source, int *type, parseSkip_t *skip)
{
    indent_s *indent; // [esp+0h] [ebp-4h]

    *type = 0;
    *skip = SKIP_NO;
    indent = source->indentstack;
    if (indent)
    {
        if (source->indentstack->script == source->scriptstack)
        {
            *type = indent->type;
            *skip = indent->skip;
            source->indentstack = source->indentstack->next;
            source->skip -= indent->skip != SKIP_NO;
            FreeMemory((char *)indent);
        }
    }
}

int __cdecl PC_UnreadSourceToken(source_s *source, token_s *token)
{
    token_s *v2; // eax

    v2 = PC_CopyToken(token);
    v2->next = source->tokens;
    source->tokens = v2;
    return 1;
}

int __cdecl PC_ReadDefineParms(source_s *source, define_s *define, token_s **parms, int maxparms)
{
    token_s *t; // [esp+50h] [ebp-454h]
    int done; // [esp+54h] [ebp-450h]
    int lastcomma; // [esp+58h] [ebp-44Ch]
    int indent; // [esp+5Ch] [ebp-448h]
    int numparms; // [esp+60h] [ebp-444h]
    token_s token; // [esp+64h] [ebp-440h] BYREF
    int i; // [esp+49Ch] [ebp-8h]
    token_s *last; // [esp+4A0h] [ebp-4h]

    if (!PC_ReadSourceToken(source, &token))
    {
        SourceError(source, "define %s missing parms", define->name);
        return 0;
    }
    if (define->numparms > maxparms)
    {
        SourceError(source, "define with more than %d parameters", maxparms);
        return 0;
    }
    for (i = 0; i < define->numparms; ++i)
        parms[i] = 0;
    if (strcmp(token.string, "("))
    {
        PC_UnreadSourceToken(source, &token);
        SourceError(source, "define %s missing parms", define->name);
        return 0;
    }
    done = 0;
    numparms = 0;
    indent = 0;
LABEL_11:
    if (!done)
    {
        if (numparms >= maxparms)
        {
            SourceError(source, "define %s with too many parms", define->name);
            return 0;
        }
        if (numparms >= define->numparms)
        {
            SourceWarning(source, "define %s has too many parms", define->name);
            return 0;
        }
        parms[numparms] = 0;
        lastcomma = 1;
        last = 0;
        while (1)
        {
            if (!PC_ReadSourceToken(source, &token))
            {
                SourceError(source, "define %s incomplete", define->name);
                return 0;
            }
            if (!strcmp(token.string, ",") && !indent)
            {
                if (lastcomma)
                    SourceWarning(source, "too many comma's");
                goto LABEL_38;
            }
            lastcomma = 0;
            if (!strcmp(token.string, "("))
            {
                ++indent;
            }
            else if (!strcmp(token.string, ")"))
            {
                if (!indent)
                {
                    if (!parms[define->numparms - 1])
                        SourceWarning(source, "too few define parms to %s", define->name);
                    done = 1;
                LABEL_38:
                    ++numparms;
                    goto LABEL_11;
                }
                --indent;
            }
            if (numparms < define->numparms)
            {
                t = PC_CopyToken(&token);
                t->next = 0;
                if (last)
                    last->next = t;
                else
                    parms[numparms] = t;
                last = t;
            }
        }
    }
    return 1;
}

int __cdecl PC_StringizeTokens(token_s *tokens, token_s *token)
{
    token->type = 1;
    token->whitespace_p = 0;
    token->endwhitespace_p = 0;
    token->string[0] = 0;
    strcat(token->string, "\"");
    while (tokens)
    {
        strncat(token->string, tokens->string, 1024 - strlen(token->string));
        tokens = tokens->next;
    }
    strncat(token->string, "\"", 1024 - strlen(token->string));
    return 1;
}

int __cdecl PC_MergeTokens(token_s *t1, token_s *t2)
{
    if (t1->type == 4 && (t2->type == 4 || t2->type == 3))
    {
        strcat(t1->string, t2->string);
        return 1;
    }
    else if (t1->type == 1 && t2->type == 1)
    {
        t1->string[strlen(t1->string) - 1] = 0;
        strcat(t1->string, &t2->string[1]);
        return 1;
    }
    else
    {
        return 0;
    }
}

int __cdecl PC_NameHash(char *name)
{
    int hash; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    hash = 0;
    for (i = 0; name[i]; ++i)
        hash += (i + 119) * name[i];
    return ((unsigned __int16)(hash >> 20) ^ (unsigned __int16)(hash ^ (hash >> 10))) & 0x3FF;
}

void __cdecl PC_AddDefineToHash(define_s *define, define_s **definehash)
{
    int hash; // [esp+0h] [ebp-4h]

    hash = PC_NameHash(define->name);
    define->hashnext = definehash[hash];
    definehash[hash] = define;
}

define_s *__cdecl PC_FindHashedDefine(define_s **definehash, char *name)
{
    define_s *d; // [esp+14h] [ebp-8h]

    for (d = definehash[PC_NameHash(name)]; d; d = d->hashnext)
    {
        if (!strcmp(d->name, name))
            return d;
    }
    return 0;
}

int __cdecl PC_FindDefineParm(define_s *define, const char *name)
{
    int i; // [esp+14h] [ebp-8h]
    token_s *p; // [esp+18h] [ebp-4h]

    i = 0;
    for (p = define->parms; p; p = p->next)
    {
        if (!strcmp(p->string, name))
            return i;
        ++i;
    }
    return -1;
}

void __cdecl PC_FreeDefine(define_s *define)
{
    token_s *t; // [esp+0h] [ebp-8h]
    token_s *ta; // [esp+0h] [ebp-8h]
    token_s *next; // [esp+4h] [ebp-4h]
    token_s *nexta; // [esp+4h] [ebp-4h]

    for (t = define->parms; t; t = next)
    {
        next = t->next;
        PC_FreeToken(t);
    }
    for (ta = define->tokens; ta; ta = nexta)
    {
        nexta = ta->next;
        PC_FreeToken(ta);
    }
    FreeMemory((char *)define);
}

int __cdecl PC_ExpandBuiltinDefine(
    source_s *source,
    token_s *deftoken,
    define_s *define,
    token_s **firsttoken,
    token_s **lasttoken)
{
    char v6; // [esp+47h] [ebp-21h]
    token_s *v7; // [esp+4Ch] [ebp-1Ch]
    script_s *scriptstack; // [esp+50h] [ebp-18h]
    __int64 t; // [esp+58h] [ebp-10h] BYREF
    char *curtime; // [esp+60h] [ebp-8h]
    token_s *token; // [esp+64h] [ebp-4h]

    token = PC_CopyToken(deftoken);
    switch (define->builtin)
    {
    case 1:
        sprintf(token->string, "%d", deftoken->line);
        token->intvalue = deftoken->line;
        token->floatvalue = (double)deftoken->line;
        token->type = 3;
        token->subtype = 4104;
        *firsttoken = token;
        *lasttoken = token;
        return 1;
    case 2:
        scriptstack = source->scriptstack;
        v7 = token;
        do
        {
            v6 = scriptstack->filename[0];
            v7->string[0] = scriptstack->filename[0];
            scriptstack = (script_s *)((char *)scriptstack + 1);
            v7 = (token_s *)((char *)v7 + 1);
        } while (v6);
        token->type = 4;
        goto LABEL_8;
    case 3:
        t = _time64(0);
        curtime = _ctime64(&t);
        strcpy(token->string, "\"");
        strncat(token->string, curtime + 4, 7u);
        strncat(&token->string[7], curtime + 20, 4u);
        strcat(token->string, "\"");
        free(curtime);
        token->type = 4;
        goto LABEL_8;
    case 4:
        t = _time64(0);
        curtime = _ctime64(&t);
        strcpy(token->string, "\"");
        strncat(token->string, curtime + 11, 8u);
        strcat(token->string, "\"");
        free(curtime);
        token->type = 4;
    LABEL_8:
        token->subtype = strlen(token->string);
        *firsttoken = token;
        *lasttoken = token;
        break;
    default:
        *firsttoken = 0;
        *lasttoken = 0;
        break;
    }
    return 1;
}

int __cdecl PC_ExpandDefine(
    source_s *source,
    token_s *deftoken,
    define_s *define,
    token_s **firsttoken,
    token_s **lasttoken)
{
    int parmnum; // [esp+0h] [ebp-65Ch]
    int parmnuma; // [esp+0h] [ebp-65Ch]
    token_s *dt; // [esp+4h] [ebp-658h]
    token_s *t; // [esp+8h] [ebp-654h]
    token_s *ta; // [esp+8h] [ebp-654h]
    token_s *tb; // [esp+8h] [ebp-654h]
    token_s *nextpt; // [esp+10h] [ebp-64Ch]
    token_s *parms[128]; // [esp+14h] [ebp-648h] BYREF
    token_s *t2; // [esp+214h] [ebp-448h]
    token_s *pt; // [esp+218h] [ebp-444h]
    token_s token; // [esp+21Ch] [ebp-440h] BYREF
    int i; // [esp+650h] [ebp-Ch]
    token_s *last; // [esp+654h] [ebp-8h]
    token_s *first; // [esp+658h] [ebp-4h]

    if (define->builtin)
        return PC_ExpandBuiltinDefine(source, deftoken, define, firsttoken, lasttoken);
    if (define->numparms && !PC_ReadDefineParms(source, define, parms, 128))
        return 0;
    first = 0;
    last = 0;
    for (dt = define->tokens; dt; dt = dt->next)
    {
        parmnum = -1;
        if (dt->type == 4)
            parmnum = PC_FindDefineParm(define, dt->string);
        if (parmnum < 0)
        {
            if (dt->string[0] != 35 || dt->string[1])
            {
                ta = PC_CopyToken(dt);
            }
            else
            {
                if (dt->next)
                    parmnuma = PC_FindDefineParm(define, dt->next->string);
                else
                    parmnuma = -1;
                if (parmnuma < 0)
                {
                    SourceWarning(source, "stringizing operator without define parameter");
                    continue;
                }
                dt = dt->next;
                if (!PC_StringizeTokens(parms[parmnuma], &token))
                {
                    SourceError(source, "can't stringize tokens");
                    return 0;
                }
                ta = PC_CopyToken(&token);
            }
            ta->next = 0;
            if (last)
                last->next = ta;
            else
                first = ta;
            last = ta;
        }
        else
        {
            for (pt = parms[parmnum]; pt; pt = pt->next)
            {
                t = PC_CopyToken(pt);
                t->next = 0;
                if (last)
                    last->next = t;
                else
                    first = t;
                last = t;
            }
        }
    }
    tb = first;
    while (tb)
    {
        if (tb->next && tb->next->string[0] == 35 && tb->next->string[1] == 35 && (t2 = tb->next->next) != 0)
        {
            if (!PC_MergeTokens(tb, t2))
            {
                SourceError(source, "can't merge %s with %s", tb->string, t2->string);
                return 0;
            }
            PC_FreeToken(tb->next);
            tb->next = t2->next;
            if (t2 == last)
                last = tb;
            PC_FreeToken(t2);
        }
        else
        {
            tb = tb->next;
        }
    }
    *firsttoken = first;
    *lasttoken = last;
    for (i = 0; i < define->numparms; ++i)
    {
        for (pt = parms[i]; pt; pt = nextpt)
        {
            nextpt = pt->next;
            PC_FreeToken(pt);
        }
    }
    return 1;
}

void __cdecl PC_ConvertPath(char *path)
{
    char v1; // dl
    char *v2; // [esp+8h] [ebp-Ch]
    char *v3; // [esp+Ch] [ebp-8h]
    char *ptr; // [esp+10h] [ebp-4h]
    char *ptra; // [esp+10h] [ebp-4h]

    ptr = path;
    while (*ptr)
    {
        if ((*ptr == 92 || *ptr == 47) && (ptr[1] == 92 || ptr[1] == 47))
        {
            v3 = ptr + 1;
            v2 = ptr;
            do
            {
                v1 = *v3;
                *v2++ = *v3++;
            } while (v1);
        }
        else
        {
            ++ptr;
        }
    }
    for (ptra = path; *ptra; ++ptra)
    {
        if (*ptra == 47 || *ptra == 92)
            *ptra = 92;
    }
}

void __cdecl StripDoubleQuotes(char *string)
{
    char v1; // dl
    char *v2; // [esp+28h] [ebp-8h]
    char *v3; // [esp+2Ch] [ebp-4h]

    if (*string == 34)
    {
        v3 = string + 1;
        v2 = string;
        do
        {
            v1 = *v3;
            *v2++ = *v3++;
        } while (v1);
    }
    if (string[strlen(string) - 1] == 34)
        string[strlen(string) - 1] = 0;
}

unsigned __int8 *__cdecl GetClearedMemory(unsigned int size)
{
    unsigned __int8 *ptr; // [esp+0h] [ebp-4h]

    ptr = (unsigned __int8 *)GetMemory(size);
    memset(ptr, 0, size);
    return ptr;
}

void __cdecl PS_CreatePunctuationTable(script_s *script, punctuation_s *punctuations)
{
    punctuation_s *lastp; // [esp+20h] [ebp-10h]
    int i; // [esp+24h] [ebp-Ch]
    punctuation_s *newp; // [esp+28h] [ebp-8h]
    punctuation_s *p; // [esp+2Ch] [ebp-4h]

    if (!script->punctuationtable)
        script->punctuationtable = (punctuation_s **)GetMemory(0x400u);
    memset((unsigned __int8 *)script->punctuationtable, 0, 0x400u);
    for (i = 0; punctuations[i].p; ++i)
    {
        newp = &punctuations[i];
        lastp = 0;
        for (p = script->punctuationtable[*newp->p]; p; p = p->next)
        {
            if (strlen(p->p) < strlen(newp->p))
            {
                newp->next = p;
                if (lastp)
                    lastp->next = newp;
                else
                    script->punctuationtable[*newp->p] = newp;
                break;
            }
            lastp = p;
        }
        if (!p)
        {
            newp->next = 0;
            if (lastp)
                lastp->next = newp;
            else
                script->punctuationtable[*newp->p] = newp;
        }
    }
}

void __cdecl SetScriptPunctuations(script_s *script)
{
    PS_CreatePunctuationTable(script, default_punctuations);
    script->punctuations = default_punctuations;
}

script_s *__cdecl LoadScriptFile(const char *filename)
{
    char v2; // [esp+3h] [ebp-61h]
    script_s *v3; // [esp+8h] [ebp-5Ch]
    const char *v4; // [esp+Ch] [ebp-58h]
    script_s *buffer; // [esp+10h] [ebp-54h]
    int fp; // [esp+18h] [ebp-4Ch] BYREF
    char pathname[64]; // [esp+1Ch] [ebp-48h] BYREF
    int length; // [esp+60h] [ebp-4h]

    Com_sprintf(pathname, 0x40u, "%s", filename);
    length = FS_FOpenFileRead(pathname, &fp);
    if (!fp)
        return 0;
    buffer = (script_s *)GetClearedMemory(length + 1201);
    v4 = filename;
    v3 = buffer;
    do
    {
        v2 = *v4;
        v3->filename[0] = *v4++;
        v3 = (script_s *)((char *)v3 + 1);
    } while (v2);
    buffer->buffer = buffer[1].filename;
    buffer->buffer[length] = 0;
    buffer->length = length;
    buffer->script_p = buffer->buffer;
    buffer->lastscript_p = buffer->buffer;
    buffer->end_p = &buffer->buffer[length];
    buffer->tokenavailable = 0;
    buffer->line = 1;
    buffer->lastline = 1;
    SetScriptPunctuations(buffer);
    FS_Read((unsigned __int8 *)buffer->buffer, length, fp);
    FS_FCloseFile(fp);
    buffer->length = Com_Compress(buffer->buffer);
    return buffer;
}

int __cdecl PC_Directive_include(source_s *source)
{
    char v2; // [esp+1Bh] [ebp-4B9h]
    char *v3; // [esp+20h] [ebp-4B4h]
    char *v4; // [esp+24h] [ebp-4B0h]
    char v5; // [esp+2Bh] [ebp-4A9h]
    token_s *v6; // [esp+2Ch] [ebp-4A8h]
    char *v7; // [esp+40h] [ebp-494h]
    char v8; // [esp+47h] [ebp-48Dh]
    char *v9; // [esp+4Ch] [ebp-488h]
    char *includepath; // [esp+50h] [ebp-484h] BYREF
    char path[68]; // [esp+54h] [ebp-480h] BYREF
    script_s *script; // [esp+98h] [ebp-43Ch]
    token_s token; // [esp+9Ch] [ebp-438h] BYREF

    if (source->skip > 0)
        return 1;
    if (!PC_ReadSourceToken(source, &token) || token.linescrossed > 0)
        goto LABEL_4;
    if (token.type == 1)
    {
        StripDoubleQuotes(token.string);
        PC_ConvertPath(token.string);
        script = LoadScriptFile(token.string);
        if (!script)
        {
            includepath = source->includepath;
            v9 = path;
            do
            {
                v8 = *includepath;
                *v9++ = *includepath++;
            } while (v8);
            v7 = &token.string[strlen(token.string) + 1];
            v6 = (token_s *)((char *)&includepath + 3);
            do
            {
                v5 = v6->string[1];
                v6 = (token_s *)((char *)v6 + 1);
            } while (v5);
            memcpy(v6, &token, v7 - (char *)&token);
            script = LoadScriptFile(path);
        }
        goto LABEL_30;
    }
    if (token.type != 5 || token.string[0] != 60)
    {
    LABEL_4:
        SourceError(source, "#include without file name");
        return 0;
    }
    v4 = source->includepath;
    v3 = path;
    do
    {
        v2 = *v4;
        *v3++ = *v4++;
    } while (v2);
    while (PC_ReadSourceToken(source, &token))
    {
        if (token.linescrossed > 0)
        {
            PC_UnreadSourceToken(source, &token);
            break;
        }
        if (token.type == 5 && token.string[0] == 62)
            break;
        strncat(path, token.string, 0x40u);
    }
    if (token.string[0] != 62)
        SourceWarning(source, "#include missing trailing >");
    if (&path[strlen(path) + 1] == &path[1])
    {
        SourceError(source, "#include without file name between < >");
        return 0;
    }
    PC_ConvertPath(path);
    script = LoadScriptFile(path);
LABEL_30:
    if (script)
    {
        PC_PushScript(source, script);
        return 1;
    }
    else
    {
        SourceError(source, "file %s not found", path);
        return 0;
    }
}

bool __cdecl PC_WhiteSpaceBeforeToken(token_s *token)
{
    return token->endwhitespace_p - token->whitespace_p > 0;
}

void __cdecl PC_ClearTokenWhiteSpace(token_s *token)
{
    token->whitespace_p = 0;
    token->endwhitespace_p = 0;
    token->linescrossed = 0;
}

int __cdecl PC_Directive_undef(source_s *source)
{
    define_s *lastdefine; // [esp+14h] [ebp-444h]
    int hash; // [esp+18h] [ebp-440h]
    define_s *define; // [esp+1Ch] [ebp-43Ch]
    token_s token; // [esp+20h] [ebp-438h] BYREF

    if (source->skip > 0)
        return 1;
    if (PC_ReadLine(source, &token, 0))
    {
        if (token.type == 4)
        {
            hash = PC_NameHash(token.string);
            lastdefine = 0;
            for (define = source->definehash[hash]; define; define = define->hashnext)
            {
                if (!strcmp(define->name, token.string))
                {
                    if ((define->flags & 1) != 0)
                    {
                        SourceWarning(source, "can't undef %s", token.string);
                    }
                    else
                    {
                        if (lastdefine)
                            lastdefine->hashnext = define->hashnext;
                        else
                            source->definehash[hash] = define->hashnext;
                        PC_FreeDefine(define);
                    }
                    return 1;
                }
                lastdefine = define;
            }
            return 1;
        }
        else
        {
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "expected name, found %s", token.string);
            return 0;
        }
    }
    else
    {
        SourceError(source, "undef without name");
        return 0;
    }
}

int __cdecl PC_ReadLine(source_s *source, token_s *token, bool expandDefines)
{
    int crossline; // [esp+14h] [ebp-8h]
    define_s *define; // [esp+18h] [ebp-4h]

    for (crossline = 0; ; crossline = 1)
    {
        while (1)
        {
            if (!PC_ReadSourceToken(source, token))
                return 0;
            if (token->linescrossed > crossline)
            {
                PC_UnreadSourceToken(source, token);
                return 0;
            }
            if (token->type != 4)
                break;
            if (!expandDefines)
                break;
            define = PC_FindHashedDefine(source->definehash, token->string);
            if (!define)
                break;
            if (!PC_ExpandDefineIntoSource(source, token, define))
                return 0;
        }
        if (strcmp(token->string, "\\"))
            break;
    }
    return 1;
}

int __cdecl PC_ExpandDefineIntoSource(source_s *source, token_s *deftoken, define_s *define)
{
    token_s *firsttoken; // [esp+0h] [ebp-8h] BYREF
    token_s *lasttoken; // [esp+4h] [ebp-4h] BYREF

    if (!PC_ExpandDefine(source, deftoken, define, &firsttoken, &lasttoken))
        return 0;
    if (firsttoken)
    {
        if (lasttoken)
        {
            lasttoken->next = source->tokens;
            source->tokens = firsttoken;
        }
    }
    return 1;
}

int __cdecl PC_Directive_define(source_s *source)
{
    char v2; // [esp+7Bh] [ebp-45Dh]
    char *name; // [esp+80h] [ebp-458h]
    token_s *p_token; // [esp+84h] [ebp-454h]
    token_s *t; // [esp+98h] [ebp-440h]
    token_s *ta; // [esp+98h] [ebp-440h]
    define_s *define; // [esp+9Ch] [ebp-43Ch]
    define_s *definea; // [esp+9Ch] [ebp-43Ch]
    token_s token; // [esp+A0h] [ebp-438h] BYREF
    token_s *last; // [esp+4D4h] [ebp-4h]

    if (source->skip > 0)
        return 1;
    if (!PC_ReadLine(source, &token, 0))
    {
        SourceError(source, "#define without name");
        return 0;
    }
    if (token.type != 4)
    {
        PC_UnreadSourceToken(source, &token);
        SourceError(source, "expected name after #define, found %s", token.string);
        return 0;
    }
    define = PC_FindHashedDefine(source->definehash, token.string);
    if (define)
    {
        if ((define->flags & 1) != 0)
        {
            SourceError(source, "can't redefine %s", token.string);
            return 0;
        }
        SourceWarning(source, "redefinition of %s", token.string);
        PC_UnreadSourceToken(source, &token);
        if (!PC_Directive_undef(source))
            return 0;
        PC_FindHashedDefine(source->definehash, token.string);
    }
    definea = (define_s *)GetMemory(&token.string[strlen(token.string) + 1] - &token.string[1] + 33);
    definea->name = 0;
    definea->flags = 0;
    definea->builtin = 0;
    definea->numparms = 0;
    definea->parms = 0;
    definea->tokens = 0;
    definea->next = 0;
    definea->hashnext = 0;
    definea->name = (char *)&definea[1];
    p_token = &token;
    name = definea->name;
    do
    {
        v2 = p_token->string[0];
        *name = p_token->string[0];
        p_token = (token_s *)((char *)p_token + 1);
        ++name;
    } while (v2);
    PC_AddDefineToHash(definea, source->definehash);
    if (!PC_ReadLine(source, &token, 0))
        return 1;
    if (PC_WhiteSpaceBeforeToken(&token) || strcmp(token.string, "("))
    {
    LABEL_37:
        last = 0;
        do
        {
            ta = PC_CopyToken(&token);
            if (ta->type == 4 && !strcmp(ta->string, definea->name))
            {
                SourceError(source, "recursive define (removed recursion)");
            }
            else
            {
                PC_ClearTokenWhiteSpace(ta);
                ta->next = 0;
                if (last)
                    last->next = ta;
                else
                    definea->tokens = ta;
                last = ta;
            }
        } while (PC_ReadLine(source, &token, 0));
        if (!last || strcmp(definea->tokens->string, "##") && strcmp(last->string, "##"))
            return 1;
        SourceError(source, "define with misplaced ##");
        return 0;
    }
    last = 0;
    if (PC_CheckTokenString(source, ")"))
    {
    LABEL_35:
        if (!PC_ReadLine(source, &token, 0))
            return 1;
        goto LABEL_37;
    }
    do
    {
        if (!PC_ReadLine(source, &token, 0))
        {
            SourceError(source, "expected define parameter");
            return 0;
        }
        if (token.type != 4)
        {
            SourceError(source, "invalid define parameter");
            return 0;
        }
        if (PC_FindDefineParm(definea, token.string) >= 0)
        {
            SourceError(source, "two of the same define parameters");
            return 0;
        }
        t = PC_CopyToken(&token);
        PC_ClearTokenWhiteSpace(t);
        t->next = 0;
        if (last)
            last->next = t;
        else
            definea->parms = t;
        last = t;
        ++definea->numparms;
        if (!PC_ReadLine(source, &token, 0))
        {
            SourceError(source, "define parameters not terminated");
            return 0;
        }
        if (!strcmp(token.string, ")"))
            goto LABEL_35;
    } while (!strcmp(token.string, ","));
    SourceError(source, "define not terminated");
    return 0;
}

script_s *__cdecl LoadScriptMemory(char *ptr, int length, const char *name)
{
    char v4; // [esp+3h] [ebp-15h]
    script_s *v5; // [esp+8h] [ebp-10h]
    script_s *buffer; // [esp+10h] [ebp-8h]

    buffer = (script_s *)GetClearedMemory(length + 1201);
    v5 = buffer;
    do
    {
        v4 = *name;
        v5->filename[0] = *name++;
        v5 = (script_s *)((char *)v5 + 1);
    } while (v4);
    buffer->buffer = buffer[1].filename;
    buffer->buffer[length] = 0;
    buffer->length = length;
    buffer->script_p = buffer->buffer;
    buffer->lastscript_p = buffer->buffer;
    buffer->end_p = &buffer->buffer[length];
    buffer->tokenavailable = 0;
    buffer->line = 1;
    buffer->lastline = 1;
    SetScriptPunctuations(buffer);
    memcpy((unsigned __int8 *)buffer->buffer, (unsigned __int8 *)ptr, length);
    return buffer;
}

define_s *__cdecl PC_DefineFromString(char *string)
{
    source_s src; // [esp+10h] [ebp-4E8h] BYREF
    token_s *t; // [esp+4E4h] [ebp-14h]
    define_s *def; // [esp+4E8h] [ebp-10h]
    script_s *script; // [esp+4ECh] [ebp-Ch]
    int i; // [esp+4F0h] [ebp-8h]
    int res; // [esp+4F4h] [ebp-4h]

    script = LoadScriptMemory(string, strlen(string), "*extern");
    memset((unsigned __int8 *)&src, 0, sizeof(src));
    strncpy((unsigned __int8 *)&src, (unsigned __int8 *)"*extern", 0x40u);
    src.scriptstack = script;
    src.definehash = (define_s **)GetClearedMemory(0x1000u);
    res = PC_Directive_define(&src);
    for (t = src.tokens; t; t = src.tokens)
    {
        src.tokens = src.tokens->next;
        PC_FreeToken(t);
    }
    def = 0;
    for (i = 0; i < 1024; ++i)
    {
        if (src.definehash[i])
        {
            def = src.definehash[i];
            break;
        }
    }
    FreeMemory((char *)src.definehash);
    FreeScript(script);
    if (res > 0)
        return def;
    if (src.defines)
        PC_FreeDefine(def);
    return 0;
}

int __cdecl PC_AddDefine(source_s *source, char *string)
{
    define_s *define; // [esp+0h] [ebp-4h]

    define = PC_DefineFromString(string);
    if (!define)
        return 0;
    PC_AddDefineToHash(define, source->definehash);
    return 1;
}

define_s *__cdecl PC_CopyDefine(source_s *source, define_s *define)
{
    char v2; // dl
    _BYTE *v4; // [esp+8h] [ebp-28h]
    char *name; // [esp+Ch] [ebp-24h]
    unsigned int *newdefine; // [esp+20h] [ebp-10h]
    token_s *newtoken; // [esp+24h] [ebp-Ch]
    token_s *newtokena; // [esp+24h] [ebp-Ch]
    token_s *token; // [esp+28h] [ebp-8h]
    token_s *tokena; // [esp+28h] [ebp-8h]
    token_s *lasttoken; // [esp+2Ch] [ebp-4h]
    token_s *lasttokena; // [esp+2Ch] [ebp-4h]

    newdefine = GetMemory(strlen(define->name) + 33);
    *newdefine = (unsigned int)(newdefine + 8);
    name = define->name;
    v4 = (_BYTE *)*newdefine;
    do
    {
        v2 = *name;
        *v4++ = *name++;
    } while (v2);
    newdefine[1] = define->flags;
    newdefine[2] = define->builtin;
    newdefine[3] = define->numparms;
    newdefine[6] = 0;
    newdefine[7] = 0;
    newdefine[5] = 0;
    lasttoken = 0;
    for (token = define->tokens; token; token = token->next)
    {
        newtoken = PC_CopyToken(token);
        newtoken->next = 0;
        if (lasttoken)
            lasttoken->next = newtoken;
        else
            newdefine[5] = (unsigned int)newtoken;
        lasttoken = newtoken;
    }
    newdefine[4] = 0;
    lasttokena = 0;
    for (tokena = define->parms; tokena; tokena = tokena->next)
    {
        newtokena = PC_CopyToken(tokena);
        newtokena->next = 0;
        if (lasttokena)
            lasttokena->next = newtokena;
        else
            newdefine[4] = (unsigned int)newtokena;
        lasttokena = newtokena;
    }
    return (define_s *)newdefine;
}

define_s *globaldefines;
void __cdecl PC_AddGlobalDefinesToSource(source_s *source)
{
    define_s *newdefine; // [esp+0h] [ebp-8h]
    define_s *define; // [esp+4h] [ebp-4h]

    for (define = globaldefines; define; define = define->next)
    {
        newdefine = PC_CopyDefine(source, define);
        PC_AddDefineToHash(newdefine, source->definehash);
    }
}

int __cdecl PC_Directive_if_def(source_s *source, int type)
{
    define_s *d; // [esp+0h] [ebp-440h]
    token_s token; // [esp+8h] [ebp-438h] BYREF

    if (PC_ReadLine(source, &token, 0))
    {
        if (token.type == 4)
        {
            d = PC_FindHashedDefine(source->definehash, token.string);
            PC_PushIndent(source, type, (parseSkip_t)((type == 8) == (d == 0)));
            return 1;
        }
        else
        {
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "expected name after #ifdef, found %s", token.string);
            return 0;
        }
    }
    else
    {
        SourceError(source, "#ifdef without name");
        return 0;
    }
}

void __cdecl PC_PushIndent(source_s *source, int type, parseSkip_t skip)
{
    indent_s *indent; // [esp+0h] [ebp-4h]

    indent = (indent_s *)GetMemory(0x10u);
    indent->type = type;
    indent->script = source->scriptstack;
    indent->skip = skip;
    source->skip += indent->skip != SKIP_NO;
    indent->next = source->indentstack;
    source->indentstack = indent;
}

int __cdecl PC_Directive_ifdef(source_s *source)
{
    return PC_Directive_if_def(source, 8);
}

int __cdecl PC_Directive_ifndef(source_s *source)
{
    return PC_Directive_if_def(source, 16);
}

int __cdecl PC_Directive_else(source_s *source)
{
    parseSkip_t skip; // [esp+0h] [ebp-8h] BYREF
    int type; // [esp+4h] [ebp-4h] BYREF

    PC_PopIndent(source, &type, &skip);
    if (type)
    {
        if (type == 2)
        {
            SourceError(source, "#else after #else");
            return 0;
        }
        else
        {
            PC_PushIndent(source, 2, (parseSkip_t)(skip != SKIP_YES));
            return 1;
        }
    }
    else
    {
        SourceError(source, "misplaced #else");
        return 0;
    }
}

int __cdecl PC_Directive_endif(source_s *source)
{
    parseSkip_t skip; // [esp+0h] [ebp-8h] BYREF
    int type; // [esp+4h] [ebp-4h] BYREF

    PC_PopIndent(source, &type, &skip);
    if (type)
        return 1;
    SourceError(source, "misplaced #endif");
    return 0;
}

int __cdecl PC_OperatorPriority(int op)
{
    int result; // eax

    switch (op)
    {
    case 5:
        result = 7;
        break;
    case 6:
        result = 6;
        break;
    case 7:
        result = 12;
        break;
    case 8:
        result = 12;
        break;
    case 9:
        result = 11;
        break;
    case 10:
        result = 11;
        break;
    case 21:
        result = 13;
        break;
    case 22:
        result = 13;
        break;
    case 26:
        result = 15;
        break;
    case 27:
        result = 15;
        break;
    case 28:
        result = 15;
        break;
    case 29:
        result = 14;
        break;
    case 30:
        result = 14;
        break;
    case 32:
        result = 10;
        break;
    case 33:
        result = 8;
        break;
    case 34:
        result = 9;
        break;
    case 35:
        result = 16;
        break;
    case 36:
        result = 16;
        break;
    case 37:
        result = 12;
        break;
    case 38:
        result = 12;
        break;
    case 42:
        result = 5;
        break;
    case 43:
        result = 5;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

int __cdecl PC_EvaluateTokens(source_s *source, token_s *tokens, int *intvalue, double *floatvalue, int integer)
{
    long double v5; // st7
    bool v7; // [esp+18h] [ebp-DC0h]
    bool v8; // [esp+1Ch] [ebp-DBCh]
    bool v9; // [esp+20h] [ebp-DB8h]
    bool v10; // [esp+24h] [ebp-DB4h]
    int type; // [esp+74h] [ebp-D64h]
    operator_s *o; // [esp+78h] [ebp-D60h]
    operator_s *oa; // [esp+78h] [ebp-D60h]
    operator_s *ob; // [esp+78h] [ebp-D60h]
    double questmarkfloatvalue; // [esp+80h] [ebp-D58h]
    operator_s *lastoperator; // [esp+8Ch] [ebp-D4Ch]
    value_s value_heap[64]; // [esp+90h] [ebp-D48h] BYREF
    operator_s *firstoperator; // [esp+894h] [ebp-544h]
    int lastwasvalue; // [esp+898h] [ebp-540h]
    value_s *v2; // [esp+89Ch] [ebp-53Ch]
    int error; // [esp+8A0h] [ebp-538h]
    int negativevalue; // [esp+8A4h] [ebp-534h]
    operator_s operator_heap[64]; // [esp+8A8h] [ebp-530h] BYREF
    value_s *firstvalue; // [esp+DACh] [ebp-2Ch]
    value_s *v1; // [esp+DB0h] [ebp-28h]
    int numvalues; // [esp+DB4h] [ebp-24h]
    int parentheses; // [esp+DB8h] [ebp-20h]
    int numoperators; // [esp+DBCh] [ebp-1Ch]
    value_s *v; // [esp+DC0h] [ebp-18h]
    value_s *lastvalue; // [esp+DC4h] [ebp-14h]
    int lastoperatortype; // [esp+DC8h] [ebp-10h]
    int brace; // [esp+DCCh] [ebp-Ch]
    int questmarkintvalue; // [esp+DD0h] [ebp-8h]
    int gotquestmarkvalue; // [esp+DD4h] [ebp-4h]

    brace = 0;
    parentheses = 0;
    error = 0;
    lastwasvalue = 0;
    negativevalue = 0;
    questmarkintvalue = 0;
    gotquestmarkvalue = 0;
    lastoperatortype = 0;
    numoperators = 0;
    numvalues = 0;
    lastoperator = 0;
    firstoperator = 0;
    lastvalue = 0;
    firstvalue = 0;
    if (intvalue)
        *intvalue = 0;
    if (floatvalue)
        *floatvalue = 0.0;
    while (tokens)
    {
        type = tokens->type;
        switch (type)
        {
        case 3:
            if (lastwasvalue)
                goto LABEL_37;
            if (numvalues >= 64)
            {
            LABEL_24:
                SourceError(source, "out of value space\n");
                error = 1;
                break;
            }
            v = &value_heap[numvalues++];
            if (negativevalue)
            {
                v->intvalue = -(int)tokens->intvalue;
                v5 = -tokens->floatvalue;
            }
            else
            {
                v->intvalue = tokens->intvalue;
                v5 = tokens->floatvalue;
            }
            v->floatvalue = v5;
            v->parentheses = parentheses;
            v->next = 0;
            v->prev = lastvalue;
            if (lastvalue)
                lastvalue->next = v;
            else
                firstvalue = v;
            lastvalue = v;
            lastwasvalue = 1;
            negativevalue = 0;
            break;
        case 4:
            if (lastwasvalue || negativevalue)
            {
            LABEL_37:
                SourceError(source, "syntax error in #if/#elif");
                error = 1;
                break;
            }
            if (strcmp(tokens->string, "defined"))
            {
                SourceError(source, "undefined name %s in #if/#elif", tokens->string);
                error = 1;
                break;
            }
            tokens = tokens->next;
            if (!strcmp(tokens->string, "("))
            {
                brace = 1;
                tokens = tokens->next;
            }
            if (tokens && tokens->type == 4)
            {
                if (numvalues >= 64)
                    goto LABEL_24;
                v = &value_heap[numvalues++];
                if (PC_FindHashedDefine(source->definehash, tokens->string))
                {
                    v->intvalue = 1;
                    v->floatvalue = 1.0;
                }
                else
                {
                    v->intvalue = 0;
                    v->floatvalue = 0.0;
                }
                v->parentheses = parentheses;
                v->next = 0;
                v->prev = lastvalue;
                if (lastvalue)
                    lastvalue->next = v;
                else
                    firstvalue = v;
                lastvalue = v;
                if (!brace || (tokens = tokens->next) != 0 && !strcmp(tokens->string, ")"))
                {
                    brace = 0;
                    lastwasvalue = 1;
                }
                else
                {
                    SourceError(source, "defined without ) in #if/#elif");
                    error = 1;
                }
            }
            else
            {
                if (tokens)
                    SourceError(source, "defined without name in #if/#elif; got %s", tokens->string);
                else
                    SourceError(source, "defined without name in #if/#elif; got %s", "end-of-file");
                error = 1;
            }
            break;
        case 5:
            if (negativevalue)
            {
                SourceError(source, "misplaced minus sign in #if/#elif");
                error = 1;
            }
            else if (tokens->subtype == 44)
            {
                ++parentheses;
            }
            else if (tokens->subtype == 45)
            {
                if (--parentheses < 0)
                {
                    SourceError(source, "too many ) in #if/#elsif");
                    error = 1;
                }
            }
            else if (!integer
                && (tokens->subtype == 35
                    || tokens->subtype == 28
                    || tokens->subtype == 21
                    || tokens->subtype == 22
                    || tokens->subtype == 32
                    || tokens->subtype == 33
                    || tokens->subtype == 34))
            {
                SourceError(source, "illigal operator %s on floating point operands\n", tokens->string);
                error = 1;
            }
            else
            {
                switch (tokens->subtype)
                {
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 0xA:
                case 0x15:
                case 0x16:
                case 0x1A:
                case 0x1B:
                case 0x1C:
                case 0x1D:
                case 0x20:
                case 0x21:
                case 0x22:
                case 0x25:
                case 0x26:
                case 0x2A:
                case 0x2B:
                    goto $LN82;
                case 0x10:
                case 0x11:
                    SourceError(source, "++ or -- used in #if/#elif");
                    break;
                case 0x1E:
                    if (lastwasvalue)
                    {
                    $LN82:
                        if (!lastwasvalue)
                        {
                            SourceError(source, "operator %s after operator in #if/#elif", tokens->string);
                            error = 1;
                        }
                    }
                    else
                    {
                        negativevalue = 1;
                    }
                    break;
                case 0x23:
                case 0x24:
                    if (lastwasvalue)
                    {
                        SourceError(source, "! or ~ after value in #if/#elif");
                        error = 1;
                    }
                    break;
                default:
                    SourceError(source, "invalid operator %s in #if/#elif", tokens->string);
                    error = 1;
                    break;
                }
                if (!error && !negativevalue)
                {
                    if (numoperators < 64)
                    {
                        o = &operator_heap[numoperators++];
                        o->op = tokens->subtype;
                        o->priority = PC_OperatorPriority(tokens->subtype);
                        o->parentheses = parentheses;
                        o->next = 0;
                        o->prev = lastoperator;
                        if (lastoperator)
                            lastoperator->next = o;
                        else
                            firstoperator = o;
                        lastoperator = o;
                        lastwasvalue = 0;
                    }
                    else
                    {
                        SourceError(source, "out of operator space\n");
                        error = 1;
                    }
                }
            }
            break;
        default:
            SourceError(source, "unknown %s in #if/#elif", tokens->string);
            error = 1;
            break;
        }
        if (error)
            break;
        tokens = tokens->next;
    }
    if (!error)
    {
        if (lastwasvalue)
        {
            if (parentheses)
            {
                SourceError(source, "too many ( in #if/#elif");
                error = 1;
            }
        }
        else
        {
            SourceError(source, "trailing operator in #if/#elif");
            error = 1;
        }
    }
    gotquestmarkvalue = 0;
    questmarkintvalue = 0;
    questmarkfloatvalue = 0.0;
    while (!error && firstoperator)
    {
        v = firstvalue;
        for (oa = firstoperator;
            oa->next
            && oa->parentheses <= oa->next->parentheses
            && (oa->parentheses != oa->next->parentheses || oa->priority < oa->next->priority);
            oa = oa->next)
        {
            if (oa->op != 36 && oa->op != 35)
                v = v->next;
            if (!v)
            {
                SourceError(source, "mising values in #if/#elif");
                error = 1;
                break;
            }
        }
        if (error)
            break;
        v1 = v;
        v2 = v->next;
        switch (oa->op)
        {
        case 5:
            v10 = v1->intvalue && v2->intvalue;
            v1->intvalue = v10;
            v9 = 0.0 != v1->floatvalue && 0.0 != v2->floatvalue;
            v1->floatvalue = (double)v9;
            break;
        case 6:
            v8 = v1->intvalue || v2->intvalue;
            v1->intvalue = v8;
            v7 = 0.0 != v1->floatvalue || 0.0 != v2->floatvalue;
            v1->floatvalue = (double)v7;
            break;
        case 7:
            v1->intvalue = v1->intvalue >= v2->intvalue;
            v1->floatvalue = (double)(v2->floatvalue <= v1->floatvalue);
            break;
        case 8:
            v1->intvalue = v1->intvalue <= v2->intvalue;
            v1->floatvalue = (double)(v2->floatvalue >= v1->floatvalue);
            break;
        case 9:
            v1->intvalue = v1->intvalue == v2->intvalue;
            v1->floatvalue = (double)(v2->floatvalue == v1->floatvalue);
            break;
        case 0xA:
            v1->intvalue = v1->intvalue != v2->intvalue;
            v1->floatvalue = (double)(v2->floatvalue != v1->floatvalue);
            break;
        case 0x15:
            v1->intvalue >>= v2->intvalue;
            break;
        case 0x16:
            v1->intvalue <<= v2->intvalue;
            break;
        case 0x1A:
            v1->intvalue *= v2->intvalue;
            v1->floatvalue = v1->floatvalue * v2->floatvalue;
            break;
        case 0x1B:
            if (!v2->intvalue || 0.0 == v2->floatvalue)
                goto LABEL_113;
            v1->intvalue /= v2->intvalue;
            v1->floatvalue = v1->floatvalue / v2->floatvalue;
            break;
        case 0x1C:
            if (v2->intvalue)
            {
                v1->intvalue %= v2->intvalue;
            }
            else
            {
            LABEL_113:
                SourceError(source, "divide by zero in #if/#elif\n");
                error = 1;
            }
            break;
        case 0x1D:
            v1->intvalue += v2->intvalue;
            v1->floatvalue = v1->floatvalue + v2->floatvalue;
            break;
        case 0x1E:
            v1->intvalue -= v2->intvalue;
            v1->floatvalue = v1->floatvalue - v2->floatvalue;
            break;
        case 0x20:
            v1->intvalue &= v2->intvalue;
            break;
        case 0x21:
            v1->intvalue |= v2->intvalue;
            break;
        case 0x22:
            v1->intvalue ^= v2->intvalue;
            break;
        case 0x23:
            v1->intvalue = ~v1->intvalue;
            break;
        case 0x24:
            v1->intvalue = v1->intvalue == 0;
            v1->floatvalue = (double)(0.0 == v1->floatvalue);
            break;
        case 0x25:
            v1->intvalue = v1->intvalue > v2->intvalue;
            v1->floatvalue = (double)(v2->floatvalue < v1->floatvalue);
            break;
        case 0x26:
            v1->intvalue = v1->intvalue < v2->intvalue;
            v1->floatvalue = (double)(v2->floatvalue > v1->floatvalue);
            break;
        case 0x2A:
            if (gotquestmarkvalue)
            {
                if (integer)
                {
                    if (!questmarkintvalue)
                        v1->intvalue = v2->intvalue;
                }
                else if (0.0 == questmarkfloatvalue)
                {
                    v1->floatvalue = v2->floatvalue;
                }
                gotquestmarkvalue = 0;
            }
            else
            {
                SourceError(source, ": without ? in #if/#elif");
                error = 1;
            }
            break;
        case 0x2B:
            if (gotquestmarkvalue)
            {
                SourceError(source, "? after ? in #if/#elif");
                error = 1;
            }
            else
            {
                questmarkintvalue = v1->intvalue;
                questmarkfloatvalue = v1->floatvalue;
                gotquestmarkvalue = 1;
            }
            break;
        default:
            break;
        }
        if (error)
            break;
        lastoperatortype = oa->op;
        if (oa->op != 36 && oa->op != 35)
        {
            if (oa->op != 43)
                v = v->next;
            if (v->prev)
                v->prev->next = v->next;
            else
                firstvalue = v->next;
            if (v->next)
                v->next->prev = v->prev;
            else
                lastvalue = v->prev;
        }
        if (oa->prev)
            oa->prev->next = oa->next;
        else
            firstoperator = oa->next;
        if (oa->next)
            oa->next->prev = oa->prev;
    }
    if (firstvalue)
    {
        if (intvalue)
            *intvalue = firstvalue->intvalue;
        if (floatvalue)
            *floatvalue = firstvalue->floatvalue;
    }
    for (ob = firstoperator; ob; ob = ob->next)
        ;
    for (v = firstvalue; v; v = lastvalue)
        lastvalue = v->next;
    if (!error)
        return 1;
    if (intvalue)
        *intvalue = 0;
    if (floatvalue)
        *floatvalue = 0.0;
    return 0;
}

int __cdecl PC_DollarEvaluate(source_s *source, int *intvalue, double *floatvalue, int integer)
{
    token_s *t; // [esp+14h] [ebp-454h]
    token_s *ta; // [esp+14h] [ebp-454h]
    token_s *tb; // [esp+14h] [ebp-454h]
    token_s *tc; // [esp+14h] [ebp-454h]
    define_s *define; // [esp+18h] [ebp-450h]
    int defined; // [esp+1Ch] [ebp-44Ch]
    token_s *nexttoken; // [esp+20h] [ebp-448h]
    int indent; // [esp+24h] [ebp-444h]
    token_s token; // [esp+28h] [ebp-440h] BYREF
    token_s *firsttoken; // [esp+460h] [ebp-8h]
    token_s *lasttoken; // [esp+464h] [ebp-4h]

    defined = 0;
    if (intvalue)
        *intvalue = 0;
    if (floatvalue)
        *floatvalue = 0.0;
    if (!PC_ReadSourceToken(source, &token))
    {
        SourceError(source, "no leading ( after $evalint/$evalfloat");
        return 0;
    }
    if (!PC_ReadSourceToken(source, &token))
    {
        SourceError(source, "nothing to evaluate");
        return 0;
    }
    indent = 1;
    firsttoken = 0;
    lasttoken = 0;
    do
    {
        if (token.type == 4)
        {
            if (defined)
            {
                defined = 0;
                t = PC_CopyToken(&token);
                t->next = 0;
                if (lasttoken)
                    lasttoken->next = t;
                else
                    firsttoken = t;
                lasttoken = t;
            }
            else if (!strcmp(token.string, "defined"))
            {
                defined = 1;
                ta = PC_CopyToken(&token);
                ta->next = 0;
                if (lasttoken)
                    lasttoken->next = ta;
                else
                    firsttoken = ta;
                lasttoken = ta;
            }
            else
            {
                define = PC_FindHashedDefine(source->definehash, token.string);
                if (!define)
                {
                    SourceError(source, "can't evaluate %s, not defined", token.string);
                    return 0;
                }
                if (!PC_ExpandDefineIntoSource(source, &token, define))
                    return 0;
            }
            continue;
        }
        if (token.type != 3 && token.type != 5)
        {
            SourceError(source, "can't evaluate %s", token.string);
            return 0;
        }
        if (token.string[0] == 40)
        {
            ++indent;
        }
        else if (token.string[0] == 41)
        {
            --indent;
        }
        if (indent <= 0)
            break;
        tb = PC_CopyToken(&token);
        tb->next = 0;
        if (lasttoken)
            lasttoken->next = tb;
        else
            firsttoken = tb;
        lasttoken = tb;
    } while (PC_ReadSourceToken(source, &token));
    if (!PC_EvaluateTokens(source, firsttoken, intvalue, floatvalue, integer))
        return 0;
    for (tc = firsttoken; tc; tc = nexttoken)
    {
        nexttoken = tc->next;
        PC_FreeToken(tc);
    }
    return 1;
}

int __cdecl PC_Directive_elif(source_s *source)
{
    parseSkip_t skip; // [esp+0h] [ebp-Ch] BYREF
    int type; // [esp+4h] [ebp-8h] BYREF
    int value; // [esp+8h] [ebp-4h] BYREF

    PC_PopIndent(source, &type, &skip);
    if (type && type != 2)
    {
        if (PC_Evaluate(source, &value, 0, 1))
        {
            if (skip == SKIP_YES)
                skip = (parseSkip_t)(value == 0);
            else
                skip = SKIP_ALL_ELIFS;
            PC_PushIndent(source, 4, skip);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        SourceError(source, "misplaced #elif");
        return 0;
    }
}

int __cdecl PC_Evaluate(source_s *source, int *intvalue, double *floatvalue, int integer)
{
    token_s *t; // [esp+14h] [ebp-450h]
    token_s *ta; // [esp+14h] [ebp-450h]
    token_s *tb; // [esp+14h] [ebp-450h]
    token_s *tc; // [esp+14h] [ebp-450h]
    define_s *define; // [esp+18h] [ebp-44Ch]
    token_s *nexttoken; // [esp+1Ch] [ebp-448h]
    int defined; // [esp+20h] [ebp-444h]
    token_s token; // [esp+24h] [ebp-440h] BYREF
    token_s *firsttoken; // [esp+45Ch] [ebp-8h]
    token_s *lasttoken; // [esp+460h] [ebp-4h]

    defined = 0;
    if (intvalue)
        *intvalue = 0;
    if (floatvalue)
        *floatvalue = 0.0;
    if (PC_ReadLine(source, &token, 1))
    {
        firsttoken = 0;
        lasttoken = 0;
        do
        {
            if (token.type == 4)
            {
                if (defined)
                {
                    defined = 0;
                    t = PC_CopyToken(&token);
                    t->next = 0;
                    if (lasttoken)
                        lasttoken->next = t;
                    else
                        firsttoken = t;
                    lasttoken = t;
                }
                else if (!strcmp(token.string, "defined"))
                {
                    defined = 1;
                    ta = PC_CopyToken(&token);
                    ta->next = 0;
                    if (lasttoken)
                        lasttoken->next = ta;
                    else
                        firsttoken = ta;
                    lasttoken = ta;
                }
                else
                {
                    define = PC_FindHashedDefine(source->definehash, token.string);
                    if (!define)
                    {
                        SourceError(source, "can't evaluate %s, not defined", token.string);
                        return 0;
                    }
                    if (!PC_ExpandDefineIntoSource(source, &token, define))
                        return 0;
                }
            }
            else
            {
                if (token.type != 3 && token.type != 5)
                {
                    SourceError(source, "can't evaluate %s", token.string);
                    return 0;
                }
                tb = PC_CopyToken(&token);
                tb->next = 0;
                if (lasttoken)
                    lasttoken->next = tb;
                else
                    firsttoken = tb;
                lasttoken = tb;
            }
        } while (PC_ReadLine(source, &token, defined == 0));
        if (PC_EvaluateTokens(source, firsttoken, intvalue, floatvalue, integer))
        {
            for (tc = firsttoken; tc; tc = nexttoken)
            {
                nexttoken = tc->next;
                PC_FreeToken(tc);
            }
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        SourceError(source, "no value after #if/#elif");
        return 0;
    }
}

int __cdecl PC_Directive_if(source_s *source)
{
    int value; // [esp+4h] [ebp-4h] BYREF

    if (!PC_Evaluate(source, &value, 0, 1))
        return 0;
    PC_PushIndent(source, 1, (parseSkip_t)(value == 0));
    return 1;
}

int __cdecl PC_Directive_line(source_s *source)
{
    SourceError(source, "#line directive not supported");
    return 0;
}

int __cdecl PC_Directive_error(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF

    token.string[0] = 0;
    PC_ReadSourceToken(source, &token);
    if (source->skip)
        return 1;
    SourceError(source, "#error directive: %s", token.string);
    return 0;
}

int __cdecl PC_Directive_pragma(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF

    SourceWarning(source, "#pragma directive not supported");
    while (PC_ReadLine(source, &token, 0))
        ;
    return 1;
}

void __cdecl UnreadSignToken(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF

    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    strcpy(token.string, "-");
    token.type = 5;
    token.subtype = 30;
    PC_UnreadSourceToken(source, &token);
}

int __cdecl PC_Directive_eval(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF
    int value; // [esp+434h] [ebp-4h] BYREF

    if (!PC_Evaluate(source, &value, 0, 1))
        return 0;
    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    sprintf(token.string, "%d", abs(value));
    token.type = 3;
    token.subtype = 12296;
    PC_UnreadSourceToken(source, &token);
    if (value < 0)
        UnreadSignToken(source);
    return 1;
}

int __cdecl PC_Directive_evalfloat(source_s *source)
{
    float v2; // [esp+8h] [ebp-448h]
    float v3; // [esp+Ch] [ebp-444h]
    token_s token; // [esp+10h] [ebp-440h] BYREF
    double value; // [esp+448h] [ebp-8h] BYREF

    if (!PC_Evaluate(source, 0, &value, 0))
        return 0;
    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    v3 = value;
    v2 = fabs(v3);
    sprintf(token.string, "%1.2f", v2);
    token.type = 3;
    token.subtype = 10248;
    PC_UnreadSourceToken(source, &token);
    if (value < 0.0)
        UnreadSignToken(source);
    return 1;
}

int __cdecl PC_ReadDirective(source_s *source)
{
    token_s token; // [esp+14h] [ebp-438h] BYREF
    int i; // [esp+448h] [ebp-4h]

    if (PC_ReadSourceToken(source, &token))
    {
        if (token.linescrossed <= 0)
        {
            if (token.type == 4)
            {
                for (i = 0; directives[i].name; ++i)
                {
                    if (!strcmp(directives[i].name, token.string))
                        return directives[i].func(source);
                }
            }
            SourceError(source, "unknown precompiler directive %s", token.string);
            return 0;
        }
        else
        {
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "found # at end of line");
            return 0;
        }
    }
    else
    {
        SourceError(source, "found # without name");
        return 0;
    }
}

int __cdecl PC_DollarDirective_evalint(source_s *source)
{
    token_s token; // [esp+0h] [ebp-438h] BYREF
    int value; // [esp+434h] [ebp-4h] BYREF

    if (!PC_DollarEvaluate(source, &value, 0, 1))
        return 0;
    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    sprintf(token.string, "%d", abs(value));
    token.type = 3;
    token.subtype = 12296;
    token.intvalue = value;
    token.floatvalue = (double)value;
    PC_UnreadSourceToken(source, &token);
    if (value < 0)
        UnreadSignToken(source);
    return 1;
}

int __cdecl PC_DollarDirective_evalfloat(source_s *source)
{
    float v2; // [esp+18h] [ebp-448h]
    float v3; // [esp+1Ch] [ebp-444h]
    token_s token; // [esp+20h] [ebp-440h] BYREF
    double value; // [esp+458h] [ebp-8h] BYREF

    if (!PC_DollarEvaluate(source, 0, &value, 0))
        return 0;
    token.line = source->scriptstack->line;
    token.whitespace_p = source->scriptstack->script_p;
    token.endwhitespace_p = source->scriptstack->script_p;
    token.linescrossed = 0;
    v3 = value;
    v2 = fabs(v3);
    sprintf(token.string, "%1.2f", v2);
    token.type = 3;
    token.subtype = 10248;
    token.intvalue = (__int64)value;
    token.floatvalue = value;
    PC_UnreadSourceToken(source, &token);
    if (value < 0.0)
        UnreadSignToken(source);
    return 1;
}

int __cdecl PC_ReadDollarDirective(source_s *source)
{
    token_s token; // [esp+14h] [ebp-438h] BYREF
    int i; // [esp+448h] [ebp-4h]

    if (PC_ReadSourceToken(source, &token))
    {
        if (token.linescrossed <= 0)
        {
            if (token.type == 4)
            {
                for (i = 0; dollardirectives[i].name; ++i)
                {
                    if (!strcmp(dollardirectives[i].name, token.string))
                        return dollardirectives[i].func(source);
                }
            }
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "unknown precompiler directive %s", token.string);
            return 0;
        }
        else
        {
            PC_UnreadSourceToken(source, &token);
            SourceError(source, "found $ at end of line");
            return 0;
        }
    }
    else
    {
        SourceError(source, "found $ without name");
        return 0;
    }
}

int __cdecl PC_ReadToken(source_s *source, token_s *token)
{
    token_s newtoken; // [esp+54h] [ebp-438h] BYREF
    define_s *define; // [esp+488h] [ebp-4h]

    while (1)
    {
        do
        {
            while (1)
            {
                while (1)
                {
                    if (!PC_ReadSourceToken(source, token))
                        return 0;
                    if (token->type != 5 || token->string[0] != 35 || token->string[1])
                        break;
                    if (!PC_ReadDirective(source))
                        return 0;
                }
                if (token->type != 5 || token->string[0] != 36)
                    break;
                if (!PC_ReadDollarDirective(source))
                    return 0;
            }
        } while (source->skip);
        if (token->type == 1 && PC_ReadToken(source, &newtoken))
        {
            if (newtoken.type == 1)
            {
                token->string[strlen(token->string) - 1] = 0;
                if (strlen(token->string) + &newtoken.string[strlen(&newtoken.string[1]) + 2] - &newtoken.string[2] + 1 >= 0x400)
                {
                    SourceError(source, "string longer than MAX_TOKEN %d\n", 1024);
                    return 0;
                }
                memcpy(
                    &token->string[strlen(token->string)],
                    &newtoken.string[1],
                    &newtoken.string[strlen(&newtoken.string[1]) + 2] - &newtoken.string[1]);
            }
            else
            {
                PC_UnreadToken(source, &newtoken);
            }
        }
        if (token->type != 4)
            break;
        define = PC_FindHashedDefine(source->definehash, token->string);
        if (!define)
            break;
        if (!PC_ExpandDefineIntoSource(source, token, define))
            return 0;
    }
    memcpy(&source->token, token, sizeof(source->token));
    return 1;
}

int __cdecl PC_CheckTokenString(source_s *source, const char *string)
{
    token_s tok; // [esp+14h] [ebp-438h] BYREF

    if (!PC_ReadToken(source, &tok))
        return 0;
    if (!strcmp(tok.string, string))
        return 1;
    PC_UnreadSourceToken(source, &tok);
    return 0;
}

void __cdecl PC_UnreadToken(source_s *source, token_s *token)
{
    PC_UnreadSourceToken(source, token);
}

source_s *__cdecl LoadSourceFile(char *filename)
{
    source_s *source; // [esp+0h] [ebp-8h]
    script_s *script; // [esp+4h] [ebp-4h]

    script = LoadScriptFile(filename);
    if (!script)
        return 0;
    script->next = 0;
    source = (source_s *)GetMemory(0x4D0u);
    memset((unsigned __int8 *)source, 0, sizeof(source_s));
    strncpy((unsigned __int8 *)source, (unsigned __int8 *)filename, 0x40u);
    source->scriptstack = script;
    source->tokens = 0;
    source->defines = 0;
    source->indentstack = 0;
    source->skip = 0;
    source->definehash = (define_s **)GetClearedMemory(0x1000u);
    PC_AddGlobalDefinesToSource(source);
    return source;
}

source_s *sourceFiles[64];
int __cdecl PC_LoadSourceHandle(char *filename, const char **builtinDefines)
{
    source_s *source; // [esp+0h] [ebp-Ch]
    int defineIter; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for (i = 1; i < 64 && sourceFiles[i]; ++i)
        ;
    if (i >= 64)
        return 0;
    source = LoadSourceFile(filename);
    if (!source)
        return 0;
    if (builtinDefines)
    {
        for (defineIter = 0; builtinDefines[defineIter]; ++defineIter)
            PC_AddDefine(source, (char *)builtinDefines[defineIter]);
    }
    sourceFiles[i] = source;
    return i;
}

void __cdecl FreeSource(source_s *source)
{
    script_s *script; // [esp+0h] [ebp-14h]
    define_s *define; // [esp+4h] [ebp-10h]
    indent_s *indent; // [esp+8h] [ebp-Ch]
    token_s *token; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    while (source->scriptstack)
    {
        script = source->scriptstack;
        source->scriptstack = script->next;
        FreeScript(script);
    }
    while (source->tokens)
    {
        token = source->tokens;
        source->tokens = token->next;
        PC_FreeToken(token);
    }
    for (i = 0; i < 1024; ++i)
    {
        while (source->definehash[i])
        {
            define = source->definehash[i];
            source->definehash[i] = define->hashnext;
            PC_FreeDefine(define);
        }
    }
    while (source->indentstack)
    {
        indent = source->indentstack;
        source->indentstack = indent->next;
        FreeMemory((char *)indent);
    }
    if (source->definehash)
        FreeMemory((char *)source->definehash);
    FreeMemory(source->filename);
}

int __cdecl PC_FreeSourceHandle(int handle)
{
    if (handle < 1 || handle >= 64)
        return 0;
    if (!sourceFiles[handle])
        return 0;
    FreeSource(sourceFiles[handle]);
    sourceFiles[handle] = 0;
    return 1;
}

int __cdecl PC_ReadTokenHandle(int handle, pc_token_s *pc_token)
{
    char v3; // [esp+3h] [ebp-449h]
    char *string; // [esp+8h] [ebp-444h]
    token_s *p_token; // [esp+Ch] [ebp-440h]
    int ret; // [esp+10h] [ebp-43Ch]
    token_s token; // [esp+14h] [ebp-438h] BYREF

    if (handle < 1 || handle >= 64)
        return 0;
    if (!sourceFiles[handle])
        return 0;
    ret = PC_ReadToken(sourceFiles[handle], &token);
    p_token = &token;
    string = pc_token->string;
    do
    {
        v3 = p_token->string[0];
        *string = p_token->string[0];
        p_token = (token_s *)((char *)p_token + 1);
        ++string;
    } while (v3);
    pc_token->type = token.type;
    pc_token->subtype = token.subtype;
    pc_token->intvalue = token.intvalue;
    pc_token->floatvalue = token.floatvalue;
    if (pc_token->type == 1)
        StripDoubleQuotes(pc_token->string);
    return ret;
}

int __cdecl PC_ReadLineHandle(int handle, pc_token_s *pc_token)
{
    char v3; // [esp+3h] [ebp-449h]
    char *string; // [esp+8h] [ebp-444h]
    token_s *p_token; // [esp+Ch] [ebp-440h]
    int ret; // [esp+10h] [ebp-43Ch]
    token_s token; // [esp+14h] [ebp-438h] BYREF

    if (handle < 1 || handle >= 64)
        return 0;
    if (!sourceFiles[handle])
        return 0;
    ret = PC_ReadLine(sourceFiles[handle], &token, 1);
    p_token = &token;
    string = pc_token->string;
    do
    {
        v3 = p_token->string[0];
        *string = p_token->string[0];
        p_token = (token_s *)((char *)p_token + 1);
        ++string;
    } while (v3);
    pc_token->type = token.type;
    pc_token->subtype = token.subtype;
    pc_token->intvalue = token.intvalue;
    pc_token->floatvalue = token.floatvalue;
    if (pc_token->type == 1)
        StripDoubleQuotes(pc_token->string);
    return ret;
}

int __cdecl PC_SourceFileAndLine(int handle, char *filename, int *line)
{
    char v4; // cl
    char v5; // dl
    char *v6; // [esp+8h] [ebp-1Ch]
    source_s *v7; // [esp+Ch] [ebp-18h]
    char *v8; // [esp+18h] [ebp-Ch]
    script_s *v9; // [esp+1Ch] [ebp-8h]
    script_s *scriptstack; // [esp+20h] [ebp-4h]

    if (handle < 1 || handle >= 64)
        return 0;
    if (!sourceFiles[handle])
        return 0;
    scriptstack = sourceFiles[handle]->scriptstack;
    if (scriptstack)
    {
        v9 = sourceFiles[handle]->scriptstack;
        v8 = filename;
        do
        {
            v4 = v9->filename[0];
            *v8 = v9->filename[0];
            v9 = (script_s *)((char *)v9 + 1);
            ++v8;
        } while (v4);
        *line = scriptstack->line;
    }
    else
    {
        v7 = sourceFiles[handle];
        v6 = filename;
        do
        {
            v5 = v7->filename[0];
            *v6 = v7->filename[0];
            v7 = (source_s *)((char *)v7 + 1);
            ++v6;
        } while (v5);
        *line = 0;
    }
    return 1;
}

int __cdecl PC_String_Parse(int handle, const char **out)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    *out = String_Alloc(token.string);
    return 1;
}

int __cdecl PC_Int_Parse(int handle, int *i)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF
    int negative; // [esp+414h] [ebp-4h]

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    if (token.string[0] == 40)
        return PC_Int_Expression_Parse(handle, i);
    negative = 0;
    if (token.string[0] == 45)
    {
        if (!PC_ReadTokenHandle(handle, &token))
            return 0;
        negative = 1;
    }
    if (token.type == 3)
    {
        *i = token.intvalue;
        if (negative)
            *i = -*i;
        return 1;
    }
    else
    {
        PC_SourceError(handle, (char*)"expected integer but found %s\n", token.string);
        return 0;
    }
}

void PC_SourceError(int handle, char *format, ...)
{
    static char string_3[4096];

    char filename[132]; // [esp+0h] [ebp-90h] BYREF
    char *argptr; // [esp+88h] [ebp-8h]
    int line; // [esp+8Ch] [ebp-4h] BYREF
    va_list va; // [esp+A0h] [ebp+10h] BYREF

    va_start(va, format);
    _vsnprintf(string_3, 0x1000u, format, va);
    argptr = 0;
    filename[0] = 0;
    line = 0;
    PC_SourceFileAndLine(handle, filename, &line);
    Com_PrintError(13, "Menu load error: %s, line %d: %s\n", filename, line, string_3);
}

bool __cdecl Eval_CanPushValue(const Eval *eval)
{
    const char *pExceptionObject; // [esp+0h] [ebp-4h] BYREF

    if (!eval->valStackPos)
        return 1;
    if (eval->valStackPos == 1024)
    {
        pExceptionObject = "evaluation stack overflow - expression is too complex";
        //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&pExceptionObject, &PA.deinit);
        iassert(0);
    }
    return eval->pushedOp;
}

char __cdecl Eval_PushInteger(Eval *eval, int value)
{
    if (!Eval_CanPushValue(eval))
        return 0;
    eval->valStack[eval->valStackPos].type = EVAL_VALUE_INT;
    eval->valStack[eval->valStackPos++].u.i = value;
    eval->pushedOp = 0;
    return 1;
}

bool __cdecl Eval_OperatorForToken(const char *text, EvalOperatorType *op)
{
    bool result; // al

    if (!text)
        MyAssertHandler(".\\universal\\eval.cpp", 669, 0, "%s", "text");
    if (!op)
        MyAssertHandler(".\\universal\\eval.cpp", 670, 0, "%s", "op");
    switch (*text)
    {
    case '!':
        if (text[1] == 61)
            *op = EVAL_OP_NOT_EQUAL;
        else
            *op = EVAL_OP_LOGICAL_NOT;
        result = 1;
        break;
    case '%':
        *op = EVAL_OP_MODULUS;
        result = 1;
        break;
    case '&':
        if (text[1] == 38)
            *op = EVAL_OP_LOGICAL_AND;
        else
            *op = EVAL_OP_BITWISE_AND;
        result = 1;
        break;
    case '(':
        *op = EVAL_OP_LPAREN;
        result = 1;
        break;
    case ')':
        *op = EVAL_OP_RPAREN;
        result = 1;
        break;
    case '*':
        *op = EVAL_OP_MULTIPLY;
        result = 1;
        break;
    case '+':
        *op = EVAL_OP_PLUS;
        result = 1;
        break;
    case '-':
        *op = EVAL_OP_MINUS;
        result = 1;
        break;
    case '/':
        *op = EVAL_OP_DIVIDE;
        result = 1;
        break;
    case ':':
        *op = EVAL_OP_COLON;
        result = 1;
        break;
    case '<':
        if (text[1] == 60)
        {
            *op = EVAL_OP_LSHIFT;
            result = 1;
        }
        else
        {
            if (text[1] == 61)
                *op = EVAL_OP_LESS_EQUAL;
            else
                *op = EVAL_OP_LESS;
            result = 1;
        }
        break;
    case '=':
        if (text[1] != 61)
            goto LABEL_44;
        *op = EVAL_OP_EQUALS;
        result = 1;
        break;
    case '>':
        if (text[1] == 62)
        {
            *op = EVAL_OP_RSHIFT;
            result = 1;
        }
        else
        {
            if (text[1] == 61)
                *op = EVAL_OP_GREATER_EQUAL;
            else
                *op = EVAL_OP_GREATER;
            result = 1;
        }
        break;
    case '?':
        *op = EVAL_OP_QUESTION;
        result = 1;
        break;
    case '^':
        *op = EVAL_OP_BITWISE_XOR;
        result = 1;
        break;
    case '|':
        if (text[1] == 124)
            *op = EVAL_OP_LOGICAL_OR;
        else
            *op = EVAL_OP_BITWISE_OR;
        result = 1;
        break;
    case '~':
        *op = EVAL_OP_BITWISE_NOT;
        result = 1;
        break;
    default:
    LABEL_44:
        result = 0;
        break;
    }
    return result;
}

char s_precedence[26] =
{
  'c',
  '\0',
  '\x03',
  '\x02',
  '\v',
  '\v',
  '\r',
  '\r',
  '\f',
  '\f',
  '\f',
  '\n',
  '\n',
  '\r',
  '\a',
  '\x05',
  '\x06',
  '\r',
  '\x02',
  '\x01',
  '\b',
  '\b',
  '\t',
  '\t',
  '\t',
  '\t'
}; // idb

bool s_rightToLeft[26] =
{
  false,
  false,
  true,
  true,
  false,
  false,
  true,
  true,
  false,
  false,
  false,
  false,
  false,
  true,
  false,
  false,
  false,
  true,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false
}; // idb

bool __cdecl Eval_IsUnaryOp(const Eval *eval)
{
    return !eval->valStackPos || eval->pushedOp;
}

void __cdecl Eval_PrepareBinaryOpSameTypes(Eval *eval)
{
    int v1; // eax
    const char *v2; // [esp+4h] [ebp-8h] BYREF
    const char *pExceptionObject; // [esp+8h] [ebp-4h] BYREF

    if (eval->valStackPos < 2)
    {
        pExceptionObject = "missing operand (for example, 'a + ' or ' / b')";
        iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&pExceptionObject, &PA.deinit);
    }
    if (eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
        || eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON)
    {
        v2 = "operation not valid on strings";
        iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v2, &PA.deinit);
    }
    if (eval->opStack[4 * eval->valStackPos + 1016] != eval->opStack[4 * eval->valStackPos + 1020])
    {
        if (eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_RPAREN)
        {
            *(double *)&eval->opStack[4 * eval->valStackPos + 1018] = (double)(int)eval->opStack[4 * eval->valStackPos + 1018];
            v1 = eval->valStackPos - 2;
        }
        else
        {
            *(double *)&eval->opStack[4 * eval->valStackPos + 1022] = (double)(int)eval->opStack[4 * eval->valStackPos + 1022];
            v1 = eval->valStackPos - 1;
        }
        eval->valStack[v1].type = EVAL_VALUE_DOUBLE;
    }
}

void __cdecl Eval_PrepareBinaryOpIntegers(Eval *eval)
{
    const char *v1; // [esp+0h] [ebp-8h] BYREF
    const char *pExceptionObject; // [esp+4h] [ebp-4h] BYREF

    if (eval->valStackPos < 2)
    {
        pExceptionObject = "missing operand (for example, 'a + ' or ' / b')";
        iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&pExceptionObject, &PA.deinit);
    }
    if (eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
        || eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON)
    {
        v1 = "operation not valid on strings";
        iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v1, &PA.deinit);
    }
    if (eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_LPAREN)
    {
        eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)((int)*(double *)&eval->opStack[4 * eval->valStackPos + 1018]);
        eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
    }
    if (eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_LPAREN)
    {
        eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)((int)*(double *)&eval->opStack[4 * eval->valStackPos + 1022]);
        eval->opStack[4 * eval->valStackPos + 1020] = EVAL_OP_RPAREN;
    }
}

void __cdecl Eval_PrepareBinaryOpBoolean(Eval *eval)
{
    const char *v1; // [esp+8h] [ebp-8h] BYREF
    const char *pExceptionObject; // [esp+Ch] [ebp-4h] BYREF

    if (eval->valStackPos < 2)
    {
        pExceptionObject = "missing operand (for example, 'a + ' or ' / b')";
        iassert(0); //iassert(0); //_CxxThrowException(&pExceptionObject, &PA.deinit);
    }
    if (eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
        || eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON)
    {
        v1 = "operation not valid on strings";
        iassert(0); //iassert(0); //_CxxThrowException(&v1, &PA.deinit);
    }
    if (eval->opStack[4 * eval->valStackPos + 1016])
    {
        eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1018] != EVAL_OP_LPAREN);
    }
    else
    {
        eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(0.0 != *(double *)&eval->opStack[4 * eval->valStackPos + 1018]);
        eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
    }
    if (eval->opStack[4 * eval->valStackPos + 1020])
    {
        eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1022] != EVAL_OP_LPAREN);
    }
    else
    {
        eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(0.0 != *(double *)&eval->opStack[4 * eval->valStackPos + 1022]);
        eval->opStack[4 * eval->valStackPos + 1020] = EVAL_OP_RPAREN;
    }
}

bool __cdecl Eval_EvaluationStep(Eval *eval)
{
    bool result; // al
    EvalValue *v2; // edx
    EvalOperatorType *v3; // ecx
    int v4; // [esp+10h] [ebp-94h]
    const char *v5; // [esp+5Ch] [ebp-48h] BYREF
    const char *v6; // [esp+60h] [ebp-44h] BYREF
    const char *v7; // [esp+64h] [ebp-40h] BYREF
    const char *v8; // [esp+68h] [ebp-3Ch] BYREF
    const char *v9; // [esp+6Ch] [ebp-38h] BYREF
    const char *v10; // [esp+70h] [ebp-34h] BYREF
    const char *v11; // [esp+74h] [ebp-30h] BYREF
    const char *v12; // [esp+78h] [ebp-2Ch] BYREF
    const char *v13; // [esp+7Ch] [ebp-28h] BYREF
    const char *v14; // [esp+80h] [ebp-24h] BYREF
    const char *pExceptionObject; // [esp+84h] [ebp-20h] BYREF
    bool v16; // [esp+8Ah] [ebp-1Ah]
    bool same; // [esp+8Bh] [ebp-19h]
    long double dQuotientFloor; // [esp+8Ch] [ebp-18h]
    char *s; // [esp+94h] [ebp-10h]
    int length[2]; // [esp+98h] [ebp-Ch]
    int i; // [esp+A0h] [ebp-4h]

    if (!eval->opStackPos)
        return 0;
    if (eval->opStack[--eval->opStackPos] == EVAL_OP_LPAREN)
        return 1;
    if (eval->opStack[eval->opStackPos] == EVAL_OP_QUESTION)
    {
        pExceptionObject = "found '?' with no following ':' in expression of type 'a ? b : c'";
        //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&pExceptionObject, &PA.deinit);
        iassert(0);
    }
    if (!eval->valStackPos)
    {
        v14 = "missing operand (for example, 'a + ' or ' / b')";
        iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v14, &PA.deinit);
    }
    switch (eval->opStack[eval->opStackPos])
    {
    case EVAL_OP_COLON:
        if (eval->valStackPos < 3)
        {
            v6 = "missing operand (for example, 'a + ' or ' / b')";
            iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v6, &PA.deinit);
        }
        if (eval->opStack[4 * eval->valStackPos + 1012])
        {
            if (eval->opStack[4 * eval->valStackPos + 1012] != EVAL_OP_RPAREN)
            {
                v5 = "can only switch on numbers";
                iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v5, &PA.deinit);
            }
            i = -(eval->opStack[4 * eval->valStackPos + 1014] != EVAL_OP_LPAREN) - 1;
        }
        else
        {
            if (0.0 == *(double *)&eval->opStack[4 * eval->valStackPos + 1014])
                v4 = -1;
            else
                v4 = -2;
            i = v4;
        }
        if (eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
            && eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON)
        {
            free(eval->valStack[1 - i + eval->valStackPos].u.s);
        }
        else
        {
            Eval_PrepareBinaryOpSameTypes(eval);
        }
        v2 = &eval->valStack[i + eval->valStackPos];
        v3 = &eval->opStack[4 * eval->valStackPos + 1012];
        *v3 = (EvalOperatorType)v2->type;
        v3[1] = *((EvalOperatorType *)&v2->type + 1);
        v3[2] = (EvalOperatorType)v2->u.i;
        v3[3] = *((EvalOperatorType *)&v2->u.s + 1);
        eval->valStackPos -= 2;
        --eval->opStackPos;
        goto LABEL_121;
    case EVAL_OP_PLUS:
        if (eval->valStackPos >= 2
            && eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
            && eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON)
        {
            length[0] = strlen((const char *)eval->opStack[4 * eval->valStackPos + 1018]);
            length[1] = strlen((const char *)eval->opStack[4 * eval->valStackPos + 1022]);
            s = (char *)malloc(length[0] + length[1] + 1);
            memcpy((unsigned __int8 *)s, (unsigned __int8 *)eval->opStack[4 * eval->valStackPos + 1018], length[0]);
            memcpy(
                (unsigned __int8 *)&s[length[0]],
                (unsigned __int8 *)eval->opStack[4 * eval->valStackPos + 1022],
                length[1] + 1);
            free((void *)eval->opStack[4 * eval->valStackPos + 1018]);
            free((void *)eval->opStack[4 * eval->valStackPos + 1022]);
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)((uintptr_t)s);
        }
        else
        {
            Eval_PrepareBinaryOpSameTypes(eval);
            if (eval->opStack[4 * eval->valStackPos + 1016])
                eval->opStack[4 * eval->valStackPos + 1018] += eval->opStack[4 * eval->valStackPos + 1022];
            else
                *(double *)&eval->opStack[4 * eval->valStackPos + 1018] = *(double *)&eval->opStack[4 * eval->valStackPos
                + 1018]
                + *(double *)&eval->opStack[4 * eval->valStackPos
                + 1022];
        }
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_MINUS:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
            eval->opStack[4 * eval->valStackPos + 1018] -= eval->opStack[4 * eval->valStackPos + 1022];
        else
            *(double *)&eval->opStack[4 * eval->valStackPos + 1018] = *(double *)&eval->opStack[4 * eval->valStackPos + 1018]
            - *(double *)&eval->opStack[4 * eval->valStackPos + 1022];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_UNARY_PLUS:
        goto LABEL_121;
    case EVAL_OP_UNARY_MINUS:
        if (eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_RPAREN)
        {
            eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(-eval->opStack[4 * eval->valStackPos + 1022]);
        }
        else
        {
            if (eval->opStack[4 * eval->valStackPos + 1020])
            {
                v13 = "cannot negate strings";
                //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v13, &PA.deinit);
                iassert(0);
            }
            *(double *)&eval->opStack[4 * eval->valStackPos + 1022] = -*(double *)&eval->opStack[4 * eval->valStackPos
                + 1022];
        }
        goto LABEL_121;
    case EVAL_OP_MULTIPLY:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
            eval->opStack[4 * eval->valStackPos + 1018] *= eval->opStack[4 * eval->valStackPos + 1022];
        else
            *(double *)&eval->opStack[4 * eval->valStackPos + 1018] = *(double *)&eval->opStack[4 * eval->valStackPos + 1018]
            * *(double *)&eval->opStack[4 * eval->valStackPos + 1022];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_DIVIDE:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
        {
            if (eval->opStack[4 * eval->valStackPos + 1022] == EVAL_OP_LPAREN)
            {
                v9 = "divide by zero";
                //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v9, &PA.deinit);
                iassert(0);
            }
            eval->opStack[4 * eval->valStackPos + 1018] /= eval->opStack[4 * eval->valStackPos + 1022];
        }
        else
        {
            if (0.0 == *(double *)&eval->opStack[4 * eval->valStackPos + 1022])
            {
                v10 = "divide by zero";
                //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v10, &PA.deinit);
                iassert(0);
            }
            *(double *)&eval->opStack[4 * eval->valStackPos + 1018] = *(double *)&eval->opStack[4 * eval->valStackPos + 1018]
                / *(double *)&eval->opStack[4 * eval->valStackPos + 1022];
        }
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_MODULUS:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
        {
            if (eval->opStack[4 * eval->valStackPos + 1022] == EVAL_OP_LPAREN)
            {
                v7 = "divide by zero";
                //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v7, &PA.deinit);
                iassert(0);
            }
            eval->opStack[4 * eval->valStackPos + 1018] %= eval->opStack[4 * eval->valStackPos + 1022];
        }
        else
        {
            if (0.0 == *(double *)&eval->opStack[4 * eval->valStackPos + 1022])
            {
                v8 = "divide by zero";
                //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v8, &PA.deinit);
                iassert(0);
            }
            dQuotientFloor = floor(
                *(double *)&eval->opStack[4 * eval->valStackPos + 1018]
                / *(double *)&eval->opStack[4 * eval->valStackPos + 1022]);
            *(long double *)&eval->opStack[4 * eval->valStackPos + 1018] = *(double *)&eval->opStack[4 * eval->valStackPos
                + 1018]
                - *(double *)&eval->opStack[4 * eval->valStackPos
                + 1022]
                * dQuotientFloor;
        }
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_LSHIFT:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
            eval->opStack[4 * eval->valStackPos + 1018] <<= eval->opStack[4 * eval->valStackPos + 1022];
        else
            *(long double *)&eval->opStack[4 * eval->valStackPos + 1018] = pow(
                2.0,
                *(double *)&eval->opStack[4 * eval->valStackPos
                + 1022])
            * *(double *)&eval->opStack[4 * eval->valStackPos
            + 1018];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_RSHIFT:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
            eval->opStack[4 * eval->valStackPos + 1018] >>= eval->opStack[4 * eval->valStackPos + 1022];
        else
            *(long double *)&eval->opStack[4 * eval->valStackPos + 1018] = pow(
                2.0,
                -*(double *)&eval->opStack[4 * eval->valStackPos + 1022])
            * *(double *)&eval->opStack[4 * eval->valStackPos
            + 1018];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_BITWISE_NOT:
        if (eval->opStack[4 * eval->valStackPos + 1020])
        {
            if (eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON)
            {
                v11 = "cannot bitwise invert strings";
                iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v11, &PA.deinit);
            }
        }
        else
        {
            eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)((int)*(double *)&eval->opStack[4 * eval->valStackPos + 1022]);
            eval->opStack[4 * eval->valStackPos + 1020] = EVAL_OP_RPAREN;
        }
        eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(~eval->opStack[4 * eval->valStackPos + 1022]);
        goto LABEL_121;
    case EVAL_OP_BITWISE_AND:
        Eval_PrepareBinaryOpIntegers(eval);
        eval->opStack[4 * eval->valStackPos + 1018] &= eval->opStack[4 * eval->valStackPos + 1022];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_BITWISE_OR:
        Eval_PrepareBinaryOpIntegers(eval);
        eval->opStack[4 * eval->valStackPos + 1018] |= eval->opStack[4 * eval->valStackPos + 1022];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_BITWISE_XOR:
        Eval_PrepareBinaryOpIntegers(eval);
        eval->opStack[4 * eval->valStackPos + 1018] ^= eval->opStack[4 * eval->valStackPos + 1022];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_LOGICAL_NOT:
        if (eval->opStack[4 * eval->valStackPos + 1020])
        {
            if (eval->opStack[4 * eval->valStackPos + 1020] != EVAL_OP_RPAREN)
            {
                v12 = "cannot logical invert strings";
                iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v12, &PA.deinit);
            }
            eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1022] == EVAL_OP_LPAREN);
        }
        else
        {
            eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(0.0 == *(double *)&eval->opStack[4 * eval->valStackPos + 1022]);
        }
        eval->opStack[4 * eval->valStackPos + 1020] = EVAL_OP_RPAREN;
        goto LABEL_121;
    case EVAL_OP_LOGICAL_AND:
        Eval_PrepareBinaryOpBoolean(eval);
        eval->opStack[4 * eval->valStackPos + 1018] &= eval->opStack[4 * eval->valStackPos + 1022];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_LOGICAL_OR:
        Eval_PrepareBinaryOpBoolean(eval);
        eval->opStack[4 * eval->valStackPos + 1018] |= eval->opStack[4 * eval->valStackPos + 1022];
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_EQUALS:
        if (eval->valStackPos >= 2
            && eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
            && eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON)
        {
            same = _stricmp(
                (const char *)eval->opStack[4 * eval->valStackPos + 1018],
                (const char *)eval->opStack[4 * eval->valStackPos + 1022]) == 0;
            free((void *)eval->opStack[4 * eval->valStackPos + 1018]);
            free((void *)eval->opStack[4 * eval->valStackPos + 1022]);
            eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)same;
        }
        else
        {
            Eval_PrepareBinaryOpSameTypes(eval);
            if (eval->opStack[4 * eval->valStackPos + 1016])
            {
                eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1018] == eval->opStack[4 * eval->valStackPos + 1022]);
            }
            else
            {
                eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(*(double *)&eval->opStack[4 * eval->valStackPos + 1022] == *(double *)&eval->opStack[4 * eval->valStackPos + 1018]);
                eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
            }
        }
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_NOT_EQUAL:
        if (eval->valStackPos >= 2
            && eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
            && eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON)
        {
            v16 = _stricmp(
                (const char *)eval->opStack[4 * eval->valStackPos + 1018],
                (const char *)eval->opStack[4 * eval->valStackPos + 1022]) == 0;
            free((void *)eval->opStack[4 * eval->valStackPos + 1018]);
            free((void *)eval->opStack[4 * eval->valStackPos + 1022]);
            eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)!v16;
        }
        else
        {
            Eval_PrepareBinaryOpSameTypes(eval);
            if (eval->opStack[4 * eval->valStackPos + 1016])
            {
                eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1018] != eval->opStack[4 * eval->valStackPos + 1022]);
            }
            else
            {
                eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(*(double *)&eval->opStack[4 * eval->valStackPos + 1022] != *(double *)&eval->opStack[4 * eval->valStackPos + 1018]);
                eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
            }
        }
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_LESS:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
        {
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1018] < eval->opStack[4 * eval->valStackPos + 1022]);
        }
        else
        {
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(*(double *)&eval->opStack[4 * eval->valStackPos + 1022] > *(double *)&eval->opStack[4 * eval->valStackPos + 1018]);
            eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
        }
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_LESS_EQUAL:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
        {
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1018] <= eval->opStack[4 * eval->valStackPos + 1022]);
        }
        else
        {
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(*(double *)&eval->opStack[4 * eval->valStackPos + 1022] >= *(double *)&eval->opStack[4 * eval->valStackPos + 1018]);
            eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
        }
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_GREATER:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
        {
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1018] > eval->opStack[4 * eval->valStackPos + 1022]);
        }
        else
        {
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(*(double *)&eval->opStack[4 * eval->valStackPos + 1022] < *(double *)&eval->opStack[4 * eval->valStackPos + 1018]);
            eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
        }
        --eval->valStackPos;
        goto LABEL_121;
    case EVAL_OP_GREATER_EQUAL:
        Eval_PrepareBinaryOpSameTypes(eval);
        if (eval->opStack[4 * eval->valStackPos + 1016])
        {
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1018] >= eval->opStack[4 * eval->valStackPos + 1022]);
        }
        else
        {
            eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(*(double *)&eval->opStack[4 * eval->valStackPos + 1022] <= *(double *)&eval->opStack[4 * eval->valStackPos + 1018]);
            eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
        }
        --eval->valStackPos;
    LABEL_121:
        result = 1;
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(".\\universal\\eval.cpp", 488, 0, "unknown operator type");
        result = 0;
        break;
    }
    return result;
}

char __cdecl Eval_PushOperator(Eval *eval, EvalOperatorType op)
{
    bool v3; // [esp+0h] [ebp-10h]
    const char *v4; // [esp+4h] [ebp-Ch] BYREF
    const char *pExceptionObject; // [esp+8h] [ebp-8h] BYREF
    bool leftToRight; // [esp+Dh] [ebp-3h]
    char precedence; // [esp+Eh] [ebp-2h]
    bool higherPrecedence; // [esp+Fh] [ebp-1h]

    if (s_precedence[op] < 0)
        return 0;
    if (op == EVAL_OP_RPAREN && !eval->parenCount)
        return 0;
    if (op == EVAL_OP_LPAREN)
    {
        if (eval->valStackPos && !eval->pushedOp)
            return 0;
        ++eval->parenCount;
    }
    if (op == EVAL_OP_PLUS)
    {
        if (Eval_IsUnaryOp(eval))
            op = EVAL_OP_UNARY_PLUS;
    }
    else if (op == EVAL_OP_MINUS && Eval_IsUnaryOp(eval))
    {
        op = EVAL_OP_UNARY_MINUS;
    }
    if ((unsigned int)op >= EVAL_OP_COUNT)
        MyAssertHandler(".\\universal\\eval.cpp", 549, 0, "%s", "op >= 0 && op < ARRAY_COUNT( s_precedence )");
    precedence = s_precedence[op];
    while (eval->opStackPos > 0)
    {
        higherPrecedence = s_precedence[eval->opStack[eval->opStackPos - 1]] > precedence;
        v3 = s_precedence[eval->opStack[eval->opStackPos - 1]] == precedence
            && !s_rightToLeft[eval->opStack[eval->opStackPos - 1]];
        leftToRight = v3;
        if (!higherPrecedence && !leftToRight)
            break;
        if (eval->opStack[eval->opStackPos - 1] == EVAL_OP_LPAREN)
        {
            if (op == EVAL_OP_RPAREN)
            {
                --eval->parenCount;
                --eval->opStackPos;
                eval->pushedOp = 0;
                return 1;
            }
            break;
        }
        if (!Eval_EvaluationStep(eval))
            return 0;
    }
    if (op != EVAL_OP_COLON || eval->opStackPos && eval->opStack[eval->opStackPos - 1] == EVAL_OP_QUESTION)
    {
        if (eval->opStackPos == 1024)
        {
            v4 = "evaluation stack overflow - expression is too complex";
            //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&v4, &PA.deinit);
            iassert(0);
        }
        eval->opStack[eval->opStackPos++] = op;
        eval->pushedOp = 1;
        return 1;
    }
    else
    {
        if (eval->parenCount)
        {
            pExceptionObject = "found ':' without preceding '?' in expression of type 'a ? b : c'";
            //iassert(0); //iassert(0); //iassert(0); //iassert(0); //iassert(0); //_CxxThrowException(&pExceptionObject, &PA.deinit);
            iassert(0);
        }
        return 0;
    }
}

int __cdecl PC_Int_Expression_Parse(int handle, int *i)
{
    EvalValue result; // [esp+0h] [ebp-5468h] BYREF
    EvalValue v4; // [esp+10h] [ebp-5458h]
    EvalOperatorType op; // [esp+24h] [ebp-5444h] BYREF
    EvalValue v6; // [esp+28h] [ebp-5440h]
    Eval eval; // [esp+38h] [ebp-5430h] BYREF
    int v8; // [esp+504Ch] [ebp-41Ch]
    pc_token_s pc_token; // [esp+5050h] [ebp-418h] BYREF

    memset(&eval.opStackPos, 0, 13);
    v8 = 0;
    while (1)
    {
        while (1)
        {
            if (!PC_ReadTokenHandle(handle, &pc_token))
                return 0;
            if (pc_token.type != 3)
                break;
            if (!Eval_PushInteger(&eval, pc_token.intvalue))
                goto LABEL_6;
        }
        if (!Eval_OperatorForToken(pc_token.string, &op))
        {
            PC_SourceError(handle, (char*)"expected operator but found %s\n", pc_token.string);
            return 0;
        }
        if (op == EVAL_OP_RPAREN)
            break;
        if (op == EVAL_OP_LPAREN && ++v8 > 16)
        {
            PC_SourceError(handle, (char *)"too much recursive macro expansion\n");
            return 0;
        }
    LABEL_16:
        Eval_PushOperator(&eval, op);
    }
    if (v8)
    {
        --v8;
        goto LABEL_16;
    }
    if (Eval_AnyMissingOperands(&eval))
    {
    LABEL_6:
        PC_SourceError(handle, (char *)"error evaluating expression\n");
        return 0;
    }
    v4 = *Eval_Solve(&result, &eval);
    v6 = v4;
    if (v4.type != EVAL_VALUE_INT)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 464, 0, "%s", "result.type == EVAL_VALUE_INT");
    *i = v6.u.i;
    return 1;
}

int __cdecl PC_Float_Parse(int handle, float *f)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF
    int negative; // [esp+414h] [ebp-4h]

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    if (token.string[0] == 40)
        return PC_Float_Expression_Parse(handle, f);
    negative = 0;
    if (token.string[0] == 45)
    {
        if (!PC_ReadTokenHandle(handle, &token))
            return 0;
        negative = 1;
    }
    if (token.type == 3)
    {
        if (negative)
            *f = -token.floatvalue;
        else
            *f = token.floatvalue;
        return 1;
    }
    else
    {
        PC_SourceError(handle, (char *)"expected float but found %s\n", token.string);
        return 0;
    }
}

int __cdecl PC_Rect_Parse(int handle, rectDef_s *r)
{
    if (!PC_Float_Parse(handle, &r->x)
        || !PC_Float_Parse(handle, &r->y)
        || !PC_Float_Parse(handle, &r->w)
        || !PC_Float_Parse(handle, &r->h))
    {
        return 0;
    }
    if (!PC_Int_ParseLine(handle, &r->horzAlign))
        r->horzAlign = 0;
    if (!PC_Int_ParseLine(handle, &r->vertAlign))
        r->vertAlign = 0;
    return 1;
}

char __cdecl Eval_PushNumber(Eval *eval, long double value)
{
    if (!Eval_CanPushValue(eval))
        return 0;
    eval->valStack[eval->valStackPos].type = EVAL_VALUE_DOUBLE;
    eval->valStack[eval->valStackPos++].u.d = value;
    eval->pushedOp = 0;
    return 1;
}

bool __cdecl Eval_AnyMissingOperands(const Eval *eval)
{
    int opIndex; // [esp+0h] [ebp-8h]
    int requiredOperandCount; // [esp+4h] [ebp-4h]

    requiredOperandCount = 1;
    for (opIndex = 0; opIndex < eval->opStackPos; ++opIndex)
        requiredOperandCount += s_consumedOperandCount[eval->opStack[opIndex]];
    return requiredOperandCount != eval->valStackPos;
}

EvalValue *__cdecl Eval_Solve(EvalValue *result, Eval *eval)
{
    int v3; // [esp+0h] [ebp-2Ch] BYREF
    const char *v5; // [esp+14h] [ebp-18h] BYREF
   // _DWORD pExceptionObject[5]; // [esp+18h] [ebp-14h] BYREF

    //pExceptionObject[1] = &v3;
    //pExceptionObject[4] = 0;
    if (eval->parenCount)
    {
        //pExceptionObject[0] = "missing ')'";
        iassert(0); //_CxxThrowException(pExceptionObject, &PA.deinit);
    }
    while (Eval_EvaluationStep(eval))
        ;
    if (eval->opStackPos)
        MyAssertHandler(".\\universal\\eval.cpp", 647, 0, "%s", "eval->opStackPos == 0");
    if (eval->valStackPos > 1)
    {
        v5 = "extra operand (for example, 'a b +')";
        iassert(0); //_CxxThrowException(&v5, &PA.deinit);
    }
    *result = eval->valStack[0];
    return result;
}

int __cdecl PC_Float_Expression_Parse(int handle, float *f)
{
    EvalValue result; // [esp+8h] [ebp-5468h] BYREF
    EvalValue v4; // [esp+18h] [ebp-5458h]
    EvalOperatorType op; // [esp+2Ch] [ebp-5444h] BYREF
    EvalValue v6; // [esp+30h] [ebp-5440h]
    Eval eval; // [esp+40h] [ebp-5430h] BYREF
    int v8; // [esp+5054h] [ebp-41Ch]
    pc_token_s pc_token; // [esp+5058h] [ebp-418h] BYREF

    memset(&eval.opStackPos, 0, 13);
    v8 = 0;
    while (1)
    {
        while (1)
        {
            if (!PC_ReadTokenHandle(handle, &pc_token))
                return 0;
            if (pc_token.type != 3)
                break;
            if (!Eval_PushNumber(&eval, pc_token.floatvalue))
                goto LABEL_6;
        }
        if (!Eval_OperatorForToken(pc_token.string, &op))
        {
            PC_SourceError(handle, (char *)"expected operator but found %s\n", pc_token.string);
            return 0;
        }
        if (op == EVAL_OP_RPAREN)
            break;
        if (op == EVAL_OP_LPAREN && ++v8 > 16)
        {
            PC_SourceError(handle, (char *)"too much recursive macro expansion\n");
            return 0;
        }
    LABEL_16:
        Eval_PushOperator(&eval, op);
    }
    if (v8)
    {
        --v8;
        goto LABEL_16;
    }
    if (Eval_AnyMissingOperands(&eval))
    {
    LABEL_6:
        PC_SourceError(handle, (char*)"error evaluating expression\n");
        return 0;
    }
    v4 = *Eval_Solve(&result, &eval);
    v6 = v4;
    if (v4.type)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 313, 0, "%s", "result.type == EVAL_VALUE_DOUBLE");
    *f = v6.u.d;
    return 1;
}

int __cdecl PC_Int_ParseLine(int handle, int *i)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF
    int negative; // [esp+414h] [ebp-4h]

    if (!PC_ReadLineHandle(handle, &token))
        return 0;
    negative = 0;
    if (token.string[0] == 45)
    {
        if (!PC_ReadLineHandle(handle, &token))
            return 0;
        negative = 1;
    }
    if (token.type == 3)
    {
        *i = token.intvalue;
        if (negative)
            *i = -*i;
        return 1;
    }
    else
    {
        PC_SourceError(handle, (char*)"expected integer but found %s\n", token.string);
        return 0;
    }
}

int __cdecl PC_Script_Parse(int handle, const char **out)
{
    char *v3; // eax
    char dst[5120]; // [esp+30h] [ebp-1818h] BYREF
    pc_token_s pc_token; // [esp+1430h] [ebp-418h] BYREF

    memset((unsigned __int8 *)dst, 0, sizeof(dst));
    if (!PC_ReadTokenHandle(handle, &pc_token))
        return 0;
    if (I_stricmp(pc_token.string, "{"))
        return 0;
    while (1)
    {
        if (!PC_ReadTokenHandle(handle, &pc_token))
            return 0;
        if (!I_stricmp(pc_token.string, "}"))
        {
            *out = String_Alloc(dst);
            return 1;
        }
        if ((unsigned int)(&pc_token.string[strlen(pc_token.string) + 1]
            - &pc_token.string[1]
            + &dst[strlen(dst) + 1]
            - &dst[1]) > 0x1400)
            break;
        if (pc_token.string[0] && !pc_token.string[1])
        {
            I_strncat(dst, 5120, pc_token.string);
        }
        else
        {
            v3 = va("\"%s\"", pc_token.string);
            I_strncat(dst, 5120, v3);
        }
        if ((unsigned int)(&dst[strlen(dst) + 1] - &dst[1] + 1) > 0x1400)
            break;
        I_strncat(dst, 5120, " ");
    }
    Com_PrintError(16, "action block too long that starts with: %s\n", dst);
    return 0;
}

int __cdecl PC_Color_Parse(int handle, float (*c)[4])
{
    float f; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        if (!PC_Float_Parse(handle, &f))
            return 0;
        (*c)[i] = f;
    }
    return 1;
}

int __cdecl PC_Char_Parse(int handle, char *out)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    *out = token.string[0];
    return 1;
}



int __cdecl MenuParse_name(menuDef_t *menu, int handle)
{
    if (!PC_String_Parse(handle, &menu->window.name))
        return 0;
    I_strlwr((char *)menu->window.name);
    return 1;
}

int __cdecl MenuParse_fullscreen(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->fullScreen);
}

int __cdecl MenuParse_rect(menuDef_t *menu, int handle)
{
    return PC_Rect_Parse(handle, &menu->window.rect);
}

void __cdecl free_expression(statement_s *statement)
{
    expressionEntry *entry; // [esp+0h] [ebp-8h]
    int entryNum; // [esp+4h] [ebp-4h]

    if (statement->numEntries)
    {
        for (entryNum = 0; entryNum < statement->numEntries; ++entryNum)
        {
            entry = statement->entries[entryNum];
            if (entry->type == 1 && entry->data.op == OP_MULTIPLY)
                Z_Free((char *)entry->data.operand.internals.intVal, 34);
            Z_Free((char *)entry, 34);
            statement->entries[entryNum] = 0;
        }
        Z_Free((char *)statement->entries, 34);
        statement->entries = 0;
        statement->numEntries = 0;
    }
}

const char *g_expOperatorNames[81] =
{
  "NOOP",
  ")",
  "*",
  "/",
  "%",
  "+",
  "-",
  "!",
  "<",
  "<=",
  ">",
  ">=",
  "==",
  "!=",
  "&",
  "||",
  "(",
  ",",
  "&",
  "|",
  "~",
  "<<",
  ">>",
  "sin",
  "cos",
  "min",
  "max",
  "milliseconds",
  "dvarint",
  "dvarbool",
  "dvarfloat",
  "dvarstring",
  "stat",
  "ui_active",
  "flashbanged",
  "scoped",
  "scoreboard_visible",
  "inkillcam",
  "player",
  "selecting_location",
  "team",
  "otherteam",
  "marinesfield",
  "opforfield",
  "menuisopen",
  "writingdata",
  "inlobby",
  "inprivateparty",
  "privatepartyhost",
  "privatepartyhostinlobby",
  "aloneinparty",
  "adsjavelin",
  "weaplockblink",
  "weapattacktop",
  "weapattackdirect",
  "secondsastime",
  "tablelookup",
  "locstring",
  "localvarint",
  "localvarbool",
  "localvarfloat",
  "localvarstring",
  "timeleft",
  "secondsascountdown",
  "gamemsgwndactive",
  "int",
  "string",
  "float",
  "gametypename",
  "gametype",
  "gametypedescription",
  "scoreatrank",
  "friendsonline",
  "spectatingclient",
  "statrangeanybitsset",
  "keybinding",
  "actionslotusable",
  "hudfade",
  "maxrecommendedplayers",
  "acceptinginvite",
  "isintermission"
}; // idb

int __cdecl parse_operatorToken(const char *token)
{
    int opNum; // [esp+0h] [ebp-4h]

    for (opNum = 0; opNum < 81; ++opNum)
    {
        if (!I_stricmp(g_expOperatorNames[opNum], token))
            return opNum;
    }
    return 0;
}

int currentTempOperand_0;
char s_tempOperandValueAsString_0[4][256];
char *__cdecl GetValueAsString(Operand operand)
{
    char *result; // [esp+8h] [ebp-4h]

    if ((unsigned int)currentTempOperand_0 >= 4)
        MyAssertHandler(
            ".\\ui\\ui_expressions_obj.cpp",
            106,
            0,
            "currentTempOperand doesn't index NUM_OPERAND_STRINGS\n\t%i not in [0, %i)",
            currentTempOperand_0,
            4);
    result = s_tempOperandValueAsString_0[currentTempOperand_0];
    currentTempOperand_0 = (currentTempOperand_0 + 1) % 4;
    if (operand.dataType == VAL_STRING)
    {
        Com_sprintf(result, 0x100u, "%s", operand.internals.string);
    }
    else if (operand.dataType)
    {
        if (operand.dataType == VAL_FLOAT)
            Com_sprintf(result, 0x100u, "%f", operand.internals.floatVal);
    }
    else
    {
        Com_sprintf(result, 0x100u, "%i", operand.internals.intVal);
    }
    return result;
}

void __cdecl Statement_AddEntry(statement_s *statement, expressionEntry *entry)
{
    statement->entries[statement->numEntries++] = entry;
}

void __cdecl Statement_AddOperator(statement_s *statement, operationEnum op)
{
    unsigned int *v2; // eax

    v2 = (unsigned int*)Z_Malloc(12, "Statement_AddOperator", 34);
    *v2 = 0;
    v2[1] = op;
    Statement_AddEntry(statement, (expressionEntry *)v2);
}

void __cdecl Statement_AddIntOperand(statement_s *statement, int val)
{
    unsigned int *v2; // eax

    v2 = (unsigned int*)Z_Malloc(12, "Statement_AddIntOperand", 34);
    *v2 = 1;
    v2[1] = 0;
    v2[2] = val;
    Statement_AddEntry(statement, (expressionEntry *)v2);
}

void __cdecl Statement_AddFloatOperand(statement_s *statement, float val)
{
    unsigned int *v2; // eax

    v2 = (unsigned int*)Z_Malloc(12, "Statement_AddFloatOperand", 34);
    *v2 = 1;
    v2[1] = 1;
    *((float *)v2 + 2) = val;
    Statement_AddEntry(statement, (expressionEntry *)v2);
}

void __cdecl Statement_AddStringOperand(statement_s *statement, char *str)
{
    expressionEntry *entry; // [esp+20h] [ebp-4h]

    entry = (expressionEntry *)Z_Malloc(12, "Statement_AddStringOperand", 34);
    entry->type = 1;
    entry->data.op = OP_MULTIPLY;
    entry->data.operand.internals.intVal = (int)Z_Malloc(strlen(str) + 1, "Statement_AddStringOperand", 34);
    I_strncpyz((char *)entry->data.operand.internals.intVal, str, strlen(str) + 1);
    Statement_AddEntry(statement, entry);
}

char __cdecl parse_expression_internal(int handle, statement_s *statement, int maxEntries)
{
    expressionEntry *v4; // edx
    operandInternalDataUnion v5; // ecx
    char *ValueAsString; // eax
    int type; // [esp+8h] [ebp-42Ch]
    operationEnum op; // [esp+Ch] [ebp-428h]
    operationEnum lastOp; // [esp+10h] [ebp-424h]
    pc_token_s token; // [esp+14h] [ebp-420h] BYREF
    Operand lastOperand; // [esp+428h] [ebp-Ch]
    int numOpenLeftParens; // [esp+430h] [ebp-4h]

    numOpenLeftParens = 0;
    while (PC_ReadTokenHandle(handle, &token))
    {
        if (!I_stricmp(token.string, ";"))
            return 1;
        if (statement->numEntries == maxEntries)
        {
            Com_PrintError(
                16,
                "Need to increment MAX_TOKENS_PER_STATEMENT - this statement has more than %i tokens\n",
                maxEntries);
            free_expression(statement);
            return 0;
        }
        if (token.type == 1)
            op = OP_NOOP;
        else
            op = (operationEnum)parse_operatorToken(token.string);
        if (op)
        {
            if (op != OP_LEFTPAREN)
            {
                if (op == OP_RIGHTPAREN)
                {
                    if (--numOpenLeftParens < 0)
                    {
                        Com_PrintError(
                            16,
                            "UI Expression error: Found a right parenthesis that doesn't match any left parenthesis\n");
                        return 1;
                    }
                    if (!numOpenLeftParens)
                        return 1;
                }
                goto LABEL_28;
            }
            ++numOpenLeftParens;
            if (!statement->numEntries)
                goto LABEL_28;
            type = statement->entries[statement->numEntries - 1]->type;
            if (type)
            {
                if (type == 1)
                {
                    v4 = statement->entries[statement->numEntries - 1];
                    v5.intVal = (int)v4->data.operand.internals;
                    lastOperand.dataType = v4->data.operand.dataType;
                    lastOperand.internals = v5;
                    //ValueAsString = GetValueAsString((Operand)__PAIR64__(v5.intVal, lastOperand.dataType));
                    ValueAsString = GetValueAsString(lastOperand);
                    Com_PrintError(16, "Probably UI Expression error: %s(...\n", ValueAsString);
                }
                goto LABEL_28;
            }
            lastOp = statement->entries[statement->numEntries - 1]->data.op;
            if (lastOp != OP_NOT && (lastOp < OP_SIN || lastOp >= NUM_OPERATORS))
                LABEL_28:
            Statement_AddOperator(statement, op);
        }
        else
        {
            switch (token.type)
            {
            case 1:
                goto LABEL_32;
            case 3:
                if (token.floatvalue == (double)token.intvalue)
                    Statement_AddIntOperand(statement, token.intvalue);
                else
                    Statement_AddFloatOperand(statement, token.floatvalue);
                break;
            case 4:
            LABEL_32:
                Statement_AddStringOperand(statement, token.string);
                break;
            default:
                Com_PrintError(16, "ERROR: Unknown token '%s'\n", token.string);
                free_expression(statement);
                return 0;
            }
        }
    }
    return 1;
}

char __cdecl parse_expression(int handle, statement_s *statement, int maxEntries)
{
    statement->numEntries = 0;
    return parse_expression_internal(handle, statement, maxEntries);
}

int __cdecl MenuParse_visible(menuDef_t *menu, int handle)
{
    const char *string; // [esp+0h] [ebp-Ch] BYREF
    int flags; // [esp+8h] [ebp-4h]

    if (!PC_String_Parse(handle, &string))
        return 0;
    if (!I_stricmp(string, "when") || !I_stricmp(string, "if"))
    {
        flags = menu->window.dynamicFlags[0];
        Window_SetDynamicFlags(0, &menu->window, flags | 4);
        menu->visibleExp.entries = (expressionEntry **)Z_Malloc(2400, "Statement_AddOperator", 34);
        if (parse_expression(handle, &menu->visibleExp, 200))
            return 1;
    }
    if (atoi(string))
    {
        flags = menu->window.dynamicFlags[0];
        Window_SetDynamicFlags(0, &menu->window, flags | 4);
    }
    return 1;
}

int __cdecl MenuParse_onOpen(menuDef_t *menu, int handle)
{
    return PC_Script_Parse(handle, &menu->onOpen);
}

int __cdecl MenuParse_onClose(menuDef_t *menu, int handle)
{
    return PC_Script_Parse(handle, &menu->onClose);
}

int __cdecl MenuParse_onESC(menuDef_t *menu, int handle)
{
    return PC_Script_Parse(handle, &menu->onESC);
}

int __cdecl MenuParse_execExp(menuDef_t *menu, int handle)
{
    const char *expressionType; // [esp+0h] [ebp-41Ch] BYREF
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF
    int flags; // [esp+418h] [ebp-4h]

    if (!PC_String_Parse(handle, &expressionType))
        return 0;
    if (I_stricmp(expressionType, "visible"))
    {
        if (!I_stricmp(expressionType, "rect"))
        {
            if (!PC_ReadTokenHandle(handle, &token))
            {
                Com_PrintError(16, "ERROR: line ended early after \"exp rect\"\n");
                return 0;
            }
            if (I_stricmp(token.string, "X"))
            {
                if (I_stricmp(token.string, "Y"))
                {
                    Com_PrintError(16, "ERROR: Expected 'X' or 'Y' after \"exp rect\" but found \"%s\"\n", token.string);
                    return 0;
                }
                menu->rectYExp.entries = (expressionEntry **)Z_Malloc(2400, "MenuParse_execExp", 34);
                if (parse_expression(handle, &menu->rectYExp, 200))
                    return 1;
            }
            else
            {
                menu->rectXExp.entries = (expressionEntry **)Z_Malloc(2400, "MenuParse_execExp", 34);
                if (parse_expression(handle, &menu->rectXExp, 200))
                    return 1;
            }
        }
    }
    else
    {
        if (!PC_ReadTokenHandle(handle, &token))
        {
            Com_PrintError(16, "ERROR: line ended early after \"visible\"\n");
            return 0;
        }
        if (I_stricmp(token.string, "when"))
        {
            Com_PrintError(16, "ERROR: Expected 'when' after \"visible\" but found \"%s\"\n", token.string);
            return 0;
        }
        flags = menu->window.dynamicFlags[0];
        Window_SetDynamicFlags(0, &menu->window, flags | 4);
        menu->visibleExp.entries = (expressionEntry **)Z_Malloc(2400, "MenuParse_execExp", 34);
        if (parse_expression(handle, &menu->visibleExp, 200))
            return 1;
    }
    return 0;
}

int __cdecl MenuParse_border(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->window.border);
}

int __cdecl MenuParse_borderSize(menuDef_t *menu, int handle)
{
    return PC_Float_Parse(handle, &menu->window.borderSize);
}

int __cdecl MenuParse_backcolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->window.backColor);
}

int __cdecl MenuParse_forecolor(menuDef_t *menu, int handle)
{
    if (!PC_Color_Parse(handle, (float (*)[4])menu->window.foreColor))
        return 0;
    Window_SetDynamicFlags(0, &menu->window, menu->window.dynamicFlags[0] | 0x10000);
    return 1;
}

int __cdecl MenuParse_bordercolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->window.borderColor);
}

int __cdecl MenuParse_focuscolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->focusColor);
}

int __cdecl MenuParse_disablecolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->disableColor);
}

int __cdecl MenuParse_outlinecolor(menuDef_t *menu, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])menu->window.outlineColor);
}

int __cdecl MenuParse_background(menuDef_t *menu, int handle)
{
    char name[64]; // [esp+0h] [ebp-48h] BYREF
    const char *buff; // [esp+44h] [ebp-4h] BYREF

    if (!PC_String_Parse(handle, &buff))
        return 0;
    I_strncpyz(name, (char *)buff, 64);
    I_strlwr(name);
    menu->window.background = Material_RegisterHandle(name, menu->imageTrack);
    return 1;
}

int __cdecl MenuParse_ownerdraw(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->window.ownerDraw);
}

int __cdecl MenuParse_popup(menuDef_t *menu)
{
    Window_SetStaticFlags(&menu->window, menu->window.staticFlags | 0x1000000);
    return 1;
}

int __cdecl MenuParse_outOfBounds(menuDef_t *menu)
{
    Window_SetStaticFlags(&menu->window, menu->window.staticFlags | 0x2000000);
    return 1;
}

int __cdecl MenuParse_soundLoop(menuDef_t *menu, int handle)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    if (token.string[0])
        menu->soundName = String_Alloc(token.string);
    return 1;
}

int __cdecl MenuParse_fadeClamp(menuDef_t *menu, int handle)
{
    return PC_Float_Parse(handle, &menu->fadeClamp);
}

int __cdecl MenuParse_fadeAmount(menuDef_t *menu, int handle)
{
    return PC_Float_Parse(handle, &menu->fadeAmount);
}

int __cdecl MenuParse_fadeInAmount(menuDef_t *menu, int handle)
{
    return PC_Float_Parse(handle, &menu->fadeInAmount);
}

int __cdecl MenuParse_fadeCycle(menuDef_t *menu, int handle)
{
    return PC_Int_Parse(handle, &menu->fadeCycle);
}

void __cdecl Window_Init(windowDef_t *w)
{
    memset((unsigned __int8 *)w, 0, sizeof(windowDef_t));
    w->borderSize = 1.0;
    w->foreColor[3] = 1.0;
    w->foreColor[2] = 1.0;
    w->foreColor[1] = 1.0;
    w->foreColor[0] = 1.0;
}

void __cdecl Item_Init(itemDef_s *item, int imageTrack)
{
    memset((unsigned __int8 *)item, 0, sizeof(itemDef_s));
    item->textscale = 0.55000001;
    item->imageTrack = imageTrack;
    Window_Init(&item->window);
}

int __cdecl KeywordHash_Key_256_3855_(const char *keyword)
{
    int hash; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    hash = 0;
    for (i = 0; keyword[i]; ++i)
        hash += (i + 3855) * tolower(keyword[i]);
    return (unsigned __int8)(hash + BYTE1(hash));
}

const KeywordHashEntry<itemDef_s, 256, 3855> *__cdecl KeywordHash_Find_itemDef_s_256_3855_(
    const KeywordHashEntry<itemDef_s, 256, 3855> **table,
    const char *keyword)
{
    const KeywordHashEntry<itemDef_s, 256, 3855> *key; // [esp+Ch] [ebp-4h]

    key = table[KeywordHash_Key_256_3855_(keyword)];
    if (!key || I_stricmp(key->keyword, keyword))
        return 0;
    else
        return key;
}

int __cdecl Item_Parse(int handle, itemDef_s *item)
{
    const KeywordHashEntry<itemDef_s, 256, 3855> *key; // [esp+0h] [ebp-41Ch]
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    if (token.string[0] != 123)
        return 0;
    do
    {
        while (1)
        {
            do
            {
                if (!PC_ReadTokenHandle(handle, &token))
                {
                    PC_SourceError(handle, (char*)"end of file inside menu item\n");
                    return 0;
                }
                if (token.string[0] == 125)
                    return 1;
            } while (token.string[0] == 59);
            key = KeywordHash_Find_itemDef_s_256_3855_(itemParseKeywordHash, token.string);
            if (key)
                break;
            PC_SourceError(handle, (char *)"unknown menu item keyword %s", token.string);
        }
    } while (key->func(item, handle));
    PC_SourceError(handle, (char *)"couldn't parse menu item keyword %s", token.string);
    return 0;
}

void __cdecl Menu_FreeItemMemory(itemDef_s *item)
{
    if (!item)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 106, 0, "%s", "item");
    free_expression(&item->visibleExp);
    free_expression(&item->materialExp);
    free_expression(&item->textExp);
    free_expression(&item->rectXExp);
    free_expression(&item->rectYExp);
    free_expression(&item->rectWExp);
    free_expression(&item->rectHExp);
    free_expression(&item->forecolorAExp);
}

void __cdecl Item_InitControls(itemDef_s *item)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    if (item && item->type == 6)
    {
        item->cursorPos[0] = 0;
        listPtr = Item_GetListBoxDef(item);
        if (listPtr)
        {
            listPtr->startPos[0] = 0;
            listPtr->endPos[0] = 0;
        }
    }
}

int __cdecl MenuParse_itemDef(menuDef_t *menu, int handle)
{
    itemDef_s *item; // [esp+0h] [ebp-4h]

    if (menu->itemCount < 256)
    {
        item = (itemDef_s *)UI_Alloc(0x174u, 4);
        Item_Init(item, menu->imageTrack);
        if (!Item_Parse(handle, item))
        {
            Menu_FreeItemMemory(item);
            return 0;
        }
        Item_InitControls(item);
        item->parent = menu;
        menu->items[menu->itemCount++] = item;
    }
    return 1;
}

int __cdecl MenuParse_execKey(menuDef_t *menu, int handle)
{
    const char *action; // [esp+0h] [ebp-10h] BYREF
    __int16 keyindex; // [esp+4h] [ebp-Ch]
    char keyname; // [esp+Bh] [ebp-5h] BYREF
    ItemKeyHandler *handler; // [esp+Ch] [ebp-4h]

    if (!PC_Char_Parse(handle, &keyname))
        return 0;
    keyindex = (unsigned __int8)keyname;
    if (!PC_Script_Parse(handle, &action))
        return 0;
    handler = (ItemKeyHandler *)UI_Alloc(0xCu, 4);
    handler->key = keyindex;
    handler->action = action;
    handler->next = menu->onKey;
    menu->onKey = handler;
    return 1;
}

int __cdecl MenuParse_execKeyInt(menuDef_t *menu, int handle)
{
    const char *action; // [esp+0h] [ebp-Ch] BYREF
    int keyname; // [esp+4h] [ebp-8h] BYREF
    ItemKeyHandler *handler; // [esp+8h] [ebp-4h]

    if (!PC_Int_Parse(handle, &keyname))
        return 0;
    if (!PC_Script_Parse(handle, &action))
        return 0;
    handler = (ItemKeyHandler *)UI_Alloc(0xCu, 4);
    handler->key = keyname;
    handler->action = action;
    handler->next = menu->onKey;
    menu->onKey = handler;
    return 1;
}

int __cdecl SetItemStaticFlag(menuDef_t *menu, int handle, int flag)
{
    if (!menu)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 1150, 0, "%s", "menu");
    Window_SetStaticFlags(&menu->window, flag | menu->window.staticFlags);
    return 1;
}

int __cdecl MenuParse_blurWorld(menuDef_t *menu, int handle)
{
    if (!menu)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 1134, 0, "%s", "menu");
    if (!PC_Float_Parse(handle, &menu->blurRadius))
        return 0;
    if (menu->blurRadius >= 0.0)
        return 1;
    PC_SourceError(handle, (char*)"blur must be >= 0; %g is invalid", menu->blurRadius);
    return 0;
}

int __cdecl MenuParse_legacySplitScreenScale(menuDef_t *menu, int handle)
{
    return SetItemStaticFlag(menu, handle, 0x4000000);
}

int __cdecl MenuParse_hiddenDuringScope(menuDef_t *menu, int handle)
{
    return SetItemStaticFlag(menu, handle, 0x20000000);
}

int __cdecl MenuParse_hiddenDuringFlashbang(menuDef_t *menu, int handle)
{
    return SetItemStaticFlag(menu, handle, 0x10000000);
}

int __cdecl MenuParse_hiddenDuringUI(menuDef_t *menu, int handle)
{
    return SetItemStaticFlag(menu, handle, 0x40000000);
}

int __cdecl MenuParse_allowedBinding(menuDef_t *menu, int handle)
{
    if (menu->allowedBinding)
        PC_SourceError(handle, (char*)"Only one 'allowedBinding' is supported");
    return PC_String_Parse(handle, &menu->allowedBinding);
}

int __cdecl ItemParse_style(itemDef_s *item, int handle)
{
    return PC_Int_Parse(handle, &item->window.style);
}

int __cdecl ItemParse_ownerdrawFlag(itemDef_s *item, int handle)
{
    int i; // [esp+0h] [ebp-4h] BYREF

    if (!PC_Int_Parse(handle, &i))
        return 0;
    item->window.ownerDrawFlags |= i;
    return 1;
}

int __cdecl ItemParse_name(itemDef_s *item, int handle)
{
    return PC_String_Parse(handle, &item->window.name);
}

int __cdecl ItemParse_focusSound(itemDef_s *item, int handle)
{
    const char *temp; // [esp+0h] [ebp-4h] BYREF

    if (!PC_String_Parse(handle, &temp))
        return 0;
    item->focusSound = Com_FindSoundAlias(temp);
    return 1;
}

int __cdecl ItemParse_text(itemDef_s *item, int handle)
{
    return PC_String_Parse(handle, &item->text);
}

char *__cdecl UI_FileText(char *fileName)
{
    unsigned int len; // [esp+4h] [ebp-8h]
    int f; // [esp+8h] [ebp-4h] BYREF

    len = FS_FOpenFileByMode(fileName, &f, FS_READ);
    if (!f)
        return 0;
    if (len < 0x1000)
    {
        FS_Read((unsigned __int8 *)menuBuf1, len, f);
        menuBuf1[len] = 0;
        FS_FCloseFile(f);
        return menuBuf1;
    }
    else
    {
        FS_FCloseFile(f);
        Com_PrintError(16, "Menu file %s is larger than the %i byte buffer used to parse menu files\n", fileName, 4096);
        return 0;
    }
}

int __cdecl ItemParse_textfile(itemDef_s *item, int handle)
{
    char *newtext; // [esp+0h] [ebp-41Ch]
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    newtext = UI_FileText(token.string);
    item->text = String_Alloc(newtext);
    return 1;
}

int __cdecl ItemParse_textsavegame(itemDef_s *item)
{
    item->text = "savegameinfo";
    item->itemFlags |= 1u;
    return 1;
}

int __cdecl ItemParse_textcinematicsubtitle(itemDef_s *item)
{
    item->text = "cinematicsubtitle";
    item->itemFlags |= 2u;
    return 1;
}

int __cdecl ItemParse_group(itemDef_s *item, int handle)
{
    return PC_String_Parse(handle, &item->window.group);
}

int __cdecl ItemParse_rect(itemDef_s *item, int handle)
{
    return PC_Rect_Parse(handle, &item->window.rectClient);
}

int __cdecl ItemParse_origin(itemDef_s *item, int handle)
{
    int x; // [esp+0h] [ebp-8h] BYREF
    int y; // [esp+4h] [ebp-4h] BYREF

    if (!PC_Int_Parse(handle, &x))
        return 0;
    if (!PC_Int_Parse(handle, &y))
        return 0;
    item->window.rectClient.x = (double)x + item->window.rectClient.x;
    item->window.rectClient.y = (double)y + item->window.rectClient.y;
    return 1;
}

int __cdecl ItemParse_decoration(itemDef_s *item)
{
    Window_SetStaticFlags(&item->window, item->window.staticFlags | 0x100000);
    return 1;
}

void __cdecl Item_ValidateTypeData(itemDef_s *item, int handle)
{
    editFieldDef_s *editDef; // [esp+0h] [ebp-4h]

    if (item->typeData.listBox)
    {
        if (item->dataType != item->type)
            PC_SourceError(
                handle,
                (char*)"Attempting to change type from %d to %d.\nMove the type definition higher up in the itemDef.\n",
                item->dataType,
                item->type);
    }
    else
    {
        item->dataType = item->type;
        switch (item->type)
        {
        case 6:
            item->typeData.listBox = (listBoxDef_s *)UI_Alloc(0x154u, 4);
            break;
        case 4:
        case 9:
        case 0x10:
        case 0x12:
        case 0xB:
        case 0xE:
        case 0xA:
        case 0:
        case 0x11:
            item->typeData.listBox = (listBoxDef_s *)UI_Alloc(0x20u, 4);
            if (item->type == 4 || item->type == 16 || item->type == 9 || item->type == 18 || item->type == 17)
            {
                editDef = Item_GetEditFieldDef(item);
                if (!editDef)
                    MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 1293, 0, "%s", "editDef");
                if (!editDef->maxPaintChars)
                    editDef->maxPaintChars = 256;
            }
            break;
        case 0xC:
            item->typeData.listBox = (listBoxDef_s *)UI_Alloc(0x188u, 4);
            break;
        }
    }
}

int __cdecl ItemParse_notselectable(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        return 0;
    if (item->type == 6)
        listPtr->notselectable = 1;
    return 1;
}

int __cdecl ItemParse_noScrollBars(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        return 0;
    if (item->type == 6)
        listPtr->noScrollBars = 1;
    return 1;
}

int __cdecl ItemParse_usePaging(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        return 0;
    if (item->type == 6)
        listPtr->usePaging = 1;
    return 1;
}

int __cdecl ItemParse_autowrapped(itemDef_s *item)
{
    Window_SetStaticFlags(&item->window, item->window.staticFlags | 0x800000);
    return 1;
}

int __cdecl ItemParse_horizontalscroll(itemDef_s *item)
{
    Window_SetStaticFlags(&item->window, item->window.staticFlags | 0x200000);
    return 1;
}

int __cdecl ItemParse_type(itemDef_s *item, int handle)
{
    if (!PC_Int_Parse(handle, &item->type))
        return 0;
    Item_ValidateTypeData(item, handle);
    return 1;
}

bool __cdecl ItemParse_elementwidth(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    return listPtr && PC_Float_Parse(handle, &listPtr->elementWidth) != 0;
}

bool __cdecl ItemParse_elementheight(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    return listPtr && PC_Float_Parse(handle, &listPtr->elementHeight) != 0;
}

bool __cdecl ItemParse_special(itemDef_s *item, int handle)
{
    return PC_Float_Parse(handle, &item->special) != 0;
}

bool __cdecl ItemParse_elementtype(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    listPtr = Item_GetListBoxDef(item);
    return listPtr && PC_Int_Parse(handle, &listPtr->elementStyle) != 0;
}

int __cdecl ItemParse_columns(itemDef_s *item, int handle)
{
    int pos; // [esp+0h] [ebp-1Ch] BYREF
    int width; // [esp+4h] [ebp-18h] BYREF
    int align; // [esp+8h] [ebp-14h] BYREF
    int maxChars; // [esp+Ch] [ebp-10h] BYREF
    listBoxDef_s *listPtr; // [esp+10h] [ebp-Ch]
    int num; // [esp+14h] [ebp-8h] BYREF
    int i; // [esp+18h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        return 0;
    if (!PC_Int_Parse(handle, &num))
        return 0;
    if (num > 16)
        num = 16;
    listPtr->numColumns = num;
    for (i = 0; i < num; ++i)
    {
        if (!PC_Int_Parse(handle, &pos) || !PC_Int_Parse(handle, &width) || !PC_Int_Parse(handle, &maxChars))
            return 0;
        listPtr->columnInfo[i].pos = pos;
        listPtr->columnInfo[i].width = width;
        listPtr->columnInfo[i].maxChars = maxChars;
        if (PC_Int_ParseLine(handle, &align))
            listPtr->columnInfo[i].alignment = align;
        else
            listPtr->columnInfo[i].alignment = 0;
    }
    return 1;
}

bool __cdecl ItemParse_border(itemDef_s *item, int handle)
{
    return PC_Int_Parse(handle, &item->window.border) != 0;
}

bool __cdecl ItemParse_bordersize(itemDef_s *item, int handle)
{
    return PC_Float_Parse(handle, &item->window.borderSize) != 0;
}

int __cdecl ItemParse_visible(itemDef_s *item, int handle)
{
    const char *string; // [esp+0h] [ebp-Ch] BYREF
    int flags; // [esp+8h] [ebp-4h]

    if (!PC_String_Parse(handle, &string))
        return 0;
    if (!I_stricmp(string, "when") || !I_stricmp(string, "if"))
    {
        flags = item->window.dynamicFlags[0];
        Window_SetDynamicFlags(0, &item->window, flags | 4);
        item->visibleExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_visible", 34);
        if (parse_expression(handle, &item->visibleExp, 200))
            return 1;
    }
    if (atoi(string))
    {
        flags = item->window.dynamicFlags[0];
        Window_SetDynamicFlags(0, &item->window, flags | 4);
    }
    return 1;
}

int __cdecl ItemParse_ownerdraw(itemDef_s *item, int handle)
{
    if (!PC_Int_Parse(handle, &item->window.ownerDraw))
        return 0;
    item->type = 8;
    return 1;
}

bool __cdecl ItemParse_align(itemDef_s *item, int handle)
{
    return PC_Int_Parse(handle, &item->alignment) != 0;
}

bool __cdecl ItemParse_IsValidTextAlignment(unsigned int textAlignMode)
{
    return textAlignMode < 0x10 && (textAlignMode & 3) != 3;
}

int __cdecl ItemParse_textalign(itemDef_s *item, int handle)
{
    if (!PC_Int_Parse(handle, &item->textAlignMode))
        return 0;
    if (ItemParse_IsValidTextAlignment(item->textAlignMode))
        return 1;
    PC_SourceError(handle, (char*)"expected ITEM_ALIGN_* value\n");
    return 0;
}

bool __cdecl ItemParse_textalignx(itemDef_s *item, int handle)
{
    return PC_Float_Parse(handle, &item->textalignx) != 0;
}

bool __cdecl ItemParse_textaligny(itemDef_s *item, int handle)
{
    return PC_Float_Parse(handle, &item->textaligny) != 0;
}

bool __cdecl ItemParse_textscale(itemDef_s *item, int handle)
{
    return PC_Float_Parse(handle, &item->textscale) != 0;
}

bool __cdecl ItemParse_textstyle(itemDef_s *item, int handle)
{
    return PC_Int_Parse(handle, &item->textStyle) != 0;
}

bool __cdecl ItemParse_textfont(itemDef_s *item, int handle)
{
    return PC_Int_Parse(handle, &item->fontEnum) != 0;
}

int __cdecl ItemParse_backcolor(itemDef_s *item, int handle)
{
    float f; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        if (!PC_Float_Parse(handle, &f))
            return 0;
        item->window.backColor[i] = f;
    }
    return 1;
}

int __cdecl ItemParse_forecolor(itemDef_s *item, int handle)
{
    float f; // [esp+0h] [ebp-Ch] BYREF
    int i; // [esp+4h] [ebp-8h]
    int flags; // [esp+8h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        if (!PC_Float_Parse(handle, &f))
            return 0;
        item->window.foreColor[i] = f;
        flags = item->window.dynamicFlags[0];
        Window_SetDynamicFlags(0, &item->window, flags | 0x10000);
    }
    return 1;
}

int __cdecl ItemParse_bordercolor(itemDef_s *item, int handle)
{
    float f; // [esp+0h] [ebp-8h] BYREF
    int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        if (!PC_Float_Parse(handle, &f))
            return 0;
        item->window.borderColor[i] = f;
    }
    return 1;
}

bool __cdecl ItemParse_outlinecolor(itemDef_s *item, int handle)
{
    return PC_Color_Parse(handle, (float (*)[4])item->window.outlineColor) != 0;
}

int __cdecl ItemParse_background(itemDef_s *item, int handle)
{
    char name[64]; // [esp+0h] [ebp-48h] BYREF
    const char *temp; // [esp+44h] [ebp-4h] BYREF

    if (!PC_String_Parse(handle, &temp))
        return 0;
    I_strncpyz(name, (char *)temp, 64);
    I_strlwr(name);
    item->window.background = Material_RegisterHandle(name, item->imageTrack);
    return 1;
}

bool __cdecl ItemParse_doubleClick(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    listPtr = Item_GetListBoxDef(item);
    return listPtr && PC_Script_Parse(handle, &listPtr->doubleClick) != 0;
}

bool __cdecl ItemParse_onFocus(itemDef_s *item, int handle)
{
    return PC_Script_Parse(handle, &item->onFocus) != 0;
}

bool __cdecl ItemParse_leaveFocus(itemDef_s *item, int handle)
{
    return PC_Script_Parse(handle, &item->leaveFocus) != 0;
}

bool __cdecl ItemParse_mouseEnter(itemDef_s *item, int handle)
{
    return PC_Script_Parse(handle, &item->mouseEnter) != 0;
}

bool __cdecl ItemParse_mouseExit(itemDef_s *item, int handle)
{
    return PC_Script_Parse(handle, &item->mouseExit) != 0;
}

bool __cdecl ItemParse_mouseEnterText(itemDef_s *item, int handle)
{
    return PC_Script_Parse(handle, &item->mouseEnterText) != 0;
}

bool __cdecl ItemParse_mouseExitText(itemDef_s *item, int handle)
{
    return PC_Script_Parse(handle, &item->mouseExitText) != 0;
}

bool __cdecl ItemParse_action(itemDef_s *item, int handle)
{
    return PC_Script_Parse(handle, &item->action) != 0;
}

bool __cdecl ItemParse_accept(itemDef_s *item, int handle)
{
    return PC_Script_Parse(handle, &item->onAccept) != 0;
}

bool __cdecl ItemParse_dvarTest(itemDef_s *item, int handle)
{
    return PC_String_Parse(handle, &item->dvarTest) != 0;
}

int __cdecl ItemParse_dvar(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if (!PC_String_Parse(handle, &item->dvar))
        return 0;
    if (item->typeData.listBox && Item_IsEditFieldDef(item))
    {
        editPtr = Item_GetEditFieldDef(item);
        if (!editPtr)
            MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 1984, 0, "%s", "editPtr");
        editPtr->minVal = -1.0;
        editPtr->maxVal = -1.0;
        editPtr->defVal = -1.0;
    }
    return 1;
}

int __cdecl ItemParse_maxChars(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-8h]
    int maxChars; // [esp+4h] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    if (!PC_Int_Parse(handle, &maxChars))
        return 0;
    editPtr = Item_GetEditFieldDef(item);
    if (!editPtr)
        return 0;
    editPtr->maxChars = maxChars;
    return 1;
}

int __cdecl ItemParse_maxPaintChars(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-8h]
    int maxChars; // [esp+4h] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    if (!PC_Int_Parse(handle, &maxChars))
        return 0;
    editPtr = Item_GetEditFieldDef(item);
    if (!editPtr)
        return 0;
    editPtr->maxPaintChars = maxChars;
    return 1;
}

bool __cdecl ItemParse_dvarFloat(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    editPtr = Item_GetEditFieldDef(item);
    if (!editPtr)
        return 0;
    return PC_String_Parse(handle, &item->dvar)
        && PC_Float_Parse(handle, &editPtr->defVal)
        && PC_Float_Parse(handle, &editPtr->minVal)
        && PC_Float_Parse(handle, &editPtr->maxVal);
}

int __cdecl ItemParse_dvarStrList(itemDef_s *item, int handle)
{
    int pass; // [esp+0h] [ebp-420h]
    multiDef_s *multiPtr; // [esp+4h] [ebp-41Ch]
    pc_token_s token; // [esp+8h] [ebp-418h] BYREF

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    if (item->type != 12)
        return 0;
    multiPtr = Item_GetMultiDef(item);
    if (!multiPtr)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 2082, 1, "%s", "multiPtr");
    multiPtr->count = 0;
    multiPtr->strDef = 1;
    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    if (token.string[0] != 123)
        return 0;
    pass = 0;
    do
    {
        while (1)
        {
            do
            {
                if (!PC_ReadTokenHandle(handle, &token))
                {
                    PC_SourceError(handle, (char*)"end of file inside menu item\n");
                    return 0;
                }
                if (token.string[0] == 125)
                    return 1;
            } while (token.string[0] == 44 || token.string[0] == 59);
            if (pass)
                break;
            multiPtr->dvarList[multiPtr->count] = String_Alloc(token.string);
            pass = 1;
        }
        multiPtr->dvarStr[multiPtr->count] = String_Alloc(token.string);
        pass = 0;
        ++multiPtr->count;
    } while (multiPtr->count < 32);
    return 0;
}

int __cdecl ItemParse_dvarFloatList(itemDef_s *item, int handle)
{
    multiDef_s *multiPtr; // [esp+0h] [ebp-41Ch]
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    if (item->type != 12)
        return 0;
    multiPtr = Item_GetMultiDef(item);
    if (!multiPtr)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 2143, 1, "%s", "multiPtr");
    multiPtr->count = 0;
    multiPtr->strDef = 0;
    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    if (token.string[0] != 123)
        return 0;
    do
    {
        do
        {
            if (!PC_ReadTokenHandle(handle, &token))
            {
                PC_SourceError(handle, (char*)"end of file inside menu item\n");
                return 0;
            }
            if (token.string[0] == 125)
                return 1;
        } while (token.string[0] == 44 || token.string[0] == 59);
        multiPtr->dvarList[multiPtr->count] = String_Alloc(token.string);
        if (!PC_Float_Parse(handle, &multiPtr->dvarValue[multiPtr->count]))
            return 0;
        ++multiPtr->count;
    } while (multiPtr->count < 32);
    return 0;
}

int __cdecl ItemParse_dvarEnumList(itemDef_s *item, int handle)
{
    Item_ValidateTypeData(item, handle);
    if (item->type != 13)
        return 0;
    if (!item->typeData.listBox)
        return PC_String_Parse(handle, (const char **)&item->typeData);
    PC_SourceError(handle, (char*)"enumDvarList already given");
    return 0;
}

int __cdecl ItemParse_enableDvar(itemDef_s *item, int handle)
{
    if (!PC_Script_Parse(handle, &item->enableDvar))
        return 0;
    item->dvarFlags |= 1u;
    return 1;
}

int __cdecl ItemParse_disableDvar(itemDef_s *item, int handle)
{
    if (!PC_Script_Parse(handle, &item->enableDvar))
        return 0;
    item->dvarFlags |= 2u;
    return 1;
}

int __cdecl ItemParse_showDvar(itemDef_s *item, int handle)
{
    if (!PC_Script_Parse(handle, &item->enableDvar))
        return 0;
    item->dvarFlags |= 4u;
    return 1;
}

int __cdecl ItemParse_hideDvar(itemDef_s *item, int handle)
{
    if (!PC_Script_Parse(handle, &item->enableDvar))
        return 0;
    item->dvarFlags |= 8u;
    return 1;
}

int __cdecl ItemParse_focusDvar(itemDef_s *item, int handle)
{
    if (!PC_Script_Parse(handle, &item->enableDvar))
        return 0;
    item->dvarFlags |= 0x10u;
    return 1;
}

int __cdecl ItemParse_execKey(itemDef_s *item, int handle)
{
    const char *action; // [esp+0h] [ebp-10h] BYREF
    __int16 keyindex; // [esp+4h] [ebp-Ch]
    char keyname; // [esp+Bh] [ebp-5h] BYREF
    ItemKeyHandler *handler; // [esp+Ch] [ebp-4h]

    if (!PC_Char_Parse(handle, &keyname))
        return 0;
    keyindex = (unsigned __int8)keyname;
    if (!PC_Script_Parse(handle, &action))
        return 0;
    handler = (ItemKeyHandler *)UI_Alloc(0xCu, 4);
    handler->key = keyindex;
    handler->action = action;
    handler->next = item->onKey;
    item->onKey = handler;
    return 1;
}

int __cdecl ItemParse_execKeyInt(itemDef_s *item, int handle)
{
    const char *action; // [esp+0h] [ebp-Ch] BYREF
    int keyname; // [esp+4h] [ebp-8h] BYREF
    ItemKeyHandler *handler; // [esp+8h] [ebp-4h]

    if (!PC_Int_Parse(handle, &keyname))
        return 0;
    if (!PC_Script_Parse(handle, &action))
        return 0;
    handler = (ItemKeyHandler *)UI_Alloc(0xCu, 4);
    handler->key = keyname;
    handler->action = action;
    handler->next = item->onKey;
    item->onKey = handler;
    return 1;
}

int __cdecl ItemParse_execExp(itemDef_s *item, int handle)
{
    const char *expressionType; // [esp+0h] [ebp-8h] BYREF
    int flags; // [esp+4h] [ebp-4h]

    if (!PC_String_Parse(handle, &expressionType))
        return 0;
    if (I_stricmp(expressionType, "visible"))
    {
        if (I_stricmp(expressionType, "text"))
        {
            if (I_stricmp(expressionType, "material"))
            {
                if (I_stricmp(expressionType, "rect"))
                {
                    if (!I_stricmp(expressionType, "forecolor"))
                    {
                        if (!PC_String_Parse(handle, &expressionType))
                        {
                            Com_PrintError(16, "ERROR: line ended early after \"exp forecolor\"\n");
                            return 0;
                        }
                        if (I_stricmp(expressionType, "A"))
                        {
                            Com_PrintError(16, "ERROR: Expected 'A' after \"exp forecolor\" but found \"%s\"\n", expressionType);
                            return 0;
                        }
                        item->forecolorAExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_execExp", 34);
                        if (parse_expression(handle, &item->forecolorAExp, 200))
                            return 1;
                    }
                }
                else
                {
                    if (!PC_String_Parse(handle, &expressionType))
                    {
                        Com_PrintError(16, "ERROR: line ended early after \"exp rect\"\n");
                        return 0;
                    }
                    if (I_stricmp(expressionType, "X"))
                    {
                        if (!I_stricmp(expressionType, "Y"))
                        {
                            item->rectYExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_execExp", 34);
                            if (parse_expression(handle, &item->rectYExp, 200))
                                return 1;
                        }
                    }
                    else
                    {
                        item->rectXExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_execExp", 34);
                        if (parse_expression(handle, &item->rectXExp, 200))
                            return 1;
                    }
                    if (I_stricmp(expressionType, "W"))
                    {
                        if (I_stricmp(expressionType, "H"))
                        {
                            Com_PrintError(
                                16,
                                "ERROR: Expected 'X', 'Y', 'W', or 'H' after \"exp rect\" but found \"%s\"\n",
                                expressionType);
                            return 0;
                        }
                        item->rectHExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_execExp", 34);
                        if (parse_expression(handle, &item->rectHExp, 200))
                            return 1;
                    }
                    else
                    {
                        item->rectWExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_execExp", 34);
                        if (parse_expression(handle, &item->rectWExp, 200))
                            return 1;
                    }
                }
            }
            else
            {
                item->materialExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_execExp", 34);
                if (parse_expression(handle, &item->materialExp, 200))
                    return 1;
            }
        }
        else
        {
            item->textExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_execExp", 34);
            if (parse_expression(handle, &item->textExp, 200))
                return 1;
        }
    }
    else
    {
        flags = item->window.dynamicFlags[0];
        Window_SetDynamicFlags(0, &item->window, flags | 4);
        item->visibleExp.entries = (expressionEntry **)Z_Malloc(2400, "ItemParse_execExp", 34);
        if (parse_expression(handle, &item->visibleExp, 200))
            return 1;
    }
    return 0;
}

int __cdecl ItemParse_gameMsgWindowIndex(itemDef_s *item, int handle)
{
    if (!item)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 2406, 0, "%s", "item");
    return PC_Int_Parse(handle, &item->gameMsgWindowIndex);
}

int __cdecl ItemParse_gameMsgWindowMode(itemDef_s *item, int handle)
{
    if (!item)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 2413, 0, "%s", "item");
    return PC_Int_Parse(handle, &item->gameMsgWindowMode);
}

int __cdecl ItemParse_selectBorder(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if (listPtr)
        return PC_Color_Parse(handle, (float (*)[4])listPtr->selectBorder);
    else
        return 0;
}

int __cdecl ItemParse_disableColor(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if (listPtr)
        return PC_Color_Parse(handle, (float (*)[4])listPtr->disableColor);
    else
        return 0;
}

int __cdecl ItemParse_selectIcon(itemDef_s *item, int handle)
{
    listBoxDef_s *listPtr; // [esp+0h] [ebp-4Ch]
    char name[64]; // [esp+4h] [ebp-48h] BYREF
    const char *temp; // [esp+48h] [ebp-4h] BYREF

    Item_ValidateTypeData(item, handle);
    listPtr = Item_GetListBoxDef(item);
    if (!listPtr)
        return 0;
    if (!PC_String_Parse(handle, &temp))
        return 0;
    I_strncpyz(name, (char *)temp, 64);
    I_strlwr(name);
    listPtr->selectIcon = Material_RegisterHandle(name, item->imageTrack);
    return 1;
}

int __cdecl Item_Parse_maxCharsGotoNext(itemDef_s *item, int handle)
{
    editFieldDef_s *editPtr; // [esp+0h] [ebp-4h]

    Item_ValidateTypeData(item, handle);
    if (!item->typeData.listBox)
        return 0;
    editPtr = Item_GetEditFieldDef(item);
    if (!editPtr)
        return 0;
    editPtr->maxCharsGotoNext = 1;
    return 1;
}


const KeywordHashEntry<menuDef_t, 128, 128> menuParseKeywords[36] =
{
  { "name", &MenuParse_name },
  { "fullscreen", &MenuParse_fullscreen },
  { "rect", &MenuParse_rect },
  { "style", (int(__cdecl *)(menuDef_t*, int))&ItemParse_style},
  { "visible", &MenuParse_visible },
  { "onOpen", &MenuParse_onOpen },
  { "onClose", &MenuParse_onClose },
  { "onESC", &MenuParse_onESC },
  { "border", &MenuParse_border },
  { "borderSize", &MenuParse_borderSize },
  { "backcolor", &MenuParse_backcolor },
  { "forecolor", &MenuParse_forecolor },
  { "bordercolor", &MenuParse_bordercolor },
  { "focuscolor", &MenuParse_focuscolor },
  { "disablecolor", &MenuParse_disablecolor },
  { "outlinecolor", &MenuParse_outlinecolor },
  { "background", &MenuParse_background },
  { "ownerdraw", &MenuParse_ownerdraw },
  { "ownerdrawFlag", (int(__cdecl *)(menuDef_t *, int)) &ItemParse_ownerdrawFlag },
  { "outOfBoundsClick", (int(__cdecl *)(menuDef_t *, int)) &MenuParse_outOfBounds },
  { "soundLoop", &MenuParse_soundLoop },
  { "itemDef", &MenuParse_itemDef },
  { "exp", &MenuParse_execExp },
  { "popup", (int(__cdecl *)(menuDef_t *, int)) &MenuParse_popup },
  { "fadeClamp", &MenuParse_fadeClamp },
  { "fadeCycle", &MenuParse_fadeCycle },
  { "fadeAmount", &MenuParse_fadeAmount },
  { "fadeInAmount", &MenuParse_fadeInAmount },
  { "execKey", &MenuParse_execKey },
  { "execKeyInt", &MenuParse_execKeyInt },
  { "blurWorld", &MenuParse_blurWorld },
  { "legacySplitScreenScale", &MenuParse_legacySplitScreenScale },
  { "hiddenDuringScope", &MenuParse_hiddenDuringScope },
  { "hiddenDuringFlashbang", &MenuParse_hiddenDuringFlashbang },
  { "hiddenDuringUI", &MenuParse_hiddenDuringUI },
  { "allowedBinding", &MenuParse_allowedBinding }
}; // idb
const KeywordHashEntry<itemDef_s, 256, 3855> itemParseKeywords[71] =
{
  { "name", &ItemParse_name },
  { "text", &ItemParse_text },
  { "textfile", &ItemParse_textfile },
  { "textsavegame", (int(__cdecl *)(itemDef_s *, int))&ItemParse_textsavegame },
  { "textcinematicsubtitle",(int(__cdecl *)(itemDef_s *, int))&ItemParse_textcinematicsubtitle },
  { "group", &ItemParse_group },
  { "rect", &ItemParse_rect },
  { "origin", &ItemParse_origin },
  { "style", &ItemParse_style },
  { "decoration", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_decoration },
  { "notselectable", &ItemParse_notselectable },
  { "noscrollbars", &ItemParse_noScrollBars },
  { "usepaging", &ItemParse_usePaging },
  { "autowrapped", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_autowrapped },
  { "horizontalscroll", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_horizontalscroll },
  { "type", &ItemParse_type },
  { "elementwidth", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_elementwidth },
  { "elementheight", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_elementheight },
  { "feeder",(int(__cdecl *)(itemDef_s *, int)) &ItemParse_special },
  { "elementtype", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_elementtype },
  { "columns", &ItemParse_columns },
  { "border", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_border },
  { "bordersize", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_bordersize },
  { "visible", &ItemParse_visible },
  { "ownerdraw", &ItemParse_ownerdraw },
  { "align", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_align },
  { "textalign", &ItemParse_textalign },
  { "textalignx", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_textalignx },
  { "textaligny", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_textaligny },
  { "textscale", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_textscale },
  { "textstyle", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_textstyle },
  { "textfont", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_textfont },
  { "backcolor", &ItemParse_backcolor },
  { "forecolor", &ItemParse_forecolor },
  { "bordercolor", &ItemParse_bordercolor },
  { "outlinecolor", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_outlinecolor },
  { "background", &ItemParse_background },
  { "onFocus",(int(__cdecl *)(itemDef_s *, int)) &ItemParse_onFocus },
  { "leaveFocus", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_leaveFocus },
  { "mouseEnter", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_mouseEnter },
  { "mouseExit",(int(__cdecl *)(itemDef_s *, int)) &ItemParse_mouseExit },
  { "mouseEnterText",(int(__cdecl *)(itemDef_s *, int)) &ItemParse_mouseEnterText },
  { "mouseExitText", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_mouseExitText },
  { "action", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_action },
  { "accept", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_accept },
  { "special", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_special },
  { "dvar", &ItemParse_dvar },
  { "maxChars", &ItemParse_maxChars },
  { "maxCharsGotoNext", &Item_Parse_maxCharsGotoNext },
  { "maxPaintChars", &ItemParse_maxPaintChars },
  { "focusSound", &ItemParse_focusSound },
  { "dvarFloat", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_dvarFloat },
  { "dvarStrList", &ItemParse_dvarStrList },
  { "dvarFloatList", &ItemParse_dvarFloatList },
  { "dvarEnumList", &ItemParse_dvarEnumList },
  { "ownerdrawFlag", &ItemParse_ownerdrawFlag },
  { "enableDvar", &ItemParse_enableDvar },
  { "dvarTest", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_dvarTest },
  { "disableDvar", &ItemParse_disableDvar },
  { "showDvar", &ItemParse_showDvar },
  { "hideDvar", &ItemParse_hideDvar },
  { "focusDvar", &ItemParse_focusDvar },
  { "doubleclick", (int(__cdecl *)(itemDef_s *, int)) &ItemParse_doubleClick },
  { "execKey", &ItemParse_execKey },
  { "execKeyInt", &ItemParse_execKeyInt },
  { "exp", &ItemParse_execExp },
  { "gamemsgwindowindex", &ItemParse_gameMsgWindowIndex },
  { "gamemsgwindowmode", &ItemParse_gameMsgWindowMode },
  { "selectBorder", &ItemParse_selectBorder },
  { "disablecolor", &ItemParse_disableColor },
  { "selectIcon", &ItemParse_selectIcon }
}; // idb

int __cdecl KeywordHash_KeySeed(const char *keyword, int hashCount, int seed)
{
    int hash; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    hash = 0;
    for (i = 0; keyword[i]; ++i)
        hash += (i + seed) * tolower(keyword[i]);
    return (hashCount - 1) & (hash + (hash >> 8));
}

char __cdecl KeywordHash_IsValidSeed_itemDef_s_256_3855_(
    const KeywordHashEntry<itemDef_s, 256, 3855> *array,
    int count,
    int seed)
{
    unsigned __int8 used[260]; // [esp+0h] [ebp-110h] BYREF
    int hash; // [esp+108h] [ebp-8h]
    int index; // [esp+10Ch] [ebp-4h]

    memset(used, 0, 0x100u);
    for (index = 0; index < count; ++index)
    {
        hash = KeywordHash_KeySeed(array[index].keyword, 256, seed);
        if (used[hash])
            return 0;
        used[hash] = 1;
    }
    return 1;
}

void __cdecl KeywordHash_Add_itemDef_s_256_3855_(
    const KeywordHashEntry<itemDef_s, 256, 3855> **table,
    const KeywordHashEntry<itemDef_s, 256, 3855> *key)
{
    int hash; // [esp+8h] [ebp-4h]

    hash = KeywordHash_Key_256_3855_(key->keyword);
    if (table[hash])
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 695, 0, "%s", "table[hash] == NULL");
    table[hash] = key;
}

int __cdecl KeywordHash_PickSeed_itemDef_s_256_3855_(const KeywordHashEntry<itemDef_s, 256, 3855> *array, int count)
{
    int seed; // [esp+0h] [ebp-4h]

    for (seed = 0; !KeywordHash_IsValidSeed_itemDef_s_256_3855_(array, count, seed); ++seed)
    {
        if (seed == 0x10000)
            MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 673, 0, "seed != 65536\n\t%i, %i", 0x10000, 0x10000);
    }
    return seed;
}

void __cdecl KeywordHash_Validate_itemDef_s_256_3855_(const KeywordHashEntry<itemDef_s, 256, 3855> *array, int count)
{
    int v2; // eax

    if (!KeywordHash_IsValidSeed_itemDef_s_256_3855_(array, count, 3855))
    {
        v2 = KeywordHash_PickSeed_itemDef_s_256_3855_(array, count);
        MyAssertHandler(
            ".\\ui\\ui_shared_obj.cpp",
            685,
            0,
            "%s\n\t(KeywordHash_PickSeed( array, count )) = %i",
            "(KeywordHash_IsValidSeed( array, count, HASH_SEED ))",
            v2);
    }
}

void __cdecl Item_SetupKeywordHash()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    KeywordHash_Validate_itemDef_s_256_3855_(itemParseKeywords, 71);
    memset((unsigned __int8 *)itemParseKeywordHash, 0, sizeof(itemParseKeywordHash));
    for (i = 0; i < 0x47; ++i)
        KeywordHash_Add_itemDef_s_256_3855_(
            itemParseKeywordHash,
            (const KeywordHashEntry<itemDef_s, 256, 3855> *)(8 * i + 9147016));
}

char __cdecl KeywordHash_IsValidSeed_menuDef_t_128_128_(
    const KeywordHashEntry<menuDef_t, 128, 128> *array,
    int count,
    int seed)
{
    unsigned __int8 used[132]; // [esp+0h] [ebp-90h] BYREF
    int hash; // [esp+88h] [ebp-8h]
    int index; // [esp+8Ch] [ebp-4h]

    memset(used, 0, 0x80u);
    for (index = 0; index < count; ++index)
    {
        hash = KeywordHash_KeySeed(array[index].keyword, 128, seed);
        if (used[hash])
            return 0;
        used[hash] = 1;
    }
    return 1;
}

int __cdecl KeywordHash_PickSeed_menuDef_t_128_128_(const KeywordHashEntry<menuDef_t, 128, 128> *array, int count)
{
    int seed; // [esp+0h] [ebp-4h]

    for (seed = 0; !KeywordHash_IsValidSeed_menuDef_t_128_128_(array, count, seed); ++seed)
    {
        if (seed == 0x10000)
            MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 673, 0, "seed != 65536\n\t%i, %i", 0x10000, 0x10000);
    }
    return seed;
}

void __cdecl KeywordHash_Validate_menuDef_t_128_128_(const KeywordHashEntry<menuDef_t, 128, 128> *array, int count)
{
    int v2; // eax

    if (!KeywordHash_IsValidSeed_menuDef_t_128_128_(array, count, 128))
    {
        v2 = KeywordHash_PickSeed_menuDef_t_128_128_(array, count);
        MyAssertHandler(
            ".\\ui\\ui_shared_obj.cpp",
            685,
            0,
            "%s\n\t(KeywordHash_PickSeed( array, count )) = %i",
            "(KeywordHash_IsValidSeed( array, count, HASH_SEED ))",
            v2);
    }
}

int __cdecl KeywordHash_Key_128_128_(const char *keyword)
{
    int hash; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    hash = 0;
    for (i = 0; keyword[i]; ++i)
        hash += (i + 128) * tolower(keyword[i]);
    return ((_BYTE)hash + BYTE1(hash)) & 0x7F;
}

void __cdecl KeywordHash_Add_menuDef_t_128_128_(
    const KeywordHashEntry<menuDef_t, 128, 128> **table,
    const KeywordHashEntry<menuDef_t, 128, 128> *key)
{
    int hash; // [esp+8h] [ebp-4h]

    hash = KeywordHash_Key_128_128_(key->keyword);
    if (table[hash])
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 695, 0, "%s", "table[hash] == NULL");
    table[hash] = key;
}

void __cdecl Menu_SetupKeywordHash()
{
    unsigned int i; // [esp+0h] [ebp-4h]

    KeywordHash_Validate_menuDef_t_128_128_(menuParseKeywords, 36);
    memset((unsigned __int8 *)menuParseKeywordHash, 0, sizeof(menuParseKeywordHash));
    for (i = 0; i < 0x24; ++i)
        KeywordHash_Add_menuDef_t_128_128_(
            menuParseKeywordHash,
            (const KeywordHashEntry<menuDef_t, 128, 128> *)(8 * i + 9146728));
}

void __cdecl Menu_Init(menuDef_t *menu, int imageTrack)
{
    memset((unsigned __int8 *)menu, 0, sizeof(menuDef_t));
    Menu_SetCursorItem(0, menu, -1);
    menu->fadeAmount = g_load_0.loadAssets.fadeAmount;
    menu->fadeInAmount = g_load_0.loadAssets.fadeInAmount;
    menu->fadeClamp = g_load_0.loadAssets.fadeClamp;
    menu->fadeCycle = g_load_0.loadAssets.fadeCycle;
    menu->imageTrack = imageTrack;
    menu->items = g_load_0.items;
    Window_Init(&menu->window);
}

const KeywordHashEntry<menuDef_t, 128, 128> *__cdecl KeywordHash_Find_menuDef_t_128_128_(
    const KeywordHashEntry<menuDef_t, 128, 128> **table,
    const char *keyword)
{
    const KeywordHashEntry<menuDef_t, 128, 128> *key; // [esp+Ch] [ebp-4h]

    key = table[KeywordHash_Key_128_128_(keyword)];
    if (!key || I_stricmp(key->keyword, keyword))
        return 0;
    else
        return key;
}

int __cdecl Menu_Parse(int handle, menuDef_t *menu)
{
    const KeywordHashEntry<menuDef_t, 128, 128> *key; // [esp+0h] [ebp-41Ch]
    pc_token_s token; // [esp+4h] [ebp-418h] BYREF

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    if (token.string[0] != 123)
        return 0;
    do
    {
        while (1)
        {
            do
            {
                memset((unsigned __int8 *)&token, 0, sizeof(token));
                if (!PC_ReadTokenHandle(handle, &token))
                {
                    PC_SourceError(handle, (char*)"end of file inside menu\n");
                    return 0;
                }
                if (token.string[0] == 125)
                    return 1;
            } while (token.string[0] == 59);
            key = KeywordHash_Find_menuDef_t_128_128_(menuParseKeywordHash, token.string);
            if (key)
                break;
            PC_SourceError(handle, (char*)"unknown menu keyword %s", token.string);
        }
    } while (key->func(menu, handle));
    PC_SourceError(handle, (char*)"couldn't parse menu keyword %s", token.string);
    return 0;
}

void __cdecl Menu_PostParse(menuDef_t *menu)
{
    unsigned int size; // [esp+0h] [ebp-4h]

    if (!menu)
        MyAssertHandler(".\\ui\\ui_shared_obj.cpp", 2653, 0, "%s", "menu");
    size = 4 * menu->itemCount;
    menu->items = (itemDef_s **)UI_Alloc(size, 4);
    memcpy((unsigned __int8 *)menu->items, (unsigned __int8 *)g_load_0.items, size);
    if (menu->fullScreen)
    {
        menu->window.rect.x = 0.0;
        menu->window.rect.y = 0.0;
        menu->window.rect.w = 640.0;
        menu->window.rect.h = 480.0;
    }
    Menu_UpdatePosition(0, menu);
}

char __cdecl Menu_New(int handle, int imageTrack)
{
    menuDef_t *menu; // [esp+0h] [ebp-4h]

    menu = (menuDef_t *)UI_Alloc(0x11Cu, 4);
    Menu_Init(menu, imageTrack);
    if (Menu_Parse(handle, menu))
    {
        if (menu->window.name)
        {
            Menu_PostParse(menu);
            if (g_load_0.menuList.menuCount >= 512)
                Com_Error(ERR_DROP, "Menu_New: (Kisak) Out of memory"); // KISAKTODO: proper err msg... IDA shows it as hex and cba
            g_load_0.menuList.menus[g_load_0.menuList.menuCount++] = menu;
            return 1;
        }
        else
        {
            PC_SourceError(handle, (char*)"menu has no name");
            Menu_FreeMemory(menu);
            return 0;
        }
    }
    else
    {
        Menu_FreeMemory(menu);
        return 0;
    }
}

int __cdecl Asset_Parse(int handle)
{
    pc_token_s token; // [esp+0h] [ebp-418h] BYREF

    if (!PC_ReadTokenHandle(handle, &token))
        return 0;
    if (I_stricmp(token.string, "{"))
        return 0;
    do
    {
        while (1)
        {
            while (1)
            {
                while (1)
                {
                    while (1)
                    {
                        do
                        {
                            if (!PC_ReadTokenHandle(handle, &token))
                                return 0;
                        } while (!I_stricmp(token.string, ";"));
                        if (!I_stricmp(token.string, "}"))
                            return 1;
                        if (I_stricmp(token.string, "fadeClamp"))
                            break;
                        if (!PC_Float_Parse(handle, &g_load_0.loadAssets.fadeClamp))
                            return 0;
                    }
                    if (I_stricmp(token.string, "fadeCycle"))
                        break;
                    if (!PC_Int_Parse(handle, &g_load_0.loadAssets.fadeCycle))
                        return 0;
                }
                if (I_stricmp(token.string, "fadeAmount"))
                    break;
                if (!PC_Float_Parse(handle, &g_load_0.loadAssets.fadeAmount))
                    return 0;
            }
            if (!I_stricmp(token.string, "fadeInAmount"))
                break;
            PC_SourceError(
                handle,
                (char*)"Unknown token %s in assetGlobalDef.  Valid commands are 'fadeClamp', 'fadeCycle', 'fadeAmount', and 'fadeInAmount'\n",
                token.string);
        }
    } while (PC_Float_Parse(handle, &g_load_0.loadAssets.fadeInAmount));
    return 0;
}

char __cdecl UI_ParseMenuInternal(char *menuFile, int imageTrack)
{
    int handle; // [esp+0h] [ebp-424h]
    const char *builtinDefines[2]; // [esp+4h] [ebp-420h] BYREF
    pc_token_s token; // [esp+Ch] [ebp-418h] BYREF

    builtinDefines[0] = "PC";
    builtinDefines[1] = 0;
    Com_Printf(13, "\tLoading '%s'...\n", menuFile);
    handle = PC_LoadSourceHandle(menuFile, builtinDefines);
    if (handle)
    {
        while (PC_ReadTokenHandle(handle, &token))
        {
            if (I_stricmp(token.string, "}") && I_stricmp(token.string, "{"))
            {
                if (I_stricmp(token.string, "assetGlobalDef"))
                {
                    if (I_stricmp(token.string, "menudef"))
                    {
                        PC_SourceError(
                            handle,
                            (char*)"Unknown token %s in menu file.  Expected \"menudef\" or \"assetglobaldef\".\n",
                            token.string);
                    }
                    else if (!Menu_New(handle, imageTrack))
                    {
                        break;
                    }
                }
                else if (!Asset_Parse(handle))
                {
                    break;
                }
            }
        }
        PC_FreeSourceHandle(handle);
        return 1;
    }
    else
    {
        Com_PrintError(13, "Couldn't find menu file '%s'\n", menuFile);
        return 0;
    }
}

MenuList *__cdecl UI_LoadMenu_LoadObj(char *menuFile, int imageTrack)
{
    memset((unsigned __int8 *)&g_load_0, 0, sizeof(g_load_0));
    g_load_0.menuList.menus = g_load_0.menus;
    if (!UI_ParseMenuInternal(menuFile, imageTrack))
    {
        Com_PrintWarning(13, "WARNING: menu file not found: %s\n", menuFile);
        if (!UI_ParseMenuInternal((char*)"ui/default.menu", imageTrack))
            Com_Error(ERR_DROP, "default.menu file not found. This is a default menu that you should have.");
    }
    return &g_load_0.menuList;
}

int __cdecl Load_Menu(const char **p, int imageTrack)
{
    parseInfo_t *token; // [esp+0h] [ebp-4h]

    if (Com_Parse(p)->token[0] != 123)
        return 0;
    while (1)
    {
        token = Com_Parse(p);
        if (!I_stricmp(token->token, "}"))
            return 1;
        if (!token || !token->token[0])
            break;
        UI_ParseMenuInternal(token->token, imageTrack);
    }
    return 0;
}

char menuBuf[32768];
MenuList *__cdecl UI_LoadMenus_LoadObj(char *menuFile, int imageTrack)
{
    int len; // [esp+0h] [ebp-10h]
    int f; // [esp+4h] [ebp-Ch] BYREF
    const char *token; // [esp+8h] [ebp-8h]
    const char *p; // [esp+Ch] [ebp-4h] BYREF

    memset((unsigned __int8 *)&g_load_0, 0, sizeof(g_load_0));
    g_load_0.menuList.menus = g_load_0.menus;
    len = FS_FOpenFileByMode(menuFile, &f, FS_READ);
    if (!f)
    {
        Com_Printf(13, "^3WARNING: menu file not found: %s\n", menuFile);
        len = FS_FOpenFileByMode((char*)"ui/default.menu", &f, FS_READ);
        if (!f)
            Com_Error(ERR_DROP, "default.menu file not found. This is a default menu that you should have.");
    }
    if (len >= 0x8000)
    {
        FS_FCloseFile(f);
        Com_Error(ERR_DROP, "^1menu file too large: %s is %i, max allowed is %i", menuFile, len, 0x8000);
    }
    FS_Read((unsigned __int8 *)menuBuf, len, f);
    menuBuf[len] = 0;
    FS_FCloseFile(f);
    Com_Compress(menuBuf);
    p = menuBuf;
    Com_BeginParseSession(menuFile);
    do
        token = (const char *)Com_Parse(&p);
    while (token
        && *token
        && *token != 125
        && I_stricmp(token, "}")
        && (I_stricmp(token, "loadmenu") || Load_Menu(&p, imageTrack)));
    Com_EndParseSession();
    return &g_load_0.menuList;
}