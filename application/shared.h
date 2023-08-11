#ifndef SHARED_H_
#define SHARED_H_

#include <config/bsp_cfg.h>
#include <kernel/include/os.h>

typedef struct config {
  // Ethernet Configuration
  CPU_CHAR *eth_ip;
  CPU_CHAR *eth_netmask;
  CPU_CHAR *eth_gateway;
} config_t;

typedef struct data_packet {
  uint32_t unix_timestamp;
  uint32_t logger_serial;
  uint32_t com1_serial;
  int16_t com1_primary;
  int16_t com1_secondary;
  uint32_t com2_serial;
  int16_t com2_primary;
  int16_t com2_secondary;
  uint32_t com3_serial;
  int16_t com3_primary;
  int16_t com3_secondary;
  uint16_t battery;
} data_packet_t;

typedef enum display_msg_type {
  DISPLAY_MSG_TYPE_NONE,
  DISPLAY_MSG_TYPE_DATA,
  DISPLAY_MSG_TYPE_CONFIG,
  DISPLAY_MSG_TYPE_ERROR,
} display_msg_type_t;

typedef struct display_packet {
  display_msg_type_t type;
  union {
    data_packet_t data;
    config_t config;
    char *error;
  };
} display_packet_t;

extern OS_Q     *Ex_IqrfMsgQueue;
extern OS_Q     *Ex_DisplayMsgQueue;
extern config_t *deviceConfig;

#endif
