#ifndef ADC_H_
#define ADC_H_

/*
 * AD7739 Header File
 *
 *  Created on: Jan 13, 2015
 *      Author: Austin Cross
 *
 * ADC header file for the AD7739 copied from Dr. Bazin BPS code
 *
 */

 // MSP430 SPI interface header file

// Public Function Prototypes

extern void adc_spi_init( void );	                          // set up USCB0 as SPI
extern void adc_spi_transmit( unsigned char data );      // transmit data, ignore received data
extern unsigned char adc_spi_exchange( unsigned char data );  // exchange data (data type: char)

// Public Function Prototypes

void adc_reset(void); // ADC Reset
extern char adc_init(void); // ADC Initialization
void adc_ctset(char adc_tcword, char adc_channel); // Channel Time Conversion
extern char adc_run_config(void); // Run Configuration - Set Volltage Levels
extern char adc_selfcal(void); // ADC Self-Calibration - Run Zero and Full Calibrations on all Channels
extern char  adc_zselfcal(char adc_channel); // ADC Zero Calibration for one channel
extern char  adc_fselfcal(char adc_channel); // ADC Full Calibration for one channel
extern signed long adc_in(char adc_channel); // Single Conversion read from one channel
extern signed long adc_indump(char adc_channel); // Single Conversion Mode read from one channel with channel status
extern void adc_convert(void); // Continuous Converion for all channels
extern void adc_contconv_start(char adc_channel); // ADC Continious Conversion Start
extern void adc_idle(char adc_channel); // ADC Idle Mode
extern void adc_read_convert(char adc_channel); //Read Channel Data Registers

extern void adc_io(void); // Toggle IO pins
extern char adc_status(void); // Read the ADC Status Register
extern char adc_chstatus(char adc_channel); // Recieve a channel status

extern char adc_temp_check(void);
extern char adc_remote_temp_check(void);

// SPI port interface macros
#define adc_select		P4OUT &= ~ADC4_CSn; //Set CS low		HAVE TO FIGURE OUT PIN LAYOUT TO SEE WHAT PIN THE CS IS
#define adc_deselect	P4OUT |= ADC4_CSn;	//Set CS high

// variables
/* FWRATE Settings (1.5x the following):
 * 2 - 84 us, 9 - 230 us, 10 - 251 us, 17 - 397 us
 * 46 - 1001 us, 127 - 2689 us
 * est. 44 - 500 us, 60 - 666 us,
 */
#define FWRATE 60 // Used for 999 microsecond conversions

// ADC Registers

/* ADC Comm
0 0 0 0 0 0 Communications Register during a Write Operation (W)
0 0 0 0 0 1 I/O Port (R/W)
0 0 0 0 1 0 Revision (R)
0 0 0 0 1 1 Test (R/W)
0 0 0 1 0 0 ADC Status (R)
0 0 0 1 0 1 Checksum (R/W)
0 0 0 1 1 0 ADC Zero-Scale Cal (R/W)
0 0 0 1 1 1 ADC Full Scale (R/W)

0 0 1 0 0 0 Channel Data 0
to
0 0 1 1 1 1 Channel Data 7

0 1 0 0 0 0 Channel Zero-Scale Cal 0
to
0 1 0 1 1 1 Channel Zero-Scale Cal 7

0 1 1 0 0 0 Channel Full-Scale Cal 0
to
0 1 1 0 1 1 Channel Full-Scale Cal 7

1 0 0 0 0 0 Channel Status 0
to
1 0 0 1 1 1 Channel Status 7

1 0 1 0 0 0 Channel Setup 0
to
1 0 1 0 1 1 Channel Setup 7

1 1 0 0 0 0 Channel Conversion Time 0
to
1 1 0 0 1 1 Channel Conversion Time 7

1 1 1 0 0 0 Channel Mode
to
1 1 1 1 1 1 Channel Mode
*/

#define ADC_COMM 0x00		//Write Only
#define ADC_IOPORT 0x01
#define ADC_REV 0x02		//Read Only
#define ADC_TEST 0x03		//Read Only
#define ADC_STATUS 0x04		//Read Only
#define ADC_CHECKSUM 0x05
#define ADC_ZSCAL 0x06
#define ADC_FS 0x07

