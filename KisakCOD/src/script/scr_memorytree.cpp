#include "scr_stringlist.h"
#include <win32/win_local.h>

struct scrMemTreePub_t scrMemTreePub;
int marker_scr_memorytree;

int scrMemTreeGlob[];

void MT_Init()
{
	// KISAKFINISHFUNCTION
	Sys_EnterCriticalSection(CRITSECT_ALLOC_MARK);








	Sys_LeaveCriticalSection(CRITSECT_ALLOC_MARK);
}