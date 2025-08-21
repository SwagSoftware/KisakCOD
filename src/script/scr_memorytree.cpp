#include "scr_memorytree.h"
#include "scr_stringlist.h"

#include <win32/win_local.h>
#include <qcommon/qcommon.h>
#include <cstdint>
#include <universal/profile.h>

scrMemTreePub_t scrMemTreePub;
int marker_scr_memorytree;

scrMemTreeGlob_t scrMemTreeGlob;

struct scrMemTreeDebugGlob_t // sizeof=0x20000
{                                       // ...
    unsigned __int8 mt_usage[MEMORY_NODE_COUNT];    // ...
    unsigned __int8 mt_usage_size[MEMORY_NODE_COUNT]; // ...
};
scrMemTreeDebugGlob_t scrMemTreeDebugGlob = { 0 };

void MT_Init()
{
	Sys_EnterCriticalSection(CRITSECT_MEMORY_TREE);

    scrMemTreePub.mt_buffer = (char*)&scrMemTreeGlob.nodes;
    MT_InitBits();

    for (int i = 0; i <= MEMORY_NODE_BITS; ++i)
        scrMemTreeGlob.head[i] = 0;

    scrMemTreeGlob.nodes[0].prev = 0;
    scrMemTreeGlob.nodes[0].next = 0;

    for (int i = 0; i < MEMORY_NODE_BITS; ++i)
        MT_AddMemoryNode(1 << i, i);

    scrMemTreeGlob.totalAlloc = 0;
    scrMemTreeGlob.totalAllocBuckets = 0;
    memset(scrMemTreeDebugGlob.mt_usage, 0, sizeof(scrMemTreeDebugGlob.mt_usage));
    memset(scrMemTreeDebugGlob.mt_usage_size, 0, sizeof(scrMemTreeDebugGlob.mt_usage_size));

	Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
}

void MT_InitBits(void)
{
    unsigned __int8 bits; // [esp+0h] [ebp-Ch]
    int temp; // [esp+4h] [ebp-8h]

    for (int i = 0; i < NUM_BUCKETS; ++i)
    {
        bits = 0;
        for (temp = i; temp; temp >>= 1)
        {
            if ((temp & 1) != 0)
                ++bits;
        }
        scrMemTreeGlob.numBits[i] = bits;

        for (bits = 8; (i & ((1 << bits) - 1)) != 0; --bits);

        scrMemTreeGlob.leftBits[i] = bits;
        bits = 0;
        for (temp = i; temp; temp >>= 1)
        {
            ++bits;
        }
        scrMemTreeGlob.logBits[i] = bits;
    }
}

void* MT_Alloc(int numBytes, int type)
{
    return &scrMemTreeGlob.nodes[MT_AllocIndex(numBytes, type)];
}

unsigned short MT_AllocIndex(int numBytes, int type)
{
    const char* v2; // eax
    const char* v3; // eax
    unsigned int nodeNum; // [esp+4Ch] [ebp-Ch]
    unsigned int size; // [esp+50h] [ebp-8h]
    int newSize; // [esp+54h] [ebp-4h]

    PROF_SCOPED("scriptMemory");

    size = MT_GetSize(numBytes);
    iassert(size >= 0 && size <= MEMORY_NODE_BITS);

    Sys_EnterCriticalSection(CRITSECT_MEMORY_TREE);
    for (newSize = size; ; ++newSize)
    {
        if (newSize > MEMORY_NODE_BITS)
        {
            Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
            MT_Error("MT_AllocIndex", numBytes);
            return 0;
        }
        nodeNum = scrMemTreeGlob.head[newSize];
        if (scrMemTreeGlob.head[newSize])
            break;
    }
    MT_RemoveHeadMemoryNode(newSize);
    while (newSize != size)
    {
        --newSize;
        MT_AddMemoryNode(nodeNum + (1 << newSize), newSize);
    }
    ++scrMemTreeGlob.totalAlloc;
    scrMemTreeGlob.totalAllocBuckets += 1 << size;

    iassert(type);
    iassert((!scrMemTreeDebugGlob.mt_usage[nodeNum]));
    iassert((!scrMemTreeDebugGlob.mt_usage_size[nodeNum]));

    scrMemTreeDebugGlob.mt_usage[nodeNum] = type;
    scrMemTreeDebugGlob.mt_usage_size[nodeNum] = size;
    Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
    return nodeNum;
}

