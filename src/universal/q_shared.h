#pragma once

// LWSS: Not complete. It's going to be somewhat game-specific

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

#ifdef _WIN32
#pragma warning(disable : 4018)     // signed/unsigned mismatch
//#pragma warning(disable : 4032)		//formal parameter 'number' has different type when promoted
//#pragma warning(disable : 4051)		//type conversion; possible loss of data
//#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
//#pragma warning(disable : 4115)		//'type' : named type definition in parentheses
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
//#pragma warning(disable : 4136)		//conversion between different floating-point types
//#pragma warning(disable : 4201)		//nonstandard extension used : nameless struct/union
//#pragma warning(disable : 4214)		//nonstandard extension used : bit field types other than int
//#pragma warning(disable : 4220)		// varargs matches remaining parameters
#pragma warning(disable : 4244)		//'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable : 4284)		// return type not UDT
//#pragma warning(disable : 4305)		// truncation from const double to float
#pragma warning(disable : 4310)		// cast truncates constant value
#pragma warning(disable : 4514)		//unreferenced inline/local function has been removed
#pragma warning(disable : 4710)		// not inlined
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4786)		// identifier was truncated
#endif // _WIN32

#include "../universal/assertive.h" // LWSS add

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>
#include <ctype.h>
#include <cfloat> // FLT_MAX
#include <cstdint>

// this is the define for determining if we have an asm version of a C function
#if (defined _M_IX86 || defined __i386__) && !defined __sun__  && !defined __LCC__
#define id386	1
#else
#define id386	0
#endif

// for windows fastcall option
#define	QDECL

//======================= WIN32 DEFINES =================================
#ifdef WIN32

#define	MAC_STATIC

#undef QDECL
#define	QDECL	__cdecl

// buildstring will be incorporated into the version string
#ifdef NDEBUG
#ifdef _M_IX86
#define	CPUSTRING	"win-x86"
#elif defined _M_ALPHA
#define	CPUSTRING	"win-AXP"
#endif
#else
#ifdef _M_IX86
#define	CPUSTRING	"win-x86-debug"
#elif defined _M_ALPHA
#define	CPUSTRING	"win-AXP-debug"
#endif
#endif

#define ID_INLINE __inline 

int __cdecl ShortSwap(__int16 l);
int __cdecl LongSwap(int l);

static ID_INLINE short BigShort(short l) { return ShortSwap(l); }
#define LittleShort
static ID_INLINE int BigLong(int l) { return LongSwap(l); }
#define LittleLong
//static ID_INLINE float BigFloat(const float *l) { FloatSwap(l); }
#define LittleFloat

#define	PATH_SEP '\\'

#endif // WIN32

#define PI_DIV_180		0.017453292519943295769236907684886
#define INV_PI_DIV_180	57.295779513082320876798154814105

// Punish Aurelio if you don't like these performance enhancements. :-)
#define DEG2RAD( a ) ( ( (a) * PI_DIV_180 ) )
#define RAD2DEG( a ) ( ( (a) * INV_PI_DIV_180 ) )

//=============================================================

typedef unsigned long		ulong;
typedef unsigned short		word;

typedef unsigned char 		byte;

typedef const char* LPCSTR;

typedef enum { qfalse, qtrue }	qboolean;
#define	qboolean	int		//don't want strict type checking on the qboolean

typedef int		qhandle_t;
typedef int		thandle_t;
typedef int		fxHandle_t;
typedef int		sfxHandle_t;
typedef int		fileHandle_t;
typedef int		clipHandle_t;

#define ptype_int intptr_t // LWSS: this type is used in cod4


#ifndef NULL
#define NULL ((void *)0)
#endif

#define	MAX_QINT			0x7fffffff
#define	MIN_QINT			(-MAX_QINT-1)


// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	256		// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024


#define	MAX_QPATH			64		// max length of a quake game pathname
#define	MAX_OSPATH			260		// max length of a filesystem pathname

