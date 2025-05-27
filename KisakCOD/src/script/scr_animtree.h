#pragma once
#include <xanim/xanim.h>
#include <bgame/bg_local.h>

#define MAX_XANIMTREE_NUM       0x80 // 128

struct scrAnimPub_t // sizeof=0x41C
{                                       // ...
    unsigned int animtrees;             // ...
    unsigned int animtree_node;         // ...
    unsigned int animTreeNames;         // ...
    scr_animtree_t xanim_lookup[2][MAX_XANIMTREE_NUM]; // ...
    unsigned int xanim_num[2];          // ...
    unsigned int animTreeIndex;         // ...
    bool animtree_loading;              // ...
    // padding byte
    // padding byte
    // padding byte
};

struct scrAnimGlob_t // sizeof=0x20C
{                                       // ...
    const char *start;                  // ...
    const char *pos;                    // ...
    unsigned __int16 using_xanim_lookup[2][MAX_XANIMTREE_NUM]; // ...
    int bAnimCheck;                     // ...
};

void __cdecl TRACK_scr_animtree();
void __cdecl SetAnimCheck(int bAnimCheck);
void __cdecl Scr_EmitAnimation(char *pos, unsigned int animName, unsigned int sourcePos);
void __cdecl Scr_EmitAnimationInternal(char *pos, unsigned int animName, unsigned int names);
int __cdecl Scr_GetAnimsIndex(const XAnim_s *anims);
XAnim_s *__cdecl Scr_GetAnims(unsigned int index);
void __cdecl Scr_UsingTree(const char *filename, unsigned int sourcePos);
unsigned int __cdecl Scr_UsingTreeInternal(const char *filename, unsigned int *index, int user);
void __cdecl Scr_LoadAnimTreeAtIndex(unsigned int index, void *(__cdecl *Alloc)(int), int user);
int __cdecl Scr_GetAnimTreeSize(unsigned int parentNode);
void __cdecl ConnectScriptToAnim(
    unsigned int names,
    unsigned __int16 index,
    unsigned int filename,
    unsigned int name,
    unsigned __int16 treeIndex);
int __cdecl Scr_CreateAnimationTree(
    unsigned int parentNode,
    unsigned int names,
    XAnim_s *anims,
    unsigned int childIndex,
    const char *parentName,
    unsigned int parentIndex,
    unsigned int filename,
    int treeIndex,
    unsigned __int16 flags);
void __cdecl Scr_CheckAnimsDefined(unsigned int names, unsigned int filename);
bool __cdecl Scr_LoadAnimTreeInternal(const char *filename, unsigned int parentNode, unsigned int names);
void __cdecl Scr_AnimTreeParse(const char *pos, unsigned int parentNode, unsigned int names);
void __cdecl AnimTreeCompileError(const char *msg);
bool __cdecl AnimTreeParseInternal(
    unsigned int parentNode,
    unsigned int names,
    bool bIncludeParent,
    bool bLoop,
    bool bComplete);
int __cdecl GetAnimTreeParseProperties();
scr_animtree_t __cdecl Scr_FindAnimTree(const char *filename);
void __cdecl Scr_FindAnim(const char *filename, const char *animName, scr_anim_s *anim, int user);

extern scrAnimPub_t scrAnimPub;