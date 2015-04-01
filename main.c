#include "./inc/main.h"

/**
 * @defgroup mainProto Private Main Function Prototypes
 */
static void InitController(void);
static void GeneralOperation(void);
static void ChargeOnly(void);
static void HumanInterruptCheck(void);
static void ToggleMPPT(unsigned int mppt, MPPTState state);
static int GetMPPTData(unsigned int mppt);
void ToggleError(FunctionalState toggle);
/**@}*/

CarState carState = INIT; // The state that the car is in.
ADCState adcState = AIN0; // The state determining what the ADC should be converting at this moment.

/**
 * @defgroup privRs232Vars Private Main RS-232 Variables
 */
volatile int RX_INT_count = 0;
char tx_PC_buffer[60];
char rx_PC_buffer[60];
char *putPC_ptr, *getPC_ptr;
bool put_status_PC, get_status_PC;
bool Prompt_Active = FALSE;
bool AC2PC_RX_flag = FALSE;
/**@}*/

/*Global Variables*/
volatile unsigned char adc_stat[8] = { 0 };
volatile signed long adc_voltage[8] = { 0 };

volatile unsigned char int_op_flag = 0x00;
volatile unsigned char adc_rdy_flag = 0x00;
volatile unsigned char dr_switch_flag = 0x00;
bool int_enable_flag = FALSE;
bool dc_504_flag = FALSE;

int i;


/*
 * main.c
 */
int main(void) {
	WDTCTL = WDTPW | WDTHOLD; /*Stop watchdog timer to prevent time out reset*/

    while(1) {
    	HumanInterruptCheck();

    	switch(carState) {
    	case INIT:
    		_DINT(); // Disable interrupts
			InitController();
    		carState = IDLE;
    		break;

    	case IDLE:
			P4OUT &= ~(LED4 | LED5); // Turn on two LEDs to show we're idling.
			/** @todo Poll for the 504 message. This should be added during system testing. */
    		break;

    	case RUNNING:
    		if(int_enable_flag == FALSE) {
    			_EINT(); // Enable interrupts
    			int_enable_flag = TRUE;
    		}
    		GeneralOperation();
    		break;

    	case CHARGING:
    		ChargeOnly();
    		break;
    	}
    }
	
	return 0;
}

static void HumanInterruptCheck(void) {
	/*BUTTON1 Press Received*/
	if(((P1IN & BUTTON1) != BUTTON1) || ((int_op_flag & 0x08) == 0x08))
	{
		int_op_flag &= 0x08;
		P4OUT ^= LED4;

	}

	/*BUTTON2 Press Received*/
	if(((P1IN & BUTTON2) != BUTTON2) || ((int_op_flag & 0x04) == 0x04))
	{
		 int_op_flag &= 0x04;
		 P4OUT ^= LED5;
	}

	/*Driver Switch 1 Interrupt Received*/
	if((P2IN & DRIVER_SW1) != DRIVER_SW1)
	{
		 dr_switch_flag &= 0x08;
		 P4OUT ^= LED2;

	}

	/*Driver Switch 2 Interrupt Received*/
	if((P2IN & DRIVER_SW2) != DRIVER_SW2)
	{
		dr_switch_flag &= 0x01;
		P4OUT ^= LED3;
	}

	/*Driver Switch 3 Interrupt Received*/
	if((P2IN & DRIVER_SW3) != DRIVER_SW3)
	{
		dr_switch_flag &= 0x04;
		P4OUT ^= LED4;
	}
}

/**
 * Enable or disable a MPPT.
 */
static void ToggleMPPT(unsigned int mppt, MPPTState state) {
	can_MPPT.address = AC_CAN_BASE2 + mppt;

	if(state == MPPT_ON) {
		can_MPPT.data.data_u16[0] = 0x0001;
	} else {
		can_MPPT.data.data_u16[0] = 0x0000;
	}

	can_MPPT.data.data_u16[3] = 0x0000; // to base address 0x600
	can_MPPT.data.data_u16[2] = 0x0000;
	can_MPPT.data.data_u16[1] = 0x0000;

	can_transmit_MPPT();
}

/**
 * Get a data dump from the MPPTs.
 * @todo Austin is still testing this, so it may change later.
 */
