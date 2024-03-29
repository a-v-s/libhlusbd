/*

 File:       usbd_stm.c
 Author:     André van Schoubroeck
 License:    MIT


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

#include <string.h>
#include "usbd.h"
#include "usbd_stm.h"

static PCD_HandleTypeDef m_hpcd;

static bscp_usbd_handle_t m_usbd_handle = { .driver.device_specific = &m_hpcd,
		.driver.transmit = &usbd_stm32_transmit, .driver.set_address =
				&usbd_stm32_set_address, .driver.ep_set_stall =
				&usbd_stm32_ep_set_stall, .driver.ep_clear_stall =
				&usbd_stm32_ep_clear_stall, .driver.ep_open =
				&usbd_stm32_ep_open, .driver.ep_close = &usbd_stm32_ep_close, };

//uint8_t test_buffer[64];

bscp_usbd_handle_t* usbd_init() {
	GPIO_InitTypeDef GPIO_InitStruct;
	memset(&m_hpcd, 0, sizeof(m_hpcd));

	// For now... we'll see how to make this neat later
	static usbd_stm32_usbfs_v1_config config;
	m_usbd_handle.driver.driver_config = &config;
	config.PmaPos = PMAPOS_MIN;

	bscp_usbd_demo_setup_descriptors(&m_usbd_handle);

	__HAL_RCC_GPIOA_CLK_ENABLE();

#if defined (USB) 
	__HAL_RCC_USB_CLK_ENABLE();
#elif defined (USB_OTG_FS)
	__HAL_RCC_USB_OTG_FS_CLK_ENABLE();
#endif

	// TODO: PullUp / Reset Behaviour
#if defined STM32F1
	// Configure USB DM/DP pins
	GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
	GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#elif defined STM32F3
	GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF14_USB;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#elif defined STM32F0
	GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_USB;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#elif defined STM32F4
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // $(ucdev)/ext/STM32CubeF4/Projects/STM32F401-Discovery/Demonstrations/Src/usbd_conf.c:250
#endif

	/* // TODO: Low Power Support
	 if (hpcd->Init.low_power_enable == 1) {
	 // Enable EXTI for USB wakeup
	 __HAL_USB_WAKEUP_EXTI_CLEAR_FLAG();
	 __HAL_USB_WAKEUP_EXTI_ENABLE_RISING_EDGE();
	 __HAL_USB_WAKEUP_EXTI_ENABLE_IT();

	 // USB Wakeup Interrupt
	 HAL_NVIC_EnableIRQ(USBWakeUp_IRQn);

	 // Enable USB Wake-up interrupt
	 HAL_NVIC_SetPriority(USBWakeUp_IRQn, 0, 0);
	 }
	 */

#if defined (USB_OTG_FS)
	HAL_NVIC_SetPriority(OTG_FS_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
#elif defined STM32F1
	HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
#elif defined STM32F3
	HAL_NVIC_SetPriority(USB_LP_CAN_RX0_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);
#elif defined STM32F0
	HAL_NVIC_SetPriority(USB_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(USB_IRQn);
#endif

#if defined (USB) 
	m_hpcd.Instance = USB;
#elif defined (USB_OTG_FS)
	m_hpcd.Instance = USB_OTG_FS;
#endif
	m_hpcd.Init.dev_endpoints = 8;	// Does this value differ per family?
	m_hpcd.Init.phy_itface = PCD_PHY_EMBEDDED;
	m_hpcd.Init.speed = PCD_SPEED_FULL;
	m_hpcd.Init.ep0_mps = 64;

	m_hpcd.pData = &m_usbd_handle;
	m_usbd_handle.driver.device_specific = &m_hpcd;

	// Initialize LL Driver
	HAL_PCD_Init(&m_hpcd);

	int ep0size = m_usbd_handle.descriptor_device->bMaxPacketSize0;

#if defined (USB) 
	HAL_PCDEx_PMAConfig(&m_hpcd, 0x00, PCD_SNG_BUF, config.PmaPos);
	config.PmaPos += ep0size;
	HAL_PCDEx_PMAConfig(&m_hpcd, 0x80, PCD_SNG_BUF, config.PmaPos);
	config.PmaPos += ep0size;
#elif defined (USB_OTG_FS)
	// This should allocate the buffers for receiving and sending data
	// over endpoint 0 (thus 0x00 and 0x80)
	// Still... something is wrong. Might be here, or somewhere else
	// We *receive* the GET_DESCRIPTOR request fine on the STM32F401, but
	// the reply does not get received by the host.
	HAL_PCDEx_SetRxFiFo(&m_hpcd, 0x80);
	HAL_PCDEx_SetTxFiFo(&m_hpcd, 0, ep0size);
	#endif

	HAL_PCD_Start(&m_hpcd);

	// Moved to Open EP
	/*
	 HAL_PCDEx_PMAConfig(&m_hpcd, 0x01, PCD_SNG_BUF, config.PmaPos);
	 config.PmaPos += 0x40;
	 HAL_PCDEx_PMAConfig(&m_hpcd, 0x81, PCD_SNG_BUF, config.PmaPos);
	 config.PmaPos += 0x40;
	 HAL_PCDEx_PMAConfig(&m_hpcd, 0x82, PCD_SNG_BUF, config.PmaPos);
	 */

	return &m_usbd_handle;

}

//--
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd) {
	bscp_usbd_handle_request(hpcd->pData, (usb_setuprequest_t*) (hpcd->Setup));
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	// Reception Complete Callback
	// TODO: Continuation transmissions
	// TODO: Range checking
	size_t received_amount = HAL_PCD_EP_GetRxCount(hpcd, epnum);
	if ((epnum & 0x7F) == 0x00) {
		// Setup
		HAL_PCD_EP_SetStall(hpcd, 0x80);

		// Do we need to stall 0x00 as well?
		//HAL_PCD_EP_SetStall(hpcd, 0x00);

		// This broke the STM32F4 (USB_OTG_FS)
		// Is it required F0,F1,F3 ??? (USB)
		// HAL_PCD_EP_Transmit(hpcd, 0x00, NULL, 0);

	} else {
		// Callback, deliver received data to application
		if (m_usbd_handle.ep_out[0x7F & epnum].data_cb)
			m_usbd_handle.ep_out[0x7F & epnum].data_cb(hpcd->pData, epnum,
					m_usbd_handle.ep_out[epnum & 0x7F].data_buffer,
					received_amount);

		// Prepare to receive the next transmission from the host
		HAL_PCD_EP_Receive(hpcd, epnum,
				m_usbd_handle.ep_out[epnum & 0x7F].data_buffer,
				m_usbd_handle.ep_out[epnum & 0x7F].data_size);

	}
}



