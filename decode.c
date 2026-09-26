#include<stdio.h>
#include<string.h>
#include "encode.h"
#include "decode.h"
#include "common.h"
#include "types.h"


Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo)
{
    //stego file Extension validation
    char *dot = strchr(argv[2],'.');

    if((dot == NULL) || (strcmp(dot,".bmp") != 0)){
        printf("Error : Output file must be .bmp file\n");
        return e_failure;
    }

    //storing stego file name
    decInfo->stego_image_fname = argv[2];

    //storing output secret/decode.txt file name
    if(argv[3] == NULL){
        strcpy(decInfo->decode_fname,"decode.txt");
    }
    else{
        //storing name of decode file till '.'
        sscanf(argv[3],"%[^.]",decInfo->decode_fname);
        strcat(decInfo->decode_fname,".txt");   //storing .txt in the end
    }

    //open all files
    if(open_decfiles(decInfo) == e_failure){
        printf("File not opened\n");
        return e_failure;
    }

    //Signature(BM) validation
    char signature[2];

        //reading first 2 bytes of stego image
    if(fread(signature,2,1,decInfo->fptr_stego_image) == 0)
        return e_failure;

    if(signature[0] != 'B' || signature[1] != 'M'){
        printf("Error : Signature is wrong\n");
        return e_failure;
    }
    
    return e_success;
}

Status open_decfiles(DecodeInfo *decInfo)
{
    //stego file open
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"r");
    if(decInfo->fptr_stego_image == NULL){
        printf("Error : Stego file not opened\n");
        return e_failure;
    }

    //decode file open
    decInfo->fptr_decode = fopen(decInfo->decode_fname,"w");
    if(decInfo->fptr_decode == NULL){
        printf("Error : Decode file not opened\n");
        return e_failure;
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    if(decode_magic_string(MAGIC_STRING,decInfo) == e_failure){
        printf("Error : Magic string doesnot match\n");
        return e_failure;
    }

    if(decode_secret_file_extn_size(decInfo) == e_failure){
        printf("Error : Failed to decode secret file extension size\n");
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo) == e_failure){
        printf("Error : Failed to decode secret file extension\n");
        return e_failure;
    }

    if(decode_secret_file_size(decInfo) == e_failure){
        printf("Error : Failed to decode secret file size\n");
        return e_failure;
    }

    if(decode_secret_file_data(decInfo) == e_failure){
        printf("Error : Failed to decode secret file data\n");
        return e_failure;
    }


    return e_success;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    fseek(decInfo->fptr_stego_image,54,SEEK_SET);

    char buffer[8];

    for(int i=0;i<2;i++){
        if(fread(buffer,8,1,decInfo->fptr_stego_image) == 0)
            return e_failure;

        char ch = 0;

        for(int j=7; j>=0; j--){
            if(buffer[7- j] & 1)
                ch = ch | (1<<j);
        }

        if(ch != magic_string[i])
            return e_failure;
    }
    
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];

    //read 32 byte from stego file
    if(fread(buffer,32,1,decInfo->fptr_stego_image) == 0)
        return e_failure;


    int size = 0;

    for(int i=31;i>=0;i--){
        if(buffer[31-i] & 1)
            size = size | (1<<i);
    }

    decInfo->secret_file_extn_size = size;

    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];

    int i;
    for(i=0;i < decInfo->secret_file_extn_size;i++){
        if(fread(buffer,8,1,decInfo->fptr_stego_image) == 0)    //reading 8 bytes from stego file to buffer
            return e_failure;

        char ch = 0;

        for(int j=7; j>=0; j--){    //decoding 
            if(buffer[7- j] & 1)
                ch = ch | (1<<j);
        }

        decInfo->extn_secret_file[i] = ch;      //storing decoded data
    }

    decInfo->extn_secret_file[i] = '\0';


    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if(fread(buffer,32,1,decInfo->fptr_stego_image) == 0)
        return e_failure;

    int size = 0;

    for(int i=31;i>=0;i--){
        if(buffer[31-i] & 1)
            size = size | (1<<i);
    }

    decInfo->secret_file_size = size;

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];

    for(int i=0;i < decInfo->secret_file_size; i++){
        if(fread(buffer,8,1,decInfo->fptr_stego_image) == 0)
            return e_failure;

        char ch = 0;

        for(int j=7; j>=0; j--){    //decoding 
            if(buffer[7- j] & 1)
                ch = ch | (1<<j);
        }

        if(fwrite(&ch,1,1,decInfo->fptr_decode) == 0)
            return e_failure;
    }

    return e_success;
}