#include "scr_compiler.h"
#include "scr_main.h"
#include "scr_debugger.h"
#include "scr_parser.h"
#include "scr_evaluate.h"
#include "scr_vm.h"
#include <game_mp/g_public_mp.h>
#include "scr_animtree.h"
#include <universal/profile.h>

#undef GetObject 

scrCompilePub_t scrCompilePub;
scrCompileGlob_t scrCompileGlob;

void __cdecl EmitOpcode(unsigned int op, int offset, int callType);
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
    return *expr.codePosValue == 31;
}

bool __cdecl IsUndefinedExpression(sval_u expr)
{
    return *expr.codePosValue == 6 && IsUndefinedPrimitiveExpression(*(expr.codePosValue + 4));
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
                EmitOpcode(0x16u, 0, 0);
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
    int index; // [esp+0h] [ebp-8h]
    unsigned int opcode; // [esp+4h] [ebp-4h]

    if (scrCompilePub.developer_statement == 3)
    {
        EmitOpcode(0x83u, 1, 0);
        EmitCanonicalString(expr.stringValue);
    }
    else
    {
        index = Scr_FindLocalVarIndex(expr.stringValue, sourcePos, 0, block);
        if (index > 5)
        {
            opcode = 30;
            EmitOpcode(0x1Eu, 1, 0);
        }
        else
        {
            opcode = index + 24;
            EmitOpcode(index + 24, 1, 0);
        }
        if (opcode == 30)
            EmitByte(index);
        AddOpcodePos(sourcePos.stringValue, 1);
    }
}

