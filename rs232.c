/*
 * rs232.c
 *
 *  Created on: Dec 15, 2014
 *      Author: Joel
 *      Based off of Dr. Bazuin's BPS code.
 */
#include "./inc/main.h"

/*
 * Internal RS-232 Communication Code (From MCU to GPS)
 */
// Initializes the ICL3222 to begin internal transmitting.
void AC2GPS_init(void) {
	UCA0CTL1 |= UCSWRST;                      // Resets the state machine governing the UART.
	UCA0CTL1 |= UCSSEL_2;                     // Sets the USCI 0 Clock Source 2 to SMCLK.
	UCA0BRW = 208;                              // 8 MHz 38400 (see User's Guide) ///@todo Figure out how he chooses the baud rate.
	UCA0MCTL |= UCBRS_3 + UCBRF_0;            // Modulation UCBRSx=3, UCBRFx=0 (Oversampling disabled)

	UCA0IFG &= ~UCTXIFG;			// Clear the transmit and receive interrupt flags.
	UCA0IFG &= ~UCRXIFG;
	UCA0CTL1 &= ~UCSWRST;			// Initialize state machine
}

// Loads the Transmit (TX) Buffer with data.
void AC2GPS_putchar(char data)
{
    while((UCA0IFG & UCTXIFG) == 0); // Wait until the interrupt flag is set; this signals that the buffer is ready for another character.
    UCA0TXBUF = data; // Once the interrupt flag is set, load the transmit buffer with data.
}


// Retrieves data from the Receive (RX) Buffer
unsigned char AC2GPS_getchar(void)
{
    while((UCA0IFG & UCRXIFG) == 0);
    return(UCA0RXBUF);
}

// Transmits data to the UART lines by placing data in the TX buffer.
int AC2GPS_puts(char *str)
{
	int i;
    char ch;
    i = 0;

    while((ch=*str)!= '\0') // While the current character is NOT the null terminator...
    {
    	AC2GPS_putchar(ch); // Place that character into the buffer.
    	str++; // Move to the next character
    	i++;
    }
    AC2GPS_putchar(0x0A); // Sends out a Line Feed (LF) and Carriage Return (CR) to signify that the transmit has completed.
    AC2GPS_putchar(0x0D);

    return(i); // Return the number of characters sent.
}

// Retrieves data sent through the RS-232 communication channel.
// Probably too slow to be used; the interrupt should be used instead.
int AC2GPS_gets(char *ptr)
{
    int i;
    i = 0;
    while (1) {
          *ptr = AC2GPS_getchar(); // Retrieve a character from the RX buffer.
          if (*ptr == 0x0D){ // If that character is a CR, then the transfer is complete.
             *ptr = 0;
             return(i); // Returns the number of characters received.
          }
          else // If it isn't, then move to the next character in the buffer.
          {
          	ptr++;
          	i++;
          }
     }
}
// Initializes a TX operation.
void AC2GPS_put_int(void)
{
    extern char *put_ptr; // This is declared in the main file!
    extern char put_status;
    char ch;

    ch = *put_ptr++;

	if (ch == '\0') // If the character is the null terminator...
	{
		UCA0IE &= ~UCTXIE; // Disable the TX interrupt.
		put_status = FALSE;
	}
	else
	{
		UCA0TXBUF = ch; // Else, enable the TX interrupt and place the character into the TX buffer.
		UCA0IE |= UCTXIE;
		put_status = TRUE;
	}
}

/*
 * External RS-232 Communication Code (From MCU to PC)
 *
 * Much of this is the same as the internal RS-232 code up above.
 */
void AC2PC_init(void)
{
    UCA3CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA3CTL1 |= UCSSEL_2;                     // SMCLK
    UCA3BRW = 833;                              // 8 MHz 9600 (see User's Guide) ///@todo Inquire about this baud rate.
    UCA3MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0

	UCA3IFG &= ~UCTXIFG;			//Clear Xmit and Rec interrupt flags
	UCA3IFG &= ~UCRXIFG;
	UCA3CTL1 &= ~UCSWRST;			// initalize state machine
}

void AC2PC_putchar(char data)
{
    while((UCA3IFG & UCTXIFG) == 0);
    UCA3TXBUF = data;
}

unsigned char AC2PC_getchar(void)
{
    while((UCA3IFG & UCRXIFG) == 0);
    return(UCA3RXBUF);
}

int AC2PC_puts(char *str)
{
	int i;
    char ch;
    i = 0;

    while((ch=*str)!= '\0')
    {
    	AC2PC_putchar(ch);
    	str++;
    	i++;
    }
    AC2PC_putchar(0x0A);
    AC2PC_putchar(0x0D);

    return(i);
}

int AC2PC_gets(char *ptr)
{
    int i;
    i = 0;
    while (1) {
          *ptr = AC2PC_getchar();
          if (*ptr == 0x0D){
             *ptr = 0;
             return(i);
          }
          else
          {
          	ptr++;
          	i++;
          }
     }
}

void AC2PC_put_int(void)
{
    extern char *putPC_ptr;
    extern char put_status_PC;
    char ch;

    ch = *putPC_ptr++;

	if (ch == '\0')
	{
		UCA3IE &= ~UCTXIE;
		put_status_PC = FALSE;
	}
	else
	{
		UCA3TXBUF = ch;
		UCA3IE |= UCTXIE;
		put_status_PC = TRUE;
	}
}
