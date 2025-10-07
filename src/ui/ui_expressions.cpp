#include <qcommon/qcommon.h>

#include "ui_shared.h"
#include <client/client.h>
#include <stringed/stringed_hooks.h>

#include <win32/win_storage.h>
#include <universal/profile.h>

#ifdef KISAK_MP
#include <cgame_mp/cg_local_mp.h>
#include <game_mp/g_main_mp.h>
#elif KISAK_SP
#include <cgame/cg_main.h>
#include "ui.h"
#include <game/g_local.h>
#endif

int s_operatorPrecedence[81] =
{
  2147483647,
  0,
  11,
  11,
  11,
  13,
  13,
  9,
  15,
  15,
  15,
  15,
  16,
  16,
  25,
  25,
  99,
  80,
  17,
  18,
  9,
  14,
  14,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5,
  5
}; // idb

const char *g_expOperatorNames[81] =
{
  "NOOP",
  ")",
  "*",
  "/",
  "%",
  "+",
  "-",
  "!",
  "<",
  "<=",
  ">",
  ">=",
  "==",
  "!=",
  "&&", // '&' IDA Betrayeth again
  "||",
  "(",
  ",",
  "&",
  "|",
  "~",
  "<<",
  ">>",
  "sin",
  "cos",
  "min",
  "max",
  "milliseconds",
  "dvarint",
  "dvarbool",
  "dvarfloat",
  "dvarstring",
  "stat",
  "ui_active",
  "flashbanged",
  "scoped",
  "scoreboard_visible",
  "inkillcam",
  "player",
  "selecting_location",
  "team",
  "otherteam",
  "marinesfield",
  "opforfield",
  "menuisopen",
  "writingdata",
  "inlobby",
  "inprivateparty",
  "privatepartyhost",
  "privatepartyhostinlobby",
  "aloneinparty",
  "adsjavelin",
  "weaplockblink",
  "weapattacktop",
  "weapattackdirect",
  "secondsastime",
  "tablelookup",
  "locstring",
  "localvarint",
  "localvarbool",
  "localvarfloat",
  "localvarstring",
  "timeleft",
  "secondsascountdown",
  "gamemsgwndactive",
  "int",
  "string",
  "float",
  "gametypename",
  "gametype",
  "gametypedescription",
  "scoreatrank",
  "friendsonline",
  "spectatingclient",
  "statrangeanybitsset",
  "keybinding",
  "actionslotusable",
  "hudfade",
  "maxrecommendedplayers",
  "acceptinginvite",
  "isintermission"
}; // idb

int currentTempOperand;
char s_tempOperandValueAsString[16][256];

struct ValidOperation // sizeof=0x10
{
    operationEnum op;
    expDataType leftSide;
    expDataType rightSide;
    void(__cdecl *function)(Operand *, Operand *, Operand *);
};

ValidOperation validOperations[84] =
{
  { OP_EQUALS, VAL_STRING, VAL_STRING, &compare_doesStringEqualString },
  { OP_NOTEQUAL, VAL_STRING, VAL_STRING, &compare_doesStringNotEqualString },
  { OP_EQUALS, VAL_INT, VAL_INT, &compare_doesIntEqualInt },
  { OP_EQUALS, VAL_INT, VAL_FLOAT, &compare_doesIntEqualFloat },
  { OP_EQUALS, VAL_FLOAT, VAL_INT, &compare_doesFloatEqualInt },
  { OP_EQUALS, VAL_FLOAT, VAL_FLOAT, &compare_doesFloatEqualFloat },
  { OP_NOTEQUAL, VAL_INT, VAL_INT, &compare_doesIntNotEqualInt },
  { OP_NOTEQUAL, VAL_INT, VAL_FLOAT, &compare_doesIntNotEqualFloat },
  { OP_NOTEQUAL, VAL_FLOAT, VAL_INT, &compare_doesFloatNotEqualInt },
  { OP_NOTEQUAL, VAL_FLOAT, VAL_FLOAT, &compare_doesFloatNotEqualFloat },
  { OP_LESSTHAN, VAL_INT, VAL_INT, &compare_isIntLessThanInt },
  { OP_LESSTHAN, VAL_INT, VAL_FLOAT, &compare_isIntLessThanFloat },
  { OP_LESSTHAN, VAL_FLOAT, VAL_INT, &compare_isFloatLessThanInt },
  { OP_LESSTHAN, VAL_FLOAT, VAL_FLOAT, &compare_isFloatLessThanFloat },
  { OP_LESSTHANEQUALTO, VAL_INT, VAL_INT, &compare_isIntLessThanEqualToInt },
  { OP_LESSTHANEQUALTO, VAL_INT, VAL_FLOAT, &compare_isIntLessThanEqualToFloat },
  { OP_LESSTHANEQUALTO, VAL_FLOAT, VAL_INT, &compare_isFloatLessThanEqualToInt },
  {
    OP_LESSTHANEQUALTO,
    VAL_FLOAT,
    VAL_FLOAT,
    &compare_isFloatLessThanEqualToFloat
  },
  { OP_GREATERTHAN, VAL_INT, VAL_INT, &compare_isIntGreaterThanInt },
  { OP_GREATERTHAN, VAL_INT, VAL_FLOAT, &compare_isIntGreaterThanFloat },
  { OP_GREATERTHAN, VAL_FLOAT, VAL_INT, &compare_isFloatGreaterThanInt },
  { OP_GREATERTHAN, VAL_FLOAT, VAL_FLOAT, &compare_isFloatGreaterThanFloat },
  {
    OP_GREATERTHANEQUALTO,
    VAL_INT,
    VAL_INT,
    &compare_isIntGreaterThanEqualToInt
  },
  {
    OP_GREATERTHANEQUALTO,
    VAL_INT,
    VAL_FLOAT,
    &compare_isIntGreaterThanEqualToFloat
  },
  {
    OP_GREATERTHANEQUALTO,
    VAL_FLOAT,
    VAL_INT,
    &compare_isFloatGreaterThanEqualToInt
  },
  {
    OP_GREATERTHANEQUALTO,
    VAL_FLOAT,
    VAL_FLOAT,
    &compare_isFloatGreaterThanEqualToFloat
  },
  { OP_ADD, VAL_INT, VAL_INT, &add_IntWithInt },
  { OP_ADD, VAL_INT, VAL_FLOAT, &add_IntWithFloat },
  { OP_ADD, VAL_FLOAT, VAL_INT, &add_FloatWithInt },
  { OP_ADD, VAL_STRING, VAL_STRING, &add_StringWithString },
  { OP_ADD, VAL_STRING, VAL_INT, &add_StringWithInt },
  { OP_ADD, VAL_INT, VAL_STRING, &add_IntWithString },
  { OP_ADD, VAL_STRING, VAL_FLOAT, &add_StringWithFloat },
  { OP_ADD, VAL_FLOAT, VAL_STRING, &add_FloatWithString },
  { OP_ADD, VAL_FLOAT, VAL_FLOAT, &add_FloatWithFloat },
  { OP_MULTIPLY, VAL_INT, VAL_INT, &multiply_IntByInt },
  { OP_MULTIPLY, VAL_INT, VAL_FLOAT, &multiply_IntByFloat },
  { OP_MULTIPLY, VAL_FLOAT, VAL_INT, &multiply_FloatByInt },
  { OP_MULTIPLY, VAL_FLOAT, VAL_FLOAT, &multiply_FloatByFloat },
  { OP_SUBTRACT, VAL_INT, VAL_INT, &subtract_IntFromInt },
  { OP_SUBTRACT, VAL_INT, VAL_FLOAT, &subtract_FloatFromInt },
  { OP_SUBTRACT, VAL_FLOAT, VAL_INT, &subtract_IntFromFloat },
  { OP_SUBTRACT, VAL_FLOAT, VAL_FLOAT, &subtract_FloatFromFloat },
  { OP_DIVIDE, VAL_INT, VAL_INT, &divide_IntByInt },
  { OP_DIVIDE, VAL_INT, VAL_FLOAT, &divide_IntByFloat },
  { OP_DIVIDE, VAL_FLOAT, VAL_INT, &divide_FloatByInt },
  { OP_DIVIDE, VAL_FLOAT, VAL_FLOAT, &divide_FloatByFloat },
  { OP_MODULUS, VAL_INT, VAL_INT, &mod_IntByInt },
  { OP_MODULUS, VAL_INT, VAL_FLOAT, &mod_IntByFloat },
  { OP_MODULUS, VAL_FLOAT, VAL_INT, &mod_FloatByInt },
  { OP_MODULUS, VAL_FLOAT, VAL_FLOAT, &mod_FloatByFloat },
  { OP_AND, VAL_INT, VAL_INT, &and_IntWithInt },
  { OP_AND, VAL_FLOAT, VAL_INT, &and_FloatWithInt },
  { OP_AND, VAL_INT, VAL_FLOAT, &and_IntWithFloat },
  { OP_AND, VAL_STRING, VAL_INT, &and_StringWithInt },
  { OP_AND, VAL_INT, VAL_STRING, &and_IntWithString },
  { OP_AND, VAL_STRING, VAL_FLOAT, &and_StringWithFloat },
  { OP_AND, VAL_FLOAT, VAL_STRING, &and_FloatWithString },
  { OP_AND, VAL_FLOAT, VAL_FLOAT, &and_FloatWithFloat },
  { OP_OR, VAL_INT, VAL_INT, &or_IntWithInt },
  { OP_OR, VAL_FLOAT, VAL_INT, &or_FloatWithInt },
  { OP_OR, VAL_INT, VAL_FLOAT, &or_IntWithFloat },
  { OP_OR, VAL_STRING, VAL_INT, &or_StringWithInt },
  { OP_OR, VAL_INT, VAL_STRING, &or_IntWithString },
  { OP_OR, VAL_STRING, VAL_FLOAT, &or_StringWithFloat },
  { OP_OR, VAL_FLOAT, VAL_STRING, &or_FloatWithString },
  { OP_OR, VAL_FLOAT, VAL_FLOAT, &or_FloatWithFloat },
  { OP_BITWISEAND, VAL_INT, VAL_INT, &bitwiseAnd },
  { OP_BITWISEAND, VAL_FLOAT, VAL_INT, &bitwiseAnd },
  { OP_BITWISEAND, VAL_INT, VAL_FLOAT, &bitwiseAnd },
  { OP_BITWISEAND, VAL_STRING, VAL_INT, &bitwiseAnd },
  { OP_BITWISEAND, VAL_INT, VAL_STRING, &bitwiseAnd },
  { OP_BITWISEAND, VAL_STRING, VAL_FLOAT, &bitwiseAnd },
  { OP_BITWISEAND, VAL_FLOAT, VAL_STRING, &bitwiseAnd },
  { OP_BITWISEAND, VAL_FLOAT, VAL_FLOAT, &bitwiseAnd },
  { OP_BITWISEOR, VAL_INT, VAL_INT, &bitwiseOr },
  { OP_BITWISEOR, VAL_FLOAT, VAL_INT, &bitwiseOr },
  { OP_BITWISEOR, VAL_INT, VAL_FLOAT, &bitwiseOr },
  { OP_BITWISEOR, VAL_STRING, VAL_INT, &bitwiseOr },
  { OP_BITWISEOR, VAL_INT, VAL_STRING, &bitwiseOr },
  { OP_BITWISEOR, VAL_STRING, VAL_FLOAT, &bitwiseOr },
  { OP_BITWISEOR, VAL_FLOAT, VAL_STRING, &bitwiseOr },
  { OP_BITWISEOR, VAL_FLOAT, VAL_FLOAT, &bitwiseOr },
  { OP_NOOP, VAL_INT, VAL_INT, NULL }
}; // idb

