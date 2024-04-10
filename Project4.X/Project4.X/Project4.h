//function declarations
void system_init (void);
unsigned int read_buttons(void);
void decode_buttons(unsigned int buttons, unsigned int *step_delay, unsigned int *dir, unsigned int *mode);
unsigned int sw_fsm(unsigned int dir, unsigned int mode);
void output_sm_code(unsigned int sm_code);
void sw_msDelay(unsigned int step_delay);
void Timer1_delay(unsigned int flagdelay,unsigned int &button_delay, unsigned int &step_delay);

//Clockwise and CounterClockwise for direction (dir)
#define CW 0
#define CCW 1

//Half step and Full step for mode
#define HS 0
#define FS 1

//Software Delay 
#define COUNTS_PER_MS 8891
//HS is half the time of full step
#define HSDelay 20
#define FSDelay 40

//Button Delay, initial step delay
#define btndelay 100
#define stpdelay 20
//RPM in steps
#define RPM15 20
#define RPM10 0
#define RPM25 0

//Timer 1 defines
#define T1_PRESCALE 1
#define TOGGLES_PER_SEC 1000
#define T1_TICK (FPB/T1_PRESCALE/TOGGLES_PER_SEC)