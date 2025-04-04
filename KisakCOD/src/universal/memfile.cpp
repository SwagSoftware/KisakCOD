#include <qcommon/qcommon.h>

#include "memfile.h"

void MemFile_ArchiveData(MemoryFile* memFile, int bytes, void* data)
{
    iassert(memFile);
    iassert(memFile->archiveProc);

    memFile->archiveProc(memFile, bytes, data);
}