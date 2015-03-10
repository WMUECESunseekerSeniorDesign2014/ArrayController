#include "./inc/main.h"
#include "./inc/test/test_suite.h"

/**
 * @defgroup privRs232Vars Private Main RS-232 Variables
 */
volatile int RX_INT_count = 0;
char tx_PC_buffer[60];
char rx_PC_buffer[60];
char *putPC_ptr, *getPC_ptr;
bool put_status_PC, get_status_PC;
bool Prompt_Active = FALSE;
/**@}*/


/*
 * main.c
 */
int main(void) {
	UnitTest test[2];
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    _DINT();	/*Disable interrupts*/

    /*Initializations*/
    io_init();
    clock_init();
    timerA_init();
    timerB_init();

    // Initialize the RS-232 interface.
    AC2PC_init();
    UCA0IE |= UCRXIE; // Enable interrupts on the RX line.

    _EINT();	/*Enable interrupts*/

	// No interrupt has come through yet, so mark this to FALSE initially.
	put_status_PC = FALSE;
	Prompt_Active = FALSE;

    while(1) {
    }
	
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
		case DELAY_3750:
			__delay_cycles(DELAY_3750);
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
    P2OUT &= ~ADC_RESETn ;
    Delay(DELAY_100);
    Delay(DELAY_100);
    P2OUT |= ADC_RESETn ;
//    P2IE = ADC_RDYn | DRIVER_SW1 | DRIVER_SW2 | DRIVER_SW3;	/*Enable Interrupts*/

    /*Port 3 Initialization*/
	P3OUT = 0x00;	/*Set outputs to ground*/
   	P3DIR = ADC_DIN | ADC_SCLK | TX_EXT | CAN_SCLK2 | P3_UNUSED;	/*Setup output pins*/
   	P3DIR &= ~(ADC_DOUT | RX_EXT);
    P3SEL = ADC_DIN | ADC_DOUT |ADC_SCLK | TX_EXT | RX_EXT | CAN_SCLK2;	/*Setup pins for secondary function*/
    P3OUT = CAN_SCLK2;
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
    P5SEL = XT2IN | XT2OUT | CAN_CSn2 | CAN_SI2 | CAN_SO2;	/*Setup pins for secondary function*/
    P5OUT = CAN_RSTn2;	/*Send a reset signal*/
    P5OUT &= ~P5_UNUSED;
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

/*
* Initialize Timer A
*/
void timerA_init(void)
{
 /*Set up Watch Crystal and TIMER A*/
 TA0CCR0 = 32767;
 /*The TACCRO initializes the value of Timer A to count up to before setting CCIFG flag
 (255 for 1/128 sec) (8191 1/4 sec) (16383 1/2 sec) (32767 1 sec)  tick time*/
 TA0CCTL0 = 0x0010;	/*Enables CCIFG to cause an interrupt*/
 TA0CTL = 0x0110;	/*Set Timer A to ACLK, Start Timer A,  Up mode, Timer-A interrupt enabled*/
}


/*
*  Initialize Timer B
*/
void timerB_init( void )
{
	/*Set TBCTL to SMCLK at 8 MHz with prescaler of /8 */
	//TBCTL = CNTL_0 | TBSSEL_2 | ID_3 | TBCLR;	/*TBSSEL_2 = SMCLK, ID_3 = SMCLK/8, clear TBR*/
	//TBCCR0 = (34000);		/*Set timer to count to this value = */
	//TBCCTL0 = CCIE;	/*Enable CCR0 interrupt*/
	//TBCTL |= MC_1;	/*Set timer to 'up' count mode*/

	/*Assign ACLK to Timer B with prescaler /2*/
	TBCTL = CNTL_0 | TBSSEL_1 | ID_1 | TBCLR;	/*TBSSEL_1 = ACLK, ID_2 = ACLK/2 while ID_0 = CLK, clear TBR with TBCLR*/
	TBCCR0 = (8192);	/*Set timer to count to this value = */
	TBCCTL0 = CCIE;	/*Enable CCR0 interrupt*/
	TBCTL |= MC_1;	/*Set timer to 'up' count mode*/
}

void AC2PC_Interpret(void) {
	extern char *getPC_ptr, *putPC_ptr;
	extern bool Prompt_Active, put_status_PC;

	switch((*getPC_ptr) - '0') { // Hackish way to tell MCU that *getPC_ptr is a number.
		case PROMPT_EXIT:
			/** @todo Implement exiting the prompt. */
			Prompt_Active = FALSE;
			putPC_ptr = &RS232NotActive[0];
			put_status_PC = TRUE;
			break;
		case PROMPT_BATT_DUMP:
			/** @todo Dump battery stats. */
			break;
		case PROMPT_SHUNT_DUMP:
			/** @todo Dump shunt stats. */
			break;
		case PROMPT_MPPT_DUMP:
			/** @todo Dump MPPT stats. */
			break;
		case PROMPT_THERM_DUMP:
			/** @todo Dump thermistor temperatures. */
			break;
		case PROMPT_MPPT_SWITCH_DUMP:
			/** @todo Dump status of MPPT switches. */
			break;
		default:
			/** @todo Display an error to the user. Maybe error light? */
			break;
	}
	UCA0TXBUF = *putPC_ptr++;
	UCA0IE |= UCTXIE;
}

/*
 * Interrupts
 */
/*
* BPS2PC Interrupt Service Routine
*/
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
	extern char *putPC_ptr, *getPC_ptr;
	extern bool put_status_PC, get_status_PC, Prompt_Active;
	char ch;

    switch(__even_in_range(UCA0IV,16))
    {
    case 0:		                              // Vector 0 - no interrupt
    	break;
    case 2:                                   // Data Received - UCRXIFG
    	ch = UCA0RXBUF;

    	if (ch == 0x0D && Prompt_Active == FALSE) { // Activate prompt.
    		Prompt_Active = TRUE;
    		putPC_ptr = &RS232Active[0];
    		UCA0TXBUF = *putPC_ptr++;
    		UCA0IE |= UCTXIE;
    	} else if (Prompt_Active && put_status_PC == FALSE) { // Prevent too many commands coming in at once.
    		getPC_ptr = &ch;
    		AC2PC_Interpret(); // Interpret command.
    	} else {
    		// Toggle error light?
    	}
		break;
    case 4:                                   // TX Buffer Empty - UCTXIFG
    	ch = *putPC_ptr++;

    	if (ch == '\0') {
    		UCA0IE &= ~UCTXIE;
    		put_status_PC = FALSE;
    	} else {
    		UCA0TXBUF = ch;
    	}
      break;
    default:
      break;
    }
}


/*
* Timer A interrupts
*/
#pragma vector = TIMER0_A0_VECTOR
 __interrupt void TIMERA_ISR(void)
{
	 P1OUT ^= LED0;
	 TA0CCTL0 &= 0xFFFE;	/*Clear Flags*/
}

 /*
 * Timer B interrupts
 * interrupt(TIMERB0_VECTOR) timer_b0(void)
 */
 #pragma vector = TIMERB0_VECTOR
 __interrupt void timer_b0(void)
 {
	 P1OUT ^= LED1;
 }

