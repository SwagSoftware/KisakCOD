#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "g_save.h"

#include "g_local.h"
#include "savememory.h"

#include <xanim/xanim.h>
#include "g_main.h"
#include <script/scr_readwrite.h>
#include <script/scr_animtree.h>
#include "actor_threat.h"
#include "actor_event_listeners.h"
#include <server/sv_public.h>

bool g_useDevSaveArea;

gclient_s tempClient;

const char *monthStr[12] =
{
  "JAN",
  "FEB",
  "MAR",
  "APR",
  "MAY",
  "JUN",
  "JUL",
  "AUG",
  "SEP",
  "OCT",
  "NOV",
  "DEC"
};

const saveField_t badplaceFields[2] = { { 8, SF_STRING }, { 0, SF_NONE } };
const saveField_t badplaceBrushParmsFields[2] = { { 0, SF_ENTITY }, { 0, SF_NONE } };
const saveField_t badplaceDefaultParmsFields[1] = { { 0, SF_NONE } };
const saveField_t pathnodeFields[2] = { { 0, SF_SENTIENTHANDLE }, { 0, SF_NONE } };
const saveField_t turretFields[4] =
{
  { 20, SF_ENTHANDLE },
  { 16, SF_ENTHANDLE },
  { 100, SF_SENTIENTHANDLE },
  { 0, SF_NONE }
};

const saveField_t vehicleFields[14] =
{
  { 56, SF_STRING },
  { 58, SF_STRING },
  { 60, SF_STRING },
  { 62, SF_STRING },
  { 124, SF_STRING },
  { 126, SF_STRING },
  { 128, SF_STRING },
  { 130, SF_STRING },
  { 576, SF_STRING },
  { 578, SF_STRING },
  { 728, SF_ENTHANDLE },
  { 732, SF_ENTHANDLE },
  { 636, SF_ENTHANDLE },
  { 0, SF_NONE }
};

const saveField_t threatGroupFields[17] =
{
  { 0, SF_STRING },
  { 2, SF_STRING },
  { 4, SF_STRING },
  { 6, SF_STRING },
  { 8, SF_STRING },
  { 10, SF_STRING },
  { 12, SF_STRING },
  { 14, SF_STRING },
  { 16, SF_STRING },
  { 18, SF_STRING },
  { 20, SF_STRING },
  { 22, SF_STRING },
  { 24, SF_STRING },
  { 26, SF_STRING },
  { 28, SF_STRING },
  { 30, SF_STRING },
  { 0, SF_NONE }
};


const saveField_t gentityFields[86] =
{
  { 256, SF_CLIENT },
  { 260, SF_ACTOR },
  { 264, SF_SENTIENT },
  { 268, SF_VEHICLE },
  { 272, SF_TURRETINFO },
  { 284, SF_STRING },
  { 280, SF_MODELUSHORT },
  { 316, SF_ENTHANDLE },
  { 290, SF_STRING },
  { 292, SF_STRING },
  { 344, SF_ENTITY },
  { 348, SF_ENTITY },
  { 286, SF_STRING },
  { 288, SF_STRING },
  { 484, SF_MODELUSHORT },
  { 486, SF_MODELUSHORT },
  { 488, SF_MODELUSHORT },
  { 490, SF_MODELUSHORT },
  { 492, SF_MODELUSHORT },
  { 494, SF_MODELUSHORT },
  { 496, SF_MODELUSHORT },
  { 498, SF_MODELUSHORT },
  { 500, SF_MODELUSHORT },
  { 502, SF_MODELUSHORT },
  { 504, SF_MODELUSHORT },
  { 506, SF_MODELUSHORT },
  { 508, SF_MODELUSHORT },
  { 510, SF_MODELUSHORT },
  { 512, SF_MODELUSHORT },
  { 514, SF_MODELUSHORT },
  { 516, SF_MODELUSHORT },
  { 518, SF_MODELUSHORT },
  { 520, SF_MODELUSHORT },
  { 522, SF_MODELUSHORT },
  { 524, SF_MODELUSHORT },
  { 526, SF_MODELUSHORT },
  { 528, SF_MODELUSHORT },
  { 530, SF_MODELUSHORT },
  { 532, SF_MODELUSHORT },
  { 534, SF_MODELUSHORT },
  { 536, SF_MODELUSHORT },
  { 538, SF_MODELUSHORT },
  { 540, SF_MODELUSHORT },
  { 542, SF_MODELUSHORT },
  { 544, SF_MODELUSHORT },
  { 546, SF_STRING },
  { 548, SF_STRING },
  { 550, SF_STRING },
  { 552, SF_STRING },
  { 554, SF_STRING },
  { 556, SF_STRING },
  { 558, SF_STRING },
  { 560, SF_STRING },
  { 562, SF_STRING },
  { 564, SF_STRING },
  { 566, SF_STRING },
  { 568, SF_STRING },
  { 570, SF_STRING },
  { 572, SF_STRING },
  { 574, SF_STRING },
  { 576, SF_STRING },
  { 578, SF_STRING },
  { 580, SF_STRING },
  { 582, SF_STRING },
  { 584, SF_STRING },
  { 586, SF_STRING },
  { 588, SF_STRING },
  { 590, SF_STRING },
  { 592, SF_STRING },
  { 594, SF_STRING },
  { 596, SF_STRING },
  { 598, SF_STRING },
  { 600, SF_STRING },
  { 602, SF_STRING },
  { 604, SF_STRING },
  { 606, SF_STRING },
  { 248, SF_ENTHANDLE },
  { 448, SF_ENTHANDLE },
  { 620, SF_ANIMTREE },
  { 472, SF_TYPE_TAG_INFO },
  { 480, SF_TYPE_SCRIPTED },
  { 476, SF_ENTITY },
  { 456, SF_STRING },
  { 452, SF_STRING },
  { 454, SF_STRING },
  { 0, SF_NONE }
};

const saveField_t actorFields[77] =
{
  { 0, SF_ENTITY },
  { 4, SF_SENTIENT },
  { 496, SF_THREAD },
  { 508, SF_STRING },
  { 500, SF_ANIMSCRIPT },
  { 2084, SF_ANIMSCRIPT },
  { 2024, SF_PATHNODE },
  { 2028, SF_PATHNODE },
  { 2032, SF_PATHNODE },
  { 2036, SF_PATHNODE },
  { 2040, SF_PATHNODE },
  { 2044, SF_PATHNODE },
  { 2048, SF_PATHNODE },
  { 2052, SF_PATHNODE },
  { 2056, SF_PATHNODE },
  { 2060, SF_PATHNODE },
  { 2136, SF_PATHNODE },
  { 2176, SF_PATHNODE },
  { 2216, SF_PATHNODE },
  { 2256, SF_PATHNODE },
  { 2296, SF_PATHNODE },
  { 2336, SF_PATHNODE },
  { 2376, SF_PATHNODE },
  { 2416, SF_PATHNODE },
  { 2456, SF_PATHNODE },
  { 2496, SF_PATHNODE },
  { 2536, SF_PATHNODE },
  { 2576, SF_PATHNODE },
  { 2616, SF_PATHNODE },
  { 2656, SF_PATHNODE },
  { 2696, SF_PATHNODE },
  { 2736, SF_PATHNODE },
  { 2776, SF_PATHNODE },
  { 2816, SF_PATHNODE },
  { 2856, SF_PATHNODE },
  { 2896, SF_PATHNODE },
  { 2936, SF_PATHNODE },
  { 2976, SF_PATHNODE },
  { 3016, SF_PATHNODE },
  { 3056, SF_PATHNODE },
  { 3096, SF_PATHNODE },
  { 3136, SF_PATHNODE },
  { 3176, SF_PATHNODE },
  { 3216, SF_PATHNODE },
  { 3256, SF_PATHNODE },
  { 3296, SF_PATHNODE },
  { 3336, SF_PATHNODE },
  { 3376, SF_PATHNODE },
  { 3416, SF_PATHNODE },
  { 3472, SF_SENTIENT },
  { 3496, SF_SENTIENT },
  { 3520, SF_SENTIENT },
  { 3544, SF_SENTIENT },
  { 488, SF_STRING },
  { 490, SF_STRING },
  { 212, SF_STRING },
  { 214, SF_STRING },
  { 3420, SF_SENTIENTHANDLE },
  { 3608, SF_ENTHANDLE },
  { 3616, SF_STRING },
  { 3680, SF_ENTITY },
  { 3712, SF_STRING },
  { 3714, SF_STRING },
  { 3716, SF_STRING },
  { 3718, SF_ENTHANDLE },
  { 1828, SF_ACTOR },
  { 1832, SF_ENTITY },
  { 318, SF_STRING },
  { 1896, SF_PATHNODE },
  { 1900, SF_ENTITY },
  { 1936, SF_ENTHANDLE },
  { 1928, SF_PATHNODE },
  { 1932, SF_ENTITY },
  { 1968, SF_ENTHANDLE },
  { 1972, SF_PATHNODE },
  { 3464, SF_PATHNODE },
  { 0, SF_NONE }
};

const saveField_t sentientFields[10] =
{
  { 0, SF_ENTITY },
  { 52, SF_ENTHANDLE },
  { 56, SF_ENTHANDLE },
  { 88, SF_PATHNODE },
  { 92, SF_PATHNODE },
  { 96, SF_PATHNODE },
  { 104, SF_PATHNODE },
  { 44, SF_ENTITY },
  { 48, SF_ENTHANDLE },
  { 0, SF_NONE }
};





void __cdecl TRACK_g_save()
{
    track_static_alloc_internal(&tempClient, sizeof(gclient_s), "tempClient", 9);
}

void __cdecl Scr_FreeFields(const saveField_t *fields, unsigned __int8 *base)
{
    const saveFieldtype_t *p_type; // r11
    const saveField_t *i; // r31
    saveFieldtype_t v5; // r11
    EntHandle *enthand;
    SentientHandle *senthand;

    p_type = &fields->type;
    for (i = fields; i->type; p_type = &i->type)
    {
        v5 = *p_type;
        switch (v5)
        {
        case SF_STRING:
            Scr_SetString((unsigned __int16 *)&base[i->ofs], 0);
            break;
        case SF_ENTHANDLE:
            enthand = (EntHandle *)&base[i->ofs];
            enthand->setEnt(NULL);
            break;
        case SF_SENTIENTHANDLE:
            senthand = (SentientHandle *)&base[i->ofs];
            senthand->setSentient(NULL);
            break;
        }
        ++i;
    }
}

void __cdecl Scr_FreeEntityFields(gentity_s *ent)
{
    Scr_FreeFields(gentityFields, &ent->s.eType);
}

void __cdecl Scr_FreeActorFields(actor_s *pActor)
{
    Scr_FreeFields(actorFields, (unsigned __int8 *)pActor);
}

void __cdecl Scr_FreeSentientFields(sentient_s *sentient)
{
    Scr_FreeFields(sentientFields, (unsigned __int8 *)sentient);
}

// local variable allocation has failed, the output may be wrong!
void G_SaveError(
    errorParm_t code,
    SaveErrorType errorType,
    __int64 fmt,
    __int64 a4,
    __int64 a5,
    int a6,
    int a7,
    int a8,
    int a9,
    int a10,
    int a11,
    int a12,
    int a13,
    ...)
{
    const char *v15; // r31
    char v16[544]; // [sp+60h] [-220h] BYREF
    __int64 v17; // [sp+2A8h] [+28h] BYREF
    va_list va; // [sp+2A8h] [+28h]
    __int64 v19; // [sp+2B0h] [+30h]
    __int64 v20; // [sp+2B8h] [+38h]
    __int64 v21; // [sp+2C0h] [+40h]
    __int64 v22; // [sp+2C8h] [+48h]
    va_list va1; // [sp+2D0h] [+50h] BYREF

    va_start(va1, a13);
    va_start(va, a13);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    va_arg(va1, unsigned int);
    v17 = fmt;
    v19 = *(__int64 *)((char *)&a4 + 4);
    v20 = a4;
    v21 = *(__int64 *)((char *)&a5 + 4);
    v22 = a5;
    vsnprintf(v16, 0x200u, (const char *)HIDWORD(fmt), va);
    v15 = v16;
    v16[511] = 0;
    if (errorType)
    {
        if (errorType == SAVE_ERROR_CORRUPT_SAVE)
            v15 = "PLATFORM_ERR_SAVEGAME_BAD";
    }
    else
    {
        v15 = "PLATFORM_UNABLE_TO_READ_FROM_DEVICE";
    }
    Com_PrintError(10, v16);
    Com_Error(code, v15);
}

