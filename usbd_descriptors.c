/*
 * usbd_descriptors.c
 *
 *  Created on: 20 okt. 2019
 *      Author: andre
 */

#include "usbd_descriptors.h"

void* add_descriptor(usbd_handle_t *handle, size_t size) {
	if ( (handle->usbd_descriptor_buffer_offset + size) > USBD_DESCRIPTOR_BUFFER_SIZE ) return NULL;
	void* retval = handle->usbd_descriptor_buffer + handle->usbd_descriptor_buffer_offset;
	handle->usbd_descriptor_buffer_offset += size;
	memset(retval,0,size);
	*((uint8_t*)(retval))=size;
	return retval;
}
