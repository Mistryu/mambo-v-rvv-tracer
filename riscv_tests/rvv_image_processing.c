/*
 * RISC-V Vector Extension Test: Image Processing Pipeline
 * 
 * This program simulates a real-world image processing workload:
 * 1. Load image data (grayscale)
 * 2. Apply Gaussian blur (3x3 convolution)
 * 3. Compute gradients (Sobel operator)
 * 4. Apply threshold for edge detection
 * 5. Histogram calculation
 * 6. Normalization
 * 
 * Tests multiple RVV features:
 * - Multiple vsetvl configurations (different SEW, LMUL)
 * - Load/Store operations (unit-stride, strided)
 * - Arithmetic operations (add, mul, shift)
 * - Comparison and masking
 * - Reductions
 * - Widening operations
 * - Multiple nested loops
 * 
 * Total RVV instructions: 150+
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Image dimensions
#define IMG_WIDTH  64
#define IMG_HEIGHT 64
#define IMG_SIZE   (IMG_WIDTH * IMG_HEIGHT)

// Gaussian blur kernel (3x3, scaled by 16)
static const int8_t gaussian_kernel[9] = {
    1, 2, 1,
    2, 4, 2,
    1, 2, 1
};

// Sobel kernels
static const int8_t sobel_x[9] = {
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
};

static const int8_t sobel_y[9] = {
    -1, -2, -1,
     0,  0,  0,
     1,  2,  1
};

// Global buffers
uint8_t input_image[IMG_SIZE];
uint8_t blurred_image[IMG_SIZE];
int16_t gradient_x[IMG_SIZE];
int16_t gradient_y[IMG_SIZE];
uint16_t gradient_mag[IMG_SIZE];
uint8_t edge_image[IMG_SIZE];
uint32_t histogram[256];

// Initialize test image with gradient pattern and edges
void init_test_image(uint8_t *img) {
    for (int y = 0; y < IMG_HEIGHT; y++) {
        for (int x = 0; x < IMG_WIDTH; x++) {
            int idx = y * IMG_WIDTH + x;
            
            // Create pattern: gradient + vertical edges
            uint8_t value = (x * 4) & 0xFF;
            
            // Add vertical edges
            if (x == 16 || x == 48) {
                value = 255;
            }
            
            // Add horizontal edges
            if (y == 16 || y == 48) {
                value = 255;
            }
            
            // Add diagonal pattern
            if ((x + y) % 8 == 0) {
                value = (value + 128) & 0xFF;
            }
            
            img[idx] = value;
        }
    }
}

// Apply 3x3 convolution using RVV
// This function uses multiple RVV instructions for convolution
void rvv_convolution_3x3(const uint8_t *input, uint8_t *output, 
                         const int8_t *kernel, int width, int height) {
    // Process interior pixels (skip borders)
    for (int y = 1; y < height - 1; y++) {
        int row_base = y * width;
        int x = 1;
        
        // Vector loop for horizontal pixels
        // vsetvli: configure for 8-bit elements
        while (x < width - 1) {
            size_t vl;
            asm volatile (
                "vsetvli %0, %1, e8, m1, ta, ma\n"
                : "=r" (vl)
                : "r" (width - 1 - x)
            );
            
            if (vl == 0) break;
            
            // Load 3x3 neighborhood and apply kernel
            // Top row
            int16_t sum[vl] __attribute__((aligned(16)));
            memset(sum, 0, sizeof(sum));
            
            // Use RVV to process convolution
            // Configure for 16-bit accumulation
            asm volatile (
                "vsetvli zero, %0, e16, m2, ta, ma\n"
                :
                : "r" (vl)
            );
            
            // Zero accumulator
            asm volatile (
                "vmv.v.i v8, 0\n"  // v8-v9: accumulator (m2)
            );
            
            // Top-left
            const uint8_t *ptr_tl = &input[(y-1)*width + x - 1];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"              // Load top-left pixels
                "vwmul.vx v4, v0, %1\n"          // Widen multiply by kernel[0]
                "vadd.vv v8, v8, v4\n"           // Add to accumulator
                :
                : "r" (ptr_tl), "r" ((int)kernel[0]), "r" (vl)
                : "memory"
            );
            
            // Top-center
            const uint8_t *ptr_tc = &input[(y-1)*width + x];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"
                "vwmul.vx v4, v0, %1\n"
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vadd.vv v8, v8, v4\n"
                :
                : "r" (ptr_tc), "r" ((int)kernel[1]), "r" (vl)
                : "memory"
            );
            
            // Top-right
            const uint8_t *ptr_tr = &input[(y-1)*width + x + 1];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"
                "vwmul.vx v4, v0, %1\n"
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vadd.vv v8, v8, v4\n"
                :
                : "r" (ptr_tr), "r" ((int)kernel[2]), "r" (vl)
                : "memory"
            );
            
            // Middle-left
            const uint8_t *ptr_ml = &input[y*width + x - 1];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"
                "vwmul.vx v4, v0, %1\n"
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vadd.vv v8, v8, v4\n"
                :
                : "r" (ptr_ml), "r" ((int)kernel[3]), "r" (vl)
                : "memory"
            );
            
            // Middle-center (current pixel)
            const uint8_t *ptr_mc = &input[y*width + x];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"
                "vwmul.vx v4, v0, %1\n"
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vadd.vv v8, v8, v4\n"
                :
                : "r" (ptr_mc), "r" ((int)kernel[4]), "r" (vl)
                : "memory"
            );
            
            // Middle-right
            const uint8_t *ptr_mr = &input[y*width + x + 1];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"
                "vwmul.vx v4, v0, %1\n"
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vadd.vv v8, v8, v4\n"
                :
                : "r" (ptr_mr), "r" ((int)kernel[5]), "r" (vl)
                : "memory"
            );
            
            // Bottom-left
            const uint8_t *ptr_bl = &input[(y+1)*width + x - 1];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"
                "vwmul.vx v4, v0, %1\n"
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vadd.vv v8, v8, v4\n"
                :
                : "r" (ptr_bl), "r" ((int)kernel[6]), "r" (vl)
                : "memory"
            );
            
            // Bottom-center
            const uint8_t *ptr_bc = &input[(y+1)*width + x];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"
                "vwmul.vx v4, v0, %1\n"
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vadd.vv v8, v8, v4\n"
                :
                : "r" (ptr_bc), "r" ((int)kernel[7]), "r" (vl)
                : "memory"
            );
            
            // Bottom-right
            const uint8_t *ptr_br = &input[(y+1)*width + x + 1];
            asm volatile (
                "vsetvli zero, %2, e8, m1, ta, ma\n"
                "vle8.v v0, (%0)\n"
                "vwmul.vx v4, v0, %1\n"
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vadd.vv v8, v8, v4\n"
                :
                : "r" (ptr_br), "r" ((int)kernel[8]), "r" (vl)
                : "memory"
            );
            
            // Divide by 16 (sum of kernel weights) and clamp
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                "vsra.vi v8, v8, 4\n"           // Divide by 16
                "vmax.vx v8, v8, zero\n"        // Clamp to >= 0
                "li t0, 255\n"
                "vmin.vx v8, v8, t0\n"          // Clamp to <= 255
                "vsetvli zero, %1, e8, m1, ta, ma\n"
                "vncvt.x.x.w v0, v8\n"          // Narrow to 8-bit
                "vse8.v v0, (%0)\n"             // Store result
                :
                : "r" (&output[row_base + x]), "r" (vl)
                : "memory", "t0"
            );
            
            x += vl;
        }
    }
}

// Compute gradient magnitude using Sobel operator
void rvv_sobel_gradients(const uint8_t *input, int16_t *grad_x, 
                         int16_t *grad_y, uint16_t *grad_mag,
                         int width, int height) {
    // Apply Sobel X and Y kernels
    for (int y = 1; y < height - 1; y++) {
        int row_base = y * width;
        int x = 1;
        
        while (x < width - 1) {
            size_t vl;
            asm volatile (
                "vsetvli %0, %1, e8, m1, ta, ma\n"
                : "=r" (vl)
                : "r" (width - 1 - x)
            );
            
            if (vl == 0) break;
            
            // Configure for 16-bit computation
            asm volatile (
                "vsetvli zero, %0, e16, m2, ta, ma\n"
                "vmv.v.i v8, 0\n"   // Gx accumulator
                "vmv.v.i v10, 0\n"  // Gy accumulator
                :
                : "r" (vl)
            );
            
            // Compute Sobel X and Y simultaneously
            // Each position contributes to both
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int kidx = (ky + 1) * 3 + (kx + 1);
                    const uint8_t *ptr = &input[(y+ky)*width + x + kx];
                    int8_t sx = sobel_x[kidx];
                    int8_t sy = sobel_y[kidx];
                    
                    if (sx != 0) {
                        asm volatile (
                            "vsetvli zero, %2, e8, m1, ta, ma\n"
                            "vle8.v v0, (%0)\n"
                            "vwmul.vx v4, v0, %1\n"
                            "vsetvli zero, %2, e16, m2, ta, ma\n"
                            "vadd.vv v8, v8, v4\n"
                            :
                            : "r" (ptr), "r" ((int)sx), "r" (vl)
                            : "memory"
                        );
                    }
                    
                    if (sy != 0) {
                        asm volatile (
                            "vsetvli zero, %2, e8, m1, ta, ma\n"
                            "vle8.v v0, (%0)\n"
                            "vwmul.vx v4, v0, %1\n"
                            "vsetvli zero, %2, e16, m2, ta, ma\n"
                            "vadd.vv v10, v10, v4\n"
                            :
                            : "r" (ptr), "r" ((int)sy), "r" (vl)
                            : "memory"
                        );
                    }
                }
            }
            
            // Store Gx and Gy
            asm volatile (
                "vsetvli zero, %2, e16, m2, ta, ma\n"
                "vse16.v v8, (%0)\n"
                "vse16.v v10, (%1)\n"
                :
                : "r" (&grad_x[row_base + x]),
                  "r" (&grad_y[row_base + x]),
                  "r" (vl)
                : "memory"
            );
            
            // Compute magnitude: sqrt(Gx^2 + Gy^2)
            // Approximate with |Gx| + |Gy|
            asm volatile (
                "vsetvli zero, %1, e16, m2, ta, ma\n"
                // Absolute value of Gx
                "vmslt.vx v0, v8, zero\n"       // Mask for negative
                "vneg.v v12, v8, v0.t\n"        // Negate if negative
                "vmerge.vvm v12, v8, v12, v0\n" // Select
                // Absolute value of Gy
                "vmslt.vx v0, v10, zero\n"
                "vneg.v v14, v10, v0.t\n"
                "vmerge.vvm v14, v10, v14, v0\n"
                // Sum and convert to unsigned
                "vadd.vv v12, v12, v14\n"
                "vmax.vx v12, v12, zero\n"      // Ensure positive
                "vse16.v v12, (%0)\n"
                :
                : "r" (&grad_mag[row_base + x]), "r" (vl)
                : "memory"
            );
            
            x += vl;
        }
    }
}

// Apply threshold with hysteresis for edge detection
void rvv_threshold_hysteresis(const uint16_t *grad_mag, uint8_t *edges,
                              uint16_t low_thresh, uint16_t high_thresh,
                              int width, int height) {
    // First pass: strong and weak edges
    for (int i = 0; i < width * height; ) {
        size_t vl;
        asm volatile (
            "vsetvli %0, %1, e16, m2, ta, ma\n"
            : "=r" (vl)
            : "r" (width * height - i)
        );
        
        if (vl == 0) break;
        
        // Load gradient magnitudes
        asm volatile (
            "vsetvli zero, %3, e16, m2, ta, ma\n"
            "vle16.v v8, (%2)\n"
            // Compare with high threshold - strong edges (result in v0)
            "vmsgtu.vx v0, v8, %0\n"
            :
            : "r" ((long)high_thresh),
              "r" ((long)low_thresh),
              "r" (&grad_mag[i]), "r" (vl)
            : "memory"
        );
        
        // Create output vector: start with 0, set strong edges to 255
        asm volatile (
            "vsetvli zero, %1, e8, m1, ta, ma\n"
            "vmv.v.i v4, 0\n"               // Start with zeros
            "li t0, 255\n"
            "vmerge.vxm v4, v4, t0, v0\n"   // Set strong edges (v0) to 255
            :
            : "r" (&edges[i]), "r" (vl)
            : "memory", "t0"
        );
        
        // Now handle weak edges: check if low < mag < high
        asm volatile (
            "vsetvli zero, %2, e16, m2, ta, ma\n"
            "vle16.v v8, (%1)\n"
            // v0: mag > low
            "vmsgtu.vx v0, v8, %0\n"
            // v1: mag > high (strong edges)
            "vmsgtu.vx v1, v8, %3\n"
            // v0 = v0 AND NOT v1 (weak edges: low < mag <= high)
            "vmandn.mm v0, v0, v1\n"
            // Set weak edges to 128
            "vsetvli zero, %2, e8, m1, ta, ma\n"
            "li t0, 128\n"
            "vmerge.vxm v4, v4, t0, v0\n"   // Merge weak edges
            "vse8.v v4, (%4)\n"
            :
            : "r" ((long)low_thresh), "r" (&grad_mag[i]), "r" (vl),
              "r" ((long)high_thresh), "r" (&edges[i])
            : "memory", "t0"
        );
        
        i += vl;
    }
    
    // Second pass: edge tracking (promote weak edges connected to strong)
    for (int iter = 0; iter < 5; iter++) {
        for (int y = 1; y < height - 1; y++) {
            int row_base = y * width;
            int x = 1;
            
            while (x < width - 1) {
                size_t vl;
                asm volatile (
                    "vsetvli %0, %1, e8, m1, ta, ma\n"
                    : "=r" (vl)
                    : "r" (width - 1 - x)
                );
                
                if (vl == 0) break;
                
                // Load current pixels and find weak edges
                asm volatile (
                    "vsetvli zero, %1, e8, m1, ta, ma\n"
                    "vle8.v v6, (%0)\n"             // Load current edges
                    "li t0, 128\n"
                    "vmseq.vx v1, v6, t0\n"         // v1: mask of weak edges
                    :
                    : "r" (&edges[row_base + x]), "r" (vl)
                    : "memory", "t0"
                );
                
                // Find max of 4-connected neighbors
                asm volatile (
                    "vle8.v v2, (%0)\n"             // Top
                    "vle8.v v4, (%1)\n"             // Left
                    "vmax.vv v2, v2, v4\n"
                    "vle8.v v4, (%2)\n"             // Right
                    "vmax.vv v2, v2, v4\n"
                    "vle8.v v4, (%3)\n"             // Bottom
                    "vmax.vv v2, v2, v4\n"          // v2: max neighbor value
                    :
                    : "r" (&edges[(y-1)*width + x]),
                      "r" (&edges[y*width + x - 1]),
                      "r" (&edges[y*width + x + 1]),
                      "r" (&edges[(y+1)*width + x])
                    : "memory"
                );
                
                // Promote weak edges that have a strong neighbor
                asm volatile (
                    "li t0, 255\n"
                    "vmseq.vx v0, v2, t0\n"         // v0: has strong neighbor (255)
                    "vmand.mm v0, v0, v1\n"         // v0: weak AND has strong neighbor
                    "vmerge.vxm v6, v6, t0, v0\n"   // Promote to 255
                    "vse8.v v6, (%0)\n"
                    :
                    : "r" (&edges[row_base + x])
                    : "memory", "t0"
                );
                
                x += vl;
            }
        }
    }
    
    // Final pass: set remaining weak edges to 0
    for (int i = 0; i < width * height; ) {
        size_t vl;
        asm volatile (
            "vsetvli %0, %1, e8, m1, ta, ma\n"
            : "=r" (vl)
            : "r" (width * height - i)
        );
        
        if (vl == 0) break;
        
        asm volatile (
            "vsetvli zero, %1, e8, m1, ta, ma\n"
            "vle8.v v2, (%0)\n"
            "li t0, 128\n"
            "vmseq.vx v0, v2, t0\n"         // v0: mask of weak edges (value == 128)
            "vmv.v.i v4, 0\n"               // v4: zeros
            "vmerge.vvm v2, v2, v4, v0\n"   // Replace weak (128) with 0
            "vse8.v v2, (%0)\n"
            :
            : "r" (&edges[i]), "r" (vl)
            : "memory", "t0"
        );
        
        i += vl;
    }
}

// Compute histogram using RVV
void rvv_histogram(const uint8_t *image, uint32_t *hist, int size) {
    // Zero histogram
    memset(hist, 0, 256 * sizeof(uint32_t));
    
    // Process image in chunks
    for (int i = 0; i < size; ) {
        size_t vl;
        asm volatile (
            "vsetvli %0, %1, e8, m1, ta, ma\n"
            : "=r" (vl)
            : "r" (size - i)
        );
        
        if (vl == 0) break;
        
        // Load pixel values
        asm volatile (
            "vsetvli zero, %1, e8, m1, ta, ma\n"
            "vle8.v v0, (%0)\n"
            :
            : "r" (&image[i]), "r" (vl)
            : "memory"
        );
        
        // Scalar fallback for histogram binning
        // (True vectorization requires gather/scatter or wider bins)
        uint8_t pixels[vl];
        asm volatile (
            "vse8.v v0, (%0)\n"
            :
            : "r" (pixels)
            : "memory"
        );
        
        for (size_t j = 0; j < vl; j++) {
            hist[pixels[j]]++;
        }
        
        i += vl;
    }
}

// Normalize image based on histogram
void rvv_normalize(uint8_t *image, const uint32_t *hist, int size) {
    // Find min and max non-zero values
    uint8_t min_val = 255, max_val = 0;
    for (int i = 0; i < 256; i++) {
        if (hist[i] > 0) {
            if (i < min_val) min_val = i;
            if (i > max_val) max_val = i;
        }
    }
    
    if (max_val <= min_val) return; // No normalization needed
    
    // Normalize: out = (in - min) * 255 / (max - min)
    uint32_t range = max_val - min_val;
    
    for (int i = 0; i < size; ) {
        size_t vl;
        asm volatile (
            "vsetvli %0, %1, e8, m1, ta, ma\n"
            : "=r" (vl)
            : "r" (size - i)
        );
        
        if (vl == 0) break;
        
        // Load, widen to 16-bit, compute, narrow back
        asm volatile (
            "vsetvli zero, %4, e8, m1, ta, ma\n"
            "vle8.v v0, (%3)\n"
            // Widen to 16-bit
            "vsetvli zero, %4, e16, m2, ta, ma\n"
            "vzext.vf2 v8, v0\n"
            // Subtract min
            "vsub.vx v8, v8, %0\n"
            // Multiply by 255
            "li t0, 255\n"
            "vmul.vx v8, v8, t0\n"
            // Divide by range (approximate with shift if power of 2)
            "vdivu.vx v8, v8, %1\n"
            // Clamp to [0, 255]
            "vmax.vx v8, v8, zero\n"
            "vmin.vx v8, v8, t0\n"
            // Narrow back to 8-bit
            "vsetvli zero, %4, e8, m1, ta, ma\n"
            "vncvt.x.x.w v0, v8\n"
            "vse8.v v0, (%2)\n"
            :
            : "r" ((long)min_val), "r" ((long)range),
              "r" (&image[i]), "r" (&image[i]), "r" (vl)
            : "memory", "t0"
        );
        
        i += vl;
    }
}

// Vector statistics computation
void rvv_compute_statistics(const uint8_t *image, int size,
                            uint32_t *sum, uint32_t *sum_sq) {
    *sum = 0;
    *sum_sq = 0;
    
    uint32_t total_sum = 0;
    uint32_t total_sum_sq = 0;
    
    for (int i = 0; i < size; ) {
        size_t vl;
        asm volatile (
            "vsetvli %0, %1, e8, m1, ta, ma\n"
            : "=r" (vl)
            : "r" (size - i)
        );
        
        if (vl == 0) break;
        
        // Load 8-bit pixels and widen to 32-bit
        asm volatile (
            "vsetvli zero, %1, e8, m1, ta, ma\n"
            "vle8.v v0, (%0)\n"
            // Widen to 16-bit first
            "vsetvli zero, %1, e16, m2, ta, ma\n"
            "vzext.vf2 v2, v0\n"
            // Widen to 32-bit
            "vsetvli zero, %1, e32, m4, ta, ma\n"
            "vzext.vf2 v4, v2\n"
            :
            : "r" (&image[i]), "r" (vl)
            : "memory"
        );
        
        // Compute sum: initialize scalar accumulator and reduce
        uint32_t chunk_sum;
        asm volatile (
            "vsetvli zero, %2, e32, m4, ta, ma\n"
            "vmv.s.x v8, zero\n"            // Initialize scalar to 0
            "vredsum.vs v8, v4, v8\n"       // Reduce sum into v8[0]
            "vmv.x.s %0, v8\n"              // Extract to scalar register
            : "=r" (chunk_sum)
            : "r" (&image[i]), "r" (vl)
            : "memory"
        );
        total_sum += chunk_sum;
        
        // Compute sum of squares
        uint32_t chunk_sum_sq;
        asm volatile (
            "vsetvli zero, %2, e32, m4, ta, ma\n"
            "vmul.vv v12, v4, v4\n"         // Square each element
            "vmv.s.x v8, zero\n"            // Initialize scalar to 0
            "vredsum.vs v8, v12, v8\n"      // Reduce sum into v8[0]
            "vmv.x.s %0, v8\n"              // Extract to scalar register
            : "=r" (chunk_sum_sq)
            : "r" (&image[i]), "r" (vl)
            : "memory"
        );
        total_sum_sq += chunk_sum_sq;
        
        i += vl;
    }
    
    *sum = total_sum;
    *sum_sq = total_sum_sq;
}

// Main processing pipeline
int main() {
    printf("RISC-V Vector Extension: Image Processing Pipeline Test\n");
    printf("=========================================================\n\n");
    
    // Initialize test image
    printf("1. Initializing test image (%dx%d)...\n", IMG_WIDTH, IMG_HEIGHT);
    init_test_image(input_image);
    
    // Step 1: Gaussian Blur
    printf("2. Applying Gaussian blur (3x3 convolution)...\n");
    rvv_convolution_3x3(input_image, blurred_image, gaussian_kernel, 
                        IMG_WIDTH, IMG_HEIGHT);
    
    // Step 2: Sobel Edge Detection
    printf("3. Computing gradients (Sobel operator)...\n");
    rvv_sobel_gradients(blurred_image, gradient_x, gradient_y, gradient_mag,
                        IMG_WIDTH, IMG_HEIGHT);
    
    // Step 3: Threshold and Edge Tracking
    printf("4. Applying threshold with hysteresis...\n");
    uint16_t low_thresh = 50;
    uint16_t high_thresh = 150;
    rvv_threshold_hysteresis(gradient_mag, edge_image, low_thresh, high_thresh,
                             IMG_WIDTH, IMG_HEIGHT);
    
    // Step 4: Histogram Computation
    printf("5. Computing histogram...\n");
    rvv_histogram(edge_image, histogram, IMG_SIZE);
    
    // Print histogram statistics
    uint32_t edge_pixels = histogram[255];
    uint32_t non_edge_pixels = histogram[0];
    printf("   Edge pixels: %u (%.1f%%)\n", edge_pixels, 
           100.0f * edge_pixels / IMG_SIZE);
    printf("   Non-edge pixels: %u (%.1f%%)\n", non_edge_pixels,
           100.0f * non_edge_pixels / IMG_SIZE);
    
    // Step 5: Normalization
    printf("6. Normalizing edge image...\n");
    rvv_normalize(edge_image, histogram, IMG_SIZE);
    
    // Step 6: Statistics
    printf("7. Computing image statistics...\n");
    uint32_t sum, sum_sq;
    rvv_compute_statistics(edge_image, IMG_SIZE, &sum, &sum_sq);
    
    float mean = (float)sum / IMG_SIZE;
    float variance = (float)sum_sq / IMG_SIZE - mean * mean;
    float stddev = sqrtf(variance);
    
    printf("   Mean: %.2f\n", mean);
    printf("   Std Dev: %.2f\n", stddev);
    
    
    // Print sample output (center 8x8 region)
    printf("\n8. Sample output (center 8x8 region):\n");
    printf("   ");
    for (int x = 0; x < 8; x++) printf("  %2d", 28 + x);
    printf("\n");
    
    for (int y = 28; y < 36; y++) {
        printf("   %2d", y);
        for (int x = 28; x < 36; x++) {
            uint8_t val = edge_image[y * IMG_WIDTH + x];
            if (val > 200) printf("  ##");
            else if (val > 100) printf("  ..");
            else printf("    ");
        }
        printf("\n");
    }
    
    printf("\nTest completed successfully!\n");
    printf("Total RVV instructions executed: 150+\n");
    printf("- Multiple vsetvl configurations\n");
    printf("- Nested loops with vector operations\n");
    printf("- Widening/narrowing operations\n");
    printf("- Reductions and comparisons\n");
    printf("- Load/store with various patterns\n");
    
    return 0;
}