void __cdecl WriteCStyleString(const char *psz, int maxlen, SaveGame *save)
{
    const char *v6; // r11
    int v8; // r31
    int v9; // r4
    __int16 *v10; // r3
    char v11; // [sp+50h] [-40h] BYREF
    __int16 v12; // [sp+52h] [-3Eh] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 610, 0, "%s", "save");
    if (!psz)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 611, 0, "%s", "psz");
    if (maxlen > 0x10000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            612,
            0,
            "%s\n\t(maxlen) = %i",
            "(maxlen <= 65536)",
            maxlen);
    v6 = psz;
    while (*(unsigned __int8 *)v6++)
        ;
    v8 = v6 - psz - 1;
    if (v8 >= maxlen)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 614, 0, "len < maxlen\n\t%i, %i", v8, maxlen);
    if (maxlen > 256)
    {
        v9 = 2;
        v12 = v8;
        v10 = &v12;
    }
    else
    {
        v9 = 1;
        v11 = v8;
        v10 = (__int16 *)&v11;
    }
    SaveMemory_SaveWrite(v10, v9, save);
    SaveMemory_SaveWrite(psz, v8, save);
}

void __cdecl ReadCStyleString(char *psz, int maxlen, SaveGame *save)
{
    int v6; // r31
    _BYTE v7[2]; // [sp+50h] [-30h] BYREF
    unsigned __int16 v8; // [sp+52h] [-2Eh] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
    if (maxlen > 0x10000)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            641,
            0,
            "%s\n\t(maxlen) = %i",
            "(maxlen <= 65536)",
            maxlen);
    if (maxlen > 256)
    {
        SaveMemory_LoadRead(&v8, 2, save);
        v6 = v8;
    }
    else
    {
        SaveMemory_LoadRead(v7, 1, save);
        v6 = v7[0];
    }
    if (v6 >= maxlen)
        Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
    SaveMemory_LoadRead(psz, v6, save);
    psz[v6] = 0;
}

void __cdecl WriteWeaponIndex(unsigned int weapon, SaveGame *save)
{
    WeaponDef *WeaponDef; // r29

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 670, 0, "%s", "save");
    if (weapon)
    {
        WeaponDef = BG_GetWeaponDef(weapon);
        if (!WeaponDef)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 674, 0, "%s", "weapDef");
        if (!WeaponDef->szInternalName)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 675, 0, "%s", "weapDef->szInternalName");
        WriteCStyleString(WeaponDef->szInternalName, 256, save);
    }
    else
    {
        WriteCStyleString("", 256, save);
    }
}

int __cdecl ReadWeaponIndex(SaveGame *save)
{
    int v2; // r31
    _BYTE v4[16]; // [sp+50h] [-140h] BYREF
    char v5[304]; // [sp+60h] [-130h] BYREF

    if (!save)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 694, 0, "%s", "save");
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
    }
    SaveMemory_LoadRead(v4, 1, save);
    v2 = v4[0];
    SaveMemory_LoadRead(v5, v4[0], save);
    v5[v2] = 0;
    if (v5[0])
        return G_GetWeaponIndexForName(v5);
    else
        return 0;
}

void __cdecl WriteItemIndex(int iIndex, SaveGame *save)
{
    gitem_s *v4; // r29
    char v5; // [sp+50h] [-40h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 714, 0, "%s", "save");
    if (iIndex)
    {
        v4 = &bg_itemlist[iIndex];
        if (!v4)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 718, 0, "%s", "pItem");
        if (v4->giType != IT_WEAPON)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 720, 0, "%s", "pItem->giType == IT_WEAPON");
        WriteWeaponIndex(iIndex % 128, save);
        v5 = iIndex / 128;
        SaveMemory_SaveWrite(&v5, 1, save);
    }
    else
    {
        WriteCStyleString("", 256, save);
    }
}

int __cdecl ReadItemIndex(SaveGame *save)
{
    int v2; // r31
    const gitem_s *Item; // r3
    unsigned __int8 v5; // [sp+50h] [-140h] BYREF
    _BYTE v6[15]; // [sp+51h] [-13Fh] BYREF
    char v7[304]; // [sp+60h] [-130h] BYREF

    iassert(save);

    SaveMemory_LoadRead(v6, 1, save);
    v2 = v6[0];
    SaveMemory_LoadRead(v7, v6[0], save);
    v7[v2] = 0;
    if (v7[0] && (SaveMemory_LoadRead(&v5, 1, save), (Item = G_FindItem(v7, v5)) != 0))
        return Item - bg_itemlist;
    else
        return 0;
}

void __cdecl WriteVehicleIndex(__int16 index, SaveGame *save)
{
    const char *VehicleInfoName; // r3

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 771, 0, "%s", "save");
    VehicleInfoName = G_GetVehicleInfoName(index);
    WriteCStyleString(VehicleInfoName, 256, save);
}

int __cdecl ReadVehicleIndex(SaveGame *save)
{
    int v2; // r31
    _BYTE v4[16]; // [sp+50h] [-140h] BYREF
    char v5[304]; // [sp+60h] [-130h] BYREF

    if (!save)
    {
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 788, 0, "%s", "save");
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
    }
    SaveMemory_LoadRead(v4, 1, save);
    v2 = v4[0];
    SaveMemory_LoadRead(v5, v4[0], save);
    v5[v2] = 0;
    return G_GetVehicleInfoIndex(v5);
}