#define ADC_DATA0 0x08
#define ADC_DATA1 0x09
#define ADC_DATA2 0x0A
#define ADC_DATA3 0x0B
#define ADC_DATA4 0x0C
#define ADC_DATA5 0x0D
#define ADC_DATA6 0x0E
#define ADC_DATA7 0x0F
#define ADC_ZSCAL0 0x10
#define ADC_ZSCAL1 0x11
#define ADC_ZSCAL2 0x12
#define ADC_ZSCAL3 0x13
#define ADC_ZSCAL4 0x14
#define ADC_ZSCAL5 0x15
#define ADC_ZSCAL6 0x16
#define ADC_ZSCAL7 0x17
#define ADC_FS0 0x18
#define ADC_FS1 0x19
#define ADC_FS2 0x1A
#define ADC_FS3 0x1B
#define ADC_FS4 0x1C
#define ADC_FS5 0x1D
#define ADC_FS6 0x1E
#define ADC_FS7 0x1F
#define ADC_STATUS0 0x20
#define ADC_STATUS1 0x21
#define ADC_STATUS2 0x22
#define ADC_STATUS3 0x23
#define ADC_STATUS4 0x24
#define ADC_STATUS5 0x25
#define ADC_STATUS6 0x26
#define ADC_STATUS7 0x27
#define ADC_SETUP0 0x28
#define ADC_SETUP1 0x29
#define ADC_SETUP2 0x2A
#define ADC_SETUP3 0x2B
#define ADC_SETUP4 0x2C
#define ADC_SETUP5 0x2D
#define ADC_SETUP6 0x2E
#define ADC_SETUP7 0x2F
#define ADC_CT0 0x30
#define ADC_CT1 0x31
#define ADC_CT2 0x32
#define ADC_CT3 0x33
#define ADC_CT4 0x34
#define ADC_CT5 0x35
#define ADC_CT6 0x36
#define ADC_CT7 0x37
#define ADC_MODE0 0x38
#define ADC_MODE1 0x39
#define ADC_MODE2 0x3A
#define ADC_MODE3 0x3B
#define ADC_MODE4 0x3C
#define ADC_MODE5 0x3D
#define ADC_MODE6 0x3E
#define ADC_MODE7 0x3F

#define ADC_COMM_RD 0x40
#define ADC_COMM_WRn ~(0x40)

//ADC_IOPORT Configs
#define ADCPO 0x80
#define ADCP1 0x40
#define ADCPODIR 0x20	//0-output, 1-input
#define ADCP1DIR 0x10	//0-output, 1-input
#define RDYFN 0x08		//0-any unread, 1-all unread
#define RDYPWR 0x04		//0-high power mode, 1-low power mode (4 MHz)
#define ADCSYNC 0x01	//0-P1 digital IO, 1-P1 used as sync

//ADC_STATUS Configs
#define RDY0 0x01
#define RDY1 0x02
#define RDY2 0x04
#define RDY3 0x08
#define RDY4 0x10 // Ready Flag
#define RDY5 0x20
#define RDY6 0x40
#define RDY7 0x80

//ADC_STATUS0,1,2,3,4,5,6,7 Configs
#define CH210 0x70
#define SP0 0x10
#define SP1 0x08
#define NOREF 0x04
#define SIGN 0x02
#define OVR 0x01

//ADC_SETUP0,1,2,3,4,5,6,7 Configs
#define BUFOFF 0x80		// 0
#define COM1 0x40		// 0
#define COM0 0x20		// 0
#define STSTOPT 0x10	// 1
#define ENABLE 0x08		// enable = 1, disable = 0
#define RNG0 0x00		// +/- 1.25V
#define RNG1 0x01		// + 1.25V
#define RNG2 0x02		// +/- 0.625V
#define RNG3 0x03		// +0.625V
#define RNG4 0x04		// +/- 2.5V
#define RNG5 0x05		// +2.5V

//ADC_CT0,1,2,3,4,5,6,7 Configs, conversion time register
#define CHOP 0x80
#define FW 0x7F

//ADC_MODE0,1,2,3,4,5,6,7 Configs
#define MIDLE 0x00 // Idle Mode
#define MCONT 0x20 // Continuous Conversion Mode
#define MSINGLE 0x40 // Single Conversion Mode
#define MPWRDWN 0x60 // Power Down Mode
#define MZSELFCAL 0x80
#define MFSELFCAL 0xA0
#define MZSSYSCAL 0xC0
#define MFSSYSCAL 0xE0

#define CLKDIS 0x10 // MCLK disabled
#define DUMP 0x08
#define CONTRD 0x04
#define BIT24_16n 0x02
#define CLAMP 0x01

#endif /* ADC_H_ */
