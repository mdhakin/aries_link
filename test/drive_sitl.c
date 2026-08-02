#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "drive.h"
#include "drive_parser.h"
#include "drive_limits.h"

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
        "  limits              Display drive output limits\n"
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
    const drive_limits_t *limits,
    drive_output_t *output,
    int tick_count)
{
    for (int tick = 1; tick <= tick_count; ++tick)
    {
        drive_update(current, target, config);
        drive_mix(current, limits, output);

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
    drive_limits_t limits;

    char line[128];

    drive_state_init(&current);
    drive_state_init(&target);
    drive_ramp_config_init(&ramp_config);
    drive_limits_init(&limits);
    drive_mix(&current, &limits, &output);

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

            int tick_count = 0;

            /*
            * These are SIL-only commands.
            * They exist to control and inspect the simulation.
            */
            if (strcmp(line, "tick") == 0)
            {
                run_ticks(
                    &current,
                    &target,
                    &ramp_config,
                    &limits,
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
                    &limits,
                    &output,
                    tick_count);
            }
            else if (strcmp(line, "state") == 0)
            {
                drive_mix(&current, &limits, &output);
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
            else if (strcmp(line, "limits") == 0)
            {
                printf(
                    "maximum forward speed: %.2f\n"
                    "maximum reverse speed: %.2f\n"
                    "maximum turn:          %.2f\n"
                    "maximum motor velocity: %.2f\n",
                    limits.max_forward_speed,
                    limits.max_reverse_speed,
                    limits.max_turn,
                    limits.max_motor_velocity);
            }
            else if (line[0] == '\0')
            {
                continue;
            }
            else
            {
                /*
                * Anything that is not a SIL command is passed to
                * the real drive-command parser.
                *
                * This is where speed, turn and stop are handled.
                */
                drive_parse_result_t result =
                    drive_parse_command(line, &target);

                if (result == DRIVE_PARSE_OK)
                {
                    drive_apply_limits(&target, &limits);

                    printf("Command accepted.\n");

                    drive_mix(&current, &limits, &output);
                    print_state(&current, &target, &output);
                }
                else
                {
                    printf("Unknown or invalid command: %s\n", line);
                }
            }
}

    return 0;
}