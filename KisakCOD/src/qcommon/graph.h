#pragma once

enum DevEventType : __int32
{                                       // ...
    EVENT_ACTIVATE = 0x0,
    EVENT_DEACTIVATE = 0x1,
    EVENT_ACCEPT = 0x2,
    EVENT_UPDATE = 0x3,
    EVENT_DRAW = 0x4,
    EVENT_SAVE = 0x5,
};

struct DevGraph // sizeof=0x20
{                                       // ...
    float (*knots)[2];                  // ...
    int *knotCount;                     // ...
    int knotCountMax;                   // ...
    int selectedKnot;
    void(__cdecl *eventCallback)(const DevGraph *, DevEventType, int); // ...
    void(__cdecl *textCallback)(const DevGraph *, const float, const float, char *, const int);
    void *data;                         // ...
    bool disableEditingEndPoints;       // ...
    // padding byte
    // padding byte
    // padding byte
};
struct GraphFloat // sizeof=0x168
{                                       // ...
    char name[64];
    float knots[32][2];
    int knotCount;
    float scale;
    DevGraph devguiGraph;
};

void __cdecl GraphFloat_Load(GraphFloat *graph, char *fileName, float scale);
void __cdecl GraphFloat_ParseBuffer(GraphFloat *graph, const char *buffer, char *fileName);
void __cdecl GraphFloat_CreateDevGui(GraphFloat *graph, const char *devguiPath);
void __cdecl GraphFloat_DevGuiCB_Event(const DevGraph *graph, DevEventType event);
void __cdecl GraphFloat_SaveToFile(const GraphFloat *graph);
void __cdecl GraphFloat_DevGuiCB_Text(const DevGraph *devGuiGraph, float inputX, float inputY, char *text);