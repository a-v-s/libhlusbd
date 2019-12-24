/*

File: 		usbd_nrfx.h
Author:		André van Schoubroeck
License:	MIT


MIT License

Copyright (c) 2018, 2019 André van Schoubroeck

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


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
