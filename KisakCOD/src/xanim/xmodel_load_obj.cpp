#include "xmodel.h"

int __cdecl XModelGetStaticBounds(const XModel *model, float (*axis)[3], float *mins, float *maxs)
{
    float v5; // [esp+0h] [ebp-34h]
    float v6; // [esp+4h] [ebp-30h]
    float v7; // [esp+8h] [ebp-2Ch]
    int j; // [esp+Ch] [ebp-28h]
    int k; // [esp+10h] [ebp-24h]
    const XModelCollSurf_s *csurf; // [esp+14h] [ebp-20h]
    float rotated[3]; // [esp+18h] [ebp-1Ch] BYREF
    int i; // [esp+24h] [ebp-10h]
    float corner[3]; // [esp+28h] [ebp-Ch] BYREF

    if (model->numCollSurfs)
    {
        *mins = 3.4028235e38;
        mins[1] = 3.4028235e38;
        mins[2] = 3.4028235e38;
        *maxs = -3.4028235e38;
        maxs[1] = -3.4028235e38;
        maxs[2] = -3.4028235e38;
        for (i = 0; i < model->numCollSurfs; ++i)
        {
            csurf = &model->collSurfs[i];
            for (k = 0; k < 8; ++k)
            {
                if ((k & 1) != 0)
                    v7 = csurf->mins[0];
                else
                    v7 = csurf->maxs[0];
                corner[0] = v7;
                if ((k & 2) != 0)
                    v6 = csurf->mins[1];
                else
                    v6 = csurf->maxs[1];
                corner[1] = v6;
                if ((k & 4) != 0)
                    v5 = csurf->mins[2];
                else
                    v5 = csurf->maxs[2];
                corner[2] = v5;
                MatrixTransformVector(corner, axis, rotated);
                for (j = 0; j < 3; ++j)
                {
                    if (rotated[j] < (double)mins[j])
                        mins[j] = rotated[j];
                    if (rotated[j] > (double)maxs[j])
                        maxs[j] = rotated[j];
                }
            }
        }
        return 1;
    }
    else
    {
        if (model->contents)
            MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 1218, 0, "%s", "!model->contents");
        return 0;
    }
}