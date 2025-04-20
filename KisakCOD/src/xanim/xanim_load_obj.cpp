#include "xanim.h"
#include "xmodel.h"
#include <gfx_d3d/r_model.h>
#include <universal/com_files.h>
#include <qcommon/qcommon.h>

const char *g_highPrecisionParts[18] =
{
  "j_ankle_le",
  "j_ankle_ri",
  "j_clavicle_le",
  "j_clavicle_ri",
  "j_elbow_le",
  "j_elbow_ri",
  "j_hip_le",
  "j_hip_ri",
  "j_knee_le",
  "j_knee_ri",
  "j_mainroot",
  "j_shoulder_le",
  "j_shoulder_ri",
  "j_spine4",
  "j_spinelower",
  "j_spineupper",
  "j_wrist_le",
  "j_wrist_ri"
}; // idb

int g_highPrecisionPart[128];

HunkUser *g_animUser;
XAnimPartQuatPtr *g_partQuatArray;
XAnimPartTransPtr *g_partTransArray;
char *g_simpleQuatBits;

unsigned int __cdecl XAnimGetPartQuatType(unsigned int animPartIndex)
{
    XAnimPartQuat *quat; // [esp+0h] [ebp-4h]

    quat = g_partQuatArray[animPartIndex].quat;
    if (((1 << (animPartIndex & 7)) & g_simpleQuatBits[animPartIndex >> 3]) != 0)
    {
        if (quat)
        {
            if (quat->size)
                return 1;
            else
                return 3;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if (!quat)
            MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 668, 0, "%s", "quat");
        if (quat->size)
            return 2;
        else
            return 4;
    }
}

unsigned int __cdecl XAnimGetPartTransType(unsigned int animPartIndex)
{
    XAnimPartTrans *trans; // [esp+0h] [ebp-4h]

    trans = g_partTransArray[animPartIndex].trans;
    if (!trans)
        return 8;
    if (!trans->size)
        return 7;
    if (trans->smallTrans)
        return 5;
    return 6;
}

int __cdecl XAnimCompareTransParts(const void* arg1, const void* arg2)
{
    unsigned __int16 *p0 = (unsigned __int16 *)arg1;
    unsigned __int16 *p1 = (unsigned __int16 *)arg2;
    unsigned int PartTransType; // eax
    unsigned int partArrayIndices[2]; // [esp+0h] [ebp-18h]
    int diff; // [esp+8h] [ebp-10h]
    unsigned int type[2]; // [esp+Ch] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-4h]

    partArrayIndices[0] = *p0;
    partArrayIndices[1] = *p1;
    for (i = 0; i < 2; ++i)
    {
        PartTransType = XAnimGetPartTransType(partArrayIndices[i]);
        type[i] = PartTransType;
    }
    diff = type[0] - type[1];
    if (type[0] == type[1])
        return g_partTransArray[partArrayIndices[0]].partIndex - g_partTransArray[partArrayIndices[1]].partIndex;
    else
        return diff;
}

void __cdecl XAnimEmitFrameIndices(
    unsigned int tableSize,
    unsigned __int8 **dataByte,
    __int16 **dataShort,
    XAnimIndices *indices,
    XAnimDynamicIndices *frameIndices,
    bool useSmallIndices)
{
    unsigned int i; // [esp+4h] [ebp-8h]
    unsigned int longTableSize; // [esp+8h] [ebp-4h]

    if (useSmallIndices)
    {
        memcpy(*dataByte, frameIndices->_1, tableSize + 1);
        *dataByte += tableSize + 1;
    }
    else if (tableSize >= 0x40)
    {
        memcpy(indices->_1, frameIndices->_1, 2 * tableSize + 2);
        indices->_1 += 2 * tableSize + 2;
        longTableSize = ((tableSize - 1) >> 8) + 1;
        if (frameIndices->_2[0])
            MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 817, 0, "%s", "!frameIndices->_2[0]");
        for (i = 0; i < longTableSize; ++i)
            (*dataShort)[i] = frameIndices[256 * i]._2[0];
        (*dataShort)[longTableSize] = frameIndices[tableSize]._2[0];
        *dataShort += longTableSize + 1;
    }
    else
    {
        memcpy(*dataShort, frameIndices->_1, 2 * tableSize + 2);
        *dataShort += tableSize + 1;
    }
}

int __cdecl XAnimCompareQuatParts(const void *arg1, const void *arg2)
{
    unsigned __int16 *p0 = (unsigned __int16*)arg1;
    unsigned __int16 *p1 = (unsigned __int16 *)arg2;

    unsigned int PartQuatType; // eax
    unsigned int partArrayIndices[2]; // [esp+0h] [ebp-18h]
    int diff; // [esp+8h] [ebp-10h]
    unsigned int type[2]; // [esp+Ch] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-4h]

    partArrayIndices[0] = *p0;
    partArrayIndices[1] = *p1;
    for (i = 0; i < 2; ++i)
    {
        PartQuatType = XAnimGetPartQuatType(partArrayIndices[i]);
        type[i] = PartQuatType;
    }
    diff = type[0] - type[1];
    if (type[0] == type[1])
        return g_partQuatArray[partArrayIndices[0]].partIndex - g_partQuatArray[partArrayIndices[1]].partIndex;
    else
        return diff;
}

int __cdecl XAnimIsHighPrecisionPart(const char *name)
{
    int v2; // [esp+4h] [ebp-20h]
    unsigned int low; // [esp+18h] [ebp-Ch]
    unsigned int high; // [esp+1Ch] [ebp-8h]

    low = 0;
    high = 18;
    while (low < high)
    {
        v2 = strcmp(name, g_highPrecisionParts[(high + low) >> 1]);
        if (v2 >= 0)
        {
            if (v2 <= 0)
                return 1;
            low = ((high + low) >> 1) + 1;
        }
        else
        {
            high = (high + low) >> 1;
        }
    }
    return 0;
}

int __cdecl XAnimTempAlloc(unsigned int size)
{
    if (!g_animUser)
        MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 382, 0, "%s", "g_animUser");
    return Hunk_UserAlloc(g_animUser, size, 4);
}

