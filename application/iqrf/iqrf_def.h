#ifndef IQRF_DEF_H
#define IQRF_DEF_H

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

// TR Module SPI Status
#define TR_STAT_SPI_DISABLED   0x00    // SPI not active (disabled by the disableSPI() command).
#define TR_STAT_USER_STOP      0x07    // SPI suspended (by the stopSPI() command).
#define TR_STAT_CRCM_ERR       0x3E    // SPI not ready (buffer full, last CRCM error).
#define TR_STAT_CRCM_OK        0x3F    // SPI not ready (buffer full, last CRCM OK).
#define TR_STAT_COM_MODE       0x80    // SPI ready (communication mode).
#define TR_STAT_PROG_MODE      0x81    // SPI ready (programming mode).
#define TR_STAT_DEBUG_MODE     0x82    // SPI ready (debugging mode).
#define TR_STAT_SLOW_MODE      0x83    // SPI not working on background (slow mode).
#define TR_STAT_HW_ERR         0xFF    // SPI not active (HW error).

// TR Module SPI Commands
#define TR_CMD_SPI_CHECK       0x00    // Get the status of the TR module.
#define TR_CMD_READ_WRITE      0xF0    // All SPI ready modes: Data read / write (exchanged between the Master and bufferCOM at the Slave).
#define TR_CMD_DPA_WRITE       0xFA    // Communication mode only: The same as 0xF0. Intended for DPA. For DPA data read use 0xF0.
#define TR_CMD_GET_TR_INFO     0xF5    // Communication mode only: Get TR Module Info. 16 bytes are returned, but only the first 8 bytes are implemented in the OS.
#define TR_CMD_EEPROM_WRITE    0xF3    // Programming mode only: Write data to EEPROM. Write other TR configuration parameters.
#define TR_CMD_EEPROM_READ     0xF2    // Programming mode only: Read data from EEPROM. Read other TR configuration parameters.
#define TR_CMD_FLASH_WRITE     0xF6    // Programming mode only: Write data to EEEPROM or Flash. Read data from EEEPROM. Write HWP configuration.
#define TR_CMD_FLASH_VERIFY    0xFC    // Programming mode only: Verify data in Flash. Read HWP configuration.
#define TR_CMD_UPLOAD_IQRF     0xF9    // Programming mode only: Upload from proprietary secured .IQRF file provided by IQRF producer.

// TR Module special bytes
#define TR_DATA_LEN_MIN        0x40    // The minimum data length hex symbol.
#define TR_DATA_LEN_MAX        0x7F    // The maximum data length hex symbol.
#define TR_CRC_START           0x5F    // The starting value for all CRC calculations.
#define TR_CTYPE_WRITE         0x80    // The value indicating that an SPI_CMD packet is writing to the IQRF.
#define TR_CTYPE_READ          0x00    // The value indicating that an SPI_CMD packet is reading from the IQRF.

#define DATE_TIME_LENGTH       20

#endif
