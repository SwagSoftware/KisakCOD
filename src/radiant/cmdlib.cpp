#ifndef KISAK_RADIANT
#error this file is only for Radiant!
#endif
// cod3src\libs\cmdlib\cmdlib.cpp

#include "stdafx.h"
#include <universal/assertive.h>

// ---------------------------------------------------------------------------
// Global console-output callback (set by the MFC app on startup via
// SetConsoleHandler).  Com_PrintMessage / Com_PrintError / R_Warn all funnel
// through this pointer so that output lands in the Radiant console window.
// ---------------------------------------------------------------------------
static void ( *console_stuff )( const char *fmt, va_list args ) = nullptr;

// Three additional printf-style callbacks that the app wires up; they are
// referenced by their setter / caller trampolines below.  In practice only
// console_stuff is actively set in CoD4Radiant; the others exist for
// compatibility with the BSP-tools heritage and are never called from the
// editor proper.
static void ( *s_qprintf_fn )( int level, const char *fmt, va_list args ) = nullptr;
static void ( *s_qerrprintf_fn )( int level, int line, const char *fmt, va_list args ) = nullptr;
static void ( *s_dprintf_fn )( int level, int line, const char *fmt, va_list args ) = nullptr;

// ---------------------------------------------------------------------------
// SetConsoleHandler / QPrintf / QErrPrintf / DPrintf — callback setters and
// callers.  These are the small stub functions at 0x40a9e0–0x40aa10.
// ---------------------------------------------------------------------------

// 0x40a9e0  SetConsoleHandler — installs the one-arg (fmt, va_list) printer.
void SetConsoleHandler( void ( *fn )( const char *, va_list ) )
{
    console_stuff = fn;
}

// 0x40a9f0  — installs the QPrintf handler.
void SetQPrintfHandler( void ( *fn )( int, const char *, va_list ) )
{
    s_qprintf_fn = fn;
}

// 0x40aa00  — installs the QErrPrintf handler.
void SetQErrPrintfHandler( void ( *fn )( int, int, const char *, va_list ) )
{
    s_qerrprintf_fn = fn;
}

// 0x40aa10  — installs the DPrintf handler.
void SetDPrintfHandler( void ( *fn )( int, int, const char *, va_list ) )
{
    s_dprintf_fn = fn;
}

// ---------------------------------------------------------------------------
// Com_PrintMessage (0x40a960) — forward through console_stuff.
// Called by SafeRead / SafeWrite on read/write count mismatch.
// ---------------------------------------------------------------------------
void Com_PrintMessage( const char *fmt, ... )
{
    if ( console_stuff )
    {
        va_list args;
        va_start( args, fmt );
        console_stuff( fmt, args );
        va_end( args );
    }
}

// ---------------------------------------------------------------------------
// R_Warn (0x40b630) — the editor's renderer-warning print.  Its IDB home is the
// engine print-shim cluster (Com_Printf 0x40b5d0 / Com_PrintError 0x40b610 /
// R_Warn 0x40b630), but it is defined HERE for the same reason Com_PrintMessage is:
// console_stuff is a file-static of this TU.  The binary IGNORES the warning-type
// argument entirely — it is a bare `if (console_stuff) console_stuff(fmt, va)` — so
// this is byte-faithful and NOT the game's R_WarnOncePerFrame throttling.
// Caller: VehiclePath_DrawPath (vehiclepath.cpp) on an infinite vehicle path.
// ---------------------------------------------------------------------------
void R_Warn( int warnType, const char *fmt, ... )
{
    (void)warnType;                  // faithful: the original never reads it
    if ( console_stuff )
    {
        va_list args;
        va_start( args, fmt );
        console_stuff( fmt, args );
        va_end( args );
    }
}

// 0x40a980  QPrintf trampoline.
void QPrintf( int level, const char *fmt, ... )
{
    if ( s_qprintf_fn )
    {
        va_list args;
        va_start( args, fmt );
        s_qprintf_fn( level, fmt, args );
        va_end( args );
    }
}

// 0x40a9a0  QErrPrintf trampoline.
void QErrPrintf( int level, int line, const char *fmt, ... )
{
    if ( s_qerrprintf_fn )
    {
        va_list args;
        va_start( args, fmt );
        s_qerrprintf_fn( level, line, fmt, args );
        va_end( args );
    }
}

// 0x40a9c0  DPrintf trampoline.
void DPrintf( int level, int line, const char *fmt, ... )
{
    if ( s_dprintf_fn )
    {
        va_list args;
        va_start( args, fmt );
        s_dprintf_fn( level, line, fmt, args );
        va_end( args );
    }
}

// ---------------------------------------------------------------------------
// qblockmalloc (0x40aa20) — page-aligned zeroing allocator.
// Rounds up to next 4096-byte boundary, malloc + memset.
// ---------------------------------------------------------------------------
#define MEM_BLOCKSIZE 4096
void *qblockmalloc( size_t nSize )
{
    int nAllocSize = (int)( nSize % MEM_BLOCKSIZE );
    if ( nAllocSize > 0 )
    {
        nSize += MEM_BLOCKSIZE - nAllocSize;
    }
    void *b = malloc( nSize );
    memset( b, 0, nSize );
    return b;
}

