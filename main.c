#include "./inc/main.h"

/**
 * @defgroup mainProto Private Main Function Prototypes
 */
static void InitController(void);
static void IdleController(void);
static void GeneralOperation(void);
static void ChargeOnly(void);
static void HumanInterruptCheck(void);
static void ToggleMPPT(unsigned int mppt, FunctionalState state);
static int GetMPPTData(unsigned int mppt);
static void ToggleError(bool toggle);
static void CoulombCount(void);
static void ReportCoulombCount(void);
static int ConvertADCVal(ADCState state);
/**@}*/

CarState carState = INIT; // The state that the car is in.
ADCState adcState = AIN3; // The state determining what the ADC should be converting at this moment.

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
bool coulomb_count_flag = FALSE;
bool data_dump_flag = FALSE;
bool error_blink_flag = FALSE;

char mppt_status = 0; // Bits 0-2 indicate if the MPPT is enabled or disabled.
char mppt_control = 0x0F; // Bits 0-2 indicate if we are intelligently controlling the MPPTs.

// Every time timA_cnt hits 512, timA_total_cnt needs to be incremented.
short timA_cnt = 0;
unsigned long timA_total_cnt = 0;

signed long tempOne, tempTwo, tempThree, refTemp, adcRef, internal12V;
unsigned long coulombCnt = 0;
unsigned long shuntCurrent = 0;
unsigned int battVoltage = 0;
unsigned long power = 0;
unsigned long powerAvg = 0;

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
    		if(int_enable_flag == FALSE) {
				_EINT(); // Enable interrupts
				int_enable_flag = TRUE;
			}
			P4OUT &= ~(LED4 | LED5); // Turn on two LEDs to show we're idling.
			IdleController();
    		break;

    	case RUNNING:
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
}

/**
 * Enable or disable a MPPT.
 */
static void ToggleMPPT(unsigned int mppt, FunctionalState state) {
	can_MPPT.address = AC_CAN_BASE2 + mppt;

	if(state == ON) {
		can_MPPT.data.data_u16[0] = 0x0001;
	} else {
		can_MPPT.data.data_u16[0] = 0x0000;
	}

	can_MPPT.data.data_u16[3] = 0x0000; // to base address 0x600
	can_MPPT.data.data_u16[2] = 0x0000;
	can_MPPT.data.data_u16[1] = 0x0000;

	// Only transmit if needed to prevent hanging up the microcontroller with sending an
	// unnecessary CAN message. This was broken into multiple IF statements for readability.
	if(state == ON) { // The MPPT is OFF and we want to turn it on.
		if((mppt_status & 0x01) == 0) {
			can_transmit_MPPT();
		} else if((mppt_status & 0x02) == 0) {
			can_transmit_MPPT();
		} else if((mppt_status & 0x04) == 0) {
			can_transmit_MPPT();
		}
	} else { // The MPPT is ON and we want to turn it off.
		if((mppt_status & 0x01) > 0) {
			can_transmit_MPPT();
		} else if((mppt_status & 0x02) > 0) {
			can_transmit_MPPT();
		} else if((mppt_status & 0x04) > 0) {
			can_transmit_MPPT();
		}
	}

	// Keep track of the status of each MPPT.
	switch(mppt) {
		case MPPT_ZERO:
			mppt_status = (state == ON) ? mppt_status | 0x01 : mppt_status & ~(0x01);
			break;
		case MPPT_ONE:
			can_transmit_MPPT();
			mppt_status = (state == ON) ? mppt_status | 0x02 : mppt_status & ~(0x02);
			break;
		case MPPT_TWO:
			can_transmit_MPPT();
			mppt_status = (state == ON) ? mppt_status | 0x04 : mppt_status & ~(0x04);
			break;
		default:
			break;
	}
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
		case CAN_OK:
		case CAN_RTR:
			ToggleError(FALSE);
			return 1;
		case CAN_ERROR:
			ToggleError(TRUE);
		default:
			return 0;
	}
}

