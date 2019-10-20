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
#include "usbd_descriptors.h"
// This is a temporary function to test the dynamic descriptor setup,
//
void usbd_setup_descriptors(usbd_handle_t *handle) {
	handle->descriptor_device = add_descriptor(handle, sizeof(usb_descriptor_device_t));
	handle->descriptor_device->bDescriptorType = USB_DT_DEVICE;
	handle->descriptor_device->bMaxPacketSize0 = 64;
	handle->descriptor_device->bNumConfigurations = 1;
	handle->descriptor_device->bcdUSB = 0x0100;
	handle->descriptor_device->idVendor = 0xdead;
	handle->descriptor_device->idProduct = 0xbeef;

	handle->descriptor_configuration = add_descriptor(handle,
			sizeof(usb_descriptor_configuration_t));
	handle->descriptor_configuration->wTotalLength =
			handle->descriptor_configuration->bLength;
	handle->descriptor_configuration->bDescriptorType = USB_DT_CONFIGURATION;
	handle->descriptor_configuration->bConfigurationValue = 1;
	handle->descriptor_configuration->bMaxPower = 10;
	handle->descriptor_configuration->bmAttributes = 0x80;
	handle->descriptor_configuration->bNumInterfaces = 1;

	usb_descriptor_interface_t *iface = add_descriptor(handle,
			sizeof(usb_descriptor_configuration_t));
	handle->descriptor_configuration->wTotalLength += iface->bLength;
	iface->bDescriptorType = USB_DT_INTERFACE;
	iface->bInterfaceProtocol = 0xff;
	iface->bNumEndpoints = 2;
	iface->bInterfaceNumber = 0;
	iface->bAlternateSetting = 0;

	usb_descriptor_endpoint_t *ep01 = add_descriptor(handle,
			sizeof(usb_descriptor_endpoint_t));
	handle->descriptor_configuration->wTotalLength += ep01->bLength;
	usb_descriptor_endpoint_t *ep81 = add_descriptor(handle,
			sizeof(usb_descriptor_endpoint_t));
	handle->descriptor_configuration->wTotalLength += ep81->bLength;

	ep01->bDescriptorType = ep81->bDescriptorType =
	USB_DT_ENDPOINT;
	ep01->bmAttributes = ep81->bmAttributes = 2;
	ep01->wMaxPacketSize = ep81->wMaxPacketSize = 64;
	ep81->bEndpointAddress = 0x81;
	ep01->bEndpointAddress = 0x01;

}

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
		switch (req->wValue >> 8) {
		case USB_DT_DEVICE: {
			handle->transmit(0x00, handle->descriptor_device,
					handle->descriptor_device->bLength);
			return 0;
		}
		case USB_DT_CONFIGURATION: {
					// Do we still have to do the truncate thing?
			size_t size =
					(req->wLength
							< handle->descriptor_configuration->wTotalLength) ?
							req->wLength :
							handle->descriptor_configuration->wTotalLength;
			handle->transmit(0x00, handle->descriptor_configuration, size);

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

		}
		break;
	case USB_REQ_SET_DESCRIPTOR:
		break;
	case USB_REQ_GET_CONFIGURATION:
		break;
	case USB_REQ_SET_CONFIGURATION:
		// No configuration support yet
		// TODO open the endpoints

		// Just send an ack


		handle->transmit(0x00, NULL, 0);
		break;
	case USB_REQ_GET_INTERFACE:
		break;
	}
	return 0;

}

