#include "scr_main.h"
#include "scr_variable.h"
#include "scr_stringlist.h"
#include "scr_memorytree.h"
#include "scr_vm.h"

#include <qcommon/qcommon.h>
#include <Windows.h>
#include <universal/com_files.h>
#include "scr_parser.h"
#include <database/database.h>
#include <universal/q_parse.h>

#undef GetObject
#undef FindObject

scrVarPub_t scrVarPub;
scrVarDebugPub_t* scrVarDebugPub;
scrVarDebugPub_t scrVarDebugPubBuf;
scrVarGlob_t scrVarGlob;

static scr_classStruct_t g_classMap[4] =
{
	{ 0, 0, 0x65, "entity" },
	{ 0, 0, 0x68, "hudelem" },
	{ 0, 0, 0x70, "pathnode" },
	{ 0, 0, 0x76, "vehiclenode" }
};

bool IsObject(VariableValueInternal* entryValue)
{
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	
	return (entryValue->w.status & 0x1F) >= 14;
}

bool IsObject(VariableValue* value)
{
	return value->type >= 14;
}

void Scr_InitVariables()
{
	if (!scrVarDebugPub)
		scrVarDebugPub = &scrVarDebugPubBuf;

	memset((unsigned __int8*)scrVarDebugPub->leakCount, 0, sizeof(scrVarDebugPub->leakCount));

	scrVarPub.totalObjectRefCount = 0;
	scrVarPub.totalVectorRefCount = 0;

	if (scrVarDebugPub)
		memset((unsigned __int8*)scrVarDebugPub->extRefCount, 0, sizeof(scrVarDebugPub->extRefCount));

	scrVarPub.numScriptValues = 0;
	scrVarPub.numScriptObjects = 0;

	if (scrVarDebugPub)
		memset((unsigned __int8*)scrVarDebugPub, 0, 0x60000u);

	Scr_InitVariableRange(1u, 0x8001u);
	Scr_InitVariableRange(0x8002u, 0x18000u);
}

void Scr_InitVariableRange(unsigned int begin, unsigned int end)
{
	unsigned int index; // [esp+4h] [ebp-8h]
	VariableValueInternal* value; // [esp+8h] [ebp-4h]
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

	iassert(!(value->w.type & VAR_MASK));

	valuea->w.status = valuea->w.status;
	valuea->hash.id = 0;
	valuea->v.next = 0;
	valuea->u.next = 1;
	scrVarGlob.variableList[begin + 1].hash.u.prev = 0;
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

unsigned int  Scr_GetNumScriptVars(void)
{
	return scrVarPub.numScriptObjects + scrVarPub.numScriptValues;
}

unsigned int  GetVariableKeyObject(unsigned int id)
{
	iassert(!IsObject(&scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + id]));
	return (scrVarGlob.variableList[id + 0x8002].w.status >> 8) - 0x10000;
}

unsigned int  Scr_GetVarId(unsigned int index)
{
	return scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN].hash.id;
}

void  Scr_SetThreadNotifyName(unsigned int startLocalId, unsigned int stringValue)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[startLocalId + 1];

	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	iassert(((entryValue->w.type & VAR_MASK) == VAR_THREAD));

	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status = (unsigned __int8)entryValue->w.status;
	entryValue->w.status |= 0xFu;
	entryValue->w.status |= stringValue << 8;
}

unsigned short  Scr_GetThreadNotifyName(unsigned int startLocalId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + startLocalId].w.type & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + startLocalId].w.type & VAR_MASK) == VAR_NOTIFY_THREAD);

	return scrVarGlob.variableList[startLocalId + 1].w.status >> 8;
}

void  Scr_SetThreadWaitTime(unsigned int startLocalId, unsigned int waitTime)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[startLocalId + 1];
	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert(((entryValue->w.type & VAR_MASK) == VAR_THREAD) || !Scr_GetThreadNotifyName(startLocalId));

	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status = (unsigned __int8)entryValue->w.status;
	entryValue->w.status |= 0x10u;

	scrVarGlob.variableList[startLocalId + 1].w.status |= waitTime << 8;
}

void  Scr_ClearWaitTime(unsigned int startLocalId)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[startLocalId + 1];
	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert((entryValue->w.type & VAR_MASK) == VAR_TIME_THREAD);

	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status |= 0xEu;
}

unsigned int  Scr_GetThreadWaitTime(unsigned int startLocalId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + startLocalId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + startLocalId].w.type & VAR_MASK) == VAR_TIME_THREAD);

	return scrVarGlob.variableList[startLocalId + 1].w.status >> 8;
}

unsigned int  GetParentLocalId(unsigned int threadId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) == VAR_CHILD_THREAD);

	return scrVarGlob.variableList[threadId + 1].w.status >> 8;
}

unsigned int  GetSafeParentLocalId(unsigned int threadId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) >= VAR_THREAD 
		&& (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD);

	if ((scrVarGlob.variableList[threadId + 1].w.status & VAR_MASK) == VAR_CHILD_THREAD)
		return scrVarGlob.variableList[threadId + 1].w.status >> 8;
	else
		return 0;
}

unsigned int  GetStartLocalId(unsigned int threadId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL);
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) >= VAR_THREAD 
		&& (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD);

	while ((scrVarGlob.variableList[threadId + 1].w.status & 0x1F) == 0x11)
		threadId = scrVarGlob.variableList[threadId + 1].w.status >> 8;

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
	{
		Com_Error(ERR_DROP, "exceeded maximum number of script variables (KISAK)");
		//Scr_TerminalError("exceeded maximum number of script variables");
	}

	entry = &scrVarGlob.variableList[index + VARIABLELIST_CHILD_BEGIN];
	entryValue = &scrVarGlob.variableList[entry->hash.id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE);

	next = entryValue->u.next;
	if (entry != entryValue && (entry->w.status & 0x60) == 0)
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
	entryValue->w.status = 96;

	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.status = entryValue->w.status;
	return entry->hash.id;
}

unsigned int  AllocObject(void)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id + 1];
	entryValue->w.status = 96;

	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.status |= 0x12u;
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
	entryValue = &scrVarGlob.variableList[id + 1];
	entryValue->w.status = 96;

	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.status |= 0x15u;
	entryValue->u.next = 0;
	entryValue->u.o.u.size = 0;
	scrVarPub.varUsagePos = varUsagePos;
	return id;
}

unsigned int  AllocThread(ObjectInfo_u self)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id + 1];
	entryValue->w.status = 96;

	iassert(!(entryValue->w.type & VAR_MASK));

	entryValue->w.status |= 0xEu;
	entryValue->u.next = 0;
	entryValue->u.o.u = self;
	return id;
}

unsigned int  AllocChildThread(ObjectInfo_u self, unsigned int parentLocalId)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	id = AllocVariable();
	entryValue = &scrVarGlob.variableList[id + 1];
	entryValue->w.status = 96;
	iassert(!(entryValue->w.type & VAR_MASK));
	entryValue->w.status |= 0x11u;
	iassert(!(entryValue->w.parentLocalId & VAR_NAME_HIGH_MASK)); 
	entryValue->w.status |= parentLocalId << 8;
	entryValue->u.next = 0;
	entryValue->u.o.u = self;

	return id;
}

unsigned int  Scr_GetSelf(unsigned int threadId)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) >= VAR_THREAD) &&
		((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + threadId].w.type & VAR_MASK) <= VAR_CHILD_THREAD));

	return scrVarGlob.variableList[threadId + 1].u.o.u.size;
}

void  AddRefToObject(unsigned int id)
{
	iassert(id >= 1 && id < VARIABLELIST_PARENT_SIZE);

	++scrVarPub.totalObjectRefCount;
	if (scrVarDebugPub)
	{
		iassert(scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]);
		++scrVarDebugPub->leakCount[id + 1];
	}
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(IsObject(&scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id]));
	iassert(scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id].u.o.refCount);
}

void  RemoveRefToEmptyObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[id + 1];
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
			--scrVarDebugPub->leakCount[id + 1];
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

	entryValue = &scrVarGlob.variableList[id + 1];

	iassert(((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));
	iassert(IsObject(entryValue));
	return entryValue->u.next;
}

float const* Scr_AllocVector(float const* v)
{
	float* result; // eax

	result = Scr_AllocVector();
	*result = *v;
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
	unsigned int value; // [esp+0h] [ebp-4h]

	value = type - 1;
	if ((unsigned int)(type - 1) < 4)
	{
		if (type == 1)
		{
			RemoveRefToObject(u.stringValue);
		}
		else if (value > 2)
		{
			if (type != 4)
				MyAssertHandler(
					".\\script\\scr_variable.cpp",
					2419,
					0,
					"%s\n\t(value) = %i",
					"(value == VAR_VECTOR - VAR_BEGIN_REF)",
					value);
			RemoveRefToVector(u.vectorValue);
		}
		else
		{
			SL_RemoveRefToString(u.stringValue);
		}
	}
}

bool  IsValidArrayIndex(unsigned int unsignedValue)
{
	//  return (char *)&clients[0].parseEntities[1742].un1.helicopterStage + 3 >= (char *)&loc_7E7FFB + unsignedValue + 5;
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

	if (name >= 0x10000)
	{
		if (name >= 0x18000)
		{
			value.type = 6;
			value.u.intValue = name - 0x800000;
		}
		else
		{
			value.type = 1;
			value.u.intValue = name - 0x10000;
		}
	}
	else
	{
		value.type = 2;
		value.u.intValue = (unsigned __int16)name;
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
	entryValue->u.u.intValue = value->u.intValue;
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

	entryValue->w.status |= value->type;
	entryValue->u.u.intValue = value->u.intValue;
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

VariableValueInternal_u Scr_EvalVariableObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-8h]
	unsigned int type; // [esp+4h] [ebp-4h]

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	iassert(((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE) || !id);

	type = entryValue->w.status & 0x1F;
	if (type == 1 && (type = scrVarGlob.variableList[entryValue->u.u.intValue + 1].w.status & 0x1F, type < 0x15))
	{
		if ((scrVarGlob.variableList[entryValue->u.u.intValue + 1].w.status & 0x1F) < 0xE)
			MyAssertHandler(".\\script\\scr_variable.cpp", 3023, 0, "%s", "type >= FIRST_OBJECT");

		return entryValue->u;
	}
	else
	{
		const char* v2 = va("%s is not a field object", var_typename[type]);
		Com_Error(ERR_DROP, "%s", v2);
		//Scr_Error(v2);
	}
}

unsigned int  GetArraySize(unsigned int id)
{
	iassert(id);
	iassert((scrVarGlob.variableList[id + 1].w.status & VAR_MASK) == VAR_ARRAY);

	return scrVarGlob.variableList[id + 1].u.o.u.size;
}

unsigned int  FindFirstSibling(unsigned int id)
{
	iassert(id);
	iassert(IsObject(&scrVarGlob.variableList[id + 1]));

	return scrVarGlob.variableList[id + 1].nextSibling;
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
	parentValue = &scrVarGlob.variableList[parentId + 1];
	iassert(IsObject(parentValue));
	iassert(((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL));

	parent = &scrVarGlob.variableList[parentValue->v.next + 1];
	if (!parent->hash.u.prev)
		return 0;
	index = FindVariableIndexInternal(parentId, scrVarGlob.variableList[parent->hash.u.prev + 32770].w.status >> 8);

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

VariableValueInternal_u GetObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(id);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED || (entryValue->w.type & VAR_MASK) == VAR_POINTER);

	if ((entryValue->w.status & 0x1F) == 0)
	{
		entryValue->w.status |= 1u;
		entryValue->u.u.intValue = AllocObject();
	}

	iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u;
}

VariableValueInternal_u GetArray(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(id);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert((entryValue->w.type & VAR_MASK) == VAR_UNDEFINED || (entryValue->w.type & VAR_MASK) == VAR_POINTER);

	if ((entryValue->w.status & 0x1F) == 0)
	{
		entryValue->w.status |= 1u;
		entryValue->u.u.intValue = Scr_AllocArray();
	}

	iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u;
}

VariableValueInternal_u FindObject(unsigned int id)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(id);

	entryValue = &scrVarGlob.variableList[id + VARIABLELIST_CHILD_BEGIN];

	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);

	return entryValue->u;
}

