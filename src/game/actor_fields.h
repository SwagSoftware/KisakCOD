#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

unsigned __int8 *__cdecl BaseForFields(unsigned __int8 *actor, const actor_fields_s *fields);
const actor_fields_s *__cdecl FindFieldForName(const actor_fields_s *fields, const char *pszFieldName);
void __cdecl ActorScr_SetSpecies(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_GetSpecies(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_Clamp_0_1(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_Clamp_0_Positive(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_ReadOnly(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_SetGoalRadius(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_SetGoalHeight(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_SetTime(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_GetTime(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_SetWeapon(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_GetWeapon(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_GetGroundType(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_SetAnimPos(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_SetLastEnemySightPos(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_GetLastEnemySightPos(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl ActorScr_GetPathGoalPos(actor_s *self, const actor_fields_s *field);
void __cdecl ActorScr_SetFixedNode(actor_s *self, const actor_fields_s *field);
void __cdecl ActorScr_GetMoveMode(actor_s *pSelf, const actor_fields_s *pField);
void __cdecl PrintFieldUsage(const actor_fields_s *fields);
void Cmd_AI_PrintUsage();
void __cdecl Cmd_AI_DisplayInfo(actor_s *actor);
void __cdecl Cmd_AI_Delete(actor_s *actor);
void __cdecl Cmd_AI_DisplayValue(actor_s *pSelf, unsigned __int8 *pBase, const actor_fields_s *pField);
void __cdecl Cmd_AI_SetValue(actor_s *pSelf, int argc, unsigned __int8 *pBase, const actor_fields_s *pField);
void __cdecl Cmd_AI_Dispatch(int argc, actor_s *pSelf, const actor_fields_s *fields, const actor_fields_s *pField);
void __cdecl Cmd_AI_EntityNumber(
    int argc,
    const actor_fields_s *fields,
    const actor_fields_s *pField,
    const char *szNum,
    int bInvertSelection);
void __cdecl Cmd_AI_Team(
    int argc,
    const actor_fields_s *fields,
    const actor_fields_s *pField,
    int iTeamFlags,
    int bInvertSelection);
void __cdecl Cmd_AI_Name(
    int argc,
    const actor_fields_s *fields,
    const actor_fields_s *pField,
    const char *szName,
    int bInvertSelection);
void __cdecl Cmd_AI_f();
void __cdecl GScr_AddFieldsForActor();
void __cdecl Scr_SetActorField(actor_s *actor, unsigned int offset);
void __cdecl Scr_GetActorField(actor_s *actor, unsigned int offset);
