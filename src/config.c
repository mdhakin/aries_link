#include "config.h"

#include <stdio.h>
#include <string.h>

void aries_config_set_defaults(aries_config_t *config)
{
    if (config == NULL)
    {
        return;
    }

    config->left_motor_id = 3U;
    config->right_motor_id = 4U;
    config->max_speed = 3.0f;
}

int aries_config_load(
    const char *path,
    aries_config_t *config)
{
    if (path == NULL || config == NULL)
    {
        return -1;
    }

    FILE *file = fopen(path, "r");

    if (file == NULL)
    {
        return -1;
    }

    char line[128];

    while (fgets(line, sizeof(line), file) != NULL)
    {
        unsigned int uint_value = 0;
        float float_value = 0.0f;

        if (sscanf(line, "left_motor_id=%u", &uint_value) == 1)
        {
            config->left_motor_id = uint_value;
            continue;
        }

        if (sscanf(line, "right_motor_id=%u", &uint_value) == 1)
        {
            config->right_motor_id = uint_value;
            continue;
        }

        if (sscanf(line, "max_speed=%f", &float_value) == 1)
        {
            config->max_speed = float_value;
            continue;
        }
    }

    fclose(file);
    return 0;
}

int aries_config_save(
    const char *path,
    const aries_config_t *config)
{
    if (path == NULL || config == NULL)
    {
        return -1;
    }

    FILE *file = fopen(path, "w");

    if (file == NULL)
    {
        return -1;
    }

    fprintf(file, "left_motor_id=%u\n", config->left_motor_id);
    fprintf(file, "right_motor_id=%u\n", config->right_motor_id);
    fprintf(file, "max_speed=%.3f\n", config->max_speed);

    fclose(file);
    return 0;
}