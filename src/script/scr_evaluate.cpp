#include "scr_evaluate.h"
#include "scr_animtree.h"
#include "scr_variable.h"
#include "scr_main.h"
#include "scr_vm.h"
#include "scr_parser.h"
#include "scr_parsetree.h"
#include <qcommon/qcommon.h>
#include "scr_yacc.h"
#include "scr_compiler.h"

#include <qcommon/mem_track.h>
#include <setjmp.h>


//  unsigned int g_breakonObject      83043224     scr_evaluate.obj
//  int marker_scr_evaluate  83043238     scr_evaluate.obj
//  int g_breakonHit         8304323c     scr_evaluate.obj
//  unsigned int g_breakonString      83043244     scr_evaluate.obj

debugger_sval_s *g_debugExprHead;
int g_breakonExpr; // thread_local in blops?
int g_breakonHit;
unsigned int g_breakonObject;
unsigned int g_breakonString;

scrEvaluateGlob_t scrEvaluateGlob;
int g_script_error_level;

void __cdecl TRACK_scr_evaluate()
{
    track_static_alloc_internal(&scrEvaluateGlob, 16, "scrEvaluateGlob", 7);
}

int __cdecl Scr_CompareCanonicalStrings(unsigned int *arg1, unsigned int *arg2)
{
    return scrEvaluateGlob.canonicalStringLookup[*arg1] - scrEvaluateGlob.canonicalStringLookup[*arg2];
}

void __cdecl Scr_ArchiveCanonicalStrings()
{
    char v0; // [esp+13h] [ebp-35h]
    char *v1; // [esp+18h] [ebp-30h]
    const char *v2; // [esp+1Ch] [ebp-2Ch]
    unsigned int stringValue; // [esp+30h] [ebp-18h]
    unsigned int stringValuea; // [esp+30h] [ebp-18h]
    unsigned int len; // [esp+34h] [ebp-14h]
    int lena; // [esp+34h] [ebp-14h]
    unsigned __int16 canonicalStr; // [esp+38h] [ebp-10h]
    const char *s; // [esp+3Ch] [ebp-Ch]
    char *debugString; // [esp+40h] [ebp-8h]
    unsigned __int16 i; // [esp+44h] [ebp-4h]
    unsigned __int16 ia; // [esp+44h] [ebp-4h]

    len = 0;
    for (stringValue = 0; stringValue < 0x10000; ++stringValue)
    {
        if (scrCompilePub.canonicalStrings[stringValue])
            len += strlen(SL_ConvertToString(stringValue)) + 1;
    }
    scrEvaluateGlob.archivedCanonicalStringsBuf = (char *)Hunk_AllocDebugMem(len);
    scrEvaluateGlob.archivedCanonicalStrings = (ArchivedCanonicalStringInfo *)Hunk_AllocDebugMem(8 * scrVarPub.canonicalStrCount);
    scrEvaluateGlob.canonicalStringLookup = (int *)Hunk_AllocDebugMem(4 * scrVarPub.canonicalStrCount + 4);
    i = 0;
    lena = 0;
    for (stringValuea = 0; stringValuea < 0x10000; ++stringValuea)
    {
        canonicalStr = scrCompilePub.canonicalStrings[stringValuea];
        if (canonicalStr)
        {
            if (canonicalStr > (int)scrVarPub.canonicalStrCount)
                MyAssertHandler(".\\script\\scr_evaluate.cpp", 149, 0, "%s", "canonicalStr <= scrVarPub.canonicalStrCount");
            s = SL_ConvertToString(stringValuea);
            debugString = &scrEvaluateGlob.archivedCanonicalStringsBuf[lena];
            v2 = s;
            v1 = &scrEvaluateGlob.archivedCanonicalStringsBuf[lena];
            do
            {
                v0 = *v2;
                *v1++ = *v2++;
            } while (v0);
            scrEvaluateGlob.archivedCanonicalStrings[i].canonicalStr = canonicalStr;
            scrEvaluateGlob.archivedCanonicalStrings[i].value = debugString;
            lena += strlen(s) + 1;
            ++i;
        }
    }
    if (i != scrVarPub.canonicalStrCount)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 160, 0, "%s", "i == scrVarPub.canonicalStrCount");
    qsort(
        scrEvaluateGlob.archivedCanonicalStrings,
        scrVarPub.canonicalStrCount,
        8u,
        (int(__cdecl *)(const void *, const void *))CompareCanonicalStrings);
    for (ia = 0; ia < (int)scrVarPub.canonicalStrCount; ++ia)
    {
        if (!scrEvaluateGlob.archivedCanonicalStrings[ia].canonicalStr)
            MyAssertHandler(
                ".\\script\\scr_evaluate.cpp",
                166,
                0,
                "%s",
                "scrEvaluateGlob.archivedCanonicalStrings[i].canonicalStr");
        if (scrEvaluateGlob.archivedCanonicalStrings[ia].canonicalStr > (int)scrVarPub.canonicalStrCount)
            MyAssertHandler(
                ".\\script\\scr_evaluate.cpp",
                167,
                0,
                "%s",
                "scrEvaluateGlob.archivedCanonicalStrings[i].canonicalStr <= scrVarPub.canonicalStrCount");
        scrEvaluateGlob.canonicalStringLookup[scrEvaluateGlob.archivedCanonicalStrings[ia].canonicalStr] = ia;
    }
    *scrEvaluateGlob.canonicalStringLookup = 0;
}

int __cdecl CompareCanonicalStrings(const char **arg1, const char **arg2)
{
    return strcmp(arg1[1], arg2[1]);
}

const char *__cdecl Scr_GetCanonicalString(unsigned int fieldName)
{
    if (!fieldName)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 183, 0, "%s", "fieldName");
    if (fieldName > scrVarPub.canonicalStrCount)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 184, 0, "%s", "fieldName <= scrVarPub.canonicalStrCount");
    if (scrEvaluateGlob.canonicalStringLookup[fieldName] >= (unsigned int)scrVarPub.canonicalStrCount)
        MyAssertHandler(
            ".\\script\\scr_evaluate.cpp",
            185,
            0,
            "%s",
            "(unsigned)scrEvaluateGlob.canonicalStringLookup[fieldName] < scrVarPub.canonicalStrCount");
    return scrEvaluateGlob.archivedCanonicalStrings[scrEvaluateGlob.canonicalStringLookup[fieldName]].value;
}

void __cdecl Scr_InitEvaluate()
{
    scrEvaluateGlob.archivedCanonicalStringsBuf = 0;
    scrEvaluateGlob.archivedCanonicalStrings = 0;
    scrEvaluateGlob.canonicalStringLookup = 0;
    KISAK_NULLSUB();
}

void __cdecl Scr_EndLoadEvaluate()
{
    if (scrVarPub.developer)
        Scr_ArchiveCanonicalStrings();
}

void __cdecl Scr_ShutdownEvaluate()
{
    if (scrEvaluateGlob.archivedCanonicalStringsBuf)
    {
        Hunk_FreeDebugMem(scrEvaluateGlob.archivedCanonicalStringsBuf);
        scrEvaluateGlob.archivedCanonicalStringsBuf = 0;
    }
    if (scrEvaluateGlob.archivedCanonicalStrings)
    {
        Hunk_FreeDebugMem(scrEvaluateGlob.archivedCanonicalStrings);
        scrEvaluateGlob.archivedCanonicalStrings = 0;
    }
    if (scrEvaluateGlob.canonicalStringLookup)
    {
        Hunk_FreeDebugMem(scrEvaluateGlob.canonicalStringLookup);
        scrEvaluateGlob.canonicalStringLookup = 0;
    }
    KISAK_NULLSUB();
}

unsigned __int16 __cdecl Scr_CompileCanonicalString(unsigned int stringValue)
{
    int v2; // [esp+4h] [ebp-24h]
    int low; // [esp+18h] [ebp-10h]
    int middle; // [esp+1Ch] [ebp-Ch]
    const char *s; // [esp+20h] [ebp-8h]
    int high; // [esp+24h] [ebp-4h]

    s = SL_ConvertToString(stringValue);
    low = 0;
    high = scrVarPub.canonicalStrCount;
    while (low < high)
    {
        middle = (high + low) / 2;
        v2 = strcmp(s, scrEvaluateGlob.archivedCanonicalStrings[middle].value);
        if (v2 >= 0)
        {
            if (v2 <= 0)
                return scrEvaluateGlob.archivedCanonicalStrings[middle].canonicalStr;
            low = middle + 1;
        }
        else
        {
            high = (high + low) / 2;
        }
    }
    return 0;
}

