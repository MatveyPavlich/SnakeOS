// main.c
void print_string_pm(const char* str);

void kernel_main() {
    print_string_pm("Hello from C kernel!\0");

    // Halt forever
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
