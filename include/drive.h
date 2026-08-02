#ifndef DRIVE_H
#define DRIVE_H

typedef struct
{
    float speed;
    float turn;
} drive_state_t;

typedef struct
{
    float left_v;
    float right_v;
} drive_output_t;

typedef struct
{
    float speed_delta_per_tick;
    float turn_delta_per_tick;
} drive_ramp_config_t;

typedef struct
{
    float max_forward_speed;
    float max_reverse_speed;
    float max_turn;
    float max_motor_velocity;
} drive_limits_t;

void drive_state_init(drive_state_t *state);

void drive_ramp_config_init(drive_ramp_config_t *config);

void drive_update(
    drive_state_t *current,
    const drive_state_t *target,
    const drive_ramp_config_t *config);

void drive_mix(
    const drive_state_t *state,
    const drive_limits_t *limits,
    drive_output_t *output);

#endif