void __cdecl Scr_GetFieldValue(unsigned int objectId, const char *fieldName, int len, char *text)
{
    unsigned __int16 v4; // ax
    unsigned int stringValue; // [esp+0h] [ebp-Ch]
    VariableValue tempValue; // [esp+4h] [ebp-8h] BYREF

    stringValue = SL_FindString(fieldName);
    if (stringValue)
    {
        v4 = Scr_CompileCanonicalString(stringValue);
        if (v4)
        {
            Scr_EvalFieldVariableInternal(objectId, v4, &tempValue);
            if (scrVarPub.error_message)
            {
                Scr_ClearErrorMessage();
                *text = 0;
            }
            else
            {
                Scr_GetValueString(0, &tempValue, len, text);
                RemoveRefToValue(tempValue.type, tempValue.u);
            }
        }
        else
        {
            *text = 0;
        }
    }
    else
    {
        *text = 0;
    }
}

void __cdecl Scr_GetValueString(unsigned int localId, VariableValue *value, int len, char *s)
{
    const XAnim_s *Anims; // eax
    char *AnimDebugName; // eax
    char EntClassId; // al
    unsigned int ArraySize; // eax
    unsigned int intValue; // [esp+14h] [ebp-20h]
    int EntNum; // [esp+14h] [ebp-20h]
    unsigned int type; // [esp+2Ch] [ebp-8h]
    VariableUnion id; // [esp+30h] [ebp-4h]

    if (value->type >= 0x17u)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 342, 0, "%s", "(unsigned)value->type < VAR_COUNT");
    switch (value->type)
    {
    case 0:
        I_strncpyz(s, "undefined", len);
        break;
    case 1:
        id.intValue = value->u.intValue;
        type = GetObjectType(value->u.intValue);
        if (type == 20)
        {
            EntNum = Scr_GetEntNum(id.stringValue);
            EntClassId = Scr_GetEntClassId(id.stringValue);
            Com_sprintf(s, len, "$%c%i", EntClassId, EntNum);
        }
        else if (id.intValue == scrVarPub.levelId)
        {
            I_strncpyz(s, "level", len);
        }
        else if (id.intValue == scrVarPub.animId)
        {
            I_strncpyz(s, "anim", len);
        }
        else
        {
            if (type >= 0x17)
                MyAssertHandler(".\\script\\scr_evaluate.cpp", 397, 0, "%s", "(unsigned)type < VAR_COUNT");
            switch (type)
            {
            case 0xEu:
            case 0xFu:
            case 0x10u:
            case 0x11u:
                Com_sprintf(s, len, "$t%i", id.intValue);
                break;
            case 0x12u:
                if (!localId || id.intValue != Scr_GetSelf(localId))
                    goto LABEL_26;
                I_strncpyz(s, "self", len);
                break;
            case 0x15u:
                ArraySize = GetArraySize(id.stringValue);
                Com_sprintf(s, len, "<array of size %i>", ArraySize);
                break;
            default:
            LABEL_26:
                Com_sprintf(s, len, "<%s>", var_typename[type]);
                break;
            }
        }
        break;
    case 2:
        Com_sprintf(s, len, "\"%s\"", SL_ConvertToString(value->u.intValue));
        break;
    case 3:
        Com_sprintf(s, len, "&\"%s\"", SL_ConvertToString(value->u.intValue));
        break;
    case 4:
        sprintf(
            s,
            "(%g, %g, %g)",
            *(float *)value->u.intValue,
            *(float *)(value->u.intValue + 4),
            *(float *)(value->u.intValue + 8));
        break;
    case 5:
        Com_sprintf(s, len, "%g", value->u.floatValue);
        break;
    case 6:
        Com_sprintf(s, len, "%i", value->u.intValue);
        break;
    case 9:
        Scr_GetCodePos((const char *)(value->u.intValue - 1), 1u, s, len);
        break;
    case 0xB:
        intValue = (unsigned __int16)value->u.intValue;
        Anims = Scr_GetAnims(HIWORD(value->u.intValue));
        AnimDebugName = XAnimGetAnimDebugName(Anims, intValue);
        Com_sprintf(s, len, "%%%s", AnimDebugName);
        break;
    default:
        Com_sprintf(s, len, "<%s>", var_typename[value->type]);
        break;
    }
}

void __cdecl Scr_EvalArrayVariable(unsigned int arrayId, VariableValue *value)
{
    VariableValue parentValue; // [esp+0h] [ebp-8h] BYREF

    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 481, 0, "%s", "scrVarPub.evaluate");
    parentValue.type = VAR_POINTER;
    parentValue.u.intValue = arrayId;
    AddRefToObject(arrayId);
    Scr_EvalArrayVariableInternal(&parentValue, value);
}

void __cdecl Scr_EvalArrayVariableInternal(VariableValue *parentValue, VariableValue *value)
{
    if (scrVarPub.error_message || (Scr_EvalArray(parentValue, value), scrVarPub.error_message))
    {
        Scr_ClearValue(value);
        Scr_ClearValue(parentValue);
    }
}

void __cdecl Scr_ClearValue(VariableValue *value)
{
    RemoveRefToValue(value->type, value->u);
    value->type = VAR_UNDEFINED;
}

void __cdecl Scr_EvalFieldVariableInternal(unsigned int objectId, unsigned int fieldName, VariableValue *value)
{
    unsigned int outparamcount; // [esp+8h] [ebp-8h]
    VariableValue *savedTop; // [esp+Ch] [ebp-4h]

    if (!fieldName)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1087, 0, "%s", "fieldName");
    if (IsFieldObject(objectId))
    {
        outparamcount = scrVmPub.outparamcount;
        scrVmPub.outparamcount = 0;
        savedTop = scrVmPub.top;
        *value = Scr_FindVariableField(objectId, fieldName);
        scrVmPub.outparamcount = outparamcount;
        scrVmPub.top = savedTop;
        if (!value->type && !FindVariable(objectId, fieldName))
            Scr_Error("unknown field");
    }
    else
    {
        value->type = VAR_UNDEFINED;
        Scr_Error("not a field object");
    }
}

void __cdecl Scr_EvalFieldVariable(unsigned int fieldName, VariableValue *value, unsigned int objectId)
{
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1122, 0, "%s", "scrVarPub.evaluate");
    Scr_EvalFieldVariableInternal(objectId, fieldName, value);
}

void __cdecl Scr_CompileExpression(sval_u *expr)
{
    switch (expr->node[0].type)
    {
    case ENUM_primitive_expression:
        Scr_CompilePrimitiveExpression(&expr->node[1]);
        *expr = debugger_node1(ENUM_primitive_expression, expr->node[1]);
        break;
    case ENUM_bool_or:
    case ENUM_bool_and:
        Scr_CompileExpression(&expr->node[1]);
        Scr_CompileExpression(&expr->node[2]);
        *expr = debugger_node2(
            expr->node[0].type,
            expr->node[1],
            expr->node[2]);
        break;
    case ENUM_binary:
        Scr_CompileExpression(&expr->node[1]);
        Scr_CompileExpression(&expr->node[2]);
        *expr = debugger_node3(
            expr->node[0].type,
            expr->node[1],
            expr->node[2],
            expr->node[3]);
        break;
    case ENUM_bool_not:
    case ENUM_bool_complement:
        Scr_CompileExpression(&expr->node[1]);
        *expr = debugger_node1(expr->node[0].type, expr->node[1]);
        break;
    default:
        return;
    }
}

