#pragma once
#include <xanim/xanim.h>
#include <ui_mp/ui_mp.h>
#include <physics/ode/objects.h>
#include <bgame/bg_local.h>



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
void __cdecl Menu_HandleKey(UiContext *dc, menuDef_t *menu, int key, int down);
bool __cdecl Item_TextField_HandleKey(UiContext *dc, itemDef_s *item, int key);
void __cdecl Item_TextField_EnsureCursorVisible(int localClientNum, itemDef_s *item, const char *text);
int __cdecl Item_HandleKey(UiContext *dc, itemDef_s *item, int down);
int __cdecl Item_OwnerDraw_HandleKey(itemDef_s *item, int key);
int __cdecl Item_ListBox_HandleKey(UiContext *dc, itemDef_s *item, int key, int down, int force);
void __cdecl Item_ListBox_Page(int localClientNum, itemDef_s *item, int max, int scrollmax, int viewmax, int delta);
void __cdecl Item_ListBox_Scroll(int localClientNum, itemDef_s *item, int max, int scrollmax, int viewmax, int delta);
int __cdecl Item_YesNo_HandleKey(UiContext *dc, itemDef_s *item, int key);
bool __cdecl Item_ShouldHandleKey(UiContext *dc, itemDef_s *item, int key);
int __cdecl Item_Multi_HandleKey(UiContext *dc, itemDef_s *item);
int __cdecl Item_Multi_CountSettings(itemDef_s *item);
int __cdecl Item_Multi_FindDvarByValue(itemDef_s *item);
int __cdecl Item_List_NextEntryForKey(int key, int current, int count);
int __cdecl Item_DvarEnum_HandleKey(UiContext *dc, itemDef_s *item, int key);
int __cdecl Item_DvarEnum_CountSettings(itemDef_s *item);
int __cdecl Item_DvarEnum_EnumIndex(itemDef_s *item);
double __cdecl Item_Slider_ThumbPosition(int localClientNum, itemDef_s *item);
double __cdecl Item_GetRectPlacementX(int alignX, float x0, float containerWidth, float selfWidth);
double __cdecl Item_ListBox_ThumbPosition(int localClientNum, itemDef_s *item);
int __cdecl Item_Slider_HandleKey(UiContext *dc, itemDef_s *item);
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
            MyAssertHandler(
                ".\\ui\\ui_shared_obj.cpp",
                685,
                0,
                "%s\n\t(KeywordHash_PickSeed( array, count )) = %i",
                "(KeywordHash_IsValidSeed( array, count, HASH_SEED ))",
                v2);
        }
    }
    const char *keyword;
    int(__cdecl *func)(T *, int);
};
void __cdecl free_expression(statement_s *statement);
void __cdecl Menu_FreeItemMemory(itemDef_s *item);
void __cdecl Item_SetupKeywordHash();
void __cdecl Menu_SetupKeywordHash();
MenuList *__cdecl UI_LoadMenu_LoadObj(char *menuFile, int imageTrack);
MenuList *__cdecl UI_LoadMenus_LoadObj(char *menuFile, int imageTrack);