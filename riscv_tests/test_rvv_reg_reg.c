/*
  C wrapper for RISC-V Vector Extension Register-Register Instruction Test
  
  This file provides a main function to call the assembly test function
  and ensures the test can be compiled and executed.
*/

#include <stdio.h>
#include <stdlib.h>

// Declare the assembly function
extern void test_vector_ops(void);

int main(void) {
    printf("Starting RISC-V Vector Extension Register-Register Instruction Test\n");
    printf("This test exercises various RVV register-register instructions (opcode 0x57)\n");
    printf("including masked and unmasked operations with different register combinations.\n\n");
    
    // Call the assembly test function
    test_vector_ops();
    
    printf("\nTest completed successfully!\n");
    printf("Check trace.json for captured register values.\n");
    
    return 0;
}

