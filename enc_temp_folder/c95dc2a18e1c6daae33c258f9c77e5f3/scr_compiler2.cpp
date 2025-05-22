#include "../qcommon/qcommon.h"

#include "scr_compiler.h"
#include "scr_main.h"
#include "scr_debugger.h"
#include "scr_parser.h"
#include "scr_evaluate.h"
#include "scr_vm.h"
#include <game_mp/g_public_mp.h>
#include "scr_animtree.h"
#include <universal/profile.h>
#include "scr_parsetree.h"
#include "scr_yacc.h"

void EmitStatement(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block);
void EmitOpcode(unsigned int op, int offset, int callType);
void Scr_CalcLocalVarsVariableExpressionRef(sval_u expr, scr_block_s *block);
void Scr_CalcLocalVarsDeveloperStatementList(sval_u val, scr_block_s *block, sval_u *devStatBlock);
void EmitExpression(sval_u expr, scr_block_s *block);
void Scr_CalcLocalVarsStatementList(sval_u val, scr_block_s *block);
void Scr_CalcLocalVarsStatement(sval_u val, scr_block_s *block);
bool EmitOrEvalPrimitiveExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block);
void EmitExpression(sval_u expr, scr_block_s *block);
bool EmitOrEvalExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block);
bool EvalExpression(sval_u expr, VariableCompileValue *constValue);
void EmitStatementList(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block);
void EmitPrimitiveExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block);
void EmitVariableExpressionRef(sval_u expr, scr_block_s *block);
void EmitVariableExpression(sval_u expr, scr_block_s *block);
void EmitBreakOn(sval_u expr, sval_u param, sval_u sourcePos);

enum
{
	MAX_VM_STACK_DEPTH = 0x20,
	MAX_VM_OPERAND_STACK = 0x800,
};

#pragma GCC push_options
#pragma GCC optimize ("O0")

#undef GetObject

scrCompilePub_t scrCompilePub;
scrCompileGlob_t scrCompileGlob;

enum
{
	FUNC_SCOPE_LOCAL,
	FUNC_SCOPE_FAR
};

enum scr_builtin_type_t
{
	BUILTIN_ANY = 0x0,
	BUILTIN_DEVELOPER_ONLY = 0x1,
};

enum : __int32
{
	CALL_NONE = 0x0,
	CALL_BUILTIN = 0x1,
	CALL_THREAD = 0x2,
	CALL_FUNCTION = 0x3,
};

/*
============
CompareCaseInfo
============
*/
int CompareCaseInfo(const void *elem1, const void *elem2)
{
	if (*(intptr_t *)elem1 > *(intptr_t *)elem2)
	{
		return -1;
	}

	return *(intptr_t *)elem1 < *(intptr_t *)elem2;
}

/*
============
GetExpressionCount
============
*/
int GetExpressionCount(sval_u exprlist)
{
	sval_u *node;
	int expr_count = 0;

	for (node = exprlist.node[0].node; node; node = node[1].node)
	{
		expr_count++;
	}

	return expr_count;
}

/*
============
Scr_GetBuiltin
============
*/
//unsigned int Scr_GetBuiltin(sval_u func_name)
//{
//	if (func_name.node[0].type != ENUM_script_call)
//	{
//		return 0;
//	}
//
//	func_name = func_name.node[1];
//
//	if (func_name.node[0].type != ENUM_function)
//	{
//		return 0;
//	}
//
//	func_name = func_name.node[1];
//
//	if (func_name.node[0].type != ENUM_local_function)
//	{
//		return 0;
//	}
//
//	func_name = func_name.node[1];
//
//	if (FindVariable(scrCompileGlob.fileId, func_name.idValue))
//	{
//		return 0;
//	}
//
//	return func_name.idValue;
//}

/*
============
Scr_CompileStatement
============
*/
void Scr_CompileStatement(sval_u parseData)
{
	EmitStatement(parseData, false, 0, NULL);
	EmitOpcode(OP_abort, 0, CALL_NONE);
}

/*
============
EmitBreakpointStatement
============
*/
void EmitBreakpointStatement(sval_u sourcePos)
{
	if (!scrVarPub.developer_script)
	{
		return;
	}

	EmitOpcode(OP_breakpoint, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
Scr_IsLastStatement
============
*/
bool Scr_IsLastStatement(sval_u *node)
{
	if (!node)
	{
		return true;
	}

	if (scrVarPub.developer_script)
	{
		return false;
	}

	while (node)
	{
		if (node[0].node[0].type != ENUM_developer_statement_list)
		{
			return false;
		}

		node = node[1].node;
	}

	return true;
}

/*
============
IsUndefinedPrimitiveExpression
============
*/
bool IsUndefinedPrimitiveExpression(sval_u expr)
{
	return expr.node[0].type == ENUM_undefined;
}

/*
============
Scr_CalcLocalVarsArrayPrimitiveExpressionRef
============
*/
void Scr_CalcLocalVarsArrayPrimitiveExpressionRef(sval_u expr, scr_block_s *block)
{
	if (expr.node[0].type != ENUM_variable)
	{
		return;
	}

	Scr_CalcLocalVarsVariableExpressionRef(expr.node[1], block);
}

/*
============
Scr_GetUncacheType
============
*/
int Scr_GetUncacheType(int type)
{
	if (type == VAR_CODEPOS)
	{
		return BUILTIN_ANY;
	}

	iassert(type == VAR_DEVELOPER_CODEPOS);
	return BUILTIN_DEVELOPER_ONLY;
}

/*
============
Scr_GetCacheType
============
*/
int Scr_GetCacheType(int type)
{
	if (type == BUILTIN_ANY)
	{
		return VAR_CODEPOS;
	}

	iassert(type == BUILTIN_DEVELOPER_ONLY);
	return VAR_DEVELOPER_CODEPOS;
}

/*
============
GetSingleParameter
============
*/
sval_u *GetSingleParameter(sval_u exprlist)
{
	if (exprlist.node[0].node == NULL)
	{
		return NULL;
	}

	if (exprlist.node[0].node[1].node != NULL)
	{
		return NULL;
	}

	return exprlist.node[0].node;
}

/*
============
EmitExpressionList
============
*/
int EmitExpressionList(sval_u exprlist, scr_block_s *block)
{
	sval_u *node;
	int expr_count = 0;

	for (node = exprlist.node[0].node; node; node = node[1].node)
	{
		EmitExpression(node[0].node[0], block);
		expr_count++;
	}

	return expr_count;
}

/*
============
Scr_CalcLocalVarsArrayVariableRef
============
*/
void Scr_CalcLocalVarsArrayVariableRef(sval_u expr, scr_block_s *block)
{
	Scr_CalcLocalVarsArrayPrimitiveExpressionRef(expr, block);
}

/*
============
Scr_PopValue
============
*/
void Scr_PopValue()
{
	iassert(scrCompilePub.value_count);
	scrCompilePub.value_count--;
}

/*
============
EvalIString
============
*/
void EvalIString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue)
{
	iassert(constValue);
	constValue->value.type = VAR_ISTRING;
	constValue->value.u.stringValue = value;
	constValue->sourcePos = sourcePos;
}

/*
============
EvalString
============
*/
void EvalString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue)
{
	iassert(constValue);
	constValue->value.type = VAR_STRING;
	constValue->value.u.stringValue = value;
	constValue->sourcePos = sourcePos;
}

/*
============
Scr_InitFromChildBlocks
============
*/
void Scr_InitFromChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block)
{
	int localVarsCreateCount, childIndex, i;
	scr_block_s *childBlock;
	unsigned int name;

	if (!childCount)
	{
		return;
	}

	localVarsCreateCount = childBlocks[0]->localVarsPublicCount;

	for (childIndex = 1; childIndex < childCount; childIndex++)
	{
		childBlock = childBlocks[childIndex];

		if (childBlock->localVarsPublicCount < localVarsCreateCount)
		{
			localVarsCreateCount = childBlock->localVarsPublicCount;
		}
	}

	iassert(block->localVarsCreateCount <= localVarsCreateCount);
	iassert(localVarsCreateCount <= block->localVarsCount);

	block->localVarsCreateCount = localVarsCreateCount;

	for (i = 0; i < localVarsCreateCount; i++)
	{
		iassert(i < block->localVarsCount);

		if (!((1 << (i & 7)) & block->localVarsInitBits[i >> 3]))
		{
			name = block->localVars[i].name;

			for (childIndex = 0; childIndex < childCount; childIndex++)
			{
				childBlock = childBlocks[childIndex];

				iassert(localVarsCreateCount <= childBlock->localVarsPublicCount);
				iassert(i < childBlock->localVarsPublicCount);
				iassert(childBlock->localVars[i].name == name);

				if (!((1 << (i & 7)) & childBlock->localVarsInitBits[i >> 3]))
				{
					goto out;
				}
			}

			block->localVarsInitBits[i >> 3] |= 1 << (i & 7);
		}

	out:
		;
	}
}

/*
============
Scr_FindLocalVar
============
*/
int Scr_FindLocalVar(scr_block_s *block, int startIndex, unsigned int name)
{
	for (int i = startIndex; i < block->localVarsCount; i++)
	{
		if (block->localVars[i].name == name)
		{
			return i;
		}
	}

	return -1;
}

/*
============
EvalFloat
============
*/
void EvalFloat(float value, sval_u sourcePos, VariableCompileValue *constValue)
{
	iassert(constValue);
	constValue->value.type = VAR_FLOAT;
	constValue->value.u.floatValue = value;
	constValue->sourcePos = sourcePos;
}

/*
============
EvalInteger
============
*/
void EvalInteger(int value, sval_u sourcePos, VariableCompileValue *constValue)
{
	iassert(constValue);
	constValue->value.type = VAR_INTEGER;
	constValue->value.u.intValue = value;
	constValue->sourcePos = sourcePos;
}

/*
============
EvalUndefined
============
*/
void EvalUndefined(sval_u sourcePos, VariableCompileValue *constValue)
{
	iassert(constValue);
	constValue->value.type = VAR_UNDEFINED;
	constValue->sourcePos = sourcePos;
}

/*
============
IsUndefinedExpression
============
*/
bool IsUndefinedExpression(sval_u expr)
{
	if (expr.node[0].type == ENUM_primitive_expression)
	{
		if (IsUndefinedPrimitiveExpression(expr.node[1]))
		{
			return true;
		}
	}

	return false;
}

/*
============
Scr_AppendChildBlocks
============
*/
void Scr_AppendChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block)
{
	unsigned int name;
	int childIndex, i;

	if (!childCount)
	{
		return;
	}

	if (block->abortLevel != SCR_ABORT_NONE)
	{
		return;
	}

	for (childIndex = 0; childIndex < childCount; childIndex++)
	{
		childBlocks[childIndex]->abortLevel = SCR_ABORT_NONE;
	}

	for (i = 0; i < childBlocks[0]->localVarsCount; i++)
	{
		name = childBlocks[0]->localVars[i].name;

		if (Scr_FindLocalVar(block, 0, name) >= 0)
		{
			continue;
		}

		for (childIndex = 1; childIndex < childCount; childIndex++)
		{
			if (Scr_FindLocalVar(childBlocks[childIndex], 0, name) < 0)
			{
				goto out;
			}
		}

		block->localVars[block->localVarsCount].name = name;
		block->localVarsCount++;
	out:
		;
	}
}

/*
============
AddExpressionListOpcodePos
============
*/
void AddExpressionListOpcodePos(sval_u exprlist)
{
	if (!scrVarPub.developer)
	{
		return;
	}

	for (sval_u *node = exprlist.node[0].node; node; node = node[1].node)
	{
		AddOpcodePos(node[0].node[1].sourcePosValue, SOURCE_TYPE_NONE);
	}
}

