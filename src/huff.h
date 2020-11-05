#ifndef HUFF_DEF
#define HUFF_DEF

#include <stdint.h>
#include <stdio.h>

#include "bitStream.h"

#define BUFFER_SIZE 4096
#define POSSIBLE_VALUES 256

// DECOMPRESS and COMPRESS are the type of file respectively
// COMPRESS is the compressed file
enum IO_TYPE {huff_NONE, DECOMPRESS, COMPRESS};
enum huff_ERROR {huff_OK, huff_hIN_BAD, huff_hOUT_BAD, huff_COMPRESS_BADCHECKSUM,
                 huff_DECOMPRESS_BADCHECKSUM, huff_OOM, huff_BADHEADER};
#define huff_CHECKFILE 1
#define huff_CHECKFILE_SKIP 0

// head_number shall be "HUFF" always
// hi - compressed checksum only
// lo - decompressed checksum only
// sizeof(huff_header) is 12 bytes
typedef struct {
    char head_number[3];
    uint8_t last_bits_ignore;
    union {
	struct {
	    uint8_t hi;
	    uint8_t lo;
	};
	uint16_t all;
    } checksum;
    uint16_t frequencies_length;
} __attribute__((packed)) huff_header;

typedef struct {
    uint32_t freq;
    uint8_t value;
} __attribute__((packed)) huff_table;

typedef struct {
    enum IO_TYPE type;
    FILE *file;
    uint32_t bytes_read;
    uint32_t buffer_at;
    bitStream *buffer;
} huff_fileIO;

enum huff_ERROR huff_initIO(char *infile, char *outfile, enum IO_TYPE comp);

void huff_clean();

enum huff_ERROR huff_compressWrite();

enum huff_ERROR huff_decompressWrite(unsigned int checkSum_bypass);

#endif
