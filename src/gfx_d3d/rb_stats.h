#pragma once
#include "rb_backend.h"

void __cdecl TRACK_rb_stats();
void __cdecl R_TrackPrims(GfxCmdBufState *state, GfxPrimStatsTarget target);
void __cdecl RB_TrackImmediatePrims(GfxPrimStatsTarget target);
void __cdecl RB_EndTrackImmediatePrims();
void __cdecl RB_TrackDrawPrimCall(int triCount);
int __cdecl RB_Stats_TotalIndexCount();
int __cdecl RB_Stats_TotalVertexCount();
int __cdecl RB_Stats_TotalPrimCount();
int __cdecl RB_Stats_ViewIndexCount(const GfxViewStats *viewStats);
void __cdecl RB_Stats_f();
void __cdecl RB_Stats_Display(const GfxFrameStats *frameStats);
void __cdecl RB_Stats_Summarize(const char *label, const GfxViewStats *viewStats);
void __cdecl RB_Stats_AccumulatePrimStats(const GfxPrimStats *primStats, GfxPrimStats *total);
void __cdecl RB_Stats_SummarizePrimStats(const char *label, const GfxPrimStats *primStats);
void __cdecl RB_Stats_UpdateMaxs(const GfxFrameStats *frameStatsCur, GfxFrameStats *frameStatsMax);
void __cdecl RB_DrawPrimHistogramOverlay();


extern GfxPrimStats *backupPrimStats;
extern GfxViewStats *g_viewStats;
extern GfxPrimStats *g_primStats;
extern GfxFrameStats g_frameStatsCur;