void __cdecl Scr_CompilePrimitiveExpression(sval_u *expr)
{
    sval_u tempVariableId; // [esp+2Ch] [ebp-8h]
    sval_u tempVariableIda; // [esp+2Ch] [ebp-8h]

    switch (expr->node[0].idValue)
    {
    case ENUM_integer:
    case ENUM_float:
    case ENUM_minus_integer:
    case ENUM_minus_float:
        *expr = debugger_node1(expr->node[0].type, expr->node[1]);
        break;
    case ENUM_string:
    case ENUM_istring:
        *expr = debugger_string(expr->node[0].type, (char *)SL_ConvertToString(*(unsigned int *)(expr->type + 4)));
        break;
    case ENUM_variable:
        Scr_CompileVariableExpression(&expr->node[1]);
        tempVariableId.block = (scr_block_s*)AllocValue();
        *expr = debugger_node2(ENUM_variable, expr->node[1], tempVariableId);
        break;
    case ENUM_call_expression:
        if (!Scr_CompileCallExpression(&expr->node[1]))
            goto LABEL_13;
        tempVariableIda.block = (scr_block_s*)AllocValue();
        *expr = debugger_node2(ENUM_call_expression, expr->node[1], tempVariableIda);
        break;
    case ENUM_undefined:
    case ENUM_level:
    case ENUM_game:
    case ENUM_anim:
    case ENUM_empty_array:
    case ENUM_false:
    case ENUM_true:
        *expr = debugger_node0(expr->node[0].type);
        break;
    case ENUM_self:
        *expr = debugger_node2(expr->node[0].type, 0, 0);
        break;
    case ENUM_expression_list:
        Scr_CompilePrimitiveExpressionList(&expr->node[1]);
        *expr = debugger_node1(ENUM_expression_list, expr->node[1]);
        break;
    case ENUM_size_field:
        Scr_CompilePrimitiveExpression(&expr->node[1]);
        *expr = debugger_node1(ENUM_size_field, expr->node[1]);
        break;
    case ENUM_breakon:
        ++scrVmDebugPub.checkBreakon;
        g_breakonExpr = 1;
        Scr_CompilePrimitiveExpression(&expr->node[1]);
        Scr_CompileExpression(&expr->node[2]);
        *expr = debugger_node3(
            expr->node[0].type,
            expr->node[1],
            expr->node[2],
            0);
        break;
    default:
    LABEL_13:
        *expr = debugger_node0(ENUM_bad_expression);
        break;
    }
}

void __cdecl Scr_CompileVariableExpression(sval_u *expr)
{
    signed int ObjectType; // [esp+14h] [ebp-54h]
    int argumentIndex; // [esp+48h] [ebp-20h]
    sval_u classnum; // [esp+4Ch] [ebp-1Ch]
    sval_u tempVariableId; // [esp+50h] [ebp-18h]
    const char *s; // [esp+58h] [ebp-10h]
    sval_u entnum; // [esp+5Ch] [ebp-Ch]
    sval_u idValue; // [esp+64h] [ebp-4h]

    switch (expr->node[0].type)
    {
    case ENUM_local_variable:
        *(unsigned int *)(expr->type + 4) = Scr_CompileCanonicalString(*(unsigned int *)(expr->type + 4));
        if (*(unsigned int *)(expr->type + 4))
        {
            tempVariableId.block = (scr_block_s*)AllocValue();
            *expr = debugger_node4(ENUM_local_variable, expr->node[1], 0, 0, tempVariableId);
        }
        else
        {
            *expr = debugger_node0(ENUM_unknown_variable);
        }
        break;
    case ENUM_array_variable:
        Scr_CompileExpression(&expr->node[2]);
        Scr_CompilePrimitiveExpression(&expr->node[1]);
        *expr = debugger_node2(ENUM_array_variable, expr->node[1], expr->node[2]);
        break;
    case ENUM_field_variable:
        Scr_CompilePrimitiveExpressionFieldObject(&expr->node[1]);
        *(unsigned int *)(expr->type + 8) = Scr_CompileCanonicalString(*(unsigned int *)(expr->type + 8));
        if (*(unsigned int *)(expr->type + 8))
            *expr = debugger_node3(ENUM_field_variable, expr->node[1], expr->node[2], 0);
        else
            *expr = debugger_node0(ENUM_unknown_field);
        break;
    case ENUM_self_field:
        Scr_CompilePrimitiveExpression(&expr->node[1]);
        *expr = debugger_node1(ENUM_self_field, expr->node[1]);
        break;
    case ENUM_object:
        s = SL_ConvertToString(*(unsigned int *)(expr->type + 4));
        if (*s == 116)
        {
            idValue.intValue = atoi(s + 1);
            if (!idValue.type)
                goto LABEL_28;
            if (idValue.type >= 0x8000u)
                goto LABEL_28;
            if (IsObjectFree(idValue.stringValue))
                goto LABEL_28;
            ObjectType = GetObjectType(idValue.stringValue);
            if (ObjectType < VAR_THREAD || ObjectType > VAR_CHILD_THREAD && ObjectType != VAR_DEAD_THREAD)
                goto LABEL_28;
            *expr = debugger_node1(ENUM_thread_object, idValue);
            AddRefToObject(idValue.stringValue);
        }
        else if (*s == 97)
        {
            argumentIndex = atoi(s + 1);
            if (argumentIndex <= 0 && strcmp(s + 1, "0"))
                goto LABEL_28;
            *expr = debugger_node1(ENUM_argument, (sval_u)argumentIndex);
        }
        else
        {
            classnum.intValue = Scr_GetClassnumForCharId(*s);
            if (classnum.type < 0)
                goto LABEL_28;
            entnum.intValue = atoi(s + 1);
            if (!entnum.type && s[1] != 48)
                goto LABEL_28;
            *expr = debugger_node3(ENUM_object, classnum, entnum, 0);
        }
        break;
    default:
    LABEL_28:
        *expr = debugger_node0(ENUM_bad_expression);
        break;
    }
}

void __cdecl Scr_CompilePrimitiveExpressionFieldObject(sval_u *expr)
{
    sval_u tempVariableId; // [esp+18h] [ebp-8h]
    sval_u tempVariableIda; // [esp+18h] [ebp-8h]

    switch (expr->node[0].type)
    {
    case ENUM_variable:
        Scr_CompileVariableExpression(&expr->node[1]);
        tempVariableId.block = (scr_block_s*)AllocValue();
        *expr = debugger_node2(ENUM_variable, expr->node[1], tempVariableId);
        break;
    case ENUM_call_expression:
        Scr_CompileCallExpression(&expr->node[1]);
        tempVariableIda.block = (scr_block_s*)AllocValue();
        *expr = debugger_node2(ENUM_call_expression, expr->node[1], tempVariableIda);
        break;
    case ENUM_self:
        *expr = debugger_node2(expr->node[0].type, 0, 0);
        break;
    case ENUM_level:
    case ENUM_anim:
        *expr = debugger_node0(expr->node[0].type);
        break;
    default:
        *expr = debugger_node0(ENUM_bad_expression);
        break;
    }
}

//int __cdecl GetExpressionCount(sval_u exprlist)
//{
//    sval_u *node; // [esp+0h] [ebp-8h]
//    int expr_count; // [esp+4h] [ebp-4h]
//
//    expr_count = 0;
//    for (node = *(sval_u**)exprlist.type; node; node = node[1].node)
//        ++expr_count;
//    return expr_count;
//}

void __cdecl Scr_CompilePrimitiveExpressionList(sval_u *exprlist)
{
    sval_u *nodea; // [esp+8h] [ebp-18h]
    sval_u *node; // [esp+8h] [ebp-18h]
    int expr_count; // [esp+Ch] [ebp-14h]
    int i; // [esp+10h] [ebp-10h]
    sval_u expr[3]; // [esp+14h] [ebp-Ch]

    expr_count = GetExpressionCount((sval_u)exprlist->type);
    if (expr_count == 1)
    {
        nodea = *(sval_u **)exprlist->type;
        Scr_CompileExpression(nodea->node);
        exprlist->type = nodea->node->type;
    }
    else if (expr_count == 3)
    {
        i = 0;
        for (node = *(sval_u **)exprlist->type; node; node = node[1].node)
        {
            Scr_CompileExpression(node->node);
            expr[i++] = (sval_u)node->node->type;
        }
        exprlist->type = debugger_node3(ENUM_vector, expr[0], expr[1], expr[2]).type;
    }
    else
    {
        exprlist->type = debugger_node0(ENUM_bad_expression).type;
    }
}

