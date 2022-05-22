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
#define F_CPU 		8000000UL
#include <util/delay.h>

/*
 * @Layer: MCAL
 * @Imports: MCAL libraries
 */
#include "DIO_interface.h"
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
#define SIMULATION	0
#define REAL		1
#define AROM_Mode		REAL
#define LM317_VREF 1.25
#define LM317_R  10
/*
 * End of defines
 */

/*
 * @Global_Variables
 */
u16 AROM_Au16Resistances[] = {
#if   (AROM_Mode==REAL)
    10,
    208, //R1
    548, //R2
    983, //R3
    2110, //R4
    4550, //R5
    9730, //R6
    22600 //R7
#else
    220, //R1
    560, //R2
    1000, //R3
    2200, //R4
    4700, //R5
    10000, //R6
    22000//R7
#endif

};
u8 AROM_Au8Channels[] = {0x01, //CH1
    0x02, //CH2
    0x04, //CH3
    0x08, //CH4
    0x10, //CH5
    0x20, //CH6
    0x40, //CH7
    0x80};

u16 AROM_Au16DigitalReadings[8] = {0};

int main(void) {

    /*
     * @Local_Variables
     */
    u8 Local_u8Index = 0;
    u16 Local_u16DigitalValue = 0;
    f32 Local_f32ResX = 0;
    f32 Local_f32VoltX_mv = 0;
    f32 Local_f32Loading = 0;

    /*
     * End of Local Variables
     */

    /*
     * @Local Strings
     */
    u8 Local_u8String_Load[] = "====Loading====";
    u8 Local_u8String_R1[] = "R1=";
    u8 Local_u8String_Rx[] = "Rx1=";
    u8 Local_u8String_ADC[] = "ADC=";
    u8 Local_u8String_ConvEr[] = "Conversion Error";
    u8 Local_u8String_mv[] = "mV";
    /*
     * End of Local Strings
     */

    /*
     * @Initialization Functions of Peripherals & Devices
     */
    DIO_voidInit();
    LCD_voidInit();
    ADC_voidInit();
    ADC_u8SetPrescaler(ADC_u8_PS_CLK_BY_2);
    /*
     * End of Initializations
     */

    /*
     * @Super_Loop
     */
    while (1) {
        LCD_voidWriteCommand(LCD_Clear);
        LCD_voidWriteString(Local_u8String_Load);
        /*Swapping between the 7 Channels and Take Readings*/
        for (Local_u8Index = 0; Local_u8Index < 8; Local_u8Index++) {
            DIO_u8SetPortValue(DIO_u8_PORTC, AROM_Au8Channels[Local_u8Index]); //Setting High one channel and the other to Low

            /*
             * The if condition to check that there is no error return from the ADC_u8GetDigitalValueSync
             * hint: >>>> the instruction lines beside the debug comment, it for debugging and will be deleted 
             *            at the Release state.
             */
            if (STD_TYPES_OK
                    == ADC_u8GetDigitalValueSync(ADC_u8_SINGLE_ADC7,
                    &Local_u16DigitalValue)) {
                /*
                 * Taking the reading and assign it to the Array of Readings to the corresponding reading element 
                 */
                AROM_Au16DigitalReadings[Local_u8Index] = Local_u16DigitalValue;

                Local_f32Loading = 14.3 * (Local_u8Index + 1);
                LCD_voidGoToXY(6, LCD_u8_LINE_2);
                LCD_voidWritNumber((u32) Local_f32Loading);
                LCD_voidWriteChar('%');
                _delay_ms(10);

            } else {
                /*
                 * if the ADC_u8GetDigitalValueSync Function return Error
                 * in this case it assign 0 to the corresponding Digital Reading in the array
                 */
                AROM_Au16DigitalReadings[Local_u8Index] = 0;
                LCD_voidWriteCommand(LCD_Clear);
                LCD_voidWriteString(Local_u8String_ConvEr);
            }
        }
        /*End of for loop*/
        Local_f32Loading = 0;

        /*Swapping for checking for valid ranges and Calculate the equivalent Resistance*/
        for (Local_u8Index = 7; Local_u8Index >= 0; Local_u8Index--) {
            /*@description:
             * It checks the array from bottom to top of it as if the range of Digital reading
             * (corresponding to large resistors) is nearly equals to (1/2) of ADC Range (1024)
             * it means that the unknown resistance is approximately equals to this resistor
             * and it belongs to its range. So there is no need to check the other readings as
             * they will be very large and inaccurate.
             */
            if ((AROM_Au16DigitalReadings[Local_u8Index] >= 430)
                    || (AROM_Au16DigitalReadings[Local_u8Index] < 430
                    && Local_u8Index == 0)) {
                Local_f32VoltX_mv = (f32) (AROM_Au16DigitalReadings[Local_u8Index]
                        * (5000UL)) / 1024;
                Local_f32ResX = (((Local_f32VoltX_mv)/LM317_VREF )-1)*((f32)LM317_R) ;
                //Local_f32ResX = (Local_f32VoltX_mv* AROM_Au16Resistances[Local_u8Index]) / (4440UL - Local_f32VoltX_mv);
                LCD_voidWriteCommand(LCD_Clear);

                LCD_voidWriteString(Local_u8String_Rx);






                LCD_voidWritNumber(Local_f32ResX);
                LCD_voidWriteChar(0xF4); // The OHM symbol

                LCD_voidWriteChar(' ');
                LCD_voidWritNumber(Local_f32VoltX_mv); //debug
                LCD_voidWriteString(Local_u8String_mv); //debug
                LCD_voidGoToXY(0, LCD_u8_LINE_2); //debug
                LCD_voidWriteString(Local_u8String_ADC); //debug
                LCD_voidWritNumber(AROM_Au16DigitalReadings[Local_u8Index]); //debug
                LCD_voidWriteChar(' '); //debug
                LCD_voidWriteString(Local_u8String_R1); //debug
                LCD_voidWritNumber(AROM_Au16Resistances[Local_u8Index]); //debug
                _delay_ms(500);
                break;
            }
        }
        /*
         * End of for loop
         */
        _delay_ms(100);
    }
    /*
     * End of Super Loop
     */

    return 0;
}
/*
 * End of main function
 */
