#include <stdio.h>
#include <stdlib.h>

extern void test_v0_ops(void);

int main(void) {
    printf("Starting RISC-V Vector Extension v0 Register Test\n");
    printf("This test exercises operations on the v0 mask register:\n");
    printf("  - Setting v0 as a mask using vmv.v.x\n");
    printf("  - Masked vector operations using v0.t\n");
    printf("  - Direct operations on v0 register\n");
    printf("  - Different mask patterns (0xAA, 0x55, 0xFF)\n\n");
    
    test_v0_ops();
    
    printf("\nTest completed successfully!\n");
    return 0;
}

