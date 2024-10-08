/*

File: 	 	usbd_descriptors.h
Author:		André van Schoubroeck
License:	MIT


MIT License

Copyright (c) 2018 - 2022 André van Schoubroeck <andre@blaatschaap.be>

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

/*
 * usbd_descriptors.h
 *
 *  Created on: 20 okt. 2019
 *      Author: andre
 */

#ifndef USBD_DESCRIPTORS_H_
#define USBD_DESCRIPTORS_H_

#include "usbd.h"
#include "usb_descriptors.h"

	

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void* add_descriptor(bscp_usbd_handle_t *handle, size_t size);
void* add_string_descriptor_utf16(bscp_usbd_handle_t *handle, uint16_t *utf16_string) ;
void* add_string_descriptor_utf8(bscp_usbd_handle_t *handle, uint8_t *utf8_string) ;

#endif /* USBD_DESCRIPTORS_H_ */
