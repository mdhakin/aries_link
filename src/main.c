
#define _POSIX_C_SOURCE 199309L
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "config_parser.h"
#include "drive.h"
#include "drive_limits.h"
#include "drive_parser.h"
#include "info_board.h"
#include "motor_telemetry.h"
#include "parser.h"
#include "ramp.h"
#include "serial.h"
#include "udp_tx.h"

#define ARIES_CONFIG_PATH "/etc/aries-link.conf"

static uint64_t monotonic_ms(void) {
  struct timespec now;

  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
    return 0;
  }

  return ((uint64_t)now.tv_sec * 1000ULL) + ((uint64_t)now.tv_nsec / 1000000ULL);
}

static int send_motion_packet(int udp_sock, const motion_state_t* state, unsigned int left_motor_id,
                              unsigned int right_motor_id) {
  char msg[256];

  snprintf(msg, sizeof(msg),
           "%u v %.2f\n"
           "%u kp %.2f\n"
           "%u kd %.2f\n"
           "%u t %.2f\n"
           "%u v %.2f\n"
           "%u kp %.2f\n"
           "%u kd %.2f\n"
           "%u t %.2f\n",
           left_motor_id, state->left_v, left_motor_id, state->kp, left_motor_id, state->kd,
           left_motor_id, state->torque,

           right_motor_id, state->right_v, right_motor_id, state->kp, right_motor_id, state->kd,
           right_motor_id, state->torque);

  return udp_send_text(udp_sock, msg);
}

static void sleep_ms(long milliseconds) {
  struct timespec ts;

  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000L;

  nanosleep(&ts, NULL);
}
static volatile sig_atomic_t keep_running = 1;

static void handle_signal(int signal_number) {
  (void)signal_number;
  keep_running = 0;
}

