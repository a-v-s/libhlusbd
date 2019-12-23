/*
 * usbd.c
 *
 *  Created on: 19 okt. 2019
 *      Author: andre
 */

// TODO:
//		* Create libopencm3-like callback mechanisms that allows overriding.
//	Configuration/Initialisation : Create "Classes" Support to compose devices
#include "usbd.h"
#include "usbd_descriptors.h"

#if USBD_UNICODE_CONVERSION_ENABLED
// If we have Unicode Conversions enabled (UTF8 to UTF16)
#include "ConvertUTF/ConvertUTF.h"
#endif

// Some temprary stuff to test buffers and callbacks
uint8_t temp_recv_buffer[64];
void transfer_in_complete(usbd_handle_t *handle, uint8_t epnum, void *data,
		size_t size) {

}

void transfer_out_complete(usbd_handle_t *handle, uint8_t epnum, void *data,
		size_t size) {

	// This is a test to reply the data increased by 1;
	((uint8_t*) (data))[0]++;
	usbd_transmit(handle, 0x80 | epnum, data, size);

}

//
void usbd_add_endpoint_in(usbd_handle_t *handle, uint8_t config, uint8_t epnum,
		uint8_t eptype, uint16_t epsize, uint8_t epinterval,
		usbd_transfer_cb_f cb) {
	// TODO Multi Configuration Support
	// TODO Range checking
	// TODO Endpoint type

	usb_descriptor_endpoint_t *ep = add_descriptor(handle,
			sizeof(usb_descriptor_endpoint_t));
	handle->descriptor_configuration[config - 1]->wTotalLength += ep->bLength;

	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bmAttributes = eptype;
	ep->wMaxPacketSize = epsize;
	ep->bEndpointAddress = 0x80 | epnum;
	ep->bInterval = epinterval;

	handle->ep_out[0x7F & epnum].cb = cb;

}

void usbd_add_endpoint_out(usbd_handle_t *handle, uint8_t config, uint8_t epnum,
		uint8_t eptype, uint16_t epsize, uint8_t epinterval, void *buffer,
		size_t size, usbd_transfer_cb_f cb) {
	// TODO Multi Configuration Support
	// TODO Range checking
	// TODO Endpoint type
	// TODO Make a Set Buffer function, so we can swap buffers

	usb_descriptor_endpoint_t *ep = add_descriptor(handle,
			sizeof(usb_descriptor_endpoint_t));
	handle->descriptor_configuration[config - 1]->wTotalLength += ep->bLength;

	ep->bDescriptorType = USB_DT_ENDPOINT;
	ep->bmAttributes = eptype;
	ep->wMaxPacketSize = epsize;
	ep->bEndpointAddress = 0x7F & epnum;
	ep->bInterval = epinterval;

	handle->ep_out[0x7F & epnum].buffer = buffer;
	handle->ep_out[0x7F & epnum].size = size;
	handle->ep_out[0x7F & epnum].cb = cb;

}
void usbd_demo_setup_descriptors(usbd_handle_t *handle) {
	handle->descriptor_device = add_descriptor(handle,
			sizeof(usb_descriptor_device_t));
	handle->descriptor_device->bDescriptorType = USB_DT_DEVICE;
	handle->descriptor_device->bMaxPacketSize0 = 64;
	handle->descriptor_device->bNumConfigurations = 1;
	handle->descriptor_device->bcdUSB = 0x0100;
	handle->descriptor_device->idVendor = 0xdead;
	handle->descriptor_device->idProduct = 0xbeef;

	handle->descriptor_device->iManufacturer = 1;
	handle->descriptor_device->iProduct = 2;

	handle->descriptor_configuration[0] = add_descriptor(handle,
			sizeof(usb_descriptor_configuration_t));
	handle->descriptor_configuration[0]->wTotalLength =
			handle->descriptor_configuration[0]->bLength;
	handle->descriptor_configuration[0]->bDescriptorType = USB_DT_CONFIGURATION;
	handle->descriptor_configuration[0]->bConfigurationValue = 1;
	handle->descriptor_configuration[0]->bMaxPower = 10;
	handle->descriptor_configuration[0]->bmAttributes = 0x80;
	handle->descriptor_configuration[0]->bNumInterfaces = 1;

	// TODO Make function to add interfaces (and altsettings)

	usb_descriptor_interface_t *iface = add_descriptor(handle,
			sizeof(usb_descriptor_configuration_t));
	handle->descriptor_configuration[0]->wTotalLength += iface->bLength;
	iface->bDescriptorType = USB_DT_INTERFACE;
	iface->bInterfaceProtocol = 0xff;
	iface->bNumEndpoints = 2;
	iface->bInterfaceNumber = 0;
	iface->bAlternateSetting = 0;

	usbd_add_endpoint_in(handle, 1, 1, USB_EP_ATTR_TYPE_INTERRUPT, 64, 1,
			(usbd_transfer_cb_f) &transfer_in_complete);
	usbd_add_endpoint_out(handle, 1, 1, USB_EP_ATTR_TYPE_BULK, 64, 1,
			temp_recv_buffer, 64, (usbd_transfer_cb_f) &transfer_out_complete);

	// Be sure to save the file as UTF-8. ;)
	handle->descriptor_string[1] = add_string_descriptor_utf8(handle, u8"🐈Blaat!");

	// The u"string" prefix encodes it as UTF16 from the start
	handle->descriptor_string[2] = add_string_descriptor_utf16(handle, u"Schaap!🐼");

}



