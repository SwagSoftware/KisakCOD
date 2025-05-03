#include "scr_parsetree.h"
#include <universal/assertive.h>
#include <universal/com_memory.h>
#include "scr_evaluate.h"
#include "scr_vm.h"


//struct debugger_sval_s *g_debugExprHead 83123658     scr_parsetree.obj

HunkUser *g_allocNodeUser;

void __cdecl Scr_InitAllocNode()
{
    if (g_allocNodeUser)
        MyAssertHandler(".\\script\\scr_parsetree.cpp", 66, 0, "%s", "!g_allocNodeUser");
    g_allocNodeUser = Hunk_UserCreate(0x10000, "Scr_InitAllocNode", 0, 1, 7);
}

void __cdecl Scr_ShutdownAllocNode()
{
    if (g_allocNodeUser)
    {
        Hunk_UserDestroy(g_allocNodeUser);
        g_allocNodeUser = 0;
    }
}

sval_u __cdecl node0(int type)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(1);
    *(unsigned int *)result.type = type;
    return result;
}

sval_u *__cdecl Scr_AllocNode(int size)
{
    if (!g_allocNodeUser)
        MyAssertHandler(".\\script\\scr_parsetree.cpp", 82, 0, "%s", "g_allocNodeUser");
    return (sval_u *)Hunk_UserAlloc(g_allocNodeUser, 4 * size, 4);
}

sval_u __cdecl node2(int type, sval_u val1, sval_u val2)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(3);
    *(unsigned int *)result.type = type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    return result;
}

sval_u __cdecl node1(int val1, sval_u val2)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(2);
    *(unsigned int *)result.type = val1;
    *(sval_u *)(result.type + 4) = val2;
    return result;
}

sval_u __cdecl node3(int type, sval_u val1, sval_u val2, sval_u val3)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(4);
    *(unsigned int *)result.type = type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    *(sval_u *)(result.type + 12) = val3;
    return result;
}

sval_u __cdecl node4(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(5);
    *(unsigned int *)result.type = type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    *(sval_u *)(result.type + 12) = val3;
    *(sval_u *)(result.type + 16) = val4;
    return result;
}

sval_u __cdecl node5(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(6);
    *(unsigned int *)result.type = type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    *(sval_u *)(result.type + 12) = val3;
    *(sval_u *)(result.type + 16) = val4;
    *(sval_u *)(result.type + 20) = val5;
    return result;
}

sval_u __cdecl node6(int type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(7);
    *(unsigned int *)result.type = type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    *(sval_u *)(result.type + 12) = val3;
    *(sval_u *)(result.type + 16) = val4;
    *(sval_u *)(result.type + 20) = val5;
    *(sval_u *)(result.type + 24) = val6;
    return result;
}

sval_u __cdecl node7(
    int type,
    sval_u val1,
    sval_u val2,
    sval_u val3,
    sval_u val4,
    sval_u val5,
    sval_u val6,
    sval_u val7)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(8);
    *(unsigned int *)result.type = type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    *(sval_u *)(result.type + 12) = val3;
    *(sval_u *)(result.type + 16) = val4;
    *(sval_u *)(result.type + 20) = val5;
    *(sval_u *)(result.type + 24) = val6;
    *(sval_u *)(result.type + 28) = val7;
    return result;
}

sval_u __cdecl node8(
    int type,
    sval_u val1,
    sval_u val2,
    sval_u val3,
    sval_u val4,
    sval_u val5,
    sval_u val6,
    sval_u val7,
    sval_u val8)
{
    sval_u result; // eax

    result.type = (int)Scr_AllocNode(9);
    *(unsigned int *)result.type = type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    *(sval_u *)(result.type + 12) = val3;
    *(sval_u *)(result.type + 16) = val4;
    *(sval_u *)(result.type + 20) = val5;
    *(sval_u *)(result.type + 24) = val6;
    *(sval_u *)(result.type + 28) = val7;
    *(sval_u *)(result.type + 32) = val8;
    return result;
}

// KISAKTODO: These linked_list functions are horrible
sval_u *__cdecl linked_list_end(sval_u *val)
{
    sval_u *result; // eax
    sval_u *node; // [esp+0h] [ebp-8h]

    node = Scr_AllocNode(2);
    node[0] = *val;
    node[1].type = 0;
    result = Scr_AllocNode(2);
    result[0] = node[0];
    result[1] = node[0];
    return result;
}

sval_u *__cdecl prepend_node(sval_u *val1, sval_u *val2)
{
    sval_u *v2; // eax

    v2 = Scr_AllocNode(2);
    v2[0] = *val1;
    v2[1] = *val2;
    *val2 = v2[0];
    return val2;
}

