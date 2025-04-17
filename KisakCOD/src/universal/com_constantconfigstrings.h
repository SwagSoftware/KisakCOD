#pragma once

struct constantConfigString // sizeof=0x10
{
    int configStringNum;
    char *configString;
    int configStringHash;
    int lowercaseConfigStringHash;
};

unsigned int __cdecl GetHashCode(const char *str, unsigned int len);
unsigned int __cdecl lowercaseHash(const char *str);
void __cdecl CCS_InitConstantConfigStrings();
int __cdecl CCS_GetConstConfigStringIndex(const char *configString);
int __cdecl CCS_GetConfigStringNumForConstIndex(unsigned int index);
unsigned int __cdecl CCS_IsConfigStringIndexConstant(int index);


extern constantConfigString constantConfigStrings[833];
