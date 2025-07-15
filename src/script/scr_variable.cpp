#include "scr_main.h"
#include "scr_animtree.h"
#include "scr_variable.h"
#include "scr_stringlist.h"
#include "scr_memorytree.h"
#include "scr_vm.h"
#include "scr_compiler.h"

#include <qcommon/qcommon.h>
#include <Windows.h>
#include <universal/com_files.h>
#include "scr_parser.h"
#include <database/database.h>
#include <universal/q_parse.h>
#include <cstdint>

#undef GetObject
#undef FindObject

scrVarPub_t scrVarPub;
scrVarDebugPub_t* scrVarDebugPub;
scrVarDebugPub_t scrVarDebugPubBuf;
scrVarGlob_t scrVarGlob;

scr_classStruct_t g_classMap[4] =
{
	{ 0, 0, 0x65, "entity" },
	{ 0, 0, 0x68, "hudelem" },
	{ 0, 0, 0x70, "pathnode" },
	{ 0, 0, 0x76, "vehiclenode" }
};

int  VariableInfoFunctionCompare(void *p_info1, void *p_info2)
{
	const char *functionName2; // [esp+0h] [ebp-Ch]
	const char *functionName1; // [esp+4h] [ebp-8h]
	int fileNameCompare; // [esp+8h] [ebp-4h]

	_DWORD *info1 = (_DWORD *)p_info1;
	_DWORD *info2 = (_DWORD *)p_info2;

	fileNameCompare = VariableInfoFileNameCompare(info1, info2);
	if (fileNameCompare)
		return fileNameCompare;
	functionName1 = (const char *)info1[2];
	functionName2 = (const char *)info2[2];
	if (!functionName1)
		return 1;
	if (functionName2)
		return I_stricmp(functionName1, functionName2);
	return -1;
}

int __cdecl CompareThreadIndices(unsigned int *arg1, unsigned int *arg2)
{
	return *arg1 - *arg2;
}

void __cdecl Scr_Cleanup()
{
	scrVarPub.evaluate = 0;
}

bool IsObject(VariableValueInternal* entryValue)
{
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	
	return (entryValue->w.type & VAR_MASK) >= VAR_THREAD;
}

bool IsObject(VariableValue* value)
{
	return value->type >= VAR_THREAD;
}

void Scr_InitVariables()
{
	if (!scrVarDebugPub)
		scrVarDebugPub = &scrVarDebugPubBuf;

	memset(scrVarDebugPub->leakCount, 0, sizeof(scrVarDebugPub->leakCount));

	scrVarPub.totalObjectRefCount = 0;
	scrVarPub.totalVectorRefCount = 0;

	if (scrVarDebugPub)
		memset(scrVarDebugPub->extRefCount, 0, sizeof(scrVarDebugPub->extRefCount));

	scrVarPub.numScriptValues = 0;
	scrVarPub.numScriptObjects = 0;

	if (scrVarDebugPub)
		memset(scrVarDebugPub, 0, 0x60000u);

	Scr_InitVariableRange(1u, 0x8001u);
	Scr_InitVariableRange(VARIABLELIST_CHILD_BEGIN, 0x18000u);
}

void Scr_InitVariableRange(unsigned int begin, unsigned int end)
{
	unsigned int index; // [esp+4h] [ebp-8h]
	VariableValueInternal* value = NULL; // [esp+8h] [ebp-4h]
	VariableValueInternal* valuea; // [esp+8h] [ebp-4h]

	for (index = begin + 1; index < end; ++index)
	{
		value = &scrVarGlob.variableList[index];
		value->w.status = 0;

		iassert(!(value->w.type & VAR_MASK));

		value->hash.id = index - begin;
		value->v.next = index - begin;
		value->u.next = index - begin + 1;
		value->hash.u.prev = index - begin - 1;
	}

	valuea = &scrVarGlob.variableList[begin];
	valuea->w.status = 0;

	iassert(!(valuea->w.type & VAR_MASK));

	valuea->w.status = valuea->w.status;
	valuea->hash.id = 0;
	valuea->v.next = 0;
	valuea->u.next = 1;
	scrVarGlob.variableList[begin + VARIABLELIST_PARENT_BEGIN].hash.u.prev = 0;
	valuea->hash.u.prev = end - begin - 1;
	scrVarGlob.variableList[end - 1].u.next = 0;
}

void Scr_InitClassMap()
{
	for (int classnum = 0; classnum < 4; ++classnum)
	{
		g_classMap[classnum].entArrayId = 0;
		g_classMap[classnum].id = 0;
	}
}

unsigned int Scr_GetNumScriptVars(void)
{
	return scrVarPub.numScriptObjects + scrVarPub.numScriptValues;
}

unsigned int  GetVariableKeyObject(unsigned int id)
{
	iassert(!IsObject(&scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + id]));

	return (scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].w.status >> VAR_NAME_BITS) - SL_MAX_STRING_INDEX;
}

unsigned int  Scr_GetVarId(unsigned int index)
{
	return scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN].hash.id;
}

void  Scr_SetThreadNotifyName(unsigned int startLocalId, unsigned int stringValue)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[startLocalId + VARIABLELIST_PARENT_BEGIN];

	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(((entryValue->w.type & VAR_MASK) == VAR_THREAD));

	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status = (unsigned char)entryValue->w.status;
	entryValue->w.type |= VAR_NOTIFY_THREAD;
	entryValue->w.notifyName |= stringValue << VAR_NAME_BITS;
}

unsigned short  Scr_GetThreadNotifyName(unsigned int startLocalId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + startLocalId].w.type & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + startLocalId].w.type & VAR_MASK) == VAR_NOTIFY_THREAD);

	return scrVarGlob.variableList[startLocalId + VARIABLELIST_PARENT_BEGIN].w.notifyName >> VAR_NAME_BITS;
}

void  Scr_SetThreadWaitTime(unsigned int startLocalId, unsigned int waitTime)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[startLocalId + VARIABLELIST_PARENT_BEGIN];
	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert(((entryValue->w.type & VAR_MASK) == VAR_THREAD) || !Scr_GetThreadNotifyName(startLocalId));

	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status = (unsigned char)entryValue->w.status;
	entryValue->w.status |= 0x10u;

	entryValue->w.waitTime |= waitTime << VAR_NAME_BITS;
}

void  Scr_ClearWaitTime(unsigned int startLocalId)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[startLocalId + VARIABLELIST_PARENT_BEGIN];
	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert((entryValue->w.type & VAR_MASK) == VAR_TIME_THREAD);

	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status |= 0xEu;
}

unsigned int  Scr_GetThreadWaitTime(unsigned int startLocalId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + startLocalId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + startLocalId].w.type & VAR_MASK) == VAR_TIME_THREAD);

	return scrVarGlob.variableList[startLocalId + VARIABLELIST_PARENT_BEGIN].w.waitTime >> VAR_NAME_BITS;
}

unsigned int  GetParentLocalId(unsigned int threadId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) == VAR_CHILD_THREAD);

	return scrVarGlob.variableList[threadId + VARIABLELIST_PARENT_BEGIN].w.parentLocalId >> VAR_NAME_BITS;
}

unsigned int  GetSafeParentLocalId(unsigned int threadId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) >= VAR_THREAD 
		&& (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD);

	if ((scrVarGlob.variableList[threadId + VARIABLELIST_PARENT_BEGIN].w.type & VAR_MASK) == VAR_CHILD_THREAD)
		return scrVarGlob.variableList[threadId + VARIABLELIST_PARENT_BEGIN].w.parentLocalId >> VAR_NAME_BITS;
	else
		return 0;
}

unsigned int  GetStartLocalId(unsigned int threadId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) >= VAR_THREAD 
		&& (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD);

	while ((scrVarGlob.variableList[threadId + VARIABLELIST_PARENT_BEGIN].w.type & VAR_MASK) == VAR_CHILD_THREAD)
		threadId = scrVarGlob.variableList[threadId + VARIABLELIST_PARENT_BEGIN].w.parentLocalId >> VAR_NAME_BITS;

	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) >= VAR_THREAD
		&& (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) <= VAR_TIME_THREAD);

	return threadId;
}

unsigned int  AllocValue(void)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-14h]
	unsigned __int16 newIndex; // [esp+4h] [ebp-10h]
	unsigned __int16 next; // [esp+8h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+Ch] [ebp-8h]
	unsigned __int16 index; // [esp+10h] [ebp-4h]

	index = scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next;

	if (!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next)
		Scr_TerminalError("exceeded maximum number of script variables");

	entry = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
	entryValue = &scrVarGlob.variableList[entry->hash.id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE);

	next = entryValue->u.next;
	if (entry != entryValue && (entry->w.status & VAR_STAT_MASK) == 0)
	{
		newIndex = entry->v.next;
		iassert(newIndex != index);
		scrVarGlob.variableList[newIndex + VARIABLELIST_CHILD_BEGIN].hash.id = entry->hash.id;
		entry->hash.id = index;
		entryValue->v.next = newIndex;
		entryValue->u.next = entry->u.next;
		entryValue = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
	}
	scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next = next;
	scrVarGlob.variableList[next + VARIABLELIST_CHILD_BEGIN].hash.u.prev = 0;
	entryValue->v.next = index;
	entryValue->nextSibling = 0;
	entry->hash.u.prev = 0;

	iassert(entry->hash.id > 0 && entry->hash.id < VARIABLELIST_CHILD_SIZE);

	++scrVarPub.totalObjectRefCount;

	if (scrVarDebugPub)
	{
		iassert(!scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + entry->hash.id]);
		++scrVarDebugPub->leakCount[entry->hash.id + VARIABLELIST_CHILD_BEGIN];
	}
	++scrVarPub.numScriptValues;

	iassert(scrVarPub.varUsagePos);

	if (scrVarDebugPub)
	{
		iassert(!scrVarDebugPub->varUsage[VARIABLELIST_CHILD_BEGIN + entry->hash.id]);
		scrVarDebugPub->varUsage[entry->hash.id + VARIABLELIST_CHILD_BEGIN] = scrVarPub.varUsagePos;
	}
	entryValue->w.status = VAR_STAT_EXTERNAL;

	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.status = (unsigned char)entryValue->w.status;
	return entry->hash.id;
}

unsigned int  AllocObject(void)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.status |= VAR_OBJECT;
	entryValue->u.next = 0;
	return id;
}

unsigned int  Scr_AllocArray(void)
{
	const char* varUsagePos; // [esp+0h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+4h] [ebp-8h]
	unsigned int id; // [esp+8h] [ebp-4h]

	varUsagePos = scrVarPub.varUsagePos;
	if (!scrVarPub.varUsagePos)
		scrVarPub.varUsagePos = "<script array variable>";
	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.type |= VAR_ARRAY;
	
	entryValue->u.next = 0;
	entryValue->u.o.u.size = 0;
	scrVarPub.varUsagePos = varUsagePos;
	return id;
}

unsigned int  AllocThread(unsigned int self)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	id = AllocVariable();

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	iassert(!(entryValue->w.type & VAR_MASK));
	entryValue->w.status |= VAR_THREAD;

	entryValue->u.next = 0;
	entryValue->u.o.u.self = self;

	return id;
}

unsigned int  AllocChildThread(unsigned int self, unsigned int parentLocalId)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN];
	entryValue->w.status = VAR_STAT_EXTERNAL;

	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.status |= VAR_CHILD_THREAD;

	iassert(!(entryValue->w.parentLocalId & VAR_NAME_HIGH_MASK)); 

	entryValue->w.status |= parentLocalId << 8;
	entryValue->u.next = 0;
	entryValue->u.o.u.self = self;

	return id;
}

unsigned int  Scr_GetSelf(unsigned int threadId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) >= VAR_THREAD) &&
		((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD));

	return scrVarGlob.variableList[threadId + VARIABLELIST_PARENT_BEGIN].u.o.u.self;
}

void  AddRefToObject(unsigned int id)
{
	iassert(id >= 1 && id < VARIABLELIST_PARENT_SIZE);

	++scrVarPub.totalObjectRefCount;

	if (scrVarDebugPub)
	{
		iassert(scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]);
		++scrVarDebugPub->leakCount[id + VARIABLELIST_PARENT_BEGIN];
	}
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(IsObject(&scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id]));
	//iassert(scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id].u.o.refCount);
	//iassert(++scrVarGlob.variableList[id + 1].u.next);
	++scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id].u.o.refCount;
	iassert(scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id].u.o.refCount);
}

void  RemoveRefToEmptyObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN];
	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert(IsObject(entryValue));
	iassert(!entryValue->nextSibling);

	if (entryValue->u.next)
	{
		iassert(id >= 1 && id < VARIABLELIST_PARENT_SIZE);
		--scrVarPub.totalObjectRefCount;
		if (scrVarDebugPub)
		{
			iassert(scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]);
			--scrVarDebugPub->leakCount[id + VARIABLELIST_PARENT_BEGIN];
		}
		--entryValue->u.next;
	}
	else
	{
		FreeVariable(id);
	}
}

int  Scr_GetRefCountToObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN];

	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert(IsObject(entryValue));
	return entryValue->u.o.refCount;
}

float const* Scr_AllocVector(float const* v)
{
	float* result;

	result = Scr_AllocVector();
	
	result[0] = v[0];
	result[1] = v[1];
	result[2] = v[2];

	return result;
}

void  AddRefToVector(float const* vectorValue)
{
	if (!*((_BYTE*)vectorValue - 1))
	{
		InterlockedIncrement(&scrVarPub.totalVectorRefCount);
		if (scrStringDebugGlob)
		{
			iassert(scrStringDebugGlob->refCount[((char*)vectorValue - 4 - scrMemTreePub.mt_buffer) / MT_NODE_SIZE] >= 0);
			InterlockedIncrement(&scrStringDebugGlob->refCount[((char*)(vectorValue - 1) - scrMemTreePub.mt_buffer) / 12]);
		}
		((unsigned short*)vectorValue)[-2]++;
		iassert(((unsigned short*)vectorValue)[-2]);
	}
}

void  RemoveRefToVector(float const* vectorValue)
{
	if (!*((_BYTE*)vectorValue - 1))
	{
		InterlockedDecrement(&scrVarPub.totalVectorRefCount);
		if (scrStringDebugGlob)
		{
			iassert(scrStringDebugGlob->refCount[((char*)vectorValue - 4 - scrMemTreePub.mt_buffer) / MT_NODE_SIZE] >= 0);
			InterlockedDecrement(&scrStringDebugGlob->refCount[((char*)(vectorValue - 1) - scrMemTreePub.mt_buffer) / 12]);
		}
		if (*((_WORD*)vectorValue - 2))
			--*((_WORD*)vectorValue - 2);
		else
			MT_Free((_BYTE*)vectorValue - 4, 16);
	}
}

void  AddRefToValue(int type, VariableUnion u)
{
	unsigned int value; // [esp+0h] [ebp-4h]

	value = type - 1;
	if ((unsigned int)(type - 1) < 4)
	{
		if (type == 1)
		{
			AddRefToObject(u.stringValue);
		}
		else if (value > 2)
		{
			if (type != 4)
				MyAssertHandler(
					".\\script\\scr_variable.cpp",
					2393,
					0,
					"%s\n\t(value) = %i",
					"(value == VAR_VECTOR - VAR_BEGIN_REF)",
					value);
			AddRefToVector(u.vectorValue);
		}
		else
		{
			SL_AddRefToString(u.stringValue);
		}
	}
}

void  RemoveRefToValue(int type, VariableUnion u)
{
	switch (type)
	{
	case VAR_POINTER:
		RemoveRefToObject(u.pointerValue);
		break;

	case VAR_STRING:
	case VAR_ISTRING:
		SL_RemoveRefToString(u.stringValue);
		break;

	case VAR_VECTOR:
		iassert(type - 1 == VAR_VECTOR - VAR_BEGIN_REF);
		RemoveRefToVector(u.vectorValue);
		break;
	}
}

bool  IsValidArrayIndex(unsigned int unsignedValue)
{
	return unsignedValue + 0x7E8000 <= 0xFE7FFF;
}

unsigned int  GetInternalVariableIndex(unsigned int unsignedValue)
{
	iassert(IsValidArrayIndex(unsignedValue));

	return (unsignedValue + 0x800000) & 0xFFFFFF;
}

unsigned int  FindArrayVariable(unsigned int parentId, int intValue)
{
	return scrVarGlob.variableList[FindArrayVariableIndex(parentId, intValue) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

unsigned int  FindVariable(unsigned int parentId, unsigned int unsignedValue)
{
	return scrVarGlob.variableList[FindVariableIndexInternal(parentId, unsignedValue) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

unsigned int  FindObjectVariable(unsigned int parentId, unsigned int id)
{
	return scrVarGlob.variableList[FindVariableIndexInternal(parentId, id + 0x10000) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

struct VariableValue  Scr_GetArrayIndexValue(unsigned int name)
{
	VariableValue value; // [esp+0h] [ebp-8h]

	iassert(name);

	if (name >= SL_MAX_STRING_INDEX)
	{
		if (name >= 0x18000)
		{
			value.type = VAR_INTEGER;
			value.u.intValue = name - MAX_ARRAYINDEX;
		}
		else
		{
			value.type = VAR_POINTER;
			value.u.intValue = name - SL_MAX_STRING_INDEX;
		}
	}
	else
	{
		value.type = VAR_STRING;
		value.u.intValue = (uint16_t)name;
	}
	return value;
}

void  SetVariableValue(unsigned int id, struct VariableValue* value)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(id);
	iassert(!IsObject(value));
	iassert(value->type >= 0 && value->type < VAR_COUNT);
	iassert(value->type != VAR_STACK);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	iassert(!IsObject(entryValue));
	iassert((entryValue->w.type & VAR_MASK) != VAR_STACK);

	RemoveRefToValue(entryValue->w.status & 0x1F, entryValue->u.u);
	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status |= value->type;
	entryValue->u.u = value->u;
}

void  SetNewVariableValue(unsigned int id, struct VariableValue* value)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(!IsObject(value));

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(!IsObject(entryValue));
	iassert(value->type >= 0 && value->type < VAR_COUNT);
	iassert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED);
	iassert((value->type != VAR_POINTER) || ((entryValue->w.type & VAR_MASK) < FIRST_DEAD_OBJECT));
	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.type |= value->type;
	entryValue->u.u = value->u;
}

VariableValueInternal_u* GetVariableValueAddress(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]
	iassert(id);
	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert((entryValue->w.type & VAR_MASK) != VAR_UNDEFINED);
	iassert(!IsObject(entryValue));

	return &entryValue->u;
}

void  ClearVariableValue(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(id);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(!IsObject(entryValue));
	iassert((entryValue->w.type & VAR_MASK) != VAR_STACK);

	RemoveRefToValue(entryValue->w.status & 0x1F, entryValue->u.u);

	entryValue->w.status &= 0xFFFFFFE0;

	iassert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED);
}

unsigned int Scr_EvalVariableObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int type; // [esp+4h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

	iassert(((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE) || !id);

	type = entryValue->w.type & VAR_MASK;

	if (type == VAR_POINTER)
	{
		type = scrVarGlob.variableList[entryValue->u.u.intValue + VARIABLELIST_PARENT_BEGIN].w.type & VAR_MASK;
		if (type < VAR_ARRAY)
		{
			iassert(type >= FIRST_OBJECT);
			return entryValue->u.u.pointerValue;
		}
	}

	Scr_Error(va("%s is not a field object", var_typename[type]));
	return 0;
}

unsigned int  GetArraySize(unsigned int id)
{
	iassert(id);
	iassert((scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN].w.status & VAR_MASK) == VAR_ARRAY);

	return scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN].u.o.u.size;
}

