#include <plib.h>
#include "CerebotMX7cK.h"
#include "LCDLIB.h"
#include "I2C_EEPROM_LIB.h"



void system_init()
{
    //initialize resources
    Cerebot_mx7cK_setup(); //setup
    init_EEPROM(Fsck);
    PMP_init();
    LCD_init();
}

int main(void){
    int SlaveAddress=0x50;
    char i2cData[200];
    int mem_addr=0x1234;
    int len=4;
    system_init();
    //create i2c frame
    int i;
    for (i=0;i<len;i++)
    {
        i2cData[i]=i;
        //printf(i);
    }
     
    I2CWriteEEPROM(SlaveAddress, mem_addr, &i2cData,len );
            //(int SlaveAddress, int mem_addr, char *i2cData, int len)
    while(1);
    
    return (1);
}