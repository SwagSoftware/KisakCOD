#pragma once
#include "scr_stringlist.h"
#include <cstdio> // FILE
#include <Windows.h>

// LWSS: this enum name is kinda retarded
enum Enum_t : __int32
{
    ENUM_NOP = 0x0,
    ENUM_program = 0x1,
    ENUM_assignment = 0x2,
    ENUM_unknown_variable = 0x3,
    ENUM_local_variable = 0x4,
    ENUM_local_variable_frozen = 0x5,
    ENUM_primitive_expression = 0x6,
    ENUM_integer = 0x7,
    ENUM_float = 0x8,
    ENUM_minus_integer = 0x9,
    ENUM_minus_float = 0xA,
    ENUM_string = 0xB,
    ENUM_istring = 0xC,
    ENUM_array_variable = 0xD,
    ENUM_unknown_field = 0xE,
    ENUM_field_variable = 0xF,
    ENUM_field_variable_frozen = 0x10,
    ENUM_variable = 0x11,
    ENUM_function = 0x12,
    ENUM_call_expression = 0x13,
    ENUM_local_function = 0x14,
    ENUM_far_function = 0x15,
    ENUM_function_pointer = 0x16,
    ENUM_call = 0x17,
    ENUM_method = 0x18,
    ENUM_call_expression_statement = 0x19,
    ENUM_script_call = 0x1A,
    ENUM_return = 0x1B,
    ENUM_return2 = 0x1C,
    ENUM_wait = 0x1D,
    ENUM_script_thread_call = 0x1E,
    ENUM_undefined = 0x1F,
    ENUM_self = 0x20,
    ENUM_self_frozen = 0x21,
    ENUM_level = 0x22,
    ENUM_game = 0x23,
    ENUM_anim = 0x24,
    ENUM_if = 0x25,
    ENUM_if_else = 0x26,
    ENUM_while = 0x27,
    ENUM_for = 0x28,
    ENUM_inc = 0x29,
    ENUM_dec = 0x2A,
    ENUM_binary_equals = 0x2B,
    ENUM_statement_list = 0x2C,
    ENUM_developer_statement_list = 0x2D,
    ENUM_expression_list = 0x2E,
    ENUM_bool_or = 0x2F,
    ENUM_bool_and = 0x30,
    ENUM_binary = 0x31,
    ENUM_bool_not = 0x32,
    ENUM_bool_complement = 0x33,
    ENUM_size_field = 0x34,
    ENUM_self_field = 0x35,
    ENUM_precachetree = 0x36,
    ENUM_waittill = 0x37,
    ENUM_waittillmatch = 0x38,
    ENUM_waittillFrameEnd = 0x39,
    ENUM_notify = 0x3A,
    ENUM_endon = 0x3B,
    ENUM_switch = 0x3C,
    ENUM_case = 0x3D,
    ENUM_default = 0x3E,
    ENUM_break = 0x3F,
    ENUM_continue = 0x40,
    ENUM_expression = 0x41,
    ENUM_empty_array = 0x42,
    ENUM_animation = 0x43,
    ENUM_thread = 0x44,
    ENUM_begin_developer_thread = 0x45,
    ENUM_end_developer_thread = 0x46,
    ENUM_usingtree = 0x47,
    ENUM_false = 0x48,
    ENUM_true = 0x49,
    ENUM_animtree = 0x4A,
    ENUM_breakon = 0x4B,
    ENUM_breakpoint = 0x4C,
    ENUM_prof_begin = 0x4D,
    ENUM_prof_end = 0x4E,
    ENUM_vector = 0x4F,
    ENUM_object = 0x50,
    ENUM_thread_object = 0x51,
    ENUM_local = 0x52,
    ENUM_statement = 0x53,
    ENUM_bad_expression = 0x54,
    ENUM_bad_statement = 0x55,
    ENUM_include = 0x56,
    ENUM_argument = 0x57,
};

typedef struct
{
    FILE *yy_input_file;
    char *yy_ch_buf;
    char *yy_buf_pos;
    unsigned int yy_buf_size;
    int yy_n_chars;
    int yy_is_our_buffer;
    int yy_is_interactive;
    int yy_at_bol;
    int yy_fill_buffer;
    int yy_buffer_status;
} yy_buffer_state;

int __cdecl yyparse();
int __cdecl yylex();
void __cdecl TextValue(char *str, int len);
int __cdecl StringValue(char *str, int len);
void __cdecl IntegerValue(char *str);
void __cdecl FloatValue(char *str);
int __cdecl yy_get_next_buffer();
int __cdecl yy_get_previous_state();
int __cdecl yy_try_NUL_trans(int yy_current_state);
void __cdecl yy_load_buffer_state();
void __cdecl yy_flush_buffer(yy_buffer_state *b);
void __cdecl  yy_fatal_error(const char *msg);
LPVOID __cdecl yy_flex_alloc(unsigned int size);
void *__cdecl yy_flex_realloc(void *ptr, unsigned int size);
int __cdecl yyerror();
void __cdecl ScriptParse(union sval_u *parseData, unsigned __int8 user);
