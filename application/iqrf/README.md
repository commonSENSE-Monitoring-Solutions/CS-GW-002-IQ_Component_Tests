# IQRF Driver

Driver order of operation:

1. Check radio module for status (BUSY, NOT BUSY, etc.)
2. Check radio module for data (returns number of bytes in buffer)
3. Fetch data from radio module based on number of bytes previously returned
4. Send current UNIX timestamp to radio module for syncing node times (also acts as a receipt)

## Resources

1. [IQRF SPI Guide](https://static.iqrf.org/Tech_Guide_IQRF_SPI_TR-7xD+7xG_230517.pdf)


## Current Issues

### 1. Can't read received data from IQRF

The application cannot currently read the data packet from the IQRF module, however status checks and requests for packet size seem to work fine. Example output from VCOM is shown below:

```
Polling IQRF
Reading 35 bytes of data: 
Req Packet: F0.23.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.8C.00.
Res Packet: 63.F0.23.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.8C.
```

The code for sending this command is on line `255` of the file `iqrf.c`.