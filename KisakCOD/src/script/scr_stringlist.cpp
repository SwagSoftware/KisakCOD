#include "scr_stringlist.h"

#include <string.h> // strlen()
#include <universal/assertive.h>
#include <win32/win_local.h>
#include <universal/q_shared.h>
#include <qcommon/qcommon.h>

#include <client_mp/client_mp.h>

#include "scr_memorytree.h"
#include <universal/profile.h>
#include <universal/com_constantconfigstrings.h>
#include "scr_variable.h"

scrStringDebugGlob_t* scrStringDebugGlob;
static scrStringDebugGlob_t scrStringDebugGlobBuf;
static scrStringGlob_t scrStringGlob; // 0x244E300

HashEntry_unnamed_type_u __cdecl Scr_AllocString(char *s, int sys)
{
	if (sys != 1)
		MyAssertHandler(".\\script\\scr_stringlist.cpp", 1030, 0, "%s", "sys == SCR_SYS_GAME");
	return SL_GetString(s, 1u);
}

void SL_Init()
{
	iassert(!scrStringGlob.inited);

	MT_Init();

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);
	scrStringGlob.hashTable[0].status_next = 0;
	unsigned int prev = 0;
	for (unsigned int hash = 1; hash < 20'000; ++hash)
	{
		iassert(!(hash & HASH_STAT_MASK));
		scrStringGlob.hashTable[hash].status_next = 0;
		scrStringGlob.hashTable[prev].status_next |= hash;
		scrStringGlob.hashTable[hash].u.prev = prev;
		prev = hash;
	}
	scrStringGlob.hashTable[0].u.prev = prev;
	SL_InitCheckLeaks();
	scrStringGlob.inited = 1;
	Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
}

void SL_InitCheckLeaks()
{
	iassert(!scrStringDebugGlob);

	scrStringDebugGlob = &scrStringDebugGlobBuf;
	Com_Memset(&scrStringDebugGlobBuf, 0, 0x40000);
	scrStringDebugGlob->totalRefCount = 0;
}

static unsigned int SL_ConvertFromRefString(RefString *refString)
{
	return ((char *)refString - scrMemTreePub.mt_buffer) / MT_NODE_SIZE;
}

void SL_AddUserInternal(RefString* refStr, unsigned int user)
{
	if (((unsigned __int8)user & refStr->user) == 0)
	{
		int str = SL_ConvertFromRefString(refStr);
		if (scrStringDebugGlob)
		{
			iassert((scrStringDebugGlob->refCount[str] < 65536));
			iassert((scrStringDebugGlob->refCount[str] >= 0));

			InterlockedIncrement(&scrStringDebugGlob->totalRefCount);
			InterlockedIncrement(&scrStringDebugGlob->refCount[str]);
		}

		volatile int Comperand;
		do
			Comperand = refStr->data;
		while (InterlockedCompareExchange((volatile LONG*)refStr, Comperand | (user << 16), Comperand) != Comperand);
		InterlockedIncrement((volatile LONG*)refStr);
	}
}

void SL_AddRefToString(unsigned int stringValue)
{
	Profile_Begin(334);
	if (scrStringDebugGlob)
	{
		iassert(scrStringDebugGlob->refCount[stringValue]);
		iassert((scrStringDebugGlob->refCount[stringValue] < 65536));

		InterlockedIncrement(&scrStringDebugGlob->totalRefCount);
		InterlockedIncrement(&scrStringDebugGlob->refCount[stringValue]);
	}
	RefString* refStr = GetRefString(stringValue);
	InterlockedIncrement((volatile LONG*)refStr);

	iassert(refStr->refCount);

	Profile_EndInternal(0);
}

void SL_CheckExists(unsigned int stringValue)
{
	if (scrStringDebugGlob)
	{
		iassert(!scrStringDebugGlob || scrStringDebugGlob->refCount[stringValue]);
	}
}

static void SL_CheckLeaks()
{
	if (scrStringDebugGlob)
	{
		if (!scrStringDebugGlob->ignoreLeaks)
		{
			for (int i = 1; i < 0x10000; ++i)
			{
				iassert(!scrStringDebugGlob->refCount[i]);
			}
			iassert((!scrStringDebugGlob->totalRefCount));
		}
		scrStringDebugGlob = 0;
	}
}

