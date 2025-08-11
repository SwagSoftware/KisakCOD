// adapted from: https://github.com/voron00/CoD2rev_Server/blob/41e33ed97d0339ac631772f25eee3910ddef87e5/src/script/scr_yacc.cpp
// (GPL v3.0) (Thanks)

#include "../qcommon/qcommon.h"
//#include "script_public.h"
#include "scr_yacc.h"
#include "scr_debugger.h"
#include "scr_parsetree.h"
#include <cstdint>
#include "scr_main.h"
#include "scr_compiler.h"

#include "scr_yacc_structs.h"
#include "scr_vm.h"

#ifdef KISAK_MP
#include <client_mp/client_mp.h>
#endif

#pragma GCC push_options
#pragma GCC optimize ("O0")

struct stype_t // sizeof=0x8
{                                       // ...
	sval_u val;                         // ...
	unsigned int pos;                   // ...
};

#define YY_BUF_SIZE 0x4000 //16384

unsigned int g_out_pos;
unsigned int g_sourcePos;
unsigned char g_parse_user;
sval_u g_dummyVal;
int yy_init = 0;
yy_buffer_state *yy_current_buffer;
int yy_start;
int yy_n_chars;
char *yy_c_buf_p;
char *yytext;
FILE *yyin = 0;
FILE *yyout = 0;
char yy_hold_char;
char ch_buf[YY_BUF_SIZE];
sval_u yaccResult;
int yynerrs;
int yy_last_accepting_state;
char *yy_last_accepting_cpos;
int yy_did_buffer_switch_on_eof;
stype_t yylval;
unsigned int yyleng;
int yychar;


FILE *yy_stdin()
{
	return stdin;
}

FILE *yy_stdout()
{
	return stdout;
}

void __cdecl IntegerValue(char *str)
{
	sscanf(str, "%d", &yylval);
}

void __cdecl FloatValue(char *str)
{
	sscanf(str, "%f", &yylval);
}

int StringValue(char *str, int len)
{
	char c;
	char string[8192];
	char *pC1;
	char *pC2;
	int n;

	if (len < 0x2000)
	{
		pC1 = string;
		while (len)
		{
			if (*str == 92)
			{
				n = len - 1;
				if (!n)
					break;
				pC2 = str + 1;
				c = *pC2;
				if (*pC2 == 110)
				{
					*pC1++ = 10;
				}
				else if (c == 114)
				{
					*pC1++ = 13;
				}
				else
				{
					if (c == 116)
						*pC1 = 9;
					else
						*pC1 = *pC2;
					++pC1;
				}
				len = n - 1;
				str = pC2 + 1;
			}
			else
			{
				--len;
				*pC1++ = *str++;
			}
		}
		*pC1 = 0;
		yylval.val.stringValue = SL_GetString_(string, g_parse_user, 14);
		return 1;
	}
	else
	{
		CompileError(g_sourcePos, "max string length exceeded: \"%s\"", str);
		return 0;
	}
}

unsigned int LowerCase(unsigned int strVal)
{
	return SL_ConvertToLowercase(strVal, g_parse_user, 14);
}

void yy_load_buffer_state()
{
	yy_n_chars = yy_current_buffer->yy_n_chars;
	yy_c_buf_p = yy_current_buffer->yy_buf_pos;
	yytext = yy_c_buf_p;
	yyin = yy_current_buffer->yy_input_file;
	yy_hold_char = *yy_c_buf_p;
}

void yy_flush_buffer(yy_buffer_state *b)
{
	if (b)
	{
		b->yy_n_chars = 0;
		*b->yy_ch_buf = 0;
		b->yy_ch_buf[1] = 0;
		b->yy_buf_pos = b->yy_ch_buf;
		b->yy_at_bol = 1;
		b->yy_buffer_status = 0;

		if (b == yy_current_buffer)
		{
			yy_load_buffer_state();
		}
	}
}

int yy_try_NUL_trans(int yy_current_state)
{
	unsigned char yy_c;
	int yy_current_statea;

	yy_c = 1;
	if (yy_accept[yy_current_state])
	{
		yy_last_accepting_state = yy_current_state;
		yy_last_accepting_cpos = yy_c_buf_p;
	}
	while (yy_chk[yy_c + yy_base[yy_current_state]] != yy_current_state)
	{
		yy_current_state = yy_def[yy_current_state];
		if (yy_current_state >= 258) // 262 on t5
		{
			yy_c = yy_meta[yy_c];
		}
	}
	yy_current_statea = yy_nxt[yy_c + yy_base[yy_current_state]];
	return yy_current_statea != 257 ? yy_current_statea : 0; // 261 on t5
}

int yy_get_previous_state()
{
	unsigned char yy_c;
	int yy_current_state;
	char *yy_cp;

	yy_current_state = yy_start;
	for (yy_cp = yytext; yy_cp < yy_c_buf_p; ++yy_cp)
	{
		if (*yy_cp)
		{
			yy_c = yy_ec[(unsigned char)*yy_cp];
		}
		else
		{
			yy_c = 1;
		}

		if (yy_accept[yy_current_state])
		{
			yy_last_accepting_state = yy_current_state;
			yy_last_accepting_cpos = yy_cp;
		}

		while (yy_chk[yy_c + yy_base[yy_current_state]] != yy_current_state)
		{
			yy_current_state = yy_def[yy_current_state];
			if (yy_current_state >= 258) // 262 on t5
			{
				yy_c = yy_meta[yy_c];
			}
		}
		yy_current_state = yy_nxt[yy_c + yy_base[yy_current_state]];
	}
	return yy_current_state;
}

LPVOID __cdecl yy_flex_alloc(unsigned int size)
{
	return malloc(size);
}

void yy_fatal_error(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(2);
}

void *yy_flex_realloc(void *ptr, unsigned int size)
{
	return realloc(ptr, size);
}