unsigned int  FindFirstSibling(unsigned int id)
{
	iassert(id);
	iassert(IsObject(&scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN]));

	return scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN].nextSibling;
}

unsigned int  FindNextSibling(unsigned int id)
{
	unsigned int childId; // [esp+4h] [ebp-8h]
	VariableValueInternal* entryValue; // [esp+8h] [ebp-4h]

	iassert(id);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

	iassert(!IsObject(entryValue));

	if (!entryValue->nextSibling)
		return 0;

	childId = scrVarGlob.variableList[entryValue->nextSibling + VARIABLELIST_CHILD_BEGIN].hash.id;

	iassert(!IsObject(&scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + childId]));

	return childId;
}

unsigned int  FindLastSibling(unsigned int parentId)
{
	VariableValueInternal* parentValue; // [esp+0h] [ebp-10h]
	VariableValueInternal* parent; // [esp+4h] [ebp-Ch]
	unsigned int index; // [esp+8h] [ebp-8h]

	iassert(parentId);
	parentValue = &scrVarGlob.variableList[parentId + VARIABLELIST_PARENT_BEGIN];
	iassert(IsObject(parentValue));
	iassert(((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));

	parent = &scrVarGlob.variableList[parentValue->v.next + VARIABLELIST_PARENT_BEGIN];
	if (!parent->hash.u.prev)
		return 0;
	index = FindVariableIndexInternal(parentId, scrVarGlob.variableList[parent->hash.u.prev + VARIABLELIST_CHILD_BEGIN].w.status >> 8);

	iassert(index);

	return index;
}

unsigned int  FindPrevSibling(unsigned int index)
{
	return scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN].hash.u.prev;
}

unsigned int  GetVariableName(unsigned int id)
{
	iassert(!IsObject(&scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + id]));

	return scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].w.status >> 8;
}

unsigned int GetObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(id);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED || (entryValue->w.type & VAR_MASK) == VAR_POINTER);

	if ((entryValue->w.status & 0x1F) == 0)
	{
		entryValue->w.status |= VAR_POINTER;
		entryValue->u.u.pointerValue = AllocObject();
	}

	iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u.u.pointerValue;
}

unsigned int GetArray(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(id);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED || (entryValue->w.type & VAR_MASK) == VAR_POINTER);

	if ((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED)
	{
		entryValue->w.type |= VAR_POINTER;
		entryValue->u.u.pointerValue = Scr_AllocArray();
	}

	iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u.u.pointerValue;
}

unsigned int FindObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(id);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u.u.pointerValue;
}

bool  IsFieldObject(unsigned int id)
{
	iassert(id);
	iassert(IsObject(&scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN]));

	return (scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN].w.status & VAR_MASK) < VAR_ARRAY;
}

int Scr_IsThreadAlive(unsigned int thread)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(scrVarPub.timeArrayId);

	entryValue = &scrVarGlob.variableList[thread + VARIABLELIST_PARENT_BEGIN];
	
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(((entryValue->w.type & VAR_MASK) >= VAR_THREAD && (entryValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD) || (entryValue->w.type & VAR_MASK) == VAR_DEAD_THREAD);

	return (entryValue->w.status & VAR_MASK) != VAR_DEAD_THREAD;
}

bool  IsObjectFree(unsigned int id)
{
	return (scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN].w.status & VAR_STAT_MASK) == 0;
}

Vartype_t GetValueType(unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	return (Vartype_t)(scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].w.status & VAR_MASK);
}

unsigned int GetObjectType(unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	return scrVarGlob.variableList[id + VARIABLELIST_PARENT_BEGIN].w.type & VAR_MASK;
}

void  Scr_SetClassMap(unsigned int classnum)
{
	iassert(!g_classMap[classnum].entArrayId);
	iassert(!g_classMap[classnum].id);

	g_classMap[classnum].entArrayId = Scr_AllocArray();

	if (scrVarDebugPub)
		++scrVarDebugPub->extRefCount[g_classMap[classnum].entArrayId];

	g_classMap[classnum].id = Scr_AllocArray();

	if (scrVarDebugPub)
		++scrVarDebugPub->extRefCount[g_classMap[classnum].id];
}

int Scr_GetOffset(unsigned int classnum, const char* name)
{
	unsigned int fieldId;

	fieldId = FindVariable(g_classMap[classnum].id, SL_ConvertFromString(name));

	if (fieldId)
	{
		return scrVarGlob.variableList[fieldId + VARIABLELIST_CHILD_BEGIN].u.u.entityOffset;
	}

	return -1;
}

unsigned int FindEntityId(unsigned int entnum, unsigned int classnum)
{
	unsigned int entArrayId; // [esp+0h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+4h] [ebp-8h]
	unsigned int id; // [esp+8h] [ebp-4h]

	iassert((unsigned)entnum < (1 << 16));

	entArrayId = g_classMap[classnum].entArrayId;

	iassert(entArrayId);

	id = FindArrayVariable(entArrayId, entnum);

	if (id)
	{
		entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
		iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
		iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);
		iassert(entryValue->u.u.pointerValue);

		return entryValue->u.u.pointerValue;
	}
	else
	{
		return 0;
	}
}

void  SetEmptyArray(unsigned int parentId)
{
	VariableValue tempValue; // [esp+0h] [ebp-8h] BYREF

	tempValue.type = VAR_POINTER;
	tempValue.u.intValue = Scr_AllocArray();
	SetVariableValue(parentId, &tempValue);
}

void  Scr_AddArrayKeys(unsigned int parentId)
{
	VariableValue ArrayIndexValue_DONE; // rax
	VariableValueInternal* entryValue; // [esp+18h] [ebp-8h]
	unsigned int id; // [esp+1Ch] [ebp-4h]

	iassert(parentId);
	iassert(GetObjectType(parentId) == VAR_ARRAY);

	Scr_MakeArray();

	for (id = FindFirstSibling(parentId); id; id = FindNextSibling(id))
	{
		entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
		iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL);
		iassert(!IsObject(entryValue));

		ArrayIndexValue_DONE = Scr_GetArrayIndexValue(entryValue->w.status >> 8);

		if (ArrayIndexValue_DONE.type == VAR_STRING)
		{
			Scr_AddConstString(ArrayIndexValue_DONE.u.stringValue);
		}
		else if (ArrayIndexValue_DONE.type == VAR_INTEGER)
		{
			Scr_AddInt(ArrayIndexValue_DONE.u.intValue);
		}
		else if (!alwaysfails)
		{
			MyAssertHandler(".\\script\\scr_variable.cpp", 4822, 1, "bad case");
		}
		Scr_AddArray();
	}
}

scr_entref_t Scr_GetEntityIdRef(unsigned int entId)
{
	scr_entref_t entref; // [esp+0h] [ebp-8h]
	VariableValueInternal* entValue; // [esp+4h] [ebp-4h]

	entValue = &scrVarGlob.variableList[entId + 1];

	iassert((entValue->w.type & VAR_MASK) == VAR_ENTITY);
	iassert((entValue->w.name >> VAR_NAME_BITS) < CLASS_NUM_COUNT);

	entref.entnum = entValue->u.o.u.size;
	entref.classnum = entValue->w.status >> 8;
	return entref;
}

unsigned int  Scr_FindField(char const* name, int* type)
{
	const char* pos; // [esp+10h] [ebp-Ch]
	int len; // [esp+14h] [ebp-8h]
	unsigned int index; // [esp+18h] [ebp-4h]

	iassert(scrVarPub.fieldBuffer);

	for (pos = scrVarPub.fieldBuffer; *pos; pos += len + 3)
	{
		len = strlen(pos) + 1;

		if (!I_stricmp(name, pos))
		{
			index = *(uint16_t *)&pos[len];
			*type = pos[len + 2];
			return index;
		}
	}
	return 0;
}

void __cdecl Scr_AddFields_LoadObj(const char *path, const char *extension)
{
	char filename[68]; // [esp+10h] [ebp-58h] BYREF
	int numFiles; // [esp+58h] [ebp-10h] BYREF
	char *targetPos; // [esp+5Ch] [ebp-Ch]
	int i; // [esp+60h] [ebp-8h]
	const char **files; // [esp+64h] [ebp-4h]

	files = FS_ListFiles(path, extension, FS_LIST_PURE_ONLY, &numFiles);
	scrVarPub.fieldBuffer = TempMalloc(0);
	*scrVarPub.fieldBuffer = 0;
	for (i = 0; i < numFiles; ++i)
	{
		sprintf(filename, "%s/%s", path, files[i]);
		if (strlen(filename) >= 0x40)
			MyAssertHandler(".\\script\\scr_variable.cpp", 5191, 0, "%s", "strlen( filename ) < MAX_QPATH");
		Scr_AddFieldsForFile(filename);
	}
	if (files)
		FS_FreeFileList(files);
	targetPos = TempMalloc(1u);
	*targetPos = 0;
}

void  Scr_AddFields(char const* path, char const* extension)
{
	if (useFastFile->current.enabled)
		Scr_AddFields_FastFile(path, extension);
	else
		Scr_AddFields_LoadObj(path, extension);
}

void  Scr_AllocGameVariable(void)
{
	if (!scrVarPub.gameId)
	{
		iassert(!scrVarPub.varUsagePos);
		scrVarPub.varUsagePos = "<game variable>";
		scrVarPub.gameId = AllocValue();
		SetEmptyArray(scrVarPub.gameId);
		scrVarPub.varUsagePos = 0;
		Scr_UpdateDebugger();
	}
}

int  Scr_GetClassnumForCharId(char charId)
{
	int i; // [esp+0h] [ebp-4h]

	for (i = 0; i < 4; ++i)
	{
		if (g_classMap[i].charId == charId)
			return i;
	}
	return -1;
}

unsigned int  Scr_FindAllThreads(unsigned int selfId, unsigned int* threads, unsigned int localId)
{
	VariableValueInternal_u Object{ 0 }; // eax
	unsigned int stackId; // [esp+4h] [ebp-1Ch]
	unsigned int timeId; // [esp+8h] [ebp-18h]
	unsigned int notifyListId; // [esp+Ch] [ebp-14h]
	unsigned int notifyListIda; // [esp+Ch] [ebp-14h]
	unsigned int threadId; // [esp+10h] [ebp-10h]
	unsigned int threadIda; // [esp+10h] [ebp-10h]
	VariableValueInternal* entryValue; // [esp+14h] [ebp-Ch]
	int count; // [esp+18h] [ebp-8h]
	unsigned int id; // [esp+1Ch] [ebp-4h]

	count = 0;
	if (localId && selfId == Scr_GetSelf(localId))
	{
		if (threads)
			*threads = localId;
		count = 1;
	}
	for (id = 1; id < 0xFFFE; ++id)
	{
		entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
		if ((entryValue->w.status & 0x60) != 0 && (entryValue->w.status & 0x1F) == 0xA)
		{
			for (threadId = *(unsigned int*)(entryValue->u.u.intValue + 8);
				threadId;
				threadId = GetSafeParentLocalId(threadId))
			{
				if (selfId == Scr_GetSelf(threadId))
				{
					if (threads)
						threads[count] = threadId;
					++count;
					break;
				}
			}
		}
	}
	notifyListId = FindVariable(selfId, 0x18000u);
	if (notifyListId)
	{
		Object = FindObject(notifyListId);
		for (notifyListIda = FindFirstSibling(Object.u.stringValue);
			notifyListIda;
			notifyListIda = FindNextSibling(notifyListIda))
		{
			timeId = FindObject(notifyListIda);
			iassert(timeId);
			for (stackId = FindFirstSibling(timeId); stackId; stackId = FindNextSibling(stackId))
			{
				if (GetValueType(stackId) == 10)
				{
					for (threadIda = *(unsigned int*)(GetVariableValueAddress(stackId)->u.intValue + 8);
						threadIda;
						threadIda = GetSafeParentLocalId(threadIda))
					{
						if (selfId != Scr_GetSelf(threadIda))
						{
							if (threads)
								threads[count] = threadIda;
							++count;
							break;
						}
					}
				}
			}
		}
	}
	return count;
}

unsigned int  Scr_FindAllEndons(unsigned int threadId, unsigned int* names)
{
	unsigned int localId; // [esp+0h] [ebp-20h]
	VariableValueInternal_u selfNameId{ 0 }; // [esp+4h] [ebp-1Ch]
	unsigned int name; // [esp+8h] [ebp-18h]
	VariableValueInternal* threadValue; // [esp+Ch] [ebp-14h]
	unsigned int count; // [esp+14h] [ebp-Ch]
	unsigned int id; // [esp+18h] [ebp-8h]
	unsigned int notifyListEntry; // [esp+1Ch] [ebp-4h]

	iassert(threadId);

	threadValue = &scrVarGlob.variableList[threadId + 1];

	iassert((threadValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert(((threadValue->w.type & VAR_MASK) >= VAR_THREAD) && ((threadValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD));
	count = 0;
	for (localId = threadId; localId; localId = GetSafeParentLocalId(localId))
	{
		id = FindObjectVariable(scrVarPub.pauseArrayId, localId);
		if (id)
		{
			selfNameId = FindObject(id);
			for (notifyListEntry = FindFirstSibling(selfNameId.u.stringValue);
				notifyListEntry;
				notifyListEntry = FindNextSibling(notifyListEntry))
			{
				iassert((scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + notifyListEntry].w.type & VAR_MASK) == VAR_POINTER);
				name = scrVarGlob.variableList[notifyListEntry + VARIABLELIST_CHILD_BEGIN].w.status >> 8;
				iassert((name - SL_MAX_STRING_INDEX) < (1 << 16));
				if (names)
					names[count] = Scr_GetThreadNotifyName(name);
				++count;
			}
		}
	}
	return count;
}

void  Scr_DumpScriptVariables(bool spreadsheet,
	bool summary,
	bool total,
	bool functionSummary,
	bool lineSort,
	const char* fileName,
	const char* functionName,
	int minCount)
{
	unsigned int NumScriptVars; // eax
	const char* pos; // [esp+0h] [ebp-24h]
	int(__cdecl * VariableInfoCompareCallBack)(const void*, const void*); // [esp+4h] [ebp-20h]
	unsigned int index; // [esp+8h] [ebp-1Ch]
	VariableDebugInfo* pInfo; // [esp+Ch] [ebp-18h]
	VariableDebugInfo* pInfoa; // [esp+Ch] [ebp-18h]
	VariableDebugInfo* pInfob; // [esp+Ch] [ebp-18h]
	signed int num; // [esp+10h] [ebp-14h]
	int filteredCount; // [esp+14h] [ebp-10h]
	int i; // [esp+18h] [ebp-Ch]
	int ia; // [esp+18h] [ebp-Ch]
	VariableDebugInfo* infoArray; // [esp+1Ch] [ebp-8h]
	int count; // [esp+20h] [ebp-4h]

	if (scrVarDebugPub
		&& (scrVarPub.developer || !spreadsheet && !fileName && !functionName && !lineSort && !functionSummary && !minCount))
	{
		infoArray = (VariableDebugInfo*)Z_TryVirtualAlloc(1572864, "Scr_DumpScriptVariables", 0);
		if (infoArray)
		{
			num = 0;
			for (index = 0; index < 0x18000; ++index)
			{
				pos = scrVarDebugPub->varUsage[index];
				if (pos)
				{
					pInfo = &infoArray[num];
					if (!fileName || Scr_PrevCodePosFileNameMatches((char*)pos, fileName))
					{
						if (functionName || functionSummary)
							pInfo->functionName = Scr_PrevCodePosFunctionName((char *)pos);
						else
							pInfo->functionName = 0;
						if (!functionName || pInfo->functionName && I_stristr(pInfo->functionName, functionName))
						{
							pInfo->pos = pos;
							pInfo->fileName = Scr_PrevCodePosFileName((char *)pos);
							pInfo->varUsage = 1;
							++num;
						}
					}
				}
			}
			if (total)
			{
				Com_Printf(0, "num vars:          %d\n", num);
				Z_VirtualFree(infoArray);
			}
			else
			{
				if (summary)
				{
					VariableInfoCompareCallBack = (int(*)(const void *, const void *))VariableInfoFileNameCompare;
					qsort(infoArray, num, 0x10u, (int(*)(const void *, const void *))VariableInfoFileNameCompare);
				}
				else if (functionSummary)
				{
					VariableInfoCompareCallBack = (int(*)(const void *, const void *))VariableInfoFunctionCompare;
					qsort(infoArray, num, 0x10u, (int(*)(const void *, const void *))VariableInfoFunctionCompare);
				}
				else
				{
					VariableInfoCompareCallBack = (int(*)(const void *, const void *))CompareThreadIndices;
					qsort(infoArray, num, 0x10u, (int(*)(const void *, const void *))CompareThreadIndices);
				}
				i = 0;
				while (i < num)
				{
					pInfoa = &infoArray[i];
					do
					{
						++pInfoa->varUsage;
						--infoArray[i++].varUsage;
					} while (i < num && !VariableInfoCompareCallBack(pInfoa, &infoArray[i]));
				}
				if (lineSort)
					qsort(infoArray, num, 0x10u, (int(*)(const void *, const void *))VariableInfoFileLineCompare);
				else
					qsort(infoArray, num, 0x10u, (int(*)(const void *, const void *))VariableInfoCountCompare);
				Com_Printf(23, "********************************\n");
				if (spreadsheet)
				{
					if (summary)
					{
						Com_Printf(0, "count\tfile\n");
					}
					else if (functionSummary)
					{
						Com_Printf(0, "count\tfile\tfunction\n");
					}
					else
					{
						Com_Printf(0, "count\tfile\tline\tsource\tcol\n");
					}
				}
				count = 0;
				filteredCount = 0;
				for (ia = 0; ia < num; ++ia)
				{
					pInfob = &infoArray[ia];
					if (pInfob->varUsage)
					{
						count += pInfob->varUsage;
						if (pInfob->varUsage >= minCount)
						{
							filteredCount += pInfob->varUsage;
							if (spreadsheet)
							{
								Com_Printf(0, "%d\t", pInfob->varUsage);
								Scr_PrintPrevCodePosSpreadSheet(0, (char *)pInfob->pos, summary, functionSummary);
							}
							else
							{
								if (summary)
									MyAssertHandler(".\\script\\scr_variable.cpp", 746, 0, "%s", "!summary");
								Com_Printf(0, "count: %d\n", pInfob->varUsage);
								Scr_PrintPrevCodePos(0, (char*)pInfob->pos, 0);
							}
						}
					}
				}
				if (num != count)
					MyAssertHandler(".\\script\\scr_variable.cpp", 753, 0, "%s", "num == count");
				Com_Printf(0, "********************************\n");
				Com_Printf(0, "num vars:          %d\n", filteredCount);
				NumScriptVars = Scr_GetNumScriptVars();
				Com_Printf(0, "num unlisted vars: %d\n", NumScriptVars - filteredCount);
				Com_Printf(0, "********************************\n");
				Z_VirtualFree(infoArray);
			}
		}
		else
		{
			Com_Printf(23, "Cannot dump script variables: out of memory\n");
		}
	}
}

unsigned int  GetVariableIndexInternal(unsigned int parentId, unsigned int name)
{
	unsigned int newIndex; // [esp+8h] [ebp-8h]

	iassert(parentId);
	iassert((scrVarGlob.variableList[parentId + 1].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert(IsObject(&scrVarGlob.variableList[parentId + 1]));

	newIndex = FindVariableIndexInternal2(name, (parentId + 101 * name) % 0xFFFD + 1);
	if (newIndex)
		return newIndex;
	else
		return GetNewVariableIndexInternal2(parentId, name, (parentId + 101 * name) % 0xFFFD + 1);
}

void ClearObject(unsigned int parentId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) != VAR_ENTITY) 
		&& !((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) >= FIRST_OBJECT 
			&& (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) < FIRST_CLEARABLE_OBJECT)
	);
	
	AddRefToObject(parentId);
	ClearObjectInternal(parentId);
	RemoveRefToEmptyObject(parentId);
}

void  Scr_RemoveThreadNotifyName(unsigned int startLocalId)
{
	unsigned __int16 stringValue; // [esp+0h] [ebp-8h]
	VariableValueInternal* entryValue; // [esp+4h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[startLocalId + 1];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert((entryValue->w.type & VAR_MASK) == VAR_NOTIFY_THREAD);

	stringValue = Scr_GetThreadNotifyName(startLocalId);

	iassert(stringValue);

	SL_RemoveRefToString(stringValue);
	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status |= 0xEu;
}

void  FreeValue(unsigned int id)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+4h] [ebp-8h]
	unsigned __int16 index; // [esp+8h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert(!IsObject(entryValue));
	iassert(scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + entryValue->v.index].hash.id == id);

	RemoveRefToValue(entryValue->w.status & VAR_MASK, entryValue->u.u);

	iassert(id > 0 && id < VARIABLELIST_CHILD_SIZE);

	--scrVarPub.totalObjectRefCount;

	if (scrVarDebugPub)
	{
		iassert(scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + id]);
		scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + id]--;
		iassert(!scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + id]);
	}
	--scrVarPub.numScriptValues;
	if (scrVarDebugPub)
	{
		iassert(scrVarDebugPub->varUsage[VARIABLELIST_CHILD_BEGIN + id]);
		scrVarDebugPub->varUsage[id + VARIABLELIST_CHILD_BEGIN] = 0;
	}
	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));

	index = entryValue->v.next;
	entry = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];

	iassert(entry->hash.id == id);
	iassert(!entry->hash.u.prev);
	iassert(!entryValue->nextSibling);

	entryValue->w.status = 0;
	entryValue->u.next = scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next;
	entry->hash.u.prev = 0;
	scrVarGlob.variableList[scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next + VARIABLELIST_CHILD_BEGIN].hash.u.prev = index;
	scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next = index;
}