void SL_Shutdown()
{
	if (scrStringGlob.inited)
	{
		scrStringGlob.inited = 0;
		SL_CheckLeaks();
	}
}

void SL_ShutdownSystem(unsigned int user)
{
	iassert(user);

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

	for (unsigned int hash = 1; hash < 20'000; ++hash)
	{
		do
		{
			if ((scrStringGlob.hashTable[hash].status_next & 0x30000) == 0)
				break;
			RefString* refStr = GetRefString(scrStringGlob.hashTable[hash].u.prev);

			if (((unsigned __int8)user & refStr->user) == 0)
				break;

			refStr->data = ((unsigned __int8)(~(BYTE)user & HIWORD(refStr->data)) << 16) | refStr->data & 0xFF00FFFF;

			scrStringGlob.nextFreeEntry = 0;
			SL_RemoveRefToString(scrStringGlob.hashTable[hash].u.prev);
		} while (scrStringGlob.nextFreeEntry);
	}

	Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
}

int SL_IsLowercaseString(unsigned int stringValue)
{
	iassert(stringValue);

	for (char* str = SL_ConvertToString(stringValue); *str; ++str)
	{
		int cmp = *str;
		if (cmp != (char)tolower(cmp))
		{
			return 0;
		}
	}

	return 1;
}

void SL_TransferSystem(unsigned int from, unsigned int to)
{
	iassert(from);
	iassert(to);

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);
	for (unsigned int hash = 1; hash < 20'000; ++hash)
	{
		if ((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) != 0)
		{
			RefString* refStr = GetRefString(scrStringGlob.hashTable[hash].u.prev);
			if (((unsigned __int8)from & refStr->user) != 0)
			{
				refStr->data = ((unsigned __int8)(~(BYTE)from & HIWORD(refStr->data)) << 16) | refStr->data & 0xFF00FFFF;
				refStr->data = ((unsigned __int8)(to | HIWORD(refStr->data)) << 16) | refStr->data & 0xFF00FFFF;
			}
		}
	}
	Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
}

unsigned int SL_GetString_(const char* str, unsigned int user, int type)
{
	return (int)(SL_GetStringOfSize(str, user, strlen(str) + 1, type).prev); // LWSS: bit hacky here with the union
}

HashEntry_unnamed_type_u SL_GetStringOfSize(const char* str, unsigned int user, unsigned int len, int type)
{
	// KISAKFINISHFUNCTION
	Profile_Begin(334); // KISAKPROFILER
	iassert(str);

	unsigned int hash = GetHashCode(str, len);

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

	RefString* refStr = NULL;
	RefString* refStrA; // [esp+90h] [ebp-10h]
	RefString* refStrB; // [esp+90h] [ebp-10h]
	HashEntry_unnamed_type_u stringValue = 0;

	unsigned int newIndexA; // [esp+88h] [ebp-18h]
	unsigned int newIndexB; // [esp+88h] [ebp-18h]

	if ((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) == HASH_STAT_HEAD)
	{
		refStr = GetRefString(scrStringGlob.hashTable[hash].u.prev);
		if (HIBYTE(refStr->data) == (unsigned __int8)len && !memcmp(refStr->str, str, len))
		{
			SL_AddUserInternal(refStr, user);
			iassert((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
		
			stringValue.prev = (unsigned int)scrStringGlob.hashTable[hash].u;

			iassert(refStr->str == SL_ConvertToString(stringValue));
			
			Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
			Profile_EndInternal(0);
			return stringValue;
		}

		unsigned int prev = hash;
		unsigned int newIndex = (unsigned __int16)scrStringGlob.hashTable[hash].status_next;
		for (HashEntry* newEntry = &scrStringGlob.hashTable[newIndex];
			newEntry != &scrStringGlob.hashTable[hash];
			newEntry = &scrStringGlob.hashTable[newIndex]
			)
		{
			iassert((newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE);
			refStrA = GetRefString(newEntry->u.prev);
			if (HIBYTE(refStrA->data) == (unsigned __int8)len && !memcmp(refStrA->str, str, len))
			{
				scrStringGlob.hashTable[prev].status_next = (unsigned __int16)newEntry->status_next | scrStringGlob.hashTable[prev].status_next & 0x30000;
				newEntry->status_next = (unsigned __int16)scrStringGlob.hashTable[hash].status_next | newEntry->status_next & 0x30000;
				scrStringGlob.hashTable[hash].status_next = newIndex | scrStringGlob.hashTable[hash].status_next & 0x30000;
				stringValue.prev = (unsigned int)newEntry->u;
				newEntry->u.prev = scrStringGlob.hashTable[hash].u.prev;
				scrStringGlob.hashTable[hash].u = stringValue;
				SL_AddUserInternal(refStrA, user);

				iassert((newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
				iassert((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
				iassert(refStrA->str == SL_ConvertToString(stringValue));

				Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
				Profile_EndInternal(0);
				return stringValue;
			}
			prev = newIndex;
			newIndex = (unsigned __int16)newEntry->status_next;
		} //for()
		newIndexA = scrStringGlob.hashTable[0].status_next;

		if (!scrStringGlob.hashTable[0].status_next)
		{
			// KISAKTODO?
			//Scr_DumpScriptThreads();
			//Scr_DumpScriptVariablesDefault();
			Com_Error(ERR_DROP, "exceeded maximum number of script strings (increase STRINGLIST_SIZE)");
		}
		stringValue.prev = MT_AllocIndex(len + 4, type);
		iassert((scrStringGlob.hashTable[newIndexA].status_next & HASH_STAT_MASK) == HASH_STAT_FREE);

		scrStringGlob.hashTable[0].status_next = (unsigned __int16)scrStringGlob.hashTable[newIndexA].status_next;
		scrStringGlob.hashTable[scrStringGlob.hashTable[0].status_next].u.prev = 0;
		scrStringGlob.hashTable[newIndexA].status_next = (unsigned __int16)scrStringGlob.hashTable[hash].status_next | 0x10000;
		scrStringGlob.hashTable[hash].status_next = (unsigned __int16)newIndexA | scrStringGlob.hashTable[hash].status_next & 0x30000;
		scrStringGlob.hashTable[newIndexA].u.prev = scrStringGlob.hashTable[hash].u.prev;
	}
	else
	{
		if ((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) != 0)
		{
			iassert((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE);
			unsigned int next = (unsigned __int16)scrStringGlob.hashTable[hash].status_next;

			unsigned int preva;
			for (preva = next;
				(unsigned __int16)scrStringGlob.hashTable[preva].status_next != hash;
				preva = (unsigned __int16)scrStringGlob.hashTable[preva].status_next)
			{
				;
			}

			iassert(preva);

			newIndexB = scrStringGlob.hashTable[0].status_next;
			if (!scrStringGlob.hashTable[0].status_next)
			{
				// KISAKTODO?
				//Scr_DumpScriptThreads();
				//Scr_DumpScriptVariablesDefault();
				Com_Error(ERR_DROP, "exceeded maximum number of script strings");
			}
			stringValue.prev = MT_AllocIndex(len + 4, type);
			iassert((scrStringGlob.hashTable[newIndexB].status_next & HASH_STAT_MASK) == HASH_STAT_FREE);
			scrStringGlob.hashTable[0].status_next = (unsigned __int16)scrStringGlob.hashTable[newIndexB].status_next;
			scrStringGlob.hashTable[scrStringGlob.hashTable[0].status_next].u.prev = 0;
			scrStringGlob.hashTable[preva].status_next = newIndexB | scrStringGlob.hashTable[preva].status_next & 0x30000;
			scrStringGlob.hashTable[newIndexB].status_next = next | 0x10000;
			scrStringGlob.hashTable[newIndexB].u.prev = scrStringGlob.hashTable[hash].u.prev;
		}
		else
		{
			stringValue.prev = MT_AllocIndex(len + 4, type);
			unsigned int nextA = (unsigned __int16)scrStringGlob.hashTable[hash].status_next;
			unsigned int v4 = (unsigned int)scrStringGlob.hashTable[hash].u;
			scrStringGlob.hashTable[v4].status_next = nextA | scrStringGlob.hashTable[v4].status_next & 0x30000;
			scrStringGlob.hashTable[nextA].u = v4;
		}
		iassert(!(hash & HASH_STAT_MASK));
		scrStringGlob.hashTable[hash].status_next = hash | 0x20000;
	}
	iassert(stringValue);
	scrStringGlob.hashTable[hash].u = stringValue;

	refStrB = GetRefString(stringValue.prev);
	memcpy((unsigned __int8*)refStrB->str, (unsigned __int8*)str, len);
	refStrB->data = ((unsigned __int8)user << 16) | refStrB->data & 0xFF00FFFF;
	iassert(refStrB->user == user);
	refStrB->data = refStrB->data & 0xFFFF0000 | 1;
	refStrB->data = (len << 24) | refStrB->data & 0xFFFFFF;
	if (scrStringDebugGlob)
	{
		InterlockedIncrement(&scrStringDebugGlob->totalRefCount);
		InterlockedIncrement(&scrStringDebugGlob->refCount[stringValue.prev]);
	}
	//iassert((scrStringGlob.hashTable[hash].status_next & 0x30000) == 0);
	iassert(refStrB->str == SL_ConvertToString(stringValue));
END_CLEANUP:
	Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
	Profile_EndInternal(0);
	return stringValue;
}

char* SL_ConvertToString(unsigned int stringValue)
{
	//iassert((!stringValue || !scrStringDebugGlob || scrStringDebugGlob->refCount[stringValue]));

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

	return (RefString*)(&scrMemTreePub.mt_buffer[12 * stringValue]);
}
RefString* GetRefString(const char* str)
{
	iassert(str >= scrMemTreePub.mt_buffer && str < scrMemTreePub.mt_buffer + MT_SIZE);

	return (RefString*)(str - 4);
}

int SL_GetStringLen(unsigned int stringValue)
{
	iassert(stringValue);
	RefString* refString = GetRefString(stringValue);
	return SL_GetRefStringLen(refString);
}

static HashEntry_unnamed_type_u FindStringOfSize(const char* str, unsigned int len)
{
	HashEntry_unnamed_type_u stringValue = 0;

	Profile_Begin(334);
	iassert(str);
	unsigned int hash = GetHashCode(str, len);

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

	if ((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) != HASH_STAT_HEAD)
	{
		Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
		Profile_EndInternal(0);
		return 0;
	}

	RefString* refStr = GetRefString(scrStringGlob.hashTable[hash].u.prev);

	if ((unsigned __int8)HIBYTE(*(DWORD*)&refStr->data) != (unsigned __int8)len || memcmp(refStr->str, str, len))
	{
		unsigned int prev = hash;
		unsigned int newIndex = (word)scrStringGlob.hashTable[hash].status_next;

		for (HashEntry* newEntry = &scrStringGlob.hashTable[newIndex]; 
			newEntry != &scrStringGlob.hashTable[hash]; 
			newEntry = &scrStringGlob.hashTable[newIndex])
		{
			iassert((newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE);
			RefString* refStrA = GetRefString(newEntry->u.prev);

			if (HIBYTE(refStrA->data) == (unsigned __int8)len && !memcmp(refStrA->str, str, len))
			{

				scrStringGlob.hashTable[prev].status_next = (unsigned __int16)newEntry->status_next | scrStringGlob.hashTable[prev].status_next & 0x30000;
				newEntry->status_next = (unsigned __int16)scrStringGlob.hashTable[hash].status_next | newEntry->status_next & 0x30000;
				scrStringGlob.hashTable[hash].status_next = newIndex | scrStringGlob.hashTable[hash].status_next & 0x30000;
				stringValue.prev = (unsigned int)newEntry->u;
				newEntry->u.prev = scrStringGlob.hashTable[hash].u.prev;
				scrStringGlob.hashTable[hash].u = stringValue;
				iassert((newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
				iassert((scrStringGlob.hashTable[hash].status_next & 0x30000) != HASH_STAT_FREE);
				iassert(refStrA->str == SL_ConvertToString(stringValue.prev));
		
				Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
				Profile_EndInternal(0);
				return stringValue;
			}
			prev = newIndex;
			newIndex = (unsigned __int16)newEntry->status_next;
		} // for()
		Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
		Profile_EndInternal(0);
		return 0;
	} //memcmp
	iassert((scrStringGlob.hashTable[hash].status_next & 0x30000) != HASH_STAT_FREE);

	stringValue.prev = scrStringGlob.hashTable[hash].u;

	iassert(refStr->str == SL_ConvertToString(stringValue));

	Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
	Profile_EndInternal(0);
	return stringValue;
}

unsigned int SL_FindString(const char* str)
{
	return FindStringOfSize(str, strlen(str) + 1).prev;
}

void __cdecl SL_TransferRefToUser(unsigned int stringValue, unsigned int user)
{
	const char *v2; // eax
	const char *v3; // eax
	volatile LONG Comperand; // [esp+20h] [ebp-28h]
	RefString *refStr; // [esp+44h] [ebp-4h]

	Profile_Begin(334);
	refStr = GetRefString(stringValue);
	if ((user & BYTE2(refStr->data)) != 0)
	{
		if (refStr->data <= 1u)
		{
			v2 = SL_DebugConvertToString(stringValue);
			MyAssertHandler(
				".\\script\\scr_stringlist.cpp",
				803,
				0,
				"%s\n\t(SL_DebugConvertToString( stringValue )) = %s",
				"(refStr->refCount > 1)",
				v2);
		}
		if (scrStringDebugGlob)
		{
			if (!scrStringDebugGlob->refCount[stringValue])
			{
				v3 = SL_DebugConvertToString(stringValue);
				MyAssertHandler(
					".\\script\\scr_stringlist.cpp",
					808,
					0,
					"%s\n\t(SL_DebugConvertToString( stringValue )) = %s",
					"(scrStringDebugGlob->refCount[stringValue])",
					v3);
			}
			InterlockedDecrement(&scrStringDebugGlob->totalRefCount);
			InterlockedDecrement(&scrStringDebugGlob->refCount[stringValue]);
		}
		InterlockedDecrement(&refStr->data);
		Profile_EndInternal(0);
	}
	else
	{
		do
			Comperand = refStr->data;
		while (InterlockedCompareExchange(&refStr->data, Comperand | (user << 16), Comperand) != Comperand);
		Profile_EndInternal(0);
	}
}

unsigned int SL_GetStringForVector(const float* v)
{
	char tempString[132];

	sprintf(tempString, "(%g, %g, %g)", *v, v[1], v[2]);
	return SL_GetString_(tempString, 0, 15);
}

unsigned int SL_GetStringForInt(int i)
{
	char tempString[132]; // [esp+0h] [ebp-88h] BYREF

	sprintf(tempString, "%i", i);
	return SL_GetString_(tempString, 0, 15);
}

unsigned int SL_GetStringForFloat(float f)
{
	char tempString[132]; // [esp+8h] [ebp-88h] BYREF

	sprintf(tempString, "%g", f);
	return SL_GetString_(tempString, 0, 15);
}

unsigned int SL_GetString(const char* str, unsigned int user)
{
	return SL_GetString_(str, user, 6);
}

int SL_GetRefStringLen(RefString* refString)
{
	int len; // [esp+0h] [ebp-4h]

	for (len = (unsigned __int8)(HIBYTE(refString->data) - 1); refString->str[len]; len += 256)
		;
	return len;
}

static HashEntry_unnamed_type_u  GetLowercaseStringOfSize(
	const char* str,
	unsigned int user,
	unsigned int len,
	int type)
{
	char stra[8192]; // [esp+4Ch] [ebp-2008h] BYREF
	unsigned int i; // [esp+2050h] [ebp-4h]

	Profile_Begin(334);
	if (len <= 0x2000)
	{
		for (i = 0; i < len; ++i)
			stra[i] = tolower(str[i]);
		Profile_EndInternal(0);
		return SL_GetStringOfSize(stra, user, len, type);
	}
	else
	{
		Profile_EndInternal(0);
		Com_Error(ERR_DROP, "max string length exceeded: \"%s\"", str);
		return 0;
	}
}

unsigned int SL_GetLowercaseString_(const char* str, unsigned int user, int type)
{
	return GetLowercaseStringOfSize(str, user, strlen(str) + 1, type);
}
unsigned int SL_GetLowercaseString(const char* str, unsigned int user)
{
	return SL_GetLowercaseString_(str, user, 6);
}

void SL_RemoveRefToString(unsigned int stringValue)
{
	RefString* refStr; // [esp+30h] [ebp-8h]
	int len; // [esp+34h] [ebp-4h]

	Profile_Begin(334); // KISAKPROFILER
	refStr = GetRefString(stringValue);
	len = SL_GetRefStringLen(refStr) + 1;
	Profile_EndInternal(0); // KISAKPROFILER
	SL_RemoveRefToStringOfSize(stringValue, len);
}

static void SL_FreeString(unsigned int stringValue, RefString* refStr, unsigned int len)
{
	Profile_Begin(334); // KISAKPROFILER
	unsigned int index = GetHashCode(refStr->str, len);
	HashEntry* entry = &scrStringGlob.hashTable[index];

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

	if (refStr->data)
	{
		Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
		Profile_EndInternal(0);
	}
	else
	{
		iassert(!refStr->user);
		MT_FreeIndex(stringValue, len + 4);

		iassert(((entry->status_next & HASH_STAT_MASK) == HASH_STAT_HEAD));

		unsigned int newIndex = scrStringGlob.hashTable[index].status_next;
		HashEntry* newEntry = &scrStringGlob.hashTable[newIndex];

		if (scrStringGlob.hashTable[index].u.prev == stringValue)
		{
			if (newEntry == entry)
			{
				newEntry = &scrStringGlob.hashTable[index];
				newIndex = index;
			}
			else
			{
				scrStringGlob.hashTable[index].status_next = (unsigned __int16)scrStringGlob.hashTable[newIndex].status_next | HASH_STAT_HEAD;
				scrStringGlob.hashTable[index].u.prev = scrStringGlob.hashTable[newIndex].u.prev;
				scrStringGlob.nextFreeEntry = &scrStringGlob.hashTable[index];
			}
		}
		else
		{
			unsigned int prev = index;
			while (1)
			{
				iassert(newEntry != entry);
				iassert((newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE);

				if (newEntry->u.prev == stringValue)
					break;

				prev = newIndex;
				newIndex = (unsigned __int16)newEntry->status_next;
				newEntry = &scrStringGlob.hashTable[newIndex];
			}
			scrStringGlob.hashTable[prev].status_next = (unsigned __int16)newEntry->status_next | scrStringGlob.hashTable[prev].status_next & HASH_STAT_MASK;
		}
		//iassert((newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
		unsigned int newNext = scrStringGlob.hashTable[0].status_next;
		iassert((newNext & HASH_STAT_MASK) == HASH_STAT_FREE);
		newEntry->status_next = newNext;
		newEntry->u.prev = 0;
		scrStringGlob.hashTable[newNext].u.prev = newIndex;
		scrStringGlob.hashTable[0].status_next = newIndex;
		Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
		Profile_EndInternal(0);
	}
}

const char* __cdecl SL_DebugConvertToString(unsigned int stringValue)
{
	int len; // [esp+0h] [ebp-10h]
	int i; // [esp+8h] [ebp-8h]
	RefString* refString; // [esp+Ch] [ebp-4h]

	if (!stringValue)
		return "<NULL>";
	refString = GetRefString(stringValue);
	len = (unsigned __int8)(HIBYTE(refString->data) - 1);
	if (refString->str[len])
		return "<BINARY>";
	for (i = 0; i < len; ++i)
	{
		if (!isprint((unsigned __int8)refString->str[i]))
			return "<BINARY>";
	}
	return refString->str;
}

unsigned int SL_ConvertFromString(const char* str)
{
	iassert(str);
	RefString* refStr = GetRefString(str);
	return SL_ConvertFromRefString(refStr);
}

HashEntry_unnamed_type_u SL_FindLowercaseString(const char* str)
{
	char stra[8196]; // [esp+5Ch] [ebp-2010h] BYREF
	unsigned int len; // [esp+2064h] [ebp-8h]
	signed int i; // [esp+2068h] [ebp-4h]

	Profile_Begin(334);
	len = strlen(str) + 1;
	if ((int)len <= 0x2000)
	{
		for (i = 0; i < (int)len; ++i)
			stra[i] = tolower(str[i]);
		Profile_EndInternal(0);
		return FindStringOfSize(stra, len);
	}
	else
	{
		Profile_EndInternal(0);
		return 0;
	}
}

void SL_RemoveRefToStringOfSize(unsigned int stringValue, unsigned int len)
{
	Profile_Begin(334); // KISAKPROFILER
	RefString* refStr = GetRefString(stringValue);

	if (InterlockedDecrement((volatile LONG*)refStr) << 16)
	{
		if (scrStringDebugGlob)
		{
			iassert(scrStringDebugGlob->totalRefCount && scrStringDebugGlob->refCount[stringValue]);
			iassert(scrStringDebugGlob->refCount[stringValue]);

			InterlockedDecrement(&scrStringDebugGlob->totalRefCount);
			InterlockedDecrement(&scrStringDebugGlob->refCount[stringValue]);
		}
		Profile_EndInternal(0);
	}
	else
	{
		Profile_EndInternal(0);
		SL_FreeString(stringValue, refStr, len);
		if (scrStringDebugGlob)
		{
			iassert(scrStringDebugGlob->totalRefCount && scrStringDebugGlob->refCount[stringValue]);
			iassert(scrStringDebugGlob->refCount[stringValue]);

			InterlockedDecrement(&scrStringDebugGlob->totalRefCount);
			InterlockedDecrement(&scrStringDebugGlob->refCount[stringValue]);
		}
	}
}

void __cdecl SL_AddUser(unsigned int stringValue, unsigned int user)
{
	RefString *RefString; // eax

	RefString = GetRefString(stringValue);
	SL_AddUserInternal(RefString, user);
}

void __cdecl Scr_SetString(unsigned __int16 *to, unsigned int from)
{
	if (from)
		SL_AddRefToString(from);
	if (*to)
		SL_RemoveRefToString(*to);
	*to = from;
}

HashEntry_unnamed_type_u __cdecl SL_ConvertToLowercase(unsigned int stringValue, unsigned int user, int type)
{
	char *v4; // [esp+4Ch] [ebp-2014h]
	char str[8192]; // [esp+50h] [ebp-2010h] BYREF
	HashEntry_unnamed_type_u v6; // [esp+2054h] [ebp-Ch]
	unsigned int len; // [esp+2058h] [ebp-8h]
	unsigned int i; // [esp+205Ch] [ebp-4h]

	Profile_Begin(334);
	len = SL_GetStringLen(stringValue) + 1;
	if (len <= 0x2000)
	{
		v4 = SL_ConvertToString(stringValue);
		for (i = 0; i < len; ++i)
			str[i] = tolower(v4[i]);
		v6.prev = SL_GetStringOfSize(str, user, len, type).prev;
		SL_RemoveRefToString(stringValue);
		Profile_EndInternal(0);
		return v6;
	}
	else
	{
		Profile_EndInternal(0);
		return (HashEntry_unnamed_type_u)stringValue;
	}
}

void __cdecl CreateCanonicalFilename(char *newFilename, const char *filename, int count)
{
	unsigned int c; // [esp+0h] [ebp-4h]
	const int oldCount = count; // addition because the old assert was broken, lol
	if (!count)
		MyAssertHandler(".\\script\\scr_stringlist.cpp", 1146, 0, "%s", "count");
	do
	{
		do
		{
			do
				c = *filename++;
			while (c == 92);
		} while (c == 47);
		while (c >= 0x20)
		{
			*newFilename++ = tolower(c);
			if (!--count)
				Com_Error(ERR_DROP, "Filename %s exceeds maximum length of %d", filename, oldCount);
			if (c == 47)
				break;
			c = *filename++;
			if (c == 92)
				c = 47;
		}
	} while (c);
	*newFilename = 0;
}

HashEntry_unnamed_type_u __cdecl Scr_CreateCanonicalFilename(const char *filename)
{
	char newFilename[1028]; // [esp+0h] [ebp-408h] BYREF

	CreateCanonicalFilename(newFilename, filename, 1024);
	return SL_GetString_(newFilename, 0, 7);
}