usbd_handler_result_t usbd_handle_standard_device_request(usbd_handle_t *handle,
		usb_setuprequest_t *req, void **buf, size_t *size) {

	switch (req->bRequest) {
	case USB_REQ_GET_STATUS: {

		// If the request is directed to a device (check bmRequestType)
		// This returns whether the device is bus of self powered (bit 0)
		//		and the remote wakeup status (bit 1).
		// TODO: Make this a non-static response
		// 			1) Store the result within the usbd_handle_t
		//			2) Provide an API to indicate power source
		//	The Remote Wakeup bit indicates whether the device can wake up the
		//	host during suspend. It can be set and cleared by
		//  USB_REQ_CLEAR_FEATURE and USB_REQ_SET_FEATURE. TODO: Investigate
		//  how we're supposed to behave when this has been set.

		// If the request is directed to an interface (check bmRequestType)
		// Interface number is wIndex
		// We're always supposed to return 0

		// If the request is directed to an endpoint (check bmRequestType)
		// Endpoint number is wIndex
		// Bit 0 indicates the endpoint is stalled.


		/// TODO
		static uint16_t status = 0;
		//usbd_transmit(handle, 0x80, &status, 2);
		*buf = &status;
		*size = sizeof(status);
		return RESULT_HANDLED;

		return 0;

	}
		break;
	case USB_REQ_CLEAR_FEATURE:
		break;
	case USB_REQ_SET_FEATURE:
		break;
	case USB_REQ_SET_ADDRESS:
		usbd_set_address(handle, req->wValue & 0x7F);
		//usbd_transmit(handle, 0x00, NULL, 0);
		return RESULT_HANDLED;
		break;
	case USB_REQ_GET_DESCRIPTOR: {
		int index = req->wValue & 0xFF;
		int descriptor = req->wValue >> 8;

		switch (descriptor) {
		case USB_DT_DEVICE: {
			/*
			usbd_transmit(handle, 0x00, handle->descriptor_device,
					handle->descriptor_device->bLength);
					*/
			*buf = handle->descriptor_device;
			*size = handle->descriptor_device->bLength;
			return RESULT_HANDLED;

			return 0;
		}
		case USB_DT_CONFIGURATION: {

			if (index < USBD_CONFIGURATION_COUNT) {
				// Do we still have to do the truncate thing?
				// How to handle larger transactions?

				/*
				size_t size =
						(req->wLength
								< handle->descriptor_configuration[index]->wTotalLength) ?
								req->wLength :
								handle->descriptor_configuration[index]->wTotalLength;
				usbd_transmit(handle, 0x00,
						handle->descriptor_configuration[index], size);

						*/

				*buf = handle->descriptor_configuration[index];
				//*size = handle->descriptor_configuration[index]->wTotalLength;
				*size = (req->wLength
						< handle->descriptor_configuration[index]->wTotalLength) ?
						req->wLength :
						handle->descriptor_configuration[index]->wTotalLength;


				return RESULT_HANDLED;

			} else {
				return RESULT_REJECTED;
			}

			return 0;
		}
			break;

			//case USB_DT_INTERFACE: not requestable
			//case USB_DT_ENDPOINT: not requestable
		case USB_DT_STRING: {
			// TODO: String Support
			// Internal we'll only do 0x0409 (en_US)
			// Additional languages will be in user mode
			if (index == 0) {
				// TODO, store String Descriptor 0 in global descriptor store
				static usb_descriptor_string_t only0 = { 0 };
				only0.bDescriptorType = USB_DT_STRING;
				only0.bLength = 4;			//6;
				only0.wLANGID[0] = 0x0409;
				//usbd_transmit(handle, 0x00, &only0, only0.bLength);
				*buf = &only0;
				*size = only0.bLength;
				return RESULT_HANDLED;

			} else {
				/*
				usbd_transmit(handle, 0x00, handle->descriptor_string[index],
						handle->descriptor_string[index]->bLength);
						*/
				*buf = handle->descriptor_string[index];
				*size = handle->descriptor_string[index]->bLength;
				return RESULT_HANDLED;

			}

			break;
		}
		default:
			// Unsupported descriptor requested
			//usbd_ep_set_stall(handle, 0x80); // Stall Endpoint 0x80
			return RESULT_REJECTED;

		}

	}
		break;
	case USB_REQ_SET_DESCRIPTOR:
		// What purpose has the SET_DESCRIPTOR request?
		break;
	case USB_REQ_GET_CONFIGURATION:
		// usbd_transmit(handle, 0x00, &handle->configuration, 1);
		*buf = &handle->configuration;
		*size = 1;
		return RESULT_HANDLED;

		break;
	case USB_REQ_SET_CONFIGURATION: {
		// No configuration support yet
		// TODO open the endpoints

		// Configuration 0 = return to address mode = ?? As in unadressed?
		// Configuration 1...n = apply config
		// Configuration n+1 = Error, stall
		int config = req->wValue & 0xFF;
		if ((config) && (config - 1) < USBD_CONFIGURATION_COUNT) {
			handle->configuration = req->wValue;
			// TODO: What about alternative interfaces
			handle->interface = 0;

			// TODO: Apply configuration (configure endpoints)
			// TODO: Callbacks when configuration changes

			for (int i = 0; i < USBD_ENDPOINTS_COUNT; i++) {

			}

			return RESULT_HANDLED;
		} else {
			return RESULT_REJECTED;
		}
		break;
	}

	default:
		return RESULT_REJECTED;
	}
	return RESULT_REJECTED;
}

