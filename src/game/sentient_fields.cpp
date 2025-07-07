#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl SentientScr_ReadOnly(sentient_s *pSelf, const sentient_fields_s *pField)
{
    const char *v3; // r3

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp", 48, 0, "%s", "pSelf");
    v3 = va("sentient property '%s' is read-only", pField->name);
    Scr_Error(v3);
}

void __cdecl SentientScr_SetTeam(sentient_s *pSelf, const sentient_fields_s *pField)
{
    const char *String; // r31
    const char *v4; // r3

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp", 60, 0, "%s", "pSelf");
    String = Scr_GetString(0);
    if (I_stricmp(String, "axis"))
    {
        if (I_stricmp(String, "allies"))
        {
            if (I_stricmp(String, "neutral"))
            {
                v4 = va("unknown team '%s', should be axis, allies, or neutral\n", String);
                Scr_Error(v4);
            }
            else
            {
                Sentient_SetTeam(pSelf, TEAM_NEUTRAL);
            }
        }
        else
        {
            Sentient_SetTeam(pSelf, TEAM_ALLIES);
        }
    }
    else
    {
        Sentient_SetTeam(pSelf, TEAM_AXIS);
    }
}

void __cdecl SentientScr_GetTeam(sentient_s *pSelf, const sentient_fields_s *pField)
{
    team_t eTeam; // r11

    if (!pSelf)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp", 82, 0, "%s", "pSelf");
    eTeam = pSelf->eTeam;
    if (eTeam <= TEAM_FREE || eTeam >= TEAM_NUM_TEAMS)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp",
            83,
            0,
            "%s",
            "pSelf->eTeam > TEAM_BAD && pSelf->eTeam < TEAM_NUM_TEAMS");
    switch (pSelf->eTeam)
    {
    case TEAM_AXIS:
        Scr_AddString("axis");
        break;
    case TEAM_ALLIES:
        Scr_AddString("allies");
        break;
    case TEAM_NEUTRAL:
        Scr_AddString("neutral");
        break;
    case TEAM_DEAD:
        Scr_AddString("dead");
        break;
    default:
        if (!alwaysfails)
            MyAssertHandler(
                "c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp",
                100,
                0,
                "SentientScr_GetTeam: default case (shouldn't happen");
        break;
    }
}

void __cdecl GScr_AddFieldsForSentient()
{
    const sentient_fields_s *v0; // r25
    int v1; // r27

    v0 = fields_2;
    if ("team")
    {
        v1 = 0;
        do
        {
            if (((v1 / 20) & 0xC000) != 0)
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp",
                    119,
                    0,
                    "%s",
                    "!((f - fields) & ENTFIELD_MASK)");
            if (v1 / 20 != (unsigned __int16)(v1 / 20))
                MyAssertHandler(
                    "c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp",
                    120,
                    0,
                    "%s",
                    "(f - fields) == (unsigned short)( f - fields )");
            Scr_AddClassField(0, v0->name, (unsigned __int16)(v1 / 20) | 0x4000);
            ++v0;
            v1 += 20;
        } while (v0->name);
    }
}

void __cdecl Scr_SetSentientField(sentient_s *sentient, unsigned int offset)
{
    const sentient_fields_s *v4; // r4
    void(__cdecl * setter)(sentient_s *, const sentient_fields_s *); // r11

    if (!sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp", 135, 0, "%s", "sentient");
    if (offset >= 0xD)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp",
            136,
            0,
            "%s",
            "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
    v4 = &fields_2[offset];
    setter = v4->setter;
    if (setter)
        ((void(__cdecl *)(sentient_s *))setter)(sentient);
    else
        Scr_SetGenericField((unsigned __int8 *)sentient, v4->type, v4->ofs);
}

void __cdecl Scr_GetSentientField(sentient_s *sentient, unsigned int offset)
{
    const sentient_fields_s *v4; // r4
    void(__cdecl * getter)(sentient_s *, const sentient_fields_s *); // r11

    if (!sentient)
        MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp", 159, 0, "%s", "sentient");
    if (offset >= 0xD)
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\sentient_fields.cpp",
            160,
            0,
            "%s",
            "(unsigned)offset < ARRAY_COUNT( fields ) - 1");
    v4 = &fields_2[offset];
    getter = v4->getter;
    if (getter)
        ((void(__cdecl *)(sentient_s *))getter)(sentient);
    else
        Scr_GetGenericField((unsigned __int8 *)sentient, v4->type, v4->ofs);
}

