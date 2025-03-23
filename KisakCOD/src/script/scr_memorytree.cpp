#include "scr_memorytree.h"
#include "scr_stringlist.h"

#include <win32/win_local.h>
#include <qcommon/qcommon.h>

scrMemTreePub_t scrMemTreePub;
int marker_scr_memorytree;

static scrMemTreeGlob_t scrMemTreeGlob;

struct scrMemTreeDebugGlob_t // sizeof=0x20000
{                                       // ...
    unsigned __int8 mt_usage[MEMORY_NODE_COUNT];    // ...
    unsigned __int8 mt_usage_size[MEMORY_NODE_COUNT]; // ...
};
static scrMemTreeDebugGlob_t scrMemTreeDebugGlob;

void MT_Init()
{
	// KISAKFINISHFUNCTION
	Sys_EnterCriticalSection(CRITSECT_ALLOC_MARK);

    scrMemTreePub.mt_buffer = (char*)&scrMemTreeGlob;
    MT_InitBits();

    for (int i = 0; i <= 16; ++i)
        scrMemTreeGlob.head[i] = 0;

    scrMemTreeGlob.nodes[0].prev = 0;
    scrMemTreeGlob.nodes[0].next = 0;

    for (int ia = 0; ia < 16; ++ia)
        MT_AddMemoryNode(1 << ia, ia);

    scrMemTreeGlob.totalAlloc = 0;
    scrMemTreeGlob.totalAllocBuckets = 0;
    memset(scrMemTreeDebugGlob.mt_usage, 0, sizeof(scrMemTreeDebugGlob.mt_usage));
    memset(scrMemTreeDebugGlob.mt_usage_size, 0, sizeof(scrMemTreeDebugGlob.mt_usage_size));

	Sys_LeaveCriticalSection(CRITSECT_ALLOC_MARK);
}

void MT_InitBits(void)
{
    unsigned __int8 bits; // [esp+0h] [ebp-Ch]
    unsigned __int8 bitsa; // [esp+0h] [ebp-Ch]
    unsigned __int8 bitsb; // [esp+0h] [ebp-Ch]
    int temp; // [esp+4h] [ebp-8h]
    int tempa; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    for (i = 0; i < 256; ++i)
    {
        bits = 0;
        for (temp = i; temp; temp >>= 1)
        {
            if ((temp & 1) != 0)
                ++bits;
        }
        scrMemTreeGlob.numBits[i] = bits;
        for (bitsa = 8; (i & ((1 << bitsa) - 1)) != 0; --bitsa)
            ;
        scrMemTreeGlob.leftBits[i] = bitsa;
        bitsb = 0;
        for (tempa = i; tempa; tempa >>= 1)
            ++bitsb;
        scrMemTreeGlob.logBits[i] = bitsb;
    }
}

void* MT_Alloc(int numBytes, int type)
{
    return (void*)(&scrMemTreeGlob + 12 * MT_AllocIndex(numBytes, type));
}