XModelPieces *__cdecl XModelPiecesLoadFile(const char *name, void *(__cdecl *Alloc)(int))
{
    char v3; // [esp+3h] [ebp-D9h]
    char *v4; // [esp+8h] [ebp-D4h]
    char *v5; // [esp+Ch] [ebp-D0h]
    float v6; // [esp+20h] [ebp-BCh]
    float v7; // [esp+24h] [ebp-B8h]
    float v8; // [esp+28h] [ebp-B4h]
    __int16 v9; // [esp+30h] [ebp-ACh]
    XModelPiece *piece; // [esp+38h] [ebp-A4h]
    int pieceIndex; // [esp+3Ch] [ebp-A0h]
    const unsigned __int8 *pos; // [esp+40h] [ebp-9Ch]
    const unsigned __int8 *posa; // [esp+40h] [ebp-9Ch]
    const unsigned __int8 *posb; // [esp+40h] [ebp-9Ch]
    const unsigned __int8 *posc; // [esp+40h] [ebp-9Ch]
    char filename[64]; // [esp+44h] [ebp-98h] BYREF
    char piecename[64]; // [esp+84h] [ebp-58h] BYREF
    unsigned __int8 *buf; // [esp+C8h] [ebp-14h] BYREF
    int version; // [esp+CCh] [ebp-10h]
    XModelPieces *xmodelPieces; // [esp+D0h] [ebp-Ch]
    int len; // [esp+D4h] [ebp-8h]
    int fileSize; // [esp+D8h] [ebp-4h]

    if (Com_sprintf(filename, 0x40u, "xmodelpieces/%s", name) >= 0)
    {
        fileSize = FS_ReadFile(filename, (void **)&buf);
        if (fileSize >= 0)
        {
            if (fileSize)
            {
                if (!buf)
                    MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 1118, 0, "%s", "buf");
                pos = buf + 2;
                version = *(__int16 *)buf;
                if (version == 1)
                {
                    xmodelPieces = (XModelPieces *)Alloc(12);
                    if (!xmodelPieces)
                        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 1130, 0, "%s", "xmodelPieces");
                    v9 = *(_WORD *)pos;
                    posa = pos + 2;
                    xmodelPieces->numpieces = v9;
                    xmodelPieces->pieces = (XModelPiece *)Alloc(16 * xmodelPieces->numpieces);
                    for (pieceIndex = 0; pieceIndex < xmodelPieces->numpieces; ++pieceIndex)
                    {
                        piece = &xmodelPieces->pieces[pieceIndex];
                        len = strlen((const char *)posa) + 1;
                        if (len > 64)
                        {
                            Com_PrintError(19, "ERROR: piecename '%s' too long\n", (const char *)posa);
                            FS_FreeFile((char *)buf);
                            return 0;
                        }
                        v5 = (char *)posa;
                        v4 = piecename;
                        do
                        {
                            v3 = *v5;
                            *v4++ = *v5++;
                        } while (v3);
                        posb = &posa[len];
                        piece->model = R_RegisterModel(piecename);
                        if (!piece->model)
                        {
                            Com_PrintError(1, "ERROR: xmodel piece '%s' missing from pieces model 's%'\n", piecename, filename);
                            FS_FreeFile((char *)buf);
                            return 0;
                        }
                        v8 = *(float *)posb;
                        posc = posb + 4;
                        piece->offset[0] = v8;
                        v7 = *(float *)posc;
                        posc += 4;
                        piece->offset[1] = v7;
                        v6 = *(float *)posc;
                        posa = posc + 4;
                        piece->offset[2] = v6;
                    }
                    FS_FreeFile((char *)buf);
                    return xmodelPieces;
                }
                else
                {
                    FS_FreeFile((char *)buf);
                    Com_PrintError(19, "ERROR: xmodelpieces '%s' out of date (version %d, expecting %d).\n", name, version, 1);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: xmodelpieces '%s' has 0 length\n", name);
                FS_FreeFile((char *)buf);
                return 0;
            }
        }
        else
        {
            if (buf)
                MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 1106, 0, "%s", "!buf");
            Com_PrintError(19, "ERROR: xmodelpieces '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", filename);
        return 0;
    }
}

XModelPieces *__cdecl XModelPiecesPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    XModelPieces *xmodelPieces; // [esp+0h] [ebp-4h]
    XModelPieces *xmodelPiecesa; // [esp+0h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 1177, 0, "%s", "name");
    if (!*name)
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 1178, 0, "%s", "name[0]");
    xmodelPieces = (XModelPieces *)Hunk_FindDataForFile(8, name);
    if (xmodelPieces)
        return xmodelPieces;
    xmodelPiecesa = XModelPiecesLoadFile(name, Alloc);
    if (xmodelPiecesa)
    {
        xmodelPiecesa->name = Hunk_SetDataForFile(8, name, xmodelPiecesa, Alloc);
        return xmodelPiecesa;
    }
    else
    {
        Com_PrintError(20, "ERROR: Cannot find xmodel pieces '%s'.\n", name);
        return 0;
    }
}

unsigned __int8 *__cdecl LoadTrans(
    void *(__cdecl *Alloc)(int),
    unsigned __int8 *pos,
    unsigned __int16 numTransIndices,
    XAnimPartTrans *trans)
{
    float v5; // [esp+8h] [ebp-1Ch]
    float v6; // [esp+Ch] [ebp-18h]
    float v7; // [esp+10h] [ebp-14h]
    float v8; // [esp+14h] [ebp-10h]
    float v9; // [esp+18h] [ebp-Ch]
    float v10; // [esp+1Ch] [ebp-8h]
    int size; // [esp+20h] [ebp-4h]
    unsigned __int8 *posb; // [esp+30h] [ebp+Ch]
    unsigned __int8 *posa; // [esp+30h] [ebp+Ch]

    trans->smallTrans = *pos;
    posb = pos + 1;
    v10 = *posb;
    posb += 4;
    trans->u.frames.mins[0] = v10;
    v9 = *posb;
    posb += 4;
    trans->u.frames.mins[1] = v9;
    v8 = *posb;
    posb += 4;
    trans->u.frames.mins[2] = v8;
    v7 = *posb;
    posb += 4;
    trans->u.frames.size[0] = v7;
    v6 = *posb;
    posb += 4;
    trans->u.frames.size[1] = v6;
    v5 = *posb;
    posa = (posb + 4);
    trans->u.frames.size[2] = v5;
    if (trans->smallTrans)
        Vec3Scale(trans->u.frames.size, 0.0039215689, trans->u.frames.size);
    else
        Vec3Scale(trans->u.frames.size, 0.000015259022, trans->u.frames.size);
    trans->size = numTransIndices - 1;
    if (trans->smallTrans)
        size = 3 * numTransIndices;
    else
        size = 6 * numTransIndices;
    trans->u.frames.frames._1 = (unsigned __int8 (*)[3])Alloc(size);
    memcpy(trans->u.frames.frames._1, posa, size);
    return &posa[size];
}

void __cdecl ConsumeQuat(const unsigned __int8 **pos, __int16 *out)
{
    int v2; // [esp+8h] [ebp-34h]
    float v3; // [esp+Ch] [ebp-30h]
    float v4; // [esp+10h] [ebp-2Ch]
    float v5; // [esp+14h] [ebp-28h]
    __int16 v6; // [esp+1Ch] [ebp-20h]
    __int16 v7; // [esp+20h] [ebp-1Ch]
    __int16 v8; // [esp+24h] [ebp-18h]
    int temp; // [esp+38h] [ebp-4h]

    v8 = **pos;
    *pos += 2;
    *out = v8;
    v7 = **pos;
    *pos += 2;
    out[1] = v7;
    v6 = **pos;
    *pos += 2;
    out[2] = v6;
    temp = 1073676289 - (out[2] * out[2] + out[1] * out[1] + *out * *out);
    if (temp <= 0)
    {
        v2 = 0;
    }
    else
    {
        v5 = sqrt(temp);
        v4 = v5 + 0.5;
        v3 = floor(v4);
        v2 = v3;
    }
    if (v2 != v2)
        MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 112, 0, "%s", "iQ[3] == (short)iQ[3]");
    out[3] = v2;
}

void __cdecl ConsumeQuat2(const unsigned __int8 **pos, __int16 *out)
{
    int v2; // [esp+8h] [ebp-24h]
    float v3; // [esp+Ch] [ebp-20h]
    float v4; // [esp+10h] [ebp-1Ch]
    float v5; // [esp+14h] [ebp-18h]
    __int16 v6; // [esp+1Ch] [ebp-10h]
    int temp; // [esp+28h] [ebp-4h]

    v6 = **pos;
    *pos += 2;
    *out = v6;
    temp = 1073676289 - *out * *out;
    if (temp <= 0)
    {
        v2 = 0;
    }
    else
    {
        v5 = sqrt(temp);
        v4 = v5 + 0.5;
        v3 = floor(v4);
        v2 = v3;
    }
    if (v2 != v2)
        MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 130, 0, "%s", "iQ[1] == (short)iQ[1]");
    out[1] = v2;
}