/**
 * GeneralOperation() is where the Array Controller will be most of the time. In this function, the
 * ArrayController will:
 *  * Calculate the state-of-charge (coulomb count).
 *  * Dump telemetry data out to the CAN bus.
 *  * Poll the driver switches to see if the driver is requesting that the MPPTs turn off.
 *  * Poll the thermistors and, if needed, send an emergency CAN message.
 */
static void GeneralOperation(void) {
	// TIMA has gone off.
	if(coulomb_count_flag == TRUE) {
		CoulombCount();
		coulomb_count_flag = FALSE;
	}

	// One second has passed.
	if(data_dump_flag == TRUE) {
		ReportCoulombCount();
		data_dump_flag = FALSE;
	}

	// Enable/disable MPPTs based on driver switch status.
	// The first MPPT.
	if((dr_switch_flag & 0x08) > 0) { // Switch is on.
		mppt_control |= ~(0xFE);
	} else {
		// The driver has ultimate control over turning off the MPPT.
		ToggleMPPT(MPPT_ZERO, OFF);
		mppt_control &= 0xFE;
	}

	// The second MPPT.
	if((dr_switch_flag & 0x01) > 0) { // Switch is on.
		mppt_control &= ~(0xFD);
	} else {
		ToggleMPPT(MPPT_ONE, OFF);
		mppt_control &= 0xFD;
	}

	// The third MPPT.
	if((dr_switch_flag & 0x04) > 0) { // Switch is on.
		mppt_control &= ~(0xFB);
	} else {
		ToggleMPPT(MPPT_TWO, OFF);
		mppt_control &= 0xFB;
	}

	// Calculate the percentage of deliverable power left in the batteries. powerAvg is divided by 3600
	// to convert it from watt-seconds to watt-hours.
	battPercentage = ((BATT_MAX_WATTH - (powerAvg / 3600)) / BATT_MAX_WATTH) * 100; // Convert to a percentage.

	// Enable/disable the MPPTs based on the percentage that was calculated AND if the driver is allowing
	// us to control the MPPTs.
	if(battPercentage <= BATT_HIGH_LOWER && (mppt_control & 0x01) > 0) {
		ToggleMPPT(MPPT_ZERO, ON);
	} else if(battPercentage >= BATT_HIGH_UPPER) {
		ToggleMPPT(MPPT_ZERO, OFF);
	}

	if(battPercentage <= BATT_MEDI && (mppt_control & 0x02) > 0) {
		ToggleMPPT(MPPT_ONE, ON);
	} else {
		ToggleMPPT(MPPT_ONE, OFF);
	}

	if(battPercentage <= BATT_LOW && (mppt_control & 0x04) > 0) {
		ToggleMPPT(MPPT_TWO, ON);
	} else {
		ToggleMPPT(MPPT_TWO, OFF);
	}

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
			/** @todo Figure out what CAN messages we need to care about. */
	   }
	   if(can_MPPT.status == CAN_RTR)
	   {
			//do nothing
	   }
	   if(can_MPPT.status == CAN_ERROR)
	   {
			ToggleError(TRUE);
	   }
	 }

	switch(adcState) {
		case AIN0:
			tempOne = ConvertADCVal(adcState);
			adcState = AIN1;
			break;
		case AIN1:
			tempTwo = ConvertADCVal(adcState);
			adcState = AIN2;
			break;
		case AIN2:
			tempThree = ConvertADCVal(adcState);
			adcState = AIN3;
			break;
		case AIN3:
			refTemp = ConvertADCVal(adcState);
			adcState = REF;
			break;
		case REF:
			adcRef = ConvertADCVal(adcState);
			adcState = INT12V;
			break;
		case INT12V:
			internal12V = ConvertADCVal(adcState);
			adcState = AIN0;
			break;
	}
}

/**
 * ChargeOnly() is where the Array Controller will simply manage charging the batteries. It will
 * perform the following operations:
 *  * Calculate the state-of-charge (coulomb count).
 *  * Disable the MPPTs as the batteries fill.
 * @note Max voltage of the battery array is 160V.
 * @note This is unfinished and untested; Dr. Bazuin has requested we leave this state alone as
 * 		 Sunseeker currently doesn't have a way to indicate when the Array Controller should
 * 		 switch to CHARGING.
 */
