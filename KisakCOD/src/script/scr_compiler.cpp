// Some functions adapted from: https://github.com/voron00/CoD2rev_Server/blob/41e33ed97d0339ac631772f25eee3910ddef87e5/src/script/scr_compiler.cpp
// (GPL v3.0) (Thanks)

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

#undef GetObject 

scrCompilePub_t scrCompilePub;
scrCompileGlob_t scrCompileGlob;

void __cdecl EmitOpcode(Opcode_t op, int offset, int callType);
char __cdecl EmitOrEvalPrimitiveExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block);
void __cdecl EmitExpression(sval_u expr, scr_block_s *block);
void __cdecl EmitExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block);
void __cdecl EmitVariableExpression(sval_u expr, scr_block_s *block);
void __cdecl EmitPrimitiveExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block);
char __cdecl EmitOrEvalExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block);
char __cdecl EvalExpression(sval_u expr, VariableCompileValue *constValue);
void __cdecl EmitStatement(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block);
char __cdecl EvalPrimitiveExpression(sval_u expr, VariableCompileValue *constValue);
void __cdecl EmitArrayPrimitiveExpressionRef(sval_u expr, sval_u sourcePos, scr_block_s *block);

bool __cdecl IsUndefinedPrimitiveExpression(sval_u expr)
{
    return expr.node[0].type == ENUM_undefined;
}

bool __cdecl IsUndefinedExpression(sval_u expr)
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

void __cdecl Scr_CompileRemoveRefToString(unsigned int stringValue)
{
    if (!stringValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 178, 0, "%s", "stringValue");
    if (!scrCompileGlob.bConstRefCount && scrCompilePub.developer_statement != 3)
        SL_RemoveRefToString(stringValue);
}

void __cdecl EmitCanonicalString(unsigned int stringValue)
{
    if (!stringValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 192, 0, "%s", "stringValue");
    scrCompileGlob.codePos = (unsigned char*)TempMallocAlignStrict(2u);
    if (scrCompilePub.developer_statement == 2)
    {
        if (scrVarPub.developer_script)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 198, 0, "%s", "!scrVarPub.developer_script");
        Scr_CompileRemoveRefToString(stringValue);
    }
    else if (scrCompilePub.developer_statement == 3)
    {
        *scrCompileGlob.codePos = Scr_CompileCanonicalString(stringValue);
        if (!*scrCompileGlob.codePos)
            CompileError(0, "unknown field");
    }
    else
    {
        if (scrCompileGlob.bConstRefCount)
            SL_AddRefToString(stringValue);
        *scrCompileGlob.codePos = SL_TransferToCanonicalString(stringValue);
    }
}

void __cdecl EmitCanonicalStringConst(unsigned int stringValue)
{
    bool bConstRefCount; // [esp+3h] [ebp-1h]

    bConstRefCount = scrCompileGlob.bConstRefCount;
    scrCompileGlob.bConstRefCount = 1;
    EmitCanonicalString(stringValue);
    scrCompileGlob.bConstRefCount = bConstRefCount;
}

int __cdecl Scr_FindLocalVarIndex(unsigned int name, sval_u sourcePos, bool create, scr_block_s *block)
{
    char *v5; // eax
    int i; // [esp+4h] [ebp-4h]

    if (scrCompilePub.developer_statement == 3)
        MyAssertHandler(
            ".\\script\\scr_compiler.cpp",
            759,
            0,
            "%s",
            "scrCompilePub.developer_statement != SCR_DEV_EVALUATE");
    if (block)
    {
        for (i = 0; ; ++i)
        {
            if (i >= block->localVarsCount)
                goto LABEL_18;
            if (i == block->localVarsCreateCount)
            {
                ++block->localVarsCreateCount;
                EmitOpcode(OP_CreateLocalVariable, 0, 0);
                EmitCanonicalStringConst(block->localVars[i].name);
                AddOpcodePos(block->localVars[i].sourcePos, 0);
            }
            if (block->localVars[i].name == name)
                break;
        }
        Scr_CompileRemoveRefToString(name);
        if (((1 << (i & 7)) & block->localVarsInitBits[i >> 3]) == 0)
        {
            if (!create || scrCompileGlob.forceNotCreate)
            {
            LABEL_18:
                if (!create || scrCompileGlob.forceNotCreate)
                {
                    v5 = SL_ConvertToString(name);
                    CompileError(sourcePos.stringValue, "uninitialised variable '%s'", v5);
                    return 0;
                }
                goto unreachable;
            }
            block->localVarsInitBits[i >> 3] |= 1 << (i & 7);
        }
        if (block->localVarsCreateCount - 1 < i)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 789, 0, "%s", "(block->localVarsCreateCount - 1) >= i");
        return block->localVarsCreateCount - 1 - i;
    }
unreachable:
    CompileError(sourcePos.stringValue, "unreachable code");
    return 0;
}

void __cdecl EmitByte(unsigned __int8 value)
{
    scrCompileGlob.codePos = (unsigned char*)TempMalloc(1u);
    *scrCompileGlob.codePos = value;
}

void __cdecl EmitLocalVariable(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    int index;
    Opcode_t opcode;

    if (scrCompilePub.developer_statement == 3)
    {
        EmitOpcode(OP_EvalLocalVariable, 1, 0);
        EmitCanonicalString(expr.stringValue);
    }
    else
    {
        index = Scr_FindLocalVarIndex(expr.stringValue, sourcePos, 0, block);
        if (index > 5)
        {
            opcode = OP_EvalLocalVariableCached;
        }
        else
        {
            opcode = (Opcode_t)((int)OP_EvalLocalVariableCached0 + index);
        }
        EmitOpcode(opcode, 1, 0);
        if (opcode == OP_EvalLocalVariableCached)
            EmitByte(index);
        AddOpcodePos(sourcePos.stringValue, 1);
    }
}

void __cdecl EmitGetUndefined(sval_u sourcePos)
{
    EmitOpcode(OP_GetUndefined, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitShort(__int16 value)
{
    scrCompileGlob.codePos = (unsigned char*)TempMallocAlignStrict(2u);
    *scrCompileGlob.codePos = value;
}

void __cdecl CompileTransferRefToString(unsigned int stringValue, unsigned int user)
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

void __cdecl EmitGetString(unsigned int value, sval_u sourcePos)
{
    EmitOpcode(OP_GetString, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    EmitShort(value);
    CompileTransferRefToString(value, 1u);
}

void __cdecl EmitGetIString(unsigned int value, sval_u sourcePos)
{
    EmitOpcode(OP_GetIString, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    EmitShort(value);
    CompileTransferRefToString(value, 1u);
}

void __cdecl EmitFloat(float value)
{
    scrCompileGlob.codePos = (unsigned char*)TempMallocAlignStrict(4u);
    *scrCompileGlob.codePos = value;
}

void __cdecl EmitGetVector(const float *value, sval_u sourcePos)
{
    int i; // [esp+4h] [ebp-4h]

    EmitOpcode(OP_GetVector, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    for (i = 0; i < 3; ++i)
        EmitFloat(value[i]);
    RemoveRefToVector(value);
}

void __cdecl EmitGetFloat(float value, sval_u sourcePos)
{
    EmitOpcode(OP_GetFloat, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    EmitFloat(value);
}

void __cdecl EmitCodepos(const char *pos)
{
    scrCompileGlob.codePos = (unsigned char*)TempMallocAlignStrict(4u);
    *(unsigned int*)scrCompileGlob.codePos = (unsigned int)pos;
}

void __cdecl EmitGetInteger(int value, sval_u sourcePos)
{
    if (value < 0)
    {
        if (value > -256)
        {
            EmitOpcode(OP_GetNegByte, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            EmitByte(-value);
            return;
        }
        if (value > -65536)
        {
            EmitOpcode(OP_GetNegUnsignedShort, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            EmitShort(-value);
            return;
        }
    }
    else
    {
        if (!value)
        {
            EmitOpcode(OP_GetZero, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            return;
        }
        if (value < 256)
        {
            EmitOpcode(OP_GetByte, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            EmitByte(value);
            return;
        }
        if (value < 0x10000)
        {
            EmitOpcode(OP_GetUnsignedShort, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            EmitShort(value);
            return;
        }
    }
    EmitOpcode(OP_GetInteger, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    EmitCodepos((const char*)value);
}

void __cdecl EmitValue(VariableCompileValue *constValue)
{
    switch (constValue->value.type)
    {
    case 0:
        EmitGetUndefined(constValue->sourcePos);
        break;
    case 2:
        EmitGetString(constValue->value.u.intValue, constValue->sourcePos);
        break;
    case 3:
        EmitGetIString(constValue->value.u.intValue, constValue->sourcePos);
        break;
    case 4:
        EmitGetVector(constValue->value.u.vectorValue, constValue->sourcePos);
        break;
    case 5:
        EmitGetFloat(constValue->value.u.floatValue, constValue->sourcePos);
        break;
    case 6:
        EmitGetInteger(constValue->value.u.intValue, constValue->sourcePos);
        break;
    default:
        return;
    }
}

void __cdecl EmitPrimitiveExpression(sval_u expr, scr_block_s *block)
{
    VariableCompileValue constValue; // [esp+0h] [ebp-Ch] BYREF

    if (EmitOrEvalPrimitiveExpression(expr, &constValue, block))
        EmitValue(&constValue);
}

void __cdecl EmitEvalArray(sval_u sourcePos, sval_u indexSourcePos)
{
    EmitOpcode(OP_EvalArray, -1, 0);
    AddOpcodePos(indexSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitArrayVariable(sval_u expr, sval_u index, sval_u sourcePos, sval_u indexSourcePos, scr_block_s *block)
{
    EmitExpression(index, block);
    EmitPrimitiveExpression(expr, block);
    EmitEvalArray(sourcePos, indexSourcePos);
}

void EmitPreAssignmentPos()
{
    if (scrVarPub.developer && scrCompilePub.developer_statement != 3)
        Scr_AddAssignmentPos((char*)scrCompilePub.opcodePos);
}

void __cdecl EmitCastFieldObject(sval_u sourcePos)
{
    EmitOpcode(OP_CastFieldObject, -1, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 0);
}

int __cdecl Scr_GetUncacheType(int type)
{
    if (type == 7)
        return 0;
    if (type != 12)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2032, 0, "%s", "type == VAR_DEVELOPER_CODEPOS");
    return 1;
}

Vartype_t __cdecl Scr_GetCacheType(int type)
{
    if (!type)
        return VAR_CODEPOS;
    if (type != 1)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2018, 0, "%s", "type == BUILTIN_DEVELOPER_ONLY");
    return VAR_DEVELOPER_CODEPOS;

}

void __cdecl Scr_BeginDevScript(int *type, char **savedPos)
{
    if (scrCompilePub.developer_statement)
    {
        *type = 0;
    }
    else
    {
        if (scrVarPub.developer_script)
        {
            scrCompilePub.developer_statement = 1;
        }
        else
        {
            *savedPos = TempMalloc(0);
            scrCompilePub.developer_statement = 2;
        }
        *type = 1;
    }
}

int __cdecl EmitExpressionList(sval_u exprlist, scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-8h]
    int expr_count; // [esp+4h] [ebp-4h]

    expr_count = 0;
    for (node = exprlist.node[0].node; node; node = node[1].node)
    {
        EmitExpression(node[0].node[0], block);
        ++expr_count;
    }
    return expr_count;
}

void __cdecl EmitCallBuiltinOpcode(int param_count, sval_u sourcePos)
{
    Opcode_t opcode; // [esp+0h] [ebp-4h]

    if (param_count > 5)
    {
        opcode = OP_CallBuiltin;
    }
    else
    {
        opcode = (Opcode_t)((int)OP_CallBuiltin0 + param_count);
    }
    EmitOpcode(opcode, 1 - param_count, 1);
    AddOpcodePos(sourcePos.stringValue, 9);
    if (opcode == OP_CallBuiltin)
        EmitByte(param_count);
}

int __cdecl AddFunction(int func, const char *name)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < scrCompilePub.func_table_size; ++i)
    {
        if (scrCompilePub.func_table[i] == func)
            return i;
    }
    if (i != scrCompilePub.func_table_size)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1835, 0, "%s", "i == scrCompilePub.func_table_size");
    if (scrCompilePub.func_table_size == 1024)
        Com_Error(ERR_DROP, "SCR_FUNC_TABLE_SIZE exceeded");
    scrCompilePub.func_table[scrCompilePub.func_table_size] = func;
    scrVmDebugPub.func_table[scrCompilePub.func_table_size].breakpointCount = 0;
    scrVmDebugPub.func_table[scrCompilePub.func_table_size++].name = name;
    return i;
}

void __cdecl AddExpressionListOpcodePos(sval_u exprlist)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    if (scrVarPub.developer)
    {
        for (sval_u *node = exprlist.node[0].node; node; node = node[1].node)
            AddOpcodePos(node[0].node[1].sourcePosValue, 0);
    }
}

void EmitDecTop()
{
    EmitOpcode(OP_DecTop, -1, 0);
}

void __cdecl Scr_EndDevScript(int type, char **savedPos)
{
    if (type != 1)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2001, 0, "%s", "type == BUILTIN_DEVELOPER_ONLY");
    scrCompilePub.developer_statement = 0;
    if (!scrVarPub.developer_script)
        TempMemorySetPos(*savedPos);
}

void __cdecl EmitPreFunctionCall(sval_u func_name)
{
    if (*func_name.codePosValue == 26)
        EmitOpcode(OP_PreScriptCall, 1, 0);
}

unsigned int  __cdecl AddFilePrecache(unsigned int filename, unsigned int sourcePos, bool include)
{
    unsigned int Variable_DONE; // eax

    if (!scrCompileGlob.precachescriptList)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1646, 0, "%s", "scrCompileGlob.precachescriptList");
    SL_AddRefToString(filename);
    Scr_CompileRemoveRefToString(filename);
    scrCompileGlob.precachescriptList->filename = filename;
    scrCompileGlob.precachescriptList->sourcePos = sourcePos;
    scrCompileGlob.precachescriptList->include = include;
    ++scrCompileGlob.precachescriptList;
    Variable_DONE = GetVariable(scrCompilePub.scripts, filename);
    return GetObject(Variable_DONE);
}

void __cdecl EmitFunction(sval_u func, sval_u sourcePos)
{
    char *v2; // eax
    unsigned int Variable; // eax
    unsigned int valueId; // [esp+1Ch] [ebp-3Ch]
    VariableValue pos; // [esp+20h] [ebp-38h]
    HashEntry_unnamed_type_u filename; // [esp+28h] [ebp-30h]
    unsigned int posId; // [esp+2Ch] [ebp-2Ch]
    unsigned int threadId; // [esp+30h] [ebp-28h]
    int scope; // [esp+38h] [ebp-20h]
    unsigned int countId; // [esp+3Ch] [ebp-1Ch]
    VariableValue count; // [esp+40h] [ebp-18h] BYREF
    VariableValue value; // [esp+48h] [ebp-10h] BYREF
    unsigned int fileId; // [esp+50h] [ebp-8h]
    unsigned int threadPtr; // [esp+54h] [ebp-4h]

    if (scrCompilePub.developer_statement == 3)
    {
        CompileError(sourcePos.stringValue, "cannot evaluate in the debugger");
        return;
    }
    if (scrCompilePub.developer_statement == 2)
    {
        Scr_CompileRemoveRefToString(func.node[1].stringValue);
        if (func.node[0].type == ENUM_far_function)
        {
            Scr_CompileRemoveRefToString(func.node[2].stringValue);
            --scrCompilePub.far_function_count;
        }
        return;
    }
    if (func.node[0].type == ENUM_local_function)
    {
        scope = 0;
        threadPtr = GetVariable(scrCompileGlob.fileId, func.node[1].idValue);
		CompileTransferRefToString(func.node[1].stringValue, 2);
        threadId = GetObject(threadPtr);
        if (!threadId)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 1708, 0, "%s", "threadId");
        goto LABEL_39;
    }
    if (func.node[0].type != 21)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1712, 0, "%s", "func.node[0].type == ENUM_far_function");
    scope = 1;
    v2 = SL_ConvertToString(func.node[1].stringValue);
    filename.prev = Scr_CreateCanonicalFilename(v2).prev;
    Scr_CompileRemoveRefToString(func.node[1].stringValue);
    Variable = FindVariable(scrCompilePub.loadedscripts, filename.prev);
    value = Scr_EvalVariable(Variable);
    fileId = AddFilePrecache(filename.prev, sourcePos.stringValue, 0);
    if (value.type)
    {
        threadPtr = FindVariable(fileId, func.node[2].idValue);
        if (!threadPtr || GetValueType(threadPtr) != 1)
        {
        LABEL_26:
            CompileError(sourcePos.stringValue, "unknown function");
            return;
        }
    }
    else
    {
        threadPtr = GetVariable(fileId, func.node[2].idValue);
    }
    CompileTransferRefToString(func.node[2].stringValue, 2);
    threadId = GetObject(threadPtr);
    posId = FindVariable(threadId, 1u);
    if (!posId)
        goto LABEL_39;
    pos = Scr_EvalVariable(posId);
    if (pos.type != 7 && pos.type != 12 && pos.type != 13)
        MyAssertHandler(
            ".\\script\\scr_compiler.cpp",
            1748,
            0,
            "%s\n\t(pos.type) = %i",
            "(pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS || pos.type == VAR_INCLUDE_CODEPOS)",
            pos.type);
    if (pos.type == 13)
        goto LABEL_26;
    if (!pos.u.intValue)
    {
    LABEL_39:
        if (!threadId)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 1781, 0, "%s", "threadId");
        EmitCodepos((const char*)scope);
        countId = GetVariable(threadId, 0);
        count = Scr_EvalVariable(countId);
        if (count.type && count.type != 6)
            MyAssertHandler(
                ".\\script\\scr_compiler.cpp",
                1787,
                0,
                "%s",
                "(count.type == VAR_UNDEFINED) || (count.type == VAR_INTEGER)");
        if (!count.type)
        {
            count.type = VAR_INTEGER;
            count.u.intValue = 0;
        }
        valueId = GetNewVariable(threadId, count.u.intValue + 2);
        value.u.intValue = (int)scrCompileGlob.codePos;
        if (scrCompilePub.developer_statement)
        {
            if (!scrVarPub.developer_script)
                MyAssertHandler(".\\script\\scr_compiler.cpp", 1800, 0, "%s", "scrVarPub.developer_script");
            value.type = VAR_DEVELOPER_CODEPOS;
        }
        else
        {
            value.type = VAR_CODEPOS;
        }
        SetNewVariableValue(valueId, &value);
        ++count.u.intValue;
        SetVariableValue(countId, &count);
        AddOpcodePos(sourcePos.stringValue, 0);
        return;
    }
    if (pos.type == 7)
        goto LABEL_29;
    if (pos.type != 12)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1767, 0, "%s", "pos.type == VAR_DEVELOPER_CODEPOS");
    if (!scrVarPub.developer_script)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1768, 0, "%s", "scrVarPub.developer_script");
    if (scrCompilePub.developer_statement == 2)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1769, 0, "%s", "scrCompilePub.developer_statement != SCR_DEV_IGNORE");
    if (scrCompilePub.developer_statement)
        LABEL_29:
    EmitCodepos(pos.u.codePosValue);
    else
        CompileError(sourcePos.stringValue, "normal script cannot reference a function in a /# ... #/ comment");
}

