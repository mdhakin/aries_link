#include "config_parser.h"

#include <stdio.h>
#include <string.h>

config_parse_result_t config_parse_command(const char* line, config_command_t* command) {
  if (line == NULL || command == NULL) {
    return CONFIG_PARSE_INVALID;
  }

  if (strcmp(line, "config") == 0) {
    return CONFIG_PARSE_SHOW;
  }

  float max_speed = 0.0f;

  if (sscanf(line, "config max_speed %f", &max_speed) == 1) {
    command->max_speed = max_speed;
    return CONFIG_PARSE_SET_MAX_SPEED;
  }

  unsigned int left_id = 0;
  unsigned int right_id = 0;

  if (sscanf(line, "config motor %u %u", &left_id, &right_id) == 2) {
    command->left_motor_id = left_id;
    command->right_motor_id = right_id;

    return CONFIG_PARSE_SET_MOTORS;
  }

  return CONFIG_PARSE_UNKNOWN;
}