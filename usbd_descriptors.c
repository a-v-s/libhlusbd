/*
 * usbd_descriptors.c
 *
 *  Created on: 20 okt. 2019
 *      Author: André van Schoubroeck
 */

#include "usbd_descriptors.h"

void* add_descriptor(usbd_handle_t *handle, size_t size) {
	if ((handle->usbd_descriptor_buffer_offset + size)
			> USBD_DESCRIPTOR_BUFFER_SIZE)
		return NULL;
	void *retval = handle->usbd_descriptor_buffer
			+ handle->usbd_descriptor_buffer_offset;
	handle->usbd_descriptor_buffer_offset += size;
	memset(retval, 0, size);
	*((uint8_t*) (retval)) = size;
	return retval;
}

void* add_string_descriptor_ascii(usbd_handle_t *handle, uint8_t *ascii_string) {
	usb_descriptor_string_t *strdesc =
			(usb_descriptor_string_t*) (handle->usbd_descriptor_buffer
					+ handle->usbd_descriptor_buffer_offset);
	int offset = handle->usbd_descriptor_buffer_offset + 2;
	int initial_offset = handle->usbd_descriptor_buffer_offset;
	while (*ascii_string) {
		if (offset > (USBD_DESCRIPTOR_BUFFER_SIZE - 2))
			return NULL;
		handle->usbd_descriptor_buffer[offset] = *ascii_string;
		offset++;
		handle->usbd_descriptor_buffer[offset] = 0;
		offset++;
		ascii_string++;
	}
	strdesc->bLength = offset - initial_offset;
	strdesc->bDescriptorType = USB_DT_STRING;
	return strdesc;
}

#if USBD_UNICODE_CONVERSION_ENABLED

void* add_string_descriptor_utf8(usbd_handle_t *handle, uint8_t *utf8_string) {
	usb_descriptor_string_t *strdesc =
			(usb_descriptor_string_t*) (handle->usbd_descriptor_buffer
					+ handle->usbd_descriptor_buffer_offset);
	int initial_offset = handle->usbd_descriptor_buffer_offset;
	void *end_offset = strdesc->bString;

	// NOTE: Be aware of Endianness. USB expects Little Endian, which
	// most microcontrollers are these days. If we're running on a Big
	// Endian system, we need to add conversion.

	// TODO: catch return value, make sure the string fits
	ConvertUTF8toUTF16(&utf8_string, utf8_string + strlen(utf8_string),
			&end_offset,
			handle->usbd_descriptor_buffer + USBD_DESCRIPTOR_BUFFER_SIZE);
	handle->usbd_descriptor_buffer_offset = (int) end_offset
			- (int) handle->usbd_descriptor_buffer;
	strdesc->bLength = handle->usbd_descriptor_buffer_offset - initial_offset;
	strdesc->bDescriptorType = USB_DT_STRING;
	return strdesc;
}
#endif

// If the string is provided in UTF16LE encoding, no conversion is needed
// Note: When running on a Big Endian machine, conversion is needed.
void* add_string_descriptor_utf16(usbd_handle_t *handle, uint16_t *utf16_string) {
	usb_descriptor_string_t *strdesc =
			(usb_descriptor_string_t*) (handle->usbd_descriptor_buffer
					+ handle->usbd_descriptor_buffer_offset);
	int offset = handle->usbd_descriptor_buffer_offset + 2;
	int initial_offset = handle->usbd_descriptor_buffer_offset;

	// This isn't the best, but enough for a test to see if the u"string" works
	int i = 0;
	while (*utf16_string) {
		if (offset > (USBD_DESCRIPTOR_BUFFER_SIZE - 2))
			return NULL;
		strdesc->bString[i] = *utf16_string;
		utf16_string++;
		i++;
		offset += 2;
	}

	strdesc->bLength = offset - initial_offset;
	strdesc->bDescriptorType = USB_DT_STRING;
	return strdesc;
}