static int GetMPPTData(unsigned int mppt) {
	can_MPPT.address = AC_CAN_BASE1 + mppt;
	can_sendRTR(0); //Send RTR request
	can_transmit_MPPT();
	can_sendRTR(1);

	// Wait until the response is sent from the MPPT, and then read it.
	if((P1IN & CAN_INTn0) == 0x00) {
		can_receive_MPPT();
	}

	switch(can_MPPT.status) {
	case CAN_ERROR:
		ToggleError(ON);
		return 0;
	case CAN_OK:
	case CAN_RTR:
		ToggleError(OFF);
		return 1;
	}
}

/**
 * GeneralOperation() is where the Array Controller will be most of the time. In this function, the
 * ArrayController will:
 *  * Calculate the state-of-charge (coulomb count).
 *  * Dump telemetry data out to the CAN bus.
 *  * Make decisions based on the state-of-charge to enable/disable the MPPTs.
 *  * Poll the driver switches to see if the driver is requesting that the MPPTs turn off.
 *  * Poll the thermistors and, if needed, send an emergency CAN message.
 */
static void GeneralOperation(void) {
	/*Check for CAN packet reception on CAN_MPPT (Polling)*/
	if((P1IN & CAN_INTn0) == 0x00)
	{
	   //IRQ flag is set, so run the receive routine to either get the message, or the error
	   can_receive_MPPT();
	   // Check the status
	   // Modification: case based updating of actual current and velocity added
	   // - messages received at 5 times per second 16/(2*5) = 1.6 sec smoothing
	   if(can_MPPT.status == CAN_OK)
	   {
			P4OUT ^= LED2;
	   }
	   if(can_MPPT.status == CAN_RTR)
	   {
			//do nothing
	   }
	   if(can_MPPT.status == CAN_ERROR)
	   {
			P4OUT ^= LED3;
	   }
	 }
}

/**
 * ChargeOnly() is where the Array Controller will simply manage charging the batteries. It will
 * perform the following operations:
 *  * Calculate the state-of-charge (coulomb count).
 *  * Disable the MPPTs as the batteries fill.
 * @note Max voltage of the battery array is 160V.
 */
static void ChargeOnly(void) {

}

/**
 * IdleController() is where the Array Controller will sit while waiting for the 504 message from
 * the Driver Controller. This is the point where the Array Controller will check its connections
 * through the information available. In this state, the following will be checked:
 *
 *  1. If the battery voltages are different, the one that is different has a blown fuse. This will
 *     be indicated through enabling the red LED (LED1) and dumping this data out on the RS-232 port.
 *  2. If the battery voltages are equivalent to the array voltages, everything is fine. LED1 is off.
 *  3. If the battery voltages are equivalent to each other BUT all three are sitting at the max
 *     voltage (160V), the array is not connect to the batteries. This will be shown by enabling the
 *     LED1 and dumping data to the RS-232 port.
 *
 * If either (1) or (3) occur and a 504 message is received, the Array Controller will acknowledge the
 * 504 message by setting dc_504_flag = TRUE but it will not move to the next state.
 */
static void IdleController(void) {
	int i;
	int status = 0;
	unsigned int arrayV[3];
	unsigned int battV[3];

	// Loop and get data from MPPT.
	for(i = 0; i <= MPPT_TWO; i++) {
		// If the previous call to GetMPPTData() resulted in an error, roll back i.
		if(status == 0) {
			i = (i <= 1) ? 0 : (i - 1);
		} else { // If the RTR was successful, store the data.
			// Change the data from mV to V for simple comparisons.
			arrayV[i] = can_MPPT.data.data_u16[0] / MPPT_AV_SCALE;
			battV[i] = can_MPPT.data.data_u16[2] / MPPT_BV_SCALE;
		}

		status = GetMPPTData(i);
	}

	// If the battery voltages as reported by the MPPTs are different, then a fuse
	// has blown. Note that if the first statement is true, the system will not check
	// the second; if the first statement is false, the system will check the second
	// statement and since battV[MPPT_ZERO] == battV[MPPT_ONE], a third comparison
	// is not needed.
	if((battV[MPPT_ZERO] != battV[MPPT_ONE]) || (battV[MPPT_ONE] != battV[MPPT_TWO])) {

	}

	// Broke this into multiple if statements for readability. This verifies that the
	// voltages the MPPTs read from the batteries are different and they're not within
	// the maximum voltage range. If the inner most if statement is reached, then the
	// array is not connected to the rest of the car!
	if(battV[MPPT_ZERO] == battV[MPPT_ONE]) {
		if(battV[MPPT_ONE] == battV[MPPT_TWO]) {
			if((battV[MPPT_ZERO] >= BATT_MAX_LOWER_V) && (battV[MPPT_ZERO] <= BATT_MAX_UPPER_V)) {

			}
		}
	}

}

