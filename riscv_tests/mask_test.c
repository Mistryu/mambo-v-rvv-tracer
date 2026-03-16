#include <stdio.h>
#include <stdlib.h>

extern void mask_test(void);

int main(void) {
    printf("Starting RISC-V Vector Extension Mask Test Study Case\n");
    
    mask_test();
    
    printf("\nTest completed successfully!\n");
    
    return 0;
}