usbd_handler_result_t usbd_handle_standard_interface_request(
		usbd_handle_t *handle, usb_setuprequest_t *req, void **buf, size_t *len) {

	switch (req->bRequest) {
	case USB_REQ_GET_STATUS: {
		// TODO
		break;
	}
	case USB_REQ_CLEAR_FEATURE: {
		// TODO
		break;
	}
	case USB_REQ_SET_FEATURE: {
		// TODO
		break;
	}
	case USB_REQ_GET_INTERFACE: {
		break;
	}
	case USB_REQ_SET_INTERFACE: {
		break;
	}
	}

	return RESULT_REJECTED;

}

usbd_handler_result_t usbd_handle_standard_endpoint_request(
		usbd_handle_t *handle, usb_setuprequest_t *req, void **buf, size_t *len) {

	switch (req->bRequest) {
	case USB_REQ_GET_STATUS: {
		// TODO
		break;
	}
	case USB_REQ_CLEAR_FEATURE: {
		// TODO
		break;
	}
	case USB_REQ_SET_FEATURE: {
		// TODO
		break;
	}
	case USB_REQ_SYNCH_FRAME: {
		// TODO
		break;
	}
	}

	return RESULT_REJECTED;
}

usbd_handler_result_t usbd_handle_standard_request(usbd_handle_t *handle,
		usb_setuprequest_t *req, void **buf, size_t *len) {


	switch (req->bmRequest.Recipient) {
	case USB_REQTYPE_RECIPIENT_DEVICE:
		return usbd_handle_standard_device_request(handle, req, buf, len);
		break;
	case USB_REQTYPE_RECIPIENT_INTERFACE:
		return usbd_handle_standard_interface_request(handle, req, buf, len);
		break;
	case USB_REQTYPE_RECIPIENT_ENDPOINT:
		return usbd_handle_standard_endpoint_request(handle, req, buf, len);
		break;
	default:
		break;
	}

	return RESULT_REJECTED;

}

usbd_handler_result_t usbd_handle_request(usbd_handle_t *handle, usb_setuprequest_t *req) {
	usbd_handler_result_t result = RESULT_NEXT_PARSER;
	void *buf = NULL;
	size_t size = 0;

	// TODO, User handlers


	if (result == RESULT_NEXT_PARSER) {
		result = usbd_handle_standard_request(handle, req, &buf, &size);
	}

	if (result == RESULT_HANDLED) {
		usbd_transmit(handle, req->bmRequest.Direction << 7, buf, size);
		//usbd_transmit(handle, 0x00, buf, size);
	} else {
		usbd_ep_set_stall(handle, req->bmRequest.Direction << 7);
	}
	return result;
}

int usbd_transmit(usbd_handle_t *handle, uint8_t ep, void *data, size_t size) {
	return handle->driver.transmit(handle->driver.device_specific, ep, data,
			size);
}
int usbd_set_address(usbd_handle_t *handle, uint8_t address) {
	return handle->driver.set_address(handle->driver.device_specific, address);
}
int usbd_ep_set_stall(usbd_handle_t *handle, uint8_t epnum) {
	return handle->driver.ep_set_stall(handle->driver.device_specific, epnum);
}
int usbd_ep_clear_stall(usbd_handle_t *handle, uint8_t epnum) {
	return handle->driver.ep_clear_stall(handle->driver.device_specific, epnum);
}
int usbd_ep_close(usbd_handle_t *handle, uint8_t epnum) {
	return handle->driver.ep_close(handle->driver.device_specific, epnum);
}
int usbd_ep_open(usbd_handle_t *handle, uint8_t epnum, uint8_t epsize,
		uint8_t eptype) {
	return handle->driver.ep_open(handle->driver.device_specific, epnum, epsize,
			eptype);
}

