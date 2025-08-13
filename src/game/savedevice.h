#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif


static const char *CONSOLE_DEFAULT_SAVE_NAME = "savegame.svg";


void __cdecl SaveDevice_Init(void);
void __cdecl SV_DisplaySaveErrorUI(void);
bool __cdecl BuildCleanSavePath(char *, unsigned int, char const *, enum SaveType);
bool __cdecl SaveDevice_IsAccessingDevice(void);
void __cdecl WriteSaveToDeviceCleanup(void);
bool __cdecl SaveDevice_IsSaveSuccessful(void);
int __cdecl ReadFromDevice(void *, int, void *);
int __cdecl OpenDevice(char const *, void **);
void __cdecl CloseDevice(void *);
bool __cdecl SaveExists(char const *);
int __cdecl WriteSaveToDevice(unsigned char *, struct SaveHeader const *, bool);
void __cdecl SV_ForceSelectSaveDevice_f(void);
void __cdecl SV_SelectSaveDevice_f(void);
bool __cdecl SaveDevice_CheckForError(struct MemcardError const *);
int __cdecl WriteSaveToDeviceInternal(struct SaveHeader const *);
void __cdecl BeginScreenUpdateIfSupported(void);
void __cdecl EndScreenUpdateIfSupported(void);
void *__cdecl SaveExists_OpenContextFile(char const *);