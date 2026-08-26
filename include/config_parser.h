#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

typedef enum {
  CONFIG_PARSE_UNKNOWN,
  CONFIG_PARSE_SHOW,
  CONFIG_PARSE_SET_MAX_SPEED,
  CONFIG_PARSE_SET_MOTORS,
  CONFIG_PARSE_INVALID
} config_parse_result_t;

typedef struct {
  float max_speed;
  unsigned int left_motor_id;
  unsigned int right_motor_id;
} config_command_t;

config_parse_result_t config_parse_command(const char* line, config_command_t* command);

#endif