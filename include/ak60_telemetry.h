#ifndef AK60_TELEMETRY_H
#define AK60_TELEMETRY_H

#include <stdbool.h>
#include <stdint.h>

#define AK60_POSITION_MIN_RAD   (-12.5f)
#define AK60_POSITION_MAX_RAD   (12.5f)

#define AK60_VELOCITY_MIN_RAD_S (-45.0f)
#define AK60_VELOCITY_MAX_RAD_S (45.0f)

#define AK60_TORQUE_MIN_NM      (-15.0f)
#define AK60_TORQUE_MAX_NM      (15.0f)

typedef struct
{
    uint8_t motor_id;

    float position_rad;
    float velocity_rad_s;
    float torque_nm;

    int temperature_c;
    uint8_t fault_code;
} ak60_telemetry_t;

/**
 * Decode an 8-byte AK60 MIT-mode telemetry response.
 *
 * Returns false when:
 * - data is NULL
 * - output is NULL
 * - DLC is not 8
 * - the first payload byte does not match the CAN ID
 */
bool ak60_decode_telemetry(
    uint32_t can_id,
    const uint8_t *data,
    uint8_t dlc,
    ak60_telemetry_t *output);

const char *ak60_fault_string(uint8_t fault_code);

#endif