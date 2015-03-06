/*
 * rs232.c
 *
 *  Created on: Dec 15, 2014
 *      Author: Joel
 *      Based off of Dr. Bazuin's BPS code.
 */
#include "./inc/main.h"

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
