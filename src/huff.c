#include <stdlib.h>
#include <string.h>

#include "huff.h"
#include "minTree.h"
#include "bitStream.h"

huff_header header = {
    .head_number = {'H', 'U', 'F'},
    .checksum.all = 0,
    .frequencies_length = 0,
};

huff_table ht[256];
minBTree *huffTree;
unsigned int ht_length;

huff_fileIO hIN = {.type = huff_NONE};
huff_fileIO hOUT = {.type = huff_NONE};

FILE *huff_openFile(char *fname, char method) {
    if(method != 'r' && method != 'w') {
	return NULL;
    }

    FILE *result = method == 'r' ? fopen(fname, "r") : fopen(fname, "w+");
    if(result == NULL) {
	return NULL;
    }

    return result;
}

enum huff_ERROR huff_initIO(char *infile, char *outfile, enum IO_TYPE comp) {
    // Later check if outfile already exists
    // Warn, and ask to continue

    hIN = (huff_fileIO){
	.type = comp == COMPRESS ? DECOMPRESS : COMPRESS,
	.file = huff_openFile(infile, 'r'),
	.bytes_read = 0,
	.buffer_at = 0,
	.buffer = bitStream_create(BUFFER_SIZE)
    };
    if (hIN.file == NULL) {
	bitStream_destroy(hIN.buffer);
	return huff_hIN_BAD;
    }
    else if (hIN.buffer == NULL) {
	fclose(hIN.file);
	return huff_OOM;
    }
    
    hOUT = (huff_fileIO){
	.type = comp == COMPRESS ? COMPRESS : DECOMPRESS,
	.file = huff_openFile(outfile, 'w'),
	.bytes_read = 0,
	.buffer_at = 0,
	.buffer = bitStream_create(BUFFER_SIZE)
    };
    if (hOUT.file == NULL) {
	bitStream_destroy(hIN.buffer);
	bitStream_destroy(hOUT.buffer);
	fclose(hIN.file);
	return huff_hOUT_BAD;
    }
    else if (hOUT.buffer == NULL) {
	bitStream_destroy(hIN.buffer);
	fclose(hIN.file);
	fclose(hOUT.file);
	return huff_hOUT_BAD;
    }

    return huff_OK;
}

void huff_clean() {
    if (hIN.file != NULL) {
	fclose(hIN.file);
    }
    if (hIN.buffer != NULL) {
	bitStream_destroy(hIN.buffer);
    }
    if (hOUT.file != NULL) {
	fclose(hOUT.file);
    }
    if (hOUT.buffer != NULL) {
	bitStream_destroy(hOUT.buffer);
    }

    hIN = (huff_fileIO){
	.type = huff_NONE,
	.file = NULL,
	.bytes_read = 0,
	.buffer_at = 0,
	.buffer = NULL
    };

    hOUT = (huff_fileIO){
	.type = huff_NONE,
	.file = NULL,
	.bytes_read = 0,
	.buffer_at = 0,
	.buffer = NULL
    };

    if (huffTree != NULL) {
	minTree_destroyTree(huffTree);
    }
}

void huff_resetFileState(huff_fileIO *f) {
    fseek(f->file, 0, SEEK_SET);
    memset(f->buffer->data, 0, BUFFER_SIZE);
    f->bytes_read = 0;
    f->buffer_at = 0;    
}

unsigned int huff_readHeader() {
    if(hIN.type == COMPRESS) {
	fread(&header, 1, sizeof(huff_header), hIN.file);
    }
    ht_length = header.frequencies_length;
    huff_resetFileState(&hIN);

    return 0;
}

unsigned int huff_uniqueTable() {
    // Pull all entries in the table into lowest free indexes
    // We are going to play a little cat and mouse at first
    unsigned int lowest_idx = 0;
    for (int i = lowest_idx; i < POSSIBLE_VALUES && lowest_idx < POSSIBLE_VALUES; i++) {
	while(ht[lowest_idx].freq && lowest_idx < POSSIBLE_VALUES) lowest_idx++;
	while(!ht[i].freq && i < POSSIBLE_VALUES) i++;
	if (ht[i].freq && i < POSSIBLE_VALUES && lowest_idx < POSSIBLE_VALUES) {
	    ht[lowest_idx] = ht[i];
	    ht[i] = (huff_table){0, 0};
	}
    }

    // May as well set ht_length now
    ht_length = lowest_idx;

    // Sort the table
    // We are just using bubble sort at the moment
    // because 256^2 is defined as the worst case
    // There is no reason to be more agressive and
    // bubble sort is very easy to reason about
    for (unsigned int i = 0; i < ht_length; i++) {
	for (unsigned int j = 0; j < ht_length - 1; j++) {
	    if (ht[j].freq > ht[j + 1].freq) {
		huff_table temp = ht[j];
		ht[j] = ht[j + 1];
		ht[j + 1] = temp;
	    }
	}
    }

    // Ensure that all resulting elements in the table are unique
    // This is done by spreading the upper frequencies up by one
    // for every subsequent index in the remaining table
    for (unsigned int i = 0; i < ht_length - 1; i++) {
	if (ht[i].freq == ht[i + 1].freq) {
	    for (unsigned int j = i + 1; j < ht_length; j++) {
		ht[j].freq++;
	    }
	}
    }

    return ht_length;
}

