#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Small signal parameters to limit instruction count
#define SIGNAL_LENGTH 64      // Small signal (was 1024)
#define FILTER_TAPS   8       // Small filter (was 32)
#define NUM_CHANNELS  2       // Only 2 channels (was 8)

// Simple FIR filter coefficients (8 taps, Q15 format)
static const int16_t filter_coef[FILTER_TAPS] = {
    1024, 2048, 3072, 4096, 4096, 3072, 2048, 1024
};

// Signal buffers
int16_t input_signal[NUM_CHANNELS][SIGNAL_LENGTH];
int16_t filtered_output[NUM_CHANNELS][SIGNAL_LENGTH];
int32_t channel_energy[NUM_CHANNELS];

// Initialize test signal
void init_signal(void) {
    
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        for (int i = 0; i < SIGNAL_LENGTH; i++) {
            // Simple sawtooth pattern
            input_signal[ch][i] = (int16_t)((i * 512 + ch * 1024) & 0x7FFF);
        }
    }
}

void rvv_fir_filter(void) {
    
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        
        const int16_t *input = input_signal[ch];
        int16_t *output = filtered_output[ch];
        
        // Process samples
        for (int n = FILTER_TAPS; n < SIGNAL_LENGTH; ) {
            size_t vl;
            
            // CSR change: Configure for 16-bit elements, LMUL=2
            asm volatile (
                "vsetvli %0, %1, e16, m2, ta, ma\n"
                : "=r" (vl)
                : "r" (SIGNAL_LENGTH - n)
            );
            
            if (vl == 0) break;
            
            // Initialize accumulator (32-bit)
            // CSR change: e32, m4
            asm volatile (
                "vsetvli zero, %0, e32, m4, ta, ma\n"
                "vmv.v.i v8, 0\n"
                :
                : "r" (vl)
            );
            
            // Apply FIR: y[n] = sum(h[k] * x[n-k])
            for (int k = 0; k < FILTER_TAPS; k++) {
                const int16_t *x_ptr = &input[n - k];
                
                // Load input samples
                // CSR change: e16, m2
                asm volatile (
                    "vsetvli zero, %1, e16, m2, ta, ma\n"
                    "vle16.v v0, (%0)\n"
                    :
                    : "r" (x_ptr), "r" (vl)
                    : "memory"
                );
                
                // Multiply-accumulate: widening multiply
                asm volatile (
                    "vsetvli zero, %1, e16, m2, ta, ma\n"
                    "vwmacc.vx v8, %0, v0\n"
                    :
                    : "r" ((long)filter_coef[k]), "r" (vl)
                    : "memory"
                );
            }
            
            // Scale down from Q30 to Q15
            // CSR change: e32, m4
            asm volatile (
                "vsetvli zero, %1, e32, m4, ta, ma\n"
                "vsra.vi v8, v8, 15\n"
                :
                : "r" (&output[n]), "r" (vl)
            );
            
            // Narrow to 16-bit and store
            // CSR change: e16, m2
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                "vnclip.wi v0, v8, 0\n"
                "vse16.v v0, (%0)\n"
                :
                : "r" (&output[n]), "r" (vl)
                : "memory"
            );
            
            n += vl;
        }
        
        // Zero initial samples
        memset(output, 0, FILTER_TAPS * sizeof(int16_t));
    }
    
    //printf("Filtering completed.\n");
}

void rvv_energy_normalize(void) {
    
    // Compute energy for each channel
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        
        int16_t *signal = filtered_output[ch];
        int64_t energy = 0;
        
        for (int i = 0; i < SIGNAL_LENGTH; ) {
            size_t vl;
            
            // CSR change: e16, m2
            asm volatile (
                "vsetvli %0, %1, e16, m2, ta, ma\n"
                : "=r" (vl)
                : "r" (SIGNAL_LENGTH - i)
            );
            
            if (vl == 0) break;
            
            // Load samples
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                "vle16.v v0, (%0)\n"
                :
                : "r" (&signal[i]), "r" (vl)
                : "memory"
            );
            
            // Square samples: widening multiply
            asm volatile (
                "vsetvli zero, %0, e16, m2, ta, ma\n"
                "vwmul.vv v8, v0, v0\n"
                :
                : "r" (vl)
            );
            
            // Reduce sum
            // CSR change: e32, m4
            int32_t chunk_energy;
            asm volatile (
                "vsetvli zero, %1, e32, m4, ta, ma\n"
                "vmv.s.x v12, zero\n"
                "vredsum.vs v12, v8, v12\n"
                "vmv.x.s %0, v12\n"
                : "=r" (chunk_energy)
                : "r" (vl)
            );
            
            energy += chunk_energy;
            i += vl;
        }
        
        channel_energy[ch] = (int32_t)(energy / SIGNAL_LENGTH);
    }
    
    // Find max energy for normalization
    int32_t max_energy = channel_energy[0];
    for (int ch = 1; ch < NUM_CHANNELS; ch++) {
        if (channel_energy[ch] > max_energy) {
            max_energy = channel_energy[ch];
        }
    }
    
    if (max_energy == 0) max_energy = 1;
    
    // Normalize all channels
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        int16_t *signal = filtered_output[ch];
        int32_t scale = (32767 * 1024) / max_energy;
        
        for (int i = 0; i < SIGNAL_LENGTH; ) {
            size_t vl;
            
            // CSR change: e16, m2
            asm volatile (
                "vsetvli %0, %1, e16, m2, ta, ma\n"
                : "=r" (vl)
                : "r" (SIGNAL_LENGTH - i)
            );
            
            if (vl == 0) break;
            
            // Load samples
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                "vle16.v v0, (%0)\n"
                :
                : "r" (&signal[i]), "r" (vl)
                : "memory"
            );
            
            // Scale: widen, multiply, shift, narrow
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                "vwmul.vx v8, v0, %0\n"
                :
                : "r" ((long)scale), "r" (vl)
            );
            
            // CSR change: e32, m4
            asm volatile (
                "vsetvli zero, %0, e32, m4, ta, ma\n"
                "vsra.vi v8, v8, 10\n"
                :
                : "r" (vl)
            );
            
            // Narrow back
            // CSR change: e16, m2
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                "vnclip.wi v0, v8, 0\n"
                "vse16.v v0, (%0)\n"
                :
                : "r" (&signal[i]), "r" (vl)
                : "memory"
            );
            
            i += vl;
        }
    }
    
}

// Print results
void print_results(void) {
    printf("\n=== Results ===\n");
    
    printf("Channel Energies:\n");
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        printf("  Channel %d: %d\n", ch, channel_energy[ch]);
    }
    
    printf("\nFirst 16 filtered samples (Channel 0):\n");
    for (int i = FILTER_TAPS; i < FILTER_TAPS + 16 && i < SIGNAL_LENGTH; i++) {
        printf("%6d ", filtered_output[0][i]);
    }
    printf("\n");
}

int main(void) {

    init_signal();
    
    // LOOP 1: FIR filtering
    rvv_fir_filter();
    
    // LOOP 2: Energy and normalization
    rvv_energy_normalize();
    
    print_results();
    
    return 0;
}
