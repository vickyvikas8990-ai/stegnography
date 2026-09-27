#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc,char *argv[])
{
    if(argc<3)
    {
        printf("\n-------- SAMPLE INPUTS --------\n");
            printf("\n./a.out -e source_file.bmp secret_file.txt [output_file.bmp]\n");
            printf("./a.out -d source_file.bmp [output_file(.bmp .py .txt )]\n");
            printf("\n");
            return 0;
    }
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    
    
    if(check_operation_type(argv[1][1]) == e_encode){
        
        if(read_and_validate_encode_args(argv,&encInfo) == e_failure)
            return e_failure;
        else{
            if(do_encoding(&encInfo) == e_success)
                printf("Encoding is success\n");
        }
    }

    if(check_operation_type(argv[1][1]) == e_decode){
        if(read_and_validate_decode_args(argv,&decInfo) == e_failure)
            return e_failure;
        else{
            if(do_decoding(&decInfo) == e_success)
                printf("Decoding is success\n");
        }
    }

    return 0;
}


OperationType check_operation_type(char opt)
{
    if(opt == 'e')
        return e_encode;
    else if(opt == 'd')
        return e_decode;
    else{
    printf("ERROR : UNSUPPORTED");
        return e_unsupported;
    }
}