void __cdecl EmitGetUndefined(sval_u sourcePos)
{
    EmitOpcode(2u, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitShort(__int16 value)
{
    scrCompileGlob.codePos = (unsigned char*)TempMallocAlignStrict(2u);
    *scrCompileGlob.codePos = value;
}

void __cdecl CompileTransferRefToString(unsigned int stringValue, unsigned int user)
{
    if (!stringValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 233, 0, "%s", "stringValue");
    if (scrCompilePub.developer_statement == 2)
    {
        Scr_CompileRemoveRefToString(stringValue);
    }
    else if (scrCompilePub.developer_statement != 3)
    {
        if (scrCompileGlob.bConstRefCount)
            SL_AddRefToString(stringValue);
        SL_TransferRefToUser(stringValue, user);
    }
}

void __cdecl EmitGetString(unsigned int value, sval_u sourcePos)
{
    EmitOpcode(0xAu, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    EmitShort(value);
    CompileTransferRefToString(value, 1u);
}

void __cdecl EmitGetIString(unsigned int value, sval_u sourcePos)
{
    EmitOpcode(0xBu, 1, 0);
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

    EmitOpcode(0xCu, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    for (i = 0; i < 3; ++i)
        EmitFloat(value[i]);
    RemoveRefToVector(value);
}

void __cdecl EmitGetFloat(float value, sval_u sourcePos)
{
    EmitOpcode(9u, 1, 0);
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
            EmitOpcode(5u, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            EmitByte(-value);
            return;
        }
        if (value > -65536)
        {
            EmitOpcode(7u, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            EmitShort(-value);
            return;
        }
    }
    else
    {
        if (!value)
        {
            EmitOpcode(3u, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            return;
        }
        if (value < 256)
        {
            EmitOpcode(4u, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            EmitByte(value);
            return;
        }
        if (value < 0x10000)
        {
            EmitOpcode(6u, 1, 0);
            AddOpcodePos(sourcePos.stringValue, 1);
            EmitShort(value);
            return;
        }
    }
    EmitOpcode(8u, 1, 0);
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
    EmitOpcode(0x20u, -1, 0);
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
    EmitOpcode(0x59u, -1, 0);
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

int __cdecl Scr_GetCacheType(int type)
{
    if (!type)
        return 7;
    if (type != 1)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2018, 0, "%s", "type == BUILTIN_DEVELOPER_ONLY");
    return 12;

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
    for (node = *(sval_u**)exprlist.type; node; node = node[1].node)
    {
        EmitExpression(node->node->type, block);
        ++expr_count;
    }
    return expr_count;
}

void __cdecl EmitCallBuiltinOpcode(int param_count, sval_u sourcePos)
{
    unsigned int opcode; // [esp+0h] [ebp-4h]

    if (param_count > 5)
    {
        opcode = 68;
        EmitOpcode(0x44u, 1 - param_count, 1);
    }
    else
    {
        opcode = param_count + '>';
        EmitOpcode(param_count + '>', 1 - param_count, 1);
    }
    AddOpcodePos(sourcePos.stringValue, 9);
    if (opcode == 'D')
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
        for (node = *(sval_u**)exprlist.type; node; node = node[1].node)
            AddOpcodePos(*(unsigned int*)(node->type + 4), 0);
    }
}

void EmitDecTop()
{
    EmitOpcode(0x58u, -1, 0);
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
        EmitOpcode(0x4Eu, 1, 0);
}

VariableValueInternal_u __cdecl AddFilePrecache(unsigned int filename, unsigned int sourcePos, bool include)
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
        Scr_CompileRemoveRefToString(*(func.codePosValue + 4));
        if (*func.codePosValue == 21)
        {
            Scr_CompileRemoveRefToString(*(func.codePosValue + 8));
            --scrCompilePub.far_function_count;
        }
        return;
    }
    if (*func.codePosValue == 20)
    {
        scope = 0;
        threadPtr = GetVariable(scrCompileGlob.fileId, *(func.codePosValue + 4));
        CompileTransferRefToString(*(func.codePosValue + 4), 2u);
        threadId = GetObject(threadPtr).u.stringValue;
        if (!threadId)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 1708, 0, "%s", "threadId");
        goto LABEL_39;
    }
    if (func.node[0].type != 21)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1712, 0, "%s", "func.node[0].type == ENUM_far_function");
    scope = 1;
    v2 = SL_ConvertToString(*(func.codePosValue + 4));
    filename.prev = Scr_CreateCanonicalFilename(v2).prev;
    Scr_CompileRemoveRefToString(*(func.codePosValue + 4));
    Variable = FindVariable(scrCompilePub.loadedscripts, filename.prev);
    value = Scr_EvalVariable(Variable);
    fileId = AddFilePrecache(filename.prev, sourcePos.stringValue, 0).u.stringValue;
    if (value.type)
    {
        threadPtr = FindVariable(fileId, *(func.codePosValue + 8));
        if (!threadPtr || GetValueType(threadPtr) != 1)
        {
        LABEL_26:
            CompileError(sourcePos.stringValue, "unknown function");
            return;
        }
    }
    else
    {
        threadPtr = GetVariable(fileId, *(func.codePosValue + 8));
    }
    CompileTransferRefToString(*(func.codePosValue + 8), 2u);
    threadId = GetObject(threadPtr).u.stringValue;
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
            count.type = 6;
            count.u.intValue = 0;
        }
        valueId = GetNewVariable(threadId, count.u.intValue + 2);
        value.u.intValue = (int)scrCompileGlob.codePos;
        if (scrCompilePub.developer_statement)
        {
            if (!scrVarPub.developer_script)
                MyAssertHandler(".\\script\\scr_compiler.cpp", 1800, 0, "%s", "scrVarPub.developer_script");
            value.type = 12;
        }
        else
        {
            value.type = 7;
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
        EmitOpcode(0x52u, -param_count - 1, 3);
    else
        EmitOpcode(0x50u, -param_count, 3);
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
        EmitOpcode(0x53u, -param_count - 2, 3);
    else
        EmitOpcode(0x51u, -param_count - 1, 3);
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
    if (*func_name.codePosValue == 18)
    {
        EmitPostScriptFunction(*(func_name.codePosValue + 4), param_count, bMethod, nameSourcePos);
    }
    else if (*func_name.codePosValue == 22)
    {
        EmitPostScriptFunctionPointer(
            *(func_name.codePosValue + 4),
            param_count,
            bMethod,
            nameSourcePos,
            *(func_name.codePosValue + 8),
            block);
    }
}

void __cdecl EmitPostScriptThread(sval_u func, int param_count, bool bMethod, sval_u sourcePos)
{
    if (bMethod)
        EmitOpcode(0x56u, -param_count, 2);
    else
        EmitOpcode(0x54u, 1 - param_count, 2);
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
        EmitOpcode(0x57u, -param_count - 1, 2);
    else
        EmitOpcode(0x55u, -param_count, 2);
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
    if (*func_name.codePosValue == 18)
    {
        EmitPostScriptThread(*(func_name.codePosValue + 4), param_count, bMethod, nameSourcePos);
    }
    else if (*func_name.codePosValue == 22)
    {
        EmitPostScriptThreadPointer(*(func_name.codePosValue + 4), param_count, bMethod, *(func_name.codePosValue + 8), block);
    }
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitPostFunctionCall(sval_u func_name, int param_count, bool bMethod, scr_block_s *block)
{
    if (*func_name.codePosValue == 26)
    {
        EmitPostScriptFunctionCall(*(func_name.codePosValue + 4), param_count, bMethod, *(func_name.codePosValue + 8), block);
    }
    else if (*func_name.codePosValue == 30)
    {
        EmitPostScriptThreadCall(
            *(func_name.codePosValue + 4),
            param_count,
            bMethod,
            *(func_name.codePosValue + 8),
            *(func_name.codePosValue + 12),
            block);
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
    sourcePos.type = *(func_name.codePosValue + 8);
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
    unsigned int opcode; // [esp+0h] [ebp-4h]

    if (param_count > 5)
    {
        opcode = 75;
        EmitOpcode(0x4Bu, -param_count, 1);
    }
    else
    {
        opcode = param_count + 69;
        EmitOpcode(param_count + 69, -param_count, 1);
    }
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 9);
    if (opcode == 75)
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
    sourcePos.type = *(func_name.codePosValue + 8);
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
            CompileError(*(func_name.codePosValue + 8), "unknown builtin method");
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
    if (*expr.codePosValue == 23)
    {
        EmitCall(*(expr.codePosValue + 4), *(expr.codePosValue + 8), 0, block);
        EmitCastFieldObject(*(expr.codePosValue + 12));
    }
    else if (*expr.codePosValue == 24)
    {
        EmitMethod(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), *(expr.codePosValue + 16), 0, block);
        EmitCastFieldObject(*(expr.codePosValue + 20));
    }
}

void __cdecl EmitSelfObject(sval_u sourcePos)
{
    EmitOpcode(0x26u, 0, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitLevelObject(sval_u sourcePos)
{
    EmitOpcode(0xDu, 0, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitAnimObject(sval_u sourcePos)
{
    EmitOpcode(0xEu, 0, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 1);
}

sval_u *__cdecl GetSingleParameter(sval_u exprlist)
{
    if (!*exprlist.codePosValue)
        return 0;
    if (*(sval_u **)(*exprlist.codePosValue + 4))
        return 0;
    return *(sval_u**)exprlist.codePosValue;
}

void __cdecl EmitExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if (*expr.codePosValue == 6)
        EmitPrimitiveExpressionFieldObject(*(expr.codePosValue + 4), *(expr.codePosValue + 8), block);
    else
        CompileError(sourcePos.stringValue, "not an object");
}

void __cdecl EmitExpressionListFieldObject(sval_u exprlist, sval_u sourcePos, scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    node = GetSingleParameter(exprlist);
    if (node)
        EmitExpressionFieldObject(node->node->type, *(node->codePosValue + 4), block);
    else
        CompileError(sourcePos.stringValue, "not an object");
}

void __cdecl EmitPrimitiveExpressionFieldObject(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    switch (*expr.codePosValue)
    {
    case 0x11:
        EmitVariableExpression(*(expr.codePosValue + 4), block);
        EmitCastFieldObject(*(expr.codePosValue + 8));
        break;
    case 0x13:
        EmitCallExpressionFieldObject(*(expr.codePosValue + 4), block);
        break;
    case 0x20:
        EmitSelfObject(*(expr.codePosValue + 4));
        break;
    case 0x22:
        EmitLevelObject(*(expr.codePosValue + 4));
        break;
    case 0x24:
        EmitAnimObject(*(expr.codePosValue + 4));
        break;
    case 0x2E:
        EmitExpressionListFieldObject(*(expr.codePosValue + 4), sourcePos, block);
        break;
    default:
        CompileError(sourcePos.stringValue, "not an object");
        break;
    }
}

void __cdecl EmitFieldVariable(sval_u expr, sval_u field, sval_u sourcePos, scr_block_s *block)
{
    EmitPrimitiveExpressionFieldObject(expr, sourcePos, block);
    EmitOpcode(0x2Au, 1, 0);
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
    classnum = (const char*)Scr_GetClassnumForCharId(*s);
    if ((int)classnum < 0)
        goto LABEL_17;
    entnum = (const char*)atoi(s + 1); // KISAKTODO: seems wrong
    if (!entnum && s[1] != 48)
        goto LABEL_17;
    EmitOpcode(0x81u, 1, 0);
    EmitCodepos(classnum);
    EmitCodepos(entnum);
}

void __cdecl EmitVariableExpression(sval_u expr, scr_block_s *block)
{
    switch (*expr.codePosValue)
    {
    case 4:
        EmitLocalVariable(*(expr.codePosValue + 4), *(expr.codePosValue + 8), block);
        break;
    case 0xD:
        EmitArrayVariable(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), *(expr.codePosValue + 16), block);
        break;
    case 0xF:
        EmitFieldVariable(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), block);
        break;
    case 0x35:
        if (scrCompilePub.script_loading)
            CompileError(*(expr.codePosValue + 8), "self field can only be used in the script debugger");
        else
            CompileError(*(expr.codePosValue + 8), "self field in assignment expression not currently supported");
        break;
    case 0x50:
        EmitObject(*(expr.codePosValue + 4), *(expr.codePosValue + 8));
        break;
    default:
        return;
    }
}

void __cdecl EmitGetFunction(sval_u func, sval_u sourcePos)
{
    EmitOpcode(0x15u, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 3);
    EmitFunction(func, sourcePos);
}

void __cdecl EmitCallExpression(sval_u expr, bool bStatement, scr_block_s *block)
{
    if (*expr.codePosValue == 23)
    {
        EmitCall(*(expr.codePosValue + 4), *(expr.codePosValue + 8), bStatement, block);
    }
    else if (*expr.codePosValue == 24)
    {
        EmitMethod(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), *(expr.codePosValue + 16), bStatement, block);
    }
}

void __cdecl EmitSelf(sval_u sourcePos)
{
    EmitOpcode(0xFu, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitLevel(sval_u sourcePos)
{
    EmitOpcode(0x10u, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitGame(sval_u sourcePos)
{
    EmitOpcode(0x11u, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitAnim(sval_u sourcePos)
{
    EmitOpcode(0x12u, 1, 0);
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
    value->type = 4;
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
        success = 1;
        for (node = *(sval_u**)exprlist.type; node; node = node[1].node)
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
            EmitOpcode(0x7Eu, -2, 0);
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
    EmitOpcode(0x76u, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitEmptyArray(sval_u sourcePos)
{
    EmitOpcode(0x25u, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
}


void __cdecl EmitAnimation(sval_u anim, sval_u sourcePos)
{
    EmitOpcode(0x13u, 1, 0);
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
    if (!constValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 640, 0, "%s", "constValue");
    constValue->value.type = 6;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalFloat(float value, sval_u sourcePos, VariableCompileValue *constValue)
{
    if (!constValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 702, 0, "%s", "constValue");
    constValue->value.type = 5;
    constValue->value.u.floatValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue)
{
    if (!constValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1110, 0, "%s", "constValue");
    constValue->value.type = 2;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalIString(unsigned int value, sval_u sourcePos, VariableCompileValue *constValue)
{
    if (!constValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1129, 0, "%s", "constValue");
    constValue->value.type = 3;
    constValue->value.u.intValue = value;
    constValue->sourcePos = sourcePos;
}

void __cdecl EvalUndefined(sval_u sourcePos, VariableCompileValue *constValue)
{
    if (!constValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 624, 0, "%s", "constValue");
    constValue->value.type = 0;
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
    if (*expr.codePosValue == 6)
        return EvalPrimitiveExpression(*(expr.codePosValue + 4), constValue);
    if (*expr.codePosValue == 49)
        return EvalBinaryOperatorExpression(
            *(expr.codePosValue + 4),
            *(expr.codePosValue + 8),
            *(expr.codePosValue + 12),
            *(expr.codePosValue + 16),
            constValue);
    return 0;
}

char __cdecl EvalPrimitiveExpressionList(sval_u exprlist, sval_u sourcePos, VariableCompileValue *constValue)
{
    sval_u *node; // [esp+0h] [ebp-30h]
    VariableCompileValue constValue2[3]; // [esp+4h] [ebp-2Ch] BYREF
    int expr_count; // [esp+28h] [ebp-8h]
    int i; // [esp+2Ch] [ebp-4h]

    if (!constValue)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2500, 0, "%s", "constValue");
    expr_count = GetExpressionCount(exprlist);
    if (expr_count == 1)
        return EvalExpression(exprlist.node[0].node[0].type, constValue); // KISAKTODO: might be wrong

    if (expr_count != 3)
        return 0;
    i = 0;
    for (node = *(sval_u**)exprlist.type; node; node = node[1].node)
    {
        if (!EvalExpression(node->node->type, &constValue2[i]))
            return 0;
        ++i;
    }
    Scr_CreateVector(constValue2, &constValue->value);
    constValue->sourcePos = sourcePos;
    return 1;
}

char __cdecl EvalPrimitiveExpression(sval_u expr, VariableCompileValue *constValue)
{
    char result; // al
    float value; // [esp+0h] [ebp-10h]

    switch (*expr.codePosValue)
    {
    case 7:
        EvalInteger(*(expr.codePosValue + 4), *(expr.codePosValue + 8), constValue);
        result = 1;
        break;
    case 8:
        EvalFloat(*(expr.codePosValue + 4), *(expr.codePosValue + 8), constValue);
        result = 1;
        break;
    case 9:
        EvalInteger(-*(expr.codePosValue + 4), *(expr.codePosValue + 8), constValue);
        result = 1;
        break;
    case 0xA:
        value = -*(expr.codePosValue + 4);
        EvalFloat(value, *(expr.codePosValue + 8), constValue);
        result = 1;
        break;
    case 0xB:
        EvalString(*(expr.codePosValue + 4), *(expr.codePosValue + 8), constValue);
        result = 1;
        break;
    case 0xC:
        EvalIString(*(expr.codePosValue + 4), *(expr.codePosValue + 8), constValue);
        result = 1;
        break;
    case 0x1F:
        EvalUndefined(*(expr.codePosValue + 4), constValue);
        result = 1;
        break;
    case 0x2E:
        result = EvalPrimitiveExpressionList(*(expr.codePosValue + 4), *(expr.codePosValue + 8), constValue);
        break;
    case 0x48:
        EvalInteger(0, *(expr.codePosValue + 4), constValue);
        result = 1;
        break;
    case 0x49:
        EvalInteger(1, *(expr.codePosValue + 4), constValue);
        result = 1;
        break;
    default:
        result = 0;
        break;
    }
    return result;

}

void __cdecl EmitCastBool(sval_u sourcePos)
{
    EmitOpcode(0x5Bu, 0, 0);
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
    EmitOpcode(0x61u, -1, 0);
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
    char result; // al

    switch (*expr.codePosValue)
    {
    case 0x11:
        EmitVariableExpression(*(expr.codePosValue + 4), block);
        result = 0;
        break;
    case 0x12:
        EmitGetFunction(*(expr.codePosValue + 4), *(expr.codePosValue + 8));
        result = 0;
        break;
    case 0x13:
        EmitCallExpression(*(expr.codePosValue + 4), 0, block);
        result = 0;
        break;
    case 0x20:
        EmitSelf(*(expr.codePosValue + 4));
        result = 0;
        break;
    case 0x22:
        EmitLevel(*(expr.codePosValue + 4));
        result = 0;
        break;
    case 0x23:
        EmitGame(*(expr.codePosValue + 4));
        result = 0;
        break;
    case 0x24:
        EmitAnim(*(expr.codePosValue + 4));
        result = 0;
        break;
    case 0x2E:
        result = EmitOrEvalPrimitiveExpressionList(*(expr.codePosValue + 4), *(expr.codePosValue + 8), constValue, block);
        break;
    case 0x34:
        EmitSize(*(expr.codePosValue + 4), *(expr.codePosValue + 8), block);
        result = 0;
        break;
    case 0x42:
        EmitEmptyArray(*(expr.codePosValue + 4));
        result = 0;
        break;
    case 0x43:
        EmitAnimation(*(expr.codePosValue + 4), *(expr.codePosValue + 8));
        result = 0;
        break;
    case 0x4A:
        EmitAnimTree(*(expr.codePosValue + 4));
        result = 0;
        break;
    case 0x4B:
        EmitBreakOn(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12));
        result = 0;
        break;
    default:
        result = EvalPrimitiveExpression(expr, constValue);
        break;
    }
    return result;
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
    EmitOpcode(0x60u, -1, 0);
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
        EmitOpcode(SLOBYTE(opcode.type), -1, 0);
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
    EmitOpcode(0x5Cu, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitBoolComplement(sval_u sourcePos)
{
    EmitOpcode(0x5Du, 0, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

char __cdecl EmitOrEvalExpression(sval_u expr, VariableCompileValue *constValue, scr_block_s *block)
{
    char result; // al

    switch (*expr.codePosValue)
    {
    case 6:
        result = EmitOrEvalPrimitiveExpression(*(expr.codePosValue + 4), constValue, block);
        break;
    case 0x2F:
        EmitBoolOrExpression(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), *(expr.codePosValue + 16), block);
        result = 0;
        break;
    case 0x30:
        EmitBoolAndExpression(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), *(expr.codePosValue + 16), block);
        result = 0;
        break;
    case 0x31:
        result = EmitOrEvalBinaryOperatorExpression(
            *(expr.codePosValue + 4),
            *(expr.codePosValue + 8),
            *(expr.codePosValue + 12),
            *(expr.codePosValue + 16),
            constValue,
            block);
        break;
    case 0x32:
        EmitExpression(*(expr.codePosValue + 4), block);
        EmitBoolNot(*(expr.codePosValue + 8));
        result = 0;
        break;
    case 0x33:
        EmitExpression(*(expr.codePosValue + 4), block);
        EmitBoolComplement(*(expr.codePosValue + 8));
        result = 0;
        break;
    default:
        result = 0;
        break;
    }
    return result;
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
        EmitOpcode(0x84u, 0, 0);
        EmitCanonicalString(expr.stringValue);
    }
    else
    {
        index = Scr_FindLocalVarIndex(expr.stringValue, sourcePos, 1, block);
        EmitOpcode((index != 0) + 54, 0, 0);
        EmitPreAssignmentPos();
        if (index)
            EmitByte(index);
        AddOpcodePos(sourcePos.stringValue, 1);
    }
}

void __cdecl EmitEvalArrayRef(sval_u sourcePos, sval_u indexSourcePos)
{
    EmitOpcode(0x23u, -1, 0);
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
    EmitOpcode(0x2Eu, 0, 0);
    EmitCanonicalString(field.stringValue);
}

void __cdecl EmitVariableExpressionRef(sval_u expr, scr_block_s *block)
{
    switch (*expr.codePosValue)
    {
    case 4:
        EmitLocalVariableRef(*(expr.codePosValue + 4), *(expr.codePosValue + 8), block);
        break;
    case 0xD:
        EmitArrayVariableRef(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), *(expr.codePosValue + 16), block);
        break;
    case 0xF:
        EmitFieldVariableRef(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), block);
        break;
    case 0x35:
    case 0x50:
        if (scrCompilePub.script_loading)
            CompileError(*(expr.codePosValue + 8), "$ and self field can only be used in the script debugger");
        else
            CompileError(*(expr.codePosValue + 8), "not an lvalue");
        break;
    default:
        return;
    }
}

void __cdecl EmitGameRef(sval_u sourcePos)
{
    EmitOpcode(0x14u, 0, 0);
    EmitPreAssignmentPos();
    AddOpcodePos(sourcePos.stringValue, 1);
}

void __cdecl EmitClearArray(sval_u sourcePos, sval_u indexSourcePos)
{
    EmitOpcode(0x24u, -1, 0);
    AddOpcodePos(indexSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitAssignmentPos();
}

void __cdecl EmitArrayPrimitiveExpressionRef(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if (*expr.codePosValue == 17)
    {
        EmitVariableExpressionRef(*(expr.codePosValue + 4), block);
    }
    else if (*expr.codePosValue == 35)
    {
        EmitGameRef(*(expr.codePosValue + 4));
    }
    else
    {
        CompileError(sourcePos.stringValue, "not an lvalue");
    }
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
    EmitOpcode(0x2Fu, 0, 0);
    AddOpcodePos(rhsSourcePos.stringValue, 0);
    EmitCanonicalString(field.stringValue);
    EmitAssignmentPos();
}

char __cdecl EmitClearVariableExpression(sval_u expr, sval_u rhsSourcePos, scr_block_s *block)
{
    switch (*expr.codePosValue)
    {
    case 4:
        return 0;
    case 0xD:
        EmitClearArrayVariable(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), *(expr.codePosValue + 16), block);
        return 1;
    case 0xF:
        EmitClearFieldVariable(*(expr.codePosValue + 4), *(expr.codePosValue + 8), *(expr.codePosValue + 12), rhsSourcePos, block);
        return 1;
    case 0x35:
    case 0x50:
        if (scrCompilePub.script_loading)
            CompileError(*(expr.codePosValue + 8), "$ and self field can only be used in the script debugger");
        else
            CompileError(*(expr.codePosValue + 8), "not an lvalue");
        return 1;
    default:
        return 1;
    }
}

void __cdecl EmitSetVariableField(sval_u sourcePos)
{
    EmitOpcode(0x39u, -1, 0);
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
    EmitOpcode(1u, -1, 0);
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
    EmitOpcode(0, 0, 0);
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
    EmitOpcode(0x4Cu, -1, 0);
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
    unsigned int v2; // ecx
    int j; // [esp+4h] [ebp-10h]
    unsigned int localVar; // [esp+8h] [ebp-Ch]
    unsigned int localVar_4; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if (to->localVarsPublicCount > from->localVarsCount)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 1034, 0, "%s", "to->localVarsPublicCount <= from->localVarsCount");
    for (i = 0; i < to->localVarsPublicCount || i < from->localVarsCreateCount; ++i)
    {
        localVar = from->localVars[i].name;
        localVar_4 = from->localVars[i].sourcePos;
        j = Scr_FindLocalVar(to, i, localVar);
        if (j < 0)
        {
            j = to->localVarsCount;
            Scr_CheckLocalVarsCount(j);
            ++to->localVarsCount;
        }
        if (j >= to->localVarsPublicCount && ++to->localVarsPublicCount > from->localVarsCount)
            MyAssertHandler(".\\script\\scr_compiler.cpp", 1049, 0, "%s", "to->localVarsPublicCount <= from->localVarsCount");
        while (j > i)
        {
            v2 = *&to->localVarsInitBits[8 * j + 4];
            to->localVars[j].name = *&to->localVarsInitBits[8 * j];
            to->localVars[j--].sourcePos = v2;
        }
        to->localVars[i].name = localVar;
        to->localVars[i].sourcePos = localVar_4;
        if (((1 << (i & 7)) & from->localVarsInitBits[i >> 3]) != 0)
            to->localVarsInitBits[i >> 3] |= 1 << (i & 7);
    }
    if (from->localVarsCreateCount > to->localVarsPublicCount)
        MyAssertHandler(
            ".\\script\\scr_compiler.cpp",
            1061,
            0,
            "%s",
            "from->localVarsCreateCount <= to->localVarsPublicCount");
    to->localVarsCreateCount = from->localVarsCreateCount;
    to->abortLevel = 0;
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
            EmitOpcode(0x17u, 0, 0);
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
    EmitOpcode(0x5Eu, -1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitShort(0);
    pos = scrCompileGlob.codePos;
    nextPos = TempMalloc(0);
    Scr_TransferBlock(block, ifStatBlock->block);
    EmitStatement(stmt, lastStatement, endSourcePos, ifStatBlock->block);
    if (*(ifStatBlock->codePosValue + 8) != block->localVarsCreateCount)
        MyAssertHandler(
            ".\\script\\scr_compiler.cpp",
            3166,
            0,
            "%s",
            "ifStatBlock->block->localVarsPublicCount == block->localVarsCreateCount");
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
    EmitOpcode(0x5Eu, -1, 0);
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
        EmitOpcode(0x62u, 0, 0);
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
            EmitOpcode(0x16u, 0, 0);
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
    if (*(int*)(forStatBlock->type + 4) > block->localVarsCount)
        MyAssertHandler(
            ".\\script\\scr_compiler.cpp",
            3566,
            0,
            "%s",
            "forStatBlock->block->localVarsCreateCount <= block->localVarsCount");
    block->localVarsCreateCount = *(int*)(forStatBlock->type + 4);
    Scr_TransferBlock(block, forStatPostBlock->block);
    pos1 = TempMalloc(0);
    if (*expr.codePosValue == 65)
    {
        constConditional = 0;
        if (EmitOrEvalExpression(*(expr.codePosValue + 4), &constValue, block))
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
        EmitOpcode(0x5Eu, -1, 0);
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
    EmitOpcode(0x63u, 0, 0);
    AddOpcodePos(forSourcePos.stringValue, 0);
    if (*stmt.codePosValue == 44)
        AddOpcodePos(*(stmt.codePosValue + 12), 1);
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
    if (*(int*)(whileStatBlock->type + 4) > block->localVarsCount)
        MyAssertHandler(
            ".\\script\\scr_compiler.cpp",
            3350,
            0,
            "%s",
            "whileStatBlock->block->localVarsCreateCount <= block->localVarsCount");
    block->localVarsCreateCount = *(int*)(whileStatBlock->type + 4);
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
        EmitOpcode(0x5Eu, -1, 0);
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
    EmitOpcode(0x63u, 0, 0);
    AddOpcodePos(whileSourcePos.stringValue, 0);
    if (*(int*)stmt.type == 44)
        AddOpcodePos(*(stmt.codePosValue + 12), 1);
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
    if (scrCompileGlob.forceNotCreate)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3778, 0, "%s", "!scrCompileGlob.forceNotCreate");
    scrCompileGlob.forceNotCreate = 1;
    EmitVariableExpressionRef(expr, block);
    if (!scrCompileGlob.forceNotCreate)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3781, 0, "%s", "scrCompileGlob.forceNotCreate");
    scrCompileGlob.forceNotCreate = 0;
    EmitOpcode(0x64u, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    EmitSetVariableField(sourcePos);
}

void __cdecl EmitDecStatement(sval_u expr, sval_u sourcePos, scr_block_s *block)
{
    if (scrCompileGlob.forceNotCreate)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3797, 0, "%s", "!scrCompileGlob.forceNotCreate");
    scrCompileGlob.forceNotCreate = 1;
    EmitVariableExpressionRef(expr, block);
    if (!scrCompileGlob.forceNotCreate)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3800, 0, "%s", "scrCompileGlob.forceNotCreate");
    scrCompileGlob.forceNotCreate = 0;
    EmitOpcode(0x65u, 1, 0);
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
    if (scrCompileGlob.bConstRefCount)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2820, 0, "%s", "!scrCompileGlob.bConstRefCount");
    scrCompileGlob.bConstRefCount = 1;
    EmitVariableExpression(lhs, block);
    if (!scrCompileGlob.bConstRefCount)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 2823, 0, "%s", "scrCompileGlob.bConstRefCount");
    scrCompileGlob.bConstRefCount = 0;
    EmitExpression(rhs, block);
    EmitOpcode(SLOBYTE(opcode.type), -1, 0);
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
        if (*(int*)node->type != 45)
            return 0;
        node = node[1].node;
    }
    return 1;
}

void __cdecl EmitStatementList(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
    sval_u *node; // [esp+4h] [ebp-8h]
    sval_u *nextNode; // [esp+8h] [ebp-4h]

    for (node = *(*(sval_u***)(val.type + 4)); node; node = nextNode)
    {
        nextNode = node[1].node;
        if (lastStatement && Scr_IsLastStatement(nextNode))
            EmitStatement(node->type, 1, endSourcePos, block);
        else
            EmitStatement(node->type, 0, endSourcePos, block);
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
            scrCompilePub.developer_statement = 1;
            EmitStatementList(val, 0, 0, devStatBlock->block);
            EmitRemoveLocalVars(devStatBlock->block, devStatBlock->block);
        }
        else
        {
            savedPos = TempMalloc(0);
            scrCompilePub.developer_statement = 2;
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
    EmitOpcode(0x33u, 0, 0);
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
        EmitSafeSetWaittillVariableField(node->node->type, *(unsigned int*)(node->type + 4), block);
    }
}

void __cdecl EmitWaittillStatement(
    sval_u obj,
    sval_u exprlist,
    sval_u sourcePos,
    sval_u waitSourcePos,
    scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-4h]

    node = *(*(sval_u***)(exprlist.type + 4));
    if (!node)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3849, 0, "%s", "node");
    EmitExpression(node->node->type, block);
    EmitPrimitiveExpression(obj, block);
    EmitOpcode(0x78u, -2, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    AddOpcodePos(*(unsigned int*)(node->type + 4), 0);
    EmitFormalWaittillParameterListRefInternal(node, block);
    EmitOpcode(0x34u, 0, 0);
}

void __cdecl EmitWaittillmatchStatement(
    sval_u obj,
    sval_u exprlist,
    sval_u sourcePos,
    sval_u waitSourcePos,
    scr_block_s *block)
{
    sval_u *node; // [esp+0h] [ebp-8h]
    sval_u *nodea; // [esp+0h] [ebp-8h]
    int exprCount; // [esp+4h] [ebp-4h]

    node = *(sval_u**)(*exprlist.codePosValue + 4);
    if (!node)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3881, 0, "%s", "node");
    for (exprCount = 0; ; ++exprCount)
    {
        node = node[1].node;
        if (!node)
            break;
        EmitExpression(node->node->type, block);
    }
    nodea = *(sval_u**)(*exprlist.codePosValue + 4);
    if (!nodea)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3895, 0, "%s", "node");
    EmitExpression(nodea->node->type, block);
    EmitPrimitiveExpression(obj, block);
    EmitOpcode(0x77u, -2 - exprCount, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(waitSourcePos.stringValue, 0);
    AddOpcodePos(sourcePos.stringValue, 0);
    AddOpcodePos(*(unsigned int*)(nodea->type + 4), 0);
    while (1)
    {
        nodea = nodea[1].node;
        if (!nodea)
            break;
        AddOpcodePos(*(unsigned int*)(nodea->type + 4), 0);
    }
    if (exprCount >= 256)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3914, 0, "%s", "exprCount < 256");
    EmitByte(exprCount);
    EmitOpcode(0x34u, 0, 0);
}

void __cdecl EmitWaittillFrameEnd(sval_u sourcePos)
{
    EmitOpcode(0x4Du, 0, 0);
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

    EmitOpcode(0x7Bu, 1, 0);
    AddOpcodePos(sourcePos.stringValue, 1);
    expr_count = 0;
    start_node = 0;
    for (node = *(sval_u**)exprlist.type; node; node = node[1].node)
    {
        start_node = node;
        EmitExpression(node->node->type, block);
        ++expr_count;
    }
    if (!start_node)
        MyAssertHandler(".\\script\\scr_compiler.cpp", 3940, 0, "%s", "start_node");
    EmitPrimitiveExpression(obj, block);
    EmitOpcode(0x79u, -expr_count - 2, 0);
    AddOpcodePos(notifySourcePos.stringValue, 16);
    AddOpcodePos(*(unsigned int*)(start_node->type + 4), 0);
    AddOpcodePos(sourcePos.stringValue, 0);
}

void __cdecl EmitEndOnStatement(sval_u obj, sval_u expr, sval_u sourcePos, sval_u exprSourcePos, scr_block_s *block)
{
    EmitExpression(expr, block);
    EmitPrimitiveExpression(obj, block);
    EmitOpcode(0x7Au, -2, 0);
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
    const char *v2; // eax
    unsigned int name; // [esp+4h] [ebp-4h]
    unsigned int namea; // [esp+4h] [ebp-4h]

    if (*expr.codePosValue == 7)
    {
        if (IsValidArrayIndex(*(expr.codePosValue + 4)))
        {
            name = GetInternalVariableIndex(*(expr.codePosValue + 4));
            EmitCaseStatementInfo(name, sourcePos);
        }
        else
        {
            v2 = va("case index %d out of range", *(expr.codePosValue + 4));
            CompileError(sourcePos.stringValue, v2);
        }
    }
    else if (*expr.codePosValue == 11)
    {
        namea = *(expr.codePosValue + 4);
        CompileTransferRefToString(namea, 1u);
        EmitCaseStatementInfo(namea, sourcePos);
    }
    else
    {
        CompileError(sourcePos.stringValue, "case expression must be an int or string");
    }
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
    for (node = *(sval_u**)(*val.codePosValue + 4); node; node = nextNode)
    {
        nextNode = node[1].node;
        if (*(int*)node->type == 61 || *(int *)node->type == 62)
        {
            if (scrCompileGlob.breakBlock)
            {
                if (!scrCompileGlob.bCanBreak)
                    MyAssertHandler(".\\script\\scr_compiler.cpp", 4053, 0, "%s", "scrCompileGlob.bCanBreak");
                scrCompileGlob.bCanBreak = 0;
                EmitRemoveLocalVars(scrCompileGlob.breakBlock, scrCompileGlob.breakBlock);
            }
            if (*(int *)node->type == 61)
            {
                scrCompileGlob.breakBlock = *(scr_block_s**)(node->type + 12);
                EmitCaseStatement(*(sval_u*)(node->type + 4), *(sval_u *)(node->type + 8));
            }
            else
            {
                scrCompileGlob.breakBlock = *(scr_block_s**)(node->type + 8);
                hasDefault = 1;
                EmitDefaultStatement(*(sval_u*)(node->type + 4));
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
    EmitOpcode(0x7Cu, -1, 0);
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
        EmitOpcode(0x62u, 0, 0);
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
        block->abortLevel = 1;
        EmitOpcode(0x62u, 0, 0);
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
        EmitOpcode(0x87u, 0, 0);
        AddOpcodePos(sourcePos.stringValue, 1);
    }
}

void __cdecl EmitProfStatement(sval_u profileName, sval_u sourcePos, unsigned __int8 op)
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
    EmitProfStatement(profileName, sourcePos, 0x85u);
}

void __cdecl EmitProfEndStatement(sval_u profileName, sval_u sourcePos)
{
    EmitProfStatement(profileName, sourcePos, 0x86u);
}

void __cdecl EmitStatement(sval_u val, bool lastStatement, unsigned int endSourcePos, scr_block_s *block)
{
    if (scrCompilePub.developer_statement == 3)
    {
        switch (*val.codePosValue)
        {
        case 2:
        case 0x19:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x3A:
            goto LABEL_3;
        default:
            CompileError(0, "illegal statement in debugger");
            break;
        }
    }
    else
    {
    LABEL_3:
        switch (*val.codePosValue)
        {
        case 2:
            EmitAssignmentStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), *(val.codePosValue + 16), block);
            break;
        case 0x19:
            EmitCallExpressionStatement(*(val.codePosValue + 4), block);
            break;
        case 0x1B:
            EmitReturnStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), block);
            break;
        case 0x1C:
            EmitEndStatement(*(val.codePosValue + 4), block);
            break;
        case 0x1D:
            EmitWaitStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), block);
            break;
        case 0x25:
            EmitIfStatement(
                *(val.codePosValue + 4),
                *(val.codePosValue + 8),
                *(val.codePosValue + 12),
                lastStatement,
                endSourcePos,
                block,
                (sval_u*)(val.codePosValue + 16));
            break;
        case 0x26:
            EmitIfElseStatement(
                *(val.codePosValue + 4),
                *(val.codePosValue + 8),
                *(val.codePosValue + 12),
                *(val.codePosValue + 16),
                *(val.codePosValue + 20),
                lastStatement,
                endSourcePos,
                block,
                (sval_u*)(val.codePosValue + 24),
                (sval_u*)(val.codePosValue + 28));
            break;
        case 0x27:
            EmitWhileStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), *(val.codePosValue + 16), block, (sval_u*)(val.codePosValue + 20));
            break;
        case 0x28:
            EmitForStatement(
                *(val.codePosValue + 4),
                *(val.codePosValue + 8),
                *(val.codePosValue + 12),
                *(val.codePosValue + 16),
                *(val.codePosValue + 20),
                *(val.codePosValue + 24),
                block,
                (sval_u*)(val.codePosValue + 28),
                (sval_u*)(val.codePosValue + 32));
            break;
        case 0x29:
            EmitIncStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), block);
            break;
        case 0x2A:
            EmitDecStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), block);
            break;
        case 0x2B:
            EmitBinaryEqualsOperatorExpression(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), *(val.codePosValue + 16), block);
            break;
        case 0x2C:
            EmitStatementList(*(val.codePosValue + 4), lastStatement, endSourcePos, block);
            break;
        case 0x2D:
            EmitDeveloperStatementList(*(val.codePosValue + 4), *(val.codePosValue + 8), block, (sval_u*)(val.codePosValue + 12));
            break;
        case 0x37:
            EmitWaittillStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), *(val.codePosValue + 16), block);
            break;
        case 0x38:
            EmitWaittillmatchStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), *(val.codePosValue + 16), block);
            break;
        case 0x39:
            EmitWaittillFrameEnd(*(val.codePosValue + 4));
            break;
        case 0x3A:
            EmitNotifyStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), *(val.codePosValue + 16), block);
            break;
        case 0x3B:
            EmitEndOnStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), *(val.codePosValue + 16), block);
            break;
        case 0x3C:
            EmitSwitchStatement(*(val.codePosValue + 4), *(val.codePosValue + 8), *(val.codePosValue + 12), lastStatement, endSourcePos, block);
            break;
        case 0x3D:
            CompileError(*(val.codePosValue + 8), "illegal case statement");
            break;
        case 0x3E:
            CompileError(*(val.codePosValue + 4), "illegal default statement");
            break;
        case 0x3F:
            EmitBreakStatement(*(val.codePosValue + 4), block);
            break;
        case 0x40:
            EmitContinueStatement(*(val.codePosValue + 4), block);
            break;
        case 0x4C:
            EmitBreakpointStatement(*(val.codePosValue + 4));
            break;
        case 0x4D:
            EmitProfBeginStatement(*(val.codePosValue + 4), *(val.codePosValue + 8));
            break;
        case 0x4E:
            EmitProfEndStatement(*(val.codePosValue + 4), *(val.codePosValue + 8));
            break;
        default:
            return;
        }
    }
}

void __cdecl EmitOpcode(unsigned int op, int offset, int callType)
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
    EmitOpcode(0x80u, 0, 0);
}
