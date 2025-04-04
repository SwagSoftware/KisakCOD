#include "r_cmdbuf.h"


void __cdecl R_InitContext(const GfxBackEndData *data, GfxCmdBuf *cmdBuf)
{
	cmdBuf->device = dx.device;
}