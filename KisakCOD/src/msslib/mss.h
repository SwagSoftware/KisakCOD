//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSS.H: Miles Sound System main header file                            ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Initial, derived from AIL.H V3.02          ##
//##          1.01 of 19-Jun-95: Added various functions for V3.03 release  ##
//##          1.02 of 22-Nov-95: C++ typedef problem fixed, declspecs added ##
//##          1.03 of 15-Feb-96: Changes for 16 bit callbacks and multiple  ##
//##                             16 bit DLL loads (JKR)                     ##
//##          1.04 of  2-Nov-97: Changes made to handle DLS in future       ##
//##                             versions                                   ##
//##          1.05 of  1-Jan-98: Massive changes for version 4.0            ##
//##          1.06 of 17-Sep-98: Massive changes for version 5.0            ##
//##          1.07 of  2-Feb-99: Changes for new input API                  ##
//##          1.08 of  8-Feb-99: Changes for new filter helper functions    ##
//##          1.09 of  8-Feb-03: Changes for xbox and linux                 ##
//##          1.10 of 15-May-05: Massive changes for version 7.0            ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#ifndef MSS_VERSION

#define MSS_VERSION      "7.2e"     // Version string needs to be 4 characters long for benefit of MIDIECHW and SETSOUND 
#define MSS_MAJOR_VERSION 7
#define MSS_MINOR_VERSION 2
#define MSS_SUB_VERSION   4
#define MSS_VERSION_DATE "30-Oct-08"

#define MSS_COPYRIGHT "Copyright (C) 1991-2008, RAD Game Tools, Inc."

#endif

#ifndef MSS_H
#define MSS_H

// IS_DOS for DOS
// IS_WINDOWS for Windows or Win32
// IS_WIN64 for Win64
// IS_WIN32 for Win32
// IS_WIN16 for Windows
// IS_WIN32API for Windows, Xbox and Xenon
// IS_64REG when CPU registers are 64-bit - Xenon, PS3, Win64 and PS2
// IS_32 for 32-bit DOS or Win32
// IS_16 for 16-bit Windows
// IS_LE for little endian (PCs)
// IS_BE for big endian (Macs, x360, ps3)
// IS_X86 for Intel
// IS_MAC for Mac
// IS_MACHO for Macho Mac
// IS_PPC for PPC Mac
// IS_68K for 68K Mac
// IS_LINUX for Linux
// IS_XBOX for Xbox
// IS_XENON for Xbox 360
// IS_PS2 for PS/2
// IS_PS3 for PS/3
// IS_WII for Wii
// IS_STATIC for static versions (DOS, Xbox, Xbox 360, GameCube, PS2, PS3, Wii)


#ifdef IS_DOS
#undef IS_DOS
#endif

#ifdef IS_WINDOWS
#undef IS_WINDOWS
#endif

#ifdef IS_WIN32
#undef IS_WIN32
#endif

#ifdef IS_WIN64
#undef IS_WIN64
#endif

#ifdef IS_WIN16
#undef IS_WIN16
#endif

#ifdef IS_32
#undef IS_32
#endif

#ifdef IS_16
#undef IS_16
#endif

#ifdef IS_LE
#undef IS_LE
#endif

#ifdef IS_BE
#undef IS_BE
#endif

#ifdef IS_X86
#undef IS_X86
#endif

#ifdef IS_MAC
#undef IS_MAC
#endif

#ifdef IS_PPC
#undef IS_PPC
#endif

#ifdef IS_68K
#undef IS_68K
#endif

#ifdef IS_LINUX
#undef IS_LINUX
#endif

#ifdef IS_STATIC
#undef IS_STATIC
#endif

#ifdef IS_XBOX
#undef IS_XBOX
#endif

#ifdef IS_XENON
#undef IS_XENON
#endif

#undef MSSRESTRICT
#define MSSRESTRICT

#if defined(R5900)
  #define IS_PS2
  #define IS_32
  #define IS_64REGS
  #define IS_STATIC
  #define IS_LE
  #undef MSSRESTRICT
  #define MSSRESTRICT __restrict
#elif defined(__CELLOS_LV2__)

  #define IS_PS3
  #define IS_32
  #define IS_64REGS
  #define IS_STATIC
  #define IS_BE
  #define IS_PPC

  #ifndef __LP32__
  #error "PS3 32bit ABI support only"
  #endif

  #undef MSSRESTRICT
  #define MSSRESTRICT __restrict__
  
#elif defined(HOLLYWOOD_REV) || defined(REVOLUTION) 

  #define IS_WII
  #define IS_32
  #define IS_STATIC
  #define IS_BE
  #define IS_PPC

  #undef MSSRESTRICT
  #define MSSRESTRICT
  
#elif defined( __DOS__ )
  #define IS_DOS
  #define IS_32
  #define IS_LE
  #define IS_X86
  #define IS_STATIC
#else
  #if defined(_XENON) || (_XBOX_VER == 200)

    #undef MSSRESTRICT
    #define MSSRESTRICT __restrict

    // Remember that Xenon also defines _XBOX
    #define IS_WIN32API
    #define IS_32
    #define IS_64REGS
    #define IS_BE
    #define IS_XENON
    #define IS_STATIC
    #define IS_PPC

  #else
    #ifdef _XBOX
      #define IS_WIN32API
      #define IS_32
      #define IS_LE
      #define IS_X86
      #define IS_XBOX
      #define IS_STATIC
    #else
      #if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(__NT__) || defined(__WIN32__)
        #define IS_WIN32API
        #define IS_WINDOWS
        #define IS_WIN32
        #define IS_32
        #define IS_LE
        #define IS_X86
        #if defined(_WIN64)      // We consider Win64 to be a superset of Win32!
          #define IS_WIN64
          #define IS_64REGS
        #endif

        #if _MSC_VER >= 1400
          #undef MSSRESTRICT
          #define MSSRESTRICT __restrict
        #endif

      #else
        #ifdef _WINDOWS
          #define IS_WINDOWS
          #define IS_WIN16
          #define IS_16
          #define IS_LE
          #define IS_X86
        #else
          #if defined(_WINDLL) || defined(WINDOWS) || defined(__WINDOWS__) || defined(_Windows)
            #define IS_WINDOWS
            #define IS_WIN16
            #define IS_16
            #define IS_LE
            #define IS_X86
          #else
            #if (defined(__MWERKS__) && !defined(__INTEL__)) || defined(__MRC__) || defined(THINK_C) || defined(powerc) || defined(macintosh) || defined(__powerc) || defined(__APPLE__) || defined(__MACH__)
              #define IS_MAC
              #if TARGET_API_MAC_CARBON
                #define IS_CARBON
              #endif
              #define IS_32

              #if defined(__MACHO__) || defined(__MACH__)
                #define IS_MACHO
              #elif defined(__GNUC__) || defined(__GNUG__)
                #define IS_MACHO
              #endif

              #if defined(__powerc) || defined(powerc) || defined(__POWERPC__)
                #define IS_PPC
                #define IS_BE
              #elif defined(__i386__)
                #define IS_X86
                #define IS_LE
              #else
                #if defined(__MC68K__)
                  #define IS_68K
                  #define IS_BE
                #endif
              #endif

              #ifndef IS_MACHO
                #define ON_MAC_USE_FSS
              #endif
              
            #else
              #ifdef linux
                #define IS_LINUX
                #define IS_32
                #define IS_LE
                #ifdef i386
                  #define IS_X86
                #endif
              #endif
            #endif
          #endif
        #endif
      #endif
    #endif
  #endif
#endif

#if (!defined(IS_LE) && !defined(IS_BE))
  #error MSS.H did not detect your platform.  Define __DOS__, _WINDOWS, WIN32, WIN64, or macintosh.
#endif

#if !defined(IS_PS2) && !defined(IS_PS3)

#if defined(_PUSHPOP_SUPPORTED) || PRAGMA_STRUCT_PACKPUSH
  #pragma pack(push,1)
#else
  #pragma pack(1)
#endif

#endif

//
// Pipeline filters supported on following platforms
//

#if defined(IS_WIN32API) || defined(IS_MAC) || defined(IS_LINUX) || defined(IS_DOS) || defined(IS_PS2) || defined(IS_XENON) || defined(IS_PS3) || defined(IS_WII)
   #define MSS_FLT_SUPPORTED 1
   #define EXTRA_BUILD_BUFFERS 1
   #define FLT_A (MAX_SPEAKERS)

   #if defined(IS_WIN32API) 
      #define MSS_VFLT_SUPPORTED 1
   #endif
#else
   #define EXTRA_BUILD_BUFFERS 0
#endif

#if defined(IS_WIN32) || defined(IS_MAC) || defined(IS_LINUX) || defined(IS_PS2) || defined(IS_XENON) || defined(IS_PS3) || defined(IS_WII)
   #define MSS_REVERB_SUPPORTED 1
#endif

#ifdef __cplusplus
extern "C" {
#define RADDEFSTART extern "C" {
#define RADDEFEND }
#else
#define RADDEFSTART
#define RADDEFEND
#endif

#undef MSS_STRUCT
#define MSS_STRUCT struct

#ifdef IS_PS2

#if !defined(__MWERKS__)
#undef MSS_STRUCT
#define MSS_STRUCT struct __attribute__((__packed__))
#endif

#define AILCALLBACK //$ __attribute__((cdecl))
#define AILEXPORT //$ __attribute__((cdecl))
#define DXDEC extern
#define DXDEF
#define AILCALL //$__attribute__((cdecl))
#define FAR
#define HIWORD(ptr) (((U32)ptr)>>16)
#define LOWORD(ptr) ((U16)((U32)ptr))
#define WINAPI

#define FOURCC U32

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
              ((U32)(U8)(ch0) | ((U32)(U8)(ch1) << 8) |   \
              ((U32)(U8)(ch2) << 16) | ((U32)(U8)(ch3) << 24 ))

#define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)

#define AILLIBCALLBACK //__attribute__((cdecl))

#define MSS_MAIN_DEF

#define MSS_REDIST_DIR_NAME "redist"

#define MSS_DIR_SEP "\\"
#define MSS_DIR_UP ".." MSS_DIR_SEP
#define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP


#elif defined( IS_DOS )

#define AILCALLBACK __pascal
#define AILEXPORT cdecl
#define DXDEC extern
#define DXDEF
#define AILCALL cdecl
#define FAR
#define HIWORD(ptr) (((U32)ptr)>>16)
#define LOWORD(ptr) ((U16)((U32)ptr))

#define FOURCC U32

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
              ((U32)(U8)(ch0) | ((U32)(U8)(ch1) << 8) |   \
              ((U32)(U8)(ch2) << 16) | ((U32)(U8)(ch3) << 24 ))

#define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)

#define AILLIBCALLBACK __pascal

#define MSS_MAIN_DEF

#define MSS_REDIST_DIR_NAME "redist"

#define MSS_DIR_SEP "\\"
#define MSS_DIR_UP ".." MSS_DIR_SEP
#define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

#else

#ifdef IS_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif

#ifndef STRICT
#define STRICT
#endif

typedef char CHAR;
typedef short SHORT;
typedef int                 BOOL;
typedef long LONG;
typedef CHAR *LPSTR, *PSTR;

#ifdef IS_WIN64
  typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#else
  #ifdef _Wp64
    #if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
      typedef __w64 unsigned long ULONG_PTR, *PULONG_PTR;
    #else
      typedef unsigned long ULONG_PTR, *PULONG_PTR;
    #endif
  #else
    typedef unsigned long ULONG_PTR, *PULONG_PTR;
  #endif
#endif

typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef unsigned int        UINT;
typedef struct HWAVE__ *HWAVE;
typedef struct HWAVEIN__ *HWAVEIN;
typedef struct HWAVEOUT__ *HWAVEOUT;
typedef HWAVEIN  *LPHWAVEIN;
typedef HWAVEOUT  *LPHWAVEOUT;

#ifndef WAVE_MAPPER
#define WAVE_MAPPER     ((UINT)-1)
#endif

typedef struct waveformat_tag *LPWAVEFORMAT;

typedef struct HMIDIOUT__ *HMIDIOUT;
typedef HMIDIOUT  *LPHMIDIOUT;
typedef struct HWND__ *HWND;
typedef struct HINSTANCE__ *HINSTANCE;
typedef HINSTANCE HMODULE;      
typedef struct wavehdr_tag *LPWAVEHDR;
typedef DWORD   FOURCC;         /* a four character code */

  #ifndef MAKEFOURCC

  #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((U32)(U8)(ch0) | ((U32)(U8)(ch1) << 8) |   \
                ((U32)(U8)(ch2) << 16) | ((U32)(U8)(ch3) << 24 ))
  #endif

  #ifndef mmioFOURCC
  #define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)
  #endif


#define MSS_MAIN_DEF __cdecl

//
// If compiling MSS DLL, use __declspec(dllexport) for both
// declarations and definitions
//
// If compiling MSS16 library or application, use "extern" in declarations
// and nothing in definitions
//

#ifdef IS_WIN32

  #undef FAR
  #define FAR

  #ifndef FORNONWIN
  #define AILLIBCALLBACK __stdcall
  #define AILCALL        __stdcall
  #define AILCALLBACK    __stdcall
  #define AILEXPORT      __stdcall
  #else
  #define AILLIBCALLBACK __cdecl
  #define AILCALL        __cdecl
  #define AILCALLBACK    __cdecl
  #define AILEXPORT      __cdecl
  #endif
  
  #ifdef __RADINDLL__
    #define DXDEC __declspec(dllexport)
    #define DXDEF __declspec(dllexport)
  #else

    #if defined( __BORLANDC__ ) || defined( MSS_SPU_PROCESS )
      #define DXDEC extern
    #else
      #define DXDEC __declspec(dllimport)
    #endif

  #endif

  #ifdef IS_WIN64
    #define MSSDLLNAME "MSS64.DLL"
    #define MSS_REDIST_DIR_NAME "redist64"
  #else
    #define MSSDLLNAME "MSS32.DLL"
    #define MSS_REDIST_DIR_NAME "redist"
  #endif

  #define MSS_DIR_SEP "\\"
  #define MSS_DIR_UP ".." MSS_DIR_SEP
  #define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

#else

  #define AILCALL WINAPI
  #define AILEXPORT __export WINAPI
  #define AILCALLBACK AILEXPORT
  #define AILLIBCALLBACK WINAPI

  #define DXDEC  extern
  #define DXDEF

  #define MSSDLLNAME "MSS16.DLL"
  #define MSS_REDIST_DIR_NAME "redist"

  #define MSS_DIR_SEP "\\"
  #define MSS_DIR_UP ".." MSS_DIR_SEP
  #define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

#endif

typedef void * LPVOID;
typedef LPVOID AILLPDIRECTSOUND;
typedef LPVOID AILLPDIRECTSOUNDBUFFER;

#else

#ifdef IS_MAC

#if !defined(__FILES__)  && defined( ON_MAC_USE_FSS )
  #define FSSpec void
#endif

#define FAR

  #ifdef IS_X86
    #define AILLIBCALLBACK __attribute__((cdecl))
    #define AILCALL        __attribute__((cdecl))
    #define AILCALLBACK    __attribute__((cdecl))
    #define AILEXPORT      __attribute__((cdecl)) __attribute__((visibility("default")))

    #ifdef __RADINDLL__
      #define DXDEC __attribute__((visibility("default")))
      #define DXDEF
    #else
      #define DXDEC extern
      #define DXDEF
    #endif

  #else
    #define AILLIBCALLBACK 
    #define AILCALL        
    #define AILEXPORT      
    #define AILCALLBACK    

    #ifdef __RADINDLL__
      #define DXDEC __declspec(export)
      #define DXDEF
    #else
      #define DXDEC extern
      #define DXDEF
    #endif

  #endif

#ifdef IS_MACHO

#ifdef IS_X86
  #define MSS_REDIST_DIR_NAME "redist/macx86"
#else
  #define MSS_REDIST_DIR_NAME "redist/macho"
#endif
  
#define MSS_DIR_SEP "/"
#define MSS_DIR_UP ".." MSS_DIR_SEP
#define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

#else

#ifdef IS_CARBON
#define MSS_REDIST_DIR_NAME "redist:carbon"
#define MSSDLLNAME "Miles Carbon Library"
#else
#define MSS_REDIST_DIR_NAME "redist:classic"
#define MSSDLLNAME "Miles Shared Library"
#endif

#define MSS_DIR_SEP ":"
#define MSS_DIR_UP ":" MSS_DIR_SEP
#define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_SEP

#endif

#define HIWORD(ptr) (((U32)ptr)>>16)
#define LOWORD(ptr) ((U16)((U32)ptr))

#define FOURCC U32

#ifndef MAKEFOURCC

#ifdef IS_X86
  #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((U32)(U8)(ch0) | ((U32)(U8)(ch1) << 8) |   \
                ((U32)(U8)(ch2) << 16) | ((U32)(U8)(ch3) << 24 ))
#else

  #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
               (((U32)(U8)(ch0) << 24) | ((U32)(U8)(ch1) << 16) |   \
               ((U32)(U8)(ch2) <<  8) | ((U32)(U8)(ch3)      ))
#endif

#endif

#define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)

#define MSS_MAIN_DEF

#else

#ifdef IS_LINUX

#define FAR

  #ifdef IS_X86
    #define AILLIBCALLBACK __attribute__((cdecl))
    #define AILCALL        __attribute__((cdecl))
    #define AILCALLBACK    __attribute__((cdecl))
    #define AILEXPORT      __attribute__((cdecl))
  #else
    #define AILLIBCALLBACK 
    #define AILCALL        
    #define AILEXPORT      
    #define AILCALLBACK    
  #endif

#define DXDEC extern
#define DXDEF

#define MSS_REDIST_DIR_NAME "redist"
#define MSSDLLNAME "Miles Shared Library"

#define MSS_DIR_SEP "/"
#define MSS_DIR_UP "../"
#define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

#define HIWORD(ptr) (((U32)ptr)>>16)
#define LOWORD(ptr) ((U16)((U32)ptr))

#define FOURCC U32

#ifndef MAKEFOURCC

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
              ((U32)(U8)(ch0) | ((U32)(U8)(ch1) << 8) |   \
              ((U32)(U8)(ch2) << 16) | ((U32)(U8)(ch3) << 24 ))
#endif

#define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)

#define MSS_MAIN_DEF

#else

#ifdef IS_XBOX

  #define FOURCC U32

  #ifndef MAKEFOURCC

  #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((U32)(U8)(ch0) | ((U32)(U8)(ch1) << 8) |   \
                ((U32)(U8)(ch2) << 16) | ((U32)(U8)(ch3) << 24 ))
  #endif

  #define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)

  #undef FAR
  #define FAR

  #define AILLIBCALLBACK __stdcall
  #define AILCALL        __stdcall
  #define AILEXPORT      __stdcall
  #define AILCALLBACK    __stdcall

  #define DXDEC extern
  #define DXDEF

  #define MSS_REDIST_DIR_NAME "redist"

  #define MSS_DIR_SEP "\\"
  #define MSS_DIR_UP ".." MSS_DIR_SEP
  #define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

typedef void * LPVOID;
typedef LPVOID AILLPDIRECTSOUND;
typedef LPVOID AILLPDIRECTSOUNDBUFFER;

#else

#ifdef IS_XENON

  #define FOURCC U32

  #undef MAKEFOURCC  // refine it - the xtl makes a bad one

  #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
        (((U32)(U8)(ch0) << 24) | ((U32)(U8)(ch1) << 16) |   \
              ((U32)(U8)(ch2) <<  8) | ((U32)(U8)(ch3)      ))

  #define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)

  #undef FAR
  #define FAR

  #define AILLIBCALLBACK __stdcall
  #define AILCALL        __stdcall
  #define AILEXPORT      __stdcall
  #define AILCALLBACK    __stdcall

  #define DXDEC extern
  #define DXDEF

  #define MSS_REDIST_DIR_NAME "redist"

  #define MSS_DIR_SEP "\\"
  #define MSS_DIR_UP ".." MSS_DIR_SEP
  #define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

typedef void * AILLPDIRECTSOUND;
typedef void * AILLPDIRECTSOUNDBUFFER;

#else

#ifdef IS_PS3

  #undef MSS_STRUCT
  #define MSS_STRUCT struct __attribute__((__packed__))

  #define FOURCC U32

  #undef MAKEFOURCC  // refine it - the xtl makes a bad one

  #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
        (((U32)(U8)(ch0) << 24) | ((U32)(U8)(ch1) << 16) |   \
              ((U32)(U8)(ch2) <<  8) | ((U32)(U8)(ch3)      ))

  #define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)

  #undef FAR
  #define FAR

  #define AILLIBCALLBACK
  #define AILCALL       
  #define AILEXPORT     
  #define AILCALLBACK   

  #define DXDEC extern
  #define DXDEF 

  #define MSS_REDIST_DIR_NAME "redist"

  #define MSS_DIR_SEP "/"
  #define MSS_DIR_UP ".." MSS_DIR_SEP
  #define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

#elif defined( IS_WII)

  #define FOURCC U32

  #undef MAKEFOURCC  // refine it - the xtl makes a bad one

  #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
        (((U32)(U8)(ch0) << 24) | ((U32)(U8)(ch1) << 16) |   \
              ((U32)(U8)(ch2) <<  8) | ((U32)(U8)(ch3)      ))

  #define mmioFOURCC(w,x,y,z) MAKEFOURCC(w,x,y,z)

  #undef FAR
  #define FAR

  #define AILLIBCALLBACK
  #define AILCALL       
  #define AILEXPORT     
  #define AILCALLBACK   

  #define DXDEC extern
  #define DXDEF 

  #define MSS_REDIST_DIR_NAME "redist"

  #define MSS_DIR_SEP "/"
  #define MSS_DIR_UP ".." MSS_DIR_SEP
  #define MSS_DIR_UP_TWO MSS_DIR_UP MSS_DIR_UP

#endif
#endif

#endif

#endif

#endif

#endif

#endif

#ifndef NULL
#define NULL 0
#endif

//
// Misc. constant definitions
//

#define MAX_DRVRS                16       // Max. # of simultaneous drivers
#define MAX_TIMERS               16       // Max. # of simultaneous timers
#define MAX_NOTES                32       // Max # of notes "on"
#define FOR_NEST                 4        // # of nested XMIDI FOR loops
#define NUM_CHANS                16       // # of possible MIDI channels
#define MAX_W_VOICES             16       // Max virtual wave synth voice cnt
#define MAX_W_ENTRIES            512      // 512 wave library entries max.
#ifdef IS_WIN32
#define MAX_SPEAKERS             9        // Up to 9 hardware output channels supported on Win32
#elif defined(IS_PS3) || defined(IS_PS2) || defined(IS_WII)
#define MAX_SPEAKERS             8        // Up to 8 hardware output channels on PS3, PS2, Wii
#else
#define MAX_SPEAKERS             6        // Up to 6 hardware output channels supported on other platforms
#endif
#define MAX_RECEIVER_SPECS       32       // Up to 32 receiver point specifications

#define MAX_SAVED_FILTER_PROPERTIES 48    // Up to 48 filter attributes can be stored in an HSATTRIBS structure

#define MIN_CHAN                 ( 1-1)   // Min channel recognized (0-based)
#define MAX_CHAN                 (16-1)   // Max channel recognized
#define MIN_LOCK_CHAN            ( 1-1)   // Min channel available for locking
#define MAX_LOCK_CHAN            (16-1)   // Max channel available for locking
#define PERCUSS_CHAN             (10-1)   // Percussion channel (no locking)

#define AIL_MAX_FILE_HEADER_SIZE 8192     // AIL_set_named_sample_file() requires at least 8K
                                          // of data or the entire file image, whichever is less,
                                          // to determine sample format
#define DIG_F_16BITS_MASK          1
#define DIG_F_STEREO_MASK          2
#define DIG_F_ADPCM_MASK           4
#define DIG_F_XBOX_ADPCM_MASK      8
#define DIG_F_MULTICHANNEL_MASK    16
#define DIG_F_OUTPUT_FILTER_IN_USE 32

#define DIG_F_MONO_8                     0        // PCM data formats
#define DIG_F_MONO_16                    (DIG_F_16BITS_MASK)
#define DIG_F_STEREO_8                   (DIG_F_STEREO_MASK)
#define DIG_F_MULTICHANNEL_8             (DIG_F_MULTICHANNEL_MASK)   // (not actually supported)
#define DIG_F_STEREO_16                  (DIG_F_STEREO_MASK|DIG_F_16BITS_MASK)
#define DIG_F_MULTICHANNEL_16            (DIG_F_MULTICHANNEL_MASK|DIG_F_16BITS_MASK)
#define DIG_F_ADPCM_MONO_16              (DIG_F_ADPCM_MASK |DIG_F_16BITS_MASK)
#define DIG_F_ADPCM_STEREO_16            (DIG_F_ADPCM_MASK |DIG_F_16BITS_MASK|DIG_F_STEREO_MASK)
#define DIG_F_ADPCM_MULTICHANNEL_16      (DIG_F_ADPCM_MASK |DIG_F_16BITS_MASK|DIG_F_STEREO_MASK)
#define DIG_F_XBOX_ADPCM_MONO_16         (DIG_F_XBOX_ADPCM_MASK |DIG_F_16BITS_MASK)
#define DIG_F_XBOX_ADPCM_STEREO_16       (DIG_F_XBOX_ADPCM_MASK |DIG_F_16BITS_MASK|DIG_F_STEREO_MASK)
#define DIG_F_XBOX_ADPCM_MULTICHANNEL_16 (DIG_F_XBOX_ADPCM_MASK |DIG_F_16BITS_MASK|DIG_F_MULTICHANNEL_MASK)

#define DIG_F_NOT_8_BITS (DIG_F_16BITS_MASK | DIG_F_ADPCM_MASK | DIG_F_XBOX_ADPCM_MASK | DIG_F_MULTICHANNEL_MASK)

#define DIG_F_USING_ASI          16

#define DIG_PCM_POLARITY         0x0004   // PCM flags used by driver hardware
#define DIG_PCM_SPLIT            0x0008
#define DIG_BUFFER_SERVICE       0x0010
#define DIG_DUAL_DMA             0x0020
#define DIG_RECORDING_SUPPORTED  0x8000

#ifndef WAVE_FORMAT_PCM
  #define WAVE_FORMAT_PCM          1
#endif
#ifndef WAVE_FORMAT_IMA_ADPCM
  #define WAVE_FORMAT_IMA_ADPCM    0x0011
#endif
#ifndef WAVE_FORMAT_XBOX_ADPCM
  #define WAVE_FORMAT_XBOX_ADPCM   0x0069
#endif
#ifndef WAVE_FORMAT_EXTENSIBLE
  #define WAVE_FORMAT_EXTENSIBLE   0xFFFE
#endif

typedef enum
{
   MSS_SPEAKER_FRONT_LEFT            = 0,     // Speaker order indexes correspond to 
   MSS_SPEAKER_FRONT_RIGHT           = 1,     // bitmasks in PSDK's ksmedia.h
   MSS_SPEAKER_FRONT_CENTER          = 2,     // Also see microsoft.com/whdc/device/audio/multichaud.mspx
   MSS_SPEAKER_LOW_FREQUENCY         = 3,
   MSS_SPEAKER_BACK_LEFT             = 4,
   MSS_SPEAKER_BACK_RIGHT            = 5,
   MSS_SPEAKER_FRONT_LEFT_OF_CENTER  = 6,
   MSS_SPEAKER_FRONT_RIGHT_OF_CENTER = 7,
   MSS_SPEAKER_BACK_CENTER           = 8,
   MSS_SPEAKER_SIDE_LEFT             = 9,
   MSS_SPEAKER_SIDE_RIGHT            = 10,
   MSS_SPEAKER_TOP_CENTER            = 11,
   MSS_SPEAKER_TOP_FRONT_LEFT        = 12,
   MSS_SPEAKER_TOP_FRONT_CENTER      = 13,
   MSS_SPEAKER_TOP_FRONT_RIGHT       = 14,
   MSS_SPEAKER_TOP_BACK_LEFT         = 15,
   MSS_SPEAKER_TOP_BACK_CENTER       = 16,
   MSS_SPEAKER_TOP_BACK_RIGHT        = 17,
   MSS_SPEAKER_MAX_INDEX             = 17
}
MSS_SPEAKER;

#ifdef IS_DOS

#define AIL3DIG                  0        // .DIG driver
#define AIL3MDI                  1        // .MDI driver

#define DIG_DETECT_8_BIT_ONLY    0x0001   // Detect 8-bit DMA only
#define DIG_DETECT_16_BIT_ONLY   0x0002   // Detect 16-bit DMA only
#define DIG_DETECT_8_AND_16_BITS 0x0003   // Detect both 8- and 16-bit DMA

#define DRV_INIT          0x300     // Functions common to .MDI and .DIG
#define DRV_GET_INFO      0x301     // drivers
#define DRV_SERVE         0x302
#define DRV_PARSE_ENV     0x303
#define DRV_VERIFY_IO     0x304
#define DRV_INIT_DEV      0x305
#define DRV_SHUTDOWN_DEV  0x306

#define DIG_HW_VOLUME     0x400     // .DIG driver functions
#define DIG_START_P_CMD   0x401
#define DIG_STOP_P_REQ    0x402
#define DIG_START_R_CMD   0x403
#define DIG_STOP_R_REQ    0x404
#define DIG_VSE           0x405

#define MDI_HW_VOLUME     0x500     // .MDI driver functions
#define MDI_INIT_INS_MGR  0x501
#define MDI_MIDI_XMIT     0x502
#define MDI_INSTALL_T_SET 0x503
#define MDI_GET_T_STATUS  0x504
#define MDI_PROT_UNPROT_T 0x505
#define MDI_VSE           0x506