unsigned int GetArrayVariableIndex(unsigned int parentId, unsigned int unsignedValue)
{
	iassert(IsValidArrayIndex(unsignedValue));
	return GetVariableIndexInternal(parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

unsigned int  Scr_GetVariableFieldIndex(unsigned int parentId, unsigned int name)
{
	unsigned int index; // [esp+4h] [ebp-8h]
	unsigned int type; // [esp+8h] [ebp-4h]

	iassert(parentId);
	iassert(&scrVarGlob.variableList[parentId + 1]);

	type = scrVarGlob.variableList[parentId + 1].w.type & VAR_MASK;

	if (type <= VAR_OBJECT)
		return GetVariableIndexInternal(parentId, name);

	if (type == VAR_ENTITY)
	{
		index = FindVariableIndexInternal(parentId, name);
		if (index)
		{
			return index;
		}
		else
		{
			scrVarPub.entId = parentId;
			scrVarPub.entFieldName = name;
			return 0;
		}
	}
	else
	{
		Scr_Error(va("cannot set field of %s", var_typename[type]));
		return 0;
	}
}

unsigned int  Scr_FindAllVariableField(unsigned int parentId, unsigned int* names)
{
	unsigned int classnum; // [esp+4h] [ebp-1Ch]
	VariableValueInternal* parentValue; // [esp+8h] [ebp-18h]
	unsigned int name; // [esp+10h] [ebp-10h]
	unsigned int namea; // [esp+10h] [ebp-10h]
	unsigned int nameb; // [esp+10h] [ebp-10h]
	unsigned int count; // [esp+18h] [ebp-8h]
	unsigned int id; // [esp+1Ch] [ebp-4h]
	unsigned int ida; // [esp+1Ch] [ebp-4h]
	unsigned int idb; // [esp+1Ch] [ebp-4h]

	parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(IsObject(parentValue));

	count = 0;

	switch (parentValue->w.status & 0x1F)
	{
	case 0xEu:
	case 0xFu:
	case 0x10u:
	case 0x11u:
	case 0x12u:
	case 0x13u:
		goto $LN18_17;
	case 0x14u:
		classnum = parentValue->w.status >> 8;
		if (classnum >= 4)
			MyAssertHandler(".\\script\\scr_variable.cpp", 2558, 0, "%s", "classnum < CLASS_NUM_COUNT");
		for (id = FindFirstSibling(g_classMap[classnum].id); id; id = FindNextSibling(id))
		{
			name = (scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].w.status >> 8) - 0x800000;
			if (scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].w.status >> 8 == 0x800000)
				MyAssertHandler(".\\script\\scr_variable.cpp", 2566, 0, "%s", "name");
			if (name <= scrVarPub.canonicalStrCount && !FindVariable(parentId, name))
			{
				if (names)
					names[count] = name;
				++count;
			}
		}
	$LN18_17:
		for (ida = FindFirstSibling(parentId); ida; ida = FindNextSibling(ida))
		{
			namea = scrVarGlob.variableList[ida + VARIABLELIST_CHILD_BEGIN].w.status >> 8;
			iassert(namea);
			if (namea != 98304 && namea != 98305)
			{
				iassert(namea <= scrVarPub.canonicalStrCount);
				if (names)
					names[count] = namea;
				++count;
			}
		}
		break;
	case 0x15u:
		for (idb = FindFirstSibling(parentId); idb; idb = FindNextSibling(idb))
		{
			nameb = scrVarGlob.variableList[idb + VARIABLELIST_CHILD_BEGIN].w.status >> 8;
			iassert(nameb);
			if (names)
				names[count] = nameb;
			++count;
		}
		break;
	default:
		return count;
	}
	return count;
}

