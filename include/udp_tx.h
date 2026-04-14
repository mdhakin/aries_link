#ifndef UDP_TX_H
#define UDP_TX_H

int udp_open(const char *ip, int port);
int udp_send_text(int sockfd, const char *text);
void udp_close(int sockfd);

#endif