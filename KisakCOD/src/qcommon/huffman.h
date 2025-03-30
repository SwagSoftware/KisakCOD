#pragma once
#include "qcommon.h"

int __cdecl get_bit(const unsigned __int8 *fin);
void __cdecl Huff_offsetReceive(nodetype *node, int *ch, const unsigned __int8 *fin, int *offset);
void __cdecl huffman_send(nodetype *node, nodetype *child, unsigned __int8 *fout);
void __cdecl add_bit(char bit, unsigned __int8 *fout);
int __cdecl huffman_bitCountForNode(nodetype *node, nodetype *child);
int __cdecl Huff_bitCount(huff_t *huff, unsigned int ch);
void __cdecl Huff_offsetTransmit(huff_t *huff, int ch, unsigned __int8 *fout, int *offset);
void __cdecl Huff_Init(huffman_t *huff);
nodetype *__cdecl Huff_initNode(huff_t *huff, int ch, int weight);
int __cdecl nodeCmp(const void *left, const void *right);
void __cdecl Huff_BuildFromData(huff_t *huff, const int *msg_hData);