bool  IsFieldObject(unsigned int id)
{
	iassert(id);
	iassert(IsObject(&scrVarGlob.variableList[id + 1]));

	return (scrVarGlob.variableList[id + 1].w.status & 0x1F) < 0x15;
}

int Scr_IsThreadAlive(unsigned int thread)
{
	VariableValueInternal* entryValue; // [esp+0h] [ebp-4h]

	iassert(scrVarPub.timeArrayId);

	entryValue = &scrVarGlob.variableList[thread + 1];
	
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	if (((entryValue->w.status & 0x1F) < 0xE || (entryValue->w.status & 0x1F) > 0x11) && (entryValue->w.status & 0x1F) != 0x16)
	{
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			3276,
			0,
			"%s",
			"((entryValue->w.type & VAR_MASK) >= VAR_THREAD && (entryValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD) || (entryVa"
			"lue->w.type & VAR_MASK) == VAR_DEAD_THREAD");
	}

	return (entryValue->w.status & 0x1F) != 22;
}

bool  IsObjectFree(unsigned int id)
{
	return (scrVarGlob.variableList[id + 1].w.status & 0x60) == 0;
}

unsigned int GetValueType(unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	return scrVarGlob.variableList[id + 32770].w.status & 0x1F;
}

unsigned int GetObjectType(unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + id].w.status & VAR_STAT_MASK) != VAR_STAT_FREE);

	return scrVarGlob.variableList[id + 1].w.status & 0x1F;
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

VariableValueInternal_u Scr_GetOffset(unsigned int classnum, const char* name)
{
	unsigned int v2; // eax
	unsigned int classId; // [esp+0h] [ebp-8h]
	unsigned int fieldId; // [esp+4h] [ebp-4h]

	classId = g_classMap[classnum].id;
	v2 = SL_ConvertFromString(name);
	fieldId = FindVariable(classId, v2);
	if (fieldId)
	{
		return scrVarGlob.variableList[fieldId + 32770].u;
	}
	else
	{
		return -1;
	}
}

VariableValueInternal_u FindEntityId(unsigned int entnum, unsigned int classnum)
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
		entryValue = &scrVarGlob.variableList[id + 32770];
		iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
		iassert((entryValue->w.type & VAR_MASK) == VAR_POINTER);
		iassert(entryValue->u.u.pointerValue);

		return entryValue->u;
	}
	else
	{
		return 0;
	}
}

