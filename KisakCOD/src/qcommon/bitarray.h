#pragma once


template <int BIT_COUNT>
struct bitarray // sizeof=0x10
{                                       // ...
    void setBit(unsigned int pos)
    {
        iassert(pos < BIT_COUNT);
        array[pos / 32] |= 0x80000000 >> (pos & 0x1F);
    }

    bool testBit(unsigned int pos) const
    {
        iassert(pos < BIT_COUNT);
        return (array[pos / 32] & (0x80000000 >> (pos & 0x1F))) != 0;
    }

//private:
    int array[BIT_COUNT / 32];                       // ...
};

#define BIT_COUNT 0x80 // bitarray size for variables such as `ignorePartBits`
