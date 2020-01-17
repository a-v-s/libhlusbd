/*

 File:         usbd_nrfx.c
 Author:    André van Schoubroeck
 License:    MIT


 MIT License

 Copyright (c) 2018, 2019 André van Schoubroeck

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
                static nrfx_usbd_transfer_t transfer = { 0 };

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
                    // IN
                    if (!nrfx_usbd_errata_154()) {
                        nrfx_usbd_setup_clear();
                    }

                    if (m_usbd_handle.ep_in[0x7F & p_event->data.eptransfer.ep].cb)
                        m_usbd_handle.ep_in[0x7F & p_event->data.eptransfer.ep].cb(
                                &m_usbd_handle, p_event->data.eptransfer.ep,
                                m_usbd_handle.ep_out[0x7F
                                        & p_event->data.eptransfer.ep].buffer,
                                m_usbd_handle.ep_out[0x7F
                                        & p_event->data.eptransfer.ep].size);

                } else {
                    // OUT

                    if (!nrfx_usbd_errata_154()) {
                        nrfx_usbd_setup_clear();
                    }

                    if (m_usbd_handle.ep_out[p_event->data.eptransfer.ep].cb)
                        m_usbd_handle.ep_out[p_event->data.eptransfer.ep].cb(
                                &m_usbd_handle, p_event->data.eptransfer.ep,
                                m_usbd_handle.ep_out[p_event->data.eptransfer.ep].buffer,
                                nrfx_usbd_epout_size_get(
                                        p_event->data.eptransfer.ep));

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

