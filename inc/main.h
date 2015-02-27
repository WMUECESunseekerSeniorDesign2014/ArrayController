#include<msp430x54xa.h>

/**
* @enum bool
*
* An enumeration defining a boolean (bool) type.
*/
typedef enum { TRUE, FALSE } bool;

/**
 * @note Include guard to prevent recursive inclusion.
 */
#ifndef MAIN_H_
#define MAIN_H_

#include "./adc.h"
#include "./rs232.h"

/**
 * @note The pins are in the following order: 0, 1, 2, 3, 4, 5, 6, 7, unused pins.
 * @defgroup pindefs Pin Definitions
 * @{
 */
/**
 * @defgroup port1 Port One Pin Definitions
 * @{
 */
#define LED0		      	0x01 /** LED0: Output */
#define LED1			    0x02 /** LED1: Output */
#define BUTTON2				0x04 /** BUTTON2: Input */
#define BUTTON1 			0x08 /** BUTTON1: Input */
#define CAN_INTn0			0x10 /** CAN_INTn0: Input */
#define CAN_INTn1	      	0x20 /** CAN_INTn1: Input */
#define P16					0x40 /** P16: N/A */
#define P17					0x80 /** P17: N/A */
#define P1_UNUSED			0x40 | 0x80
/**@}*/

/**
 * @defgroup port2 Port Two Pin Definitions
 * @{
 */
#define ADC_CSn				0x01 /** ADC_CSn: Output */
#define ADC_RESETn			0x02 /** ADC_RESETn: Output */
#define ADC_RDYn			0x04 /** ADC_RDYn: Input */
#define DRIVER_SW1			0x08 /** DRIVER_SW1: Input */
#define DRIVER_SW2			0x10 /** DRIVER_SW2: Input */
#define DRIVER_SW3			0x20 /** DRIVER_SW3: Input */
#define P26					0x40 /** P26: N/A */
#define P27					0x80 /** P27: N/A */
#define P2_UNUSED			0x80 | 0x40
/**@}*/

/**
 * @defgroup port3 Port Three Pin Definitions
 * @{
 */
#define P30					0x01 /** P30: N/A */
#define ADC_DIN				0x02 /** ADC_DIN: Output */
#define ADC_DOUT			0x04 /** ADC_DOUT: Input */
#define ADC_SCLK			0x08 /** ADC_SCLK: Output */
#define TX_EXT				0x10 /** TX_EXT: Output */
#define RX_EXT				0x20 /** RX_EXT: Input */
#define CANSCLK2			0x40 /** CANSCLK2: Output */
#define P37					0x80 /** P37: N/A */
#define P3_UNUSED			0x01 | 0x80
/**@}*/

/**
 * @defgroup port4 Port Four Pin Definitions
 * @{
 */
#define LED2				0x01 /** LED2: Output */
#define LED3				0x02 /** LED3: Output */
#define LED4				0x04 /** LED4: Output */
#define LED5				0x08 /** LED5: Output */
#define P44					0x10 /** P44: N/A */
#define P45					0x20 /** P45: N/A */
#define CANRX1n2			0x40 /** CANRX1n2: Input */
#define CANRX2n2			0x80 /** CANRX1n2: Input */
#define P4_UNUSED			0x10 | 0x20
/**@}*/

/**
 * @defgroup port5 Port Five Pin Definitions
 * @{
 */
#define P50					0x01 /** P50: N/A */
#define P51					0x02 /** P51: N/A */
#define XT2IN				0x04 /** XT2IN: Input */
#define XT2OUT				0x08 /** XT2OUT: Output */
#define CANRSTn2			0x10 /** CANRSTn2: Output */
								 /** @todo Find out if this is request-to-send. */
#define CANCSn2				0x20 /** CANCSn2: Output */
#define CANSI2				0x40 /** CANSI2: Output */
#define CANSO2				0x80 /** CANSO2: Input */
#define P5_UNUSED			0x01 | 0x02
/**@}*/

// Port 6
#define S0					0x01
#define S1					0x02
#define S2					0x04
#define S3					0x08
#define S4					0x10
#define S5					0x20
#define S6					0x40
#define S7					0x80
#define P6_UNUSED			0x00

// Port 7
#define XT1IN				0x01
#define XT1OUT				0x02
#define P72					0x04
#define P73					0x08
#define P74					0x10
#define P75					0x20
#define P76					0x40
#define P77					0x80
#define P7_UNUSED			0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80

// Port 8
#define JUMPER2				0x01
#define JUMPER1				0x02
#define JUMPER0				0x04
#define P83					0x08
#define P84					0x10
#define P85					0x20
#define P86					0x40
#define CAN_RSTn			0x80
#define P8_UNUSED			0x08 | 0x10 | 0x20 | 0x40

// Port 9
#define SCLK_1				0x01
#define MOSI_2				0x02
#define MISO_2				0x04
#define SCLK_2				0x08
#define MOSI_1				0x10
#define MISO_1				0x20
#define CSn_1				0x40
#define CSn_2				0x80
#define P9_UNUSED			0x00

// Port 10
#define CAN_CSn				0x01
#define CAN_SIMO			0x02
#define CAN_SOMI			0x04
#define CAN_SCLK			0x08
#define EXT_TX				0x10
#define EXT_RX				0x20
#define RELAY4				0x40
#define RELAY3				0x80
#define P10_UNUSED			0x00

// Port 11
#define RELAY2				0x01
#define RELAY1				0x02
#define RELAY0				0x04

// Port J
#define JTAG_TDO			0x01
#define JTAG_TDI			0x02
#define JTAG_TMS			0x04
#define JTAG_TCK			0x08

/**@}*/

#endif /* MAIN_H_ */
