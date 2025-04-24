#pragma once
#include "scr_stringlist.h"
#include <cstdio> // FILE
#include <minwindef.h>
#include "scr_debugger.h"

struct stype_t // sizeof=0x8
{                                       // ...
    sval_u val;                         // ...
    unsigned int pos;                   // ...
};

struct yy_buffer_state // sizeof=0x28
{                                       // ...
    FILE *yy_input_file;
    char *yy_ch_buf;                    // ...
    char *yy_buf_pos;
    unsigned int yy_buf_size;           // ...
    int yy_n_chars;
    int yy_is_our_buffer;               // ...
    int yy_is_interactive;
    int yy_at_bol;
    int yy_fill_buffer;
    int yy_buffer_status;
};

int __cdecl yyparse();
HashEntry_unnamed_type_u __cdecl LowerCase(unsigned int stringValue);
void __cdecl _yy_memcpy(char *to, char *from, unsigned int count);
int __cdecl yylex();
void __cdecl TextValue(char *str, int len);
int __cdecl StringValue(char *str, int len);
void __cdecl IntegerValue(char *str);
void __cdecl FloatValue(char *str);
int __cdecl yy_get_next_buffer();
int __cdecl yy_get_previous_state();
int __cdecl yy_try_NUL_trans(int yy_current_state);
void __cdecl yyrestart(FILE *input_file);
void __cdecl yy_load_buffer_state();
yy_buffer_state *__cdecl yy_create_buffer(FILE *file, unsigned int size);
void __cdecl yy_init_buffer(yy_buffer_state *b, FILE *file);
void __cdecl yy_flush_buffer(yy_buffer_state *b);
void __cdecl  yy_fatal_error(const char *msg);
LPVOID __cdecl yy_flex_alloc(unsigned int size);
void *__cdecl yy_flex_realloc(void *ptr, unsigned int size);
int __cdecl yyerror();
void __cdecl ScriptParse(sval_u *parseData, unsigned __int8 user);
