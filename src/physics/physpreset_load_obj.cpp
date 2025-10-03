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

struct PhysPresetLite // LWSS add custom struct to adhere to the above field offfsets
{
    float mass;   // 0
    float bounce; // 4
    float friction; // 8
    int isFrictionInfinity; // 12
    float bulletForceScale; // 16
    float explosiveForceScale; // 20
    const char *sndAliasPrefix; // 24
    float piecesSpreadFraction; // 28
    float piecesUpwardVelocity; // 32
    bool tempDefaultToCylinder; // 36
};

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
    char dest[64]; // [esp+24h] [ebp-2080h] BYREF
    char buffer[8192]; // [esp+64h] [ebp-2040h] BYREF
    PhysPresetLite pStruct;
    PhysPreset *physPreset; // [esp+2090h] [ebp-14h]
    char *last; // [esp+2094h] [ebp-10h]
    signed int filelen; // [esp+2098h] [ebp-Ch]
    int f; // [esp+209Ch] [ebp-8h] BYREF
    int len; // [esp+20A0h] [ebp-4h]

    last = (char*)"PHYSIC";
    len = strlen("PHYSIC");
    if (!strlen(name))
        return 0;
    if (Com_sprintf(dest, 0x40u, "physic/%s", name) >= 0)
    {
        filelen = FS_FOpenFileByMode(dest, &f, FS_READ);
        if (filelen >= 0)
        {
            FS_Read((unsigned __int8 *)buffer, len, f);
            buffer[len] = 0;
            if (!strncmp(buffer, last, len))
            {
                if (filelen - len < 0x2000)
                {
                    FS_Read((unsigned __int8 *)buffer, filelen - len, f);
                    buffer[filelen - len] = 0;
                    FS_FCloseFile(f);
                    if (Info_Validate(buffer))
                    {
                        memset(&pStruct, 0, sizeof(pStruct));
                        pStruct.sndAliasPrefix = "";
                        physAlloc = Alloc;
                        if (ParseConfigStringToStruct((unsigned char*)&pStruct, physPresetFields, 10, buffer, 0, 0, PhysPreset_Strcpy))
                        {
                            iassert(sizeof(PhysPreset) == 44);

                            physPreset = (PhysPreset *)Alloc(sizeof(PhysPreset));

                            iassert(physPreset);

                            physPreset->mass = pStruct.mass;
                            physPreset->bounce = pStruct.bounce;

                            if (pStruct.isFrictionInfinity)
                                physPreset->friction = FLT_MAX;
                            else
                                physPreset->friction = pStruct.friction;

                            physPreset->bulletForceScale = pStruct.bulletForceScale;
                            physPreset->explosiveForceScale = pStruct.explosiveForceScale;
                            physPreset->sndAliasPrefix = pStruct.sndAliasPrefix;
                            physPreset->piecesSpreadFraction = pStruct.piecesSpreadFraction;
                            physPreset->piecesUpwardVelocity = pStruct.piecesUpwardVelocity;
                            physPreset->tempDefaultToCylinder = pStruct.tempDefaultToCylinder;
                            return physPreset;
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