/**
 * InitController() should initialize the controller to the point where it should be ready to aid the car
 * in its main operation. InitController should do the following:
 *  1. Initialize the vital components (CLK, IO, etc).
 *  2. Initialize the ADC.
 *  3. Run diagnostics on the ADC.
 *  4. Initialize CAN.
 *  5. Initialize RS-232
 */
static void InitController(void) {
	int i = 0;

	/* Vital Initializations */
	io_init();
	clock_init();
	timerA_init();
	timerB_init();
	// Turn off the error light.
	ToggleError(OFF);
	P4OUT |= LED2 | LED3 | LED4 | LED5; // Turn all of the LEDs off.
	P4OUT &= ~(LED5); // 0 0 0 1

	/* Initialize the ADC. */
	adc_spi_init();	/*Setup tranmission to ADC*/
	adc_init();	/*Initialize ADC*/
	adc_selfcal();	/*Run a selfcal on all channels*/
	adc_read_convert(0);
	P4OUT &= ~LED4;
	P4OUT |= LED5; // 0 0 1 0

	/* ADC Diagnostics */
	adc_selfcal(); // Calibrate all of the channels.
	P4OUT &= ~LED5; // 0 0 1 1

	/* Main CAN Initialization */
	canspi_init();
	can_init_MAIN();
	P4OUT &= ~LED3; // 0 1 0 0
	P4OUT |= (LED4 | LED5);

	/* MPPT CAN Initialization */
	can_init_MPPT();
	Delay(DELAY_HALFSEC); // Give the MPPTs time to initialize themselves.
	for(i = 0; i <= MPPT_TWO; i++) { ToggleMPPT(i, MPPT_OFF); } // Disable MPPTs initially.
	P4OUT &= ~LED5; // 0 1 0 1

	/* Initialize RS-232 */
	// No interrupt has come through yet, so mark this to FALSE initially.
	put_status_PC = FALSE;
	Prompt_Active = FALSE;
	AC2PC_init();
	UCA0IE |= UCRXIE; // Enable interrupts on the RX line.
	P4OUT &= ~(LED3 | LED4); // 0 1 1 0
	P4OUT |= (LED5);

	// Set up the LEDs for the next state.
	P4OUT |= LED2 | LED3 | LED4 | LED5;
}

/**
 * A wrapper for __delay_cycles() which allows the developer to create a **synchronous** delay
 * of a number in microseconds.
 *
 * @note This should not be used in conjunction with functions that rely on ISRs!
 *
 * @param[in] delayConstant One of the constants defined in Delay Timings.
 */