// ---------------------------------------------------------------------------
// Q_filelength (0x40aa80) — return file length without disturbing position.
// ---------------------------------------------------------------------------
int Q_filelength( FILE *f )
{
    int pos = ftell( f );
    fseek( f, 0, SEEK_END );
    int end = ftell( f );
    fseek( f, pos, SEEK_SET );
    return end;
}

// ---------------------------------------------------------------------------
// SafeRead (0x40ab30) — assert count > 0, fread, warn on short read.
// __usercall: count in ESI; FILE* and buffer on stack.
// Ported as normal cdecl.
// ---------------------------------------------------------------------------
void SafeRead( int count, FILE *f, void *buf )
{
    vassert( count > 0, "(count) = %i", count );

    size_t n = fread( buf, 1, (size_t)count, f );
    if ( (int)n != count )
    {
        Com_PrintMessage( "File read failure - read %i of %i bytes", (int)n, count );
    }
}

// ---------------------------------------------------------------------------
// SafeWrite (0x40ab80) — assert count >= 0, fwrite, warn on short write.
// __usercall: count in ESI; FILE* and buffer on stack.
// Ported as normal cdecl.
// ---------------------------------------------------------------------------
void SafeWrite( int count, FILE *f, const void *buf )
{
    vassert( count >= 0, "(count) = %i", count );

    size_t n = fwrite( buf, 1, (size_t)count, f );
    if ( (int)n != count )
    {
        Com_PrintMessage( "File write failure - wrote %i of %i bytes", (int)n, count );
    }
}

// ---------------------------------------------------------------------------
// LoadFile (0x40abd0) — load file into qblockmalloc'd buffer.
// Returns file size, or -1 if path is NULL/empty/not found.
// __usercall: path in EDX; buffer out-ptr on stack.
// ---------------------------------------------------------------------------
int LoadFile( const char *filename, void **bufferptr )
{
    *bufferptr = nullptr;

    if ( !filename || !*filename )
    {
        return -1;
    }

    FILE *f = fopen( filename, "rb" );
    if ( !f )
    {
        return -1;
    }

    int length = Q_filelength( f );
    char *buffer = (char *)qblockmalloc( (size_t)( length + 1 ) );
    buffer[length] = '\0';

    if ( length > 0 )
    {
        SafeRead( length, f, buffer );
    }

    fclose( f );
    *bufferptr = buffer;
    return length;
}

// ---------------------------------------------------------------------------
// LoadFileNoCrash (0x40ac60) — load file, return -1 (no assert) if missing.
// Uses raw malloc + memset instead of qblockmalloc.
// __usercall: path in EAX; buffer out-ptr on stack.
// ---------------------------------------------------------------------------
int LoadFileNoCrash( const char *filename, void **bufferptr )
{
    FILE *f = fopen( filename, "rb" );
    if ( !f )
    {
        return -1;
    }

    int pos = ftell( f );
    fseek( f, 0, SEEK_END );
    int length = ftell( f );
    fseek( f, pos, SEEK_SET );

    char *buffer = (char *)malloc( (size_t)( length + 2 ) );
    memset( buffer, 0, (size_t)( length + 1 ) );
    buffer[length] = '\0';

    SafeRead( length, f, buffer );
    fclose( f );

    *bufferptr = buffer;
    return length;
}

// ---------------------------------------------------------------------------
// DefaultExtension (0x40ad50) — append extension if path has none.
// Scans backwards; returns if '.' found before '/' (or start of string).
// __usercall: path in EAX; extension on stack.
// ---------------------------------------------------------------------------
void DefaultExtension( char *path, const char *extension )
{
    // Scan back from end looking for '.' or '/' (path separator).
    char *src = path + strlen( path ) - 1;

    while ( src != path && *src != '/' )
    {
        if ( *src == '.' )
        {
            return; // already has extension
        }
        src--;
    }

    // No extension found — append.
    strcat( path, extension );
}

// ---------------------------------------------------------------------------
// StripExtension (0x40ae90) — remove the extension from a path in-place.
// Scans backwards; truncates at '.' (stops at '/' — no extension case).
// __usercall: path in EDX (second register arg, first is unused).
// DIVERGENCE vs IDA (documented, port kept): the binary checks '.' at `result` but
// '/' at `result-1` (offset: `if(a2[--result]==47) return`); this port checks both
// at the same index. They differ ONLY for a path ENDING in '/' that contains a '.'
// (binary strips at the internal '.', e.g. "a.b/"->"a"; this leaves "a.b/"). That
// offset is an off-by-one quirk and StripExtension is only ever called on filenames
// (no trailing '/'), so the case is unreachable -- the standard no-strip-past-'/'
// behavior is kept rather than reproduce the quirk. (DefaultExtension 0x40ad50 has
// NO such offset -- its '/' check is at the current char, verified equivalent.)
// ---------------------------------------------------------------------------
void StripExtension( char *path )
{
    int length = (int)strlen( path ) - 1;

    while ( length > 0 )
    {
        if ( path[length] == '.' )
        {
            path[length] = '\0';
            return;
        }
        if ( path[length] == '/' )
        {
            return; // no extension
        }
        length--;
    }
}
