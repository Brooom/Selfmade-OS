#include "formating.h"
#include "../allocator.h"

void format_string(const char * input, char *output, int max_output_size, va_list va_list)
{
    int idx_input = 0;
    int idx_output = 0;
    while(input[idx_input] != '\0' && idx_output<(max_output_size-1)) {
        if(input[idx_input] == '%'){
            idx_input++;
            //replace string variable
            if(input[idx_input]=='s'){
                int idx_arg = 0;
                const char *arg = va_arg(va_list, const char *);
                while(*arg && idx_output < (max_output_size-1)){
                    output[idx_output] = *arg++;
                    idx_output++;
                }
            }
            //replace single character
            else if(input[idx_input]=='c'){
                int arg = va_arg(va_list, int);
                if(arg && idx_output < (max_output_size-1)){
                    output[idx_output] = (char) arg;
                    idx_output++;
                }
            }
            //replace unsigned decimal integer
            else if(input[idx_input]=='u'){
                int idx_param = 0;
                char string_param[10];
                int_to_str(va_arg(va_list, int), string_param);
                while(string_param[idx_param] && idx_output < (max_output_size-1) && idx_param<10){
                    output[idx_output] = string_param[idx_param];
                    idx_output++;
                    idx_param++;
                }
            } 
            //repalce usigned hexadecimal integer
            else if(input[idx_input]=='x'){
                int idx_param = 0;
                char string_param[10];
                int param = va_arg(va_list, int);
                int_to_hex_string(&param, sizeof(param), string_param);
                while(string_param[idx_param] && idx_output < (max_output_size-1) && idx_param<10){
                    output[idx_output] = string_param[idx_param];
                    idx_output++;
                    idx_param++;
                }
            } 
            //replace pointer in hexadecimal
            else if(input[idx_input]=='p'){
                int idx_param = 0;
                char string_param[18];
                uint64_t param = (uint64_t) va_arg(va_list, void *);
                int_to_hex_string(&param, sizeof(param), string_param);
                while(string_param[idx_param] && idx_output < (max_output_size-1) && idx_param<18){
                    output[idx_output] = string_param[idx_param];
                    idx_output++;
                    idx_param++;
                }
            } 
            //draw a % sign
            else if(input[idx_input]=='%'){
                if(idx_output < (max_output_size-1)){
                    output[idx_output] = '%';
                    idx_output++;
                }
            } 
            else{

            }
        }
        else{
            output[idx_output] = input[idx_input];
            idx_output++;
        }
        idx_input++;
        output[idx_output] = '\0';
    }
}

void int_to_hex_string(void *p, size_t size, char *buffer){
    size *= 8;
    if(size==8){
        u8_to_hex(*(uint8_t*)p, buffer);
    }
    else if(size==16){
        u16_to_hex(*(uint16_t*)p, buffer);
    }
    else if(size==32){
        u32_to_hex(*(uint32_t*)p, buffer);
    }
    else if(size==64){
        u64_to_hex(*(uint64_t*)p, buffer);
    }
}



void u64_to_hex(uint64_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 16; ++i) {
        int shift = 60 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[18] = '\0';
}

void u32_to_hex(uint32_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 8; ++i) {
        int shift = 28 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[10] = '\0';
}

void u16_to_hex(uint16_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 4; ++i) {
        int shift = 12 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[6] = '\0';
}

void u8_to_hex(uint8_t value, char *buf) {
    const char hex[] = "0123456789abcdef";

    buf[0] = '0';
    buf[1] = 'x';
    for (int i = 0; i < 2; ++i) {
        int shift = 4 - (i * 4);
        buf[2 + i] = hex[(value >> shift) & 0xF];
    }
    buf[4] = '\0';
}

void int_to_str(int value, char *buf) {
    char tmp[10];
    int len = 0;

    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (value > 0) {          // collect digits in reverse
        tmp[len++] = '0' + (value % 10);
        value /= 10;
    }

    for (int i = 0; i < len; ++i) // reverse into output buffer
        buf[i] = tmp[len - 1 - i];
    buf[len] = '\0';
}