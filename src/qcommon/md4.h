#pragma once

struct MD4_CTX // sizeof=0x58
{                                       // ...
    unsigned int state[4];
    unsigned int count[2];
    unsigned __int8 buffer[64];
};

void __cdecl Com_BlockChecksum128(unsigned __int8 *buffer, unsigned int length, int key, unsigned __int8 *outChecksum);
void __cdecl Com_BlockChecksum128Cat(
    unsigned __int8 *buffer0,
    unsigned int length0,
    unsigned __int8 *buffer1,
    unsigned int length1,
    unsigned __int8 *outChecksum);

void __cdecl MD4Init(MD4_CTX *context);
void __cdecl MD4Update(MD4_CTX *context, unsigned __int8 *input, unsigned int inputLen);
void __cdecl MD4Final(unsigned __int8 *digest, MD4_CTX *context);
void __cdecl MD4Transform(unsigned int *state, unsigned __int8 *block);
