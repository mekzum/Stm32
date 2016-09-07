


#define STK_CTRL	(*((volatile unsigned int *) 0xE000E010))
#define STK_LOAD	(*((volatile unsigned int *) 0xE000E014))
#define STK_VAL		(*((volatile unsigned int *) 0xE000E018))

#include <stdio.h>	 
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
unsigned int count_;
void delay_ms (int ms);
void LcdInit (void);
void LcdWriteInt (int line, char number);
void DmaInit (void);
void SysTickInit ()
{
	STK_LOAD = 0X00029040;		// 168E6/1000/2=168000 TICK TIME 1MS */
	STK_CTRL &= ~0x00010000;	// Count Flagi silelim, */
	STK_CTRL |= 0x00000007;		// int enb ve count enb yapalim, clk kaynagini AHB yapalim  */
}
void SysTick_Handler ()
{
	count_++;
	STK_CTRL &= ~0x00010000;	// Count Flagi silelim */
	
}
char Source[1024];
char Target[1024];
void SystemInit ()
{
	unsigned int i;
	(*((int *) 0xE000ED88)) |= 0x0F00000; // Register write enable */
	for(i = 0; i < 0x00100000; i++);

	// OSC oturtma ve kurtarma rutini */
	RCC->CFGR |= 0x00009400;		// AHB ve APB hizlarini max degerlere set edelim */
	RCC->CR |= 0x00010000;			// HSE Xtal osc calismaya baslasin  */
	while(!(RCC->CR & 0x00020000));

	// Xtal osc stabil hale gelsin */
	RCC->PLLCFGR = 0x07402A04;		// PLL katsayilarini M=4, N=150, P=2 ve Q=7 yapalim 150 Mhz  */
	RCC->CR |= 0x01000000;			// PLL calismaya baslasin (Rehber Sayfa 95) */
	while(!(RCC->CR & 0x02000000));

	// Pll hazir oluncaya kadar bekle */
	FLASH->ACR = 0x00000605;		// Flash ROM icin 5 Wait state secelim ve ART yi aktif edelim (Rehber Sayfa 55) */
	RCC->CFGR |= 0x00000002;		// Sistem Clk u PLL uzerinden besleyelim */
	while((RCC->CFGR & 0x0000000F) != 0x0000000A);

	// Besleninceye kadar bekle */
	RCC->AHB1ENR |= 0x0000000F;		// GPIO A,B,C,D clock'u aktif edelim  */
	RCC->AHB1ENR |= 0x00000040;		// G portu clok aktif */
	GPIOG->MODER |= 0x14000000;		// G13,14 output */
	GPIOG->OSPEEDR = 0xFFFFFFFF;
	GPIOD->MODER = 0x55550000;		// GPIOD nin 15, 14, 13, 12, 11, 10, 9, 8 pinleri cikis tanimlandi (LEDler icin) */
	GPIOD->OSPEEDR = 0xFFFFFFFF;	// GPIOD nin tum cikislari en yuksek hizda kullanacagiz  */
	
}
int main ()
{
	SysTickInit();
	LcdInit();
	DmaInit();

	volatile int i=0;
	while(1)
	{
		
		GPIOG->ODR = 0x6000; // G13,14 high */
		GPIOD->ODR = 0xF000;	// D12 ,13,14,15 high */
		
		LcdWriteInt(100,Target[i]);
		i++;
		if(i == 1024) i = 0;	// i koruma ve arttirma */
		
		delay_ms(1000);
		GPIOD->ODR = 0;		// D ve G low */
		GPIOG->ODR = 0;
		delay_ms(1000);
		
	}
	
}
void delay_ms (int ms)
{
	while(count_ < ms);
	count_ = 0;
	
}
void LcdInit ()
{

	// LCD initialization */
	LCD_Init();

	// LCD Layer initialization */
	LCD_LayerInit();

	// Enable the LTDC */
	LTDC_Cmd(ENABLE);

	// Set LCD foreground layer */
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
	LCD_DisplayStringLine(72, "C*");
	
}
void LcdWriteInt (int line, char number)
{
 int num=(int)number;

	uint8_t text[50];
	sprintf((char *) text, "%d", num);		// a int text charina geliyor 
	LCD_DisplayStringLine(line,text);			// text yazdiyorz
	
}
void DmaInit ()
{
	volatile int i;
	// Source alanina datalarimizi koyalim */
		for(i = 0; i < 1024; i++) Source[i] = i;
	RCC->AHB1ENR |= 0x00400000; // DMA2 clock'u aktif edelim  */
	while(DMA2_Stream0->CR & 1);

	// DMA 0 kanalinin isi varsa bitmesini bekleyelim */
	DMA2_Stream0->PAR = (int) &Source[0];	// Source array dan datalari alacagiz */
	DMA2_Stream0->M0AR = (int) &Target[0];	// Target array a datalari tasiyacagiz */
	DMA2_Stream0->NDTR = 1024;	//  */
	DMA2_Stream0->CR = 0x00A00680;
	DMA2_Stream0->CR |= 1;		// Transferi baslat */
	
	
	// Artik biz kendi yolumuza DMA kendi yoluna  */
	delay_ms(100);
	while(DMA2_Stream0->CR & 1); // DMA2 0 kanalinin isini bitirmediyse bekleyelim */
}
