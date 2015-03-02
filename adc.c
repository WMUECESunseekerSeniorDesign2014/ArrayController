/* Author: Austin Cross and Joel Dewey
 *
 * AD7739 code for the 2015 MPPT Array Controller
 *
 * Code if modified from Dr. Bazuin BPS Code
 *
 */

#include "./inc/main.h"
#include "./inc/adc.h"

/*================================== ADC Functions ======================================*/

void adc_reset()    //reset the ADC
{
  // Software Reset
  adc_select;
  adc_spi_transmit(0x00);
  adc_spi_transmit(0xFF);
  adc_spi_transmit(0xFF);
  adc_spi_transmit(0xFF);
  adc_spi_transmit(0xFF);
  adc_deselect;
}

char adc_init()    // initialize the ADC
{
  char status_adc;
  char i;

  // reset the device to default conditions
  adc_reset();

  // IO Control Initialization
  adc_select;
  adc_spi_transmit(ADC_IOPORT);             // Command Register write for IOPORT
  adc_spi_transmit(ADCPODIR | ADCP1DIR | ADCPO);	// P0 and P1 Outputs, Data P0=1 P1=0
  adc_deselect;

  // Initialize Channel 0-7

  for (i=0;i<=7;i++)
  {
    adc_select;
    adc_spi_transmit(ADC_SETUP0 | i);      	// Command Register write for SETUP0  			// | i changes the channel
    adc_spi_transmit(ENABLE | RNG5);             // 0-5 V conversion, RDY in status, not-continuous conversion
    adc_deselect;

    adc_select;
    adc_spi_transmit(ADC_CT0 | i);      	// Command Register write for TC0
    adc_spi_transmit(CHOP | FWRATE); // Use Chopping and FW = FWRATE
    adc_deselect;

    adc_select;
    adc_spi_transmit(ADC_MODE0 | i);      	// Command Register write for MODE0
    adc_spi_transmit(MIDLE | CLKDIS | BIT24_16n);  // Idle, MCLKOUT disabled, 24 bit conv.
    adc_deselect;

    status_adc=adc_status();
  }

  status_adc=adc_status();
  return(status_adc);
}

char adc_run_config(void)    //Operational ADC configuration (read channels 0-3)
{
  char status_adc;
  char i;

  // Reconfigure Channel 0-7

  for (i=0;i<=3;i++)
  {
    adc_select;
    adc_spi_transmit(ADC_SETUP0 | i);      	// Command Register write for SETUP0
    adc_spi_transmit(RNG5);            		// 0-5 V conversion, RDY in status, not-continuous conversion
    adc_deselect;
  }

  for (i=4;i<=7;i++)
  {
    adc_select;
    adc_spi_transmit(ADC_SETUP0 | i);      	// Command Register write for SETUP0
    adc_spi_transmit(ENABLE | RNG5);        // 0-5 V conversion, RDY in status, not-continuous conversion
    adc_deselect;
  }

  status_adc=adc_status();
  return(status_adc);
}


void adc_ctset(char adc_tcword, char adc_channel)    //Set the channel conversion time
{
  adc_select;
  adc_spi_transmit(ADC_CT0 | adc_channel);      	// Command Register write for TC0,1,2,3
  adc_spi_transmit(adc_tcword);             // Use char provided
  adc_deselect;
}


// Run self cal on all 8 channels
char adc_selfcal(void)	//Self-calibration (internal shorting)
{
  char status_adc, i;

  for(i=0;i<=7;i++)		// zero cal for each channel
  {
  	status_adc=adc_zselfcal(i);
  }

  for(i=0;i<=7;i++)
  {
  	status_adc=adc_fselfcal(i);		// full scale cal for each channel
  }

  status_adc=adc_status();

  return(status_adc);
}

char adc_zselfcal(char adc_channel)	//Per channel zero self-calibration (internal shorting)
{
  unsigned int i;
  char status_adc;

  // Set the channel MODE to zero-scale self cal
  adc_select;
  adc_spi_transmit(ADC_MODE0 | adc_channel);      	// Command Register write for MODE
  adc_spi_transmit(MZSELFCAL | CLKDIS | BIT24_16n | CLAMP); // Mode zero-Scale Self Cal with other mode bits
  adc_deselect;

  i = 32;		// Short SW Delay
  do i--;
  while (i != 0);

  //Wait for the conversion to be done
  while ( (P2IN & ADC_RDYn) == ADC_RDYn);
  status_adc=adc_status();
  return(status_adc);
}