void __cdecl EmitPostScriptFunction(sval_u func, int param_count, bool bMethod, sval_u nameSourcePos)
{
    if (bMethod)
        EmitOpcode(OP_ScriptMethodCall, -param_count - 1, 3);
    else
        EmitOpcode(OP_ScriptFunctionCall, -param_count, 3);
    AddOpcodePos(nameSourcePos.stringValue, 3);
    EmitFunction(func, nameSourcePos);
}

void __cdecl EmitPostScriptFunctionPointer(
    sval_u expr,
    int param_count,
    bool bMethod,
    sval_u nameSourcePos,
    sval_u sourcePos,
    scr_block_s *block)
{
    EmitExpression(expr, block);
    if (bMethod)
        EmitOpcode(OP_ScriptMethodCallPointer, -param_count - 2, 3);
    else
        EmitOpcode(OP_ScriptFunctionCallPointer, -param_count - 1, 3);
    AddOpcodePos(sourcePos.stringValue, 0);
    AddOpcodePos(nameSourcePos.stringValue, 1);
}

void __cdecl EmitPostScriptFunctionCall(
    sval_u func_name,
    int param_count,
    bool bMethod,
    sval_u nameSourcePos,
    scr_block_s *block)
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

void __cdecl EmitPostScriptThread(sval_u func, int param_count, bool bMethod, sval_u sourcePos)
{
    if (bMethod)
        EmitOpcode(OP_ScriptMethodThreadCall, -param_count, 2);
    else
        EmitOpcode(OP_ScriptThreadCall, 1 - param_count, 2);
    AddOpcodePos(sourcePos.stringValue, 3);
    EmitFunction(func, sourcePos);
    EmitCodepos((const char*)param_count);
}

void __cdecl EmitPostScriptThreadPointer(
    sval_u expr,
    int param_count,
    bool bMethod,
    sval_u sourcePos,
    scr_block_s *block)
{
    EmitExpression(expr, block);
    if (bMethod)
        EmitOpcode(OP_ScriptMethodThreadCallPointer, -param_count - 1, 2);
    else
        EmitOpcode(OP_ScriptThreadCallPointer, -param_count, 2);
    AddOpcodePos(sourcePos.stringValue, 1);
    EmitCodepos((const char*)param_count);
}

void __cdecl EmitPostScriptThreadCall(
    sval_u func_name,
    int param_count,
    bool bMethod,
    sval_u sourcePos,
    sval_u nameSourcePos,
    scr_block_s *block)
{
    if (func_name.node[0].type == ENUM_function)
    {
        EmitPostScriptThread(func_name.node[1], param_count, bMethod, nameSourcePos);
    }
    else if (func_name.node[0].type == ENUM_function_pointer)
    {
        EmitPostScriptThreadPointer(func_name.node[1], param_count, bMethod, func_name.node[2], block);
    }

    AddOpcodePos(sourcePos.sourcePosValue, 0);
}

void __cdecl EmitPostFunctionCall(sval_u func_name, int param_count, bool bMethod, scr_block_s *block)
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

void __cdecl EmitCall(sval_u func_name, sval_u params, bool bStatement, scr_block_s *block)
{
    __int16 v4; // ax
    int v5; // eax
    void(__cdecl * func)(); // [esp+8h] [ebp-28h]
    char *savedPos; // [esp+Ch] [ebp-24h] BYREF
    unsigned int funcId; // [esp+10h] [ebp-20h]
    int param_count; // [esp+14h] [ebp-1Ch]
    unsigned int name; // [esp+18h] [ebp-18h]
    const char *pName; // [esp+1Ch] [ebp-14h] BYREF
    int type; // [esp+20h] [ebp-10h] BYREF
    sval_u sourcePos; // [esp+24h] [ebp-Ch]
    VariableValue value; // [esp+28h] [ebp-8h] BYREF

    savedPos = 0;
    name = Scr_GetBuiltin(func_name);
    if (!name)
        goto script_function;
    pName = SL_ConvertToString(name);

    sourcePos = func_name.node[2];

    if (scrCompilePub.developer_statement == 3)
    {
        type = 0;
        func = Scr_GetFunction(&pName, &type);
    }
    else
    {
        funcId = FindVariable(scrCompilePub.builtinFunc, name);
        if (funcId)
        {
            value = Scr_EvalVariable(funcId);
            type = Scr_GetUncacheType(value.type);
            func = (void(*)())value.u.intValue;
        }
        else
        {
            type = 0;
            func = Scr_GetFunction(&pName, &type);
            funcId = GetNewVariable(scrCompilePub.builtinFunc, name);
            value.type = Scr_GetCacheType(type);
            value.u.intValue = (int)func;
            SetVariableValue(funcId, &value);
        }
    }
    if (func)
    {
        if (type == 1 && (Scr_BeginDevScript(&type, &savedPos), type == 1) && !bStatement)
        {
            CompileError(
                sourcePos.stringValue,
                "return value of developer command can not be accessed if not in a /# ... #/ comment");
        }
        else
        {
            param_count = EmitExpressionList(params, block);
            if (param_count < 256)
            {
                Scr_CompileRemoveRefToString(name);
                EmitCallBuiltinOpcode(param_count, sourcePos);
                v4 = AddFunction((int)func, pName);
                EmitShort(v4);
                AddExpressionListOpcodePos(params);
                if (bStatement)
                    EmitDecTop();
                if (type == 1)
                    Scr_EndDevScript(1, &savedPos);
            }
            else
            {
                CompileError(sourcePos.stringValue, "parameter count exceeds 256");
            }
        }
    }
    else
    {
    script_function:
        if (scrCompilePub.developer_statement == 3)
        {
            CompileError(*(unsigned int*)(func_name.type + 8), "unknown builtin function");
        }
        else
        {
            EmitPreFunctionCall(func_name);
            v5 = EmitExpressionList(params, block);
            EmitPostFunctionCall(func_name, v5, 0, block);
            AddExpressionListOpcodePos(params);
            if (bStatement)
                EmitDecTop();
        }
    }
}

void __cdecl EmitCallBuiltinMethodOpcode(int param_count, sval_u sourcePos)
{
    Opcode_t opcode; // [esp+0h] [ebp-4h]

    if (param_count > 5)
    {
        opcode = OP_CallBuiltinMethod;
    }
    else
    {
        opcode = (Opcode_t)((int)OP_CallBuiltinMethod0 + param_count);
    }

    EmitOpcode(opcode, -param_count, 1);

    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 9);
    if (opcode == OP_CallBuiltinMethod)
        EmitByte(param_count);
}

void EmitAssignmentPos()
{
    char *v0; // eax

    if (scrVarPub.developer && scrCompilePub.developer_statement != 3)
    {
        v0 = TempMalloc(0);
        Scr_AddAssignmentPos(v0);
    }
}

void __cdecl EmitMethod(
    sval_u expr,
    sval_u func_name,
    sval_u params,
    sval_u methodSourcePos,
    bool bStatement,
    scr_block_s *block)
{
    __int16 v6; // ax
    char *savedPos; // [esp+8h] [ebp-28h] BYREF
    unsigned int methId; // [esp+Ch] [ebp-24h]
    void(__cdecl * meth)(scr_entref_t); // [esp+10h] [ebp-20h]
    int param_count; // [esp+14h] [ebp-1Ch]
    unsigned int name; // [esp+18h] [ebp-18h]
    const char *pName; // [esp+1Ch] [ebp-14h] BYREF
    int type; // [esp+20h] [ebp-10h] BYREF
    sval_u sourcePos; // [esp+24h] [ebp-Ch]
    VariableValue value; // [esp+28h] [ebp-8h] BYREF

    savedPos = 0;
    name = Scr_GetBuiltin(func_name);
    if (!name)
        goto script_method;
    pName = SL_ConvertToString(name);
    sourcePos = func_name.node[2];
    if (scrCompilePub.developer_statement == 3)
    {
        type = 0;
        meth = Scr_GetMethod(&pName, &type);
    }
    else
    {
        methId = FindVariable(scrCompilePub.builtinMeth, name);
        if (methId)
        {
            value = Scr_EvalVariable(methId);
            type = Scr_GetUncacheType(value.type);
            meth = (void(*)(scr_entref_t))value.u.intValue;
        }
        else
        {
            type = 0;
            meth = Scr_GetMethod(&pName, &type);
            methId = GetNewVariable(scrCompilePub.builtinMeth, name);
            value.type = Scr_GetCacheType(type);
            value.u.intValue = (int)meth;
            SetVariableValue(methId, &value);
        }
    }
    if (meth)
    {
        if (type == 1 && (Scr_BeginDevScript(&type, &savedPos), type == 1) && !bStatement)
        {
            CompileError(
                sourcePos.stringValue,
                "return value of developer command can not be accessed if not in a /# ... #/ comment");
        }
        else
        {
            param_count = EmitExpressionList(params, block);
            EmitPrimitiveExpression(expr, block);
            if (param_count < 256)
            {
                Scr_CompileRemoveRefToString(name);
                EmitCallBuiltinMethodOpcode(param_count, sourcePos);
                v6 = AddFunction((int)meth, pName);
                EmitShort(v6);
                AddOpcodePos(methodSourcePos.stringValue, 0);
                AddExpressionListOpcodePos(params);
                if (bStatement)
                    EmitDecTop();
                EmitAssignmentPos();
                if (type == 1)
                    Scr_EndDevScript(1, &savedPos);
            }
            else
            {
                CompileError(sourcePos.stringValue, "parameter count exceeds 256");
            }
        }
    }
    else
    {
    script_method:
        if (scrCompilePub.developer_statement == 3)
        {
            CompileError(*(unsigned int*)(func_name.type + 8), "unknown builtin method");
        }
        else
        {
            EmitPreFunctionCall(func_name);
            param_count = EmitExpressionList(params, block);
            EmitPrimitiveExpression(expr, block);
            EmitPostFunctionCall(func_name, param_count, 1, block);
            AddOpcodePos(methodSourcePos.stringValue, 0);
            AddExpressionListOpcodePos(params);
            if (bStatement)
                EmitDecTop();
        }
    }
}

