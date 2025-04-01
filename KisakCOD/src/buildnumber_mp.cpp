#include "buildnumber_mp.h"
#include <stdio.h>

char buf[128];

char *__cdecl getBuildNumber()
{
	sprintf(buf, "%d %s", 13620, "Thu Oct 04 00:43:04 2007");
	return buf;
}

int getBuildNumberAsInt()
{
	return 13620;
}