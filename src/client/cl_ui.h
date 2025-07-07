#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

void __cdecl Key_KeynumToStringBuf(int keynum, char *buf, int buflen);
int __cdecl CL_ShutdownUI();
void __cdecl CL_InitUI();
