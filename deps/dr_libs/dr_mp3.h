/*
MP3 audio decoder. Choice of public domain or MIT-0. See license statements at the end of this file.
dr_mp3 - v0.7.4 - TBD

David Reid - mackron@gmail.com

GitHub: https://github.com/mackron/dr_libs

Based on minimp3 (https://github.com/lieff/minimp3) which is where the real work was done. See the bottom of this file for differences between minimp3 and dr_mp3.
*/

/*
Introduction
=============
dr_mp3 is a single file library. To use it, do something like the following in one .c file.

    ```c
    #define DR_MP3_IMPLEMENTATION
    #include "dr_mp3.h"
    ```

You can then #include this file in other parts of the program as you would with any other header file. To decode audio data, do something like the following:

    ```c
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, "MySong.mp3", NULL)) {
        // Failed to open file
    }

    ...

    drmp3_uint64 framesRead = drmp3_read_pcm_frames_f32(pMP3, framesToRead, pFrames);
    ```

The drmp3 object is transparent so you can get access to the channel count and sample rate like so:

    ```
    drmp3_uint32 channels = mp3.channels;
    drmp3_uint32 sampleRate = mp3.sampleRate;
    ```

The example above initializes a decoder from a file, but you can also initialize it from a block of memory and read and seek callbacks with
`drmp3_init_memory()` and `drmp3_init()` respectively.

You do not need to do any annoying memory management when reading PCM frames - this is all managed internally. You can request any number of PCM frames in each
call to `drmp3_read_pcm_frames_f32()` and it will return as many PCM frames as it can, up to the requested amount.

You can also decode an entire file in one go with `drmp3_open_and_read_pcm_frames_f32()`, `drmp3_open_memory_and_read_pcm_frames_f32()` and
`drmp3_open_file_and_read_pcm_frames_f32()`.


Build Options
=============
#define these options before including this file.

#define DR_MP3_NO_STDIO
  Disable drmp3_init_file(), etc.

#define DR_MP3_NO_SIMD
  Disable SIMD optimizations.
*/

#ifndef dr_mp3_h
#define dr_mp3_h

#ifdef __cplusplus
extern "C" {
#endif

#define DRMP3_STRINGIFY(x)      #x
#define DRMP3_XSTRINGIFY(x)     DRMP3_STRINGIFY(x)

#define DRMP3_VERSION_MAJOR     0
#define DRMP3_VERSION_MINOR     7
#define DRMP3_VERSION_REVISION  4
#define DRMP3_VERSION_STRING    DRMP3_XSTRINGIFY(DRMP3_VERSION_MAJOR) "." DRMP3_XSTRINGIFY(DRMP3_VERSION_MINOR) "." DRMP3_XSTRINGIFY(DRMP3_VERSION_REVISION)

#include <stddef.h> /* For size_t. */
#include <stdio.h>

/* Sized Types */
typedef   signed char           drmp3_int8;
typedef unsigned char           drmp3_uint8;
typedef   signed short          drmp3_int16;
typedef unsigned short          drmp3_uint16;
typedef   signed int            drmp3_int32;
typedef unsigned int            drmp3_uint32;
#if defined(_MSC_VER) && !defined(__clang__)
    typedef   signed __int64    drmp3_int64;
    typedef unsigned __int64    drmp3_uint64;
#else
    #if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wlong-long"
        #if defined(__clang__)
            #pragma GCC diagnostic ignored "-Wc++11-long-long"
        #endif
    #endif
    typedef   signed long long  drmp3_int64;
    typedef unsigned long long  drmp3_uint64;
    #if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
        #pragma GCC diagnostic pop
    #endif
#endif
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC) || defined(__powerpc64__)
    typedef drmp3_uint64        drmp3_uintptr;
#else
    typedef drmp3_uint32        drmp3_uintptr;
#endif
typedef drmp3_uint8             drmp3_bool8;
typedef drmp3_uint32            drmp3_bool32;
#define DRMP3_TRUE              1
#define DRMP3_FALSE             0

