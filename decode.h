#ifndef DECODE_H
#define DECODE_H

#include<stdio.h>
#include "types.h"

#define MAX_FILE_SUFFIX 5

typedef struct _DecodeInfo
{
    /* Output secret file info(decode.txt) */
    char decode_fname[50];
    FILE *fptr_decode;
    
    /* Secret file info */
    int secret_file_extn_size;
    char extn_secret_file[MAX_FILE_SUFFIX];
    long secret_file_size;


    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;

/* Decoding function prototype */

/* Check operation type */
OperationType check_operation_type(char opt);

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decfiles(DecodeInfo *decInfo);

/* Decode Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

//Decode secret file extenstion size
Status decode_secret_file_extn_size(DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Encode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);




#endif