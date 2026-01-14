#include <stdio.h>
#include <stdlib.h>

extern void test_v1_ops(void);

int main(void) {
    printf("Starting RISC-V Vector Extension v1 Register Test\n");
    printf("This test performs 8 different operations on the v1 register:\n");
    printf("  1. Vector-vector add (unmasked)\n");
    printf("  2. Vector-vector subtract\n");
    printf("  3. Vector-vector multiply\n");
    printf("  4. Vector-scalar add\n");
    printf("  5. Vector-scalar multiply\n");
    printf("  6. Masked vector add\n");
    printf("  7. Vector bitwise AND\n");
    printf("  8. Vector bitwise OR\n\n");
    
    test_v1_ops();
    
    printf("\nTest completed successfully!\n");
    return 0;
}

