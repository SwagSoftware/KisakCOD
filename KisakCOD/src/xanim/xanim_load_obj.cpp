#include "xanim.h"
#include "xmodel.h"
#include <gfx_d3d/r_model.h>
#include <universal/com_files.h>
#include <qcommon/qcommon.h>

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