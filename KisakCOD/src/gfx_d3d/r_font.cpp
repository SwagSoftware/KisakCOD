#include "r_font.h"
#include <stringed/stringed_hooks.h>
#include <qcommon/mem_track.h>
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include "r_init.h"
#include <database/database.h>

Font_s *registeredFont[16];
int registeredFontCount;
const char MYRANDOMCHARS[63] =
{
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890"
};

const Glyph *__cdecl R_GetCharacterGlyph(Font_s *font, unsigned int letter)
{
    Glyph *glyph; // [esp+0h] [ebp-10h]
    int top; // [esp+4h] [ebp-Ch]
    int bottom; // [esp+8h] [ebp-8h]
    int mid; // [esp+Ch] [ebp-4h]

    if (letter < 0x20 || letter > 0x7F)
    {
        top = font->glyphCount - 1;
        bottom = 96;
        while (bottom <= top)
        {
            mid = (bottom + top) / 2;
            if (font->glyphs[mid].letter == letter)
                return &font->glyphs[mid];
            if (font->glyphs[mid].letter >= letter)
                top = mid - 1;
            else
                bottom = mid + 1;
        }
        return font->glyphs + 14;
    }
    else
    {
        glyph = &font->glyphs[letter - 32];
        if (glyph->letter != letter)
            MyAssertHandler(".\\r_font.cpp", 51, 0, "%s", "glyph->letter == letter");
        return glyph;
    }
}

unsigned int __cdecl R_FontGetRandomLetter(Font_s *font, int seed)
{
    return MYRANDOMCHARS[RandWithSeed(&seed) % 0x3E];
}

void __cdecl TRACK_r_font()
{
    track_static_alloc_internal(registeredFont, 64, "registeredFont", 18);
}

Font_s *__cdecl R_RegisterFont(const char *name, int imageTrack)
{
    if (useFastFile->current.enabled)
        return (Font_s *)((int(__cdecl *)(const char *, int))R_RegisterFont_FastFile)(name, imageTrack);
    else
        return R_RegisterFont_LoadObj(name, imageTrack);
}

Font_s *__cdecl R_RegisterFont_FastFile(const char *fontName)
{
    return DB_FindXAssetHeader(ASSET_TYPE_FONT, fontName).font;
}

Font_s *__cdecl R_LoadFont(const char *fontName, int imageTrack)
{
    Material *v3; // eax
    char glowMaterialName[64]; // [esp+0h] [ebp-68h] BYREF
    Font_s *font; // [esp+44h] [ebp-24h]
    int totalMemSize; // [esp+48h] [ebp-20h]
    int file; // [esp+4Ch] [ebp-1Ch] BYREF
    int materialNameOffset; // [esp+50h] [ebp-18h] BYREF
    const char *materialName; // [esp+54h] [ebp-14h]
    int fileLen; // [esp+58h] [ebp-10h]
    int varMemSize; // [esp+5Ch] [ebp-Ch]
    int fontNameOffset; // [esp+60h] [ebp-8h] BYREF
    char *variableFontData; // [esp+64h] [ebp-4h]

    fileLen = FS_FOpenFileRead(fontName, &file);
    if (fileLen >= 0)
    {
        if (fileLen >= 16)
        {
            varMemSize = fileLen - 16;
            totalMemSize = fileLen - 16 + 24;
            font = (Font_s *)Hunk_Alloc(totalMemSize, "R_LoadFont", 22);
            variableFontData = (char *)&font[1];
            FS_Read((unsigned __int8 *)&fontNameOffset, 4u, file);
            FS_Read((unsigned __int8 *)&font->pixelHeight, 4u, file);
            FS_Read((unsigned __int8 *)&font->glyphCount, 4u, file);
            FS_Read((unsigned __int8 *)&materialNameOffset, 4u, file);
            FS_Read((unsigned __int8 *)variableFontData, varMemSize, file);
            FS_FCloseFile(file);
            font->glyphs = (Glyph *)variableFontData;
            font->fontName = &variableFontData[fontNameOffset - 16];
            materialName = &variableFontData[materialNameOffset - 16];
            font->material = Material_RegisterHandle((char *)materialName, imageTrack);
            Com_sprintf(glowMaterialName, 0x3Fu, "%s_glow", materialName);
            v3 = Material_RegisterHandle(glowMaterialName, imageTrack);
            font->glowMaterial = v3;
            return font;
        }
        else
        {
            FS_FCloseFile(file);
            Com_PrintError(8, "ERROR: Font file '%s' too small\n", fontName);
            return 0;
        }
    }
    else
    {
        if (fileLen == -2)
            Com_PrintError(8, "ERROR: Couldn't find font in iwd files or localized directories '%s'\n", fontName);
        else
            Com_PrintError(8, "ERROR: Couldn't find font '%s'\n", fontName);
        return 0;
    }
}

