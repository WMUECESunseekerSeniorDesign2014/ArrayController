/*
 * Initialise ADCs
 * 	- ADC 1, 2, and 3 use USCA1
 *  - ADC 4 uses USCB0
 * 	- Master, 8 bits, mode 0:0, max speed, 3 wire
 */

#include "./inc/main.h"

/*============================= USCB0 SPI Functions ======================================*/

// Initilize UCB0 SPI Port

void adc_spi_init( void )
{
	UCB0CTL1 |= UCSWRST;		//software reset
    UCB0CTL0 = UCCKPL | UCMSB | UCMST | UCMODE_0 | UCSYNC; // CKPL=11, SPI Master, 3-pin mode.
    UCB0CTL1 = UCSSEL1 | UCSSEL0 | UCSWRST; // SMCLK, Software reset

    // Baud Rate Select
    UCB0BR0 = 0x02;				// SPICLK (8 MHz)/8 = BRCLK (1 MHz)
    UCB0BR1 = 0x00;				// SPICLK/8 = BRCLK

	UCB0STAT = 0x00;			//not in loopback mode

    UCB0CTL1 &= ~UCSWRST; // SPI enable

//   UCB0IE |= UCTXIE | UCRXIE; 	// Interrupt enables

}

/*
 * Transmits data on UCB0 SPI connection
 *	- Busy waits until entire shift is complete
 */
void adc_spi_transmit( unsigned char data )
{
	UCB0TXBUF = data;
	while(( UCB0IFG & UCRXIFG ) == 0x00 );	// Wait for Rx completion (implies Tx is also complete)
	UCB0RXBUF;
}

/*
 * Exchanges data on UCB0 SPI connection
 *	- Busy waits until entire shift is complete
 */
unsigned char adc_spi_exchange( unsigned char data )
{
    UCB0TXBUF = data;
	while(( UCB0IFG & UCRXIFG ) == 0x00 );	// Wait for Rx completion (implies Tx is also complete)
	return (UCB0RXBUF);
}
/*================================== End USCB0 SPI Functions ======================================*/

///*============================= USCA1 SPI Functions ======================================*/
//
//// Initilize UCA1 SPI Port
//
//void adc1spi_init( void )
//{
//	UCA1CTL1 |= UCSWRST;		//software reset
//    UCA1CTL0 = UCCKPL | UCMSB | UCMST | UCMODE_0 | UCSYNC; // CKPL=11, SPI Master, 3-pin mode.
//    UCA1CTL1 = UCSSEL1 | UCSSEL0 | UCSWRST; // SMCLK, Software reset
//
//    // Baud Rate Select
//    UCA1BR0 = 0x02;				// SPICLK (8 MHz)/8 = BRCLK (1 MHz)
//    UCA1BR1 = 0x00;				// SPICLK/8 = BRCLK
//
//	UCA1STAT = 0x00;			//not in loopback mode
//    UCA1MCTL = 0x00;			// No modulation
//
//    UCA1CTL1 &= ~UCSWRST; 		// SPI enable
//
////   UCA1IE |= UCTXIE | UCRXIE; 	// Interrupt enables
//
//}
//
///*
// * Transmits data on UCA1 SPI connection
// *	- Busy waits until entire shift is complete
// */
//void adc1spi_transmit( unsigned char data )
//{
//	UCA1TXBUF = data;
//	while(( UCA1IFG & UCRXIFG ) == 0x00 );	// Wait for Rx completion (implies Tx is also complete)
//	UCA1RXBUF;
//}
//
///*
// * Exchanges data on UCA1 SPI connection
// *	- Busy waits until entire shift is complete
// */
//unsigned char adc1spi_exchange( unsigned char data )
//{
//    UCA1TXBUF = data;
//	while(( UCA1IFG & UCRXIFG ) == 0x00 );	// Wait for Rx completion (implies Tx is also complete)
//	return (UCA1RXBUF);
//}
//
///*================================== End USCA1 SPI Functions ======================================*/