#else

//
// Pass to AIL_midiOutOpen for NULL MIDI driver
//

#define MIDI_NULL_DRIVER ((U32)(S32)-2)

#endif


//
// Non-specific XMIDI/MIDI controllers and event types
//

#define SYSEX_BYTE        105
#define PB_RANGE          106
#define CHAN_MUTE         107
#define CALLBACK_PFX      108
#define SEQ_BRANCH        109
#define CHAN_LOCK         110
#define CHAN_PROTECT      111
#define VOICE_PROTECT     112
#define TIMBRE_PROTECT    113
#define PATCH_BANK_SEL    114
#define INDIRECT_C_PFX    115
#define FOR_LOOP          116
#define NEXT_LOOP         117
#define CLEAR_BEAT_BAR    118
#define CALLBACK_TRIG     119
#define SEQ_INDEX         120

#define GM_BANK_MSB       0
#define MODULATION        1
#define DATA_MSB          6
#define PART_VOLUME       7
#define PANPOT            10
#define EXPRESSION        11
#define GM_BANK_LSB       32
#define DATA_LSB          38
#define SUSTAIN           64
#define REVERB            91
#define CHORUS            93
#define RPN_LSB           100
#define RPN_MSB           101
#define RESET_ALL_CTRLS   121
#define ALL_NOTES_OFF     123

#define EV_NOTE_OFF       0x80
#define EV_NOTE_ON        0x90
#define EV_POLY_PRESS     0xa0
#define EV_CONTROL        0xb0
#define EV_PROGRAM        0xc0
#define EV_CHAN_PRESS     0xd0
#define EV_PITCH          0xe0
#define EV_SYSEX          0xf0
#define EV_ESC            0xf7
#define EV_META           0xff

#define META_EOT          0x2f
#define META_TEMPO        0x51
#define META_TIME_SIG     0x58

//
// SAMPLE.system_data[] usage
//

#define VOC_BLK_PTR       1  // Pointer to current block
#define VOC_REP_BLK       2  // Pointer to beginning of repeat loop block
#define VOC_N_REPS        3  // # of iterations left in repeat loop
#define VOC_MARKER        4  // Marker to search for, or -1 if all
#define VOC_MARKER_FOUND  5  // Desired marker found if 1, else 0
#define STR_HSTREAM       6  // Stream, if any, that owns the HSAMPLE
#define SSD_TEMP          7  // Temporary storage location for general use

//
// Timer status values
//

#define AILT_FREE         0         // Timer handle is free for allocation
#define AILT_STOPPED      1         // Timer is stopped
#define AILT_RUNNING      2         // Timer is running

//
// SAMPLE.status flag values
//

#define SMP_FREE          0x0001    // Sample is available for allocation

#define SMP_DONE          0x0002    // Sample has finished playing, or has
                                    // never been started

#define SMP_PLAYING       0x0004    // Sample is playing

#define SMP_STOPPED       0x0008    // Sample has been stopped

#define SMP_PLAYINGBUTRELEASED 0x0010 // Sample is playing, but digital handle
                                      // has been temporarily released



//
// SEQUENCE.status flag values
//

#define SEQ_FREE          0x0001    // Sequence is available for allocation

#define SEQ_DONE          0x0002    // Sequence has finished playing, or has
                                    // never been started

#define SEQ_PLAYING       0x0004    // Sequence is playing

#define SEQ_STOPPED       0x0008    // Sequence has been stopped

#define SEQ_PLAYINGBUTRELEASED 0x0010 // Sequence is playing, but MIDI handle
                                      // has been temporarily released

#ifdef IS_DOS

//
// MIDI driver types
//

#define MDIDRVRTYPE_GM    0         // General MIDI driver (Roland-compatible)
#define MDIDRVRTYPE_FM_2  1         // 2-operator FM MIDI driver (OPL2)
#define MDIDRVRTYPE_FM_4  2         // 4-operator FM MIDI driver (OPL3)
#define MDIDRVRTYPE_SPKR  3         // Tandy or PC speaker "beep" driver

//
// .INI installation result codes
//

#define AIL_INIT_SUCCESS  0         // Driver installed successfully
#define AIL_NO_INI_FILE   1         // No MDI.INI or DIG.INI file exists
#define AIL_INIT_FAILURE  2         // Driver could not be initialized

#ifdef __BORLANDC__

 #ifndef REALPTR
 #define REALPTR(x) ((void *) (U32) ((((U32) (x))>>16<<4) + ((x) & 0xffff) \
                    - AIL_sel_base(_DS)))
 #endif

#else

 #ifndef REALPTR
 #define REALPTR(x) ((void *) (U32) ((((U32) (x))>>16<<4) + ((x) & 0xffff)))
 #endif

#endif

#else

#ifdef IS_WINDOWS

//
// AIL_set_direct_buffer_control() command values
//

#define AILDS_RELINQUISH  0         // App returns control of secondary buffer
#define AILDS_SEIZE       1         // App takes control of secondary buffer
#define AILDS_SEIZE_LOOP  2         // App wishes to loop the secondary buffer

#endif

#endif

//
// General type definitions for portability
//

#ifndef C8
#define C8 char
#endif

#ifndef U8
#define U8 unsigned char
#endif

#ifndef S8
#define S8 signed char
#endif

#ifndef U16
#define U16 unsigned short
#endif

#ifndef S16
#define S16 signed short
#endif

typedef void VOIDFUNC(void);

#ifdef IS_PS2

#ifndef U128
#define U128 u_long128
#endif

#ifndef U64
#define U64 unsigned long
#endif

#ifndef S64
#define S64 signed long
#endif

#ifndef U32
#define U32 unsigned int
#endif

#ifndef S32
#define S32 signed int
#endif

#ifndef UINTa
#define UINTa unsigned int
#endif

#ifndef SINTa
#define SINTa signed int
#endif

#else

#ifdef IS_PS3

#ifndef U64
#define U64 unsigned long long
#endif

#ifndef S64
#define S64 signed long long
#endif

#ifndef U32
#define U32 unsigned int
#endif

#ifndef S32
#define S32 signed int
#endif

#ifndef UINTa
#define UINTa unsigned int
#endif

#ifndef SINTa
#define SINTa signed int
#endif

#else

#ifdef IS_16

#ifndef U32
#define U32 unsigned long
#endif

#ifndef S32
#define S32 signed long
#endif

#ifndef UINTa
#define UINTa unsigned long
#endif

#ifndef SINTa
#define SINTa signed long
#endif
#else

#if defined(IS_MAC) || defined(IS_WII)


#ifndef U64
#define U64 unsigned long long
#endif

#ifndef S64
#define S64 signed long long
#endif

#else

#ifndef U64
#define U64 unsigned __int64
#endif

#ifndef S64
#define S64 signed __int64
#endif

#endif

#ifndef U32
#define U32 unsigned int
#endif

#ifndef S32
#define S32 signed int
#endif

#ifdef _WIN64

#ifndef UINTa
#define UINTa unsigned __int64
#endif

#ifndef SINTa
#define SINTa signed __int64
#endif

#else

#ifdef _Wp64

#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300

#ifndef UINTa
typedef __w64 unsigned long UINTa;
#define UINTa UINTa
#endif

#ifndef SINTa
typedef __w64 signed long SINTa;
#define SINTa SINTa
#endif

#else

#ifndef UINTa
#define UINTa unsigned long
#endif

#ifndef SINTa
#define SINTa signed long
#endif
#endif

#else

#ifndef UINTa
#define UINTa unsigned int
#endif

#ifndef SINTa
#define SINTa signed int
#endif

#endif

#endif

#endif

#endif

#endif

#ifndef F32
#define F32 float
#endif

#ifndef F64
#define F64 double
#endif


#ifndef REALFAR
#define REALFAR unsigned int
#endif

#ifndef FILE_ERRS
#define FILE_ERRS

#define AIL_NO_ERROR        0
#define AIL_IO_ERROR        1
#define AIL_OUT_OF_MEMORY   2
#define AIL_FILE_NOT_FOUND  3
#define AIL_CANT_WRITE_FILE 4
#define AIL_CANT_READ_FILE  5
#define AIL_DISK_FULL       6

#endif

#define MIN_VAL 0
#define NOM_VAL 1
#define MAX_VAL 2

//
// Preference names and default values
//

#define DIG_RESAMPLING_TOLERANCE   0
#define DEFAULT_DRT                131    // Resampling triggered at +/- 0.2%

#define DIG_MIXER_CHANNELS         1
#define DEFAULT_DMC                64     // 64 allocatable SAMPLE structures

#define DIG_MAX_PREDELAY_MS        2
#define DEFAULT_MPDMS              500    // Max predelay reverb time in ms

#define MDI_SERVICE_RATE           3
#define DEFAULT_MSR                120    // XMIDI sequencer timing = 120 Hz

#define MDI_SEQUENCES              4
#define DEFAULT_MS                 8      // 8 sequence handles/driver

#define MDI_DEFAULT_VOLUME         5
#define DEFAULT_MDV                127    // Default sequence volume = 127 (0-127)

#define MDI_QUANT_ADVANCE          6
#define DEFAULT_MQA                1      // Beat/bar count +1 interval

#define MDI_ALLOW_LOOP_BRANCHING   7
#define DEFAULT_ALB                NO     // Branches cancel XMIDI FOR loops

#define MDI_DEFAULT_BEND_RANGE     8
#define DEFAULT_MDBR               2      // Default pitch-bend range = 2

#define DIG_3D_MUTE_AT_MAX        46
#define DEFAULT_D3MAM             NO      // off by default

#if defined(IS_WINDOWS) || defined(IS_DOS)

#define MDI_DOUBLE_NOTE_OFF        9
#define DEFAULT_MDNO               NO     // For stuck notes on SB daughterboards

#endif

#define DIG_ENABLE_RESAMPLE_FILTER 31     // Enable resampling filter by
#define DEFAULT_DERF               YES    // default

#if defined(IS_WINDOWS) || defined(IS_MAC) || defined(IS_LINUX)

#define MDI_SYSEX_BUFFER_SIZE      10
#define DEFAULT_MSBS               1536   // Default sysex buffer = 1536 bytes

#define DIG_OUTPUT_BUFFER_SIZE     11
#define DEFAULT_DOBS               49152  // 48K output buffer size

#define AIL_MM_PERIOD              12

#if defined(IS_WINDOWS)
#define DEFAULT_AMP                5      // Default MM timer period = 5 msec.
#else
#define DEFAULT_AMP                15     // Default MM timer period = 15 msec (mac and linux)
#endif

#endif

#ifdef IS_PS2

#define MDI_SYSEX_BUFFER_SIZE      10
#define DEFAULT_MSBS               1536   // Default sysex buffer = 1536 bytes

#define AIL_MM_PERIOD              12
#define DEFAULT_AMP                5      // Default MM timer period = 5 msec.

#define AIL_TIMERS                 13
#define DEFAULT_AT                 16     // 16 allocatable HTIMER handles

#define AIL_MUTEX_PROTECTION       44
#define DEFAULT_AMPR               YES    // Lock each call into Miles with a mutex

#define DIG_DS_FRAGMENT_SIZE       34
#define DEFAULT_DDFS               12     // Use 12 millisecond buffer fragments with PS2

#define DIG_DS_FRAGMENT_CNT        35
#define DEFAULT_DDFC               32     // Use 32 buffers with PS2

#define AIL_LOCK_PROTECTION        18
#define DEFAULT_ALP                NO     // Don't suspend foreground thread by default

#endif

#ifdef IS_PS3

#define MDI_SYSEX_BUFFER_SIZE      10
#define DEFAULT_MSBS               1536   // Default sysex buffer = 1536 bytes

#define DIG_DS_FRAGMENT_SIZE       34
#define DEFAULT_DDFS               5     // Use 5 millisecond buffer fragments 

#define DIG_DS_FRAGMENT_CNT        35
#define DEFAULT_DDFC               32     // Use 32 buffer fragments 

#define DIG_DS_MIX_FRAGMENT_CNT    42
#define DEFAULT_DDMFC              8      // Mix ahead 8 buffer fragments


#define AIL_MM_PERIOD              12
#define DEFAULT_AMP                5      // Default MM timer period = 5 msec.

#define AIL_TIMERS                 13
#define DEFAULT_AT                 16     // 16 allocatable HTIMER handles

#define AIL_MUTEX_PROTECTION       44
#define DEFAULT_AMPR               YES    // Lock each call into Miles with a mutex

#define AIL_LOCK_PROTECTION        18
#define DEFAULT_ALP                NO     // Don't suspend foreground thread by default

#endif

#ifdef IS_WII

#define MDI_SYSEX_BUFFER_SIZE      10
#define DEFAULT_MSBS               1536   // Default sysex buffer = 1536 bytes

#define DIG_DS_FRAGMENT_CNT        35
#define DEFAULT_DDFC               96     // Use 96 buffer fragments (of 1 sec buffer) 

#define DIG_DS_MIX_FRAGMENT_CNT    42
#define DEFAULT_DDMFC              3      // Mix ahead 3 buffer fragments


#define AIL_MM_PERIOD              12
#define DEFAULT_AMP                5      // Default MM timer period = 5 msec.

#define AIL_TIMERS                 13
#define DEFAULT_AT                 16     // 16 allocatable HTIMER handles

#endif

#ifdef IS_XBOX

#define MDI_SYSEX_BUFFER_SIZE      10
#define DEFAULT_MSBS               1536   // Default sysex buffer = 1536 bytes

#define AIL_MM_PERIOD              12
#define DEFAULT_AMP                5      // Default MM timer period = 5 msec.

#define AIL_TIMERS                 13
#define DEFAULT_AT                 16     // 16 allocatable HTIMER handles

#define AIL_MUTEX_PROTECTION       44
#define DEFAULT_AMPR               YES    // Lock each call into Miles with a mutex

#define DIG_DS_FRAGMENT_SIZE       34
#define DEFAULT_DDFS               12     // Use 12 millisecond buffer fragments with DirectSound

#define DIG_DS_FRAGMENT_CNT        35
#define DEFAULT_DDFC               32     // Use 32 buffers with DirectSound

#define AIL_LOCK_PROTECTION        18
#define DEFAULT_ALP                NO     // Don't suspend foreground thread by default

#endif

#ifdef IS_XENON

#define MDI_SYSEX_BUFFER_SIZE      10
#define DEFAULT_MSBS               1536   // Default sysex buffer = 1536 bytes

#define DIG_DS_FRAGMENT_SIZE       34
#define DEFAULT_DDFS               16     // Use 16 millisecond buffer fragments 

#define DIG_DS_FRAGMENT_CNT        35
#define DEFAULT_DDFC               48     // Use 48 buffer fragments 

#define DIG_DS_MIX_FRAGMENT_CNT    42
#define DEFAULT_DDMFC              4      // Mix ahead 4 buffer fragments


#define AIL_MM_PERIOD              12
#define DEFAULT_AMP                7      // Default MM timer period = 7 msec.

#define AIL_TIMERS                 13
#define DEFAULT_AT                 16     // 16 allocatable HTIMER handles

#define AIL_MUTEX_PROTECTION       44
#define DEFAULT_AMPR               YES    // Lock each call into Miles with a mutex

#define AIL_LOCK_PROTECTION        18
#define DEFAULT_ALP                NO     // Don't suspend foreground thread by default

#define AIL_CPU_AFFINITY           50
#define DEFAULT_CPUAF              2      // Use Xenon CPU 2 by default

#endif

#ifdef IS_LINUX

#define AIL_TIMERS                 13
#define DEFAULT_AT                 16     // 16 allocatable HTIMER handles

#define AIL_MUTEX_PROTECTION       44
#define DEFAULT_AMPR               YES    // Lock each call into Miles with a mutex

#endif

#ifdef IS_MAC

#define DIG_SM_FRAGMENT_SIZE       34
#define DEFAULT_DSFS                8     // Use 8 millisecond buffer fragments with Sound Manager

#define DIG_SM_MIX_FRAGMENT_CNT    42
#define DEFAULT_DSMFC               8     // Buffer 8 fragments ahead (should be an even number)

#define DIG_SM_FRAGMENT_CNT        35
#define DEFAULT_DSFC               64     // Allow up to 64 buffer fragments with Sound Manager (should be an even number)

#endif

#if defined(IS_WINDOWS)

#define DIG_DS_FRAGMENT_SIZE       34
#define DEFAULT_DDFS               8      // Use 8 millisecond buffer fragments with DirectSound if MSS mixer in use

#define DIG_DS_FRAGMENT_CNT        35
#define DEFAULT_DDFC               96     // Use 96 buffer fragments with DirectSound if MSS mixer in use

#define DIG_DS_MIX_FRAGMENT_CNT    42
#define DEFAULT_DDMFC              8      // Mix ahead 8 buffer fragments

#define DIG_DS_USE_PRIMARY         36
#define DEFAULT_DDUP               NO     // Mix into secondary DirectSound buffer by default

#define DIG_DS_DSBCAPS_CTRL3D      37
#define DEFAULT_DDDC               NO     // Do not use DSBCAPS_CTRL3D by default

#define DIG_DS_CREATION_HANDLER    38
#define DEFAULT_DDCH               NULL   // Use DirectSoundCreate() by default

#define AIL_TIMERS                 13
#define DEFAULT_AT                 16     // 16 allocatable HTIMER handles

#define DIG_MAX_CHAIN_ELEMENT_SIZE 14
#define DEFAULT_MCES               8192   // max of 8192 bytes/waveOut buffer

#define DIG_MIN_CHAIN_ELEMENT_TIME 45
#define DEFAULT_MCET               100    // 100 milliseconds buffers

#define DIG_USE_WAVEOUT            15
#define DEFAULT_DUW                NO     // Use DirectSound by default

#define DIG_DS_SECONDARY_SIZE      16
#define DEFAULT_DDSS               (32*1024L)  // Must be 2^n -- use 32K by default

#define DIG_DS_SAMPLE_CEILING      17
#define DEFAULT_DDSC               44100  // Allow up to 44 kHz samples

#define AIL_LOCK_PROTECTION        18
#define DEFAULT_ALP                NO     // Don't suspend foreground thread by default

#define AIL_MUTEX_PROTECTION       44
#define DEFAULT_AMPR               YES    // Lock each call into Miles with a mutex

#define DIG_PREFERRED_WO_DEVICE    40    
#define DEFAULT_DPWOD              WAVE_MAPPER  // Preferred WaveOut device == WAVE_MAPPER

#define DIG_PREFERRED_DS_DEVICE    47
#define DEFAULT_DPDSD              0            // Preferred DirectSound device == default NULL GUID

#else

#ifdef IS_DOS

#define DIG_SERVICE_RATE          10
#define DEFAULT_DSR               200     // DMA buffer-polling rate = 200 Hz

#define DIG_HARDWARE_SAMPLE_RATE  11
#define DEFAULT_DHSR              NOM_VAL // Use nominal sample rate by default

#define DIG_DMA_RESERVE           12
#define DEFAULT_DDR               32768   // Reserve 32K real-mode mem for DMA

#define DIG_LATENCY               13
#define DEFAULT_DL                100     // Half-buffer size in ms = 100

#define DIG_USE_STEREO            14
#define DEFAULT_DUS               NO      // Use mono output only

#define DIG_USE_16_BITS           15
#define DEFAULT_DU16              NO      // Use 8-bit output by default

#define DIG_ALLOW_16_BIT_DMA      16
#define DEFAULT_DA16DMA           YES     // OK to use 16-bit DMA if necessary

#define DIG_SS_LOCK               17
#define DEFAULT_DSL               NO      // Don't disable IRQs while mixing

#define AIL_SCAN_FOR_HARDWARE     18
#define DEFAULT_ASH               YES     // Scan for I/O settings if necessary

#define AIL_ALLOW_VDM_EXECUTION   19
#define DEFAULT_AVE               YES     // Allow Windows "DOS box" execution

#endif

#endif

// ----------------------------------
// DLS Preference names and default values
// Unless otherwise specified, values must be established
// BEFORE calling DLSMSSOpen()!
// ----------------------------------

#define DLS_TIMEBASE               20
#define DEFAULT_DTB                120  // 120 intervals/second by default

#define DLS_VOICE_LIMIT            21
#define DEFAULT_DVL                64   // 64 voices supported

#define DLS_BANK_SELECT_ALIAS      22
#define DEFAULT_DBSA               NO   // Do not treat controller 114 as bank

#define DLS_STREAM_BOOTSTRAP       23   // Don't submit first stream buffer
#define DEFAULT_DSB                YES  // until at least 2 available

#define DLS_VOLUME_BOOST           24
#define DEFAULT_DVB                0    // Boost final volume by 0 dB

#define DLS_ENABLE_FILTERING       25   // Filtering = on by default
#define DEFAULT_DEF                YES  // (may be changed at any time)

#ifdef IS_X86

#define AIL_ENABLE_MMX_SUPPORT     27   // Enable MMX support if present
#define DEFAULT_AEMS               YES  // (may be changed at any time)

#endif

#define DLS_GM_PASSTHROUGH         28   // Pass unrecognized traffic on to
#define DEFAULT_DGP                YES  // default GM driver layer
                                        // (may be changed at any time)

#define DLS_ADPCM_TO_ASI_THRESHOLD 39   // Size in samples to switch to ASI
#define DEFAULT_DATAT              32768

//
// Add'l platform-independent prefs
//

#define DIG_INPUT_LATENCY          41   // Use >= 250-millisecond input buffers if
#define DEFAULT_DIL                250  // explicit size request cannot be satisfied

#ifdef IS_WINDOWS

#define DIG_USE_WAVEIN             43
#define DEFAULT_DUWI               YES  // Use waveIn for input by default

#endif

#define DIG_LEVEL_RAMP_SAMPLES     48
#define DEFAULT_DLRS               32  // Ramp level changes over first 32 samples in each buffer to reduce zipper noise

#define DIG_ENCODE_QUALITY_PERCENT 49
#define DEFAULT_DEQP               50  // Default encoding quality (e.g., for DTS) = 50%

#define N_PREFS 51                      // # of preference types

typedef struct Mwavehdr_tag {
    C8 *       lpData;                 
    U32       dwBufferLength;         
    U32       dwBytesRecorded;        
    UINTa   dwUser;                 
    U32       dwFlags;                
    U32       dwLoops;                
    struct Mwavehdr_tag  *lpNext;     
    UINTa   reserved;               
} MWAVEHDR;
typedef MSS_STRUCT Mwaveformat_tag {
    U16    wFormatTag;        
    U16    nChannels;         
    U32    nSamplesPerSec;    
    U32    nAvgBytesPerSec;   
    U16    nBlockAlign;       
} MWAVEFORMAT;
typedef MSS_STRUCT Mpcmwaveformat_tag {
    MWAVEFORMAT  wf;
    U16        wBitsPerSample;
} MPCMWAVEFORMAT;
typedef MSS_STRUCT Mwaveformatex_tag {
    U16    wFormatTag;        
    U16    nChannels;         
    U32    nSamplesPerSec;    
    U32    nAvgBytesPerSec;   
    U16    nBlockAlign;       
    U16    wBitsPerSample;
    U16    cbSize;
} MWAVEFORMATEX;
typedef MSS_STRUCT Mwaveformatextensible_tag {
  MWAVEFORMATEX Format;
  union {
      U16 wValidBitsPerSample;
      U16 wSamplesPerBlock;
      U16 wReserved;
  } Samples;
  U32 dwChannelMask;
  U8 SubFormat[16];
} MWAVEFORMATEXTENSIBLE;

typedef struct _AILSOUNDINFO {
  S32 format;
  void const FAR* data_ptr;
  U32 data_len;
  U32 rate;
  S32 bits;
  S32 channels;
  U32 channel_mask;
  U32 samples;
  U32 block_size;
  void const FAR* initial_ptr;
} AILSOUNDINFO;

// ----------------------------
// MSS8-specific declarations
// ----------------------------

typedef enum         // Property storage types
{
   MP_S32,           // 32-bit int
   MP_S64,           // 64-bit int
   MP_SINTa,         // size_t
   MP_F32,           // 32-bit float
   MP_BOOL,          // boolean
   MP_VEC3,          // 3-vector of F32s
   MP_VEC3PAIR,      // Two VEC3s (e.g., for face/up vectors)
   MP_SZ,            // zero-terminated ASCII string
   MP_BITMASK,       // 32-bit bitmask
   MP_ENUM,          // Enumerated type
   MP_CHANMAPVAL     // MSS_CHANNEL_MAP_VALUE
}
MSS_PROPERTY_TYPE;

typedef enum            // Property semantic types
{
   MP_SPECIAL,          // Other semantic type not listed here
   MP_UNSPECIFIED,      // No specific semantics (e.g., a simple numeric quantity or unitless value)
   MP_NAME,             // User-specified name for an asset
   MP_FILENAME,         // Filename
   MP_USER_SINTa,       // User data of size size_t
   MP_OUTPUT_FORMAT,    // MSS_MC_SPEC enum
   MP_ROOM_TYPE,        // MSS_ROOM_TYPE enum
   MP_SOUND_STATUS,     // Status mask
   MP_FREQUENCY_HZ,     // Directly-specified frequency or rate
   MP_NYQUIST_FRACTION, // Frequency specified as a fraction of Nyquist
   MP_BYTE_OFFSET,      // Offset in bytes from start of audio data
   MP_BYTES,            // Other value specified in bytes
   MP_POWER_PAN,        // Constant-power pan value (0=left/front, 1=right/rear)
   MP_DB,               // dB
   MP_DBFS,             // dB relative to full scale (e.g., -48 = 48 dB down from full scale)
   MP_LOUDNESS,         // Loudness factor
   MP_DISTANCE,         // Scalar distance (in user's units)
   MP_SCALAR,           // General-purpose linear scalar quantity
   MP_PERCENT,          // Value should be displayed as a percentage (if float, 1.0=100%, 0.0=0%)
   MP_MIX,              // Value is a floating-point wet/dry mix (0.0=all dry, 1.0=all wet)
   MP_Q,                // Value is a floating-point Q factor (ratio of center/cutoff frequency to 3-dB bandwidth)
   MP_RADIANS,          // Radians
   MP_DEGREES,          // Degrees
   MP_VEL_AXIS,         // Velocity along a vector or orthogonal axis, in meters/ms
   MP_AXIS,             // Vector component
   MP_POS_VEC,          // 3D position vector
   MP_ORIENT_PAIR,      // 3D up/face vector pair
   MP_VEL_VEC,          // 3D velocity vector, in meters/ms
   MP_MS,               // Milliseconds
   MP_SEC               // Seconds
}
MSS_PROPERTY_SUBTYPE;

#define MPF_READ_ONLY        0x00000001    // Property can be updated only by the system, not by the API
#define MPF_UNENUMERABLE     0x00000002    // Property should not be returned by enumeration functions
#define MPF_INVISIBLE        0x00000004    // Property should not be displayed in Miles Sound Studio or other production tools
#define MPF_EDITABLE         0x00000008    // Property is suitable (but not necessarily recommended) for editing under user control
#define MPF_OPT_EDITABLE     0x00000010    // Property can be edited, but is usually reserved for runtime control
#define MPF_SECONDARY        0x00000020    // Property is an alternate lower-resolution representation of another property, and does not need to be saved/restored separately
#define MPF_UNSAVED          0x00000040    // Property is otherwise not needed to recreate the sound or environment state
#define MPF_NO_DISABLE_CHK   0x00000080    // Property is always needed to construct new instances (no enable/disable checkbox), or it's programmatically enabled
#define MPF_ENABLED_3D       0x00000200    // Property is always disabled in 2D sounds
#define MPF_ACTIVE           0x00000400    // Value cannot be queried without an active driver or sound handle
#define MPF_CTRL_INVERT      0x00000800    // Control scale should be inverted (e.g., front-back pan)
                             
#define MPF_CTRL_HSLIDER     0x00001000    // Suggested control type = horizontal slider
#define MPF_CTRL_VSLIDER     0x00002000    // Suggested control type = vertical slider
#define MPF_CTRL_VEC_SLIDER  0x00003000    // Suggested control type = vector slider
#define MPF_CTRL_CHAN_SLIDER 0x00004000    // Suggested control type = channel level slider
#define MPF_CTRL_ENUM        0x00005000    // Suggested control type = enumerated (e.g., room types) 
#define MPF_CTRL_CHKBOX      0x00006000    // Suggested control type = check box
#define MPF_CTRL_TEXT        0x00007000    // Suggested control type = literal text entry (e.g., unitless numeric quantities)
#define MPF_CTRL_ROOMTYPE    0x00008000    // Suggested control type = dropdown list box with standard room types
#define MPF_CTRL_ORIENT      0x00009000    // Suggested control type = literal text entry for face/up vectors
#define MPF_CTRL_OTHER       0x0000F000    // Suggested control type is undefined (application-specific)
                             
#define MPF_CTRL_MASK        0x0000F000    // Mask corresponding to MPF_CTRL place value
                             
#define MPF_TAB_SHIFT        16            // Amount to shift right to obtain tab #
#define MPF_TAB_MASK         0x000F0000    // Mask corresponding to MPF_TAB place value
                             
#define MPF_PROP_ANY         0x00000000    // Matches any type of property when enumerating
#define MPF_PROP_HMDRIVER    0x00100000    // Property can be applied to an HMDRIVER
#define MPF_PROP_HMFILE      0x00200000    // Property can be applied to an HMFILE
#define MPF_PROP_HMENV       0x00400000    // Property can be applied to an HMENV
#define MPF_PROP_HMPRESET    0x00800000    // Property can be applied to an HMPRESET
#define MPF_PROP_HMSND       0x01000000    // Property can be applied to an HMSND instance
                             
