/*
  RISC-V Vector Extension (RVV) v0 Register Test
  
  This test focuses on operations involving the v0 register, which serves as
  the mask register in RVV. It tests:
  - Setting v0 as a mask using vmv.v.x
  - Masked vector operations using v0.t
  - Direct operations on v0 register
  
  Compile with: riscv64-unknown-linux-gnu-gcc -march=rv64gcv -mabi=lp64d test_rvv_v0.S test_rvv_v0.c -o test_rvv_v0
*/

.section .text
.global test_v0_ops
.type test_v0_ops, @function

test_v0_ops:
    // Save registers
    addi sp, sp, -256
    sd ra, 248(sp)
    sd s0, 240(sp)
    sd s1, 232(sp)
    
    // Set up vector length - configure for 8 elements (SEW=64, LMUL=1)
    li t0, 8
    vsetvli t0, t0, e64, m1, ta, ma
    
    // Load test data into vector registers from memory
    la t1, test_data1
    la t2, test_data2
    
    // Load data into v1, v2 using vector load instructions
    // After: v1 = [1, 2, 3, 4, 5, 6, 7, 8]
    vle64.v v1, (t1)
    // After: v2 = [16, 32, 48, 64, 80, 96, 112, 128]
    vle64.v v2, (t2)
    
    // ============================================
    // Test 1: Set v0 as mask using vmv.v.x
    // vmv.v.x v0, t4
    // After: v0 = [0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA]
    //        Mask pattern: 0xAA = 0b10101010 (elements 0,2,4,6 active)
    // ============================================
    li t4, 0xAA
    vmv.v.x v0, t4
    
    // ============================================
    // Test 2: Masked vector add using v0.t
    // vadd.vv v3, v1, v2, v0.t
    // After: v3 = [17, 0, 51, 0, 85, 0, 119, 0]
    //        (masked: active elements 0,2,4,6 get v1+v2; inactive keep old value, assumed 0)
    // ============================================
    vadd.vv v3, v1, v2, v0.t
    
    // ============================================
    // Test 3: Set v0 to different mask pattern
    // vmv.v.x v0, t5
    // After: v0 = [0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
    //        Mask pattern: 0x55 = 0b01010101 (elements 1,3,5,7 active)
    // ============================================
    li t5, 0x55
    vmv.v.x v0, t5
    
    // ============================================
    // Test 4: Masked vector subtract using new v0 mask
    // vsub.vv v4, v2, v1, v0.t
    // After: v4 = [0, 30, 0, 60, 0, 90, 0, 120]
    //        (masked: active elements 1,3,5,7 get v2-v1; inactive keep old value, assumed 0)
    // ============================================
    vsub.vv v4, v2, v1, v0.t
    
    // ============================================
    // Test 5: Direct operation on v0 - set all elements to 0xFF
    // vmv.v.x v0, t6
    // After: v0 = [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]
    //        Mask pattern: all elements active (0xFF = 0b11111111)
    // ============================================
    li t6, 0xFF
    vmv.v.x v0, t6
    
    // ============================================
    // Test 6: Unmasked operation (vm=1, doesn't use v0)
    // vadd.vv v5, v1, v2
    // After: v5 = [17, 34, 51, 68, 85, 102, 119, 136]
    //        (all elements computed, v0 mask is ignored)
    // ============================================
    vadd.vv v5, v1, v2
    
    // ============================================
    // Test 7: Masked operation with all-active mask
    // vmul.vv v6, v1, v2, v0.t
    // After: v6 = [16, 64, 144, 256, 400, 576, 784, 1024]
    //        (all elements active, so all get v1*v2)
    // ============================================
    vmul.vv v6, v1, v2, v0.t
    
    // Restore registers and return
    ld ra, 248(sp)
    ld s0, 240(sp)
    ld s1, 232(sp)
    addi sp, sp, 256
    ret

.section .data
.align 8
test_data1:
    .quad 0x0000000000000001
    .quad 0x0000000000000002
    .quad 0x0000000000000003
    .quad 0x0000000000000004
    .quad 0x0000000000000005
    .quad 0x0000000000000006
    .quad 0x0000000000000007
    .quad 0x0000000000000008

test_data2:
    .quad 0x0000000000000010
    .quad 0x0000000000000020
    .quad 0x0000000000000030
    .quad 0x0000000000000040
    .quad 0x0000000000000050
    .quad 0x0000000000000060
    .quad 0x0000000000000070
    .quad 0x0000000000000080