static void ChargeOnly(void) {
	signed int battPercentage = 0;

	if(coulomb_count_flag == TRUE) {
		CoulombCount();
		coulomb_count_flag = FALSE;
	}

	if(data_dump_flag == TRUE) {
		// Calculate the percentage of deliverable power left in the batteries. powerAvg is divided by 3600
		// to convert it from watt-seconds to watt-hours.
		battPercentage = ((BATT_MAX_WATTH - (powerAvg / 3600)) / BATT_MAX_WATTH) * 100; // Convert to a percentage.

		// Enable/disable the MPPTs based on the percentage that was calculated.
		if(battPercentage <= BATT_HIGH_LOWER) {
			ToggleMPPT(MPPT_ZERO, ON);
		} else if(battPercentage >= BATT_HIGH_UPPER) {
			ToggleMPPT(MPPT_ZERO, OFF);
		}

		if(battPercentage <= BATT_MEDI) {
			ToggleMPPT(MPPT_ONE, ON);
		} else {
			ToggleMPPT(MPPT_ONE, OFF);
		}

		if(battPercentage <= BATT_LOW) {
			ToggleMPPT(MPPT_TWO, ON);
		} else {
			ToggleMPPT(MPPT_TWO, OFF);
		}

		ReportCoulombCount();
	}
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
	char buffer[80];
	bool error_flag = FALSE;

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
		// Dump data via RS-232 and CAN every second.
		if(data_dump_flag == TRUE) {
			AC2PC_puts("BLOWN FUSE\r\n");
			sprintf(buffer, "0: %d V, 1: %d V, 2: %d V\r\n", battV[MPPT_ZERO], battV[MPPT_ONE], battV[MPPT_TWO]);
			AC2PC_puts(buffer);

			can_MAIN.address = AC_CAN_MAIN_BASE + AC_BLOWN_FUSE; /** @todo Who do I send these messages to? */
			can_MAIN.data.data_u16[0] = battV[MPPT_ZERO];
			can_MAIN.data.data_u16[1] = battV[MPPT_ONE];
			can_MAIN.data.data_u16[2] = battV[MPPT_TWO];
			can_MAIN.data.data_u16[3] = 0; // No use for this yet.
			can_transmit_MAIN();

			error_flag = TRUE;
			data_dump_flag = FALSE;
		}
	}

	// Broke this into multiple if statements for readability. This verifies that the
	// voltages the MPPTs read from the batteries are different and they're not within
	// the maximum voltage range. If the inner most if statement is reached, then the
	// array is not connected to the rest of the car!
	if(battV[MPPT_ZERO] == battV[MPPT_ONE]) {
		if(battV[MPPT_ONE] == battV[MPPT_TWO]) {
			if((battV[MPPT_ZERO] >= BATT_MAX_LOWER_V) && (battV[MPPT_ZERO] <= BATT_MAX_UPPER_V)) {
				// Dump data via RS-232 and CAN every second.
				if(data_dump_flag == TRUE) {
					AC2PC_puts("ARRAY UNCONNECTED\r\n");
					sprintf(buffer, "0: %d V, 1: %d V, 2: %d V\r\n", battV[MPPT_ZERO], battV[MPPT_ONE], battV[MPPT_TWO]);
					AC2PC_puts(buffer);

					can_MAIN.address = AC_CAN_MAIN_BASE + AC_ARR_CABLE; /** @todo Who do I send these messages to? */
					can_MAIN.data.data_u16[0] = battV[MPPT_ZERO];
					can_MAIN.data.data_u16[1] = battV[MPPT_ONE];
					can_MAIN.data.data_u16[2] = battV[MPPT_TWO];
					can_MAIN.data.data_u16[3] = 0; // No use for this yet.
					can_transmit_MAIN();

					error_flag = TRUE;
					data_dump_flag = FALSE;
				}
			}
		}
	}

	// Poll CAN interrupt to see if we have received the 504 message.
	if((P4IN & (CAN_RX1n2 | CAN_RX2n2)) == 0x00) {
		can_receive_MAIN();

		if(can_MAIN.status == CAN_OK) {
			switch(can_MAIN.address) {
				case DC_CAN_BASE + DC_SWITCH:
					dc_504_flag = TRUE;
					/** @todo Do we need to do anything with the data sent with 504? */
					break;
				default:
					// Do nothing because we don't know what the message is.
					break;
			}
		} else if(can_MAIN.status == CAN_RTR) {
			can_MAIN.address = AC_CAN_MAIN_BASE; // Send out battery stats since we're still idling
												 // and no coulomb count has occurred.
			can_MAIN.data.data_u16[0] = battV[MPPT_ZERO];
			can_MAIN.data.data_u16[1] = battV[MPPT_ONE];
			can_MAIN.data.data_u16[2] = battV[MPPT_TWO];
			can_MAIN.data.data_u16[3] = 0; // No use for this yet.
			can_transmit_MAIN();
		} else {
			error_flag = TRUE;
		}
	}

	// dc_504_flag should get set in the Main CAN controller interrupt.
	if(error_flag == FALSE && dc_504_flag == TRUE) {
		carState = RUNNING;
		P4OUT |= (LED4 | LED5);
		// Reset the counters for the next state.
		timA_cnt = 0;
		timA_total_cnt = 0;
		battVoltage = battV[0]; // All battery voltages should be about the same.
	}

	ToggleError(error_flag);
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
	ToggleError(FALSE);
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
	for(i = 0; i <= MPPT_TWO; i++) { ToggleMPPT(i, OFF); } // Disable MPPTs initially.
	P4OUT &= ~LED5; // 0 1 0 1

	/* Initialize RS-232 */
	// No interrupt has come through yet, so mark this to FALSE initially.
	put_status_PC = FALSE;
	Prompt_Active = FALSE;
	AC2PC_init();
	UCA0IE |= UCRXIE; // Enable interrupts on the RX line.
	P4OUT &= ~(LED3 | LED4); // 0 1 1 0
	P4OUT |= (LED5);

	/* GPIO Interrupts */
	P2IE = DRIVER_SW1 | DRIVER_SW2 | DRIVER_SW3;
	// Driver switches interrupt on low-to-high transitions.
	P2IES = ~(DRIVER_SW1 | DRIVER_SW2 | DRIVER_SW3);
	P4OUT &= ~(LED5); // 0 1 1 1

	// Set up the LEDs for the next state.
	P4OUT |= LED2 | LED3 | LED4 | LED5;
}

