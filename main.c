#include "./inc/main.h"

/**
 * @defgroup privRs232Vars Private Main RS-232 Variables
 */
volatile int RX_INT_count = 0;
char test_buffer_PC[60];
char *putPC_ptr, *getPC_ptr;
bool put_status_PC, get_status_PC;
bool AC2PC_RX_flag = FALSE;
/**@}*/

/*
 * main.c
 */
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    test[0].PreDelay = DELAY_0;
    test[0].Test = BLINKY;
    test[0].PostDelay = DELAY_FOREVER; // Repeat the test indefinitely.

    ExecuteTests(test, 1);
	
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
			/** @note As this is necessary for the IO init function, the printf
			 *  was removed.
			 *  @note __delay_cycles() requires a constant value; it cannot have
			 *  a variable parameter passed to it. Therefore, the default case
			 *  does nothing.*/
			break;
	}
}

/*
 * Initialize I/O port directions and states
 *	- Drive unused pins as outputs to avoid floating inputs
 *	- XT1 setup for ACLK and XT2 setup for MCLK
 */
void io_init( void )
{
	/*Port 1 Initialization*/
	P1OUT = 0x00;	/*Set outputs to ground*/
  	P1DIR = LED0 | LED1 | P1_UNUSED;	/*Setup output pins*/
	P1OUT = LED0 | LED1;	/*Turn on LEDs*/
//    P1SEL = BUTTON1 | BUTTON2 | CAN_INTn0 | CAN_INTn1;	 /*Set as interrupt pins*/
	/*Generate an interrupt on a negative edge transition (high to low)*/
    P1IES = BUTTON1 | BUTTON2 | CAN_INTn0 | CAN_INTn1;
    P1IFG = 0x00;       /*Clears all interrupt flags on Port 1*/
    Delay(DELAY_100);
//    P1IE  = BUTTON1 | BUTTON2 | CAN_INTn0 | CAN_INTn1;	/*Enable Interrupts*/

    /*Port 2 Initialization*/
	P2OUT = 0x00;	/*Set outputs to ground*/
 	P2DIR = ADC_CSn | ADC_RESETn | P2_UNUSED;	/*Setup output pins*/
//    P2SEL = ADC_RDYn | DRIVER_SW1 | DRIVER_SW2 | DRIVER_SW3;	/*Set as interrupt pins*/
 	/*Generate an interrupt on a negative edge transition (high to low)*/
    P2IES = ADC_RDYn | DRIVER_SW1 | DRIVER_SW2 | DRIVER_SW3;
    P2IFG = 0x00;	/*Clears all interrupt flags on Port 2*/
    Delay(DELAY_100);
//    P2IE = ADC_RDYn | DRIVER_SW1 | DRIVER_SW2 | DRIVER_SW3;	/*Enable Interrupts*/

    /*Port 3 Initialization*/
	P3OUT = 0x00;	/*Set outputs to ground*/
   	P3DIR = ADC_DIN | ADC_SCLK | TX_EXT | CAN_SCLK2 | P3_UNUSED;	/*Setup output pins*/
    P3SEL = ADC_DIN | ADC_DOUT |ADC_SCLK | TX_EXT | RX_EXT | CAN_SCLK2;	/*Setup pins for secondary function*/
    Delay(DELAY_100);

    /*Port 4 Initialization*/
	P4OUT = 0x00;	/*Set outputs to ground*/
    P4DIR = LED2 | LED3 | LED4 | LED5 | P4_UNUSED;	/*Setup output pins*/
	P4OUT = ~(LED2 | LED3 | LED4 | LED5);	/*Turn on LEDs*/
	Delay(DELAY_100);

    /*Port 5 Initialization*/
	P5OUT = 0x00;	/*Set outputs to ground*/
    P5DIR = XT2OUT | CAN_RSTn2 | CAN_CSn2 | CAN_SI2 | P5_UNUSED;	/*Setup output pins*/
    P5OUT = CAN_CSn2 | CAN_SI2;
    P3OUT = CAN_SCLK2;
    P5SEL = XT2IN | XT2OUT | CAN_CSn2 | CAN_SI2 | CAN_SO2;	/*Setup pins for secondary function*/
    P5OUT = CAN_RSTn2;	/*Send a reset signal*/
    P5OUT &= ~P8_UNUSED;
    Delay(DELAY_100);

    /*Port 6 Initialization*/
	P6OUT = 0x00;	/*Set outputs to ground*/
	P6DIR = P6_UNUSED;	/*Setup output pins*/

    /*Port 7 Initialization*/
	P7OUT = 0x00;	/*Set outputs to ground*/
	P7DIR = XT1OUT | P7_UNUSED;	/*Setup output pins*/
    P7SEL = XT1IN | XT1OUT;	/*Setup pins for secondary function*/
    Delay(DELAY_100);

    /*Port 8 Initialization*/
	P8OUT = 0x00;	/*Set outputs to ground*/
	P8DIR = P8_UNUSED;	/*Setup output pins*/

	/*Port 9 Initialization*/
	P9OUT = 0x00;	/*Set outputs to ground*/
	P9DIR = P9_UNUSED;	/*Setup output pins*/

	/*Port 10 Initialization*/
	P10OUT = 0x00;	/*Set outputs to ground*/
	P10DIR = CAN_SCLK | CAN_CSn | CAN_RSTn | CAN_SI | P10_UNUSED;	/*Setup output pins*/
	P10OUT = CAN_CSn | CAN_SI | CAN_SCLK;
	P10SEL = CAN_SCLK | CAN_CSn | CAN_SI | CAN_SO;
    P10OUT = CAN_RSTn;	/*Send a reset signal*/
    P10OUT &= ~P10_UNUSED;
    Delay(DELAY_100);

    /*Port 11 Initialization*/
 	P11OUT = 0x00;	/*Set outputs to ground*/
 	P11DIR = P11_UNUSED;	/*Setup output pins*/

 	/*Port J Initialization*/
	PJOUT = 0x00;
	PJDIR = 0x0F;	/*set to output as per user's guide*/
}

void timerA_init(void)
{
 /*Set up Watch Crystal and TIMER A*/
 TACCR0 = 255;
 /*The TACCRO initializes the value of Timer A to count up to before setting CCIFG flag
 (255 for 1/128 sec) (8191 1/4 sec) (16383 1/2 sec) (32767 1 sec)  tick time*/
 TACCTL0 = 0x0010;	/*Enables CCIFG to cause an interrupt*/
 TACTL = 0x0110;	/*Set Timer A to ALCK, Start Timer A,  Up mode, Timer-A interrupt enabled*/
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
	extern bool put_status_PC, get_status_PC, AC2PC_RX_flag;
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
             AC2PC_RX_flag = TRUE;
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

/*
* Timer A interrupts
*/
#pragma vector = TIMERA0_VECTOR
 __interrupt void TIMERA_ISR(void)
{

  TACCTL0 &= 0xFFFE;	/*Clear Flags*/
}
