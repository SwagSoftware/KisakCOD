#include "assertive.h"
#include <win32/win_local.h>
#include <cstdarg>

enum AssertOccurance : __int32
{
    FIRST_TIME = 0x0,
    RECURSIVE = 0x1,
}; // idb

char message[1024];
int isHandlingAssert;
char assertMessage[4096];
int lastAssertType;

void(__cdecl* AssertCallback)(const char*);

BOOL CopyMessageToClipboard()
{
    HWND DesktopWindow; // eax
    BOOL result; // eax
    char v2; // dl
    char* v3; // [esp+8h] [ebp-20h]
    char* v4; // [esp+Ch] [ebp-1Ch]
    char* mem; // [esp+20h] [ebp-8h]
    HGLOBAL memoryHandle; // [esp+24h] [ebp-4h]

    DesktopWindow = GetDesktopWindow();
    result = OpenClipboard(DesktopWindow);
    if (result)
    {
        EmptyClipboard();
        memoryHandle = GlobalAlloc(2u, strlen(assertMessage) + 1);
        if (memoryHandle)
        {
            mem = (char*)GlobalLock(memoryHandle);
            if (mem)
            {
                v4 = assertMessage;
                v3 = mem;
                do
                {
                    v2 = *v4;
                    *v3++ = *v4++;
                } while (v2);
                GlobalUnlock(memoryHandle);
                SetClipboardData(1u, memoryHandle);
            }
        }
        return CloseClipboard();
    }
    return result;
}

char __cdecl AssertNotify(int type, AssertOccurance occurance)
{
    HWND ActiveWindow; // eax
    const char* msg; // [esp+8h] [ebp-4h]

    if (AssertCallback)
        AssertCallback(assertMessage);
    if (type)
    {
        if (type == 1)
            msg = "SANITY CHECK FAILURE... (this text is on the clipboard)";
        else
            msg = "INTERNAL ERROR";
    }
    else
    {
        msg = "ASSERTION FAILURE... (this text is on the clipboard)";
    }
    ActiveWindow = GetActiveWindow();
    if (MessageBoxA(ActiveWindow, assertMessage, msg, 0x12011u) != 1)
        return 1;
    if (occurance != RECURSIVE)
        ExitProcess(0xFFFFFFFF);
    return 1;
}

HINSTANCE__* __cdecl GetModuleBase(char* name)
{
    const char* v2; // [esp+Ch] [ebp-11Ch]
    char moduleName[264]; // [esp+10h] [ebp-118h] BYREF
    HINSTANCE__* moduleHandle; // [esp+11Ch] [ebp-Ch]
    int nameLength; // [esp+120h] [ebp-8h]
    int nameIndex; // [esp+124h] [ebp-4h]

    v2 = &name[strlen(name) + 1];
    nameLength = v2 - (name + 1);
    for (nameIndex = nameLength - 1;
        nameIndex >= 0 && name[nameIndex] != 46 && name[nameIndex] != 47 && name[nameIndex] != 92;
        --nameIndex)
    {
        ;
    }
    if (nameIndex >= 0 && name[nameIndex] == 46)
        nameLength = nameIndex;
    memcpy((unsigned __int8*)moduleName, (unsigned __int8*)name, nameLength);
    strcpy(&moduleName[nameLength], ".exe");
    moduleHandle = GetModuleHandleA(moduleName);
    if (moduleHandle)
        return moduleHandle;
    strcpy(&moduleName[nameLength], ".dll");
    return GetModuleHandleA(moduleName);
}

char lineBuffer[0x100];
unsigned int lineBufferStartPos, lineBufferEndPos;

