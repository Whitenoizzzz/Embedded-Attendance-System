/**
	@file one.c
	@brief Main programm for time clock
*/

const char MtrNum[] __attribute__((__progmem__)) = "28850";


/**
	@brief The CPU speed in Hz
*/
#define F_CPU 8000000UL

/**
	@brief I2C Address of the DS1307
*/
#define DS1307_I2C_ADR 0xD0 /*todo*/

/**
	@brief I2C Address of the EEPROM
*/
#define EEPROM_I2C_ADR 0b10100000 /*todo*/




/**
	@brief Testing mode (automatic time skips)
*/
#define TESTMODE




/******************************************************************************/
/* INCLUDED FILES                                                             */
/******************************************************************************/
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include "i2c_master.h"
#include "init.h"
#include "lcd.h"
#include "stdio.h"
/******************************************************************************/


/******************************************************************************/
/* GLOBAL MEMORY                                                              */
/******************************************************************************/

// Global Time memory
uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
uint8_t year;
uint8_t weekday;

uint8_t rtc_address = 0x08;
uint8_t ee_address = 0;
/******************************************************************************/



/******************************************************************************/
/* FUNCTIONS                                                                  */
/******************************************************************************/

/**
	@brief Convert from BCD to Binary
	@param in BCD input (00-99)
	@return Binary output
*/
uint8_t ds1307_decodeBcd(uint8_t in){
	return ((in>>4)*10 + (in&0xF));
	//todo
}
/******************************************************************************/

/**
	@brief Convert from Binary to BCD
	@param in Binary input (0-99)
	@return BCD output
*/
uint8_t ds1307_encodeBcd(uint8_t in){
	return ((in / 10) << 4 ) | (in % 10); 
}
/******************************************************************************/


/**
	@brief Show time/date with the LCD
*/
void display_standby(uint8_t present, uint8_t pos){
	char str[16];
	
	// Time and Year
	snprintf(str, 16, "%02d:%02d:%02d  %02d.%02d", hour, minute,
			second, day, month);
	
	lcd_clear();
	lcd_string(str);
	

	// Show present or absent
	if (present)
		snprintf(str, 16, "present %d", pos);
	else
		snprintf(str, 16, "absent %d", pos);	
	
	lcd_setcursor(0,2);
	lcd_string(str);

	return;
}
/******************************************************************************/

/**
	@brief Write a row byte to the DS1307
	@param adr address to write
	@param data byte to write
*/
void ds1307_write(uint8_t adr, uint8_t data){
	
	if (i2c_master_open_write(DS1307_I2C_ADR))
		return;
	
	i2c_master_write(adr);
	i2c_master_write(data);
	
	i2c_master_close();
}
/******************************************************************************/

/**
	@brief Read a row byte from the DS1307
	@param adr address to read
	@return the received byte
*/
uint8_t ds1307_read(uint8_t adr){
	uint8_t ret;

	if (i2c_master_open_write(DS1307_I2C_ADR))
		return 0;
	
	i2c_master_write(adr);
	i2c_master_open_read(DS1307_I2C_ADR);
	ret = i2c_master_read_last();
	
	i2c_master_close();

	return ret;

}
/******************************************************************************/

/**
	@brief Start or freeze the clock of the DS1307
	@param run zero for stop, all other for run
*/
void ds1307_rtc(uint8_t run){
	
	uint8_t readout;
	
	// Read current value
	readout = ds1307_read(0x00);
	
	
	// Set CH bit
	if (run)
		readout &= ~(0x80);
	else
		readout |= 0x80;
		
	// Write value back
	ds1307_write(0x00, readout);
}
/******************************************************************************/

/**
	@brief Write the current time to the DS1307
	@return zero for no error, one for communication error
*/
uint8_t ds1307_setTime(void){
	uint8_t chbit = ds1307_read(0x00) & 0x80;

	// Open device for write
	if (i2c_master_open_write(DS1307_I2C_ADR))
		return 1;

	i2c_master_write(0x00);
	if (chbit)
		i2c_master_write(ds1307_encodeBcd(second) | 0x80);
	else
		i2c_master_write(ds1307_encodeBcd(second) & 0x7F);		
	
	i2c_master_write(ds1307_encodeBcd(minute));
	i2c_master_write(ds1307_encodeBcd(hour));
	
	i2c_master_write(weekday);		
	
	i2c_master_write(ds1307_encodeBcd(day));
	i2c_master_write(ds1307_encodeBcd(month));
	i2c_master_write(ds1307_encodeBcd(year));		
	
	
	// Close I2C bus
	i2c_master_close();
	
	return 0;
}
/******************************************************************************/

