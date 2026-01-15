#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdint.h>
#include <stddef.h>
#include "plugins.h"

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

// Buffer layout for saved_inst_reg_rvv (lightweight dump: PC, instruction, vlenb, vd, vs1, vs2)
// Layout:
//   PC: 8 bytes
//   Instruction: 8 bytes
//   VLENB: 8 bytes
//   vd: vlenb bytes (offset 24)
//   vs1: vlenb bytes (offset 24 + vlenb)
//   vs2: vlenb bytes (offset 24 + 2*vlenb)
// Since the value of vlenb is not known at compile time, we need to use a fixed size buffer. Here 1024 per entry
#define INST_REG_HDR_BYTES  24u        // PC (8) + Instruction (8) + VLENB (8)
#define INST_REG_VREG_BYTES 256u       // Max bytes per vector register
#define INST_REG_NUM_VREGS  3u         // vd, vs1, vs2
#define INST_REG_ENTRY_BYTES 1024u     // Size per entry (rounded up )
#define INST_REG_MAX_ENTRIES 1u       // Maximum number of instructions to store before flushing /* Very important variable change when needed */
#define INST_REG_TOTAL_BYTES (INST_REG_MAX_ENTRIES * INST_REG_ENTRY_BYTES)  // 10 * 1024 = 10240 bytes

__attribute__((aligned(64)))
static uint8_t saved_inst_reg_rvv[INST_REG_TOTAL_BYTES]; // This is the main buffer we are using to save vector registers for json dump

// Buffer offsets for a single entry in saved_inst_reg_rvv
#define INST_REG_OFF_PC      0
#define INST_REG_OFF_INSN    8
#define INST_REG_OFF_VLENB   16
#define INST_REG_OFF_VD      24
#define INST_REG_OFF_VS1     (INST_REG_OFF_VD + INST_REG_VREG_BYTES)
#define INST_REG_OFF_VS2     (INST_REG_OFF_VS1 + INST_REG_VREG_BYTES)

// Counter for current position in the buffer (0 to INST_REG_MAX_ENTRIES-1)
static uint32_t saved_inst_reg_index = 0;

// Buffer for saving full vector state (all 32 registers + CSRs)
// Layout: PC (8) + VL (8) + VTYPE (8) + VSTART (8) + VCSR (8) + VLENB (8) + VREGS (32 * 256 = 8192)
// Total: 64 + 8192 = 8256 bytes, rounded to 64-byte alignment
#define RVV_BUF_BYTES 8256u
#define OFF_PC      0
#define OFF_VL      8
#define OFF_VTYPE   16
#define OFF_VSTART  24
#define OFF_VCSR    32
#define OFF_VLENB   40
#define OFF_VREGS   64

__attribute__((aligned(64)))
static uint8_t g_saved_rvv[RVV_BUF_BYTES];

// Forward declarations
static void rvv_print_to_json(uint32_t num_entries);

// External function declarations
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
void emit_riscv_v_load_1(mambo_context *ctx,
                         unsigned int nf,
                         unsigned int mew,
                         unsigned int mop,
                         unsigned int vm,
                         unsigned int v2,
                         unsigned int rs1,
                         unsigned int vd);

// Register number definitions
#define X_SP  2u
#define X_T5 30u
#define X_T6 31u
#define X_A0 10u

// CSR helper functions
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

