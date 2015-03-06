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

#ifndef TEST_SUITE_H_
#define TEST_SUITE_H_

/**
 * RS-232 variables have been placed here for testing.
 */
volatile int RX_INT_count = 0;
char test_buffer_PC[60];
char *putPC_ptr, *getPC_ptr;
bool put_status_PC, get_status_PC;
bool AC2PC_RX_flag = FALSE;

/**
 * @defgroup testDefines Test Defines
 * @{
 */
#define BLINKY 			0x01 	   // LED blinking test.
#define RS232_PC_PUTS	0x02 	   // Write to the PC via RS-232.
#define ADC				0x03 	   // Read from the ADC and print the results out to the console.
#define CAN_MPPT_ON		0x04 	   // Enable MPPTs using CAN.
#define CAN_MPPT_OFF	0x05 	   // Disable MPPTs using CAN.
#define CAN_MPPT_DATA	0x06 	   // Retrieve data from the MPPTs via CAN.
#define CAN_CAR_READ	0x07 	   // Read from the car's CAN.
#define CAN_CAR_WRITE	0x08 	   // Send data frame from AC to CAN bus.
#define RS232_PC_GETS	0x09	   // Read from the PC via RS-232.
/**
 * @}
 */

#endif /* TEST_SUITE_H_ */
