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
    vle64.v v1, (t1)
    
    // After: v2 = [16, 32, 48, 64, 80, 96, 112, 128] (0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80)
    vle64.v v2, (t2)
    
    // After: v3 = [256, 512, 768, 1024, 1280, 1536, 1792, 2048] (0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800)
    vle64.v v3, (t3)
    
    // Initialize mask register v0
    // v0 mask: 0xAA = 0b10101010
    // So elements 1,3,5,7 are active (but VL=4, so only elements 1,3 are used)
    li t4, 0xAA
    vmv.v.x v0, t4
    
    // After: v10 = [17, 34, 51, 68, 85, 102, 119, 136]
    vadd.vv v10, v1, v2
    
    // After: v11 = [0, 34, 0, 68, 0, 102, 0, 136]
    vadd.vv v11, v1, v2, v0.t
    
    // After: v12 = [15, 30, 45, 60, 75, 90, 105, 120]
    vsub.vv v12, v2, v1
    
    // After: v13 = [0, 30, 0, 60, 0, 90, 0, 120]
    vsub.vv v13, v2, v1, v0.t
    
    // After: v14 = [256, 1024, 2304, 4096, 6400, 9216, 12544, 16384]
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