sval_u *__cdecl append_node(sval_u *val1, sval_u *val2)
{
    sval_u *v2; // eax

    v2 = Scr_AllocNode(2);
    v2[0] = *val2;
    v2[1].type = 0;
    *(sval_u**)(val1[1].type + 4) = v2;
    val1[1].type = (int)v2;
    return val1;
}

void __cdecl Scr_ClearDebugExpr(debugger_sval_s *debugExprHead)
{
    while (debugExprHead)
    {
        Scr_ClearDebugExprValue(*(sval_u*)&debugExprHead[1]);
        debugExprHead = debugExprHead->next;
    }
}

sval_u __cdecl Scr_AllocDebugExpr(debugger_sval_s *type, int size, const char *name)
{
    debugger_sval_s *v3; // eax

    v3 = (debugger_sval_s *)Z_Malloc(size + 4, name, 0);
    v3->next = g_debugExprHead;
    g_debugExprHead = v3;
    v3[1].next = type;
    return *(sval_u*)&v3[1];
}

void __cdecl Scr_FreeDebugExpr(ScriptExpression_t *expr)
{
    char *debugExprHead; // [esp+0h] [ebp-Ch]
    debugger_sval_s *nextDebugExprHead; // [esp+8h] [ebp-4h]

    if (expr->breakonExpr)
        --scrVmDebugPub.checkBreakon;
    debugExprHead = (char *)expr->exprHead;
    if (!debugExprHead)
        MyAssertHandler(".\\script\\scr_parsetree.cpp", 395, 0, "%s", "debugExprHead");
    while (debugExprHead)
    {
        Scr_FreeDebugExprValue(*(sval_u*)(debugExprHead + 4));
        nextDebugExprHead = *(debugger_sval_s **)debugExprHead;
        Z_Free(debugExprHead, 0);
        debugExprHead = (char *)nextDebugExprHead;
    }
}

sval_u __cdecl debugger_node0(debugger_sval_s *type)
{
    return Scr_AllocDebugExpr(type, 4, "debugger_node0");
}

sval_u __cdecl debugger_node1(debugger_sval_s *type, sval_u val1)
{
    sval_u result; // eax

    result.type = Scr_AllocDebugExpr(type, 8, "debugger_node1").type;
    *(sval_u *)(result.type + 4) = val1;
    return result;
}

sval_u __cdecl debugger_node2(debugger_sval_s *type, sval_u val1, sval_u val2)
{
    sval_u result; // eax

    result.type = Scr_AllocDebugExpr(type, 12, "debugger_node2").type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    return result;
}

sval_u __cdecl debugger_node3(debugger_sval_s *type, sval_u val1, sval_u val2, sval_u val3)
{
    sval_u result; // eax

    result.type = Scr_AllocDebugExpr(type, 16, "debugger_node3").type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    *(sval_u *)(result.type + 12) = val3;
    return result;
}

sval_u __cdecl debugger_node4(debugger_sval_s *type, sval_u val1, sval_u val2, sval_u val3, sval_u val4)
{
    sval_u result; // eax

    result.type = Scr_AllocDebugExpr(type, 20, "debugger_node4").type;
    *(sval_u *)(result.type + 4) = val1;
    *(sval_u *)(result.type + 8) = val2;
    *(sval_u *)(result.type + 12) = val3;
    *(sval_u *)(result.type + 16) = val4;
    return result;
}

sval_u __cdecl debugger_prepend_node(sval_u val1, sval_u val2)
{
    *(unsigned int *)val2.type = debugger_node2(0, val1, (sval_u)val2.node->type).type + 4;
    return val2;
}

sval_u __cdecl debugger_buffer(debugger_sval_s *type, char *buf, unsigned int size, int alignment)
{
    sval_u result; // [esp+4h] [ebp-8h]
    unsigned __int8 *bufCopy; // [esp+8h] [ebp-4h]
    int alignmenta; // [esp+20h] [ebp+14h]

    if ((alignment & (alignment - 1)) != 0)
        MyAssertHandler(".\\script\\scr_parsetree.cpp", 594, 0, "%s", "IsPowerOf2( alignment )");
    alignmenta = alignment - 1;
    result.type = Scr_AllocDebugExpr(type, size + alignmenta + 8, "debugger_buffer").type;
    bufCopy = (unsigned __int8 *)(~alignmenta & (result.type + alignmenta + 8));
    memcpy(bufCopy, (unsigned __int8 *)buf, size);
    *(unsigned int *)(result.type + 4) = (unsigned int)bufCopy;
    return result;
}

sval_u __cdecl debugger_string(debugger_sval_s *type, char *s)
{
    return debugger_buffer(type, s, strlen(s) + 1, 1);
}

