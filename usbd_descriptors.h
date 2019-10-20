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

void* add_descriptor(usbd_handle_t *handle, size_t size);

#endif /* USBD_DESCRIPTORS_H_ */
