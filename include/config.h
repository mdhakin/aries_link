#ifndef CONFIG_H
#define CONFIG_H

typedef struct
{
    unsigned int left_motor_id;
    unsigned int right_motor_id;
    float max_speed;
} aries_config_t;

void aries_config_set_defaults(aries_config_t *config);

int aries_config_load(
    const char *path,
    aries_config_t *config);

int aries_config_save(
    const char *path,
    const aries_config_t *config);

#endif