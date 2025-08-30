#pragma once
#include <database/database.h>
#include "scr_debugger.h"
#include "scr_yacc.h"

void __cdecl Scr_InitAllocNode();
void __cdecl Scr_ShutdownAllocNode();
sval_u *__cdecl Scr_AllocNode(int size);
sval_u __cdecl node0(Enum_t type);
sval_u __cdecl node1(Enum_t type, sval_u val2);
sval_u __cdecl node2(Enum_t type, sval_u val1, sval_u val2);
sval_u __cdecl node3(Enum_t type, sval_u val1, sval_u val2, sval_u val3);
sval_u __cdecl node4(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4);
sval_u __cdecl node5(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5);
sval_u __cdecl node6(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6);
sval_u __cdecl node7(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6, sval_u val7);
sval_u __cdecl node8(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6, sval_u val7, sval_u val8);

inline sval_u __cdecl node0(int type)
{
	return node0((Enum_t)type);
}
inline sval_u __cdecl node1(int type, sval_u val2)
{
	return node1((Enum_t)type, val2);
}
inline sval_u __cdecl node2(int type, sval_u val1, sval_u val2)
{
	return node2((Enum_t)type, val1, val2);
}
inline sval_u __cdecl node3(int type, sval_u val1, sval_u val2, sval_u val3)
{
	return node3((Enum_t)type, val1, val2, val3);
}
inline sval_u __cdecl node4(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4)
{
	return node4((Enum_t)type, val1, val2, val3, val4);
}
inline sval_u __cdecl node5(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5)
{
	return node5((Enum_t)type, val1, val2, val3, val4, val5);
}
inline sval_u __cdecl node6(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6)
{
	return node6((Enum_t)type, val1, val2, val3, val4, val5, val6);
}
inline sval_u __cdecl node7(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6, sval_u val7)
{
	return node7((Enum_t)type, val1, val2, val3, val4, val5, val6, val7);
}
inline sval_u __cdecl node8(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6, sval_u val7, sval_u val8)
{
	return node8((Enum_t)type, val1, val2, val3, val4, val5, val6, val7, val8);
}


sval_u linked_list_end(sval_u val1);
sval_u prepend_node(sval_u val1, sval_u val2);
sval_u append_node(sval_u val1, sval_u val2);

void __cdecl Scr_ClearDebugExpr(debugger_sval_s *debugExprHead);
sval_u *__cdecl Scr_AllocDebugExpr(Enum_t type, int size, const char *name);
void __cdecl Scr_FreeDebugExpr(ScriptExpression_t *expr);

sval_u __cdecl debugger_node0(Enum_t type);
sval_u __cdecl debugger_node1(Enum_t type, sval_u val1);
sval_u __cdecl debugger_node2(Enum_t type, sval_u val1, sval_u val2);
sval_u __cdecl debugger_node3(Enum_t type, sval_u val1, sval_u val2, sval_u val3);
sval_u __cdecl debugger_node4(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4);

sval_u __cdecl debugger_prepend_node(sval_u val1, sval_u val2);
sval_u __cdecl debugger_buffer(Enum_t type, char *buf, unsigned int size, int alignment);
sval_u __cdecl debugger_string(Enum_t type, char *s);


extern HunkUser *g_allocNodeUser;