/*
============
EmitGetUndefined
============
*/
void EmitGetUndefined(sval_u sourcePos)
{
	EmitOpcode(OP_GetUndefined, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
Scr_CompileRemoveRefToString
============
*/
void Scr_CompileRemoveRefToString(unsigned int stringValue)
{
	iassert(stringValue);

	if (scrCompileGlob.bConstRefCount)
	{
		return;
	}

	SL_RemoveRefToString(stringValue);
}

/*
============
AddFilePrecache
============
*/
unsigned int AddFilePrecache(unsigned int filename, unsigned int sourcePos, bool include)
{
	iassert(scrCompileGlob.precachescriptList);

	SL_AddRefToString(filename);
	Scr_CompileRemoveRefToString(filename);

	scrCompileGlob.precachescriptList->filename = filename;
	scrCompileGlob.precachescriptList->sourcePos = sourcePos;
	scrCompileGlob.precachescriptList->include = include;

	scrCompileGlob.precachescriptList++;

	//return GetObject(GetVariable(scrCompilePub.scriptsPos, filename));
	
	return GetObject(GetVariable(scrCompilePub.scripts, filename));
}

/*
============
CompileTransferRefToString
============
*/
void CompileTransferRefToString(unsigned int stringValue, unsigned char user)
{
	iassert(stringValue);

	if (scrCompilePub.developer_statement == SCR_DEV_IGNORE)
	{
		Scr_CompileRemoveRefToString(stringValue);
		return;
	}

	if (scrCompileGlob.bConstRefCount)
	{
		SL_AddRefToString(stringValue);
	}

	SL_TransferRefToUser(stringValue, user);
}

/*
============
EmitCaseStatementInfo
============
*/
void EmitCaseStatementInfo(unsigned int name, sval_u sourcePos)
{
	CaseStatementInfo *newCaseStatement;

	if (scrCompilePub.developer_statement == SCR_DEV_IGNORE)
	{
		iassert(!scrVarPub.developer_script);
		return;
	}

	//newCaseStatement = (CaseStatementInfo *)Hunk_AllocateTempMemoryHighInternal(sizeof(*newCaseStatement));
	newCaseStatement = (CaseStatementInfo *)Hunk_AllocateTempMemoryHigh(16, "EmitCaseStatementInfo");

	newCaseStatement->name = name;
	newCaseStatement->codePos = (char *)TempMalloc(0);
	newCaseStatement->sourcePos = sourcePos.sourcePosValue;
	newCaseStatement->next = scrCompileGlob.currentCaseStatement;

	scrCompileGlob.currentCaseStatement = newCaseStatement;
}

/*
============
EmitDefaultStatement
============
*/
void EmitDefaultStatement(sval_u sourcePos)
{
	EmitCaseStatementInfo(0, sourcePos);
}

/*
============
ConnectContinueStatements
============
*/
void ConnectContinueStatements()
{
	const char *codePos = (char *)TempMalloc(0);

	for (ContinueStatementInfo *statement = scrCompileGlob.currentContinueStatement; statement; statement = statement->next)
	{
		*(intptr_t *)statement->codePos = codePos - statement->nextCodePos;
	}
}

/*
============
ConnectBreakStatements
============
*/
void ConnectBreakStatements()
{
	iassert(!scrCompilePub.value_count);
	const char *codePos = (char *)TempMalloc(0);

	for (BreakStatementInfo *statement = scrCompileGlob.currentBreakStatement; statement; statement = statement->next)
	{
		*(intptr_t *)statement->codePos = codePos - statement->nextCodePos;
	}
}

/*
============
SetThreadPosition
============
*/
void SetThreadPosition(unsigned int posId)
{
	VariableUnion *value;

	value = &GetVariableValueAddress(FindVariable(posId, 1))->u;
	value->codePosValue = (char *)TempMalloc(0);
}

/*
============
Scr_BeginDevScript
============
*/
void Scr_BeginDevScript(int *type, char **savedPos)
{
	if (scrCompilePub.developer_statement != SCR_DEV_NO)
	{
		*type = BUILTIN_ANY;
		return;
	}

	if (scrVarPub.developer_script)
	{
		scrCompilePub.developer_statement = SCR_DEV_YES;
		*type = BUILTIN_DEVELOPER_ONLY;
		return;
	}

	*savedPos = (char *)TempMalloc(0);
	scrCompilePub.developer_statement = SCR_DEV_IGNORE;
	*type = BUILTIN_DEVELOPER_ONLY;
}

/*
============
AddFunction
============
*/
int AddFunction(intptr_t func)
{
	int i;

	for (i = 0; i < scrCompilePub.func_table_size; i++)
	{
		if (scrCompilePub.func_table[i] == func)
		{
			return i;
		}
	}

	iassert(i == scrCompilePub.func_table_size);

	if (scrCompilePub.func_table_size == SCR_FUNC_TABLE_SIZE)
	{
		Com_Error(ERR_DROP, "SCR_FUNC_TABLE_SIZE exceeded");
	}

	scrCompilePub.func_table[scrCompilePub.func_table_size] = func;
	scrCompilePub.func_table_size++;

	return i;
}

/*
============
Scr_CopyBlock
============
*/
void Scr_CopyBlock(scr_block_s *from, scr_block_s **to)
{
	if (*to == NULL)
	{
		//*to = (scr_block_s *)Hunk_AllocateTempMemoryHighInternal(sizeof(**to));
		*to = (scr_block_s*)Hunk_AllocateTempMemoryHigh(536, "Scr_CopyBlock");
	}

	**to = *from;
	to[0]->localVarsPublicCount = 0;
}

/*
============
Scr_CheckLocalVarsCount
============
*/
void Scr_CheckLocalVarsCount(int localVarsCount)
{
	if (localVarsCount < LOCAL_VAR_STACK_SIZE)
	{
		return;
	}

	Com_Error(ERR_DROP, "LOCAL_VAR_STACK_SIZE exceeded");
}

/*
============
Scr_CheckMaxSwitchCases
============
*/
void Scr_CheckMaxSwitchCases(int count)
{
	if (count < MAX_SWITCH_CASES)
	{
		return;
	}

	Com_Error(ERR_DROP, "MAX_SWITCH_CASES exceeded");
}

/*
============
EmitByte
============
*/
void EmitByte(byte value)
{
	scrCompileGlob.codePos = (byte *)TempMalloc(sizeof(byte));
	*(byte *)scrCompileGlob.codePos = value;
}

/*
============
EmitInclude
============
*/
void EmitInclude(sval_u val)
{
	iassert(val.node[0].type == ENUM_include);

	unsigned int filename = Scr_CreateCanonicalFilename(SL_ConvertToString(val.node[1].stringValue));
	Scr_CompileRemoveRefToString(val.node[1].stringValue);

	AddFilePrecache(filename, val.node[2].sourcePosValue, true);
}

/*
============
Scr_AddContinueBlock
============
*/
void Scr_AddContinueBlock(scr_block_s *block)
{
	if (block->abortLevel)
	{
		return;
	}

	if (!scrCompileGlob.continueChildBlocks)
	{
		return;
	}

	Scr_CheckMaxSwitchCases(*scrCompileGlob.continueChildCount);

	scrCompileGlob.continueChildBlocks[*scrCompileGlob.continueChildCount] = block;
	(*scrCompileGlob.continueChildCount)++;
}

/*
============
Scr_AddBreakBlock
============
*/
void Scr_AddBreakBlock(scr_block_s *block)
{
	if (block->abortLevel)
	{
		return;
	}

	if (!scrCompileGlob.breakChildBlocks)
	{
		return;
	}

	Scr_CheckMaxSwitchCases(*scrCompileGlob.breakChildCount);

	scrCompileGlob.breakChildBlocks[*scrCompileGlob.breakChildCount] = block;
	(*scrCompileGlob.breakChildCount)++;
}

/*
============
Scr_EndDevScript
============
*/
void Scr_EndDevScript(int type, char **savedPos)
{
	if (type != BUILTIN_DEVELOPER_ONLY)
	{
		return;
	}

	iassert(type == BUILTIN_DEVELOPER_ONLY);
	scrCompilePub.developer_statement = SCR_DEV_NO;

	if (scrVarPub.developer_script)
	{
		return;
	}

	TempMemorySetPos(*savedPos);
}

/*
============
Scr_TransferBlock
============
*/
void Scr_TransferBlock(scr_block_s *from, scr_block_s *to)
{
	unsigned int name;
	unsigned int sourcePos;
	int i, j;

	iassert(to->localVarsPublicCount <= from->localVarsCount);

	for (i = 0; i < to->localVarsPublicCount || i < from->localVarsCreateCount; i++)
	{
		name = from->localVars[i].name;
		sourcePos = from->localVars[i].sourcePos;
		j = Scr_FindLocalVar(to, i, name);

		if (j < 0)
		{
			j = to->localVarsCount;

			Scr_CheckLocalVarsCount(j);
			to->localVarsCount++;
		}

		iassert(to->localVarsPublicCount <= from->localVarsCount);

		if (j >= to->localVarsPublicCount)
		{
			to->localVarsPublicCount++;
		}

		while (j > i)
		{
			//to->localVars[j] = *(scr_localVar_t *)&to->localVarsInitBits[sizeof(scr_localVar_t) * j + sizeof(unsigned int)];
			to->localVars[j].name = *(unsigned int*)&to->localVarsInitBits[8 * j];
			to->localVars[j].sourcePos = *(unsigned int*)&to->localVarsInitBits[8 * j + 4];
			j--;
		}

		to->localVars[i].name = name;
		to->localVars[i].sourcePos = sourcePos;

		if ((1 << (i & 7)) & from->localVarsInitBits[i >> 3])
		{
			to->localVarsInitBits[i >> 3] |= 1 << (i & 7);
		}
	}

	iassert(from->localVarsCreateCount <= to->localVarsPublicCount);

	to->localVarsCreateCount = from->localVarsCreateCount;
	to->abortLevel = SCR_ABORT_NONE;
}

/*
============
Scr_MergeChildBlocks
============
*/
void Scr_MergeChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block)
{
	scr_block_s *childBlock;
	unsigned int name;
	unsigned int sourcePos;
	int childIndex, i, j;

	if (!childCount)
	{
		return;
	}

	if (block->abortLevel != SCR_ABORT_NONE)
	{
		return;
	}

	for (childIndex = 0; childIndex < childCount; childIndex++)
	{
		childBlock = childBlocks[childIndex];

		iassert(!childBlock->localVarsPublicCount);
		childBlock->localVarsPublicCount = block->localVarsCount;

		for (i = 0; i < block->localVarsCount; i++)
		{
			name = block->localVars[i].name;
			sourcePos = block->localVars[i].sourcePos;
			j = Scr_FindLocalVar(childBlock, i, name);

			if (j < 0)
			{
				j = childBlock->localVarsCount;

				Scr_CheckLocalVarsCount(j);
				childBlock->localVarsCount++;
			}

			while (j > i)
			{
				//childBlock->localVars[j] = *(scr_localVar_t *)&childBlock->localVarsInitBits[sizeof(scr_localVar_t) * j + sizeof(unsigned int)];
				childBlock->localVars[j].name = *(unsigned int *)&childBlock->localVarsInitBits[8 * j];
				childBlock->localVars[j].sourcePos = *(unsigned int *)&childBlock->localVarsInitBits[8 * j + 4];
				j--;
			}

			childBlock->localVars[i].name = name;
			childBlock->localVars[i].sourcePos = sourcePos;
		}
	}
}

/*
============
Scr_RegisterLocalVar
============
*/
void Scr_RegisterLocalVar(unsigned int name, sval_u sourcePos, scr_block_s *block)
{
	if (block->abortLevel != SCR_ABORT_NONE)
	{
		return;
	}

	for (int i = 0; i < block->localVarsCount; i++)
	{
		if (block->localVars[i].name == name)
		{
			return;
		}
	}

	iassert(name);

	Scr_CheckLocalVarsCount(block->localVarsCount);

	block->localVars[block->localVarsCount].name = name;
	// block->localVars[block->localVarsCount].sourcePos = sourcePos.sourcePosValue;

	block->localVarsCount++;
}

/*
============
EmitCodepos
============
*/
void EmitCodepos(const char *pos)
{
	scrCompileGlob.codePos = (byte *)TempMallocAlignStrict(sizeof(const char *));
	*(const char **)scrCompileGlob.codePos = pos;
}

/*
============
EmitString
============
*/
void EmitString(unsigned int value)
{
	scrCompileGlob.codePos = (byte *)TempMallocAlignStrict(sizeof(unsigned short));
	*(unsigned short *)scrCompileGlob.codePos = value;
}

/*
============
EmitFloat
============
*/
void EmitFloat(float value)
{
	scrCompileGlob.codePos = (byte *)TempMallocAlignStrict(sizeof(float));
	*(float *)scrCompileGlob.codePos = value;
}

/*
============
EmitUnsignedShort
============
*/
void EmitUnsignedShort(unsigned short value)
{
	scrCompileGlob.codePos = (byte *)TempMallocAlignStrict(sizeof(unsigned short));
	*(unsigned short *)scrCompileGlob.codePos = value;
}

/*
============
EmitShort
============
*/
void EmitShort(short value)
{
	scrCompileGlob.codePos = (byte *)TempMallocAlignStrict(sizeof(short));
	*(short *)scrCompileGlob.codePos = value;
}

/*
============
EmitInteger
============
*/
void EmitInteger(int value)
{
	scrCompileGlob.codePos = (byte *)TempMallocAlignStrict(sizeof(int));
	*(int *)scrCompileGlob.codePos = value;
}

/*
============
EmitCanonicalString
============
*/
void EmitCanonicalString(unsigned int stringValue)
{
	iassert(stringValue);
	scrCompileGlob.codePos = (byte *)TempMallocAlignStrict(sizeof(unsigned short));

	if (scrCompilePub.developer_statement == SCR_DEV_IGNORE)
	{
		iassert(!scrVarPub.developer_script);
		Scr_CompileRemoveRefToString(stringValue);
		return;
	}

	if (scrCompileGlob.bConstRefCount)
	{
		SL_AddRefToString(stringValue);
	}

	//*(unsigned short *)scrCompileGlob.codePos = Scr_GetCanonicalStringIndex(stringValue);
	*(unsigned short*)scrCompileGlob.codePos = SL_TransferToCanonicalString(stringValue);
}

/*
============
EmitIncludeList
============
*/
void EmitIncludeList(sval_u val)
{
	for (sval_u *node = val.node[0].node[1].node; node; node = node[1].node)
	{
		EmitInclude(node[0]);
	}
}

/*
============
Scr_CalcLocalVarsDeveloperStatementList
============
*/
void Scr_CalcLocalVarsDeveloperStatementList(sval_u val, scr_block_s *block, sval_u *devStatBlock)
{
	Scr_CopyBlock(block, &devStatBlock->block);
	Scr_CalcLocalVarsStatementList(val, devStatBlock->block);
	Scr_MergeChildBlocks(&devStatBlock->block, 1, block);
}

/*
============
Scr_CalcLocalVarsSwitchStatement
============
*/
void Scr_CalcLocalVarsSwitchStatement(sval_u stmtlist, scr_block_s *block)
{
	int childCount, breakChildCount;
	int abortLevel = SCR_ABORT_RETURN;
	bool hasDefault;
	scr_block_s *currentBlock;
	int *oldBreakChildCount;
	scr_block_s **oldBreakChildBlocks, **breakChildBlocks, **childBlocks;
	sval_u *node;

	oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
	oldBreakChildCount = scrCompileGlob.breakChildCount;

	breakChildCount = 0;
	//breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHighInternal(sizeof(scr_block_s **) * MAX_SWITCH_CASES);
	breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s **) * MAX_SWITCH_CASES, "Scr_CalcLocalVarsSwitchStatement");

	scrCompileGlob.breakChildBlocks = breakChildBlocks;
	scrCompileGlob.breakChildCount = &breakChildCount;

	childCount = 0;
	currentBlock = NULL;

	hasDefault = false;
	//childBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHighInternal(sizeof(scr_block_s **) * MAX_SWITCH_CASES);
	childBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s **) * MAX_SWITCH_CASES, "Scr_CalcLocalVarsSwitchStatement");

	for (node = stmtlist.node[0].node[1].node; node; node = node[1].node)
	{
		if (node[0].node[0].type == ENUM_case || node[0].node[0].type == ENUM_default)
		{
			currentBlock = NULL;
			Scr_CopyBlock(block, &currentBlock);

			if (node[0].node[0].type == ENUM_case)
			{
				node[0].node[3].block = currentBlock;
			}
			else
			{
				node[0].node[2].block = currentBlock;
				hasDefault = true;
			}
		}
		else if (currentBlock)
		{
			Scr_CalcLocalVarsStatement(node[0], currentBlock);

			if (currentBlock->abortLevel != SCR_ABORT_NONE)
			{
				if (currentBlock->abortLevel == SCR_ABORT_BREAK)
				{
					currentBlock->abortLevel = SCR_ABORT_NONE;
					abortLevel = SCR_ABORT_NONE;

					Scr_CheckMaxSwitchCases(childCount);

					childBlocks[childCount] = currentBlock;
					childCount++;
				}
				else if (currentBlock->abortLevel <= abortLevel)
				{
					abortLevel = currentBlock->abortLevel;
				}

				currentBlock = NULL;
			}
		}
	}

	if (hasDefault)
	{
		if (currentBlock)
		{
			Scr_AddBreakBlock(currentBlock);
			Scr_CheckMaxSwitchCases(childCount);

			childBlocks[childCount] = currentBlock;
			childCount++;
		}

		if (block->abortLevel == SCR_ABORT_NONE)
		{
			block->abortLevel = abortLevel;
		}

		Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
		Scr_MergeChildBlocks(childBlocks, childCount, block);
	}

	scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
	scrCompileGlob.breakChildCount = oldBreakChildCount;
}

/*
============
Scr_CalcLocalVarsIfElseStatement
============
*/
void Scr_CalcLocalVarsIfElseStatement(sval_u stmt1, sval_u stmt2, scr_block_s *block, sval_u *ifStatBlock, sval_u *elseStatBlock)
{
	int childCount = 0, abortLevel = SCR_ABORT_RETURN;
	scr_block_s *childBlocks[2];

	Scr_CopyBlock(block, &ifStatBlock->block);
	Scr_CalcLocalVarsStatement(stmt1, ifStatBlock->block);

	if (ifStatBlock->node[0].intValue <= SCR_ABORT_MAX)
	{
		abortLevel = ifStatBlock->node[0].intValue;

		if (abortLevel == SCR_ABORT_NONE)
		{
			childBlocks[0] = ifStatBlock->block;
			childCount = 1;
		}
	}

	Scr_CopyBlock(block, &elseStatBlock->block);
	Scr_CalcLocalVarsStatement(stmt2, elseStatBlock->block);

	if (elseStatBlock->node[0].intValue <= abortLevel)
	{
		abortLevel = elseStatBlock->node[0].intValue;

		if (abortLevel == SCR_ABORT_NONE)
		{
			childBlocks[childCount] = elseStatBlock->block;
			childCount++;
		}
	}

	if (block->abortLevel == SCR_ABORT_NONE)
	{
		block->abortLevel = abortLevel;
	}

	Scr_AppendChildBlocks(childBlocks, childCount, block);
	Scr_MergeChildBlocks(childBlocks, childCount, block);
}

/*
============
Scr_CalcLocalVarsIfStatement
============
*/
void Scr_CalcLocalVarsIfStatement(sval_u stmt, scr_block_s *block, sval_u *ifStatBlock)
{
	Scr_CopyBlock(block, &ifStatBlock->block);
	Scr_CalcLocalVarsStatement(stmt, ifStatBlock->block);
	Scr_MergeChildBlocks(&ifStatBlock->block, 1, block);
}

/*
============
EmitGetVector
============
*/
void EmitGetVector(const float *value, sval_u sourcePos)
{
	EmitOpcode(OP_GetVector, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);

	for (int i = 0; i < 3; i++)
	{
		EmitFloat(value[i]);
	}

	RemoveRefToVector(value);
}

/*
============
EmitGetIString
============
*/
void EmitGetIString(unsigned int value, sval_u sourcePos)
{
	EmitOpcode(OP_GetIString, 1, CALL_NONE);
	AddOpcodePos(sourcePos.stringValue, SOURCE_TYPE_BREAKPOINT);
	EmitString(value);
	CompileTransferRefToString(value, 1);
}

/*
============
EmitGetString
============
*/
void EmitGetString(unsigned int value, sval_u sourcePos)
{
	EmitOpcode(OP_GetString, 1, CALL_NONE);
	AddOpcodePos(sourcePos.stringValue, 1);
	EmitString(value);
	CompileTransferRefToString(value, 1);
}

/*
============
Scr_CalcLocalVarsSafeSetVariableField
============
*/
void Scr_CalcLocalVarsSafeSetVariableField(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	Scr_RegisterLocalVar(expr.idValue, sourcePos, block);
}