#define MPF_PROP_MASK        0x01F00000    // Mask corresponding to MPF_PROP place value
                             
#define MPF_OPTLIMITS        0x02000000    // Min/max limits are not enforced (i.e., for control-labelling purposes only)
#define MPF_TIMLIMIT         0x04000000    // Min/max limits are dynamically obtained from sample duration in milliseconds
#define MPF_LENLIMIT         0x08000000    // Min/max limits are dynamically obtained from sample length in bytes
#define MPF_MAXLIMIT         0x10000000    // Min/max limits are dynamically obtained from sample pos/neg max distance
#define MPF_MAX_NYQUIST      0x20000000    // Max limit is the sample's Nyquist limit in Hz
#define MPF_NORMALIZE        0x40000000    // Vector property will always be stored in normalized form
                             
#define MSS_PROPERTY      const C8 *

//
// Most of these enums correspond to AIL_ calls made in update_HDIGDRIVER() and
// update_HSAMPLE().  They allow preset authors to specify that existing
// AIL properties should be left alone by the high-level API
//

enum ENVCTRL
{
   BIT_ENV_REVERB_ROOM_TYPE,   
   BIT_ENV_REVERB_PARAMS,     // disabled by default (but queryable)
   BIT_ENV_REVERB_MASTER_VOLUME,
   BIT_ENV_REVERB_SPEAKER_LEVELS,
   BIT_ENV_MASTER_VOLUME,          
   BIT_ENV_ROLLOFF,                
   BIT_ENV_DOPPLER,                
   BIT_ENV_FALLOFF_POWER,          
   BIT_ENV_LISTENER_POS,           
   BIT_ENV_LISTENER_ORIENT,
   BIT_ENV_LISTENER_VEL           
};

enum SNDCTRL
{
   BIT_SND_IS_3D,                
   BIT_SND_3D_POS,                
   BIT_SND_3D_CONE,
   BIT_SND_3D_DISTANCES,
   BIT_SND_3D_ORIENT,
   BIT_SND_3D_VEL,
   BIT_SND_REVERB_LEVELS, 
   BIT_SND_LPF_CUTOFF,
   BIT_SND_OCCLUSION,       // disabled by default
   BIT_SND_EXCLUSION,
   BIT_SND_OBSTRUCTION,
   BIT_SND_PLAYBACK_RATE,
   BIT_SND_VOLUME_PAN,
   BIT_SND_INITIAL_LOOP_COUNT,
   BIT_SND_LOOP_BLOCK,
   BIT_SND_LOOP_COUNT,      // applicable only to HMSNDs
   BIT_SND_INITIAL_OFFSET_BYTES,
   BIT_SND_OFFSET_BYTES,
   BIT_SND_OFFSET_MS,       // disabled by default
   BIT_SND_PRIORITY,
   BIT_SND_EOS_CALLBACK,
   BIT_SND_FALLOFF_CALLBACK,
   BIT_SND_AUTO_STREAM,
   BIT_SND_USER,
   BIT_SND_FILTER_PROPERTY, // (filter properties aren't individually classified since they don't usually have redundant representations)
   BIT_SND_CHAN_LEVELS
};

typedef MSS_STRUCT
{
   const char          *name;
   const char          *label;            // (if NULL, use name as label text)
   MSS_PROPERTY_TYPE    type;
   MSS_PROPERTY_SUBTYPE subtype;
   S32                  size_bytes;
   U32                  MPF_flags;
   S32                  enable_bitnum;
   const char          *min;
   const char          *max;
   const char          *def;              // (if NULL, property has no meaningful default value)
   const char          *help;
}
MSS_PROPERTY_INFO;

typedef MSS_STRUCT
{
   MSS_SPEAKER src_chan;
   MSS_SPEAKER dest_chan;
   F32         value;
}
MSS_CHANNEL_MAP_VALUE;

// ----------------------------
// End MSS8 declarations
// ----------------------------

#ifndef RIB_H        // RIB.H contents included if RIB.H not already included

#define RIB_H
#define ARY_CNT(x) (sizeof((x)) / sizeof((x)[0]))

// ----------------------------------
// RIB data types
// ----------------------------------

typedef S32 RIBRESULT;

#define RIB_NOERR                    0   // Success -- no error
#define RIB_NOT_ALL_AVAILABLE        1   // Some requested functions/attribs not available
#define RIB_NOT_FOUND                2   // Resource not found
#define RIB_OUT_OF_MEM               3   // Out of system RAM

//
// Handle to interface provider
//

typedef U32 HPROVIDER;

//
// Handle representing token used to obtain property data
//
// This needs to be large enough to store a function pointer
//

typedef UINTa HPROPERTY;

//
// Handle representing an enumerated interface entry
//
// RIB_enumerate_interface() returns 1 if valid next entry found, else
// 0 if end of list reached
//

typedef U32 HINTENUM;
#define HINTENUM_FIRST 0

//
// Handle representing an enumerated provider entry
//
// RIB_enumerate_providers() returns 1 if valid next entry found, else
// 0 if end of list reached
//

typedef U32 HPROENUM;
#define HPROENUM_FIRST 0

//
// Data types for RIB properties
//

typedef enum
{
   RIB_NONE = 0, // No type
   RIB_CUSTOM,   // Used for pointers to application-specific structures
   RIB_DEC,      // Used for 32-bit integer values to be reported in decimal
   RIB_HEX,      // Used for 32-bit integer values to be reported in hex
   RIB_FLOAT,    // Used for 32-bit single-precision FP values
   RIB_PERCENT,  // Used for 32-bit single-precision FP values to be reported as percentages
   RIB_BOOL,     // Used for Boolean-constrained integer values to be reported as TRUE or FALSE
   RIB_STRING,   // Used for pointers to null-terminated ASCII strings
   RIB_READONLY = 0x80000000  // Property is read-only
}
RIB_DATA_SUBTYPE;

//
// RIB_ENTRY_TYPE structure, used to register an interface or request one
//

typedef enum
{
   RIB_FUNCTION = 0,
   RIB_PROPERTY       // Property: read-only or read-write data type
}
RIB_ENTRY_TYPE;

//
// RIB_INTERFACE_ENTRY, used to represent a function or data entry in an
// interface
//

typedef struct
{
   RIB_ENTRY_TYPE   type;        // See list above
   const C8 FAR          *entry_name;  // Name of desired function or property
   UINTa            token;       // Function pointer or property token
   RIB_DATA_SUBTYPE subtype;     // Property subtype
}
RIB_INTERFACE_ENTRY;

//
// Standard RAD Interface Broker provider identification properties
//

#define PROVIDER_NAME    ((U32) (S32) (-100))    // RIB_STRING name of decoder
#define PROVIDER_VERSION ((U32) (S32) (-101))    // RIB_HEX BCD version number

//
// Standard function to obtain provider properties (see PROVIDER_ defines
// above)
//
// Each provider of a searchable interface must export this function
//

typedef S32 (AILCALL FAR *PROVIDER_PROPERTY) (HPROPERTY        index,
                                              void FAR *       before_value,
                                              void const FAR * new_value,
                                              void FAR *       after_value
                                              );

//
// Macros to simplify interface registrations/requests for functions,
// and properties
//

#define FN(entry_name)        { RIB_FUNCTION, #entry_name, (UINTa) &(entry_name), RIB_NONE }
#define REG_FN(entry_name)    { RIB_FUNCTION, #entry_name, (UINTa) &(entry_name), RIB_NONE }

#define PR(entry_name,ID)             { RIB_PROPERTY, (entry_name), (UINTa) &(ID), RIB_NONE }
#define REG_PR(entry_name,ID,subtype) { RIB_PROPERTY, (entry_name), (UINTa)  (ID), subtype  }

#define RIB_register(x,y,z)   RIB_register_interface  ((HPROVIDER)(x), y, ARY_CNT(z), z)
#define RIB_unregister(x,y,z) RIB_unregister_interface((HPROVIDER)(ssx), y, ARY_CNT(z), z)
#define RIB_unregister_all(x) RIB_unregister_interface((HPROVIDER)(x), NULL, 0, NULL)
#define RIB_free_libraries()  RIB_free_provider_library((HPROVIDER)(NULL));
#define RIB_request(x,y,z)    RIB_request_interface   (x, y, ARY_CNT(z), z)

// ----------------------------------
// Standard RIB API prototypes
// ----------------------------------

DXDEC  HPROVIDER  AILCALL RIB_alloc_provider_handle   (long           module);
DXDEC  void       AILCALL RIB_free_provider_handle    (HPROVIDER      provider);

DXDEC  HPROVIDER  AILCALL RIB_load_provider_library   (C8 const FAR *filename);
DXDEC  void       AILCALL RIB_free_provider_library   (HPROVIDER      provider);

DXDEC  RIBRESULT  AILCALL RIB_register_interface      (HPROVIDER                      provider,
                                                       C8 const FAR                   *interface_name,
                                                       S32                            entry_count,
                                                       RIB_INTERFACE_ENTRY const FAR *rlist);

DXDEC  RIBRESULT  AILCALL RIB_unregister_interface    (HPROVIDER                      provider,
                                                       C8 const FAR                  *interface_name,
                                                       S32                            entry_count,
                                                       RIB_INTERFACE_ENTRY const FAR *rlist);

DXDEC  RIBRESULT  AILCALL RIB_request_interface       (HPROVIDER                provider,
                                                       C8 const FAR            *interface_name,
                                                       S32                      entry_count,
                                                       RIB_INTERFACE_ENTRY FAR *rlist);

DXDEC  RIBRESULT  AILCALL RIB_request_interface_entry (HPROVIDER                provider,
                                                       C8 const FAR            *interface_name,
                                                       RIB_ENTRY_TYPE           entry_type,
                                                       C8 const FAR            *entry_name,
                                                       UINTa FAR               *token);

DXDEC  S32        AILCALL RIB_enumerate_interface     (HPROVIDER                provider,
                                                       C8 FAR                  *interface_name,
                                                       RIB_ENTRY_TYPE           type,
                                                       HINTENUM FAR            *next,
                                                       RIB_INTERFACE_ENTRY FAR *dest);

DXDEC  S32        AILCALL RIB_enumerate_providers     (C8 FAR                  *interface_name,
                                                       HPROENUM FAR            *next,
                                                       HPROVIDER FAR           *dest);

DXDEC  C8 FAR *   AILCALL RIB_type_string             (void const FAR *         data,
                                                       RIB_DATA_SUBTYPE         subtype);

DXDEC  HPROVIDER  AILCALL RIB_find_file_provider      (C8 const FAR            *interface_name,
                                                       C8 const FAR            *property_name,
                                                       C8 const FAR            *file_suffix);

DXDEC  HPROVIDER  AILCALL RIB_find_provider           (C8 const FAR            *interface_name,
                                                       C8 const FAR            *property_name,
                                                       void const FAR          *property_value);

//
// Static library definitions
//

#ifdef IS_STATIC
  #define RIB_MAIN_NAME( name ) name##_RIB_Main

  DXDEC S32 AILCALL RIB_MAIN_NAME(SRS)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(DTS)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(DolbySurround)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(MP3Dec)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(OggDec)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(SpxDec)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(SpxEnc)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(Voice)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(SpxVoice)( HPROVIDER provider_handle, U32 up_down );
  DXDEC S32 AILCALL RIB_MAIN_NAME(DSP)( HPROVIDER provider_handle, U32 up_down );

#ifdef IS_XENON
  DXDEC S32 AILCALL RIB_MAIN_NAME(XMADec)( HPROVIDER provider_handle, U32 up_down );
#endif

  #define Register_RIB(name) RIB_load_static_provider_library(RIB_MAIN_NAME(name),#name)

#else
  #define RIB_MAIN_NAME( name ) RIB_Main
  DXDEC S32 AILCALL RIB_Main( HPROVIDER provider_handle, U32 up_down );
#endif

typedef S32 ( AILCALL FAR * RIB_MAIN_FUNC) ( HPROVIDER provider_handle, U32 up_down );

DXDEC HPROVIDER AILCALL RIB_load_static_provider_library (RIB_MAIN_FUNC main, char FAR* description);


DXDEC  HPROVIDER  AILCALL RIB_find_files_provider     (C8 const FAR *interface_name,
                                                       C8 const FAR *property_name_1,
                                                       C8 const FAR *file_suffix_1,
                                                       C8 const FAR *property_name_2,
                                                       C8 const FAR *file_suffix_2);

DXDEC  HPROVIDER  AILCALL RIB_find_file_dec_provider  (C8 const FAR *interface_name,
                                                       C8 const FAR *property_name_1,
                                                       U32           decimal_property_value_1,
                                                       C8 const FAR *property_name_2,
                                                       C8 const FAR *file_suffix_2);

DXDEC  S32        AILCALL RIB_load_application_providers
                                                      (C8 const FAR  *filespec);

DXDEC  void       AILCALL RIB_set_provider_user_data  (HPROVIDER provider,
                                                       U32       index,
                                                       SINTa     value);

DXDEC  SINTa      AILCALL RIB_provider_user_data      (HPROVIDER provider,
                                                       U32       index);

DXDEC  void       AILCALL RIB_set_provider_system_data
                                                      (HPROVIDER provider,
                                                       U32       index,
                                                       SINTa     value);

DXDEC  SINTa      AILCALL RIB_provider_system_data    (HPROVIDER provider,
                                                       U32       index);

DXDEC  C8 FAR *  AILCALL  RIB_error                   (void);

#endif

#ifndef MSS_ASI_VERSION // MSSASI.H contents included if MSSASI.H not already included

#define AIL_ASI_VERSION  1
#define AIL_ASI_REVISION 0

//
// Handle to stream being managed by ASI codec
//

typedef SINTa HASISTREAM;

//
// ASI result codes
//

typedef S32 ASIRESULT;

#define ASI_NOERR                   0   // Success -- no error
#define ASI_NOT_ENABLED             1   // ASI not enabled
#define ASI_ALREADY_STARTED         2   // ASI already started
#define ASI_INVALID_PARAM           3   // Invalid parameters used
#define ASI_INTERNAL_ERR            4   // Internal error in ASI driver
#define ASI_OUT_OF_MEM              5   // Out of system RAM
#define ASI_ERR_NOT_IMPLEMENTED     6   // Feature not implemented
#define ASI_NOT_FOUND               7   // ASI supported device not found
#define ASI_NOT_INIT                8   // ASI not initialized
#define ASI_CLOSE_ERR               9   // ASI not closed correctly

// ----------------------------------
// Application-provided ASI callbacks
// ----------------------------------

//
// AILASIFETCHCB: Called by ASI to obtain data from stream source
//
// offset normally will be either 0 at the first call made by the codec
// or -1 to specify a continuous stream, except when ASI_stream_seek()
// is called to restart the stream codec at a new stream offset.  In this
// case, the application must execute the seek operation on the ASI codec's
// behalf.
//
// In response to this callback, the application should read the requested
// data and copy it to the specified destination buffer, returning the number
// of bytes copied (which can be less than bytes_requested if the end of
// the stream is reached).
//

typedef S32 (AILCALLBACK FAR * AILASIFETCHCB) (UINTa     user,            // User value passed to ASI_open_stream()
                                               void FAR *dest,            // Location to which stream data should be copied by app
                                               S32       bytes_requested, // # of bytes requested by ASI codec
                                               S32       offset);         // If not -1, application should seek to this point in stream

//############################################################################
//##                                                                        ##
//## Interface "ASI codec"                                                  ##
//##                                                                        ##
//############################################################################

//
// Initialize ASI stream codec
//
// No other ASI functions may be called outside an ASI_startup() /
// ASI_shutdown() pair, except for the standard RIB function
// PROVIDER_property() where appropriate. 
//

typedef ASIRESULT (AILCALL FAR *ASI_STARTUP)(void);

//
// Shut down ASI codec
//

typedef ASIRESULT (AILCALL FAR * ASI_SHUTDOWN)(void);

//
// Return codec error message, or NULL if no errors have occurred since
// last call
//
// The ASI error text state is global to all streams
//

typedef C8 FAR *  (AILCALL FAR * ASI_ERROR)(void);

//############################################################################
//##                                                                        ##
//## Interface "ASI stream"                                                 ##
//##                                                                        ##
//############################################################################

//
// Open a stream, returning handle to stream
//

typedef HASISTREAM (AILCALL FAR *ASI_STREAM_OPEN) (UINTa         user,              // User value passed to fetch callback
                                                   AILASIFETCHCB fetch_CB,          // Source data fetch handler
                                                   U32           total_size);       // Total size for %-done calculations (0=unknown)

//
// Translate data in stream, returning # of bytes actually decoded or encoded
//
// Any number of bytes may be requested.  Requesting more data than is
// available in the codec's internal buffer will cause the AILASIFETCHCB
// handler to be called to fetch more data from the stream.
//

typedef S32  (AILCALL FAR *ASI_STREAM_PROCESS) (HASISTREAM  stream,              // Handle of stream
                                                void FAR   *buffer,              // Destination for processed data
                                                S32         buffer_size);        // # of bytes to return in buffer

//
// Restart stream decoding process at new offset
//
// Relevant for decoders only
//
// Seek destination is given as offset in bytes from beginning of stream
//
// At next ASI_stream_process() call, decoder will seek to the closest possible
// point in the stream which occurs at or after the specified position
//
// This function has no effect for decoders which do not support random
// seeks on a given stream type
//
// Warning: some decoders may need to implement seeking by reparsing
// the entire stream up to the specified offset, through multiple calls
// to the data-fetch callback.  This operation may be extremely
// time-consuming on large files or slow network connections.
//
// A stream_offset value of -1 may be used to inform the decoder that the
// application has changed the input stream offset on its own, e.g. for a
// double-buffering application where the ASI decoder is not accessing the
// stream directly.  ASI decoders should respond to this by flushing all
// internal buffers and resynchronizing themselves to the data stream.
//

typedef ASIRESULT (AILCALL FAR *ASI_STREAM_SEEK)    (HASISTREAM stream,
                                                     S32        stream_offset);

//
// Retrieve or set a property value by index (returns 1 on success)
//

typedef S32 (AILCALL FAR *ASI_STREAM_PROPERTY) (HASISTREAM stream,
                                                HPROPERTY  property,
                                                void FAR * before_value,
                                                void const FAR * new_value,
                                                void FAR * after_value
                                                );

//
// Close stream, freeing handle and all internally-allocated resources
//

typedef ASIRESULT (AILCALL FAR *ASI_STREAM_CLOSE) (HASISTREAM stream);

#endif

//############################################################################
//##                                                                        ##
//## Interface "MSS mixer services"                                         ##
//##                                                                        ##
//############################################################################

//
// Operation flags used by mixer and filter modules
//

#define M_DEST_STEREO  1       // Set to enable stereo mixer output
#define M_SRC_16       2       // Set to enable mixing of 16-bit samples
#define M_FILTER       4       // Set to enable filtering when resampling
#define M_SRC_STEREO   8       // Set to enable mixing of stereo input samples
#define M_RESAMPLE     16      // Set to enable playback ratios other than 65536
#define M_VOL_SCALING  32      // Set to enable volume scalars other than 2048
#define M_COPY16_NOVOL 64

#ifdef IS_32

//
// Initialize mixer
//
// No other mixer functions may be called outside a MIXER_startup() /
// MIXER_shutdown() pair, except for the standard RIB function
// PROVIDER_property() as appropriate.
//

typedef void (AILCALL FAR *MIXER_STARTUP)(void);

//
// Shut down mixer
//

typedef void (AILCALL FAR *MIXER_SHUTDOWN)(void);

//
// Flush mixer buffer
//

typedef void (AILCALL FAR *MIXER_FLUSH)   (S32 FAR *dest,
                                           S32      len
#ifdef IS_X86
                                           ,U32             MMX_available
#endif
                                           );

//
// Perform audio mixing operation
//

typedef void (AILCALL FAR *MIXER_MERGE)   (void const FAR * FAR *src,
                                           U32        FAR *src_fract,
                                           void const FAR *src_end,
                                           S32  FAR * FAR *dest,
                                           void FAR       *dest_end,
                                           S32  FAR       *left_val,
                                           S32  FAR       *right_val,
                                           S32             playback_ratio,
                                           S32             scale_left,
                                           S32             scale_right,
                                           U32             operation
#ifdef IS_X86
                                           ,U32             MMX_available
#endif
                                           );

//
// Translate mixer buffer contents to final output format
//
// "option" parameter is big_endian_output on Mac, MMX on x86, overwrite flag on PS2 
//

typedef void (AILCALL FAR *MIXER_COPY) (void const FAR  *src,
                                        S32       src_len,
                                        void FAR *dest,
                                        U32       operation
#if defined(IS_BE) || defined(IS_X86) || defined(IS_PS2) 
                                        ,U32       option
#endif
                                        );  

#else

//
// Initialize mixer
//
// No other mixer functions may be called outside a MIXER_startup() /
// MIXER_shutdown() pair, except for the standard RIB function
// PROVIDER_property() as appropriate.
//

typedef void (AILCALL FAR *MIXER_STARTUP)(void);

//
// Shut down mixer
//

typedef void (AILCALL FAR *MIXER_SHUTDOWN)(void);

//
// Flush mixer buffer
//

typedef void (AILCALL FAR *MIXER_FLUSH)   (S32 FAR *dest,
                                           S32      len,
                                           U32      MMX_available);

//
// Perform audio mixing operation
//

typedef void (AILCALL FAR *MIXER_MERGE)   (U32             src_sel,
                                           U32             dest_sel,
                                           U32        FAR *src_fract,
                                           U32        FAR *src_offset,
                                           U32        FAR *dest_offset,
                                           U32             src_end_offset,
                                           U32             dest_end_offset,
                                           S32  FAR       *left_val,
                                           S32  FAR       *right_val,
                                           S32             playback_ratio,
                                           S32             scale_both,
                                           U32             operation);

//
// Translate mixer buffer contents to final output format
//

typedef void (AILCALL FAR *MIXER_COPY) (void const FAR *src,
                                        S32       src_len,
                                        void FAR *dest,
                                        U32       operation,
                                        U32       option);
#endif


typedef struct _MSS_BB              // Used in both MC and conventional mono/stereo configurations
{
   S32 FAR *buffer;                 // Build buffer
   S32      bytes;                  // Size in bytes
   S32      chans;                  // Always mono (1) or stereo (2)

   S32      speaker_offset;         // Destination offset in interleaved PCM block for left channel
}
MSS_BB;

typedef struct _ADPCMDATATAG
{
  U32   blocksize;
  U32   extrasamples;
  U32   blockleft;
  U32   step;
  UINTa savesrc;
  U32   sample;
  UINTa destend;
  UINTa srcend;
  U32   samplesL;
  U32   samplesR;
  U16   moresamples[16];
} ADPCMDATA;

typedef void (AILCALL FAR * MIXER_MC_COPY) ( MSS_BB FAR * build,
                                             S32 n_build_buffers,
                                             void FAR * lpWaveAddr,
                                             S32 hw_format,
#ifdef IS_X86
                                             S32 use_MMX,
#endif
                                             S32 samples_per_buffer,
                                             S32 physical_channels_per_sample );


typedef void (AILCALL FAR * MIXER_ADPCM_DECODE ) ( void FAR * dest,
                                                   void const FAR * in,
                                                   S32 out_len,
                                                   S32 in_len,
                                                   S32 input_format,
                                                   ADPCMDATA FAR *adpcm_data);

//
// Type definitions
//

struct _DIG_DRIVER;

struct _MDI_DRIVER;

typedef struct _DIG_DRIVER FAR * HDIGDRIVER;    // Handle to digital driver

typedef struct _MDI_DRIVER FAR * HMDIDRIVER;    // Handle to XMIDI driver

typedef struct _SAMPLE FAR * HSAMPLE;           // Handle to sample

typedef struct _SMPATTRIBS FAR * HSATTRIBS;     // Handle to sample performance attributes 

typedef struct _SEQUENCE FAR * HSEQUENCE;       // Handle to sequence

typedef S32 HTIMER;                             // Handle to timer


#ifdef IS_DOS

//
// Type definitions
//

typedef MSS_STRUCT                      // I/O parameters structure
{
   S16 IO;
   S16 IRQ;
   S16 DMA_8_bit;
   S16 DMA_16_bit;
   S32 IO_reserved[4];
}
IO_PARMS;

typedef MSS_STRUCT                      // Standard MSS 3.X VDI driver header
{
   S8     ID[8];                    // "AIL3xxx" ID string, followed by ^Z

   U32    driver_version;

   REALFAR  common_IO_configurations;
   U16    num_IO_configurations;

   REALFAR  environment_string;

   IO_PARMS IO;

   S16     service_rate;

   U16    busy;

   U16    driver_num;             // Driver number

   U16    this_ISR;               // Offset of INT 66H dispatcher
   REALFAR  prev_ISR;               // Pointer to previous INT 66H ISR

   S8     scratch[128];           // Shared scratch workspace

   S8     dev_name[80];           // Device name (VDI version >= 1.12 only)
}
VDI_HDR;

typedef MSS_STRUCT
{
   U16 minimum_physical_sample_rate;
   U16 nominal_physical_sample_rate;
   U16 maximum_physical_sample_rate;

   U16 minimum_DMA_half_buffer_size;
   U16 maximum_DMA_half_buffer_size;

   U32 flags;
}
DIG_MODE;

typedef MSS_STRUCT
{
   U8    format_supported[16];
   DIG_MODE format_data[16];
}
DIG_DDT;

typedef MSS_STRUCT
{
   REALFAR  DMA_buffer_A;
   REALFAR  DMA_buffer_B;
   S16     active_buffer;
}
DIG_DST;

typedef MSS_STRUCT
{
   REALFAR  library_environment;
   REALFAR  GTL_suffix;

   U16    num_voices;

   U16    max_melodic_channel;
   U16    min_melodic_channel;
   U16    percussion_channel;
}
MDI_DDT;

typedef MSS_STRUCT
{
   S8     library_directory[128];
   S8     GTL_filename[128];

   S8     MIDI_data[512];
}
MDI_DST;

typedef MSS_STRUCT                   // Initialization file structure
{
   char     device_name[128];    // Device name
   char     driver_name[128];    // Driver filename
   IO_PARMS IO;                  // I/O parameters for driver
}
AIL_INI;

typedef MSS_STRUCT                   // Handle to driver
{
   REALFAR  seg;                 // Seg:off pointer to driver (off=0)
   U32    sel;                   // Selector for driver (off=0)
   void    *buf;                 // Protected-mode pointer to driver
   U32    size;                  // Size of driver image
   VDI_HDR *VHDR;                // Pointer to driver header (same as buf)
   S32     type;                 // AIL3DIG or AIL3MDI (see below)

   S32     initialized;          // 1 if hardware successfully init'd, else 0

   S32     PM_ISR;               // -1 if no PM ISR hooked, else IRQ #

   HTIMER   server;              // DRV_SERVE periodic timer, if requested

                                 // Vector to high-level destructor, if any

   void (AILCALL *destructor)(HDIGDRIVER);

                                 // High-level descriptor (DIG_ or MDI_DRIVER)
   void        *descriptor;
}
AIL_DRIVER;

typedef MSS_STRUCT                     // VDI interface register structure
{
   S16 AX;
   S16 BX;
   S16 CX;
   S16 DX;
   S16 SI;
   S16 DI;
}
VDI_CALL;

#endif

//
// Function pointer types
//

typedef void (AILCALLBACK FAR* AILINCB)       (void const FAR *data, S32 len, UINTa user_data);

typedef void (AILCALLBACK FAR* AILTRACECB)    (C8 *text, S32 nest_depth);

typedef void (AILCALLBACK FAR* AILTIMERCB)    (UINTa user);

typedef void (AILCALLBACK FAR* AILSAMPLECB)   (HSAMPLE sample);

typedef F32  (AILCALLBACK FAR* AILFALLOFFCB)  (HSAMPLE sample, F32 distance, F32 rolloff_factor, F32 min_dist, F32 max_dist);

typedef S32  (AILCALLBACK FAR* AILEVENTCB)    (HMDIDRIVER hmi,HSEQUENCE seq,S32 status,S32 data_1,S32 data_2);

typedef S32  (AILCALLBACK FAR* AILTIMBRECB)   (HMDIDRIVER hmi,S32 bank,S32 patch);

typedef S32  (AILCALLBACK FAR* AILPREFIXCB)   (HSEQUENCE seq,S32 log,S32 data);

typedef void (AILCALLBACK FAR* AILTRIGGERCB)  (HSEQUENCE seq,S32 log,S32 data);

typedef void (AILCALLBACK FAR* AILBEATCB)     (HMDIDRIVER hmi,HSEQUENCE seq,S32 beat,S32 measure);

typedef void (AILCALLBACK FAR* AILSEQUENCECB) (HSEQUENCE seq);

typedef S32 (AILCALLBACK FAR *SS_STREAM_CB)   (HSAMPLE  S, S16 FAR *dest_mono_sample_buffer, S32 dest_buffer_size);

//
// Handle to sample and driver being managed by pipeline filter
//

typedef SINTa HSAMPLESTATE;
typedef SINTa HDRIVERSTATE;

//
// Digital pipeline stages
//
// These are the points at which external modules may be installed into
// a given HSAMPLE or HDIGDRIVER's processing pipeline
//

