#ifndef DRIVE_LIMITS_H
#define DRIVE_LIMITS_H

#include "drive.h"

void drive_limits_init(drive_limits_t *limits);

void drive_apply_limits(
    drive_state_t *state,
    const drive_limits_t *limits);

#endif