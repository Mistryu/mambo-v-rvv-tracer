/*
  Comprehensive RISC-V Vector Extension (RVV) Register-Register Instruction Test
  
  This test file exercises various register-register RVV instructions (opcode 0x57)
  to test the vector tracer plugin. It includes:
  - Different instruction types (add, sub, mul, and, or, xor, etc.)
  - Different vector registers (v0-v31)
  - Masked and unmasked operations (vm=0 and vm=1)
  - Different source and destination register combinations
  
  Compile with: riscv64-unknown-linux-gnu-gcc -march=rv64gcv -mabi=lp64d test_rvv_reg_reg.S test_rvv_reg_reg.c -o test_rvv_reg_reg
*/

.section .text
.global test_vector_ops
.type test_vector_ops, @function

// Vector register-register instructions have opcode 0x57
// Format: aaaaaabc ccccdddd deeeffff f1010111
// where: funct6=aaaaaa, vm=b, vs2=ccccc, vs1=ddddd, funct3=eee, vd=fffff

// Common RVV register-register instruction encodings:
// vadd.vv:  funct6=000000, funct3=000 (vector-vector add)
// vsub.vv:  funct6=000010, funct3=000 (vector-vector subtract)
// vmul.vv:  funct6=100101, funct3=010 (vector-vector multiply)
// vand.vv:  funct6=001001, funct3=000 (vector-vector and)
// vor.vv:   funct6=001010, funct3=000 (vector-vector or)
// vxor.vv:  funct6=001011, funct3=000 (vector-vector xor)
// vsll.vv:  funct6=100101, funct3=001 (vector-vector shift left)
// vsrl.vv:  funct6=101000, funct3=101 (vector-vector shift right logical)
// vsra.vv:  funct6=101001, funct3=101 (vector-vector shift right arithmetic)
// vminu.vv: funct6=000100, funct3=000 (vector-vector min unsigned)
// vmaxu.vv: funct6=000110, funct3=000 (vector-vector max unsigned)
// vmseq.vv: funct6=011000, funct3=000 (vector-vector compare equal)
// vmsne.vv: funct6=011001, funct3=000 (vector-vector compare not equal)

