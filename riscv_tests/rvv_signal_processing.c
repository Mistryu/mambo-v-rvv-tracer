#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Signal parameters
#define SIGNAL_LENGTH 1024
#define FILTER_TAPS   32
#define NUM_CHANNELS  8

// FIR filter coefficients (32 taps, Q15 format)
// Low-pass filter (cutoff at Fs/8)
static const int16_t lowpass_coef[FILTER_TAPS] = {
    -43, -89, -123, -98, 31, 245, 479, 641,
    641, 479, 245, 31, -98, -123, -89, -43,
    -43, -89, -123, -98, 31, 245, 479, 641,
    641, 479, 245, 31, -98, -123, -89, -43
};

// High-pass filter (cutoff at Fs/4)
static const int16_t highpass_coef[FILTER_TAPS] = {
    89, 123, 98, -31, -245, -479, -641, -641,
    -479, -245, -31, 98, 123, 89, 43, 0,
    0, 43, 89, 123, 98, -31, -245, -479,
    -641, -641, -479, -245, -31, 98, 123, 89
};

// Band-pass filter (Fs/8 to Fs/4)
static const int16_t bandpass_coef[FILTER_TAPS] = {
    0, 43, 89, 123, 98, -31, -245, -479,
    -641, -641, -479, -245, -31, 98, 123, 89,
    89, 123, 98, -31, -245, -479, -641, -641,
    -479, -245, -31, 98, 123, 89, 43, 0
};

// Hamming window coefficients (Q15 format)
static int16_t hamming_window[SIGNAL_LENGTH];

// Signal buffers
int16_t input_signal[NUM_CHANNELS][SIGNAL_LENGTH];
int16_t lowpass_output[NUM_CHANNELS][SIGNAL_LENGTH];
int16_t highpass_output[NUM_CHANNELS][SIGNAL_LENGTH];
int16_t bandpass_output[NUM_CHANNELS][SIGNAL_LENGTH];
int16_t windowed_signal[NUM_CHANNELS][SIGNAL_LENGTH];
int32_t energy_per_channel[NUM_CHANNELS];

// Initialize test signal with multi-frequency components
void init_test_signal(void) {
    
    // Generate Hamming window: w(n) = 0.54 - 0.46*cos(2*pi*n/(N-1))
    for (int i = 0; i < SIGNAL_LENGTH; i++) {
        double angle = 2.0 * M_PI * i / (SIGNAL_LENGTH - 1);
        double window = 0.54 - 0.46 * cos(angle);
        hamming_window[i] = (int16_t)(window * 32767.0);
    }
    
    // Generate multi-channel test signals
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        for (int i = 0; i < SIGNAL_LENGTH; i++) {
            // Mix of frequencies: fundamental + harmonics
            double t = (double)i / SIGNAL_LENGTH;
            double freq1 = 2.0 * M_PI * (ch + 1) * 4.0;  // Low frequency
            double freq2 = 2.0 * M_PI * (ch + 1) * 16.0; // Mid frequency
            double freq3 = 2.0 * M_PI * (ch + 1) * 32.0; // High frequency
            
            double signal = 0.5 * sin(freq1 * t) +
                          0.3 * sin(freq2 * t) +
                          0.2 * sin(freq3 * t);
            
            input_signal[ch][i] = (int16_t)(signal * 16384.0);
        }
    }
}