bool MT_Realloc(int oldNumBytes, int newNumbytes)
{
    return MT_GetSize(oldNumBytes) >= MT_GetSize(newNumbytes);
}

void MT_RemoveHeadMemoryNode(int size)
{
    MemoryNode tempNodeValue;
    int oldNode;
    MemoryNode oldNodeValue;
    uint16_t *parentNode;
    int prevScore;
    int nextScore;

    iassert(size >= 0 && size <= MEMORY_NODE_BITS);

    parentNode = &scrMemTreeGlob.head[size];
    oldNodeValue = scrMemTreeGlob.nodes[*parentNode];

    while (1)
    {
        if (!oldNodeValue.prev)
        {
            oldNode = oldNodeValue.next;
            *parentNode = oldNodeValue.next;
            if (!oldNode)
            {
                break;
            }
            parentNode = &scrMemTreeGlob.nodes[oldNode].next;
        }
        else
        {
            if (oldNodeValue.next)
            {
                prevScore = MT_GetScore(oldNodeValue.prev);
                nextScore = MT_GetScore(oldNodeValue.next);

                iassert(prevScore != nextScore);

                if (prevScore >= nextScore)
                {
                    oldNode = oldNodeValue.prev;
                    *parentNode = oldNode;
                    parentNode = &scrMemTreeGlob.nodes[oldNode].prev;
                }
                else
                {
                    oldNode = oldNodeValue.next;
                    *parentNode = oldNode;
                    parentNode = &scrMemTreeGlob.nodes[oldNode].next;
                }
            }
            else
            {
                oldNode = oldNodeValue.prev;
                *parentNode = oldNode;
                parentNode = &scrMemTreeGlob.nodes[oldNode].prev;
            }
        }
        iassert(oldNode != 0);

        tempNodeValue = oldNodeValue;
        oldNodeValue = scrMemTreeGlob.nodes[oldNode];
        scrMemTreeGlob.nodes[oldNode] = tempNodeValue;
    }
}

void MT_FreeIndex(unsigned int nodeNum, int numBytes)
{
    const char* v2; // eax
    int size; // [esp+30h] [ebp-8h]
    int lowBit; // [esp+34h] [ebp-4h]

    PROF_SCOPED("scriptMemory");

    size = MT_GetSize(numBytes);

    iassert(size >= 0 && size <= MEMORY_NODE_BITS);
    iassert(nodeNum > 0 && nodeNum < MEMORY_NODE_COUNT);

    Sys_EnterCriticalSection(CRITSECT_MEMORY_TREE);
    --scrMemTreeGlob.totalAlloc;
    scrMemTreeGlob.totalAllocBuckets -= 1 << size;

    iassert(scrMemTreeDebugGlob.mt_usage[nodeNum]);

    if (scrMemTreeDebugGlob.mt_usage_size[nodeNum] != size)
    {
        //v2 = MT_NodeInfoString(nodeNum);
        iassert((scrMemTreeDebugGlob.mt_usage_size[nodeNum] == size));
    }
    scrMemTreeDebugGlob.mt_usage[nodeNum] = 0;
    scrMemTreeDebugGlob.mt_usage_size[nodeNum] = 0;
    while (1)
    {
        iassert(size <= MEMORY_NODE_BITS);

        lowBit = 1 << size;

        iassert(nodeNum == (nodeNum & ~(lowBit - 1)));

        if (size == 16 || !MT_RemoveMemoryNode(lowBit ^ nodeNum, size))
            break;

        nodeNum &= ~lowBit;
        ++size;
    }
    MT_AddMemoryNode(nodeNum, size);
    Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
}

