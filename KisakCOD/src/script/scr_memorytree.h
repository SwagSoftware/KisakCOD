#pragma once

//void TRACK_scr_memorytree(void);
//unsigned int Scr_GetStringUsage(void);
//char const* MT_NodeInfoString(unsigned int);
//void MT_InitBits(void);
//int MT_GetScore(int);
//void MT_AddMemoryNode(int, int);
//bool MT_RemoveMemoryNode(int, int);
//void MT_RemoveHeadMemoryNode(int);
//void MT_Error(char const*, int);
//int MT_GetSize(int);

int MT_GetSubTreeSize(int);
void MT_DumpTree(void);
void MT_FreeIndex(unsigned int, int);

void MT_Free(void*, int);
int MT_Realloc(int, int);

void MT_Init(void);
unsigned short MT_AllocIndex(int, int);
void* MT_Alloc(int, int);