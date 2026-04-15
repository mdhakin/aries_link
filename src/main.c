#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "info_board.h"
#include "parser.h"
#include "serial.h"
#include "udp_tx.h"

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

int main(void)
{
    info_board_t board;
    motion_state_t state;

    info_board_init(&board);
    motion_state_init(&state);

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

        if (serial_read_line(serial_fd, line, sizeof(line)) <= 0)
        {
            continue;
        }

        printf("RX: %s\n", line);

        if (strcmp(line, "version") == 0)
        {
            char response[128];
            snprintf(response, sizeof(response), "ok version %s", board.version);

            serial_write_line(serial_fd, response);
            continue;
        }

        if (parse_command(line, &state) == 0)
        {
            if (send_motion_packet(udp_sock, &state) != 0)
            {
                printf("Failed to send motion packet.\n");
                break;
            }

            serial_write_line(serial_fd, "ok motion");
        }
        else
        {
            serial_write_line(serial_fd, "err unknown command");
        }
    }

    udp_close(udp_sock);
    serial_close(serial_fd);
    return 0;
}