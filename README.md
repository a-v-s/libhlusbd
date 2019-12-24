# libhlusbd - High Level USB Device Library 

# License: MIT

# About

This is a high level USB Device library. This implements the USB protocol,
and requires some low level device specific implementation to interface with.

It comes with some drivers to interface with STM32CubeF1 and NRFX.
In the [ucdev](https://github.com/a-v-s/ucdev/tree/master/demos/usbd) repo are demos for this library 
for the STM32F103CBT6 ("Blue Pill") and nRF52840. 


# State: Alpha 
    Basic functionality works, the device can enumerate, send and receive data
    on bulk and interrupt endpoints (with atm some manual configuration)

    Some features, such as multiple configurations, alternative interfaces, etc.
    are not implemented yet. This also includes automatic configuration of the
    endpoints, etc. 



