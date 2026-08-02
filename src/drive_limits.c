#include "drive_limits.h"

static float clamp_float(
    float value,
    float minimum,
    float maximum)
{
    if (value < minimum)
    {
        return minimum;
    }

    if (value > maximum)
    {
        return maximum;
    }

    return value;
}

void drive_limits_init(drive_limits_t *limits)
{
    limits->max_forward_speed = 3.0f;
    limits->max_reverse_speed = 3.0f;
    limits->max_turn = 1.0f;
    limits->max_motor_velocity = 3.0f;
}

void drive_apply_limits(
    drive_state_t *state,
    const drive_limits_t *limits)
{
    state->speed = clamp_float(
        state->speed,
        -limits->max_reverse_speed,
        limits->max_forward_speed);

    state->turn = clamp_float(
        state->turn,
        -limits->max_turn,
        limits->max_turn);
}