void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	// Transmission Complete Callback
	// Range checking
	if ((epnum & 0x7F) >= USBD_ENDPOINTS_COUNT)
		return;

	// Is it Setup or Data
	bool setup = ((epnum & 0x7F) == 0x00);
#ifdef USB_OTG_FS

	if (setup) {
		// It seems the OTG version handles the complete transfers for EP1+
		// But for EP0 (setup) we need to handle multi-packet transfers like we
		// would on the USBFSv1/USBFSv2 hardware

		// First a quick and dirty copy paste solution, if the above turns out to be
		// corrent, then make a neat solution

		// Well... it doesn't work.


		m_usbd_handle.ep_in[epnum & 0x7F].data_cnt += hpcd->IN_ep[0x7F & epnum].xfer_len;

		if (m_usbd_handle.ep_in[epnum & 0x7F].data_size
								> m_usbd_handle.ep_in[epnum & 0x7F].data_cnt) {
				HAL_PCD_EP_Transmit(hpcd, epnum,
						m_usbd_handle.ep_in[epnum & 0x7F].data_buffer
								+ m_usbd_handle.ep_in[epnum & 0x7F].data_cnt,
						m_usbd_handle.ep_in[epnum & 0x7F].data_size
								- m_usbd_handle.ep_in[epnum & 0x7F].data_cnt);
		} else {
			//if (!(m_usbd_handle.ep_in[epnum & 0x7F].data_size%m_usbd_handle.ep_in[epnum & 0x7F].ep_size ))	HAL_PCD_EP_Transmit(hpcd, epnum,NULL,0);
			HAL_PCD_EP_SetStall(hpcd, 0x80);
		}

		HAL_PCD_EP_Receive(hpcd, 0x00, NULL, 0);

	}

		if (hpcd->IN_ep[0x7F & epnum].xfer_count == m_usbd_handle.ep_in[epnum & 0x7F].data_size) {
			if (m_usbd_handle.ep_in[0x7F & epnum].data_cb)
				m_usbd_handle.ep_in[0x7F & epnum].data_cb(hpcd->pData, epnum,
						m_usbd_handle.ep_in[epnum & 0x7F].data_buffer,
						m_usbd_handle.ep_in[epnum & 0x7F].data_size);
		}
#else