void  SetEmptyArray(unsigned int parentId)
{
	VariableValue tempValue; // [esp+0h] [ebp-8h] BYREF

	tempValue.type = 1;
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
		entryValue = &scrVarGlob.variableList[id + 32770];
		if ((entryValue->w.status & 0x60) == 0 || (entryValue->w.status & 0x60) == 0x60)
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				4805,
				0,
				"%s",
				"(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL");

		iassert(!IsObject(entryValue));

		ArrayIndexValue_DONE = Scr_GetArrayIndexValue(entryValue->w.status >> 8);

		if (ArrayIndexValue_DONE.type == 2)
		{
			Scr_AddConstString(ArrayIndexValue_DONE.u.stringValue);
		}
		else if (ArrayIndexValue_DONE.type == 6)
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
	const char* posa; // [esp+10h] [ebp-Ch]
	int len; // [esp+14h] [ebp-8h]
	unsigned int index; // [esp+18h] [ebp-4h]

	iassert(scrVarPub.fieldBuffer);

	for (pos = scrVarPub.fieldBuffer; *pos; pos += len + 3)
	{
		len = strlen(pos) + 1;
		if (!I_stricmp(name, pos))
		{
			posa = &pos[len];
			index = *(unsigned __int16*)posa;
			*type = posa[2];
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
		entryValue = &scrVarGlob.variableList[id + 32770];
		if ((entryValue->w.status & 0x60) != 0 && (entryValue->w.status & 0x1F) == 0xA)
		{
			for (threadId = *(unsigned __int16*)(entryValue->u.u.intValue + 8);
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
			timeId = FindObject(notifyListIda).u.stringValue;
			iassert(timeId);
			for (stackId = FindFirstSibling(timeId); stackId; stackId = FindNextSibling(stackId))
			{
				if (GetValueType(stackId) == 10)
				{
					for (threadIda = *(unsigned __int16*)(GetVariableValueAddress(stackId)->u.intValue + 8);
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
				name = scrVarGlob.variableList[notifyListEntry + 32770].w.status >> 8;
				iassert((name - SL_MAX_STRING_INDEX) < (1 << 16));
				if (names)
					names[count] = (unsigned __int16)Scr_GetThreadNotifyName((unsigned __int16)name);
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
					VariableInfoCompareCallBack = VariableInfoFileNameCompare;
					qsort(infoArray, num, 0x10u, VariableInfoFileNameCompare);
				}
				else if (functionSummary)
				{
					VariableInfoCompareCallBack = VariableInfoFunctionCompare;
					qsort(infoArray, num, 0x10u, VariableInfoFunctionCompare);
				}
				else
				{
					VariableInfoCompareCallBack = CompareThreadIndices;
					qsort(infoArray, num, 0x10u, CompareThreadIndices);
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
					qsort(infoArray, num, 0x10u, VariableInfoFileLineCompare);
				else
					qsort(infoArray, num, 0x10u, VariableInfoCountCompare);
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
				Z_VirtualFree(infoArray, 0);
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

	//iassert(entry->id == id);
	//iassert(!entry->u.prevSibling);
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
	const char* v3; // eax
	unsigned int index; // [esp+4h] [ebp-8h]
	unsigned int type; // [esp+8h] [ebp-4h]

	iassert(parentId);
	iassert(&scrVarGlob.variableList[parentId + 1]);

	type = scrVarGlob.variableList[parentId + 1].w.status & 0x1F;

	if (type <= 0x12)
		return GetVariableIndexInternal(parentId, name);

	if (type == 20)
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
		iassert(0);
		//v3 = va("cannot set field of %s", var_typename[type]);
		//Scr_Error(v3);
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
			name = (scrVarGlob.variableList[id + 32770].w.status >> 8) - 0x800000;
			if (scrVarGlob.variableList[id + 32770].w.status >> 8 == 0x800000)
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
			namea = scrVarGlob.variableList[ida + 32770].w.status >> 8;
			iassert(namea);
			if (namea != 98304 && namea != 98305)
			{
				iassert(name <= scrVarPub.canonicalStrCount);
				if (names)
					names[count] = namea;
				++count;
			}
		}
		break;
	case 0x15u:
		for (idb = FindFirstSibling(parentId); idb; idb = FindNextSibling(idb))
		{
			nameb = scrVarGlob.variableList[idb + 32770].w.status >> 8;
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
	return scrVarGlob.variableList[GetArrayVariableIndex(parentId, unsignedValue) + 32770].hash.id;
}

unsigned int  GetNewArrayVariable(unsigned int parentId, unsigned int unsignedValue)
{
	return scrVarGlob.variableList[GetNewArrayVariableIndex(parentId, unsignedValue) + 32770].hash.id;
}

unsigned int  GetVariable(unsigned int parentId, unsigned int unsignedValue)
{
	return scrVarGlob.variableList[GetVariableIndexInternal(parentId, unsignedValue) + 32770].hash.id;
}

unsigned int  GetNewVariable(unsigned int parentId, unsigned int unsignedValue)
{
	return scrVarGlob.variableList[GetNewVariableIndexInternal(parentId, unsignedValue) + 32770].hash.id;
}

unsigned int  GetObjectVariable(unsigned int parentId, unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[GetVariableIndexInternal(parentId, id + 0x10000) + 32770].hash.id;
}

unsigned int  GetNewObjectVariable(unsigned int parentId, unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[GetNewVariableIndexInternal(parentId, id + 0x10000) + 32770].hash.id;
}

unsigned int  GetNewObjectVariableReverse(unsigned int parentId, unsigned int id)
{
	iassert((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) == VAR_ARRAY);
	return scrVarGlob.variableList[GetNewVariableIndexReverseInternal(parentId, id + 0x10000) + 32770].hash.id;
}

void  RemoveVariable(unsigned int parentId, unsigned int unsignedValue)
{
	unsigned int index; // [esp+0h] [ebp-8h]
	unsigned int id; // [esp+4h] [ebp-4h]

	index = FindVariableIndexInternal(parentId, unsignedValue);
	iassert(index);
	id = scrVarGlob.variableList[index + 32770].hash.id;
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

	index = FindVariableIndexInternal(parentId, scrVarGlob.variableList[id + 32770].w.status >> 8);
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
		id = scrVarGlob.variableList[index + 32770].hash.id;
		iassert(!IsObject(&scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + id]));
		MakeVariableExternal(index, &scrVarGlob.variableList[parentId + 1]);
		FreeChildValue(parentId, id);
	}
}

void  RemoveVariableValue(unsigned int parentId, unsigned int index)
{
	unsigned int id; // [esp+0h] [ebp-4h]

	iassert(index);
	id = scrVarGlob.variableList[index + 32770].hash.id;
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
	iassert((entValue->w.type & VAR_MASK) == VAR_ENTITY);

	entValue = &scrVarGlob.variableList[entId + 1];
	iassert((entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT);
	fieldId = FindArrayVariable(g_classMap[entValue->w.status >> 8].id, fieldName);
	if (!fieldId
		|| !SetEntityFieldValue(
			entValue->w.status >> 8,
			entValue->u.o.u.size,
			scrVarGlob.variableList[fieldId + 32770].u.u.intValue,
			value))
	{
		entryValue = &scrVarGlob.variableList[GetNewVariable(entId, fieldName) + 32770];
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

	entryValue = &scrVarGlob.variableList[id + 32770];
	iassert(((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE) || !id);
	value.type = entryValue->w.status & 0x1F;
	value.u.intValue = entryValue->u.u.intValue;
	iassert(!IsObject(&value));

	AddRefToValue(value.type, value.u);

	return value;
}

void  Scr_EvalboolComplement(VariableValue* value)
{
	const char* v1; // eax
	int type; // [esp+0h] [ebp-4h]

	if (value->type == 6)
	{
		value->u.intValue = ~value->u.intValue;
	}
	else
	{
		type = value->type;
		RemoveRefToValue(type, value->u);
		value->type = 0;
		//v1 = va("~ cannot be applied to \"%s\"", var_typename[type]);
		//Scr_Error(v1);
		iassert(0); // KISAK
	}
}

void  Scr_CastBool(VariableValue* value)
{
	const char* v1; // eax
	int type; // [esp+4h] [ebp-4h]

	if (value->type == 6)
	{
		value->u.intValue = value->u.intValue != 0;
	}
	else if (value->type == 5)
	{
		value->type = 6;
		value->u.intValue = value->u.floatValue != 0.0;
	}
	else
	{
		type = value->type;
		RemoveRefToValue(type, value->u);
		value->type = 0;
		iassert(0); // KISAK
		//v1 = va("cannot cast %s to bool", var_typename[type]);
		//Scr_Error(v1);
	}
}

bool  Scr_CastString(VariableValue* value)
{
	const float* constTempVector; // [esp+4h] [ebp-4h]

	switch (value->type)
	{
	case 2:
		return 1;
	case 6:
		value->type = 2;
		value->u.stringValue = SL_GetStringForInt(value->u.intValue);
		return 1;
	case 5:
		value->type = 2;
		value->u.stringValue = SL_GetStringForFloat(value->u.floatValue);
		return 1;
	case 4:
		value->type = 2;
		constTempVector = (const float*)value->u.intValue;
		value->u.stringValue = SL_GetStringForVector(value->u.vectorValue);
		RemoveRefToVector(constTempVector);
		return 1;
	default:
		scrVarPub.error_message = va("cannot cast %s to string", var_typename[value->type]);
		RemoveRefToValue(value->type, value->u);
		value->type = 0;
		return 0;
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
	case 1:
		sa = (char*)var_typename[GetObjectType(value->u.intValue)];
		v2.prev = SL_GetString_(sa, 0, 15);
		goto LABEL_7;
	case 2:
	case 4:
	case 5:
	case 6:
		Scr_CastString(value);
		return;
	case 3:
		value->type = 2;
		return;
	case 0xB:
		intValue = (unsigned __int16)value->u.intValue;
		Anims = Scr_GetAnims(HIWORD(value->u.intValue));
		s = (char*)XAnimGetAnimDebugName(Anims, intValue);
		v2.prev = SL_GetString_(s, 0, 15);
		goto LABEL_7;
	default:
		v2.prev = SL_GetString_((char*)var_typename[value->type], 0, 15);
	LABEL_7:
		stringValue = v2.prev;
		RemoveRefToValue(value->type, value->u);
		value->type = 2;
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

	value->type = 0;
}

void  Scr_CastVector(VariableValue* value)
{
	const char* v1; // eax
	int type; // [esp+0h] [ebp-14h]
	int i; // [esp+4h] [ebp-10h]
	float vec[3]; // [esp+8h] [ebp-Ch] BYREF

	for (i = 2; i >= 0; --i)
	{
		type = value[i].type;
		if (type == 5)
		{
			vec[2 - i] = value[i].u.floatValue;
		}
		else
		{
			if (type != 6)
			{
				scrVarPub.error_index = i + 1;
				Scr_ClearVector(value);
				//v1 = va("type %s is not a float", var_typename[type]);
				//Scr_Error(v1);
				iassert(0); // KISAK
				return;
			}
			vec[2 - i] = (float)value[i].u.intValue;
		}
	}
	value->type = 4;
	value->u.intValue = (int)Scr_AllocVector(vec);
}

VariableUnion  Scr_EvalFieldObject(unsigned int tempVariable, VariableValue* value)
{
	const char* v3; // eax
	unsigned int type; // [esp+0h] [ebp-Ch]
	VariableValue tempValue; // [esp+4h] [ebp-8h] BYREF

	type = value->type;
	if (type == 1 && (type = scrVarGlob.variableList[value->u.intValue + 1].w.status & 0x1F, type < 0x15))
	{
		iassert(type >= FIRST_OBJECT);
		tempValue.type = 1;
		tempValue.u.intValue = value->u.intValue;
		SetVariableValue(tempVariable, &tempValue);
		return tempValue.u;
	}
	else
	{
		RemoveRefToValue(value->type, value->u);
		v3 = va("%s is not a field object", var_typename[type]);
		//Scr_Error(v3);
		iassert(0);
	}
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
	value1->type = 0;
	RemoveRefToValue(value2->type, value2->u);
	value2->type = 0;
	//Scr_Error(error_message);
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
	if (type == 5)
	{
		value1->type = 6;
		value1->u.intValue = value2->u.floatValue > (double)value1->u.floatValue;
	}
	else if (type == 6)
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
	if (type == 5)
	{
		value1->type = 6;
		value1->u.intValue = value2->u.floatValue < (double)value1->u.floatValue;
	}
	else if (type == 6)
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
			value1->type = 0;
			value2->type = 0;
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
			//Scr_Error("divide by 0");
			iassert(0);
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
			//Scr_Error("divide by 0");
			iassert(0); // KISAK
			return;
		}
		value1->u.floatValue = value1->u.floatValue / value2->u.floatValue;
		break;
	case 6:
		value1->type = 5;
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
			entId = FindObject(entnumId).u.stringValue;
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

	entryValue = &scrVarGlob.variableList[GetNewArrayVariable(classId, str) + 32770];
	entryValue->w.status &= 0xFFFFFFE0;
	entryValue->w.status |= 6u;
	entryValue->u.u.intValue = (unsigned __int16)offset;
	stra = SL_GetString_(name, 0, 16);
	iassert(!FindVariable(classId, str));
	fieldId = GetNewVariable(classId, stra);
	SL_RemoveRefToString(stra);
	entryValuea = &scrVarGlob.variableList[fieldId + 32770];
	entryValuea->w.status &= 0xFFFFFFE0;
	entryValuea->w.status |= 6u;
	entryValuea->u.u.intValue = (unsigned __int16)offset;
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
	entryValue = &scrVarGlob.variableList[id + 32770];
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
		entryValue = &scrVarGlob.variableList[scrVarPub.gameId + 32770];
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
		entryValue = &scrVarGlob.variableList[id + 32770];
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
				entryValue = &scrVarGlob.variableList[id + 32770];
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
			qsort(infoArray, num, 0x8Cu, ThreadInfoCompare);
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
				} while (i < num && !ThreadInfoCompare(pInfo, &infoArray[i]));
				varUsage = varUsage + info.varUsage;
				endonUsage = endonUsage + info.endonUsage;
				Com_Printf(23, "count: %d, var usage: %d, endon usage: %d\n", count, (int)info.varUsage, (int)info.endonUsage);
				Scr_PrintPrevCodePos(23, pInfo->pos[0], 0);
				for (ja = 1; ja < pInfo->posSize; ++ja)
				{
					Com_Printf(23, "called from:\n");
					Scr_PrintPrevCodePos(23, pInfo->pos[ja], 0);
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
						if ((scrVarGlob.variableList[entId + 32770].w.status & 0x1F) == 1)
						{
							ObjectUsage = Scr_GetObjectUsage(scrVarGlob.variableList[entId + 32770].u.u.stringValue);
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
				valuea->type = 0;
				SetEntityFieldValue(
					classnum,
					parentValue->u.o.u.size,
					scrVarGlob.variableList[fieldId + 32770].u.u.intValue,
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

	if (value->type == 1)
	{
		id.intValue = (int)value->u.intValue;
		entryValue = &scrVarGlob.variableList[value->u.intValue + 1];
		value->type = 6;
		if ((entryValue->w.status & 0x1F) == 0x15)
			v1.intValue = entryValue->u.o.u.size;
		else
			v1.intValue = 1;
		value->u = v1;
		RemoveRefToObject(id.stringValue);
	}
	else if (value->type == 2)
	{
		value->type = 6;
		stringValue.intValue = (int)value->u.intValue;
		value->u.intValue = strlen(SL_ConvertToString(value->u.intValue));
		SL_RemoveRefToString(stringValue.stringValue);
	}
	else
	{
		if (value->type == 10)
			MyAssertHandler(".\\script\\scr_variable.cpp", 3104, 0, "%s", "value->type != VAR_STACK");
		error_message = va("size cannot be applied to %s", var_typename[value->type]);
		RemoveRefToValue(value->type, value->u);
		value->type = 0;
		//Scr_Error(error_message);
		iassert(0);
	}
}

void  Scr_EvalboolNot(VariableValue* value)
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
		value1->type = 6;
		value1->u.intValue = 1;
		break;
	case 1:
		if (((scrVarGlob.variableList[value1->u.intValue + 32770].w.status & 0x1F) == 0x15
			|| (scrVarGlob.variableList[value2->u.intValue + 32770].w.status & 0x1F) == 0x15)
			&& !scrVarPub.evaluate)
		{
			goto LABEL_20;
		}
		value1->type = 6;
		tempInta = value1->u.intValue == value2->u.intValue;
		RemoveRefToObject(value1->u.intValue);
		RemoveRefToObject(value2->u.intValue);
		value1->u.intValue = tempInta;
		break;
	case 2:
	case 3:
		value1->type = 6;
		tempInt = value1->u.intValue == value2->u.intValue;
		SL_RemoveRefToString(value1->u.intValue);
		SL_RemoveRefToString(value2->u.intValue);
		value1->u.intValue = tempInt;
		break;
	case 4:
		value1->type = 6;
		v2 = *(float*)value2->u.intValue == *(float*)value1->u.intValue
			&& *(float*)(value2->u.intValue + 4) == *(float*)(value1->u.intValue + 4)
			&& *(float*)(value2->u.intValue + 8) == *(float*)(value1->u.intValue + 8);
		RemoveRefToVector(value1->u.vectorValue);
		RemoveRefToVector(value2->u.vectorValue);
		value1->u.intValue = v2;
		break;
	case 5:
		value1->type = 6;
		v4 = value1->u.floatValue - value2->u.floatValue;
		v3 = fabs(v4);
		value1->u.intValue = v3 < 0.0000009999999974752427;
		break;
	case 6:
		value1->u.intValue = value1->u.intValue == value2->u.intValue;
		break;
	case 9:
		value1->type = 6;
		value1->u.intValue = value1->u.intValue == value2->u.intValue;
		break;
	case 0xB:
		value1->type = 6;
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

void  Scr_EvalBinaryOperator(int op, VariableValue* value1, VariableValue* value2)
{
	switch (op)
	{
	case 'f':
		Scr_EvalOr(value1, value2);
		break;
	case 'g':
		Scr_EvalExOr(value1, value2);
		break;
	case 'h':
		Scr_EvalAnd(value1, value2);
		break;
	case 'i':
		Scr_EvalEquality(value1, value2);
		break;
	case 'j':
		Scr_EvalInequality(value1, value2);
		break;
	case 'k':
		Scr_EvalLess(value1, value2);
		break;
	case 'l':
		Scr_EvalGreater(value1, value2);
		break;
	case 'm':
		Scr_EvalLessEqual(value1, value2);
		break;
	case 'n':
		Scr_EvalGreaterEqual(value1, value2);
		break;
	case 'o':
		Scr_EvalShiftLeft(value1, value2);
		break;
	case 'p':
		Scr_EvalShiftRight(value1, value2);
		break;
	case 'q':
		Scr_EvalPlus(value1, value2);
		break;
	case 'r':
		Scr_EvalMinus(value1, value2);
		break;
	case 's':
		Scr_EvalMultiply(value1, value2);
		break;
	case 't':
		Scr_EvalDivide(value1, value2);
		break;
	case 'u':
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
	const char* v2; // eax
	const char* v3; // eax
	const char* v4; // eax
	const char* v5; // eax
	const char* v6; // eax
	unsigned int ArrayIndex; // eax
	const char* v8; // eax
	int type; // [esp+10h] [ebp-18h]
	char c[4]; // [esp+1Ch] [ebp-Ch] BYREF
	const char* s; // [esp+20h] [ebp-8h]
	VariableValueInternal* entryValue; // [esp+24h] [ebp-4h]

	iassert(value != index);
	type = value->type;
	switch (type)
	{
	case 1:
		entryValue = &scrVarGlob.variableList[value->u.intValue + 1];
		iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
		iassert(IsObject(entryValue));
		if ((entryValue->w.status & 0x1F) == 0x15)
		{
			ArrayIndex = Scr_FindArrayIndex(value->u.intValue, index);
			*index = Scr_EvalVariable(ArrayIndex);
			RemoveRefToObject(value->u.intValue);
		}
		else
		{
			scrVarPub.error_index = 1;
			v6 = va("%s is not an array", var_typename[entryValue->w.status & 0x1F]);
			//Scr_Error(v6);
			iassert(0);
		}
		break;
	case 2:
		if (index->type == 6)
		{
			if (index->u.intValue < 0
				|| (s = SL_ConvertToString(value->u.intValue), index->u.intValue >= (signed int)strlen(s)))
			{
				v2 = va("string index %d out of range", index->u.intValue);
				//Scr_Error(v2);
				iassert(0);
			}
			else
			{
				index->type = 2;
				c[0] = s[index->u.intValue];
				c[1] = 0;
				index->u.stringValue = SL_GetStringOfSize(c, 0, 2u, 15).prev;
				SL_RemoveRefToString(value->u.intValue);
			}
		}
		else
		{
			v3 = va("%s is not a string index", var_typename[index->type]);
			//Scr_Error(v3);
			iassert(0);
		}
		break;
	case 4:
		if (index->type == 6)
		{
			if (index->u.intValue >= 3u)
			{
				v4 = va("vector index %d out of range", index->u.intValue);
				//Scr_Error(v4);
				iassert(0);
			}
			else
			{
				index->type = 5;
				index->u.floatValue = *(float*)(value->u.intValue + 4 * index->u.intValue);
				RemoveRefToVector(value->u.vectorValue);
			}
		}
		else
		{
			v5 = va("%s is not a vector index", var_typename[index->type]);
			//Scr_Error(v5);
			iassert(0);
		}
		break;
	default:
		if (value->type == 10)
			MyAssertHandler(".\\script\\scr_variable.cpp", 4557, 0, "%s", "value->type != VAR_STACK");
		scrVarPub.error_index = 1;
		v8 = va("%s is not an array, string, or vector", var_typename[value->type]);
		//Scr_Error(v8);
		iassert(0);
		break;
	}
}

VariableUnion Scr_EvalArrayRef(unsigned int parentId)
{
	const char* v2; // eax
	const char* v3; // eax
	VariableValueInternal* parentValue; // [esp+Ch] [ebp-1Ch]
	VariableValueInternal* entValue; // [esp+10h] [ebp-18h]
	VariableValue varValue; // [esp+14h] [ebp-14h]
	VariableValueInternal* entryValue; // [esp+1Ch] [ebp-Ch]
	unsigned int fieldId; // [esp+20h] [ebp-8h]
	VariableUnion id; // [esp+24h] [ebp-4h]

	if (parentId)
	{
		parentValue = &scrVarGlob.variableList[parentId + 32770];
		iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
		varValue.type = parentValue->w.status & 0x1F;
		if (varValue.type)
		{
			varValue.u.intValue = parentValue->u.u.intValue;
		add_array:
			if (varValue.type == 1)
			{
				entryValue = &scrVarGlob.variableList[varValue.u.intValue + 1];
				iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
				iassert(IsObject(entryValue));
				if ((entryValue->w.status & 0x1F) == 0x15)
				{
					if (entryValue->u.next)
					{
						id.intValue = (int)varValue.u;
						RemoveRefToObject(varValue.u.stringValue);
						varValue.u.intValue = Scr_AllocArray();
						CopyArray(id.stringValue, varValue.u.stringValue);
						iassert(parentValue);
						parentValue->u.u.intValue = varValue.u.intValue;
					}
					return varValue.u;
				}
				else
				{
					scrVarPub.error_index = 1;
					v3 = va("%s is not an array", var_typename[entryValue->w.status & 0x1F]);
					//Scr_Error(v3);
					iassert(0);
					return 0;
				}
			}
			else
			{
				iassert(varValue.type != VAR_STACK);
				scrVarPub.error_index = 1;
				if (varValue.type == 2)
				{
					//Scr_Error("string characters cannot be individually changed");
					iassert(0);
					return 0;
				}
				else
				{
					if (varValue.type == 4)
					{
						//Scr_Error("vector components cannot be individually changed");
						iassert(0);
					}
					else
					{
						v2 = va("%s is not an array", var_typename[varValue.type]);
						//Scr_Error(v2);
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
			varValue = GetEntityFieldValue(
				entValue->w.status >> 8,
				entValue->u.o.u.size,
				scrVarGlob.variableList[fieldId + 32770].u.u.intValue);
			if (varValue.type)
			{
				if (varValue.type == 1 && !scrVarGlob.variableList[varValue.u.intValue + 1].u.next)
				{
					RemoveRefToValue(1, varValue.u);
					scrVarPub.error_index = 1;
					iassert(0);
					//Scr_Error("read-only array cannot be changed");
					//return 0;
				}
				RemoveRefToValue(varValue.type, varValue.u);
				iassert((varValue.type != VAR_POINTER) || !scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + varValue.u.pointerValue].u.o.refCount);
				parentValue = 0;
				goto add_array;
			}
		}
		parentValue = &scrVarGlob.variableList[GetNewVariable(scrVarPub.entId, scrVarPub.entFieldName) + 32770];
	}

	iassert(!(parentValue->w.type& VAR_MASK));

	parentValue->w.status |= 1u;
	parentValue->u.u.intValue = Scr_AllocArray();
	return parentValue->u.u;
}

void  ClearArray(unsigned int parentId, VariableValue* value)
{
	const char* v2; // eax
	const char* v3; // eax
	const char* v4; // eax
	const char* v5; // eax
	VariableValueInternal* parentValue; // [esp+8h] [ebp-1Ch]
	VariableValueInternal* entValue; // [esp+Ch] [ebp-18h]
	VariableValue varValue; // [esp+10h] [ebp-14h]
	VariableValueInternal* entryValue; // [esp+18h] [ebp-Ch]
	unsigned int fieldId; // [esp+1Ch] [ebp-8h]
	VariableUnion id; // [esp+20h] [ebp-4h]

	if (parentId)
	{
		parentValue = &scrVarGlob.variableList[parentId + 32770];
		iassert((parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
		varValue.type = parentValue->w.status & 0x1F;
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
				scrVarGlob.variableList[fieldId + 32770].u.u.intValue),
				!varValue.type))
		{
			varValue.type = 0;
		error_0:
			scrVarPub.error_index = 1;
			v2 = va("%s is not an array", var_typename[varValue.type]);
			Scr_Error(v2);
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
	iassert(varValue.type != VAR_STACK);
	entryValue = &scrVarGlob.variableList[varValue.u.intValue + 1];
	iassert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE);
	iassert(IsObject(entryValue));
	if ((entryValue->w.status & 0x1F) == 0x15)
	{
		if (entryValue->u.next)
		{
			id.intValue = (int)varValue.u;
			RemoveRefToObject(varValue.u.stringValue);
			varValue.u.intValue = Scr_AllocArray();
			CopyArray(id.stringValue, varValue.u.stringValue);
			if (!parentValue)
				MyAssertHandler(".\\script\\scr_variable.cpp", 4753, 0, "%s", "parentValue");
			parentValue->u.u.intValue = varValue.u.intValue;
		}
		if (value->type == 6)
		{
			if (IsValidArrayIndex(value->u.intValue))
			{
				SafeRemoveArrayVariable(varValue.u.stringValue, value->u.intValue);
			}
			else
			{
				v4 = va("array index %d out of range", value->u.intValue);
				Scr_Error(v4);
			}
		}
		else if (value->type == 2)
		{
			SL_RemoveRefToString(value->u.intValue);
			SafeRemoveVariable(varValue.u.stringValue, value->u.intValue);
		}
		else
		{
			v5 = va("%s is not an array index", var_typename[value->type]);
			Scr_Error(v5);
		}
	}
	else
	{
		scrVarPub.error_index = 1;
		v3 = va("%s is not an array", var_typename[entryValue->w.status & 0x1F]);
		Scr_Error(v3);
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
	unsigned int value; // [esp+14h] [ebp-Ch]
	unsigned int id; // [esp+1Ch] [ebp-4h]

	iassert(parentId);
	iassert(IsObject(&scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId]));
	iassert(((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) >= FIRST_OBJECT 
		&& (scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) < FIRST_NONFIELD_OBJECT) 
		|| ((scrVarGlob.variableList[VARIABLELIST_PARENT_BEGIN + parentId].w.type & VAR_MASK) >= FIRST_DEAD_OBJECT));

	id = FindVariable(parentId, name);
	if (id)
		return Scr_EvalVariable(id);
	if ((scrVarGlob.variableList[parentId + 1].w.status & 0x1F) == 0x14)
		return Scr_EvalVariableEntityField(parentId, name);
	return (VariableValue)value;
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
		for (selfNameId = FindObject(id).u.stringValue; ; RemoveObjectVariable(selfNameId, (unsigned __int16)name))
		{
			notifyListEntry = FindFirstSibling(selfNameId);
			if (!notifyListEntry)
				break;

			iassert((scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + notifyListEntry].w.type & VAR_MASK) == VAR_POINTER);
			name = scrVarGlob.variableList[notifyListEntry + 32770].w.status >> 8;
			iassert((name - SL_MAX_STRING_INDEX) < (1 << 16));

			ObjectVariable_DONE = FindObjectVariable(selfNameId, (unsigned __int16)name);
			VariableValueAddress_DONE = GetVariableValueAddress(ObjectVariable_DONE);
			VM_CancelNotify(VariableValueAddress_DONE->u.intValue, (unsigned __int16)name);
			Scr_KillEndonThread((unsigned __int16)name);
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
		Scr_CheckLeakRange(0x8002u, 0x18000u);
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

int  VariableInfoFunctionCompare(void* p_info1, void* p_info2)
{
	const char* functionName2; // [esp+0h] [ebp-Ch]
	const char* functionName1; // [esp+4h] [ebp-8h]
	int fileNameCompare; // [esp+8h] [ebp-4h]

	_DWORD* info1 = (_DWORD *)p_info1;
	_DWORD* info2 = (_DWORD *)p_info2;

	fileNameCompare = VariableInfoFileNameCompare(info1, info2);
	if (fileNameCompare)
		return fileNameCompare;
	functionName1 = (const char*)info1[2];
	functionName2 = (const char*)info2[2];
	if (!functionName1)
		return 1;
	if (functionName2)
		return I_stricmp(functionName1, functionName2);
	return -1;
}

int VariableInfoCountCompare(_DWORD* info1, _DWORD* info2)
{
	return info1[3] - info2[3];
}

int __cdecl CompareThreadIndices(_DWORD* arg1, _DWORD* arg2)
{
	return *arg1 - *arg2;
}

int __cdecl VariableInfoFileLineCompare(_DWORD* info1, _DWORD* info2)
{
	int fileCompare; // [esp+0h] [ebp-4h]

	fileCompare = VariableInfoFileNameCompare(info1, info2);
	if (fileCompare)
		return fileCompare;
	else
		return CompareThreadIndices(info1, info2);
}

unsigned int  FindVariableIndexInternal2(unsigned int name, unsigned int index)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-14h]
	unsigned int newIndex; // [esp+4h] [ebp-10h]
	VariableValueInternal* entryValue; // [esp+8h] [ebp-Ch]
	VariableValueInternal* newEntryValue; // [esp+Ch] [ebp-8h]
	VariableValueInternal* newEntry; // [esp+10h] [ebp-4h]

	iassert(!(name & ~VAR_NAME_LOW_MASK));
	if (index >= 0xFFFE)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			930,
			0,
			"index doesn't index VARIABLELIST_CHILD_SIZE\n\t%i not in [0, %i)",
			index,
			65534);
	entry = &scrVarGlob.variableList[index + 32770];
	if (entry->hash.id >= 0xFFFEu)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			934,
			0,
			"entry->id doesn't index VARIABLELIST_CHILD_SIZE\n\t%i not in [0, %i)",
			entry->hash.id,
			65534);
	entryValue = &scrVarGlob.variableList[entry->hash.id + 32770];
	if ((entryValue->w.status & 0x60) == 0x40)
	{
		iassert(!IsObject(entryValue));
		if (entryValue->w.status >> 8 == name)
			return index;
		newIndex = entryValue->v.next;
		for (newEntry = &scrVarGlob.variableList[newIndex + 32770];
			newEntry != entry;
			newEntry = &scrVarGlob.variableList[newIndex + 32770])
		{
			newEntryValue = &scrVarGlob.variableList[newEntry->hash.id + 32770];
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
	if ((entryValue->w.status & 0x60) != 0)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1859,
			0,
			"%s",
			"(entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE");
	next = entryValue->u.next;
	if (entry != entryValue && (entry->w.status & 0x60) == 0)
	{
		newIndex = entry->v.next;
		if (newIndex == index)
			MyAssertHandler(".\\script\\scr_variable.cpp", 1867, 0, "%s", "newIndex != index");
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
	if (!entry->hash.id || entry->hash.id >= 0x8000u)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1883,
			0,
			"%s",
			"entry->hash.id > 0 && entry->hash.id < VARIABLELIST_PARENT_SIZE");
	++scrVarPub.totalObjectRefCount;
	if (scrVarDebugPub)
	{
		if (scrVarDebugPub->leakCount[entry->hash.id + 1])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1888,
				0,
				"%s",
				"!scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + entry->hash.id]");
		++scrVarDebugPub->leakCount[entry->hash.id + 1];
	}
	++scrVarPub.numScriptObjects;
	if (!scrVarPub.varUsagePos)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1895, 0, "%s", "scrVarPub.varUsagePos");
	if (scrVarDebugPub)
	{
		if (scrVarDebugPub->varUsage[entry->hash.id + 1])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1898,
				0,
				"%s",
				"!scrVarDebugPub->varUsage[VARIABLELIST_PARENT_BEGIN + entry->hash.id]");
		scrVarDebugPub->varUsage[entry->hash.id + 1] = scrVarPub.varUsagePos;
	}
	return entry->hash.id;
}

void  FreeVariable(unsigned int id)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-Ch]
	VariableValueInternal* entryValue; // [esp+4h] [ebp-8h]
	unsigned int index; // [esp+8h] [ebp-4h]

	if (!id || id >= 0x8000)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1914, 0, "%s", "id > 0 && id < VARIABLELIST_PARENT_SIZE");
	--scrVarPub.totalObjectRefCount;
	if (scrVarDebugPub)
	{
		if (!scrVarDebugPub->leakCount[id + 1])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1919,
				0,
				"%s",
				"scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]");
		if (--scrVarDebugPub->leakCount[id + 1])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1921,
				0,
				"%s",
				"!scrVarDebugPub->leakCount[VARIABLELIST_PARENT_BEGIN + id]");
	}
	--scrVarPub.numScriptObjects;
	if (scrVarDebugPub)
	{
		if (!scrVarDebugPub->varUsage[id + 1])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1929,
				0,
				"%s",
				"scrVarDebugPub->varUsage[VARIABLELIST_PARENT_BEGIN + id]");
		scrVarDebugPub->varUsage[id + 1] = 0;
	}
	entryValue = &scrVarGlob.variableList[id + 1];
	if ((entryValue->w.status & 0x60) != 0x60)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1936,
			0,
			"%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
			"((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
			entryValue->w.status & 0x60);
	index = entryValue->v.next;
	entry = &scrVarGlob.variableList[index + 1];
	if (entry->hash.id != id)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1940, 0, "%s", "entry->id == id");
	if (entry->hash.u.prev)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1941, 0, "%s", "!entry->u.prevSibling");
	if (entryValue->nextSibling)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1942, 0, "%s", "!entryValue->nextSibling");
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
	entryValue->w.status = 96;
	if ((entryValue->w.status & 0x1F) != 0)
		MyAssertHandler(".\\script\\scr_variable.cpp", 2048, 0, "%s", "!(entryValue->w.type & VAR_MASK)");
	entryValue->w.status |= 0x14u;
	if ((entryValue->w.status & 0xFFFFFF00) != 0)
		MyAssertHandler(".\\script\\scr_variable.cpp", 2050, 0, "%s", "!(entryValue->w.classnum & VAR_NAME_HIGH_MASK)");
	entryValue->w.status |= classnum << 8;
	entryValue->u.next = 0;
	entryValue->u.o.u.size = entnum;
	return id;
}