void __cdecl WriteField1(const saveField_t *field, const unsigned __int8 *base, unsigned __int8 *original)
{
    EntHandle *v3; // r28
    unsigned int v4; // r31
    unsigned int v5; // r3
    int v6; // r31
    unsigned int v7; // r31
    unsigned int v8; // r31
    unsigned int v9; // r31
    unsigned int v10; // r31
    unsigned int v11; // r31
    unsigned int v12; // r31
    unsigned __int8 *v13; // r11
    int v14; // r11
    int v15; // r31
    const XAnim_s *anims; // r29
    int index; // r29

    EntHandle *enthand;
    SentientHandle *senthand;

    v3 = (EntHandle *)&base[field->ofs];

    switch (field->type)
    {
    case SF_STRING:
        if (v3->number)
            v3->number = 1;
        break;
    case SF_ENTITY:
        if (*v3)
        {
            v4 = (*(unsigned int *)v3 - (int)g_entities) / 628 + 1;
            if (v4 > 0x880)
                Com_Error(ERR_DROP, "WriteField1: entity out of range (%i)", v4);
            *v3 = (EntHandle)v4;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_ENTHANDLE:
        enthand = (EntHandle *)&base[field->ofs];
        if (enthand->isDefined())
        {
            v5 = v3->entnum();
            v6 = v5 + 1;
            if ((int)(v5 + 1) > 2176 || v6 < 0)
                Com_Error(ERR_DROP, "WriteField1: entity out of range (%i)", v5 + 1);
            *v3 = (EntHandle)v6;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_CLIENT:
        if (*v3)
        {
            v7 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.clients) / 46104 + 1;
            if (v7 >= 2)
                Com_Error(ERR_DROP, "WriteField1: client out of range (%i)", v7);
            *v3 = (EntHandle)v7;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_ACTOR:
        if (*v3)
        {
            v8 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.actors) / 7824 + 1;
            if (v8 > 0x20)
                Com_Error(ERR_DROP, "WriteField1: actor out of range (%i)", v8);
            *v3 = (EntHandle)v8;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_SENTIENT:
        if (*v3)
        {
            v9 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.sentients) / 116 + 1;
            if (v9 >= 0x22)
                Com_Error(ERR_DROP, "WriteField1: sentient out of range (%i)", v9);
            *v3 = (EntHandle)v9;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_SENTIENTHANDLE:
        senthand = (SentientHandle *)&base[field->ofs];
        if (senthand->isDefined())
        {
            //v10 = SentientHandle::sentient((SentientHandle *)v3) - level.sentients + 1;
            v10 = senthand->sentient() - level.sentients + 1;
            if (v10 >= 0x22)
                Com_Error(ERR_DROP, "WriteField1: sentient out of range (%i)", v10);
            *v3 = (EntHandle)v10;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_VEHICLE:
        if (*v3)
        {
            v11 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.vehicles) / 824 + 1;
            if (v11 > 0x40)
                Com_Error(ERR_DROP, "WriteField1: vehicle out of range (%i)", v11);
            *v3 = (EntHandle)v11;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_TURRETINFO:
        if (*v3)
        {
            v12 = (signed int)(*(unsigned int *)v3 - (unsigned int)level.turrets) / 188 + 1;
            if (v12 > 0x20)
                Com_Error(ERR_DROP, "WriteField1: turret out of range (%i)", v12);
            *v3 = (EntHandle)v12;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_THREAD:
        v3->number = Scr_ConvertThreadToSave(v3->number);
        break;
    case SF_ANIMSCRIPT:
        v13 = (unsigned __int8 *)*v3;
        if (*v3)
        {
            if (v13 == original + 504)
            {
                *v3 = (EntHandle)-1;
            }
            else
            {
                v14 = (v13 - (unsigned __int8 *)&g_scr_data.anim) >> 3;
                v15 = v14 + 1;
                if (v14 + 1 <= 0 || v15 > 298)
                    MyAssertHandler(
                        "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
                        908,
                        0,
                        "%s\n\t(index) = %i",
                        "(index > 0 && index <= (int)( sizeof( AnimScriptList ) * MAX_AI_SPECIES / sizeof( scr_animscript_t ) ))",
                        v14 + 1);
                *v3 = (EntHandle)v15;
            }
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_PATHNODE:
        *v3 = (EntHandle)Path_SaveIndex(*(const pathnode_t **)v3);
        break;
    case SF_ANIMTREE:
        if (*v3)
        {
            anims = XAnimGetAnims(*(const XAnimTree_s **)v3);
            iassert(anims);
            index = Scr_GetAnimsIndex(anims);
            iassert(index);
            *v3 = (EntHandle)index;
        }
        else
        {
            *v3 = 0;
        }
        break;
    case SF_TYPE_TAG_INFO:
    case SF_TYPE_SCRIPTED:
        *v3 = (EntHandle)((_cntlzw((unsigned int)*v3) & 0x20) == 0);
        break;
    case SF_MODELUSHORT:
    case SF_MODELINT:
        return;
    default:
        Com_Error(ERR_DROP, "WriteField1: unknown field type");
        break;
    }
}

void __cdecl WriteField2(const saveField_t *field, unsigned __int8 *base, SaveGame *save)
{
    saveFieldtype_t type; // r11
    int ofs; // r30
    const void *v8; // r4
    MemoryFile *v9; // r3
    unsigned int v10; // r3
    const void *v11; // r4
    MemoryFile *v12; // r3
    unsigned int v13; // r3
    MemoryFile *MemoryFile; // r3
    unsigned int UsedSize; // r3
    const char *v16; // r30
    MemoryFile *v17; // r3
    MemoryFile *v18; // r3
    unsigned int v19; // r3
    unsigned __int8 v20[96]; // [sp+50h] [-F0h] BYREF
    unsigned __int8 v21[144]; // [sp+B0h] [-90h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 961, 0, "%s", "save");
    type = field->type;
    ofs = field->ofs;
    switch (type)
    {
    case SF_STRING:
        MemoryFile = SaveMemory_GetMemoryFile(save);
        UsedSize = MemFile_GetUsedSize(MemoryFile);
        //ProfMem_Begin("string", UsedSize);
        if (*(_WORD *)&base[ofs])
        {
            v16 = SL_ConvertToString(*(unsigned __int16 *)&base[ofs]);
            v17 = SaveMemory_GetMemoryFile(save);
            MemFile_WriteCString(v17, v16);
        }
        goto LABEL_12;
    case SF_TYPE_TAG_INFO:
        v11 = *(const void **)&base[ofs];
        if (!v11)
            return;
        memcpy(v21, v11, 0x70u);
        v12 = SaveMemory_GetMemoryFile(save);
        v13 = MemFile_GetUsedSize(v12);
        //ProfMem_Begin("tagInfo", v13);
        G_WriteStruct(tagInfoFields, *(unsigned __int8 **)&base[ofs], v21, 112, save);
        goto LABEL_12;
    case SF_TYPE_SCRIPTED:
        v8 = *(const void **)&base[ofs];
        if (v8)
        {
            memcpy(v20, v8, sizeof(v20));
            v9 = SaveMemory_GetMemoryFile(save);
            v10 = MemFile_GetUsedSize(v9);
            //ProfMem_Begin("animscripted", v10);
            G_WriteStruct(animscriptedFields, *(unsigned __int8 **)&base[ofs], v20, 96, save);
        LABEL_12:
            v18 = SaveMemory_GetMemoryFile(save);
            v19 = MemFile_GetUsedSize(v18);
            //ProfMem_End(v19);
        }
        break;
    }
}

void __cdecl ReadField(const saveField_t *field, unsigned __int8 *base, SaveGame *save)
{
    __int32 v6; // r11
    EntHandle *v7; // r31
    MemoryFile *MemoryFile; // r3
    const char *CString; // r3
    int v10; // r30
    bool v11; // cr58
    int v12; // r30
    int v13; // r30
    bool v14; // cr58
    int v15; // r30
    bool v16; // cr58
    int v17; // r30
    bool v18; // cr58
    int v19; // r30
    int v20; // r30
    bool v21; // cr58
    EntHandle v22; // r30
    int v23; // r30
    XAnim_s *Anims; // r30
    unsigned __int8 *v25; // r3
    unsigned __int8 *v26; // r3

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1022, 0, "%s", "save");
    v6 = field->type - 1;
    v7 = (EntHandle *)&base[field->ofs];
    switch (v6)
    {
    case 0:
        if (v7->number)
        {
            MemoryFile = SaveMemory_GetMemoryFile(save);
            CString = MemFile_ReadCString(MemoryFile);
            v7->number = SL_GetString(CString, 0);
        }
        break;
    case 1:
        v10 = (int)*v7;
        if (*(unsigned int *)v7 > 2176 || (v11 = v10 == 0, v10 < 0))
        {
            Com_Error(ERR_DROP, byte_82034F04, *v7);
            v11 = v10 == 0;
        }
        if (v11)
            goto LABEL_58;
        *v7 = (EntHandle)&g_entities[v10 - 1];
        break;
    case 2:
        v12 = (int)*v7;
        if (*(unsigned int *)v7 > 2176 || v12 < 0)
            Com_Error(ERR_DROP, byte_82034F04, *v7);
        *v7 = 0;
        if (v12)
            EntHandle::setEnt(v7, &g_entities[v12 - 1]);
        break;
    case 3:
        v13 = (int)*v7;
        if (*(unsigned int *)v7 > 1 || (v14 = v13 == 0, v13 < 0))
        {
            Com_Error(ERR_DROP, byte_82034EDC, *v7);
            v14 = v13 == 0;
        }
        if (v14)
            goto LABEL_58;
        *v7 = (EntHandle)&level.clients[v13 - 1];
        break;
    case 4:
        v15 = (int)*v7;
        if (*(unsigned int *)v7 > 32 || (v16 = v15 == 0, v15 < 0))
        {
            Com_Error(ERR_DROP, byte_82034EB8, *v7);
            v16 = v15 == 0;
        }
        if (v16)
            goto LABEL_58;
        *v7 = (EntHandle)&level.actors[v15 - 1];
        break;
    case 5:
        v17 = (int)*v7;
        if (*(unsigned int *)v7 > 33 || (v18 = v17 == 0, v17 < 0))
        {
            Com_Error(ERR_DROP, byte_82034E90, *v7);
            v18 = v17 == 0;
        }
        if (v18)
            goto LABEL_58;
        *v7 = (EntHandle)&level.sentients[v17 - 1];
        break;
    case 6:
        v19 = (int)*v7;
        if (*(unsigned int *)v7 > 33 || v19 < 0)
            Com_Error(ERR_DROP, byte_82034E90, *v7);
        *v7 = 0;
        if (v19)
            SentientHandle::setSentient((SentientHandle *)v7, &level.sentients[v19 - 1]);
        break;
    case 7:
        v20 = (int)*v7;
        if (*(unsigned int *)v7 > 64 || (v21 = v20 == 0, v20 < 0))
        {
            Com_Error(ERR_DROP, byte_82034E68, *v7);
            v21 = v20 == 0;
        }
        if (v21)
            goto LABEL_58;
        *v7 = (EntHandle)&level.vehicles[v20 - 1];
        break;
    case 8:
        v22 = *v7;
        if (*(unsigned int *)v7 > 0x40u)
            Com_Error(ERR_DROP, byte_82034E40, *v7);
        if (!*(unsigned int *)&v22)
            goto LABEL_58;
        *v7 = (EntHandle)&level.turrets[*(unsigned int *)&v22 - 1];
        break;
    case 9:
        v7->number = Scr_ConvertThreadFromLoad(v7->number);
        break;
    case 10:
        v23 = (int)*v7;
        if (*(unsigned int *)v7 > 298 || v23 < -1)
            Com_Error(ERR_DROP, byte_82034E14, *v7);
        if (!v23)
            goto LABEL_58;
        if (v23 == -1)
            *v7 = (EntHandle)(base + 504);
        else
            *v7 = (EntHandle)(&g_scr_data.scripted_init + 2 * v23);
        break;
    case 11:
        *v7 = (EntHandle)Path_LoadNode((unsigned int)*v7);
        break;
    case 12:
        if (*v7)
        {
            Anims = Scr_GetAnims((unsigned int)*v7);
            if (!Anims)
                MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1141, 0, "%s", "anims");
            *v7 = (EntHandle)Com_XAnimCreateSmallTree(Anims);
        }
        else
        {
        LABEL_58:
            *v7 = 0;
        }
        break;
    case 13:
        if (*v7)
        {
            v25 = (unsigned __int8 *)MT_Alloc(112, 17);
            *v7 = (EntHandle)v25;
            G_ReadStruct(tagInfoFields, v25, 112, save);
        }
        break;
    case 14:
        if (*v7)
        {
            v26 = (unsigned __int8 *)MT_Alloc(96, 17);
            *v7 = (EntHandle)v26;
            G_ReadStruct(animscriptedFields, v26, 96, save);
        }
        break;
    case 15:
        v7->number = *(unsigned __int16 *)((char *)level.modelMap + __ROL4__(v7->number, 1));
        break;
    case 16:
        *v7 = (EntHandle)level.modelMap[*(unsigned int *)v7];
        break;
    default:
        Com_Error(ERR_DROP, byte_82034DF4);
        break;
    }
}

void __cdecl G_WriteStruct(
    const saveField_t *fields,
    unsigned __int8 *original,
    const unsigned __int8 *source,
    int sourcesize,
    SaveGame *save)
{
    const saveField_t *i; // r30
    MemoryFile *MemoryFile; // r3
    unsigned int UsedSize; // r3
    MemoryFile *v13; // r3
    unsigned int v14; // r3

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1190, 0, "%s", "save");
    for (i = fields; i->type; ++i)
        WriteField1(i, source, original);
    MemoryFile = SaveMemory_GetMemoryFile(save);
    UsedSize = MemFile_GetUsedSize(MemoryFile);
    ProfMem_Begin("writestruct struct", UsedSize);
    SaveMemory_SaveWrite(source, sourcesize, save);
    v13 = SaveMemory_GetMemoryFile(save);
    v14 = MemFile_GetUsedSize(v13);
    ProfMem_End(v14);
    for (; fields->type; ++fields)
        WriteField2(fields, original, save);
}

void __cdecl G_ReadStruct(const saveField_t *fields, unsigned __int8 *dest, int tempsize, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1214, 0, "%s", "save");
    SaveMemory_LoadRead(dest, tempsize, save);
    for (; fields->type; ++fields)
        ReadField(fields, dest, save);
}

void __cdecl WriteClient(gclient_s *cl, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1232, 0, "%s", "save");
    memcpy(&tempClient, cl, sizeof(tempClient));
    tempClient.ps.events[0] = 0;
    tempClient.ps.events[1] = 0;
    tempClient.ps.events[2] = 0;
    tempClient.ps.events[3] = 0;
    tempClient.ps.eventParms[0] = 0;
    tempClient.ps.eventParms[1] = 0;
    tempClient.ps.eventParms[2] = 0;
    tempClient.ps.eventParms[3] = 0;
    tempClient.ps.eventSequence = 0;
    tempClient.ps.oldEventSequence = 0;
    tempClient.ps.entityEventSequence = 0;
    G_WriteStruct(gclientFields, (unsigned __int8 *)cl, (const unsigned __int8 *)&tempClient, 46104, save);
    SaveMemory_SaveWrite(&cl->pers.cmd.buttons, 4, save);
    WriteWeaponIndex(cl->pers.cmd.weapon, save);
    WriteWeaponIndex(cl->pers.cmd.offHandIndex, save);
}

void __cdecl ReadClient(gclient_s *client, SaveGame *save)
{
    unsigned __int8 WeaponIndex; // r3
    int weapon; // r4
    int buttons; // r3
    unsigned __int8 v7; // r11
    int v8; // r5

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1262, 0, "%s", "save");
    G_ReadStruct(gclientFields, (unsigned __int8 *)client, 46104, save);
    SaveMemory_LoadRead(&client->pers.cmd.buttons, 4, save);
    client->pers.cmd.weapon = ReadWeaponIndex(save);
    WeaponIndex = ReadWeaponIndex(save);
    weapon = client->pers.cmd.weapon;
    v7 = WeaponIndex;
    v8 = WeaponIndex;
    buttons = client->pers.cmd.buttons;
    client->pers.cmd.offHandIndex = v7;
    SV_SetUsercmdButtonsWeapons(buttons, weapon, v8);
}

void WriteEntity(gentity_s *ent, SaveGame *save)
{
    MemoryFile *memFile; // r3
    unsigned int UsedSize; // r3
    MemoryFile *v6; // r3
    unsigned int v7; // r3
    unsigned __int8 v8[632]; // [sp+50h] [-290h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1286, 0, "%s", "save");
    memcpy(v8, ent, 0x274u);
    memFile = SaveMemory_GetMemoryFile(save);
    UsedSize = MemFile_GetUsedSize(memFile);
    //ProfMem_Begin("WriteStruct", UsedSize);
    G_WriteStruct(gentityFields, &ent->s.eType, v8, 628, save);
    v6 = SaveMemory_GetMemoryFile(save);
    v7 = MemFile_GetUsedSize(v6);
    //ProfMem_End(v7);
    if (ent->s.weapon)
    {
        WriteWeaponIndex(ent->s.weapon, save);
        SaveMemory_SaveWrite(&ent->s.weaponModel, 1, save);
    }
    WriteEntityDisconnectedLinks(ent, save);
}

void __cdecl ReadEntity(gentity_s *ent, SaveGame *save)
{
    _BYTE v4[8]; // [sp+50h] [-20h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1308, 0, "%s", "save");
    G_ReadStruct(gentityFields, &ent->s.eType, 628, save);
    if (ent->s.weapon)
    {
        ent->s.weapon = ReadWeaponIndex(save);
        SaveMemory_LoadRead(v4, 1, save);
    }
    ReadEntityDisconnectedLinks(ent, save);
    if (ent->snd_wait.notifyString)
    {
        if (ent->snd_wait.duration < 0)
            G_AddEvent(ent, 44, ent->snd_wait.index);
    }
}

