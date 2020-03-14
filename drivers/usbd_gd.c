#include "usbd.h"
#include "usbd_gd.h"


//#include "usbd_core.h"
uint8_t g_device_address = 0U;

static usbd_core_handle_struct m_core_handle;
extern usbd_core_handle_struct usb_device_dev  __attribute__ ((alias ("m_core_handle")));


static bscp_usbd_handle_t m_usbd_handle ={ .driver.device_specific = &m_core_handle,
		.driver.transmit = &usbd_gd32_transmit, .driver.set_address =
				&usbd_gd32_set_address, .driver.ep_set_stall =
				&usbd_gd32_ep_set_stall, .driver.ep_clear_stall =
				&usbd_gd32_ep_clear_stall, .driver.ep_open =
				&usbd_gd32_ep_open, .driver.ep_close = &usbd_gd32_ep_close, };

// We need to change this if we want to add SOF support later
usbd_int_cb_struct *usbd_int_fops = NULL;


bscp_usbd_handle_t* usbd_init() {
	bscp_usbd_demo_setup_descriptors(&m_usbd_handle);
	
    /* enable USB pull-up pin clock */ 
    rcu_periph_clock_enable(RCC_AHBPeriph_GPIO_PULLUP);

    /* configure USB model clock from PLL clock */
    rcu_usb_clock_config(RCU_CKUSB_CKPLL_DIV1_5);

    /* enable USB APB1 clock */
    rcu_periph_clock_enable(RCU_USBD);
    
    // Just to make sure... if the driver doesn't do this for us
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11||GPIO_PIN_12);
    
    
 
    	
    	
	usbd_core_init(&m_core_handle);
	
	// We need to enable the IRQ ourselves
	NVIC_EnableIRQ(USBD_LP_CAN0_RX0_IRQn);	
	
	// That doesn't seem to make a difference either 
	gpio_bit_set(USB_PULLUP, USB_PULLUP_PIN);
	
	/* now the usb device is connected */
	usb_device_dev.status = USBD_CONNECTED;
	
	m_usbd_handle.ep_in[0].ep_size = 64;
	m_usbd_handle.ep_out[0].ep_size = 64;

	return &m_usbd_handle;
}

uint8_t  usbd_setup_transaction (usbd_core_handle_struct *pudev){
	bscp_usbd_handle_request(&m_usbd_handle, (usb_setuprequest_t *)pudev->setup_packet);
	
    pudev->ctl_count = ((usb_setuprequest_t *)pudev->setup_packet)->wLength;

	// So here we need to plug in: check if the request is setconfiguration, then 
	pudev->status = USBD_CONFIGURED;

	 return USBD_OK;
}

uint8_t  usbd_out_transaction (usbd_core_handle_struct *pudev, uint8_t ep_num){
	// Code from the GD32 library
	// What is going on with EP0 vs other endpoints? 

    usb_ep_struct *ep = &pudev->out_ep[ep_num];

    if (0U == ep_num) {
        if (0U != pudev->ctl_count) {
            if (ep->trs_len > ep->maxpacket) {
                /* one data packet has been received, update trs_len */
                ep->trs_len -= ep->maxpacket;

                /* continue to receive remain data */
                usbd_ep_rx(pudev, EP0_OUT, ep->trs_buf, (uint16_t)ep->trs_len);
            } else {
                if (USBD_CONFIGURED == pudev->status) {
                    /* device class handle */
                    //pudev->class_data_handler(pudev, USBD_RX, EP0);	
                	// TODO ?? 
                }

                /* enter the control transaction status stage */
                USBD_CONTRL_STATUS_TX();

                pudev->ctl_count = 0U;
            }
        } else {
            /* clear endpoint status_out status */
            USBD_STATUS_OUT_CLEAR(EP0);
        }
    } else {
        if (USBD_CONFIGURED == pudev->status) {
            //pudev->class_data_handler(pudev, USBD_RX, ep_num);

			// So we need to do something equal as above. Receive untill we're
			// received all data. So how do we get the amount there is?
			// I guess that's where the ZLP comes in.... or is the trs_len set somewhere?
		    //

        	int recv = usbd_rx_count_get(pudev, 0x7F & ep_num);
        	if (recv == m_usbd_handle.ep_out[0x7F & ep_num].ep_size) {
        		// Multi Packet Transfer
        		// 
        		m_usbd_handle.ep_out[ep_num & 0x7F].data_left += recv;
        		// TODO: Buffer checking!
        	} else {
        		// Single packet Transfer, or last packet in multi packet transfer
        		m_usbd_handle.ep_out[ep_num & 0x7F].data_left += recv;
        		int received_amount = m_usbd_handle.ep_out[ep_num & 0x7F].data_left;

        		if (m_usbd_handle.ep_out[0x7F & ep_num].data_cb)
        						m_usbd_handle.ep_out[0x7F & ep_num].data_cb(&m_usbd_handle, ep_num,
        								m_usbd_handle.ep_out[ep_num & 0x7F].data_buffer,
        								received_amount);

        		m_usbd_handle.ep_out[ep_num & 0x7F].data_left = 0;

        	}
        	// Continue or restart transfer
        	usbd_ep_rx(pudev, ep_num, (uint8_t*)(m_usbd_handle.ep_out[ep_num & 0x7F].data_buffer + m_usbd_handle.ep_out[ep_num & 0x7F].data_left), m_usbd_handle.ep_out[0x7F & ep_num].ep_size);
        }
    }
    return USBD_OK;
}