/* Weird shifting syntax is for VC6 compatibility. */
#define DRMP3_UINT64_MAX        (((drmp3_uint64)0xFFFFFFFF << 32) | (drmp3_uint64)0xFFFFFFFF)
/* End Sized Types */

/* Decorations */
#if !defined(DRMP3_API)
    #if defined(DRMP3_DLL)
        #if defined(_WIN32)
            #define DRMP3_DLL_IMPORT  __declspec(dllimport)
            #define DRMP3_DLL_EXPORT  __declspec(dllexport)
            #define DRMP3_DLL_PRIVATE static
        #else
            #if defined(__GNUC__) && __GNUC__ >= 4
                #define DRMP3_DLL_IMPORT  __attribute__((visibility("default")))
                #define DRMP3_DLL_EXPORT  __attribute__((visibility("default")))
                #define DRMP3_DLL_PRIVATE __attribute__((visibility("hidden")))
            #else
                #define DRMP3_DLL_IMPORT
                #define DRMP3_DLL_EXPORT
                #define DRMP3_DLL_PRIVATE static
            #endif
        #endif

        #if defined(DR_MP3_IMPLEMENTATION)
            #define DRMP3_API  DRMP3_DLL_EXPORT
        #else
            #define DRMP3_API  DRMP3_DLL_IMPORT
        #endif
        #define DRMP3_PRIVATE DRMP3_DLL_PRIVATE
    #else
        #define DRMP3_API extern
        #define DRMP3_PRIVATE static
    #endif
#endif
/* End Decorations */

/* Result Codes */
typedef drmp3_int32 drmp3_result;
#define DRMP3_SUCCESS                        0
#define DRMP3_ERROR                         -1   /* A generic error. */
#define DRMP3_INVALID_ARGS                  -2
#define DRMP3_INVALID_OPERATION             -3
#define DRMP3_OUT_OF_MEMORY                 -4
#define DRMP3_OUT_OF_RANGE                  -5
#define DRMP3_ACCESS_DENIED                 -6
#define DRMP3_DOES_NOT_EXIST                -7
#define DRMP3_ALREADY_EXISTS                -8
#define DRMP3_TOO_MANY_OPEN_FILES           -9
#define DRMP3_INVALID_FILE                  -10
#define DRMP3_TOO_BIG                       -11
#define DRMP3_PATH_TOO_LONG                 -12
#define DRMP3_NAME_TOO_LONG                 -13
#define DRMP3_NOT_DIRECTORY                 -14
#define DRMP3_IS_DIRECTORY                  -15
#define DRMP3_DIRECTORY_NOT_EMPTY           -16
#define DRMP3_END_OF_FILE                   -17
#define DRMP3_NO_SPACE                      -18
#define DRMP3_BUSY                          -19
#define DRMP3_IO_ERROR                      -20
#define DRMP3_INTERRUPT                     -21
#define DRMP3_UNAVAILABLE                   -22
#define DRMP3_ALREADY_IN_USE                -23
#define DRMP3_BAD_ADDRESS                   -24
#define DRMP3_BAD_SEEK                      -25
#define DRMP3_BAD_PIPE                      -26
#define DRMP3_DEADLOCK                      -27
#define DRMP3_TOO_MANY_LINKS                -28
#define DRMP3_NOT_IMPLEMENTED               -29
#define DRMP3_NO_MESSAGE                    -30
#define DRMP3_BAD_MESSAGE                   -31
#define DRMP3_NO_DATA_AVAILABLE             -32
#define DRMP3_INVALID_DATA                  -33
#define DRMP3_TIMEOUT                       -34
#define DRMP3_NO_NETWORK                    -35
#define DRMP3_NOT_UNIQUE                    -36
#define DRMP3_NOT_SOCKET                    -37
#define DRMP3_NO_ADDRESS                    -38
#define DRMP3_BAD_PROTOCOL                  -39
#define DRMP3_PROTOCOL_UNAVAILABLE          -40
#define DRMP3_PROTOCOL_NOT_SUPPORTED        -41
#define DRMP3_PROTOCOL_FAMILY_NOT_SUPPORTED -42
#define DRMP3_ADDRESS_FAMILY_NOT_SUPPORTED  -43
#define DRMP3_SOCKET_NOT_SUPPORTED          -44
#define DRMP3_CONNECTION_RESET              -45
#define DRMP3_ALREADY_CONNECTED             -46
#define DRMP3_NOT_CONNECTED                 -47
#define DRMP3_CONNECTION_REFUSED            -48
#define DRMP3_NO_HOST                       -49
#define DRMP3_IN_PROGRESS                   -50
#define DRMP3_CANCELLED                     -51
#define DRMP3_MEMORY_ALREADY_MAPPED         -52
#define DRMP3_AT_END                        -53
/* End Result Codes */

