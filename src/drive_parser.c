#include "drive_parser.h"

#include <stdio.h>
#include <string.h>

drive_parse_result_t drive_parse_command(
    const char *line,
    drive_state_t *target)
{
    char command[32];
    float value1 = 0.0f;
    float value2 = 0.0f;

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

    /*
     * Atomic drive command:
     *
     *   drive <speed> <turn>
     */
    if (sscanf(line, "%31s %f %f", command, &value1, &value2) == 3)
    {
        if (strcmp(command, "drive") == 0)
        {
            target->speed = value1;
            target->turn = value2;
            return DRIVE_PARSE_OK;
        }

        return DRIVE_PARSE_UNKNOWN_COMMAND;
    }

    /*
     * Existing single-value commands:
     *
     *   speed <value>
     *   turn  <value>
     */
    if (sscanf(line, "%31s %f", command, &value1) == 2)
    {
        if (strcmp(command, "speed") == 0)
        {
            target->speed = value1;
            return DRIVE_PARSE_OK;
        }

        if (strcmp(command, "turn") == 0)
        {
            target->turn = value1;
            return DRIVE_PARSE_OK;
        }
    }

    return DRIVE_PARSE_UNKNOWN_COMMAND;
}