#include "iqrf.h"
#include "iqrf_def.h"

#include <string.h>
#include <stdio.h>

#include "spidrv.h"

#define IQRF_POLL_TASK_PRIORITY    1u
#define IQRF_POLL_TASK_STACK_SIZE  1024u
#define IQRF_MSG_QUEUE_SIZE        10u
#define DISPLAY_MSG_QUEUE_SIZE     10u
static  CPU_STK Ex_IqrfPollTaskStk[IQRF_POLL_TASK_STACK_SIZE];
static  OS_TCB  Ex_IqrfPollTaskTCB;
static  void    Ex_IqrfPollTask(void *p_arg);
static  OS_SEM  Ex_IqrfTxSem;

// IQRF buffers.
static char iqrf_tx_buffer[IQRF_BUF_LEN_MAX];
static char iqrf_rx_buffer[IQRF_BUF_LEN_MAX];
static char iqrf_dummy_bytes[IQRF_BUF_LEN_MAX];
static char iqrf_last_rf[IQRF_BUF_LEN_MAX];

/**
 * @brief Callback function for the IQRF SPI driver.
 * 
 * @param handle 
 * @param transferStatus 
 * @param itemsTransferred 
 */
void iqrfSpiTransferCb(
  SPIDRV_HandleData_t *handle,
  Ecode_t transferStatus,
  int itemsTransferred
) {
  RTOS_ERR err;
  PP_UNUSED_PARAM(handle);
  PP_UNUSED_PARAM(itemsTransferred);

  if (transferStatus == ECODE_EMDRV_SPIDRV_OK) {
    // Signal the IQRF TX semaphore.
    OSSemPost(&Ex_IqrfTxSem, OS_OPT_POST_1, &err);
  }
}

/**
 * @brief Take a string and format as an IQRF data packet.
 * Populates the iqrf_tx_buffer variable.
 * 
 * @param cmd         The command type
 * @param dataOp      The data operation type (read / write)
 * @param data        Data to format
 * @param dataLen     The length of the data
 * 
 * @return packetLen  The final length of the SPI packet
 */
static uint8_t iqrfFormSpiCmdPacket(
  uint8_t cmd,
  uint8_t dataOp,
  char data[],
  uint8_t dataLen
) {
  uint8_t i;
  uint8_t packetLen = dataLen + SPI_CMD_HEADER;
  uint8_t crcm = TR_CRC_START;
  uint8_t crc_index = packetLen - 2;

  // No need to clear the IQRF TX buffer as it was zeroed (set to
  // TR_CMD_SPI_CHECK) before this function was called.

  iqrf_tx_buffer[0] = cmd;
  iqrf_tx_buffer[1] = (dataOp | dataLen);

  // Copy the data to the IQRF TX buffer.
  for (i = 2; i < (dataLen + 2); i++) {
    iqrf_tx_buffer[i] = data[i - 2];
  }

  // Calculate the CRC for the data.
  for (i = 0; i < crc_index; i++) {
      crcm ^= iqrf_tx_buffer[i];
  }

  iqrf_tx_buffer[crc_index] = crcm;

  // No need to add a State Check to 'iqrf_tx_buffer[packetLen - 1]' as the
  // IQRF TX buffer was zeroed (set to TR_CMD_SPI_CHECK) before this function
  // was called.

  return packetLen;
}

/**
 * @brief             Check the CRC for a packet received from the IQRF module.
 * 
 * @return true       CRC calculation matches the received CRC
 * @return false      CRC calculation does not match the received CRC
 */