char __cdecl ReadLine(FILE* fp)
{
    bool flush; // [esp+3h] [ebp-5h]
    int i; // [esp+4h] [ebp-4h]

    while (1)
    {
        flush = 0;
        lineBufferEndPos -= lineBufferStartPos;
        memmove((unsigned __int8*)lineBuffer, (unsigned __int8*)&lineBuffer[lineBufferStartPos], lineBufferEndPos);
        lineBufferStartPos = 0;
    retry_18:
        lineBufferEndPos += fread(&lineBuffer[lineBufferEndPos], 1u, 256 - lineBufferEndPos - 1, fp);
        lineBuffer[lineBufferEndPos] = 0;
        if (!lineBufferEndPos)
            return 0;
        for (i = 0; ; ++i)
        {
            if (i >= (int)lineBufferEndPos)
            {
                flush = 1;
                lineBufferEndPos = 0;
                goto retry_18;
            }
            if (lineBuffer[i] == 10)
                break;
        }
        lineBuffer[i] = 0;
        if (lineBuffer[i + 1] == 13)
            lineBufferStartPos = i + 2;
        else
            lineBufferStartPos = i + 1;
        if (!flush)
            return 1;
    }
}

char __cdecl SkipLines(int lineCount, FILE* fp)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < lineCount; ++i)
    {
        if (!ReadLine(fp))
            return 0;
    }
    return 1;
}

void __cdecl ParseError(const char* msg)
{
    HWND ActiveWindow; // eax

    ActiveWindow = GetActiveWindow();
    MessageBoxA(ActiveWindow, msg, ".map parse error", 0x10u);
}

struct AddressInfo
{
    unsigned int address;
    char moduleName[64];
    char bestFunction[64];
    char bestFunctionFilename[64];
    unsigned int bestFunctionAddress;
    char bestLineFilename[64];
    unsigned int bestLineAddress;
    unsigned int bestLineNumber;
}; // idb

unsigned int g_assertAddressCount;
AddressInfo g_assertAddress[0x20];

