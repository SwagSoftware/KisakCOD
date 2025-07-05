#pragma once

#ifndef KISAK_SP
#error This file is for SinglePlayer only
#endif

#include "cg_local.h"

float __fastcall CG_FadeObjectives(const cg_s *cgameGlob);
void __fastcall CG_DrawObjectiveBackdrop(const cg_s *cgameGlob, const float *color);
void __fastcall CG_DrawObjectiveHeader(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    int textStyle);
const char *__fastcall CG_WordWrap(
    const char *inputText,
    Font_s *font,
    double scale,
    int maxChars,
    int maxPixelWidth,
    int *charCount,
    int *a7);
void __fastcall CG_DrawObjectiveList(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    int textStyle);
void __fastcall CG_DrawPausedMenuLine(
    int localClientNum,
    const rectDef_s *rect,
    Font_s *font,
    double scale,
    float *color,
    int textStyle);
int __fastcall CG_DrawScoreboard(int localClientNum);
void __fastcall CG_ParseObjectiveChange(int localClientNum, unsigned int num);
