#include "r_cmdbuf.h"
#include "r_utils.h"


void __cdecl R_InitContext(const GfxBackEndData *data, GfxCmdBuf *cmdBuf)
{
	cmdBuf->device = dx.device;
}

void __cdecl R_CmdBufSet3D(GfxCmdBufSourceState *source)
{
	GfxCmdBufSourceState *worldMatrix; // [esp+0h] [ebp-4h]

	++source->matrixVersions[1];
	++source->matrixVersions[2];
	++source->matrixVersions[4];
	worldMatrix = R_GetActiveWorldMatrix(source);
	R_MatrixIdentity44((float (*)[4])worldMatrix);
	Vec3Sub(worldMatrix->matrices.matrix[0].m[3], source->eyeOffset, worldMatrix->matrices.matrix[0].m[3]);
}