void __cdecl ConsumeQuatNoSwap(const unsigned __int8 **pos, __int16 *out)
{
    int v2; // [esp+8h] [ebp-34h]
    float v3; // [esp+Ch] [ebp-30h]
    float v4; // [esp+10h] [ebp-2Ch]
    float v5; // [esp+14h] [ebp-28h]
    __int16 v6; // [esp+1Ch] [ebp-20h]
    __int16 v7; // [esp+20h] [ebp-1Ch]
    __int16 v8; // [esp+24h] [ebp-18h]
    int temp; // [esp+38h] [ebp-4h]

    v8 = **pos;
    *pos += 2;
    *out = v8;
    v7 = **pos;
    *pos += 2;
    out[1] = v7;
    v6 = **pos;
    *pos += 2;
    out[2] = v6;
    temp = 1073676289 - (out[2] * out[2] + out[1] * out[1] + *out * *out);
    if (temp <= 0)
    {
        v2 = 0;
    }
    else
    {
        v5 = sqrt(temp);
        v4 = v5 + 0.5;
        v3 = floor(v4);
        v2 = v3;
    }
    if (v2 != v2)
        MyAssertHandler(".\\xanim\\xmodel_load_obj.cpp", 60, 0, "%s", "iQ[3] == (short)iQ[3]");
    out[3] = v2;
}

unsigned __int8 *__cdecl GetDeltaQuaternions(
    XAnimDeltaPart *deltaPart,
    void *(__cdecl *Alloc)(int),
    unsigned __int8 *pos,
    unsigned __int16 numloopframes,
    bool useSmallIndices)
{
    unsigned __int16 v6; // [esp+0h] [ebp-34h]
    __int16 *sQ; // [esp+4h] [ebp-30h]
    int m; // [esp+10h] [ebp-24h]
    int k; // [esp+14h] [ebp-20h]
    int j; // [esp+18h] [ebp-1Ch]
    int i; // [esp+1Ch] [ebp-18h]
    __int16 quat[4]; // [esp+20h] [ebp-14h] BYREF
    int size; // [esp+2Ch] [ebp-8h]
    unsigned __int16 numQuatIndices; // [esp+30h] [ebp-4h]

    v6 = *pos;
    pos += 2;
    numQuatIndices = v6;
    if (v6)
    {
        if (numQuatIndices == 1)
        {
            ConsumeQuat2(&pos, quat);
            deltaPart->quat = (XAnimDeltaPartQuat *)Alloc(8);
            deltaPart->quat->size = 0;
            deltaPart->quat->u.frame0[0] = quat[0];
            deltaPart->quat->u.frame0[1] = quat[1];
        }
        else
        {
            if (numQuatIndices > numloopframes)
                MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 192, 0, "%s", "numQuatIndices <= numloopframes");
            if (useSmallIndices)
            {
                deltaPart->quat = (XAnimDeltaPartQuat *)Alloc(numQuatIndices + 8);
                if (numQuatIndices >= numloopframes)
                {
                    for (i = 0; i < numQuatIndices; ++i)
                        deltaPart->quat->u.frames.indices._1[i] = i;
                }
                else
                {
                    size = numQuatIndices;
                    memcpy(deltaPart->quat->u.frames.indices._1, pos, numQuatIndices);
                    pos += numQuatIndices;
                }
            }
            else
            {
                deltaPart->quat = (XAnimDeltaPartQuat *)Alloc(2 * numQuatIndices + 8);
                if (numQuatIndices >= numloopframes)
                {
                    for (j = 0; j < numQuatIndices; ++j)
                        deltaPart->quat->u.frames.indices._2[j] = j;
                }
                else
                {
                    size = 2 * numQuatIndices;
                    memcpy(deltaPart->quat->u.frames.indices._1, pos, size);
                    pos += size;
                }
            }
            deltaPart->quat->size = numQuatIndices - 1;
            deltaPart->quat->u.frames.frames = (__int16 (*)[2])Alloc(4 * numQuatIndices);
            for (k = 0; k < numQuatIndices; ++k)
            {
                ConsumeQuat2(&pos, quat);
                deltaPart->quat->u.frames.frames[k][0] = quat[0];
                deltaPart->quat->u.frames.frames[k][1] = quat[1];
            }
            for (m = 1; m < numQuatIndices; ++m)
            {
                sQ = deltaPart->quat->u.frames.frames[m];
                if (*(sQ - 1) * sQ[1] + *(sQ - 2) * *sQ < 0)
                {
                    *sQ = -*sQ;
                    sQ[1] = -sQ[1];
                }
            }
        }
    }
    else
    {
        deltaPart->quat = 0;
    }
    return pos;
}

unsigned __int8 *__cdecl GetDeltaTranslations(
    XAnimDeltaPart *deltaPart,
    void *(__cdecl *Alloc)(int),
    unsigned __int8 *pos,
    unsigned __int16 numloopframes,
    bool useSmallIndices)
{
    XAnimPartTransData *p_u; // edx
    float v7; // [esp+4h] [ebp-2Ch]
    float v8; // [esp+8h] [ebp-28h]
    float v9; // [esp+Ch] [ebp-24h]
    unsigned __int16 v10; // [esp+10h] [ebp-20h]
    int j; // [esp+14h] [ebp-1Ch]
    int i; // [esp+18h] [ebp-18h]
    unsigned __int8 *posa; // [esp+40h] [ebp+10h]
    unsigned __int8 *posb; // [esp+40h] [ebp+10h]

    v10 = *pos;
    posa = (pos + 2);
    if (v10)
    {
        if (v10 == 1)
        {
            v9 = *posa;
            posb = posa + 4;
            v8 = *posb;
            posb += 4;
            v7 = *posb;
            posa = (posb + 4);
            deltaPart->trans = (XAnimPartTrans*)Alloc(16);
            deltaPart->trans->size = 0;
            p_u = &deltaPart->trans->u;
            p_u->frames.mins[0] = v9;
            p_u->frames.mins[1] = v8;
            p_u->frames.mins[2] = v7;
        }
        else
        {
            if (v10 > numloopframes)
                MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 333, 0, "%s", "numTransIndices <= numloopframes");
            if (useSmallIndices)
            {
                deltaPart->trans = (XAnimPartTrans *)Alloc(v10 + 32);
                if (v10 >= numloopframes)
                {
                    for (i = 0; i < v10; ++i)
                        deltaPart->trans->u.frames.indices._1[i] = i;
                }
                else
                {
                    memcpy(deltaPart->trans->u.frames.indices._1, posa, v10);
                    posa += v10;
                }
            }
            else
            {
                deltaPart->trans = (XAnimPartTrans *)Alloc(2 * v10 + 32);
                if (v10 >= numloopframes)
                {
                    for (j = 0; j < v10; ++j)
                        deltaPart->trans->u.frames.indices._2[j] = j;
                }
                else
                {
                    memcpy(deltaPart->trans->u.frames.indices._1, posa, 2 * v10);
                    posa += 2 * v10;
                }
            }
            return LoadTrans(Alloc, posa, v10, deltaPart->trans);
        }
    }
    else
    {
        deltaPart->trans = 0;
    }
    return posa;
}

