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

/**
 * A wrapper for __delay_cycles() which allows the developer to create a **synchronous** delay
 * of a number in microseconds.
 *
 * @note This should not be used in conjunction with functions that rely on ISRs!
 *
 * @param[in] delayConstant One of the constants defined in Delay Timings.
 */
extern void Delay(unsigned int delayConstant) {
	switch(delayConstant) {
		case DELAY_100:
			__delay_cycles(DELAY_100);
			break;
		case DELAY_500:
			__delay_cycles(DELAY_500);
			break;
		case DELAY_1000:
			__delay_cycles(DELAY_1000);
			break;
		case DELAY_FOREVER: // If DELAY_FOREVER is passed in, don't do anything!
			break;
		default: // If an unknown value is given, then don't perform the delay.
			printf("Unknown delay constant: %d", delayConstant);
			break;
	}
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
