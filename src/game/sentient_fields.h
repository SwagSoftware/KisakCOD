#pragma once

#ifndef KISAK_SP 
#error This file is for SinglePlayer only 
#endif

void __fastcall SentientScr_ReadOnly(sentient_s *pSelf, const sentient_fields_s *pField);
void __fastcall SentientScr_SetTeam(sentient_s *pSelf, const sentient_fields_s *pField);
void __fastcall SentientScr_GetTeam(sentient_s *pSelf, const sentient_fields_s *pField);
void __fastcall GScr_AddFieldsForSentient();
void __fastcall Scr_SetSentientField(sentient_s *sentient, unsigned int offset);
void __fastcall Scr_GetSentientField(sentient_s *sentient, unsigned int offset);
