#include "q_shared.h"

#include "../qcommon/qcommon.h"
#include <qcommon/mem_track.h>
#include <qcommon/threads.h>

#include <EffectsCore/fx_system.h>

#include <string.h>
#include <gfx_d3d/r_model.h>
#include "com_sndalias.h"

#include <setjmp.h>

//Line 53466:  0006 : 02bc009c       int marker_q_shared      8537009c     q_shared.obj
//Line 53467 : 0006 : 02bc58d0       struct TraceThreadInfo *g_traceThreadInfo 853758d0     q_shared.obj
//Line 14278 : 0001 : 0006fb0c       struct orientation_t const orIdentity 8207010c     q_shared.obj

void I_strncat(char* dest, int size, const char* src)
{
    int destLen; // [esp+10h] [ebp-4h]

    iassert(size != sizeof(char*));
    destLen = strlen(dest);

    if (destLen >= size)
        Com_Error(ERR_FATAL, "I_strncat: already overflowed");

    I_strncpyz(&dest[destLen], src, size - destLen);
}

void I_strncpyz(char* dest, const char* src, int destsize)
{
    iassert(src);
    iassert(dest);
    iassert(destsize >= 1);

    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = 0;
}

int I_stricmp(const char* s0, const char* s1)
{
    iassert(s0);
    iassert(s1);

    return I_strnicmp(s0, s1, 0x7FFFFFFF);
}

const char *__cdecl I_stristr(const char *s0, const char *substr)
{
    int v3; // esi
    int s0Char; // [esp+4h] [ebp-8h]
    int substrChar; // [esp+8h] [ebp-4h]

    iassert(s0);
    iassert(substr);

    for (s0Char = 0; s0[s0Char]; ++s0Char)
    {
        substrChar = -1;
        do
        {
            if (!substr[++substrChar])
                return &s0[s0Char];
            v3 = tolower(s0[substrChar + s0Char]);
        } while (v3 == tolower(substr[substrChar]));
    }

    return 0;
}

int I_strnicmp(const char* s0, const char* s1, int n)
{
    int c1; // [esp+0h] [ebp-8h]
    int c0; // [esp+4h] [ebp-4h]

    do
    {
        c0 = *(unsigned __int8*)s0;
        c1 = *(unsigned __int8*)s1;
        ++s0;
        ++s1;
        if (!n--)
            return 0;
        if (c0 != c1)
        {
            if (I_isupper(c0))
                c0 += 32;
            if (I_isupper(c1))
                c1 += 32;
            if (c0 != c1)
                return 2 * (c0 >= c1) - 1;
        }
    } while (c0);
    return 0;
}

