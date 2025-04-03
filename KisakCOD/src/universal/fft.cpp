#include "fft.h"

void __cdecl FFT(complex_s *data_inout, unsigned int log2_count, int *bitSwap, complex_s *trigTable)
{
    int v4; // eax
    float imag; // ecx
    float *v6; // edi
    unsigned int v7; // ecx
    float *v8; // esi
    double v9; // st7
    double v10; // st6
    double v11; // st5
    double v12; // st4
    double v13; // st3
    double v14; // st0
    double v15; // st0
    double v16; // st1
    double v17; // st2
    double v18; // st3
    double v19; // rt0
    double v20; // st4
    float *v21; // edx
    float *v22; // eax
    float *v23; // edx
    float *v24; // eax
    float *v25; // edx
    float *v26; // ecx
    float *v27; // ecx
    float *v28; // eax
    unsigned int v29; // [esp+44h] [ebp-10C8h]
    unsigned int j; // [esp+48h] [ebp-10C4h]
    _DWORD v31[2]; // [esp+4Ch] [ebp-10C0h] BYREF
    float v32[510]; // [esp+54h] [ebp-10B8h] BYREF
    unsigned int v33; // [esp+84Ch] [ebp-8C0h]
    unsigned int v34; // [esp+850h] [ebp-8BCh]
    float v35[512]; // [esp+854h] [ebp-8B8h] BYREF
    float v36; // [esp+1054h] [ebp-B8h]
    float v37; // [esp+1058h] [ebp-B4h]
    float v38; // [esp+105Ch] [ebp-B0h]
    float v39; // [esp+1060h] [ebp-ACh]
    float v40; // [esp+1064h] [ebp-A8h]
    float v41; // [esp+1068h] [ebp-A4h]
    float v42; // [esp+106Ch] [ebp-A0h]
    float v43; // [esp+1070h] [ebp-9Ch]
    _DWORD *v44; // [esp+1074h] [ebp-98h]
    int v45; // [esp+1078h] [ebp-94h]
    float real; // [esp+107Ch] [ebp-90h]
    float v47; // [esp+1080h] [ebp-8Ch]
    float v48; // [esp+1084h] [ebp-88h]
    float v49; // [esp+1088h] [ebp-84h]
    float v50; // [esp+108Ch] [ebp-80h]
    float v51; // [esp+1090h] [ebp-7Ch]
    float v52; // [esp+1094h] [ebp-78h]
    float v53; // [esp+1098h] [ebp-74h]
    float v54; // [esp+109Ch] [ebp-70h]
    float v55; // [esp+10A0h] [ebp-6Ch]
    float v56; // [esp+10A4h] [ebp-68h]
    float v57; // [esp+10A8h] [ebp-64h]
    float v58; // [esp+10ACh] [ebp-60h]
    float v59; // [esp+10B0h] [ebp-5Ch]
    float v60; // [esp+10B4h] [ebp-58h]
    float v61; // [esp+10B8h] [ebp-54h]
    int v62; // [esp+10BCh] [ebp-50h]
    float v63; // [esp+10C0h] [ebp-4Ch]
    float v64; // [esp+10C4h] [ebp-48h]
    float v65; // [esp+10C8h] [ebp-44h]
    float v66; // [esp+10CCh] [ebp-40h]
    float v67; // [esp+10D0h] [ebp-3Ch]
    float v68; // [esp+10D4h] [ebp-38h]
    float v69; // [esp+10D8h] [ebp-34h]
    float v70; // [esp+10DCh] [ebp-30h]
    unsigned int i; // [esp+10E0h] [ebp-2Ch]
    float v72; // [esp+10E4h] [ebp-28h]
    float v73; // [esp+10E8h] [ebp-24h]
    float v74; // [esp+10ECh] [ebp-20h]
    float v75; // [esp+10F0h] [ebp-1Ch]
    float v76; // [esp+10F4h] [ebp-18h]
    float v77; // [esp+10F8h] [ebp-14h]
    float v78; // [esp+10FCh] [ebp-10h]
    float v79; // [esp+1100h] [ebp-Ch]
    unsigned int v80; // [esp+1108h] [ebp-4h]

    v80 = 1 << log2_count;
    v62 = 8 - log2_count;
    for (i = 0; i < v80; ++i)
    {
        v4 = bitSwap[i] >> v62;
        imag = data_inout[i].imag;
        v31[2 * v4] = LODWORD(data_inout[i].real);
        *(float *)&v31[2 * v4 + 1] = imag;
    }
    v44 = v31;
    v6 = (float *)v31;
    v7 = v80 >> 2;
    v8 = v32;
    do
    {
        v9 = v6[1];
        v10 = v6[4];
        v11 = v6[5];
        v12 = *v6;
        v13 = v12 + *v8;
        v14 = v10 + v8[4];
        *v6 = v13 + v14;
        v6[4] = v13 - v14;
        v15 = v6[5] + v8[5];
        v16 = v6[1] + v8[1];
        v6[1] = v16 + v15;
        v6[5] = v16 - v15;
        v17 = v8[5] - v11;
        v18 = *v8 - v12;
        *v8 = v18 - v17;
        v19 = v8[4] - v10;
        v8[4] = v17 + v18;
        v20 = v8[1] - v9;
        v8[1] = v19 + v20;
        v8[5] = v20 - v19;
        --v7;
        v6 += 8;
        v8 += 8;
    } while (v7);
    for (i = 0; i < v80; i += 4)
    {
        v33 = i >> 2;
        v35[8 * (i >> 2)] = *(float *)&v31[2 * i];
        v35[8 * v33 + 1] = v32[2 * i];
        v35[8 * v33 + 2] = v32[2 * i + 2];
        v35[8 * v33 + 3] = v32[2 * i + 4];
        v35[8 * v33 + 4] = *(float *)&v31[2 * i + 1];
        v35[8 * v33 + 5] = v32[2 * i + 1];
        v35[8 * v33 + 6] = v32[2 * i + 3];
        v35[8 * v33 + 7] = v32[2 * i + 5];
    }
    v34 = 4;
    v62 = 5;
    while (v34 < v80)
    {
        v45 = 2 * v34;
        for (j = 0; j < v34; j += 4)
        {
            real = trigTable[j << v62].real;
            v47 = trigTable[(j + 1) << v62].real;
            v48 = trigTable[(j + 2) << v62].real;
            v49 = trigTable[(j + 3) << v62].real;
            v50 = trigTable[j << v62].imag;
            v51 = trigTable[(j + 1) << v62].imag;
            v52 = trigTable[(j + 2) << v62].imag;
            v53 = trigTable[(j + 3) << v62].imag;
            v63 = real;
            v64 = v47;
            v65 = v48;
            v66 = v49;
            v67 = v50;
            v68 = v51;
            v69 = v52;
            v70 = v53;
            for (i = j; i < v80; i += v45)
            {
                v33 = i >> 2;
                v29 = (v34 + i) >> 2;
                v21 = &v35[8 * v29];
                v72 = *v21;
                v73 = v21[1];
                v74 = v21[2];
                v75 = v21[3];
                v22 = &v35[8 * v29 + 4];
                v76 = *v22;
                v77 = v22[1];
                v78 = v22[2];
                v79 = v22[3];
                v54 = v63 * v72;
                v55 = v64 * v73;
                v56 = v65 * v74;
                v57 = v66 * v75;
                v58 = v67 * v76;
                v59 = v68 * v77;
                v60 = v69 * v78;
                v61 = v70 * v79;
                v36 = v54 - v58;
                v37 = v55 - v59;
                v38 = v56 - v60;
                v39 = v57 - v61;
                v54 = v63 * v76;
                v55 = v64 * v77;
                v56 = v65 * v78;
                v57 = v66 * v79;
                v58 = v67 * v72;
                v59 = v68 * v73;
                v60 = v69 * v74;
                v61 = v70 * v75;
                v40 = v54 + v58;
                v41 = v55 + v59;
                v42 = v56 + v60;
                v43 = v57 + v61;
                v23 = &v35[8 * (i >> 2)];
                v72 = *v23;
                v73 = v23[1];
                v74 = v23[2];
                v75 = v23[3];
                v24 = &v35[8 * (i >> 2) + 4];
                v76 = *v24;
                v77 = v24[1];
                v78 = v24[2];
                v79 = v24[3];
                v54 = v72 - v36;
                v55 = v73 - v37;
                v56 = v74 - v38;
                v57 = v75 - v39;
                v58 = v76 - v40;
                v59 = v77 - v41;
                v60 = v78 - v42;
                v61 = v79 - v43;
                v72 = v72 + v36;
                v73 = v73 + v37;
                v74 = v74 + v38;
                v75 = v75 + v39;
                v76 = v76 + v40;
                v77 = v77 + v41;
                v78 = v78 + v42;
                v79 = v79 + v43;
                v25 = &v35[8 * v29];
                *v25 = v54;
                v25[1] = v55;
                v25[2] = v56;
                v25[3] = v57;
                v26 = &v35[8 * v29 + 4];
                *v26 = v58;
                v26[1] = v59;
                v26[2] = v60;
                v26[3] = v61;
                v27 = &v35[8 * v33];
                *v27 = v72;
                v27[1] = v73;
                v27[2] = v74;
                v27[3] = v75;
                v28 = &v35[8 * v33 + 4];
                *v28 = v76;
                v28[1] = v77;
                v28[2] = v78;
                v28[3] = v79;
            }
        }
        v34 = v45;
        --v62;
    }
    for (i = 0; i < v80; i += 4)
    {
        v33 = i >> 2;
        data_inout[i].real = v35[8 * (i >> 2)];
        data_inout[i].imag = v35[8 * v33 + 4];
        data_inout[i + 1].real = v35[8 * v33 + 1];
        data_inout[i + 1].imag = v35[8 * v33 + 5];
        data_inout[i + 2].real = v35[8 * v33 + 2];
        data_inout[i + 2].imag = v35[8 * v33 + 6];
        data_inout[i + 3].real = v35[8 * v33 + 3];
        data_inout[i + 3].imag = v35[8 * v33 + 7];
    }
}

void __cdecl FFT_Init(int *fftBitswap, complex_s *fftTrigTable)
{
    float v2; // [esp+8h] [ebp-Ch]
    int fftIndex; // [esp+Ch] [ebp-8h]
    int logIndex; // [esp+10h] [ebp-4h]

    if (!fftBitswap)
        MyAssertHandler(".\\universal\\fft.cpp", 11, 0, "%s", "fftBitswap");
    if (!fftTrigTable)
        MyAssertHandler(".\\universal\\fft.cpp", 12, 0, "%s", "fftTrigTable");
    for (fftIndex = 0; fftIndex < 256; ++fftIndex)
    {
        fftBitswap[fftIndex] = 0;
        for (logIndex = 0; logIndex < 8; ++logIndex)
        {
            if ((fftIndex & (1 << logIndex)) != 0)
                fftBitswap[fftIndex] |= 1 << (7 - logIndex);
        }
        v2 = (double)fftIndex * 0.02454369328916073;
        fftTrigTable[fftIndex].real = cos(v2);
        fftTrigTable[fftIndex].imag = sin(v2);
    }
}