bool __cdecl MT_RemoveMemoryNode(int oldNode, unsigned int size)
{
    MemoryNode tempNodeValue;
    int node;
    MemoryNode oldNodeValue;
    int nodeNum;
    uint16_t *parentNode;
    int prevScore;
    int nextScore;
    int level;

    iassert(size >= 0 && size <= MEMORY_NODE_BITS);

    nodeNum = 0;
    level = MEMORY_NODE_COUNT;
    parentNode = &scrMemTreeGlob.head[size];

    for (node = *parentNode; node; node = *parentNode)
    {
        if (oldNode == node)
        {
            oldNodeValue = scrMemTreeGlob.nodes[oldNode];

            while (1)
            {
                if (oldNodeValue.prev)
                {
                    if (oldNodeValue.next)
                    {
                        prevScore = MT_GetScore(oldNodeValue.prev);
                        nextScore = MT_GetScore(oldNodeValue.next);

                        iassert(prevScore != nextScore);

                        if (prevScore >= nextScore)
                        {
                            oldNode = oldNodeValue.prev;
                            *parentNode = oldNodeValue.prev;
                            parentNode = &scrMemTreeGlob.nodes[oldNodeValue.prev].prev;
                        }
                        else
                        {
                            oldNode = oldNodeValue.next;
                            *parentNode = oldNodeValue.next;
                            parentNode = &scrMemTreeGlob.nodes[oldNodeValue.next].next;
                        }
                    }
                    else
                    {
                        oldNode = oldNodeValue.prev;
                        *parentNode = oldNodeValue.prev;
                        parentNode = &scrMemTreeGlob.nodes[oldNodeValue.prev].prev;
                    }
                }
                else
                {
                    oldNode = oldNodeValue.next;
                    *parentNode = oldNodeValue.next;

                    if (!oldNodeValue.next)
                    {
                        return true;
                    }

                    parentNode = &scrMemTreeGlob.nodes[oldNodeValue.next].next;
                }

                iassert(oldNode != 0);

                tempNodeValue = oldNodeValue;
                oldNodeValue = scrMemTreeGlob.nodes[oldNode];
                scrMemTreeGlob.nodes[oldNode] = tempNodeValue;
            }
        }

        if (oldNode == nodeNum)
        {
            return false;
        }

        level >>= 1;

        if (oldNode >= nodeNum)
        {
            parentNode = &scrMemTreeGlob.nodes[node].next;
            nodeNum += level;
        }
        else
        {
            parentNode = &scrMemTreeGlob.nodes[node].prev;
            nodeNum -= level;
        }
    }

    return false;
}

void MT_Free(byte* p, int numBytes)
{
	iassert(((MemoryNode*)p - scrMemTreeGlob.nodes >= 0 && (MemoryNode*)p - scrMemTreeGlob.nodes < MEMORY_NODE_COUNT));

    MT_FreeIndex((MemoryNode *)p - scrMemTreeGlob.nodes, numBytes);
}

int MT_GetSize(int numBytes)
{
    int numBuckets; // [esp+4h] [ebp-4h]

    iassert(numBytes > 0);

    if (numBytes >= MEMORY_NODE_COUNT)
    {
        MT_Error("MT_GetSize: max allocation exceeded", numBytes);
        return 0;
    }
    else
    {
        numBuckets = (numBytes + 11) / 12 - 1;
        if (numBuckets > 255)
            return scrMemTreeGlob.logBits[numBuckets >> 8] + 8;
        else
            return scrMemTreeGlob.logBits[numBuckets];
    }
}

int MT_GetScore(int num)
{
    char bits;

    iassert(num != 0);

    union MTnum_t
    {
        int i;
        uint8_t b[4];
    };

    MTnum_t mtnum;

    mtnum.i = MEMORY_NODE_COUNT - num;
    iassert(mtnum.i != 0);

    bits = scrMemTreeGlob.leftBits[mtnum.b[0]];

    if (!mtnum.b[0])
    {
        bits += scrMemTreeGlob.leftBits[mtnum.b[1]];
    }

    return mtnum.i - (scrMemTreeGlob.numBits[mtnum.b[1]] + scrMemTreeGlob.numBits[mtnum.b[0]]) + (1 << bits);
}

int MT_GetSubTreeSize(int nodeNum)
{
    if (!nodeNum)
        return 0;

    return MT_GetSubTreeSize(scrMemTreeGlob.nodes[nodeNum].prev) + MT_GetSubTreeSize(scrMemTreeGlob.nodes[nodeNum].next) + 1;
}