Font_s *__cdecl R_RegisterFont_LoadObj(const char *fontName, int imageTrack)
{
    Font_s *font; // [esp+0h] [ebp-8h]
    int fontIndex; // [esp+4h] [ebp-4h]

    if (!rg.registered)
        MyAssertHandler(".\\r_font.cpp", 101, 0, "%s", "rg.registered");
    for (fontIndex = 0; fontIndex < registeredFontCount; ++fontIndex)
    {
        if (!I_stricmp(fontName, registeredFont[fontIndex]->fontName))
            return registeredFont[fontIndex];
    }
    if (registeredFontCount < 16)
    {
        font = R_LoadFont(fontName, imageTrack);
        if (!font)
            Com_Error(ERR_DROP, "R_RegisterFont: Error while reading font '%s'", fontName);
        registeredFont[registeredFontCount++] = font;
        return font;
    }
    else
    {
        Com_Error(ERR_DROP, "R_RegisterFont: Too many fonts registered already.\n");
        return 0;
    }
}

double __cdecl R_NormalizedTextScale(Font_s *font, float scale)
{
    if (!font)
        MyAssertHandler(".\\r_font.cpp", 158, 0, "%s", "font");
    if (font->pixelHeight <= 0)
        MyAssertHandler(".\\r_font.cpp", 159, 0, "%s", "font->pixelHeight > 0");
    return (float)(scale * 48.0 / (double)font->pixelHeight);
}

int __cdecl R_LetterWidth(unsigned int letter, Font_s *font)
{
    return R_GetCharacterGlyph(font, letter)->dx;
}

int __cdecl R_TextWidth(const char *text, int maxChars, Font_s *font)
{
    unsigned int letter; // [esp+0h] [ebp-10h]
    int lineWidth; // [esp+4h] [ebp-Ch]
    int maxWidth; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    if (!text)
        MyAssertHandler(".\\r_font.cpp", 177, 0, "%s", "text");
    if (!font)
        MyAssertHandler(".\\r_font.cpp", 178, 0, "%s", "font");
    lineWidth = 0;
    maxWidth = 0;
    if (maxChars <= 0)
        maxChars = 0x7FFFFFFF;
    count = 0;
    while (*text && count < maxChars)
    {
        letter = SEH_ReadCharFromString(&text, 0);
        if (letter == 13 || letter == 10)
        {
            lineWidth = 0;
        }
        else if (letter == 94 && text && *text != 94 && *text >= 48 && *text <= 57)
        {
            ++text;
        }
        else
        {
            lineWidth += R_GetCharacterGlyph(font, letter)->dx;
            if (maxWidth < lineWidth)
                maxWidth = lineWidth;
            ++count;
        }
    }
    return maxWidth;
}

int __cdecl R_TextHeight(Font_s *font)
{
    if (!font)
        MyAssertHandler(".\\r_font.cpp", 214, 0, "%s", "font");
    return font->pixelHeight;
}

