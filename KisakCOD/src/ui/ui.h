#pragma once
#include <xanim/xanim.h>
#include <ui_mp/ui_mp.h>
#include <physics/ode/objects.h>
#include <bgame/bg_local.h>

struct Scr_WatchElement_s;

enum parseSkip_t : __int32
{                                       // ...
    SKIP_NO = 0x0,
    SKIP_YES = 0x1,
    SKIP_ALL_ELIFS = 0x2,
};

struct directive_s // sizeof=0x8
{                                       // ...
    char *name;                         // ...
    int(__cdecl *func)(struct source_s *);    // ...
};

struct punctuation_s // sizeof=0xC
{
    char *p;
    int n;
    punctuation_s *next;
};

struct pc_token_s // sizeof=0x410
{                                       // ...
    int type;                           // ...
    int subtype;
    int intvalue;                       // ...
    float floatvalue;                   // ...
    char string[1024];                  // ...
};

struct loadAssets_t // sizeof=0x10
{                                       // ...
    float fadeClamp;                    // ...
    int fadeCycle;                      // ...
    float fadeAmount;                   // ...
    float fadeInAmount;                 // ...
};

struct $F99A9AECA2B60514CA5C8024B8EAC369 // sizeof=0xC1C
{                                       // ...
    loadAssets_t loadAssets;            // ...
    MenuList menuList;                  // ...
    itemDef_s *items[256];              // ...
    menuDef_t *menus[512];              // ...
};

struct operator_s // sizeof=0x14
{                                       // ...
    int op;
    int priority;
    int parentheses;
    operator_s *prev;
    operator_s *next;
};
struct __declspec(align(8)) token_s // sizeof=0x430
{                                       // ...
    char string[1024];                  // ...
    int type;                           // ...
    int subtype;                        // ...
    unsigned int intvalue;              // ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    long double floatvalue;             // ...
    char *whitespace_p;                 // ...
    char *endwhitespace_p;              // ...
    int line;                           // ...
    int linescrossed;                   // ...
    token_s *next;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
struct __declspec(align(8)) value_s // sizeof=0x20
{                                       // ...
    int intvalue;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    long double floatvalue;
    int parentheses;
    value_s *prev;
    value_s *next;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
struct __declspec(align(8)) script_s // sizeof=0x4B0
{
    char filename[64];
    char *buffer;
    char *script_p;
    char *end_p;
    char *lastscript_p;
    char *whitespace_p;
    char *endwhitespace_p;
    int length;
    int line;
    int lastline;
    int tokenavailable;
    int flags;
    punctuation_s *punctuations;
    punctuation_s **punctuationtable;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    token_s token;
    script_s *next;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};
struct define_s // sizeof=0x20
{
    char *name;
    int flags;
    int builtin;
    int numparms;
    token_s *parms;
    token_s *tokens;
    define_s *next;
    define_s *hashnext;
};
struct indent_s // sizeof=0x10
{
    int type;
    parseSkip_t skip;
    script_s *script;
    indent_s *next;
};
struct source_s // sizeof=0x4D0
{                                       // ...
    char filename[64];
    char includepath[64];
    punctuation_s *punctuations;
    script_s *scriptstack;              // ...
    token_s *tokens;                    // ...
    define_s *defines;                  // ...
    define_s **definehash;              // ...
    indent_s *indentstack;
    int skip;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    token_s token;
};


enum EvalValueType : __int32
{                                       // ...
    EVAL_VALUE_DOUBLE = 0x0,
    EVAL_VALUE_INT = 0x1,
    EVAL_VALUE_STRING = 0x2,
};
enum EvalOperatorType : __int32
{                                       // ...
    EVAL_OP_LPAREN = 0x0,
    EVAL_OP_RPAREN = 0x1,
    EVAL_OP_COLON = 0x2,
    EVAL_OP_QUESTION = 0x3,
    EVAL_OP_PLUS = 0x4,
    EVAL_OP_MINUS = 0x5,
    EVAL_OP_UNARY_PLUS = 0x6,
    EVAL_OP_UNARY_MINUS = 0x7,
    EVAL_OP_MULTIPLY = 0x8,
    EVAL_OP_DIVIDE = 0x9,
    EVAL_OP_MODULUS = 0xA,
    EVAL_OP_LSHIFT = 0xB,
    EVAL_OP_RSHIFT = 0xC,
    EVAL_OP_BITWISE_NOT = 0xD,
    EVAL_OP_BITWISE_AND = 0xE,
    EVAL_OP_BITWISE_OR = 0xF,
    EVAL_OP_BITWISE_XOR = 0x10,
    EVAL_OP_LOGICAL_NOT = 0x11,
    EVAL_OP_LOGICAL_AND = 0x12,
    EVAL_OP_LOGICAL_OR = 0x13,
    EVAL_OP_EQUALS = 0x14,
    EVAL_OP_NOT_EQUAL = 0x15,
    EVAL_OP_LESS = 0x16,
    EVAL_OP_LESS_EQUAL = 0x17,
    EVAL_OP_GREATER = 0x18,
    EVAL_OP_GREATER_EQUAL = 0x19,
    EVAL_OP_COUNT = 0x1A,
};
EvalOperatorType operator-=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) - static_cast<int>(rhs));
}
EvalOperatorType operator+=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) + static_cast<int>(rhs));
}
EvalOperatorType operator/=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) / static_cast<int>(rhs));
}
EvalOperatorType operator*=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) * static_cast<int>(rhs));
}
EvalOperatorType operator<<=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) << static_cast<int>(rhs));
}
EvalOperatorType operator>>=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) >> static_cast<int>(rhs));
}
EvalOperatorType operator%=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) % static_cast<int>(rhs));
}
EvalOperatorType operator&=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) & static_cast<int>(rhs));
}
EvalOperatorType operator|=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) | static_cast<int>(rhs));
}
EvalOperatorType operator^=(EvalOperatorType lhs, EvalOperatorType rhs)
{
    return (EvalOperatorType)(static_cast<int>(lhs) ^ static_cast<int>(rhs));
}

