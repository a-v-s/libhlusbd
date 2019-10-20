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

// Some temprary stuff to test buffers and callbacks
uint8_t temp_recv_buffer[64];
void transfer_in_complete(usbd_handle_t *handle, uint8_t epnum, void *data,
		size_t size) {
}
void transfer_out_complete(usbd_handle_t *handle, uint8_t epnum, void *data,
		size_t size) {
	((uint8_t*) (data))[0]++;
	handle->transmit(0x80 | epnum, data, size);
}

//
void usbd_add_endpoint_in(usbd_handle_t *handle, uint8_t config, uint8_t epnum,
		usbd_transfer_cb_f cb) {
	// TODO Multi Configuration Support
	// TODO Range checking
	// TODO Endpoint type

	usb_descriptor_endpoint_t *ep = add_descriptor(handle,
			sizeof(usb_descriptor_endpoint_t));
	handle->descriptor_configuration[config-1]->wTotalLength += ep->bLength;

	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bmAttributes = 2;
	ep->wMaxPacketSize = 64;
	ep->bEndpointAddress = 0x80 | epnum;

	handle->ep_out[0x7F & epnum].cb = cb;

}

void usbd_add_endpoint_out(usbd_handle_t *handle,uint8_t config, uint8_t epnum, void *buffer,
		size_t size, usbd_transfer_cb_f cb) {
	// TODO Multi Configuration Support
	// TODO Range checking
	// TODO Endpoint type
	// TODO Make a Set Buffer function, so we can swap buffers

	usb_descriptor_endpoint_t *ep = add_descriptor(handle,
			sizeof(usb_descriptor_endpoint_t));
	handle->descriptor_configuration[config-1]->wTotalLength += ep->bLength;

	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bmAttributes = 2;
	ep->wMaxPacketSize = 64;
	ep->bEndpointAddress = 0x7F & epnum;

	handle->ep_out[0x7F & epnum].buffer = buffer;
	handle->ep_out[0x7F & epnum].size = size;
	handle->ep_out[0x7F & epnum].cb = cb;

}
void usbd_setup_descriptors(usbd_handle_t *handle) {
	handle->descriptor_device = add_descriptor(handle,
			sizeof(usb_descriptor_device_t));
	handle->descriptor_device->bDescriptorType = USB_DT_DEVICE;
	handle->descriptor_device->bMaxPacketSize0 = 64;
	handle->descriptor_device->bNumConfigurations = 1;
	handle->descriptor_device->bcdUSB = 0x0100;
	handle->descriptor_device->idVendor = 0xdead;
	handle->descriptor_device->idProduct = 0xbeef;

	handle->descriptor_configuration[0] = add_descriptor(handle,
			sizeof(usb_descriptor_configuration_t));
	handle->descriptor_configuration[0]->wTotalLength =
			handle->descriptor_configuration[0]->bLength;
	handle->descriptor_configuration[0]->bDescriptorType = USB_DT_CONFIGURATION;
	handle->descriptor_configuration[0]->bConfigurationValue = 1;
	handle->descriptor_configuration[0]->bMaxPower = 10;
	handle->descriptor_configuration[0]->bmAttributes = 0x80;
	handle->descriptor_configuration[0]->bNumInterfaces = 1;

	usb_descriptor_interface_t *iface = add_descriptor(handle,
			sizeof(usb_descriptor_configuration_t));
	handle->descriptor_configuration[0]->wTotalLength += iface->bLength;
	iface->bDescriptorType = USB_DT_INTERFACE;
	iface->bInterfaceProtocol = 0xff;
	iface->bNumEndpoints = 2;
	iface->bInterfaceNumber = 0;
	iface->bAlternateSetting = 0;

	usbd_add_endpoint_in(handle, 1,1, &transfer_in_complete);
	usbd_add_endpoint_out(handle, 1, 1, temp_recv_buffer, 64,
			&transfer_out_complete);

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
			int index = req->wValue & 0xFF;
			if (index < USBD_CONFIGURATION_COUNT) {
				// Do we still have to do the truncate thing?
				size_t size =
						(req->wLength
								< handle->descriptor_configuration[index]->wTotalLength) ?
								req->wLength :
								handle->descriptor_configuration[index]->wTotalLength;
				handle->transmit(0x00, handle->descriptor_configuration[index], size);
			} else {
				// Invalid configuration requested!
			}

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
		// Verify this
		handle->transmit(0x00, &handle->configuration, 1);
		break;
	case USB_REQ_SET_CONFIGURATION: {
		// No configuration support yet
		// TODO open the endpoints

		// Configuration Numbers are 1-based?
		int config = req->wValue & 0xFF;
		if ((config) && (config - 1) < USBD_CONFIGURATION_COUNT) {
			handle->configuration = req->wValue;
			// Just send an ack
			handle->transmit(0x00, NULL, 0);
		} else {
			/// Should we send a NACK?
		}
		break;
	}
	case USB_REQ_GET_INTERFACE:
		break;
	}
	return 0;

}

