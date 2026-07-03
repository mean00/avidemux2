#include <check.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

/* AV_INPUT_BUFFER_PADDING_SIZE as defined in FFmpeg/libav */
#define AV_INPUT_BUFFER_PADDING_SIZE 64

/*
 * Security invariant: Any allocation size derived from parsed stream data
 * must not overflow when combined with padding constants, and must not
 * result in a smaller allocation than the data being copied into it.
 *
 * Specifically:
 *   1. originalNalSize + AV_INPUT_BUFFER_PADDING_SIZE must not wrap around
 *   2. 16 + sei_size + 1 must not wrap around
 *   3. (tail - head) + AV_INPUT_BUFFER_PADDING_SIZE must not wrap around
 *   4. All sizes must be within reasonable bounds before allocation
 */

/* Maximum reasonable NAL/SEI size: 256 MB */
#define MAX_REASONABLE_NAL_SIZE  (256 * 1024 * 1024)
#define MAX_REASONABLE_SEI_SIZE  (256 * 1024 * 1024)

/* Simulate the allocation size calculation for originalNalSize */
static int check_nal_alloc_safe(uint32_t originalNalSize) {
    /* Check for integer overflow: originalNalSize + AV_INPUT_BUFFER_PADDING_SIZE */
    if (originalNalSize > (UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE)) {
        return 0; /* Would overflow */
    }
    uint32_t alloc_size = originalNalSize + AV_INPUT_BUFFER_PADDING_SIZE;
    /* Check for unreasonable size */
    if (alloc_size > MAX_REASONABLE_NAL_SIZE) {
        return 0; /* Unreasonably large */
    }
    return 1; /* Safe */
}

/* Simulate the allocation size calculation for sei_size */
static int check_sei_alloc_safe(uint32_t sei_size) {
    /* Check for integer overflow: 16 + sei_size + 1 */
    if (sei_size > (UINT32_MAX - 16 - 1)) {
        return 0; /* Would overflow */
    }
    uint32_t alloc_size = 16 + sei_size + 1;
    if (alloc_size > MAX_REASONABLE_SEI_SIZE) {
        return 0; /* Unreasonably large */
    }
    return 1; /* Safe */
}

/* Simulate the allocation size calculation for tail-head range */
static int check_range_alloc_safe(uint32_t head, uint32_t tail) {
    /* tail must be >= head */
    if (tail < head) {
        return 0; /* Invalid range */
    }
    uint32_t range = tail - head;
    /* Check for integer overflow: range + AV_INPUT_BUFFER_PADDING_SIZE */
    if (range > (UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE)) {
        return 0; /* Would overflow */
    }
    uint32_t alloc_size = range + AV_INPUT_BUFFER_PADDING_SIZE;
    if (alloc_size > MAX_REASONABLE_NAL_SIZE) {
        return 0; /* Unreasonably large */
    }
    return 1; /* Safe */
}

/* Test that adversarial NAL sizes are rejected before allocation */
START_TEST(test_nal_size_overflow_rejected)
{
    /* Invariant: NAL sizes that would cause integer overflow when combined
     * with AV_INPUT_BUFFER_PADDING_SIZE must be detected and rejected */

    uint32_t adversarial_nal_sizes[] = {
        UINT32_MAX,
        UINT32_MAX - 1,
        UINT32_MAX - (AV_INPUT_BUFFER_PADDING_SIZE - 1),
        UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE,  /* boundary: exactly overflows */
        0xFFFFFF00,
        0xFFFFFFC0,
        0xFFFFFFBF,  /* UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE + 1 */
        0x80000000,
        0x7FFFFFFF,
        (uint32_t)(MAX_REASONABLE_NAL_SIZE + 1),
        (uint32_t)(MAX_REASONABLE_NAL_SIZE + 1000),
    };

    int num_sizes = sizeof(adversarial_nal_sizes) / sizeof(adversarial_nal_sizes[0]);

    for (int i = 0; i < num_sizes; i++) {
        uint32_t size = adversarial_nal_sizes[i];

        /* If the size would overflow, it must be detected as unsafe */
        if (size > (UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE)) {
            int safe = check_nal_alloc_safe(size);
            ck_assert_msg(safe == 0,
                "NAL size 0x%08X would cause integer overflow but was not rejected",
                size);
        }

        /* If the resulting allocation would be unreasonably large, reject it */
        if (size <= (UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE)) {
            uint32_t alloc_size = size + AV_INPUT_BUFFER_PADDING_SIZE;
            if (alloc_size > MAX_REASONABLE_NAL_SIZE) {
                int safe = check_nal_alloc_safe(size);
                ck_assert_msg(safe == 0,
                    "NAL alloc size 0x%08X is unreasonably large but was not rejected",
                    alloc_size);
            }
        }
    }
}
END_TEST