#define	MAX_NAME_LENGTH		32		// max length of a client name

#ifdef KISAK_SP
#define	MAX_GENTITIES		(2176) // 0x880
#elif KISAK_MP
#define	MAX_GENTITIES		(1024) // 0x400
#endif

#define ENTITYNUM_WORLD (MAX_GENTITIES - 2)
#define ENTITYNUM_NONE (MAX_GENTITIES - 1)

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
	// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility

// Light Style Constants

#define LS_STYLES_START			0
#define LS_NUM_STYLES			32
#define	LS_SWITCH_START			(LS_STYLES_START+LS_NUM_STYLES)
#define LS_NUM_SWITCH			32
#define MAX_LIGHT_STYLES		64

// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;

// font rendering values used by ui and cgame
#define PROP_GAP_WIDTH			2
//#define PROP_GAP_WIDTH			3
#define PROP_SPACE_WIDTH		4
#define PROP_HEIGHT				16

#define PROP_TINY_SIZE_SCALE	1
#define PROP_SMALL_SIZE_SCALE	1
#define PROP_BIG_SIZE_SCALE		1
#define PROP_GIANT_SIZE_SCALE	2

#define PROP_TINY_HEIGHT		10
#define PROP_GAP_TINY_WIDTH		1
#define PROP_SPACE_TINY_WIDTH	3

#define PROP_BIG_HEIGHT			24
#define PROP_GAP_BIG_WIDTH		3
#define PROP_SPACE_BIG_WIDTH	6


#define BLINK_DIVISOR			600
#define PULSE_DIVISOR			75

#define UI_LEFT			0x00000000	// default
#define UI_CENTER		0x00000001
#define UI_RIGHT		0x00000002
#define UI_FORMATMASK	0x00000007
#define UI_SMALLFONT	0x00000010
#define UI_BIGFONT		0x00000020	// default
#define UI_GIANTFONT	0x00000040
#define UI_DROPSHADOW	0x00000800
#define UI_BLINK		0x00001000
#define UI_INVERSE		0x00002000
#define UI_PULSE		0x00004000
#define UI_UNDERLINE	0x00008000
#define UI_TINYFONT		0x00010000


// stuff for TA's ROQ cinematic code...
//
#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16



//=============================================
void I_strncat(char* dest, int size, const char* src);
void I_strncpyz(char* dest, const char* src, int destsize);
int I_stricmp(const char* s0, const char* s1);
int I_strnicmp(const char* s0, const char* s1, int n);
const char *__cdecl I_stristr(const char *s0, const char *substr);

bool I_islower(int c);
bool I_isupper(int c);
bool I_isalpha(int c);

bool I_iscsym(int c);

bool __cdecl I_isdigit(int c);
bool __cdecl I_isalnum(int c);
bool __cdecl I_isforfilename(int c);
int __cdecl I_strncmp(const char *s0, const char *s1, int n);
int __cdecl I_strcmp(const char *s0, const char *s1);
int __cdecl I_stricmpwild(const char *wild, const char *s);
char *__cdecl I_strlwr(char *s);
char *__cdecl I_strupr(char *s);
int __cdecl I_DrawStrlen(const char *str);
char *__cdecl I_CleanStr(char *string);
unsigned __int8 __cdecl I_CleanChar(unsigned __int8 character);

// lwss: fcking winblows!
#undef MAKEWORD
#undef MAKELONG
#undef LOWORD
#undef HIWORD
#undef LOBYTE
#undef HIBYTE

