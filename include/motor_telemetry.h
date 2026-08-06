#ifndef MOTOR_TELEMETRY_H
#define MOTOR_TELEMETRY_H

#include <stdbool.h>
#include <stdint.h>

#include "ak60_telemetry.h"

#define MOTOR_TELEMETRY_MOTOR_3 3U
#define MOTOR_TELEMETRY_MOTOR_4 4U

typedef struct {
  ak60_telemetry_t motor3;
  ak60_telemetry_t motor4;

  bool motor3_valid;
  bool motor4_valid;

  uint64_t motor3_last_update_ms;
  uint64_t motor4_last_update_ms;

  uint64_t motor3_sequence;
  uint64_t motor4_sequence;
} motor_telemetry_snapshot_t;

typedef struct motor_telemetry motor_telemetry_t;

motor_telemetry_t* motor_telemetry_create(void);

bool motor_telemetry_start(motor_telemetry_t* telemetry, const char* can_interface);

bool motor_telemetry_get_snapshot(motor_telemetry_t* telemetry, motor_telemetry_snapshot_t* snapshot);

void motor_telemetry_stop(motor_telemetry_t* telemetry);

void motor_telemetry_destroy(motor_telemetry_t* telemetry);

#endif