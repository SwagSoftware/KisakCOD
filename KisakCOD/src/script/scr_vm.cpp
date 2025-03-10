#include "scr_vm.h"

void Scr_Error(const char* error)
{
	Scr_SetErrorMessage(error);
	Scr_ErrorInternal();
}