/* Test that adversarial SEI sizes are rejected before allocation */
START_TEST(test_sei_size_overflow_rejected)
{
    /* Invariant: SEI sizes that would cause integer overflow when combined
     * with the 16+1 overhead must be detected and rejected */

    uint32_t adversarial_sei_sizes[] = {
        UINT32_MAX,
        UINT32_MAX - 1,
        UINT32_MAX - 16,
        UINT32_MAX - 17,
        UINT32_MAX - 16 - 1,  /* boundary */
        0xFFFFFFE0,
        0xFFFFFFF0,
        0x80000000,
        0xFFFFFFF0,
        (uint32_t)(MAX_REASONABLE_SEI_SIZE + 1),
        (uint32_t)(MAX_REASONABLE_SEI_SIZE + 500),
    };

    int num_sizes = sizeof(adversarial_sei_sizes) / sizeof(adversarial_sei_sizes[0]);

    for (int i = 0; i < num_sizes; i++) {
        uint32_t sei_size = adversarial_sei_sizes[i];

        /* If the size would overflow 16 + sei_size + 1, must be rejected */
        if (sei_size > (UINT32_MAX - 16 - 1)) {
            int safe = check_sei_alloc_safe(sei_size);
            ck_assert_msg(safe == 0,
                "SEI size 0x%08X would cause integer overflow but was not rejected",
                sei_size);
        }
    }
}
END_TEST

/* Test that adversarial head/tail pairs are rejected before allocation */
START_TEST(test_range_alloc_overflow_rejected)
{
    /* Invariant: head/tail pairs that would cause integer overflow or
     * invalid ranges must be detected and rejected */

    struct { uint32_t head; uint32_t tail; } adversarial_ranges[] = {
        /* tail < head: invalid range */
        { 100,        0          },
        { UINT32_MAX, 0          },
        { 1000,       500        },
        /* overflow cases: tail - head is huge */
        { 0,          UINT32_MAX },
        { 0,          UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE + 1 },
        { 0,          UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE     },
        { 1,          UINT32_MAX },
        { 0,          0xFFFFFFC1 },
        /* unreasonably large range */
        { 0,          (uint32_t)(MAX_REASONABLE_NAL_SIZE + 1) },
        { 100,        (uint32_t)(MAX_REASONABLE_NAL_SIZE + 200) },
    };

    int num_ranges = sizeof(adversarial_ranges) / sizeof(adversarial_ranges[0]);

    for (int i = 0; i < num_ranges; i++) {
        uint32_t head = adversarial_ranges[i].head;
        uint32_t tail = adversarial_ranges[i].tail;

        /* Invalid range: tail < head must be rejected */
        if (tail < head) {
            int safe = check_range_alloc_safe(head, tail);
            ck_assert_msg(safe == 0,
                "Range [head=0x%08X, tail=0x%08X] is invalid (tail < head) but was not rejected",
                head, tail);
            continue;
        }

        uint32_t range = tail - head;
        /* Overflow check */
        if (range > (UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE)) {
            int safe = check_range_alloc_safe(head, tail);
            ck_assert_msg(safe == 0,
                "Range 0x%08X would cause integer overflow but was not rejected",
                range);
        }
    }
}
END_TEST

