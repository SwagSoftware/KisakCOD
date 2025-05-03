#pragma once
#include <database/database.h>
#include "scr_debugger.h"

void __cdecl Scr_InitAllocNode();
void __cdecl Scr_ShutdownAllocNode();
sval_u __cdecl node0(int type);
sval_u *__cdecl Scr_AllocNode(int size);
sval_u __cdecl node2(int type, sval_u val1, sval_u val2);
sval_u __cdecl node1(int val1, sval_u val2);
sval_u __cdecl node3(int type, sval_u val1, sval_u val2, sval_u val3);
sval_u __cdecl node4(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4);
sval_u __cdecl node5(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5);
sval_u __cdecl node6(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6);
sval_u __cdecl node7(
    int type,
    sval_u val1,
    sval_u val2,
    sval_u val3,
    sval_u val4,
    sval_u val5,
    sval_u val6,
    sval_u val7);
sval_u __cdecl node8(
    int type,
    sval_u val1,
    sval_u val2,
    sval_u val3,
    sval_u val4,
    sval_u val5,
    sval_u val6,
    sval_u val7,
    sval_u val8);
sval_u* __cdecl linked_list_end(sval_u val);
sval_u __cdecl prepend_node(sval_u val1, sval_u val2);
sval_u __cdecl append_node(sval_u val1, sval_u val2);
void __cdecl Scr_ClearDebugExpr(debugger_sval_s *debugExprHead);
sval_u __cdecl Scr_AllocDebugExpr(debugger_sval_s *type, int size, const char *name);
void __cdecl Scr_FreeDebugExpr(ScriptExpression_t *expr);
sval_u __cdecl debugger_node0(debugger_sval_s *type);
sval_u __cdecl debugger_node1(debugger_sval_s *type, sval_u val1);
sval_u __cdecl debugger_node2(debugger_sval_s *type, sval_u val1, sval_u val2);
sval_u __cdecl debugger_node3(debugger_sval_s *type, sval_u val1, sval_u val2, sval_u val3);
sval_u __cdecl debugger_node4(debugger_sval_s *type, sval_u val1, sval_u val2, sval_u val3, sval_u val4);
sval_u __cdecl debugger_prepend_node(sval_u val1, sval_u val2);
sval_u __cdecl debugger_buffer(debugger_sval_s *type, char *buf, unsigned int size, int alignment);
sval_u __cdecl debugger_string(debugger_sval_s *type, char *s);


extern HunkUser *g_allocNodeUser;