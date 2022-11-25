/*
 * usbd_descriptor_winusb.h
 *
 *  Created on: 25 nov. 2022
 *      Author: andre
 */

#ifndef LIBHLUSBD_USBD_DESCRIPTOR_WINUSB_H_
#define LIBHLUSBD_USBD_DESCRIPTOR_WINUSB_H_

#include <stdint.h>

#include "usbd_descriptor_bos.h"

#pragma pack(push, 1) // exact fit - no padding

typedef struct {
	uint8_t str8[8];
} str8_t;

#define REQUESTVALUE_MICROSOFT 0x20
#define USBD_MSOS_DRIVER_WINUSB  (str8_t){'W', 'I', 'N', 'U' , 'S', 'B',  0 ,  0} // Recommended, native
#define USBD_MSOS_DRIVER_LIBUSBK (str8_t){'L', 'I', 'B', 'U' , 'S', 'B', 'K',  0} // Recommended after WinUSB
#define USBD_MSOS_DRIVER_LIBUSB0 (str8_t){'L', 'I', 'B', 'U' , 'S', 'B', '0',  0} // Old, not recommended

#define MS_OS_20_SET_HEADER_DESCRIPTOR	(0x00)
#define MS_OS_20_FEATURE_COMPATIBLE_ID  (0x03)

#define USBD_BOS_CAP_MICROSOFT_UUID  (guid_t ) { 0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C, 0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F }
#define NTDDI_WINBLUE (0x06030000)
typedef struct {

	uint8_t bLength; // Size of this descriptor (in bytes)
	uint8_t bDescriptorType; // Descriptor type.
	uint8_t bDevCapabilityType; // Device Capability type
	uint8_t bReserved; // should be 0
	guid_t guid; // microsoft guid  {D8DD60DF-4589-4CC7-9CD2-659D9E648A9F}
	uint32_t dwWindowsVersion; // NTDDI_WINBLUE (0x06030000) (Windows 8.1)
	uint16_t wMSOSDescriptorSetTotalLength;
	uint8_t bMS_VendorCode; // bRequestValue used to retrieve MSOS20 descriptor
	uint8_t bAltEnumCode;
} usbd_bos_capability_microsoft_descriptor_t;


typedef struct {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint32_t dwWindowsVersion;
    uint16_t wTotalLength;
} usbd_msos20_set_header_descriptor_t;

typedef struct {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint8_t bFirstInterface;
    uint8_t bReserved;
    uint16_t wSubsetLength;
} usbd_msos20_subset_header_descriptor_t;

typedef struct {
    uint16_t wLength;
    uint16_t wDescriptorType;
    str8_t CompatibleID;
    str8_t SubCompatibleID;
} usbd_msos20_compatible_id_descriptor_t;

typedef struct {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint16_t wPropertyDataType;
    uint16_t wPropertyNameLength;
    uint16_t PropertyName[21];
    uint16_t wPropertyDataLength;
    uint16_t PropertyData[40];

} usbd_msos20_registry_property_descriptor_t;

#pragma pack(pop)

#endif /* LIBHLUSBD_USBD_DESCRIPTOR_WINUSB_H_ */