// Save a single vector register to memory at address in xbase
static inline void emit_riscv_vs1r_v(mambo_context *ctx,
                                    unsigned int vS /* vector register number */,
                                    unsigned int xBase /* base GPR */)
{
  emit_riscv_v_store_1(ctx,
    /*nf*/  0,
    /*mew*/ 0,
    /*mop*/ 0,
    /*vm*/  1,
    /*v2*/  RVV_SUMOP_WHOLE,
    /*rs1*/ xBase,
    /*vd*/  vS
  );
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

// Add immediate 2048 (used for advancing pointer between register groups)
// Each group of 8 registers takes VLENB bytes. If VLENB = 256, then 8 * 256 = 2048 bytes.
static inline void emit_addi_2048(mambo_context *ctx, unsigned rd, unsigned rs)
{
    // rd = rs + 2048 (addi immediate max is 2047)
    emit_riscv_addi(ctx, rd, rs, 2047);
    emit_riscv_addi(ctx, rd, rd, 1);
}

// Emit vsetvl instruction: vsetvl rd, rs1, rs2
// Copy of working implementation from vector_inst_new.c
void emit_riscv_v_arith_generic(mambo_context *ctx,
                                 unsigned int funct6,
                                 unsigned int bit25,
                                 unsigned int vs2,
                                 unsigned int vs1,
                                 unsigned int funct3,
                                 unsigned int rd);

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

// Checks if an instruction is a vector word instruction
static inline bool rvv_is_vector_word(uint32_t insn) {
    if (insn == 0x5e00b057)
        return false;
    uint32_t opcode = insn & 0x7F;
    uint32_t funct3 = (insn >> 12) & 0x7;

    switch (opcode) {
    case 0x57:
        if (funct3 == 0b111)
            return false;
        return true;

    case 0x07:
    case 0x27:
        switch (funct3) {
        // get all vector encodings under LOAD-FP/STORE-FP opcodes
        case 0b000:
        case 0b101:
        case 0b110:
        case 0b111:
            return true;
        default:
            return false;
        }

    default:
        return false;
    }
}

bool mambo_is_vector(mambo_context *ctx) {
    uint32_t insn = *(uint32_t *)ctx->code.read_address;
    return rvv_is_vector_word(insn);
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

// Dumps PC, instruction, and three vector registers (vd, vs1, vs2) to saved_inst_reg_rvv buffer
static inline void emit_rvv_dump_inst_to_buf(mambo_context *ctx,
                                             uintptr_t pc,
                                             uint32_t insn,
                                             unsigned int x_buf, // buffer pointer
                                             unsigned int x_tmp) // temporary register
{
    // Extracting registers from instruction
    uint8_t vd, vs1, vs2;
    rvv_extract_regs(insn, &vd, &vs1, &vs2);
    
    // Save VSTART
    emit_riscv_addi(ctx, X_SP, X_SP, -8);
    emit_riscv_csrr(ctx, X_T5, CSR_VSTART);
    emit_riscv_sd_off(ctx, X_T5, X_SP, 0);
    emit_riscv_csrw_imm(ctx, CSR_VSTART, 0);
    
    // Save PC to buffer
    emit_set_reg(ctx, x_tmp, pc);
    emit_riscv_addi(ctx, X_T6, x_buf, INST_REG_OFF_PC);
    emit_riscv_sd_off(ctx, x_tmp, X_T6, 0);
    
    // Save instruction to buffer
    emit_set_reg(ctx, x_tmp, (uintptr_t)insn);
    emit_riscv_addi(ctx, X_T6, x_buf, INST_REG_OFF_INSN);
    emit_riscv_sd_off(ctx, x_tmp, X_T6, 0);
    
    // Save VLENB to buffer to know reg size
    emit_riscv_csrr(ctx, X_T5, CSR_VLENB);
    emit_riscv_addi(ctx, X_T6, x_buf, INST_REG_OFF_VLENB);
    emit_riscv_sd_off(ctx, X_T5, X_T6, 0);
    
    // Save vd register
    emit_riscv_addi(ctx, X_T6, x_buf, INST_REG_OFF_VD);
    emit_riscv_vs1r_v(ctx, vd, X_T6);
    
    // Save vs1 register
    emit_riscv_addi(ctx, X_T6, x_buf, INST_REG_OFF_VS1);
    emit_riscv_vs1r_v(ctx, vs1, X_T6);
    
    // Save vs2 register
    emit_riscv_addi(ctx, X_T6, x_buf, INST_REG_OFF_VS2);
    emit_riscv_vs1r_v(ctx, vs2, X_T6);
    
    // Restore VSTART
    emit_riscv_ld_off(ctx, X_T5, X_SP, 0);
    emit_riscv_csrw(ctx, CSR_VSTART, X_T5);
    emit_riscv_addi(ctx, X_SP, X_SP, 8);
}



// Main function that saves all vector registers and calls the C function
// This is the ONLY function allowed to make system calls (via emit_safe_fcall)
// It saves the entire vector state, calls rvv_print_to_json, then restores everything
// tmp0 and tmp1 are temporary registers (should be reg2 and reg3)
static inline void emit_rvv_print_vtype_preserve(mambo_context *ctx,
                                                 unsigned tmp0,
                                                 unsigned tmp1)
{
    // Push mask to stack
    uint32_t mask = (1u << reg0) | (1u << reg1) | (1u << tmp0) | (1u << tmp1);
    emit_push(ctx, mask);

    // Set buffer pointer to g_saved_rvv in reg0 (X_A0)
    emit_set_reg(ctx, reg0, (uintptr_t)g_saved_rvv);

    // Save temporaries to stack
    emit_riscv_addi(ctx, X_SP, X_SP, -16);
    emit_riscv_sd_off(ctx, X_T5, X_SP, 0);
    emit_riscv_sd_off(ctx, X_T6, X_SP, 8);

    // Save vector CSRs to buffer
    emit_riscv_csrr(ctx, X_T5, CSR_VL);     emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_VL);
    emit_riscv_csrr(ctx, X_T5, CSR_VTYPE);  emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_VTYPE);
    emit_riscv_csrr(ctx, X_T5, CSR_VSTART); emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_VSTART);
    emit_riscv_csrr(ctx, X_T5, CSR_VCSR);   emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_VCSR);
    emit_riscv_csrr(ctx, X_T5, CSR_VLENB);  emit_riscv_sd_off(ctx, X_T5, X_A0, OFF_VLENB);
    emit_riscv_csrw_imm(ctx, CSR_VSTART, 0);

    // Save all 32 vector registers to buffer
    emit_riscv_addi(ctx, X_T6, X_A0, OFF_VREGS);

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

    // Call the C function to write to JSON and pass number of entries saved
    // This is the ONLY system call allowed - it's protected by full state save/restore
    emit_set_reg(ctx, reg0, (uintptr_t)saved_inst_reg_index);
    emit_safe_fcall(ctx, (void*)rvv_print_to_json, 1);

    // Reset the index to 0 after flushing
    emit_set_reg(ctx, reg0, (uintptr_t)&saved_inst_reg_index);
    emit_set_reg(ctx, reg1, 0);
    emit_riscv_sd_off(ctx, reg1, reg0, 0);

    // Set buffer pointer back to g_saved_rvv for restoration
    emit_set_reg(ctx, reg0, (uintptr_t)g_saved_rvv);

    // Store temporaries to stack
    emit_riscv_addi(ctx, X_SP, X_SP, -16);
    emit_riscv_sd_off(ctx, X_T5, X_SP, 0);
    emit_riscv_sd_off(ctx, X_T6, X_SP, 8);

    // CRITICAL: After emit_safe_fcall, vector state may be corrupted
    // We need to restore VL and VTYPE FIRST to establish a valid vector state
    // before we can use any vector instructions (like vl8re8_v)
    emit_riscv_ld_off(ctx, X_T5, X_A0, OFF_VL);
    emit_riscv_ld_off(ctx, X_T6, X_A0, OFF_VTYPE);

    // Restore VL and VTYPE first to establish valid vector state
    emit_riscv_vsetvl(ctx, X0, X_T5, X_T6);

    // Now set VSTART to 0 before loading registers
    emit_riscv_csrw_imm(ctx, CSR_VSTART, 0);

    // Move pointer to start of vector register data
    emit_riscv_addi(ctx, X_T6, X_A0, OFF_VREGS);

    // Restore all 32 vector registers from buffer
    emit_riscv_vl8re8_v(ctx,  0, X_T6);
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vl8re8_v(ctx,  8, X_T6);
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vl8re8_v(ctx, 16, X_T6);
    emit_addi_2048(ctx, X_T6, X_T6);

    emit_riscv_vl8re8_v(ctx, 24, X_T6);

    // Restore remaining vector CSRs from buffer
    emit_riscv_ld_off(ctx, X_T5, X_A0, OFF_VCSR);
    emit_riscv_csrw(ctx, CSR_VCSR, X_T5);

    emit_riscv_ld_off(ctx, X_T5, X_A0, OFF_VSTART);
    emit_riscv_csrw(ctx, CSR_VSTART, X_T5);

    // Restore temporaries from stack
    emit_riscv_ld_off(ctx, X_T5, X_SP, 0);
    emit_riscv_ld_off(ctx, X_T6, X_SP, 8);
    emit_riscv_addi(ctx, X_SP, X_SP, 16);

    // Restore mask
    emit_pop(ctx, mask);
}







