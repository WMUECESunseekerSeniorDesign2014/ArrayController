/*
 * - Implements the following UCB3 interface functions
 *	- init
 *	- transmit
 *	- exchange
 *
 */

// Include files
#include<msp430x54xa.h>
#include "./inc/main.h"
#include "./inc/can.h"

/*
 * Initializes UCA3 and UA1 ports
 */

// Public Function prototypes
void canspi_init(void)
{
	UCA3CTL1 |= UCSWRST;					//software reset
	UCA3CTL0 |= UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC; //data-capt then change; MSB first; Master; 3-pin SPI; sync
	UCA3CTL1 |= UCSSEL_2;				//set SMCLK
	UCA3BR0 = 0x01;					//set clk prescaler to 1
	UCA3BR1 = 0x00;
	UCA3STAT = 0x00;					//not in loopback mode
	UCA3CTL1 &= ~UCSWRST;				//SPI enable turn off software reset
	// UCB3IE |= UCTXIE | UCRXIE;		// Interrupt Enable

	UCA1CTL1 |= UCSWRST;					//software reset
	UCA1CTL0 |= UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC; //data-capt then change; MSB first; Master; 3-pin SPI; sync
	UCA1CTL1 |= UCSSEL_2;				//set SMCLK
	UCA1BR0 = 0x01;					//set clk prescaler to 1
	UCA1BR1 = 0x00;
	UCA1STAT = 0x00;					//not in loopback mode
	UCA1CTL1 &= ~UCSWRST;				//SPI enable turn off software reset
	// UCA1IE |= UCTXIE | UCRXIE;		// Interrupt Enable
}

/*
* Transmits data on UCA3 connection
*	- Busy waits until entire shift is complete
*/
void canspi_transmit_MPPT(unsigned char data)
{
  unsigned char forceread;
  UCA3TXBUF = data;
  while((UCA3IFG & UCRXIFG) == 0x00);	// Wait for Rx completion (implies Tx is also complete)
  forceread = UCA3RXBUF;
}

/*
* Transmits data on UCA1 connection
*	- Busy waits until entire shift is complete
*/
void canspi_transmit_MAIN(unsigned char data)
{
  unsigned char forceread;
  UCA1TXBUF = data;
  while((UCA1IFG & UCRXIFG) == 0x00);	// Wait for Rx completion (implies Tx is also complete)
  forceread = UCA1RXBUF;
}

/*
* Exchanges data on UCA3 connection
*	- Busy waits until entire shift is complete
*	- This function is safe to use to control hardware lines that rely on shifting being finalised
*/
unsigned char canspi_exchange_MPPT(unsigned char data)
{
	UCA3TXBUF = data;
	while((UCA3IFG & UCRXIFG) == 0x00);	// Wait for Rx completion (implies Tx is also complete)
	return(UCA3RXBUF);
}

/*
* Exchanges data on UCA1 connection
*	- Busy waits until entire shift is complete
*	- This function is safe to use to control hardware lines that rely on shifting being finalised
*/
unsigned char canspi_exchange_MAIN(unsigned char data)
{
  UCA1TXBUF = data;
  while((UCA1IFG & UCRXIFG) == 0x00);	// Wait for Rx completion (implies Tx is also complete)
  return(UCA1RXBUF);
}

