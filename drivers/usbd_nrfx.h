#ifndef __USBD_NRFX__
#define __USBD_NRFX__

#include <stdint.h>
#include <stdlib.h>

int usbd_nrfx_transmit(void *pcd_handle, uint8_t ep, void *data, size_t size);
int usbd_nrfx_set_address(void *pcd_handle, uint8_t address);

int usbd_nrfx_ep_set_stall(void *pcd_handle, uint8_t ep);

int usbd_nrfx_ep_clear_stall(void *pcd_handle, uint8_t ep);
int usbd_nrfx_ep_close(void *pcd_handle, uint8_t ep);
int usbd_nrfx_ep_open(void *pcd_handle, uint8_t epnum, uint8_t epsize,
		uint8_t eptype);



#endif //__USBD_NRFX__
