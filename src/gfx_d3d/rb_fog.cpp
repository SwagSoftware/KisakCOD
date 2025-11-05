#include "rb_fog.h"
#include "r_dvars.h"
#include "r_utils.h"
#include "r_state.h"


void __cdecl R_SetFrameFog(GfxCmdBufInput *input)
{
    float fogColorVec[4]; // [esp+1Ch] [ebp-28h] BYREF
    const GfxBackEndData *data; // [esp+2Ch] [ebp-18h]
    float parms[4]; // [esp+30h] [ebp-14h] BYREF
    const GfxFog *fog; // [esp+40h] [ebp-4h]

    if (r_fog->current.enabled)
    {
        data = input->data;
        iassert( data );
        fog = &data->fogSettings;
        Byte4UnpackBgra((const unsigned __int8 *)&data->fogSettings.color, fogColorVec);
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_FOG_COLOR, fogColorVec);
        parms[0] = 0.0;
        parms[1] = 1.0;
        parms[2] = -fog->density;
        parms[3] = fog->fogStart * fog->density;
        R_SetInputCodeConstantFromVec4(input, CONST_SRC_CODE_FOG, parms);
    }
    else
    {
        R_SetInputCodeConstant(input, CONST_SRC_CODE_FOG, 0.0, 1.0, 0.0, 0.0);
    }
}