char __cdecl Scr_CompileCallExpression(sval_u *expr)
{
    int v2; // [esp+0h] [ebp-Ch]

    v2 = *(unsigned int *)expr->type;
    if (v2 == 23)
    {
        if (Scr_CompileFunction(&expr->node[1], &expr->node[2]))
        {
            *expr = debugger_node2(ENUM_call, expr->node[1], expr->node[2]);
            return 1;
        }
    }
    else if (v2 == 24
        && Scr_CompileMethod(&expr->node[1], &expr->node[2], (sval_u *)(expr->type + 12)))
    {
        *expr = debugger_node3(
            ENUM_method,
            expr->node[1],
            expr->node[2],
            expr->node[3]);
        return 1;
    }
    return 0;
}

unsigned int __cdecl Scr_GetBuiltin(sval_u func_name)
{
    if (func_name.node[0].type != ENUM_script_call)
        return 0;

    func_name = func_name.node[1];

    if (func_name.node[0].type != ENUM_function)
        return 0;

    func_name = func_name.node[1];

    if (func_name.node[0].type != ENUM_local_function)
        return 0;

    func_name = func_name.node[1];

    if (scrCompilePub.developer_statement == 3 || !FindVariable(scrCompileGlob.fileId, func_name.idValue))
        return func_name.idValue;

    return 0;
}

char __cdecl Scr_CompileFunction(sval_u *func_name, sval_u *params)
{
    void(__cdecl * func)(); // [esp+0h] [ebp-10h]
    unsigned int name; // [esp+4h] [ebp-Ch]
    const char *pName; // [esp+8h] [ebp-8h] BYREF
    int type; // [esp+Ch] [ebp-4h] BYREF

    name = Scr_GetBuiltin((sval_u)func_name->type);
    if (!name)
        return 0;
    pName = SL_ConvertToString(name);
    type = 0;
    func = Scr_GetFunction(&pName, &type);
    if (!func)
        return 0;
    func_name->block = (scr_block_s*)func;
    Scr_CompileCallExpressionList(params);
    return 1;
}

void __cdecl Scr_CompileCallExpressionList(sval_u *exprlist)
{
    sval_u *node; // [esp+8h] [ebp-8h]
    sval_u expr; // [esp+Ch] [ebp-4h]

    expr.type = debugger_node0(ENUM_NOP).type;
    for (node = *(sval_u **)exprlist->type; node; node = node[1].node)
    {
        Scr_CompileExpression(node->node);
        expr.type = debugger_prepend_node((sval_u)node->node->type, expr).type;
    }
    exprlist->type = expr.type;
}

char __cdecl Scr_CompileMethod(sval_u *expr, sval_u *func_name, sval_u *params)
{
    void(__cdecl * meth)(scr_entref_t); // [esp+0h] [ebp-10h]
    unsigned int name; // [esp+4h] [ebp-Ch]
    const char *pName; // [esp+8h] [ebp-8h] BYREF
    int type; // [esp+Ch] [ebp-4h] BYREF

    name = Scr_GetBuiltin((sval_u)func_name->type);
    if (!name)
        return 0;
    pName = SL_ConvertToString(name);
    type = 0;
    meth = Scr_GetMethod(&pName, &type);
    if (!meth)
        return 0;
    Scr_CompilePrimitiveExpression(expr);
    func_name->block = (scr_block_s*)meth;
    Scr_CompileCallExpressionList(params);
    return 1;
}

void __cdecl Scr_CompileText(const char *text, ScriptExpression_t *scriptExpr)
{
    g_debugExprHead = 0;
    g_breakonExpr = 0;
    Scr_CompileTextInternal(text, scriptExpr);
    if (!g_debugExprHead)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 2191, 0, "%s", "g_debugExprHead");
    scriptExpr->exprHead = g_debugExprHead;
    scriptExpr->breakonExpr = g_breakonExpr;
}

void __cdecl Scr_CompileTextInternal(const char *text, ScriptExpression_t *scriptExpr)
{
    const char *varUsagePos; // [esp+24h] [ebp-18h]
    char *start; // [esp+28h] [ebp-14h]
    char *end; // [esp+2Ch] [ebp-10h]
    HunkUser *user; // [esp+30h] [ebp-Ch]
    unsigned int *expr; // [esp+38h] [ebp-4h]

    if (!strcmp(text, "<locals>"))
    {
        scriptExpr->parseData = debugger_node1(ENUM_local, 0);
    }
    else
    {
        Scr_InitAllocNode();
        scrCompilePub.in_ptr = "*";
        scrCompilePub.parseBuf = text;
        if (scrVarPub.error_message)
            MyAssertHandler(".\\script\\scr_evaluate.cpp", 2109, 0, "%s", "!scrVarPub.error_message");
        if (!scrVarPub.evaluate)
            MyAssertHandler(".\\script\\scr_evaluate.cpp", 2111, 0, "%s", "scrVarPub.evaluate");
        ScriptParse(&scriptExpr->parseData, 2u);
        if (scrVarPub.error_message)
        {
            if (*text)
                Com_PrintError(23, "%s\n", scrVarPub.error_message);
            Scr_ClearErrorMessage();
            scriptExpr->parseData = debugger_node0(ENUM_bad_expression);
            SL_ShutdownSystem(2);
        }
        else
        {
            scrCompilePub.developer_statement = 3;
            expr = (unsigned int *)scriptExpr->parseData.type;
            scriptExpr->parseData.type = (Enum_t)*(unsigned int *)(scriptExpr->parseData.type + 4);
            if (*expr == 65)
            {
                varUsagePos = scrVarPub.varUsagePos;
                if (!scrVarPub.varUsagePos)
                    scrVarPub.varUsagePos = "<debug expression>";
                Scr_CompileExpression(&scriptExpr->parseData);
                scrVarPub.varUsagePos = varUsagePos;
                if (scrVarPub.error_message)
                    MyAssertHandler(".\\script\\scr_evaluate.cpp", 2137, 0, "%s", "!scrVarPub.error_message");
                SL_ShutdownSystem(2);
            }
            else
            {
                if (*expr != 83)
                    MyAssertHandler(".\\script\\scr_evaluate.cpp", 2142, 0, "%s", "expr.node[0].type == ENUM_statement");
                user = Hunk_UserCreate(0x10000, "Scr_CompileTextInternal", 0, 0, 0);
                TempMemoryReset(user);
                start = TempMalloc(0);
                Scr_CompileStatement(scriptExpr->parseData);
                if (scrVarPub.error_message)
                {
                    Com_PrintError(23, "%s\n", scrVarPub.error_message);
                    Scr_ClearErrorMessage();
                    scriptExpr->parseData = debugger_node0(ENUM_bad_statement);
                    SL_ShutdownSystem(2);
                }
                else
                {
                    end = TempMalloc(0);
                    scriptExpr->parseData = debugger_buffer(ENUM_statement, start, end - start, 32);
                }
                Hunk_UserDestroy(user);
            }
        }
        Scr_ShutdownAllocNode();
    }
}

bool __cdecl Scr_EvalScriptExpression(
    ScriptExpression_t *expr,
    unsigned int localId,
    VariableValue *value,
    bool freezeScope,
    bool freezeObjects)
{
    iassert(expr);
    scrEvaluateGlob.freezeScope = freezeScope;
    scrEvaluateGlob.freezeObjects = freezeObjects;
    scrEvaluateGlob.objectChanged = 0;
    Scr_EvalExpression(expr->parseData, localId, value);
    return scrEvaluateGlob.objectChanged;
}

