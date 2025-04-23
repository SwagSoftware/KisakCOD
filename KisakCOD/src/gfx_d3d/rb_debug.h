#pragma once

int __cdecl RB_AddDebugLine(
    const float *start,
    const float *end,
    const float *color,
    bool depthTest,
    int vertCount,
    int vertLimit,
    struct GfxPointVertex *verts);

int __cdecl RB_EndDebugLines(bool depthTest, int vertCount, const struct GfxPointVertex *verts);

void __cdecl RB_DrawDebug(const GfxViewParms *viewParms);