#define DRMP3_MAX_PCM_FRAMES_PER_MP3_FRAME  1152
#define DRMP3_MAX_SAMPLES_PER_FRAME         (DRMP3_MAX_PCM_FRAMES_PER_MP3_FRAME*2)

/* Inline */
#ifdef _MSC_VER
    #define DRMP3_INLINE __forceinline
#elif defined(__GNUC__)
    /*
    I've had a bug report where GCC is emitting warnings about functions possibly not being inlineable. This warning happens when
    the __attribute__((always_inline)) attribute is defined without an "inline" statement. I think therefore there must be some
    case where "__inline__" is not always defined, thus the compiler emitting these warnings. When using -std=c89 or -ansi on the
    command line, we cannot use the "inline" keyword and instead need to use "__inline__". In an attempt to work around this issue
    I am using "__inline__" only when we're compiling in strict ANSI mode.
    */
    #if defined(__STRICT_ANSI__)
        #define DRMP3_GNUC_INLINE_HINT __inline__
    #else
        #define DRMP3_GNUC_INLINE_HINT inline
    #endif

    #if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 2)) || defined(__clang__)
        #define DRMP3_INLINE DRMP3_GNUC_INLINE_HINT __attribute__((always_inline))
    #else
        #define DRMP3_INLINE DRMP3_GNUC_INLINE_HINT
    #endif
#elif defined(__WATCOMC__)
    #define DRMP3_INLINE __inline
#else
    #define DRMP3_INLINE
#endif
/* End Inline */


DRMP3_API void drmp3_version(drmp3_uint32* pMajor, drmp3_uint32* pMinor, drmp3_uint32* pRevision);
DRMP3_API const char* drmp3_version_string(void);


/* Allocation Callbacks */
typedef struct
{
    void* pUserData;
    void* (* onMalloc)(size_t sz, void* pUserData);
    void* (* onRealloc)(void* p, size_t sz, void* pUserData);
    void  (* onFree)(void* p, void* pUserData);
} drmp3_allocation_callbacks;
/* End Allocation Callbacks */


/*
Low Level Push API
==================
*/
#define DRMP3_MAX_BITRESERVOIR_BYTES      511
#define DRMP3_MAX_FREE_FORMAT_FRAME_SIZE  2304    /* more than ISO spec's */
#define DRMP3_MAX_L3_FRAME_PAYLOAD_BYTES  DRMP3_MAX_FREE_FORMAT_FRAME_SIZE /* MUST be >= 320000/8/32000*1152 = 1440 */

typedef struct
{
    int frame_bytes, channels, sample_rate, layer, bitrate_kbps;
} drmp3dec_frame_info;

typedef struct
{
    const drmp3_uint8 *buf;
    int pos, limit;
} drmp3_bs;

typedef struct
{
    const drmp3_uint8 *sfbtab;
    drmp3_uint16 part_23_length, big_values, scalefac_compress;
    drmp3_uint8 global_gain, block_type, mixed_block_flag, n_long_sfb, n_short_sfb;
    drmp3_uint8 table_select[3], region_count[3], subblock_gain[3];
    drmp3_uint8 preflag, scalefac_scale, count1_table, scfsi;
} drmp3_L3_gr_info;

