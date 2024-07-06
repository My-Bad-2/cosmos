#ifndef UTILS_FIXED_POINT_H
#define UTILS_FIXED_POINT_H

#include <stdint.h>

// Some fixed point arithmetics

struct fixed_point {
	uint32_t unshifted_val;
	uint32_t val_32;
	uint32_t val_64;
};

static inline void fixed_point_div_32(struct fixed_point* result,
									  uint32_t dividend, uint32_t divisor) {
	uint64_t tmp = ((uint64_t)dividend << 32) / divisor;
	uint32_t rem = (uint32_t)(((uint64_t)dividend << 32) % divisor);

	result->unshifted_val = (uint32_t)(tmp >> 32);
	result->val_32 = (uint32_t)tmp;
	tmp = ((uint64_t)rem << 32) / divisor;
	result->val_64 = (uint32_t)tmp;
}

static inline uint64_t mul_u32(uint32_t a, uint32_t b) {
	return (uint64_t)a * b;
}

static inline uint64_t mul_u32_fixed_point(uint32_t a, struct fixed_point b) {
	uint64_t res_0 = mul_u32(a, b.unshifted_val);
	uint64_t tmp = mul_u32(a, b.val_32);
	res_0 += tmp >> 32;

	uint64_t res_val_32 = (uint32_t)tmp;
	res_val_32 += mul_u32(a, b.val_64) >> 32;
	res_0 += res_val_32 >> 32;

	uint32_t res_val_32_32 = (uint32_t)res_val_32;
	uint64_t ret = res_0 + (res_val_32_32 >> 31);

	return ret;
}

static inline uint32_t mul_u64_fixed_point(uint64_t a, struct fixed_point b) {
	uint32_t a_r32 = (uint32_t)(a >> 32);
	uint32_t a_0 = (uint32_t)a;

	uint64_t res_val_32 = mul_u32(a_0, b.unshifted_val) << 32;
	res_val_32 += mul_u32(a_r32, b.val_32) << 32;
	res_val_32 += mul_u32(a_0, b.val_32);
	res_val_32 += mul_u32(a_r32, b.val_64);
	res_val_32 += mul_u32(a_0, b.val_64) >> 32;

	uint32_t ret =
		(uint32_t)((res_val_32 >> 32) + ((uint32_t)res_val_32 >> 31));

	return ret;
}

static inline uint64_t u64_mul_u64_fixed_point(uint64_t a,
											   struct fixed_point b) {
	uint32_t a_r32 = (uint32_t)(a >> 32);
	uint32_t a_0 = (uint32_t)a;
	uint64_t tmp = mul_u32(a_r32, b.unshifted_val);
	uint64_t res_0 = tmp << 32;

	tmp = mul_u32(a_0, b.unshifted_val);
	res_0 += tmp;

	tmp = mul_u32(a_r32, b.val_32);
	res_0 += tmp;

	tmp = mul_u32(a_0, b.val_32);
	res_0 += tmp >> 32;

	uint64_t res_val_32 = (uint32_t)tmp;

	tmp = mul_u32(a_r32, b.val_64);
	res_0 += tmp >> 32;
	res_val_32 += (uint32_t)tmp;

	tmp = mul_u32(a_0, b.val_64);
	res_val_32 += tmp >> 32;
	res_0 += res_val_32 >> 32;

	uint32_t res_val_32_32 = (uint32_t)(res_val_32);
	uint64_t ret = res_0 + (res_val_32_32 >> 31);

	return ret;
}

#endif	// UTILS_FIXED_POINT_H