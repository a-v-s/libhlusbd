/*
 * usbd_descriptor_webusb.h
 *
 *  Created on: 25 nov. 2022
 *      Author: andre
 */

#ifndef LIBHLUSBD_USBD_DESCRIPTOR_WEBUSB_H_
#define LIBHLUSBD_USBD_DESCRIPTOR_WEBUSB_H_

#include "usbd_descriptor_bos.h"

#define USBD_BOS_CAP_WEBUSB_UUID  (guid_t ) {0x38, 0xB6, 0x08, 0x34, 0xA9, 0x09, 0xA0, 0x47, 0x8B, 0xFD, 0xA0, 0x76, 0x88, 0x15, 0xB6, 0x65}

typedef struct {

	uint8_t bLength; // Size of this descriptor (in bytes)
	uint8_t bDescriptorType; // Descriptor type.
	uint8_t bDevCapabilityType; // Device Capability type
	uint8_t bReserved; // should be 0
	guid_t guid; // {3408b638-09a9-47a0-8bfd-a0768815b665}
	uint16_t bcdVersion; // WebUSB Version
	uint8_t bCendorCode; // bRequestValue for getting WebUSB descriptor
	uint8_t iLandingPage; // URL for landing page

} usbd_bos_capability_webusb_descriptor_t;

#endif /* LIBHLUSBD_USBD_DESCRIPTOR_WEBUSB_H_ */
