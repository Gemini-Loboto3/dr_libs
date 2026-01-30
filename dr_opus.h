/*
dr_opus.h - Single-header Opus audio decoder
Based on libopus 1.4

USAGE:
    #define DR_OPUS_IMPLEMENTATION
    #include "dr_opus.h"
    
    // Simple API - decode entire .opus file:
    drlibs_uint64 frameCount;
    int channels, sampleRate;
    float* samples = dr_opus_open_file_and_read_pcm_frames_f32(
        "file.opus", &channels, &sampleRate, &frameCount, NULL);
    dr_opus_free(samples, NULL);
    
    // Streaming API:
    dr_opus opus;
    dr_opus_init_file("file.opus", NULL, &opus);
    float buffer[4096];
    drlibs_uint64 read;
    while ((read = dr_opus_read_pcm_frames_f32(&opus, 1024, buffer)) > 0) {
        // process samples
    }
    dr_opus_uninit(&opus);

LICENSE: BSD (libopus) + Public Domain (Ogg parsing/dr_libs wrapper)
*/
#ifndef DR_OPUS_H
#define DR_OPUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/*******************************************************************************
 * COMPILER COMPATIBILITY
 ******************************************************************************/

/* MSVC compatibility */
#ifdef _MSC_VER
    #define DR_OPUS_INLINE __inline
    #define DR_OPUS_RESTRICT __restrict
    #ifndef __attribute__
        #define __attribute__(x)
    #endif
    #ifndef __restrict
        #define __restrict
    #endif
    /* Count leading zeros - MSVC version */
    #include <intrin.h>
    static __inline int dr_opus__clz(unsigned int x) {
        unsigned long index;
        if (_BitScanReverse(&index, x)) return 31 - (int)index;
        return 32;
    }
    #define __builtin_clz(x) dr_opus__clz(x)
    /* Stack allocation for MSVC (no VLA support) */
    #include <malloc.h>
    #define DR_OPUS_ALLOCA(size) _alloca(size)
#else
    #define DR_OPUS_ALLOCA(size) __builtin_alloca(size)
    #define DR_OPUS_INLINE inline
    #define DR_OPUS_RESTRICT restrict
#endif

/* Visibility attribute - not needed for static library */
#ifdef __GNUC__
    #define OPUS_EXPORT __attribute__ ((visibility ("default")))
#else
    #define OPUS_EXPORT
#endif

/* dr_libs style types */
typedef int8_t   drlibs_int8;
typedef uint8_t  drlibs_uint8;
typedef int16_t  drlibs_int16;
typedef uint16_t drlibs_uint16;
typedef int32_t  drlibs_int32;
typedef uint32_t drlibs_uint32;
typedef int64_t  drlibs_int64;
typedef uint64_t drlibs_uint64;
typedef drlibs_uint8  drlibs_bool8;
typedef drlibs_uint32 drlibs_bool32;
#define DRLIBS_TRUE  1
#define DRLIBS_FALSE 0

/* libopus types */
typedef int8_t   opus_int8;
typedef uint8_t  opus_uint8;
typedef int16_t  opus_int16;
typedef uint16_t opus_uint16;
typedef int32_t  opus_int32;
typedef uint32_t opus_uint32;
typedef int64_t  opus_int64;
typedef uint64_t opus_uint64;
typedef float    opus_val16;
typedef float    opus_val32;
typedef float    opus_val64;

typedef enum {
    DR_OPUS_SUCCESS = 0,
    DR_OPUS_ERROR = -1,
    DR_OPUS_INVALID_ARGS = -2,
    DR_OPUS_INVALID_FILE = -3,
    DR_OPUS_OUT_OF_MEMORY = -4
} dr_opus_result;

typedef struct {
    void* pUserData;
    void* (*onMalloc)(size_t sz, void* pUserData);
    void* (*onRealloc)(void* p, size_t sz, void* pUserData);
    void  (*onFree)(void* p, void* pUserData);
} dr_opus_allocation_callbacks;

/* Forward declaration for internal libopus decoder */
typedef struct OpusDecoder OpusDecoder;

/* Main dr_opus structure */
typedef struct {
    drlibs_uint32 channels;
    drlibs_uint32 sampleRate;
    drlibs_uint32 inputSampleRate;
    drlibs_uint16 preSkip;
    drlibs_int16  outputGain;
    drlibs_uint64 totalPCMFrameCount;
    
    /* Internal */
    OpusDecoder* pOpusDecoder;
    void* pFile;
    drlibs_uint8* pMemory;
    size_t memorySize;
    size_t memoryPos;
    dr_opus_allocation_callbacks alloc;
    
    /* Ogg state */
    drlibs_uint32 oggSerial;
    drlibs_bool32 oggHeadersRead;
    drlibs_uint8 oggPageBuffer[65307];
    drlibs_uint32 oggPageDataSize;
    drlibs_uint32 oggPageDataPos;
    drlibs_uint8 oggSegmentTable[255];
    drlibs_uint8 oggSegmentCount;
    drlibs_uint8 oggCurrentSegment;
    drlibs_int64 oggGranulePosition;
    
    /* Decode buffer */
    float decodeBuffer[5760*2];
    drlibs_uint32 decodeBufferLen;
    drlibs_uint32 decodeBufferPos;
    drlibs_uint32 preSkipRemaining;
} dr_opus;

/* Init/Uninit */
dr_opus_result dr_opus_init_file(const char* pFilePath, const dr_opus_allocation_callbacks* pAlloc, dr_opus* pOpus);
dr_opus_result dr_opus_init_memory(const void* pData, size_t dataSize, const dr_opus_allocation_callbacks* pAlloc, dr_opus* pOpus);
void dr_opus_uninit(dr_opus* pOpus);

/* Reading */
drlibs_uint64 dr_opus_read_pcm_frames_f32(dr_opus* pOpus, drlibs_uint64 framesToRead, float* pBufferOut);
drlibs_uint64 dr_opus_read_pcm_frames_s16(dr_opus* pOpus, drlibs_uint64 framesToRead, drlibs_int16* pBufferOut);

/* High-level API */
float* dr_opus_open_file_and_read_pcm_frames_f32(const char* pFilePath, int* pChannels, int* pSampleRate, drlibs_uint64* pTotalFrameCount, const dr_opus_allocation_callbacks* pAlloc);
drlibs_int16* dr_opus_open_file_and_read_pcm_frames_s16(const char* pFilePath, int* pChannels, int* pSampleRate, drlibs_uint64* pTotalFrameCount, const dr_opus_allocation_callbacks* pAlloc);
float* dr_opus_open_memory_and_read_pcm_frames_f32(const void* pData, size_t dataSize, int* pChannels, int* pSampleRate, drlibs_uint64* pTotalFrameCount, const dr_opus_allocation_callbacks* pAlloc);
drlibs_int16* dr_opus_open_memory_and_read_pcm_frames_s16(const void* pData, size_t dataSize, int* pChannels, int* pSampleRate, drlibs_uint64* pTotalFrameCount, const dr_opus_allocation_callbacks* pAlloc);
void dr_opus_free(void* p, const dr_opus_allocation_callbacks* pAlloc);

/* Low-level libopus API */
int opus_decoder_get_size(int channels);
OpusDecoder* opus_decoder_create(opus_int32 Fs, int channels, int *error);
int opus_decode(OpusDecoder *st, const unsigned char *data, opus_int32 len, opus_int16 *pcm, int frame_size, int decode_fec);
int opus_decode_float(OpusDecoder *st, const unsigned char *data, opus_int32 len, float *pcm, int frame_size, int decode_fec);
int opus_decoder_ctl(OpusDecoder *st, int request, ...);
void opus_decoder_destroy(OpusDecoder *st);
int opus_packet_get_bandwidth(const unsigned char *data);
int opus_packet_get_samples_per_frame(const unsigned char *data, opus_int32 Fs);
int opus_packet_get_nb_channels(const unsigned char *data);
int opus_packet_get_nb_frames(const unsigned char packet[], opus_int32 len);
int opus_packet_get_nb_samples(const unsigned char packet[], opus_int32 len, opus_int32 Fs);
const char* opus_strerror(int error);
const char* opus_get_version_string(void);

#define OPUS_OK 0
#define OPUS_BAD_ARG -1
#define OPUS_BUFFER_TOO_SMALL -2
#define OPUS_INTERNAL_ERROR -3
#define OPUS_INVALID_PACKET -4
#define OPUS_RESET_STATE 4028

#ifdef __cplusplus
}
#endif
#endif /* DR_OPUS_H */

/*******************************************************************************
 * IMPLEMENTATION
 ******************************************************************************/
#ifdef DR_OPUS_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdarg.h>

/* Memory helpers */
static void* dr_opus__malloc(size_t sz, const dr_opus_allocation_callbacks* pAlloc) {
    if (pAlloc && pAlloc->onMalloc) return pAlloc->onMalloc(sz, pAlloc->pUserData);
    return malloc(sz);
}
static void* dr_opus__realloc(void* p, size_t sz, const dr_opus_allocation_callbacks* pAlloc) {
    if (pAlloc && pAlloc->onRealloc) return pAlloc->onRealloc(p, sz, pAlloc->pUserData);
    return realloc(p, sz);
}
static void dr_opus__free(void* p, const dr_opus_allocation_callbacks* pAlloc) {
    if (pAlloc && pAlloc->onFree) { pAlloc->onFree(p, pAlloc->pUserData); return; }
    free(p);
}

/*******************************************************************************
 * EMBEDDED LIBOPUS 1.4 DECODER
 ******************************************************************************/
typedef float celt_sig;
typedef float celt_norm;
typedef float celt_ener;

extern const opus_uint32 SMALL_DIV_TABLE[129];







typedef opus_uint32 ec_window;
typedef struct ec_ctx ec_ctx;
typedef struct ec_ctx ec_enc;
typedef struct ec_ctx ec_dec;
struct ec_ctx{

   unsigned char *buf;

   opus_uint32 storage;

   opus_uint32 end_offs;

   ec_window end_window;

   int nend_bits;


   int nbits_total;

   opus_uint32 offs;

   opus_uint32 rng;



   opus_uint32 val;


   opus_uint32 ext;

   int rem;

   int error;
};

static inline opus_uint32 ec_range_bytes(ec_ctx *_this){
  return _this->offs;
}

static inline unsigned char *ec_get_buffer(ec_ctx *_this){
  return _this->buf;
}

static inline int ec_get_error(ec_ctx *_this){
  return _this->error;
}







static inline int ec_tell(ec_ctx *_this){
  return _this->nbits_total-(((int)sizeof(unsigned)*8
                           )-(__builtin_clz(_this->rng)));
}







opus_uint32 ec_tell_frac(ec_ctx *_this);


static inline opus_uint32 celt_udiv(opus_uint32 n, opus_uint32 d) {
   ;
   return n/d;

}

static inline opus_int32 celt_sudiv(opus_int32 n, opus_int32 d) {
   ;






   return n/d;

}
opus_uint32 ec_tell_frac(ec_ctx *_this){
  static const unsigned correction[8] =
    {35733, 38967, 42495, 46340,
     50535, 55109, 60097, 65535};
  opus_uint32 nbits;
  opus_uint32 r;
  int l;
  unsigned b;
  nbits=_this->nbits_total<<3;
  l=(((int)sizeof(unsigned)*8
   )-(__builtin_clz(_this->rng)));
  r=_this->rng>>(l-16);
  b = (r>>12)-8;
  b += r>correction[b];
  l = (l<<3)+b;
  return nbits-l;
}







static inline void *opus_alloc (size_t size)
{
   return malloc(size);
}




static inline void *opus_alloc_scratch (size_t size)
{

   return opus_alloc(size);
}




static inline void opus_free (void *ptr)
{
   free(ptr);
}





void ec_dec_init(ec_dec *_this,unsigned char *_buf,opus_uint32 _storage);
unsigned ec_decode(ec_dec *_this,unsigned _ft);


unsigned ec_decode_bin(ec_dec *_this,unsigned _bits);
void ec_dec_update(ec_dec *_this,unsigned _fl,unsigned _fh,unsigned _ft);


int ec_dec_bit_logp(ec_dec *_this,unsigned _logp);
int ec_dec_icdf(ec_dec *_this,const unsigned char *_icdf,unsigned _ftb);







opus_uint32 ec_dec_uint(ec_dec *_this,opus_uint32 _ft);







opus_uint32 ec_dec_bits(ec_dec *_this,unsigned _ftb);
static int ec_read_byte(ec_dec *_this){
  return _this->offs<_this->storage?_this->buf[_this->offs++]:0;
}

static int ec_read_byte_from_end(ec_dec *_this){
  return _this->end_offs<_this->storage?
   _this->buf[_this->storage-++(_this->end_offs)]:0;
}



static void ec_dec_normalize(ec_dec *_this){

  while(_this->rng<=((((opus_uint32)1U)<<((32)-1))>>(8))){
    int sym;
    _this->nbits_total+=(8);
    _this->rng<<=(8);

    sym=_this->rem;

    _this->rem=ec_read_byte(_this);

    sym=(sym<<(8)|_this->rem)>>((8)-(((32)-2)%(8)+1));

    _this->val=((_this->val<<(8))+(((1U<<(8))-1)&~sym))&((((opus_uint32)1U)<<((32)-1))-1);
  }
}

void ec_dec_init(ec_dec *_this,unsigned char *_buf,opus_uint32 _storage){
  _this->buf=_buf;
  _this->storage=_storage;
  _this->end_offs=0;
  _this->end_window=0;
  _this->nend_bits=0;



  _this->nbits_total=(32)+1
   -(((32)-(((32)-2)%(8)+1))/(8))*(8);
  _this->offs=0;
  _this->rng=1U<<(((32)-2)%(8)+1);
  _this->rem=ec_read_byte(_this);
  _this->val=_this->rng-1-(_this->rem>>((8)-(((32)-2)%(8)+1)));
  _this->error=0;

  ec_dec_normalize(_this);
}

unsigned ec_decode(ec_dec *_this,unsigned _ft){
  unsigned s;
  _this->ext=celt_udiv(_this->rng,_ft);
  s=(unsigned)(_this->val/_this->ext);
  return _ft-((s+1)+(((_ft)-(s+1))&-((_ft)<(s+1))));
}

unsigned ec_decode_bin(ec_dec *_this,unsigned _bits){
   unsigned s;
   _this->ext=_this->rng>>_bits;
   s=(unsigned)(_this->val/_this->ext);
   return (1U<<_bits)-((s+1U)+(((1U<<_bits)-(s+1U))&-((1U<<_bits)<(s+1U))));
}

void ec_dec_update(ec_dec *_this,unsigned _fl,unsigned _fh,unsigned _ft){
  opus_uint32 s;
  s=((_this->ext)*(_ft-_fh));
  _this->val-=s;
  _this->rng=_fl>0?((_this->ext)*(_fh-_fl)):_this->rng-s;
  ec_dec_normalize(_this);
}


int ec_dec_bit_logp(ec_dec *_this,unsigned _logp){
  opus_uint32 r;
  opus_uint32 d;
  opus_uint32 s;
  int ret;
  r=_this->rng;
  d=_this->val;
  s=r>>_logp;
  ret=d<s;
  if(!ret)_this->val=d-s;
  _this->rng=ret?s:r-s;
  ec_dec_normalize(_this);
  return ret;
}

int ec_dec_icdf(ec_dec *_this,const unsigned char *_icdf,unsigned _ftb){
  opus_uint32 r;
  opus_uint32 d;
  opus_uint32 s;
  opus_uint32 t;
  int ret;
  s=_this->rng;
  d=_this->val;
  r=s>>_ftb;
  ret=-1;
  do{
    t=s;
    s=((r)*(_icdf[++ret]));
  }
  while(d<s);
  _this->val=d-s;
  _this->rng=t-s;
  ec_dec_normalize(_this);
  return ret;
}

opus_uint32 ec_dec_uint(ec_dec *_this,opus_uint32 _ft){
  unsigned ft;
  unsigned s;
  int ftb;

  ;
  _ft--;
  ftb=(((int)sizeof(unsigned)*8
     )-(__builtin_clz(_ft)));
  if(ftb>(8)){
    opus_uint32 t;
    ftb-=(8);
    ft=(unsigned)(_ft>>ftb)+1;
    s=ec_decode(_this,ft);
    ec_dec_update(_this,s,s+1,ft);
    t=(opus_uint32)s<<ftb|ec_dec_bits(_this,ftb);
    if(t<=_ft)return t;
    _this->error=1;
    return _ft;
  }
  else{
    _ft++;
    s=ec_decode(_this,(unsigned)_ft);
    ec_dec_update(_this,s,s+1,(unsigned)_ft);
    return s;
  }
}

opus_uint32 ec_dec_bits(ec_dec *_this,unsigned _bits){
  ec_window window;
  int available;
  opus_uint32 ret;
  window=_this->end_window;
  available=_this->nend_bits;
  if((unsigned)available<_bits){
    do{
      window|=(ec_window)ec_read_byte_from_end(_this)<<available;
      available+=(8);
    }
    while(available<=((int)sizeof(ec_window)*8
                    )-(8));
  }
  ret=(opus_uint32)window&(((opus_uint32)1<<_bits)-1U);
  window>>=_bits;
  available-=_bits;
  _this->end_window=window;
  _this->nend_bits=available;
  _this->nbits_total+=_bits;
  return ret;
}




void ec_enc_init(ec_enc *_this,unsigned char *_buf,opus_uint32 _size);
void ec_encode(ec_enc *_this,unsigned _fl,unsigned _fh,unsigned _ft);


void ec_encode_bin(ec_enc *_this,unsigned _fl,unsigned _fh,unsigned _bits);


void ec_enc_bit_logp(ec_enc *_this,int _val,unsigned _logp);
void ec_enc_icdf(ec_enc *_this,int _s,const unsigned char *_icdf,unsigned _ftb);





void ec_enc_uint(ec_enc *_this,opus_uint32 _fl,opus_uint32 _ft);





void ec_enc_bits(ec_enc *_this,opus_uint32 _fl,unsigned _ftb);
void ec_enc_patch_initial_bits(ec_enc *_this,unsigned _val,unsigned _nbits);
void ec_enc_shrink(ec_enc *_this,opus_uint32 _size);




void ec_enc_done(ec_enc *_this);
static int ec_write_byte(ec_enc *_this,unsigned _value){
  if(_this->offs+_this->end_offs>=_this->storage)return -1;
  _this->buf[_this->offs++]=(unsigned char)_value;
  return 0;
}

static int ec_write_byte_at_end(ec_enc *_this,unsigned _value){
  if(_this->offs+_this->end_offs>=_this->storage)return -1;
  _this->buf[_this->storage-++(_this->end_offs)]=(unsigned char)_value;
  return 0;
}
static void ec_enc_carry_out(ec_enc *_this,int _c){
  if(_c!=((1U<<(8))-1)){

    int carry;
    carry=_c>>(8);


    if(_this->rem>=0)_this->error|=ec_write_byte(_this,_this->rem+carry);
    if(_this->ext>0){
      unsigned sym;
      sym=(((1U<<(8))-1)+carry)&((1U<<(8))-1);
      do _this->error|=ec_write_byte(_this,sym);
      while(--(_this->ext)>0);
    }
    _this->rem=_c&((1U<<(8))-1);
  }
  else _this->ext++;
}

static inline void ec_enc_normalize(ec_enc *_this){

  while(_this->rng<=((((opus_uint32)1U)<<((32)-1))>>(8))){
    ec_enc_carry_out(_this,(int)(_this->val>>((32)-(8)-1)));

    _this->val=(_this->val<<(8))&((((opus_uint32)1U)<<((32)-1))-1);
    _this->rng<<=(8);
    _this->nbits_total+=(8);
  }
}

void ec_enc_init(ec_enc *_this,unsigned char *_buf,opus_uint32 _size){
  _this->buf=_buf;
  _this->end_offs=0;
  _this->end_window=0;
  _this->nend_bits=0;

  _this->nbits_total=(32)+1;
  _this->offs=0;
  _this->rng=(((opus_uint32)1U)<<((32)-1));
  _this->rem=-1;
  _this->val=0;
  _this->ext=0;
  _this->storage=_size;
  _this->error=0;
}

void ec_encode(ec_enc *_this,unsigned _fl,unsigned _fh,unsigned _ft){
  opus_uint32 r;
  r=celt_udiv(_this->rng,_ft);
  if(_fl>0){
    _this->val+=_this->rng-((r)*((_ft-_fl)));
    _this->rng=((r)*((_fh-_fl)));
  }
  else _this->rng-=((r)*((_ft-_fh)));
  ec_enc_normalize(_this);
}

void ec_encode_bin(ec_enc *_this,unsigned _fl,unsigned _fh,unsigned _bits){
  opus_uint32 r;
  r=_this->rng>>_bits;
  if(_fl>0){
    _this->val+=_this->rng-((r)*(((1U<<_bits)-_fl)));
    _this->rng=((r)*((_fh-_fl)));
  }
  else _this->rng-=((r)*(((1U<<_bits)-_fh)));
  ec_enc_normalize(_this);
}


void ec_enc_bit_logp(ec_enc *_this,int _val,unsigned _logp){
  opus_uint32 r;
  opus_uint32 s;
  opus_uint32 l;
  r=_this->rng;
  l=_this->val;
  s=r>>_logp;
  r-=s;
  if(_val)_this->val=l+r;
  _this->rng=_val?s:r;
  ec_enc_normalize(_this);
}

void ec_enc_icdf(ec_enc *_this,int _s,const unsigned char *_icdf,unsigned _ftb){
  opus_uint32 r;
  r=_this->rng>>_ftb;
  if(_s>0){
    _this->val+=_this->rng-((r)*(_icdf[_s-1]));
    _this->rng=((r)*(_icdf[_s-1]-_icdf[_s]));
  }
  else _this->rng-=((r)*(_icdf[_s]));
  ec_enc_normalize(_this);
}

void ec_enc_uint(ec_enc *_this,opus_uint32 _fl,opus_uint32 _ft){
  unsigned ft;
  unsigned fl;
  int ftb;

  ;
  _ft--;
  ftb=(((int)sizeof(unsigned)*8
     )-(__builtin_clz(_ft)));
  if(ftb>(8)){
    ftb-=(8);
    ft=(_ft>>ftb)+1;
    fl=(unsigned)(_fl>>ftb);
    ec_encode(_this,fl,fl+1,ft);
    ec_enc_bits(_this,_fl&(((opus_uint32)1<<ftb)-1U),ftb);
  }
  else ec_encode(_this,_fl,_fl+1,_ft+1);
}

void ec_enc_bits(ec_enc *_this,opus_uint32 _fl,unsigned _bits){
  ec_window window;
  int used;
  window=_this->end_window;
  used=_this->nend_bits;
  ;
  if(used+_bits>((int)sizeof(ec_window)*8
               )){
    do{
      _this->error|=ec_write_byte_at_end(_this,(unsigned)window&((1U<<(8))-1));
      window>>=(8);
      used-=(8);
    }
    while(used>=(8));
  }
  window|=(ec_window)_fl<<used;
  used+=_bits;
  _this->end_window=window;
  _this->nend_bits=used;
  _this->nbits_total+=_bits;
}

void ec_enc_patch_initial_bits(ec_enc *_this,unsigned _val,unsigned _nbits){
  int shift;
  unsigned mask;
  ;
  shift=(8)-_nbits;
  mask=((1<<_nbits)-1)<<shift;
  if(_this->offs>0){

    _this->buf[0]=(unsigned char)((_this->buf[0]&~mask)|_val<<shift);
  }
  else if(_this->rem>=0){

    _this->rem=(_this->rem&~mask)|_val<<shift;
  }
  else if(_this->rng<=((((opus_uint32)1U)<<((32)-1))>>_nbits)){

    _this->val=(_this->val&~((opus_uint32)mask<<((32)-(8)-1)))|
     (opus_uint32)_val<<(((32)-(8)-1)+shift);
  }

  else _this->error=-1;
}

void ec_enc_shrink(ec_enc *_this,opus_uint32 _size){
  ;
  (memmove((_this->buf+_size-_this->end_offs), (_this->buf+_this->storage-_this->end_offs), (_this->end_offs)*sizeof(*(_this->buf+_size-_this->end_offs)) + 0*((_this->buf+_size-_this->end_offs)-(_this->buf+_this->storage-_this->end_offs)) ))
                                                             ;
  _this->storage=_size;
}

void ec_enc_done(ec_enc *_this){
  ec_window window;
  int used;
  opus_uint32 msk;
  opus_uint32 end;
  int l;


  l=(32)-(((int)sizeof(unsigned)*8
                )-(__builtin_clz(_this->rng)));
  msk=((((opus_uint32)1U)<<((32)-1))-1)>>l;
  end=(_this->val+msk)&~msk;
  if((end|msk)>=_this->val+_this->rng){
    l++;
    msk>>=1;
    end=(_this->val+msk)&~msk;
  }
  while(l>0){
    ec_enc_carry_out(_this,(int)(end>>((32)-(8)-1)));
    end=(end<<(8))&((((opus_uint32)1U)<<((32)-1))-1);
    l-=(8);
  }

  if(_this->rem>=0||_this->ext>0)ec_enc_carry_out(_this,0);

  window=_this->end_window;
  used=_this->nend_bits;
  while(used>=(8)){
    _this->error|=ec_write_byte_at_end(_this,(unsigned)window&((1U<<(8))-1));
    window>>=(8);
    used-=(8);
  }

  if(!_this->error){
    (memset((_this->buf+_this->offs), 0, (_this->storage-_this->offs-_this->end_offs)*sizeof(*(_this->buf+_this->offs))))
                                                ;
    if(used>0){

      if(_this->end_offs>=_this->storage)_this->error=-1;
      else{
        l=-l;


        if(_this->offs+_this->end_offs>=_this->storage&&l<used){
          window&=(1<<l)-1;
          _this->error=-1;
        }
        _this->buf[_this->storage-_this->end_offs-1]|=(unsigned char)window;
      }
    }
  }
}
void ec_laplace_encode(ec_enc *enc, int *value, unsigned fs, int decay);
int ec_laplace_decode(ec_dec *dec, unsigned fs, int decay);







unsigned isqrt32(opus_uint32 _val);







static inline float fast_atan2f(float y, float x) {
   float x2, y2;
   x2 = x*x;
   y2 = y*y;


   if (x2 + y2 < 1e-18f)
   {
      return 0;
   }
   if(x2<y2){
      float den = (y2 + 0.67848403f*x2) * (y2 + 0.08595542f*x2);
      return -x*y*(y2 + 0.43157974f*x2) / den + (y<0 ? -((float)3.141592653f/2) : ((float)3.141592653f/2));
   }else{
      float den = (x2 + 0.67848403f*y2) * (x2 + 0.08595542f*y2);
      return x*y*(x2 + 0.43157974f*y2) / den + (y<0 ? -((float)3.141592653f/2) : ((float)3.141592653f/2)) - (x*y<0 ? -((float)3.141592653f/2) : ((float)3.141592653f/2));
   }
}
static inline opus_val32 celt_maxabs16(const opus_val16 *x, int len)
{
   int i;
   opus_val16 maxval = 0;
   opus_val16 minval = 0;
   for (i=0;i<len;i++)
   {
      maxval = ((maxval) > (x[i]) ? (maxval) : (x[i]));
      minval = ((minval) < (x[i]) ? (minval) : (x[i]));
   }
   return (((maxval)) > (-(minval)) ? ((maxval)) : (-(minval)));
}
static unsigned ec_laplace_get_freq1(unsigned fs0, int decay)
{
   unsigned ft;
   ft = 32768 - (1<<(0))*(2*(16)) - fs0;
   return ft*(opus_int32)(16384-decay)>>15;
}

void ec_laplace_encode(ec_enc *enc, int *value, unsigned fs, int decay)
{
   unsigned fl;
   int val = *value;
   fl = 0;
   if (val)
   {
      int s;
      int i;
      s = -(val<0);
      val = (val+s)^s;
      fl = fs;
      fs = ec_laplace_get_freq1(fs, decay);

      for (i=1; fs > 0 && i < val; i++)
      {
         fs *= 2;
         fl += fs+2*(1<<(0));
         fs = (fs*(opus_int32)decay)>>15;
      }

      if (!fs)
      {
         int di;
         int ndi_max;
         ndi_max = (32768-fl+(1<<(0))-1)>>(0);
         ndi_max = (ndi_max-s)>>1;
         di = ((val - i) < (ndi_max - 1) ? (val - i) : (ndi_max - 1));
         fl += (2*di+1+s)*(1<<(0));
         fs = (((1<<(0))) < (32768-fl) ? ((1<<(0))) : (32768-fl));
         *value = (i+di+s)^s;
      }
      else
      {
         fs += (1<<(0));
         fl += fs&~s;
      }
      ;
      ;
   }
   ec_encode_bin(enc, fl, fl+fs, 15);
}

int ec_laplace_decode(ec_dec *dec, unsigned fs, int decay)
{
   int val=0;
   unsigned fl;
   unsigned fm;
   fm = ec_decode_bin(dec, 15);
   fl = 0;
   if (fm >= fs)
   {
      val++;
      fl = fs;
      fs = ec_laplace_get_freq1(fs, decay)+(1<<(0));

      while(fs > (1<<(0)) && fm >= fl+2*fs)
      {
         fs *= 2;
         fl += fs;
         fs = ((fs-2*(1<<(0)))*(opus_int32)decay)>>15;
         fs += (1<<(0));
         val++;
      }

      if (fs <= (1<<(0)))
      {
         int di;
         di = (fm-fl)>>((0)+1);
         val += di;
         fl += 2*di*(1<<(0));
      }
      if (fm < fl+fs)
         val = -val;
      else
         fl += fs;
   }
   ;
   ;
   ;
   ;
   ec_dec_update(dec, fl, ((fl+fs) < (32768) ? (fl+fs) : (32768)), 32768);
   return val;
}
static inline int _opus_false(void) {return 0;}




int log2_frac(opus_uint32 val, int frac);


void get_required_bits(opus_int16 *bits, int N, int K, int frac);

void encode_pulses(const int *_y, int N, int K, ec_enc *enc);

opus_val32 decode_pulses(int *_y, int N, int K, ec_dec *dec);
int log2_frac(opus_uint32 val, int frac)
{
  int l;
  l=(((int)sizeof(unsigned)*8
   )-(__builtin_clz(val)));
  if(val&(val-1)){



    if(l>16)val=((val-1)>>(l-16))+1;
    else val<<=16-l;
    l=(l-1)<<frac;


    do{
      int b;
      b=(int)(val>>16);
      l+=b<<frac;
      val=(val+b)>>b;
      val=(val*val+0x7FFF)>>15;
    }
    while(frac-->0);

    return l+(val>0x8000);
  }

  else return (l-1)<<frac;
}
static const opus_uint32 CELT_PVQ_U_DATA[1488]={




  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,


  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0,


  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,


  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1,


  3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41,
  43, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 69, 71, 73, 75, 77, 79,
  81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113,
  115, 117, 119, 121, 123, 125, 127, 129, 131, 133, 135, 137, 139, 141, 143,
  145, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169, 171, 173,
  175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201, 203,
  205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 227, 229, 231, 233,
  235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255, 257, 259, 261, 263,
  265, 267, 269, 271, 273, 275, 277, 279, 281, 283, 285, 287, 289, 291, 293,
  295, 297, 299, 301, 303, 305, 307, 309, 311, 313, 315, 317, 319, 321, 323,
  325, 327, 329, 331, 333, 335, 337, 339, 341, 343, 345, 347, 349, 351,


  353, 355, 357, 359, 361, 363, 365, 367, 369, 371, 373, 375, 377, 379, 381,
  383, 385, 387, 389, 391, 393, 395, 397, 399, 401, 403, 405, 407, 409, 411,
  413, 415,


  13, 25, 41, 61, 85, 113, 145, 181, 221, 265, 313, 365, 421, 481, 545, 613,
  685, 761, 841, 925, 1013, 1105, 1201, 1301, 1405, 1513, 1625, 1741, 1861,
  1985, 2113, 2245, 2381, 2521, 2665, 2813, 2965, 3121, 3281, 3445, 3613, 3785,
  3961, 4141, 4325, 4513, 4705, 4901, 5101, 5305, 5513, 5725, 5941, 6161, 6385,
  6613, 6845, 7081, 7321, 7565, 7813, 8065, 8321, 8581, 8845, 9113, 9385, 9661,
  9941, 10225, 10513, 10805, 11101, 11401, 11705, 12013, 12325, 12641, 12961,
  13285, 13613, 13945, 14281, 14621, 14965, 15313, 15665, 16021, 16381, 16745,
  17113, 17485, 17861, 18241, 18625, 19013, 19405, 19801, 20201, 20605, 21013,
  21425, 21841, 22261, 22685, 23113, 23545, 23981, 24421, 24865, 25313, 25765,
  26221, 26681, 27145, 27613, 28085, 28561, 29041, 29525, 30013, 30505, 31001,
  31501, 32005, 32513, 33025, 33541, 34061, 34585, 35113, 35645, 36181, 36721,
  37265, 37813, 38365, 38921, 39481, 40045, 40613, 41185, 41761, 42341, 42925,
  43513, 44105, 44701, 45301, 45905, 46513, 47125, 47741, 48361, 48985, 49613,
  50245, 50881, 51521, 52165, 52813, 53465, 54121, 54781, 55445, 56113, 56785,
  57461, 58141, 58825, 59513, 60205, 60901, 61601,


  62305, 63013, 63725, 64441, 65161, 65885, 66613, 67345, 68081, 68821, 69565,
  70313, 71065, 71821, 72581, 73345, 74113, 74885, 75661, 76441, 77225, 78013,
  78805, 79601, 80401, 81205, 82013, 82825, 83641, 84461, 85285, 86113,


  63, 129, 231, 377, 575, 833, 1159, 1561, 2047, 2625, 3303, 4089, 4991, 6017,
  7175, 8473, 9919, 11521, 13287, 15225, 17343, 19649, 22151, 24857, 27775,
  30913, 34279, 37881, 41727, 45825, 50183, 54809, 59711, 64897, 70375, 76153,
  82239, 88641, 95367, 102425, 109823, 117569, 125671, 134137, 142975, 152193,
  161799, 171801, 182207, 193025, 204263, 215929, 228031, 240577, 253575,
  267033, 280959, 295361, 310247, 325625, 341503, 357889, 374791, 392217,
  410175, 428673, 447719, 467321, 487487, 508225, 529543, 551449, 573951,
  597057, 620775, 645113, 670079, 695681, 721927, 748825, 776383, 804609,
  833511, 863097, 893375, 924353, 956039, 988441, 1021567, 1055425, 1090023,
  1125369, 1161471, 1198337, 1235975, 1274393, 1313599, 1353601, 1394407,
  1436025, 1478463, 1521729, 1565831, 1610777, 1656575, 1703233, 1750759,
  1799161, 1848447, 1898625, 1949703, 2001689, 2054591, 2108417, 2163175,
  2218873, 2275519, 2333121, 2391687, 2451225, 2511743, 2573249, 2635751,
  2699257, 2763775, 2829313, 2895879, 2963481, 3032127, 3101825, 3172583,
  3244409, 3317311, 3391297, 3466375, 3542553, 3619839, 3698241, 3777767,
  3858425, 3940223, 4023169, 4107271, 4192537, 4278975, 4366593, 4455399,
  4545401, 4636607, 4729025, 4822663, 4917529, 5013631, 5110977, 5209575,
  5309433, 5410559, 5512961, 5616647, 5721625, 5827903, 5935489, 6044391,
  6154617, 6266175, 6379073, 6493319, 6608921, 6725887, 6844225, 6963943,
  7085049, 7207551,


  7331457, 7456775, 7583513, 7711679, 7841281, 7972327, 8104825, 8238783,
  8374209, 8511111, 8649497, 8789375, 8930753, 9073639, 9218041, 9363967,
  9511425, 9660423, 9810969, 9963071, 10116737, 10271975, 10428793, 10587199,
  10747201, 10908807, 11072025, 11236863, 11403329, 11571431, 11741177,
  11912575,


  321, 681, 1289, 2241, 3649, 5641, 8361, 11969, 16641, 22569, 29961, 39041,
  50049, 63241, 78889, 97281, 118721, 143529, 172041, 204609, 241601, 283401,
  330409, 383041, 441729, 506921, 579081, 658689, 746241, 842249, 947241,
  1061761, 1186369, 1321641, 1468169, 1626561, 1797441, 1981449, 2179241,
  2391489, 2618881, 2862121, 3121929, 3399041, 3694209, 4008201, 4341801,
  4695809, 5071041, 5468329, 5888521, 6332481, 6801089, 7295241, 7815849,
  8363841, 8940161, 9545769, 10181641, 10848769, 11548161, 12280841, 13047849,
  13850241, 14689089, 15565481, 16480521, 17435329, 18431041, 19468809,
  20549801, 21675201, 22846209, 24064041, 25329929, 26645121, 28010881,
  29428489, 30899241, 32424449, 34005441, 35643561, 37340169, 39096641,
  40914369, 42794761, 44739241, 46749249, 48826241, 50971689, 53187081,
  55473921, 57833729, 60268041, 62778409, 65366401, 68033601, 70781609,
  73612041, 76526529, 79526721, 82614281, 85790889, 89058241, 92418049,
  95872041, 99421961, 103069569, 106816641, 110664969, 114616361, 118672641,
  122835649, 127107241, 131489289, 135983681, 140592321, 145317129, 150160041,
  155123009, 160208001, 165417001, 170752009, 176215041, 181808129, 187533321,
  193392681, 199388289, 205522241, 211796649, 218213641, 224775361, 231483969,
  238341641, 245350569, 252512961, 259831041, 267307049, 274943241, 282741889,
  290705281, 298835721, 307135529, 315607041, 324252609, 333074601, 342075401,
  351257409, 360623041, 370174729, 379914921, 389846081, 399970689, 410291241,
  420810249, 431530241, 442453761, 453583369, 464921641, 476471169, 488234561,
  500214441, 512413449, 524834241, 537479489, 550351881, 563454121, 576788929,
  590359041, 604167209, 618216201, 632508801,


  647047809, 661836041, 676876329, 692171521, 707724481, 723538089, 739615241,
  755958849, 772571841, 789457161, 806617769, 824056641, 841776769, 859781161,
  878072841, 896654849, 915530241, 934702089, 954173481, 973947521, 994027329,
  1014416041, 1035116809, 1056132801, 1077467201, 1099123209, 1121104041,
  1143412929, 1166053121, 1189027881, 1212340489, 1235994241,


  1683, 3653, 7183, 13073, 22363, 36365, 56695, 85305, 124515, 177045, 246047,
  335137, 448427, 590557, 766727, 982729, 1244979, 1560549, 1937199, 2383409,
  2908411, 3522221, 4235671, 5060441, 6009091, 7095093, 8332863, 9737793,
  11326283, 13115773, 15124775, 17372905, 19880915, 22670725, 25765455,
  29189457, 32968347, 37129037, 41699767, 46710137, 52191139, 58175189,
  64696159, 71789409, 79491819, 87841821, 96879431, 106646281, 117185651,
  128542501, 140763503, 153897073, 167993403, 183104493, 199284183, 216588185,
  235074115, 254801525, 275831935, 298228865, 322057867, 347386557, 374284647,
  402823977, 433078547, 465124549, 499040399, 534906769, 572806619, 612825229,
  655050231, 699571641, 746481891, 795875861, 847850911, 902506913, 959946283,
  1020274013, 1083597703, 1150027593, 1219676595, 1292660325, 1369097135,
  1449108145, 1532817275, 1620351277, 1711839767, 1807415257, 1907213187,
  2011371957, 2120032959,


  2233340609U, 2351442379U, 2474488829U, 2602633639U, 2736033641U, 2874848851U,
  3019242501U, 3169381071U, 3325434321U, 3487575323U, 3655980493U, 3830829623U,
  4012305913U,


  8989, 19825, 40081, 75517, 134245, 227305, 369305, 579125, 880685, 1303777,
  1884961, 2668525, 3707509, 5064793, 6814249, 9041957, 11847485, 15345233,
  19665841, 24957661, 31388293, 39146185, 48442297, 59511829, 72616013,
  88043969, 106114625, 127178701, 151620757, 179861305, 212358985, 249612805,
  292164445, 340600625, 395555537, 457713341, 527810725, 606639529, 695049433,
  793950709, 904317037, 1027188385, 1163673953, 1314955181, 1482288821,
  1667010073, 1870535785, 2094367717,


  2340095869U, 2609401873U, 2904062449U, 3225952925U, 3577050821U, 3959439497U,


  48639, 108545, 224143, 433905, 795455, 1392065, 2340495, 3800305, 5984767,
  9173505, 13726991, 20103025, 28875327, 40754369, 56610575, 77500017,
  104692735, 139703809, 184327311, 240673265, 311207743, 398796225, 506750351,
  638878193, 799538175, 993696769, 1226990095, 1505789553, 1837271615,
  2229491905U,


  2691463695U, 3233240945U, 3866006015U,


  265729, 598417, 1256465, 2485825, 4673345, 8405905, 14546705, 24331777,
  39490049, 62390545, 96220561, 145198913, 214828609, 312193553, 446304145,
  628496897, 872893441, 1196924561, 1621925137, 2173806145U,


  2883810113U,


  1462563, 3317445, 7059735, 14218905, 27298155, 50250765, 89129247, 152951073,
  254831667, 413442773, 654862247, 1014889769, 1541911931, 2300409629U,
  3375210671U,

  8097453, 18474633, 39753273, 81270333, 158819253, 298199265, 540279585,
  948062325, 1616336765,


  2684641785U,


  45046719, 103274625, 224298231, 464387817, 921406335, 1759885185,
  3248227095U,

  251595969, 579168825, 1267854873, 2653649025U,

  1409933619
};


static const opus_uint32 *const CELT_PVQ_U_ROW[15]={
  CELT_PVQ_U_DATA+ 0,CELT_PVQ_U_DATA+ 208,CELT_PVQ_U_DATA+ 415,
  CELT_PVQ_U_DATA+ 621,CELT_PVQ_U_DATA+ 826,CELT_PVQ_U_DATA+1030,
  CELT_PVQ_U_DATA+1233,CELT_PVQ_U_DATA+1336,CELT_PVQ_U_DATA+1389,
  CELT_PVQ_U_DATA+1421,CELT_PVQ_U_DATA+1441,CELT_PVQ_U_DATA+1455,
  CELT_PVQ_U_DATA+1464,CELT_PVQ_U_DATA+1470,CELT_PVQ_U_DATA+1473
};
void get_required_bits(opus_int16 *_bits,int _n,int _maxk,int _frac){
  int k;

  ;
  _bits[0]=0;
  for(k=1;k<=_maxk;k++)_bits[k]=log2_frac(((CELT_PVQ_U_ROW[((_n) < (k) ? (_n) : (k))][((_n) > (k) ? (_n) : (k))])+(CELT_PVQ_U_ROW[((_n) < ((k)+1) ? (_n) : ((k)+1))][((_n) > ((k)+1) ? (_n) : ((k)+1))])),_frac);
}


static opus_uint32 icwrs(int _n,const int *_y){
  opus_uint32 i;
  int j;
  int k;
  ;
  j=_n-1;
  i=_y[j]<0;
  k=abs(_y[j]);
  do{
    j--;
    i+=(CELT_PVQ_U_ROW[((_n-j) < (k) ? (_n-j) : (k))][((_n-j) > (k) ? (_n-j) : (k))]);
    k+=abs(_y[j]);
    if(_y[j]<0)i+=(CELT_PVQ_U_ROW[((_n-j) < (k+1) ? (_n-j) : (k+1))][((_n-j) > (k+1) ? (_n-j) : (k+1))]);
  }
  while(j>0);
  return i;
}

void encode_pulses(const int *_y,int _n,int _k,ec_enc *_enc){
  ;
  ec_enc_uint(_enc,icwrs(_n,_y),((CELT_PVQ_U_ROW[((_n) < (_k) ? (_n) : (_k))][((_n) > (_k) ? (_n) : (_k))])+(CELT_PVQ_U_ROW[((_n) < ((_k)+1) ? (_n) : ((_k)+1))][((_n) > ((_k)+1) ? (_n) : ((_k)+1))])));
}

static opus_val32 cwrsi(int _n,int _k,opus_uint32 _i,int *_y){
  opus_uint32 p;
  int s;
  int k0;
  opus_int16 val;
  opus_val32 yy=0;
  ;
  ;
  while(_n>2){
    opus_uint32 q;

    if(_k>=_n){
      const opus_uint32 *row;
      row=CELT_PVQ_U_ROW[_n];

      p=row[_k+1];
      s=-(_i>=p);
      _i-=p&s;

      k0=_k;
      q=row[_n];
      if(q>_i){
        ;
        _k=_n;
        do p=CELT_PVQ_U_ROW[--_k][_n];
        while(p>_i);
      }
      else for(p=row[_k];p>_i;p=row[_k])_k--;
      _i-=p;
      val=(k0-_k+s)^s;
      *_y++=val;
      yy=((yy)+(opus_val32)(val)*(opus_val32)(val));
    }

    else{

      p=CELT_PVQ_U_ROW[_k][_n];
      q=CELT_PVQ_U_ROW[_k+1][_n];
      if(p<=_i&&_i<q){
        _i-=p;
        *_y++=0;
      }
      else{

        s=-(_i>=q);
        _i-=q&s;

        k0=_k;
        do p=CELT_PVQ_U_ROW[--_k][_n];
        while(p>_i);
        _i-=p;
        val=(k0-_k+s)^s;
        *_y++=val;
        yy=((yy)+(opus_val32)(val)*(opus_val32)(val));
      }
    }
    _n--;
  }

  p=2*_k+1;
  s=-(_i>=p);
  _i-=p&s;
  k0=_k;
  _k=(_i+1)>>1;
  if(_k)_i-=2*_k-1;
  val=(k0-_k+s)^s;
  *_y++=val;
  yy=((yy)+(opus_val32)(val)*(opus_val32)(val));

  s=-(int)_i;
  val=(_k+s)^s;
  *_y=val;
  yy=((yy)+(opus_val32)(val)*(opus_val32)(val));
  return yy;
}

opus_val32 decode_pulses(int *_y,int _n,int _k,ec_dec *_dec){
  return cwrsi(_n,_k,ec_dec_uint(_dec,((CELT_PVQ_U_ROW[((_n) < (_k) ? (_n) : (_k))][((_n) > (_k) ? (_n) : (_k))])+(CELT_PVQ_U_ROW[((_n) < ((_k)+1) ? (_n) : ((_k)+1))][((_n) > ((_k)+1) ? (_n) : ((_k)+1))]))),_y);
}
unsigned isqrt32(opus_uint32 _val){
  unsigned b;
  unsigned g;
  int bshift;




  g=0;
  bshift=((((int)sizeof(unsigned)*8
         )-(__builtin_clz(_val)))-1)>>1;
  b=1U<<bshift;
  do{
    opus_uint32 t;
    t=(((opus_uint32)g<<1)+b)<<bshift;
    if(t<=_val){
      g+=b;
      _val-=t;
    }
    b>>=1;
    bshift--;
  }
  while(bshift>=0);
  return g;
}
typedef struct OpusCustomEncoder OpusCustomEncoder;






typedef struct OpusCustomDecoder OpusCustomDecoder;
typedef struct OpusCustomMode OpusCustomMode;
  OpusCustomMode *opus_custom_mode_create(opus_int32 Fs, int frame_size, int *error);





 void opus_custom_mode_destroy(OpusCustomMode *mode);
  OpusCustomEncoder *opus_custom_encoder_create(
    const OpusCustomMode *mode,
    int channels,
    int *error
) ;





 void opus_custom_encoder_destroy(OpusCustomEncoder *st);
  int opus_custom_encode_float(
    OpusCustomEncoder *st,
    const float *pcm,
    int frame_size,
    unsigned char *compressed,
    int maxCompressedBytes
) ;
  int opus_custom_encode(
    OpusCustomEncoder *st,
    const opus_int16 *pcm,
    int frame_size,
    unsigned char *compressed,
    int maxCompressedBytes
) ;







 int opus_custom_encoder_ctl(OpusCustomEncoder * restrict st, int request, ...) ;
  OpusCustomDecoder *opus_custom_decoder_create(
    const OpusCustomMode *mode,
    int channels,
    int *error
) ;




 void opus_custom_decoder_destroy(OpusCustomDecoder *st);
  int opus_custom_decode_float(
    OpusCustomDecoder *st,
    const unsigned char *data,
    int len,
    float *pcm,
    int frame_size
) ;
  int opus_custom_decode(
    OpusCustomDecoder *st,
    const unsigned char *data,
    int len,
    opus_int16 *pcm,
    int frame_size
) ;







 int opus_custom_decoder_ctl(OpusCustomDecoder * restrict st, int request, ...) ;
typedef struct {
   int valid;
   float tonality;
   float tonality_slope;
   float noisiness;
   float activity;
   float music_prob;
   float music_prob_min;
   float music_prob_max;
   int bandwidth;
   float activity_probability;
   float max_pitch_ratio;

   unsigned char leak_boost[19];
} AnalysisInfo;

typedef struct {
   int signalType;
   int offset;
} SILKInfo;
int celt_encoder_get_size(int channels);

int celt_encode_with_ec(OpusCustomEncoder * restrict st, const opus_val16 * pcm, int frame_size, unsigned char *compressed, int nbCompressedBytes, ec_enc *enc);

int celt_encoder_init(OpusCustomEncoder *st, opus_int32 sampling_rate, int channels,
                      int arch);





int celt_decoder_get_size(int channels);


int celt_decoder_init(OpusCustomDecoder *st, opus_int32 sampling_rate, int channels);

int celt_decode_with_ec(OpusCustomDecoder * restrict st, const unsigned char *data,
      int len, opus_val16 * restrict pcm, int frame_size, ec_dec *dec, int accum);
static const unsigned char trim_icdf[11] = {126, 124, 119, 109, 87, 41, 19, 9, 4, 2, 0};

static const unsigned char spread_icdf[4] = {25, 23, 2, 0};

static const unsigned char tapset_icdf[3]={2,1,0};


static const unsigned char toOpusTable[20] = {
      0xE0, 0xE8, 0xF0, 0xF8,
      0xC0, 0xC8, 0xD0, 0xD8,
      0xA0, 0xA8, 0xB0, 0xB8,
      0x00, 0x00, 0x00, 0x00,
      0x80, 0x88, 0x90, 0x98,
};

static const unsigned char fromOpusTable[16] = {
      0x80, 0x88, 0x90, 0x98,
      0x40, 0x48, 0x50, 0x58,
      0x20, 0x28, 0x30, 0x38,
      0x00, 0x08, 0x10, 0x18
};

static inline int toOpus(unsigned char c)
{
   int ret=0;
   if (c<0xA0)
      ret = toOpusTable[c>>3];
   if (ret == 0)
      return -1;
   else
      return ret|(c&0x7);
}

static inline int fromOpus(unsigned char c)
{
   if (c<0x80)
      return -1;
   else
      return fromOpusTable[(c>>3)-16] | (c&0x7);
}





extern const signed char tf_select_table[4][8];
int resampling_factor(opus_int32 rate);

void celt_preemphasis(const opus_val16 * restrict pcmp, celt_sig * restrict inp,
                        int N, int CC, int upsample, const opus_val16 *coef, celt_sig *mem, int clip);

void comb_filter(opus_val32 *y, opus_val32 *x, int T0, int T1, int N,
      opus_val16 g0, opus_val16 g1, int tapset0, int tapset1,
      const opus_val16 *window, int overlap, int arch);
void init_caps(const OpusCustomMode *m,int *cap,int LM,int C);
static inline int opus_select_arch(void)
{
  return 0;
}
typedef struct {
    float r;
    float i;
}kiss_fft_cpx;

typedef struct {
   float r;
   float i;
} kiss_twiddle_cpx;







typedef struct arch_fft_state{
   int is_supported;
   void *priv;
} arch_fft_state;

typedef struct kiss_fft_state{
    int nfft;
    opus_val16 scale;



    int shift;
    opus_int16 factors[2*8];
    const opus_int16 *bitrev;
    const kiss_twiddle_cpx *twiddles;
    arch_fft_state *arch_fft;
} kiss_fft_state;
kiss_fft_state *opus_fft_alloc_twiddles(int nfft,void * mem,size_t * lenmem, const kiss_fft_state *base, int arch);

kiss_fft_state *opus_fft_alloc(int nfft,void * mem,size_t * lenmem, int arch);
void opus_fft_c(const kiss_fft_state *cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);
void opus_ifft_c(const kiss_fft_state *cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);

void opus_fft_impl(const kiss_fft_state *st,kiss_fft_cpx *fout);
void opus_ifft_impl(const kiss_fft_state *st,kiss_fft_cpx *fout);

void opus_fft_free(const kiss_fft_state *cfg, int arch);


void opus_fft_free_arch_c(kiss_fft_state *st);
int opus_fft_alloc_arch_c(kiss_fft_state *st);


typedef struct {
   int n;
   int maxshift;
   const kiss_fft_state *kfft[4];
   const float * restrict trig;
} mdct_lookup;






int clt_mdct_init(mdct_lookup *l,int N, int maxshift, int arch);
void clt_mdct_clear(mdct_lookup *l, int arch);


void clt_mdct_forward_c(const mdct_lookup *l, float *in,
                        float * restrict out,
                        const opus_val16 *window, int overlap,
                        int shift, int stride, int arch);



void clt_mdct_backward_c(const mdct_lookup *l, float *in,
      float * restrict out,
      const opus_val16 * restrict window,
      int overlap, int shift, int stride, int arch);





typedef struct {
   int size;
   const opus_int16 *index;
   const unsigned char *bits;
   const unsigned char *caps;
} PulseCache;




struct OpusCustomMode {
   opus_int32 Fs;
   int overlap;

   int nbEBands;
   int effEBands;
   opus_val16 preemph[4];
   const opus_int16 *eBands;

   int maxLM;
   int nbShortMdcts;
   int shortMdctSize;

   int nbAllocVectors;
   const unsigned char *allocVectors;
   const opus_int16 *logN;

   const opus_val16 *window;
   mdct_lookup mdct;
   PulseCache cache;
};
void compute_pulse_cache(OpusCustomMode *m, int LM);

static inline int get_pulses(int i)
{
   return i<8 ? i : (8 + (i&7)) << ((i>>3)-1);
}

static inline int bits2pulses(const OpusCustomMode *m, int band, int LM, int bits)
{
   int i;
   int lo, hi;
   const unsigned char *cache;

   LM++;
   cache = m->cache.bits + m->cache.index[LM*m->nbEBands+band];

   lo = 0;
   hi = cache[0];
   bits--;
   for (i=0;i<6;i++)
   {
      int mid = (lo+hi+1)>>1;

      if ((int)cache[mid] >= bits)
         hi = mid;
      else
         lo = mid;
   }
   if (bits- (lo == 0 ? -1 : (int)cache[lo]) <= (int)cache[hi]-bits)
      return lo;
   else
      return hi;
}

static inline int pulses2bits(const OpusCustomMode *m, int band, int LM, int pulses)
{
   const unsigned char *cache;

   LM++;
   cache = m->cache.bits + m->cache.index[LM*m->nbEBands+band];
   return pulses == 0 ? 0 : cache[pulses]+1;
}
int clt_compute_allocation(const OpusCustomMode *m, int start, int end, const int *offsets, const int *cap, int alloc_trim, int *intensity, int *dual_stereo,
      opus_int32 total, opus_int32 *balance, int *pulses, int *ebits, int *fine_priority, int C, int LM, ec_ctx *ec, int encode, int prev, int signalBandwidth);


extern const opus_val16 eMeans[25];


void amp2Log2(const OpusCustomMode *m, int effEnd, int end,
      celt_ener *bandE, opus_val16 *bandLogE, int C);

void log2Amp(const OpusCustomMode *m, int start, int end,
      celt_ener *eBands, const opus_val16 *oldEBands, int C);

void quant_coarse_energy(const OpusCustomMode *m, int start, int end, int effEnd,
      const opus_val16 *eBands, opus_val16 *oldEBands, opus_uint32 budget,
      opus_val16 *error, ec_enc *enc, int C, int LM,
      int nbAvailableBytes, int force_intra, opus_val32 *delayedIntra,
      int two_pass, int loss_rate, int lfe);

void quant_fine_energy(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, opus_val16 *error, int *fine_quant, ec_enc *enc, int C);

void quant_energy_finalise(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, opus_val16 *error, int *fine_quant, int *fine_priority, int bits_left, ec_enc *enc, int C);

void unquant_coarse_energy(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, int intra, ec_dec *dec, int C, int LM);

void unquant_fine_energy(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, int *fine_quant, ec_dec *dec, int C);

void unquant_energy_finalise(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, int *fine_quant, int *fine_priority, int bits_left, ec_dec *dec, int C);


static const opus_int16 eband5ms[] = {

  0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 34, 40, 48, 60, 78, 100
};




static const unsigned char band_allocation[] = {

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 90, 80, 75, 69, 63, 56, 49, 40, 34, 29, 20, 18, 10, 0, 0, 0, 0, 0, 0, 0, 0,
110,100, 90, 84, 78, 71, 65, 58, 51, 45, 39, 32, 26, 20, 12, 0, 0, 0, 0, 0, 0,
118,110,103, 93, 86, 80, 75, 70, 65, 59, 53, 47, 40, 31, 23, 15, 4, 0, 0, 0, 0,
126,119,112,104, 95, 89, 83, 78, 72, 66, 60, 54, 47, 39, 32, 25, 17, 12, 1, 0, 0,
134,127,120,114,103, 97, 91, 85, 78, 72, 66, 60, 54, 47, 41, 35, 29, 23, 16, 10, 1,
144,137,130,124,113,107,101, 95, 88, 82, 76, 70, 64, 57, 51, 45, 39, 33, 26, 15, 1,
152,145,138,132,123,117,111,105, 98, 92, 86, 80, 74, 67, 61, 55, 49, 43, 36, 20, 1,
162,155,148,142,133,127,121,115,108,102, 96, 90, 84, 77, 71, 65, 59, 53, 46, 30, 1,
172,165,158,152,143,137,131,125,118,112,106,100, 94, 87, 81, 75, 69, 63, 56, 45, 20,
200,200,200,200,200,200,200,200,198,193,188,183,178,173,168,163,158,153,148,129,104,
};





static const opus_val16 window120[120] = {
6.7286966e-05f, 0.00060551348f, 0.0016815970f, 0.0032947962f, 0.0054439943f,
0.0081276923f, 0.011344001f, 0.015090633f, 0.019364886f, 0.024163635f,
0.029483315f, 0.035319905f, 0.041668911f, 0.048525347f, 0.055883718f,
0.063737999f, 0.072081616f, 0.080907428f, 0.090207705f, 0.099974111f,
0.11019769f, 0.12086883f, 0.13197729f, 0.14351214f, 0.15546177f,
0.16781389f, 0.18055550f, 0.19367290f, 0.20715171f, 0.22097682f,
0.23513243f, 0.24960208f, 0.26436860f, 0.27941419f, 0.29472040f,
0.31026818f, 0.32603788f, 0.34200931f, 0.35816177f, 0.37447407f,
0.39092462f, 0.40749142f, 0.42415215f, 0.44088423f, 0.45766484f,
0.47447104f, 0.49127978f, 0.50806798f, 0.52481261f, 0.54149077f,
0.55807973f, 0.57455701f, 0.59090049f, 0.60708841f, 0.62309951f,
0.63891306f, 0.65450896f, 0.66986776f, 0.68497077f, 0.69980010f,
0.71433873f, 0.72857055f, 0.74248043f, 0.75605424f, 0.76927895f,
0.78214257f, 0.79463430f, 0.80674445f, 0.81846456f, 0.82978733f,
0.84070669f, 0.85121779f, 0.86131698f, 0.87100183f, 0.88027111f,
0.88912479f, 0.89756398f, 0.90559094f, 0.91320904f, 0.92042270f,
0.92723738f, 0.93365955f, 0.93969656f, 0.94535671f, 0.95064907f,
0.95558353f, 0.96017067f, 0.96442171f, 0.96834849f, 0.97196334f,
0.97527906f, 0.97830883f, 0.98106616f, 0.98356480f, 0.98581869f,
0.98784191f, 0.98964856f, 0.99125274f, 0.99266849f, 0.99390969f,
0.99499004f, 0.99592297f, 0.99672162f, 0.99739874f, 0.99796667f,
0.99843728f, 0.99882195f, 0.99913147f, 0.99937606f, 0.99956527f,
0.99970802f, 0.99981248f, 0.99988613f, 0.99993565f, 0.99996697f,
0.99998518f, 0.99999457f, 0.99999859f, 0.99999982f, 1.0000000f,
};




static const opus_int16 logN400[21] = {
0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 16, 16, 16, 21, 21, 24, 29, 34, 36, };




static const opus_int16 cache_index50[105] = {
-1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 41, 41, 41,
82, 82, 123, 164, 200, 222, 0, 0, 0, 0, 0, 0, 0, 0, 41,
41, 41, 41, 123, 123, 123, 164, 164, 240, 266, 283, 295, 41, 41, 41,
41, 41, 41, 41, 41, 123, 123, 123, 123, 240, 240, 240, 266, 266, 305,
318, 328, 336, 123, 123, 123, 123, 123, 123, 123, 123, 240, 240, 240, 240,
305, 305, 305, 318, 318, 343, 351, 358, 364, 240, 240, 240, 240, 240, 240,
240, 240, 305, 305, 305, 305, 343, 343, 343, 351, 351, 370, 376, 382, 387,
};
static const unsigned char cache_bits50[392] = {
40, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 40, 15, 23, 28,
31, 34, 36, 38, 39, 41, 42, 43, 44, 45, 46, 47, 47, 49, 50,
51, 52, 53, 54, 55, 55, 57, 58, 59, 60, 61, 62, 63, 63, 65,
66, 67, 68, 69, 70, 71, 71, 40, 20, 33, 41, 48, 53, 57, 61,
64, 66, 69, 71, 73, 75, 76, 78, 80, 82, 85, 87, 89, 91, 92,
94, 96, 98, 101, 103, 105, 107, 108, 110, 112, 114, 117, 119, 121, 123,
124, 126, 128, 40, 23, 39, 51, 60, 67, 73, 79, 83, 87, 91, 94,
97, 100, 102, 105, 107, 111, 115, 118, 121, 124, 126, 129, 131, 135, 139,
142, 145, 148, 150, 153, 155, 159, 163, 166, 169, 172, 174, 177, 179, 35,
28, 49, 65, 78, 89, 99, 107, 114, 120, 126, 132, 136, 141, 145, 149,
153, 159, 165, 171, 176, 180, 185, 189, 192, 199, 205, 211, 216, 220, 225,
229, 232, 239, 245, 251, 21, 33, 58, 79, 97, 112, 125, 137, 148, 157,
166, 174, 182, 189, 195, 201, 207, 217, 227, 235, 243, 251, 17, 35, 63,
86, 106, 123, 139, 152, 165, 177, 187, 197, 206, 214, 222, 230, 237, 250,
25, 31, 55, 75, 91, 105, 117, 128, 138, 146, 154, 161, 168, 174, 180,
185, 190, 200, 208, 215, 222, 229, 235, 240, 245, 255, 16, 36, 65, 89,
110, 128, 144, 159, 173, 185, 196, 207, 217, 226, 234, 242, 250, 11, 41,
74, 103, 128, 151, 172, 191, 209, 225, 241, 255, 9, 43, 79, 110, 138,
163, 186, 207, 227, 246, 12, 39, 71, 99, 123, 144, 164, 182, 198, 214,
228, 241, 253, 9, 44, 81, 113, 142, 168, 192, 214, 235, 255, 7, 49,
90, 127, 160, 191, 220, 247, 6, 51, 95, 134, 170, 203, 234, 7, 47,
87, 123, 155, 184, 212, 237, 6, 52, 97, 137, 174, 208, 240, 5, 57,
106, 151, 192, 231, 5, 59, 111, 158, 202, 243, 5, 55, 103, 147, 187,
224, 5, 60, 113, 161, 206, 248, 4, 65, 122, 175, 224, 4, 67, 127,
182, 234, };
static const unsigned char cache_caps50[168] = {
224, 224, 224, 224, 224, 224, 224, 224, 160, 160, 160, 160, 185, 185, 185,
178, 178, 168, 134, 61, 37, 224, 224, 224, 224, 224, 224, 224, 224, 240,
240, 240, 240, 207, 207, 207, 198, 198, 183, 144, 66, 40, 160, 160, 160,
160, 160, 160, 160, 160, 185, 185, 185, 185, 193, 193, 193, 183, 183, 172,
138, 64, 38, 240, 240, 240, 240, 240, 240, 240, 240, 207, 207, 207, 207,
204, 204, 204, 193, 193, 180, 143, 66, 40, 185, 185, 185, 185, 185, 185,
185, 185, 193, 193, 193, 193, 193, 193, 193, 183, 183, 172, 138, 65, 39,
207, 207, 207, 207, 207, 207, 207, 207, 204, 204, 204, 204, 201, 201, 201,
188, 188, 176, 141, 66, 40, 193, 193, 193, 193, 193, 193, 193, 193, 193,
193, 193, 193, 194, 194, 194, 184, 184, 173, 139, 65, 39, 204, 204, 204,
204, 204, 204, 204, 204, 201, 201, 201, 201, 198, 198, 198, 187, 187, 175,
140, 66, 40, };




static const kiss_twiddle_cpx fft_twiddles48000_960[480] = {
{1.0000000f, -0.0000000f}, {0.99991433f, -0.013089596f},
{0.99965732f, -0.026176948f}, {0.99922904f, -0.039259816f},
{0.99862953f, -0.052335956f}, {0.99785892f, -0.065403129f},
{0.99691733f, -0.078459096f}, {0.99580493f, -0.091501619f},
{0.99452190f, -0.10452846f}, {0.99306846f, -0.11753740f},
{0.99144486f, -0.13052619f}, {0.98965139f, -0.14349262f},
{0.98768834f, -0.15643447f}, {0.98555606f, -0.16934950f},
{0.98325491f, -0.18223553f}, {0.98078528f, -0.19509032f},
{0.97814760f, -0.20791169f}, {0.97534232f, -0.22069744f},
{0.97236992f, -0.23344536f}, {0.96923091f, -0.24615329f},
{0.96592583f, -0.25881905f}, {0.96245524f, -0.27144045f},
{0.95881973f, -0.28401534f}, {0.95501994f, -0.29654157f},
{0.95105652f, -0.30901699f}, {0.94693013f, -0.32143947f},
{0.94264149f, -0.33380686f}, {0.93819134f, -0.34611706f},
{0.93358043f, -0.35836795f}, {0.92880955f, -0.37055744f},
{0.92387953f, -0.38268343f}, {0.91879121f, -0.39474386f},
{0.91354546f, -0.40673664f}, {0.90814317f, -0.41865974f},
{0.90258528f, -0.43051110f}, {0.89687274f, -0.44228869f},
{0.89100652f, -0.45399050f}, {0.88498764f, -0.46561452f},
{0.87881711f, -0.47715876f}, {0.87249601f, -0.48862124f},
{0.86602540f, -0.50000000f}, {0.85940641f, -0.51129309f},
{0.85264016f, -0.52249856f}, {0.84572782f, -0.53361452f},
{0.83867057f, -0.54463904f}, {0.83146961f, -0.55557023f},
{0.82412619f, -0.56640624f}, {0.81664156f, -0.57714519f},
{0.80901699f, -0.58778525f}, {0.80125381f, -0.59832460f},
{0.79335334f, -0.60876143f}, {0.78531693f, -0.61909395f},
{0.77714596f, -0.62932039f}, {0.76884183f, -0.63943900f},
{0.76040597f, -0.64944805f}, {0.75183981f, -0.65934582f},
{0.74314483f, -0.66913061f}, {0.73432251f, -0.67880075f},
{0.72537437f, -0.68835458f}, {0.71630194f, -0.69779046f},
{0.70710678f, -0.70710678f}, {0.69779046f, -0.71630194f},
{0.68835458f, -0.72537437f}, {0.67880075f, -0.73432251f},
{0.66913061f, -0.74314483f}, {0.65934582f, -0.75183981f},
{0.64944805f, -0.76040597f}, {0.63943900f, -0.76884183f},
{0.62932039f, -0.77714596f}, {0.61909395f, -0.78531693f},
{0.60876143f, -0.79335334f}, {0.59832460f, -0.80125381f},
{0.58778525f, -0.80901699f}, {0.57714519f, -0.81664156f},
{0.56640624f, -0.82412619f}, {0.55557023f, -0.83146961f},
{0.54463904f, -0.83867057f}, {0.53361452f, -0.84572782f},
{0.52249856f, -0.85264016f}, {0.51129309f, -0.85940641f},
{0.50000000f, -0.86602540f}, {0.48862124f, -0.87249601f},
{0.47715876f, -0.87881711f}, {0.46561452f, -0.88498764f},
{0.45399050f, -0.89100652f}, {0.44228869f, -0.89687274f},
{0.43051110f, -0.90258528f}, {0.41865974f, -0.90814317f},
{0.40673664f, -0.91354546f}, {0.39474386f, -0.91879121f},
{0.38268343f, -0.92387953f}, {0.37055744f, -0.92880955f},
{0.35836795f, -0.93358043f}, {0.34611706f, -0.93819134f},
{0.33380686f, -0.94264149f}, {0.32143947f, -0.94693013f},
{0.30901699f, -0.95105652f}, {0.29654157f, -0.95501994f},
{0.28401534f, -0.95881973f}, {0.27144045f, -0.96245524f},
{0.25881905f, -0.96592583f}, {0.24615329f, -0.96923091f},
{0.23344536f, -0.97236992f}, {0.22069744f, -0.97534232f},
{0.20791169f, -0.97814760f}, {0.19509032f, -0.98078528f},
{0.18223553f, -0.98325491f}, {0.16934950f, -0.98555606f},
{0.15643447f, -0.98768834f}, {0.14349262f, -0.98965139f},
{0.13052619f, -0.99144486f}, {0.11753740f, -0.99306846f},
{0.10452846f, -0.99452190f}, {0.091501619f, -0.99580493f},
{0.078459096f, -0.99691733f}, {0.065403129f, -0.99785892f},
{0.052335956f, -0.99862953f}, {0.039259816f, -0.99922904f},
{0.026176948f, -0.99965732f}, {0.013089596f, -0.99991433f},
{6.1230318e-17f, -1.0000000f}, {-0.013089596f, -0.99991433f},
{-0.026176948f, -0.99965732f}, {-0.039259816f, -0.99922904f},
{-0.052335956f, -0.99862953f}, {-0.065403129f, -0.99785892f},
{-0.078459096f, -0.99691733f}, {-0.091501619f, -0.99580493f},
{-0.10452846f, -0.99452190f}, {-0.11753740f, -0.99306846f},
{-0.13052619f, -0.99144486f}, {-0.14349262f, -0.98965139f},
{-0.15643447f, -0.98768834f}, {-0.16934950f, -0.98555606f},
{-0.18223553f, -0.98325491f}, {-0.19509032f, -0.98078528f},
{-0.20791169f, -0.97814760f}, {-0.22069744f, -0.97534232f},
{-0.23344536f, -0.97236992f}, {-0.24615329f, -0.96923091f},
{-0.25881905f, -0.96592583f}, {-0.27144045f, -0.96245524f},
{-0.28401534f, -0.95881973f}, {-0.29654157f, -0.95501994f},
{-0.30901699f, -0.95105652f}, {-0.32143947f, -0.94693013f},
{-0.33380686f, -0.94264149f}, {-0.34611706f, -0.93819134f},
{-0.35836795f, -0.93358043f}, {-0.37055744f, -0.92880955f},
{-0.38268343f, -0.92387953f}, {-0.39474386f, -0.91879121f},
{-0.40673664f, -0.91354546f}, {-0.41865974f, -0.90814317f},
{-0.43051110f, -0.90258528f}, {-0.44228869f, -0.89687274f},
{-0.45399050f, -0.89100652f}, {-0.46561452f, -0.88498764f},
{-0.47715876f, -0.87881711f}, {-0.48862124f, -0.87249601f},
{-0.50000000f, -0.86602540f}, {-0.51129309f, -0.85940641f},
{-0.52249856f, -0.85264016f}, {-0.53361452f, -0.84572782f},
{-0.54463904f, -0.83867057f}, {-0.55557023f, -0.83146961f},
{-0.56640624f, -0.82412619f}, {-0.57714519f, -0.81664156f},
{-0.58778525f, -0.80901699f}, {-0.59832460f, -0.80125381f},
{-0.60876143f, -0.79335334f}, {-0.61909395f, -0.78531693f},
{-0.62932039f, -0.77714596f}, {-0.63943900f, -0.76884183f},
{-0.64944805f, -0.76040597f}, {-0.65934582f, -0.75183981f},
{-0.66913061f, -0.74314483f}, {-0.67880075f, -0.73432251f},
{-0.68835458f, -0.72537437f}, {-0.69779046f, -0.71630194f},
{-0.70710678f, -0.70710678f}, {-0.71630194f, -0.69779046f},
{-0.72537437f, -0.68835458f}, {-0.73432251f, -0.67880075f},
{-0.74314483f, -0.66913061f}, {-0.75183981f, -0.65934582f},
{-0.76040597f, -0.64944805f}, {-0.76884183f, -0.63943900f},
{-0.77714596f, -0.62932039f}, {-0.78531693f, -0.61909395f},
{-0.79335334f, -0.60876143f}, {-0.80125381f, -0.59832460f},
{-0.80901699f, -0.58778525f}, {-0.81664156f, -0.57714519f},
{-0.82412619f, -0.56640624f}, {-0.83146961f, -0.55557023f},
{-0.83867057f, -0.54463904f}, {-0.84572782f, -0.53361452f},
{-0.85264016f, -0.52249856f}, {-0.85940641f, -0.51129309f},
{-0.86602540f, -0.50000000f}, {-0.87249601f, -0.48862124f},
{-0.87881711f, -0.47715876f}, {-0.88498764f, -0.46561452f},
{-0.89100652f, -0.45399050f}, {-0.89687274f, -0.44228869f},
{-0.90258528f, -0.43051110f}, {-0.90814317f, -0.41865974f},
{-0.91354546f, -0.40673664f}, {-0.91879121f, -0.39474386f},
{-0.92387953f, -0.38268343f}, {-0.92880955f, -0.37055744f},
{-0.93358043f, -0.35836795f}, {-0.93819134f, -0.34611706f},
{-0.94264149f, -0.33380686f}, {-0.94693013f, -0.32143947f},
{-0.95105652f, -0.30901699f}, {-0.95501994f, -0.29654157f},
{-0.95881973f, -0.28401534f}, {-0.96245524f, -0.27144045f},
{-0.96592583f, -0.25881905f}, {-0.96923091f, -0.24615329f},
{-0.97236992f, -0.23344536f}, {-0.97534232f, -0.22069744f},
{-0.97814760f, -0.20791169f}, {-0.98078528f, -0.19509032f},
{-0.98325491f, -0.18223553f}, {-0.98555606f, -0.16934950f},
{-0.98768834f, -0.15643447f}, {-0.98965139f, -0.14349262f},
{-0.99144486f, -0.13052619f}, {-0.99306846f, -0.11753740f},
{-0.99452190f, -0.10452846f}, {-0.99580493f, -0.091501619f},
{-0.99691733f, -0.078459096f}, {-0.99785892f, -0.065403129f},
{-0.99862953f, -0.052335956f}, {-0.99922904f, -0.039259816f},
{-0.99965732f, -0.026176948f}, {-0.99991433f, -0.013089596f},
{-1.0000000f, -1.2246064e-16f}, {-0.99991433f, 0.013089596f},
{-0.99965732f, 0.026176948f}, {-0.99922904f, 0.039259816f},
{-0.99862953f, 0.052335956f}, {-0.99785892f, 0.065403129f},
{-0.99691733f, 0.078459096f}, {-0.99580493f, 0.091501619f},
{-0.99452190f, 0.10452846f}, {-0.99306846f, 0.11753740f},
{-0.99144486f, 0.13052619f}, {-0.98965139f, 0.14349262f},
{-0.98768834f, 0.15643447f}, {-0.98555606f, 0.16934950f},
{-0.98325491f, 0.18223553f}, {-0.98078528f, 0.19509032f},
{-0.97814760f, 0.20791169f}, {-0.97534232f, 0.22069744f},
{-0.97236992f, 0.23344536f}, {-0.96923091f, 0.24615329f},
{-0.96592583f, 0.25881905f}, {-0.96245524f, 0.27144045f},
{-0.95881973f, 0.28401534f}, {-0.95501994f, 0.29654157f},
{-0.95105652f, 0.30901699f}, {-0.94693013f, 0.32143947f},
{-0.94264149f, 0.33380686f}, {-0.93819134f, 0.34611706f},
{-0.93358043f, 0.35836795f}, {-0.92880955f, 0.37055744f},
{-0.92387953f, 0.38268343f}, {-0.91879121f, 0.39474386f},
{-0.91354546f, 0.40673664f}, {-0.90814317f, 0.41865974f},
{-0.90258528f, 0.43051110f}, {-0.89687274f, 0.44228869f},
{-0.89100652f, 0.45399050f}, {-0.88498764f, 0.46561452f},
{-0.87881711f, 0.47715876f}, {-0.87249601f, 0.48862124f},
{-0.86602540f, 0.50000000f}, {-0.85940641f, 0.51129309f},
{-0.85264016f, 0.52249856f}, {-0.84572782f, 0.53361452f},
{-0.83867057f, 0.54463904f}, {-0.83146961f, 0.55557023f},
{-0.82412619f, 0.56640624f}, {-0.81664156f, 0.57714519f},
{-0.80901699f, 0.58778525f}, {-0.80125381f, 0.59832460f},
{-0.79335334f, 0.60876143f}, {-0.78531693f, 0.61909395f},
{-0.77714596f, 0.62932039f}, {-0.76884183f, 0.63943900f},
{-0.76040597f, 0.64944805f}, {-0.75183981f, 0.65934582f},
{-0.74314483f, 0.66913061f}, {-0.73432251f, 0.67880075f},
{-0.72537437f, 0.68835458f}, {-0.71630194f, 0.69779046f},
{-0.70710678f, 0.70710678f}, {-0.69779046f, 0.71630194f},
{-0.68835458f, 0.72537437f}, {-0.67880075f, 0.73432251f},
{-0.66913061f, 0.74314483f}, {-0.65934582f, 0.75183981f},
{-0.64944805f, 0.76040597f}, {-0.63943900f, 0.76884183f},
{-0.62932039f, 0.77714596f}, {-0.61909395f, 0.78531693f},
{-0.60876143f, 0.79335334f}, {-0.59832460f, 0.80125381f},
{-0.58778525f, 0.80901699f}, {-0.57714519f, 0.81664156f},
{-0.56640624f, 0.82412619f}, {-0.55557023f, 0.83146961f},
{-0.54463904f, 0.83867057f}, {-0.53361452f, 0.84572782f},
{-0.52249856f, 0.85264016f}, {-0.51129309f, 0.85940641f},
{-0.50000000f, 0.86602540f}, {-0.48862124f, 0.87249601f},
{-0.47715876f, 0.87881711f}, {-0.46561452f, 0.88498764f},
{-0.45399050f, 0.89100652f}, {-0.44228869f, 0.89687274f},
{-0.43051110f, 0.90258528f}, {-0.41865974f, 0.90814317f},
{-0.40673664f, 0.91354546f}, {-0.39474386f, 0.91879121f},
{-0.38268343f, 0.92387953f}, {-0.37055744f, 0.92880955f},
{-0.35836795f, 0.93358043f}, {-0.34611706f, 0.93819134f},
{-0.33380686f, 0.94264149f}, {-0.32143947f, 0.94693013f},
{-0.30901699f, 0.95105652f}, {-0.29654157f, 0.95501994f},
{-0.28401534f, 0.95881973f}, {-0.27144045f, 0.96245524f},
{-0.25881905f, 0.96592583f}, {-0.24615329f, 0.96923091f},
{-0.23344536f, 0.97236992f}, {-0.22069744f, 0.97534232f},
{-0.20791169f, 0.97814760f}, {-0.19509032f, 0.98078528f},
{-0.18223553f, 0.98325491f}, {-0.16934950f, 0.98555606f},
{-0.15643447f, 0.98768834f}, {-0.14349262f, 0.98965139f},
{-0.13052619f, 0.99144486f}, {-0.11753740f, 0.99306846f},
{-0.10452846f, 0.99452190f}, {-0.091501619f, 0.99580493f},
{-0.078459096f, 0.99691733f}, {-0.065403129f, 0.99785892f},
{-0.052335956f, 0.99862953f}, {-0.039259816f, 0.99922904f},
{-0.026176948f, 0.99965732f}, {-0.013089596f, 0.99991433f},
{-1.8369095e-16f, 1.0000000f}, {0.013089596f, 0.99991433f},
{0.026176948f, 0.99965732f}, {0.039259816f, 0.99922904f},
{0.052335956f, 0.99862953f}, {0.065403129f, 0.99785892f},
{0.078459096f, 0.99691733f}, {0.091501619f, 0.99580493f},
{0.10452846f, 0.99452190f}, {0.11753740f, 0.99306846f},
{0.13052619f, 0.99144486f}, {0.14349262f, 0.98965139f},
{0.15643447f, 0.98768834f}, {0.16934950f, 0.98555606f},
{0.18223553f, 0.98325491f}, {0.19509032f, 0.98078528f},
{0.20791169f, 0.97814760f}, {0.22069744f, 0.97534232f},
{0.23344536f, 0.97236992f}, {0.24615329f, 0.96923091f},
{0.25881905f, 0.96592583f}, {0.27144045f, 0.96245524f},
{0.28401534f, 0.95881973f}, {0.29654157f, 0.95501994f},
{0.30901699f, 0.95105652f}, {0.32143947f, 0.94693013f},
{0.33380686f, 0.94264149f}, {0.34611706f, 0.93819134f},
{0.35836795f, 0.93358043f}, {0.37055744f, 0.92880955f},
{0.38268343f, 0.92387953f}, {0.39474386f, 0.91879121f},
{0.40673664f, 0.91354546f}, {0.41865974f, 0.90814317f},
{0.43051110f, 0.90258528f}, {0.44228869f, 0.89687274f},
{0.45399050f, 0.89100652f}, {0.46561452f, 0.88498764f},
{0.47715876f, 0.87881711f}, {0.48862124f, 0.87249601f},
{0.50000000f, 0.86602540f}, {0.51129309f, 0.85940641f},
{0.52249856f, 0.85264016f}, {0.53361452f, 0.84572782f},
{0.54463904f, 0.83867057f}, {0.55557023f, 0.83146961f},
{0.56640624f, 0.82412619f}, {0.57714519f, 0.81664156f},
{0.58778525f, 0.80901699f}, {0.59832460f, 0.80125381f},
{0.60876143f, 0.79335334f}, {0.61909395f, 0.78531693f},
{0.62932039f, 0.77714596f}, {0.63943900f, 0.76884183f},
{0.64944805f, 0.76040597f}, {0.65934582f, 0.75183981f},
{0.66913061f, 0.74314483f}, {0.67880075f, 0.73432251f},
{0.68835458f, 0.72537437f}, {0.69779046f, 0.71630194f},
{0.70710678f, 0.70710678f}, {0.71630194f, 0.69779046f},
{0.72537437f, 0.68835458f}, {0.73432251f, 0.67880075f},
{0.74314483f, 0.66913061f}, {0.75183981f, 0.65934582f},
{0.76040597f, 0.64944805f}, {0.76884183f, 0.63943900f},
{0.77714596f, 0.62932039f}, {0.78531693f, 0.61909395f},
{0.79335334f, 0.60876143f}, {0.80125381f, 0.59832460f},
{0.80901699f, 0.58778525f}, {0.81664156f, 0.57714519f},
{0.82412619f, 0.56640624f}, {0.83146961f, 0.55557023f},
{0.83867057f, 0.54463904f}, {0.84572782f, 0.53361452f},
{0.85264016f, 0.52249856f}, {0.85940641f, 0.51129309f},
{0.86602540f, 0.50000000f}, {0.87249601f, 0.48862124f},
{0.87881711f, 0.47715876f}, {0.88498764f, 0.46561452f},
{0.89100652f, 0.45399050f}, {0.89687274f, 0.44228869f},
{0.90258528f, 0.43051110f}, {0.90814317f, 0.41865974f},
{0.91354546f, 0.40673664f}, {0.91879121f, 0.39474386f},
{0.92387953f, 0.38268343f}, {0.92880955f, 0.37055744f},
{0.93358043f, 0.35836795f}, {0.93819134f, 0.34611706f},
{0.94264149f, 0.33380686f}, {0.94693013f, 0.32143947f},
{0.95105652f, 0.30901699f}, {0.95501994f, 0.29654157f},
{0.95881973f, 0.28401534f}, {0.96245524f, 0.27144045f},
{0.96592583f, 0.25881905f}, {0.96923091f, 0.24615329f},
{0.97236992f, 0.23344536f}, {0.97534232f, 0.22069744f},
{0.97814760f, 0.20791169f}, {0.98078528f, 0.19509032f},
{0.98325491f, 0.18223553f}, {0.98555606f, 0.16934950f},
{0.98768834f, 0.15643447f}, {0.98965139f, 0.14349262f},
{0.99144486f, 0.13052619f}, {0.99306846f, 0.11753740f},
{0.99452190f, 0.10452846f}, {0.99580493f, 0.091501619f},
{0.99691733f, 0.078459096f}, {0.99785892f, 0.065403129f},
{0.99862953f, 0.052335956f}, {0.99922904f, 0.039259816f},
{0.99965732f, 0.026176948f}, {0.99991433f, 0.013089596f},
};


static const opus_int16 fft_bitrev480[480] = {
0, 96, 192, 288, 384, 32, 128, 224, 320, 416, 64, 160, 256, 352, 448,
8, 104, 200, 296, 392, 40, 136, 232, 328, 424, 72, 168, 264, 360, 456,
16, 112, 208, 304, 400, 48, 144, 240, 336, 432, 80, 176, 272, 368, 464,
24, 120, 216, 312, 408, 56, 152, 248, 344, 440, 88, 184, 280, 376, 472,
4, 100, 196, 292, 388, 36, 132, 228, 324, 420, 68, 164, 260, 356, 452,
12, 108, 204, 300, 396, 44, 140, 236, 332, 428, 76, 172, 268, 364, 460,
20, 116, 212, 308, 404, 52, 148, 244, 340, 436, 84, 180, 276, 372, 468,
28, 124, 220, 316, 412, 60, 156, 252, 348, 444, 92, 188, 284, 380, 476,
1, 97, 193, 289, 385, 33, 129, 225, 321, 417, 65, 161, 257, 353, 449,
9, 105, 201, 297, 393, 41, 137, 233, 329, 425, 73, 169, 265, 361, 457,
17, 113, 209, 305, 401, 49, 145, 241, 337, 433, 81, 177, 273, 369, 465,
25, 121, 217, 313, 409, 57, 153, 249, 345, 441, 89, 185, 281, 377, 473,
5, 101, 197, 293, 389, 37, 133, 229, 325, 421, 69, 165, 261, 357, 453,
13, 109, 205, 301, 397, 45, 141, 237, 333, 429, 77, 173, 269, 365, 461,
21, 117, 213, 309, 405, 53, 149, 245, 341, 437, 85, 181, 277, 373, 469,
29, 125, 221, 317, 413, 61, 157, 253, 349, 445, 93, 189, 285, 381, 477,
2, 98, 194, 290, 386, 34, 130, 226, 322, 418, 66, 162, 258, 354, 450,
10, 106, 202, 298, 394, 42, 138, 234, 330, 426, 74, 170, 266, 362, 458,
18, 114, 210, 306, 402, 50, 146, 242, 338, 434, 82, 178, 274, 370, 466,
26, 122, 218, 314, 410, 58, 154, 250, 346, 442, 90, 186, 282, 378, 474,
6, 102, 198, 294, 390, 38, 134, 230, 326, 422, 70, 166, 262, 358, 454,
14, 110, 206, 302, 398, 46, 142, 238, 334, 430, 78, 174, 270, 366, 462,
22, 118, 214, 310, 406, 54, 150, 246, 342, 438, 86, 182, 278, 374, 470,
30, 126, 222, 318, 414, 62, 158, 254, 350, 446, 94, 190, 286, 382, 478,
3, 99, 195, 291, 387, 35, 131, 227, 323, 419, 67, 163, 259, 355, 451,
11, 107, 203, 299, 395, 43, 139, 235, 331, 427, 75, 171, 267, 363, 459,
19, 115, 211, 307, 403, 51, 147, 243, 339, 435, 83, 179, 275, 371, 467,
27, 123, 219, 315, 411, 59, 155, 251, 347, 443, 91, 187, 283, 379, 475,
7, 103, 199, 295, 391, 39, 135, 231, 327, 423, 71, 167, 263, 359, 455,
15, 111, 207, 303, 399, 47, 143, 239, 335, 431, 79, 175, 271, 367, 463,
23, 119, 215, 311, 407, 55, 151, 247, 343, 439, 87, 183, 279, 375, 471,
31, 127, 223, 319, 415, 63, 159, 255, 351, 447, 95, 191, 287, 383, 479,
};




static const opus_int16 fft_bitrev240[240] = {
0, 48, 96, 144, 192, 16, 64, 112, 160, 208, 32, 80, 128, 176, 224,
4, 52, 100, 148, 196, 20, 68, 116, 164, 212, 36, 84, 132, 180, 228,
8, 56, 104, 152, 200, 24, 72, 120, 168, 216, 40, 88, 136, 184, 232,
12, 60, 108, 156, 204, 28, 76, 124, 172, 220, 44, 92, 140, 188, 236,
1, 49, 97, 145, 193, 17, 65, 113, 161, 209, 33, 81, 129, 177, 225,
5, 53, 101, 149, 197, 21, 69, 117, 165, 213, 37, 85, 133, 181, 229,
9, 57, 105, 153, 201, 25, 73, 121, 169, 217, 41, 89, 137, 185, 233,
13, 61, 109, 157, 205, 29, 77, 125, 173, 221, 45, 93, 141, 189, 237,
2, 50, 98, 146, 194, 18, 66, 114, 162, 210, 34, 82, 130, 178, 226,
6, 54, 102, 150, 198, 22, 70, 118, 166, 214, 38, 86, 134, 182, 230,
10, 58, 106, 154, 202, 26, 74, 122, 170, 218, 42, 90, 138, 186, 234,
14, 62, 110, 158, 206, 30, 78, 126, 174, 222, 46, 94, 142, 190, 238,
3, 51, 99, 147, 195, 19, 67, 115, 163, 211, 35, 83, 131, 179, 227,
7, 55, 103, 151, 199, 23, 71, 119, 167, 215, 39, 87, 135, 183, 231,
11, 59, 107, 155, 203, 27, 75, 123, 171, 219, 43, 91, 139, 187, 235,
15, 63, 111, 159, 207, 31, 79, 127, 175, 223, 47, 95, 143, 191, 239,
};




static const opus_int16 fft_bitrev120[120] = {
0, 24, 48, 72, 96, 8, 32, 56, 80, 104, 16, 40, 64, 88, 112,
4, 28, 52, 76, 100, 12, 36, 60, 84, 108, 20, 44, 68, 92, 116,
1, 25, 49, 73, 97, 9, 33, 57, 81, 105, 17, 41, 65, 89, 113,
5, 29, 53, 77, 101, 13, 37, 61, 85, 109, 21, 45, 69, 93, 117,
2, 26, 50, 74, 98, 10, 34, 58, 82, 106, 18, 42, 66, 90, 114,
6, 30, 54, 78, 102, 14, 38, 62, 86, 110, 22, 46, 70, 94, 118,
3, 27, 51, 75, 99, 11, 35, 59, 83, 107, 19, 43, 67, 91, 115,
7, 31, 55, 79, 103, 15, 39, 63, 87, 111, 23, 47, 71, 95, 119,
};




static const opus_int16 fft_bitrev60[60] = {
0, 12, 24, 36, 48, 4, 16, 28, 40, 52, 8, 20, 32, 44, 56,
1, 13, 25, 37, 49, 5, 17, 29, 41, 53, 9, 21, 33, 45, 57,
2, 14, 26, 38, 50, 6, 18, 30, 42, 54, 10, 22, 34, 46, 58,
3, 15, 27, 39, 51, 7, 19, 31, 43, 55, 11, 23, 35, 47, 59,
};




static const kiss_fft_state fft_state48000_960_0 = {
480,
0.002083333f,
-1,
{5, 96, 3, 32, 4, 8, 2, 4, 4, 1, 0, 0, 0, 0, 0, 0, },
fft_bitrev480,
fft_twiddles48000_960,




((void *)0)
   ,

};




static const kiss_fft_state fft_state48000_960_1 = {
240,
0.004166667f,
1,
{5, 48, 3, 16, 4, 4, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, },
fft_bitrev240,
fft_twiddles48000_960,




((void *)0)
   ,

};




static const kiss_fft_state fft_state48000_960_2 = {
120,
0.008333333f,
2,
{5, 24, 3, 8, 2, 4, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, },
fft_bitrev120,
fft_twiddles48000_960,




((void *)0)
   ,

};




static const kiss_fft_state fft_state48000_960_3 = {
60,
0.016666667f,
3,
{5, 12, 3, 4, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, },
fft_bitrev60,
fft_twiddles48000_960,




((void *)0)
   ,

};






static const opus_val16 mdct_twiddles960[1800] = {
0.99999994f, 0.99999321f, 0.99997580f, 0.99994773f, 0.99990886f,
0.99985933f, 0.99979913f, 0.99972820f, 0.99964654f, 0.99955416f,
0.99945110f, 0.99933738f, 0.99921292f, 0.99907774f, 0.99893188f,
0.99877530f, 0.99860805f, 0.99843007f, 0.99824142f, 0.99804211f,
0.99783206f, 0.99761140f, 0.99737996f, 0.99713790f, 0.99688518f,
0.99662173f, 0.99634761f, 0.99606287f, 0.99576741f, 0.99546129f,
0.99514455f, 0.99481714f, 0.99447906f, 0.99413031f, 0.99377096f,
0.99340093f, 0.99302030f, 0.99262899f, 0.99222708f, 0.99181455f,
0.99139136f, 0.99095762f, 0.99051321f, 0.99005818f, 0.98959261f,
0.98911643f, 0.98862964f, 0.98813224f, 0.98762429f, 0.98710573f,
0.98657662f, 0.98603696f, 0.98548669f, 0.98492593f, 0.98435456f,
0.98377270f, 0.98318028f, 0.98257732f, 0.98196387f, 0.98133987f,
0.98070538f, 0.98006040f, 0.97940493f, 0.97873890f, 0.97806245f,
0.97737551f, 0.97667813f, 0.97597027f, 0.97525197f, 0.97452319f,
0.97378403f, 0.97303438f, 0.97227436f, 0.97150391f, 0.97072303f,
0.96993178f, 0.96913016f, 0.96831810f, 0.96749574f, 0.96666300f,
0.96581990f, 0.96496642f, 0.96410263f, 0.96322852f, 0.96234411f,
0.96144938f, 0.96054435f, 0.95962906f, 0.95870346f, 0.95776761f,
0.95682150f, 0.95586514f, 0.95489854f, 0.95392174f, 0.95293468f,
0.95193744f, 0.95093000f, 0.94991243f, 0.94888461f, 0.94784665f,
0.94679856f, 0.94574034f, 0.94467193f, 0.94359344f, 0.94250488f,
0.94140619f, 0.94029742f, 0.93917859f, 0.93804967f, 0.93691075f,
0.93576175f, 0.93460274f, 0.93343377f, 0.93225473f, 0.93106574f,
0.92986679f, 0.92865789f, 0.92743903f, 0.92621022f, 0.92497152f,
0.92372292f, 0.92246443f, 0.92119598f, 0.91991776f, 0.91862965f,
0.91733170f, 0.91602397f, 0.91470635f, 0.91337901f, 0.91204184f,
0.91069490f, 0.90933824f, 0.90797186f, 0.90659571f, 0.90520984f,
0.90381432f, 0.90240908f, 0.90099424f, 0.89956969f, 0.89813554f,
0.89669174f, 0.89523834f, 0.89377540f, 0.89230281f, 0.89082074f,
0.88932908f, 0.88782793f, 0.88631725f, 0.88479710f, 0.88326746f,
0.88172835f, 0.88017982f, 0.87862182f, 0.87705445f, 0.87547767f,
0.87389153f, 0.87229604f, 0.87069118f, 0.86907703f, 0.86745358f,
0.86582077f, 0.86417878f, 0.86252749f, 0.86086690f, 0.85919720f,
0.85751826f, 0.85583007f, 0.85413277f, 0.85242635f, 0.85071075f,
0.84898609f, 0.84725231f, 0.84550947f, 0.84375757f, 0.84199661f,
0.84022665f, 0.83844769f, 0.83665979f, 0.83486289f, 0.83305705f,
0.83124226f, 0.82941860f, 0.82758605f, 0.82574469f, 0.82389444f,
0.82203537f, 0.82016748f, 0.81829083f, 0.81640542f, 0.81451124f,
0.81260836f, 0.81069672f, 0.80877650f, 0.80684757f, 0.80490994f,
0.80296379f, 0.80100900f, 0.79904562f, 0.79707366f, 0.79509324f,
0.79310423f, 0.79110676f, 0.78910083f, 0.78708643f, 0.78506362f,
0.78303236f, 0.78099275f, 0.77894479f, 0.77688843f, 0.77482378f,
0.77275085f, 0.77066964f, 0.76858020f, 0.76648247f, 0.76437658f,
0.76226246f, 0.76014024f, 0.75800985f, 0.75587130f, 0.75372469f,
0.75157005f, 0.74940729f, 0.74723655f, 0.74505776f, 0.74287105f,
0.74067634f, 0.73847371f, 0.73626316f, 0.73404479f, 0.73181850f,
0.72958434f, 0.72734243f, 0.72509271f, 0.72283524f, 0.72057003f,
0.71829706f, 0.71601641f, 0.71372813f, 0.71143216f, 0.70912862f,
0.70681745f, 0.70449871f, 0.70217246f, 0.69983864f, 0.69749737f,
0.69514859f, 0.69279242f, 0.69042879f, 0.68805778f, 0.68567938f,
0.68329364f, 0.68090063f, 0.67850029f, 0.67609268f, 0.67367786f,
0.67125577f, 0.66882652f, 0.66639012f, 0.66394657f, 0.66149592f,
0.65903819f, 0.65657341f, 0.65410155f, 0.65162271f, 0.64913690f,
0.64664418f, 0.64414448f, 0.64163786f, 0.63912445f, 0.63660413f,
0.63407701f, 0.63154310f, 0.62900239f, 0.62645501f, 0.62390089f,
0.62134010f, 0.61877263f, 0.61619854f, 0.61361790f, 0.61103064f,
0.60843682f, 0.60583651f, 0.60322970f, 0.60061646f, 0.59799677f,
0.59537065f, 0.59273821f, 0.59009939f, 0.58745426f, 0.58480281f,
0.58214509f, 0.57948118f, 0.57681108f, 0.57413477f, 0.57145232f,
0.56876373f, 0.56606907f, 0.56336832f, 0.56066155f, 0.55794877f,
0.55523002f, 0.55250537f, 0.54977477f, 0.54703826f, 0.54429591f,
0.54154772f, 0.53879374f, 0.53603399f, 0.53326851f, 0.53049731f,
0.52772039f, 0.52493787f, 0.52214974f, 0.51935595f, 0.51655668f,
0.51375180f, 0.51094145f, 0.50812566f, 0.50530440f, 0.50247771f,
0.49964568f, 0.49680826f, 0.49396557f, 0.49111754f, 0.48826426f,
0.48540577f, 0.48254207f, 0.47967321f, 0.47679919f, 0.47392011f,
0.47103590f, 0.46814668f, 0.46525243f, 0.46235323f, 0.45944905f,
0.45653993f, 0.45362595f, 0.45070711f, 0.44778344f, 0.44485497f,
0.44192174f, 0.43898380f, 0.43604112f, 0.43309379f, 0.43014181f,
0.42718524f, 0.42422408f, 0.42125839f, 0.41828820f, 0.41531351f,
0.41233435f, 0.40935081f, 0.40636289f, 0.40337059f, 0.40037400f,
0.39737311f, 0.39436796f, 0.39135858f, 0.38834500f, 0.38532731f,
0.38230544f, 0.37927949f, 0.37624949f, 0.37321547f, 0.37017745f,
0.36713544f, 0.36408952f, 0.36103970f, 0.35798600f, 0.35492846f,
0.35186714f, 0.34880206f, 0.34573323f, 0.34266070f, 0.33958447f,
0.33650464f, 0.33342120f, 0.33033419f, 0.32724363f, 0.32414958f,
0.32105204f, 0.31795108f, 0.31484672f, 0.31173897f, 0.30862790f,
0.30551350f, 0.30239585f, 0.29927495f, 0.29615086f, 0.29302359f,
0.28989318f, 0.28675964f, 0.28362307f, 0.28048345f, 0.27734083f,
0.27419522f, 0.27104670f, 0.26789525f, 0.26474094f, 0.26158381f,
0.25842386f, 0.25526115f, 0.25209570f, 0.24892756f, 0.24575676f,
0.24258332f, 0.23940729f, 0.23622867f, 0.23304754f, 0.22986393f,
0.22667783f, 0.22348931f, 0.22029841f, 0.21710514f, 0.21390954f,
0.21071166f, 0.20751151f, 0.20430915f, 0.20110460f, 0.19789790f,
0.19468907f, 0.19147816f, 0.18826519f, 0.18505022f, 0.18183327f,
0.17861435f, 0.17539354f, 0.17217083f, 0.16894630f, 0.16571994f,
0.16249183f, 0.15926196f, 0.15603039f, 0.15279715f, 0.14956227f,
0.14632578f, 0.14308774f, 0.13984816f, 0.13660708f, 0.13336454f,
0.13012058f, 0.12687522f, 0.12362850f, 0.12038045f, 0.11713112f,
0.11388054f, 0.11062872f, 0.10737573f, 0.10412160f, 0.10086634f,
0.097609997f, 0.094352618f, 0.091094226f, 0.087834857f, 0.084574550f,
0.081313334f, 0.078051247f, 0.074788325f, 0.071524605f, 0.068260118f,
0.064994894f, 0.061728980f, 0.058462404f, 0.055195201f, 0.051927410f,
0.048659060f, 0.045390189f, 0.042120833f, 0.038851023f, 0.035580799f,
0.032310195f, 0.029039243f, 0.025767982f, 0.022496443f, 0.019224664f,
0.015952680f, 0.012680525f, 0.0094082337f, 0.0061358409f, 0.0028633832f,
-0.00040910527f, -0.0036815894f, -0.0069540343f, -0.010226404f, -0.013498665f,
-0.016770782f, -0.020042717f, -0.023314439f, -0.026585912f, -0.029857099f,
-0.033127967f, -0.036398482f, -0.039668605f, -0.042938303f, -0.046207540f,
-0.049476285f, -0.052744497f, -0.056012146f, -0.059279196f, -0.062545612f,
-0.065811358f, -0.069076397f, -0.072340697f, -0.075604223f, -0.078866936f,
-0.082128808f, -0.085389800f, -0.088649876f, -0.091909006f, -0.095167145f,
-0.098424271f, -0.10168034f, -0.10493532f, -0.10818918f, -0.11144188f,
-0.11469338f, -0.11794366f, -0.12119267f, -0.12444039f, -0.12768677f,
-0.13093179f, -0.13417540f, -0.13741758f, -0.14065829f, -0.14389749f,
-0.14713514f, -0.15037122f, -0.15360570f, -0.15683852f, -0.16006967f,
-0.16329910f, -0.16652679f, -0.16975269f, -0.17297678f, -0.17619900f,
-0.17941935f, -0.18263777f, -0.18585424f, -0.18906870f, -0.19228116f,
-0.19549155f, -0.19869985f, -0.20190603f, -0.20511003f, -0.20831184f,
-0.21151142f, -0.21470875f, -0.21790376f, -0.22109644f, -0.22428675f,
-0.22747467f, -0.23066014f, -0.23384315f, -0.23702365f, -0.24020162f,
-0.24337701f, -0.24654980f, -0.24971995f, -0.25288740f, -0.25605217f,
-0.25921419f, -0.26237345f, -0.26552987f, -0.26868346f, -0.27183419f,
-0.27498198f, -0.27812684f, -0.28126872f, -0.28440759f, -0.28754342f,
-0.29067615f, -0.29380578f, -0.29693225f, -0.30005556f, -0.30317566f,
-0.30629250f, -0.30940607f, -0.31251630f, -0.31562322f, -0.31872672f,
-0.32182685f, -0.32492352f, -0.32801670f, -0.33110636f, -0.33419248f,
-0.33727503f, -0.34035397f, -0.34342924f, -0.34650084f, -0.34956875f,
-0.35263291f, -0.35569328f, -0.35874987f, -0.36180258f, -0.36485144f,
-0.36789638f, -0.37093741f, -0.37397444f, -0.37700745f, -0.38003644f,
-0.38306138f, -0.38608220f, -0.38909888f, -0.39211139f, -0.39511973f,
-0.39812380f, -0.40112361f, -0.40411916f, -0.40711036f, -0.41009718f,
-0.41307965f, -0.41605768f, -0.41903123f, -0.42200032f, -0.42496487f,
-0.42792490f, -0.43088034f, -0.43383113f, -0.43677729f, -0.43971881f,
-0.44265559f, -0.44558764f, -0.44851488f, -0.45143735f, -0.45435500f,
-0.45726776f, -0.46017563f, -0.46307856f, -0.46597654f, -0.46886954f,
-0.47175750f, -0.47464043f, -0.47751826f, -0.48039100f, -0.48325855f,
-0.48612097f, -0.48897815f, -0.49183011f, -0.49467680f, -0.49751821f,
-0.50035429f, -0.50318497f, -0.50601029f, -0.50883019f, -0.51164466f,
-0.51445359f, -0.51725709f, -0.52005500f, -0.52284735f, -0.52563411f,
-0.52841520f, -0.53119069f, -0.53396046f, -0.53672451f, -0.53948283f,
-0.54223537f, -0.54498214f, -0.54772300f, -0.55045801f, -0.55318713f,
-0.55591035f, -0.55862761f, -0.56133890f, -0.56404412f, -0.56674337f,
-0.56943649f, -0.57212353f, -0.57480448f, -0.57747924f, -0.58014780f,
-0.58281022f, -0.58546633f, -0.58811617f, -0.59075975f, -0.59339696f,
-0.59602785f, -0.59865236f, -0.60127044f, -0.60388207f, -0.60648727f,
-0.60908598f, -0.61167812f, -0.61426371f, -0.61684275f, -0.61941516f,
-0.62198097f, -0.62454009f, -0.62709254f, -0.62963831f, -0.63217729f,
-0.63470948f, -0.63723493f, -0.63975352f, -0.64226526f, -0.64477009f,
-0.64726806f, -0.64975911f, -0.65224314f, -0.65472025f, -0.65719032f,
-0.65965337f, -0.66210932f, -0.66455823f, -0.66700000f, -0.66943461f,
-0.67186207f, -0.67428231f, -0.67669535f, -0.67910111f, -0.68149966f,
-0.68389088f, -0.68627477f, -0.68865126f, -0.69102043f, -0.69338220f,
-0.69573659f, -0.69808346f, -0.70042288f, -0.70275480f, -0.70507920f,
-0.70739603f, -0.70970529f, -0.71200693f, -0.71430099f, -0.71658736f,
-0.71886611f, -0.72113711f, -0.72340041f, -0.72565591f, -0.72790372f,
-0.73014367f, -0.73237586f, -0.73460019f, -0.73681659f, -0.73902518f,
-0.74122584f, -0.74341851f, -0.74560326f, -0.74778003f, -0.74994880f,
-0.75210953f, -0.75426215f, -0.75640678f, -0.75854325f, -0.76067162f,
-0.76279181f, -0.76490390f, -0.76700771f, -0.76910341f, -0.77119076f,
-0.77326995f, -0.77534080f, -0.77740335f, -0.77945763f, -0.78150350f,
-0.78354102f, -0.78557014f, -0.78759086f, -0.78960317f, -0.79160696f,
-0.79360235f, -0.79558921f, -0.79756755f, -0.79953730f, -0.80149853f,
-0.80345118f, -0.80539525f, -0.80733067f, -0.80925739f, -0.81117553f,
-0.81308490f, -0.81498563f, -0.81687760f, -0.81876087f, -0.82063532f,
-0.82250100f, -0.82435787f, -0.82620591f, -0.82804507f, -0.82987541f,
-0.83169687f, -0.83350939f, -0.83531296f, -0.83710766f, -0.83889335f,
-0.84067005f, -0.84243774f, -0.84419644f, -0.84594607f, -0.84768665f,
-0.84941816f, -0.85114056f, -0.85285389f, -0.85455805f, -0.85625303f,
-0.85793889f, -0.85961550f, -0.86128294f, -0.86294121f, -0.86459017f,
-0.86622989f, -0.86786032f, -0.86948150f, -0.87109333f, -0.87269586f,
-0.87428904f, -0.87587279f, -0.87744725f, -0.87901229f, -0.88056785f,
-0.88211405f, -0.88365078f, -0.88517809f, -0.88669586f, -0.88820416f,
-0.88970292f, -0.89119220f, -0.89267188f, -0.89414203f, -0.89560264f,
-0.89705360f, -0.89849502f, -0.89992678f, -0.90134889f, -0.90276134f,
-0.90416414f, -0.90555727f, -0.90694070f, -0.90831441f, -0.90967834f,
-0.91103262f, -0.91237706f, -0.91371179f, -0.91503674f, -0.91635185f,
-0.91765714f, -0.91895264f, -0.92023826f, -0.92151409f, -0.92277998f,
-0.92403603f, -0.92528218f, -0.92651838f, -0.92774469f, -0.92896110f,
-0.93016750f, -0.93136400f, -0.93255049f, -0.93372697f, -0.93489349f,
-0.93604994f, -0.93719643f, -0.93833286f, -0.93945926f, -0.94057560f,
-0.94168180f, -0.94277799f, -0.94386405f, -0.94494003f, -0.94600588f,
-0.94706154f, -0.94810712f, -0.94914252f, -0.95016778f, -0.95118284f,
-0.95218778f, -0.95318246f, -0.95416695f, -0.95514119f, -0.95610523f,
-0.95705903f, -0.95800257f, -0.95893586f, -0.95985889f, -0.96077162f,
-0.96167403f, -0.96256620f, -0.96344805f, -0.96431959f, -0.96518075f,
-0.96603161f, -0.96687216f, -0.96770233f, -0.96852213f, -0.96933156f,
-0.97013056f, -0.97091925f, -0.97169751f, -0.97246534f, -0.97322279f,
-0.97396982f, -0.97470641f, -0.97543252f, -0.97614825f, -0.97685349f,
-0.97754824f, -0.97823256f, -0.97890645f, -0.97956979f, -0.98022264f,
-0.98086500f, -0.98149687f, -0.98211825f, -0.98272908f, -0.98332942f,
-0.98391914f, -0.98449844f, -0.98506713f, -0.98562527f, -0.98617285f,
-0.98670989f, -0.98723638f, -0.98775226f, -0.98825759f, -0.98875231f,
-0.98923647f, -0.98971003f, -0.99017298f, -0.99062532f, -0.99106705f,
-0.99149817f, -0.99191868f, -0.99232858f, -0.99272782f, -0.99311644f,
-0.99349445f, -0.99386179f, -0.99421853f, -0.99456459f, -0.99489999f,
-0.99522477f, -0.99553883f, -0.99584228f, -0.99613506f, -0.99641716f,
-0.99668860f, -0.99694937f, -0.99719942f, -0.99743885f, -0.99766755f,
-0.99788558f, -0.99809295f, -0.99828959f, -0.99847561f, -0.99865085f,
-0.99881548f, -0.99896932f, -0.99911255f, -0.99924499f, -0.99936682f,
-0.99947786f, -0.99957830f, -0.99966794f, -0.99974692f, -0.99981517f,
-0.99987274f, -0.99991959f, -0.99995571f, -0.99998116f, -0.99999589f,
0.99999964f, 0.99997288f, 0.99990326f, 0.99979085f, 0.99963558f,
0.99943751f, 0.99919659f, 0.99891287f, 0.99858636f, 0.99821711f,
0.99780506f, 0.99735034f, 0.99685282f, 0.99631262f, 0.99572974f,
0.99510419f, 0.99443603f, 0.99372530f, 0.99297196f, 0.99217612f,
0.99133772f, 0.99045694f, 0.98953366f, 0.98856801f, 0.98756003f,
0.98650974f, 0.98541719f, 0.98428243f, 0.98310548f, 0.98188645f,
0.98062533f, 0.97932225f, 0.97797716f, 0.97659022f, 0.97516143f,
0.97369087f, 0.97217858f, 0.97062469f, 0.96902919f, 0.96739221f,
0.96571374f, 0.96399397f, 0.96223283f, 0.96043050f, 0.95858705f,
0.95670253f, 0.95477700f, 0.95281059f, 0.95080340f, 0.94875544f,
0.94666684f, 0.94453770f, 0.94236809f, 0.94015813f, 0.93790787f,
0.93561745f, 0.93328691f, 0.93091643f, 0.92850608f, 0.92605597f,
0.92356616f, 0.92103678f, 0.91846794f, 0.91585976f, 0.91321236f,
0.91052586f, 0.90780038f, 0.90503591f, 0.90223277f, 0.89939094f,
0.89651060f, 0.89359182f, 0.89063478f, 0.88763964f, 0.88460642f,
0.88153529f, 0.87842643f, 0.87527996f, 0.87209594f, 0.86887461f,
0.86561602f, 0.86232042f, 0.85898781f, 0.85561842f, 0.85221243f,
0.84876984f, 0.84529096f, 0.84177583f, 0.83822471f, 0.83463764f,
0.83101481f, 0.82735640f, 0.82366252f, 0.81993335f, 0.81616908f,
0.81236988f, 0.80853581f, 0.80466717f, 0.80076402f, 0.79682660f,
0.79285502f, 0.78884947f, 0.78481019f, 0.78073722f, 0.77663082f,
0.77249116f, 0.76831841f, 0.76411277f, 0.75987434f, 0.75560343f,
0.75130010f, 0.74696463f, 0.74259710f, 0.73819780f, 0.73376691f,
0.72930455f, 0.72481096f, 0.72028631f, 0.71573079f, 0.71114463f,
0.70652801f, 0.70188117f, 0.69720417f, 0.69249737f, 0.68776089f,
0.68299496f, 0.67819971f, 0.67337549f, 0.66852236f, 0.66364062f,
0.65873051f, 0.65379208f, 0.64882571f, 0.64383155f, 0.63880974f,
0.63376063f, 0.62868434f, 0.62358117f, 0.61845124f, 0.61329484f,
0.60811216f, 0.60290343f, 0.59766883f, 0.59240872f, 0.58712316f,
0.58181250f, 0.57647687f, 0.57111657f, 0.56573176f, 0.56032276f,
0.55488980f, 0.54943299f, 0.54395270f, 0.53844911f, 0.53292239f,
0.52737290f, 0.52180082f, 0.51620632f, 0.51058978f, 0.50495136f,
0.49929130f, 0.49360985f, 0.48790723f, 0.48218375f, 0.47643960f,
0.47067502f, 0.46489030f, 0.45908567f, 0.45326138f, 0.44741765f,
0.44155475f, 0.43567297f, 0.42977250f, 0.42385364f, 0.41791660f,
0.41196167f, 0.40598908f, 0.39999911f, 0.39399201f, 0.38796803f,
0.38192743f, 0.37587047f, 0.36979741f, 0.36370850f, 0.35760403f,
0.35148421f, 0.34534934f, 0.33919969f, 0.33303553f, 0.32685706f,
0.32066461f, 0.31445843f, 0.30823877f, 0.30200592f, 0.29576012f,
0.28950164f, 0.28323078f, 0.27694780f, 0.27065292f, 0.26434645f,
0.25802869f, 0.25169984f, 0.24536023f, 0.23901010f, 0.23264973f,
0.22627939f, 0.21989937f, 0.21350993f, 0.20711134f, 0.20070387f,
0.19428782f, 0.18786344f, 0.18143101f, 0.17499080f, 0.16854310f,
0.16208819f, 0.15562633f, 0.14915779f, 0.14268288f, 0.13620184f,
0.12971498f, 0.12322257f, 0.11672486f, 0.11022217f, 0.10371475f,
0.097202882f, 0.090686858f, 0.084166944f, 0.077643424f, 0.071116582f,
0.064586692f, 0.058054037f, 0.051518895f, 0.044981543f, 0.038442269f,
0.031901345f, 0.025359053f, 0.018815678f, 0.012271495f, 0.0057267868f,
-0.00081816671f, -0.0073630852f, -0.013907688f, -0.020451695f, -0.026994826f,
-0.033536803f, -0.040077340f, -0.046616159f, -0.053152986f, -0.059687532f,
-0.066219524f, -0.072748676f, -0.079274714f, -0.085797355f, -0.092316322f,
-0.098831341f, -0.10534211f, -0.11184838f, -0.11834986f, -0.12484626f,
-0.13133731f, -0.13782275f, -0.14430228f, -0.15077563f, -0.15724251f,
-0.16370267f, -0.17015581f, -0.17660165f, -0.18303993f, -0.18947038f,
-0.19589271f, -0.20230664f, -0.20871192f, -0.21510825f, -0.22149536f,
-0.22787298f, -0.23424086f, -0.24059868f, -0.24694622f, -0.25328314f,
-0.25960925f, -0.26592422f, -0.27222782f, -0.27851975f, -0.28479972f,
-0.29106751f, -0.29732284f, -0.30356544f, -0.30979502f, -0.31601134f,
-0.32221413f, -0.32840309f, -0.33457801f, -0.34073856f, -0.34688455f,
-0.35301566f, -0.35913166f, -0.36523229f, -0.37131724f, -0.37738630f,
-0.38343921f, -0.38947567f, -0.39549544f, -0.40149832f, -0.40748394f,
-0.41345215f, -0.41940263f, -0.42533514f, -0.43124944f, -0.43714526f,
-0.44302234f, -0.44888046f, -0.45471936f, -0.46053877f, -0.46633846f,
-0.47211814f, -0.47787762f, -0.48361665f, -0.48933494f, -0.49503228f,
-0.50070840f, -0.50636309f, -0.51199609f, -0.51760709f, -0.52319598f,
-0.52876246f, -0.53430629f, -0.53982723f, -0.54532504f, -0.55079949f,
-0.55625033f, -0.56167740f, -0.56708032f, -0.57245898f, -0.57781315f,
-0.58314258f, -0.58844697f, -0.59372622f, -0.59897995f, -0.60420811f,
-0.60941035f, -0.61458647f, -0.61973625f, -0.62485951f, -0.62995601f,
-0.63502556f, -0.64006782f, -0.64508271f, -0.65007001f, -0.65502942f,
-0.65996075f, -0.66486382f, -0.66973841f, -0.67458433f, -0.67940134f,
-0.68418926f, -0.68894786f, -0.69367695f, -0.69837630f, -0.70304573f,
-0.70768511f, -0.71229410f, -0.71687263f, -0.72142041f, -0.72593731f,
-0.73042315f, -0.73487765f, -0.73930067f, -0.74369204f, -0.74805158f,
-0.75237900f, -0.75667429f, -0.76093709f, -0.76516730f, -0.76936477f,
-0.77352923f, -0.77766061f, -0.78175867f, -0.78582323f, -0.78985411f,
-0.79385114f, -0.79781419f, -0.80174309f, -0.80563760f, -0.80949765f,
-0.81332302f, -0.81711352f, -0.82086903f, -0.82458937f, -0.82827437f,
-0.83192390f, -0.83553779f, -0.83911592f, -0.84265804f, -0.84616417f,
-0.84963393f, -0.85306740f, -0.85646427f, -0.85982448f, -0.86314780f,
-0.86643422f, -0.86968350f, -0.87289548f, -0.87607014f, -0.87920725f,
-0.88230664f, -0.88536829f, -0.88839203f, -0.89137769f, -0.89432514f,
-0.89723432f, -0.90010506f, -0.90293723f, -0.90573072f, -0.90848541f,
-0.91120118f, -0.91387796f, -0.91651553f, -0.91911387f, -0.92167282f,
-0.92419231f, -0.92667222f, -0.92911243f, -0.93151283f, -0.93387336f,
-0.93619382f, -0.93847424f, -0.94071442f, -0.94291431f, -0.94507378f,
-0.94719279f, -0.94927126f, -0.95130903f, -0.95330608f, -0.95526224f,
-0.95717752f, -0.95905179f, -0.96088499f, -0.96267700f, -0.96442777f,
-0.96613729f, -0.96780539f, -0.96943200f, -0.97101706f, -0.97256058f,
-0.97406244f, -0.97552258f, -0.97694093f, -0.97831738f, -0.97965199f,
-0.98094457f, -0.98219514f, -0.98340368f, -0.98457009f, -0.98569429f,
-0.98677629f, -0.98781598f, -0.98881340f, -0.98976845f, -0.99068111f,
-0.99155134f, -0.99237907f, -0.99316430f, -0.99390697f, -0.99460709f,
-0.99526459f, -0.99587947f, -0.99645168f, -0.99698120f, -0.99746799f,
-0.99791211f, -0.99831343f, -0.99867201f, -0.99898779f, -0.99926084f,
-0.99949104f, -0.99967843f, -0.99982297f, -0.99992472f, -0.99998361f,
0.99999869f, 0.99989158f, 0.99961317f, 0.99916345f, 0.99854255f,
0.99775058f, 0.99678761f, 0.99565387f, 0.99434954f, 0.99287480f,
0.99122995f, 0.98941529f, 0.98743105f, 0.98527765f, 0.98295540f,
0.98046476f, 0.97780609f, 0.97497988f, 0.97198665f, 0.96882683f,
0.96550101f, 0.96200979f, 0.95835376f, 0.95453346f, 0.95054960f,
0.94640291f, 0.94209403f, 0.93762374f, 0.93299282f, 0.92820197f,
0.92325211f, 0.91814411f, 0.91287869f, 0.90745693f, 0.90187967f,
0.89614785f, 0.89026248f, 0.88422459f, 0.87803519f, 0.87169534f,
0.86520612f, 0.85856867f, 0.85178405f, 0.84485358f, 0.83777827f,
0.83055943f, 0.82319832f, 0.81569612f, 0.80805415f, 0.80027372f,
0.79235619f, 0.78430289f, 0.77611518f, 0.76779449f, 0.75934225f,
0.75075996f, 0.74204898f, 0.73321080f, 0.72424710f, 0.71515924f,
0.70594883f, 0.69661748f, 0.68716675f, 0.67759830f, 0.66791373f,
0.65811473f, 0.64820296f, 0.63818014f, 0.62804794f, 0.61780810f,
0.60746247f, 0.59701276f, 0.58646071f, 0.57580817f, 0.56505698f,
0.55420899f, 0.54326600f, 0.53222996f, 0.52110273f, 0.50988621f,
0.49858227f, 0.48719296f, 0.47572014f, 0.46416581f, 0.45253196f,
0.44082057f, 0.42903364f, 0.41717321f, 0.40524128f, 0.39323992f,
0.38117120f, 0.36903715f, 0.35683987f, 0.34458145f, 0.33226398f,
0.31988961f, 0.30746040f, 0.29497850f, 0.28244606f, 0.26986524f,
0.25723818f, 0.24456702f, 0.23185398f, 0.21910121f, 0.20631088f,
0.19348522f, 0.18062639f, 0.16773662f, 0.15481812f, 0.14187308f,
0.12890373f, 0.11591230f, 0.10290100f, 0.089872077f, 0.076827750f,
0.063770257f, 0.050701842f, 0.037624735f, 0.024541186f, 0.011453429f,
-0.0016362892f, -0.014725727f, -0.027812643f, -0.040894791f, -0.053969935f,
-0.067035832f, -0.080090240f, -0.093130924f, -0.10615565f, -0.11916219f,
-0.13214831f, -0.14511178f, -0.15805040f, -0.17096193f, -0.18384418f,
-0.19669491f, -0.20951195f, -0.22229309f, -0.23503613f, -0.24773891f,
-0.26039925f, -0.27301496f, -0.28558388f, -0.29810387f, -0.31057280f,
-0.32298848f, -0.33534884f, -0.34765175f, -0.35989508f, -0.37207675f,
-0.38419467f, -0.39624676f, -0.40823093f, -0.42014518f, -0.43198743f,
-0.44375566f, -0.45544785f, -0.46706200f, -0.47859612f, -0.49004826f,
-0.50141639f, -0.51269865f, -0.52389306f, -0.53499764f, -0.54601061f,
-0.55693001f, -0.56775403f, -0.57848072f, -0.58910829f, -0.59963489f,
-0.61005878f, -0.62037814f, -0.63059121f, -0.64069623f, -0.65069145f,
-0.66057515f, -0.67034572f, -0.68000144f, -0.68954057f, -0.69896162f,
-0.70826286f, -0.71744281f, -0.72649974f, -0.73543227f, -0.74423873f,
-0.75291771f, -0.76146764f, -0.76988715f, -0.77817470f, -0.78632891f,
-0.79434842f, -0.80223179f, -0.80997771f, -0.81758487f, -0.82505190f,
-0.83237761f, -0.83956063f, -0.84659988f, -0.85349399f, -0.86024189f,
-0.86684239f, -0.87329435f, -0.87959671f, -0.88574833f, -0.89174819f,
-0.89759529f, -0.90328854f, -0.90882701f, -0.91420978f, -0.91943592f,
-0.92450452f, -0.92941469f, -0.93416560f, -0.93875647f, -0.94318646f,
-0.94745487f, -0.95156091f, -0.95550388f, -0.95928317f, -0.96289814f,
-0.96634805f, -0.96963239f, -0.97275060f, -0.97570217f, -0.97848648f,
-0.98110318f, -0.98355180f, -0.98583186f, -0.98794299f, -0.98988485f,
-0.99165714f, -0.99325943f, -0.99469161f, -0.99595332f, -0.99704438f,
-0.99796462f, -0.99871385f, -0.99929196f, -0.99969882f, -0.99993443f,
0.99999464f, 0.99956632f, 0.99845290f, 0.99665523f, 0.99417448f,
0.99101239f, 0.98717111f, 0.98265326f, 0.97746199f, 0.97160077f,
0.96507365f, 0.95788515f, 0.95004016f, 0.94154406f, 0.93240267f,
0.92262226f, 0.91220951f, 0.90117162f, 0.88951606f, 0.87725091f,
0.86438453f, 0.85092574f, 0.83688372f, 0.82226819f, 0.80708915f,
0.79135692f, 0.77508235f, 0.75827658f, 0.74095112f, 0.72311783f,
0.70478898f, 0.68597710f, 0.66669506f, 0.64695615f, 0.62677377f,
0.60616189f, 0.58513457f, 0.56370622f, 0.54189157f, 0.51970547f,
0.49716324f, 0.47428027f, 0.45107225f, 0.42755505f, 0.40374488f,
0.37965798f, 0.35531086f, 0.33072025f, 0.30590299f, 0.28087607f,
0.25565663f, 0.23026201f, 0.20470956f, 0.17901683f, 0.15320139f,
0.12728097f, 0.10127331f, 0.075196236f, 0.049067631f, 0.022905400f,
-0.0032725304f, -0.029448219f, -0.055603724f, -0.081721120f, -0.10778251f,
-0.13377003f, -0.15966587f, -0.18545228f, -0.21111161f, -0.23662624f,
-0.26197869f, -0.28715160f, -0.31212771f, -0.33688989f, -0.36142120f,
-0.38570482f, -0.40972409f, -0.43346253f, -0.45690393f, -0.48003218f,
-0.50283146f, -0.52528608f, -0.54738069f, -0.56910020f, -0.59042966f,
-0.61135447f, -0.63186026f, -0.65193301f, -0.67155898f, -0.69072473f,
-0.70941705f, -0.72762316f, -0.74533063f, -0.76252723f, -0.77920127f,
-0.79534131f, -0.81093621f, -0.82597536f, -0.84044844f, -0.85434550f,
-0.86765707f, -0.88037395f, -0.89248747f, -0.90398932f, -0.91487163f,
-0.92512697f, -0.93474823f, -0.94372886f, -0.95206273f, -0.95974404f,
-0.96676767f, -0.97312868f, -0.97882277f, -0.98384601f, -0.98819500f,
-0.99186671f, -0.99485862f, -0.99716878f, -0.99879545f, -0.99973762f,
};


static const OpusCustomMode mode48000_960_120 = {
48000,
120,
21,
21,
{0.85000610f, 0.0000000f, 1.0000000f, 1.0000000f, },
eband5ms,
3,
8,
120,
11,
band_allocation,
logN400,
window120,
{1920, 3, {&fft_state48000_960_0, &fft_state48000_960_1, &fft_state48000_960_2, &fft_state48000_960_3, }, mdct_twiddles960},
{392, cache_index50, cache_bits50, cache_caps50},
};



static const OpusCustomMode * const static_mode_list[1] = {
&mode48000_960_120,
};
static const opus_int16 bark_freq[25 +1] = {
      0, 100, 200, 300, 400,
    510, 630, 770, 920, 1080,
   1270, 1480, 1720, 2000, 2320,
   2700, 3150, 3700, 4400, 5300,
   6400, 7700, 9500, 12000, 15500,
  20000};

static opus_int16 *compute_ebands(opus_int32 Fs, int frame_size, int res, int *nbEBands)
{
   opus_int16 *eBands;
   int i, j, lin, low, high, nBark, offset=0;


   if (Fs == 400*(opus_int32)frame_size)
   {
      *nbEBands = sizeof(eband5ms)/sizeof(eband5ms[0])-1;
      eBands = opus_alloc(sizeof(opus_int16)*(*nbEBands+1));
      for (i=0;i<*nbEBands+1;i++)
         eBands[i] = eband5ms[i];
      return eBands;
   }

   for (nBark=1;nBark<25;nBark++)
    if (bark_freq[nBark+1]*2 >= Fs)
       break;


   for (lin=0;lin<nBark;lin++)
      if (bark_freq[lin+1]-bark_freq[lin] >= res)
         break;

   low = (bark_freq[lin]+res/2)/res;
   high = nBark-lin;
   *nbEBands = low+high;
   eBands = opus_alloc(sizeof(opus_int16)*(*nbEBands+2));

   if (eBands==
              ((void *)0)
                  )
      return 
            ((void *)0)
                ;


   for (i=0;i<low;i++)
      eBands[i] = i;
   if (low>0)
      offset = eBands[low-1]*res - bark_freq[lin-1];

   for (i=0;i<high;i++)
   {
      int target = bark_freq[lin+i];

      eBands[i+low] = (target+offset/2+res)/(2*res)*2;
      offset = eBands[i+low]*res - target;
   }

   for (i=0;i<*nbEBands;i++)
      if (eBands[i] < i)
         eBands[i] = i;

   eBands[*nbEBands] = (bark_freq[nBark]+res)/(2*res)*2;
   if (eBands[*nbEBands] > frame_size)
      eBands[*nbEBands] = frame_size;
   for (i=1;i<*nbEBands-1;i++)
   {
      if (eBands[i+1]-eBands[i] < eBands[i]-eBands[i-1])
      {
         eBands[i] -= (2*eBands[i]-eBands[i-1]-eBands[i+1])/2;
      }
   }

   for (i=j=0;i<*nbEBands;i++)
      if(eBands[i+1]>eBands[j])
         eBands[++j]=eBands[i+1];
   *nbEBands=j;

   for (i=1;i<*nbEBands;i++)
   {

      ;

      ;
   }

   return eBands;
}

static void compute_allocation_table(OpusCustomMode *mode)
{
   int i, j;
   unsigned char *allocVectors;
   int maxBands = sizeof(eband5ms)/sizeof(eband5ms[0])-1;

   mode->nbAllocVectors = 11;
   allocVectors = opus_alloc(sizeof(unsigned char)*(11*mode->nbEBands));
   if (allocVectors==
                    ((void *)0)
                        )
   {
      mode->allocVectors = 
                          ((void *)0)
                              ;
      return;
   }


   if (mode->Fs == 400*(opus_int32)mode->shortMdctSize)
   {
      for (i=0;i<11*mode->nbEBands;i++)
         allocVectors[i] = band_allocation[i];
      mode->allocVectors = allocVectors;
      return;
   }


   for (i=0;i<11;i++)
   {
      for (j=0;j<mode->nbEBands;j++)
      {
         int k;
         for (k=0;k<maxBands;k++)
         {
            if (400*(opus_int32)eband5ms[k] > mode->eBands[j]*(opus_int32)mode->Fs/mode->shortMdctSize)
               break;
         }
         if (k>maxBands-1)
            allocVectors[i*mode->nbEBands+j] = band_allocation[i*maxBands + maxBands-1];
         else {
            opus_int32 a0, a1;
            a1 = mode->eBands[j]*(opus_int32)mode->Fs/mode->shortMdctSize - 400*(opus_int32)eband5ms[k-1];
            a0 = 400*(opus_int32)eband5ms[k] - mode->eBands[j]*(opus_int32)mode->Fs/mode->shortMdctSize;
            allocVectors[i*mode->nbEBands+j] = (a0*band_allocation[i*maxBands+k-1]
                                             + a1*band_allocation[i*maxBands+k])/(a0+a1);
         }
      }
   }
   mode->allocVectors = allocVectors;
}



OpusCustomMode *opus_custom_mode_create(opus_int32 Fs, int frame_size, int *error)
{
   int i;

   OpusCustomMode *mode=
                 ((void *)0)
                     ;
   int res;
   opus_val16 *window;
   opus_int16 *logN;
   int LM;
   int arch = opus_select_arch();
   ;







   for (i=0;i<1;i++)
   {
      int j;
      for (j=0;j<4;j++)
      {
         if (Fs == static_mode_list[i]->Fs &&
               (frame_size<<j) == static_mode_list[i]->shortMdctSize*static_mode_list[i]->nbShortMdcts)
         {
            if (error)
               *error = 0;
            return (OpusCustomMode*)static_mode_list[i];
         }
      }
   }
   if (Fs < 8000 || Fs > 96000)
   {
      if (error)
         *error = -1;
      return 
            ((void *)0)
                ;
   }
   if (frame_size < 40 || frame_size > 1024 || frame_size%2!=0)
   {
      if (error)
         *error = -1;
      return 
            ((void *)0)
                ;
   }

   if ((opus_int32)frame_size*1000 < Fs)
   {
      if (error)
         *error = -1;
      return 
            ((void *)0)
                ;
   }

   if ((opus_int32)frame_size*75 >= Fs && (frame_size%16)==0)
   {
     LM = 3;
   } else if ((opus_int32)frame_size*150 >= Fs && (frame_size%8)==0)
   {
     LM = 2;
   } else if ((opus_int32)frame_size*300 >= Fs && (frame_size%4)==0)
   {
     LM = 1;
   } else
   {
     LM = 0;
   }


   if ((opus_int32)(frame_size>>LM)*300 > Fs)
   {
      if (error)
         *error = -1;
      return 
            ((void *)0)
                ;
   }

   mode = opus_alloc(sizeof(OpusCustomMode));
   if (mode==
            ((void *)0)
                )
      goto failure;
   mode->Fs = Fs;




   if(Fs < 12000)
   {
      mode->preemph[0] = (0.3500061035f);
      mode->preemph[1] = -(0.1799926758f);
      mode->preemph[2] = (0.2719968125f);
      mode->preemph[3] = (3.6765136719f);
   } else if(Fs < 24000)
   {
      mode->preemph[0] = (0.6000061035f);
      mode->preemph[1] = -(0.1799926758f);
      mode->preemph[2] = (0.4424998650f);
      mode->preemph[3] = (2.2598876953f);
   } else if(Fs < 40000)
   {
      mode->preemph[0] = (0.7799987793f);
      mode->preemph[1] = -(0.1000061035f);
      mode->preemph[2] = (0.7499771125f);
      mode->preemph[3] = (1.3333740234f);
   } else
   {
      mode->preemph[0] = (0.8500061035f);
      mode->preemph[1] = (0.0f);
      mode->preemph[2] = (1.f);
      mode->preemph[3] = (1.f);
   }

   mode->maxLM = LM;
   mode->nbShortMdcts = 1<<LM;
   mode->shortMdctSize = frame_size/mode->nbShortMdcts;
   res = (mode->Fs+mode->shortMdctSize)/(2*mode->shortMdctSize);

   mode->eBands = compute_ebands(Fs, mode->shortMdctSize, res, &mode->nbEBands);
   if (mode->eBands==
                    ((void *)0)
                        )
      goto failure;



   if ((mode->eBands[mode->nbEBands] - mode->eBands[mode->nbEBands-1])<<LM >
    208) {
       goto failure;
   }


   mode->effEBands = mode->nbEBands;
   while (mode->eBands[mode->effEBands] > mode->shortMdctSize)
      mode->effEBands--;


   mode->overlap = ((mode->shortMdctSize>>2)<<2);

   compute_allocation_table(mode);
   if (mode->allocVectors==
                          ((void *)0)
                              )
      goto failure;

   window = (opus_val16*)opus_alloc(mode->overlap*sizeof(opus_val16));
   if (window==
              ((void *)0)
                  )
      goto failure;


   for (i=0;i<mode->overlap;i++)
      window[i] = (float)sin(.5*
                               3.14159265358979323846
                                   * sin(.5*
                                            3.14159265358979323846
                                                *(i+.5)/mode->overlap) * sin(.5*
                                                                                3.14159265358979323846
                                                                                    *(i+.5)/mode->overlap));




   mode->window = window;

   logN = (opus_int16*)opus_alloc(mode->nbEBands*sizeof(opus_int16));
   if (logN==
            ((void *)0)
                )
      goto failure;

   for (i=0;i<mode->nbEBands;i++)
      logN[i] = log2_frac(mode->eBands[i+1]-mode->eBands[i], 3);
   mode->logN = logN;

   compute_pulse_cache(mode, mode->maxLM);

   if (clt_mdct_init(&mode->mdct, 2*mode->shortMdctSize*mode->nbShortMdcts,
           mode->maxLM, arch) == 0)
      goto failure;

   if (error)
      *error = 0;

   return mode;
failure:
   if (error)
      *error = -7;
   if (mode!=
            ((void *)0)
                )
      opus_custom_mode_destroy(mode);
   return 
         ((void *)0)
             ;

}


void opus_custom_mode_destroy(OpusCustomMode *mode)
{
   int arch = opus_select_arch();

   if (mode == 
              ((void *)0)
                  )
      return;

   {
     int i;
     for (i=0;i<1;i++)
     {
        if (mode == static_mode_list[i])
        {
           return;
        }
     }
   }

   opus_free((opus_int16*)mode->eBands);
   opus_free((unsigned char*)mode->allocVectors);

   opus_free((opus_val16*)mode->window);
   opus_free((opus_int16*)mode->logN);

   opus_free((opus_int16*)mode->cache.index);
   opus_free((unsigned char*)mode->cache.bits);
   opus_free((unsigned char*)mode->cache.caps);
   clt_mdct_clear(&mode->mdct, arch);

   opus_free((OpusCustomMode *)mode);
}
static void kf_bfly2(
                     kiss_fft_cpx * Fout,
                     int m,
                     int N
                    )
{
   kiss_fft_cpx * Fout2;
   int i;
   (void)m;

   if (m==1)
   {
      ;
      for (i=0;i<N;i++)
      {
         kiss_fft_cpx t;
         Fout2 = Fout + 1;
         t = *Fout2;
         do { (*Fout2).r=(*Fout).r-(t).r; (*Fout2).i=(*Fout).i-(t).i; }while(0);
         do { (*Fout).r += (t).r; (*Fout).i += (t).i; }while(0);
         Fout += 2;
      }
   } else

   {
      opus_val16 tw;
      tw = (0.7071067812f);

      ;
      for (i=0;i<N;i++)
      {
         kiss_fft_cpx t;
         Fout2 = Fout + 4;
         t = Fout2[0];
         do { (Fout2[0]).r=(Fout[0]).r-(t).r; (Fout2[0]).i=(Fout[0]).i-(t).i; }while(0);
         do { (Fout[0]).r += (t).r; (Fout[0]).i += (t).i; }while(0);

         t.r = ( (((Fout2[1].r)+(Fout2[1].i)))*(tw) );
         t.i = ( (((Fout2[1].i)-(Fout2[1].r)))*(tw) );
         do { (Fout2[1]).r=(Fout[1]).r-(t).r; (Fout2[1]).i=(Fout[1]).i-(t).i; }while(0);
         do { (Fout[1]).r += (t).r; (Fout[1]).i += (t).i; }while(0);

         t.r = Fout2[2].i;
         t.i = -Fout2[2].r;
         do { (Fout2[2]).r=(Fout[2]).r-(t).r; (Fout2[2]).i=(Fout[2]).i-(t).i; }while(0);
         do { (Fout[2]).r += (t).r; (Fout[2]).i += (t).i; }while(0);

         t.r = ( (((Fout2[3].i)-(Fout2[3].r)))*(tw) );
         t.i = ( ((-(((Fout2[3].i)+(Fout2[3].r)))))*(tw) );
         do { (Fout2[3]).r=(Fout[3]).r-(t).r; (Fout2[3]).i=(Fout[3]).i-(t).i; }while(0);
         do { (Fout[3]).r += (t).r; (Fout[3]).i += (t).i; }while(0);
         Fout += 8;
      }
   }
}

static void kf_bfly4(
                     kiss_fft_cpx * Fout,
                     const size_t fstride,
                     const kiss_fft_state *st,
                     int m,
                     int N,
                     int mm
                    )
{
   int i;

   if (m==1)
   {

      for (i=0;i<N;i++)
      {
         kiss_fft_cpx scratch0, scratch1;

         do { (scratch0).r=(*Fout).r-(Fout[2]).r; (scratch0).i=(*Fout).i-(Fout[2]).i; }while(0);
         do { (*Fout).r += (Fout[2]).r; (*Fout).i += (Fout[2]).i; }while(0);
         do { (scratch1).r=(Fout[1]).r+(Fout[3]).r; (scratch1).i=(Fout[1]).i+(Fout[3]).i; }while(0);
         do { (Fout[2]).r=(*Fout).r-(scratch1).r; (Fout[2]).i=(*Fout).i-(scratch1).i; }while(0);
         do { (*Fout).r += (scratch1).r; (*Fout).i += (scratch1).i; }while(0);
         do { (scratch1).r=(Fout[1]).r-(Fout[3]).r; (scratch1).i=(Fout[1]).i-(Fout[3]).i; }while(0);

         Fout[1].r = ((scratch0.r)+(scratch1.i));
         Fout[1].i = ((scratch0.i)-(scratch1.r));
         Fout[3].r = ((scratch0.r)-(scratch1.i));
         Fout[3].i = ((scratch0.i)+(scratch1.r));
         Fout+=4;
      }
   } else {
      int j;
      kiss_fft_cpx scratch[6];
      const kiss_twiddle_cpx *tw1,*tw2,*tw3;
      const int m2=2*m;
      const int m3=3*m;
      kiss_fft_cpx * Fout_beg = Fout;
      for (i=0;i<N;i++)
      {
         Fout = Fout_beg + i*mm;
         tw3 = tw2 = tw1 = st->twiddles;

         for (j=0;j<m;j++)
         {
            do{ (scratch[0]).r = (Fout[m]).r*(*tw1).r - (Fout[m]).i*(*tw1).i; (scratch[0]).i = (Fout[m]).r*(*tw1).i + (Fout[m]).i*(*tw1).r; }while(0);
            do{ (scratch[1]).r = (Fout[m2]).r*(*tw2).r - (Fout[m2]).i*(*tw2).i; (scratch[1]).i = (Fout[m2]).r*(*tw2).i + (Fout[m2]).i*(*tw2).r; }while(0);
            do{ (scratch[2]).r = (Fout[m3]).r*(*tw3).r - (Fout[m3]).i*(*tw3).i; (scratch[2]).i = (Fout[m3]).r*(*tw3).i + (Fout[m3]).i*(*tw3).r; }while(0);

            do { (scratch[5]).r=(*Fout).r-(scratch[1]).r; (scratch[5]).i=(*Fout).i-(scratch[1]).i; }while(0);
            do { (*Fout).r += (scratch[1]).r; (*Fout).i += (scratch[1]).i; }while(0);
            do { (scratch[3]).r=(scratch[0]).r+(scratch[2]).r; (scratch[3]).i=(scratch[0]).i+(scratch[2]).i; }while(0);
            do { (scratch[4]).r=(scratch[0]).r-(scratch[2]).r; (scratch[4]).i=(scratch[0]).i-(scratch[2]).i; }while(0);
            do { (Fout[m2]).r=(*Fout).r-(scratch[3]).r; (Fout[m2]).i=(*Fout).i-(scratch[3]).i; }while(0);
            tw1 += fstride;
            tw2 += fstride*2;
            tw3 += fstride*3;
            do { (*Fout).r += (scratch[3]).r; (*Fout).i += (scratch[3]).i; }while(0);

            Fout[m].r = ((scratch[5].r)+(scratch[4].i));
            Fout[m].i = ((scratch[5].i)-(scratch[4].r));
            Fout[m3].r = ((scratch[5].r)-(scratch[4].i));
            Fout[m3].i = ((scratch[5].i)+(scratch[4].r));
            ++Fout;
         }
      }
   }
}




static void kf_bfly3(
                     kiss_fft_cpx * Fout,
                     const size_t fstride,
                     const kiss_fft_state *st,
                     int m,
                     int N,
                     int mm
                    )
{
   int i;
   size_t k;
   const size_t m2 = 2*m;
   const kiss_twiddle_cpx *tw1,*tw2;
   kiss_fft_cpx scratch[5];
   kiss_twiddle_cpx epi3;

   kiss_fft_cpx * Fout_beg = Fout;




   epi3 = st->twiddles[fstride*m];

   for (i=0;i<N;i++)
   {
      Fout = Fout_beg + i*mm;
      tw1=tw2=st->twiddles;

      k=m;
      do {

         do{ (scratch[1]).r = (Fout[m]).r*(*tw1).r - (Fout[m]).i*(*tw1).i; (scratch[1]).i = (Fout[m]).r*(*tw1).i + (Fout[m]).i*(*tw1).r; }while(0);
         do{ (scratch[2]).r = (Fout[m2]).r*(*tw2).r - (Fout[m2]).i*(*tw2).i; (scratch[2]).i = (Fout[m2]).r*(*tw2).i + (Fout[m2]).i*(*tw2).r; }while(0);

         do { (scratch[3]).r=(scratch[1]).r+(scratch[2]).r; (scratch[3]).i=(scratch[1]).i+(scratch[2]).i; }while(0);
         do { (scratch[0]).r=(scratch[1]).r-(scratch[2]).r; (scratch[0]).i=(scratch[1]).i-(scratch[2]).i; }while(0);
         tw1 += fstride;
         tw2 += fstride*2;

         Fout[m].r = ((Fout->r)-(((scratch[3].r)*.5f)));
         Fout[m].i = ((Fout->i)-(((scratch[3].i)*.5f)));

         do{ (scratch[0]).r *= (epi3.i); (scratch[0]).i *= (epi3.i); }while(0);

         do { (*Fout).r += (scratch[3]).r; (*Fout).i += (scratch[3]).i; }while(0);

         Fout[m2].r = ((Fout[m].r)+(scratch[0].i));
         Fout[m2].i = ((Fout[m].i)-(scratch[0].r));

         Fout[m].r = ((Fout[m].r)-(scratch[0].i));
         Fout[m].i = ((Fout[m].i)+(scratch[0].r));

         ++Fout;
      } while(--k);
   }
}



static void kf_bfly5(
                     kiss_fft_cpx * Fout,
                     const size_t fstride,
                     const kiss_fft_state *st,
                     int m,
                     int N,
                     int mm
                    )
{
   kiss_fft_cpx *Fout0,*Fout1,*Fout2,*Fout3,*Fout4;
   int i, u;
   kiss_fft_cpx scratch[13];
   const kiss_twiddle_cpx *tw;
   kiss_twiddle_cpx ya,yb;
   kiss_fft_cpx * Fout_beg = Fout;







   ya = st->twiddles[fstride*m];
   yb = st->twiddles[fstride*2*m];

   tw=st->twiddles;

   for (i=0;i<N;i++)
   {
      Fout = Fout_beg + i*mm;
      Fout0=Fout;
      Fout1=Fout0+m;
      Fout2=Fout0+2*m;
      Fout3=Fout0+3*m;
      Fout4=Fout0+4*m;


      for ( u=0; u<m; ++u ) {
         scratch[0] = *Fout0;

         do{ (scratch[1]).r = (*Fout1).r*(tw[u*fstride]).r - (*Fout1).i*(tw[u*fstride]).i; (scratch[1]).i = (*Fout1).r*(tw[u*fstride]).i + (*Fout1).i*(tw[u*fstride]).r; }while(0);
         do{ (scratch[2]).r = (*Fout2).r*(tw[2*u*fstride]).r - (*Fout2).i*(tw[2*u*fstride]).i; (scratch[2]).i = (*Fout2).r*(tw[2*u*fstride]).i + (*Fout2).i*(tw[2*u*fstride]).r; }while(0);
         do{ (scratch[3]).r = (*Fout3).r*(tw[3*u*fstride]).r - (*Fout3).i*(tw[3*u*fstride]).i; (scratch[3]).i = (*Fout3).r*(tw[3*u*fstride]).i + (*Fout3).i*(tw[3*u*fstride]).r; }while(0);
         do{ (scratch[4]).r = (*Fout4).r*(tw[4*u*fstride]).r - (*Fout4).i*(tw[4*u*fstride]).i; (scratch[4]).i = (*Fout4).r*(tw[4*u*fstride]).i + (*Fout4).i*(tw[4*u*fstride]).r; }while(0);

         do { (scratch[7]).r=(scratch[1]).r+(scratch[4]).r; (scratch[7]).i=(scratch[1]).i+(scratch[4]).i; }while(0);
         do { (scratch[10]).r=(scratch[1]).r-(scratch[4]).r; (scratch[10]).i=(scratch[1]).i-(scratch[4]).i; }while(0);
         do { (scratch[8]).r=(scratch[2]).r+(scratch[3]).r; (scratch[8]).i=(scratch[2]).i+(scratch[3]).i; }while(0);
         do { (scratch[9]).r=(scratch[2]).r-(scratch[3]).r; (scratch[9]).i=(scratch[2]).i-(scratch[3]).i; }while(0);

         Fout0->r = ((Fout0->r)+(((scratch[7].r)+(scratch[8].r))));
         Fout0->i = ((Fout0->i)+(((scratch[7].i)+(scratch[8].i))));

         scratch[5].r = ((scratch[0].r)+(((( (scratch[7].r)*(ya.r) ))+(( (scratch[8].r)*(yb.r) )))));
         scratch[5].i = ((scratch[0].i)+(((( (scratch[7].i)*(ya.r) ))+(( (scratch[8].i)*(yb.r) )))));

         scratch[6].r = ((( (scratch[10].i)*(ya.i) ))+(( (scratch[9].i)*(yb.i) )));
         scratch[6].i = (-(((( (scratch[10].r)*(ya.i) ))+(( (scratch[9].r)*(yb.i) )))));

         do { (*Fout1).r=(scratch[5]).r-(scratch[6]).r; (*Fout1).i=(scratch[5]).i-(scratch[6]).i; }while(0);
         do { (*Fout4).r=(scratch[5]).r+(scratch[6]).r; (*Fout4).i=(scratch[5]).i+(scratch[6]).i; }while(0);

         scratch[11].r = ((scratch[0].r)+(((( (scratch[7].r)*(yb.r) ))+(( (scratch[8].r)*(ya.r) )))));
         scratch[11].i = ((scratch[0].i)+(((( (scratch[7].i)*(yb.r) ))+(( (scratch[8].i)*(ya.r) )))));
         scratch[12].r = ((( (scratch[9].i)*(ya.i) ))-(( (scratch[10].i)*(yb.i) )));
         scratch[12].i = ((( (scratch[10].r)*(yb.i) ))-(( (scratch[9].r)*(ya.i) )));

         do { (*Fout2).r=(scratch[11]).r+(scratch[12]).r; (*Fout2).i=(scratch[11]).i+(scratch[12]).i; }while(0);
         do { (*Fout3).r=(scratch[11]).r-(scratch[12]).r; (*Fout3).i=(scratch[11]).i-(scratch[12]).i; }while(0);

         ++Fout0;++Fout1;++Fout2;++Fout3;++Fout4;
      }
   }
}
static
void compute_bitrev_table(
         int Fout,
         opus_int16 *f,
         const size_t fstride,
         int in_stride,
         opus_int16 * factors,
         const kiss_fft_state *st
            )
{
   const int p=*factors++;
   const int m=*factors++;


   if (m==1)
   {
      int j;
      for (j=0;j<p;j++)
      {
         *f = Fout+j;
         f += fstride*in_stride;
      }
   } else {
      int j;
      for (j=0;j<p;j++)
      {
         compute_bitrev_table( Fout , f, fstride*p, in_stride, factors,st);
         f += fstride*in_stride;
         Fout += m;
      }
   }
}





static
int kf_factor(int n,opus_int16 * facbuf)
{
    int p=4;
    int i;
    int stages=0;
    int nbak = n;


    do {
        while (n % p) {
            switch (p) {
                case 4: p = 2; break;
                case 2: p = 3; break;
                default: p += 2; break;
            }
            if (p>32000 || (opus_int32)p*(opus_int32)p > n)
                p = n;
        }
        n /= p;



        if (p>5)

        {
           return 0;
        }
        facbuf[2*stages] = p;
        if (p==2 && stages > 1)
        {
           facbuf[2*stages] = 4;
           facbuf[2] = 2;
        }
        stages++;
    } while (n > 1);
    n = nbak;



    for (i=0;i<stages/2;i++)
    {
       int tmp;
       tmp = facbuf[2*i];
       facbuf[2*i] = facbuf[2*(stages-i-1)];
       facbuf[2*(stages-i-1)] = tmp;
    }
    for (i=0;i<stages;i++)
    {
        n /= facbuf[2*i];
        facbuf[2*i+1] = n;
    }
    return 1;
}

static void compute_twiddles(kiss_twiddle_cpx *twiddles, int nfft)
{
   int i;






   for (i=0;i<nfft;++i) {
      const double pi=3.14159265358979323846264338327;
      double phase = ( -2*pi /nfft ) * i;
      do{ (twiddles+i)->r = (float) cos(phase); (twiddles+i)->i = (float) sin(phase); }while(0);
   }

}

int opus_fft_alloc_arch_c(kiss_fft_state *st) {
   (void)st;
   return 0;
}







kiss_fft_state *opus_fft_alloc_twiddles(int nfft,void * mem,size_t * lenmem,
                                        const kiss_fft_state *base, int arch)
{
    kiss_fft_state *st=
                      ((void *)0)
                          ;
    size_t memneeded = sizeof(struct kiss_fft_state);

    if ( lenmem==
                ((void *)0) 
                     ) {
        st = ( kiss_fft_state*)opus_alloc( memneeded );
    }else{
        if (mem != 
                  ((void *)0) 
                       && *lenmem >= memneeded)
            st = (kiss_fft_state*)mem;
        *lenmem = memneeded;
    }
    if (st) {
        opus_int16 *bitrev;
        kiss_twiddle_cpx *twiddles;

        st->nfft=nfft;







        st->scale = 1.f/nfft;

        if (base != 
                   ((void *)0)
                       )
        {
           st->twiddles = base->twiddles;
           st->shift = 0;
           while (st->shift < 32 && nfft<<st->shift != base->nfft)
              st->shift++;
           if (st->shift>=32)
              goto fail;
        } else {
           st->twiddles = twiddles = (kiss_twiddle_cpx*)opus_alloc(sizeof(kiss_twiddle_cpx)*nfft);
           compute_twiddles(twiddles, nfft);
           st->shift = -1;
        }
        if (!kf_factor(nfft,st->factors))
        {
           goto fail;
        }


        st->bitrev = bitrev = (opus_int16*)opus_alloc(sizeof(opus_int16)*nfft);
        if (st->bitrev==
                       ((void *)0)
                           )
            goto fail;
        compute_bitrev_table(0, bitrev, 1,1, st->factors,st);


        if (((void)(arch), opus_fft_alloc_arch_c(st)))
            goto fail;
    }
    return st;
fail:
    opus_fft_free(st, arch);
    return 
          ((void *)0)
              ;
}

kiss_fft_state *opus_fft_alloc(int nfft,void * mem,size_t * lenmem, int arch)
{
   return opus_fft_alloc_twiddles(nfft, mem, lenmem, 
                                                    ((void *)0)
                                                        , arch);
}

void opus_fft_free_arch_c(kiss_fft_state *st) {
   (void)st;
}

void opus_fft_free(const kiss_fft_state *cfg, int arch)
{
   if (cfg)
   {
      ((void)(arch), opus_fft_free_arch_c((kiss_fft_state *)cfg));
      opus_free((opus_int16*)cfg->bitrev);
      if (cfg->shift < 0)
         opus_free((kiss_twiddle_cpx*)cfg->twiddles);
      opus_free((kiss_fft_state*)cfg);
   }
}



void opus_fft_impl(const kiss_fft_state *st,kiss_fft_cpx *fout)
{
    int m2, m;
    int p;
    int L;
    int fstride[8];
    int i;
    int shift;


    shift = st->shift>0 ? st->shift : 0;

    fstride[0] = 1;
    L=0;
    do {
       p = st->factors[2*L];
       m = st->factors[2*L+1];
       fstride[L+1] = fstride[L]*p;
       L++;
    } while(m!=1);
    m = st->factors[2*L-1];
    for (i=L-1;i>=0;i--)
    {
       if (i!=0)
          m2 = st->factors[2*i-1];
       else
          m2 = 1;
       switch (st->factors[2*i])
       {
       case 2:
          kf_bfly2(fout, m, fstride[i]);
          break;
       case 4:
          kf_bfly4(fout,fstride[i]<<shift,st,m, fstride[i], m2);
          break;

       case 3:
          kf_bfly3(fout,fstride[i]<<shift,st,m, fstride[i], m2);
          break;
       case 5:
          kf_bfly5(fout,fstride[i]<<shift,st,m, fstride[i], m2);
          break;

       }
       m = m2;
    }
}

void opus_fft_c(const kiss_fft_state *st,const kiss_fft_cpx *fin,kiss_fft_cpx *fout)
{
   int i;
   opus_val16 scale;





   scale = st->scale;

   ;

   for (i=0;i<st->nfft;i++)
   {
      kiss_fft_cpx x = fin[i];
      fout[st->bitrev[i]].r = (((scale)*(x.r)));
      fout[st->bitrev[i]].i = (((scale)*(x.i)));
   }
   opus_fft_impl(st, fout);
}


void opus_ifft_c(const kiss_fft_state *st,const kiss_fft_cpx *fin,kiss_fft_cpx *fout)
{
   int i;
   ;

   for (i=0;i<st->nfft;i++)
      fout[st->bitrev[i]] = fin[i];
   for (i=0;i<st->nfft;i++)
      fout[i].i = -fout[i].i;
   opus_fft_impl(st, fout);
   for (i=0;i<st->nfft;i++)
      fout[i].i = -fout[i].i;
}
int clt_mdct_init(mdct_lookup *l,int N, int maxshift, int arch)
{
   int i;
   float *trig;
   int shift;
   int N2=N>>1;
   l->n = N;
   l->maxshift = maxshift;
   for (i=0;i<=maxshift;i++)
   {
      if (i==0)
         l->kfft[i] = opus_fft_alloc(N>>2>>i, 0, 0, arch);
      else
         l->kfft[i] = opus_fft_alloc_twiddles(N>>2>>i, 0, 0, l->kfft[0], arch);

      if (l->kfft[i]==
                     ((void *)0)
                         )
         return 0;

   }
   l->trig = trig = (float*)opus_alloc((N-(N2>>maxshift))*sizeof(float));
   if (l->trig==
               ((void *)0)
                   )
     return 0;
   for (shift=0;shift<=maxshift;shift++)
   {
      for (i=0;i<N2;i++)
         trig[i] = (float)cos(2*3.141592653f*(i+.125)/N);

      trig += N2;
      N2 >>= 1;
      N >>= 1;
   }
   return 1;
}

void clt_mdct_clear(mdct_lookup *l, int arch)
{
   int i;
   for (i=0;i<=l->maxshift;i++)
      opus_fft_free(l->kfft[i], arch);
   opus_free((float*)l->trig);
}





void clt_mdct_forward_c(const mdct_lookup *l, float *in, float * restrict out,
      const opus_val16 *window, int overlap, int shift, int stride, int arch)
{
   int i;
   int N, N2, N4;
   ;
   ;
   const kiss_fft_state *st = l->kfft[shift];
   const float *trig;
   opus_val16 scale;





   ;
   (void)arch;
   scale = st->scale;

   N = l->n;
   trig = l->trig;
   for (i=0;i<shift;i++)
   {
      N >>= 1;
      trig += N;
   }
   N2 = N>>1;
   N4 = N>>2;

   float *f = (float*)DR_OPUS_ALLOCA(sizeof(float)*(N2));
   kiss_fft_cpx *f2 = (kiss_fft_cpx*)DR_OPUS_ALLOCA(sizeof(kiss_fft_cpx)*(N4));



   {

      const float * restrict xp1 = in+(overlap>>1);
      const float * restrict xp2 = in+N2-1+(overlap>>1);
      float * restrict yp = f;
      const opus_val16 * restrict wp1 = window+(overlap>>1);
      const opus_val16 * restrict wp2 = window+(overlap>>1)-1;
      for(i=0;i<((overlap+3)>>2);i++)
      {

         *yp++ = ((*wp2)*(xp1[N2])) + ((*wp1)*(*xp2));
         *yp++ = ((*wp1)*(*xp1)) - ((*wp2)*(xp2[-N2]));
         xp1+=2;
         xp2-=2;
         wp1+=2;
         wp2-=2;
      }
      wp1 = window;
      wp2 = window+overlap-1;
      for(;i<N4-((overlap+3)>>2);i++)
      {

         *yp++ = *xp2;
         *yp++ = *xp1;
         xp1+=2;
         xp2-=2;
      }
      for(;i<N4;i++)
      {

         *yp++ = -((*wp1)*(xp1[-N2])) + ((*wp2)*(*xp2));
         *yp++ = ((*wp2)*(*xp1)) + ((*wp1)*(xp2[N2]));
         xp1+=2;
         xp2-=2;
         wp1+=2;
         wp2-=2;
      }
   }

   {
      float * restrict yp = f;
      const float *t = &trig[0];
      for(i=0;i<N4;i++)
      {
         kiss_fft_cpx yc;
         float t0, t1;
         float re, im, yr, yi;
         t0 = t[i];
         t1 = t[N4+i];
         re = *yp++;
         im = *yp++;
         yr = ( (re)*(t0) ) - ( (im)*(t1) );
         yi = ( (im)*(t0) ) + ( (re)*(t1) );
         yc.r = yr;
         yc.i = yi;
         yc.r = (((scale)*(yc.r)));
         yc.i = (((scale)*(yc.i)));
         f2[st->bitrev[i]] = yc;
      }
   }


   opus_fft_impl(st, f2);


   {

      const kiss_fft_cpx * restrict fp = f2;
      float * restrict yp1 = out;
      float * restrict yp2 = out+stride*(N2-1);
      const float *t = &trig[0];

      for(i=0;i<N4;i++)
      {
         float yr, yi;
         yr = ( (fp->i)*(t[N4+i]) ) - ( (fp->r)*(t[i]) );
         yi = ( (fp->r)*(t[N4+i]) ) + ( (fp->i)*(t[i]) );
         *yp1 = yr;
         *yp2 = yi;
         fp++;
         yp1 += 2*stride;
         yp2 -= 2*stride;
      }
   }
   ;
}



void clt_mdct_backward_c(const mdct_lookup *l, float *in, float * restrict out,
      const opus_val16 * restrict window, int overlap, int shift, int stride, int arch)
{
   int i;
   int N, N2, N4;
   const float *trig;
   (void) arch;

   N = l->n;
   trig = l->trig;
   for (i=0;i<shift;i++)
   {
      N >>= 1;
      trig += N;
   }
   N2 = N>>1;
   N4 = N>>2;


   {

      const float * restrict xp1 = in;
      const float * restrict xp2 = in+stride*(N2-1);
      float * restrict yp = out+(overlap>>1);
      const float * restrict t = &trig[0];
      const opus_int16 * restrict bitrev = l->kfft[shift]->bitrev;
      for(i=0;i<N4;i++)
      {
         int rev;
         float yr, yi;
         rev = *bitrev++;
         yr = ((( (*xp2)*(t[i]) ))+(( (*xp1)*(t[N4+i]) )));
         yi = ((( (*xp1)*(t[i]) ))-(( (*xp2)*(t[N4+i]) )));

         yp[2*rev+1] = yr;
         yp[2*rev] = yi;

         xp1+=2*stride;
         xp2-=2*stride;
      }
   }

   opus_fft_impl(l->kfft[shift], (kiss_fft_cpx*)(out+(overlap>>1)));



   {
      float * yp0 = out+(overlap>>1);
      float * yp1 = out+(overlap>>1)+N2-2;
      const float *t = &trig[0];


      for(i=0;i<(N4+1)>>1;i++)
      {
         float re, im, yr, yi;
         float t0, t1;

         re = yp0[1];
         im = yp0[0];
         t0 = t[i];
         t1 = t[N4+i];

         yr = ((( (re)*(t0) ))+(( (im)*(t1) )));
         yi = ((( (re)*(t1) ))-(( (im)*(t0) )));

         re = yp1[1];
         im = yp1[0];
         yp0[0] = yr;
         yp1[1] = yi;

         t0 = t[(N4-i-1)];
         t1 = t[(N2-i-1)];

         yr = ((( (re)*(t0) ))+(( (im)*(t1) )));
         yi = ((( (re)*(t1) ))-(( (im)*(t0) )));
         yp1[0] = yr;
         yp0[1] = yi;
         yp0 += 2;
         yp1 -= 2;
      }
   }


   {
      float * restrict xp1 = out+overlap-1;
      float * restrict yp1 = out;
      const opus_val16 * restrict wp1 = window;
      const opus_val16 * restrict wp2 = window+overlap-1;

      for(i = 0; i < overlap/2; i++)
      {
         float x1, x2;
         x1 = *xp1;
         x2 = *yp1;
         *yp1++ = ((((*wp2)*(x2)))-(((*wp1)*(x1))));
         *xp1-- = ((((*wp1)*(x2)))+(((*wp2)*(x1))));
         wp1++;
         wp2--;
      }
   }
}
void _celt_lpc(opus_val16 *_lpc, const opus_val32 *ac, int p);

void celt_fir_c(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16 *y,
         int N,
         int ord,
         int arch);






void celt_iir(const opus_val32 *x,
         const opus_val16 *den,
         opus_val32 *y,
         int N,
         int ord,
         opus_val16 *mem,
         int arch);

int _celt_autocorr(const opus_val16 *x, opus_val32 *ac,
         const opus_val16 *window, int overlap, int lag, int n, int arch);


void pitch_downsample(celt_sig * restrict x[], opus_val16 * restrict x_lp,
      int len, int C, int arch);

void pitch_search(const opus_val16 * restrict x_lp, opus_val16 * restrict y,
                  int len, int max_pitch, int *pitch, int arch);

opus_val16 remove_doubling(opus_val16 *x, int maxperiod, int minperiod,
      int N, int *T0, int prev_period, opus_val16 prev_gain, int arch);




static inline void xcorr_kernel_c(const opus_val16 * x, const opus_val16 * y, opus_val32 sum[4], int len)
{
   int j;
   opus_val16 y_0, y_1, y_2, y_3;
   ;
   y_3=0;
   y_0=*y++;
   y_1=*y++;
   y_2=*y++;
   for (j=0;j<len-3;j+=4)
   {
      opus_val16 tmp;
      tmp = *x++;
      y_3=*y++;
      sum[0] = ((sum[0])+(opus_val32)(tmp)*(opus_val32)(y_0));
      sum[1] = ((sum[1])+(opus_val32)(tmp)*(opus_val32)(y_1));
      sum[2] = ((sum[2])+(opus_val32)(tmp)*(opus_val32)(y_2));
      sum[3] = ((sum[3])+(opus_val32)(tmp)*(opus_val32)(y_3));
      tmp=*x++;
      y_0=*y++;
      sum[0] = ((sum[0])+(opus_val32)(tmp)*(opus_val32)(y_1));
      sum[1] = ((sum[1])+(opus_val32)(tmp)*(opus_val32)(y_2));
      sum[2] = ((sum[2])+(opus_val32)(tmp)*(opus_val32)(y_3));
      sum[3] = ((sum[3])+(opus_val32)(tmp)*(opus_val32)(y_0));
      tmp=*x++;
      y_1=*y++;
      sum[0] = ((sum[0])+(opus_val32)(tmp)*(opus_val32)(y_2));
      sum[1] = ((sum[1])+(opus_val32)(tmp)*(opus_val32)(y_3));
      sum[2] = ((sum[2])+(opus_val32)(tmp)*(opus_val32)(y_0));
      sum[3] = ((sum[3])+(opus_val32)(tmp)*(opus_val32)(y_1));
      tmp=*x++;
      y_2=*y++;
      sum[0] = ((sum[0])+(opus_val32)(tmp)*(opus_val32)(y_3));
      sum[1] = ((sum[1])+(opus_val32)(tmp)*(opus_val32)(y_0));
      sum[2] = ((sum[2])+(opus_val32)(tmp)*(opus_val32)(y_1));
      sum[3] = ((sum[3])+(opus_val32)(tmp)*(opus_val32)(y_2));
   }
   if (j++<len)
   {
      opus_val16 tmp = *x++;
      y_3=*y++;
      sum[0] = ((sum[0])+(opus_val32)(tmp)*(opus_val32)(y_0));
      sum[1] = ((sum[1])+(opus_val32)(tmp)*(opus_val32)(y_1));
      sum[2] = ((sum[2])+(opus_val32)(tmp)*(opus_val32)(y_2));
      sum[3] = ((sum[3])+(opus_val32)(tmp)*(opus_val32)(y_3));
   }
   if (j++<len)
   {
      opus_val16 tmp=*x++;
      y_0=*y++;
      sum[0] = ((sum[0])+(opus_val32)(tmp)*(opus_val32)(y_1));
      sum[1] = ((sum[1])+(opus_val32)(tmp)*(opus_val32)(y_2));
      sum[2] = ((sum[2])+(opus_val32)(tmp)*(opus_val32)(y_3));
      sum[3] = ((sum[3])+(opus_val32)(tmp)*(opus_val32)(y_0));
   }
   if (j<len)
   {
      opus_val16 tmp=*x++;
      y_1=*y++;
      sum[0] = ((sum[0])+(opus_val32)(tmp)*(opus_val32)(y_2));
      sum[1] = ((sum[1])+(opus_val32)(tmp)*(opus_val32)(y_3));
      sum[2] = ((sum[2])+(opus_val32)(tmp)*(opus_val32)(y_0));
      sum[3] = ((sum[3])+(opus_val32)(tmp)*(opus_val32)(y_1));
   }
}







static inline void dual_inner_prod_c(const opus_val16 *x, const opus_val16 *y01, const opus_val16 *y02,
      int N, opus_val32 *xy1, opus_val32 *xy2)
{
   int i;
   opus_val32 xy01=0;
   opus_val32 xy02=0;
   for (i=0;i<N;i++)
   {
      xy01 = ((xy01)+(opus_val32)(x[i])*(opus_val32)(y01[i]));
      xy02 = ((xy02)+(opus_val32)(x[i])*(opus_val32)(y02[i]));
   }
   *xy1 = xy01;
   *xy2 = xy02;
}
static inline opus_val32 celt_inner_prod_c(const opus_val16 *x,
      const opus_val16 *y, int N)
{
   int i;
   opus_val32 xy=0;
   for (i=0;i<N;i++)
      xy = ((xy)+(opus_val32)(x[i])*(opus_val32)(y[i]));
   return xy;
}
void

celt_pitch_xcorr_c(const opus_val16 *_x, const opus_val16 *_y,
      opus_val32 *xcorr, int len, int max_pitch, int arch);

void _celt_lpc(
      opus_val16 *_lpc,
const opus_val32 *ac,
int p
)
{
   int i, j;
   opus_val32 r;
   opus_val32 error = ac[0];



   float *lpc = _lpc;


   (memset((lpc), 0, (p)*sizeof(*(lpc))));



   if (ac[0] > 1e-10f)

   {
      for (i = 0; i < p; i++) {

         opus_val32 rr = 0;
         for (j = 0; j < i; j++)
            rr += ((lpc[j])*(ac[i - j]));
         rr += (ac[i + 1]);
         r = -((float)((rr))/(error));

         lpc[i] = (r);
         for (j = 0; j < (i+1)>>1; j++)
         {
            opus_val32 tmp1, tmp2;
            tmp1 = lpc[j];
            tmp2 = lpc[i-1-j];
            lpc[j] = tmp1 + ((r)*(tmp2));
            lpc[i-1-j] = tmp2 + ((r)*(tmp1));
         }

         error = error - ((((r)*(r)))*(error));





         if (error<=.001f*ac[0])
            break;

      }
   }
}


void celt_fir_c(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16 *y,
         int N,
         int ord,
         int arch)
{
   int i,j;
   ;
   ;
   ;
   opus_val16 *rnum = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(ord));
   for(i=0;i<ord;i++)
      rnum[i] = num[ord-i-1];
   for (i=0;i<N-3;i+=4)
   {
      opus_val32 sum[4];
      sum[0] = ((x[i ]));
      sum[1] = ((x[i+1]));
      sum[2] = ((x[i+2]));
      sum[3] = ((x[i+3]));
      ((void)(arch),xcorr_kernel_c(rnum, x+i-ord, sum, ord));
      y[i ] = (sum[0]);
      y[i+1] = (sum[1]);
      y[i+2] = (sum[2]);
      y[i+3] = (sum[3]);
   }
   for (;i<N;i++)
   {
      opus_val32 sum = ((x[i]));
      for (j=0;j<ord;j++)
         sum = ((sum)+(opus_val32)(rnum[j])*(opus_val32)(x[i+j-ord]));
      y[i] = (sum);
   }
   ;
}

void celt_iir(const opus_val32 *_x,
         const opus_val16 *den,
         opus_val32 *_y,
         int N,
         int ord,
         opus_val16 *mem,
         int arch)
{
   int i,j;
   ;
   ;
   ;

   ;
   opus_val16 *rden = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(ord));
   opus_val16 *y = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(N+ord));
   for(i=0;i<ord;i++)
      rden[i] = den[ord-i-1];
   for(i=0;i<ord;i++)
      y[i] = -mem[ord-i-1];
   for(;i<N+ord;i++)
      y[i]=0;
   for (i=0;i<N-3;i+=4)
   {

      opus_val32 sum[4];
      sum[0]=_x[i];
      sum[1]=_x[i+1];
      sum[2]=_x[i+2];
      sum[3]=_x[i+3];
      ((void)(arch),xcorr_kernel_c(rden, y+i, sum, ord));


      y[i+ord ] = -(sum[0]);
      _y[i ] = sum[0];
      sum[1] = ((sum[1])+(opus_val32)(y[i+ord ])*(opus_val32)(den[0]));
      y[i+ord+1] = -(sum[1]);
      _y[i+1] = sum[1];
      sum[2] = ((sum[2])+(opus_val32)(y[i+ord+1])*(opus_val32)(den[0]));
      sum[2] = ((sum[2])+(opus_val32)(y[i+ord ])*(opus_val32)(den[1]));
      y[i+ord+2] = -(sum[2]);
      _y[i+2] = sum[2];

      sum[3] = ((sum[3])+(opus_val32)(y[i+ord+2])*(opus_val32)(den[0]));
      sum[3] = ((sum[3])+(opus_val32)(y[i+ord+1])*(opus_val32)(den[1]));
      sum[3] = ((sum[3])+(opus_val32)(y[i+ord ])*(opus_val32)(den[2]));
      y[i+ord+3] = -(sum[3]);
      _y[i+3] = sum[3];
   }
   for (;i<N;i++)
   {
      opus_val32 sum = _x[i];
      for (j=0;j<ord;j++)
         sum -= ((opus_val32)(rden[j])*(opus_val32)(y[i+j]));
      y[i+ord] = (sum);
      _y[i] = sum;
   }
   for(i=0;i<ord;i++)
      mem[i] = _y[N-i-1];
   ;

}

int _celt_autocorr(
                   const opus_val16 *x,
                   opus_val32 *ac,
                   const opus_val16 *window,
                   int overlap,
                   int lag,
                   int n,
                   int arch
                  )
{
   opus_val32 d;
   int i, k;
   int fastN=n-lag;
   int shift;
   const opus_val16 *xptr;
   ;
   ;
   opus_val16 *xx = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(n));
   ;
   ;
   if (overlap == 0)
   {
      xptr = x;
   } else {
      for (i=0;i<n;i++)
         xx[i] = x[i];
      for (i=0;i<overlap;i++)
      {
         xx[i] = ((x[i])*(window[i]));
         xx[n-i-1] = ((x[n-i-1])*(window[i]));
      }
      xptr = xx;
   }
   shift=0;
   celt_pitch_xcorr_c(xptr, xptr, ac, fastN, lag+1, arch);
   for (k=0;k<=lag;k++)
   {
      for (i = k+fastN, d = 0; i < n; i++)
         d = ((d)+(opus_val32)(xptr[i])*(opus_val32)(xptr[i-k]));
      ac[k] += d;
   }
   ;
   return shift;
}
static void find_best_pitch(opus_val32 *xcorr, opus_val16 *y, int len,
                            int max_pitch, int *best_pitch



                            )
{
   int i, j;
   opus_val32 Syy=1;
   opus_val16 best_num[2];
   opus_val32 best_den[2];






   best_num[0] = -1;
   best_num[1] = -1;
   best_den[0] = 0;
   best_den[1] = 0;
   best_pitch[0] = 0;
   best_pitch[1] = 1;
   for (j=0;j<len;j++)
      Syy = ((Syy)+((((opus_val32)(y[j])*(opus_val32)(y[j])))));
   for (i=0;i<max_pitch;i++)
   {
      if (xcorr[i]>0)
      {
         opus_val16 num;
         opus_val32 xcorr16;
         xcorr16 = ((xcorr[i]));



         xcorr16 *= 1e-12f;

         num = ((xcorr16)*(xcorr16));
         if (((num)*(best_den[1])) > ((best_num[1])*(Syy)))
         {
            if (((num)*(best_den[0])) > ((best_num[0])*(Syy)))
            {
               best_num[1] = best_num[0];
               best_den[1] = best_den[0];
               best_pitch[1] = best_pitch[0];
               best_num[0] = num;
               best_den[0] = Syy;
               best_pitch[0] = i;
            } else {
               best_num[1] = num;
               best_den[1] = Syy;
               best_pitch[1] = i;
            }
         }
      }
      Syy += (((opus_val32)(y[i+len])*(opus_val32)(y[i+len]))) - (((opus_val32)(y[i])*(opus_val32)(y[i])));
      Syy = ((1) > (Syy) ? (1) : (Syy));
   }
}

static void celt_fir5(opus_val16 *x,
         const opus_val16 *num,
         int N)
{
   int i;
   opus_val16 num0, num1, num2, num3, num4;
   opus_val32 mem0, mem1, mem2, mem3, mem4;
   num0=num[0];
   num1=num[1];
   num2=num[2];
   num3=num[3];
   num4=num[4];
   mem0=0;
   mem1=0;
   mem2=0;
   mem3=0;
   mem4=0;
   for (i=0;i<N;i++)
   {
      opus_val32 sum = ((x[i]));
      sum = ((sum)+(opus_val32)(num0)*(opus_val32)(mem0));
      sum = ((sum)+(opus_val32)(num1)*(opus_val32)(mem1));
      sum = ((sum)+(opus_val32)(num2)*(opus_val32)(mem2));
      sum = ((sum)+(opus_val32)(num3)*(opus_val32)(mem3));
      sum = ((sum)+(opus_val32)(num4)*(opus_val32)(mem4));
      mem4 = mem3;
      mem3 = mem2;
      mem2 = mem1;
      mem1 = mem0;
      mem0 = x[i];
      x[i] = (sum);
   }
}


void pitch_downsample(celt_sig * restrict x[], opus_val16 * restrict x_lp,
      int len, int C, int arch)
{
   int i;
   opus_val32 ac[5];
   opus_val16 tmp=1.0f;
   opus_val16 lpc[4];
   opus_val16 lpc2[5];
   opus_val16 c1 = (.8f);
   for (i=1;i<len>>1;i++)
      x_lp[i] = .25f*x[0][(2*i-1)] + .25f*x[0][(2*i+1)] + .5f*x[0][2*i];
   x_lp[0] = .25f*x[0][1] + .5f*x[0][0];
   if (C==2)
   {
      for (i=1;i<len>>1;i++)
         x_lp[i] += .25f*x[1][(2*i-1)] + .25f*x[1][(2*i+1)] + .5f*x[1][2*i];
      x_lp[0] += .25f*x[1][1] + .5f*x[1][0];
   }

   _celt_autocorr(x_lp, ac, 
                           ((void *)0)
                               , 0,
                  4, len>>1, arch);





   ac[0] *= 1.0001f;


   for (i=1;i<=4;i++)
   {




      ac[i] -= ac[i]*(.008f*i)*(.008f*i);

   }

   _celt_lpc(lpc, ac, 4);
   for (i=0;i<4;i++)
   {
      tmp = (((.9f))*(tmp));
      lpc[i] = ((lpc[i])*(tmp));
   }

   lpc2[0] = lpc[0] + (.8f);
   lpc2[1] = lpc[1] + ((c1)*(lpc[0]));
   lpc2[2] = lpc[2] + ((c1)*(lpc[1]));
   lpc2[3] = lpc[3] + ((c1)*(lpc[2]));
   lpc2[4] = ((c1)*(lpc[3]));
   celt_fir5(x_lp, lpc2, len>>1);
}





void

celt_pitch_xcorr_c(const opus_val16 *_x, const opus_val16 *_y,
      opus_val32 *xcorr, int len, int max_pitch, int arch)
{
   int i;






   ;
   ;
   for (i=0;i<max_pitch-3;i+=4)
   {
      opus_val32 sum[4]={0,0,0,0};
      ((void)(arch),xcorr_kernel_c(_x, _y+i, sum, len));
      xcorr[i]=sum[0];
      xcorr[i+1]=sum[1];
      xcorr[i+2]=sum[2];
      xcorr[i+3]=sum[3];






   }

   for (;i<max_pitch;i++)
   {
      opus_val32 sum;
      sum = ((void)(arch),celt_inner_prod_c(_x, _y+i, len));
      xcorr[i] = sum;



   }




}

void pitch_search(const opus_val16 * restrict x_lp, opus_val16 * restrict y,
                  int len, int max_pitch, int *pitch, int arch)
{
   int i, j;
   int lag;
   int best_pitch[2]={0,0};
   ;
   ;
   ;





   int offset;

   ;

   ;
   ;
   lag = len+max_pitch;

   opus_val16 *x_lp4 = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(len>>2));
   opus_val16 *y_lp4 = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(lag>>2));
   opus_val32 *xcorr = (opus_val32*)DR_OPUS_ALLOCA(sizeof(opus_val32)*(max_pitch>>1));


   for (j=0;j<len>>2;j++)
      x_lp4[j] = x_lp[2*j];
   for (j=0;j<lag>>2;j++)
      y_lp4[j] = y[2*j];
   celt_pitch_xcorr_c(x_lp4, y_lp4, xcorr, len>>2, max_pitch>>2, arch);

   find_best_pitch(xcorr, y_lp4, len>>2, max_pitch>>2, best_pitch



                   );





   for (i=0;i<max_pitch>>1;i++)
   {
      opus_val32 sum;
      xcorr[i] = 0;
      if (abs(i-2*best_pitch[0])>2 && abs(i-2*best_pitch[1])>2)
         continue;





      sum = ((void)(arch),celt_inner_prod_c(x_lp, y+i, len>>1));

      xcorr[i] = ((-1) > (sum) ? (-1) : (sum));



   }
   find_best_pitch(xcorr, y, len>>1, max_pitch>>1, best_pitch



                   );


   if (best_pitch[0]>0 && best_pitch[0]<(max_pitch>>1)-1)
   {
      opus_val32 a, b, c;
      a = xcorr[best_pitch[0]-1];
      b = xcorr[best_pitch[0]];
      c = xcorr[best_pitch[0]+1];
      if ((c-a) > (((.7f))*(b-a)))
         offset = 1;
      else if ((a-c) > (((.7f))*(b-c)))
         offset = -1;
      else
         offset = 0;
   } else {
      offset = 0;
   }
   *pitch = 2*best_pitch[0]-offset;

   ;
}
static opus_val16 compute_pitch_gain(opus_val32 xy, opus_val32 xx, opus_val32 yy)
{
   return xy/((float)sqrt(1+xx*yy));
}


static const int second_check[16] = {0, 0, 3, 2, 3, 2, 5, 2, 3, 2, 3, 2, 5, 2, 3, 2};
opus_val16 remove_doubling(opus_val16 *x, int maxperiod, int minperiod,
      int N, int *T0_, int prev_period, opus_val16 prev_gain, int arch)
{
   int k, i, T, T0;
   opus_val16 g, g0;
   opus_val16 pg;
   opus_val32 xy,xx,yy,xy2;
   opus_val32 xcorr[3];
   opus_val32 best_xy, best_yy;
   int offset;
   int minperiod0;
   ;
   ;

   minperiod0 = minperiod;
   maxperiod /= 2;
   minperiod /= 2;
   *T0_ /= 2;
   prev_period /= 2;
   N /= 2;
   x += maxperiod;
   if (*T0_>=maxperiod)
      *T0_=maxperiod-1;

   T = T0 = *T0_;
   opus_val32 *yy_lookup = (opus_val32*)DR_OPUS_ALLOCA(sizeof(opus_val32)*(maxperiod+1));
   ((void)(arch),dual_inner_prod_c(x, x, x-T0, N, &xx, &xy));
   yy_lookup[0] = xx;
   yy=xx;
   for (i=1;i<=maxperiod;i++)
   {
      yy = yy+((opus_val32)(x[-i])*(opus_val32)(x[-i]))-((opus_val32)(x[N-i])*(opus_val32)(x[N-i]));
      yy_lookup[i] = ((0) > (yy) ? (0) : (yy));
   }
   yy = yy_lookup[T0];
   best_xy = xy;
   best_yy = yy;
   g = g0 = compute_pitch_gain(xy, xx, yy);

   for (k=2;k<=15;k++)
   {
      int T1, T1b;
      opus_val16 g1;
      opus_val16 cont=0;
      opus_val16 thresh;
      T1 = celt_udiv(2*T0+k, 2*k);
      if (T1 < minperiod)
         break;

      if (k==2)
      {
         if (T1+T0>maxperiod)
            T1b = T0;
         else
            T1b = T0+T1;
      } else
      {
         T1b = celt_udiv(2*second_check[k]*T0+k, 2*k);
      }
      ((void)(arch),dual_inner_prod_c(x, &x[-T1], &x[-T1b], N, &xy, &xy2));
      xy = (.5f*(xy + xy2));
      yy = (.5f*(yy_lookup[T1] + yy_lookup[T1b]));
      g1 = compute_pitch_gain(xy, xx, yy);
      if (abs(T1-prev_period)<=1)
         cont = prev_gain;
      else if (abs(T1-prev_period)<=2 && 5*k*k < T0)
         cont = (.5f*(prev_gain));
      else
         cont = 0;
      thresh = (((.3f)) > ((((.7f))*(g0))-cont) ? ((.3f)) : ((((.7f))*(g0))-cont));


      if (T1<3*minperiod)
         thresh = (((.4f)) > ((((.85f))*(g0))-cont) ? ((.4f)) : ((((.85f))*(g0))-cont));
      else if (T1<2*minperiod)
         thresh = (((.5f)) > ((((.9f))*(g0))-cont) ? ((.5f)) : ((((.9f))*(g0))-cont));
      if (g1 > thresh)
      {
         best_xy = xy;
         best_yy = yy;
         T = T1;
         g = g1;
      }
   }
   best_xy = ((0) > (best_xy) ? (0) : (best_xy));
   if (best_yy <= best_xy)
      pg = 1.0f;
   else
      pg = (((float)(best_xy)/(best_yy+1)));

   for (k=0;k<3;k++)
      xcorr[k] = ((void)(arch),celt_inner_prod_c(x, x-(T+k-1), N));
   if ((xcorr[2]-xcorr[0]) > (((.7f))*(xcorr[1]-xcorr[0])))
      offset = 1;
   else if ((xcorr[0]-xcorr[2]) > (((.7f))*(xcorr[1]-xcorr[2])))
      offset = -1;
   else
      offset = 0;
   if (pg > g)
      pg = g;
   *T0_ = 2*T+offset;

   if (*T0_<minperiod0)
      *T0_=minperiod0;
   ;
   return pg;
}
void exp_rotation(celt_norm *X, int len, int dir, int stride, int K, int spread);

opus_val16 op_pvq_search_c(celt_norm *X, int *iy, int K, int N, int arch);
unsigned alg_quant(celt_norm *X, int N, int K, int spread, int B, ec_enc *enc,
      opus_val16 gain, int resynth, int arch);
unsigned alg_unquant(celt_norm *X, int N, int K, int spread, int B,
      ec_dec *dec, opus_val16 gain);

void renormalise_vector(celt_norm *X, int N, opus_val16 gain, int arch);

int stereo_itheta(const celt_norm *X, const celt_norm *Y, int stereo, int N, int arch);


opus_int16 bitexact_cos(opus_int16 x);
int bitexact_log2tan(int isin,int icos);






void compute_band_energies(const OpusCustomMode *m, const celt_sig *X, celt_ener *bandE, int end, int C, int LM, int arch);
void normalise_bands(const OpusCustomMode *m, const celt_sig * restrict freq, celt_norm * restrict X, const celt_ener *bandE, int end, int C, int M);






void denormalise_bands(const OpusCustomMode *m, const celt_norm * restrict X,
      celt_sig * restrict freq, const opus_val16 *bandE, int start,
      int end, int M, int downsample, int silence);






int spreading_decision(const OpusCustomMode *m, const celt_norm *X, int *average,
      int last_decision, int *hf_average, int *tapset_decision, int update_hf,
      int end, int C, int M, const int *spread_weight);





void haar1(celt_norm *X, int N0, int stride);
void quant_all_bands(int encode, const OpusCustomMode *m, int start, int end,
      celt_norm * X, celt_norm * Y, unsigned char *collapse_masks,
      const celt_ener *bandE, int *pulses, int shortBlocks, int spread,
      int dual_stereo, int intensity, int *tf_res, opus_int32 total_bits,
      opus_int32 balance, ec_ctx *ec, int M, int codedBands, opus_uint32 *seed,
      int complexity, int arch, int disable_inv);

void anti_collapse(const OpusCustomMode *m, celt_norm *X_,
      unsigned char *collapse_masks, int LM, int C, int size, int start,
      int end, const opus_val16 *logE, const opus_val16 *prev1logE,
      const opus_val16 *prev2logE, const int *pulses, opus_uint32 seed,
      int arch);

opus_uint32 celt_lcg_rand(opus_uint32 seed);

int hysteresis_decision(opus_val16 val, const opus_val16 *thresholds, const opus_val16 *hysteresis, int N, int prev);
static void exp_rotation1(celt_norm *X, int len, int stride, opus_val16 c, opus_val16 s)
{
   int i;
   opus_val16 ms;
   celt_norm *Xptr;
   Xptr = X;
   ms = (-(s));
   for (i=0;i<len-stride;i++)
   {
      celt_norm x1, x2;
      x1 = Xptr[0];
      x2 = Xptr[stride];
      Xptr[stride] = ((((((opus_val32)(c)*(opus_val32)(x2)))+(opus_val32)(s)*(opus_val32)(x1))));
      *Xptr++ = ((((((opus_val32)(c)*(opus_val32)(x1)))+(opus_val32)(ms)*(opus_val32)(x2))));
   }
   Xptr = &X[len-2*stride-1];
   for (i=len-2*stride-1;i>=0;i--)
   {
      celt_norm x1, x2;
      x1 = Xptr[0];
      x2 = Xptr[stride];
      Xptr[stride] = ((((((opus_val32)(c)*(opus_val32)(x2)))+(opus_val32)(s)*(opus_val32)(x1))));
      *Xptr-- = ((((((opus_val32)(c)*(opus_val32)(x1)))+(opus_val32)(ms)*(opus_val32)(x2))));
   }
}


void exp_rotation(celt_norm *X, int len, int dir, int stride, int K, int spread)
{
   static const int SPREAD_FACTOR[3]={15,10,5};
   int i;
   opus_val16 c, s;
   opus_val16 gain, theta;
   int stride2=0;
   int factor;

   if (2*K>=len || spread==(0))
      return;
   factor = SPREAD_FACTOR[spread-1];

   gain = (((opus_val32)((opus_val32)(((opus_val16)1.f))*(opus_val32)(len)))/((opus_val32)(len+factor*K)));
   theta = (.5f*(((gain)*(gain))));

   c = ((float)cos((.5f*3.141592653f)*((theta))));
   s = ((float)cos((.5f*3.141592653f)*((((1.0f)-(theta))))));

   if (len>=8*stride)
   {
      stride2 = 1;


      while ((stride2*stride2+stride2)*stride + (stride>>2) < len)
         stride2++;
   }


   len = celt_udiv(len, stride);
   for (i=0;i<stride;i++)
   {
      if (dir < 0)
      {
         if (stride2)
            exp_rotation1(X+i*len, len, stride2, s, c);
         exp_rotation1(X+i*len, len, 1, c, s);
      } else {
         exp_rotation1(X+i*len, len, 1, c, -s);
         if (stride2)
            exp_rotation1(X+i*len, len, stride2, s, -c);
      }
   }
}



static void normalise_residual(int * restrict iy, celt_norm * restrict X,
      int N, opus_val32 Ryy, opus_val16 gain)
{
   int i;



   opus_val32 t;
   opus_val16 g;




   t = (Ryy);
   g = ((((1.f/((float)sqrt(t)))))*(gain));

   i=0;
   do
      X[i] = ((((opus_val32)(g)*(opus_val32)(iy[i]))));
   while (++i < N);
}

static unsigned extract_collapse_mask(int *iy, int N, int B)
{
   unsigned collapse_mask;
   int N0;
   int i;
   if (B<=1)
      return 1;


   N0 = celt_udiv(N, B);
   collapse_mask = 0;
   i=0; do {
      int j;
      unsigned tmp=0;
      j=0; do {
         tmp |= iy[i*N0+j];
      } while (++j<N0);
      collapse_mask |= (tmp!=0)<<i;
   } while (++i<B);
   return collapse_mask;
}

opus_val16 op_pvq_search_c(celt_norm *X, int *iy, int K, int N, int arch)
{
   ;
   ;
   int i, j;
   int pulsesLeft;
   opus_val32 sum;
   opus_val32 xy;
   opus_val16 yy;
   ;

   (void)arch;
   celt_norm *y = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(N));
   int *signx = (int*)DR_OPUS_ALLOCA(sizeof(int)*(N));


   sum = 0;
   j=0; do {
      signx[j] = X[j]<0;

      X[j] = ((float)fabs(X[j]));
      iy[j] = 0;
      y[j] = 0;
   } while (++j<N);

   xy = yy = 0;

   pulsesLeft = K;


   if (K > (N>>1))
   {
      opus_val16 rcp;
      j=0; do {
         sum += X[j];
      } while (++j<N);







      if (!(sum > 1e-15f && sum < 64))

      {
         X[0] = (1.f);
         j=1; do
            X[j]=0;
         while (++j<N);
         sum = (1.f);
      }




      rcp = (((K+0.8f)*((1.f/(sum)))));

      j=0; do {




         iy[j] = (int)floor(rcp*X[j]);

         y[j] = (celt_norm)iy[j];
         yy = ((yy)+(opus_val32)(y[j])*(opus_val32)(y[j]));
         xy = ((xy)+(opus_val32)(X[j])*(opus_val32)(y[j]));
         y[j] *= 2;
         pulsesLeft -= iy[j];
      } while (++j<N);
   }
   ;






   if (pulsesLeft > N+3)
   {
      opus_val16 tmp = (opus_val16)pulsesLeft;
      yy = ((yy)+(opus_val32)(tmp)*(opus_val32)(tmp));
      yy = ((yy)+(opus_val32)(tmp)*(opus_val32)(y[0]));
      iy[0] += pulsesLeft;
      pulsesLeft=0;
   }

   for (i=0;i<pulsesLeft;i++)
   {
      opus_val16 Rxy, Ryy;
      int best_id;
      opus_val32 best_num;
      opus_val16 best_den;






      best_id = 0;


      yy = ((yy)+(1));





      Rxy = ((((xy)+((X[0])))));

      Ryy = ((yy)+(y[0]));



      Rxy = ((Rxy)*(Rxy));
      best_den = Ryy;
      best_num = Rxy;
      j=1;
      do {

         Rxy = ((((xy)+((X[j])))));

         Ryy = ((yy)+(y[j]));



         Rxy = ((Rxy)*(Rxy));






         if ((((opus_val32)(best_den)*(opus_val32)(Rxy)) > ((opus_val32)(Ryy)*(opus_val32)(best_num))))
         {
            best_den = Ryy;
            best_num = Rxy;
            best_id = j;
         }
      } while (++j<N);


      xy = ((xy)+((X[best_id])));

      yy = ((yy)+(y[best_id]));



      y[best_id] += 2;
      iy[best_id]++;
   }


   j=0;
   do {



      iy[j] = (iy[j]^-signx[j]) + signx[j];
   } while (++j<N);
   ;
   return yy;
}

unsigned alg_quant(celt_norm *X, int N, int K, int spread, int B, ec_enc *enc,
      opus_val16 gain, int resynth, int arch)
{
   ;
   opus_val16 yy;
   unsigned collapse_mask;
   ;

   ;
   ;


   int *iy = (int*)DR_OPUS_ALLOCA(sizeof(int)*(N+3));

   exp_rotation(X, N, 1, B, K, spread);

   yy = (op_pvq_search_c(X, iy, K, N, arch));

   encode_pulses(iy, N, K, enc);

   if (resynth)
   {
      normalise_residual(iy, X, N, yy, gain);
      exp_rotation(X, N, -1, B, K, spread);
   }

   collapse_mask = extract_collapse_mask(iy, N, B);
   ;
   return collapse_mask;
}



unsigned alg_unquant(celt_norm *X, int N, int K, int spread, int B,
      ec_dec *dec, opus_val16 gain)
{
   opus_val32 Ryy;
   unsigned collapse_mask;
   ;
   ;

   ;
   ;
   int *iy = (int*)DR_OPUS_ALLOCA(sizeof(int)*(N));
   Ryy = decode_pulses(iy, N, K, dec);
   normalise_residual(iy, X, N, Ryy, gain);
   exp_rotation(X, N, -1, B, K, spread);
   collapse_mask = extract_collapse_mask(iy, N, B);
   ;
   return collapse_mask;
}


void renormalise_vector(celt_norm *X, int N, opus_val16 gain, int arch)
{
   int i;



   opus_val32 E;
   opus_val16 g;
   opus_val32 t;
   celt_norm *xptr;
   E = 1e-15f + ((void)(arch),celt_inner_prod_c(X, X, N));



   t = (E);
   g = ((((1.f/((float)sqrt(t)))))*(gain));

   xptr = X;
   for (i=0;i<N;i++)
   {
      *xptr = ((((opus_val32)(g)*(opus_val32)(*xptr))));
      xptr++;
   }

}


int stereo_itheta(const celt_norm *X, const celt_norm *Y, int stereo, int N, int arch)
{
   int i;
   int itheta;
   opus_val16 mid, side;
   opus_val32 Emid, Eside;

   Emid = Eside = 1e-15f;
   if (stereo)
   {
      for (i=0;i<N;i++)
      {
         celt_norm m, s;
         m = (((X[i]))+((Y[i])));
         s = (((X[i]))-((Y[i])));
         Emid = ((Emid)+(opus_val32)(m)*(opus_val32)(m));
         Eside = ((Eside)+(opus_val32)(s)*(opus_val32)(s));
      }
   } else {
      Emid += ((void)(arch),celt_inner_prod_c(X, X, N));
      Eside += ((void)(arch),celt_inner_prod_c(Y, Y, N));
   }
   mid = ((float)sqrt(Emid));
   side = ((float)sqrt(Eside));




   itheta = (int)floor(.5f+16384*0.63662f*fast_atan2f(side,mid));


   return itheta;
}
int hysteresis_decision(opus_val16 val, const opus_val16 *thresholds, const opus_val16 *hysteresis, int N, int prev)
{
   int i;
   for (i=0;i<N;i++)
   {
      if (val < thresholds[i])
         break;
   }
   if (i>prev && val < thresholds[prev]+hysteresis[prev])
      i=prev;
   if (i<prev && val > thresholds[prev-1]-hysteresis[prev-1])
      i=prev;
   return i;
}

opus_uint32 celt_lcg_rand(opus_uint32 seed)
{
   return 1664525 * seed + 1013904223;
}



opus_int16 bitexact_cos(opus_int16 x)
{
   opus_int32 tmp;
   opus_int16 x2;
   tmp = (4096+((opus_int32)(x)*(x)))>>13;
   ;
   x2 = tmp;
   x2 = (32767-x2) + ((16384+((opus_int32)(opus_int16)(x2)*(opus_int16)((-7651 + ((16384+((opus_int32)(opus_int16)(x2)*(opus_int16)((8277 + ((16384+((opus_int32)(opus_int16)(-626)*(opus_int16)(x2)))>>15)))))>>15)))))>>15);
   ;
   return 1+x2;
}

int bitexact_log2tan(int isin,int icos)
{
   int lc;
   int ls;
   lc=(((int)sizeof(unsigned)*8
     )-(__builtin_clz(icos)));
   ls=(((int)sizeof(unsigned)*8
     )-(__builtin_clz(isin)));
   icos<<=15-lc;
   isin<<=15-ls;
   return (ls-lc)*(1<<11)
         +((16384+((opus_int32)(opus_int16)(isin)*(opus_int16)(((16384+((opus_int32)(opus_int16)(isin)*(opus_int16)(-2597)))>>15) + 7932)))>>15)
         -((16384+((opus_int32)(opus_int16)(icos)*(opus_int16)(((16384+((opus_int32)(opus_int16)(icos)*(opus_int16)(-2597)))>>15) + 7932)))>>15);
}
void compute_band_energies(const OpusCustomMode *m, const celt_sig *X, celt_ener *bandE, int end, int C, int LM, int arch)
{
   int i, c, N;
   const opus_int16 *eBands = m->eBands;
   N = m->shortMdctSize<<LM;
   c=0; do {
      for (i=0;i<end;i++)
      {
         opus_val32 sum;
         sum = 1e-27f + ((void)(arch),celt_inner_prod_c(&X[c*N+(eBands[i]<<LM)], &X[c*N+(eBands[i]<<LM)], (eBands[i+1]-eBands[i])<<LM));
         bandE[i+c*m->nbEBands] = ((float)sqrt(sum));

      }
   } while (++c<C);

}


void normalise_bands(const OpusCustomMode *m, const celt_sig * restrict freq, celt_norm * restrict X, const celt_ener *bandE, int end, int C, int M)
{
   int i, c, N;
   const opus_int16 *eBands = m->eBands;
   N = M*m->shortMdctSize;
   c=0; do {
      for (i=0;i<end;i++)
      {
         int j;
         opus_val16 g = 1.f/(1e-27f+bandE[i+c*m->nbEBands]);
         for (j=M*eBands[i];j<M*eBands[i+1];j++)
            X[j+c*N] = freq[j+c*N]*g;
      }
   } while (++c<C);
}




void denormalise_bands(const OpusCustomMode *m, const celt_norm * restrict X,
      celt_sig * restrict freq, const opus_val16 *bandLogE, int start,
      int end, int M, int downsample, int silence)
{
   int i, N;
   int bound;
   celt_sig * restrict f;
   const celt_norm * restrict x;
   const opus_int16 *eBands = m->eBands;
   N = M*m->shortMdctSize;
   bound = M*eBands[end];
   if (downsample!=1)
      bound = ((bound) < (N/downsample) ? (bound) : (N/downsample));
   if (silence)
   {
      bound = 0;
      start = end = 0;
   }
   f = freq;
   x = X+M*eBands[start];
   for (i=0;i<M*eBands[start];i++)
      *f++ = 0;
   for (i=start;i<end;i++)
   {
      int j, band_end;
      opus_val16 g;
      opus_val16 lg;



      j=M*eBands[i];
      band_end = M*eBands[i+1];
      lg = (((bandLogE[i])+(((opus_val32)eMeans[i]))));

      g = ((float)exp(0.6931471805599453094*(((32.f) < (lg) ? (32.f) : (lg)))));
         do {
            *f++ = (((opus_val32)(*x++)*(opus_val32)(g)));
         } while (++j<band_end);
   }
   ;
   (memset((&freq[bound]), 0, (N-bound)*sizeof(*(&freq[bound]))));
}


void anti_collapse(const OpusCustomMode *m, celt_norm *X_, unsigned char *collapse_masks, int LM, int C, int size,
      int start, int end, const opus_val16 *logE, const opus_val16 *prev1logE,
      const opus_val16 *prev2logE, const int *pulses, opus_uint32 seed, int arch)
{
   int c, i, j, k;
   for (i=start;i<end;i++)
   {
      int N0;
      opus_val16 thresh, sqrt_1;
      int depth;





      N0 = m->eBands[i+1]-m->eBands[i];

      ;
      depth = celt_udiv(1+pulses[i], (m->eBands[i+1]-m->eBands[i]))>>LM;
      thresh = .5f*((float)exp(0.6931471805599453094*(-.125f*depth)));
      sqrt_1 = (1.f/((float)sqrt(N0<<LM)));


      c=0; do
      {
         celt_norm *X;
         opus_val16 prev1;
         opus_val16 prev2;
         opus_val32 Ediff;
         opus_val16 r;
         int renormalize=0;
         prev1 = prev1logE[c*m->nbEBands+i];
         prev2 = prev2logE[c*m->nbEBands+i];
         if (C==1)
         {
            prev1 = ((prev1) > (prev1logE[m->nbEBands+i]) ? (prev1) : (prev1logE[m->nbEBands+i]));
            prev2 = ((prev2) > (prev2logE[m->nbEBands+i]) ? (prev2) : (prev2logE[m->nbEBands+i]));
         }
         Ediff = (logE[c*m->nbEBands+i])-(((prev1) < (prev2) ? (prev1) : (prev2)));
         Ediff = ((0) > (Ediff) ? (0) : (Ediff));
         r = 2.f*((float)exp(0.6931471805599453094*(-Ediff)));
         if (LM==3)
            r *= 1.41421356f;
         r = ((thresh) < (r) ? (thresh) : (r));
         r = r*sqrt_1;

         X = X_+c*size+(m->eBands[i]<<LM);
         for (k=0;k<1<<LM;k++)
         {

            if (!(collapse_masks[i*C+c]&1<<k))
            {

               for (j=0;j<N0;j++)
               {
                  seed = celt_lcg_rand(seed);
                  X[(j<<LM)+k] = (seed&0x8000 ? r : -r);
               }
               renormalize = 1;
            }
         }

         if (renormalize)
            renormalise_vector(X, N0<<LM, 1.0f, arch);
      } while (++c<C);
   }
}
static void compute_channel_weights(celt_ener Ex, celt_ener Ey, opus_val16 w[2])
{
   celt_ener minE;



   minE = ((Ex) < (Ey) ? (Ex) : (Ey));

   Ex = ((Ex)+(minE/3));
   Ey = ((Ey)+(minE/3));



   w[0] = (Ex);
   w[1] = (Ey);
}

static void intensity_stereo(const OpusCustomMode *m, celt_norm * restrict X, const celt_norm * restrict Y, const celt_ener *bandE, int bandID, int N)
{
   int i = bandID;
   int j;
   opus_val16 a1, a2;
   opus_val16 left, right;
   opus_val16 norm;



   left = (bandE[i]);
   right = (bandE[i+m->nbEBands]);
   norm = 1e-15f + ((float)sqrt(1e-15f +((opus_val32)(left)*(opus_val32)(left))+((opus_val32)(right)*(opus_val32)(right))));
   a1 = (((opus_val32)(((left))))/(opus_val16)(norm));
   a2 = (((opus_val32)(((right))))/(opus_val16)(norm));
   for (j=0;j<N;j++)
   {
      celt_norm r, l;
      l = X[j];
      r = Y[j];
      X[j] = ((((((opus_val32)(a1)*(opus_val32)(l)))+(opus_val32)(a2)*(opus_val32)(r))));

   }
}

static void stereo_split(celt_norm * restrict X, celt_norm * restrict Y, int N)
{
   int j;
   for (j=0;j<N;j++)
   {
      opus_val32 r, l;
      l = ((opus_val32)((.70710678f))*(opus_val32)(X[j]));
      r = ((opus_val32)((.70710678f))*(opus_val32)(Y[j]));
      X[j] = ((((l)+(r))));
      Y[j] = ((((r)-(l))));
   }
}

static void stereo_merge(celt_norm * restrict X, celt_norm * restrict Y, opus_val16 mid, int N, int arch)
{
   int j;
   opus_val32 xp=0, side=0;
   opus_val32 El, Er;
   opus_val16 mid2;



   opus_val32 t, lgain, rgain;


   ((void)(arch),dual_inner_prod_c(Y, X, Y, N, &xp, &side));

   xp = ((mid)*(xp));

   mid2 = (mid);
   El = ((opus_val32)(mid2)*(opus_val32)(mid2)) + side - 2*xp;
   Er = ((opus_val32)(mid2)*(opus_val32)(mid2)) + side + 2*xp;
   if (Er < (6e-4f) || El < (6e-4f))
   {
      (memcpy((Y), (X), (N)*sizeof(*(Y)) + 0*((Y)-(X)) ));
      return;
   }





   t = (El);
   lgain = ((1.f/((float)sqrt(t))));
   t = (Er);
   rgain = ((1.f/((float)sqrt(t))));
   for (j=0;j<N;j++)
   {
      celt_norm r, l;

      l = ((mid)*(X[j]));
      r = Y[j];
      X[j] = ((((opus_val32)(lgain)*(opus_val32)(((l)-(r))))));
      Y[j] = ((((opus_val32)(rgain)*(opus_val32)(((l)+(r))))));
   }
}


int spreading_decision(const OpusCustomMode *m, const celt_norm *X, int *average,
      int last_decision, int *hf_average, int *tapset_decision, int update_hf,
      int end, int C, int M, const int *spread_weight)
{
   int i, c, N0;
   int sum = 0, nbBands=0;
   const opus_int16 * restrict eBands = m->eBands;
   int decision;
   int hf_sum=0;

   ;

   N0 = M*m->shortMdctSize;

   if (M*(eBands[end]-eBands[end-1]) <= 8)
      return (0);
   c=0; do {
      for (i=0;i<end;i++)
      {
         int j, N, tmp=0;
         int tcount[3] = {0,0,0};
         const celt_norm * restrict x = X+M*eBands[i]+c*N0;
         N = M*(eBands[i+1]-eBands[i]);
         if (N<=8)
            continue;

         for (j=0;j<N;j++)
         {
            opus_val32 x2N;

            x2N = ((opus_val32)(((x[j])*(x[j])))*(opus_val32)(N));
            if (x2N < (0.25f))
               tcount[0]++;
            if (x2N < (0.0625f))
               tcount[1]++;
            if (x2N < (0.015625f))
               tcount[2]++;
         }


         if (i>m->nbEBands-4)
            hf_sum += celt_udiv(32*(tcount[1]+tcount[0]), N);
         tmp = (2*tcount[2] >= N) + (2*tcount[1] >= N) + (2*tcount[0] >= N);
         sum += tmp*spread_weight[i];
         nbBands+=spread_weight[i];
      }
   } while (++c<C);

   if (update_hf)
   {
      if (hf_sum)
         hf_sum = celt_udiv(hf_sum, C*(4-m->nbEBands+end));
      *hf_average = (*hf_average+hf_sum)>>1;
      hf_sum = *hf_average;
      if (*tapset_decision==2)
         hf_sum += 4;
      else if (*tapset_decision==0)
         hf_sum -= 4;
      if (hf_sum > 22)
         *tapset_decision=2;
      else if (hf_sum > 18)
         *tapset_decision=1;
      else
         *tapset_decision=0;
   }

   ;
   ;
   sum = celt_udiv((opus_int32)sum<<8, nbBands);

   sum = (sum+*average)>>1;
   *average = sum;

   sum = (3*sum + (((3-last_decision)<<7) + 64) + 2)>>2;
   if (sum < 80)
   {
      decision = (3);
   } else if (sum < 256)
   {
      decision = (2);
   } else if (sum < 384)
   {
      decision = (1);
   } else {
      decision = (0);
   }




   return decision;
}





static const int ordery_table[] = {
       1, 0,
       3, 0, 2, 1,
       7, 0, 4, 3, 6, 1, 5, 2,
      15, 0, 8, 7, 12, 3, 11, 4, 14, 1, 9, 6, 13, 2, 10, 5,
};

static void deinterleave_hadamard(celt_norm *X, int N0, int stride, int hadamard)
{
   int i,j;
   ;
   int N;
   ;
   N = N0*stride;
   celt_norm *tmp = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(N));
   ;
   if (hadamard)
   {
      const int *ordery = ordery_table+stride-2;
      for (i=0;i<stride;i++)
      {
         for (j=0;j<N0;j++)
            tmp[ordery[i]*N0+j] = X[j*stride+i];
      }
   } else {
      for (i=0;i<stride;i++)
         for (j=0;j<N0;j++)
            tmp[i*N0+j] = X[j*stride+i];
   }
   (memcpy((X), (tmp), (N)*sizeof(*(X)) + 0*((X)-(tmp)) ));
   ;
}

static void interleave_hadamard(celt_norm *X, int N0, int stride, int hadamard)
{
   int i,j;
   ;
   int N;
   ;
   N = N0*stride;
   celt_norm *tmp = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(N));
   if (hadamard)
   {
      const int *ordery = ordery_table+stride-2;
      for (i=0;i<stride;i++)
         for (j=0;j<N0;j++)
            tmp[j*stride+i] = X[ordery[i]*N0+j];
   } else {
      for (i=0;i<stride;i++)
         for (j=0;j<N0;j++)
            tmp[j*stride+i] = X[i*N0+j];
   }
   (memcpy((X), (tmp), (N)*sizeof(*(X)) + 0*((X)-(tmp)) ));
   ;
}

void haar1(celt_norm *X, int N0, int stride)
{
   int i, j;
   N0 >>= 1;
   for (i=0;i<stride;i++)
      for (j=0;j<N0;j++)
      {
         opus_val32 tmp1, tmp2;
         tmp1 = ((opus_val32)((.70710678f))*(opus_val32)(X[stride*2*j+i]));
         tmp2 = ((opus_val32)((.70710678f))*(opus_val32)(X[stride*(2*j+1)+i]));
         X[stride*2*j+i] = ((((tmp1)+(tmp2))));
         X[stride*(2*j+1)+i] = ((((tmp1)-(tmp2))));
      }
}

static int compute_qn(int N, int b, int offset, int pulse_cap, int stereo)
{
   static const opus_int16 exp2_table8[8] =
      {16384, 17866, 19483, 21247, 23170, 25267, 27554, 30048};
   int qn, qb;
   int N2 = 2*N-1;
   if (stereo && N==2)
      N2--;



   qb = celt_sudiv(b+N2*offset, N2);
   qb = ((b-pulse_cap-(4<<3)) < (qb) ? (b-pulse_cap-(4<<3)) : (qb));

   qb = ((8<<3) < (qb) ? (8<<3) : (qb));

   if (qb<(1<<3>>1)) {
      qn = 1;
   } else {
      qn = exp2_table8[qb&0x7]>>(14-(qb>>3));
      qn = (qn+1)>>1<<1;
   }
   ;
   return qn;
}

struct band_ctx {
   int encode;
   int resynth;
   const OpusCustomMode *m;
   int i;
   int intensity;
   int spread;
   int tf_change;
   ec_ctx *ec;
   opus_int32 remaining_bits;
   const celt_ener *bandE;
   opus_uint32 seed;
   int arch;
   int theta_round;
   int disable_inv;
   int avoid_split_noise;
};

struct split_ctx {
   int inv;
   int imid;
   int iside;
   int delta;
   int itheta;
   int qalloc;
};

static void compute_theta(struct band_ctx *ctx, struct split_ctx *sctx,
      celt_norm *X, celt_norm *Y, int N, int *b, int B, int B0,
      int LM,
      int stereo, int *fill)
{
   int qn;
   int itheta=0;
   int delta;
   int imid, iside;
   int qalloc;
   int pulse_cap;
   int offset;
   opus_int32 tell;
   int inv=0;
   int encode;
   const OpusCustomMode *m;
   int i;
   int intensity;
   ec_ctx *ec;
   const celt_ener *bandE;

   encode = ctx->encode;
   m = ctx->m;
   i = ctx->i;
   intensity = ctx->intensity;
   ec = ctx->ec;
   bandE = ctx->bandE;


   pulse_cap = m->logN[i]+LM*(1<<3);
   offset = (pulse_cap>>1) - (stereo&&N==2 ? 16 : 4);
   qn = compute_qn(N, *b, offset, pulse_cap, stereo);
   if (stereo && i>=intensity)
      qn = 1;
   if (encode)
   {




      itheta = stereo_itheta(X, Y, stereo, N, ctx->arch);
   }
   tell = ec_tell_frac(ec);
   if (qn!=1)
   {
      if (encode)
      {
         if (!stereo || ctx->theta_round == 0)
         {
            itheta = (itheta*(opus_int32)qn+8192)>>14;
            if (!stereo && ctx->avoid_split_noise && itheta > 0 && itheta < qn)
            {



               int unquantized = celt_udiv((opus_int32)itheta*16384, qn);
               imid = bitexact_cos((opus_int16)unquantized);
               iside = bitexact_cos((opus_int16)(16384-unquantized));
               delta = ((16384+((opus_int32)(opus_int16)((N-1)<<7)*(opus_int16)(bitexact_log2tan(iside,imid))))>>15);
               if (delta > *b)
                  itheta = qn;
               else if (delta < -*b)
                  itheta = 0;
            }
         } else {
            int down;

            int bias = itheta > 8192 ? 32767/qn : -32767/qn;
            down = ((qn-1) < (((0) > ((itheta*(opus_int32)qn + bias)>>14) ? (0) : ((itheta*(opus_int32)qn + bias)>>14))) ? (qn-1) : (((0) > ((itheta*(opus_int32)qn + bias)>>14) ? (0) : ((itheta*(opus_int32)qn + bias)>>14))));
            if (ctx->theta_round < 0)
               itheta = down;
            else
               itheta = down+1;
         }
      }


      if (stereo && N>2)
      {
         int p0 = 3;
         int x = itheta;
         int x0 = qn/2;
         int ft = p0*(x0+1) + x0;

         if (encode)
         {
            ec_encode(ec,x<=x0?p0*x:(x-1-x0)+(x0+1)*p0,x<=x0?p0*(x+1):(x-x0)+(x0+1)*p0,ft);
         } else {
            int fs;
            fs=ec_decode(ec,ft);
            if (fs<(x0+1)*p0)
               x=fs/p0;
            else
               x=x0+1+(fs-(x0+1)*p0);
            ec_dec_update(ec,x<=x0?p0*x:(x-1-x0)+(x0+1)*p0,x<=x0?p0*(x+1):(x-x0)+(x0+1)*p0,ft);
            itheta = x;
         }
      } else if (B0>1 || stereo) {

         if (encode)
            ec_enc_uint(ec, itheta, qn+1);
         else
            itheta = ec_dec_uint(ec, qn+1);
      } else {
         int fs=1, ft;
         ft = ((qn>>1)+1)*((qn>>1)+1);
         if (encode)
         {
            int fl;

            fs = itheta <= (qn>>1) ? itheta + 1 : qn + 1 - itheta;
            fl = itheta <= (qn>>1) ? itheta*(itheta + 1)>>1 :
             ft - ((qn + 1 - itheta)*(qn + 2 - itheta)>>1);

            ec_encode(ec, fl, fl+fs, ft);
         } else {

            int fl=0;
            int fm;
            fm = ec_decode(ec, ft);

            if (fm < ((qn>>1)*((qn>>1) + 1)>>1))
            {
               itheta = (isqrt32(8*(opus_uint32)fm + 1) - 1)>>1;
               fs = itheta + 1;
               fl = itheta*(itheta + 1)>>1;
            }
            else
            {
               itheta = (2*(qn + 1)
                - isqrt32(8*(opus_uint32)(ft - fm - 1) + 1))>>1;
               fs = qn + 1 - itheta;
               fl = ft - ((qn + 1 - itheta)*(qn + 2 - itheta)>>1);
            }

            ec_dec_update(ec, fl, fl+fs, ft);
         }
      }
      ;
      itheta = celt_udiv((opus_int32)itheta*16384, qn);
      if (encode && stereo)
      {
         if (itheta==0)
            intensity_stereo(m, X, Y, bandE, i, N);
         else
            stereo_split(X, Y, N);
      }


   } else if (stereo) {
      if (encode)
      {
         inv = itheta > 8192 && !ctx->disable_inv;
         if (inv)
         {
            int j;
            for (j=0;j<N;j++)
               Y[j] = -Y[j];
         }
         intensity_stereo(m, X, Y, bandE, i, N);
      }
      if (*b>2<<3 && ctx->remaining_bits > 2<<3)
      {
         if (encode)
            ec_enc_bit_logp(ec, inv, 2);
         else
            inv = ec_dec_bit_logp(ec, 2);
      } else
         inv = 0;

      if (ctx->disable_inv)
         inv = 0;
      itheta = 0;
   }
   qalloc = ec_tell_frac(ec) - tell;
   *b -= qalloc;

   if (itheta == 0)
   {
      imid = 32767;
      iside = 0;
      *fill &= (1<<B)-1;
      delta = -16384;
   } else if (itheta == 16384)
   {
      imid = 0;
      iside = 32767;
      *fill &= ((1<<B)-1)<<B;
      delta = 16384;
   } else {
      imid = bitexact_cos((opus_int16)itheta);
      iside = bitexact_cos((opus_int16)(16384-itheta));


      delta = ((16384+((opus_int32)(opus_int16)((N-1)<<7)*(opus_int16)(bitexact_log2tan(iside,imid))))>>15);
   }

   sctx->inv = inv;
   sctx->imid = imid;
   sctx->iside = iside;
   sctx->delta = delta;
   sctx->itheta = itheta;
   sctx->qalloc = qalloc;
}
static unsigned quant_band_n1(struct band_ctx *ctx, celt_norm *X, celt_norm *Y,
      celt_norm *lowband_out)
{
   int c;
   int stereo;
   celt_norm *x = X;
   int encode;
   ec_ctx *ec;

   encode = ctx->encode;
   ec = ctx->ec;

   stereo = Y != 
                ((void *)0)
                    ;
   c=0; do {
      int sign=0;
      if (ctx->remaining_bits>=1<<3)
      {
         if (encode)
         {
            sign = x[0]<0;
            ec_enc_bits(ec, sign, 1);
         } else {
            sign = ec_dec_bits(ec, 1);
         }
         ctx->remaining_bits -= 1<<3;
      }
      if (ctx->resynth)
         x[0] = sign ? -1.f : 1.f;
      x = Y;
   } while (++c<1+stereo);
   if (lowband_out)
      lowband_out[0] = (X[0]);
   return 1;
}





static unsigned quant_partition(struct band_ctx *ctx, celt_norm *X,
      int N, int b, int B, celt_norm *lowband,
      int LM,
      opus_val16 gain, int fill)
{
   const unsigned char *cache;
   int q;
   int curr_bits;
   int imid=0, iside=0;
   int B0=B;
   opus_val16 mid=0, side=0;
   unsigned cm=0;
   celt_norm *Y=
               ((void *)0)
                   ;
   int encode;
   const OpusCustomMode *m;
   int i;
   int spread;
   ec_ctx *ec;

   encode = ctx->encode;
   m = ctx->m;
   i = ctx->i;
   spread = ctx->spread;
   ec = ctx->ec;


   cache = m->cache.bits + m->cache.index[(LM+1)*m->nbEBands+i];
   if (LM != -1 && b > cache[cache[0]]+12 && N>2)
   {
      int mbits, sbits, delta;
      int itheta;
      int qalloc;
      struct split_ctx sctx;
      celt_norm *next_lowband2=
                              ((void *)0)
                                  ;
      opus_int32 rebalance;

      N >>= 1;
      Y = X+N;
      LM -= 1;
      if (B==1)
         fill = (fill&1)|(fill<<1);
      B = (B+1)>>1;

      compute_theta(ctx, &sctx, X, Y, N, &b, B, B0, LM, 0, &fill);
      imid = sctx.imid;
      iside = sctx.iside;
      delta = sctx.delta;
      itheta = sctx.itheta;
      qalloc = sctx.qalloc;




      mid = (1.f/32768)*imid;
      side = (1.f/32768)*iside;



      if (B0>1 && (itheta&0x3fff))
      {
         if (itheta > 8192)

            delta -= delta>>(4-LM);
         else

            delta = ((0) < (delta + (N<<3>>(5-LM))) ? (0) : (delta + (N<<3>>(5-LM))));
      }
      mbits = ((0) > (((b) < ((b-delta)/2) ? (b) : ((b-delta)/2))) ? (0) : (((b) < ((b-delta)/2) ? (b) : ((b-delta)/2))));
      sbits = b-mbits;
      ctx->remaining_bits -= qalloc;

      if (lowband)
         next_lowband2 = lowband+N;

      rebalance = ctx->remaining_bits;
      if (mbits >= sbits)
      {
         cm = quant_partition(ctx, X, N, mbits, B, lowband, LM,
               ((gain)*(mid)), fill);
         rebalance = mbits - (rebalance-ctx->remaining_bits);
         if (rebalance > 3<<3 && itheta!=0)
            sbits += rebalance - (3<<3);
         cm |= quant_partition(ctx, Y, N, sbits, B, next_lowband2, LM,
               ((gain)*(side)), fill>>B)<<(B0>>1);
      } else {
         cm = quant_partition(ctx, Y, N, sbits, B, next_lowband2, LM,
               ((gain)*(side)), fill>>B)<<(B0>>1);
         rebalance = sbits - (rebalance-ctx->remaining_bits);
         if (rebalance > 3<<3 && itheta!=16384)
            mbits += rebalance - (3<<3);
         cm |= quant_partition(ctx, X, N, mbits, B, lowband, LM,
               ((gain)*(mid)), fill);
      }
   } else {

      q = bits2pulses(m, i, LM, b);
      curr_bits = pulses2bits(m, i, LM, q);
      ctx->remaining_bits -= curr_bits;


      while (ctx->remaining_bits < 0 && q > 0)
      {
         ctx->remaining_bits += curr_bits;
         q--;
         curr_bits = pulses2bits(m, i, LM, q);
         ctx->remaining_bits -= curr_bits;
      }

      if (q!=0)
      {
         int K = get_pulses(q);


         if (encode)
         {
            cm = alg_quant(X, N, K, spread, B, ec, gain, ctx->resynth, ctx->arch);
         } else {
            cm = alg_unquant(X, N, K, spread, B, ec, gain);
         }
      } else {

         int j;
         if (ctx->resynth)
         {
            unsigned cm_mask;


            cm_mask = (unsigned)(1UL<<B)-1;
            fill &= cm_mask;
            if (!fill)
            {
               (memset((X), 0, (N)*sizeof(*(X))));
            } else {
               if (lowband == 
                             ((void *)0)
                                 )
               {

                  for (j=0;j<N;j++)
                  {
                     ctx->seed = celt_lcg_rand(ctx->seed);
                     X[j] = (celt_norm)((opus_int32)ctx->seed>>20);
                  }
                  cm = cm_mask;
               } else {

                  for (j=0;j<N;j++)
                  {
                     opus_val16 tmp;
                     ctx->seed = celt_lcg_rand(ctx->seed);

                     tmp = (1.0f/256);
                     tmp = (ctx->seed)&0x8000 ? tmp : -tmp;
                     X[j] = lowband[j]+tmp;
                  }
                  cm = fill;
               }
               renormalise_vector(X, N, gain, ctx->arch);
            }
         }
      }
   }

   return cm;
}



static unsigned quant_band(struct band_ctx *ctx, celt_norm *X,
      int N, int b, int B, celt_norm *lowband,
      int LM, celt_norm *lowband_out,
      opus_val16 gain, celt_norm *lowband_scratch, int fill)
{
   int N0=N;
   int N_B=N;
   int N_B0;
   int B0=B;
   int time_divide=0;
   int recombine=0;
   int longBlocks;
   unsigned cm=0;
   int k;
   int encode;
   int tf_change;

   encode = ctx->encode;
   tf_change = ctx->tf_change;

   longBlocks = B0==1;

   N_B = celt_udiv(N_B, B);


   if (N==1)
   {
      return quant_band_n1(ctx, X, 
                                  ((void *)0)
                                      , lowband_out);
   }

   if (tf_change>0)
      recombine = tf_change;


   if (lowband_scratch && lowband && (recombine || ((N_B&1) == 0 && tf_change<0) || B0>1))
   {
      (memcpy((lowband_scratch), (lowband), (N)*sizeof(*(lowband_scratch)) + 0*((lowband_scratch)-(lowband)) ));
      lowband = lowband_scratch;
   }

   for (k=0;k<recombine;k++)
   {
      static const unsigned char bit_interleave_table[16]={
            0,1,1,1,2,3,3,3,2,3,3,3,2,3,3,3
      };
      if (encode)
         haar1(X, N>>k, 1<<k);
      if (lowband)
         haar1(lowband, N>>k, 1<<k);
      fill = bit_interleave_table[fill&0xF]|bit_interleave_table[fill>>4]<<2;
   }
   B>>=recombine;
   N_B<<=recombine;


   while ((N_B&1) == 0 && tf_change<0)
   {
      if (encode)
         haar1(X, N_B, B);
      if (lowband)
         haar1(lowband, N_B, B);
      fill |= fill<<B;
      B <<= 1;
      N_B >>= 1;
      time_divide++;
      tf_change++;
   }
   B0=B;
   N_B0 = N_B;


   if (B0>1)
   {
      if (encode)
         deinterleave_hadamard(X, N_B>>recombine, B0<<recombine, longBlocks);
      if (lowband)
         deinterleave_hadamard(lowband, N_B>>recombine, B0<<recombine, longBlocks);
   }

   cm = quant_partition(ctx, X, N, b, B, lowband, LM, gain, fill);


   if (ctx->resynth)
   {

      if (B0>1)
         interleave_hadamard(X, N_B>>recombine, B0<<recombine, longBlocks);


      N_B = N_B0;
      B = B0;
      for (k=0;k<time_divide;k++)
      {
         B >>= 1;
         N_B <<= 1;
         cm |= cm>>B;
         haar1(X, N_B, B);
      }

      for (k=0;k<recombine;k++)
      {
         static const unsigned char bit_deinterleave_table[16]={
               0x00,0x03,0x0C,0x0F,0x30,0x33,0x3C,0x3F,
               0xC0,0xC3,0xCC,0xCF,0xF0,0xF3,0xFC,0xFF
         };
         cm = bit_deinterleave_table[cm];
         haar1(X, N0>>k, 1<<k);
      }
      B<<=recombine;


      if (lowband_out)
      {
         int j;
         opus_val16 n;
         n = ((float)sqrt(((N0))));
         for (j=0;j<N0;j++)
            lowband_out[j] = ((n)*(X[j]));
      }
      cm &= (1<<B)-1;
   }
   return cm;
}



static unsigned quant_band_stereo(struct band_ctx *ctx, celt_norm *X, celt_norm *Y,
      int N, int b, int B, celt_norm *lowband,
      int LM, celt_norm *lowband_out,
      celt_norm *lowband_scratch, int fill)
{
   int imid=0, iside=0;
   int inv = 0;
   opus_val16 mid=0, side=0;
   unsigned cm=0;
   int mbits, sbits, delta;
   int itheta;
   int qalloc;
   struct split_ctx sctx;
   int orig_fill;
   int encode;
   ec_ctx *ec;

   encode = ctx->encode;
   ec = ctx->ec;


   if (N==1)
   {
      return quant_band_n1(ctx, X, Y, lowband_out);
   }

   orig_fill = fill;

   compute_theta(ctx, &sctx, X, Y, N, &b, B, B, LM, 1, &fill);
   inv = sctx.inv;
   imid = sctx.imid;
   iside = sctx.iside;
   delta = sctx.delta;
   itheta = sctx.itheta;
   qalloc = sctx.qalloc;




   mid = (1.f/32768)*imid;
   side = (1.f/32768)*iside;





   if (N==2)
   {
      int c;
      int sign=0;
      celt_norm *x2, *y2;
      mbits = b;
      sbits = 0;

      if (itheta != 0 && itheta != 16384)
         sbits = 1<<3;
      mbits -= sbits;
      c = itheta > 8192;
      ctx->remaining_bits -= qalloc+sbits;

      x2 = c ? Y : X;
      y2 = c ? X : Y;
      if (sbits)
      {
         if (encode)
         {

            sign = x2[0]*y2[1] - x2[1]*y2[0] < 0;
            ec_enc_bits(ec, sign, 1);
         } else {
            sign = ec_dec_bits(ec, 1);
         }
      }
      sign = 1-2*sign;


      cm = quant_band(ctx, x2, N, mbits, B, lowband, LM, lowband_out, 1.0f,
            lowband_scratch, orig_fill);


      y2[0] = -sign*x2[1];
      y2[1] = sign*x2[0];
      if (ctx->resynth)
      {
         celt_norm tmp;
         X[0] = ((mid)*(X[0]));
         X[1] = ((mid)*(X[1]));
         Y[0] = ((side)*(Y[0]));
         Y[1] = ((side)*(Y[1]));
         tmp = X[0];
         X[0] = ((tmp)-(Y[0]));
         Y[0] = ((tmp)+(Y[0]));
         tmp = X[1];
         X[1] = ((tmp)-(Y[1]));
         Y[1] = ((tmp)+(Y[1]));
      }
   } else {

      opus_int32 rebalance;

      mbits = ((0) > (((b) < ((b-delta)/2) ? (b) : ((b-delta)/2))) ? (0) : (((b) < ((b-delta)/2) ? (b) : ((b-delta)/2))));
      sbits = b-mbits;
      ctx->remaining_bits -= qalloc;

      rebalance = ctx->remaining_bits;
      if (mbits >= sbits)
      {


         cm = quant_band(ctx, X, N, mbits, B, lowband, LM, lowband_out, 1.0f,
               lowband_scratch, fill);
         rebalance = mbits - (rebalance-ctx->remaining_bits);
         if (rebalance > 3<<3 && itheta!=0)
            sbits += rebalance - (3<<3);



         cm |= quant_band(ctx, Y, N, sbits, B, 
                                              ((void *)0)
                                                  , LM, 
                                                        ((void *)0)
                                                            , side, 
                                                                    ((void *)0)
                                                                        , fill>>B);
      } else {


         cm = quant_band(ctx, Y, N, sbits, B, 
                                             ((void *)0)
                                                 , LM, 
                                                       ((void *)0)
                                                           , side, 
                                                                   ((void *)0)
                                                                       , fill>>B);
         rebalance = sbits - (rebalance-ctx->remaining_bits);
         if (rebalance > 3<<3 && itheta!=16384)
            mbits += rebalance - (3<<3);


         cm |= quant_band(ctx, X, N, mbits, B, lowband, LM, lowband_out, 1.0f,
               lowband_scratch, fill);
      }
   }



   if (ctx->resynth)
   {
      if (N!=2)
         stereo_merge(X, Y, mid, N, ctx->arch);
      if (inv)
      {
         int j;
         for (j=0;j<N;j++)
            Y[j] = -Y[j];
      }
   }
   return cm;
}


static void special_hybrid_folding(const OpusCustomMode *m, celt_norm *norm, celt_norm *norm2, int start, int M, int dual_stereo)
{
   int n1, n2;
   const opus_int16 * restrict eBands = m->eBands;
   n1 = M*(eBands[start+1]-eBands[start]);
   n2 = M*(eBands[start+2]-eBands[start+1]);


   (memcpy((&norm[n1]), (&norm[2*n1 - n2]), (n2-n1)*sizeof(*(&norm[n1])) + 0*((&norm[n1])-(&norm[2*n1 - n2])) ));
   if (dual_stereo)
      (memcpy((&norm2[n1]), (&norm2[2*n1 - n2]), (n2-n1)*sizeof(*(&norm2[n1])) + 0*((&norm2[n1])-(&norm2[2*n1 - n2])) ));
}


void quant_all_bands(int encode, const OpusCustomMode *m, int start, int end,
      celt_norm *X_, celt_norm *Y_, unsigned char *collapse_masks,
      const celt_ener *bandE, int *pulses, int shortBlocks, int spread,
      int dual_stereo, int intensity, int *tf_res, opus_int32 total_bits,
      opus_int32 balance, ec_ctx *ec, int LM, int codedBands,
      opus_uint32 *seed, int complexity, int arch, int disable_inv)
{
   int i;
   opus_int32 remaining_bits;
   const opus_int16 * restrict eBands = m->eBands;
   celt_norm * restrict norm, * restrict norm2;
   ;
   ;
   ;
   ;
   ;
   ;
   ;
   int resynth_alloc;
   celt_norm *lowband_scratch;
   int B;
   int M;
   int lowband_offset;
   int update_lowband = 1;
   int C = Y_ != 
                ((void *)0) 
                     ? 2 : 1;
   int norm_offset;
   int theta_rdo = encode && Y_!=
                                ((void *)0) 
                                     && !dual_stereo && complexity>=8;



   int resynth = !encode || theta_rdo;

   struct band_ctx ctx;
   ;

   M = 1<<LM;
   B = shortBlocks ? M : 1;
   norm_offset = M*eBands[start];


   celt_norm *_norm = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(C*(M*eBands[m->nbEBands-1]-norm_offset)));
   norm = _norm;
   norm2 = norm + M*eBands[m->nbEBands-1]-norm_offset;




   if (encode && resynth)
      resynth_alloc = M*(eBands[m->nbEBands]-eBands[m->nbEBands-1]);
   else
      resynth_alloc = 1;
   celt_norm *_lowband_scratch = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(resynth_alloc));
   if (encode && resynth)
      lowband_scratch = _lowband_scratch;
   else
      lowband_scratch = X_+M*eBands[m->nbEBands-1];
   celt_norm *X_save = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(resynth_alloc));
   celt_norm *Y_save = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(resynth_alloc));
   celt_norm *X_save2 = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(resynth_alloc));
   celt_norm *Y_save2 = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(resynth_alloc));
   celt_norm *norm_save2 = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(resynth_alloc));

   lowband_offset = 0;
   ctx.bandE = bandE;
   ctx.ec = ec;
   ctx.encode = encode;
   ctx.intensity = intensity;
   ctx.m = m;
   ctx.seed = *seed;
   ctx.spread = spread;
   ctx.arch = arch;
   ctx.disable_inv = disable_inv;
   ctx.resynth = resynth;
   ctx.theta_round = 0;

   ctx.avoid_split_noise = B > 1;
   for (i=start;i<end;i++)
   {
      opus_int32 tell;
      int b;
      int N;
      opus_int32 curr_balance;
      int effective_lowband=-1;
      celt_norm * restrict X, * restrict Y;
      int tf_change=0;
      unsigned x_cm;
      unsigned y_cm;
      int last;

      ctx.i = i;
      last = (i==end-1);

      X = X_+M*eBands[i];
      if (Y_!=
             ((void *)0)
                 )
         Y = Y_+M*eBands[i];
      else
         Y = 
            ((void *)0)
                ;
      N = M*eBands[i+1]-M*eBands[i];
      ;
      tell = ec_tell_frac(ec);


      if (i != start)
         balance -= tell;
      remaining_bits = total_bits-tell-1;
      ctx.remaining_bits = remaining_bits;
      if (i <= codedBands-1)
      {
         curr_balance = celt_sudiv(balance, ((3) < (codedBands-i) ? (3) : (codedBands-i)));
         b = ((0) > (((16383) < (((remaining_bits+1) < (pulses[i]+curr_balance) ? (remaining_bits+1) : (pulses[i]+curr_balance))) ? (16383) : (((remaining_bits+1) < (pulses[i]+curr_balance) ? (remaining_bits+1) : (pulses[i]+curr_balance))))) ? (0) : (((16383) < (((remaining_bits+1) < (pulses[i]+curr_balance) ? (remaining_bits+1) : (pulses[i]+curr_balance))) ? (16383) : (((remaining_bits+1) < (pulses[i]+curr_balance) ? (remaining_bits+1) : (pulses[i]+curr_balance))))));
      } else {
         b = 0;
      }


      if (resynth && (M*eBands[i]-N >= M*eBands[start] || i==start+1) && (update_lowband || lowband_offset==0))
            lowband_offset = i;
      if (i == start+1)
         special_hybrid_folding(m, norm, norm2, start, M, dual_stereo);





      tf_change = tf_res[i];
      ctx.tf_change = tf_change;
      if (i>=m->effEBands)
      {
         X=norm;
         if (Y_!=
                ((void *)0)
                    )
            Y = norm;
         lowband_scratch = 
                          ((void *)0)
                              ;
      }
      if (last && !theta_rdo)
         lowband_scratch = 
                          ((void *)0)
                              ;



      if (lowband_offset != 0 && (spread!=(3) || B>1 || tf_change<0))
      {
         int fold_start;
         int fold_end;
         int fold_i;

         effective_lowband = ((0) > (M*eBands[lowband_offset]-norm_offset-N) ? (0) : (M*eBands[lowband_offset]-norm_offset-N));
         fold_start = lowband_offset;
         while(M*eBands[--fold_start] > effective_lowband+norm_offset);
         fold_end = lowband_offset-1;

         while(++fold_end < i && M*eBands[fold_end] < effective_lowband+norm_offset+N);



         x_cm = y_cm = 0;
         fold_i = fold_start; do {
           x_cm |= collapse_masks[fold_i*C+0];
           y_cm |= collapse_masks[fold_i*C+C-1];
         } while (++fold_i<fold_end);
      }


      else
         x_cm = y_cm = (1<<B)-1;

      if (dual_stereo && i==intensity)
      {
         int j;


         dual_stereo = 0;
         if (resynth)
            for (j=0;j<M*eBands[i]-norm_offset;j++)
               norm[j] = (.5f*(norm[j]+norm2[j]));
      }
      if (dual_stereo)
      {
         x_cm = quant_band(&ctx, X, N, b/2, B,
               effective_lowband != -1 ? norm+effective_lowband : 
                                                                 ((void *)0)
                                                                     , LM,
               last?
                   ((void *)0)
                       :norm+M*eBands[i]-norm_offset, 1.0f, lowband_scratch, x_cm);
         y_cm = quant_band(&ctx, Y, N, b/2, B,
               effective_lowband != -1 ? norm2+effective_lowband : 
                                                                  ((void *)0)
                                                                      , LM,
               last?
                   ((void *)0)
                       :norm2+M*eBands[i]-norm_offset, 1.0f, lowband_scratch, y_cm);
      } else {
         if (Y!=
               ((void *)0)
                   )
         {
            if (theta_rdo && i < intensity)
            {
               ec_ctx ec_save, ec_save2;
               struct band_ctx ctx_save, ctx_save2;
               opus_val32 dist0, dist1;
               unsigned cm, cm2;
               int nstart_bytes, nend_bytes, save_bytes;
               unsigned char *bytes_buf;
               unsigned char bytes_save[1275];
               opus_val16 w[2];
               compute_channel_weights(bandE[i], bandE[i+m->nbEBands], w);

               cm = x_cm|y_cm;
               ec_save = *ec;
               ctx_save = ctx;
               (memcpy((X_save), (X), (N)*sizeof(*(X_save)) + 0*((X_save)-(X)) ));
               (memcpy((Y_save), (Y), (N)*sizeof(*(Y_save)) + 0*((Y_save)-(Y)) ));

               ctx.theta_round = -1;
               x_cm = quant_band_stereo(&ctx, X, Y, N, b, B,
                     effective_lowband != -1 ? norm+effective_lowband : 
                                                                       ((void *)0)
                                                                           , LM,
                     last?
                         ((void *)0)
                             :norm+M*eBands[i]-norm_offset, lowband_scratch, cm);
               dist0 = ((w[0])*(((void)(arch),celt_inner_prod_c(X_save, X, N)))) + ((w[1])*(((void)(arch),celt_inner_prod_c(Y_save, Y, N))));


               cm2 = x_cm;
               ec_save2 = *ec;
               ctx_save2 = ctx;
               (memcpy((X_save2), (X), (N)*sizeof(*(X_save2)) + 0*((X_save2)-(X)) ));
               (memcpy((Y_save2), (Y), (N)*sizeof(*(Y_save2)) + 0*((Y_save2)-(Y)) ));
               if (!last)
                  (memcpy((norm_save2), (norm+M*eBands[i]-norm_offset), (N)*sizeof(*(norm_save2)) + 0*((norm_save2)-(norm+M*eBands[i]-norm_offset)) ));
               nstart_bytes = ec_save.offs;
               nend_bytes = ec_save.storage;
               bytes_buf = ec_save.buf+nstart_bytes;
               save_bytes = nend_bytes-nstart_bytes;
               (memcpy((bytes_save), (bytes_buf), (save_bytes)*sizeof(*(bytes_save)) + 0*((bytes_save)-(bytes_buf)) ));


               *ec = ec_save;
               ctx = ctx_save;
               (memcpy((X), (X_save), (N)*sizeof(*(X)) + 0*((X)-(X_save)) ));
               (memcpy((Y), (Y_save), (N)*sizeof(*(Y)) + 0*((Y)-(Y_save)) ));

               if (i == start+1)
                  special_hybrid_folding(m, norm, norm2, start, M, dual_stereo);


               ctx.theta_round = 1;
               x_cm = quant_band_stereo(&ctx, X, Y, N, b, B,
                     effective_lowband != -1 ? norm+effective_lowband : 
                                                                       ((void *)0)
                                                                           , LM,
                     last?
                         ((void *)0)
                             :norm+M*eBands[i]-norm_offset, lowband_scratch, cm);
               dist1 = ((w[0])*(((void)(arch),celt_inner_prod_c(X_save, X, N)))) + ((w[1])*(((void)(arch),celt_inner_prod_c(Y_save, Y, N))));
               if (dist0 >= dist1) {
                  x_cm = cm2;
                  *ec = ec_save2;
                  ctx = ctx_save2;
                  (memcpy((X), (X_save2), (N)*sizeof(*(X)) + 0*((X)-(X_save2)) ));
                  (memcpy((Y), (Y_save2), (N)*sizeof(*(Y)) + 0*((Y)-(Y_save2)) ));
                  if (!last)
                     (memcpy((norm+M*eBands[i]-norm_offset), (norm_save2), (N)*sizeof(*(norm+M*eBands[i]-norm_offset)) + 0*((norm+M*eBands[i]-norm_offset)-(norm_save2)) ));
                  (memcpy((bytes_buf), (bytes_save), (save_bytes)*sizeof(*(bytes_buf)) + 0*((bytes_buf)-(bytes_save)) ));
               }
            } else {
               ctx.theta_round = 0;
               x_cm = quant_band_stereo(&ctx, X, Y, N, b, B,
                     effective_lowband != -1 ? norm+effective_lowband : 
                                                                       ((void *)0)
                                                                           , LM,
                     last?
                         ((void *)0)
                             :norm+M*eBands[i]-norm_offset, lowband_scratch, x_cm|y_cm);
            }
         } else {
            x_cm = quant_band(&ctx, X, N, b, B,
                  effective_lowband != -1 ? norm+effective_lowband : 
                                                                    ((void *)0)
                                                                        , LM,
                  last?
                      ((void *)0)
                          :norm+M*eBands[i]-norm_offset, 1.0f, lowband_scratch, x_cm|y_cm);
         }
         y_cm = x_cm;
      }
      collapse_masks[i*C+0] = (unsigned char)x_cm;
      collapse_masks[i*C+C-1] = (unsigned char)y_cm;
      balance += pulses[i] + tell;


      update_lowband = b>(N<<3);


      ctx.avoid_split_noise = 0;
   }
   *seed = ctx.seed;

   ;
}
void ec_laplace_encode(ec_enc *enc, int *value, unsigned fs, int decay);
int ec_laplace_decode(ec_dec *dec, unsigned fs, int decay);
const opus_val16 eMeans[25] = {
      6.437500f, 6.250000f, 5.750000f, 5.312500f, 5.062500f,
      4.812500f, 4.500000f, 4.375000f, 4.875000f, 4.687500f,
      4.562500f, 4.437500f, 4.875000f, 4.625000f, 4.312500f,
      4.500000f, 4.375000f, 4.625000f, 4.750000f, 4.437500f,
      3.750000f, 3.750000f, 3.750000f, 3.750000f, 3.750000f
};







static const opus_val16 pred_coef[4] = {29440/32768., 26112/32768., 21248/32768., 16384/32768.};
static const opus_val16 beta_coef[4] = {30147/32768., 22282/32768., 12124/32768., 6554/32768.};
static const opus_val16 beta_intra = 4915/32768.;







static const unsigned char e_prob_model[4][2][42] = {

   {

      {
          72, 127, 65, 129, 66, 128, 65, 128, 64, 128, 62, 128, 64, 128,
          64, 128, 92, 78, 92, 79, 92, 78, 90, 79, 116, 41, 115, 40,
         114, 40, 132, 26, 132, 26, 145, 17, 161, 12, 176, 10, 177, 11
      },

      {
          24, 179, 48, 138, 54, 135, 54, 132, 53, 134, 56, 133, 55, 132,
          55, 132, 61, 114, 70, 96, 74, 88, 75, 88, 87, 74, 89, 66,
          91, 67, 100, 59, 108, 50, 120, 40, 122, 37, 97, 43, 78, 50
      }
   },

   {

      {
          83, 78, 84, 81, 88, 75, 86, 74, 87, 71, 90, 73, 93, 74,
          93, 74, 109, 40, 114, 36, 117, 34, 117, 34, 143, 17, 145, 18,
         146, 19, 162, 12, 165, 10, 178, 7, 189, 6, 190, 8, 177, 9
      },

      {
          23, 178, 54, 115, 63, 102, 66, 98, 69, 99, 74, 89, 71, 91,
          73, 91, 78, 89, 86, 80, 92, 66, 93, 64, 102, 59, 103, 60,
         104, 60, 117, 52, 123, 44, 138, 35, 133, 31, 97, 38, 77, 45
      }
   },

   {

      {
          61, 90, 93, 60, 105, 42, 107, 41, 110, 45, 116, 38, 113, 38,
         112, 38, 124, 26, 132, 27, 136, 19, 140, 20, 155, 14, 159, 16,
         158, 18, 170, 13, 177, 10, 187, 8, 192, 6, 175, 9, 159, 10
      },

      {
          21, 178, 59, 110, 71, 86, 75, 85, 84, 83, 91, 66, 88, 73,
          87, 72, 92, 75, 98, 72, 105, 58, 107, 54, 115, 52, 114, 55,
         112, 56, 129, 51, 132, 40, 150, 33, 140, 29, 98, 35, 77, 42
      }
   },

   {

      {
          42, 121, 96, 66, 108, 43, 111, 40, 117, 44, 123, 32, 120, 36,
         119, 33, 127, 33, 134, 34, 139, 21, 147, 23, 152, 20, 158, 25,
         154, 26, 166, 21, 173, 16, 184, 13, 184, 10, 150, 13, 139, 15
      },

      {
          22, 178, 63, 114, 74, 82, 84, 83, 92, 82, 103, 62, 96, 72,
          96, 67, 101, 73, 107, 72, 113, 55, 118, 52, 125, 52, 118, 52,
         117, 55, 135, 49, 137, 39, 157, 32, 145, 29, 97, 33, 77, 40
      }
   }
};

static const unsigned char small_energy_icdf[3]={2,1,0};

static opus_val32 loss_distortion(const opus_val16 *eBands, opus_val16 *oldEBands, int start, int end, int len, int C)
{
   int c, i;
   opus_val32 dist = 0;
   c=0; do {
      for (i=start;i<end;i++)
      {
         opus_val16 d = (((eBands[i+c*len]))-((oldEBands[i+c*len])));
         dist = ((dist)+(opus_val32)(d)*(opus_val32)(d));
      }
   } while (++c<C);
   return ((200) < ((dist)) ? (200) : ((dist)));
}

static int quant_coarse_energy_impl(const OpusCustomMode *m, int start, int end,
      const opus_val16 *eBands, opus_val16 *oldEBands,
      opus_int32 budget, opus_int32 tell,
      const unsigned char *prob_model, opus_val16 *error, ec_enc *enc,
      int C, int LM, int intra, opus_val16 max_decay, int lfe)
{
   int i, c;
   int badness = 0;
   opus_val32 prev[2] = {0,0};
   opus_val16 coef;
   opus_val16 beta;

   if (tell+3 <= budget)
      ec_enc_bit_logp(enc, intra, 3);
   if (intra)
   {
      coef = 0;
      beta = beta_intra;
   } else {
      beta = beta_coef[LM];
      coef = pred_coef[LM];
   }


   for (i=start;i<end;i++)
   {
      c=0;
      do {
         int bits_left;
         int qi, qi0;
         opus_val32 q;
         opus_val16 x;
         opus_val32 f, tmp;
         opus_val16 oldE;
         opus_val16 decay_bound;
         x = eBands[i+c*m->nbEBands];
         oldE = ((-(9.f)) > (oldEBands[i+c*m->nbEBands]) ? (-(9.f)) : (oldEBands[i+c*m->nbEBands]));







         f = x-coef*oldE-prev[c];

         qi = (int)floor(.5f+f);
         decay_bound = ((-(28.f)) > (oldEBands[i+c*m->nbEBands]) ? (-(28.f)) : (oldEBands[i+c*m->nbEBands])) - max_decay;



         if (qi < 0 && x < decay_bound)
         {
            qi += (int)(((decay_bound)-(x)));
            if (qi > 0)
               qi = 0;
         }
         qi0 = qi;


         tell = ec_tell(enc);
         bits_left = budget-tell-3*C*(end-i);
         if (i!=start && bits_left < 30)
         {
            if (bits_left < 24)
               qi = ((1) < (qi) ? (1) : (qi));
            if (bits_left < 16)
               qi = ((-1) > (qi) ? (-1) : (qi));
         }
         if (lfe && i>=2)
            qi = ((qi) < (0) ? (qi) : (0));
         if (budget-tell >= 15)
         {
            int pi;
            pi = 2*((i) < (20) ? (i) : (20));
            ec_laplace_encode(enc, &qi,
                  prob_model[pi]<<7, prob_model[pi+1]<<6);
         }
         else if(budget-tell >= 2)
         {
            qi = ((-1) > (((qi) < (1) ? (qi) : (1))) ? (-1) : (((qi) < (1) ? (qi) : (1))));
            ec_enc_icdf(enc, 2*qi^-(qi<0), small_energy_icdf, 2);
         }
         else if(budget-tell >= 1)
         {
            qi = ((0) < (qi) ? (0) : (qi));
            ec_enc_bit_logp(enc, -qi, 1);
         }
         else
            qi = -1;
         error[i+c*m->nbEBands] = (f) - (qi);
         badness += abs(qi0-qi);
         q = (opus_val32)((qi));

         tmp = (((opus_val32)(coef)*(opus_val32)(oldE))) + prev[c] + (q);



         oldEBands[i+c*m->nbEBands] = (tmp);
         prev[c] = prev[c] + (q) - ((opus_val32)(beta)*(opus_val32)((q)));
      } while (++c < C);
   }
   return lfe ? 0 : badness;
}

void quant_coarse_energy(const OpusCustomMode *m, int start, int end, int effEnd,
      const opus_val16 *eBands, opus_val16 *oldEBands, opus_uint32 budget,
      opus_val16 *error, ec_enc *enc, int C, int LM, int nbAvailableBytes,
      int force_intra, opus_val32 *delayedIntra, int two_pass, int loss_rate, int lfe)
{
   int intra;
   opus_val16 max_decay;
   ;
   ;
   ec_enc enc_start_state;
   opus_uint32 tell;
   int badness1=0;
   opus_int32 intra_bias;
   opus_val32 new_distortion;
   ;

   intra = force_intra || (!two_pass && *delayedIntra>2*C*(end-start) && nbAvailableBytes > (end-start)*C);
   intra_bias = (opus_int32)((budget**delayedIntra*loss_rate)/(C*512));
   new_distortion = loss_distortion(eBands, oldEBands, start, effEnd, m->nbEBands, C);

   tell = ec_tell(enc);
   if (tell+3 > budget)
      two_pass = intra = 0;

   max_decay = (16.f);
   if (end-start>10)
   {



      max_decay = ((max_decay) < (.125f*nbAvailableBytes) ? (max_decay) : (.125f*nbAvailableBytes));

   }
   if (lfe)
      max_decay = (3.f);
   enc_start_state = *enc;

   opus_val16 *oldEBands_intra = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(C*m->nbEBands));
   opus_val16 *error_intra = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(C*m->nbEBands));
   (memcpy((oldEBands_intra), (oldEBands), (C*m->nbEBands)*sizeof(*(oldEBands_intra)) + 0*((oldEBands_intra)-(oldEBands)) ));

   if (two_pass || intra)
   {
      badness1 = quant_coarse_energy_impl(m, start, end, eBands, oldEBands_intra, budget,
            tell, e_prob_model[LM][1], error_intra, enc, C, LM, 1, max_decay, lfe);
   }

   if (!intra)
   {
      unsigned char *intra_buf;
      ec_enc enc_intra_state;
      opus_int32 tell_intra;
      opus_uint32 nstart_bytes;
      opus_uint32 nintra_bytes;
      opus_uint32 save_bytes;
      int badness2;
      ;

      tell_intra = ec_tell_frac(enc);

      enc_intra_state = *enc;

      nstart_bytes = ec_range_bytes(&enc_start_state);
      nintra_bytes = ec_range_bytes(&enc_intra_state);
      intra_buf = ec_get_buffer(&enc_intra_state) + nstart_bytes;
      save_bytes = nintra_bytes-nstart_bytes;
      if (save_bytes == 0)
         save_bytes = 1;
      unsigned char *intra_bits = (unsigned char*)DR_OPUS_ALLOCA(sizeof(unsigned char)*(save_bytes));

      (memcpy((intra_bits), (intra_buf), (nintra_bytes - nstart_bytes)*sizeof(*(intra_bits)) + 0*((intra_bits)-(intra_buf)) ));

      *enc = enc_start_state;

      badness2 = quant_coarse_energy_impl(m, start, end, eBands, oldEBands, budget,
            tell, e_prob_model[LM][intra], error, enc, C, LM, 0, max_decay, lfe);

      if (two_pass && (badness1 < badness2 || (badness1 == badness2 && ((opus_int32)ec_tell_frac(enc))+intra_bias > tell_intra)))
      {
         *enc = enc_intra_state;

         (memcpy((intra_buf), (intra_bits), (nintra_bytes - nstart_bytes)*sizeof(*(intra_buf)) + 0*((intra_buf)-(intra_bits)) ));
         (memcpy((oldEBands), (oldEBands_intra), (C*m->nbEBands)*sizeof(*(oldEBands)) + 0*((oldEBands)-(oldEBands_intra)) ));
         (memcpy((error), (error_intra), (C*m->nbEBands)*sizeof(*(error)) + 0*((error)-(error_intra)) ));
         intra = 1;
      }
   } else {
      (memcpy((oldEBands), (oldEBands_intra), (C*m->nbEBands)*sizeof(*(oldEBands)) + 0*((oldEBands)-(oldEBands_intra)) ));
      (memcpy((error), (error_intra), (C*m->nbEBands)*sizeof(*(error)) + 0*((error)-(error_intra)) ));
   }

   if (intra)
      *delayedIntra = new_distortion;
   else
      *delayedIntra = ((((((pred_coef[LM])*(pred_coef[LM])))*(*delayedIntra)))+(new_distortion))
                           ;

   ;
}

void quant_fine_energy(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, opus_val16 *error, int *fine_quant, ec_enc *enc, int C)
{
   int i, c;


   for (i=start;i<end;i++)
   {
      opus_int16 frac = 1<<fine_quant[i];
      if (fine_quant[i] <= 0)
         continue;
      c=0;
      do {
         int q2;
         opus_val16 offset;




         q2 = (int)floor((error[i+c*m->nbEBands]+.5f)*frac);

         if (q2 > frac-1)
            q2 = frac-1;
         if (q2<0)
            q2 = 0;
         ec_enc_bits(enc, q2, fine_quant[i]);



         offset = (q2+.5f)*(1<<(14-fine_quant[i]))*(1.f/16384) - .5f;

         oldEBands[i+c*m->nbEBands] += offset;
         error[i+c*m->nbEBands] -= offset;

      } while (++c < C);
   }
}

void quant_energy_finalise(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, opus_val16 *error, int *fine_quant, int *fine_priority, int bits_left, ec_enc *enc, int C)
{
   int i, prio, c;


   for (prio=0;prio<2;prio++)
   {
      for (i=start;i<end && bits_left>=C ;i++)
      {
         if (fine_quant[i] >= 8 || fine_priority[i]!=prio)
            continue;
         c=0;
         do {
            int q2;
            opus_val16 offset;
            q2 = error[i+c*m->nbEBands]<0 ? 0 : 1;
            ec_enc_bits(enc, q2, 1);



            offset = (q2-.5f)*(1<<(14-fine_quant[i]-1))*(1.f/16384);

            oldEBands[i+c*m->nbEBands] += offset;
            error[i+c*m->nbEBands] -= offset;
            bits_left--;
         } while (++c < C);
      }
   }
}

void unquant_coarse_energy(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, int intra, ec_dec *dec, int C, int LM)
{
   const unsigned char *prob_model = e_prob_model[LM][intra];
   int i, c;
   opus_val32 prev[2] = {0, 0};
   opus_val16 coef;
   opus_val16 beta;
   opus_int32 budget;
   opus_int32 tell;

   if (intra)
   {
      coef = 0;
      beta = beta_intra;
   } else {
      beta = beta_coef[LM];
      coef = pred_coef[LM];
   }

   budget = dec->storage*8;


   for (i=start;i<end;i++)
   {
      c=0;
      do {
         int qi;
         opus_val32 q;
         opus_val32 tmp;



         ;
         tell = ec_tell(dec);
         if(budget-tell>=15)
         {
            int pi;
            pi = 2*((i) < (20) ? (i) : (20));
            qi = ec_laplace_decode(dec,
                  prob_model[pi]<<7, prob_model[pi+1]<<6);
         }
         else if(budget-tell>=2)
         {
            qi = ec_dec_icdf(dec, small_energy_icdf, 2);
            qi = (qi>>1)^-(qi&1);
         }
         else if(budget-tell>=1)
         {
            qi = -ec_dec_bit_logp(dec, 1);
         }
         else
            qi = -1;
         q = (opus_val32)((qi));

         oldEBands[i+c*m->nbEBands] = ((-(9.f)) > (oldEBands[i+c*m->nbEBands]) ? (-(9.f)) : (oldEBands[i+c*m->nbEBands]));
         tmp = (((opus_val32)(coef)*(opus_val32)(oldEBands[i+c*m->nbEBands]))) + prev[c] + (q);



         oldEBands[i+c*m->nbEBands] = (tmp);
         prev[c] = prev[c] + (q) - ((opus_val32)(beta)*(opus_val32)((q)));
      } while (++c < C);
   }
}

void unquant_fine_energy(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, int *fine_quant, ec_dec *dec, int C)
{
   int i, c;

   for (i=start;i<end;i++)
   {
      if (fine_quant[i] <= 0)
         continue;
      c=0;
      do {
         int q2;
         opus_val16 offset;
         q2 = ec_dec_bits(dec, fine_quant[i]);



         offset = (q2+.5f)*(1<<(14-fine_quant[i]))*(1.f/16384) - .5f;

         oldEBands[i+c*m->nbEBands] += offset;
      } while (++c < C);
   }
}

void unquant_energy_finalise(const OpusCustomMode *m, int start, int end, opus_val16 *oldEBands, int *fine_quant, int *fine_priority, int bits_left, ec_dec *dec, int C)
{
   int i, prio, c;


   for (prio=0;prio<2;prio++)
   {
      for (i=start;i<end && bits_left>=C ;i++)
      {
         if (fine_quant[i] >= 8 || fine_priority[i]!=prio)
            continue;
         c=0;
         do {
            int q2;
            opus_val16 offset;
            q2 = ec_dec_bits(dec, 1);



            offset = (q2-.5f)*(1<<(14-fine_quant[i]-1))*(1.f/16384);

            oldEBands[i+c*m->nbEBands] += offset;
            bits_left--;
         } while (++c < C);
      }
   }
}

void amp2Log2(const OpusCustomMode *m, int effEnd, int end,
      celt_ener *bandE, opus_val16 *bandLogE, int C)
{
   int c, i;
   c=0;
   do {
      for (i=0;i<effEnd;i++)
      {
         bandLogE[i+c*m->nbEBands] =
               ((float)(1.442695040888963387*log(bandE[i+c*m->nbEBands])))
               - ((opus_val16)eMeans[i]);




      }
      for (i=effEnd;i<end;i++)
         bandLogE[c*m->nbEBands+i] = -(14.f);
   } while (++c < C);
}


static const unsigned char LOG2_FRAC_TABLE[24]={
   0,
   8,13,
  16,19,21,23,
  24,26,27,28,29,30,31,32,
  32,33,34,34,35,36,36,37,37
};





static int fits_in32(int _n, int _k)
{
   static const opus_int16 maxN[15] = {
      32767, 32767, 32767, 1476, 283, 109, 60, 40,
       29, 24, 20, 18, 16, 14, 13};
   static const opus_int16 maxK[15] = {
      32767, 32767, 32767, 32767, 1172, 238, 95, 53,
       36, 27, 22, 18, 16, 15, 13};
   if (_n>=14)
   {
      if (_k>=14)
         return 0;
      else
         return _n <= maxN[_k];
   } else {
      return _k <= maxK[_n];
   }
}

void compute_pulse_cache(OpusCustomMode *m, int LM)
{
   int C;
   int i;
   int j;
   int curr=0;
   int nbEntries=0;
   int entryN[100], entryK[100], entryI[100];
   const opus_int16 *eBands = m->eBands;
   PulseCache *cache = &m->cache;
   opus_int16 *cindex;
   unsigned char *bits;
   unsigned char *cap;

   cindex = (opus_int16 *)opus_alloc(sizeof(cache->index[0])*m->nbEBands*(LM+2));
   cache->index = cindex;


   for (i=0;i<=LM+1;i++)
   {
      for (j=0;j<m->nbEBands;j++)
      {
         int k;
         int N = (eBands[j+1]-eBands[j])<<i>>1;
         cindex[i*m->nbEBands+j] = -1;

         for (k=0;k<=i;k++)
         {
            int n;
            for (n=0;n<m->nbEBands && (k!=i || n<j);n++)
            {
               if (N == (eBands[n+1]-eBands[n])<<k>>1)
               {
                  cindex[i*m->nbEBands+j] = cindex[k*m->nbEBands+n];
                  break;
               }
            }
         }
         if (cache->index[i*m->nbEBands+j] == -1 && N!=0)
         {
            int K;
            entryN[nbEntries] = N;
            K = 0;
            while (fits_in32(N,get_pulses(K+1)) && K<40)
               K++;
            entryK[nbEntries] = K;
            cindex[i*m->nbEBands+j] = curr;
            entryI[nbEntries] = curr;

            curr += K+1;
            nbEntries++;
         }
      }
   }
   bits = (unsigned char *)opus_alloc(sizeof(unsigned char)*curr);
   cache->bits = bits;
   cache->size = curr;

   for (i=0;i<nbEntries;i++)
   {
      unsigned char *ptr = bits+entryI[i];
      opus_int16 tmp[128 +1];
      get_required_bits(tmp, entryN[i], get_pulses(entryK[i]), 3);
      for (j=1;j<=entryK[i];j++)
         ptr[j] = tmp[get_pulses(j)]-1;
      ptr[0] = entryK[i];
   }



   cache->caps = cap = (unsigned char *)opus_alloc(sizeof(cache->caps[0])*(LM+1)*2*m->nbEBands);
   for (i=0;i<=LM;i++)
   {
      for (C=1;C<=2;C++)
      {
         for (j=0;j<m->nbEBands;j++)
         {
            int N0;
            int max_bits;
            N0 = m->eBands[j+1]-m->eBands[j];

            if (N0<<i == 1)
               max_bits = C*(1+8)<<3;
            else
            {
               const unsigned char *pcache;
               opus_int32 num;
               opus_int32 den;
               int LM0;
               int N;
               int offset;
               int ndof;
               int qb;
               int k;
               LM0 = 0;


               if (N0 > 2)
               {
                  N0>>=1;
                  LM0--;
               }

               else if (N0 <= 1)
               {
                  LM0=((i) < (1) ? (i) : (1));
                  N0<<=LM0;
               }


               pcache = bits + cindex[(LM0+1)*m->nbEBands+j];
               max_bits = pcache[pcache[0]]+1;

               N = N0;
               for(k=0;k<i-LM0;k++){
                  max_bits <<= 1;



                  offset = ((m->logN[j]+((LM0+k)<<3))>>1)-4;




                  num=459*(opus_int32)((2*N-1)*offset+max_bits);
                  den=((opus_int32)(2*N-1)<<9)-459;
                  qb = (((num+(den>>1))/den) < (57) ? ((num+(den>>1))/den) : (57));
                  ;
                  max_bits += qb;
                  N <<= 1;
               }

               if (C==2)
               {
                  max_bits <<= 1;
                  offset = ((m->logN[j]+(i<<3))>>1)-(N==2?16:4);
                  ndof = 2*N-1-(N==2);


                  num = (N==2?512:487)*(opus_int32)(max_bits+ndof*offset);
                  den = ((opus_int32)ndof<<9)-(N==2?512:487);
                  qb = (((num+(den>>1))/den) < ((N==2?64:61)) ? ((num+(den>>1))/den) : ((N==2?64:61)));
                  ;
                  max_bits += qb;
               }


               ndof = C*N + ((C==2 && N>2) ? 1 : 0);


               offset = ((m->logN[j] + (i<<3))>>1)-21;

               if (N==2)
                  offset += 1<<3>>2;


               num = max_bits+ndof*offset;
               den = (ndof-1)<<3;
               qb = (((num+(den>>1))/den) < (8) ? ((num+(den>>1))/den) : (8));
               ;
               max_bits += C*qb<<3;
            }
            max_bits = (4*max_bits/(C*((m->eBands[j+1]-m->eBands[j])<<i)))-64;
            ;
            ;
            *cap++ = (unsigned char)max_bits;
         }
      }
   }
}





static inline int interp_bits2pulses(const OpusCustomMode *m, int start, int end, int skip_start,
      const int *bits1, const int *bits2, const int *thresh, const int *cap, opus_int32 total, opus_int32 *_balance,
      int skip_rsv, int *intensity, int intensity_rsv, int *dual_stereo, int dual_stereo_rsv, int *bits,
      int *ebits, int *fine_priority, int C, int LM, ec_ctx *ec, int encode, int prev, int signalBandwidth)
{
   opus_int32 psum;
   int lo, hi;
   int i, j;
   int logM;
   int stereo;
   int codedBands=-1;
   int alloc_floor;
   opus_int32 left, percoeff;
   int done;
   opus_int32 balance;
   ;

   alloc_floor = C<<3;
   stereo = C>1;

   logM = LM<<3;
   lo = 0;
   hi = 1<<6;
   for (i=0;i<6;i++)
   {
      int mid = (lo+hi)>>1;
      psum = 0;
      done = 0;
      for (j=end;j-->start;)
      {
         int tmp = bits1[j] + (mid*(opus_int32)bits2[j]>>6);
         if (tmp >= thresh[j] || done)
         {
            done = 1;

            psum += ((tmp) < (cap[j]) ? (tmp) : (cap[j]));
         } else {
            if (tmp >= alloc_floor)
               psum += alloc_floor;
         }
      }
      if (psum > total)
         hi = mid;
      else
         lo = mid;
   }
   psum = 0;

   done = 0;
   for (j=end;j-->start;)
   {
      int tmp = bits1[j] + ((opus_int32)lo*bits2[j]>>6);
      if (tmp < thresh[j] && !done)
      {
         if (tmp >= alloc_floor)
            tmp = alloc_floor;
         else
            tmp = 0;
      } else
         done = 1;

      tmp = ((tmp) < (cap[j]) ? (tmp) : (cap[j]));
      bits[j] = tmp;
      psum += tmp;
   }


   for (codedBands=end;;codedBands--)
   {
      int band_width;
      int band_bits;
      int rem;
      j = codedBands-1;






      if (j<=skip_start)
      {

         total += skip_rsv;
         break;
      }


      left = total-psum;
      percoeff = celt_udiv(left, m->eBands[codedBands]-m->eBands[start]);
      left -= (m->eBands[codedBands]-m->eBands[start])*percoeff;
      rem = ((left-(m->eBands[j]-m->eBands[start])) > (0) ? (left-(m->eBands[j]-m->eBands[start])) : (0));
      band_width = m->eBands[codedBands]-m->eBands[j];
      band_bits = (int)(bits[j] + percoeff*band_width + rem);



      if (band_bits >= ((thresh[j]) > (alloc_floor+(1<<3)) ? (thresh[j]) : (alloc_floor+(1<<3))))
      {
         if (encode)
         {



            int depth_threshold;


            if (codedBands > 17)
               depth_threshold = j<prev ? 7 : 9;
            else
               depth_threshold = 0;





            if (codedBands<=start+2 || (band_bits > (depth_threshold*band_width<<LM<<3)>>4 && j<=signalBandwidth))

            {
               ec_enc_bit_logp(ec, 1, 1);
               break;
            }
            ec_enc_bit_logp(ec, 0, 1);
         } else if (ec_dec_bit_logp(ec, 1)) {
            break;
         }

         psum += 1<<3;
         band_bits -= 1<<3;
      }

      psum -= bits[j]+intensity_rsv;
      if (intensity_rsv > 0)
         intensity_rsv = LOG2_FRAC_TABLE[j-start];
      psum += intensity_rsv;
      if (band_bits >= alloc_floor)
      {

         psum += alloc_floor;
         bits[j] = alloc_floor;
      } else {

         bits[j] = 0;
      }
   }

   ;

   if (intensity_rsv > 0)
   {
      if (encode)
      {
         *intensity = ((*intensity) < (codedBands) ? (*intensity) : (codedBands));
         ec_enc_uint(ec, *intensity-start, codedBands+1-start);
      }
      else
         *intensity = start+ec_dec_uint(ec, codedBands+1-start);
   }
   else
      *intensity = 0;
   if (*intensity <= start)
   {
      total += dual_stereo_rsv;
      dual_stereo_rsv = 0;
   }
   if (dual_stereo_rsv > 0)
   {
      if (encode)
         ec_enc_bit_logp(ec, *dual_stereo, 1);
      else
         *dual_stereo = ec_dec_bit_logp(ec, 1);
   }
   else
      *dual_stereo = 0;


   left = total-psum;
   percoeff = celt_udiv(left, m->eBands[codedBands]-m->eBands[start]);
   left -= (m->eBands[codedBands]-m->eBands[start])*percoeff;
   for (j=start;j<codedBands;j++)
      bits[j] += ((int)percoeff*(m->eBands[j+1]-m->eBands[j]));
   for (j=start;j<codedBands;j++)
   {
      int tmp = (int)((left) < (m->eBands[j+1]-m->eBands[j]) ? (left) : (m->eBands[j+1]-m->eBands[j]));
      bits[j] += tmp;
      left -= tmp;
   }


   balance = 0;
   for (j=start;j<codedBands;j++)
   {
      int N0, N, den;
      int offset;
      int NClogN;
      opus_int32 excess, bit;

      ;
      N0 = m->eBands[j+1]-m->eBands[j];
      N=N0<<LM;
      bit = (opus_int32)bits[j]+balance;

      if (N>1)
      {
         excess = ((bit-cap[j]) > (0) ? (bit-cap[j]) : (0));
         bits[j] = bit-excess;


         den=(C*N+ ((C==2 && N>2 && !*dual_stereo && j<*intensity) ? 1 : 0));

         NClogN = den*(m->logN[j] + logM);



         offset = (NClogN>>1)-den*21;


         if (N==2)
            offset += den<<3>>2;



         if (bits[j] + offset < den*2<<3)
            offset += NClogN>>2;
         else if (bits[j] + offset < den*3<<3)
            offset += NClogN>>3;


         ebits[j] = ((0) > ((bits[j] + offset + (den<<(3 -1)))) ? (0) : ((bits[j] + offset + (den<<(3 -1)))));
         ebits[j] = celt_udiv(ebits[j], den)>>3;


         if (C*ebits[j] > (bits[j]>>3))
            ebits[j] = bits[j] >> stereo >> 3;


         ebits[j] = ((ebits[j]) < (8) ? (ebits[j]) : (8));



         fine_priority[j] = ebits[j]*(den<<3) >= bits[j]+offset;


         bits[j] -= C*ebits[j]<<3;

      } else {

         excess = ((0) > (bit-(C<<3)) ? (0) : (bit-(C<<3)));
         bits[j] = bit-excess;
         ebits[j] = 0;
         fine_priority[j] = 1;
      }




      if(excess > 0)
      {
         int extra_fine;
         int extra_bits;
         extra_fine = ((excess>>(stereo+3)) < (8 -ebits[j]) ? (excess>>(stereo+3)) : (8 -ebits[j]));
         ebits[j] += extra_fine;
         extra_bits = extra_fine*C<<3;
         fine_priority[j] = extra_bits >= excess-balance;
         excess -= extra_bits;
      }
      balance = excess;

      ;
      ;
   }


   *_balance = balance;


   for (;j<end;j++)
   {
      ebits[j] = bits[j] >> stereo >> 3;
      ;
      bits[j] = 0;
      fine_priority[j] = ebits[j]<1;
   }
   ;
   return codedBands;
}

int clt_compute_allocation(const OpusCustomMode *m, int start, int end, const int *offsets, const int *cap, int alloc_trim, int *intensity, int *dual_stereo,
      opus_int32 total, opus_int32 *balance, int *pulses, int *ebits, int *fine_priority, int C, int LM, ec_ctx *ec, int encode, int prev, int signalBandwidth)
{
   int lo, hi, len, j;
   int codedBands;
   int skip_start;
   int skip_rsv;
   int intensity_rsv;
   int dual_stereo_rsv;
   ;
   ;
   ;
   ;
   ;

   total = ((total) > (0) ? (total) : (0));
   len = m->nbEBands;
   skip_start = start;

   skip_rsv = total >= 1<<3 ? 1<<3 : 0;
   total -= skip_rsv;

   intensity_rsv = dual_stereo_rsv = 0;
   if (C==2)
   {
      intensity_rsv = LOG2_FRAC_TABLE[end-start];
      if (intensity_rsv>total)
         intensity_rsv = 0;
      else
      {
         total -= intensity_rsv;
         dual_stereo_rsv = total>=1<<3 ? 1<<3 : 0;
         total -= dual_stereo_rsv;
      }
   }
   int *bits1 = (int*)DR_OPUS_ALLOCA(sizeof(int)*(len));
   int *bits2 = (int*)DR_OPUS_ALLOCA(sizeof(int)*(len));
   int *thresh = (int*)DR_OPUS_ALLOCA(sizeof(int)*(len));
   int *trim_offset = (int*)DR_OPUS_ALLOCA(sizeof(int)*(len));

   for (j=start;j<end;j++)
   {

      thresh[j] = (((C)<<3) > ((3*(m->eBands[j+1]-m->eBands[j])<<LM<<3)>>4) ? ((C)<<3) : ((3*(m->eBands[j+1]-m->eBands[j])<<LM<<3)>>4));

      trim_offset[j] = C*(m->eBands[j+1]-m->eBands[j])*(alloc_trim-5-LM)*(end-j-1)
            *(1<<(LM+3))>>6;


      if ((m->eBands[j+1]-m->eBands[j])<<LM==1)
         trim_offset[j] -= C<<3;
   }
   lo = 1;
   hi = m->nbAllocVectors - 1;
   do
   {
      int done = 0;
      int psum = 0;
      int mid = (lo+hi) >> 1;
      for (j=end;j-->start;)
      {
         int bitsj;
         int N = m->eBands[j+1]-m->eBands[j];
         bitsj = C*N*m->allocVectors[mid*len+j]<<LM>>2;
         if (bitsj > 0)
            bitsj = ((0) > (bitsj + trim_offset[j]) ? (0) : (bitsj + trim_offset[j]));
         bitsj += offsets[j];
         if (bitsj >= thresh[j] || done)
         {
            done = 1;

            psum += ((bitsj) < (cap[j]) ? (bitsj) : (cap[j]));
         } else {
            if (bitsj >= C<<3)
               psum += C<<3;
         }
      }
      if (psum > total)
         hi = mid - 1;
      else
         lo = mid + 1;

   }
   while (lo <= hi);
   hi = lo--;

   for (j=start;j<end;j++)
   {
      int bits1j, bits2j;
      int N = m->eBands[j+1]-m->eBands[j];
      bits1j = C*N*m->allocVectors[lo*len+j]<<LM>>2;
      bits2j = hi>=m->nbAllocVectors ?
            cap[j] : C*N*m->allocVectors[hi*len+j]<<LM>>2;
      if (bits1j > 0)
         bits1j = ((0) > (bits1j + trim_offset[j]) ? (0) : (bits1j + trim_offset[j]));
      if (bits2j > 0)
         bits2j = ((0) > (bits2j + trim_offset[j]) ? (0) : (bits2j + trim_offset[j]));
      if (lo > 0)
         bits1j += offsets[j];
      bits2j += offsets[j];
      if (offsets[j]>0)
         skip_start = j;
      bits2j = ((0) > (bits2j-bits1j) ? (0) : (bits2j-bits1j));
      bits1[j] = bits1j;
      bits2[j] = bits2j;
   }
   codedBands = interp_bits2pulses(m, start, end, skip_start, bits1, bits2, thresh, cap,
         total, balance, skip_rsv, intensity, intensity_rsv, dual_stereo, dual_stereo_rsv,
         pulses, ebits, fine_priority, C, LM, ec, encode, prev, signalBandwidth);
   ;
   return codedBands;
}



/* Portable float-to-int conversion (replaces SSE version) */
static inline opus_int32 float2int(float x) {
    return (opus_int32)(x + (x >= 0 ? 0.5f : -0.5f));
}

static inline opus_int16 FLOAT2INT16(float x) {
   x = x * 32768.f;
   x = ((x) > (-32768) ? (x) : (-32768));
   x = ((x) < (32767) ? (x) : (32767));
   return (opus_int16)float2int(x);
}

int resampling_factor(opus_int32 rate)
{
   int ret;
   switch (rate)
   {
   case 48000:
      ret = 1;
      break;
   case 24000:
      ret = 2;
      break;
   case 16000:
      ret = 3;
      break;
   case 12000:
      ret = 4;
      break;
   case 8000:
      ret = 6;
      break;
   default:



      ret = 0;
      break;
   }
   return ret;
}
static

void comb_filter_const_c(opus_val32 *y, opus_val32 *x, int T, int N,
      opus_val16 g10, opus_val16 g11, opus_val16 g12)
{
   opus_val32 x0, x1, x2, x3, x4;
   int i;
   x4 = x[-T-2];
   x3 = x[-T-1];
   x2 = x[-T];
   x1 = x[-T+1];
   for (i=0;i<N;i++)
   {
      x0=x[i-T+2];
      y[i] = x[i]
               + ((g10)*(x2))
               + ((g11)*(((x1)+(x3))))
               + ((g12)*(((x0)+(x4))));
      y[i] = (y[i]);
      x4=x3;
      x3=x2;
      x2=x1;
      x1=x0;
   }

}




void comb_filter(opus_val32 *y, opus_val32 *x, int T0, int T1, int N,
      opus_val16 g0, opus_val16 g1, int tapset0, int tapset1,
      const opus_val16 *window, int overlap, int arch)
{
   int i;

   opus_val16 g00, g01, g02, g10, g11, g12;
   opus_val32 x0, x1, x2, x3, x4;
   static const opus_val16 gains[3][3] = {
         {(0.3066406250f), (0.2170410156f), (0.1296386719f)},
         {(0.4638671875f), (0.2680664062f), (0.f)},
         {(0.7998046875f), (0.1000976562f), (0.f)}};

   if (g0==0 && g1==0)
   {

      if (x!=y)
         (memmove((y), (x), (N)*sizeof(*(y)) + 0*((y)-(x)) ));
      return;
   }


   T0 = ((T0) > (15) ? (T0) : (15));
   T1 = ((T1) > (15) ? (T1) : (15));
   g00 = ((g0)*(gains[tapset0][0]));
   g01 = ((g0)*(gains[tapset0][1]));
   g02 = ((g0)*(gains[tapset0][2]));
   g10 = ((g1)*(gains[tapset1][0]));
   g11 = ((g1)*(gains[tapset1][1]));
   g12 = ((g1)*(gains[tapset1][2]));
   x1 = x[-T1+1];
   x2 = x[-T1 ];
   x3 = x[-T1-1];
   x4 = x[-T1-2];

   if (g0==g1 && T0==T1 && tapset0==tapset1)
      overlap=0;
   for (i=0;i<overlap;i++)
   {
      opus_val16 f;
      x0=x[i-T1+2];
      f = ((window[i])*(window[i]));
      y[i] = x[i]
               + (((((1.0f -f))*(g00)))*(x[i-T0]))
               + (((((1.0f -f))*(g01)))*(((x[i-T0+1])+(x[i-T0-1]))))
               + (((((1.0f -f))*(g02)))*(((x[i-T0+2])+(x[i-T0-2]))))
               + ((((f)*(g10)))*(x2))
               + ((((f)*(g11)))*(((x1)+(x3))))
               + ((((f)*(g12)))*(((x0)+(x4))));
      y[i] = (y[i]);
      x4=x3;
      x3=x2;
      x2=x1;
      x1=x0;

   }
   if (g1==0)
   {

      if (x!=y)
         (memmove((y+overlap), (x+overlap), (N-overlap)*sizeof(*(y+overlap)) + 0*((y+overlap)-(x+overlap)) ));
      return;
   }


   ((void)(arch),comb_filter_const_c(y+i, x+i, T1, N-i, g10, g11, g12));
}






const signed char tf_select_table[4][8] = {

      {0, -1, 0, -1, 0,-1, 0,-1},
      {0, -1, 0, -2, 1, 0, 1,-1},
      {0, -2, 0, -3, 2, 0, 1,-1},
      {0, -2, 0, -3, 3, 0, 1,-1},
};


void init_caps(const OpusCustomMode *m,int *cap,int LM,int C)
{
   int i;
   for (i=0;i<m->nbEBands;i++)
   {
      int N;
      N=(m->eBands[i+1]-m->eBands[i])<<LM;
      cap[i] = (m->cache.caps[m->nbEBands*(2*LM+C-1)+i]+64)*C*N>>2;
   }
}



const char *opus_strerror(int error)
{
   static const char * const error_strings[8] = {
      "success",
      "invalid argument",
      "buffer too small",
      "internal error",
      "corrupted stream",
      "request not implemented",
      "invalid state",
      "memory allocation failed"
   };
   if (error > 0 || error < -7)
      return "unknown error";
   else
      return error_strings[-error];
}

const char *opus_get_version_string(void)
{
    return "libopus " "unknown"
          ;
}
struct OpusCustomDecoder {
   const OpusCustomMode *mode;
   int overlap;
   int channels;
   int stream_channels;

   int downsample;
   int start, end;
   int signalling;
   int disable_inv;
   int arch;




   opus_uint32 rng;
   int error;
   int last_pitch_index;
   int loss_duration;
   int skip_plc;
   int postfilter_period;
   int postfilter_period_old;
   opus_val16 postfilter_gain;
   opus_val16 postfilter_gain_old;
   int postfilter_tapset;
   int postfilter_tapset_old;

   celt_sig preemph_memD[2];

   celt_sig _decode_mem[1];





};
/* Forward declarations */
int opus_custom_decoder_get_size(const OpusCustomMode *mode, int channels);
int opus_custom_decoder_init(OpusCustomDecoder *st, const OpusCustomMode *mode, int channels);

int celt_decoder_get_size(int channels)
{
   const OpusCustomMode *mode = opus_custom_mode_create(48000, 960, 
                                                             ((void *)0)
                                                                 );
   return opus_custom_decoder_get_size(mode, channels);
}

 int opus_custom_decoder_get_size(const OpusCustomMode *mode, int channels)
{
   int size = sizeof(struct OpusCustomDecoder)
            + (channels*(2048 +mode->overlap)-1)*sizeof(celt_sig)
            + channels*24*sizeof(opus_val16)
            + 4*2*mode->nbEBands*sizeof(opus_val16);
   return size;
}


OpusCustomDecoder *opus_custom_decoder_create(const OpusCustomMode *mode, int channels, int *error)
{
   int ret;
   OpusCustomDecoder *st = (OpusCustomDecoder *)opus_alloc(opus_custom_decoder_get_size(mode, channels));
   ret = opus_custom_decoder_init(st, mode, channels);
   if (ret != 0)
   {
      opus_custom_decoder_destroy(st);
      st = 
          ((void *)0)
              ;
   }
   if (error)
      *error = ret;
   return st;
}


int celt_decoder_init(OpusCustomDecoder *st, opus_int32 sampling_rate, int channels)
{
   int ret;
   ret = opus_custom_decoder_init(st, opus_custom_mode_create(48000, 960, 
                                                                         ((void *)0)
                                                                             ), channels);
   if (ret != 0)
      return ret;
   st->downsample = resampling_factor(sampling_rate);
   if (st->downsample==0)
      return -1;
   else
      return 0;
}

 int opus_custom_decoder_init(OpusCustomDecoder *st, const OpusCustomMode *mode, int channels)
{
   if (channels < 0 || channels > 2)
      return -1;

   if (st==
          ((void *)0)
              )
      return -7;

   (memset(((char*)st), 0, (opus_custom_decoder_get_size(mode, channels))*sizeof(*((char*)st))));

   st->mode = mode;
   st->overlap = mode->overlap;
   st->stream_channels = st->channels = channels;

   st->downsample = 1;
   st->start = 0;
   st->end = st->mode->effEBands;
   st->signalling = 1;

   st->disable_inv = channels == 1;



   st->arch = opus_select_arch();

   opus_custom_decoder_ctl(st, 4028);

   return 0;
}


void opus_custom_decoder_destroy(OpusCustomDecoder *st)
{
   opus_free(st);
}
static

void deemphasis(celt_sig *in[], opus_val16 *pcm, int N, int C, int downsample, const opus_val16 *coef,
      celt_sig *mem, int accum)
{
   int c;
   int Nd;
   int apply_downsampling=0;
   opus_val16 coef0;
   ;
   ;
   (void)accum;
   ;

   celt_sig *scratch = (celt_sig*)DR_OPUS_ALLOCA(sizeof(celt_sig)*(N));
   coef0 = coef[0];
   Nd = N/downsample;
   c=0; do {
      int j;
      celt_sig * restrict x;
      opus_val16 * restrict y;
      celt_sig m = mem[c];
      x =in[c];
      y = pcm+c;

      if (coef[1] != 0)
      {
         opus_val16 coef1 = coef[1];
         opus_val16 coef3 = coef[3];
         for (j=0;j<N;j++)
         {
            celt_sig tmp = x[j] + m + 1e-30f;
            m = ((coef0)*(tmp))
                          - ((coef1)*(x[j]));
            tmp = (((coef3)*(tmp)));
            scratch[j] = tmp;
         }
         apply_downsampling=1;
      } else

      if (downsample>1)
      {

         for (j=0;j<N;j++)
         {
            celt_sig tmp = x[j] + 1e-30f + m;
            m = ((coef0)*(tmp));
            scratch[j] = tmp;
         }
         apply_downsampling=1;
      } else {
         {
            for (j=0;j<N;j++)
            {
               celt_sig tmp = x[j] + 1e-30f + m;
               m = ((coef0)*(tmp));
               y[j*C] = (((tmp))*(1/32768.f));
            }
         }
      }
      mem[c] = m;

      if (apply_downsampling)
      {
         {
            for (j=0;j<Nd;j++)
               y[j*C] = (((scratch[j*downsample]))*(1/32768.f));
         }
      }
   } while (++c<C);
   ;
}


static

void celt_synthesis(const OpusCustomMode *mode, celt_norm *X, celt_sig * out_syn[],
                    opus_val16 *oldBandE, int start, int effEnd, int C, int CC,
                    int isTransient, int LM, int downsample,
                    int silence, int arch)
{
   int c, i;
   int M;
   int b;
   int B;
   int N, NB;
   int shift;
   int nbEBands;
   int overlap;
   ;
   ;

   overlap = mode->overlap;
   nbEBands = mode->nbEBands;
   N = mode->shortMdctSize<<LM;
   celt_sig *freq = (celt_sig*)DR_OPUS_ALLOCA(sizeof(celt_sig)*(N));
   M = 1<<LM;

   if (isTransient)
   {
      B = M;
      NB = mode->shortMdctSize;
      shift = mode->maxLM;
   } else {
      B = 1;
      NB = mode->shortMdctSize<<LM;
      shift = mode->maxLM-LM;
   }

   if (CC==2&&C==1)
   {

      celt_sig *freq2;
      denormalise_bands(mode, X, freq, oldBandE, start, effEnd, M,
            downsample, silence);

      freq2 = out_syn[1]+overlap/2;
      (memcpy((freq2), (freq), (N)*sizeof(*(freq2)) + 0*((freq2)-(freq)) ));
      for (b=0;b<B;b++)
         clt_mdct_backward_c(&mode->mdct, &freq2[b], out_syn[0]+NB*b, mode->window, overlap, shift, B, arch);
      for (b=0;b<B;b++)
         clt_mdct_backward_c(&mode->mdct, &freq[b], out_syn[1]+NB*b, mode->window, overlap, shift, B, arch);
   } else if (CC==1&&C==2)
   {

      celt_sig *freq2;
      freq2 = out_syn[0]+overlap/2;
      denormalise_bands(mode, X, freq, oldBandE, start, effEnd, M,
            downsample, silence);

      denormalise_bands(mode, X+N, freq2, oldBandE+nbEBands, start, effEnd, M,
            downsample, silence);
      for (i=0;i<N;i++)
         freq[i] = (((.5f*(freq[i])))+((.5f*(freq2[i]))));
      for (b=0;b<B;b++)
         clt_mdct_backward_c(&mode->mdct, &freq[b], out_syn[0]+NB*b, mode->window, overlap, shift, B, arch);
   } else {

      c=0; do {
         denormalise_bands(mode, X+c*N, freq, oldBandE+c*nbEBands, start, effEnd, M,
               downsample, silence);
         for (b=0;b<B;b++)
            clt_mdct_backward_c(&mode->mdct, &freq[b], out_syn[c]+NB*b, mode->window, overlap, shift, B, arch);
      } while (++c<CC);
   }


   c=0; do {
      for (i=0;i<N;i++)
         out_syn[c][i] = (out_syn[c][i]);
   } while (++c<CC);
   ;
}

static void tf_decode(int start, int end, int isTransient, int *tf_res, int LM, ec_dec *dec)
{
   int i, curr, tf_select;
   int tf_select_rsv;
   int tf_changed;
   int logp;
   opus_uint32 budget;
   opus_uint32 tell;

   budget = dec->storage*8;
   tell = ec_tell(dec);
   logp = isTransient ? 2 : 4;
   tf_select_rsv = LM>0 && tell+logp+1<=budget;
   budget -= tf_select_rsv;
   tf_changed = curr = 0;
   for (i=start;i<end;i++)
   {
      if (tell+logp<=budget)
      {
         curr ^= ec_dec_bit_logp(dec, logp);
         tell = ec_tell(dec);
         tf_changed |= curr;
      }
      tf_res[i] = curr;
      logp = isTransient ? 4 : 5;
   }
   tf_select = 0;
   if (tf_select_rsv &&
     tf_select_table[LM][4*isTransient+0+tf_changed] !=
     tf_select_table[LM][4*isTransient+2+tf_changed])
   {
      tf_select = ec_dec_bit_logp(dec, 1);
   }
   for (i=start;i<end;i++)
   {
      tf_res[i] = tf_select_table[LM][4*isTransient+2*tf_select+tf_res[i]];
   }
}

static int celt_plc_pitch_search(celt_sig *decode_mem[2], int C, int arch)
{
   int pitch_index;
   ;
   ;
   opus_val16 lp_pitch_buf[2048>>1];
   pitch_downsample(decode_mem, lp_pitch_buf,
         2048, C, arch);
   pitch_search(lp_pitch_buf+((720)>>1), lp_pitch_buf,
         2048 -(720),
         (720)-(100), &pitch_index, arch);
   pitch_index = (720)-pitch_index;
   ;
   return pitch_index;
}

static void celt_decode_lost(OpusCustomDecoder * restrict st, int N, int LM)
{
   int c;
   int i;
   const int C = st->channels;
   celt_sig *decode_mem[2];
   celt_sig *out_syn[2];
   opus_val16 *lpc;
   opus_val16 *oldBandE, *oldLogE, *oldLogE2, *backgroundLogE;
   const OpusCustomMode *mode;
   int nbEBands;
   int overlap;
   int start;
   int loss_duration;
   int noise_based;
   const opus_int16 *eBands;
   ;

   mode = st->mode;
   nbEBands = mode->nbEBands;
   overlap = mode->overlap;
   eBands = mode->eBands;

   c=0; do {
      decode_mem[c] = st->_decode_mem + c*(2048 +overlap);
      out_syn[c] = decode_mem[c]+2048 -N;
   } while (++c<C);
   lpc = (opus_val16*)(st->_decode_mem+(2048 +overlap)*C);
   oldBandE = lpc+C*24;
   oldLogE = oldBandE + 2*nbEBands;
   oldLogE2 = oldLogE + 2*nbEBands;
   backgroundLogE = oldLogE2 + 2*nbEBands;

   loss_duration = st->loss_duration;
   start = st->start;
   noise_based = loss_duration >= 40 || start != 0 || st->skip_plc;
   if (noise_based)
   {




      ;

      opus_uint32 seed;
      int end;
      int effEnd;
      opus_val16 decay;
      end = st->end;
      effEnd = ((start) > (((end) < (mode->effEBands) ? (end) : (mode->effEBands))) ? (start) : (((end) < (mode->effEBands) ? (end) : (mode->effEBands))));






      celt_norm *X = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(C*N));

      c=0; do {
         (memmove((decode_mem[c]), (decode_mem[c]+N), (2048 -N+(overlap>>1))*sizeof(*(decode_mem[c])) + 0*((decode_mem[c])-(decode_mem[c]+N)) ))
                                                 ;
      } while (++c<C);


      decay = loss_duration==0 ? (1.5f) : (.5f);
      c=0; do
      {
         for (i=start;i<end;i++)
            oldBandE[c*nbEBands+i] = ((backgroundLogE[c*nbEBands+i]) > (oldBandE[c*nbEBands+i] - decay) ? (backgroundLogE[c*nbEBands+i]) : (oldBandE[c*nbEBands+i] - decay));
      } while (++c<C);
      seed = st->rng;
      for (c=0;c<C;c++)
      {
         for (i=start;i<effEnd;i++)
         {
            int j;
            int boffs;
            int blen;
            boffs = N*c+(eBands[i]<<LM);
            blen = (eBands[i+1]-eBands[i])<<LM;
            for (j=0;j<blen;j++)
            {
               seed = celt_lcg_rand(seed);
               X[boffs+j] = (celt_norm)((opus_int32)seed>>20);
            }
            renormalise_vector(X+boffs, blen, 1.0f, st->arch);
         }
      }
      st->rng = seed;

      celt_synthesis(mode, X, out_syn, oldBandE, start, effEnd, C, C, 0, LM, st->downsample, 0, st->arch);
   } else {
      int exc_length;

      const opus_val16 *window;
      opus_val16 *exc;
      opus_val16 fade = 1.0f;
      int pitch_index;
      ;
      ;
      ;

      if (loss_duration == 0)
      {
         st->last_pitch_index = pitch_index = celt_plc_pitch_search(decode_mem, C, st->arch);
      } else {
         pitch_index = st->last_pitch_index;
         fade = (.8f);
      }



      exc_length = ((2*pitch_index) < (1024) ? (2*pitch_index) : (1024));

      opus_val32 *etmp = (opus_val32*)DR_OPUS_ALLOCA(sizeof(opus_val32)*(overlap));
      opus_val16 _exc[1024 +24];
      opus_val16 *fir_tmp = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(exc_length));
      exc = _exc+24;
      window = mode->window;
      c=0; do {
         opus_val16 decay;
         opus_val16 attenuation;
         opus_val32 S1=0;
         celt_sig *buf;
         int extrapolation_offset;
         int extrapolation_len;
         int j;

         buf = decode_mem[c];
         for (i=0;i<1024 +24;i++)
            exc[i-24] = (buf[2048 -1024 -24 +i]);

         if (loss_duration == 0)
         {
            opus_val32 ac[24 +1];


            _celt_autocorr(exc, ac, window, overlap,
                   24, 1024, st->arch);




            ac[0] *= 1.0001f;


            for (i=1;i<=24;i++)
            {




               ac[i] -= ac[i]*(0.008f*0.008f)*i*i;

            }
            _celt_lpc(lpc+c*24, ac, 24);
         }


         {


            (celt_fir_c(exc+1024 -exc_length, lpc+c*24, fir_tmp, exc_length, 24, st->arch))
                                                           ;
            (memcpy((exc+1024 -exc_length), (fir_tmp), (exc_length)*sizeof(*(exc+1024 -exc_length)) + 0*((exc+1024 -exc_length)-(fir_tmp)) ));
         }




         {
            opus_val32 E1=1, E2=1;
            int decay_length;



            decay_length = exc_length>>1;
            for (i=0;i<decay_length;i++)
            {
               opus_val16 e;
               e = exc[1024 -decay_length+i];
               E1 += (((opus_val32)(e)*(opus_val32)(e)));
               e = exc[1024 -2*decay_length+i];
               E2 += (((opus_val32)(e)*(opus_val32)(e)));
            }
            E1 = ((E1) < (E2) ? (E1) : (E2));
            decay = ((float)sqrt(((float)((E1))/(E2))));
         }




         (memmove((buf), (buf+N), (2048 -N)*sizeof(*(buf)) + 0*((buf)-(buf+N)) ));




         extrapolation_offset = 1024 -pitch_index;


         extrapolation_len = N+overlap;

         attenuation = ((fade)*(decay));
         for (i=j=0;i<extrapolation_len;i++,j++)
         {
            opus_val16 tmp;
            if (j >= pitch_index) {
               j -= pitch_index;
               attenuation = ((attenuation)*(decay));
            }
            buf[2048 -N+i] =
                  ((((attenuation)*(exc[extrapolation_offset+j]))))
                                                                 ;


            tmp = (buf[2048 -1024 -N+extrapolation_offset+j])

                            ;
            S1 += (((opus_val32)(tmp)*(opus_val32)(tmp)));
         }
         {
            opus_val16 lpc_mem[24];


            for (i=0;i<24;i++)
               lpc_mem[i] = (buf[2048 -N-1-i]);


            celt_iir(buf+2048 -N, lpc+c*24,
                  buf+2048 -N, extrapolation_len, 24,
                  lpc_mem, st->arch);




         }




         {
            opus_val32 S2=0;
            for (i=0;i<extrapolation_len;i++)
            {
               opus_val16 tmp = (buf[2048 -N+i]);
               S2 += (((opus_val32)(tmp)*(opus_val32)(tmp)));
            }






            if (!(S1 > 0.2f*S2))

            {
               for (i=0;i<extrapolation_len;i++)
                  buf[2048 -N+i] = 0;
            } else if (S1 < S2)
            {
               opus_val16 ratio = ((float)sqrt(((float)((S1)+1)/(S2+1))));
               for (i=0;i<overlap;i++)
               {
                  opus_val16 tmp_g = 1.0f
                        - ((window[i])*(1.0f -ratio));
                  buf[2048 -N+i] =
                        ((tmp_g)*(buf[2048 -N+i]));
               }
               for (i=overlap;i<extrapolation_len;i++)
               {
                  buf[2048 -N+i] =
                        ((ratio)*(buf[2048 -N+i]));
               }
            }
         }




         comb_filter(etmp, buf+2048,
              st->postfilter_period, st->postfilter_period, overlap,
              -st->postfilter_gain, -st->postfilter_gain,
              st->postfilter_tapset, st->postfilter_tapset, 
                                                           ((void *)0)
                                                               , 0, st->arch);



         for (i=0;i<overlap/2;i++)
         {
            buf[2048 +i] =
               ((window[i])*(etmp[overlap-1-i]))
               + ((window[overlap-i-1])*(etmp[i]));
         }
      } while (++c<C);
   }


   st->loss_duration = ((10000) < (loss_duration+(1<<LM)) ? (10000) : (loss_duration+(1<<LM)));

   ;
}

int celt_decode_with_ec(OpusCustomDecoder * restrict st, const unsigned char *data,
      int len, opus_val16 * restrict pcm, int frame_size, ec_dec *dec, int accum)
{
   int c, i, N;
   int spread_decision;
   opus_int32 bits;
   ec_dec _dec;



   ;

   ;
   ;
   ;
   ;
   ;
   ;
   ;
   celt_sig *decode_mem[2];
   celt_sig *out_syn[2];
   opus_val16 *lpc;
   opus_val16 *oldBandE, *oldLogE, *oldLogE2, *backgroundLogE;

   int shortBlocks;
   int isTransient;
   int intra_ener;
   const int CC = st->channels;
   int LM, M;
   int start;
   int end;
   int effEnd;
   int codedBands;
   int alloc_trim;
   int postfilter_pitch;
   opus_val16 postfilter_gain;
   int intensity=0;
   int dual_stereo=0;
   opus_int32 total_bits;
   opus_int32 balance;
   opus_int32 tell;
   int dynalloc_logp;
   int postfilter_tapset;
   int anti_collapse_rsv;
   int anti_collapse_on=0;
   int silence;
   int C = st->stream_channels;
   const OpusCustomMode *mode;
   int nbEBands;
   int overlap;
   const opus_int16 *eBands;
   opus_val16 max_background_increase;
   ;

   ;
   mode = st->mode;
   nbEBands = mode->nbEBands;
   overlap = mode->overlap;
   eBands = mode->eBands;
   start = st->start;
   end = st->end;
   frame_size *= st->downsample;

   lpc = (opus_val16*)(st->_decode_mem+(2048 +overlap)*CC);
   oldBandE = lpc+CC*24;
   oldLogE = oldBandE + 2*nbEBands;
   oldLogE2 = oldLogE + 2*nbEBands;
   backgroundLogE = oldLogE2 + 2*nbEBands;


   if (st->signalling && data!=
                              ((void *)0)
                                  )
   {
      int data0=data[0];

      if (mode->Fs==48000 && mode->shortMdctSize==120)
      {
         data0 = fromOpus(data0);
         if (data0<0)
            return -4;
      }
      st->end = end = ((1) > (mode->effEBands-2*(data0>>5)) ? (1) : (mode->effEBands-2*(data0>>5)));
      LM = (data0>>3)&0x3;
      C = 1 + ((data0>>2)&0x1);
      data++;
      len--;
      if (LM>mode->maxLM)
         return -4;
      if (frame_size < mode->shortMdctSize<<LM)
         return -2;
      else
         frame_size = mode->shortMdctSize<<LM;
   } else {



      for (LM=0;LM<=mode->maxLM;LM++)
         if (mode->shortMdctSize<<LM==frame_size)
            break;
      if (LM>mode->maxLM)
         return -1;
   }
   M=1<<LM;

   if (len<0 || len>1275 || pcm==
                                ((void *)0)
                                    )
      return -1;

   N = M*mode->shortMdctSize;
   c=0; do {
      decode_mem[c] = st->_decode_mem + c*(2048 +overlap);
      out_syn[c] = decode_mem[c]+2048 -N;
   } while (++c<CC);

   effEnd = end;
   if (effEnd > mode->effEBands)
      effEnd = mode->effEBands;

   if (data == 
              ((void *)0) 
                   || len<=1)
   {
      celt_decode_lost(st, N, LM);
      deemphasis(out_syn, pcm, N, CC, st->downsample, mode->preemph, st->preemph_memD, accum);
      ;
      return frame_size/st->downsample;
   }



   st->skip_plc = st->loss_duration != 0;

   if (dec == 
             ((void *)0)
                 )
   {
      ec_dec_init(&_dec,(unsigned char*)data,len);
      dec = &_dec;
   }

   if (C==1)
   {
      for (i=0;i<nbEBands;i++)
         oldBandE[i]=((oldBandE[i]) > (oldBandE[nbEBands+i]) ? (oldBandE[i]) : (oldBandE[nbEBands+i]));
   }

   total_bits = len*8;
   tell = ec_tell(dec);

   if (tell >= total_bits)
      silence = 1;
   else if (tell==1)
      silence = ec_dec_bit_logp(dec, 15);
   else
      silence = 0;
   if (silence)
   {

      tell = len*8;
      dec->nbits_total+=tell-ec_tell(dec);
   }

   postfilter_gain = 0;
   postfilter_pitch = 0;
   postfilter_tapset = 0;
   if (start==0 && tell+16 <= total_bits)
   {
      if(ec_dec_bit_logp(dec, 1))
      {
         int qg, octave;
         octave = ec_dec_uint(dec, 6);
         postfilter_pitch = (16<<octave)+ec_dec_bits(dec, 4+octave)-1;
         qg = ec_dec_bits(dec, 3);
         if (ec_tell(dec)+2<=total_bits)
            postfilter_tapset = ec_dec_icdf(dec, tapset_icdf, 2);
         postfilter_gain = (.09375f)*(qg+1);
      }
      tell = ec_tell(dec);
   }

   if (LM > 0 && tell+3 <= total_bits)
   {
      isTransient = ec_dec_bit_logp(dec, 3);
      tell = ec_tell(dec);
   }
   else
      isTransient = 0;

   if (isTransient)
      shortBlocks = M;
   else
      shortBlocks = 0;


   intra_ener = tell+3<=total_bits ? ec_dec_bit_logp(dec, 3) : 0;

   unquant_coarse_energy(mode, start, end, oldBandE,
         intra_ener, dec, C, LM);

   int *tf_res = (int*)DR_OPUS_ALLOCA(sizeof(int)*(nbEBands));
   tf_decode(start, end, isTransient, tf_res, LM, dec);

   tell = ec_tell(dec);
   spread_decision = (2);
   if (tell+4 <= total_bits)
      spread_decision = ec_dec_icdf(dec, spread_icdf, 5);

   int *cap = (int*)DR_OPUS_ALLOCA(sizeof(int)*(nbEBands));

   init_caps(mode,cap,LM,C);

   int *offsets = (int*)DR_OPUS_ALLOCA(sizeof(int)*(nbEBands));

   dynalloc_logp = 6;
   total_bits<<=3;
   tell = ec_tell_frac(dec);
   for (i=start;i<end;i++)
   {
      int width, quanta;
      int dynalloc_loop_logp;
      int boost;
      width = C*(eBands[i+1]-eBands[i])<<LM;


      quanta = ((width<<3) < (((6<<3) > (width) ? (6<<3) : (width))) ? (width<<3) : (((6<<3) > (width) ? (6<<3) : (width))));
      dynalloc_loop_logp = dynalloc_logp;
      boost = 0;
      while (tell+(dynalloc_loop_logp<<3) < total_bits && boost < cap[i])
      {
         int flag;
         flag = ec_dec_bit_logp(dec, dynalloc_loop_logp);
         tell = ec_tell_frac(dec);
         if (!flag)
            break;
         boost += quanta;
         total_bits -= quanta;
         dynalloc_loop_logp = 1;
      }
      offsets[i] = boost;

      if (boost>0)
         dynalloc_logp = ((2) > (dynalloc_logp-1) ? (2) : (dynalloc_logp-1));
   }

   int *fine_quant = (int*)DR_OPUS_ALLOCA(sizeof(int)*(nbEBands));
   alloc_trim = tell+(6<<3) <= total_bits ?
         ec_dec_icdf(dec, trim_icdf, 7) : 5;

   bits = (((opus_int32)len*8)<<3) - ec_tell_frac(dec) - 1;
   anti_collapse_rsv = isTransient&&LM>=2&&bits>=((LM+2)<<3) ? (1<<3) : 0;
   bits -= anti_collapse_rsv;

   int *pulses = (int*)DR_OPUS_ALLOCA(sizeof(int)*(nbEBands));
   int *fine_priority = (int*)DR_OPUS_ALLOCA(sizeof(int)*(nbEBands));

   codedBands = clt_compute_allocation(mode, start, end, offsets, cap,
         alloc_trim, &intensity, &dual_stereo, bits, &balance, pulses,
         fine_quant, fine_priority, C, LM, dec, 0, 0, 0);

   unquant_fine_energy(mode, start, end, oldBandE, fine_quant, dec, C);

   c=0; do {
      (memmove((decode_mem[c]), (decode_mem[c]+N), (2048 -N+overlap/2)*sizeof(*(decode_mem[c])) + 0*((decode_mem[c])-(decode_mem[c]+N)) ));
   } while (++c<CC);


   unsigned char *collapse_masks = (unsigned char*)DR_OPUS_ALLOCA(sizeof(unsigned char)*(C*nbEBands));






   celt_norm *X = (celt_norm*)DR_OPUS_ALLOCA(sizeof(celt_norm)*(C*N));


   quant_all_bands(0, mode, start, end, X, C==2 ? X+N : 
                                                       ((void *)0)
                                                           , collapse_masks,
         
        ((void *)0)
            , pulses, shortBlocks, spread_decision, dual_stereo, intensity, tf_res,
         len*(8<<3)-anti_collapse_rsv, balance, dec, LM, codedBands, &st->rng, 0,
         st->arch, st->disable_inv);

   if (anti_collapse_rsv > 0)
   {
      anti_collapse_on = ec_dec_bits(dec, 1);
   }

   unquant_energy_finalise(mode, start, end, oldBandE,
         fine_quant, fine_priority, len*8-ec_tell(dec), dec, C);

   if (anti_collapse_on)
      anti_collapse(mode, X, collapse_masks, LM, C, N,
            start, end, oldBandE, oldLogE, oldLogE2, pulses, st->rng, st->arch);

   if (silence)
   {
      for (i=0;i<C*nbEBands;i++)
         oldBandE[i] = -(28.f);
   }

   celt_synthesis(mode, X, out_syn, oldBandE, start, effEnd,
                  C, CC, isTransient, LM, st->downsample, silence, st->arch);

   c=0; do {
      st->postfilter_period=((st->postfilter_period) > (15) ? (st->postfilter_period) : (15));
      st->postfilter_period_old=((st->postfilter_period_old) > (15) ? (st->postfilter_period_old) : (15));
      comb_filter(out_syn[c], out_syn[c], st->postfilter_period_old, st->postfilter_period, mode->shortMdctSize,
            st->postfilter_gain_old, st->postfilter_gain, st->postfilter_tapset_old, st->postfilter_tapset,
            mode->window, overlap, st->arch);
      if (LM!=0)
         comb_filter(out_syn[c]+mode->shortMdctSize, out_syn[c]+mode->shortMdctSize, st->postfilter_period, postfilter_pitch, N-mode->shortMdctSize,
               st->postfilter_gain, postfilter_gain, st->postfilter_tapset, postfilter_tapset,
               mode->window, overlap, st->arch);

   } while (++c<CC);
   st->postfilter_period_old = st->postfilter_period;
   st->postfilter_gain_old = st->postfilter_gain;
   st->postfilter_tapset_old = st->postfilter_tapset;
   st->postfilter_period = postfilter_pitch;
   st->postfilter_gain = postfilter_gain;
   st->postfilter_tapset = postfilter_tapset;
   if (LM!=0)
   {
      st->postfilter_period_old = st->postfilter_period;
      st->postfilter_gain_old = st->postfilter_gain;
      st->postfilter_tapset_old = st->postfilter_tapset;
   }

   if (C==1)
      (memcpy((&oldBandE[nbEBands]), (oldBandE), (nbEBands)*sizeof(*(&oldBandE[nbEBands])) + 0*((&oldBandE[nbEBands])-(oldBandE)) ));

   if (!isTransient)
   {
      (memcpy((oldLogE2), (oldLogE), (2*nbEBands)*sizeof(*(oldLogE2)) + 0*((oldLogE2)-(oldLogE)) ));
      (memcpy((oldLogE), (oldBandE), (2*nbEBands)*sizeof(*(oldLogE)) + 0*((oldLogE)-(oldBandE)) ));
   } else {
      for (i=0;i<2*nbEBands;i++)
         oldLogE[i] = ((oldLogE[i]) < (oldBandE[i]) ? (oldLogE[i]) : (oldBandE[i]));
   }



   max_background_increase = ((160) < (st->loss_duration+M) ? (160) : (st->loss_duration+M))*(0.001f);
   for (i=0;i<2*nbEBands;i++)
      backgroundLogE[i] = ((backgroundLogE[i] + max_background_increase) < (oldBandE[i]) ? (backgroundLogE[i] + max_background_increase) : (oldBandE[i]));

   c=0; do
   {
      for (i=0;i<start;i++)
      {
         oldBandE[c*nbEBands+i]=0;
         oldLogE[c*nbEBands+i]=oldLogE2[c*nbEBands+i]=-(28.f);
      }
      for (i=end;i<nbEBands;i++)
      {
         oldBandE[c*nbEBands+i]=0;
         oldLogE[c*nbEBands+i]=oldLogE2[c*nbEBands+i]=-(28.f);
      }
   } while (++c<2);
   st->rng = dec->rng;

   deemphasis(out_syn, pcm, N, CC, st->downsample, mode->preemph, st->preemph_memD, accum);
   st->loss_duration = 0;
   ;
   if (ec_tell(dec) > 8*len)
      return -3;
   if(ec_get_error(dec))
      st->error = 1;
   return frame_size/st->downsample;
}
int opus_custom_decode_float(OpusCustomDecoder * restrict st, const unsigned char *data, int len, float * restrict pcm, int frame_size)
{
   return celt_decode_with_ec(st, data, len, pcm, frame_size, 
                                                             ((void *)0)
                                                                 , 0);
}

int opus_custom_decode(OpusCustomDecoder * restrict st, const unsigned char *data, int len, opus_int16 * restrict pcm, int frame_size)
{
   int j, ret, C, N;
   ;
   ;

   if (pcm==
           ((void *)0)
               )
      return -1;

   C = st->channels;
   N = frame_size;
   celt_sig *out = (celt_sig*)DR_OPUS_ALLOCA(sizeof(celt_sig)*(C*N));

   ret=celt_decode_with_ec(st, data, len, out, frame_size, 
                                                          ((void *)0)
                                                              , 0);

   if (ret>0)
      for (j=0;j<C*ret;j++)
         pcm[j] = FLOAT2INT16 (out[j]);

   ;
   return ret;
}




int opus_custom_decoder_ctl(OpusCustomDecoder * restrict st, int request, ...)
{
   va_list ap;

   
  va_start(
  ap
  ,
  request
  )
                       ;
   switch (request)
   {
      case 10010:
      {
         opus_int32 value = 
                           va_arg(
                           ap
                           ,
                           opus_int32
                           )
                                                 ;
         if (value<0 || value>=st->mode->nbEBands)
            goto bad_arg;
         st->start = value;
      }
      break;
      case 10012:
      {
         opus_int32 value = 
                           va_arg(
                           ap
                           ,
                           opus_int32
                           )
                                                 ;
         if (value<1 || value>st->mode->nbEBands)
            goto bad_arg;
         st->end = value;
      }
      break;
      case 10008:
      {
         opus_int32 value = 
                           va_arg(
                           ap
                           ,
                           opus_int32
                           )
                                                 ;
         if (value<1 || value>2)
            goto bad_arg;
         st->stream_channels = value;
      }
      break;
      case 10007:
      {
         opus_int32 *value = 
                            va_arg(
                            ap
                            ,
                            opus_int32*
                            )
                                                   ;
         if (value==
                   ((void *)0)
                       )
            goto bad_arg;
         *value=st->error;
         st->error = 0;
      }
      break;
      case 4027:
      {
         opus_int32 *value = 
                            va_arg(
                            ap
                            ,
                            opus_int32*
                            )
                                                   ;
         if (value==
                   ((void *)0)
                       )
            goto bad_arg;
         *value = st->overlap/st->downsample;
      }
      break;
      case 4028:
      {
         int i;
         opus_val16 *lpc, *oldBandE, *oldLogE, *oldLogE2;
         lpc = (opus_val16*)(st->_decode_mem+(2048 +st->overlap)*st->channels);
         oldBandE = lpc+st->channels*24;
         oldLogE = oldBandE + 2*st->mode->nbEBands;
         oldLogE2 = oldLogE + 2*st->mode->nbEBands;
         (memset(((char*)&st->rng), 0, (opus_custom_decoder_get_size(st->mode, st->channels)- ((char*)&st->rng - (char*)st))*sizeof(*((char*)&st->rng))))

                                                             ;
         for (i=0;i<2*st->mode->nbEBands;i++)
            oldLogE[i]=oldLogE2[i]=-(28.f);
         st->skip_plc = 1;
      }
      break;
      case 4033:
      {
         opus_int32 *value = 
                            va_arg(
                            ap
                            ,
                            opus_int32*
                            )
                                                   ;
         if (value==
                   ((void *)0)
                       )
            goto bad_arg;
         *value = st->postfilter_period;
      }
      break;
      case 10015:
      {
         const OpusCustomMode ** value = 
                                  va_arg(
                                  ap
                                  ,
                                  const OpusCustomMode**
                                  )
                                                              ;
         if (value==0)
            goto bad_arg;
         *value=st->mode;
      }
      break;
      case 10016:
      {
         opus_int32 value = 
                           va_arg(
                           ap
                           ,
                           opus_int32
                           )
                                                 ;
         st->signalling = value;
      }
      break;
      case 4031:
      {
         opus_uint32 * value = 
                              va_arg(
                              ap
                              ,
                              opus_uint32 *
                              )
                                                       ;
         if (value==0)
            goto bad_arg;
         *value=st->rng;
      }
      break;
      case 4046:
      {
          opus_int32 value = 
                            va_arg(
                            ap
                            ,
                            opus_int32
                            )
                                                  ;
          if(value<0 || value>1)
          {
             goto bad_arg;
          }
          st->disable_inv = value;
      }
      break;
      case 4047:
      {
          opus_int32 *value = 
                             va_arg(
                             ap
                             ,
                             opus_int32*
                             )
                                                    ;
          if (!value)
          {
             goto bad_arg;
          }
          *value = st->disable_inv;
      }
      break;
      default:
         goto bad_request;
   }
   
  va_end(
  ap
  )
            ;
   return 0;
bad_arg:
   
  va_end(
  ap
  )
            ;
   return -1;
bad_request:
      
     va_end(
     ap
     )
               ;
  return -5;
}


typedef struct _silk_resampler_state_struct{
    opus_int32 sIIR[ 6 ];
    union{
        opus_int32 i32[ 36 ];
        opus_int16 i16[ 36 ];
    } sFIR;
    opus_int16 delayBuf[ 48 ];
    int resampler_function;
    int batchSize;
    opus_int32 invRatio_Q16;
    int FIR_Order;
    int FIR_Fracs;
    int Fs_in_kHz;
    int Fs_out_kHz;
    int inputDelay;
    const opus_int16 *Coefs;
} silk_resampler_state_struct;

static inline opus_int32 silk_CLZ16(opus_int16 in16)
{
    return 32 - (((int)sizeof(unsigned)*8
               )-(__builtin_clz(in16<<16|0x8000)));
}



static inline opus_int32 silk_CLZ32(opus_int32 in32)
{
    return in32 ? 32 - (((int)sizeof(unsigned)*8
                      )-(__builtin_clz(in32))) : 32;
}
int silk_resampler_init(
    silk_resampler_state_struct *S,
    opus_int32 Fs_Hz_in,
    opus_int32 Fs_Hz_out,
    int forEnc
);




int silk_resampler(
    silk_resampler_state_struct *S,
    opus_int16 out[],
    const opus_int16 in[],
    opus_int32 inLen
);




void silk_resampler_down2(
    opus_int32 *S,
    opus_int16 *out,
    const opus_int16 *in,
    opus_int32 inLen
);




void silk_resampler_down2_3(
    opus_int32 *S,
    opus_int16 *out,
    const opus_int16 *in,
    opus_int32 inLen
);






void silk_biquad_alt_stride1(
    const opus_int16 *in,
    const opus_int32 *B_Q28,
    const opus_int32 *A_Q28,
    opus_int32 *S,
    opus_int16 *out,
    const opus_int32 len
);

void silk_biquad_alt_stride2_c(
    const opus_int16 *in,
    const opus_int32 *B_Q28,
    const opus_int32 *A_Q28,
    opus_int32 *S,
    opus_int16 *out,
    const opus_int32 len
);


void silk_LPC_analysis_filter(
    opus_int16 *out,
    const opus_int16 *in,
    const opus_int16 *B,
    const opus_int32 len,
    const opus_int32 d,
    int arch
);


void silk_bwexpander(
    opus_int16 *ar,
    const int d,
    opus_int32 chirp_Q16
);


void silk_bwexpander_32(
    opus_int32 *ar,
    const int d,
    opus_int32 chirp_Q16
);



opus_int32 silk_LPC_inverse_pred_gain_c(
    const opus_int16 *A_Q12,
    const int order
);


void silk_ana_filt_bank_1(
    const opus_int16 *in,
    opus_int32 *S,
    opus_int16 *outL,
    opus_int16 *outH,
    const opus_int32 N
);
opus_int32 silk_lin2log(
    const opus_int32 inLin
);


int silk_sigm_Q15(
    int in_Q5
);



opus_int32 silk_log2lin(
    const opus_int32 inLog_Q7
);



void silk_sum_sqr_shift(
    opus_int32 *energy,
    int *shift,
    const opus_int16 *x,
    int len
);




opus_int32 silk_schur(
    opus_int16 *rc_Q15,
    const opus_int32 *c,
    const opus_int32 order
);




opus_int32 silk_schur64(
    opus_int32 rc_Q16[],
    const opus_int32 c[],
    opus_int32 order
);


void silk_k2a(
    opus_int32 *A_Q24,
    const opus_int16 *rc_Q15,
    const opus_int32 order
);


void silk_k2a_Q16(
    opus_int32 *A_Q24,
    const opus_int32 *rc_Q16,
    const opus_int32 order
);






void silk_apply_sine_window(
    opus_int16 px_win[],
    const opus_int16 px[],
    const int win_type,
    const int length
);


void silk_autocorr(
    opus_int32 *results,
    int *scale,
    const opus_int16 *inputData,
    const int inputDataSize,
    const int correlationCount,
    int arch
);

void silk_decode_pitch(
    opus_int16 lagIndex,
    opus_int8 contourIndex,
    int pitch_lags[],
    const int Fs_kHz,
    const int nb_subfr
);

int silk_pitch_analysis_core(
    const opus_int16 *frame,
    int *pitch_out,
    opus_int16 *lagIndex,
    opus_int8 *contourIndex,
    int *LTPCorr_Q15,
    int prevLag,
    const opus_int32 search_thres1_Q16,
    const int search_thres2_Q13,
    const int Fs_kHz,
    const int complexity,
    const int nb_subfr,
    int arch
);



void silk_A2NLSF(
    opus_int16 *NLSF,
    opus_int32 *a_Q16,
    const int d
);


void silk_NLSF2A(
    opus_int16 *a_Q12,
    const opus_int16 *NLSF,
    const int d,
    int arch
);


void silk_LPC_fit(
    opus_int16 *a_QOUT,
    opus_int32 *a_QIN,
    const int QOUT,
    const int QIN,
    const int d
);

void silk_insertion_sort_increasing(
    opus_int32 *a,
    int *idx,
    const int L,
    const int K
);

void silk_insertion_sort_decreasing_int16(
    opus_int16 *a,
    int *idx,
    const int L,
    const int K
);

void silk_insertion_sort_increasing_all_values_int16(
     opus_int16 *a,
     const int L
);


void silk_NLSF_stabilize(
          opus_int16 *NLSF_Q15,
    const opus_int16 *NDeltaMin_Q15,
    const int L
);


void silk_NLSF_VQ_weights_laroia(
    opus_int16 *pNLSFW_Q_OUT,
    const opus_int16 *pNLSF_Q15,
    const int D
);


void silk_burg_modified_c(
    opus_int32 *res_nrg,
    int *res_nrg_Q,
    opus_int32 A_Q16[],
    const opus_int16 x[],
    const opus_int32 minInvGain_Q30,
    const int subfr_length,
    const int nb_subfr,
    const int D,
    int arch
);


void silk_scale_copy_vector16(
    opus_int16 *data_out,
    const opus_int16 *data_in,
    opus_int32 gain_Q16,
    const int dataSize
);


void silk_scale_vector32_Q26_lshift_18(
    opus_int32 *data1,
    opus_int32 gain_Q26,
    int dataSize
);







opus_int32 silk_inner_prod_aligned(
    const opus_int16 *const inVec1,
    const opus_int16 *const inVec2,
    const int len,
    int arch
);


opus_int32 silk_inner_prod_aligned_scale(
    const opus_int16 *const inVec1,
    const opus_int16 *const inVec2,
    const int scale,
    const int len
);

opus_int64 silk_inner_prod16_c(
    const opus_int16 *inVec1,
    const opus_int16 *inVec2,
    const int len
);
static inline opus_int32 silk_ROR32( opus_int32 a32, int rot )
{
    opus_uint32 x = (opus_uint32) a32;
    opus_uint32 r = (opus_uint32) rot;
    opus_uint32 m = (opus_uint32) -rot;
    if( rot == 0 ) {
        return a32;
    } else if( rot < 0 ) {
        return (opus_int32) ((x << m) | (x >> (32 - m)));
    } else {
        return (opus_int32) ((x << (32 - r)) | (x >> r));
    }
}
static inline int silk_min_int(int a, int b)
{
    return (((a) < (b)) ? (a) : (b));
}
static inline opus_int16 silk_min_16(opus_int16 a, opus_int16 b)
{
    return (((a) < (b)) ? (a) : (b));
}
static inline opus_int32 silk_min_32(opus_int32 a, opus_int32 b)
{
    return (((a) < (b)) ? (a) : (b));
}
static inline opus_int64 silk_min_64(opus_int64 a, opus_int64 b)
{
    return (((a) < (b)) ? (a) : (b));
}


static inline int silk_max_int(int a, int b)
{
    return (((a) > (b)) ? (a) : (b));
}
static inline opus_int16 silk_max_16(opus_int16 a, opus_int16 b)
{
    return (((a) > (b)) ? (a) : (b));
}
static inline opus_int32 silk_max_32(opus_int32 a, opus_int32 b)
{
    return (((a) > (b)) ? (a) : (b));
}
static inline opus_int64 silk_max_64(opus_int64 a, opus_int64 b)
{
    return (((a) > (b)) ? (a) : (b));
}
static inline opus_int32 silk_CLZ64( opus_int64 in )
{
    opus_int32 in_upper;

    in_upper = (opus_int32)((in)>>(32));
    if (in_upper == 0) {

        return 32 + silk_CLZ32( (opus_int32) in );
    } else {

        return silk_CLZ32( in_upper );
    }
}


static inline void silk_CLZ_FRAC(
    opus_int32 in,
    opus_int32 *lz,
    opus_int32 *frac_Q7
)
{
    opus_int32 lzeros = silk_CLZ32(in);

    * lz = lzeros;
    * frac_Q7 = silk_ROR32(in, 24 - lzeros) & 0x7f;
}




static inline opus_int32 silk_SQRT_APPROX( opus_int32 x )
{
    opus_int32 y, lz, frac_Q7;

    if( x <= 0 ) {
        return 0;
    }

    silk_CLZ_FRAC(x, &lz, &frac_Q7);

    if( lz & 1 ) {
        y = 32768;
    } else {
        y = 46214;
    }


    y >>= ((lz)>>(1));


    y = ((opus_int32)((y) + (((y) * (opus_int64)((opus_int16)(((opus_int32)((opus_int16)(213)) * (opus_int32)((opus_int16)(frac_Q7)))))) >> 16)));

    return y;
}


static inline opus_int32 silk_DIV32_varQ(
    const opus_int32 a32,
    const opus_int32 b32,
    const int Qres
)
{
    int a_headrm, b_headrm, lshift;
    opus_int32 b32_inv, a32_nrm, b32_nrm, result;

    ;
    ;


    a_headrm = silk_CLZ32( (((a32) > 0) ? (a32) : -(a32)) ) - 1;
    a32_nrm = ((opus_int32)((opus_uint32)(a32)<<(a_headrm)));
    b_headrm = silk_CLZ32( (((b32) > 0) ? (b32) : -(b32)) ) - 1;
    b32_nrm = ((opus_int32)((opus_uint32)(b32)<<(b_headrm)));


    b32_inv = ((opus_int32)((0x7FFFFFFF >> 2) / (((b32_nrm)>>(16)))));


    result = ((opus_int32)(((a32_nrm) * (opus_int64)((opus_int16)(b32_inv))) >> 16));



    a32_nrm = ((opus_int32)((opus_uint32)(a32_nrm) - (opus_uint32)(((opus_int32)((opus_uint32)((opus_int32)((((opus_int64)((b32_nrm)) * ((result))))>>(32))) << (3))))));


    result = ((opus_int32)((result) + (((a32_nrm) * (opus_int64)((opus_int16)(b32_inv))) >> 16)));


    lshift = 29 + a_headrm - b_headrm - Qres;
    if( lshift < 0 ) {
        return (((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((-lshift)))) > (((0x7FFFFFFF)>>((-lshift)))) ? (((result)) > (((((opus_int32)0x80000000))>>((-lshift)))) ? (((((opus_int32)0x80000000))>>((-lshift)))) : (((result)) < (((0x7FFFFFFF)>>((-lshift)))) ? (((0x7FFFFFFF)>>((-lshift)))) : ((result)))) : (((result)) > (((0x7FFFFFFF)>>((-lshift)))) ? (((0x7FFFFFFF)>>((-lshift)))) : (((result)) < (((((opus_int32)0x80000000))>>((-lshift)))) ? (((((opus_int32)0x80000000))>>((-lshift)))) : ((result))))))<<((-lshift)))));
    } else {
        if( lshift < 32){
            return ((result)>>(lshift));
        } else {

            return 0;
        }
    }
}


static inline opus_int32 silk_INVERSE32_varQ(
    const opus_int32 b32,
    const int Qres
)
{
    int b_headrm, lshift;
    opus_int32 b32_inv, b32_nrm, err_Q32, result;

    ;
    ;


    b_headrm = silk_CLZ32( (((b32) > 0) ? (b32) : -(b32)) ) - 1;
    b32_nrm = ((opus_int32)((opus_uint32)(b32)<<(b_headrm)));


    b32_inv = ((opus_int32)((0x7FFFFFFF >> 2) / (((b32_nrm)>>(16)))));


    result = ((opus_int32)((opus_uint32)(b32_inv)<<(16)));


    err_Q32 = ((opus_int32)((opus_uint32)(((opus_int32)1<<29) - ((opus_int32)(((b32_nrm) * (opus_int64)((opus_int16)(b32_inv))) >> 16)))<<(3)));


    result = ((opus_int32)((result) + (((opus_int64)(err_Q32) * (b32_inv)) >> 16)));


    lshift = 61 - b_headrm - Qres;
    if( lshift <= 0 ) {
        return (((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((-lshift)))) > (((0x7FFFFFFF)>>((-lshift)))) ? (((result)) > (((((opus_int32)0x80000000))>>((-lshift)))) ? (((((opus_int32)0x80000000))>>((-lshift)))) : (((result)) < (((0x7FFFFFFF)>>((-lshift)))) ? (((0x7FFFFFFF)>>((-lshift)))) : ((result)))) : (((result)) > (((0x7FFFFFFF)>>((-lshift)))) ? (((0x7FFFFFFF)>>((-lshift)))) : (((result)) < (((((opus_int32)0x80000000))>>((-lshift)))) ? (((((opus_int32)0x80000000))>>((-lshift)))) : ((result))))))<<((-lshift)))));
    } else {
        if( lshift < 32){
            return ((result)>>(lshift));
        }else{

            return 0;
        }
    }
}
typedef struct {
    opus_int16 xq[ 2 * ( ( 5 * 4 ) * 16 ) ];
    opus_int32 sLTP_shp_Q14[ 2 * ( ( 5 * 4 ) * 16 ) ];
    opus_int32 sLPC_Q14[ ( 5 * 16 ) + 16 ];
    opus_int32 sAR2_Q14[ 24 ];
    opus_int32 sLF_AR_shp_Q14;
    opus_int32 sDiff_shp_Q14;
    int lagPrev;
    int sLTP_buf_idx;
    int sLTP_shp_buf_idx;
    opus_int32 rand_seed;
    opus_int32 prev_gain_Q16;
    int rewhite_flag;
} silk_nsq_state;




typedef struct {
    opus_int32 AnaState[ 2 ];
    opus_int32 AnaState1[ 2 ];
    opus_int32 AnaState2[ 2 ];
    opus_int32 XnrgSubfr[ 4 ];
    opus_int32 NrgRatioSmth_Q8[ 4 ];
    opus_int16 HPstate;
    opus_int32 NL[ 4 ];
    opus_int32 inv_NL[ 4 ];
    opus_int32 NoiseLevelBias[ 4 ];
    opus_int32 counter;
} silk_VAD_state;


typedef struct {
    opus_int32 In_LP_State[ 2 ];
    opus_int32 transition_frame_no;
    int mode;
    opus_int32 saved_fs_kHz;
} silk_LP_state;


typedef struct {
    const opus_int16 nVectors;
    const opus_int16 order;
    const opus_int16 quantStepSize_Q16;
    const opus_int16 invQuantStepSize_Q6;
    const opus_uint8 *CB1_NLSF_Q8;
    const opus_int16 *CB1_Wght_Q9;
    const opus_uint8 *CB1_iCDF;
    const opus_uint8 *pred_Q8;
    const opus_uint8 *ec_sel;
    const opus_uint8 *ec_iCDF;
    const opus_uint8 *ec_Rates_Q5;
    const opus_int16 *deltaMin_Q15;
} silk_NLSF_CB_struct;

typedef struct {
    opus_int16 pred_prev_Q13[ 2 ];
    opus_int16 sMid[ 2 ];
    opus_int16 sSide[ 2 ];
    opus_int32 mid_side_amp_Q0[ 4 ];
    opus_int16 smth_width_Q14;
    opus_int16 width_prev_Q14;
    opus_int16 silent_side_len;
    opus_int8 predIx[ 3 ][ 2 ][ 3 ];
    opus_int8 mid_only_flags[ 3 ];
} stereo_enc_state;

typedef struct {
    opus_int16 pred_prev_Q13[ 2 ];
    opus_int16 sMid[ 2 ];
    opus_int16 sSide[ 2 ];
} stereo_dec_state;

typedef struct {
    opus_int8 GainsIndices[ 4 ];
    opus_int8 LTPIndex[ 4 ];
    opus_int8 NLSFIndices[ 16 + 1 ];
    opus_int16 lagIndex;
    opus_int8 contourIndex;
    opus_int8 signalType;
    opus_int8 quantOffsetType;
    opus_int8 NLSFInterpCoef_Q2;
    opus_int8 PERIndex;
    opus_int8 LTP_scaleIndex;
    opus_int8 Seed;
} SideInfoIndices;




typedef struct {
    opus_int32 In_HP_State[ 2 ];
    opus_int32 variable_HP_smth1_Q15;
    opus_int32 variable_HP_smth2_Q15;
    silk_LP_state sLP;
    silk_VAD_state sVAD;
    silk_nsq_state sNSQ;
    opus_int16 prev_NLSFq_Q15[ 16 ];
    int speech_activity_Q8;
    int allow_bandwidth_switch;
    opus_int8 LBRRprevLastGainIndex;
    opus_int8 prevSignalType;
    int prevLag;
    int pitch_LPC_win_length;
    int max_pitch_lag;
    opus_int32 API_fs_Hz;
    opus_int32 prev_API_fs_Hz;
    int maxInternal_fs_Hz;
    int minInternal_fs_Hz;
    int desiredInternal_fs_Hz;
    int fs_kHz;
    int nb_subfr;
    int frame_length;
    int subfr_length;
    int ltp_mem_length;
    int la_pitch;
    int la_shape;
    int shapeWinLength;
    opus_int32 TargetRate_bps;
    int PacketSize_ms;
    int PacketLoss_perc;
    opus_int32 frameCounter;
    int Complexity;
    int nStatesDelayedDecision;
    int useInterpolatedNLSFs;
    int shapingLPCOrder;
    int predictLPCOrder;
    int pitchEstimationComplexity;
    int pitchEstimationLPCOrder;
    opus_int32 pitchEstimationThreshold_Q16;
    opus_int32 sum_log_gain_Q7;
    int NLSF_MSVQ_Survivors;
    int first_frame_after_reset;
    int controlled_since_last_payload;
    int warping_Q16;
    int useCBR;
    int prefillFlag;
    const opus_uint8 *pitch_lag_low_bits_iCDF;
    const opus_uint8 *pitch_contour_iCDF;
    const silk_NLSF_CB_struct *psNLSF_CB;
    int input_quality_bands_Q15[ 4 ];
    int input_tilt_Q15;
    int SNR_dB_Q7;

    opus_int8 VAD_flags[ 3 ];
    opus_int8 LBRR_flag;
    int LBRR_flags[ 3 ];

    SideInfoIndices indices;
    opus_int8 pulses[ ( ( 5 * 4 ) * 16 ) ];

    int arch;


    opus_int16 inputBuf[ ( ( 5 * 4 ) * 16 ) + 2 ];
    int inputBufIx;
    int nFramesPerPacket;
    int nFramesEncoded;

    int nChannelsAPI;
    int nChannelsInternal;
    int channelNb;


    int frames_since_onset;


    int ec_prevSignalType;
    opus_int16 ec_prevLagIndex;

    silk_resampler_state_struct resampler_state;


    int useDTX;
    int inDTX;
    int noSpeechCounter;


    int useInBandFEC;
    int LBRR_enabled;
    int LBRR_GainIncreases;
    SideInfoIndices indices_LBRR[ 3 ];
    opus_int8 pulses_LBRR[ 3 ][ ( ( 5 * 4 ) * 16 ) ];
} silk_encoder_state;



typedef struct {
    opus_int32 pitchL_Q8;
    opus_int16 LTPCoef_Q14[ 5 ];
    opus_int16 prevLPC_Q12[ 16 ];
    int last_frame_lost;
    opus_int32 rand_seed;
    opus_int16 randScale_Q14;
    opus_int32 conc_energy;
    int conc_energy_shift;
    opus_int16 prevLTP_scale_Q14;
    opus_int32 prevGain_Q16[ 2 ];
    int fs_kHz;
    int nb_subfr;
    int subfr_length;
} silk_PLC_struct;


typedef struct {
    opus_int32 CNG_exc_buf_Q14[ ( ( 5 * 4 ) * 16 ) ];
    opus_int16 CNG_smth_NLSF_Q15[ 16 ];
    opus_int32 CNG_synth_state[ 16 ];
    opus_int32 CNG_smth_Gain_Q16;
    opus_int32 rand_seed;
    int fs_kHz;
} silk_CNG_struct;




typedef struct {
    opus_int32 prev_gain_Q16;
    opus_int32 exc_Q14[ ( ( 5 * 4 ) * 16 ) ];
    opus_int32 sLPC_Q14_buf[ 16 ];
    opus_int16 outBuf[ ( ( 5 * 4 ) * 16 ) + 2 * ( 5 * 16 ) ];
    int lagPrev;
    opus_int8 LastGainIndex;
    int fs_kHz;
    opus_int32 fs_API_hz;
    int nb_subfr;
    int frame_length;
    int subfr_length;
    int ltp_mem_length;
    int LPC_order;
    opus_int16 prevNLSF_Q15[ 16 ];
    int first_frame_after_reset;
    const opus_uint8 *pitch_lag_low_bits_iCDF;
    const opus_uint8 *pitch_contour_iCDF;


    int nFramesDecoded;
    int nFramesPerPacket;


    int ec_prevSignalType;
    opus_int16 ec_prevLagIndex;

    int VAD_flags[ 3 ];
    int LBRR_flag;
    int LBRR_flags[ 3 ];

    silk_resampler_state_struct resampler_state;

    const silk_NLSF_CB_struct *psNLSF_CB;


    SideInfoIndices indices;


    silk_CNG_struct sCNG;


    int lossCnt;
    int prevSignalType;
    int arch;

    silk_PLC_struct sPLC;

} silk_decoder_state;




typedef struct {

    int pitchL[ 4 ];
    opus_int32 Gains_Q16[ 4 ];

    opus_int16 PredCoef_Q12[ 2 ][ 16 ];
    opus_int16 LTPCoef_Q14[ 5 * 4 ];
    int LTP_scale_Q14;
} silk_decoder_control;
extern const opus_uint8 silk_gain_iCDF[ 3 ][ 64 / 8 ];
extern const opus_uint8 silk_delta_gain_iCDF[ 36 - -4 + 1 ];

extern const opus_uint8 silk_pitch_lag_iCDF[ 2 * ( 18 - 2 ) ];
extern const opus_uint8 silk_pitch_delta_iCDF[ 21 ];
extern const opus_uint8 silk_pitch_contour_iCDF[ 34 ];
extern const opus_uint8 silk_pitch_contour_NB_iCDF[ 11 ];
extern const opus_uint8 silk_pitch_contour_10_ms_iCDF[ 12 ];
extern const opus_uint8 silk_pitch_contour_10_ms_NB_iCDF[ 3 ];

extern const opus_uint8 silk_pulses_per_block_iCDF[ 10 ][ 16 + 2 ];
extern const opus_uint8 silk_pulses_per_block_BITS_Q5[ 10 - 1 ][ 16 + 2 ];

extern const opus_uint8 silk_rate_levels_iCDF[ 2 ][ 10 - 1 ];
extern const opus_uint8 silk_rate_levels_BITS_Q5[ 2 ][ 10 - 1 ];

extern const opus_uint8 silk_max_pulses_table[ 4 ];

extern const opus_uint8 silk_shell_code_table0[ 152 ];
extern const opus_uint8 silk_shell_code_table1[ 152 ];
extern const opus_uint8 silk_shell_code_table2[ 152 ];
extern const opus_uint8 silk_shell_code_table3[ 152 ];
extern const opus_uint8 silk_shell_code_table_offsets[ 16 + 1 ];

extern const opus_uint8 silk_lsb_iCDF[ 2 ];

extern const opus_uint8 silk_sign_iCDF[ 42 ];

extern const opus_uint8 silk_uniform3_iCDF[ 3 ];
extern const opus_uint8 silk_uniform4_iCDF[ 4 ];
extern const opus_uint8 silk_uniform5_iCDF[ 5 ];
extern const opus_uint8 silk_uniform6_iCDF[ 6 ];
extern const opus_uint8 silk_uniform8_iCDF[ 8 ];

extern const opus_uint8 silk_NLSF_EXT_iCDF[ 7 ];

extern const opus_uint8 silk_LTP_per_index_iCDF[ 3 ];
extern const opus_uint8 * const silk_LTP_gain_iCDF_ptrs[ 3 ];
extern const opus_uint8 * const silk_LTP_gain_BITS_Q5_ptrs[ 3 ];
extern const opus_int8 * const silk_LTP_vq_ptrs_Q7[ 3 ];
extern const opus_uint8 * const silk_LTP_vq_gain_ptrs_Q7[3];
extern const opus_int8 silk_LTP_vq_sizes[ 3 ];

extern const opus_uint8 silk_LTPscale_iCDF[ 3 ];
extern const opus_int16 silk_LTPScales_table_Q14[ 3 ];

extern const opus_uint8 silk_type_offset_VAD_iCDF[ 4 ];
extern const opus_uint8 silk_type_offset_no_VAD_iCDF[ 2 ];

extern const opus_int16 silk_stereo_pred_quant_Q13[ 16 ];
extern const opus_uint8 silk_stereo_pred_joint_iCDF[ 25 ];
extern const opus_uint8 silk_stereo_only_code_mid_iCDF[ 2 ];

extern const opus_uint8 * const silk_LBRR_flags_iCDF_ptr[ 2 ];

extern const opus_uint8 silk_NLSF_interpolation_factor_iCDF[ 5 ];

extern const silk_NLSF_CB_struct silk_NLSF_CB_WB;
extern const silk_NLSF_CB_struct silk_NLSF_CB_NB_MB;


extern const opus_int16 silk_Quantization_Offsets_Q10[ 2 ][ 2 ];


extern const opus_int32 silk_Transition_LP_B_Q28[ 5 ][ 3 ];
extern const opus_int32 silk_Transition_LP_A_Q28[ 5 ][ 2 ];


extern const opus_int16 silk_LSFCosTab_FIX_Q12[ 128 + 1 ];
void silk_PLC_Reset(
    silk_decoder_state *psDec
);

void silk_PLC(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    opus_int16 frame[],
    int lost,
    int arch
);

void silk_PLC_glue_frames(
    silk_decoder_state *psDec,
    opus_int16 frame[],
    int length
);
typedef struct {

    opus_int32 nChannelsAPI;


    opus_int32 nChannelsInternal;


    opus_int32 API_sampleRate;


    opus_int32 maxInternalSampleRate;


    opus_int32 minInternalSampleRate;


    opus_int32 desiredInternalSampleRate;


    int payloadSize_ms;


    opus_int32 bitRate;


    int packetLossPercentage;


    int complexity;


    int useInBandFEC;


    int LBRR_coded;


    int useDTX;


    int useCBR;


    int maxBits;


    int toMono;


    int opusCanSwitch;


    int reducedDependency;


    opus_int32 internalSampleRate;


    int allowBandwidthSwitch;


    int inWBmodeWithoutVariableLP;


    int stereoWidth_Q14;


    int switchReady;


    int signalType;


    int offset;
} silk_EncControlStruct;




typedef struct {

    opus_int32 nChannelsAPI;


    opus_int32 nChannelsInternal;


    opus_int32 API_sampleRate;


    opus_int32 internalSampleRate;


    int payloadSize_ms;


    int prevPitchLag;
} silk_DecControlStruct;
void silk_stereo_LR_to_MS(
    stereo_enc_state *state,
    opus_int16 x1[],
    opus_int16 x2[],
    opus_int8 ix[ 2 ][ 3 ],
    opus_int8 *mid_only_flag,
    opus_int32 mid_side_rates_bps[],
    opus_int32 total_rate_bps,
    int prev_speech_act_Q8,
    int toMono,
    int fs_kHz,
    int frame_length
);


void silk_stereo_MS_to_LR(
    stereo_dec_state *state,
    opus_int16 x1[],
    opus_int16 x2[],
    const opus_int32 pred_Q13[],
    int fs_kHz,
    int frame_length
);


opus_int32 silk_stereo_find_predictor(
    opus_int32 *ratio_Q14,
    const opus_int16 x[],
    const opus_int16 y[],
    opus_int32 mid_res_amp_Q0[],
    int length,
    int smooth_coef_Q16
);


void silk_stereo_quant_pred(
    opus_int32 pred_Q13[],
    opus_int8 ix[ 2 ][ 3 ]
);


void silk_stereo_encode_pred(
    ec_enc *psRangeEnc,
    opus_int8 ix[ 2 ][ 3 ]
);


void silk_stereo_encode_mid_only(
    ec_enc *psRangeEnc,
    opus_int8 mid_only_flag
);


void silk_stereo_decode_pred(
    ec_dec *psRangeDec,
    opus_int32 pred_Q13[]
);


void silk_stereo_decode_mid_only(
    ec_dec *psRangeDec,
    int *decode_only_mid
);


void silk_encode_signs(
    ec_enc *psRangeEnc,
    const opus_int8 pulses[],
    int length,
    const int signalType,
    const int quantOffsetType,
    const int sum_pulses[ ( ( ( 5 * 4 ) * 16 ) / 16 ) ]
);


void silk_decode_signs(
    ec_dec *psRangeDec,
    opus_int16 pulses[],
    int length,
    const int signalType,
    const int quantOffsetType,
    const int sum_pulses[ ( ( ( 5 * 4 ) * 16 ) / 16 ) ]
);


int check_control_input(
    silk_EncControlStruct *encControl
);


int silk_control_audio_bandwidth(
    silk_encoder_state *psEncC,
    silk_EncControlStruct *encControl
);


int silk_control_SNR(
    silk_encoder_state *psEncC,
    opus_int32 TargetRate_bps
);






void silk_encode_pulses(
    ec_enc *psRangeEnc,
    const int signalType,
    const int quantOffsetType,
    opus_int8 pulses[],
    const int frame_length
);


void silk_shell_encoder(
    ec_enc *psRangeEnc,
    const int *pulses0
);


void silk_shell_decoder(
    opus_int16 *pulses0,
    ec_dec *psRangeDec,
    const int pulses4
);


void silk_gains_quant(
    opus_int8 ind[ 4 ],
    opus_int32 gain_Q16[ 4 ],
    opus_int8 *prev_ind,
    const int conditional,
    const int nb_subfr
);


void silk_gains_dequant(
    opus_int32 gain_Q16[ 4 ],
    const opus_int8 ind[ 4 ],
    opus_int8 *prev_ind,
    const int conditional,
    const int nb_subfr
);


opus_int32 silk_gains_ID(
    const opus_int8 ind[ 4 ],
    const int nb_subfr
);


void silk_interpolate(
    opus_int16 xi[ 16 ],
    const opus_int16 x0[ 16 ],
    const opus_int16 x1[ 16 ],
    const int ifact_Q2,
    const int d
);


void silk_quant_LTP_gains(
    opus_int16 B_Q14[ 4 * 5 ],
    opus_int8 cbk_index[ 4 ],
    opus_int8 *periodicity_index,
    opus_int32 *sum_gain_dB_Q7,
    int *pred_gain_dB_Q7,
    const opus_int32 XX_Q17[ 4*5*5 ],
    const opus_int32 xX_Q17[ 4*5 ],
    const int subfr_len,
    const int nb_subfr,
    int arch
);


void silk_VQ_WMat_EC_c(
    opus_int8 *ind,
    opus_int32 *res_nrg_Q15,
    opus_int32 *rate_dist_Q8,
    int *gain_Q7,
    const opus_int32 *XX_Q17,
    const opus_int32 *xX_Q17,
    const opus_int8 *cb_Q7,
    const opus_uint8 *cb_gain_Q7,
    const opus_uint8 *cl_Q5,
    const int subfr_len,
    const opus_int32 max_gain_Q7,
    const int L
);
void silk_NSQ_c(
    const silk_encoder_state *psEncC,
    silk_nsq_state *NSQ,
    SideInfoIndices *psIndices,
    const opus_int16 x16[],
    opus_int8 pulses[],
    const opus_int16 PredCoef_Q12[ 2 * 16 ],
    const opus_int16 LTPCoef_Q14[ 5 * 4 ],
    const opus_int16 AR_Q13[ 4 * 24 ],
    const int HarmShapeGain_Q14[ 4 ],
    const int Tilt_Q14[ 4 ],
    const opus_int32 LF_shp_Q14[ 4 ],
    const opus_int32 Gains_Q16[ 4 ],
    const int pitchL[ 4 ],
    const int Lambda_Q10,
    const int LTP_scale_Q14
);
void silk_NSQ_del_dec_c(
    const silk_encoder_state *psEncC,
    silk_nsq_state *NSQ,
    SideInfoIndices *psIndices,
    const opus_int16 x16[],
    opus_int8 pulses[],
    const opus_int16 PredCoef_Q12[ 2 * 16 ],
    const opus_int16 LTPCoef_Q14[ 5 * 4 ],
    const opus_int16 AR_Q13[ 4 * 24 ],
    const int HarmShapeGain_Q14[ 4 ],
    const int Tilt_Q14[ 4 ],
    const opus_int32 LF_shp_Q14[ 4 ],
    const opus_int32 Gains_Q16[ 4 ],
    const int pitchL[ 4 ],
    const int Lambda_Q10,
    const int LTP_scale_Q14
);
int silk_VAD_Init(
    silk_VAD_state *psSilk_VAD
);


int silk_VAD_GetSA_Q8_c(
    silk_encoder_state *psEncC,
    const opus_int16 pIn[]
);
void silk_LP_variable_cutoff(
    silk_LP_state *psLP,
    opus_int16 *frame,
    const int frame_length
);





void silk_process_NLSFs(
    silk_encoder_state *psEncC,
    opus_int16 PredCoef_Q12[ 2 ][ 16 ],
    opus_int16 pNLSF_Q15[ 16 ],
    const opus_int16 prev_NLSFq_Q15[ 16 ]
);

opus_int32 silk_NLSF_encode(
          opus_int8 *NLSFIndices,
          opus_int16 *pNLSF_Q15,
    const silk_NLSF_CB_struct *psNLSF_CB,
    const opus_int16 *pW_QW,
    const int NLSF_mu_Q20,
    const int nSurvivors,
    const int signalType
);


void silk_NLSF_VQ(
    opus_int32 err_Q26[],
    const opus_int16 in_Q15[],
    const opus_uint8 pCB_Q8[],
    const opus_int16 pWght_Q9[],
    const int K,
    const int LPC_order
);


opus_int32 silk_NLSF_del_dec_quant(
    opus_int8 indices[],
    const opus_int16 x_Q10[],
    const opus_int16 w_Q5[],
    const opus_uint8 pred_coef_Q8[],
    const opus_int16 ec_ix[],
    const opus_uint8 ec_rates_Q5[],
    const int quant_step_size_Q16,
    const opus_int16 inv_quant_step_size_Q6,
    const opus_int32 mu_Q20,
    const opus_int16 order
);


void silk_NLSF_unpack(
          opus_int16 ec_ix[],
          opus_uint8 pred_Q8[],
    const silk_NLSF_CB_struct *psNLSF_CB,
    const int CB1_index
);




void silk_NLSF_decode(
          opus_int16 *pNLSF_Q15,
          opus_int8 *NLSFIndices,
    const silk_NLSF_CB_struct *psNLSF_CB
);




int silk_init_decoder(
    silk_decoder_state *psDec
);


int silk_decoder_set_fs(
    silk_decoder_state *psDec,
    int fs_kHz,
    opus_int32 fs_API_Hz
);




int silk_decode_frame(
    silk_decoder_state *psDec,
    ec_dec *psRangeDec,
    opus_int16 pOut[],
    opus_int32 *pN,
    int lostFlag,
    int condCoding,
    int arch
);


void silk_decode_indices(
    silk_decoder_state *psDec,
    ec_dec *psRangeDec,
    int FrameIndex,
    int decode_LBRR,
    int condCoding
);


void silk_decode_parameters(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    int condCoding
);


void silk_decode_core(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    opus_int16 xq[],
    const opus_int16 pulses[ ( ( 5 * 4 ) * 16 ) ],
    int arch
);


void silk_decode_pulses(
    ec_dec *psRangeDec,
    opus_int16 pulses[],
    const int signalType,
    const int quantOffsetType,
    const int frame_length
);






void silk_CNG_Reset(
    silk_decoder_state *psDec
);


void silk_CNG(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    opus_int16 frame[],
    int length
);


void silk_encode_indices(
    silk_encoder_state *psEncC,
    ec_enc *psRangeEnc,
    int FrameIndex,
    int encode_LBRR,
    int condCoding
);




int silk_init_decoder(
    silk_decoder_state *psDec
)
{

    memset((psDec), (0), (sizeof( silk_decoder_state )));


    psDec->first_frame_after_reset = 1;
    psDec->prev_gain_Q16 = 65536;
    psDec->arch = opus_select_arch();


    silk_CNG_Reset( psDec );


    silk_PLC_Reset( psDec );

    return(0);
}




void silk_decode_core(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    opus_int16 xq[],
    const opus_int16 pulses[ ( ( 5 * 4 ) * 16 ) ],
    int arch
)
{
    int i, k, lag = 0, start_idx, sLTP_buf_idx, NLSF_interpolation_flag, signalType;
    opus_int16 *A_Q12, *B_Q14, *pxq, A_Q12_tmp[ 16 ];
    ;
    ;
    opus_int32 LTP_pred_Q13, LPC_pred_Q10, Gain_Q10, inv_gain_Q31, gain_adj_Q16, rand_seed, offset_Q10;
    opus_int32 *pred_lag_ptr, *pexc_Q14, *pres_Q14;
    ;
    ;
    ;

    ;

    opus_int16 *sLTP = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*(psDec->ltp_mem_length));
    opus_int32 *sLTP_Q15 = (opus_int32*)DR_OPUS_ALLOCA(sizeof(opus_int32)*(psDec->ltp_mem_length + psDec->frame_length));
    opus_int32 *res_Q14 = (opus_int32*)DR_OPUS_ALLOCA(sizeof(opus_int32)*(psDec->subfr_length));
    opus_int32 *sLPC_Q14 = (opus_int32*)DR_OPUS_ALLOCA(sizeof(opus_int32)*(psDec->subfr_length + 16));

    offset_Q10 = silk_Quantization_Offsets_Q10[ psDec->indices.signalType >> 1 ][ psDec->indices.quantOffsetType ];

    if( psDec->indices.NLSFInterpCoef_Q2 < 1 << 2 ) {
        NLSF_interpolation_flag = 1;
    } else {
        NLSF_interpolation_flag = 0;
    }


    rand_seed = psDec->indices.Seed;
    for( i = 0; i < psDec->frame_length; i++ ) {
        rand_seed = (((opus_int32)((opus_uint32)(((907633515))) + (opus_uint32)((opus_uint32)((rand_seed)) * (opus_uint32)((196314165))))));
        psDec->exc_Q14[ i ] = ((opus_int32)((opus_uint32)((opus_int32)pulses[ i ])<<(14)));
        if( psDec->exc_Q14[ i ] > 0 ) {
            psDec->exc_Q14[ i ] -= 80 << 4;
        } else
        if( psDec->exc_Q14[ i ] < 0 ) {
            psDec->exc_Q14[ i ] += 80 << 4;
        }
        psDec->exc_Q14[ i ] += offset_Q10 << 4;
        if( rand_seed < 0 ) {
           psDec->exc_Q14[ i ] = -psDec->exc_Q14[ i ];
        }

        rand_seed = ((opus_int32)((opus_uint32)(rand_seed) + (opus_uint32)(pulses[ i ])));
    }


    memcpy((sLPC_Q14), (psDec->sLPC_Q14_buf), (16 * sizeof( opus_int32 )));

    pexc_Q14 = psDec->exc_Q14;
    pxq = xq;
    sLTP_buf_idx = psDec->ltp_mem_length;

    for( k = 0; k < psDec->nb_subfr; k++ ) {
        pres_Q14 = res_Q14;
        A_Q12 = psDecCtrl->PredCoef_Q12[ k >> 1 ];


        memcpy((A_Q12_tmp), (A_Q12), (psDec->LPC_order * sizeof( opus_int16 )));
        B_Q14 = &psDecCtrl->LTPCoef_Q14[ k * 5 ];
        signalType = psDec->indices.signalType;

        Gain_Q10 = ((psDecCtrl->Gains_Q16[ k ])>>(6));
        inv_gain_Q31 = silk_INVERSE32_varQ( psDecCtrl->Gains_Q16[ k ], 47 );


        if( psDecCtrl->Gains_Q16[ k ] != psDec->prev_gain_Q16 ) {
            gain_adj_Q16 = silk_DIV32_varQ( psDec->prev_gain_Q16, psDecCtrl->Gains_Q16[ k ], 16 );


            for( i = 0; i < 16; i++ ) {
                sLPC_Q14[ i ] = ((opus_int32)(((opus_int64)(gain_adj_Q16) * (sLPC_Q14[ i ])) >> 16));
            }
        } else {
            gain_adj_Q16 = (opus_int32)1 << 16;
        }


        ;
        psDec->prev_gain_Q16 = psDecCtrl->Gains_Q16[ k ];


        if( psDec->lossCnt && psDec->prevSignalType == 2 &&
            psDec->indices.signalType != 2 && k < 4/2 ) {

            memset((B_Q14), (0), (5 * sizeof( opus_int16 )));
            B_Q14[ 5/2 ] = ((opus_int32)((0.25) * ((opus_int64)1 << (14)) + 0.5));

            signalType = 2;
            psDecCtrl->pitchL[ k ] = psDec->lagPrev;
        }

        if( signalType == 2 ) {

            lag = psDecCtrl->pitchL[ k ];


            if( k == 0 || ( k == 2 && NLSF_interpolation_flag ) ) {

                start_idx = psDec->ltp_mem_length - lag - psDec->LPC_order - 5 / 2;
                ;

                if( k == 2 ) {
                    memcpy((&psDec->outBuf[ psDec->ltp_mem_length ]), (xq), (2 * psDec->subfr_length * sizeof( opus_int16 )));
                }

                silk_LPC_analysis_filter( &sLTP[ start_idx ], &psDec->outBuf[ start_idx + k * psDec->subfr_length ],
                    A_Q12, psDec->ltp_mem_length - start_idx, psDec->LPC_order, arch );


                if( k == 0 ) {

                    inv_gain_Q31 = ((opus_int32)((opus_uint32)(((opus_int32)(((inv_gain_Q31) * (opus_int64)((opus_int16)(psDecCtrl->LTP_scale_Q14))) >> 16)))<<(2)));
                }
                for( i = 0; i < lag + 5/2; i++ ) {
                    sLTP_Q15[ sLTP_buf_idx - i - 1 ] = ((opus_int32)(((inv_gain_Q31) * (opus_int64)((opus_int16)(sLTP[ psDec->ltp_mem_length - i - 1 ]))) >> 16));
                }
            } else {

                if( gain_adj_Q16 != (opus_int32)1 << 16 ) {
                    for( i = 0; i < lag + 5/2; i++ ) {
                        sLTP_Q15[ sLTP_buf_idx - i - 1 ] = ((opus_int32)(((opus_int64)(gain_adj_Q16) * (sLTP_Q15[ sLTP_buf_idx - i - 1 ])) >> 16));
                    }
                }
            }
        }


        if( signalType == 2 ) {

            pred_lag_ptr = &sLTP_Q15[ sLTP_buf_idx - lag + 5 / 2 ];
            for( i = 0; i < psDec->subfr_length; i++ ) {


                LTP_pred_Q13 = 2;
                LTP_pred_Q13 = ((opus_int32)((LTP_pred_Q13) + (((pred_lag_ptr[ 0 ]) * (opus_int64)((opus_int16)(B_Q14[ 0 ]))) >> 16)));
                LTP_pred_Q13 = ((opus_int32)((LTP_pred_Q13) + (((pred_lag_ptr[ -1 ]) * (opus_int64)((opus_int16)(B_Q14[ 1 ]))) >> 16)));
                LTP_pred_Q13 = ((opus_int32)((LTP_pred_Q13) + (((pred_lag_ptr[ -2 ]) * (opus_int64)((opus_int16)(B_Q14[ 2 ]))) >> 16)));
                LTP_pred_Q13 = ((opus_int32)((LTP_pred_Q13) + (((pred_lag_ptr[ -3 ]) * (opus_int64)((opus_int16)(B_Q14[ 3 ]))) >> 16)));
                LTP_pred_Q13 = ((opus_int32)((LTP_pred_Q13) + (((pred_lag_ptr[ -4 ]) * (opus_int64)((opus_int16)(B_Q14[ 4 ]))) >> 16)));
                pred_lag_ptr++;


                pres_Q14[ i ] = (((pexc_Q14[ i ])) + (((opus_int32)((opus_uint32)((LTP_pred_Q13))<<((1))))));


                sLTP_Q15[ sLTP_buf_idx ] = ((opus_int32)((opus_uint32)(pres_Q14[ i ])<<(1)));
                sLTP_buf_idx++;
            }
        } else {
            pres_Q14 = pexc_Q14;
        }

        for( i = 0; i < psDec->subfr_length; i++ ) {

            ;

            LPC_pred_Q10 = ((psDec->LPC_order)>>(1));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 1 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 0 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 2 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 1 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 3 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 2 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 4 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 3 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 5 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 4 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 6 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 5 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 7 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 6 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 8 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 7 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 9 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 8 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 10 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 9 ]))) >> 16)));
            if( psDec->LPC_order == 16 ) {
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 11 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 10 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 12 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 11 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 13 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 12 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 14 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 13 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 15 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 14 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14[ 16 + i - 16 ]) * (opus_int64)((opus_int16)(A_Q12_tmp[ 15 ]))) >> 16)));
            }


            sLPC_Q14[ 16 + i ] = ((((opus_uint32)(pres_Q14[ i ]) + (opus_uint32)((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) == 0 ? ((((pres_Q14[ i ]) & ((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) != 0 ? ((opus_int32)0x80000000) : (pres_Q14[ i ])+((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) : ((((pres_Q14[ i ]) | ((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) == 0 ? 0x7FFFFFFF : (pres_Q14[ i ])+((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) );


            pxq[ i ] = (opus_int16)((((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14[ 16 + i ]) * (Gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1))));
        }


        memcpy((sLPC_Q14), (&sLPC_Q14[ psDec->subfr_length ]), (16 * sizeof( opus_int32 )));
        pexc_Q14 += psDec->subfr_length;
        pxq += psDec->subfr_length;
    }


    memcpy((psDec->sLPC_Q14_buf), (sLPC_Q14), (16 * sizeof( opus_int32 )));
    ;
}
int silk_decode_frame(
    silk_decoder_state *psDec,
    ec_dec *psRangeDec,
    opus_int16 pOut[],
    opus_int32 *pN,
    int lostFlag,
    int condCoding,
    int arch
)
{
    ;
    int L, mv_len, ret = 0;
    ;

    L = psDec->frame_length;
    silk_decoder_control psDecCtrl[1];
    psDecCtrl->LTP_scale_Q14 = 0;


    ;

    if( lostFlag == 0 ||
        ( lostFlag == 2 && psDec->LBRR_flags[ psDec->nFramesDecoded ] == 1 ) )
    {
        ;
        opus_int16 *pulses = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*((L + 16 - 1) & ~(16 - 1)))
                                                                    ;



        silk_decode_indices( psDec, psRangeDec, psDec->nFramesDecoded, lostFlag, condCoding );




        silk_decode_pulses( psRangeDec, pulses, psDec->indices.signalType,
                psDec->indices.quantOffsetType, psDec->frame_length );




        silk_decode_parameters( psDec, psDecCtrl, condCoding );




        silk_decode_core( psDec, psDecCtrl, pOut, pulses, arch );




        silk_PLC( psDec, psDecCtrl, pOut, 0, arch );

        psDec->lossCnt = 0;
        psDec->prevSignalType = psDec->indices.signalType;
        ;


        psDec->first_frame_after_reset = 0;
    } else {

        silk_PLC( psDec, psDecCtrl, pOut, 1, arch );
    }




    ;
    mv_len = psDec->ltp_mem_length - psDec->frame_length;
    memmove((psDec->outBuf), (&psDec->outBuf[ psDec->frame_length ]), (mv_len * sizeof(opus_int16)));
    memcpy((&psDec->outBuf[ mv_len ]), (pOut), (psDec->frame_length * sizeof( opus_int16 )));




    silk_CNG( psDec, psDecCtrl, pOut, L );




    silk_PLC_glue_frames( psDec, pOut, L );


    psDec->lagPrev = psDecCtrl->pitchL[ psDec->nb_subfr - 1 ];


    *pN = L;

    ;
    return ret;
}
void silk_decode_parameters(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    int condCoding
)
{
    int i, k, Ix;
    opus_int16 pNLSF_Q15[ 16 ], pNLSF0_Q15[ 16 ];
    const opus_int8 *cbk_ptr_Q7;


    silk_gains_dequant( psDecCtrl->Gains_Q16, psDec->indices.GainsIndices,
        &psDec->LastGainIndex, condCoding == 2, psDec->nb_subfr );




    silk_NLSF_decode( pNLSF_Q15, psDec->indices.NLSFIndices, psDec->psNLSF_CB );


    silk_NLSF2A( psDecCtrl->PredCoef_Q12[ 1 ], pNLSF_Q15, psDec->LPC_order, psDec->arch );



    if( psDec->first_frame_after_reset == 1 ) {
        psDec->indices.NLSFInterpCoef_Q2 = 4;
    }

    if( psDec->indices.NLSFInterpCoef_Q2 < 4 ) {


        for( i = 0; i < psDec->LPC_order; i++ ) {
            pNLSF0_Q15[ i ] = psDec->prevNLSF_Q15[ i ] + ((((psDec->indices.NLSFInterpCoef_Q2) * (pNLSF_Q15[ i ] - psDec->prevNLSF_Q15[ i ])))>>(2))
                                                                ;
        }


        silk_NLSF2A( psDecCtrl->PredCoef_Q12[ 0 ], pNLSF0_Q15, psDec->LPC_order, psDec->arch );
    } else {

        memcpy((psDecCtrl->PredCoef_Q12[ 0 ]), (psDecCtrl->PredCoef_Q12[ 1 ]), (psDec->LPC_order * sizeof( opus_int16 )));
    }

    memcpy((psDec->prevNLSF_Q15), (pNLSF_Q15), (psDec->LPC_order * sizeof( opus_int16 )));


    if( psDec->lossCnt ) {
        silk_bwexpander( psDecCtrl->PredCoef_Q12[ 0 ], psDec->LPC_order, 63570 );
        silk_bwexpander( psDecCtrl->PredCoef_Q12[ 1 ], psDec->LPC_order, 63570 );
    }

    if( psDec->indices.signalType == 2 ) {





        silk_decode_pitch( psDec->indices.lagIndex, psDec->indices.contourIndex, psDecCtrl->pitchL, psDec->fs_kHz, psDec->nb_subfr );


        cbk_ptr_Q7 = silk_LTP_vq_ptrs_Q7[ psDec->indices.PERIndex ];

        for( k = 0; k < psDec->nb_subfr; k++ ) {
            Ix = psDec->indices.LTPIndex[ k ];
            for( i = 0; i < 5; i++ ) {
                psDecCtrl->LTPCoef_Q14[ k * 5 + i ] = ((opus_int32)((opus_uint32)(cbk_ptr_Q7[ Ix * 5 + i ])<<(7)));
            }
        }




        Ix = psDec->indices.LTP_scaleIndex;
        psDecCtrl->LTP_scale_Q14 = silk_LTPScales_table_Q14[ Ix ];
    } else {
        memset((psDecCtrl->pitchL), (0), (psDec->nb_subfr * sizeof( int )));
        memset((psDecCtrl->LTPCoef_Q14), (0), (5 * psDec->nb_subfr * sizeof( opus_int16 )));
        psDec->indices.PERIndex = 0;
        psDecCtrl->LTP_scale_Q14 = 0;
    }
}
void silk_decode_indices(
    silk_decoder_state *psDec,
    ec_dec *psRangeDec,
    int FrameIndex,
    int decode_LBRR,
    int condCoding
)
{
    int i, k, Ix;
    int decode_absolute_lagIndex, delta_lagIndex;
    opus_int16 ec_ix[ 16 ];
    opus_uint8 pred_Q8[ 16 ];




    if( decode_LBRR || psDec->VAD_flags[ FrameIndex ] ) {
        Ix = ec_dec_icdf( psRangeDec, silk_type_offset_VAD_iCDF, 8 ) + 2;
    } else {
        Ix = ec_dec_icdf( psRangeDec, silk_type_offset_no_VAD_iCDF, 8 );
    }
    psDec->indices.signalType = (opus_int8)((Ix)>>(1));
    psDec->indices.quantOffsetType = (opus_int8)( Ix & 1 );





    if( condCoding == 2 ) {

        psDec->indices.GainsIndices[ 0 ] = (opus_int8)ec_dec_icdf( psRangeDec, silk_delta_gain_iCDF, 8 );
    } else {

        psDec->indices.GainsIndices[ 0 ] = (opus_int8)((opus_int32)((opus_uint32)(ec_dec_icdf( psRangeDec, silk_gain_iCDF[ psDec->indices.signalType ], 8 ))<<(3)));
        psDec->indices.GainsIndices[ 0 ] += (opus_int8)ec_dec_icdf( psRangeDec, silk_uniform8_iCDF, 8 );
    }


    for( i = 1; i < psDec->nb_subfr; i++ ) {
        psDec->indices.GainsIndices[ i ] = (opus_int8)ec_dec_icdf( psRangeDec, silk_delta_gain_iCDF, 8 );
    }




    psDec->indices.NLSFIndices[ 0 ] = (opus_int8)ec_dec_icdf( psRangeDec, &psDec->psNLSF_CB->CB1_iCDF[ ( psDec->indices.signalType >> 1 ) * psDec->psNLSF_CB->nVectors ], 8 );
    silk_NLSF_unpack( ec_ix, pred_Q8, psDec->psNLSF_CB, psDec->indices.NLSFIndices[ 0 ] );
    ;
    for( i = 0; i < psDec->psNLSF_CB->order; i++ ) {
        Ix = ec_dec_icdf( psRangeDec, &psDec->psNLSF_CB->ec_iCDF[ ec_ix[ i ] ], 8 );
        if( Ix == 0 ) {
            Ix -= ec_dec_icdf( psRangeDec, silk_NLSF_EXT_iCDF, 8 );
        } else if( Ix == 2 * 4 ) {
            Ix += ec_dec_icdf( psRangeDec, silk_NLSF_EXT_iCDF, 8 );
        }
        psDec->indices.NLSFIndices[ i+1 ] = (opus_int8)( Ix - 4 );
    }


    if( psDec->nb_subfr == 4 ) {
        psDec->indices.NLSFInterpCoef_Q2 = (opus_int8)ec_dec_icdf( psRangeDec, silk_NLSF_interpolation_factor_iCDF, 8 );
    } else {
        psDec->indices.NLSFInterpCoef_Q2 = 4;
    }

    if( psDec->indices.signalType == 2 )
    {




        decode_absolute_lagIndex = 1;
        if( condCoding == 2 && psDec->ec_prevSignalType == 2 ) {

            delta_lagIndex = (opus_int16)ec_dec_icdf( psRangeDec, silk_pitch_delta_iCDF, 8 );
            if( delta_lagIndex > 0 ) {
                delta_lagIndex = delta_lagIndex - 9;
                psDec->indices.lagIndex = (opus_int16)( psDec->ec_prevLagIndex + delta_lagIndex );
                decode_absolute_lagIndex = 0;
            }
        }
        if( decode_absolute_lagIndex ) {

            psDec->indices.lagIndex = (opus_int16)ec_dec_icdf( psRangeDec, silk_pitch_lag_iCDF, 8 ) * ((psDec->fs_kHz)>>(1));
            psDec->indices.lagIndex += (opus_int16)ec_dec_icdf( psRangeDec, psDec->pitch_lag_low_bits_iCDF, 8 );
        }
        psDec->ec_prevLagIndex = psDec->indices.lagIndex;


        psDec->indices.contourIndex = (opus_int8)ec_dec_icdf( psRangeDec, psDec->pitch_contour_iCDF, 8 );





        psDec->indices.PERIndex = (opus_int8)ec_dec_icdf( psRangeDec, silk_LTP_per_index_iCDF, 8 );

        for( k = 0; k < psDec->nb_subfr; k++ ) {
            psDec->indices.LTPIndex[ k ] = (opus_int8)ec_dec_icdf( psRangeDec, silk_LTP_gain_iCDF_ptrs[ psDec->indices.PERIndex ], 8 );
        }




        if( condCoding == 0 ) {
            psDec->indices.LTP_scaleIndex = (opus_int8)ec_dec_icdf( psRangeDec, silk_LTPscale_iCDF, 8 );
        } else {
            psDec->indices.LTP_scaleIndex = 0;
        }
    }
    psDec->ec_prevSignalType = psDec->indices.signalType;




    psDec->indices.Seed = (opus_int8)ec_dec_icdf( psRangeDec, silk_uniform4_iCDF, 8 );
}
void silk_decode_pulses(
    ec_dec *psRangeDec,
    opus_int16 pulses[],
    const int signalType,
    const int quantOffsetType,
    const int frame_length
)
{
    int i, j, k, iter, abs_q, nLS, RateLevelIndex;
    int sum_pulses[ ( ( ( 5 * 4 ) * 16 ) / 16 ) ], nLshifts[ ( ( ( 5 * 4 ) * 16 ) / 16 ) ];
    opus_int16 *pulses_ptr;
    const opus_uint8 *cdf_ptr;




    RateLevelIndex = ec_dec_icdf( psRangeDec, silk_rate_levels_iCDF[ signalType >> 1 ], 8 );


    ;
    iter = ((frame_length)>>(4));
    if( iter * 16 < frame_length ) {
        ;
        iter++;
    }




    cdf_ptr = silk_pulses_per_block_iCDF[ RateLevelIndex ];
    for( i = 0; i < iter; i++ ) {
        nLshifts[ i ] = 0;
        sum_pulses[ i ] = ec_dec_icdf( psRangeDec, cdf_ptr, 8 );


        while( sum_pulses[ i ] == 16 + 1 ) {
            nLshifts[ i ]++;

            sum_pulses[ i ] = ec_dec_icdf( psRangeDec,
                    silk_pulses_per_block_iCDF[ 10 - 1] + ( nLshifts[ i ] == 10 ), 8 );
        }
    }




    for( i = 0; i < iter; i++ ) {
        if( sum_pulses[ i ] > 0 ) {
            silk_shell_decoder( &pulses[ ((opus_int32)((opus_int16)(i)) * (opus_int32)((opus_int16)(16))) ], psRangeDec, sum_pulses[ i ] );
        } else {
            memset((&pulses[ ((opus_int32)((opus_int16)(i)) * (opus_int32)((opus_int16)(16))) ]), (0), (16 * sizeof( pulses[0] )));
        }
    }




    for( i = 0; i < iter; i++ ) {
        if( nLshifts[ i ] > 0 ) {
            nLS = nLshifts[ i ];
            pulses_ptr = &pulses[ ((opus_int32)((opus_int16)(i)) * (opus_int32)((opus_int16)(16))) ];
            for( k = 0; k < 16; k++ ) {
                abs_q = pulses_ptr[ k ];
                for( j = 0; j < nLS; j++ ) {
                    abs_q = ((opus_int32)((opus_uint32)(abs_q)<<(1)));
                    abs_q += ec_dec_icdf( psRangeDec, silk_lsb_iCDF, 8 );
                }
                pulses_ptr[ k ] = abs_q;
            }

            sum_pulses[ i ] |= nLS << 5;
        }
    }




    silk_decode_signs( psRangeDec, pulses, frame_length, signalType, quantOffsetType, sum_pulses );
}
extern const opus_int8 silk_CB_lags_stage2[ 4 ][ 11 ];
extern const opus_int8 silk_CB_lags_stage3[ 4 ][ 34 ];
extern const opus_int8 silk_Lag_range_stage3[ 2 + 1 ] [ 4 ][ 2 ];
extern const opus_int8 silk_nb_cbk_searchs_stage3[ 2 + 1 ];


extern const opus_int8 silk_CB_lags_stage2_10_ms[ 4 >> 1][ 3 ];
extern const opus_int8 silk_CB_lags_stage3_10_ms[ 4 >> 1 ][ 12 ];
extern const opus_int8 silk_Lag_range_stage3_10_ms[ 4 >> 1 ][ 2 ];

void silk_decode_pitch(
    opus_int16 lagIndex,
    opus_int8 contourIndex,
    int pitch_lags[],
    const int Fs_kHz,
    const int nb_subfr
)
{
    int lag, k, min_lag, max_lag, cbk_size;
    const opus_int8 *Lag_CB_ptr;

    if( Fs_kHz == 8 ) {
        if( nb_subfr == 4 ) {
            Lag_CB_ptr = &silk_CB_lags_stage2[ 0 ][ 0 ];
            cbk_size = 11;
        } else {
            ;
            Lag_CB_ptr = &silk_CB_lags_stage2_10_ms[ 0 ][ 0 ];
            cbk_size = 3;
        }
    } else {
        if( nb_subfr == 4 ) {
            Lag_CB_ptr = &silk_CB_lags_stage3[ 0 ][ 0 ];
            cbk_size = 34;
        } else {
            ;
            Lag_CB_ptr = &silk_CB_lags_stage3_10_ms[ 0 ][ 0 ];
            cbk_size = 12;
        }
    }

    min_lag = ((opus_int32)((opus_int16)(2)) * (opus_int32)((opus_int16)(Fs_kHz)));
    max_lag = ((opus_int32)((opus_int16)(18)) * (opus_int32)((opus_int16)(Fs_kHz)));
    lag = min_lag + lagIndex;

    for( k = 0; k < nb_subfr; k++ ) {
        pitch_lags[ k ] = lag + (*((Lag_CB_ptr) + ((k)*(cbk_size)+(contourIndex))));
        pitch_lags[ k ] = ((min_lag) > (max_lag) ? ((pitch_lags[ k ]) > (min_lag) ? (min_lag) : ((pitch_lags[ k ]) < (max_lag) ? (max_lag) : (pitch_lags[ k ]))) : ((pitch_lags[ k ]) > (max_lag) ? (max_lag) : ((pitch_lags[ k ]) < (min_lag) ? (min_lag) : (pitch_lags[ k ]))));
    }
}
int silk_decoder_set_fs(
    silk_decoder_state *psDec,
    int fs_kHz,
    opus_int32 fs_API_Hz
)
{
    int frame_length, ret = 0;

    ;
    ;


    psDec->subfr_length = ((opus_int32)((opus_int16)(5)) * (opus_int32)((opus_int16)(fs_kHz)));
    frame_length = ((opus_int32)((opus_int16)(psDec->nb_subfr)) * (opus_int32)((opus_int16)(psDec->subfr_length)));


    if( psDec->fs_kHz != fs_kHz || psDec->fs_API_hz != fs_API_Hz ) {

        ret += silk_resampler_init( &psDec->resampler_state, ((opus_int32)((opus_int16)(fs_kHz)) * (opus_int32)((opus_int16)(1000))), fs_API_Hz, 0 );

        psDec->fs_API_hz = fs_API_Hz;
    }

    if( psDec->fs_kHz != fs_kHz || frame_length != psDec->frame_length ) {
        if( fs_kHz == 8 ) {
            if( psDec->nb_subfr == 4 ) {
                psDec->pitch_contour_iCDF = silk_pitch_contour_NB_iCDF;
            } else {
                psDec->pitch_contour_iCDF = silk_pitch_contour_10_ms_NB_iCDF;
            }
        } else {
            if( psDec->nb_subfr == 4 ) {
                psDec->pitch_contour_iCDF = silk_pitch_contour_iCDF;
            } else {
                psDec->pitch_contour_iCDF = silk_pitch_contour_10_ms_iCDF;
            }
        }
        if( psDec->fs_kHz != fs_kHz ) {
            psDec->ltp_mem_length = ((opus_int32)((opus_int16)(20)) * (opus_int32)((opus_int16)(fs_kHz)));
            if( fs_kHz == 8 || fs_kHz == 12 ) {
                psDec->LPC_order = 10;
                psDec->psNLSF_CB = &silk_NLSF_CB_NB_MB;
            } else {
                psDec->LPC_order = 16;
                psDec->psNLSF_CB = &silk_NLSF_CB_WB;
            }
            if( fs_kHz == 16 ) {
                psDec->pitch_lag_low_bits_iCDF = silk_uniform8_iCDF;
            } else if( fs_kHz == 12 ) {
                psDec->pitch_lag_low_bits_iCDF = silk_uniform6_iCDF;
            } else if( fs_kHz == 8 ) {
                psDec->pitch_lag_low_bits_iCDF = silk_uniform4_iCDF;
            } else {

                ;
            }
            psDec->first_frame_after_reset = 1;
            psDec->lagPrev = 100;
            psDec->LastGainIndex = 10;
            psDec->prevSignalType = 0;
            memset((psDec->outBuf), (0), (sizeof(psDec->outBuf)));
            memset((psDec->sLPC_Q14_buf), (0), (sizeof(psDec->sLPC_Q14_buf)));
        }

        psDec->fs_kHz = fs_kHz;
        psDec->frame_length = frame_length;
    }


    ;

    return ret;
}
typedef struct {
    int VADFlag;
    int VADFlags[ 3 ];
    int inbandFECFlag;
} silk_TOC_struct;
int silk_Get_Encoder_Size(
    int *encSizeBytes
);




int silk_InitEncoder(
    void *encState,
    int arch,
    silk_EncControlStruct *encStatus
);






int silk_Encode(
    void *encState,
    silk_EncControlStruct *encControl,
    const opus_int16 *samplesIn,
    int nSamplesIn,
    ec_enc *psRangeEnc,
    opus_int32 *nBytesOut,
    const int prefillFlag,
    int activity
);
int silk_Get_Decoder_Size(
    int *decSizeBytes
);




int silk_InitDecoder(
    void *decState
);




int silk_Decode(
    void* decState,
    silk_DecControlStruct* decControl,
    int lostFlag,
    int newPacketFlag,
    ec_dec *psRangeDec,
    opus_int16 *samplesOut,
    opus_int32 *nSamplesOut,
    int arch
);






typedef struct {
    silk_decoder_state channel_state[ 2 ];
    stereo_dec_state sStereo;
    int nChannelsAPI;
    int nChannelsInternal;
    int prev_decode_only_middle;
} silk_decoder;





int silk_Get_Decoder_Size(
    int *decSizeBytes
)
{
    int ret = 0;

    *decSizeBytes = sizeof( silk_decoder );

    return ret;
}


int silk_InitDecoder(
    void *decState
)
{
    int n, ret = 0;
    silk_decoder_state *channel_state = ((silk_decoder *)decState)->channel_state;

    for( n = 0; n < 2; n++ ) {
        ret = silk_init_decoder( &channel_state[ n ] );
    }
    memset((&((silk_decoder *)decState)->sStereo), (0), (sizeof(((silk_decoder *)decState)->sStereo)));

    ((silk_decoder *)decState)->prev_decode_only_middle = 0;

    return ret;
}


int silk_Decode(
    void* decState,
    silk_DecControlStruct* decControl,
    int lostFlag,
    int newPacketFlag,
    ec_dec *psRangeDec,
    opus_int16 *samplesOut,
    opus_int32 *nSamplesOut,
    int arch
)
{
    int i, n, decode_only_middle = 0, ret = 0;
    opus_int32 nSamplesOutDec, LBRR_symbol;
    opus_int16 *samplesOut1_tmp[ 2 ];
    ;
    ;
    ;
    opus_int32 MS_pred_Q13[ 2 ] = { 0 };
    opus_int16 *resample_out_ptr;
    silk_decoder *psDec = ( silk_decoder * )decState;
    silk_decoder_state *channel_state = psDec->channel_state;
    int has_side;
    int stereo_to_mono;
    int delay_stack_alloc;
    ;

    ;




    if( newPacketFlag ) {
        for( n = 0; n < decControl->nChannelsInternal; n++ ) {
            channel_state[ n ].nFramesDecoded = 0;
        }
    }


    if( decControl->nChannelsInternal > psDec->nChannelsInternal ) {
        ret += silk_init_decoder( &channel_state[ 1 ] );
    }

    stereo_to_mono = decControl->nChannelsInternal == 1 && psDec->nChannelsInternal == 2 &&
                     ( decControl->internalSampleRate == 1000*channel_state[ 0 ].fs_kHz );

    if( channel_state[ 0 ].nFramesDecoded == 0 ) {
        for( n = 0; n < decControl->nChannelsInternal; n++ ) {
            int fs_kHz_dec;
            if( decControl->payloadSize_ms == 0 ) {

                channel_state[ n ].nFramesPerPacket = 1;
                channel_state[ n ].nb_subfr = 2;
            } else if( decControl->payloadSize_ms == 10 ) {
                channel_state[ n ].nFramesPerPacket = 1;
                channel_state[ n ].nb_subfr = 2;
            } else if( decControl->payloadSize_ms == 20 ) {
                channel_state[ n ].nFramesPerPacket = 1;
                channel_state[ n ].nb_subfr = 4;
            } else if( decControl->payloadSize_ms == 40 ) {
                channel_state[ n ].nFramesPerPacket = 2;
                channel_state[ n ].nb_subfr = 4;
            } else if( decControl->payloadSize_ms == 60 ) {
                channel_state[ n ].nFramesPerPacket = 3;
                channel_state[ n ].nb_subfr = 4;
            } else {
                ;
                ;
                return -203;
            }
            fs_kHz_dec = ( decControl->internalSampleRate >> 10 ) + 1;
            if( fs_kHz_dec != 8 && fs_kHz_dec != 12 && fs_kHz_dec != 16 ) {
                ;
                ;
                return -200;
            }
            ret += silk_decoder_set_fs( &channel_state[ n ], fs_kHz_dec, decControl->API_sampleRate );
        }
    }

    if( decControl->nChannelsAPI == 2 && decControl->nChannelsInternal == 2 && ( psDec->nChannelsAPI == 1 || psDec->nChannelsInternal == 1 ) ) {
        memset((psDec->sStereo.pred_prev_Q13), (0), (sizeof( psDec->sStereo.pred_prev_Q13 )));
        memset((psDec->sStereo.sSide), (0), (sizeof( psDec->sStereo.sSide )));
        memcpy((&channel_state[ 1 ].resampler_state), (&channel_state[ 0 ].resampler_state), (sizeof( silk_resampler_state_struct )));
    }
    psDec->nChannelsAPI = decControl->nChannelsAPI;
    psDec->nChannelsInternal = decControl->nChannelsInternal;

    if( decControl->API_sampleRate > (opus_int32)48 * 1000 || decControl->API_sampleRate < 8000 ) {
        ret = -200;
        ;
        return( ret );
    }

    if( lostFlag != 1 && channel_state[ 0 ].nFramesDecoded == 0 ) {


        for( n = 0; n < decControl->nChannelsInternal; n++ ) {
            for( i = 0; i < channel_state[ n ].nFramesPerPacket; i++ ) {
                channel_state[ n ].VAD_flags[ i ] = ec_dec_bit_logp(psRangeDec, 1);
            }
            channel_state[ n ].LBRR_flag = ec_dec_bit_logp(psRangeDec, 1);
        }

        for( n = 0; n < decControl->nChannelsInternal; n++ ) {
            memset((channel_state[ n ].LBRR_flags), (0), (sizeof( channel_state[ n ].LBRR_flags )));
            if( channel_state[ n ].LBRR_flag ) {
                if( channel_state[ n ].nFramesPerPacket == 1 ) {
                    channel_state[ n ].LBRR_flags[ 0 ] = 1;
                } else {
                    LBRR_symbol = ec_dec_icdf( psRangeDec, silk_LBRR_flags_iCDF_ptr[ channel_state[ n ].nFramesPerPacket - 2 ], 8 ) + 1;
                    for( i = 0; i < channel_state[ n ].nFramesPerPacket; i++ ) {
                        channel_state[ n ].LBRR_flags[ i ] = ((LBRR_symbol)>>(i)) & 1;
                    }
                }
            }
        }

        if( lostFlag == 0 ) {

            for( i = 0; i < channel_state[ 0 ].nFramesPerPacket; i++ ) {
                for( n = 0; n < decControl->nChannelsInternal; n++ ) {
                    if( channel_state[ n ].LBRR_flags[ i ] ) {
                        opus_int16 pulses[ ( ( 5 * 4 ) * 16 ) ];
                        int condCoding;

                        if( decControl->nChannelsInternal == 2 && n == 0 ) {
                            silk_stereo_decode_pred( psRangeDec, MS_pred_Q13 );
                            if( channel_state[ 1 ].LBRR_flags[ i ] == 0 ) {
                                silk_stereo_decode_mid_only( psRangeDec, &decode_only_middle );
                            }
                        }

                        if( i > 0 && channel_state[ n ].LBRR_flags[ i - 1 ] ) {
                            condCoding = 2;
                        } else {
                            condCoding = 0;
                        }
                        silk_decode_indices( &channel_state[ n ], psRangeDec, i, 1, condCoding );
                        silk_decode_pulses( psRangeDec, pulses, channel_state[ n ].indices.signalType,
                            channel_state[ n ].indices.quantOffsetType, channel_state[ n ].frame_length );
                    }
                }
            }
        }
    }


    if( decControl->nChannelsInternal == 2 ) {
        if( lostFlag == 0 ||
            ( lostFlag == 2 && channel_state[ 0 ].LBRR_flags[ channel_state[ 0 ].nFramesDecoded ] == 1 ) )
        {
            silk_stereo_decode_pred( psRangeDec, MS_pred_Q13 );

            if( ( lostFlag == 0 && channel_state[ 1 ].VAD_flags[ channel_state[ 0 ].nFramesDecoded ] == 0 ) ||
                ( lostFlag == 2 && channel_state[ 1 ].LBRR_flags[ channel_state[ 0 ].nFramesDecoded ] == 0 ) )
            {
                silk_stereo_decode_mid_only( psRangeDec, &decode_only_middle );
            } else {
                decode_only_middle = 0;
            }
        } else {
            for( n = 0; n < 2; n++ ) {
                MS_pred_Q13[ n ] = psDec->sStereo.pred_prev_Q13[ n ];
            }
        }
    }


    if( decControl->nChannelsInternal == 2 && decode_only_middle == 0 && psDec->prev_decode_only_middle == 1 ) {
        memset((psDec->channel_state[ 1 ].outBuf), (0), (sizeof(psDec->channel_state[ 1 ].outBuf)));
        memset((psDec->channel_state[ 1 ].sLPC_Q14_buf), (0), (sizeof(psDec->channel_state[ 1 ].sLPC_Q14_buf)));
        psDec->channel_state[ 1 ].lagPrev = 100;
        psDec->channel_state[ 1 ].LastGainIndex = 10;
        psDec->channel_state[ 1 ].prevSignalType = 0;
        psDec->channel_state[ 1 ].first_frame_after_reset = 1;
    }




    delay_stack_alloc = decControl->internalSampleRate*decControl->nChannelsInternal
          < decControl->API_sampleRate*decControl->nChannelsAPI;
    opus_int16 *samplesOut1_tmp_storage1 = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*(delay_stack_alloc ? 1 : decControl->nChannelsInternal*(channel_state[ 0 ].frame_length + 2 )))

                       ;
    if ( delay_stack_alloc )
    {
       samplesOut1_tmp[ 0 ] = samplesOut;
       samplesOut1_tmp[ 1 ] = samplesOut + channel_state[ 0 ].frame_length + 2;
    } else {
       samplesOut1_tmp[ 0 ] = samplesOut1_tmp_storage1;
       samplesOut1_tmp[ 1 ] = samplesOut1_tmp_storage1 + channel_state[ 0 ].frame_length + 2;
    }

    if( lostFlag == 0 ) {
        has_side = !decode_only_middle;
    } else {
        has_side = !psDec->prev_decode_only_middle
              || (decControl->nChannelsInternal == 2 && lostFlag == 2 && channel_state[1].LBRR_flags[ channel_state[1].nFramesDecoded ] == 1 );
    }

    for( n = 0; n < decControl->nChannelsInternal; n++ ) {
        if( n == 0 || has_side ) {
            int FrameIndex;
            int condCoding;

            FrameIndex = channel_state[ 0 ].nFramesDecoded - n;

            if( FrameIndex <= 0 ) {
                condCoding = 0;
            } else if( lostFlag == 2 ) {
                condCoding = channel_state[ n ].LBRR_flags[ FrameIndex - 1 ] ? 2 : 0;
            } else if( n > 0 && psDec->prev_decode_only_middle ) {


                condCoding = 1;
            } else {
                condCoding = 2;
            }
            ret += silk_decode_frame( &channel_state[ n ], psRangeDec, &samplesOut1_tmp[ n ][ 2 ], &nSamplesOutDec, lostFlag, condCoding, arch);
        } else {
            memset((&samplesOut1_tmp[ n ][ 2 ]), (0), (nSamplesOutDec * sizeof( opus_int16 )));
        }
        channel_state[ n ].nFramesDecoded++;
    }

    if( decControl->nChannelsAPI == 2 && decControl->nChannelsInternal == 2 ) {

        silk_stereo_MS_to_LR( &psDec->sStereo, samplesOut1_tmp[ 0 ], samplesOut1_tmp[ 1 ], MS_pred_Q13, channel_state[ 0 ].fs_kHz, nSamplesOutDec );
    } else {

        memcpy((samplesOut1_tmp[ 0 ]), (psDec->sStereo.sMid), (2 * sizeof( opus_int16 )));
        memcpy((psDec->sStereo.sMid), (&samplesOut1_tmp[ 0 ][ nSamplesOutDec ]), (2 * sizeof( opus_int16 )));
    }


    *nSamplesOut = ((opus_int32)((nSamplesOutDec * decControl->API_sampleRate) / (((opus_int32)((opus_int16)(channel_state[ 0 ].fs_kHz)) * (opus_int32)((opus_int16)(1000))))));


    opus_int16 *samplesOut2_tmp = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*(decControl->nChannelsAPI == 2 ? *nSamplesOut : 1))
                                                                                  ;
    if( decControl->nChannelsAPI == 2 ) {
        resample_out_ptr = samplesOut2_tmp;
    } else {
        resample_out_ptr = samplesOut;
    }

    opus_int16 *samplesOut1_tmp_storage2 = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*(delay_stack_alloc ? decControl->nChannelsInternal*(channel_state[ 0 ].frame_length + 2 ) : 1))


                       ;
    if ( delay_stack_alloc ) {
       (memcpy((samplesOut1_tmp_storage2), (samplesOut), (decControl->nChannelsInternal*(channel_state[ 0 ].frame_length + 2))*sizeof(*(samplesOut1_tmp_storage2)) + 0*((samplesOut1_tmp_storage2)-(samplesOut)) ));
       samplesOut1_tmp[ 0 ] = samplesOut1_tmp_storage2;
       samplesOut1_tmp[ 1 ] = samplesOut1_tmp_storage2 + channel_state[ 0 ].frame_length + 2;
    }
    for( n = 0; n < (((decControl->nChannelsAPI) < (decControl->nChannelsInternal)) ? (decControl->nChannelsAPI) : (decControl->nChannelsInternal)); n++ ) {


        ret += silk_resampler( &channel_state[ n ].resampler_state, resample_out_ptr, &samplesOut1_tmp[ n ][ 1 ], nSamplesOutDec );


        if( decControl->nChannelsAPI == 2 ) {
            for( i = 0; i < *nSamplesOut; i++ ) {
                samplesOut[ n + 2 * i ] = resample_out_ptr[ i ];
            }
        }
    }


    if( decControl->nChannelsAPI == 2 && decControl->nChannelsInternal == 1 ) {
        if ( stereo_to_mono ){


            ret += silk_resampler( &channel_state[ 1 ].resampler_state, resample_out_ptr, &samplesOut1_tmp[ 0 ][ 1 ], nSamplesOutDec );

            for( i = 0; i < *nSamplesOut; i++ ) {
                samplesOut[ 1 + 2 * i ] = resample_out_ptr[ i ];
            }
        } else {
            for( i = 0; i < *nSamplesOut; i++ ) {
                samplesOut[ 1 + 2 * i ] = samplesOut[ 0 + 2 * i ];
            }
        }
    }


    if( channel_state[ 0 ].prevSignalType == 2 ) {
        int mult_tab[ 3 ] = { 6, 4, 3 };
        decControl->prevPitchLag = channel_state[ 0 ].lagPrev * mult_tab[ ( channel_state[ 0 ].fs_kHz - 8 ) >> 2 ];
    } else {
        decControl->prevPitchLag = 0;
    }

    if( lostFlag == 1 ) {


       for ( i = 0; i < psDec->nChannelsInternal; i++ )
          psDec->channel_state[ i ].LastGainIndex = 10;
    } else {
       psDec->prev_decode_only_middle = decode_only_middle;
    }
    ;
    return ret;
}
static inline void silk_CNG_exc(
    opus_int32 exc_Q14[],
    opus_int32 exc_buf_Q14[],
    int length,
    opus_int32 *rand_seed
)
{
    opus_int32 seed;
    int i, idx, exc_mask;

    exc_mask = 255;
    while( exc_mask > length ) {
        exc_mask = ((exc_mask)>>(1));
    }

    seed = *rand_seed;
    for( i = 0; i < length; i++ ) {
        seed = (((opus_int32)((opus_uint32)(((907633515))) + (opus_uint32)((opus_uint32)((seed)) * (opus_uint32)((196314165))))));
        idx = (int)( ((seed)>>(24)) & exc_mask );
        ;
        ;
        exc_Q14[ i ] = exc_buf_Q14[ idx ];
    }
    *rand_seed = seed;
}

void silk_CNG_Reset(
    silk_decoder_state *psDec
)
{
    int i, NLSF_step_Q15, NLSF_acc_Q15;

    NLSF_step_Q15 = ((opus_int32)((0x7FFF) / (psDec->LPC_order + 1)));
    NLSF_acc_Q15 = 0;
    for( i = 0; i < psDec->LPC_order; i++ ) {
        NLSF_acc_Q15 += NLSF_step_Q15;
        psDec->sCNG.CNG_smth_NLSF_Q15[ i ] = NLSF_acc_Q15;
    }
    psDec->sCNG.CNG_smth_Gain_Q16 = 0;
    psDec->sCNG.rand_seed = 3176576;
}


void silk_CNG(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    opus_int16 frame[],
    int length
)
{
    int i, subfr;
    opus_int32 LPC_pred_Q10, max_Gain_Q16, gain_Q16, gain_Q10;
    opus_int16 A_Q12[ 16 ];
    silk_CNG_struct *psCNG = &psDec->sCNG;
    ;

    if( psDec->fs_kHz != psCNG->fs_kHz ) {

        silk_CNG_Reset( psDec );

        psCNG->fs_kHz = psDec->fs_kHz;
    }
    if( psDec->lossCnt == 0 && psDec->prevSignalType == 0 ) {



        for( i = 0; i < psDec->LPC_order; i++ ) {
            psCNG->CNG_smth_NLSF_Q15[ i ] += ((opus_int32)((((opus_int32)psDec->prevNLSF_Q15[ i ] - (opus_int32)psCNG->CNG_smth_NLSF_Q15[ i ]) * (opus_int64)((opus_int16)(16348))) >> 16));
        }

        max_Gain_Q16 = 0;
        subfr = 0;
        for( i = 0; i < psDec->nb_subfr; i++ ) {
            if( psDecCtrl->Gains_Q16[ i ] > max_Gain_Q16 ) {
                max_Gain_Q16 = psDecCtrl->Gains_Q16[ i ];
                subfr = i;
            }
        }

        memmove((&psCNG->CNG_exc_buf_Q14[ psDec->subfr_length ]), (psCNG->CNG_exc_buf_Q14), (( psDec->nb_subfr - 1 ) * psDec->subfr_length * sizeof( opus_int32 )));
        memcpy((psCNG->CNG_exc_buf_Q14), (&psDec->exc_Q14[ subfr * psDec->subfr_length ]), (psDec->subfr_length * sizeof( opus_int32 )));


        for( i = 0; i < psDec->nb_subfr; i++ ) {
            psCNG->CNG_smth_Gain_Q16 += ((opus_int32)(((psDecCtrl->Gains_Q16[ i ] - psCNG->CNG_smth_Gain_Q16) * (opus_int64)((opus_int16)(4634))) >> 16));

            if( ((opus_int32)(((opus_int64)(psCNG->CNG_smth_Gain_Q16) * (46396)) >> 16)) > psDecCtrl->Gains_Q16[ i ] ) {
                psCNG->CNG_smth_Gain_Q16 = psDecCtrl->Gains_Q16[ i ];
            }
        }
    }


    if( psDec->lossCnt ) {
        ;
        opus_int32 *CNG_sig_Q14 = (opus_int32*)DR_OPUS_ALLOCA(sizeof(opus_int32)*(length + 16));


        gain_Q16 = ((opus_int32)(((opus_int64)(psDec->sPLC.randScale_Q14) * (psDec->sPLC.prevGain_Q16[1])) >> 16));
        if( gain_Q16 >= (1 << 21) || psCNG->CNG_smth_Gain_Q16 > (1 << 23) ) {
            gain_Q16 = (((gain_Q16) >> 16) * ((gain_Q16) >> 16));
            gain_Q16 = ((((((psCNG->CNG_smth_Gain_Q16) >> 16) * ((psCNG->CNG_smth_Gain_Q16) >> 16)))) - (((opus_int32)((opus_uint32)((gain_Q16))<<((5))))));
            gain_Q16 = ((opus_int32)((opus_uint32)(silk_SQRT_APPROX( gain_Q16 ))<<(16)));
        } else {
            gain_Q16 = ((opus_int32)(((opus_int64)(gain_Q16) * (gain_Q16)) >> 16));
            gain_Q16 = (((((opus_int32)(((opus_int64)(psCNG->CNG_smth_Gain_Q16) * (psCNG->CNG_smth_Gain_Q16)) >> 16)))) - (((opus_int32)((opus_uint32)((gain_Q16))<<((5))))));
            gain_Q16 = ((opus_int32)((opus_uint32)(silk_SQRT_APPROX( gain_Q16 ))<<(8)));
        }
        gain_Q10 = ((gain_Q16)>>(6));

        silk_CNG_exc( CNG_sig_Q14 + 16, psCNG->CNG_exc_buf_Q14, length, &psCNG->rand_seed );


        silk_NLSF2A( A_Q12, psCNG->CNG_smth_NLSF_Q15, psDec->LPC_order, psDec->arch );


        memcpy((CNG_sig_Q14), (psCNG->CNG_synth_state), (16 * sizeof( opus_int32 )));
        ;
        for( i = 0; i < length; i++ ) {

            LPC_pred_Q10 = ((psDec->LPC_order)>>(1));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 1 ]) * (opus_int64)((opus_int16)(A_Q12[ 0 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 2 ]) * (opus_int64)((opus_int16)(A_Q12[ 1 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 3 ]) * (opus_int64)((opus_int16)(A_Q12[ 2 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 4 ]) * (opus_int64)((opus_int16)(A_Q12[ 3 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 5 ]) * (opus_int64)((opus_int16)(A_Q12[ 4 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 6 ]) * (opus_int64)((opus_int16)(A_Q12[ 5 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 7 ]) * (opus_int64)((opus_int16)(A_Q12[ 6 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 8 ]) * (opus_int64)((opus_int16)(A_Q12[ 7 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 9 ]) * (opus_int64)((opus_int16)(A_Q12[ 8 ]))) >> 16)));
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 10 ]) * (opus_int64)((opus_int16)(A_Q12[ 9 ]))) >> 16)));
            if( psDec->LPC_order == 16 ) {
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 11 ]) * (opus_int64)((opus_int16)(A_Q12[ 10 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 12 ]) * (opus_int64)((opus_int16)(A_Q12[ 11 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 13 ]) * (opus_int64)((opus_int16)(A_Q12[ 12 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 14 ]) * (opus_int64)((opus_int16)(A_Q12[ 13 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 15 ]) * (opus_int64)((opus_int16)(A_Q12[ 14 ]))) >> 16)));
                LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((CNG_sig_Q14[ 16 + i - 16 ]) * (opus_int64)((opus_int16)(A_Q12[ 15 ]))) >> 16)));
            }


            CNG_sig_Q14[ 16 + i ] = ((((opus_uint32)(CNG_sig_Q14[ 16 + i ]) + (opus_uint32)((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) == 0 ? ((((CNG_sig_Q14[ 16 + i ]) & ((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) != 0 ? ((opus_int32)0x80000000) : (CNG_sig_Q14[ 16 + i ])+((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) : ((((CNG_sig_Q14[ 16 + i ]) | ((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) == 0 ? 0x7FFFFFFF : (CNG_sig_Q14[ 16 + i ])+((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) );


            frame[ i ] = (opus_int16)(opus_int16)(((((opus_int32)(frame[ i ])) + ((((((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)))))))) > 0x7FFF ? 0x7FFF : (((((opus_int32)(frame[ i ])) + ((((((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)))))))) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : ((((opus_int32)(frame[ i ])) + ((((((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(CNG_sig_Q14[ 16 + i ]) * (gain_Q10)) >> 16))) >> ((8) - 1)) + 1) >> 1))))))))));

        }
        memcpy((psCNG->CNG_synth_state), (&CNG_sig_Q14[ length ]), (16 * sizeof( opus_int32 )));
    } else {
        memset((psCNG->CNG_synth_state), (0), (psDec->LPC_order * sizeof( opus_int32 )));
    }
    ;
}
void silk_encode_signs(
    ec_enc *psRangeEnc,
    const opus_int8 pulses[],
    int length,
    const int signalType,
    const int quantOffsetType,
    const int sum_pulses[ ( ( ( 5 * 4 ) * 16 ) / 16 ) ]
)
{
    int i, j, p;
    opus_uint8 icdf[ 2 ];
    const opus_int8 *q_ptr;
    const opus_uint8 *icdf_ptr;

    icdf[ 1 ] = 0;
    q_ptr = pulses;
    i = ((opus_int32)((opus_int16)(7)) * (opus_int32)((opus_int16)(((quantOffsetType) + ((opus_int32)((opus_uint32)((signalType))<<((1))))))));
    icdf_ptr = &silk_sign_iCDF[ i ];
    length = ((length + 16/2)>>(4));
    for( i = 0; i < length; i++ ) {
        p = sum_pulses[ i ];
        if( p > 0 ) {
            icdf[ 0 ] = icdf_ptr[ (((p & 0x1F) < (6)) ? (p & 0x1F) : (6)) ];
            for( j = 0; j < 16; j++ ) {
                if( q_ptr[ j ] != 0 ) {
                    ec_enc_icdf( psRangeEnc, ( (((q_ptr[ j ]))>>(15)) + 1 ), icdf, 8 );
                }
            }
        }
        q_ptr += 16;
    }
}


void silk_decode_signs(
    ec_dec *psRangeDec,
    opus_int16 pulses[],
    int length,
    const int signalType,
    const int quantOffsetType,
    const int sum_pulses[ ( ( ( 5 * 4 ) * 16 ) / 16 ) ]
)
{
    int i, j, p;
    opus_uint8 icdf[ 2 ];
    opus_int16 *q_ptr;
    const opus_uint8 *icdf_ptr;

    icdf[ 1 ] = 0;
    q_ptr = pulses;
    i = ((opus_int32)((opus_int16)(7)) * (opus_int32)((opus_int16)(((quantOffsetType) + ((opus_int32)((opus_uint32)((signalType))<<((1))))))));
    icdf_ptr = &silk_sign_iCDF[ i ];
    length = ((length + 16/2)>>(4));
    for( i = 0; i < length; i++ ) {
        p = sum_pulses[ i ];
        if( p > 0 ) {
            icdf[ 0 ] = icdf_ptr[ (((p & 0x1F) < (6)) ? (p & 0x1F) : (6)) ];
            for( j = 0; j < 16; j++ ) {
                if( q_ptr[ j ] > 0 ) {
                    q_ptr[ j ] *= ( ((opus_int32)((opus_uint32)((ec_dec_icdf( psRangeDec, icdf, 8 )))<<(1))) - 1 );

                }
            }
        }
        q_ptr += 16;
    }
}
static const opus_int16 HARM_ATT_Q15[2] = { 32440, 31130 };
static const opus_int16 PLC_RAND_ATTENUATE_V_Q15[2] = { 31130, 26214 };
static const opus_int16 PLC_RAND_ATTENUATE_UV_Q15[2] = { 32440, 29491 };

static inline void silk_PLC_update(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl
);

static inline void silk_PLC_conceal(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    opus_int16 frame[],
    int arch
);


void silk_PLC_Reset(
    silk_decoder_state *psDec
)
{
    psDec->sPLC.pitchL_Q8 = ((opus_int32)((opus_uint32)(psDec->frame_length)<<(8 - 1)));
    psDec->sPLC.prevGain_Q16[ 0 ] = ((opus_int32)((1) * ((opus_int64)1 << (16)) + 0.5));
    psDec->sPLC.prevGain_Q16[ 1 ] = ((opus_int32)((1) * ((opus_int64)1 << (16)) + 0.5));
    psDec->sPLC.subfr_length = 20;
    psDec->sPLC.nb_subfr = 2;
}

void silk_PLC(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    opus_int16 frame[],
    int lost,
    int arch
)
{

    if( psDec->fs_kHz != psDec->sPLC.fs_kHz ) {
        silk_PLC_Reset( psDec );
        psDec->sPLC.fs_kHz = psDec->fs_kHz;
    }

    if( lost ) {



        silk_PLC_conceal( psDec, psDecCtrl, frame, arch );

        psDec->lossCnt++;
    } else {



        silk_PLC_update( psDec, psDecCtrl );
    }
}




static inline void silk_PLC_update(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl
)
{
    opus_int32 LTP_Gain_Q14, temp_LTP_Gain_Q14;
    int i, j;
    silk_PLC_struct *psPLC;

    psPLC = &psDec->sPLC;


    psDec->prevSignalType = psDec->indices.signalType;
    LTP_Gain_Q14 = 0;
    if( psDec->indices.signalType == 2 ) {

        for( j = 0; j * psDec->subfr_length < psDecCtrl->pitchL[ psDec->nb_subfr - 1 ]; j++ ) {
            if( j == psDec->nb_subfr ) {
                break;
            }
            temp_LTP_Gain_Q14 = 0;
            for( i = 0; i < 5; i++ ) {
                temp_LTP_Gain_Q14 += psDecCtrl->LTPCoef_Q14[ ( psDec->nb_subfr - 1 - j ) * 5 + i ];
            }
            if( temp_LTP_Gain_Q14 > LTP_Gain_Q14 ) {
                LTP_Gain_Q14 = temp_LTP_Gain_Q14;
                memcpy((psPLC->LTPCoef_Q14), (&psDecCtrl->LTPCoef_Q14[ ((opus_int32)((opus_int16)(psDec->nb_subfr - 1 - j)) * (opus_int32)((opus_int16)(5))) ]), (5 * sizeof( opus_int16 )))

                                                      ;

                psPLC->pitchL_Q8 = ((opus_int32)((opus_uint32)(psDecCtrl->pitchL[ psDec->nb_subfr - 1 - j ])<<(8)));
            }
        }

        memset((psPLC->LTPCoef_Q14), (0), (5 * sizeof( opus_int16 )));
        psPLC->LTPCoef_Q14[ 5 / 2 ] = LTP_Gain_Q14;


        if( LTP_Gain_Q14 < 11469 ) {
            int scale_Q10;
            opus_int32 tmp;

            tmp = ((opus_int32)((opus_uint32)(11469)<<(10)));
            scale_Q10 = ((opus_int32)((tmp) / ((((LTP_Gain_Q14) > (1)) ? (LTP_Gain_Q14) : (1)))));
            for( i = 0; i < 5; i++ ) {
                psPLC->LTPCoef_Q14[ i ] = ((((opus_int32)((opus_int16)(psPLC->LTPCoef_Q14[ i ])) * (opus_int32)((opus_int16)(scale_Q10))))>>(10));
            }
        } else if( LTP_Gain_Q14 > 15565 ) {
            int scale_Q14;
            opus_int32 tmp;

            tmp = ((opus_int32)((opus_uint32)(15565)<<(14)));
            scale_Q14 = ((opus_int32)((tmp) / ((((LTP_Gain_Q14) > (1)) ? (LTP_Gain_Q14) : (1)))));
            for( i = 0; i < 5; i++ ) {
                psPLC->LTPCoef_Q14[ i ] = ((((opus_int32)((opus_int16)(psPLC->LTPCoef_Q14[ i ])) * (opus_int32)((opus_int16)(scale_Q14))))>>(14));
            }
        }
    } else {
        psPLC->pitchL_Q8 = ((opus_int32)((opus_uint32)(((opus_int32)((opus_int16)(psDec->fs_kHz)) * (opus_int32)((opus_int16)(18))))<<(8)));
        memset((psPLC->LTPCoef_Q14), (0), (5 * sizeof( opus_int16 )));
    }


    memcpy((psPLC->prevLPC_Q12), (psDecCtrl->PredCoef_Q12[ 1 ]), (psDec->LPC_order * sizeof( opus_int16 )));
    psPLC->prevLTP_scale_Q14 = psDecCtrl->LTP_scale_Q14;


    memcpy((psPLC->prevGain_Q16), (&psDecCtrl->Gains_Q16[ psDec->nb_subfr - 2 ]), (2 * sizeof( opus_int32 )));

    psPLC->subfr_length = psDec->subfr_length;
    psPLC->nb_subfr = psDec->nb_subfr;
}

static inline void silk_PLC_energy(opus_int32 *energy1, int *shift1, opus_int32 *energy2, int *shift2,
      const opus_int32 *exc_Q14, const opus_int32 *prevGain_Q10, int subfr_length, int nb_subfr)
{
    int i, k;
    ;
    opus_int16 *exc_buf_ptr;
    ;
    opus_int16 *exc_buf = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*(2*subfr_length));


    exc_buf_ptr = exc_buf;
    for( k = 0; k < 2; k++ ) {
        for( i = 0; i < subfr_length; i++ ) {
            exc_buf_ptr[ i ] = (opus_int16)((((((opus_int32)(((opus_int64)(exc_Q14[ i + ( k + nb_subfr - 2 ) * subfr_length ]) * (prevGain_Q10[ k ])) >> 16)))>>(8))) > 0x7FFF ? 0x7FFF : ((((((opus_int32)(((opus_int64)(exc_Q14[ i + ( k + nb_subfr - 2 ) * subfr_length ]) * (prevGain_Q10[ k ])) >> 16)))>>(8))) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((((opus_int32)(((opus_int64)(exc_Q14[ i + ( k + nb_subfr - 2 ) * subfr_length ]) * (prevGain_Q10[ k ])) >> 16)))>>(8)))))
                                                                                                           ;
        }
        exc_buf_ptr += subfr_length;
    }

    silk_sum_sqr_shift( energy1, shift1, exc_buf, subfr_length );
    silk_sum_sqr_shift( energy2, shift2, &exc_buf[ subfr_length ], subfr_length );
    ;
}

static inline void silk_PLC_conceal(
    silk_decoder_state *psDec,
    silk_decoder_control *psDecCtrl,
    opus_int16 frame[],
    int arch
)
{
    int i, j, k;
    int lag, idx, sLTP_buf_idx, shift1, shift2;
    opus_int32 rand_seed, harm_Gain_Q15, rand_Gain_Q15, inv_gain_Q30;
    opus_int32 energy1, energy2, *rand_ptr, *pred_lag_ptr;
    opus_int32 LPC_pred_Q10, LTP_pred_Q12;
    opus_int16 rand_scale_Q14;
    opus_int16 *B_Q14;
    opus_int32 *sLPC_Q14_ptr;
    opus_int16 A_Q12[ 16 ];



    ;

    ;
    silk_PLC_struct *psPLC = &psDec->sPLC;
    opus_int32 prevGain_Q10[2];
    ;

    opus_int32 *sLTP_Q14 = (opus_int32*)DR_OPUS_ALLOCA(sizeof(opus_int32)*(psDec->ltp_mem_length + psDec->frame_length));




    opus_int16 *sLTP = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*(psDec->ltp_mem_length));


    prevGain_Q10[0] = ((psPLC->prevGain_Q16[ 0 ])>>(6));
    prevGain_Q10[1] = ((psPLC->prevGain_Q16[ 1 ])>>(6));

    if( psDec->first_frame_after_reset ) {
       memset((psPLC->prevLPC_Q12), (0), (sizeof( psPLC->prevLPC_Q12 )));
    }

    silk_PLC_energy(&energy1, &shift1, &energy2, &shift2, psDec->exc_Q14, prevGain_Q10, psDec->subfr_length, psDec->nb_subfr);

    if( ((energy1)>>(shift2)) < ((energy2)>>(shift1)) ) {

        rand_ptr = &psDec->exc_Q14[ silk_max_int( 0, ( psPLC->nb_subfr - 1 ) * psPLC->subfr_length - 128 ) ];
    } else {

        rand_ptr = &psDec->exc_Q14[ silk_max_int( 0, psPLC->nb_subfr * psPLC->subfr_length - 128 ) ];
    }


    B_Q14 = psPLC->LTPCoef_Q14;
    rand_scale_Q14 = psPLC->randScale_Q14;


    harm_Gain_Q15 = HARM_ATT_Q15[ silk_min_int( 2 - 1, psDec->lossCnt ) ];
    if( psDec->prevSignalType == 2 ) {
        rand_Gain_Q15 = PLC_RAND_ATTENUATE_V_Q15[ silk_min_int( 2 - 1, psDec->lossCnt ) ];
    } else {
        rand_Gain_Q15 = PLC_RAND_ATTENUATE_UV_Q15[ silk_min_int( 2 - 1, psDec->lossCnt ) ];
    }


    silk_bwexpander( psPLC->prevLPC_Q12, psDec->LPC_order, ((opus_int32)((0.99) * ((opus_int64)1 << (16)) + 0.5)) );


    memcpy((A_Q12), (psPLC->prevLPC_Q12), (psDec->LPC_order * sizeof( opus_int16 )));


    if( psDec->lossCnt == 0 ) {
        rand_scale_Q14 = 1 << 14;


        if( psDec->prevSignalType == 2 ) {
            for( i = 0; i < 5; i++ ) {
                rand_scale_Q14 -= B_Q14[ i ];
            }
            rand_scale_Q14 = silk_max_16( 3277, rand_scale_Q14 );
            rand_scale_Q14 = (opus_int16)((((opus_int32)((opus_int16)(rand_scale_Q14)) * (opus_int32)((opus_int16)(psPLC->prevLTP_scale_Q14))))>>(14));
        } else {

            opus_int32 invGain_Q30, down_scale_Q30;

            invGain_Q30 = ((void)(arch), silk_LPC_inverse_pred_gain_c(psPLC->prevLPC_Q12, psDec->LPC_order));

            down_scale_Q30 = silk_min_32( (((opus_int32)1 << 30)>>(3)), invGain_Q30 );
            down_scale_Q30 = silk_max_32( (((opus_int32)1 << 30)>>(8)), down_scale_Q30 );
            down_scale_Q30 = ((opus_int32)((opus_uint32)(down_scale_Q30)<<(3)));

            rand_Gain_Q15 = ((((opus_int32)(((down_scale_Q30) * (opus_int64)((opus_int16)(rand_Gain_Q15))) >> 16)))>>(14));
        }
    }

    rand_seed = psPLC->rand_seed;
    lag = ((8) == 1 ? ((psPLC->pitchL_Q8) >> 1) + ((psPLC->pitchL_Q8) & 1) : (((psPLC->pitchL_Q8) >> ((8) - 1)) + 1) >> 1);
    sLTP_buf_idx = psDec->ltp_mem_length;


    idx = psDec->ltp_mem_length - lag - psDec->LPC_order - 5 / 2;
    ;
    silk_LPC_analysis_filter( &sLTP[ idx ], &psDec->outBuf[ idx ], A_Q12, psDec->ltp_mem_length - idx, psDec->LPC_order, arch );

    inv_gain_Q30 = silk_INVERSE32_varQ( psPLC->prevGain_Q16[ 1 ], 46 );
    inv_gain_Q30 = (((inv_gain_Q30) < (0x7FFFFFFF >> 1)) ? (inv_gain_Q30) : (0x7FFFFFFF >> 1));
    for( i = idx + psDec->LPC_order; i < psDec->ltp_mem_length; i++ ) {
        sLTP_Q14[ i ] = ((opus_int32)(((inv_gain_Q30) * (opus_int64)((opus_int16)(sLTP[ i ]))) >> 16));
    }




    for( k = 0; k < psDec->nb_subfr; k++ ) {

        pred_lag_ptr = &sLTP_Q14[ sLTP_buf_idx - lag + 5 / 2 ];
        for( i = 0; i < psDec->subfr_length; i++ ) {


            LTP_pred_Q12 = 2;
            LTP_pred_Q12 = ((opus_int32)((LTP_pred_Q12) + (((pred_lag_ptr[ 0 ]) * (opus_int64)((opus_int16)(B_Q14[ 0 ]))) >> 16)));
            LTP_pred_Q12 = ((opus_int32)((LTP_pred_Q12) + (((pred_lag_ptr[ -1 ]) * (opus_int64)((opus_int16)(B_Q14[ 1 ]))) >> 16)));
            LTP_pred_Q12 = ((opus_int32)((LTP_pred_Q12) + (((pred_lag_ptr[ -2 ]) * (opus_int64)((opus_int16)(B_Q14[ 2 ]))) >> 16)));
            LTP_pred_Q12 = ((opus_int32)((LTP_pred_Q12) + (((pred_lag_ptr[ -3 ]) * (opus_int64)((opus_int16)(B_Q14[ 3 ]))) >> 16)));
            LTP_pred_Q12 = ((opus_int32)((LTP_pred_Q12) + (((pred_lag_ptr[ -4 ]) * (opus_int64)((opus_int16)(B_Q14[ 4 ]))) >> 16)));
            pred_lag_ptr++;


            rand_seed = (((opus_int32)((opus_uint32)(((907633515))) + (opus_uint32)((opus_uint32)((rand_seed)) * (opus_uint32)((196314165))))));
            idx = ((rand_seed)>>(25)) & ( 128 - 1 );
            sLTP_Q14[ sLTP_buf_idx ] = ((opus_int32)((opus_uint32)(((opus_int32)((LTP_pred_Q12) + (((rand_ptr[ idx ]) * (opus_int64)((opus_int16)(rand_scale_Q14))) >> 16))))<<(2)));
            sLTP_buf_idx++;
        }


        for( j = 0; j < 5; j++ ) {
            B_Q14[ j ] = ((((opus_int32)((opus_int16)(harm_Gain_Q15)) * (opus_int32)((opus_int16)(B_Q14[ j ]))))>>(15));
        }

        rand_scale_Q14 = ((((opus_int32)((opus_int16)(rand_scale_Q14)) * (opus_int32)((opus_int16)(rand_Gain_Q15))))>>(15));


        psPLC->pitchL_Q8 = ((opus_int32)((psPLC->pitchL_Q8) + (((psPLC->pitchL_Q8) * (opus_int64)((opus_int16)(655))) >> 16)));
        psPLC->pitchL_Q8 = silk_min_32( psPLC->pitchL_Q8, ((opus_int32)((opus_uint32)(((opus_int32)((opus_int16)(18)) * (opus_int32)((opus_int16)(psDec->fs_kHz))))<<(8))) );
        lag = ((8) == 1 ? ((psPLC->pitchL_Q8) >> 1) + ((psPLC->pitchL_Q8) & 1) : (((psPLC->pitchL_Q8) >> ((8) - 1)) + 1) >> 1);
    }




    sLPC_Q14_ptr = &sLTP_Q14[ psDec->ltp_mem_length - 16 ];


    memcpy((sLPC_Q14_ptr), (psDec->sLPC_Q14_buf), (16 * sizeof( opus_int32 )));

    ;
    for( i = 0; i < psDec->frame_length; i++ ) {


        LPC_pred_Q10 = ((psDec->LPC_order)>>(1));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 1 ]) * (opus_int64)((opus_int16)(A_Q12[ 0 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 2 ]) * (opus_int64)((opus_int16)(A_Q12[ 1 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 3 ]) * (opus_int64)((opus_int16)(A_Q12[ 2 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 4 ]) * (opus_int64)((opus_int16)(A_Q12[ 3 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 5 ]) * (opus_int64)((opus_int16)(A_Q12[ 4 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 6 ]) * (opus_int64)((opus_int16)(A_Q12[ 5 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 7 ]) * (opus_int64)((opus_int16)(A_Q12[ 6 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 8 ]) * (opus_int64)((opus_int16)(A_Q12[ 7 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 9 ]) * (opus_int64)((opus_int16)(A_Q12[ 8 ]))) >> 16)));
        LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - 10 ]) * (opus_int64)((opus_int16)(A_Q12[ 9 ]))) >> 16)));
        for( j = 10; j < psDec->LPC_order; j++ ) {
            LPC_pred_Q10 = ((opus_int32)((LPC_pred_Q10) + (((sLPC_Q14_ptr[ 16 + i - j - 1 ]) * (opus_int64)((opus_int16)(A_Q12[ j ]))) >> 16)));
        }


        sLPC_Q14_ptr[ 16 + i ] = ((((opus_uint32)(sLPC_Q14_ptr[ 16 + i ]) + (opus_uint32)((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) == 0 ? ((((sLPC_Q14_ptr[ 16 + i ]) & ((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) != 0 ? ((opus_int32)0x80000000) : (sLPC_Q14_ptr[ 16 + i ])+((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) : ((((sLPC_Q14_ptr[ 16 + i ]) | ((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) & 0x80000000) == 0 ? 0x7FFFFFFF : (sLPC_Q14_ptr[ 16 + i ])+((((opus_int32)((opus_uint32)(((((((opus_int32)0x80000000))>>((4)))) > (((0x7FFFFFFF)>>((4)))) ? (((LPC_pred_Q10)) > (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : (((LPC_pred_Q10)) < (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : ((LPC_pred_Q10)))) : (((LPC_pred_Q10)) > (((0x7FFFFFFF)>>((4)))) ? (((0x7FFFFFFF)>>((4)))) : (((LPC_pred_Q10)) < (((((opus_int32)0x80000000))>>((4)))) ? (((((opus_int32)0x80000000))>>((4)))) : ((LPC_pred_Q10))))))<<((4))))))) )
                                                                                 ;


        frame[ i ] = (opus_int16)((((((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1))))) > 0x7FFF ? 0x7FFF : ((((((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1))))) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> 1) + ((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) & 1) : (((((opus_int32)(((opus_int64)(sLPC_Q14_ptr[ 16 + i ]) * (prevGain_Q10[ 1 ])) >> 16))) >> ((8) - 1)) + 1) >> 1)))))));
    }


    memcpy((psDec->sLPC_Q14_buf), (&sLPC_Q14_ptr[ psDec->frame_length ]), (16 * sizeof( opus_int32 )));




    psPLC->rand_seed = rand_seed;
    psPLC->randScale_Q14 = rand_scale_Q14;
    for( i = 0; i < 4; i++ ) {
        psDecCtrl->pitchL[ i ] = lag;
    }
    ;
}


void silk_PLC_glue_frames(
    silk_decoder_state *psDec,
    opus_int16 frame[],
    int length
)
{
    int i, energy_shift;
    opus_int32 energy;
    silk_PLC_struct *psPLC;
    psPLC = &psDec->sPLC;

    if( psDec->lossCnt ) {

        silk_sum_sqr_shift( &psPLC->conc_energy, &psPLC->conc_energy_shift, frame, length );

        psPLC->last_frame_lost = 1;
    } else {
        if( psDec->sPLC.last_frame_lost ) {

            silk_sum_sqr_shift( &energy, &energy_shift, frame, length );


            if( energy_shift > psPLC->conc_energy_shift ) {
                psPLC->conc_energy = ((psPLC->conc_energy)>>(energy_shift - psPLC->conc_energy_shift));
            } else if( energy_shift < psPLC->conc_energy_shift ) {
                energy = ((energy)>>(psPLC->conc_energy_shift - energy_shift));
            }


            if( energy > psPLC->conc_energy ) {
                opus_int32 frac_Q24, LZ;
                opus_int32 gain_Q16, slope_Q16;

                LZ = silk_CLZ32( psPLC->conc_energy );
                LZ = LZ - 1;
                psPLC->conc_energy = ((opus_int32)((opus_uint32)(psPLC->conc_energy)<<(LZ)));
                energy = ((energy)>>(silk_max_32( 24 - LZ, 0 )));

                frac_Q24 = ((opus_int32)((psPLC->conc_energy) / ((((energy) > (1)) ? (energy) : (1)))));

                gain_Q16 = ((opus_int32)((opus_uint32)(silk_SQRT_APPROX( frac_Q24 ))<<(4)));
                slope_Q16 = ((opus_int32)((( (opus_int32)1 << 16 ) - gain_Q16) / (length)));

                slope_Q16 = ((opus_int32)((opus_uint32)(slope_Q16)<<(2)));

                for( i = 0; i < length; i++ ) {
                    frame[ i ] = ((opus_int32)(((gain_Q16) * (opus_int64)((opus_int16)(frame[ i ]))) >> 16));
                    gain_Q16 += slope_Q16;
                    if( gain_Q16 > (opus_int32)1 << 16 ) {
                        break;
                    }
                }
            }
        }
        psPLC->last_frame_lost = 0;
    }
}
void silk_bwexpander(
    opus_int16 *ar,
    const int d,
    opus_int32 chirp_Q16
)
{
    int i;
    opus_int32 chirp_minus_one_Q16 = chirp_Q16 - 65536;



    for( i = 0; i < d - 1; i++ ) {
        ar[ i ] = (opus_int16)((16) == 1 ? ((((chirp_Q16) * (ar[ i ]))) >> 1) + ((((chirp_Q16) * (ar[ i ]))) & 1) : (((((chirp_Q16) * (ar[ i ]))) >> ((16) - 1)) + 1) >> 1);
        chirp_Q16 += ((16) == 1 ? ((((chirp_Q16) * (chirp_minus_one_Q16))) >> 1) + ((((chirp_Q16) * (chirp_minus_one_Q16))) & 1) : (((((chirp_Q16) * (chirp_minus_one_Q16))) >> ((16) - 1)) + 1) >> 1);
    }
    ar[ d - 1 ] = (opus_int16)((16) == 1 ? ((((chirp_Q16) * (ar[ d - 1 ]))) >> 1) + ((((chirp_Q16) * (ar[ d - 1 ]))) & 1) : (((((chirp_Q16) * (ar[ d - 1 ]))) >> ((16) - 1)) + 1) >> 1);
}
void silk_bwexpander_32(
    opus_int32 *ar,
    const int d,
    opus_int32 chirp_Q16
)
{
    int i;
    opus_int32 chirp_minus_one_Q16 = chirp_Q16 - 65536;

    for( i = 0; i < d - 1; i++ ) {
        ar[ i ] = ((opus_int32)(((opus_int64)(chirp_Q16) * (ar[ i ])) >> 16));
        chirp_Q16 += ((16) == 1 ? ((((chirp_Q16) * (chirp_minus_one_Q16))) >> 1) + ((((chirp_Q16) * (chirp_minus_one_Q16))) & 1) : (((((chirp_Q16) * (chirp_minus_one_Q16))) >> ((16) - 1)) + 1) >> 1);
    }
    ar[ d - 1 ] = ((opus_int32)(((opus_int64)(chirp_Q16) * (ar[ d - 1 ])) >> 16));
}
void silk_stereo_decode_pred(
    ec_dec *psRangeDec,
    opus_int32 pred_Q13[]
)
{
    int n, ix[ 2 ][ 3 ];
    opus_int32 low_Q13, step_Q13;


    n = ec_dec_icdf( psRangeDec, silk_stereo_pred_joint_iCDF, 8 );
    ix[ 0 ][ 2 ] = ((opus_int32)((n) / (5)));
    ix[ 1 ][ 2 ] = n - 5 * ix[ 0 ][ 2 ];
    for( n = 0; n < 2; n++ ) {
        ix[ n ][ 0 ] = ec_dec_icdf( psRangeDec, silk_uniform3_iCDF, 8 );
        ix[ n ][ 1 ] = ec_dec_icdf( psRangeDec, silk_uniform5_iCDF, 8 );
    }


    for( n = 0; n < 2; n++ ) {
        ix[ n ][ 0 ] += 3 * ix[ n ][ 2 ];
        low_Q13 = silk_stereo_pred_quant_Q13[ ix[ n ][ 0 ] ];
        step_Q13 = ((opus_int32)(((silk_stereo_pred_quant_Q13[ ix[ n ][ 0 ] + 1 ] - low_Q13) * (opus_int64)((opus_int16)(((opus_int32)((0.5 / 5) * ((opus_int64)1 << (16)) + 0.5))))) >> 16))
                                                                ;
        pred_Q13[ n ] = ((low_Q13) + ((opus_int32)((opus_int16)(step_Q13))) * (opus_int32)((opus_int16)(2 * ix[ n ][ 1 ] + 1)));
    }


    pred_Q13[ 0 ] -= pred_Q13[ 1 ];
}


void silk_stereo_decode_mid_only(
    ec_dec *psRangeDec,
    int *decode_only_mid
)
{

    *decode_only_mid = ec_dec_icdf( psRangeDec, silk_stereo_only_code_mid_iCDF, 8 );
}
void silk_stereo_MS_to_LR(
    stereo_dec_state *state,
    opus_int16 x1[],
    opus_int16 x2[],
    const opus_int32 pred_Q13[],
    int fs_kHz,
    int frame_length
)
{
    int n, denom_Q16, delta0_Q13, delta1_Q13;
    opus_int32 sum, diff, pred0_Q13, pred1_Q13;


    memcpy((x1), (state->sMid), (2 * sizeof( opus_int16 )));
    memcpy((x2), (state->sSide), (2 * sizeof( opus_int16 )));
    memcpy((state->sMid), (&x1[ frame_length ]), (2 * sizeof( opus_int16 )));
    memcpy((state->sSide), (&x2[ frame_length ]), (2 * sizeof( opus_int16 )));


    pred0_Q13 = state->pred_prev_Q13[ 0 ];
    pred1_Q13 = state->pred_prev_Q13[ 1 ];
    denom_Q16 = ((opus_int32)(((opus_int32)1 << 16) / (8 * fs_kHz)));
    delta0_Q13 = ((16) == 1 ? ((((opus_int32)((opus_int16)(pred_Q13[ 0 ] - state->pred_prev_Q13[ 0 ])) * (opus_int32)((opus_int16)(denom_Q16)))) >> 1) + ((((opus_int32)((opus_int16)(pred_Q13[ 0 ] - state->pred_prev_Q13[ 0 ])) * (opus_int32)((opus_int16)(denom_Q16)))) & 1) : (((((opus_int32)((opus_int16)(pred_Q13[ 0 ] - state->pred_prev_Q13[ 0 ])) * (opus_int32)((opus_int16)(denom_Q16)))) >> ((16) - 1)) + 1) >> 1);
    delta1_Q13 = ((16) == 1 ? ((((opus_int32)((opus_int16)(pred_Q13[ 1 ] - state->pred_prev_Q13[ 1 ])) * (opus_int32)((opus_int16)(denom_Q16)))) >> 1) + ((((opus_int32)((opus_int16)(pred_Q13[ 1 ] - state->pred_prev_Q13[ 1 ])) * (opus_int32)((opus_int16)(denom_Q16)))) & 1) : (((((opus_int32)((opus_int16)(pred_Q13[ 1 ] - state->pred_prev_Q13[ 1 ])) * (opus_int32)((opus_int16)(denom_Q16)))) >> ((16) - 1)) + 1) >> 1);
    for( n = 0; n < 8 * fs_kHz; n++ ) {
        pred0_Q13 += delta0_Q13;
        pred1_Q13 += delta1_Q13;
        sum = ((opus_int32)((opus_uint32)((((x1[ n ] + (opus_int32)x1[ n + 2 ])) + (((opus_int32)((opus_uint32)((x1[ n + 1 ]))<<((1)))))))<<(9)));
        sum = ((opus_int32)((((opus_int32)((opus_uint32)((opus_int32)x2[ n + 1 ])<<(8)))) + (((sum) * (opus_int64)((opus_int16)(pred0_Q13))) >> 16)));
        sum = ((opus_int32)((sum) + (((((opus_int32)((opus_uint32)((opus_int32)x1[ n + 1 ])<<(11)))) * (opus_int64)((opus_int16)(pred1_Q13))) >> 16)));
        x2[ n + 1 ] = (opus_int16)((((8) == 1 ? ((sum) >> 1) + ((sum) & 1) : (((sum) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((sum) >> 1) + ((sum) & 1) : (((sum) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((sum) >> 1) + ((sum) & 1) : (((sum) >> ((8) - 1)) + 1) >> 1))));
    }
    pred0_Q13 = pred_Q13[ 0 ];
    pred1_Q13 = pred_Q13[ 1 ];
    for( n = 8 * fs_kHz; n < frame_length; n++ ) {
        sum = ((opus_int32)((opus_uint32)((((x1[ n ] + (opus_int32)x1[ n + 2 ])) + (((opus_int32)((opus_uint32)((x1[ n + 1 ]))<<((1)))))))<<(9)));
        sum = ((opus_int32)((((opus_int32)((opus_uint32)((opus_int32)x2[ n + 1 ])<<(8)))) + (((sum) * (opus_int64)((opus_int16)(pred0_Q13))) >> 16)));
        sum = ((opus_int32)((sum) + (((((opus_int32)((opus_uint32)((opus_int32)x1[ n + 1 ])<<(11)))) * (opus_int64)((opus_int16)(pred1_Q13))) >> 16)));
        x2[ n + 1 ] = (opus_int16)((((8) == 1 ? ((sum) >> 1) + ((sum) & 1) : (((sum) >> ((8) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((8) == 1 ? ((sum) >> 1) + ((sum) & 1) : (((sum) >> ((8) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((8) == 1 ? ((sum) >> 1) + ((sum) & 1) : (((sum) >> ((8) - 1)) + 1) >> 1))));
    }
    state->pred_prev_Q13[ 0 ] = pred_Q13[ 0 ];
    state->pred_prev_Q13[ 1 ] = pred_Q13[ 1 ];


    for( n = 0; n < frame_length; n++ ) {
        sum = x1[ n + 1 ] + (opus_int32)x2[ n + 1 ];
        diff = x1[ n + 1 ] - (opus_int32)x2[ n + 1 ];
        x1[ n + 1 ] = (opus_int16)((sum) > 0x7FFF ? 0x7FFF : ((sum) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (sum)));
        x2[ n + 1 ] = (opus_int16)((diff) > 0x7FFF ? 0x7FFF : ((diff) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (diff)));
    }
}
void silk_LPC_analysis_filter(
    opus_int16 *out,
    const opus_int16 *in,
    const opus_int16 *B,
    const opus_int32 len,
    const opus_int32 d,
    int arch
)
{
    int j;



    int ix;
    opus_int32 out32_Q12, out32;
    const opus_int16 *in_ptr;


    ;
    ;
    ;
    (void)arch;
    for( ix = d; ix < len; ix++ ) {
        in_ptr = &in[ ix - 1 ];

        out32_Q12 = ((opus_int32)((opus_int16)(in_ptr[ 0 ])) * (opus_int32)((opus_int16)(B[ 0 ])));


        out32_Q12 = (((opus_int32)((opus_uint32)((out32_Q12)) + (opus_uint32)(((opus_int32)((opus_int16)(in_ptr[ -1 ]))) * (opus_int32)((opus_int16)(B[ 1 ]))))));
        out32_Q12 = (((opus_int32)((opus_uint32)((out32_Q12)) + (opus_uint32)(((opus_int32)((opus_int16)(in_ptr[ -2 ]))) * (opus_int32)((opus_int16)(B[ 2 ]))))));
        out32_Q12 = (((opus_int32)((opus_uint32)((out32_Q12)) + (opus_uint32)(((opus_int32)((opus_int16)(in_ptr[ -3 ]))) * (opus_int32)((opus_int16)(B[ 3 ]))))));
        out32_Q12 = (((opus_int32)((opus_uint32)((out32_Q12)) + (opus_uint32)(((opus_int32)((opus_int16)(in_ptr[ -4 ]))) * (opus_int32)((opus_int16)(B[ 4 ]))))));
        out32_Q12 = (((opus_int32)((opus_uint32)((out32_Q12)) + (opus_uint32)(((opus_int32)((opus_int16)(in_ptr[ -5 ]))) * (opus_int32)((opus_int16)(B[ 5 ]))))));
        for( j = 6; j < d; j += 2 ) {
            out32_Q12 = (((opus_int32)((opus_uint32)((out32_Q12)) + (opus_uint32)(((opus_int32)((opus_int16)(in_ptr[ -j ]))) * (opus_int32)((opus_int16)(B[ j ]))))));
            out32_Q12 = (((opus_int32)((opus_uint32)((out32_Q12)) + (opus_uint32)(((opus_int32)((opus_int16)(in_ptr[ -j - 1 ]))) * (opus_int32)((opus_int16)(B[ j + 1 ]))))));
        }


        out32_Q12 = ((opus_int32)((opus_uint32)(((opus_int32)((opus_uint32)((opus_int32)in_ptr[ 1 ])<<(12)))) - (opus_uint32)(out32_Q12)));


        out32 = ((12) == 1 ? ((out32_Q12) >> 1) + ((out32_Q12) & 1) : (((out32_Q12) >> ((12) - 1)) + 1) >> 1);


        out[ ix ] = (opus_int16)((out32) > 0x7FFF ? 0x7FFF : ((out32) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (out32)));
    }


    memset((out), (0), (d * sizeof( opus_int16 )));

}
static opus_int32 LPC_inverse_pred_gain_QA_c(
    opus_int32 A_QA[ 24 ],
    const int order
)
{
    int k, n, mult2Q;
    opus_int32 invGain_Q30, rc_Q31, rc_mult1_Q30, rc_mult2, tmp1, tmp2;

    invGain_Q30 = ((opus_int32)((1) * ((opus_int64)1 << (30)) + 0.5));
    for( k = order - 1; k > 0; k-- ) {

        if( ( A_QA[ k ] > ((opus_int32)((0.99975) * ((opus_int64)1 << (24)) + 0.5)) ) || ( A_QA[ k ] < -((opus_int32)((0.99975) * ((opus_int64)1 << (24)) + 0.5)) ) ) {
            return 0;
        }


        rc_Q31 = -((opus_int32)((opus_uint32)(A_QA[ k ])<<(31 - 24)));


        rc_mult1_Q30 = ((((opus_int32)((1) * ((opus_int64)1 << (30)) + 0.5))) - ((opus_int32)((((opus_int64)((rc_Q31)) * ((rc_Q31))))>>(32))));
        ;
        ;



        invGain_Q30 = ((opus_int32)((opus_uint32)((opus_int32)((((opus_int64)((invGain_Q30)) * ((rc_mult1_Q30))))>>(32)))<<(2)));
        ;
        ;
        if( invGain_Q30 < ((opus_int32)((1.0f / 1e4f) * ((opus_int64)1 << (30)) + 0.5)) ) {
            return 0;
        }


        mult2Q = 32 - silk_CLZ32( (((rc_mult1_Q30) > 0) ? (rc_mult1_Q30) : -(rc_mult1_Q30)) );
        rc_mult2 = silk_INVERSE32_varQ( rc_mult1_Q30, mult2Q + 30 );


        for( n = 0; n < (k + 1) >> 1; n++ ) {
            opus_int64 tmp64;
            tmp1 = A_QA[ n ];
            tmp2 = A_QA[ k - n - 1 ];
            tmp64 = ((mult2Q) == 1 ? ((((opus_int64)(((((opus_uint32)(tmp1)-(opus_uint32)(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) & 0x80000000) == 0 ? (( (tmp1) & ((((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))^0x80000000) & 0x80000000) ? ((opus_int32)0x80000000) : (tmp1)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) : ((((tmp1)^0x80000000) & (((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1)))) & 0x80000000) ? 0x7FFFFFFF : (tmp1)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) )) * (rc_mult2))) >> 1) + ((((opus_int64)(((((opus_uint32)(tmp1)-(opus_uint32)(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) & 0x80000000) == 0 ? (( (tmp1) & ((((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))^0x80000000) & 0x80000000) ? ((opus_int32)0x80000000) : (tmp1)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) : ((((tmp1)^0x80000000) & (((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1)))) & 0x80000000) ? 0x7FFFFFFF : (tmp1)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) )) * (rc_mult2))) & 1) : (((((opus_int64)(((((opus_uint32)(tmp1)-(opus_uint32)(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) & 0x80000000) == 0 ? (( (tmp1) & ((((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))^0x80000000) & 0x80000000) ? ((opus_int32)0x80000000) : (tmp1)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) : ((((tmp1)^0x80000000) & (((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1)))) & 0x80000000) ? 0x7FFFFFFF : (tmp1)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp2) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp2) * (rc_Q31))) & 1) : (((((opus_int64)(tmp2) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) )) * (rc_mult2))) >> ((mult2Q) - 1)) + 1) >> 1)
                                                                         ;
            if( tmp64 > 0x7FFFFFFF || tmp64 < ((opus_int32)0x80000000) ) {
               return 0;
            }
            A_QA[ n ] = ( opus_int32 )tmp64;
            tmp64 = ((mult2Q) == 1 ? ((((opus_int64)(((((opus_uint32)(tmp2)-(opus_uint32)(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) & 0x80000000) == 0 ? (( (tmp2) & ((((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))^0x80000000) & 0x80000000) ? ((opus_int32)0x80000000) : (tmp2)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) : ((((tmp2)^0x80000000) & (((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1)))) & 0x80000000) ? 0x7FFFFFFF : (tmp2)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) )) * (rc_mult2))) >> 1) + ((((opus_int64)(((((opus_uint32)(tmp2)-(opus_uint32)(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) & 0x80000000) == 0 ? (( (tmp2) & ((((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))^0x80000000) & 0x80000000) ? ((opus_int32)0x80000000) : (tmp2)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) : ((((tmp2)^0x80000000) & (((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1)))) & 0x80000000) ? 0x7FFFFFFF : (tmp2)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) )) * (rc_mult2))) & 1) : (((((opus_int64)(((((opus_uint32)(tmp2)-(opus_uint32)(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) & 0x80000000) == 0 ? (( (tmp2) & ((((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))^0x80000000) & 0x80000000) ? ((opus_int32)0x80000000) : (tmp2)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) : ((((tmp2)^0x80000000) & (((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1)))) & 0x80000000) ? 0x7FFFFFFF : (tmp2)-(((opus_int32)(((31) == 1 ? ((((opus_int64)(tmp1) * (rc_Q31))) >> 1) + ((((opus_int64)(tmp1) * (rc_Q31))) & 1) : (((((opus_int64)(tmp1) * (rc_Q31))) >> ((31) - 1)) + 1) >> 1))))) )) * (rc_mult2))) >> ((mult2Q) - 1)) + 1) >> 1)
                                                                        ;
            if( tmp64 > 0x7FFFFFFF || tmp64 < ((opus_int32)0x80000000) ) {
               return 0;
            }
            A_QA[ k - n - 1 ] = ( opus_int32 )tmp64;
        }
    }


    if( ( A_QA[ k ] > ((opus_int32)((0.99975) * ((opus_int64)1 << (24)) + 0.5)) ) || ( A_QA[ k ] < -((opus_int32)((0.99975) * ((opus_int64)1 << (24)) + 0.5)) ) ) {
        return 0;
    }


    rc_Q31 = -((opus_int32)((opus_uint32)(A_QA[ 0 ])<<(31 - 24)));


    rc_mult1_Q30 = ((((opus_int32)((1) * ((opus_int64)1 << (30)) + 0.5))) - ((opus_int32)((((opus_int64)((rc_Q31)) * ((rc_Q31))))>>(32))));



    invGain_Q30 = ((opus_int32)((opus_uint32)((opus_int32)((((opus_int64)((invGain_Q30)) * ((rc_mult1_Q30))))>>(32)))<<(2)));
    ;
    ;
    if( invGain_Q30 < ((opus_int32)((1.0f / 1e4f) * ((opus_int64)1 << (30)) + 0.5)) ) {
        return 0;
    }

    return invGain_Q30;
}


opus_int32 silk_LPC_inverse_pred_gain_c(
    const opus_int16 *A_Q12,
    const int order
)
{
    int k;
    opus_int32 Atmp_QA[ 24 ];
    opus_int32 DC_resp = 0;


    for( k = 0; k < order; k++ ) {
        DC_resp += (opus_int32)A_Q12[ k ];
        Atmp_QA[ k ] = ((opus_int32)((opus_uint32)((opus_int32)A_Q12[ k ])<<(24 - 12)));
    }

    if( DC_resp >= 4096 ) {
        return 0;
    }
    return LPC_inverse_pred_gain_QA_c( Atmp_QA, order );
}
static inline void silk_NLSF_residual_dequant(
          opus_int16 x_Q10[],
    const opus_int8 indices[],
    const opus_uint8 pred_coef_Q8[],
    const int quant_step_size_Q16,
    const opus_int16 order
)
{
    int i, out_Q10, pred_Q10;

    out_Q10 = 0;
    for( i = order-1; i >= 0; i-- ) {
        pred_Q10 = ((((opus_int32)((opus_int16)(out_Q10)) * (opus_int32)((opus_int16)((opus_int16)pred_coef_Q8[ i ]))))>>(8));
        out_Q10 = ((opus_int32)((opus_uint32)(indices[ i ])<<(10)));
        if( out_Q10 > 0 ) {
            out_Q10 = ((out_Q10) - (((opus_int32)((0.1) * ((opus_int64)1 << (10)) + 0.5))));
        } else if( out_Q10 < 0 ) {
            out_Q10 = ((out_Q10) + (((opus_int32)((0.1) * ((opus_int64)1 << (10)) + 0.5))));
        }
        out_Q10 = ((opus_int32)((pred_Q10) + ((((opus_int32)out_Q10) * (opus_int64)((opus_int16)(quant_step_size_Q16))) >> 16)));
        x_Q10[ i ] = out_Q10;
    }
}





void silk_NLSF_decode(
          opus_int16 *pNLSF_Q15,
          opus_int8 *NLSFIndices,
    const silk_NLSF_CB_struct *psNLSF_CB
)
{
    int i;
    opus_uint8 pred_Q8[ 16 ];
    opus_int16 ec_ix[ 16 ];
    opus_int16 res_Q10[ 16 ];
    opus_int32 NLSF_Q15_tmp;
    const opus_uint8 *pCB_element;
    const opus_int16 *pCB_Wght_Q9;


    silk_NLSF_unpack( ec_ix, pred_Q8, psNLSF_CB, NLSFIndices[ 0 ] );


    silk_NLSF_residual_dequant( res_Q10, &NLSFIndices[ 1 ], pred_Q8, psNLSF_CB->quantStepSize_Q16, psNLSF_CB->order );


    pCB_element = &psNLSF_CB->CB1_NLSF_Q8[ NLSFIndices[ 0 ] * psNLSF_CB->order ];
    pCB_Wght_Q9 = &psNLSF_CB->CB1_Wght_Q9[ NLSFIndices[ 0 ] * psNLSF_CB->order ];
    for( i = 0; i < psNLSF_CB->order; i++ ) {
        NLSF_Q15_tmp = (((((opus_int32)((((opus_int32)((opus_uint32)((opus_int32)res_Q10[ i ])<<(14)))) / (pCB_Wght_Q9[ i ]))))) + (((opus_int32)((opus_uint32)(((opus_int16)pCB_element[ i ]))<<((7))))));
        pNLSF_Q15[ i ] = (opus_int16)((0) > (32767) ? ((NLSF_Q15_tmp) > (0) ? (0) : ((NLSF_Q15_tmp) < (32767) ? (32767) : (NLSF_Q15_tmp))) : ((NLSF_Q15_tmp) > (32767) ? (32767) : ((NLSF_Q15_tmp) < (0) ? (0) : (NLSF_Q15_tmp))));
    }


    silk_NLSF_stabilize( pNLSF_Q15, psNLSF_CB->deltaMin_Q15, psNLSF_CB->order );
}
static inline void silk_NLSF2A_find_poly(
    opus_int32 *out,
    const opus_int32 *cLSF,
    int dd
)
{
    int k, n;
    opus_int32 ftmp;

    out[0] = ((opus_int32)((opus_uint32)(1)<<(16)));
    out[1] = -cLSF[0];
    for( k = 1; k < dd; k++ ) {
        ftmp = cLSF[2*k];
        out[k+1] = ((opus_int32)((opus_uint32)(out[k-1])<<(1))) - (opus_int32)((16) == 1 ? ((((opus_int64)(ftmp) * (out[k]))) >> 1) + ((((opus_int64)(ftmp) * (out[k]))) & 1) : (((((opus_int64)(ftmp) * (out[k]))) >> ((16) - 1)) + 1) >> 1);
        for( n = k; n > 1; n-- ) {
            out[n] += out[n-2] - (opus_int32)((16) == 1 ? ((((opus_int64)(ftmp) * (out[n-1]))) >> 1) + ((((opus_int64)(ftmp) * (out[n-1]))) & 1) : (((((opus_int64)(ftmp) * (out[n-1]))) >> ((16) - 1)) + 1) >> 1);
        }
        out[1] -= ftmp;
    }
}


void silk_NLSF2A(
    opus_int16 *a_Q12,
    const opus_int16 *NLSF,
    const int d,
    int arch
)
{


    static const unsigned char ordering16[16] = {
      0, 15, 8, 7, 4, 11, 12, 3, 2, 13, 10, 5, 6, 9, 14, 1
    };
    static const unsigned char ordering10[10] = {
      0, 9, 6, 3, 4, 5, 8, 1, 2, 7
    };
    const unsigned char *ordering;
    int k, i, dd;
    opus_int32 cos_LSF_QA[ 24 ];
    opus_int32 P[ 24 / 2 + 1 ], Q[ 24 / 2 + 1 ];
    opus_int32 Ptmp, Qtmp, f_int, f_frac, cos_val, delta;
    opus_int32 a32_QA1[ 24 ];

    ;
    ;


    ordering = d == 16 ? ordering16 : ordering10;
    for( k = 0; k < d; k++ ) {
        ;


        f_int = ((NLSF[k])>>(15 - 7));


        f_frac = NLSF[k] - ((opus_int32)((opus_uint32)(f_int)<<(15 - 7)));

        ;
        ;


        cos_val = silk_LSFCosTab_FIX_Q12[ f_int ];
        delta = silk_LSFCosTab_FIX_Q12[ f_int + 1 ] - cos_val;


        cos_LSF_QA[ordering[k]] = ((20 - 16) == 1 ? ((((opus_int32)((opus_uint32)(cos_val)<<(8))) + ((delta) * (f_frac))) >> 1) + ((((opus_int32)((opus_uint32)(cos_val)<<(8))) + ((delta) * (f_frac))) & 1) : (((((opus_int32)((opus_uint32)(cos_val)<<(8))) + ((delta) * (f_frac))) >> ((20 - 16) - 1)) + 1) >> 1);
    }

    dd = ((d)>>(1));


    silk_NLSF2A_find_poly( P, &cos_LSF_QA[ 0 ], dd );
    silk_NLSF2A_find_poly( Q, &cos_LSF_QA[ 1 ], dd );


    for( k = 0; k < dd; k++ ) {
        Ptmp = P[ k+1 ] + P[ k ];
        Qtmp = Q[ k+1 ] - Q[ k ];


        a32_QA1[ k ] = -Qtmp - Ptmp;
        a32_QA1[ d-k-1 ] = Qtmp - Ptmp;
    }


    silk_LPC_fit( a_Q12, a32_QA1, 12, 16 + 1, d );

    for( i = 0; ((void)(arch), silk_LPC_inverse_pred_gain_c(a_Q12, d)) == 0 && i < 16; i++ ) {


        silk_bwexpander_32( a32_QA1, d, 65536 - ((opus_int32)((opus_uint32)(2)<<(i))) );
        for( k = 0; k < d; k++ ) {
            a_Q12[ k ] = (opus_int16)((16 + 1 - 12) == 1 ? ((a32_QA1[ k ]) >> 1) + ((a32_QA1[ k ]) & 1) : (((a32_QA1[ k ]) >> ((16 + 1 - 12) - 1)) + 1) >> 1);
        }
    }
}
void silk_NLSF_stabilize(
          opus_int16 *NLSF_Q15,
    const opus_int16 *NDeltaMin_Q15,
    const int L
)
{
    int i, I=0, k, loops;
    opus_int16 center_freq_Q15;
    opus_int32 diff_Q15, min_diff_Q15, min_center_Q15, max_center_Q15;


    ;

    for( loops = 0; loops < 20; loops++ ) {




        min_diff_Q15 = NLSF_Q15[0] - NDeltaMin_Q15[0];
        I = 0;

        for( i = 1; i <= L-1; i++ ) {
            diff_Q15 = NLSF_Q15[i] - ( NLSF_Q15[i-1] + NDeltaMin_Q15[i] );
            if( diff_Q15 < min_diff_Q15 ) {
                min_diff_Q15 = diff_Q15;
                I = i;
            }
        }

        diff_Q15 = ( 1 << 15 ) - ( NLSF_Q15[L-1] + NDeltaMin_Q15[L] );
        if( diff_Q15 < min_diff_Q15 ) {
            min_diff_Q15 = diff_Q15;
            I = L;
        }




        if( min_diff_Q15 >= 0 ) {
            return;
        }

        if( I == 0 ) {

            NLSF_Q15[0] = NDeltaMin_Q15[0];

        } else if( I == L) {

            NLSF_Q15[L-1] = ( 1 << 15 ) - NDeltaMin_Q15[L];

        } else {

            min_center_Q15 = 0;
            for( k = 0; k < I; k++ ) {
                min_center_Q15 += NDeltaMin_Q15[k];
            }
            min_center_Q15 += ((NDeltaMin_Q15[I])>>(1));


            max_center_Q15 = 1 << 15;
            for( k = L; k > I; k-- ) {
                max_center_Q15 -= NDeltaMin_Q15[k];
            }
            max_center_Q15 -= ((NDeltaMin_Q15[I])>>(1));


            center_freq_Q15 = (opus_int16)((min_center_Q15) > (max_center_Q15) ? ((((1) == 1 ? (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> 1) + (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) & 1) : ((((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> ((1) - 1)) + 1) >> 1)) > (min_center_Q15) ? (min_center_Q15) : ((((1) == 1 ? (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> 1) + (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) & 1) : ((((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> ((1) - 1)) + 1) >> 1)) < (max_center_Q15) ? (max_center_Q15) : (((1) == 1 ? (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> 1) + (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) & 1) : ((((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> ((1) - 1)) + 1) >> 1)))) : ((((1) == 1 ? (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> 1) + (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) & 1) : ((((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> ((1) - 1)) + 1) >> 1)) > (max_center_Q15) ? (max_center_Q15) : ((((1) == 1 ? (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> 1) + (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) & 1) : ((((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> ((1) - 1)) + 1) >> 1)) < (min_center_Q15) ? (min_center_Q15) : (((1) == 1 ? (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> 1) + (((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) & 1) : ((((opus_int32)NLSF_Q15[I-1] + (opus_int32)NLSF_Q15[I]) >> ((1) - 1)) + 1) >> 1)))))
                                                ;
            NLSF_Q15[I-1] = center_freq_Q15 - ((NDeltaMin_Q15[I])>>(1));
            NLSF_Q15[I] = NLSF_Q15[I-1] + NDeltaMin_Q15[I];
        }
    }


    if( loops == 20 )
    {



        silk_insertion_sort_increasing_all_values_int16( &NLSF_Q15[0], L );


        NLSF_Q15[0] = silk_max_int( NLSF_Q15[0], NDeltaMin_Q15[0] );


        for( i = 1; i < L; i++ )
            NLSF_Q15[i] = silk_max_int( NLSF_Q15[i], (opus_int16)(((((opus_int32)(NLSF_Q15[i-1])) + ((NDeltaMin_Q15[i])))) > 0x7FFF ? 0x7FFF : (((((opus_int32)(NLSF_Q15[i-1])) + ((NDeltaMin_Q15[i])))) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : ((((opus_int32)(NLSF_Q15[i-1])) + ((NDeltaMin_Q15[i])))))) );


        NLSF_Q15[L-1] = silk_min_int( NLSF_Q15[L-1], (1<<15) - NDeltaMin_Q15[L] );


        for( i = L-2; i >= 0; i-- )
            NLSF_Q15[i] = silk_min_int( NLSF_Q15[i], NLSF_Q15[i+1] - NDeltaMin_Q15[i+1] );
    }
}
void silk_NLSF_VQ_weights_laroia(
    opus_int16 *pNLSFW_Q_OUT,
    const opus_int16 *pNLSF_Q15,
    const int D
)
{
    int k;
    opus_int32 tmp1_int, tmp2_int;

    ;
    ;


    tmp1_int = silk_max_int( pNLSF_Q15[ 0 ], 1 );
    tmp1_int = ((opus_int32)(((opus_int32)1 << ( 15 + 2 )) / (tmp1_int)));
    tmp2_int = silk_max_int( pNLSF_Q15[ 1 ] - pNLSF_Q15[ 0 ], 1 );
    tmp2_int = ((opus_int32)(((opus_int32)1 << ( 15 + 2 )) / (tmp2_int)));
    pNLSFW_Q_OUT[ 0 ] = (opus_int16)silk_min_int( tmp1_int + tmp2_int, 0x7FFF );
    ;


    for( k = 1; k < D - 1; k += 2 ) {
        tmp1_int = silk_max_int( pNLSF_Q15[ k + 1 ] - pNLSF_Q15[ k ], 1 );
        tmp1_int = ((opus_int32)(((opus_int32)1 << ( 15 + 2 )) / (tmp1_int)));
        pNLSFW_Q_OUT[ k ] = (opus_int16)silk_min_int( tmp1_int + tmp2_int, 0x7FFF );
        ;

        tmp2_int = silk_max_int( pNLSF_Q15[ k + 2 ] - pNLSF_Q15[ k + 1 ], 1 );
        tmp2_int = ((opus_int32)(((opus_int32)1 << ( 15 + 2 )) / (tmp2_int)));
        pNLSFW_Q_OUT[ k + 1 ] = (opus_int16)silk_min_int( tmp1_int + tmp2_int, 0x7FFF );
        ;
    }


    tmp1_int = silk_max_int( ( 1 << 15 ) - pNLSF_Q15[ D - 1 ], 1 );
    tmp1_int = ((opus_int32)(((opus_int32)1 << ( 15 + 2 )) / (tmp1_int)));
    pNLSFW_Q_OUT[ D - 1 ] = (opus_int16)silk_min_int( tmp1_int + tmp2_int, 0x7FFF );
    ;
}
void silk_NLSF_unpack(
          opus_int16 ec_ix[],
          opus_uint8 pred_Q8[],
    const silk_NLSF_CB_struct *psNLSF_CB,
    const int CB1_index
)
{
    int i;
    opus_uint8 entry;
    const opus_uint8 *ec_sel_ptr;

    ec_sel_ptr = &psNLSF_CB->ec_sel[ CB1_index * psNLSF_CB->order / 2 ];
    for( i = 0; i < psNLSF_CB->order; i += 2 ) {
        entry = *ec_sel_ptr++;
        ec_ix [ i ] = ((opus_int32)((opus_int16)(((entry)>>(1)) & 7)) * (opus_int32)((opus_int16)(2 * 4 + 1)));
        pred_Q8[ i ] = psNLSF_CB->pred_Q8[ i + ( entry & 1 ) * ( psNLSF_CB->order - 1 ) ];
        ec_ix [ i + 1 ] = ((opus_int32)((opus_int16)(((entry)>>(5)) & 7)) * (opus_int32)((opus_int16)(2 * 4 + 1)));
        pred_Q8[ i + 1 ] = psNLSF_CB->pred_Q8[ i + ( ((entry)>>(4)) & 1 ) * ( psNLSF_CB->order - 1 ) + 1 ];
    }
}
static const opus_int16 silk_resampler_down2_0 = 9872;
static const opus_int16 silk_resampler_down2_1 = 39809 - 65536;


static const opus_int16 silk_resampler_up2_hq_0[ 3 ] = { 1746, 14986, 39083 - 65536 };
static const opus_int16 silk_resampler_up2_hq_1[ 3 ] = { 6854, 25769, 55542 - 65536 };


extern const opus_int16 silk_Resampler_3_4_COEFS[ 2 + 3 * 18 / 2 ];
extern const opus_int16 silk_Resampler_2_3_COEFS[ 2 + 2 * 18 / 2 ];
extern const opus_int16 silk_Resampler_1_2_COEFS[ 2 + 24 / 2 ];
extern const opus_int16 silk_Resampler_1_3_COEFS[ 2 + 36 / 2 ];
extern const opus_int16 silk_Resampler_1_4_COEFS[ 2 + 36 / 2 ];
extern const opus_int16 silk_Resampler_1_6_COEFS[ 2 + 36 / 2 ];
extern const opus_int16 silk_Resampler_2_3_COEFS_LQ[ 2 + 2 * 2 ];


extern const opus_int16 silk_resampler_frac_FIR_12[ 12 ][ 8 / 2 ];







void silk_resampler_private_IIR_FIR(
    void *SS,
    opus_int16 out[],
    const opus_int16 in[],
    opus_int32 inLen
);


void silk_resampler_private_down_FIR(
    void *SS,
    opus_int16 out[],
    const opus_int16 in[],
    opus_int32 inLen
);


void silk_resampler_private_up2_HQ_wrapper(
    void *SS,
    opus_int16 *out,
    const opus_int16 *in,
    opus_int32 len
);


void silk_resampler_private_up2_HQ(
    opus_int32 *S,
    opus_int16 *out,
    const opus_int16 *in,
    opus_int32 len
);


void silk_resampler_private_AR2(
    opus_int32 S[],
    opus_int32 out_Q8[],
    const opus_int16 in[],
    const opus_int16 A_Q14[],
    opus_int32 len
);


static const opus_int8 delay_matrix_enc[ 5 ][ 3 ] = {

           { 6, 0, 3 },
           { 0, 7, 3 },
           { 0, 1, 10 },
           { 0, 2, 6 },
           { 18, 10, 12 }
};

static const opus_int8 delay_matrix_dec[ 3 ][ 5 ] = {

           { 4, 0, 2, 0, 0 },
           { 0, 9, 4, 7, 4 },
           { 0, 3, 12, 7, 7 }
};
int silk_resampler_init(
    silk_resampler_state_struct *S,
    opus_int32 Fs_Hz_in,
    opus_int32 Fs_Hz_out,
    int forEnc
)
{
    int up2x;


    memset((S), (0), (sizeof( silk_resampler_state_struct )));


    if( forEnc ) {
        if( ( Fs_Hz_in != 8000 && Fs_Hz_in != 12000 && Fs_Hz_in != 16000 && Fs_Hz_in != 24000 && Fs_Hz_in != 48000 ) ||
            ( Fs_Hz_out != 8000 && Fs_Hz_out != 12000 && Fs_Hz_out != 16000 ) ) {
            ;
            return -1;
        }
        S->inputDelay = delay_matrix_enc[ ( ( ( ((Fs_Hz_in)>>12) - ((Fs_Hz_in)>16000) ) >> ((Fs_Hz_in)>24000) ) - 1 ) ][ ( ( ( ((Fs_Hz_out)>>12) - ((Fs_Hz_out)>16000) ) >> ((Fs_Hz_out)>24000) ) - 1 ) ];
    } else {
        if( ( Fs_Hz_in != 8000 && Fs_Hz_in != 12000 && Fs_Hz_in != 16000 ) ||
            ( Fs_Hz_out != 8000 && Fs_Hz_out != 12000 && Fs_Hz_out != 16000 && Fs_Hz_out != 24000 && Fs_Hz_out != 48000 ) ) {
            ;
            return -1;
        }
        S->inputDelay = delay_matrix_dec[ ( ( ( ((Fs_Hz_in)>>12) - ((Fs_Hz_in)>16000) ) >> ((Fs_Hz_in)>24000) ) - 1 ) ][ ( ( ( ((Fs_Hz_out)>>12) - ((Fs_Hz_out)>16000) ) >> ((Fs_Hz_out)>24000) ) - 1 ) ];
    }

    S->Fs_in_kHz = ((opus_int32)((Fs_Hz_in) / (1000)));
    S->Fs_out_kHz = ((opus_int32)((Fs_Hz_out) / (1000)));


    S->batchSize = S->Fs_in_kHz * 10;


    up2x = 0;
    if( Fs_Hz_out > Fs_Hz_in ) {

        if( Fs_Hz_out == ((Fs_Hz_in) * (2)) ) {

            S->resampler_function = (1);
        } else {

            S->resampler_function = (2);
            up2x = 1;
        }
    } else if ( Fs_Hz_out < Fs_Hz_in ) {

         S->resampler_function = (3);
        if( ((Fs_Hz_out) * (4)) == ((Fs_Hz_in) * (3)) ) {
            S->FIR_Fracs = 3;
            S->FIR_Order = 18;
            S->Coefs = silk_Resampler_3_4_COEFS;
        } else if( ((Fs_Hz_out) * (3)) == ((Fs_Hz_in) * (2)) ) {
            S->FIR_Fracs = 2;
            S->FIR_Order = 18;
            S->Coefs = silk_Resampler_2_3_COEFS;
        } else if( ((Fs_Hz_out) * (2)) == Fs_Hz_in ) {
            S->FIR_Fracs = 1;
            S->FIR_Order = 24;
            S->Coefs = silk_Resampler_1_2_COEFS;
        } else if( ((Fs_Hz_out) * (3)) == Fs_Hz_in ) {
            S->FIR_Fracs = 1;
            S->FIR_Order = 36;
            S->Coefs = silk_Resampler_1_3_COEFS;
        } else if( ((Fs_Hz_out) * (4)) == Fs_Hz_in ) {
            S->FIR_Fracs = 1;
            S->FIR_Order = 36;
            S->Coefs = silk_Resampler_1_4_COEFS;
        } else if( ((Fs_Hz_out) * (6)) == Fs_Hz_in ) {
            S->FIR_Fracs = 1;
            S->FIR_Order = 36;
            S->Coefs = silk_Resampler_1_6_COEFS;
        } else {

            ;
            return -1;
        }
    } else {

        S->resampler_function = (0);
    }


    S->invRatio_Q16 = ((opus_int32)((opus_uint32)(((opus_int32)((((opus_int32)((opus_uint32)(Fs_Hz_in)<<(14 + up2x)))) / (Fs_Hz_out))))<<(2)));

    while( ((opus_int32)(((opus_int64)(S->invRatio_Q16) * (Fs_Hz_out)) >> 16)) < ((opus_int32)((opus_uint32)(Fs_Hz_in)<<(up2x))) ) {
        S->invRatio_Q16++;
    }

    return 0;
}



int silk_resampler(
    silk_resampler_state_struct *S,
    opus_int16 out[],
    const opus_int16 in[],
    opus_int32 inLen
)
{
    int nSamples;


    ;

    ;

    nSamples = S->Fs_in_kHz - S->inputDelay;


    memcpy((&S->delayBuf[ S->inputDelay ]), (in), (nSamples * sizeof( opus_int16 )));

    switch( S->resampler_function ) {
        case (1):
            silk_resampler_private_up2_HQ_wrapper( S, out, S->delayBuf, S->Fs_in_kHz );
            silk_resampler_private_up2_HQ_wrapper( S, &out[ S->Fs_out_kHz ], &in[ nSamples ], inLen - S->Fs_in_kHz );
            break;
        case (2):
            silk_resampler_private_IIR_FIR( S, out, S->delayBuf, S->Fs_in_kHz );
            silk_resampler_private_IIR_FIR( S, &out[ S->Fs_out_kHz ], &in[ nSamples ], inLen - S->Fs_in_kHz );
            break;
        case (3):
            silk_resampler_private_down_FIR( S, out, S->delayBuf, S->Fs_in_kHz );
            silk_resampler_private_down_FIR( S, &out[ S->Fs_out_kHz ], &in[ nSamples ], inLen - S->Fs_in_kHz );
            break;
        default:
            memcpy((out), (S->delayBuf), (S->Fs_in_kHz * sizeof( opus_int16 )));
            memcpy((&out[ S->Fs_out_kHz ]), (&in[ nSamples ]), (( inLen - S->Fs_in_kHz ) * sizeof( opus_int16 )));
    }


    memcpy((S->delayBuf), (&in[ inLen - S->inputDelay ]), (S->inputDelay * sizeof( opus_int16 )));

    return 0;
}
void silk_resampler_private_AR2(
    opus_int32 S[],
    opus_int32 out_Q8[],
    const opus_int16 in[],
    const opus_int16 A_Q14[],
    opus_int32 len
)
{
    opus_int32 k;
    opus_int32 out32;

    for( k = 0; k < len; k++ ) {
        out32 = (((S[ 0 ])) + (((opus_int32)((opus_uint32)(((opus_int32)in[ k ]))<<((8))))));
        out_Q8[ k ] = out32;
        out32 = ((opus_int32)((opus_uint32)(out32)<<(2)));
        S[ 0 ] = ((opus_int32)((S[ 1 ]) + (((out32) * (opus_int64)((opus_int16)(A_Q14[ 0 ]))) >> 16)));
        S[ 1 ] = ((opus_int32)(((out32) * (opus_int64)((opus_int16)(A_Q14[ 1 ]))) >> 16));
    }
}
static inline opus_int16 *silk_resampler_private_down_FIR_INTERPOL(
    opus_int16 *out,
    opus_int32 *buf,
    const opus_int16 *FIR_Coefs,
    int FIR_Order,
    int FIR_Fracs,
    opus_int32 max_index_Q16,
    opus_int32 index_increment_Q16
)
{
    opus_int32 index_Q16, res_Q6;
    opus_int32 *buf_ptr;
    opus_int32 interpol_ind;
    const opus_int16 *interpol_ptr;

    switch( FIR_Order ) {
        case 18:
            for( index_Q16 = 0; index_Q16 < max_index_Q16; index_Q16 += index_increment_Q16 ) {

                buf_ptr = buf + ((index_Q16)>>(16));


                interpol_ind = ((opus_int32)(((index_Q16 & 0xFFFF) * (opus_int64)((opus_int16)(FIR_Fracs))) >> 16));


                interpol_ptr = &FIR_Coefs[ 18 / 2 * interpol_ind ];
                res_Q6 = ((opus_int32)(((buf_ptr[ 0 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 0 ]))) >> 16));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 1 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 1 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 2 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 2 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 3 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 3 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 4 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 4 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 5 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 5 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 6 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 6 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 7 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 7 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 8 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 8 ]))) >> 16)));
                interpol_ptr = &FIR_Coefs[ 18 / 2 * ( FIR_Fracs - 1 - interpol_ind ) ];
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 17 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 0 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 16 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 1 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 15 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 2 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 14 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 3 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 13 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 4 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 12 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 5 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 11 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 6 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 10 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 7 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 9 ]) * (opus_int64)((opus_int16)(interpol_ptr[ 8 ]))) >> 16)));


                *out++ = (opus_int16)((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1))));
            }
            break;
        case 24:
            for( index_Q16 = 0; index_Q16 < max_index_Q16; index_Q16 += index_increment_Q16 ) {

                buf_ptr = buf + ((index_Q16)>>(16));


                res_Q6 = ((opus_int32)(((((buf_ptr[ 0 ]) + (buf_ptr[ 23 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 0 ]))) >> 16));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 1 ]) + (buf_ptr[ 22 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 1 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 2 ]) + (buf_ptr[ 21 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 2 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 3 ]) + (buf_ptr[ 20 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 3 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 4 ]) + (buf_ptr[ 19 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 4 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 5 ]) + (buf_ptr[ 18 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 5 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 6 ]) + (buf_ptr[ 17 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 6 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 7 ]) + (buf_ptr[ 16 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 7 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 8 ]) + (buf_ptr[ 15 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 8 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 9 ]) + (buf_ptr[ 14 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 9 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 10 ]) + (buf_ptr[ 13 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 10 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 11 ]) + (buf_ptr[ 12 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 11 ]))) >> 16)));


                *out++ = (opus_int16)((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1))));
            }
            break;
        case 36:
            for( index_Q16 = 0; index_Q16 < max_index_Q16; index_Q16 += index_increment_Q16 ) {

                buf_ptr = buf + ((index_Q16)>>(16));


                res_Q6 = ((opus_int32)(((((buf_ptr[ 0 ]) + (buf_ptr[ 35 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 0 ]))) >> 16));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 1 ]) + (buf_ptr[ 34 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 1 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 2 ]) + (buf_ptr[ 33 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 2 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 3 ]) + (buf_ptr[ 32 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 3 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 4 ]) + (buf_ptr[ 31 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 4 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 5 ]) + (buf_ptr[ 30 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 5 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 6 ]) + (buf_ptr[ 29 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 6 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 7 ]) + (buf_ptr[ 28 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 7 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 8 ]) + (buf_ptr[ 27 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 8 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 9 ]) + (buf_ptr[ 26 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 9 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 10 ]) + (buf_ptr[ 25 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 10 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 11 ]) + (buf_ptr[ 24 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 11 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 12 ]) + (buf_ptr[ 23 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 12 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 13 ]) + (buf_ptr[ 22 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 13 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 14 ]) + (buf_ptr[ 21 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 14 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 15 ]) + (buf_ptr[ 20 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 15 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 16 ]) + (buf_ptr[ 19 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 16 ]))) >> 16)));
                res_Q6 = ((opus_int32)((res_Q6) + (((((buf_ptr[ 17 ]) + (buf_ptr[ 18 ]))) * (opus_int64)((opus_int16)(FIR_Coefs[ 17 ]))) >> 16)));


                *out++ = (opus_int16)((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1))));
            }
            break;
        default:
            ;
    }
    return out;
}


void silk_resampler_private_down_FIR(
    void *SS,
    opus_int16 out[],
    const opus_int16 in[],
    opus_int32 inLen
)
{
    silk_resampler_state_struct *S = (silk_resampler_state_struct *)SS;
    opus_int32 nSamplesIn;
    opus_int32 max_index_Q16, index_increment_Q16;
    ;
    const opus_int16 *FIR_Coefs;
    ;

    opus_int32 *buf = (opus_int32*)DR_OPUS_ALLOCA(sizeof(opus_int32)*(S->batchSize + S->FIR_Order));


    memcpy((buf), (S->sFIR.i32), (S->FIR_Order * sizeof( opus_int32 )));

    FIR_Coefs = &S->Coefs[ 2 ];


    index_increment_Q16 = S->invRatio_Q16;
    while( 1 ) {
        nSamplesIn = (((inLen) < (S->batchSize)) ? (inLen) : (S->batchSize));


        silk_resampler_private_AR2( S->sIIR, &buf[ S->FIR_Order ], in, S->Coefs, nSamplesIn );

        max_index_Q16 = ((opus_int32)((opus_uint32)(nSamplesIn)<<(16)));


        out = silk_resampler_private_down_FIR_INTERPOL( out, buf, FIR_Coefs, S->FIR_Order,
            S->FIR_Fracs, max_index_Q16, index_increment_Q16 );

        in += nSamplesIn;
        inLen -= nSamplesIn;

        if( inLen > 1 ) {

            memcpy((buf), (&buf[ nSamplesIn ]), (S->FIR_Order * sizeof( opus_int32 )));
        } else {
            break;
        }
    }


    memcpy((S->sFIR.i32), (&buf[ nSamplesIn ]), (S->FIR_Order * sizeof( opus_int32 )));
    ;
}
static inline opus_int16 *silk_resampler_private_IIR_FIR_INTERPOL(
    opus_int16 *out,
    opus_int16 *buf,
    opus_int32 max_index_Q16,
    opus_int32 index_increment_Q16
)
{
    opus_int32 index_Q16, res_Q15;
    opus_int16 *buf_ptr;
    opus_int32 table_index;


    for( index_Q16 = 0; index_Q16 < max_index_Q16; index_Q16 += index_increment_Q16 ) {
        table_index = ((opus_int32)(((index_Q16 & 0xFFFF) * (opus_int64)((opus_int16)(12))) >> 16));
        buf_ptr = &buf[ index_Q16 >> 16 ];

        res_Q15 = ((opus_int32)((opus_int16)(buf_ptr[ 0 ])) * (opus_int32)((opus_int16)(silk_resampler_frac_FIR_12[ table_index ][ 0 ])));
        res_Q15 = ((res_Q15) + ((opus_int32)((opus_int16)(buf_ptr[ 1 ]))) * (opus_int32)((opus_int16)(silk_resampler_frac_FIR_12[ table_index ][ 1 ])));
        res_Q15 = ((res_Q15) + ((opus_int32)((opus_int16)(buf_ptr[ 2 ]))) * (opus_int32)((opus_int16)(silk_resampler_frac_FIR_12[ table_index ][ 2 ])));
        res_Q15 = ((res_Q15) + ((opus_int32)((opus_int16)(buf_ptr[ 3 ]))) * (opus_int32)((opus_int16)(silk_resampler_frac_FIR_12[ table_index ][ 3 ])));
        res_Q15 = ((res_Q15) + ((opus_int32)((opus_int16)(buf_ptr[ 4 ]))) * (opus_int32)((opus_int16)(silk_resampler_frac_FIR_12[ 11 - table_index ][ 3 ])));
        res_Q15 = ((res_Q15) + ((opus_int32)((opus_int16)(buf_ptr[ 5 ]))) * (opus_int32)((opus_int16)(silk_resampler_frac_FIR_12[ 11 - table_index ][ 2 ])));
        res_Q15 = ((res_Q15) + ((opus_int32)((opus_int16)(buf_ptr[ 6 ]))) * (opus_int32)((opus_int16)(silk_resampler_frac_FIR_12[ 11 - table_index ][ 1 ])));
        res_Q15 = ((res_Q15) + ((opus_int32)((opus_int16)(buf_ptr[ 7 ]))) * (opus_int32)((opus_int16)(silk_resampler_frac_FIR_12[ 11 - table_index ][ 0 ])));
        *out++ = (opus_int16)((((15) == 1 ? ((res_Q15) >> 1) + ((res_Q15) & 1) : (((res_Q15) >> ((15) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((15) == 1 ? ((res_Q15) >> 1) + ((res_Q15) & 1) : (((res_Q15) >> ((15) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((15) == 1 ? ((res_Q15) >> 1) + ((res_Q15) & 1) : (((res_Q15) >> ((15) - 1)) + 1) >> 1))));
    }
    return out;
}

void silk_resampler_private_IIR_FIR(
    void *SS,
    opus_int16 out[],
    const opus_int16 in[],
    opus_int32 inLen
)
{
    silk_resampler_state_struct *S = (silk_resampler_state_struct *)SS;
    opus_int32 nSamplesIn;
    opus_int32 max_index_Q16, index_increment_Q16;
    ;
    ;

    opus_int16 *buf = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*(2 * S->batchSize + 8));


    memcpy((buf), (S->sFIR.i16), (8 * sizeof( opus_int16 )));


    index_increment_Q16 = S->invRatio_Q16;
    while( 1 ) {
        nSamplesIn = (((inLen) < (S->batchSize)) ? (inLen) : (S->batchSize));


        silk_resampler_private_up2_HQ( S->sIIR, &buf[ 8 ], in, nSamplesIn );

        max_index_Q16 = ((opus_int32)((opus_uint32)(nSamplesIn)<<(16 + 1)));
        out = silk_resampler_private_IIR_FIR_INTERPOL( out, buf, max_index_Q16, index_increment_Q16 );
        in += nSamplesIn;
        inLen -= nSamplesIn;

        if( inLen > 0 ) {

            memcpy((buf), (&buf[ nSamplesIn << 1 ]), (8 * sizeof( opus_int16 )));
        } else {
            break;
        }
    }


    memcpy((S->sFIR.i16), (&buf[ nSamplesIn << 1 ]), (8 * sizeof( opus_int16 )));
    ;
}
void silk_resampler_private_up2_HQ(
    opus_int32 *S,
    opus_int16 *out,
    const opus_int16 *in,
    opus_int32 len
)
{
    opus_int32 k;
    opus_int32 in32, out32_1, out32_2, Y, X;

    ;
    ;
    ;
    ;
    ;
    ;


    for( k = 0; k < len; k++ ) {

        in32 = ((opus_int32)((opus_uint32)((opus_int32)in[ k ])<<(10)));


        Y = ((in32) - (S[ 0 ]));
        X = ((opus_int32)(((Y) * (opus_int64)((opus_int16)(silk_resampler_up2_hq_0[ 0 ]))) >> 16));
        out32_1 = ((S[ 0 ]) + (X));
        S[ 0 ] = ((in32) + (X));


        Y = ((out32_1) - (S[ 1 ]));
        X = ((opus_int32)(((Y) * (opus_int64)((opus_int16)(silk_resampler_up2_hq_0[ 1 ]))) >> 16));
        out32_2 = ((S[ 1 ]) + (X));
        S[ 1 ] = ((out32_1) + (X));


        Y = ((out32_2) - (S[ 2 ]));
        X = ((opus_int32)((Y) + (((Y) * (opus_int64)((opus_int16)(silk_resampler_up2_hq_0[ 2 ]))) >> 16)));
        out32_1 = ((S[ 2 ]) + (X));
        S[ 2 ] = ((out32_2) + (X));


        out[ 2 * k ] = (opus_int16)((((10) == 1 ? ((out32_1) >> 1) + ((out32_1) & 1) : (((out32_1) >> ((10) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((10) == 1 ? ((out32_1) >> 1) + ((out32_1) & 1) : (((out32_1) >> ((10) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((10) == 1 ? ((out32_1) >> 1) + ((out32_1) & 1) : (((out32_1) >> ((10) - 1)) + 1) >> 1))));


        Y = ((in32) - (S[ 3 ]));
        X = ((opus_int32)(((Y) * (opus_int64)((opus_int16)(silk_resampler_up2_hq_1[ 0 ]))) >> 16));
        out32_1 = ((S[ 3 ]) + (X));
        S[ 3 ] = ((in32) + (X));


        Y = ((out32_1) - (S[ 4 ]));
        X = ((opus_int32)(((Y) * (opus_int64)((opus_int16)(silk_resampler_up2_hq_1[ 1 ]))) >> 16));
        out32_2 = ((S[ 4 ]) + (X));
        S[ 4 ] = ((out32_1) + (X));


        Y = ((out32_2) - (S[ 5 ]));
        X = ((opus_int32)((Y) + (((Y) * (opus_int64)((opus_int16)(silk_resampler_up2_hq_1[ 2 ]))) >> 16)));
        out32_1 = ((S[ 5 ]) + (X));
        S[ 5 ] = ((out32_2) + (X));


        out[ 2 * k + 1 ] = (opus_int16)((((10) == 1 ? ((out32_1) >> 1) + ((out32_1) & 1) : (((out32_1) >> ((10) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((10) == 1 ? ((out32_1) >> 1) + ((out32_1) & 1) : (((out32_1) >> ((10) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((10) == 1 ? ((out32_1) >> 1) + ((out32_1) & 1) : (((out32_1) >> ((10) - 1)) + 1) >> 1))));
    }
}

void silk_resampler_private_up2_HQ_wrapper(
    void *SS,
    opus_int16 *out,
    const opus_int16 *in,
    opus_int32 len
)
{
    silk_resampler_state_struct *S = (silk_resampler_state_struct *)SS;
    silk_resampler_private_up2_HQ( S->sIIR, out, in, len );
}
void silk_resampler_down2(
    opus_int32 *S,
    opus_int16 *out,
    const opus_int16 *in,
    opus_int32 inLen
)
{
    opus_int32 k, len2 = ((inLen)>>(1));
    opus_int32 in32, out32, Y, X;

    ;
    ;


    for( k = 0; k < len2; k++ ) {

        in32 = ((opus_int32)((opus_uint32)((opus_int32)in[ 2 * k ])<<(10)));


        Y = ((in32) - (S[ 0 ]));
        X = ((opus_int32)((Y) + (((Y) * (opus_int64)((opus_int16)(silk_resampler_down2_1))) >> 16)));
        out32 = ((S[ 0 ]) + (X));
        S[ 0 ] = ((in32) + (X));


        in32 = ((opus_int32)((opus_uint32)((opus_int32)in[ 2 * k + 1 ])<<(10)));


        Y = ((in32) - (S[ 1 ]));
        X = ((opus_int32)(((Y) * (opus_int64)((opus_int16)(silk_resampler_down2_0))) >> 16));
        out32 = ((out32) + (S[ 1 ]));
        out32 = ((out32) + (X));
        S[ 1 ] = ((in32) + (X));


        out[ k ] = (opus_int16)((((11) == 1 ? ((out32) >> 1) + ((out32) & 1) : (((out32) >> ((11) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((11) == 1 ? ((out32) >> 1) + ((out32) & 1) : (((out32) >> ((11) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((11) == 1 ? ((out32) >> 1) + ((out32) & 1) : (((out32) >> ((11) - 1)) + 1) >> 1))));
    }
}
void silk_resampler_down2_3(
    opus_int32 *S,
    opus_int16 *out,
    const opus_int16 *in,
    opus_int32 inLen
)
{
    opus_int32 nSamplesIn, counter, res_Q6;
    ;
    opus_int32 *buf_ptr;
    ;

    opus_int32 buf[( 10 * 48 ) + 4];


    memcpy((buf), (S), (4 * sizeof( opus_int32 )));


    while( 1 ) {
        nSamplesIn = (((inLen) < (( 10 * 48 ))) ? (inLen) : (( 10 * 48 )));


        silk_resampler_private_AR2( &S[ 4 ], &buf[ 4 ], in,
            silk_Resampler_2_3_COEFS_LQ, nSamplesIn );


        buf_ptr = buf;
        counter = nSamplesIn;
        while( counter > 2 ) {

            res_Q6 = ((opus_int32)(((buf_ptr[ 0 ]) * (opus_int64)((opus_int16)(silk_Resampler_2_3_COEFS_LQ[ 2 ]))) >> 16));
            res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 1 ]) * (opus_int64)((opus_int16)(silk_Resampler_2_3_COEFS_LQ[ 3 ]))) >> 16)));
            res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 2 ]) * (opus_int64)((opus_int16)(silk_Resampler_2_3_COEFS_LQ[ 5 ]))) >> 16)));
            res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 3 ]) * (opus_int64)((opus_int16)(silk_Resampler_2_3_COEFS_LQ[ 4 ]))) >> 16)));


            *out++ = (opus_int16)((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1))));

            res_Q6 = ((opus_int32)(((buf_ptr[ 1 ]) * (opus_int64)((opus_int16)(silk_Resampler_2_3_COEFS_LQ[ 4 ]))) >> 16));
            res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 2 ]) * (opus_int64)((opus_int16)(silk_Resampler_2_3_COEFS_LQ[ 5 ]))) >> 16)));
            res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 3 ]) * (opus_int64)((opus_int16)(silk_Resampler_2_3_COEFS_LQ[ 3 ]))) >> 16)));
            res_Q6 = ((opus_int32)((res_Q6) + (((buf_ptr[ 4 ]) * (opus_int64)((opus_int16)(silk_Resampler_2_3_COEFS_LQ[ 2 ]))) >> 16)));


            *out++ = (opus_int16)((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((6) == 1 ? ((res_Q6) >> 1) + ((res_Q6) & 1) : (((res_Q6) >> ((6) - 1)) + 1) >> 1))));

            buf_ptr += 3;
            counter -= 3;
        }

        in += nSamplesIn;
        inLen -= nSamplesIn;

        if( inLen > 0 ) {

            memcpy((buf), (&buf[ nSamplesIn ]), (4 * sizeof( opus_int32 )));
        } else {
            break;
        }
    }


    memcpy((S), (&buf[ nSamplesIn ]), (4 * sizeof( opus_int32 )));
    ;
}
void silk_insertion_sort_increasing(
    opus_int32 *a,
    int *idx,
    const int L,
    const int K
)
{
    opus_int32 value;
    int i, j;


    ;
    ;
    ;


    for( i = 0; i < K; i++ ) {
        idx[ i ] = i;
    }


    for( i = 1; i < K; i++ ) {
        value = a[ i ];
        for( j = i - 1; ( j >= 0 ) && ( value < a[ j ] ); j-- ) {
            a[ j + 1 ] = a[ j ];
            idx[ j + 1 ] = idx[ j ];
        }
        a[ j + 1 ] = value;
        idx[ j + 1 ] = i;
    }



    for( i = K; i < L; i++ ) {
        value = a[ i ];
        if( value < a[ K - 1 ] ) {
            for( j = K - 2; ( j >= 0 ) && ( value < a[ j ] ); j-- ) {
                a[ j + 1 ] = a[ j ];
                idx[ j + 1 ] = idx[ j ];
            }
            a[ j + 1 ] = value;
            idx[ j + 1 ] = i;
        }
    }
}
void silk_insertion_sort_increasing_all_values_int16(
     opus_int16 *a,
     const int L
)
{
    int value;
    int i, j;


    ;


    for( i = 1; i < L; i++ ) {
        value = a[ i ];
        for( j = i - 1; ( j >= 0 ) && ( value < a[ j ] ); j-- ) {
            a[ j + 1 ] = a[ j ];
        }
        a[ j + 1 ] = value;
    }
}
void silk_sum_sqr_shift(
    opus_int32 *energy,
    int *shift,
    const opus_int16 *x,
    int len
)
{
    int i, shft;
    opus_uint32 nrg_tmp;
    opus_int32 nrg;


    shft = 31-silk_CLZ32(len);

    nrg = len;
    for( i = 0; i < len - 1; i += 2 ) {
        nrg_tmp = ((opus_int32)((opus_int16)(x[ i ])) * (opus_int32)((opus_int16)(x[ i ])));
        nrg_tmp = (((opus_int32)((opus_uint32)((nrg_tmp)) + (opus_uint32)(((opus_int32)((opus_int16)(x[ i + 1 ]))) * (opus_int32)((opus_int16)(x[ i + 1 ]))))));
        nrg = (opus_int32)((nrg) + (((nrg_tmp)) >> ((shft))));
    }
    if( i < len ) {

        nrg_tmp = ((opus_int32)((opus_int16)(x[ i ])) * (opus_int32)((opus_int16)(x[ i ])));
        nrg = (opus_int32)((nrg) + (((nrg_tmp)) >> ((shft))));
    }
    ;


    shft = silk_max_32(0, shft+3 - silk_CLZ32(nrg));
    nrg = 0;
    for( i = 0 ; i < len - 1; i += 2 ) {
        nrg_tmp = ((opus_int32)((opus_int16)(x[ i ])) * (opus_int32)((opus_int16)(x[ i ])));
        nrg_tmp = (((opus_int32)((opus_uint32)((nrg_tmp)) + (opus_uint32)(((opus_int32)((opus_int16)(x[ i + 1 ]))) * (opus_int32)((opus_int16)(x[ i + 1 ]))))));
        nrg = (opus_int32)((nrg) + (((nrg_tmp)) >> ((shft))));
    }
    if( i < len ) {

        nrg_tmp = ((opus_int32)((opus_int16)(x[ i ])) * (opus_int32)((opus_int16)(x[ i ])));
        nrg = (opus_int32)((nrg) + (((nrg_tmp)) >> ((shft))));
    }

    ;


    *shift = shft;
    *energy = nrg;
}
opus_int32 silk_lin2log(
    const opus_int32 inLin
)
{
    opus_int32 lz, frac_Q7;

    silk_CLZ_FRAC( inLin, &lz, &frac_Q7 );


    return (((((opus_int32)((frac_Q7) + (((((frac_Q7) * (128 - frac_Q7))) * (opus_int64)((opus_int16)(179))) >> 16))))) + (((opus_int32)((opus_uint32)((31 - lz))<<((7))))));
}
opus_int32 silk_log2lin(
    const opus_int32 inLog_Q7
)
{
    opus_int32 out, frac_Q7;

    if( inLog_Q7 < 0 ) {
        return 0;
    } else if ( inLog_Q7 >= 3967 ) {
        return 0x7FFFFFFF;
    }

    out = ((opus_int32)((opus_uint32)(1)<<(((inLog_Q7)>>(7)))));
    frac_Q7 = inLog_Q7 & 0x7F;
    if( inLog_Q7 < 2048 ) {

        out = (((out)) + ((((((out) * (((opus_int32)((frac_Q7) + (((((opus_int32)((opus_int16)(frac_Q7)) * (opus_int32)((opus_int16)(128 - frac_Q7)))) * (opus_int64)((opus_int16)(-174))) >> 16)))))))>>((7)))));
    } else {

        out = (((out)) + (((((out)>>(7))) * (((opus_int32)((frac_Q7) + (((((opus_int32)((opus_int16)(frac_Q7)) * (opus_int32)((opus_int16)(128 - frac_Q7)))) * (opus_int64)((opus_int16)(-174))) >> 16)))))));
    }
    return out;
}
void silk_gains_quant(
    opus_int8 ind[ 4 ],
    opus_int32 gain_Q16[ 4 ],
    opus_int8 *prev_ind,
    const int conditional,
    const int nb_subfr
)
{
    int k, double_step_size_threshold;

    for( k = 0; k < nb_subfr; k++ ) {

        ind[ k ] = ((opus_int32)(((( ( 65536 * ( 64 - 1 ) ) / ( ( ( 88 - 2 ) * 128 ) / 6 ) )) * (opus_int64)((opus_int16)(silk_lin2log( gain_Q16[ k ] ) - ( ( 2 * 128 ) / 6 + 16 * 128 )))) >> 16));


        if( ind[ k ] < *prev_ind ) {
            ind[ k ]++;
        }
        ind[ k ] = ((0) > (64 - 1) ? ((ind[ k ]) > (0) ? (0) : ((ind[ k ]) < (64 - 1) ? (64 - 1) : (ind[ k ]))) : ((ind[ k ]) > (64 - 1) ? (64 - 1) : ((ind[ k ]) < (0) ? (0) : (ind[ k ]))));


        if( k == 0 && conditional == 0 ) {

            ind[ k ] = ((*prev_ind + -4) > (64 - 1) ? ((ind[ k ]) > (*prev_ind + -4) ? (*prev_ind + -4) : ((ind[ k ]) < (64 - 1) ? (64 - 1) : (ind[ k ]))) : ((ind[ k ]) > (64 - 1) ? (64 - 1) : ((ind[ k ]) < (*prev_ind + -4) ? (*prev_ind + -4) : (ind[ k ]))));
            *prev_ind = ind[ k ];
        } else {

            ind[ k ] = ind[ k ] - *prev_ind;


            double_step_size_threshold = 2 * 36 - 64 + *prev_ind;
            if( ind[ k ] > double_step_size_threshold ) {
                ind[ k ] = double_step_size_threshold + ((ind[ k ] - double_step_size_threshold + 1)>>(1));
            }

            ind[ k ] = ((-4) > (36) ? ((ind[ k ]) > (-4) ? (-4) : ((ind[ k ]) < (36) ? (36) : (ind[ k ]))) : ((ind[ k ]) > (36) ? (36) : ((ind[ k ]) < (-4) ? (-4) : (ind[ k ]))));


            if( ind[ k ] > double_step_size_threshold ) {
                *prev_ind += ((opus_int32)((opus_uint32)(ind[ k ])<<(1))) - double_step_size_threshold;
                *prev_ind = silk_min_int( *prev_ind, 64 - 1 );
            } else {
                *prev_ind += ind[ k ];
            }


            ind[ k ] -= -4;
        }


        gain_Q16[ k ] = silk_log2lin( silk_min_32( ((opus_int32)(((( ( 65536 * ( ( ( 88 - 2 ) * 128 ) / 6 ) ) / ( 64 - 1 ) )) * (opus_int64)((opus_int16)(*prev_ind))) >> 16)) + ( ( 2 * 128 ) / 6 + 16 * 128 ), 3967 ) );
    }
}


void silk_gains_dequant(
    opus_int32 gain_Q16[ 4 ],
    const opus_int8 ind[ 4 ],
    opus_int8 *prev_ind,
    const int conditional,
    const int nb_subfr
)
{
    int k, ind_tmp, double_step_size_threshold;

    for( k = 0; k < nb_subfr; k++ ) {
        if( k == 0 && conditional == 0 ) {

            *prev_ind = silk_max_int( ind[ k ], *prev_ind - 16 );
        } else {

            ind_tmp = ind[ k ] + -4;


            double_step_size_threshold = 2 * 36 - 64 + *prev_ind;
            if( ind_tmp > double_step_size_threshold ) {
                *prev_ind += ((opus_int32)((opus_uint32)(ind_tmp)<<(1))) - double_step_size_threshold;
            } else {
                *prev_ind += ind_tmp;
            }
        }
        *prev_ind = ((0) > (64 - 1) ? ((*prev_ind) > (0) ? (0) : ((*prev_ind) < (64 - 1) ? (64 - 1) : (*prev_ind))) : ((*prev_ind) > (64 - 1) ? (64 - 1) : ((*prev_ind) < (0) ? (0) : (*prev_ind))));


        gain_Q16[ k ] = silk_log2lin( silk_min_32( ((opus_int32)(((( ( 65536 * ( ( ( 88 - 2 ) * 128 ) / 6 ) ) / ( 64 - 1 ) )) * (opus_int64)((opus_int16)(*prev_ind))) >> 16)) + ( ( 2 * 128 ) / 6 + 16 * 128 ), 3967 ) );
    }
}


opus_int32 silk_gains_ID(
    const opus_int8 ind[ 4 ],
    const int nb_subfr
)
{
    int k;
    opus_int32 gainsID;

    gainsID = 0;
    for( k = 0; k < nb_subfr; k++ ) {
        gainsID = (((ind[ k ])) + (((opus_int32)((opus_uint32)((gainsID))<<((8))))));
    }

    return gainsID;
}
const opus_int8 silk_CB_lags_stage2_10_ms[ 4 >> 1][ 3 ] =
{
    {0, 1, 0},
    {0, 0, 1}
};

const opus_int8 silk_CB_lags_stage3_10_ms[ 4 >> 1 ][ 12 ] =
{
    { 0, 0, 1,-1, 1,-1, 2,-2, 2,-2, 3,-3},
    { 0, 1, 0, 1,-1, 2,-1, 2,-2, 3,-2, 3}
};

const opus_int8 silk_Lag_range_stage3_10_ms[ 4 >> 1 ][ 2 ] =
{
    {-3, 7},
    {-2, 7}
};

const opus_int8 silk_CB_lags_stage2[ 4 ][ 11 ] =
{
    {0, 2,-1,-1,-1, 0, 0, 1, 1, 0, 1},
    {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
    {0,-1, 2, 1, 0, 1, 1, 0, 0,-1,-1}
};

const opus_int8 silk_CB_lags_stage3[ 4 ][ 34 ] =
{
    {0, 0, 1,-1, 0, 1,-1, 0,-1, 1,-2, 2,-2,-2, 2,-3, 2, 3,-3,-4, 3,-4, 4, 4,-5, 5,-6,-5, 6,-7, 6, 5, 8,-9},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0,-1, 1, 0, 0, 1,-1, 0, 1,-1,-1, 1,-1, 2, 1,-1, 2,-2,-2, 2,-2, 2, 2, 3,-3},
    {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1,-1, 1, 0, 0, 2, 1,-1, 2,-1,-1, 2,-1, 2, 2,-1, 3,-2,-2,-2, 3},
    {0, 1, 0, 0, 1, 0, 1,-1, 2,-1, 2,-1, 2, 3,-2, 3,-2,-2, 4, 4,-3, 5,-3,-4, 6,-4, 6, 5,-5, 8,-6,-5,-7, 9}
};

const opus_int8 silk_Lag_range_stage3[ 2 + 1 ] [ 4 ][ 2 ] =
{

    {
        {-5,8},
        {-1,6},
        {-1,6},
        {-4,10}
    },

    {
        {-6,10},
        {-2,6},
        {-1,6},
        {-5,10}
    },

    {
        {-9,12},
        {-3,7},
        {-2,7},
        {-7,13}
    }
};

const opus_int8 silk_nb_cbk_searchs_stage3[ 2 + 1 ] =
{
    16,
    24,
    34
};
const opus_uint8 silk_gain_iCDF[ 3 ][ 64 / 8 ] =
{
{
       224, 112, 44, 15, 3, 2, 1, 0
},
{
       254, 237, 192, 132, 70, 23, 4, 0
},
{
       255, 252, 226, 155, 61, 11, 2, 0
}
};

const opus_uint8 silk_delta_gain_iCDF[ 36 - -4 + 1 ] = {
       250, 245, 234, 203, 71, 50, 42, 38,
        35, 33, 31, 29, 28, 27, 26, 25,
        24, 23, 22, 21, 20, 19, 18, 17,
        16, 15, 14, 13, 12, 11, 10, 9,
         8, 7, 6, 5, 4, 3, 2, 1,
         0
};
const opus_uint8 silk_LTP_per_index_iCDF[3] = {
       179, 99, 0
};

static const opus_uint8 silk_LTP_gain_iCDF_0[8] = {
        71, 56, 43, 30, 21, 12, 6, 0
};

static const opus_uint8 silk_LTP_gain_iCDF_1[16] = {
       199, 165, 144, 124, 109, 96, 84, 71,
        61, 51, 42, 32, 23, 15, 8, 0
};

static const opus_uint8 silk_LTP_gain_iCDF_2[32] = {
       241, 225, 211, 199, 187, 175, 164, 153,
       142, 132, 123, 114, 105, 96, 88, 80,
        72, 64, 57, 50, 44, 38, 33, 29,
        24, 20, 16, 12, 9, 5, 2, 0
};

static const opus_uint8 silk_LTP_gain_BITS_Q5_0[8] = {
        15, 131, 138, 138, 155, 155, 173, 173
};

static const opus_uint8 silk_LTP_gain_BITS_Q5_1[16] = {
        69, 93, 115, 118, 131, 138, 141, 138,
       150, 150, 155, 150, 155, 160, 166, 160
};

static const opus_uint8 silk_LTP_gain_BITS_Q5_2[32] = {
       131, 128, 134, 141, 141, 141, 145, 145,
       145, 150, 155, 155, 155, 155, 160, 160,
       160, 160, 166, 166, 173, 173, 182, 192,
       182, 192, 192, 192, 205, 192, 205, 224
};

const opus_uint8 * const silk_LTP_gain_iCDF_ptrs[3] = {
    silk_LTP_gain_iCDF_0,
    silk_LTP_gain_iCDF_1,
    silk_LTP_gain_iCDF_2
};

const opus_uint8 * const silk_LTP_gain_BITS_Q5_ptrs[3] = {
    silk_LTP_gain_BITS_Q5_0,
    silk_LTP_gain_BITS_Q5_1,
    silk_LTP_gain_BITS_Q5_2
};

static const opus_int8 silk_LTP_gain_vq_0[8][5] =
{
{
         4, 6, 24, 7, 5
},
{
         0, 0, 2, 0, 0
},
{
        12, 28, 41, 13, -4
},
{
        -9, 15, 42, 25, 14
},
{
         1, -2, 62, 41, -9
},
{
       -10, 37, 65, -4, 3
},
{
        -6, 4, 66, 7, -8
},
{
        16, 14, 38, -3, 33
}
};

static const opus_int8 silk_LTP_gain_vq_1[16][5] =
{
{
        13, 22, 39, 23, 12
},
{
        -1, 36, 64, 27, -6
},
{
        -7, 10, 55, 43, 17
},
{
         1, 1, 8, 1, 1
},
{
         6, -11, 74, 53, -9
},
{
       -12, 55, 76, -12, 8
},
{
        -3, 3, 93, 27, -4
},
{
        26, 39, 59, 3, -8
},
{
         2, 0, 77, 11, 9
},
{
        -8, 22, 44, -6, 7
},
{
        40, 9, 26, 3, 9
},
{
        -7, 20, 101, -7, 4
},
{
         3, -8, 42, 26, 0
},
{
       -15, 33, 68, 2, 23
},
{
        -2, 55, 46, -2, 15
},
{
         3, -1, 21, 16, 41
}
};

static const opus_int8 silk_LTP_gain_vq_2[32][5] =
{
{
        -6, 27, 61, 39, 5
},
{
       -11, 42, 88, 4, 1
},
{
        -2, 60, 65, 6, -4
},
{
        -1, -5, 73, 56, 1
},
{
        -9, 19, 94, 29, -9
},
{
         0, 12, 99, 6, 4
},
{
         8, -19, 102, 46, -13
},
{
         3, 2, 13, 3, 2
},
{
         9, -21, 84, 72, -18
},
{
       -11, 46, 104, -22, 8
},
{
        18, 38, 48, 23, 0
},
{
       -16, 70, 83, -21, 11
},
{
         5, -11, 117, 22, -8
},
{
        -6, 23, 117, -12, 3
},
{
         3, -8, 95, 28, 4
},
{
       -10, 15, 77, 60, -15
},
{
        -1, 4, 124, 2, -4
},
{
         3, 38, 84, 24, -25
},
{
         2, 13, 42, 13, 31
},
{
        21, -4, 56, 46, -1
},
{
        -1, 35, 79, -13, 19
},
{
        -7, 65, 88, -9, -14
},
{
        20, 4, 81, 49, -29
},
{
        20, 0, 75, 3, -17
},
{
         5, -9, 44, 92, -8
},
{
         1, -3, 22, 69, 31
},
{
        -6, 95, 41, -12, 5
},
{
        39, 67, 16, -4, 1
},
{
         0, -6, 120, 55, -36
},
{
       -13, 44, 122, 4, -24
},
{
        81, 5, 11, 3, 7
},
{
         2, 0, 9, 10, 88
}
};

const opus_int8 * const silk_LTP_vq_ptrs_Q7[3] = {
    (opus_int8 *)&silk_LTP_gain_vq_0[0][0],
    (opus_int8 *)&silk_LTP_gain_vq_1[0][0],
    (opus_int8 *)&silk_LTP_gain_vq_2[0][0]
};



static const opus_uint8 silk_LTP_gain_vq_0_gain[8] = {
      46, 2, 90, 87, 93, 91, 82, 98
};

static const opus_uint8 silk_LTP_gain_vq_1_gain[16] = {
     109, 120, 118, 12, 113, 115, 117, 119,
      99, 59, 87, 111, 63, 111, 112, 80
};

static const opus_uint8 silk_LTP_gain_vq_2_gain[32] = {
     126, 124, 125, 124, 129, 121, 126, 23,
     132, 127, 127, 127, 126, 127, 122, 133,
     130, 134, 101, 118, 119, 145, 126, 86,
     124, 120, 123, 119, 170, 173, 107, 109
};

const opus_uint8 * const silk_LTP_vq_gain_ptrs_Q7[3] = {
    &silk_LTP_gain_vq_0_gain[0],
    &silk_LTP_gain_vq_1_gain[0],
    &silk_LTP_gain_vq_2_gain[0]
};

const opus_int8 silk_LTP_vq_sizes[3] = {
    8, 16, 32
};
static const opus_uint8 silk_NLSF_CB1_NB_MB_Q8[ 320 ] = {
        12, 35, 60, 83, 108, 132, 157, 180,
       206, 228, 15, 32, 55, 77, 101, 125,
       151, 175, 201, 225, 19, 42, 66, 89,
       114, 137, 162, 184, 209, 230, 12, 25,
        50, 72, 97, 120, 147, 172, 200, 223,
        26, 44, 69, 90, 114, 135, 159, 180,
       205, 225, 13, 22, 53, 80, 106, 130,
       156, 180, 205, 228, 15, 25, 44, 64,
        90, 115, 142, 168, 196, 222, 19, 24,
        62, 82, 100, 120, 145, 168, 190, 214,
        22, 31, 50, 79, 103, 120, 151, 170,
       203, 227, 21, 29, 45, 65, 106, 124,
       150, 171, 196, 224, 30, 49, 75, 97,
       121, 142, 165, 186, 209, 229, 19, 25,
        52, 70, 93, 116, 143, 166, 192, 219,
        26, 34, 62, 75, 97, 118, 145, 167,
       194, 217, 25, 33, 56, 70, 91, 113,
       143, 165, 196, 223, 21, 34, 51, 72,
        97, 117, 145, 171, 196, 222, 20, 29,
        50, 67, 90, 117, 144, 168, 197, 221,
        22, 31, 48, 66, 95, 117, 146, 168,
       196, 222, 24, 33, 51, 77, 116, 134,
       158, 180, 200, 224, 21, 28, 70, 87,
       106, 124, 149, 170, 194, 217, 26, 33,
        53, 64, 83, 117, 152, 173, 204, 225,
        27, 34, 65, 95, 108, 129, 155, 174,
       210, 225, 20, 26, 72, 99, 113, 131,
       154, 176, 200, 219, 34, 43, 61, 78,
        93, 114, 155, 177, 205, 229, 23, 29,
        54, 97, 124, 138, 163, 179, 209, 229,
        30, 38, 56, 89, 118, 129, 158, 178,
       200, 231, 21, 29, 49, 63, 85, 111,
       142, 163, 193, 222, 27, 48, 77, 103,
       133, 158, 179, 196, 215, 232, 29, 47,
        74, 99, 124, 151, 176, 198, 220, 237,
        33, 42, 61, 76, 93, 121, 155, 174,
       207, 225, 29, 53, 87, 112, 136, 154,
       170, 188, 208, 227, 24, 30, 52, 84,
       131, 150, 166, 186, 203, 229, 37, 48,
        64, 84, 104, 118, 156, 177, 201, 230
};

static const opus_int16 silk_NLSF_CB1_Wght_Q9[ 320 ] = {
     2897, 2314, 2314, 2314, 2287, 2287, 2314, 2300, 2327, 2287,
     2888, 2580, 2394, 2367, 2314, 2274, 2274, 2274, 2274, 2194,
     2487, 2340, 2340, 2314, 2314, 2314, 2340, 2340, 2367, 2354,
     3216, 2766, 2340, 2340, 2314, 2274, 2221, 2207, 2261, 2194,
     2460, 2474, 2367, 2394, 2394, 2394, 2394, 2367, 2407, 2314,
     3479, 3056, 2127, 2207, 2274, 2274, 2274, 2287, 2314, 2261,
     3282, 3141, 2580, 2394, 2247, 2221, 2207, 2194, 2194, 2114,
     4096, 3845, 2221, 2620, 2620, 2407, 2314, 2394, 2367, 2074,
     3178, 3244, 2367, 2221, 2553, 2434, 2340, 2314, 2167, 2221,
     3338, 3488, 2726, 2194, 2261, 2460, 2354, 2367, 2207, 2101,
     2354, 2420, 2327, 2367, 2394, 2420, 2420, 2420, 2460, 2367,
     3779, 3629, 2434, 2527, 2367, 2274, 2274, 2300, 2207, 2048,
     3254, 3225, 2713, 2846, 2447, 2327, 2300, 2300, 2274, 2127,
     3263, 3300, 2753, 2806, 2447, 2261, 2261, 2247, 2127, 2101,
     2873, 2981, 2633, 2367, 2407, 2354, 2194, 2247, 2247, 2114,
     3225, 3197, 2633, 2580, 2274, 2181, 2247, 2221, 2221, 2141,
     3178, 3310, 2740, 2407, 2274, 2274, 2274, 2287, 2194, 2114,
     3141, 3272, 2460, 2061, 2287, 2500, 2367, 2487, 2434, 2181,
     3507, 3282, 2314, 2700, 2647, 2474, 2367, 2394, 2340, 2127,
     3423, 3535, 3038, 3056, 2300, 1950, 2221, 2274, 2274, 2274,
     3404, 3366, 2087, 2687, 2873, 2354, 2420, 2274, 2474, 2540,
     3760, 3488, 1950, 2660, 2897, 2527, 2394, 2367, 2460, 2261,
     3028, 3272, 2740, 2888, 2740, 2154, 2127, 2287, 2234, 2247,
     3695, 3657, 2025, 1969, 2660, 2700, 2580, 2500, 2327, 2367,
     3207, 3413, 2354, 2074, 2888, 2888, 2340, 2487, 2247, 2167,
     3338, 3366, 2846, 2780, 2327, 2154, 2274, 2287, 2114, 2061,
     2327, 2300, 2181, 2167, 2181, 2367, 2633, 2700, 2700, 2553,
     2407, 2434, 2221, 2261, 2221, 2221, 2340, 2420, 2607, 2700,
     3038, 3244, 2806, 2888, 2474, 2074, 2300, 2314, 2354, 2380,
     2221, 2154, 2127, 2287, 2500, 2793, 2793, 2620, 2580, 2367,
     3676, 3713, 2234, 1838, 2181, 2753, 2726, 2673, 2513, 2207,
     2793, 3160, 2726, 2553, 2846, 2513, 2181, 2394, 2221, 2181
};

static const opus_uint8 silk_NLSF_CB1_iCDF_NB_MB[ 64 ] = {
       212, 178, 148, 129, 108, 96, 85, 82,
        79, 77, 61, 59, 57, 56, 51, 49,
        48, 45, 42, 41, 40, 38, 36, 34,
        31, 30, 21, 12, 10, 3, 1, 0,
       255, 245, 244, 236, 233, 225, 217, 203,
       190, 176, 175, 161, 149, 136, 125, 114,
       102, 91, 81, 71, 60, 52, 43, 35,
        28, 20, 19, 18, 12, 11, 5, 0
};

static const opus_uint8 silk_NLSF_CB2_SELECT_NB_MB[ 160 ] = {
        16, 0, 0, 0, 0, 99, 66, 36,
        36, 34, 36, 34, 34, 34, 34, 83,
        69, 36, 52, 34, 116, 102, 70, 68,
        68, 176, 102, 68, 68, 34, 65, 85,
        68, 84, 36, 116, 141, 152, 139, 170,
       132, 187, 184, 216, 137, 132, 249, 168,
       185, 139, 104, 102, 100, 68, 68, 178,
       218, 185, 185, 170, 244, 216, 187, 187,
       170, 244, 187, 187, 219, 138, 103, 155,
       184, 185, 137, 116, 183, 155, 152, 136,
       132, 217, 184, 184, 170, 164, 217, 171,
       155, 139, 244, 169, 184, 185, 170, 164,
       216, 223, 218, 138, 214, 143, 188, 218,
       168, 244, 141, 136, 155, 170, 168, 138,
       220, 219, 139, 164, 219, 202, 216, 137,
       168, 186, 246, 185, 139, 116, 185, 219,
       185, 138, 100, 100, 134, 100, 102, 34,
        68, 68, 100, 68, 168, 203, 221, 218,
       168, 167, 154, 136, 104, 70, 164, 246,
       171, 137, 139, 137, 155, 218, 219, 139
};

static const opus_uint8 silk_NLSF_CB2_iCDF_NB_MB[ 72 ] = {
       255, 254, 253, 238, 14, 3, 2, 1,
         0, 255, 254, 252, 218, 35, 3, 2,
         1, 0, 255, 254, 250, 208, 59, 4,
         2, 1, 0, 255, 254, 246, 194, 71,
        10, 2, 1, 0, 255, 252, 236, 183,
        82, 8, 2, 1, 0, 255, 252, 235,
       180, 90, 17, 2, 1, 0, 255, 248,
       224, 171, 97, 30, 4, 1, 0, 255,
       254, 236, 173, 95, 37, 7, 1, 0
};

static const opus_uint8 silk_NLSF_CB2_BITS_NB_MB_Q5[ 72 ] = {
       255, 255, 255, 131, 6, 145, 255, 255,
       255, 255, 255, 236, 93, 15, 96, 255,
       255, 255, 255, 255, 194, 83, 25, 71,
       221, 255, 255, 255, 255, 162, 73, 34,
        66, 162, 255, 255, 255, 210, 126, 73,
        43, 57, 173, 255, 255, 255, 201, 125,
        71, 48, 58, 130, 255, 255, 255, 166,
       110, 73, 57, 62, 104, 210, 255, 255,
       251, 123, 65, 55, 68, 100, 171, 255
};

static const opus_uint8 silk_NLSF_PRED_NB_MB_Q8[ 18 ] = {
       179, 138, 140, 148, 151, 149, 153, 151,
       163, 116, 67, 82, 59, 92, 72, 100,
        89, 92
};

static const opus_int16 silk_NLSF_DELTA_MIN_NB_MB_Q15[ 11 ] = {
       250, 3, 6, 3, 3, 3, 4, 3,
         3, 3, 461
};

const silk_NLSF_CB_struct silk_NLSF_CB_NB_MB =
{
    32,
    10,
    ((opus_int32)((0.18) * ((opus_int64)1 << (16)) + 0.5)),
    ((opus_int32)((1.0 / 0.18) * ((opus_int64)1 << (6)) + 0.5)),
    silk_NLSF_CB1_NB_MB_Q8,
    silk_NLSF_CB1_Wght_Q9,
    silk_NLSF_CB1_iCDF_NB_MB,
    silk_NLSF_PRED_NB_MB_Q8,
    silk_NLSF_CB2_SELECT_NB_MB,
    silk_NLSF_CB2_iCDF_NB_MB,
    silk_NLSF_CB2_BITS_NB_MB_Q5,
    silk_NLSF_DELTA_MIN_NB_MB_Q15,
};
static const opus_uint8 silk_NLSF_CB1_WB_Q8[ 512 ] = {
         7, 23, 38, 54, 69, 85, 100, 116,
       131, 147, 162, 178, 193, 208, 223, 239,
        13, 25, 41, 55, 69, 83, 98, 112,
       127, 142, 157, 171, 187, 203, 220, 236,
        15, 21, 34, 51, 61, 78, 92, 106,
       126, 136, 152, 167, 185, 205, 225, 240,
        10, 21, 36, 50, 63, 79, 95, 110,
       126, 141, 157, 173, 189, 205, 221, 237,
        17, 20, 37, 51, 59, 78, 89, 107,
       123, 134, 150, 164, 184, 205, 224, 240,
        10, 15, 32, 51, 67, 81, 96, 112,
       129, 142, 158, 173, 189, 204, 220, 236,
         8, 21, 37, 51, 65, 79, 98, 113,
       126, 138, 155, 168, 179, 192, 209, 218,
        12, 15, 34, 55, 63, 78, 87, 108,
       118, 131, 148, 167, 185, 203, 219, 236,
        16, 19, 32, 36, 56, 79, 91, 108,
       118, 136, 154, 171, 186, 204, 220, 237,
        11, 28, 43, 58, 74, 89, 105, 120,
       135, 150, 165, 180, 196, 211, 226, 241,
         6, 16, 33, 46, 60, 75, 92, 107,
       123, 137, 156, 169, 185, 199, 214, 225,
        11, 19, 30, 44, 57, 74, 89, 105,
       121, 135, 152, 169, 186, 202, 218, 234,
        12, 19, 29, 46, 57, 71, 88, 100,
       120, 132, 148, 165, 182, 199, 216, 233,
        17, 23, 35, 46, 56, 77, 92, 106,
       123, 134, 152, 167, 185, 204, 222, 237,
        14, 17, 45, 53, 63, 75, 89, 107,
       115, 132, 151, 171, 188, 206, 221, 240,
         9, 16, 29, 40, 56, 71, 88, 103,
       119, 137, 154, 171, 189, 205, 222, 237,
        16, 19, 36, 48, 57, 76, 87, 105,
       118, 132, 150, 167, 185, 202, 218, 236,
        12, 17, 29, 54, 71, 81, 94, 104,
       126, 136, 149, 164, 182, 201, 221, 237,
        15, 28, 47, 62, 79, 97, 115, 129,
       142, 155, 168, 180, 194, 208, 223, 238,
         8, 14, 30, 45, 62, 78, 94, 111,
       127, 143, 159, 175, 192, 207, 223, 239,
        17, 30, 49, 62, 79, 92, 107, 119,
       132, 145, 160, 174, 190, 204, 220, 235,
        14, 19, 36, 45, 61, 76, 91, 108,
       121, 138, 154, 172, 189, 205, 222, 238,
        12, 18, 31, 45, 60, 76, 91, 107,
       123, 138, 154, 171, 187, 204, 221, 236,
        13, 17, 31, 43, 53, 70, 83, 103,
       114, 131, 149, 167, 185, 203, 220, 237,
        17, 22, 35, 42, 58, 78, 93, 110,
       125, 139, 155, 170, 188, 206, 224, 240,
         8, 15, 34, 50, 67, 83, 99, 115,
       131, 146, 162, 178, 193, 209, 224, 239,
        13, 16, 41, 66, 73, 86, 95, 111,
       128, 137, 150, 163, 183, 206, 225, 241,
        17, 25, 37, 52, 63, 75, 92, 102,
       119, 132, 144, 160, 175, 191, 212, 231,
        19, 31, 49, 65, 83, 100, 117, 133,
       147, 161, 174, 187, 200, 213, 227, 242,
        18, 31, 52, 68, 88, 103, 117, 126,
       138, 149, 163, 177, 192, 207, 223, 239,
        16, 29, 47, 61, 76, 90, 106, 119,
       133, 147, 161, 176, 193, 209, 224, 240,
        15, 21, 35, 50, 61, 73, 86, 97,
       110, 119, 129, 141, 175, 198, 218, 237
};

static const opus_int16 silk_NLSF_CB1_WB_Wght_Q9[ 512 ] = {
     3657, 2925, 2925, 2925, 2925, 2925, 2925, 2925, 2925, 2925, 2925, 2925, 2963, 2963, 2925, 2846,
     3216, 3085, 2972, 3056, 3056, 3010, 3010, 3010, 2963, 2963, 3010, 2972, 2888, 2846, 2846, 2726,
     3920, 4014, 2981, 3207, 3207, 2934, 3056, 2846, 3122, 3244, 2925, 2846, 2620, 2553, 2780, 2925,
     3516, 3197, 3010, 3103, 3019, 2888, 2925, 2925, 2925, 2925, 2888, 2888, 2888, 2888, 2888, 2753,
     5054, 5054, 2934, 3573, 3385, 3056, 3085, 2793, 3160, 3160, 2972, 2846, 2513, 2540, 2753, 2888,
     4428, 4149, 2700, 2753, 2972, 3010, 2925, 2846, 2981, 3019, 2925, 2925, 2925, 2925, 2888, 2726,
     3620, 3019, 2972, 3056, 3056, 2873, 2806, 3056, 3216, 3047, 2981, 3291, 3291, 2981, 3310, 2991,
     5227, 5014, 2540, 3338, 3526, 3385, 3197, 3094, 3376, 2981, 2700, 2647, 2687, 2793, 2846, 2673,
     5081, 5174, 4615, 4428, 2460, 2897, 3047, 3207, 3169, 2687, 2740, 2888, 2846, 2793, 2846, 2700,
     3122, 2888, 2963, 2925, 2925, 2925, 2925, 2963, 2963, 2963, 2963, 2925, 2925, 2963, 2963, 2963,
     4202, 3207, 2981, 3103, 3010, 2888, 2888, 2925, 2972, 2873, 2916, 3019, 2972, 3010, 3197, 2873,
     3760, 3760, 3244, 3103, 2981, 2888, 2925, 2888, 2972, 2934, 2793, 2793, 2846, 2888, 2888, 2660,
     3854, 4014, 3207, 3122, 3244, 2934, 3047, 2963, 2963, 3085, 2846, 2793, 2793, 2793, 2793, 2580,
     3845, 4080, 3357, 3516, 3094, 2740, 3010, 2934, 3122, 3085, 2846, 2846, 2647, 2647, 2846, 2806,
     5147, 4894, 3225, 3845, 3441, 3169, 2897, 3413, 3451, 2700, 2580, 2673, 2740, 2846, 2806, 2753,
     4109, 3789, 3291, 3160, 2925, 2888, 2888, 2925, 2793, 2740, 2793, 2740, 2793, 2846, 2888, 2806,
     5081, 5054, 3047, 3545, 3244, 3056, 3085, 2944, 3103, 2897, 2740, 2740, 2740, 2846, 2793, 2620,
     4309, 4309, 2860, 2527, 3207, 3376, 3376, 3075, 3075, 3376, 3056, 2846, 2647, 2580, 2726, 2753,
     3056, 2916, 2806, 2888, 2740, 2687, 2897, 3103, 3150, 3150, 3216, 3169, 3056, 3010, 2963, 2846,
     4375, 3882, 2925, 2888, 2846, 2888, 2846, 2846, 2888, 2888, 2888, 2846, 2888, 2925, 2888, 2846,
     2981, 2916, 2916, 2981, 2981, 3056, 3122, 3216, 3150, 3056, 3010, 2972, 2972, 2972, 2925, 2740,
     4229, 4149, 3310, 3347, 2925, 2963, 2888, 2981, 2981, 2846, 2793, 2740, 2846, 2846, 2846, 2793,
     4080, 4014, 3103, 3010, 2925, 2925, 2925, 2888, 2925, 2925, 2846, 2846, 2846, 2793, 2888, 2780,
     4615, 4575, 3169, 3441, 3207, 2981, 2897, 3038, 3122, 2740, 2687, 2687, 2687, 2740, 2793, 2700,
     4149, 4269, 3789, 3657, 2726, 2780, 2888, 2888, 3010, 2972, 2925, 2846, 2687, 2687, 2793, 2888,
     4215, 3554, 2753, 2846, 2846, 2888, 2888, 2888, 2925, 2925, 2888, 2925, 2925, 2925, 2963, 2888,
     5174, 4921, 2261, 3432, 3789, 3479, 3347, 2846, 3310, 3479, 3150, 2897, 2460, 2487, 2753, 2925,
     3451, 3685, 3122, 3197, 3357, 3047, 3207, 3207, 2981, 3216, 3085, 2925, 2925, 2687, 2540, 2434,
     2981, 3010, 2793, 2793, 2740, 2793, 2846, 2972, 3056, 3103, 3150, 3150, 3150, 3103, 3010, 3010,
     2944, 2873, 2687, 2726, 2780, 3010, 3432, 3545, 3357, 3244, 3056, 3010, 2963, 2925, 2888, 2846,
     3019, 2944, 2897, 3010, 3010, 2972, 3019, 3103, 3056, 3056, 3010, 2888, 2846, 2925, 2925, 2888,
     3920, 3967, 3010, 3197, 3357, 3216, 3291, 3291, 3479, 3704, 3441, 2726, 2181, 2460, 2580, 2607
};

static const opus_uint8 silk_NLSF_CB1_iCDF_WB[ 64 ] = {
       225, 204, 201, 184, 183, 175, 158, 154,
       153, 135, 119, 115, 113, 110, 109, 99,
        98, 95, 79, 68, 52, 50, 48, 45,
        43, 32, 31, 27, 18, 10, 3, 0,
       255, 251, 235, 230, 212, 201, 196, 182,
       167, 166, 163, 151, 138, 124, 110, 104,
        90, 78, 76, 70, 69, 57, 45, 34,
        24, 21, 11, 6, 5, 4, 3, 0
};

static const opus_uint8 silk_NLSF_CB2_SELECT_WB[ 256 ] = {
         0, 0, 0, 0, 0, 0, 0, 1,
       100, 102, 102, 68, 68, 36, 34, 96,
       164, 107, 158, 185, 180, 185, 139, 102,
        64, 66, 36, 34, 34, 0, 1, 32,
       208, 139, 141, 191, 152, 185, 155, 104,
        96, 171, 104, 166, 102, 102, 102, 132,
         1, 0, 0, 0, 0, 16, 16, 0,
        80, 109, 78, 107, 185, 139, 103, 101,
       208, 212, 141, 139, 173, 153, 123, 103,
        36, 0, 0, 0, 0, 0, 0, 1,
        48, 0, 0, 0, 0, 0, 0, 32,
        68, 135, 123, 119, 119, 103, 69, 98,
        68, 103, 120, 118, 118, 102, 71, 98,
       134, 136, 157, 184, 182, 153, 139, 134,
       208, 168, 248, 75, 189, 143, 121, 107,
        32, 49, 34, 34, 34, 0, 17, 2,
       210, 235, 139, 123, 185, 137, 105, 134,
        98, 135, 104, 182, 100, 183, 171, 134,
       100, 70, 68, 70, 66, 66, 34, 131,
        64, 166, 102, 68, 36, 2, 1, 0,
       134, 166, 102, 68, 34, 34, 66, 132,
       212, 246, 158, 139, 107, 107, 87, 102,
       100, 219, 125, 122, 137, 118, 103, 132,
       114, 135, 137, 105, 171, 106, 50, 34,
       164, 214, 141, 143, 185, 151, 121, 103,
       192, 34, 0, 0, 0, 0, 0, 1,
       208, 109, 74, 187, 134, 249, 159, 137,
       102, 110, 154, 118, 87, 101, 119, 101,
         0, 2, 0, 36, 36, 66, 68, 35,
        96, 164, 102, 100, 36, 0, 2, 33,
       167, 138, 174, 102, 100, 84, 2, 2,
       100, 107, 120, 119, 36, 197, 24, 0
};

static const opus_uint8 silk_NLSF_CB2_iCDF_WB[ 72 ] = {
       255, 254, 253, 244, 12, 3, 2, 1,
         0, 255, 254, 252, 224, 38, 3, 2,
         1, 0, 255, 254, 251, 209, 57, 4,
         2, 1, 0, 255, 254, 244, 195, 69,
         4, 2, 1, 0, 255, 251, 232, 184,
        84, 7, 2, 1, 0, 255, 254, 240,
       186, 86, 14, 2, 1, 0, 255, 254,
       239, 178, 91, 30, 5, 1, 0, 255,
       248, 227, 177, 100, 19, 2, 1, 0
};

static const opus_uint8 silk_NLSF_CB2_BITS_WB_Q5[ 72 ] = {
       255, 255, 255, 156, 4, 154, 255, 255,
       255, 255, 255, 227, 102, 15, 92, 255,
       255, 255, 255, 255, 213, 83, 24, 72,
       236, 255, 255, 255, 255, 150, 76, 33,
        63, 214, 255, 255, 255, 190, 121, 77,
        43, 55, 185, 255, 255, 255, 245, 137,
        71, 43, 59, 139, 255, 255, 255, 255,
       131, 66, 50, 66, 107, 194, 255, 255,
       166, 116, 76, 55, 53, 125, 255, 255
};

static const opus_uint8 silk_NLSF_PRED_WB_Q8[ 30 ] = {
       175, 148, 160, 176, 178, 173, 174, 164,
       177, 174, 196, 182, 198, 192, 182, 68,
        62, 66, 60, 72, 117, 85, 90, 118,
       136, 151, 142, 160, 142, 155
};

static const opus_int16 silk_NLSF_DELTA_MIN_WB_Q15[ 17 ] = {
       100, 3, 40, 3, 3, 3, 5, 14,
        14, 10, 11, 3, 8, 9, 7, 3,
       347
};

const silk_NLSF_CB_struct silk_NLSF_CB_WB =
{
    32,
    16,
    ((opus_int32)((0.15) * ((opus_int64)1 << (16)) + 0.5)),
    ((opus_int32)((1.0 / 0.15) * ((opus_int64)1 << (6)) + 0.5)),
    silk_NLSF_CB1_WB_Q8,
    silk_NLSF_CB1_WB_Wght_Q9,
    silk_NLSF_CB1_iCDF_WB,
    silk_NLSF_PRED_WB_Q8,
    silk_NLSF_CB2_SELECT_WB,
    silk_NLSF_CB2_iCDF_WB,
    silk_NLSF_CB2_BITS_WB_Q5,
    silk_NLSF_DELTA_MIN_WB_Q15,
};
const opus_int16 silk_stereo_pred_quant_Q13[ 16 ] = {
    -13732, -10050, -8266, -7526, -6500, -5000, -2950, -820,
       820, 2950, 5000, 6500, 7526, 8266, 10050, 13732
};
const opus_uint8 silk_stereo_pred_joint_iCDF[ 25 ] = {
    249, 247, 246, 245, 244,
    234, 210, 202, 201, 200,
    197, 174, 82, 59, 56,
     55, 54, 46, 22, 12,
     11, 10, 9, 7, 0
};
const opus_uint8 silk_stereo_only_code_mid_iCDF[ 2 ] = { 64, 0 };


static const opus_uint8 silk_LBRR_flags_2_iCDF[ 3 ] = { 203, 150, 0 };
static const opus_uint8 silk_LBRR_flags_3_iCDF[ 7 ] = { 215, 195, 166, 125, 110, 82, 0 };
const opus_uint8 * const silk_LBRR_flags_iCDF_ptr[ 2 ] = {
    silk_LBRR_flags_2_iCDF,
    silk_LBRR_flags_3_iCDF
};


const opus_uint8 silk_lsb_iCDF[ 2 ] = { 120, 0 };


const opus_uint8 silk_LTPscale_iCDF[ 3 ] = { 128, 64, 0 };


const opus_uint8 silk_type_offset_VAD_iCDF[ 4 ] = {
       232, 158, 10, 0
};
const opus_uint8 silk_type_offset_no_VAD_iCDF[ 2 ] = {
       230, 0
};


const opus_uint8 silk_NLSF_interpolation_factor_iCDF[ 5 ] = { 243, 221, 192, 181, 0 };


const opus_int16 silk_Quantization_Offsets_Q10[ 2 ][ 2 ] = {
    { 100, 240 }, { 32, 100 }
};


const opus_int16 silk_LTPScales_table_Q14[ 3 ] = { 15565, 12288, 8192 };


const opus_uint8 silk_uniform3_iCDF[ 3 ] = { 171, 85, 0 };
const opus_uint8 silk_uniform4_iCDF[ 4 ] = { 192, 128, 64, 0 };
const opus_uint8 silk_uniform5_iCDF[ 5 ] = { 205, 154, 102, 51, 0 };
const opus_uint8 silk_uniform6_iCDF[ 6 ] = { 213, 171, 128, 85, 43, 0 };
const opus_uint8 silk_uniform8_iCDF[ 8 ] = { 224, 192, 160, 128, 96, 64, 32, 0 };

const opus_uint8 silk_NLSF_EXT_iCDF[ 7 ] = { 100, 40, 16, 7, 3, 1, 0 };






const opus_int32 silk_Transition_LP_B_Q28[ 5 ][ 3 ] =
{
{ 250767114, 501534038, 250767114 },
{ 209867381, 419732057, 209867381 },
{ 170987846, 341967853, 170987846 },
{ 131531482, 263046905, 131531482 },
{ 89306658, 178584282, 89306658 }
};


const opus_int32 silk_Transition_LP_A_Q28[ 5 ][ 2 ] =
{
{ 506393414, 239854379 },
{ 411067935, 169683996 },
{ 306733530, 116694253 },
{ 185807084, 77959395 },
{ 35497197, 57401098 }
};
const opus_uint8 silk_pitch_lag_iCDF[ 2 * ( 18 - 2 ) ] = {
       253, 250, 244, 233, 212, 182, 150, 131,
       120, 110, 98, 85, 72, 60, 49, 40,
        32, 25, 19, 15, 13, 11, 9, 8,
         7, 6, 5, 4, 3, 2, 1, 0
};

const opus_uint8 silk_pitch_delta_iCDF[21] = {
       210, 208, 206, 203, 199, 193, 183, 168,
       142, 104, 74, 52, 37, 27, 20, 14,
        10, 6, 4, 2, 0
};

const opus_uint8 silk_pitch_contour_iCDF[34] = {
       223, 201, 183, 167, 152, 138, 124, 111,
        98, 88, 79, 70, 62, 56, 50, 44,
        39, 35, 31, 27, 24, 21, 18, 16,
        14, 12, 10, 8, 6, 4, 3, 2,
         1, 0
};

const opus_uint8 silk_pitch_contour_NB_iCDF[11] = {
       188, 176, 155, 138, 119, 97, 67, 43,
        26, 10, 0
};

const opus_uint8 silk_pitch_contour_10_ms_iCDF[12] = {
       165, 119, 80, 61, 47, 35, 27, 20,
        14, 9, 4, 0
};

const opus_uint8 silk_pitch_contour_10_ms_NB_iCDF[3] = {
       113, 63, 0
};
const opus_uint8 silk_max_pulses_table[ 4 ] = {
         8, 10, 12, 16
};

const opus_uint8 silk_pulses_per_block_iCDF[ 10 ][ 18 ] = {
{
       125, 51, 26, 18, 15, 12, 11, 10,
         9, 8, 7, 6, 5, 4, 3, 2,
         1, 0
},
{
       198, 105, 45, 22, 15, 12, 11, 10,
         9, 8, 7, 6, 5, 4, 3, 2,
         1, 0
},
{
       213, 162, 116, 83, 59, 43, 32, 24,
        18, 15, 12, 9, 7, 6, 5, 3,
         2, 0
},
{
       239, 187, 116, 59, 28, 16, 11, 10,
         9, 8, 7, 6, 5, 4, 3, 2,
         1, 0
},
{
       250, 229, 188, 135, 86, 51, 30, 19,
        13, 10, 8, 6, 5, 4, 3, 2,
         1, 0
},
{
       249, 235, 213, 185, 156, 128, 103, 83,
        66, 53, 42, 33, 26, 21, 17, 13,
        10, 0
},
{
       254, 249, 235, 206, 164, 118, 77, 46,
        27, 16, 10, 7, 5, 4, 3, 2,
         1, 0
},
{
       255, 253, 249, 239, 220, 191, 156, 119,
        85, 57, 37, 23, 15, 10, 6, 4,
         2, 0
},
{
       255, 253, 251, 246, 237, 223, 203, 179,
       152, 124, 98, 75, 55, 40, 29, 21,
        15, 0
},
{
       255, 254, 253, 247, 220, 162, 106, 67,
        42, 28, 18, 12, 9, 6, 4, 3,
         2, 0
}
};

const opus_uint8 silk_pulses_per_block_BITS_Q5[ 9 ][ 18 ] = {
{
        31, 57, 107, 160, 205, 205, 255, 255,
       255, 255, 255, 255, 255, 255, 255, 255,
       255, 255
},
{
        69, 47, 67, 111, 166, 205, 255, 255,
       255, 255, 255, 255, 255, 255, 255, 255,
       255, 255
},
{
        82, 74, 79, 95, 109, 128, 145, 160,
       173, 205, 205, 205, 224, 255, 255, 224,
       255, 224
},
{
       125, 74, 59, 69, 97, 141, 182, 255,
       255, 255, 255, 255, 255, 255, 255, 255,
       255, 255
},
{
       173, 115, 85, 73, 76, 92, 115, 145,
       173, 205, 224, 224, 255, 255, 255, 255,
       255, 255
},
{
       166, 134, 113, 102, 101, 102, 107, 118,
       125, 138, 145, 155, 166, 182, 192, 192,
       205, 150
},
{
       224, 182, 134, 101, 83, 79, 85, 97,
       120, 145, 173, 205, 224, 255, 255, 255,
       255, 255
},
{
       255, 224, 192, 150, 120, 101, 92, 89,
        93, 102, 118, 134, 160, 182, 192, 224,
       224, 224
},
{
       255, 224, 224, 182, 155, 134, 118, 109,
       104, 102, 106, 111, 118, 131, 145, 160,
       173, 131
}
};

const opus_uint8 silk_rate_levels_iCDF[ 2 ][ 9 ] =
{
{
       241, 190, 178, 132, 87, 74, 41, 14,
         0
},
{
       223, 193, 157, 140, 106, 57, 39, 18,
         0
}
};

const opus_uint8 silk_rate_levels_BITS_Q5[ 2 ][ 9 ] =
{
{
       131, 74, 141, 79, 80, 138, 95, 104,
       134
},
{
        95, 99, 91, 125, 93, 76, 123, 115,
       123
}
};

const opus_uint8 silk_shell_code_table0[ 152 ] = {
       128, 0, 214, 42, 0, 235, 128, 21,
         0, 244, 184, 72, 11, 0, 248, 214,
       128, 42, 7, 0, 248, 225, 170, 80,
        25, 5, 0, 251, 236, 198, 126, 54,
        18, 3, 0, 250, 238, 211, 159, 82,
        35, 15, 5, 0, 250, 231, 203, 168,
       128, 88, 53, 25, 6, 0, 252, 238,
       216, 185, 148, 108, 71, 40, 18, 4,
         0, 253, 243, 225, 199, 166, 128, 90,
        57, 31, 13, 3, 0, 254, 246, 233,
       212, 183, 147, 109, 73, 44, 23, 10,
         2, 0, 255, 250, 240, 223, 198, 166,
       128, 90, 58, 33, 16, 6, 1, 0,
       255, 251, 244, 231, 210, 181, 146, 110,
        75, 46, 25, 12, 5, 1, 0, 255,
       253, 248, 238, 221, 196, 164, 128, 92,
        60, 35, 18, 8, 3, 1, 0, 255,
       253, 249, 242, 229, 208, 180, 146, 110,
        76, 48, 27, 14, 7, 3, 1, 0
};

const opus_uint8 silk_shell_code_table1[ 152 ] = {
       129, 0, 207, 50, 0, 236, 129, 20,
         0, 245, 185, 72, 10, 0, 249, 213,
       129, 42, 6, 0, 250, 226, 169, 87,
        27, 4, 0, 251, 233, 194, 130, 62,
        20, 4, 0, 250, 236, 207, 160, 99,
        47, 17, 3, 0, 255, 240, 217, 182,
       131, 81, 41, 11, 1, 0, 255, 254,
       233, 201, 159, 107, 61, 20, 2, 1,
         0, 255, 249, 233, 206, 170, 128, 86,
        50, 23, 7, 1, 0, 255, 250, 238,
       217, 186, 148, 108, 70, 39, 18, 6,
         1, 0, 255, 252, 243, 226, 200, 166,
       128, 90, 56, 30, 13, 4, 1, 0,
       255, 252, 245, 231, 209, 180, 146, 110,
        76, 47, 25, 11, 4, 1, 0, 255,
       253, 248, 237, 219, 194, 163, 128, 93,
        62, 37, 19, 8, 3, 1, 0, 255,
       254, 250, 241, 226, 205, 177, 145, 111,
        79, 51, 30, 15, 6, 2, 1, 0
};

const opus_uint8 silk_shell_code_table2[ 152 ] = {
       129, 0, 203, 54, 0, 234, 129, 23,
         0, 245, 184, 73, 10, 0, 250, 215,
       129, 41, 5, 0, 252, 232, 173, 86,
        24, 3, 0, 253, 240, 200, 129, 56,
        15, 2, 0, 253, 244, 217, 164, 94,
        38, 10, 1, 0, 253, 245, 226, 189,
       132, 71, 27, 7, 1, 0, 253, 246,
       231, 203, 159, 105, 56, 23, 6, 1,
         0, 255, 248, 235, 213, 179, 133, 85,
        47, 19, 5, 1, 0, 255, 254, 243,
       221, 194, 159, 117, 70, 37, 12, 2,
         1, 0, 255, 254, 248, 234, 208, 171,
       128, 85, 48, 22, 8, 2, 1, 0,
       255, 254, 250, 240, 220, 189, 149, 107,
        67, 36, 16, 6, 2, 1, 0, 255,
       254, 251, 243, 227, 201, 166, 128, 90,
        55, 29, 13, 5, 2, 1, 0, 255,
       254, 252, 246, 234, 213, 183, 147, 109,
        73, 43, 22, 10, 4, 2, 1, 0
};

const opus_uint8 silk_shell_code_table3[ 152 ] = {
       130, 0, 200, 58, 0, 231, 130, 26,
         0, 244, 184, 76, 12, 0, 249, 214,
       130, 43, 6, 0, 252, 232, 173, 87,
        24, 3, 0, 253, 241, 203, 131, 56,
        14, 2, 0, 254, 246, 221, 167, 94,
        35, 8, 1, 0, 254, 249, 232, 193,
       130, 65, 23, 5, 1, 0, 255, 251,
       239, 211, 162, 99, 45, 15, 4, 1,
         0, 255, 251, 243, 223, 186, 131, 74,
        33, 11, 3, 1, 0, 255, 252, 245,
       230, 202, 158, 105, 57, 24, 8, 2,
         1, 0, 255, 253, 247, 235, 214, 179,
       132, 84, 44, 19, 7, 2, 1, 0,
       255, 254, 250, 240, 223, 196, 159, 112,
        69, 36, 15, 6, 2, 1, 0, 255,
       254, 253, 245, 231, 209, 176, 136, 93,
        55, 27, 11, 3, 2, 1, 0, 255,
       254, 253, 252, 239, 221, 194, 158, 117,
        76, 42, 18, 4, 3, 2, 1, 0
};

const opus_uint8 silk_shell_code_table_offsets[ 17 ] = {
         0, 0, 2, 5, 9, 14, 20, 27,
        35, 44, 54, 65, 77, 90, 104, 119,
       135
};

const opus_uint8 silk_sign_iCDF[ 42 ] = {
       254, 49, 67, 77, 82, 93, 99,
       198, 11, 18, 24, 31, 36, 45,
       255, 46, 66, 78, 87, 94, 104,
       208, 14, 21, 32, 42, 51, 66,
       255, 94, 104, 109, 112, 115, 118,
       248, 53, 69, 80, 88, 95, 102
};
static inline void combine_pulses(
    int *out,
    const int *in,
    const int len
)
{
    int k;
    for( k = 0; k < len; k++ ) {
        out[ k ] = in[ 2 * k ] + in[ 2 * k + 1 ];
    }
}

static inline void encode_split(
    ec_enc *psRangeEnc,
    const int p_child1,
    const int p,
    const opus_uint8 *shell_table
)
{
    if( p > 0 ) {
        ec_enc_icdf( psRangeEnc, p_child1, &shell_table[ silk_shell_code_table_offsets[ p ] ], 8 );
    }
}

static inline void decode_split(
    opus_int16 *p_child1,
    opus_int16 *p_child2,
    ec_dec *psRangeDec,
    const int p,
    const opus_uint8 *shell_table
)
{
    if( p > 0 ) {
        p_child1[ 0 ] = ec_dec_icdf( psRangeDec, &shell_table[ silk_shell_code_table_offsets[ p ] ], 8 );
        p_child2[ 0 ] = p - p_child1[ 0 ];
    } else {
        p_child1[ 0 ] = 0;
        p_child2[ 0 ] = 0;
    }
}


void silk_shell_encoder(
    ec_enc *psRangeEnc,
    const int *pulses0
)
{
    int pulses1[ 8 ], pulses2[ 4 ], pulses3[ 2 ], pulses4[ 1 ];


    ;


    combine_pulses( pulses1, pulses0, 8 );
    combine_pulses( pulses2, pulses1, 4 );
    combine_pulses( pulses3, pulses2, 2 );
    combine_pulses( pulses4, pulses3, 1 );

    encode_split( psRangeEnc, pulses3[ 0 ], pulses4[ 0 ], silk_shell_code_table3 );

    encode_split( psRangeEnc, pulses2[ 0 ], pulses3[ 0 ], silk_shell_code_table2 );

    encode_split( psRangeEnc, pulses1[ 0 ], pulses2[ 0 ], silk_shell_code_table1 );
    encode_split( psRangeEnc, pulses0[ 0 ], pulses1[ 0 ], silk_shell_code_table0 );
    encode_split( psRangeEnc, pulses0[ 2 ], pulses1[ 1 ], silk_shell_code_table0 );

    encode_split( psRangeEnc, pulses1[ 2 ], pulses2[ 1 ], silk_shell_code_table1 );
    encode_split( psRangeEnc, pulses0[ 4 ], pulses1[ 2 ], silk_shell_code_table0 );
    encode_split( psRangeEnc, pulses0[ 6 ], pulses1[ 3 ], silk_shell_code_table0 );

    encode_split( psRangeEnc, pulses2[ 2 ], pulses3[ 1 ], silk_shell_code_table2 );

    encode_split( psRangeEnc, pulses1[ 4 ], pulses2[ 2 ], silk_shell_code_table1 );
    encode_split( psRangeEnc, pulses0[ 8 ], pulses1[ 4 ], silk_shell_code_table0 );
    encode_split( psRangeEnc, pulses0[ 10 ], pulses1[ 5 ], silk_shell_code_table0 );

    encode_split( psRangeEnc, pulses1[ 6 ], pulses2[ 3 ], silk_shell_code_table1 );
    encode_split( psRangeEnc, pulses0[ 12 ], pulses1[ 6 ], silk_shell_code_table0 );
    encode_split( psRangeEnc, pulses0[ 14 ], pulses1[ 7 ], silk_shell_code_table0 );
}



void silk_shell_decoder(
    opus_int16 *pulses0,
    ec_dec *psRangeDec,
    const int pulses4
)
{
    opus_int16 pulses3[ 2 ], pulses2[ 4 ], pulses1[ 8 ];


    ;

    decode_split( &pulses3[ 0 ], &pulses3[ 1 ], psRangeDec, pulses4, silk_shell_code_table3 );

    decode_split( &pulses2[ 0 ], &pulses2[ 1 ], psRangeDec, pulses3[ 0 ], silk_shell_code_table2 );

    decode_split( &pulses1[ 0 ], &pulses1[ 1 ], psRangeDec, pulses2[ 0 ], silk_shell_code_table1 );
    decode_split( &pulses0[ 0 ], &pulses0[ 1 ], psRangeDec, pulses1[ 0 ], silk_shell_code_table0 );
    decode_split( &pulses0[ 2 ], &pulses0[ 3 ], psRangeDec, pulses1[ 1 ], silk_shell_code_table0 );

    decode_split( &pulses1[ 2 ], &pulses1[ 3 ], psRangeDec, pulses2[ 1 ], silk_shell_code_table1 );
    decode_split( &pulses0[ 4 ], &pulses0[ 5 ], psRangeDec, pulses1[ 2 ], silk_shell_code_table0 );
    decode_split( &pulses0[ 6 ], &pulses0[ 7 ], psRangeDec, pulses1[ 3 ], silk_shell_code_table0 );

    decode_split( &pulses2[ 2 ], &pulses2[ 3 ], psRangeDec, pulses3[ 1 ], silk_shell_code_table2 );

    decode_split( &pulses1[ 4 ], &pulses1[ 5 ], psRangeDec, pulses2[ 2 ], silk_shell_code_table1 );
    decode_split( &pulses0[ 8 ], &pulses0[ 9 ], psRangeDec, pulses1[ 4 ], silk_shell_code_table0 );
    decode_split( &pulses0[ 10 ], &pulses0[ 11 ], psRangeDec, pulses1[ 5 ], silk_shell_code_table0 );

    decode_split( &pulses1[ 6 ], &pulses1[ 7 ], psRangeDec, pulses2[ 3 ], silk_shell_code_table1 );
    decode_split( &pulses0[ 12 ], &pulses0[ 13 ], psRangeDec, pulses1[ 6 ], silk_shell_code_table0 );
    decode_split( &pulses0[ 14 ], &pulses0[ 15 ], psRangeDec, pulses1[ 7 ], silk_shell_code_table0 );
}
void silk_process_NLSFs(
    silk_encoder_state *psEncC,
    opus_int16 PredCoef_Q12[ 2 ][ 16 ],
    opus_int16 pNLSF_Q15[ 16 ],
    const opus_int16 prev_NLSFq_Q15[ 16 ]
)
{
    int i, doInterpolate;
    int NLSF_mu_Q20;
    opus_int16 i_sqr_Q15;
    opus_int16 pNLSF0_temp_Q15[ 16 ];
    opus_int16 pNLSFW_QW[ 16 ];
    opus_int16 pNLSFW0_temp_QW[ 16 ];

    ;
    ;
    ;





    NLSF_mu_Q20 = ((opus_int32)((((opus_int32)((0.003) * ((opus_int64)1 << (20)) + 0.5))) + (((((opus_int32)((-0.001) * ((opus_int64)1 << (28)) + 0.5))) * (opus_int64)((opus_int16)(psEncC->speech_activity_Q8))) >> 16)));
    if( psEncC->nb_subfr == 2 ) {

        NLSF_mu_Q20 = ((NLSF_mu_Q20) + (((NLSF_mu_Q20))>>((1))));
    }

    ;
    ;


    silk_NLSF_VQ_weights_laroia( pNLSFW_QW, pNLSF_Q15, psEncC->predictLPCOrder );


    doInterpolate = ( psEncC->useInterpolatedNLSFs == 1 ) && ( psEncC->indices.NLSFInterpCoef_Q2 < 4 );
    if( doInterpolate ) {

        silk_interpolate( pNLSF0_temp_Q15, prev_NLSFq_Q15, pNLSF_Q15,
            psEncC->indices.NLSFInterpCoef_Q2, psEncC->predictLPCOrder );


        silk_NLSF_VQ_weights_laroia( pNLSFW0_temp_QW, pNLSF0_temp_Q15, psEncC->predictLPCOrder );


        i_sqr_Q15 = ((opus_int32)((opus_uint32)(((opus_int32)((opus_int16)(psEncC->indices.NLSFInterpCoef_Q2)) * (opus_int32)((opus_int16)(psEncC->indices.NLSFInterpCoef_Q2))))<<(11)));
        for( i = 0; i < psEncC->predictLPCOrder; i++ ) {
            pNLSFW_QW[ i ] = ((((pNLSFW_QW[ i ])>>(1))) + (((((opus_int32)((opus_int16)(pNLSFW0_temp_QW[ i ])) * (opus_int32)((opus_int16)(i_sqr_Q15))))>>(16))))
                                                                       ;
            ;
        }
    }

    silk_NLSF_encode( psEncC->indices.NLSFIndices, pNLSF_Q15, psEncC->psNLSF_CB, pNLSFW_QW,
        NLSF_mu_Q20, psEncC->NLSF_MSVQ_Survivors, psEncC->indices.signalType );


    silk_NLSF2A( PredCoef_Q12[ 1 ], pNLSF_Q15, psEncC->predictLPCOrder, psEncC->arch );

    if( doInterpolate ) {

        silk_interpolate( pNLSF0_temp_Q15, prev_NLSFq_Q15, pNLSF_Q15,
            psEncC->indices.NLSFInterpCoef_Q2, psEncC->predictLPCOrder );


        silk_NLSF2A( PredCoef_Q12[ 0 ], pNLSF0_temp_Q15, psEncC->predictLPCOrder, psEncC->arch );

    } else {

        ;
        memcpy((PredCoef_Q12[ 0 ]), (PredCoef_Q12[ 1 ]), (psEncC->predictLPCOrder * sizeof( opus_int16 )));
    }
}


static inline void silk_A2NLSF_trans_poly(
    opus_int32 *p,
    const int dd
)
{
    int k, n;

    for( k = 2; k <= dd; k++ ) {
        for( n = dd; n > k; n-- ) {
            p[ n - 2 ] -= p[ n ];
        }
        p[ k - 2 ] -= ((opus_int32)((opus_uint32)(p[ k ])<<(1)));
    }
}


static inline opus_int32 silk_A2NLSF_eval_poly(
    opus_int32 *p,
    const opus_int32 x,
    const int dd
)
{
    int n;
    opus_int32 x_Q16, y32;

    y32 = p[ dd ];
    x_Q16 = ((opus_int32)((opus_uint32)(x)<<(4)));

    if ( (8 == dd) )
    {
        y32 = ((opus_int32)((p[ 7 ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
        y32 = ((opus_int32)((p[ 6 ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
        y32 = ((opus_int32)((p[ 5 ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
        y32 = ((opus_int32)((p[ 4 ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
        y32 = ((opus_int32)((p[ 3 ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
        y32 = ((opus_int32)((p[ 2 ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
        y32 = ((opus_int32)((p[ 1 ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
        y32 = ((opus_int32)((p[ 0 ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
    }
    else
    {
        for( n = dd - 1; n >= 0; n-- ) {
            y32 = ((opus_int32)((p[ n ]) + (((opus_int64)(y32) * (x_Q16)) >> 16)));
        }
    }
    return y32;
}

static inline void silk_A2NLSF_init(
     const opus_int32 *a_Q16,
     opus_int32 *P,
     opus_int32 *Q,
     const int dd
)
{
    int k;


    P[dd] = ((opus_int32)((opus_uint32)(1)<<(16)));
    Q[dd] = ((opus_int32)((opus_uint32)(1)<<(16)));
    for( k = 0; k < dd; k++ ) {
        P[ k ] = -a_Q16[ dd - k - 1 ] - a_Q16[ dd + k ];
        Q[ k ] = -a_Q16[ dd - k - 1 ] + a_Q16[ dd + k ];
    }




    for( k = dd; k > 0; k-- ) {
        P[ k - 1 ] -= P[ k ];
        Q[ k - 1 ] += Q[ k ];
    }


    silk_A2NLSF_trans_poly( P, dd );
    silk_A2NLSF_trans_poly( Q, dd );
}



void silk_A2NLSF(
    opus_int16 *NLSF,
    opus_int32 *a_Q16,
    const int d
)
{
    int i, k, m, dd, root_ix, ffrac;
    opus_int32 xlo, xhi, xmid;
    opus_int32 ylo, yhi, ymid, thr;
    opus_int32 nom, den;
    opus_int32 P[ 24 / 2 + 1 ];
    opus_int32 Q[ 24 / 2 + 1 ];
    opus_int32 *PQ[ 2 ];
    opus_int32 *p;


    PQ[ 0 ] = P;
    PQ[ 1 ] = Q;

    dd = ((d)>>(1));

    silk_A2NLSF_init( a_Q16, P, Q, dd );


    p = P;

    xlo = silk_LSFCosTab_FIX_Q12[ 0 ];
    ylo = silk_A2NLSF_eval_poly( p, xlo, dd );

    if( ylo < 0 ) {

        NLSF[ 0 ] = 0;
        p = Q;
        ylo = silk_A2NLSF_eval_poly( p, xlo, dd );
        root_ix = 1;
    } else {
        root_ix = 0;
    }
    k = 1;
    i = 0;
    thr = 0;
    while( 1 ) {

        xhi = silk_LSFCosTab_FIX_Q12[ k ];
        yhi = silk_A2NLSF_eval_poly( p, xhi, dd );


        if( ( ylo <= 0 && yhi >= thr ) || ( ylo >= 0 && yhi <= -thr ) ) {
            if( yhi == 0 ) {


                thr = 1;
            } else {
                thr = 0;
            }

            ffrac = -256;
            for( m = 0; m < 3; m++ ) {

                xmid = ((1) == 1 ? ((xlo + xhi) >> 1) + ((xlo + xhi) & 1) : (((xlo + xhi) >> ((1) - 1)) + 1) >> 1);
                ymid = silk_A2NLSF_eval_poly( p, xmid, dd );


                if( ( ylo <= 0 && ymid >= 0 ) || ( ylo >= 0 && ymid <= 0 ) ) {

                    xhi = xmid;
                    yhi = ymid;
                } else {

                    xlo = xmid;
                    ylo = ymid;
                    ffrac = ((ffrac) + (((128))>>((m))));
                }
            }


            if( (((ylo) > 0) ? (ylo) : -(ylo)) < 65536 ) {

                den = ylo - yhi;
                nom = ((opus_int32)((opus_uint32)(ylo)<<(8 - 3))) + ((den)>>(1));
                if( den != 0 ) {
                    ffrac += ((opus_int32)((nom) / (den)));
                }
            } else {

                ffrac += ((opus_int32)((ylo) / (((ylo - yhi)>>(8 - 3)))));
            }
            NLSF[ root_ix ] = (opus_int16)silk_min_32( ((opus_int32)((opus_uint32)((opus_int32)k)<<(8))) + ffrac, 0x7FFF );

            ;

            root_ix++;
            if( root_ix >= d ) {

                break;
            }

            p = PQ[ root_ix & 1 ];


            xlo = silk_LSFCosTab_FIX_Q12[ k - 1 ];
            ylo = ((opus_int32)((opus_uint32)(1 - ( root_ix & 2 ))<<(12)));
        } else {

            k++;
            xlo = xhi;
            ylo = yhi;
            thr = 0;

            if( k > 128 ) {
                i++;
                if( i > 16 ) {

                    NLSF[ 0 ] = (opus_int16)((opus_int32)((1 << 15) / (d + 1)));
                    for( k = 1; k < d; k++ ) {
                        NLSF[ k ] = (opus_int16)((NLSF[ k-1 ]) + (NLSF[ 0 ]));
                    }
                    return;
                }


                silk_bwexpander_32( a_Q16, d, 65536 - ((opus_int32)((opus_uint32)(1)<<(i))) );

                silk_A2NLSF_init( a_Q16, P, Q, dd );
                p = P;
                xlo = silk_LSFCosTab_FIX_Q12[ 0 ];
                ylo = silk_A2NLSF_eval_poly( p, xlo, dd );
                if( ylo < 0 ) {

                    NLSF[ 0 ] = 0;
                    p = Q;
                    ylo = silk_A2NLSF_eval_poly( p, xlo, dd );
                    root_ix = 1;
                } else {
                    root_ix = 0;
                }
                k = 1;
            }
        }
    }
}
void silk_interpolate(
    opus_int16 xi[ 16 ],
    const opus_int16 x0[ 16 ],
    const opus_int16 x1[ 16 ],
    const int ifact_Q2,
    const int d
)
{
    int i;

    ;
    ;

    for( i = 0; i < d; i++ ) {
        xi[ i ] = (opus_int16)((x0[ i ]) + (((((opus_int32)((opus_int16)(x1[ i ] - x0[ i ])) * (opus_int32)((opus_int16)(ifact_Q2)))))>>((2))));
    }
}
opus_int32 silk_NLSF_encode(
          opus_int8 *NLSFIndices,
          opus_int16 *pNLSF_Q15,
    const silk_NLSF_CB_struct *psNLSF_CB,
    const opus_int16 *pW_Q2,
    const int NLSF_mu_Q20,
    const int nSurvivors,
    const int signalType
)
{
    int i, s, ind1, bestIndex, prob_Q8, bits_q7;
    opus_int32 W_tmp_Q9, ret;
    ;
    ;
    ;
    ;
    opus_int16 res_Q10[ 16 ];
    opus_int16 NLSF_tmp_Q15[ 16 ];
    opus_int16 W_adj_Q5[ 16 ];
    opus_uint8 pred_Q8[ 16 ];
    opus_int16 ec_ix[ 16 ];
    const opus_uint8 *pCB_element, *iCDF_ptr;
    const opus_int16 *pCB_Wght_Q9;
    ;

    ;
    ;


    silk_NLSF_stabilize( pNLSF_Q15, psNLSF_CB->deltaMin_Q15, psNLSF_CB->order );


    opus_int32 *err_Q24 = (opus_int32*)DR_OPUS_ALLOCA(sizeof(opus_int32)*(psNLSF_CB->nVectors));
    silk_NLSF_VQ( err_Q24, pNLSF_Q15, psNLSF_CB->CB1_NLSF_Q8, psNLSF_CB->CB1_Wght_Q9, psNLSF_CB->nVectors, psNLSF_CB->order );


    int *tempIndices1 = (int*)DR_OPUS_ALLOCA(sizeof(int)*(nSurvivors));
    silk_insertion_sort_increasing( err_Q24, tempIndices1, psNLSF_CB->nVectors, nSurvivors );

    opus_int32 *RD_Q25 = (opus_int32*)DR_OPUS_ALLOCA(sizeof(opus_int32)*(nSurvivors));
    opus_int8 *tempIndices2 = (opus_int8*)DR_OPUS_ALLOCA(sizeof(opus_int8)*(nSurvivors * 16));


    for( s = 0; s < nSurvivors; s++ ) {
        ind1 = tempIndices1[ s ];


        pCB_element = &psNLSF_CB->CB1_NLSF_Q8[ ind1 * psNLSF_CB->order ];
        pCB_Wght_Q9 = &psNLSF_CB->CB1_Wght_Q9[ ind1 * psNLSF_CB->order ];
        for( i = 0; i < psNLSF_CB->order; i++ ) {
            NLSF_tmp_Q15[ i ] = ((opus_int16)((opus_uint16)((opus_int16)pCB_element[ i ])<<(7)));
            W_tmp_Q9 = pCB_Wght_Q9[ i ];
            res_Q10[ i ] = (opus_int16)((((opus_int32)((opus_int16)(pNLSF_Q15[ i ] - NLSF_tmp_Q15[ i ])) * (opus_int32)((opus_int16)(W_tmp_Q9))))>>(14));
            W_adj_Q5[ i ] = silk_DIV32_varQ( (opus_int32)pW_Q2[ i ], ((opus_int32)((opus_int16)(W_tmp_Q9)) * (opus_int32)((opus_int16)(W_tmp_Q9))), 21 );
        }


        silk_NLSF_unpack( ec_ix, pred_Q8, psNLSF_CB, ind1 );


        RD_Q25[ s ] = silk_NLSF_del_dec_quant( &tempIndices2[ s * 16 ], res_Q10, W_adj_Q5, pred_Q8, ec_ix,
            psNLSF_CB->ec_Rates_Q5, psNLSF_CB->quantStepSize_Q16, psNLSF_CB->invQuantStepSize_Q6, NLSF_mu_Q20, psNLSF_CB->order );


        iCDF_ptr = &psNLSF_CB->CB1_iCDF[ ( signalType >> 1 ) * psNLSF_CB->nVectors ];
        if( ind1 == 0 ) {
            prob_Q8 = 256 - iCDF_ptr[ ind1 ];
        } else {
            prob_Q8 = iCDF_ptr[ ind1 - 1 ] - iCDF_ptr[ ind1 ];
        }
        bits_q7 = ( 8 << 7 ) - silk_lin2log( prob_Q8 );
        RD_Q25[ s ] = ((RD_Q25[ s ]) + ((opus_int32)((opus_int16)(bits_q7))) * (opus_int32)((opus_int16)(((NLSF_mu_Q20)>>(2)))));
    }


    silk_insertion_sort_increasing( RD_Q25, &bestIndex, nSurvivors, 1 );

    NLSFIndices[ 0 ] = (opus_int8)tempIndices1[ bestIndex ];
    memcpy((&NLSFIndices[ 1 ]), (&tempIndices2[ bestIndex * 16 ]), (psNLSF_CB->order * sizeof( opus_int8 )));


    silk_NLSF_decode( pNLSF_Q15, NLSFIndices, psNLSF_CB );

    ret = RD_Q25[ 0 ];
    ;
    return ret;
}
const opus_int16 silk_LSFCosTab_FIX_Q12[ 128 + 1 ] = {
            8192, 8190, 8182, 8170,
            8152, 8130, 8104, 8072,
            8034, 7994, 7946, 7896,
            7840, 7778, 7714, 7644,
            7568, 7490, 7406, 7318,
            7226, 7128, 7026, 6922,
            6812, 6698, 6580, 6458,
            6332, 6204, 6070, 5934,
            5792, 5648, 5502, 5352,
            5198, 5040, 4880, 4718,
            4552, 4382, 4212, 4038,
            3862, 3684, 3502, 3320,
            3136, 2948, 2760, 2570,
            2378, 2186, 1990, 1794,
            1598, 1400, 1202, 1002,
             802, 602, 402, 202,
               0, -202, -402, -602,
            -802, -1002, -1202, -1400,
           -1598, -1794, -1990, -2186,
           -2378, -2570, -2760, -2948,
           -3136, -3320, -3502, -3684,
           -3862, -4038, -4212, -4382,
           -4552, -4718, -4880, -5040,
           -5198, -5352, -5502, -5648,
           -5792, -5934, -6070, -6204,
           -6332, -6458, -6580, -6698,
           -6812, -6922, -7026, -7128,
           -7226, -7318, -7406, -7490,
           -7568, -7644, -7714, -7778,
           -7840, -7896, -7946, -7994,
           -8034, -8072, -8104, -8130,
           -8152, -8170, -8182, -8190,
           -8192
};
 const opus_int16 silk_Resampler_3_4_COEFS[ 2 + 3 * 18 / 2 ] = {
    -20694, -13867,
       -49, 64, 17, -157, 353, -496, 163, 11047, 22205,
       -39, 6, 91, -170, 186, 23, -896, 6336, 19928,
       -19, -36, 102, -89, -24, 328, -951, 2568, 15909,
};

 const opus_int16 silk_Resampler_2_3_COEFS[ 2 + 2 * 18 / 2 ] = {
    -14457, -14019,
        64, 128, -122, 36, 310, -768, 584, 9267, 17733,
        12, 128, 18, -142, 288, -117, -865, 4123, 14459,
};

 const opus_int16 silk_Resampler_1_2_COEFS[ 2 + 24 / 2 ] = {
       616, -14323,
       -10, 39, 58, -46, -84, 120, 184, -315, -541, 1284, 5380, 9024,
};

 const opus_int16 silk_Resampler_1_3_COEFS[ 2 + 36 / 2 ] = {
     16102, -15162,
       -13, 0, 20, 26, 5, -31, -43, -4, 65, 90, 7, -157, -248, -44, 593, 1583, 2612, 3271,
};

 const opus_int16 silk_Resampler_1_4_COEFS[ 2 + 36 / 2 ] = {
     22500, -15099,
         3, -14, -20, -15, 2, 25, 37, 25, -16, -71, -107, -79, 50, 292, 623, 982, 1288, 1464,
};

 const opus_int16 silk_Resampler_1_6_COEFS[ 2 + 36 / 2 ] = {
     27540, -15257,
        17, 12, 8, 1, -10, -22, -30, -32, -22, 3, 44, 100, 168, 243, 317, 381, 429, 455,
};

 const opus_int16 silk_Resampler_2_3_COEFS_LQ[ 2 + 2 * 2 ] = {
     -2797, -6507,
      4697, 10739,
      1567, 8276,
};


 const opus_int16 silk_resampler_frac_FIR_12[ 12 ][ 8 / 2 ] = {
    { 189, -600, 617, 30567 },
    { 117, -159, -1070, 29704 },
    { 52, 221, -2392, 28276 },
    { -4, 529, -3350, 26341 },
    { -48, 758, -3956, 23973 },
    { -80, 905, -4235, 21254 },
    { -99, 972, -4222, 18278 },
    { -107, 967, -3957, 15143 },
    { -103, 896, -3487, 11950 },
    { -91, 773, -2865, 8798 },
    { -71, 611, -2143, 5784 },
    { -46, 425, -1375, 2996 },
};
void silk_LPC_fit(
    opus_int16 *a_QOUT,
    opus_int32 *a_QIN,
    const int QOUT,
    const int QIN,
    const int d
)
{
    int i, k, idx = 0;
    opus_int32 maxabs, absval, chirp_Q16;


    for( i = 0; i < 10; i++ ) {

        maxabs = 0;
        for( k = 0; k < d; k++ ) {
            absval = (((a_QIN[k]) > 0) ? (a_QIN[k]) : -(a_QIN[k]));
            if( absval > maxabs ) {
                maxabs = absval;
                idx = k;
            }
        }
        maxabs = ((QIN - QOUT) == 1 ? ((maxabs) >> 1) + ((maxabs) & 1) : (((maxabs) >> ((QIN - QOUT) - 1)) + 1) >> 1);

        if( maxabs > 0x7FFF ) {

            maxabs = (((maxabs) < (163838)) ? (maxabs) : (163838));
            chirp_Q16 = ((opus_int32)((0.999) * ((opus_int64)1 << (16)) + 0.5)) - ((opus_int32)((((opus_int32)((opus_uint32)(maxabs - 0x7FFF)<<(14)))) / (((((maxabs) * (idx + 1)))>>(2)))))
                                                                                        ;
            silk_bwexpander_32( a_QIN, d, chirp_Q16 );
        } else {
            break;
        }
    }

    if( i == 10 ) {

        for( k = 0; k < d; k++ ) {
            a_QOUT[ k ] = (opus_int16)((((QIN - QOUT) == 1 ? ((a_QIN[ k ]) >> 1) + ((a_QIN[ k ]) & 1) : (((a_QIN[ k ]) >> ((QIN - QOUT) - 1)) + 1) >> 1)) > 0x7FFF ? 0x7FFF : ((((QIN - QOUT) == 1 ? ((a_QIN[ k ]) >> 1) + ((a_QIN[ k ]) & 1) : (((a_QIN[ k ]) >> ((QIN - QOUT) - 1)) + 1) >> 1)) < ((opus_int16)0x8000) ? ((opus_int16)0x8000) : (((QIN - QOUT) == 1 ? ((a_QIN[ k ]) >> 1) + ((a_QIN[ k ]) & 1) : (((a_QIN[ k ]) >> ((QIN - QOUT) - 1)) + 1) >> 1))));
            a_QIN[ k ] = ((opus_int32)((opus_uint32)((opus_int32)a_QOUT[ k ])<<(QIN - QOUT)));
        }
    } else {
        for( k = 0; k < d; k++ ) {
            a_QOUT[ k ] = (opus_int16)((QIN - QOUT) == 1 ? ((a_QIN[ k ]) >> 1) + ((a_QIN[ k ]) & 1) : (((a_QIN[ k ]) >> ((QIN - QOUT) - 1)) + 1) >> 1);
        }
    }
}





typedef struct OpusEncoder OpusEncoder;






  int opus_encoder_get_size(int channels);
  OpusEncoder *opus_encoder_create(
    opus_int32 Fs,
    int channels,
    int application,
    int *error
);
 int opus_encoder_init(
    OpusEncoder *st,
    opus_int32 Fs,
    int channels,
    int application
) ;
  opus_int32 opus_encode(
    OpusEncoder *st,
    const opus_int16 *pcm,
    int frame_size,
    unsigned char *data,
    opus_int32 max_data_bytes
) ;
  opus_int32 opus_encode_float(
    OpusEncoder *st,
    const float *pcm,
    int frame_size,
    unsigned char *data,
    opus_int32 max_data_bytes
) ;




 void opus_encoder_destroy(OpusEncoder *st);
 int opus_encoder_ctl(OpusEncoder *st, int request, ...) ;
typedef struct OpusDecoder OpusDecoder;






  int opus_decoder_get_size(int channels);
  OpusDecoder *opus_decoder_create(
    opus_int32 Fs,
    int channels,
    int *error
);
 int opus_decoder_init(
    OpusDecoder *st,
    opus_int32 Fs,
    int channels
) ;
  int opus_decode(
    OpusDecoder *st,
    const unsigned char *data,
    opus_int32 len,
    opus_int16 *pcm,
    int frame_size,
    int decode_fec
) ;
  int opus_decode_float(
    OpusDecoder *st,
    const unsigned char *data,
    opus_int32 len,
    float *pcm,
    int frame_size,
    int decode_fec
) ;
 int opus_decoder_ctl(OpusDecoder *st, int request, ...) ;




 void opus_decoder_destroy(OpusDecoder *st);
 int opus_packet_parse(
   const unsigned char *data,
   opus_int32 len,
   unsigned char *out_toc,
   const unsigned char *frames[48],
   opus_int16 size[48],
   int *payload_offset
) ;
  int opus_packet_get_bandwidth(const unsigned char *data) ;
  int opus_packet_get_samples_per_frame(const unsigned char *data, opus_int32 Fs) ;






  int opus_packet_get_nb_channels(const unsigned char *data) ;
  int opus_packet_get_nb_frames(const unsigned char packet[], opus_int32 len) ;
  int opus_packet_get_nb_samples(const unsigned char packet[], opus_int32 len, opus_int32 Fs) ;
  int opus_decoder_get_nb_samples(const OpusDecoder *dec, const unsigned char packet[], opus_int32 len) ;
 void opus_pcm_soft_clip(float *pcm, int frame_size, int channels, float *softclip_mem);
typedef struct OpusRepacketizer OpusRepacketizer;




  int opus_repacketizer_get_size(void);
 OpusRepacketizer *opus_repacketizer_init(OpusRepacketizer *rp) ;




  OpusRepacketizer *opus_repacketizer_create(void);





 void opus_repacketizer_destroy(OpusRepacketizer *rp);
 int opus_repacketizer_cat(OpusRepacketizer *rp, const unsigned char *data, opus_int32 len) ;
  opus_int32 opus_repacketizer_out_range(OpusRepacketizer *rp, int begin, int end, unsigned char *data, opus_int32 maxlen) ;
  int opus_repacketizer_get_nb_frames(OpusRepacketizer *rp) ;
  opus_int32 opus_repacketizer_out(OpusRepacketizer *rp, unsigned char *data, opus_int32 maxlen) ;
 int opus_packet_pad(unsigned char *data, opus_int32 len, opus_int32 new_len);
  opus_int32 opus_packet_unpad(unsigned char *data, opus_int32 len);
 int opus_multistream_packet_pad(unsigned char *data, opus_int32 len, opus_int32 new_len, int nb_streams);
  opus_int32 opus_multistream_packet_unpad(unsigned char *data, opus_int32 len, int nb_streams);



struct OpusRepacketizer {
   unsigned char toc;
   int nb_frames;
   const unsigned char *frames[48];
   opus_int16 len[48];
   int framesize;
};

typedef struct ChannelLayout {
   int nb_channels;
   int nb_streams;
   int nb_coupled_streams;
   unsigned char mapping[256];
} ChannelLayout;

typedef enum {
  MAPPING_TYPE_NONE,
  MAPPING_TYPE_SURROUND,
  MAPPING_TYPE_AMBISONICS
} MappingType;

struct OpusMSEncoder {
   ChannelLayout layout;
   int arch;
   int lfe_stream;
   int application;
   int variable_duration;
   MappingType mapping_type;
   opus_int32 bitrate_bps;



};

struct OpusMSDecoder {
   ChannelLayout layout;

};

int opus_multistream_encoder_ctl_va_list(struct OpusMSEncoder *st, int request,
  va_list ap);
int opus_multistream_decoder_ctl_va_list(struct OpusMSDecoder *st, int request,
  va_list ap);

int validate_layout(const ChannelLayout *layout);
int get_left_channel(const ChannelLayout *layout, int stream_id, int prev);
int get_right_channel(const ChannelLayout *layout, int stream_id, int prev);
int get_mono_channel(const ChannelLayout *layout, int stream_id, int prev);

typedef void (*opus_copy_channel_in_func)(
  opus_val16 *dst,
  int dst_stride,
  const void *src,
  int src_stride,
  int src_channel,
  int frame_size,
  void *user_data
);

typedef void (*opus_copy_channel_out_func)(
  void *dst,
  int dst_stride,
  int dst_channel,
  const opus_val16 *src,
  int src_stride,
  int frame_size,
  void *user_data
);
typedef void (*downmix_func)(const void *, opus_val32 *, int, int, int, int, int);
void downmix_float(const void *_x, opus_val32 *sub, int subframe, int offset, int c1, int c2, int C);
void downmix_int(const void *_x, opus_val32 *sub, int subframe, int offset, int c1, int c2, int C);
int is_digital_silence(const opus_val16* pcm, int frame_size, int channels, int lsb_depth);

int encode_size(int size, unsigned char *data);

opus_int32 frame_size_select(opus_int32 frame_size, int variable_duration, opus_int32 Fs);

opus_int32 opus_encode_native(OpusEncoder *st, const opus_val16 *pcm, int frame_size,
      unsigned char *data, opus_int32 out_data_bytes, int lsb_depth,
      const void *analysis_pcm, opus_int32 analysis_size, int c1, int c2,
      int analysis_channels, downmix_func downmix, int float_api);

int opus_decode_native(OpusDecoder *st, const unsigned char *data, opus_int32 len,
      opus_val16 *pcm, int frame_size, int decode_fec, int self_delimited,
      opus_int32 *packet_offset, int soft_clip);


static inline int align(int i)
{
    struct foo {char c; union { void* p; opus_int32 i; opus_val32 v; } u;};

    unsigned int alignment = 
                            offsetof (
                            struct foo
                            , 
                            u
                            )
                                                   ;



    return ((i + alignment - 1) / alignment) * alignment;
}

int opus_packet_parse_impl(const unsigned char *data, opus_int32 len,
      int self_delimited, unsigned char *out_toc,
      const unsigned char *frames[48], opus_int16 size[48],
      int *payload_offset, opus_int32 *packet_offset);

opus_int32 opus_repacketizer_out_range_impl(OpusRepacketizer *rp, int begin, int end,
      unsigned char *data, opus_int32 maxlen, int self_delimited, int pad);

int pad_frame(unsigned char *data, opus_int32 len, opus_int32 new_len);

int opus_multistream_encode_native
(
  struct OpusMSEncoder *st,
  opus_copy_channel_in_func copy_channel_in,
  const void *pcm,
  int analysis_frame_size,
  unsigned char *data,
  opus_int32 max_data_bytes,
  int lsb_depth,
  downmix_func downmix,
  int float_api,
  void *user_data
);

int opus_multistream_decode_native(
  struct OpusMSDecoder *st,
  const unsigned char *data,
  opus_int32 len,
  void *pcm,
  opus_copy_channel_out_func copy_channel_out,
  int frame_size,
  int decode_fec,
  int soft_clip,
  void *user_data
);



struct OpusDecoder {
   int celt_dec_offset;
   int silk_dec_offset;
   int channels;
   opus_int32 Fs;
   silk_DecControlStruct DecControl;
   int decode_gain;
   int arch;



   int stream_channels;

   int bandwidth;
   int mode;
   int prev_mode;
   int frame_size;
   int prev_redundancy;
   int last_packet_duration;

   opus_val16 softclip_mem[2];


   opus_uint32 rangeFinal;
};
int opus_decoder_get_size(int channels)
{
   int silkDecSizeBytes, celtDecSizeBytes;
   int ret;
   if (channels<1 || channels > 2)
      return 0;
   ret = silk_Get_Decoder_Size( &silkDecSizeBytes );
   if(ret)
      return 0;
   silkDecSizeBytes = align(silkDecSizeBytes);
   celtDecSizeBytes = celt_decoder_get_size(channels);
   return align(sizeof(OpusDecoder))+silkDecSizeBytes+celtDecSizeBytes;
}

int opus_decoder_init(OpusDecoder *st, opus_int32 Fs, int channels)
{
   void *silk_dec;
   OpusCustomDecoder *celt_dec;
   int ret, silkDecSizeBytes;

   if ((Fs!=48000&&Fs!=24000&&Fs!=16000&&Fs!=12000&&Fs!=8000)
    || (channels!=1&&channels!=2))
      return -1;

   (memset(((char*)st), 0, (opus_decoder_get_size(channels))*sizeof(*((char*)st))));

   ret = silk_Get_Decoder_Size(&silkDecSizeBytes);
   if (ret)
      return -3;

   silkDecSizeBytes = align(silkDecSizeBytes);
   st->silk_dec_offset = align(sizeof(OpusDecoder));
   st->celt_dec_offset = st->silk_dec_offset+silkDecSizeBytes;
   silk_dec = (char*)st+st->silk_dec_offset;
   celt_dec = (OpusCustomDecoder*)((char*)st+st->celt_dec_offset);
   st->stream_channels = st->channels = channels;

   st->Fs = Fs;
   st->DecControl.API_sampleRate = st->Fs;
   st->DecControl.nChannelsAPI = st->channels;


   ret = silk_InitDecoder( silk_dec );
   if(ret)return -3;


   ret = celt_decoder_init(celt_dec, Fs, channels);
   if(ret!=0)return -3;

   opus_custom_decoder_ctl(celt_dec, 10016, (((void)((0) == (opus_int32)0)), (opus_int32)(0)));

   st->prev_mode = 0;
   st->frame_size = Fs/400;
   st->arch = opus_select_arch();
   return 0;
}

OpusDecoder *opus_decoder_create(opus_int32 Fs, int channels, int *error)
{
   int ret;
   OpusDecoder *st;
   if ((Fs!=48000&&Fs!=24000&&Fs!=16000&&Fs!=12000&&Fs!=8000)
    || (channels!=1&&channels!=2))
   {
      if (error)
         *error = -1;
      return 
            ((void *)0)
                ;
   }
   st = (OpusDecoder *)opus_alloc(opus_decoder_get_size(channels));
   if (st == 
            ((void *)0)
                )
   {
      if (error)
         *error = -7;
      return 
            ((void *)0)
                ;
   }
   ret = opus_decoder_init(st, Fs, channels);
   if (error)
      *error = ret;
   if (ret != 0)
   {
      opus_free(st);
      st = 
          ((void *)0)
              ;
   }
   return st;
}

static void smooth_fade(const opus_val16 *in1, const opus_val16 *in2,
      opus_val16 *out, int overlap, int channels,
      const opus_val16 *window, opus_int32 Fs)
{
   int i, c;
   int inc = 48000/Fs;
   for (c=0;c<channels;c++)
   {
      for (i=0;i<overlap;i++)
      {
         opus_val16 w = ((window[i*inc])*(window[i*inc]));
         out[i*channels+c] = (((((opus_val32)(w)*(opus_val32)(in2[i*channels+c])))+(opus_val32)(1.0f -w)*(opus_val32)(in1[i*channels+c])))
                                                                    ;
      }
   }
}

static int opus_packet_get_mode(const unsigned char *data)
{
   int mode;
   if (data[0]&0x80)
   {
      mode = 1002;
   } else if ((data[0]&0x60) == 0x60)
   {
      mode = 1001;
   } else {
      mode = 1000;
   }
   return mode;
}

static int opus_decode_frame(OpusDecoder *st, const unsigned char *data,
      opus_int32 len, opus_val16 *pcm, int frame_size, int decode_fec)
{
   void *silk_dec;
   OpusCustomDecoder *celt_dec;
   int i, silk_ret=0, celt_ret=0;
   ec_dec dec;
   opus_int32 silk_frame_size;
   int pcm_silk_size;
   ;
   int pcm_transition_silk_size;
   ;
   int pcm_transition_celt_size;
   ;
   opus_val16 *pcm_transition=
                             ((void *)0)
                                 ;
   int redundant_audio_size;
   ;

   int audiosize;
   int mode;
   int bandwidth;
   int transition=0;
   int start_band;
   int redundancy=0;
   int redundancy_bytes = 0;
   int celt_to_silk=0;
   int c;
   int F2_5, F5, F10, F20;
   const opus_val16 *window;
   opus_uint32 redundant_rng = 0;
   int celt_accum;
   ;

   silk_dec = (char*)st+st->silk_dec_offset;
   celt_dec = (OpusCustomDecoder*)((char*)st+st->celt_dec_offset);
   F20 = st->Fs/50;
   F10 = F20>>1;
   F5 = F10>>1;
   F2_5 = F5>>1;
   if (frame_size < F2_5)
   {
      ;
      return -2;
   }

   frame_size = ((frame_size) < (st->Fs/25*3) ? (frame_size) : (st->Fs/25*3));

   if (len<=1)
   {
      data = 
            ((void *)0)
                ;

      frame_size = ((frame_size) < (st->frame_size) ? (frame_size) : (st->frame_size));
   }
   if (data != 
              ((void *)0)
                  )
   {
      audiosize = st->frame_size;
      mode = st->mode;
      bandwidth = st->bandwidth;
      ec_dec_init(&dec,(unsigned char*)data,len);
   } else {
      audiosize = frame_size;

      mode = st->prev_redundancy ? 1002 : st->prev_mode;
      bandwidth = 0;

      if (mode == 0)
      {

         for (i=0;i<audiosize*st->channels;i++)
            pcm[i] = 0;
         ;
         return audiosize;
      }



      if (audiosize > F20)
      {
         do {
            int ret = opus_decode_frame(st, 
                                           ((void *)0)
                                               , 0, pcm, ((audiosize) < (F20) ? (audiosize) : (F20)), 0);
            if (ret<0)
            {
               ;
               return ret;
            }
            pcm += ret*st->channels;
            audiosize -= ret;
         } while (audiosize > 0);
         ;
         return frame_size;
      } else if (audiosize < F20)
      {
         if (audiosize > F10)
            audiosize = F10;
         else if (mode != 1000 && audiosize > F5 && audiosize < F10)
            audiosize = F5;
      }
   }






   celt_accum = 0;


   pcm_transition_silk_size = 1;
   pcm_transition_celt_size = 1;
   if (data!=
            ((void *)0) 
                 && st->prev_mode > 0 && (
       (mode == 1002 && st->prev_mode != 1002 && !st->prev_redundancy)
    || (mode != 1002 && st->prev_mode == 1002) )
      )
   {
      transition = 1;

      if (mode == 1002)
         pcm_transition_celt_size = F5*st->channels;
      else
         pcm_transition_silk_size = F5*st->channels;
   }
   opus_val16 *pcm_transition_celt = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(pcm_transition_celt_size));
   if (transition && mode == 1002)
   {
      pcm_transition = pcm_transition_celt;
      opus_decode_frame(st, 
                           ((void *)0)
                               , 0, pcm_transition, ((F5) < (audiosize) ? (F5) : (audiosize)), 0);
   }
   if (audiosize > frame_size)
   {

      ;
      return -1;
   } else {
      frame_size = audiosize;
   }


   pcm_silk_size = (mode != 1002 && !celt_accum) ? ((F10) > (frame_size) ? (F10) : (frame_size))*st->channels : 1;
   opus_int16 *pcm_silk = (opus_int16*)DR_OPUS_ALLOCA(sizeof(opus_int16)*(pcm_silk_size));


   if (mode != 1002)
   {
      int lost_flag, decoded_samples;
      opus_int16 *pcm_ptr;





         pcm_ptr = pcm_silk;

      if (st->prev_mode==1002)
         silk_InitDecoder( silk_dec );


      st->DecControl.payloadSize_ms = ((10) > (1000 * audiosize / st->Fs) ? (10) : (1000 * audiosize / st->Fs));

      if (data != 
                 ((void *)0)
                     )
      {
        st->DecControl.nChannelsInternal = st->stream_channels;
        if( mode == 1000 ) {
           if( bandwidth == 1101 ) {
              st->DecControl.internalSampleRate = 8000;
           } else if( bandwidth == 1102 ) {
              st->DecControl.internalSampleRate = 12000;
           } else if( bandwidth == 1103 ) {
              st->DecControl.internalSampleRate = 16000;
           } else {
              st->DecControl.internalSampleRate = 16000;
              ;
           }
        } else {

           st->DecControl.internalSampleRate = 16000;
        }
     }

     lost_flag = data == 
                        ((void *)0) 
                             ? 1 : 2 * decode_fec;
     decoded_samples = 0;
     do {

        int first_frame = decoded_samples == 0;
        silk_ret = silk_Decode( silk_dec, &st->DecControl,
                                lost_flag, first_frame, &dec, pcm_ptr, &silk_frame_size, st->arch );
        if( silk_ret ) {
           if (lost_flag) {

              silk_frame_size = frame_size;
              for (i=0;i<frame_size*st->channels;i++)
                 pcm_ptr[i] = 0;
           } else {
             ;
             return -3;
           }
        }
        pcm_ptr += silk_frame_size * st->channels;
        decoded_samples += silk_frame_size;
      } while( decoded_samples < frame_size );
   }

   start_band = 0;
   if (!decode_fec && mode != 1002 && data != 
                                                       ((void *)0)
    
   && ec_tell(&dec)+17+20*(mode == 1001) <= 8*len)
   {

      if (mode == 1001)
         redundancy = ec_dec_bit_logp(&dec, 12);
      else
         redundancy = 1;
      if (redundancy)
      {
         celt_to_silk = ec_dec_bit_logp(&dec, 1);


         redundancy_bytes = mode==1001 ?
               (opus_int32)ec_dec_uint(&dec, 256)+2 :
               len-((ec_tell(&dec)+7)>>3);
         len -= redundancy_bytes;


         if (len*8 < ec_tell(&dec))
         {
            len = 0;
            redundancy_bytes = 0;
            redundancy = 0;
         }

         dec.storage -= redundancy_bytes;
      }
   }
   if (mode != 1002)
      start_band = 17;

   if (redundancy)
   {
      transition = 0;
      pcm_transition_silk_size=1;
   }

   opus_val16 *pcm_transition_silk = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(pcm_transition_silk_size));

   if (transition && mode != 1002)
   {
      pcm_transition = pcm_transition_silk;
      opus_decode_frame(st, 
                           ((void *)0)
                               , 0, pcm_transition, ((F5) < (audiosize) ? (F5) : (audiosize)), 0);
   }


   if (bandwidth)
   {
      int endband=21;

      switch(bandwidth)
      {
      case 1101:
         endband = 13;
         break;
      case 1102:
      case 1103:
         endband = 17;
         break;
      case 1104:
         endband = 19;
         break;
      case 1105:
         endband = 21;
         break;
      default:
         ;
         break;
      }
      do {if((opus_custom_decoder_ctl(celt_dec, 10012, (((void)((endband) == (opus_int32)0)), (opus_int32)(endband)))) != 0) {; return -3;} } while (0);
   }
   do {if((opus_custom_decoder_ctl(celt_dec, 10008, (((void)((st->stream_channels) == (opus_int32)0)), (opus_int32)(st->stream_channels)))) != 0) {; return -3;} } while (0);


   redundant_audio_size = redundancy ? F5*st->channels : 1;
   opus_val16 *redundant_audio = (opus_val16*)DR_OPUS_ALLOCA(sizeof(opus_val16)*(redundant_audio_size));


   if (redundancy && celt_to_silk)
   {





      do {if((opus_custom_decoder_ctl(celt_dec, 10010, (((void)((0) == (opus_int32)0)), (opus_int32)(0)))) != 0) {; return -3;} } while (0);
      celt_decode_with_ec(celt_dec, data+len, redundancy_bytes,
                          redundant_audio, F5, 
                                              ((void *)0)
                                                  , 0);
      do {if((opus_custom_decoder_ctl(celt_dec, 4031, ((&redundant_rng) + ((&redundant_rng) - (opus_uint32*)(&redundant_rng))))) != 0) {; return -3;} } while (0);
   }


   do {if((opus_custom_decoder_ctl(celt_dec, 10010, (((void)((start_band) == (opus_int32)0)), (opus_int32)(start_band)))) != 0) {; return -3;} } while (0);

   if (mode != 1000)
   {
      int celt_frame_size = ((F20) < (frame_size) ? (F20) : (frame_size));

      if (mode != st->prev_mode && st->prev_mode > 0 && !st->prev_redundancy)
         do {if((opus_custom_decoder_ctl(celt_dec, 4028)) != 0) {; return -3;} } while (0);

      celt_ret = celt_decode_with_ec(celt_dec, decode_fec ? 
                                                           ((void *)0) 
                                                                : data,
                                     len, pcm, celt_frame_size, &dec, celt_accum);
   } else {
      unsigned char silence[2] = {0xFF, 0xFF};
      if (!celt_accum)
      {
         for (i=0;i<frame_size*st->channels;i++)
            pcm[i] = 0;
      }


      if (st->prev_mode == 1001 && !(redundancy && celt_to_silk && st->prev_redundancy) )
      {
         do {if((opus_custom_decoder_ctl(celt_dec, 10010, (((void)((0) == (opus_int32)0)), (opus_int32)(0)))) != 0) {; return -3;} } while (0);
         celt_decode_with_ec(celt_dec, silence, 2, pcm, F2_5, 
                                                             ((void *)0)
                                                                 , celt_accum);
      }
   }

   if (mode != 1002 && !celt_accum)
   {




      for (i=0;i<frame_size*st->channels;i++)
         pcm[i] = pcm[i] + (opus_val16)((1.f/32768.f)*pcm_silk[i]);

   }

   {
      const OpusCustomMode *celt_mode;
      do {if((opus_custom_decoder_ctl(celt_dec, 10015, ((&celt_mode) + ((&celt_mode) - (const OpusCustomMode**)(&celt_mode))))) != 0) {; return -3;} } while (0);
      window = celt_mode->window;
   }


   if (redundancy && !celt_to_silk)
   {
      do {if((opus_custom_decoder_ctl(celt_dec, 4028)) != 0) {; return -3;} } while (0);
      do {if((opus_custom_decoder_ctl(celt_dec, 10010, (((void)((0) == (opus_int32)0)), (opus_int32)(0)))) != 0) {; return -3;} } while (0);

      celt_decode_with_ec(celt_dec, data+len, redundancy_bytes, redundant_audio, F5, 
                                                                                    ((void *)0)
                                                                                        , 0);
      do {if((opus_custom_decoder_ctl(celt_dec, 4031, ((&redundant_rng) + ((&redundant_rng) - (opus_uint32*)(&redundant_rng))))) != 0) {; return -3;} } while (0);
      smooth_fade(pcm+st->channels*(frame_size-F2_5), redundant_audio+st->channels*F2_5,
                  pcm+st->channels*(frame_size-F2_5), F2_5, st->channels, window, st->Fs);
   }



   if (redundancy && celt_to_silk && (st->prev_mode != 1000 || st->prev_redundancy))
   {
      for (c=0;c<st->channels;c++)
      {
         for (i=0;i<F2_5;i++)
            pcm[st->channels*i+c] = redundant_audio[st->channels*i+c];
      }
      smooth_fade(redundant_audio+st->channels*F2_5, pcm+st->channels*F2_5,
                  pcm+st->channels*F2_5, F2_5, st->channels, window, st->Fs);
   }
   if (transition)
   {
      if (audiosize >= F5)
      {
         for (i=0;i<st->channels*F2_5;i++)
            pcm[i] = pcm_transition[i];
         smooth_fade(pcm_transition+st->channels*F2_5, pcm+st->channels*F2_5,
                     pcm+st->channels*F2_5, F2_5,
                     st->channels, window, st->Fs);
      } else {





         smooth_fade(pcm_transition, pcm,
                     pcm, F2_5,
                     st->channels, window, st->Fs);
      }
   }

   if(st->decode_gain)
   {
      opus_val32 gain;
      gain = ((float)exp(0.6931471805599453094*((((6.48814081e-4f))*(st->decode_gain)))));
      for (i=0;i<frame_size*st->channels;i++)
      {
         opus_val32 x;
         x = ((pcm[i])*(gain));
         pcm[i] = (x);
      }
   }

   if (len <= 1)
      st->rangeFinal = 0;
   else
      st->rangeFinal = dec.rng ^ redundant_rng;

   st->prev_mode = mode;
   st->prev_redundancy = redundancy && !celt_to_silk;

   if (celt_ret>=0)
   {
      if (_opus_false())
         do{}while(0);
   }

   ;
   return celt_ret < 0 ? celt_ret : audiosize;

}

int opus_decode_native(OpusDecoder *st, const unsigned char *data,
      opus_int32 len, opus_val16 *pcm, int frame_size, int decode_fec,
      int self_delimited, opus_int32 *packet_offset, int soft_clip)
{
   int i, nb_samples;
   int count, offset;
   unsigned char toc;
   int packet_frame_size, packet_bandwidth, packet_mode, packet_stream_channels;

   opus_int16 size[48];
   ;
   if (decode_fec<0 || decode_fec>1)
      return -1;

   if ((decode_fec || len==0 || data==
                                     ((void *)0)
                                         ) && frame_size%(st->Fs/400)!=0)
      return -1;
   if (len==0 || data==
                      ((void *)0)
                          )
   {
      int pcm_count=0;
      do {
         int ret;
         ret = opus_decode_frame(st, 
                                    ((void *)0)
                                        , 0, pcm+pcm_count*st->channels, frame_size-pcm_count, 0);
         if (ret<0)
            return ret;
         pcm_count += ret;
      } while (pcm_count < frame_size);
      ;
      if (_opus_false())
         do{}while(0);
      st->last_packet_duration = pcm_count;
      return pcm_count;
   } else if (len<0)
      return -1;

   packet_mode = opus_packet_get_mode(data);
   packet_bandwidth = opus_packet_get_bandwidth(data);
   packet_frame_size = opus_packet_get_samples_per_frame(data, st->Fs);
   packet_stream_channels = opus_packet_get_nb_channels(data);

   count = opus_packet_parse_impl(data, len, self_delimited, &toc, 
                                                                  ((void *)0)
                                                                      ,
                                  size, &offset, packet_offset);
   if (count<0)
      return count;

   data += offset;

   if (decode_fec)
   {
      int duration_copy;
      int ret;

      if (frame_size < packet_frame_size || packet_mode == 1002 || st->mode == 1002)
         return opus_decode_native(st, 
                                      ((void *)0)
                                          , 0, pcm, frame_size, 0, 0, 
                                                                      ((void *)0)
                                                                          , soft_clip);

      duration_copy = st->last_packet_duration;
      if (frame_size-packet_frame_size!=0)
      {
         ret = opus_decode_native(st, 
                                     ((void *)0)
                                         , 0, pcm, frame_size-packet_frame_size, 0, 0, 
                                                                                       ((void *)0)
                                                                                           , soft_clip);
         if (ret<0)
         {
            st->last_packet_duration = duration_copy;
            return ret;
         }
         ;
      }

      st->mode = packet_mode;
      st->bandwidth = packet_bandwidth;
      st->frame_size = packet_frame_size;
      st->stream_channels = packet_stream_channels;
      ret = opus_decode_frame(st, data, size[0], pcm+st->channels*(frame_size-packet_frame_size),
            packet_frame_size, 1);
      if (ret<0)
         return ret;
      else {
         if (_opus_false())
            do{}while(0);
         st->last_packet_duration = frame_size;
         return frame_size;
      }
   }

   if (count*packet_frame_size > frame_size)
      return -2;


   st->mode = packet_mode;
   st->bandwidth = packet_bandwidth;
   st->frame_size = packet_frame_size;
   st->stream_channels = packet_stream_channels;

   nb_samples=0;
   for (i=0;i<count;i++)
   {
      int ret;
      ret = opus_decode_frame(st, data, size[i], pcm+nb_samples*st->channels, frame_size-nb_samples, 0);
      if (ret<0)
         return ret;
      ;
      data += size[i];
      nb_samples += ret;
   }
   st->last_packet_duration = nb_samples;
   if (_opus_false())
      do{}while(0);

   if (soft_clip)
      opus_pcm_soft_clip(pcm, nb_samples, st->channels, st->softclip_mem);
   else
      st->softclip_mem[0]=st->softclip_mem[1]=0;

   return nb_samples;
}
int opus_decode(OpusDecoder *st, const unsigned char *data,
      opus_int32 len, opus_int16 *pcm, int frame_size, int decode_fec)
{
   ;
   int ret, i;
   int nb_samples;
   ;

   if(frame_size<=0)
   {
      ;
      return -1;
   }

   if (data != 
              ((void *)0) 
                   && len > 0 && !decode_fec)
   {
      nb_samples = opus_decoder_get_nb_samples(st, data, len);
      if (nb_samples>0)
         frame_size = ((frame_size) < (nb_samples) ? (frame_size) : (nb_samples));
      else
         return -4;
   }
   ;
   float *out = (float*)DR_OPUS_ALLOCA(sizeof(float)*(frame_size*st->channels));

   ret = opus_decode_native(st, data, len, out, frame_size, decode_fec, 0, 
                                                                          ((void *)0)
                                                                              , 1);
   if (ret > 0)
   {
      for (i=0;i<ret*st->channels;i++)
         pcm[i] = FLOAT2INT16(out[i]);
   }
   ;
   return ret;
}

int opus_decode_float(OpusDecoder *st, const unsigned char *data,
      opus_int32 len, opus_val16 *pcm, int frame_size, int decode_fec)
{
   if(frame_size<=0)
      return -1;
   return opus_decode_native(st, data, len, pcm, frame_size, decode_fec, 0, 
                                                                           ((void *)0)
                                                                               , 0);
}



int opus_decoder_ctl(OpusDecoder *st, int request, ...)
{
   int ret = 0;
   va_list ap;
   void *silk_dec;
   OpusCustomDecoder *celt_dec;

   silk_dec = (char*)st+st->silk_dec_offset;
   celt_dec = (OpusCustomDecoder*)((char*)st+st->celt_dec_offset);


   
  va_start(
  ap
  ,
  request
  )
                       ;

   switch (request)
   {
   case 4009:
   {
      opus_int32 *value = 
                         va_arg(
                         ap
                         ,
                         opus_int32*
                         )
                                                ;
      if (!value)
      {
         goto bad_arg;
      }
      *value = st->bandwidth;
   }
   break;
   case 4031:
   {
      opus_uint32 *value = 
                          va_arg(
                          ap
                          ,
                          opus_uint32*
                          )
                                                  ;
      if (!value)
      {
         goto bad_arg;
      }
      *value = st->rangeFinal;
   }
   break;
   case 4028:
   {
      (memset(((char*)&st->stream_channels), 0, (sizeof(OpusDecoder)- ((char*)&st->stream_channels - (char*)st))*sizeof(*((char*)&st->stream_channels))))

                                                               ;

      opus_custom_decoder_ctl(celt_dec, 4028);
      silk_InitDecoder( silk_dec );
      st->stream_channels = st->channels;
      st->frame_size = st->Fs/400;
   }
   break;
   case 4029:
   {
      opus_int32 *value = 
                         va_arg(
                         ap
                         ,
                         opus_int32*
                         )
                                                ;
      if (!value)
      {
         goto bad_arg;
      }
      *value = st->Fs;
   }
   break;
   case 4033:
   {
      opus_int32 *value = 
                         va_arg(
                         ap
                         ,
                         opus_int32*
                         )
                                                ;
      if (!value)
      {
         goto bad_arg;
      }
      if (st->prev_mode == 1002)
         ret = opus_custom_decoder_ctl(celt_dec, 4033, ((value) + ((value) - (opus_int32*)(value))));
      else
         *value = st->DecControl.prevPitchLag;
   }
   break;
   case 4045:
   {
      opus_int32 *value = 
                         va_arg(
                         ap
                         ,
                         opus_int32*
                         )
                                                ;
      if (!value)
      {
         goto bad_arg;
      }
      *value = st->decode_gain;
   }
   break;
   case 4034:
   {
       opus_int32 value = 
                         va_arg(
                         ap
                         ,
                         opus_int32
                         )
                                               ;
       if (value<-32768 || value>32767)
       {
          goto bad_arg;
       }
       st->decode_gain = value;
   }
   break;
   case 4039:
   {
      opus_int32 *value = 
                         va_arg(
                         ap
                         ,
                         opus_int32*
                         )
                                                ;
      if (!value)
      {
         goto bad_arg;
      }
      *value = st->last_packet_duration;
   }
   break;
   case 4046:
   {
       opus_int32 value = 
                         va_arg(
                         ap
                         ,
                         opus_int32
                         )
                                               ;
       if(value<0 || value>1)
       {
          goto bad_arg;
       }
       ret = opus_custom_decoder_ctl(celt_dec, 4046, (((void)((value) == (opus_int32)0)), (opus_int32)(value)));
   }
   break;
   case 4047:
   {
       opus_int32 *value = 
                          va_arg(
                          ap
                          ,
                          opus_int32*
                          )
                                                 ;
       if (!value)
       {
          goto bad_arg;
       }
       ret = opus_custom_decoder_ctl(celt_dec, 4047, ((value) + ((value) - (opus_int32*)(value))));
   }
   break;
   default:

      ret = -5;
      break;
   }

   
  va_end(
  ap
  )
            ;
   return ret;
bad_arg:
   
  va_end(
  ap
  )
            ;
   return -1;
}

void opus_decoder_destroy(OpusDecoder *st)
{
   opus_free(st);
}


int opus_packet_get_bandwidth(const unsigned char *data)
{
   int bandwidth;
   if (data[0]&0x80)
   {
      bandwidth = 1102 + ((data[0]>>5)&0x3);
      if (bandwidth == 1102)
         bandwidth = 1101;
   } else if ((data[0]&0x60) == 0x60)
   {
      bandwidth = (data[0]&0x10) ? 1105 :
                                   1104;
   } else {
      bandwidth = 1101 + ((data[0]>>5)&0x3);
   }
   return bandwidth;
}

int opus_packet_get_nb_channels(const unsigned char *data)
{
   return (data[0]&0x4) ? 2 : 1;
}

int opus_packet_get_nb_frames(const unsigned char packet[], opus_int32 len)
{
   int count;
   if (len<1)
      return -1;
   count = packet[0]&0x3;
   if (count==0)
      return 1;
   else if (count!=3)
      return 2;
   else if (len<2)
      return -4;
   else
      return packet[1]&0x3F;
}

int opus_packet_get_nb_samples(const unsigned char packet[], opus_int32 len,
      opus_int32 Fs)
{
   int samples;
   int count = opus_packet_get_nb_frames(packet, len);

   if (count<0)
      return count;

   samples = count*opus_packet_get_samples_per_frame(packet, Fs);

   if (samples*25 > Fs*3)
      return -4;
   else
      return samples;
}

int opus_decoder_get_nb_samples(const OpusDecoder *dec,
      const unsigned char packet[], opus_int32 len)
{
   return opus_packet_get_nb_samples(packet, len, dec->Fs);
}
 void opus_pcm_soft_clip(float *_x, int N, int C, float *declip_mem)
{
   int c;
   int i;
   float *x;

   if (C<1 || N<1 || !_x || !declip_mem) return;





   for (i=0;i<N*C;i++)
      _x[i] = ((-2.f) > (((2.f) < (_x[i]) ? (2.f) : (_x[i]))) ? (-2.f) : (((2.f) < (_x[i]) ? (2.f) : (_x[i]))));
   for (c=0;c<C;c++)
   {
      float a;
      float x0;
      int curr;

      x = _x+c;
      a = declip_mem[c];


      for (i=0;i<N;i++)
      {
         if (x[i*C]*a>=0)
            break;
         x[i*C] = x[i*C]+a*x[i*C]*x[i*C];
      }

      curr=0;
      x0 = x[0];
      while(1)
      {
         int start, end;
         float maxval;
         int special=0;
         int peak_pos;
         for (i=curr;i<N;i++)
         {
            if (x[i*C]>1 || x[i*C]<-1)
               break;
         }
         if (i==N)
         {
            a=0;
            break;
         }
         peak_pos = i;
         start=end=i;
         maxval=((float)fabs(x[i*C]));

         while (start>0 && x[i*C]*x[(start-1)*C]>=0)
            start--;

         while (end<N && x[i*C]*x[end*C]>=0)
         {

            if (((float)fabs(x[end*C]))>maxval)
            {
               maxval = ((float)fabs(x[end*C]));
               peak_pos = end;
            }
            end++;
         }

         special = (start==0 && x[i*C]*x[0]>=0);


         a=(maxval-1)/(maxval*maxval);



         a += a*2.4e-7f;
         if (x[i*C]>0)
            a = -a;

         for (i=start;i<end;i++)
            x[i*C] = x[i*C]+a*x[i*C]*x[i*C];

         if (special && peak_pos>=2)
         {


            float delta;
            float offset = x0-x[0];
            delta = offset / peak_pos;
            for (i=curr;i<peak_pos;i++)
            {
               offset -= delta;
               x[i*C] += offset;
               x[i*C] = ((-1.f) > (((1.f) < (x[i*C]) ? (1.f) : (x[i*C]))) ? (-1.f) : (((1.f) < (x[i*C]) ? (1.f) : (x[i*C]))));
            }
         }
         curr = end;
         if (curr==N)
            break;
      }
      declip_mem[c] = a;
   }
}


int encode_size(int size, unsigned char *data)
{
   if (size < 252)
   {
      data[0] = size;
      return 1;
   } else {
      data[0] = 252+(size&0x3);
      data[1] = (size-(int)data[0])>>2;
      return 2;
   }
}

static int parse_size(const unsigned char *data, opus_int32 len, opus_int16 *size)
{
   if (len<1)
   {
      *size = -1;
      return -1;
   } else if (data[0]<252)
   {
      *size = data[0];
      return 1;
   } else if (len<2)
   {
      *size = -1;
      return -1;
   } else {
      *size = 4*data[1] + data[0];
      return 2;
   }
}

int opus_packet_get_samples_per_frame(const unsigned char *data,
      opus_int32 Fs)
{
   int audiosize;
   if (data[0]&0x80)
   {
      audiosize = ((data[0]>>3)&0x3);
      audiosize = (Fs<<audiosize)/400;
   } else if ((data[0]&0x60) == 0x60)
   {
      audiosize = (data[0]&0x08) ? Fs/50 : Fs/100;
   } else {
      audiosize = ((data[0]>>3)&0x3);
      if (audiosize == 3)
         audiosize = Fs*60/1000;
      else
         audiosize = (Fs<<audiosize)/100;
   }
   return audiosize;
}

int opus_packet_parse_impl(const unsigned char *data, opus_int32 len,
      int self_delimited, unsigned char *out_toc,
      const unsigned char *frames[48], opus_int16 size[48],
      int *payload_offset, opus_int32 *packet_offset)
{
   int i, bytes;
   int count;
   int cbr;
   unsigned char ch, toc;
   int framesize;
   opus_int32 last_size;
   opus_int32 pad = 0;
   const unsigned char *data0 = data;

   if (size==
            ((void *)0) 
                 || len<0)
      return -1;
   if (len==0)
      return -4;

   framesize = opus_packet_get_samples_per_frame(data, 48000);

   cbr = 0;
   toc = *data++;
   len--;
   last_size = len;
   switch (toc&0x3)
   {

   case 0:
      count=1;
      break;

   case 1:
      count=2;
      cbr = 1;
      if (!self_delimited)
      {
         if (len&0x1)
            return -4;
         last_size = len/2;

         size[0] = (opus_int16)last_size;
      }
      break;

   case 2:
      count = 2;
      bytes = parse_size(data, len, size);
      len -= bytes;
      if (size[0]<0 || size[0] > len)
         return -4;
      data += bytes;
      last_size = len-size[0];
      break;

   default:
      if (len<1)
         return -4;

      ch = *data++;
      count = ch&0x3F;
      if (count <= 0 || framesize*(opus_int32)count > 5760)
         return -4;
      len--;

      if (ch&0x40)
      {
         int p;
         do {
            int tmp;
            if (len<=0)
               return -4;
            p = *data++;
            len--;
            tmp = p==255 ? 254: p;
            len -= tmp;
            pad += tmp;
         } while (p==255);
      }
      if (len<0)
         return -4;

      cbr = !(ch&0x80);
      if (!cbr)
      {

         last_size = len;
         for (i=0;i<count-1;i++)
         {
            bytes = parse_size(data, len, size+i);
            len -= bytes;
            if (size[i]<0 || size[i] > len)
               return -4;
            data += bytes;
            last_size -= bytes+size[i];
         }
         if (last_size<0)
            return -4;
      } else if (!self_delimited)
      {

         last_size = len/count;
         if (last_size*count!=len)
            return -4;
         for (i=0;i<count-1;i++)
            size[i] = (opus_int16)last_size;
      }
      break;
   }

   if (self_delimited)
   {
      bytes = parse_size(data, len, size+count-1);
      len -= bytes;
      if (size[count-1]<0 || size[count-1] > len)
         return -4;
      data += bytes;

      if (cbr)
      {
         if (size[count-1]*count > len)
            return -4;
         for (i=0;i<count-1;i++)
            size[i] = size[count-1];
      } else if (bytes+size[count-1] > last_size)
         return -4;
   } else
   {



      if (last_size > 1275)
         return -4;
      size[count-1] = (opus_int16)last_size;
   }

   if (payload_offset)
      *payload_offset = (int)(data-data0);

   for (i=0;i<count;i++)
   {
      if (frames)
         frames[i] = data;
      data += size[i];
   }

   if (packet_offset)
      *packet_offset = pad+(opus_int32)(data-data0);

   if (out_toc)
      *out_toc = toc;

   return count;
}

int opus_packet_parse(const unsigned char *data, opus_int32 len,
      unsigned char *out_toc, const unsigned char *frames[48],
      opus_int16 size[48], int *payload_offset)
{
   return opus_packet_parse_impl(data, len, 0, out_toc,
                                 frames, size, payload_offset, 
                                                              ((void *)0)
                                                                  );
}
int opus_repacketizer_get_size(void)
{
   return sizeof(OpusRepacketizer);
}

OpusRepacketizer *opus_repacketizer_init(OpusRepacketizer *rp)
{
   rp->nb_frames = 0;
   return rp;
}

OpusRepacketizer *opus_repacketizer_create(void)
{
   OpusRepacketizer *rp;
   rp=(OpusRepacketizer *)opus_alloc(opus_repacketizer_get_size());
   if(rp==
         ((void *)0)
             )return 
                     ((void *)0)
                         ;
   return opus_repacketizer_init(rp);
}

void opus_repacketizer_destroy(OpusRepacketizer *rp)
{
   opus_free(rp);
}

static int opus_repacketizer_cat_impl(OpusRepacketizer *rp, const unsigned char *data, opus_int32 len, int self_delimited)
{
   unsigned char tmp_toc;
   int curr_nb_frames,ret;

   if (len<1) return -4;
   if (rp->nb_frames == 0)
   {
      rp->toc = data[0];
      rp->framesize = opus_packet_get_samples_per_frame(data, 8000);
   } else if ((rp->toc&0xFC) != (data[0]&0xFC))
   {

      return -4;
   }
   curr_nb_frames = opus_packet_get_nb_frames(data, len);
   if(curr_nb_frames<1) return -4;


   if ((curr_nb_frames+rp->nb_frames)*rp->framesize > 960)
   {
      return -4;
   }

   ret=opus_packet_parse_impl(data, len, self_delimited, &tmp_toc, &rp->frames[rp->nb_frames], &rp->len[rp->nb_frames], 
                                                                                                                       ((void *)0)
                                                                                                                           , 
                                                                                                                             ((void *)0)
                                                                                                                                 );
   if(ret<1)return ret;

   rp->nb_frames += curr_nb_frames;
   return 0;
}

int opus_repacketizer_cat(OpusRepacketizer *rp, const unsigned char *data, opus_int32 len)
{
   return opus_repacketizer_cat_impl(rp, data, len, 0);
}

int opus_repacketizer_get_nb_frames(OpusRepacketizer *rp)
{
   return rp->nb_frames;
}

opus_int32 opus_repacketizer_out_range_impl(OpusRepacketizer *rp, int begin, int end,
      unsigned char *data, opus_int32 maxlen, int self_delimited, int pad)
{
   int i, count;
   opus_int32 tot_size;
   opus_int16 *len;
   const unsigned char **frames;
   unsigned char * ptr;

   if (begin<0 || begin>=end || end>rp->nb_frames)
   {

      return -1;
   }
   count = end-begin;

   len = rp->len+begin;
   frames = rp->frames+begin;
   if (self_delimited)
      tot_size = 1 + (len[count-1]>=252);
   else
      tot_size = 0;

   ptr = data;
   if (count==1)
   {

      tot_size += len[0]+1;
      if (tot_size > maxlen)
         return -2;
      *ptr++ = rp->toc&0xFC;
   } else if (count==2)
   {
      if (len[1] == len[0])
      {

         tot_size += 2*len[0]+1;
         if (tot_size > maxlen)
            return -2;
         *ptr++ = (rp->toc&0xFC) | 0x1;
      } else {

         tot_size += len[0]+len[1]+2+(len[0]>=252);
         if (tot_size > maxlen)
            return -2;
         *ptr++ = (rp->toc&0xFC) | 0x2;
         ptr += encode_size(len[0], ptr);
      }
   }
   if (count > 2 || (pad && tot_size < maxlen))
   {

      int vbr;
      int pad_amount=0;


      ptr = data;
      if (self_delimited)
         tot_size = 1 + (len[count-1]>=252);
      else
         tot_size = 0;
      vbr = 0;
      for (i=1;i<count;i++)
      {
         if (len[i] != len[0])
         {
            vbr=1;
            break;
         }
      }
      if (vbr)
      {
         tot_size += 2;
         for (i=0;i<count-1;i++)
            tot_size += 1 + (len[i]>=252) + len[i];
         tot_size += len[count-1];

         if (tot_size > maxlen)
            return -2;
         *ptr++ = (rp->toc&0xFC) | 0x3;
         *ptr++ = count | 0x80;
      } else {
         tot_size += count*len[0]+2;
         if (tot_size > maxlen)
            return -2;
         *ptr++ = (rp->toc&0xFC) | 0x3;
         *ptr++ = count;
      }
      pad_amount = pad ? (maxlen-tot_size) : 0;
      if (pad_amount != 0)
      {
         int nb_255s;
         data[1] |= 0x40;
         nb_255s = (pad_amount-1)/255;
         for (i=0;i<nb_255s;i++)
            *ptr++ = 255;
         *ptr++ = pad_amount-255*nb_255s-1;
         tot_size += pad_amount;
      }
      if (vbr)
      {
         for (i=0;i<count-1;i++)
            ptr += encode_size(len[i], ptr);
      }
   }
   if (self_delimited) {
      int sdlen = encode_size(len[count-1], ptr);
      ptr += sdlen;
   }

   for (i=0;i<count;i++)
   {




      (memmove((ptr), (frames[i]), (len[i])*sizeof(*(ptr)) + 0*((ptr)-(frames[i])) ));
      ptr += len[i];
   }
   if (pad)
   {

      while (ptr<data+maxlen)
         *ptr++=0;
   }
   return tot_size;
}

opus_int32 opus_repacketizer_out_range(OpusRepacketizer *rp, int begin, int end, unsigned char *data, opus_int32 maxlen)
{
   return opus_repacketizer_out_range_impl(rp, begin, end, data, maxlen, 0, 0);
}

opus_int32 opus_repacketizer_out(OpusRepacketizer *rp, unsigned char *data, opus_int32 maxlen)
{
   return opus_repacketizer_out_range_impl(rp, 0, rp->nb_frames, data, maxlen, 0, 0);
}

int opus_packet_pad(unsigned char *data, opus_int32 len, opus_int32 new_len)
{
   OpusRepacketizer rp;
   opus_int32 ret;
   if (len < 1)
      return -1;
   if (len==new_len)
      return 0;
   else if (len > new_len)
      return -1;
   opus_repacketizer_init(&rp);

   (memmove((data+new_len-len), (data), (len)*sizeof(*(data+new_len-len)) + 0*((data+new_len-len)-(data)) ));
   ret = opus_repacketizer_cat(&rp, data+new_len-len, len);
   if (ret != 0)
      return ret;
   ret = opus_repacketizer_out_range_impl(&rp, 0, rp.nb_frames, data, new_len, 0, 1);
   if (ret > 0)
      return 0;
   else
      return ret;
}

opus_int32 opus_packet_unpad(unsigned char *data, opus_int32 len)
{
   OpusRepacketizer rp;
   opus_int32 ret;
   if (len < 1)
      return -1;
   opus_repacketizer_init(&rp);
   ret = opus_repacketizer_cat(&rp, data, len);
   if (ret < 0)
      return ret;
   ret = opus_repacketizer_out_range_impl(&rp, 0, rp.nb_frames, data, len, 0, 0);
   ;
   return ret;
}

int opus_multistream_packet_pad(unsigned char *data, opus_int32 len, opus_int32 new_len, int nb_streams)
{
   int s;
   int count;
   unsigned char toc;
   opus_int16 size[48];
   opus_int32 packet_offset;
   opus_int32 amount;

   if (len < 1)
      return -1;
   if (len==new_len)
      return 0;
   else if (len > new_len)
      return -1;
   amount = new_len - len;

   for (s=0;s<nb_streams-1;s++)
   {
      if (len<=0)
         return -4;
      count = opus_packet_parse_impl(data, len, 1, &toc, 
                                                        ((void *)0)
                                                            ,
                                     size, 
                                          ((void *)0)
                                              , &packet_offset);
      if (count<0)
         return count;
      data += packet_offset;
      len -= packet_offset;
   }
   return opus_packet_pad(data, len, len+amount);
}

opus_int32 opus_multistream_packet_unpad(unsigned char *data, opus_int32 len, int nb_streams)
{
   int s;
   unsigned char toc;
   opus_int16 size[48];
   opus_int32 packet_offset;
   OpusRepacketizer rp;
   unsigned char *dst;
   opus_int32 dst_len;

   if (len < 1)
      return -1;
   dst = data;
   dst_len = 0;

   for (s=0;s<nb_streams;s++)
   {
      opus_int32 ret;
      int self_delimited = s!=nb_streams-1;
      if (len<=0)
         return -4;
      opus_repacketizer_init(&rp);
      ret = opus_packet_parse_impl(data, len, self_delimited, &toc, 
                                                                   ((void *)0)
                                                                       ,
                                     size, 
                                          ((void *)0)
                                              , &packet_offset);
      if (ret<0)
         return ret;
      ret = opus_repacketizer_cat_impl(&rp, data, packet_offset, self_delimited);
      if (ret < 0)
         return ret;
      ret = opus_repacketizer_out_range_impl(&rp, 0, rp.nb_frames, dst, len, self_delimited, 0);
      if (ret < 0)
         return ret;
      else
         dst_len += ret;
      dst += ret;
      data += packet_offset;
      len -= packet_offset;
   }
   return dst_len;
}
void silk_NLSF_VQ(
    opus_int32 err_Q24[],
    const opus_int16 in_Q15[],
    const opus_uint8 pCB_Q8[],
    const opus_int16 pWght_Q9[],
    const int K,
    const int LPC_order
)
{
    int i, m;
    opus_int32 diff_Q15, diffw_Q24, sum_error_Q24, pred_Q24;
    const opus_int16 *w_Q9_ptr;
    const opus_uint8 *cb_Q8_ptr;

    ;


    cb_Q8_ptr = pCB_Q8;
    w_Q9_ptr = pWght_Q9;
    for( i = 0; i < K; i++ ) {
        sum_error_Q24 = 0;
        pred_Q24 = 0;
        for( m = LPC_order-2; m >= 0; m -= 2 ) {

            diff_Q15 = (((in_Q15[ m + 1 ])) - (((opus_int32)((opus_uint32)(((opus_int32)cb_Q8_ptr[ m + 1 ]))<<((7))))));
            diffw_Q24 = ((opus_int32)((opus_int16)(diff_Q15)) * (opus_int32)((opus_int16)(w_Q9_ptr[ m + 1 ])));
            sum_error_Q24 = ((sum_error_Q24) + (((((((diffw_Q24)) - ((((pred_Q24))>>((1)))))) > 0) ? ((((diffw_Q24)) - ((((pred_Q24))>>((1)))))) : -((((diffw_Q24)) - ((((pred_Q24))>>((1)))))))));
            pred_Q24 = diffw_Q24;


            diff_Q15 = (((in_Q15[ m ])) - (((opus_int32)((opus_uint32)(((opus_int32)cb_Q8_ptr[ m ]))<<((7))))));
            diffw_Q24 = ((opus_int32)((opus_int16)(diff_Q15)) * (opus_int32)((opus_int16)(w_Q9_ptr[ m ])));
            sum_error_Q24 = ((sum_error_Q24) + (((((((diffw_Q24)) - ((((pred_Q24))>>((1)))))) > 0) ? ((((diffw_Q24)) - ((((pred_Q24))>>((1)))))) : -((((diffw_Q24)) - ((((pred_Q24))>>((1)))))))));
            pred_Q24 = diffw_Q24;

            ;
        }
        err_Q24[ i ] = sum_error_Q24;
        cb_Q8_ptr += LPC_order;
        w_Q9_ptr += LPC_order;
    }
}
opus_int32 silk_NLSF_del_dec_quant(
    opus_int8 indices[],
    const opus_int16 x_Q10[],
    const opus_int16 w_Q5[],
    const opus_uint8 pred_coef_Q8[],
    const opus_int16 ec_ix[],
    const opus_uint8 ec_rates_Q5[],
    const int quant_step_size_Q16,
    const opus_int16 inv_quant_step_size_Q6,
    const opus_int32 mu_Q20,
    const opus_int16 order
)
{
    int i, j, nStates, ind_tmp, ind_min_max, ind_max_min, in_Q10, res_Q10;
    int pred_Q10, diff_Q10, rate0_Q5, rate1_Q5;
    opus_int16 out0_Q10, out1_Q10;
    opus_int32 RD_tmp_Q25, min_Q25, min_max_Q25, max_min_Q25;
    int ind_sort[ ( 1 << 2 ) ];
    opus_int8 ind[ ( 1 << 2 ) ][ 16 ];
    opus_int16 prev_out_Q10[ 2 * ( 1 << 2 ) ];
    opus_int32 RD_Q25[ 2 * ( 1 << 2 ) ];
    opus_int32 RD_min_Q25[ ( 1 << 2 ) ];
    opus_int32 RD_max_Q25[ ( 1 << 2 ) ];
    const opus_uint8 *rates_Q5;

    int out0_Q10_table[2 * 10];
    int out1_Q10_table[2 * 10];

    for (i = -10; i <= 10 -1; i++)
    {
        out0_Q10 = ((opus_int32)((opus_uint32)(i)<<(10)));
        out1_Q10 = ((out0_Q10) + (1024));
        if( i > 0 ) {
            out0_Q10 = ((out0_Q10) - (((opus_int32)((0.1) * ((opus_int64)1 << (10)) + 0.5))));
            out1_Q10 = ((out1_Q10) - (((opus_int32)((0.1) * ((opus_int64)1 << (10)) + 0.5))));
        } else if( i == 0 ) {
            out1_Q10 = ((out1_Q10) - (((opus_int32)((0.1) * ((opus_int64)1 << (10)) + 0.5))));
        } else if( i == -1 ) {
            out0_Q10 = ((out0_Q10) + (((opus_int32)((0.1) * ((opus_int64)1 << (10)) + 0.5))));
        } else {
            out0_Q10 = ((out0_Q10) + (((opus_int32)((0.1) * ((opus_int64)1 << (10)) + 0.5))));
            out1_Q10 = ((out1_Q10) + (((opus_int32)((0.1) * ((opus_int64)1 << (10)) + 0.5))));
        }
        out0_Q10_table[ i + 10 ] = ((((opus_int32)((opus_int16)(out0_Q10)) * (opus_int32)((opus_int16)(quant_step_size_Q16))))>>(16));
        out1_Q10_table[ i + 10 ] = ((((opus_int32)((opus_int16)(out1_Q10)) * (opus_int32)((opus_int16)(quant_step_size_Q16))))>>(16));
    }

    ;

    nStates = 1;
    RD_Q25[ 0 ] = 0;
    prev_out_Q10[ 0 ] = 0;
    for( i = order - 1; i >= 0; i-- ) {
        rates_Q5 = &ec_rates_Q5[ ec_ix[ i ] ];
        in_Q10 = x_Q10[ i ];
        for( j = 0; j < nStates; j++ ) {
            pred_Q10 = ((((opus_int32)((opus_int16)((opus_int16)pred_coef_Q8[ i ])) * (opus_int32)((opus_int16)(prev_out_Q10[ j ]))))>>(8));
            res_Q10 = ((in_Q10) - (pred_Q10));
            ind_tmp = ((((opus_int32)((opus_int16)(inv_quant_step_size_Q6)) * (opus_int32)((opus_int16)(res_Q10))))>>(16));
            ind_tmp = ((-10) > (10 -1) ? ((ind_tmp) > (-10) ? (-10) : ((ind_tmp) < (10 -1) ? (10 -1) : (ind_tmp))) : ((ind_tmp) > (10 -1) ? (10 -1) : ((ind_tmp) < (-10) ? (-10) : (ind_tmp))));
            ind[ j ][ i ] = (opus_int8)ind_tmp;


            out0_Q10 = out0_Q10_table[ ind_tmp + 10 ];
            out1_Q10 = out1_Q10_table[ ind_tmp + 10 ];

            out0_Q10 = ((out0_Q10) + (pred_Q10));
            out1_Q10 = ((out1_Q10) + (pred_Q10));
            prev_out_Q10[ j ] = out0_Q10;
            prev_out_Q10[ j + nStates ] = out1_Q10;


            if( ind_tmp + 1 >= 4 ) {
                if( ind_tmp + 1 == 4 ) {
                    rate0_Q5 = rates_Q5[ ind_tmp + 4 ];
                    rate1_Q5 = 280;
                } else {
                    rate0_Q5 = ((280 - 43 * 4) + ((opus_int32)((opus_int16)(43))) * (opus_int32)((opus_int16)(ind_tmp)));
                    rate1_Q5 = ((rate0_Q5) + (43));
                }
            } else if( ind_tmp <= -4 ) {
                if( ind_tmp == -4 ) {
                    rate0_Q5 = 280;
                    rate1_Q5 = rates_Q5[ ind_tmp + 1 + 4 ];
                } else {
                    rate0_Q5 = ((280 - 43 * 4) + ((opus_int32)((opus_int16)(-43))) * (opus_int32)((opus_int16)(ind_tmp)));
                    rate1_Q5 = ((rate0_Q5) - (43));
                }
            } else {
                rate0_Q5 = rates_Q5[ ind_tmp + 4 ];
                rate1_Q5 = rates_Q5[ ind_tmp + 1 + 4 ];
            }
            RD_tmp_Q25 = RD_Q25[ j ];
            diff_Q10 = ((in_Q10) - (out0_Q10));
            RD_Q25[ j ] = (((((RD_tmp_Q25)) + (((((opus_int32)((opus_int16)(diff_Q10)) * (opus_int32)((opus_int16)(diff_Q10)))) * (w_Q5[ i ]))))) + ((opus_int32)((opus_int16)(mu_Q20))) * (opus_int32)((opus_int16)(rate0_Q5)));
            diff_Q10 = ((in_Q10) - (out1_Q10));
            RD_Q25[ j + nStates ] = (((((RD_tmp_Q25)) + (((((opus_int32)((opus_int16)(diff_Q10)) * (opus_int32)((opus_int16)(diff_Q10)))) * (w_Q5[ i ]))))) + ((opus_int32)((opus_int16)(mu_Q20))) * (opus_int32)((opus_int16)(rate1_Q5)));
        }

        if( nStates <= ( 1 << 2 )/2 ) {

            for( j = 0; j < nStates; j++ ) {
                ind[ j + nStates ][ i ] = ind[ j ][ i ] + 1;
            }
            nStates = ((opus_int32)((opus_uint32)(nStates)<<(1)));
            for( j = nStates; j < ( 1 << 2 ); j++ ) {
                ind[ j ][ i ] = ind[ j - nStates ][ i ];
            }
        } else {

            for( j = 0; j < ( 1 << 2 ); j++ ) {
                if( RD_Q25[ j ] > RD_Q25[ j + ( 1 << 2 ) ] ) {
                    RD_max_Q25[ j ] = RD_Q25[ j ];
                    RD_min_Q25[ j ] = RD_Q25[ j + ( 1 << 2 ) ];
                    RD_Q25[ j ] = RD_min_Q25[ j ];
                    RD_Q25[ j + ( 1 << 2 ) ] = RD_max_Q25[ j ];

                    out0_Q10 = prev_out_Q10[ j ];
                    prev_out_Q10[ j ] = prev_out_Q10[ j + ( 1 << 2 ) ];
                    prev_out_Q10[ j + ( 1 << 2 ) ] = out0_Q10;
                    ind_sort[ j ] = j + ( 1 << 2 );
                } else {
                    RD_min_Q25[ j ] = RD_Q25[ j ];
                    RD_max_Q25[ j ] = RD_Q25[ j + ( 1 << 2 ) ];
                    ind_sort[ j ] = j;
                }
            }


            while( 1 ) {
                min_max_Q25 = 0x7FFFFFFF;
                max_min_Q25 = 0;
                ind_min_max = 0;
                ind_max_min = 0;
                for( j = 0; j < ( 1 << 2 ); j++ ) {
                    if( min_max_Q25 > RD_max_Q25[ j ] ) {
                        min_max_Q25 = RD_max_Q25[ j ];
                        ind_min_max = j;
                    }
                    if( max_min_Q25 < RD_min_Q25[ j ] ) {
                        max_min_Q25 = RD_min_Q25[ j ];
                        ind_max_min = j;
                    }
                }
                if( min_max_Q25 >= max_min_Q25 ) {
                    break;
                }

                ind_sort[ ind_max_min ] = ind_sort[ ind_min_max ] ^ ( 1 << 2 );
                RD_Q25[ ind_max_min ] = RD_Q25[ ind_min_max + ( 1 << 2 ) ];
                prev_out_Q10[ ind_max_min ] = prev_out_Q10[ ind_min_max + ( 1 << 2 ) ];
                RD_min_Q25[ ind_max_min ] = 0;
                RD_max_Q25[ ind_min_max ] = 0x7FFFFFFF;
                memcpy((ind[ ind_max_min ]), (ind[ ind_min_max ]), (16 * sizeof( opus_int8 )));
            }

            for( j = 0; j < ( 1 << 2 ); j++ ) {
                ind[ j ][ i ] += ((ind_sort[ j ])>>(2));
            }
        }
    }


    ind_tmp = 0;
    min_Q25 = 0x7FFFFFFF;
    for( j = 0; j < 2 * ( 1 << 2 ); j++ ) {
        if( min_Q25 > RD_Q25[ j ] ) {
            min_Q25 = RD_Q25[ j ];
            ind_tmp = j;
        }
    }
    for( j = 0; j < order; j++ ) {
        indices[ j ] = ind[ ind_tmp & ( ( 1 << 2 ) - 1 ) ][ j ];
        ;
        ;
    }
    indices[ 0 ] += ((ind_tmp)>>(2));
    ;
    ;
    return min_Q25;
}

/*==============================================================================
 * OGG CONTAINER PARSING
 *============================================================================*/

static drlibs_uint32 dr_opus__crc32_table[256];
static drlibs_bool32 dr_opus__crc32_initialized = DRLIBS_FALSE;

static void dr_opus__init_crc32(void) {
    if (dr_opus__crc32_initialized) return;
    for (drlibs_uint32 i = 0; i < 256; i++) {
        drlibs_uint32 c = i << 24;
        for (int j = 0; j < 8; j++) {
            c = (c << 1) ^ ((c & 0x80000000) ? 0x04C11DB7 : 0);
        }
        dr_opus__crc32_table[i] = c;
    }
    dr_opus__crc32_initialized = DRLIBS_TRUE;
}

static drlibs_uint32 dr_opus__crc32(const drlibs_uint8* data, size_t len) {
    drlibs_uint32 crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc = (crc << 8) ^ dr_opus__crc32_table[((crc >> 24) ^ data[i]) & 0xFF];
    }
    return crc;
}

/* File I/O */
static size_t dr_opus__read(dr_opus* pOpus, void* pBuffer, size_t bytesToRead) {
    if (pOpus->pFile) {
        return fread(pBuffer, 1, bytesToRead, (FILE*)pOpus->pFile);
    } else if (pOpus->pMemory) {
        size_t remaining = pOpus->memorySize - pOpus->memoryPos;
        if (bytesToRead > remaining) bytesToRead = remaining;
        memcpy(pBuffer, pOpus->pMemory + pOpus->memoryPos, bytesToRead);
        pOpus->memoryPos += bytesToRead;
        return bytesToRead;
    }
    return 0;
}

/* Read Ogg page */
static drlibs_bool32 dr_opus__read_ogg_page(dr_opus* pOpus) {
    drlibs_uint8 header[27];
    
    /* Find sync */
    for (;;) {
        if (dr_opus__read(pOpus, header, 4) != 4) return DRLIBS_FALSE;
        if (header[0] == 'O' && header[1] == 'g' && header[2] == 'g' && header[3] == 'S') break;
        /* Resync - shift and try again */
        memmove(header, header + 1, 3);
        if (dr_opus__read(pOpus, header + 3, 1) != 1) return DRLIBS_FALSE;
    }
    
    if (dr_opus__read(pOpus, header + 4, 23) != 23) return DRLIBS_FALSE;
    
    pOpus->oggGranulePosition = (drlibs_int64)header[6] | ((drlibs_int64)header[7] << 8) |
        ((drlibs_int64)header[8] << 16) | ((drlibs_int64)header[9] << 24) |
        ((drlibs_int64)header[10] << 32) | ((drlibs_int64)header[11] << 40) |
        ((drlibs_int64)header[12] << 48) | ((drlibs_int64)header[13] << 56);
    
    pOpus->oggSerial = header[14] | (header[15] << 8) | (header[16] << 16) | (header[17] << 24);
    pOpus->oggSegmentCount = header[26];
    
    if (dr_opus__read(pOpus, pOpus->oggSegmentTable, pOpus->oggSegmentCount) != pOpus->oggSegmentCount)
        return DRLIBS_FALSE;
    
    pOpus->oggPageDataSize = 0;
    for (int i = 0; i < pOpus->oggSegmentCount; i++) {
        pOpus->oggPageDataSize += pOpus->oggSegmentTable[i];
    }
    
    if (dr_opus__read(pOpus, pOpus->oggPageBuffer, pOpus->oggPageDataSize) != pOpus->oggPageDataSize)
        return DRLIBS_FALSE;
    
    pOpus->oggPageDataPos = 0;
    pOpus->oggCurrentSegment = 0;
    return DRLIBS_TRUE;
}

/* Read next Opus packet from Ogg stream */
static drlibs_int32 dr_opus__read_packet(dr_opus* pOpus, drlibs_uint8* pBuffer, drlibs_uint32 bufferSize) {
    drlibs_uint32 packetSize = 0;
    
    for (;;) {
        if (pOpus->oggCurrentSegment >= pOpus->oggSegmentCount) {
            if (!dr_opus__read_ogg_page(pOpus)) return -1;
        }
        
        drlibs_uint8 segSize = pOpus->oggSegmentTable[pOpus->oggCurrentSegment++];
        
        if (packetSize + segSize > bufferSize) return -1;
        memcpy(pBuffer + packetSize, pOpus->oggPageBuffer + pOpus->oggPageDataPos, segSize);
        pOpus->oggPageDataPos += segSize;
        packetSize += segSize;
        
        if (segSize < 255) break; /* End of packet */
    }
    
    return (drlibs_int32)packetSize;
}

/* Parse OpusHead */
static drlibs_bool32 dr_opus__read_opus_head(dr_opus* pOpus) {
    drlibs_uint8 packet[276];
    drlibs_int32 size = dr_opus__read_packet(pOpus, packet, sizeof(packet));
    
    if (size < 19) return DRLIBS_FALSE;
    if (memcmp(packet, "OpusHead", 8) != 0) return DRLIBS_FALSE;
    if (packet[8] != 1) return DRLIBS_FALSE; /* Version */
    
    pOpus->channels = packet[9];
    pOpus->preSkip = packet[10] | (packet[11] << 8);
    pOpus->inputSampleRate = packet[12] | (packet[13] << 8) | (packet[14] << 16) | (packet[15] << 24);
    pOpus->outputGain = (drlibs_int16)(packet[16] | (packet[17] << 8));
    pOpus->sampleRate = 48000; /* Opus always decodes to 48kHz */
    
    return DRLIBS_TRUE;
}

/* Skip OpusTags */
static drlibs_bool32 dr_opus__read_opus_tags(dr_opus* pOpus) {
    drlibs_uint8 packet[65536];
    drlibs_int32 size = dr_opus__read_packet(pOpus, packet, sizeof(packet));
    if (size < 8) return DRLIBS_FALSE;
    if (memcmp(packet, "OpusTags", 8) != 0) return DRLIBS_FALSE;
    return DRLIBS_TRUE;
}

/*==============================================================================
 * HIGH-LEVEL API
 *============================================================================*/

dr_opus_result dr_opus_init_file(const char* pFilePath, const dr_opus_allocation_callbacks* pAlloc, dr_opus* pOpus) {
    if (!pOpus || !pFilePath) return DR_OPUS_INVALID_ARGS;
    
    memset(pOpus, 0, sizeof(*pOpus));
    if (pAlloc) pOpus->alloc = *pAlloc;
    
    dr_opus__init_crc32();
    
    pOpus->pFile = fopen(pFilePath, "rb");
    if (!pOpus->pFile) return DR_OPUS_INVALID_FILE;
    
    /* Read Ogg/Opus headers */
    if (!dr_opus__read_ogg_page(pOpus)) { fclose(pOpus->pFile); return DR_OPUS_INVALID_FILE; }
    if (!dr_opus__read_opus_head(pOpus)) { fclose(pOpus->pFile); return DR_OPUS_INVALID_FILE; }
    if (!dr_opus__read_ogg_page(pOpus)) { fclose(pOpus->pFile); return DR_OPUS_INVALID_FILE; }
    if (!dr_opus__read_opus_tags(pOpus)) { fclose(pOpus->pFile); return DR_OPUS_INVALID_FILE; }
    
    /* Create decoder */
    int err;
    pOpus->pOpusDecoder = opus_decoder_create(48000, pOpus->channels, &err);
    if (!pOpus->pOpusDecoder) { fclose(pOpus->pFile); return DR_OPUS_ERROR; }
    
    pOpus->preSkipRemaining = pOpus->preSkip;
    pOpus->oggHeadersRead = DRLIBS_TRUE;
    
    return DR_OPUS_SUCCESS;
}

dr_opus_result dr_opus_init_memory(const void* pData, size_t dataSize, const dr_opus_allocation_callbacks* pAlloc, dr_opus* pOpus) {
    if (!pOpus || !pData || dataSize == 0) return DR_OPUS_INVALID_ARGS;
    
    memset(pOpus, 0, sizeof(*pOpus));
    if (pAlloc) pOpus->alloc = *pAlloc;
    
    dr_opus__init_crc32();
    
    pOpus->pMemory = (drlibs_uint8*)pData;
    pOpus->memorySize = dataSize;
    pOpus->memoryPos = 0;
    
    if (!dr_opus__read_ogg_page(pOpus)) return DR_OPUS_INVALID_FILE;
    if (!dr_opus__read_opus_head(pOpus)) return DR_OPUS_INVALID_FILE;
    if (!dr_opus__read_ogg_page(pOpus)) return DR_OPUS_INVALID_FILE;
    if (!dr_opus__read_opus_tags(pOpus)) return DR_OPUS_INVALID_FILE;
    
    int err;
    pOpus->pOpusDecoder = opus_decoder_create(48000, pOpus->channels, &err);
    if (!pOpus->pOpusDecoder) return DR_OPUS_ERROR;
    
    pOpus->preSkipRemaining = pOpus->preSkip;
    pOpus->oggHeadersRead = DRLIBS_TRUE;
    
    return DR_OPUS_SUCCESS;
}

void dr_opus_uninit(dr_opus* pOpus) {
    if (!pOpus) return;
    if (pOpus->pOpusDecoder) opus_decoder_destroy(pOpus->pOpusDecoder);
    if (pOpus->pFile) fclose((FILE*)pOpus->pFile);
    memset(pOpus, 0, sizeof(*pOpus));
}

drlibs_uint64 dr_opus_read_pcm_frames_f32(dr_opus* pOpus, drlibs_uint64 framesToRead, float* pBufferOut) {
    if (!pOpus || !pOpus->pOpusDecoder || framesToRead == 0) return 0;
    
    drlibs_uint64 framesRead = 0;
    
    while (framesRead < framesToRead) {
        /* Use buffered samples first */
        if (pOpus->decodeBufferPos < pOpus->decodeBufferLen) {
            drlibs_uint32 toCopy = pOpus->decodeBufferLen - pOpus->decodeBufferPos;
            if (toCopy > framesToRead - framesRead) toCopy = (drlibs_uint32)(framesToRead - framesRead);
            
            if (pBufferOut) {
                memcpy(pBufferOut + framesRead * pOpus->channels,
                       pOpus->decodeBuffer + pOpus->decodeBufferPos * pOpus->channels,
                       toCopy * pOpus->channels * sizeof(float));
            }
            pOpus->decodeBufferPos += toCopy;
            framesRead += toCopy;
            continue;
        }
        
        /* Decode next packet */
        drlibs_uint8 packet[4096];
        drlibs_int32 packetSize = dr_opus__read_packet(pOpus, packet, sizeof(packet));
        if (packetSize <= 0) break;
        
        int samples = opus_decode_float(pOpus->pOpusDecoder, packet, packetSize, 
                                        pOpus->decodeBuffer, 5760, 0);
        if (samples <= 0) break;
        
        /* Handle pre-skip */
        drlibs_uint32 skip = 0;
        if (pOpus->preSkipRemaining > 0) {
            skip = (pOpus->preSkipRemaining > (drlibs_uint32)samples) ? samples : pOpus->preSkipRemaining;
            pOpus->preSkipRemaining -= skip;
        }
        
        pOpus->decodeBufferPos = skip;
        pOpus->decodeBufferLen = samples;
    }
    
    return framesRead;
}

drlibs_uint64 dr_opus_read_pcm_frames_s16(dr_opus* pOpus, drlibs_uint64 framesToRead, drlibs_int16* pBufferOut) {
    float tempBuffer[1024];
    drlibs_uint64 framesRead = 0;
    
    while (framesRead < framesToRead) {
        drlibs_uint64 toRead = framesToRead - framesRead;
        if (toRead > 1024 / pOpus->channels) toRead = 1024 / pOpus->channels;
        
        drlibs_uint64 got = dr_opus_read_pcm_frames_f32(pOpus, toRead, tempBuffer);
        if (got == 0) break;
        
        for (drlibs_uint64 i = 0; i < got * pOpus->channels; i++) {
            float s = tempBuffer[i] * 32767.0f;
            if (s > 32767) s = 32767;
            if (s < -32768) s = -32768;
            pBufferOut[framesRead * pOpus->channels + i] = (drlibs_int16)s;
        }
        framesRead += got;
    }
    return framesRead;
}

drlibs_bool32 dr_opus_seek_to_pcm_frame(dr_opus* pOpus, drlibs_uint64 frameIndex) {
    (void)pOpus; (void)frameIndex;
    return DRLIBS_FALSE; /* Seeking not implemented yet */
}

float* dr_opus_open_file_and_read_pcm_frames_f32(const char* pFilePath, int* pChannels, int* pSampleRate, drlibs_uint64* pTotalFrameCount, const dr_opus_allocation_callbacks* pAlloc) {
    dr_opus opus;
    if (dr_opus_init_file(pFilePath, pAlloc, &opus) != DR_OPUS_SUCCESS) return NULL;
    
    size_t capacity = 48000 * 10 * opus.channels;
    float* pSamples = (float*)dr_opus__malloc(capacity * sizeof(float), pAlloc);
    drlibs_uint64 totalFrames = 0;
    
    float buffer[4096];
    for (;;) {
        drlibs_uint64 got = dr_opus_read_pcm_frames_f32(&opus, 1024, buffer);
        if (got == 0) break;
        
        if ((totalFrames + got) * opus.channels > capacity) {
            capacity *= 2;
            pSamples = (float*)dr_opus__realloc(pSamples, capacity * sizeof(float), pAlloc);
        }
        memcpy(pSamples + totalFrames * opus.channels, buffer, got * opus.channels * sizeof(float));
        totalFrames += got;
    }
    
    if (pChannels) *pChannels = opus.channels;
    if (pSampleRate) *pSampleRate = 48000;
    if (pTotalFrameCount) *pTotalFrameCount = totalFrames;
    
    dr_opus_uninit(&opus);
    return pSamples;
}

drlibs_int16* dr_opus_open_file_and_read_pcm_frames_s16(const char* pFilePath, int* pChannels, int* pSampleRate, drlibs_uint64* pTotalFrameCount, const dr_opus_allocation_callbacks* pAlloc) {
    int ch;
    drlibs_uint64 frames;
    float* f = dr_opus_open_file_and_read_pcm_frames_f32(pFilePath, &ch, pSampleRate, &frames, pAlloc);
    if (!f) return NULL;
    
    drlibs_int16* s = (drlibs_int16*)dr_opus__malloc(frames * ch * sizeof(drlibs_int16), pAlloc);
    for (drlibs_uint64 i = 0; i < frames * ch; i++) {
        float v = f[i] * 32767.0f;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        s[i] = (drlibs_int16)v;
    }
    dr_opus__free(f, pAlloc);
    
    if (pChannels) *pChannels = ch;
    if (pTotalFrameCount) *pTotalFrameCount = frames;
    return s;
}

float* dr_opus_open_memory_and_read_pcm_frames_f32(const void* pData, size_t dataSize, int* pChannels, int* pSampleRate, drlibs_uint64* pTotalFrameCount, const dr_opus_allocation_callbacks* pAlloc) {
    dr_opus opus;
    if (dr_opus_init_memory(pData, dataSize, pAlloc, &opus) != DR_OPUS_SUCCESS) return NULL;
    
    size_t capacity = 48000 * 10 * opus.channels;
    float* pSamples = (float*)dr_opus__malloc(capacity * sizeof(float), pAlloc);
    drlibs_uint64 totalFrames = 0;
    
    float buffer[4096];
    for (;;) {
        drlibs_uint64 got = dr_opus_read_pcm_frames_f32(&opus, 1024, buffer);
        if (got == 0) break;
        
        if ((totalFrames + got) * opus.channels > capacity) {
            capacity *= 2;
            pSamples = (float*)dr_opus__realloc(pSamples, capacity * sizeof(float), pAlloc);
        }
        memcpy(pSamples + totalFrames * opus.channels, buffer, got * opus.channels * sizeof(float));
        totalFrames += got;
    }
    
    if (pChannels) *pChannels = opus.channels;
    if (pSampleRate) *pSampleRate = 48000;
    if (pTotalFrameCount) *pTotalFrameCount = totalFrames;
    
    dr_opus_uninit(&opus);
    return pSamples;
}

drlibs_int16* dr_opus_open_memory_and_read_pcm_frames_s16(const void* pData, size_t dataSize, int* pChannels, int* pSampleRate, drlibs_uint64* pTotalFrameCount, const dr_opus_allocation_callbacks* pAlloc) {
    int ch;
    drlibs_uint64 frames;
    float* f = dr_opus_open_memory_and_read_pcm_frames_f32(pData, dataSize, &ch, pSampleRate, &frames, pAlloc);
    if (!f) return NULL;
    
    drlibs_int16* s = (drlibs_int16*)dr_opus__malloc(frames * ch * sizeof(drlibs_int16), pAlloc);
    for (drlibs_uint64 i = 0; i < frames * ch; i++) {
        float v = f[i] * 32767.0f;
        if (v > 32767) v = 32767;
        if (v < -32768) v = -32768;
        s[i] = (drlibs_int16)v;
    }
    dr_opus__free(f, pAlloc);
    
    if (pChannels) *pChannels = ch;
    if (pTotalFrameCount) *pTotalFrameCount = frames;
    return s;
}

void dr_opus_free(void* p, const dr_opus_allocation_callbacks* pAlloc) {
    dr_opus__free(p, pAlloc);
}


#endif /* DR_OPUS_IMPLEMENTATION */