float* Scr_AllocVector(void)
{
	char* v0; // eax
	float* result; // [esp+4h] [ebp-4h]

	v0 = (char*)MT_Alloc(16, 2);
	result = (float*)(v0 + 4);
	*(_DWORD*)v0 = 0;
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
	const char* v3; // eax
	const char* v4; // eax
	unsigned int id; // [esp+0h] [ebp-4h]

	if (index->type == 6)
	{
		if (IsValidArrayIndex(index->u.intValue))
		{
			return FindArrayVariable(parentId, index->u.intValue);
		}
		else
		{
			v3 = va("array index %d out of range", index->u.intValue);
			Scr_Error(v3);
			AddRefToObject(parentId);
			return 0;
		}
	}
	else if (index->type == 2)
	{
		id = FindVariable(parentId, index->u.intValue);
		SL_RemoveRefToString(index->u.intValue);
		return id;
	}
	else
	{
		v4 = va("%s is not an array index", var_typename[index->type]);
		Scr_Error(v4);
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
	if ((parentValue->w.status & 0x60) == 0)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			4924,
			0,
			"%s",
			"(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE");
	if (!IsObject(parentValue))
		MyAssertHandler(".\\script\\scr_variable.cpp", 4925, 0, "%s", "IsObject( parentValue )");
	if ((parentValue->w.status & 0x1F) != 0x15)
		return 0.0;
	return (float)(Scr_GetObjectUsage(u.stringValue) / ((double)parentValue->u.next + 1.0));

}