typedef struct
{
    drmp3_bs bs;
    drmp3_uint8 maindata[DRMP3_MAX_BITRESERVOIR_BYTES + DRMP3_MAX_L3_FRAME_PAYLOAD_BYTES];
    drmp3_L3_gr_info gr_info[4];
    float grbuf[2][576], scf[40], syn[18 + 15][2*32];
    drmp3_uint8 ist_pos[2][39];
} drmp3dec_scratch;

typedef struct
{
    float mdct_overlap[2][9*32], qmf_state[15*2*32];
    int reserv, free_format_bytes;
    drmp3_uint8 header[4], reserv_buf[511];
    drmp3dec_scratch scratch;
} drmp3dec;

/* Initializes a low level decoder. */
DRMP3_API void drmp3dec_init(drmp3dec *dec);

/* Reads a frame from a low level decoder. */
DRMP3_API int drmp3dec_decode_frame(drmp3dec *dec, const drmp3_uint8 *mp3, int mp3_bytes, void *pcm, drmp3dec_frame_info *info);

/* Helper for converting between f32 and s16. */
DRMP3_API void drmp3dec_f32_to_s16(const float *in, drmp3_int16 *out, size_t num_samples);



/*
Main API (Pull API)
===================
*/
typedef enum
{
    DRMP3_SEEK_SET,
    DRMP3_SEEK_CUR,
    DRMP3_SEEK_END
} drmp3_seek_origin;

typedef struct
{
    drmp3_uint64 seekPosInBytes;        /* Points to the first byte of an MP3 frame. */
    drmp3_uint64 pcmFrameIndex;         /* The index of the PCM frame this seek point targets. */
    drmp3_uint16 mp3FramesToDiscard;    /* The number of whole MP3 frames to be discarded before pcmFramesToDiscard. */
    drmp3_uint16 pcmFramesToDiscard;    /* The number of leading samples to read and discard. These are discarded after mp3FramesToDiscard. */
} drmp3_seek_point;

typedef enum
{
    DRMP3_METADATA_TYPE_ID3V1,
    DRMP3_METADATA_TYPE_ID3V2,
    DRMP3_METADATA_TYPE_APE,
    DRMP3_METADATA_TYPE_XING,
    DRMP3_METADATA_TYPE_VBRI
} drmp3_metadata_type;

typedef struct
{
    drmp3_metadata_type type;
    const void* pRawData;               /* A pointer to the raw data. */
    size_t rawDataSize;
} drmp3_metadata;


/*
Callback for when data is read. Return value is the number of bytes actually read.

pUserData   [in]  The user data that was passed to drmp3_init(), and family.
pBufferOut  [out] The output buffer.
bytesToRead [in]  The number of bytes to read.

Returns the number of bytes actually read.

A return value of less than bytesToRead indicates the end of the stream. Do _not_ return from this callback until
either the entire bytesToRead is filled or you have reached the end of the stream.
*/
typedef size_t (* drmp3_read_proc)(void* pUserData, void* pBufferOut, size_t bytesToRead);

/*
Callback for when data needs to be seeked.

pUserData [in] The user data that was passed to drmp3_init(), and family.
offset    [in] The number of bytes to move, relative to the origin. Can be negative.
origin    [in] The origin of the seek.

Returns whether or not the seek was successful.
*/
typedef drmp3_bool32 (* drmp3_seek_proc)(void* pUserData, int offset, drmp3_seek_origin origin);

/*
Callback for retrieving the current cursor position.

pUserData [in]  The user data that was passed to drmp3_init(), and family.
pCursor   [out] The cursor position in bytes from the start of the stream.

Returns whether or not the cursor position was successfully retrieved.
*/
typedef drmp3_bool32 (* drmp3_tell_proc)(void* pUserData, drmp3_int64* pCursor);