// Helper function to convert vector register bytes to hex string
static void vreg_to_hex_string(char *out, size_t out_size, const uint8_t *data, size_t len) {
    size_t pos = 0;
    for (size_t i = 0; i < len && pos < out_size - 1; i++) {
        pos += snprintf(out + pos, out_size - pos, "%02x", data[i]);
    }
    out[out_size - 1] = '\0';
}

// Needed to track "," in JSON file
static bool json_first_entry = true;

// Prints num_entries registers in saved_inst_reg_rvv to JSON file
static void rvv_print_to_json(uint32_t num_entries) {
    if (num_entries == 0) return;
    
    FILE *fp = fopen("vector_trace.json", "a");
    if (fp == NULL) {
        fprintf(stderr, "Error: Failed to open vector_trace.json for writing\n");
        return;
    }
    
    for (uint32_t i = 0; i < num_entries; i++) {
        uint64_t entry_offset = (uint64_t)i * INST_REG_ENTRY_BYTES;
        const uint8_t *entry = saved_inst_reg_rvv + entry_offset;
        
        uint64_t pc = *(const uint64_t *)(entry + INST_REG_OFF_PC);
        uint32_t insn = (uint32_t)*(const uint64_t *)(entry + INST_REG_OFF_INSN);
        uint64_t vlenb = *(const uint64_t *)(entry + INST_REG_OFF_VLENB);
        
        // Validate vlenb
        if (vlenb == 0 || vlenb > INST_REG_VREG_BYTES) {
            // Fallback to max size if vlenb is invalid
            vlenb = INST_REG_VREG_BYTES;
        }
        
        const uint8_t *vd_data = entry + INST_REG_OFF_VD;
        const uint8_t *vs1_data = entry + INST_REG_OFF_VS1;
        const uint8_t *vs2_data = entry + INST_REG_OFF_VS2;
        
        // Extract register numbers
        uint8_t vd, vs1, vs2;
        rvv_extract_regs(insn, &vd, &vs1, &vs2);
        
        // Convert registers to hex strings
        char vd_hex[INST_REG_VREG_BYTES * 2 + 1];
        char vs1_hex[INST_REG_VREG_BYTES * 2 + 1];
        char vs2_hex[INST_REG_VREG_BYTES * 2 + 1];
        
        vreg_to_hex_string(vd_hex, sizeof(vd_hex), vd_data, (size_t)vlenb);
        vreg_to_hex_string(vs1_hex, sizeof(vs1_hex), vs1_data, (size_t)vlenb);
        vreg_to_hex_string(vs2_hex, sizeof(vs2_hex), vs2_data, (size_t)vlenb);
        
        // Writing JSON entry
        if (!json_first_entry) {
            fprintf(fp, ",\n");
        }
        json_first_entry = false;
        
        fprintf(fp, "  {\n");
        fprintf(fp, "    \"pc\": \"0x%016" PRIx64 "\",\n", pc);
        fprintf(fp, "    \"instruction\": \"0x%08" PRIx32 "\",\n", insn);
        fprintf(fp, "    \"vd\": %u,\n", vd);
        fprintf(fp, "    \"vs1\": %u,\n", vs1);
        fprintf(fp, "    \"vs2\": %u,\n", vs2);
        fprintf(fp, "    \"vd_data\": \"%s\",\n", vd_hex);
        fprintf(fp, "    \"vs1_data\": \"%s\",\n", vs1_hex);
        fprintf(fp, "    \"vs2_data\": \"%s\"\n", vs2_hex);
        fprintf(fp, "  }");
    }
    
    fclose(fp);
}

