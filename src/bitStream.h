#ifndef BITSTREAM_DEF
#define BITSTREAM_DEF

#include <stdint.h>

typedef struct {
    uint32_t data_size;
    uint32_t current_byte_offset;

    uint8_t *data;

    uint64_t last_bit;
    uint64_t current_bit_offset;
} bitStream;

bitStream *bitStream_create(uint32_t size);

void bitStream_destroy(bitStream *toDestroy);

// Reset data stream to init state
void bitStream_clearData(bitStream *toClear);

// Reads single bit from data stream
// Updates data stream bit position
// Does not track "End of Stream"
uint8_t bitStream_readBit(bitStream *toRead);

// Adds values to data at bit level
// If data stream is filled then returns
// number of unfilled bits
int64_t bitStream_addToData(bitStream *toFill, uint8_t *values,
			    uint64_t bits_size, uint64_t initial_bit_offset);

#endif