float  Scr_GetEntryUsage(VariableValueInternal* entryValue)
{
	return (float)(Scr_GetEntryUsage(entryValue->w.status & 0x1F, entryValue->u.u) + 1.0);
}

float  Scr_GetObjectUsage(unsigned int parentId)
{
	float usage; // [esp+4h] [ebp-8h]
	unsigned int id; // [esp+8h] [ebp-4h]

	if ((scrVarGlob.variableList[parentId + 1].w.status & 0x60) == 0)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			4966,
			0,
			"%s",
			"(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE");
	if (!IsObject(&scrVarGlob.variableList[parentId + 1]))
		MyAssertHandler(".\\script\\scr_variable.cpp", 4967, 0, "%s", "IsObject( parentValue )");
	usage = 1.0;
	for (id = FindFirstSibling(parentId); id; id = FindNextSibling(id))
		usage = Scr_GetEntryUsage(&scrVarGlob.variableList[id + 32770]) + usage;
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
			type = 5;
		}
		else if (!strcmp(token, "int"))
		{
			type = 6;
		}
		else if (!strcmp(token, "string"))
		{
			type = 2;
		}
		else
		{
			if (strcmp(token, "vector"))
			{
				v2 = va("Unknown type %s in %s", token, filename);
				Com_Error(ERR_DROP, v2);
				return;
			}
			type = 4;
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
	scrVarPub.fieldBuffer = NULL;
	sprintf(filename, "%s/%s.%s", path, "keys", extension);
	Scr_AddFieldsForFile(filename);
	targetPos = TempMalloc(1);
	*targetPos = 0;
}