char __cdecl ParseMapFile(FILE* fp, unsigned int baseAddress, char* mapName)
{
    int v4; // eax
    const char* v5; // eax
    char* v6; // eax
    const char* v7; // eax
    const char* v8; // eax
    char* v9; // eax
    const char* v10; // eax
    char* v11; // eax
    char* v12; // eax
    const char* v13; // eax
    char* pszNameStop; // [esp+14h] [ebp-878h]
    char* pszNameStart; // [esp+18h] [ebp-874h]
    unsigned int loadAddress; // [esp+20h] [ebp-86Ch] BYREF
    const char* filenameSubStr; // [esp+24h] [ebp-868h]
    int j; // [esp+28h] [ebp-864h]
    unsigned int address; // [esp+2Ch] [ebp-860h] BYREF
    const char* filename; // [esp+30h] [ebp-85Ch]
    AddressInfo* addressInfo; // [esp+34h] [ebp-858h]
    unsigned int relAddress; // [esp+38h] [ebp-854h]
    unsigned int lineOffset[4]; // [esp+3Ch] [ebp-850h] BYREF
    char filenameBuffer[1024]; // [esp+4Ch] [ebp-840h] BYREF
    unsigned int offset; // [esp+44Ch] [ebp-440h] BYREF
    unsigned int baseEndAddress; // [esp+450h] [ebp-43Ch]
    unsigned int group; // [esp+454h] [ebp-438h] BYREF
    const char* funcName; // [esp+458h] [ebp-434h]
    unsigned int lineGroup[4]; // [esp+45Ch] [ebp-430h] BYREF
    int i; // [esp+46Ch] [ebp-420h]
    unsigned int lineNumber[4]; // [esp+470h] [ebp-41Ch] BYREF
    char* atChar; // [esp+480h] [ebp-40Ch]
    char function[1024]; // [esp+484h] [ebp-408h] BYREF
    int readCount; // [esp+888h] [ebp-4h]

    do
    {
        if (!ReadLine(fp))
            return 0;
    } while (sscanf(lineBuffer, " Preferred load address is %x\r\n", &loadAddress) != 1);
    if (!SkipLines(2, fp))
        return 0;
    baseEndAddress = 0;
    while (1)
    {
        if (!ReadLine(fp))
            return 0;
        if (!lineBuffer[0])
            break;
        if (sscanf(lineBuffer, "%x:%x %xH %s %s", &group, &offset, &address, function, filenameBuffer) != 5)
        {
            ParseError("Unknown line format in the segments section");
            return 0;
        }
        if (group == 1 && baseEndAddress < address + offset + baseAddress + 4096)
            baseEndAddress = address + offset + baseAddress + 4096;
    }
    for (j = 0; j < g_assertAddressCount; ++j)
    {
        addressInfo = &g_assertAddress[j];
        if (addressInfo->address >= baseAddress && addressInfo->address < baseEndAddress)
            I_strncpyz(addressInfo->moduleName, mapName, 64);
    }
    do
    {
        if (!ReadLine(fp))
            return 0;
        v4 = !!strstr(lineBuffer, "Publics by Value");
    } while (!v4);
    if (!SkipLines(1, fp))
        return 0;
    while (1)
    {
        if (!ReadLine(fp))
            return 0;
        if (!lineBuffer[0])
            break;
        if (sscanf(lineBuffer, "%x:%x %s %x", &group, &offset, function, &address) != 4)
        {
            ParseError("Unknown line format in the public symbols section");
            return 0;
        }
        v5 = strrchr(lineBuffer, 0x20u);
        filenameSubStr = v5;
        if (!v5 || sscanf(filenameSubStr + 1, "%s", filenameBuffer) != 1)
        {
            ParseError("Couldn't parse file name in the public symbols section");
            return 0;
        }
        relAddress = address;
        for (j = 0; j < g_assertAddressCount; ++j)
        {
            addressInfo = &g_assertAddress[j];
            if (addressInfo->address >= baseAddress
                && addressInfo->address < baseEndAddress
                && relAddress <= addressInfo->address
                && (!addressInfo->bestFunction[0] || addressInfo->bestFunctionAddress < relAddress))
            {
                addressInfo->bestFunctionAddress = relAddress;
                funcName = function;
                if (function[0] == 95 || *funcName == 63)
                    ++funcName;
                I_strncpyz(addressInfo->bestFunction, (char*)funcName, 64);
                v6 = strchr(addressInfo->bestFunction, 0x40u);
                atChar = v6;
                if (v6)
                    *atChar = 0;
                v7 = strrchr(filenameBuffer, 0x5Cu);
                filename = v7;
                if (v7)
                    ++filename;
                else
                    filename = filenameBuffer;
                I_strncpyz(addressInfo->bestFunctionFilename, (char*)filename, 64);
            }
        }
    }
    if (!SkipLines(2, fp))
        return 0;
    if (!ReadLine(fp))
        return 0;
    if (strcmp(lineBuffer, " Static symbols\r"))
        goto LABEL_90;
    if (!SkipLines(1, fp))
        return 0;
    while (ReadLine(fp) && lineBuffer[0])
    {
        if (sscanf(lineBuffer, "%x:%x %s %x", &group, &offset, function, &address) != 4)
        {
            ParseError("Unknown line format in the static symbols section");
            return 0;
        }
        v8 = strrchr(lineBuffer, 0x20u);
        filenameSubStr = v8;
        if (!v8 || sscanf(filenameSubStr + 1, "%s", filenameBuffer) != 1)
        {
            ParseError("Couldn't parse file name in the static symbols section");
            return 0;
        }
        relAddress = address;
        for (j = 0; j < g_assertAddressCount; ++j)
        {
            addressInfo = &g_assertAddress[j];
            if (addressInfo->address >= baseAddress
                && addressInfo->address < baseEndAddress
                && relAddress <= addressInfo->address
                && (!addressInfo->bestFunction[0] || addressInfo->bestFunctionAddress < relAddress))
            {
                addressInfo->bestFunctionAddress = relAddress;
                funcName = function;
                if (function[0] == 95 || *funcName == 63)
                    ++funcName;
                I_strncpyz(addressInfo->bestFunction, (char*)funcName, 64);
                v9 = strchr(addressInfo->bestFunction, 0x40u);
                atChar = v9;
                if (v9)
                    *atChar = 0;
                v10 = strrchr(filenameBuffer, 0x5Cu);
                filename = v10;
                if (v10)
                    ++filename;
                else
                    filename = filenameBuffer;
                I_strncpyz(addressInfo->bestFunctionFilename, (char*)filename, 64);
            }
        }
    }
LABEL_90:
    while (ReadLine(fp))
    {
        if (strncmp(lineBuffer, "Line numbers for ", 0x11u))
        {
            ParseError("Expected line number section");
            return 0;
        }
        v11 = strchr(lineBuffer, 0x28u);
        pszNameStart = (char*)v11;
        if (!v11)
        {
            ParseError("Couldn't find '(' for the name of the source file in line number section");
            return 0;
        }
        v12 = strchr(v11, 0x29u);
        pszNameStop = v12;
        if (!v12)
        {
            ParseError("Couldn't find ')' for the name of the source file in line number section");
            return 0;
        }
        strncpy((unsigned __int8*)filenameBuffer, (unsigned __int8*)pszNameStart + 1, v12 - pszNameStart - 1);
        filenameBuffer[pszNameStop - pszNameStart - 1] = 0;
        filenameSubStr = filenameBuffer;
        if (!SkipLines(1, fp))
            return 0;
        while (ReadLine(fp) && lineBuffer[0])
        {
            readCount = sscanf(
                lineBuffer,
                "%i %x:%x %i %x:%x %i %x:%x %i %x:%x\r\n",
                lineNumber,
                lineGroup,
                lineOffset,
                &lineNumber[1],
                &lineGroup[1],
                &lineOffset[1],
                &lineNumber[2],
                &lineGroup[2],
                &lineOffset[2],
                &lineNumber[3],
                &lineGroup[3],
                &lineOffset[3]);
            if (readCount % 3 || readCount / 3 <= 0)
            {
                ParseError("unknown line format in the line number section");
                return 0;
            }
            for (i = 0; 3 * i < readCount; ++i)
            {
                relAddress = lineOffset[i] + baseAddress + 4096;
                for (j = 0; j < g_assertAddressCount; ++j)
                {
                    addressInfo = &g_assertAddress[j];
                    if (addressInfo->address >= baseAddress
                        && addressInfo->address < baseEndAddress
                        && relAddress <= addressInfo->address
                        && (!addressInfo->bestLineFilename[0] || addressInfo->bestLineAddress < relAddress))
                    {
                        addressInfo->bestLineAddress = relAddress;
                        addressInfo->bestLineNumber = lineNumber[i];
                        v13 = strrchr(filenameSubStr, 0x5Cu);
                        filename = v13;
                        if (v13)
                            ++filename;
                        else
                            filename = filenameSubStr;
                        I_strncpyz(addressInfo->bestLineFilename, (char*)filename, 64);
                    }
                }
            }
        }
    }
    return 1;
}