/*
Callback for when metadata is read.

Only the raw data is provided. The client is responsible for parsing the contents of the data themsevles.
*/
typedef void (* drmp3_meta_proc)(void* pUserData, const drmp3_metadata* pMetadata);


typedef struct
{
    drmp3_uint32 channels;
    drmp3_uint32 sampleRate;
} drmp3_config;

typedef struct
{
    drmp3dec decoder;
    drmp3_uint32 channels;
    drmp3_uint32 sampleRate;
    drmp3_read_proc onRead;
    drmp3_seek_proc onSeek;
    drmp3_meta_proc onMeta;
    void* pUserData;
    void* pUserDataMeta;
    drmp3_allocation_callbacks allocationCallbacks;
    drmp3_uint32 mp3FrameChannels;      /* The number of channels in the currently loaded MP3 frame. Internal use only. */
    drmp3_uint32 mp3FrameSampleRate;    /* The sample rate of the currently loaded MP3 frame. Internal use only. */
    drmp3_uint32 pcmFramesConsumedInMP3Frame;
    drmp3_uint32 pcmFramesRemainingInMP3Frame;
    drmp3_uint8 pcmFrames[sizeof(float)*DRMP3_MAX_SAMPLES_PER_FRAME];  /* <-- Multipled by sizeof(float) to ensure there's enough room for DR_MP3_FLOAT_OUTPUT. */
    drmp3_uint64 currentPCMFrame;       /* The current PCM frame, globally. */
    drmp3_uint64 streamCursor;          /* The current byte the decoder is sitting on in the raw stream. */
    drmp3_uint64 streamLength;          /* The length of the stream in bytes. dr_mp3 will not read beyond this. If a ID3v1 or APE tag is present, this will be set to the first byte of the tag. */
    drmp3_uint64 streamStartOffset;     /* The offset of the start of the MP3 data. This is used for skipping ID3v2 and VBR tags. */
    drmp3_seek_point* pSeekPoints;      /* NULL by default. Set with drmp3_bind_seek_table(). Memory is owned by the client. dr_mp3 will never attempt to free this pointer. */
    drmp3_uint32 seekPointCount;        /* The number of items in pSeekPoints. When set to 0 assumes to no seek table. Defaults to zero. */
    drmp3_uint32 delayInPCMFrames;
    drmp3_uint32 paddingInPCMFrames;
    drmp3_uint64 totalPCMFrameCount;    /* Set to DRMP3_UINT64_MAX if the length is unknown. Includes delay and padding. */
    drmp3_bool32 isVBR;
    drmp3_bool32 isCBR;
    size_t dataSize;
    size_t dataCapacity;
    size_t dataConsumed;
    drmp3_uint8* pData;
    drmp3_bool32 atEnd;
    struct
    {
        const drmp3_uint8* pData;
        size_t dataSize;
        size_t currentReadPos;
    } memory;   /* Only used for decoders that were opened against a block of memory. */
} drmp3;

/*
Initializes an MP3 decoder.

onRead    [in]           The function to call when data needs to be read from the client.
onSeek    [in]           The function to call when the read position of the client data needs to move.
onTell    [in]           The function to call when the read position of the client data needs to be retrieved.
pUserData [in, optional] A pointer to application defined data that will be passed to onRead and onSeek.

Returns true if successful; false otherwise.

Close the loader with drmp3_uninit().

See also: drmp3_init_file(), drmp3_init_memory(), drmp3_uninit()
*/
DRMP3_API drmp3_bool32 drmp3_init(drmp3* pMP3, drmp3_read_proc onRead, drmp3_seek_proc onSeek, drmp3_tell_proc onTell, drmp3_meta_proc onMeta, void* pUserData, const drmp3_allocation_callbacks* pAllocationCallbacks);