#if defined(__GNUC__)
typedef          long long ll;
typedef unsigned long long ull;
#define __int64 long long
#define __int32 int
#define __int16 short
#define __int8  char
#define MAKELL(num) num ## LL
#define FMT_64 "ll"
#elif defined(_MSC_VER)
typedef          __int64 ll;
typedef unsigned __int64 ull;
#define MAKELL(num) num ## i64
#define FMT_64 "I64"
#elif defined (__BORLANDC__)
typedef          __int64 ll;
typedef unsigned __int64 ull;
#define MAKELL(num) num ## i64
#define FMT_64 "L"
#else
#error "unknown compiler"
#endif
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef   signed char   int8; // LWSS: slight changes here made to conform with steam api...
typedef   signed char   sint8;
typedef unsigned char   uint8;
typedef          short  int16;
typedef   signed short  sint16;
typedef unsigned short  uint16;
typedef __int32				int32;
typedef unsigned __int32 uint32;
typedef signed long			sint32;
typedef ll              int64;
typedef ll              sint64;
typedef ull             uint64;

// Partially defined types. They are used when the decompiler does not know
// anything about the type except its size.
#define _BYTE  uint8
#define _WORD  uint16
#define _DWORD uint32
#define _QWORD uint64
#if !defined(_MSC_VER)
#define _LONGLONG __int128
#endif

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define M_PI_HALF (M_PI / 2.0) // LWSS ADD

// Some convenience macros to make partial accesses nicer
#define LAST_IND(x,part_type)    (sizeof(x)/sizeof(part_type) - 1)
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
#  define LOW_IND(x,part_type)   LAST_IND(x,part_type)
#  define HIGH_IND(x,part_type)  0
#else
#  define HIGH_IND(x,part_type)  LAST_IND(x,part_type)
#  define LOW_IND(x,part_type)   0
#endif
// first unsigned macros:
#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
#define WORDn(x, n)   (*((_WORD*)&(x)+n))
#define DWORDn(x, n)  (*((_DWORD*)&(x)+n))

#define LOBYTE(x)  BYTEn(x,LOW_IND(x,_BYTE))
#define LOWORD(x)  WORDn(x,LOW_IND(x,_WORD))
#define LODWORD(x) DWORDn(x,LOW_IND(x,_DWORD))
#define HIBYTE(x)  BYTEn(x,HIGH_IND(x,_BYTE))
#define HIWORD(x)  WORDn(x,HIGH_IND(x,_WORD))
#define HIDWORD(x) DWORDn(x,HIGH_IND(x,_DWORD))
#define BYTE1(x)   BYTEn(x,  1)         // byte 1 (counting from 0)
#define BYTE2(x)   BYTEn(x,  2)
#define BYTE3(x)   BYTEn(x,  3)
#define BYTE4(x)   BYTEn(x,  4)
#define BYTE5(x)   BYTEn(x,  5)
#define BYTE6(x)   BYTEn(x,  6)
#define BYTE7(x)   BYTEn(x,  7)
#define BYTE8(x)   BYTEn(x,  8)
#define BYTE9(x)   BYTEn(x,  9)
#define BYTE10(x)  BYTEn(x, 10)
#define BYTE11(x)  BYTEn(x, 11)
#define BYTE12(x)  BYTEn(x, 12)
#define BYTE13(x)  BYTEn(x, 13)
#define BYTE14(x)  BYTEn(x, 14)
#define BYTE15(x)  BYTEn(x, 15)
#define WORD1(x)   WORDn(x,  1)
#define WORD2(x)   WORDn(x,  2)         // third word of the object, unsigned
#define WORD3(x)   WORDn(x,  3)
#define WORD4(x)   WORDn(x,  4)
#define WORD5(x)   WORDn(x,  5)
#define WORD6(x)   WORDn(x,  6)
#define WORD7(x)   WORDn(x,  7)

// now signed macros (the same but with sign extension)
#define SBYTEn(x, n)   (*((int8*)&(x)+n))
#define SWORDn(x, n)   (*((int16*)&(x)+n))
#define SDWORDn(x, n)  (*((int32*)&(x)+n))

