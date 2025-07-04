#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

void __fastcall Key_KeynumToStringBuf(int keynum, char *buf, int buflen);
int __fastcall CL_ShutdownUI();
void __fastcall CL_InitUI();
