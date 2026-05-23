#pragma once
#include <cstdint>

void __cdecl R_PixelCost_PrintColorCodeKey();

void __cdecl RB_PixelCost_BuildColorCodeMap(uint8_t (*pixels)[4], int pixelCount);