void __cdecl LoadMapFilesForDir(const char* dir)
{
    char* v1; // eax
    char* v2; // eax
    char* v3; // eax
    char v4; // [esp+13h] [ebp-1165h]
    char* p_file; // [esp+18h] [ebp-1160h]
    char* cFileName; // [esp+1Ch] [ebp-115Ch]
    _WIN32_FIND_DATAA FindFileData; // [esp+20h] [ebp-1158h] BYREF
    char file[MAX_PATH]; // [esp+160h] [ebp-1018h] BYREF
    unsigned int baseAddress; // [esp+964h] [ebp-814h]
    FILE* fp; // [esp+968h] [ebp-810h]
    HANDLE hFindFile; // [esp+96Ch] [ebp-80Ch]
    char string[2052]; // [esp+970h] [ebp-808h] BYREF

    if (*dir)
    {
        v1 = Sys_DefaultInstallPath();
        sprintf(string, "%s\\%s\\*.map", v1, dir);
    }
    else
    {
        v2 = Sys_DefaultInstallPath();
        sprintf(string, "%s\\*.map", v2);
    }
    hFindFile = FindFirstFileA(string, &FindFileData);
    if (hFindFile != (HANDLE)-1)
    {
        do
        {
            baseAddress = (unsigned int)GetModuleBase(FindFileData.cFileName);
            if (baseAddress)
            {
                v3 = Sys_DefaultInstallPath();
                sprintf(file, "%s\\%s", v3, FindFileData.cFileName);
                fp = fopen(file, "rb");
                if (fp)
                {
                    cFileName = FindFileData.cFileName;
                    p_file = file;
                    do
                    {
                        v4 = *cFileName;
                        *p_file++ = *cFileName++;
                    } while (v4);
                    FindFileData.cFileName[strlen(file) + 1] = 0;
                    ParseMapFile(fp, baseAddress, file);
                    fclose(fp);
                }
            }
        } while (FindNextFileA(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
}

int __cdecl LoadMapFiles(char* msg)
{
    int v1; // eax
    int j; // [esp+0h] [ebp-Ch]
    AddressInfo* addressInfo; // [esp+4h] [ebp-8h]
    char* curPos; // [esp+8h] [ebp-4h]
    char* curPosa; // [esp+8h] [ebp-4h]
    char* curPosb; // [esp+8h] [ebp-4h]

    LoadMapFilesForDir("");
    curPos = msg;
    for (j = 0; j < g_assertAddressCount; ++j)
    {
        addressInfo = &g_assertAddress[j];
        if (addressInfo->moduleName[0])
        {
            curPosa = &curPos[sprintf(curPos, "%s:    ", addressInfo->moduleName)];
            if (addressInfo->bestLineFilename[0])
            {
                curPosb = &curPosa[sprintf(
                    curPosa,
                    "%s        ...%s, line %i",
                    addressInfo->bestFunction,
                    addressInfo->bestLineFilename,
                    addressInfo->bestLineNumber)];
                v1 = sprintf(curPosb, "\n");
            }
            else
            {
                if (addressInfo->bestFunction[0])
                    curPosb = &curPosa[sprintf(
                        curPosa,
                        "%s        ...%s, address %x",
                        addressInfo->bestFunction,
                        addressInfo->bestFunctionFilename,
                        addressInfo->address)];
                else
                    curPosb = &curPosa[sprintf(curPosa, "%s, address %x", addressInfo->bestFunction, addressInfo->address)];
                v1 = sprintf(curPosb, "\n");
            }
            curPos = &curPosb[v1];
        }
    }
    return curPos - msg;
}

char g_module[MAX_PATH];

#include <intrin.h>

// KISAKX64
// this is broken right now
int __cdecl DoStackTrace(char* msg, int nIgnore)
{
    int* v2; // ecx
    int* reg_ebp; // [esp+4h] [ebp-10h]
    int i; // [esp+10h] [ebp-4h]

    memset((unsigned __int8*)g_assertAddress, 0, sizeof(g_assertAddress));
    g_assertAddressCount = 0;
    reg_ebp = 0;
    __asm {
        mov reg_ebp, ebp
    }
    for (i = 0; i < nIgnore + 32; ++i)
    {
        v2 = reg_ebp;
        if ((unsigned int)reg_ebp <= 0x400)
            break;
        reg_ebp = (int*)*reg_ebp;
        if (i >= nIgnore)
        {
            g_assertAddress[g_assertAddressCount++].address = v2[1] - 5;
            if (!reg_ebp)
                break;
        }
    }
    return LoadMapFiles(msg);
}

void __cdecl BuildAssertMessage(const char* expr, const char* filename, int line, int type, int skipLevels, char* message)
{
    const char* String; // eax
    int v7; // eax
    char* curPos; // [esp+0h] [ebp-14h]
    char unknown[12]; // [esp+4h] [ebp-10h] BYREF

    strcpy(unknown, "<unknown>");
    if (!filename)
        filename = unknown;
    if (!expr)
        expr = unknown;
    if (!GetModuleFileNameA(0, g_module, sizeof(g_module)))
        strcpy(g_module, "<unknown application>");
    String = Dvar_GetString("version");
    curPos = &message[sprintf(message, "Build: %s\n", String)];
    v7 = sprintf(
        curPos,
        "Expression:\n    %s\n\nModule:    %s\nFile:    %s\nLine:    %d\n\n",
        expr,
        g_module,
        filename,
        line);
    DoStackTrace(&curPos[v7], skipLevels + 1);
}

HWND g_hwndGame[4];
unsigned int g_hiddenCount;

int __stdcall HideWindowCallback(HWND hwnd, LPARAM lParam)
{
    LONG styleEx; // [esp+14h] [ebp-40Ch]
    char caption[1024]; // [esp+18h] [ebp-408h] BYREF
    int style; // [esp+41Ch] [ebp-4h]

    if (GetWindowTextA(hwnd, caption, 1024) && strcmp(caption, "Call of Duty 4 Multiplayer"))
        return 1;
    style = GetWindowLongA(hwnd, -16);
    styleEx = GetWindowLongA(hwnd, -20);
    if ((style & 0x10000000) != 0)
    {
        g_hwndGame[g_hiddenCount++] = hwnd;
        SetWindowLongA(hwnd, -16, style & 0xEFFFFFFF);
        SetWindowLongA(hwnd, -20, styleEx & 0xFFFFFFF7);
    }
    return 1;
}

void __cdecl FixWindowsDesktop()
{
    DWORD CurrentThreadId; // eax
    HDC__* hdc; // [esp+0h] [ebp-614h]
    _WORD ramp[770]; // [esp+4h] [ebp-610h] BYREF
    unsigned __int16 i; // [esp+60Ch] [ebp-8h]
    HWND__* hwndDesktop; // [esp+610h] [ebp-4h]

    ChangeDisplaySettingsA(0, 0);
    CurrentThreadId = GetCurrentThreadId();
    EnumThreadWindows(CurrentThreadId, HideWindowCallback, 0);
    hwndDesktop = GetDesktopWindow();
    hdc = GetDC(hwndDesktop);
    for (i = 0; i < 0x100u; ++i)
    {
        ramp[i] = 257 * i;
        ramp[i + 256] = 257 * i;
        ramp[i + 512] = 257 * i;
    }
    SetDeviceGammaRamp(hdc, ramp);
    ReleaseDC(hwndDesktop, hdc);
}

bool __cdecl QuitOnError();
void MyAssertHandler(const char *filename, int line, int type, const char *fmt, ...)
{
    char shouldBreak; // [esp+3h] [ebp-5h]
    va_list va; // [esp+20h] [ebp+18h] BYREF
    
    va_start(va, fmt);
    Sys_EnterCriticalSection(CRITSECT_ASSERT);
    _vsnprintf(message, 0x400u, fmt, va);
    message[1023] = 0;
    if (isHandlingAssert)
    {
        CopyMessageToClipboard();
        AssertNotify(lastAssertType, RECURSIVE);
        BuildAssertMessage(message, filename, line, type, 1, assertMessage);
        if (isHandlingAssert == 1)
        {
            isHandlingAssert = 2;
            Com_Printf(16, "ASSERTBEGIN - ( Recursive assert )---------------------------------------------\n");
            Com_Printf(16, assertMessage);
            Com_Printf(16, "ASSERTEND - ( Recursive assert ) ----------------------------------------------\n\n");
        }
        exit(-1);
    }
    lastAssertType = type;
    isHandlingAssert = 1;
    FixWindowsDesktop();
    BuildAssertMessage(message, filename, line, type, 1, assertMessage);
    Com_Printf(16, "ASSERTBEGIN -------------------------------------------------------------------\n");
    Com_Printf(16, "%s", assertMessage);
    Com_Printf(16, "ASSERTEND ---------------------------------------------------------------------\n");
    if (QuitOnError())
        ExitProcess(0xFFFFFFFF);
    CopyMessageToClipboard();
    shouldBreak = AssertNotify(type, FIRST_TIME);
    isHandlingAssert = 0;
    Sys_LeaveCriticalSection(CRITSECT_ASSERT);
    if (shouldBreak)
        DebugBreak();
}