/**
	@brief Get the current time from the DS1307
	@return zero for no error, one for communication error
*/
uint8_t ds1307_getTime(void){

	// Open device for write
	if (i2c_master_open_write(DS1307_I2C_ADR))
		return 1;
	
	// select reading position (0x00)
	i2c_master_write(0x00);
	
	// (Re-)Open device for read
	i2c_master_open_read(DS1307_I2C_ADR);
	
	// Read value
	second = ds1307_decodeBcd(i2c_master_read_next() & 0x7F);
	// TODO minute, hour, ...
	minute= ds1307_decodeBcd(i2c_master_read_next());
	hour= ds1307_decodeBcd(i2c_master_read_next() & 0x3F);
	weekday= ds1307_decodeBcd(i2c_master_read_next());
	day= ds1307_decodeBcd(i2c_master_read_next());
	month= ds1307_decodeBcd(i2c_master_read_next());;
	year= ds1307_decodeBcd(i2c_master_read_last());
	
	// Close I2C bus
	i2c_master_close();
	
	return 0;
}
/******************************************************************************/


/**
	@brief Load 8 bit value from the EEPROM
	@return loaded value
*/

uint8_t load_value8bit(uint8_t pos){
	uint8_t value;

	/* TODO */
	i2c_master_open_write(EEPROM_I2C_ADR);
	i2c_master_write(pos);
	i2c_master_open_read(EEPROM_I2C_ADR);
	value= i2c_master_read_last();
	i2c_master_close();
	
	return value;
}
/******************************************************************************/


/**
	@brief Save a 8 bit value to the EEPROM
	@param tosave value to save
*/
void save_value8bit(uint8_t tosave, uint8_t pos){

	/* TODO */
	i2c_master_open_write(EEPROM_I2C_ADR);
	i2c_master_write(pos);
	i2c_master_write(tosave);

	i2c_master_close();
	_delay_ms(10); // wait 10ms to make sure that data is written
}
/******************************************************************************/


/**
	@brief Reset memory. Set memory pos to zero and reset present flag
*/
void reset_memory(void){
	
	ds1307_setTime();
	// Reset data (only DS1307, no need to reset the EEPROM data)
	//todo
	rtc_address = 0x08;
	
	
	
	
	// Show reset info
	lcd_clear();
	lcd_string("memory reset");

	// Wait for buttons release
	while (( ~PINB & (1 << PB0) ) || ( ~PINB & (1 << PB1) ))
		_delay_ms(100);
	
	_delay_ms(1000);
	
}
/******************************************************************************/

/**
	@brief Read memory. Show all datasets step by step
*/
void read_data(void){
	uint8_t i;
	
	// Check for no data
	if (ds1307_read(0x08) == 0){
		lcd_clear();
		lcd_string("no data");

		// Wait for buttons release
		while (( ~PINB & (1 << PB0) ) || ( ~PINB & (1 << PB1) ))
			_delay_ms(100);	
		_delay_ms(1000);

		return;
	}
	
	
	lcd_clear();
	lcd_string("read data");	
	
	// Wait for buttons release
	while (( ~PINB & (1 << PB0) ) || ( ~PINB & (1 << PB1) ))
		_delay_ms(100);	
	_delay_ms(1000);	
	
	
	// Show data sets
	for (i = 0; i < 1 /*todo*/; i++){
		display_standby(ds1307_read(i), i);
		if ((( ~PINB & (1 << PB0) ) || ( ~PINB & (1 << PB1) )))
			continue;
		else
			;
		// todo
	}
	
	
	// Ask for data reset
	lcd_clear();
	lcd_string("Yes  Reset");	
	lcd_setcursor(0,2);
	lcd_string("No   data?");	
	_delay_ms(20);
		while (( ~PINB & (1 << PB0) ) || ( ~PINB & (1 << PB1) ))
		;
	_delay_ms(20);
	
	// Continue if button 2 is pressed
	while (PINB & (1 << PB1)){
		// Reset if button 1 is pressed
		if (~PINB & (1 << PB0)){
			reset_memory();
			return;
		}
	}
}


