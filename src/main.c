#include <stdio.h>

#include "udp_tx.h"

int main(void)
{
    int udp_sock = udp_open("127.0.0.1", 9750);
    if (udp_sock < 0)
    {
        return 1;
    }

    const char *msg =
        "3 v 0.00\n"
        "3 kp 0.07\n"
        "3 kd 2.00\n"
        "3 t 0.00\n"
        "4 v 0.00\n"
        "4 kp 0.07\n"
        "4 kd 2.00\n"
        "4 t 0.00\n";

    printf("Sending test UDP packet to Aries-Vector...\n");

    if (udp_send_text(udp_sock, msg) != 0)
    {
        udp_close(udp_sock);
        return 1;
    }

    printf("Done.\n");
    udp_close(udp_sock);
    return 0;
}