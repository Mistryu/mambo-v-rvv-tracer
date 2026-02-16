#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "plugins.h"

#include <stdint.h>
#include <sys/types.h>


#ifndef CSR_VSTART
#define CSR_VSTART 0x008u
#endif
#ifndef CSR_VCSR
#define CSR_VCSR   0x00Fu
#endif
#ifndef CSR_VL
#define CSR_VL     0xC20u
#endif
#ifndef CSR_VTYPE
#define CSR_VTYPE  0xC21u
#endif
#ifndef CSR_VLENB
#define CSR_VLENB  0xC22u
#endif

#ifndef X0
#define X0 0u
#endif

#ifndef RVV_SUMOP_WHOLE
#define RVV_SUMOP_WHOLE 0x08u
#endif

// Buffer layout for g_saved_rvv
#define RVV_VREG_BYTES      256u      // Max bytes per vector register
#define RVV_NUM_VREGS       32u        // Total vector registers
#define RVV_VREG_SAVE_BYTES (RVV_NUM_VREGS * RVV_VREG_BYTES)  // 8192 bytes
#define RVV_HDR_BYTES       64u        // Header space for alignment (PC + CSRs)
#define RVV_LS_HDR_BYTES    64u        // Header space for load/store (includes rs1)
#define RVV_CSR_HDR_BYTES   80u        // Header space for CSR (includes rd, rs1, rs2)
#define RVV_SAVE_BYTES      (RVV_CSR_HDR_BYTES + RVV_VREG_SAVE_BYTES)  // 8272 bytes total

__attribute__((aligned(64)))
static uint8_t g_saved_rvv[RVV_SAVE_BYTES];

// Buffer type identifiers
#define RVV_TYPE_REG_REG    1u  // Register-register
#define RVV_TYPE_CSR         2u  // CSR instructions
#define RVV_TYPE_LOAD_STORE  3u  // Load/store 

// Buffer offsets - Type 1 Register-register
#define OFF_TYPE     0
#define OFF_PC       8
#define OFF_VL       16
#define OFF_VTYPE    24
#define OFF_VSTART   32
#define OFF_VCSR     40
#define OFF_VLENB    48
#define OFF_REG_RS1  56
#define OFF_VREGS    64

// Buffer offsets - Type 2 Load/store
#define OFF_LS_TYPE     0
#define OFF_LS_PC       8
#define OFF_LS_VL       16
#define OFF_LS_VTYPE    24
#define OFF_LS_VSTART   32
#define OFF_LS_VCSR     40
#define OFF_LS_VLENB    48
#define OFF_LS_RS1      56  // rs1 scalar register value (64-bit)
#define OFF_LS_RS2      64  // rs2 scalar register value (64-bit, for segmented load/store)
#define OFF_LS_VREGS    72  // Vector registers start

// Buffer offsets - Type 3 CSR
#define OFF_CSR_TYPE     0
#define OFF_CSR_PC       8
#define OFF_CSR_VL       16
#define OFF_CSR_VTYPE    24
#define OFF_CSR_VSTART   32
#define OFF_CSR_VCSR     40
#define OFF_CSR_VLENB    48
#define OFF_CSR_RD       56  // rd scalar register value - contains VL
#define OFF_CSR_RS1      64  // rs1 scalar register value - contains AVL
#define OFF_CSR_RS2      72  // rs2 scalar register value - contains VTYPE
#define OFF_CSR_VREGS    80  // Vector registers start

static uint64_t vector_inst_count = 0;
static uint64_t all_inst_count = 0;

void emit_riscv_v_store_1(mambo_context *ctx,
                          unsigned int nf,
                          unsigned int mew,
                          unsigned int mop,
                          unsigned int vm,
                          unsigned int v2,
                          unsigned int rs1,
                          unsigned int vd);

void emit_riscv_csrrw (mambo_context *ctx, unsigned int rd, unsigned int csr_reg, unsigned int rs1);
void emit_riscv_csrrs (mambo_context *ctx, unsigned int rd, unsigned int csr_reg, unsigned int rs1);
void emit_riscv_csrrwi(mambo_context *ctx, unsigned int rd, unsigned int csr_reg, unsigned int uimm);



static inline void emit_riscv_csrr(mambo_context *ctx, unsigned int rd, unsigned int csr)
{
  emit_riscv_csrrs(ctx, rd, csr, X0);
}