void __cdecl EmitCallExpressionFieldObject(sval_u expr, scr_block_s *block)
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

void __cdecl EmitSelfObject(sval_u sourcePos)
{
    EmitOpcode(OP_GetSelfObject, 0, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitLevelObject(sval_u sourcePos)
{
    EmitOpcode(OP_GetLevelObject, 0, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitAnimObject(sval_u sourcePos)
{
    EmitOpcode(OP_GetAnimObject, 0, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 1);
}

sval_u *__cdecl GetSingleParameter(sval_u exprlist)
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

void __cdecl EmitExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if (expr.node[0].type == ENUM_primitive_expression)
    {
        EmitPrimitiveExpressionFieldObject(expr.node[1], expr.node[2], block);
        return;
    }

    CompileError(sourcePos.sourcePosValue, "not an object");
}

void __cdecl EmitExpressionListFieldObject(sval_u exprlist, sval_u sourcePos, scr_block_s *block)
{
    sval_u *node = GetSingleParameter(exprlist);

    if (node)
    {
        EmitExpressionFieldObject(node[0].node[0], node[0].node[1], block);
        return;
    }

    CompileError(sourcePos.sourcePosValue, "not an object");
}

void __cdecl EmitPrimitiveExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block)
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

void __cdecl EmitFieldVariable(sval_u expr, sval_u field, sval_u sourcePos, scr_block_s *block)
{
    EmitPrimitiveExpressionFieldObject(expr, sourcePos, block);
    EmitOpcode(OP_EvalFieldVariable, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitCanonicalString(field.stringValue);
}

void __cdecl EmitObject(sval_u expr, sval_u sourcePos)
{
    signed int ObjectType; // [esp+0h] [ebp-18h]
    const char *classnum; // [esp+4h] [ebp-14h]
    char *s; // [esp+Ch] [ebp-Ch]
    const char *entnum; // [esp+10h] [ebp-8h]
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
                    EmitOpcode(OP_thread_object, 1, 0);
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
    classnum = (const char*)Scr_GetClassnumForCharId(*s);
    if ((int)classnum < 0)
        goto LABEL_17;
    entnum = (const char*)atoi(s + 1); // KISAKTODO: seems wrong
    if (!entnum && s[1] != 48)
        goto LABEL_17;
    EmitOpcode(OP_object, 1, 0);
    EmitCodepos(classnum);
    EmitCodepos(entnum);
}

void __cdecl EmitVariableExpression(sval_u expr, scr_block_s *block)
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

void __cdecl EmitGetFunction(sval_u func, sval_u sourcePos)
{
    EmitOpcode(OP_GetFunction, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 3);
    EmitFunction(func, sourcePos);
}

void __cdecl EmitCallExpression(sval_u expr, bool bStatement, scr_block_s *block)
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

void __cdecl EmitSelf(sval_u sourcePos)
{
    EmitOpcode(OP_GetSelf, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitLevel(sval_u sourcePos)
{
    EmitOpcode(OP_GetLevel, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitGame(sval_u sourcePos)
{
    EmitOpcode(OP_GetGame, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitAnim(sval_u sourcePos)
{
    EmitOpcode(OP_GetAnim, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl Scr_CreateVector(VariableCompileValue *constValue, VariableValue *value)
{
    int type; // [esp+0h] [ebp-14h]
    int i; // [esp+4h] [ebp-10h]
    float vec[3]; // [esp+8h] [ebp-Ch] BYREF

    for (i = 0; i < 3; ++i)
    {
        type = constValue[i].value.type;
        if (type == 5)
        {
            vec[2 - i] = constValue[i].value.u.floatValue;
        }
        else
        {
            if (type != 6)
            {
                CompileError(constValue[i].sourcePos.stringValue, "type %s is not a float", var_typename[type]);
                return;
            }
            vec[2 - i] = constValue[i].value.u.intValue;
        }
    }
    value->type = VAR_VECTOR;
    value->u.intValue = (int)Scr_AllocVector(vec);
}

void __cdecl Scr_PushValue(VariableCompileValue *constValue)
{
    if (scrCompilePub.value_count < 32)
        scrCompileGlob.value_start[scrCompilePub.value_count++] = *constValue;
    else
        CompileError(constValue->sourcePos.stringValue, "VALUE_STACK_SIZE exceeded");
}

bool __cdecl EmitOrEvalPrimitiveExpressionList(
    sval_u exprlist,
    sval_u sourcePos,
    VariableCompileValue *constValue,
    scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-18h]
    VariableCompileValue constValue2; // [esp+4h] [ebp-14h] BYREF
    int expr_count; // [esp+10h] [ebp-8h]
    bool success; // [esp+17h] [ebp-1h]

    if (!constValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2536, 0, "%s", "constValue");
    expr_count = GetExpressionCount(exprlist);
    if (expr_count == 1)
        return EmitOrEvalExpression(exprlist.node[0].node[0].type, constValue, block); // KISAKTODO: might be wrong 
    if (expr_count == 3)
    {
        success = true;
        for (node = exprlist.node[0].node; node; node = node[1].node)
        {
            if (success)
            {
                success = EmitOrEvalExpression(node->node->type, &constValue2, block);
                if (success)
                    Scr_PushValue(&constValue2);
            }
            else
            {
                EmitExpression(node->node->type, block);
            }
        }
        if (success)
        {
            if (scrCompilePub.value_count < 3)
                MyAssertHandler(".\\script\\scr_compiler.cpp", 2565, 0, "%s", "scrCompilePub.value_count >= 3");
            scrCompilePub.value_count -= 3;
            Scr_CreateVector(&scrCompileGlob.value_start[scrCompilePub.value_count], &constValue->value);
            constValue->sourcePos = sourcePos;
            return 1;
        }
        else
        {
            EmitOpcode(OP_vector, -2, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            AddExpressionListOpcodePos(exprlist);
            return 0;
        }
    }
    else
    {
        CompileError(sourcePos.stringValue, "expression list must have 1 or 3 parameters");
        return 0;
    }
}

void __cdecl EmitSize(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    EmitPrimitiveExpression(expr, block);
    EmitOpcode(OP_size, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitEmptyArray(sval_u sourcePos)
{
    EmitOpcode(OP_EmptyArray, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}


void __cdecl EmitAnimation(sval_u anim, sval_u sourcePos)
{
    EmitOpcode(OP_GetAnimation, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    EmitCodepos((const char*)0xFFFFFFFF);
    if (scrCompilePub.developer_statement != 2)
        Scr_EmitAnimation((char*)scrCompileGlob.codePos, anim.stringValue, sourcePos.stringValue);
    Scr_CompileRemoveRefToString(anim.stringValue);
}

void __cdecl EmitAnimTree(sval_u sourcePos)
{
    if (scrAnimPub.animTreeIndex)
        EmitGetInteger(scrAnimPub.animTreeIndex, sourcePos);
    else
        CompileError(sourcePos.stringValue, "#using_animtree was not specified");
}

void __cdecl EmitBreakOn(sval_u expr, sval_u param, sval_u sourcePos)
{
    CompileError(sourcePos.stringValue, "illegal function name");
}

void __cdecl EvalInteger(int value, sval_u sourcePos, VariableCompileValue *constValue)
{
    iassert(constValue);

    constValue->value.type = VAR_INTEGER;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalFloat(float value, sval_u sourcePos, VariableCompileValue *constValue)
{
    iassert(constValue);

    constValue->value.type = VAR_FLOAT;
    constValue->value.u.floatValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue)
{
    iassert(constValue);

    constValue->value.type = VAR_STRING;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalIString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue)
{
    iassert(constValue);

    constValue->value.type = VAR_ISTRING;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalUndefined(sval_u sourcePos, VariableCompileValue *constValue)
{
    iassert(constValue);

    constValue->value.type = VAR_UNDEFINED;
    constValue->sourcePos = sourcePos;
}

char __cdecl EvalBinaryOperatorExpression(
    sval_u expr1,
    sval_u expr2,
    sval_u opcode,
    sval_u sourcePos,
    VariableCompileValue *constValue)
{
    VariableCompileValue constValue2; // [esp+0h] [ebp-18h] BYREF
    VariableCompileValue constValue1; // [esp+Ch] [ebp-Ch] BYREF

    if (!EvalExpression(expr1, &constValue1))
        return 0;
    if (!EvalExpression(expr2, &constValue2))
        return 0;
    AddRefToValue(constValue1.value.type, constValue1.value.u);
    AddRefToValue(constValue2.value.type, constValue2.value.u);
    Scr_EvalBinaryOperator(opcode.type, &constValue1.value, &constValue2.value);
    if (scrVarPub.error_message)
    {
        CompileError(sourcePos.stringValue, "%s", scrVarPub.error_message);
        return 0;
    }
    else
    {
        constValue->value.u.intValue = constValue1.value.u.intValue;
        constValue->value.type = constValue1.value.type;
        constValue->sourcePos = sourcePos;
        return 1;
    }
}

char __cdecl EvalExpression(sval_u expr, VariableCompileValue *constValue)
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

bool __cdecl EvalPrimitiveExpressionList(sval_u exprlist, sval_u sourcePos, VariableCompileValue *constValue)
{
    sval_u *node; // [esp+0h] [ebp-30h]
    VariableCompileValue constValue2[3]; // [esp+4h] [ebp-2Ch] BYREF
    int expr_count; // [esp+28h] [ebp-8h]
    int i; // [esp+2Ch] [ebp-4h]

    iassert(constValue);

    expr_count = GetExpressionCount(exprlist);

    if (expr_count == 1)
        return EvalExpression(exprlist.node[0].node[0].node[0], constValue);

    if (expr_count != 3)
        return false;

    i = 0;
    for (node = exprlist.node[0].node; node; node = node[1].node, i++)
    {
        if (!EvalExpression(node[0].node[0], &constValue2[i]))
            return false;
        ++i;
    }
    Scr_CreateVector(constValue2, &constValue->value);
    constValue->sourcePos = sourcePos;
    return true;
}

char __cdecl EvalPrimitiveExpression(sval_u expr, VariableCompileValue *constValue)
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

void __cdecl EmitCastBool(sval_u sourcePos)
{
    EmitOpcode(OP_CastBool, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitBoolOrExpression(
    sval_u expr1,
    sval_u expr2,
    sval_u expr1sourcePos,
    sval_u expr2sourcePos,
    scr_block_s *block)
{
    unsigned __int8 *pos; // [esp+0h] [ebp-Ch]
    char *offset; // [esp+4h] [ebp-8h]
    char *nextPos; // [esp+8h] [ebp-4h]

    EmitExpression(expr1, block);
    EmitOpcode(OP_JumpOnTrueExpr, -1, 0);
    AddOpcodePos(expr1sourcePos.stringValue, 0);
    EmitShort(0);
    pos = scrCompileGlob.codePos;
    nextPos = TempMalloc(0);
    EmitExpression(expr2, block);
    EmitCastBool(expr2sourcePos);
    offset = (char*)(TempMalloc(0) - nextPos);
    if (offset >= (char*)0x10000)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2731, 0, "%s", "offset < 65536");
    *pos = (unsigned char)offset;
}

char __cdecl EmitOrEvalPrimitiveExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block)
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

void __cdecl EmitBoolAndExpression(
    sval_u expr1,
    sval_u expr2,
    sval_u expr1sourcePos,
    sval_u expr2sourcePos,
    scr_block_s *block)
{
    unsigned __int8 *pos; // [esp+0h] [ebp-Ch]
    char *offset; // [esp+4h] [ebp-8h]
    char *nextPos; // [esp+8h] [ebp-4h]

    EmitExpression(expr1, block);
    EmitOpcode(OP_JumpOnFalseExpr, -1, 0);
    AddOpcodePos(expr1sourcePos.stringValue, 0);
    EmitShort(0);
    pos = scrCompileGlob.codePos;
    nextPos = TempMalloc(0);
    EmitExpression(expr2, block);
    EmitCastBool(expr2sourcePos);
    offset = (char*)(TempMalloc(0) - nextPos);
    if (offset >= (char*)0x10000)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2751, 0, "%s", "offset < 65536");
    *pos = (unsigned char)offset;
}

int Scr_PopValue()
{
    if (!scrCompilePub.value_count)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1209, 0, "%s", "scrCompilePub.value_count");
    return --scrCompilePub.value_count;
}

char __cdecl EmitOrEvalBinaryOperatorExpression(
    sval_u expr1,
    sval_u expr2,
    sval_u opcode,
    sval_u sourcePos,
    VariableCompileValue *constValue,
    scr_block_s *block)
{
    VariableCompileValue constValue2; // [esp+0h] [ebp-18h] BYREF
    VariableCompileValue constValue1; // [esp+Ch] [ebp-Ch] BYREF

    if (!EmitOrEvalExpression(expr1, &constValue1, block))
    {
        EmitExpression(expr2, block);
    emitOpcode:
        EmitOpcode((Opcode_t)opcode.type, -1, 0);
        AddOpcodePos(sourcePos.stringValue, 0);
        return 0;
    }
    Scr_PushValue(&constValue1);
    if (!EmitOrEvalExpression(expr2, &constValue2, block))
        goto emitOpcode;
    Scr_PopValue();
    Scr_EvalBinaryOperator(opcode.type, &constValue1.value, &constValue2.value);
    if (scrVarPub.error_message)
    {
        CompileError(sourcePos.stringValue, "%s", scrVarPub.error_message);
        return 0;
    }
    else
    {
        constValue->value.u.intValue = constValue1.value.u.intValue;
        constValue->value.type = constValue1.value.type;
        constValue->sourcePos = sourcePos;
        return 1;
    }
}

void __cdecl EmitBoolNot(sval_u sourcePos)
{
    EmitOpcode(OP_BoolNot, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitBoolComplement(sval_u sourcePos)
{
    EmitOpcode(OP_BoolComplement, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

char __cdecl EmitOrEvalExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block)
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

void __cdecl EmitExpression(sval_u expr, scr_block_s *block)
{
    VariableCompileValue constValue; // [esp+0h] [ebp-Ch] BYREF

    if (EmitOrEvalExpression(expr, &constValue, block))
        EmitValue(&constValue);

}

void __cdecl EmitLocalVariableRef(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    int index; // [esp+0h] [ebp-4h]

    if (scrCompilePub.developer_statement == 3)
    {
        EmitOpcode(OP_EvalLocalVariableRef, 0, 0);
        EmitCanonicalString(expr.stringValue);
    }
    else
    {
        index = Scr_FindLocalVarIndex(expr.stringValue, sourcePos, 1, block);
        if (index)
        {
            EmitOpcode(OP_EvalLocalVariableRefCached, 0, 0);
        }
        else
        {
            EmitOpcode(OP_EvalLocalVariableRefCached0, 0, 0);
        }
        EmitPreAssignmentPos();
        if (index)
            EmitByte(index);
        AddOpcodePos(sourcePos.stringValue, 1);
    }
}

void __cdecl EmitEvalArrayRef(sval_u sourcePos, sval_u indexSourcePos)
{
    EmitOpcode(OP_EvalArrayRef, -1, 0);
    AddOpcodePos(indexSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitArrayVariableRef(
    sval_u expr,
    sval_u index,
    sval_u sourcePos,
    sval_u indexSourcePos,
    scr_block_s *block)
{
    EmitExpression(index, block);
    EmitArrayPrimitiveExpressionRef(expr, sourcePos, block);
    EmitEvalArrayRef(sourcePos, indexSourcePos);
}

void __cdecl EmitFieldVariableRef(sval_u expr, sval_u field, sval_u sourcePos, scr_block_s *block)
{
    EmitPrimitiveExpressionFieldObject(expr, sourcePos, block);
    EmitOpcode(OP_EvalFieldVariableRef, 0, 0);
    EmitCanonicalString(field.stringValue);
}

void __cdecl EmitVariableExpressionRef(sval_u expr, scr_block_s *block)
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

void __cdecl EmitGameRef(sval_u sourcePos)
{
    EmitOpcode(OP_GetGameRef, 0, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitClearArray(sval_u sourcePos, sval_u indexSourcePos)
{
    EmitOpcode(OP_ClearArray, -1, 0);
    AddOpcodePos(indexSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitAssignmentPos();
}

void __cdecl EmitArrayPrimitiveExpressionRef(sval_u expr, sval_u sourcePos, scr_block_s *block)
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

void __cdecl EmitClearArrayVariable(
    sval_u expr,
    sval_u index,
    sval_u sourcePos,
    sval_u indexSourcePos,
    scr_block_s *block)
{
    EmitExpression(index, block);
    EmitArrayPrimitiveExpressionRef(expr, sourcePos, block);
    EmitClearArray(sourcePos, indexSourcePos);
}

void __cdecl EmitClearFieldVariable(
    sval_u expr,
    sval_u field,
    sval_u sourcePos,
    sval_u rhsSourcePos,
    scr_block_s *block)
{
    EmitPrimitiveExpressionFieldObject(expr, sourcePos, block);
    EmitOpcode(OP_ClearFieldVariable, 0, 0);
    AddOpcodePos(rhsSourcePos.stringValue, 0);
    EmitCanonicalString(field.stringValue);
    EmitAssignmentPos();
}

char __cdecl EmitClearVariableExpression(sval_u expr, sval_u rhsSourcePos, scr_block_s *block)
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

void __cdecl EmitSetVariableField(sval_u sourcePos)
{
    EmitOpcode(OP_SetVariableField, -1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitAssignmentPos();
}

void __cdecl EmitAssignmentStatement(sval_u lhs, sval_u rhs, sval_u sourcePos, sval_u rhsSourcePos, scr_block_s *block)
{
    if (!IsUndefinedExpression(rhs) || !EmitClearVariableExpression(lhs, rhsSourcePos, block))
    {
        EmitExpression(rhs, block);
        EmitVariableExpressionRef(lhs, block);
        EmitSetVariableField(sourcePos);
    }
}

void __cdecl EmitCallExpressionStatement(sval_u expr, scr_block_s *block)
{
    EmitCallExpression(expr, 1, block);
}

void EmitReturn()
{
    EmitOpcode(OP_Return, -1, 0);
    EmitPreAssignmentPos();
}

void __cdecl EmitReturnStatement(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if (!block->abortLevel)
        block->abortLevel = 3;
    EmitExpression(expr, block);
    EmitReturn();
    AddOpcodePos(sourcePos.stringValue, 0);
}

void EmitEnd()
{
    EmitOpcode(OP_End, 0, 0);
    EmitPreAssignmentPos();
}

void __cdecl EmitEndStatement(sval_u sourcePos, scr_block_s *block)
{
    if (!block->abortLevel)
        block->abortLevel = 3;
    EmitEnd();
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitWaitStatement(sval_u expr, sval_u sourcePos, sval_u waitSourcePos, scr_block_s *block)
{
    EmitExpression(expr, block);
    EmitOpcode(OP_wait, -1, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

int __cdecl Scr_FindLocalVar(scr_block_s *block, int startIndex, unsigned int name)
{
    while (startIndex < block->localVarsCount)
    {
        if (block->localVars[startIndex].name == name)
            return startIndex;
        ++startIndex;
    }
    return -1;
}

void __cdecl Scr_CheckLocalVarsCount(int localVarsCount)
{
    if (localVarsCount >= 64)
        Com_Error(ERR_DROP, "LOCAL_VAR_STACK_SIZE exceeded");
}

void __cdecl Scr_TransferBlock(scr_block_s *from, scr_block_s *to)
{
    unsigned int name;
    int i, j;

    assert(to->localVarsPublicCount <= from->localVarsCount);

    for (i = 0; i < to->localVarsPublicCount || i < from->localVarsCreateCount; i++)
    {
        name = from->localVars[i].name;
        j = Scr_FindLocalVar(to, i, name);

        if (j < 0)
        {
            j = to->localVarsCount;

            Scr_CheckLocalVarsCount(j);
            to->localVarsCount++;
        }

        assert(to->localVarsPublicCount <= from->localVarsCount);

        if (j >= to->localVarsPublicCount)
        {
            to->localVarsPublicCount++;
        }

        while (j > i)
        {
            to->localVars[j] = *(scr_localVar_t *)&to->localVarsInitBits[sizeof(scr_localVar_t) * j + sizeof(unsigned int)];
            j--;
        }

        to->localVars[i].name = name;

        if ((1 << (i & 7)) & from->localVarsInitBits[i >> 3])
        {
            to->localVarsInitBits[i >> 3] |= 1 << (i & 7);
        }
    }

    assert(from->localVarsCreateCount <= to->localVarsPublicCount);

    to->localVarsCreateCount = from->localVarsCreateCount;
    to->abortLevel = SCR_ABORT_NONE;
}

void __cdecl EmitRemoveLocalVars(scr_block_s *block, scr_block_s *outerBlock)
{
    int removeCount; // [esp+0h] [ebp-4h]

    if (!block->abortLevel)
    {
        if (block->localVarsCreateCount < block->localVarsPublicCount)
            MyAssertHandler(
                ".\\script\\scr_compiler.cpp",
                832,
                0,
                "%s",
                "block->localVarsCreateCount >= block->localVarsPublicCount");
        if (block->localVarsPublicCount < outerBlock->localVarsPublicCount)
            MyAssertHandler(
                ".\\script\\scr_compiler.cpp",
                833,
                0,
                "%s",
                "block->localVarsPublicCount >= outerBlock->localVarsPublicCount");
        removeCount = block->localVarsCreateCount - outerBlock->localVarsPublicCount;
        if (removeCount < 0)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 835, 0, "%s", "removeCount >= 0");
        if (removeCount)
        {
            EmitOpcode(OP_RemoveLocalVariables, 0, 0);
            EmitByte(removeCount);
            block->localVarsCreateCount = block->localVarsPublicCount;
        }
    }
}

void __cdecl EmitNOP2(bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
    unsigned int checksum; // [esp+0h] [ebp-4h]

    checksum = scrVarPub.checksum;
    if (lastStatement)
    {
        EmitEnd();
        AddOpcodePos(endSourcePos, 1);
    }
    else
    {
        EmitRemoveLocalVars(block, block);
    }
    scrVarPub.checksum = checksum + 1;
}

void __cdecl EmitIfStatement(
    sval_u expr,
    sval_u stmt,
    sval_u sourcePos,
    bool lastStatement,
    unsigned int endSourcePos,
    scr_block_s *block,
    sval_u *ifStatBlock)
{
    unsigned __int8 *pos; // [esp+0h] [ebp-Ch]
    char *offset; // [esp+4h] [ebp-8h]
    char *nextPos; // [esp+8h] [ebp-4h]

    EmitExpression(expr, block);
    EmitOpcode(OP_JumpOnFalse, -1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitShort(0);
    pos = scrCompileGlob.codePos;
    nextPos = TempMalloc(0);
    Scr_TransferBlock(block, ifStatBlock->block);
    EmitStatement(stmt, lastStatement, endSourcePos, ifStatBlock->block);
    iassert(ifStatBlock->block->localVarsPublicCount == block->localVarsCreateCount);
    EmitNOP2(lastStatement, endSourcePos, ifStatBlock->block);
    offset = (char*)(TempMalloc(0) - nextPos);
    if (offset >= (char*)0x10000)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3169, 0, "%s", "offset < 65536");
    *pos = (unsigned char)offset;
}

void __cdecl EmitIfElseStatement(
    sval_u expr,
    sval_u stmt1,
    sval_u stmt2,
    sval_u sourcePos,
    sval_u elseSourcePos,
    bool lastStatement,
    unsigned int endSourcePos,
    scr_block_s *block,
    sval_u *ifStatBlock,
    sval_u *elseStatBlock)
{
    unsigned int checksum; // [esp+0h] [ebp-24h]
    char *offset; // [esp+4h] [ebp-20h]
    char *nextPos1; // [esp+8h] [ebp-1Ch]
    unsigned __int8 *pos1; // [esp+Ch] [ebp-18h]
    scr_block_s *childBlocks[2]; // [esp+10h] [ebp-14h] BYREF
    const char *nextPos2; // [esp+18h] [ebp-Ch]
    int childCount; // [esp+1Ch] [ebp-8h]
    char *pos2; // [esp+20h] [ebp-4h]

    childCount = 0;
    EmitExpression(expr, block);
    EmitOpcode(OP_JumpOnFalse, -1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitShort(0);
    pos1 = scrCompileGlob.codePos;
    nextPos1 = TempMalloc(0);
    Scr_TransferBlock(block, ifStatBlock->block);
    EmitStatement(stmt1, lastStatement, endSourcePos, ifStatBlock->block);
    EmitRemoveLocalVars(ifStatBlock->block, ifStatBlock->block);
    if (!*(int*)ifStatBlock->type)
        childBlocks[childCount++] = (scr_block_s*)ifStatBlock->type;
    checksum = scrVarPub.checksum;
    if (lastStatement)
    {
        EmitEnd();
        EmitCodepos(0);
        AddOpcodePos(endSourcePos, 1);
        pos2 = 0;
        nextPos2 = 0;
    }
    else
    {
        EmitOpcode(OP_jump, 0, 0);
        AddOpcodePos(elseSourcePos.stringValue, 1);
        EmitCodepos(0);
        pos2 = (char*)scrCompileGlob.codePos;
        nextPos2 = TempMalloc(0);
    }
    scrVarPub.checksum = checksum + 1;
    offset = (char*)(TempMalloc(0) - nextPos1);
    if (offset >= (char*)0x10000)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3233, 0, "%s", "offset < 65536");
    *pos1 = (unsigned char)offset;
    Scr_TransferBlock(block, elseStatBlock->block);
    EmitStatement(stmt2, lastStatement, endSourcePos, elseStatBlock->block);
    EmitNOP2(lastStatement, endSourcePos, elseStatBlock->block);
    if (!*(int*)elseStatBlock->type)
        childBlocks[childCount++] = (scr_block_s*)elseStatBlock->type;
    if (!lastStatement)
        *pos2 = TempMalloc(0) - nextPos2;
    Scr_InitFromChildBlocks(childBlocks, childCount, block);
}

void __cdecl EmitCreateLocalVars(scr_block_s *block)
{
    int i; // [esp+0h] [ebp-4h]

    if (block->localVarsPublicCount < block->localVarsCreateCount)
        MyAssertHandler(
            ".\\script\\scr_compiler.cpp",
            809,
            0,
            "%s",
            "block->localVarsPublicCount >= block->localVarsCreateCount");
    if (block->localVarsCreateCount != block->localVarsPublicCount)
    {
        for (i = block->localVarsCreateCount; i < block->localVarsPublicCount; ++i)
        {
            EmitOpcode(OP_CreateLocalVariable, 0, 0);
            EmitCanonicalStringConst(block->localVars[i].name);
            AddOpcodePos(block->localVars[i].sourcePos, 0);
        }
        block->localVarsCreateCount = block->localVarsPublicCount;
    }
}

ContinueStatementInfo *ConnectContinueStatements()
{
    ContinueStatementInfo *result; // eax
    ContinueStatementInfo *continueStatement; // [esp+0h] [ebp-8h]
    char *codePos; // [esp+4h] [ebp-4h]

    codePos = TempMalloc(0);
    result = scrCompileGlob.currentContinueStatement;
    for (continueStatement = scrCompileGlob.currentContinueStatement; continueStatement; continueStatement = result)
    {
        *continueStatement->codePos = codePos - continueStatement->nextCodePos;
        result = continueStatement->next;
    }
    return result;
}

BreakStatementInfo *ConnectBreakStatements()
{
    BreakStatementInfo *result; // eax
    BreakStatementInfo *breakStatement; // [esp+0h] [ebp-8h]
    char *codePos; // [esp+4h] [ebp-4h]

    if (scrCompilePub.value_count)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3022, 0, "%s", "!scrCompilePub.value_count");
    codePos = TempMalloc(0);
    result = scrCompileGlob.currentBreakStatement;
    for (breakStatement = scrCompileGlob.currentBreakStatement; breakStatement; breakStatement = result)
    {
        *breakStatement->codePos = codePos - breakStatement->nextCodePos;
        result = breakStatement->next;
    }
    return result;
}

void __cdecl Scr_CheckMaxSwitchCases(int count)
{
    if (count >= 1024)
        Com_Error(ERR_DROP, "MAX_SWITCH_CASES exceeded");
}

void __cdecl Scr_AddContinueBlock(scr_block_s *block)
{
    if (!block->abortLevel && scrCompileGlob.continueChildBlocks && scrCompilePub.developer_statement != 2)
    {
        Scr_CheckMaxSwitchCases(*scrCompileGlob.continueChildCount);
        scrCompileGlob.continueChildBlocks[(*scrCompileGlob.continueChildCount)++] = block;
    }
}

void __cdecl EmitForStatement(
    sval_u stmt1,
    sval_u expr,
    sval_u stmt2,
    sval_u stmt,
    sval_u sourcePos,
    sval_u forSourcePos,
    scr_block_s *block,
    sval_u *forStatBlock,
    sval_u *forStatPostBlock)
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
    char *pos2; // [esp+44h] [ebp-Ch]
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
            if (constValue.value.type == 6 || constValue.value.type == 5)
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
        breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(4096, "EmitForStatement");
        scrCompileGlob.breakChildCount = &breakChildCount;
    }
    else
    {
        EmitOpcode(OP_JumpOnFalse, -1, 0);
        AddOpcodePos(sourcePos.stringValue, 0);
        EmitShort(0);
        pos2 = (char*)scrCompileGlob.codePos;
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
    EmitOpcode(OP_jumpback, 0, 0);
    AddOpcodePos(forSourcePos.stringValue, 0);

    if (stmt.node[0].type == ENUM_statement_list)
    {
        AddOpcodePos(stmt.node[3].sourcePosValue, SOURCE_TYPE_BREAKPOINT);
    }

    EmitShort(0);
    offset = TempMalloc(0) - pos1;
    iassert(offset < 65536);
    *scrCompileGlob.codePos = offset;
    if (pos2)
    {
        offset = TempMalloc(0) - nextPos2;
        iassert(offset < 65536);
        *pos2 = offset;
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

void __cdecl EmitWhileStatement(
    sval_u expr,
    sval_u stmt,
    sval_u sourcePos,
    sval_u whileSourcePos,
    scr_block_s *block,
    sval_u *whileStatBlock)
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
    char *pos2; // [esp+3Ch] [ebp-Ch]
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
        if (constValue.value.type == 6 || constValue.value.type == 5)
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
        EmitOpcode(OP_JumpOnFalse, -1, 0);
        AddOpcodePos(sourcePos.stringValue, 0);
        EmitShort(0);
        pos2 = (char*)scrCompileGlob.codePos;
        nextPos2 = TempMalloc(0);
        breakChildBlocks = 0;
    }
    scrCompileGlob.breakChildBlocks = breakChildBlocks;
    scrCompileGlob.bCanBreak = 1;
    scrCompileGlob.currentBreakStatement = 0;
    scrCompileGlob.bCanContinue = 1;
    scrCompileGlob.currentContinueStatement = 0;
    EmitStatement(stmt, 0, 0, whileStatBlock->block);
    if (*(int*)whileStatBlock->type != 3)
        *(int*)whileStatBlock->type = 0;
    scrCompileGlob.bCanBreak = 0;
    scrCompileGlob.bCanContinue = 0;
    ConnectContinueStatements();
    EmitOpcode(OP_jumpback, 0, 0);
    AddOpcodePos(whileSourcePos.stringValue, 0);

    if (stmt.node[0].type == ENUM_statement_list)
    {
        AddOpcodePos(stmt.node[3].sourcePosValue, SOURCE_TYPE_BREAKPOINT);
    }

    EmitShort(0);
    offset = TempMalloc(0) - pos1;
    iassert(65536);
    *scrCompileGlob.codePos = offset;
    if (pos2)
    {
        offset = TempMalloc(0) - nextPos2;
        iassert(offset < 65536);
        *pos2 = offset;
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

void __cdecl EmitIncStatement(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    iassert(!scrCompileGlob.forceNotCreate);
    scrCompileGlob.forceNotCreate = 1;
    EmitVariableExpressionRef(expr, block);
    iassert(scrCompileGlob.forceNotCreate);

    scrCompileGlob.forceNotCreate = 0;
    EmitOpcode(OP_inc, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitSetVariableField(sourcePos);
}

void __cdecl EmitDecStatement(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    iassert(!scrCompileGlob.forceNotCreate);
    scrCompileGlob.forceNotCreate = 1;
    EmitVariableExpressionRef(expr, block);
    iassert(scrCompileGlob.forceNotCreate);

    scrCompileGlob.forceNotCreate = 0;
    EmitOpcode(OP_dec, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitSetVariableField(sourcePos);
}

void __cdecl EmitBinaryEqualsOperatorExpression(
    sval_u lhs,
    sval_u rhs,
    sval_u opcode,
    sval_u sourcePos,
    scr_block_s *block)
{
    iassert(!scrCompileGlob.bConstRefCount);
    scrCompileGlob.bConstRefCount = 1;
    EmitVariableExpression(lhs, block);
    iassert(scrCompileGlob.bConstRefCount);

    scrCompileGlob.bConstRefCount = 0;
    EmitExpression(rhs, block);
    EmitOpcode((Opcode_t)opcode.type, -1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitVariableExpressionRef(lhs, block);
    EmitSetVariableField(sourcePos);
}

char __cdecl Scr_IsLastStatement(sval_u *node)
{
    if (!node)
        return 1;
    if (scrVarPub.developer_script)
        return 0;
    while (node)
    {
        if (node[0].node[0].type != ENUM_developer_statement_list)
            return 0;
        node = node[1].node;
    }
    return 1;
}

void __cdecl EmitStatementList(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
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

void __cdecl EmitDeveloperStatementList(sval_u val, sval_u sourcePos, scr_block_s *block, sval_u *devStatBlock)
{
    char *savedPos; // [esp+0h] [ebp-8h]
    unsigned int savedChecksum; // [esp+4h] [ebp-4h]

    if (scrCompilePub.developer_statement)
    {
        CompileError(sourcePos.stringValue, "cannot recurse /#");
    }
    else
    {
        savedChecksum = scrVarPub.checksum;
        Scr_TransferBlock(block, devStatBlock->block);
        if (scrVarPub.developer_script)
        {
            scrCompilePub.developer_statement = SCR_DEV_YES;
            EmitStatementList(val, 0, 0, devStatBlock->block);
            EmitRemoveLocalVars(devStatBlock->block, devStatBlock->block);
        }
        else
        {
            savedPos = TempMalloc(0);
            scrCompilePub.developer_statement = SCR_DEV_IGNORE;
            EmitStatementList(val, 0, 0, devStatBlock->block);
            TempMemorySetPos(savedPos);
        }
        scrCompilePub.developer_statement = 0;
        scrVarPub.checksum = savedChecksum;
    }
}

void __cdecl EmitSafeSetWaittillVariableField(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    unsigned __int8 index; // [esp+0h] [ebp-4h]

    index = Scr_FindLocalVarIndex(expr.stringValue, sourcePos, 1, block);
    EmitOpcode(OP_SafeSetWaittillVariableFieldCached, 0, 0);
    EmitPreAssignmentPos();
    EmitByte(index);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitAssignmentPos();
}

void __cdecl EmitFormalWaittillParameterListRefInternal(sval_u *node, scr_block_s *block)
{
    while (1)
    {
        node = node[1].node;
        if (!node)
            break;
        EmitSafeSetWaittillVariableField(node[0].node[0], node[0].node[1], block);
    }
}

void __cdecl EmitWaittillStatement(
    sval_u obj,
    sval_u exprlist,
    sval_u sourcePos,
    sval_u waitSourcePos,
    scr_block_s *block)
{
    sval_u *node = exprlist.node[0].node[1].node;

    iassert(node);

    EmitExpression(node[0].node[0], block);
    EmitPrimitiveExpression(obj, block);

    EmitOpcode(OP_waittill, -2, 0);

    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    AddOpcodePos(node[0].node[1].sourcePosValue, 0);

    EmitFormalWaittillParameterListRefInternal(node, block);
    EmitOpcode(OP_clearparams, 0, 0);
}

void __cdecl EmitWaittillmatchStatement(
    sval_u obj,
    sval_u exprlist,
    sval_u sourcePos,
    sval_u waitSourcePos,
    scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-8h]
    int exprCount; // [esp+4h] [ebp-4h]

    node = exprlist.node[0].node[1].node;

    iassert(node);

    for (exprCount = 0; ; ++exprCount)
    {
        node = node[1].node;
        if (!node)
            break;
        EmitExpression(node->node->type, block);
    }

    node = exprlist.node[0].node[1].node;

    iassert(node);
    EmitExpression(node[0].node[0], block);
    EmitPrimitiveExpression(obj, block);
    EmitOpcode(OP_waittillmatch, -2 - exprCount, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    AddOpcodePos(node[0].node[1].sourcePosValue, 0);
    while (1)
    {
        node = node[1].node;

        if (!node)
        {
            break;
        }

        AddOpcodePos(node[0].node[1].sourcePosValue, 0);
    }
    iassert(exprCount < 256);

    EmitByte(exprCount);
    EmitOpcode(OP_clearparams, 0, 0);
}

void __cdecl EmitWaittillFrameEnd(sval_u sourcePos)
{
    EmitOpcode(OP_waittillFrameEnd, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitNotifyStatement(
    sval_u obj,
    sval_u exprlist,
    sval_u sourcePos,
    sval_u notifySourcePos,
    scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-Ch]
    sval_u *start_node; // [esp+4h] [ebp-8h]
    int expr_count; // [esp+8h] [ebp-4h]

    EmitOpcode(OP_voidCodepos, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);

    expr_count = 0;
    start_node = 0;

    for (node = exprlist.node[0].node; node; node = node[1].node)
    {
        start_node = node;
        EmitExpression(node[0].node[0], block);
        expr_count++;
    }

    iassert(start_node);

    EmitPrimitiveExpression(obj, block);
    EmitOpcode(OP_notify, -expr_count - 2, 0);

    AddOpcodePos(notifySourcePos.stringValue, 16);
    AddOpcodePos(start_node[0].node[1].sourcePosValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitEndOnStatement(sval_u obj, sval_u expr, sval_u sourcePos, sval_u exprSourcePos, scr_block_s *block)
{
    EmitExpression(expr, block);
    EmitPrimitiveExpression(obj, block);
    EmitOpcode(OP_endon, -2, 0);
    AddOpcodePos(exprSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitCaseStatementInfo(unsigned int name, sval_u sourcePos)
{
    CaseStatementInfo *newCaseStatement; // [esp+0h] [ebp-4h]

    if (scrCompilePub.developer_statement == 2)
    {
        if (scrVarPub.developer_script)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 4300, 0, "%s", "!scrVarPub.developer_script");
    }
    else
    {
        newCaseStatement = (CaseStatementInfo*)Hunk_AllocateTempMemoryHigh(16, "EmitCaseStatementInfo");
        newCaseStatement->name = name;
        newCaseStatement->codePos = TempMalloc(0);
        newCaseStatement->sourcePos = sourcePos.stringValue;
        newCaseStatement->next = scrCompileGlob.currentCaseStatement;
        scrCompileGlob.currentCaseStatement = newCaseStatement;
    }
}

void __cdecl EmitCaseStatement(sval_u expr, sval_u sourcePos)
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

void __cdecl EmitDefaultStatement(sval_u sourcePos)
{
    EmitCaseStatementInfo(0, sourcePos);
}

void __cdecl Scr_AddBreakBlock(scr_block_s *block)
{
    if (!block->abortLevel && scrCompileGlob.breakChildBlocks && scrCompilePub.developer_statement != 2)
    {
        Scr_CheckMaxSwitchCases(*scrCompileGlob.breakChildCount);
        scrCompileGlob.breakChildBlocks[(*scrCompileGlob.breakChildCount)++] = block;
    }
}

void __cdecl EmitSwitchStatementList(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
    sval_u *node; // [esp+4h] [ebp-20h]
    sval_u *nextNode; // [esp+8h] [ebp-1Ch]
    int breakChildCount; // [esp+Ch] [ebp-18h] BYREF
    scr_block_s **breakChildBlocks; // [esp+10h] [ebp-14h]
    int *oldBreakChildCount; // [esp+14h] [ebp-10h]
    bool hasDefault; // [esp+1Bh] [ebp-9h]
    scr_block_s **oldBreakChildBlocks; // [esp+1Ch] [ebp-8h]
    scr_block_s *oldBreakBlock; // [esp+20h] [ebp-4h]

    oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
    oldBreakChildCount = scrCompileGlob.breakChildCount;
    oldBreakBlock = scrCompileGlob.breakBlock;
    breakChildCount = 0;
    breakChildBlocks = (scr_block_s**)Hunk_AllocateTempMemoryHigh(4096, "EmitSwitchStatementList");
    scrCompileGlob.breakChildBlocks = breakChildBlocks;
    scrCompileGlob.breakChildCount = &breakChildCount;
    scrCompileGlob.breakBlock = 0;
    hasDefault = 0;

    for (node = val.node[0].node[1].node; node; node = nextNode)
    {
        nextNode = node[1].node;
        if (node[0].node[0].type == ENUM_case || node[0].node[0].type == ENUM_default)
        {
            if (scrCompileGlob.breakBlock)
            {
                if (!scrCompileGlob.bCanBreak)
                    MyAssertHandler(".\\script\\scr_compiler.cpp", 4053, 0, "%s", "scrCompileGlob.bCanBreak");
                scrCompileGlob.bCanBreak = 0;
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
            if (scrCompileGlob.bCanBreak)
                MyAssertHandler(".\\script\\scr_compiler.cpp", 4072, 0, "%s", "!scrCompileGlob.bCanBreak");
            scrCompileGlob.bCanBreak = 1;
        }
        else
        {
            if (!scrCompileGlob.breakBlock)
            {
                CompileError(endSourcePos, "missing case statement");
                return;
            }
            if (lastStatement && Scr_IsLastStatement(nextNode))
                EmitStatement(node->type, 1, endSourcePos, scrCompileGlob.breakBlock);
            else
                EmitStatement(node->type, 0, endSourcePos, scrCompileGlob.breakBlock);
            if (scrCompileGlob.breakBlock && scrCompileGlob.breakBlock->abortLevel)
            {
                scrCompileGlob.breakBlock = 0;
                if (!scrCompileGlob.bCanBreak)
                    MyAssertHandler(".\\script\\scr_compiler.cpp", 4089, 0, "%s", "scrCompileGlob.bCanBreak");
                scrCompileGlob.bCanBreak = 0;
            }
        }
    }
    if (scrCompileGlob.breakBlock)
    {
        if (!scrCompileGlob.bCanBreak)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 4095, 0, "%s", "scrCompileGlob.bCanBreak");
        scrCompileGlob.bCanBreak = 0;
        EmitRemoveLocalVars(scrCompileGlob.breakBlock, scrCompileGlob.breakBlock);
    }
    if (hasDefault)
    {
        if (scrCompileGlob.breakBlock)
            Scr_AddBreakBlock(scrCompileGlob.breakBlock);
        Scr_InitFromChildBlocks(breakChildBlocks, breakChildCount, block);
    }
    scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
    scrCompileGlob.breakChildCount = oldBreakChildCount;
    scrCompileGlob.breakBlock = oldBreakBlock;
}

int __cdecl CompareCaseInfo(_DWORD *elem1, _DWORD *elem2)
{
    if (*elem1 <= *elem2)
        return *elem1 < *elem2;
    else
        return -1;
}

void __cdecl EmitSwitchStatement(
    sval_u expr,
    sval_u stmtlist,
    sval_u sourcePos,
    bool lastStatement,
    unsigned int endSourcePos,
    scr_block_s *block)
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
    EmitOpcode(OP_switch, -1, 0);
    EmitCodepos(0);
    pos1 = scrCompileGlob.codePos;
    nextPos1 = TempMalloc(0);
    scrCompileGlob.currentCaseStatement = 0;
    scrCompileGlob.currentBreakStatement = 0;
    EmitSwitchStatementList(stmtlist, lastStatement, endSourcePos, block);
    EmitOpcode(OP_endswitch, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitShort(0);
    pos2 = scrCompileGlob.codePos;
    *pos1 = scrCompileGlob.codePos - (unsigned char*)nextPos1;
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
    qsort(pos3, num, 8u, (int(*)(const void*, const void*))CompareCaseInfo);
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

void __cdecl EmitBreakStatement(sval_u sourcePos, scr_block_s *block)
{
    BreakStatementInfo *newBreakStatement; // [esp+0h] [ebp-4h]

    if (scrCompileGlob.bCanBreak && !block->abortLevel)
    {
        Scr_AddBreakBlock(block);
        if (!scrCompileGlob.breakBlock)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 4325, 0, "%s", "scrCompileGlob.breakBlock");
        EmitRemoveLocalVars(block, scrCompileGlob.breakBlock);

        block->abortLevel = 2;

        EmitOpcode(OP_jump, 0, 0);
        AddOpcodePos(sourcePos.stringValue, 1);

        EmitCodepos(0);

        if (scrCompilePub.developer_statement != 2)
        {
            newBreakStatement = (BreakStatementInfo*)Hunk_AllocateTempMemoryHigh(12, "EmitBreakStatement");
            newBreakStatement->codePos = (char*)scrCompileGlob.codePos;
            newBreakStatement->nextCodePos = TempMalloc(0);
            newBreakStatement->next = scrCompileGlob.currentBreakStatement;
            scrCompileGlob.currentBreakStatement = newBreakStatement;
        }
    }
    else
    {
        CompileError(sourcePos.stringValue, "illegal break statement");
    }
}

void __cdecl EmitContinueStatement(sval_u sourcePos, scr_block_s *block)
{
    ContinueStatementInfo *newContinueStatement; // [esp+0h] [ebp-4h]

    if (scrCompileGlob.bCanContinue && !block->abortLevel)
    {
        Scr_AddContinueBlock(block);
        EmitRemoveLocalVars(block, block);

        block->abortLevel = SCR_ABORT_CONTINUE;

        EmitOpcode(OP_jump, 0, 0);
        AddOpcodePos(sourcePos.stringValue, 1);

        EmitCodepos(0);

        if (scrCompilePub.developer_statement != 2)
        {
            newContinueStatement = (ContinueStatementInfo*)Hunk_AllocateTempMemoryHigh(12, "EmitContinueStatement");
            newContinueStatement->codePos = (char*)scrCompileGlob.codePos;
            newContinueStatement->nextCodePos = TempMalloc(0);
            newContinueStatement->next = scrCompileGlob.currentContinueStatement;
            scrCompileGlob.currentContinueStatement = newContinueStatement;
        }
    }
    else
    {
        CompileError(sourcePos.stringValue, "illegal continue statement");
    }
}

void __cdecl EmitBreakpointStatement(sval_u sourcePos)
{
    if (scrVarPub.developer_script)
    {
        EmitOpcode(OP_breakpoint, 0, 0);
        AddOpcodePos(sourcePos.stringValue, 1);
    }
}

void __cdecl EmitProfStatement(sval_u profileName, sval_u sourcePos, Opcode_t op)
{
    char *v3; // eax
    int profileIndex; // [esp+0h] [ebp-4h]

    if (scrVarPub.developer_script)
    {
        if (scrCompilePub.developer_statement == 3)
        {
            CompileError(0, "illegal statement in debugger");
        }
        else
        {
            v3 = SL_ConvertToString(profileName.stringValue);
            profileIndex = Profile_AddScriptName(v3);
            Scr_CompileRemoveRefToString(profileName.stringValue);
            if (profileIndex >= 0)
            {
                if (profileIndex >= 32)
                    MyAssertHandler(".\\script\\scr_compiler.cpp", 4413, 0, "%s", "profileIndex < 32");
                EmitOpcode(op, 0, 0);
                EmitByte(profileIndex);
                AddOpcodePos(sourcePos.stringValue, 0);
            }
            else
            {
                CompileError(sourcePos.stringValue, "max profile names exceeded");
            }
        }
    }
    else
    {
        Scr_CompileRemoveRefToString(profileName.stringValue);
    }
}

void __cdecl EmitProfBeginStatement(sval_u profileName, sval_u sourcePos)
{
    EmitProfStatement(profileName, sourcePos, OP_prof_begin);
}

void __cdecl EmitProfEndStatement(sval_u profileName, sval_u sourcePos)
{
    EmitProfStatement(profileName, sourcePos, OP_prof_end);
}

void __cdecl EmitStatement(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
    if (scrCompilePub.developer_statement == SCR_DEV_EVALUATE)
    {
        switch (val.node[0].type)
        {
        case ENUM_assignment:
        case ENUM_call_expression_statement:
        case ENUM_inc:
        case ENUM_dec:
        case ENUM_binary_equals:
        case ENUM_statement_list:
        case ENUM_notify:
            goto LABEL_3;
        default:
            CompileError(0, "illegal statement in debugger");
            break;
        }
    }
    else
    {
    LABEL_3:
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
}

void __cdecl EmitOpcode(Opcode_t op, int offset, int callType)
{
    bool v3; // [esp+4h] [ebp-10h]
    int valueIndex; // [esp+8h] [ebp-Ch]
    int value_count; // [esp+Ch] [ebp-8h]
    unsigned int index; // [esp+10h] [ebp-4h]
    unsigned int indexa; // [esp+10h] [ebp-4h]

    if (scrCompilePub.developer_statement == 3)
    {
        scrCompileGlob.codePos = (unsigned char*)TempMalloc(1u);
        *scrCompileGlob.codePos = op;
    }
    else
    {
        if (scrCompilePub.value_count)
        {
            value_count = scrCompilePub.value_count;
            scrCompilePub.value_count = 0;
            for (valueIndex = 0; valueIndex < value_count; ++valueIndex)
                EmitValue(&scrCompileGlob.value_start[valueIndex]);
        }
        v3 = !scrCompileGlob.cumulOffset || callType == 2 || callType == 3;
        scrCompilePub.allowedBreakpoint = v3;
        scrCompileGlob.cumulOffset += offset;
        if (scrCompileGlob.maxOffset < scrCompileGlob.cumulOffset)
            scrCompileGlob.maxOffset = scrCompileGlob.cumulOffset;
        if (callType && scrCompileGlob.maxCallOffset < scrCompileGlob.cumulOffset)
            scrCompileGlob.maxCallOffset = scrCompileGlob.cumulOffset;
        scrVarPub.checksum *= 31;
        scrVarPub.checksum += op;
        if (scrCompilePub.opcodePos)
        {
            scrCompileGlob.codePos = scrCompilePub.opcodePos;
            switch (op)
            {
            case ' ':
                if (*scrCompilePub.opcodePos == 30)
                {
                    RemoveOpcodePos();
                    *scrCompilePub.opcodePos = 31;
                    return;
                }
                index = *scrCompilePub.opcodePos - 24;
                if (index > 5)
                    goto LABEL_79;
                RemoveOpcodePos();
                *scrCompilePub.opcodePos = 31;
                EmitByte(index);
                return;
            case '#':
                if (*scrCompilePub.opcodePos == 55)
                {
                    RemoveOpcodePos();
                    *scrCompilePub.opcodePos = 34;
                    EmitPreAssignmentPos();
                    return;
                }
                if (*scrCompilePub.opcodePos != 54)
                    goto LABEL_79;
                RemoveOpcodePos();
                *scrCompilePub.opcodePos = 33;
                EmitPreAssignmentPos();
                return;
            case '*':
                if (*scrCompilePub.opcodePos == 38)
                {
                    *scrCompilePub.opcodePos = 41;
                    EmitPreAssignmentPos();
                    return;
                }
                if (*scrCompilePub.opcodePos == 13)
                {
                    *scrCompilePub.opcodePos = 39;
                    EmitPreAssignmentPos();
                    return;
                }
                if (*scrCompilePub.opcodePos != 14)
                    goto LABEL_79;
                *scrCompilePub.opcodePos = 40;
                EmitPreAssignmentPos();
                return;
            case '.':
                if (*scrCompilePub.opcodePos == 38)
                {
                    *scrCompilePub.opcodePos = 45;
                    EmitPreAssignmentPos();
                    return;
                }
                if (*scrCompilePub.opcodePos == 13)
                {
                    *scrCompilePub.opcodePos = 43;
                    EmitPreAssignmentPos();
                    return;
                }
                if (*scrCompilePub.opcodePos != 14)
                    goto LABEL_79;
                *scrCompilePub.opcodePos = 44;
                EmitPreAssignmentPos();
                return;
            case '1':
                if (*scrCompilePub.opcodePos != 22)
                    goto LABEL_79;
                *scrCompilePub.opcodePos = 48;
                return;
            case '9':
                switch (*scrCompilePub.opcodePos)
                {
                case '7':
                    RemoveOpcodePos();
                    *scrCompilePub.opcodePos = 61;
                    EmitPreAssignmentPos();
                    return;
                case '6':
                    RemoveOpcodePos();
                    *scrCompilePub.opcodePos = 60;
                    EmitPreAssignmentPos();
                    return;
                case '-':
                    RemoveOpcodePos();
                    *scrCompilePub.opcodePos = 59;
                    EmitPreAssignmentPos();
                    return;
                case '+':
                    RemoveOpcodePos();
                    *scrCompilePub.opcodePos = 56;
                    EmitPreAssignmentPos();
                    return;
                }
                if (*scrCompilePub.opcodePos != 44)
                    goto LABEL_79;
                RemoveOpcodePos();
                *scrCompilePub.opcodePos = 58;
                EmitPreAssignmentPos();
                return;
            case 'P':
                if (*scrCompilePub.opcodePos != 78)
                    goto LABEL_79;
                *scrCompilePub.opcodePos = 79;
                return;
            case 'R':
                if (*scrCompilePub.opcodePos != 15)
                    goto LABEL_79;
                RemoveOpcodePos();
                *scrCompilePub.opcodePos = 80;
                if (!scrCompileGlob.prevOpcodePos)
                    MyAssertHandler(".\\script\\scr_compiler.cpp", 497, 0, "%s", "scrCompileGlob.prevOpcodePos");
                if (*scrCompileGlob.prevOpcodePos == 78)
                {
                    if (scrCompilePub.opcodePos != (unsigned char*)TempMalloc(0) - 1)
                        MyAssertHandler(
                            ".\\script\\scr_compiler.cpp",
                            500,
                            0,
                            "%s",
                            "scrCompilePub.opcodePos == (byte *)TempMalloc( 0 ) - 1");
                    TempMemorySetPos((char*)scrCompilePub.opcodePos);
                    --scrCompilePub.opcodePos;
                    scrCompileGlob.prevOpcodePos = 0;
                    scrCompileGlob.codePos = scrCompilePub.opcodePos;
                    *scrCompilePub.opcodePos = 79;
                }
                return;
            case 'V':
                if (*scrCompilePub.opcodePos != 15)
                    goto LABEL_79;
                RemoveOpcodePos();
                *scrCompilePub.opcodePos = 84;
                return;
            case 'Y':
                if (*scrCompilePub.opcodePos == 30)
                {
                    *scrCompilePub.opcodePos = 90;
                    return;
                }
                indexa = *scrCompilePub.opcodePos - 24;
                if (indexa > 5)
                    goto LABEL_79;
                *scrCompilePub.opcodePos = 90;
                EmitByte(indexa);
                break;
            case '^':
                if (*scrCompilePub.opcodePos != 92)
                    goto LABEL_79;
                RemoveOpcodePos();
                *scrCompilePub.opcodePos = 95;
                return;
            default:
                goto LABEL_79;
            }
        }
        else
        {
        LABEL_79:
            scrCompileGlob.prevOpcodePos = scrCompilePub.opcodePos;
            scrCompilePub.opcodePos = (unsigned char*)TempMalloc(1u);
            scrCompileGlob.codePos = scrCompilePub.opcodePos;
            *scrCompilePub.opcodePos = op;
        }
    }
}

void __cdecl Scr_CompileStatement(sval_u parseData)
{
    EmitStatement(parseData, 0, 0, 0);
    EmitOpcode(OP_abort, 0, 0);
}

void __cdecl EmitInclude(sval_u val)
{
    iassert(val.node[0].type == ENUM_include);

    unsigned int filename = Scr_CreateCanonicalFilename(SL_ConvertToString(val.node[1].stringValue));
    Scr_CompileRemoveRefToString(val.node[1].stringValue);

    AddFilePrecache(filename, val.node[2].sourcePosValue, true);
}

void __cdecl EmitIncludeList(sval_u val)
{
    for (sval_u *node = val.node[0].node[1].node; node; node = node[1].node)
    {
        EmitInclude(node[0]);
    }
}

unsigned int __cdecl SpecifyThreadPosition(unsigned int threadId, unsigned int name, unsigned int sourcePos, Vartype_t type)
{
    char *v4; // eax
    char *v5; // eax
    char *buf; // [esp-4h] [ebp-1Ch]
    VariableValue pos; // [esp+8h] [ebp-10h] BYREF
    unsigned int posId; // [esp+14h] [ebp-4h]

    posId = GetVariable(threadId, 1u);
    pos = Scr_EvalVariable(posId);
    if (pos.type)
    {
        if (pos.u.intValue)
        {
            buf = scrParserPub.sourceBufferLookup[Scr_GetSourceBuffer(pos.u.codePosValue)].buf;
            v4 = SL_ConvertToString(name);
            CompileError(sourcePos, "function '%s' already defined in '%s'", v4, buf);
        }
        else
        {
            v5 = SL_ConvertToString(name);
            CompileError(sourcePos, "function '%s' already defined", v5);
        }
        return 0;
    }
    else
    {
        pos.type = type;
        pos.u.intValue = 0;
        SetNewVariableValue(posId, &pos);
        return posId;
    }
}

void __cdecl SpecifyThread(sval_u val)
{
    unsigned int posId;

    switch (val.node[0].type)
    {
    case ENUM_thread: // 68
        if (!scrCompileGlob.in_developer_thread || scrVarPub.developer_script)
        {
            posId = GetObject(GetVariable(scrCompileGlob.fileId, val.node[1].idValue));
            SpecifyThreadPosition(posId, val.node[1].sourcePosValue, val.node[4].sourcePosValue, scrCompileGlob.in_developer_thread ? VAR_DEVELOPER_CODEPOS : VAR_CODEPOS);
        }
        break;
    case ENUM_begin_developer_thread: // 69
        if (scrCompileGlob.in_developer_thread)
        {
            CompileError(val.node[1].sourcePosValue, "cannot recurse /#");
        }
        else
        {
            scrCompileGlob.in_developer_thread = true;
            scrCompileGlob.developer_thread_sourcePos = val.node[1].sourcePosValue;
        }
        break;
    case ENUM_end_developer_thread: // 70
        if (scrCompileGlob.in_developer_thread)
        {
            scrCompileGlob.in_developer_thread = false;
        }
        else
        {
            CompileError(val.node[1].sourcePosValue, "#/ has no matching /#");
        }
        break;
    }
}

void __cdecl Scr_RegisterLocalVar(unsigned int name, sval_u sourcePos, scr_block_s *block)
{
    int i; // [esp+0h] [ebp-4h]

    if (!block->abortLevel)
    {
        for (i = 0; i < block->localVarsCount; ++i)
        {
            if (block->localVars[i].name == name)
                return;
        }
        Scr_CheckLocalVarsCount(block->localVarsCount);
        block->localVars[block->localVarsCount].name = name;
        block->localVars[block->localVarsCount++].sourcePos = sourcePos.stringValue;
    }
}

void __cdecl Scr_CalcLocalVarsSafeSetVariableField(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    Scr_RegisterLocalVar(expr.stringValue, sourcePos, block);
}

void __cdecl Scr_CalcLocalVarsFormalParameterListInternal(sval_u *node, scr_block_s *block)
{
    while (1)
    {
        node = node[1].node;
        if (!node)
            break;
        Scr_CalcLocalVarsSafeSetVariableField(node[0].node[0], node[0].node[1], block);
    }
}

void __cdecl Scr_CalcLocalVarsArrayPrimitiveExpressionRef(sval_u expr, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsArrayVariableRef(sval_u expr, scr_block_s *block)
{
    Scr_CalcLocalVarsArrayPrimitiveExpressionRef(expr, block);
}

void __cdecl Scr_CalcLocalVarsVariableExpressionRef(sval_u expr, scr_block_s *block)
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

void __cdecl Scr_CalcLocalVarsFormalParameterList(sval_u exprlist, scr_block_s *block)
{
    Scr_CalcLocalVarsFormalParameterListInternal(exprlist.node, block);
}

void __cdecl Scr_CalcLocalVarsArrayPrimitiveExpressionRef(sval_u expr, scr_block_s *block)
{
    if (expr.node[0].type != ENUM_variable)
    {
        return;
    }

    Scr_CalcLocalVarsVariableExpressionRef(expr.node[1], block);
}

void __cdecl Scr_CalcLocalVarsAssignmentStatement(sval_u lhs, sval_u rhs, scr_block_s *block)
{
    Scr_CalcLocalVarsVariableExpressionRef(lhs, block);
}

void __cdecl Scr_CopyBlock(scr_block_s *from, scr_block_s **to)
{
    if (!*to)
        *to = (scr_block_s*)Hunk_AllocateTempMemoryHigh(536, "Scr_CopyBlock");

    //qmemcpy(*to, from, sizeof(scr_block_s));
    **to = *from;
    (*to)->localVarsPublicCount = 0;

}

void __cdecl Scr_MergeChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block)
{
    unsigned int v3; // ecx
    int j; // [esp+4h] [ebp-18h]
    unsigned int localVar; // [esp+8h] [ebp-14h]
    unsigned int localVar_4; // [esp+Ch] [ebp-10h]
    scr_block_s *childBlock; // [esp+10h] [ebp-Ch]
    int childIndex; // [esp+14h] [ebp-8h]
    int i; // [esp+18h] [ebp-4h]

    if (!childCount || block->abortLevel != SCR_ABORT_NONE)
    {
        return;
    }

    for (childIndex = 0; childIndex < childCount; ++childIndex)
    {
        childBlock = childBlocks[childIndex];
        if (childBlock->localVarsPublicCount)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 1006, 0, "%s", "!childBlock->localVarsPublicCount");
        childBlock->localVarsPublicCount = block->localVarsCount;
        for (i = 0; i < block->localVarsCount; ++i)
        {
            localVar = block->localVars[i].name;
            localVar_4 = block->localVars[i].sourcePos;
            j = Scr_FindLocalVar(childBlock, i, localVar);
            if (j < 0)
            {
                j = childBlock->localVarsCount;
                Scr_CheckLocalVarsCount(j);
                ++childBlock->localVarsCount;
            }
            while (j > i)
            {
                v3 = *&childBlock->localVarsInitBits[8 * j + 4];
                //childBlock->localVars[j] = *(scr_localVar_t *)&childBlock->localVarsInitBits[sizeof(scr_localVar_t) * j + sizeof(unsigned int)];
                childBlock->localVars[j].name = *&childBlock->localVarsInitBits[8 * j];
                childBlock->localVars[j].sourcePos = v3;
                j--;
            }
            childBlock->localVars[i].name = localVar;
            childBlock->localVars[i].sourcePos = localVar_4;
        }
    }
}

void __cdecl Scr_CalcLocalVarsStatement(sval_u val, scr_block_s *block);

void __cdecl Scr_CalcLocalVarsIfStatement(sval_u stmt, scr_block_s *block, sval_u *ifStatBlock)
{
    Scr_CopyBlock(block, &ifStatBlock->block);
    Scr_CalcLocalVarsStatement(stmt, ifStatBlock->block);
    Scr_MergeChildBlocks(&ifStatBlock->block, 1, block);
}

void __cdecl Scr_AppendChildBlocks(scr_block_s **childBlocks, int childCount, scr_block_s *block)
{
    int localVarsCount; // ecx
    unsigned int localVar; // [esp+0h] [ebp-14h]
    unsigned int localVar_4; // [esp+4h] [ebp-10h]
    int childIndex; // [esp+Ch] [ebp-8h]
    int childIndexa; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if (childCount && !block->abortLevel)
    {
        for (childIndex = 0; childIndex < childCount; ++childIndex)
            childBlocks[childIndex]->abortLevel = 0;
        for (i = 0; i < (*childBlocks)->localVarsCount; ++i)
        {
            localVar = (*childBlocks)->localVars[i].name;
            localVar_4 = (*childBlocks)->localVars[i].sourcePos;
            if (Scr_FindLocalVar(block, 0, localVar) < 0)
            {
                for (childIndexa = 1; childIndexa < childCount; ++childIndexa)
                {
                    if (Scr_FindLocalVar(childBlocks[childIndexa], 0, localVar) < 0)
                        goto LABEL_8;
                }
                localVarsCount = block->localVarsCount;
                block->localVars[localVarsCount].name = localVar;
                block->localVars[localVarsCount].sourcePos = localVar_4;
                ++block->localVarsCount;
            }
        LABEL_8:
            ;
        }
    }
}

void __cdecl Scr_CalcLocalVarsIfElseStatement(
    sval_u stmt1,
    sval_u stmt2,
    scr_block_s *block,
    sval_u *ifStatBlock,
    sval_u *elseStatBlock)
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

void __cdecl Scr_CalcLocalVarsWhileStatement(sval_u expr, sval_u stmt, scr_block_s *block, sval_u *whileStatBlock)
{
    int breakChildCount; // [esp+0h] [ebp-38h] BYREF
    int *oldBreakChildCount; // [esp+4h] [ebp-34h]
    scr_block_s **breakChildBlocks; // [esp+8h] [ebp-30h]
    bool constConditional; // [esp+Fh] [ebp-29h]
    int continueChildCount; // [esp+10h] [ebp-28h] BYREF
    int *oldContinueChildCount; // [esp+14h] [ebp-24h]
    VariableCompileValue constValue; // [esp+18h] [ebp-20h] BYREF
    int i; // [esp+24h] [ebp-14h]
    scr_block_s **continueChildBlocks; // [esp+28h] [ebp-10h]
    scr_block_s **oldBreakChildBlocks; // [esp+2Ch] [ebp-Ch]
    int abortLevel; // [esp+30h] [ebp-8h]
    scr_block_s **oldContinueChildBlocks; // [esp+34h] [ebp-4h]

    constConditional = 0;
    if (EvalExpression(expr, &constValue))
    {
        if (constValue.value.type == 6 || constValue.value.type == 5)
        {
            Scr_CastBool(&constValue.value);
            if (constValue.value.u.intValue)
                constConditional = 1;
        }
        RemoveRefToValue(constValue.value.type, constValue.value.u);
    }
    oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
    oldBreakChildCount = scrCompileGlob.breakChildCount;
    oldContinueChildBlocks = scrCompileGlob.continueChildBlocks;
    oldContinueChildCount = scrCompileGlob.continueChildCount;
    breakChildCount = 0;
    continueChildCount = 0;
    continueChildBlocks = (scr_block_s**)Hunk_AllocateTempMemoryHigh(4096, "Scr_CalcLocalVarsWhileStatement");
    scrCompileGlob.continueChildBlocks = continueChildBlocks;
    scrCompileGlob.continueChildCount = &continueChildCount;
    abortLevel = block->abortLevel;
    if (constConditional)
    {
        breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(4096, "Scr_CalcLocalVarsWhileStatement");
        scrCompileGlob.breakChildCount = &breakChildCount;
    }
    else
    {
        breakChildBlocks = 0;
    }
    scrCompileGlob.breakChildBlocks = breakChildBlocks;
    Scr_CopyBlock(block, (scr_block_s **)whileStatBlock);
    Scr_CalcLocalVarsStatement(stmt, whileStatBlock->block);
    Scr_AddContinueBlock(whileStatBlock->block);
    for (i = 0; i < continueChildCount; ++i)
        Scr_AppendChildBlocks(&continueChildBlocks[i], 1, block);
    if (constConditional)
        Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
    Scr_MergeChildBlocks((scr_block_s **)whileStatBlock, 1, block);
    scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
    scrCompileGlob.breakChildCount = oldBreakChildCount;
    scrCompileGlob.continueChildBlocks = oldContinueChildBlocks;
    scrCompileGlob.continueChildCount = oldContinueChildCount;
}

void __cdecl Scr_CalcLocalVarsForStatement(
    sval_u stmt1,
    sval_u expr,
    sval_u stmt2,
    sval_u stmt,
    scr_block_s *block,
    sval_u *forStatBlock,
    sval_u *forStatPostBlock)
{
    int breakChildCount; // [esp+0h] [ebp-38h] BYREF
    int *oldBreakChildCount; // [esp+4h] [ebp-34h]
    scr_block_s **breakChildBlocks; // [esp+8h] [ebp-30h]
    bool constConditional; // [esp+Fh] [ebp-29h]
    int continueChildCount; // [esp+10h] [ebp-28h] BYREF
    int *oldContinueChildCount; // [esp+14h] [ebp-24h]
    VariableCompileValue constValue; // [esp+18h] [ebp-20h] BYREF
    int i; // [esp+24h] [ebp-14h]
    scr_block_s **continueChildBlocks; // [esp+28h] [ebp-10h]
    scr_block_s **oldBreakChildBlocks; // [esp+2Ch] [ebp-Ch]
    int abortLevel; // [esp+30h] [ebp-8h]
    scr_block_s **oldContinueChildBlocks; // [esp+34h] [ebp-4h]

    Scr_CalcLocalVarsStatement(stmt1, block);
    if (expr.node[0].type == ENUM_expression)
    {
        constConditional = 0;
        if (EvalExpression(expr.node[1], &constValue))
        {
            if (constValue.value.type == VAR_INTEGER || constValue.value.type == VAR_FLOAT)
            {
                Scr_CastBool(&constValue.value);
                if (constValue.value.u.intValue)
                    constConditional = true;
            }
            RemoveRefToValue(constValue.value.type, constValue.value.u);
        }
    }
    else
    {
        constConditional = 1;
    }
    oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
    oldBreakChildCount = scrCompileGlob.breakChildCount;
    oldContinueChildBlocks = scrCompileGlob.continueChildBlocks;
    oldContinueChildCount = scrCompileGlob.continueChildCount;
    breakChildCount = 0;
    continueChildCount = 0;
    continueChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(4096, "Scr_CalcLocalVarsForStatement");
    scrCompileGlob.continueChildBlocks = continueChildBlocks;
    scrCompileGlob.continueChildCount = &continueChildCount;
    abortLevel = block->abortLevel;
    if (constConditional)
    {
        breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(4096, "Scr_CalcLocalVarsForStatement");
        scrCompileGlob.breakChildCount = &breakChildCount;
    }
    else
    {
        breakChildBlocks = 0;
    }
    scrCompileGlob.breakChildBlocks = breakChildBlocks;
    Scr_CopyBlock(block, (scr_block_s **)forStatBlock);
    Scr_CopyBlock(block, (scr_block_s **)forStatPostBlock);
    Scr_CalcLocalVarsStatement(stmt, forStatBlock->block);
    Scr_AddContinueBlock(forStatBlock->block);
    for (i = 0; i < continueChildCount; ++i)
        Scr_AppendChildBlocks(&continueChildBlocks[i], 1, block);
    Scr_CalcLocalVarsStatement(stmt2, forStatPostBlock->block);
    Scr_AppendChildBlocks((scr_block_s **)forStatPostBlock, 1, block);
    Scr_MergeChildBlocks((scr_block_s **)forStatPostBlock, 1, block);
    if (constConditional)
        Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
    Scr_MergeChildBlocks((scr_block_s **)forStatBlock, 1, block);
    scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
    scrCompileGlob.breakChildCount = oldBreakChildCount;
    scrCompileGlob.continueChildBlocks = oldContinueChildBlocks;
    scrCompileGlob.continueChildCount = oldContinueChildCount;
}

void __cdecl Scr_CalcLocalVarsIncStatement(sval_u expr, scr_block_s *block)
{
    Scr_CalcLocalVarsVariableExpressionRef(expr, block);
}

void __cdecl Scr_CalcLocalVarsStatementList(sval_u val, scr_block_s *block);
void __cdecl Scr_CalcLocalVarsDeveloperStatementList(sval_u val, scr_block_s *block, sval_u *devStatBlock)
{
    Scr_CopyBlock(block, &devStatBlock->block);
    Scr_CalcLocalVarsStatementList(val, devStatBlock->block);
    Scr_MergeChildBlocks(&devStatBlock->block, 1, block);
}

void __cdecl Scr_CalcLocalVarsWaittillStatement(sval_u exprlist, scr_block_s *block)
{
    sval_u *node;

    //node = *(sval_u**)(*(DWORD*)exprlist.type + 4);
    node = exprlist.node[0].node[1].node;
    iassert(node);
    Scr_CalcLocalVarsFormalParameterListInternal(node, block);
}

void __cdecl Scr_CalcLocalVarsSwitchStatement(sval_u stmtlist, scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-28h]
    int breakChildCount; // [esp+4h] [ebp-24h] BYREF
    scr_block_s **breakChildBlocks; // [esp+8h] [ebp-20h]
    int *oldBreakChildCount; // [esp+Ch] [ebp-1Ch]
    bool hasDefault; // [esp+13h] [ebp-15h]
    scr_block_s *currentBlock; // [esp+14h] [ebp-14h] BYREF
    scr_block_s **childBlocks; // [esp+18h] [ebp-10h]
    scr_block_s **oldBreakChildBlocks; // [esp+1Ch] [ebp-Ch]
    int childCount; // [esp+20h] [ebp-8h]
    int abortLevel; // [esp+24h] [ebp-4h]

    abortLevel = 3;
    oldBreakChildBlocks = scrCompileGlob.breakChildBlocks;
    oldBreakChildCount = scrCompileGlob.breakChildCount;
    breakChildCount = 0;
    breakChildBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(4096, "Scr_CalcLocalVarsSwitchStatement");
    scrCompileGlob.breakChildBlocks = breakChildBlocks;
    scrCompileGlob.breakChildCount = &breakChildCount;
    childCount = 0;
    currentBlock = 0;
    hasDefault = 0;
    childBlocks = (scr_block_s **)Hunk_AllocateTempMemoryHigh(4096, "Scr_CalcLocalVarsSwitchStatement");
    for (node = stmtlist.node[0].node[1].node; node; node = node[1].node)
    {
        if (node[0].node[0].type == ENUM_case || node[0].node[0].type == ENUM_default)
        {
            currentBlock = 0;
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
            Scr_CalcLocalVarsStatement(node->type, currentBlock);
            if (currentBlock->abortLevel)
            {
                if (currentBlock->abortLevel == 2)
                {
                    currentBlock->abortLevel = 0;
                    abortLevel = 0;
                    Scr_CheckMaxSwitchCases(childCount);
                    childBlocks[childCount++] = currentBlock;
                }
                else if (currentBlock->abortLevel <= abortLevel)
                {
                    abortLevel = currentBlock->abortLevel;
                }
                currentBlock = 0;
            }
        }
    }
    if (hasDefault)
    {
        if (currentBlock)
        {
            Scr_AddBreakBlock(currentBlock);
            Scr_CheckMaxSwitchCases(childCount);
            childBlocks[childCount++] = currentBlock;
        }
        if (!block->abortLevel)
            block->abortLevel = abortLevel;
        Scr_AppendChildBlocks(breakChildBlocks, breakChildCount, block);
        Scr_MergeChildBlocks(childBlocks, childCount, block);
    }
    scrCompileGlob.breakChildBlocks = oldBreakChildBlocks;
    scrCompileGlob.breakChildCount = oldBreakChildCount;
}

void __cdecl Scr_CalcLocalVarsStatement(sval_u val, scr_block_s *block)
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
    default:
        return;
    }
}

void __cdecl Scr_CalcLocalVarsStatementList(sval_u val, scr_block_s *block)
{
    for (sval_u *node = val.node[0].node[1].node; node; node = node[1].node)
    {
        Scr_CalcLocalVarsStatement(node[0], block);
    }
}

void __cdecl Scr_CalcLocalVarsThread(sval_u exprlist, sval_u stmtlist, sval_u *stmttblock)
{
    scrCompileGlob.forceNotCreate = false;
    stmttblock->block = (scr_block_s*)Hunk_AllocateTempMemoryHigh(536, "Scr_CalcLocalVarsThread");

    stmttblock->block->abortLevel = SCR_ABORT_NONE;
    stmttblock->block->localVarsCreateCount = 0;
    stmttblock->block->localVarsCount = 0;
    stmttblock->block->localVarsPublicCount = 0;

    memset(stmttblock->block->localVarsInitBits, 0, sizeof(stmttblock->block->localVarsInitBits));

    Scr_CalcLocalVarsFormalParameterList(exprlist, stmttblock->block);
    Scr_CalcLocalVarsStatementList(stmtlist, stmttblock->block);
}

void __cdecl SetThreadPosition(unsigned int threadId)
{
    char *v1; // esi
    unsigned int Variable; // eax

    v1 = TempMalloc(0);
    Variable = FindVariable(threadId, 1u);
    GetVariableValueAddress(Variable)->u.intValue = (int)v1;
}

void __cdecl InitThread(int type)
{
    scrCompileGlob.currentCaseStatement = 0;
    scrCompileGlob.bCanBreak = 0;
    scrCompileGlob.currentBreakStatement = 0;
    scrCompileGlob.bCanContinue = 0;
    scrCompileGlob.currentContinueStatement = 0;
    scrCompileGlob.breakChildBlocks = 0;
    scrCompileGlob.continueChildBlocks = 0;
    if (scrCompileGlob.firstThread[type])
    {
        scrCompileGlob.firstThread[type] = 0;
        EmitEnd();
        AddOpcodePos(0, 0);
        AddOpcodePos(0xFFFFFFFE, 0);
    }
}

void __cdecl EmitSafeSetVariableField(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    int index; // [esp+0h] [ebp-4h]

    index = Scr_FindLocalVarIndex(expr.stringValue, sourcePos, 1, block);
    if (index)
    {
        EmitOpcode(OP_SafeSetVariableFieldCached, 0, 0);
    }
    else
    {
        EmitOpcode(OP_SafeSetVariableFieldCached0, 0, 0);
    }
    EmitPreAssignmentPos();
    if (index)
        EmitByte(index);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitAssignmentPos();
}

void __cdecl EmitFormalParameterListInternal(sval_u *node, scr_block_s *block)
{
    while (1)
    {
        node = node[1].node;
        if (!node)
            break;
        EmitSafeSetVariableField(node[0].node[0], node[0].node[1], block);
    }
}

void __cdecl EmitFormalParameterList(sval_u exprlist, sval_u sourcePos, scr_block_s *block)
{
    EmitFormalParameterListInternal(exprlist.node[0].node, block);
    EmitOpcode(OP_checkclearparams, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitThreadInternal(
    unsigned int threadId,
    sval_u val,
    sval_u sourcePos,
    sval_u endSourcePos,
    scr_block_s *block)
{
    scrCompileGlob.threadId = threadId;
    AddThreadStartOpcodePos(sourcePos.stringValue);

    scrCompileGlob.cumulOffset = 0;
    scrCompileGlob.maxOffset = 0;
    scrCompileGlob.maxCallOffset = 0;

    CompileTransferRefToString(val.node[1].stringValue, 2u);
    EmitFormalParameterList(val.node[2], sourcePos, block);
    EmitStatementList(val.node[3], 1, endSourcePos.stringValue, block);
    EmitEnd();
    AddOpcodePos(endSourcePos.stringValue, 1);
    AddOpcodePos(0xFFFFFFFE, 0);
    
    iassert(!scrCompileGlob.cumulOffset);
    
    if (scrCompileGlob.maxOffset + 32 * scrCompileGlob.maxCallOffset >= 2048)
        CompileError(sourcePos.stringValue, "function exceeds operand stack size");
}

void __cdecl EmitDeveloperThread(sval_u val, sval_u *stmttblock)
{
    unsigned int Variable; // eax
    unsigned int savedChecksum; // [esp+4h] [ebp-8h]
    char *begin_pos; // [esp+8h] [ebp-4h]

    unsigned int posId;
    unsigned int threadId;

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

void __cdecl EmitNormalThread(sval_u val, sval_u *stmttblock)
{
    unsigned int Variable; // eax
    VariableValueInternal_u threadId; // [esp+0h] [ebp-4h]

    InitThread(0);
    Variable = FindVariable(scrCompileGlob.fileId, val.node[1].sourcePosValue);
    threadId = FindObject(Variable);
    SetThreadPosition(threadId);
    EmitThreadInternal(threadId, val, val.node[4], val.node[5], stmttblock->block);
}

void __cdecl EmitThread(sval_u val)
{
    char *v1; // eax
    unsigned int v2; // [esp-4h] [ebp-8h]

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

void __cdecl EmitThreadList(sval_u val)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    scrCompileGlob.in_developer_thread = false;

    for (node = val.node[0].node[1].node; node; node = node[1].node)
        SpecifyThread(node[0]);

    if (scrCompileGlob.in_developer_thread)
        CompileError(scrCompileGlob.developer_thread_sourcePos, "/# has no matching #/");

    scrCompileGlob.firstThread[0] = true;
    scrCompileGlob.firstThread[1] = true;

    iassert(!scrCompileGlob.in_developer_thread);

    for (node = val.node[0].node[1].node; node; node = node[1].node)
        EmitThread(node[0]);

    iassert(!scrCompileGlob.in_developer_thread);
}

void __cdecl LinkThread(unsigned int threadId, VariableValue *pos, bool allowFarCall)
{
    VariableValue v3; // [esp+0h] [ebp-28h]
    unsigned int valueId; // [esp+Ch] [ebp-1Ch]
    unsigned int countId; // [esp+10h] [ebp-18h]
    unsigned int type; // [esp+14h] [ebp-14h]
    int i; // [esp+18h] [ebp-10h]
    VariableValueInternal_u *value; // [esp+24h] [ebp-4h]

    countId = FindVariable(threadId, 0);
    if (countId)
    {
        v3 = Scr_EvalVariable(countId);
        if (v3.type != 6)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 2307, 0, "%s", "count.type == VAR_INTEGER");
        for (i = 0; i < v3.u.intValue; ++i)
        {
            valueId = FindVariable(threadId, i + 2);
            if (!valueId)
                MyAssertHandler(".\\script\\scr_compiler.cpp", 2312, 0, "%s", "valueId");
            value = GetVariableValueAddress(valueId);
            type = GetValueType(valueId);
            if (type != 7 && type != 12)
                MyAssertHandler(
                    ".\\script\\scr_compiler.cpp",
                    2315,
                    0,
                    "%s",
                    "type == VAR_CODEPOS || type == VAR_DEVELOPER_CODEPOS");
            if (pos->type == 12)
            {
                if (!scrVarPub.developer_script)
                    MyAssertHandler(".\\script\\scr_compiler.cpp", 2319, 0, "%s", "scrVarPub.developer_script");
                if (type == 7)
                {
                    CompileError2((char*)value->u.intValue, "normal script cannot reference a function in a /# ... #/ comment");
                    return;
                }
            }
            if (!pos->type || !allowFarCall && *(DWORD*)value->u.intValue == 1)
            {
                CompileError2((char*)value->u.intValue, "unknown function");
                return;
            }
            *(DWORD*)value->u.intValue = pos->u.intValue;
            RemoveVariable(threadId, i + 2);
        }
        RemoveVariable(threadId, 0);
    }
}

void __cdecl LinkFile(unsigned int fileId)
{
    VariableValue pos; // [esp+8h] [ebp-1Ch] BYREF
    unsigned int posId; // [esp+10h] [ebp-14h]
    unsigned int threadId; // [esp+14h] [ebp-10h]
    VariableValue emptyValue; // [esp+18h] [ebp-Ch] BYREF
    unsigned int threadPtr; // [esp+20h] [ebp-4h]

    for (threadPtr = FindFirstSibling(fileId); threadPtr; threadPtr = FindNextSibling(threadPtr))
    {
        threadId = FindObject(threadPtr).u.stringValue;
        if (!threadId)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 2364, 0, "%s", "threadId");
        posId = FindVariable(threadId, 1u);
        if (posId)
        {
            pos = Scr_EvalVariable(posId);
            if (pos.type == 13)
            {
                SetVariableValue(threadPtr, &emptyValue);
            }
            else
            {
                if (pos.type != 7 && pos.type != 12)
                    MyAssertHandler(
                        ".\\script\\scr_compiler.cpp",
                        2376,
                        0,
                        "%s",
                        "pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS");
                if (!pos.u.codePosValue)
                    MyAssertHandler(".\\script\\scr_compiler.cpp", 2377, 0, "%s", "pos.u.codePosValue");
                LinkThread(threadId, &pos, 1);
            }
        }
        else
        {
            LinkThread(threadId, &emptyValue, 1);
        }
    }
}

void __cdecl ScriptCompile(
    sval_u val,
    unsigned int fileId,
    unsigned int scriptId,
    PrecacheEntry *entries,
    int entriesCount)
{
    char *v5; // eax
    char *v6; // eax
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
        v5 = SL_ConvertToString(filename);
        includeFileId = Scr_LoadScriptInternal(v5, entries, entriesCounta);
        ProfLoad_Begin("ScriptCompile");
        if (!includeFileId)
        {
            v6 = SL_ConvertToString(filename);
            CompileError(precachescript->sourcePos, "Could not find script '%s'", v6);
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
                if (GetValueType(threadPtr) == 1)
                {
                    threadId = FindObject(threadPtr).u.stringValue;
                    if (!threadId)
                        MyAssertHandler(".\\script\\scr_compiler.cpp", 5026, 0, "%s", "threadId");
                    posId = FindVariable(threadId, 1u);
                    if (posId)
                    {
                        pos = Scr_EvalVariable(posId);
                        if (pos.type != 13)
                        {
                            iassert(pos.type == VAR_CODEPOS || pos.type == VAR_DEVELOPER_CODEPOS);
                            name = GetVariableName(threadPtr);
                            Variable_DONE = GetVariable(fileId, name);
                            toThreadId = GetObject(Variable_DONE);
                            toPosId = SpecifyThreadPosition(toThreadId, name, precachescript->sourcePos, VAR_DEAD_ENTITY);
                            VariableValueAddress_DONE = GetVariableValueAddress(posId);
                            *GetVariableValueAddress(toPosId) = *VariableValueAddress_DONE;
                            LinkThread(toThreadId, &pos, 0);
                        }
                    }
                }
            }
        }
    }
    LinkFile(fileId);
    value.type = VAR_INTEGER;
    SetVariableValue(scriptId, &value);
}