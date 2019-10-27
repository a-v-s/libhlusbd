/*
 * usbd.h
 *
 *  Created on: 19 okt. 2019
 *      Author: andre
 */

#ifndef USBD_H_
#define USBD_H_

#include <stdlib.h>
#include <stdint.h>

struct usbd_handle_t;

#include "usb_descriptors.h"



typedef int (*usbd_transmit_f)(void* device_specific, uint8_t ep, void* data, size_t size);
typedef int (*usbd_set_address_f)(void* device_specific, uint8_t address);
typedef int (*usbd_ep_set_stall_f)(void* device_specific, uint8_t epnum);
typedef int (*usbd_ep_clear_stall_f)(void* device_specific, uint8_t epnum);
typedef int (*usbd_ep_close_f)(void* device_specific, uint8_t epnum);
typedef int (*usbd_ep_open_f)(void* device_specific, uint8_t epnum, uint8_t epsize, uint8_t eptype);


typedef struct {
	usbd_transmit_f transmit;
	usbd_set_address_f set_address;
	usbd_ep_set_stall_f ep_set_stall;
	usbd_ep_clear_stall_f ep_clear_stall;
	usbd_ep_open_f ep_open;
	usbd_ep_close_f ep_close;
	void*	device_specific;
} usbd_driver_t;

// As the usbd_transfer_cb_f is in usbd_endpoint_t which is in usbd_handle_t
// We get some circular problems, therefore I've put struct usbd_handle_t*
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
typedef void (*usbd_transfer_cb_f)(struct usbd_handle_t *handle,uint8_t ep, void* data, size_t size);
//typedef void (*usbd_transfer_cb_f)(usbd_handle_t *handle,uint8_t ep, void* data, size_t size);
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






// bmRequestType bit definitions
// Beyond Logic USB in a Nutshell - Chapter 6
// https://www.beyondlogic.org/usbnutshell/usb6.shtml
// bit 7 : Data Phase Transfer Direction
#define USB_REQTYPE_DIR_MASK			(0b1<<7)
#define USB_REQTYPE_DIR_IN				(0b1<<7)
#define USB_REQTYPE_DIR_OUT				(0b0<<7)
// bits 6..5 : Type
#define USB_REQTYPE_TYPE_MASK			(0b11<<5)
#define USB_REQTYPE_TYPE_STANDARD		(0b00<<5)
#define USB_REQTYPE_TYPE_CLASS			(0b01<<5)
#define USB_REQTYPE_TYPE_VENDOR			(0b10<<5)
// bits 4..0 : Recipient
#define USB_REQTYPE_RECIPIENT_MASK		(0b1111)
#define USB_REQTYPE_RECIPIENT_DEVICE	(0b0000)
#define USB_REQTYPE_RECIPIENT_INTERFACE	(0b0001)
#define USB_REQTYPE_RECIPIENT_ENDPOINT	(0b0010)
#define USB_REQTYPE_RECIPIENT_OTHER		(0b0011)





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
#define USB_REQ_SET_ENCRYPTION 		13
#define USB_REQ_GET_ENCRYPTION 		14
#define USB_REQ_SET_HANDSHAKE 		15
#define USB_REQ_GET_HANDSHAKE 		16
#define USB_REQ_SET_CONNECTION 		17
#define USB_REQ_SET_SECURITY_DATA 	18
#define USB_REQ_GET_SECURITY_DATA 	19
#define USB_REQ_SET_WUSB_DATA 		20
#define USB_REQ_LOOPBACK_DATA_WRITE 21
#define USB_REQ_LOOPBACK_DATA_READ 	22
#define USB_REQ_SET_INTERFACE_DS 	23
#define USB_REQ_SET_SEL 			48
#define USB_REQ_SET_ISOCH_DELAY 	49


// USB 2.0 specs Table 9-5. Descriptor Types
// USB 3.2 specs Table 9-6. Descriptor Types
#define USB_DT_DEVICE 1
#define USB_DT_CONFIGURATION 2
#define USB_DT_STRING 3
#define USB_DT_INTERFACE 4
#define USB_DT_ENDPOINT 5
#define USB_DT_DEVICE_QUALIFIER  6 // Defined in 2.0, Reserved in 3.2
#define USB_DT_OTHER_SPEED_CONFIGURATION 7 // Defined in 2.0, Reserved in 3.2
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
	  uint8_t   bmRequest;
	  uint8_t   bRequest;
	  uint16_t  wValue;
	  uint16_t  wIndex;
	  uint16_t  wLength;
} usb_setuprequest_t;

#include "usb_descriptors.h"
#pragma pack(pop)


typedef struct {
	void* buffer;
	size_t size;
	usbd_transfer_cb_f cb;
} usbd_endpoint_t;

typedef struct {
	usbd_driver_t driver;

	uint8_t usbd_descriptor_buffer[USBD_DESCRIPTOR_BUFFER_SIZE];
	size_t usbd_descriptor_buffer_offset;
	usb_descriptor_device_t * descriptor_device;
	usb_descriptor_configuration_t * descriptor_configuration[USBD_CONFIGURATION_COUNT];
	usb_descriptor_string_t * descriptor_string[USBD_STRING_COUNT];

	usbd_endpoint_t ep_in[USBD_ENDPOINTS_COUNT];
	usbd_endpoint_t ep_out[USBD_ENDPOINTS_COUNT];

	uint8_t configuration;

} usbd_handle_t;






int usbd_transmit(usbd_handle_t *handle,uint8_t ep, void* data, size_t size);
int usbd_set_address(usbd_handle_t *handle,uint8_t address);
int usbd_ep_set_stall(usbd_handle_t *handle,uint8_t epnum);
int usbd_ep_clear_stall(usbd_handle_t *handle, uint8_t epnum);
int usbd_ep_close(usbd_handle_t *handle,uint8_t epnum);
int usbd_ep_open(usbd_handle_t *handle,uint8_t epnum, uint8_t epsize, uint8_t eptype);

int usbd_handle_standard_setup_request(usbd_handle_t *handle, usb_setuprequest_t *req);
void usbd_demo_setup_descriptors(usbd_handle_t *handle);

#endif /* USBD_H_ */