void __cdecl yy_init_buffer(yy_buffer_state *b, FILE *file)
{
	yy_flush_buffer(b);
	b->yy_input_file = file;
	b->yy_fill_buffer = 1;
	b->yy_is_interactive = 0;
}

yy_buffer_state *yy_create_buffer()
{
	yy_buffer_state *b;

	//b = (yy_buffer_state *)Z_TryMallocInternal(sizeof(yy_buffer_state));
	b = (yy_buffer_state *)yy_flex_alloc(sizeof(yy_buffer_state));
	if (!b)
	{
		yy_fatal_error("out of dynamic memory in yy_create_buffer()");
	}

	b->yy_buf_size = YY_BUF_SIZE;
	//b->yy_ch_buf = (char *)Z_TryMallocInternal(b->yy_buf_size + 2);
	b->yy_ch_buf = (char*)yy_flex_alloc(b->yy_buf_size + 2);
	if (!b->yy_ch_buf)
	{
		yy_fatal_error("out of dynamic memory in yy_create_buffer()");
	}

	b->yy_is_our_buffer = 1;
	yy_init_buffer(b, yyin);
	return b;
}

void __cdecl TextValue(char *str, int len)
{
	yylval.val.stringValue = SL_GetStringOfSize(str, 0, len + 1, 14).prev;
}

void yyrestart()
{
	if (!yy_current_buffer)
	{
		yy_current_buffer = yy_create_buffer();
	}

	yy_init_buffer(yy_current_buffer, yyin);
	yy_load_buffer_state();
}

int yy_get_next_buffer()
{
	int result;
	int yy_c_buf_p_offset;
	yy_buffer_state *b;
	signed int num_to_read;
	char *source;
	int ret_val;
	char *dest;
	int number_to_move;
	int i;

	dest = (yy_current_buffer)->yy_ch_buf;
	source = yytext;
	if (yy_c_buf_p > &dest[yy_n_chars + 1])
	{
		yy_fatal_error("fatal flex scanner internal error--end of buffer missed");
	}

	if ((yy_current_buffer)->yy_fill_buffer)
	{
		number_to_move = yy_c_buf_p - yytext - 1;
		for (i = 0;
			i < number_to_move;
			++i)
		{
			*dest++ = *source++;
		}
		if ((yy_current_buffer)->yy_buffer_status == 2)
		{
			yy_n_chars = 0;
			(yy_current_buffer)->yy_n_chars = 0;
		}
		else
		{
			for (num_to_read = (yy_current_buffer)->yy_buf_size - number_to_move - 1;
				num_to_read <= 0;
				num_to_read = (yy_current_buffer)->yy_buf_size - number_to_move - 1)
			{
				b = yy_current_buffer;
				yy_c_buf_p_offset = yy_c_buf_p - (yy_current_buffer)->yy_ch_buf;

				if ((yy_current_buffer)->yy_is_our_buffer)
				{
					if ((signed int)(2 * (yy_current_buffer)->yy_buf_size) > 0)
					{
						(yy_current_buffer)->yy_buf_size *= 2;
					}
					else
					{
						(yy_current_buffer)->yy_buf_size += (yy_current_buffer)->yy_buf_size >> 3;
					}

					b->yy_ch_buf = (char *)yy_flex_realloc(b->yy_ch_buf, b->yy_buf_size + 2);
				}
				else
				{
					(yy_current_buffer)->yy_ch_buf = 0;
				}
				if (!b->yy_ch_buf)
				{
					yy_fatal_error("fatal error - scanner input buffer overflow");
				}

				yy_c_buf_p = &b->yy_ch_buf[yy_c_buf_p_offset];
			}

			if (num_to_read > 0x2000)
			{
				num_to_read = 0x2000;
			}

			yy_n_chars = Scr_ScanFile(&((yy_current_buffer)->yy_ch_buf[number_to_move]), num_to_read);
			(yy_current_buffer)->yy_n_chars = yy_n_chars;
		}
		if (yy_n_chars)
		{
			ret_val = 0;
		}
		else if (number_to_move)
		{
			ret_val = 2;
			(yy_current_buffer)->yy_buffer_status = 2;
		}
		else
		{
			ret_val = 1;
			yyrestart();
		}
		yy_n_chars += number_to_move;
		(yy_current_buffer)->yy_ch_buf[yy_n_chars] = 0;
		(yy_current_buffer)->yy_ch_buf[yy_n_chars + 1] = 0;
		yytext = (yy_current_buffer)->yy_ch_buf;
		result = ret_val;
	}
	else if (yy_c_buf_p - yytext == 1)
	{
		result = 1;
	}
	else
	{
		result = 2;
	}

	return result;
}

