#ifndef __USBD_STM__
#define __USBD_STM__

//#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal.h"

#include "stm32f1xx_ll_usb.h"
#include "stm32f1xx_hal_pcd.h"
#include "stm32f1xx_hal_pcd_ex.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	GPIO_TypeDef* 	PullUpPort;
	uint16_t		PullUpPin;
	bool			PullUpInverted;

} usbd_stm32_usbfs_v1_config;

int usbd_stm32_transmit(void *pcd_handle, uint8_t ep, void *data, size_t size);
int usbd_stm32_set_address(void *pcd_handle, uint8_t address);

int usbd_stm32_ep_set_stall(void *pcd_handle, uint8_t ep);

int usbd_stm32_ep_clear_stall(void *pcd_handle, uint8_t ep);
int usbd_stm32_ep_close(void *pcd_handle, uint8_t ep);
int usbd_stm32_ep_open(void *pcd_handle, uint8_t epnum, uint8_t epsize,
		uint8_t eptype);

#endif //USBD_STM
