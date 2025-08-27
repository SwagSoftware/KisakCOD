#include "scr_stringlist.h"

#include <string.h> // strlen()
#include <universal/assertive.h>
#include <win32/win_local.h>
#include <universal/q_shared.h>
#include <qcommon/qcommon.h>

#include "scr_memorytree.h"
#include <universal/profile.h>
#include <universal/com_constantconfigstrings.h>
#include "scr_variable.h"

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#endif

scrStringDebugGlob_t* scrStringDebugGlob;
static scrStringDebugGlob_t scrStringDebugGlobBuf;
static scrStringGlob_t scrStringGlob; // 0x244E300

#define SCR_SYS_GAME 1

HashEntry_unnamed_type_u __cdecl Scr_AllocString(char *s, int sys)
{
	iassert(sys == SCR_SYS_GAME);
	return SL_GetString(s, 1);
}

void SL_Init()
{
	iassert(!scrStringGlob.inited);

	MT_Init();

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

	scrStringGlob.hashTable[0].status_next = 0;
	unsigned int prev = 0;
	for (unsigned int hash = 1; hash < STRINGLIST_SIZE; ++hash)
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
	PROF_SCOPED("SL_AddRefToString");

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
			for (int i = 1; i < 65536; ++i)
			{
				iassert(!scrStringDebugGlob->refCount[i]);
			}
			iassert((!scrStringDebugGlob->totalRefCount));
		}
		scrStringDebugGlob = NULL;
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

	for (unsigned int hash = 1; hash < STRINGLIST_SIZE; ++hash)
	{
		do
		{
			if ((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) == 0)
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

	for (const char* str = SL_ConvertToString(stringValue); *str; ++str)
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

	for (unsigned int hash = 1; hash < STRINGLIST_SIZE; ++hash)
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
	return SL_GetStringOfSize(str, user, strlen(str) + 1, type);
}

HashEntry_unnamed_type_u SL_GetStringOfSize(const char* str, unsigned int user, unsigned int len, int type)
{
	PROF_SCOPED("SL_GetStringOfSize");

	iassert(str);

	unsigned int hash = GetHashCode(str, len);

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

	RefString* refStr = NULL;

	unsigned int stringValue = 0;

	unsigned int prev;
	unsigned int next;
	unsigned int newIndex;

	HashEntry *entry = &scrStringGlob.hashTable[hash];
	HashEntry *newEntry;

	if ((entry->status_next & HASH_STAT_MASK) == HASH_STAT_HEAD)
	{
		refStr = GetRefString(entry->u.prev);

		// Check if this string is already stored, if it matches the string at this particular hash lookup, and return existing entry if so
		if (refStr->byteLen == len && !memcmp(refStr->str, str, len))
		{
			SL_AddUserInternal(refStr, user);

			iassert((entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
		
			stringValue = entry->u.prev;

			iassert(refStr->str == SL_ConvertToString(stringValue));
			
			Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
			return stringValue;
		}

		prev = hash;
		newIndex = (unsigned __int16)entry->status_next;

		for (newEntry = &scrStringGlob.hashTable[newIndex]; newEntry != entry; newEntry = &scrStringGlob.hashTable[newIndex])
		{
			iassert((newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE);

			refStr = GetRefString(newEntry->u.prev);

			if (refStr->byteLen == len && !memcmp(refStr->str, str, len))
			{
				scrStringGlob.hashTable[prev].status_next = (unsigned __int16)newEntry->status_next | scrStringGlob.hashTable[prev].status_next & HASH_STAT_MASK;
				newEntry->status_next = (unsigned __int16)entry->status_next | newEntry->status_next & HASH_STAT_MASK;
				entry->status_next = newIndex | entry->status_next & HASH_STAT_MASK;
				stringValue = newEntry->u.prev;
				newEntry->u.prev = entry->u.prev;
				entry->u.prev = stringValue;
				SL_AddUserInternal(refStr, user);

				iassert((newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
				iassert((entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
				iassert(refStr->str == SL_ConvertToString(stringValue));

				Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
				return stringValue;
			}
			prev = newIndex;
			newIndex = (unsigned __int16)newEntry->status_next;
		} //for()

		newIndex = scrStringGlob.hashTable[0].status_next;

		if (!newIndex)
		{
			// KISAKTODO?
			//Scr_DumpScriptThreads();
			//Scr_DumpScriptVariablesDefault();
			Com_Error(ERR_DROP, "exceeded maximum number of script strings (increase STRINGLIST_SIZE)");
		}

		stringValue = MT_AllocIndex(len + 4, type);
		newEntry = &scrStringGlob.hashTable[newIndex];
		iassert((newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_FREE);

		unsigned int newNext = newEntry->status_next;

		scrStringGlob.hashTable[0].status_next = newNext;
		scrStringGlob.hashTable[newNext].u.prev = 0;
		newEntry->status_next = (unsigned __int16)entry->status_next | HASH_STAT_MOVABLE;
		entry->status_next = (unsigned __int16)newIndex | entry->status_next & HASH_STAT_MASK;
		newEntry->u.prev = entry->u.prev;
	}
	else
	{
		if ((scrStringGlob.hashTable[hash].status_next & HASH_STAT_MASK) != 0)
		{
			iassert((entry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE);
			
			next = (unsigned __int16)entry->status_next;

			for (prev = next;
				(unsigned __int16)scrStringGlob.hashTable[prev].status_next != hash;
				prev = (unsigned __int16)scrStringGlob.hashTable[prev].status_next)
			{
				;
			}

			iassert(prev);

			newIndex = scrStringGlob.hashTable[0].status_next;

			if (!newIndex)
			{
				// KISAKTODO?
				//Scr_DumpScriptThreads();
				//Scr_DumpScriptVariablesDefault();
				Com_Error(ERR_DROP, "exceeded maximum number of script strings");
			}

			stringValue = MT_AllocIndex(len + 4, type);
			newEntry = &scrStringGlob.hashTable[newIndex];

			iassert((newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_FREE);

			unsigned int newNext = (unsigned __int16)newEntry->status_next;

			scrStringGlob.hashTable[0].status_next = newNext;
			scrStringGlob.hashTable[newNext].u.prev = 0;
			scrStringGlob.hashTable[prev].status_next = newIndex | scrStringGlob.hashTable[prev].status_next & HASH_STAT_MASK;
			newEntry->status_next = next | HASH_STAT_MOVABLE;
			newEntry->u.prev = entry->u.prev;
		}
		else
		{
			stringValue = MT_AllocIndex(len + 4, type);
			prev = entry->u.prev;
			next = (unsigned __int16)entry->status_next;

			scrStringGlob.hashTable[prev].status_next = next | scrStringGlob.hashTable[prev].status_next & HASH_STAT_MASK;
			scrStringGlob.hashTable[next].u.prev = prev;
		}
		iassert(!(hash & HASH_STAT_MASK));
		entry->status_next = hash | HASH_STAT_HEAD;
	}
	iassert(stringValue);
	entry->u.prev = stringValue;

	refStr = GetRefString(stringValue);
	memcpy((unsigned __int8*)refStr->str, (unsigned __int8*)str, len);
	refStr->data = ((unsigned __int8)user << 16) | refStr->data & 0xFF00FFFF;
	iassert(refStr->user == user);
	refStr->data = refStr->data & 0xFFFF0000 | 1;
	refStr->data = (len << 24) | refStr->data & 0xFFFFFF;

	if (scrStringDebugGlob)
	{
		InterlockedIncrement(&scrStringDebugGlob->totalRefCount);
		InterlockedIncrement(&scrStringDebugGlob->refCount[stringValue]);
	}

	iassert((entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
	iassert(refStr->str == SL_ConvertToString(stringValue));

//END_CLEANUP:
	Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
	return stringValue;
}

const char* SL_ConvertToString(unsigned int stringValue)
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

	return (RefString*)(&scrMemTreePub.mt_buffer[MT_NODE_SIZE * stringValue]);
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

static unsigned int FindStringOfSize(const char* str, unsigned int len)
{
	unsigned int stringValue = 0;

	PROF_SCOPED("FindStringOfSize");

	iassert(str);
	unsigned int hash = GetHashCode(str, len);

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

	HashEntry *entry = &scrStringGlob.hashTable[hash];

	if ((entry->status_next & HASH_STAT_MASK) != HASH_STAT_HEAD)
	{
		Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
		return 0;
	}

	RefString* refStr = GetRefString(entry->u.prev);

	if (refStr->byteLen != len || memcmp(refStr->str, str, len))
	{
		unsigned int prev = hash;
		unsigned int newIndex = (unsigned __int16)entry->status_next;

		for (HashEntry* newEntry = &scrStringGlob.hashTable[newIndex]; 
			newEntry != entry; 
			newEntry = &scrStringGlob.hashTable[newIndex])
		{
			iassert((newEntry->status_next & HASH_STAT_MASK) == HASH_STAT_MOVABLE);
			refStr = GetRefString(newEntry->u.prev);

			if (refStr->byteLen == len && !memcmp(refStr->str, str, len))
			{
				scrStringGlob.hashTable[prev].status_next = (unsigned __int16)newEntry->status_next | scrStringGlob.hashTable[prev].status_next & 0x30000;
				newEntry->status_next = (unsigned __int16)entry->status_next | newEntry->status_next & 0x30000;
				entry->status_next = newIndex | entry->status_next & 0x30000;
				stringValue = newEntry->u.prev;
				newEntry->u.prev = entry->u.prev;
				entry->u.prev = stringValue;

				iassert((newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
				iassert((entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
				iassert(refStr->str == SL_ConvertToString(stringValue));
		
				Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
				return stringValue;
			}
			prev = newIndex;
			newIndex = (unsigned __int16)newEntry->status_next;
		} // for()
		Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
		return 0;
	} //memcmp

	iassert((entry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);

	stringValue = entry->u.prev;
	iassert(refStr->str == SL_ConvertToString(stringValue));

	Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
	return stringValue;
}

unsigned int SL_FindString(const char* str)
{
	return FindStringOfSize(str, strlen(str) + 1);
}

void __cdecl SL_TransferRefToUser(unsigned int stringValue, unsigned int user)
{
	volatile LONG Comperand; // [esp+20h] [ebp-28h]
	RefString *refStr; // [esp+44h] [ebp-4h]

	PROF_SCOPED("SL_TransferRefToUser");

	refStr = GetRefString(stringValue);
	if ((user & refStr->user) != 0)
	{
		iassert(refStr->refCount > 1);

		if (scrStringDebugGlob)
		{
			iassert(scrStringDebugGlob->refCount[stringValue]);

			InterlockedDecrement(&scrStringDebugGlob->totalRefCount);
			InterlockedDecrement(&scrStringDebugGlob->refCount[stringValue]);
		}
		InterlockedDecrement(&refStr->data);
	}
	else
	{
		do
			Comperand = refStr->data;
		while (InterlockedCompareExchange(&refStr->data, Comperand | (user << 16), Comperand) != Comperand);
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

static unsigned int GetLowercaseStringOfSize(
	const char* str,
	unsigned int user,
	unsigned int len,
	int type)
{
	char stra[8192]; // [esp+4Ch] [ebp-2008h] BYREF
	unsigned int i; // [esp+2050h] [ebp-4h]

	PROF_SCOPED("GetLowercaseStringOfSize");
	if (len <= 0x2000)
	{
		for (i = 0; i < len; ++i)
			stra[i] = tolower(str[i]);
		return SL_GetStringOfSize(stra, user, len, type);
	}
	else
	{
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

	PROF_SCOPED("SL_RemoveRefToString");

	refStr = GetRefString(stringValue);
	len = SL_GetRefStringLen(refStr) + 1;
	SL_RemoveRefToStringOfSize(stringValue, len);
}

static void SL_FreeString(unsigned int stringValue, RefString* refStr, unsigned int len)
{
	PROF_SCOPED("SL_FreeString");

	unsigned int index = GetHashCode(refStr->str, len);

	Sys_EnterCriticalSection(CRITSECT_SCRIPT_STRING);

	if (refStr->refCount)
	{
		Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
		return;
	}
	else
	{
		HashEntry *entry = &scrStringGlob.hashTable[index];

		iassert(!refStr->user);

		MT_FreeIndex(stringValue, len + 4);

		iassert(((entry->status_next & HASH_STAT_MASK) == HASH_STAT_HEAD));

		unsigned int newIndex = (unsigned __int16)entry->status_next;
		HashEntry* newEntry = &scrStringGlob.hashTable[newIndex];

		if (entry->u.prev == stringValue)
		{
			if (newEntry == entry)
			{
				newEntry = entry;
				newIndex = index;
			}
			else
			{
				entry->status_next = (unsigned __int16)newEntry->status_next | HASH_STAT_HEAD;
				entry->u.prev = newEntry->u.prev;
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

		iassert((newEntry->status_next & HASH_STAT_MASK) != HASH_STAT_FREE);
		unsigned int newNext = scrStringGlob.hashTable[0].status_next;
		iassert((newNext & HASH_STAT_MASK) == HASH_STAT_FREE);

		newEntry->status_next = newNext;
		newEntry->u.prev = 0;
		scrStringGlob.hashTable[newNext].u.prev = newIndex;
		scrStringGlob.hashTable[0].status_next = newIndex;
		Sys_LeaveCriticalSection(CRITSECT_SCRIPT_STRING);
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

	PROF_SCOPED("SL_FindLowercaseString");
	len = strlen(str) + 1;
	if ((int)len <= 0x2000)
	{
		for (i = 0; i < (int)len; ++i)
			stra[i] = tolower(str[i]);
		return FindStringOfSize(stra, len);
	}
	else
	{
		return 0;
	}
}

void SL_RemoveRefToStringOfSize(unsigned int stringValue, unsigned int len)
{
	PROF_SCOPED("SL_RemoveRefToStringOfSize");

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
	}
	else
	{
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
	const char *v4; // [esp+4Ch] [ebp-2014h]
	char str[8192]; // [esp+50h] [ebp-2010h] BYREF
	HashEntry_unnamed_type_u v6; // [esp+2054h] [ebp-Ch]
	unsigned int len; // [esp+2058h] [ebp-8h]
	unsigned int i; // [esp+205Ch] [ebp-4h]

	PROF_SCOPED("SL_ConvertToLowercase");

	len = SL_GetStringLen(stringValue) + 1;
	if (len <= 0x2000)
	{
		v4 = SL_ConvertToString(stringValue);
		for (i = 0; i < len; ++i)
			str[i] = tolower(v4[i]);
		v6.prev = SL_GetStringOfSize(str, user, len, type).prev;
		SL_RemoveRefToString(stringValue);
		return v6;
	}
	else
	{
		return (HashEntry_unnamed_type_u)stringValue;
	}
}

void __cdecl CreateCanonicalFilename(char *newFilename, const char *filename, int count)
{
	unsigned int c; // [esp+0h] [ebp-4h]
	const int oldCount = count; // addition because the old assert was broken, lol

	iassert(count);
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

void Scr_SetStringFromCharString(unsigned __int16 *to, const char *from)
{
	unsigned int v4; // r3
	const char *v5; // r11

	v4 = *to;
	if (v4)
		SL_RemoveRefToString(v4);
	v5 = from;
	while (*(unsigned __int8 *)v5++)
		;
	*to = SL_GetStringOfSize(from, 0, v5 - from, 6);
}

unsigned int SL_GetUser(unsigned int stringValue)
{
	return GetRefString(stringValue)->user;
}

const char *SL_ConvertToStringSafe(unsigned int stringValue)
{
	if (!stringValue)
		return "(NULL)";

	if (scrStringDebugGlob)
	{
		iassert((!stringValue || !scrStringDebugGlob || scrStringDebugGlob->refCount[stringValue]));
	}

	return GetRefString(stringValue)->str;
}