unsigned int  GetNewVariableIndexInternal3(unsigned int parentId, unsigned int name, unsigned int index)
{
	const char* v3; // eax
	const char* v4; // eax
	VariableValueInternal* parentValue; // [esp+8h] [ebp-40h]
	scrVarGlob_t* entry; // [esp+Ch] [ebp-3Ch]
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
	scrVarGlob_t* newEntryValue; // [esp+30h] [ebp-18h]
	int type; // [esp+34h] [ebp-14h]
	VariableValueInternal* newEntrya; // [esp+38h] [ebp-10h]
	VariableValueInternal* newEntry; // [esp+38h] [ebp-10h]
	VariableValue value; // [esp+3Ch] [ebp-Ch]
	unsigned __int16 id; // [esp+44h] [ebp-4h]

	if ((name & 0xFF000000) != 0)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1021, 0, "%s", "!(name & ~VAR_NAME_LOW_MASK)");
	entry = (scrVarGlob_t*)&scrVarGlob.variableList[index + 32770];
	entryValue = &scrVarGlob.variableList[entry->variableList[0].hash.id + 32770];
	type = entryValue->w.status & 0x60;
	if (type)
	{
		if (type == 64)
		{
			if ((entry->variableList[0].w.status & 0x60) != 0)
			{
				index = scrVarGlob.variableList[32770].u.next;
				if (!scrVarGlob.variableList[32770].u.next)
					Scr_TerminalError("exceeded maximum number of script variables");
				if (!index || index >= 0xFFFE)
					MyAssertHandler(
						".\\script\\scr_variable.cpp",
						1092,
						0,
						"%s\n\t(index) = %i",
						"(index > 0 && index < VARIABLELIST_CHILD_SIZE)",
						index);
				entry = (scrVarGlob_t*)&scrVarGlob.variableList[index + 32770];
				newEntryValue = (scrVarGlob_t*)&scrVarGlob.variableList[entry->variableList[0].hash.id + 32770];
				if ((newEntryValue->variableList[0].w.status & 0x60) != 0)
					MyAssertHandler(
						".\\script\\scr_variable.cpp",
						1096,
						0,
						"%s",
						"(newEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE");
				scrVarGlob.variableList[32770].u.next = newEntryValue->variableList[0].u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[32770].u.next + 32770].hash.u.prev = 0;
				newEntryValue->variableList[0].w.status = 32;
				newEntryValue->variableList[0].v.next = entryValue->v.next;
				entryValue->v.next = index;
			}
			else
			{
				newIndexb = entry->variableList[0].v.next;
				newEntrya = &scrVarGlob.variableList[newIndexb + 32770];
				newEntryValue = (scrVarGlob_t*)&scrVarGlob.variableList[index + 32770];
				preva = newEntrya->hash.u.prev;
				nexta = entry->variableList[0].u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[preva + 32770].hash.id + 32770].u.next = nexta;
				scrVarGlob.variableList[nexta + 32770].hash.u.prev = preva;
				newEntrya->hash.id = entry->variableList[0].hash.id;
				entry->variableList[0].hash.id = index;
				newEntrya->hash.u.prev = entry->variableList[0].hash.u.prev;
				scrVarGlob.variableList[scrVarGlob.variableList[newEntrya->hash.u.prev + 32770].hash.id + 32770].nextSibling = newIndexb;
				scrVarGlob.variableList[entryValue->nextSibling + 32770].hash.u.prev = newIndexb;
				entryValue->w.status &= 0xFFFFFF9F;
				entryValue->w.status |= 0x20u;
				entry->variableList[0].w.status = 64;
			}
		}
		else
		{
			if (type != 32 && type != 96)
				MyAssertHandler(
					".\\script\\scr_variable.cpp",
					1110,
					0,
					"%s",
					"type == VAR_STAT_MOVABLE || type == VAR_STAT_EXTERNAL");
			if ((entry->variableList[0].w.status & 0x60) != 0)
			{
				newIndexa = scrVarGlob.variableList[32770].u.next;
				if (!scrVarGlob.variableList[32770].u.next)
					Scr_TerminalError("exceeded maximum number of script variables");
				newEntry = &scrVarGlob.variableList[newIndexa + 32770];
				newEntryValue = (scrVarGlob_t*)&scrVarGlob.variableList[newEntry->hash.id + 32770];
				if ((newEntryValue->variableList[0].w.status & 0x60) != 0)
					MyAssertHandler(
						".\\script\\scr_variable.cpp",
						1136,
						0,
						"%s",
						"(newEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_FREE");
				scrVarGlob.variableList[32770].u.next = newEntryValue->variableList[0].u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[32770].u.next + 32770].hash.u.prev = 0;
			}
			else
			{
				if (entry == (scrVarGlob_t*)entryValue)
					MyAssertHandler(".\\script\\scr_variable.cpp", 1115, 0, "%s", "entry != entryValue");
				newIndexa = entry->variableList[0].v.next;
				newEntry = &scrVarGlob.variableList[newIndexa + 32770];
				newEntryValue = (scrVarGlob_t*)&scrVarGlob.variableList[index + 32770];
				prevb = newEntry->hash.u.prev;
				nextb = entry->variableList[0].u.next;
				scrVarGlob.variableList[scrVarGlob.variableList[prevb + 32770].hash.id + 32770].u.next = nextb;
				scrVarGlob.variableList[nextb + 32770].hash.u.prev = prevb;
			}
			if (entry->variableList[0].hash.u.prev)
				scrVarGlob.variableList[scrVarGlob.variableList[entry->variableList[0].hash.u.prev + 32770].hash.id + 32770].nextSibling = newIndexa;
			if (entryValue->nextSibling)
				scrVarGlob.variableList[entryValue->nextSibling + 32770].hash.u.prev = newIndexa;
			if (type == 32)
			{
				prevId = scrVarGlob.variableList[entryValue->v.next + 32770].hash.id;
				if ((scrVarGlob.variableList[prevId + 32770].w.status & 0x60) != 0x20
					&& (scrVarGlob.variableList[prevId + 32770].w.status & 0x60) != 0x40)
				{
					MyAssertHandler(
						".\\script\\scr_variable.cpp",
						1164,
						0,
						"%s",
						"(scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE ||"
						" (scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_HEAD");
				}
				while (scrVarGlob.variableList[prevId + 32770].v.next != index)
				{
					prevId = scrVarGlob.variableList[scrVarGlob.variableList[prevId + 32770].v.next + 32770].hash.id;
					if ((scrVarGlob.variableList[prevId + 32770].w.status & 0x60) != 0x20
						&& (scrVarGlob.variableList[prevId + 32770].w.status & 0x60) != 0x40)
					{
						MyAssertHandler(
							".\\script\\scr_variable.cpp",
							1169,
							0,
							"%s",
							"(scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE "
							"|| (scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prevId].w.status & VAR_STAT_MASK) == VAR_STAT_HEAD");
					}
				}
				scrVarGlob.variableList[prevId + 32770].v.next = newIndexa;
			}
			else
			{
				if (type != 96)
					MyAssertHandler(".\\script\\scr_variable.cpp", 1177, 0, "%s", "type == VAR_STAT_EXTERNAL");
				entryValue->v.next = newIndexa;
			}
			newEntry->hash.u.prev = entry->variableList[0].hash.u.prev;
			id = newEntry->hash.id;
			newEntry->hash.id = entry->variableList[0].hash.id;
			entry->variableList[0].hash.id = id;
			newEntryValue->variableList[0].w.status = 64;
			newEntryValue->variableList[0].v.next = index;
		}
	}
	else
	{
		newIndex = entry->variableList[0].v.next;
		next = entryValue->u.next;
		if (newIndex == entry->variableList[0].hash.id || (entry->variableList[0].w.status & 0x60) != 0)
		{
			newEntryValue = (scrVarGlob_t*)&scrVarGlob.variableList[scrVarGlob.variableList[index + 32770].hash.id + 32770];
		}
		else
		{
			scrVarGlob.variableList[newIndex + 32770].hash.id = entry->variableList[0].hash.id;
			entry->variableList[0].hash.id = index;
			entryValue->v.next = newIndex;
			entryValue->u.next = entry->variableList[0].u.next;
			newEntryValue = (scrVarGlob_t*)&scrVarGlob.variableList[index + 32770];
		}
		prev = entry->variableList[0].hash.u.prev;
		if (scrVarGlob.variableList[prev + 32770].hash.id
			&& (scrVarGlob.variableList[scrVarGlob.variableList[prev + 32770].hash.id + 32770].w.status & 0x60) != 0)
		{
			v3 = va(
				"%d, %d, %d",
				prev,
				scrVarGlob.variableList[prev + 32770].hash.id,
				scrVarGlob.variableList[scrVarGlob.variableList[prev + 32770].hash.id + 32770].w.status & 0x60);
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1049,
				0,
				"%s\n\t%s",
				"!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prev].hash.id || (scrVarGlob.variableList[VARIABLELIST_CHILD"
				"_BEGIN + scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + prev].hash.id].w.status & VAR_STAT_MASK) == VAR_STAT_FREE",
				v3);
		}
		if (scrVarGlob.variableList[next + 32770].hash.id
			&& (scrVarGlob.variableList[scrVarGlob.variableList[next + 32770].hash.id + 32770].w.status & 0x60) != 0)
		{
			v4 = va(
				"%d, %d, %d",
				next,
				scrVarGlob.variableList[next + 32770].hash.id,
				scrVarGlob.variableList[scrVarGlob.variableList[next + 32770].hash.id + 32770].w.status & 0x60);
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1050,
				0,
				"%s\n\t%s",
				"!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + next].hash.id || (scrVarGlob.variableList[VARIABLELIST_CHILD"
				"_BEGIN + scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + next].hash.id].w.status & VAR_STAT_MASK) == VAR_STAT_FREE",
				v4);
		}
		scrVarGlob.variableList[scrVarGlob.variableList[prev + 32770].hash.id + 32770].u.next = next;
		scrVarGlob.variableList[next + 32770].hash.u.prev = prev;
		newEntryValue->variableList[0].w.status = 64;
		newEntryValue->variableList[0].v.next = index;
	}
	if (entry != (scrVarGlob_t*)&scrVarGlob.variableList[index + 32770])
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1195,
			0,
			"%s",
			"entry == &scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + index]");
	if (newEntryValue != (scrVarGlob_t*)&scrVarGlob.variableList[entry->variableList[0].hash.id + 32770])
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1196,
			0,
			"%s",
			"newEntryValue == &scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + entry->hash.id]");
	if ((newEntryValue->variableList[0].w.status & 0x1F) != 0)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1197, 0, "%s", "(newEntryValue->w.type & VAR_MASK) == VAR_UNDEFINED");
	newEntryValue->variableList[0].w.status = (unsigned __int8)newEntryValue->variableList[0].w.status;
	newEntryValue->variableList[0].w.status |= name << 8;
	if (!entry->variableList[0].hash.id || entry->variableList[0].hash.id >= 0xFFFEu)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1202,
			0,
			"%s\n\t(entry->hash.id) = %i",
			"(entry->hash.id > 0 && entry->hash.id < VARIABLELIST_CHILD_SIZE)",
			entry->variableList[0].hash.id);
	++scrVarPub.totalObjectRefCount;
	if (scrVarDebugPub)
	{
		if (scrVarDebugPub->leakCount[entry->variableList[0].hash.id + 32770])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1207,
				0,
				"%s",
				"!scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + entry->hash.id]");
		++scrVarDebugPub->leakCount[entry->variableList[0].hash.id + 32770];
	}
	++scrVarPub.numScriptValues;
	if (!scrVarPub.varUsagePos)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1214, 0, "%s", "scrVarPub.varUsagePos");
	if (scrVarDebugPub)
	{
		if (scrVarDebugPub->varUsage[entry->variableList[0].hash.id + 32770])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1217,
				0,
				"%s",
				"!scrVarDebugPub->varUsage[VARIABLELIST_CHILD_BEGIN + entry->hash.id]");
		scrVarDebugPub->varUsage[entry->variableList[0].hash.id + 32770] = scrVarPub.varUsagePos;
	}
	parentValue = &scrVarGlob.variableList[parentId + 1];
	if ((parentValue->w.status & 0x60) != 0x60)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1223,
			0,
			"%s",
			"(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL");
	if (!IsObject(parentValue))
		MyAssertHandler(".\\script\\scr_variable.cpp", 1224, 0, "%s", "IsObject( parentValue )");
	if ((parentValue->w.status & 0x1F) == 0x15)
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
	if ((parentValue->w.status & 0x60) != 0x60)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1253,
			0,
			"%s\n\t(parentValue->w.status & VAR_STAT_MASK) = %i",
			"((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
			parentValue->w.status & 0x60);
	id = scrVarGlob.variableList[indexa + 32770].hash.id;
	siblingId = parentValue->nextSibling;
	if (parentValue->nextSibling)
	{
		if (IsObject(&scrVarGlob.variableList[siblingId + 32770]))
			MyAssertHandler(".\\script\\scr_variable.cpp", 1262, 0, "%s", "!IsObject( siblingValue )");
		siblingIndex = FindVariableIndexInternal(parentId, scrVarGlob.variableList[siblingId + 32770].w.status >> 8);
		if (!siblingIndex)
			MyAssertHandler(".\\script\\scr_variable.cpp", 1265, 0, "%s", "siblingIndex");
		scrVarGlob.variableList[siblingIndex + 32770].hash.u.prev = indexa;
	}
	else
	{
		*(word*)&siblingIndex = 0; // KISAK ADD CAST
		scrVarGlob.variableList[parentValue->v.next + 1].hash.u.prev = id;
	}
	parentValue->nextSibling = id;
	scrVarGlob.variableList[indexa + 32770].hash.u.prev = 0;
	scrVarGlob.variableList[id + 32770].nextSibling = siblingIndex;
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
	if ((parentValue->w.status & 0x60) != 0x60)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1302,
			0,
			"%s\n\t(parentValue->w.status & VAR_STAT_MASK) = %i",
			"((parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
			parentValue->w.status & 0x60);
	parent = &scrVarGlob.variableList[parentValue->v.next + 1];
	id = scrVarGlob.variableList[indexa + 32770].hash.id;
	if (parent->hash.u.prev)
	{
		siblingValue = &scrVarGlob.variableList[parent->hash.u.prev + 32770];
		if (IsObject(siblingValue))
			MyAssertHandler(".\\script\\scr_variable.cpp", 1313, 0, "%s", "!IsObject( siblingValue )");
		siblingValue->nextSibling = indexa;
		siblingIndex = FindVariableIndexInternal(parentId, siblingValue->w.status >> 8);
		if (!siblingIndex)
			MyAssertHandler(".\\script\\scr_variable.cpp", 1318, 0, "%s", "siblingIndex");
	}
	else
	{
		*(word*)&siblingIndex = 0;
		parentValue->nextSibling = id;
	}
	parent->hash.u.prev = id;
	scrVarGlob.variableList[indexa + 32770].hash.u.prev = siblingIndex;
	scrVarGlob.variableList[id + 32770].nextSibling = 0;
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
	scrVarGlob_t* entryValue; // [esp+20h] [ebp-20h]
	Variable tempEntry; // [esp+24h] [ebp-1Ch]
	VariableValueInternal* oldEntry; // [esp+28h] [ebp-18h]
	VariableValueInternal* oldEntrya; // [esp+28h] [ebp-18h]
	Variable* prev; // [esp+2Ch] [ebp-14h]
	VariableValue value; // [esp+34h] [ebp-Ch]
	VariableValueInternal* oldEntryValue; // [esp+3Ch] [ebp-4h]
	VariableValueInternal* oldEntryValuea; // [esp+3Ch] [ebp-4h]

	entry = &scrVarGlob.variableList[index + 32770];
	entryValue = (scrVarGlob_t*)&scrVarGlob.variableList[entry->hash.id + 32770];
	if ((entryValue->variableList[0].w.status & 0x60) != 0x20 && (entryValue->variableList[0].w.status & 0x60) != 0x40)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1419,
			0,
			"%s",
			"(entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_HEAD");
	if (IsObject(entryValue->variableList))
		MyAssertHandler(".\\script\\scr_variable.cpp", 1420, 0, "%s", "!IsObject( entryValue )");
	if ((parentValue->w.status & 0x1F) == 0x15)
	{
		--parentValue->u.o.u.size;
		if (IsObject(entryValue->variableList))
			MyAssertHandler(".\\script\\scr_variable.cpp", 1425, 0, "%s", "!IsObject( entryValue )");
		value = Scr_GetArrayIndexValue(entryValue->variableList[0].w.status >> 8);
		RemoveRefToValue(value.type, value.u);
	}
	if ((entryValue->variableList[0].w.status & 0x60) == 0x40)
	{
		oldIndex = entryValue->variableList[0].v.next;
		oldEntry = &scrVarGlob.variableList[oldIndex + 32770];
		oldEntryValue = &scrVarGlob.variableList[oldEntry->hash.id + 32770];
		if (oldEntry != entry)
		{
			if ((oldEntryValue->w.status & 0x60) != 0x20)
				MyAssertHandler(
					".\\script\\scr_variable.cpp",
					1439,
					0,
					"%s",
					"(oldEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE");
			oldEntryValue->w.status &= 0xFFFFFF9F;
			oldEntryValue->w.status |= 0x40u;
			prevSiblingIndex = entry->hash.u.prev;
			nextSiblingIndex = entryValue->variableList[0].nextSibling;
			oldPrevSiblingIndex = oldEntry->hash.u.prev;
			if (oldEntryValue->nextSibling)
				scrVarGlob.variableList[oldEntryValue->nextSibling + 32770].hash.u.prev = index;
			if (oldPrevSiblingIndex)
				scrVarGlob.variableList[scrVarGlob.variableList[oldPrevSiblingIndex + 32770].hash.id + 32770].nextSibling = index;
			if (nextSiblingIndex)
				scrVarGlob.variableList[nextSiblingIndex + 32770].hash.u.prev = oldIndex;
			if (prevSiblingIndex)
				scrVarGlob.variableList[scrVarGlob.variableList[prevSiblingIndex + 32770].hash.id + 32770].nextSibling = oldIndex;
			tempEntry = entry->hash;
			entry->hash = oldEntry->hash;
			oldEntry->hash = tempEntry;
			entry = &scrVarGlob.variableList[oldIndex + 32770];
			*(word*)&index = oldIndex;
		}
	}
	else
	{
		oldEntrya = &scrVarGlob.variableList[index + 32770];
		oldEntryValuea = (VariableValueInternal*)entryValue;
		do
		{
			if ((oldEntryValuea->w.status & 0x60) != 0x20 && (oldEntryValuea->w.status & 0x60) != 0x40)
				MyAssertHandler(
					".\\script\\scr_variable.cpp",
					1472,
					0,
					"%s",
					"(oldEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_MOVABLE || (oldEntryValue->w.status & VAR_STAT_MASK) == VAR_STAT_HEAD");
			prev = &oldEntrya->hash;
			oldEntrya = &scrVarGlob.variableList[oldEntryValuea->v.next + 32770];
			oldEntryValuea = &scrVarGlob.variableList[oldEntrya->hash.id + 32770];
		} while (oldEntrya != entry);
		scrVarGlob.variableList[prev->id + 32770].v.next = entryValue->variableList[0].v.next;
	}
	if (entryValue != (scrVarGlob_t*)&scrVarGlob.variableList[entry->hash.id + 32770])
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1482,
			0,
			"%s",
			"entryValue == &scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + entry->hash.id]");
	entryValue->variableList[0].w.status &= 0xFFFFFF9F;
	entryValue->variableList[0].w.status |= 0x60u;
	entryValue->variableList[0].v.next = index;
}

