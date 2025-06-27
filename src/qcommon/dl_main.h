#pragma once

enum dlStatus_t : __int32
{                                       // ...
    DL_CONTINUE = 0x0,
    DL_DONE = 0x1,
    DL_FAILED = 0x2,
};

int __cdecl DL_VPrintf(const char *fmt, char *argptr);
void __cdecl DL_CancelDownload();
void __cdecl DL_InitDownload();
int __cdecl DL_BeginDownload(char *localName, char *remoteName);
int __cdecl DL_DownloadLoop();
bool __cdecl DL_InProgress();
bool __cdecl DL_DLIsMotd();