char adc_fselfcal(char adc_channel)	//Per channel zero self-calibration (internal shorting)
{
  unsigned int i;
  char status_adc;

  // Set the channel MODE to zero-scale self cal
  adc_select;
  adc_spi_transmit(ADC_MODE0 | adc_channel);      	// Command Register write for MODE
  adc_spi_transmit(MFSELFCAL | CLKDIS | BIT24_16n | CLAMP); // Mode zero-Scale Self Cal with other mode bits
  adc_deselect;

  i = 32;		// Short SW Delay
  do i--;
  while (i != 0);

  //Wait for the conversion to be done
  while ( (P2IN & ADC_RDYn) == ADC_RDYn);
  status_adc=adc_status();
  return(status_adc);
}

signed long adc_in(char adc_channel)	// read voltage from Channel adc_channel (0-3)
{
  static signed long adc_value;
  static unsigned char abyte1, abyte2, abyte3;
  unsigned int i;

  //Set the channel MODE to single conversion
  adc_select;
  adc_spi_transmit(ADC_MODE0 | adc_channel);              // Command Register for MODE0,1,2,3
  adc_spi_transmit(MSINGLE| CLKDIS | BIT24_16n);  // Mode single conversion with other mode bits
  adc_deselect;

  i = 32;		// Short SW Delay
  do i--;
  while (i != 0);

  //Wait for the conversion to be done
  while ( (P2IN & ADC_RDYn) == ADC_RDYn);

  //Read the ADC Data
  adc_select;
  adc_spi_transmit(ADC_COMM_RD | ADC_DATA0 | adc_channel);  // Command Register for ADCCON
  abyte1 = adc_spi_exchange(0x00);
  abyte2 = adc_spi_exchange(0x00);
  abyte3 = adc_spi_exchange(0x00);
  adc_deselect;

  adc_value = 0;
  adc_value = abyte1;
  adc_value = (adc_value<<8) | abyte2;
  adc_value = (adc_value<<8) | abyte3;

  return(adc_value);
}

// In the dump mode, the upper 8 bits is the channel status.
signed long adc_indump(char adc_channel)	// read voltage from Channel adc_channel (0-3)
{
  static signed long adc_value;
  static unsigned char abyte0, abyte1, abyte2, abyte3;
  unsigned int i;

  //Set the channel MODE to single conversion
  adc_select;
  adc_spi_transmit(ADC_MODE0 | adc_channel);              // Command Register for MODE0,1,2,3
  adc_spi_transmit(MSINGLE| CLKDIS | DUMP | BIT24_16n);  // Mode single conversion with other mode bits
  adc_deselect;

  i = 32;		// Short SW Delay
  do i--;
  while (i != 0);

  //Wait for the conversion to be done
  while ( (P2IN & ADC_RDYn) == ADC_RDYn);

  //Read the ADC Data
  adc_select;
  adc_spi_transmit(ADC_COMM_RD | ADC_DATA0 | adc_channel);  // Command Register for ADCCON
  abyte0 = adc_spi_exchange(0x00);
  abyte1 = adc_spi_exchange(0x00);
  abyte2 = adc_spi_exchange(0x00);
  abyte3 = adc_spi_exchange(0x00);
  adc_deselect;

  adc_value = abyte0;
  adc_value = (adc_value<<8) | abyte1;
  adc_value = (adc_value<<8) | abyte2;
  adc_value = (adc_value<<8) | abyte3;

  return(adc_value);
}

