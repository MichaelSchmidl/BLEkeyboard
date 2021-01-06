/* ----------------------------------------------------------------------------
 * Copyright (c) 2015-2017 Semiconductor Components Industries, LLC (d/b/a
 * ON Semiconductor), All Rights Reserved
 *
 * This code is the property of ON Semiconductor and may not be redistributed
 * in any form without prior written permission from ON Semiconductor.
 * The terms of use and warranty for this code are covered by contractual
 * agreements between ON Semiconductor and the licensee.
 *
 * This is Reusable Code.
 *
 * ----------------------------------------------------------------------------
 * app.c
 * - Main application file
 * ----------------------------------------------------------------------------
 * $Revision: 1.13 $
 * $Date: 2018/10/10 19:28:55 $
 * ------------------------------------------------------------------------- */

#include "app.h"
#include "usb_hid_keys.h"

static struct on_semi_banner_str on_semi_banner[] =
{
#if 0
	{ 0x15,    /* Key R     */ 0x02 },
    { 0x16,    /* Key S     */ 0x02 },
    { 0x0f,    /* Key L     */ 0x02 },
    { 0x1e,    /* Key 1     */ 0x00 },
    { 0x27,    /* Key 0     */ 0x00 },
    { 0x2c,    /* Key       */ 0x00 },
    { 0x12,    /* Key O     */ 0x02 },
    { 0x11,    /* Key N     */ 0x02 },
    { 0x2c,    /* Key       */ 0x00 },
    { 0x16,    /* Key S     */ 0x02 },
    { 0x08,    /* Key e     */ 0x00 },
    { 0x10,    /* Key m     */ 0x00 },
    { 0x0c,    /* Key i     */ 0x00 },
    { 0x06,    /* Key c     */ 0x00 },
    { 0x12,    /* Key o     */ 0x00 },
    { 0x11,    /* Key n     */ 0x00 },
    { 0x07,    /* Key d     */ 0x00 },
    { 0x18,    /* Key u     */ 0x00 },
    { 0x06,    /* Key c     */ 0x00 },
    { 0x17,    /* Key t     */ 0x00 },
    { 0x12,    /* Key o     */ 0x00 },
    { 0x15,    /* Key r     */ 0x00 },
    { 0x2c,    /* Key       */ 0x00 },
    { 0x0b,    /* Key H     */ 0x02 },
    { 0x0c,    /* Key I     */ 0x02 },
    { 0x07,    /* Key D     */ 0x02 },
    { 0x2c,    /* Key       */ 0x00 },
    { 0x0e,    /* Key K     */ 0x02 },
    { 0x08,    /* Key e     */ 0x00 },
    { 0x1c,    /* Key y     */ 0x00 },
    { 0x05,    /* Key b     */ 0x00 },
    { 0x12,    /* Key o     */ 0x00 },
    { 0x04,    /* Key a     */ 0x00 },
    { 0x15,    /* Key r     */ 0x00 },
    { 0x07,    /* Key d     */ 0x00 },
    { 0x2c,    /* Key       */ 0x00 },
    { 0x07,    /* Key D     */ 0x02 },
    { 0x08,    /* Key e     */ 0x00 },
    { 0x10,    /* Key m     */ 0x00 },
    { 0x12,    /* Key o     */ 0x00 },
    { 0x2c,    /* Key       */ 0x00 },
    { 0x28,    /* Key ENTER */ 0x00 }
#else
#if 1
    { KEY_LEFT,    /* Key Alt-Left */ KEY_MOD_LALT } // F7 ???
#else
	{ KEY_MUTE,    0 }
#endif
#endif
};
static uint32_t on_semi_banner_size = \
    sizeof(on_semi_banner) / \
    sizeof(struct on_semi_banner_str);
static uint32_t act_key = 0;

ARM_DRIVER_USART *uart;
DRIVER_GPIO_t *gpio;
char tx_buffer[1] __attribute__ ((aligned(4))) = ".";
char rx_buffer[sizeof(tx_buffer)] __attribute__ ((aligned(4)));

