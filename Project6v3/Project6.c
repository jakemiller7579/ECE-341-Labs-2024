#include "LCDLIB.h"

int main()
{
	system_init();
    char string1[]="Does Dr J prefer PIC32 or FPGA??";
    char string2[]="Answer: \116\145\151\164\150\145\162\041";
    while(1)
    {
        clearLCD();
        LCD_puts(&string1);
        sw_msDelay(5000);
        clearLCD();
        LCD_puts(&string2);
        sw_msDelay(5000);
    }
}