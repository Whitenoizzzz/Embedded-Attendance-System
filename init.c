
/**
	@version 0.1
	@file init.c
	@brief Init function for the I/O
*/


/******************************************************************************/
/* INCLUDED FILES                                                             */
/******************************************************************************/
#include <avr/io.h>
#include "init.h"
/******************************************************************************/

/******************************************************************************/
/* FUNCTIONS                                                                  */
/******************************************************************************/

/**
	@brief Init the PORTx and DDRx register for the I/O setup
*/
void init(void){
	
	// Digital input
	DDRB &= ~(1 << DDB0);		// PB0 as Input
	DDRB &= ~(1 << DDB1);		// PB1 as Input
	PORTB |= ( 1 << PB0);		// Pullup PB0
	PORTB |= ( 1 << PB1);		// Pullup PB1
	
	// Poti
	DDRC &= ~(1 << DDC0);		// PBC as Input
	PORTC &= ~( 1 << PC0);		// No Pullup PC0
	
	// ADC
	ADMUX = (1 << REFS0); // AVcc as reference, ADC0
	ADCSRA = (1 << ADPS2)| (1 << ADPS1); // ADC Prescale by 64
	ADCSRA |= (1 << ADSC)|(1 << ADATE)|(1 << ADEN); // Start conversion
												//Free running, Enalbe ADC
	
	
}
/******************************************************************************/