typedef enum
{
   SP_ASI_DECODER = 0,          // Must be "ASI codec stream" provider
   SP_FILTER,                   // Must be "MSS pipeline filter" provider
   SP_FILTER_0 = SP_FILTER,     // Must be "MSS pipeline filter" provider
   SP_FILTER_1,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_2,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_3,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_4,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_5,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_6,                 // Must be "MSS pipeline filter" provider
   SP_FILTER_7,                 // Must be "MSS pipeline filter" provider
   SP_MERGE,                    // Must be "MSS mixer" provider
   N_SAMPLE_STAGES,             // Placeholder for end of list (= # of valid sample pipeline stages)
   SP_OUTPUT = N_SAMPLE_STAGES, // Used to set/get prefs/attribs on a driver's output or matrix filter (if present) 
   SAMPLE_ALL_STAGES            // Used to signify all pipeline stages, for shutdown
}
SAMPLESTAGE;

#define N_SP_FILTER_STAGES 8    // SP_FILTER_0 ... SP_FILTER_7

typedef enum
{
   DP_FLUSH = 0,      // Must be "MSS mixer" provider
   DP_DEFAULT_FILTER, // Must be "MSS pipeline filter" provider (sets the default)
   DP_DEFAULT_MERGE,  // Must be "MSS mixer" provider (sets the default)
   DP_COPY,           // Must be "MSS mixer" provider
   DP_MC_COPY,        // Must be "MSS mixer" provider
   DP_ADPCM_DECODE,   // Must be "MSS mixer" provider
   N_DIGDRV_STAGES,   // Placeholder for end of list (= # of valid stages)
   DIGDRV_ALL_STAGES  // Used to signify all pipeline stages, for shutdown
}
DIGDRVSTAGE;

typedef struct
   {
   ASI_STREAM_OPEN           ASI_stream_open;
   ASI_STREAM_PROCESS        ASI_stream_process;
   ASI_STREAM_SEEK           ASI_stream_seek;
   ASI_STREAM_CLOSE          ASI_stream_close;
   ASI_STREAM_PROPERTY       ASI_stream_property;

   HPROPERTY INPUT_BIT_RATE;
   HPROPERTY INPUT_SAMPLE_RATE;
   HPROPERTY INPUT_BITS;
   HPROPERTY INPUT_CHANNELS;
   HPROPERTY OUTPUT_BIT_RATE;
   HPROPERTY OUTPUT_SAMPLE_RATE;
   HPROPERTY OUTPUT_BITS;
   HPROPERTY OUTPUT_CHANNELS;
   HPROPERTY OUTPUT_CHANNEL_MASK;
   HPROPERTY OUTPUT_RESERVOIR;
   HPROPERTY POSITION;
   HPROPERTY PERCENT_DONE;
   HPROPERTY MIN_INPUT_BLOCK_SIZE;
   HPROPERTY RAW_RATE;
   HPROPERTY RAW_BITS;
   HPROPERTY RAW_CHANNELS;
   HPROPERTY REQUESTED_RATE;
   HPROPERTY REQUESTED_BITS;
   HPROPERTY REQUESTED_CHANS;
   HPROPERTY STREAM_SEEK_POS;
   HPROPERTY DATA_START_OFFSET;
   HPROPERTY DATA_LEN;

   HASISTREAM stream;
   }
ASISTAGE;

typedef struct
   {
   struct _FLTPROVIDER FAR *provider;
   HSAMPLESTATE             sample_state[MAX_SPEAKERS];
   }
FLTSTAGE;

typedef struct
{
   S32       active;    // Pass-through if 0, active if 1
   HPROVIDER provider;

   union
      {
      ASISTAGE ASI;
      MIXER_MERGE MSS_mixer_merge;
      FLTSTAGE FLT;
      }
   TYPE;
}
SPINFO;

typedef struct
{
   S32       active;    // Pass-through if 0, active if 1
   HPROVIDER provider;

   union
      {
      MIXER_FLUSH         MSS_mixer_flush;
      MIXER_COPY          MSS_mixer_copy;
      MIXER_MC_COPY       MSS_mixer_mc_copy;
      MIXER_ADPCM_DECODE  MSS_mixer_adpcm_decode;
      }
   TYPE;
}
DPINFO;

//
// Other data types
//

typedef enum 
{
   WIN32_TIMER_THREAD,   // Handle to thread used to run mixer and all other background tasks/timers
   WIN32_STREAM_THREAD,  // Handle to thread to do disk IO in the streaming API
   WIN32_HWAVEOUT,       // waveOut handle for HDIGDRIVER, if any
   WIN32_HWAVEIN,        // waveIn handle for HDIGINPUT, if any
   WIN32_LPDS,           // lpDirectSound pointer for HSAMPLE
   WIN32_LPDSB,          // lpDirectSoundBuffer pointer for HSAMPLE
   WIN32_HWND,           // HWND that will be used to open DirectSound driver
   WIN32_POSITION_ERR,   // Nonzero if DirectSound play cursor stops moving (e.g., headphones removed)

   PS3_TIMER_THREAD,     // Handle to thread to run background task/timer
   PS3_STREAM_THREAD,    // Handle to thread to do disk IO in the streaming API
   PS3_AUDIO_PORT,       // cellaudio port that Miles is using
   PS3_AUDIO_ADDRESS,    // address of cellaudio sound buffer
   PS3_AUDIO_LENGTH,     // length of cellaudio sound buffer
   PS3_AUDIO_POSITION,   // current playback position of cellaudio sound buffer

   XB_TIMER_THREAD,      // Handle to thread used to run background tasks/timers 
   XB_STREAM_THREAD,     // Handle to thread to do disk IO in the streaming API
   XB_LPDS,              // lpDirectSound pointer for HSAMPLE
   XB_LPDSB,             // lpDirectSoundBuffer pointer for HSAMPLE

   XB360_TIMER_THREAD,   // Handle to thread used to run mixer and all other background tasks/timers
   XB360_STREAM_THREAD,  // Handle to thread to do disk IO in the streaming API
   XB360_LPXAB           // IXAudioSourceVoice pointer for HDIGDRIVER
}
MSS_PLATFORM_PROPERTY;


typedef struct _SMPATTRIBS
{
   F32 cone_inner_angle;
   F32 cone_outer_angle;
   F32 cone_outer_volume;
   F32 max_distance;
   F32 min_distance;
   S32 auto_3D_wet_atten;
   S32 is_3D;
   F32 X_pos;
   F32 Y_pos;
   F32 Z_pos;
   F32 X_face; 
   F32 Y_face; 
   F32 Z_face; 
   F32 X_up;   
   F32 Y_up;   
   F32 Z_up;
   F32 X_meters_per_ms;
   F32 Y_meters_per_ms;
   F32 Z_meters_per_ms;
   F32 channel_levels[MAX_SPEAKERS][MAX_SPEAKERS]; // [dest chans][src chans]
   S32 n_channel_levels;
   F32 exclusion;
   F32 obstruction;
   F32 occlusion;
   S32 loop_count;
   S32 loop_start_offset;
   S32 loop_end_offset;
   F32 LPF_cutoff;
   S32 playback_rate;
   U32 position;
   F32 dry_level;
   F32 wet_level;
   U32 playback_status;

   F32 FL_51_level;
   F32 FR_51_level;
   F32 BL_51_level;
   F32 BR_51_level;
   F32 C_51_level; 
   F32 LFE_51_level;

   F32 left_volume;     // Used in non-5.1 API modes
   F32 right_volume;

   SAMPLESTAGE filter_stages         [MAX_SAVED_FILTER_PROPERTIES];
   HPROVIDER   filter_providers      [MAX_SAVED_FILTER_PROPERTIES];
   UINTa       filter_property_tokens[MAX_SAVED_FILTER_PROPERTIES];
   U32         filter_property_values[MAX_SAVED_FILTER_PROPERTIES];
   S32         n_filter_properties;
}
SMPATTRIBS;

typedef struct _AIL_INPUT_INFO        // Input descriptor type
{
   U32        device_ID;       // DS LPGUID or wave device ID
   U32        hardware_format; // e.g., DIG_F_STEREO_16
   U32        hardware_rate;   // e.g., 22050
   AILINCB    callback;        // Callback function to receive incoming data
   S32        buffer_size;     // Maximum # of bytes to be passed to callback (-1 to use DIG_INPUT_LATENCY)
   UINTa      user_data;       // this is a user defined value
}
AIL_INPUT_INFO;

typedef struct _AILTIMER                 // Timer instance
{
   U32      status;
   AILTIMERCB callback;
   UINTa    user;

   S32      elapsed;
   S32      value;
   S32      callingCT;           // Calling EXE's task number (16 bit only)
   S32      callingDS;           // Calling EXE's DS (used in 16 bit only)
} AILTIMERSTR;

#ifndef IS_WIN64

  #define OFSblocksize     0     // these constants valid for 32-bit versions only!
  #define OFSextrasamples  4
  #define OFSblockleft     8
  #define OFSstep         12
  #define OFSsavesrc      16
  #define OFSsample       20
  #define OFSdestend      24
  #define OFSsrcend       28
  #define OFSsamplesL     32
  #define OFSsamplesR     36
  #define OFSmoresamples  40

#endif

typedef struct LOWPASS_CONSTANT_INFO
{
  S32 A;
  S32 B0, B1;
} LOWPASS_CONSTANT_INFO;

typedef struct LOWPASS_UPDATED_INFO
{
  S32 XL0, XL1;
  S32 YL0, YL1;
} LOWPASS_UPDATED_INFO;

typedef struct LOWPASS_INFO
{
  LOWPASS_UPDATED_INFO u;
  LOWPASS_CONSTANT_INFO c;
  F32 cutoff_param;         
  F32 calculated_cut;
  F32 cutoff;
  S32 on;
} LOWPASS_INFO;

typedef union STAGE_BUFFER
{
  union STAGE_BUFFER * next;
  U8 data[ 1 ];
} STAGE_BUFFER;

typedef struct _MSSVECTOR3D
{
  F32 x;
  F32 y;
  F32 z;
} MSSVECTOR3D;

typedef struct _S3DSTATE           // Portion of HSAMPLE that deals with 3D positioning
{
   MSSVECTOR3D   position;         // 3D position
   MSSVECTOR3D   face;             // 3D orientation
   MSSVECTOR3D   up;               // 3D up-vector
   MSSVECTOR3D   velocity;         // 3D velocity
                
   S32           doppler_valid;    // TRUE if OK to apply Doppler shift
   F32           doppler_shift;    // Scalar for S->playback rate
                 
   F32           inner_angle;      // Cone attenuation parameters
   F32           outer_angle;      // (Angles divided by two and convered to rads for dot-product comparisons)
   F32           outer_volume;
   S32           cone_enabled;
                 
   F32           max_dist;         // Sample distances
   F32           min_dist;
   S32           dist_changed;     // TRUE if min/max distances have changed and need to be sent to the hardware

   S32           auto_3D_atten;    // TRUE if distance/cone attenuation should be applied to wet signal
   F32           atten_3D;         // Attenuation due to distance/cone effects, calculated by software 3D positioner

   HSAMPLE       owner;            // May be NULL if used for temporary/internal calculations
   AILFALLOFFCB  falloff_function; // User function for min/max distance calculations, if desired
} 
S3DSTATE;

typedef struct _SMPBUF           
{
   void const FAR *start;          // Sample buffer address (W)
   U32             len;            // Sample buffer size in bytes (W)
   U32             pos;            // Index to next byte (R/W)
   U32             done;           // Nonzero if buffer with len=0 sent by app
   S32             reset_ASI;      // Reset the ASI decoder at the end of the buffer
   S32             reset_seek_pos; // New destination offset in stream source data, for ASI codecs that care
}
SMPBUF;

typedef struct _SAMPLE           // Sample instance
{
   char       tag[4];            // HSAM
   
   HDIGDRIVER driver;            // Driver for playback

   S32        index;             // Numeric index of this sample 

   SMPBUF   buf[8];              // Source data buffers

   U32      src_fract;           // Fractional part of source address


   // these are dynamic arrays sized as n_channels long (so 1 for mono, 2 stereo, 6 for 5.1)
   S32 * left_val;
   S32 * right_val;
   S32 * last_decomp;
   LOWPASS_INFO *lp;             // low pass info


   // these are arrays pointing to dynamic arrays, each of the sub arrays are n_channels long or [MAX_SPEAKERS][n_channels]
   F32     *user_channel_levels[MAX_SPEAKERS+1]; // Channel levels set by AIL_set_sample_channel_levels() [source_channels][driver->logical_channels]
   S32     *cur_scale          [MAX_SPEAKERS+1]; // Calculated 11-bit volume scale factors for current/previous mixing interval
   S32     *prev_scale         [MAX_SPEAKERS+1]; // (These are all indexed by build buffer*2, not speaker indexes!)
   S32     *ramps_left         [MAX_SPEAKERS+1];

   S32      lp_any_on;           // are any of the low pass filters on?
   S32      user_channels_need_deinterlace;      // do any of the user channels require a stereo sample to be deinterlaced?
   

   S32      n_buffers;           // # of buffers (default = 2)
   S32      head;
   S32      tail;
   S32      starved;             // Buffer stream has run out of data
   S32      exhaust_ASI;         // Are we prolonging the buffer lifetime until ASI output is exhausted?

   S32      loop_count;          // # of cycles-1 (1=one-shot, 0=indefinite)
   S32      loop_start;          // Starting offset of loop block (0=SOF)
   S32      loop_end;            // End offset of loop block (-1=EOF)
   S32      orig_loop_count;     // Original loop properties specified by app, before any
   S32      orig_loop_start;     // alignment constraints
   S32      orig_loop_end;

   S32      format;              // DIG_F format (8/16 bits, mono/stereo/multichannel)
   S32      n_channels;          // # of channels (which can be >2 for multichannel formats)
   U32      channel_mask;        // Same as WAVEFORMATEXTENSIBLE.dwChannelMask

   S32      playback_rate;       // Playback rate in hertz

   F32      save_volume;         // Sample volume 0-1.0
   F32      save_pan;            // Mono panpot/stereo balance (0=L ... 1.0=R)

   F32      left_volume;         // Left/mono volume 0 to 1.0
   F32      right_volume;        // Right volume 0 to 1.0

   F32      wet_level;           // reverb level 0 to 1.0
   F32      dry_level;           // non-reverb level 0 to 1.0

   F32      obstruction;
   F32      occlusion;
   F32      exclusion;

   F32      auto_3D_channel_levels[MAX_SPEAKERS]; // Channel levels set by 3D positioner (always 1.0 if not 3D-positioned)

   F32      speaker_levels[MAX_SPEAKERS];         // one level per speaker (multiplied after user or 3D)

   S8       speaker_enum_to_source_chan[20];   // array[MSS_SPEAKER_xx] = -1 if not present, else channel #

#if 0
   MSS_SPEAKER *source_chan_to_speaker_enum;       // array[channel #] = -1 if not present, else MSS_SPEAKER equate
#endif

   S32      service_type;        // 1 if single-buffered; 2 if streamed

   AILSAMPLECB  SOB;             // Start-of-block callback function
   AILSAMPLECB  EOB;             // End-of-buffer callback function
   AILSAMPLECB  EOS;             // End-of-sample callback function

   SINTa    user_data  [8];      // Miscellaneous user data
   SINTa    system_data[8];      // Miscellaneous system data

   ADPCMDATA adpcm;

   S32      doeob;               // Flags to trigger callbacks
   S32      dosob;
   S32      doeos;

   //
   // Sample pipeline stages
   //

   SPINFO   pipeline[N_SAMPLE_STAGES];
   S32      n_active_filters;    // # of SP_FILTER_n stages active

   //
   // 3D-related state for all platforms (including Xbox)
   //

   S32      is_3D;               // TRUE if channel levels are derived automatically from 3D positional state, FALSE if they're controlled manually
   S3DSTATE S3D;                 // Software version applies 3D positioning only if is_3D == TRUE, but output filters always use it

#ifdef MSS_VFLT_SUPPORTED
   void FAR *voice;              // Optional object used by output filter to store per-sample information such as DS3D buffers
#endif

   //
   // Platform-specific members
   //

#ifdef IS_XBOX
   AILLPDIRECTSOUNDBUFFER pDSB;

   S32 hw_rate;
   S32 hw_channels;
   S32 hw_bits;
   S32 direct_mode;

   STAGE_BUFFER * ds_staging_buffer;
   S32 waiting_for_which_half;
   S32 cleared_bufs;
   S32 cleared_bytes;
   S32 need_more_zeroes;
   S32 ds_stage_size;

   F32 shadow_FL;
   F32 shadow_FR;
   F32 shadow_FC;
   F32 shadow_LF;
   F32 shadow_BL;
   F32 shadow_BR;

   MSSVECTOR3D shadow_position;         
   MSSVECTOR3D shadow_face;             
   MSSVECTOR3D shadow_velocity;         
   S32         shadow_playback_rate;
   F32         shadow_volume;
   F32         shadow_occlusion;
   F32         shadow_obstruction;
   F32         shadow_exclusion;
   F32         shadow_min_dist;
   F32         shadow_max_dist;
   F32         shadow_inner_angle;
   F32         shadow_outer_angle;
   F32         shadow_outer_volume;
   F32         shadow_wet_level;
   F32         shadow_dry_level;
   S32         shadow_auto_3D_atten;
   S32         shadow_is_3D;

#endif

   F32 leftb_volume;         // Left/mono volume 0 to 1.0 (back)
   F32 rightb_volume;        // Right volume 0 to 1.0 (back)
   F32 center_volume;        // Center volume 0 to 1.0
   F32 low_volume;           // Low volume 0 to 1.0
   F32 save_fb_pan;          // Sample volume 0-1.0
   F32 save_center;          // saved center level
   F32 save_low;             // saved sub level

#if defined(HOST_SPU_PROCESS) || defined(MSS_SPU_PROCESS)
   S32    spu_on;
   U32    align[3];
#endif

#ifdef IS_WINDOWS

   //
   // DirectSound-specific data
   //

   S32      service_interval;    // Service sample every n ms
   S32      service_tick;        // Current service countdown value
   S32      buffer_segment_size; // Buffer segment size to fill

   S32      prev_segment;        // Previous segment # (0...n)
   S32      prev_cursor;         // Previous play cursor location

   S32      bytes_remaining;     // # of bytes left to play (if not -1)

   S32      direct_control;      // 1 if app controls buffer, 0 if MSS

#endif
}
SAMPLE;

//
// used for AIL_process
//

typedef struct _AILMIXINFO {
  AILSOUNDINFO Info;
  ADPCMDATA mss_adpcm;
  U32 src_fract;
  S32 left_val;
  S32 right_val;
} AILMIXINFO;


#if defined(IS_WINDOWS) || defined(IS_MAC) || defined(IS_LINUX) || defined(IS_XBOX) || defined(IS_XENON) || defined(IS_PS3) || defined(IS_PS2) || defined(IS_WII)

DXDEC  U32     AILCALL  AIL_get_timer_highest_delay   (void);

DXDEC  void    AILCALL AIL_serve(void);

#ifdef IS_MAC

typedef void * LPSTR;

#define WHDR_DONE 0

typedef struct _WAVEIN
{
  long temp;
} * HWAVEIN;

typedef struct _WAVEHDR
{
  S32  dwFlags;
  S32  dwBytesRecorded;
  S32  dwUser;
  S32  temp;
  void * lpData;
  S32  dwBufferLength;
  S32  longdwLoops;
  S32  dwLoops;
  void * lpNext;
  U32  * reserved;

} WAVEHDR, * LPWAVEHDR;

#endif

#define N_WAVEIN_BUFFERS 8     // Use a ring of 8 buffers by default

typedef struct _DIG_INPUT_DRIVER FAR *HDIGINPUT; // Handle to digital input driver

#ifdef IS_MAC

#define AIL_DIGITAL_INPUT_DEFAULT 0

typedef struct _DIG_INPUT_DRIVER    // Handle to digital input driver
{
   C8  tag[4];                      // HDIN
   S32 input_enabled;               // 1 if enabled, 0 if not
   U32 incoming_buffer_size;
   void * incoming_buffer[ 2 ];
   void* outgoing_buffer;
   U32 which_buffer;
   AIL_INPUT_INFO info;             // Input device descriptor
   AILMIXINFO incoming_info;
   long device;
   #if defined(__SOUND__)
   SPB record_rec;
   #endif
} DIG_INPUT_DRIVER;

#else

#define AIL_DIGITAL_INPUT_DEFAULT ((U32)WAVE_MAPPER)

typedef struct _DIG_INPUT_DRIVER    // Handle to digital input driver
{
   C8     tag[4];                   // HDIN

   HTIMER background_timer;         // Background timer handle

   AIL_INPUT_INFO info;             // Input device descriptor
   
   S32       input_enabled;         // 1 if enabled, 0 if not

   UINTa     callback_user;         // Callback user value

   //
   // Provider-independent data
   //

   U32       DMA_size;              // Size of each DMA sub-buffer in bytes
   void FAR *DMA[N_WAVEIN_BUFFERS]; // Simulated DMA buffers

   U32       silence;               // Silence value for current format (0 or 128)

   S32       device_active;         // 1 if buffers submittable, 0 if not

#ifdef IS_WINDOWS
   //
   // waveOut-specific data
   //

   HWAVEIN          hWaveIn;                        // Handle to wave input device
   volatile MWAVEHDR wavehdr[N_WAVEIN_BUFFERS];     // Handles to wave headers

#endif
}
DIG_INPUT_DRIVER;
#endif

#endif

typedef struct REVERB_CONSTANT_INFO
{
  F32 FAR* start0, FAR* start1, FAR* start2, FAR* start3, FAR* start4, FAR* start5;
  F32 FAR* end0, FAR* end1, FAR* end2, FAR* end3, FAR* end4, FAR* end5;
  F32 C0, C1, C2, C3, C4, C5;
  F32 A;
  F32 B0, B1;
} REVERB_CONSTANT_INFO;

typedef struct REVERB_UPDATED_INFO
{
  F32 FAR * address0, FAR * address1, FAR * address2, FAR * address3, FAR * address4, FAR * address5;
  F32 X0, X1, Y0, Y1;
} REVERB_UPDATED_INFO;

typedef struct REVERB_INFO
{
  REVERB_UPDATED_INFO u;
  REVERB_CONSTANT_INFO c;
} REVERB_INFO;

typedef struct _MSS_RECEIVER_LIST
{
   MSSVECTOR3D direction;                      // Normalized direction vector from listener

   S32         speaker_index[MAX_SPEAKERS];    // List of speakers affected by sounds in this direction
   F32         speaker_level[MAX_SPEAKERS];    // Each speaker's degree of effect from this source
   S32         n_speakers_affected; 
}
MSS_RECEIVER_LIST;

typedef struct _D3DSTATE
{
   S32         mute_at_max;

   MSSVECTOR3D listen_position;
   MSSVECTOR3D listen_face;
   MSSVECTOR3D listen_up;
   MSSVECTOR3D listen_cross;
   MSSVECTOR3D listen_velocity;

   F32         rolloff_factor;
   F32         doppler_factor;
   F32         distance_factor;
   F32         falloff_power;

   //
   // Precalculated listener info
   //

   S32         ambient_channels    [MAX_SPEAKERS];      // E.g., LFE
   S32         n_ambient_channels;

   S32         directional_channels[MAX_SPEAKERS+1];    // Channel index, or -1 if virtual
   MSSVECTOR3D listener_to_speaker [MAX_SPEAKERS+1];  
   S32         n_directional_channels;

   MSS_RECEIVER_LIST receiver_specifications[MAX_RECEIVER_SPECS];  // Constellation of receiver vectors
   S32               n_receiver_specs;         

   MSSVECTOR3D speaker_positions           [MAX_SPEAKERS]; // Listener-relative speaker locations
   F32         speaker_wet_reverb_response [MAX_SPEAKERS]; // Reverb sensitivity of each speaker
   F32         speaker_dry_reverb_response [MAX_SPEAKERS];
} 
D3DSTATE;

typedef enum 
{
   MSS_MC_INVALID             = 0,       // Used for configuration-function errors
   MSS_MC_MONO                = 1,       // For compatibility with S32 channel param
   MSS_MC_STEREO              = 2,
   MSS_MC_USE_SYSTEM_CONFIG   = 0x10,    // Leave space between entries for new variations
   MSS_MC_HEADPHONES          = 0x20,    // with similar quality levels/speaker counts
   MSS_MC_DOLBY_SURROUND      = 0x30,    
   MSS_MC_SRS_CIRCLE_SURROUND = 0x40,
   MSS_MC_40_DTS              = 0x48,
   MSS_MC_40_DISCRETE         = 0x50,
   MSS_MC_51_DTS              = 0x58,
   MSS_MC_51_DISCRETE         = 0x60,
   MSS_MC_61_DISCRETE         = 0x70,
   MSS_MC_71_DISCRETE         = 0x80,
   MSS_MC_81_DISCRETE         = 0x90,
   MSS_MC_DIRECTSOUND3D       = 0xA0,
   MSS_MC_EAX2                = 0xC0,
   MSS_MC_EAX3                = 0xD0,
   MSS_MC_EAX4                = 0xE0
}
MSS_MC_SPEC;

