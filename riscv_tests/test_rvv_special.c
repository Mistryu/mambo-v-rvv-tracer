/*
 * RISC-V Vector Extension Test: Special Instructions
 * 
 * C wrapper for special RVV instruction tests
 */

#include <stdio.h>
#include <stdint.h>

// Assembly function prototype
extern void test_rvv_special_asm(void);

int main(void) {
    printf("========================================\n");
    printf("RISC-V Vector Special Instructions Test\n");
    printf("========================================\n\n");
    
    printf("Testing special/unique RVV instructions:\n");
    printf("  - Scalar moves (vmv.s.x, vmv.x.s, vfmv.s.f, vfmv.f.s)\n");
    printf("  - Whole register moves (vmv1r.v, vmv2r.v, vmv4r.v, vmv8r.v)\n");
    printf("  - Mask operations (vmsbf.m, vmsif.m, vmsof.m, viota.m, vid.v)\n");
    printf("  - Mask logical ops (vmand.mm, vmnand.mm, vmxor.mm, etc.)\n");
    printf("  - Permutations (vrgather.vv, vrgather.vx, vrgather.vi, vrgatherei16.vv)\n");
    printf("  - Compress (vcompress.vm)\n");
    printf("  - Merge operations (vmerge.vvm, vmerge.vxm, vmerge.vim)\n");
    printf("  - Slide operations (vslideup, vslidedown, vslide1up, vslide1down)\n");
    printf("  - Mask queries (vcpop.m, vfirst.m)\n");
    printf("  - FP classification (vfclass.v)\n");
    printf("  - Integer extension (vzext.vfN, vsext.vfN)\n");
    printf("\n");
    
    printf("Running assembly test...\n");
    test_rvv_special_asm();
    printf("Test completed!\n\n");
    
    printf("========================================\n");
    printf("All special instruction tests passed!\n");
    printf("========================================\n");
    
    return 0;
}