const char *__cdecl R_TextLineWrapPosition(
    const char *text,
    int bufferSize,
    int pixelsAvailable,
    Font_s *font,
    float scale)
{
    int pixelsUsed; // [esp+Ch] [ebp-14h]
    const char *preLetterPos; // [esp+10h] [ebp-10h]
    const char *wrapPos; // [esp+14h] [ebp-Ch]
    const char *parsePos; // [esp+18h] [ebp-8h] BYREF
    unsigned int letter; // [esp+1Ch] [ebp-4h]

    if (!text)
        MyAssertHandler(".\\r_font.cpp", 233, 0, "%s", "text");
    pixelsUsed = 0;
    if (bufferSize <= 0)
        bufferSize = 0x7FFFFFFF;
    wrapPos = 0;
    preLetterPos = 0;
    parsePos = text;
    while (*parsePos)
    {
        preLetterPos = parsePos;
        letter = SEH_ReadCharFromString(&parsePos, 0);
        if (letter == 13)
        {
            pixelsUsed = 0;
        }
        else
        {
            if (letter == 10)
                return parsePos;
            if (letter == 94 && parsePos && *parsePos != 94 && *parsePos >= 48 && *parsePos <= 57)
            {
                ++parsePos;
            }
            else if (letter == 94 && (*parsePos == 1 || *parsePos == 2))
            {
                if (font)
                    pixelsUsed += (font->pixelHeight * (parsePos[1] - 16) + 16) / 32;
                if (preLetterPos != text)
                    wrapPos = preLetterPos;
                parsePos += 7;
            }
            else
            {
                if (font)
                    pixelsUsed += R_LetterWidth(letter, font);
                if (preLetterPos != text && letter < 0x100 && isspace(letter))
                    wrapPos = preLetterPos;
            }
            if (wrapPos && (double)pixelsAvailable < (double)pixelsUsed * scale)
                return wrapPos;
            if (parsePos - text > bufferSize)
            {
                if (wrapPos)
                    return wrapPos;
                else
                    return preLetterPos;
            }
        }
    }
    if (parsePos - text != bufferSize)
        return parsePos;
    if (wrapPos)
        return wrapPos;
    else
        return preLetterPos;
}

int __cdecl R_ConsoleTextWidth(const char *textPool, int poolSize, int firstChar, int charCount, Font_s *font)
{
    int indexMask; // [esp+0h] [ebp-18h]
    int parsePos; // [esp+4h] [ebp-14h]
    int width; // [esp+8h] [ebp-10h]
    unsigned int letter; // [esp+Ch] [ebp-Ch]
    int usedCharCount; // [esp+10h] [ebp-8h] BYREF
    int stopPos; // [esp+14h] [ebp-4h]

    if ((poolSize & (poolSize - 1)) != 0)
        MyAssertHandler(".\\r_font.cpp", 302, 0, "%s", "IsPowerOf2( poolSize )");
    indexMask = poolSize - 1;
    stopPos = (poolSize - 1) & (charCount + firstChar);
    parsePos = firstChar;
    width = 0;
    while (parsePos != stopPos)
    {
        letter = SEH_DecodeLetter(textPool[parsePos], textPool[indexMask & (parsePos + 1)], &usedCharCount, 0);
        parsePos = indexMask & (usedCharCount + parsePos);
        if (letter == 94
            && &textPool[parsePos]
            && textPool[parsePos] != 94
                && textPool[parsePos] >= 48
                && textPool[parsePos] <= 57)
        {
            parsePos = indexMask & (parsePos + 1);
        }
        else if (letter == 94 && (textPool[parsePos] == 1 || textPool[parsePos] == 2))
        {
            width += (font->pixelHeight * (textPool[indexMask & (parsePos + 1)] - 16) + 16) / 32;
            parsePos = indexMask & (parsePos + 7);
        }
        else
        {
            width += R_LetterWidth(letter, font);
        }
    }
    return width;
}

void __cdecl R_InitFonts()
{
    if (registeredFontCount)
        MyAssertHandler(".\\r_font.cpp", 144, 0, "%s", "registeredFontCount == 0");
}

void __cdecl R_ShutdownFonts()
{
    registeredFontCount = 0;
}