int __cdecl yylex()
{
	int yy_next_buffer; // [esp+0h] [ebp-24h]
	int yy_next_state; // [esp+8h] [ebp-1Ch]
	int yy_amount_of_matched_text; // [esp+Ch] [ebp-18h]
	unsigned __int8 yy_c; // [esp+13h] [ebp-11h]
	char *yy_bp; // [esp+14h] [ebp-10h]
	int yy_current_state; // [esp+18h] [ebp-Ch]
	int yy_act; // [esp+1Ch] [ebp-8h]
	char *yy_cp; // [esp+20h] [ebp-4h]

	if (yy_init)
	{
		yy_init = 0;
		if (!yy_start)
			yy_start = 1;

		if (!yyin)
			yyin = stdin;

		if (!yyout)
			yyout = stdout;

		if (!yy_current_buffer)
			yy_current_buffer = yy_create_buffer();

		yy_load_buffer_state();
	}
	while (1)
	{
	LABEL_11:
		yy_cp = yy_c_buf_p;
		*yy_c_buf_p = yy_hold_char;
		yy_bp = yy_cp;
		yy_current_state = yy_start;
		do
		{
		yy_match:
			yy_c = yy_ec[*yy_cp];
			if (yy_accept[yy_current_state])
			{
				yy_last_accepting_state = yy_current_state;
				yy_last_accepting_cpos = yy_cp;
			}
			while (yy_chk[yy_c + yy_base[yy_current_state]] != yy_current_state)
			{
				yy_current_state = yy_def[yy_current_state];
				if (yy_current_state >= 258)
					yy_c = yy_meta[yy_c];
			}
			yy_current_state = yy_nxt[yy_c + yy_base[yy_current_state]];
			++yy_cp;
		} while (yy_base[yy_current_state] != 435);
		while (2)
		{                                           // yy_find_action
			yy_act = yy_accept[yy_current_state];
			if (!yy_accept[yy_current_state])
			{
				yy_cp = yy_last_accepting_cpos;
				yy_act = yy_accept[yy_last_accepting_state];
			}
			yytext = yy_bp;
			yyleng = yy_cp - yy_bp;
			yy_hold_char = *yy_cp;
			*yy_cp = 0;
			yy_c_buf_p = yy_cp;
		do_action:

			switch (yy_act)
			{
			case 0:
				*yy_cp = yy_hold_char;
				yy_cp = yy_last_accepting_cpos;
				yy_current_state = yy_last_accepting_state;
				continue;
			case 1:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				goto LABEL_11;
			case 2:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				yy_start = 3;
				goto LABEL_11;
			case 3:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				goto LABEL_11;
			case 4:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				goto LABEL_11;
			case 5:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				goto LABEL_11;
			case 6:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				yy_start = 5;
				goto LABEL_11;
			case 7:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return StringValue(yytext + 1, yyleng - 2) != 0 ? 259 : 257;
			case 8:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return StringValue(yytext + 2, yyleng - 3) != 0 ? 260 : 257;
			case 9:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 261;
			case 10:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 262;
			case 11:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 263;
			case 12:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 264;
			case 13:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 265;
			case 14:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 266;
			case 15:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 267;
			case 16:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 268;
			case 17:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 269;
			case 18:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 270;
			case 19:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 271;
			case 20:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 272;
			case 21:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 273;
			case 22:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 274;
			case 23:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 275;
			case 24:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 276;
			case 25:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 277;
			case 26:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 278;
			case 27:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 279;
			case 28:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 280;
			case 29:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 281;
			case 30:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 282;
			case 31:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 283;
			case 32:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 284;
			case 33:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 285;
			case 34:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 286;
			case 35:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				IntegerValue(yytext);
				return 287;
			case 36:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				FloatValue(yytext);
				return 288;
			case 37:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 290;
			case 38:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 289;
			case 39:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 294;
			case 40:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 291;
			case 41:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 293;
			case 42:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 292;
			case 43:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 295;
			case 44:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 296;
			case 45:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 297;
			case 46:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 298;
			case 47:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 299;
			case 48:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 300;
			case 49:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 301;
			case 50:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 302;
			case 51:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 303;
			case 52:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 304;
			case 53:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 305;
			case 54:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 306;
			case 55:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 307;
			case 56:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 308;
			case 57:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 309;
			case 58:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 310;
			case 59:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 311;
			case 60:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 312;
			case 61:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 313;
			case 62:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 314;
			case 63:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 315;
			case 64:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 316;
			case 65:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 317;
			case 66:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 318;
			case 67:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 319;
			case 68:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 320;
			case 69:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 321;
			case 70:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 322;
			case 71:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 323;
			case 72:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 325;
			case 73:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 326;
			case 74:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 327;
			case 75:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 328;
			case 76:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 329;
			case 77:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 330;
			case 78:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 331;
			case 79:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 332;
			case 80:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 333;
			case 81:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 334;
			case 82:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 335;
			case 83:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 336;
			case 84:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 337;
			case 85:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 338;
			case 86:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 339;
			case 87:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 340;
			case 88:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 341;
			case 89:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 342;
			case 90:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				return 343;
			case 91:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				TextValue(yytext, yyleng);
				return 258;
			case 92:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				TextValue(yytext, yyleng);
				return 324;
			case 93:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				CompileError(g_sourcePos, "bad token '%s'", yytext);
				return 257;
			case 94:
				yylval.pos = g_out_pos;
				g_sourcePos = g_out_pos;
				g_out_pos += yyleng;
				fwrite(yytext, yyleng, 1u, yyout);
				goto LABEL_11;
			case 95:
				yy_amount_of_matched_text = yy_cp - yytext - 1;
				*yy_cp = yy_hold_char;
				if (!yy_current_buffer->yy_buffer_status)
				{
					yy_n_chars = yy_current_buffer->yy_n_chars;
					yy_current_buffer->yy_input_file = yyin;
					yy_current_buffer->yy_buffer_status = 1;
				}
				if (yy_c_buf_p <= &yy_current_buffer->yy_ch_buf[yy_n_chars])
				{
					yy_c_buf_p = &yytext[yy_amount_of_matched_text];
					yy_current_state = yy_get_previous_state();
					yy_next_state = yy_try_NUL_trans(yy_current_state);
					yy_bp = yytext;
					if (yy_next_state)
					{
						yy_cp = ++yy_c_buf_p;
						yy_current_state = yy_next_state;
						goto yy_match;
					}
					yy_cp = yy_c_buf_p;
					continue;
				}
				yy_next_buffer = yy_get_next_buffer();
				if (!yy_next_buffer)
				{
					yy_c_buf_p = &yytext[yy_amount_of_matched_text];
					yy_current_state = yy_get_previous_state();
					yy_cp = yy_c_buf_p;
					yy_bp = yytext;
					goto yy_match;
				}
				if (yy_next_buffer != 1)
				{
					if (yy_next_buffer != 2)
						goto LABEL_11;
					yy_c_buf_p = &yy_current_buffer->yy_ch_buf[yy_n_chars];
					yy_current_state = yy_get_previous_state();
					yy_cp = yy_c_buf_p;
					yy_bp = yytext;
					continue;
				}
				yy_did_buffer_switch_on_eof = 0;
				if (CL_GetLocalClientActiveCount())
				{
					yy_c_buf_p = yytext;
					yy_act = (yy_start - 1) / 2 + 96;
					goto do_action;
				}
				if (!yy_did_buffer_switch_on_eof)
					yyrestart();
				break;
			case 96:
			case 97:
			case 98:
				return 0;
			default:
				yy_fatal_error("fatal flex scanner internal error--no action found");
			}
			break;
		}                                           // while(2)
	}
}

