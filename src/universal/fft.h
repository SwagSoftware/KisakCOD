#pragma once
#include <gfx_d3d/r_material.h>


void __cdecl FFT_Init(int *fftBitswap, complex_s *fftTrigTable);
void __cdecl FFT(complex_s *data_inout, unsigned int log2_count, int *bitSwap, complex_s *trigTable);