void  FreeChildValue(unsigned int parentId, unsigned int id)
{
	VariableValueInternal* entry; // [esp+0h] [ebp-20h]
	unsigned int nextSiblingIndex; // [esp+4h] [ebp-1Ch]
	unsigned int prevSiblingIndex; // [esp+8h] [ebp-18h]
	unsigned int parentIndex; // [esp+Ch] [ebp-14h]
	VariableValueInternal* entryValue; // [esp+10h] [ebp-10h]
	unsigned int index; // [esp+14h] [ebp-Ch]

	entryValue = &scrVarGlob.variableList[id + 32770];
	if ((entryValue->w.status & 0x60) != 0x60)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1505,
			0,
			"%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
			"((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
			entryValue->w.status & 0x60);
	if (IsObject(entryValue))
		MyAssertHandler(".\\script\\scr_variable.cpp", 1506, 0, "%s", "!IsObject( entryValue )");
	if (scrVarGlob.variableList[entryValue->v.next + 32770].hash.id != id)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1507,
			0,
			"%s",
			"scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN + entryValue->v.index].hash.id == id");
	RemoveRefToValue(entryValue->w.status & 0x1F, entryValue->u.u);
	if (!id || id >= 0xFFFE)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1513, 0, "%s", "id > 0 && id < VARIABLELIST_CHILD_SIZE");
	--scrVarPub.totalObjectRefCount;
	if (scrVarDebugPub)
	{
		if (!scrVarDebugPub->leakCount[id + 32770])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1518,
				0,
				"%s",
				"scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + id]");
		if (--scrVarDebugPub->leakCount[id + 32770])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1520,
				0,
				"%s",
				"!scrVarDebugPub->leakCount[VARIABLELIST_CHILD_BEGIN + id]");
	}
	--scrVarPub.numScriptValues;
	if (scrVarDebugPub)
	{
		if (!scrVarDebugPub->varUsage[id + 32770])
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1528,
				0,
				"%s",
				"scrVarDebugPub->varUsage[VARIABLELIST_CHILD_BEGIN + id]");
		scrVarDebugPub->varUsage[id + 32770] = 0;
	}
	if ((entryValue->w.status & 0x60) != 0x60)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1533,
			0,
			"%s\n\t(entryValue->w.status & VAR_STAT_MASK) = %i",
			"((entryValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL)",
			entryValue->w.status & 0x60);
	index = entryValue->v.next;
	entry = &scrVarGlob.variableList[index + 32770];
	if (entry->hash.id != id)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1537, 0, "%s", "entry->id == id");
	nextSiblingIndex = entryValue->nextSibling;
	prevSiblingIndex = entry->hash.u.prev;
	if (entry->hash.u.prev)
	{
		scrVarGlob.variableList[scrVarGlob.variableList[prevSiblingIndex + 32770].hash.id + 32770].nextSibling = nextSiblingIndex;
	}
	else
	{
		if (scrVarGlob.variableList[32770].hash.id)
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1549,
				0,
				"%s",
				"!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].hash.id");
		scrVarGlob.variableList[parentId + 1].nextSibling = scrVarGlob.variableList[nextSiblingIndex + 32770].hash.id;
	}
	if (nextSiblingIndex)
	{
		scrVarGlob.variableList[nextSiblingIndex + 32770].hash.u.prev = prevSiblingIndex;
	}
	else
	{
		parentIndex = scrVarGlob.variableList[parentId + 1].v.next;
		if (scrVarGlob.variableList[32770].hash.id)
			MyAssertHandler(
				".\\script\\scr_variable.cpp",
				1561,
				0,
				"%s",
				"!scrVarGlob.variableList[VARIABLELIST_CHILD_BEGIN].hash.id");
		scrVarGlob.variableList[parentIndex + 1].hash.u.prev = scrVarGlob.variableList[prevSiblingIndex + 32770].hash.id;
	}
	entryValue->w.status = 0;
	entryValue->u.next = scrVarGlob.variableList[32770].u.next;
	entry->hash.u.prev = 0;
	scrVarGlob.variableList[scrVarGlob.variableList[32770].u.next + 32770].hash.u.prev = index;
	scrVarGlob.variableList[32770].u.next = index;
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
	if (!IsObject(parentValue))
		MyAssertHandler(".\\script\\scr_variable.cpp", 1587, 0, "%s", "IsObject( parentValue )");
	if (parentValue->nextSibling)
	{
		entryValue = &scrVarGlob.variableList[parentValue->nextSibling + 32770];
		if (IsObject(entryValue))
			MyAssertHandler(".\\script\\scr_variable.cpp", 1593, 0, "%s", "!IsObject( entryValue )");
		nextSibling = FindVariableIndexInternal(parentId, entryValue->w.status >> 8);
		if (!nextSibling)
			MyAssertHandler(".\\script\\scr_variable.cpp", 1597, 0, "%s", "nextSibling");
		do
		{
			id = scrVarGlob.variableList[nextSibling + 32770].hash.id;
			MakeVariableExternal(nextSibling, parentValue);
			nextSibling = scrVarGlob.variableList[id + 32770].nextSibling;
		} while (scrVarGlob.variableList[id + 32770].nextSibling);
		nextId = parentValue->nextSibling;
		do
		{
			ida = nextId;
			nextSiblinga = scrVarGlob.variableList[nextId + 32770].nextSibling;
			nextId = scrVarGlob.variableList[nextSiblinga + 32770].hash.id;
			FreeChildValue(parentId, ida);
		} while (nextSiblinga);
	}
}

