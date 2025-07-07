#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __cdecl TRACK_actor_aim();
void __cdecl Actor_DrawDebugAccuracy(const float *pos, double scale, double rowHeight, int a4, const char *a5);
void __cdecl Actor_DebugAccuracyMsg(
    unsigned int msgIndex,
    const char *msg,
    double accuracy,
    const float *color,
    float *a5);
float __cdecl Actor_GetAccuracyFraction(
    double dist,
    const WeaponDef *weapDef,
    const char **accuracyType,
    unsigned int a4);
float __cdecl Actor_GetWeaponAccuracy(
    const actor_s *self,
    const sentient_s *enemy,
    const WeaponDef *weapDef,
    WeapAccuracyType accuracyType);
float __cdecl Actor_GetPlayerStanceAccuracy(const actor_s *self, const sentient_s *enemy);
float __cdecl Actor_GetPlayerMovementAccuracy(const actor_s *self, const sentient_s *enemy);
float __cdecl Actor_GetPlayerSightAccuracy(actor_s *self, const sentient_s *enemy);
float __cdecl Actor_GetFinalAccuracy(actor_s *self, weaponParms *wp, double accuracyMod);
void __cdecl Actor_FillWeaponParms(actor_s *self, weaponParms *wp);
void __cdecl Actor_HitSentient(weaponParms *wp, sentient_s *enemy, double accuracy);
void __cdecl Actor_HitTarget(const weaponParms *wp, const float *target, float *forward);
void __cdecl Actor_HitEnemy(actor_s *self, weaponParms *wp, double accuracy);
void __cdecl Actor_MissSentient(weaponParms *wp, sentient_s *enemy, double accuracy);
void __cdecl Actor_MissTarget(const weaponParms *wp, const float *target, float *forward);
void __cdecl Actor_MissEnemy(actor_s *self, weaponParms *wp, double accuracy);
void __cdecl Actor_ShootNoEnemy(actor_s *self, weaponParms *wp);
void __cdecl Actor_ShootPos(actor_s *self, weaponParms *wp, float *pos);
void __cdecl Actor_ClampShot(actor_s *self, weaponParms *wp);
void __cdecl Actor_Shoot(actor_s *self, double accuracyMod, float (*posOverride)[3], float *lastShot, int a5);
void __cdecl Actor_ShootBlank(actor_s *self);
gentity_s *__cdecl Actor_Melee(actor_s *self, const float *direction);
float __cdecl Sentient_GetScarinessForDistance(sentient_s *self, sentient_s *enemy, double fDist);
void __cdecl Actor_GetAccuracyGraphFileName_FastFile(
    const WeaponDef *weaponDef,
    WeapAccuracyType accuracyType,
    char *filePath,
    unsigned int sizeofFilePath);
void __cdecl Actor_GetAccuracyGraphFileName(
    const WeaponDef *weaponDef,
    WeapAccuracyType accuracyType,
    char *filePath,
    unsigned int sizeofFilePath);
void __cdecl Actor_AccuracyGraphSaveToFile(
    const DevGraph *graph,
    WeaponDef *weaponDef,
    WeapAccuracyType accuracyType);
void __cdecl Actor_CommonAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    WeapAccuracyType accuracyType);
void __cdecl Actor_AiVsAiAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    int unusedLocalClientNum);
void __cdecl Actor_AiVsPlayerAccuracyGraphEventCallback(
    const DevGraph *graph,
    DevEventType event,
    int unusedLocalClientNum);
void __cdecl Actor_AccuracyGraphTextCallback(
    const DevGraph *graph,
    double inputX,
    double inputY,
    char *text,
    const int textLength,
    char *a6);
void __cdecl G_SwapAccuracyBuffers();
DevGraph *__cdecl Actor_InitWeaponAccuracyGraphForWeaponType(
    unsigned int weaponIndex,
    WeapAccuracyType accuracyType,
    void(__cdecl *eventCallback)(const DevGraph *, DevEventType, int));
void __cdecl Actor_CopyAccuGraphBuf(WeaponDef *from, WeaponDef *to);
void __cdecl Actor_InitWeaponAccuracyGraphForWeapon(unsigned int weaponIndex);
void __cdecl Actor_ShutdownWeaponAccuracyGraph();