void __cdecl WriteActorPotentialCoverNodes(actor_s *pActor, SaveGame *save)
{
    int v4; // r30
    pathnode_t **pPotentialCoverNode; // r31
    int v6; // [sp+50h] [-30h] BYREF

    v4 = 0;
    if (pActor->iPotentialCoverNodeCount > 0)
    {
        pPotentialCoverNode = pActor->pPotentialCoverNode;
        do
        {
            v6 = Path_SaveIndex(*pPotentialCoverNode);
            SaveMemory_SaveWrite(&v6, 4, save);
            ++v4;
            ++pPotentialCoverNode;
        } while (v4 < pActor->iPotentialCoverNodeCount);
    }
}

void __cdecl ReadActorPotentialCoverNodes(actor_s *pActor, SaveGame *save)
{
    int v4; // r30
    pathnode_t **pPotentialCoverNode; // r31
    unsigned int v6; // [sp+50h] [-30h] BYREF

    v4 = 0;
    if (pActor->iPotentialCoverNodeCount > 0)
    {
        pPotentialCoverNode = pActor->pPotentialCoverNode;
        do
        {
            SaveMemory_LoadRead(&v6, 4, save);
            *pPotentialCoverNode = Path_LoadNode(v6);
            ++v4;
            ++pPotentialCoverNode;
        } while (v4 < pActor->iPotentialCoverNodeCount);
    }
}

void __cdecl WriteActor(actor_s *pActor, SaveGame *save)
{
    unsigned __int8 v4[3832]; // [sp+50h] [-F10h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1365, 0, "%s", "save");
    SaveMemory_SaveWrite(&pActor->inuse, 1, save);
    if (pActor->inuse)
    {
        memcpy(v4, pActor, 0xEECu);
        G_WriteStruct(actorFields, (unsigned __int8 *)pActor, v4, 3820, save);
        WriteActorPotentialCoverNodes(pActor, save);
    }
}

void __cdecl ReadActor(actor_s *pActor, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1385, 0, "%s", "save");
    SaveMemory_LoadRead(&pActor->inuse, 1, save);
    if (pActor->inuse)
    {
        G_ReadStruct(actorFields, (unsigned __int8 *)pActor, 3820, save);
        if (!pActor->inuse)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1394, 0, "%s", "pActor->inuse");
        ReadActorPotentialCoverNodes(pActor, save);
        pActor->pszDebugInfo = "";
    }
}

void __cdecl WriteSentient(sentient_s *sentient, SaveGame *save)
{
    unsigned __int8 v4[120]; // [sp+50h] [-90h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1420, 0, "%s", "save");
    SaveMemory_SaveWrite(&sentient->inuse, 1, save);
    if (sentient->inuse)
    {
        memcpy(v4, sentient, 0x74u);
        G_WriteStruct(sentientFields, (unsigned __int8 *)sentient, v4, 116, save);
    }
}

void __cdecl ReadSentient(sentient_s *sentient, SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1437, 0, "%s", "save");
    SaveMemory_LoadRead(&sentient->inuse, 1, save);
    if (sentient->inuse)
    {
        G_ReadStruct(sentientFields, (unsigned __int8 *)sentient, 116, save);
        if (!sentient->inuse)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1446, 0, "%s", "sentient->inuse");
    }
}

void __cdecl WriteVehicle(scr_vehicle_s *pVehicle, SaveGame *save)
{
    bool v4[4]; // [sp+50h] [-360h] BYREF
    unsigned __int8 v5[824]; // [sp+60h] [-350h] BYREF

    memcpy(v5, pVehicle, sizeof(v5));
    v4[0] = (_cntlzw(pVehicle->entNum - 2175) & 0x20) == 0;
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1462, 0, "%s", "save");
    SaveMemory_SaveWrite(v4, 4, save);
    if (v4[0])
    {
        G_WriteStruct(vehicleFields, (unsigned __int8 *)pVehicle, v5, 824, save);
        WriteVehicleIndex(pVehicle->infoIdx, save);
    }
}

void __cdecl ReadVehicle(scr_vehicle_s *pVehicle, SaveGame *save)
{
    int v4; // [sp+50h] [-20h] BYREF

    v4 = 0;
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1483, 0, "%s", "save");
    SaveMemory_LoadRead(&v4, 4, save);
    if (v4)
    {
        G_ReadStruct(vehicleFields, (unsigned __int8 *)pVehicle, 824, save);
        pVehicle->infoIdx = ReadVehicleIndex(save);
    }
}

void __cdecl WriteTurretInfo(TurretInfo *pTurretInfo, SaveGame *save)
{
    bool v4[4]; // [sp+50h] [-F0h] BYREF
    unsigned __int8 v5[200]; // [sp+60h] [-E0h] BYREF

    memcpy(v5, pTurretInfo, 0xBCu);
    v4[0] = pTurretInfo->inuse;
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1508, 0, "%s", "save");
    SaveMemory_SaveWrite(v4, 4, save);
    if (v4[0])
        G_WriteStruct(turretFields, (unsigned __int8 *)pTurretInfo, v5, 188, save);
}

void __cdecl ReadTurretInfo(TurretInfo *pTurretInfo, SaveGame *save)
{
    int v4; // [sp+50h] [-20h] BYREF

    v4 = 0;
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1527, 0, "%s", "save");
    SaveMemory_LoadRead(&v4, 4, save);
    if (v4)
        G_ReadStruct(turretFields, (unsigned __int8 *)pTurretInfo, 188, save);
}

void __cdecl WritePathNodes(SaveGame *save)
{
    pathnode_t *i; // r29
    unsigned __int8 *v3; // r10
    pathnode_dynamic_t *p_dynamic; // r11
    int v5; // ctr
    int wLinkCount; // r11
    int v7; // r30
    int v8; // r31
    unsigned int v9[4]; // [sp+50h] [-60h] BYREF
    unsigned __int8 v10[80]; // [sp+60h] [-50h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1552, 0, "%s", "save");
    for (i = Path_FirstNode(-1); i; i = Path_NextNode(i, -1))
    {
        v3 = v10;
        p_dynamic = &i->dynamic;
        v5 = 8;
        do
        {
            *(SentientHandle *)v3 = p_dynamic->pOwner;
            p_dynamic = (pathnode_dynamic_t *)((char *)p_dynamic + 4);
            v3 += 4;
            --v5;
        } while (v5);
        G_WriteStruct(pathnodeFields, (unsigned __int8 *)&i->dynamic, v10, 32, save);
        wLinkCount = i->dynamic.wLinkCount;
        if (wLinkCount != i->constant.totalLinkCount)
        {
            v7 = 0;
            if (wLinkCount > 0)
            {
                v8 = 0;
                do
                {
                    v9[0] = i->constant.Links[v8].nodeNum;
                    SaveMemory_SaveWrite(v9, 4, save);
                    ++v7;
                    ++v8;
                } while (v7 < i->dynamic.wLinkCount);
            }
        }
    }
}

void __cdecl ReadPathNodes(SaveGame *save)
{
    pathnode_t *i; // r31
    int wLinkCount; // r11
    int v4; // r27
    int v5; // r29
    int v6; // r30
    int j; // r28
    pathlink_s *Links; // r11
    pathlink_s *v9; // r7
    float *p_fDist; // r11
    float fDist; // r6
    int v12; // r10
    int v13; // r9
    int v14; // r8
    unsigned int *v15; // r11
    int v16; // [sp+50h] [-60h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1584, 0, "%s", "save");
    for (i = Path_FirstNode(-1); i; i = Path_NextNode(i, -1))
    {
        G_ReadStruct(pathnodeFields, (unsigned __int8 *)&i->dynamic, 32, save);
        wLinkCount = i->dynamic.wLinkCount;
        if (wLinkCount != i->constant.totalLinkCount)
        {
            v4 = 0;
            if (wLinkCount > 0)
            {
                v5 = 0;
                do
                {
                    SaveMemory_LoadRead(&v16, 4, save);
                    v6 = i->constant.totalLinkCount - 1;
                    for (j = v6; ; --j)
                    {
                        if (v6 < 0)
                            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1597, 0, "%s", "j >= 0");
                        if (i->constant.Links[j].nodeNum == v16)
                            break;
                        --v6;
                    }
                    Links = i->constant.Links;
                    ++v4;
                    v9 = &Links[v5];
                    fDist = Links[v5].fDist;
                    p_fDist = &Links[v6].fDist;
                    v12 = *(unsigned int *)p_fDist;
                    v13 = *((unsigned int *)p_fDist + 1);
                    v14 = *((unsigned int *)p_fDist + 2);
                    *p_fDist = fDist;
                    p_fDist[1] = *(float *)&v9->nodeNum;
                    p_fDist[2] = *(float *)v9->ubBadPlaceCount;
                    v15 = (unsigned int *)&i->constant.Links[v5++].fDist;
                    *v15 = v12;
                    v15[1] = v13;
                    v15[2] = v14;
                } while (v4 < i->dynamic.wLinkCount);
            }
        }
    }
}

const saveField_t *__cdecl BadPlaceParmSaveFields(const badplace_t *badplace)
{
    if (!badplace)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1618, 0, "%s", "badplace");
    if (badplace->type == 2)
        return badplaceBrushParmsFields;
    else
        return badplaceDefaultParmsFields;
}

void __cdecl WriteBadPlaces(SaveGame *save)
{
    int v2; // r27
    unsigned __int8 *v3; // r31
    unsigned __int8 *v4; // r10
    unsigned __int8 *v5; // r11
    int v6; // ctr
    unsigned __int8 *v7; // r10
    unsigned int *v8; // r11
    int v9; // ctr
    const saveField_t *v10; // r3
    unsigned __int8 v11[32]; // [sp+50h] [-90h] BYREF
    unsigned __int8 v12[112]; // [sp+70h] [-70h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1641, 0, "%s", "save");
    v2 = 32;
    v3 = (unsigned __int8 *)g_badplaces;
    do
    {
        v4 = v12;
        v5 = v3;
        v6 = 10;
        do
        {
            *(unsigned int *)v4 = *(unsigned int *)v5;
            v5 += 4;
            v4 += 4;
            --v6;
        } while (v6);
        G_WriteStruct(badplaceFields, v3, v12, 12, save);
        v7 = v11;
        v8 = v3 + 12;
        v9 = 7;
        do
        {
            *(unsigned int *)v7 = *v8++;
            v7 += 4;
            --v9;
        } while (v9);
        if (!v3)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1618, 0, "%s", "badplace");
        v10 = badplaceDefaultParmsFields;
        if (v3[10] == 2)
            v10 = badplaceBrushParmsFields;
        G_WriteStruct(v10, v3 + 12, v11, 28, save);
        --v2;
        v3 += 40;
    } while (v2);
}

void __cdecl ReadBadPlaces(SaveGame *save)
{
    int loops; // r27
    badplace_t *badplace; // r31
    const saveField_t *v4; // r3

    iassert(save);
    loops = ARRAY_COUNT(g_badplaces);
    badplace = g_badplaces;
    do
    {
        G_ReadStruct(badplaceFields, (unsigned __int8 *)badplace, 12, save);
        if (!badplace)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1618, 0, "%s", "badplace");
        v4 = badplaceDefaultParmsFields;
        if (badplace->type == 2)
            v4 = badplaceBrushParmsFields;
        G_ReadStruct(v4, (unsigned __int8 *)&badplace->parms, 28, save);
        if (badplace->type)
            Path_UpdateBadPlaceCount(badplace, 1);
        --loops;
        ++badplace;
    } while (loops);
}

void __cdecl WriteThreatBiasGroups(SaveGame *save)
{
    unsigned __int8 v2[1064]; // [sp+50h] [-440h] BYREF

    memcpy(v2, &g_threatBias, 0x424u);
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1686, 0, "%s", "save");
    G_WriteStruct(threatGroupFields, (unsigned __int8 *)&g_threatBias, v2, 1060, save);
}

void __cdecl ReadThreatBiasGroups(SaveGame *save)
{
    G_ReadStruct(threatGroupFields, (unsigned __int8 *)&g_threatBias, 1060, save);
}