int main(void) {
  info_board_t board;
  motion_state_t current_state;

  drive_state_t current_drive;
  drive_state_t target_drive;
  drive_output_t drive_output;
  drive_ramp_config_t drive_ramp;
  drive_limits_t drive_limits;

  aries_config_t config;

  aries_config_set_defaults(&config);

  if (aries_config_load(ARIES_CONFIG_PATH, &config) != 0) {
    printf("Config file not found, using defaults.\n");
  } else {
    printf("Loaded config from %s\n", ARIES_CONFIG_PATH);
  }

  printf("Config: left_motor_id=%u right_motor_id=%u max_speed=%.2f\n", config.left_motor_id,
         config.right_motor_id, config.max_speed);

  info_board_init(&board);
  motion_state_init(&current_state);

  drive_state_init(&current_drive);
  drive_state_init(&target_drive);

  drive_ramp_config_init(&drive_ramp);

  drive_limits_init(&drive_limits, config.max_speed);

  drive_mix(&current_drive, &drive_limits, &drive_output);

  int serial_fd = serial_open("/dev/ttyUSB0", 9600);
  if (serial_fd < 0) {
    return 1;
  }

  int udp_sock = udp_open("127.0.0.1", 9750);
  if (udp_sock < 0) {
    serial_close(serial_fd);
    return 1;
  }

  printf("Aries-Link listening on /dev/ttyUSB0 at 9600...\n");

  motor_telemetry_t* motor_telemetry = motor_telemetry_create();

  if (motor_telemetry == NULL) {
    fprintf(stderr, "Failed to create motor telemetry.\n");
    return 1;
  }

  if (!motor_telemetry_start(motor_telemetry, "can0", config.left_motor_id,
                             config.right_motor_id)) {
    fprintf(stderr, "Failed to start motor telemetry.\n");

    motor_telemetry_destroy(motor_telemetry);
    return 1;
  }

  uint64_t last_telemetry_print_ms = 0;
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  while (keep_running) {
    char line[256];

    int result = serial_try_read_line(serial_fd, line, sizeof(line));

    if (result > 0) {
      printf("RX: %s\n", line);

      config_command_t config_command;

      const config_parse_result_t config_result = config_parse_command(line, &config_command);

      if (strcmp(line, "version") == 0) {
        char response[128];

        snprintf(response, sizeof(response), "ok version %s", board.version);

        serial_write_line(serial_fd, response);
      } else if (strcmp(line, "telemetry") == 0) {
        motor_telemetry_snapshot_t snapshot;

        if (!motor_telemetry_get_snapshot(motor_telemetry, &snapshot)) {
          serial_write_line(serial_fd, "err telemetry unavailable");
        } else if (!snapshot.motor3_valid || !snapshot.motor4_valid) {
          serial_write_line(serial_fd, "err telemetry not ready");
        } else {
          char response[256];

          snprintf(
              response, sizeof(response),
              "ok telemetry "
              "M%u pos=%.2f vel=%.2f torque=%.2f temp=%d fault=%u "
              "M%u pos=%.2f vel=%.2f torque=%.2f temp=%d fault=%u",
              config.left_motor_id, snapshot.motor3.position_rad, snapshot.motor3.velocity_rad_s,
              snapshot.motor3.torque_nm, snapshot.motor3.temperature_c, snapshot.motor3.fault_code,
              config.right_motor_id, snapshot.motor4.position_rad, snapshot.motor4.velocity_rad_s,
              snapshot.motor4.torque_nm, snapshot.motor4.temperature_c, snapshot.motor4.fault_code);

          serial_write_line(serial_fd, response);
        }
      } else {
        if (config_result == CONFIG_PARSE_SHOW) {
          char response[128];

          snprintf(response, sizeof(response),
                   "ok config left_motor_id=%u right_motor_id=%u max_speed=%.2f",
                   config.left_motor_id, config.right_motor_id, config.max_speed);

          serial_write_line(serial_fd, response);
        } else if (config_result == CONFIG_PARSE_SET_MAX_SPEED) {
          if (config_command.max_speed <= 0.0f) {
            serial_write_line(serial_fd, "err config invalid max_speed");
          } else {
            config.max_speed = config_command.max_speed;

            drive_limits_init(&drive_limits, config.max_speed);

            if (aries_config_save(ARIES_CONFIG_PATH, &config) != 0) {
              serial_write_line(serial_fd, "err config save failed");
            } else {
              serial_write_line(serial_fd, "ok config max_speed");
            }
          }
        } else if (config_result == CONFIG_PARSE_SET_MOTORS) {
          if (config_command.left_motor_id == 0 || config_command.right_motor_id == 0 ||
              config_command.left_motor_id == config_command.right_motor_id) {
            serial_write_line(serial_fd, "err config invalid motor ids");

          } else {
            aries_config_t pending_config = config;

            pending_config.left_motor_id = config_command.left_motor_id;
            pending_config.right_motor_id = config_command.right_motor_id;

            if (aries_config_save(ARIES_CONFIG_PATH, &pending_config) != 0) {
              serial_write_line(serial_fd, "err config save failed");
            } else {
              serial_write_line(serial_fd, "ok config motor restart_required");
            }
          }
        } else {
          drive_parse_result_t drive_result = drive_parse_command(line, &target_drive);

          if (drive_result == DRIVE_PARSE_OK) {
            drive_apply_limits(&target_drive, &drive_limits);

            printf("Drive target: speed=%.2f turn=%.2f\n", target_drive.speed, target_drive.turn);

            serial_write_line(serial_fd, "ok drive");
          } else {
            serial_write_line(serial_fd, "err unknown command");
          }
        }
      }
    }

    drive_update(&current_drive, &target_drive, &drive_ramp);

    drive_mix(&current_drive, &drive_limits, &drive_output);

    /*
     * Adapt the high-level drive output into the existing
     * motion_state_t expected by send_motion_packet().
     *
     * Keep kp, kd and torque from the initialized motion state.
     */
    current_state.left_v = drive_output.left_v;
    current_state.right_v = drive_output.right_v;

    if (send_motion_packet(udp_sock, &current_state, config.left_motor_id, config.right_motor_id) !=
        0) {
      printf("Failed to send motion packet.\n");
      break;
    }
    const uint64_t now_ms = monotonic_ms();

    if (now_ms - last_telemetry_print_ms >= 1000U) {
      motor_telemetry_snapshot_t snapshot;

      if (motor_telemetry_get_snapshot(motor_telemetry, &snapshot)) {
        if (snapshot.motor3_valid) {
          printf("M%u: pos=%.2f vel=%.2f torque=%.2f temp=%d fault=%u seq=%llu\n",
                 config.left_motor_id, snapshot.motor3.position_rad, snapshot.motor3.velocity_rad_s,
                 snapshot.motor3.torque_nm, snapshot.motor3.temperature_c,
                 snapshot.motor3.fault_code, (unsigned long long)snapshot.motor3_sequence);
        }

        if (snapshot.motor4_valid) {
          printf("M%u: pos=%.2f vel=%.2f torque=%.2f temp=%d fault=%u seq=%llu\n",
                 config.right_motor_id, snapshot.motor4.position_rad,
                 snapshot.motor4.velocity_rad_s, snapshot.motor4.torque_nm,
                 snapshot.motor4.temperature_c, snapshot.motor4.fault_code,
                 (unsigned long long)snapshot.motor4_sequence);
        }
      }

      last_telemetry_print_ms = now_ms;
    }
    sleep_ms(50);
  }

  udp_close(udp_sock);
  serial_close(serial_fd);
  motor_telemetry_destroy(motor_telemetry);
  return 0;
}