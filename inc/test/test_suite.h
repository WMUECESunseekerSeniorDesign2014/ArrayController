/**
 * @brief The header file defining the tests to be run on the Array Controller.
 * @author Joel Dewey <joel.s.dewey@wmich.edu>
 */

/**
 * Function prototype for executing tests.
 */
extern void ExecuteTests(long tests, unsigned char delay);

#ifndef TEST_SUITE_H_
#define TEST_SUITE_H_

/**
 * @defgroup testDefines Test Defines
 * @{
 */
#define BLINKY 			0x01 	   // LED blinking test.
#define RS232_PC 		0x02 	   // Read and write to the PC via RS-232.
#define RS232_GPS 		0x04 	   // Read and write to the GPS module via RS-232.
#define ADC				0x08 	   // Read from the ADC and print the results out to the console.
#define CAN_MPPT_ON		0x10 	   // Enable MPPTs using CAN.
#define CAN_MPPT_OFF	0x20 	   // Disable MPPTs using CAN.
#define CAN_MPPT_DATA	0x40 	   // Retrieve data from the MPPTs via CAN.
#define CAN_CAR_READ	0x80 	   // Read from the car's CAN.
#define CAN_CAR_WRITE	0x100 	   // Send data frame from AC to CAN bus.

#define DELAY			0x80000000 // Enable delay between each test. This is a parameter
/**
 * @}
 */

#endif /* TEST_SUITE_H_ */