void __cdecl WriteAIEventListeners(SaveGame *save)
{
    int v2[4]; // [sp+50h] [-20h] BYREF

    v2[0] = Actor_EventListener_GetCount();
    SaveMemory_SaveWrite(v2, 4, save);
    if (v2[0])
        SaveMemory_SaveWrite(g_AIEVlisteners, 8 * v2[0], save);
}

void __cdecl ReadAIEventListeners(SaveGame *save)
{
    int v2[4]; // [sp+50h] [-20h] BYREF

    if (Actor_EventListener_GetCount())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            1724,
            0,
            "%s",
            "Actor_EventListener_GetCount() == 0");
    SaveMemory_LoadRead(v2, 4, save);
    Actor_EventListener_SetCount(v2[0]);
    if (v2[0])
        SaveMemory_LoadRead(g_AIEVlisteners, 8 * v2[0], save);
}

char *__cdecl G_Save_DateStr()
{
    qtime_s v1; // [sp+50h] [-30h] BYREF

    Com_RealTime(&v1);
    return va("%s %i, %i", monthStr[v1.tm_mon], v1.tm_mday, v1.tm_year + 1900);
}

void __cdecl G_SaveConfigstrings(int iFirst, int iCount, SaveGame *save)
{
    int i; // r29
    char *v7; // r11
    int v9; // r11
    int v10; // r31
    _WORD v11[8]; // [sp+50h] [-460h] BYREF
    char v12[1104]; // [sp+60h] [-450h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1773, 0, "%s", "save");
    for (i = 0; i < iCount; ++i)
    {
        SV_GetConfigstring(i + iFirst, v12, 1024);
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 610, 0, "%s", "save");
        v7 = v12;
        while (*v7++)
            ;
        v9 = v7 - v12 - 1;
        v10 = v9;
        if (v9 >= 1024)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 614, 0, "len < maxlen\n\t%i, %i", v9, 1024);
        v11[0] = v10;
        SaveMemory_SaveWrite(v11, 2, save);
        SaveMemory_SaveWrite(v12, v10, save);
    }
}

void __cdecl G_LoadConfigstrings(int iFirst, int iCount, SaveGame *save)
{
    int i; // r31
    int v7; // r30
    _WORD v8[8]; // [sp+50h] [-470h] BYREF
    char v9[1120]; // [sp+60h] [-460h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1795, 0, "%s", "save");
    for (i = 0; i < iCount; ++i)
    {
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
        SaveMemory_LoadRead(v8, 2, save);
        v7 = v8[0];
        if (v8[0] >= 0x400u)
            Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
        SaveMemory_LoadRead(v9, v7, save);
        v9[v7] = 0;
        SV_SetConfigstring(i + iFirst, v9);
    }
}

void __cdecl G_LoadModelPrecacheList(SaveGame *save)
{
    unsigned __int16 *modelMap; // r30
    int v3; // r31
    unsigned __int16 v4; // r3
    _WORD v5[8]; // [sp+50h] [-460h] BYREF
    char v6[1104]; // [sp+60h] [-450h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1818, 0, "%s", "save");
    modelMap = level.modelMap;
    do
    {
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
        SaveMemory_LoadRead(v5, 2, save);
        v3 = v5[0];
        if (v5[0] >= 0x400u)
            Com_Error(ERR_DROP, "GAME_ERR_SAVEGAME_BAD");
        SaveMemory_LoadRead(v6, v3, save);
        v6[v3] = 0;
        if (v6[0])
            v4 = G_ModelIndex(v6);
        else
            v4 = 0;
        *modelMap++ = v4;
    } while ((int)modelMap < (int)&level.priorityNodeBias);
}

void __cdecl G_ClearConfigstrings(int iFirst, int iCount)
{
    int i; // r31

    for (i = 0; i < iCount; ++i)
        SV_SetConfigstring(i + iFirst, "");
}

void __cdecl G_ClearAllConfigstrings()
{
    int i; // r30
    int j; // r30
    int k; // r30
    int m; // r30
    int n; // r30
    int ii; // r30
    int jj; // r30

    for (i = 0; i < 100; ++i)
        SV_SetConfigstring(i + 2179, "");
    for (j = 0; j < 256; ++j)
        SV_SetConfigstring(j + 2279, "");
    for (k = 0; k < 512; ++k)
        SV_SetConfigstring(k + 1667, "");
    for (m = 0; m < 128; ++m)
        SV_SetConfigstring(m + 2583, "");
    for (n = 0; n < 1023; ++n)
        SV_SetConfigstring(n + 91, "");
    SV_SetConfigstring(1114, "");
    for (ii = 0; ii < 16; ++ii)
        SV_SetConfigstring(ii + 11, "");
    SV_SetConfigstring(6, "");
    SV_SetConfigstring(7, "");
    SV_SetConfigstring(8, "");
    SV_SetConfigstring(1148, "");
    SV_SetConfigstring(1151, "");
    for (jj = 0; jj < 32; ++jj)
        SV_SetConfigstring(jj + 27, "");
}

void __cdecl G_SaveInitConfigstrings(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1871, 0, "%s", "save");
    G_SaveConfigstrings(1155, 512, save);
    G_SaveConfigstrings(2179, 100, save);
    G_SaveConfigstrings(2279, 256, save);
    G_SaveConfigstrings(1667, 512, save);
    G_SaveConfigstrings(2583, 128, save);
    G_SaveConfigstrings(91, 1023, save);
    G_SaveConfigstrings(1114, 1, save);
    G_SaveConfigstrings(6, 1, save);
    G_SaveConfigstrings(7, 1, save);
    G_SaveConfigstrings(8, 1, save);
    G_SaveConfigstrings(1148, 1, save);
    G_SaveConfigstrings(1151, 1, save);
    G_SaveConfigstrings(1149, 1, save);
    G_SaveConfigstrings(1150, 1, save);
}

void __cdecl G_LoadInitConfigstrings(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1896, 0, "%s", "save");
    G_LoadModelPrecacheList(save);
    G_LoadConfigstrings(2179, 100, save);
    G_LoadConfigstrings(2279, 256, save);
    G_LoadConfigstrings(1667, 512, save);
    G_LoadConfigstrings(2583, 128, save);
    G_LoadConfigstrings(91, 1023, save);
    G_LoadConfigstrings(1114, 1, save);
    G_LoadConfigstrings(6, 1, save);
    G_LoadConfigstrings(7, 1, save);
    G_LoadConfigstrings(8, 1, save);
    G_LoadConfigstrings(1148, 1, save);
    G_LoadConfigstrings(1151, 1, save);
    G_LoadConfigstrings(1149, 1, save);
    G_LoadConfigstrings(1150, 1, save);
}

void __cdecl G_SaveItems(SaveGame *save)
{
    int v2; // r31
    _BYTE v3[8]; // [sp+50h] [-20h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1927, 0, "%s", "save");
    v2 = 1;
    v3[0] = 0;
    do
    {
        if (IsItemRegistered(v2))
        {
            SaveMemory_SaveWrite(v3, 1, save);
            WriteItemIndex(v2, save);
        }
        ++v2;
    } while (v2 < 128);
    v3[0] = 1;
    SaveMemory_SaveWrite(v3, 1, save);
}

void __cdecl G_SaveWeaponCue(SaveGame *save)
{
    EntHandle *droppedWeaponCue; // r31
    int number; // r11
    unsigned int v4; // [sp+50h] [-50h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1956, 0, "%s", "save");
    droppedWeaponCue = level.droppedWeaponCue;
    do
    {
        number = droppedWeaponCue->number;
        if (droppedWeaponCue->number && !g_entities[number - 1].r.inuse)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_public.h",
                336,
                0,
                "%s\n\t(number - 1) = %i",
                "(!number || g_entities[number - 1].r.inuse)",
                number - 1);
        if (droppedWeaponCue->number)
            v4 = droppedWeaponCue->entnum() + 1;
        else
            v4 = 0;
        SaveMemory_SaveWrite(&v4, 4, save);
        ++droppedWeaponCue;
    } while ((int)droppedWeaponCue < (int)&level.changelevel);
}

void __cdecl G_LoadWeaponCue(SaveGame *save)
{
    EntHandle *droppedWeaponCue; // r30
    int number; // r11
    int v4; // r5
    bool v5; // cr58
    int v6; // [sp+50h] [-70h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1979, 0, "%s", "save");
    droppedWeaponCue = level.droppedWeaponCue;
    do
    {
        SaveMemory_LoadRead(&v6, 4, save);
        number = droppedWeaponCue->number;
        if (droppedWeaponCue->number && !g_entities[number - 1].r.inuse)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_public.h",
                336,
                0,
                "%s\n\t(number - 1) = %i",
                "(!number || g_entities[number - 1].r.inuse)",
                number - 1);
        if (droppedWeaponCue->number)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
                1984,
                0,
                "%s",
                "!level.droppedWeaponCue[i].isDefined()");
        v4 = v6;
        if (v6 > 2176 || (v5 = v6 == 0, v6 < 0))
        {
            Com_Error(ERR_DROP, "G_LoadWeaponCue: entity out of range (%i)", v6);
            v4 = v6;
            v5 = v6 == 0;
        }
        if (!v5)
            droppedWeaponCue->setEnt(&g_entities[v4 - 1]);
        ++droppedWeaponCue;
    } while ((int)droppedWeaponCue < (int)&level.changelevel);
}

void __cdecl G_SaveDvars(SaveGame *save)
{
    MemoryFile *MemoryFile; // r3

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2001, 0, "%s", "save");
    MemoryFile = SaveMemory_GetMemoryFile(save);
    Dvar_SaveDvars(MemoryFile, 0x1000u);
}

void __cdecl G_LoadDvars(SaveGame *save)
{
    MemoryFile *MemoryFile; // r3

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2013, 0, "%s", "save");
    MemoryFile = SaveMemory_GetMemoryFile(save);
    Dvar_LoadDvars(MemoryFile);
}

void __cdecl G_CheckEntityDefaultModel(gentity_s *e)
{
    int model; // r3
    int eType; // r11
    unsigned int v4; // r3
    const char *v5; // r3

    model = e->model;
    if (model)
    {
        if (G_XModelBad(model))
        {
            eType = e->s.eType;
            if (eType == 14 || eType == 16)
            {
                v4 = G_ModelName(e->model);
                v5 = SL_ConvertToString(v4);
                Com_PrintWarning(10, "WARNING: actor model '%s' couldn't be found! switching to default actor model.\n", v5);
                G_OverrideModel(e->model, "defaultactor");
            }
        }
    }
}

void __cdecl G_UpdateAllEntities()
{
    int v0; // r30
    int num_entities; // r11
    entityShared_t *p_r; // r31

    v0 = 0;
    num_entities = level.num_entities;
    if (level.num_entities > 0)
    {
        p_r = &g_entities[0].r;
        do
        {
            if (p_r->inuse)
            {
                if (p_r->linked)
                {
                    SV_LinkEntity((gentity_s *)p_r[-2].maxs);
                    num_entities = level.num_entities;
                }
            }
            ++v0;
            p_r = (entityShared_t *)((char *)p_r + 628);
        } while (v0 < num_entities);
    }
}

void G_CheckAllEntities()
{
    int v0; // r30
    gentity_s *v1; // r31
    int i; // r11

    v0 = 0;
    v1 = g_entities;
    for (i = level.num_entities; v0 < i; ++v1)
    {
        if (v1->r.inuse)
        {
            G_CheckDObjUpdate(v1);
            i = level.num_entities;
        }
        ++v0;
    }
}