void __cdecl Scr_EvalExpression(sval_u expr, unsigned int localId, VariableValue *value)
{
    switch (expr.node[0].type)
    {
    case ENUM_primitive_expression:
        Scr_EvalPrimitiveExpression(expr.node[1], localId, value);
        break;
    case ENUM_bool_or:
        Scr_EvalBoolOrExpression(expr.node[1], expr.node[2], localId, value);
        break;
    case ENUM_bool_and:
        Scr_EvalBoolAndExpression(expr.node[1], expr.node[2], localId, value);
        break;
    case ENUM_binary:
        Scr_EvalBinaryOperatorExpression(
            expr.node[1],
            expr.node[2],
            expr.node[3],
            localId,
            value);
        break;
    case ENUM_bool_not:
        Scr_EvalExpression(expr.node[1], localId, value);
        Scr_EvalBoolNot(value);
        break;
    case ENUM_bool_complement:
        Scr_EvalExpression(expr.node[1], localId, value);
        Scr_EvalBoolComplement(value);
        break;
    case ENUM_vector:
        Scr_EvalVector(
            expr.node[1],
            expr.node[2],
            expr.node[3],
            localId,
            value);
        break;
    case ENUM_local:
        if (scrEvaluateGlob.freezeScope)
            localId = expr.node[1].idValue;
        if (localId && Scr_IsThreadAlive(localId))
        {
            value->type = VAR_POINTER;
            value->u.intValue = localId;
            AddRefToObject(localId);
        }
        else
        {
            localId = 0;
            value->type = VAR_UNDEFINED;
            Scr_Error("thread not active");
        }
        if (scrEvaluateGlob.freezeObjects)
        {
            if (expr.node[1].idValue != localId)
                scrEvaluateGlob.objectChanged = 1;
        }
        else
        {
            expr.node[1].idValue = localId;
        }
        break;
    default:
        value->type = VAR_UNDEFINED;
        Scr_Error("bad expression");
        break;
    }
}

void __cdecl Scr_EvalPrimitiveExpression(sval_u expr, unsigned int localId, VariableValue *value)
{
    VariableValue stringValue; // [esp+Ch] [ebp-14h] BYREF
    VariableValue objectValue; // [esp+14h] [ebp-Ch] BYREF
    unsigned int selfId; // [esp+1Ch] [ebp-4h]

    switch (expr.node[0].type)
    {
    case ENUM_integer:
        value->type = VAR_INTEGER;
        value->u.intValue = expr.node[1].intValue;
        break;
    case ENUM_float:
        value->type = VAR_FLOAT;
        value->u.floatValue = expr.node[1].floatValue;
        break;
    case ENUM_minus_integer:
        value->type = VAR_INTEGER;
        value->u.intValue = -expr.node[1].intValue;
        break;
    case ENUM_minus_float:
        value->type = VAR_FLOAT;
        value->u.floatValue = -expr.node[1].floatValue;
        break;
    case ENUM_string:
        value->type = VAR_STRING;
        value->u.stringValue = SL_GetString_(expr.node[1].debugString, 0, 20); // KISAKTODO is debugString the right union field????
        break;
    case ENUM_istring:
        value->type = VAR_ISTRING;
        value->u.stringValue = SL_GetString_(expr.node[1].debugString, 0, 20);
        break;
    case ENUM_variable:
        Scr_EvalVariableExpression(expr.node[1], localId, value);
        break;
    case ENUM_call_expression:
        Scr_EvalCallExpression(expr.node[1], localId, value);
        break;
    case ENUM_undefined:
        value->type = VAR_UNDEFINED;
        break;
    case ENUM_self:
        if (scrEvaluateGlob.freezeScope)
            localId = expr.node[1].idValue;
        if (localId && Scr_IsThreadAlive(localId))
        {
            selfId = Scr_GetSelf(localId);
            value->type = VAR_POINTER;
            value->u.intValue = selfId;
            AddRefToObject(value->u.intValue);
        }
        else
        {
            localId = 0;
            selfId = 0;
            value->type = VAR_UNDEFINED;
            Scr_Error("thread not active");
        }
        if (scrEvaluateGlob.freezeObjects)
        {
            if (expr.node[1].idValue != localId || expr.node[2].idValue != selfId)
                scrEvaluateGlob.objectChanged = 1;
        }
        else
        {
            expr.node[1].idValue = localId;
            expr.node[2].idValue = selfId;
        }
        break;
    case ENUM_self_frozen:
        iassert(expr.node[2].idValue);
        value->type = VAR_POINTER;
        value->u.intValue = expr.node[2].idValue;
        AddRefToObject(value->u.intValue);
        break;
    case ENUM_level:
        value->type = VAR_POINTER;
        value->u.intValue = scrVarPub.levelId;
        AddRefToObject(scrVarPub.levelId);
        break;
    case ENUM_game:
        *value = Scr_EvalVariable(scrVarPub.gameId);
        break;
    case ENUM_anim:
        value->type = VAR_POINTER;
        value->u.intValue = scrVarPub.animId;
        AddRefToObject(scrVarPub.animId);
        break;
    case ENUM_expression_list:
        Scr_EvalExpression(expr.node[1], localId, value);
        break;
    case ENUM_size_field:
        Scr_EvalPrimitiveExpression(expr.node[1], localId, value);
        Scr_EvalSizeValue(value);
        break;
    case ENUM_empty_array:
        value->type = VAR_UNDEFINED;
        Scr_Error("cannot evaluate []");
        break;
    case ENUM_false:
        value->type = VAR_INTEGER;
        value->u.intValue = 0;
        break;
    case ENUM_true:
        value->type = VAR_INTEGER;
        value->u.intValue = 1;
        break;
    case ENUM_breakon:
        Scr_EvalPrimitiveExpression(expr.node[1], localId, &objectValue);
        Scr_EvalExpression(expr.node[2], localId, &stringValue);
        if (objectValue.type == 1
            && stringValue.type == 2
            && g_breakonObject == objectValue.u.intValue
            && g_breakonString == stringValue.u.intValue)
        {
            g_breakonHit = 1;
            ++expr.node[3].intValue;
        }
        RemoveRefToValue(objectValue.type, objectValue.u);
        RemoveRefToValue(stringValue.type, stringValue.u);
        value->type = VAR_INTEGER;
        value->u.intValue = expr.node[3].intValue;
        break;
    case ENUM_bad_expression:
        value->type = VAR_UNDEFINED;
        Scr_Error("bad expression");
        break;
    default:
        return;
    }
}

