#include <plib.h>
#include "CerebotMX7cK.h"
#include "LCDLIB.h"
#define Fsck 400000
#define BRG_VAL ((FPB/2/Fsck)-2)
#define TEST_VAL 0X41 //hex for A 41

char BusyI2C2(void){
    return(I2C2CONbits.SEN || I2C2CONbits.PEN || I2C2CONbits.RSEN ||
            I2C2CONbits.RCEN || I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);
}//return non-zero value if I2C controller is busy

int main(void){
    unsigned char SlaveAddress, i2cbyte, write_err=0;
    char i2cdata[10]={0,0,0,0,0,0,0,0,0,0};
    int datasz, index;
    
    //initialize resources
    Cerebot_mx7cK_setup(); //setup
    OpenI2C2(I2C_EN, BRG_VAL); //enable i2c peripheral 
    if (BusyI2C2()) return(1); //i2c bus not ready
    PMP_init();
    LCD_init();
     
    //create i2c frame
    SlaveAddress=0x50;
    i2cdata[0]=((SlaveAddress <<1) | 0); //control byte
    i2cdata[1]=0x03; //memory address (MSB)
    i2cdata[2] =0x41; //Memory Address (LSB)
    i2cdata[3]=TEST_VAL; //single data byte
    
    //send i2c frame to EEPROM
    //write
    datasz=4;
    index=0;
    StartI2C2(); //non-block
    IdleI2C2(); //block
    while(datasz--)
        write_err |=MasterWriteI2C2(i2cdata[index++]);
    StopI2C2();
    IdleI2C2();
    
    if(write_err) return(1);
    
    //poll EEPROM for write completion
    //poll
    StartI2C2();
    IdleI2C2();
    while(MasterWriteI2C2(i2cdata[0])){
        RestartI2C2();
        IdleI2C2();
    }
    StopI2C2();
    IdleI2C2();
    
    //initiate read from EEPROM
    datasz=3;
    index=0;
    StartI2C2();
    IdleI2C2();
    while(datasz--)
        MasterWriteI2C2(i2cdata[index++]);
    RestartI2C2();
    IdleI2C2(); 
    MasterWriteI2C2((SlaveAddress<<1)|1);
    i2cbyte=MasterReadI2C2();
    NotAckI2C2(); //end read with nack
    IdleI2C2(); 
    StopI2C2();
    IdleI2C2();   
    
    //compare
    if(i2cbyte==TEST_VAL)
    {
        PORTSetBits(IOPORT_G, LED4);
        LCD_puts("TEST PASSED\n");  
        LCD_putc(i2cbyte);
    }
    else
    { 
        PORTSetBits(IOPORT_G, LED1);
        LCD_puts("TEST FAILED\n");
        LCD_putc(i2cbyte);
    }
    while(1);
    return (1);
}