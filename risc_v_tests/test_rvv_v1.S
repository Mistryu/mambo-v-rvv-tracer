/*
  RISC-V Vector Extension (RVV) v1 Register Test
  
  This test performs 8 different operations on the v1 register to exercise
  various vector instructions and observe how v1 changes after each operation.
  
  Compile with: riscv64-unknown-linux-gnu-gcc -march=rv64gcv -mabi=lp64d test_rvv_v1.S test_rvv_v1.c -o test_rvv_v1
*/

.section .text
.global test_v1_ops
.type test_v1_ops, @function

test_v1_ops:
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
    
    // Load initial data into v1, v2, v3
    // After: v1 = [1, 2, 3, 4, 5, 6, 7, 8]
    vle64.v v1, (t1)
    // After: v2 = [16, 32, 48, 64, 80, 96, 112, 128]
    vle64.v v2, (t2)
    // After: v3 = [256, 512, 768, 1024, 1280, 1536, 1792, 2048]
    vle64.v v3, (t3)
    
    // Initialize mask register v0
    li t4, 0xAA  // 0b10101010 - elements 0,2,4,6 active
    vmv.v.x v0, t4
    
    // ============================================
    // Operation 1: Vector-vector add (unmasked)
    // vadd.vv v1, v1, v2
    // After: v1 = [17, 34, 51, 68, 85, 102, 119, 136]
    //        (element-wise: v1[i] = v1[i] + v2[i])
    // ============================================
    vadd.vv v1, v1, v2
    
    // ============================================
    // Operation 2: Vector-vector subtract
    // vsub.vv v1, v1, v2
    // After: v1 = [1, 2, 3, 4, 5, 6, 7, 8]
    //        (element-wise: v1[i] = v1[i] - v2[i])
    // ============================================
    vsub.vv v1, v1, v2
    
    // ============================================
    // Operation 3: Vector-vector multiply
    // vmul.vv v1, v1, v3
    // After: v1 = [256, 1024, 2304, 4096, 6400, 9216, 12544, 16384]
    //        (element-wise: v1[i] = v1[i] * v3[i])
    // ============================================
    vmul.vv v1, v1, v3
    
    // ============================================
    // Operation 4: Vector-scalar add (broadcast scalar)
    // vadd.vx v1, v1, t5
    // After: v1 = [257, 1025, 2305, 4097, 6401, 9217, 12545, 16385]
    //        (element-wise: v1[i] = v1[i] + t5, where t5 = 1)
    // ============================================
    li t5, 1
    vadd.vx v1, v1, t5
    
    // ============================================
    // Operation 5: Vector-scalar multiply
    // vmul.vx v1, v1, t6
    // After: v1 = [514, 2050, 4610, 8194, 12802, 18434, 25090, 32770]
    //        (element-wise: v1[i] = v1[i] * t6, where t6 = 2)
    // ============================================
    li t6, 2
    vmul.vx v1, v1, t6
    
    // ============================================
    // Operation 6: Masked vector add (using v0 mask)
    // vadd.vv v1, v1, v2, v0.t
    // After: v1 = [530, 2050, 4658, 8194, 12882, 18434, 25206, 32770]
    //        (masked: only elements 0,2,4,6 get v1+v2; others unchanged)
    // ============================================
    vadd.vv v1, v1, v2, v0.t
    
    // ============================================
    // Operation 7: Vector and (bitwise AND)
    // vand.vv v1, v1, v2
    // After: v1 = [16, 0, 48, 0, 80, 0, 112, 0]
    //        (element-wise bitwise AND: v1[i] = v1[i] & v2[i])
    // ============================================
    vand.vv v1, v1, v2
    
    // ============================================
    // Operation 8: Vector or (bitwise OR)
    // vor.vv v1, v1, v3
    // After: v1 = [272, 512, 816, 1024, 1360, 1536, 1904, 2048]
    //        (element-wise bitwise OR: v1[i] = v1[i] | v3[i])
    // ============================================
    vor.vv v1, v1, v3
    
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