void MT_AddMemoryNode(int newNode, int size)
{
    int node;
    int nodeNum;
    int newScore;
    uint16_t *parentNode;
    int level;
    int score;

    iassert(size >= 0 && size <= MEMORY_NODE_BITS);

    parentNode = &scrMemTreeGlob.head[size];
    node = scrMemTreeGlob.head[size];

    if (scrMemTreeGlob.head[size])
    {
        newScore = MT_GetScore(newNode);
        nodeNum = 0;
        level = MEMORY_NODE_COUNT;
        do
        {
            iassert(newNode != node);
            score = MT_GetScore(node);

            iassert(score != newScore);

            if (score < newScore)
            {
                while (1)
                {
                    iassert(node == *parentNode);
                    iassert(node != newNode);

                    *parentNode = newNode;
                    scrMemTreeGlob.nodes[newNode] = scrMemTreeGlob.nodes[node];
                    if (!node)
                    {
                        break;
                    }
                    level >>= 1;

                    iassert(node != nodeNum);

                    if (node >= nodeNum)
                    {
                        parentNode = &scrMemTreeGlob.nodes[newNode].next;
                        nodeNum += level;
                    }
                    else
                    {
                        parentNode = &scrMemTreeGlob.nodes[newNode].prev;
                        nodeNum -= level;
                    }
                    newNode = node;
                    node = *parentNode;
                }
                return;
            }
            level >>= 1;

            iassert(newNode != nodeNum);

            if (newNode >= nodeNum)
            {
                parentNode = &scrMemTreeGlob.nodes[node].next;
                nodeNum += level;
            }
            else
            {
                parentNode = &scrMemTreeGlob.nodes[node].prev;
                nodeNum -= level;
            }

            node = *parentNode;
        } while (node);
    }

    *parentNode = newNode;

    scrMemTreeGlob.nodes[newNode].prev = 0;
    scrMemTreeGlob.nodes[newNode].next = 0;
}

void MT_Error(const char* funcName, int numBytes)
{
    MT_DumpTree();
    Com_Printf(23, "%s: failed memory allocation of %d bytes for script usage\n", funcName, numBytes);
    Com_Error(ERR_FATAL, "MT_Error (KISAK)\n");
    //Scr_TerminalError("failed memory allocation for script usage");
}

void MT_DumpTree()
{
    int mt_type_usage[22];

    memset(mt_type_usage, 0, sizeof(mt_type_usage));

    Com_Printf(23, "********************************\n");

    int totalAlloc = 0;
    int totalAllocBuckets = 0;
    int totalBuckets = 0;

    for (int nodeNum = 0; nodeNum < MEMORY_NODE_COUNT; nodeNum++)
    {
        int type = scrMemTreeDebugGlob.mt_usage[nodeNum];
        if (type)
        {
            Com_Printf(23, "%s\n", MT_NodeInfoString(nodeNum));
            ++totalAlloc;
            totalAllocBuckets += 1 << scrMemTreeDebugGlob.mt_usage_size[nodeNum];
            mt_type_usage[type] += 1 << scrMemTreeDebugGlob.mt_usage_size[nodeNum];
        }
    }

    iassert(scrMemTreeGlob.totalAlloc == totalAlloc);
    iassert(scrMemTreeGlob.totalAllocBuckets == totalAllocBuckets);

    Com_Printf(23, "********************************\n");

    totalBuckets = scrMemTreeGlob.totalAllocBuckets;

    for (int size = 0; size <= MEMORY_NODE_BITS; ++size)
    {
        int subTreeSize = MT_GetSubTreeSize(scrMemTreeGlob.head[size]);
        totalBuckets += subTreeSize * (1 << size);
        Com_Printf(
            23,
            "%d subtree has %d * %d = %d free buckets\n",
            size,
            subTreeSize,
            1 << size,
            subTreeSize * (1 << size));
    }

    Com_Printf(23, "********************************\n");
    for (int type = 1; type < 22; ++type)
        Com_Printf(23, "'%s' allocated: %d\n", mt_type_names[type], mt_type_usage[type]);
    Com_Printf(23, "********************************\n");
    Com_Printf(
        23,
        "total memory alloc buckets: %d (%d instances)\n",
        scrMemTreeGlob.totalAllocBuckets,
        scrMemTreeGlob.totalAlloc);
    Com_Printf(23, "total memory free buckets: %d\n", 0xFFFF - scrMemTreeGlob.totalAllocBuckets);
    Com_Printf(23, "********************************\n");

    iassert(totalBuckets == (1 << MEMORY_NODE_BITS) - 1);
}

char const* MT_NodeInfoString(unsigned int nodeNum)
{
    int type = scrMemTreeDebugGlob.mt_usage[nodeNum];

    if (!scrMemTreeDebugGlob.mt_usage[nodeNum])
        return "<FREE>";

    int v3 = scrMemTreeDebugGlob.mt_usage_size[nodeNum];
    const char* v1 = SL_DebugConvertToString(nodeNum);
    return va("%s: '%s' (%d)", mt_type_names[type], v1, v3);
}