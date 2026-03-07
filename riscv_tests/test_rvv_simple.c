#include <stdio.h>
#include <stdlib.h>

extern void test_vector_ops(void);

int main(void) {
    //printf("Starting Simple RISC-V Vector Extension Register-Register Instruction Test\n");
    //printf("This test exercises 5 RVV register-register instructions\n");
    //printf("including masked and unmasked operations with different register combinations.\n\n");
    
    test_vector_ops();
    
    //printf("\nTest completed successfully!\n");
    return 0;
}

