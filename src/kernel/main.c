// main.c
/*
    With -m64 gcc flag, function arguments are as follows:
    (1) RDI (2) RSI (3) RDX (4) RCX (5) R8 (6) R9

*/

extern void print_64_bits(const char* str);

void main() {
    print_64_bits("Hello from C!\0");
    while (1) __asm__("hlt");
}