void adc_convert(void)	// read voltage from multiple channels - continuous conversion
{
  extern signed long adc_voltage[8*4];
  extern unsigned char adc_stat[8*4];
  static signed long adc_value;
  static unsigned char abyte0, abyte1, abyte2, abyte3;
  unsigned int i;

  // IO Control Initialization
  adc_select;
  adc_spi_transmit(ADC_IOPORT);             // Command Register write for IOPORT
  adc_spi_transmit(RDYFN | ADCPODIR | ADCP1DIR | ADCPO);	// P0 and P1 Outputs, Data P0=1 P1=0
  adc_deselect;


  //Set the channel MODE to continuous conversion
  adc_select;
  adc_spi_transmit(ADC_MODE0);              // Command Register for MODE0,1,2,3
  adc_spi_transmit(MCONT| CLKDIS | DUMP | BIT24_16n);  // Mode continuous conversion with other mode bits
  adc_deselect;

  i = 32;		// Short SW Delay
  do i--;
  while (i != 0);

  //Wait for the conversion to be done
  while ( (P2IN & ADC_RDYn) == ADC_RDYn);

  //Set the channel MODE to idle
  adc_select;
  adc_spi_transmit(ADC_MODE0);              // Command Register for MODE0,1,2,3
  adc_spi_transmit(MIDLE| CLKDIS | DUMP | BIT24_16n);  // Mode single conversion with other mode bits
  adc_deselect;

  for (i=0;i<=7;i++)
  {
    //Read the ADC Data
    adc_select;
    adc_spi_transmit(ADC_COMM_RD | ADC_DATA0 | i);  // Command Register for ADCCON
    abyte0 = adc_spi_exchange(0x00);
    abyte1 = adc_spi_exchange(0x00);
    abyte2 = adc_spi_exchange(0x00);
    abyte3 = adc_spi_exchange(0x00);
    adc_deselect;

    adc_value = abyte0;
    adc_value = (adc_value<<8) | abyte1;
    adc_value = (adc_value<<8) | abyte2;
    adc_value = (adc_value<<8) | abyte3;

    adc_stat[24+i] = (char)((adc_value & 0xFF000000)>>24);
    adc_voltage[24+i] = (adc_value & 0x00FFFFFF);
// Check overflow, negative clamps, positive rolls over
    if((adc_stat[24+i] & 0x01) == 0x01)
    {
  	  if((adc_stat[24+i] & 0x02) != 0x02) adc_voltage[24+i] += 0x01000000;
    }
  }

  // IO Control Initialization
  adc_select;
  adc_spi_transmit(ADC_IOPORT);             // Command Register write for IOPORT
  adc_spi_transmit(ADCPODIR | ADCP1DIR | ADCPO);	// P0 and P1 Outputs, Data P0=1 P1=0
  adc_deselect;

}

void adc_contconv_start(char adc_channel)	// read voltage from multiple channels
{
  // IO Control Initialization
  adc_select;
  adc_spi_transmit(ADC_IOPORT);             // Command Register write for IOPORT
  adc_spi_transmit(RDYFN | ADCPODIR | ADCP1DIR | ADCPO);	// P0 and P1 Outputs, Data P0=1 P1=0
  adc_deselect;

  //Set the channel MODE to continuous conversion
  adc_select;
  adc_spi_transmit(ADC_MODE0 | adc_channel);              // Command Register for MODE0,1,2,3
  adc_spi_transmit(MCONT| CLKDIS | DUMP | BIT24_16n);  // Mode continuous conversion with other mode bits
  adc_deselect;
}

void adc_idle(char adc_channel)	// read voltage from multiple channels
{
  //Set the channel MODE to idle
  adc_select;
  adc_spi_transmit(ADC_MODE0 | adc_channel);              // Command Register for MODE0,1,2,3
  adc_spi_transmit(MIDLE| CLKDIS | DUMP | BIT24_16n);  // Mode single conversion with other mode bits
  adc_deselect;
}

void adc_read_convert(char adc_channel)
{
  extern signed long adc_voltage[8*4];
  extern unsigned char adc_stat[8*4];
  static signed long adc_value;
  static unsigned char abyte0, abyte1, abyte2, abyte3;
  unsigned int i;

  for (i=adc_channel;i<=7;i++)
  {
    //Read the ADC Data
    adc_select;
    adc_spi_transmit(ADC_COMM_RD | ADC_DATA0 | i);  // Command Register for ADCCON
    abyte0 = adc_spi_exchange(0x00);
    abyte1 = adc_spi_exchange(0x00);
    abyte2 = adc_spi_exchange(0x00);
    abyte3 = adc_spi_exchange(0x00);
    adc_deselect;

    adc_value = abyte0;
    adc_value = (adc_value<<8) | abyte1;
    adc_value = (adc_value<<8) | abyte2;
    adc_value = (adc_value<<8) | abyte3;

    adc_stat[24+i] = (char)((adc_value & 0xFF000000)>>24);
    adc_voltage[24+i] = (adc_value & 0x00FFFFFF);
// Check overflow, negative clamps, positive rolls over
    if((adc_stat[24+i] & 0x01) == 0x01)		// overflow
    {
  	  if((adc_stat[24+i] & 0x02) != 0x02) adc_voltage[24+i] += 0x01000000;
    }
  }

  // IO Control Initialization
  adc_select;
  adc_spi_transmit(ADC_IOPORT);             // Command Register write for IOPORT
  adc_spi_transmit(ADCPODIR | ADCP1DIR | ADCPO);	// P0 and P1 Outputs, Data P0=1 P1=0
  adc_deselect;

}


void adc_io(void)	// Toggle the IO Pins
{

  char p1p2_adc;

  // IO Control Initialization
  adc_select;
  adc_spi_transmit(ADC_COMM_RD | ADC_IOPORT);	// Command to read DATA IO Register
  p1p2_adc=adc_spi_exchange(0xFF);                 // P2 P1 values with IO control
  adc_deselect;

  p1p2_adc ^= 0xC0;

  adc_select;
  adc_spi_transmit(ADC_IOPORT);             // Command write DATA IO Register
  adc_spi_transmit(p1p2_adc);                 // P2 P1 outputs with IO control
  adc_deselect;
}


