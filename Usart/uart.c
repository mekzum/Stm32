


#define STK_CTRL	(*((volatile unsigned int *) 0xE000E010))
#define STK_LOAD	(*((volatile unsigned int *) 0xE000E014))
#define STK_VAL		(*((volatile unsigned int *) 0xE000E018))

#include <stdio.h>	 
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f4xx_spi.h"
unsigned int count_;
void delay_ms (int);
void LcdInit (void);
void LcdWriteInt (int line, int number);
void DmaInit (void);
void RandomInit (void);
int RandomCreate ();
int debug_c1;

char x,y,z;
uint16_t x_data,y_data,z_data;
uint8_t receive; 
unsigned char WAdr,RAdr;
char RxBuf[128];
void SysTickInit ()
{
	STK_LOAD = 0X00029040;		// 168E6/1000/2=168000 TICK TIME 1MS 
	STK_CTRL &= ~0x00010000;	// Count Flagi silelim, 
	STK_CTRL |= 0x00000007;		// int enb ve count enb yapalim, clk kaynagini AHB yapalim  
}
void SysTick_Handler ()
{
	count_++;
	STK_CTRL &= ~0x00010000;	// Count Flagi silelim 
	
}
void delay_ms (int ms)
{
	while(count_ < ms);
	count_ = 0;
	
}
char Source[1024];
char Target[1024];
void SystemInit ()
{
	unsigned int i;
	(*((int *) 0xE000ED88)) |= 0x0F00000; // Register write enable 
	for(i = 0; i < 0x00100000; i++);

	// OSC oturtma ve kurtarma rutini 
	RCC->CFGR |= 0x00009400;		// AHB ve APB hizlarini max degerlere set edelim 
	RCC->CR |= 0x00010000;			// HSE Xtal osc calismaya baslasin  
	while(!(RCC->CR & 0x00020000));

	// Xtal osc stabil hale gelsin 
	RCC->PLLCFGR = 0x07402A04;		// PLL katsayilarini M=4, N=150, P=2 ve Q=7 yapalim 150 Mhz  
	RCC->CR |= 0x01000000;			// PLL calismaya baslasin (Rehber Sayfa 95) 
	while(!(RCC->CR & 0x02000000));

	// Pll hazir oluncaya kadar bekle 
	FLASH->ACR = 0x00000605;		// Flash ROM icin 5 Wait state secelim ve ART yi aktif edelim (Rehber Sayfa 55) 
	RCC->CFGR |= 0x00000002;		// Sistem Clk u PLL uzerinden besleyelim 
	while((RCC->CFGR & 0x0000000F) != 0x0000000A);

	// Besleninceye kadar bekle 
	RCC->AHB1ENR |= 0x0000006F;		// GPIO A,B,C,D,F,G clock'u aktif edelim  
	GPIOG->MODER |= 0x14000000;		// G13,14 output 
	GPIOG->OSPEEDR = 0xFFFFFFFF;
	GPIOD->MODER = 0x55550000;		// GPIOD nin 15, 14, 13, 12, 11, 10, 9, 8 pinleri cikis tanimlandi (LEDler icin) 
	GPIOD->OSPEEDR = 0xFFFFFFFF;	// GPIOD nin tum cikislari en yuksek hizda kullanacagiz  
	//GPIOA->MODER |= 0x00000000;    //A0 input mode (önemsiz)
	//GPIOA->OSPEEDR = 0xFFFFFFFF;

}

 void UsartInit()
 {
//Usart clock enable edilecek. Rx ve Tx pinleri alternate func olarak ayarlanacak.
//Usart registerlari ayarlanacak.	 
  RCC->APB2ENR|=0x10;  // UART1 Clock enable
  RCC->APB2RSTR|=0x10;  
  GPIOA->MODER|=0x280000;	  //PA0... PA4 alternate fun.
  GPIOA->AFR[1]|=	0x770; //Alternate 
	RCC->APB2RSTR&=~0x10;
	USART1->BRR=0x222E; //Baudrate 9600
	USART1->CR1|=0x000202C; //Usart enable, interrtups enable
	
	NVIC->ISER[1]|=0x20; //Usart1 int. enable


 }	 

 
void USART1_IRQHandler()
{
static int aba=0;
 	LcdWriteInt(220,aba++);
	volatile int Sts;
     Sts=USART1->SR;
     RxBuf[WAdr]=USART1->DR;
     WAdr=(WAdr+1)&0x7F;
	LCD_DisplayStringLine(280,RxBuf);
}
 
void SendChar(int Tx) 
{
      while(!(USART1->SR&0x80));  // TX Buffer dolu ise bekle (Rehber Sayfa 646)
      USART1->DR=Tx;
}
 
void SendTxt(char *Adr)
{
      while(*Adr) 
        {
          SendChar(*Adr);
          Adr++;
        }  
}
 
char DataReady()
{
       return(WAdr-RAdr);
} 
 
char ReadChar()
{
char Dat;
    
      Dat=RxBuf[RAdr];
      RAdr=(RAdr+1)&0x7F;
      return(Dat);
}
 
// Rx ve TX pinlerini (GPIOB10 ve GPIOB11) birbirine baglarsaniz gonderdiginiz datalar geri gelecektir
 
int main()
{
int i,a=0;
UsartInit();
	LcdInit();

     // SendTxt("PicProje");
 
      while(1){
		 SendChar(0xA);
//		for(i = 0; i < 0x9111111; i++);
				LcdWriteInt(200,a++);
			}
}


void LcdInit ()
{

	// LCD initialization 
	LCD_Init();

	// LCD Layer initialization 
	LCD_LayerInit();

	// Enable the LTDC 
	LTDC_Cmd(ENABLE);

	// Set LCD foreground layer 
	LCD_SetLayer(LCD_FOREGROUND_LAYER);
	LCD_SetColors(LCD_COLOR_RED, LCD_COLOR_WHITE);
	LCD_Clear(LCD_COLOR_WHITE);
	LCD_SetTextColor(LCD_COLOR_RED);
	LCD_DisplayStringLine(0, " Oguz Emre");
	LCD_SetTextColor(LCD_COLOR_GREEN);
	LCD_DisplayStringLine(24, "oguzemre.net");
	LCD_SetTextColor(LCD_COLOR_BLUE);
	LCD_DisplayStringLine(48, " 2016");
	LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_DisplayStringLine(72, "       C*");
	int i;
			for(i = 0; i < 0x1111111; i++);
	
}
void LcdWriteInt (int line, int number)
{
 int num=(int)number;

	uint8_t text[50];
	sprintf((char *) text, "%d", num);		// a int text charina geliyor 
	LCD_DisplayStringLine(line,text);			// text yazdiyorz
	
}