static bool iqrfCheckCrcSlave(void)
{
  bool crcs_ok = false;
  uint8_t crcs = TR_CRC_START;
  uint8_t dataLen;
  uint8_t packetLen;
  uint8_t crc_index;
  uint8_t i;

  // Get the 'dataLen' from the corresponding buffer byte.
  if (iqrf_rx_buffer[1] == TR_DATA_LEN_MIN) {
    dataLen = iqrf_rx_buffer[1];
  } else {
    dataLen = iqrf_rx_buffer[1] - TR_DATA_LEN_MIN;
  }

  packetLen = dataLen + SPI_CMD_HEADER;
  crc_index = packetLen - 2;

  // Calculate the CRC for the received data.
  for (i = 2; i < crc_index; i++) {
    crcs ^= iqrf_rx_buffer[i];
  }

  crcs ^= dataLen;

  if (crcs == iqrf_rx_buffer[crc_index]) {
    crcs_ok = true;
  }

  return crcs_ok;
}

void Ex_IqrfInit(void) {
  RTOS_ERR err;

  // Create the Transfer Semaphore
  OSSemCreate(&Ex_IqrfTxSem,
              "IQRF TX Semaphore",
              (OS_SEM_CTR)0,
              &err);

  // Create the Message Queue
  #ifdef CLOUD_TASK_ENABLED
  OSQCreate(Ex_IqrfMsgQueue,
            "IQRF Message Queue",
            IQRF_MSG_QUEUE_SIZE,
            &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  #endif
  #ifdef DISPLAY_TASK_ENABLED
  OSQCreate(Ex_DisplayMsgQueue,
            "Display Message Queue",
            DISPLAY_MSG_QUEUE_SIZE,
            &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
  #endif

  // Create the Task
  OSTaskCreate(&Ex_IqrfPollTaskTCB,
               "IQRF Poll Task",
               Ex_IqrfPollTask,
               DEF_NULL,
               IQRF_POLL_TASK_PRIORITY,
               &Ex_IqrfPollTaskStk[0],
               (IQRF_POLL_TASK_STACK_SIZE / 10u),
               IQRF_POLL_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);
}

/**
 * @brief Poll the IQRF module for data.
 * 
 * @param p_arg 
 */
static void Ex_IqrfPollTask(void *p_arg) {
  RTOS_ERR err;
  Ecode_t ecode;
  CPU_CHAR *pIqrfData;
  pIqrfData = malloc(64);
  PP_UNUSED_PARAM(p_arg);

  while (DEF_ON) {
    // Delay to allow slave to start
    // OSTimeDlyHMSM(0, 0, 10, 0, OS_OPT_TIME_DLY, &err);
    printf("Polling IQRF\n");

    bool crcs_ok = true;
    uint8_t dataLen = 0;
    uint8_t packetLen = 0;

    // Zero the IQRF buffers by setting them to TR_CMD_SPI_CHECK
    memset(iqrf_tx_buffer, TR_CMD_SPI_CHECK, IQRF_BUF_LEN_MAX);
    memset(iqrf_rx_buffer, TR_CMD_SPI_CHECK, IQRF_BUF_LEN_MAX);
    memset(iqrf_dummy_bytes, TR_CMD_SPI_CHECK, IQRF_BUF_LEN_MAX);

    // Test the IQRF Status
    ecode = SPIDRV_MTransfer(IQRF_HANDLE,
                             iqrf_tx_buffer,
                             iqrf_rx_buffer,
                             IQRF_BUF_LEN_STATUS,
                             iqrfSpiTransferCb);
    APP_RTOS_ASSERT_DBG((ecode == ECODE_EMDRV_SPIDRV_OK), ;);

    // Wait for the IQRF TX semaphore to be posted.
    OSSemPend(&Ex_IqrfTxSem,
              0,
              OS_OPT_PEND_BLOCKING,
              DEF_NULL,
              &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);

    packetLen = IQRF_BUF_LEN_STATUS;

    // Check the status of the module
    switch(iqrf_rx_buffer[0]) {
      // Deliberate fall through
      case TR_STAT_SPI_DISABLED:
      case TR_STAT_USER_STOP:
      case TR_STAT_CRCM_ERR:
      case TR_STAT_CRCM_OK:
      case TR_STAT_COM_MODE:
      case TR_STAT_PROG_MODE:
      case TR_STAT_DEBUG_MODE:
      case TR_STAT_SLOW_MODE:
      case TR_STAT_HW_ERR:
        break;
      default:
        // Check if there is a valid data length in the buffer
        if ((iqrf_rx_buffer[0] >= TR_DATA_LEN_MIN) &&
            (iqrf_rx_buffer[0] <= TR_DATA_LEN_MAX)) {
          if (iqrf_rx_buffer[0] == TR_DATA_LEN_MIN) {
            dataLen = iqrf_rx_buffer[0];
          } else {
            dataLen = iqrf_rx_buffer[0] - TR_DATA_LEN_MIN;
          }

          // Read DATA by sending a data request to the IQRF
          printf("Reading %d bytes of data: \n", dataLen);
          packetLen = iqrfFormSpiCmdPacket(TR_CMD_READ_WRITE,
                                          TR_CTYPE_READ,
                                          iqrf_dummy_bytes,
                                          dataLen);

          // Print the data
          printf("Req Packet: ");
          for (uint8_t i = 0; i < packetLen; i++) {
            printf("%02X.", iqrf_tx_buffer[i]);
          }
          printf("\n");

          ecode = SPIDRV_MTransfer(IQRF_HANDLE,
                              iqrf_tx_buffer,
                              iqrf_rx_buffer,
                              packetLen,
                              iqrfSpiTransferCb);
          //APP_RTOS_ASSERT_DBG((ecode == ECODE_EMDRV_SPIDRV_OK), ;);
          if (ecode != ECODE_EMDRV_SPIDRV_OK) {
              printf("SPI failed with code: %d\n", ecode);
              goto finish_task;
          }

          // Wait for the IQRF TX semaphore to be posted.
          OSSemPend(&Ex_IqrfTxSem,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    DEF_NULL,
                    &err);
          //APP_RTOS_ASSERT_DBG((ecode == ECODE_EMDRV_SPIDRV_OK), ;);
          if (ecode != ECODE_EMDRV_SPIDRV_OK) {
              printf("SPI failed with code: %d\n", ecode);
              goto finish_task;
          }

          // Print the data
          printf("Res Packet: ");
          for (uint8_t i = 0; i < packetLen; i++) {
            printf("%02X.", iqrf_rx_buffer[i]);
          }
          printf("\n");

          // Check the CRC
          crcs_ok = iqrfCheckCrcSlave();
        }
        break;
    }

    // Compare the Data with the last received packet
    if (packetLen > 1) {
      if (memcmp(iqrf_rx_buffer, iqrf_last_rf, packetLen) != 0) {
        // Copy the new data into the last received packet buffer
        memcpy(iqrf_last_rf, iqrf_rx_buffer, packetLen);
        // Copy the new data into the IQRf data buffer
        memcpy(pIqrfData, iqrf_rx_buffer, packetLen);

        // Print the data
        printf("Chgd Data:  ");
        for (uint8_t i = 0; i < packetLen; i++) {
          printf("%02X.", iqrf_rx_buffer[i]);
        }
        printf("\n");

        // Add the data to the message queue
        #ifdef CLOUD_TASK_ENABLED
        OSQPost(Ex_IqrfMsgQueue,
                &pIqrfData,
                packetLen,
                OS_OPT_POST_FIFO,
                &err);
        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
        #endif
        #ifdef DISPLAY_TASK_ENABLED
        // Copy the Data into a Display Packet Struct
        display_packet_t displayPacket;
        displayPacket.type = DISPLAY_MSG_TYPE_DATA;
        memcpy(&displayPacket.data, iqrf_rx_buffer, packetLen);
        OSQPost(Ex_DisplayMsgQueue,
                &displayPacket,
                packetLen,
                OS_OPT_POST_FIFO,
                &err);
        APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
        #endif
      }
    }

    // TODO: transmit the UNIX timestamp to the IQRF module

    // Delay for 1 second
    finish_task:
    OSTimeDly(10000,
              OS_OPT_TIME_DLY,
              &err);
    APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), ;);
  } // while (DEF_ON)
}
