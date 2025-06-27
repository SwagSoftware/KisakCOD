#include "client_mp.h"


int __cdecl CL_GetRankForXp(int xp)
{
  const char *ColumnValueForRow; // eax
  const char *v2; // eax
  const char *v3; // eax
  int toprow; // [esp+0h] [ebp-18h]
  const StringTable *table; // [esp+4h] [ebp-14h] BYREF
  int maxxp; // [esp+8h] [ebp-10h]
  int row; // [esp+Ch] [ebp-Ch]
  int bottomrow; // [esp+10h] [ebp-8h]
  int minxp; // [esp+14h] [ebp-4h]

  StringTable_GetAsset("mp/rankTable.csv", (XAssetHeader *)&table);
  toprow = table->rowCount - 1;
  bottomrow = 0;
  row = toprow / 2;
  while ( toprow > bottomrow )
  {
    ColumnValueForRow = StringTable_GetColumnValueForRow(table, row, 2);
    minxp = atoi(ColumnValueForRow);
    v2 = StringTable_GetColumnValueForRow(table, row, 7);
    maxxp = atoi(v2);
    if ( maxxp < minxp )
      MyAssertHandler(".\\client_mp\\cl_rank.cpp", 30, 0, "%s", "maxxp >= minxp");
    if ( xp >= minxp )
    {
      if ( xp < maxxp )
        break;
      bottomrow = row + 1;
    }
    else
    {
      toprow = row - 1;
    }
    row = bottomrow + (toprow - bottomrow) / 2;
  }
  if ( row < 0 || row >= table->rowCount )
    MyAssertHandler(".\\client_mp\\cl_rank.cpp", 41, 0, "%s\n\t(row) = %i", "(row >= 0 && row < table->rowCount)", row);
  v3 = StringTable_GetColumnValueForRow(table, row, 0);
  return atoi(v3);
}

const char *__cdecl CL_GetRankData(int rank, rankTableColumns_t column)
{
  const StringTable *table; // [esp+4h] [ebp-8h] BYREF
  char level[4]; // [esp+8h] [ebp-4h] BYREF

  if ( (unsigned int)column >= MP_RANKTABLE_COUNT )
    MyAssertHandler(
      ".\\client_mp\\cl_rank.cpp",
      53,
      0,
      "column doesn't index MP_RANKTABLE_COUNT\n\t%i not in [0, %i)",
      column,
      15);
  StringTable_GetAsset("mp/rankTable.csv", (XAssetHeader *)&table);
  if ( !table )
    MyAssertHandler(".\\client_mp\\cl_rank.cpp", 56, 0, "%s", "table");
  Com_sprintf(level, 4u, "%i", rank);
  return StringTable_Lookup(table, 0, level, column);
}

void __cdecl CL_GetRankIcon(int rank, int prestige, Material **handle)
{
  const StringTable *table; // [esp+0h] [ebp-Ch] BYREF
  const char *rankIconName; // [esp+4h] [ebp-8h]
  char id[4]; // [esp+8h] [ebp-4h] BYREF

  if ( rank < 0 )
    MyAssertHandler(".\\client_mp\\cl_rank.cpp", 71, 0, "%s\n\t(rank) = %i", "(rank >= 0)", rank);
  if ( prestige < 0 )
    MyAssertHandler(".\\client_mp\\cl_rank.cpp", 72, 0, "%s\n\t(prestige) = %i", "(prestige >= 0)", prestige);
  if ( !handle )
    MyAssertHandler(".\\client_mp\\cl_rank.cpp", 73, 0, "%s", "handle");
  StringTable_GetAsset("mp/rankIconTable.csv", (XAssetHeader *)&table);
  if ( !table )
    MyAssertHandler(".\\client_mp\\cl_rank.cpp", 76, 0, "%s", "table");
  Com_sprintf(id, 4u, "%i", rank);
  rankIconName = StringTable_Lookup(table, 0, id, prestige + 1);
  *handle = Material_RegisterHandle((char *)rankIconName, 7);
  if ( Material_IsDefault(*handle) )
    *handle = 0;
}

