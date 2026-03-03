/*
 * RISC-V Vector Extension Test: Special and Unique Instructions
 * 
 * Tests all the weird/uncommon vector instructions:
 * - Scalar move instructions (vmv.s.x, vmv.x.s, vfmv.s.f, vfmv.f.s)
 * - Whole register moves (vmv1r.v, vmv2r.v, vmv4r.v, vmv8r.v)
 * - Mask operations (vmsbf.m, vmsif.m, vmsof.m, viota.m, vid.v)
 * - Mask logical operations (vmand.mm, vmnand.mm, vmxor.mm, etc.)
 * - Compress/permutation (vcompress.vm, vrgather.vv, vrgather.vx, vrgather.vi)
 * - Merge operations (vmerge.vvm, vmerge.vxm, vmerge.vim)
 * - Slide operations (vslideup, vslidedown, vslide1up, vslide1down)
 * - Mask bit operations (vcpop.m, vfirst.m)
 * - Floating-point classification (vfclass.v)
 * - Integer extension (vzext.vf2, vsext.vf2, etc.)
 */

.text
.globl test_rvv_special_asm
.type test_rvv_special_asm, @function

test_rvv_special_asm:
    // Save return address
    addi sp, sp, -16
    sd ra, 0(sp)
    
    // ========================================
    // TEST 1: Scalar Move Instructions
    // ========================================
    
    // Setup: e32, m1
    li t0, 16
    vsetvli t1, t0, e32, m1, ta, ma
    
    // Initialize v8 with data
    li t2, 0x12345678
    vmv.s.x v8, t2          // Move scalar to v8[0]
    
    // Extract scalar from v8[0]
    vmv.x.s t3, v8          // Move v8[0] to t3
    
    // Floating-point scalar moves (requires FP data)
    li t2, 0x3f800000       // 1.0 in IEEE 754
    vmv.s.x v9, t2
    vfmv.f.s f10, v9        // Move v9[0] to f10
    
    li t2, 0x40000000       // 2.0 in IEEE 754
    fmv.w.x f11, t2
    vfmv.s.f v10, f11       // Move f11 to v10[0]
    
    // ========================================
    // TEST 2: Whole Register Moves
    // ========================================
    
    // Setup test data in v0-v7
    vsetvli t1, t0, e32, m2, ta, ma
    vid.v v0                // v0[i] = i
    vadd.vi v2, v0, 10      // v2[i] = i + 10
    
    // For values > 15, use scalar register
    li t2, 20
    vadd.vx v4, v0, t2      // v4[i] = i + 20
    
    li t2, 30
    vadd.vx v6, v0, t2      // v6[i] = i + 30
    
    // Whole register moves (different sizes)
    vmv1r.v v8, v0          // Move 1 register: v8 = v0
    vmv2r.v v10, v2         // Move 2 registers: v10-v11 = v2-v3
    vmv4r.v v12, v4         // Move 4 registers: v12-v15 = v4-v7
    vmv8r.v v16, v8         // Move 8 registers: v16-v23 = v8-v15
    
    // ========================================
    // TEST 3: Mask Set Operations
    // ========================================
    
    vsetvli t1, t0, e8, m1, ta, ma
    
    // Create a test mask pattern in v0
    li t2, 0x0F0F0F0F
    vmv.s.x v1, t2
    vand.vi v0, v1, 1       // Create mask: 0b01010101...
    
    // Mask set before first (set mask bits before first set bit)
    vmsbf.m v2, v0
    
    // Mask set including first (set mask bits up to and including first set bit)
    vmsif.m v3, v0
    
    // Mask set only first (set only the first set bit)
    vmsof.m v4, v0
    
    // ========================================
    // TEST 4: Mask Logical Operations
    // ========================================
    
    // Setup two masks
    li t2, 0xAAAAAAAA
    vmv.s.x v5, t2
    vand.vi v5, v5, 1       // Mask A: 0b10101010...
    
    li t2, 0xCCCCCCCC
    vmv.s.x v6, t2
    vand.vi v6, v6, 1       // Mask B: 0b11001100...
    
    // Mask logical operations (all variants)
    vmand.mm v7, v5, v6     // AND
    vmnand.mm v8, v5, v6    // NAND
    vmandn.mm v9, v5, v6    // AND-NOT
    vmxor.mm v10, v5, v6    // XOR
    vmor.mm v11, v5, v6     // OR
    vmnor.mm v12, v5, v6    // NOR
    vmorn.mm v13, v5, v6    // OR-NOT
    vmxnor.mm v14, v5, v6   // XNOR
    
    // ========================================
    // TEST 5: Index and Element Operations
    // ========================================
    
    vsetvli t1, t0, e32, m1, ta, ma
    
    // Element index (vid.v) - already used above
    vid.v v15               // v15[i] = i
    
    // Index of first set bit in mask
    vmseq.vi v0, v15, 5     // Create mask where v15[i] == 5
    vfirst.m t2, v0         // Find first set bit
    
    // Population count (number of set bits)
    vcpop.m t3, v0          // Count set bits in mask
    
    // Integer one-extend (iota - running sum of mask bits)
    viota.m v16, v0         // v16[i] = count of set bits before position i
    
    // ========================================
    // TEST 6: Permutation Operations
    // ========================================
    
    // Setup source and index vectors
    vsetvli t1, t0, e32, m1, ta, ma
    vid.v v17               // v17[i] = i
    
    // Create index pattern: gather every other element
    li t2, 2
    vmul.vx v18, v17, t2    // v18[i] = i * 2
    
    // Gather operations
    vrgather.vv v19, v17, v18  // Gather with vector indices
    
    li t2, 3
    vrgather.vx v20, v17, t2  // Gather with scalar index (broadcast v17[3])
    
    vrgather.vi v21, v17, 5   // Gather with immediate index (broadcast v17[5])
    
    // Gather with 16-bit indices (for larger vectors)
    vsetvli t1, t0, e16, m1, ta, ma
    vid.v v22
    vsetvli t1, t0, e32, m1, ta, ma
    vrgatherei16.vv v23, v17, v22  // Gather using 16-bit indices
    
    // ========================================
    // TEST 7: Compress Operation
    // ========================================
    
    vsetvli t1, t0, e32, m1, ta, ma
    vid.v v24               // v24[i] = i
    
    // Create mask: select even indices
    vand.vi v0, v24, 1
    vmseq.vi v0, v0, 0      // Mask = (i % 2 == 0)
    
    // Compress: pack selected elements
    vcompress.vm v25, v24, v0  // v25 contains only even-indexed elements
    
    // ========================================
    // TEST 8: Merge Operations
    // ========================================
    
    vsetvli t1, t0, e32, m1, ta, ma
    
    // Setup source vectors
    li t2, 0xAAAAAAAA
    vmv.v.x v26, t2         // v26 = 0xAAAAAAAA
    
    li t2, 0x55555555
    vmv.v.x v27, t2         // v27 = 0x55555555
    
    // Create alternating mask
    vid.v v28
    vand.vi v0, v28, 1      // Mask alternates 0, 1, 0, 1...
    
    // Merge with vector source
    vmerge.vvm v29, v26, v27, v0  // Select from v27 where mask=1, else v26
    
    // Merge with scalar source
    li t2, 0xFFFFFFFF
    vmerge.vxm v30, v26, t2, v0   // Select scalar where mask=1, else v26
    
    // Merge with immediate source
    vmerge.vim v31, v26, 15, v0   // Select immediate 15 where mask=1, else v26
    
    // ========================================
    // TEST 9: Slide Operations
    // ========================================
    
    vsetvli t1, t0, e32, m1, ta, ma
    vid.v v0                // v0[i] = i
    
    // Slide up by 3 positions (insert zeros at bottom)
    li t2, 3
    vslideup.vx v1, v0, t2
    
    // Slide down by 2 positions (discard bottom elements)
    li t2, 2
    vslidedown.vx v2, v0, t2
    
    // Slide up with immediate
    vslideup.vi v3, v0, 4
    
    // Slide down with immediate
    vslidedown.vi v4, v0, 3
    
    // Slide1up: shift up and insert scalar at bottom
    li t2, 0x99999999
    vslide1up.vx v5, v0, t2
    
    // Slide1down: shift down and insert scalar at top
    li t2, 0x88888888
    vslide1down.vx v6, v0, t2
    
    // Floating-point slide1
    li t2, 0x3f800000       // 1.0
    fmv.w.x f12, t2
    vfslide1up.vf v7, v0, f12
    vfslide1down.vf v8, v0, f12
    
    // ========================================
    // TEST 10: Floating-Point Classification
    // ========================================
    
    vsetvli t1, t0, e32, m1, ta, ma
    
    // Create FP test values
    li t2, 0x7F800000       // +Infinity
    vmv.s.x v9, t2
    
    li t2, 0xFF800000       // -Infinity
    vmv.s.x v10, t2
    
    li t2, 0x7FC00000       // NaN
    vmv.s.x v11, t2
    
    li t2, 0x00000000       // +0
    vmv.s.x v12, t2
    
    li t2, 0x80000000       // -0
    vmv.s.x v13, t2
    
    // Classify floating-point values
    vfclass.v v14, v9       // Classify +Inf
    vfclass.v v15, v10      // Classify -Inf
    vfclass.v v16, v11      // Classify NaN
    vfclass.v v17, v12      // Classify +0
    vfclass.v v18, v13      // Classify -0
    
    // ========================================
    // TEST 11: Integer Extension Operations
    // ========================================
    
    // First, create e8 source data
    vsetvli t1, t0, e8, m1, ta, ma
    vid.v v1                // v1[i] = i (8-bit)
    
    // Now set vtype for e16 destination (SEW=16)
    vsetvli t1, t0, e16, m2, ta, ma
    
    // Zero-extend from e8 to e16 (SEW/2 = 16/2 = 8)
    vzext.vf2 v4, v1        // Reads v1 as e8, writes v4-v5 as e16
    
    // Sign-extend from e8 to e16
    vsext.vf2 v6, v1        // Reads v1 as e8, writes v6-v7 as e16
    
    // For e8 to e32 extension
    // First recreate e8 source (v1 was interpreted as e16 view above)
    vsetvli t1, t0, e8, m1, ta, ma
    vid.v v1                // Re-initialize v1 as e8
    
    // Set vtype for e32 destination
    vsetvli t1, t0, e32, m4, ta, ma
    
    // Zero-extend from e8 to e32 (SEW/4 = 32/4 = 8)
    vzext.vf4 v8, v1        // Reads v1 as e8, writes v8-v11 as e32
    
    // Sign-extend from e8 to e32
    vsetvli t1, t0, e8, m1, ta, ma
    vid.v v1                // Re-initialize v1 as e8
    vsetvli t1, t0, e32, m4, ta, ma
    vsext.vf4 v12, v1       // Reads v1 as e8, writes v12-v15 as e32
    
    // For e8 to e64 extension
    vsetvli t1, t0, e8, m1, ta, ma
    vid.v v1                // Re-initialize v1 as e8
    
    // Set vtype for e64 destination
    vsetvli t1, t0, e64, m8, ta, ma
    
    // Zero-extend from e8 to e64 (SEW/8 = 64/8 = 8)
    vzext.vf8 v16, v1       // Reads v1 as e8, writes v16-v23 as e64
    
    // Sign-extend from e8 to e64
    vsetvli t1, t0, e8, m1, ta, ma
    vid.v v1                // Re-initialize v1 as e8
    vsetvli t1, t0, e64, m8, ta, ma
    vsext.vf8 v24, v1       // Reads v1 as e8, writes v24-v31 as e64
    
    // ========================================
    // TEST 12: Mask Population/First for Different LMUL
    // ========================================
    
    // Test vcpop.m with different LMUL
    vsetvli t1, t0, e32, m1, ta, ma
    vid.v v2
    vmslt.vi v0, v2, 8      // Create mask
    vcpop.m t2, v0          // Should be 8
    
    vsetvli t1, t0, e32, m2, ta, ma
    vid.v v2
    li t2, 16
    vmslt.vx v0, v2, t2     // Create mask (16 > 15, need scalar)
    vcpop.m t3, v0          // Should be 16
    
    // Test vfirst.m
    vsetvli t1, t0, e32, m1, ta, ma
    vid.v v3
    vmseq.vi v0, v3, 7      // Mask has first bit at position 7
    vfirst.m t4, v0         // Should return 7
    
 // ========================================
    // TEST 13: Widening Integer Extension
    // ========================================
    
    vsetvli t1, t0, e16, m1, ta, ma
    vid.v v2                // 16-bit values
    
    // Widening zero-extend to 32-bit (1 source register)
    vsetvli t1, t0, e32, m2, ta, ma
    vzext.vf2 v4, v2        // Source: v2 (1 reg), Dest: v4-v5 (2 regs)
    
    // Widening sign-extend to 32-bit (1 source register)
    vsext.vf2 v6, v2        // Source: v2 (1 reg), Dest: v6-v7 (2 regs)
    
    // ========================================
    // TEST 14: Extension with 2 Source Registers
    // ========================================
    
    // vzext.vf2: 2 source registers
    vsetvli t1, t0, e16, m2, ta, ma  // Use symbolic form (assembler will encode)
    vid.v v8                         // Create e16 data: v8-v9 (2 regs)

    vsetvli t1, t0, e32, m4, ta, ma  // Use symbolic form
    vzext.vf2 v12, v8                // Source: v8-v9 (2 regs), Dest: v12-v15 (4 regs)

    // vsext.vf2: 2 source registers
    vsetvli t1, t0, e16, m2, ta, ma
    vid.v v10                        // Create e16 data: v10-v11 (2 regs)

    vsetvli t1, t0, e32, m4, ta, ma
    vsext.vf2 v16, v10               // Source: v10-v11 (2 regs), Dest: v16-v19 (4 regs)

    // vzext.vf4: 2 source registers
    vsetvli t1, t0, e16, m2, ta, ma
    vid.v v20                        // Create e16 data: v20-v21 (2 regs)

    vsetvli t1, t0, e64, m8, ta, ma
    vzext.vf4 v24, v20               // Source: v20-v21 (2 regs), Dest: v24-v31 (8 regs)

    // vsext.vf4: 2 source registers
    vsetvli t1, t0, e16, m2, ta, ma
    vid.v v8                         // Reuse v8-v9 (2 regs)

    vsetvli t1, t0, e64, m8, ta, ma
    vsext.vf4 v16, v8                // Source: v8-v9 (2 regs), Dest: v16-v23 (8 regs)
    
    // ========================================
    // Done
    // ========================================
    
    // Restore and return
    ld ra, 0(sp)
    addi sp, sp, 16
    ret

.size test_rvv_special_asm, .-test_rvv_special_asm