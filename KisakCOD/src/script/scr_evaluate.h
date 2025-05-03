#pragma once
#include "scr_variable.h"
#include "scr_debugger.h"

struct ArchivedCanonicalStringInfo // sizeof=0x8
{
    unsigned __int16 canonicalStr;
    // padding byte
    // padding byte
    const char *value;
};

struct scrEvaluateGlob_t // sizeof=0x10
{                                       // ...
    char *archivedCanonicalStringsBuf;  // ...
    ArchivedCanonicalStringInfo *archivedCanonicalStrings; // ...
    int *canonicalStringLookup;         // ...
    bool freezeScope;                   // ...
    bool freezeObjects;                 // ...
    bool objectChanged;                 // ...
    // padding byte
};

void __cdecl TRACK_scr_evaluate();
unsigned int __cdecl Scr_GetBuiltin(sval_u func_name);
int __cdecl Scr_CompareCanonicalStrings(unsigned int *arg1, unsigned int *arg2);
void __cdecl Scr_ArchiveCanonicalStrings();
int __cdecl CompareCanonicalStrings(const char **arg1, const char **arg2);
const char *__cdecl Scr_GetCanonicalString(unsigned int fieldName);
void __cdecl Scr_InitEvaluate();
void __cdecl Scr_EndLoadEvaluate();
void __cdecl Scr_ShutdownEvaluate();
unsigned __int16 __cdecl Scr_CompileCanonicalString(unsigned int stringValue);
void __cdecl Scr_GetFieldValue(unsigned int objectId, const char *fieldName, int len, char *text);
void __cdecl Scr_GetValueString(unsigned int localId, VariableValue *value, int len, char *s);
void __cdecl Scr_EvalArrayVariable(unsigned int arrayId, VariableValue *value);
void __cdecl Scr_EvalArrayVariableInternal(VariableValue *parentValue, VariableValue *value);
void __cdecl Scr_ClearValue(VariableValue *value);
void __cdecl Scr_EvalFieldVariableInternal(unsigned int objectId, unsigned int fieldName, VariableValue *value);
void __cdecl Scr_EvalFieldVariable(unsigned int fieldName, VariableValue *value, unsigned int objectId);
void __cdecl Scr_CompileExpression(sval_u *expr);
void __cdecl Scr_CompilePrimitiveExpression(sval_u *expr);
void __cdecl Scr_CompileVariableExpression(sval_u *expr);
void __cdecl Scr_CompilePrimitiveExpressionFieldObject(sval_u *expr);
int __cdecl GetExpressionCount(sval_u exprlist);
void __cdecl Scr_CompilePrimitiveExpressionList(sval_u *exprlist);
char __cdecl Scr_CompileCallExpression(sval_u *expr);
char __cdecl Scr_CompileFunction(sval_u *func_name, sval_u *params);
void __cdecl Scr_CompileCallExpressionList(sval_u *exprlist);
char __cdecl Scr_CompileMethod(sval_u *expr, sval_u *func_name, sval_u *params);
void __cdecl Scr_CompileText(const char *text, ScriptExpression_t *scriptExpr);
void __cdecl Scr_CompileTextInternal(const char *text, ScriptExpression_t *scriptExpr);
bool __cdecl Scr_EvalScriptExpression(
    ScriptExpression_t *expr,
    unsigned int localId,
    VariableValue *value,
    bool freezeScope,
    bool freezeObjects);
void __cdecl Scr_EvalExpression(sval_u expr, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalPrimitiveExpression(sval_u expr, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalVariableExpression(sval_u expr, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalArrayVariableExpression(sval_u array, sval_u index, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalLocalVariable(sval_u expr, unsigned int localId, VariableValue *value);
VariableValueInternal_u __cdecl Scr_EvalObject(sval_u classnum, sval_u entnum, VariableValue *value);
void __cdecl Scr_EvalSelfValue(VariableValue *value);
void __cdecl Scr_GetValue(unsigned int index, VariableValue *value);
unsigned int __cdecl Scr_EvalPrimitiveExpressionFieldObject(sval_u expr, unsigned int localId);
void __cdecl Scr_EvalCallExpression(sval_u expr, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalFunction(sval_u func_name, sval_u params, unsigned int localId, VariableValue *value);
void __cdecl Scr_PreEvalBuiltin(sval_u params, unsigned int localId);
void __cdecl Scr_PostEvalBuiltin(VariableValue *value);
void __cdecl Scr_EvalMethod(sval_u expr, sval_u func_name, sval_u params, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalBoolOrExpression(sval_u expr1, sval_u expr2, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalBoolAndExpression(sval_u expr1, sval_u expr2, unsigned int localId, VariableValue *value);
void __cdecl Scr_EvalBinaryOperatorExpression(
    sval_u expr1,
    sval_u expr2,
    sval_u opcode,
    unsigned int localId,
    VariableValue *value);
void __cdecl Scr_EvalVector(sval_u expr1, sval_u expr2, sval_u expr3, unsigned int localId, VariableValue *value);
void __cdecl Scr_ClearDebugExprValue(sval_u val);
bool __cdecl Scr_RefScriptExpression(ScriptExpression_t *expr);
bool __cdecl Scr_RefExpression(sval_u expr);
bool __cdecl Scr_RefPrimitiveExpression(sval_u expr);
bool __cdecl Scr_RefVariableExpression(sval_u expr);
bool __cdecl Scr_RefArrayVariableExpression(sval_u array, sval_u index);
bool __cdecl Scr_RefBreakonExpression(sval_u expr, sval_u param);
bool __cdecl Scr_RefCallExpression(sval_u expr);
bool __cdecl Scr_RefCall(sval_u params);
bool __cdecl Scr_RefMethod(sval_u expr, sval_u params);
bool __cdecl Scr_RefBinaryOperatorExpression(sval_u expr1, sval_u expr2);
bool __cdecl Scr_RefVector(sval_u expr1, sval_u expr2, sval_u expr3);
void __cdecl Scr_FreeDebugExprValue(sval_u val);


extern scrEvaluateGlob_t scrEvaluateGlob;
extern debugger_sval_s *g_debugExprHead;
extern int g_script_error_level;