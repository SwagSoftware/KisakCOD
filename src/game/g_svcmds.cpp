#include "game_public.h"
#include <qcommon/cmd.h>
#include <server/sv_game.h>
#include <game_mp/g_utils_mp.h>

ipFilter_s ipFilters[1024];
int numIPFilters;

void __cdecl G_ProcessIPBans()
{
    char *v0; // eax
    char str[1028]; // [esp+0h] [ebp-410h] BYREF
    char *t; // [esp+408h] [ebp-8h]
    char *s; // [esp+40Ch] [ebp-4h]

    numIPFilters = 0;
    I_strncpyz(str, (char *)g_banIPs->current.integer, 1024);
    s = str;
    for (t = str; *t; t = s)
    {
        v0 = strchr(s, 0x20u);
        s = v0;
        if (!v0)
            break;
        while (*s == 32)
            *s++ = 0;
        if (*t)
            AddIP(t);
    }
}

void __cdecl AddIP(char *str)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < numIPFilters && ipFilters[i].compare != -1; ++i)
        ;
    if (i == numIPFilters)
    {
        if (numIPFilters == 1024)
        {
            Com_Printf(15, "IP filter list is full\n");
            return;
        }
        ++numIPFilters;
    }
    if (!StringToFilter(str, &ipFilters[i]))
        ipFilters[i].compare = -1;
    UpdateIPBans();
}

int __cdecl StringToFilter(char *s, ipFilter_s *f)
{
    unsigned __int8 v3; // al
    int j; // [esp+0h] [ebp-94h]
    unsigned __int8 m[4]; // [esp+4h] [ebp-90h]
    unsigned __int8 b[4]; // [esp+8h] [ebp-8Ch]
    char num[128]; // [esp+Ch] [ebp-88h] BYREF
    int i; // [esp+90h] [ebp-4h]

    for (i = 0; i < 4; ++i)
    {
        b[i] = 0;
        m[i] = 0;
    }
    for (i = 0; i < 4; ++i)
    {
        if (*s < 48 || *s > 57)
        {
            Com_Printf(15, "Bad filter address: %s\n", s);
            return 0;
        }
        j = 0;
        while (*s >= 48 && *s <= 57)
            num[j++] = *s++;
        num[j] = 0;
        v3 = atoi(num);
        b[i] = v3;
        if (b[i])
            m[i] = -1;
        if (!*s)
            break;
        ++s;
    }
    f->mask = *(unsigned int *)m;
    f->compare = *(unsigned int *)b;
    return 1;
}

void UpdateIPBans()
{
    unsigned int b; // [esp+20h] [ebp-410h]
    int i; // [esp+24h] [ebp-40Ch]
    char iplist[1028]; // [esp+28h] [ebp-408h] BYREF

    iplist[0] = 0;
    for (i = 0; i < numIPFilters; ++i)
    {
        if (ipFilters[i].compare != -1)
        {
            b = ipFilters[i].compare;
            Com_sprintf(
                &iplist[&iplist[strlen(iplist) + 1] - &iplist[1]],
                1024 - (&iplist[strlen(iplist) + 1] - &iplist[1]),
                "%i.%i.%i.%i ",
                (unsigned __int8)b,
                BYTE1(b),
                BYTE2(b),
                HIBYTE(b));
        }
    }
    Dvar_SetString((dvar_s *)g_banIPs, iplist);
}

void __cdecl Svcmd_AddIP_f()
{
    char str[1028]; // [esp+0h] [ebp-408h] BYREF

    if (SV_Cmd_Argc() >= 2)
    {
        SV_Cmd_ArgvBuffer(1, str, 1024);
        AddIP(str);
    }
    else
    {
        Com_Printf(0, "Usage:  addip <ip-mask>\n");
    }
}

void __cdecl Svcmd_RemoveIP_f()
{
    char str[1024]; // [esp+0h] [ebp-410h] BYREF
    ipFilter_s f; // [esp+404h] [ebp-Ch] BYREF
    int i; // [esp+40Ch] [ebp-4h]

    if (SV_Cmd_Argc() >= 2)
    {
        SV_Cmd_ArgvBuffer(1, str, 1024);
        if (StringToFilter(str, &f))
        {
            for (i = 0; i < numIPFilters; ++i)
            {
                if (ipFilters[i].mask == f.mask && ipFilters[i].compare == f.compare)
                {
                    ipFilters[i].compare = -1;
                    Com_Printf(0, "Removed.\n");
                    UpdateIPBans();
                    return;
                }
            }
            Com_Printf(0, "Didn't find %s.\n", str);
        }
    }
    else
    {
        Com_Printf(0, "Usage:  sv removeip <ip-mask>\n");
    }
}

void __cdecl Svcmd_EntityList_f()
{
    const char *EntityTypeName; // eax
    int e; // [esp+0h] [ebp-8h]
    gentity_s *check; // [esp+4h] [ebp-4h]

    check = &g_entities[1];
    for (e = 1; e < level.num_entities; ++e)
    {
        if (check->r.inuse)
        {
            Com_Printf(0, "%3i: ", e);
            EntityTypeName = G_GetEntityTypeName(check);
            Com_Printf(0, "'%s'", EntityTypeName);
            if (check->classname)
            {
                Com_Printf(0, ", '%s'", SL_ConvertToString(check->classname));
            }
            Com_Printf(0, "\n");
        }
        ++check;
    }
}

int __cdecl ConsoleCommand()
{
#ifdef KISAK_MP
    char *v1; // eax
    const char *v2; // eax
    const char *cmd; // [esp+0h] [ebp-4h]

    cmd = Cmd_Argv(0);
    if (I_stricmp(cmd, "entitylist"))
    {
        if (I_stricmp(cmd, "addip"))
        {
            if (I_stricmp(cmd, "removeip"))
            {
                if (I_stricmp(cmd, "listip"))
                {
                    if (!g_dedicated->current.integer || I_stricmp(cmd, "say"))
                    {
                        return 0;
                    }
                    else
                    {
                        v1 = ConcatArgs(1);
                        v2 = va("%c \"GAME_SERVER: %s\"", 101, v1);
                        SV_GameSendServerCommand(-1, SV_CMD_CAN_IGNORE, v2);
                        return 1;
                    }
                }
                else
                {
                    Cbuf_InsertText(0, "g_banIPs\n");
                    return 1;
                }
            }
            else
            {
                Svcmd_RemoveIP_f();
                return 1;
            }
        }
        else
        {
            Svcmd_AddIP_f();
            return 1;
        }
    }
    else
    {
        Svcmd_EntityList_f();
        return 1;
    }
#elif KISAK_SP
    int nesting; // r7
    const char *v1; // r3

    nesting = cmd_args.nesting;
    if (cmd_args.nesting >= 8u)
    {
        MyAssertHandler(
            "c:\\trees\\cod3\\cod3src\\src\\game\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
        nesting = cmd_args.nesting;
    }
    if (cmd_args.argc[nesting] <= 0)
        v1 = "";
    else
        v1 = *cmd_args.argv[nesting];
    if (I_stricmp(v1, "entitylist"))
        return 0;
    Svcmd_EntityList_f();
    return 1;
#endif
}

