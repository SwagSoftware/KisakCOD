#include "r_image.h"

void __cdecl R_DownsampleMipMapBilinear(
    const unsigned __int8 *src,
    int srcWidth,
    int srcHeight,
    int texelPitch,
    unsigned __int8 *dst)
{
    int srcRowPitch; // [esp+0h] [ebp-24h]
    int channelIndex; // [esp+4h] [ebp-20h]
    int channelIndexa; // [esp+4h] [ebp-20h]
    unsigned __int8 *dstPos; // [esp+8h] [ebp-1Ch]
    unsigned __int8 *dstPosa; // [esp+8h] [ebp-1Ch]
    int dstWidth; // [esp+Ch] [ebp-18h]
    int dstWidtha; // [esp+Ch] [ebp-18h]
    int dstHeight; // [esp+10h] [ebp-14h]
    int colIndex; // [esp+14h] [ebp-10h]
    int colIndexa; // [esp+14h] [ebp-10h]
    const unsigned __int8 *srcPos; // [esp+1Ch] [ebp-8h]
    const unsigned __int8 *srcPosa; // [esp+1Ch] [ebp-8h]
    int rowIndex; // [esp+20h] [ebp-4h]

    iassert( src );
    iassert( (srcWidth > 0) );
    iassert( (srcHeight > 0) );
    iassert( (texelPitch > 0) );
    iassert( dst );
    if (srcWidth != 1 || srcHeight != 1)
    {
        srcPos = src;
        dstPos = dst;
        dstWidth = srcWidth >> 1;
        dstHeight = srcHeight >> 1;
        srcRowPitch = texelPitch * srcWidth;
        if (srcWidth >> 1 && dstHeight)
        {
            for (rowIndex = 0; rowIndex < dstHeight; ++rowIndex)
            {
                srcPosa = &src[2 * rowIndex * srcRowPitch];
                dstPosa = &dst[rowIndex * texelPitch * (srcWidth >> 1)];
                for (colIndexa = 0; colIndexa < dstWidth; ++colIndexa)
                {
                    for (channelIndexa = 0; channelIndexa < texelPitch; ++channelIndexa)
                        dstPosa[channelIndexa] = (srcPosa[texelPitch + channelIndexa + srcRowPitch]
                            + srcPosa[channelIndexa + srcRowPitch]
                            + srcPosa[texelPitch + channelIndexa]
                            + srcPosa[channelIndexa]) >> 2;
                    dstPosa += texelPitch;
                    srcPosa += 2 * texelPitch;
                }
            }
        }
        else
        {
            dstWidtha = dstHeight + dstWidth;
            for (colIndex = 0; colIndex < dstWidtha; ++colIndex)
            {
                for (channelIndex = 0; channelIndex < texelPitch; ++channelIndex)
                    dstPos[channelIndex] = (srcPos[texelPitch + channelIndex] + srcPos[channelIndex]) >> 1;
                dstPos += texelPitch;
                srcPos += 2 * texelPitch;
            }
        }
    }
}