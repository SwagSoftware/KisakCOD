#pragma once
#include <cstdint>

struct MemoryNode // sizeof=0xC
{                                       // XREF: scrMemTreeGlob_t/r
    uint16_t prev;              // XREF: MT_Init(void)+46/w
    uint16_t next;              // XREF: MT_Init(void)+4E/w
    uint32_t padding[2];            // XREF: MT_RemoveHeadMemoryNode+61/w
};
static_assert(sizeof(MemoryNode) == 12);

#define MEMORY_NODE_BITS 16
#define MEMORY_NODE_COUNT 0x10000
#define NUM_BUCKETS 256

struct __declspec(align(128)) scrMemTreeGlob_t // sizeof=0xC0380
{                                       // XREF: .data:scrMemTreeGlob/r
    MemoryNode nodes[MEMORY_NODE_COUNT];            // XREF: MT_Init(void)+46/w
                                        // MT_Init(void)+4E/w ...
    uint8_t leftBits[NUM_BUCKETS];      // XREF: MT_InitBits+89/w
                                        // MT_GetScore+88/r ...
    uint8_t numBits[NUM_BUCKETS];       // XREF: MT_InitBits+59/w
                                        // MT_GetScore+6A/r ...
    uint8_t logBits[NUM_BUCKETS];       // XREF: MT_InitBits+BB/w
                                        // MT_GetSize+55/r ...
    uint16_t head[MEMORY_NODE_BITS + 1];// 0x242E200          // XREF: MT_DumpTree(void)+14B/r
                                        // MT_Init(void)+3A/w ...
    // padding byte
    // padding byte
    int totalAlloc;                     // XREF: MT_DumpTree(void):loc_59E783/r
                                        // MT_DumpTree(void)+1FB/r ...
    int totalAllocBuckets;              // XREF: MT_DumpTree(void):loc_59E7AE/r
};
static_assert(sizeof(scrMemTreeGlob_t) == 0xC0380);

static const char* mt_type_names[22] =
{
    "empty",
    "thread",
    "vector",
    "notetrack",
    "anim tree",
    "small anim tree",
    "external",
    "temp",
    "surface",
    "anim part",
    "model part",
    "model part map",
    "duplicate parts",
    "model list",
    "script parse",
    "script string",
    "class",
    "tag info",
    "animscripted",
    "config string",
    "debugger string",
    "generic",
};

int MT_GetSubTreeSize(int nodeNum);
void MT_DumpTree(void);
void MT_FreeIndex(uint32_t nodeNum, int numBytes);

void MT_Free(unsigned char* p, int numBytes);
bool MT_Realloc(int oldNumBytes, int newNumbytes);

void MT_Init(void);
unsigned short MT_AllocIndex(int numBytes, int type);
void* MT_Alloc(int numBytes, int type);

//void TRACK_scr_memorytree(void);
//uint32_t Scr_GetStringUsage(void);

char const* MT_NodeInfoString(uint32_t nodeNum);
int MT_GetScore(int num);
void MT_AddMemoryNode(int newNode, int size);
bool MT_RemoveMemoryNode(int oldNode, uint32_t size);
void MT_RemoveHeadMemoryNode(int size);
void MT_Error(char const* funcName, int numBytes);
int MT_GetSize(int numBytes);


extern scrMemTreeGlob_t scrMemTreeGlob;