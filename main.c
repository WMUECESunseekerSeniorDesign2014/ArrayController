#include "./inc/main.h"

/**
 * @defgroup privRs232Vars Private Main RS-232 Variables
 */
volatile int RX_INT_count = 0;
char test_buffer[60];
char test_buffer_PC[60];
/**@}*/

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	return 0;
}

/*
 * Interrupts
 */
/*
* BPS2PC Interrupt Service Routine
*/
	#pragma vector=USCI_A3_VECTOR
__interrupt void USCI_A3_ISR(void)
{
	extern char *putPC_ptr, *getPC_ptr;
	extern char put_status_PC, get_status_PC, BPS2PC_RX_flag;
	char ch;

    switch(__even_in_range(UCA3IV,16))
    {
    case 0:		                              // Vector 0 - no interrupt
      break;
    case 2:                                   // Data Received - UCRXIFG
          ch = UCA3RXBUF;
          *getPC_ptr++ = ch;
          if (ch == 0x0D){
             *getPC_ptr = 0;
             getPC_ptr = &test_buffer_PC[0];
             BPS2PC_RX_flag = TRUE;
		     get_status_PC = FALSE;
          }
          else
          {
		     get_status_PC = TRUE;
         }
     break;
    case 4:                                   // TX Buffer Empty - UCTXIFG
      ch = *putPC_ptr++;
	  if (ch == '\0')
	  {
	  	UCA3IE &= ~UCTXIE;
		put_status_PC = FALSE;
	  }
	  else
	  {
		UCA3TXBUF = ch;
	  }
      break;
    default:
      break;
    }
}