bool I_islower(int c)
{
    return c >= 'a' && c <= 'z';
}
bool I_isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}
bool I_isalpha(int c)
{
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}
bool I_iscsym(int c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    if (c < '0' || c > '9')
        return c == '_';
    return 1;
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char* QDECL va(const char* format, ...) {
    va_list		argptr;
    static char		string[2][32000];	// in case va is called by nested functions
    static int		index = 0;
    char* buf;

    buf = string[index & 1];
    index++;

    va_start(argptr, format);
    vsprintf(buf, format, argptr);
    va_end(argptr);

    return buf;
}

va_info_t va_info[7];
jmp_buf g_com_error[7];
TraceThreadInfo g_traceThreadInfo[7];

static char value1[2][2][8192];

void __cdecl TRACK_q_shared()
{
    TRACK_STATIC_ARR(va_info, 10);
    TRACK_STATIC_ARR(g_com_error, 10);
    TRACK_STATIC_ARR(g_traceThreadInfo, 10);
    TRACK_STATIC_ARR(value1, 10);
}

unsigned __int8 __cdecl ColorIndex(unsigned __int8 c)
{
    if ((unsigned __int8)(c - 48) >= 0xAu)
        return 7;
    else
        return c - 48;
}

const char *__cdecl Com_GetFilenameSubString(const char *pathname)
{
    const char *last; // [esp+0h] [ebp-4h]

    last = pathname;
    while (*pathname)
    {
        if (*pathname == 47 || *pathname == 92)
            last = pathname + 1;
        ++pathname;
    }
    return last;
}

void __cdecl Com_AssembleFilepath(char *folder, char *name, char *extension, char *path, int maxCharCount)
{
    unsigned int v5; // [esp+0h] [ebp-3Ch]
    unsigned int v6; // [esp+10h] [ebp-2Ch]
    unsigned int v7; // [esp+20h] [ebp-1Ch]
    char *patha; // [esp+50h] [ebp+14h]

    if (!folder)
        MyAssertHandler(".\\universal\\q_shared.cpp", 148, 0, "%s", "folder");
    if (!name)
        MyAssertHandler(".\\universal\\q_shared.cpp", 149, 0, "%s", "name");
    if (!extension)
        MyAssertHandler(".\\universal\\q_shared.cpp", 150, 0, "%s", "extension");
    if (!path)
        MyAssertHandler(".\\universal\\q_shared.cpp", 151, 0, "%s", "path");
    if (maxCharCount <= 0)
        MyAssertHandler(".\\universal\\q_shared.cpp", 152, 0, "%s", "maxCharCount > 0");
    v7 = strlen(folder);
    v6 = strlen(name);
    v5 = strlen(extension);
    if ((int)(v5 + v6 + v7) >= maxCharCount)
        Com_Error(ERR_DROP, "filepath '%s%s%s' is longer than %i characters", folder, name, extension, maxCharCount - 1);
    memcpy((unsigned __int8 *)path, (unsigned __int8 *)folder, v7);
    patha = &path[v7];
    memcpy((unsigned __int8 *)patha, (unsigned __int8 *)name, v6);
    memcpy((unsigned __int8 *)&patha[v6], (unsigned __int8 *)extension, v5 + 1);
}

const char *__cdecl Com_GetExtensionSubString(const char *filename)
{
    const char *substr; // [esp+0h] [ebp-4h]

    if (!filename)
        MyAssertHandler(".\\universal\\q_shared.cpp", 177, 0, "%s", "filename");
    substr = 0;
    while (*filename)
    {
        if (*filename == 46)
        {
            substr = filename;
        }
        else if (*filename == 47 || *filename == 92)
        {
            substr = 0;
        }
        ++filename;
    }
    if (!substr)
        return filename;
    return substr;
}

void __cdecl Com_StripExtension(char *in, char *out)
{
    const char *extension; // [esp+0h] [ebp-4h]

    extension = Com_GetExtensionSubString(in);
    while (in != extension)
        *out++ = *in++;
    *out = 0;
}

void __cdecl Com_DefaultExtension(char *path, unsigned int maxSize, const char *extension)
{
    char *src; // [esp+10h] [ebp-4Ch]
    char oldPath[68]; // [esp+14h] [ebp-48h] BYREF

    for (src = &path[strlen(path) - 1]; *src != 47 && src != path; --src)
    {
        if (*src == 46)
            return;
    }
    I_strncpyz(oldPath, path, 64);
    Com_sprintf(path, maxSize, "%s%s", oldPath, extension);
}

int __cdecl ShortSwap(__int16 l)
{
    return HIBYTE(l) + ((unsigned __int8)l << 8);
}

__int16 __cdecl ShortNoSwap(__int16 l)
{
    return l;
}

int __cdecl LongSwap(int l)
{
    return HIBYTE(l) + (BYTE2(l) << 8) + (BYTE1(l) << 16) + ((unsigned __int8)l << 24);
}

unsigned __int64 __cdecl Long64Swap(unsigned __int64 l)
{
    return HIBYTE(l)
        + ((unsigned __int64)BYTE6(l) << 8)
        + ((unsigned __int64)BYTE5(l) << 16)
        + ((unsigned __int64)BYTE4(l) << 24)
        + ((unsigned __int64)BYTE3(l) << 32)
        + ((unsigned __int64)BYTE2(l) << 40)
        + ((unsigned __int64)BYTE1(l) << 48)
        + ((unsigned __int64)(unsigned __int8)l << 56);
}

unsigned __int64 __cdecl Long64NoSwap(unsigned __int64 ll)
{
    return ll;
}

double __cdecl FloatReadSwap(int n)
{
    FloatReadSwap_union dat2; // [esp+4h] [ebp-4h]

    dat2.b[0] = HIBYTE(n);
    dat2.b[1] = BYTE2(n);
    dat2.b[2] = BYTE1(n);
    dat2.b[3] = n;
    return dat2.f;
}

double __cdecl FloatReadNoSwap(int n)
{
    return *(float *)&n;
}

FloatWriteSwap_union __cdecl FloatWriteSwap(float f)
{
    FloatWriteSwap_union dat2; // [esp+4h] [ebp-4h]

    dat2.b[0] = HIBYTE(f);
    dat2.b[1] = BYTE2(f);
    dat2.b[2] = BYTE1(f);
    dat2.b[3] = LOBYTE(f);
    return dat2;
}


typedef __int16 (*BigShortFn)(__int16);
BigShortFn _BigShort;

void __cdecl Swap_InitLittleEndian()
{
    _BigShort = (__int16(*)(__int16))ShortSwap;
    // LWSS: Rest are unused afaik
    //LittleShort = ShortNoSwap;
    //BigLong = LongSwap;
    //LittleLong = (int(__cdecl *)(int))LongNoSwap;
    //LittleLong64 = Long64NoSwap;
    //LittleFloatRead = (float(__cdecl *)(int))FloatReadNoSwap;
    //LittleFloatWrite = (int(__cdecl *)(float))LongNoSwap;
}

void __cdecl Swap_InitBigEndian()
{
    _BigShort = ShortNoSwap;
    // LWSS: Rest are unused afaik
    //LittleShort = (__int16(__cdecl *)(__int16))ShortSwap;
    //BigLong = (int(__cdecl *)(int))LongNoSwap;
    //LittleLong = LongSwap;
    //LittleLong64 = Long64Swap;
    //LittleFloatRead = (float(__cdecl *)(int))FloatReadSwap;
    //LittleFloatWrite = (int(__cdecl *)(float))FloatWriteSwap;
}

void __cdecl Swap_Init()
{
    Swap_InitLittleEndian();
}

bool __cdecl I_isdigit(int c)
{
    return c >= 48 && c <= 57;
}

bool __cdecl I_isalnum(int c)
{
    return I_isalpha(c) || I_isdigit(c);
}

bool __cdecl I_isforfilename(int c)
{
    return I_isalnum(c) || c == 95 || c == 45;
}

int __cdecl I_strncmp(const char *s0, const char *s1, int n)
{
    int c1; // [esp+0h] [ebp-8h]
    int c0; // [esp+4h] [ebp-4h]

    do
    {
        c0 = *s0++;
        c1 = *s1++;
        if (!n--)
            return 0;
        if (c0 != c1)
            return 2 * (c0 >= c1) - 1;
    } while (c0);
    return 0;
}

int __cdecl I_strcmp(const char *s0, const char *s1)
{
    if (!s0)
        MyAssertHandler(".\\universal\\q_shared.cpp", 649, 0, "%s", "s0");
    if (!s1)
        MyAssertHandler(".\\universal\\q_shared.cpp", 650, 0, "%s", "s1");
    return I_strncmp(s0, s1, 0x7FFFFFFF);
}

int __cdecl I_stricmpwild(const char *wild, const char *s)
{
    int v3; // esi
    char charWild; // [esp+7h] [ebp-9h]
    int delta; // [esp+8h] [ebp-8h]
    char charRef; // [esp+Fh] [ebp-1h]

    if (!wild)
        MyAssertHandler(".\\universal\\q_shared.cpp", 661, 0, "%s", "wild");
    if (!s)
        MyAssertHandler(".\\universal\\q_shared.cpp", 662, 0, "%s", "s");
    do
    {
        charWild = *wild++;
        if (charWild == 42)
        {
            if (!*wild)
                return 0;
            if (*s && !I_stricmpwild(wild - 1, s + 1))
                return 0;
        }
        else
        {
            charRef = *s++;
            if (charWild != charRef && charWild != 63)
            {
                v3 = tolower(charWild);
                delta = v3 - tolower(charRef);
                if (delta)
                    return 2 * (delta >= 0) - 1;
            }
        }
    } while (charWild);
    return 0;
}

char *__cdecl I_strlwr(char *s)
{
    char *iter; // [esp+0h] [ebp-4h]

    for (iter = s; *iter; ++iter)
    {
        if (I_isupper(*iter))
            *iter += 32;
    }
    return s;
}

char *__cdecl I_strupr(char *s)
{
    char *iter; // [esp+0h] [ebp-4h]

    for (iter = s; *iter; ++iter)
    {
        if (I_islower(*iter))
            *iter -= 32;
    }
    return s;
}

int __cdecl I_DrawStrlen(const char *str)
{
    int count; // [esp+4h] [ebp-4h]

    count = 0;
    while (*str)
    {
        if (str && *str == 94 && str[1] && str[1] != 94 && str[1] >= 48 && str[1] <= 57)
        {
            str += 2;
        }
        else
        {
            ++count;
            ++str;
        }
    }
    return count;
}

char *__cdecl I_CleanStr(char *string)
{
    char *d; // [esp+0h] [ebp-Ch]
    char c; // [esp+7h] [ebp-5h]
    char *s; // [esp+8h] [ebp-4h]

    s = string;
    d = string;
    while (1)
    {
        c = *s;
        if (!*s)
            break;
        if (s && *s == 94 && s[1] && s[1] != 94 && s[1] >= 48 && s[1] <= 57)
        {
            ++s;
        }
        else if (c >= 32 && c != 127)
        {
            *d++ = c;
        }
        ++s;
    }
    *d = 0;
    return string;
}

unsigned __int8 __cdecl I_CleanChar(unsigned __int8 character)
{
    if (character == 146)
        return 39;
    else
        return character;
}

int Com_sprintf(char *dest, unsigned int size, const char *fmt, ...)
{
    int result; // eax
    va_list va; // [esp+1Ch] [ebp+14h] BYREF

    va_start(va, fmt);
    result = _vsnprintf(dest, size, fmt, va);
    dest[size - 1] = 0;
    return result;
}

int Com_sprintfPos(char *dest, int destSize, int *destPos, const char *fmt, ...)
{
    int len; // [esp+4h] [ebp-Ch]
    char *destMod; // [esp+8h] [ebp-8h]
    unsigned int destModSize; // [esp+Ch] [ebp-4h]
    va_list va; // [esp+28h] [ebp+18h] BYREF

    va_start(va, fmt);
    if (*destPos >= destSize - 1)
        return -1;
    destMod = &dest[*destPos];
    destModSize = destSize - *destPos;
    len = _vsnprintf(destMod, destModSize, fmt, va);
    destMod[destModSize - 1] = 0;
    if (len == destModSize || len == -1)
        *destPos = destSize - 1;
    else
        *destPos += len;
    return len;
}

bool __cdecl CanKeepStringPointer(const char *string)
{
    va_info_t *info; // [esp+0h] [ebp-8h]
    char stackArray[4]; // [esp+4h] [ebp-4h] BYREF

    // KISAKTODO: re-eval
    //if (string >= stackArray && string < (char *)&STACK[0x2004])
        //return 0;

    info = (va_info_t *)Sys_GetValue(1);
    return string < (char *)info || string > &info->va_string[1][1023];
}

void __cdecl Com_InitThreadData(int threadContext)
{
    Sys_SetValue(1, &va_info[threadContext]);
    Sys_SetValue(2, &g_com_error[threadContext]);
    Sys_SetValue(3, &g_traceThreadInfo[threadContext]);
}

static int valueindex;
const char *__cdecl Info_ValueForKey(const char *s, const char *key)
{
    char *v3; // [esp+0h] [ebp-2010h]
    char *v4; // [esp+0h] [ebp-2010h]
    char *v5; // [esp+4h] [ebp-200Ch]
    char s1[8196]; // [esp+8h] [ebp-2008h] BYREF TODO: dear lord
    const char *v7; // [esp+2018h] [ebp+8h]

    if (!s || !key)
        return "";

    valueindex ^= 1u;

    if (*s == 92)
        ++s;

    while (1)
    {
        v3 = s1;
        while (*s != 92)
        {
            if (!*s)
                return "";
            *v3++ = *s++;
            if (v3 - s1 >= 0x2000)
                Com_Error(ERR_DROP, "Info_ValueForKey: oversize key %d", v3 - s1);
        }
        *v3 = 0;
        v7 = s + 1;
        if (Sys_IsMainThread())
        {
            v5 = value1[0][valueindex];
        }
        else
        {
            if (!Sys_IsRenderThread())
                MyAssertHandler(".\\universal\\q_shared.cpp", 1026, 0, "%s", "Sys_IsRenderThread()");
            v5 = (char *)((valueindex << 13) + 231767416); // TODO: what
        }
        v4 = v5;
        while (*v7 != 92 && *v7)
        {
            *v4++ = *v7++;
            if (v4 - v5 >= 0x2000)
                Com_Error(ERR_DROP, "Info_ValueForKey: oversize key %d", v4 - v5);
        }
        *v4 = 0;
        if (!I_stricmp(key, s1))
            return v5;
        if (!*v7)
            break;
        s = v7 + 1;
    }
    return "";
}

void __cdecl Info_NextPair(const char **head, char *key, char *value)
{
    char *o; // [esp+0h] [ebp-8h]
    char *oa; // [esp+0h] [ebp-8h]
    const char *s; // [esp+4h] [ebp-4h]
    const char *sa; // [esp+4h] [ebp-4h]

    s = *head;
    if (**head == 92)
        ++s;
    *key = 0;
    *value = 0;
    o = key;
    while (*s != 92)
    {
        if (!*s)
        {
            *o = 0;
            *head = s;
            return;
        }
        *o++ = *s++;
    }
    *o = 0;
    sa = s + 1;
    oa = value;
    while (*sa != 92 && *sa)
        *oa++ = *sa++;
    *oa = 0;
    *head = sa;
}

void __cdecl Info_RemoveKey(char *s, const char *key)
{
    char *v2; // eax
    char v3; // al
    char *v4; // [esp+8h] [ebp-83Ch]
    char *v5; // [esp+Ch] [ebp-838h]
    char *o; // [esp+34h] [ebp-810h]
    char *oa; // [esp+34h] [ebp-810h]
    char *start; // [esp+38h] [ebp-80Ch]
    char pkey[1024]; // [esp+3Ch] [ebp-808h] BYREF
    char value[1024]; // [esp+43Ch] [ebp-408h] BYREF

    if (strlen(s) >= 0x400)
        Com_Error(ERR_DROP, "Info_RemoveKey: oversize infostring");

    v2 = (char*)strchr(key, 0x5Cu);

    if (!v2)
    {
        while (1)
        {
            start = s;
            if (*s == 92)
                ++s;
            o = pkey;
            while (*s != 92)
            {
                if (!*s)
                    return;
                *o++ = *s++;
            }
            *o = 0;
            ++s;
            oa = value;
            while (*s != 92 && *s)
                *oa++ = *s++;
            *oa = 0;
            if (!strcmp(key, pkey))
                break;
            if (!*s)
                return;
        }
        v5 = s;
        v4 = start;
        do
        {
            v3 = *v5;
            *v4++ = *v5++;
        } while (v3);
    }
}

void __cdecl Info_RemoveKey_Big(char *s, const char *key)
{
    char *v2; // eax
    char v3; // al
    char *v4; // [esp+8h] [ebp-403Ch]
    char *v5; // [esp+Ch] [ebp-4038h]
    char *v6; // [esp+34h] [ebp-4010h]
    char *v7; // [esp+34h] [ebp-4010h]
    char *v8; // [esp+38h] [ebp-400Ch]
    char v9[8192]; // [esp+3Ch] [ebp-4008h] BYREF
    char v10; // [esp+203Ch] [ebp-2008h] BYREF

    if (strlen(s) >= 0x2000)
        Com_Error(ERR_DROP, "Info_RemoveKey_Big: oversize infostring");

    v2 = (char*)strchr(key, 0x5Cu);

    if (!v2)
    {
        while (1)
        {
            v8 = s;
            if (*s == 92)
                ++s;
            v6 = v9;
            while (*s != 92)
            {
                if (!*s)
                    return;
                *v6++ = *s++;
            }
            *v6 = 0;
            ++s;
            v7 = &v10;
            while (*s != 92 && *s)
                *v7++ = *s++;
            *v7 = 0;
            if (!strcmp(key, v9))
                break;
            if (!*s)
                return;
        }
        v5 = s;
        v4 = v8;
        do
        {
            v3 = *v5;
            *v4++ = *v5++;
        } while (v3);
    }
}

bool __cdecl Info_Validate(const char *s)
{
    int v1; // eax
    int v3; // eax

    v1 = (int)strchr(s, 0x22u);

    if (v1)
        return 0;

    v3 = (int)strchr(s, 0x3Bu);

    return v3 == 0;
}

void __cdecl Info_SetValueForKey(char *s, const char *key, const char *value)
{
    int v3; // eax
    int v4; // eax
    int v5; // eax
    int j; // [esp+54h] [ebp-818h]
    char c; // [esp+5Bh] [ebp-811h]
    char cleanValue[1028]; // [esp+5Ch] [ebp-810h] BYREF
    int len; // [esp+460h] [ebp-40Ch]
    char newi[1024]; // [esp+464h] [ebp-408h] BYREF
    int i; // [esp+868h] [ebp-4h]

    if (!value)
        MyAssertHandler(".\\universal\\q_shared.cpp", 1254, 0, "%s", "value");
    if (strlen(s) < 0x400)
    {
        j = 0;
        for (i = 0; i < 1023; ++i)
        {
            c = value[i];
            if (!c)
                break;
            if (c != 92 && c != 59 && c != 34)
            {
                if (j >= 1024)
                    MyAssertHandler(".\\universal\\q_shared.cpp", 1270, 0, "%s", "j < MAX_INFO_STRING");
                cleanValue[j++] = c;
            }
        }
        if (j >= 1024)
            MyAssertHandler(".\\universal\\q_shared.cpp", 1275, 0, "%s", "j < MAX_INFO_STRING");
        cleanValue[j] = 0;
        v3 = (int)strchr(key, 0x5Cu);
        if (v3)
        {
            Com_Printf(16, (char *)"Can't use keys with a \\ key: %s value: %s", key, value);
        }
        else
        {
            v4 = (int)strchr(key, 0x3Bu);
            if (v4)
            {
                Com_Printf(16, (char *)"Can't use keys with a semicolon. key: %s value: %s", key, value);
            }
            else
            {
                v5 = (int)strchr(key, 0x22u);
                if (v5)
                {
                    Com_Printf(16, (char *)"Can't use keys with a \". key: %s value: %s", key, value);
                }
                else
                {
                    Info_RemoveKey(s, key);
                    if (cleanValue[0])
                    {
                        len = Com_sprintf(newi, 0x400u, "\\%s\\%s", key, cleanValue);
                        if (len > 0)
                        {
                            if (strlen(s) + &newi[strlen(newi) + 1] - &newi[1] <= 0x400)
                                memcpy(&s[strlen(s)], newi, &newi[strlen(newi) + 1] - newi);
                            else
                                Com_Printf(16, (char *)"Info string length exceeded. key: %s value: %s Info string: %s", key, value, s);
                        }
                        else
                        {
                            Com_Printf(16, (char *)"Info buffer length exceeded, not including key/value pair in response.");
                        }
                    }
                }
            }
        }
    }
    else
    {
        Com_Printf(16, (char *)"Info_SetValueForKey: oversize infostring");
    }
}

void __cdecl Info_SetValueForKey_Big(char *s, const char *key, const char *value)
{
    int v3; // eax
    int v4; // eax
    int v5; // eax
    int v6; // [esp+54h] [ebp-4018h]
    char v7; // [esp+5Bh] [ebp-4011h]
    char v8[8196]; // [esp+5Ch] [ebp-4010h] BYREF
    int v9; // [esp+2060h] [ebp-200Ch]
    char dest; // [esp+2064h] [ebp-2008h] BYREF
    _BYTE v11[3]; // [esp+2065h] [ebp-2007h] BYREF
    int i; // [esp+4068h] [ebp-4h]

    if (!value)
        MyAssertHandler(".\\universal\\q_shared.cpp", 1334, 0, "%s", "value");

    if (strlen(s) < 0x2000)
    {
        v6 = 0;
        for (i = 0; i < 0x1FFF; ++i)
        {
            v7 = value[i];
            if (!v7)
                break;
            if (v7 != 92 && v7 != 59 && v7 != 34)
            {
                if (v6 >= 0x2000)
                    MyAssertHandler(".\\universal\\q_shared.cpp", 1350, 0, "%s", "j < BIG_INFO_STRING");
                v8[v6++] = v7;
            }
        }
        if (v6 >= 0x2000)
            MyAssertHandler(".\\universal\\q_shared.cpp", 1355, 0, "%s", "j < BIG_INFO_STRING");
        v8[v6] = 0;
        v3 = (int)strchr(key, 0x5Cu);
        if (v3)
        {
            Com_Printf(16, "Can't use keys with a \\ key: %s value: %s", key, value);
        }
        else
        {
            v4 = (int)strchr(key, 0x3Bu);
            if (v4)
            {
                Com_Printf(16, "Can't use keys with a semicolon. key: %s value: %s", key, value);
            }
            else
            {
                v5 = (int)strchr(key, 0x22u);
                if (v5)
                {
                    Com_Printf(16, "Can't use keys with a \". key: %s value: %s", key, value);
                }
                else
                {
                    Info_RemoveKey_Big(s, key);
                    if (v8[0])
                    {
                        v9 = Com_sprintf(&dest, 0x2000u, "\\%s\\%s", key, v8);
                        if (v9 > 0)
                        {
                            if (strlen(s) + &v11[strlen(&dest)] - v11 <= 0x400)
                                memcpy(&s[strlen(s)], &dest, (char*)&v11[strlen(&dest)] - &dest);
                            else
                                Com_Printf(16, "Info string length exceeded. key: %s value: %s Info string: %s", key, value, s);
                        }
                        else
                        {
                            Com_Printf(16, "Info buffer length exceeded, not including key/value pair in response.");
                        }
                    }
                }
            }
        }
    }
    else
    {
        Com_Printf(16, "Info_SetValueForKey_Big: oversize infostring");
    }
}

bool __cdecl ParseConfigStringToStruct(
    unsigned __int8 *pStruct,
    const cspField_t *pFieldList,
    int iNumFields,
    char *pszBuffer,
    int iMaxFieldTypes,
    int(__cdecl *parseSpecialFieldType)(unsigned __int8 *, const char *, const int),
    void(__cdecl *parseStrcpy)(unsigned __int8 *, const char *))
{
    return ParseConfigStringToStructCustomSize(
        pStruct,
        pFieldList,
        iNumFields,
        pszBuffer,
        iMaxFieldTypes,
        parseSpecialFieldType,
        parseStrcpy);
}

bool __cdecl ParseConfigStringToStructCustomSize(
    unsigned __int8 *pStruct,
    const cspField_t *pFieldList,
    int iNumFields,
    char *pszBuffer,
    int iMaxFieldTypes,
    int(__cdecl *parseSpecialFieldType)(unsigned __int8 *, const char *, const int),
    void(__cdecl *parseStrcpy)(unsigned __int8 *, const char *))
{
    int v7; // eax
    int v8; // eax
    const FxEffectDef *v9; // eax
    Material *v10; // eax
    snd_alias_list_t *SoundAlias; // eax
    const char *v12; // eax
    const char *v14; // eax
    float v15; // [esp+0h] [ebp-2024h]
    float v16; // [esp+4h] [ebp-2020h]
    const char *src; // [esp+Ch] [ebp-2018h]
    char v18; // [esp+13h] [ebp-2011h]
    char dest[8192]; // [esp+14h] [ebp-2010h] BYREF
    const cspField_t *v20; // [esp+2018h] [ebp-Ch]
    int v21; // [esp+201Ch] [ebp-8h]
    XModel *v22; // [esp+2020h] [ebp-4h]

    v18 = 0;
    v21 = 0;
    v20 = pFieldList;
    while (v21 < iNumFields)
    {
        src = Info_ValueForKey(pszBuffer, (char *)v20->szName);
        if (*src)
        {
            if (v20->iFieldType >= 12)
            {
                if (iMaxFieldTypes <= 0 || v20->iFieldType >= iMaxFieldTypes)
                {
                    if (!alwaysfails)
                    {
                        v14 = va("Bad field type %i\n", v20->iFieldType);
                        MyAssertHandler(".\\universal\\q_shared.cpp", 1498, 0, v14);
                    }
                    Com_Error(ERR_DROP, "Bad field type %i", v20->iFieldType);
                }
                else
                {
                    if (!parseSpecialFieldType)
                        MyAssertHandler(".\\universal\\q_shared.cpp", 1492, 0, "%s", "parseSpecialFieldType != NULL");
                    if (!parseSpecialFieldType(pStruct, src, v20->iFieldType))
                        return 0;
                }
            }
            else
            {
                switch (v20->iFieldType)
                {
                case 0:
                    parseStrcpy(&pStruct[v20->iOffset], src);
                    break;
                case 1:
                    I_strncpyz((char *)&pStruct[v20->iOffset], src, 1024);
                    break;
                case 2:
                    I_strncpyz((char *)&pStruct[v20->iOffset], src, 64);
                    break;
                case 3:
                    I_strncpyz((char *)&pStruct[v20->iOffset], src, 256);
                    break;
                case 4:
                    v7 = atoi(src);
                    *(unsigned int *)&pStruct[v20->iOffset] = v7;
                    break;
                case 5:
                    v8 = atoi(src);
                    *(unsigned int *)&pStruct[v20->iOffset] = v8 != 0;
                    break;
                case 6:
                    v16 = atof(src);
                    *(float *)&pStruct[v20->iOffset] = v16;
                    break;
                case 7:
                    v15 = atof(src);
                    *(unsigned int *)&pStruct[v20->iOffset] = (int)(v15 * 1000.0);
                    break;
                case 8:
                    if (!com_dedicated->current.integer)
                    {
                        v9 = FX_Register(src);
                        *(unsigned int *)&pStruct[v20->iOffset] = (unsigned int)v9;
                    }
                    break;
                case 9:
                    I_strncpyz(dest, src, 0x2000);
                    v22 = R_RegisterModel(dest);
                    *(unsigned int *)&pStruct[v20->iOffset] = (unsigned int)v22;
                    if (!v22)
                        v18 = 1;
                    break;
                case 0xA:
                    if (!com_dedicated->current.integer)
                    {
                        v10 = Material_RegisterHandle(src, 0);
                        *(unsigned int *)&pStruct[v20->iOffset] = (unsigned int)v10;
                    }
                    break;
                case 0xB:
                    SoundAlias = Com_FindSoundAlias(src);
                    *(unsigned int *)&pStruct[v20->iOffset] = (unsigned int)SoundAlias;
                    break;
                default:
                    if (v20->iFieldType >= 0)
                    {
                        if (!alwaysfails)
                            MyAssertHandler(
                                ".\\universal\\q_shared.cpp",
                                1487,
                                0,
                                "ParseConfigStringToStruct is out of sync with the csParseFieldType_t enum list\n");
                    }
                    else if (!alwaysfails)
                    {
                        v12 = va("Negative field type %i given to ParseConfigStringToStruct\n", v20->iFieldType);
                        MyAssertHandler(".\\universal\\q_shared.cpp", 1485, 0, v12);
                    }
                    break;
                }
            }
        }
        ++v21;
        ++v20;
    }
    return v21 == iNumFields && !v18;
}

double __cdecl GetLeanFraction(float fFrac)
{
    float v3; // [esp+4h] [ebp-4h]

    v3 = fabs(fFrac);
    return (float)((2.0 - v3) * fFrac);
}

double __cdecl UnGetLeanFraction(float fFrac)
{
    float v3; // [esp+4h] [ebp-8h]
    float v4; // [esp+8h] [ebp-4h]

    if (fFrac < 0.0)
        MyAssertHandler(".\\universal\\q_shared.cpp", 1543, 0, "%s", "fFrac >= 0");
    if (fFrac > 1.0)
        MyAssertHandler(".\\universal\\q_shared.cpp", 1544, 0, "%s", "fFrac <= 1.f");
    v4 = 1.0 - fFrac;
    v3 = sqrt(v4);
    return (float)(1.0 - v3);
}

void __cdecl AddLeanToPosition(float *position, float fViewYaw, float fLeanFrac, float fViewRoll, float fLeanDist)
{
    float fLean; // [esp+Ch] [ebp-1Ch]
    float fLeana; // [esp+Ch] [ebp-1Ch]
    float vRight[3]; // [esp+10h] [ebp-18h] BYREF
    float vAng[3]; // [esp+1Ch] [ebp-Ch] BYREF

    if (fLeanFrac != 0.0)
    {
        fLean = GetLeanFraction(fLeanFrac);
        vAng[0] = 0.0;
        vAng[1] = fViewYaw;
        vAng[2] = fViewRoll * fLean;
        AngleVectors(vAng, 0, vRight, 0);
        fLeana = fLean * fLeanDist;
        Vec3Mad(position, fLeana, vRight, position);
    }
}

bool __cdecl Com_IsLegacyXModelName(const char *name)
{
    return !I_strnicmp(name, "xmodel", 6) && (name[6] == 47 || name[6] == 92);
}

unsigned int __cdecl LongNoSwap(unsigned int color)
{
    return color;
}