unsigned int huff_createTable() {
    if(hIN.type == DECOMPRESS) {
	// init the ht table
	for (int i = 0; i < POSSIBLE_VALUES; i++) {
	    ht[i].freq = 0;
	    ht[i].value = i;
	}

	while ((hIN.bytes_read = fread(hIN.buffer->data, 1, BUFFER_SIZE, hIN.file)) > 0) {
	    hIN.buffer_at = 0;
	    while(hIN.buffer_at < hIN.bytes_read) {
		ht[hIN.buffer->data[hIN.buffer_at]].freq++;
		hIN.buffer_at++;
	    }
	}
	// Ensure all values in the table are indeed unique
	huff_uniqueTable();
    }
    else {
	// Just read in the table from the file
	fseek(hIN.file, sizeof(huff_header), SEEK_SET);
	fread(&ht, sizeof(huff_table), header.frequencies_length, hIN.file);
	ht_length = header.frequencies_length;
    }
    
    // Reset hIN back to an unread state
    huff_resetFileState(&hIN);

    return 1;
}

uint8_t huff_checksum(huff_fileIO *hf, uint32_t start) {
    uint8_t result = 0;

    fseek(hf->file, start, SEEK_SET);
    while ((hf->bytes_read = fread(hf->buffer->data, 1, 4096, hf->file)) > 0) {
	hf->buffer_at = 0;
	while(hf->buffer_at < hf->bytes_read) {
	    result = result ^ hf->buffer->data[hf->buffer_at];
	    hf->buffer_at++;
	}
    }        

    huff_resetFileState(hf);
    return result;
}

unsigned int huff_tableToTree() {
    huffTree = minTree_createTree(ht_length, ht);
    if (huffTree == NULL)
	return 0;
    return 1;
}

// This ties everything in above to actually do the work
// of compressing everything
enum huff_ERROR huff_compressWrite() {
    if (hIN.type != DECOMPRESS || hIN.file == NULL) {
	return huff_hIN_BAD;
    }
    if (hOUT.type != COMPRESS || hOUT.file == NULL) {
	return huff_hOUT_BAD;
    }

    huff_createTable();
    if (!huff_tableToTree()) {
	return huff_OOM;
    }

    // Skip forward in the hOUT file far enough to later
    // write out the header and table
    fwrite(&header, sizeof(huff_header), 1, hOUT.file);
    fwrite(ht, sizeof(huff_table), ht_length, hOUT.file);

    // Start reading in, and writing out our bits
    while ((hIN.bytes_read = fread(hIN.buffer->data, 1, 4096, hIN.file)) > 0) {
	hIN.buffer_at = 0;
	
	while (hIN.buffer_at < hIN.bytes_read) {
	    uint64_t bitsRead = minTree_toBits(hIN.buffer->data[hIN.buffer_at], huffTree);
	    uint64_t shouldWrite = bitStream_addToData(hOUT.buffer, (uint8_t *)treeBits, bitsRead, 0);
	    if (shouldWrite) {
		fwrite(hOUT.buffer->data, 1, hOUT.buffer->current_byte_offset, hOUT.file);
		memset(hOUT.buffer->data, 0, BUFFER_SIZE);
		hOUT.buffer_at = 0;
		bitStream_clearData(hOUT.buffer);
		// Write in the bits that would have overflowed the data buffer
		bitStream_addToData(hOUT.buffer, (uint8_t *)treeBits, bitsRead, bitsRead - shouldWrite);
	    }
	    hIN.buffer_at++;
	}
    }

    // Off by 1 for buffer->current_byte_offset
    fwrite(hOUT.buffer->data, 1, hOUT.buffer->current_byte_offset + 1, hOUT.file);