// Finalizes JSON file by writing any remaining entries
static void finalize_json(void) {
    if (saved_inst_reg_index == 0) {
        return;
    }
    
    rvv_print_to_json(saved_inst_reg_index);
    saved_inst_reg_index = 0;
}

static uint64_t vector_inst_count = 0;

// Post-instruction callback that saves instruction data to saved_inst_reg_rvv buffer
static int vector_post_inst_cb(mambo_context *ctx) {
    if (!mambo_is_vector(ctx)) return 0;

    uint32_t insn = *(uint32_t *)ctx->code.read_address;
    uintptr_t pc = (uintptr_t)ctx->code.read_address;

    emit_counter64_incr(ctx, &vector_inst_count, 1);
    
    emit_push(ctx, (1 << reg0) | (1 << reg1) | (1 << reg2) | (1 << reg3));
    
    // Load current index into reg1
    emit_set_reg(ctx, reg0, (uintptr_t)&saved_inst_reg_index);
    emit_riscv_ld(ctx, reg1, reg0, 0);
    
    // Save index address
    emit_riscv_addi(ctx, X_SP, X_SP, -8);
    emit_riscv_sd_off(ctx, reg0, X_SP, 0);
    
    // Calculation of buffer entry offset: reg2 = reg1 * INST_REG_ENTRY_BYTES
    // We need to multiply reg1 by INST_REG_ENTRY_BYTES (1024)
    // Since 1024 = 2^10, we can use left shift: reg1 << 10 since it's faster than multiplication
    emit_riscv_slli(ctx, reg2, reg1, 10);  // reg2 = reg1 * 1024
    
    // Calculation of buffer address: reg0 = saved_inst_reg_rvv + reg2
    emit_set_reg(ctx, reg0, (uintptr_t)saved_inst_reg_rvv);
    emit_riscv_add(ctx, reg0, reg0, reg2);  // reg0 = buffer base + offset
    
    // Save PC, instruction, and registers to buffer
    emit_rvv_dump_inst_to_buf(ctx, pc, insn, reg0, reg2);
    
    // Increment index
    emit_riscv_addi(ctx, reg1, reg1, 1);

    // Restore index address
    emit_riscv_ld_off(ctx, reg0, X_SP, 0);
    emit_riscv_sd_off(ctx, reg1, reg0, 0);
    emit_riscv_addi(ctx, X_SP, X_SP, 8);
    
    //TODO new idea. Have a new variable called number_entries inside the buffer and increment it each time. 
    // if the number is greater then INST_REG_MAX_ENTRIES, flush the buffer and reset the number to 0
    // Then we cna use vtype_preserve function to call the printing function so this block executes 
    // each time and printing is only done inside the vtype_preserve function.

    //TODO This is the part that I think is broken and produces Illigal instruction error
    // Check if buffer is full and flush if needed
    // We want to flush when: current_index >= INST_REG_MAX_ENTRIES
    // Compare reg1 (current index) with INST_REG_MAX_ENTRIES
    emit_set_reg(ctx, reg2, (uintptr_t)INST_REG_MAX_ENTRIES);
    emit_riscv_sltu(ctx, reg3, reg1, reg2);  // reg3 = (current_index < MAX_ENTRIES) ? 1 : 0
    // reg3 == 1 means buffer is NOT full
    // reg3 == 0 means buffer IS full
    
    // Reserve branch location to skip flush if buffer is not full (reg3 == 1)
    mambo_branch skip_flush;
    int ret = mambo_reserve_branch_cbz(ctx, &skip_flush);
    if (ret == 0) {
        // Use cbnz (branch if NOT zero) to skip when reg3 == 1
        emit_local_branch_cbnz(ctx, &skip_flush, reg3);
        
        // Only reached if buffer is full (reg3 == 0, meaning index >= MAX_ENTRIES)
        emit_rvv_print_vtype_preserve(ctx, reg2, reg3);
        
    } else {
        // Branch reservation failed - fallback: always flush (safe but may be inefficient)
        // This should never happen in normal operation, but we handle it gracefully
        emit_rvv_print_vtype_preserve(ctx, reg2, reg3);
    }
    
    // Restore mask
    emit_pop(ctx, (1 << reg0) | (1 << reg1) | (1 << reg2) | (1 << reg3));
    
    return 0;
}

__attribute__((constructor)) static void vector_counter_init(void) {
    FILE *fp = fopen("vector_trace.json", "w");
    if (fp == NULL) {
        fprintf(stderr, "Warning: Failed to create/truncate vector_trace.json\n");
    } else {
        fprintf(fp, "[\n");
        fclose(fp);
    }
    
    saved_inst_reg_index = 0;
    
    mambo_context *ctx = mambo_register_plugin();
    int set_cb = mambo_register_post_inst_cb(ctx, &vector_post_inst_cb);
    assert(set_cb == MAMBO_SUCCESS);
}

// Prints count at exit and finalizes JSON
__attribute__((destructor)) static void vector_counter_fini(void) {
    fprintf(stderr, "[vector_counter] total vector instructions executed: %" PRIu64 "\n",
            vector_inst_count);
            
    finalize_json();
    
    FILE *fp = fopen("vector_trace.json", "a");
    if (fp != NULL) {
        fprintf(fp, "]\n");
        fclose(fp);
    }
}
