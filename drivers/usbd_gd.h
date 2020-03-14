// GigeDevice's includes
#include "usbd_core.h"
#include "usbd_int.h"
#include "usbd_pwr.h"
#include "usbd_regs.h"
#include "usbd_std.h"

int usbd_gd32_transmit(void *pcd_handle, uint8_t ep, void *data, size_t size);
int usbd_gd32_set_address(void *pcd_handle, uint8_t address);

int usbd_gd32_ep_set_stall(void *pcd_handle, uint8_t ep);

int usbd_gd32_ep_clear_stall(void *pcd_handle, uint8_t ep);
int usbd_gd32_ep_close(void *pcd_handle, uint8_t ep);
int usbd_gd32_ep_open(void *pcd_handle, uint8_t epnum, uint8_t epsize,
		uint8_t eptype);
