#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall TRACK_actor_aim();
void __fastcall Actor_DrawDebugAccuracy(const float *pos, double scale, double rowHeight, int a4, const char *a5);
void __fastcall Actor_DebugAccuracyMsg(
    unsigned int msgIndex,
    const char *msg,
    double accuracy,
    const float *color,
    float *a5);
float __fastcall Actor_GetAccuracyFraction(
    double dist,
    const WeaponDef *weapDef,
    const char **accuracyType,
    unsigned int a4);
float __fastcall Actor_GetWeaponAccuracy(
    const actor_s *self,
    const sentient_s *enemy,
    const WeaponDef *weapDef,
    WeapAccuracyType accuracyType);
float __fastcall Actor_GetPlayerStanceAccuracy(const actor_s *self, const sentient_s *enemy);
float __fastcall Actor_GetPlayerMovementAccuracy(const actor_s *self, const sentient_s *enemy);
float __fastcall Actor_GetPlayerSightAccuracy(actor_s *self, const sentient_s *enemy);
float __fastcall Actor_GetFinalAccuracy(actor_s *self, weaponParms *wp, double accuracyMod);
void __fastcall Actor_FillWeaponParms(actor_s *self, weaponParms *wp);
void __fastcall Actor_HitSentient(weaponParms *wp, sentient_s *enemy, double accuracy);
void __fastcall Actor_HitTarget(const weaponParms *wp, const float *target, float *forward);
void __fastcall Actor_HitEnemy(actor_s *self, weaponParms *wp, double accuracy);
void __fastcall Actor_MissSentient(weaponParms *wp, sentient_s *enemy, double accuracy);
void __fastcall Actor_MissTarget(const weaponParms *wp, const float *target, float *forward);
void __fastcall Actor_MissEnemy(actor_s *self, weaponParms *wp, double accuracy);
void __fastcall Actor_ShootNoEnemy(actor_s *self, weaponParms *wp);
void __fastcall Actor_ShootPos(actor_s *self, weaponParms *wp, float *pos);
void __fastcall Actor_ClampShot(actor_s *self, weaponParms *wp);
void __fastcall Actor_Shoot(actor_s *self, double accuracyMod, float (*posOverride)[3], float *lastShot, int a5);
void __fastcall Actor_ShootBlank(actor_s *self);
gentity_s *__fastcall Actor_Melee(actor_s *self, const float *direction);
float __fastcall Sentient_GetScarinessForDistance(sentient_s *self, sentient_s *enemy, double fDist);
void __fastcall Actor_GetAccuracyGraphFileName_FastFile(
    const WeaponDef *weaponDef,
    WeapAccuracyType accuracyType,
    char *filePath,
    unsigned int sizeofFilePath);
void __fastcall Actor_GetAccuracyGraphFileName(
    const WeaponDef *weaponDef,
    WeapAccuracyType accuracyType,
    char *filePath,
    unsigned int sizeofFilePath);
void __fastcall Actor_AccuracyGraphSaveToFile(
    const DevGraph *graph,
    WeaponDef *weaponDef,
    WeapAccuracyType accuracyType);
void __fastcall Actor_CommonAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    WeapAccuracyType accuracyType);
void __fastcall Actor_AiVsAiAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    int unusedLocalClientNum);
void __fastcall Actor_AiVsPlayerAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    int unusedLocalClientNum);
void __fastcall Actor_AccuracyGraphTextCallback(
    const DevGraph *graph,
    double inputX,
    double inputY,
    char *text,
    const int textLength,
    char *a6);
void __fastcall G_SwapAccuracyBuffers();
DevGraph *__fastcall Actor_InitWeaponAccuracyGraphForWeaponType(
    unsigned int weaponIndex,
    WeapAccuracyType accuracyType,
    void(__fastcall *eventCallback)(const DevGraph *, DevEventType, int));
void __fastcall Actor_CopyAccuGraphBuf(WeaponDef *from, WeaponDef *to);
void __fastcall Actor_InitWeaponAccuracyGraphForWeapon(unsigned int weaponIndex);
void __fastcall Actor_ShutdownWeaponAccuracyGraph();
