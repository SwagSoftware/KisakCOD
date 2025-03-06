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
#include <time.h>
#include <ctype.h>

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


#define	PATH_SEP '\\'

#endif // WIN32

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

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_DISCONNECT,				// don't kill server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;

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
void I_strncat(char* dest, int size, char* src);
void I_strncpyz(char* dest, char* src, int destsize);
int I_stricmp(const char* s0, const char* s1);
int I_strnicmp(const char* s0, const char* s1, int n);

bool I_islower(int c);
bool I_isupper(int c);
bool I_isalpha(int c);


//=============================================
#define DVAR_WRITEPROTECT	0x10			
#define	DVAR_LATCH			0x20			// will only change when C code next does
											// a Cvar_Get(), so it can't be changed
											// without proper initialization.  modified
											// will be set, even though the value hasn't
											// changed yet
#define DVAR_ROM   0x40						// display only, cannot be set by user at all (can be set by code)
#define DVAR_CHEAT 0x80						// can not be changed if cheats are disabled

#define DVAR_AUTOEXEC 0x200 // not 100% sure
#define DVAR_NORESTART 0x400		// do not clear when a cvar_restart is issued

#define DVAR_SAVED 0x1000 // not 100% sure
#define DVAR_EXTERNAL 0x4000

#define DVAR_CHANGEABLE_RESET 0x8000 // not 100% sure

enum
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

union DvarValue 
{                
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
	DvarLimits_Enumeration enumeration;
	DvarLimits_Integer integer;
	DvarLimits_Value value;
	DvarLimits_Vector vector;
};

// nothing outside the Dvar_*() functions should modify these fields!
typedef struct dvar_s {
	const char *name;
	const char *description;
	word flags;
	byte type;
	bool modified;
	DvarValue current;
	DvarValue latched;
	DvarValue reset;
	DvarLimits domain;
	bool (__cdecl *domainFunc)(dvar_s *, DvarValue);
	dvar_s *hashNext;
} dvar_t;