#define YYINITDEPTH 200 + sizeof(stype_t)
int yyparse()
{
	/*-------------------------.
	| yyparse or yypush_parse.  |
	`-------------------------*/

	int yystate;
	/* Number of tokens to shift before error messages enabled.  */
	int yyerrorstatus;

	/* The semantic value stack.  */
	stype_t yyvsa[YYINITDEPTH];
	stype_t *yyvsp;
	stype_t *yyvs;

	/* The stacks and their tools:
	`yyss': related to states.
	`yyvs': related to semantic values.

	Refer to the stacks thru separate pointers, to allow yyoverflow
	to reallocate them elsewhere.  */

	/* The state stack.  */
	short yyssa[YYINITDEPTH];
	short *yyss;
	short *yyssp;

	int yystacksize;

	bool needs_free = false;
	void *free1addr = NULL;
	void *free2addr = NULL;

	int yyn;
	/* Lookahead token as an internal (translated) token number.  */
	int yytoken; // yychar1
	/* The variables used to return semantic value and location from the
	action routines.  */
	stype_t yyval;

	short *yyss1;
	int yysize;
	stype_t *yyvs1;

	sval_u valstack[6];
	unsigned int s;

	/* The number of symbols on the RHS of the reduced rule.
	Keep to zero when no symbol should be popped.  */
	int yylen = 0;

	yytoken = 0;
	yyss = yyssa;
	yyvs = yyvsa;
	yystacksize = YYINITDEPTH;

	// YYDPRINTF ((stderr, "Starting parse\n"));

	yystate = 0;
	yyerrorstatus = 0;
	yynerrs = 0;
	yychar = -2; // YYEMPTY /* Cause a token to be read.  */

	/* Initialize stack pointers.
	Waste one element of value and location stack
	so that they stay on the same level as the state stack.
	The wasted elements are never initialized.  */
	yyssp = yyss;
	yyvsp = yyvs;

//	goto yysetstate;

	while (1)
	{
		/*------------------------------------------------------------.
		| yynewstate -- Push a new state, which is found in yystate.  |
		`------------------------------------------------------------*/
yynewstate:
		/* In all cases, when you get here, the value and location stacks
		have just been pushed.  So pushing a state here evens the stacks.  */
		//yyssp++; // LWSS CHANGE - move ++ below

//	yysetstate:
		*++yyssp = yystate;

		if (yyssp >= &yyss[yystacksize - 1])
		{
			yyvs1 = yyvs;
			yyss1 = yyss;

			/* Get the current used size of the three stacks, in elements.  */
			yysize = yyssp - yyss + 1;

			/* Extend the stack our own way.  */
			if (yystacksize >= 10000) // YYMAXDEPTH
			{
				if (!yychar) // yyerror yyexhaustedlab
				{
					CompileError(g_sourcePos, "unexpected end of file found");
				}

				if (yychar != 257)
				{
					CompileError(g_sourcePos, "bad syntax");
				}

				if (needs_free)
				{
					free(free1addr);
					free(free2addr);
				}

				return 2;
			}

			yystacksize *= 2;
			if (yystacksize > 10000) // YYMAXDEPTH
			{
				yystacksize = 10000; // YYMAXDEPTH
			}

			//iassert(needs_free == false);
			//needs_free = true;

			// YYSTACK_RELOCATE (yyss_alloc, yyss);
			s = sizeof(short) * yystacksize;
			yyss = (short *)alloca(s);
			//yyss = (short *)malloc(s);
			free1addr = yyss;
			//memcpy(yyss, yyss1, s);
			memcpy(yyss, yyss1, sizeof(short) * yysize); // LWSS CHANGE

			// YYSTACK_RELOCATE (yyvs_alloc, yyvs);
			s = sizeof(stype_t) * yystacksize;
			yyvs = (stype_t *)alloca(s);
			//yyvs = (stype_t *)malloc(s);
			free2addr = yyvs;
			//memcpy(yyss, yyvs1, s);
			memcpy(yyvs, yyvs1, sizeof(stype_t) * yysize); // LWSS CHANGE

			yyvsp = &yyvs[yysize - 1];
			yyssp = &yyss[yysize - 1];

			// YYDPRINTF ((stderr, "Stack size increased to %lu\n", (unsigned long int) yystacksize));

			if (yyssp >= &yyss[yystacksize - 1])
			{
				goto yyabortlab;
				//return 1;                               // yyabortlab  YYABORT
			}
		}

//		goto yybackup;

		/*-----------.
		| yybackup.  |
		`-----------*/
//	yybackup:

		/* Do appropriate processing given the current state.  Read a
		lookahead token if we need one and don't already have one.  */

		/* First try to decide what to do without reference to lookahead token.  */
		yyn = yypact[yystate];

		if (yyn == -32768) // YYPACT_NINF
		{
			break; // yydefault
		}

		/* Not known => get a lookahead token if don't already have one.  */

		/* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
		if (yychar == -2)
		{
			// YYDPRINTF ((stderr, "Reading a token: "));
			yychar = yylex();
		}

		if (yychar > 0) // YYEOF
		{
			yytoken = (unsigned int)yychar > 0x158 ? 119 : yytranslate[yychar];// t5 is 0x159 and 120  YYTRANSLATE
			// YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
		}
		else
		{
			yytoken = 0;
			yychar = 0;
			// YYDPRINTF ((stderr, "Now at end of input.\n"));
		}

		/* If the proper action on seeing token YYTOKEN is to reduce or to
		detect an error, take that action.  */
		yyn += yytoken;

		//printf("yyn %i | yytoken %i | yycheck[yyn] %i\n", yyn, yytoken, yycheck[yyn]);

		if (yyn < 0 || yyn >= 0x544 || yycheck[yyn] != yytoken) // 0x59B on t5  YYLAST
		{
			break; // yydefault
		}

		yyn = yytable[yyn];

		if (yyn >= 0)
		{
			if (!yyn)
			{
				goto yyerrlab;
			}

			if (yyn == 261)                         // 267 on t5   YYFINAL
			{
				//return 0;                               // yyacceptlab
				goto yyacceptlab;
			}

			if (yychar)
			{
				/* Shift the lookahead token.  */
				//YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

				/* Discard the shifted token.  */
				yychar = -2;
			}

			*++yyvsp = yylval;

			/* Count tokens shifted since error; after three, turn off error
			status.  */
			if (yyerrorstatus)
			{
				--yyerrorstatus;
			}

			yystate = yyn;
			//continue; // yynewstate
		}
		else
		{
			if (yyn == -32768) // YYTABLE_NINF
			{
				goto yyerrlab;
			}

			yyn = -yyn;

			/*-----------------------------.
			| yyreduce -- Do a reduction.  |
			`-----------------------------*/
		yyreduce:
			/* yyn is the number of a rule to reduce with.  */
			yylen = yyr2[yyn];

			/* If YYLEN is nonzero, implement the default value of the action:
			`$$ = $1'.

			Otherwise, the following line sets YYVAL to garbage.
			This behavior is undocumented and Bison
			users should not rely upon it.  Assigning to YYVAL
			unconditionally makes the parser a bit smaller, and it avoids a
			GCC warning that YYVAL may be used uninitialized.  */
			if (yylen > 0)
			{
				yyval = yyvsp[1 - yylen];
			}

			// YY_REDUCE_PRINT (yyn);

			//printf("yyn %i\n", yyn);

			switch (yyn)
			{
			case 1:
				yaccResult = node1(yyvsp[-1].val.type, yyvsp->val);// node2_
				break;
			case 2:
				yaccResult = node1(ENUM_expression, yyvsp->val);// node1
				break;
			case 3:
			case 4:
				yaccResult = node1(ENUM_statement, yyvsp->val);
				break;
			case 5:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_primitive_expression, yyvsp->val, valstack[5]);
				break;
			case 6:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].sourcePosValue = yyvsp->pos;
				valstack[3].sourcePosValue = yyvsp[-2].pos;
				valstack[2] = yyvsp->val;
				valstack[1] = yyvsp[-2].val;
				yyval.val = node5(
					ENUM_bool_or,
					valstack[1],
					valstack[2],
					valstack[3],
					valstack[4],
					valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 7:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].sourcePosValue = yyvsp->pos;
				valstack[3].sourcePosValue = yyvsp[-2].pos;
				valstack[2] = yyvsp->val;
				valstack[1] = yyvsp[-2].val;
				yyval.val = node5(
					ENUM_bool_and,
					valstack[1],
					valstack[2],
					valstack[3],
					valstack[4],
					valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 8:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_bit_or;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 9:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_bit_ex_or;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 10:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_bit_and;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 11:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_equality;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 12:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_inequality;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 13:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_less;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 14:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_greater;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 15:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_less_equal;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 16:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_greater_equal;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 17:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_shift_left;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 18:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_shift_right;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 19:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_plus;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 20:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_minus;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 21:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_multiply;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 22:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_divide;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 23:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_mod;
				yyval.val = node4(ENUM_binary, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				break;
			case 24:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_bool_not, yyvsp->val, valstack[5]);
				break;
			case 25:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_bool_complement, yyvsp->val, valstack[5]);
				break;
			case 26:
				yyval.val = node1(ENUM_expression, yyvsp->val);
				break;
			case 27:
				yyval.val = node0(ENUM_NOP);
				break;
			case 28:
			case 29:
				yyvsp->val.stringValue = LowerCase(yyvsp->val.stringValue);
				yyval.val = yyvsp->val;
				break;
			case 30:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				valstack[3] = yyvsp[-2].val;
				yyvsp->val = valstack[4];
				yyval.val = node3(ENUM_far_function, valstack[3], valstack[4], valstack[5]);
				++scrCompilePub.far_function_count;
				break;
			case 31:
				valstack[5].sourcePosValue = yyvsp->pos; // node_pos
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				yyvsp->val = valstack[4];
				yyval.val = node2(ENUM_local_function, valstack[4], valstack[5]);
				break;
			case 32:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				valstack[3] = yyvsp[-2].val;
				yyvsp->val = valstack[4];
				yyval.val = node3(ENUM_far_function, valstack[3], valstack[4], valstack[5]);
				yyval.pos = yyvsp[-1].pos;
				++scrCompilePub.far_function_count;
				break;
			case 33:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				yyvsp->val = valstack[4];
				yyval.val = node2(ENUM_local_function, valstack[4], valstack[5]);
				break;
			case 34:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_function, yyvsp->val, valstack[5]);
				break;
			case 35:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node2(ENUM_function_pointer, yyvsp[-2].val, valstack[5]);
				break;
			case 36:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_script_call, yyvsp->val, valstack[5]);
				break;
			case 37:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node3(ENUM_script_thread_call, yyvsp->val, valstack[4], valstack[5]);
				yyval.pos = yyvsp->pos;
				break;
			case 38:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node3(ENUM_call, yyvsp[-3].val, yyvsp[-1].val, valstack[5]);
				yyval.pos = yyvsp[-2].pos;
				break;
			case 39:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				valstack[4].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node5(
					ENUM_method,
					yyvsp[-4].val,
					yyvsp[-3].val,
					yyvsp[-1].val,
					valstack[4],
					valstack[5]);
				yyval.pos = yyvsp[-2].pos;
				break;
			case 40:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node2(ENUM_expression_list, yyvsp[-1].val, valstack[5]);
				break;
			case 41:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_integer, yyvsp->val, valstack[5]);
				break;
			case 42:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_float, yyvsp->val, valstack[5]);
				break;
			case 43:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node2(ENUM_minus_integer, yyvsp->val, valstack[5]);
				break;
			case 44:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node2(ENUM_minus_float, yyvsp->val, valstack[5]);
				break;
			case 45:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_string, yyvsp->val, valstack[5]);
				break;
			case 46:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_istring, yyvsp->val, valstack[5]);
				break;
			case 47:
				yyval.val = node1(ENUM_call_expression, yyvsp->val);
				break;
			case 48:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_variable, yyvsp->val, valstack[5]);
				break;
			case 49:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_undefined, valstack[5]);
				break;
			case 50:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_self, valstack[5]);
				break;
			case 51:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_level, valstack[5]);
				break;
			case 52:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_game, valstack[5]);
				break;
			case 53:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_anim, valstack[5]);
				break;
			case 54:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node2(ENUM_size_field, yyvsp[-1].val, valstack[5]);
				yyval.pos = yyvsp->pos;
				break;
			case 55:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node2(ENUM_function, yyvsp->val, valstack[5]);
				break;
			case 56:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node1(ENUM_empty_array, valstack[5]);
				break;
			case 57:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				yyvsp->val = valstack[4];
				yyval.val = node2(ENUM_animation, valstack[4], valstack[5]);
				break;
			case 58:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_false, valstack[5]);
				break;
			case 59:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_true, valstack[5]);
				break;
			case 60:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_animtree, valstack[5]);
				break;
			case 61:
				valstack[5].sourcePosValue = yyvsp[-3].pos;
				yyval.val = node3(ENUM_breakon, yyvsp[-4].val, yyvsp[-1].val, valstack[5]);
				break;
			case 62:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				yyvsp->val = valstack[4];
				yyval.val = node3(ENUM_field_variable, yyvsp[-2].val, valstack[4], valstack[5]);
				yyval.pos = yyvsp->pos;
				break;
			case 63:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].sourcePosValue = yyvsp[-3].pos;
				yyval.val = node4(ENUM_array_variable, yyvsp[-3].val, yyvsp[-1].val, valstack[4], valstack[5]);
				yyval.pos = yyvsp[-2].pos;
				break;
			case 64:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				yyvsp->val = valstack[4];
				yyval.val = node2(ENUM_local_variable, valstack[4], valstack[5]);
				break;
			case 65:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				yyvsp->val = valstack[4];
				yyval.val = node2(ENUM_object, valstack[4], valstack[5]);
				break;
			case 66:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node2(ENUM_self_field, yyvsp[-2].val, valstack[5]);
				yyval.pos = yyvsp->pos;
				break;
			case 67:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node4(ENUM_assignment, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 68:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node2(ENUM_return, yyvsp->val, valstack[5]);
				break;
			case 69:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_return2, valstack[5]);
				break;
			case 70:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].sourcePosValue = yyvsp->pos;
				yyval.val = node3(ENUM_wait, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 71:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node2(ENUM_inc, yyvsp[-1].val, valstack[5]);
				break;
			case 72:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node2(ENUM_dec, yyvsp[-1].val, valstack[5]);
				break;
			case 73:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_bit_or;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 74:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_bit_ex_or;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 75:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_bit_and;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 76:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_shift_left;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 77:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_shift_right;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 78:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_plus;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 79:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_minus;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 80:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_multiply;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 81:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_divide;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 82:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].intValue = OP_mod;
				yyval.val = node4(ENUM_binary_equals, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5]);
				break;
			case 83:
				valstack[5].sourcePosValue = yyvsp[-3].pos;
				valstack[4].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node4(ENUM_waittill, yyvsp[-4].val, yyvsp[-1].val, valstack[4], valstack[5]);
				break;
			case 84:
				valstack[5].sourcePosValue = yyvsp[-3].pos;
				valstack[4].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node4(ENUM_waittillmatch, yyvsp[-4].val, yyvsp[-1].val, valstack[4], valstack[5]);
				break;
			case 85:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_waittillFrameEnd, valstack[5]);
				break;
			case 86:
				valstack[5].sourcePosValue = yyvsp[-3].pos;
				valstack[4].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node4(ENUM_notify, yyvsp[-4].val, yyvsp[-1].val, valstack[4], valstack[5]);
				break;
			case 87:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node4(ENUM_endon, yyvsp[-4].val, yyvsp[-1].val, valstack[4], valstack[5]);
				break;
			case 88:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_break, valstack[5]);
				break;
			case 89:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_continue, valstack[5]);
				break;
			case 90:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_breakpoint, valstack[5]);
				break;
			case 91:
				valstack[5].sourcePosValue = yyvsp[-3].pos;
				yyval.val = node2(ENUM_prof_begin, yyvsp[-1].val, valstack[5]);
				break;
			case 92:
				valstack[5].sourcePosValue = yyvsp[-3].pos;
				yyval.val = node2(ENUM_prof_end, yyvsp[-1].val, valstack[5]);
				break;
			case 93:
				yyval.val = node1(ENUM_call_expression_statement, yyvsp->val);
				break;
			case 95:
				yyval.val = node0(ENUM_NOP);
				break;
			case 98:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node3(ENUM_statement_list, yyvsp[-1].val, valstack[4], valstack[5]);
				break;
			case 99:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node4(ENUM_if, yyvsp[-2].val, yyvsp->val, valstack[5], g_dummyVal);
				break;
			case 100: // this 0x65 on t5
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				valstack[4].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node7( // ENUM_if_else inlined opti arg
					ENUM_if_else,
					yyvsp[-4].val,
					yyvsp[-2].val,
					yyvsp->val,
					valstack[4],
					valstack[5],
					g_dummyVal,
					g_dummyVal);
				break;
			case 101:
				valstack[5].sourcePosValue = yyvsp[-4].pos;
				valstack[4].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node5(ENUM_while, yyvsp[-2].val, yyvsp->val, valstack[4], valstack[5], g_dummyVal);
				break;
			case 102:
				valstack[5].sourcePosValue = yyvsp[-7].pos;
				valstack[4].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node8( // ENUM_for inlined
					ENUM_for,
					yyvsp[-5].val,
					yyvsp[-4].val,
					yyvsp[-2].val,
					yyvsp->val,
					valstack[4],
					valstack[5],
					g_dummyVal,
					g_dummyVal);
				break;
			case 103:
				valstack[5].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node3(ENUM_switch, yyvsp[-4].val, yyvsp[-1].val, valstack[5]);
				break;
			case 104:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node3(ENUM_developer_statement_list, yyvsp[-1].val, valstack[5], g_dummyVal);
				break;
			case 105:
				yyval.val = node0(ENUM_NOP);
				break;
			case 106:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				yyval.val = node3(ENUM_case, yyvsp[-1].val, valstack[5], g_dummyVal);
				break;
			case 107:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node2(ENUM_default, valstack[5], g_dummyVal);
				break;
			case 109:
				yyval.val = append_node(yyvsp[-1].val, yyvsp->val);
				break;
			case 110:
				valstack[5] = node0(ENUM_NOP);
				yyval.val = linked_list_end(valstack[5]);
				break;
			case 111:
				valstack[4].sourcePosValue = yyvsp->pos;
				valstack[5] = node1(yyvsp->val.type, valstack[4]);
				yyval.val = prepend_node(valstack[5], yyvsp[-2].val);
				break;
			case 112:
				valstack[3].sourcePosValue = yyvsp->pos;
				valstack[5] = node0(ENUM_NOP);
				valstack[4] = node1(yyvsp->val.type, valstack[3]);
				yyval.val = prepend_node(valstack[4], valstack[5]);
				break;
			case 114:
				yyval.val = node0(ENUM_NOP);
				break;
			case 115:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				yyvsp->val = valstack[4];
				valstack[3] = node1(valstack[4].type, valstack[5]);
				yyval.val = append_node(yyvsp[-2].val, valstack[3]);
				break;
			case 116:
				yyvsp->val.stringValue = LowerCase(yyvsp->val.stringValue);
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4] = node1(yyvsp->val.type, valstack[5]);
				valstack[3] = node0(ENUM_NOP);
				valstack[2] = linked_list_end(valstack[3]);
				yyval.val = append_node(valstack[2], valstack[4]);
				break;
			case 118:
				valstack[5] = node0(ENUM_NOP);
				yyval.val = linked_list_end(valstack[5]);
				break;
			case 119:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4].stringValue = LowerCase(yyvsp->val.stringValue);
				yyvsp->val = valstack[4];
				valstack[3] = node1(valstack[4].type, valstack[5]);
				yyval.val = append_node(yyvsp[-2].val, valstack[3]);
				break;
			case 120:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4] = yyvsp->val;
				valstack[3] = node1(valstack[4].type, valstack[5]);
				valstack[2] = node0(ENUM_NOP);
				valstack[1] = linked_list_end(valstack[2]);
				yyval.val = append_node(valstack[1], valstack[3]);
				break;
			case 121:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4] = yyvsp->val;
				valstack[3] = node1(valstack[4].type, valstack[5]);
				yyval.val = append_node(yyvsp[-2].val, valstack[3]);
				break;
			case 122:
				valstack[5].sourcePosValue = yyvsp->pos;
				valstack[4] = yyvsp->val;
				valstack[3] = node1(valstack[4].type, valstack[5]);
				valstack[2] = node0(ENUM_NOP);
				valstack[1] = linked_list_end(valstack[2]);
				yyval.val = append_node(valstack[1], valstack[3]);
				break;
			case 123:
				valstack[4].sourcePosValue = yyvsp->pos;
				valstack[5] = node1(yyvsp->val.type, valstack[4]);
				yyval.val = prepend_node(valstack[5], yyvsp[-2].val);
				break;
			case 124:
				valstack[3].sourcePosValue = yyvsp->pos;
				valstack[5] = node0(ENUM_NOP);
				valstack[4] = node1(yyvsp->val.type, valstack[3]);
				yyval.val = prepend_node(valstack[4], valstack[5]);
				break;
			case 125:
				valstack[5] = g_dummyVal;
				valstack[4].sourcePosValue = yyvsp->pos;
				valstack[3].sourcePosValue = yyvsp[-6].pos;
				valstack[2] = yyvsp[-1].val;
				valstack[1] = yyvsp[-4].val;
				valstack[0].stringValue = LowerCase(yyvsp[-6].val.stringValue);
				yyvsp[-6].val = valstack[0];
				yyval.val = node6(ENUM_thread, valstack[0], valstack[1], valstack[2], valstack[3], valstack[4], valstack[5]); // ENUM_thread inlined
				break;
			case 126:
				valstack[5].sourcePosValue = yyvsp[-2].pos;
				valstack[4].sourcePosValue = yyvsp[-4].pos;
				yyval.val = node3(ENUM_usingtree, yyvsp[-2].val, valstack[4], valstack[5]);
				break;
			case 127:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_begin_developer_thread, valstack[5]);
				break;
			case 128:
				valstack[5].sourcePosValue = yyvsp->pos;
				yyval.val = node1(ENUM_end_developer_thread, valstack[5]);
				break;
			case 129:
				yyval.val = append_node(yyvsp[-1].val, yyvsp->val);
				break;
			case 130:
				valstack[5] = node0(ENUM_NOP);
				yyval.val = linked_list_end(valstack[5]);
				break;
			case 131:
				valstack[5].sourcePosValue = yyvsp[-1].pos;
				yyval.val = node2(ENUM_include, yyvsp->val, valstack[5]);
				++scrCompilePub.far_function_count;
				break;
			case 132:
				yyval.val = append_node(yyvsp[-2].val, yyvsp[-1].val);
				break;
			case 133:
				valstack[5] = node0(ENUM_NOP);
				yyval.val = linked_list_end(valstack[5]);
				break;
			default:
				break;
			}

			// YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

			yyvsp -= yylen;
			yyssp -= yylen;
			//yylen = 0; // LWSS CHANGE

			// YY_STACK_PRINT (yyss, yyssp);

			*++yyvsp = yyval;

			/* Now `shift' the result of the reduction.  Determine what state
			that goes to, based on the state we popped back to and the rule
			number reduced by.  */
			yyn = yyr1[yyn];

