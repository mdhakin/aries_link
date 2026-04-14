#include "serial.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static speed_t baudrate_to_flag(int baudrate)
{
    switch (baudrate)
    {
        case 9600:   return B9600;
        case 19200:  return B19200;
        case 38400:  return B38400;
        case 57600:  return B57600;
        case 115200: return B115200;
        default:     return B9600;
    }
}

int serial_open(const char *device, int baudrate)
{
    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(fd, &tty) != 0)
    {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    speed_t speed = baudrate_to_flag(baudrate);

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    #ifdef CRTSCTS
    tty.c_cflag &= ~CRTSCTS;
    #endif
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;

    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

int serial_read_line(int fd, char *buffer, size_t buffer_size)
{
    size_t index = 0;

    if (buffer == NULL || buffer_size == 0)
    {
        return -1;
    }

    while (index + 1 < buffer_size)
    {
        char ch;
        ssize_t n = read(fd, &ch, 1);

        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("read");
            return -1;
        }

        if (n == 0)
        {
            continue;
        }

        if (ch == '\r')
        {
            continue;
        }

        if (ch == '\n')
        {
            break;
        }

        buffer[index++] = ch;
    }

    buffer[index] = '\0';
    return (int)index;
}

void serial_close(int fd)
{
    if (fd >= 0)
    {
        close(fd);
    }
}