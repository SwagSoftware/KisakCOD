#include "assertive.h"

#include "script/scr_stringlist.h"

/*
========================
CopyString

 NOTE:	never write over the memory CopyString returns because
		memory from a memstatic_t might be returned
========================
*/
const char* CopyString(const char* in)
{
	unsigned int out;

	iassert(in);

	out = SL_GetString_(in, 0, 21);
	return SL_ConvertToString(out);
}

void FreeString(const char* str)
{
	iassert(str);

	unsigned int out = SL_FindString(str);

	iassert(out);

	SL_RemoveRefToString(out);
}