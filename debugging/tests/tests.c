/* Division by zero */

void tests_div_by_zero(void)
{
        // Division by zero interrup check
        volatile int a = 1;
        volatile int b = 0;
        volatile int c;
        c = a / b;
}