/*
Initializes an MP3 decoder from a block of memory.

This does not create a copy of the data. It is up to the application to ensure the buffer remains valid for
the lifetime of the drmp3 object.

The buffer should contain the contents of the entire MP3 file.
*/
DRMP3_API drmp3_bool32 drmp3_init_memory_with_metadata(drmp3* pMP3, const void* pData, size_t dataSize, drmp3_meta_proc onMeta, void* pUserDataMeta, const drmp3_allocation_callbacks* pAllocationCallbacks);
DRMP3_API drmp3_bool32 drmp3_init_memory(drmp3* pMP3, const void* pData, size_t dataSize, const drmp3_allocation_callbacks* pAllocationCallbacks);

#ifndef DR_MP3_NO_STDIO
/*
Initializes an MP3 decoder from a file.

This holds the internal FILE object until drmp3_uninit() is called. Keep this in mind if you're caching drmp3
objects because the operating system may restrict the number of file handles an application can have open at
any given time.
*/
DRMP3_API drmp3_bool32 drmp3_init_file_with_metadata(drmp3* pMP3, const char* pFilePath, drmp3_meta_proc onMeta, void* pUserDataMeta, const drmp3_allocation_callbacks* pAllocationCallbacks);
DRMP3_API drmp3_bool32 drmp3_init_file_with_metadata2(drmp3* pMP3, FILE* pFile, drmp3_meta_proc onMeta, void* pUserDataMeta, const drmp3_allocation_callbacks* pAllocationCallbacks);
DRMP3_API drmp3_bool32 drmp3_init_file_with_metadata_w(drmp3* pMP3, const wchar_t* pFilePath, drmp3_meta_proc onMeta, void* pUserDataMeta, const drmp3_allocation_callbacks* pAllocationCallbacks);

DRMP3_API drmp3_bool32 drmp3_init_file(drmp3* pMP3, const char* pFilePath, const drmp3_allocation_callbacks* pAllocationCallbacks);
DRMP3_API drmp3_bool32 drmp3_init_file2(drmp3* pMP3, FILE* pFile, const drmp3_allocation_callbacks* pAllocationCallbacks);
DRMP3_API drmp3_bool32 drmp3_init_file_w(drmp3* pMP3, const wchar_t* pFilePath, const drmp3_allocation_callbacks* pAllocationCallbacks);
#endif

/*
Uninitializes an MP3 decoder.
*/
DRMP3_API void drmp3_uninit(drmp3* pMP3);

/*
Reads PCM frames as interleaved 32-bit IEEE floating point PCM.

Note that framesToRead specifies the number of PCM frames to read, _not_ the number of MP3 frames.
*/
DRMP3_API drmp3_uint64 drmp3_read_pcm_frames_f32(drmp3* pMP3, drmp3_uint64 framesToRead, float* pBufferOut);

/*
Reads PCM frames as interleaved signed 16-bit integer PCM.

Note that framesToRead specifies the number of PCM frames to read, _not_ the number of MP3 frames.
*/
DRMP3_API drmp3_uint64 drmp3_read_pcm_frames_s16(drmp3* pMP3, drmp3_uint64 framesToRead, drmp3_int16* pBufferOut);

/*
Seeks to a specific frame.

Note that this is _not_ an MP3 frame, but rather a PCM frame.
*/
DRMP3_API drmp3_bool32 drmp3_seek_to_pcm_frame(drmp3* pMP3, drmp3_uint64 frameIndex);

/*
Calculates the total number of PCM frames in the MP3 stream. Cannot be used for infinite streams such as internet
radio. Runs in linear time. Returns 0 on error.
*/
DRMP3_API drmp3_uint64 drmp3_get_pcm_frame_count(drmp3* pMP3);

/*
Calculates the total number of MP3 frames in the MP3 stream. Cannot be used for infinite streams such as internet
radio. Runs in linear time. Returns 0 on error.
*/
DRMP3_API drmp3_uint64 drmp3_get_mp3_frame_count(drmp3* pMP3);