/**
 * The function which performs coulomb counting. This function:
 *  1. Reads the shunt voltage.
 *  2. Reads the bias voltage.
 *  3. Subtracts the bias from the shunt voltage.
 *  4. Converts the value from (3) into a voltage.
 *  5. Converts the value (4) into a current and stores it in a global.
 *  5. Computes the time average current and stores it in another global.
 */
void CoulombCount(void) {
	unsigned long shuntVal = 0;

	// Convert the value read from the shunt back into a voltage.
	shuntVal = ((adc_in((char)SHUNT) - adc_in((char)SHUNT_BIAS)) * ADC_REF) / ADC_RESO;
	shuntCurrent /= SHUNT_OHM; // Get the current.
	coulombCnt += (shuntCurrent - coulombCnt) >> C_CNT_SHIFT;
}

/**
 * Report the results of the coulomb count to the CAN bus.
 */
void ReportCoulombCount(void) {
	power = battVoltage * shuntCurrent;
	powerAvg = battVoltage * coulombCnt;

	// Transmit MPPT status.
	can_MAIN.address = AC_CAN_MAIN_BASE + AC_MPPT_STATUS;
	can_MAIN.data.data_u8[0] = mppt_status;
	can_transmit_MAIN();

	// Transmit currents.
	can_MAIN.address = AC_CAN_MAIN_BASE + AC_CURRENT;
	can_MAIN.data.data_u32[0] = shuntCurrent;
	can_MAIN.data.data_u32[1] = coulombCnt;
	can_transmit_MAIN();

	// Transmit powers.
	can_MAIN.address = AC_CAN_MAIN_BASE + AC_POWER;
	can_MAIN.data.data_u32[0] = power;
	can_MAIN.data.data_u32[1] = powerAvg;
	can_transmit_MAIN();
}

