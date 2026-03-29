#ifndef MINIZ_SIMPLE_H
#define MINIZ_SIMPLE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MZ_OK 0
#define MZ_STREAM_END 1
#define MZ_STREAM_ERROR -2
#define MZ_DATA_ERROR -3
#define MZ_MEM_ERROR -4

typedef struct mz_stream_s {
    const unsigned char *next_in;
    unsigned int avail_in;
    unsigned long total_in;
    
    unsigned char *next_out;
    unsigned int avail_out;
    unsigned long total_out;
    
    void *state;
    void *alloc_func;
    void *free_func;
    void *opaque;
    
    int data_type;
    unsigned long adler;
    unsigned long reserved;
} mz_stream;

int mz_compress(unsigned char *dest, unsigned long *dest_len, 
                const unsigned char *source, unsigned long source_len);
int mz_compress2(unsigned char *dest, unsigned long *dest_len, 
                 const unsigned char *source, unsigned long source_len, int level);
int mz_uncompress(unsigned char *dest, unsigned long *dest_len, 
                  const unsigned char *source, unsigned long source_len);
unsigned long mz_compressBound(unsigned long source_len);

#ifdef __cplusplus
}
#endif

#endif
