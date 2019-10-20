#include "usbd_stm.h"

#include "usbd.h"

static PCD_HandleTypeDef m_pcd_handle;

static usbd_handle_t m_usbd_handle = {.transmit = &usbd_stm32_transmit, .set_address=&usbd_stm32_set_address};


usbd_handle_t* usbd_init() {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USB_CLK_ENABLE();

	// Configure USB DM/DP pins
	GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
	GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* // Check this
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

	// Set USB Interrupt priority
	HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);

	// Enable USB Interrupt
	HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

	// What are they?
	// Set LL Driver parameters
	m_pcd_handle.Instance = USB;
	m_pcd_handle.Init.dev_endpoints = 8;
	m_pcd_handle.Init.phy_itface = PCD_PHY_EMBEDDED;
	m_pcd_handle.Init.speed = PCD_SPEED_FULL;
	m_pcd_handle.Init.low_power_enable = 0;

	// What is this link? Is this where we hook in?
	//hpcd.pData = pdev;
	//pdev->pData = &hpcd;

	// Initialize LL Driver
	HAL_PCD_Init(&m_pcd_handle);

	// This needs to be adjusted
	HAL_PCDEx_PMAConfig(&m_pcd_handle, 0x00, PCD_SNG_BUF, 0x18);
	HAL_PCDEx_PMAConfig(&m_pcd_handle, 0x80, PCD_SNG_BUF, 0x58);
	HAL_PCDEx_PMAConfig(&m_pcd_handle, 0x81, PCD_SNG_BUF, 0x100);


	return &m_usbd_handle;

}

//--
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd) {
	//USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
	usb_setuprequest_t setupRequest = *((usb_setuprequest_t*)(hpcd->Setup));
	usbd_handle_standard_setup_request(&m_usbd_handle, &setupRequest);
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	// TODO
	//USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	// TODO
	//USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);

	HAL_PCD_EP_SetStall(hpcd, 0x80);
	HAL_PCD_EP_Receive(hpcd, 0x00, NULL, 0);

}
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd) {
	//USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd) {
	//USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, USBD_SPEED_FULL);
	/* Reset Device */
	//USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);



	// Open EP0 OUT
	HAL_PCD_EP_Open(hpcd, 0x00U, 64, 0x00);
	HAL_PCD_EP_Flush(hpcd,0x00);
	HAL_PCD_EP_ClrStall(hpcd, 0x00);


	// Open EP0 IN
	HAL_PCD_EP_Open(hpcd, 0x80U, 64, 0x00);
	HAL_PCD_EP_Flush(hpcd,0x80);
	HAL_PCD_EP_ClrStall(hpcd, 0x80);

}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd) {
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
	//USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum) {
	//USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd) {
	//USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd) {
	//USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
	HAL_PCD_IRQHandler(&m_pcd_handle);
}

void USBWakeUp_IRQHandler(void)
{
  __HAL_USB_WAKEUP_EXTI_CLEAR_FLAG();
}


void SysTick_Handler(void) {
	HAL_IncTick();
}








int usbd_stm32_transmit(uint8_t ep, void* data, size_t size){
	return HAL_PCD_EP_Transmit(&m_pcd_handle, ep, data, size);
}

int usbd_stm32_set_address(uint8_t address) {
	return HAL_PCD_SetAddress(&m_pcd_handle, address);
}
