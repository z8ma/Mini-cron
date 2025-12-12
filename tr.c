

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

int main(){
    uint8_t buf[2] = { 0x23, 0x12 }; // reçu du réseau (big endian)
    uint16_t v = be16toh(*(uint16_t*)buf);
    printf("%#x\n", y);
        
    return 0;
}