

#include "delay.h"
void SysTickInit()
{
     STK_LOAD= 0X00029040; // 168E6/1000/2=168000 TICK TIME 1MS
     STK_CTRL&=~0x00010000;   // Count Flagi silelim,
     STK_CTRL|= 0x00000007;   // int enb ve count enb yapalim, clk kaynagini AHB yapalim 
}
void delay(int ms)
{
	while(count<ms)
	{
	
	}
	count=0;
	
}