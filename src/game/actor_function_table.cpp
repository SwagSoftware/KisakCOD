#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

#include "actor.h"
#include "actor_exposed.h"
#include "actor_generic.h"
#include "actor_turret.h"
#include "actor_grenade.h"
#include "actor_badplace.h"
#include "actor_cover_arrival.h"
#include "actor_death.h"
#include "actor_pain.h"
#include "actor_animscripted.h"
#include "actor_negotiation.h"
#include "actor_dog_exposed.h"

const ai_funcs_t AIHumanFuncTable[11] =
{
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Exposed_Start,
    Actor_Exposed_Finish,
    Actor_Generic_Suspend,
    Actor_Exposed_Resume,
    Actor_Exposed_Think,
    Actor_Exposed_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_Turret_Start,
    Actor_Turret_Finish,
    Actor_Turret_Suspend,
    Actor_Generic_Resume,
    Actor_Turret_Think,
    Actor_Turret_Touch,
    Actor_Turret_Pain
  },
  {
    Actor_Grenade_Start,
    Actor_Grenade_Finish,
    Actor_Grenade_Suspend,
    Actor_Grenade_Resume,
    Actor_Grenade_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_BadPlace_Flee_Start,
    Actor_BadPlace_Flee_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_BadPlace_Flee_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_CoverArrival_Start,
    Actor_Generic_Finish,
    Actor_Generic_Suspend,
    Actor_CoverArrival_Resume,
    Actor_CoverArrival_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_Death_Start,
    Actor_Generic_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Death_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_Pain_Start,
    Actor_Pain_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Pain_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_ScriptedAnim_Start,
    Actor_ScriptedAnim_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_ScriptedAnim_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_CustomAnim_Start,
    Actor_Generic_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_CustomAnim_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_Negotiation_Start,
    Actor_Generic_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Negotiation_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  }
};


const ai_funcs_t AIDogFuncTable[11] =
{
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Dog_Exposed_Start,
    Actor_Dog_Exposed_Finish,
    Actor_Dog_Exposed_Suspend,
    Actor_Generic_Resume,
    Actor_Dog_Exposed_Think,
    Actor_Exposed_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_BadPlace_Flee_Start,
    Actor_BadPlace_Flee_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_BadPlace_Flee_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
  {
    Actor_Death_Start,
    Actor_Generic_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Death_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_Pain_Start,
    Actor_Pain_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Pain_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_ScriptedAnim_Start,
    Actor_ScriptedAnim_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_ScriptedAnim_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_CustomAnim_Start,
    Actor_Generic_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_CustomAnim_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  },
  {
    Actor_Negotiation_Start,
    Actor_Generic_Finish,
    Actor_Generic_Suspend,
    Actor_Generic_Resume,
    Actor_Negotiation_Think,
    Actor_Generic_Touch,
    Actor_Generic_Pain
  }
};


const ai_funcs_t *AIFuncTable[2] = { AIHumanFuncTable, AIDogFuncTable };

