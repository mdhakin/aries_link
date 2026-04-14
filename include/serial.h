#ifndef SERIAL_H
#define SERIAL_H

#include <stddef.h>

int serial_open(const char *device, int baudrate);
int serial_read_line(int fd, char *buffer, size_t buffer_size);
void serial_close(int fd);

#endif