unsigned short MT_AllocIndex(int numBytes, int type)
{
    const char* v2; // eax
    const char* v3; // eax
    unsigned int nodeNum; // [esp+4Ch] [ebp-Ch]
    unsigned int size; // [esp+50h] [ebp-8h]
    int newSize; // [esp+54h] [ebp-4h]

    //Profile_Begin(335);
    size = MT_GetSize(numBytes);
    iassert(size >= 0 && size <= MEMORY_NODE_BITS);

    Sys_EnterCriticalSection(CRITSECT_MEMORY_TREE);
    for (newSize = size; ; ++newSize)
    {
        if (newSize > 16)
        {
            Sys_LeaveCriticalSection(CRITSECT_MEMORY_TREE);
            //Profile_EndInternal(0);
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
    //Profile_EndInternal(0);
    return nodeNum;
}

bool MT_Realloc(int oldNumBytes, int newNumbytes)
{
    return MT_GetSize(oldNumBytes) >= MT_GetSize(newNumbytes);
}

void MT_RemoveHeadMemoryNode(int size)
{
    MemoryNode* v1; // ecx
    MemoryNode* v2; // eax
    int oldNode; // [esp+0h] [ebp-28h]
    int tempNodeValue; // [esp+4h] [ebp-24h]
    unsigned int tempNodeValue_4; // [esp+8h] [ebp-20h]
    unsigned int tempNodeValue_8; // [esp+Ch] [ebp-1Ch]
    int oldNodeValue; // [esp+10h] [ebp-18h]
    unsigned int oldNodeValue_4; // [esp+14h] [ebp-14h]
    unsigned int oldNodeValue_8; // [esp+18h] [ebp-10h]
    unsigned __int16* parentNode; // [esp+1Ch] [ebp-Ch]
    int prevScore; // [esp+20h] [ebp-8h]
    int nextScore; // [esp+24h] [ebp-4h]

    iassert(size >= 0 && size <= MEMORY_NODE_BITS);

    parentNode = (unsigned __int16*)(2 * size + 37937664);
    v1 = &scrMemTreeGlob.nodes[scrMemTreeGlob.head[size]];
    oldNodeValue = *(DWORD*)&v1->prev;
    oldNodeValue_4 = v1->padding[0];
    oldNodeValue_8 = v1->padding[1];
    while ((word)oldNodeValue)
    {
        if (!HIWORD(oldNodeValue))
            goto LABEL_12;
        prevScore = MT_GetScore((unsigned __int16)oldNodeValue);
        nextScore = MT_GetScore(HIWORD(oldNodeValue));

        iassert(prevScore != nextScore);

        if (prevScore < nextScore)
        {
            oldNode = HIWORD(oldNodeValue);
            *parentNode = HIWORD(oldNodeValue);
            parentNode = (unsigned __int16*)&scrMemTreeGlob.leftBits[12 * HIWORD(oldNodeValue) - 0xBFFFE];
        }
        else
        {
        LABEL_12:
            oldNode = (unsigned __int16)oldNodeValue;
            *parentNode = oldNodeValue;
            parentNode = &scrMemTreeGlob.nodes[(unsigned __int16)oldNodeValue].prev;
        }
    LABEL_13:
        iassert(oldNode);

        tempNodeValue = oldNodeValue;
        tempNodeValue_4 = oldNodeValue_4;
        tempNodeValue_8 = oldNodeValue_8;
        v2 = &scrMemTreeGlob.nodes[oldNode];
        oldNodeValue = *(DWORD*)&v2->prev;
        oldNodeValue_4 = v2->padding[0];
        oldNodeValue_8 = v2->padding[1];
        *(DWORD*)&v2->prev = tempNodeValue;
        v2->padding[0] = tempNodeValue_4;
        v2->padding[1] = tempNodeValue_8;
    }
    oldNode = HIWORD(oldNodeValue);
    *parentNode = HIWORD(oldNodeValue);
    if (HIWORD(oldNodeValue))
    {
        parentNode = (unsigned __int16*)&scrMemTreeGlob.leftBits[12 * HIWORD(oldNodeValue) - 0xBFFFE];
        goto LABEL_13;
    }
}

void MT_FreeIndex(unsigned int nodeNum, int numBytes)
{
    const char* v2; // eax
    int size; // [esp+30h] [ebp-8h]
    int lowBit; // [esp+34h] [ebp-4h]

    //Profile_Begin(335);
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
    //Profile_EndInternal(0);
}

bool MT_RemoveMemoryNode(int oldNode, unsigned int size)
{
    MemoryNode* v2; // edx
    MemoryNode* v4; // ecx
    int node; // [esp+0h] [ebp-30h]
    int tempNodeValue; // [esp+4h] [ebp-2Ch]
    unsigned int tempNodeValue_4; // [esp+8h] [ebp-28h]
    unsigned int tempNodeValue_8; // [esp+Ch] [ebp-24h]
    int oldNodeValue; // [esp+10h] [ebp-20h]
    unsigned int oldNodeValue_4; // [esp+14h] [ebp-1Ch]
    unsigned int oldNodeValue_8; // [esp+18h] [ebp-18h]
    int nodeNum; // [esp+1Ch] [ebp-14h]
    unsigned __int8* parentNode; // [esp+20h] [ebp-10h]
    int prevScore; // [esp+24h] [ebp-Ch]
    int nextScore; // [esp+28h] [ebp-8h]
    int level; // [esp+2Ch] [ebp-4h]
    int oldNodea; // [esp+38h] [ebp+8h]

    iassert(size >= 0 && size <= MEMORY_NODE_BITS);

    nodeNum = 0;
    level = 0x10000;
    parentNode = (unsigned __int8*)&scrMemTreeGlob.head[size];
    for (node = scrMemTreeGlob.head[size]; node; node = *(unsigned __int16*)parentNode)
    {
        if (oldNode == node)
        {
            v2 = &scrMemTreeGlob.nodes[oldNode];
            oldNodeValue = *(_DWORD*)&v2->prev;
            oldNodeValue_4 = v2->padding[0];
            oldNodeValue_8 = v2->padding[1];
            while (1)
            {
                if ((_WORD)oldNodeValue)
                {
                    if (!HIWORD(oldNodeValue))
                        goto LABEL_16;
                    prevScore = MT_GetScore((unsigned __int16)oldNodeValue);
                    nextScore = MT_GetScore(HIWORD(oldNodeValue));

                    iassert(prevScore != nextScore);

                    if (prevScore < nextScore)
                    {
                        oldNodea = HIWORD(oldNodeValue);
                        *(_WORD*)parentNode = HIWORD(oldNodeValue);
                        parentNode = &scrMemTreeGlob.leftBits[12 * HIWORD(oldNodeValue) - 0xBFFFE];
                    }
                    else
                    {
                    LABEL_16:
                        oldNodea = (unsigned __int16)oldNodeValue;
                        *(_WORD*)parentNode = oldNodeValue;
                        parentNode = (unsigned __int8*)&scrMemTreeGlob.nodes[(unsigned __int16)oldNodeValue];
                    }
                }
                else
                {
                    oldNodea = HIWORD(oldNodeValue);
                    *(_WORD*)parentNode = HIWORD(oldNodeValue);
                    if (!HIWORD(oldNodeValue))
                        return 1;
                    parentNode = &scrMemTreeGlob.leftBits[12 * HIWORD(oldNodeValue) - 0xBFFFE];
                }

                iassert(oldNode);

                tempNodeValue = oldNodeValue;
                tempNodeValue_4 = oldNodeValue_4;
                tempNodeValue_8 = oldNodeValue_8;
                v4 = &scrMemTreeGlob.nodes[oldNodea];
                oldNodeValue = *(_DWORD*)&v4->prev;
                oldNodeValue_4 = v4->padding[0];
                oldNodeValue_8 = v4->padding[1];
                *(_DWORD*)&v4->prev = tempNodeValue;
                v4->padding[0] = tempNodeValue_4;
                v4->padding[1] = tempNodeValue_8;
            }
        }
        if (oldNode == nodeNum)
            return 0;
        level >>= 1;
        if (oldNode >= nodeNum)
        {
            parentNode = &scrMemTreeGlob.leftBits[12 * node - 0xBFFFE];
            nodeNum += level;
        }
        else
        {
            parentNode = (unsigned __int8*)&scrMemTreeGlob.nodes[node];
            nodeNum -= level;
        }
    }
    return 0;
}

void MT_Free(byte* p, int numBytes)
{
	iassert(((MemoryNode*)p - scrMemTreeGlob.nodes >= 0 && (MemoryNode*)p - scrMemTreeGlob.nodes < MEMORY_NODE_COUNT));

	MT_FreeIndex((p - (byte*)&scrMemTreeGlob) / 12, numBytes);
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
    unsigned __int8 bits; // [esp+8h] [ebp-8h]
    int numa; // [esp+18h] [ebp+8h]

    iassert(num);

    numa = MEMORY_NODE_COUNT - num;

    iassert(numa);

    bits = scrMemTreeGlob.leftBits[(unsigned __int8)numa];
    if (!(byte)numa)
        bits += scrMemTreeGlob.leftBits[BYTE1(numa)];
    return numa - (scrMemTreeGlob.numBits[BYTE1(numa)] + scrMemTreeGlob.numBits[(unsigned __int8)numa]) + (1 << bits);
}

int MT_GetSubTreeSize(int nodeNum)
{
    int SubTreeSize; // esi

    if (!nodeNum)
        return 0;

    SubTreeSize = MT_GetSubTreeSize(scrMemTreeGlob.nodes[nodeNum].prev);
    return SubTreeSize + MT_GetSubTreeSize(*(unsigned __int16*)&scrMemTreeGlob.leftBits[0xC * nodeNum - 0xBFFFE]) + 1;
}

void MT_AddMemoryNode(int newNode, int size)
{
    iassert(size >= 0 && size <= MEMORY_NODE_BITS);

    word* parentNode = &scrMemTreeGlob.head[size];

    int node = scrMemTreeGlob.head[size];
    if (node)
    {
        int newScore = MT_GetScore(newNode);
        int nodeNum = 0;
        int level = MEMORY_NODE_COUNT;
        int score;

        while (1)
        {
            iassert(newNode != node);
            score = MT_GetScore(node);
            iassert(score != newScore);

            if (score < newScore)
                break;

            level >>= 1;

            iassert(newNode != nodeNum);

            if (newNode >= nodeNum)
            {
                parentNode = (word*)&scrMemTreeGlob.leftBits[12 * node - 0xBFFFE];
                nodeNum += level;
            }
            else
            {
                parentNode = (word*)&scrMemTreeGlob.nodes[node];
                nodeNum -= level;
            }
            node = *parentNode;
            if (!node)
            {
                *parentNode = newNode;
                scrMemTreeGlob.nodes[newNode].prev = NULL;
                *(word*)&scrMemTreeGlob.leftBits[12 * newNode - 0xBFFFE] = 0;
            }
        }
        while (1)
        {
            iassert(node == *parentNode);
            iassert(node != newNode);

            *parentNode = newNode;

            MemoryNode* v2 = &scrMemTreeGlob.nodes[node];
            MemoryNode* v3 = &scrMemTreeGlob.nodes[newNode];

            v3->prev = v2->prev;
            v3->padding[0] = v2->padding[0];
            v3->padding[1] = v2->padding[1];

            if (!node)
                break;

            level >>= 1;

            iassert(node != nodeNum);

            if (node >= nodeNum)
            {
                parentNode = (word*)&scrMemTreeGlob.leftBits[12 * newNode - 0xBFFFE];
                nodeNum += level;
            }
            else
            {
                parentNode = (word*)&scrMemTreeGlob.nodes[newNode];
                nodeNum -= level;
            }
            newNode = node;
            node = *parentNode;
        }
    }
    else
    {
        *parentNode = newNode;
        scrMemTreeGlob.nodes[newNode].prev = NULL;
        *(word*)&scrMemTreeGlob.leftBits[12 * newNode - 0xBFFFE] = 0;
    }
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