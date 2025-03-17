#pragma once


struct CinematicHunk // sizeof=0x10
{                                       // ...
    void* base;
    void* atFront;
    void* atBack;
    void* end;
};