#ifndef __USBD_STM__
#define __USBD_STM__

//#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal.h"

#include "stm32f1xx_ll_usb.h"
#include "stm32f1xx_hal_pcd.h"
#include "stm32f1xx_hal_pcd_ex.h"



int usbd_stm32_transmit(uint8_t ep, void* data, size_t size);
int usbd_stm32_set_address(uint8_t address);

int usbd_stm32_set_stall(uint8_t ep);

int usbd_stm32_clear_stall(uint8_t ep);

#endif //USBD_STM