char *__cdecl GetSourceString(Operand operand)
{
    char *result; // [esp+8h] [ebp-4h]

    if (operand.dataType == VAL_STRING)
        return (char *)operand.internals.intVal;
    if ((unsigned int)currentTempOperand >= 0x10)
        MyAssertHandler(
            ".\\ui\\ui_expressions.cpp",
            182,
            0,
            "currentTempOperand doesn't index NUM_OPERAND_STRINGS\n\t%i not in [0, %i)",
            currentTempOperand,
            16);
    result = s_tempOperandValueAsString[currentTempOperand];
    currentTempOperand = (currentTempOperand + 1) % 16;
    if (operand.dataType)
    {
        if (operand.dataType == VAL_FLOAT)
            Com_sprintf(result, 0x100u, "%f", operand.internals.floatVal);
    }
    else
    {
        Com_sprintf(result, 0x100u, "%i", operand.internals.intVal);
    }
    return result;
}

double __cdecl GetSourceFloat(Operand *source)
{
    if (source->dataType == VAL_FLOAT)
        return source->internals.floatVal;
    if (source->dataType == VAL_INT)
        return (double)source->internals.intVal;
    if (source->dataType != VAL_STRING)
        MyAssertHandler(".\\ui\\ui_expressions.cpp", 211, 0, "%s", "source->dataType == VAL_STRING");
    return (float)atof(source->internals.string);
}

operandInternalDataUnion __cdecl GetSourceInt(Operand *source)
{
    if (source->dataType == VAL_FLOAT)
    {
        return (operandInternalDataUnion)(int)source->internals.floatVal;
    }
    else if (source->dataType)
    {
        if (source->dataType != VAL_STRING)
            MyAssertHandler(".\\ui\\ui_expressions.cpp", 222, 0, "%s", "source->dataType == VAL_STRING");
        return (operandInternalDataUnion)atoi(source->internals.string);
    }
    else
    {
        return source->internals;
    }
}

void(__cdecl *__cdecl GetOperationFunction(
    operationEnum op,
    Operand *data1,
    Operand *data2))(Operand *, Operand *, Operand *)
{
    double v4; // [esp+0h] [ebp-2Ch]
    double v5; // [esp+Ch] [ebp-20h]
    Operand temp2; // [esp+18h] [ebp-14h]
    int opNum; // [esp+20h] [ebp-Ch]
    int opNuma; // [esp+20h] [ebp-Ch]
    Operand temp; // [esp+24h] [ebp-8h]

    for (opNum = 0; validOperations[opNum].op; ++opNum)
    {
        if (validOperations[opNum].op == op
            && validOperations[opNum].leftSide == data1->dataType
            && validOperations[opNum].rightSide == data2->dataType)
        {
            return validOperations[opNum].function;
        }
    }
    if (data1->dataType == VAL_STRING)
    {
        v5 = (double)atoi(data1->internals.string);
        if (v5 == atof(data1->internals.string))
        {
            temp.dataType = VAL_INT;
            temp.internals.intVal = atoi(data1->internals.string);
        }
        else
        {
            temp.dataType = VAL_FLOAT;
            temp.internals.floatVal = atof(data1->internals.string);
        }
        *data1 = temp;
    }
    if (data2->dataType == VAL_STRING)
    {
        v4 = (double)atoi(data2->internals.string);
        if (v4 == atof(data2->internals.string))
        {
            temp2.dataType = VAL_INT;
            temp2.internals.intVal = atoi(data2->internals.string);
        }
        else
        {
            temp2.dataType = VAL_FLOAT;
            temp2.internals.floatVal = atof(data2->internals.string);
        }
        *data2 = temp2;
    }
    for (opNuma = 0; validOperations[opNuma].op; ++opNuma)
    {
        if (validOperations[opNuma].op == op
            && validOperations[opNuma].leftSide == data1->dataType
            && validOperations[opNuma].rightSide == data2->dataType)
        {
            return validOperations[opNuma].function;
        }
    }
    return 0;
}

bool __cdecl OpPairsWithRightParen(operationEnum op)
{
    return op >= OP_SIN && op <= NUM_OPERATORS || op == OP_LEFTPAREN;
}

void __cdecl RunLogicOp(
    int localClientNum,
    operationEnum op,
    OperandStack *dataStack,
    Operand data1,
    Operand data2,
    const char *opDescription)
{
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    char *v9; // [esp-Ch] [ebp-18h]
    char *v10; // [esp-Ch] [ebp-18h]
    const char *NameForValueType; // [esp-8h] [ebp-14h]
    const char *v12; // [esp-8h] [ebp-14h]
    char *SourceString; // [esp-4h] [ebp-10h]
    char *v14; // [esp-4h] [ebp-10h]
    char *v15; // [esp-4h] [ebp-10h]
    Operand operandResult; // [esp+0h] [ebp-Ch] BYREF
    void(__cdecl * function)(Operand *, Operand *, Operand *); // [esp+8h] [ebp-4h]

    if (uiscript_debug->current.integer)
    {
        SourceString = GetSourceString(data2);
        NameForValueType = GetNameForValueType(data2.dataType);
        v9 = GetSourceString(data1);
        v6 = GetNameForValueType(data1.dataType);
        Com_Printf(
            13,
            "running %s on a %s (%s) and a %s (%s)\n",
            g_expOperatorNames[op],
            v6,
            v9,
            NameForValueType,
            SourceString);
    }
    function = GetOperationFunction(op, &data1, &data2);
    if (function)
    {
        function(&data1, &data2, &operandResult);
    }
    else
    {
        v14 = GetSourceString(data2);
        v12 = GetNameForValueType(data2.dataType);
        v10 = GetSourceString(data1);
        v7 = GetNameForValueType(data1.dataType);
        Com_PrintError(13, "Error: You cannot %s a '%s' (%s) and a '%s' (%s)\n", opDescription, v7, v10, v12, v14);
        operandResult.dataType = VAL_INT;
        operandResult.internals.intVal = 0;
    }
    AddOperandToStack(dataStack, &operandResult);
    if (uiscript_debug->current.integer)
    {
        v15 = GetSourceString(operandResult);
        v8 = GetNameForValueType(operandResult.dataType);
        Com_Printf(13, "result is a %s (%s)\n", v8, v15);
    }
}

const char *__cdecl GetNameForValueType(expDataType valType)
{
    const char *v2; // eax

    switch (valType)
    {
    case VAL_INT:
        return "Int";
    case VAL_FLOAT:
        return "Float";
    case VAL_STRING:
        return "String";
    }
    if (!alwaysfails)
    {
        v2 = va("Unknown value type %i", valType);
        MyAssertHandler(".\\ui\\ui_expressions.cpp", 237, 0, v2);
    }
    return "";
}

void __cdecl AddOperandToStack(OperandStack *dataStack, Operand *data)
{
    operandInternalDataUnion v2; // ecx
    int numOperandLists; // edx

    if (data->dataType == VAL_STRING && !data->internals.intVal)
        MyAssertHandler(
            ".\\ui\\ui_expressions.cpp",
            1605,
            0,
            "%s",
            "data->dataType != VAL_STRING || data->internals.string");
    v2.intVal = (int)data->internals;
    numOperandLists = dataStack->numOperandLists;
    dataStack->stack[numOperandLists].operands[0].dataType = data->dataType;
    dataStack->stack[numOperandLists].operands[0].internals = v2;
    dataStack->stack[dataStack->numOperandLists++].operandCount = 1;
}

char __cdecl GetOperand(OperandStack *dataStack, Operand *data)
{
    operandInternalDataUnion v2; // edx
    operandInternalDataUnion v4; // eax
    OperandList *list; // [esp+0h] [ebp-4h]

    if (dataStack->numOperandLists >= 1)
    {
        list = &dataStack->stack[dataStack->numOperandLists - 1];
        if (list->operandCount == 1)
        {
            v4.intVal = (int)list->operands[0].internals;
            data->dataType = list->operands[0].dataType;
            data->internals = v4;
            --dataStack->numOperandLists;
            if (data->dataType == VAL_STRING && !data->internals.intVal)
                MyAssertHandler(
                    ".\\ui\\ui_expressions.cpp",
                    1796,
                    0,
                    "%s",
                    "data->dataType != VAL_STRING || data->internals.string");
            return 1;
        }
        else
        {
            Com_PrintError(
                13,
                "Error: Invalid operand count - expected to find one operand but instead found %i\n",
                list->operandCount);
            data->dataType = VAL_INT;
            data->internals.intVal = 0;
            return 0;
        }
    }
    else
    {
        Com_PrintError(13, "Error: Invalid operation - missing parameter inside function or parenthesis\n");
        dataStack->numOperandLists = 1;
        dataStack->stack[0].operandCount = 1;
        v2.intVal = (int)dataStack->stack[0].operands[0].internals;
        data->dataType = dataStack->stack[0].operands[0].dataType;
        data->internals = v2;
        data->dataType = VAL_INT;
        data->internals.intVal = 0;
        return 0;
    }
}

char __cdecl GetTwoOperands(OperandStack *dataStack, Operand *data1, Operand *data2)
{
    GetOperand(dataStack, data2);
    GetOperand(dataStack, data1);
    return 1;
}

char __cdecl GetOperandList(OperandStack *dataStack, OperandList *list)
{
    if (dataStack->numOperandLists >= 1)
    {
        memcpy(list, &dataStack->stack[--dataStack->numOperandLists], sizeof(OperandList));
        return 1;
    }
    else
    {
        Com_PrintError(13, "Error: Invalid operation - missing parameter inside function or parenthesis\n");
        dataStack->numOperandLists = 1;
        memcpy(list, dataStack, sizeof(OperandList));
        list->operandCount = 1;
        list->operands[0].dataType = VAL_INT;
        list->operands[0].internals.intVal = 0;
        return 0;
    }
}

