#pragma once
#include <script/scr_variable.h>

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

struct BuiltinMethodDef
{
	const char *actionString;
	void(*actionFunc)(scr_entref_t);
	int type;
};

extern const BuiltinMethodDef methods[104];

// mission_dvarlist
extern const char *missionDvarList[3];