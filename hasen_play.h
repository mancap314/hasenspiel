#ifndef HASEN_PLAY_H_
#define HASEN_PLAY_H_

#include "hasen.h"
#include <errno.h>

#typedef struct {
    uint32_t state;
    float perc_black_victory;
    bool can_force_victory;
} estate_t;

#endif