#include "q_shared.h"
#include <qcommon/qcommon.h>
#include <database/database.h>


const char *__cdecl StringTable_GetColumnValueForRow(const StringTable *table, int row, int column)
{
    if (column < table->columnCount
        && row < table->rowCount
        && row >= 0
        && column >= 0
        && (&table->values[column])[table->columnCount * row])
    {
        return (&table->values[column])[table->columnCount * row];
    }
    else
    {
        return "";
    }
}

const char *__cdecl StringTable_Lookup(
    const StringTable *table,
    int comparisonColumn,
    const char *value,
    int valueColumn)
{
    int v5; // eax

    if (table)
    {
        v5 = StringTable_LookupRowNumForValue(table, comparisonColumn, value);
        return StringTable_GetColumnValueForRow(table, v5, valueColumn);
    }
    else
    {
        Com_PrintError(13, "Unable to find the lookup table in the fastfile, aborting lookup\n");
        return "";
    }
}

int __cdecl StringTable_LookupRowNumForValue(const StringTable *table, int comparisonColumn, const char *value)
{
    int slot; // [esp+0h] [ebp-8h]
    int row; // [esp+4h] [ebp-4h]

    if (!table->columnCount)
        return -1;
    if (comparisonColumn >= table->columnCount)
        Com_Error(
            ERR_DROP,
            "Unable to compare against column number %i - there are only %i columns",
            comparisonColumn,
            table->columnCount);
    for (row = table->rowCount - 1; row >= 0; --row)
    {
        slot = comparisonColumn + table->columnCount * row;
        if (table->values[slot] && !I_stricmp(value, table->values[slot]))
            return row;
    }
    return -1;
}

void __cdecl StringTable_GetAsset(const char *filename, XAssetHeader *tablePtr)
{
    if (!useFastFile->current.enabled)
        Com_Error(ERR_DROP, "Trying to use a string table with fast file loading disabled.");
    tablePtr->xmodelPieces = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, filename).xmodelPieces;
}

