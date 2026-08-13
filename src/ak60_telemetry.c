#include "ak60_telemetry.h"

#include <stddef.h>

static float uint_to_float(
    uint32_t value,
    float minimum,
    float maximum,
    unsigned int bits)
{
    const uint32_t maximum_integer = (1U << bits) - 1U;
    const float span = maximum - minimum;

    return ((float)value * span / (float)maximum_integer) + minimum;
}

bool ak60_decode_telemetry(
    uint32_t can_id,
    const uint8_t *data,
    uint8_t dlc,
    ak60_telemetry_t *output)
{
    if (data == NULL || output == NULL)
    {
        return false;
    }

    if (dlc != 8U)
    {
        return false;
    }

    const uint8_t motor_id = data[0];

    /*
     * Aries-Vector sends command frames on the same CAN arbitration ID.
     * A valid motor response identifies itself in payload byte zero.
     */
    if (motor_id != (uint8_t)can_id)
    {
        return false;
    }

    const uint16_t position_raw =
        ((uint16_t)data[1] << 8U) |
        (uint16_t)data[2];

    const uint16_t velocity_raw =
        ((uint16_t)data[3] << 4U) |
        ((uint16_t)data[4] >> 4U);

    const uint16_t torque_raw =
        (((uint16_t)data[4] & 0x0FU) << 8U) |
        (uint16_t)data[5];

    output->motor_id = motor_id;

    output->position_rad = uint_to_float(
        position_raw,
        AK60_POSITION_MIN_RAD,
        AK60_POSITION_MAX_RAD,
        16U);

    output->velocity_rad_s = uint_to_float(
        velocity_raw,
        AK60_VELOCITY_MIN_RAD_S,
        AK60_VELOCITY_MAX_RAD_S,
        12U);

    output->torque_nm = uint_to_float(
        torque_raw,
        AK60_TORQUE_MIN_NM,
        AK60_TORQUE_MAX_NM,
        12U);

    output->temperature_c = (int)data[6] - 40;
    output->fault_code = data[7];

    return true;
}

const char *ak60_fault_string(uint8_t fault_code)
{
    switch (fault_code)
    {
        case 0:
            return "none";

        case 1:
            return "over temperature";

        case 2:
            return "over current";

        case 3:
            return "over voltage";

        case 4:
            return "under voltage";

        case 5:
            return "encoder fault";

        case 6:
            return "phase current fault";

        default:
            return "unknown";
    }
}