void __cdecl Scr_EvalVariableExpression(sval_u expr, unsigned int localId, VariableValue *value)
{
    unsigned int objectId; // [esp+4h] [ebp-4h]
    unsigned int objectIda; // [esp+4h] [ebp-4h]
    unsigned int objectIdb; // [esp+4h] [ebp-4h]

    switch (*(unsigned int *)expr.type)
    {
    case 3:
        value->type = VAR_UNDEFINED;
        Scr_Error("unknown variable");
        break;
    case 4:
        if (scrEvaluateGlob.freezeScope)
            localId = expr.node[2].idValue;
        objectId = 0;
        if (localId && Scr_IsThreadAlive(localId))
        {
            Scr_EvalLocalVariable(expr.node[1], localId, value);
            if (!scrVarPub.error_message)
            {
                AddRefToValue(value->type, value->u);
                //objectId = Scr_EvalFieldObject(*(unsigned int *)(expr.type + 16), value).stringValue;
                objectId = Scr_EvalFieldObject(expr.node[4].idValue, value);
                Scr_ClearErrorMessage();
            }
        }
        else
        {
            localId = 0;
            value->type = VAR_UNDEFINED;
            Scr_Error("thread not active");
        }
        if (scrEvaluateGlob.freezeObjects)
        {
            if (expr.node[2].idValue != localId)
                scrEvaluateGlob.objectChanged = 1;
        }
        else
        {
            expr.node[2].idValue = localId;
        }
        if (scrEvaluateGlob.freezeObjects)
        {
            if (expr.node[3].idValue != objectId)
                scrEvaluateGlob.objectChanged = 1;
        }
        else
        {
            expr.node[3].idValue = objectId;
        }
        break;
    case 5:
        if (!expr.node[3].idValue)
            goto LABEL_29;
        value->type = VAR_POINTER;
        value->u.intValue = expr.node[3].idValue;
        AddRefToObject(value->u.intValue);
        break;
    case 0xD:
        Scr_EvalArrayVariableExpression(expr.node[1], expr.node[2], localId, value);
        break;
    case 0xE:
        value->type = VAR_UNDEFINED;
        Scr_Error("unknown field");
        break;
    case 0xF:
        objectIda = Scr_EvalPrimitiveExpressionFieldObject(expr.node[1], localId);
        if (scrEvaluateGlob.freezeObjects)
        {
            if (expr.node[3].idValue != objectIda)
                scrEvaluateGlob.objectChanged = 1;
        }
        else
        {
            expr.node[3].idValue = objectIda;
        }
        if (!objectIda)
            goto LABEL_29;
        Scr_EvalFieldVariableInternal(objectIda, expr.node[2].idValue, value);
        break;
    case 0x10:
        if (expr.node[3].idValue)
        {
            Scr_EvalFieldVariableInternal(expr.node[3].idValue, expr.node[2].idValue, value);
        }
        else
        {
        LABEL_29:
            value->type = VAR_UNDEFINED;
            Scr_Error("unknown object");
        }
        break;
    case 0x35:
        Scr_EvalPrimitiveExpression(expr.node[1], localId, value);
        Scr_EvalSelfValue(value);
        break;
    case 0x50:
        objectIdb = Scr_EvalObject(expr.node[1], expr.node[2], value).u.stringValue;
        if (scrEvaluateGlob.freezeObjects)
        {
            if (expr.node[3].idValue != objectIdb)
                scrEvaluateGlob.objectChanged = 1;
        }
        else
        {
            expr.node[3].idValue = objectIdb;
        }
        break;
    case 0x51:
        if (*(unsigned int *)(expr.type + 4) && Scr_IsThreadAlive(*(unsigned int *)(expr.type + 4)))
        {
            value->u.intValue = *(unsigned int *)(expr.type + 4);
            value->type = VAR_POINTER;
            AddRefToObject(value->u.intValue);
        }
        else
        {
            if (*(unsigned int *)(expr.type + 4))
            {
                RemoveRefToObject(*(unsigned int *)(expr.type + 4));
                *(unsigned int *)(expr.type + 4) = 0;
            }
            value->type = VAR_UNDEFINED;
            Scr_Error("thread not active");
        }
        break;
    case 0x54:
        value->type = VAR_UNDEFINED;
        Scr_Error("bad expression");
        break;
    case 0x57:
        Scr_GetValue(*(unsigned int *)(expr.type + 4), value);
        break;
    default:
        return;
    }
}

void __cdecl Scr_EvalArrayVariableExpression(sval_u array, sval_u index, unsigned int localId, VariableValue *value)
{
    VariableValue exprValue; // [esp+0h] [ebp-8h] BYREF

    Scr_EvalExpression(index, localId, value);
    Scr_EvalPrimitiveExpression(array, localId, &exprValue);
    Scr_EvalArrayVariableInternal(&exprValue, value);
}

void __cdecl Scr_EvalLocalVariable(sval_u expr, unsigned int localId, VariableValue *value)
{
    unsigned int varId; // [esp+8h] [ebp-4h]

    if (!expr.type)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 532, 0, "%s", "expr.stringValue");
    varId = FindVariable(localId, expr.stringValue);
    if (varId)
    {
        *value = Scr_EvalVariable(varId);
    }
    else
    {
        value->type = VAR_UNDEFINED;
        Scr_Error("unknown variable");
    }
}

VariableValueInternal_u __cdecl Scr_EvalObject(sval_u classnum, sval_u entnum, VariableValue *value)
{
    unsigned int objectId; // [esp+0h] [ebp-4h]

    objectId = FindEntityId(entnum.stringValue, classnum.stringValue);
    if (objectId)
    {
        value->type = VAR_POINTER;
        value->u.intValue = objectId;
        AddRefToObject(objectId);
        return (VariableValueInternal_u)objectId;
    }
    else
    {
        value->type = VAR_UNDEFINED;
        Scr_Error("unknown object");
        return 0;
    }
}

void __cdecl Scr_EvalSelfValue(VariableValue *value)
{
    const char *v1; // eax
    signed int ObjectType; // [esp+0h] [ebp-8h]
    VariableUnion threadId; // [esp+4h] [ebp-4h]

    if (value->type != 1)
        goto LABEL_8;
    threadId.intValue = value->u.intValue;
    ObjectType = GetObjectType(value->u.intValue);
    if (ObjectType < 14)
        goto LABEL_8;
    if (ObjectType <= 17)
    {
        value->type = VAR_POINTER;
        value->u.intValue = Scr_GetSelf(threadId.stringValue);
        AddRefToObject(value->u.intValue);
        RemoveRefToObject(threadId.stringValue);
        return;
    }
    if (ObjectType == 22)
    {
        Scr_Error("thread not active");
        Scr_ClearValue(value);
    }
    else
    {
    LABEL_8:
        v1 = va("self cannot be applied to %s", var_typename[value->type]);
        Scr_Error(v1);
        Scr_ClearValue(value);
    }
}

void __cdecl Scr_GetValue(unsigned int index, VariableValue *value)
{
    VariableValue *v2; // edx
    Vartype_t type; // ecx
    const char *v4; // eax

    if (index >= scrVmPub.breakpointOutparamcount)
    {
        value->type = VAR_UNDEFINED;
        v4 = va("parameter %d does not exist", index);
        Scr_Error(v4);
    }
    else
    {
        v2 = &scrVmPub.top[-(int)index];
        type = v2->type;
        value->u.intValue = v2->u.intValue;
        value->type = type;
        AddRefToValue(value->type, value->u);
    }
}

VariableValue* Scr_GetValue(unsigned int param)
{
	return &scrVmPub.top[-int(param)];
}

unsigned int __cdecl Scr_EvalPrimitiveExpressionFieldObject(sval_u expr, unsigned int localId)
{
    unsigned int result; // eax
    VariableValue value; // [esp+4h] [ebp-Ch] BYREF
    unsigned int selfId; // [esp+Ch] [ebp-4h]

    switch (*(unsigned int *)expr.type)
    {
    case 0x11:
        Scr_EvalVariableExpression(expr.node[1], localId, &value);
        //result = Scr_EvalFieldObject(*(unsigned int *)(expr.type + 8), &value).stringValue;
        result = Scr_EvalFieldObject(expr.node[2].idValue, &value);
        break;
    case 0x13:
        Scr_EvalCallExpression(expr.node[1], localId, &value);
        //result = Scr_EvalFieldObject(*(unsigned int *)(expr.type + 8), &value).stringValue;
        result = Scr_EvalFieldObject(expr.node[2].idValue, &value);
        break;
    case 0x20:
        if (scrEvaluateGlob.freezeScope)
            localId = expr.node[1].idValue;
        if (localId && Scr_IsThreadAlive(localId))
        {
            selfId = Scr_GetSelf(localId);
        }
        else
        {
            localId = 0;
            selfId = 0;
            Scr_Error("thread not active");
        }
        if (scrEvaluateGlob.freezeObjects)
        {
            if (expr.node[1].idValue != localId || expr.node[2].idValue != selfId)
                scrEvaluateGlob.objectChanged = 1;
        }
        else
        {
            expr.node[1].idValue = localId;
            expr.node[2].idValue = selfId;
        }
        result = selfId;
        break;
    case 0x21:
        if (!expr.node[2].idValue)
            MyAssertHandler(".\\script\\scr_evaluate.cpp", 1016, 0, "%s", "expr.node[2].idValue");
        result = expr.node[2].idValue;
        break;
    case 0x22:
        result = scrVarPub.levelId;
        break;
    case 0x24:
        result = scrVarPub.animId;
        break;
    default:
        Scr_Error("bad expression");
        result = 0;
        break;
    }
    return result;
}

void __cdecl Scr_EvalCallExpression(sval_u expr, unsigned int localId, VariableValue *value)
{
    if (*(unsigned int *)expr.type == 23)
    {
        Scr_EvalFunction(expr.node[1], expr.node[2], localId, value);
    }
    else if (*(unsigned int *)expr.type == 24)
    {
        Scr_EvalMethod(expr.node[1], expr.node[2], expr.node[3], localId, value);
    }
}

