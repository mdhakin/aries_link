#include "motor_telemetry.h"

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
#include <unistd.h>

struct motor_telemetry {
  int can_socket;

  pthread_t thread;

  pthread_mutex_t mutex;

  bool running;

  motor_telemetry_snapshot_t snapshot;
};
static void* motor_telemetry_thread(void* context);
static int open_can_socket(const char* interface_name) {
  if (interface_name == NULL) {
    return -1;
  }

  int socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

  if (socket_fd < 0) {
    perror("socket");
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
   * Receive only standard CAN frames for motors 3 and 4.
   */
  struct can_filter filters[2];

  filters[0].can_id = MOTOR_TELEMETRY_MOTOR_3;
  filters[0].can_mask = CAN_SFF_MASK;

  filters[1].can_id = MOTOR_TELEMETRY_MOTOR_4;
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

bool motor_telemetry_start(motor_telemetry_t* telemetry, const char* can_interface) {
  if (telemetry == NULL || can_interface == NULL) {
    return false;
  }

  /*
   * Do not open a second socket if this object
   * has already been started.
   */
  if (telemetry->can_socket >= 0) {
    return true;
  }

  telemetry->can_socket = open_can_socket(can_interface);
  telemetry->running = true;

  if (pthread_create(&telemetry->thread, NULL, motor_telemetry_thread, telemetry) != 0) {
    perror("pthread_create");

    close(telemetry->can_socket);

    telemetry->can_socket = -1;

    telemetry->running = false;

    return false;
  }

  if (telemetry->can_socket < 0) {
    return false;
  }

  printf("Motor telemetry opened CAN interface %s.\n", can_interface);

  return true;
}

bool motor_telemetry_get_snapshot(motor_telemetry_t* telemetry, motor_telemetry_snapshot_t* snapshot) {
  (void)telemetry;
  (void)snapshot;

  return false;
}

void motor_telemetry_stop(motor_telemetry_t* telemetry) {
  telemetry->running = false;

  if (telemetry->can_socket >= 0) {
    close(telemetry->can_socket);

    telemetry->can_socket = -1;
  }

  pthread_join(telemetry->thread, NULL);
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
    usleep(100000);
  }

  return NULL;
}