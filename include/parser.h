#ifndef PARSER_H
#define PARSER_H

typedef struct
{
    float left_v;
    float right_v;
    float kp;
    float kd;
    float torque;
} motion_state_t;

void motion_state_init(motion_state_t *state);
int parse_command(const char *line, motion_state_t *state);

#endif