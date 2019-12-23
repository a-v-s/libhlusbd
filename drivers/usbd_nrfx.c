#include <string.h>
#include <stdbool.h>

#include "nrfx_usbd.h"
#include "nrfx_usbd_errata.h"
#include "nrfx_power.h"
#include "nrfx_systick.h"

#include "usbd.h"
#include "usbd_nrfx.h"

static usbd_handle_t m_usbd_handle = { .driver.device_specific = NULL,
		.driver.transmit = &usbd_nrfx_transmit, .driver.set_address =
				&usbd_nrfx_set_address, .driver.ep_set_stall =
				&usbd_nrfx_ep_set_stall, .driver.ep_clear_stall =
				&usbd_nrfx_ep_clear_stall, .driver.ep_open = &usbd_nrfx_ep_open,
		.driver.ep_close = &usbd_nrfx_ep_close, };

volatile bool m_transfer_out_busy = false;

static void nrfx_openusb_event_handler(nrfx_usbd_evt_t const *const p_event) {
	switch (p_event->type) {

	case NRFX_USBD_EVT_EPTRANSFER: {

		switch (p_event->data.eptransfer.status) {
		case NRFX_USBD_EP_WAITING: {

			if (m_usbd_handle.ep_out[0x7F && p_event->data.eptransfer.ep].buffer
					&& m_usbd_handle.ep_out[0x7F && p_event->data.eptransfer.ep].size) {
				static nrfx_usbd_transfer_t transfer;
				transfer.p_data.rx = m_usbd_handle.ep_out[0x7F
						&& p_event->data.eptransfer.ep].buffer;
				transfer.size = m_usbd_handle.ep_out[0x7F
						&& p_event->data.eptransfer.ep].size;
				nrfx_usbd_ep_transfer(p_event->data.eptransfer.ep, &transfer);

			}

			break;
		}
		case NRFX_USBD_EP_OK: {

			// If the packet is on endpoint 0 in or out
			if (!(0x7F & p_event->data.eptransfer.ep)) {
				nrfx_usbd_setup_clear();
			} else {
				if (p_event->data.eptransfer.ep & 0x80) {
					// OUT
					if (m_usbd_handle.ep_out[0x7F & p_event->data.eptransfer.ep].cb)
						m_usbd_handle.ep_out[0x7F & p_event->data.eptransfer.ep].cb(
								&m_usbd_handle, p_event->data.eptransfer.ep,
								&&p_event->data.eptransfer.p_data.rx,
								p_event->data.eptransfer.size);
				} else {
					// IN
					if (m_usbd_handle.ep_in[p_event->data.eptransfer.ep].cb)
						m_usbd_handle.ep_in[p_event->data.eptransfer.ep].cb(
								&m_usbd_handle, p_event->data.eptransfer.ep,
								&&p_event->data.eptransfer.p_data.tx,
								p_event->data.eptransfer.size);
				}

			}
			break;
		}
		}

	}
		break;
	case NRFX_USBD_EVT_SETUP: {
		static nrfx_usbd_setup_t setup;
		nrfx_usbd_setup_get(&setup);
		usbd_handle_request(&m_usbd_handle, &setup);
		break;
	}

	}
}

int usbd_init(void) {

	NRF_CLOCK->TASKS_HFCLKSTART = 1;
	__DSB();
	while (!NRF_CLOCK->EVENTS_HFCLKSTARTED)
		;

	usbd_demo_setup_descriptors(&m_usbd_handle);

	// Isn't my interrupt being enabled?
	NVIC_EnableIRQ(USBD_IRQn);

	nrfx_usbd_init(nrfx_openusb_event_handler);
	nrfx_usbd_enable();
	nrfx_usbd_setup_clear();

	while (NRFX_POWER_USB_STATE_CONNECTED == nrfx_power_usbstatus_get())
		;
	int status = nrfx_power_usbstatus_get();

	nrfx_usbd_start(true);
	return 0;
}

int usbd_nrfx_transmit(void *pcd_handle, uint8_t ep, void *data, size_t size) {
	m_usbd_handle.ep_in[ep & 0x7F].buffer = data;
	m_usbd_handle.ep_in[ep & 0x7F].size = size;

	static nrfx_usbd_transfer_t transfer;
	transfer.p_data.tx = m_usbd_handle.ep_in[ep & 0x7F].buffer;
	transfer.size = m_usbd_handle.ep_in[ep & 0x7F].size;
	int result = nrfx_usbd_ep_transfer(ep | 0x80, &transfer);

	// Fix for "can't set config #1, error -110"
	if (!ep && !size)
		nrfx_usbd_setup_clear();

	return result;
}

int usbd_nrfx_set_address(void *pcd_handle, uint8_t address) {
	// Automatically done in hardare
	return 0;
}

int usbd_nrfx_ep_set_stall(void *pcd_handle, uint8_t ep) {
	nrfx_usbd_ep_stall(ep);
	return 0;
}

int usbd_nrfx_ep_clear_stall(void *pcd_handle, uint8_t ep) {
	nrfx_usbd_ep_stall_clear(ep);
	return 0;
}

int usbd_nrfx_ep_close(void *pcd_handle, uint8_t epnum) {
	nrfx_usbd_ep_disable(epnum);
	return 0;
}
int usbd_nrfx_ep_open(void *pcd_handle, uint8_t epnum, uint8_t epsize,
		uint8_t eptype) {
	nrfx_usbd_ep_max_packet_size_set(epnum, epsize);
	nrfx_usbd_ep_dtoggle_clear(epnum);
	nrfx_usbd_ep_stall_clear(epnum);
	nrfx_usbd_ep_enable(epnum);
	return 0;
}