unsigned int GetArrayVariable(unsigned int parentId, unsigned int unsignedValue)
{
	return scrVarGlob.variableList[GetArrayVariableIndex(parentId, unsignedValue) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

unsigned int  GetNewArrayVariable(unsigned int parentId, unsigned int unsignedValue)
{
	return scrVarGlob.variableList[GetNewArrayVariableIndex(parentId, unsignedValue) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

unsigned int  GetVariable(unsigned int parentId, unsigned int unsignedValue)
{
	return scrVarGlob.variableList[GetVariableIndexInternal(parentId, unsignedValue) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

unsigned int  GetNewVariable(unsigned int parentId, unsigned int unsignedValue)
{
	return scrVarGlob.variableList[GetNewVariableIndexInternal(parentId, unsignedValue) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

unsigned int  GetObjectVariable(unsigned int parentId, unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[GetVariableIndexInternal(parentId, id + 0x10000) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

unsigned int  GetNewObjectVariable(unsigned int parentId, unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[GetNewVariableIndexInternal(parentId, id + 0x10000) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

unsigned int  GetNewObjectVariableReverse(unsigned int parentId, unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[GetNewVariableIndexReverseInternal(parentId, id + 0x10000) + VARIABLELIST_CHILD_BEGIN].hash.id;
}

void  RemoveVariable(unsigned int parentId, unsigned int unsignedValue)
{
	unsigned int index; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	index = FindVariableIndexInternal(parentId, unsignedValue);
	iassert(index);
	id = scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN].hash.id;
	MakeVariableExternal(index, &scrVarGlob.variableList[parentId + 1]);
	FreeChildValue(parentId, id);
}

void  RemoveNextVariable(unsigned int parentId)
{
	unsigned int index; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	id = scrVarGlob.variableList[parentId + 1].nextSibling;

	if (!scrVarGlob.variableList[parentId + 1].nextSibling)
		MyAssertHandler(".\\script\\scr_variable.cpp", 2747, 0, "%s", "id");

	index = FindVariableIndexInternal(parentId, scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].w.status >> 8);
	iassert(index);
	iassert(id == scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + index].hash.id);

	MakeVariableExternal(index, &scrVarGlob.variableList[parentId + 1]);
	FreeChildValue(parentId, id);
}

void  RemoveObjectVariable(unsigned int parentId, unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) == VAR_ARRAY);
	RemoveVariable(parentId, id + 0x10000);
}

void  SafeRemoveVariable(unsigned int parentId, unsigned int unsignedValue)
{
	unsigned int index; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	index = FindVariableIndexInternal(parentId, unsignedValue);
	if (index)
	{
		id = scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN].hash.id;
		iassert(!IsObject(&scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + id]));
		MakeVariableExternal(index, &scrVarGlob.variableList[parentId + 1]);
		FreeChildValue(parentId, id);
	}
}

void  RemoveVariableValue(unsigned int parentId, unsigned int index)
{
	unsigned int id; // [esp+0h] [ebp-4h]

	iassert(index);
	id = scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN].hash.id;
	iassert(id);

	MakeVariableExternal(index, &scrVarGlob.variableList[parentId + 1]);
	FreeChildValue(parentId, id);
}

void  SetVariableEntityFieldValue(unsigned int entId, unsigned int fieldName, VariableValue* value)
{
	VariableValueInternal* entValue; // [esp+0h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+4h] [ebp-8h]
	unsigned int fieldId; // [esp+8h] [ebp-4h]

	iassert(!IsObject(value));
	iassert(value->type != VAR_STACK);

	entValue = &scrVarGlob.variableList[entId + 1];

	iassert((entValue->w.type & VAR_MASK) == VAR_ENTITY);
	iassert((entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT);
	fieldId = FindArrayVariable(g_classMap[entValue->w.status >> 8].id, fieldName);
	if (!fieldId
		|| !SetEntityFieldValue(
			entValue->w.status >> 8,
			entValue->u.o.u.size,
			scrVarGlob.variableList[fieldId + VARIABLELIST_CHILD_BEGIN].u.u.intValue,
			value))
	{
		entryValue = &scrVarGlob.variableList[GetNewVariable(entId, fieldName) + VARIABLELIST_CHILD_BEGIN];
		iassert(!(entryValue->w.type & VAR_MASK));

		entryValue->w.status |= value->type;
		entryValue->u.u.intValue = value->u.intValue;
	}
}

void  SetVariableFieldValue(unsigned int id, VariableValue* value)
{
	if (id)
		SetVariableValue(id, value);
	else
		SetVariableEntityFieldValue(scrVarPub.entId, scrVarPub.entFieldName, value);
}

VariableValue  Scr_EvalVariable(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-Ch]
	VariableValue value; // [esp+4h] [ebp-8h] BYREF

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	iassert(((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE) || !id);
	value.type = (Vartype_t)(entryValue->w.type & VAR_MASK);
	value.u = entryValue->u.u;
	iassert(!IsObject(&value));

	AddRefToValue(value.type, value.u);

	return value;
}

void  Scr_EvalBoolComplement(VariableValue* value)
{
	int type;

	if (value->type == VAR_INTEGER)
	{
		value->u.intValue = ~value->u.intValue;
	}
	else
	{
		type = value->type;
		RemoveRefToValue(type, value->u);
		value->type = VAR_UNDEFINED;
		Scr_Error(va("~ cannot be applied to \"%s\"", var_typename[type]));
	}
}

void  Scr_CastBool(VariableValue* value)
{
	if (value->type == VAR_INTEGER)
	{
		value->u.intValue = value->u.intValue != 0;
	}
	else if (value->type == VAR_FLOAT)
	{
		value->type = VAR_INTEGER;
		value->u.intValue = value->u.floatValue != 0.0;
	}
	else
	{
		//RemoveRefToValue(value->type, value->u);
		//value->type = VAR_UNDEFINED;
		Scr_Error(va("cannot cast %s to bool", var_typename[value->type]));
	}
}

bool  Scr_CastString(VariableValue* value)
{
	const float* constTempVector; // [esp+4h] [ebp-4h]

	switch (value->type)
	{
	case VAR_STRING:
		return true;

	case VAR_INTEGER:
		value->type = VAR_STRING;
		value->u.stringValue = SL_GetStringForInt(value->u.intValue);
		return true;

	case VAR_FLOAT:
		value->type = VAR_STRING;
		value->u.stringValue = SL_GetStringForFloat(value->u.floatValue);
		return true;

	case VAR_VECTOR:
		value->type = VAR_STRING;
		constTempVector = value->u.vectorValue;
		value->u.stringValue = SL_GetStringForVector(value->u.vectorValue);
		RemoveRefToVector(constTempVector);
		return true;

	default:
		scrVarPub.error_message = va("cannot cast %s to string", var_typename[value->type]);
		RemoveRefToValue(value->type, value->u);
		value->type = VAR_UNDEFINED;
		return false;
	}
}

void  Scr_CastDebugString(VariableValue* value)
{
	const XAnim_s* Anims; // eax
	HashEntry_unnamed_type_u v2{ 0 }; // eax
	unsigned int intValue; // [esp-4h] [ebp-18h]
	unsigned int stringValue; // [esp+8h] [ebp-Ch]
	char* s; // [esp+10h] [ebp-4h]
	char* sa; // [esp+10h] [ebp-4h]

	switch (value->type)
	{
	case VAR_POINTER:
		sa = (char*)var_typename[GetObjectType(value->u.intValue)];
		v2.prev = SL_GetString_(sa, 0, 15);
		goto LABEL_7;
	case VAR_STRING:
	case VAR_VECTOR:
	case VAR_END_REF:
	case VAR_INTEGER:
		Scr_CastString(value);
		return;
	case VAR_ISTRING:
		value->type = VAR_STRING;
		return;
	case VAR_ANIMATION:
		intValue = value->u.intValue;
		Anims = Scr_GetAnims(HIWORD(value->u.intValue));
		s = XAnimGetAnimDebugName(Anims, intValue);
		v2.prev = SL_GetString_(s, 0, 15);
		goto LABEL_7;
	default:
		v2.prev = SL_GetString_((char*)var_typename[value->type], 0, 15);
	LABEL_7:
		stringValue = v2.prev;
		RemoveRefToValue(value->type, value->u);
		value->type = VAR_STRING;
		value->u.intValue = stringValue;
		return;
	}
}

char  Scr_GetEntClassId(unsigned int id) 
{
	iassert(GetObjectType(id) == VAR_ENTITY);
	return g_classMap[scrVarGlob.variableList[id + 1].w.status >> 8].charId;
}

int  Scr_GetEntNum(unsigned int id)
{
	iassert(GetObjectType(id) == VAR_ENTITY);
	return scrVarGlob.variableList[id + 1].u.o.u.size;
}

void  Scr_ClearVector(VariableValue* value)
{
	for (int i = 2; i >= 0; --i)
		RemoveRefToValue(value[i].type, value[i].u);

	value->type = VAR_UNDEFINED;
}

void  Scr_CastVector(VariableValue* value)
{
	int type, i;
	float vec[3];

	for (i = 2; i >= 0; i--)
	{
		type = value[i].type;

		switch (type)
		{
		case VAR_FLOAT:
			vec[2 - i] = value[i].u.floatValue;
			break;

		case VAR_INTEGER:
			vec[2 - i] = (float)value[i].u.intValue;
			break;

		default:
			scrVarPub.error_index = i + 1;
			Scr_ClearVector(value);
			Scr_Error(va("type %s is not a float", var_typename[type]));
			return;
		}
	}
	value->type = VAR_VECTOR;
	value->u.vectorValue = Scr_AllocVector(vec);
}

unsigned int  Scr_EvalFieldObject(unsigned int tempVariable, VariableValue* value)
{
	unsigned int type; // [esp+0h] [ebp-Ch]
	VariableValue tempValue; // [esp+4h] [ebp-8h] BYREF

	type = value->type;

	if (type == VAR_POINTER)
	{
		type = scrVarGlob.variableList[value->u.intValue + VARIABLELIST_PARENT_BEGIN].w.type & VAR_MASK;
		if (type < VAR_ARRAY)
		{
			iassert(type >= FIRST_OBJECT);

			tempValue.type = VAR_POINTER;
			tempValue.u.intValue = value->u.intValue;

			SetVariableValue(tempVariable, &tempValue);
			return tempValue.u.pointerValue;
		}
	}

	RemoveRefToValue(value->type, value->u);
	Scr_Error(va("%s is not a field object", var_typename[type]));
	return 0;
}

void  Scr_UnmatchingTypesError(VariableValue* value1, VariableValue* value2)
{
	char* v2; // eax
	char* v3; // [esp-Ch] [ebp-18h]
	const char* v4; // [esp-8h] [ebp-14h]
	const char* v5; // [esp-4h] [ebp-10h]
	int type1; // [esp+0h] [ebp-Ch]
	int type2; // [esp+4h] [ebp-8h]
	char* error_message; // [esp+8h] [ebp-4h]

	if (scrVarPub.error_message)
	{
		error_message = 0;
	}
	else
	{
		type1 = value1->type;
		type2 = value2->type;
		Scr_CastDebugString(value1);
		Scr_CastDebugString(value2);
		iassert(value1->type == VAR_STRING);
		iassert(value2->type == VAR_STRING);
		v5 = var_typename[type2];
		v4 = var_typename[type1];
		v3 = SL_ConvertToString(value2->u.intValue);
		v2 = SL_ConvertToString(value1->u.intValue);
		error_message = va("pair '%s' and '%s' has unmatching types '%s' and '%s'", v2, v3, v4, v5);
	}
	RemoveRefToValue(value1->type, value1->u);
	value1->type = VAR_UNDEFINED;
	RemoveRefToValue(value2->type, value2->u);
	value2->type = VAR_UNDEFINED;
	Scr_Error(error_message);
}

void  Scr_EvalOr(VariableValue* value1, VariableValue* value2)
{
	if (value1->type == 6 && value2->type == 6)
		value1->u.intValue |= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}

void  Scr_EvalExOr(VariableValue* value1, VariableValue* value2)
{
	if (value1->type == 6 && value2->type == 6)
		value1->u.intValue ^= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}
void  Scr_EvalAnd(VariableValue* value1, VariableValue* value2)
{
	if (value1->type == 6 && value2->type == 6)
		value1->u.intValue &= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}
void  Scr_EvalLess(VariableValue* value1, VariableValue* value2)
{
	int type; // [esp+4h] [ebp-4h]

	Scr_CastWeakerPair(value1, value2);

	iassert(value1->type == value2->type);

	type = value1->type;
	if (type == VAR_FLOAT)
	{
		value1->type = VAR_INTEGER;
		value1->u.intValue = value2->u.floatValue > (double)value1->u.floatValue;
	}
	else if (type == VAR_INTEGER)
	{
		value1->u.intValue = value1->u.intValue < value2->u.intValue;
	}
	else
	{
		Scr_UnmatchingTypesError(value1, value2);
	}
}
void  Scr_EvalGreaterEqual(VariableValue* value1, VariableValue* value2)
{
	Scr_EvalLess(value1, value2);
	iassert((value1->type == VAR_INTEGER) || (value1->type == VAR_UNDEFINED));
	value1->u.intValue = value1->u.intValue == 0;
}
void  Scr_EvalGreater(VariableValue* value1, VariableValue* value2)
{
	int type; // [esp+4h] [ebp-4h]

	Scr_CastWeakerPair(value1, value2);

	iassert(value1->type == value2->type);

	type = value1->type;
	if (type == VAR_FLOAT)
	{
		value1->type = VAR_INTEGER;
		value1->u.intValue = value2->u.floatValue < (double)value1->u.floatValue;
	}
	else if (type == VAR_INTEGER)
	{
		value1->u.intValue = value1->u.intValue > value2->u.intValue;
	}
	else
	{
		Scr_UnmatchingTypesError(value1, value2);
	}
}
void  Scr_EvalLessEqual(VariableValue* value1, VariableValue* value2)
{
	Scr_EvalGreater(value1, value2);
	iassert((value1->type == VAR_INTEGER) || (value1->type == VAR_UNDEFINED));
	value1->u.intValue = value1->u.intValue == 0;
}
void  Scr_EvalShiftLeft(VariableValue* value1, VariableValue* value2)
{
	if (value1->type == 6 && value2->type == 6)
		value1->u.intValue <<= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}
void  Scr_EvalShiftRight(VariableValue* value1, VariableValue* value2)
{
	if (value1->type == 6 && value2->type == 6)
		value1->u.intValue >>= value2->u.intValue;
	else
		Scr_UnmatchingTypesError(value1, value2);
}
void  Scr_EvalPlus(VariableValue* value1, VariableValue* value2)
{
	const char* v2; // eax
	char v3; // [esp+3h] [ebp-203Dh]
	char* v4; // [esp+8h] [ebp-2038h]
	char* v5; // [esp+Ch] [ebp-2034h]
	char v6; // [esp+13h] [ebp-202Dh]
	char* v7; // [esp+18h] [ebp-2028h]
	char* v8; // [esp+1Ch] [ebp-2024h]
	VariableUnion v9; // [esp+24h] [ebp-201Ch]
	int StringLen_DONE; // [esp+28h] [ebp-2018h]
	float* v11; // [esp+2Ch] [ebp-2014h]
	char* v12; // [esp+30h] [ebp-2010h]
	char* v13; // [esp+34h] [ebp-200Ch]
	char str[8192]; // [esp+38h] [ebp-2008h] BYREF
	unsigned int len; // [esp+203Ch] [ebp-4h]

	Scr_CastWeakerStringPair(value1, value2);
	iassert(value1->type == value2->type);
	switch (value1->type)
	{
	case 2:
		v12 = SL_ConvertToString(value1->u.intValue);
		v13 = SL_ConvertToString(value2->u.intValue);
		StringLen_DONE = SL_GetStringLen(value1->u.intValue);
		len = StringLen_DONE + SL_GetStringLen(value2->u.intValue) + 1;
		if ((int)len <= 0x2000)
		{
			v8 = v12;
			v7 = str;
			do
			{
				v6 = *v8;
				*v7++ = *v8++;
			} while (v6);
			v5 = v13;
			v4 = &str[StringLen_DONE];
			do
			{
				v3 = *v5;
				*v4++ = *v5++;
			} while (v3);
			v9.stringValue = SL_GetStringOfSize(str, 0, len, 15).prev;
			SL_RemoveRefToString(value1->u.intValue);
			SL_RemoveRefToString(value2->u.intValue);
			value1->u = v9;
		}
		else
		{
			SL_RemoveRefToString(value1->u.intValue);
			SL_RemoveRefToString(value2->u.intValue);
			value1->type = VAR_UNDEFINED;
			value2->type = VAR_UNDEFINED;
			v2 = va("cannot concat \"%s\" and \"%s\" - max string length exceeded", v12, v13);
			//Scr_Error(v2);
		}
		break;
	case 4:
		v11 = Scr_AllocVector();
		*v11 = *(float*)value1->u.intValue + *(float*)value2->u.intValue;
		v11[1] = *(float*)(value1->u.intValue + 4) + *(float*)(value2->u.intValue + 4);
		v11[2] = *(float*)(value1->u.intValue + 8) + *(float*)(value2->u.intValue + 8);
		RemoveRefToVector(value1->u.vectorValue);
		RemoveRefToVector(value2->u.vectorValue);
		value1->u.intValue = (int)v11;
		break;
	case 5:
		value1->u.floatValue = value1->u.floatValue + value2->u.floatValue;
		break;
	case 6:
		value1->u.intValue += value2->u.intValue;
		break;
	default:
		Scr_UnmatchingTypesError(value1, value2);
		break;
	}
}
void  Scr_EvalMinus(VariableValue* value1, VariableValue* value2)
{
	int type; // [esp+0h] [ebp-8h]
	float* tempVector; // [esp+4h] [ebp-4h]

	Scr_CastWeakerPair(value1, value2);
	iassert(value1->type == value2->type);

	type = value1->type;

	switch (type)
	{
	case 4:
		tempVector = Scr_AllocVector();
		*tempVector = *(float*)value1->u.intValue - *(float*)value2->u.intValue;
		tempVector[1] = *(float*)(value1->u.intValue + 4) - *(float*)(value2->u.intValue + 4);
		tempVector[2] = *(float*)(value1->u.intValue + 8) - *(float*)(value2->u.intValue + 8);
		RemoveRefToVector(value1->u.vectorValue);
		RemoveRefToVector(value2->u.vectorValue);
		value1->u.intValue = (int)tempVector;
		break;
	case 5:
		value1->u.floatValue = value1->u.floatValue - value2->u.floatValue;
		break;
	case 6:
		value1->u.intValue -= value2->u.intValue;
		break;
	default:
		Scr_UnmatchingTypesError(value1, value2);
		break;
	}
}
void  Scr_EvalMultiply(VariableValue* value1, VariableValue* value2)
{
	int type; // [esp+0h] [ebp-8h]
	float* tempVector; // [esp+4h] [ebp-4h]

	Scr_CastWeakerPair(value1, value2);
	iassert(value1->type == value2->type);
	type = value1->type;
	switch (type)
	{
	case 4:
		tempVector = Scr_AllocVector();
		*tempVector = *(float*)value1->u.intValue * *(float*)value2->u.intValue;
		tempVector[1] = *(float*)(value1->u.intValue + 4) * *(float*)(value2->u.intValue + 4);
		tempVector[2] = *(float*)(value1->u.intValue + 8) * *(float*)(value2->u.intValue + 8);
		RemoveRefToVector(value1->u.vectorValue);
		RemoveRefToVector(value2->u.vectorValue);
		value1->u.intValue = (int)tempVector;
		break;
	case 5:
		value1->u.floatValue = value1->u.floatValue * value2->u.floatValue;
		break;
	case 6:
		value1->u.intValue *= value2->u.intValue;
		break;
	default:
		Scr_UnmatchingTypesError(value1, value2);
		break;
	}
}
void  Scr_EvalDivide(VariableValue* value1, VariableValue* value2)
{
	int type; // [esp+0h] [ebp-8h]
	float* tempVector; // [esp+4h] [ebp-4h]

	Scr_CastWeakerPair(value1, value2);
	
	iassert(value1->type == value2->type);

	type = value1->type;
	switch (type)
	{
	case 4:
		tempVector = Scr_AllocVector();
		if (*(float*)value2->u.intValue == 0.0
			|| *(float*)(value2->u.intValue + 4) == 0.0
			|| *(float*)(value2->u.intValue + 8) == 0.0)
		{
			*tempVector = 0.0;
			tempVector[1] = 0.0;
			tempVector[2] = 0.0;
			RemoveRefToVector(value1->u.vectorValue);
			RemoveRefToVector(value2->u.vectorValue);
			value1->u.intValue = (int)tempVector;
			Scr_Error("divide by 0");
		}
		else
		{
			*tempVector = *(float*)value1->u.intValue / *(float*)value2->u.intValue;
			tempVector[1] = *(float*)(value1->u.intValue + 4) / *(float*)(value2->u.intValue + 4);
			tempVector[2] = *(float*)(value1->u.intValue + 8) / *(float*)(value2->u.intValue + 8);
			RemoveRefToVector(value1->u.vectorValue);
			RemoveRefToVector(value2->u.vectorValue);
			value1->u.intValue = (int)tempVector;
		}
		break;
	case 5:
		if (value2->u.floatValue == 0.0)
		{
		LABEL_8:
			value1->u.floatValue = 0.0;
			Scr_Error("divide by 0");
			return;
		}
		value1->u.floatValue = value1->u.floatValue / value2->u.floatValue;
		break;
	case 6:
		value1->type = VAR_FLOAT;
		if (value2->u.intValue)
		{
			value1->u.floatValue = (double)value1->u.intValue / (double)value2->u.intValue;
			return;
		}
		goto LABEL_8;
	default:
		Scr_UnmatchingTypesError(value1, value2);
		break;
	}
}
void  Scr_EvalMod(VariableValue* value1, VariableValue* value2)
{
	if (value1->type == 6 && value2->type == 6)
	{
		if (value2->u.intValue)
		{
			value1->u.intValue %= value2->u.intValue;
		}
		else
		{
			value1->u.intValue = 0;
			//Scr_Error("divide by 0");
			iassert(0); // KISAK
		}
	}
	else
	{
		Scr_UnmatchingTypesError(value1, value2);
	}
}

void  Scr_FreeEntityNum(unsigned int entnum, unsigned int classnum)
{
	unsigned int entArrayId; // [esp+0h] [ebp-10h]
	unsigned int entnumId; // [esp+4h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+8h] [ebp-8h]
	unsigned int entId; // [esp+Ch] [ebp-4h]

	if (scrVarPub.bInited)
	{
		entArrayId = g_classMap[classnum].entArrayId;
		iassert(entArrayId);
		entnumId = FindArrayVariable(entArrayId, entnum);
		if (entnumId)
		{
			entId = FindObject(entnumId);
			iassert(entId);
			entryValue = &scrVarGlob.variableList[entId + 1];
			iassert((entryValue->w.type & VAR_MASK) == VAR_ENTITY);
			iassert(entryValue->u.o.u.entnum == entnum);
			iassert((entryValue->w.classnum >> VAR_NAME_BITS) == classnum);
			entryValue->w.status &= 0xFFFFFFE0;
			entryValue->w.status |= 0x13u;
			AddRefToObject(entId);
			entryValue->u.o.u.size = scrVarPub.freeEntList;
			if (scrVarDebugPub)
				--scrVarDebugPub->extRefCount[scrVarPub.freeEntList];
			scrVarPub.freeEntList = entId;
			if (scrVarDebugPub)
				++scrVarDebugPub->extRefCount[scrVarPub.freeEntList];
			RemoveArrayVariable(entArrayId, entnum);
		}
	}
}

void Scr_FreeObjects()
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	for (id = 1; id < 0x8000; ++id)
	{
		entryValue = &scrVarGlob.variableList[id + 1];
		if ((entryValue->w.status & 0x60) != 0
			&& ((entryValue->w.status & 0x1F) == 0x12 || (entryValue->w.status & 0x1F) == 0x13))
		{
			Scr_CancelNotifyList(id);
			ClearObject(id);
		}
	}
}

void  Scr_AddClassField(unsigned int classnum, char* name, unsigned int offset)
{
	unsigned int str; // [esp+0h] [ebp-14h]
	unsigned int stra; // [esp+0h] [ebp-14h]
	unsigned int classId; // [esp+4h] [ebp-10h]
	VariableValueInternal* entryValue; // [esp+8h] [ebp-Ch]
	VariableValueInternal* entryValuea; // [esp+8h] [ebp-Ch]
	unsigned int fieldId; // [esp+Ch] [ebp-8h]
	const char* namePos; // [esp+10h] [ebp-4h]

	iassert(offset < (1 << 16));

	for (namePos = name; *namePos; ++namePos)
	{
		iassert((*namePos < 'A' || *namePos > 'Z'));
	}
	classId = g_classMap[classnum].id;
	str = SL_GetCanonicalString(name);

	iassert(!FindArrayVariable(classId, (unsigned)str));

	entryValue = &scrVarGlob.variableList[GetNewArrayVariable(classId, str) + VARIABLELIST_CHILD_BEGIN];
	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status |= 6u;
	entryValue->u.u.intValue = offset;
	stra = SL_GetString_(name, 0, 16);
	iassert(!FindVariable(classId, str));
	fieldId = GetNewVariable(classId, stra);
	SL_RemoveRefToString(stra);
	entryValuea = &scrVarGlob.variableList[fieldId + VARIABLELIST_CHILD_BEGIN];
	entryValuea->w.status &= 0xFFFFFFE0;
	entryValuea->w.status |= 6u;
	entryValuea->u.u.intValue = offset;
}

unsigned int  Scr_GetEntityId(unsigned int entnum, unsigned int classnum)
{
	unsigned int entArrayId; // [esp+0h] [ebp-10h]
	VariableValueInternal* entryValue; // [esp+4h] [ebp-Ch]
	unsigned int entId; // [esp+8h] [ebp-8h]
	unsigned int id; // [esp+Ch] [ebp-4h]

	iassert((unsigned)entnum < (1 << 16));
	entArrayId = g_classMap[classnum].entArrayId;
	iassert(entArrayId);
	id = GetArrayVariable(entArrayId, entnum);
	iassert(id);
	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	if ((entryValue->w.status & 0x1F) != 0)
	{
		iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);
		return entryValue->u.u.stringValue;
	}
	else
	{
		entId = AllocEntity(classnum, entnum);
		iassert(!(entryValue->w.type & VAR_MASK));
		entryValue->w.status |= 1u;
		entryValue->u.u.intValue = entId;
		return entId;
	}
}

void  Scr_FreeGameVariable(int bComplete)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(scrVarPub.gameId);

	if (bComplete)
	{
		FreeValue(scrVarPub.gameId);
		scrVarPub.gameId = 0;
	}
	else
	{
		entryValue = &scrVarGlob.variableList[scrVarPub.gameId + VARIABLELIST_CHILD_BEGIN];
		iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);
		Scr_MakeValuePrimitive(entryValue->u.u.stringValue);
	}
}

void Scr_DumpScriptThreads(void)
{
	double ThreadUsage; // st7
	double ObjectUsage; // st7
	unsigned int NumScriptVars; // eax
	unsigned int NumScriptThreads; // eax
	int j; // [esp+0h] [ebp-DCh]
	int ja; // [esp+0h] [ebp-DCh]
	unsigned int classnum; // [esp+4h] [ebp-D8h]
	const char* pos; // [esp+8h] [ebp-D4h]
	ThreadDebugInfo info; // [esp+Ch] [ebp-D0h]
	const char* buf; // [esp+A0h] [ebp-3Ch]
	int size; // [esp+A4h] [ebp-38h]
	VariableValueInternal* entryValue; // [esp+A8h] [ebp-34h]
	ThreadDebugInfo* pInfo; // [esp+ACh] [ebp-30h]
	int num; // [esp+B0h] [ebp-2Ch]
	unsigned __int8 type; // [esp+B7h] [ebp-25h]
	VariableUnion u; // [esp+B8h] [ebp-24h]
	int i; // [esp+BCh] [ebp-20h]
	const VariableStackBuffer* stackBuf; // [esp+C0h] [ebp-1Ch]
	unsigned int entId; // [esp+C4h] [ebp-18h]
	ThreadDebugInfo* infoArray; // [esp+C8h] [ebp-14h]
	int count; // [esp+CCh] [ebp-10h]
	float endonUsage; // [esp+D0h] [ebp-Ch]
	unsigned int id; // [esp+D4h] [ebp-8h]
	float varUsage; // [esp+D8h] [ebp-4h]

	num = 0;
	for (id = 1; id < 0xFFFE; ++id)
	{
		entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
		if ((entryValue->w.status & 0x60) != 0 && (entryValue->w.status & 0x1F) == 0xA)
			++num;
	}
	if (num)
	{
		infoArray = (ThreadDebugInfo*)Z_TryVirtualAlloc(140 * num, "Scr_DumpScriptThreads", 0);
		if (infoArray)
		{
			num = 0;
			for (id = 1; id < 0xFFFE; ++id)
			{
				entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
				if ((entryValue->w.status & 0x60) != 0 && (entryValue->w.status & 0x1F) == 0xA)
				{
					pInfo = &infoArray[num++];
					info.posSize = 0;
					stackBuf = entryValue->u.u.stackValue;
					size = stackBuf->size;
					pos = stackBuf->pos;
					buf = stackBuf->buf;
					while (size)
					{
						--size;
						type = *buf++;
						u.intValue = *(int*)buf;
						buf += 4;
						if (type == 7)
							info.pos[info.posSize++] = u.codePosValue;
					}
					info.pos[info.posSize++] = pos;
					ThreadUsage = Scr_GetThreadUsage(stackBuf, &pInfo->endonUsage);
					pInfo->varUsage = ThreadUsage;
					pInfo->posSize = info.posSize--;
					for (j = 0; j < pInfo->posSize; ++j)
						pInfo->pos[j] = info.pos[info.posSize - j];
				}
			}
			qsort(infoArray, num, 0x8Cu, (int(*)(const void*, const void*))ThreadInfoCompare);
			Com_Printf(23, "********************************\n");
			varUsage = 0.0;
			endonUsage = 0.0;
			i = 0;
			while (i < num)
			{
				pInfo = &infoArray[i];
				count = 0;
				info.varUsage = 0.0;
				info.endonUsage = 0.0;
				do
				{
					++count;
					info.varUsage = info.varUsage + infoArray[i].varUsage;
					info.endonUsage = info.endonUsage + infoArray[i++].endonUsage;
				} while (i < num && !ThreadInfoCompare((uint32*)pInfo, (uint32*)&infoArray[i]));
				varUsage = varUsage + info.varUsage;
				endonUsage = endonUsage + info.endonUsage;
				Com_Printf(23, "count: %d, var usage: %d, endon usage: %d\n", count, (int)info.varUsage, (int)info.endonUsage);
				Scr_PrintPrevCodePos(23, (char*)pInfo->pos[0], 0);
				for (ja = 1; ja < pInfo->posSize; ++ja)
				{
					Com_Printf(23, "called from:\n");
					Scr_PrintPrevCodePos(23, (char*)pInfo->pos[ja], 0);
				}
			}
			Z_VirtualFree(infoArray);
			Com_Printf(23, "********************************\n");
			Com_Printf(23, "var usage: %d, endon usage: %d\n", (int)varUsage, (int)endonUsage);
			Com_Printf(23, "\n");
			for (classnum = 0; classnum < 4; ++classnum)
			{
				if (g_classMap[classnum].entArrayId)
				{
					info.varUsage = 0.0;
					count = 0;
					for (entId = FindFirstSibling(g_classMap[classnum].entArrayId); entId; entId = FindNextSibling(entId))
					{
						++count;
						if ((scrVarGlob.variableList[entId + VARIABLELIST_CHILD_BEGIN].w.status & 0x1F) == 1)
						{
							ObjectUsage = Scr_GetObjectUsage(scrVarGlob.variableList[entId + VARIABLELIST_CHILD_BEGIN].u.u.stringValue);
							info.varUsage = ObjectUsage + info.varUsage;
						}
					}
					Com_Printf(
						23,
						"ent type '%s'... count: %d, var usage: %d\n",
						g_classMap[classnum].name,
						count,
						(int)info.varUsage);
				}
			}
			Com_Printf(23, "********************************\n");
			NumScriptVars = Scr_GetNumScriptVars();
			Com_Printf(23, "num vars:    %d\n", NumScriptVars);
			NumScriptThreads = Scr_GetNumScriptThreads();
			Com_Printf(23, "num threads: %d\n", NumScriptThreads);
			Com_Printf(23, "********************************\n");
		}
		else
		{
			Com_Printf(23, "Cannot dump script threads: out of memory\n");
		}
	}
}

void Scr_ShutdownVariables()
{
	if (scrVarPub.gameId)
	{
		FreeValue(scrVarPub.gameId);
		scrVarPub.gameId = 0;
	}
	if (!scrStringDebugGlob || !scrStringDebugGlob->ignoreLeaks)
	{
		iassert((!scrVarPub.numScriptValues));
		iassert((!scrVarPub.numScriptObjects));
	}
	Scr_CheckLeaks();
}

void RemoveRefToObject(unsigned int id)
{
	unsigned int classnum; // [esp+0h] [ebp-Ch]
	unsigned __int16 entArrayId; // [esp+4h] [ebp-8h]
	VariableValueInternal* entryValue; // [esp+8h] [ebp-4h]

	iassert(id >= 1 && id < VARIABLELIST_PARENT_SIZE);
	entryValue = &scrVarGlob.variableList[id + 1];
	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert(IsObject(entryValue));
	if (entryValue->u.next)
	{
		--scrVarPub.totalObjectRefCount;
		if (scrVarDebugPub)
		{
			iassert(scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]);
			--scrVarDebugPub->leakCount[id + 1];
		}
		if (!--entryValue->u.next && (entryValue->w.status & 0x1F) == 0x14 && !entryValue->nextSibling)
		{
			entryValue->w.status &= 0xFFFFFFE0;
			entryValue->w.status |= 0x13u;
			classnum = entryValue->w.status >> 8;
			iassert(classnum < CLASS_NUM_COUNT);
			entArrayId = g_classMap[classnum].entArrayId;
			iassert(entArrayId);
			RemoveArrayVariable(entArrayId, entryValue->u.o.u.size);
		}
	}
	else
	{
		if (entryValue->nextSibling)
			ClearObject(id);
		FreeVariable(id);
	}
}