unsigned int  GetNewArrayVariableIndex(unsigned int parentId, unsigned int unsignedValue)
{
	if (!IsValidArrayIndex(unsignedValue))
		MyAssertHandler(".\\script\\scr_variable.cpp", 2474, 0, "%s", "IsValidArrayIndex( unsignedValue )");
	return GetNewVariableIndexInternal(parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

void  RemoveArrayVariable(unsigned int parentId, unsigned int unsignedValue)
{
	if (!IsValidArrayIndex(unsignedValue))
		MyAssertHandler(".\\script\\scr_variable.cpp", 2777, 0, "%s", "IsValidArrayIndex( unsignedValue )");
	RemoveVariable(parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

void  CopyArray(unsigned int parentId, unsigned int newParentId)
{
	int v2; // edx
	unsigned int v3; // edx
	unsigned int nextSibling; // [esp+0h] [ebp-20h]
	VariableValueInternal* entryValue; // [esp+Ch] [ebp-14h]
	int type; // [esp+10h] [ebp-10h]
	VariableValueInternal* newEntryValue; // [esp+14h] [ebp-Ch]
	unsigned int id; // [esp+1Ch] [ebp-4h]

	if (!IsObject(&scrVarGlob.variableList[parentId + 1]))
		MyAssertHandler(".\\script\\scr_variable.cpp", 2813, 0, "%s", "IsObject( parentValue )");
	if ((scrVarGlob.variableList[parentId + 1].w.status & 0x1F) != 0x15)
		MyAssertHandler(".\\script\\scr_variable.cpp", 2814, 0, "%s", "(parentValue->w.type & VAR_MASK) == VAR_ARRAY");
	v2 = parentId + 1;
	id = scrVarGlob.variableList[v2].nextSibling;
	if (scrVarGlob.variableList[v2].nextSibling)
	{
		while (1)
		{
			entryValue = &scrVarGlob.variableList[id + 32770];
			type = entryValue->w.status & 0x1F;
			if (IsObject(entryValue))
				MyAssertHandler(".\\script\\scr_variable.cpp", 2825, 0, "%s", "!IsObject( entryValue )");
			newEntryValue = &scrVarGlob.variableList[scrVarGlob.variableList[GetVariableIndexInternal(
				newParentId,
				entryValue->w.status >> 8)
				+ 32770].hash.id
				+ 32770];
			if ((newEntryValue->w.status & 0x1F) != 0)
				MyAssertHandler(
					".\\script\\scr_variable.cpp",
					2830,
					0,
					"%s",
					"(newEntryValue->w.type & VAR_MASK) == VAR_UNDEFINED");
			if ((newEntryValue->w.status & 0x1F) != 0)
				MyAssertHandler(".\\script\\scr_variable.cpp", 2832, 0, "%s", "!(newEntryValue->w.type & VAR_MASK)");
			newEntryValue->w.status |= type;
			if (type == 1)
			{
				if ((scrVarGlob.variableList[entryValue->u.u.intValue + 1].w.status & 0x1F) == 0x15)
				{
					newEntryValue->u.u.intValue = Scr_AllocArray();
					CopyArray(entryValue->u.u.stringValue, newEntryValue->u.u.stringValue);
				}
				else
				{
					newEntryValue->u.u.intValue = entryValue->u.u.intValue;
					AddRefToObject(entryValue->u.u.stringValue);
				}
			}
			else
			{
				if (type == 10)
					MyAssertHandler(".\\script\\scr_variable.cpp", 2837, 0, "%s", "type != VAR_STACK");
				newEntryValue->u.u.intValue = entryValue->u.u.intValue;
				AddRefToValue(type, entryValue->u.u);
			}
			v3 = id + 32770;
			nextSibling = scrVarGlob.variableList[v3].nextSibling;
			if (!scrVarGlob.variableList[v3].nextSibling)
				break;
			id = scrVarGlob.variableList[nextSibling + 32770].hash.id;
			if (!scrVarGlob.variableList[nextSibling + 32770].hash.id)
				MyAssertHandler(".\\script\\scr_variable.cpp", 2861, 0, "%s", "id");
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
		if (type1 == 5 && type2 == 6)
		{
			value2->type = 5;
			value2->u.floatValue = (float)value2->u.intValue;
			return;
		}
		if (type1 == 6 && type2 == 5)
		{
			value1->type = 5;
			value1->u.floatValue = (float)value1->u.intValue;
			return;
		}
		if (type1 == 4)
		{
			if (type2 == 5)
			{
				tempVector = Scr_AllocVector();
				*tempVector = value2->u.floatValue;
				tempVector[1] = value2->u.floatValue;
				tempVector[2] = value2->u.floatValue;
				value2->u.intValue = (int)tempVector;
				value2->type = 4;
				return;
			}
			if (type2 == 6)
			{
				tempVectora = Scr_AllocVector();
				*tempVectora = (float)value2->u.intValue;
				tempVectora[1] = (float)value2->u.intValue;
				tempVectora[2] = (float)value2->u.intValue;
				value2->u.intValue = (int)tempVectora;
				value2->type = 4;
				return;
			}
		}
		if (type2 != 4)
			goto LABEL_18;
		if (type1 == 5)
		{
			tempVectorb = Scr_AllocVector();
			*tempVectorb = value1->u.floatValue;
			tempVectorb[1] = value1->u.floatValue;
			tempVectorb[2] = value1->u.floatValue;
			value1->u.intValue = (int)tempVectorb;
			value1->type = 4;
			return;
		}
		if (type1 == 6)
		{
			tempVectorc = Scr_AllocVector();
			*tempVectorc = (float)value1->u.intValue;
			tempVectorc[1] = (float)value1->u.intValue;
			tempVectorc[2] = (float)value1->u.intValue;
			value1->u.intValue = (int)tempVectorc;
			value1->type = 4;
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
	int type1; // [esp+14h] [ebp-Ch]
	int type2; // [esp+18h] [ebp-8h]
	const float* constTempVector; // [esp+1Ch] [ebp-4h]
	const float* constTempVectora; // [esp+1Ch] [ebp-4h]

	type1 = value1->type;
	type2 = value2->type;
	if (type1 != type2)
	{
		if (type1 < type2)
		{
			if (type1 == 2)
			{
				switch (type2)
				{
				case 4:
					value2->type = 2;
					constTempVector = (const float*)value2->u.intValue;
					value2->u.stringValue = SL_GetStringForVector(value2->u.vectorValue);
					RemoveRefToVector(constTempVector);
					return;
				case 5:
					value2->type = 2;
					value2->u.stringValue = SL_GetStringForFloat(value2->u.floatValue);
					return;
				case 6:
					value2->type = 2;
					value2->u.stringValue = SL_GetStringForInt(value2->u.intValue);
					return;
				}
			}
			else if (type1 != 5)
			{
			LABEL_28:
				Scr_UnmatchingTypesError(value1, value2);
				return;
			}
			if (type2 == 6)
			{
				value2->type = 5;
				value2->u.floatValue = (float)value2->u.intValue;
				return;
			}
			goto LABEL_28;
		}
		if (type2 == 2)
		{
			switch (type1)
			{
			case 4:
				value1->type = 2;
				constTempVectora = (const float*)value1->u.intValue;
				value1->u.stringValue = SL_GetStringForVector(value1->u.vectorValue);
				RemoveRefToVector(constTempVectora);
				return;
			case 5:
				value1->type = 2;
				value1->u.stringValue = SL_GetStringForFloat(value1->u.floatValue);
				return;
			case 6:
				value1->type = 2;
				value1->u.stringValue = SL_GetStringForInt(value1->u.intValue);
				return;
			}
		}
		else if (type2 != 5)
		{
			goto LABEL_28;
		}
		if (type1 == 6)
		{
			value1->type = 5;
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

	if ((scrVarGlob.variableList[parentId + 1].w.status & 0x60) == 0)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			4948,
			0,
			"%s",
			"(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE");
	if (!IsObject(&scrVarGlob.variableList[parentId + 1]))
		MyAssertHandler(".\\script\\scr_variable.cpp", 4949, 0, "%s", "IsObject( parentValue )");
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
			usage = Scr_GetEntryUsage(*(unsigned __int8*)buf, u) + usage;
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
	if ((parentValue->w.status & 0x60) != 0x60)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			5245,
			0,
			"%s",
			"(parentValue->w.status & VAR_STAT_MASK) == VAR_STAT_EXTERNAL");
	if (!IsObject(parentValue))
		MyAssertHandler(".\\script\\scr_variable.cpp", 5246, 0, "%s", "IsObject( parentValue )");
	if ((parentValue->w.status & 0x1F) == 0xE)
		MyAssertHandler(".\\script\\scr_variable.cpp", 5248, 0, "%s", "(parentValue->w.type & VAR_MASK) != VAR_THREAD");
	if ((parentValue->w.status & 0x1F) == 0xF)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			5249,
			0,
			"%s",
			"(parentValue->w.type & VAR_MASK) != VAR_NOTIFY_THREAD");
	if ((parentValue->w.status & 0x1F) == 0x10)
		MyAssertHandler(".\\script\\scr_variable.cpp", 5250, 0, "%s", "(parentValue->w.type & VAR_MASK) != VAR_TIME_THREAD");
	if ((parentValue->w.status & 0x1F) == 0x11)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			5251,
			0,
			"%s",
			"(parentValue->w.type & VAR_MASK) != VAR_CHILD_THREAD");
	if ((parentValue->w.status & 0x1F) != 0x15)
		return 0;
	while (2)
	{
		id = FindFirstSibling(parentId);
	LABEL_17:
		if (id)
		{
			entryValue = &scrVarGlob.variableList[id + 32770];
			if ((entryValue->w.status & 0x60) == 0 || (entryValue->w.status & 0x60) == 0x60)
				MyAssertHandler(
					".\\script\\scr_variable.cpp",
					5261,
					0,
					"%s",
					"(entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL");
			if (IsObject(entryValue))
				MyAssertHandler(".\\script\\scr_variable.cpp", 5262, 0, "%s", "!IsObject( entryValue )");
			name = entryValue->w.status >> 8;
			switch (entryValue->w.status & 0x1F)
			{
			case 0u:
			case 2u:
			case 3u:
			case 4u:
			case 5u:
			case 6u:
				goto LABEL_16;
			case 1u:
				if (Scr_MakeValuePrimitive(entryValue->u.u.stringValue))
					goto LABEL_16;
				goto $LN6_69;
			case 7u:
			case 8u:
			case 9u:
			case 0xAu:
			case 0xBu:
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
	if (!IsValidArrayIndex(unsignedValue))
		MyAssertHandler(".\\script\\scr_variable.cpp", 2769, 0, "%s", "IsValidArrayIndex( unsignedValue )");
	SafeRemoveVariable(parentId, (unsignedValue + 0x800000) & 0xFFFFFF);
}

VariableValue  Scr_EvalVariableEntityField(unsigned int entId, unsigned int fieldName)
{
	VariableValueInternal* entValue; // [esp+8h] [ebp-18h]
	unsigned int fieldId; // [esp+10h] [ebp-10h]
	int value; // [esp+14h] [ebp-Ch]
	VariableValue valuea; // [esp+14h] [ebp-Ch]
	VariableUnion id; // [esp+1Ch] [ebp-4h]

	entValue = &scrVarGlob.variableList[entId + 1];
	if ((entValue->w.status & 0x1F) != 0x14)
		MyAssertHandler(".\\script\\scr_variable.cpp", 3044, 0, "%s", "(entValue->w.type & VAR_MASK) == VAR_ENTITY");
	if (entValue->w.status >> 8 >= 4)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			3045,
			0,
			"%s",
			"(entValue->w.classnum >> VAR_NAME_BITS) < CLASS_NUM_COUNT");
	fieldId = FindArrayVariable(g_classMap[entValue->w.status >> 8].id, fieldName);
	if (fieldId)
	{
		valuea = GetEntityFieldValue(
			entValue->w.status >> 8,
			entValue->u.o.u.size,
			scrVarGlob.variableList[fieldId + 32770].u.u.intValue);
		if (valuea.type == 1)
		{
			if ((scrVarGlob.variableList[valuea.u.intValue + 1].w.status & 0x1F) == 0x15)
			{
				if (scrVarGlob.variableList[valuea.u.intValue + 1].u.next)
				{
					id.intValue = (int)valuea.u;
					RemoveRefToObject(valuea.u.stringValue);
					valuea.u.intValue = Scr_AllocArray();
					CopyArray(id.stringValue, valuea.u.stringValue);
				}
				return valuea.u;
			}
			else
			{
				return valuea.u;
			}
		}
		else
		{
			return valuea.u;
		}
	}
	else
	{
		return (VariableUnion)value;
	}
}

void  Scr_ClearThread(unsigned int parentId)
{
	VariableValueInternal* parentValue; // [esp+0h] [ebp-4h]

	if (!parentId)
		MyAssertHandler(".\\script\\scr_variable.cpp", 1648, 0, "%s", "parentId");
	parentValue = &scrVarGlob.variableList[parentId + 1];
	if ((parentValue->w.status & 0x60) == 0)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1651,
			0,
			"%s",
			"(parentValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE");
	if ((parentValue->w.status & 0x1F) < 0xE || (parentValue->w.status & 0x1F) > 0x11)
		MyAssertHandler(
			".\\script\\scr_variable.cpp",
			1652,
			0,
			"%s",
			"((parentValue->w.type & VAR_MASK) >= VAR_THREAD) && ((parentValue->w.type & VAR_MASK) <= VAR_CHILD_THREAD)");
	if (FindVariable(parentId, 0x18001u))
		MyAssertHandler(".\\script\\scr_variable.cpp", 1653, 0, "%s", "!FindVariable( parentId, OBJECT_STACK )");
	if (parentValue->nextSibling)
		ClearObjectInternal(parentId);
	RemoveRefToObject(parentValue->u.o.u.size);
}