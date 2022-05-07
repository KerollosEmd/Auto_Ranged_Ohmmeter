/*
 * AROM_main.c
 *
 *  Created on: May 6, 2022
 *      Author: KeroEmad
 */

/*
 * @Layer: Standard headers
 * @Imports: Standard Libraries
 */
#include "STD_Types.h"
#include "BIT_MATH.h"
#define F_CPU 		1000000UL
#include <util/delay.h>

/*
 * @Layer: MCAL
 * @Imports: MCAL libraries
 */
#include "DIO_interface.h"
#include "GIE_interface.h"
#include "ADC_interface.h"


/*
 * @Layer: HAL
 * @Imports: HAL libraries
 */
#include "LCD_interface.h"

/*
 * End of Importing Section
 */


/*
 * @defines
 */
//..........
/*
 * End of defines
 */

/*
 * @Global_Variables
 */
u16 AROM_Au16Resistances[]={
		220,				//R1
		560,                //R2
		1000,               //R3
		2200,               //R4
		4700,               //R5
		10000,              //R6
		22000               //R7
};

u8 AROM_Au8Channels[]={
		0x01,  		 //CH1
		0x02,        //CH2
		0x04,        //CH3
		0x08,        //CH4
		0x10,        //CH5
		0x20,        //CH6
		0x40         //CH7
};

u16 AROM_u16DigitalReadings[7]={
		0
};
/*
 * End of Global Variables
 */

/*
 * @Functions_Prototypes
 */
//...........
/*
 * End of Functions Prototypes
 */


/*
 * @main function
 */

int main(void){

	/*
	 * @Local_Variables
	 */
	u8  Local_u8Index=0;
	u16 Local_u16DigitalValue=0;
	f32 Local_f32ResX=0.0;
	f32 Local_f32VoltX_mv=0;
	/*
	 * End of Local Variables
	 */

	/*
	 * @Initialization Functions of Peripherals
	 */
	DIO_voidInit();
	LCD_voidInit();
	ADC_voidInit();
	/*
	 * End of Initializations
	 */
	u8 Local_u8String[]="RES=";
	LCD_voidWriteString(Local_u8String);
	/*
	 * @Super_Loop
	 */
	while(1){

		/*Swapping between the 7 Channels and Take Readings*/
		for (Local_u8Index=0;Local_u8Index<7;Local_u8Index++){
			DIO_u8SetPortValue(DIO_u8_PORTC,AROM_Au8Channels[Local_u8Index]); //Setting High one channel and the other to Low
			
			/*
			 * if condition to check that there is no error return from the ADC_u8GetDigitalValueSync 
			 * hint: >>>> the instruction lines beside the debug comment, it for debugging and will be deleted 
			 *            at the Release state.
			 */
			if(STD_TYPES_OK==ADC_u8GetDigitalValueSync(ADC_u8_SINGLE_ADC7, &Local_u16DigitalValue)){
				/*
				 * Taking the reading and assign it to the Array of Readings to the corresponding reading element 
				 */
				AROM_u16DigitalReadings[Local_u8Index]=Local_u16DigitalValue;
				
				LCD_voidGoToXY(0,LCD_u8_LINE_2);				//debug
				LCD_voidWritNumber(Local_u16DigitalValue);      //debug
				_delay_ms(500);									//debug

			}else{
				/*
				 * if the ADC_u8GetDigitalValueSync Function return Error
				 * in this case it assign 0 to the corresponding Digital Reading in the array
				 */
				AROM_u16DigitalReadings[Local_u8Index]=0;
			}
		}
		/*End of for loop*/

		/*Swapping for checking for valid ranges and Calculate the equivalent Resistance*/
		for(Local_u8Index=6;Local_u8Index>=0;Local_u8Index--){
			/*@description:
			 * It checks the array from bottom to top of it as if the range of Digital reading
			 * (corresponding to large resistors) is nearly equals to (1/2) of ADC Range (1024)
			 * it means that the unknown resistance is approximately equals to this resistor
			 * and it belongs to its range. So there is no need to check the other readings as
			 * they will be very large and inaccurate.
			 */
			if(AROM_u16DigitalReadings[Local_u8Index]>=440){
				Local_f32VoltX_mv=(f32)(Local_u16DigitalValue*(5000UL))/1024.0;
				Local_f32ResX=Local_f32VoltX_mv*AROM_Au16Resistances[Local_u8Index]/(4300UL-Local_f32VoltX_mv);
				LCD_voidGoToXY(4,LCD_u8_LINE_1);
				LCD_voidWritNumber((u32)Local_f32ResX);
				LCD_voidWriteChar(0xF4);
				LCD_voidWriteString("   ");
				break;
			}
		}
		/*
		 * End of for loop
		 */
		_delay_ms(1000);
	}
	/*
	 * End of Super Loop
	 */

	return 0;
}
/*
 * End of main function
 */