//	if (m_usbd_handle.ep_in[epnum & 0x7F].data_cnt == 0) {
//		if (m_usbd_handle.ep_in[epnum & 0x7F].data_size
//				>= m_usbd_handle.ep_in[epnum & 0x7F].ep_size)
//			m_usbd_handle.ep_in[epnum & 0x7F].data_cnt =
//					m_usbd_handle.ep_in[epnum & 0x7F].ep_size;
//		else
//			m_usbd_handle.ep_in[epnum & 0x7F].data_cnt =
//					m_usbd_handle.ep_in[epnum & 0x7F].data_size;
//	}

	// Is this a Multi Part transfer?
	if ((m_usbd_handle.ep_in[epnum & 0x7F].data_size
			- m_usbd_handle.ep_in[epnum & 0x7F].data_cnt)
			>= m_usbd_handle.ep_in[epnum & 0x7F].ep_size) {

		HAL_PCD_EP_Transmit(hpcd, epnum,
				m_usbd_handle.ep_in[epnum & 0x7F].data_buffer
						+ m_usbd_handle.ep_in[epnum & 0x7F].data_cnt,
				//m_usbd_handle.ep_in[epnum & 0x7F].ep_size);
				m_usbd_handle.ep_in[epnum & 0x7F].data_size
						- m_usbd_handle.ep_in[epnum & 0x7F].data_cnt);

		m_usbd_handle.ep_in[epnum & 0x7F].data_cnt += m_usbd_handle.ep_in[epnum
				& 0x7F].ep_size;

		if (setup) {
			HAL_PCD_EP_Receive(hpcd, 0x00, NULL, 0);
			HAL_PCD_EP_SetStall(hpcd, 0x80); // Fix the long string errors
		}

	} else {
		size_t size = m_usbd_handle.ep_in[epnum & 0x7F].data_size
				- m_usbd_handle.ep_in[epnum & 0x7F].data_cnt;

		HAL_PCD_EP_Transmit(hpcd, epnum,
				m_usbd_handle.ep_in[epnum & 0x7F].data_buffer
						+ m_usbd_handle.ep_in[epnum & 0x7F].data_cnt, size);

		m_usbd_handle.ep_in[epnum & 0x7F].data_cnt = m_usbd_handle.ep_in[epnum
				& 0x7F].data_size;

		if (setup) {
			HAL_PCD_EP_Receive(hpcd, 0x00, NULL, 0);
			HAL_PCD_EP_SetStall(hpcd, 0x80); // Order matters, first receive then stall
		} else {
			if (m_usbd_handle.ep_in[0x7F & epnum].data_cb)
				m_usbd_handle.ep_in[0x7F & epnum].data_cb(hpcd->pData, epnum,
						m_usbd_handle.ep_in[epnum & 0x7F].data_buffer,
						m_usbd_handle.ep_in[epnum & 0x7F].data_size);
		}
	}
#endif

}
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd) {
	//USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd) {
	// Feth Packet Size 0 from Device Descriptor
	bscp_usbd_handle_t *handle = (bscp_usbd_handle_t*) hpcd->pData;
	size_t epsize0 = handle->descriptor_device->bMaxPacketSize0;

	// Open EP0 OUT
	HAL_PCD_EP_Open(hpcd, 0x00, epsize0, USB_EP_ATTR_TYPE_CONTROL);
	//HAL_PCD_EP_Flush(hpcd, 0x00);
	//HAL_PCD_EP_ClrStall(hpcd, 0x00);
	m_usbd_handle.ep_out[0].ep_size = epsize0;

	// Open EP0 IN
	HAL_PCD_EP_Open(hpcd, 0x80, epsize0, USB_EP_ATTR_TYPE_CONTROL);
	//HAL_PCD_EP_Flush(hpcd, 0x80);
	//HAL_PCD_EP_ClrStall(hpcd, 0x80);
	m_usbd_handle.ep_in[0].ep_size = epsize0;

	/*
	 // Should be handled in set_configuration now

	 // Open EP1 OUT
	 HAL_PCD_EP_Open(hpcd, 0x01, 64, 0x02);
	 HAL_PCD_EP_Flush(hpcd, 0x01);
	 HAL_PCD_EP_ClrStall(hpcd, 0x01);
	 HAL_PCD_EP_Receive(hpcd, 0x01, m_usbd_handle.ep_out[1].data_buffer,
	 m_usbd_handle.ep_out[1].data_size);
	 m_usbd_handle.ep_out[1].ep_size = 64;

	 // Open EP1 IN
	 HAL_PCD_EP_Open(hpcd, 0x81, 64, 0x02);
	 HAL_PCD_EP_Flush(hpcd, 0x81);
	 HAL_PCD_EP_ClrStall(hpcd, 0x81);
	 m_usbd_handle.ep_in[1].ep_size = 64;
	 */
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd) {
	// TODO
	/*
	 // Inform USB library that core enters in suspend Mode
	 //USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
	 // Enter in STOP mode
	 if (hpcd->Init.low_power_enable) {
	 // Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register
	 SCB->SCR |= (uint32_t)(
	 (uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
	 }
	 USBD_LL_Delay(50);

	 */
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd) {
	// TODO
	/*
	 if ((hpcd->Init.low_power_enable) && (remotewakeupon == 0)) {
	 SystemClockConfig_STOP();

	 // Reset SLEEPDEEP bit of Cortex System Control Register
	 SCB->SCR &= (uint32_t) ~((uint32_t)(
	 SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
	 }
	 //USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
	 remotewakeupon = 0;
	 */
}