void  ClearVariableField(unsigned int parentId, unsigned int name, VariableValue* value)
{
	unsigned int classnum; // [esp+0h] [ebp-10h]
	VariableValueInternal* parentValue; // [esp+4h] [ebp-Ch]
	unsigned int fieldId; // [esp+Ch] [ebp-4h]
	VariableValue* valuea; // [esp+20h] [ebp+10h]

	iassert(IsObject(&scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId]));
	iassert(((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) >= FIRST_OBJECT && (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) < FIRST_NONFIELD_OBJECT) || ((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) >= FIRST_DEAD_OBJECT));

	if (FindVariableIndexInternal(parentId, name))
	{
		RemoveVariable(parentId, name);
	}
	else
	{
		parentValue = &scrVarGlob.variableList[parentId + 1];
		if ((parentValue->w.status & 0x1F) == 0x14)
		{
			iassert((parentValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT);
			classnum = parentValue->w.status >> 8;
			fieldId = FindArrayVariable(g_classMap[classnum].id, name);
			if (fieldId)
			{
				valuea = value + 1;
				valuea->type = VAR_UNDEFINED;
				SetEntityFieldValue(
					classnum,
					parentValue->u.o.u.size,
					scrVarGlob.variableList[fieldId + VARIABLELIST_CHILD_BEGIN].u.u.intValue,
					valuea);
			}
		}
	}
}

VariableValue Scr_EvalVariableField(unsigned int id)
{
	if (id)
		return Scr_EvalVariable(id);
	else
		return Scr_EvalVariableEntityField(scrVarPub.entId, scrVarPub.entFieldName);
}

void  Scr_EvalSizeValue(VariableValue* value)
{
	VariableUnion v1; // [esp+10h] [ebp-14h]
	VariableUnion stringValue; // [esp+14h] [ebp-10h]
	VariableValueInternal* entryValue; // [esp+18h] [ebp-Ch]
	char* error_message; // [esp+1Ch] [ebp-8h]
	VariableUnion id; // [esp+20h] [ebp-4h]

	if (value->type == VAR_POINTER)
	{
		id = value->u;
		entryValue = &scrVarGlob.variableList[value->u.intValue + 1];
		value->type = VAR_INTEGER;

		if ((entryValue->w.status & 0x1F) == 0x15)
			v1.intValue = entryValue->u.o.u.size;
		else
			v1.intValue = 1;

		value->u = v1;
		RemoveRefToObject(id.stringValue);
	}
	else if (value->type == VAR_STRING)
	{
		value->type = VAR_INTEGER;
		stringValue.intValue = value->u.intValue;
		value->u.intValue = strlen(SL_ConvertToString(value->u.intValue));
		SL_RemoveRefToString(stringValue.stringValue);
	}
	else
	{
		iassert(value->type != VAR_STACK);
		error_message = va("size cannot be applied to %s", var_typename[value->type]);
		RemoveRefToValue(value->type, value->u);
		value->type = VAR_UNDEFINED;
		Scr_Error(error_message);
	}
}

void  Scr_EvalBoolNot(VariableValue* value)
{
	Scr_CastBool(value);
	if (value->type == 6)
		value->u.intValue = value->u.intValue == 0;
}

void  Scr_EvalEquality(VariableValue* value1, VariableValue* value2)
{
	BOOL v2; // [esp+0h] [ebp-18h]
	float v3; // [esp+8h] [ebp-10h]
	float v4; // [esp+10h] [ebp-8h]
	BOOL tempInt; // [esp+14h] [ebp-4h]
	BOOL tempInta; // [esp+14h] [ebp-4h]

	Scr_CastWeakerPair(value1, value2);
	iassert(value1->type == value2->type);
	switch (value1->type)
	{
	case 0:
		value1->type = VAR_INTEGER;
		value1->u.intValue = 1;
		break;
	case 1:
		if (((scrVarGlob.variableList[value1->u.intValue + VARIABLELIST_CHILD_BEGIN].w.status & 0x1F) == 0x15
			|| (scrVarGlob.variableList[value2->u.intValue + VARIABLELIST_CHILD_BEGIN].w.status & 0x1F) == 0x15)
			&& !scrVarPub.evaluate)
		{
			goto LABEL_20;
		}
		value1->type = VAR_INTEGER;
		tempInta = value1->u.intValue == value2->u.intValue;
		RemoveRefToObject(value1->u.intValue);
		RemoveRefToObject(value2->u.intValue);
		value1->u.intValue = tempInta;
		break;
	case 2:
	case 3:
		value1->type = VAR_INTEGER;
		tempInt = value1->u.intValue == value2->u.intValue;
		SL_RemoveRefToString(value1->u.intValue);
		SL_RemoveRefToString(value2->u.intValue);
		value1->u.intValue = tempInt;
		break;
	case 4:
		value1->type = VAR_INTEGER;
		v2 = *(float*)value2->u.intValue == *(float*)value1->u.intValue
			&& *(float*)(value2->u.intValue + 4) == *(float*)(value1->u.intValue + 4)
			&& *(float*)(value2->u.intValue + 8) == *(float*)(value1->u.intValue + 8);
		RemoveRefToVector(value1->u.vectorValue);
		RemoveRefToVector(value2->u.vectorValue);
		value1->u.intValue = v2;
		break;
	case 5:
		value1->type = VAR_INTEGER;
		v4 = value1->u.floatValue - value2->u.floatValue;
		v3 = fabs(v4);
		value1->u.intValue = v3 < 0.0000009999999974752427;
		break;
	case 6:
		value1->u.intValue = value1->u.intValue == value2->u.intValue;
		break;
	case 9:
		value1->type = VAR_INTEGER;
		value1->u.intValue = value1->u.intValue == value2->u.intValue;
		break;
	case 0xB:
		value1->type = VAR_INTEGER;
		value1->u.intValue = value1->u.intValue == value2->u.intValue;
		break;
	default:
	LABEL_20:
		Scr_UnmatchingTypesError(value1, value2);
		break;
	}
}

void  Scr_EvalInequality(VariableValue* value1, VariableValue* value2)
{
	Scr_EvalEquality(value1, value2);
	iassert((value1->type == VAR_INTEGER) || (value1->type == VAR_UNDEFINED));
	value1->u.intValue = value1->u.intValue == 0;
}

void __cdecl Scr_EvalBinaryOperator(int op, VariableValue *value1, VariableValue *value2)
{
	switch (op)
	{
	case OP_bit_or:
		Scr_EvalOr(value1, value2);
		break;
	case OP_bit_ex_or:
		Scr_EvalExOr(value1, value2);
		break;
	case OP_bit_and:
		Scr_EvalAnd(value1, value2);
		break;
	case OP_equality:
		Scr_EvalEquality(value1, value2);
		break;
	case OP_inequality:
		Scr_EvalInequality(value1, value2);
		break;
	case OP_less:
		Scr_EvalLess(value1, value2);
		break;
	case OP_greater:
		Scr_EvalGreater(value1, value2);
		break;
	case OP_less_equal:
		Scr_EvalLessEqual(value1, value2);
		break;
	case OP_greater_equal:
		Scr_EvalGreaterEqual(value1, value2);
		break;
	case OP_shift_left:
		Scr_EvalShiftLeft(value1, value2);
		break;
	case OP_shift_right:
		Scr_EvalShiftRight(value1, value2);
		break;
	case OP_plus:
		Scr_EvalPlus(value1, value2);
		break;
	case OP_minus:
		Scr_EvalMinus(value1, value2);
		break;
	case OP_multiply:
		Scr_EvalMultiply(value1, value2);
		break;
	case OP_divide:
		Scr_EvalDivide(value1, value2);
		break;
	case OP_mod:
		Scr_EvalMod(value1, value2);
		break;
	default:
		return;
	}
}

void  Scr_FreeEntityList(void)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int entId; // [esp+4h] [ebp-4h]

	if (scrVarDebugPub)
		--scrVarDebugPub->extRefCount[scrVarPub.freeEntList];
	while (scrVarPub.freeEntList)
	{
		entId = scrVarPub.freeEntList;
		entryValue = &scrVarGlob.variableList[scrVarPub.freeEntList + 1];
		scrVarPub.freeEntList = entryValue->u.o.u.size;
		entryValue->u.o.u.size = 0;
		Scr_CancelNotifyList(entId);

		if (entryValue->nextSibling)
			ClearObjectInternal(entId);

		RemoveRefToObject(entId);
	}
}

void  Scr_RemoveClassMap(unsigned int classnum)
{
	if (scrVarPub.bInited)
	{
		if (g_classMap[classnum].entArrayId)
		{
			if (scrVarDebugPub)
				--scrVarDebugPub->extRefCount[g_classMap[classnum].entArrayId];
			RemoveRefToObject(g_classMap[classnum].entArrayId);
			g_classMap[classnum].entArrayId = 0;
		}
		if (g_classMap[classnum].id)
		{
			if (scrVarDebugPub)
				--scrVarDebugPub->extRefCount[g_classMap[classnum].id];
			RemoveRefToObject(g_classMap[classnum].id);
			g_classMap[classnum].id = 0;
		}
	}
}

void  Scr_EvalArray(VariableValue* value, VariableValue* index)
{
	char c[4]{ 0 }; // [esp+1Ch] [ebp-Ch] BYREF
	const char* s; // [esp+20h] [ebp-8h]
	VariableValueInternal* entryValue; // [esp+24h] [ebp-4h]

	iassert(value != index);
	switch (value->type)
	{
	case VAR_POINTER:
		entryValue = &scrVarGlob.variableList[value->u.pointerValue + VARIABLELIST_PARENT_BEGIN];

		iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
		iassert(IsObject(entryValue));

		if ((entryValue->w.type & VAR_MASK) == VAR_ARRAY)
		{
			//*index = Scr_EvalVariable(Scr_FindArrayIndex(value->u.intValue, index));
			VariableValue tmp = Scr_EvalVariable(Scr_FindArrayIndex(value->u.intValue, index));
			index->type = tmp.type;
			index->u = tmp.u;
			RemoveRefToObject(value->u.pointerValue);
		}
		else
		{
			scrVarPub.error_index = 1;
			Scr_Error(va("%s is not an array", var_typename[entryValue->w.type & VAR_MASK]));
		}
		break;
	case VAR_STRING:
		if (index->type == VAR_INTEGER)
		{
			if (index->u.intValue < 0)
			{
				Scr_Error(va("string index %d out of range", index->u.intValue));
				return;
			}
			s = SL_ConvertToString(value->u.stringValue);
			
			if (index->u.intValue >= strlen(s))
			{
				Scr_Error(va("string index %d out of range", index->u.intValue));
				return;
			}

			index->type = VAR_STRING;

			c[0] = s[index->u.intValue];
			c[1] = 0;

			index->u.stringValue = SL_GetStringOfSize(c, 0, 2, 15).prev;
			SL_RemoveRefToString(value->u.stringValue);
		}
		else
		{
			Scr_Error(va("%s is not a string index", var_typename[index->type]));
		}
		break;
	case VAR_VECTOR:
		if (index->type == VAR_INTEGER)
		{
			if (index->u.intValue >= 3)
			{
				Scr_Error(va("vector index %d out of range", index->u.intValue));
			}
			else
			{
				index->type = VAR_FLOAT;
				//index->u.floatValue = *(float*)(value->u.intValue + 4 * index->u.intValue);
				index->u.floatValue = value->u.vectorValue[index->u.intValue];
				RemoveRefToVector(value->u.vectorValue);
			}
		}
		else
		{
			Scr_Error(va("%s is not a vector index", var_typename[index->type]));
		}
		break;
	default:
		iassert(value->type != VAR_STACK);
		scrVarPub.error_index = 1;
		Scr_Error(va("%s is not an array, string, or vector", var_typename[value->type]));
		break;
	}
}

unsigned int Scr_EvalArrayRef(unsigned int parentId)
{
	VariableValueInternal* parentValue; // [esp+Ch] [ebp-1Ch]
	VariableValueInternal* entValue; // [esp+10h] [ebp-18h]
	VariableValue varValue; // [esp+14h] [ebp-14h]
	VariableValueInternal* entryValue; // [esp+1Ch] [ebp-Ch]
	unsigned int fieldId; // [esp+20h] [ebp-8h]
	int id; // [esp+24h] [ebp-4h]

	if (parentId)
	{
		parentValue = &scrVarGlob.variableList[parentId + VARIABLELIST_CHILD_BEGIN];
		iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
		varValue.type = (Vartype_t)(parentValue->w.type & VAR_MASK);
		if (varValue.type)
		{
			varValue.u.intValue = parentValue->u.u.intValue;
		add_array:
			if (varValue.type == VAR_POINTER)
			{
				entryValue = &scrVarGlob.variableList[varValue.u.intValue + 1];
				iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
				iassert(IsObject(entryValue));
				if ((entryValue->w.status & VAR_MASK) == VAR_ARRAY)
				{
					if (entryValue->u.next)
					{
						id = varValue.u.intValue;
						RemoveRefToObject(varValue.u.stringValue);
						varValue.u.pointerValue = Scr_AllocArray();
						CopyArray(id, varValue.u.pointerValue);
						iassert(parentValue);
						parentValue->u.u.intValue = varValue.u.intValue;
					}
					return varValue.u.pointerValue;
				}
				else
				{
					scrVarPub.error_index = 1;
					Scr_Error(va("%s is not an array", var_typename[entryValue->w.status & VAR_MASK]));
					return 0;
				}
			}
			else
			{
				iassert(varValue.type != VAR_STACK);
				scrVarPub.error_index = 1;
				if (varValue.type == VAR_STRING)
				{
					Scr_Error("string characters cannot be individually changed");
					return 0;
				}
				else
				{
					if (varValue.type == VAR_VECTOR)
					{
						Scr_Error("vector components cannot be individually changed");
					}
					else
					{
						Scr_Error(va("%s is not an array", var_typename[varValue.type]));
						iassert(0);
					}
					return 0;
				}
			}
		}
	}
	else
	{
		entValue = &scrVarGlob.variableList[scrVarPub.entId + 1];
		iassert((entValue->w.type & VAR_MASK) == VAR_ENTITY);
		iassert((entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT);
		fieldId = FindArrayVariable(g_classMap[entValue->w.status >> 8].id, scrVarPub.entFieldName);
		if (fieldId)
		{
			varValue = GetEntityFieldValue(entValue->w.classnum >> VAR_NAME_BITS, entValue->u.o.u.entnum, scrVarGlob.variableList[fieldId + VARIABLELIST_CHILD_BEGIN].u.u.entityOffset);
			if (varValue.type)
			{
				if (varValue.type == VAR_POINTER && !scrVarGlob.variableList[varValue.u.intValue + 1].u.next)
				{
					RemoveRefToValue(1, varValue.u);
					scrVarPub.error_index = 1;
					Scr_Error("read-only array cannot be changed");
					return 0;
				}
				RemoveRefToValue(varValue.type, varValue.u);
				iassert((varValue.type != VAR_POINTER) || !scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + varValue.u.pointerValue].u.o.refCount);
				parentValue = 0;
				goto add_array;
			}
		}
		parentValue = &scrVarGlob.variableList[GetNewVariable(scrVarPub.entId, scrVarPub.entFieldName) + VARIABLELIST_CHILD_BEGIN];
	}

	iassert(!(parentValue->w.type & VAR_MASK));

	parentValue->w.type |= VAR_POINTER;
	parentValue->u.u.pointerValue = Scr_AllocArray();
	return parentValue->u.u.pointerValue;
}

void  ClearArray(unsigned int parentId, VariableValue* value)
{
	VariableValueInternal* parentValue; // [esp+8h] [ebp-1Ch]
	VariableValueInternal* entValue; // [esp+Ch] [ebp-18h]
	VariableValue varValue; // [esp+10h] [ebp-14h]
	VariableValueInternal* entryValue; // [esp+18h] [ebp-Ch]
	unsigned int fieldId; // [esp+1Ch] [ebp-8h]
	VariableUnion id; // [esp+20h] [ebp-4h]

	if (parentId)
	{
		parentValue = &scrVarGlob.variableList[parentId + VARIABLELIST_CHILD_BEGIN];
		iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
		varValue.type = (Vartype_t)(parentValue->w.status & 0x1F);
		varValue.u.intValue = parentValue->u.u.intValue;
	}
	else
	{
		entValue = &scrVarGlob.variableList[scrVarPub.entId + 1];
		iassert((entValue->w.type & VAR_MASK) == VAR_ENTITY);
		iassert((entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT);
		fieldId = FindArrayVariable(g_classMap[entValue->w.status >> 8].id, scrVarPub.entFieldName);
		if (!fieldId
			|| (varValue = GetEntityFieldValue(
				entValue->w.status >> 8,
				entValue->u.o.u.size,
				scrVarGlob.variableList[fieldId + VARIABLELIST_CHILD_BEGIN].u.u.intValue),
				!varValue.type))
		{
			varValue.type = VAR_UNDEFINED;
		error_0:
			scrVarPub.error_index = 1;
			Scr_Error(va("%s is not an array", var_typename[varValue.type]));
			return;
		}
		if (varValue.type == 1 && !scrVarGlob.variableList[varValue.u.intValue + 1].u.next)
		{
			RemoveRefToValue(1, varValue.u);
			scrVarPub.error_index = 1;
			Scr_Error("read-only array cannot be changed");
			return;
		}
		RemoveRefToValue(varValue.type, varValue.u);
		iassert((varValue.type != VAR_POINTER) || !scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + varValue.u.pointerValue].u.o.refCount);
		parentValue = 0;
	}
	if (varValue.type != VAR_POINTER)
	{
		iassert(varValue.type != VAR_STACK);
		goto error_0;
	}
	entryValue = &scrVarGlob.variableList[varValue.u.intValue + 1];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(IsObject(entryValue));
	if ((entryValue->w.status & VAR_MASK) == VAR_ARRAY)
	{
		if (entryValue->u.next)
		{
			id.intValue = varValue.u.intValue;
			RemoveRefToObject(varValue.u.stringValue);
			varValue.u.intValue = Scr_AllocArray();
			CopyArray(id.stringValue, varValue.u.stringValue);
			iassert(parentValue);
			parentValue->u.u.intValue = varValue.u.intValue;
		}
		if (value->type == VAR_INTEGER)
		{
			if (IsValidArrayIndex(value->u.intValue))
			{
				SafeRemoveArrayVariable(varValue.u.stringValue, value->u.intValue);
			}
			else
			{
				Scr_Error(va("array index %d out of range", value->u.intValue));
			}
		}
		else if (value->type == VAR_STRING)
		{
			SL_RemoveRefToString(value->u.intValue);
			SafeRemoveVariable(varValue.u.stringValue, value->u.intValue);
		}
		else
		{
			Scr_Error(va("%s is not an array index", var_typename[value->type]));
		}
	}
	else
	{
		scrVarPub.error_index = 1;
		Scr_Error(va("%s is not an array", var_typename[entryValue->w.status & VAR_MASK]));
	}
}

