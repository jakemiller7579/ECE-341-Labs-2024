//Lab 6

#include <plib.h>
#include "Project6.h"


//header
void system_init();

//code
#define delay 50

int main()
{
	system_init();
}

void system_init()
{
	Cerebot_mx7cK_setup();
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));// INPUT TO BUTTONS
	PMP_init();
	LCD_init();
}

void PMP_init()
{
	int config1= PMP_ON | PMP_READ_WRITE_EN | PMP_READ_POL_HI | PMP_WRITE_POL_HI;
	int config2=PMP_DATA_BUS_8 | PMP_MODE_MASTER1 | PMP_WAIT_BEG_1 | PMP_WAIT_MID_2 | PMP_WAIT_END_1;
	int config3=PMP_PEN_0; //only PMA0 enabled
	int config4= PMP_INT_OFF; //no interrupts used
	mPMPOpen(config1, config2, config3, config4);
}

void LCD_init()
{
	//LCD Initialize
	//Power On
	sw_msDelay(delay); //wait 
	//Set Function
	PMPSetAddress(0);
	PMPMasterWrite(0x38);
	//
	sw_msDelay(delay);
	//Set Display
	PMPSetAddress(0);
	PMPMasterWrite(0x0f);
	sw_msDelay(delay);
	//Clear Display
	PMPSetAddress(0);
	PMPMasterWrite(0x01);
	sw_msDelay(delay);
	//return
}

void sw_msDelay (unsigned int mS)
{
    int i;
    while(mS --) // SW Stop breakpoint
    {
        for (i = 0; i< COUNTS_PER_MS; i++) // 1 ms delay loop
    {
            // do nothing
    }
}
// Use code from Listing 3
}