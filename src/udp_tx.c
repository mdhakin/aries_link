#include "udp_tx.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static struct sockaddr_in g_dest_addr;

int udp_open(const char *ip, int port)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    memset(&g_dest_addr, 0, sizeof(g_dest_addr));
    g_dest_addr.sin_family = AF_INET;
    g_dest_addr.sin_port = htons((unsigned short)port);

    if (inet_pton(AF_INET, ip, &g_dest_addr.sin_addr) != 1)
    {
        fprintf(stderr, "inet_pton failed for %s\n", ip);
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int udp_send_text(int sockfd, const char *text)
{
    size_t len = strlen(text);

    ssize_t sent = sendto(
        sockfd,
        text,
        len,
        0,
        (const struct sockaddr *)&g_dest_addr,
        sizeof(g_dest_addr));

    if (sent < 0)
    {
        perror("sendto");
        return -1;
    }

    return 0;
}

void udp_close(int sockfd)
{
    if (sockfd >= 0)
    {
        close(sockfd);
    }
}