/**
 * Converts a ratiometric value from the ADC into a voltage.
 */
int ConvertADCVal(ADCState state) {
	signed long adcVal = adc_in((char)state);

	return (int)((adcVal * ADC_REF) / ADC_RESO);
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
			for(i = 0; i < 2000; i++) { __delay_cycles(DELAY_HALFSEC); } // 2000 iterations will give a half second delay.
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
void ToggleError(bool toggle) {
	error_blink_flag = toggle;
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
 TA0CCR0 = 127; // Interrupt every 1/512 a second.
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
			Prompt_Active = FALSE;
			sprintf(tx_PC_buffer, "Exiting prompt.\r\n");
			put_status_PC = TRUE;
			break;
		case PROMPT_BATT_DUMP:
			/** @todo Dump battery stats. */
			break;
		case PROMPT_SHUNT_DUMP:
			sprintf(tx_PC_buffer, "I: %d, AvgI: %d, P: %d, AvgP: %d\r\n", shuntCurrent, coulombCnt, power, powerAvg);
			break;
		case PROMPT_MPPT_DUMP:
			/** @todo Dump MPPT stats. */
			break;
		case PROMPT_THERM_DUMP:
			sprintf(tx_PC_buffer, "T1: %lX, T2: %lX, T3: %lX, R: %lX\r\n", tempOne, tempTwo, tempThree, refTemp);
			break;
		case PROMPT_MPPT_STATUS_DUMP:
			sprintf(tx_PC_buffer, "MPPT: %X\r\n", mppt_status);
			break;
		case PROMPT_SELF_CHECK:
			sprintf(tx_PC_buffer, "12V: %lX, REF: %lX", internal12V, adcRef);
			break;
		default:
			// Do nothing.
			break;
	}
	putPC_ptr = &tx_PC_buffer;
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
  char inputStatus = 0;

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
	inputStatus = P4IN & DRIVER_SW1;

	if(inputStatus == 0) { // Driver switch is off.
		dr_switch_flag &= ~(0x08);
		P2IES &= ~(DRIVER_SW1); // Require a low-to-high transition.
	} else { // Driver switch is on.
		dr_switch_flag |= 0x08;
		P2IES |= DRIVER_SW1; // Require a high-to-low transition.
	}
    break;
  case 10:                                  // Vector 2.4 - DRIVER_SW2
	inputStatus = P4IN & DRIVER_SW2;

	if(inputStatus == 0) { // Driver switch is off.
		dr_switch_flag &= ~(0x01);
		P2IES &= ~(DRIVER_SW2); // Require a low-to-high transition.
	} else { // Driver switch is on.
		dr_switch_flag |= 0x01;
		P2IES |= DRIVER_SW2; // Require a high-to-low transition.
	}
    break;
  case 12:                                  // Vector 2.5 - DRIVER_SW3
	inputStatus = P4IN & DRIVER_SW3;

	if(inputStatus == 0) { // Driver switch is off.
		dr_switch_flag &= ~(0x04);
		P2IES &= ~(DRIVER_SW3); // Require a low-to-high transition.
	} else { // Driver switch is on.
		dr_switch_flag |= 0x04;
		P2IES |= DRIVER_SW3; // Require a high-to-low transition.
	}
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
	 coulomb_count_flag = TRUE;
	 if(++timA_cnt == TIMA_ONE_SEC + 1) { // If timA_cnt is equal to 513, roll over.
		 timA_cnt = 1;
		 data_dump_flag = TRUE;

		 if(error_blink_flag == TRUE) {
			 P1OUT ^= ~LED1; // Blink the LED.
		 } else {
			 P1OUT |= LED1; // Disable the LED.
		 }

		 if(++timA_total_cnt == ULONG_MAX) { // This should never happen.
			 timA_total_cnt = 0;
		 }
	 }
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