/* ----------------------------------------------------------------------------
 * Function      : void Button_EventCallback(void)
 * ----------------------------------------------------------------------------
 * Description   : This function is a callback registered by the function
 *                 Initialize. Based on event argument different actions are
 *                 executed.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Button_EventCallback(uint32_t event)
{
    static bool ignore_next_dio_int = false;
    if (ignore_next_dio_int)
    {
        ignore_next_dio_int = false;
    }
    /* Button is pressed: Ignore next interrupt.
     * This is required to deal with the debounce circuit limitations. */
    else if (event == GPIO_EVENT_0_IRQ)
    {
		    ignore_next_dio_int = true;

		    /* Set the key status */
#if 0
		    app_env.key_pushed = true;
		    app_env.key_state = KEY_PUSH;
#endif
//		    uart->Send(tx_buffer, sizeof(tx_buffer)); /* start transmission */
//TODO:       PRINTF("BUTTON PRESSED: START TRANSMISSION\n");
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void Button_EventCallback(void)
 * ----------------------------------------------------------------------------
 * Description   : This function is a callback registered by the function
 *                 Initialize. Based on event argument different actions are
 *                 executed.
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Usart_EventCallBack(uint32_t event)
{
	typedef enum{
		eWaitingForStatus,
		eWaitingForProgramChangeData // 0xCx nn
	} midiRecvState_t;
	static midiRecvState_t midiRecvState = eWaitingForStatus;

    /* Check if receive complete event occurred */
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE)
    {
    	uint8_t recv = rx_buffer[0];
        /* Receive next data */
        uart->Receive(rx_buffer, sizeof(rx_buffer));
        // handle received data
        switch (midiRecvState)
        {
        	case eWaitingForStatus:
        	{
        		if ( 0xC0 == (recv & 0xF0))
        		{
            		midiRecvState = eWaitingForProgramChangeData;
        		}
        	}break;
        	case eWaitingForProgramChangeData:
        	{
    		    app_env.key_pushed = true;
    		    app_env.key_state = KEY_PUSH;
        		midiRecvState = eWaitingForStatus;
        	}break;
        	default:
        		midiRecvState = eWaitingForStatus;
        }
    }
}

/* ----------------------------------------------------------------------------
 * Function      : void ToggleLed(uint32_t n, uint32_t delay_ms)
 * ----------------------------------------------------------------------------
 * Description   : Toggle the led pin.
 * Inputs        : n        - number of toggles
 *                  : delay_ms - delay between each toggle [ms]
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void ToggleLed(uint32_t n, uint32_t delay_ms)
{
    for (; n > 0; n--)
    {
        /* Refresh the watchdog */
        Sys_Watchdog_Refresh();

        /* Toggle led diode */
        gpio->ToggleValue(LED_DIO_NUM);

        /* Delay */
        Sys_Delay_ProgramROM((delay_ms / 1000.0) * SystemCoreClock);
    }
}


/* ----------------------------------------------------------------------------
 * Function      : void Restart_Keystroke_Env(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize keyboard environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Restart_Keystroke_Env(void)
{
    app_env.key_pushed = false;
    app_env.key_state = KEY_IDLE;
}

/* ----------------------------------------------------------------------------
 * Function      : void Update_Keystroke_Env(void)
 * ----------------------------------------------------------------------------
 * Description   : Initialize keyboard environment
 * Inputs        : None
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
void Update_Keystroke_Env(void)
{
    app_env.key_state = KEY_UPDATE;
}

/* ----------------------------------------------------------------------------
 * Function      : Send_Keystroke(void)
 * ----------------------------------------------------------------------------
 * Description   : Send keystroke to the host
 * Inputs        : key, mod_id
 * Outputs       : None
 * Assumptions   : None
 * ------------------------------------------------------------------------- */
static void Send_Keystroke(const uint8_t key, const uint8_t mod_id)
{
    switch (app_env.key_state)
    {
        case KEY_PUSH:
        {
            app_env.key_state = KEY_REL;
            Hogpd_ReportUpdReq(ble_env[0].conidx,
                               key,
                               mod_id);
        }
        break;

        case KEY_UPDATE:
        {
            /* Restart Keyboard Environment */
            Restart_Keystroke_Env();

            /* Prepare next banner's key */
            act_key++;
            act_key = act_key % on_semi_banner_size;
        }
        break;

        case KEY_REL:
        default:
        {
        }
        break;
    }
}

int main(void)
{
    App_Initialize();

    SystemCoreClockUpdate();

    // start receiving data
    uart->Receive(rx_buffer, sizeof(rx_buffer));

    /* Main application loop:
     * - Run the kernel scheduler
     * - Update the battery voltage
     * - Refresh the watchdog and wait for an event before continuing
     * - Check for the custom services
     */
    while (1)
    {
        Kernel_Schedule();

        /* Send battery level to all connected clients if battery service is
         * enabled */
        for (unsigned int i = 0; i < NUM_MASTERS; i++)
        {
            if (ble_env[i].state == APPM_CONNECTED &&
                VALID_BOND_INFO(ble_env[i].bond_info.STATE))
            {
                if (app_env.send_batt_ntf[i] && bass_support_env[i].enable)
                {
                    app_env.send_batt_ntf[i] = 0;
                    Batt_LevelUpdateSend(ble_env[i].conidx,
                                         app_env.batt_lvl, 0);
                }
            }
        }

        /* Send keystrokes to client if DIO5 is pushed */
        if (ble_env[0].state == APPM_CONNECTED &&
            VALID_BOND_INFO(ble_env[0].bond_info.STATE))
        {
            if (hogpd_support_env.enable == true && app_env.key_pushed == true)
            {
                Send_Keystroke(on_semi_banner[act_key].key,
                               on_semi_banner[act_key].mod);
            }
        }

        /* Refresh the watchdog timer */
        Sys_Watchdog_Refresh();

        /* Wait for an event before executing the scheduler again */
        SYS_WAIT_FOR_EVENT;
    }
}
