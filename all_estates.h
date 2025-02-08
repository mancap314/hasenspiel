#ifndef ALL_ESTATES_H_
#define ALL_ESTATES_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t state;
    float perc_victory;
    bool can_force_victory;
} estate_t;  // enhanced state

extern const estate_t ALL_ESTATES[776283]; 

#endif
