#ifndef SRC_COMMON_BOARD_TUYA_H_
#define SRC_COMMON_BOARD_TUYA_H_

/**************************** UART for Tuya ZT3L ***********************************
*    UART_TX_PA2 = GPIO_PA2,
*    UART_TX_PB1 = GPIO_PB1,
*    UART_TX_PC2 = GPIO_PC2,
*    UART_TX_PD0 = GPIO_PD0,
*    UART_TX_PD3 = GPIO_PD3,
*    UART_TX_PD7 = GPIO_PD7,
*    UART_RX_PA0 = GPIO_PA0,
*    UART_RX_PB0 = GPIO_PB0,
*    UART_RX_PB7 = GPIO_PB7,
*    UART_RX_PC3 = GPIO_PC3,
*    UART_RX_PC5 = GPIO_PC5,
*    UART_RX_PD6 = GPIO_PD6,
*/
#define GPIO_UART_TX            UART_TX_PD0
#define GPIO_UART_RX            UART_RX_PA0

/**************************** Configure UART ***************************************/
#if UART_PRINTF_MODE
#define DEBUG_INFO_TX_PIN       UART_TX_PB1
//GPIO_SWS
 //GPIO_PB5
#define DEBUG_BAUDRATE          9600
#endif /* UART_PRINTF_MODE */


/********************* Configure fake LED for bootloader ***************************/

// LED
#define LED_R                                           GPIO_PC2        //C2 -- red                     PWM1
#define LED_G                                           GPIO_PC3        //C3 -- green           PWM2
#define LED_B                                           GPIO_PC4        //C4 -- blue            PWM3

#define PWM_R_CHANNEL                           0//PWM1
#define PWM_R_CHANNEL_SET()                     do{     \
                                                                                gpio_set_func(LED_R, AS_PWM0);          \
                                                                        }while(0)

#define PWM_G_CHANNEL                           1//PWM0
#define PWM_G_CHANNEL_SET()                     do{     \
                                                                                gpio_set_func(LED_G, AS_PWM1);          \
                                                                        }while(0)

#define PWM_B_CHANNEL                           2//PWM3
#define PWM_B_CHANNEL_SET()                     do{     \
                                                                                gpio_set_func(LED_B, AS_PWM2);          \
                                                                        }while(0)

#define R_LIGHT_PWM_CHANNEL                     PWM_R_CHANNEL
#define G_LIGHT_PWM_CHANNEL                     PWM_G_CHANNEL
#define B_LIGHT_PWM_CHANNEL                     PWM_B_CHANNEL
#define R_LIGHT_PWM_SET()                       PWM_R_CHANNEL_SET()
#define G_LIGHT_PWM_SET()                       PWM_G_CHANNEL_SET()
#define B_LIGHT_PWM_SET()                       PWM_B_CHANNEL_SET()

//LED_Y and LED_W as GPIO.
#define LED_Y                                           GPIO_PB4
#define LED_W                                           GPIO_PB5

#define PB5_FUNC                                        AS_GPIO
#define PB5_OUTPUT_ENABLE                       1
#define PB5_INPUT_ENABLE                        0

#define PB4_FUNC                                        AS_GPIO
#define PB4_OUTPUT_ENABLE                       1
#define PB4_INPUT_ENABLE                        0

#define LED_POWER                                       LED_W
#define LED_PERMIT                                      LED_Y

// BUTTON
#define BUTTON1                  GPIO_PD4
#define PD4_FUNC                 AS_GPIO
#define PD4_OUTPUT_ENABLE        0
#define PD4_INPUT_ENABLE         1
#define PULL_WAKEUP_SRC_PD4      PM_PIN_PULLUP_10K

#define BUTTON2                   GPIO_PD3
#define PD3_FUNC                  AS_GPIO
#define PD3_OUTPUT_ENABLE         0
#define PD3_INPUT_ENABLE          1
#define PULL_WAKEUP_SRC_PD3       PM_PIN_PULLUP_10K

enum{
	VK_SW1 = 0x01,
	VK_SW2 = 0x02
};

#define	KB_MAP_NORMAL	{\
		{VK_SW1,}, \
		{VK_SW2,}, }

#define	KB_MAP_NUM		KB_MAP_NORMAL
#define	KB_MAP_FN		KB_MAP_NORMAL

#define KB_DRIVE_PINS  {NULL }
#define KB_SCAN_PINS   {BUTTON1,  BUTTON2}


#endif /* SRC_COMMON_BOARD_TUYA_H_ */