/*
============
EmitGetFloat
============
*/
void EmitGetFloat(float value, sval_u sourcePos)
{
	EmitOpcode(OP_GetFloat, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	EmitFloat(value);
}

/*
============
EmitGetInteger
============
*/
void EmitGetInteger(int value, sval_u sourcePos)
{
	if (value >= 0)
	{
		if (value == 0)
		{
			EmitOpcode(OP_GetZero, 1, CALL_NONE);
			AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
			return;
		}

		if (value <= UCHAR_MAX)
		{
			EmitOpcode(OP_GetByte, 1, CALL_NONE);
			AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
			EmitByte(value);
			return;
		}

		if (value <= USHRT_MAX)
		{
			EmitOpcode(OP_GetUnsignedShort, 1, CALL_NONE);
			AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
			EmitUnsignedShort(value);
			return;
		}
	}
	else
	{
		if (value >= -UCHAR_MAX)
		{
			EmitOpcode(OP_GetNegByte, 1, CALL_NONE);
			AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
			EmitByte(-value);
			return;
		}

		if (value >= -USHRT_MAX)
		{
			EmitOpcode(OP_GetNegUnsignedShort, 1, CALL_NONE);
			AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
			EmitUnsignedShort(-value);
			return;
		}
	}

	EmitOpcode(OP_GetInteger, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	EmitCodepos((const char *)value);
}

/*
============
EmitCanonicalStringConst
============
*/
void EmitCanonicalStringConst(unsigned int stringValue)
{
	bool bConstRefCount = scrCompileGlob.bConstRefCount;
	scrCompileGlob.bConstRefCount = true;

	EmitCanonicalString(stringValue);
	scrCompileGlob.bConstRefCount = bConstRefCount;
}

/*
============
Scr_CalcLocalVarsFormalParameterListInternal
============
*/
void Scr_CalcLocalVarsFormalParameterListInternal(sval_u *node, scr_block_s *block)
{
	while (1)
	{
		node = node[1].node;

		if (!node)
		{
			break;
		}

		Scr_CalcLocalVarsSafeSetVariableField(node[0].node[0], node[0].node[1], block);
	}
}

/*
============
Scr_CalcLocalVarsVariableExpressionRef
============
*/
void Scr_CalcLocalVarsVariableExpressionRef(sval_u expr, scr_block_s *block)
{
	if (expr.node[0].type == ENUM_local_variable)
	{
		Scr_CalcLocalVarsSafeSetVariableField(expr.node[1], expr.node[2], block);
	}
	else if (expr.node[0].type == ENUM_array_variable)
	{
		Scr_CalcLocalVarsArrayVariableRef(expr.node[1], block);
	}
}

/*
============
Scr_CalcLocalVarsBinaryEqualsOperatorExpression
============
*/
void Scr_CalcLocalVarsBinaryEqualsOperatorExpression(sval_u expr, scr_block_s *block)
{
	Scr_CalcLocalVarsVariableExpressionRef(expr, block);
}

/*
============
EmitValue
============
*/
void EmitValue(VariableCompileValue *constValue)
{
	switch (constValue->value.type)
	{
	case VAR_UNDEFINED:
		EmitGetUndefined(constValue->sourcePos);
		break;

	case VAR_STRING:
		EmitGetString(constValue->value.u.stringValue, constValue->sourcePos);
		break;

	case VAR_ISTRING:
		EmitGetIString(constValue->value.u.stringValue, constValue->sourcePos);
		break;

	case VAR_VECTOR:
		EmitGetVector(constValue->value.u.vectorValue, constValue->sourcePos);
		break;

	case VAR_FLOAT:
		EmitGetFloat(constValue->value.u.floatValue, constValue->sourcePos);
		break;

	case VAR_INTEGER:
		EmitGetInteger(constValue->value.u.intValue, constValue->sourcePos);
		break;

	default:
		break;
	}
}

/*
============
EmitOpcode
============
*/
void EmitOpcode(unsigned int op, int offset, int callType)
{
	unsigned int index;
	int value_count, valueIndex;

	if (scrCompilePub.value_count)
	{
		value_count = scrCompilePub.value_count;
		scrCompilePub.value_count = 0;

		for (valueIndex = 0; valueIndex < value_count; valueIndex++)
		{
			EmitValue(&scrCompileGlob.value_start[valueIndex]);
		}
	}

	scrCompilePub.allowedBreakpoint = false;

	if (!scrCompileGlob.cumulOffset || callType == CALL_THREAD || callType == CALL_FUNCTION)
	{
		scrCompilePub.allowedBreakpoint = true;
	}

	scrCompileGlob.cumulOffset += offset;

	if (scrCompileGlob.maxOffset < scrCompileGlob.cumulOffset)
	{
		scrCompileGlob.maxOffset = scrCompileGlob.cumulOffset;
	}

	if (callType != CALL_NONE && scrCompileGlob.maxCallOffset < scrCompileGlob.cumulOffset)
	{
		scrCompileGlob.maxCallOffset = scrCompileGlob.cumulOffset;
	}

	scrVarPub.checksum *= 31;
	scrVarPub.checksum += op;

	if (!scrCompilePub.opcodePos)
	{
		scrCompileGlob.prevOpcodePos = scrCompilePub.opcodePos;
		scrCompilePub.opcodePos = (byte *)TempMalloc(sizeof(byte));
		scrCompileGlob.codePos = scrCompilePub.opcodePos;
		*scrCompilePub.opcodePos = op;
		return;
	}

	scrCompileGlob.codePos = scrCompilePub.opcodePos;

	switch (op)
	{
	case OP_EvalArray:
		if (*scrCompilePub.opcodePos == OP_EvalLocalVariableCached)
		{
			RemoveOpcodePos();
			*scrCompilePub.opcodePos = OP_EvalLocalArrayCached;
			return;
		}

		index = *scrCompilePub.opcodePos - OP_EvalLocalVariableCached0;

		if (index > OP_GetNegByte)
		{
			break;
		}

		RemoveOpcodePos();
		*scrCompilePub.opcodePos = OP_EvalLocalArrayCached;
		EmitByte(index);
		return;

	case OP_EvalArrayRef:
		if (*scrCompilePub.opcodePos == OP_EvalLocalVariableRefCached)
		{
			RemoveOpcodePos();
			*scrCompilePub.opcodePos = OP_EvalLocalArrayRefCached;
			return;
		}

		if (*scrCompilePub.opcodePos != OP_EvalLocalVariableRefCached0)
		{
			break;
		}

		RemoveOpcodePos();
		*scrCompilePub.opcodePos = OP_EvalLocalArrayRefCached0;
		return;

	case OP_EvalFieldVariable:
		if (*scrCompilePub.opcodePos == OP_GetSelfObject)
		{
			*scrCompilePub.opcodePos = OP_EvalSelfFieldVariable;
			return;
		}

		if (*scrCompilePub.opcodePos == OP_GetLevelObject)
		{
			*scrCompilePub.opcodePos = OP_EvalLevelFieldVariable;
			return;
		}

		if (*scrCompilePub.opcodePos != OP_GetAnimObject)
		{
			break;
		}

		*scrCompilePub.opcodePos = OP_EvalAnimFieldVariable;
		return;

	case OP_EvalFieldVariableRef:
		if (*scrCompilePub.opcodePos == OP_GetSelfObject)
		{
			*scrCompilePub.opcodePos = OP_EvalSelfFieldVariableRef;
			return;
		}

		if (*scrCompilePub.opcodePos == OP_GetLevelObject)
		{
			*scrCompilePub.opcodePos = OP_EvalLevelFieldVariableRef;
			return;
		}

		if (*scrCompilePub.opcodePos != OP_GetAnimObject)
		{
			break;
		}

		*scrCompilePub.opcodePos = OP_EvalAnimFieldVariableRef;
		return;

	case OP_SafeSetVariableFieldCached0:
		if (*scrCompilePub.opcodePos != OP_CreateLocalVariable)
		{
			break;
		}

		*scrCompilePub.opcodePos = OP_SafeCreateVariableFieldCached;
		return;

	case OP_SetVariableField:
		switch (*scrCompilePub.opcodePos)
		{
		case OP_EvalLocalVariableRefCached:
			RemoveOpcodePos();
			*scrCompilePub.opcodePos = OP_SetLocalVariableFieldCached;
			return;

		case OP_EvalLocalVariableRefCached0:
			RemoveOpcodePos();
			*scrCompilePub.opcodePos = OP_SetLocalVariableFieldCached0;
			return;

		case OP_EvalSelfFieldVariableRef:
			RemoveOpcodePos();
			*scrCompilePub.opcodePos = OP_SetSelfFieldVariableField;
			return;

		case OP_EvalLevelFieldVariableRef:
			RemoveOpcodePos();
			*scrCompilePub.opcodePos = OP_SetLevelFieldVariableField;
			return;
		}

		if (*scrCompilePub.opcodePos != OP_EvalAnimFieldVariableRef)
		{
			break;
		}

		RemoveOpcodePos();
		*scrCompilePub.opcodePos = OP_SetAnimFieldVariableField;
		return;

	case OP_ScriptFunctionCall:
		if (*scrCompilePub.opcodePos != OP_PreScriptCall)
		{
			break;
		}

		*scrCompilePub.opcodePos = OP_ScriptFunctionCall2;
		return;

	case OP_ScriptMethodCall:
		if (*scrCompilePub.opcodePos != OP_GetSelf)
		{
			break;
		}

		RemoveOpcodePos();
		*scrCompilePub.opcodePos = OP_ScriptFunctionCall;
		iassert(scrCompileGlob.prevOpcodePos);

		if (*scrCompileGlob.prevOpcodePos != OP_PreScriptCall)
		{
			return;
		}

		iassert(scrCompilePub.opcodePos == (byte *)TempMalloc(0) - 1);
		TempMemorySetPos((char *)scrCompilePub.opcodePos);
		--scrCompilePub.opcodePos;
		scrCompileGlob.prevOpcodePos = NULL;
		scrCompileGlob.codePos = scrCompilePub.opcodePos;
		*scrCompilePub.opcodePos = OP_ScriptFunctionCall2;
		return;

	case OP_ScriptMethodThreadCall:
		if (*scrCompilePub.opcodePos != OP_GetSelf)
		{
			break;
		}

		RemoveOpcodePos();
		*scrCompilePub.opcodePos = OP_ScriptThreadCall;
		return;

	case OP_CastFieldObject:
		if (*scrCompilePub.opcodePos == OP_EvalLocalVariableCached)
		{
			*scrCompilePub.opcodePos = OP_EvalLocalVariableObjectCached;
			return;
		}

		index = *scrCompilePub.opcodePos - OP_EvalLocalVariableCached0;

		if (index > OP_GetNegByte)
		{
			break;
		}

		*scrCompilePub.opcodePos = OP_EvalLocalVariableObjectCached;
		EmitByte(index);
		return;

	case OP_JumpOnFalse:
		if (*scrCompilePub.opcodePos != OP_BoolNot)
		{
			break;
		}

		RemoveOpcodePos();
		*scrCompilePub.opcodePos = OP_JumpOnTrue;
		return;
	}

	scrCompileGlob.prevOpcodePos = scrCompilePub.opcodePos;
	scrCompilePub.opcodePos = (byte *)TempMalloc(sizeof(byte));
	scrCompileGlob.codePos = scrCompilePub.opcodePos;
	*scrCompilePub.opcodePos = op;
}

/*
============
Scr_CalcLocalVarsFormalParameterList
============
*/
void Scr_CalcLocalVarsFormalParameterList(sval_u exprlist, scr_block_s *block)
{
	Scr_CalcLocalVarsFormalParameterListInternal(exprlist.node[0].node, block);
}

/*
============
EmitProfStatement
============
*/
void EmitProfStatement(sval_u profileName, sval_u sourcePos, unsigned char op)
{
	if (!scrVarPub.developer_script)
	{
		Scr_CompileRemoveRefToString(profileName.stringValue);
		return;
	}

	Scr_CompileRemoveRefToString(profileName.stringValue);
	EmitOpcode(op, 0, CALL_NONE);
	EmitByte(0);
}

/*
============
Scr_CalcLocalVarsWaittillStatement
============
*/
void Scr_CalcLocalVarsWaittillStatement(sval_u exprlist, scr_block_s *block)
{
	Scr_CalcLocalVarsFormalParameterListInternal(exprlist.node[0].node[1].node, block);
}

/*
============
Scr_CalcLocalVarsIncStatement
============
*/
void Scr_CalcLocalVarsIncStatement(sval_u expr, scr_block_s *block)
{
	Scr_CalcLocalVarsVariableExpressionRef(expr, block);
}

/*
============
EmitWaittillFrameEnd
============
*/
void EmitWaittillFrameEnd(sval_u sourcePos)
{
	EmitOpcode(OP_waittillFrameEnd, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
Scr_CalcLocalVarsAssignmentStatement
============
*/
void Scr_CalcLocalVarsAssignmentStatement(sval_u lhs, sval_u rhs, scr_block_s *block)
{
	Scr_CalcLocalVarsVariableExpressionRef(lhs, block);
}

/*
============
EmitPrimitiveExpression
============
*/
void EmitPrimitiveExpression(sval_u expr, scr_block_s *block)
{
	VariableCompileValue constValue;

	if (EmitOrEvalPrimitiveExpression(expr, &constValue, block))
	{
		EmitValue(&constValue);
	}
}

/*
============
EmitCallBuiltinMethodOpcode
============
*/
void EmitCallBuiltinMethodOpcode(int param_count, sval_u sourcePos)
{
	unsigned int opcode;

	if (param_count > 5)
		opcode = OP_CallBuiltinMethod;
	else
		opcode = OP_CallBuiltinMethod0 + param_count;

	EmitOpcode(opcode, -param_count, CALL_BUILTIN);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);

	if (opcode == OP_CallBuiltinMethod)
	{
		EmitByte(param_count);
	}
}

/*
============
EmitCallBuiltinOpcode
============
*/
void EmitCallBuiltinOpcode(int param_count, sval_u sourcePos)
{
	unsigned int opcode;

	if (param_count > 5)
		opcode = OP_CallBuiltin;
	else
		opcode = OP_CallBuiltin0 + param_count;

	EmitOpcode(opcode, 1 - param_count, CALL_BUILTIN);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);

	if (opcode == OP_CallBuiltin)
	{
		EmitByte(param_count);
	}
}

/*
============
EmitPreFunctionCall
============
*/
void EmitPreFunctionCall(sval_u func_name)
{
	if (func_name.node[0].type == ENUM_script_call)
	{
		EmitOpcode(OP_PreScriptCall, 1, CALL_NONE);
	}
}

/*
============
EmitPostScriptThreadPointer
============
*/
void EmitPostScriptThreadPointer(sval_u expr, int param_count, bool bMethod, sval_u sourcePos, scr_block_s *block)
{
	EmitExpression(expr, block);

	if (bMethod)
		EmitOpcode(OP_ScriptMethodThreadCallPointer, -param_count - 1, CALL_THREAD);
	else
		EmitOpcode(OP_ScriptThreadCallPointer, -param_count, CALL_THREAD);

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	EmitCodepos((const char *)param_count);
}

/*
============
EmitPostScriptFunctionPointer
============
*/
void EmitPostScriptFunctionPointer(sval_u expr, int param_count, bool bMethod, sval_u nameSourcePos, sval_u sourcePos, scr_block_s *block)
{
	EmitExpression(expr, block);

	if (bMethod)
		EmitOpcode(OP_ScriptMethodCallPointer, -param_count - 2, CALL_FUNCTION);
	else
		EmitOpcode(OP_ScriptFunctionCallPointer, -param_count - 1, CALL_FUNCTION);

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(nameSourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitCastFieldObject
============
*/
void EmitCastFieldObject(sval_u sourcePos)
{
	EmitOpcode(OP_CastFieldObject, -1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitDecTop
============
*/
void EmitDecTop()
{
	EmitOpcode(OP_DecTop, -1, CALL_NONE);
}

/*
============
EmitEmptyArray
============
*/
void EmitEmptyArray(sval_u sourcePos)
{
	EmitOpcode(OP_EmptyArray, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitClearArray
============
*/
void EmitClearArray(sval_u sourcePos, sval_u indexSourcePos)
{
	EmitOpcode(OP_ClearArray, -1, CALL_NONE);
	AddOpcodePos(indexSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitEvalArrayRef
============
*/
void EmitEvalArrayRef(sval_u sourcePos, sval_u indexSourcePos)
{
	EmitOpcode(OP_EvalArrayRef, -1, CALL_NONE);
	AddOpcodePos(indexSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitEvalArray
============
*/
void EmitEvalArray(sval_u sourcePos, sval_u indexSourcePos)
{
	EmitOpcode(OP_EvalArray, -1, CALL_NONE);
	AddOpcodePos(indexSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitGameRef
============
*/
void EmitGameRef(sval_u sourcePos)
{
	EmitOpcode(OP_GetGameRef, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitAnimObject
============
*/
void EmitAnimObject(sval_u sourcePos)
{
	EmitOpcode(OP_GetAnimObject, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitLevelObject
============
*/
void EmitLevelObject(sval_u sourcePos)
{
	EmitOpcode(OP_GetLevelObject, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitSelfObject
============
*/
void EmitSelfObject(sval_u sourcePos)
{
	EmitOpcode(OP_GetSelfObject, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitAnim
============
*/
void EmitAnim(sval_u sourcePos)
{
	EmitOpcode(OP_GetAnim, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitGame
============
*/
void EmitGame(sval_u sourcePos)
{
	EmitOpcode(OP_GetGame, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitLevel
============
*/
void EmitLevel(sval_u sourcePos)
{
	EmitOpcode(OP_GetLevel, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitSelf
============
*/
void EmitSelf(sval_u sourcePos)
{
	EmitOpcode(OP_GetSelf, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitSize
============
*/
void EmitSize(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	EmitPrimitiveExpression(expr, block);
	EmitOpcode(OP_size, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitBoolComplement
============
*/
void EmitBoolComplement(sval_u sourcePos)
{
	EmitOpcode(OP_BoolComplement, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitBoolNot
============
*/
void EmitBoolNot(sval_u sourcePos)
{
	EmitOpcode(OP_BoolNot, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitCastBool
============
*/
void EmitCastBool(sval_u sourcePos)
{
	EmitOpcode(OP_CastBool, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitRemoveLocalVars
============
*/
void EmitRemoveLocalVars(scr_block_s *block, scr_block_s *outerBlock)
{
	if (block->abortLevel != SCR_ABORT_NONE)
	{
		return;
	}

	iassert(block->localVarsCreateCount >= block->localVarsPublicCount);
	iassert(block->localVarsPublicCount >= outerBlock->localVarsPublicCount);

	int removeCount = block->localVarsCreateCount - outerBlock->localVarsPublicCount;
	iassert(removeCount >= 0);

	if (!removeCount)
	{
		return;
	}

	EmitOpcode(OP_RemoveLocalVariables, 0, CALL_NONE);
	EmitByte(removeCount);
	block->localVarsCreateCount = block->localVarsPublicCount;
}

/*
============
EmitCreateLocalVars
============
*/
void EmitCreateLocalVars(scr_block_s *block)
{
	iassert(block->localVarsPublicCount >= block->localVarsCreateCount);

	if (block->localVarsCreateCount == block->localVarsPublicCount)
	{
		return;
	}

	for (int i = block->localVarsCreateCount; i < block->localVarsPublicCount; i++)
	{
		EmitOpcode(OP_CreateLocalVariable, 0, CALL_NONE);
		EmitCanonicalStringConst(block->localVars[i].name);
	}

	block->localVarsCreateCount = block->localVarsPublicCount;
}

/*
============
EmitSetVariableField
============
*/
void EmitSetVariableField(sval_u sourcePos)
{
	EmitOpcode(OP_SetVariableField, -1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitReturn
============
*/
void EmitReturn(void)
{
	EmitOpcode(OP_Return, -1, CALL_NONE);
}

/*
============
EmitEnd
============
*/
void EmitEnd()
{
	EmitOpcode(OP_End, 0, CALL_NONE);
}

/*
============
InitThread
============
*/
void InitThread(int type)
{
	scrCompileGlob.currentCaseStatement = 0;
	scrCompileGlob.bCanBreak = 0;
	scrCompileGlob.currentBreakStatement = 0;
	scrCompileGlob.bCanContinue = 0;
	scrCompileGlob.currentContinueStatement = 0;
	scrCompileGlob.breakChildBlocks = 0;
	scrCompileGlob.continueChildBlocks = 0;

	if (!scrCompileGlob.firstThread[type])
	{
		return;
	}

	scrCompileGlob.firstThread[type] = false;
	EmitEnd();

	AddOpcodePos(0, SOURCE_TYPE_NONE);
	AddOpcodePos(0xFFFFFFFE, SOURCE_TYPE_NONE);
}

/*
============
EmitProfEndStatement
============
*/
void EmitProfEndStatement(sval_u profileName, sval_u sourcePos)
{
	EmitProfStatement(profileName, sourcePos, OP_prof_end);
}

/*
============
EmitProfBeginStatement
============
*/
void EmitProfBeginStatement(sval_u profileName, sval_u sourcePos)
{
	EmitProfStatement(profileName, sourcePos, OP_prof_begin);
}

/*
============
EmitEndStatement
============
*/
void EmitEndStatement(sval_u sourcePos, scr_block_s *block)
{
	if (block->abortLevel == SCR_ABORT_NONE)
	{
		block->abortLevel = SCR_ABORT_RETURN;
	}

	EmitEnd();
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitBoolAndExpression
============
*/
void EmitBoolAndExpression(sval_u expr1, sval_u expr2, sval_u expr1sourcePos, sval_u expr2sourcePos, scr_block_s *block)
{
	const char *nextPos, *pos;
	unsigned int offset;

	EmitExpression(expr1, block);

	EmitOpcode(OP_JumpOnFalseExpr, -1, CALL_NONE);
	AddOpcodePos(expr1sourcePos.sourcePosValue, SOURCE_TYPE_NONE);

	EmitUnsignedShort(0);

	pos = (const char *)scrCompileGlob.codePos;
	nextPos = TempMallocAlignStrict(0);

	EmitExpression(expr2, block);
	EmitCastBool(expr2sourcePos);

	offset = TempMallocAlignStrict(0) - nextPos;
	iassert(offset < 65536);

	*(unsigned short *)pos = offset;
}

/*
============
EmitBoolOrExpression
============
*/
void EmitBoolOrExpression(sval_u expr1, sval_u expr2, sval_u expr1sourcePos, sval_u expr2sourcePos, scr_block_s *block)
{
	const char *nextPos, *pos;
	unsigned int offset;

	EmitExpression(expr1, block);

	EmitOpcode(OP_JumpOnTrueExpr, -1, CALL_NONE);
	AddOpcodePos(expr1sourcePos.stringValue, SOURCE_TYPE_NONE);

	EmitUnsignedShort(0);

	pos = (const char *)scrCompileGlob.codePos;
	nextPos = TempMallocAlignStrict(0);

	EmitExpression(expr2, block);
	EmitCastBool(expr2sourcePos);

	offset = TempMallocAlignStrict(0) - nextPos;
	iassert(offset < 65536);

	*(unsigned short *)pos = offset;
}

/*
============
EmitArrayVariable
============
*/
void EmitArrayVariable(sval_u expr, sval_u index, sval_u sourcePos, sval_u indexSourcePos, scr_block_s *block)
{
	EmitExpression(index, block);
	EmitPrimitiveExpression(expr, block);
	EmitEvalArray(sourcePos, indexSourcePos);
}

/*
============
EmitNOP2
============
*/
void EmitNOP2(bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
	int checksum = scrVarPub.checksum;

	if (lastStatement)
	{
		EmitEnd();
		AddOpcodePos(endSourcePos, SOURCE_TYPE_BREAKPOINT);
	}
	else
	{
		EmitRemoveLocalVars(block, block);
	}

	scrVarPub.checksum = checksum + 1;
}

/*
============
EmitDeveloperStatementList
============
*/
void EmitDeveloperStatementList(sval_u val, sval_u sourcePos, scr_block_s *block, sval_u *devStatBlock)
{
	char *savedPos;
	unsigned int savedChecksum;

	if (scrCompilePub.developer_statement != SCR_DEV_NO)
	{
		CompileError(sourcePos.sourcePosValue, "cannot recurse /#");
		return;
	}

	savedChecksum = scrVarPub.checksum;
	Scr_TransferBlock(block, devStatBlock->block);

	if (scrVarPub.developer_script)
	{
		scrCompilePub.developer_statement = SCR_DEV_YES;
		EmitStatementList(val, false, 0, devStatBlock->block);
		EmitRemoveLocalVars(devStatBlock->block, devStatBlock->block);
	}
	else
	{
		savedPos = TempMalloc(0);
		scrCompilePub.developer_statement = SCR_DEV_IGNORE;
		EmitStatementList(val, false, 0, devStatBlock->block);
		TempMemorySetPos(savedPos);
	}

	scrCompilePub.developer_statement = SCR_DEV_NO;
	scrVarPub.checksum = savedChecksum;
}

/*
============
EmitContinueStatement
============
*/
void EmitContinueStatement(sval_u sourcePos, scr_block_s *block)
{
	ContinueStatementInfo *newContinueStatement;

	if (!scrCompileGlob.bCanContinue || block->abortLevel != SCR_ABORT_NONE)
	{
		CompileError(sourcePos.stringValue, "illegal continue statement");
		return;
	}

	Scr_AddContinueBlock(block);
	EmitRemoveLocalVars(block, block);

	block->abortLevel = SCR_ABORT_CONTINUE;

	EmitOpcode(OP_jump, 0, CALL_NONE);
	AddOpcodePos(sourcePos.stringValue, SOURCE_TYPE_BREAKPOINT);

	EmitCodepos(0);

	//newContinueStatement = (ContinueStatementInfo *)Hunk_AllocateTempMemoryHighInternal(sizeof(*newContinueStatement));
	newContinueStatement = (ContinueStatementInfo*)Hunk_AllocateTempMemoryHigh(sizeof(ContinueStatementInfo), "EmitContinueStatement");
	newContinueStatement->codePos = (char *)scrCompileGlob.codePos;
	newContinueStatement->nextCodePos = TempMalloc(0);
	newContinueStatement->next = scrCompileGlob.currentContinueStatement;

	scrCompileGlob.currentContinueStatement = newContinueStatement;
}

/*
============
EmitBreakStatement
============
*/
void EmitBreakStatement(sval_u sourcePos, scr_block_s *block)
{
	BreakStatementInfo *newBreakStatement;

	if (!scrCompileGlob.bCanBreak || block->abortLevel != SCR_ABORT_NONE)
	{
		CompileError(sourcePos.sourcePosValue, "illegal break statement");
		return;
	}

	iassert(scrCompileGlob.breakBlock);
	Scr_AddBreakBlock(block);
	EmitRemoveLocalVars(block, scrCompileGlob.breakBlock);

	block->abortLevel = SCR_ABORT_BREAK;

	EmitOpcode(OP_jump, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);

	EmitCodepos(0);

	//newBreakStatement = (BreakStatementInfo *)Hunk_AllocateTempMemoryHighInternal(sizeof(*newBreakStatement));
	newBreakStatement = (BreakStatementInfo *)Hunk_AllocateTempMemoryHigh(sizeof(BreakStatementInfo), "EmitBreakStatement");
	newBreakStatement->codePos = (char *)scrCompileGlob.codePos;
	newBreakStatement->nextCodePos = TempMalloc(0);
	newBreakStatement->next = scrCompileGlob.currentBreakStatement;

	scrCompileGlob.currentBreakStatement = newBreakStatement;
}

/*
============
EmitCaseStatement
============
*/
void EmitCaseStatement(sval_u expr, sval_u sourcePos)
{
	unsigned int name;

	if (expr.node[0].type == ENUM_integer)
	{
		if (!IsValidArrayIndex(expr.node[1].intValue))
		{
			CompileError(sourcePos.sourcePosValue, va("case index %d out of range", expr.node[1].intValue));
			return;
		}

		name = GetInternalVariableIndex(expr.node[1].intValue);
	}
	else
	{
		if (expr.node[0].type != ENUM_string)
		{
			CompileError(sourcePos.sourcePosValue, "case expression must be an int or string");
			return;
		}

		name = expr.node[1].stringValue;
		CompileTransferRefToString(name, 1);
	}

	EmitCaseStatementInfo(name, sourcePos);
}

/*
============
EmitExpressionFieldObject
============
*/
void EmitExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	if (expr.node[0].type == ENUM_primitive_expression)
	{
		EmitPrimitiveExpressionFieldObject(expr.node[1], expr.node[2], block);
		return;
	}

	CompileError(sourcePos.sourcePosValue, "not an object");
}

/*
============
EmitArrayPrimitiveExpressionRef
============
*/
void EmitArrayPrimitiveExpressionRef(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	if (expr.node[0].type == ENUM_variable)
	{
		EmitVariableExpressionRef(expr.node[1], block);
		return;
	}

	if (expr.node[0].type == ENUM_game)
	{
		EmitGameRef(expr.node[1]);
		return;
	}

	CompileError(sourcePos.sourcePosValue, "not an lvalue");
}

/*
============
EvalBinaryOperatorExpression
============
*/
bool EvalBinaryOperatorExpression(sval_u expr1, sval_u expr2, sval_u opcode, sval_u sourcePos, VariableCompileValue *constValue)
{
	VariableCompileValue constValue1;
	VariableCompileValue constValue2;

	if (!EvalExpression(expr1, &constValue1))
	{
		return false;
	}

	if (!EvalExpression(expr2, &constValue2))
	{
		return false;
	}

	//AddRefToValue(&constValue1.value);
	//AddRefToValue(&constValue2.value);

	AddRefToValue(constValue1.value.type, constValue1.value.u);
	AddRefToValue(constValue2.value.type, constValue2.value.u);

	Scr_EvalBinaryOperator(opcode.type, &constValue1.value, &constValue2.value);

	if (scrVarPub.error_message)
	{
		CompileError(sourcePos.sourcePosValue, "%s", scrVarPub.error_message);
		return false;
	}

	constValue->value.u = constValue1.value.u;
	constValue->value.type = constValue1.value.type;
	constValue->sourcePos = sourcePos;

	return true;
}

/*
============
EmitExpressionListFieldObject
============
*/
void EmitExpressionListFieldObject(sval_u exprlist, sval_u sourcePos, scr_block_s *block)
{
	sval_u *node = GetSingleParameter(exprlist);

	if (node)
	{
		EmitExpressionFieldObject(node[0].node[0], node[0].node[1], block);
		return;
	}

	CompileError(sourcePos.sourcePosValue, "not an object");
}

/*
============
Scr_CreateVector
============
*/
void Scr_CreateVector(VariableCompileValue *constValue, VariableValue *value)
{
	float vec[3];

	for (int i = 0; i < 3; i++)
	{
		switch (constValue[i].value.type)
		{
		case VAR_FLOAT:
			vec[2 - i] = constValue[i].value.u.floatValue;
			break;

		case VAR_INTEGER:
			vec[2 - i] = (float)constValue[i].value.u.intValue;
			break;

		default:
			CompileError(constValue[i].sourcePos.sourcePosValue, "type %s is not a float", var_typename[constValue[i].value.type]);
			return;
		}
	}

	value->type = VAR_VECTOR;
	value->u.vectorValue = Scr_AllocVector(vec);
}

/*
============
SpecifyThreadPosition
============
*/
unsigned int SpecifyThreadPosition(unsigned int posId, unsigned int name, unsigned int sourcePos, int type)
{
	unsigned int id;
	VariableValue pos;

	id = GetVariable(posId, 1);
	pos = Scr_EvalVariable(id);

	if (pos.type == VAR_UNDEFINED)
	{
		pos.type = (Vartype_t)type;
		pos.u.intValue = 0;

		SetNewVariableValue(id, &pos);
		return id;
	}

	if (pos.u.intValue)
		CompileError(sourcePos, "function '%s' already defined in '%s'", SL_ConvertToString(name), scrParserPub.sourceBufferLookup[Scr_GetSourceBuffer(pos.u.codePosValue)].buf);
	else
		CompileError(sourcePos, "function '%s' already defined", SL_ConvertToString(name));

	return 0;
}

/*
============
LinkThread
============
*/
void LinkThread(unsigned int threadCountId, VariableValue *pos, bool allowFarCall)
{
	int type, i;
	VariableUnion *value;
	unsigned int valueId, countId;
	VariableValue count;

	countId = FindVariable(threadCountId, 0);

	if (!countId)
	{
		return;
	}

	count = Scr_EvalVariable(countId);
	iassert(count.type == VAR_INTEGER);

	for (i = 0; i < count.u.intValue; i++)
	{
		valueId = FindVariable(threadCountId, i + 2);
		iassert(valueId);

		value = &GetVariableValueAddress(valueId)->u;

		type = GetValueType(valueId);
		iassert(type == VAR_CODEPOS || type == VAR_DEVELOPER_CODEPOS);
		//iassert(scrVarPub.developer_script);

		if (pos->type == VAR_DEVELOPER_CODEPOS && type == VAR_CODEPOS)
		{
			CompileError2((char*)value->codePosValue, "normal script cannot reference a function in a /# ... #/ comment");
		}

		if (pos->type == VAR_UNDEFINED)
		{
			CompileError2((char *)value->codePosValue, "unknown function");
		}

		if (!allowFarCall && *(intptr_t *)value->codePosValue == FUNC_SCOPE_FAR)
		{
			CompileError2((char *)value->codePosValue, "unknown function");
		}

		*(const char **)value->codePosValue = pos->u.codePosValue;
	}
}

/*
============
EmitFunction
============
*/
void EmitFunction(sval_u func, sval_u sourcePos)
{
	unsigned int threadId, valueId, filename, fileId, posId, countId;
	int scope;
	VariableValue value, pos, count;
	bool bExists;

	if (scrCompilePub.developer_statement == SCR_DEV_IGNORE)
	{
		Scr_CompileRemoveRefToString(func.node[1].stringValue);

		if (func.node[0].type == ENUM_far_function)
		{
			Scr_CompileRemoveRefToString(func.node[2].stringValue);
			scrCompilePub.far_function_count--;
		}

		return;
	}

	threadId = 0;

	if (func.node[0].type == ENUM_local_function)
	{
		scope = FUNC_SCOPE_LOCAL;
		valueId = GetVariable(scrCompileGlob.fileId, func.node[1].idValue);

		CompileTransferRefToString(func.node[1].stringValue, 2);

		threadId = GetObject(valueId);
		iassert(threadId);
	}
	else
	{
		iassert(func.node[0].type == ENUM_far_function);
		scope = FUNC_SCOPE_FAR;

		filename = Scr_CreateCanonicalFilename(SL_ConvertToString(func.node[1].stringValue));
		Scr_CompileRemoveRefToString(func.node[1].stringValue);

		value = Scr_EvalVariable(FindVariable(scrCompilePub.loadedscripts, filename));
		bExists = value.type != VAR_UNDEFINED;

		fileId = AddFilePrecache(filename, sourcePos.sourcePosValue, false);

		if (bExists)
		{
			valueId = FindVariable(fileId, func.node[2].idValue);

			if (!valueId || GetValueType(valueId) != VAR_POINTER)
			{
				CompileError(sourcePos.sourcePosValue, "unknown function");
				return;
			}
		}
		else
		{
			valueId = GetVariable(fileId, func.node[2].idValue);
		}

		CompileTransferRefToString(func.node[2].stringValue, 2);

		threadId = GetObject(valueId);
		posId = FindVariable(threadId, 1);

		if (posId)
		{
			pos = Scr_EvalVariable(posId);
			iassert((pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS));
			//iassert(pos.u.codePosValue);

			if (pos.type == VAR_INCLUDE_CODEPOS)
			{
				CompileError(sourcePos.sourcePosValue, "unknown function");
				return;
			}

			if (pos.u.codePosValue)
			{
				if (pos.type == VAR_CODEPOS || scrCompilePub.developer_statement != SCR_DEV_NO)
				{
					EmitCodepos(pos.u.codePosValue);
				}
				else
				{
					CompileError(sourcePos.sourcePosValue, "normal script cannot reference a function in a /# ... #/ comment");
				}

				return;
			}
		}
	}

	EmitCodepos((const char *)scope);

	countId = GetVariable(threadId, 0);
	count = Scr_EvalVariable(countId);
	iassert((count.type == VAR_UNDEFINED) || (count.type == VAR_INTEGER));

	if (count.type == VAR_UNDEFINED)
	{
		count.type = VAR_INTEGER;
		count.u.intValue = 0;
	}

	valueId = GetNewVariable(threadId, count.u.intValue + 2);
	value.u.codePosValue = (const char *)scrCompileGlob.codePos;

	if (scrCompilePub.developer_statement != SCR_DEV_NO)
	{
		iassert(scrVarPub.developer_script);
		value.type = VAR_DEVELOPER_CODEPOS;
	}
	else
	{
		value.type = VAR_CODEPOS;
	}

	SetNewVariableValue(valueId, &value);
	count.u.intValue++;

	SetVariableValue(countId, &count);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitObject
============
*/
void EmitObject(sval_u expr, sval_u sourcePos)
{
	signed int ObjectType; // [esp+0h] [ebp-18h]
	int classnum; // [esp+4h] [ebp-14h]
	char *s; // [esp+Ch] [ebp-Ch]
	int entnum; // [esp+10h] [ebp-8h]
	unsigned int idValue; // [esp+14h] [ebp-4h]

	if (scrCompilePub.script_loading)
	{
		CompileError(sourcePos.stringValue, "$ can only be used in the script debugger");
		return;
	}
	s = SL_ConvertToString(expr.stringValue);
	if (*s == 116)
	{
		idValue = atoi(s + 1);
		if (idValue)
		{
			if (idValue < 0x8000 && !IsObjectFree(idValue))
			{
				ObjectType = GetObjectType(idValue);
				if (ObjectType >= 14 && (ObjectType <= 17 || ObjectType == 22))
				{
					EmitOpcode(0x82u, 1, 0);
					EmitShort(idValue);
					return;
				}
			}
		}
	LABEL_17:
		CompileError(sourcePos.stringValue, "bad expression");
		return;
	}
	if (*s == 97)
	{
		CompileError(sourcePos.stringValue, "argument expressions not supported in statements");
		return;
	}
	classnum = Scr_GetClassnumForCharId(*s);
	if (classnum < 0)
		goto LABEL_17;
	entnum = atoi(s + 1);
	if (!entnum && s[1] != 48)
		goto LABEL_17;
	EmitOpcode(0x81u, 1, 0);
	EmitCodepos((const char*)classnum);
	EmitCodepos((const char*)entnum);
}

/*
============
Scr_PushValue
============
*/
void Scr_PushValue(VariableCompileValue *constValue)
{
	if (scrCompilePub.value_count >= VALUE_STACK_SIZE)
	{
		CompileError(constValue->sourcePos.sourcePosValue, "VALUE_STACK_SIZE exceeded");
		return;
	}

	int valueIndex = scrCompilePub.value_count;

	scrCompileGlob.value_start[valueIndex].value.u = constValue->value.u;
	scrCompileGlob.value_start[valueIndex].value.type = constValue->value.type;
	scrCompileGlob.value_start[valueIndex].sourcePos.sourcePosValue = constValue->sourcePos.sourcePosValue;

	scrCompilePub.value_count++;
}

/*
============
Scr_FindLocalVarIndex
============
*/
int Scr_FindLocalVarIndex(unsigned int name, sval_u sourcePos, bool create, scr_block_s *block)
{
	int i;

	iassert(scrCompilePub.developer_statement != SCR_DEV_EVALUATE);

	if (!block)
	{
		CompileError(sourcePos.sourcePosValue, "unreachable code");
		return 0;
	}

	for (i = 0; ; i++)
	{
		if (i >= block->localVarsCount)
		{
			if (!create || scrCompileGlob.forceNotCreate)
			{
				CompileError(sourcePos.sourcePosValue, "uninitialised variable '%s'", SL_ConvertToString(name));
				return 0;
			}

			CompileError(sourcePos.sourcePosValue, "unreachable code");
			return 0;
		}

		if (i == block->localVarsCreateCount)
		{
			block->localVarsCreateCount++;

			EmitOpcode(OP_CreateLocalVariable, 0, CALL_NONE);
			EmitCanonicalStringConst(block->localVars[i].name);
		}

		if (block->localVars[i].name == name)
		{
			break;
		}
	}

	Scr_CompileRemoveRefToString(name);

	if (block->localVarsInitBits[i >> 3] & 1 << (i & 7))
	{
		iassert((block->localVarsCreateCount - 1) >= i);
		return block->localVarsCreateCount - i - 1;
	}

	if (create && !scrCompileGlob.forceNotCreate)
	{
		block->localVarsInitBits[i >> 3] |= 1 << (i & 7);
		return block->localVarsCreateCount - i - 1;
	}

	if (!create || scrCompileGlob.forceNotCreate)
	{
		CompileError(sourcePos.sourcePosValue, "uninitialised variable '%s'", SL_ConvertToString(name));
		return 0;
	}

	CompileError(sourcePos.sourcePosValue, "unreachable code");
	return 0;
}

/*
============
EmitAnimTree
============
*/
void EmitAnimTree(sval_u sourcePos)
{
	if (!scrAnimPub.animTreeIndex)
	{
		CompileError(sourcePos.sourcePosValue, "#using_animtree was not specified");
		return;
	}

	EmitGetInteger(scrAnimPub.animTreeIndex, sourcePos);
}

/*
============
SpecifyThread
============
*/
void SpecifyThread(sval_u val)
{
	unsigned int posId;

	switch (val.node[0].type)
	{
	case ENUM_begin_developer_thread:
		if (scrCompileGlob.in_developer_thread)
		{
			CompileError(val.node[1].sourcePosValue, "cannot recurse /#");
			return;
		}

		scrCompileGlob.in_developer_thread = true;
		scrCompileGlob.developer_thread_sourcePos = val.node[1].sourcePosValue;
		break;

	case ENUM_end_developer_thread:
		if (!scrCompileGlob.in_developer_thread)
		{
			CompileError(val.node[1].sourcePosValue, "#/ has no matching /#");
			return;
		}

		scrCompileGlob.in_developer_thread = false;
		break;

	case ENUM_thread:
		if (scrCompileGlob.in_developer_thread && !scrVarPub.developer_script)
		{
			return;
		}

		posId = GetObject(GetVariable(scrCompileGlob.fileId, val.node[1].idValue));

		if (scrCompileGlob.in_developer_thread)
			SpecifyThreadPosition(posId, val.node[1].sourcePosValue, val.node[4].sourcePosValue, VAR_DEVELOPER_CODEPOS);
		else
			SpecifyThreadPosition(posId, val.node[1].sourcePosValue, val.node[4].sourcePosValue, VAR_CODEPOS);

		break;
	}
}

/*
============
EmitSwitchStatementList
============
*/
void EmitSwitchStatementList(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
	scr_block_s *oldBreakBlock;
	int *oldBreakChildCount;
	scr_block_s **oldBreakChildBlocks;
	int breakChildCount;
	scr_block_s **breakChildBlocks;
	bool hasDefault;
	sval_u *node, *nextNode;

	oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
	oldBreakChildCount = scrCompileGlob.breakChildCount;
	oldBreakBlock = scrCompileGlob.breakBlock;

	breakChildCount = 0;
	//breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHighInternal(sizeof(scr_block_s **) * MAX_SWITCH_CASES);
	breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s**) * MAX_SWITCH_CASES, "EmitSwitchStatementList");

	scrCompileGlob.breakChildBlocks = breakChildBlocks;
	scrCompileGlob.breakChildCount = &breakChildCount;

	scrCompileGlob.breakBlock = NULL;
	hasDefault = false;

	for (node = val.node[0].node[1].node; node; node = nextNode)
	{
		nextNode = node[1].node;

		if (node[0].node[0].type == ENUM_case || node[0].node[0].type == ENUM_default)
		{
			if (scrCompileGlob.breakBlock)
			{
				iassert(scrCompileGlob.bCanBreak);
				scrCompileGlob.bCanBreak = false;
				EmitRemoveLocalVars(scrCompileGlob.breakBlock, scrCompileGlob.breakBlock);
			}

			if (node[0].node[0].type == ENUM_case)
			{
				scrCompileGlob.breakBlock = node[0].node[3].block;
				EmitCaseStatement(node[0].node[1], node[0].node[2]);
			}

			else
			{
				scrCompileGlob.breakBlock = node[0].node[2].block;
				hasDefault = true;
				EmitDefaultStatement(node[0].node[1]);
			}

			Scr_TransferBlock(block, scrCompileGlob.breakBlock);
			iassert(!scrCompileGlob.bCanBreak);
			scrCompileGlob.bCanBreak = true;
		}
		else
		{
			if (!scrCompileGlob.breakBlock)
			{
				CompileError(endSourcePos, "missing case statement");
				return;
			}

			if (lastStatement && Scr_IsLastStatement(nextNode))
				EmitStatement(node[0], true, endSourcePos, scrCompileGlob.breakBlock);
			else
				EmitStatement(node[0], false, endSourcePos, scrCompileGlob.breakBlock);

			if (scrCompileGlob.breakBlock && scrCompileGlob.breakBlock->abortLevel)
			{
				scrCompileGlob.breakBlock = NULL;
				iassert(scrCompileGlob.bCanBreak);
				scrCompileGlob.bCanBreak = false;
			}
		}
	}

	if (scrCompileGlob.breakBlock)
	{
		iassert(scrCompileGlob.bCanBreak);
		scrCompileGlob.bCanBreak = false;
		EmitRemoveLocalVars(scrCompileGlob.breakBlock, scrCompileGlob.breakBlock);
	}

	if (hasDefault)
	{
		if (scrCompileGlob.breakBlock)
		{
			Scr_AddBreakBlock(scrCompileGlob.breakBlock);
		}

		Scr_InitFromChildBlocks(breakChildBlocks, breakChildCount, block);
	}

	scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
	scrCompileGlob.breakChildCount = oldBreakChildCount;
	scrCompileGlob.breakBlock = oldBreakBlock;
}

/*
============
EmitOrEvalBinaryOperatorExpression
============
*/
bool EmitOrEvalBinaryOperatorExpression(sval_u expr1, sval_u expr2, sval_u opcode, sval_u sourcePos, VariableCompileValue *constValue, scr_block_s *block)
{
	VariableCompileValue constValue1, constValue2;

	if (!EmitOrEvalExpression(expr1, &constValue1, block))
	{
		EmitExpression(expr2, block);

		EmitOpcode(opcode.type, -1, CALL_NONE);
		AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);

		return false;
	}

	Scr_PushValue(&constValue1);

	if (!EmitOrEvalExpression(expr2, &constValue2, block))
	{
		EmitOpcode(opcode.type, -1, CALL_NONE);
		AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);

		return false;
	}

	Scr_PopValue();
	Scr_EvalBinaryOperator(opcode.intValue, &constValue1.value, &constValue2.value);

	if (scrVarPub.error_message)
	{
		CompileError(sourcePos.sourcePosValue, "%s", scrVarPub.error_message);
		return false;
	}

	constValue->value.u = constValue1.value.u;
	constValue->value.type = constValue1.value.type;
	constValue->sourcePos = sourcePos;

	return true;
}

/*
============
EmitOrEvalPrimitiveExpressionList
============
*/
bool EmitOrEvalPrimitiveExpressionList(sval_u exprlist, sval_u sourcePos, VariableCompileValue *constValue, scr_block_s *block)
{
	VariableCompileValue constValue2;
	bool success;
	sval_u *node;

	iassert(constValue);
	int expr_count = GetExpressionCount(exprlist);

	if (expr_count == 1)
	{
		return EmitOrEvalExpression(exprlist.node[0].node[0].node[0], constValue, block);
	}

	if (expr_count != 3)
	{
		CompileError(sourcePos.sourcePosValue, "expression list must have 1 or 3 parameters");
		return 0;
	}

	success = true;

	for (node = exprlist.node[0].node; node; node = node[1].node)
	{
		if (success)
		{
			success = EmitOrEvalExpression(node[0].node[0], &constValue2, block);

			if (success)
			{
				Scr_PushValue(&constValue2);
			}
		}
		else
		{
			EmitExpression(node[0].node[0], block);
		}
	}

	if (success)
	{
		iassert(scrCompilePub.value_count >= 3);
		scrCompilePub.value_count -= 3;
		Scr_CreateVector(&scrCompileGlob.value_start[scrCompilePub.value_count], &constValue->value);
		constValue->sourcePos = sourcePos;
		return true;
	}

	EmitOpcode(OP_vector, -2, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	AddExpressionListOpcodePos(exprlist);
	return false;
}

/*
============
EvalPrimitiveExpressionList
============
*/
bool EvalPrimitiveExpressionList(sval_u exprlist, sval_u sourcePos, VariableCompileValue *constValue)
{
	VariableCompileValue constValue2[3];
	sval_u *node;
	int i;

	iassert(constValue);
	int expr_count = GetExpressionCount(exprlist);

	if (expr_count == 1)
	{
		return EvalExpression(exprlist.node[0].node[0].node[0], constValue);
	}

	if (expr_count != 3)
	{
		return false;
	}

	for (i = 0, node = exprlist.node[0].node; node; node = node[1].node, i++)
	{
		if (!EvalExpression(node[0].node[0], &constValue2[i]))
		{
			return false;
		}
	}

	Scr_CreateVector(constValue2, &constValue->value);
	constValue->sourcePos = sourcePos;

	return true;
}

/*
============
LinkFile
============
*/
void LinkFile(unsigned int fileId)
{
	VariableValue pos, emptyValue;
	unsigned int posId, threadCountId, threadCountPtr;

	emptyValue.type = VAR_UNDEFINED;
	emptyValue.u.intValue = 0;

	for (threadCountPtr = FindNextSibling(fileId); threadCountPtr; threadCountPtr = FindNextSibling(threadCountPtr))
	{
		threadCountId = FindObject(threadCountPtr);
		iassert(threadCountId);

		posId = FindVariable(threadCountId, 1);

		if (!posId)
		{
			LinkThread(threadCountId, &emptyValue, true);
			continue;
		}

		pos = Scr_EvalVariable(posId);

		if (pos.type == VAR_INCLUDE_CODEPOS)
		{
			SetVariableValue(threadCountPtr, &emptyValue);
			continue;
		}

		iassert(pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS);
		iassert(pos.u.codePosValue);

		LinkThread(threadCountId, &pos, true);
	}
}

/*
============
EmitPostScriptThread
============
*/
void EmitPostScriptThread(sval_u func, int param_count, bool bMethod, sval_u sourcePos)
{
	if (bMethod)
		EmitOpcode(OP_ScriptMethodThreadCall, -param_count, CALL_THREAD);
	else
		EmitOpcode(OP_ScriptThreadCall, 1 - param_count, CALL_THREAD);

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT | SOURCE_TYPE_CALL);
	EmitFunction(func, sourcePos);
	EmitCodepos((const char *)param_count);
}

/*
============
EmitPostScriptFunction
============
*/
void EmitPostScriptFunction(sval_u func, int param_count, bool bMethod, sval_u nameSourcePos)
{
	if (bMethod)
		EmitOpcode(OP_ScriptMethodCall, -param_count - 1, CALL_FUNCTION);
	else
		EmitOpcode(OP_ScriptFunctionCall, -param_count, CALL_FUNCTION);

	AddOpcodePos(nameSourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT | SOURCE_TYPE_CALL);
	EmitFunction(func, nameSourcePos);
}

/*
============
EmitGetFunction
============
*/
void EmitGetFunction(sval_u func, sval_u sourcePos)
{
	EmitOpcode(OP_GetFunction, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT | SOURCE_TYPE_CALL);
	EmitFunction(func, sourcePos);
}

/*
============
EmitAnimation
============
*/
void EmitAnimation(sval_u anim, sval_u sourcePos)
{
	EmitOpcode(OP_GetAnimation, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	EmitCodepos((const char *)0xFFFFFFFF);

	Scr_EmitAnimation((char *)scrCompileGlob.codePos, anim.stringValue, sourcePos.sourcePosValue);
	Scr_CompileRemoveRefToString(anim.stringValue);
}

/*
============
EmitLocalVariableRef
============
*/
void EmitLocalVariableRef(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	int index = Scr_FindLocalVarIndex(expr.idValue, sourcePos, true, block);

	if (index)
	{
		EmitOpcode(OP_EvalLocalVariableRefCached, 0, CALL_NONE);
		EmitByte(index);
	}
	else
	{
		EmitOpcode(OP_EvalLocalVariableRefCached0, 0, CALL_NONE);
	}

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitLocalVariable
============
*/
void EmitLocalVariable(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	int index = Scr_FindLocalVarIndex(expr.idValue, sourcePos, false, block);

	if (index > 5)
	{
		EmitOpcode(OP_EvalLocalVariableCached, 1, CALL_NONE);
		EmitByte(index);
	}
	else
	{
		EmitOpcode(OP_EvalLocalVariableCached0 + index, 1, CALL_NONE);
	}

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
}

/*
============
EmitSafeSetWaittillVariableField
============
*/
void EmitSafeSetWaittillVariableField(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	byte index = Scr_FindLocalVarIndex(expr.idValue, sourcePos, true, block);

	EmitOpcode(OP_SafeSetWaittillVariableFieldCached, 0, CALL_NONE);
	EmitByte(index);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitSafeSetVariableField
============
*/
void EmitSafeSetVariableField(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	int index = Scr_FindLocalVarIndex(expr.idValue, sourcePos, true, block);

	if (index)
	{
		EmitOpcode(OP_SafeSetVariableFieldCached, 0, CALL_NONE);
		EmitByte(index);
	}
	else
	{
		EmitOpcode(OP_SafeSetVariableFieldCached0, 0, CALL_NONE);
	}

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitFormalWaittillParameterListRefInternal
============
*/
void EmitFormalWaittillParameterListRefInternal(sval_u *node, scr_block_s *block)
{
	while (1)
	{
		node = node[1].node;

		if (!node)
		{
			break;
		}

		EmitSafeSetWaittillVariableField(node[0].node[0], node[0].node[1], block);
	}
}

/*
============
EmitFormalParameterListInternal
============
*/
void EmitFormalParameterListInternal(sval_u *node, scr_block_s *block)
{
	while (1)
	{
		node = node[1].node;

		if (!node)
		{
			break;
		}

		EmitSafeSetVariableField(node[0].node[0], node[0].node[1], block);
	}
}

/*
============
EvalPrimitiveExpression
============
*/
bool EvalPrimitiveExpression(sval_u expr, VariableCompileValue *constValue)
{
	switch (expr.node[0].type)
	{
	case ENUM_integer:
		EvalInteger(expr.node[1].intValue, expr.node[2], constValue);
		return true;

	case ENUM_float:
		EvalFloat(expr.node[1].floatValue, expr.node[2], constValue);
		return true;

	case ENUM_minus_integer:
		EvalInteger(-expr.node[1].intValue, expr.node[2], constValue);
		return true;

	case ENUM_minus_float:
		EvalFloat(-expr.node[1].floatValue, expr.node[2], constValue);
		return true;

	case ENUM_string:
		EvalString(expr.node[1].stringValue, expr.node[2], constValue);
		return true;

	case ENUM_istring:
		EvalIString(expr.node[1].stringValue, expr.node[2], constValue);
		return true;

	case ENUM_undefined:
		EvalUndefined(expr.node[1], constValue);
		return true;

	case ENUM_expression_list:
		return EvalPrimitiveExpressionList(expr.node[1], expr.node[2], constValue);

	case ENUM_false:
		EvalInteger(false, expr.node[1], constValue);
		return true;

	case ENUM_true:
		EvalInteger(true, expr.node[1], constValue);
		return true;

	default:
		return false;
	}
}

/*
============
EmitPostScriptThreadCall
============
*/
void EmitPostScriptThreadCall(sval_u func_name, int param_count, bool bMethod, sval_u sourcePos, sval_u nameSourcePos, scr_block_s *block)
{
	if (func_name.node[0].type == ENUM_function)
	{
		EmitPostScriptThread(func_name.node[1], param_count, bMethod, nameSourcePos);
	}
	else if (func_name.node[0].type == ENUM_function_pointer)
	{
		EmitPostScriptThreadPointer(func_name.node[1], param_count, bMethod, func_name.node[2], block);
	}

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitPostScriptFunctionCall
============
*/
void EmitPostScriptFunctionCall(sval_u func_name, int param_count, bool bMethod, sval_u nameSourcePos, scr_block_s *block)
{
	if (func_name.node[0].type == ENUM_function)
	{
		EmitPostScriptFunction(func_name.node[1], param_count, bMethod, nameSourcePos);
	}
	else if (func_name.node[0].type == ENUM_function_pointer)
	{
		EmitPostScriptFunctionPointer(func_name.node[1], param_count, bMethod, nameSourcePos, func_name.node[2], block);
	}
}

/*
============
EmitFormalParameterList
============
*/
void EmitFormalParameterList(sval_u exprlist, sval_u sourcePos, scr_block_s *block)
{
	EmitFormalParameterListInternal(exprlist.node[0].node, block);

	EmitOpcode(OP_checkclearparams, 0, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EvalExpression
============
*/
bool EvalExpression(sval_u expr, VariableCompileValue *constValue)
{
	if (expr.node[0].type == ENUM_primitive_expression)
	{
		return EvalPrimitiveExpression(expr.node[1], constValue);
	}

	if (expr.node[0].type == ENUM_binary)
	{
		return EvalBinaryOperatorExpression(expr.node[1], expr.node[2], expr.node[3], expr.node[4], constValue);
	}

	return false;
}

/*
============
EmitPostFunctionCall
============
*/
void EmitPostFunctionCall(sval_u func_name, int param_count, bool bMethod, scr_block_s *block)
{
	if (func_name.node[0].type == ENUM_script_call)
	{
		EmitPostScriptFunctionCall(func_name.node[1], param_count, bMethod, func_name.node[2], block);
	}
	else if (func_name.node[0].type == ENUM_script_thread_call)
	{
		EmitPostScriptThreadCall(func_name.node[1], param_count, bMethod, func_name.node[2], func_name.node[3], block);
	}
}

/*
============
Scr_CalcLocalVarsForStatement
============
*/
void Scr_CalcLocalVarsForStatement(sval_u stmt1, sval_u expr, sval_u stmt2, sval_u stmt, scr_block_s *block, sval_u *forStatBlock, sval_u *forStatPostBlock)
{
	scr_block_s **oldContinueChildBlocks, **continueChildBlocks, **breakChildBlocks, **oldBreakChildBlocks;
	int continueChildCount, breakChildCount, abortLevel, i;
	int *oldBreakChildCount, *oldContinueChildCount;
	VariableCompileValue constValue;
	bool constConditional;

	Scr_CalcLocalVarsStatement(stmt1, block);

	if (expr.node[0].type == ENUM_expression)
	{
		constConditional = false;

		if (EvalExpression(expr.node[1], &constValue))
		{
			if (constValue.value.type == VAR_INTEGER || constValue.value.type == VAR_FLOAT)
			{
				Scr_CastBool(&constValue.value);

				if (constValue.value.u.intValue)
				{
					constConditional = true;
				}
			}

			//RemoveRefToValue(&constValue.value);
			RemoveRefToValue(constValue.value.type, constValue.value.u);
		}
	}
	else
	{
		constConditional = true;
	}

	oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
	oldBreakChildCount = scrCompileGlob.breakChildCount;

	oldContinueChildBlocks = scrCompileGlob.continueChildBlocks;
	oldContinueChildCount = scrCompileGlob.continueChildCount;

	breakChildCount = 0;
	continueChildCount = 0;

	//continueChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHighInternal(sizeof(scr_block_s **) * MAX_SWITCH_CASES);
	continueChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s **) * MAX_SWITCH_CASES, "Scr_CalcLocalVarsForStatement");

	scrCompileGlob.continueChildBlocks = continueChildBlocks;
	scrCompileGlob.continueChildCount = &continueChildCount;

	abortLevel = block->abortLevel;

	if (constConditional)
	{
		//breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHighInternal(sizeof(scr_block_s **) * MAX_SWITCH_CASES);
		breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s **) * MAX_SWITCH_CASES, "Scr_CalcLocalVarsForStatement");
		scrCompileGlob.breakChildCount = &breakChildCount;
	}
	else
	{
		breakChildBlocks = NULL;
	}

	scrCompileGlob.breakChildBlocks = breakChildBlocks;

	Scr_CopyBlock(block, &forStatBlock->block);
	Scr_CopyBlock(block, &forStatPostBlock->block);

	Scr_CalcLocalVarsStatement(stmt, forStatBlock->block);
	Scr_AddContinueBlock(forStatBlock->block);

	for (i = 0; i < continueChildCount; i++)
	{
		Scr_AppendChildBlocks(&continueChildBlocks[i], 1, block);
	}

	Scr_CalcLocalVarsStatement(stmt2, forStatPostBlock->block);

	Scr_AppendChildBlocks(&forStatPostBlock->block, 1, block);
	Scr_MergeChildBlocks(&forStatPostBlock->block, 1, block);

	if (constConditional)
	{
		Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
	}

	Scr_MergeChildBlocks(&forStatBlock->block, 1, block);

	scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
	scrCompileGlob.breakChildCount = oldBreakChildCount;

	scrCompileGlob.continueChildBlocks = oldContinueChildBlocks;
	scrCompileGlob.continueChildCount = oldContinueChildCount;
}

/*
============
Scr_CalcLocalVarsWhileStatement
============
*/
void Scr_CalcLocalVarsWhileStatement(sval_u expr, sval_u stmt, scr_block_s *block, sval_u *whileStatBlock)
{
	scr_block_s **oldContinueChildBlocks, **continueChildBlocks, **breakChildBlocks, **oldBreakChildBlocks;
	int continueChildCount, breakChildCount, abortLevel, i;
	int *oldBreakChildCount, *oldContinueChildCount;
	VariableCompileValue constValue;
	bool constConditional;

	constConditional = false;

	if (EvalExpression(expr, &constValue))
	{
		if (constValue.value.type == VAR_INTEGER || constValue.value.type == VAR_FLOAT)
		{
			Scr_CastBool(&constValue.value);

			if (constValue.value.u.intValue)
			{
				constConditional = true;
			}
		}

		//RemoveRefToValue(&constValue.value);
		RemoveRefToValue(constValue.value.type, constValue.value.u);
	}

	oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
	oldBreakChildCount = scrCompileGlob.breakChildCount;

	oldContinueChildBlocks = scrCompileGlob.continueChildBlocks;
	oldContinueChildCount = scrCompileGlob.continueChildCount;

	breakChildCount = 0;
	continueChildCount = 0;

	//continueChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHighInternal(sizeof(scr_block_s **) * MAX_SWITCH_CASES);
	continueChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s **) * MAX_SWITCH_CASES, "Scr_CalcLocalVarsWhileStatement");

	scrCompileGlob.continueChildBlocks = continueChildBlocks;
	scrCompileGlob.continueChildCount = &continueChildCount;

	abortLevel = block->abortLevel;

	if (constConditional)
	{
		//breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHighInternal(sizeof(scr_block_s **) * MAX_SWITCH_CASES);
		breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s **) * MAX_SWITCH_CASES, "Scr_CalcLocalVarsWhileStatement");
		scrCompileGlob.breakChildCount = &breakChildCount;
	}
	else
	{
		breakChildBlocks = NULL;
	}

	scrCompileGlob.breakChildBlocks = breakChildBlocks;

	Scr_CopyBlock(block, &whileStatBlock->block);

	Scr_CalcLocalVarsStatement(stmt, whileStatBlock->block);
	Scr_AddContinueBlock(whileStatBlock->block);

	for (i = 0; i < continueChildCount; i++)
	{
		Scr_AppendChildBlocks(&continueChildBlocks[i], 1, block);
	}

	if (constConditional)
	{
		Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
	}

	Scr_MergeChildBlocks(&whileStatBlock->block, 1, block);

	scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
	scrCompileGlob.breakChildCount = oldBreakChildCount;

	scrCompileGlob.continueChildBlocks = oldContinueChildBlocks;
	scrCompileGlob.continueChildCount = oldContinueChildCount;
}

/*
============
EmitMethod
============
*/
void EmitMethod(sval_u expr, sval_u func_name, sval_u params, sval_u methodSourcePos, bool bStatement, scr_block_s *block)
{
	VariableValue value;
	unsigned int methId, name;
	char *savedPos = NULL;
	void (*meth)(scr_entref_t);
	int type, param_count;
	const char *pName;
	sval_u sourcePos;

	name = Scr_GetBuiltin(func_name);

	if (!name)
	{
		EmitPreFunctionCall(func_name);

		param_count = EmitExpressionList(params, block);

		EmitPrimitiveExpression(expr, block);
		EmitPostFunctionCall(func_name, param_count, true, block);

		AddOpcodePos(methodSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
		AddExpressionListOpcodePos(params);

		if (bStatement)
		{
			EmitDecTop();
		}

		return;
	}

	pName = SL_ConvertToString(name);
	sourcePos = func_name.node[2];

	methId = FindVariable(scrCompilePub.builtinMeth, name);

	if (methId)
	{
		value = Scr_EvalVariable(methId);
		type = Scr_GetUncacheType(value.type);

		meth = (void (*)(scr_entref_t))value.u.pointerValue;
	}
	else
	{
		type = BUILTIN_ANY;
		meth = Scr_GetMethod(&pName, &type);

		methId = GetNewVariable(scrCompilePub.builtinMeth, name);

		value.type = (Vartype_t)Scr_GetCacheType(type);
		value.u.pointerValue = (intptr_t)meth;

		SetVariableValue(methId, &value);
	}

	if (!meth)
	{
		EmitPreFunctionCall(func_name);

		param_count = EmitExpressionList(params, block);

		EmitPrimitiveExpression(expr, block);
		EmitPostFunctionCall(func_name, param_count, true, block);

		AddOpcodePos(methodSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
		AddExpressionListOpcodePos(params);

		if (bStatement)
		{
			EmitDecTop();
		}

		return;
	}

	if (type == BUILTIN_DEVELOPER_ONLY)
	{
		Scr_BeginDevScript(&type, &savedPos);

		if (type == BUILTIN_DEVELOPER_ONLY && !bStatement)
		{
			CompileError(sourcePos.sourcePosValue, "return value of developer command can not be accessed if not in a /# ... #/ comment");
			return;
		}
	}

	param_count = EmitExpressionList(params, block);
	EmitPrimitiveExpression(expr, block);

	if (param_count > 256)
	{
		CompileError(sourcePos.sourcePosValue, "parameter count exceeds 256");
		return;
	}

	Scr_CompileRemoveRefToString(name);
	EmitCallBuiltinMethodOpcode(param_count, sourcePos);

	EmitUnsignedShort(AddFunction((intptr_t)meth));

	AddOpcodePos(methodSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddExpressionListOpcodePos(params);

	if (bStatement)
	{
		EmitDecTop();
	}

	Scr_EndDevScript(type, &savedPos);
}

/*
============
EmitCall
============
*/
void EmitCall(sval_u func_name, sval_u params, bool bStatement, scr_block_s *block)
{
	VariableValue value;
	unsigned int funcId, name;
	char *savedPos = NULL;
	void (*func)();
	int type, param_count;
	const char *pName;
	sval_u sourcePos;

	name = Scr_GetBuiltin(func_name);

	if (!name)
	{
		EmitPreFunctionCall(func_name);

		param_count = EmitExpressionList(params, block);

		EmitPostFunctionCall(func_name, param_count, 0, block);
		AddExpressionListOpcodePos(params);

		if (bStatement)
		{
			EmitDecTop();
		}

		return;
	}

	pName = SL_ConvertToString(name);
	sourcePos = func_name.node[2];

	funcId = FindVariable(scrCompilePub.builtinFunc, name);

	if (funcId)
	{
		value = Scr_EvalVariable(funcId);
		type = Scr_GetUncacheType(value.type);

		func = (void (*)())value.u.pointerValue;
	}
	else
	{
		type = BUILTIN_ANY;
		func = Scr_GetFunction(&pName, &type);

		funcId = GetNewVariable(scrCompilePub.builtinFunc, name);

		value.type = (Vartype_t)Scr_GetCacheType(type);
		value.u.pointerValue = (intptr_t)func;

		SetVariableValue(funcId, &value);
	}

	if (!func)
	{
		EmitPreFunctionCall(func_name);

		param_count = EmitExpressionList(params, block);

		EmitPostFunctionCall(func_name, param_count, false, block);
		AddExpressionListOpcodePos(params);

		if (bStatement)
		{
			EmitDecTop();
		}

		return;
	}

	if (type == BUILTIN_DEVELOPER_ONLY)
	{
		Scr_BeginDevScript(&type, &savedPos);

		if (type == BUILTIN_DEVELOPER_ONLY && !bStatement)
		{
			CompileError(sourcePos.sourcePosValue, "return value of developer command can not be accessed if not in a /# ... #/ comment");
			return;
		}
	}

	param_count = EmitExpressionList(params, block);

	if (param_count > 256)
	{
		CompileError(sourcePos.stringValue, "parameter count exceeds 256");
		return;
	}

	Scr_CompileRemoveRefToString(name);
	EmitCallBuiltinOpcode(param_count, sourcePos);

	EmitUnsignedShort(AddFunction((intptr_t)func));

	AddExpressionListOpcodePos(params);

	if (bStatement)
	{
		EmitDecTop();
	}

	Scr_EndDevScript(type, &savedPos);
}

/*
============
Scr_CalcLocalVarsStatement
============
*/
void Scr_CalcLocalVarsStatement(sval_u val, scr_block_s *block)
{
	switch (val.node[0].type)
	{
	case ENUM_assignment:
		Scr_CalcLocalVarsAssignmentStatement(val.node[1], val.node[2], block);
		break;

	case ENUM_return:
	case ENUM_return2:
		if (block->abortLevel == SCR_ABORT_NONE)
			block->abortLevel = SCR_ABORT_RETURN;
		break;

	case ENUM_if:
		Scr_CalcLocalVarsIfStatement(val.node[2], block, &val.node[4]);
		break;

	case ENUM_if_else:
		Scr_CalcLocalVarsIfElseStatement(val.node[2], val.node[3], block, &val.node[6], &val.node[7]);
		break;

	case ENUM_while:
		Scr_CalcLocalVarsWhileStatement(val.node[1], val.node[2], block, &val.node[5]);
		break;

	case ENUM_for:
		Scr_CalcLocalVarsForStatement(val.node[1], val.node[2], val.node[3], val.node[4], block, &val.node[7], &val.node[8]);
		break;

	case ENUM_inc:
	case ENUM_dec:
	case ENUM_binary_equals:
		Scr_CalcLocalVarsIncStatement(val.node[1], block);
		break;

	case ENUM_statement_list:
		Scr_CalcLocalVarsStatementList(val.node[1], block);
		break;

	case ENUM_developer_statement_list:
		Scr_CalcLocalVarsDeveloperStatementList(val.node[1], block, &val.node[3]);
		break;

	case ENUM_waittill:
		Scr_CalcLocalVarsWaittillStatement(val.node[2], block);
		break;

	case ENUM_switch:
		Scr_CalcLocalVarsSwitchStatement(val.node[2], block);
		break;

	case ENUM_break:
		Scr_AddBreakBlock(block);
		if (block->abortLevel == SCR_ABORT_NONE)
			block->abortLevel = SCR_ABORT_BREAK;
		break;

	case ENUM_continue:
		Scr_AddContinueBlock(block);
		if (block->abortLevel == SCR_ABORT_NONE)
			block->abortLevel = SCR_ABORT_CONTINUE;
		break;
	}
}

/*
============
EmitCallExpressionFieldObject
============
*/
void EmitCallExpressionFieldObject(sval_u expr, scr_block_s *block)
{
	if (expr.node[0].type == ENUM_call)
	{
		EmitCall(expr.node[1], expr.node[2], false, block);
		EmitCastFieldObject(expr.node[3]);
	}
	else if (expr.node[0].type == ENUM_method)
	{
		EmitMethod(expr.node[1], expr.node[2], expr.node[3], expr.node[4], false, block);
		EmitCastFieldObject(expr.node[5]);
	}
}

/*
============
EmitCallExpression
============
*/
void EmitCallExpression(sval_u expr, bool bStatement, scr_block_s *block)
{
	if (expr.node[0].type == ENUM_call)
	{
		EmitCall(expr.node[1], expr.node[2], bStatement, block);
	}
	else if (expr.node[0].type == ENUM_method)
	{
		EmitMethod(expr.node[1], expr.node[2], expr.node[3], expr.node[4], bStatement, block);
	}
}

/*
============
Scr_CalcLocalVarsStatementList
============
*/
void Scr_CalcLocalVarsStatementList(sval_u val, scr_block_s *block)
{
	for (sval_u *node = val.node[0].node[1].node; node; node = node[1].node)
	{
		Scr_CalcLocalVarsStatement(node[0], block);
	}
}

/*
============
EmitCallExpressionStatement
============
*/
void EmitCallExpressionStatement(sval_u expr, scr_block_s *block)
{
	EmitCallExpression(expr, true, block);
}

/*
============
EmitPrimitiveExpressionFieldObject
============
*/
void EmitPrimitiveExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	switch (expr.node[0].type)
	{
	case ENUM_variable:
		EmitVariableExpression(expr.node[1], block);
		EmitCastFieldObject(expr.node[2]);
		break;

	case ENUM_call_expression:
		EmitCallExpressionFieldObject(expr.node[1], block);
		break;

	case ENUM_self:
		EmitSelfObject(expr.node[1]);
		break;

	case ENUM_level:
		EmitLevelObject(expr.node[1]);
		break;

	case ENUM_anim:
		EmitAnimObject(expr.node[1]);
		break;

	case ENUM_expression_list:
		EmitExpressionListFieldObject(expr.node[1], sourcePos, block);
		break;

	default:
		CompileError(sourcePos.sourcePosValue, "not an object");
		break;
	}
}

/*
============
EmitClearFieldVariable
============
*/
void EmitClearFieldVariable(sval_u expr, sval_u field, sval_u sourcePos, sval_u rhsSourcePos, scr_block_s *block)
{
	EmitPrimitiveExpressionFieldObject(expr, sourcePos, block);
	EmitOpcode(OP_ClearFieldVariable, 0, CALL_NONE);
	AddOpcodePos(rhsSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	EmitCanonicalString(field.stringValue);
}

/*
============
EmitFieldVariableRef
============
*/
void EmitFieldVariableRef(sval_u expr, sval_u field, sval_u sourcePos, scr_block_s *block)
{
	EmitPrimitiveExpressionFieldObject(expr, sourcePos, block);
	EmitOpcode(OP_EvalFieldVariableRef, 0, CALL_NONE);
	EmitCanonicalString(field.stringValue);
}

/*
============
EmitFieldVariable
============
*/
void EmitFieldVariable(sval_u expr, sval_u field, sval_u sourcePos, scr_block_s *block)
{
	EmitPrimitiveExpressionFieldObject(expr, sourcePos, block);
	EmitOpcode(OP_EvalFieldVariable, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	EmitCanonicalString(field.stringValue);
}

/*
============
Scr_CalcLocalVarsThread
============
*/
void Scr_CalcLocalVarsThread(sval_u exprlist, sval_u stmtlist, sval_u *stmttblock)
{
	static_assert(sizeof(scr_block_s) == 536);

	scrCompileGlob.forceNotCreate = false;

	//stmttblock->block = (scr_block_s *)Hunk_AllocateTempMemoryHighInternal(sizeof(*stmttblock->block));
	stmttblock->block = (scr_block_s *)Hunk_AllocateTempMemoryHigh(sizeof(scr_block_s), "Scr_CalcLocalVarsThread");
	
	stmttblock->block->abortLevel = SCR_ABORT_NONE;
	stmttblock->block->localVarsCreateCount = 0;
	stmttblock->block->localVarsCount = 0;
	stmttblock->block->localVarsPublicCount = 0;

	memset(stmttblock->block->localVarsInitBits, 0, sizeof(stmttblock->block->localVarsInitBits));

	Scr_CalcLocalVarsFormalParameterList(exprlist, stmttblock->block);
	Scr_CalcLocalVarsStatementList(stmtlist, stmttblock->block);
}

/*
============
EmitVariableExpression
============
*/
void EmitVariableExpression(sval_u expr, scr_block_s *block)
{
	switch (expr.node[0].type)
	{
	case ENUM_local_variable:
		EmitLocalVariable(expr.node[1], expr.node[2], block);
		break;

	case ENUM_array_variable:
		EmitArrayVariable(expr.node[1], expr.node[2], expr.node[3], expr.node[4], block);
		break;

	case ENUM_field_variable:
		EmitFieldVariable(expr.node[1], expr.node[2], expr.node[3], block);
		break;

	case ENUM_self_field:
		if (scrCompilePub.script_loading)
			CompileError(expr.node[2].sourcePosValue, "self field can only be used in the script debugger");
		else
			CompileError(expr.node[2].sourcePosValue, "self field in assignment expression not currently supported");
		break;

	case ENUM_object:
		EmitObject(expr.node[1], expr.node[2]);
		break;
	}
}

/*
============
EmitOrEvalPrimitiveExpression
============
*/
bool EmitOrEvalPrimitiveExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block)
{
	switch (expr.node[0].type)
	{
	case ENUM_variable:
		EmitVariableExpression(expr.node[1], block);
		return false;

	case ENUM_function:
		EmitGetFunction(expr.node[1], expr.node[2]);
		return false;

	case ENUM_call_expression:
		EmitCallExpression(expr.node[1], false, block);
		return false;

	case ENUM_self:
		EmitSelf(expr.node[1]);
		return false;

	case ENUM_level:
		EmitLevel(expr.node[1]);
		return false;

	case ENUM_game:
		EmitGame(expr.node[1]);
		return false;

	case ENUM_anim:
		EmitAnim(expr.node[1]);
		return false;

	case ENUM_expression_list:
		return EmitOrEvalPrimitiveExpressionList(expr.node[1], expr.node[2], constValue, block);

	case ENUM_size_field:
		EmitSize(expr.node[1], expr.node[2], block);
		return false;

	case ENUM_empty_array:
		EmitEmptyArray(expr.node[1]);
		return false;

	case ENUM_animation:
		EmitAnimation(expr.node[1], expr.node[2]);
		return false;

	case ENUM_animtree:
		EmitAnimTree(expr.node[1]);
		return false;

	case ENUM_breakon:
		EmitBreakOn(expr.node[1], expr.node[2], expr.node[3]);
		return false;

	default:
		return EvalPrimitiveExpression(expr, constValue);
	}
}

/*
============
EmitOrEvalExpression
============
*/
bool EmitOrEvalExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block)
{
	switch (expr.node[0].type)
	{
	case ENUM_primitive_expression:
		return EmitOrEvalPrimitiveExpression(expr.node[1], constValue, block);

	case ENUM_bool_or:
		EmitBoolOrExpression(expr.node[1], expr.node[2], expr.node[3], expr.node[4], block);
		return false;

	case ENUM_bool_and:
		EmitBoolAndExpression(expr.node[1], expr.node[2], expr.node[3], expr.node[4], block);
		return false;

	case ENUM_binary:
		return EmitOrEvalBinaryOperatorExpression(expr.node[1], expr.node[2], expr.node[3], expr.node[4], constValue, block);

	case ENUM_bool_not:
		EmitExpression(expr.node[1], block);
		EmitBoolNot(expr.node[2]);
		return false;

	case ENUM_bool_complement:
		EmitExpression(expr.node[1], block);
		EmitBoolComplement(expr.node[2]);
		return false;

	default:
		return false;
	}
}

/*
============
EmitBreakOn
============
*/
void EmitBreakOn(sval_u expr, sval_u param, sval_u sourcePos)
{
	CompileError(sourcePos.sourcePosValue, "illegal function name");
}

/*
============
EmitForStatement
============
*/
void EmitForStatement(sval_u stmt1, sval_u expr, sval_u stmt2, sval_u stmt, sval_u sourcePos, sval_u forSourcePos, scr_block_s *block, sval_u *forStatBlock, sval_u *forStatPostBlock)
{
	ContinueStatementInfo *oldContinueStatement; // [esp+0h] [ebp-50h]
	int breakChildCount; // [esp+4h] [ebp-4Ch] BYREF
	int *oldBreakChildCount; // [esp+8h] [ebp-48h]
	scr_block_s **breakChildBlocks; // [esp+Ch] [ebp-44h]
	BreakStatementInfo *oldBreakStatement; // [esp+10h] [ebp-40h]
	bool constConditional; // [esp+17h] [ebp-39h]
	unsigned int offset; // [esp+18h] [ebp-38h]
	bool bOldCanBreak; // [esp+1Eh] [ebp-32h]
	bool bOldCanContinue; // [esp+1Fh] [ebp-31h]
	int continueChildCount; // [esp+20h] [ebp-30h] BYREF
	int *oldContinueChildCount; // [esp+24h] [ebp-2Ch]
	VariableCompileValue constValue; // [esp+28h] [ebp-28h] BYREF
	const char *pos1; // [esp+34h] [ebp-1Ch]
	const char *nextPos2; // [esp+38h] [ebp-18h]
	scr_block_s **continueChildBlocks; // [esp+3Ch] [ebp-14h]
	scr_block_s **oldBreakChildBlocks; // [esp+40h] [ebp-10h]
	const char *pos2; // [esp+44h] [ebp-Ch]
	scr_block_s **oldContinueChildBlocks; // [esp+48h] [ebp-8h]
	scr_block_s *oldBreakBlock; // [esp+4Ch] [ebp-4h]

	bOldCanBreak = scrCompileGlob.bCanBreak;
	oldBreakStatement = scrCompileGlob.currentBreakStatement;
	scrCompileGlob.bCanBreak = 0;
	bOldCanContinue = scrCompileGlob.bCanContinue;
	oldContinueStatement = scrCompileGlob.currentContinueStatement;
	scrCompileGlob.bCanContinue = 0;
	EmitStatement(stmt1, 0, 0, block);
	Scr_TransferBlock(block, forStatBlock->block);
	EmitCreateLocalVars(forStatBlock->block);
	iassert(forStatBlock->block->localVarsCreateCount <= block->localVarsCount);
	block->localVarsCreateCount = forStatBlock->block->localVarsCreateCount;
	Scr_TransferBlock(block, forStatPostBlock->block);
	pos1 = TempMalloc(0);
	if (expr.node[0].type == ENUM_expression)
	{
		constConditional = 0;
		if (EmitOrEvalExpression(expr.node[1], &constValue, block))
		{
			if (constValue.value.type == VAR_INTEGER || constValue.value.type == VAR_FLOAT)
			{
				Scr_CastBool(&constValue.value);
				if (!constValue.value.u.intValue)
					CompileError(sourcePos.stringValue, "conditional expression cannot be always false");
				constConditional = 1;
			}
			else
			{
				EmitValue(&constValue);
			}
		}
	}
	else
	{
		constConditional = 1;
	}
	oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
	oldBreakChildCount = scrCompileGlob.breakChildCount;
	oldBreakBlock = scrCompileGlob.breakBlock;
	oldContinueChildBlocks = scrCompileGlob.continueChildBlocks;
	oldContinueChildCount = scrCompileGlob.continueChildCount;
	breakChildCount = 0;
	continueChildCount = 0;
	continueChildBlocks = (scr_block_s**)Hunk_AllocateTempMemoryHigh(4096, "EmitForStatement");
	scrCompileGlob.continueChildBlocks = continueChildBlocks;
	scrCompileGlob.continueChildCount = &continueChildCount;
	scrCompileGlob.breakBlock = forStatBlock->block;
	if (constConditional)
	{
		pos2 = 0;
		nextPos2 = 0;
		breakChildBlocks = (scr_block_s**)Hunk_AllocateTempMemoryHigh(4096, "EmitForStatement");
		scrCompileGlob.breakChildCount = &breakChildCount;
	}
	else
	{
		EmitOpcode(OP_JumpOnFalse, -1, CALL_NONE);
		AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
		EmitShort(0);
		pos2 = (const char *)scrCompileGlob.codePos;
		nextPos2 = TempMalloc(0);
		breakChildBlocks = 0;
	}
	scrCompileGlob.breakChildBlocks = breakChildBlocks;
	scrCompileGlob.bCanBreak = 1;
	scrCompileGlob.currentBreakStatement = 0;
	scrCompileGlob.bCanContinue = 1;
	scrCompileGlob.currentContinueStatement = 0;
	EmitStatement(stmt, 0, 0, forStatBlock->block);
	Scr_AddContinueBlock(forStatBlock->block);
	scrCompileGlob.bCanBreak = 0;
	scrCompileGlob.bCanContinue = 0;
	ConnectContinueStatements();
	Scr_InitFromChildBlocks(continueChildBlocks, continueChildCount, forStatPostBlock->block);
	EmitStatement(stmt2, 0, 0, forStatPostBlock->block);
	EmitOpcode(OP_jumpback, 0, CALL_NONE);
	AddOpcodePos(forSourcePos.stringValue, SOURCE_TYPE_NONE);

	if (stmt.node[0].type == ENUM_statement_list)
	{
		AddOpcodePos(stmt.node[3].sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	}

	EmitShort(0);
	offset = TempMalloc(0) - pos1;
	if (offset >= 0x10000)
		MyAssertHandler(".\\script\\scr_compiler.cpp", 3658, 0, "%s", "offset < 65536");
	*scrCompileGlob.codePos = offset;
	if (pos2)
	{
		offset = TempMalloc(0) - nextPos2;
		if (offset >= 0x10000)
			MyAssertHandler(".\\script\\scr_compiler.cpp", 3663, 0, "%s", "offset < 65536");
		*(unsigned short *)pos2 = offset;
	}
	ConnectBreakStatements();
	scrCompileGlob.bCanBreak = bOldCanBreak;
	scrCompileGlob.currentBreakStatement = oldBreakStatement;
	scrCompileGlob.bCanContinue = bOldCanContinue;
	scrCompileGlob.currentContinueStatement = oldContinueStatement;
	if (constConditional)
		Scr_InitFromChildBlocks(breakChildBlocks, breakChildCount, block);
	scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
	scrCompileGlob.breakChildCount = oldBreakChildCount;
	scrCompileGlob.breakBlock = oldBreakBlock;
	scrCompileGlob.continueChildBlocks = oldContinueChildBlocks;
	scrCompileGlob.continueChildCount = oldContinueChildCount;
}

/*
============
EmitWhileStatement
============
*/
void EmitWhileStatement(sval_u expr, sval_u stmt, sval_u sourcePos, sval_u whileSourcePos, scr_block_s *block, sval_u *whileStatBlock)
{
	ContinueStatementInfo *oldContinueStatement; // [esp+0h] [ebp-48h]
	int breakChildCount; // [esp+4h] [ebp-44h] BYREF
	int *oldBreakChildCount; // [esp+8h] [ebp-40h]
	scr_block_s **breakChildBlocks; // [esp+Ch] [ebp-3Ch]
	BreakStatementInfo *oldBreakStatement; // [esp+10h] [ebp-38h]
	bool constConditional; // [esp+17h] [ebp-31h]
	unsigned int offset; // [esp+18h] [ebp-30h]
	bool bOldCanBreak; // [esp+1Eh] [ebp-2Ah]
	bool bOldCanContinue; // [esp+1Fh] [ebp-29h]
	int *oldContinueChildCount; // [esp+20h] [ebp-28h]
	VariableCompileValue constValue; // [esp+24h] [ebp-24h] BYREF
	const char *pos1; // [esp+30h] [ebp-18h]
	const char *nextPos2; // [esp+34h] [ebp-14h]
	scr_block_s **oldBreakChildBlocks; // [esp+38h] [ebp-10h]
	const char *pos2; // [esp+3Ch] [ebp-Ch]
	scr_block_s **oldContinueChildBlocks; // [esp+40h] [ebp-8h]
	scr_block_s *oldBreakBlock; // [esp+44h] [ebp-4h]

	bOldCanBreak = scrCompileGlob.bCanBreak;
	oldBreakStatement = scrCompileGlob.currentBreakStatement;
	scrCompileGlob.bCanBreak = 0;
	bOldCanContinue = scrCompileGlob.bCanContinue;
	oldContinueStatement = scrCompileGlob.currentContinueStatement;
	scrCompileGlob.bCanContinue = 0;
	Scr_TransferBlock(block, whileStatBlock->block);
	EmitCreateLocalVars(whileStatBlock->block);
	iassert(whileStatBlock->block->localVarsCreateCount <= block->localVarsCount);
	block->localVarsCreateCount = whileStatBlock->block->localVarsCreateCount;
	pos1 = TempMalloc(0);
	constConditional = 0;
	if (EmitOrEvalExpression(expr, &constValue, block))
	{
		if (constValue.value.type == VAR_INTEGER || constValue.value.type == VAR_FLOAT)
		{
			Scr_CastBool(&constValue.value);
			if (!constValue.value.u.intValue)
				CompileError(sourcePos.stringValue, "conditional expression cannot be always false");
			constConditional = 1;
		}
		else
		{
			EmitValue(&constValue);
		}
	}
	oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
	oldBreakChildCount = scrCompileGlob.breakChildCount;
	oldBreakBlock = scrCompileGlob.breakBlock;
	oldContinueChildBlocks = scrCompileGlob.continueChildBlocks;
	oldContinueChildCount = scrCompileGlob.continueChildCount;
	breakChildCount = 0;
	scrCompileGlob.continueChildBlocks = 0;
	scrCompileGlob.breakBlock = whileStatBlock->block;
	if (constConditional)
	{
		pos2 = 0;
		nextPos2 = 0;
		breakChildBlocks = (scr_block_s**)Hunk_AllocateTempMemoryHigh(4096, "EmitWhileStatement");
		scrCompileGlob.breakChildCount = &breakChildCount;
	}
	else
	{
		EmitOpcode(OP_JumpOnFalse, -1, CALL_NONE);
		AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
		EmitShort(0);
		pos2 = (const char *)scrCompileGlob.codePos;
		nextPos2 = TempMalloc(0);
		breakChildBlocks = 0;
	}
	scrCompileGlob.breakChildBlocks = breakChildBlocks;
	scrCompileGlob.bCanBreak = 1;
	scrCompileGlob.currentBreakStatement = 0;
	scrCompileGlob.bCanContinue = 1;
	scrCompileGlob.currentContinueStatement = 0;
	EmitStatement(stmt, 0, 0, whileStatBlock->block);

	if (whileStatBlock->block->abortLevel != SCR_ABORT_RETURN)
	{
		whileStatBlock->block->abortLevel = SCR_ABORT_NONE;
	}

	scrCompileGlob.bCanBreak = 0;
	scrCompileGlob.bCanContinue = 0;
	ConnectContinueStatements();
	EmitOpcode(OP_jumpback, 0, CALL_NONE);
	AddOpcodePos(whileSourcePos.sourcePosValue, SOURCE_TYPE_NONE);

	if (stmt.node[0].type == ENUM_statement_list)
	{
		AddOpcodePos(stmt.node[3].sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	}

	EmitShort(0);
	offset = TempMalloc(0) - pos1;
	if (offset >= 0x10000)
		MyAssertHandler(".\\script\\scr_compiler.cpp", 3429, 0, "%s", "offset < 65536");
	*scrCompileGlob.codePos = offset;
	if (pos2)
	{
		offset = TempMalloc(0) - nextPos2;
		if (offset >= 0x10000)
			MyAssertHandler(".\\script\\scr_compiler.cpp", 3434, 0, "%s", "offset < 65536");
		*(unsigned short *)pos2 = offset;
	}
	ConnectBreakStatements();
	scrCompileGlob.bCanBreak = bOldCanBreak;
	scrCompileGlob.currentBreakStatement = oldBreakStatement;
	scrCompileGlob.bCanContinue = bOldCanContinue;
	scrCompileGlob.currentContinueStatement = oldContinueStatement;
	if (constConditional)
		Scr_InitFromChildBlocks(breakChildBlocks, breakChildCount, block);
	scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
	scrCompileGlob.breakChildCount = oldBreakChildCount;
	scrCompileGlob.breakBlock = oldBreakBlock;
	scrCompileGlob.continueChildBlocks = oldContinueChildBlocks;
	scrCompileGlob.continueChildCount = oldContinueChildCount;
}

/*
============
EmitExpression
============
*/
void EmitExpression(sval_u expr, scr_block_s *block)
{
	VariableCompileValue constValue;

	if (EmitOrEvalExpression(expr, &constValue, block))
	{
		EmitValue(&constValue);
	}
}

/*
============
EmitClearArrayVariable
============
*/
void EmitClearArrayVariable(sval_u expr, sval_u index, sval_u sourcePos, sval_u indexSourcePos, scr_block_s *block)
{
	EmitExpression(index, block);
	EmitArrayPrimitiveExpressionRef(expr, sourcePos, block);
	EmitClearArray(sourcePos, indexSourcePos);
}

/*
============
EmitArrayVariableRef
============
*/
void EmitArrayVariableRef(sval_u expr, sval_u index, sval_u sourcePos, sval_u indexSourcePos, scr_block_s *block)
{
	EmitExpression(index, block);
	EmitArrayPrimitiveExpressionRef(expr, sourcePos, block);
	EmitEvalArrayRef(sourcePos, indexSourcePos);
}

/*
============
EmitSwitchStatement
============
*/
void EmitSwitchStatement(sval_u expr, sval_u stmtlist, sval_u sourcePos, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
	CaseStatementInfo *oldCaseStatement; // [esp+0h] [ebp-24h]
	char *pos3; // [esp+4h] [ebp-20h]
	BreakStatementInfo *oldBreakStatement; // [esp+8h] [ebp-1Ch]
	bool bOldCanBreak; // [esp+Fh] [ebp-15h]
	char *nextPos1; // [esp+10h] [ebp-14h]
	CaseStatementInfo *caseStatement; // [esp+14h] [ebp-10h]
	CaseStatementInfo *caseStatementa; // [esp+14h] [ebp-10h]
	unsigned __int8 *pos1; // [esp+18h] [ebp-Ch]
	signed int num; // [esp+1Ch] [ebp-8h]
	unsigned __int8 *pos2; // [esp+20h] [ebp-4h]

	oldCaseStatement = scrCompileGlob.currentCaseStatement;
	bOldCanBreak = scrCompileGlob.bCanBreak;
	oldBreakStatement = scrCompileGlob.currentBreakStatement;
	scrCompileGlob.bCanBreak = 0;

	EmitExpression(expr, block);
	EmitOpcode(OP_switch, -1, CALL_NONE);
	EmitCodepos(0);

	pos1 = scrCompileGlob.codePos;
	nextPos1 = TempMalloc(0);
	scrCompileGlob.currentCaseStatement = 0;
	scrCompileGlob.currentBreakStatement = 0;
	EmitSwitchStatementList(stmtlist, lastStatement, endSourcePos, block);
	EmitOpcode(0x7Du, 0, 0);
	AddOpcodePos(sourcePos.stringValue, 0);
	EmitShort(0);
	pos2 = scrCompileGlob.codePos;
	*(intptr_t *)pos1 = scrCompileGlob.codePos - (byte *)nextPos1;
	pos3 = TempMallocAlignStrict(0);
	num = 0;
	caseStatement = scrCompileGlob.currentCaseStatement;
	while (caseStatement)
	{
		EmitCodepos((const char*)caseStatement->name);
		EmitCodepos(caseStatement->codePos);
		caseStatement = caseStatement->next;
		++num;
	}
	*pos2 = num;
	qsort(pos3, num, 8u, CompareCaseInfo);
	while (num > 1)
	{
		if (*pos3 == *(pos3 + 2))
		{
			for (caseStatementa = scrCompileGlob.currentCaseStatement; caseStatementa; caseStatementa = caseStatementa->next)
			{
				if (caseStatementa->name == *pos3)
				{
					CompileError(caseStatementa->sourcePos, "duplicate case expression");
					return;
				}
			}
		}
		--num;
		pos3 += 8;
	}
	ConnectBreakStatements();
	scrCompileGlob.currentCaseStatement = oldCaseStatement;
	scrCompileGlob.bCanBreak = bOldCanBreak;
	scrCompileGlob.currentBreakStatement = oldBreakStatement;
}

/*
============
EmitEndOnStatement
============
*/
void EmitEndOnStatement(sval_u obj, sval_u expr, sval_u sourcePos, sval_u exprSourcePos, scr_block_s *block)
{
	EmitExpression(expr, block);
	EmitPrimitiveExpression(obj, block);
	EmitOpcode(OP_endon, -2, CALL_NONE);
	AddOpcodePos(exprSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitNotifyStatement
============
*/
void EmitNotifyStatement(sval_u obj, sval_u exprlist, sval_u sourcePos, sval_u notifySourcePos, scr_block_s *block)
{
	sval_u *node, *start_node;
	int expr_count;

	EmitOpcode(OP_voidCodepos, 1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);

	expr_count = 0;
	start_node = NULL;

	for (node = exprlist.node[0].node; node; node = node[1].node)
	{
		start_node = node;
		EmitExpression(node[0].node[0], block);
		expr_count++;
	}

	iassert(start_node);

	EmitPrimitiveExpression(obj, block);
	EmitOpcode(OP_notify, -expr_count - 2, CALL_NONE);

	AddOpcodePos(notifySourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(start_node[0].node[1].sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitWaittillmatchStatement
============
*/
void EmitWaittillmatchStatement(sval_u obj, sval_u exprlist, sval_u sourcePos, sval_u waitSourcePos, scr_block_s *block)
{
	sval_u *node;
	int exprCount;

	node = exprlist.node[0].node[1].node;
	iassert(node);

	for (exprCount = 0; ; exprCount++)
	{
		node = node[1].node;

		if (!node)
		{
			break;
		}

		EmitExpression(node[0].node[0], block);
	}

	node = exprlist.node[0].node[1].node;
	iassert(node);

	EmitExpression(node[0].node[0], block);
	EmitPrimitiveExpression(obj, block);

	EmitOpcode(OP_waittillmatch, -2 - exprCount, CALL_NONE);

	AddOpcodePos(waitSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(waitSourcePos.sourcePosValue, SOURCE_TYPE_NONE);

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(node[0].node[1].sourcePosValue, SOURCE_TYPE_NONE);

	while (1)
	{
		node = node[1].node;

		if (!node)
		{
			break;
		}

		AddOpcodePos(node[0].node[1].sourcePosValue, SOURCE_TYPE_NONE);
	}

	iassert(exprCount < 256);

	EmitByte(exprCount);
	EmitOpcode(OP_clearparams, 0, CALL_NONE);
}

/*
============
EmitWaittillStatement
============
*/
void EmitWaittillStatement(sval_u obj, sval_u exprlist, sval_u sourcePos, sval_u waitSourcePos, scr_block_s *block)
{
	sval_u *node = exprlist.node[0].node[1].node;
	iassert(node);

	EmitExpression(node[0].node[0], block);
	EmitPrimitiveExpression(obj, block);
	EmitOpcode(OP_waittill, -2, CALL_NONE);

	AddOpcodePos(waitSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(waitSourcePos.sourcePosValue, SOURCE_TYPE_NONE);

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(node[0].node[1].sourcePosValue, SOURCE_TYPE_NONE);

	EmitFormalWaittillParameterListRefInternal(node, block);
	EmitOpcode(OP_clearparams, 0, CALL_NONE);
}

/*
============
EmitIfElseStatement
============
*/
void EmitIfElseStatement(sval_u expr, sval_u stmt1, sval_u stmt2, sval_u sourcePos, sval_u elseSourcePos, bool lastStatement, unsigned int endSourcePos, scr_block_s *block, sval_u *ifStatBlock, sval_u *elseStatBlock)
{
	int childCount, checksum;
	scr_block_s *childBlocks[2];
	const char *pos1, *pos2, *nextPos1, *nextPos2;
	unsigned int offset;

	childCount = 0;

	EmitExpression(expr, block);
	EmitOpcode(OP_JumpOnFalse, -1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	EmitUnsignedShort(0);

	pos1 = (const char *)scrCompileGlob.codePos;
	nextPos1 = TempMalloc(0);

	Scr_TransferBlock(block, ifStatBlock->block);

	EmitStatement(stmt1, lastStatement, endSourcePos, ifStatBlock->block);
	EmitRemoveLocalVars(ifStatBlock->block, ifStatBlock->block);

	if (ifStatBlock->block->abortLevel == SCR_ABORT_NONE)
	{
		childBlocks[0] = ifStatBlock->block;
		childCount = 1;
	}

	checksum = scrVarPub.checksum;

	if (lastStatement)
	{
		EmitEnd();
		EmitCodepos(0);
		AddOpcodePos(endSourcePos, SOURCE_TYPE_BREAKPOINT);

		pos2 = NULL;
		nextPos2 = NULL;
	}
	else
	{
		EmitOpcode(OP_jump, 0, CALL_NONE);
		AddOpcodePos(elseSourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
		EmitCodepos(0);

		pos2 = (const char *)scrCompileGlob.codePos;
		nextPos2 = TempMalloc(0);
	}

	scrVarPub.checksum = checksum + 1;

	offset = TempMallocAlignStrict(0) - nextPos1;
	iassert(offset < 65536);
	*(unsigned short *)pos1 = offset;

	Scr_TransferBlock(block, elseStatBlock->block);

	EmitStatement(stmt2, lastStatement, endSourcePos, elseStatBlock->block);
	EmitNOP2(lastStatement, endSourcePos, elseStatBlock->block);

	if (elseStatBlock->block->abortLevel == SCR_ABORT_NONE)
	{
		childBlocks[childCount] = elseStatBlock->block;
		childCount++;
	}

	if (!lastStatement)
	{
		offset = TempMallocAlignStrict(0) - nextPos2;
		*(intptr_t *)pos2 = offset;
	}

	Scr_InitFromChildBlocks(childBlocks, childCount, block);
}

/*
============
EmitIfStatement
============
*/
void EmitIfStatement(sval_u expr, sval_u stmt, sval_u sourcePos, bool lastStatement, unsigned int endSourcePos, scr_block_s *block, sval_u *ifStatBlock)
{
	const char *pos, *nextPos;
	unsigned int offset;

	EmitExpression(expr, block);
	EmitOpcode(OP_JumpOnFalse, -1, CALL_NONE);
	AddOpcodePos(sourcePos.stringValue, SOURCE_TYPE_NONE);
	EmitUnsignedShort(0);

	pos = (const char *)scrCompileGlob.codePos;
	nextPos = TempMallocAlignStrict(0);

	Scr_TransferBlock(block, ifStatBlock->block);

	EmitStatement(stmt, lastStatement, endSourcePos, ifStatBlock->block);
	iassert(ifStatBlock->block->localVarsPublicCount == block->localVarsCreateCount);
	EmitNOP2(lastStatement, endSourcePos, ifStatBlock->block);

	offset = TempMallocAlignStrict(0) - nextPos;
	iassert(offset < 65536);
	*(unsigned short *)pos = offset;
}

/*
============
EmitWaitStatement
============
*/
void EmitWaitStatement(sval_u expr, sval_u sourcePos, sval_u waitSourcePos, scr_block_s *block)
{
	EmitExpression(expr, block);
	EmitOpcode(OP_wait, -1, CALL_NONE);

	AddOpcodePos(waitSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(waitSourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitReturnStatement
============
*/
void EmitReturnStatement(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	if (block->abortLevel == SCR_ABORT_NONE)
	{
		block->abortLevel = SCR_ABORT_RETURN;
	}

	EmitExpression(expr, block);
	EmitReturn();

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
}

/*
============
EmitClearVariableExpression
============
*/
bool EmitClearVariableExpression(sval_u expr, sval_u rhsSourcePos, scr_block_s *block)
{
	switch (expr.node[0].type)
	{
	case ENUM_local_variable:
		return false;

	case ENUM_array_variable:
		EmitClearArrayVariable(expr.node[1], expr.node[2], expr.node[3], expr.node[4], block);
		return true;

	case ENUM_field_variable:
		EmitClearFieldVariable(expr.node[1], expr.node[2], expr.node[3], rhsSourcePos, block);
		return true;

	case ENUM_self_field:
	case ENUM_object:
		if (scrCompilePub.script_loading)
			CompileError(expr.node[2].sourcePosValue, "$ and self field can only be used in the script debugger");
		else
			CompileError(expr.node[2].sourcePosValue, "not an lvalue");
		return true;

	default:
		return true;
	}
}

/*
============
EmitVariableExpressionRef
============
*/
void EmitVariableExpressionRef(sval_u expr, scr_block_s *block)
{
	switch (expr.node[0].type)
	{
	case ENUM_local_variable:
		EmitLocalVariableRef(expr.node[1], expr.node[2], block);
		break;

	case ENUM_array_variable:
		EmitArrayVariableRef(expr.node[1], expr.node[2], expr.node[3], expr.node[4], block);
		break;

	case ENUM_field_variable:
		EmitFieldVariableRef(expr.node[1], expr.node[2], expr.node[3], block);
		break;

	case ENUM_self_field:
	case ENUM_object:
		if (scrCompilePub.script_loading)
			CompileError(expr.node[2].sourcePosValue, "$ and self field can only be used in the script debugger");
		else
			CompileError(expr.node[2].sourcePosValue, "not an lvalue");
		break;
	}
}

/*
============
EmitBinaryEqualsOperatorExpression
============
*/
void EmitBinaryEqualsOperatorExpression(sval_u lhs, sval_u rhs, sval_u opcode, sval_u sourcePos, scr_block_s *block)
{
	iassert(!scrCompileGlob.bConstRefCount);
	scrCompileGlob.bConstRefCount = true;
	EmitVariableExpression(lhs, block);

	iassert(scrCompileGlob.bConstRefCount);
	scrCompileGlob.bConstRefCount = false;
	EmitExpression(rhs, block);

	EmitOpcode(opcode.type, -1, CALL_NONE);
	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);

	EmitVariableExpressionRef(lhs, block);
	EmitSetVariableField(sourcePos);
}

/*
============
EmitDecStatement
============
*/
void EmitDecStatement(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	iassert(!scrCompileGlob.forceNotCreate);
	scrCompileGlob.forceNotCreate = true;
	EmitVariableExpressionRef(expr, block);

	iassert(scrCompileGlob.forceNotCreate);
	scrCompileGlob.forceNotCreate = false;
	EmitOpcode(OP_dec, 1, CALL_NONE);

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	EmitSetVariableField(sourcePos);
}

/*
============
EmitIncStatement
============
*/
void EmitIncStatement(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
	iassert(!scrCompileGlob.forceNotCreate);
	scrCompileGlob.forceNotCreate = true;
	EmitVariableExpressionRef(expr, block);

	iassert(scrCompileGlob.forceNotCreate);
	scrCompileGlob.forceNotCreate = false;
	EmitOpcode(OP_inc, 1, CALL_NONE);

	AddOpcodePos(sourcePos.sourcePosValue, SOURCE_TYPE_NONE);
	EmitSetVariableField(sourcePos);
}

/*
============
EmitAssignmentStatement
============
*/
void EmitAssignmentStatement(sval_u lhs, sval_u rhs, sval_u sourcePos, sval_u rhsSourcePos, scr_block_s *block)
{
	if (IsUndefinedExpression(rhs))
	{
		if (EmitClearVariableExpression(lhs, rhsSourcePos, block))
		{
			return;
		}
	}

	EmitExpression(rhs, block);
	EmitVariableExpressionRef(lhs, block);
	EmitSetVariableField(sourcePos);
}

/*
============
EmitStatement
============
*/
void EmitStatement(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
	switch (val.node[0].type)
	{
	case ENUM_assignment:
		EmitAssignmentStatement(val.node[1], val.node[2], val.node[3], val.node[4], block);
		break;

	case ENUM_call_expression_statement:
		EmitCallExpressionStatement(val.node[1], block);
		break;

	case ENUM_return:
		EmitReturnStatement(val.node[1], val.node[2], block);
		break;

	case ENUM_return2:
		EmitEndStatement(val.node[1], block);
		break;

	case ENUM_wait:
		EmitWaitStatement(val.node[1], val.node[2], val.node[3], block);
		break;

	case ENUM_if:
		EmitIfStatement(val.node[1], val.node[2], val.node[3], lastStatement, endSourcePos, block, &val.node[4]);
		break;

	case ENUM_if_else:
		EmitIfElseStatement(val.node[1], val.node[2], val.node[3], val.node[4], val.node[5], lastStatement, endSourcePos, block, &val.node[6], &val.node[7]);
		break;

	case ENUM_while:
		EmitWhileStatement(val.node[1], val.node[2], val.node[3], val.node[4], block, &val.node[5]);
		break;

	case ENUM_for:
		EmitForStatement(val.node[1], val.node[2], val.node[3], val.node[4], val.node[5], val.node[6], block, &val.node[7], &val.node[8]);
		break;

	case ENUM_inc:
		EmitIncStatement(val.node[1], val.node[2], block);
		break;

	case ENUM_dec:
		EmitDecStatement(val.node[1], val.node[2], block);
		break;

	case ENUM_binary_equals:
		EmitBinaryEqualsOperatorExpression(val.node[1], val.node[2], val.node[3], val.node[4], block);
		break;

	case ENUM_statement_list:
		EmitStatementList(val.node[1], lastStatement, endSourcePos, block);
		break;

	case ENUM_developer_statement_list:
		EmitDeveloperStatementList(val.node[1], val.node[2], block, &val.node[3]);
		break;

	case ENUM_waittill:
		EmitWaittillStatement(val.node[1], val.node[2], val.node[3], val.node[4], block);
		break;

	case ENUM_waittillmatch:
		EmitWaittillmatchStatement(val.node[1], val.node[2], val.node[3], val.node[4], block);
		break;

	case ENUM_waittillFrameEnd:
		EmitWaittillFrameEnd(val.node[1]);
		break;

	case ENUM_notify:
		EmitNotifyStatement(val.node[1], val.node[2], val.node[3], val.node[4], block);
		break;

	case ENUM_endon:
		EmitEndOnStatement(val.node[1], val.node[2], val.node[3], val.node[4], block);
		break;

	case ENUM_switch:
		EmitSwitchStatement(val.node[1], val.node[2], val.node[3], lastStatement, endSourcePos, block);
		break;

	case ENUM_case:
		CompileError(val.node[2].sourcePosValue, "illegal case statement");
		break;

	case ENUM_default:
		CompileError(val.node[1].sourcePosValue, "illegal default statement");
		break;

	case ENUM_break:
		EmitBreakStatement(val.node[1], block);
		break;

	case ENUM_continue:
		EmitContinueStatement(val.node[1], block);
		break;

	case ENUM_breakpoint:
		EmitBreakpointStatement(val.node[1]);
		break;

	case ENUM_prof_begin:
		EmitProfBeginStatement(val.node[1], val.node[2]);
		break;

	case ENUM_prof_end:
		EmitProfEndStatement(val.node[1], val.node[2]);
		break;

	default:
		return;
	}
}

/*
============
EmitStatementList
============
*/
void EmitStatementList(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
	sval_u *node, *next_node;

	for (next_node = val.node[0].node[1].node; next_node; next_node = node)
	{
		node = next_node[1].node;

		if (lastStatement && Scr_IsLastStatement(node))
			EmitStatement(next_node[0], true, endSourcePos, block);
		else
			EmitStatement(next_node[0], false, endSourcePos, block);
	}
}

/*
============
EmitThreadInternal
============
*/
void EmitThreadInternal(unsigned int threadId, sval_u val, sval_u sourcePos, sval_u endSourcePos, scr_block_s *block)
{
	scrCompileGlob.threadId = threadId;
	AddThreadStartOpcodePos(sourcePos.sourcePosValue);

	scrCompileGlob.cumulOffset = 0;
	scrCompileGlob.maxOffset = 0;
	scrCompileGlob.maxCallOffset = 0;

	CompileTransferRefToString(val.node[1].stringValue, 2);

	EmitFormalParameterList(val.node[2], sourcePos, block);
	EmitStatementList(val.node[3], true, endSourcePos.sourcePosValue, block);

	EmitEnd();

	AddOpcodePos(endSourcePos.sourcePosValue, SOURCE_TYPE_BREAKPOINT);
	AddOpcodePos(0xFFFFFFFE, SOURCE_TYPE_NONE);

	iassert(!scrCompileGlob.cumulOffset);

	if (scrCompileGlob.maxOffset + MAX_VM_STACK_DEPTH * scrCompileGlob.maxCallOffset >= MAX_VM_OPERAND_STACK)
	{
		CompileError(sourcePos.sourcePosValue, "function exceeds operand stack size");
	}
}

/*
============
EmitDeveloperThread
============
*/
void EmitDeveloperThread(sval_u val, sval_u *stmttblock)
{
	unsigned int posId, threadId, savedChecksum;
	char *begin_pos;

	iassert(scrCompilePub.developer_statement == SCR_DEV_NO);

	if (scrVarPub.developer_script)
	{
		scrCompilePub.developer_statement = SCR_DEV_YES;
		InitThread(1);

		posId = FindVariable(scrCompileGlob.fileId, val.node[1].sourcePosValue);
		threadId = FindObject(posId);

		SetThreadPosition(threadId);
		EmitThreadInternal(threadId, val, val.node[4], val.node[5], stmttblock->block);
	}
	else
	{
		begin_pos = TempMalloc(0);
		savedChecksum = scrVarPub.checksum;

		scrCompilePub.developer_statement = SCR_DEV_IGNORE;
		InitThread(1);

		EmitThreadInternal(0, val, val.node[4], val.node[5], stmttblock->block);

		TempMemorySetPos(begin_pos);
		scrVarPub.checksum = savedChecksum;
	}

	scrCompilePub.developer_statement = SCR_DEV_NO;
}

/*
============
EmitNormalThread
============
*/
void EmitNormalThread(sval_u val, sval_u *stmttblock)
{
	unsigned int posId, threadId;

	InitThread(0);

	posId = FindVariable(scrCompileGlob.fileId, val.node[1].sourcePosValue);
	threadId = FindObject(posId);

	SetThreadPosition(threadId);
	EmitThreadInternal(threadId, val, val.node[4], val.node[5], stmttblock->block);
}

/*
============
EmitThread
============
*/
void EmitThread(sval_u val)
{
	switch (val.node[0].type)
	{
	case ENUM_thread:
		Scr_CalcLocalVarsThread(val.node[2], val.node[3], &val.node[6]);
		if (scrCompileGlob.in_developer_thread)
			EmitDeveloperThread(val, &val.node[6]);
		else
			EmitNormalThread(val, &val.node[6]);
		break;

	case ENUM_begin_developer_thread:
		iassert(!scrCompileGlob.in_developer_thread);
		scrCompileGlob.in_developer_thread = true;
		break;

	case ENUM_end_developer_thread:
		iassert(scrCompileGlob.in_developer_thread);
		scrCompileGlob.in_developer_thread = false;
		break;

	case ENUM_usingtree:
		if (scrCompileGlob.in_developer_thread)
			CompileError(val.node[2].sourcePosValue, "cannot put #using_animtree inside /# ... #/ comment");
		else
		{
			Scr_UsingTree(SL_ConvertToString(val.node[1].stringValue), val.node[3].sourcePosValue);
			Scr_CompileRemoveRefToString(val.node[1].stringValue);
		}
		break;
	}
}

/*
============
EmitThreadList
============
*/
void EmitThreadList(sval_u val)
{
	sval_u *node;

	scrCompileGlob.in_developer_thread = false;

	for (node = val.node[0].node[1].node; node; node = node[1].node)
	{
		SpecifyThread(node[0]);
	}

	if (scrCompileGlob.in_developer_thread)
	{
		CompileError(scrCompileGlob.developer_thread_sourcePos, "/# has no matching #/");
	}

	scrCompileGlob.firstThread[0] = true;
	scrCompileGlob.firstThread[1] = true;

	iassert(!scrCompileGlob.in_developer_thread);

	for (node = val.node[0].node[1].node; node; node = node[1].node)
	{
		EmitThread(node[0]);
	}

	iassert(!scrCompileGlob.in_developer_thread);
}

void __cdecl ScriptCompile(
	sval_u val,
	unsigned int fileId,
	unsigned int scriptId,
	PrecacheEntry *entries,
	int entriesCount)
{
	unsigned int Variable_DONE; // eax
	VariableValueInternal_u *VariableValueAddress_DONE; // esi
	PrecacheEntry *v9; // [esp+4h] [ebp-54h]
	int j; // [esp+10h] [ebp-48h]
	VariableValue pos; // [esp+14h] [ebp-44h] BYREF
	unsigned __int16 filename; // [esp+1Ch] [ebp-3Ch]
	PrecacheEntry *precachescript; // [esp+20h] [ebp-38h]
	int far_function_count; // [esp+24h] [ebp-34h]
	PrecacheEntry *precachescript2; // [esp+28h] [ebp-30h]
	unsigned int toThreadId; // [esp+2Ch] [ebp-2Ch]
	unsigned int toPosId; // [esp+30h] [ebp-28h]
	unsigned int posId; // [esp+34h] [ebp-24h]
	unsigned __int16 name; // [esp+38h] [ebp-20h]
	unsigned int threadId; // [esp+3Ch] [ebp-1Ch]
	PrecacheEntry *precachescriptList; // [esp+40h] [ebp-18h]
	int i; // [esp+44h] [ebp-14h]
	unsigned int includeFileId; // [esp+48h] [ebp-10h]
	VariableValue value; // [esp+4Ch] [ebp-Ch] BYREF
	unsigned int threadPtr; // [esp+54h] [ebp-4h]
	int entriesCounta; // [esp+70h] [ebp+18h]

	scrCompileGlob.fileId = fileId;
	scrCompileGlob.bConstRefCount = 0;
	scrAnimPub.animTreeIndex = 0;
	scrCompilePub.developer_statement = 0;
	if (scrCompilePub.far_function_count)
		v9 = &entries[entriesCount];
	else
		v9 = 0;
	precachescriptList = v9;
	entriesCounta = scrCompilePub.far_function_count + entriesCount;
	if (entriesCounta > 1024)
		Com_Error(ERR_DROP, "MAX_PRECACHE_ENTRIES exceeded");
	scrCompileGlob.precachescriptList = precachescriptList;
	EmitIncludeList(val.node[0]);
	EmitThreadList(val.node[1]);
	scrCompilePub.programLen = TempMalloc(0) - scrVarPub.programBuffer;
	Scr_ShutdownAllocNode();
	Hunk_ClearTempMemoryHigh();
	if (scrCompilePub.far_function_count != scrCompileGlob.precachescriptList - precachescriptList)
		MyAssertHandler(
			".\\script\\scr_compiler.cpp",
			4990,
			0,
			"%s",
			"scrCompilePub.far_function_count == scrCompileGlob.precachescriptList - precachescriptList");
	far_function_count = scrCompilePub.far_function_count;
	for (i = 0; i < far_function_count; ++i)
	{
		precachescript = &precachescriptList[i];
		filename = precachescript->filename;
		ProfLoad_End();
		includeFileId = Scr_LoadScriptInternal(SL_ConvertToString(filename), entries, entriesCounta);
		ProfLoad_Begin("ScriptCompile");
		if (!includeFileId)
		{
			CompileError(precachescript->sourcePos, "Could not find script '%s'", SL_ConvertToString(filename));
			return;
		}
		SL_RemoveRefToString(filename);
		if (precachescript->include)
		{
			for (j = i + 1; j < far_function_count; ++j)
			{
				precachescript2 = &precachescriptList[j];
				if (!precachescript2->include)
					break;
				if (precachescript2->filename == filename)
				{
					CompileError(precachescript2->sourcePos, "Duplicate #include");
					return;
				}
			}
			precachescript->include = 0;
			for (threadPtr = FindFirstSibling(includeFileId); threadPtr; threadPtr = FindNextSibling(threadPtr))
			{
				if (GetValueType(threadPtr) != VAR_POINTER)
				{
					continue;
				}
				threadId = FindObject(threadPtr);
				iassert(threadId);
				posId = FindVariable(threadId, 1u);
				if (posId)
				{
					pos = Scr_EvalVariable(posId);
					if (pos.type == VAR_INCLUDE_CODEPOS)
					{
						continue;
					}
					iassert((pos.type == VAR_CODEPOS) || (pos.type == VAR_DEVELOPER_CODEPOS));

					name = GetVariableName(threadPtr);
					Variable_DONE = GetVariable(fileId, name);
					toThreadId = GetObject(Variable_DONE);
					toPosId = SpecifyThreadPosition(toThreadId, name, precachescript->sourcePos, VAR_INCLUDE_CODEPOS);
					VariableValueAddress_DONE = GetVariableValueAddress(posId);
					*GetVariableValueAddress(toPosId) = *VariableValueAddress_DONE;
					LinkThread(toThreadId, &pos, false);
				}
			}
		}
	}
	LinkFile(fileId);
	value.type = VAR_INTEGER;
	SetVariableValue(scriptId, &value);
}

#pragma GCC pop_options