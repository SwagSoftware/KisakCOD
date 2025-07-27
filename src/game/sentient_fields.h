#pragma once
#include "sentient.h"
#include "game_public.h"

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

struct sentient_fields_s
{
	const char *name;
	int ofs;
	fieldtype_t type;
	void(*setter)(sentient_s *, const sentient_fields_s *);
	void(*getter)(sentient_s *, const sentient_fields_s *);
};

void __cdecl SentientScr_ReadOnly(sentient_s *pSelf, const sentient_fields_s *pField);
void __cdecl SentientScr_SetTeam(sentient_s *pSelf, const sentient_fields_s *pField);
void __cdecl SentientScr_GetTeam(sentient_s *pSelf, const sentient_fields_s *pField);
void __cdecl GScr_AddFieldsForSentient();
void __cdecl Scr_SetSentientField(sentient_s *sentient, unsigned int offset);
void __cdecl Scr_GetSentientField(sentient_s *sentient, unsigned int offset);
