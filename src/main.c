
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "info_board.h"
#include "parser.h"
#include "serial.h"
#include "udp_tx.h"
#include "ramp.h"
#include "drive.h"
#include "drive_parser.h"
#include "drive_limits.h"

static int send_motion_packet(int udp_sock, const motion_state_t *state)
{
    char msg[256];

    snprintf(
        msg,
        sizeof(msg),
        "3 v %.2f\n"
        "3 kp %.2f\n"
        "3 kd %.2f\n"
        "3 t %.2f\n"
        "4 v %.2f\n"
        "4 kp %.2f\n"
        "4 kd %.2f\n"
        "4 t %.2f\n",
        state->left_v,
        state->kp,
        state->kd,
        state->torque,
        state->right_v,
        state->kp,
        state->kd,
        state->torque);

    return udp_send_text(udp_sock, msg);
}

static void sleep_ms(long milliseconds)
{
    struct timespec ts;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000L;

    nanosleep(&ts, NULL);
}

int main(void)
{
    info_board_t board;
    motion_state_t current_state;

    drive_state_t current_drive;
    drive_state_t target_drive;
    drive_output_t drive_output;
    drive_ramp_config_t drive_ramp;
    drive_limits_t drive_limits;

    info_board_init(&board);
    motion_state_init(&current_state);

    drive_state_init(&current_drive);
    drive_state_init(&target_drive);

    drive_ramp_config_init(&drive_ramp);

    drive_limits_init(&drive_limits);

    drive_mix(
        &current_drive,
        &drive_limits,
        &drive_output);

    int serial_fd = serial_open("/dev/ttyUSB0", 9600);
    if (serial_fd < 0)
    {
        return 1;
    }

    int udp_sock = udp_open("127.0.0.1", 9750);
    if (udp_sock < 0)
    {
        serial_close(serial_fd);
        return 1;
    }

    printf("Aries-Link listening on /dev/ttyUSB0 at 9600...\n");

    while (1)
    {
        char line[256];

        int result = serial_try_read_line(
            serial_fd,
            line,
            sizeof(line));

        if (result > 0)
        {
            printf("RX: %s\n", line);

            if (strcmp(line, "version") == 0)
            {
                char response[128];

                snprintf(
                    response,
                    sizeof(response),
                    "ok version %s",
                    board.version);

                serial_write_line(serial_fd, response);
            }
            else
            {
                drive_parse_result_t drive_result =
                    drive_parse_command(line, &target_drive);

                if (drive_result == DRIVE_PARSE_OK)
                {
                    drive_apply_limits(
                        &target_drive,
                        &drive_limits);

                    printf(
                        "Drive target: speed=%.2f turn=%.2f\n",
                        target_drive.speed,
                        target_drive.turn);

                    serial_write_line(serial_fd, "ok drive");
                }
                else
                {
                    serial_write_line(
                        serial_fd,
                        "err unknown command");
                }
            }
        }

        drive_update(
            &current_drive,
            &target_drive,
            &drive_ramp);

        drive_mix(
            &current_drive,
            &drive_limits,
            &drive_output);

        /*
         * Adapt the high-level drive output into the existing
         * motion_state_t expected by send_motion_packet().
         *
         * Keep kp, kd and torque from the initialized motion state.
         */
        current_state.left_v = drive_output.left_v;
        current_state.right_v = drive_output.right_v;

        if (send_motion_packet(udp_sock, &current_state) != 0)
        {
            printf("Failed to send motion packet.\n");
            break;
        }

        sleep_ms(50);
    }

    udp_close(udp_sock);
    serial_close(serial_fd);
    return 0;
}