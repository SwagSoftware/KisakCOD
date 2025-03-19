#pragma once


inline const char *__cdecl _Cmd_Argv(int argIndex)
{
    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\../qcommon/cmd.h",
            174,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
    if (argIndex < 0)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\../qcommon/cmd.h",
            175,
            0,
            "%s\n\t(argIndex) = %i",
            "(argIndex >= 0)",
            argIndex);
    if (argIndex >= cmd_args.argc[cmd_args.nesting])
        return "";
    else
        return cmd_args.argv[cmd_args.nesting][argIndex];
}

inline int __cdecl _Cmd_Argc()
{
    if (cmd_args.nesting >= 8u)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\cgame_mp\\../qcommon/cmd.h",
            160,
            0,
            "cmd_args.nesting doesn't index CMD_MAX_NESTING\n\t%i not in [0, %i)",
            cmd_args.nesting,
            8);
    return cmd_args.argc[cmd_args.nesting];
}