#define SLOBYTE(x)  SBYTEn(x,LOW_IND(x,int8))
#define SLOWORD(x)  SWORDn(x,LOW_IND(x,int16))
#define SLODWORD(x) SDWORDn(x,LOW_IND(x,int32))
#define SHIBYTE(x)  SBYTEn(x,HIGH_IND(x,int8))
#define SHIWORD(x)  SWORDn(x,HIGH_IND(x,int16))
#define SHIDWORD(x) SDWORDn(x,HIGH_IND(x,int32))
#define SBYTE1(x)   SBYTEn(x,  1)
#define SBYTE2(x)   SBYTEn(x,  2)
#define SBYTE3(x)   SBYTEn(x,  3)
#define SBYTE4(x)   SBYTEn(x,  4)
#define SBYTE5(x)   SBYTEn(x,  5)
#define SBYTE6(x)   SBYTEn(x,  6)
#define SBYTE7(x)   SBYTEn(x,  7)
#define SBYTE8(x)   SBYTEn(x,  8)
#define SBYTE9(x)   SBYTEn(x,  9)
#define SBYTE10(x)  SBYTEn(x, 10)
#define SBYTE11(x)  SBYTEn(x, 11)
#define SBYTE12(x)  SBYTEn(x, 12)
#define SBYTE13(x)  SBYTEn(x, 13)
#define SBYTE14(x)  SBYTEn(x, 14)
#define SBYTE15(x)  SBYTEn(x, 15)
#define SWORD1(x)   SWORDn(x,  1)
#define SWORD2(x)   SWORDn(x,  2)
#define SWORD3(x)   SWORDn(x,  3)
#define SWORD4(x)   SWORDn(x,  4)
#define SWORD5(x)   SWORDn(x,  5)
#define SWORD6(x)   SWORDn(x,  6)
#define SWORD7(x)   SWORDn(x,  7)

// Generate a pair of operands. S stands for 'signed'
#define __SPAIR16__(high, low)  (((int16)  (high) <<  8) | (uint8) (low))
#define __SPAIR32__(high, low)  (((int32)  (high) << 16) | (uint16)(low))
#define __SPAIR64__(high, low)  (((int64)  (high) << 32) | (uint32)(low))
#define __SPAIR128__(high, low) (((int128) (high) << 64) | (uint64)(low))
#define __PAIR16__(high, low)   (((uint16) (high) <<  8) | (uint8) (low))
#define __PAIR32__(high, low)   (((uint32) (high) << 16) | (uint16)(low))
#define __PAIR64__(high, low)   (((uint64) (high) << 32) | (uint32)(low))
#define __PAIR128__(high, low)  (((uint128)(high) << 64) | (uint64)(low))

// rotate left
template<class T> T __ROL__(T value, int count)
{
	const uint nbits = sizeof(T) * 8;

	if (count > 0)
	{
		count %= nbits;
		T high = value >> (nbits - count);
		if (T(-1) < 0) // signed value
			high &= ~((T(-1) << count));
		value <<= count;
		value |= high;
	}
	else
	{
		count = -count % nbits;
		T low = value << (nbits - count);
		value >>= count;
		value |= low;
	}
	return value;
}

inline uint8  __ROL1__(uint8  value, int count) { return __ROL__((uint8)value, count); }
inline uint16 __ROL2__(uint16 value, int count) { return __ROL__((uint16)value, count); }
inline uint32 __ROL4__(uint32 value, int count) { return __ROL__((uint32)value, count); }
inline uint64 __ROL8__(uint64 value, int count) { return __ROL__((uint64)value, count); }
inline uint8  __ROR1__(uint8  value, int count) { return __ROL__((uint8)value, -count); }
inline uint16 __ROR2__(uint16 value, int count) { return __ROL__((uint16)value, -count); }
inline uint32 __ROR4__(uint32 value, int count) { return __ROL__((uint32)value, -count); }
inline uint64 __ROR8__(uint64 value, int count) { return __ROL__((uint64)value, -count); }


//=============================================
void Com_Memset(void* dest, const int val, const size_t count);
void Com_Memcpy(void* dest, const void* src, const size_t count);

