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
    vle64.v v1, (t1)
    vle64.v v2, (t2)
    vle64.v v3, (t3)
    
    // Set v0 to alternating pattern: 10101010...
    li t4, 0xAA
    vmv.v.x v0, t4
    
    // Test 1: Unmasked vector-vector add (vm=1)
    vadd.vv v10, v1, v2
    
    // Test 2: Masked vector-vector add (vm=0)
    vadd.vv v11, v1, v2, v0.t
    
    // Test 3: Unmasked vector-vector subtract
    vsub.vv v12, v2, v1
    
    // Test 4: Masked vector-vector subtract
    vsub.vv v13, v2, v1, v0.t
    
    // Test 5: Unmasked vector-vector multiply
    vmul.vv v14, v1, v3
    
    // Test 6: Masked vector-vector multiply
    vmul.vv v15, v1, v3, v0.t
    
    // Test 7: Unmasked vector-vector AND
    vand.vv v16, v1, v2
    
    // Test 8: Masked vector-vector AND
    vand.vv v17, v1, v2, v0.t
    
    // Test 9: Unmasked vector-vector OR
    vor.vv v18, v1, v2
    
    // Test 10: Masked vector-vector OR
    vor.vv v19, v1, v2, v0.t
    
    // Test 11: Unmasked vector-vector XOR
    vxor.vv v20, v1, v2
    
    // Test 12: Masked vector-vector XOR
    vxor.vv v21, v1, v2, v0.t
    
    // Test 13: Unmasked vector-vector shift left
    vsll.vv v22, v1, v2
    
    // Test 14: Masked vector-vector shift left
    vsll.vv v23, v1, v2, v0.t
    
    // Test 15: Unmasked vector-vector shift right logical
    vsrl.vv v24, v1, v2
    
    // Test 16: Masked vector-vector shift right logical
    vsrl.vv v25, v1, v2, v0.t
    
    // Test 17: Unmasked vector-vector shift right arithmetic
    vsra.vv v26, v1, v2
    
    // Test 18: Masked vector-vector shift right arithmetic
    vsra.vv v27, v1, v2, v0.t
    
    // Test 19: Unmasked vector-vector min unsigned
    vminu.vv v28, v1, v2
    
    // Test 20: Masked vector-vector min unsigned
    vminu.vv v29, v1, v2, v0.t
    
    // Test 21: Unmasked vector-vector max unsigned
    vmaxu.vv v30, v1, v2
    
    // Test 22: Masked vector-vector max unsigned
    vmaxu.vv v31, v1, v2, v0.t
    
    // Test 23: Unmasked vector-vector compare equal
    vmseq.vv v4, v1, v2
    
    // Test 24: Masked vector-vector compare equal
    vmseq.vv v5, v1, v2, v0.t
    
    // Test 25: Unmasked vector-vector compare not equal
    vmsne.vv v6, v1, v2
    
    // Test 26: Masked vector-vector compare not equal
    vmsne.vv v7, v1, v2, v0.t
    
    // Test 27: Test with different register combinations
    vadd.vv v8, v10, v11
    
    // Test 28: Test with v0 as destination (should work)
    vadd.vv v0, v1, v2
    
    // Test 29: Chain operations
    vadd.vv v9, v8, v12
    
    // Restore registers
    ld s1, 232(sp)
    ld s0, 240(sp)
    ld ra, 248(sp)
    addi sp, sp, 256
    
    ret

# Random data:
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

