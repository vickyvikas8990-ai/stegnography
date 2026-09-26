#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */


Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    
    char *dot = strchr(argv[2],'.');        //source file extension validation

    if(dot==NULL || (strcmp(dot,".bmp") != 0)){     
        printf("Error : Source image file must be .bmp file\n");
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];     //storing src file name 

    encInfo->secret_fname = argv[3];        //storing secret file name

    if(argv[4]==NULL){                      //storing output/stego file name
        encInfo->stego_image_fname = "output.bmp";
    }
    else{
        dot = strchr(argv[4],'.');          //output file extension validation

        if(dot==NULL || (strcmp(dot,".bmp") != 0)){
            printf("Error : Output file must be .bmp file\n");
            return e_failure;
        }

        encInfo->stego_image_fname = argv[4];   //storing output file name
    }

    if(open_files(encInfo) == e_failure){
        printf("File not opened\n");
        return e_failure;
    }

    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    //Src file open
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname,"r");    
    if(encInfo->fptr_src_image == NULL){
        printf("Error : Src file not opened\n");
        return e_failure;
    }

    //Secret file open
    encInfo->fptr_secret = fopen(encInfo->secret_fname,"r");
    if(encInfo->fptr_secret == NULL){
        printf("Error : Secret file not opened\n");
        return e_failure;
    }

    //Stego file open
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname,"w");
    if(encInfo->fptr_stego_image == NULL){
        printf("Error : Output file not opened\n");
        return e_failure;
    }

    return e_success;
}


Status do_encoding(EncodeInfo *encInfo)
{
    //capacity check for .bmp file and secret file
    if(check_capacity(encInfo) == e_failure){
        printf("Error : Insufficient image capacity to store secret file\n");
        return e_failure;
    }

    //copy .bmp header to stego
    if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure){
        printf("Error : BMP Header not copied\n");
        return e_failure;
    }

    //Encoding magic string(#*)
    if(encode_magic_string(MAGIC_STRING,encInfo) == e_failure){
        printf("Error : Unable to encode magic string\n");
        return e_failure;
    }

    //Encoding secret file extension size
    if(encode_secret_file_extn_size(encInfo) == e_failure){
        printf("Error : Failed to encode secret file extension size\n");
        return e_failure;
    }

    //Encoding secret file extension
    if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_failure){
        printf("Error : Unable to encode Secrect file extension\n");
        return e_failure;
    }

    //Encoding secret file size (of data inside secret file)
    if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_failure){
        printf("Error : Unable to encode Secret file size\n");
        return e_failure;
    }

    //Encoding secret file data
    if(encode_secret_file_data(encInfo) == e_failure){
        printf("Error : Unable to encode Secret file data\n");
        return e_failure;
    }

    //Copying the remaining data of src file to stego file
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_failure){
        printf("Error : Unable to copy remaining image\n");
        return e_failure;
    }

    return e_success;
}


Status check_capacity(EncodeInfo *encInfo)
{
    //storing size of src.bmp file
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    //storing size of secret file
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if(encInfo->image_capacity < (14 + encInfo->size_secret_file * 8)){
        printf("Error : Secret file size is bigger than .bmp file\n");
        return e_failure;
    }

    return e_success;
}

uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    return ftell(fptr);
}


Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char buffer[54];

    rewind(fptr_src_image);

    if(fread(buffer,54,1,fptr_src_image) == 0)
        return e_failure;

    if(fwrite(buffer,54,1,fptr_dest_image) == 0)
        return e_failure;

    return e_success;   
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char buffer[8];

    for(int i=0;i<2;i++){
        if(fread(buffer,8,1,encInfo->fptr_src_image) == 0)
            return e_failure;

        encode_byte_to_lsb(magic_string[i],buffer);

        if(fwrite(buffer,8,1,encInfo->fptr_stego_image) == 0)
            return e_failure;
    }

    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=7;i>=0;i--){
        if(data & (1<<i)){
            image_buffer[7-i] = image_buffer[7-i] | 1;
        }
        else{
            image_buffer[7-i] = image_buffer[7-i] & ~1;
        }
    }

    return e_success;
}

Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    //copying the extension of secrete file to dot
    char *dot = strchr(encInfo->secret_fname,'.');
    strcpy(encInfo->extn_secret_file,dot);

    char src_buffer[32];

    //read 32 bytes from src_file into buff
    if(fread(src_buffer,32,1,encInfo->fptr_src_image) == 0)
        return e_failure;

    encode_size_to_lsb(strlen(encInfo->extn_secret_file),src_buffer);

    if(fwrite(src_buffer,32,1,encInfo->fptr_stego_image) == 0)
        return e_failure;

    return e_success;
}

Status encode_size_to_lsb(int size,char *image_buffer)
{
    for(int i=31;i>=0;i--){
        if(size & (1<<i)){
            image_buffer[31-i] = image_buffer[31-i] | 1;   //set
        }
        else    
            image_buffer[31-i] = image_buffer[31-i] & ~1;   //clear
    }

    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char buffer[8];

    for(int i=0;file_extn[i] != 0;i++){
        if(fread(buffer,8,1,encInfo->fptr_src_image) == 0)
            return e_failure;

        encode_byte_to_lsb(file_extn[i],buffer);

        if(fwrite(buffer,8,1,encInfo->fptr_stego_image) == 0)
            return e_failure;
    }

    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char buffer[32];

    if(fread(buffer,32,1,encInfo->fptr_src_image) == 0)
        return e_failure;

    encode_size_to_lsb(file_size,buffer);

    if(fwrite(buffer,32,1,encInfo->fptr_stego_image) == 0)
        return e_failure;

    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char buffer[8];
    rewind(encInfo->fptr_secret);
    
    int ch;

    while((ch=fgetc(encInfo->fptr_secret)) != EOF){
        if(fread(buffer,8,1,encInfo->fptr_src_image) == 0)
            return e_failure;

        encode_byte_to_lsb(ch,buffer);

        if(fwrite(buffer,8,1,encInfo->fptr_stego_image) == 0)
            return e_failure;
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    int ch;

    while((ch=fgetc(fptr_src)) != EOF){
        if(fwrite(&ch,1,1,fptr_dest) == 0)
            return e_failure;
    }

    return e_success;
}