typedef struct _DIG_DRIVER          // Handle to digital audio driver
{
   char        tag[4];              // HDIG

   HTIMER      backgroundtimer;     // Background timer handle

   F32         master_volume;       // Master sample volume 0-1.0

   S32         DMA_rate;            // Hardware sample rate
   S32         hw_format;           // DIG_F code in use
   S32         n_active_samples;    // # of samples being processed

   MSS_MC_SPEC channel_spec;        // Original "channels" value passed to AIL_open_digital_driver()

   D3DSTATE    D3D;                 // 3D listener parms for all platforms

#ifdef IS_XBOX
   HDIGDRIVER          next;        // Pointer to next HDIGDRIVER in use
   AILLPDIRECTSOUND    pDS;         // DirectSound output driver
   S32                 use_3d;      // are we using 3D?
   S32                 use_reverb;

   STAGE_BUFFER * staging_buffer_ring;
   STAGE_BUFFER * staging_mem_ptr;

   MSSVECTOR3D shadow_listen_position;
   MSSVECTOR3D shadow_listen_face;
   MSSVECTOR3D shadow_listen_up;
   MSSVECTOR3D shadow_listen_velocity;
   F32         shadow_listen_doppler_factor;
   F32         shadow_listen_distance_factor;
   F32         shadow_listen_rolloff_factor;
   S32         shadow_room_type;
   F32         shadow_master_wet;     
   F32         shadow_reverb_decay_time_s;    
   F32         shadow_reverb_damping; 
   F32         shadow_reverb_predelay_s;

#else
   S32         quiet;               // # of consecutive quiet sample periods
   S32         playing;             // Playback active if non-zero
#endif

   S32         bytes_per_channel;            // # of bytes per channel (always 1 or 2 for 8- or 16-bit hardware output)
   S32         samples_per_buffer;           // # of samples per build buffer / half-buffer
   S32         physical_channels_per_sample; // # of channels per *physical* sample (1 or 2, or more in discrete MC mode)
   S32         logical_channels_per_sample;  // # of logical channels per sample (may differ from physical channel count in matrix formats)

#ifdef IS_LINUX
   S32         released;            // has the sound manager been released?
#endif

#ifdef IS_PS2
   struct RAD_IOP_SOUND_DATA * IOP;
   S32         IOP_overwrite;       // FALSE if RADIOP layer requests addition to output buffer (rather than writes)
   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use
#endif

   HSAMPLE     samples;             // Pointer to list of SAMPLEs
   
   U32        *sample_status;       // SMP_ flags: _FREE, _DONE, _PLAYING, moved out of SAMPLEs for faster iteration
   S32         n_samples;           // # of SAMPLEs

   SINTa       system_data[8];      // Miscellaneous system data

   //
   // Build buffers
   //
   // In multichannel mode, source samples may be mixed into more than one 
   // build buffer
   //

   MSS_BB      build[MAX_SPEAKERS+EXTRA_BUILD_BUFFERS];
   S32         n_build_buffers;      // # of build buffers actually used for output processing

   S32         hardware_buffer_size; // Size of each output buffer

#ifdef IS_WINDOWS

   //
   // waveOut-specific interface data
   //

   HWAVEOUT    hWaveOut;            // Wave output driver

   U32         reset_works;         // TRUE if OK to do waveOutReset
   U32         request_reset;       // If nonzero, do waveOutReset ASAP

   LPWAVEHDR   first;               // Pointer to first WAVEHDR in chain
   S32         n_buffers;           // # of output WAVEHDRs in chain

   LPWAVEHDR volatile FAR *return_list; // Circular list of returned WAVEHDRs
   S32       volatile      return_head; // Head of WAVEHDR list (insertion point)
   S32       volatile      return_tail; // Tail of WAVEHDR list (retrieval point)

   //
   // DirectSound-specific interface data
   //

   UINTa                  guid;        // The guid id of the ds driver
   AILLPDIRECTSOUND       pDS;         // DirectSound output driver (don't
                                       // use with Smacker directly anymore!)

   U32                    ds_priority; // priority opened with

   S32                    emulated_ds; // is ds emulated or not?
   AILLPDIRECTSOUNDBUFFER lppdsb;      // primary buffer or null

   UINTa                  dsHwnd;      // HWND used with DirectSound

   AILLPDIRECTSOUNDBUFFER FAR * lpbufflist;   // List of pointers to secondary buffers
   HSAMPLE         FAR *samp_list;      // HSAMPLE associated with each buffer
   S32             FAR *sec_format;     // DIG_F_ format for secondary buffer
   S32                  max_buffs;      // Max. allowable # of secondary buffers

   //
   // Driver output configuration
   //
   // Note: # of "logical" (source) channels per sample = dig->channels_per_sample
   //       # of "physical" (DAC) channels per sample = dig->wformat.wf.nChannels
   //
   //       These may be different if a matrix format (e.g., Dolby/SRS) 
   //       is in use! 
   //

   MPCMWAVEFORMAT wformat;          // format from waveout open
   C8             wfextra[32];      // Extension to PCMWAVEFORMAT (e.g., WAVE_FORMAT_EXTENSIBLE)

   //
   // Misc. data
   //

   S32         released;            // has the sound manager been released?

   U32         foreground_timer;    // Foreground timer handle

   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use
   S32      callingCT;              // Calling EXE's task number (16 bit only)
   S32      callingDS;              // Calling EXE's DS (used in 16 bit only)

   //
   // Vars for waveOut emulation
   //

   S32 DS_initialized;

   AILLPDIRECTSOUNDBUFFER DS_sec_buff;    // Secondary buffer (or NULL if none)
   AILLPDIRECTSOUNDBUFFER DS_out_buff;    // Output buffer (may be sec or prim)
   S32 DS_buffer_size;                    // Size of entire output buffer

   S32 DS_frag_cnt;                 // Total fragment count and size, and
   S32 DS_frag_size;                // last fragment occupied by play cursor
   S32 DS_last_frag;
   S32 DS_last_write;
   S32 DS_last_timer;
   S32 DS_skip_time;

   S32 DS_use_default_format;       // 1 to force use of default DS primary buffer format

   U32 position_error;              // last status from position report (can be used
                                    //   to watch for headset removal)
   U32 last_ds_play;
   U32 last_ds_write;
   U32 last_ds_move;
#else

   #ifdef IS_DOS

   U32         hw_mode_flags;       // DIG_PCM_ flags for mode in use

   // must be first in the DOS section
   void       *DMA[2];              // Protected-mode pointers to half-buffers
                                    // (note that DMA[0] may != DMA_buf)


   REALFAR     DMA_seg;             // Seg:off pointer to DMA buffers (off=0)
   U32         DMA_sel;             // Selector for DMA buffers (off=0)
   void       *DMA_buf;             // Protected-mode pointer to DMA buffers

   S16        *buffer_flag;         // Protected-mode pointer to buffer flag
   S32         last_buffer;         // Last active buffer flag value in driver

   AIL_DRIVER *drvr;                // Base driver descriptor

   DIG_DDT    *DDT;                 // Protected-mode pointer to DDT
   DIG_DST    *DST;                 // Protected-mode pointer to DST

   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use

   #endif

#endif

#ifdef IS_X86
   S32         use_MMX;             // Use MMX with this driver if TRUE
#endif

   U32 us_count;
   U32 ms_count;
   U32 last_ms_polled;
   U32 last_percent;

   void * MC_buffer;
   
   //
   // Digital driver pipeline filter stages
   //

   DPINFO pipeline[N_DIGDRV_STAGES];

#ifdef MSS_VFLT_SUPPORTED
   struct _FLTPROVIDER FAR *voice_filter;
   SS_STREAM_CB             stream_callback;
#endif

   struct _FLTPROVIDER FAR *matrix_filter;

   //
   // Reverb
   //

   S32      room_type;           // Changes to this drive master_wet and duration/damping/predelay!
   F32      master_wet;          // Master reverb level 0-1.0
   F32      master_dry;          // Master non-reverb level 0-1.0

   REVERB_INFO ri;

   S32 FAR *reverb_build_buffer;
   S32      reverb_total_size;
   S32      reverb_fragment_size;
   S32      reverb_buffer_size;
   S32      reverb_on;
   U32      reverb_off_time_ms;

   U32      reverb_duration_ms;

   F32      reverb_decay_time_s;
   F32      reverb_predelay_s;
   F32      reverb_damping;

   S32      reverb_head;
   S32      reverb_tail;

#ifdef IS_XENON
   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use

   void * hw_buf;
   U32    hw_datarate;
   U32    hw_align;
   void * XA_voice;
   S32    XA_buffer_size;
   S32    XA_frag_cnt;
   S32    XA_frag_size;
   S32    XA_last_frag;
   S32    XA_last_write;
   S32    XA_skip_time;
   U32    last_xa_play;
   U32    last_xa_move;
   S32    XA_last_timer;
#endif

#ifdef IS_PS3
   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use

   void * hw_buf;
   U32    hw_datarate;
   U32    hw_align;
   U32    port;
   S32    hw_buffer_size;
   S32    snd_frag_cnt;
   S32    snd_frag_size;
   S32    snd_last_frag;
   S32    snd_last_write;
   S32    snd_skip_time;
   U32    snd_last_play;
   U32    snd_last_move;
   S32    snd_last_timer;
#endif


#ifdef IS_WII
   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use

   U32    hw_datarate;
   S32    hw_buffer_size;
   S32    each_buffer_size;
   S32    snd_frag_cnt;
   S32    snd_frag_size;
   S32    snd_last_frag;
   S32    snd_last_write;
   S32    snd_skip_time;
   U32    snd_last_play;
   U32    snd_last_move;
   S32    snd_last_timer;

   void * buffer[ 2 ];
   U32    physical[ 2 ];

   #ifdef AX_OUTPUT_BUFFER_DOUBLE
   AXVPB* voice[ 2 ];
   #endif
   
#endif


#ifdef IS_WINDOWS
   S32                  no_wom_done;    // don't process WOM_DONEs on this driver
   U32                  wom_done_buffers;
#endif

#ifdef XAUDIOFRAMESIZE_NATIVE
   XAUDIOPACKET packet;
#endif

#ifdef IS_MAC
   U32         n_samples_allocated;
   U32         n_samples_used;
   U32         n_samples_played;
   U32         total_buffer_size;

   HDIGDRIVER  next;                // Pointer to next HDIGDRIVER in use
   U32         reset_works;         // TRUE if OK to do waveOutReset
   U32         request_reset;       // If nonzero, do waveOutReset ASAP
   S32         released;            // has the sound manager been released?

   U32            loaded[2];
   U32            work_buffer;
   U32            play_buffer;

   #if defined(__SOUND__)
   ExtSoundHeader sound_header;
   SndChannelPtr  sound_channel;
   SndCallBackUPP global_callback;
   Ptr            buffers[2];
   #endif
#endif

#if defined(HOST_SPU_PROCESS) || defined(MSS_SPU_PROCESS)
   U32    spu_num;
   S32    spu_on;
#endif
}
DIG_DRIVER;

typedef struct                      // MIDI status log structure
   {
   S32      program   [NUM_CHANS];  // Program Change
   S32      pitch_l   [NUM_CHANS];  // Pitch Bend LSB
   S32      pitch_h   [NUM_CHANS];  // Pitch Bend MSB

   S32      c_lock    [NUM_CHANS];  // Channel Lock
   S32      c_prot    [NUM_CHANS];  // Channel Lock Protection
   S32      c_mute    [NUM_CHANS];  // Channel Mute
   S32      c_v_prot  [NUM_CHANS];  // Voice Protection
   S32      bank      [NUM_CHANS];  // Patch Bank Select
   S32      gm_bank_l [NUM_CHANS];  // GM Bank Select
   S32      gm_bank_m [NUM_CHANS];  // GM Bank Select
   S32      indirect  [NUM_CHANS];  // ICA indirect controller value
   S32      callback  [NUM_CHANS];  // Callback Trigger

   S32      mod       [NUM_CHANS];  // Modulation
   S32      vol       [NUM_CHANS];  // Volume
   S32      pan       [NUM_CHANS];  // Panpot
   S32      exp       [NUM_CHANS];  // Expression
   S32      sus       [NUM_CHANS];  // Sustain
   S32      reverb    [NUM_CHANS];  // Reverb
   S32      chorus    [NUM_CHANS];  // Chorus

   S32      bend_range[NUM_CHANS];  // Bender Range (data MSB, RPN 0 assumed)

   S32      RPN_L     [NUM_CHANS];  // RPN # LSB
   S32      RPN_M     [NUM_CHANS];  // RPN # MSB
   }
CTRL_LOG;

typedef struct _SEQUENCE                  // XMIDI sequence state table
{
   char     tag[4];                       // HSEQ

   HMDIDRIVER driver;                     // Driver for playback

   U32      status;                       // SEQ_ flagsstruct

   void const   FAR *TIMB;                // XMIDI IFF chunk pointers
   void const   FAR *RBRN;
   void const   FAR *EVNT;

   U8 const     FAR *EVNT_ptr;            // Current event pointer

   U8      FAR *ICA;                      // Indirect Controller Array

   AILPREFIXCB   prefix_callback;         // XMIDI Callback Prefix handler
   AILTRIGGERCB  trigger_callback;        // XMIDI Callback Trigger handler
   AILBEATCB     beat_callback;           // XMIDI beat/bar change handler
   AILSEQUENCECB EOS;                     // End-of-sequence callback function

   S32      loop_count;                   // 0=one-shot, -1=indefinite, ...

   S32      interval_count;               // # of intervals until next event
   S32      interval_num;                 // # of intervals since start

   S32      volume;                       // Sequence volume 0-127
   S32      volume_target;                // Target sequence volume 0-127
   S32      volume_accum;                 // Accumulated volume period
   S32      volume_period;                // Period for volume stepping

   S32      tempo_percent;                // Relative tempo percentage 0-100
   S32      tempo_target;                 // Target tempo 0-100
   S32      tempo_accum;                  // Accumulated tempo period
   S32      tempo_period;                 // Period for tempo stepping
   S32      tempo_error;                  // Error counter for tempo DDA

   S32      beat_count;                   // Sequence playback position
   S32      measure_count;

   S32      time_numerator;               // Sequence timing data
   S32      time_fraction;
   S32      beat_fraction;
   S32      time_per_beat;

   void const FAR *FOR_ptrs[FOR_NEST];    // Loop stack
   S32      FOR_loop_count [FOR_NEST];

   S32      chan_map       [NUM_CHANS];   // Physical channel map for sequence

   CTRL_LOG shadow;                       // Controller values for sequence

   S32      note_count;                   // # of notes "on"

   S32      note_chan      [MAX_NOTES];   // Channel for queued note (-1=free)
   S32      note_num       [MAX_NOTES];   // Note # for queued note
   S32      note_time      [MAX_NOTES];   // Remaining duration in intervals

   SINTa    user_data  [8];               // Miscellaneous user data
   SINTa    system_data[8];               // Miscellaneous system data

} SEQUENCE;

#if defined(IS_MAC) || defined(IS_LINUX) || defined(IS_XBOX) || defined(IS_XENON) || defined(IS_PS2) || defined(IS_PS3) || defined(IS_WII)

struct MIDIHDR;
struct MIDIOUT;
typedef struct MIDIOUT* HMIDIOUT;
typedef HMIDIOUT* LPHMIDIOUT;

#endif

typedef struct _MDI_DRIVER          // Handle to XMIDI driver
{
   char     tag[4];                 // HMDI

   HTIMER      timer;               // XMIDI quantization timer
   S32         interval_time;       // XMIDI quantization timer interval in uS

   S32         disable;             // > 0 to disable XMIDI service

   HSEQUENCE   sequences;           // Pointer to list of SEQUENCEs
   S32         n_sequences;         // # of SEQUENCEs

   S32         lock  [NUM_CHANS];   // 1 if locked, 2 if protected, else 0
   HSEQUENCE   locker[NUM_CHANS];   // HSEQUENCE which locked channel
   HSEQUENCE   owner [NUM_CHANS];   // HSEQUENCE which owned locked channel
   HSEQUENCE   user  [NUM_CHANS];   // Last sequence to use channel
   S32         state [NUM_CHANS];   // Lock state prior to being locked

   S32         notes [NUM_CHANS];   // # of active notes in channel

   AILEVENTCB  event_trap;          // MIDI event trap callback function
   AILTIMBRECB timbre_trap;         // Timbre request callback function

   S32         master_volume;       // Master XMIDI note volume 0-127

   SINTa       system_data[8];      // Miscellaneous system data

#if defined(IS_WINDOWS) || defined(IS_MAC) || defined(IS_LINUX)

   S32         released;            // has the hmidiout handle been released
   U32         deviceid;            // ID of the MIDI device
   U8      FAR *sysdata;            // SysEx buffer

#endif

#if defined(IS_XBOX) || defined(IS_XENON) || defined(IS_WII) || defined(IS_PS2) || defined(IS_PS3)
   HMDIDRIVER  next;                // Pointer to next HMDIDRIVER in use
#endif

#ifdef IS_LINUX
   struct MIDIHDR FAR *mhdr;        // SysEx header
   HMDIDRIVER  next;                // Pointer to next HMDIDRIVER in use
   HMIDIOUT    hMidiOut;            // MIDI output driver
#endif

#ifdef IS_WINDOWS

   struct midihdr_tag FAR *mhdr;               // SysEx header

   HMDIDRIVER  next;                // Pointer to next HMDIDRIVER in use
   S32      callingCT;              // Calling EXE's task number (16 bit only)
   S32      callingDS;              // Calling EXE's DS (used in 16 bit only)

   HMIDIOUT    hMidiOut;            // MIDI output driver

#else

   #ifdef IS_DOS

   S32        message_count;       // MIDI message count
   S32        offset;              // MIDI buffer offset

   AIL_DRIVER *drvr;               // Base driver descriptor

   MDI_DDT    *DDT;                // Protected-mode pointer to DDT
   MDI_DST    *DST;                // Protected-mode pointer to DST
   #else
     #if defined(IS_MAC)
       struct MIDIHDR FAR *mhdr;           // SysEx header
       HMDIDRIVER  next;                // Pointer to next HMDIDRIVER in use
       HMIDIOUT    hMidiOut;            // MIDI output driver
     #endif
   #endif

#endif

}
MDI_DRIVER;

typedef MSS_STRUCT                      // XMIDI TIMB IFF chunk
   {
   S8    name[4];

   U8    msb;
   U8    lsb;
   U8    lsb2;
   U8    lsb3;

   U16   n_entries;

   U16   timbre[1];
   }
TIMB_chunk;

typedef MSS_STRUCT                      // XMIDI RBRN IFF entry
   {
   S16   bnum;
   U32   offset;
   }
RBRN_entry;

typedef struct                      // Wave library entry
{
   S32   bank;                      // XMIDI bank, MIDI patch for sample
   S32   patch;

   S32   root_key;                  // Root MIDI note # for sample (or -1)

   U32   file_offset;               // Offset of wave data from start-of-file
   U32   size;                      // Size of wave sample in bytes

   S32   format;                    // DIG_F format (8/16 bits, mono/stereo)
   S32   playback_rate;             // Playback rate in hertz
}
WAVE_ENTRY;

typedef struct                      // Virtual "wave synthesizer" descriptor
{
   HMDIDRIVER mdi;                  // MIDI driver for use with synthesizer
   HDIGDRIVER dig;                  // Digital driver for use with synthesizer

   WAVE_ENTRY FAR *library;         // Pointer to wave library

   AILEVENTCB  prev_event_fn;       // Previous MIDI event trap function
   AILTIMBRECB prev_timb_fn;        // Previous timbre request trap function

   CTRL_LOG    controls;            // MIDI controller states

   WAVE_ENTRY FAR *wave [NUM_CHANS];// Pointer to WAVE_ENTRY for each channel

   HSAMPLE     S    [MAX_W_VOICES]; // List of HSAMPLE voices
   S32         n_voices;            // Actual # of voices allocated to synth

   S32         chan [MAX_W_VOICES]; // MIDI channel for each voice, or -1
   S32         note [MAX_W_VOICES]; // MIDI note number for voice
   S32         root [MAX_W_VOICES]; // MIDI root note for voice
   S32         rate [MAX_W_VOICES]; // Playback rate for voice
   S32         vel  [MAX_W_VOICES]; // MIDI note velocity for voice
   U32         time [MAX_W_VOICES]; // Timestamp for voice

   U32         event;               // Event counter for LRU timestamps
}
WAVE_SYNTH;

typedef WAVE_SYNTH FAR * HWAVESYNTH;// Handle to virtual wave synthesizer

//
// Handle to thread which called AIL_startup()
//
// This thread is suspended by MSS callback threads, to simulate DOS-style
// interrupt handler behavior
//

#ifdef IS_WIN32API

#ifdef NTAPI
extern HANDLE hAppThread;
#endif

#endif

//
// Background flag for timers
//

extern volatile S32   AIL_bkgnd_flag;

//
// Global preference array
//

extern SINTa          AIL_preference   [N_PREFS];

//
// DIG_DRIVER list
//

extern HDIGDRIVER     DIG_first;

//
// MDI_DRIVER list
//

extern HMDIDRIVER     MDI_first;

//
// Miscellaneous system services
//

#define FILE_READ_WITH_SIZE ((void FAR*)(S32)-1)

#ifndef NO_OLD_SYS_FUNCTIONS

#define MEM_alloc_lock AIL_mem_alloc_lock
#define MEM_free_lock  AIL_mem_free_lock
#define FILE_error     AIL_file_error
#define FILE_size      AIL_file_size
#define FILE_read      AIL_file_read
#define FILE_write     AIL_file_write

#ifdef IS_DOS

#define MEM_alloc        AIL_mem_alloc
#define MEM_free         AIL_mem_free
#define MEM_use_malloc   AIL_mem_use_malloc
#define MEM_use_free     AIL_mem_use_free
#define MEM_alloc_DOS    AIL_mem_alloc_DOS
#define MEM_free_DOS     AIL_mem_free_DOS
#define VMM_lock_range   AIL_vmm_lock_range
#define VMM_unlock_range AIL_vmm_unlock_range
#define VMM_lock         AIL_vmm_lock
#define VMM_unlock       AIL_vmm_unlock

#endif

#endif

extern S32 AILCALLBACK SP_ASI_DECODER_callback(UINTa     user,
                                               void FAR *dest,
                                               S32       bytes_requested,
                                               S32       offset);

DXDEC  void FAR * AILCALL AIL_mem_alloc_lock(UINTa     size);
DXDEC  void       AILCALL AIL_mem_free_lock (void FAR *ptr);

DXDEC  S32        AILCALL AIL_file_error   (void);

DXDEC  S32        AILCALL AIL_file_size    (char const FAR  *filename);

DXDEC  void FAR * AILCALL AIL_file_read    (char const FAR   *filename,
                                           void FAR *dest);

DXDEC  S32        AILCALL AIL_file_write   (char const FAR    *filename,
                                            void const FAR  *buf,
                                            U32       len);

DXDEC  S32        AILCALL AIL_WAV_file_write
                                           (char const FAR    *filename,
                                            void const FAR  *buf,
                                            U32       len,
                                            S32       rate,
                                            S32       format);

DXDEC  S32        AILCALL AIL_file_append  (char const FAR *filename,
                                            void const FAR *buf, U32 len);

#ifndef IS_DOS

typedef void FAR * (AILCALLBACK FAR *AILMEMALLOCCB)(UINTa size);
typedef void (AILCALLBACK FAR *AILMEMFREECB)(void FAR *);

DXDEC AILMEMALLOCCB AILCALL AIL_mem_use_malloc(AILMEMALLOCCB fn);
DXDEC AILMEMFREECB AILCALL AIL_mem_use_free  (AILMEMFREECB fn);

#endif

#ifdef ON_MAC_USE_FSS

DXDEC  S32        AILCALL AIL_file_fss_size(FSSpec const FAR  *filename);

DXDEC  void FAR * AILCALL AIL_file_fss_read(FSSpec const FAR   *filename,
                                            void FAR *dest);

DXDEC  S32        AILCALL AIL_file_fss_write(FSSpec const FAR    *filename,
                                             void const FAR  *buf,
                                             U32       len);

DXDEC  S32        AILCALL AIL_file_fss_attrib_write
                                            (FSSpec const FAR    *filename,
                                             void const FAR  *buf,
                                             U32       len,
                                             U32       type,
                                             U32       creator );

DXDEC  S32        AILCALL AIL_WAV_file_fss_write
                                           (FSSpec const FAR    *filename,
                                            void const FAR  *buf,
                                            U32       len,
                                            S32       rate,
                                            S32       format);

#endif

#ifdef IS_DOS

extern void * AILCALLBACK (*AIL_mem_alloc) (UINTa);
extern void   AILCALLBACK (*AIL_mem_free)  (void *);

typedef void * (AILCALLBACK *AILMEMALLOCCB)(UINTa size);
typedef void (AILCALLBACK *AILMEMFREECB)(void *);
  
AILMEMALLOCCB cdecl AIL_mem_use_malloc(AILMEMALLOCCB allocf);
AILMEMFREECB cdecl AIL_mem_use_free  (AILMEMFREECB freef);

//
// Other memory-management functions
//

DXDEC  S32   AILCALL AIL_mem_alloc_DOS (U32  n_paras,
                                        void **protected_ptr,
                                        U32 *segment_far_ptr,
                                        U32 *selector);

DXDEC  void  AILCALL AIL_mem_free_DOS (void  *protected_ptr,
                                       U32  segment_far_ptr,
                                       U32  selector);

DXDEC  S32   AILCALL AIL_vmm_lock_range   (void const *p1, void const *p2);
DXDEC  S32   AILCALL AIL_vmm_unlock_range (void const *p1, void const *p2);

DXDEC  S32   AILCALL AIL_vmm_lock         (void const *start, U32 size);
DXDEC  S32   AILCALL AIL_vmm_unlock       (void const *start, U32 size);

DXDEC  U32  AILCALL AIL_sel_base         (U32  sel);

DXDEC  void   AILCALL AIL_sel_set_limit  (U32  sel,
                                          U32  limit);
//
// Last IO_PARMS structure used to attempt device detection
//

extern IO_PARMS       AIL_last_IO_attempt;

//
// Low-level support services
//

DXDEC REALFAR AILCALL  AIL_get_real_vect            (U32       vectnum);

DXDEC void    AILCALL  AIL_set_real_vect            (U32       vectnum,
                                                     REALFAR   real_ptr);

DXDEC void    AILCALL  AIL_set_USE16_ISR            (S32        IRQ,
                                                     REALFAR    real_base,
                                                     U32        ISR_offset);

DXDEC void    AILCALL  AIL_restore_USE16_ISR        (S32        IRQ);

DXDEC U32     AILCALL  AIL_disable_interrupts       (void);
DXDEC void    AILCALL  AIL_restore_interrupts       (U32        FD_register);

DXDEC void    AILCALL  AIL_switch_stack             (void       *stack,
                                                     U32        size,
                                                     U32        *SS,
                                                     void       **ESP,
                                                     void       **EBP);

DXDEC void    AILCALL  AIL_restore_stack            (U32        SS,
                                                     void       *ESP,
                                                     void       *EBP);

DXDEC S32     AILCALL  AIL_call_driver              (AIL_DRIVER *drvr,
                                                     S32        fn,
                                                     VDI_CALL   *in,
                                                     VDI_CALL   *out);

DXDEC S32     AILCALL  AIL_read_INI                 (AIL_INI    *INI,
                                                     char       *filename);

DXDEC U32     AILCALL  AIL_interrupt_divisor        (void);

#endif


#ifdef __WATCOMC__
#pragma warning 14 10      // disable "no reference to symbol" warning on function args
void MSSBreakPoint();
#pragma aux MSSBreakPoint = "int 3";
#elif defined(IS_PS2)
#define MSSBreakPoint() __asm__("break 0")
#elif defined( IS_XENON )
void __twi (const unsigned int to, int a, const int b);
#define MSSBreakPoint() {__twi(31,0,22);}
#elif defined( IS_PS3 )
#define MSSBreakPoint() __asm__ volatile("trap");
#elif defined( IS_WII )
#define MSSBreakPoint() __asm__ volatile("trap");
#elif defined( IS_SPU )
#define MSSBreakPoint() __asm volatile ("stopd 0,1,1");
#elif defined(IS_WIN64)
#define MSSBreakPoint() __debugbreak();
#elif defined(IS_WINDOWS)
#define MSSBreakPoint() __asm {int 3}
#else
#define MSSBreakPoint() *(int*)0=0;
#endif


//
// Compiler-independent CRTL helper functions for PS2
// Exported here for use in demo programs as well as MSS itself
//

#ifdef IS_PS2

DXDEC F32 AILCALL AIL_sin(F32 x);
DXDEC F32 AILCALL AIL_cos(F32 x);
DXDEC F32 AILCALL AIL_tan( F32 x );
DXDEC F32 AILCALL AIL_acos(F32 x);
DXDEC F32 AILCALL AIL_atan(F32 x);
DXDEC F32 AILCALL AIL_ceil( F32 x );
DXDEC F32 AILCALL AIL_floor( F32 x );
DXDEC F32 AILCALL AIL_fsqrt( F32 x );
DXDEC F32 AILCALL AIL_fabs ( F32 x );
DXDEC F32 AILCALL AIL_log10( F32 x );
DXDEC F32 AILCALL AIL_log( F32 x );
DXDEC F32 AILCALL AIL_pow( F32 x, F32 p );
DXDEC F32 AILCALL AIL_frexpf( F32 x, S32 *pw2 );
DXDEC F32 AILCALL AIL_ldexpf( F32 x, S32 pw2 );
#define AIL_exp(x) AIL_pow(2.718281828F,(x)) 

#else
                 
#ifdef IS_WATCOM
#define AIL_pow   powf
#define AIL_tan   tanf
#else
#define AIL_pow   pow
#define AIL_tan   tan
#endif

#define AIL_sin   sin
#define AIL_cos   cos
#define AIL_acos  acos
#define AIL_atan  atan
#define AIL_ceil  ceil
#define AIL_floor floor
#define AIL_fsqrt(arg) ((F32) sqrt(arg))
#define AIL_fabs  fabs
#define AIL_log10 log10
#define AIL_log   log
#define AIL_frexpf(a1,a2) ((F32) frexp(a1,a2))
#define AIL_ldexpf(a1,a2) ((F32) ldexp(a1,a2))
#define AIL_exp   exp

#endif

DXDEC C8 * AILCALL AIL_ftoa(F32 val);

//
// High-level support services
//

#ifdef IS_DOS

#ifdef __SW_3R
extern S32 AILCALL AIL_startup_reg               (void);
#define AIL_startup AIL_startup_reg
#else
extern S32 AILCALL AIL_startup_stack             (void);
#define AIL_startup AIL_startup_stack
#endif

#define AIL_quick_startup(ud,um,opr,opb,ops) AIL_quick_startup_with_start(&AIL_startup,ud,um,opr,opb,ops)

#define AIL_get_preference(number) (AIL_preference[number])

#else

DXDEC  S32     AILCALL  AIL_startup                   (void);

DXDEC  SINTa   AILCALL  AIL_get_preference            (U32         number);

#endif

DXDEC  void    AILCALL  AIL_shutdown                  (void);

DXDEC  SINTa   AILCALL  AIL_set_preference            (U32         number,
                                                       SINTa       value);

DXDEC char FAR *AILCALL  AIL_last_error                (void);

DXDEC  void    AILCALL  AIL_set_error                 (char const FAR * error_msg);

//
// Low-level support services
//

DXDEC  void
#if !defined(IS_MAC) && !defined(IS_LINUX) && !defined(IS_PS2) && !defined(IS_PS3) && !defined(IS_WII)
__cdecl
#endif
AIL_debug_printf              (C8 const FAR *fmt, ...);

#ifdef IS_X86

DXDEC  U32     AILCALL  AIL_MMX_available             (void);

#endif

DXDEC  void    AILCALL  AIL_lock                      (void);
DXDEC  void    AILCALL  AIL_unlock                    (void);

#if defined(IS_WIN32API) || defined(IS_MAC) || defined(IS_LINUX)

DXDEC  void    AILCALL  AIL_lock_mutex                (void);
DXDEC  void    AILCALL  AIL_unlock_mutex              (void);

#endif

DXDEC  void    AILCALL  AIL_delay                     (S32         intervals);

DXDEC  S32     AILCALL  AIL_background                (void);

#ifndef IS_DOS

DXDEC  AILTRACECB AILCALL AIL_register_trace_callback (AILTRACECB cb, 
                                                       S32 level);

#endif

//
// Process services
//

DXDEC  HTIMER  AILCALL  AIL_register_timer            (AILTIMERCB  fn);

DXDEC  UINTa   AILCALL  AIL_set_timer_user            (HTIMER      timer,
                                                       UINTa       user);

DXDEC  void    AILCALL  AIL_set_timer_period          (HTIMER      timer,
                                                       U32         microseconds);

DXDEC  void    AILCALL  AIL_set_timer_frequency       (HTIMER      timer,
                                                       U32         hertz);

DXDEC  void    AILCALL  AIL_set_timer_divisor         (HTIMER      timer,
                                                       U32         PIT_divisor);

DXDEC  void    AILCALL  AIL_start_timer               (HTIMER      timer);
DXDEC  void    AILCALL  AIL_start_all_timers          (void);

DXDEC  void    AILCALL  AIL_stop_timer                (HTIMER      timer);
DXDEC  void    AILCALL  AIL_stop_all_timers           (void);

DXDEC  void    AILCALL  AIL_release_timer_handle      (HTIMER      timer);
DXDEC  void    AILCALL  AIL_release_all_timers        (void);

#ifdef IS_WIN32

/*
#ifndef __RADINDLL__

// static function that handles shutdown
int __cdecl MSS_auto_cleanup(void);

#ifdef _MSC_VER
// on MSVC, automatically register a cleanup function
#define AIL_startup() (MSS_auto_cleanup(),AIL_startup())
#endif

#endif
*/

DXDEC  HWND    AILCALL  AIL_HWND                      (void);

#else
  #ifdef IS_MAC
   #if defined(__PROCESSES__)
    DXDEC ProcessSerialNumber AIL_Process(void);
   #endif
  #endif
#endif

//
// high-level digital services
//

