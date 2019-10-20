# stm32usb

WARNING: This is pre-alpha / proof of concept level code.

The STM32Core* repositories have been released under a BSD license.
However, some parts such as the USB Middleware are under a SLA0044
license, which has some restrictions that might be incompatible
with licenses such as GPL. 

This will be an attempt to write a high level USB implementation,
which rests on the low level implementation in the BSD-licensed HAL.
The target is to make it independant from the low level implementation,
so it can be used by other targets as well (eg. nrfx) 

The design will be inspired by libopencm3's USBD implementation.
Specifically, it will support being able to override the libraries
implementation in user code, so additional requests can be implemented.

