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
	/// @todo Implement.
}

/**
 * Read and write to the PC using the RS-232 controller on the
 * Array Controller.
 */
static void RS232_PC() {
	/// @todo Implement.
}

/**
 * Read values from the ADC and print them to the console.
 */
static void ADC() {
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
 * A wrapper for __delay_cycles() which allows the developer to create a delay
 * of a number in microseconds.
 *
 * @note This should not be used in conjunction with functions that rely on ISRs!
 * @note This function will support a max delay of 65535 microseconds.
 *
 * @param[in] time The duration of the delay in microseconds.
 */
static void Delay(int time) {
	__delay_cycles((time * 1000)/CYCLE_TIME);
}

/**
 * The main function for executing tests for the Array Controller.
 *
 * @param[in] tests Describes each test via the use of the UnitTest structure.
 * @param[in] numOfTests The number of tests to be run.
 */
extern void ExecuteTests(UnitTest tests[], int numOfTests) {

}
