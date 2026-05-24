#pragma once
#include <cstdint>

struct MD4_CTX // sizeof=0x58
{                                       // ...
    uint32_t state[4];
    uint32_t count[2];
    uint8_t buffer[64];
};

void __cdecl Com_BlockChecksum128(uint8_t *buffer, uint32_t length, int key, uint8_t *outChecksum);
void __cdecl Com_BlockChecksum128Cat(
    uint8_t *buffer0,
    uint32_t length0,
    uint8_t *buffer1,
    uint32_t length1,
    uint8_t *outChecksum);

void __cdecl MD4Init(MD4_CTX *context);
void __cdecl MD4Update(MD4_CTX *context, uint8_t *input, uint32_t inputLen);
void __cdecl MD4Final(uint8_t *digest, MD4_CTX *context);
void __cdecl MD4Transform(uint32_t *state, uint8_t *block);