static inline void emit_riscv_csrw(mambo_context *ctx, unsigned int csr, unsigned int rs1)
{
  emit_riscv_csrrw(ctx, X0, csr, rs1);
}

static inline void emit_riscv_csrw_imm(mambo_context *ctx, unsigned int csr, unsigned int uimm)
{
  emit_riscv_csrrwi(ctx, X0, csr, uimm);
}

static inline void emit_riscv_vsetvli(mambo_context *ctx,
                                      unsigned rd,
                                      unsigned rs1_avl,
                                      unsigned vtypei /* 12-bit */)
{
    vtypei &= 0xFFF;
    unsigned funct6 = (vtypei >> 6) & 0x3F;
    unsigned vm_bit = (vtypei >> 5) & 0x1;   // this is vtypei bit5, not "masking"
    unsigned vs2    = (vtypei >> 0) & 0x1F;

    emit_riscv_v_arith_generic(ctx,
                              funct6,
                              vm_bit,
                              vs2,
                              rs1_avl,
                              0x7,          // funct3 = 111
                              rd);
}

static inline void emit_riscv_vsetvl(mambo_context *ctx,
                                     unsigned rd,
                                     unsigned rs1_avl,
                                     unsigned rs2_vtype)
{
    emit_riscv_v_arith_generic(ctx,
                              0x20,         // funct6 for vsetvl
                              0,            // bit25 must be 0
                              rs2_vtype,    // vs2 = rs2
                              rs1_avl,      // vs1 = rs1
                              0x7,          // funct3 = 111
                              rd);
}

// Saves 8 consecutive vector registers starting from vbase to memory at address in xbase
static inline void emit_riscv_vs8r_v(mambo_context *ctx, unsigned vbase, unsigned xbase)
{
    // vs8r.v vbase, (xbase)  => nf=7, umop/sumop=WHOLE
    emit_riscv_v_store_1(ctx, 7, 0, 0, 1, RVV_SUMOP_WHOLE, xbase, vbase);
}

// Loads 8 consecutive vector registers from memory to registers starting from vbase
static inline void emit_riscv_vl8re8_v(mambo_context *ctx, unsigned vbase, unsigned xbase)
{
    // vl8re8.v vbase, (xbase) => nf=7, lumop=WHOLE, width=e8 in the _load_1 encoder
    emit_riscv_v_load_1(ctx, 7, 0, 0, 1, RVV_SUMOP_WHOLE, xbase, vbase);
}


// Stores a value to memory at address in rs1 + offset
static inline void emit_riscv_sd_off(mambo_context *ctx,
                                     unsigned int rs2,
                                     unsigned int rs1,
                                     int offset)
{
    unsigned int u = (unsigned int)(offset & 0xFFF);
    unsigned int immlo = u & 0x1F;
    unsigned int immhi = (u >> 5) & 0x7F;
    emit_riscv_sd(ctx, rs2, rs1, immhi, immlo);
}

// Loads a value from memory at address in rs1 + offset
static inline void emit_riscv_ld_off(mambo_context *ctx,
                                     unsigned int rd,
                                     unsigned int rs1,
                                     int offset)
{
    emit_riscv_ld(ctx, rd, rs1, offset);
}

// Returns: 0 = not vector, 1 = register-register, 2 = CSR, 3 = load/store
static inline int rvv_classify_instruction(uint32_t insn) {
    uint32_t opcode = insn & 0x7F;
    uint32_t funct3 = (insn >> 12) & 0x7;

    switch (opcode) {
    case 0x57:  // Vector arithmetic/CSR
        if (funct3 == 0b111) {
            return 2;  // CSR vsetvli, vsetvl
        }
        return 1;  // Register-register

    case 0x07:  // LOAD-FP
    case 0x27:  // STORE-FP
        switch (funct3) {
        case 0b000:
        case 0b101:
        case 0b110:
        case 0b111:
            return 3;  // Load/store
        default:
            return 0;  // Not rvv
        }

    default:
        return 0;  // Not rvv
    }
}

bool mambo_is_vector(mambo_context *ctx) {
    uint32_t insn = *(uint32_t *)ctx->code.read_address;
    return rvv_classify_instruction(insn) != 0;
}

