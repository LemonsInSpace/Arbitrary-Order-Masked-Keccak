#include "masked_types.h"
#include "global_rng.h"
#include "stm32f4xx_hal_rng.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "masked_gadgets.h"

/**
 * Fill a randomness matrix for masked AND operations.
 *
 * Each entry r[i][j] represents a fresh 64-bit random value
 * shared symmetrically for masked gadget use.
 * Diagonal elements are zeroed.
 *
 * @param r Output 2D matrix of random 64-bit values
 */

#if MASKING_N == 1
void fill_random_matrix(uint64_t r[1][1]) {
    r[0][0] = 0;
}

#elif MASKING_N == 2
void fill_random_matrix(uint64_t r[2][2]) {
    uint64_t r01 = get_random64();

    r[0][0] = 0;
    r[0][1] = r01;  r[1][0] = r01;
    r[1][1] = 0;

#ifdef DEBUG_LOG_VERBOSE
    debug_log("Random matrix (2x2) filled with 1 random64 call.\r\n");
#endif
}

#elif MASKING_N == 3
void fill_random_matrix(uint64_t r[3][3]) {
    uint64_t r01 = get_random64();
    uint64_t r02 = get_random64();
    uint64_t r12 = get_random64();

    r[0][0] = 0;
    r[0][1] = r01;  r[1][0] = r01;
    r[0][2] = r02;  r[2][0] = r02;

    r[1][1] = 0;
    r[1][2] = r12;  r[2][1] = r12;

    r[2][2] = 0;

#ifdef DEBUG_LOG_VERBOSE
    debug_log("Random matrix (3x3) filled with 3 random64 calls.\r\n");
#endif
}

#elif MASKING_N == 4
void fill_random_matrix(uint64_t r[4][4]) {
    uint64_t r01 = get_random64();
    uint64_t r02 = get_random64();
    uint64_t r03 = get_random64();
    uint64_t r12 = get_random64();
    uint64_t r13 = get_random64();
    uint64_t r23 = get_random64();

    r[0][0] = 0;
    r[0][1] = r01;  r[1][0] = r01;
    r[0][2] = r02;  r[2][0] = r02;
    r[0][3] = r03;  r[3][0] = r03;

    r[1][1] = 0;
    r[1][2] = r12;  r[2][1] = r12;
    r[1][3] = r13;  r[3][1] = r13;

    r[2][2] = 0;
    r[2][3] = r23;  r[3][2] = r23;

    r[3][3] = 0;

#ifdef DEBUG_LOG_VERBOSE
    debug_log("Random matrix (4x4) filled with 6 random64 calls.\r\n");
#endif
}
#elif MASKING_N == 5
void fill_random_matrix(uint64_t r[5][5]) {
    uint64_t r01 = get_random64();
    uint64_t r02 = get_random64();
    uint64_t r03 = get_random64();
    uint64_t r04 = get_random64();
    uint64_t r12 = get_random64();
    uint64_t r13 = get_random64();
    uint64_t r14 = get_random64();
    uint64_t r23 = get_random64();
    uint64_t r24 = get_random64();
    uint64_t r34 = get_random64();

    r[0][0] = 0;
    r[0][1] = r01;  r[1][0] = r01;
    r[0][2] = r02;  r[2][0] = r02;
    r[0][3] = r03;  r[3][0] = r03;
    r[0][4] = r04;  r[4][0] = r04;

    r[1][1] = 0;
    r[1][2] = r12;  r[2][1] = r12;
    r[1][3] = r13;  r[3][1] = r13;
    r[1][4] = r14;  r[4][1] = r14;

    r[2][2] = 0;
    r[2][3] = r23;  r[3][2] = r23;
    r[2][4] = r24;  r[4][2] = r24;

    r[3][3] = 0;
    r[3][4] = r34;  r[4][3] = r34;

    r[4][4] = 0;

#ifdef DEBUG_LOG_VERBOSE
    debug_log("Random matrix (5x5) filled with 10 random64 calls.\r\n");
#endif
}
#else
	void fill_random_matrix(uint64_t r[MASKING_N][MASKING_N]) {
		for (size_t i = 0; i < MASKING_N; i++) {
			for (size_t j = i + 1; j < MASKING_N; j++) {
				// Each r[i][j] must be a fresh 64-bit random value for secure masking.
				// get_random64() is expected to enforce validity and invoke error handling on failure.
				uint64_t val = get_random64();
				r[i][j] = val;
				r[j][i] = val;  // Fill symmetric entry
			}
			r[i][i] = 0;  // Diagonal should be zero or ignored
		}

	#ifdef DEBUG_LOG_VERBOSE
		debug_log("Randomness matrix filled for masked AND.\r\n");
	#endif
	}
