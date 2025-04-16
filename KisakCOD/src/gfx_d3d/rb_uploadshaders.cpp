#include "rb_uploadshaders.h"
#include <qcommon/threads.h>
#include "r_dvars.h"

struct $C28D828B354D71D7584331F40DBDE744 // sizeof=0x100C
{                                       // ...
    MaterialTechniqueSet *techSet[1024]; // ...
    unsigned int techTypeIter;          // ...
    volatile unsigned int get;          // ...
    volatile unsigned int put;          // ...
};

$C28D828B354D71D7584331F40DBDE744 mtlUploadGlob;

void __cdecl Material_UploadShaders(MaterialTechniqueSet *techSet)
{
    if (!Sys_IsDatabaseThread())
        MyAssertHandler(".\\rb_uploadshaders.cpp", 26, 0, "%s", "Sys_IsDatabaseThread()");
    if (mtlUploadGlob.put - mtlUploadGlob.get >= 0x400)
        MyAssertHandler(
            ".\\rb_uploadshaders.cpp",
            27,
            0,
            "%s",
            "mtlUploadGlob.put - mtlUploadGlob.get < MAX_TECHNIQUE_SETS");
    if (r_preloadShaders->current.enabled)
    {
        techSet->hasBeenUploaded = 0;
        mtlUploadGlob.techSet[mtlUploadGlob.put % 0x400] = techSet;
        ++mtlUploadGlob.put;
    }
}