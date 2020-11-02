#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "parseArgs.h"
#include "huff.h"

int main(int argc, char *argv[]) {
    // Qualify arguments given
    char *outfile = NULL;
    char *infile = NULL;;
    char compression = 0;

    if (argc > 4) {
	printf("usage: ./huff [-c | -d] infile outfile\n");
	return 1;
    }

    args_setup(argc, argv);
    arg a = args_getOne();

    if (a.type == FLAG && (a.arg.flag == 'd' || a.arg.flag == 'c')) {
	compression = a.arg.flag;
    }
    else {
	printf("usage: ./huff [-c | -d] infile outfile\n");
	return 1;
    }

    a = args_getOne();
    if (a.type == STRING)
	infile = a.arg.argument;
    a = args_getOne();
    if (a.type == STRING)
	outfile = a.arg.argument;
    if (infile == NULL || outfile == NULL) {
	printf("usage: ./huff [-c | -d] infile outfile\n");
	return 1;
    }

    // If the file to write out already exists warn user before
    // overwriting the existing file
    if (access(outfile, F_OK) != -1) {
	while (1) {
	    printf("%s already exists!\nDo you want to overwrite? (y/n) : ", outfile);
	    char check = getchar();
	    if (check == 'N' || check == 'n') {
		printf("Aborting\n");
		while((check = getchar()) != '\n' && check != EOF);
		return 0;
	    }
	    else if (check == 'Y' || check == 'y') {
		while((check = getchar()) != '\n' && check != EOF);
		break;
	    }
	    while((check = getchar()) != '\n' && check != EOF);
	}
    }

    enum huff_ERROR hError = huff_OK;

    if (compression == 'c') {
	hError = huff_initIO(infile, outfile, COMPRESS);
	if (hError != huff_OK) {
	    printf("Couldn't figure out how to open the files\n");
	    return 1;
	}
	
	hError = huff_compressWrite();
	if (hError != huff_OK) {
	    printf("Totally failed compressing the file\n");
	    return 1;
	}
    }
    else if (compression == 'd') {
	hError = huff_initIO(infile, outfile, DECOMPRESS);
	if (hError != huff_OK) {
	    printf("Couldn't figure out how to open the files\n");
	    return 1;
	}
	hError = huff_decompressWrite(huff_CHECKFILE);
	if (hError == huff_BADHEADER) {
	    printf("It appears the compressed file is not the right format... Exiting\n");
	    return 0;
	}
	if (hError == huff_COMPRESS_BADCHECKSUM) {
	    printf("Checksum mismatch for compressed file.\nThe compressed file may be corruped or have been tampered with.\n");
	    char proceed = 0;	    
	    while (1) {
		printf("Do you wish to proceed? (y/n) : ");
		proceed = getchar();
		if (proceed == 'n' || proceed == 'N') {
		    printf("Exiting...\n");
		    return 0;
		}
		if (proceed == 'y' || proceed == 'Y') {
		    while ((proceed = getchar()) != '\n' && proceed != EOF);
		    break;
		}
	    }
	    hError = huff_decompressWrite(huff_CHECKFILE_SKIP);	    
	}
	if (hError == huff_DECOMPRESS_BADCHECKSUM) {
	    printf("Complete: Checksum mismatch for decompressed file\n");
	    return 1;
	}
	else if (hError != huff_OK) {
	    printf("Decompression failed!\n");
	}
    }

    huff_clean();
    return 0;
}