unsigned __int8 *__cdecl GetQuaternions(
    XAnimPartQuatPtr *part,
    unsigned __int8 *pos,
    bool bFlipQuat,
    bool bSimpleQuat,
    unsigned __int16 numloopframes,
    bool useSmallIndices)
{
    unsigned __int16 v7; // [esp+0h] [ebp-48h]
    __int16 *v8; // [esp+4h] [ebp-44h]
    __int16 *v9; // [esp+8h] [ebp-40h]
    int ii; // [esp+10h] [ebp-38h]
    int n; // [esp+14h] [ebp-34h]
    __int16 *sQ; // [esp+18h] [ebp-30h]
    int m; // [esp+24h] [ebp-24h]
    int k; // [esp+28h] [ebp-20h]
    int j; // [esp+2Ch] [ebp-1Ch]
    int i; // [esp+30h] [ebp-18h]
    __int16 quat[4]; // [esp+34h] [ebp-14h] BYREF
    int size; // [esp+40h] [ebp-8h]
    unsigned __int16 numQuatIndices; // [esp+44h] [ebp-4h]

    v7 = *pos;
    pos += 2;
    numQuatIndices = v7;
    if (v7)
    {
        if (numQuatIndices == 1)
        {
            if (bSimpleQuat)
            {
                ConsumeQuat2(&pos, quat);
                if (bFlipQuat)
                {
                    quat[0] = -quat[0];
                    quat[1] = -quat[1];
                }
                part->quat = (XAnimPartQuat*)XAnimTempAlloc(8u);
                part->quat->u.frame0[0] = quat[0];
                part->quat->u.frame0[1] = quat[1];
            }
            else
            {
                ConsumeQuat(&pos, quat);
                if (bFlipQuat)
                {
                    quat[0] = -quat[0];
                    quat[1] = -quat[1];
                    quat[2] = -quat[2];
                    quat[3] = -quat[3];
                }
                part->quat = (XAnimPartQuat *)XAnimTempAlloc(0xCu);
                //part->quat->u = *quat;
                part->quat->u.frame0[0] = quat[0];
                part->quat->u.frame0[1] = quat[1];
                part->quat->u.frame0[2] = quat[2];
                part->quat->u.frame0[3] = quat[3];
            }
            part->quat->size = 0;
        }
        else
        {
            if (numQuatIndices > numloopframes)
                MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 438, 0, "%s", "numQuatIndices <= numloopframes");
            if (useSmallIndices)
            {
                part->quat = (XAnimPartQuat*)XAnimTempAlloc(numQuatIndices + 8);
                if (numQuatIndices >= numloopframes)
                {
                    for (i = 0; i < numQuatIndices; ++i)
                        part->quat->u.frames.indices._1[i] = i;
                }
                else
                {
                    size = numQuatIndices;
                    memcpy(part->quat->u.frames.indices._1, pos, numQuatIndices);
                    pos += numQuatIndices;
                }
            }
            else
            {
                part->quat = (XAnimPartQuat*)XAnimTempAlloc(2 * numQuatIndices + 8);
                if (numQuatIndices >= numloopframes)
                {
                    for (j = 0; j < numQuatIndices; ++j)
                        part->quat->u.frames.indices._2[j] = j;
                }
                else
                {
                    size = 2 * numQuatIndices;
                    memcpy(part->quat->u.frames.indices._1, pos, size);
                    pos += size;
                }
            }
            if (bSimpleQuat)
            {
                part->quat->u.frames.u.frames = (__int16 (*)[4])XAnimTempAlloc(4 * numQuatIndices);
                ConsumeQuat2(&pos, quat);
                if (bFlipQuat)
                {
                    quat[0] = -quat[0];
                    quat[1] = -quat[1];
                }
                (*part->quat->u.frames.u.frames)[0] = quat[0];
                (*part->quat->u.frames.u.frames)[1] = quat[1];
                for (k = 1; k < numQuatIndices; ++k)
                {
                    ConsumeQuat2(&pos, quat);
                    (*part->quat->u.frames.u.frames)[2 * k] = quat[0];
                    (*part->quat->u.frames.u.frames)[2 * k + 1] = quat[1];
                }
                for (m = 1; m < numQuatIndices; ++m)
                {
                    sQ = &(*part->quat->u.frames.u.frames)[2 * m];
                    if (*(sQ - 1) * sQ[1] + *(sQ - 2) * *sQ < 0)
                    {
                        *sQ = -*sQ;
                        sQ[1] = -sQ[1];
                    }
                }
            }
            else
            {
                part->quat->u.frames.u.frames = (__int16 (*)[4])XAnimTempAlloc(8 * numQuatIndices);
                ConsumeQuat(&pos, quat);
                if (bFlipQuat)
                {
                    quat[0] = -quat[0];
                    quat[1] = -quat[1];
                    quat[2] = -quat[2];
                    quat[3] = -quat[3];
                }
                *&(*part->quat->u.frames.u.frames)[0] = *quat;
                for (n = 1; n < numQuatIndices; ++n)
                {
                    ConsumeQuat(&pos, quat);
                    part->quat->u.frames.u.frames[n][0] = quat[0];
                    part->quat->u.frames.u.frames[n][1] = quat[1];
                    part->quat->u.frames.u.frames[n][2] = quat[2];
                    part->quat->u.frames.u.frames[n][3] = quat[3];
                }
                for (ii = 1; ii < numQuatIndices; ++ii)
                {
                    v8 = part->quat->u.frames.u.frames[ii];
                    v9 = v8 - 4;
                    if (v9[3] * v8[3] + v9[2] * v8[2] + v9[1] * v8[1] + *v9 * *v8 < 0)
                    {
                        *v8 = -*v8;
                        v8[1] = -v8[1];
                        v8[2] = -v8[2];
                        v8[3] = -v8[3];
                    }
                }
            }
            part->quat->size = numQuatIndices - 1;
        }
    }
    else
    {
        if (!bSimpleQuat)
            MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 396, 0, "%s", "bSimpleQuat");
        part->quat = 0;
    }
    return pos;
}

unsigned __int8 *__cdecl GetTranslations(
    XAnimPartTransPtr *part,
    unsigned __int8 *pos,
    unsigned __int16 numloopframes,
    bool useSmallIndices)
{
    XAnimPartTransData *p_u; // edx
    float v6; // [esp+4h] [ebp-2Ch]
    float v7; // [esp+8h] [ebp-28h]
    float v8; // [esp+Ch] [ebp-24h]
    unsigned __int16 v9; // [esp+10h] [ebp-20h]
    int j; // [esp+14h] [ebp-1Ch]
    int i; // [esp+18h] [ebp-18h]
    unsigned __int8 *posa; // [esp+3Ch] [ebp+Ch]
    unsigned __int8 *posb; // [esp+3Ch] [ebp+Ch]

    v9 = *pos;
    posa = (pos + 2);
    if (v9)
    {
        if (v9 == 1)
        {
            v8 = *posa;
            posb = posa + 4;
            v7 = *posb;
            posb += 4;
            v6 = *posb;
            posa = (posb + 4);
            part->trans = (XAnimPartTrans*)XAnimTempAlloc(0x10u);
            part->trans->size = 0;
            p_u = &part->trans->u;
            p_u->frames.mins[0] = v8;
            p_u->frames.mins[1] = v7;
            p_u->frames.mins[2] = v6;
        }
        else
        {
            if (v9 > numloopframes)
                MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 587, 0, "%s", "numTransIndices <= numloopframes");
            if (useSmallIndices)
            {
                part->trans = (XAnimPartTrans *)XAnimTempAlloc(v9 + 32);
                if (v9 >= numloopframes)
                {
                    for (i = 0; i < v9; ++i)
                        part->trans->u.frames.indices._1[i] = i;
                }
                else
                {
                    memcpy(part->trans->u.frames.indices._1, posa, v9);
                    posa += v9;
                }
            }
            else
            {
                part->trans = (XAnimPartTrans *)XAnimTempAlloc(2 * v9 + 32);
                if (v9 >= numloopframes)
                {
                    for (j = 0; j < v9; ++j)
                        part->trans->u.frames.indices._2[j] = j;
                }
                else
                {
                    memcpy(part->trans->u.frames.indices._1, posa, 2 * v9);
                    posa += 2 * v9;
                }
            }
            return LoadTrans((void*(*)(int))XAnimTempAlloc, posa, v9, part->trans);
        }
    }
    else
    {
        part->trans = 0;
    }
    return posa;
}

