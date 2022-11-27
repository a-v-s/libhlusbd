/*

 File: 		usbd_descriptor_webusb.h
 Author:	André van Schoubroeck
 License:	MIT


 MIT License

 Copyright (c) 2022 André van Schoubroeck <andre@blaatschaap.be>

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
