#include "./inc/main.h"
#include "./inc/test/test_suite.h"

/**
 * @defgroup privRs232Vars Private Main RS-232 Variables
 */
volatile int RX_INT_count = 0;
char test_buffer_PC[60];
char *putPC_ptr, *getPC_ptr;
bool put_status_PC, get_status_PC;
bool AC2PC_RX_flag = FALSE;

/*Global Variables*/
volatile unsigned char adc_stat[8];
volatile signed long adc_voltage[8];

volatile unsigned char int_op_flag = 0x00;
volatile unsigned char adc_rdy_flag = 0x00;
volatile unsigned char dr_switch_flag = 0x00;

int i;


/**@}*/

/*
 * main.c
 */
int main(void) {

	WDTCTL = WDTPW | WDTHOLD; /*Stop watchdog timer to prevent time out reset*/

    _DINT();	/*Disable interrupts*/

    /*Initialize global variables to 0*/
    for(i=0;i<=8;i++) adc_voltage[i]=0;
    for(i=0;i<=8;i++) adc_stat[i]=0;

    /*Initializations*/
    io_init();
    clock_init();
    //timerA_init();
    //timerB_init();

    P1OUT |= LED0;	/*Initialization 1 passed*/

    adc_spi_init();	/*Setup tranmission to ADC*/
    adc_init();	/*Initialize ADC*/
	adc_selfcal();	/*Run a selfcal on all channels*/
	adc_read_convert(0);

    P1OUT |= LED1;	/*Initialization 2 passed*/

    canspi_init();
    //can_init_MPPT();
    can_init_MAIN();

    //P4OUT |= ~(LED2);	/*Initialization 3 passed*/
    P4OUT |= LED2;	/*Initialization 3 passed*/

   //RS-232 Init function calls go here

   // P4OUT |= ~(LED3);	/*Initialization 4 passed*/
    P4OUT |= LED3;	/*Initialization 4 passed*/

    /*Parallel Pin Interrupts*/
    P1IFG &= ~(BUTTON1 | BUTTON2 | CAN_INTn0 | CAN_INTn1);

	//WDTCTL = WDT_ARST_1000; /*Stop watchdog timer to prevent time out reset*/
    _EINT();	/*Enable interrupts*/

//	// Transmission off data to the MPPT
//    can_MAIN.address = AC_CAN_BASE2;
//	can_MAIN.data.data_u16[3] = 0x0000; // to base address 0x600
//	can_MAIN.data.data_u16[2] = 0x0000;
//	can_MAIN.data.data_u16[1] = 0x0000;
//	can_MAIN.data.data_u16[0] = 0x0000; //Disable the MPPT
//	can_transmit_MAIN();

	// Transmission on data to the MPPT
//	can_MAIN.address = AC_CAN_BASE2;
//	can_MAIN.data.data_u16[3] = 0x0000; // to base address 0x600
//	can_MAIN.data.data_u16[2] = 0x0000;
//	can_MAIN.data.data_u16[1] = 0x0000;
//	can_MAIN.data.data_u16[0] = 0x0001; //Enable the MPPT
//	can_transmit_MAIN();
//
//	adc_voltage[0] = adc_in(0); /*Read in ADC channel reading*/
//	adc_voltage[1] = adc_in(1); /*Read in ADC channel reading*/
//	adc_voltage[2] = adc_in(2); /*Read in ADC channel reading*/
//	adc_voltage[3] = adc_in(3); /*Read in ADC channel reading*/
//	adc_voltage[4] = adc_in(4); /*Read in ADC channel reading*/
//	adc_voltage[5] = adc_in(5); /*Read in ADC channel reading*/
//	adc_voltage[6] = adc_in(6); /*Read in ADC channel reading*/
//	adc_voltage[7] = adc_in(7); /*Read in ADC channel reading*/

    while(1)
    {
    	 	 /*Check for CAN packet reception on CAN_MAIN (Polling)*/
    		 if((P1IN & CAN_INTn1) == 0x00)
    		 {
    		  	// IRQ flag is set, so run the receive routine to either get the message, or the error
    			//P6OUT ^= 0x08;
    			can_receive_MAIN();
    			// Check the status
    			// Modification: case based updating of actual current and velocity added
    			// - messages received at 5 times per second 16/(2*5) = 1.6 sec smoothing
    			if(can_MAIN.status == CAN_OK)
    			{
    				P4OUT ^= LED2;
    			}
    			if(can_MAIN.status == CAN_RTR)
    			{
    				//do nothing
    			}
    			if(can_MAIN.status == CAN_ERROR)
    			{
    				P4OUT ^= LED3;
    			}

    		  }

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
//
//    		  /*Driver Switch 1 Interrupt Received*/
//    		  if((P2IN & DRIVER_SW1) != DRIVER_SW1)
//    		  {
//    		       dr_switch_flag &= 0x08;
//    		       P4OUT ^= LED2;
//
//    		  }
//
//    		  /*Driver Switch 2 Interrupt Received*/
//    		  if((P2IN & DRIVER_SW2) != DRIVER_SW2)
//    		  {
//    		       dr_switch_flag &= 0x01;
//    		       P4OUT ^= LED3;
//    		  }
//
//    		  /*Driver Switch 3 Interrupt Received*/
//    		  if((P2IN & DRIVER_SW3) != DRIVER_SW3)
//    		  {
//    		       dr_switch_flag &= 0x04;
//    		       P4OUT ^= LED4;
//    		  }

    }
	
	//return 0;
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
    P5DIR = XT2OUT | CAN_RSTn2 | CAN_CSn2  | CAN_SI2 | P5_UNUSED;	/*Setup output pins */
    //P5OUT = CAN_RSTn2;	/*Send a reset signal*/
    P5OUT &= ~CAN_RSTn2;
    Delay(DELAY_100);
    Delay(DELAY_100);
    P5OUT |= CAN_RSTn2 ;
    P5OUT &= ~P5_UNUSED;
    Delay(DELAY_100);
    P5OUT |= CAN_CSn2 | CAN_SI2;
    P5SEL = XT2IN | XT2OUT | CAN_SI2 | CAN_SO2;	/*Setup pins for secondary function*/
    Delay(DELAY_100);\

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
    //P10OUT = CAN_RSTn;	/*Send a reset signal*/
    P10OUT &= ~CAN_RSTn;
    Delay(DELAY_100);
    Delay(DELAY_100);
    P10OUT |= CAN_RSTn;
    Delay(DELAY_100);
    P10OUT &= ~P10_UNUSED;
    P10OUT |= CAN_CSn | CAN_SI | CAN_SCLK;
	P10SEL = CAN_SCLK | CAN_SI | CAN_SO;
    Delay(DELAY_100);

    /*Port 11 Initialization*/
 	P11OUT = 0x00;	/*Set outputs to ground*/
 	P11DIR = P11_UNUSED;	/*Setup output pins*/

 	/*Port J Initialization*/
	PJOUT = 0x00;
	PJDIR = 0x0F;	/*set to output as per user's guide*/

    P1OUT = ~(LED0 | LED1);	/*Turn off LEDs*/
    P4OUT = LED2 | LED3 | LED4 | LED5;	/*Turn off LEDs*/
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
	TBCTL = CNTL_0 | TBSSEL_2 | ID_3 | TBCLR;	/*TBSSEL_2 = SMCLK, ID_3 = SMCLK/8, clear TBR*/
	TBCCR0 = (65000);	/*Set timer to count to this value = */
	TBCCTL0 = CCIE;	/*Enable CCR0 interrupt*/
	TBCTL |= MC_1;	/*Set timer to 'up' count mode*/

	/*Assign ACLK to Timer B with prescaler /2*/
//	TBCTL = CNTL_0 | TBSSEL_1 | ID_1 | TBCLR;	/*TBSSEL_1 = ACLK, ID_2 = ACLK/2 while ID_0 = CLK, clear TBR with TBCLR*/
//	TBCCR0 = (8192);	/*Set timer to count to this value = */
//	TBCCTL0 = CCIE;	/*Enable CCR0 interrupt*/
//	TBCTL |= MC_1;	/*Set timer to 'up' count mode*/
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

