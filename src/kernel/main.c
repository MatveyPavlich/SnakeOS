// main.c
void print_string_64(const char* str);

void kernel_main() {
    print_string_64("Hello from C kernel!\0");

    // Halt forever
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