// Extract register fields from RISC-V vector instruction encoding
// vd = bits [11:7]
// vs1 = bits [19:15]
// vs2 = bits [24:20]
static inline void rvv_extract_regs(uint32_t insn, uint8_t *vd, uint8_t *vs1, uint8_t *vs2) {
    *vd = (insn >> 7) & 0x1F;
    *vs1 = (insn >> 15) & 0x1F;
    *vs2 = (insn >> 20) & 0x1F;
}

// Forward declarations
static void rvv_print_to_json(uint32_t insn, uintptr_t pc);

// Register numbers
#define X_SP  2u
#define X_T5 30u
#define X_T6 31u
#define X_A0 10u
#define X_A1 11u

//TODO: this is hardcoded for 2048, we should make it dynamic since Vlenb can be different then 256
// Each group of 8 registers takes VLENB bytes. If VLENB = 256, then 8 * 256 = 2048 bytes.
// Ask if this is a potential issue. Fix should be simple. 
static inline void emit_addi_2048(mambo_context *ctx, unsigned rd, unsigned rs)
{
    // rd = rs + 2048 (addi immediate max is 2047)
    emit_riscv_addi(ctx, rd, rs, 2047);
    emit_riscv_addi(ctx, rd, rd, 1);
}


// Main function that saves all vector registers and calls the C function
// X_A0 buffer pointer 
// X_T5 and X_T6 are temporaries
// inst_type: 1 = reg-reg, 2 = CSR, 3 = load/store
static inline void emit_rvv_print_vtype_preserve(mambo_context *ctx,
                                                 unsigned tmp0,
                                                 unsigned tmp1,
                                                 uint32_t insn,
                                                 uintptr_t pc,
                                                 int inst_type)
{
    uint8_t buffer_type;
    int pc_offset, vl_offset, vtype_offset, vstart_offset, vcsr_offset, vlenb_offset, vregs_offset, rs1_offset;
    int func3 = (insn >> 12) & 0x7;
    // Order: reg-reg (most common), load/store, CSR (least common)
    if (inst_type == 1) { // Register-register (most common)
        buffer_type = RVV_TYPE_REG_REG;
        rs1_offset = OFF_REG_RS1;
        pc_offset = OFF_PC;
        vl_offset = OFF_VL;
        vtype_offset = OFF_VTYPE;
        vstart_offset = OFF_VSTART;
        vcsr_offset = OFF_VCSR;
        vlenb_offset = OFF_VLENB;
        vregs_offset = OFF_VREGS;
    } else if (inst_type == 3) { // Load/store (second most common)
        buffer_type = RVV_TYPE_LOAD_STORE;
        rs1_offset = OFF_LS_RS1;
        pc_offset = OFF_LS_PC;
        vl_offset = OFF_LS_VL;
        vtype_offset = OFF_LS_VTYPE;
        vstart_offset = OFF_LS_VSTART;
        vcsr_offset = OFF_LS_VCSR;
        vlenb_offset = OFF_LS_VLENB;
        vregs_offset = OFF_LS_VREGS;
    } else { // CSR (least common, inst_type == 2)
        buffer_type = RVV_TYPE_CSR;
        pc_offset = OFF_CSR_PC;
        vl_offset = OFF_CSR_VL;
        vtype_offset = OFF_CSR_VTYPE;
        vstart_offset = OFF_CSR_VSTART;
        vcsr_offset = OFF_CSR_VCSR;
        vlenb_offset = OFF_CSR_VLENB;
        vregs_offset = OFF_CSR_VREGS;
    }

    // Push mask to stack
    uint32_t mask = (1u << reg0) | (1u << reg1) | (1u << tmp0) | (1u << tmp1);
    emit_push(ctx, mask);

    // Set buffer pointer to g_saved_rvv
    emit_set_reg(ctx, reg0, (uintptr_t)g_saved_rvv);

    // Save temporaries to stack
    emit_riscv_addi(ctx, X_SP, X_SP, -16);
    emit_riscv_sd_off(ctx, X_T5, X_SP, 0);
    emit_riscv_sd_off(ctx, X_T6, X_SP, 8);

    // Write type byte to buffer (all types store type at offset 0)
    emit_set_reg(ctx, X_T5, buffer_type);
    emit_riscv_sd_off(ctx, X_T5, X_A0, 0);

    // Save PC to buffer
    emit_set_reg(ctx, X_T5, pc);
    emit_riscv_sd_off(ctx, X_T5, X_A0, pc_offset);

    // Save vector CSRs to buffer
    emit_riscv_csrr(ctx, X_T5, CSR_VL);     emit_riscv_sd_off(ctx, X_T5, X_A0, vl_offset);
    emit_riscv_csrr(ctx, X_T5, CSR_VTYPE);  emit_riscv_sd_off(ctx, X_T5, X_A0, vtype_offset);
    emit_riscv_csrr(ctx, X_T5, CSR_VSTART); emit_riscv_sd_off(ctx, X_T5, X_A0, vstart_offset);
    emit_riscv_csrr(ctx, X_T5, CSR_VCSR);   emit_riscv_sd_off(ctx, X_T5, X_A0, vcsr_offset);
    emit_riscv_csrr(ctx, X_T5, CSR_VLENB);  emit_riscv_sd_off(ctx, X_T5, X_A0, vlenb_offset);
    emit_riscv_csrw_imm(ctx, CSR_VSTART, 0);

    // For load/store, save rs1 value
    if (inst_type == 3 || (inst_type == 1 && ((func3 == 0b100) || (func3 == 0b101) || (func3 == 0b110)))) { // load/store or OPIVX/OPIVF/OPMVX
        uint8_t rs1_num = (insn >> 15) & 0x1F;
        
        // Read rs1 
        emit_mov(ctx, X_T5, (enum reg)rs1_num);
        emit_riscv_sd_off(ctx, X_T5, X_A0, rs1_offset);

        // For segmented load/store save rs2
        if (((insn >> 26) & 0x3) == 0b10) { 
            uint8_t rs2_num = (insn >> 20) & 0x1F;
            emit_mov(ctx, X_T5, (enum reg)rs2_num);
            emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_LS_RS2);
        }

    } else if (inst_type == 2) { // For CSR instructions, saving rd, rs1, and rs2 values
        uint8_t rd_num = (insn >> 7) & 0x1F;
        uint8_t rs1_num = (insn >> 15) & 0x1F;
        uint8_t rs2_num = (insn >> 20) & 0x1F;
        
        // Read rd
        emit_mov(ctx, X_T5, (enum reg)rd_num);
        emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_CSR_RD);
        
        // Read rs1
        emit_mov(ctx, X_T5, (enum reg)rs1_num);
        emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_CSR_RS1);
        
        // Read rs2
        emit_mov(ctx, X_T5, (enum reg)rs2_num);
        emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_CSR_RS2);
    }

    // Save vector registers to buffer
    emit_riscv_addi(ctx, X_T6, X_A0, vregs_offset);

    emit_riscv_vs8r_v(ctx,  0, X_T6);   // v0..v7
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vs8r_v(ctx,  8, X_T6);   // v8..v15
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vs8r_v(ctx, 16, X_T6);   // v16..v23
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vs8r_v(ctx, 24, X_T6);   // v24..v31

    // Restore temporaries from stack
    emit_riscv_ld_off(ctx, X_T5, X_SP, 0);
    emit_riscv_ld_off(ctx, X_T6, X_SP, 8);
    emit_riscv_addi(ctx, X_SP, X_SP, 16);

    // Call the C function to write to JSON with instruction and PC
    emit_set_reg(ctx, reg0, (uintptr_t)insn);
    emit_set_reg(ctx, reg1, pc);
    emit_safe_fcall(ctx, (void*)rvv_print_to_json, 2);

    emit_set_reg(ctx, reg0, (uintptr_t)g_saved_rvv);

    // Store temporaries to stack
    emit_riscv_addi(ctx, X_SP, X_SP, -16);
    emit_riscv_sd_off(ctx, X_T5, X_SP, 0);
    emit_riscv_sd_off(ctx, X_T6, X_SP, 8);

    // Move pointer to start of vector register data
    emit_riscv_addi(ctx, X_T6, X_A0, vregs_offset);

    // Restore registers from buffer
    emit_riscv_vl8re8_v(ctx,  0, X_T6);
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vl8re8_v(ctx,  8, X_T6);
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vl8re8_v(ctx, 16, X_T6);
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vl8re8_v(ctx, 24, X_T6);

    // Restore vector CSRs from buffer
    // CRITICAL: Restore VL and VTYPE FIRST to establish valid vector state
    emit_riscv_ld_off(ctx, X_T5, X_A0, vl_offset);
    emit_riscv_ld_off(ctx, X_T6, X_A0, vtype_offset);
    emit_riscv_vsetvl(ctx, X0, X_T5, X_T6);

    // Now restore remaining CSRs
    emit_riscv_ld_off(ctx, X_T5, X_A0, vcsr_offset);
    emit_riscv_csrw(ctx, CSR_VCSR, X_T5);

    emit_riscv_ld_off(ctx, X_T5, X_A0, vstart_offset);
    emit_riscv_csrw(ctx, CSR_VSTART, X_T5);

    // Restore temporaries from stack
    emit_riscv_ld_off(ctx, X_T5, X_SP, 0);
    emit_riscv_ld_off(ctx, X_T6, X_SP, 8);
    emit_riscv_addi(ctx, X_SP, X_SP, 16);

    // Restore mask
    emit_pop(ctx, mask);
}

