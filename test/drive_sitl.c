#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "drive.h"

static void print_help(void)
{
    printf(
        "Commands:\n"
        "  speed <value>       Set target forward/reverse speed\n"
        "  turn <value>        Set target steering input\n"
        "  tick                Advance one control-loop tick\n"
        "  tick <count>        Advance multiple control-loop ticks\n"
        "  state               Display target, current and motor output\n"
        "  ramp                Display ramp configuration\n"
        "  stop                Set target speed and turn to zero\n"
        "  help                Show commands\n"
        "  quit                Exit\n");
}

static void print_state(
    const drive_state_t *current,
    const drive_state_t *target,
    const drive_output_t *output)
{
    printf(
        "target:  speed=%6.2f  turn=%6.2f\n"
        "current: speed=%6.2f  turn=%6.2f\n"
        "output:  motor1=%6.2f  motor2=%6.2f\n",
        target->speed,
        target->turn,
        current->speed,
        current->turn,
        output->left_v,
        output->right_v);
}

static void run_ticks(
    drive_state_t *current,
    const drive_state_t *target,
    const drive_ramp_config_t *config,
    drive_output_t *output,
    int tick_count)
{
    for (int tick = 1; tick <= tick_count; ++tick)
    {
        drive_update(current, target, config);
        drive_mix(current, output);

        printf(
            "tick %3d: speed=%6.2f turn=%6.2f "
            "motor1=%6.2f motor2=%6.2f\n",
            tick,
            current->speed,
            current->turn,
            output->left_v,
            output->right_v);
    }
}

int main(void)
{
    drive_state_t current;
    drive_state_t target;
    drive_output_t output;
    drive_ramp_config_t ramp_config;

    char line[128];

    drive_state_init(&current);
    drive_state_init(&target);
    drive_ramp_config_init(&ramp_config);
    drive_mix(&current, &output);

    printf("Aries-Link Drive SIL\n");
    print_help();
    print_state(&current, &target, &output);

    while (1)
    {
        printf("> ");

        if (fgets(line, sizeof(line), stdin) == NULL)
        {
            break;
        }

        line[strcspn(line, "\r\n")] = '\0';

        float value = 0.0f;
        int tick_count = 0;

        if (sscanf(line, "speed %f", &value) == 1)
        {
            target.speed = value;
            printf("Target speed set to %.2f\n", value);
        }
        else if (sscanf(line, "turn %f", &value) == 1)
        {
            target.turn = value;
            printf("Target turn set to %.2f\n", value);
        }
        else if (strcmp(line, "tick") == 0)
        {
            run_ticks(
                &current,
                &target,
                &ramp_config,
                &output,
                1);
        }
        else if (sscanf(line, "tick %d", &tick_count) == 1)
        {
            if (tick_count <= 0)
            {
                printf("Tick count must be greater than zero.\n");
                continue;
            }

            run_ticks(
                &current,
                &target,
                &ramp_config,
                &output,
                tick_count);
        }
        else if (strcmp(line, "stop") == 0)
        {
            target.speed = 0.0f;
            target.turn = 0.0f;

            printf("Target speed and turn set to zero.\n");
        }
        else if (strcmp(line, "state") == 0)
        {
            drive_mix(&current, &output);
            print_state(&current, &target, &output);
        }
        else if (strcmp(line, "ramp") == 0)
        {
            printf(
                "speed delta/tick: %.2f\n"
                "turn delta/tick:  %.2f\n",
                ramp_config.speed_delta_per_tick,
                ramp_config.turn_delta_per_tick);
        }
        else if (strcmp(line, "help") == 0)
        {
            print_help();
        }
        else if (strcmp(line, "quit") == 0)
        {
            break;
        }
        else if (line[0] != '\0')
        {
            printf("Unknown command: %s\n", line);
        }
    }

    return 0;
}