void __cdecl Scr_EvalFunction(sval_u func_name, sval_u params, unsigned int localId, VariableValue *value)
{
    Scr_PreEvalBuiltin(params, localId);
    if ((unsigned int)++g_script_error_level >= 0x21)
        MyAssertHandler(
            ".\\script\\scr_evaluate.cpp",
            1641,
            0,
            "g_script_error_level doesn't index ARRAY_COUNT( g_script_error )\n\t%i not in [0, %i)",
            g_script_error_level,
            33);
    //if (!_setjmp3(g_script_error[g_script_error_level], 0)) // KISAKTRYCATCH
    if (!_setjmp(g_script_error[g_script_error_level])) // KISAKTRYCATCH
        ((void (*)(void))func_name.type)();
    if (g_script_error_level < 0)
        MyAssertHandler(
            ".\\script\\scr_evaluate.cpp",
            1646,
            0,
            "%s\n\t(g_script_error_level) = %i",
            "(g_script_error_level >= 0)",
            g_script_error_level);
    --g_script_error_level;
    Scr_PostEvalBuiltin(value);
}

void __cdecl Scr_PreEvalBuiltin(sval_u params, unsigned int localId)
{
    sval_u *node; // [esp+0h] [ebp-Ch]
    unsigned int expr_count; // [esp+4h] [ebp-8h]
    int index; // [esp+8h] [ebp-4h]

    if (scrVmPub.outparamcount)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1574, 0, "%s", "!scrVmPub.outparamcount");
    if (scrVmPub.inparamcount)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1575, 0, "%s", "!scrVmPub.inparamcount");
    expr_count = GetExpressionCount(params);
    if (scrVmPub.top < scrVmPub.stack)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1578, 0, "%s", "scrVmPub.top >= scrVmPub.stack");
    if (scrVmPub.top > scrVmPub.maxstack)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1579, 0, "%s", "scrVmPub.top <= scrVmPub.maxstack");
    scrVmPub.top += expr_count;
    if (scrVmPub.top > scrVmPub.maxstack)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1581, 0, "%s", "scrVmPub.top <= scrVmPub.maxstack");
    index = 0;
    for (node = *(sval_u **)params.type; node; node = node[1].node)
        Scr_EvalExpression((sval_u)node->type, localId, &scrVmPub.top[-index++]);
    scrVmPub.outparamcount = expr_count;
    if (!scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1593, 0, "%s", "scrVarPub.evaluate");
    scrVarPub.evaluate = 0;
    if (scrVmPub.debugCode)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1596, 0, "%s", "!scrVmPub.debugCode");
    scrVmPub.debugCode = 1;
}

void __cdecl Scr_PostEvalBuiltin(VariableValue *value)
{
    Vartype_t type; // ecx

    if (!scrVmPub.debugCode)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1609, 0, "%s", "scrVmPub.debugCode");
    scrVmPub.debugCode = 0;
    if (scrVarPub.evaluate)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 1612, 0, "%s", "!scrVarPub.evaluate");
    scrVarPub.evaluate = 1;
    Scr_ClearOutParams();
    if (scrVmPub.inparamcount)
    {
        if (scrVmPub.inparamcount != 1)
            MyAssertHandler(".\\script\\scr_evaluate.cpp", 1619, 0, "%s", "scrVmPub.inparamcount == 1");
        scrVmPub.inparamcount = 0;
        type = scrVmPub.top->type;
        value->u.intValue = scrVmPub.top->u.intValue;
        value->type = type;
        --scrVmPub.top;
    }
    else
    {
        value->type = VAR_UNDEFINED;
    }
}

void __cdecl Scr_EvalMethod(sval_u expr, sval_u func_name, sval_u params, unsigned int localId, VariableValue *value)
{
    const char *v5; // eax
    const char *v6; // eax
    scr_entref_t entref; // [esp+4h] [ebp-14h]
    VariableUnion objectId; // [esp+8h] [ebp-10h]
    int type; // [esp+Ch] [ebp-Ch]
    unsigned int typea; // [esp+Ch] [ebp-Ch]
    VariableValue objectValue; // [esp+10h] [ebp-8h] BYREF

    Scr_EvalPrimitiveExpression(expr, localId, &objectValue);
    Scr_PreEvalBuiltin(params, localId);
    if ((unsigned int)++g_script_error_level >= 0x21)
        MyAssertHandler(
            ".\\script\\scr_evaluate.cpp",
            1671,
            0,
            "g_script_error_level doesn't index ARRAY_COUNT( g_script_error )\n\t%i not in [0, %i)",
            g_script_error_level,
            33);
    //if (!_setjmp3(g_script_error[g_script_error_level], 0)) // KISAKTRYCATCH
    if (!_setjmp(g_script_error[g_script_error_level])) // KISAKTRYCATCH
    {
        if (objectValue.type != 1)
        {
            type = objectValue.type;
            RemoveRefToValue(objectValue.type, objectValue.u);
            scrVarPub.error_index = -1;
            v5 = va("%s is not an entity", var_typename[type]);
            Scr_Error(v5);
        }
        objectId.intValue = objectValue.u.intValue;
        if (GetObjectType(objectValue.u.stringValue) != 20)
        {
            typea = GetObjectType(objectId.stringValue);
            RemoveRefToObject(objectId.stringValue);
            scrVarPub.error_index = -1;
            v6 = va("%s is not an entity", var_typename[typea]);
            Scr_Error(v6);
        }
        entref = Scr_GetEntityIdRef(objectId.stringValue);
        RemoveRefToObject(objectId.stringValue);
        ((void(__cdecl *)(unsigned int))func_name.type)(entref.entnum); // KISAKTODO: fubar'd union 'entref'
    }
    if (g_script_error_level < 0)
        MyAssertHandler(
            ".\\script\\scr_evaluate.cpp",
            1699,
            0,
            "%s\n\t(g_script_error_level) = %i",
            "(g_script_error_level >= 0)",
            g_script_error_level);
    --g_script_error_level;
    Scr_PostEvalBuiltin(value);
}

void __cdecl Scr_EvalBoolOrExpression(sval_u expr1, sval_u expr2, unsigned int localId, VariableValue *value)
{
    bool v4; // [esp+0h] [ebp-Ch]
    bool v5; // [esp+4h] [ebp-8h]

    Scr_EvalExpression(expr1, localId, value);
    Scr_CastBool(value);
    v5 = value->type == 6 && value->u.intValue;
    RemoveRefToValue(value->type, value->u);
    Scr_EvalExpression(expr2, localId, value);
    Scr_CastBool(value);
    v4 = v5 || value->type == 6 && value->u.intValue;
    RemoveRefToValue(value->type, value->u);
    value->type = VAR_INTEGER;
    value->u.intValue = v4;
}

void __cdecl Scr_EvalBoolAndExpression(sval_u expr1, sval_u expr2, unsigned int localId, VariableValue *value)
{
    bool v4; // [esp+0h] [ebp-Ch]
    bool v5; // [esp+4h] [ebp-8h]

    Scr_EvalExpression(expr1, localId, value);
    Scr_CastBool(value);
    v5 = value->type == 6 && value->u.intValue;
    RemoveRefToValue(value->type, value->u);
    Scr_EvalExpression(expr2, localId, value);
    Scr_CastBool(value);
    v4 = v5 && value->type == 6 && value->u.intValue;
    RemoveRefToValue(value->type, value->u);
    value->type = VAR_INTEGER;
    value->u.intValue = v4;
}

void __cdecl Scr_EvalBinaryOperatorExpression(
    sval_u expr1,
    sval_u expr2,
    sval_u opcode,
    unsigned int localId,
    VariableValue *value)
{
    VariableValue value2; // [esp+0h] [ebp-8h] BYREF

    Scr_EvalExpression(expr1, localId, value);
    Scr_EvalExpression(expr2, localId, &value2);
    Scr_EvalBinaryOperator(opcode.type, value, &value2);
}

void __cdecl Scr_EvalVector(sval_u expr1, sval_u expr2, sval_u expr3, unsigned int localId, VariableValue *value)
{
    VariableValue vectorValue[3]; // [esp+0h] [ebp-18h] BYREF

    Scr_EvalExpression(expr1, localId, vectorValue);
    Scr_EvalExpression(expr2, localId, &vectorValue[1]);
    Scr_EvalExpression(expr3, localId, &vectorValue[2]);
    Scr_CastVector(vectorValue);
    *value = vectorValue[0];
}

