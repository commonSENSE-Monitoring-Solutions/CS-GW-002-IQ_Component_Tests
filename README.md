# CS-GW-002-IQ Peripheral Tests

This repo contains the peripheral tests for the new CS-GW-002-IQ gateway device, based on the EFM32 Giant Gecko MCU.

## Current Issues

### 1. Can't read received data from IQRF

The application cannot currently read the data packet from the IQRF module, however status checks and requests for packet size seem to work fine. Example output from VCOM is shown below:

```
Polling IQRF
Reading 35 bytes of data: 
Req Packet: F0.23.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.8C.00.
Res Packet: 63.F0.23.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.8C.
```

### Micrium OS Network Example

This example shows how to use the Micrium OS network stack with the ETH 
peripheral on the EFM32GG11B starter kit. This example will initialize 
the RMII interface to the external and setup a 100 Mbit connection.

The example will output messages on the VCOM port and it will show status 
on the memory LCD display on the kit. The display will show the current 
link status along with the current IPv4 address and the currently 
configured IPv6 addresses.

Micrium OS Support SEGGER SystemView to view the runtime behavior or a system.
SystemView Trace is enabled by default when the segger_systemview component
is included in the project. SystemView can be used to inspect the runtime
behaviour of this example. It will give an overview
of the tasks and interrupts in the application. SystemView can be downloaded 
from https://www.segger.com/systemview.html.