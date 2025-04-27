#pragma once

template <int BIT_COUNT>
struct bitarray // sizeof=0x10
{                                       // ...
    int array[BIT_COUNT / 32];                       // ...

    void setBit(unsigned int pos)
    {
        if (pos >= BIT_COUNT)
            MyAssertHandler("c:\\trees\\cod3\\src\\xanim\\../qcommon/bitarray.h", 72, 0, "%s", "pos < BIT_COUNT");
        //this->array[pos >> 5] |= 0x80000000 >> (pos & 0x1F);
        array[pos / 32] |= 0x80000000 >> (pos & 0x1F);
    }
};