static inline uint64_t calc_vreg_offset(uint8_t vr, uint64_t vlenb) {
    unsigned int group = vr / 8;
    unsigned int index_in_group = vr % 8;
    return (uint64_t)group * 2048 + (uint64_t)index_in_group * vlenb;
}

static inline void vreg_to_hex_string(char *out, size_t out_size, const uint8_t *data, size_t len) {
    size_t pos = 0;
    for (size_t i = 0; i < len && pos < out_size - 1; i++) {
        pos += snprintf(out + pos, out_size - pos, "%02x", data[i]);
    }
    out[out_size - 1] = '\0';
}

// Helper function to extract LMUL * 8 from vtype encoding
// We don't want the RISC-V machine to require floating point support only vector. 
static inline int32_t extract_lmul_x8_from_vtype(uint64_t vtype) {
    uint8_t vlmul = vtype & 0x7;  // bits [2:0]
    
    switch (vlmul) {
        case 0b000: return 8;    // LMUL=1   1*8 = 8
        case 0b001: return 16;   // LMUL=2   2*8 = 16
        case 0b010: return 32;   // LMUL=4   4*8 = 32
        case 0b011: return 64;   // LMUL=8   8*8 = 64
        case 0b101: return 1;    // LMUL=1/8 0.125*8 = 1
        case 0b110: return 2;    // LMUL=1/4 0.25*8 = 2
        case 0b111: return 4;    // LMUL=1/2 0.5*8 = 4
        default:    return 8;
    }
}