void  Scr_FreeValue(unsigned int id)
{
	iassert(id);
	RemoveRefToObject(id);
}

void  Scr_StopThread(unsigned int threadId)
{
	iassert(threadId);
	Scr_ClearThread(threadId);
	scrVarGlob.variableList[threadId + 1].u.o.u.size = scrVarPub.levelId;
	AddRefToObject(scrVarPub.levelId);
}

void  Scr_KillEndonThread(unsigned int threadId)
{
	VariableValueInternal* parentValue; // [esp+0h] [ebp-4h]

	parentValue = &scrVarGlob.variableList[threadId + 1];
	iassert((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((parentValue->w.type & VAR_MASK) == VAR_THREAD);
	iassert(!parentValue->nextSibling);
	RemoveRefToObject(parentValue->u.o.u.size);
	iassert(!FindObjectVariable(scrVarPub.pauseArrayId, threadId));
	parentValue->w.status &= 0xFFFFFFE0;
	parentValue->w.status |= 0x16u;
}

VariableValue Scr_FindVariableField(unsigned int parentId, unsigned int name)
{
	unsigned int id; // [esp+1Ch] [ebp-4h]

	iassert(parentId);
	iassert(IsObject(&scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId]));
	iassert(((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) >= FIRST_OBJECT 
		&& (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) < FIRST_NONFIELD_OBJECT) 
		|| ((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) >= FIRST_DEAD_OBJECT));

	id = FindVariable(parentId, name);

	if (id)
		return Scr_EvalVariable(id); // LWSS: this is NOT 'return Scr_EvalVariable(id).u.stringValue;'. The type is confirmed '2' for the 1st call

	if ((scrVarGlob.variableList[parentId + VARIABLELIST_PARENT_BEGIN].w.type & VAR_MASK) == VAR_ENTITY)
		return Scr_EvalVariableEntityField(parentId, name);

	VariableValue value;
	value.type = VAR_UNDEFINED;
	return value;
}

void  Scr_KillThread(unsigned int parentId)
{
	unsigned int ObjectVariable_DONE; // eax
	VariableValueInternal_u* VariableValueAddress_DONE; // eax
	VariableValueInternal* parentValue; // [esp+0h] [ebp-18h]
	unsigned int selfNameId; // [esp+4h] [ebp-14h]
	unsigned int name; // [esp+8h] [ebp-10h]
	unsigned int id; // [esp+10h] [ebp-8h]
	unsigned int notifyListEntry; // [esp+14h] [ebp-4h]

	iassert(parentId);
	parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert(((parentValue->w.type & VAR_MASK) >= VAR_THREAD) && ((parentValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD));
	Scr_ClearThread(parentId);
	id = FindObjectVariable(scrVarPub.pauseArrayId, parentId);
	if (id)
	{
		for (selfNameId = FindObject(id); ; RemoveObjectVariable(selfNameId, name))
		{
			notifyListEntry = FindFirstSibling(selfNameId);
			if (!notifyListEntry)
				break;

			iassert((scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + notifyListEntry].w.type & VAR_MASK) == VAR_POINTER);
			name = scrVarGlob.variableList[notifyListEntry + VARIABLELIST_CHILD_BEGIN].w.status >> 8;
			iassert((name - SL_MAX_STRING_INDEX) < (1 << 16));
			name -= SL_MAX_STRING_INDEX;

			ObjectVariable_DONE = FindObjectVariable(selfNameId, name);
			VariableValueAddress_DONE = GetVariableValueAddress(ObjectVariable_DONE);
			VM_CancelNotify(VariableValueAddress_DONE->u.intValue, name);
			Scr_KillEndonThread(name);
		}
		iassert(!GetArraySize(selfNameId));
		RemoveObjectVariable(scrVarPub.pauseArrayId, parentId);
	}
	parentValue->w.status &= 0xFFFFFFE0;
	parentValue->w.status |= 0x16u;
}

void  Scr_CheckLeakRange(unsigned int begin, unsigned int end)
{
	Variable* entry; // [esp+0h] [ebp-10h]
	int index; // [esp+8h] [ebp-8h]
	VariableValueInternal* value; // [esp+Ch] [ebp-4h]

	for (index = 1; index < (int)(end - begin); ++index)
	{
		entry = &scrVarGlob.variableList[index + begin].hash;
		value = &scrVarGlob.variableList[begin + entry->id];
		switch (value->w.status & 0x60)
		{
		case ' ':
			Com_Printf(
				23,
				"move: %d -> %d\n",
				begin + entry->id,
				begin + scrVarGlob.variableList[begin + value->v.next].hash.id);
		LABEL_11:
			Com_Printf(23, "%d -> %d\n", begin + entry->id, scrVarGlob.variableList[begin + value->nextSibling].hash.id);
			Com_Printf(23, "%d <- %d\n", begin + entry->id, scrVarGlob.variableList[begin + entry->u.prev].hash.id);
			continue;
		case '@':
			Com_Printf(
				23,
				"head: %d -> %d\n",
				begin + entry->id,
				begin + scrVarGlob.variableList[begin + value->v.next].hash.id);
			goto LABEL_11;
		case '`':
			Com_Printf(
				23,
				"ext: %d %d\n",
				begin + entry->id,
				begin + scrVarGlob.variableList[begin + value->v.next].hash.id);
			goto LABEL_11;
		}
	}
}

void  Scr_CheckLeaks(void)
{
	bool bLeak; // [esp+3h] [ebp-5h]
	unsigned int id; // [esp+4h] [ebp-4h]
	unsigned int ida; // [esp+4h] [ebp-4h]

	if (!scrStringDebugGlob || !scrStringDebugGlob->ignoreLeaks)
	{
		Scr_CheckLeakRange(1u, 0x8001u);
		Scr_CheckLeakRange(VARIABLELIST_CHILD_BEGIN, 0x18000u);
		iassert(!scrVarPub.totalObjectRefCount);
		iassert(!scrVarPub.totalVectorRefCount);
		iassert(!scrVarPub.ext_threadcount);
	}
	if (scrVarDebugPub)
	{
		bLeak = 0;
		for (id = 0; id < 0x18000; ++id)
		{
			if (scrVarDebugPub->leakCount[id])
				bLeak = 1;
		}
		if (bLeak)
		{
			Com_Printf(23, "leak:\n");
			for (ida = 0; ida < 0x18000; ++ida)
			{
				if (scrVarDebugPub->leakCount[ida])
					Com_Printf(23, "%d, %d\n", ida, scrVarDebugPub->leakCount[ida]);
			}
			Com_Printf(23, "\n");
			if ((!scrStringDebugGlob || !scrStringDebugGlob->ignoreLeaks) && !alwaysfails)
				MyAssertHandler(".\\script\\scr_variable.cpp", 182, 0, "leak");
		}
		scrVarDebugPub = 0;
	}
}

int  ThreadInfoCompare(_DWORD* info1, _DWORD* info2)
{
	const char* pos1; // [esp+0h] [ebp-Ch]
	int i; // [esp+4h] [ebp-8h]
	const char* pos2; // [esp+8h] [ebp-4h]

	for (i = 0; ; ++i)
	{
		if (i >= info1[32] || i >= info2[32])
			return info1[32] - info2[32];
		pos1 = (const char*)info1[i];
		pos2 = (const char*)info2[i];
		if (pos1 != pos2)
			break;
	}
	return pos1 - pos2;
}

int VariableInfoFileNameCompare(_DWORD* info1, _DWORD* info2)
{
	const char* fileName1; // [esp+0h] [ebp-8h]
	const char* fileName2; // [esp+4h] [ebp-4h]

	fileName1 = (const char*)info1[1];
	fileName2 = (const char*)info2[1];
	if (!fileName1)
		return 1;
	if (fileName2)
		return I_stricmp(fileName1, fileName2);
	return -1;
}

int VariableInfoCountCompare(_DWORD* info1, _DWORD* info2)
{
	return info1[3] - info2[3];
}

int __cdecl VariableInfoFileLineCompare(_DWORD* info1, _DWORD* info2)
{
	int fileCompare; // [esp+0h] [ebp-4h]

	fileCompare = VariableInfoFileNameCompare(info1, info2);
	if (fileCompare)
		return fileCompare;
	else
		return CompareThreadIndices((unsigned int*)info1, (unsigned int*)info2);
}

unsigned int  FindVariableIndexInternal2(unsigned int name, unsigned int index)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-14h]
	unsigned int newIndex; // [esp+4h] [ebp-10h]
	VariableValueInternal* entryValue; // [esp+8h] [ebp-Ch]
	VariableValueInternal* newEntryValue; // [esp+Ch] [ebp-8h]
	VariableValueInternal* newEntry; // [esp+10h] [ebp-4h]

	iassert(!(name & ~VAR_NAME_LOW_MASK));
	iassert(index < 0xFFFE);
	entry = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
	iassert(entry->hash.id < 0xFFFEu);
	entryValue = &scrVarGlob.variableList[entry->hash.id + VARIABLELIST_CHILD_BEGIN];
	if ((entryValue->w.status & 0x60) == 0x40)
	{
		iassert(!IsObject(entryValue));
		if (entryValue->w.status >> 8 == name)
			return index;
		newIndex = entryValue->v.next;
		for (newEntry = &scrVarGlob.variableList[newIndex + VARIABLELIST_CHILD_BEGIN];
			newEntry != entry;
			newEntry = &scrVarGlob.variableList[newIndex + VARIABLELIST_CHILD_BEGIN])
		{
			newEntryValue = &scrVarGlob.variableList[newEntry->hash.id + VARIABLELIST_CHILD_BEGIN];
			iassert((newEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE);
			iassert(!IsObject(newEntryValue));

			if (newEntryValue->w.status >> 8 == name)
				return newIndex;
			newIndex = newEntryValue->v.next;
		}
	}
	return 0;
}

unsigned int FindVariableIndexInternal(unsigned int parentId, unsigned int name)
{
	iassert(parentId);
	iassert((scrVarGlob.variableList[parentId + 1].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert(IsObject(&scrVarGlob.variableList[parentId + 1]));

	return FindVariableIndexInternal2(name, (parentId + 101 * name) % 0xFFFD + 1);
}

unsigned short  AllocVariable(void)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-14h]
	unsigned __int16 newIndex; // [esp+4h] [ebp-10h]
	unsigned __int16 next; // [esp+8h] [ebp-Ch]
	unsigned __int16 index; // [esp+Ch] [ebp-8h]
	VariableValueInternal* entryValue; // [esp+10h] [ebp-4h]

	index = scrVarGlob.variableList[1].u.next;

	if (!scrVarGlob.variableList[1].u.next)
		Scr_TerminalError("exceeded maximum number of script variables");

	entry = &scrVarGlob.variableList[index + 1];
	entryValue = &scrVarGlob.variableList[entry->hash.id + 1];
	iassert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE);
	next = entryValue->u.next;
	if (entry != entryValue && (entry->w.status & VAR_STAT_MASK) == VAR_STAT_FREE)
	{
		newIndex = entry->v.next;
		iassert(newIndex != index);
		scrVarGlob.variableList[newIndex + 1].hash.id = entry->hash.id;
		entry->hash.id = index;
		entryValue->v.next = newIndex;
		entryValue->u.next = entry->u.next;
		entryValue = &scrVarGlob.variableList[index + 1];
	}
	scrVarGlob.variableList[1].u.next = next;
	scrVarGlob.variableList[next + 1].hash.u.prev = 0;
	entryValue->v.next = index;
	entryValue->nextSibling = 0;
	entry->hash.u.prev = 0;
	iassert(entry->hash.id > 0 && entry->hash.id < VARIABLELIST_PARENT_SIZE);

	++scrVarPub.totalObjectRefCount;

	if (scrVarDebugPub)
	{
		iassert(!scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + entry->hash.id]);
		++scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + entry->hash.id];
	}

	++scrVarPub.numScriptObjects;

	iassert(scrVarPub.varUsagePos);

	if (scrVarDebugPub)
	{
		iassert(!scrVarDebugPub->varUsage[VARIABLELIST_PARENT_BEGIN + entry->hash.id]);
		scrVarDebugPub->varUsage[VARIABLELIST_PARENT_BEGIN + entry->hash.id] = scrVarPub.varUsagePos;
	}
	return entry->hash.id;
}

void  FreeVariable(unsigned int id)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+4h] [ebp-8h]
	unsigned int index; // [esp+8h] [ebp-4h]

	iassert(id > 0 && id < VARIABLELIST_PARENT_SIZE);

	--scrVarPub.totalObjectRefCount;

	if (scrVarDebugPub)
	{
		iassert(scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]);
		scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]--;
		iassert(!scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]);
	}

	--scrVarPub.numScriptObjects;

	if (scrVarDebugPub)
	{
		iassert(scrVarDebugPub->varUsage[VARIABLELIST_PARENT_BEGIN + id]);
		scrVarDebugPub->varUsage[id + 1] = 0;
	}

	entryValue = &scrVarGlob.variableList[id + 1];
	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	index = entryValue->v.next;
	entry = &scrVarGlob.variableList[index + 1];
	iassert(entry->hash.id == id);
	iassert(!entry->hash.u.prev);
	iassert(!entryValue->nextSibling);
	entryValue->w.status = 0;
	entryValue->u.next = scrVarGlob.variableList[1].u.next;
	entry->hash.u.prev = 0;
	scrVarGlob.variableList[scrVarGlob.variableList[1].u.next + 1].hash.u.prev = index;
	scrVarGlob.variableList[1].u.next = index;
}

unsigned int  AllocEntity(unsigned int classnum, unsigned short entnum)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id + 1];
	entryValue->w.status = VAR_STAT_EXTERNAL;
	iassert(!(entryValue->w.type & VAR_MASK));
	entryValue->w.status |= VAR_ENTITY;
	iassert(!(entryValue->w.classnum & VAR_NAME_HIGH_MASK));
	entryValue->w.classnum|= classnum << VAR_NAME_BITS;
	entryValue->u.next = 0;
	entryValue->u.o.u.entnum = entnum;
	return id;
}

float* Scr_AllocVector(void)
{
	RefVector* vec; // eax
	float* result; // [esp+4h] [ebp-4h]
	
	vec = (RefVector *)MT_Alloc(sizeof(RefVector), 2);
	result = &vec->vec[0];
	vec->head = 0;

	InterlockedIncrement(&scrVarPub.totalVectorRefCount);
	if (scrStringDebugGlob)
		InterlockedIncrement(&scrStringDebugGlob->refCount[((char*)(result - 1) - scrMemTreePub.mt_buffer) / 12]);

	return result;
}

unsigned int  FindArrayVariableIndex(unsigned int parentId, unsigned int unsignedValue)
{
	iassert(IsValidArrayIndex(unsignedValue));
	return FindVariableIndexInternal(parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

unsigned int  Scr_FindArrayIndex(unsigned int parentId, VariableValue* index)
{
	unsigned int id; // [esp+0h] [ebp-4h]

	if (index->type == VAR_INTEGER)
	{
		if (IsValidArrayIndex(index->u.intValue))
		{
			return FindArrayVariable(parentId, index->u.intValue);
		}
		else
		{
			Scr_Error(va("array index %d out of range", index->u.intValue));
			AddRefToObject(parentId);
			return 0;
		}
	}
	else if (index->type == VAR_STRING)
	{
		id = FindVariable(parentId, index->u.intValue);
		SL_RemoveRefToString(index->u.intValue);
		return id;
	}
	else
	{
		Scr_Error(va("%s is not an array index", var_typename[index->type]));
		AddRefToObject(parentId);
		return 0;
	}
}

float  Scr_GetEntryUsage(unsigned int type, VariableUnion u)
{
	VariableValueInternal* parentValue; // [esp+Ch] [ebp-4h]

	if (type != 1)
		return 0.0;

	parentValue = &scrVarGlob.variableList[u.intValue + 1];

	iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(IsObject(parentValue));

	if ((parentValue->w.status & VAR_MASK) != VAR_ARRAY)
		return 0.0;

	return (float)(Scr_GetObjectUsage(u.stringValue) / ((double)parentValue->u.next + 1.0));

}

float  Scr_GetEntryUsage(VariableValueInternal* entryValue)
{
	return (float)(Scr_GetEntryUsage(entryValue->w.status & VAR_MASK, entryValue->u.u) + 1.0);
}

float  Scr_GetObjectUsage(unsigned int parentId)
{
	float usage; // [esp+4h] [ebp-8h]
	unsigned int id; // [esp+8h] [ebp-4h]

	VariableValueInternal *parentValue = &scrVarGlob.variableList[parentId + 1];

	iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(IsObject(parentValue));

	usage = 1.0;

	for (id = FindFirstSibling(parentId); id; id = FindNextSibling(id))
		usage = Scr_GetEntryUsage(&scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN]) + usage;

	return usage;
}

char* Scr_GetSourceFile_FastFile(char const* filename)
{
	const char* v1; // eax
	RawFile* rawfile; // [esp+4h] [ebp-4h]

	rawfile = DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, filename).rawfile;
	if (!rawfile)
	{
		v1 = va("cannot find %s", filename);
		Com_Error(ERR_DROP, v1);
	}
	return (char*)rawfile->buffer;
}

char* Scr_GetSourceFile(char const* filename)
{
	const char* v1; // eax
	char* sourceBuffer; // [esp+0h] [ebp-Ch]
	int len; // [esp+4h] [ebp-8h]
	int f; // [esp+8h] [ebp-4h] BYREF

	len = FS_FOpenFileByMode((char*)filename, &f, FS_READ);
	if (len < 0)
	{
		v1 = va("cannot find %s", filename);
		Com_Error(ERR_DROP, v1);
	}
	sourceBuffer = (char*)Hunk_AllocateTempMemoryHigh(len + 1, "Scr_LoadAnimTreeInternal");
	FS_Read((unsigned char*)sourceBuffer, len, f);
	sourceBuffer[len] = 0;
	FS_FCloseFile(f);
	return sourceBuffer;
}

char *__cdecl Scr_GetSourceFile_LoadObj(const char *filename)
{
	const char *v1; // eax
	char *sourceBuffer; // [esp+0h] [ebp-Ch]
	int len; // [esp+4h] [ebp-8h]
	int f; // [esp+8h] [ebp-4h] BYREF

	len = FS_FOpenFileByMode((char*)filename, &f, FS_READ);
	if (len < 0)
	{
		v1 = va("cannot find %s", filename);
		Com_Error(ERR_DROP, v1);
	}
	sourceBuffer = (char*)Hunk_AllocateTempMemoryHigh(len + 1, "Scr_LoadAnimTreeInternal");
	FS_Read((unsigned char*)sourceBuffer, len, f);
	sourceBuffer[len] = 0;
	FS_FCloseFile(f);
	return sourceBuffer;
}

