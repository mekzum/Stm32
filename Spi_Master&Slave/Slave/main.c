


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
#include "functions.h"
unsigned int count_;
void delay_ms (int);
void LcdInit (void);
void LcdWriteInt (int , int );

int debug_c1;


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
void SpiInit()
{
GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	// enable clock for used IO pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	/* configure pins used by SPI1
	 * PA5 = SCK
	 * PA6 = MISO
	 * PA7 = MOSI
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	// connect SPI1 pins to SPI alternate function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	// enable clock for used IO pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	/* Configure the chip select pin
	   in this case we will use PE7 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	

	
	// enable peripheral clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	/* configure SPI1 in Mode 0 
	 * CPOL = 0 --> clock is low when idle
	 * CPHA = 0 --> data is sampled at the first edge
	 */
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
	SPI_InitStruct.SPI_Mode = SPI_Mode_Slave;     // transmit in master mode, NSS pin has to be always high
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;        // clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; // set the NSS management to internal and pull internal NSS high
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // SPI frequency is APB2 frequency / 4
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted MSB first
	SPI_Init(SPI1, &SPI_InitStruct); 
	//SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_I2S_ITConfig(SPI1, SPI_I2S_IT_RXNE, ENABLE);    
	SPI_Cmd(SPI1, ENABLE); // enable SPI1
	
	NVIC->ISER[1]|=0x8; //spi1 int. enable
}
uint8_t SPI1_send(uint8_t data){

	SPI1->DR = data; // write data to be transmitted to the SPI data register
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
	while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
	while( SPI1->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
	return SPI1->DR; // return received data from SPI data register
}
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
	RCC->AHB1ENR |= 0x0000007F;		// GPIO A,B,C,D,F,G clock'u aktif edelim  
	GPIOG->MODER |= 0x14000000;		// G13,14 output 
	GPIOG->OSPEEDR = 0xFFFFFFFF;
	GPIOD->MODER = 0x55550000;		// GPIOD nin 15, 14, 13, 12, 11, 10, 9, 8 pinleri cikis tanimlandi (LEDler icin) 
	GPIOD->OSPEEDR = 0xFFFFFFFF;	// GPIOD nin tum cikislari en yuksek hizda kullanacagiz  
	
	//GPIOA->MODER |= 0x00000000;    //A0 input mode (önemsiz)
//	GPIOA->OSPEEDR = 0xFFFFFFFF;

}

uint8_t data_spi;
int cont;
  volatile uint8_t aRxBuffer[3];  //Spi1 data buffer
  volatile uint8_t bRxBuffer[3];  //Spi2 data buffer
extern  __IO uint8_t cRxBuffer[];  //Spi3 data buffer
 volatile uint8_t aRxIndex; //Spi1 data buffer index
extern __IO uint8_t bRxIndex; //Spi2 data buffer index
extern __IO uint8_t cRxIndex; //Spi3 data buffer index
 void SPI1_IRQHandler()
 {
	 
	 if(!(GPIOE->IDR&0x80)){ //Spi1 data PinE7
		  if (aRxIndex < 3)  // buffer lengt 3 byte
    {
		aRxBuffer[aRxIndex++]=SPI1->DR;   
	  cont++; //for debug
		 }
		else
			aRxIndex=0;
	                      }
	 
 }



int main()
{

SpiInit();
    while(1)
    {
//SPI1->DR = 0xDC;
   
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