char* QDECL va(const char* format, ...);


//=============================================
enum DvarType
{
	DVAR_TYPE_BOOL = 0x0,
	DVAR_TYPE_FLOAT = 0x1,
	DVAR_TYPE_FLOAT_2 = 0x2,
	DVAR_TYPE_FLOAT_3 = 0x3,
	DVAR_TYPE_FLOAT_4 = 0x4,
	DVAR_TYPE_INT = 0x5,
	DVAR_TYPE_ENUM = 0x6,
	DVAR_TYPE_STRING = 0x7,
	DVAR_TYPE_COLOR = 0x8,
	DVAR_TYPE_COUNT = 0x9,
};

enum DvarFlags : uint16
{
	DVAR_NOFLAG             = 0x0,
	DVAR_ARCHIVE            = 0x1,  // will be saved to config(_mp).cfg
	DVAR_USERINFO           = 0x2,  // sent to server on connect or change
	DVAR_SERVERINFO         = 0x4,  // sent in response to front end requests
	DVAR_SYSTEMINFO         = 0x8,  // this is sent (replicated) to all clients if you are host
	DVAR_INIT               = 0x10, // don't allow change from console at all (i think)
	DVAR_LATCH              = 0x20, // will only change when C code next does
                                    // a Cvar_Get(), so it can't be changed
                                    // without proper initialization.  modified
                                    // will be set, even though the value hasn't changed yet
	
	DVAR_ROM                = 0x40, // display only, cannot be set by user at all (can be set by code)
	DVAR_CHEAT              = 0x80, // can not be changed if cheats are disabled

	// DVAR_AUTOEXEC, DVAR_SAVED, and DVAR_CHANGEABLE_RESET are not 100% sure
	DVAR_TEMP               = 0x100,
	DVAR_AUTOEXEC           = 0x200,
	DVAR_NORESTART          = 0x400,    // do not clear when a cvar_restart is issued
	DVAR_SAVED              = 0x1000,
	DVAR_EXTERNAL           = 0x4000,   // created by a set command or setclientdvar
	DVAR_CHANGEABLE_RESET   = 0x8000,
};

union DvarValue 
{                
	DvarValue()
	{
		integer = 0;
	}
	DvarValue(int i)
	{
		integer = i;
	}
	DvarValue(bool b)
	{
		enabled = b;
	}
	DvarValue(float f)
	{
		value = f;
	}
	DvarValue(const char *str)
	{
		string = str;
	}
	DvarValue(char *str)
	{
		string = str;
	}
    bool enabled;
    int integer;
    unsigned int unsignedInt;
    float value;
    float vector[4];
    const char *string;
    byte color[4];
};
struct DvarLimits_Enumeration
{
	int stringCount;
	const char** strings;
};
struct DvarLimits_Integer
{
	int min;
	int max;
};
struct DvarLimits_Value
{
	float min; 
	float max;
};
struct DvarLimits_Vector
{
	float min;
	float max;
};
union DvarLimits
{
	// LWSS: KISAKTODO double check this...
	DvarLimits()
	{
		integer.min = INT_MIN;
		integer.max = INT_MAX;
	}
	DvarLimits(uint64 val)
	{
		integer.max = HIDWORD(val);
		integer.min = LODWORD(val);
	}
	DvarLimits(int min, int max)
	{
		integer.min = min; 
		integer.max = max;
	}
	DvarLimits_Enumeration enumeration;
	DvarLimits_Integer integer;
	DvarLimits_Value value;
	DvarLimits_Vector vector;
};

// nothing outside the Dvar_*() functions should modify these fields!
struct dvar_s {
	const char *name;
	const char *description;
	word flags;
	byte type;
	bool modified;
	DvarValue current;
	DvarValue latched;
	DvarValue reset;
	DvarLimits domain;
	bool (__cdecl *domainFunc)(dvar_s*, DvarValue);
	dvar_s* hashNext;
};

