#include "q_shared.h"

#include <string.h>
#include <qcommon/qcommon.h>

char __cdecl Com_Filter(const char *filter, char *name, int casesensitive)
{
    const char *v3; // eax
    int v5; // esi
    int v6; // esi
    int v7; // esi
    int v8; // esi
    char buf[1024]; // [esp+24h] [ebp-410h] BYREF
    const char *ptr; // [esp+428h] [ebp-Ch]
    int i; // [esp+42Ch] [ebp-8h]
    int found; // [esp+430h] [ebp-4h]
    const char *filtera; // [esp+43Ch] [ebp+8h]

    while (*filter)
    {
        if (*filter == 42)
        {
            ++filter;
            i = 0;
            while (*filter && *filter != 42 && *filter != 63)
                buf[i++] = *filter++;
            buf[i] = 0;
            if (&buf[strlen(buf) + 1] != &buf[1])
            {
                if (casesensitive)
                {
                    ptr = I_stristr(name, buf);
                }
                else
                {
                    strstr((char*)name, (char*)buf);
                    ptr = v3;
                }
                if (!ptr)
                    return 0;
                name = (char *)&ptr[strlen(buf)];
            }
        }
        else if (*filter == 63)
        {
            ++filter;
            ++name;
        }
        else if (*filter == 91 && filter[1] == 91)
        {
            ++filter;
        }
        else if (*filter == 91)
        {
            filtera = filter + 1;
            found = 0;
            while (*filtera && !found && (*filtera != 93 || filtera[1] == 93))
            {
                if (filtera[1] == 45 && filtera[2] && (filtera[2] != 93 || filtera[3] == 93))
                {
                    if (casesensitive)
                    {
                        if (*name >= *filtera && *name <= filtera[2])
                            found = 1;
                    }
                    else
                    {
                        v5 = toupper(*name);
                        if (v5 >= toupper(*filtera))
                        {
                            v6 = toupper(*name);
                            if (v6 <= toupper(filtera[2]))
                                found = 1;
                        }
                    }
                    filtera += 3;
                }
                else
                {
                    if (casesensitive)
                    {
                        if (*filtera == *name)
                            found = 1;
                    }
                    else
                    {
                        v7 = toupper(*filtera);
                        if (v7 == toupper(*name))
                            found = 1;
                    }
                    ++filtera;
                }
            }
            if (!found)
                return 0;
            while (*filtera && (*filtera != 93 || filtera[1] == 93))
                ++filtera;
            filter = filtera + 1;
            ++name;
        }
        else
        {
            if (casesensitive)
            {
                if (*filter != *name)
                    return 0;
            }
            else
            {
                v8 = toupper(*filter);
                if (v8 != toupper(*name))
                    return 0;
            }
            ++filter;
            ++name;
        }
    }
    return 1;
}

char __cdecl Com_FilterPath(const char *filter, const char *name, int casesensitive)
{
    char new_filter[64]; // [esp+0h] [ebp-88h] BYREF
    char new_name[64]; // [esp+40h] [ebp-48h] BYREF
    int i; // [esp+84h] [ebp-4h]

    for (i = 0; i < 63 && filter[i]; ++i)
    {
        if (filter[i] == 92 || filter[i] == 58)
            new_filter[i] = 47;
        else
            new_filter[i] = filter[i];
    }
    new_filter[i] = 0;
    for (i = 0; i < 63 && name[i]; ++i)
    {
        if (name[i] == 92 || name[i] == 58)
            new_name[i] = 47;
        else
            new_name[i] = name[i];
    }
    new_name[i] = 0;
    return Com_Filter(new_filter, new_name, casesensitive);
}

int __cdecl Com_HashKey(const char *string, int maxlen)
{
    int hash; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    hash = 0;
    for (i = 0; i < maxlen && string[i]; ++i)
        hash += (i + 119) * string[i];
    return (hash >> 20) ^ hash ^ (hash >> 10);
}

int __cdecl Com_RealTime(qtime_s *qtime)
{
    __int64 t; // [esp+0h] [ebp-10h] BYREF
    tm *tms; // [esp+Ch] [ebp-4h]

    t = _time64(0);
    if (!qtime)
        return t;
    tms = _localtime64(&t);
    if (tms)
    {
        qtime->tm_sec = tms->tm_sec;
        qtime->tm_min = tms->tm_min;
        qtime->tm_hour = tms->tm_hour;
        qtime->tm_mday = tms->tm_mday;
        qtime->tm_mon = tms->tm_mon;
        qtime->tm_year = tms->tm_year;
        qtime->tm_wday = tms->tm_wday;
        qtime->tm_yday = tms->tm_yday;
        qtime->tm_isdst = tms->tm_isdst;
    }
    return t;
}

