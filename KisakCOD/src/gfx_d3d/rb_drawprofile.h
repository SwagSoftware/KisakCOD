#pragma once
#include "r_gfx.h"
#include "r_font.h"
#include <universal/profile.h>


struct __declspec(align(8)) DrawProfileGlobals // sizeof=0x81A8
{                                       // ...
    Font_s *font;                       // ...
    float fontWidth;                    // ...
    float fontHeight;                   // ...
    GfxColor textColor;                 // ...
    GfxColor labelColor;                // ...
    ProfileProbe log[5];                // ...
    int sortedProbeIndices[432];        // ...
    int lastSortTime;                   // ...
    ProfileReadableGlobal global[432];  // ...
    int devguiSelection;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct SourceBufferInfo // sizeof=0x2C
{
    const char *codePos;
    char *buf;
    const char *sourceBuf;
    int len;
    int sortedIndex;
    bool archive;
    // padding byte
    // padding byte
    // padding byte
    int time;
    int avgTime;
    int maxTime;
    float totalTime;
    float totalBuiltIn;
};


void __cdecl TRACK_rb_drawprofile();
void __cdecl RB_AddProfileThread(int threadContext);
char __cdecl RB_IsUsingAnyProfile();
void __cdecl RB_DrawProfile();
void __cdecl RB_DrawProfileHistory(const ProfileSettings *profSettings);
void __cdecl RB_DrawProfileHistoryGraph(const ProfileReadable *read, int parity, int probeIndex, float x, float y);
void __cdecl RB_DrawProfileRect(float x, float y, float width, float height, GfxColor color);
void __cdecl RB_DrawProfileBar(float x, float y, float pixelsPerClock, unsigned int clockCount, GfxColor color);
void __cdecl RB_DrawProfileHistoryLabel(int profEnum, float x, float y);
int __cdecl RB_GetProfileHistoryProbeIndex(int historyIndex, const ProfileSettings *profSettings);
int RB_DrawProfileOverlay();
void __cdecl RB_DrawAllProfileBackgrounds(const ProfileSettings *profSettings, int rowCount);
void __cdecl RB_DrawProfileBackground(float y);
double __cdecl RB_DrawProfileLabels();
void __cdecl RB_DrawProfileString(int column, float y, const char *s, GfxColor textColor);
double __cdecl RB_DrawProfileRow(int probeIndex, int indentation, float y);
int __cdecl CompareSelfTimes(unsigned int *e0, unsigned int *e1);
int __cdecl CompareTotalTimes(unsigned int *e0, unsigned int *e1);
int __cdecl CompareAvgTotalTimes(int *e0, int *e1);
int __cdecl CompareMaxTimes(unsigned int *e0, unsigned int *e1);
int __cdecl CompareAvgSelfTimes(int *e0, int *e1);
int __cdecl CompareMaxSelfTimes(unsigned int *e0, unsigned int *e1);
void __cdecl RB_DrawProfileScript();
void __cdecl RB_ProfileInit();
