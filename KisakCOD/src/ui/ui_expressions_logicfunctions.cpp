#include "ui.h"

int __cdecl compare_impact_files(const char **pe0, const char **pe1)
{
    return I_stricmp(*pe0, *pe1);
}

int __cdecl compare_hudelems(const void *pe0, const void *pe1)
{
    float delta; // [esp+0h] [ebp-Ch]

    delta = *(float *)(*(unsigned int *)pe0 + 128) - *(float *)(*(unsigned int *)pe1 + 128);
    if (delta >= 0.0)
        return delta > 0.0;
    else
        return -1;
}

int __cdecl compare_use(float *pe1, float *pe2)
{
    return (int)(pe1[1] - pe2[1]);
}

void __cdecl compare_doesStringEqualString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            12,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            13,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_STRING)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = I_stricmp(leftSide->internals.string, rightSide->internals.string) == 0;
}

void __cdecl compare_doesStringNotEqualString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            21,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            22,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_STRING)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = I_stricmp(leftSide->internals.string, rightSide->internals.string) != 0;
}

void __cdecl compare_doesIntEqualInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            30,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            31,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.intVal == rightSide->internals.intVal;
}

void __cdecl compare_doesIntNotEqualInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            38,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            39,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.intVal != rightSide->internals.intVal;
}

void __cdecl compare_doesIntEqualFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            46,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            47,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal == (double)leftSide->internals.intVal;
}

void __cdecl compare_doesFloatEqualInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    compare_doesIntEqualFloat(rightSide, leftSide, result);
}

void __cdecl compare_doesFloatEqualFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            59,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            60,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal == leftSide->internals.floatVal;
}

void __cdecl compare_doesIntNotEqualFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            67,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            68,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal != (double)leftSide->internals.intVal;
}

void __cdecl compare_doesFloatNotEqualInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    compare_doesIntNotEqualFloat(rightSide, leftSide, result);
}

void __cdecl compare_doesFloatNotEqualFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            80,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            81,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal != leftSide->internals.floatVal;
}

void __cdecl compare_isIntLessThanInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            88,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            89,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.intVal < rightSide->internals.intVal;
}

void __cdecl compare_isIntLessThanFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            96,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            97,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal > (double)leftSide->internals.intVal;
}

void __cdecl compare_isFloatLessThanInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            104,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            105,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.floatVal < (double)rightSide->internals.intVal;
}

void __cdecl compare_isFloatLessThanFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            112,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            113,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal > (double)leftSide->internals.floatVal;
}

void __cdecl compare_isIntLessThanEqualToInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            120,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            121,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.intVal <= rightSide->internals.intVal;
}

void __cdecl compare_isFloatGreaterThanEqualToInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            128,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            129,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.floatVal >= (double)rightSide->internals.intVal;
}

void __cdecl compare_isIntLessThanEqualToFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    compare_isFloatGreaterThanEqualToInt(rightSide, leftSide, result);
}

void __cdecl compare_isFloatLessThanEqualToInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            141,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            142,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.floatVal <= (double)rightSide->internals.intVal;
}

void __cdecl compare_isFloatLessThanEqualToFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            149,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            150,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal >= (double)leftSide->internals.floatVal;
}

void __cdecl compare_isIntGreaterThanInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    compare_isIntLessThanInt(rightSide, leftSide, result);
}

void __cdecl compare_isIntGreaterThanFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    compare_isFloatLessThanInt(rightSide, leftSide, result);
}

void __cdecl compare_isFloatGreaterThanInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            167,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            168,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.floatVal > (double)rightSide->internals.intVal;
}

void __cdecl compare_isFloatGreaterThanFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            175,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            176,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal < (double)leftSide->internals.floatVal;
}

void __cdecl compare_isIntGreaterThanEqualToInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            183,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            184,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.intVal >= rightSide->internals.intVal;
}

void __cdecl compare_isIntGreaterThanEqualToFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            191,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            192,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal <= (double)leftSide->internals.intVal;
}

void __cdecl compare_isFloatGreaterThanEqualToFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            199,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            200,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.floatVal <= (double)leftSide->internals.floatVal;
}