// lol
using dvar_t = dvar_s;

//=============================================

struct cspField_t // sizeof=0xC
{                                       // ...
	const char *szName;                 // ...
	int iOffset;                        // ...
	int iFieldType;                     // ...
};

union FloatWriteSwap_union // sizeof=0x4
{                                       // ...
	float f;
	int n;
	unsigned __int8 b[4];
};

union FloatReadSwap_union // sizeof=0x4
{                                       // ...
	float f;
	int n;
	unsigned __int8 b[4];
};

static const float colorBlack[4] = { 0.0, 0.0, 0.0, 1.0 }; // idb
static const float colorRed[4] = { 1.0, 0.0, 0.0, 1.0 }; // idb
static const float colorGreen[4] = { 0.0, 1.0, 0.0, 1.0 }; // idb
static const float colorLtGreen[4] = { 0.0, 0.69999999f, 0.0, 1.0 }; // idb
static const float colorBlue[4] = { 0.0, 0.0, 1.0, 1.0 }; // idb
static const float colorLtBlue[4] = { 0.5, 0.5, 1.0, 1.0 }; // idb
static const float colorYellow[4] = { 1.0, 1.0, 0.0, 1.0 }; // idb
static const float colorLtYellow[4] = { 0.75, 0.75f, 0.0f, 1.0f };
static const float colorMdYellow[4] = { 0.5, 0.5, 0.0, 1.0 }; // idb
static const float colorMagenta[4] = { 1.0, 0.0, 1.0, 1.0 }; // idb
static const float colorCyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
static const float colorLtCyan[4] = { 0.0, 0.75, 0.75, 1.0 }; // idb
static const float colorMdCyan[4] = { 0.0f, 0.5f, 0.5f, 1.0f };
static const float colorDkCyan[4] = { 0.0f, 0.25f, 0.25f, 1.0f };
static const float colorWhite[4] = { 1.0, 1.0, 1.0, 1.0 }; // idb
static const float colorLtGrey[4] = { 0.75, 0.75, 0.75, 1.0 }; // idb
static const float colorMdGrey[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
static const float colorDkGrey[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
static const float colorOrange[4] = { 1.0, 0.69999999f, 0.0, 1.0 }; // idb
static const float colorLtOrange[4] = { 0.75, 0.52499998f, 0.0, 1.0 }; // idb
static const float colorWhiteFaded[4] = { 1.0, 1.0, 1.0, 0.75 }; // idb
static const float colorGreenFaded[4] = { 0.0, 1.0, 0.0, 0.75 }; // idb
static const float colorRedFaded[4] = { 0.75, 0.25, 0.0, 0.75 }; // idb

void __cdecl TRACK_q_shared();
unsigned __int8 __cdecl ColorIndex(unsigned __int8 c);
const char *__cdecl Com_GetFilenameSubString(const char *pathname);
void __cdecl Com_AssembleFilepath(char *folder, char *name, char *extension, char *path, int maxCharCount);
const char *__cdecl Com_GetExtensionSubString(const char *filename);
void __cdecl Com_StripExtension(char *in, char *out);
void __cdecl Com_DefaultExtension(char *path, unsigned int maxSize, const char *extension);
__int16 __cdecl BigShort(__int16 l);
int __cdecl ShortSwap(__int16 l);
__int16 __cdecl ShortNoSwap(__int16 l);
int __cdecl LongSwap(int l);
unsigned __int64 __cdecl Long64Swap(unsigned __int64 l);
unsigned __int64 __cdecl Long64NoSwap(unsigned __int64 ll);
double __cdecl FloatReadSwap(int n);
double __cdecl FloatReadNoSwap(int n);
FloatWriteSwap_union __cdecl FloatWriteSwap(float f);
void __cdecl Swap_InitLittleEndian();
void __cdecl Swap_InitBigEndian();
void __cdecl Swap_Init();


int Com_sprintf(char *dest, unsigned int size, const char *fmt, ...);
int Com_sprintfPos(char *dest, int destSize, int *destPos, const char *fmt, ...);

bool __cdecl CanKeepStringPointer(const char *string);
void __cdecl Com_InitThreadData(int threadContext);
const char *__cdecl Info_ValueForKey(const char *s, const char *key);
void __cdecl Info_NextPair(const char **head, char *key, char *value);
void __cdecl Info_RemoveKey(const char *s, const char *key);
void __cdecl Info_RemoveKey_Big(const char *s, const char *key);
bool __cdecl Info_Validate(const char *s);
void __cdecl Info_SetValueForKey(char *s, const char *key, const char *value);
void __cdecl Info_SetValueForKey_Big(char *s, const char *key, const char *value);
bool __cdecl ParseConfigStringToStruct(
	unsigned __int8 *pStruct,
	const cspField_t *pFieldList,
	int iNumFields,
	char *pszBuffer,
	int iMaxFieldTypes,
	int(__cdecl *parseSpecialFieldType)(unsigned __int8 *, const char *, const int),
	void(__cdecl *parseStrcpy)(unsigned __int8 *, const char *));
bool __cdecl ParseConfigStringToStructCustomSize(
	unsigned __int8 *pStruct,
	const cspField_t *pFieldList,
	int iNumFields,
	char *pszBuffer,
	int iMaxFieldTypes,
	int(__cdecl *parseSpecialFieldType)(unsigned __int8 *, const char *, const int),
	void(__cdecl *parseStrcpy)(unsigned __int8 *, const char *));
double __cdecl GetLeanFraction(float fFrac);
double __cdecl UnGetLeanFraction(float fFrac);
void __cdecl AddLeanToPosition(float *position, float fViewYaw, float fLeanFrac, float fViewRoll, float fLeanDist);
bool __cdecl Com_IsLegacyXModelName(const char *name);
unsigned int __cdecl LongNoSwap(unsigned int color);

#define arr_esize(a) (sizeof((a)[0]))
#define arr_cnt(a) (sizeof(a)/arr_esize(a))
#define ARRAY_COUNT(a) arr_cnt(a)
struct va_info_t
{
	char va_string[2][1024];
	int index;
};


struct TraceCheckCount
{
	int global;
	int *partitions;
};
struct TraceThreadInfo
{
	TraceCheckCount checkcount;
	struct cbrush_t *box_brush;
	struct cmodel_t *box_model;
};

enum TraceHitType : __int32
{                                       // ...
	TRACE_HITTYPE_NONE = 0x0,
	TRACE_HITTYPE_ENTITY = 0x1,
	TRACE_HITTYPE_DYNENT_MODEL = 0x2,
	TRACE_HITTYPE_DYNENT_BRUSH = 0x3,
};

#define SURF_INVALID -1

struct trace_t // sizeof=0x2C
{   
	trace_t()
	{
		memset(this, 0, sizeof(trace_t)); // Blops added this, might help
	}
	// ...
	float fraction;                     // ...
	float normal[3];                    // ...
	int surfaceFlags;                   // ...
	int contents;                       // ...
	const char *material;               // ...
	TraceHitType hitType;               // ...
	unsigned __int16 hitId;
	unsigned __int16 modelIndex;        // ...
	unsigned __int16 partName;          // ...
	unsigned __int16 partGroup;         // ...
	bool allsolid;                      // ...
	bool startsolid;                    // ...
	bool walkable;                      // ...
	// padding byte
};

// win_shared
unsigned int __cdecl Sys_Milliseconds();
unsigned int __cdecl Sys_MillisecondsRaw();
void __cdecl Sys_SnapVector(float *v);

// com_shared
struct qtime_s // sizeof=0x24
{                                       // ...
	int tm_sec;
	int tm_min;                         // ...
	int tm_hour;                        // ...
	int tm_mday;                        // ...
	int tm_mon;                         // ...
	int tm_year;                        // ...
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};
char __cdecl Com_Filter(const char *filter, char *name, int casesensitive);
char __cdecl Com_FilterPath(const char *filter, const char *name, int casesensitive);
int __cdecl Com_HashKey(const char *string, int maxlen);
int __cdecl Com_RealTime(qtime_s *qtime);
//void __cdecl Com_Memcpy(char *dest, char *src, int count);
//void __cdecl Com_Memset(unsigned int *dest, int val, int count);

inline bool __cdecl Com_BitCheckAssert(const unsigned int *array, int bitNum, int size)
{
	iassert(array);
	iassert(bitNum < (8 * size));
	
	return (array[bitNum >> 5] & (1 << (bitNum & 0x1F))) != 0;
}

inline void __cdecl Com_BitClearAssert(unsigned int *array, int bitNum, int size)
{
	iassert(array);
	iassert(bitNum < (8 * size));

	array[bitNum >> 5] &= ~(1 << (bitNum & 0x1F));
}

inline void __cdecl Com_BitSetAssert(unsigned int *array, int bitNum, int size)
{
	iassert(array);
	iassert(bitNum < (8 * size));
	
	array[bitNum >> 5] |= 1 << (bitNum & 0x1F);
}

enum trType_t : __int32
{                                       // XREF: trajectory_t/r
	TR_STATIONARY = 0x0,
	TR_INTERPOLATE = 0x1,
	TR_LINEAR = 0x2,
	TR_LINEAR_STOP = 0x3,
	TR_SINE = 0x4,
	TR_GRAVITY = 0x5,
	TR_ACCELERATE = 0x6,
	TR_DECELERATE = 0x7,
	TR_PHYSICS = 0x8,
	TR_FIRST_RAGDOLL = 0x9,
	TR_RAGDOLL = 0x9,
	TR_RAGDOLL_GRAVITY = 0xA,
	TR_RAGDOLL_INTERPOLATE = 0xB,
	TR_LAST_RAGDOLL = 0xB,
};

struct trajectory_t // sizeof=0x24 // (SP/MP same)
{                                       // XREF: LerpEntityState/r
	trType_t trType;                    // XREF: ScriptMover_SetupMoveSpeed+563/w
	int trTime;                         // XREF: ScriptMover_SetupMoveSpeed+570/w
	int trDuration;                     // XREF: ScriptMover_SetupMoveSpeed+583/w
	float trBase[3];                    // XREF: Mantle_FindMantleSurface+244/o
	float trDelta[3];                   // XREF: CountBitsEnabled(uint)+1B/o
};

inline bool __cdecl Com_IsRagdollTrajectory(const trajectory_t *trajectory)
{
	iassert(trajectory);
	
	return trajectory->trType >= TR_FIRST_RAGDOLL && trajectory->trType <= TR_RAGDOLL_INTERPOLATE;
}


// com_stringtable
struct StringTable // sizeof=0x10
{                                       // ...
	const char *name;
	int columnCount;
	int rowCount;
	const char **values;
};
static_assert(sizeof(StringTable) == 16);

const char *__cdecl StringTable_GetColumnValueForRow(const StringTable *table, int row, int column);
const char *__cdecl StringTable_Lookup(
	const StringTable *table,
	int comparisonColumn,
	const char *value,
	int valueColumn);
int __cdecl StringTable_LookupRowNumForValue(const StringTable *table, int comparisonColumn, const char *value);

union XAssetHeader;
void __cdecl StringTable_GetAsset(const char *filename, StringTable **tablePtr);


extern const dvar_t *useFastFile;

inline bool IsFastFileLoad()
{
#ifdef KISAK_NO_FASTFILES
	return false;
#endif
	return useFastFile->current.enabled;
}


extern unsigned __int64(__cdecl *LittleLong64)(unsigned __int64);