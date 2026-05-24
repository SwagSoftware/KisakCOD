#pragma once
#include <cstdint>
//#include "r_material.h"

struct Glyph // sizeof=0x18
{
    uint16_t letter;
    char x0;
    char y0;
    uint8_t dx;
    uint8_t pixelWidth;
    uint8_t pixelHeight;
    // padding byte
    float s0;
    float t0;
    float s1;
    float t1;
};

struct Font_s // sizeof=0x18 // (SP/MP same)
{                                       // ...
    const char *fontName;
    int pixelHeight;
    int glyphCount;
    struct Material *material;
    struct Material *glowMaterial;
    Glyph *glyphs;
};
static_assert(sizeof(Font_s) == 24);

const Glyph *__cdecl R_GetCharacterGlyph(Font_s *font, uint32_t letter);
uint32_t __cdecl R_FontGetRandomLetter(Font_s *font, int seed);
void __cdecl TRACK_r_font();
Font_s *__cdecl R_RegisterFont(const char *name, int imageTrack);
Font_s *__cdecl R_RegisterFont_FastFile(const char *fontName);
Font_s *__cdecl R_RegisterFont_LoadObj(const char *fontName, int imageTrack);
Font_s *__cdecl R_LoadFont(const char *fontName, int imageTrack);
double __cdecl R_NormalizedTextScale(Font_s *font, float scale);
int __cdecl R_LetterWidth(uint32_t letter, Font_s *font);
int __cdecl R_TextWidth(const char *text, int maxChars, Font_s *font);
int __cdecl R_TextHeight(Font_s *font);
const char *__cdecl R_TextLineWrapPosition(
    const char *text,
    int bufferSize,
    int pixelsAvailable,
    Font_s *font,
    float scale);
int __cdecl R_ConsoleTextWidth(const char *textPool, int poolSize, int firstChar, int charCount, Font_s *font);
void __cdecl R_InitFonts();
void __cdecl R_ShutdownFonts();
