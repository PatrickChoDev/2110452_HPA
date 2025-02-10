#include "neon_add.h"

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

void add_neon(int size, int *a, int *b) {
    // Ensure proper alignment
    if ((reinterpret_cast<uintptr_t>(a) & 0xF) != 0 || 
        (reinterpret_cast<uintptr_t>(b) & 0xF) != 0) {
        // Fall back to scalar addition if unaligned
        for (int i = 0; i < size; i++) {
            a[i] += b[i];
        }
        return;
    }

    int i = 0;

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    // Process in chunks of 16 integers for better throughput
    for (; i + 16 <= size; i += 16) {
        // Load 16 elements at once (4 vectors of 4 integers each)
        int32x4_t av1 = vld1q_s32(&a[i]);
        int32x4_t bv1 = vld1q_s32(&b[i]);
        int32x4_t av2 = vld1q_s32(&a[i + 4]);
        int32x4_t bv2 = vld1q_s32(&b[i + 4]);
        int32x4_t av3 = vld1q_s32(&a[i + 8]);
        int32x4_t bv3 = vld1q_s32(&b[i + 8]);
        int32x4_t av4 = vld1q_s32(&a[i + 12]);
        int32x4_t bv4 = vld1q_s32(&b[i + 12]);

        // Perform additions
        av1 = vaddq_s32(av1, bv1);
        av2 = vaddq_s32(av2, bv2);
        av3 = vaddq_s32(av3, bv3);
        av4 = vaddq_s32(av4, bv4);

        // Store results
        vst1q_s32(&a[i], av1);
        vst1q_s32(&a[i + 4], av2);
        vst1q_s32(&a[i + 8], av3);
        vst1q_s32(&a[i + 12], av4);
    }
#endif

    // Process remaining elements
    for (; i < size; i++) {
        a[i] += b[i];
    }
}