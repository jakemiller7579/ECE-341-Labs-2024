#include <plib.h>
#include "CerebotMX7cK.h"
#include "LCDLIB.h"
#include "I2C_EEPROM_LIB.h"
unsigned char read[200];
unsigned char write[200];

void compare()
{
    int i;
    int testpass=0;
    
    for (i=0;i<200;i++)
    {
        if(write[i]==read[i])
        {
            testpass++;
        }
        else if(write[i]!=read[i])
        {
        }
        if (testpass==199)
        {
            PORTSetBits(IOPORT_G, LED1);
            LCD_puts("TEST PASSED\n");
        }
        else 
        {
            PORTSetBits(IOPORT_G, LED2);
            LCD_puts("TEST FAILED\n");
        }
    }
}

int I2CWriteEEPROM(int SlaveAddress, unsigned int mem_addr, char *i2cData, int len)
{
    int j;
    for(j=0;j<200;j++)
    {
        write[j]=0; //initialize global 
    }
    unsigned int write_err;
    unsigned int memMSB=(mem_addr) &~ (BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
    memMSB=memMSB>>8;
    unsigned int memLSB=(mem_addr) &~ (BIT_15 | BIT_14 | BIT_13 | BIT_12 | BIT_11 | BIT_10 | BIT_9 | BIT_8);
    StartI2C2(); //non-block --Start symbol
    IdleI2C2(); //block
    unsigned int controlbyte=((SlaveAddress <<1) | 0); //control byte
    MasterWriteI2C2(controlbyte);
    MasterWriteI2C2(memMSB);
    MasterWriteI2C2(memLSB);
    unsigned int pagetrack;
    int i=0;
    //int numpages=mem_addr;
    while(1)
    {
        write[i]=MasterWriteI2C2(*i2cData);
        i2cData++;
        i++;
        len--;
        mem_addr++;
        //numpages++;
        if (len==0)
        {
            //stop
            StopI2C2();
            IdleI2C2();   
            //wait
            wait_i2c_xfer(SlaveAddress);
            //stop
            StopI2C2();
            IdleI2C2();    
            return 0; //exits function
        }
        pagetrack=mem_addr & (BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
        if ((pagetrack%64)==0) //is address divisible by 64?
        {
            memMSB=(mem_addr) &~ (BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
            memMSB=memMSB>>8;
            memLSB=(mem_addr) &~ (BIT_15 | BIT_14 | BIT_13 | BIT_12 | BIT_11 | BIT_10 | BIT_9 | BIT_8);
            //stop
            StopI2C2();
            IdleI2C2();  
            //wait
            wait_i2c_xfer(SlaveAddress);  
            //controlbyte=((SlaveAddress <<1) | 0);
           // MasterWriteI2C2(controlbyte);
            MasterWriteI2C2(memMSB);
            MasterWriteI2C2(memLSB);
        }
    }
}

int I2CReadEEPROM(int SlaveAddress, int mem_addr, char *i2cData, int len)
{
    int j;
    for(j=0;j<200;j++)
    {
        read[j]=0; //initialize global 
    }
    //clear values
    unsigned int memMSB=mem_addr & ~((BIT_7 |BIT_6 |BIT_5 |BIT_4 |BIT_3 | BIT_2 | BIT_1 | BIT_0));
    memMSB=memMSB>>8; //shift new values right
    //clear values
    unsigned int memLSB=mem_addr & ~((BIT_15 |BIT_14 |BIT_13 |BIT_12 |BIT_11 | BIT_10 | BIT_9 | BIT_8));
    unsigned int controlbyte=((SlaveAddress <<1) | 0); //control byte
    StartI2C2(); //non-block --Start symbol
    IdleI2C2(); //block

    MasterWriteI2C2(controlbyte);
    MasterWriteI2C2(memMSB);
    MasterWriteI2C2(memLSB);
    RestartI2C2();
    IdleI2C2();
    controlbyte=((SlaveAddress <<1) | 1);
    int i=0;
    MasterWriteI2C2(controlbyte); //Read control byte
    while(len--)
    {
        //MasterWriteI2C2(*i2cData);
            //read[i]=
        MasterReadI2C2();
           // i++;
        //mem_addr++; 
        //i2cData++;
        if (len>0)
       {

            AckI2C2();
            IdleI2C2();
        }

//        if (!(0x0000<=(i2cData)<=0x7FFF))
//        {
//            return 1; //invalid memory address
//        }
//        if (*i2cData=='\0')
//        {
//            return 2; //null pointer
//        }
//        if((len>200) | (len<0))
//        {
//            return 3; //invalid len arg
//        }
        //I2C bus error  
    }

            NotAckI2C2();
            IdleI2C2();
            StopI2C2();
            IdleI2C2();
            //return 0;



} 
    
//int wait_i2c_xfer(int SlaveAddress)
//{
//    unsigned int controlbyte=((SlaveAddress <<1) | 0);
//    StartI2C2();
//    IdleI2C2();
////    while(MasterWriteI2C2(controlbyte))
////    {
////        RestartI2C2();
////        IdleI2C2(); 
////    }
////    StopI2C2();
////    IdleI2C2();
////}
//
//    while(1)
//    {
//        MasterWriteI2C2(controlbyte);
//        //* TRUE    - If the receiver ACK'd the byte
//        //* FALSE   - If the receiver NAK'd the byte
//        if (I2CTransmissionHasCompleted(I2C2))
//        {
//            if (!I2CByteWasAcknowledged(I2C2)) //NACK
//            {
//                // transmission successful
//                 return 0;
//            }
//        }
////        if(!I2CByteWasAcknowledged(I2C2)) //NAK'd the byte
////        {
////            return 0;
////        }
//        RestartI2C2();
//        IdleI2C2();   
//    }
//}

int wait_i2c_xfer(int SlaveAddress)
{
    unsigned int controlbyte=((SlaveAddress <<1) | 0);
    StartI2C2();
    IdleI2C2();
    //int ack;
    //while(1)
   // {
        //ack=I2CByteWasAcknowledged(0x50);
        
        while(MasterWriteI2C2(controlbyte))//if(ack==0) //NACK
        {
            RestartI2C2();
            IdleI2C2();
        }
        StopI2C2();
        IdleI2C2();
    //}
}

char BusyI2C2(void)
{
    return(I2C2CONbits.SEN || I2C2CONbits.PEN || I2C2CONbits.RSEN ||
            I2C2CONbits.RCEN || I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);
}//return non-zero value if I2C controller is busy

void init_EEPROM(int SCK_FREQ)
{
    unsigned int BRG_VAL=((FPB/2/SCK_FREQ)-2); 
    OpenI2C2(I2C_EN, BRG_VAL); //enable i2c peripheral 
}