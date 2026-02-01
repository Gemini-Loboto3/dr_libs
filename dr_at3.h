/*
 * dr_at3.h - ATRAC3/ATRAC3+ Decoder - Single Header Library
 * 
 * Ported from FFmpeg via PPSSPP's standalone extraction.
 *
 * Copyright (c) 2010-2013 Maxim Poliakovski (ATRAC3+)
 * Copyright (c) 2002 The FFmpeg Project (ATRAC3)
 * Copyright (c) 2024 PPSSPP Project (standalone extraction)
 *
 * Decoder: LGPL v2.1 | Container parser & wrapper: Public domain
 *
 * USAGE
 *
 * This is a single-file library. To use it, do something like the following in one .c file:
 *
 *     #define DR_AT3_IMPLEMENTATION
 *     #include "dr_at3.h"
 *
 * You can then #include this file in other parts of the program as you would with any other header.
 *
 * QUICK START - Simple API (decode entire file):
 *
 *     drat3_uint32 channels, sampleRate;
 *     drat3_uint64 totalFrames;
 *     float* samples = drat3_open_file_and_read_pcm_frames_f32("file.at3", &channels, &sampleRate, &totalFrames, NULL);
 *     if (samples) {
 *         // Use samples...
 *         drat3_free(samples, NULL);
 *     }
 *
 * QUICK START - Streaming API (decode frame by frame):
 *
 *     drat3 at3;
 *     if (drat3_init_file("file.at3", NULL, &at3) == DRAT3_SUCCESS) {
 *         float buffer[4096];
 *         drat3_uint64 framesRead;
 *         while ((framesRead = drat3_read_pcm_frames_f32(&at3, 1024, buffer)) > 0) {
 *             // Process samples...
 *         }
 *         drat3_uninit(&at3);
 *     }
 *
 * SUPPORTED FORMATS
 *
 *   - ATRAC3 in RIFF WAV container (.at3, .wav)
 *   - ATRAC3+ in RIFF WAV container (.at3, .wav)
 *   - ATRAC3/3+ in OMA container (.oma, .aa3)
 *
 * OPTIONS
 *
 *   #define DR_AT3_NO_STDIO
 *     Disables file I/O APIs (drat3_init_file, drat3_open_file_and_read_pcm_frames_*).
 */

#ifndef DR_AT3_H
#define DR_AT3_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* Container Parser API */
typedef enum { DRAT3_CONTAINER_UNKNOWN = 0, DRAT3_CONTAINER_RIFF_WAV, DRAT3_CONTAINER_OMA } drat3_container_type;
typedef enum { DRAT3_CODEC_UNKNOWN = 0, DRAT3_CODEC_ATRAC3 = 0x0270, DRAT3_CODEC_ATRAC3P = 0x0E23 } drat3_codec_type;

typedef struct {
    drat3_container_type container_type;
    drat3_codec_type codec_type;
    uint32_t sample_rate;
    uint16_t channels;
    uint16_t block_align;
    uint32_t bit_rate;
    uint8_t joint_stereo;
    uint8_t extradata[14];
    int extradata_size;
    uint64_t data_offset;
    uint64_t data_size;
    uint64_t total_frames;
    uint32_t samples_per_frame;
    uint64_t total_samples;
} drat3_container_info;

typedef struct drat3_container drat3_container;

drat3_container* drat3_container_open_file(const char* filename);
drat3_container* drat3_container_open_memory(const void* data, size_t size);
void drat3_container_close(drat3_container* container);
const drat3_container_info* drat3_container_get_info(const drat3_container* container);
int drat3_container_read_frame(drat3_container* container, void* buffer, size_t buffer_size);
int drat3_container_seek_frame(drat3_container* container, uint64_t frame_index);
uint64_t drat3_container_get_frame_index(const drat3_container* container);
drat3_container_type drat3_container_probe(const void* data, size_t size);

/* Decoder API */
struct ATRAC3Context;
struct ATRAC3PContext;

struct ATRAC3Context *atrac3_alloc(int channels, int *block_align, const uint8_t *extra_data, int extra_data_size);
void atrac3_free(struct ATRAC3Context *ctx);
void atrac3_flush_buffers(struct ATRAC3Context *ctx);
int atrac3_decode_frame(struct ATRAC3Context *ctx, float *out_data[2], int *nb_samples, const uint8_t *buf, int buf_size);

struct ATRAC3PContext *atrac3p_alloc(int channels, int *block_align);
void atrac3p_free(struct ATRAC3PContext *ctx);
void atrac3p_flush_buffers(struct ATRAC3PContext *ctx);
int atrac3p_decode_frame(struct ATRAC3PContext *ctx, float *out_data[2], int *nb_samples, const uint8_t *buf, int buf_size);

/*******************************************************************************
 * dr_libs Style High-Level API
 ******************************************************************************/

#define DRAT3_VERSION_MAJOR     0
#define DRAT3_VERSION_MINOR     1
#define DRAT3_VERSION_REVISION  0
#define DRAT3_VERSION_STRING    "0.1.0"

/* Sized types */
typedef signed char        drat3_int8;
typedef unsigned char      drat3_uint8;
typedef signed short       drat3_int16;
typedef unsigned short     drat3_uint16;
typedef signed int         drat3_int32;
typedef unsigned int       drat3_uint32;
#if defined(_MSC_VER) && !defined(__clang__)
    typedef signed   __int64 drat3_int64;
    typedef unsigned __int64 drat3_uint64;
#else
    #if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wlong-long"
        #if defined(__clang__)
            #pragma GCC diagnostic ignored "-Wc++11-long-long"
        #endif
    #endif
    typedef signed   long long drat3_int64;
    typedef unsigned long long drat3_uint64;
    #if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)))
        #pragma GCC diagnostic pop
    #endif
#endif
typedef drat3_uint8 drat3_bool8;
typedef drat3_uint32 drat3_bool32;
#define DRAT3_TRUE  1
#define DRAT3_FALSE 0

#if !defined(DRAT3_API)
    #if defined(DR_AT3_DLL)
        #if defined(_WIN32)
            #define DRAT3_DLL_IMPORT  __declspec(dllimport)
            #define DRAT3_DLL_EXPORT  __declspec(dllexport)
            #define DRAT3_DLL_PRIVATE static
        #else
            #if defined(__GNUC__) && __GNUC__ >= 4
                #define DRAT3_DLL_IMPORT  __attribute__((visibility("default")))
                #define DRAT3_DLL_EXPORT  __attribute__((visibility("default")))
                #define DRAT3_DLL_PRIVATE __attribute__((visibility("hidden")))
            #else
                #define DRAT3_DLL_IMPORT
                #define DRAT3_DLL_EXPORT
                #define DRAT3_DLL_PRIVATE static
            #endif
        #endif
        #if defined(DR_AT3_IMPLEMENTATION)
            #define DRAT3_API  DRAT3_DLL_EXPORT
        #else
            #define DRAT3_API  DRAT3_DLL_IMPORT
        #endif
    #else
        #define DRAT3_API extern
    #endif
#endif

/* Result codes */
typedef drat3_int32 drat3_result;
#define DRAT3_SUCCESS               0
#define DRAT3_ERROR                -1
#define DRAT3_INVALID_ARGS         -2
#define DRAT3_INVALID_OPERATION    -3
#define DRAT3_OUT_OF_MEMORY        -4
#define DRAT3_OUT_OF_RANGE         -5
#define DRAT3_INVALID_FILE         -10
#define DRAT3_AT_END               -17
#define DRAT3_BAD_SEEK             -25
#define DRAT3_NOT_IMPLEMENTED      -29

/* Seek origins */
typedef enum {
    drat3_seek_origin_start,
    drat3_seek_origin_current
} drat3_seek_origin;

/* Allocation callbacks */
typedef struct {
    void* pUserData;
    void* (* onMalloc)(size_t sz, void* pUserData);
    void* (* onRealloc)(void* p, size_t sz, void* pUserData);
    void  (* onFree)(void* p, void* pUserData);
} drat3_allocation_callbacks;

/* Config */
typedef struct {
    drat3_allocation_callbacks allocationCallbacks;
} drat3_config;

/* Main decoder structure */
typedef struct drat3 {
    drat3_uint32 channels;
    drat3_uint32 sampleRate;
    drat3_uint64 totalPCMFrameCount;
    drat3_uint64 currentPCMFrame;
    drat3_codec_type codecType;
    
    /* Internal state */
    drat3_container* pContainer;
    void* pDecoder;                     /* ATRAC3Context or ATRAC3PContext */
    drat3_uint8* pFrameBuffer;          /* Buffer for encoded frame data */
    drat3_uint32 frameBufferSize;
    float* pDecodeBuffer;               /* Interleaved decode buffer */
    drat3_uint32 decodeBufferCapacity;  /* In PCM frames */
    drat3_uint32 leftoverFrames;        /* Leftover frames from previous decode */
    drat3_uint32 leftoverOffset;        /* Offset into decode buffer */
    drat3_allocation_callbacks allocationCallbacks;
} drat3;

/*
 * Low-Level API
 */

#ifndef DR_AT3_NO_STDIO
/* Initialize from file path. */
DRAT3_API drat3_result drat3_init_file(const char* pFilePath, const drat3_config* pConfig, drat3* pAt3);
#endif

/* Initialize from memory block. Data must remain valid for decoder lifetime. */
DRAT3_API drat3_result drat3_init_memory(const void* pData, size_t dataSize, const drat3_config* pConfig, drat3* pAt3);

/* Uninitialize decoder. */
DRAT3_API void drat3_uninit(drat3* pAt3);

/* Read PCM frames as 32-bit float interleaved. Returns frames actually read. */
DRAT3_API drat3_uint64 drat3_read_pcm_frames_f32(drat3* pAt3, drat3_uint64 framesToRead, float* pBufferOut);

/* Read PCM frames as signed 16-bit interleaved. Returns frames actually read. */
DRAT3_API drat3_uint64 drat3_read_pcm_frames_s16(drat3* pAt3, drat3_uint64 framesToRead, drat3_int16* pBufferOut);

/* Seek to specific PCM frame. Returns DRAT3_SUCCESS on success. */
DRAT3_API drat3_result drat3_seek_to_pcm_frame(drat3* pAt3, drat3_uint64 targetPCMFrameIndex);

/* Get current position in PCM frames. */
DRAT3_API drat3_uint64 drat3_get_cursor_in_pcm_frames(drat3* pAt3);

/* Get total length in PCM frames. */
DRAT3_API drat3_uint64 drat3_get_length_in_pcm_frames(drat3* pAt3);

/*
 * High-Level API
 */

#ifndef DR_AT3_NO_STDIO
/* Open file and read entire contents. Free with drat3_free(). */
DRAT3_API float* drat3_open_file_and_read_pcm_frames_f32(const char* pFilePath, drat3_uint32* pChannels, drat3_uint32* pSampleRate, drat3_uint64* pTotalPCMFrameCount, const drat3_allocation_callbacks* pAllocationCallbacks);
DRAT3_API drat3_int16* drat3_open_file_and_read_pcm_frames_s16(const char* pFilePath, drat3_uint32* pChannels, drat3_uint32* pSampleRate, drat3_uint64* pTotalPCMFrameCount, const drat3_allocation_callbacks* pAllocationCallbacks);
#endif

/* Open memory and read entire contents. Free with drat3_free(). */
DRAT3_API float* drat3_open_memory_and_read_pcm_frames_f32(const void* pData, size_t dataSize, drat3_uint32* pChannels, drat3_uint32* pSampleRate, drat3_uint64* pTotalPCMFrameCount, const drat3_allocation_callbacks* pAllocationCallbacks);
DRAT3_API drat3_int16* drat3_open_memory_and_read_pcm_frames_s16(const void* pData, size_t dataSize, drat3_uint32* pChannels, drat3_uint32* pSampleRate, drat3_uint64* pTotalPCMFrameCount, const drat3_allocation_callbacks* pAllocationCallbacks);

/* Free memory allocated by drat3. */
DRAT3_API void drat3_free(void* p, const drat3_allocation_callbacks* pAllocationCallbacks);

#ifdef __cplusplus
}
#endif

#ifdef DR_AT3_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <limits.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752440f
#endif

/* Container Implementation */
static inline uint16_t drat3c_rl16(const uint8_t* p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static inline uint32_t drat3c_rl32(const uint8_t* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
static inline uint32_t drat3c_rb24(const uint8_t* p) { return ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | (uint32_t)p[2]; }
static inline int drat3c_4cc(const uint8_t* p, const char* c) { return p[0]==c[0] && p[1]==c[1] && p[2]==c[2] && p[3]==c[3]; }
static const uint16_t drat3c_sr[8] = { 320, 441, 480, 882, 960, 0, 0, 0 };

struct drat3_container { drat3_container_info info; FILE* file; const uint8_t* mem; size_t msz, mpos; uint64_t frame; int eof; };

static size_t drat3c_rd(drat3_container* c, void* b, size_t n) {
    if (c->file) return fread(b, 1, n, c->file);
    if (c->mem) { size_t a = c->msz - c->mpos; if (n > a) n = a; memcpy(b, c->mem + c->mpos, n); c->mpos += n; return n; }
    return 0;
}
static int drat3c_sk(drat3_container* c, int64_t o, int w) {
    if (c->file) return fseek(c->file, (long)o, w);
    if (c->mem) { int64_t p; if (w==SEEK_SET) p=o; else if (w==SEEK_CUR) p=c->mpos+o; else p=c->msz+o; if (p<0||p>(int64_t)c->msz) return -1; c->mpos=(size_t)p; return 0; }
    return -1;
}
static int64_t drat3c_tl(drat3_container* c) { if (c->file) return ftell(c->file); if (c->mem) return (int64_t)c->mpos; return -1; }

static int drat3c_riff(drat3_container* c) {
    uint8_t h[12], ch[8], f[40]; int ff=0, fd=0;
    if (drat3c_rd(c,h,12)!=12 || !drat3c_4cc(h,"RIFF") || !drat3c_4cc(h+8,"WAVE")) return -1;
    while (!fd) {
        if (drat3c_rd(c,ch,8)!=8) break;
        uint32_t sz = drat3c_rl32(ch+4);
        if (drat3c_4cc(ch,"fmt ")) {
            size_t r = sz<sizeof(f)?sz:sizeof(f); if (drat3c_rd(c,f,r)!=r) return -1;
            uint16_t t = drat3c_rl16(f); c->info.channels = drat3c_rl16(f+2); c->info.sample_rate = drat3c_rl32(f+4);
            c->info.bit_rate = drat3c_rl32(f+8)*8; c->info.block_align = drat3c_rl16(f+12);
            if (t==0x0270) { c->info.codec_type=DRAT3_CODEC_ATRAC3; c->info.samples_per_frame=1024; if(sz>=28){c->info.extradata_size=14;memset(c->info.extradata,0,14);c->info.extradata[0]=1;memcpy(c->info.extradata+2,f+18,10);c->info.joint_stereo=drat3c_rl16(f+22)?1:0;}}
            else if (t==0x0E23||t==0xFFFE) { c->info.codec_type=DRAT3_CODEC_ATRAC3P; c->info.samples_per_frame=2048; if(t==0xFFFE&&sz>=40){uint16_t s=drat3c_rl16(f+24);if(s==0x0270){c->info.codec_type=DRAT3_CODEC_ATRAC3;c->info.samples_per_frame=1024;}}}
            ff=1; if(r<sz) drat3c_sk(c,sz-r,SEEK_CUR);
        } else if (drat3c_4cc(ch,"data")) { c->info.data_offset=drat3c_tl(c); c->info.data_size=sz; fd=1; }
        else drat3c_sk(c,(sz+1)&~1,SEEK_CUR);
    }
    if (!ff||!fd||c->info.codec_type==DRAT3_CODEC_UNKNOWN||c->info.block_align==0) return -1;
    c->info.container_type=DRAT3_CONTAINER_RIFF_WAV; c->info.total_frames=c->info.data_size/c->info.block_align;
    c->info.total_samples=c->info.total_frames*c->info.samples_per_frame; drat3c_sk(c,c->info.data_offset,SEEK_SET); return 0;
}

static int drat3c_oma(drat3_container* c) {
    uint8_t h[96]; if (drat3c_rd(c,h,10)!=10 || h[0]!='e'||h[1]!='a'||h[2]!='3') return -1;
    uint32_t i3 = ((uint32_t)(h[6]&0x7F)<<21)|((uint32_t)(h[7]&0x7F)<<14)|((uint32_t)(h[8]&0x7F)<<7)|(uint32_t)(h[9]&0x7F);
    drat3c_sk(c,10+i3,SEEK_SET); if (drat3c_rd(c,h,96)!=96 || h[0]!='E'||h[1]!='A'||h[2]!='3'||h[4]!=0||h[5]!=96) return -1;
    c->info.data_offset=drat3c_tl(c); uint8_t cid=h[32]; uint32_t cp=drat3c_rb24(h+33);
    if (cid==0) { c->info.codec_type=DRAT3_CODEC_ATRAC3; c->info.samples_per_frame=1024; c->info.sample_rate=drat3c_sr[(cp>>13)&7]*100;
        c->info.block_align=(cp&0x3FF)*8; c->info.joint_stereo=(cp>>17)&1; c->info.channels=2;
        c->info.extradata_size=14; memset(c->info.extradata,0,14); c->info.extradata[0]=1;
        c->info.extradata[2]=c->info.sample_rate&0xFF; c->info.extradata[3]=(c->info.sample_rate>>8)&0xFF;
        c->info.extradata[6]=c->info.joint_stereo; c->info.extradata[8]=c->info.joint_stereo; c->info.extradata[10]=1;
    } else if (cid==1) { c->info.codec_type=DRAT3_CODEC_ATRAC3P; c->info.samples_per_frame=2048;
        c->info.sample_rate=drat3c_sr[(cp>>13)&7]*100; c->info.block_align=((cp&0x3FF)*8)+8;
        uint32_t ch=(cp>>10)&7; static const uint8_t cc[]={0,1,2,3,4,6,7,8}; c->info.channels=(ch>0&&ch<8)?cc[ch]:2;
    } else return -1;
    if (c->info.sample_rate==0||c->info.block_align==0) return -1;
    c->info.bit_rate=c->info.sample_rate*c->info.block_align*8/c->info.samples_per_frame;
    int64_t cur=drat3c_tl(c); drat3c_sk(c,0,SEEK_END); c->info.data_size=drat3c_tl(c)-c->info.data_offset; drat3c_sk(c,cur,SEEK_SET);
    c->info.container_type=DRAT3_CONTAINER_OMA; c->info.total_frames=c->info.data_size/c->info.block_align;
    c->info.total_samples=c->info.total_frames*c->info.samples_per_frame; return 0;
}

drat3_container_type drat3_container_probe(const void* d, size_t n) {
    const uint8_t* p=(const uint8_t*)d;
    if(n<12) return DRAT3_CONTAINER_UNKNOWN;
    if(drat3c_4cc(p,"RIFF")&&drat3c_4cc(p+8,"WAVE")) return DRAT3_CONTAINER_RIFF_WAV;
    if(p[0]=='e'&&p[1]=='a'&&p[2]=='3') return DRAT3_CONTAINER_OMA;
    return DRAT3_CONTAINER_UNKNOWN;
}

drat3_container* drat3_container_open_file(const char* fn) {
    FILE* f=fopen(fn,"rb");
    if(!f) return NULL;
    drat3_container* c=(drat3_container*)calloc(1,sizeof(drat3_container));
    if(!c){fclose(f);return NULL;}
    c->file=f;
    if(drat3c_riff(c)==0) return c;
    fseek(f,0,SEEK_SET);
    if(drat3c_oma(c)==0) return c;
    fclose(f);free(c);return NULL;
}

drat3_container* drat3_container_open_memory(const void* d, size_t n) {
    drat3_container* c=(drat3_container*)calloc(1,sizeof(drat3_container));
    if(!c) return NULL;
    c->mem=(const uint8_t*)d; c->msz=n;
    if(drat3c_riff(c)==0) return c;
    c->mpos=0;
    if(drat3c_oma(c)==0) return c;
    free(c);return NULL;
}

void drat3_container_close(drat3_container* c) { if(!c) return; if(c->file) fclose(c->file); free(c); }
const drat3_container_info* drat3_container_get_info(const drat3_container* c) { return c?&c->info:NULL; }

int drat3_container_read_frame(drat3_container* c, void* b, size_t n) {
    if(!c||!b||c->eof||n<c->info.block_align) return -1;
    size_t r=drat3c_rd(c,b,c->info.block_align);
    if(r<c->info.block_align){c->eof=1;if(r==0)return 0;memset((uint8_t*)b+r,0,c->info.block_align-r);}
    c->frame++; return (int)c->info.block_align;
}

int drat3_container_seek_frame(drat3_container* c, uint64_t i) {
    if(!c||i>=c->info.total_frames) return -1;
    if(drat3c_sk(c,c->info.data_offset+i*c->info.block_align,SEEK_SET)!=0) return -1;
    c->frame=i; c->eof=0; return 0;
}

uint64_t drat3_container_get_frame_index(const drat3_container* c) { return c?c->frame:0; }

/* Decoder Implementation */
#if defined(__clang__)
#define DECLARE_ALIGNED(n, t, v)      t __attribute__((aligned(n))) v
#elif defined(__GNUC__)
#define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (n))) v
#elif defined(_MSC_VER)
#define DECLARE_ALIGNED(n,t,v)      __declspec(align(n)) t v
#else
#define DECLARE_ALIGNED(n,t,v)      t v
#endif

#define AV_INPUT_BUFFER_PADDING_SIZE 32
#define av_restrict

#define AVERROR(e) (-(e))
#define FFERRTAG(a, b, c, d) (-(int)(((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))))
#define AVERROR_INVALIDDATA        FFERRTAG( 'I','N','D','A')
#define AVERROR_PATCHWELCOME       FFERRTAG( 'P','A','W','E')

#define AV_LOG_ERROR    16
#define AV_LOG_WARNING  24
#define AV_LOG_DEBUG    48

static void av_log(int level, const char *fmt, ...) { (void)level; (void)fmt; }

#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFSWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

#define av_assert0(cond) ((void)0)
#define av_assert1(cond) ((void)0)
#define av_assert2(cond) ((void)0)
#define sign_extend dr_sign_extend
#define zero_extend dr_zero_extend

/* Memory allocation */
static void* av_malloc(size_t size) { return malloc(size); }
static void* av_mallocz(size_t size) { void* p = malloc(size); if(p) memset(p,0,size); return p; }
static void* av_mallocz_array(size_t n, size_t s) { return av_mallocz(n*s); }
static void* av_malloc_array(size_t n, size_t s) { return malloc(n*s); }
static void av_free(void* p) { free(p); }
static void av_freep(void* arg) { void** p = (void**)arg; if(p) { free(*p); *p = NULL; } }


/* ============================================================================
 * Byte Reading Macros
 * ============================================================================ */

#define AV_RB16(x) ((((const uint8_t*)(x))[0] << 8) | ((const uint8_t*)(x))[1])
#define AV_RB32(x) (((uint32_t)((const uint8_t*)(x))[0] << 24) | \
                   (((const uint8_t*)(x))[1] << 16) | \
                   (((const uint8_t*)(x))[2] <<  8) | \
                    ((const uint8_t*)(x))[3])
#define AV_RB64(x) (((uint64_t)AV_RB32(x) << 32) | AV_RB32((const uint8_t*)(x)+4))

#define AV_RL16(x) ((((const uint8_t*)(x))[1] << 8) | ((const uint8_t*)(x))[0])
#define AV_RL32(x) (((uint32_t)((const uint8_t*)(x))[3] << 24) | \
                   (((const uint8_t*)(x))[2] << 16) | \
                   (((const uint8_t*)(x))[1] <<  8) | \
                    ((const uint8_t*)(x))[0])

#define AV_RN16(p) AV_RL16(p)
#define AV_RN32(p) AV_RL32(p)
#define av_be2ne32(x) (((x) >> 24) | (((x) >> 8) & 0xff00) | (((x) << 8) & 0xff0000) | ((x) << 24))

static inline unsigned int bytestream_get_le16(const uint8_t **b) {
    unsigned int v = AV_RL16(*b);
    *b += 2;
    return v;
}
static inline unsigned int bytestream_get_le32(const uint8_t **b) {
    unsigned int v = AV_RL32(*b);
    *b += 4;
    return v;
}
static inline unsigned int bytestream_get_be16(const uint8_t **b) {
    unsigned int v = AV_RB16(*b);
    *b += 2;
    return v;
}
static inline unsigned int bytestream_get_be32(const uint8_t **b) {
    unsigned int v = AV_RB32(*b);
    *b += 4;
    return v;
}


/* ============================================================================
 * Bitstream Reader
 * ============================================================================ */

#define NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))

static inline unsigned int dr_zero_extend(unsigned int val, unsigned int bits) {
    return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
}

static inline int dr_sign_extend(int val, unsigned int bits) {
    unsigned int shift = 8 * sizeof(int) - bits;
    union { unsigned int u; int s; } v = { (unsigned int)val << shift };
    return v.s >> shift;
}

typedef struct GetBitContext {
    const uint8_t *buffer, *buffer_end;
    int index;
    int size_in_bits;
    int size_in_bits_plus8;
} GetBitContext;

#define VLC_TYPE int16_t

typedef struct VLC {
    int bits;
    VLC_TYPE (*table)[2];
    int table_size, table_allocated;
} VLC;

#define OPEN_READER(name, gb)            \
    unsigned int name ## _index = (gb)->index;  \
    unsigned int name ## _cache

#define CLOSE_READER(name, gb) (gb)->index = name ## _index

#define UPDATE_CACHE(name, gb) name ## _cache = \
    AV_RB32((gb)->buffer + (name ## _index >> 3)) << (name ## _index & 7)

#define SKIP_CACHE(name, gb, num) name ## _cache <<= (num)
#define SKIP_COUNTER(name, gb, num) name ## _index += (num)
#define SKIP_BITS(name, gb, num) do { SKIP_CACHE(name, gb, num); SKIP_COUNTER(name, gb, num); } while (0)
#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)

#define SHOW_UBITS(name, gb, num) NEG_USR32(name ## _cache, num)
#define SHOW_SBITS(name, gb, num) NEG_SSR32(name ## _cache, num)
#define GET_CACHE(name, gb) ((uint32_t) name ## _cache)

static inline int get_bits_count(const GetBitContext *s) { return s->index; }

static inline int get_sbits(GetBitContext *s, int n) {
    int tmp;
    OPEN_READER(re, s);
    UPDATE_CACHE(re, s);
    tmp = SHOW_SBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
    return tmp;
}

static inline unsigned int get_bits(GetBitContext *s, int n) {
    unsigned int tmp;
    OPEN_READER(re, s);
    UPDATE_CACHE(re, s);
    tmp = SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
    return tmp;
}

static inline int get_bitsz(GetBitContext *s, int n) {
    return n ? get_bits(s, n) : 0;
}

static inline void skip_bits(GetBitContext *s, int n) {
    OPEN_READER(re, s);
    (void)re_cache; /* suppress unused variable warning */
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
}

static inline unsigned int get_bits1(GetBitContext *s) {
    unsigned int index = s->index;
    uint8_t result = s->buffer[index >> 3];
    result <<= index & 7;
    result >>= 7;
    s->index = index + 1;
    return result;
}

static inline int init_get_bits(GetBitContext *s, const uint8_t *buffer, int bit_size) {
    int buffer_size;
    if (bit_size >= INT_MAX - 7 || bit_size < 0 || !buffer) {
        bit_size = 0;
        buffer = NULL;
    }
    buffer_size = (bit_size + 7) >> 3;
    s->buffer = buffer;
    s->size_in_bits = bit_size;
    s->size_in_bits_plus8 = bit_size + 8;
    s->buffer_end = buffer + buffer_size;
    s->index = 0;
    return 0;
}

static inline int init_get_bits8(GetBitContext *s, const uint8_t *buffer, int byte_size) {
    if (byte_size > INT_MAX / 8 || byte_size < 0) byte_size = -1;
    return init_get_bits(s, buffer, byte_size * 8);
}

static inline int get_bits_left(GetBitContext *gb) {
    return gb->size_in_bits - get_bits_count(gb);
}

/* VLC decoding */
#define INIT_VLC_LE             2
#define INIT_VLC_USE_NEW_STATIC 4

static int ff_init_vlc_sparse(VLC *vlc, int nb_bits, int nb_codes,
                       const void *bits, int bits_wrap, int bits_size,
                       const void *codes, int codes_wrap, int codes_size,
                       const void *symbols, int symbols_wrap, int symbols_size,
                       int flags);
static void ff_free_vlc(VLC *vlc);

#define init_vlc(vlc, nb_bits, nb_codes, bits, bits_wrap, bits_size, codes, codes_wrap, codes_size, flags) \
    ff_init_vlc_sparse(vlc, nb_bits, nb_codes, bits, bits_wrap, bits_size, codes, codes_wrap, codes_size, NULL, 0, 0, flags)

#define INIT_VLC_STATIC(vlc, bits, a, b, c, d, e, f, g, static_size) \
    do { \
        static VLC_TYPE table[static_size][2]; \
        (vlc)->table = table; \
        (vlc)->table_allocated = static_size; \
        init_vlc(vlc, bits, a, b, c, d, e, f, g, INIT_VLC_USE_NEW_STATIC); \
    } while (0)

#define GET_VLC(code, name, gb, table, bits, max_depth) \
    do { \
        int n, nb_bits; \
        unsigned int index; \
        index = SHOW_UBITS(name, gb, bits); \
        code  = table[index][0]; \
        n     = table[index][1]; \
        if (max_depth > 1 && n < 0) { \
            LAST_SKIP_BITS(name, gb, bits); \
            UPDATE_CACHE(name, gb); \
            nb_bits = -n; \
            index = SHOW_UBITS(name, gb, nb_bits) + code; \
            code  = table[index][0]; \
            n     = table[index][1]; \
            if (max_depth > 2 && n < 0) { \
                LAST_SKIP_BITS(name, gb, nb_bits); \
                UPDATE_CACHE(name, gb); \
                nb_bits = -n; \
                index = SHOW_UBITS(name, gb, nb_bits) + code; \
                code  = table[index][0]; \
                n     = table[index][1]; \
            } \
        } \
        SKIP_BITS(name, gb, n); \
    } while (0)

static inline int get_vlc2(GetBitContext *s, VLC_TYPE (*table)[2], int bits, int max_depth) {
    int code;
    OPEN_READER(re, s);
    UPDATE_CACHE(re, s);
    GET_VLC(code, re, s, table, bits, max_depth);
    CLOSE_READER(re, s);
    return code;
}


/* ============================================================================
 * FFT/MDCT
 * ============================================================================ */

#define CMUL(dre, dim, are, aim, bre, bim) do { \
    (dre) = (are) * (bre) - (aim) * (bim); \
    (dim) = (are) * (bim) + (aim) * (bre); \
} while (0)

typedef float FFTSample;
typedef float FFTDouble;

typedef struct FFTComplex {
    FFTSample re, im;
} FFTComplex;

enum mdct_permutation_type {
    FF_MDCT_PERM_NONE,
    FF_MDCT_PERM_INTERLEAVE,
};

typedef struct FFTContext {
    int nbits;
    int inverse;
    uint16_t *revtab;
    FFTComplex *tmp_buf;
    int mdct_size;
    int mdct_bits;
    FFTSample *tcos;
    FFTSample *tsin;
    enum mdct_permutation_type mdct_permutation;
} FFTContext;

static void fft_calc(FFTContext *s, FFTComplex *z);
static void imdct_calc(FFTContext *s, FFTSample *output, const FFTSample *input);
static void imdct_half(FFTContext *s, FFTSample *output, const FFTSample *input);
static int ff_fft_init(FFTContext *s, int nbits, int inverse);
static void ff_fft_end(FFTContext *s);
static int ff_mdct_init(FFTContext *s, int nbits, int inverse, double scale);
static void ff_mdct_end(FFTContext *s);

/* Cosine tables */
#define COSTABLE(size) static DECLARE_ALIGNED(32, FFTSample, av_cos_##size)[size/2]
COSTABLE(16);
COSTABLE(32);
COSTABLE(64);
COSTABLE(128);
COSTABLE(256);
COSTABLE(512);
COSTABLE(1024);

static FFTSample * const av_cos_tabs[] = {
    NULL, NULL, NULL, NULL,
    av_cos_16, av_cos_32, av_cos_64, av_cos_128, av_cos_256, av_cos_512, av_cos_1024,
};

static void ff_init_ff_cos_tabs(int index) {
    int i, m = 1 << index;
    double freq = 2 * M_PI / m;
    FFTSample *tab = av_cos_tabs[index];
    for (i = 0; i <= m/4; i++)
        tab[i] = (FFTSample)cos(i * freq);
    for (i = 1; i < m/4; i++)
        tab[m/2 - i] = tab[i];
}

/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */



static inline void vector_fmul(float * av_restrict dst, const float * av_restrict src, int len) {
    for (int i = 0; i < len; i++)
        dst[i] = dst[i] * src[i];
}

/**
* Multiply a vector of floats by a scalar float.  Source and
* destination vectors must overlap exactly or not at all.
*/
static inline void vector_fmul_scalar(float *dst, float mul, int len) {
    for (int i = 0; i < len; i++)
        dst[i] *= mul;
}

/**
* Calculate the entry wise product of two vectors of floats, and store the result
* in a vector of floats. The second vector of floats is iterated over
* in reverse order.
*
* @param dst  output and first input vector
*             constraints: 32-byte aligned
* @param src second input vector
*             constraints: 32-byte aligned
* @param len  number of elements in the input
*             constraints: multiple of 16
*/
static inline void vector_fmul_reverse(float * av_restrict dst, const float * av_restrict src, int len) {
    src += len - 1;
    for (int i = 0; i < len; i++)
        dst[i] *= src[-i];
}

/* ============================================================================
 * ATRAC Common (atrac.h)
 * ============================================================================ */
/*
 * common functions for the ATRAC family of decoders
 *
 * Copyright (c) 2009-2013 Maxim Poliakovski
 * Copyright (c) 2009 Benjamin Larsson
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * ATRAC common header
 */



/**
 *  Gain control parameters for one subband.
 */
typedef struct AtracGainInfo {
    int   num_points;   ///< number of gain control points
    int   lev_code[7];  ///< level at corresponding control point
    int   loc_code[7];  ///< location of gain control points
} AtracGainInfo;

/**
 *  Gain compensation context structure.
 */
typedef struct AtracGCContext {
    float   gain_tab1[16];  ///< gain compensation level table
    float   gain_tab2[31];  ///< gain compensation interpolation table
    int     id2exp_offset;  ///< offset for converting level index into level exponent
    int     loc_scale;      ///< scale of location code = 2^loc_scale samples
    int     loc_size;       ///< size of location code in samples
} AtracGCContext;

extern float av_atrac_sf_table[64];

/**
 * Generate common tables.
 */
void ff_atrac_generate_tables(void);

/**
 *  Initialize gain compensation context.
 *
 * @param gctx            pointer to gain compensation context to initialize
 * @param id2exp_offset   offset for converting level index into level exponent
 * @param loc_scale       location size factor
 */
void ff_atrac_init_gain_compensation(AtracGCContext *gctx, int id2exp_offset,
                                     int loc_scale);

/**
 * Apply gain compensation and perform the MDCT overlapping part.
 *
 * @param gctx         pointer to gain compensation context
 * @param in           input buffer
 * @param prev         previous buffer to perform overlap against
 * @param gc_now       gain control information for current frame
 * @param gc_next      gain control information for next frame
 * @param num_samples  number of samples to process
 * @param out          output data goes here
 */
void ff_atrac_gain_compensation(AtracGCContext *gctx, float *in, float *prev,
                                AtracGainInfo *gc_now, AtracGainInfo *gc_next,
                                int num_samples, float *out);

/**
 * Quadrature mirror synthesis filter.
 *
 * @param inlo      lower part of spectrum
 * @param inhi      higher part of spectrum
 * @param nIn       size of spectrum buffer
 * @param pOut      out buffer
 * @param delayBuf  delayBuf buffer
 * @param temp      temp buffer
 */
void ff_atrac_iqmf(float *inlo, float *inhi, unsigned int nIn, float *pOut,
                   float *delayBuf, float *temp);


/* ============================================================================
 * ATRAC3+ Structures (atrac3plus.h)
 * ============================================================================ */
/*
 * ATRAC3+ compatible decoder
 *
 * Copyright (c) 2010-2013 Maxim Poliakovski
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Global structures, constants and data for ATRAC3+ decoder.
 */




/** Global unit sizes */
#define ATRAC3P_SUBBANDS        16  ///< number of PQF subbands
#define ATRAC3P_SUBBAND_SAMPLES 128 ///< number of samples per subband
#define ATRAC3P_FRAME_SAMPLES   (ATRAC3P_SUBBAND_SAMPLES * ATRAC3P_SUBBANDS)

#define ATRAC3P_PQF_FIR_LEN     12  ///< length of the prototype FIR of the PQF

/** Global constants */
#define ATRAC3P_POWER_COMP_OFF  15  ///< disable power compensation

/** ATRAC3+ channel unit types */
enum Atrac3pChannelUnitTypes {
    CH_UNIT_MONO       = 0, ///< unit containing one coded channel
    CH_UNIT_STEREO     = 1, ///< unit containing two jointly-coded channels
    CH_UNIT_EXTENSION  = 2, ///< unit containing extension information
    CH_UNIT_TERMINATOR = 3  ///< unit sequence terminator
};

/** Per-channel IPQF history */
typedef struct Atrac3pIPQFChannelCtx {
    DECLARE_ALIGNED(32, float, buf1)[ATRAC3P_PQF_FIR_LEN * 2][8];
    DECLARE_ALIGNED(32, float, buf2)[ATRAC3P_PQF_FIR_LEN * 2][8];
    int pos;
} Atrac3pIPQFChannelCtx;

/** Amplitude envelope of a group of sine waves */
typedef struct Atrac3pWaveEnvelope {
    int has_start_point;    ///< indicates start point within the GHA window
    int has_stop_point;     ///< indicates stop point within the GHA window
    int start_pos;          ///< start position expressed in n*4 samples
    int stop_pos;           ///< stop  position expressed in n*4 samples
} Atrac3pWaveEnvelope;

/** Parameters of a group of sine waves */
typedef struct Atrac3pWavesData {
    Atrac3pWaveEnvelope pend_env;   ///< pending envelope from the previous frame
    Atrac3pWaveEnvelope curr_env;   ///< group envelope from the current frame
    int num_wavs;           ///< number of sine waves in the group
    int start_index;        ///< start index into global tones table for that subband
} Atrac3pWavesData;

/** Parameters of a single sine wave */
typedef struct Atrac3pWaveParam {
    int   freq_index;   ///< wave frequency index
    int   amp_sf;       ///< quantized amplitude scale factor
    int   amp_index;    ///< quantized amplitude index
    int   phase_index;  ///< quantized phase index
} Atrac3pWaveParam;

/** Sound channel parameters */
typedef struct Atrac3pChanParams {
    int ch_num;
    int num_coded_vals;         ///< number of transmitted quant unit values
    int fill_mode;
    int split_point;
    int table_type;             ///< table type: 0 - tone?, 1- noise?
    int qu_wordlen[32];         ///< array of word lengths for each quant unit
    int qu_sf_idx[32];          ///< array of scale factor indexes for each quant unit
    int qu_tab_idx[32];         ///< array of code table indexes for each quant unit
    int16_t spectrum[2048];     ///< decoded IMDCT spectrum
    uint8_t power_levs[5];      ///< power compensation levels

    /* imdct window shape history (2 frames) for overlapping. */
    uint8_t wnd_shape_hist[2][ATRAC3P_SUBBANDS];    ///< IMDCT window shape, 0=sine/1=steep
    uint8_t *wnd_shape;         ///< IMDCT window shape for current frame
    uint8_t *wnd_shape_prev;    ///< IMDCT window shape for previous frame

    /* gain control data history (2 frames) for overlapping. */
    AtracGainInfo gain_data_hist[2][ATRAC3P_SUBBANDS];  ///< gain control data for all subbands
    AtracGainInfo *gain_data;       ///< gain control data for next frame
    AtracGainInfo *gain_data_prev;  ///< gain control data for previous frame
    int num_gain_subbands;      ///< number of subbands with gain control data

    /* tones data history (2 frames) for overlapping. */
    Atrac3pWavesData tones_info_hist[2][ATRAC3P_SUBBANDS];
    Atrac3pWavesData *tones_info;
    Atrac3pWavesData *tones_info_prev;
} Atrac3pChanParams;

/* Per-unit sine wave parameters */
typedef struct Atrac3pWaveSynthParams {
    int tones_present;                      ///< 1 - tones info present
    int amplitude_mode;                     ///< 1 - low range, 0 - high range
    int num_tone_bands;                     ///< number of PQF bands with tones
    uint8_t tone_sharing[ATRAC3P_SUBBANDS]; ///< 1 - subband-wise tone sharing flags
    uint8_t tone_master[ATRAC3P_SUBBANDS];  ///< 1 - subband-wise tone channel swapping
    uint8_t invert_phase[ATRAC3P_SUBBANDS]; ///< 1 - subband-wise phase inversion
    int tones_index;                        ///< total sum of tones in this unit
    Atrac3pWaveParam waves[48];
} Atrac3pWaveSynthParams;

/** Channel unit parameters */
typedef struct Atrac3pChanUnitCtx {
    /* channel unit variables */
    int unit_type;                          ///< unit type (mono/stereo)
    int num_quant_units;
    int num_subbands;
    int used_quant_units;                   ///< number of quant units with coded spectrum
    int num_coded_subbands;                 ///< number of subbands with coded spectrum
    int mute_flag;                          ///< mute flag
    int use_full_table;                     ///< 1 - full table list, 0 - restricted one
    int noise_present;                      ///< 1 - global noise info present
    int noise_level_index;                  ///< global noise level index
    int noise_table_index;                  ///< global noise RNG table index
    uint8_t swap_channels[ATRAC3P_SUBBANDS];    ///< 1 - perform subband-wise channel swapping
    uint8_t negate_coeffs[ATRAC3P_SUBBANDS];    ///< 1 - subband-wise IMDCT coefficients negation
    Atrac3pChanParams channels[2];

    /* Variables related to GHA tones */
    Atrac3pWaveSynthParams wave_synth_hist[2];     ///< waves synth history for two frames
    Atrac3pWaveSynthParams *waves_info;
    Atrac3pWaveSynthParams *waves_info_prev;

    Atrac3pIPQFChannelCtx ipqf_ctx[2];
    DECLARE_ALIGNED(32, float, prev_buf)[2][ATRAC3P_FRAME_SAMPLES]; ///< overlapping buffer
} Atrac3pChanUnitCtx;

/**
 * Initialize VLC tables for bitstream parsing.
 */
void ff_atrac3p_init_vlcs(void);

/**
 * Decode bitstream data of a channel unit.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @return result code: 0 = OK, otherwise - error code
 */
int ff_atrac3p_decode_channel_unit(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   int num_channels);

/**
 * Initialize IMDCT transform.
 *
 * @param[in]   mdct_ctx   pointer to MDCT transform context
 */
void ff_atrac3p_init_imdct(FFTContext *mdct_ctx);

/**
 * Initialize sine waves synthesizer.
 */
void ff_atrac3p_init_wave_synth(void);

/**
 * Synthesize sine waves for a particular subband.
 *
 * @param[in]   ch_unit   pointer to the channel unit context
 * @param[in]   fdsp      pointer to float DSP context
 * @param[in]   ch_num    which channel to process
 * @param[in]   sb        which subband to process
 * @param[out]  out       receives processed data
 */
void ff_atrac3p_generate_tones(Atrac3pChanUnitCtx *ch_unit, int ch_num, int sb, float *out);

/**
 * Perform power compensation aka noise dithering.
 *
 * @param[in]      ctx         ptr to the channel context
 * @param[in]      ch_index    which channel to process
 * @param[in,out]  sp          ptr to channel spectrum to process
 * @param[in]      rng_index   indicates which RNG table to use
 * @param[in]      sb_num      which subband to process
 */
void ff_atrac3p_power_compensation(Atrac3pChanUnitCtx *ctx, int ch_index,
                                   float *sp, int rng_index, int sb_num);

/**
 * Regular IMDCT and windowing without overlapping,
 * with spectrum reversal in the odd subbands.
 *
 * @param[in]   fdsp       pointer to float DSP context
 * @param[in]   mdct_ctx   pointer to MDCT transform context
 * @param[in]   pIn        float input
 * @param[out]  pOut       float output
 * @param[in]   wind_id    which MDCT window to apply
 * @param[in]   sb         subband number
 */
void ff_atrac3p_imdct(FFTContext *mdct_ctx, float *pIn,
                      float *pOut, int wind_id, int sb);

/**
 * Subband synthesis filter based on the polyphase quadrature (pseudo-QMF)
 * filter bank.
 *
 * @param[in]      dct_ctx   ptr to the pre-initialized IDCT context
 * @param[in,out]  hist      ptr to the filter history
 * @param[in]      in        input data to process
 * @param[out]     out       receives processed data
 */
void ff_atrac3p_ipqf(FFTContext *dct_ctx, Atrac3pIPQFChannelCtx *hist,
                     const float *in, float *out);

extern const uint16_t av_atrac3p_qu_to_spec_pos[33];
extern const float av_atrac3p_sf_tab[64];
extern const float av_atrac3p_mant_tab[8];


/* ============================================================================
 * ATRAC3 Data Tables (atrac3data.h)
 * ============================================================================ */
/*
 * ATRAC3 compatible decoder data
 * Copyright (c) 2006-2007 Maxim Poliakovski
 * Copyright (c) 2006-2007 Benjamin Larsson
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * ATRAC3 AKA RealAudio 8 compatible decoder data
 */



/* VLC tables */

static const uint8_t huffcode1[9] = {
    0x0, 0x4, 0x5, 0xC, 0xD, 0x1C, 0x1D, 0x1E, 0x1F
};

static const uint8_t huffbits1[9] = { 1, 3, 3, 4, 4, 5, 5, 5, 5 };

static const uint8_t huffcode2[5] = { 0x0, 0x4, 0x5, 0x6, 0x7 };

static const uint8_t huffbits2[5] = { 1, 3, 3, 3, 3 };

static const uint8_t huffcode3[7] = { 0x0, 0x4, 0x5, 0xC, 0xD, 0xE, 0xF };

static const uint8_t huffbits3[7] = { 1, 3, 3, 4, 4, 4, 4 };

static const uint8_t huffcode4[9] = {
    0x0, 0x4, 0x5, 0xC, 0xD, 0x1C, 0x1D, 0x1E, 0x1F
};

static const uint8_t huffbits4[9] = { 1, 3, 3, 4, 4, 5, 5, 5, 5 };

static const uint8_t huffcode5[15] = {
    0x00, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B, 0x1C,
    0x1D, 0x3C, 0x3D, 0x3E, 0x3F, 0x0C, 0x0D
};

static const uint8_t huffbits5[15] = {
    2, 3, 3, 4, 4, 4, 4, 5, 5, 6, 6, 6, 6, 4, 4
};

static const uint8_t huffcode6[31] = {
    0x00, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x14,
    0x15, 0x16, 0x17, 0x18, 0x19, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x3A, 0x3B, 0x78, 0x79, 0x7A,
    0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x08, 0x09
};

static const uint8_t huffbits6[31] = {
    3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6,
    6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 4, 4
};

static const uint8_t huffcode7[63] = {
    0x00, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0x68, 0x69, 0x6A, 0x6B, 0x6C,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74,
    0x75, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2,
    0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA,
    0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0x02, 0x03
};

static const uint8_t huffbits7[63] = {
    3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4
};

static const uint8_t huff_tab_sizes[7] = {
    9, 5, 7, 9, 15, 31, 63,
};

static const uint8_t* const huff_codes[7] = {
    huffcode1, huffcode2, huffcode3, huffcode4, huffcode5, huffcode6, huffcode7
};

static const uint8_t* const huff_bits[7] = {
    huffbits1, huffbits2, huffbits3, huffbits4, huffbits5, huffbits6, huffbits7,
};

static const uint16_t atrac3_vlc_offs[9] = {
    0, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4096
};

/* selector tables */

static const uint8_t clc_length_tab[8] = { 0, 4, 3, 3, 4, 4, 5, 6 };

static const int8_t mantissa_clc_tab[4] = { 0, 1, -2, -1 };

static const int8_t mantissa_vlc_tab[18] = {
    0, 0,  0, 1,  0, -1,  1, 0,  -1, 0,  1, 1,  1, -1,  -1, 1,  -1, -1
};


/* tables for the scalefactor decoding */

static const float inv_max_quant[8] = {
      0.0f,       1.0f / 1.5f, 1.0f /  2.5f, 1.0f /  3.5f,
      1.0f / 4.5f, 1.0f / 7.5f, 1.0f / 15.5f, 1.0f / 31.5f
};

static const uint16_t subband_tab[33] = {
      0,   8,  16,  24,  32,  40,  48,  56,
     64,  80,  96, 112, 128, 144, 160, 176,
    192, 224, 256, 288, 320, 352, 384, 416,
    448, 480, 512, 576, 640, 704, 768, 896,
    1024
};

/* joint stereo related tables */
static const float matrix_coeffs[8] = {
    0.0f, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 1.0f, 1.0f
};


/* ============================================================================
 * ATRAC3+ Data Tables (atrac3plus_data.h)
 * ============================================================================ */
/*
 * ATRAC3+ compatible decoder
 *
 * Copyright (c) 2010-2013 Maxim Poliakovski
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */



/** VLC tables for wordlen */
static const uint8_t atrac3p_wl_huff_code1[3] = { 0, 2, 3 };

static const uint8_t atrac3p_wl_huff_bits1[3] = { 1, 2, 2 };

static const uint8_t atrac3p_wl_huff_xlat1[3] = { 0, 1, 7 };

static const uint8_t atrac3p_wl_huff_code2[5] = { 0, 4, 5, 6, 7 };

static const uint8_t atrac3p_wl_huff_bits2[5] = { 1, 3, 3, 3, 3 };

static const uint8_t atrac3p_wl_huff_xlat2[5] = { 0, 1, 2, 6, 7 };

static const uint8_t atrac3p_wl_huff_code3[8] = {
    0, 4, 0xC, 0x1E, 0x1F, 0xD, 0xE, 5
};

static const uint8_t atrac3p_wl_huff_bits3[8] = { 1, 3, 4, 5, 5, 4, 4, 3 };

static const uint8_t atrac3p_wl_huff_code4[8] = {
    0, 4, 0xC, 0xD, 0x1E, 0x1F, 0xE, 5
};

static const uint8_t atrac3p_wl_huff_bits4[8] = { 1, 3, 4, 4, 5, 5, 4, 3 };

/** VLC tables for scale factor indexes */
static const uint16_t atrac3p_sf_huff_code1[64] = {
        0,     2,     3,     4,     5,   0xC,   0xD,  0xE0,
     0xE1,  0xE2,  0xE3,  0xE4,  0xE5,  0xE6, 0x1CE, 0x1CF,
    0x1D0, 0x1D1, 0x1D2, 0x1D3, 0x1D4, 0x1D5, 0x1D6, 0x1D7,
    0x1D8, 0x1D9, 0x1DA, 0x1DB, 0x1DC, 0x1DD, 0x1DE, 0x1DF,
    0x1E0, 0x1E1, 0x1E2, 0x1E3, 0x1E4, 0x1E5, 0x1E6, 0x1E7,
    0x1E8, 0x1E9, 0x1EA, 0x1EB, 0x1EC, 0x1ED, 0x1EE, 0x1EF,
    0x1F0, 0x1F1, 0x1F2, 0x1F3, 0x1F4, 0x1F5, 0x1F6, 0x1F7,
    0x1F8, 0x1F9, 0x1FA, 0x1FB, 0x1FC, 0x1FD, 0x1FE, 0x1FF
};

static const uint8_t atrac3p_sf_huff_bits1[64] = {
    2, 3, 3, 3, 3, 4, 4, 8, 8, 8, 8, 8, 8, 8, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9
};

static const uint8_t atrac3p_sf_huff_xlat1[64] = {
     0,  1, 61, 62, 63,  2, 60,  3,  4,  5,  6, 57, 58, 59,  7,  8,
     9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56
};

static const uint8_t atrac3p_sf_huff_xlat2[64] = {
    0,   1,  2, 62, 63,  3, 61,  4,  5,  6, 57, 58, 59, 60,  7,  8,
    9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56
};

static const uint16_t atrac3p_sf_huff_code2[64] = {
        0,     4,  0x18,  0x19,  0x70, 0x1CA, 0x1CB, 0x1CC,
    0x1CD, 0x1CE, 0x1CF, 0x1D0, 0x1D1, 0x1D2, 0x1D3, 0x1D4,
    0x1D5, 0x1D6, 0x1D7, 0x1D8, 0x1D9, 0x1DA, 0x1DB, 0x1DC,
    0x1DD, 0x1DE, 0x1DF, 0x1E0, 0x1E1, 0x1E2, 0x1E3, 0x1E4,
    0x1E5, 0x1E6, 0x1E7, 0x1E8, 0x1E9, 0x1EA, 0x1EB, 0x1EC,
    0x1ED, 0x1EE, 0x1EF, 0x1F0, 0x1F1, 0x1F2, 0x1F3, 0x1F4,
    0x1F5, 0x1F6, 0x1F7, 0x1F8, 0x1F9, 0x1FA, 0x1FB, 0x1FC,
    0x1FD, 0x1FE, 0x1FF,  0xE4,  0x71,  0x1A,  0x1B,     5
};

static const uint8_t atrac3p_sf_huff_bits2[64] = {
    1, 3, 5, 5, 7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 7, 5, 5, 3
};

static const uint16_t atrac3p_sf_huff_code3[64] = {
        0,     2,     3,  0x18,  0x19,  0x70, 0x1CC, 0x1CD,
    0x1CE, 0x1CF, 0x1D0, 0x1D1, 0x1D2, 0x1D3, 0x1D4, 0x1D5,
    0x1D6, 0x1D7, 0x1D8, 0x1D9, 0x1DA, 0x1DB, 0x1DC, 0x1DD,
    0x1DE, 0x1DF, 0x1E0, 0x1E1, 0x1E2, 0x1E3, 0x1E4, 0x1E5,
    0x1E6, 0x1E7, 0x1E8, 0x1E9, 0x1EA, 0x1EB, 0x1EC, 0x1ED,
    0x1EE, 0x1EF, 0x1F0, 0x1F1, 0x1F2, 0x1F3, 0x1F4, 0x1F5,
    0x1F6, 0x1F7, 0x1F8, 0x1F9, 0x1FA, 0x1FB, 0x1FC, 0x1FD,
    0x1FE, 0x1FF,  0x71,  0x72,  0x1A,  0x1B,     4,     5
};

static const uint8_t atrac3p_sf_huff_bits3[64] = {
    2, 3, 3, 5, 5, 7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 7, 7, 5, 5, 3, 3
};

static const uint16_t atrac3p_sf_huff_code4[16] = {
    0, 2, 3, 4, 5, 0xC, 0xD, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0, 0x3D, 0x3E, 0x3F
};

static const uint8_t atrac3p_sf_huff_bits4[16] = {
    2, 3, 3, 3, 3, 4, 4, 6, 6, 6, 6, 6, 0, 6, 6, 6
};

static const uint8_t atrac3p_sf_huff_xlat4[16] = {
    0, 1, 13, 14, 15, 2, 12, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

static const uint8_t atrac3p_sf_huff_xlat5[16] = {
    0, 1, 2, 14, 15, 3, 13, 4, 5, 6, 7, 9, 8, 10, 11, 12
};

static const uint16_t atrac3p_sf_huff_code5[16] = {
    0,    4,  0xC, 0x1C, 0x78, 0x79, 0x7A, 0x7B,
    0, 0x7C, 0x7D, 0x7E, 0x7F, 0x1D, 0xD,     5
};

static const uint8_t atrac3p_sf_huff_bits5[16] = {
    1, 3, 4, 5, 7, 7, 7, 7, 0, 7, 7, 7, 7, 5, 4, 3
};

static const uint16_t atrac3p_sf_huff_code6[16] = {
    0, 2, 3, 0xC, 0x1C, 0x3C, 0x7C, 0x7D, 0, 0x7E, 0x7F, 0x3D, 0x1D, 0xD, 4, 5
};

static const uint8_t atrac3p_sf_huff_bits6[16] = {
    2, 3, 3, 4, 5, 6, 7, 7, 0, 7, 7, 6, 5, 4, 3, 3
};

/** VLC tables for code table indexes */
static const uint8_t atrac3p_ct_huff_code1[4] = { 0, 2, 6, 7 };

static const uint8_t atrac3p_ct_huff_bits1[4] = { 1, 2, 3, 3 };

static const uint8_t atrac3p_ct_huff_code2[8] = { 0, 2, 3, 4, 5, 6, 0xE, 0xF };

static const uint8_t atrac3p_ct_huff_bits2[8] = { 2, 3, 3, 3, 3, 3, 4, 4 };

static const uint8_t atrac3p_ct_huff_xlat1[8] = { 0, 1, 2, 3, 6, 7, 4, 5 };

static const uint8_t atrac3p_ct_huff_code3[8] = {
    0, 4, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
};

static const uint8_t atrac3p_ct_huff_bits3[8] = { 1, 3, 4, 4, 4, 4, 4, 4 };

/* weights for quantized word lengths */
static const int8_t atrac3p_wl_weights[6][32] = {
    { 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 },
    { 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 5, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 6, 5, 5, 5, 5, 5, 5, 5, 3, 3, 3, 3, 2, 2, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

/* weights for quantized scale factors
 * sf_weights[i] = i / (tab_idx + 1)
 * where tab_idx = [1,2] */
static const int8_t atrac3p_sf_weights[2][32] = {
    { 0, 0, 1, 1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7,
      8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15 },
    { 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4,  4,  5,
      5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10 }
};

/** Ungroup table for word length segments.
 *  Numbers in this table tell which coeff belongs to which segment. */
static const uint8_t atrac3p_qu_num_to_seg[32] = {
    0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
    5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 9, 9
};

/** Map quant unit number to subband number */
static const uint8_t atrac3p_qu_to_subband[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1,  1,  1,  2,  2,  2, 2,
    3, 3, 4, 4, 5, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

/** Map subband number to number of power compensation groups */
static const int atrac3p_subband_to_num_powgrps[16] = {
    1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5
};

/** 3D base shape tables. The values are grouped together as follows:
 *  [num_start_values = 8][num_shape_tables = 16][num_seg_coeffs = 9]
 *  For each of the 8 start values there are 16 different shapes each
 *  9 coefficients long. */
static const int8_t atrac3p_wl_shapes[8][16][9] = {
    { {  0,  0,  0,  0,  0,  0,  0, -2, -1 },
      {  0,  0,  0,  0,  0,  0,  0, -5, -1 },
      {  0,  0,  0, -7,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0, -7,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0, -5,  0,  0 },
      {  0,  0,  0,  0, -5,  0,  0,  0,  0 },
      { -7, -7,  0,  0,  0,  0,  0,  0,  0 },
      {  0, -7,  0,  0,  0,  0,  0,  0,  0 },
      { -2, -2, -5,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0, -2, -5,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0, -2, -5,  0,  0 },
      {  0,  0,  0, -5,  0,  0,  0,  0,  0 },
      {  0, -2, -7, -2,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0, -2, -5,  0,  0,  0 },
      {  0,  0,  0, -5, -5,  0,  0,  0,  0 },
      {  0,  0,  0, -5, -2,  0,  0,  0,  0 } },
    { { -1, -5, -3, -2, -1, -1,  0,  0,  0 },
      { -2, -5, -3, -3, -2, -1, -1,  0,  0 },
      {  0, -1, -1, -1,  0,  0,  0,  0,  0 },
      { -1, -3,  0,  0,  0,  0,  0,  0,  0 },
      { -1, -2,  0,  0,  0,  0,  0,  0,  0 },
      { -1, -3, -1,  0,  0,  0,  0,  1,  1 },
      { -1, -5, -3, -3, -2, -1,  0,  0,  0 },
      { -1, -1, -4, -2, -2, -1, -1,  0,  0 },
      { -1, -1, -3, -2, -3, -1, -1, -1,  0 },
      { -1, -4, -2, -3, -1,  0,  0,  0,  0 },
      {  0, -1, -2, -2, -1, -1,  0,  0,  0 },
      {  0, -2, -1,  0,  0,  0,  0,  0,  0 },
      { -1, -1,  0,  0,  0,  0,  0,  0,  0 },
      { -1, -1, -3, -2, -2, -1, -1, -1,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0, -1, -3, -2, -2, -1, -1, -1,  0 }, },
    { { -1, -2,  0,  1,  1,  1,  1,  1,  1 },
      {  0, -1,  1,  1,  1,  1,  1,  1,  1 },
      {  0, -2,  1,  1,  1,  1,  1,  1,  1 },
      {  0, -2,  0,  1,  1,  1,  1,  1,  1 },
      { -1, -1,  0,  1,  1,  1,  1,  1,  1 },
      {  0,  0, -1,  0,  1,  1,  1,  1,  1 },
      { -1, -1,  1,  1,  1,  1,  1,  1,  1 },
      {  0,  0, -1,  1,  1,  1,  1,  1,  1 },
      {  0, -1,  0,  1,  1,  1,  1,  1,  1 },
      { -1, -1, -1,  1,  1,  1,  1,  1,  1 },
      {  0,  0,  0,  0,  1,  1,  1,  1,  1 },
      {  0,  0,  0,  1,  1,  1,  1,  1,  1 },
      {  0, -1, -1,  1,  1,  1,  1,  1,  1 },
      {  0,  1,  0,  1,  1,  1,  1,  1,  1 },
      {  0, -3, -2,  1,  1,  1,  1,  2,  2 },
      { -3, -5, -3,  2,  2,  2,  2,  2,  2 }, },
    { { -1, -2,  0,  2,  2,  2,  2,  2,  2 },
      { -1, -2,  0,  1,  2,  2,  2,  2,  2 },
      {  0, -2,  0,  2,  2,  2,  2,  2,  2 },
      { -1,  0,  1,  2,  2,  2,  2,  2,  2 },
      {  0,  0,  1,  2,  2,  2,  2,  2,  2 },
      {  0, -2,  0,  1,  2,  2,  2,  2,  2 },
      {  0, -1,  1,  2,  2,  2,  2,  2,  2 },
      { -1, -1,  0,  2,  2,  2,  2,  2,  2 },
      { -1, -1,  0,  1,  2,  2,  2,  2,  2 },
      { -1, -2, -1,  2,  2,  2,  2,  2,  2 },
      {  0, -1,  0,  2,  2,  2,  2,  2,  2 },
      {  1,  1,  0,  1,  2,  2,  2,  2,  2 },
      {  0,  1,  2,  2,  2,  2,  2,  2,  2 },
      {  1,  0,  0,  1,  2,  2,  2,  2,  2 },
      {  0,  0,  0,  1,  2,  2,  2,  2,  2 },
      { -1, -1, -1,  1,  2,  2,  2,  2,  2 }, },
    { {  0,  1,  2,  3,  3,  3,  3,  3,  3 },
      {  1,  1,  2,  3,  3,  3,  3,  3,  3 },
      { -1,  0,  1,  2,  3,  3,  3,  3,  3 },
      {  0,  0,  2,  3,  3,  3,  3,  3,  3 },
      { -1,  0,  1,  3,  3,  3,  3,  3,  3 },
      {  0,  0,  1,  3,  3,  3,  3,  3,  3 },
      {  1,  2,  3,  3,  3,  3,  3,  3,  3 },
      {  1,  2,  2,  3,  3,  3,  3,  3,  3 },
      {  0,  1,  1,  3,  3,  3,  3,  3,  3 },
      {  0,  0,  1,  2,  3,  3,  3,  3,  3 },
      { -1,  1,  2,  3,  3,  3,  3,  3,  3 },
      { -1,  0,  2,  3,  3,  3,  3,  3,  3 },
      {  2,  2,  3,  3,  3,  3,  3,  3,  3 },
      {  1,  1,  3,  3,  3,  3,  3,  3,  3 },
      {  0,  2,  3,  3,  3,  3,  3,  3,  3 },
      {  0,  1,  1,  2,  3,  3,  3,  3,  3 }, },
    { {  0,  1,  2,  3,  4,  4,  4,  4,  4 },
      {  1,  2,  3,  4,  4,  4,  4,  4,  4 },
      {  0,  0,  2,  3,  4,  4,  4,  4,  4 },
      {  1,  1,  2,  4,  4,  4,  4,  4,  4 },
      {  0,  1,  2,  4,  4,  4,  4,  4,  4 },
      { -1,  0,  1,  3,  4,  4,  4,  4,  4 },
      {  0,  0,  1,  3,  4,  4,  4,  4,  4 },
      {  1,  1,  2,  3,  4,  4,  4,  4,  4 },
      {  0,  1,  1,  3,  4,  4,  4,  4,  4 },
      {  2,  2,  3,  4,  4,  4,  4,  4,  4 },
      {  1,  1,  3,  4,  4,  4,  4,  4,  4 },
      {  1,  2,  2,  4,  4,  4,  4,  4,  4 },
      { -1,  0,  2,  3,  4,  4,  4,  4,  4 },
      {  0,  1,  3,  4,  4,  4,  4,  4,  4 },
      {  1,  2,  2,  3,  4,  4,  4,  4,  4 },
      {  0,  2,  3,  4,  4,  4,  4,  4,  4 }, },
    { {  1,  2,  3,  4,  5,  5,  5,  5,  5 },
      {  0,  1,  2,  3,  4,  5,  5,  5,  5 },
      {  0,  1,  2,  3,  5,  5,  5,  5,  5 },
      {  1,  1,  3,  4,  5,  5,  5,  5,  5 },
      {  1,  1,  2,  4,  5,  5,  5,  5,  5 },
      {  1,  2,  2,  4,  5,  5,  5,  5,  5 },
      {  1,  1,  2,  3,  5,  5,  5,  5,  5 },
      {  2,  2,  3,  4,  5,  5,  5,  5,  5 },
      {  0,  1,  2,  4,  5,  5,  5,  5,  5 },
      {  2,  2,  3,  5,  5,  5,  5,  5,  5 },
      {  1,  2,  3,  5,  5,  5,  5,  5,  5 },
      {  0,  1,  3,  4,  5,  5,  5,  5,  5 },
      {  1,  2,  2,  3,  5,  5,  5,  5,  5 },
      {  2,  3,  4,  5,  5,  5,  5,  5,  5 },
      {  0,  2,  3,  4,  5,  5,  5,  5,  5 },
      {  1,  1,  1,  3,  4,  5,  5,  5,  5 }, },
    { {  1,  2,  3,  4,  5,  5,  5,  6,  6 },
      {  1,  2,  3,  4,  5,  6,  6,  6,  6 },
      {  2,  3,  4,  5,  6,  6,  6,  6,  6 },
      {  1,  2,  3,  4,  6,  6,  6,  6,  6 },
      {  2,  2,  3,  4,  5,  5,  5,  6,  6 },
      {  1,  2,  3,  4,  5,  5,  6,  6,  6 },
      {  2,  2,  3,  4,  6,  6,  6,  6,  6 },
      {  2,  2,  3,  4,  5,  6,  6,  6,  6 },
      {  2,  2,  4,  5,  6,  6,  6,  6,  6 },
      {  2,  2,  3,  5,  6,  6,  6,  6,  6 },
      {  1,  2,  3,  5,  6,  6,  6,  6,  6 },
      {  2,  3,  3,  5,  6,  6,  6,  6,  6 },
      {  1,  2,  4,  5,  6,  6,  6,  6,  6 },
      {  2,  2,  3,  4,  5,  5,  6,  6,  6 },
      {  2,  3,  3,  4,  6,  6,  6,  6,  6 },
      {  1,  3,  4,  5,  6,  6,  6,  6,  6 } }
};

/** 2D base shape tables for scale factor coding.
 *  The values are grouped together as follows:
 *  [num_shape_tables = 64][num_seg_coeffs = 9] */
static const int8_t atrac3p_sf_shapes[64][9] = {
    { -3, -2, -1,  0,  3,  5,  6,  8, 40 },
    { -3, -2,  0,  1,  7,  9, 11, 13, 20 },
    { -1,  0,  0,  1,  6,  8, 10, 13, 41 },
    {  0,  0,  0,  2,  5,  5,  6,  8, 14 },
    {  0,  0,  0,  2,  6,  7,  8, 11, 47 },
    {  0,  0,  1,  2,  5,  7,  8, 10, 32 },
    {  0,  0,  1,  3,  8, 10, 12, 14, 47 },
    {  0,  0,  2,  4,  9, 10, 12, 14, 40 },
    {  0,  0,  3,  5,  9, 10, 12, 14, 22 },
    {  0,  1,  3,  5, 10, 14, 18, 22, 31 },
    {  0,  2,  5,  6, 10, 10, 10, 12, 46 },
    {  0,  2,  5,  7, 12, 14, 15, 18, 44 },
    {  1,  1,  4,  5,  7,  7,  8,  9, 15 },
    {  1,  2,  2,  2,  4,  5,  7,  9, 26 },
    {  1,  2,  2,  3,  6,  7,  7,  8, 47 },
    {  1,  2,  2,  3,  6,  8, 10, 13, 22 },
    {  1,  3,  4,  7, 13, 17, 21, 24, 41 },
    {  1,  4,  0,  4, 10, 12, 13, 14, 17 },
    {  2,  3,  3,  3,  6,  8, 10, 13, 48 },
    {  2,  3,  3,  4,  9, 12, 14, 17, 47 },
    {  2,  3,  3,  5, 10, 12, 14, 17, 25 },
    {  2,  3,  5,  7,  8,  9,  9,  9, 13 },
    {  2,  3,  5,  9, 16, 21, 25, 28, 33 },
    {  2,  4,  5,  8, 12, 14, 17, 19, 26 },
    {  2,  4,  6,  8, 12, 13, 13, 15, 20 },
    {  2,  4,  7, 12, 20, 26, 30, 32, 35 },
    {  3,  3,  5,  6, 12, 14, 16, 19, 34 },
    {  3,  4,  4,  5,  7,  9, 10, 11, 48 },
    {  3,  4,  5,  6,  8,  9, 10, 11, 16 },
    {  3,  5,  5,  5,  7,  9, 10, 13, 35 },
    {  3,  5,  5,  7, 10, 12, 13, 15, 49 },
    {  3,  5,  7,  7,  8,  7,  9, 12, 21 },
    {  3,  5,  7,  8, 12, 14, 15, 15, 24 },
    {  3,  5,  7, 10, 16, 21, 24, 27, 44 },
    {  3,  5,  8, 14, 21, 26, 28, 29, 42 },
    {  3,  6, 10, 13, 18, 19, 20, 22, 27 },
    {  3,  6, 11, 16, 24, 27, 28, 29, 31 },
    {  4,  5,  4,  3,  4,  6,  8, 11, 18 },
    {  4,  6,  5,  6,  9, 10, 12, 14, 20 },
    {  4,  6,  7,  6,  6,  6,  7,  8, 46 },
    {  4,  6,  7,  9, 13, 16, 18, 20, 48 },
    {  4,  6,  7,  9, 14, 17, 20, 23, 31 },
    {  4,  6,  9, 11, 14, 15, 15, 17, 21 },
    {  4,  8, 13, 20, 27, 32, 35, 36, 38 },
    {  5,  6,  6,  4,  5,  6,  7,  6,  6 },
    {  5,  7,  7,  8,  9,  9, 10, 12, 49 },
    {  5,  8,  9,  9, 10, 11, 12, 13, 42 },
    {  5,  8, 10, 12, 15, 16, 17, 19, 42 },
    {  5,  8, 12, 17, 26, 31, 32, 33, 44 },
    {  5,  9, 13, 16, 20, 22, 23, 23, 35 },
    {  6,  8,  8,  7,  6,  5,  6,  8, 15 },
    {  6,  8,  8,  8,  9, 10, 12, 16, 24 },
    {  6,  8,  8,  9, 10, 10, 11, 11, 13 },
    {  6,  8, 10, 13, 19, 21, 24, 26, 32 },
    {  6,  9, 10, 11, 13, 13, 14, 16, 49 },
    {  7,  9,  9, 10, 13, 14, 16, 19, 27 },
    {  7, 10, 12, 13, 16, 16, 17, 17, 27 },
    {  7, 10, 12, 14, 17, 19, 20, 22, 48 },
    {  8,  9, 10,  9, 10, 11, 11, 11, 19 },
    {  8, 11, 12, 12, 13, 13, 13, 13, 17 },
    {  8, 11, 13, 14, 16, 17, 19, 20, 27 },
    {  8, 12, 17, 22, 26, 28, 29, 30, 33 },
    { 10, 14, 16, 19, 21, 22, 22, 24, 28 },
    { 10, 15, 17, 18, 21, 22, 23, 25, 43 }
};

static const uint8_t atrac3p_ct_restricted_to_full[2][7][4] = {
    { { 0, 5, 4, 1 },
      { 0, 1, 2, 3 },
      { 3, 0, 4, 2 },
      { 4, 0, 1, 2 },
      { 1, 0, 4, 3 },
      { 3, 0, 2, 1 },
      { 0, 3, 1, 2 } },
    { { 4, 0, 1, 2 },
      { 0, 3, 2, 1 },
      { 0, 1, 2, 3 },
      { 0, 1, 2, 4 },
      { 0, 1, 2, 3 },
      { 1, 4, 2, 0 },
      { 0, 1, 2, 3 } }
};

/** Tables for spectrum coding */
static const uint8_t huff_a01_cb[14] = {
    1, 12, 1, 0, 0, 1, 7, 0, 19, 5, 13, 21, 6, 8
};

static const uint8_t huff_a01_xlat[81] = {
    0x00, 0x03, 0x40, 0xC0, 0x10, 0x30, 0x04, 0x0C, 0x01, 0x50, 0xD0, 0x70,
    0xF0, 0xC4, 0x14, 0x34, 0x4C, 0x1C, 0x3C, 0x41, 0xC1, 0x31, 0x05, 0x0D,
    0xC3, 0x13, 0x07, 0x0F, 0x44, 0xCC, 0x11, 0x43, 0x33, 0x54, 0x74, 0xDC,
    0xFC, 0x71, 0x15, 0x4D, 0xCD, 0x1D, 0xD3, 0xC7, 0x37, 0x3F, 0xD4, 0xF4,
    0x5C, 0x7C, 0x51, 0xD1, 0xF1, 0x45, 0xC5, 0x35, 0xDD, 0x3D, 0x53, 0x73,
    0xF3, 0x47, 0x17, 0x77, 0x4F, 0xCF, 0x1F, 0x55, 0xF5, 0x7D, 0xD7, 0x5F,
    0xFF, 0xD5, 0x75, 0x5D, 0xFD, 0x57, 0xF7, 0xDF, 0x7F
};

static const uint8_t huff_a02_cb[13] = {
    2, 12, 1, 0, 4, 11, 0, 1, 29, 6, 20, 7, 2
};

static const uint8_t huff_a02_xlat[81] = {
    0x00, 0x40, 0x10, 0x04, 0x01, 0x50, 0x44, 0x14, 0x54, 0x41, 0x11, 0x51,
    0x05, 0x45, 0x15, 0x55, 0x90, 0x80, 0x20, 0x60, 0x84, 0x94, 0x24, 0x64,
    0x08, 0x48, 0x18, 0x58, 0x81, 0x91, 0x21, 0x85, 0x95, 0x65, 0x09, 0x49,
    0x19, 0x59, 0x02, 0x42, 0x12, 0x52, 0x06, 0x46, 0x16, 0x56, 0x88, 0x61,
    0x25, 0x29, 0x69, 0x5A, 0xA0, 0xA4, 0x98, 0x28, 0x68, 0xA1, 0xA5, 0x89,
    0x99, 0xA9, 0x82, 0x92, 0x22, 0x62, 0x96, 0x26, 0x66, 0x0A, 0x4A, 0x1A,
    0xA8, 0x86, 0xA6, 0x8A, 0x9A, 0x2A, 0x6A, 0xA2, 0xAA
};

static const uint8_t huff_a03_cb[9] = { 3, 9, 1, 8, 0, 13, 18, 7, 2 };

static const uint8_t huff_a03_xlat[49] = {
    0x00, 0x08, 0x38, 0x01, 0x09, 0x39, 0x07, 0x0F, 0x3F, 0x10, 0x30, 0x11,
    0x31, 0x02, 0x0A, 0x3A, 0x05, 0x06, 0x0E, 0x3E, 0x17, 0x37, 0x18, 0x28,
    0x19, 0x29, 0x2A, 0x32, 0x03, 0x0B, 0x33, 0x3B, 0x0D, 0x15, 0x3D, 0x16,
    0x1E, 0x36, 0x1F, 0x2F, 0x12, 0x1A, 0x13, 0x2B, 0x1D, 0x35, 0x2E, 0x1B,
    0x2D
};

static const uint8_t huff_a04_cb[4]   = { 2, 3, 2, 4 };
static const uint8_t huff_a04_xlat[6] = { 1, 2, 0, 3, 4, 5 };

static const uint8_t huff_a05_cb[12] = {
    3, 12, 1, 3, 5, 8, 12, 23, 72, 68, 31, 2
};

static const uint8_t huff_a05_xlat[225] = {
    0x00, 0x10, 0xF0, 0x01, 0x11, 0xF1, 0x0F, 0x1F, 0xFF, 0x20, 0xE0, 0xE1,
    0x02, 0xF2, 0x0E, 0x1E, 0x2F, 0x30, 0xD0, 0x21, 0x12, 0x22, 0xE2, 0x03,
    0x0D, 0x2E, 0xEE, 0xFE, 0xEF, 0x40, 0xC0, 0x31, 0xC1, 0xD1, 0x32, 0xD2,
    0x13, 0x23, 0xE3, 0xF3, 0x04, 0xF4, 0x0C, 0x1C, 0x1D, 0x2D, 0xED, 0xFD,
    0x3E, 0xDE, 0x3F, 0xDF, 0x50, 0x60, 0x70, 0x90, 0xA0, 0xB0, 0x41, 0x51,
    0x61, 0x71, 0x91, 0xA1, 0xB1, 0x42, 0x62, 0x92, 0xA2, 0xC2, 0x33, 0xC3,
    0xD3, 0x14, 0x24, 0x34, 0xD4, 0xE4, 0x05, 0x15, 0xF5, 0x06, 0x16, 0x26,
    0xE6, 0xF6, 0x07, 0x17, 0xE7, 0xF7, 0x09, 0x19, 0x29, 0xF9, 0x0A, 0x1A,
    0x2A, 0xEA, 0xFA, 0x0B, 0x1B, 0xFB, 0x2C, 0x3C, 0xDC, 0xEC, 0xFC, 0x3D,
    0x4D, 0xCD, 0xDD, 0x4E, 0x6E, 0x7E, 0xAE, 0xCE, 0x4F, 0x5F, 0x6F, 0x7F,
    0x9F, 0xAF, 0xBF, 0xCF, 0x52, 0x72, 0xB2, 0x43, 0x53, 0x63, 0x73, 0x93,
    0xA3, 0xB3, 0x44, 0x64, 0x74, 0x94, 0xA4, 0xB4, 0xC4, 0x25, 0x35, 0xA5,
    0xC5, 0xD5, 0xE5, 0x36, 0x46, 0xB6, 0xC6, 0xD6, 0x27, 0x37, 0x47, 0xB7,
    0xC7, 0xD7, 0x39, 0x49, 0x59, 0xC9, 0xD9, 0xE9, 0x3A, 0x4A, 0x5A, 0xCA,
    0xDA, 0x2B, 0x3B, 0x4B, 0x6B, 0x7B, 0xDB, 0xEB, 0x4C, 0x5C, 0x6C, 0x7C,
    0x9C, 0xAC, 0xCC, 0x5D, 0x6D, 0x7D, 0x9D, 0xAD, 0xBD, 0x5E, 0x9E, 0xBE,
    0x54, 0x45, 0x55, 0x65, 0x75, 0x95, 0xB5, 0x56, 0x66, 0x76, 0x96, 0xA6,
    0x57, 0x67, 0x97, 0xA7, 0x69, 0x79, 0xA9, 0xB9, 0x6A, 0x7A, 0x9A, 0xAA,
    0xBA, 0x5B, 0x9B, 0xAB, 0xBB, 0xCB, 0xBC, 0x77, 0x99
};

static const uint8_t huff_a06_cb[7] = {
    2, 6, 1, 3, 2, 6, 4
};

static const uint8_t huff_a06_xlat[16] = {
    1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 13, 14, 10, 11, 12, 15
};

static const uint8_t huff_a07_cb[11] = {
    2, 10, 1, 2, 2, 2, 6, 14, 21, 13, 2
};

static const uint8_t huff_a07_xlat[63] = {
    0,   1, 63,  2, 62,  3, 61,  4,  5,  6, 58, 59, 60,  7,  8,  9,
    10, 26, 27, 28, 36, 37, 38, 54, 55, 56, 57, 11, 12, 13, 14, 15,
    16, 25, 29, 30, 31, 33, 34, 35, 39, 47, 48, 49, 50, 51, 52, 53,
    17, 18, 19, 20, 21, 22, 23, 41, 42, 43, 44, 45, 46, 24, 40
};

static const uint8_t huff_a11_cb[13] = {
    1, 11, 1, 0, 0, 0, 8, 1, 18, 9, 22, 10, 12
};

static const uint8_t huff_a11_xlat[81] = {
    0x00, 0x40, 0xC0, 0x10, 0x30, 0x04, 0x0C, 0x01, 0x03, 0xD0, 0x50, 0x70,
    0xF0, 0xC4, 0x34, 0x4C, 0xCC, 0x1C, 0x41, 0xC1, 0x31, 0x05, 0x0D, 0x43,
    0xC3, 0x13, 0x07, 0x0F, 0x44, 0x14, 0x74, 0xDC, 0x3C, 0x11, 0x1D, 0x33,
    0x37, 0x54, 0xD4, 0xF4, 0x5C, 0x7C, 0xFC, 0xD1, 0x71, 0xF1, 0x15, 0x35,
    0x4D, 0xCD, 0xDD, 0x3D, 0xD3, 0x73, 0x47, 0xC7, 0x17, 0x77, 0x3F, 0x51,
    0x45, 0xC5, 0x55, 0x53, 0xF3, 0x4F, 0xCF, 0x1F, 0xFF, 0xD5, 0x75, 0xF5,
    0x5D, 0x7D, 0xFD, 0x57, 0xD7, 0xF7, 0x5F, 0xDF, 0x7F
};

static const uint8_t huff_a12_cb[8] = { 5, 10, 16, 11, 32, 19, 1, 2 };

static const uint8_t huff_a12_xlat[81] = {
    0x00, 0x40, 0x10, 0x50, 0x04, 0x44, 0x14, 0x54, 0x01, 0x41, 0x11, 0x51,
    0x05, 0x45, 0x15, 0x55, 0x90, 0x94, 0x58, 0x91, 0x95, 0x19, 0x59, 0x06,
    0x46, 0x16, 0x56, 0x80, 0x60, 0x84, 0x24, 0x64, 0xA4, 0x08, 0x48, 0x18,
    0x68, 0x81, 0x21, 0x61, 0xA1, 0x85, 0x25, 0x65, 0xA5, 0x09, 0x49, 0x99,
    0x69, 0xA9, 0x02, 0x42, 0x12, 0x52, 0x96, 0x26, 0x66, 0x1A, 0x5A, 0x20,
    0xA0, 0x88, 0x98, 0x28, 0xA8, 0x89, 0x29, 0x82, 0x92, 0x22, 0x62, 0x86,
    0xA6, 0x0A, 0x4A, 0x9A, 0x6A, 0xAA, 0xA2, 0x8A, 0x2A
};

static const uint8_t huff_a13_cb[12] = {
    1, 10, 1, 0, 0, 4, 2, 2, 9, 15, 12, 4
};

static const uint8_t huff_a13_xlat[49] = {
    0x00, 0x08, 0x38, 0x01, 0x07, 0x39, 0x0F, 0x09, 0x3F, 0x10, 0x30, 0x31,
    0x02, 0x3A, 0x06, 0x0E, 0x3E, 0x17, 0x18, 0x28, 0x11, 0x29, 0x0A, 0x32,
    0x03, 0x0B, 0x3B, 0x05, 0x0D, 0x3D, 0x16, 0x1F, 0x37, 0x19, 0x12, 0x1A,
    0x2A, 0x13, 0x33, 0x15, 0x35, 0x1E, 0x2E, 0x36, 0x2F, 0x1B, 0x2B, 0x1D,
    0x2D
};

static const uint8_t huff_a14_cb[12] = {
    2, 11, 1, 0, 4, 3, 5, 16, 28, 34, 26, 4
};

static const uint8_t huff_a14_xlat[121] = {
    0x00, 0x10, 0xF0, 0x01, 0x0F, 0xF1, 0x1F, 0xFF, 0x20, 0xE0, 0x11, 0x02,
    0x0E, 0x30, 0x50, 0xB0, 0xD0, 0x21, 0xE1, 0x12, 0xF2, 0x03, 0x05, 0x0B,
    0x0D, 0x1E, 0xFE, 0x2F, 0xEF, 0x40, 0xC0, 0x31, 0x51, 0xB1, 0xC1, 0xD1,
    0x22, 0x52, 0xE2, 0x13, 0xF3, 0x04, 0x15, 0xF5, 0x1B, 0xEB, 0xFB, 0x0C,
    0x1D, 0xFD, 0x2E, 0x5E, 0xEE, 0x3F, 0x5F, 0xBF, 0xDF, 0x41, 0x32, 0x42,
    0xB2, 0xD2, 0x23, 0x53, 0xB3, 0xE3, 0x14, 0x24, 0xE4, 0xF4, 0x25, 0x35,
    0xD5, 0xE5, 0x2B, 0x3B, 0xDB, 0x1C, 0x2C, 0xBC, 0xEC, 0xFC, 0x2D, 0xBD,
    0xED, 0x3E, 0x4E, 0xBE, 0xDE, 0x4F, 0xCF, 0xC2, 0x33, 0x43, 0xC3, 0xD3,
    0x34, 0x44, 0x54, 0xB4, 0xD4, 0x45, 0x55, 0xC5, 0x4B, 0xCB, 0x3C, 0x4C,
    0x5C, 0xCC, 0xDC, 0x3D, 0x4D, 0x5D, 0xCD, 0xDD, 0xCE, 0xC4, 0xB5, 0x5B,
    0xBB
};

static const uint8_t huff_a15_cb[9] = { 5, 11, 9, 12, 16, 44, 98, 42, 4 };

static const uint8_t huff_a15_xlat[225] = {
    0x00, 0x10, 0xF0, 0x01, 0x11, 0xF1, 0x0F, 0x1F, 0xFF, 0x20, 0xE0, 0x21,
    0xE1, 0x02, 0x12, 0xF2, 0x0E, 0x1E, 0xFE, 0x2F, 0xEF, 0x30, 0xD0, 0x31,
    0xD1, 0x22, 0xE2, 0x03, 0x13, 0xF3, 0x0D, 0x1D, 0xFD, 0x2E, 0xEE, 0x3F,
    0xDF, 0x40, 0x60, 0x70, 0x90, 0xA0, 0xC0, 0x41, 0xC1, 0x32, 0x42, 0xC2,
    0xD2, 0x23, 0x33, 0xD3, 0xE3, 0x04, 0x14, 0x24, 0xE4, 0xF4, 0x06, 0x16,
    0xF6, 0x07, 0x09, 0x0A, 0x1A, 0xFA, 0x0C, 0x1C, 0x2C, 0xEC, 0xFC, 0x2D,
    0x3D, 0xDD, 0xED, 0x3E, 0x4E, 0xCE, 0xDE, 0x4F, 0xCF, 0x50, 0xB0, 0x51,
    0x61, 0x71, 0x91, 0xA1, 0xB1, 0x52, 0x62, 0x72, 0x92, 0xA2, 0xB2, 0x43,
    0x53, 0x63, 0x73, 0x93, 0xA3, 0xC3, 0x34, 0x44, 0x64, 0xA4, 0xC4, 0xD4,
    0x05, 0x15, 0x25, 0x35, 0xD5, 0xE5, 0xF5, 0x26, 0x36, 0x46, 0xC6, 0xD6,
    0xE6, 0x17, 0x27, 0x37, 0xC7, 0xD7, 0xE7, 0xF7, 0x19, 0x29, 0x39, 0xC9,
    0xD9, 0xE9, 0xF9, 0x2A, 0x3A, 0x4A, 0x5A, 0xCA, 0xDA, 0xEA, 0x0B, 0x1B,
    0x2B, 0x3B, 0xCB, 0xDB, 0xEB, 0xFB, 0x3C, 0x4C, 0x6C, 0x7C, 0x9C, 0xAC,
    0xBC, 0xCC, 0xDC, 0x4D, 0x5D, 0x6D, 0x7D, 0x9D, 0xAD, 0xBD, 0xCD, 0x5E,
    0x6E, 0x7E, 0x9E, 0xAE, 0xBE, 0x5F, 0x6F, 0x7F, 0x9F, 0xAF, 0xBF, 0xB3,
    0x54, 0x74, 0x94, 0xB4, 0x45, 0x55, 0x65, 0x75, 0x95, 0xA5, 0xB5, 0xC5,
    0x56, 0x66, 0x76, 0x96, 0xA6, 0xB6, 0x47, 0x57, 0x67, 0xA7, 0xB7, 0x49,
    0x59, 0x69, 0xA9, 0xB9, 0x6A, 0x7A, 0x9A, 0xAA, 0xBA, 0x4B, 0x5B, 0x6B,
    0x7B, 0x9B, 0xAB, 0xBB, 0x5C, 0x77, 0x97, 0x79, 0x99
};

static const uint8_t huff_a16_cb[13] = {
    2, 12, 1, 1, 2, 2, 5, 7, 21, 54, 85, 62, 16
};

static const uint8_t huff_a16_xlat[256] = {
    0x00, 0x01, 0x10, 0x11, 0x21, 0x12, 0x20, 0x31, 0x02, 0x22, 0x13, 0x30,
    0x41, 0x32, 0x03, 0x23, 0x14, 0x24, 0x40, 0x51, 0x61, 0xD1, 0xE1, 0x42,
    0x52, 0xD2, 0x33, 0x43, 0xD3, 0x04, 0x34, 0x05, 0x15, 0x25, 0x16, 0x1D,
    0x2D, 0x1E, 0x2E, 0x50, 0x60, 0xD0, 0xE0, 0xF0, 0x71, 0x81, 0xF1, 0x62,
    0x72, 0xE2, 0xF2, 0x53, 0x63, 0xE3, 0xF3, 0x44, 0x54, 0xD4, 0xE4, 0xF4,
    0x35, 0x45, 0x55, 0xD5, 0xE5, 0xF5, 0x06, 0x26, 0x36, 0xD6, 0x07, 0x17,
    0x27, 0x37, 0xD7, 0x18, 0x28, 0x1C, 0x0D, 0x3D, 0x4D, 0x5D, 0x6D, 0x8D,
    0x0E, 0x3E, 0x4E, 0x5E, 0x0F, 0x1F, 0x2F, 0x3F, 0x5F, 0x70, 0x80, 0x90,
    0xC0, 0x91, 0xA1, 0xB1, 0xC1, 0x82, 0x92, 0xA2, 0xC2, 0x73, 0x83, 0x93,
    0xA3, 0xC3, 0x64, 0x74, 0x84, 0x94, 0xA4, 0xC4, 0x65, 0x75, 0x85, 0x46,
    0x56, 0x66, 0xC6, 0xE6, 0xF6, 0x47, 0x57, 0xE7, 0xF7, 0x08, 0x38, 0x48,
    0x58, 0x68, 0xD8, 0xE8, 0xF8, 0x09, 0x19, 0x29, 0x39, 0x59, 0xD9, 0xE9,
    0xF9, 0x1A, 0x2A, 0x3A, 0xDA, 0xEA, 0xFA, 0x1B, 0x2B, 0xDB, 0xEB, 0xFB,
    0x0C, 0x2C, 0x3C, 0xDC, 0xEC, 0x7D, 0x9D, 0xAD, 0xBD, 0xCD, 0x6E, 0x7E,
    0x8E, 0x9E, 0xAE, 0xBE, 0xCE, 0x4F, 0x6F, 0x7F, 0x8F, 0xAF, 0xA0, 0xB2,
    0xB3, 0xB4, 0x95, 0xA5, 0xB5, 0xC5, 0x76, 0x86, 0x96, 0xA6, 0xB6, 0x67,
    0x77, 0x87, 0x97, 0xC7, 0x78, 0x88, 0x98, 0xC8, 0x49, 0x69, 0x79, 0x89,
    0x99, 0xC9, 0x0A, 0x4A, 0x5A, 0x6A, 0x7A, 0xCA, 0x0B, 0x3B, 0x4B, 0x5B,
    0x6B, 0xCB, 0x4C, 0x5C, 0x6C, 0x7C, 0x8C, 0x9C, 0xAC, 0xBC, 0xCC, 0xFC,
    0xDD, 0xED, 0xFD, 0xDE, 0xEE, 0xFE, 0x9F, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF,
    0xB0, 0xA7, 0xB7, 0xA8, 0xB8, 0xA9, 0xB9, 0x8A, 0x9A, 0xAA, 0xBA, 0x7B,
    0x8B, 0x9B, 0xAB, 0xBB
};

static const uint8_t huff_a17_cb[9] = { 3, 9, 3, 2, 5, 7, 17, 23, 6 };

static const uint8_t huff_a17_xlat[63] = {
    0,   1, 63,  2, 62,  3,  4, 59, 60, 61,  5,  6,  7,  8, 56, 57,
    58,  9, 10, 11, 12, 13, 14, 26, 27, 36, 37, 38, 50, 51, 52, 53,
    54, 55, 15, 16, 17, 18, 19, 20, 21, 25, 28, 29, 30, 31, 33, 34,
    35, 39, 43, 44, 45, 46, 47, 48, 49, 22, 23, 24, 40, 41, 42
};

static const uint8_t huff_a21_cb[14] = {
    1, 12, 1, 0, 0, 2, 6, 0, 7, 21, 15, 17, 8, 4
};

static const uint8_t huff_a21_xlat[81] = {
    0x00, 0x40, 0xC0, 0x10, 0x30, 0x04, 0x0C, 0x01, 0x03, 0xD0, 0x70, 0x34,
    0x1C, 0x0D, 0x13, 0x07, 0x50, 0xF0, 0x44, 0xC4, 0x14, 0x74, 0x4C, 0xCC,
    0xDC, 0x3C, 0x41, 0xC1, 0x11, 0x31, 0x05, 0x1D, 0x43, 0xC3, 0x33, 0x37,
    0x0F, 0x54, 0xF4, 0xFC, 0xD1, 0x71, 0x15, 0x4D, 0xCD, 0xDD, 0xD3, 0x73,
    0x47, 0xC7, 0x77, 0x3F, 0xD4, 0x5C, 0x7C, 0x51, 0xF1, 0x45, 0xC5, 0x55,
    0x35, 0x3D, 0x53, 0xF3, 0x17, 0x4F, 0xCF, 0x1F, 0xFF, 0x75, 0xF5, 0x5D,
    0x7D, 0xD7, 0xF7, 0x5F, 0xDF, 0xD5, 0xFD, 0x57, 0x7F
};

static const uint8_t huff_a22_cb[10] = { 2, 9, 1, 4, 0, 4, 3, 8, 3, 2 };

static const uint8_t huff_a22_xlat[25] = {
    0x00, 0x08, 0x38, 0x01, 0x07, 0x09, 0x39, 0x0F, 0x3F, 0x10, 0x02, 0x06,
    0x30, 0x11, 0x31, 0x0A, 0x3A, 0x0E, 0x17, 0x37, 0x32, 0x16, 0x3E, 0x12,
    0x36
};

static const uint8_t huff_a23_cb[9] = { 3, 9, 5, 0, 4, 6, 10, 16, 8 };

static const uint8_t huff_a23_xlat[49] = {
    0x00, 0x08, 0x38, 0x01, 0x07, 0x09, 0x39, 0x0F, 0x3F, 0x10, 0x30, 0x02,
    0x3A, 0x06, 0x0E, 0x18, 0x28, 0x11, 0x31, 0x0A, 0x03, 0x05, 0x3E, 0x17,
    0x37, 0x19, 0x29, 0x12, 0x2A, 0x32, 0x0B, 0x33, 0x3B, 0x0D, 0x15, 0x3D,
    0x16, 0x1E, 0x36, 0x1F, 0x2F, 0x1A, 0x13, 0x1B, 0x2B, 0x1D, 0x2D, 0x35,
    0x2E
};

static const uint8_t huff_a24_cb[5] = { 2, 4, 3, 1, 2 };

static const uint8_t huff_a25_cb[5] = { 2, 4, 1, 5, 2 };

static const uint8_t huff_a25_xlat[8] = { 1, 0, 2, 3, 4, 5, 6, 7 };

static const uint8_t huff_a26_cb[10] = { 4, 11, 3, 4, 12, 15, 34, 83, 75, 30 };

static const uint8_t huff_a26_xlat[256] = {
    0x00, 0x01, 0x11, 0x10, 0x21, 0x12, 0x22, 0x20, 0x30, 0x31, 0x41, 0x02,
    0x32, 0x03, 0x13, 0x23, 0x33, 0x14, 0x24, 0x40, 0x51, 0x61, 0x42, 0x52,
    0x43, 0x53, 0x04, 0x34, 0x44, 0x15, 0x25, 0x35, 0x16, 0x26, 0x50, 0x60,
    0x71, 0x81, 0xD1, 0x62, 0x72, 0x82, 0xD2, 0x63, 0x73, 0xD3, 0x54, 0x64,
    0x05, 0x45, 0x55, 0x65, 0x06, 0x36, 0x46, 0x56, 0x17, 0x27, 0x37, 0x47,
    0x18, 0x28, 0x38, 0x19, 0x1D, 0x2D, 0x3D, 0x1E, 0x70, 0x80, 0x90, 0xD0,
    0xE0, 0x91, 0xA1, 0xB1, 0xC1, 0xE1, 0xF1, 0x92, 0xA2, 0xC2, 0xE2, 0xF2,
    0x83, 0x93, 0xA3, 0xC3, 0xE3, 0xF3, 0x74, 0x84, 0x94, 0xA4, 0xC4, 0xD4,
    0xE4, 0xF4, 0x75, 0x85, 0x95, 0xD5, 0xE5, 0x66, 0x76, 0x86, 0xD6, 0xE6,
    0x07, 0x57, 0x67, 0x77, 0xD7, 0x08, 0x48, 0x58, 0x68, 0xD8, 0x09, 0x29,
    0x39, 0x49, 0x59, 0x69, 0x1A, 0x2A, 0x3A, 0x4A, 0x1B, 0x2B, 0x1C, 0x2C,
    0x3C, 0x4C, 0x0D, 0x4D, 0x5D, 0x6D, 0x7D, 0x8D, 0x0E, 0x2E, 0x3E, 0x4E,
    0x5E, 0x6E, 0x1F, 0x2F, 0x3F, 0x4F, 0x5F, 0xA0, 0xB0, 0xC0, 0xF0, 0xB2,
    0xB3, 0xB4, 0xA5, 0xB5, 0xC5, 0xF5, 0x96, 0xA6, 0xB6, 0xC6, 0xF6, 0x87,
    0x97, 0xA7, 0xB7, 0xC7, 0xE7, 0xF7, 0x78, 0x88, 0x98, 0xA8, 0xC8, 0xE8,
    0xF8, 0x79, 0x89, 0x99, 0xC9, 0xD9, 0xE9, 0xF9, 0x0A, 0x5A, 0x6A, 0x7A,
    0x8A, 0xDA, 0xEA, 0xFA, 0x0B, 0x3B, 0x4B, 0x5B, 0x6B, 0x7B, 0x8B, 0xDB,
    0x0C, 0x5C, 0x6C, 0x7C, 0x8C, 0x9C, 0xDC, 0x9D, 0xAD, 0xBD, 0xCD, 0x7E,
    0x8E, 0x9E, 0xAE, 0xBE, 0x0F, 0x6F, 0x7F, 0x8F, 0x9F, 0xAF, 0xB8, 0xA9,
    0xB9, 0x9A, 0xAA, 0xBA, 0xCA, 0x9B, 0xAB, 0xBB, 0xCB, 0xEB, 0xFB, 0xAC,
    0xBC, 0xCC, 0xEC, 0xFC, 0xDD, 0xED, 0xFD, 0xCE, 0xDE, 0xEE, 0xFE, 0xBF,
    0xCF, 0xDF, 0xEF, 0xFF
};

static const uint8_t huff_a27_cb[7] = { 4, 8, 3, 14, 10, 20, 16 };

static const uint8_t huff_a27_xlat[63] = {
     0,  2,  3,  1,  5,  6,  7,  9, 54, 55, 56, 58, 59, 60, 61, 62,
    63,  4,  8, 10, 11, 12, 14, 49, 52, 53, 57, 13, 15, 16, 17, 18,
    19, 22, 23, 25, 26, 30, 39, 43, 44, 45, 46, 47, 48, 50, 51, 20,
    21, 24, 27, 28, 29, 31, 33, 34, 35, 36, 37, 38, 40, 41, 42
};

static const uint8_t huff_a31_cb[8] = { 1, 6, 1, 0, 3, 1, 0, 4 };

static const uint8_t huff_a31_xlat[9] = {
    0x00, 0x04, 0x0C, 0x01, 0x03, 0x05, 0x0D, 0x07, 0x0F
};

static const uint8_t huff_a32_cb[13] = {
    1, 11, 1, 0, 0, 2, 2, 6, 12, 18, 19, 15, 6
};

static const uint8_t huff_a32_xlat[81] = {
    0x00, 0x40, 0x01, 0x10, 0x04, 0x80, 0x50, 0x20, 0x14, 0x05, 0x02, 0x90,
    0x60, 0x44, 0x54, 0x24, 0x08, 0x18, 0x41, 0x11, 0x15, 0x09, 0x06, 0xA0,
    0x84, 0x94, 0x64, 0xA4, 0x48, 0x58, 0x28, 0x51, 0x21, 0x45, 0x55, 0x25,
    0x19, 0x12, 0x16, 0x0A, 0x1A, 0x68, 0xA8, 0x81, 0x91, 0x61, 0xA1, 0x85,
    0x95, 0x65, 0xA5, 0x49, 0x59, 0x29, 0x69, 0x42, 0x52, 0x46, 0x56, 0x2A,
    0x88, 0x98, 0x89, 0x99, 0xA9, 0x82, 0x92, 0x22, 0x62, 0x86, 0x26, 0x66,
    0x4A, 0x5A, 0x6A, 0xA2, 0x96, 0xA6, 0x8A, 0x9A, 0xAA
};

static const uint8_t huff_a33_cb[12] = {
    3, 12, 1, 1, 13, 1, 14, 28, 33, 81, 32, 52
};

static const uint8_t huff_a33_xlat[256] = {
    0x00, 0x10, 0x40, 0x50, 0x04, 0x44, 0x14, 0x54, 0x01, 0x41, 0x11, 0x51,
    0x05, 0x45, 0x15, 0x55, 0x90, 0x20, 0x94, 0x64, 0x18, 0x21, 0x95, 0x19,
    0x69, 0x02, 0x52, 0x06, 0x46, 0x16, 0x80, 0x60, 0x84, 0xD4, 0x24, 0x08,
    0x48, 0x58, 0x68, 0x81, 0x91, 0x61, 0x85, 0x25, 0x65, 0xA5, 0x09, 0x49,
    0x59, 0x29, 0x42, 0x12, 0x56, 0x96, 0xA6, 0x0A, 0x17, 0x1B, 0xD0, 0xC4,
    0x74, 0xF4, 0x88, 0xC8, 0x28, 0xA1, 0x71, 0xC5, 0xD5, 0x75, 0x99, 0xB9,
    0x4D, 0x1D, 0x2D, 0x6D, 0x22, 0x62, 0x66, 0x4A, 0x1A, 0x9A, 0x6A, 0x8E,
    0x5E, 0x43, 0x23, 0x07, 0x47, 0x57, 0x6B, 0xC0, 0xA0, 0xE0, 0x70, 0xB0,
    0xA4, 0xE4, 0x34, 0xB4, 0x98, 0xD8, 0xA8, 0x38, 0x78, 0x0C, 0x4C, 0x1C,
    0x5C, 0x9C, 0x6C, 0x7C, 0xC1, 0xD1, 0xE1, 0x31, 0xE5, 0x35, 0xB5, 0xF5,
    0x89, 0xA9, 0x79, 0xF9, 0x0D, 0xCD, 0x9D, 0xDD, 0xAD, 0x3D, 0x7D, 0x82,
    0xC2, 0x92, 0xD2, 0xE2, 0x72, 0xF2, 0x86, 0xD6, 0xE6, 0x76, 0xB6, 0x8A,
    0x5A, 0xDA, 0xEA, 0xFA, 0x4E, 0x1E, 0x9E, 0xEE, 0x03, 0x13, 0x53, 0x97,
    0xB7, 0x0B, 0x4B, 0x8B, 0x5B, 0x9B, 0xEB, 0x7B, 0x0F, 0x4F, 0x1F, 0x5F,
    0x9F, 0x2F, 0x3F, 0xBF, 0xE8, 0xB8, 0xF8, 0x8C, 0x2C, 0x3C, 0xFC, 0xB1,
    0xC9, 0xD9, 0xE9, 0x39, 0x5D, 0xED, 0xBD, 0xA2, 0x32, 0x26, 0x36, 0x2A,
    0xAA, 0xBA, 0x0E, 0x2E, 0x6E, 0x83, 0xC3, 0x93, 0x63, 0xB3, 0xA7, 0x37,
    0x30, 0xF0, 0xCC, 0xDC, 0xAC, 0xEC, 0xBC, 0xF1, 0x8D, 0xFD, 0xB2, 0xC6,
    0xF6, 0xCA, 0x3A, 0x7A, 0xCE, 0xDE, 0xAE, 0x3E, 0x7E, 0xBE, 0xFE, 0xD3,
    0xA3, 0xE3, 0x33, 0x73, 0xF3, 0x87, 0xC7, 0xD7, 0x27, 0x67, 0xE7, 0x77,
    0xF7, 0xCB, 0xDB, 0x2B, 0xAB, 0x3B, 0xBB, 0xFB, 0x8F, 0xCF, 0xDF, 0x6F,
    0xAF, 0xEF, 0x7F, 0xFF
};

static const uint8_t huff_a34_cb[7] = { 1, 5, 1, 1, 1, 1, 2 };

static const uint8_t huff_a34_xlat[6] = { 1, 0, 2, 3, 4, 5 };

static const uint8_t huff_a35_cb[11] = { 2, 10, 1, 0, 2, 3, 6, 19, 9, 75, 110 };

static const uint8_t huff_a35_xlat[225] = {
    0x00, 0xF0, 0x0F, 0x10, 0x01, 0xFF, 0x20, 0xE0, 0x11, 0xF1, 0x0E, 0x1F,
    0x30, 0x40, 0xD0, 0x21, 0xE1, 0x02, 0x12, 0x22, 0xE2, 0xF2, 0x03, 0x13,
    0x1E, 0x2E, 0x3E, 0xEE, 0xFE, 0x2F, 0xEF, 0xD2, 0x43, 0xF3, 0x04, 0x0D,
    0x2D, 0x3D, 0x3F, 0xDF, 0x50, 0x60, 0x70, 0x90, 0xB0, 0x31, 0x41, 0x91,
    0xA1, 0xC1, 0xD1, 0x42, 0xA2, 0xC2, 0x23, 0x33, 0xE3, 0x24, 0x34, 0xB4,
    0xD4, 0xF4, 0x05, 0x15, 0x45, 0xE5, 0x16, 0x36, 0x56, 0xA6, 0xC6, 0xD6,
    0xF6, 0x57, 0xC7, 0xF7, 0x09, 0x29, 0x49, 0x59, 0x69, 0xF9, 0x0A, 0x2A,
    0x3A, 0x4A, 0xDA, 0xEA, 0xFA, 0x0B, 0x2B, 0xAB, 0xEB, 0xFB, 0x0C, 0x1C,
    0x2C, 0x3C, 0x4C, 0x5C, 0xCC, 0xDC, 0xFC, 0x1D, 0x4D, 0x6D, 0xBD, 0xCD,
    0xED, 0xFD, 0x4E, 0x6E, 0xCE, 0xDE, 0x7F, 0xA0, 0xC0, 0x51, 0x61, 0x71,
    0xB1, 0x32, 0x52, 0x62, 0x72, 0x92, 0xB2, 0x53, 0x63, 0x73, 0x93, 0xA3,
    0xB3, 0xC3, 0xD3, 0x14, 0x44, 0x54, 0x64, 0x74, 0x94, 0xA4, 0xC4, 0xE4,
    0x25, 0x35, 0x55, 0x65, 0x75, 0x95, 0xA5, 0xB5, 0xC5, 0xD5, 0xF5, 0x06,
    0x26, 0x46, 0x66, 0x76, 0x96, 0xB6, 0xE6, 0x07, 0x17, 0x27, 0x37, 0x47,
    0x67, 0x77, 0x97, 0xA7, 0xB7, 0xD7, 0xE7, 0x19, 0x39, 0x79, 0x99, 0xA9,
    0xB9, 0xC9, 0xD9, 0xE9, 0x1A, 0x5A, 0x6A, 0x7A, 0x9A, 0xAA, 0xBA, 0xCA,
    0x1B, 0x3B, 0x4B, 0x5B, 0x6B, 0x7B, 0x9B, 0xBB, 0xCB, 0xDB, 0x6C, 0x7C,
    0x9C, 0xAC, 0xBC, 0xEC, 0x5D, 0x7D, 0x9D, 0xAD, 0xDD, 0x5E, 0x7E, 0x9E,
    0xAE, 0xBE, 0x4F, 0x5F, 0x6F, 0x9F, 0xAF, 0xBF, 0xCF
};

static const uint8_t huff_a36_cb[12] = {
    3, 12, 1, 3, 5, 5, 13, 27, 69, 96, 35, 2
};

static const uint8_t huff_a36_xlat[256] = {
    0x00, 0x10, 0x01, 0x11, 0x20, 0x21, 0x02, 0x12, 0x22, 0x31, 0x41, 0x32,
    0x13, 0x23, 0x30, 0x40, 0x51, 0x42, 0x03, 0x33, 0x43, 0x04, 0x14, 0x24,
    0x34, 0x15, 0x25, 0x50, 0x61, 0x71, 0xD1, 0x52, 0x62, 0x72, 0xD2, 0x53,
    0x63, 0xD3, 0x44, 0x54, 0x64, 0x05, 0x35, 0x45, 0x55, 0x16, 0x26, 0x36,
    0x46, 0x17, 0x27, 0x1D, 0x2D, 0x3D, 0x60, 0x70, 0xD0, 0x81, 0x91, 0xA1,
    0xC1, 0xE1, 0xF1, 0x82, 0x92, 0xC2, 0xE2, 0xF2, 0x73, 0x83, 0xE3, 0xF3,
    0x74, 0x84, 0xC4, 0xD4, 0xE4, 0xF4, 0x65, 0x75, 0x85, 0xD5, 0xE5, 0x06,
    0x56, 0x66, 0xD6, 0xE6, 0x07, 0x37, 0x47, 0x57, 0x67, 0xD7, 0xE7, 0x18,
    0x28, 0x38, 0x48, 0x58, 0xD8, 0x19, 0x29, 0x2A, 0x1C, 0x2C, 0x0D, 0x4D,
    0x5D, 0x6D, 0x7D, 0x8D, 0x9D, 0x1E, 0x2E, 0x3E, 0x4E, 0x5E, 0x6E, 0x7E,
    0x1F, 0x2F, 0x3F, 0x80, 0x90, 0xA0, 0xC0, 0xE0, 0xF0, 0xB1, 0xA2, 0xB2,
    0x93, 0xA3, 0xB3, 0xC3, 0x94, 0xA4, 0xB4, 0x95, 0xA5, 0xB5, 0xC5, 0xF5,
    0x76, 0x86, 0x96, 0xA6, 0xC6, 0xF6, 0x77, 0x87, 0x97, 0xA7, 0xC7, 0xF7,
    0x08, 0x68, 0x78, 0x88, 0x98, 0xC8, 0xE8, 0xF8, 0x09, 0x39, 0x49, 0x59,
    0x69, 0x79, 0x89, 0xD9, 0xE9, 0xF9, 0x0A, 0x1A, 0x3A, 0x4A, 0x5A, 0x6A,
    0xDA, 0xEA, 0xFA, 0x1B, 0x2B, 0x3B, 0x4B, 0x5B, 0xDB, 0xEB, 0xFB, 0x0C,
    0x3C, 0x4C, 0x5C, 0x6C, 0x7C, 0x8C, 0x9C, 0xDC, 0xEC, 0xAD, 0xBD, 0xCD,
    0xDD, 0xED, 0x0E, 0x8E, 0x9E, 0xAE, 0xBE, 0x0F, 0x4F, 0x5F, 0x6F, 0x7F,
    0x8F, 0x9F, 0xAF, 0xB0, 0xB6, 0xB7, 0xA8, 0xB8, 0x99, 0xA9, 0xB9, 0xC9,
    0x7A, 0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0x0B, 0x6B, 0x7B, 0x8B, 0x9B, 0xCB,
    0xAC, 0xBC, 0xCC, 0xFC, 0xFD, 0xCE, 0xDE, 0xEE, 0xFE, 0xBF, 0xCF, 0xDF,
    0xEF, 0xFF, 0xAB, 0xBB
};

static const uint8_t huff_a37_cb[7] = { 4, 8, 7, 6, 8, 22, 20 };

static const uint8_t huff_a37_xlat[63] = {
     0,  1,  2,  3, 61, 62, 63,  4,  5,  6, 58, 59, 60,  7,  8,  9,
    10, 54, 55, 56, 57, 11, 12, 13, 14, 15, 16, 25, 26, 27, 28, 29,
    30, 35, 36, 37, 38, 48, 49, 50, 51, 52, 53, 17, 18, 19, 20, 21,
    22, 23, 24, 31, 33, 34, 39, 40, 41, 42, 43, 44, 45, 46, 47
};

static const uint8_t huff_a41_cb[14] = {
    1, 12, 1, 0, 0, 6, 2, 0, 0, 0, 19, 9, 24, 20
};

static const uint8_t huff_a41_xlat[81] = {
    0x00, 0x40, 0xC0, 0x10, 0x30, 0x04, 0x0C, 0x01, 0x03, 0x50, 0xD0, 0x70,
    0xF0, 0xC4, 0x34, 0x4C, 0xCC, 0x1C, 0x41, 0xC1, 0x31, 0x05, 0x0D, 0x43,
    0xC3, 0x13, 0x07, 0x0F, 0x44, 0x14, 0x74, 0xDC, 0x3C, 0x11, 0x1D, 0x33,
    0x37, 0x54, 0xD4, 0xF4, 0x5C, 0x7C, 0xFC, 0xD1, 0x71, 0xF1, 0xC5, 0x15,
    0x35, 0x4D, 0xCD, 0xDD, 0x3D, 0xD3, 0x73, 0x47, 0xC7, 0x17, 0x77, 0x1F,
    0x3F, 0x51, 0x45, 0x55, 0xD5, 0x75, 0xF5, 0x5D, 0x7D, 0xFD, 0x53, 0xF3,
    0x57, 0xD7, 0xF7, 0x4F, 0xCF, 0x5F, 0xDF, 0x7F, 0xFF
};

static const uint8_t huff_a42_cb[10] = { 3, 10, 1, 2, 13, 1, 31, 13, 16, 4 };

static const uint8_t huff_a42_xlat[81] = {
    0x00, 0x40, 0x01, 0x10, 0x50, 0x04, 0x44, 0x14, 0x54, 0x41, 0x11, 0x51,
    0x05, 0x45, 0x15, 0x55, 0x59, 0x80, 0x90, 0x20, 0x60, 0x84, 0x94, 0x24,
    0x64, 0x08, 0x48, 0x18, 0x58, 0x81, 0x91, 0x21, 0x61, 0x85, 0x95, 0x25,
    0x65, 0x09, 0x49, 0x19, 0x02, 0x42, 0x12, 0x52, 0x06, 0x46, 0x16, 0x56,
    0xA0, 0xA4, 0x68, 0xA1, 0xA5, 0x99, 0x29, 0x69, 0x96, 0x66, 0x4A, 0x1A,
    0x5A, 0x88, 0x98, 0x28, 0x89, 0xA9, 0x82, 0x92, 0x22, 0x62, 0x86, 0x26,
    0xA6, 0x0A, 0x9A, 0x2A, 0x6A, 0xA8, 0xA2, 0x8A, 0xAA
};

static const uint8_t huff_a43_cb[5] = { 2, 4, 2, 3, 2 };

static const uint8_t huff_a43_xlat[7] = { 0, 7, 1, 2, 6, 3, 5 };

static const uint8_t huff_a44_cb[9] = { 4, 10, 5, 4, 12, 17, 47, 24, 12 };

static const uint8_t huff_a44_xlat[121] = {
    0x00, 0x10, 0xF0, 0x01, 0x0F, 0x11, 0xF1, 0x1F, 0xFF, 0x20, 0xE0, 0x21,
    0xE1, 0x02, 0x12, 0xF2, 0x0E, 0x1E, 0xFE, 0x2F, 0xEF, 0x30, 0x50, 0xD0,
    0xD1, 0x22, 0xE2, 0x03, 0x13, 0xF3, 0x0D, 0x1D, 0x2D, 0xFD, 0x2E, 0xEE,
    0x3F, 0xDF, 0x40, 0xB0, 0xC0, 0x31, 0x41, 0x51, 0xB1, 0xC1, 0x32, 0xB2,
    0xC2, 0xD2, 0x23, 0xB3, 0xD3, 0xE3, 0x04, 0x14, 0xE4, 0xF4, 0x05, 0x15,
    0xD5, 0xE5, 0xF5, 0x0B, 0x1B, 0x2B, 0x3B, 0xEB, 0xFB, 0x0C, 0x1C, 0x2C,
    0xFC, 0x3D, 0x5D, 0xED, 0x3E, 0x4E, 0x5E, 0xBE, 0xDE, 0x4F, 0x5F, 0xBF,
    0xCF, 0x42, 0x52, 0x33, 0x53, 0xC3, 0x24, 0xB4, 0xD4, 0x25, 0x35, 0xC5,
    0x4B, 0xCB, 0xDB, 0x3C, 0x4C, 0x5C, 0xDC, 0xEC, 0x4D, 0xBD, 0xCD, 0xDD,
    0xCE, 0x43, 0x34, 0x44, 0x54, 0xC4, 0x45, 0x55, 0xB5, 0x5B, 0xBB, 0xBC,
    0xCC
};

static const uint8_t huff_a45_cb[5] = { 2, 4, 2, 2, 4 };

static const uint8_t huff_a45_xlat[8] = { 1, 2, 0, 3, 4, 5, 6, 7 };

static const uint8_t huff_a46_cb[7] = { 5, 9, 1, 16, 31, 36, 172 };

static const uint8_t huff_a46_xlat[256] = {
    0x02, 0x00, 0x30, 0x21, 0x31, 0x41, 0x61, 0x12, 0x22, 0x42, 0x62, 0x43,
    0x53, 0x24, 0x45, 0x26, 0x27, 0x10, 0x40, 0xB0, 0x01, 0x11, 0x81, 0x32,
    0x52, 0x72, 0x92, 0x03, 0x13, 0x33, 0x63, 0x14, 0x34, 0x54, 0x64, 0x74,
    0x05, 0x15, 0x25, 0x35, 0x55, 0x65, 0x06, 0x46, 0x56, 0x57, 0x67, 0x88,
    0x20, 0x51, 0x91, 0xD1, 0xF2, 0x23, 0x83, 0x93, 0x04, 0x44, 0x84, 0x94,
    0x75, 0x85, 0xC5, 0x36, 0x66, 0x96, 0xB6, 0x07, 0x37, 0x97, 0x08, 0x28,
    0x38, 0x48, 0x68, 0x09, 0x69, 0x79, 0x0A, 0x2A, 0x1B, 0x9B, 0x2C, 0x4D,
    0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xC0, 0xD0, 0xE0, 0xF0, 0x71, 0xA1,
    0xB1, 0xC1, 0xE1, 0xF1, 0x82, 0xA2, 0xB2, 0xC2, 0xD2, 0xE2, 0x73, 0xA3,
    0xB3, 0xC3, 0xD3, 0xE3, 0xF3, 0xA4, 0xB4, 0xC4, 0xD4, 0xE4, 0xF4, 0x95,
    0xA5, 0xB5, 0xD5, 0xE5, 0xF5, 0x16, 0x76, 0x86, 0xA6, 0xC6, 0xD6, 0xE6,
    0xF6, 0x17, 0x47, 0x77, 0x87, 0xA7, 0xB7, 0xC7, 0xD7, 0xE7, 0xF7, 0x18,
    0x58, 0x78, 0x98, 0xA8, 0xB8, 0xC8, 0xD8, 0xE8, 0xF8, 0x19, 0x29, 0x39,
    0x49, 0x59, 0x89, 0x99, 0xA9, 0xB9, 0xC9, 0xD9, 0xE9, 0xF9, 0x1A, 0x3A,
    0x4A, 0x5A, 0x6A, 0x7A, 0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0xFA,
    0x0B, 0x2B, 0x3B, 0x4B, 0x5B, 0x6B, 0x7B, 0x8B, 0xAB, 0xBB, 0xCB, 0xDB,
    0xEB, 0xFB, 0x0C, 0x1C, 0x3C, 0x4C, 0x5C, 0x6C, 0x7C, 0x8C, 0x9C, 0xAC,
    0xBC, 0xCC, 0xDC, 0xEC, 0xFC, 0x0D, 0x1D, 0x2D, 0x3D, 0x5D, 0x6D, 0x7D,
    0x8D, 0x9D, 0xAD, 0xBD, 0xCD, 0xDD, 0xED, 0xFD, 0x0E, 0x1E, 0x2E, 0x3E,
    0x4E, 0x5E, 0x6E, 0x7E, 0x8E, 0x9E, 0xAE, 0xBE, 0xCE, 0xDE, 0xEE, 0xFE,
    0x0F, 0x1F, 0x2F, 0x3F, 0x4F, 0x5F, 0x6F, 0x7F, 0x8F, 0x9F, 0xAF, 0xBF,
    0xCF, 0xDF, 0xEF, 0xFF
};

static const uint8_t huff_a47_cb[8] = { 4, 9, 5, 12, 9, 12, 15, 10 };

static const uint8_t huff_a47_xlat[63] = {
     0,  1,  2, 62, 63,  3,  4,  5,  6,  8, 54, 56, 57, 58, 59, 60,
    61,  7,  9, 10, 11, 12, 13, 14, 53, 55, 15, 16, 17, 18, 19, 20,
    21, 36, 37, 39, 42, 52, 22, 25, 28, 35, 38, 40, 41, 43, 45, 46,
    47, 48, 49, 50, 51, 23, 24, 26, 27, 29, 30, 31, 33, 34, 44
};

static const uint8_t huff_a51_cb[12] = {
    2, 11, 1, 0, 6, 2, 6, 18, 4, 26, 6, 12
};

static const uint8_t huff_a51_xlat[81] = {
    0x00, 0x40, 0xC0, 0x30, 0x04, 0x01, 0x03, 0x10, 0x0C, 0xD0, 0x70, 0x34,
    0x1C, 0x0D, 0x07, 0x50, 0xF0, 0x44, 0xC4, 0x14, 0x4C, 0xCC, 0x3C, 0x41,
    0xC1, 0x11, 0x31, 0x05, 0x43, 0xC3, 0x13, 0x33, 0x0F, 0x74, 0xDC, 0x1D,
    0x37, 0x54, 0xD4, 0xF4, 0x5C, 0x7C, 0xFC, 0xD1, 0x71, 0xF1, 0xC5, 0x15,
    0x35, 0x4D, 0xCD, 0xDD, 0x3D, 0x53, 0xD3, 0x73, 0x47, 0xC7, 0x17, 0x77,
    0x4F, 0x1F, 0x3F, 0x51, 0x45, 0x55, 0xF3, 0xCF, 0xFF, 0xD5, 0x75, 0xF5,
    0x5D, 0x7D, 0xFD, 0x57, 0xD7, 0xF7, 0x5F, 0xDF, 0x7F
};

static const uint8_t huff_a52_cb[12] = { 1, 10, 1, 0, 2, 2, 0, 4, 3, 8, 3, 2 };

static const uint8_t huff_a52_xlat[25] = {
    0x00, 0x08, 0x38, 0x01, 0x07, 0x09, 0x39, 0x0F, 0x3F, 0x10, 0x02, 0x06,
    0x30, 0x11, 0x31, 0x0A, 0x3A, 0x0E, 0x17, 0x37, 0x32, 0x16, 0x3E, 0x12,
    0x36
};

static const uint8_t huff_a53_xlat[7] = { 0, 1, 2, 6, 7, 3, 5 };

static const uint8_t huff_a54_cb[8] = { 4, 9, 4, 7, 12, 19, 21, 58 };

static const uint8_t huff_a54_xlat[121] = {
    0x00, 0x01, 0x0F, 0x1F, 0x10, 0xE0, 0xF0, 0x11, 0xF1, 0x2F, 0xFF, 0x20,
    0x21, 0xE1, 0x02, 0x12, 0xF2, 0x03, 0xF3, 0x0E, 0x2E, 0xFE, 0x3F, 0x30,
    0x40, 0xD0, 0xC1, 0xD1, 0x22, 0xC2, 0x33, 0xE3, 0x0C, 0xCC, 0x0D, 0x1D,
    0x2D, 0xFD, 0x1E, 0x3E, 0x5E, 0xEF, 0xC0, 0x52, 0xB2, 0xD2, 0x43, 0xC3,
    0xD3, 0x24, 0x45, 0xF5, 0x4B, 0x5B, 0xFB, 0x1C, 0x3D, 0xBD, 0xDD, 0xEE,
    0xBF, 0xCF, 0xDF, 0x50, 0xB0, 0x31, 0x41, 0x51, 0xB1, 0x32, 0x42, 0xE2,
    0x13, 0x23, 0x53, 0xB3, 0x04, 0x14, 0x34, 0x44, 0x54, 0xB4, 0xC4, 0xD4,
    0xE4, 0xF4, 0x05, 0x15, 0x25, 0x35, 0x55, 0xB5, 0xC5, 0xD5, 0xE5, 0x0B,
    0x1B, 0x2B, 0x3B, 0xBB, 0xCB, 0xDB, 0xEB, 0x2C, 0x3C, 0x4C, 0x5C, 0xBC,
    0xDC, 0xEC, 0xFC, 0x4D, 0x5D, 0xCD, 0xED, 0x4E, 0xBE, 0xCE, 0xDE, 0x4F,
    0x5F
};

static const uint8_t huff_a55_cb[8] = { 1, 6, 1, 1, 1, 0, 3, 2 };

static const uint8_t huff_a55_xlat[8] = { 0, 1, 2, 3, 6, 7, 4, 5 };

static const uint8_t huff_a56_cb[7] = { 3, 7, 1, 8, 6, 8, 8 };

static const uint8_t huff_a56_xlat[31] = {
    4,  0,  1,  2,  3, 28, 29, 30, 31,  5,  6,  7, 24, 25, 27, 8,
    9, 14, 19, 21, 22, 23, 26, 10, 11, 12, 13, 15, 17, 18, 20
};

static const uint8_t huff_a57_cb[9] = { 3, 9, 1, 5, 7, 8, 16, 22, 4 };

static const uint8_t huff_a57_xlat[63] = {
     0,  1,  2, 61, 62, 63,  3,  4,  5,  6, 58, 59,
    60,  7,  8,  9, 10, 54, 55, 56, 57, 11, 12, 13,
    14, 15, 26, 27, 28, 36, 37, 38, 49, 50, 51, 52,
    53, 16, 17, 18, 19, 20, 21, 23, 24, 25, 29, 30,
    31, 33, 34, 35, 39, 43, 44, 45, 46, 47, 48, 22,
    40, 41, 42
};

static const uint8_t huff_a61_cb[12] = {
    2, 11, 1, 0, 8, 0, 1, 16, 10, 29, 12, 4
};

static const uint8_t huff_a61_xlat[81] = {
    0x00, 0x40, 0xC0, 0x10, 0x30, 0x04, 0x0C, 0x01, 0x03, 0x70, 0x50, 0xD0,
    0xF0, 0x44, 0xC4, 0x14, 0x34, 0x4C, 0x1C, 0x3C, 0x31, 0x05, 0x0D, 0x13,
    0x07, 0x0F, 0x74, 0xCC, 0xDC, 0xFC, 0x41, 0xC1, 0x11, 0x43, 0xC3, 0x33,
    0x54, 0xD4, 0xF4, 0x5C, 0x7C, 0x51, 0xD1, 0x71, 0xF1, 0x45, 0xC5, 0x15,
    0x35, 0x4D, 0xCD, 0x1D, 0x3D, 0x53, 0xD3, 0x73, 0xF3, 0x47, 0xC7, 0x17,
    0x37, 0x4F, 0xCF, 0x1F, 0x3F, 0x55, 0xD5, 0x75, 0xF5, 0x5D, 0xDD, 0xFD,
    0x57, 0xD7, 0x77, 0xF7, 0xFF, 0x7D, 0x5F, 0xDF, 0x7F
};

static const uint8_t huff_a62_cb[8] = { 3, 8, 5, 2, 2, 9, 5, 2 };

static const uint8_t huff_a62_xlat[25] = {
    0x00, 0x08, 0x38, 0x01, 0x07, 0x39, 0x0F, 0x09, 0x3F, 0x10, 0x30, 0x31,
    0x02, 0x3A, 0x06, 0x0E, 0x17, 0x37, 0x11, 0x0A, 0x32, 0x16, 0x3E, 0x12,
    0x36
};

static const uint8_t huff_a63_cb[11] = {
    3, 11, 1, 1, 10, 4, 16, 29, 46, 75, 74
};

static const uint8_t huff_a63_xlat[256] = {
    0x00, 0x40, 0x10, 0x50, 0x04, 0x44, 0x14, 0x01, 0x41, 0x05, 0x45, 0x55,
    0x54, 0x11, 0x51, 0x15, 0x80, 0x90, 0x60, 0x24, 0x64, 0xA4, 0x48, 0x61,
    0x95, 0x25, 0xA5, 0x02, 0x42, 0x52, 0x16, 0x56, 0x20, 0x84, 0x94, 0x18,
    0x58, 0x81, 0x91, 0x85, 0x65, 0x09, 0x49, 0x19, 0x59, 0x99, 0x29, 0x69,
    0x79, 0x5D, 0x12, 0x62, 0x06, 0x46, 0x86, 0x66, 0x1A, 0x5A, 0x6A, 0x47,
    0x17, 0xC0, 0xA0, 0xE0, 0xC4, 0xD4, 0x74, 0x08, 0x78, 0x0C, 0x4C, 0x1C,
    0x5C, 0xD1, 0x21, 0xE1, 0x71, 0xC5, 0xE5, 0x75, 0xB5, 0x89, 0xBD, 0x92,
    0x22, 0x96, 0xA6, 0x36, 0x0A, 0x4A, 0x8A, 0x9A, 0x2A, 0x7A, 0xDE, 0x6E,
    0x43, 0x13, 0x53, 0x23, 0x07, 0x77, 0x4B, 0x1B, 0x9B, 0x6B, 0x2F, 0xD0,
    0x30, 0x70, 0xE4, 0x34, 0xF4, 0xC8, 0x98, 0x28, 0x68, 0xA8, 0xE8, 0x38,
    0xB8, 0xF8, 0x9C, 0x2C, 0x6C, 0x7C, 0xA1, 0xB1, 0xD5, 0x35, 0xC9, 0xD9,
    0xA9, 0xE9, 0x39, 0xB9, 0xF9, 0xCD, 0x1D, 0x2D, 0xAD, 0x7D, 0xC2, 0xD2,
    0xA2, 0xB2, 0xF2, 0xC6, 0x26, 0x76, 0xB6, 0xDA, 0xAA, 0xEA, 0x3A, 0xFA,
    0x0E, 0x4E, 0x2E, 0x7E, 0xBE, 0xFE, 0x03, 0x83, 0x63, 0xA3, 0xB3, 0x87,
    0x57, 0x97, 0xD7, 0x27, 0x0B, 0x8B, 0x5B, 0x2B, 0xAB, 0xCF, 0x1F, 0x9F,
    0x7F, 0xBF, 0xB0, 0xF0, 0xB4, 0x88, 0xD8, 0x8C, 0xCC, 0xDC, 0xAC, 0xEC,
    0x3C, 0xBC, 0xFC, 0xC1, 0x31, 0xF1, 0xF5, 0x0D, 0x4D, 0x8D, 0x9D, 0xDD,
    0x6D, 0xED, 0x3D, 0xFD, 0x82, 0xE2, 0x32, 0x72, 0xD6, 0xE6, 0xF6, 0xCA,
    0xBA, 0x8E, 0xCE, 0x1E, 0x5E, 0x9E, 0xAE, 0xEE, 0x3E, 0xC3, 0x93, 0xD3,
    0xE3, 0x33, 0x73, 0xF3, 0xC7, 0x67, 0xA7, 0xE7, 0x37, 0xB7, 0xF7, 0xCB,
    0xDB, 0xEB, 0x3B, 0x7B, 0xBB, 0xFB, 0x0F, 0x4F, 0x8F, 0x5F, 0xDF, 0x6F,
    0xAF, 0xEF, 0x3F, 0xFF
};

static const uint8_t huff_a64_cb[8] = { 4, 9, 1, 7, 12, 36, 63, 2 };

static const uint8_t huff_a64_xlat[121] = {
    0x00, 0x10, 0x20, 0xE0, 0xF0, 0x02, 0x0E, 0xEF, 0x30, 0x01, 0x11, 0x21,
    0x31, 0xF1, 0x12, 0xF2, 0x1E, 0xEE, 0xDF, 0xFF, 0x40, 0xC0, 0xD0, 0xD1,
    0xE1, 0x22, 0x32, 0x42, 0xD2, 0xE2, 0x03, 0x13, 0x23, 0xB3, 0xC3, 0xE3,
    0xF3, 0xE4, 0x05, 0xF5, 0x2B, 0x0C, 0xFC, 0x1D, 0x2D, 0xBD, 0xDD, 0xFD,
    0x2E, 0x4E, 0xDE, 0xFE, 0x0F, 0x1F, 0x2F, 0x3F, 0x50, 0xB0, 0x41, 0x51,
    0xB1, 0xC1, 0x52, 0xB2, 0xC2, 0x33, 0x43, 0x53, 0xD3, 0x04, 0x14, 0x24,
    0x34, 0x44, 0x54, 0xB4, 0xC4, 0xD4, 0xF4, 0x15, 0x25, 0x35, 0x45, 0x55,
    0xB5, 0xC5, 0xD5, 0xE5, 0x0B, 0x1B, 0x3B, 0x4B, 0x5B, 0xBB, 0xCB, 0xDB,
    0xEB, 0xFB, 0x1C, 0x2C, 0x3C, 0x4C, 0x5C, 0xBC, 0xCC, 0xDC, 0xEC, 0x0D,
    0x3D, 0x4D, 0x5D, 0xCD, 0xED, 0x3E, 0x5E, 0xBE, 0xCE, 0x4F, 0xCF, 0x5F,
    0xBF
};

static const uint8_t huff_a65_cb[8] = { 2, 7, 3, 0, 1, 3, 4, 4 };

static const uint8_t huff_a65_xlat[15] = {
    0, 1, 15, 14, 2, 3, 13, 4, 6, 10, 12, 5, 7, 9, 11
};

static const uint8_t huff_a66_cb[11] = { 2, 10, 1, 2, 2, 6, 8, 6, 3, 1, 2 };

static const uint8_t huff_a66_xlat[31] = {
     0,  1, 31, 2, 30,  3,  4, 15, 17, 28, 29,  5,  6,  7,  8, 24,
    25, 26, 27, 9, 10, 11, 21, 22, 23, 12, 19, 20, 13, 14, 18
};

static const uint8_t huff_a67_cb[10] = { 2, 9, 1, 1, 3, 4, 6, 13, 25, 10 };

static const uint8_t huff_a67_xlat[63] = {
     0,  1,  2, 62, 63,  3,  4, 60, 61,  5,  6,  7, 57, 58, 59,  8,
     9, 10, 11, 12, 13, 26, 38, 52, 53, 54, 55, 56, 14, 15, 16, 17,
    18, 19, 25, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 39, 45, 46,
    47, 48, 49, 50, 51, 20, 21, 22, 23, 24, 40, 41, 42, 43, 44
};

static const uint8_t huff_a71_cb[5] = { 1, 3, 1, 1, 2 };

static const uint8_t huff_a72_cb[12] = {
    2, 11, 1, 0, 4, 8, 3, 8, 24, 17, 12, 4
};

static const uint8_t huff_a72_xlat[81] = {
    0x00, 0x40, 0x10, 0x04, 0x01, 0x50, 0x44, 0x14, 0x54, 0x41, 0x11, 0x05,
    0x15, 0x51, 0x45, 0x55, 0x80, 0x90, 0x20, 0x64, 0x08, 0x19, 0x02, 0x06,
    0x60, 0x84, 0x94, 0x24, 0x48, 0x18, 0x58, 0x81, 0x91, 0x21, 0x61, 0x85,
    0x95, 0x25, 0x65, 0x09, 0x49, 0x59, 0x42, 0x12, 0x52, 0x46, 0x16, 0x56,
    0xA0, 0xA4, 0x98, 0x28, 0x68, 0xA1, 0xA5, 0x99, 0x29, 0x69, 0x96, 0x26,
    0x66, 0x0A, 0x4A, 0x1A, 0x5A, 0x88, 0xA8, 0x89, 0xA9, 0x82, 0x92, 0x22,
    0x62, 0x86, 0xA6, 0x2A, 0x6A, 0xA2, 0x8A, 0x9A, 0xAA
};

static const uint8_t huff_a73_cb[11] = { 2, 10, 1, 1, 5, 2, 8, 7, 13, 8, 4 };

static const uint8_t huff_a73_xlat[49] = {
    0x00, 0x08, 0x38, 0x01, 0x39, 0x07, 0x0F, 0x09, 0x3F, 0x10, 0x30, 0x31,
    0x02, 0x3A, 0x06, 0x0E, 0x17, 0x11, 0x0A, 0x32, 0x0D, 0x16, 0x3E, 0x37,
    0x18, 0x28, 0x19, 0x29, 0x12, 0x2A, 0x03, 0x3B, 0x05, 0x15, 0x1E, 0x1F,
    0x2F, 0x1A, 0x0B, 0x2B, 0x33, 0x35, 0x3D, 0x2E, 0x36, 0x13, 0x1B, 0x1D,
    0x2D
};

static const uint8_t huff_a74_cb[14] = {
    1, 12, 1, 0, 0, 4, 0, 4, 5, 9, 30, 45, 21, 2
};

static const uint8_t huff_a74_xlat[121] = {
    0x00, 0x10, 0xF0, 0x01, 0x0F, 0x11, 0xF1, 0x1F, 0xFF, 0x20, 0xE0, 0x02,
    0xF2, 0x0E, 0x21, 0xE1, 0x12, 0xE2, 0x1E, 0x2E, 0xFE, 0x2F, 0xEF, 0x30,
    0x50, 0xB0, 0xC0, 0xD0, 0x31, 0xB1, 0xD1, 0x22, 0xD2, 0x03, 0x13, 0xE3,
    0xF3, 0xF4, 0x05, 0xE5, 0xF5, 0x0B, 0x1B, 0x0C, 0x0D, 0x1D, 0x2D, 0xFD,
    0x3E, 0xEE, 0x3F, 0x5F, 0xDF, 0x40, 0x41, 0x51, 0xC1, 0x32, 0x42, 0x52,
    0xB2, 0xC2, 0x23, 0x33, 0xB3, 0xC3, 0xD3, 0x04, 0x14, 0x24, 0xD4, 0xE4,
    0x15, 0x25, 0xC5, 0xD5, 0x2B, 0x3B, 0xEB, 0xFB, 0x1C, 0x2C, 0x3C, 0x5C,
    0xEC, 0xFC, 0x3D, 0x5D, 0xDD, 0xED, 0x4E, 0x5E, 0xBE, 0xCE, 0xDE, 0x4F,
    0xBF, 0xCF, 0x43, 0x53, 0x34, 0x54, 0xB4, 0xC4, 0x35, 0x45, 0x55, 0xB5,
    0x4B, 0x5B, 0xCB, 0xDB, 0x4C, 0xBC, 0xCC, 0xDC, 0x4D, 0xBD, 0xCD, 0x44,
    0xBB
};

static const uint8_t huff_a75_cb[7] = { 2, 6, 1, 3, 3, 4, 4 };

static const uint8_t huff_a75_xlat[15] = {
    0, 1, 14, 15, 2, 3, 13, 4, 6, 10, 12, 5, 7, 9, 11
};

static const uint8_t huff_a76_cb[12] = {
    3, 12, 1, 3, 4, 8, 10, 36, 60, 78, 48, 8
};

static const uint8_t huff_a76_xlat[256] = {
    0x00, 0x10, 0x01, 0x11, 0x20, 0x21, 0x02, 0x12, 0x30, 0x31, 0x41, 0x22,
    0x03, 0x13, 0x23, 0x14, 0x40, 0x51, 0x61, 0x32, 0x42, 0x33, 0x04, 0x24,
    0x15, 0x16, 0x50, 0x60, 0xD0, 0x71, 0x81, 0xD1, 0xE1, 0xF1, 0x52, 0x62,
    0x72, 0xD2, 0x43, 0x53, 0x63, 0xD3, 0x34, 0x44, 0x54, 0x05, 0x25, 0x35,
    0x45, 0x06, 0x26, 0x36, 0x17, 0x27, 0x18, 0x0D, 0x1D, 0x2D, 0x3D, 0x1E,
    0x2E, 0x1F, 0x70, 0x80, 0xE0, 0xF0, 0x91, 0xA1, 0xC1, 0x82, 0x92, 0xC2,
    0xE2, 0xF2, 0x73, 0x83, 0x93, 0xE3, 0xF3, 0x64, 0x74, 0x84, 0xD4, 0xE4,
    0xF4, 0x55, 0x65, 0xD5, 0xE5, 0xF5, 0x46, 0x56, 0x66, 0xD6, 0x07, 0x37,
    0x47, 0x57, 0x08, 0x28, 0x38, 0x48, 0x19, 0x29, 0x39, 0x1A, 0x2A, 0x1B,
    0x1C, 0x2C, 0x3C, 0x4D, 0x5D, 0x6D, 0x0E, 0x3E, 0x4E, 0x5E, 0x0F, 0x2F,
    0x3F, 0x4F, 0x90, 0xA0, 0xB0, 0xC0, 0xB1, 0xA2, 0xB2, 0xA3, 0xB3, 0xC3,
    0x94, 0xA4, 0xB4, 0xC4, 0x75, 0x85, 0x95, 0xA5, 0xC5, 0x76, 0x86, 0x96,
    0xE6, 0xF6, 0x67, 0x77, 0x87, 0xD7, 0xE7, 0xF7, 0x58, 0x68, 0x78, 0x88,
    0xD8, 0xE8, 0xF8, 0x09, 0x49, 0x59, 0x69, 0xD9, 0xE9, 0xF9, 0x0A, 0x3A,
    0x4A, 0x5A, 0xDA, 0xEA, 0x0B, 0x2B, 0x3B, 0x4B, 0xDB, 0x0C, 0x4C, 0x5C,
    0x6C, 0xDC, 0x7D, 0x8D, 0x9D, 0xAD, 0xBD, 0xCD, 0x6E, 0x7E, 0x8E, 0x9E,
    0xAE, 0xBE, 0x5F, 0x6F, 0x7F, 0x8F, 0x9F, 0xAF, 0xB5, 0xA6, 0xB6, 0xC6,
    0x97, 0xA7, 0xC7, 0x98, 0xA8, 0xB8, 0xC8, 0x79, 0x89, 0x99, 0xA9, 0xB9,
    0xC9, 0x6A, 0x7A, 0x8A, 0x9A, 0xAA, 0xCA, 0xFA, 0x5B, 0x6B, 0x7B, 0x8B,
    0xCB, 0xEB, 0xFB, 0x7C, 0x8C, 0x9C, 0xAC, 0xBC, 0xEC, 0xFC, 0xDD, 0xED,
    0xFD, 0xCE, 0xDE, 0xEE, 0xFE, 0xBF, 0xCF, 0xDF, 0xB7, 0xBA, 0x9B, 0xAB,
    0xBB, 0xCC, 0xEF, 0xFF
};

static const uint8_t huff_b01_cb[14] = {
    1, 12, 1, 0, 0, 2, 6, 0, 11, 13, 12, 24, 4, 8
};

static const uint8_t huff_b01_xlat[81] = {
    0x00, 0x01, 0x03, 0x40, 0xC0, 0x10, 0x30, 0x04, 0x0C, 0x50, 0xD0, 0x70,
    0xF0, 0x34, 0x1C, 0x05, 0x0D, 0x13, 0x07, 0x0F, 0x44, 0xC4, 0x14, 0x4C,
    0xCC, 0x3C, 0x41, 0xC1, 0x11, 0x31, 0x43, 0xC3, 0x33, 0x54, 0x74, 0xDC,
    0xFC, 0x71, 0x15, 0x4D, 0x1D, 0xD3, 0xC7, 0x37, 0x3F, 0xD4, 0xF4, 0x5C,
    0x7C, 0x51, 0xD1, 0xF1, 0x45, 0xC5, 0x55, 0x35, 0xCD, 0xDD, 0x3D, 0x53,
    0x73, 0xF3, 0x47, 0x17, 0x77, 0x4F, 0xCF, 0x1F, 0xFF, 0xF5, 0x7D, 0xD7,
    0x5F, 0xD5, 0x75, 0x5D, 0xFD, 0x57, 0xF7, 0xDF, 0x7F
};

static const uint8_t huff_b02_cb[14] = {
    1, 12, 1, 0, 0, 4, 0, 8, 4, 9, 19, 13, 13, 10
};

static const uint8_t huff_b02_xlat[81] = {
    0x00, 0x40, 0x10, 0x04, 0x01, 0x50, 0x44, 0x14, 0x54, 0x41, 0x11, 0x05,
    0x15, 0x80, 0x51, 0x45, 0x55, 0x90, 0x20, 0x60, 0x24, 0x08, 0x18, 0x09,
    0x02, 0x06, 0x84, 0x94, 0x64, 0x48, 0x58, 0x81, 0x91, 0x21, 0x61, 0x95,
    0x25, 0x65, 0x19, 0x59, 0x42, 0x12, 0x46, 0x16, 0x56, 0xA0, 0xA4, 0x28,
    0x68, 0x85, 0xA5, 0x49, 0x29, 0x69, 0x52, 0x0A, 0x1A, 0x5A, 0x88, 0x98,
    0xA1, 0x89, 0x99, 0xA9, 0x22, 0x62, 0x96, 0x26, 0x66, 0x4A, 0x6A, 0xA8,
    0x82, 0x92, 0xA2, 0x86, 0xA6, 0x8A, 0x9A, 0x2A, 0xAA
};

static const uint8_t huff_b03_cb[11] = { 1, 9, 1, 0, 0, 4, 0, 5, 12, 13, 14 };

static const uint8_t huff_b03_xlat[49] = {
    0x00, 0x08, 0x38, 0x01, 0x07, 0x30, 0x09, 0x39, 0x0F, 0x3F, 0x10, 0x18,
    0x28, 0x31, 0x02, 0x3A, 0x03, 0x05, 0x06, 0x0E, 0x17, 0x37, 0x11, 0x19,
    0x29, 0x0A, 0x32, 0x0B, 0x3B, 0x0D, 0x15, 0x3D, 0x3E, 0x1F, 0x2F, 0x12,
    0x1A, 0x2A, 0x13, 0x1B, 0x2B, 0x33, 0x1D, 0x2D, 0x35, 0x16, 0x1E, 0x2E,
    0x36
};

static const uint8_t huff_b04_cb[12] = {
    2, 11, 1, 0, 4, 4, 5, 9, 30, 45, 21, 2
};

static const uint8_t huff_b04_xlat[121] = {
    0x00, 0x10, 0xF0, 0x01, 0x0F, 0x11, 0xF1, 0x1F, 0xFF, 0x20, 0xE0, 0x02,
    0xF2, 0x0E, 0x21, 0xE1, 0x12, 0xE2, 0x1E, 0x2E, 0xFE, 0x2F, 0xEF, 0x30,
    0x50, 0xB0, 0xC0, 0xD0, 0x31, 0xB1, 0xD1, 0x22, 0xD2, 0x03, 0x13, 0xE3,
    0xF3, 0xF4, 0x05, 0xE5, 0xF5, 0x0B, 0x1B, 0x0C, 0x0D, 0x1D, 0x2D, 0xFD,
    0x3E, 0xEE, 0x3F, 0x5F, 0xDF, 0x40, 0x41, 0x51, 0xC1, 0x32, 0x42, 0x52,
    0xB2, 0xC2, 0x23, 0x33, 0xB3, 0xC3, 0xD3, 0x04, 0x14, 0x24, 0xD4, 0xE4,
    0x15, 0x25, 0xC5, 0xD5, 0x2B, 0x3B, 0xEB, 0xFB, 0x1C, 0x2C, 0x3C, 0x5C,
    0xEC, 0xFC, 0x3D, 0x5D, 0xDD, 0xED, 0x4E, 0x5E, 0xBE, 0xCE, 0xDE, 0x4F,
    0xBF, 0xCF, 0x43, 0x53, 0x34, 0x54, 0xB4, 0xC4, 0x35, 0x45, 0x55, 0xB5,
    0x4B, 0x5B, 0xCB, 0xDB, 0x4C, 0xBC, 0xCC, 0xDC, 0x4D, 0xBD, 0xCD, 0x44,
    0xBB
};

static const uint8_t huff_b05_cb[11] = {
    3, 11, 1, 4, 4, 4, 12, 30, 73, 75, 22
};

static const uint8_t huff_b05_xlat[225] = {
    0x00, 0x10, 0xF0, 0x01, 0x0F, 0x11, 0xF1, 0x1F, 0xFF, 0x20, 0xE0, 0x02,
    0x0E, 0x30, 0xD0, 0x21, 0xE1, 0x12, 0xF2, 0x03, 0x0D, 0x1E, 0xFE, 0x2F,
    0xEF, 0x40, 0x60, 0x70, 0x90, 0xA0, 0xC0, 0x31, 0xD1, 0x22, 0x32, 0xD2,
    0xE2, 0x13, 0x23, 0xE3, 0xF3, 0x04, 0x06, 0x07, 0x09, 0x0A, 0x0C, 0x1D,
    0x2D, 0xFD, 0x2E, 0x3E, 0xEE, 0x3F, 0xDF, 0x50, 0xB0, 0x41, 0x51, 0x61,
    0x71, 0x91, 0xA1, 0xB1, 0xC1, 0x42, 0x62, 0x72, 0x92, 0xA2, 0xC2, 0x33,
    0x93, 0xA3, 0xD3, 0x14, 0x24, 0xE4, 0xF4, 0x05, 0x15, 0xF5, 0x16, 0x26,
    0xD6, 0xE6, 0xF6, 0x17, 0x27, 0xD7, 0xE7, 0xF7, 0x19, 0x29, 0x39, 0xE9,
    0xF9, 0x1A, 0x2A, 0xEA, 0xFA, 0x0B, 0x1B, 0xFB, 0x1C, 0x2C, 0xEC, 0xFC,
    0x3D, 0x7D, 0x9D, 0xDD, 0xED, 0x4E, 0x6E, 0x7E, 0x9E, 0xAE, 0xCE, 0xDE,
    0x4F, 0x5F, 0x6F, 0x7F, 0x9F, 0xAF, 0xBF, 0xCF, 0x52, 0xB2, 0x43, 0x53,
    0x63, 0x73, 0xB3, 0xC3, 0x34, 0x44, 0x64, 0x74, 0x94, 0xA4, 0xB4, 0xC4,
    0xD4, 0x25, 0x35, 0x65, 0x75, 0x95, 0xA5, 0xD5, 0xE5, 0x36, 0x46, 0x56,
    0x66, 0xA6, 0xB6, 0xC6, 0x37, 0x47, 0x57, 0xB7, 0xC7, 0x49, 0x59, 0x69,
    0xB9, 0xC9, 0xD9, 0x3A, 0x4A, 0x5A, 0x6A, 0xAA, 0xBA, 0xCA, 0xDA, 0x2B,
    0x3B, 0x6B, 0x7B, 0x9B, 0xAB, 0xDB, 0xEB, 0x3C, 0x4C, 0x6C, 0x7C, 0x9C,
    0xAC, 0xCC, 0xDC, 0x4D, 0x5D, 0x6D, 0xAD, 0xBD, 0xCD, 0x5E, 0xBE, 0x54,
    0x45, 0x55, 0xB5, 0xC5, 0x76, 0x96, 0x67, 0x77, 0x97, 0xA7, 0x79, 0x99,
    0xA9, 0x7A, 0x9A, 0x4B, 0x5B, 0xBB, 0xCB, 0x5C, 0xBC
};

static const uint8_t huff_b07_cb[9] = { 3, 9, 3, 2, 4, 8, 23, 13, 10 };

static const uint8_t huff_b07_xlat[63] = {
     0,  1, 63,  2, 62,  3,  4, 60, 61,  5,  6,  7,  8, 56, 57, 58,
    59,  9, 10, 11, 12, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36,
    37, 38, 39, 51, 52, 53, 54, 55, 13, 14, 15, 16, 17, 18, 19, 45,
    46, 47, 48, 49, 50, 20, 21, 22, 23, 24, 40, 41, 42, 43, 44
};

static const uint8_t huff_b12_cb[10] = { 3, 10, 1, 3, 12, 0, 30, 9, 18, 8 };

static const uint8_t huff_b12_xlat[81] = {
    0x00, 0x40, 0x04, 0x01, 0x10, 0x50, 0x44, 0x14, 0x54, 0x41, 0x11, 0x51,
    0x05, 0x45, 0x15, 0x55, 0x80, 0x90, 0x20, 0x60, 0x84, 0x94, 0x24, 0x64,
    0x08, 0x48, 0x18, 0x81, 0x91, 0x61, 0x85, 0x95, 0x25, 0x65, 0x09, 0x49,
    0x19, 0x59, 0x02, 0x42, 0x12, 0x52, 0x06, 0x46, 0x16, 0x56, 0xA4, 0x58,
    0x68, 0x21, 0xA5, 0x29, 0x69, 0x1A, 0x5A, 0xA0, 0x88, 0x98, 0x28, 0xA1,
    0x89, 0x99, 0xA9, 0x92, 0x22, 0x62, 0x86, 0x96, 0x26, 0x66, 0x0A, 0x4A,
    0x6A, 0xA8, 0x82, 0xA2, 0xA6, 0x8A, 0x9A, 0x2A, 0xAA
};

static const uint8_t huff_b14_cb[14] = {
    1, 12, 1, 0, 0, 4, 0, 3, 5, 16, 28, 34, 26, 4
};

static const uint8_t huff_b14_xlat[121] = {
    0x00, 0x10, 0xF0, 0x01, 0x0F, 0xF1, 0x1F, 0xFF, 0x20, 0xE0, 0x11, 0x02,
    0x0E, 0x30, 0x50, 0xB0, 0xD0, 0x21, 0xE1, 0x12, 0xF2, 0x03, 0x05, 0x0B,
    0x0D, 0x1E, 0xFE, 0x2F, 0xEF, 0x40, 0xC0, 0x31, 0x51, 0xB1, 0xC1, 0xD1,
    0x22, 0x52, 0xE2, 0x13, 0xF3, 0x04, 0x15, 0xF5, 0x1B, 0xEB, 0xFB, 0x0C,
    0x1D, 0xFD, 0x2E, 0x5E, 0xEE, 0x3F, 0x5F, 0xBF, 0xDF, 0x41, 0x32, 0x42,
    0xB2, 0xD2, 0x23, 0x53, 0xB3, 0xE3, 0x14, 0x24, 0xE4, 0xF4, 0x25, 0x35,
    0xD5, 0xE5, 0x2B, 0x3B, 0xDB, 0x1C, 0x2C, 0xBC, 0xEC, 0xFC, 0x2D, 0xBD,
    0xED, 0x3E, 0x4E, 0xBE, 0xDE, 0x4F, 0xCF, 0xC2, 0x33, 0x43, 0xC3, 0xD3,
    0x34, 0x44, 0x54, 0xB4, 0xD4, 0x45, 0x55, 0xC5, 0x4B, 0xCB, 0x3C, 0x4C,
    0x5C, 0xCC, 0xDC, 0x3D, 0x4D, 0x5D, 0xCD, 0xDD, 0xCE, 0xC4, 0xB5, 0x5B,
    0xBB
};

static const uint8_t huff_b16_cb[11] = {
    4, 12, 4, 4, 9, 13, 37, 76, 72, 39, 2
};

static const uint8_t huff_b16_xlat[256] = {
    0x00, 0x10, 0x01, 0x11, 0x20, 0x21, 0x02, 0x12, 0x30, 0x31, 0x41, 0x22,
    0x32, 0x03, 0x13, 0x23, 0x14, 0x40, 0x51, 0x61, 0x42, 0x52, 0x33, 0x43,
    0x04, 0x24, 0x34, 0x15, 0x25, 0x16, 0x50, 0x60, 0x70, 0x71, 0x81, 0xD1,
    0xE1, 0x62, 0x72, 0x82, 0xD2, 0x53, 0x63, 0x73, 0xD3, 0x44, 0x54, 0x05,
    0x35, 0x45, 0x55, 0x06, 0x26, 0x36, 0x07, 0x17, 0x27, 0x37, 0x18, 0x28,
    0x19, 0x1D, 0x2D, 0x3D, 0x1E, 0x2E, 0x1F, 0x80, 0x90, 0xD0, 0xE0, 0xF0,
    0x91, 0xA1, 0xB1, 0xC1, 0xF1, 0x92, 0xA2, 0xB2, 0xC2, 0xE2, 0xF2, 0x83,
    0x93, 0xA3, 0xC3, 0xE3, 0xF3, 0x64, 0x74, 0x84, 0x94, 0xD4, 0xE4, 0xF4,
    0x65, 0x75, 0x85, 0xD5, 0xE5, 0x46, 0x56, 0x66, 0x76, 0xD6, 0xE6, 0x47,
    0x57, 0x67, 0xD7, 0x08, 0x38, 0x48, 0x58, 0x09, 0x29, 0x39, 0x49, 0x0A,
    0x1A, 0x2A, 0x3A, 0x1B, 0x2B, 0x0C, 0x1C, 0x2C, 0x3C, 0x0D, 0x4D, 0x5D,
    0x6D, 0x7D, 0x0E, 0x3E, 0x4E, 0x5E, 0x6E, 0x0F, 0x2F, 0x3F, 0x4F, 0xA0,
    0xB0, 0xC0, 0xB3, 0xA4, 0xB4, 0xC4, 0x95, 0xA5, 0xB5, 0xC5, 0xF5, 0x86,
    0x96, 0xA6, 0xB6, 0xC6, 0xF6, 0x77, 0x87, 0x97, 0xA7, 0xC7, 0xE7, 0xF7,
    0x68, 0x78, 0x88, 0x98, 0xD8, 0xE8, 0xF8, 0x59, 0x69, 0x79, 0x89, 0xD9,
    0xE9, 0xF9, 0x4A, 0x5A, 0x6A, 0x7A, 0xDA, 0xEA, 0x0B, 0x3B, 0x4B, 0x5B,
    0xDB, 0xEB, 0x4C, 0x5C, 0x6C, 0x7C, 0x8C, 0xDC, 0x8D, 0x9D, 0xAD, 0xBD,
    0xCD, 0x7E, 0x8E, 0x9E, 0xAE, 0xBE, 0x5F, 0x6F, 0x7F, 0x8F, 0x9F, 0xB7,
    0xA8, 0xB8, 0xC8, 0x99, 0xA9, 0xB9, 0xC9, 0x8A, 0x9A, 0xAA, 0xBA, 0xCA,
    0xFA, 0x6B, 0x7B, 0x8B, 0x9B, 0xAB, 0xBB, 0xCB, 0xFB, 0x9C, 0xAC, 0xBC,
    0xCC, 0xEC, 0xFC, 0xDD, 0xED, 0xFD, 0xCE, 0xDE, 0xEE, 0xFE, 0xAF, 0xBF,
    0xCF, 0xDF, 0xEF, 0xFF
};

static const uint8_t huff_b26_cb[12] = {
    3, 12, 2, 2, 4, 5, 11, 26, 67, 78, 51, 10
};

static const uint8_t huff_b26_xlat[256] = {
    0x00, 0x01, 0x10, 0x11, 0x20, 0x21, 0x02, 0x12, 0x30, 0x31, 0x22, 0x03,
    0x13, 0x40, 0x41, 0x51, 0x32, 0x42, 0x23, 0x33, 0x04, 0x14, 0x24, 0x15,
    0x50, 0x61, 0x71, 0xD1, 0xE1, 0x52, 0x62, 0xD2, 0x43, 0x53, 0xD3, 0x34,
    0x44, 0x05, 0x25, 0x35, 0x06, 0x16, 0x26, 0x17, 0x18, 0x1D, 0x2D, 0x3D,
    0x1E, 0x2E, 0x60, 0x70, 0x80, 0xD0, 0xE0, 0xF0, 0x81, 0x91, 0xA1, 0xC1,
    0xF1, 0x72, 0x82, 0x92, 0xC2, 0xE2, 0xF2, 0x63, 0x73, 0xE3, 0xF3, 0x54,
    0x64, 0x74, 0xD4, 0xE4, 0xF4, 0x45, 0x55, 0x65, 0xD5, 0xE5, 0xF5, 0x36,
    0x46, 0x56, 0xD6, 0xE6, 0x07, 0x27, 0x37, 0x47, 0xD7, 0x08, 0x28, 0x38,
    0x19, 0x29, 0x1A, 0x1B, 0x1C, 0x2C, 0x0D, 0x4D, 0x5D, 0x6D, 0x7D, 0x0E,
    0x3E, 0x4E, 0x5E, 0x6E, 0x0F, 0x1F, 0x2F, 0x3F, 0x4F, 0x90, 0xA0, 0xC0,
    0xB1, 0xA2, 0xB2, 0x83, 0x93, 0xA3, 0xB3, 0xC3, 0x84, 0x94, 0xA4, 0xC4,
    0x75, 0x85, 0x95, 0xC5, 0x66, 0x76, 0x86, 0x96, 0xC6, 0xF6, 0x57, 0x67,
    0x77, 0xE7, 0xF7, 0x48, 0x58, 0x68, 0x78, 0xD8, 0xE8, 0xF8, 0x09, 0x39,
    0x49, 0x59, 0xD9, 0xE9, 0xF9, 0x0A, 0x2A, 0x3A, 0x4A, 0xDA, 0xEA, 0x0B,
    0x2B, 0x3B, 0xDB, 0xEB, 0x0C, 0x3C, 0x4C, 0x5C, 0x6C, 0xDC, 0x8D, 0x9D,
    0xAD, 0xBD, 0xCD, 0x7E, 0x8E, 0x9E, 0xAE, 0xBE, 0xCE, 0x5F, 0x6F, 0x7F,
    0x8F, 0x9F, 0xAF, 0xB0, 0xB4, 0xA5, 0xB5, 0xA6, 0xB6, 0x87, 0x97, 0xA7,
    0xB7, 0xC7, 0x88, 0x98, 0xA8, 0xC8, 0x69, 0x79, 0x89, 0x99, 0xA9, 0xC9,
    0x5A, 0x6A, 0x7A, 0x9A, 0xCA, 0xFA, 0x4B, 0x5B, 0x6B, 0x7B, 0xCB, 0xFB,
    0x7C, 0x8C, 0x9C, 0xAC, 0xBC, 0xCC, 0xEC, 0xFC, 0xDD, 0xED, 0xFD, 0xDE,
    0xEE, 0xFE, 0xBF, 0xCF, 0xDF, 0xEF, 0xB8, 0xB9, 0x8A, 0xAA, 0xBA, 0x8B,
    0x9B, 0xAB, 0xBB, 0xFF
};

static const uint8_t huff_b32_cb[12] = {
    2, 11, 1, 0, 4, 6, 7, 10, 22, 11, 16, 4
};

static const uint8_t huff_b32_xlat[81] = {
    0x00, 0x40, 0x10, 0x04, 0x01, 0x50, 0x44, 0x14, 0x41, 0x11, 0x05, 0x80,
    0x54, 0x51, 0x45, 0x15, 0x55, 0x02, 0x90, 0x20, 0x60, 0x84, 0x24, 0x08,
    0x18, 0x09, 0x12, 0x06, 0xA0, 0x94, 0x64, 0x48, 0x58, 0x81, 0x91, 0x21,
    0x61, 0x85, 0x95, 0x25, 0x65, 0x49, 0x19, 0x59, 0x42, 0x52, 0x46, 0x16,
    0x56, 0x0A, 0xA4, 0x28, 0x68, 0xA1, 0xA5, 0x29, 0x69, 0x26, 0x4A, 0x1A,
    0x5A, 0x88, 0x98, 0xA8, 0x89, 0x99, 0xA9, 0x82, 0x92, 0x22, 0x62, 0x86,
    0x96, 0x66, 0x9A, 0x2A, 0x6A, 0xA2, 0xA6, 0x8A, 0xAA
};

static const uint8_t huff_b33_cb[13] = {
    2, 12, 1, 0, 0, 4, 11, 8, 28, 92, 97, 13, 2
};

static const uint8_t huff_b33_xlat[256] = {
    0x00, 0x40, 0x10, 0x04, 0x01, 0x50, 0x44, 0x14, 0x54, 0x41, 0x11, 0x51,
    0x05, 0x45, 0x15, 0x55, 0x20, 0x95, 0x65, 0x49, 0x59, 0x52, 0x46, 0x16,
    0x80, 0x90, 0x60, 0x84, 0x94, 0x24, 0x64, 0xA4, 0x08, 0x48, 0x18, 0x58,
    0x81, 0x91, 0x21, 0x61, 0x85, 0x25, 0x09, 0x19, 0x69, 0x02, 0x42, 0x12,
    0x06, 0x56, 0x5A, 0x57, 0xD0, 0x74, 0x68, 0x5C, 0xC1, 0xD5, 0xA5, 0xE5,
    0x75, 0xB5, 0xF5, 0x99, 0xD9, 0xA9, 0xE9, 0x79, 0xB9, 0xF9, 0x1D, 0x5D,
    0x9D, 0xDD, 0x6D, 0xAD, 0xED, 0x7D, 0xBD, 0xFD, 0x82, 0x92, 0xD2, 0x62,
    0x96, 0xD6, 0x26, 0x66, 0xA6, 0xE6, 0x76, 0xB6, 0xF6, 0x0A, 0x4A, 0x1A,
    0x9A, 0xDA, 0x2A, 0x6A, 0xAA, 0xEA, 0x7A, 0xBA, 0xFA, 0x5E, 0x9E, 0xDE,
    0x6E, 0xAE, 0xEE, 0x7E, 0xBE, 0xFE, 0x03, 0x13, 0x53, 0x17, 0x97, 0xD7,
    0x67, 0xA7, 0xE7, 0x77, 0xB7, 0xF7, 0x5B, 0x9B, 0xDB, 0x6B, 0xAB, 0xEB,
    0x7B, 0xBB, 0xFB, 0x5F, 0x9F, 0xDF, 0x6F, 0xAF, 0xEF, 0x7F, 0xBF, 0xFF,
    0xC0, 0xA0, 0xE0, 0x30, 0xC4, 0xD4, 0xE4, 0x34, 0xB4, 0xF4, 0x88, 0xC8,
    0x98, 0xD8, 0x28, 0xA8, 0xE8, 0x38, 0x78, 0xB8, 0xF8, 0x0C, 0x4C, 0x1C,
    0x9C, 0xDC, 0x6C, 0xAC, 0xEC, 0x7C, 0xBC, 0xFC, 0xD1, 0xA1, 0xE1, 0x31,
    0x71, 0xB1, 0xF1, 0xC5, 0x35, 0x89, 0xC9, 0x29, 0x39, 0x0D, 0x4D, 0x8D,
    0xCD, 0x2D, 0x3D, 0x22, 0xA2, 0xE2, 0x72, 0xB2, 0xF2, 0x86, 0xC6, 0x36,
    0x8A, 0xCA, 0x3A, 0x0E, 0x4E, 0x8E, 0xCE, 0x1E, 0x2E, 0x3E, 0x43, 0x83,
    0x93, 0xD3, 0x23, 0x63, 0xA3, 0xE3, 0x73, 0xB3, 0xF3, 0x07, 0x47, 0x87,
    0xC7, 0x27, 0x37, 0x4B, 0x8B, 0xCB, 0x1B, 0x2B, 0x3B, 0x4F, 0x8F, 0xCF,
    0x1F, 0x70, 0xB0, 0xF0, 0x8C, 0xCC, 0x2C, 0x3C, 0xC2, 0x32, 0xC3, 0x0F,
    0x2F, 0x3F, 0x33, 0x0B
};

static const uint8_t huff_b35_cb[14] = {
    1, 12, 1, 0, 0, 0, 4, 6, 6, 14, 42, 63, 59, 30
};

static const uint8_t huff_b35_xlat[225] = {
    0x00, 0x10, 0xF0, 0x01, 0x0F, 0x11, 0xF1, 0x02, 0x0E, 0x1F, 0xFF, 0x20,
    0xE0, 0x21, 0xF2, 0xFE, 0xEF, 0x30, 0xD0, 0xE1, 0x12, 0x22, 0xE2, 0x03,
    0x0D, 0x1D, 0x1E, 0x2E, 0xEE, 0x2F, 0xDF, 0x40, 0x60, 0x70, 0x90, 0xA0,
    0xB0, 0xC0, 0x31, 0x71, 0x91, 0xC1, 0xD1, 0x32, 0xD2, 0x13, 0xE3, 0xF3,
    0x04, 0x05, 0x06, 0x07, 0x17, 0xF7, 0x09, 0x19, 0x0A, 0x1A, 0xFA, 0x0C,
    0x1C, 0x2D, 0xED, 0xFD, 0x3E, 0x7E, 0xDE, 0x3F, 0x6F, 0x7F, 0x9F, 0xAF,
    0xCF, 0x50, 0x41, 0x51, 0x61, 0xA1, 0xB1, 0x62, 0x72, 0x92, 0xA2, 0xC2,
    0x23, 0x33, 0x63, 0x73, 0x93, 0xA3, 0xD3, 0x14, 0x24, 0x34, 0xD4, 0xE4,
    0xF4, 0x15, 0xF5, 0x16, 0x26, 0xD6, 0xE6, 0xF6, 0x27, 0x37, 0x47, 0xE7,
    0x29, 0x39, 0xC9, 0xD9, 0xE9, 0xF9, 0x2A, 0xEA, 0x0B, 0x1B, 0xFB, 0x2C,
    0x7C, 0xEC, 0xFC, 0x3D, 0x4D, 0x6D, 0x7D, 0xDD, 0x4E, 0x5E, 0x6E, 0x9E,
    0xAE, 0xCE, 0x4F, 0x5F, 0x42, 0x52, 0xB2, 0x43, 0xB3, 0xC3, 0x44, 0x64,
    0x74, 0x94, 0xA4, 0x25, 0x35, 0x65, 0x75, 0x95, 0xA5, 0xE5, 0x36, 0x46,
    0x66, 0x76, 0x96, 0xA6, 0xB6, 0xC6, 0x57, 0xA7, 0xB7, 0xC7, 0xD7, 0x59,
    0xA9, 0xB9, 0x3A, 0x4A, 0x6A, 0xCA, 0xDA, 0x2B, 0x3B, 0x6B, 0x9B, 0xAB,
    0xDB, 0xEB, 0x3C, 0x6C, 0x9C, 0xAC, 0xCC, 0xDC, 0x5D, 0x9D, 0xAD, 0xBD,
    0xCD, 0xBE, 0xBF, 0x53, 0x54, 0xB4, 0xC4, 0x45, 0x55, 0xB5, 0xC5, 0xD5,
    0x56, 0x67, 0x77, 0x97, 0x49, 0x69, 0x79, 0x99, 0x5A, 0x7A, 0x9A, 0xAA,
    0xBA, 0x4B, 0x5B, 0x7B, 0xBB, 0xCB, 0x4C, 0x5C, 0xBC
};

static const uint8_t huff_b37_cb[13] = {
    1, 11, 1, 0, 2, 0, 2, 2, 6, 17, 14, 13, 6
};

static const uint8_t huff_b37_xlat[63] = {
     0,  1, 63,  2, 62,  3, 61,  4,  5,  6, 58, 59, 60,  7,  8,  9,
    10, 25, 26, 27, 28, 29, 34, 35, 37, 38, 39, 55, 56, 57, 11, 13,
    14, 15, 30, 31, 33, 36, 49, 50, 51, 52, 53, 54, 12, 16, 17, 18,
    19, 21, 41, 43, 44, 45, 46, 47, 48, 20, 22, 23, 24, 40, 42
};

static const uint8_t huff_b41_cb[14] = {
    1, 12, 1, 0, 0, 1, 7, 0, 20, 4, 10, 24, 2, 12
};

static const uint8_t huff_b41_xlat[81] = {
    0x00, 0x01, 0x40, 0xC0, 0x10, 0x30, 0x04, 0x0C, 0x03, 0x50, 0xD0, 0x70,
    0xF0, 0xC4, 0x14, 0x34, 0x4C, 0x1C, 0x3C, 0xC1, 0x11, 0x31, 0x05, 0x0D,
    0xC3, 0x13, 0x33, 0x07, 0x0F, 0x44, 0xCC, 0x41, 0x43, 0x54, 0x74, 0xDC,
    0xFC, 0x71, 0x15, 0x4D, 0x1D, 0x37, 0x3F, 0xD4, 0xF4, 0x5C, 0x7C, 0x51,
    0xD1, 0xF1, 0x45, 0xC5, 0x35, 0xCD, 0xDD, 0x3D, 0x53, 0xD3, 0x73, 0xF3,
    0x47, 0xC7, 0x17, 0x77, 0x4F, 0xCF, 0x1F, 0x55, 0xFF, 0xD5, 0x75, 0xF5,
    0x5D, 0x7D, 0xFD, 0x57, 0xD7, 0xF7, 0x5F, 0xDF, 0x7F
};

static const uint8_t huff_b42_cb[11] = { 1, 9, 1, 0, 1, 3, 2, 3, 7, 4, 4 };

static const uint8_t huff_b42_xlat[25] = {
    0x00, 0x07, 0x08, 0x38, 0x01, 0x39, 0x0F, 0x10, 0x09, 0x3F, 0x30, 0x31,
    0x02, 0x3A, 0x06, 0x0E, 0x17, 0x11, 0x0A, 0x3E, 0x37, 0x12, 0x32, 0x16,
    0x36
};

static const uint8_t huff_b43_cb[10] = { 2, 9, 1, 1, 3, 4, 9, 15, 12, 4 };

static const uint8_t huff_b43_xlat[49] = {
    0x00, 0x07, 0x08, 0x38, 0x01, 0x09, 0x39, 0x0F, 0x3F, 0x10, 0x30, 0x31,
    0x02, 0x3A, 0x06, 0x0E, 0x17, 0x37, 0x18, 0x28, 0x11, 0x19, 0x29, 0x0A,
    0x03, 0x0B, 0x3B, 0x05, 0x0D, 0x3D, 0x3E, 0x1F, 0x2F, 0x12, 0x1A, 0x2A,
    0x32, 0x13, 0x33, 0x15, 0x35, 0x16, 0x1E, 0x2E, 0x36, 0x1B, 0x2B, 0x1D,
    0x2D
};

static const uint8_t huff_b47_cb[10] = { 2, 9, 1, 1, 3, 4, 6, 14, 22, 12 };

static const uint8_t huff_b47_xlat[63] = {
     0,  1,  2, 62, 63,  3,  4, 60, 61,  5,  6,  7, 57, 58, 59,  8,
     9, 10, 11, 12, 26, 27, 37, 38, 52, 53, 54, 55, 56, 13, 14, 15,
    16, 17, 18, 25, 28, 29, 30, 31, 33, 34, 35, 36, 39, 46, 47, 48,
    49, 50, 51, 19, 20, 21, 22, 23, 24, 40, 41, 42, 43, 44, 45
};

static const uint8_t huff_b52_cb[11] = { 1, 9, 1, 0, 1, 3, 2, 3, 7, 4, 4 };

static const uint8_t huff_b52_xlat[25] = {
    0x00, 0x01, 0x08, 0x38, 0x07, 0x39, 0x0F, 0x30, 0x09, 0x3F, 0x10, 0x31,
    0x02, 0x3A, 0x06, 0x0E, 0x17, 0x11, 0x0A, 0x3E, 0x37, 0x12, 0x32, 0x16,
    0x36
};

static const uint8_t huff_b53_cb[7] = { 1, 5, 1, 1, 1, 0, 4 }; // same as b63!!!

static const uint8_t huff_b53_xlat[7] = { 0, 7, 1, 2, 3, 5, 6 };

static const uint8_t huff_b56_cb[11] = { 1, 9, 1, 0, 2, 0, 2, 4, 11, 9, 2 };

static const uint8_t huff_b56_xlat[31] = {
     0,  1, 31,  2, 30, 3,  4, 13, 29,  5,  6,  7, 14, 15, 17, 18,
    19, 26, 27, 28,  8, 9, 12, 20, 21, 22, 23, 24, 25, 10, 11
};

static const uint8_t huff_b62_cb[14] = {
    1, 12, 1, 0, 0, 2, 3, 5, 12, 14, 18, 15, 9, 2
};

static const uint8_t huff_b62_xlat[81] = {
    0x00, 0x40, 0x01, 0x10, 0x04, 0x02, 0x80, 0x50, 0x90, 0x05, 0x06, 0x20,
    0x60, 0x44, 0x14, 0x54, 0x24, 0x08, 0x18, 0x41, 0x11, 0x15, 0x09, 0xA0,
    0x84, 0x94, 0x64, 0xA4, 0x28, 0x51, 0x45, 0x55, 0x19, 0x12, 0x16, 0x0A,
    0x1A, 0x48, 0x58, 0x68, 0x81, 0x91, 0x21, 0x61, 0x85, 0x95, 0x25, 0x65,
    0x49, 0x59, 0x29, 0x69, 0x42, 0x46, 0x56, 0x88, 0x98, 0xA8, 0xA1, 0xA5,
    0x99, 0xA9, 0x52, 0x22, 0x26, 0x66, 0x4A, 0x5A, 0x2A, 0x6A, 0x89, 0x82,
    0x92, 0x62, 0x86, 0x96, 0xA6, 0x8A, 0xAA, 0xA2, 0x9A
};

static const uint8_t huff_b63_cb[7] = { 1, 5, 1, 1, 1, 0, 4 };

static const uint8_t huff_b63_xlat[7] = { 0, 1, 7, 2, 3, 5, 6 };

static const uint8_t huff_b64_cb[7] = { 1, 5, 1, 1, 1, 1, 2 };

static const uint8_t huff_b64_xlat[6] = { 1, 0, 2, 5, 3, 4 };

static const uint8_t huff_b65_cb[14] = {
    1, 12, 1, 0, 0, 2, 2, 2, 6, 12, 34, 92, 54, 20
};

static const uint8_t huff_b65_xlat[225] = {
    0x00, 0xF0, 0x01, 0x10, 0x0F, 0x11, 0xF1, 0x20, 0xE0, 0x02, 0x0E, 0x1F,
    0xFF, 0xD0, 0x21, 0xE1, 0x12, 0xF2, 0x07, 0x0A, 0x0D, 0x1E, 0xFE, 0x2F,
    0xEF, 0x30, 0x70, 0x90, 0xA0, 0xC0, 0x71, 0x91, 0xC1, 0xD1, 0x32, 0x92,
    0xE2, 0x03, 0x13, 0x63, 0x04, 0x06, 0xE6, 0xE7, 0xF7, 0x09, 0x19, 0x39,
    0xFA, 0x0C, 0x1C, 0xDD, 0xED, 0xFD, 0x2E, 0x7E, 0x9E, 0x3F, 0x9F, 0x40,
    0x50, 0x60, 0xB0, 0x31, 0x41, 0x61, 0xA1, 0xB1, 0x22, 0x42, 0x72, 0xA2,
    0xB2, 0xC2, 0xD2, 0x23, 0x33, 0x73, 0xA3, 0xC3, 0xD3, 0xE3, 0xF3, 0x14,
    0x24, 0x34, 0x44, 0x74, 0xD4, 0xE4, 0x05, 0x25, 0x45, 0x65, 0x95, 0xA5,
    0x16, 0x26, 0x46, 0x76, 0xA6, 0xB6, 0xC6, 0xD6, 0xF6, 0x17, 0x27, 0x37,
    0x47, 0x67, 0xA7, 0xD7, 0x29, 0x69, 0xB9, 0xD9, 0xE9, 0xF9, 0x1A, 0x2A,
    0x3A, 0x9A, 0xCA, 0xDA, 0xEA, 0x0B, 0x1B, 0x3B, 0x6B, 0xEB, 0xFB, 0x2C,
    0x6C, 0xEC, 0xFC, 0x1D, 0x2D, 0x4D, 0x6D, 0x9D, 0xAD, 0x3E, 0x4E, 0x6E,
    0xAE, 0xCE, 0xEE, 0x4F, 0x5F, 0x6F, 0xDF, 0x51, 0x52, 0x62, 0x43, 0x93,
    0xB3, 0x54, 0x94, 0xA4, 0xF4, 0x15, 0x75, 0xB5, 0xE5, 0xF5, 0x36, 0x56,
    0x66, 0x96, 0x57, 0x77, 0x49, 0x59, 0xA9, 0xC9, 0x4A, 0x5A, 0x6A, 0x7A,
    0xAA, 0xBA, 0x2B, 0x4B, 0x7B, 0x9B, 0xAB, 0xDB, 0x3C, 0x4C, 0x7C, 0x9C,
    0xAC, 0xBC, 0xCC, 0x3D, 0x5D, 0x7D, 0xBD, 0xCD, 0x5E, 0xBE, 0xDE, 0xBF,
    0xCF, 0x53, 0x64, 0xB4, 0xC4, 0x35, 0x55, 0xC5, 0xD5, 0x97, 0xB7, 0xC7,
    0x79, 0x99, 0x5B, 0xBB, 0xCB, 0x5C, 0xDC, 0x7F, 0xAF
};

static const uint8_t huff_b66_cb[14] = {
    1, 12, 1, 0, 0, 3, 0, 3, 3, 10, 40, 85, 61, 50
};

static const uint8_t huff_b66_xlat[256] = {
    0x00, 0x10, 0x01, 0x11, 0x21, 0x02, 0x12, 0x20, 0x22, 0x13, 0x30, 0x31,
    0x41, 0xD1, 0xE1, 0x32, 0x52, 0x03, 0x23, 0x2D, 0x40, 0x50, 0x60, 0x80,
    0xD0, 0xE0, 0x51, 0x61, 0xF1, 0x42, 0x62, 0xD2, 0xE2, 0xF2, 0x33, 0x43,
    0xC3, 0xD3, 0xE3, 0x04, 0x14, 0xD4, 0xF4, 0x25, 0x35, 0x16, 0x17, 0xF7,
    0xD8, 0x1C, 0x3C, 0x0D, 0x1D, 0x3D, 0x5D, 0x0E, 0x1E, 0x2E, 0x7E, 0x2F,
    0xC0, 0xF0, 0x71, 0x81, 0x91, 0xC1, 0x72, 0x82, 0x92, 0xB2, 0xC2, 0x53,
    0x63, 0x73, 0x93, 0xA3, 0xF3, 0x24, 0x44, 0x64, 0x84, 0xA4, 0xB4, 0x05,
    0x15, 0x95, 0xD5, 0x06, 0x26, 0x36, 0x46, 0x96, 0xD6, 0xE6, 0xF6, 0x07,
    0x27, 0x37, 0xD7, 0xE7, 0x08, 0x18, 0x28, 0x38, 0xE8, 0xF8, 0x09, 0x19,
    0x29, 0xE9, 0xF9, 0x0A, 0x1A, 0xCA, 0xDA, 0xEA, 0x0B, 0x1B, 0xDB, 0xEB,
    0xFB, 0x2C, 0x4C, 0x5C, 0x7C, 0x8C, 0x4D, 0x6D, 0x8D, 0x9D, 0xFD, 0x3E,
    0x5E, 0x6E, 0x8E, 0x9E, 0xEE, 0x0F, 0x1F, 0x3F, 0x4F, 0x5F, 0x6F, 0x7F,
    0xCF, 0x70, 0xA1, 0xA2, 0x83, 0xB3, 0x34, 0x74, 0xC4, 0xE4, 0x55, 0x65,
    0x85, 0xA5, 0xC5, 0xE5, 0xF5, 0x56, 0x66, 0x76, 0x86, 0xA6, 0xC6, 0x57,
    0x67, 0x77, 0x97, 0xA7, 0x48, 0x88, 0x98, 0x49, 0x59, 0x79, 0x99, 0x3A,
    0x4A, 0x8A, 0xBA, 0xFA, 0x2B, 0x7B, 0x0C, 0xAC, 0xBC, 0xCC, 0xEC, 0x7D,
    0xAD, 0xBD, 0xDD, 0x4E, 0xBE, 0xCE, 0xFE, 0x8F, 0x9F, 0xAF, 0xBF, 0xDF,
    0xEF, 0xFF, 0x90, 0xA0, 0xB0, 0xB1, 0x54, 0x94, 0x45, 0x75, 0xB5, 0xB6,
    0x47, 0x87, 0xB7, 0xC7, 0x58, 0x68, 0x78, 0xA8, 0xB8, 0xC8, 0x39, 0x69,
    0x89, 0xA9, 0xB9, 0xC9, 0xD9, 0x2A, 0x5A, 0x6A, 0x7A, 0x9A, 0xAA, 0x3B,
    0x4B, 0x5B, 0x6B, 0x8B, 0x9B, 0xAB, 0xBB, 0xCB, 0x6C, 0x9C, 0xDC, 0xFC,
    0xCD, 0xED, 0xAE, 0xDE
};

static const uint8_t huff_b67_cb[10] = { 2, 9, 1, 2, 1, 4, 7, 10, 26, 12 };

static const uint8_t huff_b67_xlat[63] = {
     0,  1, 63, 62,  2,  3, 60, 61,  4,  5,  6,  7, 57, 58, 59,  8,
     9, 10, 11, 12, 52, 53, 54, 55, 56, 13, 14, 15, 16, 17, 18, 25,
    26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39, 46, 47, 48,
    49, 50, 51, 19, 20, 21, 22, 23, 24, 40, 41, 42, 43, 44, 45
};

static const uint8_t huff_b71_cb[14] = {
    1, 12, 1, 0, 0, 1, 7, 0, 19, 5, 13, 23, 0, 12
};

static const uint8_t huff_b71_xlat[81] = {
    0x00, 0x03, 0x40, 0xC0, 0x10, 0x30, 0x04, 0x0C, 0x01, 0x50, 0xD0, 0x70,
    0xF0, 0xC4, 0x14, 0x34, 0x4C, 0x1C, 0x3C, 0xC1, 0x11, 0x31, 0x05, 0x0D,
    0x13, 0x33, 0x07, 0x0F, 0x44, 0xCC, 0x41, 0x43, 0xC3, 0x54, 0x74, 0xDC,
    0xFC, 0xF1, 0xC5, 0x15, 0x1D, 0x53, 0xC7, 0x37, 0x4F, 0x3F, 0xD4, 0xF4,
    0x5C, 0x7C, 0x51, 0xD1, 0x71, 0x45, 0x55, 0x35, 0x4D, 0xCD, 0xDD, 0x3D,
    0xD3, 0x73, 0xF3, 0x47, 0x17, 0x77, 0xCF, 0x1F, 0xFF, 0xD5, 0x75, 0xF5,
    0x5D, 0x7D, 0xFD, 0x57, 0xD7, 0xF7, 0x5F, 0xDF, 0x7F
};

static const uint8_t huff_b73_cb[13] = {
    1, 11, 1, 0, 0, 0, 1, 4, 9, 4, 103, 110, 24
};

static const uint8_t huff_b73_xlat[256] = {
    0x00, 0x40, 0x10, 0x04, 0x01, 0x05, 0x50, 0x14, 0x54, 0x41, 0x11, 0x51,
    0x45, 0x15, 0x55, 0x44, 0x95, 0x6A, 0x03, 0x80, 0xC0, 0x90, 0xD0, 0x94,
    0xD4, 0x24, 0x64, 0x58, 0x91, 0xA1, 0x85, 0xD5, 0x25, 0x65, 0xA5, 0xE5,
    0x75, 0xB5, 0xF5, 0x19, 0x59, 0x99, 0xD9, 0x69, 0xA9, 0xE9, 0x79, 0xB9,
    0xF9, 0x4D, 0x5D, 0x9D, 0xDD, 0x6D, 0xAD, 0xED, 0x7D, 0xBD, 0xFD, 0x02,
    0x42, 0x52, 0x06, 0x46, 0x16, 0x56, 0x96, 0xD6, 0x26, 0x66, 0xA6, 0xE6,
    0x76, 0xB6, 0xF6, 0x1A, 0x5A, 0x9A, 0xDA, 0xAA, 0xEA, 0x7A, 0xBA, 0xFA,
    0x5E, 0x9E, 0xDE, 0x6E, 0xAE, 0xEE, 0x7E, 0xBE, 0xFE, 0x07, 0x47, 0x57,
    0x97, 0xD7, 0x67, 0xA7, 0xE7, 0x77, 0xB7, 0xF7, 0x5B, 0x9B, 0xDB, 0x6B,
    0xAB, 0xEB, 0x7B, 0xBB, 0xFB, 0x5F, 0x9F, 0xDF, 0x6F, 0xAF, 0xEF, 0x7F,
    0xBF, 0xFF, 0x20, 0x60, 0x70, 0xB0, 0xF0, 0x84, 0xC4, 0xA4, 0xE4, 0x74,
    0xB4, 0xF4, 0x08, 0x88, 0x18, 0x98, 0xD8, 0x68, 0xA8, 0xE8, 0x78, 0xB8,
    0xF8, 0x1C, 0x5C, 0x9C, 0xDC, 0x6C, 0xAC, 0xEC, 0x3C, 0x7C, 0xBC, 0xFC,
    0x81, 0xD1, 0x21, 0x61, 0xE1, 0x71, 0xB1, 0xF1, 0xC5, 0x35, 0x09, 0x49,
    0x89, 0xC9, 0x29, 0x39, 0x0D, 0x8D, 0xCD, 0x1D, 0x2D, 0x3D, 0x92, 0xD2,
    0x22, 0x62, 0xA2, 0xE2, 0x72, 0xB2, 0xF2, 0x86, 0xC6, 0x36, 0x0A, 0x4A,
    0x8A, 0xCA, 0x2A, 0x3A, 0x0E, 0x4E, 0x8E, 0xCE, 0x1E, 0x2E, 0x3E, 0x13,
    0x53, 0x93, 0xD3, 0x63, 0xA3, 0xE3, 0x73, 0xB3, 0xF3, 0x87, 0xC7, 0x17,
    0x27, 0x37, 0x0B, 0x4B, 0x8B, 0xCB, 0x1B, 0x2B, 0x3B, 0x0F, 0x4F, 0x8F,
    0xCF, 0x1F, 0x2F, 0x3F, 0xA0, 0xE0, 0x30, 0x34, 0x48, 0xC8, 0x28, 0x38,
    0x0C, 0x4C, 0x8C, 0xCC, 0x2C, 0xC1, 0x31, 0x82, 0xC2, 0x12, 0x32, 0x43,
    0x83, 0xC3, 0x23, 0x33
};

static const uint8_t huff_b74_cb[8] = { 1, 6, 1, 0, 2, 2, 2, 4 };

static const uint8_t huff_b74_xlat[11] = {
    0, 1, 15, 2, 14, 5, 13, 3, 4, 11, 12
};

static const uint8_t huff_b75_cb[13] = {
    2, 12, 1, 4, 0, 0, 0, 8, 11, 24, 53, 64, 60
};

static const uint8_t huff_b75_xlat[225] = {
    0x00, 0x10, 0xF0, 0x01, 0x0F, 0x20, 0xE0, 0x11, 0xF1, 0x02, 0x0E, 0x1F,
    0xFF, 0xD0, 0x21, 0xE1, 0x12, 0xF2, 0x03, 0x0D, 0x1E, 0xFE, 0x2F, 0xEF,
    0x30, 0x40, 0x60, 0x70, 0x90, 0xA0, 0xC0, 0x31, 0xD1, 0x22, 0xE2, 0x13,
    0xF3, 0x04, 0x06, 0x07, 0x09, 0x0C, 0x1D, 0xFD, 0x2E, 0xEE, 0x3F, 0xDF,
    0x50, 0xB0, 0x41, 0x61, 0x71, 0x91, 0xA1, 0xC1, 0x32, 0x62, 0x72, 0x92,
    0xA2, 0xD2, 0x23, 0xD3, 0xE3, 0x14, 0xF4, 0x05, 0x16, 0x26, 0xE6, 0xF6,
    0x17, 0x27, 0xE7, 0xF7, 0x19, 0x29, 0xF9, 0x0A, 0x1A, 0x2A, 0xFA, 0x0B,
    0x1C, 0x2C, 0xFC, 0x2D, 0x3D, 0xED, 0x3E, 0x4E, 0x7E, 0x9E, 0xDE, 0x4F,
    0x6F, 0x7F, 0x9F, 0xAF, 0xCF, 0x51, 0xB1, 0x42, 0x52, 0xB2, 0xC2, 0x33,
    0x63, 0x73, 0x93, 0xA3, 0xB3, 0xC3, 0x24, 0x34, 0x74, 0xA4, 0xD4, 0xE4,
    0x15, 0x25, 0x65, 0x95, 0xE5, 0xF5, 0x36, 0xD6, 0x37, 0x47, 0xC7, 0xD7,
    0x39, 0x59, 0xB9, 0xC9, 0xD9, 0xE9, 0x3A, 0x6A, 0xDA, 0xEA, 0x1B, 0x2B,
    0x9B, 0xAB, 0xEB, 0xFB, 0x6C, 0x7C, 0x9C, 0xAC, 0xEC, 0x4D, 0x6D, 0x7D,
    0x9D, 0xAD, 0xBD, 0xDD, 0x5E, 0x6E, 0xAE, 0xCE, 0x5F, 0x43, 0x53, 0x44,
    0x54, 0x64, 0x94, 0xB4, 0xC4, 0x35, 0x45, 0x55, 0x75, 0xA5, 0xB5, 0xC5,
    0xD5, 0x46, 0x56, 0x66, 0x76, 0x96, 0xA6, 0xB6, 0xC6, 0x57, 0x67, 0x77,
    0x97, 0xA7, 0xB7, 0x49, 0x69, 0x79, 0x99, 0xA9, 0x4A, 0x5A, 0x7A, 0x9A,
    0xAA, 0xBA, 0xCA, 0x3B, 0x4B, 0x5B, 0x6B, 0x7B, 0xBB, 0xCB, 0xDB, 0x3C,
    0x4C, 0x5C, 0xBC, 0xCC, 0xDC, 0x5D, 0xCD, 0xBE, 0xBF
};

static const uint8_t huff_b77_cb[12] = {
    2, 11, 1, 0, 4, 6, 10, 12, 7, 15, 4, 4
};

static const uint8_t huff_b77_xlat[63] = {
     0,  1,  2, 62, 63,  3,  4,  5, 59, 60, 61,  6,  7,  8,  9, 10,
    54, 55, 56, 57, 58, 11, 12, 13, 14, 15, 16, 47, 49, 50, 51, 52,
    53, 17, 18, 19, 20, 45, 46, 48, 21, 22, 23, 24, 25, 26, 27, 37,
    38, 39, 40, 41, 42, 43, 44, 28, 29, 30, 35, 31, 33, 34, 36
};

/** Tables for spectrum coding. */
typedef struct Atrac3pSpecCodeTab {
    uint8_t group_size;  ///< number of coefficients grouped together
    uint8_t num_coeffs;  ///< 1 - map index to a single value, > 1 - map index to a vector of values
    uint8_t bits;        ///< number of bits a single coefficient occupy
    uint8_t is_signed;   ///< 1 - values in that table are signed ones, otherwise - absolute ones

    int redirect;        ///< if >= 0: tells which huffman table must be reused
    const uint8_t *cb;   ///< pointer to the codebook descriptor
    const uint8_t *xlat; ///< pointer to the translation table or NULL if none
} Atrac3pSpecCodeTab;

static const Atrac3pSpecCodeTab atrac3p_spectra_tabs[112] = {
    /* table set = A */
    /* code table = 0 */
    { 1, 4, 2, 1, -1, huff_a01_cb, huff_a01_xlat }, // wordlen = 1
    { 1, 4, 2, 0, -1, huff_a02_cb, huff_a02_xlat }, // wordlen = 2
    { 1, 2, 3, 1, -1, huff_a03_cb, huff_a03_xlat }, // wordlen = 3
    { 1, 1, 3, 0, -1, huff_a04_cb, huff_a04_xlat }, // wordlen = 4
    { 1, 2, 4, 1, -1, huff_a05_cb, huff_a05_xlat }, // wordlen = 5
    { 1, 1, 4, 0, -1, huff_a06_cb, huff_a06_xlat }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_a07_cb, huff_a07_xlat }, // wordlen = 7

    /* code table = 1 */
    { 4, 4, 2, 1, -1, huff_a11_cb, huff_a11_xlat }, // wordlen = 1
    { 1, 4, 2, 0, -1, huff_a12_cb, huff_a12_xlat }, // wordlen = 2
    { 1, 2, 3, 1, -1, huff_a13_cb, huff_a13_xlat }, // wordlen = 3
    { 1, 2, 4, 1, -1, huff_a14_cb, huff_a14_xlat }, // wordlen = 4
    { 1, 2, 4, 1, -1, huff_a15_cb, huff_a15_xlat }, // wordlen = 5
    { 1, 2, 4, 0, -1, huff_a16_cb, huff_a16_xlat }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_a17_cb, huff_a17_xlat }, // wordlen = 7

    /* code table = 2 */
    { 1, 4, 2, 1, -1, huff_a21_cb, huff_a21_xlat }, // wordlen = 1
    { 1, 2, 3, 1, -1, huff_a22_cb, huff_a22_xlat }, // wordlen = 2
    { 1, 2, 3, 1, -1, huff_a23_cb, huff_a23_xlat }, // wordlen = 3
    { 1, 1, 3, 0, -1, huff_a24_cb, NULL          }, // wordlen = 4
    { 1, 1, 3, 0, -1, huff_a25_cb, huff_a25_xlat }, // wordlen = 5
    { 1, 2, 4, 0, -1, huff_a26_cb, huff_a26_xlat }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_a27_cb, huff_a27_xlat }, // wordlen = 7

    /* code table = 3 */
    { 1, 2, 2, 1, -1, huff_a31_cb, huff_a31_xlat }, // wordlen = 1
    { 1, 4, 2, 0, -1, huff_a32_cb, huff_a32_xlat }, // wordlen = 2
    { 1, 4, 2, 0, -1, huff_a33_cb, huff_a33_xlat }, // wordlen = 3
    { 1, 1, 3, 0, -1, huff_a34_cb, huff_a34_xlat }, // wordlen = 4
    { 1, 2, 4, 1, -1, huff_a35_cb, huff_a35_xlat }, // wordlen = 5
    { 1, 2, 4, 0, -1, huff_a36_cb, huff_a36_xlat }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_a37_cb, huff_a37_xlat }, // wordlen = 7

    /* code table = 4 */
    { 1, 4, 2, 1, -1, huff_a41_cb, huff_a41_xlat }, // wordlen = 1
    { 1, 4, 2, 0, -1, huff_a42_cb, huff_a42_xlat }, // wordlen = 2
    { 1, 1, 3, 1, -1, huff_a43_cb, huff_a43_xlat }, // wordlen = 3
    { 1, 2, 4, 1, -1, huff_a44_cb, huff_a44_xlat }, // wordlen = 4
    { 1, 1, 3, 0, -1, huff_a45_cb, huff_a45_xlat }, // wordlen = 5
    { 1, 2, 4, 0, -1, huff_a46_cb, huff_a46_xlat }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_a47_cb, huff_a47_xlat }, // wordlen = 7

    /* code table = 5 */
    { 1, 4, 2, 1, -1, huff_a51_cb, huff_a51_xlat }, // wordlen = 1
    { 1, 2, 3, 1, -1, huff_a52_cb, huff_a52_xlat }, // wordlen = 2
    { 1, 1, 3, 1, -1, huff_a43_cb, huff_a53_xlat }, // wordlen = 3
    { 1, 2, 4, 1, -1, huff_a54_cb, huff_a54_xlat }, // wordlen = 4
    { 1, 1, 3, 0, -1, huff_a55_cb, huff_a55_xlat }, // wordlen = 5
    { 1, 1, 5, 1, -1, huff_a56_cb, huff_a56_xlat }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_a57_cb, huff_a57_xlat }, // wordlen = 7

    /* code table = 6 */
    { 2, 4, 2, 1, -1, huff_a61_cb, huff_a61_xlat }, // wordlen = 1
    { 1, 2, 3, 1, -1, huff_a62_cb, huff_a62_xlat }, // wordlen = 2
    { 1, 4, 2, 0, -1, huff_a63_cb, huff_a63_xlat }, // wordlen = 3
    { 1, 2, 4, 1, -1, huff_a64_cb, huff_a64_xlat }, // wordlen = 4
    { 1, 1, 4, 1, -1, huff_a65_cb, huff_a65_xlat }, // wordlen = 5
    { 1, 1, 5, 1, -1, huff_a66_cb, huff_a66_xlat }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_a67_cb, huff_a67_xlat }, // wordlen = 7

    /* code table = 7 */
    { 1, 2, 1, 0, -1, huff_a71_cb, NULL          }, // wordlen = 1
    { 2, 4, 2, 0, -1, huff_a72_cb, huff_a72_xlat }, // wordlen = 2
    { 1, 2, 3, 1, -1, huff_a73_cb, huff_a73_xlat }, // wordlen = 3
    { 1, 2, 4, 1, -1, huff_a74_cb, huff_a74_xlat }, // wordlen = 4
    { 1, 1, 4, 1, -1, huff_a75_cb, huff_a75_xlat }, // wordlen = 5
    { 2, 2, 4, 0, -1, huff_a76_cb, huff_a76_xlat }, // wordlen = 6
    { 4, 1, 6, 1,  6, NULL,        NULL          }, // wordlen = 7

    /* table set = B */
    /* code table = 0 */
    { 4, 4, 2, 1, -1, huff_b01_cb, huff_b01_xlat }, // wordlen = 1
    { 1, 4, 2, 0, -1, huff_b02_cb, huff_b02_xlat }, // wordlen = 2
    { 4, 2, 3, 1, -1, huff_b03_cb, huff_b03_xlat }, // wordlen = 3
    { 1, 2, 4, 1, -1, huff_b04_cb, huff_b04_xlat }, // wordlen = 4
    { 1, 2, 4, 1, -1, huff_b05_cb, huff_b05_xlat }, // wordlen = 5
    { 1, 1, 4, 0,  5, NULL,        NULL          }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_b07_cb, huff_b07_xlat }, // wordlen = 7

    /* code table = 1 */
    { 1, 4, 2, 1, 14, NULL,        NULL          }, // wordlen = 1
    { 1, 4, 2, 0, -1, huff_b12_cb, huff_b12_xlat }, // wordlen = 2
    { 1, 2, 3, 1,  9, NULL,        NULL          }, // wordlen = 3
    { 1, 2, 4, 1, -1, huff_b14_cb, huff_b14_xlat }, // wordlen = 4
    { 1, 2, 4, 1, 11, NULL,        NULL          }, // wordlen = 5
    { 1, 2, 4, 0, -1, huff_b16_cb, huff_b16_xlat }, // wordlen = 6
    { 1, 1, 6, 1,  6, NULL,        NULL          }, // wordlen = 7

    /* code table = 2 */
    { 4, 4, 2, 1, 28, NULL,        NULL          }, // wordlen = 1
    { 4, 4, 2, 0, 22, NULL,        NULL          }, // wordlen = 2
    { 1, 2, 3, 1,  2, NULL,        NULL          }, // wordlen = 3
    { 1, 2, 4, 1, 31, NULL,        NULL          }, // wordlen = 4
    { 2, 2, 4, 1, 60, NULL,        NULL          }, // wordlen = 5
    { 2, 2, 4, 0, -1, huff_b26_cb, huff_b26_xlat }, // wordlen = 6
    { 4, 1, 6, 1,  6, NULL,        NULL          }, // wordlen = 7

    /* code table = 3 */
    { 1, 4, 2, 1, 35, NULL,        NULL          }, // wordlen = 1
    { 1, 4, 2, 0, -1, huff_b32_cb, huff_b32_xlat }, // wordlen = 2
    { 1, 4, 2, 0, -1, huff_b33_cb, huff_b33_xlat }, // wordlen = 3
    { 2, 2, 4, 1, 59, NULL,        NULL          }, // wordlen = 4
    { 1, 2, 4, 1, -1, huff_b35_cb, huff_b35_xlat }, // wordlen = 5
    { 1, 2, 4, 0, 75, NULL,        NULL          }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_b37_cb, huff_b37_xlat }, // wordlen = 7

    /* code table = 4 */
    { 1, 4, 2, 1, -1, huff_b41_cb, huff_b41_xlat }, // wordlen = 1
    { 4, 2, 3, 1, -1, huff_b42_cb, huff_b42_xlat }, // wordlen = 2
    { 1, 2, 3, 1, -1, huff_b43_cb, huff_b43_xlat }, // wordlen = 3
    { 4, 2, 4, 1, 66, NULL,        NULL          }, // wordlen = 4
    { 1, 1, 3, 0, 32, NULL,        NULL          }, // wordlen = 5
    { 1, 2, 4, 0, 12, NULL,        NULL          }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_b47_cb, huff_b47_xlat }, // wordlen = 7

    /* code table = 5 */
    { 2, 4, 2, 1, 42, NULL,        NULL          }, // wordlen = 1
    { 1, 2, 3, 1, -1, huff_b52_cb, huff_b52_xlat }, // wordlen = 2
    { 4, 1, 3, 1, -1, huff_b53_cb, huff_b53_xlat }, // wordlen = 3
    { 1, 1, 3, 0, 17, NULL,        NULL          }, // wordlen = 4
    { 1, 1, 3, 0, 39, NULL,        NULL          }, // wordlen = 5
    { 1, 1, 5, 1, -1, huff_b56_cb, huff_b56_xlat }, // wordlen = 6
    { 2, 1, 6, 1, 62, NULL,        NULL          }, // wordlen = 7

    /* code table = 6 */
    { 1, 4, 2, 1, 28, NULL,        NULL          }, // wordlen = 1
    { 1, 4, 2, 0, -1, huff_b62_cb, huff_b62_xlat }, // wordlen = 2
    { 1, 1, 3, 1, -1, huff_b63_cb, huff_b63_xlat }, // wordlen = 3
    { 1, 1, 3, 0, -1, huff_b64_cb, huff_b64_xlat }, // wordlen = 4
    { 4, 2, 4, 1, -1, huff_b65_cb, huff_b65_xlat }, // wordlen = 5
    { 1, 2, 4, 0, -1, huff_b66_cb, huff_b66_xlat }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_b67_cb, huff_b67_xlat }, // wordlen = 7

    /* code table = 7 */
    { 1, 4, 2, 1, -1, huff_b71_cb, huff_b71_xlat }, // wordlen = 1
    { 4, 4, 2, 0, 78, NULL,        NULL          }, // wordlen = 2
    { 4, 4, 2, 0, -1, huff_b73_cb, huff_b73_xlat }, // wordlen = 3
    { 1, 1, 4, 1, -1, huff_b74_cb, huff_b74_xlat }, // wordlen = 4
    { 1, 2, 4, 1, -1, huff_b75_cb, huff_b75_xlat }, // wordlen = 5
    { 1, 1, 5, 1, 47, NULL,        NULL          }, // wordlen = 6
    { 1, 1, 6, 1, -1, huff_b77_cb, huff_b77_xlat }, // wordlen = 7
};

/* Huffman tables for gain control data. */
static const uint8_t atrac3p_huff_gain_npoints1_cb[9] = {
    1, 7, 1, 1, 1, 1, 1, 1, 2
};

static const uint8_t atrac3p_huff_gain_npoints2_xlat[8] = {
    0, 1, 7, 2, 6, 3, 4, 5
};

static const uint8_t atrac3p_huff_gain_lev1_cb[9] = { 1, 7, 1, 0, 2, 2, 1, 2, 8 };
static const uint8_t atrac3p_huff_gain_lev1_xlat[16] = {
    7, 5, 8, 6, 9, 4, 10, 11, 0, 1, 2, 3, 12, 13, 14, 15
};

static const uint8_t atrac3p_huff_gain_lev2_cb[11] = {
    1, 9, 1, 1, 1, 1, 1, 0, 2, 0, 8
};

static const uint8_t atrac3p_huff_gain_lev2_xlat[15] = {
    15, 14, 1, 13, 2, 3, 12, 4, 5, 6, 7, 8, 9, 10, 11
};

static const uint8_t atrac3p_huff_gain_lev3_cb[11] = {
    1, 9, 1, 0, 3, 1, 1, 0, 2, 0, 8
};

static const uint8_t atrac3p_huff_gain_lev3_xlat[16] = {
    0, 1, 14, 15, 2, 13, 3, 12, 4, 5, 6, 7, 8, 9, 10, 11
};

static const uint8_t atrac3p_huff_gain_lev4_cb[11] = {
    1, 9, 1, 1, 1, 1, 1, 0, 1, 2, 8
};

static const uint8_t atrac3p_huff_gain_lev4_xlat[16] = {
    0, 1, 15, 14, 2, 13, 3, 12, 4, 5, 6, 7, 8, 9, 10, 11
};

static const uint8_t atrac3p_huff_gain_loc1_cb[9] = { 2, 8, 1, 2, 4, 4, 4, 0, 16 };
static const uint8_t atrac3p_huff_gain_loc1_xlat[31] = {
     1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

static const uint8_t atrac3p_huff_gain_loc2_cb[8] = { 3, 8, 5, 3, 2, 3, 2, 16 };
static const uint8_t atrac3p_huff_gain_loc2_xlat[31] = {
     2,  3,  4,  5,  6,  1,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

static const uint8_t atrac3p_huff_gain_loc3_cb[7] = { 2, 6, 1, 0, 2, 11, 18 };
static const uint8_t atrac3p_huff_gain_loc3_xlat[32] = {
    0,   1, 31,  2,  3,  4,  5,  6,  7, 26, 27, 28, 29, 30,  8,  9,
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25
};

static const uint8_t atrac3p_huff_gain_loc4_cb[5] = { 4, 6, 3, 23, 6 };
static const uint8_t atrac3p_huff_gain_loc4_xlat[32] = {
    0,  28, 29,  1,  2,  3,  4, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 30, 31,  5,  6,  7,  8,  9, 10
};

static const uint8_t atrac3p_huff_gain_loc5_cb[9] = { 1, 7, 1, 0, 0, 3, 2, 6, 20 };
static const uint8_t atrac3p_huff_gain_loc5_xlat[32] = {
    0,   1,  2, 31,  3,  4,  5,  6,  7,  8, 29, 30,  9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28
};

/* Huffman tables for GHA waves data. */
static const uint8_t atrac3p_huff_tonebands_cb[8] = { 1, 6, 1, 0, 1, 2, 4, 8 };
static const uint8_t atrac3p_huff_numwavs1_cb[9] = { 1, 7, 1, 1, 1, 1, 1, 1, 2 };
static const uint8_t atrac3p_huff_numwavs2_cb[8] = { 1, 6, 1, 1, 1, 1, 0, 4 };
static const uint8_t atrac3p_huff_numwavs2_xlat[8] = { 0, 1, 7, 2, 3, 4, 5, 6 };
static const uint8_t atrac3p_huff_wav_ampsf1_cb[7] = { 4, 8, 10, 8, 6, 0, 8 };
static const uint8_t atrac3p_huff_wav_ampsf1_xlat[32] = {
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 5, 6,  7, 18, 19, 20,
    21, 22,  4, 23, 24, 25, 26, 27,  0,  1, 2, 3, 28, 29, 30, 31
};

static const uint8_t atrac3p_huff_wav_ampsf2_cb[7] = { 4, 8, 11, 5, 6, 6, 4 };
static const uint8_t atrac3p_huff_wav_ampsf2_xlat[32] = {
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 14, 15, 16, 17, 29,
     9, 10, 11, 12, 13, 30,  4,  5,  6,  7,  8, 31,  0,  1,  2,  3
};

static const uint8_t atrac3p_huff_wav_ampsf3_cb[9] = { 2, 8, 1, 3, 3, 1, 4, 4, 16 };
static const uint8_t atrac3p_huff_wav_ampsf3_xlat[32] = {
    0, 1,  2, 31,  3, 29, 30,  4,  5,  6, 27, 28,  7, 24, 25, 26,
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
};

static const uint8_t atrac3p_huff_freq_cb[13] = {
    1, 11, 1, 0, 0, 2, 2, 0, 9, 9, 29, 104, 100
};

static const uint8_t atrac3p_huff_freq_xlat[256] = {
      0,   1, 255,   2, 254,   3,   4,   5,   6,   7,   8, 251, 252, 253,   9,  10,
     11,  12, 246, 247, 248, 249, 250,  13,  14,  15,  16,  17,  18,  19,  20,  21,
     22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,
     38, 243, 244, 245,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,
     51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,
     67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,
     83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93, 194, 195, 196, 197, 198,
    199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
    215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
    231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242,  94,  95,  96,  97,
     98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
    114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
    146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161,
    162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177,
    178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193
};


/* ============================================================================
 * VLC Implementation (get_bits.cpp)
 * ============================================================================ */
/*
 * Common bit i/o utils
 * Copyright (c) 2000, 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 * Copyright (c) 2010 Loren Merritt
 *
 * alternative bitstream reader & writer by Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * bitstream api.
 */



static const uint8_t ff_reverse[256] = {
0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF,
};

/* VLC decoding */

#define GET_DATA(v, table, i, wrap, size)                   \
{                                                           \
    const uint8_t *ptr = (const uint8_t *)table + i * wrap; \
    switch(size) {                                          \
    case 1:                                                 \
        v = *(const uint8_t *)ptr;                          \
        break;                                              \
    case 2:                                                 \
        v = *(const uint16_t *)ptr;                         \
        break;                                              \
    default:                                                \
        v = *(const uint32_t *)ptr;                         \
        break;                                              \
    }                                                       \
}

/* GET_DATA variant with explicit cast for narrowing conversions */
#define GET_DATA_U8(v, table, i, wrap, size)                \
{                                                           \
    const uint8_t *ptr = (const uint8_t *)table + i * wrap; \
    switch(size) {                                          \
    case 1:                                                 \
        v = *(const uint8_t *)ptr;                          \
        break;                                              \
    case 2:                                                 \
        v = (uint8_t)*(const uint16_t *)ptr;                \
        break;                                              \
    default:                                                \
        v = (uint8_t)*(const uint32_t *)ptr;                \
        break;                                              \
    }                                                       \
}

static int alloc_table(VLC *vlc, int size, int use_static)
{
    int index = vlc->table_size;

    vlc->table_size += size;
    if (vlc->table_size > vlc->table_allocated) {
        if (use_static)
            abort(); // cannot do anything, init_vlc() is used with too little memory
        vlc->table_allocated += (1 << vlc->bits);
        vlc->table = (VLC_TYPE(*)[2])realloc(vlc->table, vlc->table_allocated);
        if (!vlc->table) {
            vlc->table_allocated = 0;
            vlc->table_size = 0;
            return AVERROR(ENOMEM);
        }
        memset(vlc->table + vlc->table_allocated - (unsigned int)(1UL << vlc->bits), 0, sizeof(VLC_TYPE) * 2 << vlc->bits);
    }
    return index;
}

static inline uint32_t bitswap_32(uint32_t x)
{
    return (uint32_t)ff_reverse[ x        & 0xFF] << 24 |
           (uint32_t)ff_reverse[(x >> 8)  & 0xFF] << 16 |
           (uint32_t)ff_reverse[(x >> 16) & 0xFF] << 8  |
           (uint32_t)ff_reverse[ x >> 24];
}

typedef struct VLCcode {
    /** codeword, with the first bit-to-be-read in the msb
     * (even if intended for a little-endian bitstream reader) */
    uint32_t code;
    uint16_t symbol;
    uint8_t bits;
} VLCcode;

static int compare_vlcspec(const void *a, const void *b)
{
	const VLCcode *sa = (VLCcode *)a;
	const VLCcode *sb = (VLCcode *)b;
    return (sa->code >> 1) - (sb->code >> 1);
}
/**
 * Build VLC decoding tables suitable for use with get_vlc().
 *
 * @param vlc            the context to be initted
 *
 * @param table_nb_bits  max length of vlc codes to store directly in this table
 *                       (Longer codes are delegated to subtables.)
 *
 * @param nb_codes       number of elements in codes[]
 *
 * @param codes          descriptions of the vlc codes
 *                       These must be ordered such that codes going into the same subtable are contiguous.
 *                       Sorting by VLCcode.code is sufficient, though not necessary.
 */
static int build_table(VLC *vlc, int table_nb_bits, int nb_codes,
                       VLCcode *codes, int flags)
{
    int table_size, table_index, index, code_prefix, symbol, subtable_bits;
    int i, j, k, n, nb, inc;
    uint32_t code;
    volatile VLC_TYPE (* volatile table)[2]; // the double volatile is needed to prevent a internal compiler error in gcc 4.2f

    table_size = 1 << table_nb_bits;
    if (table_nb_bits > 30)
       return -1;
    table_index = alloc_table(vlc, table_size, flags & INIT_VLC_USE_NEW_STATIC);
    if (table_index < 0)
        return table_index;
    table = (volatile VLC_TYPE (*)[2])&vlc->table[table_index];

    /* first pass: map codes and compute auxiliary table sizes */
    for (i = 0; i < nb_codes; i++) {
        n      = codes[i].bits;
        code   = codes[i].code;
        symbol = codes[i].symbol;
        if (n <= table_nb_bits) {
            /* no need to add another table */
            j = code >> (32 - table_nb_bits);
            nb = 1 << (table_nb_bits - n);
            inc = 1;
            if (flags & INIT_VLC_LE) {
                j = bitswap_32(code);
                inc = 1 << n;
            }
            for (k = 0; k < nb; k++) {
                int bits = table[j][1];
                if (bits != 0 && bits != n) {
                    av_log(AV_LOG_ERROR, "incorrect codes");
                    return AVERROR_INVALIDDATA;
                }
                table[j][1] = n; //bits
                table[j][0] = symbol;
                j += inc;
            }
        } else {
            /* fill auxiliary table recursively */
            n -= table_nb_bits;
            code_prefix = code >> (32 - table_nb_bits);
            subtable_bits = n;
            codes[i].bits = n;
            codes[i].code = code << table_nb_bits;
            for (k = i+1; k < nb_codes; k++) {
                n = codes[k].bits - table_nb_bits;
                if (n <= 0)
                    break;
                code = codes[k].code;
                if (code >> (32 - table_nb_bits) != code_prefix)
                    break;
                codes[k].bits = n;
                codes[k].code = code << table_nb_bits;
                subtable_bits = FFMAX(subtable_bits, n);
            }
            subtable_bits = FFMIN(subtable_bits, table_nb_bits);
            j = (flags & INIT_VLC_LE) ? bitswap_32(code_prefix) >> (32 - table_nb_bits) : code_prefix;
            table[j][1] = -subtable_bits;
            index = build_table(vlc, subtable_bits, k-i, codes+i, flags);
            if (index < 0)
                return index;
            /* note: realloc has been done, so reload tables */
            table = (volatile VLC_TYPE (*)[2])&vlc->table[table_index];
            table[j][0] = index; //code
            i = k-1;
        }
    }

    for (i = 0; i < table_size; i++) {
        if (table[i][1] == 0) //bits
            table[i][0] = -1; //codes
    }

    return table_index;
}


/* Build VLC decoding tables suitable for use with get_vlc().

   'nb_bits' set the decoding table size (2^nb_bits) entries. The
   bigger it is, the faster is the decoding. But it should not be too
   big to save memory and L1 cache. '9' is a good compromise.

   'nb_codes' : number of vlcs codes

   'bits' : table which gives the size (in bits) of each vlc code.

   'codes' : table which gives the bit pattern of of each vlc code.

   'symbols' : table which gives the values to be returned from get_vlc().

   'xxx_wrap' : give the number of bytes between each entry of the
   'bits' or 'codes' tables.

   'xxx_size' : gives the number of bytes of each entry of the 'bits'
   or 'codes' tables.

   'wrap' and 'size' make it possible to use any memory configuration and types
   (byte/word/long) to store the 'bits', 'codes', and 'symbols' tables.

   'use_static' should be set to 1 for tables, which should be freed
   with av_free_static(), 0 if ff_free_vlc() will be used.
*/
int ff_init_vlc_sparse(VLC *vlc_arg, int nb_bits, int nb_codes,
                       const void *bits, int bits_wrap, int bits_size,
                       const void *codes, int codes_wrap, int codes_size,
                       const void *symbols, int symbols_wrap, int symbols_size,
                       int flags)
{
    VLCcode *buf;
    int i, j, ret;
    VLCcode localbuf[1500]; // the maximum currently needed is 1296 by rv34
    VLC localvlc, *vlc;

    vlc = vlc_arg;
    vlc->bits = nb_bits;
    if (flags & INIT_VLC_USE_NEW_STATIC) {
        av_assert0(nb_codes + 1 <= FF_ARRAY_ELEMS(localbuf));
        buf = localbuf;
        localvlc = *vlc_arg;
        vlc = &localvlc;
        vlc->table_size = 0;
    } else {
        vlc->table           = NULL;
        vlc->table_allocated = 0;
        vlc->table_size      = 0;

        buf = (VLCcode *)av_malloc_array((nb_codes + 1), sizeof(VLCcode));
        if (!buf)
            return AVERROR(ENOMEM);
    }


    av_assert0(symbols_size <= 2 || !symbols);
    j = 0;
#define COPY(condition)\
    for (i = 0; i < nb_codes; i++) {                                        \
        GET_DATA_U8(buf[j].bits, bits, i, bits_wrap, bits_size);            \
        if (!(condition))                                                   \
            continue;                                                       \
        if (buf[j].bits > 3*nb_bits || buf[j].bits>32) {                    \
            av_log(AV_LOG_ERROR, "Too long VLC (%d) in init_vlc", buf[j].bits);\
            if (!(flags & INIT_VLC_USE_NEW_STATIC))                         \
                av_free(buf);                                               \
            return -1;                                                      \
        }                                                                   \
        GET_DATA(buf[j].code, codes, i, codes_wrap, codes_size);            \
        if (buf[j].code >= (1LL<<buf[j].bits)) {                            \
            av_log(AV_LOG_ERROR, "Invalid code in init_vlc");               \
            if (!(flags & INIT_VLC_USE_NEW_STATIC))                         \
                av_free(buf);                                               \
            return -1;                                                      \
        }                                                                   \
        if (flags & INIT_VLC_LE)                                            \
            buf[j].code = bitswap_32(buf[j].code);                          \
        else                                                                \
            buf[j].code <<= 32 - buf[j].bits;                               \
        if (symbols)                                                        \
            GET_DATA(buf[j].symbol, symbols, i, symbols_wrap, symbols_size) \
        else                                                                \
            buf[j].symbol = i;                                              \
        j++;                                                                \
    }
    COPY(buf[j].bits > nb_bits);
    // qsort is the slowest part of init_vlc, and could probably be improved or avoided
    qsort(buf, j, sizeof(struct VLCcode), compare_vlcspec);
    COPY(buf[j].bits && buf[j].bits <= nb_bits);
    nb_codes = j;

    ret = build_table(vlc, nb_bits, nb_codes, buf, flags);

    if (flags & INIT_VLC_USE_NEW_STATIC) {
        if(vlc->table_size != vlc->table_allocated)
            av_log(AV_LOG_ERROR, "needed %d had %d", vlc->table_size, vlc->table_allocated);

        av_assert0(ret >= 0);
        *vlc_arg = *vlc;
    } else {
        av_free(buf);
        if (ret < 0) {
            free(vlc->table);
            vlc->table = 0;
            return ret;
        }
    }
    return 0;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static void ff_free_vlc(VLC *vlc) {
    free(vlc->table);
    vlc->table = NULL;
}

/* ============================================================================
 * FFT Implementation (fft.cpp)
 * ============================================================================ */
/*
 * FFT/IFFT transforms
 * Copyright (c) 2008 Loren Merritt
 * Copyright (c) 2002 Fabrice Bellard
 * Partly based on libdjbfft by D. J. Bernstein
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * FFT/IFFT transforms.
 */




#define sqrthalf (float)M_SQRT1_2

void imdct_calc(FFTContext *s, FFTSample *output, const FFTSample *input);
void imdct_half(FFTContext *s, FFTSample *output, const FFTSample *input);

/* cos tables already defined above */

// static FFTSample * const av_cos_tabs[] = {
//     NULL, NULL, NULL, NULL,
//     av_cos_16,
//     av_cos_32,
//     av_cos_64,
//     av_cos_128,
//     av_cos_256,
//     av_cos_512,
//     av_cos_1024,
// };

void fft_calc(FFTContext *s, FFTComplex *z);

static int split_radix_permutation(int i, int n, int inverse)
{
    int m;
    if(n <= 2) return i&1;
    m = n >> 1;
    if(!(i&m))            return split_radix_permutation(i, m, inverse)*2;
    m >>= 1;
    if(inverse == !(i&m)) return split_radix_permutation(i, m, inverse)*4 + 1;
    else                  return split_radix_permutation(i, m, inverse)*4 - 1;
}

// void ff_init_ff_cos_tabs(int index)
// {
//     int i;
//     int m = 1<<index;
//     double freq = 2*M_PI/m;
//     FFTSample *tab = av_cos_tabs[index];
//     for(i=0; i<=m/4; i++)
//         tab[i] = cos(i*freq);
//     for(i=1; i<m/4; i++)
//         tab[m/2-i] = tab[i];
// }

static const int avx_tab[] = {
    0, 4, 1, 5, 8, 12, 9, 13, 2, 6, 3, 7, 10, 14, 11, 15
};

#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static int is_second_half_of_fft32(int i, int n)
{
    if (n <= 32)
        return i >= 16;
    else if (i < n/2)
        return is_second_half_of_fft32(i, n/2);
    else if (i < 3*n/4)
        return is_second_half_of_fft32(i - n/2, n/4);
    else
        return is_second_half_of_fft32(i - 3*n/4, n/4);
}

int ff_fft_init(FFTContext *s, int nbits, int inverse)
{
    int i, j, n;

    if (nbits < 2 || nbits > 16)
        goto fail;
    s->nbits = nbits;
    n = 1 << nbits;

    s->revtab = (uint16_t *)av_malloc(n * sizeof(uint16_t));
    if (!s->revtab)
        goto fail;
    s->tmp_buf = (FFTComplex *)av_malloc(n * sizeof(FFTComplex));
    if (!s->tmp_buf)
        goto fail;
    s->inverse = inverse;

    for(j=4; j<=nbits; j++) {
        ff_init_ff_cos_tabs(j);
    }

    for(i=0; i<n; i++) {
        j = i;
		int index = -split_radix_permutation(i, n, s->inverse) & (n - 1);
        s->revtab[index] = j;
    }

    return 0;
 fail:
    av_freep(&s->revtab);
    av_freep(&s->tmp_buf);
    return -1;
}

void ff_fft_end(FFTContext *s)
{
    av_freep(&s->revtab);
    av_freep(&s->tmp_buf);
}

#define BF(x, y, a, b) do {                     \
        x = a - b;                              \
        y = a + b;                              \
    } while (0)

#define BUTTERFLIES(a0,a1,a2,a3) {\
    BF(t3, t5, t5, t1);\
    BF(a2.re, a0.re, a0.re, t5);\
    BF(a3.im, a1.im, a1.im, t3);\
    BF(t4, t6, t2, t6);\
    BF(a3.re, a1.re, a1.re, t4);\
    BF(a2.im, a0.im, a0.im, t6);\
}

// force loading all the inputs before storing any.
// this is slightly slower for small data, but avoids store->load aliasing
// for addresses separated by large powers of 2.
#define BUTTERFLIES_BIG(a0,a1,a2,a3) {\
    FFTSample r0=a0.re, i0=a0.im, r1=a1.re, i1=a1.im;\
    BF(t3, t5, t5, t1);\
    BF(a2.re, a0.re, r0, t5);\
    BF(a3.im, a1.im, i1, t3);\
    BF(t4, t6, t2, t6);\
    BF(a3.re, a1.re, r1, t4);\
    BF(a2.im, a0.im, i0, t6);\
}

#define TRANSFORM(a0,a1,a2,a3,wre,wim) {\
    CMUL(t1, t2, a2.re, a2.im, wre, -wim);\
    CMUL(t5, t6, a3.re, a3.im, wre,  wim);\
    BUTTERFLIES(a0,a1,a2,a3)\
}

#define TRANSFORM_ZERO(a0,a1,a2,a3) {\
    t1 = a2.re;\
    t2 = a2.im;\
    t5 = a3.re;\
    t6 = a3.im;\
    BUTTERFLIES(a0,a1,a2,a3)\
}

/* z[0...8n-1], w[1...2n-1] */
#define PASS(name)\
static void name(FFTComplex *z, const FFTSample *wre, unsigned int n)\
{\
    FFTDouble t1, t2, t3, t4, t5, t6;\
    int o1 = 2*n;\
    int o2 = 4*n;\
    int o3 = 6*n;\
    const FFTSample *wim = wre+o1;\
    n--;\
\
    TRANSFORM_ZERO(z[0],z[o1],z[o2],z[o3]);\
    TRANSFORM(z[1],z[o1+1],z[o2+1],z[o3+1],wre[1],wim[-1]);\
    do {\
        z += 2;\
        wre += 2;\
        wim -= 2;\
        TRANSFORM(z[0],z[o1],z[o2],z[o3],wre[0],wim[0]);\
        TRANSFORM(z[1],z[o1+1],z[o2+1],z[o3+1],wre[1],wim[-1]);\
    } while(--n);\
}

PASS(pass)
#undef BUTTERFLIES
#define BUTTERFLIES BUTTERFLIES_BIG
PASS(pass_big)

#define DECL_FFT(n,n2,n4)\
static void fft##n(FFTComplex *z)\
{\
    fft##n2(z);\
    fft##n4(z+n4*2);\
    fft##n4(z+n4*3);\
    pass(z,av_cos_##n,n4/2);\
}

static void fft4(FFTComplex *z)
{
    FFTDouble t1, t2, t3, t4, t5, t6, t7, t8;

    BF(t3, t1, z[0].re, z[1].re);
    BF(t8, t6, z[3].re, z[2].re);
    BF(z[2].re, z[0].re, t1, t6);
    BF(t4, t2, z[0].im, z[1].im);
    BF(t7, t5, z[2].im, z[3].im);
    BF(z[3].im, z[1].im, t4, t8);
    BF(z[3].re, z[1].re, t3, t7);
    BF(z[2].im, z[0].im, t2, t5);
}

static void fft8(FFTComplex *z)
{
    FFTDouble t1, t2, t3, t4, t5, t6;

    fft4(z);

    BF(t1, z[5].re, z[4].re, -z[5].re);
    BF(t2, z[5].im, z[4].im, -z[5].im);
    BF(t5, z[7].re, z[6].re, -z[7].re);
    BF(t6, z[7].im, z[6].im, -z[7].im);

    BUTTERFLIES(z[0],z[2],z[4],z[6]);
    TRANSFORM(z[1],z[3],z[5],z[7],sqrthalf,sqrthalf);
}

static void fft16(FFTComplex *z)
{
    FFTDouble t1, t2, t3, t4, t5, t6;
    FFTSample cos_16_1 = av_cos_16[1];
    FFTSample cos_16_3 = av_cos_16[3];

    fft8(z);
    fft4(z+8);
    fft4(z+12);

    TRANSFORM_ZERO(z[0],z[4],z[8],z[12]);
    TRANSFORM(z[2],z[6],z[10],z[14],sqrthalf,sqrthalf);
    TRANSFORM(z[1],z[5],z[9],z[13],cos_16_1,cos_16_3);
    TRANSFORM(z[3],z[7],z[11],z[15],cos_16_3,cos_16_1);
}
DECL_FFT(32,16,8)
DECL_FFT(64,32,16)
DECL_FFT(128,64,32)
DECL_FFT(256,128,64)
DECL_FFT(512,256,128)
#define pass pass_big
DECL_FFT(1024,512,256)

static void (* const fft_dispatch[])(FFTComplex*) = {
    fft4, fft8, fft16, fft32, fft64, fft128, fft256, fft512, fft1024,
};

void fft_calc(FFTContext *s, FFTComplex *z) {
    fft_dispatch[s->nbits-2](z);
}



/**
 * init MDCT or IMDCT computation.
 */
int ff_mdct_init(FFTContext *s, int nbits, int inverse, double scale)
{
	int n, n4, i;
	double alpha, theta;
	int tstep;

	memset(s, 0, sizeof(*s));
	n = 1 << nbits;
	s->mdct_bits = nbits;
	s->mdct_size = n;
	n4 = n >> 2;
	s->mdct_permutation = FF_MDCT_PERM_NONE;

	if (ff_fft_init(s, s->mdct_bits - 2, inverse) < 0)
		goto fail;

	s->tcos = (FFTSample *)av_malloc_array(n / 2, sizeof(FFTSample));
	if (!s->tcos)
		goto fail;

	switch (s->mdct_permutation) {
	case FF_MDCT_PERM_NONE:
		s->tsin = s->tcos + n4;
		tstep = 1;
		break;
	case FF_MDCT_PERM_INTERLEAVE:
		s->tsin = s->tcos + 1;
		tstep = 2;
		break;
	default:
		goto fail;
	}

	theta = 1.0f / 8.0f + (scale < 0 ? n4 : 0);
	scale = (FFTSample)sqrt(fabs(scale));
	for (i = 0; i < n4; i++) {
		alpha = 2 * M_PI * (i + theta) / n;
		s->tcos[i * tstep] = (FFTSample)(-cos(alpha) * scale);
		s->tsin[i * tstep] = (FFTSample)(-sin(alpha) * scale);
	}
	return 0;
fail:
	ff_mdct_end(s);
	return -1;
}

/**
 * Compute the middle half of the inverse MDCT of size N = 2^nbits,
 * thus excluding the parts that can be derived by symmetry
 * @param output N/2 samples
 * @param input N/2 samples
 */
void imdct_half(FFTContext *s, FFTSample *output, const FFTSample *input)
{
	int k, n8, n4, n2, n, j;
	const uint16_t *revtab = s->revtab;
	const FFTSample *tcos = s->tcos;
	const FFTSample *tsin = s->tsin;
	const FFTSample *in1, *in2;
	FFTComplex *z = (FFTComplex *)output;

	n = 1 << s->mdct_bits;
	n2 = n >> 1;
	n4 = n >> 2;
	n8 = n >> 3;

	/* pre rotation */
	in1 = input;
	in2 = input + n2 - 1;
	for (k = 0; k < n4; k++) {
		j = revtab[k];
		CMUL(z[j].re, z[j].im, *in2, *in1, tcos[k], tsin[k]);
		in1 += 2;
		in2 -= 2;
	}
	fft_calc(s, z);

	/* post rotation + reordering */
	for (k = 0; k < n8; k++) {
		FFTSample r0, i0, r1, i1;
		CMUL(r0, i1, z[n8 - k - 1].im, z[n8 - k - 1].re, tsin[n8 - k - 1], tcos[n8 - k - 1]);
		CMUL(r1, i0, z[n8 + k].im, z[n8 + k].re, tsin[n8 + k], tcos[n8 + k]);
		z[n8 - k - 1].re = r0;
		z[n8 - k - 1].im = i0;
		z[n8 + k].re = r1;
		z[n8 + k].im = i1;
	}
}

/**
 * Compute inverse MDCT of size N = 2^nbits
 * @param output N samples
 * @param input N/2 samples
 */
void imdct_calc(FFTContext *s, FFTSample *output, const FFTSample *input)
{
	int k;
	int n = 1 << s->mdct_bits;
	int n2 = n >> 1;
	int n4 = n >> 2;

	imdct_half(s, output + n4, input);

	for (k = 0; k < n4; k++) {
		output[k] = -output[n2 - k - 1];
		output[n - k - 1] = output[n2 + k];
	}
}

void ff_mdct_end(FFTContext *s)
{
	av_freep(&s->tcos);
	ff_fft_end(s);
}

/* ============================================================================
 * ATRAC Common Implementation (atrac.cpp)
 * ============================================================================ */
/*
 * common functions for the ATRAC family of decoders
 *
 * Copyright (c) 2006-2013 Maxim Poliakovski
 * Copyright (c) 2006-2008 Benjamin Larsson
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 */



float av_atrac_sf_table[64];
static float qmf_window[48];

static const float qmf_48tap_half[24] = {
   -0.00001461907f, -0.00009205479f,-0.000056157569f,0.00030117269f,
    0.0002422519f,  -0.00085293897f,-0.0005205574f,  0.0020340169f,
    0.00078333891f, -0.0042153862f, -0.00075614988f, 0.0078402944f,
   -0.000061169922f,-0.01344162f,    0.0024626821f,  0.021736089f,
   -0.007801671f,   -0.034090221f,   0.01880949f,    0.054326009f,
   -0.043596379f,   -0.099384367f,   0.13207909f,    0.46424159f
};

void ff_atrac_generate_tables(void)
{
    int i;
    float s;

    /* Generate scale factors */
    if (!av_atrac_sf_table[63])
        for (i=0 ; i<64 ; i++)
            av_atrac_sf_table[i] = (float)pow(2.0f, (i - 15) / 3.0f);

    /* Generate the QMF window. */
    if (!qmf_window[47])
        for (i=0 ; i<24; i++) {
            s = qmf_48tap_half[i] * 2.0f;
            qmf_window[i] = qmf_window[47 - i] = s;
        }
}

void ff_atrac_init_gain_compensation(AtracGCContext *gctx, int id2exp_offset,
                                             int loc_scale)
{
    int i;

    gctx->loc_scale     = loc_scale;
    gctx->loc_size      = 1 << loc_scale;
    gctx->id2exp_offset = id2exp_offset;

    /* Generate gain level table. */
    for (i = 0; i < 16; i++)
        gctx->gain_tab1[i] = powf(2.0f, (float)(id2exp_offset - i));

    /* Generate gain interpolation table. */
    for (i = -15; i < 16; i++)
        gctx->gain_tab2[i + 15] = powf(2.0f, -1.0f / gctx->loc_size * i);
}

void ff_atrac_gain_compensation(AtracGCContext *gctx, float *in, float *prev,
                                AtracGainInfo *gc_now, AtracGainInfo *gc_next,
                                int num_samples, float *out)
{
    float lev, gc_scale, gain_inc;
    int i, pos, lastpos;

    gc_scale = gc_next->num_points ? gctx->gain_tab1[gc_next->lev_code[0]]
                                   : 1.0f;

    if (!gc_now->num_points) {
        for (pos = 0; pos < num_samples; pos++)
            out[pos] = in[pos] * gc_scale + prev[pos];
    } else {
        pos = 0;

        for (i = 0; i < gc_now->num_points; i++) {
            lastpos = gc_now->loc_code[i] << gctx->loc_scale;

            lev = gctx->gain_tab1[gc_now->lev_code[i]];
            gain_inc = gctx->gain_tab2[(i + 1 < gc_now->num_points ? gc_now->lev_code[i + 1]
                                                                   : gctx->id2exp_offset) -
                                       gc_now->lev_code[i] + 15];

            /* apply constant gain level and overlap */
            for (; pos < lastpos; pos++)
                out[pos] = (in[pos] * gc_scale + prev[pos]) * lev;

            /* interpolate between two different gain levels */
            for (; pos < lastpos + gctx->loc_size; pos++) {
                out[pos] = (in[pos] * gc_scale + prev[pos]) * lev;
                lev *= gain_inc;
            }
        }

        for (; pos < num_samples; pos++)
            out[pos] = in[pos] * gc_scale + prev[pos];
    }

    /* copy the overlapping part into the delay buffer */
    memcpy(prev, &in[num_samples], num_samples * sizeof(float));
}

void ff_atrac_iqmf(float *inlo, float *inhi, unsigned int nIn, float *pOut,
                   float *delayBuf, float *temp)
{
    int   i, j;
    float   *p1, *p3;

    memcpy(temp, delayBuf, 46*sizeof(float));

    p3 = temp + 46;

    /* loop1 */
    for(i=0; i<(int)nIn; i+=2){
        p3[2*i+0] = inlo[i  ] + inhi[i  ];
        p3[2*i+1] = inlo[i  ] - inhi[i  ];
        p3[2*i+2] = inlo[i+1] + inhi[i+1];
        p3[2*i+3] = inlo[i+1] - inhi[i+1];
    }

    /* loop2 */
    p1 = temp;
    for (j = (int)nIn; j != 0; j--) {
        float s1 = 0.0f;
        float s2 = 0.0f;

        for (i = 0; i < 48; i += 2) {
            s1 += p1[i] * qmf_window[i];
            s2 += p1[i+1] * qmf_window[i+1];
        }

        pOut[0] = s2;
        pOut[1] = s1;

        p1 += 2;
        pOut += 2;
    }

    /* Update the delay buffer. */
    memcpy(delayBuf, temp + nIn*2, 46*sizeof(float));
}

/* ============================================================================
 * ATRAC3 Decoder (atrac3.cpp)
 * ============================================================================ */
/*
 * ATRAC3 compatible decoder
 * Copyright (c) 2006-2008 Maxim Poliakovski
 * Copyright (c) 2006-2008 Benjamin Larsson
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * ATRAC3 compatible decoder.
 * This decoder handles Sony's ATRAC3 data.
 *
 * Container formats used to store ATRAC3 data:
 * RealMedia (.rm), RIFF WAV (.wav, .at3), Sony OpenMG (.oma, .aa3).
 *
 * To use this decoder, a calling application must supply the extradata
 * bytes provided in the containers above.
 */
#define _USE_MATH_DEFINES




#define FFALIGN(x, a) (((x)+(a)-1)&~((a)-1))

#define JOINT_STEREO    0x12
#define STEREO          0x2

#define SAMPLES_PER_FRAME 1024
#define MDCT_SIZE          512

typedef struct GainBlock {
    AtracGainInfo g_block[4];
} GainBlock;

typedef struct TonalComponent {
    int pos;
    int num_coefs;
    float coef[8];
} TonalComponent;

typedef struct ChannelUnit {
    int            bands_coded;
    int            num_components;
    float          prev_frame[SAMPLES_PER_FRAME];
    int            gc_blk_switch;
    TonalComponent components[64];
    GainBlock      gain_block[2];

    DECLARE_ALIGNED(32, float, spectrum)[SAMPLES_PER_FRAME];
    DECLARE_ALIGNED(32, float, imdct_buf)[SAMPLES_PER_FRAME];

    float          delay_buf1[46]; ///<qmf delay buffers
    float          delay_buf2[46];
    float          delay_buf3[46];
} ChannelUnit;

typedef struct ATRAC3Context {
    GetBitContext gb;
    //@{
    /** stream data */
    int coding_mode;

    ChannelUnit *units;
    //@}
    //@{
    /** joint-stereo related variables */
    int matrix_coeff_index_prev[4];
    int matrix_coeff_index_now[4];
    int matrix_coeff_index_next[4];
    int weighting_delay[6];
    //@}
    //@{
    /** data buffers */
    uint8_t *decoded_bytes_buffer;
    float temp_buf[1070];
    //@}
    //@{
    /** extradata */
    int scrambled_stream;
    //@}

    AtracGCContext    gainc_ctx;
    FFTContext        mdct_ctx;

    int block_align;
    int channels;
} ATRAC3Context;

static DECLARE_ALIGNED(32, float, mdct_window)[MDCT_SIZE];
static VLC_TYPE atrac3_vlc_table[4096][2];
static VLC   spectral_coeff_tab[7];

/**
 * Regular 512 points IMDCT without overlapping, with the exception of the
 * swapping of odd bands caused by the reverse spectra of the QMF.
 *
 * @param odd_band  1 if the band is an odd band
 */
static void imlt(ATRAC3Context *q, float *input, float *output, int odd_band)
{
    int i;

    if (odd_band) {
        /**
         * Reverse the odd bands before IMDCT, this is an effect of the QMF
         * transform or it gives better compression to do it this way.
         * FIXME: It should be possible to handle this in imdct_calc
         * for that to happen a modification of the prerotation step of
         * all SIMD code and C code is needed.
         * Or fix the functions before so they generate a pre reversed spectrum.
         */
        for (i = 0; i < 128; i++)
            FFSWAP(float, input[i], input[255 - i]);
    }

    imdct_calc(&q->mdct_ctx, output, input);

    /* Perform windowing on the output. */
    vector_fmul(output, mdct_window, MDCT_SIZE);
}

/*
 * indata descrambling, only used for data coming from the rm container
 */
static int decode_bytes(const uint8_t *input, uint8_t *out, int bytes)
{
    int i, off;
    uint32_t c;
    const uint32_t *buf;
    uint32_t *output = (uint32_t *)out;

    off = (intptr_t)input & 3;
    buf = (const uint32_t *)(input - off);
    if (off)
        c = av_be2ne32((0x537F6103U >> (off * 8)) | (0x537F6103U << (32 - (off * 8))));
    else
        c = av_be2ne32(0x537F6103U);
    bytes += 3 + off;
    for (i = 0; i < bytes / 4; i++)
        output[i] = c ^ buf[i];

    //if (off)
    //    avpriv_request_sample(NULL, "Offset of %d", off);

    return off;
}

static void init_imdct_window(void)
{
    int i, j;

    /* generate the mdct window, for details see
     * http://wiki.multimedia.cx/index.php?title=RealAudio_atrc#Windows */
    for (i = 0, j = 255; i < 128; i++, j--) {
        float wi = (float)sin(((i + 0.5f) / 256.0f - 0.5f) * M_PI) + 1.0f;
        float wj = (float)sin(((j + 0.5f) / 256.0f - 0.5f) * M_PI) + 1.0f;
        float w  = 0.5f * (wi * wi + wj * wj);
        mdct_window[i] = mdct_window[511 - i] = wi / w;
        mdct_window[j] = mdct_window[511 - j] = wj / w;
    }
}

void atrac3_free(ATRAC3Context *ctx)
{
    av_freep(&ctx->units);
    av_freep(&ctx->decoded_bytes_buffer);

    ff_mdct_end(&ctx->mdct_ctx);
    av_freep(&ctx);
}

/**
 * Mantissa decoding
 *
 * @param selector     which table the output values are coded with
 * @param coding_flag  constant length coding or variable length coding
 * @param mantissas    mantissa output table
 * @param num_codes    number of values to get
 */
static void read_quant_spectral_coeffs(GetBitContext *gb, int selector,
                                       int coding_flag, int *mantissas,
                                       int num_codes)
{
    int i, code, huff_symb;

    if (selector == 1)
        num_codes /= 2;

    if (coding_flag != 0) {
        /* constant length coding (CLC) */
        int num_bits = clc_length_tab[selector];

        if (selector > 1) {
            for (i = 0; i < num_codes; i++) {
                if (num_bits)
                    code = get_sbits(gb, num_bits);
                else
                    code = 0;
                mantissas[i] = code;
            }
        } else {
            for (i = 0; i < num_codes; i++) {
                if (num_bits)
                    code = get_bits(gb, num_bits); // num_bits is always 4 in this case
                else
                    code = 0;
                mantissas[i * 2    ] = mantissa_clc_tab[code >> 2];
                mantissas[i * 2 + 1] = mantissa_clc_tab[code &  3];
            }
        }
    } else {
        /* variable length coding (VLC) */
        if (selector != 1) {
            for (i = 0; i < num_codes; i++) {
                huff_symb = get_vlc2(gb, spectral_coeff_tab[selector-1].table,
                                     spectral_coeff_tab[selector-1].bits, 3);
                huff_symb += 1;
                code = huff_symb >> 1;
                if (huff_symb & 1)
                    code = -code;
                mantissas[i] = code;
            }
        } else {
            for (i = 0; i < num_codes; i++) {
                huff_symb = get_vlc2(gb, spectral_coeff_tab[selector - 1].table,
                                     spectral_coeff_tab[selector - 1].bits, 3);
                mantissas[i * 2    ] = mantissa_vlc_tab[huff_symb * 2    ];
                mantissas[i * 2 + 1] = mantissa_vlc_tab[huff_symb * 2 + 1];
            }
        }
    }
}

/**
 * Restore the quantized band spectrum coefficients
 *
 * @return subband count, fix for broken specification/files
 */
static int atrac3_decode_spectrum(GetBitContext *gb, float *output)
{
    int num_subbands, coding_mode, i, j, first, last, subband_size;
    int subband_vlc_index[32], sf_index[32];
    int mantissas[128];
    float scale_factor;

    num_subbands = get_bits(gb, 5);  // number of coded subbands
    coding_mode  = get_bits1(gb);    // coding Mode: 0 - VLC/ 1-CLC

    /* get the VLC selector table for the subbands, 0 means not coded */
    for (i = 0; i <= num_subbands; i++)
        subband_vlc_index[i] = get_bits(gb, 3);

    /* read the scale factor indexes from the stream */
    for (i = 0; i <= num_subbands; i++) {
        if (subband_vlc_index[i] != 0)
            sf_index[i] = get_bits(gb, 6);
    }

    for (i = 0; i <= num_subbands; i++) {
        first = subband_tab[i    ];
        last  = subband_tab[i + 1];

        subband_size = last - first;

        if (subband_vlc_index[i] != 0) {
            /* decode spectral coefficients for this subband */
            /* TODO: This can be done faster is several blocks share the
             * same VLC selector (subband_vlc_index) */
            read_quant_spectral_coeffs(gb, subband_vlc_index[i], coding_mode,
                                       mantissas, subband_size);

            /* decode the scale factor for this subband */
            scale_factor = av_atrac_sf_table[sf_index[i]] *
                           inv_max_quant[subband_vlc_index[i]];

            /* inverse quantize the coefficients */
            for (j = 0; first < last; first++, j++)
                output[first] = mantissas[j] * scale_factor;
        } else {
            /* this subband was not coded, so zero the entire subband */
            memset(output + first, 0, subband_size * sizeof(*output));
        }
    }

    /* clear the subbands that were not coded */
    first = subband_tab[i];
    memset(output + first, 0, (SAMPLES_PER_FRAME - first) * sizeof(*output));
    return num_subbands;
}

/**
 * Restore the quantized tonal components
 *
 * @param components tonal components
 * @param num_bands  number of coded bands
 */
static int decode_tonal_components(GetBitContext *gb,
                                   TonalComponent *components, int num_bands)
{
    int i, b, c, m;
    int nb_components, coding_mode_selector, coding_mode;
    int band_flags[4], mantissa[8];
    int component_count = 0;

    nb_components = get_bits(gb, 5);

    /* no tonal components */
    if (nb_components == 0)
        return 0;

    coding_mode_selector = get_bits(gb, 2);
    if (coding_mode_selector == 2)
        return AVERROR_INVALIDDATA;

    coding_mode = coding_mode_selector & 1;

    for (i = 0; i < nb_components; i++) {
        int coded_values_per_component, quant_step_index;

        for (b = 0; b <= num_bands; b++)
            band_flags[b] = get_bits1(gb);

        coded_values_per_component = get_bits(gb, 3);

        quant_step_index = get_bits(gb, 3);
        if (quant_step_index <= 1)
            return AVERROR_INVALIDDATA;

        if (coding_mode_selector == 3)
            coding_mode = get_bits1(gb);

        for (b = 0; b < (num_bands + 1) * 4; b++) {
            int coded_components;

            if (band_flags[b >> 2] == 0)
                continue;

            coded_components = get_bits(gb, 3);

            for (c = 0; c < coded_components; c++) {
                TonalComponent *cmp = &components[component_count];
                int sf_index, coded_values, max_coded_values;
                float scale_factor;

                sf_index = get_bits(gb, 6);
                if (component_count >= 64)
                    return AVERROR_INVALIDDATA;

                cmp->pos = b * 64 + get_bits(gb, 6);

                max_coded_values = SAMPLES_PER_FRAME - cmp->pos;
                coded_values     = coded_values_per_component + 1;
                coded_values     = FFMIN(max_coded_values, coded_values);

                scale_factor = av_atrac_sf_table[sf_index] *
                               inv_max_quant[quant_step_index];

                read_quant_spectral_coeffs(gb, quant_step_index, coding_mode,
                                           mantissa, coded_values);

                cmp->num_coefs = coded_values;

                /* inverse quant */
                for (m = 0; m < coded_values; m++)
                    cmp->coef[m] = mantissa[m] * scale_factor;

                component_count++;
            }
        }
    }

    return component_count;
}

/**
 * Decode gain parameters for the coded bands
 *
 * @param block      the gainblock for the current band
 * @param num_bands  amount of coded bands
 */
static int decode_gain_control(GetBitContext *gb, GainBlock *block,
                               int num_bands)
{
    int b, j;
    int *level, *loc;

    AtracGainInfo *gain = block->g_block;

    for (b = 0; b <= num_bands; b++) {
        gain[b].num_points = get_bits(gb, 3);
        level              = gain[b].lev_code;
        loc                = gain[b].loc_code;

        for (j = 0; j < gain[b].num_points; j++) {
            level[j] = get_bits(gb, 4);
            loc[j]   = get_bits(gb, 5);
            if (j && loc[j] <= loc[j - 1])
                return AVERROR_INVALIDDATA;
        }
    }

    /* Clear the unused blocks. */
    for (; b < 4 ; b++)
        gain[b].num_points = 0;

    return 0;
}

/**
 * Combine the tonal band spectrum and regular band spectrum
 *
 * @param spectrum        output spectrum buffer
 * @param num_components  number of tonal components
 * @param components      tonal components for this band
 * @return                position of the last tonal coefficient
 */
static int add_tonal_components(float *spectrum, int num_components,
                                TonalComponent *components)
{
    int i, j, last_pos = -1;
    float *input, *output;

    for (i = 0; i < num_components; i++) {
        last_pos = FFMAX(components[i].pos + components[i].num_coefs, last_pos);
        input    = components[i].coef;
        output   = &spectrum[components[i].pos];

        for (j = 0; j < components[i].num_coefs; j++)
            output[j] += input[j];
    }

    return last_pos;
}

#define INTERPOLATE(old, new, nsample) \
    ((old) + (nsample) * 0.125f * ((new) - (old)))

static void reverse_matrixing(float *su1, float *su2, int *prev_code,
                              int *curr_code)
{
    int i, nsample, band;
    float mc1_l, mc1_r, mc2_l, mc2_r;

    for (i = 0, band = 0; band < 4 * 256; band += 256, i++) {
        int s1 = prev_code[i];
        int s2 = curr_code[i];
        nsample = band;

        if (s1 != s2) {
            /* Selector value changed, interpolation needed. */
            mc1_l = matrix_coeffs[s1 * 2    ];
            mc1_r = matrix_coeffs[s1 * 2 + 1];
            mc2_l = matrix_coeffs[s2 * 2    ];
            mc2_r = matrix_coeffs[s2 * 2 + 1];

            /* Interpolation is done over the first eight samples. */
            for (; nsample < band + 8; nsample++) {
                float c1 = su1[nsample];
                float c2 = su2[nsample];
                c2 = c1 * INTERPOLATE(mc1_l, mc2_l, nsample - band) +
                     c2 * INTERPOLATE(mc1_r, mc2_r, nsample - band);
                su1[nsample] = c2;
                su2[nsample] = c1 * 2.0f - c2;
            }
        }

        /* Apply the matrix without interpolation. */
        switch (s2) {
        case 0:     /* M/S decoding */
            for (; nsample < band + 256; nsample++) {
                float c1 = su1[nsample];
                float c2 = su2[nsample];
                su1[nsample] =  c2       * 2.0f;
                su2[nsample] = (c1 - c2) * 2.0f;
            }
            break;
        case 1:
            for (; nsample < band + 256; nsample++) {
                float c1 = su1[nsample];
                float c2 = su2[nsample];
                su1[nsample] = (c1 + c2) *  2.0f;
                su2[nsample] =  c2       * -2.0f;
            }
            break;
        case 2:
        case 3:
            for (; nsample < band + 256; nsample++) {
                float c1 = su1[nsample];
                float c2 = su2[nsample];
                su1[nsample] = c1 + c2;
                su2[nsample] = c1 - c2;
            }
            break;
        default:
            av_assert1(0);
        }
    }
}

static void get_channel_weights(int index, int flag, float ch[2])
{
    if (index == 7) {
        ch[0] = 1.0f;
        ch[1] = 1.0f;
    } else {
        ch[0] = (index & 7) / 7.0f;
        ch[1] = (float)sqrt(2 - ch[0] * ch[0]);
        if (flag)
            FFSWAP(float, ch[0], ch[1]);
    }
}

static void channel_weighting(float *su1, float *su2, int *p3)
{
    int band, nsample;
    /* w[x][y] y=0 is left y=1 is right */
    float w[2][2];

    if (p3[1] != 7 || p3[3] != 7) {
        get_channel_weights(p3[1], p3[0], w[0]);
        get_channel_weights(p3[3], p3[2], w[1]);

        for (band = 256; band < 4 * 256; band += 256) {
            for (nsample = band; nsample < band + 8; nsample++) {
                su1[nsample] *= INTERPOLATE(w[0][0], w[0][1], nsample - band);
                su2[nsample] *= INTERPOLATE(w[1][0], w[1][1], nsample - band);
            }
            for(; nsample < band + 256; nsample++) {
                su1[nsample] *= w[1][0];
                su2[nsample] *= w[1][1];
            }
        }
    }
}

/**
 * Decode a Sound Unit
 *
 * @param snd           the channel unit to be used
 * @param output        the decoded samples before IQMF in float representation
 * @param channel_num   channel number
 * @param coding_mode   the coding mode (JOINT_STEREO or regular stereo/mono)
 */
static int decode_channel_sound_unit(ATRAC3Context *q, GetBitContext *gb,
                                     ChannelUnit *snd, float *output,
                                     int channel_num, int coding_mode)
{
    int band, ret, num_subbands, last_tonal, num_bands;
    GainBlock *gain1 = &snd->gain_block[    snd->gc_blk_switch];
    GainBlock *gain2 = &snd->gain_block[1 - snd->gc_blk_switch];

    if (coding_mode == JOINT_STEREO && channel_num == 1) {
        int bits = get_bits(gb, 2);
        if (bits != 3) {
            av_log(AV_LOG_ERROR,"Joint Stereo mono Sound Unit id %d != 3.", bits);
            return AVERROR_INVALIDDATA;
        }
    } else {
        int bits = get_bits(gb, 6);
        if (bits != 0x28) {
            av_log(AV_LOG_ERROR, "Sound Unit id %02x != 0x28.", bits);
            return AVERROR_INVALIDDATA;
        }
    }

    /* number of coded QMF bands */
    snd->bands_coded = get_bits(gb, 2);

    ret = decode_gain_control(gb, gain2, snd->bands_coded);
    if (ret)
        return ret;

    snd->num_components = decode_tonal_components(gb, snd->components,
                                                  snd->bands_coded);
    if (snd->num_components < 0)
        return snd->num_components;

    num_subbands = atrac3_decode_spectrum(gb, snd->spectrum);

    /* Merge the decoded spectrum and tonal components. */
    last_tonal = add_tonal_components(snd->spectrum, snd->num_components,
                                      snd->components);


    /* calculate number of used MLT/QMF bands according to the amount of coded
       spectral lines */
    num_bands = (subband_tab[num_subbands] - 1) >> 8;
    if (last_tonal >= 0)
        num_bands = FFMAX((last_tonal + 256) >> 8, num_bands);


    /* Reconstruct time domain samples. */
    for (band = 0; band < 4; band++) {
        /* Perform the IMDCT step without overlapping. */
        if (band <= num_bands)
            imlt(q, &snd->spectrum[band * 256], snd->imdct_buf, band & 1);
        else
            memset(snd->imdct_buf, 0, 512 * sizeof(*snd->imdct_buf));

        /* gain compensation and overlapping */
        ff_atrac_gain_compensation(&q->gainc_ctx, snd->imdct_buf,
                                   &snd->prev_frame[band * 256],
                                   &gain1->g_block[band], &gain2->g_block[band],
                                   256, &output[band * 256]);
    }

    /* Swap the gain control buffers for the next frame. */
    snd->gc_blk_switch ^= 1;

    return 0;
}

static int decode_frame(ATRAC3Context *q, int block_align, int channels, const uint8_t *databuf,
                        float **out_samples)
{
    int ret, i;
    uint8_t *ptr1;

    if (q->coding_mode == JOINT_STEREO) {
        /* channel coupling mode */
        /* decode Sound Unit 1 */
        init_get_bits(&q->gb, databuf, block_align * 8);

        ret = decode_channel_sound_unit(q, &q->gb, q->units, out_samples[0], 0,
                                        JOINT_STEREO);
        if (ret != 0)
            return ret;

        /* Framedata of the su2 in the joint-stereo mode is encoded in
         * reverse byte order so we need to swap it first. */
        if (databuf == q->decoded_bytes_buffer) {
            uint8_t *ptr2 = q->decoded_bytes_buffer + block_align - 1;
            ptr1          = q->decoded_bytes_buffer;
            for (i = 0; i < block_align / 2; i++, ptr1++, ptr2--)
                FFSWAP(uint8_t, *ptr1, *ptr2);
        } else {
            const uint8_t *ptr2 = databuf + block_align - 1;
            for (i = 0; i < block_align; i++)
                q->decoded_bytes_buffer[i] = *ptr2--;
        }

        /* Skip the sync codes (0xF8). */
        ptr1 = q->decoded_bytes_buffer;
        for (i = 4; *ptr1 == 0xF8; i++, ptr1++) {
            if (i >= block_align)
                return AVERROR_INVALIDDATA;
        }


        /* set the bitstream reader at the start of the second Sound Unit*/
        init_get_bits8(&q->gb, ptr1, (int)(q->decoded_bytes_buffer + block_align - ptr1));

        /* Fill the Weighting coeffs delay buffer */
        memmove(q->weighting_delay, &q->weighting_delay[2],
                4 * sizeof(*q->weighting_delay));
        q->weighting_delay[4] = get_bits1(&q->gb);
        q->weighting_delay[5] = get_bits(&q->gb, 3);

        for (i = 0; i < 4; i++) {
            q->matrix_coeff_index_prev[i] = q->matrix_coeff_index_now[i];
            q->matrix_coeff_index_now[i]  = q->matrix_coeff_index_next[i];
            q->matrix_coeff_index_next[i] = get_bits(&q->gb, 2);
        }

        /* Decode Sound Unit 2. */
        ret = decode_channel_sound_unit(q, &q->gb, &q->units[1],
                                        out_samples[1], 1, JOINT_STEREO);
        if (ret != 0)
            return ret;

        /* Reconstruct the channel coefficients. */
        reverse_matrixing(out_samples[0], out_samples[1],
                          q->matrix_coeff_index_prev,
                          q->matrix_coeff_index_now);

        channel_weighting(out_samples[0], out_samples[1], q->weighting_delay);
    } else {
        /* normal stereo mode or mono */
        /* Decode the channel sound units. */
        for (i = 0; i < channels; i++) {
            /* Set the bitstream reader at the start of a channel sound unit. */
            init_get_bits(&q->gb,
                          databuf + i * block_align / channels,
                          block_align * 8 / channels);

            ret = decode_channel_sound_unit(q, &q->gb, &q->units[i],
                                            out_samples[i], i, q->coding_mode);
            if (ret != 0)
                return ret;
        }
    }

    /* Apply the iQMF synthesis filter. */
    for (i = 0; i < channels; i++) {
        float *p1 = out_samples[i];
        float *p2 = p1 + 256;
        float *p3 = p2 + 256;
        float *p4 = p3 + 256;
        ff_atrac_iqmf(p1, p2, 256, p1, q->units[i].delay_buf1, q->temp_buf);
        ff_atrac_iqmf(p4, p3, 256, p3, q->units[i].delay_buf2, q->temp_buf);
        ff_atrac_iqmf(p1, p3, 512, p1, q->units[i].delay_buf3, q->temp_buf);
    }

    return 0;
}

int atrac3_decode_frame(ATRAC3Context *ctx, float *out_data[2], int *nb_samples, const uint8_t *buf, int buf_size)
{
    int ret;
    const uint8_t *databuf;

	const int block_align = ctx->block_align;
	const int channels = ctx->channels;

    *nb_samples = 0;

    if (buf_size < block_align) {
        av_log(AV_LOG_ERROR,
               "Frame too small (%d bytes). Truncated file?", buf_size);
        return AVERROR_INVALIDDATA;
    }

    /* Check if we need to descramble and what buffer to pass on. */
    if (ctx->scrambled_stream) {
        decode_bytes(buf, ctx->decoded_bytes_buffer, block_align);
        databuf = ctx->decoded_bytes_buffer;
    } else {
        databuf = buf;
    }

    *nb_samples = SAMPLES_PER_FRAME;

    ret = decode_frame(ctx, block_align, channels, databuf, out_data);
    if (ret) {
        av_log(AV_LOG_ERROR, "Frame decoding error!");
        return ret;
    }

    return block_align;
}

void atrac3_flush_buffers(ATRAC3Context *c) {
	// There's no known correct way to do this, so let's just reset some stuff.
	memset(c->temp_buf, 0, sizeof(c->temp_buf));
}

static void atrac3_init_static_data(void)
{
    int i;

    init_imdct_window();
    ff_atrac_generate_tables();

    /* Initialize the VLC tables. */
    for (i = 0; i < 7; i++) {
        spectral_coeff_tab[i].table = &atrac3_vlc_table[atrac3_vlc_offs[i]];
        spectral_coeff_tab[i].table_allocated = atrac3_vlc_offs[i + 1] -
                                                atrac3_vlc_offs[i    ];
        init_vlc(&spectral_coeff_tab[i], 9, huff_tab_sizes[i],
                 huff_bits[i],  1, 1,
                 huff_codes[i], 1, 1, INIT_VLC_USE_NEW_STATIC);
    }
}

static int static_init_done;

ATRAC3Context *atrac3_alloc(int channels, int *block_align, const uint8_t *extra_data, int extra_data_size) {
    int i, ret;
    int version, delay, samples_per_frame, frame_factor;

    const uint8_t *edata_ptr = extra_data;

    if (channels <= 0 || channels > 2) {
        av_log(AV_LOG_ERROR, "Channel configuration error!");
        return NULL;
    }

    ATRAC3Context *q = (ATRAC3Context *)av_mallocz(sizeof(ATRAC3Context));
    q->channels = channels;
    if (*block_align) {
        q->block_align = *block_align;
    } else {
        // Atrac3 (unlike Atrac3+) requires a specified block align.
        atrac3_free(q);
        return NULL;
    }

    if (!static_init_done)
        atrac3_init_static_data();
    static_init_done = 1;

    /* Take care of the codec-specific extradata. */
    if (extra_data_size == 14) {
        /* Parse the extradata, WAV format */
        av_log(AV_LOG_DEBUG, "[0-1] %d",
               bytestream_get_le16(&edata_ptr));  // Unknown value always 1
        edata_ptr += 4;                             // samples per channel
        q->coding_mode = bytestream_get_le16(&edata_ptr);
        av_log(AV_LOG_DEBUG,"[8-9] %d",
               bytestream_get_le16(&edata_ptr));  //Dupe of coding mode
        frame_factor = bytestream_get_le16(&edata_ptr);  // Unknown always 1
        av_log(AV_LOG_DEBUG,"[12-13] %d",
               bytestream_get_le16(&edata_ptr));  // Unknown always 0

        /* setup */
        samples_per_frame    = SAMPLES_PER_FRAME * channels;
        version              = 4;
        delay                = 0x88E;
        q->coding_mode       = q->coding_mode ? JOINT_STEREO : STEREO;
        q->scrambled_stream  = 0;

        if (q->block_align !=  96 * channels * frame_factor &&
            q->block_align != 152 * channels * frame_factor &&
            q->block_align != 192 * channels * frame_factor) {
            av_log(AV_LOG_ERROR, "Unknown frame/channel/frame_factor "
                   "configuration %d/%d/%d", block_align,
                   channels, frame_factor);
            atrac3_free(q);
            return NULL;
        }
    } else if (extra_data_size == 12 || extra_data_size == 10) {
        /* Parse the extradata, RM format. */
        version                = bytestream_get_be32(&edata_ptr);
        samples_per_frame      = bytestream_get_be16(&edata_ptr);
        delay                  = bytestream_get_be16(&edata_ptr);
        q->coding_mode         = bytestream_get_be16(&edata_ptr);
        q->scrambled_stream    = 1;

    } else {
        av_log(AV_LOG_ERROR, "Unknown extradata size %d.",
               extra_data_size);
        atrac3_free(q);
        return NULL;
    }

    /* Check the extradata */

    if (version != 4) {
        av_log(AV_LOG_ERROR, "Version %d != 4.", version);
        atrac3_free(q);
        return NULL;
    }

    if (samples_per_frame != SAMPLES_PER_FRAME &&
        samples_per_frame != SAMPLES_PER_FRAME * 2) {
        av_log(AV_LOG_ERROR, "Unknown amount of samples per frame %d.",
               samples_per_frame);
         atrac3_free(q);
         return NULL;
	}

    if (delay != 0x88E) {
        av_log(AV_LOG_ERROR, "Unknown amount of delay %x != 0x88E.",
               delay);
        atrac3_free(q);
        return NULL;
	}

    if (q->coding_mode == STEREO)
        av_log(AV_LOG_DEBUG, "Normal stereo detected.");
    else if (q->coding_mode == JOINT_STEREO) {
        if (channels != 2) {
            av_log(AV_LOG_ERROR, "Invalid coding mode");
            atrac3_free(q);
            return NULL;
		}
        av_log(AV_LOG_DEBUG, "Joint stereo detected.");
    } else {
        av_log(AV_LOG_ERROR, "Unknown channel coding mode %x!",
               q->coding_mode);
        atrac3_free(q);
        return NULL;
	}

    q->decoded_bytes_buffer = (uint8_t *)av_mallocz(FFALIGN(q->block_align, 4) + AV_INPUT_BUFFER_PADDING_SIZE);

    /* initialize the MDCT transform */
    if ((ret = ff_mdct_init(&q->mdct_ctx, 9, 1, 1.0f / 32768)) < 0) {
        av_log(AV_LOG_ERROR, "Error initializing MDCT");
        av_freep(&q->decoded_bytes_buffer);

        return NULL;
    }

    /* init the joint-stereo decoding data */
    q->weighting_delay[0] = 0;
    q->weighting_delay[1] = 7;
    q->weighting_delay[2] = 0;
    q->weighting_delay[3] = 7;
    q->weighting_delay[4] = 0;
    q->weighting_delay[5] = 7;

    for (i = 0; i < 4; i++) {
        q->matrix_coeff_index_prev[i] = 3;
        q->matrix_coeff_index_now[i]  = 3;
        q->matrix_coeff_index_next[i] = 3;
    }

    ff_atrac_init_gain_compensation(&q->gainc_ctx, 4, 3);

    q->units = (ChannelUnit *)av_mallocz_array(channels, sizeof(*q->units));
    return q;
}

/* ============================================================================
 * ATRAC3+ Bitstream Parser (atrac3plus.cpp)
 * ============================================================================ */
/*
 * ATRAC3+ compatible decoder
 *
 * Copyright (c) 2010-2013 Maxim Poliakovski
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Bitstream parser for ATRAC3+ decoder.
 */



static VLC_TYPE tables_data[154276][2];
static VLC wl_vlc_tabs[4];
static VLC sf_vlc_tabs[8];
static VLC ct_vlc_tabs[4];
static VLC spec_vlc_tabs[112];
static VLC gain_vlc_tabs[11];
static VLC tone_vlc_tabs[7];

static const uint8_t ff_logg2_tab[256] = {
    0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

// todo: Replace with clz type instructions.
int av_log2(unsigned int v)
{
    int n = 0;
    if (v & 0xffff0000) {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += ff_logg2_tab[v];

    return n;
}

/**
 * Generate canonical VLC table from given descriptor.
 *
 * @param[in]     cb          ptr to codebook descriptor
 * @param[in]     xlat        ptr to translation table or NULL
 * @param[in,out] tab_offset  starting offset to the generated vlc table
 * @param[out]    out_vlc     ptr to vlc table to be generated
 */
static void build_canonical_huff(const uint8_t *cb, const uint8_t *xlat,
                                         int *tab_offset, VLC *out_vlc)
{
    int i, b;
    uint16_t codes[256];
    uint8_t bits[256];
    unsigned code = 0;
    int index = 0;
    int min_len = *cb++; // get shortest codeword length
    int max_len = *cb++; // get longest  codeword length

    for (b = min_len; b <= max_len; b++) {
        for (i = *cb++; i > 0; i--) {
            av_assert0(index < 256);
            bits[index]  = b;
            codes[index] = code++;
            index++;
        }
        code <<= 1;
    }

    out_vlc->table = &tables_data[*tab_offset];
    out_vlc->table_allocated = 1 << max_len;

    ff_init_vlc_sparse(out_vlc, max_len, index, bits, 1, 1, codes, 2, 2,
                       xlat, 1, 1, INIT_VLC_USE_NEW_STATIC);

    *tab_offset += 1 << max_len;
}

void ff_atrac3p_init_vlcs(void)
{
    int i, wl_vlc_offs, ct_vlc_offs, sf_vlc_offs, tab_offset;

    static const int wl_nb_bits[4]  = { 2, 3, 5, 5 };
    static const int wl_nb_codes[4] = { 3, 5, 8, 8 };
    static const uint8_t * const wl_bits[4] = {
        atrac3p_wl_huff_bits1, atrac3p_wl_huff_bits2,
        atrac3p_wl_huff_bits3, atrac3p_wl_huff_bits4
    };
    static const uint8_t * const wl_codes[4] = {
        atrac3p_wl_huff_code1, atrac3p_wl_huff_code2,
        atrac3p_wl_huff_code3, atrac3p_wl_huff_code4
    };
    static const uint8_t * const wl_xlats[4] = {
        atrac3p_wl_huff_xlat1, atrac3p_wl_huff_xlat2, NULL, NULL
    };

    static const int ct_nb_bits[4]  = { 3, 4, 4, 4 };
    static const int ct_nb_codes[4] = { 4, 8, 8, 8 };
    static const uint8_t * const ct_bits[4]  = {
        atrac3p_ct_huff_bits1, atrac3p_ct_huff_bits2,
        atrac3p_ct_huff_bits2, atrac3p_ct_huff_bits3
    };
    static const uint8_t * const ct_codes[4] = {
        atrac3p_ct_huff_code1, atrac3p_ct_huff_code2,
        atrac3p_ct_huff_code2, atrac3p_ct_huff_code3
    };
    static const uint8_t * const ct_xlats[4] = {
        NULL, NULL, atrac3p_ct_huff_xlat1, NULL
    };

    static const  int sf_nb_bits[8]  = {  9,  9,  9,  9,  6,  6,  7,  7 };
    static const  int sf_nb_codes[8] = { 64, 64, 64, 64, 16, 16, 16, 16 };
    static const uint8_t  * const sf_bits[8]  = {
        atrac3p_sf_huff_bits1, atrac3p_sf_huff_bits1, atrac3p_sf_huff_bits2,
        atrac3p_sf_huff_bits3, atrac3p_sf_huff_bits4, atrac3p_sf_huff_bits4,
        atrac3p_sf_huff_bits5, atrac3p_sf_huff_bits6
    };
    static const uint16_t * const sf_codes[8] = {
        atrac3p_sf_huff_code1, atrac3p_sf_huff_code1, atrac3p_sf_huff_code2,
        atrac3p_sf_huff_code3, atrac3p_sf_huff_code4, atrac3p_sf_huff_code4,
        atrac3p_sf_huff_code5, atrac3p_sf_huff_code6
    };
    static const uint8_t  * const sf_xlats[8] = {
        atrac3p_sf_huff_xlat1, atrac3p_sf_huff_xlat2, NULL, NULL,
        atrac3p_sf_huff_xlat4, atrac3p_sf_huff_xlat5, NULL, NULL
    };

    static const uint8_t * const gain_cbs[11] = {
        atrac3p_huff_gain_npoints1_cb, atrac3p_huff_gain_npoints1_cb,
        atrac3p_huff_gain_lev1_cb, atrac3p_huff_gain_lev2_cb,
        atrac3p_huff_gain_lev3_cb, atrac3p_huff_gain_lev4_cb,
        atrac3p_huff_gain_loc3_cb, atrac3p_huff_gain_loc1_cb,
        atrac3p_huff_gain_loc4_cb, atrac3p_huff_gain_loc2_cb,
        atrac3p_huff_gain_loc5_cb
    };
    static const uint8_t * const gain_xlats[11] = {
        NULL, atrac3p_huff_gain_npoints2_xlat, atrac3p_huff_gain_lev1_xlat,
        atrac3p_huff_gain_lev2_xlat, atrac3p_huff_gain_lev3_xlat,
        atrac3p_huff_gain_lev4_xlat, atrac3p_huff_gain_loc3_xlat,
        atrac3p_huff_gain_loc1_xlat, atrac3p_huff_gain_loc4_xlat,
        atrac3p_huff_gain_loc2_xlat, atrac3p_huff_gain_loc5_xlat
    };

    static const uint8_t * const tone_cbs[7] = {
        atrac3p_huff_tonebands_cb,  atrac3p_huff_numwavs1_cb,
        atrac3p_huff_numwavs2_cb,   atrac3p_huff_wav_ampsf1_cb,
        atrac3p_huff_wav_ampsf2_cb, atrac3p_huff_wav_ampsf3_cb,
        atrac3p_huff_freq_cb
    };
    static const uint8_t * const tone_xlats[7] = {
        NULL, NULL, atrac3p_huff_numwavs2_xlat, atrac3p_huff_wav_ampsf1_xlat,
        atrac3p_huff_wav_ampsf2_xlat, atrac3p_huff_wav_ampsf3_xlat,
        atrac3p_huff_freq_xlat
    };

    for (i = 0, wl_vlc_offs = 0, ct_vlc_offs = 2508; i < 4; i++) {
        wl_vlc_tabs[i].table = &tables_data[wl_vlc_offs];
        wl_vlc_tabs[i].table_allocated = 1 << wl_nb_bits[i];
        ct_vlc_tabs[i].table = &tables_data[ct_vlc_offs];
        ct_vlc_tabs[i].table_allocated = 1 << ct_nb_bits[i];

        ff_init_vlc_sparse(&wl_vlc_tabs[i], wl_nb_bits[i], wl_nb_codes[i],
                           wl_bits[i],  1, 1,
                           wl_codes[i], 1, 1,
                           wl_xlats[i], 1, 1,
                           INIT_VLC_USE_NEW_STATIC);

        ff_init_vlc_sparse(&ct_vlc_tabs[i], ct_nb_bits[i], ct_nb_codes[i],
                           ct_bits[i],  1, 1,
                           ct_codes[i], 1, 1,
                           ct_xlats[i], 1, 1,
                           INIT_VLC_USE_NEW_STATIC);

        wl_vlc_offs += wl_vlc_tabs[i].table_allocated;
        ct_vlc_offs += ct_vlc_tabs[i].table_allocated;
    }

    for (i = 0, sf_vlc_offs = 76; i < 8; i++) {
        sf_vlc_tabs[i].table = &tables_data[sf_vlc_offs];
        sf_vlc_tabs[i].table_allocated = 1 << sf_nb_bits[i];

        ff_init_vlc_sparse(&sf_vlc_tabs[i], sf_nb_bits[i], sf_nb_codes[i],
                           sf_bits[i],  1, 1,
                           sf_codes[i], 2, 2,
                           sf_xlats[i], 1, 1,
                           INIT_VLC_USE_NEW_STATIC);
        sf_vlc_offs += sf_vlc_tabs[i].table_allocated;
    }

    tab_offset = 2564;

    /* build huffman tables for spectrum decoding */
    for (i = 0; i < 112; i++) {
        if (atrac3p_spectra_tabs[i].cb)
            build_canonical_huff(atrac3p_spectra_tabs[i].cb,
                                 atrac3p_spectra_tabs[i].xlat,
                                 &tab_offset, &spec_vlc_tabs[i]);
        else
            spec_vlc_tabs[i].table = 0;
    }

    /* build huffman tables for gain data decoding */
    for (i = 0; i < 11; i++)
        build_canonical_huff(gain_cbs[i], gain_xlats[i], &tab_offset, &gain_vlc_tabs[i]);

    /* build huffman tables for tone decoding */
    for (i = 0; i < 7; i++)
        build_canonical_huff(tone_cbs[i], tone_xlats[i], &tab_offset, &tone_vlc_tabs[i]);
}

/**
 * Decode number of coded quantization units.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] chan          ptr to the channel parameters
 * @param[in,out] ctx           ptr to the channel unit context
 * @return result code: 0 = OK, otherwise - error code
 */
static int num_coded_units(GetBitContext *gb, Atrac3pChanParams *chan,
                           Atrac3pChanUnitCtx *ctx)
{
    chan->fill_mode = get_bits(gb, 2);
    if (!chan->fill_mode) {
        chan->num_coded_vals = ctx->num_quant_units;
    } else {
        chan->num_coded_vals = get_bits(gb, 5);
        if (chan->num_coded_vals > ctx->num_quant_units) {
            av_log(AV_LOG_ERROR,
                   "Invalid number of transmitted units!\n");
            return AVERROR_INVALIDDATA;
        }

        if (chan->fill_mode == 3)
            chan->split_point = get_bits(gb, 2) + (chan->ch_num << 1) + 1;
    }

    return 0;
}

/**
 * Add weighting coefficients to the decoded word-length information.
 *
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in,out] chan          ptr to the channel parameters
 * @param[in]     wtab_idx      index of the table of weights
 * @return result code: 0 = OK, otherwise - error code
 */
static int add_wordlen_weights(Atrac3pChanUnitCtx *ctx,
                               Atrac3pChanParams *chan, int wtab_idx)
{
    int i;
    const int8_t *weights_tab =
        &atrac3p_wl_weights[chan->ch_num * 3 + wtab_idx - 1][0];

    for (i = 0; i < ctx->num_quant_units; i++) {
        chan->qu_wordlen[i] += weights_tab[i];
        if (chan->qu_wordlen[i] < 0 || chan->qu_wordlen[i] > 7) {
            av_log(AV_LOG_ERROR,
                   "WL index out of range: pos=%d, val=%d!\n",
                   i, chan->qu_wordlen[i]);
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

/**
 * Subtract weighting coefficients from decoded scalefactors.
 *
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in,out] chan          ptr to the channel parameters
 * @param[in]     wtab_idx      index of table of weights
 * @return result code: 0 = OK, otherwise - error code
 */
static int subtract_sf_weights(Atrac3pChanUnitCtx *ctx,
                               Atrac3pChanParams *chan, int wtab_idx)
{
    int i;
    const int8_t *weights_tab = &atrac3p_sf_weights[wtab_idx - 1][0];

    for (i = 0; i < ctx->used_quant_units; i++) {
        chan->qu_sf_idx[i] -= weights_tab[i];
        if (chan->qu_sf_idx[i] < 0 || chan->qu_sf_idx[i] > 63) {
            av_log(AV_LOG_ERROR,
                   "SF index out of range: pos=%d, val=%d!\n",
                   i, chan->qu_sf_idx[i]);
            return AVERROR_INVALIDDATA;
        }
    }

    return 0;
}

/**
 * Unpack vector quantization tables.
 *
 * @param[in]    start_val    start value for the unpacked table
 * @param[in]    shape_vec    ptr to table to unpack
 * @param[out]   dst          ptr to output array
 * @param[in]    num_values   number of values to unpack
 */
static inline void unpack_vq_shape(int start_val, const int8_t *shape_vec,
                                   int *dst, int num_values)
{
    int i;

    if (num_values) {
        dst[0] = dst[1] = dst[2] = start_val;
        for (i = 3; i < num_values; i++)
            dst[i] = start_val - shape_vec[atrac3p_qu_num_to_seg[i] - 1];
    }
}

#define UNPACK_SF_VQ_SHAPE(gb, dst, num_vals)                            \
    start_val = get_bits((gb), 6);                                       \
    unpack_vq_shape(start_val, &atrac3p_sf_shapes[get_bits((gb), 6)][0], \
                    (dst), (num_vals))

/**
 * Decode word length for each quantization unit of a channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     ch_num        channel to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_channel_wordlen(GetBitContext *gb, Atrac3pChanUnitCtx *ctx, int ch_num)
{
    int i, weight_idx = 0, delta, diff, pos, delta_bits, min_val, flag,
        ret, start_val;
    VLC *vlc_tab;
    Atrac3pChanParams *chan     = &ctx->channels[ch_num];
    Atrac3pChanParams *ref_chan = &ctx->channels[0];

    chan->fill_mode = 0;

    switch (get_bits(gb, 2)) { /* switch according to coding mode */
    case 0: /* coded using constant number of bits */
        for (i = 0; i < ctx->num_quant_units; i++)
            chan->qu_wordlen[i] = get_bits(gb, 3);
        break;
    case 1:
        if (ch_num) {
            if ((ret = num_coded_units(gb, chan, ctx)) < 0)
                return ret;

            if (chan->num_coded_vals) {
                vlc_tab = &wl_vlc_tabs[get_bits(gb, 2)];

                for (i = 0; i < chan->num_coded_vals; i++) {
                    delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                    chan->qu_wordlen[i] = (ref_chan->qu_wordlen[i] + delta) & 7;
                }
            }
        } else {
            weight_idx = get_bits(gb, 2);
            if ((ret = num_coded_units(gb, chan, ctx)) < 0)
                return ret;

            if (chan->num_coded_vals) {
                pos = get_bits(gb, 5);
                if (pos > chan->num_coded_vals) {
                    av_log(AV_LOG_ERROR,
                           "WL mode 1: invalid position!\n");
                    return AVERROR_INVALIDDATA;
                }

                delta_bits = get_bits(gb, 2);
                min_val    = get_bits(gb, 3);

                for (i = 0; i < pos; i++)
                    chan->qu_wordlen[i] = get_bits(gb, 3);

                for (i = pos; i < chan->num_coded_vals; i++)
                    chan->qu_wordlen[i] = (min_val + get_bitsz(gb, delta_bits)) & 7;
            }
        }
        break;
    case 2:
        if ((ret = num_coded_units(gb, chan, ctx)) < 0)
            return ret;

        if (ch_num && chan->num_coded_vals) {
            vlc_tab = &wl_vlc_tabs[get_bits(gb, 2)];
            delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
            chan->qu_wordlen[0] = (ref_chan->qu_wordlen[0] + delta) & 7;

            for (i = 1; i < chan->num_coded_vals; i++) {
                diff = ref_chan->qu_wordlen[i] - ref_chan->qu_wordlen[i - 1];
                delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                chan->qu_wordlen[i] = (chan->qu_wordlen[i - 1] + diff + delta) & 7;
            }
        } else if (chan->num_coded_vals) {
            flag    = get_bits(gb, 1);
            vlc_tab = &wl_vlc_tabs[get_bits(gb, 1)];

            start_val = get_bits(gb, 3);
            unpack_vq_shape(start_val,
                            &atrac3p_wl_shapes[start_val][get_bits(gb, 4)][0],
                            chan->qu_wordlen, chan->num_coded_vals);

            if (!flag) {
                for (i = 0; i < chan->num_coded_vals; i++) {
                    delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                    chan->qu_wordlen[i] = (chan->qu_wordlen[i] + delta) & 7;
                }
            } else {
                for (i = 0; i < (chan->num_coded_vals & - 2); i += 2)
                    if (!get_bits1(gb)) {
                        chan->qu_wordlen[i]     = (chan->qu_wordlen[i] +
                                                   get_vlc2(gb, vlc_tab->table,
                                                            vlc_tab->bits, 1)) & 7;
                        chan->qu_wordlen[i + 1] = (chan->qu_wordlen[i + 1] +
                                                   get_vlc2(gb, vlc_tab->table,
                                                            vlc_tab->bits, 1)) & 7;
                    }

                if (chan->num_coded_vals & 1)
                    chan->qu_wordlen[i] = (chan->qu_wordlen[i] +
                                           get_vlc2(gb, vlc_tab->table,
                                                    vlc_tab->bits, 1)) & 7;
            }
        }
        break;
    case 3:
        weight_idx = get_bits(gb, 2);
        if ((ret = num_coded_units(gb, chan, ctx)) < 0)
            return ret;

        if (chan->num_coded_vals) {
            vlc_tab = &wl_vlc_tabs[get_bits(gb, 2)];

            /* first coefficient is coded directly */
            chan->qu_wordlen[0] = get_bits(gb, 3);

            for (i = 1; i < chan->num_coded_vals; i++) {
                delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                chan->qu_wordlen[i] = (chan->qu_wordlen[i - 1] + delta) & 7;
            }
        }
        break;
    }

    if (chan->fill_mode == 2) {
        for (i = chan->num_coded_vals; i < ctx->num_quant_units; i++)
            chan->qu_wordlen[i] = ch_num ? get_bits1(gb) : 1;
    } else if (chan->fill_mode == 3) {
        pos = ch_num ? chan->num_coded_vals + chan->split_point
                     : ctx->num_quant_units - chan->split_point;
        if (pos > FF_ARRAY_ELEMS(chan->qu_wordlen)) {
            av_log(AV_LOG_ERROR, "Split point beyond array");
            pos = FF_ARRAY_ELEMS(chan->qu_wordlen);
        }
        for (i = chan->num_coded_vals; i < pos; i++)
            chan->qu_wordlen[i] = 1;
    }

    if (weight_idx)
        return add_wordlen_weights(ctx, chan, weight_idx);

    return 0;
}

/**
 * Decode scale factor indexes for each quant unit of a channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     ch_num        channel to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_channel_sf_idx(GetBitContext *gb, Atrac3pChanUnitCtx *ctx, int ch_num)
{
    int i, weight_idx = 0, delta, diff, num_long_vals,
        delta_bits, min_val, vlc_sel, start_val;
    VLC *vlc_tab;
    Atrac3pChanParams *chan     = &ctx->channels[ch_num];
    Atrac3pChanParams *ref_chan = &ctx->channels[0];

    switch (get_bits(gb, 2)) { /* switch according to coding mode */
    case 0: /* coded using constant number of bits */
        for (i = 0; i < ctx->used_quant_units; i++)
            chan->qu_sf_idx[i] = get_bits(gb, 6);
        break;
    case 1:
        if (ch_num) {
            vlc_tab = &sf_vlc_tabs[get_bits(gb, 2)];

            for (i = 0; i < ctx->used_quant_units; i++) {
                delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                chan->qu_sf_idx[i] = (ref_chan->qu_sf_idx[i] + delta) & 0x3F;
            }
        } else {
            weight_idx = get_bits(gb, 2);
            if (weight_idx == 3) {
                UNPACK_SF_VQ_SHAPE(gb, chan->qu_sf_idx, ctx->used_quant_units);

                num_long_vals = get_bits(gb, 5);
                delta_bits    = get_bits(gb, 2);
                min_val       = get_bits(gb, 4) - 7;

                for (i = 0; i < num_long_vals; i++)
                    chan->qu_sf_idx[i] = (chan->qu_sf_idx[i] +
                                          get_bits(gb, 4) - 7) & 0x3F;

                /* all others are: min_val + delta */
                for (i = num_long_vals; i < ctx->used_quant_units; i++)
                    chan->qu_sf_idx[i] = (chan->qu_sf_idx[i] + min_val +
                                          get_bitsz(gb, delta_bits)) & 0x3F;
            } else {
                num_long_vals = get_bits(gb, 5);
                delta_bits    = get_bits(gb, 3);
                min_val       = get_bits(gb, 6);
                if (num_long_vals > ctx->used_quant_units || delta_bits == 7) {
                    av_log(AV_LOG_ERROR,
                           "SF mode 1: invalid parameters!\n");
                    return AVERROR_INVALIDDATA;
                }

                /* read full-precision SF indexes */
                for (i = 0; i < num_long_vals; i++)
                    chan->qu_sf_idx[i] = get_bits(gb, 6);

                /* all others are: min_val + delta */
                for (i = num_long_vals; i < ctx->used_quant_units; i++)
                    chan->qu_sf_idx[i] = (min_val +
                                          get_bitsz(gb, delta_bits)) & 0x3F;
            }
        }
        break;
    case 2:
        if (ch_num) {
            vlc_tab = &sf_vlc_tabs[get_bits(gb, 2)];

            delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
            chan->qu_sf_idx[0] = (ref_chan->qu_sf_idx[0] + delta) & 0x3F;

            for (i = 1; i < ctx->used_quant_units; i++) {
                diff  = ref_chan->qu_sf_idx[i] - ref_chan->qu_sf_idx[i - 1];
                delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                chan->qu_sf_idx[i] = (chan->qu_sf_idx[i - 1] + diff + delta) & 0x3F;
            }
        } else {
            vlc_tab = &sf_vlc_tabs[get_bits(gb, 2) + 4];

            UNPACK_SF_VQ_SHAPE(gb, chan->qu_sf_idx, ctx->used_quant_units);

            for (i = 0; i < ctx->used_quant_units; i++) {
                delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                chan->qu_sf_idx[i] = (chan->qu_sf_idx[i] +
                                      sign_extend(delta, 4)) & 0x3F;
            }
        }
        break;
    case 3:
        if (ch_num) {
            /* copy coefficients from reference channel */
            for (i = 0; i < ctx->used_quant_units; i++)
                chan->qu_sf_idx[i] = ref_chan->qu_sf_idx[i];
        } else {
            weight_idx = get_bits(gb, 2);
            vlc_sel    = get_bits(gb, 2);
            vlc_tab    = &sf_vlc_tabs[vlc_sel];

            if (weight_idx == 3) {
                vlc_tab = &sf_vlc_tabs[vlc_sel + 4];

                UNPACK_SF_VQ_SHAPE(gb, chan->qu_sf_idx, ctx->used_quant_units);

                diff               = (get_bits(gb, 4)    + 56)   & 0x3F;
                chan->qu_sf_idx[0] = (chan->qu_sf_idx[0] + diff) & 0x3F;

                for (i = 1; i < ctx->used_quant_units; i++) {
                    delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                    diff               = (diff + sign_extend(delta, 4)) & 0x3F;
                    chan->qu_sf_idx[i] = (diff + chan->qu_sf_idx[i])    & 0x3F;
                }
            } else {
                /* 1st coefficient is coded directly */
                chan->qu_sf_idx[0] = get_bits(gb, 6);

                for (i = 1; i < ctx->used_quant_units; i++) {
                    delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                    chan->qu_sf_idx[i] = (chan->qu_sf_idx[i - 1] + delta) & 0x3F;
                }
            }
        }
        break;
    }

    if (weight_idx && weight_idx < 3)
        return subtract_sf_weights(ctx, chan, weight_idx);

    return 0;
}

/**
 * Decode word length information for each channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_quant_wordlen(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                int num_channels)
{
    int ch_num, i, ret;

    for (ch_num = 0; ch_num < num_channels; ch_num++) {
        memset(ctx->channels[ch_num].qu_wordlen, 0,
               sizeof(ctx->channels[ch_num].qu_wordlen));

        if ((ret = decode_channel_wordlen(gb, ctx, ch_num)) < 0)
            return ret;
    }

    /* scan for last non-zero coeff in both channels and
     * set number of quant units having coded spectrum */
    for (i = ctx->num_quant_units - 1; i >= 0; i--)
        if (ctx->channels[0].qu_wordlen[i] ||
            (num_channels == 2 && ctx->channels[1].qu_wordlen[i]))
            break;
    ctx->used_quant_units = i + 1;

    return 0;
}

/**
 * Decode scale factor indexes for each channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_scale_factors(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                int num_channels)
{
    int ch_num, ret;

    if (!ctx->used_quant_units)
        return 0;

    for (ch_num = 0; ch_num < num_channels; ch_num++) {
        memset(ctx->channels[ch_num].qu_sf_idx, 0,
               sizeof(ctx->channels[ch_num].qu_sf_idx));

        if ((ret = decode_channel_sf_idx(gb, ctx, ch_num)) < 0)
            return ret;
    }

    return 0;
}

/**
 * Decode number of code table values.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @return result code: 0 = OK, otherwise - error code
 */
static int get_num_ct_values(GetBitContext *gb, Atrac3pChanUnitCtx *ctx)
{
    int num_coded_vals;

    if (get_bits1(gb)) {
        num_coded_vals = get_bits(gb, 5);
        if (num_coded_vals > ctx->used_quant_units) {
            av_log(AV_LOG_ERROR,
                   "Invalid number of code table indexes: %d!\n", num_coded_vals);
            return AVERROR_INVALIDDATA;
        }
        return num_coded_vals;
    } else
        return ctx->used_quant_units;
}

#define DEC_CT_IDX_COMMON(OP)                                           \
    num_vals = get_num_ct_values(gb, ctx);                       \
    if (num_vals < 0)                                                   \
        return num_vals;                                                \
                                                                        \
    for (i = 0; i < num_vals; i++) {                                    \
        if (chan->qu_wordlen[i]) {                                      \
            chan->qu_tab_idx[i] = OP;                                   \
        } else if (ch_num && ref_chan->qu_wordlen[i])                   \
            /* get clone master flag */                                 \
            chan->qu_tab_idx[i] = get_bits1(gb);                        \
    }

#define CODING_DIRECT get_bits(gb, num_bits)

#define CODING_VLC get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1)

#define CODING_VLC_DELTA                                                \
    (!i) ? CODING_VLC                                                   \
         : (pred + get_vlc2(gb, delta_vlc->table,                       \
                            delta_vlc->bits, 1)) & mask;                \
    pred = chan->qu_tab_idx[i]

#define CODING_VLC_DIFF                                                 \
    (ref_chan->qu_tab_idx[i] +                                          \
     get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1)) & mask

/**
 * Decode code table indexes for each quant unit of a channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     ch_num        channel to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_channel_code_tab(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   int ch_num)
{
    int i, num_vals, num_bits, pred;
    int mask = ctx->use_full_table ? 7 : 3; /* mask for modular arithmetic */
    VLC *vlc_tab, *delta_vlc;
    Atrac3pChanParams *chan     = &ctx->channels[ch_num];
    Atrac3pChanParams *ref_chan = &ctx->channels[0];

    chan->table_type = get_bits1(gb);

    switch (get_bits(gb, 2)) { /* switch according to coding mode */
    case 0: /* directly coded */
        num_bits = ctx->use_full_table + 2;
        DEC_CT_IDX_COMMON(CODING_DIRECT);
        break;
    case 1: /* entropy-coded */
        vlc_tab = ctx->use_full_table ? &ct_vlc_tabs[1]
                                      : ct_vlc_tabs;
        DEC_CT_IDX_COMMON(CODING_VLC);
        break;
    case 2: /* entropy-coded delta */
        if (ctx->use_full_table) {
            vlc_tab   = &ct_vlc_tabs[1];
            delta_vlc = &ct_vlc_tabs[2];
        } else {
            vlc_tab   = ct_vlc_tabs;
            delta_vlc = ct_vlc_tabs;
        }
        pred = 0;
        DEC_CT_IDX_COMMON(CODING_VLC_DELTA);
        break;
    case 3: /* entropy-coded difference to master */
        if (ch_num) {
            vlc_tab = ctx->use_full_table ? &ct_vlc_tabs[3]
                                          : ct_vlc_tabs;
            DEC_CT_IDX_COMMON(CODING_VLC_DIFF);
        }
        break;
    }

    return 0;
}

/**
 * Decode code table indexes for each channel.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_code_table_indexes(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                     int num_channels)
{
    int ch_num, ret;

    if (!ctx->used_quant_units)
        return 0;

    ctx->use_full_table = get_bits1(gb);

    for (ch_num = 0; ch_num < num_channels; ch_num++) {
        memset(ctx->channels[ch_num].qu_tab_idx, 0,
               sizeof(ctx->channels[ch_num].qu_tab_idx));

        if ((ret = decode_channel_code_tab(gb, ctx, ch_num)) < 0)
            return ret;
    }

    return 0;
}

/**
 * Decode huffman-coded spectral lines for a given quant unit.
 *
 * This is a generalized version for all known coding modes.
 * Its speed can be improved by creating separate functions for each mode.
 *
 * @param[in]   gb          the GetBit context
 * @param[in]   tab         code table telling how to decode spectral lines
 * @param[in]   vlc_tab     ptr to the huffman table associated with the code table
 * @param[out]  out         pointer to buffer where decoded data should be stored
 * @param[in]   num_specs   number of spectral lines to decode
 */
static void decode_qu_spectra(GetBitContext *gb, const Atrac3pSpecCodeTab *tab,
                              VLC *vlc_tab, int16_t *out, const int num_specs)
{
    int i, j, pos, cf;
    int group_size = tab->group_size;
    int num_coeffs = tab->num_coeffs;
    int bits       = tab->bits;
    int is_signed  = tab->is_signed;
    unsigned val;
    const unsigned bitmask = ((1 << bits) - 1);  // mask to clear higher bits.

    for (pos = 0; pos < num_specs;) {
        if (group_size == 1 || get_bits1(gb)) {
            for (j = 0; j < group_size; j++) {
                val = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
                for (i = 0; i < num_coeffs; i++) {
                    cf = val & bitmask;
                    if (is_signed)
                        cf = sign_extend(cf, bits);
                    else if (cf && get_bits1(gb))
                        cf = -cf;

                    out[pos++] = cf;
                    val >>= bits;
                }
            }
        } else /* group skipped */
            pos += group_size * num_coeffs;
    }
}

/**
 * Decode huffman-coded IMDCT spectrum for all channels.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 */
static void decode_spectrum(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                            int num_channels)
{
    int i, ch_num, qu, wordlen, codetab, tab_index, num_specs;
    const Atrac3pSpecCodeTab *tab;
    Atrac3pChanParams *chan;

    for (ch_num = 0; ch_num < num_channels; ch_num++) {
        chan = &ctx->channels[ch_num];

        memset(chan->spectrum, 0, sizeof(chan->spectrum));

        /* set power compensation level to disabled */
        memset(chan->power_levs, ATRAC3P_POWER_COMP_OFF, sizeof(chan->power_levs));

        for (qu = 0; qu < ctx->used_quant_units; qu++) {
            num_specs = av_atrac3p_qu_to_spec_pos[qu + 1] -
                        av_atrac3p_qu_to_spec_pos[qu];

            wordlen = chan->qu_wordlen[qu];
            codetab = chan->qu_tab_idx[qu];
            if (wordlen) {
                if (!ctx->use_full_table)
                    codetab = atrac3p_ct_restricted_to_full[chan->table_type][wordlen - 1][codetab];

                tab_index = (chan->table_type * 8 + codetab) * 7 + wordlen - 1;
                tab       = &atrac3p_spectra_tabs[tab_index];

                /* this allows reusing VLC tables */
                if (tab->redirect >= 0)
                    tab_index = tab->redirect;

                decode_qu_spectra(gb, tab, &spec_vlc_tabs[tab_index],
                                  &chan->spectrum[av_atrac3p_qu_to_spec_pos[qu]],
                                  num_specs);
            } else if (ch_num && ctx->channels[0].qu_wordlen[qu] && !codetab) {
                /* copy coefficients from master */
                memcpy(&chan->spectrum[av_atrac3p_qu_to_spec_pos[qu]],
                       &ctx->channels[0].spectrum[av_atrac3p_qu_to_spec_pos[qu]],
                       num_specs *
                       sizeof(chan->spectrum[av_atrac3p_qu_to_spec_pos[qu]]));
                chan->qu_wordlen[qu] = ctx->channels[0].qu_wordlen[qu];
            }
        }

        /* Power compensation levels only present in the bitstream
         * if there are more than 2 quant units. The lowest two units
         * correspond to the frequencies 0...351 Hz, whose shouldn't
         * be affected by the power compensation. */
        if (ctx->used_quant_units > 2) {
            num_specs = atrac3p_subband_to_num_powgrps[ctx->num_coded_subbands - 1];
            for (i = 0; i < num_specs; i++)
                chan->power_levs[i] = get_bits(gb, 4);
        }
    }
}

/**
 * Retrieve specified amount of flag bits from the input bitstream.
 * The data can be shortened in the case of the following two common conditions:
 * if all bits are zero then only one signal bit = 0 will be stored,
 * if all bits are ones then two signal bits = 1,0 will be stored.
 * Otherwise, all necessary bits will be directly stored
 * prefixed by two signal bits = 1,1.
 *
 * @param[in]   gb              ptr to the GetBitContext
 * @param[out]  out             where to place decoded flags
 * @param[in]   num_flags       number of flags to process
 * @return: 0 = all flag bits are zero, 1 = there is at least one non-zero flag bit
 */
static int get_subband_flags(GetBitContext *gb, uint8_t *out, int num_flags)
{
    int i, result;

    memset(out, 0, num_flags);

    result = get_bits1(gb);
    if (result) {
        if (get_bits1(gb))
            for (i = 0; i < num_flags; i++)
                out[i] = get_bits1(gb);
        else
            memset(out, 1, num_flags);
    }

    return result;
}

/**
 * Decode mdct window shape flags for all channels.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 */
static void decode_window_shape(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                int num_channels)
{
    int ch_num;

    for (ch_num = 0; ch_num < num_channels; ch_num++)
        get_subband_flags(gb, ctx->channels[ch_num].wnd_shape,
                          ctx->num_subbands);
}

/**
 * Decode number of gain control points.
 *
 * @param[in]     gb              the GetBit context
 * @param[in,out] ctx             ptr to the channel unit context
 * @param[in]     ch_num          channel to process
 * @param[in]     coded_subbands  number of subbands to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_gainc_npoints(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                int ch_num, int coded_subbands)
{
    int i, delta, delta_bits, min_val;
    Atrac3pChanParams *chan     = &ctx->channels[ch_num];
    Atrac3pChanParams *ref_chan = &ctx->channels[0];

    switch (get_bits(gb, 2)) { /* switch according to coding mode */
    case 0: /* fixed-length coding */
        for (i = 0; i < coded_subbands; i++)
            chan->gain_data[i].num_points = get_bits(gb, 3);
        break;
    case 1: /* variable-length coding */
        for (i = 0; i < coded_subbands; i++)
            chan->gain_data[i].num_points =
                get_vlc2(gb, gain_vlc_tabs[0].table,
                         gain_vlc_tabs[0].bits, 1);
        break;
    case 2:
        if (ch_num) { /* VLC modulo delta to master channel */
            for (i = 0; i < coded_subbands; i++) {
                delta = get_vlc2(gb, gain_vlc_tabs[1].table,
                                 gain_vlc_tabs[1].bits, 1);
                chan->gain_data[i].num_points =
                    (ref_chan->gain_data[i].num_points + delta) & 7;
            }
        } else { /* VLC modulo delta to previous */
            chan->gain_data[0].num_points =
                get_vlc2(gb, gain_vlc_tabs[0].table,
                         gain_vlc_tabs[0].bits, 1);

            for (i = 1; i < coded_subbands; i++) {
                delta = get_vlc2(gb, gain_vlc_tabs[1].table,
                                 gain_vlc_tabs[1].bits, 1);
                chan->gain_data[i].num_points =
                    (chan->gain_data[i - 1].num_points + delta) & 7;
            }
        }
        break;
    case 3:
        if (ch_num) { /* copy data from master channel */
            for (i = 0; i < coded_subbands; i++)
                chan->gain_data[i].num_points =
                    ref_chan->gain_data[i].num_points;
        } else { /* shorter delta to min */
            delta_bits = get_bits(gb, 2);
            min_val    = get_bits(gb, 3);

            for (i = 0; i < coded_subbands; i++) {
                chan->gain_data[i].num_points = min_val + get_bitsz(gb, delta_bits);
                if (chan->gain_data[i].num_points > 7)
                    return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

/**
 * Implements coding mode 3 (slave) for gain compensation levels.
 *
 * @param[out]   dst   ptr to the output array
 * @param[in]    ref   ptr to the reference channel
 */
static inline void gainc_level_mode3s(AtracGainInfo *dst, AtracGainInfo *ref)
{
    int i;

    for (i = 0; i < dst->num_points; i++)
        dst->lev_code[i] = (i >= ref->num_points) ? 7 : ref->lev_code[i];
}

/**
 * Implements coding mode 1 (master) for gain compensation levels.
 *
 * @param[in]     gb     the GetBit context
 * @param[in]     ctx    ptr to the channel unit context
 * @param[out]    dst    ptr to the output array
 */
static inline void gainc_level_mode1m(GetBitContext *gb,
                                      Atrac3pChanUnitCtx *ctx,
                                      AtracGainInfo *dst)
{
    int i, delta;

    if (dst->num_points > 0)
        dst->lev_code[0] = get_vlc2(gb, gain_vlc_tabs[2].table,
                                    gain_vlc_tabs[2].bits, 1);

    for (i = 1; i < dst->num_points; i++) {
        delta = get_vlc2(gb, gain_vlc_tabs[3].table,
                         gain_vlc_tabs[3].bits, 1);
        dst->lev_code[i] = (dst->lev_code[i - 1] + delta) & 0xF;
    }
}

/**
 * Decode level code for each gain control point.
 *
 * @param[in]     gb              the GetBit context
 * @param[in,out] ctx             ptr to the channel unit context
 * @param[in]     ch_num          channel to process
 * @param[in]     coded_subbands  number of subbands to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_gainc_levels(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                               int ch_num, int coded_subbands)
{
    int sb, i, delta, delta_bits, min_val, pred;
    Atrac3pChanParams *chan     = &ctx->channels[ch_num];
    Atrac3pChanParams *ref_chan = &ctx->channels[0];

    switch (get_bits(gb, 2)) { /* switch according to coding mode */
    case 0: /* fixed-length coding */
        for (sb = 0; sb < coded_subbands; sb++)
            for (i = 0; i < chan->gain_data[sb].num_points; i++)
                chan->gain_data[sb].lev_code[i] = get_bits(gb, 4);
        break;
    case 1:
        if (ch_num) { /* VLC modulo delta to master channel */
            for (sb = 0; sb < coded_subbands; sb++)
                for (i = 0; i < chan->gain_data[sb].num_points; i++) {
                    delta = get_vlc2(gb, gain_vlc_tabs[5].table,
                                     gain_vlc_tabs[5].bits, 1);
                    pred = (i >= ref_chan->gain_data[sb].num_points)
                           ? 7 : ref_chan->gain_data[sb].lev_code[i];
                    chan->gain_data[sb].lev_code[i] = (pred + delta) & 0xF;
                }
        } else { /* VLC modulo delta to previous */
            for (sb = 0; sb < coded_subbands; sb++)
                gainc_level_mode1m(gb, ctx, &chan->gain_data[sb]);
        }
        break;
    case 2:
        if (ch_num) { /* VLC modulo delta to previous or clone master */
            for (sb = 0; sb < coded_subbands; sb++)
                if (chan->gain_data[sb].num_points > 0) {
                    if (get_bits1(gb))
                        gainc_level_mode1m(gb, ctx, &chan->gain_data[sb]);
                    else
                        gainc_level_mode3s(&chan->gain_data[sb],
                                           &ref_chan->gain_data[sb]);
                }
        } else { /* VLC modulo delta to lev_codes of previous subband */
            if (chan->gain_data[0].num_points > 0)
                gainc_level_mode1m(gb, ctx, &chan->gain_data[0]);

            for (sb = 1; sb < coded_subbands; sb++)
                for (i = 0; i < chan->gain_data[sb].num_points; i++) {
                    delta = get_vlc2(gb, gain_vlc_tabs[4].table,
                                     gain_vlc_tabs[4].bits, 1);
                    pred = (i >= chan->gain_data[sb - 1].num_points)
                           ? 7 : chan->gain_data[sb - 1].lev_code[i];
                    chan->gain_data[sb].lev_code[i] = (pred + delta) & 0xF;
                }
        }
        break;
    case 3:
        if (ch_num) { /* clone master */
            for (sb = 0; sb < coded_subbands; sb++)
                gainc_level_mode3s(&chan->gain_data[sb],
                                   &ref_chan->gain_data[sb]);
        } else { /* shorter delta to min */
            delta_bits = get_bits(gb, 2);
            min_val    = get_bits(gb, 4);

            for (sb = 0; sb < coded_subbands; sb++)
                for (i = 0; i < chan->gain_data[sb].num_points; i++) {
                    chan->gain_data[sb].lev_code[i] = min_val + get_bitsz(gb, delta_bits);
                    if (chan->gain_data[sb].lev_code[i] > 15)
                        return AVERROR_INVALIDDATA;
                }
        }
        break;
    }

    return 0;
}

/**
 * Implements coding mode 0 for gain compensation locations.
 *
 * @param[in]     gb     the GetBit context
 * @param[in]     ctx    ptr to the channel unit context
 * @param[out]    dst    ptr to the output array
 * @param[in]     pos    position of the value to be processed
 */
static inline void gainc_loc_mode0(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   AtracGainInfo *dst, int pos)
{
    int delta_bits;

    if (!pos || dst->loc_code[pos - 1] < 15)
        dst->loc_code[pos] = get_bits(gb, 5);
    else if (dst->loc_code[pos - 1] >= 30)
        dst->loc_code[pos] = 31;
    else {
        delta_bits         = av_log2(30 - dst->loc_code[pos - 1]) + 1;
        dst->loc_code[pos] = dst->loc_code[pos - 1] +
                             get_bits(gb, delta_bits) + 1;
    }
}

/**
 * Implements coding mode 1 for gain compensation locations.
 *
 * @param[in]     gb     the GetBit context
 * @param[in]     ctx    ptr to the channel unit context
 * @param[out]    dst    ptr to the output array
 */
static inline void gainc_loc_mode1(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   AtracGainInfo *dst)
{
    int i;
    VLC *tab;

    if (dst->num_points > 0) {
        /* 1st coefficient is stored directly */
        dst->loc_code[0] = get_bits(gb, 5);

        for (i = 1; i < dst->num_points; i++) {
            /* switch VLC according to the curve direction
             * (ascending/descending) */
            tab              = (dst->lev_code[i] <= dst->lev_code[i - 1])
                               ? &gain_vlc_tabs[7]
                               : &gain_vlc_tabs[9];
            dst->loc_code[i] = dst->loc_code[i - 1] +
                               get_vlc2(gb, tab->table, tab->bits, 1);
        }
    }
}

/**
 * Decode location code for each gain control point.
 *
 * @param[in]     gb              the GetBit context
 * @param[in,out] ctx             ptr to the channel unit context
 * @param[in]     ch_num          channel to process
 * @param[in]     coded_subbands  number of subbands to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_gainc_loc_codes(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                  int ch_num, int coded_subbands)
{
    int sb, i, delta, delta_bits, min_val, pred, more_than_ref;
    AtracGainInfo *dst, *ref;
    VLC *tab;
    Atrac3pChanParams *chan     = &ctx->channels[ch_num];
    Atrac3pChanParams *ref_chan = &ctx->channels[0];

    switch (get_bits(gb, 2)) { /* switch according to coding mode */
    case 0: /* sequence of numbers in ascending order */
        for (sb = 0; sb < coded_subbands; sb++)
            for (i = 0; i < chan->gain_data[sb].num_points; i++)
                gainc_loc_mode0(gb, ctx, &chan->gain_data[sb], i);
        break;
    case 1:
        if (ch_num) {
            for (sb = 0; sb < coded_subbands; sb++) {
                if (chan->gain_data[sb].num_points <= 0)
                    continue;
                dst = &chan->gain_data[sb];
                ref = &ref_chan->gain_data[sb];

                /* 1st value is vlc-coded modulo delta to master */
                delta = get_vlc2(gb, gain_vlc_tabs[10].table,
                                 gain_vlc_tabs[10].bits, 1);
                pred = ref->num_points > 0 ? ref->loc_code[0] : 0;
                dst->loc_code[0] = (pred + delta) & 0x1F;

                for (i = 1; i < dst->num_points; i++) {
                    more_than_ref = i >= ref->num_points;
                    if (dst->lev_code[i] > dst->lev_code[i - 1]) {
                        /* ascending curve */
                        if (more_than_ref) {
                            delta =
                                get_vlc2(gb, gain_vlc_tabs[9].table,
                                         gain_vlc_tabs[9].bits, 1);
                            dst->loc_code[i] = dst->loc_code[i - 1] + delta;
                        } else {
                            if (get_bits1(gb))
                                gainc_loc_mode0(gb, ctx, dst, i);  // direct coding
                            else
                                dst->loc_code[i] = ref->loc_code[i];  // clone master
                        }
                    } else { /* descending curve */
                        tab   = more_than_ref ? &gain_vlc_tabs[7]
                                              : &gain_vlc_tabs[10];
                        delta = get_vlc2(gb, tab->table, tab->bits, 1);
                        if (more_than_ref)
                            dst->loc_code[i] = dst->loc_code[i - 1] + delta;
                        else
                            dst->loc_code[i] = (ref->loc_code[i] + delta) & 0x1F;
                    }
                }
            }
        } else /* VLC delta to previous */
            for (sb = 0; sb < coded_subbands; sb++)
                gainc_loc_mode1(gb, ctx, &chan->gain_data[sb]);
        break;
    case 2:
        if (ch_num) {
            for (sb = 0; sb < coded_subbands; sb++) {
                if (chan->gain_data[sb].num_points <= 0)
                    continue;
                dst = &chan->gain_data[sb];
                ref = &ref_chan->gain_data[sb];
                if (dst->num_points > ref->num_points || get_bits1(gb))
                    gainc_loc_mode1(gb, ctx, dst);
                else /* clone master for the whole subband */
                    for (i = 0; i < chan->gain_data[sb].num_points; i++)
                        dst->loc_code[i] = ref->loc_code[i];
            }
        } else {
            /* data for the first subband is coded directly */
            for (i = 0; i < chan->gain_data[0].num_points; i++)
                gainc_loc_mode0(gb, ctx, &chan->gain_data[0], i);

            for (sb = 1; sb < coded_subbands; sb++) {
                if (chan->gain_data[sb].num_points <= 0)
                    continue;
                dst = &chan->gain_data[sb];

                /* 1st value is vlc-coded modulo delta to the corresponding
                 * value of the previous subband if any or zero */
                delta = get_vlc2(gb, gain_vlc_tabs[6].table,
                                 gain_vlc_tabs[6].bits, 1);
                pred             = dst[-1].num_points > 0
                                   ? dst[-1].loc_code[0] : 0;
                dst->loc_code[0] = (pred + delta) & 0x1F;

                for (i = 1; i < dst->num_points; i++) {
                    more_than_ref = i >= dst[-1].num_points;
                    /* Select VLC table according to curve direction and
                     * presence of prediction. */
                    tab = &gain_vlc_tabs[(dst->lev_code[i] > dst->lev_code[i - 1]) *
                                                   2 + more_than_ref + 6];
                    delta = get_vlc2(gb, tab->table, tab->bits, 1);
                    if (more_than_ref)
                        dst->loc_code[i] = dst->loc_code[i - 1] + delta;
                    else
                        dst->loc_code[i] = (dst[-1].loc_code[i] + delta) & 0x1F;
                }
            }
        }
        break;
    case 3:
        if (ch_num) { /* clone master or direct or direct coding */
            for (sb = 0; sb < coded_subbands; sb++)
                for (i = 0; i < chan->gain_data[sb].num_points; i++) {
                    if (i >= ref_chan->gain_data[sb].num_points)
                        gainc_loc_mode0(gb, ctx, &chan->gain_data[sb], i);
                    else
                        chan->gain_data[sb].loc_code[i] =
                            ref_chan->gain_data[sb].loc_code[i];
                }
        } else { /* shorter delta to min */
            delta_bits = get_bits(gb, 2) + 1;
            min_val    = get_bits(gb, 5);

            for (sb = 0; sb < coded_subbands; sb++)
                for (i = 0; i < chan->gain_data[sb].num_points; i++)
                    chan->gain_data[sb].loc_code[i] = min_val + i +
                                                      get_bits(gb, delta_bits);
        }
        break;
    }

    /* Validate decoded information */
    for (sb = 0; sb < coded_subbands; sb++) {
        dst = &chan->gain_data[sb];
        for (i = 0; i < chan->gain_data[sb].num_points; i++) {
            if (dst->loc_code[i] < 0 || dst->loc_code[i] > 31 ||
                (i && dst->loc_code[i] <= dst->loc_code[i - 1])) {
                av_log(AV_LOG_ERROR,
                       "Invalid gain location: ch=%d, sb=%d, pos=%d, val=%d\n",
                       ch_num, sb, i, dst->loc_code[i]);
                return AVERROR_INVALIDDATA;
            }
        }
    }

    return 0;
}

/**
 * Decode gain control data for all channels.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_gainc_data(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                             int num_channels)
{
    int ch_num, coded_subbands, sb, ret;

    for (ch_num = 0; ch_num < num_channels; ch_num++) {
        memset(ctx->channels[ch_num].gain_data, 0,
               sizeof(*ctx->channels[ch_num].gain_data) * ATRAC3P_SUBBANDS);

        if (get_bits1(gb)) { /* gain control data present? */
            coded_subbands = get_bits(gb, 4) + 1;
            if (get_bits1(gb)) /* is high band gain data replication on? */
                ctx->channels[ch_num].num_gain_subbands = get_bits(gb, 4) + 1;
            else
                ctx->channels[ch_num].num_gain_subbands = coded_subbands;

            if ((ret = decode_gainc_npoints(gb, ctx, ch_num, coded_subbands)) < 0 ||
                (ret = decode_gainc_levels(gb, ctx, ch_num, coded_subbands))  < 0 ||
                (ret = decode_gainc_loc_codes(gb, ctx, ch_num, coded_subbands)) < 0)
                return ret;

            if (coded_subbands > 0) { /* propagate gain data if requested */
                for (sb = coded_subbands; sb < ctx->channels[ch_num].num_gain_subbands; sb++)
                    ctx->channels[ch_num].gain_data[sb] =
                        ctx->channels[ch_num].gain_data[sb - 1];
            }
        } else {
            ctx->channels[ch_num].num_gain_subbands = 0;
        }
    }

    return 0;
}

/**
 * Decode envelope for all tones of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 */
static void decode_tones_envelope(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                  int ch_num, int band_has_tones[])
{
    int sb;
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
    Atrac3pWavesData *ref = ctx->channels[0].tones_info;

    if (!ch_num || !get_bits1(gb)) { /* mode 0: fixed-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb])
                continue;
            dst[sb].pend_env.has_start_point = get_bits1(gb);
            dst[sb].pend_env.start_pos       = dst[sb].pend_env.has_start_point
                                               ? get_bits(gb, 5) : -1;
            dst[sb].pend_env.has_stop_point  = get_bits1(gb);
            dst[sb].pend_env.stop_pos        = dst[sb].pend_env.has_stop_point
                                               ? get_bits(gb, 5) : 32;
        }
    } else { /* mode 1(slave only): copy master */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb])
                continue;
            dst[sb].pend_env.has_start_point = ref[sb].pend_env.has_start_point;
            dst[sb].pend_env.has_stop_point  = ref[sb].pend_env.has_stop_point;
            dst[sb].pend_env.start_pos       = ref[sb].pend_env.start_pos;
            dst[sb].pend_env.stop_pos        = ref[sb].pend_env.stop_pos;
        }
    }
}

/**
 * Decode number of tones for each subband of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_band_numwavs(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                               int ch_num, int band_has_tones[])
{
    int mode, sb, delta;
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
    Atrac3pWavesData *ref = ctx->channels[0].tones_info;

    mode = get_bits(gb, ch_num + 1);
    switch (mode) {
    case 0: /** fixed-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
            if (band_has_tones[sb])
                dst[sb].num_wavs = get_bits(gb, 4);
        break;
    case 1: /** variable-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
            if (band_has_tones[sb])
                dst[sb].num_wavs =
                    get_vlc2(gb, tone_vlc_tabs[1].table,
                             tone_vlc_tabs[1].bits, 1);
        break;
    case 2: /** VLC modulo delta to master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
            if (band_has_tones[sb]) {
                delta = get_vlc2(gb, tone_vlc_tabs[2].table,
                                 tone_vlc_tabs[2].bits, 1);
                delta = sign_extend(delta, 3);
                dst[sb].num_wavs = (ref[sb].num_wavs + delta) & 0xF;
            }
        break;
    case 3: /** copy master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
            if (band_has_tones[sb])
                dst[sb].num_wavs = ref[sb].num_wavs;
        break;
    }

    /** initialize start tone index for each subband */
    for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
        if (band_has_tones[sb]) {
            if (ctx->waves_info->tones_index + dst[sb].num_wavs > 48) {
                av_log(AV_LOG_ERROR,
                       "Too many tones: %d (max. 48)!\n",
                       ctx->waves_info->tones_index + dst[sb].num_wavs);
                return AVERROR_INVALIDDATA;
            }
            dst[sb].start_index           = ctx->waves_info->tones_index;
            ctx->waves_info->tones_index += dst[sb].num_wavs;
        }

    return 0;
}

/**
 * Decode frequency information for each subband of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 */
static void decode_tones_frequency(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   int ch_num, int band_has_tones[])
{
    int sb, i, direction, nbits, pred, delta;
    Atrac3pWaveParam *iwav, *owav;
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
    Atrac3pWavesData *ref = ctx->channels[0].tones_info;

    if (!ch_num || !get_bits1(gb)) { /* mode 0: fixed-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            iwav      = &ctx->waves_info->waves[dst[sb].start_index];
            direction = (dst[sb].num_wavs > 1) ? get_bits1(gb) : 0;
            if (direction) { /** packed numbers in descending order */
                if (dst[sb].num_wavs)
                    iwav[dst[sb].num_wavs - 1].freq_index = get_bits(gb, 10);
                for (i = dst[sb].num_wavs - 2; i >= 0 ; i--) {
                    nbits = av_log2(iwav[i+1].freq_index) + 1;
                    iwav[i].freq_index = get_bits(gb, nbits);
                }
            } else { /** packed numbers in ascending order */
                for (i = 0; i < dst[sb].num_wavs; i++) {
                    if (!i || iwav[i - 1].freq_index < 512)
                        iwav[i].freq_index = get_bits(gb, 10);
                    else {
                        nbits = av_log2(1023 - iwav[i - 1].freq_index) + 1;
                        iwav[i].freq_index = get_bits(gb, nbits) +
                                             1024 - (1 << nbits);
                    }
                }
            }
        }
    } else { /* mode 1: VLC modulo delta to master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            iwav = &ctx->waves_info->waves[ref[sb].start_index];
            owav = &ctx->waves_info->waves[dst[sb].start_index];
            for (i = 0; i < dst[sb].num_wavs; i++) {
                delta = get_vlc2(gb, tone_vlc_tabs[6].table,
                                 tone_vlc_tabs[6].bits, 1);
                delta = sign_extend(delta, 8);
                pred  = (i < ref[sb].num_wavs) ? iwav[i].freq_index :
                        (ref[sb].num_wavs ? iwav[ref[sb].num_wavs - 1].freq_index : 0);
                owav[i].freq_index = (pred + delta) & 0x3FF;
            }
        }
    }
}

/**
 * Decode amplitude information for each subband of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 */
static void decode_tones_amplitude(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   int ch_num, int band_has_tones[])
{
    int mode, sb, j, i, diff, maxdiff, fi, delta, pred;
    Atrac3pWaveParam *wsrc, *wref;
    int refwaves[48] = { 0 };
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
    Atrac3pWavesData *ref = ctx->channels[0].tones_info;

    if (ch_num) {
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            wsrc = &ctx->waves_info->waves[dst[sb].start_index];
            wref = &ctx->waves_info->waves[ref[sb].start_index];
            for (j = 0; j < dst[sb].num_wavs; j++) {
                for (i = 0, fi = 0, maxdiff = 1024; i < ref[sb].num_wavs; i++) {
                    diff = FFABS(wsrc[j].freq_index - wref[i].freq_index);
                    if (diff < maxdiff) {
                        maxdiff = diff;
                        fi      = i;
                    }
                }

                if (maxdiff < 8)
                    refwaves[dst[sb].start_index + j] = fi + ref[sb].start_index;
                else if (j < ref[sb].num_wavs)
                    refwaves[dst[sb].start_index + j] = j + ref[sb].start_index;
                else
                    refwaves[dst[sb].start_index + j] = -1;
            }
        }
    }

    mode = get_bits(gb, ch_num + 1);

    switch (mode) {
    case 0: /** fixed-length coding */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            if (ctx->waves_info->amplitude_mode)
                for (i = 0; i < dst[sb].num_wavs; i++)
                    ctx->waves_info->waves[dst[sb].start_index + i].amp_sf = get_bits(gb, 6);
            else
                ctx->waves_info->waves[dst[sb].start_index].amp_sf = get_bits(gb, 6);
        }
        break;
    case 1: /** min + VLC delta */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            if (ctx->waves_info->amplitude_mode)
                for (i = 0; i < dst[sb].num_wavs; i++)
                    ctx->waves_info->waves[dst[sb].start_index + i].amp_sf =
                        get_vlc2(gb, tone_vlc_tabs[3].table,
                                 tone_vlc_tabs[3].bits, 1) + 20;
            else
                ctx->waves_info->waves[dst[sb].start_index].amp_sf =
                    get_vlc2(gb, tone_vlc_tabs[4].table,
                             tone_vlc_tabs[4].bits, 1) + 24;
        }
        break;
    case 2: /** VLC modulo delta to master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb] || !dst[sb].num_wavs)
                continue;
            for (i = 0; i < dst[sb].num_wavs; i++) {
                delta = get_vlc2(gb, tone_vlc_tabs[5].table,
                                 tone_vlc_tabs[5].bits, 1);
                delta = sign_extend(delta, 5);
                pred  = refwaves[dst[sb].start_index + i] >= 0 ?
                        ctx->waves_info->waves[refwaves[dst[sb].start_index + i]].amp_sf : 34;
                ctx->waves_info->waves[dst[sb].start_index + i].amp_sf = (pred + delta) & 0x3F;
            }
        }
        break;
    case 3: /** clone master (slave only) */
        for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
            if (!band_has_tones[sb])
                continue;
            for (i = 0; i < dst[sb].num_wavs; i++)
                ctx->waves_info->waves[dst[sb].start_index + i].amp_sf =
                    refwaves[dst[sb].start_index + i] >= 0
                    ? ctx->waves_info->waves[refwaves[dst[sb].start_index + i]].amp_sf
                    : 32;
        }
        break;
    }
}

/**
 * Decode phase information for each subband of a channel.
 *
 * @param[in]     gb                the GetBit context
 * @param[in,out] ctx               ptr to the channel unit context
 * @param[in]     ch_num            channel to process
 * @param[in]     band_has_tones    ptr to an array of per-band-flags:
 *                                  1 - tone data present
 */
static void decode_tones_phase(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                               int ch_num, int band_has_tones[])
{
    int sb, i;
    Atrac3pWaveParam *wparam;
    Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;

    for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
        if (!band_has_tones[sb])
            continue;
        wparam = &ctx->waves_info->waves[dst[sb].start_index];
        for (i = 0; i < dst[sb].num_wavs; i++)
            wparam[i].phase_index = get_bits(gb, 5);
    }
}

/**
 * Decode tones info for all channels.
 *
 * @param[in]     gb            the GetBit context
 * @param[in,out] ctx           ptr to the channel unit context
 * @param[in]     num_channels  number of channels to process
 * @return result code: 0 = OK, otherwise - error code
 */
static int decode_tones_info(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                             int num_channels)
{
    int ch_num, i, ret;
    int band_has_tones[16];

    for (ch_num = 0; ch_num < num_channels; ch_num++)
        memset(ctx->channels[ch_num].tones_info, 0,
               sizeof(*ctx->channels[ch_num].tones_info) * ATRAC3P_SUBBANDS);

    ctx->waves_info->tones_present = get_bits1(gb);
    if (!ctx->waves_info->tones_present)
        return 0;

    memset(ctx->waves_info->waves, 0, sizeof(ctx->waves_info->waves));

    ctx->waves_info->amplitude_mode = get_bits1(gb);
    if (!ctx->waves_info->amplitude_mode) {
        av_log(AV_LOG_WARNING, "GHA amplitude mode 0");
        return AVERROR_PATCHWELCOME;
    }

    ctx->waves_info->num_tone_bands =
        get_vlc2(gb, tone_vlc_tabs[0].table,
                 tone_vlc_tabs[0].bits, 1) + 1;

    if (num_channels == 2) {
        get_subband_flags(gb, ctx->waves_info->tone_sharing, ctx->waves_info->num_tone_bands);
        get_subband_flags(gb, ctx->waves_info->tone_master,  ctx->waves_info->num_tone_bands);
        get_subband_flags(gb, ctx->waves_info->invert_phase, ctx->waves_info->num_tone_bands);
    }

    ctx->waves_info->tones_index = 0;

    for (ch_num = 0; ch_num < num_channels; ch_num++) {
        for (i = 0; i < ctx->waves_info->num_tone_bands; i++)
            band_has_tones[i] = !ch_num ? 1 : !ctx->waves_info->tone_sharing[i];

        decode_tones_envelope(gb, ctx, ch_num, band_has_tones);
        if ((ret = decode_band_numwavs(gb, ctx, ch_num, band_has_tones)) < 0)
            return ret;

        decode_tones_frequency(gb, ctx, ch_num, band_has_tones);
        decode_tones_amplitude(gb, ctx, ch_num, band_has_tones);
        decode_tones_phase(gb, ctx, ch_num, band_has_tones);
    }

    if (num_channels == 2) {
        for (i = 0; i < ctx->waves_info->num_tone_bands; i++) {
            if (ctx->waves_info->tone_sharing[i])
                ctx->channels[1].tones_info[i] = ctx->channels[0].tones_info[i];

            if (ctx->waves_info->tone_master[i])
                FFSWAP(Atrac3pWavesData, ctx->channels[0].tones_info[i],
                       ctx->channels[1].tones_info[i]);
        }
    }

    return 0;
}

int ff_atrac3p_decode_channel_unit(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
                                   int num_channels)
{
    int ret;

    /* parse sound header */
    ctx->num_quant_units = get_bits(gb, 5) + 1;
    if (ctx->num_quant_units > 28 && ctx->num_quant_units < 32) {
        av_log(AV_LOG_ERROR,
               "Invalid number of quantization units: %d!\n",
               ctx->num_quant_units);
        return AVERROR_INVALIDDATA;
    }

    ctx->mute_flag = get_bits1(gb);

    /* decode various sound parameters */
    if ((ret = decode_quant_wordlen(gb, ctx, num_channels)) < 0)
        return ret;

    ctx->num_subbands       = atrac3p_qu_to_subband[ctx->num_quant_units - 1] + 1;
    ctx->num_coded_subbands = ctx->used_quant_units
                              ? atrac3p_qu_to_subband[ctx->used_quant_units - 1] + 1
                              : 0;

    if ((ret = decode_scale_factors(gb, ctx, num_channels)) < 0)
        return ret;

    if ((ret = decode_code_table_indexes(gb, ctx, num_channels)) < 0)
        return ret;

    decode_spectrum(gb, ctx, num_channels);

    if (num_channels == 2) {
        get_subband_flags(gb, ctx->swap_channels, ctx->num_coded_subbands);
        get_subband_flags(gb, ctx->negate_coeffs, ctx->num_coded_subbands);
    }

    decode_window_shape(gb, ctx, num_channels);

    if ((ret = decode_gainc_data(gb, ctx, num_channels)) < 0)
        return ret;

    if ((ret = decode_tones_info(gb, ctx, num_channels)) < 0)
        return ret;

    /* decode global noise info */
    ctx->noise_present = get_bits1(gb);
    if (ctx->noise_present) {
        ctx->noise_level_index = get_bits(gb, 4);
        ctx->noise_table_index = get_bits(gb, 4);
    }

    return 0;
}

/* ============================================================================
 * ATRAC3+ DSP (atrac3plusdsp.cpp)
 * ============================================================================ */
/*
 * ATRAC3+ compatible decoder
 *
 * Copyright (c) 2010-2013 Maxim Poliakovski
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 *  @file
 *  DSP functions for ATRAC3+ decoder.
 */


#if 0 // SIMD disabled


#elif 0

#if defined(_MSC_VER) && PPSSPP_ARCH(ARM64)
#else
#endif

#endif

#define _USE_MATH_DEFINES


/**
 *  Map quant unit number to its position in the spectrum.
 *  To get the number of spectral lines in each quant unit do the following:
 *  num_specs = qu_to_spec_pos[i+1] - qu_to_spec_pos[i]
 */
const uint16_t av_atrac3p_qu_to_spec_pos[33] = {
      0,    16,   32,   48,   64,   80,   96,  112,
    128,   160,  192,  224,  256,  288,  320,  352,
    384,   448,  512,  576,  640,  704,  768,  896,
    1024, 1152, 1280, 1408, 1536, 1664, 1792, 1920,
    2048
};

/* Scalefactors table. */
/* Approx. Equ: pow(2.0f, (i - 16.0f + 0.501783948f) / 3.0f) */
const float av_atrac3p_sf_tab[64] = {
    0.027852058f,  0.0350914f, 0.044212341f, 0.055704117f,  0.0701828f,
    0.088424683f, 0.11140823f,   0.1403656f,  0.17684937f, 0.22281647f, 0.2807312f, 0.35369873f,
    0.44563293f,   0.5614624f,  0.70739746f,  0.89126587f,  1.1229248f, 1.4147949f,  1.7825317f,
    2.2458496f,    2.8295898f,   3.5650635f,   4.4916992f,  5.6591797f,  7.130127f,  8.9833984f,
    11.318359f,    14.260254f,   17.966797f,   22.636719f,  28.520508f, 35.933594f,  45.273438f,
    57.041016f,    71.867188f,   90.546875f,   114.08203f,  143.73438f, 181.09375f,  228.16406f,
    287.46875f,     362.1875f,   456.32812f,    574.9375f,    724.375f, 912.65625f,   1149.875f,
    1448.75f,      1825.3125f,     2299.75f,      2897.5f,   3650.625f,    4599.5f,     5795.0f,
    7301.25f,         9199.0f,     11590.0f,     14602.5f,    18398.0f,   23180.0f,    29205.0f,
    36796.0f,        46360.0f,     58410.0f
};

/* Mantissa table. */
/* pow(10, x * log10(2) + 0.05f) / 2 / ([1,2,3,5,7,15,31] + 0.5f) */
const float av_atrac3p_mant_tab[8] = {
    0.0f,
    0.74801636f,
    0.44882202f,
    0.32058716f,
    0.20400238f,
    0.1496048f,
    0.07239151f,
    0.035619736f
};

DECLARE_ALIGNED(32, float, av_sine_64)[64];
DECLARE_ALIGNED(32, float, av_sine_128)[128];

// Generate a sine window.
static void ff_sine_window_init(float *window, int n) {
    int i;
    for (i = 0; i < n; i++)
        window[i] = sinf((i + 0.5f) * (M_PI / (2.0f * n)));
}

#define ATRAC3P_MDCT_SIZE (ATRAC3P_SUBBAND_SAMPLES * 2)

void ff_atrac3p_init_imdct(FFTContext *mdct_ctx)
{
    ff_sine_window_init(av_sine_64, 64);
    ff_sine_window_init(av_sine_128, 128);

    /* Initialize the MDCT transform. */
    ff_mdct_init(mdct_ctx, 8, 1, -1.0f);
}

#define TWOPI (2 * M_PI)

#define DEQUANT_PHASE(ph) (((ph) & 0x1F) << 6)

static DECLARE_ALIGNED(32, float, sine_table)[2048]; ///< wave table
static DECLARE_ALIGNED(32, float, hann_window)[256]; ///< Hann windowing function
static float amp_sf_tab[64];   ///< scalefactors for quantized amplitudes

void ff_atrac3p_init_wave_synth(void)
{
    int i;

    /* generate sine wave table */
    for (i = 0; i < 2048; i++)
        sine_table[i] = (float)sin(TWOPI * i / 2048);

    /* generate Hann window */
    for (i = 0; i < 256; i++)
        hann_window[i] = (1.0f - (float)cos(TWOPI * i / 256.0f)) * 0.5f;

    /* generate amplitude scalefactors table */
    for (i = 0; i < 64; i++)
        amp_sf_tab[i] = exp2f((i - 3) / 4.0f);
}

/**
 *  Synthesize sine waves according to given parameters.
 *
 *  @param[in]    synth_param   ptr to common synthesis parameters
 *  @param[in]    waves_info    parameters for each sine wave
 *  @param[in]    envelope      envelope data for all waves in a group
 *  @param[in]    invert_phase  flag indicating 180° phase shift
 *  @param[in]    reg_offset    region offset for trimming envelope data
 *  @param[out]   out           receives sythesized data
 */
static void waves_synth(Atrac3pWaveSynthParams *synth_param,
                        Atrac3pWavesData *waves_info,
                        Atrac3pWaveEnvelope *envelope,
                        int invert_phase, int reg_offset, float *out)
{
    int i, wn, inc, pos;
    double amp;
    Atrac3pWaveParam *wave_param = &synth_param->waves[waves_info->start_index];

    for (wn = 0; wn < waves_info->num_wavs; wn++, wave_param++) {
        /* amplitude dequantization */
        amp = amp_sf_tab[wave_param->amp_sf] *
              (!synth_param->amplitude_mode
               ? (wave_param->amp_index + 1) / 15.13f
               : 1.0f);

        inc = wave_param->freq_index;
        pos = (DEQUANT_PHASE(wave_param->phase_index) - (reg_offset ^ 128) * inc) & 2047;

        /* waveform generation */
        for (i = 0; i < 128; i++) {
            out[i] += (float)(sine_table[pos] * amp);
            pos     = (pos + inc) & 2047;
        }
    }

    /* invert phase if requested */
    if (invert_phase)
        vector_fmul_scalar(out, -1.0f, 128);

    /* fade in with steep Hann window if requested */
    if (envelope->has_start_point) {
        pos = (envelope->start_pos << 2) - reg_offset;
        if (pos > 0 && pos <= 128) {
            memset(out, 0, pos * sizeof(*out));
            if (!envelope->has_stop_point ||
                envelope->start_pos != envelope->stop_pos) {
                out[pos + 0] *= hann_window[0];
                out[pos + 1] *= hann_window[32];
                out[pos + 2] *= hann_window[64];
                out[pos + 3] *= hann_window[96];
            }
        }
    }

    /* fade out with steep Hann window if requested */
    if (envelope->has_stop_point) {
        pos = ((envelope->stop_pos + 1) << 2) - reg_offset;
        if (pos > 0 && pos <= 128) {
            out[pos - 4] *= hann_window[96];
            out[pos - 3] *= hann_window[64];
            out[pos - 2] *= hann_window[32];
            out[pos - 1] *= hann_window[0];
            memset(&out[pos], 0, (128 - pos) * sizeof(out[pos]));
        }
    }
}

void ff_atrac3p_generate_tones(Atrac3pChanUnitCtx *ch_unit, int ch_num, int sb, float *out)
{
    DECLARE_ALIGNED(32, float, wavreg1)[128] = { 0 };
    DECLARE_ALIGNED(32, float, wavreg2)[128] = { 0 };
    int i, reg1_env_nonzero, reg2_env_nonzero;
    Atrac3pWavesData *tones_now  = &ch_unit->channels[ch_num].tones_info_prev[sb];
    Atrac3pWavesData *tones_next = &ch_unit->channels[ch_num].tones_info[sb];

    /* reconstruct full envelopes for both overlapping regions
     * from truncated bitstream data */
    if (tones_next->pend_env.has_start_point &&
        tones_next->pend_env.start_pos < tones_next->pend_env.stop_pos) {
        tones_next->curr_env.has_start_point = 1;
        tones_next->curr_env.start_pos       = tones_next->pend_env.start_pos + 32;
    } else if (tones_now->pend_env.has_start_point) {
        tones_next->curr_env.has_start_point = 1;
        tones_next->curr_env.start_pos       = tones_now->pend_env.start_pos;
    } else {
        tones_next->curr_env.has_start_point = 0;
        tones_next->curr_env.start_pos       = 0;
    }

    if (tones_now->pend_env.has_stop_point &&
        tones_now->pend_env.stop_pos >= tones_next->curr_env.start_pos) {
        tones_next->curr_env.has_stop_point = 1;
        tones_next->curr_env.stop_pos       = tones_now->pend_env.stop_pos;
    } else if (tones_next->pend_env.has_stop_point) {
        tones_next->curr_env.has_stop_point = 1;
        tones_next->curr_env.stop_pos       = tones_next->pend_env.stop_pos + 32;
    } else {
        tones_next->curr_env.has_stop_point = 0;
        tones_next->curr_env.stop_pos       = 64;
    }

    /* is the visible part of the envelope non-zero? */
    reg1_env_nonzero = (tones_now->curr_env.stop_pos    < 32) ? 0 : 1;
    reg2_env_nonzero = (tones_next->curr_env.start_pos >= 32) ? 0 : 1;

    /* synthesize waves for both overlapping regions */
    if (tones_now->num_wavs && reg1_env_nonzero)
        waves_synth(ch_unit->waves_info_prev, tones_now, &tones_now->curr_env,
                    ch_unit->waves_info_prev->invert_phase[sb] & ch_num,
                    128, wavreg1);

    if (tones_next->num_wavs && reg2_env_nonzero)
        waves_synth(ch_unit->waves_info, tones_next, &tones_next->curr_env,
                    ch_unit->waves_info->invert_phase[sb] & ch_num, 0, wavreg2);

    /* Hann windowing for non-faded wave signals */
    if (tones_now->num_wavs && tones_next->num_wavs &&
        reg1_env_nonzero && reg2_env_nonzero) {
        vector_fmul(wavreg1, &hann_window[128], 128);
        vector_fmul(wavreg2,  hann_window,      128);
    } else {
        if (tones_now->num_wavs && !tones_now->curr_env.has_stop_point)
            vector_fmul(wavreg1, &hann_window[128], 128);

        if (tones_next->num_wavs && !tones_next->curr_env.has_start_point)
            vector_fmul(wavreg2, hann_window, 128);
    }

    /* Overlap and add to residual */
    for (i = 0; i < 128; i++)
        out[i] += wavreg1[i] + wavreg2[i];
}

static const int subband_to_powgrp[ATRAC3P_SUBBANDS] = {
    0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4
};

/* noise table for power compensation */
static const float noise_tab[1024] = {
    -0.01358032f,  -0.05593872f,   0.01696777f,  -0.14871216f,  -0.26412964f,  -0.09893799f,   0.25723267f,
     0.02008057f,  -0.72235107f,  -0.44351196f,  -0.22985840f,   0.16833496f,   0.46902466f,   0.05917358f,
    -0.15179443f,   0.41299438f,  -0.01287842f,   0.13360596f,   0.43557739f,  -0.09530640f,  -0.58422852f,
     0.39266968f,  -0.08343506f,  -0.25604248f,   0.22848511f,   0.26013184f,  -0.65588379f,   0.17288208f,
    -0.08673096f,  -0.05203247f,   0.07299805f,  -0.28665161f,  -0.35806274f,   0.06552124f,  -0.09387207f,
     0.21099854f,  -0.28347778f,  -0.72402954f,   0.05050659f,  -0.10635376f,  -0.18853760f,   0.29724121f,
     0.20703125f,  -0.29791260f,  -0.37634277f,   0.47970581f,  -0.09976196f,   0.32641602f,  -0.29248047f,
    -0.28237915f,   0.26028442f,  -0.36157227f,   0.22042847f,  -0.03222656f,  -0.37268066f,  -0.03759766f,
     0.09909058f,   0.23284912f,   0.19320679f,   0.14453125f,  -0.02139282f,  -0.19702148f,   0.31533813f,
    -0.16741943f,   0.35031128f,  -0.35656738f,  -0.66128540f,  -0.00701904f,   0.20898438f,   0.26837158f,
    -0.33706665f,  -0.04568481f,   0.12600708f,   0.10284424f,   0.07321167f,  -0.18280029f,   0.38101196f,
     0.21301270f,   0.04541016f,   0.01156616f,  -0.26391602f,  -0.02346802f,  -0.22125244f,   0.29760742f,
    -0.36233521f,  -0.31314087f,  -0.13967896f,  -0.11276245f,  -0.19433594f,   0.34490967f,   0.02343750f,
     0.21963501f,  -0.02777100f,  -0.67678833f,  -0.08999634f,   0.14233398f,  -0.27697754f,   0.51422119f,
    -0.05047607f,   0.48327637f,   0.37167358f,  -0.60806274f,   0.18728638f,  -0.15191650f,   0.00637817f,
     0.02832031f,  -0.15618896f,   0.60644531f,   0.21826172f,   0.06384277f,  -0.31863403f,   0.08816528f,
     0.15447998f,  -0.07015991f,  -0.08154297f,  -0.40966797f,  -0.39785767f,  -0.11709595f,   0.22052002f,
     0.18466187f,  -0.17257690f,   0.03759766f,  -0.06195068f,   0.00433350f,   0.12176514f,   0.34011841f,
     0.25610352f,  -0.05294800f,   0.41033936f,   0.16854858f,  -0.76187134f,   0.13845825f,  -0.19418335f,
    -0.21524048f,  -0.44412231f,  -0.08160400f,  -0.28195190f,  -0.01873779f,   0.15524292f,  -0.37438965f,
    -0.44860840f,   0.43096924f,  -0.24746704f,   0.49856567f,   0.14859009f,   0.38159180f,   0.20541382f,
    -0.39175415f,  -0.65850830f,  -0.43716431f,   0.13037109f,  -0.05111694f,   0.39956665f,   0.21447754f,
    -0.04861450f,   0.33654785f,   0.10589600f,  -0.88085938f,  -0.30822754f,   0.38577271f,   0.30047607f,
     0.38836670f,   0.09118652f,  -0.36477661f,  -0.01641846f,  -0.23031616f,   0.26058960f,   0.18859863f,
    -0.21868896f,  -0.17861938f,  -0.29754639f,   0.09777832f,   0.10806274f,  -0.51605225f,   0.00076294f,
     0.13259888f,   0.11090088f,  -0.24084473f,   0.24957275f,   0.01379395f,  -0.04141235f,  -0.04937744f,
     0.57394409f,   0.27410889f,   0.27587891f,   0.45013428f,  -0.32592773f,   0.11160278f,  -0.00970459f,
     0.29092407f,   0.03356934f,  -0.70925903f,   0.04882812f,   0.43499756f,   0.07720947f,  -0.27554321f,
    -0.01742554f,  -0.08413696f,  -0.04028320f,  -0.52850342f,  -0.07330322f,   0.05181885f,   0.21362305f,
    -0.18765259f,   0.07058716f,  -0.03009033f,   0.32662964f,   0.27023315f,  -0.28002930f,   0.17568970f,
     0.03338623f,   0.30242920f,  -0.03921509f,   0.32174683f,  -0.23733521f,   0.08575439f,  -0.38269043f,
     0.09194946f,  -0.07238770f,   0.17941284f,  -0.51278687f,  -0.25146484f,   0.19790649f,  -0.19195557f,
     0.16549683f,   0.42456055f,   0.39129639f,  -0.02868652f,   0.17980957f,   0.24902344f,  -0.76583862f,
    -0.20959473f,   0.61013794f,   0.37011719f,   0.36859131f,  -0.04486084f,   0.10678101f,  -0.15994263f,
    -0.05328369f,   0.28463745f,  -0.06420898f,  -0.36987305f,  -0.28009033f,  -0.11764526f,   0.04312134f,
    -0.08038330f,   0.04885864f,  -0.03067017f,  -0.00042725f,   0.34289551f,  -0.00988770f,   0.34838867f,
     0.32516479f,  -0.16271973f,   0.38269043f,   0.03240967f,   0.12417603f,  -0.14331055f,  -0.34902954f,
    -0.18325806f,   0.29421997f,   0.44284058f,   0.75170898f,  -0.67245483f,  -0.12176514f,   0.27914429f,
    -0.29806519f,   0.19863892f,   0.30087280f,   0.22680664f,  -0.36633301f,  -0.32534790f,  -0.57553101f,
    -0.16641235f,   0.43811035f,   0.08331299f,   0.15942383f,   0.26516724f,  -0.24240112f,  -0.11761475f,
    -0.16827393f,  -0.14260864f,   0.46343994f,   0.11804199f,  -0.55514526f,  -0.02520752f,  -0.14309692f,
     0.00448608f,   0.02749634f,  -0.30545044f,   0.70965576f,   0.45108032f,   0.66439819f,  -0.68255615f,
    -0.12496948f,   0.09146118f,  -0.21109009f,  -0.23791504f,   0.79943848f,  -0.35205078f,  -0.24963379f,
     0.18719482f,  -0.19079590f,   0.07458496f,   0.07623291f,  -0.28781128f,  -0.37121582f,  -0.19580078f,
    -0.01773071f,  -0.16717529f,   0.13040161f,   0.14672852f,   0.42379761f,   0.03582764f,   0.11431885f,
     0.05145264f,   0.44702148f,   0.08963013f,   0.01367188f,  -0.54519653f,  -0.12692261f,   0.21176147f,
     0.04925537f,   0.30670166f,  -0.11029053f,   0.19555664f,  -0.27740479f,   0.23043823f,   0.15554810f,
    -0.19299316f,  -0.25729370f,   0.17800903f,  -0.03579712f,  -0.05065918f,  -0.06933594f,  -0.09500122f,
    -0.07821655f,   0.23889160f,  -0.31900024f,   0.03073120f,  -0.00415039f,   0.61315918f,   0.37176514f,
    -0.13442993f,  -0.15536499f,  -0.19216919f,  -0.37899780f,   0.19992065f,   0.02630615f,  -0.12573242f,
     0.25927734f,  -0.02447510f,   0.29629517f,  -0.40731812f,  -0.17333984f,   0.24310303f,  -0.10607910f,
     0.14828491f,   0.08792114f,  -0.18743896f,  -0.05572510f,  -0.04833984f,   0.10473633f,  -0.29028320f,
    -0.67687988f,  -0.28170776f,  -0.41687012f,   0.05413818f,  -0.23284912f,   0.09555054f,  -0.08969116f,
    -0.15112305f,   0.12738037f,   0.35986328f,   0.28948975f,   0.30691528f,   0.23956299f,   0.06973267f,
    -0.31198120f,  -0.18450928f,   0.22280884f,  -0.21600342f,   0.23522949f,  -0.61840820f,  -0.13012695f,
     0.26412964f,   0.47320557f,  -0.26440430f,   0.38757324f,   0.17352295f,  -0.26104736f,  -0.25866699f,
    -0.12274170f,  -0.29733276f,   0.07687378f,   0.18588257f,  -0.08880615f,   0.31185913f,   0.05313110f,
    -0.10885620f,  -0.14901733f,  -0.22323608f,  -0.08538818f,   0.19812012f,   0.19732666f,  -0.18927002f,
     0.29058838f,   0.25555420f,  -0.48599243f,   0.18768311f,   0.01345825f,   0.34887695f,   0.21530151f,
     0.19857788f,   0.18661499f,  -0.01394653f,  -0.09063721f,  -0.38781738f,   0.27160645f,  -0.20379639f,
    -0.32119751f,  -0.23889160f,   0.27096558f,   0.24951172f,   0.07922363f,   0.07479858f,  -0.50946045f,
     0.10220337f,   0.58364868f,  -0.19503784f,  -0.18560791f,  -0.01165771f,   0.47195435f,   0.22430420f,
    -0.38635254f,  -0.03732300f,  -0.09179688f,   0.06991577f,   0.15106201f,   0.20605469f,  -0.05969238f,
    -0.41821289f,   0.12231445f,  -0.04672241f,  -0.05117798f,  -0.11523438f,  -0.51849365f,  -0.04077148f,
     0.44284058f,  -0.64086914f,   0.17019653f,   0.02236938f,   0.22848511f,  -0.23214722f,  -0.32354736f,
    -0.14068604f,  -0.29690552f,  -0.19891357f,   0.02774048f,  -0.20965576f,  -0.52191162f,  -0.19299316f,
    -0.07290649f,   0.49053955f,  -0.22302246f,   0.05642700f,   0.13122559f,  -0.20819092f,  -0.83590698f,
    -0.08181763f,   0.26797485f,  -0.00091553f,  -0.09457397f,   0.17089844f,  -0.27020264f,   0.30270386f,
     0.05496216f,   0.09564209f,  -0.08590698f,   0.02130127f,   0.35931396f,   0.21728516f,  -0.15396118f,
    -0.05053711f,   0.02719116f,   0.16302490f,   0.43212891f,   0.10229492f,  -0.40820312f,   0.21646118f,
     0.08435059f,  -0.11145020f,  -0.39962769f,  -0.05618286f,  -0.10223389f,  -0.60839844f,   0.33724976f,
    -0.06341553f,  -0.47369385f,  -0.32852173f,   0.05242920f,   0.19635010f,  -0.19137573f,  -0.67901611f,
     0.16180420f,   0.05133057f,  -0.22283936f,   0.09646606f,   0.24288940f,  -0.45007324f,   0.08804321f,
     0.14053345f,   0.22619629f,  -0.01000977f,   0.36355591f,  -0.19863892f,  -0.30364990f,  -0.24118042f,
    -0.57461548f,   0.26498413f,   0.04345703f,  -0.09796143f,  -0.47714233f,  -0.23739624f,   0.18737793f,
     0.08926392f,  -0.02795410f,   0.00305176f,  -0.08700562f,  -0.38711548f,   0.03222656f,   0.10940552f,
    -0.41906738f,  -0.01620483f,  -0.47061157f,   0.37985229f,  -0.21624756f,   0.47976685f,  -0.20046997f,
    -0.62533569f,  -0.26907349f,  -0.02877808f,   0.00671387f,  -0.29071045f,  -0.24685669f,  -0.15722656f,
    -0.26055908f,   0.29968262f,   0.28225708f,  -0.08990479f,  -0.16748047f,  -0.46759033f,  -0.25067139f,
    -0.25183105f,  -0.45932007f,   0.05828857f,   0.29006958f,   0.23840332f,  -0.17974854f,   0.26931763f,
     0.10696411f,  -0.06848145f,  -0.17126465f,  -0.10522461f,  -0.55386353f,  -0.42306519f,  -0.07608032f,
     0.24380493f,   0.38586426f,   0.16882324f,   0.26751709f,   0.17303467f,   0.35809326f,  -0.22094727f,
    -0.30703735f,  -0.28497314f,  -0.04321289f,   0.15219116f,  -0.17071533f,  -0.39334106f,   0.03439331f,
    -0.10809326f,  -0.30590820f,   0.26449585f,  -0.07412720f,   0.13638306f,  -0.01062012f,   0.27996826f,
     0.04397583f,  -0.05557251f,  -0.56933594f,   0.03363037f,  -0.00949097f,   0.52642822f,  -0.44329834f,
     0.28308105f,  -0.05499268f,  -0.23312378f,  -0.29870605f,  -0.05123901f,   0.26831055f,  -0.35238647f,
    -0.30993652f,   0.34646606f,  -0.19775391f,   0.44595337f,   0.13769531f,   0.45358276f,   0.19961548f,
     0.42681885f,   0.15722656f,   0.00128174f,   0.23757935f,   0.40988159f,   0.25164795f,  -0.00732422f,
    -0.12405396f,  -0.43420410f,  -0.00402832f,   0.34243774f,   0.36264038f,   0.18807983f,  -0.09301758f,
    -0.10296631f,   0.05532837f,  -0.31652832f,   0.14337158f,   0.35040283f,   0.32540894f,   0.05728149f,
    -0.12030029f,  -0.25942993f,  -0.20312500f,  -0.16491699f,  -0.46051025f,  -0.08004761f,   0.50772095f,
     0.16168213f,   0.28439331f,   0.08105469f,  -0.19104004f,   0.38589478f,  -0.16400146f,  -0.25454712f,
     0.20281982f,  -0.20730591f,  -0.06311035f,   0.32937622f,   0.15032959f,  -0.05340576f,   0.30487061f,
    -0.11648560f,   0.38009644f,  -0.20062256f,   0.43466187f,   0.01150513f,   0.35754395f,  -0.13146973f,
     0.67489624f,   0.05212402f,   0.27914429f,  -0.39431763f,   0.75308228f,  -0.13366699f,   0.24453735f,
     0.42248535f,  -0.65905762f,  -0.00546265f,  -0.03491211f,  -0.13659668f,  -0.08294678f,  -0.45666504f,
     0.27188110f,   0.12731934f,   0.61148071f,   0.10449219f,  -0.28836060f,   0.00091553f,   0.24618530f,
     0.13119507f,   0.05685425f,   0.17355347f,   0.42034912f,   0.08514404f,   0.24536133f,   0.18951416f,
    -0.19107056f,  -0.15036011f,   0.02334595f,   0.54986572f,   0.32321167f,  -0.16104126f,  -0.03054810f,
     0.43594360f,   0.17309570f,   0.61053467f,   0.24731445f,   0.33334351f,   0.15240479f,   0.15588379f,
     0.36425781f,  -0.30407715f,  -0.13302612f,   0.00427246f,   0.04171753f,  -0.33178711f,   0.34216309f,
    -0.12463379f,  -0.02764893f,   0.05905151f,  -0.31436157f,   0.16531372f,   0.34542847f,  -0.03292847f,
     0.12527466f,  -0.12313843f,  -0.13171387f,   0.04757690f,  -0.45095825f,  -0.19085693f,   0.35342407f,
    -0.23239136f,  -0.34387207f,   0.11264038f,  -0.15740967f,   0.05273438f,   0.74942017f,   0.21505737f,
     0.08514404f,  -0.42391968f,  -0.19531250f,   0.35293579f,   0.25305176f,   0.15731812f,  -0.70324707f,
    -0.21591187f,   0.35604858f,   0.14132690f,   0.11724854f,   0.15853882f,  -0.24597168f,   0.07019043f,
     0.02127075f,   0.12658691f,   0.06390381f,  -0.12292480f,   0.15441895f,  -0.47640991f,   0.06195068f,
     0.58981323f,  -0.15151978f,  -0.03604126f,  -0.45059204f,  -0.01672363f,  -0.46997070f,   0.25750732f,
     0.18084717f,   0.06661987f,   0.13253784f,   0.67828369f,   0.11370850f,   0.11325073f,  -0.04611206f,
    -0.07791138f,  -0.36544800f,  -0.06747437f,  -0.31594849f,   0.16131592f,   0.41983032f,   0.11071777f,
    -0.36889648f,   0.30963135f,  -0.37875366f,   0.58508301f,   0.00393677f,   0.12338257f,   0.03424072f,
    -0.21728516f,  -0.12838745f,  -0.46981812f,   0.05868530f,  -0.25015259f,   0.27407837f,   0.65240479f,
    -0.34429932f,  -0.15179443f,   0.14056396f,   0.33505249f,   0.28826904f,   0.09921265f,   0.34390259f,
     0.13656616f,  -0.23608398f,   0.00863647f,   0.02627563f,  -0.19119263f,   0.19775391f,  -0.07214355f,
     0.07809448f,   0.03454590f,  -0.03417969f,   0.00033569f,  -0.23095703f,   0.18673706f,   0.05798340f,
     0.03814697f,  -0.04318237f,   0.05487061f,   0.08633423f,   0.55950928f,  -0.06347656f,   0.10333252f,
     0.25305176f,   0.05853271f,   0.12246704f,  -0.25543213f,  -0.34262085f,  -0.36437988f,  -0.21304321f,
    -0.05093384f,   0.02777100f,   0.07620239f,  -0.21215820f,  -0.09326172f,   0.19021606f,  -0.40579224f,
    -0.01193237f,   0.19845581f,  -0.35336304f,  -0.07397461f,   0.20104980f,   0.08615112f,  -0.44375610f,
     0.11419678f,   0.24453735f,  -0.16555786f,  -0.05081177f,  -0.01406860f,   0.27893066f,  -0.18692017f,
     0.07473755f,   0.03451538f,  -0.39733887f,   0.21548462f,  -0.22534180f,  -0.39651489f,  -0.04989624f,
    -0.57662964f,   0.06390381f,   0.62020874f,  -0.13470459f,   0.04345703f,  -0.21862793f,  -0.02789307f,
     0.51696777f,  -0.27587891f,   0.39004517f,   0.09857178f,  -0.00738525f,   0.31317139f,   0.00048828f,
    -0.46572876f,   0.29531860f,  -0.10009766f,  -0.27856445f,   0.03594971f,   0.25048828f,  -0.74584961f,
    -0.25350952f,  -0.03302002f,   0.31188965f,   0.01571655f,   0.46710205f,   0.21591187f,   0.07260132f,
    -0.42132568f,  -0.53900146f,  -0.13674927f,  -0.16571045f,  -0.34454346f,   0.12359619f,  -0.11184692f,
     0.00967407f,   0.34576416f,  -0.05761719f,   0.34848022f,   0.17645264f,  -0.39395142f,   0.10339355f,
     0.18215942f,   0.20697021f,   0.59109497f,  -0.11560059f,  -0.07385254f,   0.10397339f,   0.35437012f,
    -0.22863770f,   0.01794434f,   0.17559814f,  -0.17495728f,   0.12142944f,   0.10928345f,  -1.00000000f,
    -0.01379395f,   0.21237183f,  -0.27035522f,   0.27319336f,  -0.37066650f,   0.41354370f,  -0.40054321f,
     0.00689697f,   0.26321411f,   0.39266968f,   0.65298462f,   0.41625977f,  -0.13909912f,   0.78375244f,
    -0.30941772f,   0.20169067f,  -0.39367676f,   0.94021606f,  -0.24066162f,   0.05557251f,  -0.24533081f,
    -0.05444336f,  -0.76754761f,  -0.19375610f,  -0.11041260f,  -0.17532349f,   0.16006470f,   0.02188110f,
     0.17465210f,  -0.04342651f,  -0.56777954f,  -0.40988159f,   0.26687622f,   0.11700439f,  -0.00344849f,
    -0.05395508f,   0.37426758f,  -0.40719604f,  -0.15032959f,  -0.01660156f,   0.04196167f,  -0.04559326f,
    -0.12969971f,   0.12011719f,   0.08419800f,  -0.11199951f,   0.35174561f,   0.10275269f,  -0.25686646f,
     0.48446655f,   0.03225708f,   0.28408813f,  -0.18701172f,   0.36282349f,  -0.03280640f,   0.32302856f,
     0.17233276f,   0.48269653f,   0.31112671f,  -0.04946899f,   0.12774658f,   0.52685547f,   0.10211182f,
     0.05953979f,   0.05999756f,   0.20144653f,   0.00744629f,   0.27316284f,   0.24377441f,   0.39672852f,
     0.01702881f,  -0.35513306f,   0.11364746f,  -0.13555908f,   0.48880005f,  -0.15417480f,  -0.09149170f,
    -0.02615356f,   0.46246338f,  -0.72250366f,   0.22332764f,   0.23849487f,  -0.25686646f,  -0.08514404f,
    -0.02062988f,  -0.34494019f,  -0.02297974f,  -0.80386353f,  -0.08074951f,  -0.12689209f,  -0.06896973f,
     0.24099731f,  -0.35650635f,  -0.09558105f,   0.29254150f,   0.23132324f,  -0.16726685f,   0.00000000f,
    -0.24237061f,   0.30899048f,   0.29504395f,  -0.20898438f,   0.17059326f,  -0.07672119f,  -0.14395142f,
     0.05572510f,   0.20602417f,  -0.51550293f,  -0.03167725f,  -0.48840332f,  -0.20425415f,   0.14144897f,
     0.07275391f,  -0.76669312f,  -0.22488403f,   0.20651245f,   0.03259277f,   0.00085449f,   0.03039551f,
     0.47555542f,   0.38351440f
};

/** Noise level table for power compensation.
 *  Equ: pow(2.0f, (double)(6 - i) / 3.0f) where i = 0...15 */
static const float pwc_levs[16] = {
    3.96875f, 3.15625f,     2.5f,    2.0f, 1.59375f,   1.25f,     1.0f, 0.78125f,
    0.625f,       0.5f, 0.40625f, 0.3125f,    0.25f, 0.1875f, 0.15625f, 0.0f
};

/** Map subband number to quant unit number. */
static const int subband_to_qu[17] = {
    0, 8, 12, 16, 18, 20, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
};

void ff_atrac3p_power_compensation(Atrac3pChanUnitCtx *ctx, int ch_index,
                                   float *sp, int rng_index, int sb)
{
    AtracGainInfo *g1, *g2;
    float pwcsp[ATRAC3P_SUBBAND_SAMPLES], *dst, grp_lev, qu_lev;
    int i, gain_lev, gcv = 0, qu, nsp;
    int swap_ch = (ctx->unit_type == CH_UNIT_STEREO && ctx->swap_channels[sb]) ? 1 : 0;

    if (ctx->channels[ch_index ^ swap_ch].power_levs[subband_to_powgrp[sb]] == ATRAC3P_POWER_COMP_OFF)
        return;

    /* generate initial noise spectrum */
    for (i = 0; i < ATRAC3P_SUBBAND_SAMPLES; i++, rng_index++)
        pwcsp[i] = noise_tab[rng_index & 0x3FF];

    /* check gain control information */
    g1 = &ctx->channels[ch_index ^ swap_ch].gain_data[sb];
    g2 = &ctx->channels[ch_index ^ swap_ch].gain_data_prev[sb];

    gain_lev = (g1->num_points > 0) ? (6 - g1->lev_code[0]) : 0;

    for (i = 0; i < g2->num_points; i++)
        gcv = FFMAX(gcv, gain_lev - (g2->lev_code[i] - 6));

    for (i = 0; i < g1->num_points; i++)
        gcv = FFMAX(gcv, 6 - g1->lev_code[i]);

    grp_lev = pwc_levs[ctx->channels[ch_index ^ swap_ch].power_levs[subband_to_powgrp[sb]]] / (1 << gcv);

    /* skip the lowest two quant units (frequencies 0...351 Hz) for subband 0 */
    for (qu = subband_to_qu[sb] + (!sb ? 2 : 0); qu < subband_to_qu[sb + 1]; qu++) {
        if (ctx->channels[ch_index].qu_wordlen[qu] <= 0)
            continue;

        qu_lev = av_atrac3p_sf_tab[ctx->channels[ch_index].qu_sf_idx[qu]] *
                 av_atrac3p_mant_tab[ctx->channels[ch_index].qu_wordlen[qu]] /
                 (1 << ctx->channels[ch_index].qu_wordlen[qu]) * grp_lev;

        dst = &sp[av_atrac3p_qu_to_spec_pos[qu]];
        nsp = av_atrac3p_qu_to_spec_pos[qu + 1] - av_atrac3p_qu_to_spec_pos[qu];

        for (i = 0; i < nsp; i++)
            dst[i] += pwcsp[i] * qu_lev;
    }
}

void ff_atrac3p_imdct(FFTContext *mdct_ctx, float *pIn,
                      float *pOut, int wind_id, int sb)
{
    int i;

    if (sb & 1)
        for (i = 0; i < ATRAC3P_SUBBAND_SAMPLES / 2; i++)
            FFSWAP(float, pIn[i], pIn[ATRAC3P_SUBBAND_SAMPLES - 1 - i]);

    imdct_calc(mdct_ctx, pOut, pIn);

    /* Perform windowing on the output.
     * ATRAC3+ uses two different MDCT windows:
     * - The first one is just the plain sine window of size 256
     * - The 2nd one is the plain sine window of size 128
     *   wrapped into zero (at the start) and one (at the end) regions.
     *   Both regions are 32 samples long. */
    if (wind_id & 2) { /* 1st half: steep window */
        memset(pOut, 0, sizeof(float) * 32);
        vector_fmul(&pOut[32], av_sine_64, 64);
    } else /* 1st half: simple sine window */
        vector_fmul(pOut, av_sine_128, ATRAC3P_MDCT_SIZE / 2);

    if (wind_id & 1) { /* 2nd half: steep window */
        vector_fmul_reverse(&pOut[160], av_sine_64, 64);
        memset(&pOut[224], 0, sizeof(float) * 32);
    } else /* 2nd half: simple sine window */
        vector_fmul_reverse(&pOut[128], av_sine_128, ATRAC3P_MDCT_SIZE / 2);
}

/* lookup table for fast modulo 23 op required for cyclic buffers of the IPQF */
static const int mod23_lut[26] = {
    23,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
    12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0
};

/* First half of the 384-tap IPQF filtering coefficients. */
static const float ipqf_coeffs1[ATRAC3P_PQF_FIR_LEN][16] = {
    { -5.8336207e-7f,    -8.0604229e-7f,    -4.2005411e-7f,    -4.4400572e-8f,
       3.226247e-8f,      3.530856e-8f,      1.2660377e-8f,     0.000010516783f,
      -0.000011838618f,   6.005389e-7f,      0.0000014333754f,  0.0000023108685f,
       0.0000032569742f,  0.0000046192422f,  0.0000063894258f,  0.0000070302972f },
    { -0.0000091622824f, -0.000010502935f,  -0.0000079212787f, -0.0000041712024f,
      -0.0000026336629f, -0.0000015432918f, -5.7168614e-7f,     0.0000018111954f,
       0.000023530851f,   0.00002780562f,    0.000032302323f,   0.000036968919f,
       0.000041575615f,   0.000045337845f,   0.000046043948f,   0.000048585582f },
    { -0.000064464548f,  -0.000068306952f,  -0.000073081472f,  -0.00007612785f,
      -0.000074850752f,  -0.000070208509f,  -0.000062285151f,  -0.000058270442f,
      -0.000056296329f,  -0.000049888811f,  -0.000035615325f,  -0.000018532943f,
       0.0000016657353f,  0.00002610587f,    0.000053397067f,   0.00008079566f },
    { -0.00054488552f,   -0.00052537228f,   -0.00049731287f,   -0.00045778f,
      -0.00040612387f,   -0.00034301577f,   -0.00026866337f,   -0.00018248901f,
      -0.000084307925f,   0.000025081157f,   0.00014135583f,    0.00026649953f,
       0.00039945057f,    0.00053928449f,    0.00068422867f,    0.00083093712f },
    { -0.0014771431f,    -0.001283227f,     -0.0010566821f,    -0.00079780724f,
      -0.00050782406f,   -0.00018855913f,    0.00015771533f,    0.00052769453f,
       0.00091862219f,    0.001326357f,      0.0017469483f,     0.0021754825f,
       0.0026067684f,     0.0030352892f,     0.0034549395f,     0.0038591374f },
    { -0.0022995141f,    -0.001443546f,     -0.00049266568f,    0.00055068987f,
       0.001682895f,      0.0028992873f,     0.0041943151f,     0.0055614738f,
       0.0069935122f,     0.0084823566f,     0.010018963f,      0.011593862f,
       0.013196872f,      0.014817309f,      0.016444042f,      0.018065533f },
    { -0.034426283f,     -0.034281436f,     -0.033992987f,     -0.033563249f,
      -0.032995768f,     -0.032295227f,     -0.031467363f,     -0.030518902f,
      -0.02945766f,      -0.028291954f,     -0.027031265f,     -0.025685543f,
      -0.024265358f,     -0.022781773f,     -0.021246184f,     -0.019670162f },
    { -0.0030586775f,    -0.0037203205f,    -0.0042847847f,    -0.0047529764f,
      -0.0051268316f,    -0.0054091476f,    -0.0056034233f,    -0.005714261f,
      -0.0057445862f,    -0.0057025906f,    -0.0055920109f,    -0.0054194843f,
      -0.0051914565f,    -0.0049146507f,    -0.0045959447f,    -0.0042418269f },
    { -0.0016376863f,    -0.0017651899f,    -0.0018608454f,    -0.0019252141f,
      -0.0019593791f,    -0.0019653172f,    -0.0019450618f,    -0.0018990048f,
      -0.00183808f,      -0.0017501717f,    -0.0016481078f,    -0.0015320742f,
      -0.0014046903f,    -0.0012685474f,    -0.001125814f,     -0.00097943726f },
    { -0.00055432378f,   -0.00055472925f,   -0.00054783461f,   -0.00053276919f,
      -0.00051135791f,   -0.00048466062f,   -0.00045358928f,   -0.00042499689f,
      -0.00036942671f,   -0.0003392619f,    -0.00030001783f,   -0.00025986304f,
      -0.0002197204f,    -0.00018116167f,   -0.00014691355f,   -0.00011279432f },
    { -0.000064147389f,  -0.00006174868f,   -0.000054267788f,  -0.000047133824f,
      -0.000042927582f,  -0.000039477309f,  -0.000036340745f,  -0.000029687517f,
      -0.000049787737f,  -0.000041577889f,  -0.000033864744f,  -0.000026534748f,
      -0.000019841305f,  -0.000014789486f,  -0.000013131184f,  -0.0000099198869f },
    { -0.0000062990207f, -0.0000072701259f, -0.000011984052f,  -0.000017348082f,
      -0.000019907106f,  -0.000021348773f,  -0.000021961965f,  -0.000012203576f,
      -0.000010840992f,   4.6299544e-7f,     5.2588763e-7f,     2.7792686e-7f,
      -2.3649704e-7f,    -0.0000010897784f, -9.171448e-7f,     -5.22682e-7f }
};

/* Second half of the 384-tap IPQF filtering coefficients. */
static const float ipqf_coeffs2[ATRAC3P_PQF_FIR_LEN][16] = {
    {  5.22682e-7f,       9.171448e-7f,      0.0000010897784f,  2.3649704e-7f,
      -2.7792686e-7f,    -5.2588763e-7f,    -4.6299544e-7f,     0.000010840992f,
      -0.000012203576f,  -0.000021961965f,  -0.000021348773f,  -0.000019907106f,
      -0.000017348082f,  -0.000011984052f,  -0.0000072701259f, -0.0000062990207f },
    {  0.0000099198869f,  0.000013131184f,   0.000014789486f,   0.000019841305f,
       0.000026534748f,   0.000033864744f,   0.000041577889f,   0.000049787737f,
      -0.000029687517f,  -0.000036340745f,  -0.000039477309f,  -0.000042927582f,
      -0.000047133824f,  -0.000054267788f,  -0.00006174868f,   -0.000064147389f },
    {  0.00011279432f,    0.00014691355f,    0.00018116167f,    0.0002197204f,
       0.00025986304f,    0.00030001783f,    0.0003392619f,     0.00036942671f,
      -0.00042499689f,   -0.00045358928f,   -0.00048466062f,   -0.00051135791f,
      -0.00053276919f,   -0.00054783461f,   -0.00055472925f,   -0.00055432378f },
    {  0.00097943726f,    0.001125814f,      0.0012685474f,     0.0014046903f,
       0.0015320742f,     0.0016481078f,     0.0017501717f,     0.00183808f,
      -0.0018990048f,    -0.0019450618f,    -0.0019653172f,    -0.0019593791f,
      -0.0019252141f,    -0.0018608454f,    -0.0017651899f,    -0.0016376863f },
    {  0.0042418269f,     0.0045959447f,     0.0049146507f,     0.0051914565f,
       0.0054194843f,     0.0055920109f,     0.0057025906f,     0.0057445862f,
      -0.005714261f,     -0.0056034233f,    -0.0054091476f,    -0.0051268316f,
      -0.0047529764f,    -0.0042847847f,    -0.0037203205f,    -0.0030586775f },
    {  0.019670162f,      0.021246184f,      0.022781773f,      0.024265358f,
       0.025685543f,      0.027031265f,      0.028291954f,      0.02945766f,
      -0.030518902f,     -0.031467363f,     -0.032295227f,     -0.032995768f,
      -0.033563249f,     -0.033992987f,     -0.034281436f,     -0.034426283f },
    { -0.018065533f,     -0.016444042f,     -0.014817309f,     -0.013196872f,
      -0.011593862f,     -0.010018963f,     -0.0084823566f,    -0.0069935122f,
       0.0055614738f,     0.0041943151f,     0.0028992873f,     0.001682895f,
       0.00055068987f,   -0.00049266568f,   -0.001443546f,     -0.0022995141f },
    { -0.0038591374f,    -0.0034549395f,    -0.0030352892f,    -0.0026067684f,
      -0.0021754825f,    -0.0017469483f,    -0.001326357f,     -0.00091862219f,
       0.00052769453f,    0.00015771533f,   -0.00018855913f,   -0.00050782406f,
      -0.00079780724f,   -0.0010566821f,    -0.001283227f,     -0.0014771431f },
    { -0.00083093712f,   -0.00068422867f,   -0.00053928449f,   -0.00039945057f,
      -0.00026649953f,   -0.00014135583f,   -0.000025081157f,   0.000084307925f,
      -0.00018248901f,   -0.00026866337f,   -0.00034301577f,   -0.00040612387f,
      -0.00045778f,      -0.00049731287f,   -0.00052537228f,   -0.00054488552f },
    { -0.00008079566f,   -0.000053397067f,  -0.00002610587f,   -0.0000016657353f,
       0.000018532943f,   0.000035615325f,   0.000049888811f,   0.000056296329f,
      -0.000058270442f,  -0.000062285151f,  -0.000070208509f,  -0.000074850752f,
      -0.00007612785f,   -0.000073081472f,  -0.000068306952f,  -0.000064464548f },
    { -0.000048585582f,  -0.000046043948f,  -0.000045337845f,  -0.000041575615f,
      -0.000036968919f,  -0.000032302323f,  -0.00002780562f,   -0.000023530851f,
       0.0000018111954f, -5.7168614e-7f,    -0.0000015432918f, -0.0000026336629f,
      -0.0000041712024f, -0.0000079212787f, -0.000010502935f,  -0.0000091622824f },
    { -0.0000070302972f, -0.0000063894258f, -0.0000046192422f, -0.0000032569742f,
      -0.0000023108685f, -0.0000014333754f, -6.005389e-7f,      0.000011838618f,
       0.000010516783f,   1.2660377e-8f,     3.530856e-8f,      3.226247e-8f,
      -4.4400572e-8f,    -4.2005411e-7f,    -8.0604229e-7f,    -5.8336207e-7f }
};

void ff_atrac3p_ipqf(FFTContext *dct_ctx, Atrac3pIPQFChannelCtx *hist,
                     const float *in, float *out)
{
    int i, s, sb, t, pos_now, pos_next;
    float idct_in [ATRAC3P_SUBBANDS];
    float idct_out[ATRAC3P_SUBBANDS];

    memset(out, 0, ATRAC3P_FRAME_SAMPLES * sizeof(*out));

    for (s = 0; s < ATRAC3P_SUBBAND_SAMPLES; s++) {
        /* pick up one sample from each subband */
        for (sb = 0; sb < ATRAC3P_SUBBANDS; sb++)
            idct_in[sb] = in[sb * ATRAC3P_SUBBAND_SAMPLES + s];

        /* Calculate the sine and cosine part of the PQF using IDCT-IV */
        imdct_half(dct_ctx, idct_out, idct_in);

        /* append the result to the history */
        const int hist_pos = hist->pos;
        for (i = 0; i < 8; i++) {
            hist->buf1[hist_pos][i] = idct_out[i + 8];
        }
        for (i = 0; i < 8; i++) {
            hist->buf2[hist_pos][i] = idct_out[7 - i];
        }

        pos_now  = hist->pos;
        pos_next = mod23_lut[pos_now + 2]; // pos_next = (pos_now + 1) % 23;

        for (t = 0; t < ATRAC3P_PQF_FIR_LEN; t++) {
            const float *buf1 = hist->buf1[pos_now];
            const float *buf2 = hist->buf2[pos_next];
            const float *coeffs1 = ipqf_coeffs1[t];
            const float *coeffs2 = ipqf_coeffs2[t];

            float *outp = out + s * 16;
#if 0
            auto _mm_reverse = [](__m128 x) -> __m128 {
                return _mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 1, 2, 3));
            };
            _mm_storeu_ps(outp, _mm_add_ps(_mm_loadu_ps(outp), _mm_add_ps(
                _mm_mul_ps(_mm_loadu_ps(buf1), _mm_loadu_ps(coeffs1)),
                _mm_mul_ps(_mm_loadu_ps(buf2), _mm_loadu_ps(coeffs2)))));
            _mm_storeu_ps(outp + 4, _mm_add_ps(_mm_loadu_ps(outp + 4), _mm_add_ps(
                _mm_mul_ps(_mm_loadu_ps(buf1 + 4), _mm_loadu_ps(coeffs1 + 4)),
                _mm_mul_ps(_mm_loadu_ps(buf2 + 4), _mm_loadu_ps(coeffs2 + 4)))));

            _mm_storeu_ps(outp + 8, _mm_add_ps(_mm_loadu_ps(outp + 8), _mm_add_ps(
                _mm_mul_ps(_mm_reverse(_mm_loadu_ps(buf1 + 4)), _mm_loadu_ps(coeffs1 + 8)),
                _mm_mul_ps(_mm_reverse(_mm_loadu_ps(buf2 + 4)), _mm_loadu_ps(coeffs2 + 8)))));
            _mm_storeu_ps(outp + 12, _mm_add_ps(_mm_loadu_ps(outp + 12), _mm_add_ps(
                _mm_mul_ps(_mm_reverse(_mm_loadu_ps(buf1)), _mm_loadu_ps(coeffs1 + 12)),
                _mm_mul_ps(_mm_reverse(_mm_loadu_ps(buf2)), _mm_loadu_ps(coeffs2 + 12)))));
#elif 0
            auto vreverseq_f32 = [](float32x4_t x) -> float32x4_t {
                float32x4_t rev = vrev64q_f32(x);
                float32x2_t high = vget_high_f32(rev); //{4,3}
                float32x2_t low = vget_low_f32(rev); //{1,2}
                return vcombine_f32(high, low); //{4,3,2,1}
            };
            vst1q_f32(outp, vaddq_f32(vld1q_f32(outp), vaddq_f32(
                vmulq_f32(vld1q_f32(buf1), vld1q_f32(coeffs1)),
                vmulq_f32(vld1q_f32(buf2), vld1q_f32(coeffs2)))));
            vst1q_f32(outp + 4, vaddq_f32(vld1q_f32(outp + 4), vaddq_f32(
                vmulq_f32(vld1q_f32(buf1 + 4), vld1q_f32(coeffs1 + 4)),
                vmulq_f32(vld1q_f32(buf2 + 4), vld1q_f32(coeffs2 + 4)))));

            vst1q_f32(outp + 8, vaddq_f32(vld1q_f32(outp + 8), vaddq_f32(
                vmulq_f32(vreverseq_f32(vld1q_f32(buf1 + 4)), vld1q_f32(coeffs1 + 8)),
                vmulq_f32(vreverseq_f32(vld1q_f32(buf2 + 4)), vld1q_f32(coeffs2 + 8)))));
            vst1q_f32(outp + 12, vaddq_f32(vld1q_f32(outp + 12), vaddq_f32(
                vmulq_f32(vreverseq_f32(vld1q_f32(buf1)), vld1q_f32(coeffs1 + 12)),
                vmulq_f32(vreverseq_f32(vld1q_f32(buf2)), vld1q_f32(coeffs2 + 12)))));
#else
            for (i = 0; i < 8; i++) {
                outp[i] += buf1[i] * coeffs1[i] + buf2[i] * coeffs2[i];
            }
            for (i = 0; i < 8; i++) {
                outp[i + 8] += buf1[7 - i] * coeffs1[i + 8] + buf2[7 - i] * coeffs2[i + 8];
            }
#endif

            pos_now  = mod23_lut[pos_next + 2]; // pos_now  = (pos_now  + 2) % 23;
            pos_next = mod23_lut[pos_now + 2];  // pos_next = (pos_next + 2) % 23;
        }

        hist->pos = mod23_lut[hist->pos]; // hist->pos = (hist->pos - 1) % 23;
    }
}

/* ============================================================================
 * ATRAC3+ Decoder (atrac3plusdec.cpp)
 * ============================================================================ */
/*
 * ATRAC3+ compatible decoder
 *
 * Copyright (c) 2010-2013 Maxim Poliakovski
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1f of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Sony ATRAC3+ compatible decoder.
 *
 * Container formats used to store its data:
 * RIFF WAV (.at3) and Sony OpenMG (.oma, .aa3).
 *
 * Technical description of this codec can be found here:
 * http://wiki.multimedia.cx/index.php?title=ATRAC3plus
 *
 * Kudos to Benjamin Larsson and Michael Karcher
 * for their precious technical help!
 */



struct ATRAC3PContext {
    GetBitContext gb;

    DECLARE_ALIGNED(32, float, samples)[2][ATRAC3P_FRAME_SAMPLES];  ///< quantized MDCT spectrum
    DECLARE_ALIGNED(32, float, mdct_buf)[2][ATRAC3P_FRAME_SAMPLES]; ///< output of the IMDCT
    DECLARE_ALIGNED(32, float, time_buf)[2][ATRAC3P_FRAME_SAMPLES]; ///< output of the gain compensation
    DECLARE_ALIGNED(32, float, outp_buf)[2][ATRAC3P_FRAME_SAMPLES];

    AtracGCContext gainc_ctx;   ///< gain compensation context
    FFTContext mdct_ctx;
    FFTContext ipqf_dct_ctx;    ///< IDCT context used by IPQF

    Atrac3pChanUnitCtx *ch_units;   ///< global channel units

    int num_channel_blocks;     ///< number of channel blocks
    uint8_t channel_blocks[5];  ///< channel configuration descriptor

    int block_align;
};
typedef struct ATRAC3PContext ATRAC3PContext;

void atrac3p_free(ATRAC3PContext *ctx)
{
    av_freep(&ctx->ch_units);
    ff_mdct_end(&ctx->mdct_ctx);
    ff_mdct_end(&ctx->ipqf_dct_ctx);
    av_freep(&ctx);
}

static int set_channel_params(ATRAC3PContext *ctx, int channels) {
    memset(ctx->channel_blocks, 0, sizeof(ctx->channel_blocks));
    switch (channels) {
    case 1:
        ctx->num_channel_blocks = 1;
        ctx->channel_blocks[0]  = CH_UNIT_MONO;
        break;
    case 2:
        ctx->num_channel_blocks = 1;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        break;
    case 3:
        ctx->num_channel_blocks = 2;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        break;
    case 4:
        ctx->num_channel_blocks = 3;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        ctx->channel_blocks[2]  = CH_UNIT_MONO;
        break;
    case 6:
        ctx->num_channel_blocks = 4;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        ctx->channel_blocks[2]  = CH_UNIT_STEREO;
        ctx->channel_blocks[3]  = CH_UNIT_MONO;
        break;
    case 7:
        ctx->num_channel_blocks = 5;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        ctx->channel_blocks[2]  = CH_UNIT_STEREO;
        ctx->channel_blocks[3]  = CH_UNIT_MONO;
        ctx->channel_blocks[4]  = CH_UNIT_MONO;
        break;
    case 8:
        ctx->num_channel_blocks = 5;
        ctx->channel_blocks[0]  = CH_UNIT_STEREO;
        ctx->channel_blocks[1]  = CH_UNIT_MONO;
        ctx->channel_blocks[2]  = CH_UNIT_STEREO;
        ctx->channel_blocks[3]  = CH_UNIT_STEREO;
        ctx->channel_blocks[4]  = CH_UNIT_MONO;
        break;
    default:
        av_log(AV_LOG_ERROR,
               "Unsupported channel count: %d!\n", channels);
        return AVERROR_INVALIDDATA;
    }
    return 0;
}

ATRAC3PContext *atrac3p_alloc(int channels, int *block_align) {
    int i, ch, ret;

    ATRAC3PContext *ctx = (ATRAC3PContext *)av_mallocz(sizeof(ATRAC3PContext));
	ctx->block_align = *block_align;

	if (!*block_align) {
		// No block align was passed in, using the default.
		*block_align = 0x000002e8;
	}

    ff_atrac3p_init_vlcs();

    /* initialize IPQF */
    ff_mdct_init(&ctx->ipqf_dct_ctx, 5, 1, 32.0f / 32768.0f);

    ff_atrac3p_init_imdct(&ctx->mdct_ctx);

    ff_atrac_init_gain_compensation(&ctx->gainc_ctx, 6, 2);

    ff_atrac3p_init_wave_synth();

    if ((ret = set_channel_params(ctx, channels)) < 0) {
        atrac3p_free(ctx);
        return NULL;
    }

    ctx->ch_units = (Atrac3pChanUnitCtx *)av_mallocz_array(ctx->num_channel_blocks, sizeof(*ctx->ch_units));

    if (!ctx->ch_units) {
        atrac3p_free(ctx);
        return NULL;
    }

    for (i = 0; i < ctx->num_channel_blocks; i++) {
        for (ch = 0; ch < 2; ch++) {
            ctx->ch_units[i].channels[ch].ch_num          = ch;
            ctx->ch_units[i].channels[ch].wnd_shape       = &ctx->ch_units[i].channels[ch].wnd_shape_hist[0][0];
            ctx->ch_units[i].channels[ch].wnd_shape_prev  = &ctx->ch_units[i].channels[ch].wnd_shape_hist[1][0];
            ctx->ch_units[i].channels[ch].gain_data       = &ctx->ch_units[i].channels[ch].gain_data_hist[0][0];
            ctx->ch_units[i].channels[ch].gain_data_prev  = &ctx->ch_units[i].channels[ch].gain_data_hist[1][0];
            ctx->ch_units[i].channels[ch].tones_info      = &ctx->ch_units[i].channels[ch].tones_info_hist[0][0];
            ctx->ch_units[i].channels[ch].tones_info_prev = &ctx->ch_units[i].channels[ch].tones_info_hist[1][0];
        }

        ctx->ch_units[i].waves_info      = &ctx->ch_units[i].wave_synth_hist[0];
        ctx->ch_units[i].waves_info_prev = &ctx->ch_units[i].wave_synth_hist[1];
    }

    return ctx;
}

static void decode_residual_spectrum(Atrac3pChanUnitCtx *ctx,
                                     float out[2][ATRAC3P_FRAME_SAMPLES],
                                     int num_channels)
{
    int i, sb, ch, qu, nspeclines, RNG_index;
    float *dst, q;
    int16_t *src;
    /* calculate RNG table index for each subband */
    int sb_RNG_index[ATRAC3P_SUBBANDS] = { 0 };

    if (ctx->mute_flag) {
        for (ch = 0; ch < num_channels; ch++)
            memset(out[ch], 0, ATRAC3P_FRAME_SAMPLES * sizeof(*out[ch]));
        return;
    }

    for (qu = 0, RNG_index = 0; qu < ctx->used_quant_units; qu++)
        RNG_index += ctx->channels[0].qu_sf_idx[qu] +
                     ctx->channels[1].qu_sf_idx[qu];

    for (sb = 0; sb < ctx->num_coded_subbands; sb++, RNG_index += 128)
        sb_RNG_index[sb] = RNG_index & 0x3FC;

    /* inverse quant and power compensation */
    for (ch = 0; ch < num_channels; ch++) {
        /* clear channel's residual spectrum */
        memset(out[ch], 0, ATRAC3P_FRAME_SAMPLES * sizeof(*out[ch]));

        for (qu = 0; qu < ctx->used_quant_units; qu++) {
            src        = &ctx->channels[ch].spectrum[av_atrac3p_qu_to_spec_pos[qu]];
            dst        = &out[ch][av_atrac3p_qu_to_spec_pos[qu]];
            nspeclines = av_atrac3p_qu_to_spec_pos[qu + 1] -
                         av_atrac3p_qu_to_spec_pos[qu];

            if (ctx->channels[ch].qu_wordlen[qu] > 0) {
                q = av_atrac3p_sf_tab[ctx->channels[ch].qu_sf_idx[qu]] *
                    av_atrac3p_mant_tab[ctx->channels[ch].qu_wordlen[qu]];
                for (i = 0; i < nspeclines; i++)
                    dst[i] = src[i] * q;
            }
        }

        for (sb = 0; sb < ctx->num_coded_subbands; sb++)
            ff_atrac3p_power_compensation(ctx, ch, &out[ch][0],
                                          sb_RNG_index[sb], sb);
    }

    if (ctx->unit_type == CH_UNIT_STEREO) {
        for (sb = 0; sb < ctx->num_coded_subbands; sb++) {
            if (ctx->swap_channels[sb]) {
                for (i = 0; i < ATRAC3P_SUBBAND_SAMPLES; i++)
                    FFSWAP(float, out[0][sb * ATRAC3P_SUBBAND_SAMPLES + i],
                                  out[1][sb * ATRAC3P_SUBBAND_SAMPLES + i]);
            }

            /* flip coefficients' sign if requested */
            if (ctx->negate_coeffs[sb])
                for (i = 0; i < ATRAC3P_SUBBAND_SAMPLES; i++)
                    out[1][sb * ATRAC3P_SUBBAND_SAMPLES + i] = -(out[1][sb * ATRAC3P_SUBBAND_SAMPLES + i]);
        }
    }
}

static void reconstruct_frame(ATRAC3PContext *ctx, Atrac3pChanUnitCtx *ch_unit,
                              int num_channels)
{
    int ch, sb;

    for (ch = 0; ch < num_channels; ch++) {
        for (sb = 0; sb < ch_unit->num_subbands; sb++) {
            /* inverse transform and windowing */
            ff_atrac3p_imdct(&ctx->mdct_ctx,
                             &ctx->samples[ch][sb * ATRAC3P_SUBBAND_SAMPLES],
                             &ctx->mdct_buf[ch][sb * ATRAC3P_SUBBAND_SAMPLES],
                             (ch_unit->channels[ch].wnd_shape_prev[sb] << 1) +
                             ch_unit->channels[ch].wnd_shape[sb], sb);

            /* gain compensation and overlapping */
            ff_atrac_gain_compensation(&ctx->gainc_ctx,
                                       &ctx->mdct_buf[ch][sb * ATRAC3P_SUBBAND_SAMPLES],
                                       &ch_unit->prev_buf[ch][sb * ATRAC3P_SUBBAND_SAMPLES],
                                       &ch_unit->channels[ch].gain_data_prev[sb],
                                       &ch_unit->channels[ch].gain_data[sb],
                                       ATRAC3P_SUBBAND_SAMPLES,
                                       &ctx->time_buf[ch][sb * ATRAC3P_SUBBAND_SAMPLES]);
        }

        /* zero unused subbands in both output and overlapping buffers */
        memset(&ch_unit->prev_buf[ch][ch_unit->num_subbands * ATRAC3P_SUBBAND_SAMPLES],
               0,
               (ATRAC3P_SUBBANDS - ch_unit->num_subbands) *
               ATRAC3P_SUBBAND_SAMPLES *
               sizeof(ch_unit->prev_buf[ch][ch_unit->num_subbands * ATRAC3P_SUBBAND_SAMPLES]));
        memset(&ctx->time_buf[ch][ch_unit->num_subbands * ATRAC3P_SUBBAND_SAMPLES],
               0,
               (ATRAC3P_SUBBANDS - ch_unit->num_subbands) *
               ATRAC3P_SUBBAND_SAMPLES *
               sizeof(ctx->time_buf[ch][ch_unit->num_subbands * ATRAC3P_SUBBAND_SAMPLES]));

        /* resynthesize and add tonal signal */
        if (ch_unit->waves_info->tones_present ||
            ch_unit->waves_info_prev->tones_present) {
            for (sb = 0; sb < ch_unit->num_subbands; sb++)
                if (ch_unit->channels[ch].tones_info[sb].num_wavs ||
                    ch_unit->channels[ch].tones_info_prev[sb].num_wavs) {
                    ff_atrac3p_generate_tones(ch_unit, ch, sb,
                                              &ctx->time_buf[ch][sb * 128]);
                }
        }

        /* subband synthesis and acoustic signal output */
        ff_atrac3p_ipqf(&ctx->ipqf_dct_ctx, &ch_unit->ipqf_ctx[ch],
                        &ctx->time_buf[ch][0], &ctx->outp_buf[ch][0]);
    }

    /* swap window shape and gain control buffers. */
    for (ch = 0; ch < num_channels; ch++) {
        FFSWAP(uint8_t *, ch_unit->channels[ch].wnd_shape,
               ch_unit->channels[ch].wnd_shape_prev);
        FFSWAP(AtracGainInfo *, ch_unit->channels[ch].gain_data,
               ch_unit->channels[ch].gain_data_prev);
        FFSWAP(Atrac3pWavesData *, ch_unit->channels[ch].tones_info,
               ch_unit->channels[ch].tones_info_prev);
    }

    FFSWAP(Atrac3pWaveSynthParams *, ch_unit->waves_info, ch_unit->waves_info_prev);
}

int atrac3p_decode_frame(ATRAC3PContext *ctx, float *out_data[2], int *nb_samples, const uint8_t *indata, int indata_size)
{
    int i, ret, ch_unit_id, ch_block = 0, out_ch_index = 0, channels_to_process;
	float **samples_p = out_data;

    *nb_samples = 0;

    if ((ret = init_get_bits8(&ctx->gb, indata, indata_size)) < 0)
        return ret;

    if (get_bits1(&ctx->gb)) {
        av_log(AV_LOG_ERROR, "Invalid start bit!");
        return AVERROR_INVALIDDATA;
    }

    while (get_bits_left(&ctx->gb) >= 2 &&
           (ch_unit_id = get_bits(&ctx->gb, 2)) != CH_UNIT_TERMINATOR) {
        if (ch_unit_id == CH_UNIT_EXTENSION) {
            av_log(AV_LOG_ERROR, "Missing atrac3p feature: Channel unit extension");
            return AVERROR_PATCHWELCOME;
        }
        if (ch_block >= ctx->num_channel_blocks ||
            ctx->channel_blocks[ch_block] != ch_unit_id) {
            av_log(AV_LOG_WARNING, "Frame data doesn't match channel configuration! ch_block %d >= num_channel_blocks %d", ch_block, ctx->num_channel_blocks);
            // We used to have trouble in Code Lyoko and other games here. It's usually because data is corrupted
            // or the wrong channel configuration is used..
            return AVERROR_INVALIDDATA;
        }

        ctx->ch_units[ch_block].unit_type = ch_unit_id;
        channels_to_process               = ch_unit_id + 1;

        if ((ret = ff_atrac3p_decode_channel_unit(&ctx->gb,
                                                  &ctx->ch_units[ch_block],
                                                  channels_to_process)) < 0)
            return ret;

        decode_residual_spectrum(&ctx->ch_units[ch_block], ctx->samples,
                                 channels_to_process);
        reconstruct_frame(ctx, &ctx->ch_units[ch_block],
                          channels_to_process);

        for (i = 0; i < channels_to_process; i++)
            memcpy(samples_p[out_ch_index + i], ctx->outp_buf[i],
                   ATRAC3P_FRAME_SAMPLES * sizeof(**samples_p));

        ch_block++;
        out_ch_index += channels_to_process;
    }

    *nb_samples = ATRAC3P_FRAME_SAMPLES;
    return FFMIN(ctx->block_align, indata_size);
}

void atrac3p_flush_buffers(ATRAC3PContext *ctx) {
	// TODO: Not sure what should be zeroed here.
}

/*******************************************************************************
 * dr_libs Style High-Level API Implementation
 ******************************************************************************/

/* Allocation helpers */
static void* drat3__malloc(size_t sz, const drat3_allocation_callbacks* pAlloc) {
    if (pAlloc && pAlloc->onMalloc) {
        return pAlloc->onMalloc(sz, pAlloc->pUserData);
    }
    return malloc(sz);
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static void* drat3__realloc(void* p, size_t sz, const drat3_allocation_callbacks* pAlloc) {
    if (pAlloc && pAlloc->onRealloc) {
        return pAlloc->onRealloc(p, sz, pAlloc->pUserData);
    }
    return realloc(p, sz);
}

static void drat3__free(void* p, const drat3_allocation_callbacks* pAlloc) {
    if (p == NULL) {
        return;
    }
    if (pAlloc && pAlloc->onFree) {
        pAlloc->onFree(p, pAlloc->pUserData);
        return;
    }
    free(p);
}

/* Float to int16 conversion */
static drat3_int16 drat3__f32_to_s16(float x) {
    x = x * 32767.0f;
    if (x < -32768.0f) x = -32768.0f;
    if (x >  32767.0f) x =  32767.0f;
    return (drat3_int16)x;
}

/* Internal init helper */
static drat3_result drat3__init_internal(drat3* pAt3, drat3_container* pContainer, const drat3_config* pConfig) {
    const drat3_container_info* pInfo;
    int block_align;
    
    if (pAt3 == NULL || pContainer == NULL) {
        return DRAT3_INVALID_ARGS;
    }
    
    memset(pAt3, 0, sizeof(*pAt3));
    
    if (pConfig) {
        pAt3->allocationCallbacks = pConfig->allocationCallbacks;
    }
    
    pInfo = drat3_container_get_info(pContainer);
    if (pInfo == NULL) {
        drat3_container_close(pContainer);
        return DRAT3_INVALID_FILE;
    }
    
    pAt3->pContainer = pContainer;
    pAt3->channels = pInfo->channels;
    pAt3->sampleRate = pInfo->sample_rate;
    pAt3->totalPCMFrameCount = pInfo->total_samples;
    pAt3->codecType = pInfo->codec_type;
    pAt3->currentPCMFrame = 0;
    
    /* Allocate frame buffer */
    pAt3->frameBufferSize = pInfo->block_align + 16;
    pAt3->pFrameBuffer = (drat3_uint8*)drat3__malloc(pAt3->frameBufferSize, &pAt3->allocationCallbacks);
    if (pAt3->pFrameBuffer == NULL) {
        drat3_container_close(pContainer);
        return DRAT3_OUT_OF_MEMORY;
    }
    
    /* Allocate decode buffer (enough for one frame, interleaved) */
    pAt3->decodeBufferCapacity = pInfo->samples_per_frame;
    pAt3->pDecodeBuffer = (float*)drat3__malloc(pAt3->decodeBufferCapacity * pAt3->channels * sizeof(float), &pAt3->allocationCallbacks);
    if (pAt3->pDecodeBuffer == NULL) {
        drat3__free(pAt3->pFrameBuffer, &pAt3->allocationCallbacks);
        drat3_container_close(pContainer);
        return DRAT3_OUT_OF_MEMORY;
    }
    
    /* Initialize appropriate decoder */
    block_align = (int)pInfo->block_align;
    if (pInfo->codec_type == DRAT3_CODEC_ATRAC3) {
        pAt3->pDecoder = atrac3_alloc((int)pInfo->channels, &block_align, pInfo->extradata, pInfo->extradata_size);
    } else if (pInfo->codec_type == DRAT3_CODEC_ATRAC3P) {
        pAt3->pDecoder = atrac3p_alloc((int)pInfo->channels, &block_align);
    } else {
        drat3__free(pAt3->pDecodeBuffer, &pAt3->allocationCallbacks);
        drat3__free(pAt3->pFrameBuffer, &pAt3->allocationCallbacks);
        drat3_container_close(pContainer);
        return DRAT3_INVALID_FILE;
    }
    
    if (pAt3->pDecoder == NULL) {
        drat3__free(pAt3->pDecodeBuffer, &pAt3->allocationCallbacks);
        drat3__free(pAt3->pFrameBuffer, &pAt3->allocationCallbacks);
        drat3_container_close(pContainer);
        return DRAT3_OUT_OF_MEMORY;
    }
    
    return DRAT3_SUCCESS;
}

#ifndef DR_AT3_NO_STDIO
drat3_result drat3_init_file(const char* pFilePath, const drat3_config* pConfig, drat3* pAt3) {
    drat3_container* pContainer;
    
    if (pFilePath == NULL || pAt3 == NULL) {
        return DRAT3_INVALID_ARGS;
    }
    
    pContainer = drat3_container_open_file(pFilePath);
    if (pContainer == NULL) {
        return DRAT3_INVALID_FILE;
    }
    
    return drat3__init_internal(pAt3, pContainer, pConfig);
}
#endif

drat3_result drat3_init_memory(const void* pData, size_t dataSize, const drat3_config* pConfig, drat3* pAt3) {
    drat3_container* pContainer;
    
    if (pData == NULL || dataSize == 0 || pAt3 == NULL) {
        return DRAT3_INVALID_ARGS;
    }
    
    pContainer = drat3_container_open_memory(pData, dataSize);
    if (pContainer == NULL) {
        return DRAT3_INVALID_FILE;
    }
    
    return drat3__init_internal(pAt3, pContainer, pConfig);
}

void drat3_uninit(drat3* pAt3) {
    if (pAt3 == NULL) {
        return;
    }
    
    if (pAt3->pDecoder) {
        if (pAt3->codecType == DRAT3_CODEC_ATRAC3) {
            atrac3_free((struct ATRAC3Context*)pAt3->pDecoder);
        } else if (pAt3->codecType == DRAT3_CODEC_ATRAC3P) {
            atrac3p_free((struct ATRAC3PContext*)pAt3->pDecoder);
        }
    }
    
    drat3__free(pAt3->pDecodeBuffer, &pAt3->allocationCallbacks);
    drat3__free(pAt3->pFrameBuffer, &pAt3->allocationCallbacks);
    
    if (pAt3->pContainer) {
        drat3_container_close(pAt3->pContainer);
    }
    
    memset(pAt3, 0, sizeof(*pAt3));
}

drat3_uint64 drat3_read_pcm_frames_f32(drat3* pAt3, drat3_uint64 framesToRead, float* pBufferOut) {
    drat3_uint64 totalFramesRead = 0;
    
    if (pAt3 == NULL || framesToRead == 0) {
        return 0;
    }
    
    /* First, consume any leftover frames from previous decode */
    if (pAt3->leftoverFrames > 0) {
        drat3_uint64 framesToConsume = pAt3->leftoverFrames;
        if (framesToConsume > framesToRead) {
            framesToConsume = framesToRead;
        }
        
        if (pBufferOut != NULL) {
            memcpy(pBufferOut, 
                   pAt3->pDecodeBuffer + pAt3->leftoverOffset * pAt3->channels,
                   (size_t)(framesToConsume * pAt3->channels * sizeof(float)));
            pBufferOut += framesToConsume * pAt3->channels;
        }
        
        pAt3->leftoverOffset += (drat3_uint32)framesToConsume;
        pAt3->leftoverFrames -= (drat3_uint32)framesToConsume;
        pAt3->currentPCMFrame += framesToConsume;
        totalFramesRead += framesToConsume;
        framesToRead -= framesToConsume;
    }
    
    /* Decode more frames as needed */
    while (framesToRead > 0) {
        int bytesRead;
        int nbSamples = 0;
        float* outPtrs[2];
        drat3_uint32 i;
        
        /* Read next encoded frame */
        bytesRead = drat3_container_read_frame(pAt3->pContainer, pAt3->pFrameBuffer, pAt3->frameBufferSize);
        if (bytesRead <= 0) {
            break; /* End of stream or error */
        }
        
        /* Decode the frame */
        outPtrs[0] = pAt3->pDecodeBuffer;
        outPtrs[1] = pAt3->pDecodeBuffer + pAt3->decodeBufferCapacity;
        
        if (pAt3->codecType == DRAT3_CODEC_ATRAC3) {
            if (atrac3_decode_frame((struct ATRAC3Context*)pAt3->pDecoder, outPtrs, &nbSamples, pAt3->pFrameBuffer, bytesRead) < 0) {
                break;
            }
        } else {
            if (atrac3p_decode_frame((struct ATRAC3PContext*)pAt3->pDecoder, outPtrs, &nbSamples, pAt3->pFrameBuffer, bytesRead) < 0) {
                break;
            }
        }
        
        if (nbSamples <= 0) {
            continue;
        }
        
        /* Interleave the output (decoder outputs planar) */
        if (pAt3->channels == 2) {
            float* dst = pAt3->pDecodeBuffer;
            float* src0 = outPtrs[0];
            float* src1 = outPtrs[1];
            /* We need a temp buffer since we're interleaving in-place */
            for (i = 0; i < (drat3_uint32)nbSamples; i++) {
                float s0 = src0[i];
                float s1 = src1[i];
                dst[i * 2 + 0] = s0;
                dst[i * 2 + 1] = s1;
            }
        }
        /* For mono, data is already in place */
        
        /* How many frames can we deliver now? */
        {
            drat3_uint64 framesToDeliver = (drat3_uint32)nbSamples;
            if (framesToDeliver > framesToRead) {
                framesToDeliver = framesToRead;
            }
            
            if (pBufferOut != NULL) {
                memcpy(pBufferOut, pAt3->pDecodeBuffer, (size_t)(framesToDeliver * pAt3->channels * sizeof(float)));
                pBufferOut += framesToDeliver * pAt3->channels;
            }
            
            pAt3->currentPCMFrame += framesToDeliver;
            totalFramesRead += framesToDeliver;
            framesToRead -= framesToDeliver;
            
            /* Store leftovers */
            if ((drat3_uint32)nbSamples > framesToDeliver) {
                pAt3->leftoverFrames = (drat3_uint32)nbSamples - (drat3_uint32)framesToDeliver;
                pAt3->leftoverOffset = (drat3_uint32)framesToDeliver;
            } else {
                pAt3->leftoverFrames = 0;
                pAt3->leftoverOffset = 0;
            }
        }
    }
    
    return totalFramesRead;
}

drat3_uint64 drat3_read_pcm_frames_s16(drat3* pAt3, drat3_uint64 framesToRead, drat3_int16* pBufferOut) {
    drat3_uint64 totalFramesRead = 0;
    float tempBuffer[2048];
    drat3_uint64 tempFrames = sizeof(tempBuffer) / sizeof(float) / 2; /* Assume max 2 channels */
    
    if (pAt3 == NULL || framesToRead == 0) {
        return 0;
    }
    
    if (pAt3->channels > 0) {
        tempFrames = sizeof(tempBuffer) / sizeof(float) / pAt3->channels;
    }
    
    while (framesToRead > 0) {
        drat3_uint64 framesToReadNow = framesToRead;
        drat3_uint64 framesRead;
        drat3_uint64 i;
        
        if (framesToReadNow > tempFrames) {
            framesToReadNow = tempFrames;
        }
        
        framesRead = drat3_read_pcm_frames_f32(pAt3, framesToReadNow, tempBuffer);
        if (framesRead == 0) {
            break;
        }
        
        /* Convert to s16 */
        if (pBufferOut != NULL) {
            for (i = 0; i < framesRead * pAt3->channels; i++) {
                pBufferOut[i] = drat3__f32_to_s16(tempBuffer[i]);
            }
            pBufferOut += framesRead * pAt3->channels;
        }
        
        totalFramesRead += framesRead;
        framesToRead -= framesRead;
    }
    
    return totalFramesRead;
}

drat3_result drat3_seek_to_pcm_frame(drat3* pAt3, drat3_uint64 targetPCMFrameIndex) {
    const drat3_container_info* pInfo;
    drat3_uint64 targetFrameIndex;
    drat3_uint64 targetFrameOffset;
    
    if (pAt3 == NULL) {
        return DRAT3_INVALID_ARGS;
    }
    
    pInfo = drat3_container_get_info(pAt3->pContainer);
    if (pInfo == NULL) {
        return DRAT3_ERROR;
    }
    
    if (targetPCMFrameIndex >= pAt3->totalPCMFrameCount) {
        targetPCMFrameIndex = pAt3->totalPCMFrameCount;
    }
    
    /* Calculate which encoded frame contains this PCM frame */
    targetFrameIndex = targetPCMFrameIndex / pInfo->samples_per_frame;
    targetFrameOffset = targetPCMFrameIndex % pInfo->samples_per_frame;
    
    /* Seek to that encoded frame */
    if (drat3_container_seek_frame(pAt3->pContainer, targetFrameIndex) != 0) {
        return DRAT3_BAD_SEEK;
    }
    
    /* Clear leftover buffer */
    pAt3->leftoverFrames = 0;
    pAt3->leftoverOffset = 0;
    
    /* Flush decoder */
    if (pAt3->codecType == DRAT3_CODEC_ATRAC3) {
        atrac3_flush_buffers((struct ATRAC3Context*)pAt3->pDecoder);
    } else {
        atrac3p_flush_buffers((struct ATRAC3PContext*)pAt3->pDecoder);
    }
    
    pAt3->currentPCMFrame = targetFrameIndex * pInfo->samples_per_frame;
    
    /* Skip samples to reach exact position */
    if (targetFrameOffset > 0) {
        drat3_read_pcm_frames_f32(pAt3, targetFrameOffset, NULL);
    }
    
    return DRAT3_SUCCESS;
}

drat3_uint64 drat3_get_cursor_in_pcm_frames(drat3* pAt3) {
    if (pAt3 == NULL) {
        return 0;
    }
    return pAt3->currentPCMFrame;
}

drat3_uint64 drat3_get_length_in_pcm_frames(drat3* pAt3) {
    if (pAt3 == NULL) {
        return 0;
    }
    return pAt3->totalPCMFrameCount;
}

/*
 * High-Level API Implementation
 */

#ifndef DR_AT3_NO_STDIO
float* drat3_open_file_and_read_pcm_frames_f32(const char* pFilePath, drat3_uint32* pChannels, drat3_uint32* pSampleRate, drat3_uint64* pTotalPCMFrameCount, const drat3_allocation_callbacks* pAllocationCallbacks) {
    drat3 at3;
    drat3_config config;
    float* pSampleData;
    drat3_uint64 totalFramesRead;
    
    if (pChannels) *pChannels = 0;
    if (pSampleRate) *pSampleRate = 0;
    if (pTotalPCMFrameCount) *pTotalPCMFrameCount = 0;
    
    memset(&config, 0, sizeof(config));
    if (pAllocationCallbacks) {
        config.allocationCallbacks = *pAllocationCallbacks;
    }
    
    if (drat3_init_file(pFilePath, &config, &at3) != DRAT3_SUCCESS) {
        return NULL;
    }
    
    pSampleData = (float*)drat3__malloc((size_t)(at3.totalPCMFrameCount * at3.channels * sizeof(float)), pAllocationCallbacks);
    if (pSampleData == NULL) {
        drat3_uninit(&at3);
        return NULL;
    }
    
    totalFramesRead = drat3_read_pcm_frames_f32(&at3, at3.totalPCMFrameCount, pSampleData);
    
    if (pChannels) *pChannels = at3.channels;
    if (pSampleRate) *pSampleRate = at3.sampleRate;
    if (pTotalPCMFrameCount) *pTotalPCMFrameCount = totalFramesRead;
    
    drat3_uninit(&at3);
    
    return pSampleData;
}

drat3_int16* drat3_open_file_and_read_pcm_frames_s16(const char* pFilePath, drat3_uint32* pChannels, drat3_uint32* pSampleRate, drat3_uint64* pTotalPCMFrameCount, const drat3_allocation_callbacks* pAllocationCallbacks) {
    drat3 at3;
    drat3_config config;
    drat3_int16* pSampleData;
    drat3_uint64 totalFramesRead;
    
    if (pChannels) *pChannels = 0;
    if (pSampleRate) *pSampleRate = 0;
    if (pTotalPCMFrameCount) *pTotalPCMFrameCount = 0;
    
    memset(&config, 0, sizeof(config));
    if (pAllocationCallbacks) {
        config.allocationCallbacks = *pAllocationCallbacks;
    }
    
    if (drat3_init_file(pFilePath, &config, &at3) != DRAT3_SUCCESS) {
        return NULL;
    }
    
    pSampleData = (drat3_int16*)drat3__malloc((size_t)(at3.totalPCMFrameCount * at3.channels * sizeof(drat3_int16)), pAllocationCallbacks);
    if (pSampleData == NULL) {
        drat3_uninit(&at3);
        return NULL;
    }
    
    totalFramesRead = drat3_read_pcm_frames_s16(&at3, at3.totalPCMFrameCount, pSampleData);
    
    if (pChannels) *pChannels = at3.channels;
    if (pSampleRate) *pSampleRate = at3.sampleRate;
    if (pTotalPCMFrameCount) *pTotalPCMFrameCount = totalFramesRead;
    
    drat3_uninit(&at3);
    
    return pSampleData;
}
#endif

float* drat3_open_memory_and_read_pcm_frames_f32(const void* pData, size_t dataSize, drat3_uint32* pChannels, drat3_uint32* pSampleRate, drat3_uint64* pTotalPCMFrameCount, const drat3_allocation_callbacks* pAllocationCallbacks) {
    drat3 at3;
    drat3_config config;
    float* pSampleData;
    drat3_uint64 totalFramesRead;
    
    if (pChannels) *pChannels = 0;
    if (pSampleRate) *pSampleRate = 0;
    if (pTotalPCMFrameCount) *pTotalPCMFrameCount = 0;
    
    memset(&config, 0, sizeof(config));
    if (pAllocationCallbacks) {
        config.allocationCallbacks = *pAllocationCallbacks;
    }
    
    if (drat3_init_memory(pData, dataSize, &config, &at3) != DRAT3_SUCCESS) {
        return NULL;
    }
    
    pSampleData = (float*)drat3__malloc((size_t)(at3.totalPCMFrameCount * at3.channels * sizeof(float)), pAllocationCallbacks);
    if (pSampleData == NULL) {
        drat3_uninit(&at3);
        return NULL;
    }
    
    totalFramesRead = drat3_read_pcm_frames_f32(&at3, at3.totalPCMFrameCount, pSampleData);
    
    if (pChannels) *pChannels = at3.channels;
    if (pSampleRate) *pSampleRate = at3.sampleRate;
    if (pTotalPCMFrameCount) *pTotalPCMFrameCount = totalFramesRead;
    
    drat3_uninit(&at3);
    
    return pSampleData;
}

drat3_int16* drat3_open_memory_and_read_pcm_frames_s16(const void* pData, size_t dataSize, drat3_uint32* pChannels, drat3_uint32* pSampleRate, drat3_uint64* pTotalPCMFrameCount, const drat3_allocation_callbacks* pAllocationCallbacks) {
    drat3 at3;
    drat3_config config;
    drat3_int16* pSampleData;
    drat3_uint64 totalFramesRead;
    
    if (pChannels) *pChannels = 0;
    if (pSampleRate) *pSampleRate = 0;
    if (pTotalPCMFrameCount) *pTotalPCMFrameCount = 0;
    
    memset(&config, 0, sizeof(config));
    if (pAllocationCallbacks) {
        config.allocationCallbacks = *pAllocationCallbacks;
    }
    
    if (drat3_init_memory(pData, dataSize, &config, &at3) != DRAT3_SUCCESS) {
        return NULL;
    }
    
    pSampleData = (drat3_int16*)drat3__malloc((size_t)(at3.totalPCMFrameCount * at3.channels * sizeof(drat3_int16)), pAllocationCallbacks);
    if (pSampleData == NULL) {
        drat3_uninit(&at3);
        return NULL;
    }
    
    totalFramesRead = drat3_read_pcm_frames_s16(&at3, at3.totalPCMFrameCount, pSampleData);
    
    if (pChannels) *pChannels = at3.channels;
    if (pSampleRate) *pSampleRate = at3.sampleRate;
    if (pTotalPCMFrameCount) *pTotalPCMFrameCount = totalFramesRead;
    
    drat3_uninit(&at3);
    
    return pSampleData;
}

void drat3_free(void* p, const drat3_allocation_callbacks* pAllocationCallbacks) {
    drat3__free(p, pAllocationCallbacks);
}

#endif /* DR_AT3_IMPLEMENTATION */

#endif /* DR_AT3_H */
