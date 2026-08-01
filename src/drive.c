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
    drive_output_t *output)
{
    /*
     * Motor 1 is physically mirrored relative to motor 2.
     *
     * Straight forward:
     *   motor 1 = negative
     *   motor 2 = positive
     */
    output->left_v = -(state->speed + state->turn);
    output->right_v = state->speed - state->turn;
}