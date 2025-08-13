#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "savedevice.h"

// void __cdecl SaveDevice_Init(void)    8227fb88 f   savedevice_xenon.obj
// void __cdecl SV_DisplaySaveErrorUI(void) 8227fba8 f   savedevice_xenon.obj
// BOOL __cdecl BuildCleanSavePath(char *, unsigned int, char const *, enum SaveType) 8227fbf8 f   savedevice_xenon.obj
// BOOL __cdecl SaveDevice_IsAccessingDevice(void) 8227fdf0 f   savedevice_xenon.obj
// void __cdecl WriteSaveToDeviceCleanup(void) 8227fe00 f   savedevice_xenon.obj
// BOOL __cdecl SaveDevice_IsSaveSuccessful(void) 822800c8 f   savedevice_xenon.obj
// int __cdecl ReadFromDevice(void *, int, void *) 822800d8 f   savedevice_xenon.obj
// int __cdecl OpenDevice(char const *, void **) 82280140 f   savedevice_xenon.obj
// void __cdecl CloseDevice(void *)     822801e0 f   savedevice_xenon.obj
// BOOL __cdecl SaveExists(char const *)     82280280 f   savedevice_xenon.obj
// int __cdecl WriteSaveToDevice(unsigned char *, struct SaveHeader const *, BOOL) 82280448 f   savedevice_xenon.obj
// void __cdecl SV_ForceSelectSaveDevice_f(void) 823cfd38 f   sv_ccmds.obj
// void __cdecl SV_SelectSaveDevice_f(void) 823cfd90 f   sv_ccmds.obj
// char const *const CONSOLE_DEFAULT_SAVE_NAME 826969a4     savedevice_xenon.obj
// int marker_savedevice_pc 82e2081c     savedevice_pc.obj
// int marker_savedevice_xenon 82e208b4     savedevice_xenon.obj
// BOOL __cdecl SaveDevice_CheckForError(struct MemcardError const *) 8227fba0 f   savedevice_xenon.obj
// int __cdecl WriteSaveToDeviceInternal(struct SaveHeader const *) 8227fe48 f   savedevice_xenon.obj
// void __cdecl BeginScreenUpdateIfSupported(void) 82280218 f   savedevice_xenon.obj
// void __cdecl EndScreenUpdateIfSupported(void) 82280220 f   savedevice_xenon.obj
// void *__cdecl SaveExists_OpenContextFile(char const *) 82280228 f   savedevice_xenon.obj

