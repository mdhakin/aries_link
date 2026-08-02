#ifndef DRIVE_PARSER_H
#define DRIVE_PARSER_H

#include "drive.h"

typedef enum
{
    DRIVE_PARSE_OK = 0,
    DRIVE_PARSE_UNKNOWN_COMMAND = -1,
    DRIVE_PARSE_INVALID_VALUE = -2
} drive_parse_result_t;

drive_parse_result_t drive_parse_command(
    const char *line,
    drive_state_t *target);

#endif