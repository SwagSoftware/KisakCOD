#include "com_constantconfigstrings.h"
#include "assertive.h"
#include <string.h>
#include <ctype.h>
#include "q_shared.h"


struct constantConfigString // sizeof=0x10
{
    int configStringNum;
    char *configString;
    int configStringHash;
    int lowercaseConfigStringHash;
};

char tempString[2048];
// struct constantConfigString *constantConfigStrings 827b8848     com_constantconfigstrings.obj
constantConfigString constantConfigStrings[833];
// unsigned int *reservedConfigStrings 850cc100     com_constantconfigstrings.obj
unsigned int reservedConfigStrings[77];
// int constantConfigStringCount 850cc228     com_constantconfigstrings.obj
int constantConfigStringCount;

unsigned int __cdecl GetHashCode(const char *str, unsigned int len)
{
    unsigned int hash; // [esp+4h] [ebp-8h]

    if (len >= 0x100)
        return (len >> 2) % 0x4E1F + 1;
    hash = 0;
    while (len)
    {
        hash = *str++ + 31 * hash;
        --len;
    }
    return hash % 0x4E1F + 1;
}

unsigned int __cdecl lowercaseHash(const char *str)
{
    char *d; // [esp+10h] [ebp-8h]
    const char *s; // [esp+14h] [ebp-4h]

    if (strlen(str) >= 0x800)
        MyAssertHandler(".\\universal\\com_constantconfigstrings.cpp", 889, 0, "%s", "strlen( str ) < sizeof( tempString )");
    s = str;
    d = tempString;
    while (*s)
        *d++ = tolower(*s++);
    return GetHashCode(tempString, d - tempString);
}

void __cdecl CCS_InitConstantConfigStrings()
{
    int index; // [esp+14h] [ebp-4h]

    for (index = 0; constantConfigStrings[index].configStringNum; ++index)
    {
        if (index && constantConfigStrings[index - 1].configStringNum >= constantConfigStrings[index].configStringNum)
            MyAssertHandler(
                ".\\universal\\com_constantconfigstrings.cpp",
                910,
                0,
                "%s",
                "index == 0 || constantConfigStrings[index-1].configStringNum < constantConfigStrings[index].configStringNum");
        if (constantConfigStrings[index].configStringNum < 821)
            dword_94A788[4 * index] = GetHashCode_0((&off_94A784)[4 * index], strlen((&off_94A784)[4 * index]));
        else
            dword_94A788[4 * index] = lowercaseHash((&off_94A784)[4 * index]);
        reservedConfigStrings[constantConfigStrings[index].configStringNum / 32] |= 1 << (constantConfigStrings[index].configStringNum
            % 32);
    }
    constantConfigStringCount = index;
}

int __cdecl CCS_GetConstConfigStringIndex(const char *configString)
{
    int hash; // [esp+24h] [ebp-Ch]
    int index; // [esp+28h] [ebp-8h]
    unsigned int lcaseHash; // [esp+2Ch] [ebp-4h]

    index = 0;
    hash = GetHashCode(configString, strlen(configString));
    lcaseHash = lowercaseHash(configString);
    while (constantConfigStrings[index].configStringNum)
    {
        if (constantConfigStrings[index].configStringNum < 821)
        {
            if (dword_94A788[4 * index] == hash && !strcmp(configString, (&off_94A784)[4 * index]))
                return index;
        }
        else if (dword_94A788[4 * index] == lcaseHash && !I_stricmp(configString, (&off_94A784)[4 * index]))
        {
            return index;
        }
        ++index;
    }
    return -1;
}

int __cdecl CCS_GetConfigStringNumForConstIndex(unsigned int index)
{
    if (index >= constantConfigStringCount)
        MyAssertHandler(
            ".\\universal\\com_constantconfigstrings.cpp",
            954,
            0,
            "index doesn't index constantConfigStringCount\n\t%i not in [0, %i)",
            index,
            constantConfigStringCount);
    return constantConfigStrings[index].configStringNum;
}

unsigned int __cdecl CCS_IsConfigStringIndexConstant(int index)
{
    return reservedConfigStrings[index / 32] & (1 << (index % 32));
}

