#include "phys_local.h"
#include <universal/com_memory.h>
#include <qcommon/qcommon.h>
#include <universal/com_files.h>

void *(__cdecl *physAlloc)(int);

cspField_t physPresetFields[10] =
{
  { "mass", 0, 6 },
  { "bounce", 4, 6 },
  { "friction", 8, 6 },
  { "isFrictionInfinity", 12, 5 },
  { "bulletForceScale", 16, 6 },
  { "explosiveForceScale", 20, 6 },
  { "sndAliasPrefix", 24, 0 },
  { "piecesSpreadFraction", 28, 6 },
  { "piecesUpwardVelocity", 32, 6 },
  { "tempDefaultToCylinder", 36, 5 }
}; // idb

void __cdecl PhysPreset_Strcpy(unsigned __int8 *member, const char *keyValue)
{
    char v2; // [esp+3h] [ebp-25h]
    char *v3; // [esp+8h] [ebp-20h]
    const char *v4; // [esp+Ch] [ebp-1Ch]
    char *buf; // [esp+20h] [ebp-8h]

    if (*keyValue)
    {
        buf = (char *)physAlloc(strlen(keyValue) + 1);
        v4 = keyValue;
        v3 = buf;
        do
        {
            v2 = *v4;
            *v3++ = *v4++;
        } while (v2);
        *(_DWORD *)member = (_DWORD)buf;
    }
    else
    {
        *(_DWORD *)member = (_DWORD)"";
    }
}

PhysPreset *__cdecl PhysPresetLoadFile(const char *name, void *(__cdecl *Alloc)(int))
{
    float v3; // [esp+0h] [ebp-20A4h]
    char dest[64]; // [esp+24h] [ebp-2080h] BYREF
    char buffer[8192]; // [esp+64h] [ebp-2040h] BYREF
    unsigned __int8 pStruct[4]; // [esp+2068h] [ebp-3Ch] BYREF
    float v7; // [esp+206Ch] [ebp-38h]
    float v8; // [esp+2070h] [ebp-34h]
    int v9; // [esp+2074h] [ebp-30h]
    float v10; // [esp+2078h] [ebp-2Ch]
    float v11; // [esp+207Ch] [ebp-28h]
    const char *v12; // [esp+2080h] [ebp-24h]
    float v13; // [esp+2084h] [ebp-20h]
    float v14; // [esp+2088h] [ebp-1Ch]
    int v15; // [esp+208Ch] [ebp-18h]
    float *v16; // [esp+2090h] [ebp-14h]
    char *last; // [esp+2094h] [ebp-10h]
    signed int v18; // [esp+2098h] [ebp-Ch]
    int f; // [esp+209Ch] [ebp-8h] BYREF
    int len; // [esp+20A0h] [ebp-4h]

    last = (char*)"PHYSIC";
    len = strlen("PHYSIC");
    if (!strlen(name))
        return 0;
    if (Com_sprintf(dest, 0x40u, "physic/%s", name) >= 0)
    {
        v18 = FS_FOpenFileByMode(dest, &f, FS_READ);
        if (v18 >= 0)
        {
            FS_Read((unsigned __int8 *)buffer, len, f);
            buffer[len] = 0;
            if (!strncmp(buffer, last, len))
            {
                if (v18 - len < 0x2000)
                {
                    FS_Read((unsigned __int8 *)buffer, v18 - len, f);
                    buffer[v18 - len] = 0;
                    FS_FCloseFile(f);
                    if (Info_Validate(buffer))
                    {
                        *(float *)pStruct = 0.0;
                        v7 = 0.0;
                        v8 = 0.0;
                        v9 = 0;
                        v10 = 0.0;
                        v11 = 0.0;
                        v13 = 0.0;
                        v14 = 0.0;
                        v15 = 0;
                        v12 = "";
                        physAlloc = Alloc;
                        if (ParseConfigStringToStruct(pStruct, physPresetFields, 10, buffer, 0, 0, PhysPreset_Strcpy))
                        {
                            v16 = (float *)Alloc(44);
                            if (!v16)
                                MyAssertHandler(".\\physics\\physpreset_load_obj.cpp", 126, 0, "%s", "physPreset");
                            v16[2] = *(float *)pStruct;
                            v16[3] = v7;
                            if (v9)
                                v3 = 3.4028235e38;
                            else
                                v3 = v8;
                            v16[4] = v3;
                            v16[5] = v10;
                            v16[6] = v11;
                            *((_DWORD *)v16 + 7) = (_DWORD)v12;
                            v16[8] = v13;
                            v16[9] = v14;
                            *((_BYTE *)v16 + 40) = v15;
                            return (PhysPreset *)v16;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        Com_PrintError(20, "ERROR: physics preset file [%s] is not valid\n", name);
                        return 0;
                    }
                }
                else
                {
                    Com_PrintError(20, "ERROR: physics preset file [%s] is to big\n", name);
                    FS_FCloseFile(f);
                    return 0;
                }
            }
            else
            {
                Com_PrintError(20, "ERROR: file [%s] is not a physics preset file\n", name);
                FS_FCloseFile(f);
                return 0;
            }
        }
        else
        {
            Com_PrintError(20, "ERROR: physics preset '%s' not found\n", name);
            return 0;
        }
    }
    else
    {
        Com_PrintError(20, "ERROR: filename '%s' too long\n", dest);
        return 0;
    }
}

PhysPreset *__cdecl PhysPresetPrecache(const char *name, void *(__cdecl *Alloc)(int))
{
    PhysPreset *physPreset; // [esp+0h] [ebp-4h]
    PhysPreset *physPreseta; // [esp+0h] [ebp-4h]

    if (!name)
        MyAssertHandler(".\\physics\\physpreset_load_obj.cpp", 146, 0, "%s", "name");
    if (!*name)
        MyAssertHandler(".\\physics\\physpreset_load_obj.cpp", 147, 0, "%s", "name[0]");
    physPreset = (PhysPreset *)Hunk_FindDataForFile(7, name);
    if (physPreset)
        return physPreset;
    physPreseta = PhysPresetLoadFile(name, Alloc);
    if (physPreseta)
    {
        physPreseta->name = Hunk_SetDataForFile(7, name, physPreseta, Alloc);
        return physPreseta;
    }
    else
    {
        Com_PrintError(20, "ERROR: Cannot find physics preset '%s'.\n", name);
        return 0;
    }
}