static void __cdecl Com_Prefetch(char *s, signed int bytes)
{
    signed int v3; // ecx
    unsigned int i; // ecx

    v3 = bytes;
    if (bytes > 4096)
        v3 = 4096;
    for (i = (unsigned int)(v3 + 31) >> 5; i; --i)
        s += 32;
}

void __cdecl Com_Memcpy(char *dest, char *src, int count)
{
    int v3; // ecx
    char *v4; // edx
    char *v5; // ebx
    signed int v6; // edi
    int v7; // esi
    int v8; // esi
    int v9; // esi
    int v10; // esi
    bool v11; // cc
    int v12; // eax

    if (!src && count)
        MyAssertHandler(".\\universal\\com_shared.cpp", 307, 0, "%s", "src || !count");
    if (!dest && count)
        MyAssertHandler(".\\universal\\com_shared.cpp", 308, 0, "%s", "dest || !count");
    Com_Prefetch(src, count);
    v3 = count;
    if (count)
    {
        v4 = dest;
        v5 = src;
        if (count < 32)
            goto padding_0;
        v6 = (count & 0xFFFFFFE0) - 32;
        do
        {
            v7 = *(unsigned int *)&src[v6 + 4];
            *(unsigned int *)&dest[v6] = *(unsigned int *)&src[v6];
            *(unsigned int *)&dest[v6 + 4] = v7;
            v8 = *(unsigned int *)&src[v6 + 12];
            *(unsigned int *)&dest[v6 + 8] = *(unsigned int *)&src[v6 + 8];
            *(unsigned int *)&dest[v6 + 12] = v8;
            v9 = *(unsigned int *)&src[v6 + 20];
            *(unsigned int *)&dest[v6 + 16] = *(unsigned int *)&src[v6 + 16];
            *(unsigned int *)&dest[v6 + 20] = v9;
            v10 = *(unsigned int *)&src[v6 + 28];
            *(unsigned int *)&dest[v6 + 24] = *(unsigned int *)&src[v6 + 24];
            *(unsigned int *)&dest[v6 + 28] = v10;
            v11 = v6 < 32;
            v6 -= 32;
        } while (!v11);
        v5 = &src[count & 0xFFFFFFE0];
        v4 = &dest[count & 0xFFFFFFE0];
        v3 = count & 0x1F;
        if ((count & 0x1F) != 0)
        {
        padding_0:
            if (v3 >= 16)
            {
                *(unsigned int *)v4 = *(unsigned int *)v5;
                *((unsigned int *)v4 + 1) = *((unsigned int *)v5 + 1);
                *((unsigned int *)v4 + 2) = *((unsigned int *)v5 + 2);
                *((unsigned int *)v4 + 3) = *((unsigned int *)v5 + 3);
                v3 -= 16;
                v5 += 16;
                v4 += 16;
            }
            if (v3 >= 8)
            {
                *(unsigned int *)v4 = *(unsigned int *)v5;
                v3 -= 8;
                *((unsigned int *)v4 + 1) = *((unsigned int *)v5 + 1);
                v5 += 8;
                v4 += 8;
            }
            if (v3 >= 4)
            {
                v12 = *(unsigned int *)v5;
                v5 += 4;
                v3 -= 4;
                *(unsigned int *)v4 = v12;
                v4 += 4;
            }
            if (v3 < 2)
            {
                if (v3 >= 1)
                    *v4 = *v5;
            }
            else
            {
                *(_WORD *)v4 = *(_WORD *)v5;
                if (v3 >= 3)
                    v4[2] = v5[2];
            }
        }
    }
}

void __cdecl Com_Memset(unsigned int *dest, int val, int count)
{
    unsigned int *v3; // edx
    int v4; // eax
    int v5; // eax
    int v6; // ecx
    char *v7; // ebx

    if (count >= 8)
    {
        _copyDWord(dest, val | (val << 8) | ((val | (val << 8)) << 16), count / 4);
        if ((count & 3) != 0)
        {
            v7 = (char *)dest + (count & 0xFFFFFFFC);
            if ((count & 3u) < 2)
            {
                if ((count & 3) != 0)
                    *v7 = val;
            }
            else
            {
                *(_WORD *)v7 = val | ((_WORD)val << 8);
                if ((count & 3) != 2)
                    v7[2] = val;
            }
        }
    }
    else
    {
        v3 = dest;
        v4 = val;
        BYTE1(v4) = val;
        v5 = (unsigned __int16)v4 + (v4 << 16);
        v6 = count;
        if (count >= 4)
        {
            *dest = v5;
            v3 = dest + 1;
            v6 = count - 4;
        }
        if (v6 >= 2)
        {
            *(_WORD *)v3 = v5;
            v3 = (unsigned int *)((char *)v3 + 2);
            v6 -= 2;
        }
        if (v6)
            *(_BYTE *)v3 = v5;
    }
}