void  Scr_AddFieldsForFile(char const* filename)
{
	const char* SourceFile_FastFile_DONE; // eax
	const char* v2; // eax
	const char* v3; // eax
	char v4; // [esp+3h] [ebp-9Dh]
	char* v5; // [esp+8h] [ebp-98h]
	char* v6; // [esp+Ch] [ebp-94h]
	int v7; // [esp+10h] [ebp-90h]
	int tempType[2]; // [esp+78h] [ebp-28h] BYREF
	int len; // [esp+80h] [ebp-20h]
	int size; // [esp+84h] [ebp-1Ch]
	char* targetPos; // [esp+88h] [ebp-18h]
	unsigned int index; // [esp+8Ch] [ebp-14h]
	int type; // [esp+90h] [ebp-10h]
	const char* sourcePos; // [esp+94h] [ebp-Ch] BYREF
	char* token; // [esp+98h] [ebp-8h]
	int i; // [esp+9Ch] [ebp-4h]

	Hunk_CheckTempMemoryHighClear();
	if (useFastFile->current.enabled)
		SourceFile_FastFile_DONE = (const char*)Scr_GetSourceFile_FastFile(filename);
	else
		SourceFile_FastFile_DONE = Scr_GetSourceFile_LoadObj(filename);
	tempType[1] = (int)SourceFile_FastFile_DONE;
	sourcePos = SourceFile_FastFile_DONE;
	Com_BeginParseSession("Scr_AddFields");
	for (targetPos = TempMalloc(0); ; *targetPos = 0)
	{
		token = (char*)Com_Parse(&sourcePos);
		if (!sourcePos)
			break;
		if (!strcmp(token, "float"))
		{
			type = VAR_FLOAT;
		}
		else if (!strcmp(token, "int"))
		{
			type = VAR_INTEGER;
		}
		else if (!strcmp(token, "string"))
		{
			type = VAR_STRING;
		}
		else
		{
			if (strcmp(token, "vector"))
			{
				v2 = va("Unknown type %s in %s", token, filename);
				Com_Error(ERR_DROP, v2);
				return;
			}
			type = VAR_VECTOR;
		}
		token = (char*)Com_Parse(&sourcePos);
		if (!sourcePos)
		{
			v3 = va("missing field name in %s", filename);
			Com_Error(ERR_DROP, v3);
		}
		v7 = strlen(token);
		len = v7 + 1;
		for (i = v7; i >= 0; --i)
			token[i] = tolower(token[i]);
		index = SL_GetCanonicalString(token);
		if (Scr_FindField(token, tempType))
			Com_Error(ERR_DROP, "duplicate key %s in %s", token, filename);
		TempMemorySetPos(targetPos);
		size = len + 4;
		targetPos = TempMalloc(len + 4);
		v6 = token;
		v5 = targetPos;

		do
		{
			v4 = *v6;
			*v5++ = *v6++;
		} while (v4);

		targetPos += len;
		*(_WORD*)targetPos = index;
		targetPos += 2;
		*targetPos++ = type;
	}
	Com_EndParseSession();
	Hunk_ClearTempMemoryHigh();
}

void  Scr_AddFields_FastFile(char const* path, char const* extension)
{
	char filename[64]; // [esp+0h] [ebp-48h] BYREF
	char* targetPos; // [esp+44h] [ebp-4h]

	scrVarPub.fieldBuffer = TempMalloc(0);
	*scrVarPub.fieldBuffer = 0;
	sprintf(filename, "%s/%s.%s", path, "keys", extension);
	Scr_AddFieldsForFile(filename);
	targetPos = TempMalloc(1);
	*targetPos = 0;
}

unsigned int  GetNewVariableIndexInternal3(unsigned int parentId, unsigned int name, unsigned int index)
{
	VariableValueInternal* parentValue; // [esp+8h] [ebp-40h]
	VariableValueInternal* entry; // [esp+Ch] [ebp-3Ch]
	unsigned int newIndex; // [esp+10h] [ebp-38h]
	unsigned int newIndexb; // [esp+10h] [ebp-38h]
	unsigned int newIndexa; // [esp+10h] [ebp-38h]
	unsigned int prevId; // [esp+14h] [ebp-34h]
	unsigned int next; // [esp+20h] [ebp-28h]
	unsigned int nexta; // [esp+20h] [ebp-28h]
	unsigned int nextb; // [esp+20h] [ebp-28h]
	VariableValueInternal* entryValue; // [esp+24h] [ebp-24h]
	unsigned int prev; // [esp+2Ch] [ebp-1Ch]
	unsigned int preva; // [esp+2Ch] [ebp-1Ch]
	unsigned int prevb; // [esp+2Ch] [ebp-1Ch]
	VariableValueInternal* newEntryValue; // [esp+30h] [ebp-18h]
	int type; // [esp+34h] [ebp-14h]
	VariableValueInternal* newEntrya; // [esp+38h] [ebp-10h]
	VariableValueInternal* newEntry; // [esp+38h] [ebp-10h]
	VariableValue value; // [esp+3Ch] [ebp-Ch]
	unsigned __int16 id; // [esp+44h] [ebp-4h]

	iassert(!(name & ~VAR_NAME_LOW_MASK));
	entry = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
	entryValue = &scrVarGlob.variableList[entry->hash.id + VARIABLELIST_CHILD_BEGIN];
	type = entryValue->w.status & VAR_STAT_MASK;
	if (type)
	{
		if (type == VAR_STAT_HEAD)
		{
			if ((entry->w.status & VAR_STAT_MASK) != 0)
			{
				index = scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next;

				if (!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next)
					Scr_TerminalError("exceeded maximum number of script variables");

				iassert(index > 0 && index < VARIABLELIST_CHILD_SIZE);

				entry = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
				newEntryValue = &scrVarGlob.variableList[entry->hash.id + VARIABLELIST_CHILD_BEGIN];

				iassert((newEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE);

				scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next = newEntryValue->u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next + VARIABLELIST_CHILD_BEGIN].hash.u.prev = 0;
				newEntryValue->w.status = 32;
				newEntryValue->v.next = entryValue->v.next;
				entryValue->v.next = index;
			}
			else
			{
				newIndexb = entry->v.next;
				newEntrya = &scrVarGlob.variableList[newIndexb + VARIABLELIST_CHILD_BEGIN];
				newEntryValue = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
				preva = newEntrya->hash.u.prev;
				nexta = entry->u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[preva + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN].u.next = nexta;
				scrVarGlob.variableList[nexta + VARIABLELIST_CHILD_BEGIN].hash.u.prev = preva;
				newEntrya->hash.id = entry->hash.id;
				entry->hash.id = index;
				newEntrya->hash.u.prev = entry->hash.u.prev;
				scrVarGlob.variableList[scrVarGlob.variableList[newEntrya->hash.u.prev + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN].nextSibling = newIndexb;
				scrVarGlob.variableList[entryValue->nextSibling + VARIABLELIST_CHILD_BEGIN].hash.u.prev = newIndexb;
				entryValue->w.status &= 0xFFFFFF9F;
				entryValue->w.status |= 0x20u;
				entry->w.status = 64;
			}
		}
		else
		{
			iassert(type == VAR_STAT_MOVABLE || type == VAR_STAT_EXTERNAL);
			if ((entry->w.status & VAR_STAT_MASK) != 0)
			{
				newIndexa = scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next;

				if (!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next)
					Scr_TerminalError("exceeded maximum number of script variables");

				newEntry = &scrVarGlob.variableList[newIndexa + VARIABLELIST_CHILD_BEGIN];
				newEntryValue = &scrVarGlob.variableList[newEntry->hash.id + VARIABLELIST_CHILD_BEGIN];
				iassert((newEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE);
				scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next = newEntryValue->u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next + VARIABLELIST_CHILD_BEGIN].hash.u.prev = 0;
			}
			else
			{
				iassert(entry != entryValue);
				newIndexa = entry->v.next;
				newEntry = &scrVarGlob.variableList[newIndexa + VARIABLELIST_CHILD_BEGIN];
				newEntryValue = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
				prevb = newEntry->hash.u.prev;
				nextb = entry->u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[prevb + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN].u.next = nextb;
				scrVarGlob.variableList[nextb + VARIABLELIST_CHILD_BEGIN].hash.u.prev = prevb;
			}

			if (entry->hash.u.prev)
				scrVarGlob.variableList[scrVarGlob.variableList[entry->hash.u.prev + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN].nextSibling = newIndexa;

			if (entryValue->nextSibling)
				scrVarGlob.variableList[entryValue->nextSibling + VARIABLELIST_CHILD_BEGIN].hash.u.prev = newIndexa;

			if (type == 32)
			{
				prevId = scrVarGlob.variableList[entryValue->v.next + VARIABLELIST_CHILD_BEGIN].hash.id;
				iassert((scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_HEAD);

				while (scrVarGlob.variableList[prevId + VARIABLELIST_CHILD_BEGIN].v.next != index)
				{
					prevId = scrVarGlob.variableList[scrVarGlob.variableList[prevId + VARIABLELIST_CHILD_BEGIN].v.next + VARIABLELIST_CHILD_BEGIN].hash.id;
					iassert((scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_HEAD);
				}
				scrVarGlob.variableList[prevId + VARIABLELIST_CHILD_BEGIN].v.next = newIndexa;
			}
			else
			{
				iassert(type == VAR_STAT_EXTERNAL);
				entryValue->v.next = newIndexa;
			}
			newEntry->hash.u.prev = entry->hash.u.prev;
			id = newEntry->hash.id;
			newEntry->hash.id = entry->hash.id;
			entry->hash.id = id;
			newEntryValue->w.status = 64;
			newEntryValue->v.next = index;
		}
	}
	else
	{
		newIndex = entry->v.next;
		next = entryValue->u.next;
		if (newIndex == entry->hash.id || (entry->w.status & VAR_STAT_MASK) != 0)
		{
			newEntryValue = &scrVarGlob.variableList[scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN];
		}
		else
		{
			scrVarGlob.variableList[newIndex + VARIABLELIST_CHILD_BEGIN].hash.id = entry->hash.id;
			entry->hash.id = index;
			entryValue->v.next = newIndex;
			entryValue->u.next = entry->u.next;
			newEntryValue = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
		}
		prev = entry->hash.u.prev;
		iassert(!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prev].hash.id || (scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prev].hash.id].w.status & VAR_STAT_MASK) == VAR_STAT_FREE);
		iassert(!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + next].hash.id || (scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + next].hash.id].w.status & VAR_STAT_MASK) == VAR_STAT_FREE);
		scrVarGlob.variableList[scrVarGlob.variableList[prev + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN].u.next = next;
		scrVarGlob.variableList[next + VARIABLELIST_CHILD_BEGIN].hash.u.prev = prev;
		newEntryValue->w.status = 64;
		newEntryValue->v.next = index;
	}

	iassert(entry == &scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + index]);
	iassert(newEntryValue == &scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + entry->hash.id]);
	iassert((newEntryValue->w.type &VAR_MASK) == VAR_UNDEFINED);

	newEntryValue->w.status = newEntryValue->w.status;
	newEntryValue->w.name |= name << VAR_NAME_BITS;

	iassert((entry->hash.id > 0 && entry->hash.id < VARIABLELIST_CHILD_SIZE));

	++scrVarPub.totalObjectRefCount;

	if (scrVarDebugPub)
	{
		iassert(!scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + entry->hash.id]);

		++scrVarDebugPub->leakCount[entry->hash.id + VARIABLELIST_CHILD_BEGIN];
	}

	++scrVarPub.numScriptValues;

	iassert(scrVarPub.varUsagePos);

	if (scrVarDebugPub)
	{
		iassert(!scrVarDebugPub->varUsage[VARIABLELIST_CHILD_BEGIN + entry->hash.id]);

		scrVarDebugPub->varUsage[entry->hash.id + VARIABLELIST_CHILD_BEGIN] = scrVarPub.varUsagePos;
	}
	parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert((parentValue->w.status &VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert(IsObject(parentValue));
	if ((parentValue->w.status & VAR_MASK) == VAR_ARRAY)
	{
		++parentValue->u.o.u.size;
		value = Scr_GetArrayIndexValue(name);
		AddRefToValue(value.type, value.u);
	}
	return index;
}

unsigned int  GetNewVariableIndexInternal2(unsigned int parentId, unsigned int name, unsigned int index)
{
	unsigned int siblingId; // [esp+0h] [ebp-20h]
	VariableValueInternal* parentValue; // [esp+4h] [ebp-1Ch]
	unsigned int siblingIndex; // [esp+10h] [ebp-10h]
	unsigned int id; // [esp+1Ch] [ebp-4h]
	unsigned int indexa; // [esp+30h] [ebp+10h]

	indexa = GetNewVariableIndexInternal3(parentId, name, index);
	parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	id = scrVarGlob.variableList[indexa + VARIABLELIST_CHILD_BEGIN].hash.id;
	siblingId = parentValue->nextSibling;
	if (parentValue->nextSibling)
	{
		VariableValueInternal *siblingValue = &scrVarGlob.variableList[siblingId + VARIABLELIST_CHILD_BEGIN];
		iassert(!IsObject(siblingValue));
		siblingIndex = FindVariableIndexInternal(parentId, scrVarGlob.variableList[siblingId + VARIABLELIST_CHILD_BEGIN].w.status >> VAR_NAME_BITS);
		iassert(siblingIndex);
		scrVarGlob.variableList[siblingIndex + VARIABLELIST_CHILD_BEGIN].hash.u.prev = indexa;
	}
	else
	{
		siblingIndex = 0;
		scrVarGlob.variableList[parentValue->v.next + 1].hash.u.prev = id;
	}
	parentValue->nextSibling = id;
	scrVarGlob.variableList[indexa + VARIABLELIST_CHILD_BEGIN].hash.u.prev = 0;
	scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].nextSibling = siblingIndex;
	return indexa;
}

unsigned int  GetNewVariableIndexReverseInternal2(unsigned int parentId, unsigned int name, unsigned int index)
{
	VariableValueInternal* parentValue; // [esp+0h] [ebp-20h]
	unsigned int siblingIndex; // [esp+Ch] [ebp-14h]
	VariableValueInternal* siblingValue; // [esp+10h] [ebp-10h]
	VariableValueInternal* parent; // [esp+14h] [ebp-Ch]
	unsigned int id; // [esp+1Ch] [ebp-4h]
	unsigned int indexa; // [esp+30h] [ebp+10h]

	indexa = GetNewVariableIndexInternal3(parentId, name, index);
	parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert(((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	parent = &scrVarGlob.variableList[parentValue->v.next + 1];
	id = scrVarGlob.variableList[indexa + VARIABLELIST_CHILD_BEGIN].hash.id;
	if (parent->hash.u.prev)
	{
		siblingValue = &scrVarGlob.variableList[parent->hash.u.prev + VARIABLELIST_CHILD_BEGIN];
		iassert(!IsObject(siblingValue));
		siblingValue->nextSibling = indexa;
		siblingIndex = FindVariableIndexInternal(parentId, siblingValue->w.status >> VAR_NAME_BITS);
		iassert(siblingIndex);
	}
	else
	{
		siblingIndex = 0;
		parentValue->nextSibling = id;
	}
	parent->hash.u.prev = id;
	scrVarGlob.variableList[indexa + VARIABLELIST_CHILD_BEGIN].hash.u.prev = siblingIndex;
	scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].nextSibling = 0;
	return indexa;
}

unsigned int  GetNewVariableIndexInternal(unsigned int parentId, unsigned int name)
{
	iassert(!FindVariableIndexInternal(parentId, name));
	return GetNewVariableIndexInternal2(parentId, name, (parentId + 101 * name) % 0xFFFD + 1);
}

unsigned int  GetNewVariableIndexReverseInternal(unsigned int parentId, unsigned int name)
{
	iassert(!FindVariableIndexInternal(parentId, name));
	return GetNewVariableIndexReverseInternal2(parentId, name, (parentId + 101 * name) % 0xFFFD + 1);
}

void  MakeVariableExternal(unsigned int index, VariableValueInternal* parentValue)
{
	VariableValueInternal* entry; // [esp+8h] [ebp-38h]
	unsigned int oldPrevSiblingIndex; // [esp+Ch] [ebp-34h]
	unsigned int nextSiblingIndex; // [esp+10h] [ebp-30h]
	unsigned int prevSiblingIndex; // [esp+14h] [ebp-2Ch]
	unsigned int oldIndex; // [esp+1Ch] [ebp-24h]
	VariableValueInternal * entryValue; // [esp+20h] [ebp-20h]
	Variable tempEntry; // [esp+24h] [ebp-1Ch]
	VariableValueInternal* oldEntry; // [esp+28h] [ebp-18h]
	VariableValueInternal* oldEntrya; // [esp+28h] [ebp-18h]
	Variable* prev; // [esp+2Ch] [ebp-14h]
	VariableValue value; // [esp+34h] [ebp-Ch]
	VariableValueInternal* oldEntryValue; // [esp+3Ch] [ebp-4h]

	entry = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
	entryValue = &scrVarGlob.variableList[entry->hash.id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_HEAD);
	iassert(!IsObject(entryValue));

	if ((parentValue->w.status & VAR_MASK) == VAR_ARRAY)
	{
		--parentValue->u.o.u.size;
		iassert(!IsObject(entryValue));
		value = Scr_GetArrayIndexValue(entryValue->w.status >> 8);
		RemoveRefToValue(value.type, value.u);
	}
	if ((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_HEAD)
	{
		oldIndex = entryValue->v.next;
		oldEntry = &scrVarGlob.variableList[oldIndex + VARIABLELIST_CHILD_BEGIN];
		oldEntryValue = &scrVarGlob.variableList[oldEntry->hash.id + VARIABLELIST_CHILD_BEGIN];

		if (oldEntry != entry)
		{
			iassert((oldEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE);
			oldEntryValue->w.status &= 0xFFFFFF9F;
			oldEntryValue->w.status |= VAR_STAT_HEAD;
			prevSiblingIndex = entry->hash.u.prev;
			nextSiblingIndex = entryValue->nextSibling;
			oldPrevSiblingIndex = oldEntry->hash.u.prev;
			if (oldEntryValue->nextSibling)
				scrVarGlob.variableList[oldEntryValue->nextSibling + VARIABLELIST_CHILD_BEGIN].hash.u.prev = index;
			if (oldPrevSiblingIndex)
				scrVarGlob.variableList[scrVarGlob.variableList[oldPrevSiblingIndex + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN].nextSibling = index;
			if (nextSiblingIndex)
				scrVarGlob.variableList[nextSiblingIndex + VARIABLELIST_CHILD_BEGIN].hash.u.prev = oldIndex;
			if (prevSiblingIndex)
				scrVarGlob.variableList[scrVarGlob.variableList[prevSiblingIndex + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN].nextSibling = oldIndex;
			tempEntry = entry->hash;
			entry->hash = oldEntry->hash;
			oldEntry->hash = tempEntry;
			entry = &scrVarGlob.variableList[oldIndex + VARIABLELIST_CHILD_BEGIN];
			index = oldIndex;
		}
	}
	else
	{
		oldEntrya = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
		oldEntryValue = entryValue;
		do
		{
			iassert((oldEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (oldEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_HEAD);
			prev = &oldEntrya->hash;
			oldEntrya = &scrVarGlob.variableList[oldEntryValue->v.next + VARIABLELIST_CHILD_BEGIN];
			oldEntryValue = &scrVarGlob.variableList[oldEntrya->hash.id + VARIABLELIST_CHILD_BEGIN];
		} while (oldEntrya != entry);
		scrVarGlob.variableList[prev->id + VARIABLELIST_CHILD_BEGIN].v.next = entryValue->v.next;
	}
	iassert(entryValue == &scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + entry->hash.id]);
	entryValue->w.status &= 0xFFFFFF9F;
	entryValue->w.status |= 0x60u;
	entryValue->v.next = index;
}

void  FreeChildValue(unsigned int parentId, unsigned int id)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-20h]
	unsigned int nextSiblingIndex; // [esp+4h] [ebp-1Ch]
	unsigned int prevSiblingIndex; // [esp+8h] [ebp-18h]
	unsigned int parentIndex; // [esp+Ch] [ebp-14h]
	VariableValueInternal* entryValue; // [esp+10h] [ebp-10h]
	unsigned int index; // [esp+14h] [ebp-Ch]

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

	iassert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert(!IsObject(entryValue));
	iassert(scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + entryValue->v.index].hash.id == id);

	RemoveRefToValue(entryValue->w.status & 0x1F, entryValue->u.u);

	iassert(id > 0 && id < VARIABLELIST_CHILD_SIZE);

	--scrVarPub.totalObjectRefCount;

	if (scrVarDebugPub)
	{
		iassert(scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + id]);
		scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + id]--;
		iassert(!scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + id]);
	}

	--scrVarPub.numScriptValues;

	if (scrVarDebugPub)
	{
		iassert(scrVarDebugPub->varUsage[VARIABLELIST_CHILD_BEGIN + id]);
		scrVarDebugPub->varUsage[id + VARIABLELIST_CHILD_BEGIN] = 0;
	}

	iassert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);

	index = entryValue->v.next;
	entry = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];

	iassert(entry->hash.id == id);

	nextSiblingIndex = entryValue->nextSibling;
	prevSiblingIndex = entry->hash.u.prev;

	if (entry->hash.u.prev)
	{
		scrVarGlob.variableList[scrVarGlob.variableList[prevSiblingIndex + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN].nextSibling = nextSiblingIndex;
	}
	else
	{
		iassert(!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].hash.id);
		scrVarGlob.variableList[parentId + 1].nextSibling = scrVarGlob.variableList[nextSiblingIndex + VARIABLELIST_CHILD_BEGIN].hash.id;
	}
	if (nextSiblingIndex)
	{
		scrVarGlob.variableList[nextSiblingIndex + VARIABLELIST_CHILD_BEGIN].hash.u.prev = prevSiblingIndex;
	}
	else
	{
		parentIndex = scrVarGlob.variableList[parentId + 1].v.next;
		iassert(!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].hash.id);
		scrVarGlob.variableList[parentIndex + 1].hash.u.prev = scrVarGlob.variableList[prevSiblingIndex + VARIABLELIST_CHILD_BEGIN].hash.id;
	}
	entryValue->w.status = 0;
	entryValue->u.next = scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next;
	entry->hash.u.prev = 0;
	scrVarGlob.variableList[scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next + VARIABLELIST_CHILD_BEGIN].hash.u.prev = index;
	scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].u.next = index;
}

