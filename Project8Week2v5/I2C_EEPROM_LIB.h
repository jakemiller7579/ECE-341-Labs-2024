#define Fsck 400000
//#define BRG_VAL ((FPB/2/Fsck)-2)
#define TEST_VAL 0X41 //hex for A 41
void system_init(void);
char BusyI2C2();
void init_EEPROM(int);
int I2CReadEEPROM(int SlaveAddress, int mem_addr, char *i2cData, int len);
void compare(char *read, char *write, int len);
int wait_i2c_xfer(int SlaveAddress);
int I2CWriteEEPROM(int SlaveAddress, unsigned int mem_addr, char *i2cData, int len);