static inline bool is_widening_instruction(uint32_t insn, uint32_t vs1) {
    uint32_t funct3 = (insn >> 12) & 0x7;
    uint32_t funct6 = (insn >> 26) & 0x3F;

    // widening converts
    if (funct3 == 0b001) {
        if (funct6 != 0b010010) return false;
        return (vs1 >= 0b01000 && vs1 <= 0b01111); 
    }

    return ((funct6 >= 0b110000) && (funct6 <= 0b111111));
}

static inline bool is_narrowing_instruction(uint32_t insn, uint32_t vs1) {
    uint32_t funct3 = (insn >> 12) & 0x7; 
    if (funct3 != 0b000 && funct3 != 0b011 && funct3 != 0b100 && funct3 != 0b001) return false;

    uint32_t funct6 = (insn >> 26) & 0x3F;
    
    // narrowing converts
    if (funct3 == 0b001) {
        if (funct6 != 0b010010) return false;
        return (vs1 >= 0b10000 && vs1 <= 0b10111); 
    }

     // vnsrl, vnsra, vnclipu, vnclip
    return (funct6 >= 0b101100 && funct6 <= 0b101111);
}

// Helper function to print register data as hex string in JSON output
static inline void print_vreg_data_json(FILE *fp, 
                                        const char *field_name,
                                        uint8_t vreg_start,
                                        uint8_t reg_count,
                                        const uint8_t *vregs_base,
                                        uint64_t vlenb)
{
    fprintf(fp, "    \"%s\": ", field_name);
    
    fprintf(fp, "[\n");
    for (uint8_t i = 0; i < reg_count; i++) {
        uint64_t offset = calc_vreg_offset(vreg_start + i, vlenb);
        const uint8_t *vreg_data = vregs_base + offset;
        char vreg_hex[RVV_VREG_BYTES * 2 + 1];
        vreg_to_hex_string(vreg_hex, sizeof(vreg_hex), vreg_data, (size_t)vlenb);
        
        fprintf(fp, "      \"%s\"", vreg_hex);
        if (i < reg_count - 1) fprintf(fp, ",");
        fprintf(fp, "\n");
    }
    fprintf(fp, "    ]");
}

