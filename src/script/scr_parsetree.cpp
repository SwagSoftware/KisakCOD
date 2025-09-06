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

sval_u *__cdecl Scr_AllocNode(int size)
{
    if (!g_allocNodeUser)
        MyAssertHandler(".\\script\\scr_parsetree.cpp", 82, 0, "%s", "g_allocNodeUser");
    return (sval_u *)Hunk_UserAlloc(g_allocNodeUser, 4 * size, 4);
}

sval_u __cdecl node0(Enum_t type)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(1);
    result.node[0].type = type;
    return result;
}

sval_u __cdecl node1(Enum_t type, sval_u val1)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(2);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    return result;
}

sval_u __cdecl node2(Enum_t type, sval_u val1, sval_u val2)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(3);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    return result;
}

sval_u __cdecl node3(Enum_t type, sval_u val1, sval_u val2, sval_u val3)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(4);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    return result;
}

sval_u __cdecl node4(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(5);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    return result;
}

sval_u __cdecl node5(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(6);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    result.node[5].node = val5.node;
    return result;
}

sval_u __cdecl node6(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4, sval_u val5, sval_u val6)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(7);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    result.node[5].node = val5.node;
    result.node[6].node = val6.node;
    return result;
}

sval_u __cdecl node7(
    Enum_t type,
    sval_u val1,
    sval_u val2,
    sval_u val3,
    sval_u val4,
    sval_u val5,
    sval_u val6,
    sval_u val7)
{
    sval_u result; // eax

    result.node = Scr_AllocNode(8);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    result.node[5].node = val5.node;
    result.node[6].node = val6.node;
    result.node[7].node = val7.node;
    return result;
}

sval_u __cdecl node8(
    Enum_t type,
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

    result.node = Scr_AllocNode(9);
    result.node[0].type = type;
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    result.node[5].node = val5.node;
    result.node[6].node = val6.node;
    result.node[7].node = val7.node;
    result.node[8].node = val8.node;
    return result;
}

// Decomp Status: Tested, Completed
sval_u linked_list_end(sval_u val1)
{
    sval_u *node;
    sval_u result;

    node = Scr_AllocNode(2);
    node[0].node = val1.node;
    node[1].stringValue = 0;
    result.node = Scr_AllocNode(2);
    result.node[0].node = node;
    result.node[1].node = node;
    return result;
}

// Decomp Status: Tested, Completed
sval_u prepend_node(sval_u val1, sval_u val2)
{
    sval_u *node;

    node = Scr_AllocNode(2);
    node[0] = val1;
    node[1] = *val2.node;
    val2.node->node = node;
    return val2;
}

// Decomp Status: Tested, Completed
sval_u append_node(sval_u val1, sval_u val2)
{
    sval_u *node;

    node = Scr_AllocNode(2);
    node[0] = val2;
    node[1].stringValue = 0;
    val1.node[1].node[1].node = node;
    val1.node[1].node = node;
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

sval_u *__cdecl Scr_AllocDebugExpr(Enum_t type, int size, const char *name)
{
    sval_u *val; // eax
    debugger_sval_s *debugval;

    // prefix the malloc with a `debugger_sval_s`
    unsigned char *data = (unsigned char*)Z_Malloc(sizeof(debugger_sval_s) + size, name, 0);

    debugval = (debugger_sval_s *)data;
    val = (sval_u *)(data + sizeof(debugger_sval_s));

    // prepend the global list
    debugval->next = g_debugExprHead;
    g_debugExprHead = debugval;

    // set val type (convenience vs. the non-debug way) and return it
    val->type = type;
    return val;
}

void __cdecl Scr_FreeDebugExpr(ScriptExpression_t *expr)
{
    debugger_sval_s *debugExprHead; // [esp+0h] [ebp-Ch]
    debugger_sval_s *nextDebugExprHead; // [esp+8h] [ebp-4h]

    if (expr->breakonExpr)
        --scrVmDebugPub.checkBreakon;

    debugExprHead = expr->exprHead;

    iassert(debugExprHead);

    while (debugExprHead)
    {
        // See Prefixed data in Scr_AllocDebugExpr()
        sval_u *pval = (sval_u *)((char *)debugExprHead + sizeof(debugger_sval_s));
        Scr_FreeDebugExprValue(*(sval_u*)&pval);

        nextDebugExprHead = debugExprHead->next;
        Z_Free(debugExprHead, 0);
        debugExprHead = nextDebugExprHead;
    }
}

sval_u __cdecl debugger_node0(Enum_t type)
{
    sval_u result;
    result.node = Scr_AllocDebugExpr(type, 4, "debugger_node0");
    return result;
}

sval_u __cdecl debugger_node1(Enum_t type, sval_u val1)
{
    sval_u result; // eax

    result.node = Scr_AllocDebugExpr(type, 8, "debugger_node1");
    result.node[1].node = val1.node;

    return result;
}

sval_u __cdecl debugger_node2(Enum_t type, sval_u val1, sval_u val2)
{
    sval_u result; // eax

    result.node = Scr_AllocDebugExpr(type, 12, "debugger_node2");
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    return result;
}

sval_u __cdecl debugger_node3(Enum_t type, sval_u val1, sval_u val2, sval_u val3)
{
    sval_u result; // eax

    result.node = Scr_AllocDebugExpr(type, 16, "debugger_node3");
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    return result;
}

sval_u __cdecl debugger_node4(Enum_t type, sval_u val1, sval_u val2, sval_u val3, sval_u val4)
{
    sval_u result; // eax

    result.node = Scr_AllocDebugExpr(type, 20, "debugger_node4");
    result.node[1].node = val1.node;
    result.node[2].node = val2.node;
    result.node[3].node = val3.node;
    result.node[4].node = val4.node;
    return result;
}

sval_u __cdecl debugger_prepend_node(sval_u val1, sval_u val2)
{
    *(_DWORD *)val2.type = debugger_node2(ENUM_NOP, val1, (sval_u)val2.node->type).type + 4;
    return val2;
}

sval_u __cdecl debugger_buffer(Enum_t type, char *buf, unsigned int size, int alignment)
{
    sval_u *result; // [esp+4h] [ebp-8h]
    unsigned __int8 *bufCopy; // [esp+8h] [ebp-4h]
    int alignmenta; // [esp+20h] [ebp+14h]

    if ((alignment & (alignment - 1)) != 0)
        MyAssertHandler((char *)".\\script\\scr_parsetree.cpp", 594, 0, "%s", "IsPowerOf2( alignment )");
    alignmenta = alignment - 1;
    result = Scr_AllocDebugExpr(type, size + alignmenta + 8, "debugger_buffer");
    bufCopy = (unsigned __int8 *)(~alignmenta & ((unsigned int)&result[2] + alignmenta));
    memcpy(bufCopy, (unsigned __int8 *)buf, size);
    result[1].intValue = (int)bufCopy;
    return *result; // sus deref
}

sval_u __cdecl debugger_string(Enum_t type, char *s)
{
    return debugger_buffer(type, s, strlen(s) + 1, 1);
}

