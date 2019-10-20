/*
 * usbd.c
 *
 *  Created on: 19 okt. 2019
 *      Author: andre
 */

// TODO:
//		* Create some "driver" structure with pointers to device specific functions
//		* Create libopencm3-like callback mechanisms that allows overriding.
#include "usbd.h"

int usbd_handle_standard_setup_request(usbd_handle_t *handle,
		usb_setuprequest_t *req) {
	switch (req->bRequest) {
	case USB_REQ_GET_STATUS: {
		static char status[2] = { 0, 0 };
		handle->transmit(0x80, status, 2);
		return 0;
	}
		break;
	case USB_REQ_SET_INTERFACE:
		break;
	case USB_REQ_CLEAR_FEATURE:
		break;
	case USB_REQ_SET_FEATURE:
		break;
	case USB_REQ_SET_ADDRESS:
		// TODO Why &0x7F mask
		//handle->set_address(req->wValue&0x7F);
		handle->set_address(req->wValue);
		// Send empty packet to acknowledge:
		// TODO, parse the data to send forwards
		handle->transmit(0x00, NULL, 0);
		break;
	case USB_REQ_GET_DESCRIPTOR:
		// Todo the abstraction, end up in usb_control_send_chunk()
		// usb_control_send_chunk() is the libopencm3 version
		switch (req->wValue >> 8) {
		case USB_DT_DEVICE: {
			// Just a little test... this makes no sense but
			// at least should show up something, right?
			static usb_descriptor_device_t test = { 0 };
			test.bLength = sizeof(test);
			test.bDescriptorType = USB_DT_DEVICE;
			test.bMaxPacketSize0 = 64;
			test.bNumConfigurations = 1;
			test.bcdUSB = 0x0110;
			test.idVendor = 0xdead;
			test.idProduct = 0xbeef;
			test.bDeviceClass = 0xff;
			test.bDeviceProtocol = 0xff;
			test.bDeviceSubClass = 0xff;
			test.bcdDevice = 1;
			handle->transmit(0x00, &test, test.bLength);
			return 0;
		}
		case USB_DT_CONFIGURATION: {
#pragma pack(push,1)
			static struct test {
				usb_descriptor_configuration_t config;
				usb_descriptor_interface_t iface;
				usb_descriptor_endpoint_t ep[2];
			} test = { 0 };
#pragma pack(pop)
			test.config.wTotalLength = sizeof(test);
			test.config.bLength = sizeof(test.config);
			test.config.bDescriptorType = USB_DT_CONFIGURATION;
			test.config.bConfigurationValue = 1;
			test.config.bMaxPower = 10;
			test.config.bmAttributes = 0x80;
			test.config.bNumInterfaces = 1;

			test.iface.bLength = sizeof(test.iface);
			test.iface.bDescriptorType = USB_DT_INTERFACE;
			test.iface.bInterfaceProtocol = 0xff;
			test.iface.bNumEndpoints = 2;
			test.iface.bInterfaceNumber = 0;
			test.iface.bAlternateSetting = 0;

			test.ep[0].bLength = test.ep[1].bLength = sizeof(test.ep[0]);
			test.ep[0].bDescriptorType = test.ep[1].bDescriptorType =
					USB_DT_ENDPOINT;
			test.ep[0].bmAttributes = test.ep[1].bmAttributes = 2;
			test.ep[0].wMaxPacketSize = test.ep[1].wMaxPacketSize = 64;
			test.ep[0].bEndpointAddress = 0x81;
			test.ep[1].bEndpointAddress = 0x01;

			size_t size =
					(req->wLength < test.config.wTotalLength) ?
							req->wLength : test.config.wTotalLength;

			// What happens if we truncate?
			handle->transmit(0x00, &test, size);
			// It wants me to answer only 9 ???
			//handle->transmit(0x00,&test, 9);
			return 0;
		}
			break;

			//case USB_DT_INTERFACE: not requestable
			//case USB_DT_ENDPOINT: not requestable
		case USB_DT_STRING: {
			// Only 0 for now
			static usb_descriptor_string_t only0 = { 0 };
			only0.bDescriptorType = USB_DT_STRING;
			only0.bLength = 6;
			only0.wLANGID[0] = 0x0409;
			handle->transmit(0x00, &only0, only0.bLength);

			break;
		}

			//handle->transmit(); //// EP, DATA, SIZE
		}
		break;
	case USB_REQ_SET_DESCRIPTOR:
		break;
	case USB_REQ_GET_CONFIGURATION:
		break;
	case USB_REQ_SET_CONFIGURATION:
		// No configuration support yet
		// Just send an ack
		handle->transmit(0x00, NULL, 0);
		break;
	case USB_REQ_GET_INTERFACE:
		break;
	}
	return 0;

}

