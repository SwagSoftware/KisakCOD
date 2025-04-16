#include "database.h"


void __cdecl Load_Stream(bool atStreamStart, unsigned __int8 *ptr, int size)
{
    if (atStreamStart != (ptr == DB_GetStreamPos()))
        MyAssertHandler(".\\database\\db_stream_load.cpp", 18, 0, "%s", "atStreamStart == (ptr == DB_GetStreamPos())");
    if (atStreamStart && size)
    {
        if (g_streamPosIndex - 1 < 3)
        {
            if (g_streamPosIndex == 1)
            {
                memset(ptr, 0, size);
            }
            else
            {
                if (g_streamDelayIndex >= 0x1000)
                    MyAssertHandler(
                        ".\\database\\db_stream_load.cpp",
                        33,
                        0,
                        "g_streamDelayIndex doesn't index ARRAY_COUNT( g_streamDelayArray )\n\t%i not in [0, %i)",
                        g_streamDelayIndex,
                        4096);
                g_streamDelayArray[g_streamDelayIndex].ptr = ptr;
                g_streamDelayArray[g_streamDelayIndex++].size = size;
            }
        }
        else
        {
            DB_LoadXFileData(ptr, size);
        }
        DB_IncStreamPos(size);
    }
}

void __cdecl Load_DelayStream()
{
    unsigned int index; // [esp+4h] [ebp-8h]

    for (index = 0; index < g_streamDelayIndex; ++index)
        DB_LoadXFileData((unsigned __int8 *)g_streamDelayArray[index].ptr, dword_13B927C[2 * index]);
}

void __cdecl DB_ConvertOffsetToAlias(unsigned int *data)
{
    unsigned int offset; // [esp+0h] [ebp-8h]

    offset = *data;
    if (!*data || offset >= 0xFFFFFFFE)
        MyAssertHandler(
            ".\\database\\db_stream_load.cpp",
            64,
            0,
            "%s\n\t(offset) = %i",
            "(offset && (offset != 0xFFFFFFFF) && (offset != 0xFFFFFFFE))",
            offset);
    *data = *(unsigned int *)&g_streamZoneMem->blocks[(offset - 1) >> 28].data[(offset - 1) & 0xFFFFFFF];
}

void __cdecl DB_ConvertOffsetToPointer(unsigned int *data)
{
    *data = &g_streamZoneMem->blocks[(unsigned int)(*data - 1) >> 28].data[(*data - 1) & 0xFFFFFFF];
}

void __cdecl Load_XStringCustom(char **str)
{
    unsigned __int8 *pos; // [esp+0h] [ebp-8h]
    char *s; // [esp+4h] [ebp-4h]

    s = *str;
    for (pos = (unsigned __int8 *)*str; ; ++pos)
    {
        DB_LoadXFileData(pos, 1u);
        if (!*pos)
            break;
    }
    DB_IncStreamPos(pos - (unsigned __int8 *)s + 1);
}

void __cdecl Load_TempStringCustom(char **str)
{
    HashEntry_unnamed_type_u v1; // [esp+0h] [ebp-4h]

    Load_XStringCustom(str);
    if (*str)
        v1.prev = SL_GetString(*str, 4u).prev;
    else
        v1.prev = 0;
    *str = (char *)v1.prev;
}