extern void Delay(unsigned long delayConstant) {
	int i;

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
		case DELAY_HALFSEC:
			for(i = 0; i < 2000; i++) { __delay_cycles(DELAY_HALFSEC); }
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

/**
 * Toggle the error LED on or off.
 */
void ToggleError(FunctionalState toggle) {
	if(toggle == ON) {
		P1OUT &= ~LED1;
	} else {
		P1OUT |= LED1;
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
    P3OUT = CAN_SCLK2;
    P3SEL = ADC_DIN | ADC_DOUT | ADC_SCLK | TX_EXT | RX_EXT | CAN_SCLK2;	/*Setup pins for secondary function*/
    Delay(DELAY_100);

    /*Port 4 Initialization*/
	P4OUT = 0x00;	/*Set outputs to ground*/
    P4DIR = LED2 | LED3 | LED4 | LED5 | P4_UNUSED;	/*Setup output pins*/
	P4OUT = ~(LED2 | LED3 | LED4 | LED5);	/*Turn on LEDs*/
	Delay(DELAY_100);

    /*Port 5 Initialization*/
	P5OUT = 0x00;	/*Set outputs to ground*/
    P5DIR =  XT2OUT | CAN_RSTn2 | CAN_CSn2  | CAN_SI2 | P5_UNUSED;	/*Setup output pins*/
    P5OUT &= ~CAN_RSTn2;
    Delay(DELAY_100);
    Delay(DELAY_100);
    P5OUT |= CAN_RSTn2;
    P5OUT &= ~P5_UNUSED;
    Delay(DELAY_100);
    P5OUT |= CAN_CSn2 | CAN_SI2;
    P5SEL = XT2IN | XT2OUT | CAN_SI2 | CAN_SO2;	/*Setup pins for secondary function  */
    Delay(DELAY_100);

    /*Port 6 Initialization*/
	P6OUT = 0x00;	/*Set outputs to ground*/
	P6DIR = P6_UNUSED;	/*Setup output pins*/

    /*Port 7 Initialization*/
	P7OUT = 0x00;	/*Set outputs to ground*/
	P7DIR = XT1OUT | P7_UNUSED;	/*Setup output pins */
	P7SEL = XT1IN | XT1OUT;	/*Setup pins for secondary function*/
//	Delay(DELAY_100);

    /*Port 8 Initialization*/
	P8OUT = 0x00;	/*Set outputs to ground*/
	P8DIR = P8_UNUSED;	/*Setup output pins*/

	/*Port 9 Initialization*/
	P9OUT = 0x00;	/*Set outputs to ground*/
	P9DIR = P9_UNUSED;	/*Setup output pins*/

	/*Port 10 Initialization*/
	P10OUT = 0x00;	/*Set outputs to ground*/
	P10DIR = CAN_SCLK | CAN_CSn | CAN_RSTn | CAN_SI | P10_UNUSED;	/*Setup output pins*/
    P10OUT &= ~CAN_RSTn;
    Delay(DELAY_100);
    Delay(DELAY_100);
	P10OUT |= CAN_RSTn;
    P10OUT &= ~P10_UNUSED;
    Delay(DELAY_100);
    P10OUT |= CAN_CSn | CAN_SI | CAN_SCLK;
	P10SEL = CAN_SCLK | CAN_SI | CAN_SO;
    Delay(DELAY_100);

    /*Port 11 Initialization*/
 	P11OUT = 0x00;	/*Set outputs to ground*/
 	P11DIR = P11_UNUSED;	/*Setup output pins*/
 	P11SEL = 0x07;

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
//	TBCTL = CNTL_0 | TBSSEL_2 | ID_3 | TBCLR;	/*TBSSEL_2 = SMCLK, ID_3 = SMCLK/8, clear TBR*/
//	TBCCR0 = (65000);	/*Set timer to count to this value = */
//	TBCCTL0 = CCIE;	/*Enable CCR0 interrupt*/
//	TBCTL |= MC_1;	/*Set timer to 'up' count mode*/

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
    	} else if (Prompt_Active == TRUE && put_status_PC == FALSE) { // Prevent too many commands coming in at once.
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
* Port 1 Pin Interrupt Service Routine
*/
	#pragma vector=PORT1_VECTOR
__interrupt void P1_ISR(void)
{
  switch(__even_in_range(P1IV,16))
  {
  case 0:
	break;                             		// Vector 0 - no interrupt
  case 2:                                   // Vector 1.0 -
    break;
  case 4:                                   // Vector 1.1 -
    break;
  case 6:                                   // Vector 1.2 - BUTTON1
    int_op_flag |= 0x08;
    break;
  case 8:                                   // Vector 1.3 - BUTTON2
    int_op_flag |= 0x04;
   break;
  case 10:                                  // Vector 1.4 - CAN_INTn0
    int_op_flag |= 0x80;
    break;
  case 12:                                  // Vector 1.5 - CAN_INTn1
    int_op_flag |= 0x40;
    break;
  case 14:                                  // Vector 1.6 -
    break;
  case 16:                                  // Vector 1.7 -
    break;
  default:
    break;
  }
}

/*
* Port 2 Pin Interrupt Service Routine
*/
	#pragma vector=PORT2_VECTOR
__interrupt void P2_ISR(void)
{
  switch(__even_in_range(P2IV,16))
  {
  case 0:
	break;                             		// Vector 0 - no interrupt
  case 2:                                   // Vector 2.0 -
    break;
  case 4:                                   // Vector 2.1 -
    break;
  case 6:                                   // Vector 2.2 - ADC_RDYn
    adc_rdy_flag |= 0x04;
    break;
  case 8:                                   // Vector 2.3 - DRIVER_SW1
	dr_switch_flag |= 0x08;
    break;
  case 10:                                  // Vector 2.4 - DRIVER_SW2
    dr_switch_flag |= 0x01;
    break;
  case 12:                                  // Vector 2.5 - DRIVER_SW3
	dr_switch_flag |= 0x04;
    break;
  case 14:                                  // Vector 2.6 -
    break;
  case 16:                                  // Vector 2.7 -
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

