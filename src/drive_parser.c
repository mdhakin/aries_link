#include "drive_parser.h"

#include <stdio.h>
#include <string.h>

drive_parse_result_t drive_parse_command(
    const char *line,
    drive_state_t *target)
{
    char command[32];
    float value = 0.0f;

    if (line == NULL || target == NULL)
    {
        return DRIVE_PARSE_INVALID_VALUE;
    }

    if (strcmp(line, "stop") == 0)
    {
        target->speed = 0.0f;
        target->turn = 0.0f;
        return DRIVE_PARSE_OK;
    }

    if (sscanf(line, "%31s %f", command, &value) != 2)
    {
        return DRIVE_PARSE_UNKNOWN_COMMAND;
    }

    if (strcmp(command, "speed") == 0)
    {
        target->speed = value;
        return DRIVE_PARSE_OK;
    }

    if (strcmp(command, "turn") == 0)
    {
        target->turn = value;
        return DRIVE_PARSE_OK;
    }

    return DRIVE_PARSE_UNKNOWN_COMMAND;
}