int __cdecl compare_floats(float *e0, float *e1)
{
    float diff; // [esp+4h] [ebp-4h]

    diff = *e0 - *e1;
    if (diff >= 0.0)
        return diff > 0.0;
    else
        return -1;
}

int bloc;
void __cdecl add_bit(char bit, unsigned __int8 *fout)
{
    if ((bloc & 7) == 0)
        fout[bloc >> 3] = 0;
    fout[bloc >> 3] |= bit << (bloc & 7);
    ++bloc;
}

void __cdecl add_IntWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            207,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            208,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.intVal + leftSide->internals.intVal;
}

void __cdecl add_IntWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            215,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            216,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    result->internals.floatVal = (double)leftSide->internals.intVal + rightSide->internals.floatVal;
}

void __cdecl add_FloatWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    add_IntWithFloat(rightSide, leftSide, result);
}

void __cdecl add_FloatWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            228,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            229,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    result->internals.floatVal = leftSide->internals.floatVal + rightSide->internals.floatVal;
}

char resultStr[256];
void __cdecl add_StringWithString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    char rightSideStr[256]; // [esp+0h] [ebp-208h] BYREF
    char leftSideStr[260]; // [esp+100h] [ebp-108h] BYREF

    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            240,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            241,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_STRING)",
            rightSide->dataType);
    result->dataType = VAL_STRING;
    I_strncpyz(leftSideStr, (char *)leftSide->internals.intVal, 256);
    I_strncpyz(rightSideStr, (char *)rightSide->internals.intVal, 256);
    Com_sprintf(resultStr, 0x100u, "%s%s", leftSideStr, rightSideStr);
    result->internals.intVal = (int)resultStr;
}

char resultStr_0[256];
void __cdecl add_StringWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    char leftSideStr[260]; // [esp+0h] [ebp-108h] BYREF

    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            254,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            255,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_STRING;
    I_strncpyz(leftSideStr, (char *)leftSide->internals.intVal, 256);
    Com_sprintf(resultStr_0, 0x100u, "%s%i", leftSideStr, rightSide->internals.intVal);
    result->internals.intVal = (int)resultStr_0;
}

char resultStr_1[256];
void __cdecl add_IntWithString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    char rightSideStr[260]; // [esp+0h] [ebp-108h] BYREF

    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            268,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            rightSide->dataType);
    if (rightSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            269,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(rightSide->dataType == VAL_STRING)",
            leftSide->dataType);
    result->dataType = VAL_STRING;
    I_strncpyz(rightSideStr, (char *)rightSide->internals.intVal, 256);
    Com_sprintf(resultStr_1, 0x100u, "%i%s", leftSide->internals.intVal, rightSideStr);
    result->internals.intVal = (int)resultStr_1;
}

char resultStr_2[256];
void __cdecl add_FloatWithString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    char rightSideStr[260]; // [esp+Ch] [ebp-108h] BYREF

    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            281,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    if (rightSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            282,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(rightSide->dataType == VAL_STRING)",
            leftSide->dataType);
    result->dataType = VAL_STRING;
    I_strncpyz(rightSideStr, (char *)rightSide->internals.intVal, 256);
    Com_sprintf(resultStr_2, 0x100u, "%f%s", leftSide->internals.floatVal, rightSideStr);
    result->internals.intVal = (int)resultStr_2;
}

char resultStr_3[256];
void __cdecl add_StringWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    char leftSideStr[260]; // [esp+8h] [ebp-108h] BYREF

    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            295,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            296,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_STRING;
    I_strncpyz(leftSideStr, (char *)leftSide->internals.intVal, 256);
    Com_sprintf(resultStr_3, 0x100u, "%s%f", leftSideStr, rightSide->internals.floatVal);
    result->internals.intVal = (int)resultStr_3;
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

void __cdecl multiply_IntByInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            342,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            343,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = rightSide->internals.intVal * leftSide->internals.intVal;
}

void __cdecl multiply_IntByFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            350,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            351,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    result->internals.floatVal = (double)leftSide->internals.intVal * rightSide->internals.floatVal;
}

