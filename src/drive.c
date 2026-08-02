#include "drive.h"

static float step_toward(
    float current,
    float target,
    float max_delta)
{
    const float delta = target - current;

    if (delta > max_delta)
    {
        return current + max_delta;
    }

    if (delta < -max_delta)
    {
        return current - max_delta;
    }

    return target;
}

static float abs_float(float value)
{
    return value < 0.0f ? -value : value;
}

void drive_state_init(drive_state_t *state)
{
    state->speed = 0.0f;
    state->turn = 0.0f;
}

void drive_ramp_config_init(drive_ramp_config_t *config)
{
    /*
     * Speed changes gently.
     * Steering reacts more quickly.
     */
    config->speed_delta_per_tick = 0.10f;
    config->turn_delta_per_tick = 0.25f;
}

void drive_update(
    drive_state_t *current,
    const drive_state_t *target,
    const drive_ramp_config_t *config)
{
    current->speed = step_toward(
        current->speed,
        target->speed,
        config->speed_delta_per_tick);

    current->turn = step_toward(
        current->turn,
        target->turn,
        config->turn_delta_per_tick);
}

void drive_mix(
    const drive_state_t *state,
    const drive_limits_t *limits,
    drive_output_t *output)
{
    /*
     * Motor 1 is physically mirrored relative to motor 2.
     */
    float left_v = -(state->speed + state->turn);
    float right_v = state->speed - state->turn;

    const float left_magnitude = abs_float(left_v);
    const float right_magnitude = abs_float(right_v);

    float largest_magnitude = left_magnitude;

    if (right_magnitude > largest_magnitude)
    {
        largest_magnitude = right_magnitude;
    }

    /*
     * If either motor exceeds the configured limit, scale both
     * proportionally. This preserves the steering relationship.
     */
    if (limits->max_motor_velocity > 0.0f &&
        largest_magnitude > limits->max_motor_velocity)
    {
        const float scale =
            limits->max_motor_velocity / largest_magnitude;

        left_v *= scale;
        right_v *= scale;
    }

    output->left_v = left_v;
    output->right_v = right_v;
}