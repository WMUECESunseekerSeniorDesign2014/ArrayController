/**
 * @brief A set of functions to perform testing of the various components within
 * the Array Controller.
 * @header ../inc/test_suite.h
 * @author Joel Dewey <joel.s.dewey@wmich.edu>
 */

#include <stdio.h>
#include "../inc/main.h"
#include "../inc/test/test_suite.h"

/**
 * A wrapper for __delay_cycles() which allows the developer to create a **synchronous** delay
 * of a number in microseconds.
 *
 * @note This should not be used in conjunction with functions that rely on ISRs!
 *
 * @param[in] delayConstant One of the constants defined in Delay Timings.
 */
static void Delay(unsigned int delayConstant) {
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

/**
 * A simple test blinking the LED multiple times.
 */
static void Blinky() {
	unsigned int i;
	unsigned int compareVal = 65535;

	P1DIR |= BIT0;
	P2DIR |= BIT0;
	P4DIR |= BIT0 | BIT1 | BIT2 | BIT3;

	for (i = 0; i < compareVal; i++) {
		P1OUT ^= BIT0;
		P2OUT ^= BIT0;
		P4OUT ^= BIT0 | BIT1 | BIT2 | BIT3;
	}
}

/**
 * Read and write to the PC using the RS-232 controller on the
 * Array Controller.
 */
static void Rs232_PC() {
	/// @todo Implement.
}

/**
 * Read values from the ADC and print them to the console.
 */
static void Adc() {
	/// @todo Implement.
}

/**
 * Enable the MPPTs through CAN.
 */
static void CAN_MPPT_Enable() {
	/// @todo Implement.
}

/**
 * Disable the MPPTs through CAN.
 */
static void CAN_MPPT_Disable() {
	/// @todo Implement.
}

/**
 * Retrieve data from the MPPTs using CAN.
 */
static void CAN_MPPT_Data() {
	/// @todo Implement.
}

/**
 * Read data from the solar car's main CAN bus.
 */
static void CAN_Car_Read() {
	/// @todo Implement.
}

/**
 * Write data to the solar car's main CAN bus.
 */
static void CAN_Car_Write() {
	/// @todo Implement.
}

/**
 * The main function for executing tests for the Array Controller.
 *
 * @param[in] tests Describes each test via the use of the UnitTest structure.
 * @param[in] numOfTests The number of tests to be run.
 */
extern void ExecuteTests(UnitTest tests[], int numOfTests) {
	signed int i;

	for (i = 0; i < numOfTests; i++) {
		if (tests[i].PreDelay > DELAY_0) {
			Delay(tests[i].PreDelay);
		}

		switch(tests[i].Test) {
			case BLINKY:
				Blinky();
				break;
			case RS232_PC:
				Rs232_PC();
				break;
			case ADC:
				Adc();
				break;
			case CAN_MPPT_ON:
				CAN_MPPT_Enable();
				break;
			case CAN_MPPT_OFF:
				CAN_MPPT_Disable();
				break;
			case CAN_MPPT_DATA:
				CAN_MPPT_Data();
				break;
			case CAN_CAR_READ:
				CAN_Car_Read();
				break;
			case CAN_CAR_WRITE:
				CAN_Car_Write();
				break;
			default:
				printf("Unknown test with ID: %X", tests[i].Test);
				break;
		}

		if (tests[i].PostDelay == DELAY_FOREVER) {
			i--;
		} else if (tests[i].PostDelay > DELAY_0) {
			Delay(tests[i].PostDelay);
		}
	}
}
