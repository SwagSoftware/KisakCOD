#pragma once


unsigned int __cdecl GetHashCode(const char *str, unsigned int len);
unsigned int __cdecl lowercaseHash(const char *str);
void __cdecl CCS_InitConstantConfigStrings();
int __cdecl CCS_GetConstConfigStringIndex(const char *configString);
int __cdecl CCS_GetConfigStringNumForConstIndex(unsigned int index);
unsigned int __cdecl CCS_IsConfigStringIndexConstant(int index);