void rvv_multiband_fir_filter(void) {
    
    // Process each channel
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        
        const int16_t *input = input_signal[ch];
        int16_t *lp_out = lowpass_output[ch];
        int16_t *hp_out = highpass_output[ch];
        int16_t *bp_out = bandpass_output[ch];
        
        // Process signal in chunks
        for (int n = FILTER_TAPS; n < SIGNAL_LENGTH; ) {
            size_t vl;
            
            // Configure vector length for 16-bit elements, LMUL=2
            asm volatile (
                "vsetvli %0, %1, e16, m2, ta, ma\n"
                : "=r" (vl)
                : "r" (SIGNAL_LENGTH - n)
            );
            
            if (vl == 0) break;
            
            // Initialize accumulators (32-bit for precision)
            // CSR change: e32, m4
            asm volatile (
                "vsetvli zero, %0, e32, m4, ta, ma\n"
                "vmv.v.i v8, 0\n"   // Low-pass accumulator
                "vmv.v.i v12, 0\n"  // High-pass accumulator
                "vmv.v.i v16, 0\n"  // Band-pass accumulator
                :
                : "r" (vl)
            );
            
            // Apply FIR filter: y[n] = sum(h[k] * x[n-k])
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
                
                // Low-pass filter: multiply-accumulate
                // Widen multiply: e16 * e16 -> e32
                asm volatile (
                    "vsetvli zero, %2, e16, m2, ta, ma\n"
                    "vwmacc.vx v8, %0, v0\n"  // v8 += lowpass_coef[k] * v0
                    :
                    : "r" ((long)lowpass_coef[k]), "r" (x_ptr), "r" (vl)
                    : "memory"
                );
                
                // High-pass filter
                asm volatile (
                    "vsetvli zero, %2, e16, m2, ta, ma\n"
                    "vwmacc.vx v12, %0, v0\n"  // v12 += highpass_coef[k] * v0
                    :
                    : "r" ((long)highpass_coef[k]), "r" (x_ptr), "r" (vl)
                    : "memory"
                );
                
                // Band-pass filter
                asm volatile (
                    "vsetvli zero, %2, e16, m2, ta, ma\n"
                    "vwmacc.vx v16, %0, v0\n"  // v16 += bandpass_coef[k] * v0
                    :
                    : "r" ((long)bandpass_coef[k]), "r" (x_ptr), "r" (vl)
                    : "memory"
                );
            }
            
            // Scale down and saturate to 16-bit
            // CSR change: e32, m4
            asm volatile (
                "vsetvli zero, %3, e32, m4, ta, ma\n"
                // Shift right by 15 (Q15 scaling)
                "vsra.vi v8, v8, 15\n"
                "vsra.vi v12, v12, 15\n"
                "vsra.vi v16, v16, 15\n"
                :
                : "r" (&lp_out[n]), "r" (&hp_out[n]), "r" (&bp_out[n]), "r" (vl)
                : "memory"
            );
            
            // Narrow to 16-bit with saturation
            // CSR change: e16, m2
            asm volatile (
                "vsetvli zero, %3, e16, m2, ta, ma\n"
                "vnclip.wi v0, v8, 0\n"   // Narrow with saturation
                "vse16.v v0, (%0)\n"       // Store low-pass result
                "vnclip.wi v2, v12, 0\n"
                "vse16.v v2, (%1)\n"       // Store high-pass result
                "vnclip.wi v4, v16, 0\n"
                "vse16.v v4, (%2)\n"       // Store band-pass result
                :
                : "r" (&lp_out[n]), "r" (&hp_out[n]), "r" (&bp_out[n]), "r" (vl)
                : "memory"
            );
            
            n += vl;
        }
        
        // Zero out the initial filter delay
        memset(lp_out, 0, FILTER_TAPS * sizeof(int16_t));
        memset(hp_out, 0, FILTER_TAPS * sizeof(int16_t));
        memset(bp_out, 0, FILTER_TAPS * sizeof(int16_t));
    }
    
    //printf("Multi-band filtering completed.\n");
}

void rvv_window_and_energy(void) {
    
    // Process each channel
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        
        // Select which filtered output to window (alternate between filters)
        const int16_t *filtered;
        if (ch % 3 == 0) {
            filtered = lowpass_output[ch];
        } else if (ch % 3 == 1) {
            filtered = highpass_output[ch];
        } else {
            filtered = bandpass_output[ch];
        }
        
        int16_t *windowed = windowed_signal[ch];
        
        // Initialize energy accumulator (64-bit for large sums)
        int64_t channel_energy = 0;
        
        // Process signal in chunks
        for (int i = 0; i < SIGNAL_LENGTH; ) {
            size_t vl;
            
            // Configure for 16-bit elements, LMUL=2
            asm volatile (
                "vsetvli %0, %1, e16, m2, ta, ma\n"
                : "=r" (vl)
                : "r" (SIGNAL_LENGTH - i)
            );
            
            if (vl == 0) break;
            
            // Load filtered signal
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                "vle16.v v0, (%0)\n"
                :
                : "r" (&filtered[i]), "r" (vl)
                : "memory"
            );
            
            // Load window coefficients
            asm volatile (
                "vle16.v v2, (%0)\n"
                :
                : "r" (&hamming_window[i])
                : "memory"
            );
            
            // Multiply signal by window: windowed = signal * window (Q15)
            // Use widening multiply to 32-bit
            asm volatile (
                "vsetvli zero, %0, e16, m2, ta, ma\n"
                "vwmul.vv v8, v0, v2\n"  // v8-v11: 32-bit products (m4)
                :
                : "r" (vl)
            );
            
            // Scale down from Q30 to Q15
            // CSR change: e32, m4
            asm volatile (
                "vsetvli zero, %0, e32, m4, ta, ma\n"
                "vsra.vi v8, v8, 15\n"
                :
                : "r" (vl)
            );
            
            // Narrow back to 16-bit
            // CSR change: e16, m2
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                "vnclip.wi v4, v8, 0\n"
                "vse16.v v4, (%0)\n"
                :
                : "r" (&windowed[i]), "r" (vl)
                : "memory"
            );
            
            // Compute energy: sum of squares
            // Load windowed values for energy computation
            asm volatile (
                "vle16.v v0, (%0)\n"
                :
                : "r" (&windowed[i])
                : "memory"
            );
            
            // Square: widen to 32-bit for precision
            asm volatile (
                "vsetvli zero, %0, e16, m2, ta, ma\n"
                "vwmul.vv v8, v0, v0\n"  // v8-v11: squares (32-bit, m4)
                :
                : "r" (vl)
            );
            
            // Reduce sum for this chunk
            // CSR change: e32, m4
            int32_t chunk_energy;
            asm volatile (
                "vsetvli zero, %1, e32, m4, ta, ma\n"
                "vmv.s.x v12, zero\n"         // Initialize scalar element to 0
                "vredsum.vs v12, v8, v12\n"   // Reduce sum into v12[0]
                "vmv.x.s %0, v12\n"           // Extract to scalar
                : "=r" (chunk_energy)
                : "r" (vl)
            );
            
            channel_energy += chunk_energy;
            
            i += vl;
        }
        
        // Store energy (divide by signal length for average)
        energy_per_channel[ch] = (int32_t)(channel_energy / SIGNAL_LENGTH);
    }
    
}

