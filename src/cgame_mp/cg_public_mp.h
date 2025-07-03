#pragma once

#ifndef KISAK_MP
#error This File is MultiPlayer Only
#endif

#include <universal/q_shared.h> // dvar_t

extern const dvar_t *hud_fade_sprint;
extern const dvar_t *hud_health_pulserate_injured;
extern const dvar_t *hud_health_startpulse_critical;
extern const dvar_t *hud_fade_offhand;
extern const dvar_t *hud_deathQuoteFadeTime;
extern const dvar_t *hud_fade_ammodisplay;
extern const dvar_t *hud_health_startpulse_injured;
extern const dvar_t *hud_fade_stance;
extern const dvar_t *hud_fade_compass;
extern const dvar_t *hud_health_pulserate_critical;
extern const dvar_t *hud_fade_healthbar;
extern const dvar_t *hud_fadeout_speed;
extern const dvar_t *hud_enable;
extern const dvar_t *hud_healthOverlay_regenPauseTime;
extern const dvar_t *hud_healthOverlay_pulseStart;
extern const dvar_t *hud_healthOverlay_phaseOne_pulseDuration;
extern const dvar_t *hud_healthOverlay_phaseTwo_toAlphaMultiplier;
extern const dvar_t *hud_healthOverlay_phaseTwo_pulseDuration;
extern const dvar_t *hud_healthOverlay_phaseThree_toAlphaMultiplier;
extern const dvar_t *hud_healthOverlay_phaseThree_pulseDuration;
extern const dvar_t *hud_healthOverlay_phaseEnd_toAlpha;
extern const dvar_t *hud_healthOverlay_phaseEnd_pulseDuration;
extern const dvar_t *cg_sprintMeterFullColor;
extern const dvar_t *cg_sprintMeterEmptyColor;
extern const dvar_t *cg_sprintMeterDisabledColor;
extern const dvar_t *cg_drawTalk;

// cg_vehicles_mp
extern const dvar_t *vehDebugClient;
extern const dvar_t *heli_barrelSlowdown;
extern const dvar_t *vehDriverViewFocusRange;
extern const dvar_t *heli_barrelMaxVelocity;
extern const dvar_t *vehDriverViewDist;
extern const dvar_t *heli_barrelRotation;