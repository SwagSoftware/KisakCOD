#include "buildnumber_mp.h"
#include <stdio.h>

char buildnumbuf[128];

char *__cdecl getBuildNumber()
{
	sprintf_s(buildnumbuf, "%d %s", 13620, "Thu Oct 04 00:43:04 2007");
	return buildnumbuf;
}

int getBuildNumberAsInt()
{
	return 13620;
}