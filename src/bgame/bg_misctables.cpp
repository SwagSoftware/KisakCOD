#include "bg_local.h"
#include "bg_public.h"
#include <qcommon/mem_track.h>

gitem_s bg_itemlist[2048];
int itemRegistered[2048];

void __cdecl TRACK_bg_misctables()
{
	track_static_alloc_internal(bg_itemlist, 0x2000, "bg_itemlist", 9);
}