/* Test that safe/valid sizes pass the checks */
START_TEST(test_valid_sizes_accepted)
{
    /* Invariant: Legitimate small sizes must not be incorrectly rejected */

    uint32_t valid_nal_sizes[] = {
        0, 1, 100, 1024, 65536, 1048576,
        (uint32_t)(MAX_REASONABLE_NAL_SIZE - AV_INPUT_BUFFER_PADDING_SIZE - 1),
    };

    int num_valid = sizeof(valid_nal_sizes) / sizeof(valid_nal_sizes[0]);
    for (int i = 0; i < num_valid; i++) {
        uint32_t size = valid_nal_sizes[i];
        int safe = check_nal_alloc_safe(size);
        ck_assert_msg(safe == 1,
            "Valid NAL size %u was incorrectly rejected", size);
    }

    uint32_t valid_sei_sizes[] = {
        0, 1, 100, 1024, 65536, 1048576,
    };

    int num_valid_sei = sizeof(valid_sei_sizes) / sizeof(valid_sei_sizes[0]);
    for (int i = 0; i < num_valid_sei; i++) {
        uint32_t size = valid_sei_sizes[i];
        int safe = check_sei_alloc_safe(size);
        ck_assert_msg(safe == 1,
            "Valid SEI size %u was incorrectly rejected", size);
    }

    struct { uint32_t head; uint32_t tail; } valid_ranges[] = {
        { 0,    0    },
        { 0,    100  },
        { 100,  200  },
        { 0,    65536 },
        { 1000, 1048576 },
    };

    int num_valid_ranges = sizeof(valid_ranges) / sizeof(valid_ranges[0]);
    for (int i = 0; i < num_valid_ranges; i++) {
        uint32_t head = valid_ranges[i].head;
        uint32_t tail = valid_ranges[i].tail;
        int safe = check_range_alloc_safe(head, tail);
        ck_assert_msg(safe == 1,
            "Valid range [head=%u, tail=%u] was incorrectly rejected", head, tail);
    }
}
END_TEST

/* Test that allocation size after overflow check is consistent with actual malloc */
START_TEST(test_allocation_size_consistency)
{
    /* Invariant: When a size passes the overflow check, the resulting
     * allocation must be at least as large as the original size */

    uint32_t test_sizes[] = {
        0, 1, 7, 8, 15, 16, 63, 64, 127, 128,
        255, 256, 1023, 1024, 4095, 4096,
        65535, 65536, 1048575, 1048576,
    };

    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);

    for (int i = 0; i < num_sizes; i++) {
        uint32_t size = test_sizes[i];

        if (check_nal_alloc_safe(size)) {
            uint32_t alloc_size = size + AV_INPUT_BUFFER_PADDING_SIZE;
            /* The allocation size must be >= original size (no wraparound) */
            ck_assert_msg(alloc_size >= size,
                "Allocation size %u is smaller than original size %u (overflow!)",
                alloc_size, size);
            ck_assert_msg(alloc_size >= AV_INPUT_BUFFER_PADDING_SIZE,
                "Allocation size %u is smaller than padding size (overflow!)",
                alloc_size);

            /* Actually attempt the allocation to verify it succeeds */
            void *buf = malloc(alloc_size);
            if (alloc_size > 0 && alloc_size <= 1024 * 1024) {
                /* For small sizes, malloc should succeed */
                ck_assert_msg(buf != NULL,
                    "malloc(%u) failed for a reasonable size", alloc_size);
                if (buf) {
                    /* Verify we can write to the full allocation */
                    memset(buf, 0, alloc_size);
                    free(buf);
                }
            } else if (buf) {
                free(buf);
            }
        }
    }
}
END_TEST

/* Test boundary values around the overflow threshold */
START_TEST(test_overflow_boundary_values)
{
    /* Invariant: Values at and just past the overflow boundary must be
     * correctly classified */

    /* For NAL size: overflow occurs when size > UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE */
    uint32_t overflow_threshold = UINT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE;

    /* Just below threshold: should be safe (if within reasonable bounds) */
    /* Just at threshold: size + padding = UINT32_MAX, which is huge but no overflow */
    /* Just above threshold: size + padding wraps around */

    /* Values that MUST be rejected (overflow) */
    uint32_t must_reject[] = {
        overflow_threshold + 1,
        overflow_threshold + 2,
        overflow_threshold + AV_INPUT_BUFFER_PADDING_SIZE,
        UINT32_MAX,
    };

    for (int i = 0; i < (int)(sizeof(must_reject)/sizeof(must_reject[0])); i++) {
        uint32_t size = must_reject[i];
        /* These would overflow, so check_nal_alloc_safe must return 0 */
        int safe = check_nal_alloc_safe(size);
        ck_assert_msg(safe == 0,
            "Size 0x%08X