void __cdecl G_SaveInitState(SaveGame *save)
{
    signed int i; // r28
    const char *szInternalName; // r29
    const char *v4; // r11
    int v6; // r11
    int v7; // r31
    _BYTE v8[4]; // [sp+50h] [-60h] BYREF
    signed int NumWeapons; // [sp+54h] [-5Ch] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2099, 0, "%s", "save");
    SaveMemory_StartSegment(save, 1);
    NumWeapons = BG_GetNumWeapons();
    SaveMemory_SaveWrite(&NumWeapons, 4, save);
    for (i = 1; i < NumWeapons; ++i)
    {
        szInternalName = BG_GetWeaponDef(i)->szInternalName;
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 610, 0, "%s", "save");
        if (!szInternalName)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 611, 0, "%s", "psz");
        v4 = szInternalName;
        while (*(unsigned __int8 *)v4++)
            ;
        v6 = v4 - szInternalName - 1;
        v7 = v6;
        if (v6 >= 256)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 614, 0, "len < maxlen\n\t%i, %i", v6, 256);
        v8[0] = v7;
        SaveMemory_SaveWrite(v8, 1, save);
        SaveMemory_SaveWrite(szInternalName, v7, save);
    }
    G_SaveItems(save);
    G_SaveInitConfigstrings(save);
}

void __cdecl G_SaveMainState(bool savegame, SaveGame *save)
{
    MemoryFile *MemoryFile; // r3
    unsigned int UsedSize; // r3
    MemoryFile *v6; // r3
    unsigned int v7; // r3
    MemoryFile *v8; // r25
    MemoryFile *v9; // r3
    MemoryFile *v10; // r3
    unsigned int v11; // r3
    MemoryFile *v12; // r3
    unsigned int v13; // r3
    MemoryFile *v14; // r3
    unsigned int v15; // r3
    MemoryFile *v16; // r3
    unsigned int v17; // r3
    MemoryFile *v18; // r3
    unsigned int v19; // r3
    MemoryFile *v20; // r3
    unsigned int v21; // r3
    int v22; // r11
    gentity_s *v23; // r29
    MemoryFile *v24; // r3
    unsigned int v25; // r3
    MemoryFile *v26; // r3
    unsigned int v27; // r3
    int v28; // r9
    gclient_s *v29; // r29
    int v30; // r11
    int v31; // r11
    int v32; // r11
    int v33; // r11
    int v34; // r11
    MemoryFile *v35; // r3
    unsigned int v36; // r3
    MemoryFile *v37; // r3
    unsigned int v38; // r3
    MemoryFile *v39; // r3
    unsigned int v40; // r3
    MemoryFile *v41; // r3
    unsigned int v42; // r3
    int v43; // r3
    const DObj_s *ServerDObj; // r3
    const DObj_s *v45; // r30
    unsigned int *v46; // r29
    int v47; // r30
    MemoryFile *v48; // r3
    unsigned int v49; // r3
    MemoryFile *v50; // r3
    unsigned int v51; // r3
    MemoryFile *v52; // r3
    unsigned int v53; // r3
    MemoryFile *v54; // r3
    unsigned int v55; // r3
    MemoryFile *v56; // r3
    unsigned int v57; // r3
    MemoryFile *v58; // r3
    unsigned int v59; // r3
    int i; // [sp+50h] [-4B0h] BYREF
    unsigned int v61[3]; // [sp+54h] [-4ACh] BYREF
    unsigned int v62[4]; // [sp+60h] [-4A0h] BYREF
    unsigned __int8 v63[1168]; // [sp+70h] [-490h] BYREF

    MemoryFile = SaveMemory_GetMemoryFile(save);
    UsedSize = MemFile_GetUsedSize(MemoryFile);
    ProfMem_Begin("SaveMainState", UsedSize);
    v6 = SaveMemory_GetMemoryFile(save);
    v7 = MemFile_GetUsedSize(v6);
    ProfMem_Begin("level state, dvars, hudelems", v7);
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2140, 0, "%s", "save");
    v8 = SaveMemory_GetMemoryFile(save);
    SaveMemory_StartSegment(save, 2);
    SaveMemory_SaveWrite(&level.time, 4, save);
    SaveMemory_SaveWrite(&level.framenum, 4, save);
    SaveMemory_SaveWrite(&level.framePos, 4, save);
    SaveMemory_SaveWrite(&level.soundAliasFirst, 2, save);
    SaveMemory_SaveWrite(&level.soundAliasLast, 2, save);
    SaveMemory_SaveWrite(&level.changelevel, 4, save);
    SaveMemory_SaveWrite(&level.exitTime, 4, save);
    SaveMemory_SaveWrite(&level.savepersist, 4, save);
    SaveMemory_SaveWrite(&level.bMissionSuccess, 4, save);
    SaveMemory_SaveWrite(&level.bMissionFailed, 4, save);
    SaveMemory_SaveWrite(&level.scriptPrintChannel, 4, save);
    SaveMemory_SaveWrite(g_nextMap, 64, save);
    SaveMemory_SaveWrite(level.compassMapUpperLeft, 8, save);
    SaveMemory_SaveWrite(level.compassMapWorldSize, 8, save);
    SaveMemory_SaveWrite(level.compassNorth, 8, save);
    if (level.bPlayerIgnoreRadiusDamage)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            2170,
            0,
            "%s",
            "!level.bPlayerIgnoreRadiusDamage");
    SaveMemory_SaveWrite(&level.bPlayerIgnoreRadiusDamageLatched, 4, save);
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2001, 0, "%s", "save");
    v9 = SaveMemory_GetMemoryFile(save);
    Dvar_SaveDvars(v9, 0x1000u);
    SaveMemory_SaveWrite(g_hudelems, 44032, save);
    v10 = SaveMemory_GetMemoryFile(save);
    v11 = MemFile_GetUsedSize(v10);
    ProfMem_End(v11);
    v12 = SaveMemory_GetMemoryFile(save);
    v13 = MemFile_GetUsedSize(v12);
    ProfMem_Begin("misc", v13);
    SaveMemory_SaveWrite(&level.fFogOpaqueDist, 4, save);
    SaveMemory_SaveWrite(&level.fFogOpaqueDistSqrd, 4, save);
    SaveMemory_SaveWrite(&level.bDrawCompassFriendlies, 4, save);
    SaveMemory_SaveWrite(&sv_gameskill->current, 4, save);
    SaveMemory_SaveWrite(&g_player_maxhealth->current, 4, save);
    Sentient_WriteGlob(save);
    AimTarget_WriteSaveGame(save);
    Scr_SavePre(1);
    v14 = SaveMemory_GetMemoryFile(save);
    v15 = MemFile_GetUsedSize(v14);
    ProfMem_End(v15);
    v16 = SaveMemory_GetMemoryFile(save);
    v17 = MemFile_GetUsedSize(v16);
    ProfMem_Begin("path nodes", v17);
    SaveMemory_StartSegment(save, 3);
    Path_ValidateAllNodes();
    WritePathNodes(save);
    v18 = SaveMemory_GetMemoryFile(save);
    v19 = MemFile_GetUsedSize(v18);
    ProfMem_End(v19);
    v20 = SaveMemory_GetMemoryFile(save);
    v21 = MemFile_GetUsedSize(v20);
    ProfMem_Begin("entities", v21);
    SaveMemory_SaveWrite(&level.num_entities, 4, save);
    v22 = 0;
    i = 0;
    do
    {
        v23 = &g_entities[v22];
        if (v23->r.inuse)
        {
            SaveMemory_SaveWrite(&i, 4, save);
            WriteEntity(v23, save);
            v22 = i;
        }
        i = ++v22;
    } while (v22 < 2176);
    i = -1;
    SaveMemory_SaveWrite(&i, 4, save);
    v24 = SaveMemory_GetMemoryFile(save);
    v25 = MemFile_GetUsedSize(v24);
    ProfMem_End(v25);
    v26 = SaveMemory_GetMemoryFile(save);
    v27 = MemFile_GetUsedSize(v26);
    ProfMem_Begin("misc: clients, actors, vehicles", v27);
    WriteBadPlaces(save);
    v28 = 0;
    i = 0;
    do
    {
        v29 = &level.clients[v28];
        if (v29->pers.connected == CON_CONNECTED)
        {
            SaveMemory_SaveWrite(&i, 4, save);
            WriteClient(v29, save);
            v28 = i;
        }
        i = ++v28;
    } while (v28 < 1);
    i = -1;
    SaveMemory_SaveWrite(&i, 4, save);
    v30 = 0;
    for (i = 0; i < 32; ++i)
    {
        WriteActor(&level.actors[v30], save);
        v30 = i + 1;
    }
    v31 = 0;
    for (i = 0; i < 33; ++i)
    {
        WriteSentient(&level.sentients[v31], save);
        v31 = i + 1;
    }
    v32 = 0;
    for (i = 0; i < 64; ++i)
    {
        WriteVehicle(&level.vehicles[v32], save);
        v32 = i + 1;
    }
    G_SaveVehicleInfo(save);
    v33 = 0;
    for (i = 0; i < 32; ++i)
    {
        WriteTurretInfo(&level.turrets[v33], save);
        v33 = i + 1;
    }
    v34 = 0;
    for (i = 0; i < 16; ++i)
    {
        SaveMemory_SaveWrite(&g_scr_data.actorCorpseInfo[v34].entnum, 4, save);
        if (g_scr_data.actorCorpseInfo[i].entnum != -1)
            SaveMemory_SaveWrite(&g_scr_data.actorCorpseInfo[i].proneInfo, 24, save);
        v34 = i + 1;
    }
    DynEnt_SaveEntities(v8);
    memcpy(v63, &g_threatBias, 0x424u);
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1686, 0, "%s", "save");
    G_WriteStruct(threatGroupFields, (unsigned __int8 *)&g_threatBias, v63, 1060, save);
    v61[0] = Actor_EventListener_GetCount();
    SaveMemory_SaveWrite(v61, 4, save);
    if (v61[0])
        SaveMemory_SaveWrite(g_AIEVlisteners, 8 * v61[0], save);
    if (level.currentTriggerListSize)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            2285,
            0,
            "%s",
            "level.currentTriggerListSize == 0");
    SaveMemory_SaveWrite(&level.pendingTriggerListSize, 4, save);
    SaveMemory_SaveWrite(level.pendingTriggerList, 12 * level.pendingTriggerListSize, save);
    G_SaveWeaponCue(save);
    G_SaveConfigstrings(11, 16, save);
    G_SaveConfigstrings(27, 32, save);
    G_SaveConfigstrings(59, 32, save);
    Missile_SaveAttractors(v8);
    Cmd_SaveNotifications(v8);
    v35 = SaveMemory_GetMemoryFile(save);
    v36 = MemFile_GetUsedSize(v35);
    ProfMem_End(v36);
    v37 = SaveMemory_GetMemoryFile(save);
    v38 = MemFile_GetUsedSize(v37);
    ProfMem_Begin("Script", v38);
    SaveMemory_StartSegment(save, 4);
    Scr_SavePost(v8);
    v39 = SaveMemory_GetMemoryFile(save);
    v40 = MemFile_GetUsedSize(v39);
    ProfMem_End(v40);
    v41 = SaveMemory_GetMemoryFile(save);
    v42 = MemFile_GetUsedSize(v41);
    ProfMem_Begin("Animtree", v42);
    SaveMemory_StartSegment(save, 5);
    v43 = 0;
    i = 0;
    do
    {
        if (g_entities[v43].r.inuse)
        {
            ServerDObj = Com_GetServerDObj(v43);
            v45 = ServerDObj;
            if (ServerDObj)
            {
                XAnimSaveAnimTree(ServerDObj, v8);
                DObjGetHidePartBits(v45, v62);
                v46 = v62;
                v47 = 4;
                do
                {
                    v61[0] = *v46;
                    MemFile_WriteData(v8, 4, v61);
                    --v47;
                    ++v46;
                } while (v47);
            }
            v43 = i;
        }
        i = ++v43;
    } while (v43 < 2176);
    v48 = SaveMemory_GetMemoryFile(save);
    v49 = MemFile_GetUsedSize(v48);
    ProfMem_End(v49);
    Scr_SaveShutdown(savegame);
    G_CheckAllEntities();
    SV_BeginSaveGame();
    v50 = SaveMemory_GetMemoryFile(save);
    v51 = MemFile_GetUsedSize(v50);
    ProfMem_Begin("client", v51);
    v52 = SaveMemory_GetMemoryFile(save);
    v53 = MemFile_GetUsedSize(v52);
    ProfMem_Begin("clientState", v53);
    CG_SaveEntities(save);
    CL_ArchiveClientState(v8, 6);
    v54 = SaveMemory_GetMemoryFile(save);
    v55 = MemFile_GetUsedSize(v54);
    ProfMem_End(v55);
    CL_ArchiveServerCommands(v8);
    SV_SaveServerCommands(save);
    CG_SaveViewModelAnimTrees(save);
    Phys_ArchiveState(v8);
    SV_EndSaveGame();
    v56 = SaveMemory_GetMemoryFile(save);
    v57 = MemFile_GetUsedSize(v56);
    ProfMem_End(v57);
    v58 = SaveMemory_GetMemoryFile(save);
    v59 = MemFile_GetUsedSize(v58);
    ProfMem_End(v59);
}