void __cdecl multiply_FloatByInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    multiply_IntByFloat(rightSide, leftSide, result);
}

void __cdecl multiply_FloatByFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            363,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            364,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    result->internals.floatVal = leftSide->internals.floatVal * rightSide->internals.floatVal;
}

void __cdecl divide_IntByInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            373,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            374,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    if (rightSide->internals.intVal)
        result->internals.floatVal = (double)leftSide->internals.intVal / (double)rightSide->internals.intVal;
    else
        result->internals.floatVal = 0.0;
}

void __cdecl divide_IntByFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            384,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            385,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    if (rightSide->internals.floatVal == 0.0)
        result->internals.floatVal = 0.0;
    else
        result->internals.floatVal = (double)leftSide->internals.intVal / rightSide->internals.floatVal;
}

void __cdecl divide_FloatByInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            395,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            396,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    if (rightSide->internals.intVal)
        result->internals.floatVal = leftSide->internals.floatVal / (double)rightSide->internals.intVal;
    else
        result->internals.floatVal = 0.0;
}

void __cdecl divide_FloatByFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            406,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            407,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    if (rightSide->internals.floatVal == 0.0)
        result->internals.floatVal = 0.0;
    else
        result->internals.floatVal = leftSide->internals.floatVal / rightSide->internals.floatVal;
}

void __cdecl mod_IntByInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            419,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            420,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    if (rightSide->internals.intVal)
        result->internals.intVal = leftSide->internals.intVal % rightSide->internals.intVal;
    else
        result->internals.intVal = leftSide->internals.intVal;
}

void __cdecl mod_FloatByInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            430,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            431,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    if (rightSide->internals.intVal)
        result->internals.intVal = (int)(leftSide->internals.floatVal + 9.313225746154785e-10) % rightSide->internals.intVal;
    else
        result->internals.intVal = leftSide->internals.intVal;
}

void __cdecl mod_IntByFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    int right; // [esp+10h] [ebp-4h]

    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            442,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            443,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    right = (int)(rightSide->internals.floatVal + 9.313225746154785e-10);
    if (right)
        result->internals.intVal = leftSide->internals.intVal % right;
    else
        result->internals.intVal = leftSide->internals.intVal;
}

void __cdecl mod_FloatByFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    int right; // [esp+38h] [ebp-4h]

    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            455,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            456,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    right = (int)(rightSide->internals.floatVal + 9.313225746154785e-10);
    if (right)
        result->internals.intVal = (int)(leftSide->internals.floatVal + 9.313225746154785e-10) % right;
    else
        result->internals.intVal = (int)(leftSide->internals.floatVal + 9.313225746154785e-10);
}

void __cdecl and_IntWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            469,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            470,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = leftSide->internals.intVal && rightSide->internals.intVal;
    result->internals.intVal = v3;
}

void __cdecl and_FloatWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            477,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            478,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = leftSide->internals.floatVal != 0.0 && rightSide->internals.intVal;
    result->internals.intVal = v3;
}

void __cdecl and_IntWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    and_FloatWithInt(rightSide, leftSide, result);
}

void __cdecl and_StringWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            490,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            491,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = *(_BYTE *)leftSide->internals.intVal && rightSide->internals.intVal;
    result->internals.intVal = v3;
}

void __cdecl and_IntWithString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    and_StringWithInt(rightSide, leftSide, result);
}

void __cdecl and_StringWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            503,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            504,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = *(_BYTE *)leftSide->internals.intVal && rightSide->internals.floatVal != 0.0;
    result->internals.intVal = v3;
}

void __cdecl and_FloatWithString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    and_StringWithFloat(rightSide, leftSide, result);
}

void __cdecl and_FloatWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            516,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            517,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = leftSide->internals.floatVal != 0.0 && rightSide->internals.floatVal != 0.0;
    result->internals.intVal = v3;
}

void __cdecl or_IntWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            526,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            527,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = leftSide->internals.intVal || rightSide->internals.intVal;
    result->internals.intVal = v3;
}

void __cdecl or_FloatWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            534,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            535,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = leftSide->internals.floatVal != 0.0 || rightSide->internals.intVal;
    result->internals.intVal = v3;
}

