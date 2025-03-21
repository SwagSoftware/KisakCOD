#include "graph.h"
#include <universal/q_shared.h>
#include <universal/assertive.h>
#include "qcommon.h"

void __cdecl GraphFloat_Load(GraphFloat *graph, char *fileName, float scale)
{
    char *InfoString; // eax
    char loadBuffer[8196]; // [esp+4h] [ebp-2008h] BYREF

    if (!graph)
        MyAssertHandler(".\\qcommon\\graph.cpp", 79, 0, "%s", "graph");
    if (!fileName)
        MyAssertHandler(".\\qcommon\\graph.cpp", 80, 0, "%s", "fileName");
    Com_Memset((unsigned int *)graph, 0, 360);
    InfoString = Com_LoadInfoString(fileName, "graph", "GRAPH_FLOAT_FILE", loadBuffer);
    GraphFloat_ParseBuffer(graph, InfoString, fileName);
    graph->scale = scale;
}

void __cdecl GraphFloat_ParseBuffer(GraphFloat *graph, const char *buffer, char *fileName)
{
    parseInfo_t *tokenb; // [esp+0h] [ebp-10h]
    parseInfo_t *token; // [esp+0h] [ebp-10h]
    parseInfo_t *tokena; // [esp+0h] [ebp-10h]
    int knotIndex; // [esp+4h] [ebp-Ch]
    float knots; // [esp+8h] [ebp-8h]
    float knots_4; // [esp+Ch] [ebp-4h]

    if (!graph)
        MyAssertHandler(".\\qcommon\\graph.cpp", 29, 0, "%s", "graph");
    if (!buffer)
        MyAssertHandler(".\\qcommon\\graph.cpp", 30, 0, "%s", "buffer");
    if (!fileName)
        MyAssertHandler(".\\qcommon\\graph.cpp", 31, 0, "%s", "fileName");
    I_strncpyz(graph->name, fileName, 64);
    Com_BeginParseSession(fileName);
    tokenb = Com_Parse(&buffer);
    graph->knotCount = atoi(tokenb->token);
    for (knotIndex = 0; ; ++knotIndex)
    {
        token = Com_Parse(&buffer);
        if (!token->token[0])
            break;
        if (token->token[0] == 125)
            break;
        knots = atof(token->token);
        tokena = Com_Parse(&buffer);
        if (!tokena->token[0] || tokena->token[0] == 125)
            break;
        knots_4 = atof(tokena->token);
        if (knotIndex >= 32)
            Com_Error(ERR_DROP, &byte_898FF8, fileName, 32);
        graph->knots[knotIndex][0] = knots;
        graph->knots[knotIndex][1] = knots_4;
    }
    Com_EndParseSession();
    if (knotIndex != graph->knotCount)
        Com_Error(ERR_DROP, &byte_898FC0, fileName);
}

void __cdecl GraphFloat_CreateDevGui(GraphFloat *graph, const char *devguiPath)
{
    if (!graph)
        MyAssertHandler(".\\qcommon\\graph.cpp", 202, 0, "%s", "graph");
    if (!devguiPath)
        MyAssertHandler(".\\qcommon\\graph.cpp", 203, 0, "%s", "devguiPath");
    graph->devguiGraph.knotCountMax = 32;
    graph->devguiGraph.knots = graph->knots;
    graph->devguiGraph.knotCount = &graph->knotCount;
    graph->devguiGraph.eventCallback = (void(__cdecl *)(const DevGraph *, DevEventType, int))GraphFloat_DevGuiCB_Event;
    graph->devguiGraph.textCallback = (void(__cdecl *)(const DevGraph *, const float, const float, char *, const int))GraphFloat_DevGuiCB_Text;
    graph->devguiGraph.data = graph;
    DevGui_AddGraph(devguiPath, &graph->devguiGraph);
}

void __cdecl GraphFloat_DevGuiCB_Event(const DevGraph *graph, DevEventType event)
{
    if (!graph)
        MyAssertHandler(".\\qcommon\\graph.cpp", 166, 0, "%s", "graph");
    if (!graph->data)
        MyAssertHandler(".\\qcommon\\graph.cpp", 167, 0, "%s", "graph->data");
    if (event == EVENT_SAVE)
        GraphFloat_SaveToFile((const GraphFloat *)graph->data);
}

void __cdecl GraphFloat_SaveToFile(const GraphFloat *graph)
{
    char buffer[1028]; // [esp+30h] [ebp-418h] BYREF
    int knotCount; // [esp+438h] [ebp-10h]
    int fileHandle; // [esp+43Ch] [ebp-Ch]
    const char *basePath; // [esp+440h] [ebp-8h]
    int knotIndex; // [esp+444h] [ebp-4h]

    if (!graph)
        MyAssertHandler(".\\qcommon\\graph.cpp", 121, 0, "%s", "graph");
    fileHandle = FS_FOpenTextFileWrite(graph->name);
    if (fileHandle)
    {
        if (!graph->knotCount)
            MyAssertHandler(".\\qcommon\\graph.cpp", 135, 0, "%s", "graph->knotCount");
        knotCount = graph->knotCount;
        Com_sprintf(buffer, 0x400u, "%s\n\n%d\n", "GRAPH_FLOAT_FILE", knotCount);
        FS_Write(buffer, &buffer[strlen(buffer) + 1] - &buffer[1], fileHandle);
        for (knotIndex = 0; knotIndex < knotCount; ++knotIndex)
        {
            Com_sprintf(buffer, 0x400u, "%.4f %.4f\n", graph->knots[knotIndex][0], graph->knots[knotIndex][1]);
            FS_Write(buffer, &buffer[strlen(buffer) + 1] - &buffer[1], fileHandle);
        }
        FS_FCloseFile(fileHandle);
        basePath = Sys_DefaultInstallPath();
        Com_Printf(18, "^7GraphFloat_SaveToFile: Successfully saved file [%s\\%s].\n", basePath, graph->name);
    }
    else
    {
        Com_PrintError(0, "GraphFloat_SaveToFile: Could not save file [%s].\n", graph->name);
    }
}

void __cdecl GraphFloat_DevGuiCB_Text(const DevGraph *devGuiGraph, float inputX, float inputY, char *text)
{
    float inputYa; // [esp+24h] [ebp+10h]

    inputYa = inputY * *((float *)devGuiGraph->data + 81);
    sprintf(text, "Fraction: %.3f, Value: %.3f", inputX, inputYa);
}