#endif

/**
 * Perform masked XOR between two values.
 *
 * Bitwise XOR is linear under Boolean masking,
 * so shares can be XORed independently.
 *
 * @param out Output share result
 * @param a First masked operand
 * @param b Second masked operand
 */
void masked_xor(masked_uint64_t *out,
                const masked_uint64_t *a,
                const masked_uint64_t *b) {
    for (size_t i = 0; i < MASKING_N; i++) {
        out->share[i] = a->share[i] ^ b->share[i];
    }
}

/**
 * Perform secure masked AND between two values.
 *
 * This is a non-linear operation and thus needs careful masking
 * using randomness and pairwise cross-terms.
 *
 * @param out Output masked result
 * @param a First masked operand
 * @param b Second masked operand
 * @param r Fresh randomness matrix r[i][j] per share-pair
 */
// Cross-terms between shares: ISW masking requires each unique pair (i,j)
// to contribute securely to the AND computation. This block is symmetric in i,j.
// NOTE: Computational cost is independent of actual share values, which prevents
// data-dependent timing variation.
#if MASKING_N == 1
void masked_and(masked_uint64_t *out,
                const masked_uint64_t *a,
                const masked_uint64_t *b,
                const uint64_t r[1][1]) {
    out->share[0] = a->share[0] & b->share[0];
}
#elif MASKING_N == 2
void masked_and(masked_uint64_t *out,
                const masked_uint64_t *a,
                const masked_uint64_t *b,
                const uint64_t r[2][2]) {
    out->share[0] = a->share[0] & b->share[0];
    out->share[1] = a->share[1] & b->share[1];

    uint64_t t01 = (a->share[0] & b->share[1]) ^ (a->share[1] & b->share[0]);
    out->share[0] ^= r[0][1];
    out->share[1] ^= t01 ^ r[0][1];
}
#elif MASKING_N == 3
void masked_and(masked_uint64_t *out,
                const masked_uint64_t *a,
                const masked_uint64_t *b,
                const uint64_t r[3][3]) {
    out->share[0] = a->share[0] & b->share[0];
    out->share[1] = a->share[1] & b->share[1];
    out->share[2] = a->share[2] & b->share[2];

    uint64_t t01 = (a->share[0] & b->share[1]) ^ (a->share[1] & b->share[0]);
    out->share[0] ^= r[0][1];
    out->share[1] ^= t01 ^ r[0][1];

    uint64_t t02 = (a->share[0] & b->share[2]) ^ (a->share[2] & b->share[0]);
    out->share[0] ^= r[0][2];
    out->share[2] ^= t02 ^ r[0][2];

    uint64_t t12 = (a->share[1] & b->share[2]) ^ (a->share[2] & b->share[1]);
    out->share[1] ^= r[1][2];
    out->share[2] ^= t12 ^ r[1][2];
}
#elif MASKING_N == 4
void masked_and(masked_uint64_t *out,
                const masked_uint64_t *a,
                const masked_uint64_t *b,
                const uint64_t r[4][4]) {
    // Diagonal terms
    out->share[0] = a->share[0] & b->share[0];
    out->share[1] = a->share[1] & b->share[1];
    out->share[2] = a->share[2] & b->share[2];
    out->share[3] = a->share[3] & b->share[3];

    // Cross terms with randomness
    uint64_t t01 = (a->share[0] & b->share[1]) ^ (a->share[1] & b->share[0]);
    out->share[0] ^= r[0][1];
    out->share[1] ^= t01 ^ r[0][1];

    uint64_t t02 = (a->share[0] & b->share[2]) ^ (a->share[2] & b->share[0]);
    out->share[0] ^= r[0][2];
    out->share[2] ^= t02 ^ r[0][2];

    uint64_t t03 = (a->share[0] & b->share[3]) ^ (a->share[3] & b->share[0]);
    out->share[0] ^= r[0][3];
    out->share[3] ^= t03 ^ r[0][3];

    uint64_t t12 = (a->share[1] & b->share[2]) ^ (a->share[2] & b->share[1]);
    out->share[1] ^= r[1][2];
    out->share[2] ^= t12 ^ r[1][2];

    uint64_t t13 = (a->share[1] & b->share[3]) ^ (a->share[3] & b->share[1]);
    out->share[1] ^= r[1][3];
    out->share[3] ^= t13 ^ r[1][3];

    uint64_t t23 = (a->share[2] & b->share[3]) ^ (a->share[3] & b->share[2]);
    out->share[2] ^= r[2][3];
    out->share[3] ^= t23 ^ r[2][3];
}
#elif MASKING_N == 5
void masked_and(masked_uint64_t *out,
                const masked_uint64_t *a,
                const masked_uint64_t *b,
                const uint64_t r[5][5]) {
    // Diagonal
    for (int i = 0; i < 5; i++) {
        out->share[i] = a->share[i] & b->share[i];
    }

    // Cross terms
    uint64_t t01 = (a->share[0] & b->share[1]) ^ (a->share[1] & b->share[0]);
    out->share[0] ^= r[0][1];  out->share[1] ^= t01 ^ r[0][1];

    uint64_t t02 = (a->share[0] & b->share[2]) ^ (a->share[2] & b->share[0]);
    out->share[0] ^= r[0][2];  out->share[2] ^= t02 ^ r[0][2];

    uint64_t t03 = (a->share[0] & b->share[3]) ^ (a->share[3] & b->share[0]);
    out->share[0] ^= r[0][3];  out->share[3] ^= t03 ^ r[0][3];

    uint64_t t04 = (a->share[0] & b->share[4]) ^ (a->share[4] & b->share[0]);
    out->share[0] ^= r[0][4];  out->share[4] ^= t04 ^ r[0][4];

    uint64_t t12 = (a->share[1] & b->share[2]) ^ (a->share[2] & b->share[1]);
    out->share[1] ^= r[1][2];  out->share[2] ^= t12 ^ r[1][2];

    uint64_t t13 = (a->share[1] & b->share[3]) ^ (a->share[3] & b->share[1]);
    out->share[1] ^= r[1][3];  out->share[3] ^= t13 ^ r[1][3];

    uint64_t t14 = (a->share[1] & b->share[4]) ^ (a->share[4] & b->share[1]);
    out->share[1] ^= r[1][4];  out->share[4] ^= t14 ^ r[1][4];

    uint64_t t23 = (a->share[2] & b->share[3]) ^ (a->share[3] & b->share[2]);
    out->share[2] ^= r[2][3];  out->share[3] ^= t23 ^ r[2][3];

    uint64_t t24 = (a->share[2] & b->share[4]) ^ (a->share[4] & b->share[2]);
    out->share[2] ^= r[2][4];  out->share[4] ^= t24 ^ r[2][4];

    uint64_t t34 = (a->share[3] & b->share[4]) ^ (a->share[4] & b->share[3]);
    out->share[3] ^= r[3][4];  out->share[4] ^= t34 ^ r[3][4];
}
#else
	void masked_and(masked_uint64_t *out,
					const masked_uint64_t *a,
					const masked_uint64_t *b,
					const uint64_t r[MASKING_N][MASKING_N]) {
		// Step 1: Initialize with diagonal terms
		for (size_t i = 0; i < MASKING_N; i++) {
			out->share[i] = a->share[i] & b->share[i];
		}

		// Step 2: Add cross terms with proper masking
		for (size_t i = 0; i < MASKING_N; i++) {
			for (size_t j = i + 1; j < MASKING_N; j++) {
				uint64_t cross_term = (a->share[i] & b->share[j]) ^
									 (a->share[j] & b->share[i]);

				// Distribute the random mask correctly
				out->share[i] ^= r[i][j];
				out->share[j] ^= cross_term ^ r[i][j];
			}
		}
	}
#endif

/**
 * Perform bitwise NOT on a masked value.
 *
 * Since NOT is linear, it can be applied to each share.
 * However, due to XOR mask structure, one share must be adjusted
 * to maintain correct recombined parity.
 *
 * @param dst Output masked result
 * @param src Input masked operand
 */
void masked_not(masked_uint64_t *dst, const masked_uint64_t *src) {
    // Bitwise NOT of each share — safe for Boolean masking.
    for (size_t i = 0; i < MASKING_N; ++i)
        dst->share[i] = ~src->share[i];

    // Adjust one share so that the recombined NOT is correct.
    uint64_t orig_parity = 0, inv_parity = 0;
    for (size_t i = 0; i < MASKING_N; ++i) {
        orig_parity ^= src->share[i];
        inv_parity  ^= dst->share[i];
    }
    uint64_t delta = inv_parity ^ ~orig_parity;
    dst->share[0] ^= delta;
}
