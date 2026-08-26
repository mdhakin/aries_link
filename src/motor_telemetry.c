#include "motor_telemetry.h"

#include <errno.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

struct motor_telemetry {
  int can_socket;

  pthread_t thread;
  pthread_mutex_t mutex;

  bool running;
  bool thread_started;

  unsigned int left_motor_id;
  unsigned int right_motor_id;

  motor_telemetry_snapshot_t snapshot;
};

static uint64_t monotonic_ms(void) {
  struct timespec now;

  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
    return 0;
  }

  return ((uint64_t)now.tv_sec * 1000ULL) + ((uint64_t)now.tv_nsec / 1000000ULL);
}

static void* motor_telemetry_thread(void* context);

static int open_can_socket(const char* interface_name, unsigned int left_motor_id,
                           unsigned int right_motor_id) {
  if (interface_name == NULL) {
    return -1;
  }

  int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

  if (socket_fd < 0) {
    perror("socket");
    return -1;
  }

  struct timeval timeout;

  timeout.tv_sec = 0;
  timeout.tv_usec = 250000;  // 250 ms

  if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    perror("setsockopt SO_RCVTIMEO");
    close(socket_fd);
    return -1;
  }

  struct ifreq interface_request;
  memset(&interface_request, 0, sizeof(interface_request));

  if (strlen(interface_name) >= IFNAMSIZ) {
    fprintf(stderr, "CAN interface name is too long: %s\n", interface_name);

    close(socket_fd);
    return -1;
  }

  strncpy(interface_request.ifr_name, interface_name, IFNAMSIZ - 1);

  if (ioctl(socket_fd, SIOCGIFINDEX, &interface_request) < 0) {
    perror("ioctl SIOCGIFINDEX");
    close(socket_fd);
    return -1;
  }

  /*
   * Receive only standard CAN frames for the configured drive motors.
   */
  struct can_filter filters[2];

  filters[0].can_id = left_motor_id;
  filters[0].can_mask = CAN_SFF_MASK;

  filters[1].can_id = right_motor_id;
  filters[1].can_mask = CAN_SFF_MASK;

  if (setsockopt(socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, filters, sizeof(filters)) < 0) {
    perror("setsockopt CAN_RAW_FILTER");
    close(socket_fd);
    return -1;
  }

  struct sockaddr_can address;
  memset(&address, 0, sizeof(address));

  address.can_family = AF_CAN;
  address.can_ifindex = interface_request.ifr_ifindex;

  if (bind(socket_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind");
    close(socket_fd);
    return -1;
  }

  return socket_fd;
}
motor_telemetry_t* motor_telemetry_create(void) {
  motor_telemetry_t* telemetry = malloc(sizeof(*telemetry));

  if (telemetry == NULL) {
    return NULL;
  }

  memset(telemetry, 0, sizeof(*telemetry));

  telemetry->can_socket = -1;

  if (pthread_mutex_init(&telemetry->mutex, NULL) != 0) {
    free(telemetry);
    return NULL;
  }

  return telemetry;
}

bool motor_telemetry_start(motor_telemetry_t* telemetry, const char* can_interface,
                           unsigned int left_motor_id, unsigned int right_motor_id) {
  if (telemetry == NULL || can_interface == NULL) {
    return false;
  }
  telemetry->left_motor_id = left_motor_id;
  telemetry->right_motor_id = right_motor_id;

  if (telemetry->thread_started) {
    return true;
  }

  telemetry->can_socket =
      open_can_socket(can_interface, telemetry->left_motor_id, telemetry->right_motor_id);

  if (telemetry->can_socket < 0) {
    return false;
  }

  telemetry->running = true;

  if (pthread_create(&telemetry->thread, NULL, motor_telemetry_thread, telemetry) != 0) {
    perror("pthread_create");

    telemetry->running = false;

    close(telemetry->can_socket);
    telemetry->can_socket = -1;

    return false;
  }

  telemetry->thread_started = true;

  printf("Motor telemetry opened CAN interface %s.\n", can_interface);

  return true;
}

bool motor_telemetry_get_snapshot(motor_telemetry_t* telemetry,
                                  motor_telemetry_snapshot_t* snapshot) {
  if (telemetry == NULL || snapshot == NULL) {
    return false;
  }

  pthread_mutex_lock(&telemetry->mutex);

  *snapshot = telemetry->snapshot;

  pthread_mutex_unlock(&telemetry->mutex);

  return true;
}

void motor_telemetry_stop(motor_telemetry_t* telemetry) {
  if (telemetry == NULL) {
    return;
  }

  if (!telemetry->thread_started) {
    return;
  }

  telemetry->running = false;

  if (telemetry->can_socket >= 0) {
    close(telemetry->can_socket);
    telemetry->can_socket = -1;
  }

  pthread_join(telemetry->thread, NULL);

  telemetry->thread_started = false;
}

void motor_telemetry_destroy(motor_telemetry_t* telemetry) {
  if (telemetry == NULL) {
    return;
  }

  motor_telemetry_stop(telemetry);

  pthread_mutex_destroy(&telemetry->mutex);

  free(telemetry);
}

static void* motor_telemetry_thread(void* context) {
  motor_telemetry_t* telemetry = context;

  while (telemetry->running) {
    struct can_frame frame;

    const ssize_t bytes_read = read(telemetry->can_socket, &frame, sizeof(frame));

    if (bytes_read < 0) {
      if (!telemetry->running) {
        break;
      }

      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }

      perror("motor telemetry CAN read");
      continue;
    }

    if ((size_t)bytes_read != sizeof(frame)) {
      continue;
    }

    if ((frame.can_id & CAN_EFF_FLAG) != 0U || (frame.can_id & CAN_RTR_FLAG) != 0U ||
        (frame.can_id & CAN_ERR_FLAG) != 0U) {
      continue;
    }

    const uint32_t can_id = frame.can_id & CAN_SFF_MASK;

    ak60_telemetry_t decoded;

    if (!ak60_decode_telemetry(can_id, frame.data, frame.can_dlc, &decoded)) {
      continue;
    }

    const uint64_t now_ms = monotonic_ms();

    pthread_mutex_lock(&telemetry->mutex);

    if (decoded.motor_id == telemetry->left_motor_id) {
      telemetry->snapshot.motor3 = decoded;
      telemetry->snapshot.motor3_valid = true;
      telemetry->snapshot.motor3_last_update_ms = now_ms;
      telemetry->snapshot.motor3_sequence++;
    } else if (decoded.motor_id == telemetry->right_motor_id) {
      telemetry->snapshot.motor4 = decoded;
      telemetry->snapshot.motor4_valid = true;
      telemetry->snapshot.motor4_last_update_ms = now_ms;
      telemetry->snapshot.motor4_sequence++;
    }

    pthread_mutex_unlock(&telemetry->mutex);
  }

  return NULL;
}