void  ClearObjectInternal(unsigned int parentId)
{
	unsigned int nextId; // [esp+0h] [ebp-18h]
	unsigned int nextSibling; // [esp+4h] [ebp-14h]
	unsigned int nextSiblinga; // [esp+4h] [ebp-14h]
	VariableValueInternal* parentValue; // [esp+8h] [ebp-10h]
	VariableValueInternal* entryValue; // [esp+10h] [ebp-8h]
	unsigned int id; // [esp+14h] [ebp-4h]
	unsigned int ida; // [esp+14h] [ebp-4h]

	parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert(IsObject(parentValue));

	if (parentValue->nextSibling)
	{
		entryValue = &scrVarGlob.variableList[parentValue->nextSibling + VARIABLELIST_CHILD_BEGIN];
		iassert(!IsObject(entryValue));

		nextSibling = FindVariableIndexInternal(parentId, entryValue->w.status >> 8);
		iassert(nextSibling);
		do
		{
			id = scrVarGlob.variableList[nextSibling + VARIABLELIST_CHILD_BEGIN].hash.id;
			MakeVariableExternal(nextSibling, parentValue);
			nextSibling = scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].nextSibling;
		} while (scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].nextSibling);
		nextId = parentValue->nextSibling;
		do
		{
			ida = nextId;
			nextSiblinga = scrVarGlob.variableList[nextId + VARIABLELIST_CHILD_BEGIN].nextSibling;
			nextId = scrVarGlob.variableList[nextSiblinga + VARIABLELIST_CHILD_BEGIN].hash.id;
			FreeChildValue(parentId, ida);
		} while (nextSiblinga);
	}
}

unsigned int  GetNewArrayVariableIndex(unsigned int parentId, unsigned int unsignedValue)
{
	iassert(IsValidArrayIndex(unsignedValue));

	return GetNewVariableIndexInternal(parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

void  RemoveArrayVariable(unsigned int parentId, unsigned int unsignedValue)
{
	iassert(IsValidArrayIndex(unsignedValue));

	RemoveVariable(parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

void  CopyArray(unsigned int parentId, unsigned int newParentId)
{
	unsigned int nextSibling; // [esp+0h] [ebp-20h]
	VariableValueInternal* entryValue; // [esp+Ch] [ebp-14h]
	int type; // [esp+10h] [ebp-10h]
	VariableValueInternal* newEntryValue; // [esp+14h] [ebp-Ch]
	unsigned int id; // [esp+1Ch] [ebp-4h]

	VariableValueInternal *parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert(IsObject(parentValue));
	iassert((parentValue->w.type & VAR_MASK) == VAR_ARRAY);

	id = scrVarGlob.variableList[parentId + 1].nextSibling;
	if (id)
	{
		while (1)
		{
			entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

			iassert(!IsObject(entryValue));

			newEntryValue = &scrVarGlob.variableList[scrVarGlob.variableList[GetVariableIndexInternal(newParentId, entryValue->w.classnum >> VAR_NAME_BITS) + VARIABLELIST_CHILD_BEGIN].hash.id + VARIABLELIST_CHILD_BEGIN];

			iassert((newEntryValue->w.type & VAR_MASK) == VAR_UNDEFINED);
			iassert(!(newEntryValue->w.type & VAR_MASK));

			type = entryValue->w.type & VAR_MASK;
			newEntryValue->w.type |= type;

			if (type == VAR_POINTER)
			{
				if ((scrVarGlob.variableList[entryValue->u.u.pointerValue + 1].w.type & VAR_MASK) == VAR_ARRAY)
				{
					newEntryValue->u.u.pointerValue = Scr_AllocArray();
					CopyArray(entryValue->u.u.pointerValue, newEntryValue->u.u.pointerValue);
				}
				else
				{
					newEntryValue->u.u = entryValue->u.u;
					AddRefToObject(entryValue->u.u.stringValue);
				}
			}
			else
			{
				iassert(type != VAR_STACK);
				newEntryValue->u.u = entryValue->u.u;
				AddRefToValue(type, entryValue->u.u);
			}
			nextSibling = scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].nextSibling;
			if (!scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN].nextSibling)
				break;

			id = scrVarGlob.variableList[nextSibling + VARIABLELIST_CHILD_BEGIN].hash.id;
			iassert(id);
		}
	}
}


void  Scr_CastWeakerPair(VariableValue* value1, VariableValue* value2)
{
	int type1; // [esp+0h] [ebp-Ch]
	float* tempVector; // [esp+4h] [ebp-8h]
	float* tempVectora; // [esp+4h] [ebp-8h]
	float* tempVectorb; // [esp+4h] [ebp-8h]
	float* tempVectorc; // [esp+4h] [ebp-8h]
	int type2; // [esp+8h] [ebp-4h]

	type1 = value1->type;
	type2 = value2->type;

	if (type1 != type2)
	{
		if (type1 == VAR_FLOAT && type2 == VAR_INTEGER)
		{
			value2->type = VAR_FLOAT;
			value2->u.floatValue = (float)value2->u.intValue;
			return;
		}
		if (type1 == VAR_INTEGER && type2 == VAR_FLOAT)
		{
			value1->type = VAR_FLOAT;
			value1->u.floatValue = (float)value1->u.intValue;
			return;
		}
		if (type1 == VAR_VECTOR)
		{
			if (type2 == VAR_FLOAT)
			{
				tempVector = Scr_AllocVector();
				tempVector[0] = value2->u.floatValue;
				tempVector[1] = value2->u.floatValue;
				tempVector[2] = value2->u.floatValue;
				value2->u.vectorValue = tempVector;
				value2->type = VAR_VECTOR;
				return;
			}
			if (type2 == VAR_INTEGER)
			{
				tempVectora = Scr_AllocVector();
				tempVectora[0] = (float)value2->u.intValue;
				tempVectora[1] = (float)value2->u.intValue;
				tempVectora[2] = (float)value2->u.intValue;
				value2->u.vectorValue = tempVectora;
				value2->type = VAR_VECTOR;
				return;
			}
		}
		if (type2 != VAR_VECTOR)
			goto LABEL_18;
		if (type1 == VAR_FLOAT)
		{
			tempVectorb = Scr_AllocVector();
			tempVectorb[0] = value1->u.floatValue;
			tempVectorb[1] = value1->u.floatValue;
			tempVectorb[2] = value1->u.floatValue;
			value1->u.vectorValue = tempVectorb;
			value1->type = VAR_VECTOR;
			return;
		}
		if (type1 == VAR_INTEGER)
		{
			tempVectorc = Scr_AllocVector();
			tempVectorc[0] = (float)value1->u.intValue;
			tempVectorc[1] = (float)value1->u.intValue;
			tempVectorc[2] = (float)value1->u.intValue;
			value1->u.vectorValue = tempVectorc;
			value1->type = VAR_VECTOR;
		}
		else
		{
		LABEL_18:
			Scr_UnmatchingTypesError(value1, value2);
		}
	}
}

void  Scr_CastWeakerStringPair(VariableValue* value1, VariableValue* value2)
{
	Vartype_t type1; // [esp+14h] [ebp-Ch]
	Vartype_t type2; // [esp+18h] [ebp-8h]
	const float* constTempVector; // [esp+1Ch] [ebp-4h]
	const float* constTempVectora; // [esp+1Ch] [ebp-4h]

	type1 = value1->type;
	type2 = value2->type;

	if (type1 != type2)
	{
		if (type1 < type2)
		{
			if (type1 == VAR_STRING)
			{
				switch (type2)
				{
				case VAR_VECTOR:
					value2->type = VAR_STRING;
					constTempVector = (const float*)value2->u.intValue;
					value2->u.stringValue = SL_GetStringForVector(value2->u.vectorValue);
					RemoveRefToVector(constTempVector);
					return;
				case VAR_FLOAT:
					value2->type = VAR_STRING;
					value2->u.stringValue = SL_GetStringForFloat(value2->u.floatValue);
					return;
				case VAR_INTEGER:
					value2->type = VAR_STRING;
					value2->u.stringValue = SL_GetStringForInt(value2->u.intValue);
					return;
				}
			}
			else if (type1 != VAR_FLOAT)
			{
			LABEL_28:
				Scr_UnmatchingTypesError(value1, value2);
				return;
			}
			if (type2 == VAR_INTEGER)
			{
				value2->type = VAR_FLOAT;
				value2->u.floatValue = (float)value2->u.intValue;
				return;
			}
			goto LABEL_28;
		}
		if (type2 == VAR_STRING)
		{
			switch (type1)
			{
			case VAR_VECTOR:
				value1->type = VAR_STRING;
				constTempVectora = (const float*)value1->u.intValue;
				value1->u.stringValue = SL_GetStringForVector(value1->u.vectorValue);
				RemoveRefToVector(constTempVectora);
				return;
			case VAR_FLOAT:
				value1->type = VAR_STRING;
				value1->u.stringValue = SL_GetStringForFloat(value1->u.floatValue);
				return;
			case VAR_INTEGER:
				value1->type = VAR_STRING;
				value1->u.stringValue = SL_GetStringForInt(value1->u.intValue);
				return;
			}
		}
		else if (type2 != VAR_FLOAT)
		{
			goto LABEL_28;
		}
		if (type1 == VAR_INTEGER)
		{
			value1->type = VAR_FLOAT;
			value1->u.floatValue = (float)value1->u.intValue;
			return;
		}
		goto LABEL_28;
	}
}
float  Scr_GetEndonUsage(unsigned int parentId)
{
	VariableValueInternal_u Object; // eax
	unsigned int id; // [esp+4h] [ebp-4h]

	VariableValueInternal *parentValue = &scrVarGlob.variableList[parentId + 1];

	iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(IsObject(parentValue));

	id = FindObjectVariable(scrVarPub.pauseArrayId, parentId);
	if (!id)
		return 0.0;
	Object = FindObject(id);
	return Scr_GetObjectUsage(Object.u.stringValue);
}

float  Scr_GetThreadUsage(const VariableStackBuffer* stackBuf, float* endonUsage)
{
	const char* buf; // [esp+0h] [ebp-18h]
	const char* bufa; // [esp+0h] [ebp-18h]
	unsigned int localId; // [esp+4h] [ebp-14h]
	int size; // [esp+8h] [ebp-10h]
	float usage; // [esp+Ch] [ebp-Ch]
	VariableUnion u; // [esp+10h] [ebp-8h]

	size = stackBuf->size;
	buf = &stackBuf->buf[5 * size];
	usage = Scr_GetObjectUsage(stackBuf->localId);
	*endonUsage = Scr_GetEndonUsage(stackBuf->localId);
	localId = stackBuf->localId;
	while (size)
	{
		bufa = buf - 4;
		u.intValue = *(int*)bufa;
		buf = bufa - 1;
		--size;
		if (*buf == 7)
		{
			localId = GetParentLocalId(localId);
			usage = Scr_GetObjectUsage(localId) + usage;
			*endonUsage = Scr_GetEndonUsage(localId) + *endonUsage;
		}
		else
		{
			usage = Scr_GetEntryUsage(*buf, u) + usage;
		}
	}
	return usage;
}

int  Scr_MakeValuePrimitive(unsigned int parentId)
{
	VariableValueInternal* parentValue; // [esp+4h] [ebp-10h]
	unsigned int name; // [esp+8h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+Ch] [ebp-8h]
	unsigned int id; // [esp+10h] [ebp-4h]

	parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert(IsObject(parentValue));
	iassert((parentValue->w.type & VAR_MASK) != VAR_THREAD);
	iassert((parentValue->w.type & VAR_MASK) != VAR_NOTIFY_THREAD);
	iassert((parentValue->w.type & VAR_MASK) != VAR_TIME_THREAD);
	iassert((parentValue->w.type & VAR_MASK) != VAR_CHILD_THREAD);

	if ((parentValue->w.type & VAR_MASK) != VAR_ARRAY)
		return 0;

	while (2)
	{
		id = FindFirstSibling(parentId);
	LABEL_17:
		if (id)
		{
			entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

			iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL);
			iassert(!IsObject(entryValue));

			name = entryValue->w.name >> VAR_NAME_BITS;

			switch (entryValue->w.type & VAR_MASK)
			{
			case VAR_UNDEFINED:
			case VAR_STRING:
			case VAR_ISTRING:
			case VAR_VECTOR:
			case VAR_END_REF:
			case VAR_INTEGER:
				goto LABEL_16;
			case VAR_POINTER:
				if (Scr_MakeValuePrimitive(entryValue->u.u.stringValue))
					goto LABEL_16;
				goto $LN6_69;
			case VAR_CODEPOS:
			case VAR_PRECODEPOS:
			case VAR_FUNCTION:
			case VAR_STACK:
			case VAR_ANIMATION:
			$LN6_69:
				RemoveVariable(parentId, name);
				continue;
			default:
				if (!alwaysfails)
					MyAssertHandler(".\\script\\scr_variable.cpp", 5290, 0, "unknown type");
			LABEL_16:
				id = FindNextSibling(id);
				goto LABEL_17;
			}
		}
		return 1;
	}
}

void  SafeRemoveArrayVariable(unsigned int parentId, unsigned int unsignedValue)
{
	iassert(IsValidArrayIndex(unsignedValue));

	SafeRemoveVariable(parentId, (unsignedValue + MAX_ARRAYINDEX) & VAR_NAME_LOW_MASK);
}

VariableValue  Scr_EvalVariableEntityField(unsigned int entId, unsigned int fieldName)
{
	VariableValueInternal* entValue; // [esp+8h] [ebp-18h]
	unsigned int fieldId; // [esp+10h] [ebp-10h]
	VariableValue valuea; // [esp+14h] [ebp-Ch]
	VariableUnion id; // [esp+1Ch] [ebp-4h]

	entValue = &scrVarGlob.variableList[entId + VARIABLELIST_PARENT_BEGIN];

	iassert((entValue->w.type & VAR_MASK) == VAR_ENTITY);
	iassert((entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT);

	unsigned int classnum = entValue->w.classnum >> VAR_NAME_BITS;
	fieldId = FindArrayVariable(g_classMap[classnum].id, fieldName);
	if (fieldId)
	{
		valuea = GetEntityFieldValue(classnum, entValue->u.o.u.entnum, scrVarGlob.variableList[fieldId + VARIABLELIST_CHILD_BEGIN].u.u.entityOffset);
		if (valuea.type == VAR_POINTER)
		{
			if ((scrVarGlob.variableList[valuea.u.pointerValue + 1].w.type & VAR_MASK) == VAR_ARRAY)
			{
				if (scrVarGlob.variableList[valuea.u.pointerValue + 1].u.next)
				{
					id.pointerValue = valuea.u.pointerValue;
					RemoveRefToObject(valuea.u.pointerValue);
					valuea.u.pointerValue = Scr_AllocArray();
					CopyArray(id.pointerValue, valuea.u.pointerValue);
				}
				return valuea;
			}
			else
			{
				return valuea;
			}
		}
		else
		{
			return valuea;
		}
	}
	else
	{
		valuea.type = VAR_UNDEFINED;
		valuea.u.intValue = 0;
		return valuea;
	}
}

void  Scr_ClearThread(unsigned int parentId)
{
	VariableValueInternal* parentValue; // [esp+0h] [ebp-4h]

	iassert(parentId);

	parentValue = &scrVarGlob.variableList[parentId + 1];

	iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(((parentValue->w.type & VAR_MASK) >= VAR_THREAD) && ((parentValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD));
	iassert(!FindVariable(parentId, OBJECT_STACK));

	if (parentValue->nextSibling)
		ClearObjectInternal(parentId);

	RemoveRefToObject(parentValue->u.o.u.size);
}

void Scr_GetChecksum(unsigned int *checksum)
{
	const char *programBuffer; // r11

	if (!scrVarPub.endScriptBuffer)
		MyAssertHandler(
			"c:\\trees\\cod3\\cod3src\\src\\script\\scr_variable.cpp",
			5337,
			0,
			"%s",
			"scrVarPub.endScriptBuffer");
	programBuffer = scrVarPub.programBuffer;
	if (!scrVarPub.programBuffer)
	{
		MyAssertHandler("c:\\trees\\cod3\\cod3src\\src\\script\\scr_variable.cpp", 5338, 0, "%s", "scrVarPub.programBuffer");
		programBuffer = scrVarPub.programBuffer;
	}
	if (programBuffer > scrVarPub.endScriptBuffer)
		MyAssertHandler(
			"c:\\trees\\cod3\\cod3src\\src\\script\\scr_variable.cpp",
			5339,
			0,
			"%s",
			"scrVarPub.programBuffer <= scrVarPub.endScriptBuffer");
	*checksum = scrVarPub.checksum;
	checksum[1] = scrCompilePub.programLen;
	checksum[2] = scrVarPub.endScriptBuffer - scrVarPub.programBuffer;
}