test_vector_ops:
    // Save registers
    addi sp, sp, -256
    sd ra, 248(sp)
    sd s0, 240(sp)
    sd s1, 232(sp)
    
    // Set up vector length - configure for 8 elements (SEW=64, LMUL=1)
    // vsetvli rd, rs1, vtype - set vector length
    li t0, 8
    vsetvli t0, t0, e64, m1, ta, ma
    
    // Load test data into vector registers from memory
    la t1, test_data1
    la t2, test_data2
    la t3, test_data3
    
    // Load data into v1, v2, v3 using vector load instructions
    vle64.v v1, (t1)  // Load 8 64-bit values into v1
    vle64.v v2, (t2)  // Load 8 64-bit values into v2
    vle64.v v3, (t3)  // Load 8 64-bit values into v3
    
    // Initialize mask register v0 (for masked operations)
    // Set v0 to alternating pattern: 10101010...
    li t4, 0xAA
    vmv.v.x v0, t4  // Broadcast scalar value to vector register v0
    
    // ============================================
    // Test 1: Unmasked vector-vector add (vm=1)
    // vadd.vv v10, v1, v2  (vd=10, vs1=1, vs2=2, vm=1, funct3=000, funct6=000000)
    // ============================================
    vadd.vv v10, v1, v2
    
    // ============================================
    // Test 2: Masked vector-vector add (vm=0)
    // vadd.vv v11, v1, v2, v0.t  (vd=11, vs1=1, vs2=2, vm=0, funct3=000, funct6=000000)
    // ============================================
    vadd.vv v11, v1, v2, v0.t
    
    // ============================================
    // Test 3: Unmasked vector-vector subtract
    // vsub.vv v12, v2, v1  (vd=12, vs1=2, vs2=1, vm=1, funct3=000, funct6=000010)
    // ============================================
    vsub.vv v12, v2, v1
    
    // ============================================
    // Test 4: Masked vector-vector subtract
    // vsub.vv v13, v2, v1, v0.t  (vd=13, vs1=2, vs2=1, vm=0, funct3=000, funct6=000010)
    // ============================================
    vsub.vv v13, v2, v1, v0.t
    
    // ============================================
    // Test 5: Unmasked vector-vector multiply
    // vmul.vv v14, v1, v3  (vd=14, vs1=1, vs2=3, vm=1, funct3=010, funct6=100101)
    // ============================================
    vmul.vv v14, v1, v3
    
    // ============================================
    // Test 6: Masked vector-vector multiply
    // vmul.vv v15, v1, v3, v0.t  (vd=15, vs1=1, vs2=3, vm=0, funct3=010, funct6=100101)
    // ============================================
    vmul.vv v15, v1, v3, v0.t
    
    // ============================================
    // Test 7: Unmasked vector-vector AND
    // vand.vv v16, v1, v2  (vd=16, vs1=1, vs2=2, vm=1, funct3=000, funct6=001001)
    // ============================================
    vand.vv v16, v1, v2
    
    // ============================================
    // Test 8: Masked vector-vector AND
    // vand.vv v17, v1, v2, v0.t  (vd=17, vs1=1, vs2=2, vm=0, funct3=000, funct6=001001)
    // ============================================
    vand.vv v17, v1, v2, v0.t
    
    // ============================================
    // Test 9: Unmasked vector-vector OR
    // vor.vv v18, v1, v2  (vd=18, vs1=1, vs2=2, vm=1, funct3=000, funct6=001010)
    // ============================================
    vor.vv v18, v1, v2
    
    // ============================================
    // Test 10: Masked vector-vector OR
    // vor.vv v19, v1, v2, v0.t  (vd=19, vs1=1, vs2=2, vm=0, funct3=000, funct6=001010)
    // ============================================
    vor.vv v19, v1, v2, v0.t
    
    // ============================================
    // Test 11: Unmasked vector-vector XOR
    // vxor.vv v20, v1, v2  (vd=20, vs1=1, vs2=2, vm=1, funct3=000, funct6=001011)
    // ============================================
    vxor.vv v20, v1, v2
    
    // ============================================
    // Test 12: Masked vector-vector XOR
    // vxor.vv v21, v1, v2, v0.t  (vd=21, vs1=1, vs2=2, vm=0, funct3=000, funct6=001011)
    // ============================================
    vxor.vv v21, v1, v2, v0.t
    
    // ============================================
    // Test 13: Unmasked vector-vector shift left
    // vsll.vv v22, v1, v2  (vd=22, vs1=1, vs2=2, vm=1, funct3=001, funct6=100101)
    // ============================================
    vsll.vv v22, v1, v2
    
    // ============================================
    // Test 14: Masked vector-vector shift left
    // vsll.vv v23, v1, v2, v0.t  (vd=23, vs1=1, vs2=2, vm=0, funct3=001, funct6=100101)
    // ============================================
    vsll.vv v23, v1, v2, v0.t
    
    // ============================================
    // Test 15: Unmasked vector-vector shift right logical
    // vsrl.vv v24, v1, v2  (vd=24, vs1=1, vs2=2, vm=1, funct3=101, funct6=101000)
    // ============================================
    vsrl.vv v24, v1, v2
    
    // ============================================
    // Test 16: Masked vector-vector shift right logical
    // vsrl.vv v25, v1, v2, v0.t  (vd=25, vs1=1, vs2=2, vm=0, funct3=101, funct6=101000)
    // ============================================
    vsrl.vv v25, v1, v2, v0.t
    
    // ============================================
    // Test 17: Unmasked vector-vector shift right arithmetic
    // vsra.vv v26, v1, v2  (vd=26, vs1=1, vs2=2, vm=1, funct3=101, funct6=101001)
    // ============================================
    vsra.vv v26, v1, v2
    
    // ============================================
    // Test 18: Masked vector-vector shift right arithmetic
    // vsra.vv v27, v1, v2, v0.t  (vd=27, vs1=1, vs2=2, vm=0, funct3=101, funct6=101001)
    // ============================================
    vsra.vv v27, v1, v2, v0.t
    
    // ============================================
    // Test 19: Unmasked vector-vector min unsigned
    // vminu.vv v28, v1, v2  (vd=28, vs1=1, vs2=2, vm=1, funct3=000, funct6=000100)
    // ============================================
    vminu.vv v28, v1, v2
    
    // ============================================
    // Test 20: Masked vector-vector min unsigned
    // vminu.vv v29, v1, v2, v0.t  (vd=29, vs1=1, vs2=2, vm=0, funct3=000, funct6=000100)
    // ============================================
    vminu.vv v29, v1, v2, v0.t
    
    // ============================================
    // Test 21: Unmasked vector-vector max unsigned
    // vmaxu.vv v30, v1, v2  (vd=30, vs1=1, vs2=2, vm=1, funct3=000, funct6=000110)
    // ============================================
    vmaxu.vv v30, v1, v2
    
    // ============================================
    // Test 22: Masked vector-vector max unsigned
    // vmaxu.vv v31, v1, v2, v0.t  (vd=31, vs1=1, vs2=2, vm=0, funct3=000, funct6=000110)
    // ============================================
    vmaxu.vv v31, v1, v2, v0.t
    
    // ============================================
    // Test 23: Unmasked vector-vector compare equal
    // vmseq.vv v4, v1, v2  (vd=4, vs1=1, vs2=2, vm=1, funct3=000, funct6=011000)
    // ============================================
    vmseq.vv v4, v1, v2
    
    // ============================================
    // Test 24: Masked vector-vector compare equal
    // vmseq.vv v5, v1, v2, v0.t  (vd=5, vs1=1, vs2=2, vm=0, funct3=000, funct6=011000)
    // ============================================
    vmseq.vv v5, v1, v2, v0.t
    
    // ============================================
    // Test 25: Unmasked vector-vector compare not equal
    // vmsne.vv v6, v1, v2  (vd=6, vs1=1, vs2=2, vm=1, funct3=000, funct6=011001)
    // ============================================
    vmsne.vv v6, v1, v2
    
    // ============================================
    // Test 26: Masked vector-vector compare not equal
    // vmsne.vv v7, v1, v2, v0.t  (vd=7, vs1=1, vs2=2, vm=0, funct3=000, funct6=011001)
    // ============================================
    vmsne.vv v7, v1, v2, v0.t
    
    // ============================================
    // Test 27: Test with different register combinations
    // vadd.vv v8, v10, v11  (vd=8, vs1=10, vs2=11, vm=1)
    // ============================================
    vadd.vv v8, v10, v11
    
    // ============================================
    // Test 28: Test with v0 as destination (should work)
    // vadd.vv v0, v1, v2  (vd=0, vs1=1, vs2=2, vm=1)
    // Note: v0 is also the mask register, but can be used as destination
    // ============================================
    vadd.vv v0, v1, v2
    
    // ============================================
    // Test 29: Chain operations
    // vadd.vv v9, v8, v12  (vd=9, vs1=8, vs2=12, vm=1)
    // ============================================
    vadd.vv v9, v8, v12
    
    // Restore registers
    ld s1, 232(sp)
    ld s0, 240(sp)
    ld ra, 248(sp)
    addi sp, sp, 256
    
    ret

.section .data
.align 8
test_data1:
    .quad 0x0123456789ABCDEF
    .quad 0xFEDCBA9876543210
    .quad 0x1111111111111111
    .quad 0x2222222222222222
    .quad 0x3333333333333333
    .quad 0x4444444444444444
    .quad 0x5555555555555555
    .quad 0x6666666666666666

test_data2:
    .quad 0xAAAAAAAAAAAAAAAA
    .quad 0x5555555555555555
    .quad 0xFFFFFFFFFFFFFFFF
    .quad 0x0000000000000000
    .quad 0x123456789ABCDEF0
    .quad 0xFEDCBA9876543210
    .quad 0x1111111111111111
    .quad 0x2222222222222222

test_data3:
    .quad 0x3333333333333333
    .quad 0xCCCCCCCCCCCCCCCC
    .quad 0x0000FFFF0000FFFF
    .quad 0xFFFF0000FFFF0000
    .quad 0xAAAAAAAAAAAAAAAA
    .quad 0x5555555555555555
    .quad 0x8888888888888888
    .quad 0x9999999999999999

