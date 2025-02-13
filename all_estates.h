#ifndef ALL_ESTATES_H_
#define ALL_ESTATES_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint32_t state;
	uint8_t black_value;
	uint8_t white_value;
} estate_t;

extern const estate_t ALL_ESTATES[776283];

#endif