// Static file handle for JSON output (kept open for efficiency)
static FILE *json_fp = NULL;
static bool json_first_entry = true;

// Prints a single instruction entry to JSON file
// Reads vd, vs1, vs2 from g_saved_rvv buffer based on type
static void rvv_print_to_json(uint32_t insn, uintptr_t pc) {

    if (json_fp == NULL) {
        json_fp = fopen("vector_trace.json", "a");
        if (json_fp == NULL) {
            fprintf(stderr, "Error: Failed to open vector_trace.json for writing\n");
            return;
        }
    }
    
    // Read buffer type from offset 0 (all types store type at offset 0)
    uint8_t buffer_type = g_saved_rvv[0];
    
    int vlenb_offset, vregs_offset, vl_offset, vtype_offset;
    // Order: reg-reg (most common), load/store, CSR (least common)
    if (buffer_type == RVV_TYPE_REG_REG) {
        vl_offset = OFF_VL;
        vtype_offset = OFF_VTYPE;
        vlenb_offset = OFF_VLENB;
        vregs_offset = OFF_VREGS;
    } else if (buffer_type == RVV_TYPE_LOAD_STORE) {
        vl_offset = OFF_LS_VL;
        vtype_offset = OFF_LS_VTYPE;
        vlenb_offset = OFF_LS_VLENB;
        vregs_offset = OFF_LS_VREGS;
    } else { // CSR (least common)
        vl_offset = OFF_CSR_VL;
        vtype_offset = OFF_CSR_VTYPE;
        vlenb_offset = OFF_CSR_VLENB;
        vregs_offset = OFF_CSR_VREGS;
    }
    
    uint64_t vlenb = *(const uint64_t *)(g_saved_rvv + vlenb_offset);
    uint64_t vtype = *(const uint64_t *)(g_saved_rvv + vtype_offset);
    
    // Validate vlenb
    if (vlenb == 0 || vlenb > RVV_VREG_BYTES) {
        vlenb = RVV_VREG_BYTES;
    }
    
    // Extract register numbers from instruction
    uint8_t vd, vs1_rs1_imm, vs2_rs2;
    rvv_extract_regs(insn, &vd, &vs1_rs1_imm, &vs2_rs2);

    int32_t lmul_x8 = extract_lmul_x8_from_vtype(vtype);
    int8_t funct3 = (insn >> 12) & 0x7;

    const uint8_t *vregs_base = g_saved_rvv + vregs_offset;
    
    uint64_t vd_offset = calc_vreg_offset(vd, vlenb);
    uint64_t vs1_offset = calc_vreg_offset(vs1_rs1_imm, vlenb);
    uint64_t vs2_offset = calc_vreg_offset(vs2_rs2, vlenb);
    
    const uint8_t *vd_data = vregs_base + vd_offset;
    const uint8_t *vs1_data = vregs_base + vs1_offset;
    const uint8_t *vs2_data = vregs_base + vs2_offset;

    // Writing JSON entry
    if (!json_first_entry) {
        fprintf(json_fp, ",\n");
    }
    json_first_entry = false;
    
    fprintf(json_fp, "  {\n");
    fprintf(json_fp, "    \"pc\": \"0x%016" PRIx64 "\",\n", pc);
    fprintf(json_fp, "    \"instruction\": \"0x%08" PRIx32 "\",\n", insn);
    fprintf(json_fp, "    \"type\": %u,\n", buffer_type);
    fprintf(json_fp, "    \"number\": %lu, \n", vector_inst_count);

    if (lmul_x8 >= 8) {
        fprintf(json_fp, "    \"lmul\": %u,\n", lmul_x8 / 8);
    } else {
        fprintf(json_fp, "    \"lmul\": \"1/%u\",\n", 8 / lmul_x8);
    }

    if (buffer_type == RVV_TYPE_REG_REG) {

        bool is_widening = is_widening_instruction(insn, vs1_rs1_imm);  
        bool is_narrowing = is_narrowing_instruction(insn, vs1_rs1_imm);

        // lmul_x8 / 8 gives number of registers
        uint8_t vd_reg_count = is_widening ? (uint8_t)(lmul_x8 / 4) : (uint8_t)(lmul_x8 / 8);
        if (vd_reg_count < 1) vd_reg_count = 1;

        fprintf(json_fp, "    \"vd\": %u,\n", vd);
        print_vreg_data_json(json_fp, "vd_data", vd, vd_reg_count, vregs_base, vlenb);
        fprintf(json_fp, ",\n");

        if (funct3 == 0b011) { // OPIVI
            fprintf(json_fp, "    \"imm\": %u,\n", vs1_rs1_imm);
        }
        else if (funct3 == 0b100 || funct3 == 0b101 || funct3 == 0b110) { // OPIVX, OPIVF, OPMVX
            uint64_t rs1_value = *(const uint64_t *)(g_saved_rvv + OFF_LS_RS1);

            fprintf(json_fp, "    \"rs1\": %u,\n", vs1_rs1_imm);
            fprintf(json_fp, "    \"rs1_value\": \"0x%016" PRIx64 "\",\n", rs1_value);
        } else {
            uint8_t vs1_reg_count = (lmul_x8 >= 8) ? (uint8_t)(lmul_x8 / 8) : 1;

            fprintf(json_fp, "    \"vs1\": %u,\n", vs1_rs1_imm);
            print_vreg_data_json(json_fp, "vs1_data", vs1_rs1_imm, vs1_reg_count, vregs_base, vlenb);
            fprintf(json_fp, ",\n");
        }

        fprintf(json_fp, "    \"vs2\": %u,\n", vs2_rs2);
        uint8_t vs2_reg_count = is_narrowing ? (uint8_t)(lmul_x8 / 4) : (uint8_t)(lmul_x8 / 8);
        if (vs2_reg_count < 1) vs2_reg_count = 1;
        print_vreg_data_json(json_fp, "vs2_data", vs2_rs2, vs2_reg_count, vregs_base, vlenb);


    } else if (buffer_type == RVV_TYPE_LOAD_STORE) {
        uint64_t rs1_value = *(const uint64_t *)(g_saved_rvv + OFF_LS_RS1);
        uint8_t mop = (insn >> 26) & 0x3;

        // nf = bits [31:29] + 1; multiply by LMUL (= lmul_x8/8)
        uint8_t nf = ((insn >> 29) & 0x7) + 1;
        uint8_t num_of_destination_reg = (uint8_t)((nf * lmul_x8) / 8);
        if (num_of_destination_reg < 1) num_of_destination_reg = 1;

        fprintf(json_fp, "    \"rs1\": %u,\n", vs1_rs1_imm);
        fprintf(json_fp, "    \"rs1_value\": \"0x%016" PRIx64 "\",\n", rs1_value);
        
        if (mop == 0b10) { // strided
            uint64_t rs2_value = *(const uint64_t *)(g_saved_rvv + OFF_LS_RS2);
            fprintf(json_fp, "    \"rs2\": %u,\n", vs2_rs2);
            fprintf(json_fp, "    \"rs2_value\": \"0x%016" PRIx64 "\",\n", rs2_value);
        } else if ((mop == 0b01) || (mop == 0b11)) { // Indexed
            fprintf(json_fp, "    \"vs2\": %u,\n", vs2_rs2);
            
            char vreg_hex[RVV_VREG_BYTES * 2 + 1];
            vreg_to_hex_string(vreg_hex, sizeof(vreg_hex), vs2_data, (size_t)vlenb);
            fprintf(json_fp, "    \"vs2_data\": \"%s\",\n", vreg_hex);
        }

        fprintf(json_fp, "    \"vd\": %u,\n", vd);
        print_vreg_data_json(json_fp, "vd_data", vd, num_of_destination_reg, vregs_base, vlenb);

    } else {
        uint64_t vl = *(const uint64_t *)(g_saved_rvv + OFF_CSR_VL);
        uint64_t vtype = *(const uint64_t *)(g_saved_rvv + OFF_CSR_VTYPE);
        uint64_t vstart = *(const uint64_t *)(g_saved_rvv + OFF_CSR_VSTART);
        uint64_t vcsr = *(const uint64_t *)(g_saved_rvv + OFF_CSR_VCSR);
        uint64_t vlenb = *(const uint64_t *)(g_saved_rvv + OFF_CSR_VLENB);
        uint64_t rd_value = *(const uint64_t *)(g_saved_rvv + OFF_CSR_RD);
        uint64_t rs1_value = *(const uint64_t *)(g_saved_rvv + OFF_CSR_RS1);
        uint64_t rs2_value = *(const uint64_t *)(g_saved_rvv + OFF_CSR_RS2);
        
        fprintf(json_fp, "    \"rd\": %u,\n", vd);  // rd is in vd field
        fprintf(json_fp, "    \"rd_value\": \"0x%016" PRIx64 "\",\n", rd_value);
        fprintf(json_fp, "    \"rs1\": %u,\n", vs1_rs1_imm);  // rs1 is in vs1 field
        fprintf(json_fp, "    \"rs1_value\": \"0x%016" PRIx64 "\",\n", rs1_value);
        fprintf(json_fp, "    \"rs2\": %u,\n", vs2_rs2);  // rs2 is in vs2 field
        fprintf(json_fp, "    \"rs2_value\": \"0x%016" PRIx64 "\",\n", rs2_value);
        fprintf(json_fp, "    \"vl\": \"0x%016" PRIx64 "\",\n", vl);
        fprintf(json_fp, "    \"vtype\": \"0x%016" PRIx64 "\",\n", vtype);
        fprintf(json_fp, "    \"vstart\": \"0x%016" PRIx64 "\",\n", vstart);
        fprintf(json_fp, "    \"vcsr\": \"0x%016" PRIx64 "\",\n", vcsr);
        fprintf(json_fp, "    \"vlenb\": \"0x%016" PRIx64 "\"\n", vlenb);
    }
    
    fprintf(json_fp, "  }");
    
    // Flush to ensure data is written (but keep file open should be faster this way)
    fflush(json_fp);
}