/**
	@brief react on button: Arrive
*/
void button_arrive(void){
	char str[16];


	// Check if memory is full
	if ( rtc_address == 0x3F )
		return;

	// Check if already present
	// todo
	if (ds1307_read(rtc_address) == 1)
		return;


	// Save current time for arrive
	// todo	
	ds1307_getTime();
	
	save_value8bit(day, ee_address);
	ee_address++;
	
	save_value8bit(month, ee_address);
	ee_address++;
	
	save_value8bit(hour, ee_address);
	ee_address++;
	
	save_value8bit(minute, ee_address);
	ee_address++;
	
	// Show saved time
	snprintf(str, 16, "%02d:%02d:%02d  20%02d", hour, minute,
			second, year);
	
	lcd_clear();
	lcd_string(str);
	lcd_setcursor(0,2);
	lcd_string("arrive saved");
	
	// Set present flag
	// todo
	ds1307_write(1, 0x3F);
	
	// Wait for buttons release
	while (( ~PINB & (1 << PB0) ) || ( ~PINB & (1 << PB1) ))
		_delay_ms(100);		
	_delay_ms(100);

#ifdef TESTMODE
	// todo
	hour = hour+8;
	if (hour > 24){
		day++;
		hour = hour - 24;
	}
	
	ds1307_setTime();
#endif



}
/******************************************************************************/


/**
	@brief react on button: Leave
*/
void button_leave(void){

	char str[16];

	// Check if memory is full
	if (rtc_address == 0x3F)
		return;


	// Check if already present
	// todo
	if (ds1307_read(rtc_address) == 1)
		return;
	

	// Save current time for leave
	// todo
	ds1307_getTime();
	
	save_value8bit(hour, ee_address);
	ee_address++;
	
	save_value8bit(minute, ee_address);
	ee_address++;
	
	// Show saved time
	snprintf(str, 16, "%02d:%02d:%02d  20%02d", hour, minute,
			second, year);
	
	lcd_clear();
	lcd_string(str);
	lcd_setcursor(0,2);
	lcd_string("leave saved");
	
	// reset present flag and count up memory position
	// todo
	rtc_address++;
	ds1307_write(0, 0x3F);
	

	// Wait for buttons release
	while (( ~PINB & (1 << PB0) ) || ( ~PINB & (1 << PB1) ))
		_delay_ms(100);		
	_delay_ms(1000);

#ifdef TESTMODE
	// todo
	day++;
	hour = 7;
	ds1307_setTime();
#endif

}
/******************************************************************************/


/**
	@brief Main function
	@return only a dummy to avoid a compiler warning, not used
*/
int main(void){


	DDRB |= (1 << DDB5);		// DEBUG
	
	init(); 	// Function to initialise I/Os
	lcd_init(); // Function to initialise LCD display
	i2c_master_init(1, 10); // Init TWI
	ds1307_rtc(1);
	
	
	// Check for reset button combination at power up
#ifdef TESTMODE
	if (( ~PINB & (1 << PB0) ) && ( ~PINB & (1 << PB1) )){
		// todo
		reset_memory();
	}
#endif

	// Loop forever
	for(;;){
		
		// Short delay
		_delay_ms(100);
		PORTB ^= ( 1 << PB5);
		
		// Load current time/date from DS1307
		ds1307_getTime();
		
		// Show current time/date
		display_standby(ds1307_read(rtc_address), rtc_address);

		// Buttons
		// todo
		if ( ~PINB & (1 << PB0) )
		{
			_delay_ms(55);
			button_arrive();
		}

		if ( ~PINB & (1 << PB1) )
		{
			_delay_ms(55);
			button_leave();	
		}
		
		if (( ~PINB & (1 << PB0) ) && ( ~PINB & (1 << PB1) ))
		{
			read_data();
		}
		
	}
		
		
		
		return 0;
	}

	
/******************************************************************************/