// Additional processing: Compute statistics across channels
void rvv_channel_statistics(void) {
    
    // Find min and max energy across channels
    // CSR change: e32, m2
    size_t vl;
    asm volatile (
        "vsetvli %0, %1, e32, m2, ta, ma\n"
        : "=r" (vl)
        : "r" (NUM_CHANNELS)
    );
    
    // Load all channel energies
    asm volatile (
        "vsetvli zero, %1, e32, m2, ta, ma\n"
        "vle32.v v0, (%0)\n"
        :
        : "r" (energy_per_channel), "r" (NUM_CHANNELS)
        : "memory"
    );
    
    // Find minimum
    int32_t min_energy;
    asm volatile (
        "vsetvli zero, %1, e32, m2, ta, ma\n"
        "li t0, 0x7FFFFFFF\n"         // Max int32
        "vmv.s.x v2, t0\n"            // Initialize with max value
        "vredmin.vs v2, v0, v2\n"     // Reduce minimum
        "vmv.x.s %0, v2\n"
        : "=r" (min_energy)
        : "r" (NUM_CHANNELS)
        : "t0"
    );
    
    // Find maximum
    int32_t max_energy;
    asm volatile (
        "vsetvli zero, %1, e32, m2, ta, ma\n"
        "li t0, -2147483648\n"        // Min int32
        "vmv.s.x v2, t0\n"            // Initialize with min value
        "vredmax.vs v2, v0, v2\n"     // Reduce maximum
        "vmv.x.s %0, v2\n"
        : "=r" (max_energy)
        : "r" (NUM_CHANNELS)
        : "t0"
    );
    
    // Compute sum for average
    int32_t sum_energy;
    asm volatile (
        "vsetvli zero, %1, e32, m2, ta, ma\n"
        "vmv.s.x v2, zero\n"          // Initialize sum to 0
        "vredsum.vs v2, v0, v2\n"     // Reduce sum
        "vmv.x.s %0, v2\n"
        : "=r" (sum_energy)
        : "r" (NUM_CHANNELS)
    );
    
    float avg_energy = (float)sum_energy / NUM_CHANNELS;

}

// Verify results with sample outputs
void print_sample_results(void) {
    
    printf("\nChannel 0 - First 16 samples:\n");
    printf("Low-pass:  ");
    for (int i = FILTER_TAPS; i < FILTER_TAPS + 16; i++) {
        printf("%6d ", lowpass_output[0][i]);
    }
    printf("\n");
    
    printf("High-pass: ");
    for (int i = FILTER_TAPS; i < FILTER_TAPS + 16; i++) {
        printf("%6d ", highpass_output[0][i]);
    }
    printf("\n");
    
    printf("Band-pass: ");
    for (int i = FILTER_TAPS; i < FILTER_TAPS + 16; i++) {
        printf("%6d ", bandpass_output[0][i]);
    }
    printf("\n");
    
    // Print windowed output
    printf("\nWindowed (first 16 samples):\n");
    for (int i = 0; i < 16; i++) {
        printf("%6d ", windowed_signal[0][i]);
    }
    printf("\n");
}

int main(void) {
    
    // Initialize test data
    init_test_signal();
    
    // LOOP 1: Multi-band FIR filtering
    rvv_multiband_fir_filter();
    
    // LOOP 2: Window application and energy computation
    rvv_window_and_energy();
    
    // Additional statistics
    rvv_channel_statistics();
    
    // Print sample results
    //print_sample_results();
    
    return 0;
}