void __cdecl ReadNoteTracks(const char *name, unsigned __int16 **pos, XAnimParts *parts, void *(__cdecl *Alloc)(int))
{
    const char *v4; // eax
    double v5; // [esp+8h] [ebp-30h]
    unsigned __int16 v6; // [esp+24h] [ebp-14h]
    int numNoteTracks; // [esp+28h] [ebp-10h]
    XAnimNotifyInfo *notify; // [esp+30h] [ebp-8h]
    int i; // [esp+34h] [ebp-4h]

    numNoteTracks = **pos;
    *pos = (*pos + 1);
    if (numNoteTracks >= 255)
        MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 146, 0, "%s", "numNoteTracks < 255");
    parts->notifyCount = numNoteTracks + 1;
    notify = (XAnimNotifyInfo*)Alloc(8 * parts->notifyCount);
    parts->notify = notify;
    for (i = 0; i < numNoteTracks; ++i)
    {
        notify->name = SL_GetString_((const char*)*pos, 0, 3);
        *pos = (*pos + strlen((const char*)*pos) + 1);
        v6 = *(*pos)++;
        if (parts->numframes)
            v5 = v6 / parts->numframes;
        else
            v5 = 0.0;
        notify->time = v5;
        if (notify->time < 0.0 || notify->time > 1.0)
        {
            v4 = va("xanim: '%s', time: %f", name, notify->time);
            MyAssertHandler(
                ".\\xanim\\xanim_load_obj.cpp",
                162,
                0,
                "%s\n\t%s",
                "notify->time >= 0.0f && notify->time <= 1.0f",
                v4);
        }
        ++notify;
    }
    notify->name = SL_GetString_("end", 0, 3);
    notify->time = 1.0;
}