    huff_resetFileState(&hIN);
    huff_resetFileState(&hOUT);
    header.checksum.lo = huff_checksum(&hIN, 0);
    header.checksum.hi = huff_checksum(&hOUT, sizeof(huff_header) + (sizeof(huff_table) * ht_length));

    header.frequencies_length = ht_length;
    header.last_bits_ignore = hOUT.buffer->current_bit_offset % 8;
    fwrite(&header, sizeof(huff_header), 1, hOUT.file);
    fwrite(ht, sizeof(huff_table), ht_length, hOUT.file);

    return huff_OK;
}

enum huff_ERROR huff_decompressWrite(unsigned int checkSum_bypass) {
    // Just a checkSum structure
    union {
	struct {
	    uint8_t hi;
	    uint8_t lo;
	};
	uint16_t all;
    } checkSum;

    if (hIN.type != COMPRESS || hIN.file == NULL) {
	return huff_hIN_BAD;
    }
    if (hOUT.type != DECOMPRESS || hOUT.file == NULL) {
	return huff_hOUT_BAD;
    }

    if (checkSum_bypass == huff_CHECKFILE) {
	huff_readHeader();
	if (header.head_number[0] != 'H' ||
	    header.head_number[1] != 'U' ||
	    header.head_number[2] != 'F') {
	    return huff_BADHEADER;
	}
	checkSum.hi = huff_checksum(&hIN, sizeof(huff_header) + (sizeof(huff_table) * ht_length));
	if (checkSum.hi != header.checksum.hi) {
	    return huff_COMPRESS_BADCHECKSUM;
	}
    }
    
    // Get the total file size up front
    fseek(hIN.file, 0, SEEK_END);
    uint32_t compressed_length = ftell(hIN.file);
    rewind(hIN.file);

    huff_createTable();
    if (!huff_tableToTree()) {
	return huff_OOM;
    }    

    // Read ahead to where compressed data starts
    fseek(hIN.file, sizeof(huff_header) + (header.frequencies_length * sizeof(huff_table)), SEEK_SET);

    // Now adjust compressed_length to be only the number of bytes
    // used for the compression bits
    compressed_length = compressed_length - ftell(hIN.file);
    struct valueNode_tuple check = {-1, huffTree};

    // We only want to read up to the last byte of the data
    // Due to variability of left over bits in last byte
    while (compressed_length > 1) {
	bitStream_clearData(hIN.buffer);
	hIN.bytes_read = fread(hIN.buffer->data, 1, 4096, hIN.file);
	while (hIN.buffer->current_byte_offset < hIN.bytes_read &&
	       (compressed_length - hIN.buffer->current_byte_offset) > 1) {
	    uint8_t bit = bitStream_readBit(hIN.buffer);
	    check = minTree_fromBit(bit, check.node);

	    if (check.node == NULL) {
		hOUT.buffer->data[hOUT.buffer_at] = check.value;
		check = (struct valueNode_tuple) {-1, huffTree};
		hOUT.buffer_at++;
		if (hOUT.buffer_at >= BUFFER_SIZE) {
		    fwrite(hOUT.buffer->data, 1, BUFFER_SIZE, hOUT.file);
		    bitStream_clearData(hOUT.buffer);
		    hOUT.buffer_at = 0;
		}
	    }
	}
	compressed_length -= hIN.bytes_read;
    }

    // Now we resolve the last byte of the data, and write out
    // the last chunk to our decompressed file
    uint32_t final_bits = 0;
    while (final_bits < header.last_bits_ignore) {
	uint8_t bit = bitStream_readBit(hIN.buffer);
	check = minTree_fromBit(bit, check.node);

	if (check.node == NULL) {
	    hOUT.buffer->data[hOUT.buffer_at] = check.value;
	    check = (struct valueNode_tuple) {-1, huffTree};
	    hOUT.buffer_at++;
	    if (hOUT.buffer_at >= BUFFER_SIZE) {
		fwrite(hOUT.buffer->data, 1, BUFFER_SIZE, hOUT.file);
		bitStream_clearData(hOUT.buffer);
		hOUT.buffer_at = 0;
	    }
	}
	final_bits++;
    }

    // And write out the final bytes to the decompressed file
    if (hOUT.buffer_at) {
	fwrite(hOUT.buffer->data, 1, hOUT.buffer_at, hOUT.file);
    }

    huff_resetFileState(&hOUT);
    checkSum.lo = huff_checksum(&hOUT, 0);
    if (checkSum.lo != header.checksum.lo) {
	return huff_DECOMPRESS_BADCHECKSUM;
    }

    return huff_OK;
}
