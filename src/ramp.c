#include "ramp.h"

static float abs_float(float value)
{
    return value < 0.0f ? -value : value;
}

void ramp_config_init(ramp_config_t *config)
{
    config->enabled = 1;
    config->max_delta_per_tick = 0.10f;
}

float ramp_step_float(float current, float target, float max_delta)
{
    float delta = target - current;

    if (abs_float(delta) <= max_delta)
    {
        return target;
    }

    if (delta > 0.0f)
    {
        return current + max_delta;
    }

    return current - max_delta;
}

void ramp_update(
    motion_state_t *current,
    const motion_state_t *target,
    const ramp_config_t *config)
{
    if (!config->enabled)
    {
        current->left_v = target->left_v;
        current->right_v = target->right_v;
    }
    else
    {
        current->left_v = ramp_step_float(
            current->left_v,
            target->left_v,
            config->max_delta_per_tick);

        current->right_v = ramp_step_float(
            current->right_v,
            target->right_v,
            config->max_delta_per_tick);
    }

    current->kp = target->kp;
    current->kd = target->kd;
    current->torque = target->torque;
}