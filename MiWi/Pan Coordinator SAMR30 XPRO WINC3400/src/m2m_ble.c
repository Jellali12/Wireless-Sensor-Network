/**
 *
 * \file
 *
 * \brief WINC3400 BLE Application example code.
 *
 * Copyright (c) 2017 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
#include "driver/include/m2m_types.h"
#include "driver/include/m2m_wifi.h"
#include "atmel_ble_api/include/at_ble_api.h"
#include "atmel_ble_api/include/platform.h"
#include "m2m_ble.h"
//#define BLE_API_DBG 1

static tpfAppWifiCb gpfBleAppWifiCb = NULL;

 //Callback that takes a blob from the BLE API and writes it over the HIF
 void m2m_ble_api_write_func(uint8* msg, uint32 len)
 {
#ifdef BLE_API_DBG
	 M2M_INFO("BLE_API_SEND:%x:%x:%x:%x:%x:%x\n", len, msg[0], msg[1], msg[2], msg[3], msg[4]);
#endif
	 m2m_wifi_ble_api_send(msg, len);
 }

 static void m2m_ble_plf_wait_func(void)
 {
	 while (m2m_wifi_handle_events(NULL) != M2M_SUCCESS);
 }

 void m2m_ble_wifi_callback(uint8 u8MsgType, void * pvMsg)
 {
	 if (u8MsgType == M2M_WIFI_RESP_BLE_API_RECV)
	 {
		 tstrM2mBleApiMsg* rx = (tstrM2mBleApiMsg*)pvMsg;
#ifdef BLE_API_DBG
		 M2M_INFO("BLE_API_RECV:%x:%x:%x:%x:%x:%x\n", \
			rx->u16Len, rx->data[0], rx->data[1], rx->data[2], rx->data[3], rx->data[4]);
#endif
		 platform_receive(rx->data, rx->u16Len);
		 return;
	 }

	 if (gpfBleAppWifiCb) {
		 gpfBleAppWifiCb(u8MsgType, pvMsg);
	 }
 }

sint8 m2m_ble_init(void)
{
	 plf_params_t plf;

	 plf.ble_write_cb = m2m_ble_api_write_func;
	 plf.plf_wait_cb  = m2m_ble_plf_wait_func;

	 if (gpfBleAppWifiCb == NULL)
	 {
	 	 M2M_ERR("WiFi is not initialized yet\n");
	 	 return -1;
	 }
	 
	 if (at_ble_init((void*)&plf) != AT_BLE_SUCCESS)
	 {
		 M2M_ERR("Failed to initialise BLE API\n");
		 return -1;
	 }

	 return M2M_SUCCESS;
 }

sint8 m2m_ble_event_get(at_ble_events_t *event, at_ble_event_parameter_t* params)
{
	if (at_ble_event_get(event, (uint8_t*)params, BLE_EVENT_TIMEOUT) != AT_BLE_SUCCESS)
	{
		*event = AT_BLE_UNDEFINED_EVENT;
	}
	// This stimulates BLE operation.
	while (m2m_wifi_handle_events(NULL) != M2M_SUCCESS){
	}
	

	return AT_BLE_SUCCESS;
}


sint8 m2m_ble_wifi_init(tstrWifiInitParam * param)
{
	gpfBleAppWifiCb = param->pfAppWifiCb;
	param->pfAppWifiCb = m2m_ble_wifi_callback;
#ifdef ETH_MODE
	param->strEthInitParam.pfAppWifiCb = m2m_ble_wifi_callback;
#endif
	return m2m_wifi_init(param);
}


