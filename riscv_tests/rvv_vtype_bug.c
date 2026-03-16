#include <stdio.h>
#include <stdlib.h>

extern void rvv_vtype_bug(void);

int main(void) {
    printf("Starting RISC-V Vector Extension VType Bug Test\n");
    
    rvv_vtype_bug();
    
    printf("\nTest completed successfully!\n");
    
    return 0;
}