// Post-instruction callback that prints instruction data directly to JSON
static int vector_post_inst_cb(mambo_context *ctx) {
    uint32_t insn = *(uint32_t *)ctx->code.read_address;
    int inst_type = rvv_classify_instruction(insn);
    
    if (inst_type == 0) return 0;

    emit_counter64_incr(ctx, &all_inst_count, 1);

    uintptr_t pc = (uintptr_t)ctx->code.read_address;
    emit_rvv_print_vtype_preserve(ctx, reg2, reg3, insn, pc, inst_type);
    
    emit_counter64_incr(ctx, &vector_inst_count, 1);
    
    return 0;
}

// For tracing all instructions we using pre_inst callback.
static int all_inst_pre_cb(mambo_context *ctx) {
    emit_counter64_incr(ctx, &all_inst_count, 1);
    return 0;
}

__attribute__((constructor)) static void main_tracer_init(void) {
    FILE *fp = fopen("vreg_dump.txt", "w");
    if (fp == NULL) {
        fprintf(stderr, "Warning: Failed to create/truncate vreg_dump.txt\n");
    } else {
        fclose(fp);
    }
    
    // Open JSON file and keep it open for efficient writing
    json_fp = fopen("vector_trace.json", "w");
    if (json_fp == NULL) {
        fprintf(stderr, "Warning: Failed to create/truncate vector_trace.json\n");
    } else {
        fprintf(json_fp, "[\n");
        fflush(json_fp);
    }
    
    mambo_context *ctx = mambo_register_plugin();
    int set_cb = mambo_register_post_inst_cb(ctx, &vector_post_inst_cb);
    assert(set_cb == MAMBO_SUCCESS);
    int set_all_cb = mambo_register_pre_inst_cb(ctx, &all_inst_pre_cb);
    assert(set_all_cb == MAMBO_SUCCESS);
}

// Prints count at exit and finalizes JSON
__attribute__((destructor)) static void vector_counter_fini(void) {
    fprintf(stderr, "[vector_counter] total vector instructions executed: %" PRIu64 "\n",
            vector_inst_count);
    fprintf(stderr, "[vector_counter] total instructions executed: %" PRIu64 "\n",
            all_inst_count);
    
    if (json_fp != NULL) {
        fprintf(json_fp, "]\n");
        fclose(json_fp);
        json_fp = NULL;
    }
}