void __cdecl or_IntWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    or_FloatWithInt(rightSide, leftSide, result);
}

void __cdecl or_StringWithInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            547,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            548,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = *(_BYTE *)leftSide->internals.intVal || rightSide->internals.intVal;
    result->internals.intVal = v3;
}

void __cdecl or_IntWithString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    or_StringWithInt(rightSide, leftSide, result);
}

void __cdecl or_StringWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType != VAL_STRING)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            560,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_STRING)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            561,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = *(_BYTE *)leftSide->internals.intVal || rightSide->internals.floatVal != 0.0;
    result->internals.intVal = v3;
}

void __cdecl or_FloatWithString(Operand *leftSide, Operand *rightSide, Operand *result)
{
    or_StringWithFloat(rightSide, leftSide, result);
}

void __cdecl or_FloatWithFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    bool v3; // [esp+0h] [ebp-4h]

    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            573,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            574,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    v3 = leftSide->internals.floatVal != 0.0 || rightSide->internals.floatVal != 0.0;
    result->internals.intVal = v3;
}

void __cdecl bitwiseAnd(Operand *leftSide, Operand *rightSide, Operand *result)
{
    operandInternalDataUnion v3; // esi

    result->dataType = VAL_INT;
    v3.intVal = GetSourceInt(leftSide).intVal;
    result->internals.intVal = GetSourceInt(rightSide).intVal & v3.intVal;
}

void __cdecl bitwiseOr(Operand *leftSide, Operand *rightSide, Operand *result)
{
    operandInternalDataUnion v3; // esi

    result->dataType = VAL_INT;
    v3.intVal = GetSourceInt(leftSide).intVal;
    result->internals.intVal = GetSourceInt(rightSide).intVal | v3.intVal;
}

void __cdecl multiply_J(int constraintRowCount, ConstraintRowData *rows, float *in)
{
    float suma; // [esp+0h] [ebp-18h]
    float sum; // [esp+0h] [ebp-18h]
    float sumb; // [esp+0h] [ebp-18h]
    int i; // [esp+8h] [ebp-10h]
    int body2; // [esp+Ch] [ebp-Ch]
    float *in_ptr; // [esp+10h] [ebp-8h]

    for (i = 0; i < constraintRowCount; ++i)
    {
        body2 = rows->body2;
        in_ptr = &in[6 * rows->body1];
        suma = Vec3Dot(rows->J_body1Linear, in_ptr) + (float)0.0;
        sum = Vec3Dot(rows->J_body1Angular, in_ptr + 3) + suma;
        if (body2 >= 0)
        {
            sumb = Vec3Dot(rows->J_body2Linear, &in[6 * body2]) + sum;
            sum = Vec3Dot(rows->J_body2Angular, &in[6 * body2 + 3]) + sumb;
        }
        rows->rhs = sum;
        ++rows;
    }
}


void __cdecl subtract_IntFromInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            308,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            309,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_INT;
    result->internals.intVal = leftSide->internals.intVal - rightSide->internals.intVal;
}

void __cdecl subtract_FloatFromInt(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            316,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_INT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            317,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    result->internals.floatVal = (double)leftSide->internals.intVal - rightSide->internals.floatVal;
}

void __cdecl subtract_IntFromFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            324,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            325,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_INT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    result->internals.floatVal = leftSide->internals.floatVal - (double)rightSide->internals.intVal;
}

void __cdecl subtract_FloatFromFloat(Operand *leftSide, Operand *rightSide, Operand *result)
{
    if (leftSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            332,
            0,
            "%s\n\t(leftSide->dataType) = %i",
            "(leftSide->dataType == VAL_FLOAT)",
            leftSide->dataType);
    if (rightSide->dataType != VAL_FLOAT)
        MyAssertHandler(
            ".\\ui\\ui_expressions_logicfunctions.cpp",
            333,
            0,
            "%s\n\t(rightSide->dataType) = %i",
            "(rightSide->dataType == VAL_FLOAT)",
            rightSide->dataType);
    result->dataType = VAL_FLOAT;
    result->internals.floatVal = leftSide->internals.floatVal - rightSide->internals.floatVal;
}