void __cdecl G_SaveState(bool savegame, SaveGame *save)
{
    CM_ValidateWorld();
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2398, 0, "%s", "save");
    G_SaveInitState(save);
    G_SaveMainState(savegame, save);
}

int __cdecl G_IsSavePossible(SaveType saveType)
{
    int result; // r3

    if (saveType == SAVE_TYPE_INTERNAL)
        return 1;
    result = 0;
    if (g_entities[0].health > 0)
        return 1;
    return result;
}

int __cdecl G_WriteGame(const PendingSave *pendingSave, int checksum, SaveGame *save)
{
    MemoryFile *MemoryFile; // r3
    unsigned int UsedSize; // r3
    MemoryFile *v8; // r3
    MemoryFile *v9; // r3
    unsigned int v10; // r3
    MemoryFile *v12; // r3
    unsigned int v13; // r3
    SaveGame *v14; // [sp+8h] [-D8h]
    char v15[128]; // [sp+60h] [-80h] BYREF

    //Profile_Begin(242);
    if (pendingSave == (const PendingSave *)-64)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2432, 0, "%s", "pendingSave->description");
    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2433, 0, "%s", "pendingSave->filename");
    if (pendingSave == (const PendingSave *)-320)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2434, 0, "%s", "pendingSave->screenShotName");
    Com_Printf(10, "G_WriteGame '%s' '%s'\n", pendingSave->filename, pendingSave->description);
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2438, 0, "%s", "save");
    SaveMemory_InitializeGameSave(save);
    MemoryFile = SaveMemory_GetMemoryFile(save);
    UsedSize = MemFile_GetUsedSize(MemoryFile);
    ProfMem_Begin("Game Save", UsedSize);
    v8 = SaveMemory_GetMemoryFile(save);
    Scr_SaveSource(v8);
    G_SaveState(1, save);
    SaveMemory_StartSegment(save, -1);
    if (SaveMemory_IsSuccessful(save) && BuildCleanSavePath(v15, 0x40u, pendingSave->filename, pendingSave->saveType))
    {
        SaveMemory_CreateHeader(
            v15,
            pendingSave->description,
            pendingSave->screenShotName,
            checksum,
            0,
            pendingSave->suppressPlayerNotify,
            pendingSave->saveType,
            pendingSave->saveId,
            v14);
        SaveMemory_FinalizeSave(save);
        //Profile_EndInternal(0);
        v12 = SaveMemory_GetMemoryFile(save);
        v13 = MemFile_GetUsedSize(v12);
        ProfMem_End(v13);
        ProfMem_PrintTree();
        return 1;
    }
    else
    {
        SaveMemory_RollbackSave(save);
        //Profile_EndInternal(0);
        v9 = SaveMemory_GetMemoryFile(save);
        v10 = MemFile_GetUsedSize(v9);
        ProfMem_End(v10);
        ProfMem_PrintTree();
        return 0;
    }
}

void __cdecl G_WriteCurrentCommitToDevice()
{
    SaveGame *LastCommittedSave; // r3
    SaveGame *v1; // r31

    LastCommittedSave = SaveMemory_GetLastCommittedSave();
    v1 = LastCommittedSave;
    if (LastCommittedSave)
    {
        SaveMemory_GetHeader(LastCommittedSave);
        if (SaveMemory_WriteSaveToDevice(v1))
            SV_DisplaySaveErrorUI();
    }
}

void __cdecl G_PrepareSaveMemoryForWrite(char commitLevel)
{
    SaveGame *SaveHandle; // r30
    bool v3; // r31

    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2509, 0, "%s", "save");
    v3 = g_entities[0].health <= 0;
    Memcard_CheckOngoingTasks();
    if (SaveMemory_IsWaitingForCommit(SaveHandle) && !v3 && (commitLevel & 6) != 0)
        SaveMemory_ForceCommitSave(SaveHandle);
    if (SaveMemory_IsCurrentCommittedSaveValid() && (commitLevel & 4) != 0 && !SaveMemory_IsWrittenToDevice(SaveHandle))
        G_WriteCurrentCommitToDevice();
}

int __cdecl G_ProcessCommitActions(const PendingSave *pendingSave, SaveGame *save)
{
    int v4; // r31

    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2543, 0, "%s", "pendingSave");
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2544, 0, "%s", "save");
    v4 = 0;
    if ((pendingSave->commitLevel & 2) != 0)
        SaveMemory_ForceCommitSave(save);
    if ((pendingSave->commitLevel & 4) != 0)
    {
        if (pendingSave->saveType != SAVE_TYPE_AUTOSAVE)
            MemCard_SetUseDevDrive(1);
        v4 = SaveMemory_WriteSaveToDevice(save);
        if (pendingSave->saveType != SAVE_TYPE_AUTOSAVE)
            MemCard_SetUseDevDrive(0);
    }
    return v4;
}

int __cdecl G_SaveGame(const PendingSave *pendingSave, int checksum)
{
    char v4; // r11
    SaveGame *SaveHandle; // r31

    if (!pendingSave)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2581, 0, "%s", "pendingSave");
    if (pendingSave->saveType == SAVE_TYPE_INTERNAL || (v4 = 0, g_entities[0].health > 0))
        v4 = 1;
    if (!v4)
        return 0;
    G_PrepareSaveMemoryForWrite(pendingSave->commitLevel);
    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2588, 0, "%s", "save");
    if ((unsigned __int8)G_WriteGame(pendingSave, checksum, SaveHandle))
        return G_ProcessCommitActions(pendingSave, SaveHandle);
    else
        return 0;
}

bool __cdecl G_CommitSavedGame(int saveId)
{
    SaveGame *SaveHandle; // r31

    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    if (!SaveHandle)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2603, 0, "%s", "save");
    return SaveMemory_CommitSave(SaveHandle, saveId);
}

void __cdecl G_LoadItems(SaveGame *save)
{
    _BYTE v2[16]; // [sp+50h] [-20h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2627, 0, "%s", "save");
    SaveMemory_LoadRead(v2, 1, save);
    while (!v2[0])
    {
        ReadItemIndex(save);
        SaveMemory_LoadRead(v2, 1, save);
    }
}

void __cdecl G_SetPendingLoadName(const char *filename)
{
    I_strncpyz(g_pendingLoadName, filename, 64);
}

void __cdecl G_PreLoadGame(int checksum, int *useLoadedSourceFiles, SaveGame **save)
{
    void *LoadFromDevice; // r28
    const SaveHeader *Header; // r26
    MemoryFile *MemoryFile; // r3
    MemoryFile *v9; // r3
    MemoryFile *v10; // r3
    __int64 v11; // r10
    __int64 v12; // r8
    __int64 v13; // r6
    int v14; // [sp+8h] [-C8h]
    int v15; // [sp+Ch] [-C4h]
    int v16; // [sp+10h] [-C0h]
    int v17; // [sp+14h] [-BCh]
    int v18; // [sp+18h] [-B8h]
    int v19; // [sp+1Ch] [-B4h]
    int v20; // [sp+20h] [-B0h]
    int v21; // [sp+24h] [-ACh]
    int v22[24]; // [sp+70h] [-60h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2660, 0, "%s", "save");
    //Profile_Begin(244);
    MemCard_SetUseDevDrive(g_useDevSaveArea);
    if (SaveMemory_IsCommittedSaveAvailable(g_pendingLoadName, checksum))
    {
        LoadFromDevice = 0;
        *save = SaveMemory_GetLastCommittedSave();
    }
    else
    {
        LoadFromDevice = SaveMemory_ReadLoadFromDevice(g_pendingLoadName, checksum, *useLoadedSourceFiles, save);
    }
    Header = SaveMemory_GetHeader(*save);
    if (!Header->demoPlayback)
        SV_SetLastSaveName(g_pendingLoadName);
    SaveMemory_InitializeLoad(*save, Header->bodySize);
    if (Header->demoPlayback)
    {
        MemoryFile = SaveMemory_GetMemoryFile(*save);
        Dvar_LoadDvars(MemoryFile);
    }
    if (*useLoadedSourceFiles
        && (!LoadFromDevice
            || (Scr_GetChecksum(v22), v22[0] == Header->scrCheckSum[0])
            && v22[1] == Header->scrCheckSum[1]
            && v22[2] == Header->scrCheckSum[2]))
    {
        v10 = SaveMemory_GetMemoryFile(*save);
        Scr_SkipSource(v10, LoadFromDevice);
    }
    else
    {
        if (!LoadFromDevice)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2704, 0, "%s", "fileHandle");
        if (*useLoadedSourceFiles)
        {
            *useLoadedSourceFiles = 0;
            G_ClearLowHunk();
        }
        v9 = SaveMemory_GetMemoryFile(*save);
        Scr_LoadSource(v9, LoadFromDevice);
    }
    SaveMemory_MoveToSegment(*save, -1);
    if (!SaveMemory_IsSuccessful(*save))
    {
        HIDWORD(v13) = &unk_82035194;
        G_SaveError(ERR_DROP, SAVE_ERROR_CORRUPT_SAVE, v13, v12, v11, v14, v15, v16, v17, v18, v19, v20, v21);
    }
    if (Header->demoPlayback)
    {
        if (!LoadFromDevice)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2725, 0, "%s", "fileHandle");
        SV_LoadDemo(*save, LoadFromDevice);
    }
    if (LoadFromDevice)
        CloseDevice(LoadFromDevice);
    MemCard_SetUseDevDrive(0);
    //Profile_EndInternal(0);
}

int __cdecl G_LoadWeapons(SaveGame *save)
{
    int v2; // r29
    int v3; // r31
    _BYTE v5[4]; // [sp+50h] [-160h] BYREF
    int v6[3]; // [sp+54h] [-15Ch] BYREF
    char v7[336]; // [sp+60h] [-150h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2751, 0, "%s", "save");
    SaveMemory_LoadRead(v6, 4, save);
    v2 = 1;
    if (v6[0] <= 1)
        return 1;
    while (1)
    {
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 640, 0, "%s", "save");
        SaveMemory_LoadRead(v5, 1, save);
        v3 = v5[0];
        SaveMemory_LoadRead(v7, v5[0], save);
        v7[v3] = 0;
        if (BG_GetWeaponIndexForName(v7, G_RegisterWeapon) != v2)
            break;
        if (++v2 >= v6[0])
            return 1;
    }
    Com_Printf(10, "Weapon index mismatch for '%s'\n", v7);
    return 0;
}

void __cdecl G_InitLoadGame(SaveGame *save)
{
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2774, 0, "%s", "save");
    G_LoadInitConfigstrings(save);
}

