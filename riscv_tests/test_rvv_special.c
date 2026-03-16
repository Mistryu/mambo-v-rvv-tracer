#include <stdio.h>
#include <stdint.h>

extern void test_rvv_special_asm(void);

int main(void) {
    printf("RISC-V Vector Special Instructions Test\n");
    
    test_rvv_special_asm();
    printf("Test completed!\n\n");
    
    return 0;
}