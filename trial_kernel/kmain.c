#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

// gcc -ffreestanding -m64 -nostdlib -Wall -c trial_kernel/kmain.c -o trial_kernel/build/kmain.o
// gcc trial_kernel/kmain.c -o trial_kernel/build/kmain
// ./trial_kernel/build/kmain

int main() {

    // For x86_64 / AMD64
    printf("sizeof(size_t): %zu bytes\n", sizeof(size_t));       // 8 byte
    printf("sizeof(int8_t): %zu bytes\n", sizeof(int8_t));       // 1 byte
    printf("sizeof(uint8_t): %zu bytes\n", sizeof(uint8_t));     // 1 byte
    printf("sizeof(int16_t): %zu bytes\n", sizeof(int16_t));     // 2 bytes
    printf("sizeof(uint16_t): %zu bytes\n", sizeof(uint16_t));   // 2 bytes
    printf("sizeof(int32_t): %zu bytes\n", sizeof(int32_t));     // 4 bytes
    printf("sizeof(uint32_t): %zu bytes\n", sizeof(uint32_t));   // 4 bytes
    printf("sizeof(int64_t): %zu bytes\n", sizeof(int64_t));     // 8 bytes
    printf("sizeof(uint64_t): %zu bytes\n", sizeof(uint64_t));   // 8 bytes
    printf("sizeof(intptr_t): %zu bytes\n", sizeof(intptr_t));   // 8 bytes
    printf("sizeof(uintptr_t): %zu bytes\n", sizeof(uintptr_t)); // 8 bytes
}