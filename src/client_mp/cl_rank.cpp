#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include "client_mp.h"

int __cdecl CL_GetRankForXp(int xp)
{
  const char *ColumnValueForRow; // eax
  const char *v2; // eax
  const char *v3; // eax
  int toprow; // [esp+0h] [ebp-18h]
  StringTable *table; // [esp+4h] [ebp-14h] BYREF
  int maxxp; // [esp+8h] [ebp-10h]
  int row; // [esp+Ch] [ebp-Ch]
  int bottomrow; // [esp+10h] [ebp-8h]
  int minxp; // [esp+14h] [ebp-4h]

  StringTable_GetAsset("mp/rankTable.csv", &table);
  toprow = table->rowCount - 1;
  bottomrow = 0;
  row = toprow / 2;
  while ( toprow > bottomrow )
  {
    ColumnValueForRow = StringTable_GetColumnValueForRow(table, row, 2);
    minxp = atoi(ColumnValueForRow);
    v2 = StringTable_GetColumnValueForRow(table, row, 7);
    maxxp = atoi(v2);

    iassert(maxxp >= minxp);

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

  iassert(row >= 0 && row < table->rowCount);

  v3 = StringTable_GetColumnValueForRow(table, row, 0);
  return atoi(v3);
}

const char *__cdecl CL_GetRankData(int rank, rankTableColumns_t column)
{
  StringTable *table; // [esp+4h] [ebp-8h] BYREF
  char level[4]; // [esp+8h] [ebp-4h] BYREF

  bcassert(column, MP_RANKTABLE_COUNT);
  StringTable_GetAsset("mp/rankTable.csv", &table);
  iassert(table);
  Com_sprintf(level, 4u, "%i", rank);
  return StringTable_Lookup(table, 0, level, column);
}

void __cdecl CL_GetRankIcon(int rank, int prestige, Material **handle)
{
  StringTable *table; // [esp+0h] [ebp-Ch] BYREF
  const char *rankIconName; // [esp+4h] [ebp-8h]
  char id[4]; // [esp+8h] [ebp-4h] BYREF

  iassert(rank >= 0);
  iassert(prestige >= 0);
  iassert(handle);

  StringTable_GetAsset("mp/rankIconTable.csv", &table);

  iassert(table);

  Com_sprintf(id, 4u, "%i", rank);
  rankIconName = StringTable_Lookup(table, 0, id, prestige + 1);
  *handle = Material_RegisterHandle((char *)rankIconName, 7);
  if ( Material_IsDefault(*handle) )
    *handle = 0;
}