char adc_status(void)	// Read the ADC Status Register
{
  char status_adc;

  adc_select;
  adc_spi_transmit(ADC_COMM_RD | ADC_STATUS);	// Command to read Status Register
  status_adc=adc_spi_exchange(0x00);	// Read the status
  adc_deselect;

  return(status_adc);
}

char adc_chstatus(char adc_channel)	// Read the ADC Cahnnel Status Register
{
  char chstatus_adc;

  adc_select;
  adc_spi_transmit(ADC_COMM_RD | ADC_STATUS0 | adc_channel);	// Command to read Channel Status Register
  chstatus_adc=adc_spi_exchange(0x00);	// Read the status
  adc_deselect;

  return(chstatus_adc);
}


/*================================== ADC Functions - End - ======================================*/

char adc_temp_check(void)
{

	volatile float temp_voltage;
    long max_temp_voltage;
	unsigned char i,j,index,max_temp_index,terr;
	extern float max_local_temp;
	extern int local_max_t_cell;
    extern signed long adc_voltage[8*4];
	extern unsigned char local_temp_Error[3];

	terr = 0x00;
	max_temp_index = 0;
	max_temp_voltage = 0;
	local_temp_Error[0] = 0;
	local_temp_Error[1] = 0;
	local_temp_Error[2] = 0;

	for (j=1;j<=3;j++)
	{
		for(i=0;i<=6;i++)
		{
			index = (j-1)*8+i;
			temp_voltage = (float)adc_voltage[index] * 2.5 /16777216.0;

			if(index<=20)				// 21 and 22 are inlet and outlet temps
			{
				if(adc_voltage[index] > max_temp_voltage)
				{
					max_temp_voltage = adc_voltage[index];
					max_temp_index = index;

				}
			  if(adc_voltage[index]<=0x00444952) local_temp_Error[j-1] |= (0x01<<i);// 45 deg
			  if(adc_voltage[index]> 0x00B00000) local_temp_Error[j-1] |= (0x01<<i);// no therm
			  //0x444952 is 45 deg
			  //0x6F286B is 25 deg
			  //0xA18AED is 0 deg
			  //0xC47711 is Therm not connected
			}
		}
		i=7;							// 7, 15, and 23 are 2.5V ref
		index = (j-1)*8+i;
		temp_voltage = (float)adc_voltage[index] * 2.5 /16777216.0;

		if(adc_voltage[index]<=0x00F5C28F) local_temp_Error[j-1] |= (0x01<<i);// Ref<=2.4V
		terr |= local_temp_Error[j-1];
	}
	max_local_temp = (float)max_temp_voltage * 2.5 /16777216.0;
	local_max_t_cell = (int)max_temp_index;
	return(terr);
}

char adc_remote_temp_check(void)
{

	volatile float temp_voltage;
    long max_temp_voltage;
	unsigned char i,j,index,max_temp_index,terr;
	extern float max_remote_temp;
	extern int remote_max_t_cell;
    extern long adc[8*4];
	extern unsigned char remote_temp_Error[3];

	terr = 0x00;
	max_temp_index = 0;
	max_temp_voltage = 0;
	for (j=1;j<=3;j++)
	{
		for(i=0;i<=6;i++)
		{
			index = (j-1)*8+i;
			temp_voltage = (float)adc[index] * 2.5 /16777216.0;

			if(index<=20)				// 21 and 22 are inlet and outlet temps
			{
				if(adc[index] > max_temp_voltage)
				{
					max_temp_voltage = adc[index];
					max_temp_index = index;

				}
			  if(adc[index]<=0x00444952) remote_temp_Error[j-1] |= (0x01<<i);// 45 deg
			  if(adc[index]> 0x00B00000) remote_temp_Error[j-1] |= (0x01<<i);// no therm
			  //0x6F286B is 25 deg
			  //0xA18AED is 0 deg
			  //0xC47711 is Therm not connected
			}
		}
		i=7;							// 7, 15, and 23 are 2.5V ref
		index = (j-1)*8+i;
		temp_voltage = (float)adc[index] * 2.5 /16777216.0;

		if(adc[index]<=0x00F5C28F) remote_temp_Error[j-1] |= (0x01<<i);// Ref<=2.4V
		terr |= remote_temp_Error[j-1];
	}
	max_remote_temp = (float)max_temp_voltage * 2.5 /16777216.0;
	remote_max_t_cell = (int)max_temp_index;
	return(terr);
}
