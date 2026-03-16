#include <stdio.h>
#include <stdlib.h>

extern void ternary_incorrect(void);

int main(void) {
    printf("Starting RISC-V Vector Extension Ternary Operation Test\n");
    
    ternary_incorrect();
    
    printf("\nTest completed successfully!\n");
    
    return 0;
}