#ifdef KISAK_MP
BOOL __cdecl CG_IsIntermission(int localClientNum)
{
    return CG_GetLocalClientGlobals(localClientNum)->nextSnap->ps.pm_type == PM_INTERMISSION;
}

void __cdecl GetIsIntermission(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = CG_IsIntermission(localClientNum);
    if (uiscript_debug->current.integer)
        Com_Printf(13, "isIntermission() = %i\n", result->internals.intVal);
}
#endif


void __cdecl RunOp(int localClientNum, OperatorStack *opStack, OperandStack *dataStack)
{
    operandInternalDataUnion v3; // eax
    const char *v4; // eax
    char localVarStrings[4][32]; // [esp+4h] [ebp-160h] BYREF
    OperandList list2; // [esp+8Ch] [ebp-D8h] BYREF
    operationEnum op; // [esp+E8h] [ebp-7Ch]
    OperandList list1; // [esp+ECh] [ebp-78h] BYREF
    Operand data2; // [esp+148h] [ebp-1Ch] BYREF
    Operand operandResult; // [esp+150h] [ebp-14h] BYREF
    int localVarStringIndex; // [esp+158h] [ebp-Ch]
    Operand data1; // [esp+15Ch] [ebp-8h] BYREF

    localVarStringIndex = 0;
    if (opStack->numOperators <= 0)
        MyAssertHandler(
            ".\\ui\\ui_expressions.cpp",
            1972,
            0,
            "%s\n\t(opStack->numOperators) = %i",
            "(opStack->numOperators > 0)",
            opStack->numOperators);
    op = opStack->stack[--opStack->numOperators];
    opStack->stack[opStack->numOperators] = OP_NOOP;
    if (uiscript_debug->current.integer > 1)
        Com_Printf(13, "evaluating %s\n", g_expOperatorNames[op]);
    switch (op)
    {
    case OP_NOOP:
        if (!alwaysfails)
            MyAssertHandler(".\\ui\\ui_expressions.cpp", 1985, 0, "Invalid operator NOOP made it into the stack!\n");
        return;
    case OP_RIGHTPAREN:
        op = OP_NOOP;
        do
        {
            if (!opStack->numOperators)
                break;
            op = opStack->stack[opStack->numOperators - 1];
            RunOp(localClientNum, opStack, dataStack);
        } while (!OpPairsWithRightParen(op));
        if (!OpPairsWithRightParen(op))
            Com_PrintError(13, "Error: found ')' but couldn't find what it was closing\n");
        return;
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_MODULUS:
    case OP_ADD:
    case OP_LESSTHAN:
    case OP_LESSTHANEQUALTO:
    case OP_GREATERTHAN:
    case OP_GREATERTHANEQUALTO:
    case OP_EQUALS:
    case OP_NOTEQUAL:
    case OP_AND:
    case OP_OR:
    case OP_BITWISEAND:
    case OP_BITWISEOR:
        if (GetTwoOperands(dataStack, &data1, &data2))
            RunLogicOp(localClientNum, op, dataStack, data1, data2, g_expOperatorNames[op]);
        return;
    case OP_SUBTRACT:
        if (!GetOperand(dataStack, &data2))
            return;
        if (dataStack->numOperandLists >= 1)
        {
            GetOperand(dataStack, &data1);
            RunLogicOp(localClientNum, op, dataStack, data1, data2, g_expOperatorNames[op]);
        }
        else
        {
            if (data2.dataType)
            {
                if (data2.dataType != VAL_FLOAT)
                {
                    Com_PrintError(13, "Error: trying to negate a string: %s\n", data2.internals.string);
                    return;
                }
                data2.internals.floatVal = -data2.internals.floatVal;
            }
            else
            {
                data2.internals.intVal = -data2.internals.intVal;
            }
            AddOperandToStack(dataStack, &data2);
        }
        return;
    case OP_NOT:
        GetOperand(dataStack, &data1);
        LogicalNot(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_LEFTPAREN:
        return;
    case OP_COMMA:
        if (GetOperandList(dataStack, &list2) && GetOperandList(dataStack, &list1))
            RunCommaOp(localClientNum, dataStack, &list1, &list2);
        return;
    case OP_BITWISENOT:
        GetOperand(dataStack, &data1);
        BitwiseNot(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_BITSHIFTLEFT:
        if (GetTwoOperands(dataStack, &data1, &data2))
        {
            BitShiftLeft(localClientNum, &data1, &data2, &operandResult);
            AddOperandToStack(dataStack, &operandResult);
        }
        return;
    case OP_BITSHIFTRIGHT:
        if (GetTwoOperands(dataStack, &data1, &data2))
        {
            BitShiftRight(localClientNum, &data1, &data2, &operandResult);
            AddOperandToStack(dataStack, &operandResult);
        }
        return;
    case OP_SIN:
        GetOperand(dataStack, &data1);
        GetSinValue(&data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_COS:
        GetOperand(dataStack, &data1);
        GetCosValue(&data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_MIN:
        GetOperandList(dataStack, &list1);
        MinValue(&list1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_MAX:
        GetOperandList(dataStack, &list1);
        MaxValue(&list1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_MILLISECONDS:
        GetMilliseconds(&operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_DVARINT:
        GetOperand(dataStack, &data1);
        GetDvarIntValue(&data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_DVARBOOL:
        GetOperand(dataStack, &data1);
        GetDvarBoolValue(&data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_DVARFLOAT:
        GetOperand(dataStack, &data1);
        GetDvarFloatValue(&data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_DVARSTRING:
        GetOperand(dataStack, &data1);
        GetDvarStringValue(&data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_STAT:
        GetOperand(dataStack, &data1);
        GetPlayerStat(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_UIACTIVE:
        GetUIActive(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_FLASHBANGED:
        GetFlashbanged(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_SCOPED:
        GetScoped(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_SCOREBOARDVISIBLE:
        GetScoreboardVisible(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_INKILLCAM:
        InKillcam(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_PLAYERFIELD:
        GetOperand(dataStack, &data1);
        GetPlayerField(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_SELECTINGLOCATION:
        GetSelectingLocation(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_TEAMFIELD:
        GetOperand(dataStack, &data1);
        GetTeamField(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_OTHERTEAMFIELD:
        GetOperand(dataStack, &data1);
        GetOtherTeamField(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_MARINESFIELD:
        GetOperand(dataStack, &data1);
        GetTeamMarinesField(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_OPFORFIELD:
        GetOperand(dataStack, &data1);
        GetTeamOpForField(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_MENUISOPEN:
        GetOperand(dataStack, &data1);
        IsMenuOpen(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_WRITINGDATA:
        WritingData(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_INLOBBY:
        InLobby(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_INPRIVATEPARTY:
        InPrivateParty(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_PRIVATEPARTYHOST:
        PrivatePartyHost(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_PRIVATEPARTYHOSTINLOBBY:
        PrivatePartyHostInLobby(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_ALONEINPARTY:
        AloneInPrivateParty(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_ADSJAVELIN:
        GetAdsJavelin(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_WEAPLOCKBLINK:
        GetOperand(dataStack, &data1);
        GetWeapLockBlink(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_WEAPATTACKTOP:
        GetWeapAttackTop(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_WEAPATTACKDIRECT:
        GetWeapAttackDirect(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_SECONDSASTIME:
        GetOperand(dataStack, &data1);
        SecondsToTimeDisplay(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_TABLELOOKUP:
        GetOperandList(dataStack, &list1);
        TableLookup(localClientNum, &list1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_LOCALIZESTRING:
        GetOperandList(dataStack, &list1);
        LocalizeString(&list1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_LOCALVARINT:
        GetOperand(dataStack, &data1);
        GetLocalVarIntValue(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_LOCALVARBOOL:
        GetOperand(dataStack, &data1);
        GetLocalVarBoolValue(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_LOCALVARFLOAT:
        GetOperand(dataStack, &data1);
        GetLocalVarFloatValue(localClientNum , &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_LOCALVARSTRING:
        GetOperand(dataStack, &data1);
        GetLocalVarStringValue(localClientNum, &data1, &operandResult, localVarStrings[localVarStringIndex], 0x20u);
        localVarStringIndex = ((_BYTE)localVarStringIndex + 1) & 4;
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_TIMELEFT:
        GetTimeLeft(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_SECONDSASCOUNTDOWN:
        GetOperand(dataStack, &data1);
        v3.intVal = GetSourceInt(&data1).intVal;
        SecondsToCountdownDisplay(localClientNum, v3.intVal, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_GAMEMSGWNDACTIVE:
        GetOperand(dataStack, &data1);
        GetGameMessageWindowActive(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_TOINT:
        GetOperand(dataStack, &data1);
        operandResult.dataType = VAL_INT;
        operandResult.internals = GetSourceInt(&data1);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_TOSTRING:
        GetOperand(dataStack, &data1);
        operandResult.dataType = VAL_STRING;
        operandResult.internals.intVal = (int)GetSourceString(data1);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_TOFLOAT:
        GetOperand(dataStack, &data1);
        operandResult.dataType = VAL_FLOAT;
        operandResult.internals.floatVal = GetSourceFloat(&data1);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_GAMETYPENAME:
        GetGametypeName(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_GAMETYPE:
        GetGametypeInternal(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_GAMETYPEDESCRIPTION:
        GetGametypeObjective(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_SCORE:
        GetOperand(dataStack, &data1);
        GetScore(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_FRIENDSONLINE:
    case OP_MAXPLAYERS:
    case OP_ACCEPTINGINVITE:
        UI_GetOnlineFriendCount(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_FOLLOWING:
        GetFollowing(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_STATRANGEBITSSET:
        GetOperandList(dataStack, &list1);
        GetPlayerStatRangeBitsSet(localClientNum, &list1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_KEYBINDING:
        GetOperand(dataStack, &data1);
        GetKeyBinding(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_ACTIONSLOTUSABLE:
        GetOperand(dataStack, &data1);
        GetActionSlotUsable(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
    case OP_HUDFADE:
        GetOperand(dataStack, &data1);
        GetHudFade(localClientNum, &data1, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
#ifdef KISAK_MP
    case OP_ISINTERMISSION:
        GetIsIntermission(localClientNum, &operandResult);
        AddOperandToStack(dataStack, &operandResult);
        return;
#endif
    default:
        if (!alwaysfails)
        {
            v4 = va("Unhandled operator %i in RunOp!\n", op);
            MyAssertHandler(".\\ui\\ui_expressions.cpp", 2343, 0, v4);
        }
        return;
    }
}

void __cdecl GetDvarStringValue(Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    char *VariantString; // eax
    const dvar_s *dvar; // [esp+0h] [ebp-4h]

    if (source->dataType == VAL_STRING)
    {
        result->dataType = VAL_STRING;
        dvar = Dvar_FindVar(source->internals.string);
        if (dvar)
        {
            if (dvar->type == 7)
                VariantString = CopyDvarString(dvar->current.string);
            else
                VariantString = (char *)Dvar_GetVariantString(source->internals.string);
            result->internals.intVal = (int)VariantString;
        }
        else
        {
            result->internals.intVal = (int)"";
        }
        if (uiscript_debug->current.integer)
            Com_Printf(13, "dvarstring( %s ) = %s\n", source->internals.string, result->internals.string);
    }
    else
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: Must use a string as the name of a dvar, not a %s\n", NameForValueType);
        result->dataType = VAL_STRING;
        result->internals.intVal = (int)"";
    }
}

char *__cdecl CopyDvarString(const char *string)
{
    char *result; // [esp+0h] [ebp-4h]

    if ((unsigned int)currentTempOperand >= 0x10)
        MyAssertHandler(
            ".\\ui\\ui_expressions.cpp",
            197,
            0,
            "currentTempOperand doesn't index NUM_OPERAND_STRINGS\n\t%i not in [0, %i)",
            currentTempOperand,
            16);
    result = s_tempOperandValueAsString[currentTempOperand];
    currentTempOperand = (currentTempOperand + 1) % 16;
    Com_sprintf(result, 0x100u, "%s", string);
    return result;
}

void __cdecl GetDvarBoolValue(Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    const char *VariantString; // eax

    if (source->dataType == VAL_STRING)
    {
        result->dataType = VAL_INT;
        VariantString = Dvar_GetVariantString(source->internals.string);
        result->internals.intVal = atoi(VariantString);
        if (uiscript_debug->current.integer)
            Com_Printf(13, "dvarbool( %s ) = %i\n", source->internals.string, result->internals.intVal);
    }
    else
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: Must use a string as the name of a dvar, not a %s\n", NameForValueType);
        result->dataType = VAL_INT;
        result->internals.intVal = 0;
    }
}

void __cdecl GetDvarIntValue(Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    const char *VariantString; // eax

    if (source->dataType == VAL_STRING)
    {
        result->dataType = VAL_INT;
        VariantString = Dvar_GetVariantString(source->internals.string);
        result->internals.intVal = atoi(VariantString);
        if (uiscript_debug->current.integer)
            Com_Printf(13, "dvarint( %s ) = %i\n", source->internals.string, result->internals.intVal);
    }
    else
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: Must use a string as the name of a dvar, not a %s\n", NameForValueType);
        result->dataType = VAL_INT;
        result->internals.intVal = 0;
    }
}

void __cdecl GetDvarFloatValue(Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    const char *VariantString; // eax

    if (source->dataType == VAL_STRING)
    {
        result->dataType = VAL_FLOAT;
        VariantString = Dvar_GetVariantString(source->internals.string);
        result->internals.floatVal = atof(VariantString);
        if (uiscript_debug->current.integer)
            Com_Printf(13, "dvarfloat( %s ) = %f\n", source->internals.string, result->internals.floatVal);
    }
    else
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: Must use a string as the name of a dvar, not a %s\n", NameForValueType);
        result->dataType = VAL_FLOAT;
        result->internals.floatVal = 0.0;
    }
}

void __cdecl GetLocalVarStringValue(
    int localClientNum,
    Operand *source,
    Operand *result,
    char *stringBuf,
    unsigned int size)
{
    UILocalVarContext *var; // [esp+0h] [ebp-4h]

    var = GetLocalVar(localClientNum, source);
    if (var)
    {
        result->dataType = VAL_STRING;
        result->internals.string = UILocalVar_GetString(var->table, stringBuf, size);
        if (uiscript_debug->current.integer)
            Com_Printf(13, "localVarString( %s ) = %s\n", source->internals.string, result->internals.string);
    }
    else
    {
        result->dataType = VAL_STRING;
        result->internals.string = "";
    }
}

UILocalVarContext *__cdecl GetLocalVar(int localClientNum, Operand *source)
{
    const char *NameForValueType; // eax
    UILocalVarContext *LocalVarsContext; // eax
    const char *string; // [esp-4h] [ebp-4h]

    if (source->dataType == VAL_STRING)
    {
        string = source->internals.string;
        LocalVarsContext = UI_GetLocalVarsContext(localClientNum);
        return UILocalVar_Find(LocalVarsContext, string);
    }
    else
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: Must use a string as the name of a localVar, not a %s\n", NameForValueType);
        return 0;
    }
}

void __cdecl GetLocalVarBoolValue(int localClientNum, Operand *source, Operand *result)
{
    UILocalVarContext *var; // [esp+0h] [ebp-4h]

    var = GetLocalVar(localClientNum, source);
    if (var)
    {
        result->dataType = VAL_INT;
        result->internals.intVal = UILocalVar_GetBool(var->table);
        if (uiscript_debug->current.integer)
            Com_Printf(13, "localVarBool( %s ) = %i\n", source->internals.string, result->internals.intVal);
    }
    else
    {
        result->dataType = VAL_INT;
        result->internals.intVal = 0;
    }
}

void __cdecl GetLocalVarIntValue(int localClientNum, Operand *source, Operand *result)
{
    UILocalVarContext *var; // [esp+0h] [ebp-4h]

    var = GetLocalVar(localClientNum, source);
    if (var)
    {
        result->dataType = VAL_INT;
        result->internals.intVal = UILocalVar_GetInt(var->table).integer;
        if (uiscript_debug->current.integer)
            Com_Printf(13, "localVarInt( %s ) = %i\n", source->internals.string, result->internals.intVal);
    }
    else
    {
        result->dataType = VAL_INT;
        result->internals.intVal = 0;
    }
}

void __cdecl GetLocalVarFloatValue(int localClientNum, Operand *source, Operand *result)
{
    UILocalVarContext *var; // [esp+8h] [ebp-4h]

    var = GetLocalVar(localClientNum, source);
    if (var)
    {
        result->dataType = VAL_FLOAT;
        result->internals.floatVal = UILocalVar_GetFloat(var->table);
        if (uiscript_debug->current.integer)
            Com_Printf(13, "localVarFloat( %s ) = %f\n", source->internals.string, result->internals.floatVal);
    }
    else
    {
        result->dataType = VAL_FLOAT;
        result->internals.floatVal = 0.0;
    }
}

void __cdecl GetSinValue(Operand *source, Operand *result)
{
    float v2; // [esp+10h] [ebp-8h]
    float val; // [esp+14h] [ebp-4h]

    val = GetSourceFloat(source);
    result->dataType = VAL_FLOAT;
    v2 = sin(val);
    result->internals.floatVal = v2;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "sin( %f ) = %f\n", val, result->internals.floatVal);
}

void __cdecl GetCosValue(Operand *source, Operand *result)
{
    float v2; // [esp+10h] [ebp-8h]
    float val; // [esp+14h] [ebp-4h]

    val = GetSourceFloat(source);
    result->dataType = VAL_FLOAT;
    v2 = cos(val);
    result->internals.floatVal = v2;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "cos( %f ) = %f\n", val, result->internals.floatVal);
}

void __cdecl GetMilliseconds(Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = Sys_Milliseconds();
}

void __cdecl GetPlayerField(int localClientNum, Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    operandInternalDataUnion *OurClientScore; // [esp+0h] [ebp-10h]
    operandInternalDataUnion *v5; // [esp+4h] [ebp-Ch]
    operandInternalDataUnion *v6; // [esp+8h] [ebp-8h]
#ifdef KISAK_MP
    const score_t *score; // [esp+Ch] [ebp-4h]
#endif

    if (source->dataType == VAL_STRING)
    {
        if (I_stricmp(source->internals.string, "teamname"))
        {
            if (I_stricmp(source->internals.string, "otherteamname"))
            {
                if (I_stricmp(source->internals.string, "dead"))
                {
                    if (I_stricmp(source->internals.string, "clipAmmo"))
                    {
                        if (I_stricmp(source->internals.string, "nightvision"))
                        {
                            if (I_stricmp(source->internals.string, "score"))
                            {
                                if (I_stricmp(source->internals.string, "deaths"))
                                {
                                    if (I_stricmp(source->internals.string, "kills"))
                                    {
                                        if (I_stricmp(source->internals.string, "ping"))
                                        {
                                            Com_Printf(13, "ERROR: Unknown player field '%s'\n", source->internals.string);
                                            result->dataType = VAL_INT;
                                            result->internals.intVal = 0;
                                        }
                                        else
                                        {
#ifdef KISAK_MP
                                            OurClientScore = (operandInternalDataUnion *)UI_GetOurClientScore(localClientNum);
                                            if (OurClientScore)
                                                result->internals = OurClientScore[2];
                                            else
#endif
                                            {
                                                result->internals.intVal = 0;
                                            }
                                            result->dataType = VAL_INT;
                                            if (uiscript_debug->current.integer)
                                                Com_Printf(13, "player( %s ) = %i\n", source->internals.string, result->internals.intVal);
                                        }
                                    }
                                    else
                                    {
#ifdef KISAK_MP
                                        v5 = (operandInternalDataUnion *)UI_GetOurClientScore(localClientNum);
                                        if (v5)
                                        {
                                            result->internals = v5[5];
                                        }
                                        else
#endif
                                        {
                                            result->internals.intVal = 0;
                                        }
                                        result->dataType = VAL_INT;
                                        if (uiscript_debug->current.integer)
                                            Com_Printf(13, "player( %s ) = %i\n", source->internals.string, result->internals.intVal);
                                    }
                                }
                                else
                                {
#ifdef KISAK_MP
                                    v6 = (operandInternalDataUnion *)UI_GetOurClientScore(localClientNum);
                                    if (v6)
                                    {
                                        result->internals = v6[3];
                                    }
                                    else
#endif
                                    {
                                        result->internals.intVal = 0;
                                    }
                                    result->dataType = VAL_INT;
                                    if (uiscript_debug->current.integer)
                                        Com_Printf(13, "player( %s ) = %i\n", source->internals.string, result->internals.intVal);
                                }
                            }
                            else
                            {
#ifdef KISAK_MP
                                score = UI_GetOurClientScore(localClientNum);
                                if (score)
                                {
                                    result->internals.intVal = score->score;
                                }
                                else
#endif
                                {
                                    result->internals.intVal = 0;
                                }
                                result->dataType = VAL_INT;
                                if (uiscript_debug->current.integer)
                                    Com_Printf(13, "player( %s ) = %i\n", source->internals.string, result->internals.intVal);
                            }
                        }
                        else
                        {
                            result->dataType = VAL_INT;
                            result->internals.intVal = (unsigned __int8)CG_LookingThroughNightVision(localClientNum);
                            if (uiscript_debug->current.integer)
                                Com_Printf(13, "player( %s ) = %i\n", source->internals.string, result->internals.intVal);
                        }
                    }
                    else
                    {
                        result->dataType = VAL_INT;
                        result->internals.intVal = CG_GetPlayerClipAmmoCount(localClientNum);
                        if (uiscript_debug->current.integer)
                            Com_Printf(13, "player( %s ) = %s\n", source->internals.string, result->internals.string);
                    }
                }
                else
                {
                    result->dataType = VAL_INT;
                    result->internals.intVal = CG_IsPlayerDead(localClientNum);
                    if (uiscript_debug->current.integer)
                        Com_Printf(13, "player( %s ) = %i\n", source->internals.string, result->internals.intVal);
                }
            }
            else
            {
                result->dataType = VAL_STRING;
                result->internals.intVal = (int)CG_GetPlayerOpposingTeamName(localClientNum);
                if (uiscript_debug->current.integer)
                    Com_Printf(13, "player( %s ) = %s\n", source->internals.string, result->internals.string);
            }
        }
        else
        {
            result->dataType = VAL_STRING;
            result->internals.intVal = (int)CG_GetPlayerTeamName(localClientNum);
            if (uiscript_debug->current.integer)
                Com_Printf(13, "player( %s ) = %s\n", source->internals.string, result->internals.string);
        }
    }
    else
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: Must use a string as the name of a player field, not a %s\n", NameForValueType);
        result->dataType = VAL_STRING;
        result->internals.intVal = (int)"";
    }
}

void __cdecl GetOtherTeamField(int localClientNum, Operand *fieldName, Operand *result)
{
#ifdef KISAK_MP
    team_t team; // [esp+8h] [ebp-8h]
    cg_s *cgameGlob;

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->bgs.clientinfo[cgameGlob->clientNum].infoValid)
        team = cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team;
    else
        team = TEAM_FREE;
    switch (team)
    {
    case TEAM_AXIS:
        GetFieldForTeam(localClientNum, TEAM_ALLIES, fieldName, result);
        break;
    case TEAM_ALLIES:
        GetFieldForTeam(localClientNum, TEAM_AXIS, fieldName, result);
        break;
    case TEAM_SPECTATOR:
        GetFieldForTeam(localClientNum, TEAM_SPECTATOR, fieldName, result);
        break;
    default:
        GetFieldForTeam(localClientNum, TEAM_FREE, fieldName, result);
        break;
    }
#elif KISAK_SP
    GetFieldForTeam(localClientNum, TEAM_AXIS, fieldName, result);
#endif
}

void __cdecl GetFieldForTeam(int localClientNum, team_t team, Operand *fieldName, Operand *result)
{
    const char *NameForValueType; // eax
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (fieldName->dataType == VAL_STRING)
    {
        if (I_stricmp(fieldName->internals.string, "score"))
        {
            if (I_stricmp(fieldName->internals.string, "name"))
            {
                Com_Printf(13, "ERROR: Unknown team field '%s'\n", fieldName->internals.string);
                result->dataType = VAL_INT;
                result->internals.intVal = 0;
            }
            else
            {
                result->dataType = VAL_STRING;
                result->internals.intVal = (int)CG_GetTeamName(team);
                if (uiscript_debug->current.integer)
                    Com_Printf(13, "team(%i)( %s ) = %s\n", team, fieldName->internals.string, result->internals.string);
            }
        }
        else
        {
            result->dataType = VAL_INT;
#ifdef KISAK_MP
            result->internals.intVal = cgameGlob->teamScores[team];
#elif KISAK_SP
            result->internals.intVal = 0;
#endif
            if (uiscript_debug->current.integer)
                Com_Printf(13, "team(%i)( %s ) = %i\n", team, fieldName->internals.string, result->internals.intVal);
        }
    }
    else
    {
        NameForValueType = GetNameForValueType(fieldName->dataType);
        Com_PrintError(13, "Error: Must use a string as the name of a team parameter, not a %s\n", NameForValueType);
        result->dataType = VAL_STRING;
        result->internals.intVal = (int)"";
    }
}

void __cdecl GetTeamField(int localClientNum, Operand *fieldName, Operand *result)
{
#ifdef KISAK_MP
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);

    if (cgameGlob->bgs.clientinfo[cgameGlob->clientNum].infoValid)
        GetFieldForTeam(localClientNum, cgameGlob->bgs.clientinfo[cgameGlob->clientNum].team, fieldName, result);
    else
        GetFieldForTeam(localClientNum, TEAM_FREE, fieldName, result);
#elif KISAK_SP
    GetFieldForTeam(localClientNum, TEAM_ALLIES, fieldName, result);
#endif
}

void __cdecl GetTeamMarinesField(int localClientNum, Operand *fieldName, Operand *result)
{
    GetFieldForTeam(localClientNum, TEAM_ALLIES, fieldName, result);
}

void __cdecl GetTeamOpForField(int localClientNum, Operand *fieldName, Operand *result)
{
    GetFieldForTeam(localClientNum, TEAM_AXIS, fieldName, result);
}

void __cdecl GetUIActive(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = CL_IsUIActive(localClientNum);
    if (uiscript_debug->current.integer)
        Com_Printf(13, "ui_active() = %i\n", result->internals.intVal);
}

void __cdecl GetFlashbanged(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = CG_Flashbanged(localClientNum);
    if (uiscript_debug->current.integer)
        Com_Printf(13, "flashbanged() = %i\n", result->internals.intVal);
}

void __cdecl GetScoped(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = (unsigned __int8)CG_ScopeIsOverlayed(localClientNum);
    if (uiscript_debug->current.integer)
        Com_Printf(13, "scoped() = %i\n", result->internals.intVal);
}

void __cdecl InKillcam(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
#ifdef KISAK_MP
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    result->internals.intVal = cgameGlob->inKillCam;
#elif KISAK_SP
    result->internals.intVal = 0;
#endif
    if (uiscript_debug->current.integer)
        Com_Printf(13, "InKillcam() = %i\n", result->internals.intVal);
}

void __cdecl GetScoreboardVisible(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
#ifdef KISAK_MP
    result->internals.intVal = CG_IsScoreboardDisplayed(localClientNum);
#elif KISAK_SP
    result->internals.intVal = 0;
#endif
    if (uiscript_debug->current.integer)
        Com_Printf(13, "scoreboard_visible() = %i\n", result->internals.intVal);
}

void __cdecl GetSelectingLocation(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = CG_IsSelectingLocation(localClientNum);
    if (uiscript_debug->current.integer)
        Com_Printf(13, "selecting_location() = %i\n", result->internals.intVal);
}

void __cdecl PrivatePartyHostInLobby(int localClientNum, Operand *result)
{
    result->internals.intVal = 0;
    result->dataType = VAL_INT;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "PrivatePartyHostInLobby() = %i\n", result->internals.intVal);
}

void __cdecl AloneInPrivateParty(int localClientNum, Operand *result)
{
    result->internals.intVal = 0;
    result->dataType = VAL_INT;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "AloneInPrivateParty() = %i\n", result->internals.intVal);
}

void __cdecl InLobby(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = 0;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "InLobby() = %i\n", result->internals.intVal);
}

void __cdecl InPrivateParty(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = 0;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "InPrivateParty() = %i\n", result->internals.intVal);
}

void __cdecl PrivatePartyHost(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = 0;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "PrivatePartyHost() = %i\n", result->internals.intVal);
}

void __cdecl GetPlayerStat(int localClientNum, Operand *source, Operand *result)
{
    int v3; // eax
    int index; // [esp+0h] [ebp-4h]

    index = getOperandValueInt(source).intVal;
    result->dataType = VAL_INT;
    v3 = CL_ControllerIndexFromClientNum(localClientNum);
    result->internals.intVal = LiveStorage_GetStat(v3, index); // KISAKTODO: win_storage.cpp
    if (uiscript_debug->current.integer)
        Com_Printf(13, "stat( %i ) = %i\n", index, result->internals.intVal);
}

operandInternalDataUnion __cdecl getOperandValueInt(Operand *source)
{
    if (source->dataType == VAL_STRING)
    {
        return (operandInternalDataUnion)atoi(source->internals.string);
    }
    else if (source->dataType == VAL_FLOAT)
    {
        return (operandInternalDataUnion)(int)source->internals.floatVal;
    }
    else
    {
        return source->internals;
    }
}

void __cdecl GetPlayerStatRangeBitsSet(int localClientNum, OperandList *list, Operand *result)
{
    int bitMask; // [esp+4h] [ebp-14h]
    int controllerIndex; // [esp+8h] [ebp-10h]
    operandInternalDataUnion statIndex; // [esp+Ch] [ebp-Ch]
    int minStat; // [esp+10h] [ebp-8h]
    int maxStat; // [esp+14h] [ebp-4h]

    if (!list)
        MyAssertHandler(".\\ui\\ui_expressions.cpp", 931, 0, "%s", "list");
    if (!result)
        MyAssertHandler(".\\ui\\ui_expressions.cpp", 932, 0, "%s", "result");
    if (list->operandCount == 3)
    {
        minStat = getOperandValueInt(list->operands).intVal;
        maxStat = getOperandValueInt(&list->operands[1]).intVal;
        bitMask = getOperandValueInt(&list->operands[2]).intVal;
        if (minStat <= maxStat)
        {
            result->dataType = VAL_INT;
            result->internals.intVal = 0;
            controllerIndex = CL_ControllerIndexFromClientNum(localClientNum);
            for (statIndex.intVal = minStat; statIndex.intVal <= maxStat; ++statIndex.intVal)
            {
                if ((bitMask & LiveStorage_GetStat(controllerIndex, statIndex.intVal)) != 0) // KISAKTODO: win_storage.cpp
                {
                    result->internals.intVal = 1;
                    break;
                }
            }
            if (uiscript_debug->current.integer)
                Com_Printf(13, "statRangeAnyBitsSet( %i, %i, %i ) = %i\n", minStat, maxStat, bitMask, result->internals.intVal);
        }
        else
        {
            Com_PrintError(
                13,
                "UI Expression Error: minStat %i was greater than maxStat %i in StatRangeAnyBitsSet\n",
                minStat,
                maxStat);
            result->dataType = VAL_INT;
            result->internals.intVal = 0;
        }
    }
    else
    {
        Com_PrintError(
            13,
            "UI Expression Error: Expected 3 params to function StatRangeAnyBitsSet, found %i\n",
            list->operandCount);
        result->dataType = VAL_INT;
        result->internals.intVal = 0;
    }
}

int __cdecl GetKeyBindingLocalizedString(int localClientNum, const char *command, char *keys, bool single)
{
    char *v4; // eax
    const char *v6; // [esp+4h] [ebp-120h]
    char *translation; // [esp+10h] [ebp-114h]
    const char *translationa; // [esp+10h] [ebp-114h]
    const char *translation_4; // [esp+14h] [ebp-110h]
    int bindCount; // [esp+18h] [ebp-10Ch]
    char bindings[2][128]; // [esp+1Ch] [ebp-108h] BYREF
    const char *conjunction; // [esp+120h] [ebp-4h]

    bindCount = CL_GetKeyBinding(localClientNum, command, bindings);
    if (single && bindCount > 1)
        bindCount = 1;
    if (bindCount)
    {
        if (bindCount == 1)
        {
            translation = (char *)SEH_StringEd_GetString(bindings[0]);
            if (translation)
                I_strncpyz(keys, translation, 256);
            else
                I_strncpyz(keys, bindings[0], 256);
        }
        else
        {
            if (bindCount != 2)
                MyAssertHandler(".\\ui\\ui_shared.cpp", 5084, 0, "%s\n\t(bindCount) = %i", "(bindCount == 2)", bindCount);
            translationa = SEH_StringEd_GetString(bindings[0]);
            translation_4 = SEH_StringEd_GetString(bindings[1]);
            conjunction = UI_SafeTranslateString("KEY_OR");
            if (translation_4)
                v6 = translation_4;
            else
                v6 = bindings[1];
            if (translationa)
                Com_sprintf(keys, 0x100u, "%s %s %s", translationa, conjunction, v6);
            else
                Com_sprintf(keys, 0x100u, "%s %s %s", bindings[0], conjunction, v6);
        }
    }
    else
    {
        v4 = UI_SafeTranslateString("KEY_UNBOUND");
        I_strncpyz(keys, v4, 256);
    }
    return bindCount;
}

int __cdecl UI_GetKeyBindingLocalizedStringSingle(int localClientNum, const char *command, char *keys)
{
    return GetKeyBindingLocalizedString(localClientNum, command, keys, 1);
}

void __cdecl GetKeyBinding(int localClientNum, Operand *fieldName, Operand *result)
{
    static char resultString[256];

    const char *NameForValueType; // eax

    if (fieldName->dataType == VAL_STRING)
    {
        UI_GetKeyBindingLocalizedStringSingle(localClientNum, fieldName->internals.string, resultString);
        result->dataType = VAL_STRING;
        result->internals.intVal = (int)resultString;
    }
    else
    {
        NameForValueType = GetNameForValueType(fieldName->dataType);
        Com_PrintError(13, "Error: Must use a string as KeyBinding() parameter, not a %s\n", NameForValueType);
        result->dataType = VAL_STRING;
        result->internals.intVal = (int)"";
    }
}

void __cdecl GetActionSlotUsable(int localClientNum, Operand *fieldName, Operand *result)
{
    int slotId; // [esp+0h] [ebp-4h]

    slotId = getOperandValueInt(fieldName).intVal;
    if (slotId >= 1 && slotId <= 4)
    {
        result->dataType = VAL_INT;
        result->internals.intVal = CG_ActionSlotIsUsable(localClientNum, slotId - 1) != 0;
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: ActionSlot() slot ID should be in range (1,%i) not %i.\n", 4, slotId);
        result->dataType = VAL_INT;
        result->internals.intVal = 0;
    }
}

void __cdecl GetHudFade(int localClientNum, Operand *fieldName, Operand *result)
{
    const char *v3; // eax
    const char *NameForValueType; // eax

    if (!fieldName)
        MyAssertHandler(".\\ui\\ui_expressions.cpp", 1023, 0, "%s", "fieldName");
    if (!result)
        MyAssertHandler(".\\ui\\ui_expressions.cpp", 1024, 0, "%s", "result");
    if (CL_IsCgameInitialized(localClientNum))
    {
        result->dataType = VAL_FLOAT;
        if (fieldName->dataType == VAL_STRING)
        {
            if (I_stricmp(fieldName->internals.string, "dpad"))
            {
                if (I_stricmp(fieldName->internals.string, "weapon"))
                {
                    if (I_stricmp(fieldName->internals.string, "compass"))
                    {
                        if (I_stricmp(fieldName->internals.string, "scoreboard"))
                        {
                            NameForValueType = GetNameForValueType(fieldName->dataType);
                            Com_PrintError(
                                13,
                                "Error: Argument to HudFade() must be \"dpad\", \"compass\", \"scoreboard\", or \"weapon\".\n",
                                NameForValueType);
                            result->internals.floatVal = 0.0;
                        }
                        else
                        {
                            result->internals.floatVal = 1.0;
                        }
                    }
                    else
                    {
                        result->internals.floatVal = CG_GetHudAlphaCompass(localClientNum);
                    }
                }
                else
                {
                    result->internals.floatVal = CG_GetHudAlphaAmmoCounter(localClientNum);
                }
            }
            else
            {
                result->internals.floatVal = CG_GetHudAlphaDPad(localClientNum);
            }
        }
        else
        {
            v3 = GetNameForValueType(fieldName->dataType);
            Com_PrintError(13, "Error: Must use a string as HudFade() parameter, not a %s\n", v3);
            result->internals.floatVal = 0.0;
        }
    }
    else
    {
        result->internals.floatVal = 0.0;
    }
}

void __cdecl UI_GetOnlineFriendCount(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = 0;
}

void __cdecl IsMenuOpen(int localClientNum, Operand *source, Operand *result)
{
    const char *NameForValueType; // eax

    if (source->dataType == VAL_STRING)
    {
        result->dataType = VAL_INT;
        result->internals.intVal = Menu_IsMenuOpenAndVisible(localClientNum, source->internals.string);
        if (uiscript_debug->current.integer)
            Com_Printf(13, "ismenuopen( %s ) = %i\n", source->internals.string, result->internals.intVal);
    }
    else
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: Must use a string as the name of a menu, not a %s\n", NameForValueType);
        result->dataType = VAL_INT;
        result->internals.intVal = 0;
    }
}

void __cdecl WritingData(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = 0;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "writingdata() = %i\n", result->internals.intVal);
}

void __cdecl LogicalNot(int localClientNum, Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    char *SourceString; // [esp-4h] [ebp-Ch]

    result->dataType = VAL_INT;
    if (source->dataType == VAL_STRING)
    {
        SourceString = GetSourceString(*source);
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: You cannot ! a '%s' (%s)\n", NameForValueType, SourceString);
        result->internals.intVal = 0;
    }
    else if (source->dataType)
    {
        if (source->dataType == VAL_FLOAT)
            result->internals.intVal = source->internals.floatVal == 0.0;
        else
            Com_PrintError(16, "Unknown datatype %i in LogicalNot()\n", source->dataType);
    }
    else
    {
        result->internals.intVal = source->internals.intVal == 0;
    }
}

void __cdecl BitwiseNot(int localClientNum, Operand *source, Operand *result)
{
    const char *NameForValueType; // eax
    char *SourceString; // [esp-4h] [ebp-Ch]
    int val; // [esp+4h] [ebp-4h]

    result->dataType = VAL_INT;
    if (source->dataType == VAL_STRING)
    {
        SourceString = GetSourceString(*source);
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: You cannot ~ a '%s' (%s)\n", NameForValueType, SourceString);
        result->internals.intVal = 0;
    }
    else
    {
        if (source->dataType)
        {
            if (source->dataType == VAL_FLOAT)
            {
                val = (int)source->internals.floatVal;
            }
            else
            {
                val = 0;
                Com_PrintError(16, "Unknown datatype %i in LogicalNot()\n", source->dataType);
            }
        }
        else
        {
            val = source->internals.intVal;
        }
        result->internals.intVal = ~val;
        if (uiscript_debug->current.integer)
            Com_Printf(13, "~%i = %i\n", val, result->internals.intVal);
    }
}

void __cdecl BitShiftLeft(int localClientNum, Operand *source, Operand *bitsSource, Operand *result)
{
    int val; // [esp+0h] [ebp-8h]
    int bits; // [esp+4h] [ebp-4h]

    val = GetSourceInt(source).intVal;
    bits = GetSourceInt(bitsSource).intVal;
    result->dataType = VAL_INT;
    result->internals.intVal = val << bits;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "%i << %i = %i\n", val, bits, result->internals.intVal);
}

void __cdecl BitShiftRight(int localClientNum, Operand *source, Operand *bitsSource, Operand *result)
{
    int val; // [esp+0h] [ebp-8h]
    int bits; // [esp+4h] [ebp-4h]

    val = GetSourceInt(source).intVal;
    bits = GetSourceInt(bitsSource).intVal;
    result->dataType = VAL_INT;
    result->internals.intVal = val >> bits;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "%i >> %i = %i\n", val, bits, result->internals.intVal);
}

void __cdecl GetAdsJavelin(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    result->internals.intVal = clientUIActives[0].connectionState >= CA_LOADING && CG_JavelinADS(localClientNum);
    if (uiscript_debug->current.integer)
        Com_Printf(13, "adsjavelin() = %i\n", result->internals.intVal);
}

void __cdecl GetWeapLockBlink(int localClientNum, Operand *source, Operand *result)
{
    float bps; // [esp+Ch] [ebp-4h]

    bps = GetSourceFloat(source);
    result->dataType = VAL_INT;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    result->internals.intVal = clientUIActives[0].connectionState >= CA_LOADING && G_ExitAfterConnectPaths();
    if (uiscript_debug->current.integer)
        Com_Printf(13, "weaplockblink( %.2f ) = %i\n", bps, result->internals.intVal);
}

void __cdecl GetWeapAttackTop(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    result->internals.intVal = clientUIActives[0].connectionState >= CA_LOADING && G_ExitAfterConnectPaths();
    if (uiscript_debug->current.integer)
        Com_Printf(13, "weapattacktop() = %i\n", result->internals.intVal);
}

void __cdecl GetWeapAttackDirect(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    result->internals.intVal = clientUIActives[0].connectionState >= CA_LOADING && G_ExitAfterConnectPaths();
    if (uiscript_debug->current.integer)
        Com_Printf(13, "weapattackdirect() = %i\n", result->internals.intVal);
}

void __cdecl SecondsToTimeDisplay(int localClientNum, Operand *source, Operand *result)
{
    static char resultString_0[128];
    int v3; // [esp+4h] [ebp-14h]

    v3 = (int)((float)GetSourceInt(source).intVal / 60.0f);
    _snprintf(
        resultString_0,
        0x80u,
        "%id %ih %im",
        v3 / 1440,
        v3 % 1440 / 60,
        v3 % 60);
    result->dataType = VAL_STRING;
    result->internals.intVal = (int)resultString_0;
    if (uiscript_debug->current.integer)
        Com_Printf(13, "secondsToTime() = %s\n", resultString_0);
}

void __cdecl SecondsToCountdownDisplay(int localClientNum, int seconds, Operand *result)
{
    static char resultString_1[128];

    result->dataType = VAL_STRING;
    result->internals.intVal = (int)resultString_1;
    if (seconds >= 0)
    {
        _snprintf(resultString_1, 0x80u, "%2i:%02i", seconds / 60, seconds % 60);
        if (uiscript_debug->current.integer)
            Com_Printf(13, "secondsToCountdown() = %s\n", resultString_1);
    }
    else
    {
        result->internals.intVal = (int)"";
    }
}

void __cdecl GetTimeLeft(int localClientNum, Operand *result)
{
#ifdef KISAK_MP
    operandInternalDataUnion timeLeft; // [esp+0h] [ebp-Ch]
    cgs_t *cgs;
    cg_s *cgameGlob;

    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState < CA_LOADING)
    {
        result->dataType = VAL_INT;
        result->internals.intVal = 0;
    }
    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if (cgameGlob->nextSnap)
        timeLeft.intVal = (cgs->gameEndTime - cgameGlob->nextSnap->serverTime) / 1000;
    else
        timeLeft.intVal = 0;
    result->dataType = VAL_INT;
    result->internals = timeLeft;
#elif KISAK_SP
    result->dataType = VAL_INT;
    result->internals.intVal = 0;
#endif
}

void __cdecl GetGametypeObjective(int localClientNum, Operand *result)
{
#ifdef KISAK_MP
    result->dataType = VAL_STRING;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= CA_LOADING)
    {
        if (localClientNum)
            MyAssertHandler(
                "c:\\trees\\cod3\\src\\ui\\../cgame_mp/cg_local_mp.h",
                1071,
                0,
                "%s\n\t(localClientNum) = %i",
                "(localClientNum == 0)",
                localClientNum);
        result->internals.intVal = (int)CG_GetGametypeDescription(localClientNum);
        if (!result->internals.intVal)
            result->internals.intVal = (int)"";
    }
    else
    {
        result->internals.intVal = (int)"";
    }
#elif KISAK_SP
    result->dataType = VAL_STRING;
    result->internals.intVal = (int)"";
#endif
}

void __cdecl GetGametypeName(int localClientNum, Operand *result)
{
#ifdef KISAK_MP
    cgs_t *cgs;

    result->dataType = VAL_STRING;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= CA_LOADING)
    {
        cgs = CG_GetLocalClientStaticGlobals(localClientNum);
        result->internals.intVal = (int)UI_GetGameTypeDisplayName(cgs->gametype);
    }
    else if (g_gametype)
    {
        result->internals.intVal = (int)UI_GetGameTypeDisplayName(g_gametype->current.string);
    }
    else
    {
        result->internals.intVal = (int)"";
    }
    if (!result->internals.intVal)
        result->internals.intVal = (int)"";

#elif KISAK_SP
    result->dataType = VAL_STRING;
    result->internals.intVal = (int)"";
#endif
}

void __cdecl GetGametypeInternal(int localClientNum, Operand *result)
{
#ifdef KISAK_MP
    cgs_t *cgs = CG_GetLocalClientStaticGlobals(localClientNum);

    result->dataType = VAL_STRING;
    if (localClientNum)
        MyAssertHandler(
            "c:\\trees\\cod3\\src\\ui\\../client_mp/client_mp.h",
            1112,
            0,
            "%s\n\t(localClientNum) = %i",
            "(localClientNum == 0)",
            localClientNum);
    if (clientUIActives[0].connectionState >= CA_LOADING)
        result->internals.intVal = (int)cgs->gametype;
    else
        result->internals.intVal = g_gametype->current.integer;
    if (!result->internals.intVal)
        result->internals.intVal = (int)"";

#elif KISAK_SP
    result->dataType = VAL_STRING;
    result->internals.intVal = (int)"";
#endif
}

void __cdecl GetScore(int localClientNum, Operand *source, Operand *result)
{
#ifdef KISAK_MP
    const char *NameForValueType; // eax
    const score_t *score; // [esp+0h] [ebp-4h]

    result->dataType = VAL_INT;
    result->internals.intVal = 0;
    if (source->dataType)
    {
        NameForValueType = GetNameForValueType(source->dataType);
        Com_PrintError(13, "Error: Must use an integer for the rank: %s\n", NameForValueType);
    }
    else if (source->internals.intVal > 0)
    {
        score = UI_GetScoreAtRank(localClientNum, source->internals.intVal);
        if (score)
            result->internals.intVal = score->score;
    }
    else
    {
        Com_PrintError(13, "Error: rank must be > 0: %i\n", source->internals.intVal);
    }

#elif KISAK_SP
    result->dataType = VAL_INT;
    result->internals.intVal = 0;
#endif
}

void __cdecl GetGameMessageWindowActive(int localClientNum, Operand *source, Operand *result)
{
    int windowIndex; // [esp+0h] [ebp-4h]

    result->dataType = VAL_INT;
    result->internals.intVal = 0;
    windowIndex = GetSourceInt(source).intVal;
    if (Con_IsValidGameMessageWindow(windowIndex))
        result->internals.intVal = Con_IsGameMessageWindowActive(localClientNum, windowIndex);
    else
        Com_PrintError(13, "UI Script error: gameMsgWndActive was passed an invalid window index\n");
}

void __cdecl GetFollowing(int localClientNum, Operand *result)
{
    result->dataType = VAL_INT;
    result->internals.intVal = 0;
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ((cgameGlob->predictedPlayerState.otherFlags & 2) != 0)
        result->internals.intVal = 1;
}

void __cdecl RunCommaOp(int localClientNum, OperandStack *dataStack, OperandList *list1, OperandList *list2)
{
    operandInternalDataUnion v4; // edx
    operandInternalDataUnion v5; // eax
    int list1Operand; // [esp+4h] [ebp-18h]
    Operand *finalList; // [esp+8h] [ebp-14h]
    int operand; // [esp+Ch] [ebp-10h]
    Operand operandResult; // [esp+10h] [ebp-Ch] BYREF
    int list2Operand; // [esp+18h] [ebp-4h]

    if (list2->operandCount + list1->operandCount <= 10)
    {
        dataStack->stack[dataStack->numOperandLists].operandCount = list2->operandCount + list1->operandCount;
        finalList = dataStack->stack[dataStack->numOperandLists].operands;
        operand = 0;
        for (list1Operand = 0; list1Operand < list1->operandCount; ++list1Operand)
        {
            v4.intVal = (int)list1->operands[list1Operand].internals;
            finalList[operand].dataType = list1->operands[list1Operand].dataType;
            finalList[operand].internals = v4;
            if (finalList[operand].dataType == VAL_STRING && !finalList[operand].internals.intVal)
                MyAssertHandler(
                    ".\\ui\\ui_expressions.cpp",
                    1637,
                    0,
                    "%s",
                    "finalList[operand].dataType != VAL_STRING || finalList[operand].internals.string");
            ++operand;
        }
        for (list2Operand = 0; list2Operand < list2->operandCount; ++list2Operand)
        {
            v5.intVal = (int)list2->operands[list2Operand].internals;
            finalList[operand].dataType = list2->operands[list2Operand].dataType;
            finalList[operand].internals = v5;
            if (finalList[operand].dataType == VAL_STRING && !finalList[operand].internals.intVal)
                MyAssertHandler(
                    ".\\ui\\ui_expressions.cpp",
                    1643,
                    0,
                    "%s",
                    "finalList[operand].dataType != VAL_STRING || finalList[operand].internals.string");
            ++operand;
        }
        ++dataStack->numOperandLists;
    }
    else
    {
        Com_PrintError(13, "UI Script error: No function takes %i parameters\n", list2->operandCount + list1->operandCount);
        operandResult.dataType = VAL_INT;
        operandResult.internals.intVal = 0;
        AddOperandToStack(dataStack, &operandResult);
    }
}

void __cdecl TableLookup(int localClientNum, OperandList *list, Operand *operandResult)
{
    char *SourceString; // eax
    operandInternalDataUnion v4; // eax
    char *v5; // eax
    operandInternalDataUnion v6; // [esp-10h] [ebp-14h]
    char *v7; // [esp-Ch] [ebp-10h]
    char *v8; // [esp-8h] [ebp-Ch]
    operandInternalDataUnion v9; // [esp-8h] [ebp-Ch]
    int intVal; // [esp-4h] [ebp-8h]
    const char *string; // [esp-4h] [ebp-8h]
    StringTable *table; // [esp+0h] [ebp-4h] BYREF

#ifdef KISAK_NO_FASTFILES
    if (true)
#else
    if (IsFastFileLoad())
#endif
    {
        if (list->operandCount == 4)
        {
            SourceString = GetSourceString(list->operands[0]);
            StringTable_GetAsset(SourceString, &table);
            operandResult->dataType = VAL_STRING;
            intVal = GetSourceInt(&list->operands[3]).intVal;
            v8 = GetSourceString(list->operands[2]);
            v4.intVal = GetSourceInt(&list->operands[1]).intVal;
            operandResult->internals.intVal = (int)StringTable_Lookup(table, v4.intVal, v8, intVal);
            if (uiscript_debug->current.integer)
            {
                string = operandResult->internals.string;
                v9.intVal = GetSourceInt(&list->operands[3]).intVal;
                v7 = GetSourceString(list->operands[2]);
                v6.intVal = GetSourceInt(&list->operands[1]).intVal;
                v5 = GetSourceString(list->operands[0]);
                Com_Printf(13, "tablelookup( %s, %i, %s, %i ) == %s\n", v5, v6.intVal, v7, v9.intVal, string);
            }
        }
        else
        {
            Com_PrintError(
                13,
                "UI Expression Error: Expected 4 params to function StringTableLookup, found %i\n",
                list->operandCount);
            operandResult->dataType = VAL_STRING;
            operandResult->internals.intVal = (int)"";
        }
    }
    else
    {
        operandResult->dataType = VAL_STRING;
        operandResult->internals.intVal = (int)"";
    }
}

void __cdecl MinValue(OperandList *list, Operand *operandResult)
{
    float val; // [esp+0h] [ebp-Ch]
    int operandNum; // [esp+4h] [ebp-8h]
    float min; // [esp+8h] [ebp-4h]

    if (list->operandCount >= 1)
    {
        min = GetSourceFloat(list->operands);
        for (operandNum = 1; operandNum < list->operandCount; ++operandNum)
        {
            val = GetSourceFloat(&list->operands[operandNum]);
            if (min > (double)val)
                min = val;
        }
        operandResult->dataType = VAL_FLOAT;
        operandResult->internals.floatVal = min;
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected at least 1 parameter to min()\n");
        operandResult->dataType = VAL_FLOAT;
        operandResult->internals.floatVal = 0.0;
    }
}

void __cdecl MaxValue(OperandList *list, Operand *operandResult)
{
    float max; // [esp+0h] [ebp-Ch]
    float val; // [esp+4h] [ebp-8h]
    int operandNum; // [esp+8h] [ebp-4h]

    if (list->operandCount >= 1)
    {
        max = GetSourceFloat(list->operands);
        for (operandNum = 1; operandNum < list->operandCount; ++operandNum)
        {
            val = GetSourceFloat(&list->operands[operandNum]);
            if (max < (double)val)
                max = val;
        }
        operandResult->dataType = VAL_FLOAT;
        operandResult->internals.floatVal = max;
    }
    else
    {
        Com_PrintError(13, "UI Expression Error: Expected at least 1 parameter to max()\n");
        operandResult->dataType = VAL_FLOAT;
        operandResult->internals.floatVal = 0.0;
    }
}

void __cdecl LocalizeString(OperandList *list, Operand *operandResult)
{
    const char *v2; // eax
    char string[1024]; // [esp+20h] [ebp-428h] BYREF
    bool useLocalization; // [esp+427h] [ebp-21h]
    Operand *operand; // [esp+428h] [ebp-20h]
    unsigned int charIndex; // [esp+42Ch] [ebp-1Ch]
    unsigned int tokenLen; // [esp+430h] [ebp-18h]
    bool enableLocalization; // [esp+437h] [ebp-11h]
    expDataType type; // [esp+438h] [ebp-10h]
    int parmIndex; // [esp+43Ch] [ebp-Ch]
    const char *token; // [esp+440h] [ebp-8h]
    unsigned int stringLen; // [esp+444h] [ebp-4h]

    useLocalization = 1;
    stringLen = 0;
    for (parmIndex = 0; parmIndex < list->operandCount; ++parmIndex)
    {
        operand = &list->operands[parmIndex];
        type = operand->dataType;
        enableLocalization = 0;
        if (type == VAL_STRING)
        {
            token = GetSourceString(*operand);
            tokenLen = strlen(token);
            if (tokenLen <= 1)
                continue;
            ValidateLocalizedStringRef(token++, tokenLen--);
            if (stringLen + tokenLen + 1 >= 0x400)
                Com_Error(ERR_LOCALIZATION, "Error: %s is too long. Max length is %i\n", token, 1024);
            if (stringLen)
                string[stringLen++] = 20;
            useLocalization = 1;
        }
        else
        {
            token = GetSourceString(*operand);
            tokenLen = strlen(token);
            for (charIndex = 0; charIndex < tokenLen; ++charIndex)
            {
                if (token[charIndex] == 20 || token[charIndex] == 21 || token[charIndex] == 22)
                    Com_PrintError(13, "Error: bad escape character (%i) present in string", token[charIndex]);
                if (isalpha(token[charIndex]))
                {
                    v2 = va("Non-localized UI strings are not allowed to have letters in them: \"%s\"", token);
                    LocalizationError(v2);
                    break;
                }
            }
            if (stringLen + tokenLen + 1 >= 0x400)
                Com_Error(ERR_LOCALIZATION, "Error: %s is too long. Max length is %i\n", token, 1024);
            if (tokenLen)
                string[stringLen++] = 21;
            useLocalization = 0;
        }
        for (charIndex = 0; charIndex < tokenLen; ++charIndex)
        {
            if (token[charIndex] == 20 || token[charIndex] == 21 || token[charIndex] == 22)
                string[stringLen] = 46;
            else
                string[stringLen] = token[charIndex];
            ++stringLen;
        }
    }
    string[stringLen] = 0;
    operandResult->dataType = VAL_STRING;
    operandResult->internals.intVal = (int)SEH_LocalizeTextMessage(string, "ui string", LOCMSG_NOERR);
    if (!operandResult->internals.intVal)
        operandResult->internals.intVal = (int)"";
}

void __cdecl LocalizationError(const char *errorMessage)
{
    if (Dvar_GetBool("loc_warnings"))
    {
        if (!errorMessage)
            MyAssertHandler(".\\ui\\ui_expressions.cpp", 1833, 0, "%s", "errorMessage");
        if (Dvar_GetBool("loc_warningsAsErrors"))
            Com_Error(ERR_LOCALIZATION, "Error: %s", errorMessage);
        else
            Com_PrintWarning(13, "WARNING: %s\n", errorMessage);
    }
}

void __cdecl ValidateLocalizedStringRef(const char *token, int tokenLen)
{
    const char *v2; // eax
    const char *v3; // eax
    int charIter; // [esp+0h] [ebp-4h]

    if (!token)
        MyAssertHandler(".\\ui\\ui_expressions.cpp", 1847, 0, "%s", "token");
    if (tokenLen <= 0)
        MyAssertHandler(".\\ui\\ui_expressions.cpp", 1848, 0, "%s", "tokenLen > 0");
    if (*token != 64)
    {
        v2 = va("Illegal localized string reference: %s must start with a '@'.", token);
        LocalizationError(v2);
    }
    for (charIter = 1; charIter < tokenLen; ++charIter)
    {
        if (!isalnum(token[charIter]) && token[charIter] != 95)
        {
            v3 = va(
                "Illegal localized string reference: %s must contain only alpha-numeric characters and underscores",
                token);
            LocalizationError(v3);
        }
    }
}

void __cdecl RunHigherPriorityOperators(
    int localClientNum,
    operationEnum op,
    OperatorStack *opStack,
    OperandStack *dataStack)
{
    int opNum; // [esp+0h] [ebp-4h]

    opNum = opStack->numOperators - 1;
    while (opNum >= 0 && opStack->numOperators)
    {
        opNum = opStack->numOperators - 1;
        if ((s_operatorPrecedence[opStack->stack[opNum]] >= s_operatorPrecedence[op]
            || s_operatorPrecedence[opStack->stack[opNum]] == 5 && op != OP_RIGHTPAREN)
            && (IsOpAssociative(op) || opStack->stack[opNum] != op))
        {
            break;
        }
        RunOp(localClientNum, opStack, dataStack);
    }
}

bool __cdecl IsOpAssociative(operationEnum op)
{
    return op < OP_DIVIDE || op > OP_MODULUS && op != OP_SUBTRACT;
}

int lastWarnTime;
char *__cdecl GetExpressionResultString(int localClientNum, const statement_s *statement)
{
    static char resultString_2[256];

    const char *v3; // eax
    Operand result; // [esp+Ch] [ebp-Ch] BYREF
    int len; // [esp+14h] [ebp-4h]

    if (!EvaluateExpression(localClientNum, statement, &result))
        return (char *)"";
    len = 0;
    if (result.dataType)
    {
        if (result.dataType == VAL_FLOAT)
        {
            len = Com_sprintf(resultString_2, 0x100u, "%f", result.internals.floatVal);
        }
        else
        {
            if (result.dataType != VAL_STRING)
            {
                if (!alwaysfails)
                {
                    v3 = va("Unknown result datatype of %i", result.dataType);
                    MyAssertHandler(".\\ui\\ui_expressions.cpp", 2466, 0, v3);
                }
                return (char *)"";
            }
            len = Com_sprintf(resultString_2, 0x100u, "%s", result.internals.string);
        }
    }
    else
    {
        len = Com_sprintf(resultString_2, 0x100u, "%i", result.internals.intVal);
    }
    if (len < 0 && (int)(Sys_Milliseconds() - lastWarnTime) > 5000)
    {
        lastWarnTime = Sys_Milliseconds();
        Com_PrintWarning(
            13,
            "Warning: Expression result string has been truncated, longer than %d characters: %s...\n",
            256,
            resultString_2);
    }
    return resultString_2;
}

const statement_s *g_releaseBuildStatement;
Operand *__cdecl EvaluateExpression(int localClientNum, const statement_s *statement, Operand *result)
{
    expressionEntry *v4; // [esp+0h] [ebp-14BCh]
    OperatorStack opStack; // [esp+4h] [ebp-14B8h] BYREF
    OperandStack dst; // [esp+FCh] [ebp-13C0h] BYREF
    int i; // [esp+14B8h] [ebp-4h]

    g_releaseBuildStatement = statement;
    memset((unsigned __int8 *)&dst, 0, sizeof(dst));
    opStack.numOperators = 0;
    for (i = 0; i < statement->numEntries; ++i)
    {
        v4 = statement->entries[i];
        if (v4->type == 1)
        {
            if (dst.numOperandLists == 60)
            {
                Com_PrintError(13, "Invalid expression - too many operands\n");
                return 0;
            }
            AddOperandToStack(&dst, (Operand *)&v4->data);
        }
        else if (!v4->type)
        {
            if (v4->data.op != OP_LEFTPAREN)
                RunHigherPriorityOperators(localClientNum, v4->data.op, &opStack, &dst);
            if (opStack.numOperators == 60)
            {
                Com_PrintError(13, "Invalid expression - operators are nested too deeply\n");
                return 0;
            }
            opStack.stack[opStack.numOperators++] = v4->data.op;
        }
    }
    while (opStack.numOperators)
        RunOp(localClientNum, &opStack, &dst);
    if (dst.numOperandLists <= 1)
    {
        if (dst.numOperandLists)
        {
            if (dst.stack[0].operandCount <= 1)
            {
                *result = dst.stack[0].operands[0];
                return result;
            }
            else
            {
                Com_PrintError(13, "Error: More than one operand in expression result\n");
                return 0;
            }
        }
        else
        {
            if (!alwaysfails)
                MyAssertHandler(".\\ui\\ui_expressions.cpp", 2430, 0, "At the end of IsStatementTrue, we have no operands!");
            return 0;
        }
    }
    else
    {
        Com_PrintError(13, "Error: stray operands in expression\n");
        return 0;
    }
}

bool __cdecl IsExpressionTrue(int localClientNum, const statement_s *statement)
{
    Operand result; // [esp+0h] [ebp-8h] BYREF

    return EvaluateExpression(localClientNum, statement, &result) && GetSourceInt(&result).intVal != 0;
}

double __cdecl GetExpressionFloat(int localClientNum, const statement_s *statement)
{
    PROF_SCOPED("GetExpressionFloat");

    Operand result; // [esp+0h] [ebp-8h] BYREF

    if (EvaluateExpression(localClientNum, statement, &result))
        return GetSourceFloat(&result);
    else
        return 0.0;
}

