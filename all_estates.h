#ifndef ALL_ESTATES_H_
#define ALL_ESTATES_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint32_t state;
	uint32_t sltbv;
	uint32_t sltwv;
	float perc_victory;
	bool can_force_victory;
} estate_t;

extern const estate_t ALL_ESTATES[776283];

#endif