#define AIL_OPEN_DIGITAL_FORCE_PREFERENCE 1
#define AIL_OPEN_DIGITAL_NEED_HW_3D       2
#define AIL_OPEN_DIGITAL_NEED_FULL_3D     4
#define AIL_OPEN_DIGITAL_NEED_LIGHT_3D    8
#define AIL_OPEN_DIGITAL_NEED_HW_REVERB   16
#define AIL_OPEN_DIGITAL_NEED_REVERB      32
#define AIL_OPEN_DIGITAL_USE_IOP_CORE0    64

#define AIL_OPEN_DIGITAL_USE_SPU( num )   ( ( num + 1 ) << 24 )

DXDEC HDIGDRIVER AILCALL AIL_open_digital_driver( U32 frequency,
                                                  S32 bits,
                                                  S32 channel,
                                                  U32 flags );

DXDEC void AILCALL AIL_close_digital_driver( HDIGDRIVER dig );

#ifdef IS_LINUX

#define AIL_MSS_version(str,len)        \
{                                       \
  strncpy(str, MSS_VERSION, len);   \
}

DXDEC  S32          AILCALL AIL_digital_handle_release(HDIGDRIVER drvr);

DXDEC  S32          AILCALL AIL_digital_handle_reacquire
                                                     (HDIGDRIVER drvr);
#else

#ifdef IS_WINDOWS

#define AIL_MSS_version(str,len)        \
{                                       \
  HINSTANCE l=LoadLibrary(MSSDLLNAME);  \
  if ((UINTa)l<=32)                     \
    *(str)=0;                           \
  else {                                \
    LoadString(l,1,str,len);            \
    FreeLibrary(l);                     \
  }                                     \
}

#if 0
DXDEC  S32          AILCALL AIL_waveOutOpen          (HDIGDRIVER   FAR *drvr,
                                                      LPHWAVEOUT   FAR  *lphWaveOut,
                                                      S32             wDeviceID,
                                                      LPWAVEFORMAT      lpFormat);

DXDEC  void         AILCALL AIL_waveOutClose          (HDIGDRIVER drvr);
#endif

DXDEC  S32          AILCALL AIL_digital_handle_release(HDIGDRIVER drvr);

DXDEC  S32          AILCALL AIL_digital_handle_reacquire
                                                     (HDIGDRIVER drvr);

#else

#ifdef IS_MAC

#if defined(__RESOURCES__)

typedef MSS_STRUCT MSS_VersionType_
{
  Str255 version_name;
} MSS_VersionType;

#define AIL_MSS_version(str,len)                        \
{                                                       \
  long _res = HOpenResFile(0,0,"\p" MSSDLLNAME,fsRdPerm);    \
  if (_res==-1)                                         \
  {                                                     \
    str[0]=0;                                           \
  }                                                     \
  else                                                  \
  {                                                     \
    Handle _H;                                          \
    short _Err;                                         \
    long _cur= CurResFile();                            \
    UseResFile(_res);                                   \
    _H = GetResource('vers', 2);                        \
    _Err = ResError();                                  \
    if((_Err != noErr) || (_H==0))                      \
    {                                                   \
      str[0]=0;                                         \
      UseResFile(_cur);                                 \
      CloseResFile(_res);                               \
    }                                                   \
    else                                                \
    {                                                   \
      if (GetHandleSize(_H)==0)                         \
      {                                                 \
        str[0]=0;                                       \
        UseResFile(_cur);                               \
        CloseResFile(_res);                             \
      }                                                 \
      else                                              \
      {                                                 \
        MSS_VersionType * _vt = (MSS_VersionType*)*_H;  \
        if ((U32)_vt->version_name[6]>4)                \
          _vt->version_name[6]-=4;                      \
        else                                            \
          _vt->version_name[6]=0;                       \
        if (((U32)len) <= ((U32)_vt->version_name[6]))  \
          _vt->version_name[6] = (U8)len-1;             \
        memcpy( str, _vt->version_name+11, _vt->version_name[6] ); \
        str[_vt->version_name[6]]=0;                    \
        UseResFile(_cur);                               \
        CloseResFile(_res);                             \
      }                                                 \
      ReleaseResource(_H);                              \
    }                                                   \
  }                                                     \
}

#endif

DXDEC  S32          AILCALL AIL_digital_handle_release(HDIGDRIVER drvr);

DXDEC  S32          AILCALL AIL_digital_handle_reacquire
                                                     (HDIGDRIVER drvr);

#else

#ifdef IS_DOS

//
// DOS installation services
//

DXDEC IO_PARMS *   AILCALL AIL_get_IO_environment    (AIL_DRIVER *drvr);

DXDEC AIL_DRIVER*  AILCALL AIL_install_driver        (U8 const     *driver_image,
                                                      U32       n_bytes);

DXDEC void         AILCALL AIL_uninstall_driver      (AIL_DRIVER *drvr);

DXDEC S32         AILCALL AIL_install_DIG_INI       (HDIGDRIVER *dig);

DXDEC HDIGDRIVER   AILCALL AIL_install_DIG_driver_file
                                                    (char const *filename,
                                                     IO_PARMS   *IO);

DXDEC void         AILCALL AIL_uninstall_DIG_driver  (HDIGDRIVER  dig);


DXDEC HDIGDRIVER   AILCALL AIL_install_DIG_driver_image
                                                    (void const    *driver_image,
                                                     U32     size,
                                                     IO_PARMS *IO);
#endif
#endif
#endif
#endif

DXDEC char FAR*     AILCALL AIL_set_redist_directory(char const FAR*dir);

DXDEC  S32          AILCALL AIL_background_CPU_percent (void);

DXDEC  S32          AILCALL AIL_digital_CPU_percent   (HDIGDRIVER dig);

DXDEC  S32          AILCALL AIL_digital_latency       (HDIGDRIVER dig);

DXDEC  HSAMPLE      AILCALL AIL_allocate_sample_handle
                                                      (HDIGDRIVER dig);

DXDEC void          AILCALL AIL_set_speaker_configuration
                                                      (HDIGDRIVER       dig,
                                                       MSSVECTOR3D FAR *array,
                                                       S32              n_channels,
                                                       F32              falloff_power);

DXDEC MSSVECTOR3D FAR * 
                    AILCALL AIL_speaker_configuration
                                                      (HDIGDRIVER       dig,
                                                       S32         FAR *n_physical_channels,
                                                       S32         FAR *n_logical_channels,
                                                       F32         FAR *falloff_power,
                                                       MSS_MC_SPEC FAR *channel_spec);

DXDEC void          AILCALL AIL_set_listener_relative_receiver_array
                                                      (HDIGDRIVER             dig,
                                                       MSS_RECEIVER_LIST FAR *array,
                                                       S32                    n_receivers);

DXDEC MSS_RECEIVER_LIST FAR * 
                    AILCALL AIL_listener_relative_receiver_array
                                                      (HDIGDRIVER dig,
                                                       S32   FAR *n_receivers);
#ifndef IS_XBOX

DXDEC void         AILCALL AIL_set_speaker_reverb_levels
                                                      (HDIGDRIVER   dig, 
                                                       F32 FAR     *wet_array,
                                                       F32 FAR     *dry_array,
                                                       MSS_SPEAKER *speaker_index_array,
                                                       S32          n_levels);

DXDEC S32          AILCALL AIL_speaker_reverb_levels  (HDIGDRIVER                   dig,
                                                       F32 FAR               * FAR *wet_array,
                                                       F32 FAR               * FAR *dry_array,
                                                       MSS_SPEAKER FAR const * FAR *speaker_index_array);
#endif


DXDEC  
void AILCALL AIL_set_sample_speaker_scale_factors (HSAMPLE                 S, //)
                                                   MSS_SPEAKER FAR const * dest_speaker_indexes,
                                                   F32         FAR const * levels,
                                                   S32                     n_levels );
DXDEC  
void AILCALL AIL_sample_speaker_scale_factors (HSAMPLE                 S, //)
                                               MSS_SPEAKER FAR const * dest_speaker_indexes,
                                               F32         FAR       * levels,
                                               S32                     n_levels );

DXDEC
S32 AILEXPORT AIL_set_sample_is_3D                   (HSAMPLE                S, //)
                                                      S32                    onoff);


DXDEC  
S32   AILEXPORT AIL_calculate_3D_channel_levels      (HDIGDRIVER                   dig, //)
                                                      F32                     FAR *channel_levels,
                                                      MSS_SPEAKER FAR const * FAR *speaker_array,
                                                      MSSVECTOR3D FAR             *src_pos,
                                                      MSSVECTOR3D FAR             *src_face,
                                                      MSSVECTOR3D FAR             *src_up,
                                                      F32                          src_inner_angle,
                                                      F32                          src_outer_angle,
                                                      F32                          src_outer_volume,
                                                      F32                          src_max_dist,
                                                      F32                          src_min_dist,
                                                      MSSVECTOR3D    FAR          *listen_pos, 
                                                      MSSVECTOR3D    FAR          *listen_face,
                                                      MSSVECTOR3D    FAR          *listen_up,  
                                                      F32                          rolloff_factor,
                                                      MSSVECTOR3D    FAR          *doppler_velocity,
                                                      F32            FAR          *doppler_shift);
                                                      

DXDEC  void         AILCALL AIL_release_sample_handle (HSAMPLE S);

DXDEC  S32          AILCALL AIL_init_sample         (HSAMPLE S,
                                                     S32     format);

DXDEC  S32          AILCALL AIL_set_sample_file       (HSAMPLE   S,
                                                       void const FAR *file_image,
                                                       S32       block);

DXDEC  S32          AILCALL AIL_set_sample_info       (HSAMPLE   S,
                                                       AILSOUNDINFO const FAR * info);

DXDEC  S32          AILCALL AIL_set_named_sample_file (HSAMPLE   S,
                                                       C8 const   FAR *file_type_suffix,
                                                       void const FAR *file_image,
                                                       U32       file_size,
                                                       S32       block);

DXDEC  HPROVIDER    AILCALL AIL_set_sample_processor  (HSAMPLE     S,
                                                       SAMPLESTAGE pipeline_stage,
                                                       HPROVIDER   provider);

DXDEC  HPROVIDER    AILCALL AIL_set_digital_driver_processor
                                                      (HDIGDRIVER  dig,
                                                       DIGDRVSTAGE pipeline_stage,
                                                       HPROVIDER   provider);

DXDEC  HPROVIDER    AILCALL AIL_sample_processor      (HSAMPLE     S,
                                                       SAMPLESTAGE pipeline_stage);

DXDEC  HPROVIDER    AILCALL AIL_digital_driver_processor
                                                      (HDIGDRIVER  dig,
                                                       DIGDRVSTAGE pipeline_stage);

DXDEC  void         AILCALL AIL_set_sample_adpcm_block_size
                                                     (HSAMPLE S,
                                                     U32     blocksize);

DXDEC  void         AILCALL AIL_set_sample_address    (HSAMPLE S,
                                                     void const   FAR *start,
                                                     U32     len);

DXDEC  void         AILCALL AIL_start_sample          (HSAMPLE S);

DXDEC  void         AILCALL AIL_stop_sample           (HSAMPLE S);

DXDEC  void         AILCALL AIL_resume_sample         (HSAMPLE S);

DXDEC  void         AILCALL AIL_end_sample            (HSAMPLE S);

DXDEC  void         AILCALL AIL_set_sample_playback_rate
                                                     (HSAMPLE S,
                                                     S32     playback_rate);

DXDEC  void         AILCALL AIL_set_sample_volume_pan (HSAMPLE S,
                                                       F32     volume,
                                                       F32     pan);

DXDEC  void         AILCALL AIL_set_sample_volume_levels(HSAMPLE S,
                                                         F32     left_level,
                                                         F32     right_level);

DXDEC  void         AILCALL AIL_set_sample_channel_levels (HSAMPLE        S,
                                                           MSS_SPEAKER FAR const *source_speaker_indexes,
                                                           MSS_SPEAKER FAR const *dest_speaker_indexes,
                                                           F32 FAR         const *levels,
                                                           S32                    n_levels);

DXDEC  void         AILCALL AIL_set_sample_reverb_levels(HSAMPLE S,
                                                         F32     dry_level,
                                                         F32     wet_level);

DXDEC  void         AILCALL AIL_set_sample_low_pass_cut_off(HSAMPLE S,
                                                            S32 /*-1 or MSS_SPEAKER*/ channel,
                                                            F32         cut_off);

DXDEC  void         AILCALL AIL_set_sample_loop_count (HSAMPLE S,
                                                       S32     loop_count);

DXDEC  void         AILCALL AIL_set_sample_loop_block (HSAMPLE S,
                                                       S32     loop_start_offset,
                                                       S32     loop_end_offset);

DXDEC  S32          AILCALL AIL_sample_loop_block     (HSAMPLE S,
                                                       S32    *loop_start_offset,
                                                       S32    *loop_end_offset);

DXDEC  U32          AILCALL AIL_sample_status         (HSAMPLE S);

DXDEC  S32          AILCALL AIL_sample_playback_rate  (HSAMPLE S);

DXDEC  void         AILCALL AIL_sample_volume_pan     (HSAMPLE S, F32 FAR* volume, F32 FAR* pan);

DXDEC  S32          AILCALL AIL_sample_channel_count  (HSAMPLE S, U32 *mask);

DXDEC  void         AILCALL AIL_sample_channel_levels (HSAMPLE                S,
                                                       MSS_SPEAKER FAR const *source_speaker_indexes,
                                                       MSS_SPEAKER FAR const *dest_speaker_indexes,
                                                       F32 FAR               *levels,
                                                       S32                    n_levels);

DXDEC  void         AILCALL AIL_sample_volume_levels  (HSAMPLE  S,
                                                       F32 FAR *left_level,
                                                       F32 FAR *right_level);

DXDEC  void         AILCALL AIL_sample_reverb_levels  (HSAMPLE  S,
                                                       F32 FAR *dry_level,
                                                       F32 FAR *wet_level);

DXDEC  void         AILCALL AIL_sample_output_levels  (HSAMPLE                S,
                                                       MSS_SPEAKER FAR const *source_speaker_indexes,
                                                       MSS_SPEAKER FAR const *dest_speaker_indexes,
                                                       F32 FAR               *levels,
                                                       S32                    n_levels);

DXDEC  F32          AILCALL AIL_sample_low_pass_cut_off(HSAMPLE S, S32 /*-1 or MSS_SPEAKER*/ channel);

DXDEC  S32          AILCALL AIL_sample_loop_count     (HSAMPLE S);

DXDEC  void         AILCALL AIL_set_digital_master_volume_level
                                                     (HDIGDRIVER dig,
                                                      F32        master_volume);

DXDEC  F32          AILCALL AIL_digital_master_volume_level (HDIGDRIVER dig);

DXDEC  void         AILCALL AIL_set_sample_51_volume_pan( HSAMPLE S,
                                                          F32     volume,
                                                          F32     pan,
                                                          F32     fb_pan,
                                                          F32     center_level,
                                                          F32     sub_level );

DXDEC  void         AILCALL AIL_sample_51_volume_pan    ( HSAMPLE S,
                                                          F32 FAR* volume,
                                                          F32 FAR* pan,
                                                          F32 FAR* fb_pan,
                                                          F32 FAR* center_level,
                                                          F32 FAR* sub_level );

DXDEC  void         AILCALL AIL_set_sample_51_volume_levels( HSAMPLE S,
                                                             F32     f_left_level,
                                                             F32     f_right_level,
                                                             F32     b_left_level,
                                                             F32     b_right_level,
                                                             F32     center_level,
                                                             F32     sub_level );

DXDEC  void         AILCALL AIL_sample_51_volume_levels    ( HSAMPLE S,
                                                             F32 FAR* f_left_level,
                                                             F32 FAR* f_right_level,
                                                             F32 FAR* b_left_level,
                                                             F32 FAR* b_right_level,
                                                             F32 FAR* center_level,
                                                             F32 FAR* sub_level );
#if !defined(IS_XBOX)

DXDEC  void         AILCALL AIL_set_digital_master_reverb
                                                     (HDIGDRIVER dig,
                                                      F32        reverb_decay_time,
                                                      F32        reverb_predelay,
                                                      F32        reverb_damping);

DXDEC  void         AILCALL AIL_digital_master_reverb
                                                     (HDIGDRIVER dig,
                                                      F32 FAR*   reverb_time,
                                                      F32 FAR*   reverb_predelay,
                                                      F32 FAR*   reverb_damping);

#endif

DXDEC  void         AILCALL AIL_set_digital_master_reverb_levels
                                                     (HDIGDRIVER dig,
                                                      F32        dry_level,
                                                      F32        wet_level);

DXDEC  void         AILCALL AIL_digital_master_reverb_levels
                                                     (HDIGDRIVER dig,
                                                      F32 FAR *  dry_level,
                                                      F32 FAR *  wet_level);


//
// low-level digital services
//

DXDEC  S32      AILCALL AIL_minimum_sample_buffer_size(HDIGDRIVER dig,
                                                     S32         playback_rate,
                                                     S32         format);

DXDEC  S32      AILCALL AIL_set_sample_buffer_count  (HSAMPLE S,
                                                      S32     n_buffers);

DXDEC  S32      AILCALL AIL_sample_buffer_count      (HSAMPLE S);

DXDEC  S32      AILCALL AIL_sample_buffer_available (HSAMPLE S);

DXDEC  S32      AILCALL AIL_load_sample_buffer      (HSAMPLE S,
                                                     S32     buff_num,
                                                     void const FAR *buffer,
                                                     U32     len);

DXDEC  void     AILCALL AIL_request_EOB_ASI_reset   (HSAMPLE S,
                                                     U32     buff_num,
                                                     S32     new_stream_position);

DXDEC  S32      AILCALL AIL_sample_buffer_info      (HSAMPLE S, //)
                                                     S32     buff_num,
                                                     U32     FAR *pos,
                                                     U32     FAR *len,
                                                     S32     FAR *head,
                                                     S32     FAR *tail);

DXDEC  U32      AILCALL AIL_sample_granularity      (HSAMPLE S);

DXDEC  void     AILCALL AIL_set_sample_position      (HSAMPLE S,
                                                      U32     pos);

DXDEC  U32      AILCALL AIL_sample_position          (HSAMPLE S);

DXDEC  AILSAMPLECB AILCALL AIL_register_SOB_callback
                                                    (HSAMPLE S,
                                                     AILSAMPLECB SOB);

DXDEC  AILSAMPLECB AILCALL AIL_register_EOB_callback
                                                    (HSAMPLE S,
                                                     AILSAMPLECB EOB);

DXDEC  AILSAMPLECB AILCALL AIL_register_EOS_callback
                                                    (HSAMPLE S,
                                                     AILSAMPLECB EOS);

DXDEC  AILFALLOFFCB AILCALL AIL_register_falloff_function_callback
                                                    (HSAMPLE S,
                                                     AILFALLOFFCB falloff_cb);

DXDEC  void     AILCALL AIL_set_sample_user_data   (HSAMPLE S,
                                                    U32     index,
                                                    SINTa   value);

DXDEC  SINTa    AILCALL AIL_sample_user_data       (HSAMPLE S,
                                                    U32     index);

