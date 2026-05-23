#pragma once

struct MD4_CTX // sizeof=0x58
{                                       // ...
    unsigned int state[4];
    unsigned int count[2];
    uint8_t buffer[64];
};

void __cdecl Com_BlockChecksum128(uint8_t *buffer, unsigned int length, int key, uint8_t *outChecksum);
void __cdecl Com_BlockChecksum128Cat(
    uint8_t *buffer0,
    unsigned int length0,
    uint8_t *buffer1,
    unsigned int length1,
    uint8_t *outChecksum);

void __cdecl MD4Init(MD4_CTX *context);
void __cdecl MD4Update(MD4_CTX *context, uint8_t *input, unsigned int inputLen);
void __cdecl MD4Final(uint8_t *digest, MD4_CTX *context);
void __cdecl MD4Transform(unsigned int *state, uint8_t *block);
