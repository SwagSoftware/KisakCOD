#include "huffman.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>

int bloc;

int __cdecl get_bit(const unsigned __int8 *fin)
{
    int t; // [esp+0h] [ebp-4h]

    t = ((int)fin[bloc >> 3] >> (bloc & 7)) & 1;
    ++bloc;
    return t;
}

void __cdecl Huff_offsetReceive(nodetype *node, int *ch, const unsigned __int8 *fin, int *offset)
{
    bloc = *offset;
    while (node && node->symbol == 257)
    {
        if (get_bit(fin))
            node = node->right;
        else
            node = node->left;
    }
    if (node)
    {
        *ch = node->symbol;
        *offset = bloc;
    }
    else
    {
        *ch = 0;
    }
}

void __cdecl huffman_send(nodetype *node, nodetype *child, unsigned __int8 *fout)
{
    if (node->parent)
        huffman_send(node->parent, node, fout);
    if (child)
    {
        if (node->right == child)
            add_bit(1, fout);
        else
            add_bit(0, fout);
    }
}

void __cdecl add_bit(char bit, unsigned __int8 *fout)
{
    if ((bloc & 7) == 0)
        fout[bloc >> 3] = 0;
    fout[bloc >> 3] |= bit << (bloc & 7);
    ++bloc;
}

int __cdecl huffman_bitCountForNode(nodetype *node, nodetype *child)
{
    int bits; // [esp+0h] [ebp-4h]

    bits = 0;
    if (node->parent)
        bits = huffman_bitCountForNode(node->parent, node);
    if (child)
        ++bits;
    return bits;
}

int __cdecl Huff_bitCount(huff_t *huff, unsigned int ch)
{
    if (ch >= 0x100)
        MyAssertHandler(".\\qcommon\\huffman.cpp", 152, 0, "ch doesn't index 256\n\t%i not in [0, %i)", ch, 256);
    if (!huff->loc[ch])
        MyAssertHandler(".\\qcommon\\huffman.cpp", 153, 0, "%s", "huff->loc[ch] != NULL");
    return huffman_bitCountForNode(huff->loc[ch], 0);
}

void __cdecl Huff_offsetTransmit(huff_t *huff, int ch, unsigned __int8 *fout, int *offset)
{
    bloc = *offset;
    huffman_send(huff->loc[ch], 0, fout);
    *offset = bloc;
}

void __cdecl Huff_Init(huffman_t *huff)
{
    //Com_Memset((unsigned int *)huff, 0, 19476);
    Com_Memset((unsigned int *)huff, 0, sizeof(huffman_t));
    huff->compressDecompress.loc[256] = &huff->compressDecompress.nodeList[huff->compressDecompress.blocNode++];
    huff->compressDecompress.tree = huff->compressDecompress.loc[256];
    huff->compressDecompress.tree->symbol = 256;
    huff->compressDecompress.tree->weight = 0;
    huff->compressDecompress.tree->parent = 0;
    huff->compressDecompress.tree->left = 0;
    huff->compressDecompress.tree->right = 0;
}

nodetype *__cdecl Huff_initNode(huff_t *huff, int ch, int weight)
{
    nodetype *tnode; // [esp+0h] [ebp-4h]

    tnode = &huff->nodeList[huff->blocNode++];
    tnode->symbol = ch;
    tnode->weight = weight;
    tnode->left = 0;
    tnode->right = 0;
    tnode->parent = 0;
    huff->loc[ch] = tnode;
    return tnode;
}

int __cdecl nodeCmp(const void *left, const void *right)
{
    return *(unsigned int *)(*(unsigned int *)left + 12) - *(unsigned int *)(*(unsigned int *)right + 12);
}

void __cdecl Huff_BuildFromData(huff_t *huff, const int *msg_hData)
{
    nodetype *inited; // eax
    nodetype *v3; // eax
    nodetype *v4; // eax
    int numNodes; // [esp+0h] [ebp-410h]
    nodetype *heap[256]; // [esp+8h] [ebp-408h] BYREF
    int i; // [esp+408h] [ebp-8h]
    int heapHead; // [esp+40Ch] [ebp-4h]

    numNodes = 256;
    heapHead = 0;
    for (i = 0; i < 256; ++i)
    {
        inited = Huff_initNode(huff, i, msg_hData[i]);
        heap[i] = inited;
    }
    qsort(heap, 0x100u, 4u, nodeCmp);
    v3 = Huff_initNode(huff, 257, 1);
    v3->left = huff->tree;
    v3->right = heap[0];
    v3->left->parent = v3;
    v3->right->parent = v3;
    v3->weight = v3->right->weight + v3->left->weight;
    heap[0] = v3;
    while (numNodes > 1)
    {
        qsort(&heap[heapHead], 256 - heapHead, 4u, nodeCmp);
        v4 = Huff_initNode(huff, 257, 1);
        v4->left = heap[heapHead];
        v4->right = heap[heapHead + 1];
        v4->left->parent = v4;
        v4->right->parent = v4;
        v4->weight = v4->right->weight + v4->left->weight;
        heap[++heapHead] = v4;
        --numNodes;
    }
    huff->tree = heap[heapHead];
}