int g_checkBoneOrder;
XAnimParts *__cdecl XAnimLoadFile(char *name, void *(__cdecl *Alloc)(int))
{
    const char *v2; // eax
    __int16 prev; // ax
    unsigned __int8 *Quaternions; // eax
    int v6; // eax
    unsigned int v7; // ecx
    int v8; // eax
    unsigned int v9; // ecx
    int v10; // ecx
    __int16 *v11; // edx
    void *v12; // [esp+8h] [ebp-14DCh]
    void *v13; // [esp+Ch] [ebp-14D8h]
    int *v14; // [esp+10h] [ebp-14D4h]
    unsigned __int8 *v15; // [esp+14h] [ebp-14D0h]
    unsigned __int8 *v16; // [esp+18h] [ebp-14CCh]
    int *v17; // [esp+1Ch] [ebp-14C8h]
    __int16 *v18; // [esp+20h] [ebp-14C4h]
    unsigned __int8 *v19; // [esp+24h] [ebp-14C0h]
    int v20; // [esp+28h] [ebp-14BCh]
    int v21; // [esp+2Ch] [ebp-14B8h]
    int v22; // [esp+30h] [ebp-14B4h]
    int v23; // [esp+34h] [ebp-14B0h]
    int v24; // [esp+38h] [ebp-14ACh]
    int v25; // [esp+3Ch] [ebp-14A8h]
    int v26; // [esp+40h] [ebp-14A4h]
    BOOL v27; // [esp+44h] [ebp-14A0h]
    double v28; // [esp+58h] [ebp-148Ch]
    unsigned __int16 v29; // [esp+66h] [ebp-147Eh]
    int v30; // [esp+B0h] [ebp-1434h]
    int v31; // [esp+B4h] [ebp-1430h]
    int v32; // [esp+B8h] [ebp-142Ch]
    int v33; // [esp+BCh] [ebp-1428h]
    unsigned __int8 v34; // [esp+C0h] [ebp-1424h]
    unsigned __int16 v35; // [esp+C4h] [ebp-1420h]
    __int16 v36; // [esp+C8h] [ebp-141Ch]
    unsigned __int16 v37; // [esp+CCh] [ebp-1418h]
    __int16 v38; // [esp+D0h] [ebp-1414h]
    unsigned __int8 *pos; // [esp+DCh] [ebp-1408h] BYREF
    bool v40; // [esp+E3h] [ebp-1401h]
    unsigned __int8 dst[16]; // [esp+E4h] [ebp-1400h] BYREF
    unsigned int i; // [esp+F4h] [ebp-13F0h]
    unsigned int v43; // [esp+F8h] [ebp-13ECh]
    char dest[1092]; // [esp+FCh] [ebp-13E8h] BYREF
    __int16 *dataShort; // [esp+540h] [ebp-FA4h] BYREF
    unsigned int animPartIndex; // [esp+544h] [ebp-FA0h]
    unsigned int v47; // [esp+548h] [ebp-F9Ch]
    unsigned __int8 *dataByte; // [esp+54Ch] [ebp-F98h] BYREF
    int *v49; // [esp+550h] [ebp-F94h]
    int v50; // [esp+554h] [ebp-F90h]
    int v51; // [esp+558h] [ebp-F8Ch]
    int v52; // [esp+55Ch] [ebp-F88h]
    int v53; // [esp+560h] [ebp-F84h]
    int v54; // [esp+564h] [ebp-F80h]
    int v55; // [esp+568h] [ebp-F7Ch]
    int v56; // [esp+56Ch] [ebp-F78h]
    int v57; // [esp+570h] [ebp-F74h]
    int v58; // [esp+574h] [ebp-F70h]
    int v59; // [esp+578h] [ebp-F6Ch]
    int v60; // [esp+57Ch] [ebp-F68h]
    __int16 v61; // [esp+580h] [ebp-F64h]
    unsigned __int16 v62; // [esp+584h] [ebp-F60h]
    unsigned char*buffer; // [esp+588h] [ebp-F5Ch] BYREF
    unsigned int count; // [esp+58Ch] [ebp-F58h]
    unsigned int v65; // [esp+590h] [ebp-F54h]
    int *v66; // [esp+594h] [ebp-F50h]
    int v67; // [esp+598h] [ebp-F4Ch]
    _WORD base[130]; // [esp+59Ch] [ebp-F48h] BYREF
    unsigned int v69; // [esp+6A0h] [ebp-E44h]
    unsigned __int8 *v70; // [esp+6A4h] [ebp-E40h]
    int v71; // [esp+6A8h] [ebp-E3Ch]
    unsigned __int8 *v72; // [esp+6ACh] [ebp-E38h]
    unsigned __int8 *v73; // [esp+6B0h] [ebp-E34h]
    unsigned int tableSize; // [esp+6B4h] [ebp-E30h]
    XAnimIndices indices; // [esp+6B8h] [ebp-E2Ch] BYREF
    XAnimPartQuatPtr part[128]; // [esp+6BCh] [ebp-E28h] BYREF
    _WORD v77[128]; // [esp+ABCh] [ebp-A28h]
    BOOL v78; // [esp+BBCh] [ebp-928h]
    unsigned int PartQuatType; // [esp+BC0h] [ebp-924h]
    _DWORD v80[256]; // [esp+BC4h] [ebp-920h]
    unsigned __int8 v81[2]; // [esp+FC4h] [ebp-520h] BYREF
    XAnimPartTransPtr v82[128]; // [esp+10CCh] [ebp-418h] BYREF
    XAnimParts *parts; // [esp+14CCh] [ebp-18h]
    unsigned __int16 v84; // [esp+14D0h] [ebp-14h]
    char v85; // [esp+14D7h] [ebp-Dh]
    int v86; // [esp+14D8h] [ebp-Ch]
    int v87; // [esp+14DCh] [ebp-8h]
    int v88; // [esp+14E0h] [ebp-4h]

    if (!g_checkBoneOrder)
    {
        g_checkBoneOrder = 1;
        for (i = 0; i < 0x11; ++i)
        {
            if (strcmp(g_highPrecisionParts[i], g_highPrecisionParts[i + 1]) >= 0)
            {
                v2 = va("'%s' '%s'", g_highPrecisionParts[i], g_highPrecisionParts[i + 1]);
                MyAssertHandler(
                    ".\\xanim\\xanim_load_obj.cpp",
                    879,
                    0,
                    "%s\n\t%s",
                    "strcmp( g_highPrecisionParts[j], g_highPrecisionParts[j + 1] ) < 0",
                    v2);
            }
        }
    }
    if (Com_sprintf(dest, 0x40u, "xanim/%s", name) >= 0)
    {
        v67 = FS_ReadFile(dest, (void**)&buffer);
        if (v67 >= 0)
        {
            if (v67)
            {
                if (!buffer)
                    MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 905, 0, "%s", "buf");
                pos = buffer;
                v38 = *buffer;
                pos = buffer + 2;
                v61 = v38;
                if (v38 == 17)
                {
                    v78 = I_strnicmp(name, "viewmodel_", 10) == 0;
                    parts = (XAnimParts*)Alloc(88);
                    if (!parts)
                        MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 919, 0, "%s", "parts");
                    v37 = *pos;
                    pos += 2;
                    v62 = v37;
                    v36 = *pos;
                    pos += 2;
                    if (v36 > 0x80)
                        MyAssertHandler(
                            ".\\xanim\\xanim_load_obj.cpp",
                            924,
                            0,
                            "%s\n\t(numBones) = %i",
                            "(numBones <= DOBJ_MAX_PARTS)",
                            v36);
                    parts->boneCount[9] = v36;
                    v85 = *pos++;
                    parts->bLoop = (v85 & 1) != 0;
                    parts->bDelta = (v85 & 2) != 0;
                    parts->assetType = *pos++;
                    v35 = *pos;
                    pos += 2;
                    parts->framerate = v35;
                    if (!v62)
                        MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 938, 0, "%s", "numframes");
                    if (parts->bLoop)
                        v29 = v62 + 1;
                    else
                        v29 = v62;
                    v84 = v29;
                    v40 = v29 <= 0x100u;
                    parts->numframes = v29 - 1;
                    if (parts->numframes)
                        v28 = parts->framerate / parts->numframes;
                    else
                        v28 = 0.0;
                    parts->frequency = v28;
                    if (parts->frequency < 0.0)
                        MyAssertHandler(
                            ".\\xanim\\xanim_load_obj.cpp",
                            944,
                            0,
                            "%s\n\t(parts->frequency) = %g",
                            "(parts->frequency >= 0)",
                            parts->frequency);
                    if (parts->bDelta)
                    {
                        parts->deltaPart = (XAnimDeltaPart*)Alloc(8);
                        pos = GetDeltaQuaternions(parts->deltaPart, Alloc, pos, v84, v40);
                        pos = GetDeltaTranslations(parts->deltaPart, Alloc, pos, v84, v40);
                    }
                    if (v36)
                    {
                        count = ((v36 - 1) >> 3) + 1;
                        v72 = pos;
                        pos += count;
                        memcpy(dst, pos, count);
                        pos += count;
                        memset(part, 0, 8 * v36);
                        memset(v82, 0, 8 * v36);
                        for (i = 0; i < v36; ++i)
                        {
                            part[i].partIndex = i;
                            v82[i].partIndex = i;
                        }
                    }
                    else
                    {
                        v72 = 0;
                    }
                    for (i = 0; i < v36; ++i)
                    {
                        count = strlen((const char*)pos) + 1;
                        prev = SL_GetStringOfSize((const char*)pos, 0, count, 9).prev;
                        v77[i] = prev;
                        v27 = v78 || XAnimIsHighPrecisionPart((const char*)pos);
                        g_highPrecisionPart[i] = v27;
                        pos += count;
                    }
                    if (g_animUser)
                        MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 985, 0, "%s", "!g_animUser");
                    g_animUser = Hunk_UserCreate(0x20000, "XAnimLoadFile", 0, 0, 11);
                    for (i = 0; i < v36; ++i)
                    {
                        Quaternions = GetQuaternions(
                            &part[i],
                            pos,
                            ((1 << (i & 7)) & v72[i >> 3]) != 0,
                            ((1 << (i & 7)) & dst[i >> 3]) != 0,
                            v84,
                            v40);
                        pos = GetTranslations(&v82[i], Quaternions, v84, v40);
                    }
                    ReadNoteTracks(name, (unsigned short**)&pos, parts, Alloc);
                    if (buffer + v67 != pos)
                        MyAssertHandler(
                            ".\\xanim\\xanim_load_obj.cpp",
                            1000,
                            1,
                            "%s\n\t(name) = %s",
                            "(buf + fileSize == pos)",
                            name);
                    FS_FreeFile((char*)buffer);
                    if (v36)
                    {
                        for (i = 0; i < v36; ++i)
                            base[i] = i;
                        g_partQuatArray = part;
                        g_partTransArray = v82;
                        g_simpleQuatBits = (char*)dst;
                        parts->names = (unsigned short*)Alloc(2 * v36);
                        v51 = 0;
                        v52 = 0;
                        v53 = 0;
                        v54 = 0;
                        v55 = 0;
                        v56 = 0;
                        v57 = 0;
                        v58 = 0;
                        v59 = 0;
                        v60 = 0;
                        qsort(base, v36, 2u, XAnimCompareQuatParts);
                        for (i = 0; i < v36; ++i)
                        {
                            animPartIndex = base[i];
                            part[animPartIndex].partIndex = i;
                            v82[animPartIndex].partIndex = i;
                            v6 = *&part[animPartIndex].partIndex;
                            v7 = i;
                            v80[2 * i] = (uint32)part[animPartIndex].quat;
                            v80[2 * v7 + 1] = v6;
                            parts->names[i] = v77[animPartIndex];
                            PartQuatType = XAnimGetPartQuatType(animPartIndex);
                            *(&v51 + PartQuatType) = i + 1;
                        }
                        if (v52 < v51)
                            v26 = v51;
                        else
                            v26 = v52;
                        v52 = v26;
                        if (v53 < v26)
                            v25 = v52;
                        else
                            v25 = v53;
                        v53 = v25;
                        if (v54 < v25)
                            v24 = v53;
                        else
                            v24 = v54;
                        v54 = v24;
                        if (v55 < v24)
                            v23 = v54;
                        else
                            v23 = v55;
                        v55 = v23;
                        v34 = v51;
                        if (v51 != v51)
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                                281,
                                0,
                                "i == static_cast< Type >( i )\n\t%i, %i",
                                v51,
                                v51);
                        parts->boneCount[0] = v34;
                        v33 = v52 - v51;
                        if (v52 - v51 != (v52 - v51))
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                                281,
                                0,
                                "i == static_cast< Type >( i )\n\t%i, %i",
                                v33,
                                v33);
                        parts->boneCount[1] = v33;
                        v32 = v53 - v52;
                        if (v53 - v52 != (v53 - v52))
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                                281,
                                0,
                                "i == static_cast< Type >( i )\n\t%i, %i",
                                v32,
                                v32);
                        parts->boneCount[2] = v32;
                        v31 = v54 - v53;
                        if (v54 - v53 != (v54 - v53))
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                                281,
                                0,
                                "i == static_cast< Type >( i )\n\t%i, %i",
                                v31,
                                v31);
                        parts->boneCount[3] = v31;
                        v30 = v55 - v54;
                        if (v55 - v54 != (v55 - v54))
                            MyAssertHandler(
                                "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                                281,
                                0,
                                "i == static_cast< Type >( i )\n\t%i, %i",
                                v30,
                                v30);
                        parts->boneCount[4] = v30;
                        memcpy(v81, base, 2 * v36);
                        qsort(v81, v36, 2u, XAnimCompareTransParts);
                        for (i = 0; i < v36; ++i)
                        {
                            animPartIndex = *&v81[2 * i];
                            v8 = *&v82[animPartIndex].partIndex;
                            v9 = i;
                            *&dest[8 * i + 64] = (char)v82[animPartIndex].trans;
                            *&dest[8 * v9 + 68] = v8;
                            PartQuatType = XAnimGetPartTransType(animPartIndex);
                            *(&v51 + PartQuatType) = i + 1;
                        }
                        if (v57 < v56)
                            v22 = v56;
                        else
                            v22 = v57;
                        v57 = v22;
                        if (v58 < v22)
                            v21 = v57;
                        else
                            v21 = v58;
                        v58 = v21;
                        if (v59 < v21)
                            v20 = v58;
                        else
                            v20 = v59;
                        v59 = v20;
                        parts->boneCount[5] = v56;
                        parts->boneCount[6] = v57 - v56;
                        parts->boneCount[7] = v58 - v57;
                        parts->boneCount[8] = v59 - v58;
                        v47 = 0;
                        v86 = 0;
                        v50 = 0;
                        v87 = 0;
                        v69 = 0;
                        v88 = 0;
                        v71 = 0;
                        animPartIndex = 0;
                        v65 = parts->boneCount[0];
                        animPartIndex = parts->boneCount[0];
                        v65 += parts->boneCount[1];
                        while (animPartIndex < v65)
                        {
                            tableSize = *(unsigned int *)v80[2 * animPartIndex];
                            ++v86;
                            if (v40)
                            {
                                v47 += tableSize + 1;
                            }
                            else if (tableSize >= 0x40)
                            {
                                v86 += ((tableSize - 1) >> 8) + 1 + 1;
                                v71 += tableSize + 1;
                            }
                            else
                            {
                                v86 += tableSize + 1;
                            }
                            ++animPartIndex;
                            v69 += 2 * tableSize + 2;
                        }
                        v65 += parts->boneCount[2];
                        while (animPartIndex < v65)
                        {
                            tableSize = *(unsigned int *)v80[2 * animPartIndex];
                            ++v86;
                            if (v40)
                            {
                                v47 += tableSize + 1;
                            }
                            else if (tableSize >= 0x40)
                            {
                                v86 += ((tableSize - 1) >> 8) + 1 + 1;
                                v71 += tableSize + 1;
                            }
                            else
                            {
                                v86 += tableSize + 1;
                            }
                            ++animPartIndex;
                            v69 += 4 * tableSize + 4;
                        }
                        v65 += parts->boneCount[3];
                        animPartIndex += parts->boneCount[3];
                        v86 += 2 * parts->boneCount[3];
                        v65 += parts->boneCount[4];
                        animPartIndex += parts->boneCount[4];
                        v86 += 4 * parts->boneCount[4];
                        if (animPartIndex != parts->boneCount[9])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1151,
                                0,
                                "animPartIndex == parts->boneCount[PART_TYPE_ALL]\n\t%i, %i",
                                animPartIndex,
                                parts->boneCount[9]);
                        animPartIndex = 0;
                        v65 = parts->boneCount[5];
                        v47 += parts->boneCount[5];
                        v50 += 6 * parts->boneCount[5];
                        while (animPartIndex < v65)
                        {
                            tableSize = **(unsigned int **)&dest[8 * animPartIndex + 64];
                            ++v86;
                            if (v40)
                            {
                                v47 += tableSize + 1;
                            }
                            else if (tableSize >= 0x40)
                            {
                                v86 += ((tableSize - 1) >> 8) + 1 + 1;
                                v71 += tableSize + 1;
                            }
                            else
                            {
                                v86 += tableSize + 1;
                            }
                            ++animPartIndex;
                            v87 += 3 * (tableSize + 1);
                        }
                        v65 += parts->boneCount[6];
                        v47 += parts->boneCount[6];
                        v50 += 6 * parts->boneCount[6];
                        while (animPartIndex < v65)
                        {
                            tableSize = **(unsigned int **)&dest[8 * animPartIndex + 64];
                            ++v86;
                            if (v40)
                            {
                                v47 += tableSize + 1;
                            }
                            else if (tableSize >= 0x40)
                            {
                                v86 += ((tableSize - 1) >> 8) + 1 + 1;
                                v71 += tableSize + 1;
                            }
                            else
                            {
                                v86 += tableSize + 1;
                            }
                            ++animPartIndex;
                            v69 += 3 * (tableSize + 1);
                        }
                        v65 += parts->boneCount[7];
                        animPartIndex += parts->boneCount[7];
                        v47 += parts->boneCount[7];
                        v50 += 3 * parts->boneCount[7];
                        v65 += parts->boneCount[8];
                        animPartIndex += parts->boneCount[8];
                        v47 += parts->boneCount[8];
                        if (animPartIndex != parts->boneCount[9])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1184,
                                0,
                                "%s",
                                "animPartIndex == parts->boneCount[PART_TYPE_ALL]");
                        if (v47)
                            v19 = (unsigned char*)Alloc(2 * v47);
                        else
                            v19 = 0;
                        dataByte = v19;
                        parts->dataByteCount = v47;
                        if (parts->dataByteCount != v47)
                            MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 1189, 0, "%s", "parts->dataByteCount == dataByteCount");
                        parts->dataByte = dataByte;
                        if (v86)
                            v18 = (short*)Alloc(2 * v86);
                        else
                            v18 = 0;
                        dataShort = v18;
                        parts->dataShortCount = v86;
                        if (parts->dataShortCount != v86)
                            MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 1194, 0, "%s", "parts->dataShortCount == dataShortCount");
                        parts->dataShort = dataShort;
                        if (v50)
                            v17 = (int*)Alloc(4 * v50);
                        else
                            v17 = 0;
                        v66 = v17;
                        parts->dataIntCount = v50;
                        if (parts->dataIntCount != v50)
                            MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 1199, 0, "%s", "parts->dataIntCount == dataIntCount");
                        parts->dataInt = v66;
                        if (v87)
                            v16 = (unsigned char*)Alloc(2 * v87);
                        else
                            v16 = 0;
                        v73 = v16;
                        parts->randomDataByteCount = v87;
                        if (parts->randomDataByteCount != v87)
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1205,
                                0,
                                "%s",
                                "parts->randomDataByteCount == randomDataByteCount");
                        parts->randomDataByte = v73;
                        if (v69)
                            v15 = (unsigned char*)Alloc(2 * v69);
                        else
                            v15 = 0;
                        v70 = v15;
                        parts->randomDataShortCount = v69;
                        parts->randomDataShort = (short*)v70;
                        if (v88)
                            v14 = (int*)Alloc(4 * v88);
                        else
                            v14 = 0;
                        v49 = v14;
                        parts->randomDataIntCount = v88;
                        if (parts->randomDataIntCount != v88)
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1218,
                                0,
                                "%s",
                                "parts->randomDataIntCount == randomDataIntCount");
                        parts->randomDataInt = v49;
                        if (v40)
                        {
                            if (v71)
                                v13 = Alloc(v71);
                            else
                                v13 = 0;
                            indices._1 = (unsigned char*)v13;
                        }
                        else
                        {
                            if (v71)
                                v12 = Alloc(2 * v71);
                            else
                                v12 = 0;
                            indices._1 = (unsigned char *)v12;
                        }
                        parts->indices = indices;
                        parts->indexCount = v71;
                        animPartIndex = 0;
                        v65 = parts->boneCount[0];
                        animPartIndex = parts->boneCount[0];
                        v65 += parts->boneCount[1];
                        while (animPartIndex < v65)
                        {
                            tableSize = *(unsigned int*)v80[2 * animPartIndex];
                            if (tableSize != tableSize)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                                    281,
                                    0,
                                    "i == static_cast< Type >( i )\n\t%i, %i",
                                    tableSize,
                                    tableSize);
                            *dataShort++ = tableSize;
                            memcpy(v70, (void*)*(unsigned int*)(v80[2 * animPartIndex] + 4), 2 * (2 * tableSize + 2));
                            v70 += 4 * tableSize + 4;
                            XAnimEmitFrameIndices(tableSize, &dataByte, &dataShort, &indices, (v80[2 * animPartIndex++] + 8), v40);
                        }
                        v65 += parts->boneCount[2];
                        while (animPartIndex < v65)
                        {
                            tableSize = *(unsigned int *)v80[2 * animPartIndex];
                            if (tableSize != tableSize)
                                MyAssertHandler(
                                    "c:\\trees\\cod3\\src\\qcommon\\../universal/assertive.h",
                                    281,
                                    0,
                                    "i == static_cast< Type >( i )\n\t%i, %i",
                                    tableSize,
                                    tableSize);
                            *dataShort++ = tableSize;
                            memcpy(v70, (void*)*(unsigned int *)(v80[2 * animPartIndex] + 4), 2 * (4 * tableSize + 4));
                            v70 += 8 * tableSize + 8;
                            XAnimEmitFrameIndices(tableSize, &dataByte, &dataShort, &indices, (v80[2 * animPartIndex++] + 8), v40);
                        }
                        v65 += parts->boneCount[3];
                        while (animPartIndex < v65)
                        {
                            *dataShort = *(unsigned int *)(v80[2 * animPartIndex++] + 4);
                            dataShort += 2;
                        }
                        v65 += parts->boneCount[4];
                        while (animPartIndex < v65)
                        {
                            v10 = v80[2 * animPartIndex];
                            v11 = dataShort;
                            *dataShort = *(unsigned int *)(v10 + 4);
                            *(v11 + 1) = *(unsigned int *)(v10 + 8);
                            ++animPartIndex;
                            dataShort += 4;
                        }
                        if (animPartIndex != parts->boneCount[9])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1390,
                                0,
                                "animPartIndex == parts->boneCount[PART_TYPE_ALL]\n\t%i, %i",
                                animPartIndex,
                                parts->boneCount[9]);
                        animPartIndex = 0;
                        v65 = parts->boneCount[5];
                        while (animPartIndex < v65)
                        {
                            *dataByte++ = dest[8 * animPartIndex + 68];
                            *dataShort = **&dest[8 * animPartIndex + 64];
                            tableSize = *dataShort++;
                            *v66 = *(*&dest[8 * animPartIndex + 64] + 4);
                            v66[1] = *(*&dest[8 * animPartIndex + 64] + 8);
                            v66[2] = *(*&dest[8 * animPartIndex + 64] + 12);
                            v66[3] = *(*&dest[8 * animPartIndex + 64] + 16);
                            v66[4] = *(*&dest[8 * animPartIndex + 64] + 20);
                            v66[5] = *(*&dest[8 * animPartIndex + 64] + 24);
                            v43 = 0;
                            while (v43 <= tableSize)
                            {
                                *v73 = *(3 * v43 + *(*&dest[8 * animPartIndex + 64] + 28));
                                v73[1] = *(*(*&dest[8 * animPartIndex + 64] + 28) + 3 * v43 + 1);
                                v73[2] = *(*(*&dest[8 * animPartIndex + 64] + 28) + 3 * v43++ + 2);
                                v73 += 3;
                            }
                            XAnimEmitFrameIndices(
                                tableSize,
                                &dataByte,
                                &dataShort,
                                &indices,
                                (*&dest[8 * animPartIndex++ + 64] + 32),
                                v40);
                            v66 += 6;
                        }
                        v65 += parts->boneCount[6];
                        while (animPartIndex < v65)
                        {
                            *dataByte++ = dest[8 * animPartIndex + 68];
                            *dataShort = **&dest[8 * animPartIndex + 64];
                            tableSize = *dataShort++;
                            *v66 = *(*&dest[8 * animPartIndex + 64] + 4);
                            v66[1] = *(*&dest[8 * animPartIndex + 64] + 8);
                            v66[2] = *(*&dest[8 * animPartIndex + 64] + 12);
                            v66[3] = *(*&dest[8 * animPartIndex + 64] + 16);
                            v66[4] = *(*&dest[8 * animPartIndex + 64] + 20);
                            v66[5] = *(*&dest[8 * animPartIndex + 64] + 24);
                            v43 = 0;
                            while (v43 <= tableSize)
                            {
                                *v70 = *(6 * v43 + *(*&dest[8 * animPartIndex + 64] + 28));
                                *(v70 + 1) = *(*(*&dest[8 * animPartIndex + 64] + 28) + 6 * v43 + 2);
                                *(v70 + 2) = *(*(*&dest[8 * animPartIndex + 64] + 28) + 6 * v43++ + 4);
                                v70 += 6;
                            }
                            XAnimEmitFrameIndices(
                                tableSize,
                                &dataByte,
                                &dataShort,
                                &indices,
                                (*&dest[8 * animPartIndex++ + 64] + 32),
                                v40);
                            v66 += 6;
                        }
                        v65 += parts->boneCount[7];
                        while (animPartIndex < v65)
                        {
                            *dataByte = dest[8 * animPartIndex + 68];
                            *v66 = *(*&dest[8 * animPartIndex + 64] + 4);
                            v66[1] = *(*&dest[8 * animPartIndex + 64] + 8);
                            v66[2] = *(*&dest[8 * animPartIndex++ + 64] + 12);
                            ++dataByte;
                            v66 += 3;
                        }
                        v65 += parts->boneCount[8];
                        while (animPartIndex < v65)
                            *dataByte++ = dest[8 * animPartIndex++ + 68];
                        if (animPartIndex != parts->boneCount[9])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1470,
                                0,
                                "animPartIndex == parts->boneCount[PART_TYPE_ALL]\n\t%i, %i",
                                animPartIndex,
                                parts->boneCount[9]);
                        if (v40)
                        {
                            if (indices._1 != &parts->indices._1[v71])
                                MyAssertHandler(
                                    ".\\xanim\\xanim_load_obj.cpp",
                                    1474,
                                    0,
                                    "%s",
                                    "indices._1 == &parts->indices._1[indexCount]");
                        }
                        else if (indices._1 != &parts->indices._1[2 * v71])
                        {
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1476,
                                0,
                                "%s",
                                "indices._2 == &parts->indices._2[indexCount]");
                        }
                        if (dataByte != &parts->dataByte[v47])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1477,
                                0,
                                "%s",
                                "dataByte == &parts->dataByte[dataByteCount]");
                        if (dataShort != &parts->dataShort[v86])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1478,
                                0,
                                "%s",
                                "dataShort == &parts->dataShort[dataShortCount]");
                        if (v66 != &parts->dataInt[v50])
                            MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 1479, 0, "%s", "dataInt == &parts->dataInt[dataIntCount]");
                        if (v73 != &parts->randomDataByte[v87])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1480,
                                0,
                                "%s",
                                "randomDataByte == &parts->randomDataByte[randomDataByteCount]");
                        if (v70 != (unsigned char*) &parts->randomDataShort[v69])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1481,
                                0,
                                "%s",
                                "randomDataShort == &parts->randomDataShort[randomDataShortCount]");
                        if (v49 != &parts->randomDataInt[v88])
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1482,
                                0,
                                "%s",
                                "randomDataInt == &parts->randomDataInt[randomDataIntCount]");
                        if (v47 > 0x4000)
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1484,
                                0,
                                "%s\n\t(name) = %s",
                                "(dataByteCount * sizeof( byte ) <= 16 * 1024)",
                                name);
                        if ((2 * v86) > 0x4000)
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1485,
                                0,
                                "%s\n\t(name) = %s",
                                "(dataShortCount * sizeof( ushort ) <= 16 * 1024)",
                                name);
                        if ((4 * v50) > 0x4000)
                            MyAssertHandler(
                                ".\\xanim\\xanim_load_obj.cpp",
                                1486,
                                0,
                                "%s\n\t(name) = %s",
                                "(dataIntCount * sizeof( int ) <= 16 * 1024)",
                                name);
                    }
                    if (!g_animUser)
                        MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 1489, 0, "%s", "g_animUser");
                    Hunk_UserDestroy(g_animUser);
                    g_animUser = 0;
                    return parts;
                }
                else
                {
                    FS_FreeFile((char *)buffer);
                    Com_PrintError(19, "ERROR: xanim '%s' out of date (version %d, expecting %d)\n", name, v61, 17);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(19, "ERROR: xanim '%s' has 0 length\n", name);
                FS_FreeFile((char *)buffer);
                return 0;
            }
        }
        else
        {
            if (buffer)
                MyAssertHandler(".\\xanim\\xanim_load_obj.cpp", 893, 0, "%s", "!buf");
            Com_PrintError(19, "ERROR: xanim '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(19, "ERROR: filename '%s' too long\n", dest);
        return 0;
    }
}
