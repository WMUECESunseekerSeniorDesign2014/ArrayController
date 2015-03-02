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
 * A simple test blinking the LED multiple times.
 */
static void Blinky() {
	unsigned int i;
	unsigned int compareVal = 65535;

	for (i = 0; i < compareVal; i++) {
		P1OUT ^= LED0 | LED1;
		P4OUT ^= LED2 | LED3 | LED4 | LED5;
		Delay(DELAY_3750);
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

	signed long adc_channel_val = 0;

	adc_spi_init();	/*Setup tranmission to ADC*/
	adc_init();	/*Initialize ADC*/
	adc_selfcal();	/*Run a selfcal on all channels*/
	adc_read_convert(0);

	adc_channel_val = adc_in(2); /*Read in ADC channel 2 reading*/
	printf("%ld", adc_channel_val);
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
	unsigned int i = 0;

	while(i < numOfTests) {
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

		if (tests[i].PostDelay != DELAY_FOREVER) {
			i++;
		} else if (tests[i].PostDelay > DELAY_0) {
			Delay(tests[i].PostDelay);
		}
	}
}
