#include "miniz_simple.h"
#include <string.h>

int mz_compress(unsigned char *dest, unsigned long *dest_len, 
                const unsigned char *source, unsigned long source_len) {
    // Simple implementation - just copy data without compression for now
    // This prevents corruption while we fix the compression
    if (*dest_len < source_len) return -4;
    
    memcpy(dest, source, source_len);
    *dest_len = source_len;
    return 0;
}

int mz_compress2(unsigned char *dest, unsigned long *dest_len, 
                 const unsigned char *source, unsigned long source_len, int level) {
    (void)level;
    return mz_compress(dest, dest_len, source, source_len);
}

int mz_uncompress(unsigned char *dest, unsigned long *dest_len, 
                  const unsigned char *source, unsigned long source_len) {
    // Simple implementation - just copy data without decompression for now
    if (*dest_len < source_len) return -4;
    
    memcpy(dest, source, source_len);
    *dest_len = source_len;
    return 0;
}

unsigned long mz_compressBound(unsigned long source_len) {
    return source_len + 128;
}