uint8_t  usbd_in_transaction (usbd_core_handle_struct *pudev, uint8_t ep_num){
    usb_ep_struct *ep = &pudev->in_ep[ep_num];

    if (0U == ep_num) {
        if (0U != pudev->ctl_count) {
            if (ep->trs_len > ep->maxpacket) {
                /* one data packet has been transmited, update trs_len */
                ep->trs_len -= ep->maxpacket;

                /* continue to transmit remain data */
                usbd_ep_tx (pudev, EP0_IN, ep->trs_buf, (uint16_t)ep->trs_len);

                usbd_ep_rx (pudev, 0U, NULL, 0U);
            } else {

                /* transmit length is maxpacket multiple, so send zero length packet */
                if ((0U == (ep->trs_len % ep->maxpacket)) && (ep->trs_len < pudev->ctl_count)) {
                    usbd_ep_tx (pudev, EP0_IN, NULL, 0U);

                    pudev->ctl_count = 0U;

                    usbd_ep_rx (pudev, 0U, NULL, 0U);
                } else{
                    ep->trs_len = 0U;

                    if (USBD_CONFIGURED == pudev->status) {
                        //pudev->class_data_handler(pudev, USBD_TX, EP0); 
                    }

                    USBD_CONTRL_STATUS_RX();

                    pudev->ctl_count = 0U;
                }
            }
        } else {
            if (0U != g_device_address) {
                USBD_REG_SET(USBD_DADDR, DADDR_USBEN | g_device_address);
                g_device_address = 0U;
            }
        }
    } else {
        ep->trs_len -= ep->trs_count;

        if (0U == ep->trs_len) {
            if (USBD_CONFIGURED == pudev->status) {
                //pudev->class_data_handler(pudev, USBD_TX, ep_num);
				if (m_usbd_handle.ep_in[0x7F & ep_num].data_cb)
					m_usbd_handle.ep_in[0x7F & ep_num].data_cb(&m_usbd_handle, 
						ep_num, m_usbd_handle.ep_in[ep_num & 0x7F].data_buffer,
						m_usbd_handle.ep_in[ep_num & 0x7F].data_size);
            }
        } else {
            usbd_ep_tx(pudev, ep_num, ep->trs_buf, (uint16_t)ep->trs_len);
        }
    }

    return USBD_OK;
}

//--


int usbd_gd32_transmit(void *hpcd, uint8_t ep, void *data, size_t size) {
	int ep_size = m_usbd_handle.ep_in[ep & 0x7F].ep_size;
	m_usbd_handle.ep_in[ep & 0x7F].data_buffer = data;
	m_usbd_handle.ep_in[ep & 0x7F].data_size = size;
	usbd_ep_tx(hpcd, ep, data,
			size < ep_size ? size : ep_size);

	return 0;

}

int usbd_gd32_set_address(void *hpcd, uint8_t address) {
	g_device_address = address & 0x7FU;
    if (0U != g_device_address) {
    	m_core_handle.status  = USBD_ADDRESSED;
    } else {
    	m_core_handle.status  = USBD_DEFAULT;
    }
	return 0;
}

int usbd_gd32_ep_set_stall(void *hpcd, uint8_t ep) {
	usbd_ep_stall(hpcd, ep);
	return 0;
}

int usbd_gd32_ep_clear_stall(void *hpcd, uint8_t ep) {
	usbd_ep_clear_stall(hpcd, ep);
	return 0;
}

int usbd_gd32_ep_close(void *hpcd, uint8_t epnum) {
	usbd_ep_deinit(hpcd, epnum);
	return 0;
}
int usbd_gd32_ep_open(void *hpcd, uint8_t epnum, uint8_t epsize,
		uint8_t eptype) {

	int status = 0;
	void* ep_desc = NULL;
	if (epnum & 0x80) {
	// in
		ep_desc = m_usbd_handle.ep_in[epnum&0x7F].desc;
	} else {
	// out
		ep_desc = m_usbd_handle.ep_out[epnum&0x7F].desc;
	}
	usbd_ep_init(hpcd, ENDP_SNG_BUF,  ep_desc);

	// Prepare to receive data to the assigned buffer
	if (!(epnum & 0x80))
		usbd_ep_rx(hpcd, epnum,
				m_usbd_handle.ep_out[epnum & 0x7F].data_buffer,
				m_usbd_handle.ep_out[epnum & 0x7F].data_size);

	return status;
}




void  USBD_LP_CAN0_RX0_IRQHandler (void) {
    usbd_isr();
}

void  USBD_WKUP_IRQHandler (void) {
    exti_interrupt_flag_clear(EXTI_18);
}


