#pragma once
#include <bgame/bg_local.h>

struct weaponParms // sizeof=0x40
{                                       // ...
    float forward[3];                   // ...
    float right[3];                     // ...
    float up[3];                        // ...
    float muzzleTrace[3];               // ...
    float gunForward[3];                // ...
    WeaponDef *weapDef;                 // ...
};

char __cdecl BulletTrace(
    int localClientNum,
    const BulletFireParams *bp,
    const WeaponDef *weapDef,
    const centity_s *attacker,
    BulletTraceResults *br,
    unsigned int lastSurfaceType);
float __cdecl G_GoodRandomFloat(int *idum);
void __cdecl Bullet_Endpos(int randSeed, float spread, float *end, float *dir, const weaponParms *wp, float maxRange);
void __cdecl Bullet_RandomDir(int time, float *x, float *y);
void __cdecl Bullet_Fire(
    gentity_s *attacker,
    float spread,
    const weaponParms *wp,
    const gentity_s *weaponEnt,
    int gameTime);
void __cdecl Bullet_FireExtended(BulletFireParams *bp, const WeaponDef *weapDef, gentity_s *attacker, int gameTime);
char __cdecl Bullet_Trace(
    const BulletFireParams *bp,
    const WeaponDef *weapDef,
    gentity_s *attacker,
    BulletTraceResults *br,
    unsigned int lastSurfaceType);
bool __cdecl Bullet_IgnoreHitEntity(const BulletFireParams *bp, const BulletTraceResults *br, gentity_s *attacker);
void __cdecl Bullet_Process(
    const BulletFireParams *bp,
    BulletTraceResults *br,
    const WeaponDef *weapDef,
    gentity_s *attacker,
    int dFlags,
    int gameTime,
    int *outImpactFlags,
    bool processFx);
int __cdecl Bullet_GetDamage(
    const BulletFireParams *bp,
    const BulletTraceResults *br,
    const WeaponDef *weapDef,
    gentity_s *attacker);
void __cdecl Bullet_ImpactEffect(
    const BulletFireParams *bp,
    const BulletTraceResults *br,
    const float *normal,
    const WeaponDef *weapDef,
    gentity_s *attacker,
    unsigned __int8 impactEffectFlags,
    gentity_s **outTempEnt);
void __cdecl Bullet_FirePenetrate(BulletFireParams *bp, const WeaponDef *weapDef, gentity_s *attacker, int gameTime);