/*
Calculates the total number of MP3 and PCM frames in the MP3 stream. Cannot be used for infinite streams such as internet
radio. Runs in linear time. Returns 0 on error.

This is equivalent to calling drmp3_get_mp3_frame_count() and drmp3_get_pcm_frame_count() except that it's more efficient.
*/
DRMP3_API drmp3_bool32 drmp3_get_mp3_and_pcm_frame_count(drmp3* pMP3, drmp3_uint64* pMP3FrameCount, drmp3_uint64* pPCMFrameCount);

/*
Calculates the seekpoints based on PCM frames. This is slow.

pSeekpoint count is a pointer to a uint32 containing the seekpoint count. On input it contains the desired count.
On output it contains the actual count. The reason for this design is that the client may request too many
seekpoints, in which case dr_mp3 will return a corrected count.

Note that seektable seeking is not quite sample exact when the MP3 stream contains inconsistent sample rates.
*/
DRMP3_API drmp3_bool32 drmp3_calculate_seek_points(drmp3* pMP3, drmp3_uint32* pSeekPointCount, drmp3_seek_point* pSeekPoints);

/*
Binds a seek table to the decoder.

This does _not_ make a copy of pSeekPoints - it only references it. It is up to the application to ensure this
remains valid while it is bound to the decoder.

Use drmp3_calculate_seek_points() to calculate the seek points.
*/
DRMP3_API drmp3_bool32 drmp3_bind_seek_table(drmp3* pMP3, drmp3_uint32 seekPointCount, drmp3_seek_point* pSeekPoints);


/*
Opens an decodes an entire MP3 stream as a single operation.

On output pConfig will receive the channel count and sample rate of the stream.

Free the returned pointer with drmp3_free().
*/
DRMP3_API float* drmp3_open_and_read_pcm_frames_f32(drmp3_read_proc onRead, drmp3_seek_proc onSeek, drmp3_tell_proc onTell, void* pUserData, drmp3_config* pConfig, drmp3_uint64* pTotalFrameCount, const drmp3_allocation_callbacks* pAllocationCallbacks);
DRMP3_API drmp3_int16* drmp3_open_and_read_pcm_frames_s16(drmp3_read_proc onRead, drmp3_seek_proc onSeek, drmp3_tell_proc onTell, void* pUserData, drmp3_config* pConfig, drmp3_uint64* pTotalFrameCount, const drmp3_allocation_callbacks* pAllocationCallbacks);

DRMP3_API float* drmp3_open_memory_and_read_pcm_frames_f32(const void* pData, size_t dataSize, drmp3_config* pConfig, drmp3_uint64* pTotalFrameCount, const drmp3_allocation_callbacks* pAllocationCallbacks);
DRMP3_API drmp3_int16* drmp3_open_memory_and_read_pcm_frames_s16(const void* pData, size_t dataSize, drmp3_config* pConfig, drmp3_uint64* pTotalFrameCount, const drmp3_allocation_callbacks* pAllocationCallbacks);

#ifndef DR_MP3_NO_STDIO
DRMP3_API float* drmp3_open_file_and_read_pcm_frames_f32(const char* filePath, drmp3_config* pConfig, drmp3_uint64* pTotalFrameCount, const drmp3_allocation_callbacks* pAllocationCallbacks);
DRMP3_API drmp3_int16* drmp3_open_file_and_read_pcm_frames_s16(const char* filePath, drmp3_config* pConfig, drmp3_uint64* pTotalFrameCount, const drmp3_allocation_callbacks* pAllocationCallbacks);
#endif

/*
Allocates a block of memory on the heap.
*/
DRMP3_API void* drmp3_malloc(size_t sz, const drmp3_allocation_callbacks* pAllocationCallbacks);

/*
Frees any memory that was allocated by a public drmp3 API.
*/
DRMP3_API void drmp3_free(void* p, const drmp3_allocation_callbacks* pAllocationCallbacks);

#ifdef __cplusplus
}
#endif
#endif  /* dr_mp3_h */
