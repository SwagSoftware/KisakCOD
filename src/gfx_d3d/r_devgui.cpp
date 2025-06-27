#include "r_devgui.h"
#include <qcommon/cmd.h>
#include <qcommon/mem_track.h>

void __cdecl R_CreateDevGui()
{
	Cbuf_InsertText(0, "exec devgui_renderer");
	Cbuf_InsertText(0, "exec devgui_visibility");
	KISAK_NULLSUB();
}