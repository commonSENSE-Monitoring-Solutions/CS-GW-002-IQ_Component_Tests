#ifndef IQRF_H
#define IQRF_H

#include <config/bsp_cfg.h>
#include <autogen/sl_spidrv_instances.h>
#include <cpu/include/cpu.h>
#include <common/include/common.h>
#include <kernel/include/os.h>
#include <kernel/include/os_trace.h>

#include <application/shared.h>
#include <ex_description.h>

#define IQRF_HANDLE             sl_spidrv_usart_spi_iqrf_handle

// IQRF timeouts (in microseconds).
#define IQRF_PRINT_TIMEOUT     100

// IQRF timeouts (in milliseconds).
#define IQRF_POLL_TIMEOUT      1000

// IQRF buffer lengths.
#define IQRF_BUF_LEN_MAX       64
#define IQRF_BUF_LEN_STATUS    1

#define INFO_REQ_LEN           16
#define SPI_CMD_HEADER         4
#define UNIX_TIME_LEN          4

void Ex_IqrfInit(void);

#endif
