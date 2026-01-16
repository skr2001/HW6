#include "fixed_point.h"
#include <stdio.h>
#include <stdint.h>

void print_fixed(int16_t raw, int16_t q) {

    int16_t integer_part = raw >> q;
    
    int16_t mask = (1 << q) - 1;
    int16_t fraction = raw & mask;

    printf("%d.", integer_part);

    for (int i = 0; i < 6; i++) {
        int temp = fraction * 10;
        int digit = temp >> q;
        printf("%d", digit);
        fraction = temp & mask;
    }
}

int16_t add_fixed(int16_t a, int16_t b) {
    return a + b;
}

int16_t subtract_fixed(int16_t a, int16_t b) {
    return a - b;
}

int16_t multiply_fixed(int16_t a, int16_t b, int16_t q) {
    int64_t product = (int64_t)a * (int64_t)b;
    return (int16_t)(product >> q);
}

void eval_poly_ax2_minus_bx_plus_c_fixed(int16_t x, int16_t a, int16_t b, int16_t c, int16_t q) {
    int16_t x_squared = multiply_fixed(x, x, q);
    int16_t ax2 = multiply_fixed(a, x_squared, q);
    int16_t bx = multiply_fixed(b, x, q);
    int16_t term_diff = subtract_fixed(ax2, bx);
    int16_t y = add_fixed(term_diff, c);
    
    printf("the polynomial output for a=");
    print_fixed(a, q);
    printf(", b=");
    print_fixed(b, q);
    printf(", c=");
    print_fixed(c, q);
    printf(" is ");
    print_fixed(y, q);
    printf("\n");
}