#define YYNTOKENS 91
			//yystate = *yyssp + yypgoto[yyn - YYNTOKENS]; // yystate = yypgoto[yyn - YYNTOKENS] + *yyssp; ?
			yystate = *yyssp + yypact[yyn + 171]; // LWSS CHANGE

			// LWSS CHANGE
			//if (yystate < 0x544 && yycheck[yystate] == *yyssp)
			//{
			//	yystate = yytable[yystate];
			//}
			//else
			//{
			//	yystate = yydefact[yyn + 171];
			//}
			if (yystate >= 0 && yystate <= 0x543 && yycheck[yystate] == *yyssp) // 0x59B on t5  YYLAST
			{
				yystate = yytable[yystate];
			}
			else
			{
				yystate = yydefgoto[yyn - YYNTOKENS]; // yystate = yydefgoto[yyn - YYNTOKENS]; ?
			}

			int stop = yydefgoto[0]; // LWSS HACK: just to keep the array from being optimized out (dumb)

			//continue; // yynewstate
		}
	}

	/*-----------------------------------------------------------.
	| yydefault -- do the default action for the current state.  |
	`-----------------------------------------------------------*/
//yydefault:
	yyn = yydefact[yystate];
	if (yyn)
	{
		goto yyreduce;
	}

	//goto yyerrlab;

	/*------------------------------------.
	| yyerrlab -- here on detecting error |
	`------------------------------------*/
