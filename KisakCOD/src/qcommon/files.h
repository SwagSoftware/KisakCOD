#pragma once

enum FS_SERVER_COMPARE_RESULT : __int32
{                                       // ...
    FILES_MATCH = 0x0,
    NEED_DOWNLOAD = 0x1,
    NOT_DOWNLOADABLE = 0x2,
};

char *__cdecl FS_GetMapBaseName(char *mapname);
FS_SERVER_COMPARE_RESULT __cdecl FS_CompareWithServerFiles(char *neededFiles, int len, int dlstring);
void __cdecl FS_PureServerSetLoadedIwds(char *iwdSums, char *iwdNames);
void __cdecl FS_ServerSetReferencedIwds(char *iwdSums, char *iwdNames);
void __cdecl FS_ServerSetReferencedFFs(char *FFSums, char *FFNames);