#include "scr_stringlist.h"

#include <string.h> // strlen()
#include <universal/assertive.h>
#include <win32/win_local.h>

struct scrStringDebugGlob_t 
{                           
    volatile int refCount[65536];
    volatile int totalRefCount;
    int ignoreLeaks;
};

static scrStringDebugGlob_t* scrStringDebugGlob;
static scrMemTreePub_t scrMemTreePub;

static unsigned int GetHashCode(const char* str, unsigned int len)
{
	unsigned int hash;

	if (len >= 0x100)
		return (len >> 2) % 0x4E1F + 1;
	hash = 0;
	while (len)
	{
		hash = *str++ + 31 * hash;
		--len;
	}
	return hash % 0x4E1F + 1;
}


unsigned int SL_GetString_(const char* str, unsigned int user, int type)
{
	return (int)(SL_GetStringOfSize(str, user, strlen(str) + 1, type).prev); // LWSS: bit hacky here with the union
}

HashEntry_unnamed_type_u SL_GetStringOfSize(const char* str, unsigned int user, unsigned int len, int type)
{
	// KISAKFINISHFUNCTION
	// Profile_Begin(334); // KISAKPROFILER
	iassert(str);

	unsigned int hash = GetHashCode(str, len);

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

}

char* SL_ConvertToString(unsigned int stringValue)
{
	iassert((!stringValue || !scrStringDebugGlob || scrStringDebugGlob->refCount[stringValue]));

	if (stringValue)
	{
		return GetRefString(stringValue)->str;
	}
	else
	{
		return NULL;
	}
}

RefString* GetRefString(unsigned int stringValue)
{
	iassert(stringValue);
	iassert(stringValue * MT_NODE_SIZE < MT_SIZE);

	return (RefString*)(scrMemTreePub.mt_buffer[12 * stringValue]);
}

static HashEntry_unnamed_type_u FindStringOfSize(const char* str, unsigned int len)
{
	// KISAKFINISHFUNCTION
}

unsigned int SL_FindString(const char* str)
{
	return FindStringOfSize(str, strlen(str) + 1).prev;
}

static int SL_GetRefStringLen(RefString* refString)
{
	int len;

	for (len = (byte)(HIBYTE(*(DWORD*)&refString->unionThing.subUnion) - 1); refString->str[len]; len += 256);

	return len;
}

void SL_RemoveRefToString(unsigned int stringValue)
{
	RefString* refStr; // [esp+30h] [ebp-8h]
	int len; // [esp+34h] [ebp-4h]

	//Profile_Begin(334); // KISAKPROFILER
	refStr = GetRefString(stringValue);
	len = SL_GetRefStringLen(refStr) + 1;
	//Profile_EndInternal(0); // KISAKPROFILER
	SL_RemoveRefToStringOfSize(stringValue, len);
}

void SL_RemoveRefToStringOfSize(unsigned int stringValue, unsigned int len)
{
	// KISAKFINISHFUNCTION
}