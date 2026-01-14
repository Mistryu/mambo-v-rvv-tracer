/*
  Simple RISC-V Vector Extension (RVV) Register-Register Instruction Test
  
  This is a simpler, more focused test with just a few instructions to help debug
  the vector tracer plugin. It includes:
  - A few different instruction types
  - Different vector registers
  - Masked and unmasked operations
  
  Compile with: riscv64-unknown-linux-gnu-gcc -march=rv64gcv -mabi=lp64d test_rvv_simple.S test_rvv_simple.c -o test_rvv_simple
*/

// 1 function where u test if the registers get corrupted 
// do addition and then only print once after the addition to check if we corrupt the program earlier

.section .text
.global test_vector_ops
.type test_vector_ops, @function

test_vector_ops:
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
    la t3, test_data3
    
    // Load data into v1, v2, v3 using vector load instructions
    // After: v1 = [1, 2, 3, 4, 5, 6, 7, 8]
    vle64.v v1, (t1)  // Load 8 64-bit values into v1
    
    // After: v2 = [16, 32, 48, 64, 80, 96, 112, 128] (0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80)
    vle64.v v2, (t2)  // Load 8 64-bit values into v2
    
    // After: v3 = [256, 512, 768, 1024, 1280, 1536, 1792, 2048] (0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800)
    vle64.v v3, (t3)  // Load 8 64-bit values into v3
    
    // Initialize mask register v0 (for masked operations)
    // v0 mask: 0xAA = 0b10101010
    // For SEW=64, the mask bit for element i is bit i of the least significant byte of v0[i]
    // Since v0[i] = 0xAA for all i, we use bits 0-7 of 0xAA:
    //   Bit 0 = 0 → Element 0 INACTIVE
    //   Bit 1 = 1 → Element 1 ACTIVE
    //   Bit 2 = 0 → Element 2 INACTIVE
    //   Bit 3 = 1 → Element 3 ACTIVE
    // So elements 1,3,5,7 are active (but VL=4, so only elements 1,3 are used)
    li t4, 0xAA
    // After: v0 = [0xAA, 0xAA, 0xAA, 0xAA, ...] (mask bits: active at indices 1,3,5,7)
    vmv.v.x v0, t4  // Broadcast scalar value to vector register v0
    
    // ============================================
    // Test 1: Unmasked vector-vector add (vm=1)
    // vadd.vv v10, v1, v2
    // After: v10 = [17, 34, 51, 68, 85, 102, 119, 136]
    //        (element-wise: [1+16, 2+32, 3+48, 4+64, 5+80, 6+96, 7+112, 8+128])
    // ============================================
    vadd.vv v10, v1, v2
    
    // ============================================
    // Test 2: Masked vector-vector add (vm=0)
    // vadd.vv v11, v1, v2, v0.t
    // After: v11 = [0, 34, 0, 68, 0, 102, 0, 136]
    //        (masked: active elements 1,3,5,7 get v1+v2; inactive elements 0,2,4,6 keep old value, assumed 0)
    //        Note: With VL=4, only elements 0-3 are processed, so result is [0, 34, 0, 68]
    // ============================================
    vadd.vv v11, v1, v2, v0.t
    
    // ============================================
    // Test 3: Unmasked vector-vector subtract
    // vsub.vv v12, v2, v1
    // After: v12 = [15, 30, 45, 60, 75, 90, 105, 120]
    //        (element-wise: [16-1, 32-2, 48-3, 64-4, 80-5, 96-6, 112-7, 128-8])
    // ============================================
    vsub.vv v12, v2, v1
    
    // ============================================
    // Test 4: Masked vector-vector subtract
    // vsub.vv v13, v2, v1, v0.t
    // After: v13 = [0, 30, 0, 60, 0, 90, 0, 120]
    //        (masked: active elements 1,3,5,7 get v2-v1; inactive elements 0,2,4,6 keep old value, assumed 0)
    //        Note: With VL=4, only elements 0-3 are processed, so result is [0, 30, 0, 60]
    // ============================================
    vsub.vv v13, v2, v1, v0.t
    
    // ============================================
    // Test 5: Unmasked vector-vector multiply
    // vmul.vv v14, v1, v3
    // After: v14 = [256, 1024, 2304, 4096, 6400, 9216, 12544, 16384]
    //        (element-wise: [1*256, 2*512, 3*768, 4*1024, 5*1280, 6*1536, 7*1792, 8*2048])
    // ============================================
    vmul.vv v14, v1, v3
    
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

test_data3:
    .quad 0x0000000000000100
    .quad 0x0000000000000200
    .quad 0x0000000000000300
    .quad 0x0000000000000400
    .quad 0x0000000000000500
    .quad 0x0000000000000600
    .quad 0x0000000000000700
    .quad 0x0000000000000800

