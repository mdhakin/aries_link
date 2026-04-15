#ifndef INFO_BOARD_H
#define INFO_BOARD_H

typedef struct
{
    const char *version;
} info_board_t;

void info_board_init(info_board_t *board);

#endif