DXDEC  S32      AILCALL AIL_active_sample_count    (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_digital_configuration  (HDIGDRIVER dig,
                                                      S32   FAR *rate,
                                                      S32   FAR *format,
                                                      char  FAR *string);

DXDEC S32     AILCALL AIL_platform_property (void                 *object,
                                       MSS_PLATFORM_PROPERTY property,
                                       void FAR             *before_value,
                                       void const FAR       *new_value,
                                       void FAR             *after_value);

#ifdef IS_WIN32API

DXDEC  S32      AILCALL AIL_set_direct_buffer_control (HSAMPLE S,
                                                     U32      command);

DXDEC  void     AILCALL AIL_get_DirectSound_info  (HSAMPLE              S,
                                                   AILLPDIRECTSOUND    *lplpDS,
                                                   AILLPDIRECTSOUNDBUFFER *lplpDSB);

#ifdef IS_WIN32
DXDEC  S32      AILCALL AIL_set_DirectSound_HWND(HDIGDRIVER dig, HWND wnd);
#endif 

#endif

DXDEC  void     AILCALL AIL_set_sample_ms_position (HSAMPLE    S, //)
                                                    S32        milliseconds);

DXDEC  void     AILCALL AIL_sample_ms_position     (HSAMPLE    S, //)
                                                    S32 FAR *  total_milliseconds,
                                                    S32 FAR *  current_milliseconds);

//
// Digital input services
//

#if defined(IS_WINDOWS) || defined (IS_MAC) || defined(IS_LINUX) || defined(IS_XBOX) || defined(IS_XENON)

DXDEC HDIGINPUT AILCALL AIL_open_input             (AIL_INPUT_INFO FAR *info);

DXDEC void      AILCALL AIL_close_input            (HDIGINPUT         dig);

DXDEC AIL_INPUT_INFO FAR *
                AILCALL AIL_get_input_info         (HDIGINPUT         dig);

DXDEC S32       AILCALL AIL_set_input_state        (HDIGINPUT         dig,
                                                    S32               enable);
#endif


//
// High-level XMIDI services
//

DXDEC HMDIDRIVER AILCALL AIL_open_XMIDI_driver( U32 flags );

#define AIL_OPEN_XMIDI_NULL_DRIVER 1

DXDEC void AILCALL AIL_close_XMIDI_driver( HMDIDRIVER mdi );

#if defined(IS_MAC) || defined(IS_LINUX)

DXDEC  S32          AILCALL AIL_MIDI_handle_release
                                                 (HMDIDRIVER mdi);

DXDEC  S32          AILCALL AIL_MIDI_handle_reacquire
                                                 (HMDIDRIVER mdi);

#else

#ifdef IS_WINDOWS

DXDEC  S32          AILCALL AIL_midiOutOpen(HMDIDRIVER FAR *drvr,
                                            LPHMIDIOUT FAR *lphMidiOut,
                                            S32             dwDeviceID);

DXDEC  void         AILCALL AIL_midiOutClose      (HMDIDRIVER mdi);

DXDEC  S32          AILCALL AIL_MIDI_handle_release
                                                 (HMDIDRIVER mdi);

DXDEC  S32          AILCALL AIL_MIDI_handle_reacquire
                                                 (HMDIDRIVER mdi);

#else

#ifdef IS_DOS

DXDEC S32         AILCALL AIL_install_MDI_INI       (HMDIDRIVER *mdi);

DXDEC HMDIDRIVER   AILCALL AIL_install_MDI_driver_file
                                                    (char const *filename,
                                                     IO_PARMS   *IO);

DXDEC void         AILCALL AIL_uninstall_MDI_driver  (HMDIDRIVER  mdi);


DXDEC HMDIDRIVER   AILCALL AIL_install_MDI_driver_image
                                                    (void const *driver_image,
                                                     U32       size,
                                                     IO_PARMS   *IO);

DXDEC S32     AILCALL AIL_MDI_driver_type           (HMDIDRIVER  mdi);

DXDEC void    AILCALL AIL_set_GTL_filename_prefix   (char const*prefix);

DXDEC S32     AILCALL AIL_timbre_status             (HMDIDRIVER  mdi,
                                                     S32        bank,
                                                     S32        patch);

DXDEC S32     AILCALL AIL_install_timbre            (HMDIDRIVER  mdi,
                                                     S32        bank,
                                                     S32        patch);

DXDEC void    AILCALL AIL_protect_timbre            (HMDIDRIVER  mdi,
                                                     S32        bank,
                                                     S32        patch);

DXDEC void    AILCALL AIL_unprotect_timbre          (HMDIDRIVER  mdi,
                                                     S32        bank,
                                                     S32        patch);

#endif

#endif

#endif

DXDEC  HSEQUENCE    AILCALL AIL_allocate_sequence_handle
                                                     (HMDIDRIVER mdi);

DXDEC  void         AILCALL AIL_release_sequence_handle
                                                     (HSEQUENCE S);

DXDEC  S32          AILCALL AIL_init_sequence         (HSEQUENCE S,
                                                     void const     FAR *start,
                                                     S32       sequence_num);

DXDEC  void         AILCALL AIL_start_sequence        (HSEQUENCE S);

DXDEC  void         AILCALL AIL_stop_sequence         (HSEQUENCE S);

DXDEC  void         AILCALL AIL_resume_sequence       (HSEQUENCE S);

DXDEC  void         AILCALL AIL_end_sequence          (HSEQUENCE S);

DXDEC  void         AILCALL AIL_set_sequence_tempo    (HSEQUENCE S,
                                                       S32       tempo,
                                                       S32       milliseconds);

DXDEC  void         AILCALL AIL_set_sequence_volume   (HSEQUENCE S,
                                                       S32       volume,
                                                       S32       milliseconds);

DXDEC  void         AILCALL AIL_set_sequence_loop_count
                                                     (HSEQUENCE S,
                                                      S32       loop_count);

DXDEC  U32          AILCALL AIL_sequence_status       (HSEQUENCE S);

DXDEC  S32          AILCALL AIL_sequence_tempo        (HSEQUENCE S);

DXDEC  S32          AILCALL AIL_sequence_volume       (HSEQUENCE S);

DXDEC  S32          AILCALL AIL_sequence_loop_count   (HSEQUENCE S);

DXDEC  void         AILCALL AIL_set_XMIDI_master_volume
                                                     (HMDIDRIVER mdi,
                                                      S32         master_volume);

DXDEC  S32          AILCALL AIL_XMIDI_master_volume   (HMDIDRIVER mdi);


//
// Low-level XMIDI services
//

DXDEC  S32      AILCALL AIL_active_sequence_count     (HMDIDRIVER mdi);

DXDEC  S32      AILCALL AIL_controller_value          (HSEQUENCE S,
                                                      S32       channel,
                                                      S32       controller_num);

DXDEC  S32      AILCALL AIL_channel_notes             (HSEQUENCE S,
                                                      S32       channel);

DXDEC  void     AILCALL AIL_sequence_position         (HSEQUENCE S,
                                                      S32      FAR *beat,
                                                      S32      FAR *measure);

DXDEC  void     AILCALL AIL_branch_index              (HSEQUENCE  S,
                                                      U32        marker);

DXDEC  AILPREFIXCB AILCALL AIL_register_prefix_callback
                                                     (HSEQUENCE  S,
                                                      AILPREFIXCB callback);

DXDEC  AILTRIGGERCB AILCALL AIL_register_trigger_callback
                                                     (HSEQUENCE  S,
                                                      AILTRIGGERCB callback);

DXDEC  AILSEQUENCECB AILCALL AIL_register_sequence_callback
                                                     (HSEQUENCE  S,
                                                      AILSEQUENCECB callback);

DXDEC  AILBEATCB AILCALL AIL_register_beat_callback   (HSEQUENCE  S,
                                                      AILBEATCB callback);

DXDEC  AILEVENTCB AILCALL AIL_register_event_callback (HMDIDRIVER mdi,
                                                      AILEVENTCB callback);

DXDEC  AILTIMBRECB AILCALL AIL_register_timbre_callback
                                                     (HMDIDRIVER mdi,
                                                      AILTIMBRECB callback);

DXDEC  void     AILCALL AIL_set_sequence_user_data    (HSEQUENCE S,
                                                       U32       index,
                                                       SINTa     value);

DXDEC  SINTa    AILCALL AIL_sequence_user_data        (HSEQUENCE S,
                                                       U32       index);

DXDEC  void     AILCALL AIL_register_ICA_array        (HSEQUENCE S,
                                                      U8       FAR *array);

DXDEC  S32      AILCALL AIL_lock_channel              (HMDIDRIVER mdi);

DXDEC  void     AILCALL AIL_release_channel           (HMDIDRIVER mdi,
                                                      S32         channel);

DXDEC  void     AILCALL AIL_map_sequence_channel      (HSEQUENCE S,
                                                      S32       seq_channel,
                                                      S32       new_channel);

DXDEC  S32      AILCALL AIL_true_sequence_channel     (HSEQUENCE S,
                                                      S32       seq_channel);

DXDEC  void     AILCALL AIL_send_channel_voice_message
                                                     (HMDIDRIVER  mdi,
                                                      HSEQUENCE   S,
                                                      S32         status,
                                                      S32         data_1,
                                                      S32         data_2);

DXDEC  void     AILCALL AIL_send_sysex_message        (HMDIDRIVER mdi,
                                                       void const       FAR *buffer);

DXDEC  HWAVESYNTH
                AILCALL AIL_create_wave_synthesizer   (HDIGDRIVER dig,
                                                    HMDIDRIVER mdi,
                                                       void const       FAR *wave_lib,
                                                       S32         polyphony);

DXDEC  void     AILCALL AIL_destroy_wave_synthesizer  (HWAVESYNTH W);

DXDEC  void     AILCALL AIL_set_sequence_ms_position  (HSEQUENCE S, //)
                                                       S32       milliseconds);

DXDEC  void     AILCALL AIL_sequence_ms_position(HSEQUENCE S, //)
                                                 S32 FAR *total_milliseconds,
                                                 S32 FAR *current_milliseconds);



//
// red book functions
//

#ifdef IS_LINUX
MSS_STRUCT SDL_CD;
#endif

#if !defined(IS_XBOX) && !defined(IS_XENON) && !defined(IS_PS2) && !defined(IS_PS3)  && !defined(IS_WII)

#ifdef IS_DOS
typedef MSS_STRUCT _REDBOOKTRACKINFO {
  U32 tracks;
  U32 trackstarts[100];
} REDBOOKTRACKINFO;
#endif

typedef MSS_STRUCT _REDBOOK {
  U32 DeviceID;
  U32 paused;
  U32 pausedsec;
  U32 lastendsec;
#ifdef IS_DOS
  U32 readcontents;
  REDBOOKTRACKINFO info;
#endif
#ifdef IS_MAC
  short vDRefNum;
#endif
#ifdef IS_LINUX
  struct SDL_CD *cdrom;
#endif
} REDBOOK;

typedef MSS_STRUCT _REDBOOK FAR* HREDBOOK;

#define REDBOOK_ERROR    0
#define REDBOOK_PLAYING  1
#define REDBOOK_PAUSED   2
#define REDBOOK_STOPPED  3


DXDEC  HREDBOOK   AILCALL AIL_redbook_open(U32 which);

#ifdef IS_MAC
DXDEC  HREDBOOK   AILCALL AIL_redbook_open_volume(char const * drive);
#else
DXDEC  HREDBOOK   AILCALL AIL_redbook_open_drive(S32 drive);
#endif

DXDEC  void       AILCALL AIL_redbook_close(HREDBOOK hand);

DXDEC  void       AILCALL AIL_redbook_eject(HREDBOOK hand);

DXDEC  void       AILCALL AIL_redbook_retract(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_status(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_tracks(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_track(HREDBOOK hand);

DXDEC  void       AILCALL AIL_redbook_track_info(HREDBOOK hand,U32 tracknum,
                                                     U32 FAR* startmsec,U32 FAR* endmsec);

DXDEC  U32        AILCALL AIL_redbook_id(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_position(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_play(HREDBOOK hand,U32 startmsec, U32 endmsec);

DXDEC  U32        AILCALL AIL_redbook_stop(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_pause(HREDBOOK hand);

DXDEC  U32        AILCALL AIL_redbook_resume(HREDBOOK hand);

DXDEC  F32        AILCALL AIL_redbook_volume_level(HREDBOOK hand);

DXDEC  F32        AILCALL AIL_redbook_set_volume_level(HREDBOOK hand, F32 volume);

#endif

#ifdef IS_WIN16
  #define AIL_ms_count timeGetTime
  DXDEC U32 AILCALL AIL_us_count(void);
#else
  DXDEC U32 AILCALL AIL_ms_count(void);
  DXDEC U32 AILCALL AIL_us_count(void);
#endif


typedef struct _STREAM FAR* HSTREAM;           // Handle to stream

typedef void (AILCALLBACK FAR* AILSTREAMCB)   (HSTREAM stream);

typedef struct _STREAM {

  S32 block_oriented; // 1 if this is an ADPCM or ASI-compressed stream
  S32 using_ASI;      // 1 if using ASI decoder to uncompress stream data
  ASISTAGE FAR *ASI;  // handy pointer to our ASI coded

  HSAMPLE samp;       // the sample handle

  UINTa fileh;        // the open file handle

  U8 FAR* bufs[3];    // the data buffers
  U32 bufsizes[3];    // the size of each buffer
  S32 reset_ASI[3];   // should we reset the ASI at the end of the buffer?
  S32 reset_seek_pos[3]; // new stream position after reset
  S32 bufstart[3];    // offset of where this buffer started
  void FAR* asyncs[3];// async read structures

  S32 loadedbufstart[2]; // offset of where the loaded buffer started
  S32 loadedorder[2]; // order of the buffers as they were loaded
  S32 loadorder;      // incremented as each buffer is loaded

  S32 bufsize;        // size of each buffer
  S32 readsize;       // size of each read block

  U32 buf1;           // 0,1,2 (current buffer that we are reading into)
  S32 size1;          // holds the current amount of data read

  U32 buf2;           // 0,1,2 (the next buffer that we are reading into)
  S32 size2;          // next buffer loaded up to

  U32 buf3;           // 0,1,2 (the next buffer that we are reading into)
  S32 size3;          // next buffer loaded up to

  U32 datarate;       // datarate in bytes per second
  S32 filerate;       // original datarate of the file
  S32 filetype;       // file format type
  U32 filemask;       // channel mask for stream file
  S32 totallen;       // total length of the sound data

  S32 substart;       // subblock loop start
  S32 sublen;         // subblock loop len
  S32 subpadding;     // amount to pad the final block

  U32 blocksize;      // ADPCM block size
  S32 padding;        // padding to be done
  S32 padded;         // padding done

  S32 loadedsome;     // have we done any loads?

  U32 startpos;       // point that the sound data begins
  U32 totalread;      // total bytes read from the disk

  U32 loopsleft;      // how many loops are left

  U32 error;          // read error has occurred

  S32 preload;        // preload the file into the first buffer
  U32 preloadpos;     // position to use in preload
  S32 noback;         // no background processing
  S32 alldone;        // alldone
  S32 primeamount;    // amount to load after a seek
  S32 readatleast;    // forced amount to read on next service

  S32 playcontrol;    // control: 0=stopped, 1=started, |8=paused, |16=sample paused

  AILSTREAMCB callback;  // end of stream callback

  SINTa user_data[8];   // Miscellaneous user data
  void FAR* next;       // pointer to next stream

#if defined(IS_WINDOWS) || defined(IS_MAC) || defined(IS_LINUX) || defined(IS_XBOX) || defined(IS_XENON) || defined(IS_PS2) || defined(IS_PS3) || defined(IS_WII)
  S32 autostreaming;  // are we autostreaming this stream
#endif

  S32 docallback;     // set when it time to poll for a callback
} MSTREAM_TYPE;


DXDEC HSTREAM AILCALL AIL_open_stream(HDIGDRIVER dig, char const FAR * filename, S32 stream_mem);

DXDEC void AILCALL AIL_close_stream(HSTREAM stream);

DXDEC HSAMPLE  AILCALL AIL_stream_sample_handle(HSTREAM stream);

DXDEC S32 AILCALL AIL_service_stream(HSTREAM stream, S32 fillup);

DXDEC void AILCALL AIL_start_stream(HSTREAM stream);

DXDEC void AILCALL AIL_pause_stream(HSTREAM stream, S32 onoff);

DXDEC S32 AILCALL AIL_stream_loop_count(HSTREAM stream);

DXDEC void AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count);

DXDEC void AILCALL AIL_set_stream_loop_block (HSTREAM S,
                                              S32     loop_start_offset,
                                              S32     loop_end_offset);

DXDEC S32 AILCALL AIL_stream_status(HSTREAM stream);

DXDEC void AILCALL AIL_set_stream_position(HSTREAM stream,S32 offset);

DXDEC S32 AILCALL AIL_stream_position(HSTREAM stream);

DXDEC void AILCALL AIL_stream_info(HSTREAM stream, S32 FAR* datarate, S32 FAR* sndtype, S32 FAR* length, S32 FAR* memory);

DXDEC AILSTREAMCB AILCALL AIL_register_stream_callback(HSTREAM stream, AILSTREAMCB callback);

DXDEC void AILCALL AIL_auto_service_stream(HSTREAM stream, S32 onoff);

DXDEC void     AILCALL AIL_set_stream_user_data   (HSTREAM S,
                                                   U32     index,
                                                   SINTa   value);

DXDEC SINTa    AILCALL AIL_stream_user_data       (HSTREAM S,
                                                   U32     index);

DXDEC  void     AILCALL AIL_set_stream_ms_position   (HSTREAM S,
                                                      S32        milliseconds);

DXDEC  void     AILCALL AIL_stream_ms_position     (HSTREAM    S, //)
                                                    S32 FAR *  total_milliseconds,
                                                    S32 FAR *  current_milliseconds);

#ifdef ON_MAC_USE_FSS

typedef struct MSS_FILE
{
  S32 file_type; // 0 = char*, 1 = FSSpec*
  void const FAR* file;
} MSS_FILE;

#else

typedef char MSS_FILE;

#endif

typedef U32  (AILCALLBACK FAR*AIL_file_open_callback)  (MSS_FILE const FAR* Filename,
                                                        UINTa FAR* FileHandle);

typedef void (AILCALLBACK FAR*AIL_file_close_callback) (UINTa FileHandle);

#define AIL_FILE_SEEK_BEGIN   0
#define AIL_FILE_SEEK_CURRENT 1
#define AIL_FILE_SEEK_END     2

typedef S32  (AILCALLBACK FAR*AIL_file_seek_callback)  (UINTa FileHandle,
                                                        S32 Offset,
                                                        U32 Type);

typedef U32  (AILCALLBACK FAR*AIL_file_read_callback)  (UINTa FileHandle,
                                                        void FAR* Buffer,
                                                        U32 Bytes);

DXDEC  void  AILCALL AIL_set_file_callbacks  (AIL_file_open_callback opencb,
                                              AIL_file_close_callback closecb,
                                              AIL_file_seek_callback seekcb,
                                              AIL_file_read_callback readcb);

#ifdef IS_32

typedef void FAR* (AILCALLBACK FAR *AIL_file_async_read_callback) (UINTa FileHandle,
                                                                   void FAR* Buffer,
                                                                   U32 Bytes);

typedef S32 (AILCALLBACK FAR*AIL_file_async_status_callback)  (void FAR* async,
                                                               S32 wait,
                                                               U32 FAR* BytesRead);

DXDEC  void  AILCALL AIL_set_file_async_callbacks (AIL_file_open_callback opencb,
                                                   AIL_file_close_callback closecb,
                                                   AIL_file_seek_callback seekcb,
                                                   AIL_file_async_read_callback areadcb,
                                                   AIL_file_async_status_callback statuscb);

#endif

//
// High-level DLS functions
//

typedef struct _DLSFILEID {
  SINTa id;
  struct _DLSFILEID FAR* next;
} DLSFILEID;

typedef struct _DLSFILEID FAR* HDLSFILEID;

typedef struct _DLSDEVICE {
  VOIDFUNC FAR* pGetPref;
  VOIDFUNC FAR* pSetPref;
  VOIDFUNC FAR* pMSSOpen;
  VOIDFUNC FAR* pOpen;
  VOIDFUNC FAR* pClose;
  VOIDFUNC FAR* pLoadFile;
  VOIDFUNC FAR* pLoadMem;
  VOIDFUNC FAR* pUnloadFile;
  VOIDFUNC FAR* pUnloadAll;
  VOIDFUNC FAR* pGetInfo;
  VOIDFUNC FAR* pCompact;
  VOIDFUNC FAR* pSetAttr;
  SINTa DLSHandle;
  U32 format;
  U32 buffer_size;
  void FAR* buffer[2];
  HSAMPLE sample;
  HMDIDRIVER mdi;
  HDIGDRIVER dig;
  HDLSFILEID first;
#ifdef IS_WINDOWS
  HMODULE lib;
#else
  #ifdef IS_STATIC
  char FAR* DOSname;
  #endif
#endif
} DLSDEVICE;

typedef struct _DLSDEVICE FAR* HDLSDEVICE;

typedef struct _AILDLSINFO {
  char Description[128];
  S32 MaxDLSMemory;
  S32 CurrentDLSMemory;
  S32 LargestSize;
  S32 GMAvailable;
  S32 GMBankSize;
} AILDLSINFO;

#ifdef IS_STATIC

typedef struct _AILSTATICDLS {
  char FAR* description;
  VOIDFUNC FAR* pDLSOpen;
  VOIDFUNC FAR* pMSSOpen;
  VOIDFUNC FAR* pOpen;
  VOIDFUNC FAR* pClose;
  VOIDFUNC FAR* pLoadFile;
  VOIDFUNC FAR* pLoadMem;
  VOIDFUNC FAR* pUnloadFile;
  VOIDFUNC FAR* pUnloadAll;
  VOIDFUNC FAR* pGetInfo;
  VOIDFUNC FAR* pCompact;
  VOIDFUNC FAR* pSetAttr;
} AILSTATICDLS;

#endif


DXDEC  HDLSDEVICE AILCALL AIL_DLS_open(HMDIDRIVER mdi, HDIGDRIVER dig,
#ifdef IS_STATIC
                                          AILSTATICDLS const FAR * staticdls,
#else
                                          char const FAR * libname,
#endif
                                          U32 flags, U32 rate, S32 bits, S32 channels);

//
// Parameters for the dwFlag used in DLSClose() and flags in AIL_DLS_close
//

#define RETAIN_DLS_COLLECTION   0x00000001
#define RETURN_TO_BOOTUP_STATE  0x00000002
#define RETURN_TO_GM_ONLY_STATE 0x00000004
#define DLS_COMPACT_MEMORY      0x00000008

DXDEC  void   AILCALL AIL_DLS_close(HDLSDEVICE dls, U32 flags);

DXDEC  HDLSFILEID AILCALL AIL_DLS_load_file(HDLSDEVICE dls, char const FAR* filename, U32 flags);

DXDEC  HDLSFILEID AILCALL AIL_DLS_load_memory(HDLSDEVICE dls, void const FAR* memfile, U32 flags);

//
// other parameters for AIL_DLS_unload
//

#define AIL_DLS_UNLOAD_MINE 0
#define AIL_DLS_UNLOAD_ALL  ((HDLSFILEID)(UINTa)(SINTa)-1)

DXDEC  void   AILCALL AIL_DLS_unload(HDLSDEVICE dls, HDLSFILEID dlsid);

DXDEC  void   AILCALL AIL_DLS_compact(HDLSDEVICE dls);

DXDEC  void   AILCALL AIL_DLS_get_info(HDLSDEVICE dls, AILDLSINFO FAR* info, S32 FAR* PercentCPU);

DXDEC HSAMPLE AILCALL AIL_DLS_sample_handle(HDLSDEVICE dls);

//
// Bankfile management
//

typedef struct _SOUNDLIB
{
   struct _SOUNDLIB *next;

   struct _BANKFILE *B;
}
SOUNDLIB;

typedef struct _SOUNDLIB FAR *HSOUNDLIB;

DXDEC void AILEXPORT AIL_close_library(HSOUNDLIB L);

DXDEC HSOUNDLIB AILEXPORT AIL_open_library(const C8 *filename,          
                                           S32       embedded_at_offset);

DXDEC C8 * AILCALL AIL_library_resource_filename(HSOUNDLIB     lib,  
                                                 C8 const FAR *resource_name,
                                                 C8           *filename_buffer,
                                                 S32           filename_buffer_bytes);

//
// Quick-integration service functions and data types
//

typedef struct
{
   U32 const FAR *data;
   S32  size;
   S32  type;
   void FAR *handle;
   S32  status;
   void FAR* next;
   S32  speed;
   F32  volume;
   F32  extravol;
   F32  dry;
   F32  wet;
   F32  cutoff;
   HDLSFILEID dlsid;
   void FAR* dlsmem;
   void FAR* dlsmemunc;
   S32  milliseconds;
   S32  length;
   SINTa userdata;
}
AUDIO_TYPE;


#define QSTAT_DONE          1       // Data has finished playing
#define QSTAT_LOADED        2       // Data has been loaded, but not yet played
#define QSTAT_PLAYING       3       // Data is currently playing

typedef AUDIO_TYPE FAR * HAUDIO;        // Generic handle to any audio data type

#define AIL_QUICK_USE_WAVEOUT             2
#define AIL_QUICK_MIDI_AND_DLS            2
#define AIL_QUICK_DLS_ONLY                3
#define AIL_QUICK_MIDI_AND_VORTEX_DLS     4
#define AIL_QUICK_MIDI_AND_SONICVIBES_DLS 5

DXDEC S32     AILCALL
#if defined(IS_WINDOWS) || defined(IS_MAC) || defined(IS_LINUX) || defined(IS_XBOX) || defined(IS_XENON) || defined(IS_PS2) || defined(IS_PS3) || defined(IS_WII)
                       AIL_quick_startup           (
#else
                       AIL_quick_startup_with_start(void*       startup,
#endif
                                                    S32         use_digital,
                                                    S32         use_MIDI,
                                                    U32         output_rate,
                                                    S32         output_bits,
                                                    S32         output_channels);

DXDEC void    AILCALL AIL_quick_shutdown            (void);

DXDEC void    AILCALL AIL_quick_handles             (HDIGDRIVER FAR* pdig,
                                                     HMDIDRIVER FAR* pmdi,
                                                     HDLSDEVICE FAR* pdls );

DXDEC HAUDIO  AILCALL AIL_quick_load                (char const   FAR *filename);

#ifdef ON_MAC_USE_FSS
DXDEC HAUDIO  AILCALL AIL_quick_fss_load            (FSSpec const   FAR *filename);
#endif

DXDEC HAUDIO  AILCALL AIL_quick_load_mem            (void const   FAR *mem,
                                                     U32    size);

DXDEC HAUDIO  AILCALL AIL_quick_load_named_mem      (void const   FAR *mem,
                                                     char const   FAR *filename,
                                                     U32    size);

DXDEC HAUDIO  AILCALL AIL_quick_copy                (HAUDIO      audio);

DXDEC void    AILCALL AIL_quick_unload              (HAUDIO      audio);

DXDEC S32     AILCALL AIL_quick_play                (HAUDIO      audio,
                                                    U32         loop_count);

DXDEC void    AILCALL AIL_quick_halt                (HAUDIO      audio);

DXDEC S32     AILCALL AIL_quick_status              (HAUDIO      audio);

DXDEC HAUDIO  AILCALL AIL_quick_load_and_play       (char const   FAR *filename,
                                                    U32         loop_count,
                                                    S32         wait_request);

#ifdef ON_MAC_USE_FSS
DXDEC HAUDIO  AILCALL AIL_quick_fss_load_and_play  (FSSpec const   FAR *filename,
                                                    U32         loop_count,
                                                    S32         wait_request);
#endif

DXDEC void   AILCALL AIL_quick_set_speed (HAUDIO audio, S32 speed);

DXDEC void   AILCALL AIL_quick_set_volume (HAUDIO audio, F32 volume, F32 extravol);

DXDEC void   AILCALL AIL_quick_set_reverb_levels (HAUDIO audio,
                                                  F32    dry_level,
                                                  F32    wet_level);

DXDEC void   AILCALL AIL_quick_set_low_pass_cut_off(HAUDIO S,
                                                    S32 channel,
                                                    F32 cut_off);

DXDEC void   AILCALL AIL_quick_set_ms_position(HAUDIO audio,S32 milliseconds);

DXDEC S32    AILCALL AIL_quick_ms_position(HAUDIO audio);

DXDEC S32    AILCALL AIL_quick_ms_length(HAUDIO audio);


#define AIL_QUICK_XMIDI_TYPE        1
#define AIL_QUICK_DIGITAL_TYPE      2
#define AIL_QUICK_DLS_XMIDI_TYPE    3
#define AIL_QUICK_MPEG_DIGITAL_TYPE 4
#define AIL_QUICK_OGG_VORBIS_TYPE   5
#define AIL_QUICK_V12_VOICE_TYPE    6
#define AIL_QUICK_V24_VOICE_TYPE    7
#define AIL_QUICK_V29_VOICE_TYPE    8
#define AIL_QUICK_OGG_SPEEX_TYPE    9
#define AIL_QUICK_S8_VOICE_TYPE     10
#define AIL_QUICK_S16_VOICE_TYPE    11
#define AIL_QUICK_S32_VOICE_TYPE    12

DXDEC S32    AILCALL AIL_quick_type(HAUDIO audio);

DXDEC void AILCALL AIL_save_sample_attributes(HSAMPLE   S,
                                              HSATTRIBS D);

DXDEC U32 AILCALL AIL_load_sample_attributes(HSAMPLE   S,
                                             HSATTRIBS D);

DXDEC S32 AILCALL AIL_WAV_info(void const FAR* data, AILSOUNDINFO FAR* info);

DXDEC S32 AILCALL AIL_size_processed_digital_audio(
                                 U32             dest_rate,
                                 U32             dest_format,
                                 S32             num_srcs,
                                 AILMIXINFO const FAR * src);

DXDEC S32 AILCALL AIL_process_digital_audio(
                                 void FAR       *dest_buffer,
                                 S32             dest_buffer_size,
                                 U32             dest_rate,
                                 U32             dest_format,
                                 S32             num_srcs,
                                 AILMIXINFO FAR* src);

#define AIL_LENGTHY_INIT           0
#define AIL_LENGTHY_SET_PROPERTY   1
#define AIL_LENGTHY_UPDATE         2
#define AIL_LENGTHY_DONE           3

typedef S32 (AILCALLBACK FAR* AILLENGTHYCB)(U32 state,UINTa user);

typedef S32 (AILCALLBACK FAR* AILCODECSETPROP)(char const FAR* property,void const FAR * value);

DXDEC S32 AILCALL AIL_compress_ASI(AILSOUNDINFO const FAR * info, //)
                                   char const FAR* filename_ext,
                                   void FAR* FAR* outdata,
                                   U32 FAR* outsize,
                                   AILLENGTHYCB callback);

DXDEC S32 AILCALL AIL_decompress_ASI(void const FAR* indata, //)
                                     U32 insize,
                                     char const FAR* filename_ext,
                                     void FAR* FAR* wav,
                                     U32 FAR* wavsize,
                                     AILLENGTHYCB callback);

DXDEC S32 AILCALL AIL_compress_ADPCM(AILSOUNDINFO const FAR * info,
                                     void FAR* FAR* outdata, U32 FAR* outsize);

DXDEC S32 AILCALL AIL_decompress_ADPCM(AILSOUNDINFO const FAR * info,
                                       void FAR* FAR* outdata, U32 FAR* outsize);

DXDEC S32 AILCALL AIL_compress_DLS(void const FAR* dls,
                                   char const FAR* compression_extension,
                                   void FAR* FAR* mls, U32 FAR* mlssize,
                                   AILLENGTHYCB callback);

DXDEC S32 AILCALL AIL_merge_DLS_with_XMI(void const FAR* xmi, void const FAR* dls,
                                         void FAR* FAR* mss, U32 FAR* msssize);

DXDEC  S32 AILCALL AIL_extract_DLS( void const FAR *source_image, //)
                                    U32             source_size,
                                    void FAR * FAR *XMI_output_data,
                                    U32  FAR       *XMI_output_size,
                                    void FAR * FAR *DLS_output_data,
                                    U32  FAR       *DLS_output_size,
                                    AILLENGTHYCB   callback);

#define AILFILTERDLS_USINGLIST 1

DXDEC S32 AILCALL AIL_filter_DLS_with_XMI(void const FAR* xmi, void const FAR* dls,
                                         void FAR* FAR* dlsout, U32 FAR* dlssize,
                                         S32  flags, AILLENGTHYCB callback);

#define AILMIDITOXMI_USINGLIST 1
#define AILMIDITOXMI_TOLERANT  2

DXDEC  S32 AILCALL AIL_MIDI_to_XMI       (void const FAR*  MIDI,
                                         U32        MIDI_size,
                                         void FAR*  FAR*XMIDI,
                                         U32  FAR*  XMIDI_size,
                                         S32        flags);

#define AILDLSLIST_ARTICULATION 1
#define AILDLSLIST_DUMP_WAVS    2

#if defined(IS_WIN32) || defined(IS_MAC)

DXDEC  S32          AILCALL AIL_list_DLS          (void const FAR* DLS,
                                                   char FAR* FAR* lst,
                                                   U32  FAR* lst_size,
                                                   S32       flags,
                                                   C8   FAR* title);

#define AILMIDILIST_ROLANDSYSEX 1
#define AILMIDILIST_ROLANDUN    2
#define AILMIDILIST_ROLANDAB    4

DXDEC  S32          AILCALL AIL_list_MIDI         (void const FAR* MIDI,
                                                  U32       MIDI_size,
                                                  char FAR* FAR* lst,
                                                  U32  FAR* lst_size,
                                                  S32       flags);
#endif

#define AILFILETYPE_UNKNOWN         0
#define AILFILETYPE_PCM_WAV         1
#define AILFILETYPE_ADPCM_WAV       2
#define AILFILETYPE_OTHER_WAV       3
#define AILFILETYPE_VOC             4
#define AILFILETYPE_MIDI            5
#define AILFILETYPE_XMIDI           6
#define AILFILETYPE_XMIDI_DLS       7
#define AILFILETYPE_XMIDI_MLS       8
#define AILFILETYPE_DLS             9
#define AILFILETYPE_MLS            10
#define AILFILETYPE_MPEG_L1_AUDIO  11
#define AILFILETYPE_MPEG_L2_AUDIO  12
#define AILFILETYPE_MPEG_L3_AUDIO  13
#define AILFILETYPE_OTHER_ASI_WAV  14
#define AILFILETYPE_XBOX_ADPCM_WAV 15
#define AILFILETYPE_OGG_VORBIS     16
#define AILFILETYPE_V12_VOICE      17
#define AILFILETYPE_V24_VOICE      18
#define AILFILETYPE_V29_VOICE      19
#define AILFILETYPE_OGG_SPEEX      20
#define AILFILETYPE_S8_VOICE       21
#define AILFILETYPE_S16_VOICE      22
#define AILFILETYPE_S32_VOICE      23

DXDEC S32 AILCALL AIL_file_type(void const FAR* data, U32 size);

DXDEC S32 AILCALL AIL_file_type_named(void const FAR* data, char const FAR* filename, U32 size);

DXDEC S32 AILCALL AIL_find_DLS       (void const FAR*      data, U32 size,
                                      void FAR* FAR* xmi, U32 FAR* xmisize,
                                      void FAR* FAR* dls, U32 FAR* dlssize);
typedef struct
{
   //
   // File-level data accessible to app
   //
   // This is valid after AIL_inspect_MP3() is called (even if the file contains no valid frames)
   //

   U8 FAR *MP3_file_image;       // Original MP3_file_image pointer passed to AIL_inspect_MP3()
   S32     MP3_image_size;       // Original MP3_image_size passed to AIL_inspect_MP3()

   U8 FAR *ID3v2;                // ID3v2 tag, if not NULL
   S32     ID3v2_size;           // Size of tag in bytes

   U8 FAR *ID3v1;                // ID3v1 tag, if not NULL (always 128 bytes long if present)

   U8 FAR *start_MP3_data;       // Pointer to start of data area in file (not necessarily first valid frame)
   U8 FAR *end_MP3_data;         // Pointer to last valid byte in MP3 data area (before ID3v1 tag, if any)

   //
   // Information about current frame being inspected, valid if AIL_enumerate_MP3_frames() returns 
   // TRUE
   //

   S32 sample_rate;              // Sample rate in Hz (normally constant across all frames in file)
   S32 bit_rate;                 // Bits/second for current frame
   S32 channels_per_sample;      // 1 or 2
   S32 samples_per_frame;        // Always 576 or 1152 samples in each MP3 frame, depending on rate

   S32 byte_offset;              // Offset of frame from start_MP3_data (i.e., suitable for use as loop point)
   S32 next_frame_expected;      // Anticipated offset of next frame to be enumerated, if any
   S32 average_frame_size;       // Average source bytes per frame, determined solely by bit rate and sample rate
   S32 data_size;                // # of data-only bytes in this particular frame
   S32 header_size;              // 4 or 6 bytes, depending on CRC
   S32 side_info_size;           // Valid for layer 3 side info only
   S32 ngr;                      // Always 2 for MPEG1, else 1
   S32 main_data_begin;          // Always 0 in files with no bit reservoir
   S32 hpos;                     // Current bit position in header/side buffer

   S32 MPEG1;                    // Data copied directly from frame header, see ISO docs for info... 
   S32 MPEG25;                   
   S32 layer;                    
   S32 protection_bit;
   S32 bitrate_index;
   S32 sampling_frequency;
   S32 padding_bit;
   S32 private_bit;
   S32 mode;
   S32 mode_extension;
   S32 copyright;
   S32 original;
   S32 emphasis;

   //
   // LAME/Xing info tag data
   //

   S32 Xing_valid;
   S32 Info_valid;
   U32 header_flags;
   S32 frame_count;
   S32 byte_count;
   S32 VBR_scale;
   U8  TOC[100];
   S32 enc_delay;
   S32 enc_padding;

   //
   // Private (undocumented) data used during frame enumeration
   //

   U8 FAR *ptr;
   S32 bytes_left;

   S32 check_valid;
   S32 check_MPEG1;
   S32 check_MPEG25;
   S32 check_layer;
   S32 check_protection_bit;
   S32 check_sampling_frequency;
   S32 check_mode;
   S32 check_copyright;
   S32 check_original;
}
MP3_INFO;

DXDEC void AILCALL AIL_inspect_MP3 (MP3_INFO FAR *inspection_state,
                                    U8       FAR *MP3_file_image,
                                    S32           MP3_image_size);

DXDEC S32 AILCALL AIL_enumerate_MP3_frames (MP3_INFO FAR *inspection_state);

//
// RAD room types - currently the same as EAX
//

enum
{
    ENVIRONMENT_GENERIC,                // factory default
    ENVIRONMENT_PADDEDCELL,
    ENVIRONMENT_ROOM,                   // standard environments
    ENVIRONMENT_BATHROOM,
    ENVIRONMENT_LIVINGROOM,
    ENVIRONMENT_STONEROOM,
    ENVIRONMENT_AUDITORIUM,
    ENVIRONMENT_CONCERTHALL,
    ENVIRONMENT_CAVE,
    ENVIRONMENT_ARENA,
    ENVIRONMENT_HANGAR,
    ENVIRONMENT_CARPETEDHALLWAY,
    ENVIRONMENT_HALLWAY,
    ENVIRONMENT_STONECORRIDOR,
    ENVIRONMENT_ALLEY,
    ENVIRONMENT_FOREST,
    ENVIRONMENT_CITY,
    ENVIRONMENT_MOUNTAINS,
    ENVIRONMENT_QUARRY,
    ENVIRONMENT_PLAIN,
    ENVIRONMENT_PARKINGLOT,
    ENVIRONMENT_SEWERPIPE,
    ENVIRONMENT_UNDERWATER,
    ENVIRONMENT_DRUGGED,
    ENVIRONMENT_DIZZY,
    ENVIRONMENT_PSYCHOTIC,

    ENVIRONMENT_COUNT           // total number of environments
};

//
// enumerated values for EAX
//

#ifndef EAX_H_INCLUDED

enum
{
    EAX_ENVIRONMENT_GENERIC,                // factory default
    EAX_ENVIRONMENT_PADDEDCELL,
    EAX_ENVIRONMENT_ROOM,              // standard environments
    EAX_ENVIRONMENT_BATHROOM,
    EAX_ENVIRONMENT_LIVINGROOM,
    EAX_ENVIRONMENT_STONEROOM,
    EAX_ENVIRONMENT_AUDITORIUM,
    EAX_ENVIRONMENT_CONCERTHALL,
    EAX_ENVIRONMENT_CAVE,
    EAX_ENVIRONMENT_ARENA,
    EAX_ENVIRONMENT_HANGAR,
    EAX_ENVIRONMENT_CARPETEDHALLWAY,
    EAX_ENVIRONMENT_HALLWAY,
    EAX_ENVIRONMENT_STONECORRIDOR,
    EAX_ENVIRONMENT_ALLEY,
    EAX_ENVIRONMENT_FOREST,
    EAX_ENVIRONMENT_CITY,
    EAX_ENVIRONMENT_MOUNTAINS,
    EAX_ENVIRONMENT_QUARRY,
    EAX_ENVIRONMENT_PLAIN,
    EAX_ENVIRONMENT_PARKINGLOT,
    EAX_ENVIRONMENT_SEWERPIPE,
    EAX_ENVIRONMENT_UNDERWATER,
    EAX_ENVIRONMENT_DRUGGED,
    EAX_ENVIRONMENT_DIZZY,
    EAX_ENVIRONMENT_PSYCHOTIC,

    EAX_ENVIRONMENT_COUNT           // total number of environments
};

#define EAX_REVERBMIX_USEDISTANCE (-1.0F)

#endif

#define MSS_BUFFER_HEAD (-1)

#if defined(IS_WIN32API) || defined(IS_MAC) || defined(IS_LINUX) || defined(IS_DOS) || defined(IS_PS2) || defined(IS_PS3) || defined(IS_WII)

//
// Auxiliary 2D interface calls
//

DXDEC HDIGDRIVER AILCALL AIL_primary_digital_driver  (HDIGDRIVER new_primary);

//
// 3D-related calls
//

DXDEC  S32      AILCALL AIL_room_type                (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_set_room_type            (HDIGDRIVER dig,
                                                      S32        room_type);

DXDEC  F32      AILCALL AIL_3D_rolloff_factor        (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_set_3D_rolloff_factor    (HDIGDRIVER dig,
                                                      F32       factor);

DXDEC  F32      AILCALL AIL_3D_doppler_factor        (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_set_3D_doppler_factor    (HDIGDRIVER dig,
                                                      F32       factor);

DXDEC  F32      AILCALL AIL_3D_distance_factor       (HDIGDRIVER dig);

DXDEC  void     AILCALL AIL_set_3D_distance_factor   (HDIGDRIVER dig,
                                                      F32       factor);

DXDEC void       AILCALL AIL_set_sample_obstruction  (HSAMPLE S,
                                                      F32     obstruction);

DXDEC void       AILCALL AIL_set_sample_occlusion    (HSAMPLE S,
                                                      F32     occlusion);

DXDEC void       AILCALL AIL_set_sample_exclusion    (HSAMPLE S,
                                                      F32     exclusion);

DXDEC F32        AILCALL AIL_sample_obstruction      (HSAMPLE S);

DXDEC F32        AILCALL AIL_sample_occlusion        (HSAMPLE S);

DXDEC F32        AILCALL AIL_sample_exclusion        (HSAMPLE S);

DXDEC void       AILCALL AIL_set_sample_3D_distances (HSAMPLE S,
                                                      F32     max_dist,
                                                      F32     min_dist,
                                                      S32     auto_3D_wet_atten);


DXDEC void       AILCALL AIL_sample_3D_distances     (HSAMPLE S,
                                                      F32 FAR * max_dist,
                                                      F32 FAR * min_dist,
                                                      S32 FAR * auto_3D_wet_atten);

DXDEC void       AILCALL AIL_set_sample_3D_cone        (HSAMPLE S,
                                                        F32     inner_angle,
                                                        F32     outer_angle,
                                                        F32     outer_volume_level);

DXDEC void       AILCALL AIL_sample_3D_cone            (HSAMPLE S,
                                                        F32 FAR*  inner_angle,
                                                        F32 FAR*  outer_angle,
                                                        F32 FAR*  outer_volume_level);

DXDEC void       AILCALL AIL_set_sample_3D_position    (HSAMPLE obj,
                                                        F32     X,
                                                        F32     Y,
                                                        F32     Z);

DXDEC void       AILCALL AIL_set_sample_3D_velocity    (HSAMPLE obj,
                                                        F32     dX_per_ms,
                                                        F32     dY_per_ms,
                                                        F32     dZ_per_ms,
                                                        F32     magnitude);

DXDEC void       AILCALL AIL_set_sample_3D_velocity_vector  (HSAMPLE obj,
                                                             F32     dX_per_ms,
                                                             F32     dY_per_ms,
                                                             F32     dZ_per_ms);

DXDEC void       AILCALL AIL_set_sample_3D_orientation      (HSAMPLE obj,
                                                             F32     X_face,
                                                             F32     Y_face,
                                                             F32     Z_face,
                                                             F32     X_up,
                                                             F32     Y_up,
                                                             F32     Z_up);

DXDEC S32        AILCALL AIL_sample_3D_position             (HSAMPLE  obj,
                                                             F32 FAR *X,
                                                             F32 FAR *Y,
                                                             F32 FAR *Z);

DXDEC void       AILCALL AIL_sample_3D_velocity             (HSAMPLE  obj,
                                                             F32 FAR *dX_per_ms,
                                                             F32 FAR *dY_per_ms,
                                                             F32 FAR *dZ_per_ms);

DXDEC void       AILCALL AIL_sample_3D_orientation          (HSAMPLE  obj,
                                                             F32 FAR *X_face,
                                                             F32 FAR *Y_face,
                                                             F32 FAR *Z_face,
                                                             F32 FAR *X_up,
                                                             F32 FAR *Y_up,
                                                             F32 FAR *Z_up);

DXDEC void       AILCALL AIL_update_sample_3D_position      (HSAMPLE obj,
                                                             F32     dt_milliseconds);

DXDEC void       AILCALL AIL_set_listener_3D_position         (HDIGDRIVER dig,
                                                               F32     X,
                                                               F32     Y,
                                                               F32     Z);

DXDEC void       AILCALL AIL_set_listener_3D_velocity         (HDIGDRIVER dig,
                                                               F32     dX_per_ms,
                                                               F32     dY_per_ms,
                                                               F32     dZ_per_ms,
                                                               F32     magnitude);

DXDEC void       AILCALL AIL_set_listener_3D_velocity_vector  (HDIGDRIVER dig,
                                                               F32     dX_per_ms,
                                                               F32     dY_per_ms,
                                                               F32     dZ_per_ms);

DXDEC void       AILCALL AIL_set_listener_3D_orientation      (HDIGDRIVER dig,
                                                               F32     X_face,
                                                               F32     Y_face,
                                                               F32     Z_face,
                                                               F32     X_up,
                                                               F32     Y_up,
                                                               F32     Z_up);
                                                               
DXDEC void       AILCALL AIL_listener_3D_position             (HDIGDRIVER  dig,
                                                               F32 FAR *X,
                                                               F32 FAR *Y,
                                                               F32 FAR *Z);

DXDEC void       AILCALL AIL_listener_3D_velocity             (HDIGDRIVER  dig,
                                                               F32 FAR *dX_per_ms,
                                                               F32 FAR *dY_per_ms,
                                                               F32 FAR *dZ_per_ms);

DXDEC void       AILCALL AIL_listener_3D_orientation          (HDIGDRIVER  dig,
                                                               F32 FAR *X_face,
                                                               F32 FAR *Y_face,
                                                               F32 FAR *Z_face,
                                                               F32 FAR *X_up,
                                                               F32 FAR *Y_up,
                                                               F32 FAR *Z_up);

DXDEC void       AILCALL AIL_update_listener_3D_position      (HDIGDRIVER dig,
                                                               F32     dt_milliseconds);

#endif

#if defined( HOST_SPU_PROCESS )

DXDEC S32 AILCALL MilesStartAsyncThread( S32 thread_num, void const * param );

DXDEC S32 AILCALL MilesRequestStopAsyncThread( S32 thread_num );

DXDEC S32 AILCALL MilesWaitStopAsyncThread( S32 thread_num );

#endif

#ifdef MSS_FLT_SUPPORTED

//
// Filter result codes
//

typedef SINTa FLTRESULT;

#define FLT_NOERR                   0   // Success -- no error
#define FLT_NOT_ENABLED             1   // FLT not enabled
#define FLT_ALREADY_STARTED         2   // FLT already started
#define FLT_INVALID_PARAM           3   // Invalid parameters used
#define FLT_INTERNAL_ERR            4   // Internal error in FLT driver
#define FLT_OUT_OF_MEM              5   // Out of system RAM
#define FLT_ERR_NOT_IMPLEMENTED     6   // Feature not implemented
#define FLT_NOT_FOUND               7   // FLT supported device not found
#define FLT_NOT_INIT                8   // FLT not initialized
#define FLT_CLOSE_ERR               9   // FLT not closed correctly

//############################################################################
//##                                                                        ##
//## Interface "MSS pipeline filter" (some functions shared by              ##
//## "MSS voice filter")                                                    ##
//##                                                                        ##
//############################################################################

typedef FLTRESULT (AILCALL FAR *FLT_STARTUP)(void);

typedef FLTRESULT (AILCALL FAR *FLT_SHUTDOWN)(void);

typedef C8 FAR *  (AILCALL FAR *FLT_ERROR)(void);

typedef HDRIVERSTATE (AILCALL FAR *FLT_OPEN_DRIVER) (HDIGDRIVER dig);

typedef FLTRESULT    (AILCALL FAR *FLT_CLOSE_DRIVER) (HDRIVERSTATE state);

typedef void         (AILCALL FAR *FLT_PREMIX_PROCESS) (HDRIVERSTATE driver);

typedef S32          (AILCALL FAR *FLT_POSTMIX_PROCESS) (HDRIVERSTATE driver, void FAR *output_buffer);

//############################################################################
//##                                                                        ##
//## Interface "Pipeline filter sample services"                            ##
//##                                                                        ##
//############################################################################

typedef HSAMPLESTATE (AILCALL FAR * FLTSMP_OPEN_SAMPLE) (HDRIVERSTATE driver,
                                                         HSAMPLE      S);

typedef FLTRESULT    (AILCALL FAR * FLTSMP_CLOSE_SAMPLE) (HSAMPLESTATE state);

typedef void         (AILCALL FAR * FLTSMP_SAMPLE_PROCESS) (HSAMPLESTATE    state,
                                                            void FAR *      source_buffer,
                                                            void FAR *      dest_buffer, // may be the same as src
                                                            S32             n_samples,
                                                            S32             playback_rate,
                                                            S32             is_stereo );

typedef S32          (AILCALL FAR * FLTSMP_SAMPLE_PROPERTY) (HSAMPLESTATE    state,
                                                             HPROPERTY       property,
                                                             void FAR*       before_value,
                                                             void const FAR* new_value,
                                                             void FAR*       after_value
                                                             );

//############################################################################
//##                                                                        ##
//## Interface "MSS output filter"                                          ##
//##                                                                        ##
//############################################################################

typedef S32 (AILCALL FAR * VFLT_ASSIGN_SAMPLE_VOICE) (HDRIVERSTATE driver,
                                                      HSAMPLE      S);

typedef void (AILCALL FAR * VFLT_RELEASE_SAMPLE_VOICE) (HDRIVERSTATE driver,
                                                        HSAMPLE      S);

typedef S32 (AILCALL FAR * VFLT_START_SAMPLE_VOICE) (HDRIVERSTATE driver,
                                                     HSAMPLE      S);

//############################################################################
//##                                                                        ##
//## Interface "Voice filter driver services"                               ##
//##                                                                        ##
//############################################################################

typedef S32          (AILCALL FAR * VDRV_DRIVER_PROPERTY) (HDRIVERSTATE    driver,
                                                           HPROPERTY       property,
                                                           void FAR*       before_value,
                                                           void const FAR* new_value,
                                                           void FAR*       after_value
                                                           );

typedef S32          (AILCALL FAR * VDRV_FORCE_UPDATE)     (HDRIVERSTATE driver);

//############################################################################
//##                                                                        ##
//## Interface "Voice filter sample services"                               ##
//##                                                                        ##
//############################################################################

typedef S32          (AILCALL FAR * VSMP_SAMPLE_PROPERTY) (HSAMPLE      S,
                                                           HPROPERTY       property,
                                                           void FAR*       before_value,
                                                           void const FAR* new_value,
                                                           void FAR*       after_value
                                                           );

//
// Pipeline filter calls
//

DXDEC HPROVIDER  AILCALL AIL_digital_output_filter (HDIGDRIVER dig);

DXDEC S32        AILCALL AIL_enumerate_filters  (HPROENUM  FAR *next,
                                                 HPROVIDER FAR *dest,
                                                 C8  FAR * FAR *name);
DXDEC HDRIVERSTATE
                 AILCALL AIL_open_filter        (HPROVIDER  lib,
                                                 HDIGDRIVER dig);

DXDEC void       AILCALL AIL_close_filter       (HDRIVERSTATE filter);

DXDEC S32        AILCALL AIL_find_filter        (C8 const  *name, 
                                                 HPROVIDER *ret);

DXDEC S32        AILCALL AIL_enumerate_filter_properties
                                                (HPROVIDER                  lib,
                                                 HINTENUM FAR *             next,
                                                 RIB_INTERFACE_ENTRY FAR *  dest);

DXDEC S32        AILCALL AIL_filter_property    (HPROVIDER  lib,
                                                 C8 const FAR*   name,
                                                 void FAR*       before_value,
                                                 void const FAR* new_value,
                                                 void FAR*       after_value
                                                 );

DXDEC  S32      AILCALL AIL_enumerate_output_filter_driver_properties
                                                (HPROVIDER                 lib,
                                                 HINTENUM FAR *            next,
                                                 RIB_INTERFACE_ENTRY FAR * dest);

DXDEC  S32     AILCALL AIL_output_filter_driver_property
                                                (HDIGDRIVER     dig,
                                                 C8 const FAR * name,
                                                 void FAR*       before_value,
                                                 void const FAR* new_value,
                                                 void FAR*       after_value
                                                 );

DXDEC  S32      AILCALL AIL_enumerate_output_filter_sample_properties
                                                (HPROVIDER                 lib,
                                                 HINTENUM FAR *            next,
                                                 RIB_INTERFACE_ENTRY FAR * dest);

DXDEC  S32      AILCALL AIL_enumerate_filter_sample_properties
                                                (HPROVIDER                 lib,
                                                 HINTENUM FAR *            next,
                                                 RIB_INTERFACE_ENTRY FAR * dest);

DXDEC S32       AILCALL AIL_enumerate_sample_stage_properties
                                                (HSAMPLE                    S,
                                                 SAMPLESTAGE                stage,
                                                 HINTENUM FAR *             next,
                                                 RIB_INTERFACE_ENTRY FAR *  dest);

DXDEC  S32      AILCALL AIL_sample_stage_property
                                                (HSAMPLE        S,
                                                 SAMPLESTAGE    stage,
                                                 C8 const FAR * name,
                                                 S32            channel,
                                                 void FAR*       before_value,
                                                 void const FAR* new_value,
                                                 void FAR*       after_value
                                                 );

#define AIL_filter_sample_property(S,name,beforev,newv,afterv) AIL_sample_stage_property((S),SP_FILTER_0,(name),-1,(beforev),(newv),(afterv))

typedef struct _FLTPROVIDER
{
   PROVIDER_PROPERTY               PROVIDER_property;

   FLT_STARTUP                     startup;
   FLT_ERROR                       error;
   FLT_SHUTDOWN                    shutdown;
   FLT_OPEN_DRIVER                 open_driver;
   FLT_CLOSE_DRIVER                close_driver;
   FLT_PREMIX_PROCESS              premix_process;
   FLT_POSTMIX_PROCESS             postmix_process;

   FLTSMP_OPEN_SAMPLE              open_sample;
   FLTSMP_CLOSE_SAMPLE             close_sample;
   FLTSMP_SAMPLE_PROCESS           sample_process;
   FLTSMP_SAMPLE_PROPERTY          sample_property;

   VFLT_ASSIGN_SAMPLE_VOICE        assign_sample_voice;
   VFLT_RELEASE_SAMPLE_VOICE       release_sample_voice;
   VFLT_START_SAMPLE_VOICE         start_sample_voice;

   VDRV_DRIVER_PROPERTY            driver_property;
   VDRV_FORCE_UPDATE               force_update;

   VSMP_SAMPLE_PROPERTY            output_sample_property;

   HDIGDRIVER   dig;
   HPROVIDER    provider;
   HDRIVERSTATE driver_state;
   S32          provider_flags;

   struct _FLTPROVIDER FAR *next;
}
FLTPROVIDER;

//
// Values for "Flags" property exported by all MSS Pipeline Filter and MSS Output Filter 
// providers
// 

#define FPROV_ON_SAMPLES 0x0001        // Pipeline filter that operates on input samples (and is enumerated by AIL_enumerate_filters)
#define FPROV_ON_POSTMIX 0x0002        // Pipeline filter that operates on the post mixed output (capture filter)
#define FPROV_MATRIX     0x0004        // This is a matrix output filter (e.g., SRS/Dolby) 
#define FPROV_VOICE      0x0008        // This is a per-voice output filter (e.g., DirectSound 3D)
#define FPROV_3D         0x0010        // Output filter uses S3D substructure for positioning
#define FPROV_OCCLUSION  0x0020        // Output filter supports occlusion (doesn't need per-sample lowpass)
#define FPROV_EAX        0x0040        // Output filter supports EAX-compatible environmental reverb

#ifdef IS_WIN32

#define MSS_EAX_AUTO_GAIN   1
#define MSS_EAX_AUTOWAH     2
#define MSS_EAX_CHORUS      3
#define MSS_EAX_DISTORTION  4
#define MSS_EAX_ECHO        5
#define MSS_EAX_EQUALIZER   6
#define MSS_EAX_FLANGER     7
#define MSS_EAX_FSHIFTER    8
#define MSS_EAX_VMORPHER    9
#define MSS_EAX_PSHIFTER   10
#define MSS_EAX_RMODULATOR 11 
#define MSS_EAX_REVERB     12

typedef struct EAX_SAMPLE_SLOT_VOLUME
{
  S32 Slot;       // 0, 1, 2, 3
  S32 Send;
  S32 SendHF;
  S32 Occlusion;
  F32 OcclusionLFRatio;
  F32 OcclusionRoomRatio;
  F32 OcclusionDirectRatio;
} EAX_SAMPLE_SLOT_VOLUME;

typedef struct EAX_SAMPLE_SLOT_VOLUMES
{
  U32 NumVolumes;  // 0, 1, or 2
  EAX_SAMPLE_SLOT_VOLUME volumes[ 2 ];
} EAX_SAMPLE_SLOT_VOLUMES;

// Use this structure for EAX REVERB
typedef struct EAX_REVERB
{
  S32 Effect;                  // set to MSS_EAX_REVERB
  S32 Volume;                  // -10000 to 0
  U32 Environment;             // one of the ENVIRONMENT_ enums
  F32 EnvironmentSize;         // environment size in meters
  F32 EnvironmentDiffusion;    // environment diffusion
  S32 Room;                    // room effect level (at mid frequencies)
  S32 RoomHF;                  // relative room effect level at high frequencies
  S32 RoomLF;                  // relative room effect level at low frequencies  
  F32 DecayTime;               // reverberation decay time at mid frequencies
  F32 DecayHFRatio;            // high-frequency to mid-frequency decay time ratio
  F32 DecayLFRatio;            // low-frequency to mid-frequency decay time ratio   
  S32 Reflections;             // early reflections level relative to room effect
  F32 ReflectionsDelay;        // initial reflection delay time
  F32 ReflectionsPanX;         // early reflections panning vector
  F32 ReflectionsPanY;         // early reflections panning vector
  F32 ReflectionsPanZ;         // early reflections panning vector
  S32 Reverb;                  // late reverberation level relative to room effect
  F32 ReverbDelay;             // late reverberation delay time relative to initial reflection
  F32 ReverbPanX;              // late reverberation panning vector
  F32 ReverbPanY;              // late reverberation panning vector
  F32 ReverbPanZ;              // late reverberation panning vector
  F32 EchoTime;                // echo time
  F32 EchoDepth;               // echo depth
  F32 ModulationTime;          // modulation time
  F32 ModulationDepth;         // modulation depth
  F32 AirAbsorptionHF;         // change in level per meter at high frequencies
  F32 HFReference;             // reference high frequency
  F32 LFReference;             // reference low frequency 
  F32 RoomRolloffFactor;       // like DS3D flRolloffFactor but for room effect
  U32 Flags;                   // modifies the behavior of properties
} EAX_REVERB;

// Use this structure for EAX AUTOGAIN
typedef struct EAX_AUTOGAIN
{
  S32 Effect;      // set to MSS_EAX_AUTO_GAIN
  S32 Volume;      // -10000 to 0
  U32 OnOff;       // Switch Compressor on or off (1 or 0)
} EAX_AUTOGAIN;

// Use this structure for EAX AUTOWAH
typedef struct EAX_AUTOWAH
{
   S32 Effect;        // set to MSS_EAX_AUTOWAH
   S32 Volume;        // -10000 to 0
   F32 AttackTime;    // Attack time (seconds)
   F32 ReleaseTime;   // Release time (seconds)
   S32 Resonance;     // Resonance (mB)
   S32 PeakLevel;     // Peak level (mB)
} EAX_AUTOWAH;

// Use this structure for EAX CHORUS
typedef struct EAX_CHORUS
{
  S32 Effect;       // set to MSS_EAX_CHORUS
  S32 Volume;       // -10000 to 0
  U32 Waveform;     // Waveform selector - 0 = sinusoid, 1 = triangle
  S32 Phase;        // Phase (Degrees)
  F32 Rate;         // Rate (Hz)
  F32 Depth;        // Depth (0 to 1)
  F32 Feedback;     // Feedback (-1 to 1)
  F32 Delay;        // Delay (seconds)
} EAX_CHORUS;

// Use this structure for EAX DISTORTION
typedef struct EAX_DISTORTION
{
  S32 Effect;        // set to MSS_EAX_DISTORTION
  S32 Volume;        // -10000 to 0
  F32 Edge;          // Controls the shape of the distortion (0 to 1)
  S32 Gain;          // Controls the post distortion gain (mB)
  F32 LowPassCutOff; // Controls the cut-off of the filter pre-distortion (Hz)
  F32 EQCenter;      // Controls the center frequency of the EQ post-distortion (Hz)
  F32 EQBandwidth;   // Controls the bandwidth of the EQ post-distortion (Hz)
} EAX_DISTORTION;

// Use this structure for EAX ECHO
typedef struct EAX_ECHO
{
  S32 Effect;        // set to MSS_EAX_ECHO
  S32 Volume;        // -10000 to 0
  F32 Delay;         // Controls the initial delay time (seconds)
  F32 LRDelay;       // Controls the delay time between the first and second taps (seconds)
  F32 Damping;       // Controls a low-pass filter that dampens the echoes (0 to 1)
  F32 Feedback;      // Controls the duration of echo repetition (0 to 1)
  F32 Spread;        // Controls the left-right spread of the echoes
} EAX_ECHO;

// Use this structure for EAXEQUALIZER_ALLPARAMETERS
typedef struct EAX_EQUALIZER
{
  S32 Effect;        // set to MSS_EAX_EQUALIZER
  S32 Volume;        // -10000 to 0
  S32 LowGain;       // (mB)
  F32 LowCutOff;     // (Hz)
  S32 Mid1Gain;      // (mB)
  F32 Mid1Center;    // (Hz)
  F32 Mid1Width;     // (octaves)
  F32 Mid2Gain;      // (mB)
  F32 Mid2Center;    // (Hz)
  F32 Mid2Width;     // (octaves)
  S32 HighGain;      // (mB)
  F32 HighCutOff;    // (Hz)
} EAX_EQUALIZER;

// Use this structure for EAX FLANGER
typedef struct EAX_FLANGER
{
  S32 Effect;       // set to MSS_EAX_FLANGER
  S32 Volume;       // -10000 to 0
  U32 Waveform;     // Waveform selector - 0 = sinusoid, 1 = triangle
  S32 Phase;        // Phase (Degrees)
  F32 Rate;         // Rate (Hz)
  F32 Depth;        // Depth (0 to 1)
  F32 Feedback;     // Feedback (0 to 1)
  F32 Delay;        // Delay (seconds)
} EAX_FLANGER;


// Use this structure for EAX FREQUENCY SHIFTER
typedef struct EAX_FSHIFTER
{
  S32 Effect;         // set to MSS_EAX_FSHIFTER
  S32 Volume;         // -10000 to 0
  F32 Frequency;      // (Hz)
  U32 LeftDirection;  // direction - 0 = down, 1 = up, 2 = off
  U32 RightDirection; // direction - 0 = down, 1 = up, 2 = off
} EAX_FSHIFTER;

// Use this structure for EAX VOCAL MORPHER
typedef struct EAX_VMORPHER
{
  S32 Effect;                // set to MSS_EAX_VMORPHER
  S32 Volume;                // -10000 to 0
  U32 PhonemeA;              // phoneme: 0 to 29 - A E I O U AA AE AH AO EH ER IH IY UH UW B D G J K L M N P R S T V Z
  S32 PhonemeACoarseTuning;  // (semitones)
  U32 PhonemeB;              // phoneme: 0 to 29 - A E I O U AA AE AH AO EH ER IH IY UH UW B D G J K L M N P R S T V Z
  S32 PhonemeBCoarseTuning;  // (semitones)
  U32 Waveform;              // Waveform selector - 0 = sinusoid, 1 = triangle, 2 = sawtooth
  F32 Rate;                  // (Hz)
} EAX_VMORPHER;


// Use this structure for EAX PITCH SHIFTER
typedef struct EAX_PSHIFTER
{
  S32 Effect;       // set to MSS_EAX_PSHIFTER
  S32 Volume;       // -10000 to 0
  S32 CoarseTune;   // Amount of pitch shift (semitones)
  S32 FineTune;     // Amount of pitch shift (cents)
} EAX_PSHIFTER;

// Use this structure for EAX RING MODULATOR
typedef struct EAX_RMODULATOR
{
  S32 Effect;          // set to MSS_EAX_RMODULATOR
  S32 Volume;          // -10000 to 0
  F32 Frequency;       // Frequency of modulation (Hz)
  F32 HighPassCutOff;  // Cut-off frequency of high-pass filter (Hz)
  U32 Waveform;        // Waveform selector - 0 = sinusoid, 1 = triangle, 2 = sawtooth
} EAX_RMODULATOR;

#endif

#else

typedef struct _FLTPROVIDER
{
  U32 junk;
} FLTPROVIDER;

#endif

#if !defined(IS_PS2) && !defined(IS_PS3)

#if defined(_PUSHPOP_SUPPORTED) || PRAGMA_STRUCT_PACKPUSH
  #pragma pack(pop)
#else
  #pragma pack()
#endif

#endif

#ifdef IS_PS2

// round up to multiples of 16 for DMA alignment
#define SPR_IS_NEEDED  (((2 * 578         * sizeof(S16)) + 15) & ~15)  // 2320
#define SPR_XR_NEEDED  (((2 * 32 * 18     * sizeof(F32)) + 15) & ~15)  // 4608
#define SPR_LR_NEEDED  (((2 * 32 * 18     * sizeof(F32)) + 15) & ~15)  // 4608
#define SPR_RES_NEEDED (((32 * 18         * sizeof(F32)) + 15) & ~15)  // 2304
#define SPR_S_NEEDED   (((2 * 32 * 18     * sizeof(F32)) + 15) & ~15)  // 4608
#define SPR_U_NEEDED   (((2 * 2 * 17 * 16 * sizeof(F32)) + 15) & ~15)  // 4352

#define SPR_MEM        (0x70000000)
#define SPR_U_START    (SPR_MEM)
#define SPR_S_START    (SPR_U_START + SPR_U_NEEDED)
#define SPR_RES_START  (SPR_S_START + SPR_S_NEEDED)
#define SPR_LR_START   (SPR_RES_START + SPR_RES_NEEDED)
#define SPR_IS_START   (SPR_RES_START + SPR_RES_NEEDED)  // Shared with LR space (LR is larger)
#define END_SPR_MEM    (SPR_LR_START + SPR_LR_NEEDED)
#define SPR_BYTES_USED (END_SPR_MEM - SPR_MEM)

typedef enum
{
   MSS_DO_NOT_USE,
   MSS_USE_AND_SAVE,
   MSS_USE_WITHOUT_SAVING,
   MSS_USED = 100,               // Internal use only
}
MSS_RESOURCE_POLICY;

DXDEC MSS_RESOURCE_POLICY AILCALL AIL_set_scratchpad_policy(MSS_RESOURCE_POLICY policy);
DXDEC MSS_RESOURCE_POLICY AILCALL AIL_scratchpad_policy    (void);

#endif

#ifndef __RADINDLL__
#ifdef FSSpec
#undef FSSpec
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
