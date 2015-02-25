/**
 * @brief The header file defining the tests to be run on the Array Controller.
 * @author Joel Dewey <joel.s.dewey@wmich.edu>
 */

/**
 * @struct UnitTest
 * A struct for running tests. PreDelay performs a delay before the test, test
 * indicates what test will be performed, and PostDelay performs a delay after the test.
 */
typedef struct {
	unsigned int PreDelay;
	unsigned int Test;
	unsigned int PostDelay;
} UnitTest;

/**
 * Function prototype for executing tests.
 */
extern void ExecuteTests(UnitTest tests[], int numOfTests);

/**
 * @struct UnitTest
 * A struct for running tests. PreDelay performs a delay before the test, test
 * indicates what test will be performed, and PostDelay performs a delay after the test.
 */
typedef struct {
	int PreDelay;
	int Test;
	int PostDelay;
} UnitTest;

#ifndef TEST_SUITE_H_
#define TEST_SUITE_H_

#define CYCLE_TIME 63 // Rounded up from 62.5.

/**
 * @defgroup delayDefines Delay Timings
 * @{
 */
#define DELAY_0 0
#define DELAY_100 1600
#define DELAY_500 8000
#define DELAY_1000 16000
#define DELAY_FOREVER 65535 // Run test forever.
/**@}*/

/**
 * @defgroup testDefines Test Defines
 * @{
 */
#define BLINKY 			0x01 	   // LED blinking test.
#define RS232_PC 		0x02 	   // Read and write to the PC via RS-232.
#define ADC				0x03 	   // Read from the ADC and print the results out to the console.
#define CAN_MPPT_ON		0x04 	   // Enable MPPTs using CAN.
#define CAN_MPPT_OFF	0x05 	   // Disable MPPTs using CAN.
#define CAN_MPPT_DATA	0x06 	   // Retrieve data from the MPPTs via CAN.
#define CAN_CAR_READ	0x07 	   // Read from the car's CAN.
#define CAN_CAR_WRITE	0x08 	   // Send data frame from AC to CAN bus.
/**
 * @}
 */

#endif /* TEST_SUITE_H_ */
