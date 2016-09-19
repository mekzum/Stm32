


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
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"
unsigned int count_;
void delay_ms (int);
void LcdInit (void);
void LcdWriteInt (int , int );
void DmaInit (void);
void RandomInit (void);
int RandomCreate ();
int debug_c1;
GPIO_InitTypeDef buton;

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
//	GPIOA->OSPEEDR = 0xFFFFFFFF;

}
void buton_init()
{
	 //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	buton.GPIO_Pin=GPIO_Pin_0;
	buton.GPIO_Mode=GPIO_Mode_IN;
	GPIO_Init(GPIOA,&buton);
		
}

void initpwm()
{ 
	 // RCC_APB1PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	RCC->APB2ENR|=0x40000; //tm

	GPIOF->MODER|=0x0008000; //f7 ALTERNATIF FONK TANIMLANDI
	GPIOF->OSPEEDR|=0x0008000; //f7 50 MHZ CIKIS SECILDI
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource7,GPIO_AF_TIM11);
	//TIM11->CR1|=0X0080;    //CLOCK DIVISION 0 ,AUTORELOAD AKTIF
	TIM11->PSC=167;
	TIM11->ARR=19999;
  TIM11->CCR1=1000;  //50 hz %20 DOLULUK
	TIM11->CCMR1=0x0000006C;  //Ch1 AYARLARI YAPILDI
	TIM11->CCER=0x0001;
	TIM11->CR1|=0X0001;
	
}

 

 


int main()
{
initpwm();
int pwm;
//LcdInit();
SysTickInit();

TIM11-> CCR1=2200;
    int aci;
  TIM11-> CCR1=19000;
  aci=90;
	 static uint32_t count=0;
    while(1)
    {
  count=TIM11->CNT; //counter debug
	pwm=(aci*7)+790;
	TIM11-> CCR1=pwm;
	delay_ms(100);
  aci++; 
		
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


