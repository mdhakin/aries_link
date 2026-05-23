#ifndef RAMP_H
#define RAMP_H

#include "parser.h"

typedef struct
{
    int enabled;
    float max_delta_per_tick;
} ramp_config_t;

void ramp_config_init(ramp_config_t *config);

float ramp_step_float(float current, float target, float max_delta);

void ramp_update(
    motion_state_t *current,
    const motion_state_t *target,
    const ramp_config_t *config);

#endif