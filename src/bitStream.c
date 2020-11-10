#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bitStream.h"

bitStream *bitStream_create(uint32_t size) {
    bitStream *result = calloc(sizeof(bitStream), 1);
    if (result == NULL)
	return NULL;
    result->data = calloc(sizeof(uint8_t), size);
    if (result->data == NULL)
	return NULL;
    result->data_size = size;
    result->last_bit = size * 8;
    return result;
}

void bitStream_destroy(bitStream *toDestroy) {
    free(toDestroy->data);
    free(toDestroy);
}

void bitStream_clearData(bitStream *toClear) {
    memset(toClear->data, 0, toClear->data_size);
    toClear->current_bit_offset = 0;
    toClear->current_byte_offset = 0;
}

uint8_t bitStream_readBit(bitStream *toRead) {
    uint8_t bit = toRead->data[toRead->current_byte_offset] & (1 << (7 - (toRead->current_bit_offset % 8)));
    toRead->current_bit_offset++;
    if(!(toRead->current_bit_offset % 8))
	toRead->current_byte_offset++;
    return bit ? 1 : 0;
}

int64_t bitStream_addToData(bitStream *toFill, uint8_t *values,
			    uint64_t bits_size, uint64_t initial_bit_offset) {
    char values_bitOffset = initial_bit_offset % 8;
    char toFill_bitOffset = toFill->current_bit_offset % 8;
    uint32_t values_position = initial_bit_offset / 8;
    
    while (initial_bit_offset < bits_size) {
	// Data buffer is full
	if (toFill->current_bit_offset >= toFill->last_bit) {
	    return bits_size - initial_bit_offset;
	}
	
	uint8_t bit = values[values_position] & (1 << (7 - values_bitOffset));
	if (bit)
	    toFill->data[toFill->current_byte_offset] |= (1 << (7 - toFill_bitOffset));

	toFill->current_bit_offset++;
	toFill_bitOffset++;
	values_bitOffset++;
	initial_bit_offset++;
	
	if (toFill_bitOffset >= 8) {
	    toFill->current_byte_offset++;
	    toFill_bitOffset = 0;
	}
	if (values_bitOffset >= 8) {
	    values_position++;
	    values_bitOffset = 0;
	}
    }

    return 0;
}
