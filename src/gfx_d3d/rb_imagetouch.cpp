#include "rb_backend.h"
#include "rb_shade.h"
#include "rb_logfile.h"
#include "r_image.h"
#include "rb_state.h"
#include "r_utils.h"

int __cdecl RB_CompareTouchImages(int *e0, int *e1)
{
    int image; // [esp+0h] [ebp-Ch]
    int image_4; // [esp+4h] [ebp-8h]

    image = *e0;
    image_4 = *e1;
    if (!*(_BYTE *)(*e1 + 11))
        return -1;
    if (!*(_BYTE *)(image + 11))
        return 1;
    if (*(_DWORD *)(image_4 + 16) != *(_DWORD *)(image + 16))
        return *(_DWORD *)(image_4 + 16) - *(_DWORD *)(image + 16);
    if (*(unsigned __int8 *)(image + 11) == *(unsigned __int8 *)(image_4 + 11))
        return 0;
    return *(unsigned __int8 *)(image + 11) - *(unsigned __int8 *)(image_4 + 11);
}

void __cdecl RB_TouchImage(GfxImage *image)
{
    if (image->mapType == MAPTYPE_2D)
    {
        R_SetCodeImageTexture(&gfxCmdBufSourceState, TEXTURE_SRC_CODE_FEEDBACK, image);
        R_Set2D(&gfxCmdBufSourceState);
        RB_DrawStretchPic(
            rgp.feedbackReplaceMaterial,
            0.0,
            0.0,
            8.0,
            8.0,
            0.0,
            0.0,
            1.0,
            1.0,
            0xFFFFFFFF,
            GFX_PRIM_STATS_CODE);
        RB_EndTessSurface();
    }
}

void __cdecl RB_TouchAllImages()
{
    const char *v0; // eax
    const char *v1; // eax
    int v2; // [esp+0h] [ebp-201Ch]
    int hr; // [esp+4h] [ebp-2018h]
    bool inScene; // [esp+Bh] [ebp-2011h]
    unsigned int i; // [esp+Ch] [ebp-2010h]
    int v6; // [esp+10h] [ebp-200Ch]
    ImageList imageList; // [esp+14h] [ebp-2008h] BYREF

    inScene = dx.inScene;
    if (!dx.inScene)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.device->BeginScene()\n");
            hr = dx.device->BeginScene();
            if (hr < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v0 = R_ErrorDescription(hr);
                    Com_Error(ERR_FATAL, ".\\rb_imagetouch.cpp (%i) dx.device->BeginScene() failed: %s\n", 60, v0);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
    if (tess.indexCount)
        RB_EndTessSurface();
    R_GetImageList(&imageList);
    qsort(imageList.image, imageList.count, 4u, (int(__cdecl *)(const void *, const void *))RB_CompareTouchImages);
    v6 = 0;
    for (i = 0; i < imageList.count && imageList.image[i]->semantic; ++i)
    {
        RB_TouchImage(imageList.image[i]);
        v6 += imageList.image[i]->cardMemory.platform[0];
    }
    R_SetCodeImageTexture(&gfxCmdBufSourceState, TEXTURE_SRC_CODE_FEEDBACK, 0);
    if (!inScene)
    {
        do
        {
            if (r_logFile && r_logFile->current.integer)
                RB_LogPrint("dx.device->EndScene()\n");
            v2 = dx.device->EndScene();
            if (v2 < 0)
            {
                do
                {
                    ++g_disableRendering;
                    v1 = R_ErrorDescription(v2);
                    Com_Error(ERR_FATAL, ".\\rb_imagetouch.cpp (%i) dx.device->EndScene() failed: %s\n", 77, v1);
                } while (alwaysfails);
            }
        } while (alwaysfails);
    }
}