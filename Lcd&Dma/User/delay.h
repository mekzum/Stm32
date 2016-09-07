
#define STK_CTRL (*((volatile unsigned int*) 0xE000E010)) 
#define STK_LOAD (*((volatile unsigned int*) 0xE000E014)) 
#define STK_VAL   (*((volatile unsigned int*) 0xE000E018))
	
 unsigned int count_;
 void delay(int ms);
void SysTickInit(void);