void __cdecl Scr_ClearDebugExprValue(sval_u val)
{
    switch (*(unsigned int *)val.type)
    {
    case 4:
    case 5:
        iassert(val.node[4].idValue);
        ClearVariableValue(val.node[4].idValue);
        break;
    case 0x11:
    case 0x13:
        iassert(val.node[2].idValue);
        ClearVariableValue(val.node[2].idValue);
        break;
    default:
        return;
    }
}

bool __cdecl Scr_RefScriptExpression(ScriptExpression_t *expr)
{
    bool result; // [esp+3h] [ebp-1h]

    if (!expr)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 2250, 0, "%s", "expr");
    result = Scr_RefExpression(expr->parseData);
    if (!expr->exprHead)
        MyAssertHandler(".\\script\\scr_evaluate.cpp", 2253, 0, "%s", "expr->exprHead");
    Scr_ClearDebugExpr(expr->exprHead);
    return result;
}

bool __cdecl Scr_RefExpression(sval_u expr)
{
    bool result; // al

    switch (*(unsigned int *)expr.type)
    {
    case 6:
        result = Scr_RefPrimitiveExpression(expr.node[1]);
        break;
    case 0x2F:
    case 0x30:
    case 0x31:
        result = Scr_RefBinaryOperatorExpression(expr.node[1], expr.node[2]);
        break;
    case 0x32:
    case 0x33:
        result = Scr_RefExpression(expr.node[1]);
        break;
    case 0x4F:
        result = Scr_RefVector(expr.node[1], expr.node[2], expr.node[3]);
        break;
    case 0x52:
        result = Scr_RefToVariable(expr.node[1].idValue, 1);
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

bool __cdecl Scr_RefPrimitiveExpression(sval_u expr)
{
    bool result; // al

    switch (*(unsigned int *)expr.type)
    {
    case 0x11:
        result = Scr_RefVariableExpression(expr.node[1]);
        break;
    case 0x13:
        result = Scr_RefCallExpression(expr.node[1]);
        break;
    case 0x20:
        if (!Scr_RefToVariable(expr.node[1].idValue, 1))
            goto $LN4_67;
        if (expr.node[2].idValue)
        {
            *(unsigned int *)expr.type = 33;
            goto $LN4_67;
        }
        expr.node[1].idValue = 0;
        result = 1;
        break;
    case 0x21:
    $LN4_67:
        result = Scr_RefToVariable(expr.node[2].idValue, 1);
        break;
    case 0x23:
        result = Scr_RefToVariable(scrVarPub.gameId, 0);
        break;
    case 0x2E:
        result = Scr_RefExpression(expr.node[1]);
        break;
    case 0x34:
        result = Scr_RefPrimitiveExpression(expr.node[1]);
        break;
    case 0x4B:
        result = Scr_RefBreakonExpression(expr.node[1], expr.node[2]);
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

bool __cdecl Scr_RefVariableExpression(sval_u expr)
{
    bool result; // al

    switch (*(unsigned int *)expr.type)
    {
    case 4:
        if (!Scr_RefToVariable(expr.node[2].idValue, 1))
            goto $LN11_33;
        if (expr.node[3].idValue)
        {
            *(unsigned int *)expr.type = 5;
            goto $LN11_33;
        }
        expr.node[2].idValue = 0;
        result = 1;
        break;
    case 5:
    $LN11_33:
        if (Scr_RefToVariable(expr.node[3].idValue, 1))
        {
            expr.node[3].idValue = 0;
            result = 1;
        }
        else
        {
            result = 0;
        }
        break;
    case 0xD:
        result = Scr_RefArrayVariableExpression(expr.node[1], expr.node[2]);
        break;
    case 0xF:
        if (!Scr_RefPrimitiveExpression(expr.node[1]))
            goto $LN5_66;
        if (expr.node[3].idValue)
        {
            *(unsigned int *)expr.type = 16;
            goto $LN5_66;
        }
        result = 1;
        break;
    case 0x10:
    $LN5_66:
        if (Scr_RefToVariable(expr.node[3].idValue, 1))
        {
            expr.node[3].idValue = 0;
            result = 1;
        }
        else
        {
            result = 0;
        }
        break;
    case 0x35:
        result = Scr_RefPrimitiveExpression(expr.node[1]);
        break;
    case 0x50:
        result = Scr_RefToVariable(expr.node[3].idValue, 1);
        break;
    case 0x51:
        result = Scr_RefToVariable(expr.node[1].idValue, 1);
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

bool __cdecl Scr_RefArrayVariableExpression(sval_u array, sval_u index)
{
    bool arrayRemoved; // [esp+6h] [ebp-2h]
    bool indexRemoved; // [esp+7h] [ebp-1h]

    indexRemoved = Scr_RefExpression(index);
    arrayRemoved = Scr_RefPrimitiveExpression(array);
    return indexRemoved || arrayRemoved;
}

bool __cdecl Scr_RefBreakonExpression(sval_u expr, sval_u param)
{
    bool expr1Removed; // [esp+6h] [ebp-2h]
    bool expr2Removed; // [esp+7h] [ebp-1h]

    expr1Removed = Scr_RefPrimitiveExpression(expr);
    expr2Removed = Scr_RefExpression(param);
    return expr1Removed || expr2Removed;
}

bool __cdecl Scr_RefCallExpression(sval_u expr)
{
    if (*(unsigned int *)expr.type == 23)
        return Scr_RefCall(expr.node[2]);
    if (*(unsigned int *)expr.type == 24)
        return Scr_RefMethod(expr.node[1], expr.node[3]);
    return 0;
}

bool __cdecl Scr_RefCall(sval_u params)
{
    sval_u *node; // [esp+0h] [ebp-8h]
    bool exprRemoved; // [esp+7h] [ebp-1h]

    exprRemoved = 0;
    for (node = *(sval_u **)params.type; node; node = node[1].node)
    {
        if (Scr_RefExpression((sval_u)node->type))
            exprRemoved = 1;
    }
    return exprRemoved;
}

bool __cdecl Scr_RefMethod(sval_u expr, sval_u params)
{
    bool expr1Removed; // [esp+6h] [ebp-2h]
    bool expr2Removed; // [esp+7h] [ebp-1h]

    expr1Removed = Scr_RefPrimitiveExpression(expr);
    expr2Removed = Scr_RefCall(params);
    return expr1Removed || expr2Removed;
}

bool __cdecl Scr_RefBinaryOperatorExpression(sval_u expr1, sval_u expr2)
{
    bool expr1Removed; // [esp+6h] [ebp-2h]
    bool expr2Removed; // [esp+7h] [ebp-1h]

    expr1Removed = Scr_RefExpression(expr1);
    expr2Removed = Scr_RefExpression(expr2);
    return expr1Removed || expr2Removed;
}

bool __cdecl Scr_RefVector(sval_u expr1, sval_u expr2, sval_u expr3)
{
    bool expr1Removed; // [esp+5h] [ebp-3h]
    bool expr2Removed; // [esp+6h] [ebp-2h]
    bool expr3Removed; // [esp+7h] [ebp-1h]

    expr1Removed = Scr_RefExpression(expr1);
    expr2Removed = Scr_RefExpression(expr2);
    expr3Removed = Scr_RefExpression(expr3);
    return expr1Removed || expr2Removed || expr3Removed;
}

void __cdecl Scr_FreeDebugExprValue(sval_u val)
{
    switch (*(unsigned int *)val.type)
    {
    case 4:
    case 5:
        iassert(val.node[4].idValue);
        FreeValue(val.node[4].idValue);
        break;
    case 0x11:
    case 0x13:
        iassert(val.node[2].idValue);
        FreeValue(val.node[2].idValue);
        break;
    case 0x51:
        if (val.node[1].idValue)
        {
            RemoveRefToObject(val.node[2].idValue);
            val.node[2].idValue = 0;
        }
        break;
    default:
        return;
    }
}

