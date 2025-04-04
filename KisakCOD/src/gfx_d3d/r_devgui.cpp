#include "r_devgui.h"
#include <qcommon/cmd.h>
#include <qcommon/mem_track.h>

devguiGlob_t devguiGlob;

void __cdecl TRACK_devgui()
{
	track_static_alloc_internal(&devguiGlob, 24080, "devguiGlob", 0);
}

void __cdecl R_CreateDevGui()
{
	Cbuf_InsertText(0, "exec devgui_renderer");
	Cbuf_InsertText(0, "exec devgui_visibility");
	CL_ResetStats_f();
}