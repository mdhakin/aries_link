#include "parser.h"

#include <stdio.h>
#include <string.h>

void motion_state_init(motion_state_t *state)
{
    state->left_v = 0.0f;
    state->right_v = 0.0f;
    state->kp = 0.07f;
    state->kd = 2.0f;
    state->torque = 0.0f;
}

int parse_command(const char *line, motion_state_t *state)
{
    char cmd[32];
    float value = 0.0f;

    if (strcmp(line, "stop") == 0)
    {
        state->left_v = 0.0f;
        state->right_v = 0.0f;
        return 0;
    }

    if (sscanf(line, "%31s %f", cmd, &value) != 2)
    {
        return -1;
    }

    if (strcmp(cmd, "fwd") == 0)
    {
        state->left_v = -value;
        state->right_v = value;
        return 0;
    }

    if (strcmp(cmd, "rev") == 0)
    {
        state->left_v = value;
        state->right_v = -value;
        return 0;
    }

    if (strcmp(cmd, "left") == 0)
    {
        state->left_v = value * 0.4f;
        state->right_v = -value;
        return 0;
    }

    if (strcmp(cmd, "right") == 0)
    {
        state->left_v = -value;
        state->right_v = value * 0.4f;
        return 0;
    }

    if (strcmp(cmd, "pright") == 0)
    {
        state->left_v = -value;
        state->right_v = -value * 0.4f;
        return 0;
    }

    if (strcmp(cmd, "pleft") == 0)
    {
        state->left_v = value;
        state->right_v = value * 0.4f;
        return 0;
    }

    return -1;
}