union EvalValue_u // sizeof=0x8
{                                       // ...
    double d;
    int i;
    char *s;
};
struct EvalValue // sizeof=0x10
{                                       // ...
    EvalValueType type;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    EvalValue_u u;
};

struct __declspec(align(4)) Eval // sizeof=0x5010
{                                       // ...
    EvalOperatorType opStack[1024];
    EvalValue valStack[1024];
    int opStackPos;                     // ...
    int valStackPos;                    // ...
    int parenCount;                     // ...
    bool pushedOp;                      // ...
    // padding byte
    // padding byte
    // padding byte
};

// ui_shared
bool __cdecl Window_IsVisible(int localClientNum, const windowDef_t *w);
void __cdecl Menu_Setup(UiContext *dc);
void __cdecl LerpColor(float *a, float *b, float *c, float t);
int __cdecl Color_Parse(const char **p, float (*c)[4]);
int __cdecl String_Parse(const char **p, char *out, int len);
void __cdecl Script_SetColor(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_SetBackground(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_SetItemColor(UiContext *dc, itemDef_s *item, const char **args=NULL);
int __cdecl Menu_ItemsMatchingGroup(menuDef_t *menu, char *name);
itemDef_s *__cdecl Menu_GetMatchingItemByNumber(menuDef_t *menu, int index, char *name);
int __cdecl Menus_MenuIsInStack(UiContext *dc, menuDef_t *menu);
menuDef_t *__cdecl Menus_FindByName(const UiContext *dc, const char *p);
void __cdecl Menus_HideByName(const UiContext *dc, const char *menuName);
void __cdecl Menus_ShowByName(const UiContext *dc, const char *windowName);
void __cdecl Menus_CloseByName(UiContext *dc, const char *p);
void __cdecl Menus_Close(UiContext *dc, menuDef_t *menu);
void __cdecl Menus_FreeAllMemory(UiContext *dc);
void __cdecl Menu_FreeMemory(menuDef_t *menu);
bool __cdecl Window_HasFocus(int localClientNum, const windowDef_t *w);
int __cdecl Menus_RemoveFromStack(UiContext *dc, menuDef_t *pMenu);
void __cdecl Menu_GainFocusDueToClose(UiContext *dc, menuDef_t *menu);
void __cdecl Menu_CallOnFocusDueToOpen(UiContext *dc, menuDef_t *menu);
void __cdecl Menu_RunCloseScript(UiContext *dc, menuDef_t *menu);
itemDef_s *__cdecl Menu_FindItemByName(menuDef_t *menu, const char *p);
void __cdecl Menus_CloseAll(UiContext *dc);
void __cdecl Script_Show(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Menu_ShowItemByName(int localClientNum, menuDef_t *menu, char *p, int bShow);
void __cdecl Script_Hide(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_FadeIn(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Menu_FadeItemByName(int localClientNum, menuDef_t *menu, char *p, int fadeOut);
void __cdecl Script_FadeOut(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_ShowMenu(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_HideMenu(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_Open(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_OpenForGameType(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_CloseForGameType(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_StatSetUsingStatsTable(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_StatClearBitMask(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_StatBitMaskGetArgs(UiContext *dc, itemDef_s *item, const char **args, int *statNum, int *bitMask);
void __cdecl Script_Close(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_InGameOpen(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_InGameClose(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_FocusFirstInMenu(UiContext *dc, itemDef_s *item, const char **args=NULL);
itemDef_s *__cdecl Menu_FocusFirstSelectableItem(UiContext *dc, menuDef_t *menu);
void __cdecl Script_SetFocus(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_SetFocusByDvar(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_SetDvar(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_Exec(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_ExecHandler(
    int localClientNum,
    int controllerIndex,
    itemDef_s *item,
    const char **args,
    void(__cdecl *textCallback)(int, int, const char *));
void __cdecl Script_AddTextWrapper(int clientNum, int controllerIndex, const char *text);
void __cdecl Script_ExecNow(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_ExecOnDvarStringValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_ConditionalExecHandler(
    int localClientNum,
    itemDef_s *item,
    const char **args,
    bool(__cdecl *shouldExec)(const char *, const char *),
    void(__cdecl *textCallback)(int, int, const char *));
bool __cdecl Script_ExecIfStringsEqual(const char *dvarValue, const char *testValue);
void __cdecl Script_ExecOnDvarIntValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
bool __cdecl Script_ExecIfIntsEqual(const char *dvarValue, const char *testValue);
void __cdecl Script_ExecOnDvarFloatValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
bool __cdecl Script_ExecIfFloatsEqual(const char *dvarValue, const char *testValue);
void __cdecl Script_ExecNowOnDvarStringValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_ExecNowOnDvarIntValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_ExecNowOnDvarFloatValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_RespondOnDvarStringValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_ConditionalResponseHandler(
    int localClientNum,
    itemDef_s *item,
    const char **args,
    bool(__cdecl *shouldRespond)(const char *, const char *));
void __cdecl Script_RespondOnDvarIntValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_RespondOnDvarFloatValue(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_SetLocalVarBool(UiContext *dc, itemDef_s *item, const char **args=NULL);
UILocalVarContext *__cdecl Script_ParseLocalVar(UiContext *dc, const char **args=NULL);
void __cdecl Script_SetLocalVarInt(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_SetLocalVarFloat(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_SetLocalVarString(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_FeederTop(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_FeederBottom(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_Play(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_ScriptMenuResponse(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_StatClearPerkNew(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Script_GetAutoUpdate(UiContext *dc, itemDef_s *item, const char **args=NULL);
void __cdecl Item_RunScript(UiContext *dc, itemDef_s *item, char *s);
int __cdecl Item_SetFocus(UiContext *dc, itemDef_s *item, float x, float y);
const rectDef_s *__cdecl Item_GetTextRect(int localClientNum, const itemDef_s *item);
itemDef_s *__cdecl Menu_ClearFocus(UiContext *dc, menuDef_t *menu);
bool __cdecl Rect_ContainsPoint(int localClientNum, const rectDef_s *rect, float x, float y);
int __cdecl Item_ListBox_MaxScroll(int localClientNum, itemDef_s *item);
int __cdecl Item_ListBox_Viewmax(itemDef_s *item);
void __cdecl Item_ListBox_SetCursorPos(int localClientNum, itemDef_s *item, int viewmax, int newCursorPos);
bool __cdecl Item_IsTextField(const itemDef_s *item);
void __cdecl Item_TextField_BeginEdit(int localClientNum, itemDef_s *item);
void __cdecl Menus_Open(UiContext *dc, menuDef_t *menu);
void __cdecl Menus_AddToStack(UiContext *dc, menuDef_t *pMenu);
void __cdecl Menu_LoseFocusDueToOpen(UiContext *dc, menuDef_t *menu);
int __cdecl Menus_OpenByName(UiContext *dc, const char *p);
void __cdecl Menus_PrintAllLoadedMenus(UiContext *dc);
int __cdecl Display_MouseMove(UiContext *dc);
void __cdecl Menu_HandleKey(UiContext *dc, menuDef_t *menu, int key, int down);
bool __cdecl Item_TextField_HandleKey(UiContext *dc, itemDef_s *item, int key);
void __cdecl Item_TextField_EnsureCursorVisible(int localClientNum, itemDef_s *item, const char *text);
int Item_HandleKey(UiContext *dc, itemDef_s *item, int key, int down);
int __cdecl Item_OwnerDraw_HandleKey(itemDef_s *item, int key);
int __cdecl Item_ListBox_HandleKey(UiContext *dc, itemDef_s *item, int key, int down, int force);
void __cdecl Item_ListBox_Page(int localClientNum, itemDef_s *item, int max, int scrollmax, int viewmax, int delta);
void __cdecl Item_ListBox_Scroll(int localClientNum, itemDef_s *item, int max, int scrollmax, int viewmax, int delta);
int __cdecl Item_YesNo_HandleKey(UiContext *dc, itemDef_s *item, int key);
bool __cdecl Item_ShouldHandleKey(UiContext *dc, itemDef_s *item, int key);
int __cdecl Item_Multi_HandleKey(UiContext *dc, itemDef_s *item, int key);
int __cdecl Item_Multi_CountSettings(itemDef_s *item);
int __cdecl Item_Multi_FindDvarByValue(itemDef_s *item);
int __cdecl Item_List_NextEntryForKey(int key, int current, int count);
int __cdecl Item_DvarEnum_HandleKey(UiContext *dc, itemDef_s *item, int key);
int __cdecl Item_DvarEnum_CountSettings(itemDef_s *item);
int __cdecl Item_DvarEnum_EnumIndex(itemDef_s *item);
double __cdecl Item_Slider_ThumbPosition(int localClientNum, itemDef_s *item);
double __cdecl Item_GetRectPlacementX(int alignX, float x0, float containerWidth, float selfWidth);
double __cdecl Item_ListBox_ThumbPosition(int localClientNum, itemDef_s *item);
int __cdecl Item_Slider_HandleKey(UiContext *dc, itemDef_s *item, int key, int down);
void __cdecl Item_Action(UiContext *dc, itemDef_s *item);
itemDef_s *__cdecl Menu_SetPrevCursorItem(UiContext *dc, menuDef_t *menu);
itemDef_s *__cdecl Menu_SetNextCursorItem(UiContext *dc, menuDef_t *menu);
rectDef_s *__cdecl Item_CorrectedTextRect(int localClientNum, itemDef_s *item);
int __cdecl Menu_CheckOnKey(UiContext *dc, menuDef_t *menu, int key);
void __cdecl UI_DrawWrappedText(
    const ScreenPlacement *scrPlace,
    const char *text,
    const rectDef_s *rect,
    Font_s *font,
    float x,
    float y,
    float scale,
    const float *color,
    int style,
    char textAlignMode,
    rectDef_s *textRect);
void __cdecl DrawWrappedText(
    const ScreenPlacement *scrPlace,
    const char *text,
    const rectDef_s *rect,
    Font_s *font,
    float x,
    float y,
    float scale,
    const float *color,
    int style,
    char textAlignMode,
    rectDef_s *textRect,
    bool subtitle,
    const float *subtitleGlowColor,
    bool cinematic);
double __cdecl Item_GetTextPlacementX(int alignX, float x0, float containerWidth, float selfWidth);
double __cdecl Item_GetTextPlacementY(int alignY, float y0, float containerHeight, float selfHeight);
int __cdecl UI_PickWordWrapLineWidth(
    const char *text,
    int bufferSize,
    Font_s *font,
    float normalizedScale,
    int targetLineCount,
    int widthGuess,
    int widthLimit);
void __cdecl UI_DrawWrappedTextSubtitled(
    const ScreenPlacement *scrPlace,
    const char *text,
    const rectDef_s *rect,
    Font_s *font,
    float x,
    float y,
    float scale,
    const float *color,
    int style,
    char textAlignMode,
    rectDef_s *textRect,
    const float *subtitleGlowColor,
    bool cinematic);
int __cdecl UI_GetKeyBindingLocalizedString(int localClientNum, const char *command, char *keys);
int __cdecl Display_KeyBindPending();
int __cdecl Item_Bind_HandleKey(UiContext *dc, itemDef_s *item, int key, int down);
menuDef_t *__cdecl Menu_GetFocused(UiContext *dc);
void __cdecl Menu_SetFeederSelection(UiContext *dc, menuDef_t *menu, int feeder, int index, const char *name);
int __cdecl Menus_AnyFullScreenVisible(UiContext *dc);
char __cdecl Menu_IsVisible(UiContext *dc, menuDef_t *menu);
char __cdecl Menu_Paint(UiContext *dc, menuDef_t *menu);
void __cdecl Window_Paint(
    UiContext *dc,
    windowDef_t *w,
    float fadeAmount,
    float fadeInAmount,
    float fadeClamp,
    float fadeCycle);
void __cdecl Fade(
    int *flags,
    float *f,
    float clamp,
    int *nextTime,
    int offsetTime,
    int bFlags,
    float fadeAmount,
    float fadeInAmount,
    UiContext *dc);
void __cdecl Item_Paint(UiContext *dc, itemDef_s *item);
void __cdecl Item_Text_Paint(UiContext *dc, itemDef_s *item);
void __cdecl Item_SetTextExtents(int localClientNum, itemDef_s *item, const char *text);
void __cdecl ToWindowCoords(float *x, float *y, const windowDef_t *window);
void __cdecl Item_TextColor(UiContext *dc, itemDef_s *item, float (*newColor)[4]);
void __cdecl Item_Text_AutoWrapped_Paint(
    int localClientNum,
    itemDef_s *item,
    const char *text,
    const float *color,
    bool subtitle,
    const float *subtitleGlowColor,
    bool cinematic);
void __cdecl Item_TextField_Paint(UiContext *dc, itemDef_s *item);
void __cdecl Item_YesNo_Paint(UiContext *dc, itemDef_s *item);
void __cdecl Item_Multi_Paint(UiContext *dc, itemDef_s *item);
const char *__cdecl Item_Multi_Setting(itemDef_s *item);
void __cdecl Item_DvarEnum_Paint(UiContext *dc, itemDef_s *item);
const char *__cdecl Item_DvarEnum_Setting(itemDef_s *item);
void __cdecl Item_Slider_Paint(UiContext *dc, itemDef_s *item);
double __cdecl Item_GetRectPlacementY(int alignY, float y0, float containerHeight, float selfHeight);
void __cdecl Item_Bind_Paint(UiContext *dc, itemDef_s *item);
void __cdecl Item_ListBox_Paint(UiContext *dc, itemDef_s *item);
double __cdecl Item_ListBox_ThumbDrawPosition(UiContext *dc, itemDef_s *item);
void __cdecl Item_ListBox_PaintTextElem(
    int localClientNum,
    itemDef_s *item,
    int row,
    int col,
    int maxChars,
    int alignment,
    float x,
    float y,
    float w,
    float h);
double __cdecl Item_GetTextAlignAdj(int alignment, float width, float textWidth);
void __cdecl Item_ListBox_PaintBackground(int localClientNum, itemDef_s *item, float x, float y);
void __cdecl Item_ListBox_PaintHighlight(int localClientNum, itemDef_s *item, float x, float y);
void __cdecl Item_OwnerDraw_Paint(UiContext *dc, itemDef_s *item);
void __cdecl Item_GameMsgWindow_Paint(UiContext *dc, itemDef_s *item);
int __cdecl Menu_Count(UiContext *dc);
void __cdecl Menu_PaintAll_BeginVisibleList(char *stringBegin, unsigned int stringSize);
void __cdecl Menu_PaintAll_AppendToVisibleList(char *stringBegin, unsigned int stringSize, char *stringToAppend);
void __cdecl Menu_PaintAll_DrawVisibleList(char *stringBegin, UiContext *dc);
void __cdecl Menu_PaintAll(UiContext *dc);
void __cdecl TRACK_ui_shared();
void __cdecl UI_AddMenuList(UiContext *dc, MenuList *menuList);
void __cdecl UI_AddMenu(UiContext *dc, menuDef_t *menu);
int __cdecl UI_PlayLocalSoundAliasByName(unsigned int localClientNum, const char *aliasname);
int __cdecl UI_GetMenuScreen();
int __cdecl UI_GetForcedMenuScreen();
int __cdecl UI_GetMenuScreenForError();
MenuList *__cdecl UI_LoadMenu(char *menuFile, int imageTrack);
MenuList *__cdecl UI_LoadMenus(char *menuFile, int imageTrack);
MenuList *__cdecl UI_LoadMenus_FastFile(const char *menuFile);


// ui_localvars
void __cdecl UILocalVar_Init(UILocalVarContext *context);
void __cdecl UILocalVar_Shutdown(UILocalVarContext *context);
UILocalVarContext *__cdecl UILocalVar_Find(UILocalVarContext *context, const char *name);
char __cdecl UILocalVar_FindLocation(UILocalVarContext *context, const char *name, unsigned int *hashForName);
unsigned int __cdecl UILocalVar_HashName(const char *name);
UILocalVarContext *__cdecl UILocalVar_FindOrCreate(UILocalVarContext *context, char *name);
bool __cdecl UILocalVar_GetBool(const UILocalVar *var);
UILocalVar_u __cdecl UILocalVar_GetInt(const UILocalVar *var);
double __cdecl UILocalVar_GetFloat(const UILocalVar *var);
char *__cdecl UILocalVar_GetString(const UILocalVar *var, char *stringBuf, unsigned int size);
void __cdecl UILocalVar_SetBool(UILocalVar *var, bool b);
void __cdecl UILocalVar_SetInt(UILocalVar *var, int i);
void __cdecl UILocalVar_SetFloat(UILocalVar *var, float f);
void __cdecl UILocalVar_SetString(UILocalVar *var, char *s);



// ui_utils
struct stringDef_s // sizeof=0x8
{
    stringDef_s *next;
    const char *str;
};
void __cdecl TRACK_ui_utils();
void __cdecl Window_SetDynamicFlags(int localClientNum, windowDef_t *w, int flags);
void __cdecl Window_AddDynamicFlags(int localClientNum, windowDef_t *w, int newFlags);
void __cdecl Window_RemoveDynamicFlags(int localClientNum, windowDef_t *w, int newFlags);
void __cdecl Window_SetStaticFlags(windowDef_t *w, int flags);
void __cdecl Menu_SetCursorItem(int localClientNum, menuDef_t *menu, int cursorItem);
int __cdecl Item_IsVisible(int localClientNum, itemDef_s *item);
bool __cdecl Item_EnableShowViaDvar(const itemDef_s *item, int flag);
void __cdecl Item_SetTextRect(int localClientNum, itemDef_s *item, const rectDef_s *textRect);
int __cdecl Item_GetCursorPosOffset(int localClientNum, const itemDef_s *item, const char *text, int delta);
bool __cdecl ListBox_HasValidCursorPos(int localClientNum, itemDef_s *item);
void __cdecl Menu_UpdatePosition(int localClientNum, menuDef_t *menu);
void __cdecl Item_SetScreenCoords(int localClientNum, itemDef_s *item, float x, float y, int horzAlign, int vertAlign);
int __cdecl Item_IsEditFieldDef(itemDef_s *item);
listBoxDef_s *__cdecl Item_GetListBoxDef(itemDef_s *item);
editFieldDef_s *__cdecl Item_GetEditFieldDef(itemDef_s *item);
multiDef_s *__cdecl Item_GetMultiDef(itemDef_s *item);
unsigned __int8 *__cdecl UI_Alloc(unsigned int size, int alignment);
void __cdecl String_Init();
const char *__cdecl String_Alloc(const char *p);
int __cdecl hashForString(const char *str);
int __cdecl Int_Parse(const char **p, int *i);
int __cdecl Float_Parse(const char **p, float *f);




// ui_expressions
struct OperandList // sizeof=0x54
{                                       // ...
    Operand operands[10];
    int operandCount;
};
struct OperandStack // sizeof=0x13B4
{
    OperandList stack[60];
    int numOperandLists;
};
struct OperatorStack // sizeof=0xF4
{                                       // ...
    operationEnum stack[60];            // ...
    int numOperators;                   // ...
};
int __cdecl GetKeyBindingLocalizedString(int localClientNum, const char *command, char *keys, bool single);
char *__cdecl GetSourceString(Operand operand);
double __cdecl GetSourceFloat(Operand *source);
operandInternalDataUnion __cdecl GetSourceInt(Operand *source);
void(__cdecl *__cdecl GetOperationFunction(
    operationEnum op,
    Operand *data1,
    Operand *data2))(Operand *, Operand *, Operand *);
bool __cdecl OpPairsWithRightParen(operationEnum op);
void __cdecl RunLogicOp(
    int localClientNum,
    operationEnum op,
    OperandStack *dataStack,
    Operand data1,
    Operand data2,
    const char *opDescription);
const char *__cdecl GetNameForValueType(expDataType valType);
void __cdecl AddOperandToStack(OperandStack *dataStack, Operand *data);
char __cdecl GetOperand(OperandStack *dataStack, Operand *data);
char __cdecl GetTwoOperands(OperandStack *dataStack, Operand *data1, Operand *data2);
char __cdecl GetOperandList(OperandStack *dataStack, OperandList *list);
void __cdecl RunOp(int localClientNum, OperatorStack *opStack, OperandStack *dataStack);
void __cdecl GetDvarStringValue(Operand *source, Operand *result);
char *__cdecl CopyDvarString(const char *string);
void __cdecl GetDvarBoolValue(Operand *source, Operand *result);
void __cdecl GetDvarIntValue(Operand *source, Operand *result);
void __cdecl GetDvarFloatValue(Operand *source, Operand *result);
void __cdecl GetLocalVarStringValue(
    int localClientNum,
    Operand *source,
    Operand *result,
    char *stringBuf,
    unsigned int size);
UILocalVarContext *__cdecl GetLocalVar(int localClientNum, Operand *source);
void __cdecl GetLocalVarBoolValue(int localClientNum, Operand *source, Operand *result);
void __cdecl GetLocalVarIntValue(int localClientNum, Operand *source, Operand *result);
void __cdecl GetLocalVarFloatValue(int localClientNum, Operand *source, Operand *result);
void __cdecl GetSinValue(Operand *source, Operand *result);
void __cdecl GetCosValue(Operand *source, Operand *result);
void __cdecl GetMilliseconds(Operand *result);
void __cdecl GetPlayerField(int localClientNum, Operand *source, Operand *result);
void __cdecl GetOtherTeamField(int localClientNum, Operand *fieldName, Operand *result);
void __cdecl GetFieldForTeam(int localClientNum, team_t team, Operand *fieldName, Operand *result);
void __cdecl GetTeamField(int localClientNum, Operand *fieldName, Operand *result);
void __cdecl GetTeamMarinesField(int localClientNum, Operand *fieldName, Operand *result);
void __cdecl GetTeamOpForField(int localClientNum, Operand *fieldName, Operand *result);
void __cdecl GetUIActive(int localClientNum, Operand *result);
void __cdecl GetFlashbanged(int localClientNum, Operand *result);
void __cdecl GetScoped(int localClientNum, Operand *result);
void __cdecl InKillcam(int localClientNum, Operand *result);
void __cdecl GetScoreboardVisible(int localClientNum, Operand *result);
void __cdecl GetSelectingLocation(int localClientNum, Operand *result);
void __cdecl PrivatePartyHostInLobby(int localClientNum, Operand *result);
void __cdecl AloneInPrivateParty(int localClientNum, Operand *result);
void __cdecl InLobby(int localClientNum, Operand *result);
void __cdecl InPrivateParty(int localClientNum, Operand *result);
void __cdecl PrivatePartyHost(int localClientNum, Operand *result);
void __cdecl GetPlayerStat(int localClientNum, Operand *source, Operand *result);
operandInternalDataUnion __cdecl getOperandValueInt(Operand *source);
void __cdecl GetPlayerStatRangeBitsSet(int localClientNum, OperandList *list, Operand *result);
void __cdecl GetKeyBinding(int localClientNum, Operand *fieldName, Operand *result);
void __cdecl GetActionSlotUsable(int localClientNum, Operand *fieldName, Operand *result);
void __cdecl GetHudFade(int localClientNum, Operand *fieldName, Operand *result);
void __cdecl UI_GetOnlineFriendCount(int localClientNum, Operand *result);
void __cdecl IsMenuOpen(int localClientNum, Operand *source, Operand *result);
void __cdecl WritingData(int localClientNum, Operand *result);
void __cdecl LogicalNot(int localClientNum, Operand *source, Operand *result);
void __cdecl BitwiseNot(int localClientNum, Operand *source, Operand *result);
void __cdecl BitShiftLeft(int localClientNum, Operand *source, Operand *bitsSource, Operand *result);
void __cdecl BitShiftRight(int localClientNum, Operand *source, Operand *bitsSource, Operand *result);
void __cdecl GetAdsJavelin(int localClientNum, Operand *result);
void __cdecl GetWeapLockBlink(int localClientNum, Operand *source, Operand *result);
void __cdecl GetWeapAttackTop(int localClientNum, Operand *result);
void __cdecl GetWeapAttackDirect(int localClientNum, Operand *result);
void __cdecl SecondsToTimeDisplay(int localClientNum, Operand *source, Operand *result);
void __cdecl SecondsToCountdownDisplay(int localClientNum, int seconds, Operand *result);
void __cdecl GetTimeLeft(int localClientNum, Operand *result);
void __cdecl GetGametypeObjective(int localClientNum, Operand *result);
void __cdecl GetGametypeName(int localClientNum, Operand *result);
void __cdecl GetGametypeInternal(int localClientNum, Operand *result);
void __cdecl GetScore(int localClientNum, Operand *source, Operand *result);
void __cdecl GetGameMessageWindowActive(int localClientNum, Operand *source, Operand *result);
void __cdecl GetFollowing(int localClientNum, Operand *result);
void __cdecl RunCommaOp(int localClientNum, OperandStack *dataStack, OperandList *list1, OperandList *list2);
void __cdecl TableLookup(int localClientNum, OperandList *list, Operand *operandResult);
void __cdecl MinValue(OperandList *list, Operand *operandResult);
void __cdecl MaxValue(OperandList *list, Operand *operandResult);
void __cdecl LocalizeString(OperandList *list, Operand *operandResult);
void __cdecl LocalizationError(const char *errorMessage);
void __cdecl ValidateLocalizedStringRef(const char *token, int tokenLen);
void __cdecl RunHigherPriorityOperators(
    int localClientNum,
    operationEnum op,
    OperatorStack *opStack,
    OperandStack *dataStack);
bool __cdecl IsOpAssociative(operationEnum op);
char *__cdecl GetExpressionResultString(int localClientNum, const statement_s *statement);
Operand *__cdecl EvaluateExpression(int localClientNum, const statement_s *statement, Operand *result);
bool __cdecl IsExpressionTrue(int localClientNum, const statement_s *statement);
double __cdecl GetExpressionFloat(int localClientNum, const statement_s *statement);



// ui_atoms
void __cdecl UI_DrawHandlePic(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    const float *color,
    Material *material);
void __cdecl UI_DrawLoadBar(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float w,
    float h,
    int horzAlign,
    int vertAlign,
    const float *color,
    Material *material);
double __cdecl UI_LoadBarProgress_FastFile();
void __cdecl UI_FillRectPhysical(float x, float y, float width, float height, const float *color);
void __cdecl UI_FillRect(
    const ScreenPlacement *scrPlace,
    float x,
    float y,
    float width,
    float height,
    int horzAlign,
    int vertAlign,
    const float *color);

extern int com_expectedHunkUsage;



// ui_expressions_logicfunctions
int __cdecl compare_impact_files(const char **pe0, const char **pe1);
int __cdecl compare_hudelems(const void *pe0, const void *pe1);
int __cdecl compare_use(float *pe1, float *pe2);
void __cdecl compare_doesStringEqualString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesStringNotEqualString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesIntEqualInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesIntNotEqualInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesIntEqualFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesFloatEqualInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesFloatEqualFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesIntNotEqualFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesFloatNotEqualInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_doesFloatNotEqualFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isIntLessThanInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isIntLessThanFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isFloatLessThanInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isFloatLessThanFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isIntLessThanEqualToInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isFloatGreaterThanEqualToInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isIntLessThanEqualToFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isFloatLessThanEqualToInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isFloatLessThanEqualToFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isIntGreaterThanInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isIntGreaterThanFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isFloatGreaterThanInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isFloatGreaterThanFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isIntGreaterThanEqualToInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isIntGreaterThanEqualToFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl compare_isFloatGreaterThanEqualToFloat(Operand *leftSide, Operand *rightSide, Operand *result);
int __cdecl compare_floats(float *e0, float *e1);

void __cdecl add_bit(char bit, unsigned __int8 *fout);
void __cdecl add_IntWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl add_IntWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl add_FloatWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl add_FloatWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl add_StringWithString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl add_StringWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl add_IntWithString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl add_FloatWithString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl add_StringWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);

void __cdecl subtract_IntFromInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl subtract_FloatFromInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl subtract_IntFromFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl subtract_FloatFromFloat(Operand *leftSide, Operand *rightSide, Operand *result);

void __cdecl BitwiseNot(int localClientNum, Operand *source, Operand *result);
void __cdecl multiply_IntByInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl multiply_IntByFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl multiply_FloatByInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl multiply_FloatByFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl divide_IntByInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl divide_IntByFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl divide_FloatByInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl divide_FloatByFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl mod_IntByInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl mod_FloatByInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl mod_IntByFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl mod_FloatByFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl and_IntWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl and_FloatWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl and_IntWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl and_StringWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl and_IntWithString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl and_StringWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl and_FloatWithString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl and_FloatWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl or_IntWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl or_FloatWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl or_IntWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl or_StringWithInt(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl or_IntWithString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl or_StringWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl or_FloatWithString(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl or_FloatWithFloat(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl bitwiseAnd(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl bitwiseOr(Operand *leftSide, Operand *rightSide, Operand *result);
void __cdecl multiply_J(int constraintRowCount, ConstraintRowData *rows, float *in);


// ui_shared_obj
// KISAKTODO: Rewrite the functions to actually use these templated versions. (There aren't a ton so they were just pasted in)
template<typename T, int useless, int HASH_SEED>
struct KeywordHashEntry
{
    bool KeywordHash_IsValidSeed(int count, int seed)
    {

    }
    int KeywordHash_PickSeed(int count)
    {
        for (int seed = 0; !IsValidSeed(count, HASH_SEED); seed++)
        {
            iassert(seed != 65536);
        }
    }
    void KeywordHash_Validate()
    {
        if (!KeywordHash_IsValidSeed())
        {
            // MyAssertHandler(
            //     ".\\ui\\ui_shared_obj.cpp",
            //     685,
            //     0,
            //     "%s\n\t(KeywordHash_PickSeed( array, count )) = %i",
            //     "(KeywordHash_IsValidSeed( array, count, HASH_SEED ))",
            //     v2);
        }
    }
    const char *keyword;
    int(__cdecl *func)(T *, int);
};
int __cdecl PC_CheckTokenString(source_s *source, const char *string);
EvalValue *__cdecl Eval_Solve(EvalValue *result, Eval *eval);
bool __cdecl Eval_AnyMissingOperands(const Eval *eval);
int __cdecl PC_Int_Expression_Parse(int handle, int *i);
int __cdecl PC_Int_ParseLine(int handle, int *i);
void __cdecl PC_PopIndent(source_s *source, int *type, parseSkip_t *skip);
void __cdecl PC_PushIndent(source_s *source, int type, parseSkip_t skip);
int __cdecl PC_ReadLine(source_s *source, token_s *token, bool expandDefines);
void PC_SourceError(int handle, char *format, ...);
void __cdecl PC_UnreadToken(source_s *source, token_s *token);

void __cdecl free_expression(statement_s *statement);
void __cdecl Menu_FreeItemMemory(itemDef_s *item);
void __cdecl Item_SetupKeywordHash();
void __cdecl Menu_SetupKeywordHash();
MenuList *__cdecl UI_LoadMenu_LoadObj(char *menuFile, int imageTrack);
MenuList *__cdecl UI_LoadMenus_LoadObj(char *menuFile, int imageTrack);


// ui_component
struct UI_Component_data_t // sizeof=0xAC
{                                       // ...
    float screenWidth;                  // ...
    float screenHeight;                 // ...
    float charWidth;                    // ...
    float charHeight;                   // ...
    float scrollBarSize;                // ...
    Material *cursor;                   // ...
    float cursorPos[2];                 // ...
    int hideCursor;                     // ...
    Material *filledCircle;             // ...
    int consoleReason;                  // ...
    char findText[128];                 // ...
};
struct UI_Component // sizeof=0x10
{                                       // ...
    //void(__thiscall *Init)(UI_Component *this);
    //void(__thiscall *Shutdown)(UI_Component *this);
    //void(__thiscall * ~UI_Component)(UI_Component *this);
    //void(__thiscall *Draw)(UI_Component *this, float, float, float, float, float, float);
    //bool(__thiscall *KeyEvent)(UI_Component *this, float *, int);
    //UI_Component *(__thiscall *GetCompAtLocation)(UI_Component *this, float *);
    //void(__thiscall *AddText)(UI_Component *this, const char *);

    //UI_Component_vtbl *__vftable;

    virtual void Init();
    virtual void Shutdown();
    virtual ~UI_Component();
    virtual void Draw(float one, float two, float three, float four, float five, float six);
    virtual bool KeyEvent(float *, int);
    virtual UI_Component *GetCompAtLocation(float *location);
    virtual void AddText(const char *text);

    float size[2];
    UI_Component *selectionParent;      // ...

    static void InitAssets();
    static Material *RegisterMaterialNoMip(char *name, int imageTrack);
    static void MouseEvent(int x, int y);

    static UI_Component_data_t g;
    static UI_Component *selectionComp;
};

struct UI_LinesComponent : UI_Component // sizeof=0x24
{                                       // ...
    virtual bool SetSelectedLineFocus(int newSelectedLine, bool user);

    int selectedLine;                   // ...
    bool focusOnSelectedLine;
    bool focusOnSelectedLineUser;
    // padding byte
    // padding byte
    int numLines;                       // ...
    float pos[2];

    void UpdateHeight();
    void ClearFocus();
    void IncSelectedLineFocus(bool wrap);
    void DecSelectedLineFocus(bool wrap);
};

struct __declspec(align(4)) UI_ScrollPane : UI_Component // sizeof=0x34
{                                       // ...
    UI_LinesComponent *comp;            // ...
    bool forceHorScoll;                 // ...
    // padding byte
    // padding byte
    // padding byte
    float mouseHeldScale[2];
    float mouseHeldPos[2];
    float mouseHeldCompPos[2];
    bool mouseWasDown[2];
    // padding byte
    // padding byte
};

struct Scr_ScriptWindow : UI_LinesComponent // sizeof=0x3C
{

    unsigned int bufferIndex;
    int currentTopLine;
    const char *currentBufPos;
    struct Scr_Breakpoint *breakpointHead;
    struct Scr_Breakpoint *builtinHead;
    int numCols;


    char *GetFilename();

    void FindNext();
    void FindPrev();
    void SetCurrentLine(int line);

    void GetSourcePos(unsigned int *start, unsigned int *end);

    void AddBreakpoint(
        struct Scr_Breakpoint **pBreakpoint,
        char *codePos,
        int builtinIndex,
        Scr_WatchElement_s *element,
        unsigned __int8 type);

    bool AddBreakpointAtSourcePos(
        Scr_WatchElement_s *element,
        unsigned __int8 breakpointType,
        bool user,
        struct Scr_Breakpoint **pBreakpoint,
        unsigned int startSourcePos,
        unsigned int endSourcePos);

    void ToggleBreakpointInternal(
        Scr_WatchElement_s *element,
        bool force,
        bool overwrite,
        unsigned __int8 breakpointType,
        bool user);
};

struct Scr_AbstractScriptList : UI_LinesComponent // sizeof=0x28
{                                       // ...
    Scr_ScriptWindow **scriptWindows;   // ...

    bool AddEntryName(const char *filename, bool select);
    void AddEntry(Scr_ScriptWindow *scriptWindow, bool select);
};

struct Scr_ScriptList : Scr_AbstractScriptList // sizeof=0x28
{                                       // ...
    void LoadScriptPos();
};

struct Scr_OpenScriptList : Scr_AbstractScriptList // sizeof=0x2C
{                                       // ...
    virtual bool SetSelectedLineFocus(int newSelectedLine, bool user);

    struct Scr_StringNode_s *usedHead;
};

struct Scr_ScriptWatch : UI_LinesComponent // sizeof=0x34
{                                       // ...
    virtual bool SetSelectedLineFocus(int newSelectedLine, bool user);

    Scr_WatchElement_s *elementHead;    // ...
    int elementId;                      // ...
    unsigned int localId;               // ...
    int dirty;                          // ...

    Scr_WatchElement_s *GetElementWithId_r(Scr_WatchElement_s *element, int id);

    Scr_WatchElement_s *GetElementWithId(int id)
    {
        return GetElementWithId_r(this->elementHead, id);
    }

    void AddElement(Scr_WatchElement_s *element, char *text);
    Scr_WatchElement_s *CloneElement(Scr_WatchElement_s *element);

    void DeleteElement();
    void DeleteElementInternal(Scr_WatchElement_s *element);

    void ToggleBreakpointInternal(Scr_WatchElement_s *element, unsigned __int8 type);
    void ToggleWatchElementBreakpoint(Scr_WatchElement_s *element, unsigned __int8 type);

    Scr_WatchElement_s *GetSelectedElement();
    Scr_WatchElement_s *GetSelectedElement_r(Scr_WatchElement_s *element, int *currentline);

    Scr_WatchElement_s ** GetElementRef(Scr_WatchElement_s *element);
    Scr_WatchElement_s *GetElementPrev(Scr_WatchElement_s *element);

    void ToggleExpandElement(Scr_WatchElement_s *element);

    Scr_WatchElement_s *BackspaceElementInternal(Scr_WatchElement_s *element);

    void UpdateHeight();

    void FreeWatchElement(Scr_WatchElement_s *element);
    Scr_WatchElement_s *AddBreakpoint(
        Scr_WatchElement_s *element,
        unsigned __int8 type);
    Scr_WatchElement_s *RemoveBreakpoint(Scr_WatchElement_s *element);

    void SetSelectedElement(Scr_WatchElement_s *selElement, bool user);
    bool SetSelectedElement_r(Scr_WatchElement_s *selElement, Scr_WatchElement_s *element, int *currentLine, bool user);

    void Evaluate();
    void EvaluateWatchElement(Scr_WatchElement_s *element);
    void EvaluateWatchElementExpression(
        Scr_WatchElement_s *element,
        struct VariableValue *value);

    bool EvaluateWatchChildElement(
        Scr_WatchElement_s *element,
        unsigned int fieldName,
        Scr_WatchElement_s *childElement,
        bool hardcodedField);
    void EvaluateWatchChildren(Scr_WatchElement_s *parentElement);
    bool PostEvaluateWatchElement(
        Scr_WatchElement_s *element,
        struct VariableValue *value);

    void LoadSelectedLine(struct Scr_SelectedLineInfo *info);
    void SaveSelectedLine(struct Scr_SelectedLineInfo *info);

    Scr_WatchElement_s* PasteNonBreakpointElement(
        Scr_WatchElement_s *element,
        char *text,
        bool user);

    void UpdateBreakpoints(bool add);

    void DisplayThreadPos(Scr_WatchElement_s *element);

    void SortHitBreakpointsTop();

    Scr_WatchElement_s* CreateWatchElement(
        char *text,
        Scr_WatchElement_s **prevElem,
        const char *name);

    Scr_WatchElement_s* CreateBreakpointElement(
        Scr_WatchElement_s *element,
        unsigned int bufferIndex,
        unsigned int sourcePos,
        bool user);
};

struct Scr_SourcePos2_t // sizeof=0x8
{                                       // ...
    unsigned int bufferIndex;           // ...
    unsigned int sourcePos;             // ...
};

struct Scr_ScriptCallStack : UI_LinesComponent // sizeof=0x12C
{                                       // ...
    virtual bool SetSelectedLineFocus(int newSelectedLine, bool user);

    Scr_SourcePos2_t stack[33];         // ...

    void UpdateStack();
};

struct UI_VerticalDivider : UI_Component // sizeof=0x1C
{                                       // ...
    UI_ScrollPane *topComp;             // ...
    UI_ScrollPane *bottomComp;          // ...
    float posY;                         // ...
};

void __cdecl UI_Component_Init();
