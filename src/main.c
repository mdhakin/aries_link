#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

    motion_state_t state;
    motion_state_init(&state);

    printf("Aries-Link listening on /dev/ttyUSB0 at 9600...\n");

    while (1)
    {
        char line[256];

        int len = serial_read_line(serial_fd, line, sizeof(line));
        if (len < 0)
        {
            break;
        }

        if (len == 0)
        {
            continue;
        }

        printf("RX: %s\n", line);

        if (parse_command(line, &state) == 0)
        {
            printf("State updated: left=%.2f right=%.2f\n",
                   state.left_v, state.right_v);
        }
        else
        {
            printf("Unknown command: %s\n", line);
        }

        if (send_motion_packet(udp_sock, &state) != 0)
        {
            break;
        }

        printf("Sent UDP packet to Aries-Vector.\n");
    }

    udp_close(udp_sock);
    serial_close(serial_fd);
    return 0;
}