void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	// TODO: iso transfer support: low priority
	//USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	// TODO: iso transfer support: low priority
	//USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {
	// TODO
	//USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
	// Can/should we do pullup/reset behaviour here?
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd) {
	// TODO
	//USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
	// Can/should we do pullup/reset behaviour here?
}

int usbd_stm32_transmit(void *hpcd, uint8_t ep, void *data, size_t size) {


	m_usbd_handle.ep_in[ep & 0x7F].data_buffer = data;
	m_usbd_handle.ep_in[ep & 0x7F].data_size = size;
	//m_usbd_handle.ep_in[ep & 0x7F].data_cnt = 0;


	// This seems not applicable??? Was this a bug or did the HAL API change?
//	if (m_usbd_handle.ep_in[ep & 0x7F].data_size <
//			m_usbd_handle.ep_in[ep & 0x7F].ep_size)
//		m_usbd_handle.ep_in[ep & 0x7F].data_cnt = m_usbd_handle.ep_in[ep & 0x7F].data_size;
//	else
//		m_usbd_handle.ep_in[ep & 0x7F].data_cnt = m_usbd_handle.ep_in[ep & 0x7F].ep_size;
//

	m_usbd_handle.ep_in[ep & 0x7F].data_cnt = m_usbd_handle.ep_in[ep & 0x7F].data_size;


//	ep &= 0x7F;
//	if (size > m_usbd_handle.ep_in[ep ].ep_size && (ep == 0) ) {
//		size = m_usbd_handle.ep_in[ep ].ep_size;
//	}
	int result = HAL_PCD_EP_Transmit(hpcd, ep, data, size);
	return result;
}

int usbd_stm32_set_address(void *hpcd, uint8_t address) {
	return HAL_PCD_SetAddress(hpcd, address);
}

int usbd_stm32_ep_set_stall(void *hpcd, uint8_t ep) {
	return HAL_PCD_EP_SetStall(hpcd, ep);
}

int usbd_stm32_ep_clear_stall(void *hpcd, uint8_t ep) {
	return HAL_PCD_EP_ClrStall(hpcd, ep);
}

int usbd_stm32_ep_close(void *hpcd, uint8_t epnum) {
	return HAL_PCD_EP_Close(hpcd, epnum);
}
int usbd_stm32_ep_open(void *hpcd, uint8_t epnum, uint8_t epsize,
		uint8_t eptype) {
	// We should add some checks so we do not allocate memory
	// for an endpoint already configured. Eg, when there is a call to open close open.
	// Also, investigate if it is worth adding double buffering support for the PMA

#if defined (USB)
	usbd_stm32_usbfs_v1_config *config = (usbd_stm32_usbfs_v1_config*) m_usbd_handle.driver.driver_config;
	HAL_PCDEx_PMAConfig(&m_hpcd, epnum, PCD_SNG_BUF, config->PmaPos);
	 config->PmaPos += epsize;
#elif defined (USB_OTG_FS)
	 if (epnum & 0x80) {
		 HAL_PCDEx_SetTxFiFo(&m_hpcd, epnum&0x7F,  epsize);
	 }
#endif

	int status = 0;

	status = HAL_PCD_EP_Flush(hpcd, epnum);
	if (status)
		return status;
	status = HAL_PCD_EP_Open(hpcd, epnum, epsize, eptype);
	if (status)
		return status;
	status = HAL_PCD_EP_ClrStall(hpcd, epnum);
	if (status)
		return status;

	// Prepare to receive data to the assigned buffer
	if (!(epnum & 0x80))
		status = HAL_PCD_EP_Receive(hpcd, epnum,
				m_usbd_handle.ep_out[epnum & 0x7F].data_buffer,
				m_usbd_handle.ep_out[epnum & 0x7F].data_size);

	return status;
}

#if defined (USB_OTG_FS)
void OTG_FS_IRQHandler(void)
{
   HAL_PCD_IRQHandler(&m_hpcd);
}
#elif defined STM32F0
void USB_IRQHandler(void) {
	HAL_PCD_IRQHandler(&m_hpcd);
}
#elif defined STM32F1
void USB_LP_CAN1_RX0_IRQHandler(void) {
	HAL_PCD_IRQHandler(&m_hpcd);
}
#elif defined STM32F3
void USB_LP_CAN_RX0_IRQHandler(void) {
	HAL_PCD_IRQHandler(&m_hpcd);
}
#endif

void USBWakeUp_IRQHandler(void) {
	__HAL_USB_WAKEUP_EXTI_CLEAR_FLAG();
}

void SysTick_Handler(void) {
	HAL_IncTick();
}
