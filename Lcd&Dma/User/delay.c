#define STK_CTRL (*((volatile unsigned int*) 0xE000E010)) 
#define STK_LOAD (*((volatile unsigned int*) 0xE000E014)) 
#define STK_VAL   (*((volatile unsigned int*) 0xE000E018))
	
 unsigned int count_;
 void delay(int ms);


void SysTickInit()
{
     STK_LOAD= 0X00029040; // 168E6/1000/2=168000 TICK TIME 1MS
     STK_CTRL&=~0x00010000;   // Count Flagi silelim,
     STK_CTRL|= 0x00000007;   // int enb ve count enb yapalim, clk kaynagini AHB yapalim 
}
void delay(int ms)
{
	while(count_<ms)
	{
	
	}
	count_=0;
	
}
void SysTick_Handler()
{
count_++;
STK_CTRL&= ~0x00010000; // Count Flagi silelim

}