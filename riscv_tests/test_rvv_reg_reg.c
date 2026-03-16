#include <stdio.h>
#include <stdlib.h>

extern void test_vector_ops(void);

int main(void) {
    printf("Starting RISC-V Vector Extension Register-Register Instruction Test\n");
    
    test_vector_ops();
    
    printf("\nTest completed successfully!\n");
    
    return 0;
}

