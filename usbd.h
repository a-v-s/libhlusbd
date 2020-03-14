/*

File: 		usbd.h
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

#ifndef USBD_H_
#define USBD_H_

#include <stdlib.h>
#include <stdint.h>

struct bscp_usbd_handle_t;

#include "usb_descriptors.h"

typedef int (*bscp_usbd_transmit_f)(void *device_specific, uint8_t ep, void *data,
		size_t size);
typedef int (*bscp_usbd_set_address_f)(void *device_specific, uint8_t address);
typedef int (*bscp_usbd_ep_set_stall_f)(void *device_specific, uint8_t epnum);
typedef int (*bscp_usbd_ep_clear_stall_f)(void *device_specific, uint8_t epnum);
typedef int (*bscp_usbd_ep_close_f)(void *device_specific, uint8_t epnum);
typedef int (*bscp_usbd_ep_open_f)(void *device_specific, uint8_t epnum,
		uint8_t epsize, uint8_t eptype);

typedef struct {
	bscp_usbd_transmit_f transmit;
	bscp_usbd_set_address_f set_address;
	bscp_usbd_ep_set_stall_f ep_set_stall;
	bscp_usbd_ep_clear_stall_f ep_clear_stall;
	bscp_usbd_ep_open_f ep_open;
	bscp_usbd_ep_close_f ep_close;
	void *device_specific;
	void *driver_config;
} bscp_usbd_driver_t;

// As the bscp_usbd_transfer_cb_f is in bscp_usbd_endpoint_t which is in bscp_usbd_handle_t
// We get some circular problems, therefore I've put struct bscp_usbd_handle_t*
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
typedef void (*bscp_usbd_transfer_cb_f)(struct bscp_usbd_handle_t *handle, uint8_t ep,
		void *data, size_t size);
//typedef void (*bscp_usbd_transfer_cb_f)(bscp_usbd_handle_t *handle,uint8_t ep, void* data, size_t size);
#pragma GCC diagnostic pop

#ifndef USBD_DESCRIPTOR_BUFFER_SIZE
#define USBD_DESCRIPTOR_BUFFER_SIZE 512
#endif

#ifndef USBD_ENDPOINTS_COUNT
#define USBD_ENDPOINTS_COUNT 8
#endif

#ifndef USBD_CONFIGURATION_COUNT
#define USBD_CONFIGURATION_COUNT 1
#endif

#ifndef USBD_STRING_COUNT
#define USBD_STRING_COUNT 6
#endif

#ifndef USBD_UNICODE_CONVERSION_ENABLED
#define USBD_UNICODE_CONVERSION_ENABLED 1
#endif

#ifndef USBD_REQUEST_HANDLER_COUNT
#define USBD_REQUEST_HANDLER_COUNT 4
#endif

// bmRequestType bit definitions
// Beyond Logic USB in a Nutshell - Chapter 6
// https://www.beyondlogic.org/usbnutshell/usb6.shtml
// bit 7 : Data Phase Transfer Direction
#define USB_REQTYPE_DIR_IN				(1)
#define USB_REQTYPE_DIR_OUT				(0)
// bits 6..5 : Type)
#define USB_REQTYPE_TYPE_STANDARD		(0)
#define USB_REQTYPE_TYPE_CLASS			(1)
#define USB_REQTYPE_TYPE_VENDOR			(2)
// bits 4..0 : Recipient
#define USB_REQTYPE_RECIPIENT_DEVICE	(0)
#define USB_REQTYPE_RECIPIENT_INTERFACE	(1)
#define USB_REQTYPE_RECIPIENT_ENDPOINT	(2)
#define USB_REQTYPE_RECIPIENT_OTHER		(3)

// USB 2.0 specs Table 9-4 Standard Request Codes
#define USB_REQ_GET_STATUS			(0)
#define USB_REQ_CLEAR_FEATURE		(1)
// Reserved (2)
#define USB_REQ_SET_FEATURE			(3)
// Reserved (3)
#define USB_REQ_SET_ADDRESS			(5)
#define USB_REQ_GET_DESCRIPTOR		(6)
#define USB_REQ_SET_DESCRIPTOR		(7)
#define USB_REQ_GET_CONFIGURATION	(8)
#define USB_REQ_SET_CONFIGURATION	(9)
#define USB_REQ_GET_INTERFACE		(10)
#define USB_REQ_SET_INTERFACE		(11)
#define USB_REQ_SYNCH_FRAME			(12)
// Added in USB 3.2
// USB 3.2 specs Table 9-5
#define USB_REQ_SET_ENCRYPTION 		(13)
#define USB_REQ_GET_ENCRYPTION 		(14)
#define USB_REQ_SET_HANDSHAKE 		(15)
#define USB_REQ_GET_HANDSHAKE 		(16)
#define USB_REQ_SET_CONNECTION 		(17)
#define USB_REQ_SET_SECURITY_DATA 	(18)
#define USB_REQ_GET_SECURITY_DATA 	(19)
#define USB_REQ_SET_WUSB_DATA 		(20)
#define USB_REQ_LOOPBACK_DATA_WRITE (21)
#define USB_REQ_LOOPBACK_DATA_READ 	(22)
#define USB_REQ_SET_INTERFACE_DS 	(23)
#define USB_REQ_SET_SEL 			(48)
#define USB_REQ_SET_ISOCH_DELAY 	(49)

// USB 2.0 specs Table 9-5. Descriptor Types
// USB 3.2 specs Table 9-6. Descriptor Types
#define USB_DT_DEVICE 1
#define USB_DT_CONFIGURATION 2
#define USB_DT_STRING 3
#define USB_DT_INTERFACE 4
#define USB_DT_ENDPOINT 5
#define USB_DT_DEVICE_QUALIFIER  6 			// Defined in 2.0, Reserved in 3.2
#define USB_DT_OTHER_SPEED_CONFIGURATION 7 	// Defined in 2.0, Reserved in 3.2
#define USB_DT_INTERFACE_POWER 1 8
#define USB_DT_OTG 9
#define USB_DT_DEBUG 10
#define USB_DT_INTERFACE_ASSOCIATION 11
#define USB_DT_BOS 15
#define USB_DT_DEVICE_CAPABILITY 16
#define USB_DT_SUPERSPEED_USB_ENDPOINT_COMPANION 48
#define USB_DT_SUPERSPEEDPLUS_ISOCHRONOUS_ENDPOINT_COMPANION 49

// USB 2.0 specs Table 9-13. Standard Endpoint Descriptor
#define USB_EP_ATTR_TYPE_CONTROL		0b00
#define USB_EP_ATTR_TYPE_ISOCHRONOUS	0b01
#define USB_EP_ATTR_TYPE_BULK			0b10
#define USB_EP_ATTR_TYPE_INTERRUPT		0b11
// TODO: SYNCHRONISATION AND USAGE TYPES
//	When are they used? Afaik only for ISO, which won't be added at this time.

#pragma pack(push,1)

typedef struct {
	unsigned int Recipient :5;
	unsigned int Type :2;
	unsigned int Direction :1;
} bmRequest_t;

typedef struct {
	bmRequest_t bmRequest;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} usb_setuprequest_t;

#include "usb_descriptors.h"
#pragma pack(pop)

typedef struct {
	void *data_buffer;
	size_t data_size;
	size_t data_left;
	bscp_usbd_transfer_cb_f data_cb;
	size_t ep_size;
	uint8_t ep_type;
	usb_descriptor_endpoint_t * desc;
} bscp_usbd_endpoint_t;


typedef enum {
	RESULT_NEXT_PARSER = 0,
	RESULT_HANDLED = 1,
	RESULT_REJECTED = 2,
} bscp_usbd_handler_result_t;

typedef bscp_usbd_handler_result_t (* bscp_usbd_request_handler)(void *handle,
		usb_setuprequest_t *req, void **buf, size_t *size);

typedef struct {
	bscp_usbd_driver_t driver;

	uint8_t usbd_descriptor_buffer[USBD_DESCRIPTOR_BUFFER_SIZE];
	size_t usbd_descriptor_buffer_offset;
	usb_descriptor_device_t *descriptor_device;
	usb_descriptor_configuration_t *descriptor_configuration[USBD_CONFIGURATION_COUNT];
	usb_descriptor_string_t *descriptor_string[USBD_STRING_COUNT];

	bscp_usbd_endpoint_t ep_in[USBD_ENDPOINTS_COUNT];
	bscp_usbd_endpoint_t ep_out[USBD_ENDPOINTS_COUNT];

	bscp_usbd_request_handler request_handlers[USBD_REQUEST_HANDLER_COUNT];

	uint8_t configuration;
	uint8_t interface;

} bscp_usbd_handle_t;




int bscp_usbd_transmit(bscp_usbd_handle_t *handle, uint8_t ep, void *data, size_t size);
int bscp_usbd_set_address(bscp_usbd_handle_t *handle, uint8_t address);
int bscp_usbd_ep_set_stall(bscp_usbd_handle_t *handle, uint8_t epnum);
int bscp_usbd_ep_clear_stall(bscp_usbd_handle_t *handle, uint8_t epnum);
int bscp_usbd_ep_close(bscp_usbd_handle_t *handle, uint8_t epnum);
int bscp_usbd_ep_open(bscp_usbd_handle_t *handle, uint8_t epnum, uint8_t epsize,
		uint8_t eptype);

bscp_usbd_handler_result_t bscp_usbd_handle_request(bscp_usbd_handle_t *handle, usb_setuprequest_t *req);
void bscp_usbd_demo_setup_descriptors(bscp_usbd_handle_t *handle);

int bscp_usbd_request_handler_add(bscp_usbd_handler_result_t* handler);

#endif /* USBD_H_ */