yyerrlab:
	/* If not already recovering from an error, report this error.  */
	if (!yyerrorstatus)
	{
		++yynerrs;
		if (!yychar) // yyerror
		{
			CompileError(g_sourcePos, "unexpected end of file found");
		}

		if (yychar != 257)
		{
			CompileError(g_sourcePos, "bad syntax");
		}
	}


	if (yyerrorstatus == 3)
	{
		/* If just tried and failed to reuse lookahead token after an
		error, discard it.  */

		/* Return failure if at end of input.  */
		if (!yychar)
		{
yyabortlab:
			if (needs_free)
			{
				free(free1addr);
				free(free2addr);
			}
			return 1; // YYABORT
		}

		// yydestruct ("Error: discarding", yytoken, &yylval);

		yychar = -2;
	}

	/*-------------------------------------------------------------.
	| yyerrlab1 -- common code for both syntax error and YYERROR.  |
	`-------------------------------------------------------------*/
yyerrlab1:
	yyerrorstatus = 3; /* Each real token shifted decrements this.  */

	while (2)
	{
		yyn = yypact[yystate];
		if (yyn == -32768) // YYPACT_NINF
		{
			goto yyerrpop;
		}

		//if (++yyn < 0 || yyn > 0x543 || yycheck[yyn] != 1) // 0x59B on t5
		if (++yyn >= 0x544 || yycheck[yyn] != 1) // LWSS CHANGE
		{
			goto yyerrpop;
		}

		yyn = yytable[yyn];
		if (yyn < 0)
		{
			if (yyn != -32768)
			{
				yyn = -yyn;
				goto yyreduce;
			}

			goto yyerrpop;
		}

		if (!yyn)
		{
			/* Pop the current state because it cannot handle the error token.  */
yyerrpop:
			if (yyssp == yyss)
			{
				goto yyabortlab;
				//return 1;                               // yyabortlab
			}

			--yyvsp;
			yystate = *--yyssp;
			continue;
		}
		break;
	}

	if (yyn != 261)
	{
		*++yyvsp = yylval;
		yystate = yyn;
		goto yynewstate;
	}
yyacceptlab:
	if (needs_free)
	{
		free(free1addr);
		free(free2addr);
	}
	return 0;
}

void ScriptParse(sval_u *parseData, unsigned char user)
{
	yy_buffer_state buffer_state;

	g_out_pos = -1;
	g_sourcePos = 0;
	g_parse_user = user;
	g_dummyVal.node = 0;
	yy_init = 1;
	buffer_state.yy_buf_size = YY_BUF_SIZE;
	buffer_state.yy_ch_buf = ch_buf;
	buffer_state.yy_is_our_buffer = 0;
	yy_init_buffer(&buffer_state, 0);
	yy_current_buffer = &buffer_state;
	yy_start = 3;
	yyparse();
	*parseData = yaccResult;
}

#pragma GCC pop_options