void __cdecl G_LoadMainState(SaveGame *save)
{
    MemoryFile *MemoryFile; // r21
    MemoryFile *v3; // r3
    int i; // r5
    gentity_s *lastFreeEnt; // r10
    int v6; // r11
    int num_entities; // r9
    int v8; // r8
    int k; // r11
    gclient_s *v10; // r29
    int v11; // r11
    int v12; // r11
    int v13; // r11
    int v14; // r11
    TurretInfo *v15; // r29
    int v16; // r11
    int v17; // r11
    gentity_s *v18; // r29
    int eType; // r11
    unsigned int v20; // r3
    const char *v21; // r3
    DObj_s *ServerDObj; // r3
    DObj_s *v23; // r27
    unsigned int *v24; // r28
    int v25; // r29
    int j; // [sp+50h] [-90h] BYREF
    int v27[3]; // [sp+54h] [-8Ch] BYREF
    unsigned int v28[32]; // [sp+60h] [-80h] BYREF

    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2796, 0, "%s", "save");
    G_FreeEntities();
    HudElem_DestroyAll();
    G_FreePathnodesScriptInfo();
    G_FreeVehiclePathsScriptInfo();
    Actor_ClearThreatBiasGroups();
    Cmd_UnregisterAllNotifications();
    Scr_ShutdownSystem(1u, 1);
    Path_ValidateAllNodes();
    MemoryFile = SaveMemory_GetMemoryFile(save);
    level.initializing = 1;
    SaveMemory_MoveToSegment(save, 2);
    SaveMemory_LoadRead(&level.time, 4, save);
    SaveMemory_LoadRead(&level.framenum, 4, save);
    SaveMemory_LoadRead(&level.framePos, 4, save);
    SaveMemory_LoadRead(&level.soundAliasFirst, 2, save);
    SaveMemory_LoadRead(&level.soundAliasLast, 2, save);
    SaveMemory_LoadRead(&level.changelevel, 4, save);
    SaveMemory_LoadRead(&level.exitTime, 4, save);
    SaveMemory_LoadRead(&level.savepersist, 4, save);
    SaveMemory_LoadRead(&level.bMissionSuccess, 4, save);
    SaveMemory_LoadRead(&level.bMissionFailed, 4, save);
    SaveMemory_LoadRead(&level.scriptPrintChannel, 4, save);
    SaveMemory_LoadRead(g_nextMap, 64, save);
    SaveMemory_LoadRead(level.compassMapUpperLeft, 8, save);
    SaveMemory_LoadRead(level.compassMapWorldSize, 8, save);
    SaveMemory_LoadRead(level.compassNorth, 8, save);
    if (level.bPlayerIgnoreRadiusDamage)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            2837,
            0,
            "%s",
            "!level.bPlayerIgnoreRadiusDamage");
    SaveMemory_LoadRead(&level.bPlayerIgnoreRadiusDamageLatched, 4, save);
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2013, 0, "%s", "save");
    v3 = SaveMemory_GetMemoryFile(save);
    Dvar_LoadDvars(v3);
    SaveMemory_LoadRead(g_hudelems, 44032, save);
    SaveMemory_LoadRead(&level.fFogOpaqueDist, 4, save);
    SaveMemory_LoadRead(&level.fFogOpaqueDistSqrd, 4, save);
    SaveMemory_LoadRead(&level.bDrawCompassFriendlies, 4, save);
    SaveMemory_LoadRead(&j, 4, save);
    Dvar_SetInt(sv_gameskill, j);
    SaveMemory_LoadRead(&j, 4, save);
    Dvar_SetInt(g_player_maxhealth, j);
    Sentient_ReadGlob(save);
    AimTarget_ReadSaveGame(save);
    SaveMemory_MoveToSegment(save, 4);
    Scr_LoadPre(1, MemoryFile);
    SaveMemory_MoveToSegment(save, 3);
    ReadPathNodes(save);
    if (level.num_entities)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 2881, 0, "%s", "!level.num_entities");
    SaveMemory_LoadRead(&level.num_entities, 4, save);
    SaveMemory_LoadRead(&j, 4, save);
    for (i = j; j >= 0; i = j)
    {
        if (i >= 2176)
        {
            Com_Error(ERR_DROP, byte_8200713C);
            i = j;
        }
        ReadEntity(&g_entities[i], save);
        SaveMemory_LoadRead(&j, 4, save);
    }
    ReadBadPlaces(save);
    lastFreeEnt = 0;
    level.firstFreeEnt = 0;
    v6 = 1;
    level.lastFreeEnt = 0;
    num_entities = level.num_entities;
    for (j = 1; v6 < num_entities; j = v6)
    {
        v8 = v6;
        if (!g_entities[v6].r.inuse)
        {
            if (lastFreeEnt)
            {
                lastFreeEnt->nextFree = &g_entities[v8];
                v6 = j;
            }
            else
            {
                level.firstFreeEnt = &g_entities[v8];
            }
            level.lastFreeEnt = &g_entities[v6];
            level.lastFreeEnt->nextFree = 0;
            v6 = j;
            lastFreeEnt = level.lastFreeEnt;
            num_entities = level.num_entities;
        }
        ++v6;
    }
    Path_ValidateAllNodes();
    SaveMemory_LoadRead(&j, 4, save);
    for (k = j; j >= 0; k = j)
    {
        if (k > 1)
        {
            Com_Error(ERR_DROP, byte_820351E8);
            k = j;
        }
        v10 = &level.clients[k];
        if (v10->pers.connected == CON_DISCONNECTED)
            Com_Error(ERR_DROP, byte_8203520C);
        ReadClient(v10, save);
        SaveMemory_LoadRead(&j, 4, save);
    }
    v11 = 0;
    for (j = 0; j < 32; ++j)
    {
        ReadActor(&level.actors[v11], save);
        v11 = j + 1;
    }
    v12 = 0;
    for (j = 0; j < 33; ++j)
    {
        ReadSentient(&level.sentients[v12], save);
        v12 = j + 1;
    }
    v13 = 0;
    for (j = 0; j < 64; ++j)
    {
        ReadVehicle(&level.vehicles[v13], save);
        v13 = j + 1;
    }
    G_LoadVehicleInfo(save);
    v14 = 0;
    for (j = 0; j < 32; ++j)
    {
        v27[0] = 0;
        v15 = &level.turrets[v14];
        if (!save)
            MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 1527, 0, "%s", "save");
        SaveMemory_LoadRead(v27, 4, save);
        if (v27[0])
            G_ReadStruct(turretFields, (unsigned __int8 *)v15, 188, save);
        v14 = j + 1;
    }
    v16 = 0;
    for (j = 0; j < 16; ++j)
    {
        SaveMemory_LoadRead(&g_scr_data.actorCorpseInfo[v16].entnum, 4, save);
        if (g_scr_data.actorCorpseInfo[j].entnum != -1)
            SaveMemory_LoadRead(&g_scr_data.actorCorpseInfo[j].proneInfo, 24, save);
        v16 = j + 1;
    }
    level.actorCorpseCount = 16;
    DynEnt_LoadEntities(MemoryFile);
    G_ReadStruct(threatGroupFields, (unsigned __int8 *)&g_threatBias, 1060, save);
    if (Actor_EventListener_GetCount())
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            1724,
            0,
            "%s",
            "Actor_EventListener_GetCount() == 0");
    SaveMemory_LoadRead(v27, 4, save);
    Actor_EventListener_SetCount(v27[0]);
    if (v27[0])
        SaveMemory_LoadRead(g_AIEVlisteners, 8 * v27[0], save);
    if (level.currentTriggerListSize)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp",
            2969,
            0,
            "%s",
            "level.currentTriggerListSize == 0");
    SaveMemory_LoadRead(&level.pendingTriggerListSize, 4, save);
    SaveMemory_LoadRead(level.pendingTriggerList, 12 * level.pendingTriggerListSize, save);
    G_LoadWeaponCue(save);
    G_LoadConfigstrings(11, 16, save);
    G_LoadConfigstrings(27, 32, save);
    G_LoadConfigstrings(59, 32, save);
    G_LoadTargets();
    Missile_LoadAttractors(MemoryFile);
    Cmd_LoadNotifications(MemoryFile);
    SaveMemory_MoveToSegment(save, 5);
    v17 = 0;
    j = 0;
    do
    {
        v18 = &g_entities[v17];
        if (v18->r.inuse)
        {
            if (v18->model)
            {
                if (G_XModelBad(v18->model))
                {
                    eType = v18->s.eType;
                    if (eType == 14 || eType == 16)
                    {
                        v20 = G_ModelName(v18->model);
                        v21 = SL_ConvertToString(v20);
                        Com_PrintWarning(
                            10,
                            "WARNING: actor model '%s' couldn't be found! switching to default actor model.\n",
                            v21);
                        G_OverrideModel(v18->model, "defaultactor");
                    }
                }
            }
            G_DObjUpdate(v18);
            ServerDObj = Com_GetServerDObj(j);
            v23 = ServerDObj;
            if (ServerDObj)
            {
                XAnimLoadAnimTree(ServerDObj, MemoryFile);
                v24 = v28;
                v25 = 4;
                do
                {
                    MemFile_ReadData(MemoryFile, 4, v27);
                    --v25;
                    *v24++ = v27[0];
                } while (v25);
                DObjSetHidePartBits(v23, v28);
            }
            v17 = j;
        }
        j = ++v17;
    } while (v17 < 2176);
    SV_SendGameState();
    CG_LoadEntities(save);
    CL_ArchiveClientState(MemoryFile, 6);
    CL_LoadServerCommands(save);
    SV_LoadServerCommands(save);
    CG_LoadViewModelAnimTrees(save, &level.clients->ps);
    Phys_ArchiveState(MemoryFile);
    SaveMemory_MoveToSegment(save, -1);
    Scr_LoadShutdown();
    SV_LocateGameData(level.gentities, level.num_entities, 628, &level.clients->ps, 46104);
    level.initializing = 0;
}

void __cdecl G_LoadGame(int checksum, SaveGame *save)
{
    const SaveHeader *Header; // r31
    __int64 v4; // r10
    __int64 v5; // r8
    __int64 v6; // r6
    long double v7; // fp2
    int v8; // [sp+8h] [-A8h]
    int v9; // [sp+Ch] [-A4h]
    int v10; // [sp+10h] [-A0h]
    int v11; // [sp+14h] [-9Ch]
    int v12; // [sp+18h] [-98h]
    int v13; // [sp+1Ch] [-94h]
    int v14; // [sp+20h] [-90h]
    int v15; // [sp+24h] [-8Ch]
    unsigned int RandomSeed; // [sp+70h] [-40h] BYREF
    int v17[14]; // [sp+78h] [-38h] BYREF

    Com_Printf(10, "=== G_LoadGame ===\n");
    //Profile_Begin(244);
    R_Cinematic_UnsetNextPlayback();
    if (!save)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 3067, 0, "%s", "save");
    Header = SaveMemory_GetHeader(save);
    if (!Header)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\g_save.cpp", 3069, 0, "%s", "header");
    if (Header->isUsingScriptChecksum)
    {
        Scr_GetChecksum(v17);
        if (v17[0] != Header->scrCheckSum[0] || v17[1] != Header->scrCheckSum[1] || v17[2] != Header->scrCheckSum[2])
            Com_Error(ERR_DROP, byte_82035250, Header->filename, Header->buildNumber);
    }
    if (Header->saveCheckSum != SaveMemory_CalculateChecksum(save))
    {
        HIDWORD(v6) = &unk_82035194;
        G_SaveError(ERR_DROP, SAVE_ERROR_CORRUPT_SAVE, v6, v5, v4, v8, v9, v10, v11, v12, v13, v14, v15);
    }
    G_LoadMainState(save);
    SaveMemory_FinalizeLoad(save);
    RandomSeed = G_GetRandomSeed();
    level.demoplaying = SV_InitDemo((int *)&RandomSeed);
    G_srand(RandomSeed);
    if (!SV_UsingDemoSave())
    {
        SV_GetUsercmd(0, &level.clients->pers.cmd);
        InitClientDeltaAngles(level.clients, v7);
    }
    G_PruneLoadedCorpses();
    //Profile_EndInternal(0);
}

int __cdecl G_LoadErrorCleanup()
{
    SaveGame *SaveHandle; // r31
    SaveGame *v1; // r30

    SaveHandle = SaveMemory_GetSaveHandle(SAVE_GAME_HANDLE);
    v1 = SaveMemory_GetSaveHandle(SAVE_DEMO_HANDLE);
    if (!SaveMemory_IsLoading(SaveHandle) && !SaveMemory_IsLoading(v1))
        return 0;
    Com_InitDObj();
    SaveMemory_CleanupSaveMemory();
    return 1;
}

