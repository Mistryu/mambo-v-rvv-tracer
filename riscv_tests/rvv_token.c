#include <stdint.h>
#include <stdio.h>

// Random-looking hardcoded seed
#define RANDOM_SEED   0xA5C3F17Bu
#define RANDOM_SEED2  0x13579BDFu

// Large loop to generate a lot of RVV instructions
#define MAIN_ITERS    20000
#define OBF_ITERS     2000

static volatile uint32_t obf_sink = 0;

// Main deterministic token computation
// Now uses full vector length (VLMAX) and keeps state across all lanes in v8.
static uint32_t compute_token_rvv(uint32_t input) {
    uint32_t token = 0;
    uint32_t state = input ^ RANDOM_SEED;
    size_t vl = 0;

    // Select max VL for e32,m1 on this machine.
    asm volatile (
        "vsetvli %0, zero, e32, m1, ta, ma\n"
        : "=r"(vl)
        :
        : "memory"
    );

    // Initialize full v8:
    //   v8[:] = state + lane_id
    asm volatile (
        "vsetvli zero, %1, e32, m1, ta, ma\n"
        "vmv.v.x v8, %0\n"
        "vid.v   v9\n"
        "vadd.vv v8, v8, v9\n"
        :
        : "r"(state), "r"(vl)
        : "memory"
    );

    // Same recurrence length, now applied to all lanes:
    // v8 = v8 * A + B + input
    for (int i = 0; i < MAIN_ITERS; i++) {
        uint32_t A = 1664525u + (uint32_t)(i & 7);
        uint32_t B = 1013904223u;

        asm volatile (
            "vsetvli zero, %3, e32, m1, ta, ma\n"
            "vmul.vx v8, v8, %0\n"
            "vadd.vx v8, v8, %1\n"
            "vadd.vx v8, v8, %2\n"
            :
            : "r"(A), "r"(B), "r"(input), "r"(vl)
            : "memory"
        );
    }

    // Reduce full vector -> scalar token (sum of lanes)
    asm volatile (
        "vsetvli zero, %1, e32, m1, ta, ma\n"
        "vmv.v.i   v10, 0\n"
        "vredsum.vs v10, v8, v10\n"
        "vmv.x.s   %0, v10\n"
        : "=r"(token)
        : "r"(vl)
        : "memory"
    );

    return token;
}

// Unrelated short obfuscation loop (does not affect token)
// Now also runs on full vector length in v24.
static void obfuscation_loop_rvv(void) {
    uint32_t x = 0x42424242u ^ RANDOM_SEED2;
    uint32_t out = 0;
    size_t vl = 0;

    asm volatile (
        "vsetvli %0, zero, e32, m1, ta, ma\n"
        : "=r"(vl)
        :
        : "memory"
    );

    // v24[:] = x + lane_id
    asm volatile (
        "vsetvli zero, %1, e32, m1, ta, ma\n"
        "vmv.v.x v24, %0\n"
        "vid.v   v25\n"
        "vadd.vv v24, v24, v25\n"
        :
        : "r"(x), "r"(vl)
        : "memory"
    );

    for (int i = 0; i < OBF_ITERS; i++) {
        uint32_t m1 = 1103515245u;
        uint32_t a1 = 12345u;
        uint32_t m2 = 3u;

        asm volatile (
            "vsetvli zero, %3, e32, m1, ta, ma\n"
            "vmul.vx v24, v24, %0\n"
            "vadd.vx v24, v24, %1\n"
            "vmul.vx v24, v24, %2\n"
            :
            : "r"(m1), "r"(a1), "r"(m2), "r"(vl)
            : "memory"
        );
    }

    // Reduce vector to scalar only for sink side-effect.
    asm volatile (
        "vsetvli zero, %1, e32, m1, ta, ma\n"
        "vmv.v.i   v26, 0\n"
        "vredsum.vs v26, v24, v26\n"
        "vmv.x.s   %0, v26\n"
        : "=r"(out)
        : "r"(vl)
        : "memory"
    );

    obf_sink = out; // prevent optimization away
}

int main(void) {
    uint32_t input;

    printf("Enter integer input: ");
    if (scanf("%u", &input) != 1) {
        printf("Invalid input.\n");
        return 1;
    }

    uint32_t token = compute_token_rvv(input);
    obfuscation_loop_rvv();

    printf("input=%u token=%u\n", input, token);

    (void)token;
    return 0;
}