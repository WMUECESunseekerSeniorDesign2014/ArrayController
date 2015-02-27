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
 * Pin Definitions
 */
// Port 1
#define LED0		      	0x01 // 0
#define LED1			    0x02 // 1
#define BUTTON2				0x04 // 2
#define BUTTON1 			0x08 // 3
#define CAN_INTn0			0x10 // 4
#define CAN_INTn1	      	0x20 // 5
#define P16					0x40 // 6
#define P17					0x80 // 7
#define P1_UNUSED			0x40 | 0x80

// Port 2
#define ADC4_RDYn			0x01
#define ADC3_RDYn			0x02
#define ADC2_RDYn			0x04
#define ADC1_RDYn			0x08
#define INT_CTS				0x10 // Input from other BPS
#define INT_RTS				0x20 // Output to other BPS
#define P26					0x40
#define P27					0x80
#define P2_UNUSED			0x80 | 0x40

// Port 3
#define P30					0x01
#define ADC4_SIMO			0x02
#define ADC4_SOMI			0x04
#define ADC4_CLK			0x08
#define INT_TX				0x10
#define INT_RX				0x20
#define ADC1_CLK			0x40
#define P37					0x80
#define P3_UNUSED			0x01 | 0x80

// Port 4
#define ADC4_CSn			0x01
#define ADC3_CSn			0x02
#define ADC2_CSn			0x04
#define ADC1_CSn			0x08
#define ADC_RSTn			0x10
#define P45					0x20
#define P46					0x40
#define P47					0x80
#define P4_UNUSED			0x20 | 0x40 | 0x80

// Port 5
#define P50					0x01
#define P51					0x02
#define XT2IN				0x04
#define XT2OUT				0x08
#define P54					0x10
#define P55					0x20
#define ADC1_SIMO			0x40
#define ADC1_SOMI			0x80
#define P5_UNUSED			0x01 | 0x02 | 0x10 | 0x20

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

#endif /* MAIN_H_ */
