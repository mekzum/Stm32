#include "STM32F4xx.h"  
#include "delay.c"  

uint8_t receive; 
unsigned char WAdr,RAdr;
char RxBuf[128];
void SpiSendData(int spi, char* data);
void SpiInit()
{
GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;
	
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
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 |GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 ;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	//Spi idle high
	GPIOE->BSRRL |= GPIO_Pin_7; // set PE7 high
	GPIOE->BSRRL |= GPIO_Pin_8; 
	GPIOE->BSRRL |= GPIO_Pin_9; 
	// enable peripheral clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	/* configure SPI1 in Mode 0 
	 * CPOL = 0 --> clock is low when idle
	 * CPHA = 0 --> data is sampled at the first edge
	 */
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;        // clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      // data sampled at first edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft ; // set the NSS management to internal and pull internal NSS high
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // SPI frequency is APB2 frequency / 4
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted MSB first
//	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStruct); 
	
	SPI_Cmd(SPI1, ENABLE); // enable SPI1
}
uint8_t SPI1_send(uint8_t data){

	SPI1->DR = data; // write data to be transmitted to the SPI data register
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
	while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
	while( SPI1->SR & SPI_I2S_FLAG_BSY ); // wait until SPI is not busy anymore
	return SPI1->DR; // return received data from SPI data register
}
void SystemInit() 
{ 
unsigned int i;   
       (*((int*)0xE000ED88))|=0x0F00000;  
       for (i=0;i<0x00100000;i++);           // OSC oturtma ve kurtarma rutini 
       RCC->CFGR |= 0x00009400;          // AHB ve APB hizlarini max degerlere set edelim 
       RCC->CR |= 0x00010000;             // HSE Xtal osc calismaya baslasin 
       while (!(RCC->CR & 0x00020000)); // Xtal osc stabil hale gelsin 
       RCC->PLLCFGR = 0x07402A04;       // PLL katsayilarini M=4, N=168, P=2 ve Q=7 yapalim   168 Mhz
       RCC->CR |= 0x01000000;              // PLL calismaya baslasin  (Rehber Sayfa 95) 
       while(!(RCC->CR & 0x02000000));   // Pll hazir oluncaya kadar bekle 
       FLASH->ACR = 0x00000605;          // Flash ROM icin 5 Wait state secelim ve ART yi aktif edelim (Rehber Sayfa 55) 
       RCC->CFGR |= 0x00000002;          // Sistem Clk u PLL uzerinden besleyelim 
       while ((RCC->CFGR & 0x0000000F) != 0x0000000A); // Besleninceye kadar bekle 
       RCC->AHB1ENR |= 0x0000001F;     // GPIO A,B,C,D,E clock'u aktif edelim 
       GPIOD->MODER  = 0x55550000;     // GPIOD nin 15, 14, 13, 12, 11, 10, 9, 8 pinleri cikis tanimlandi (LEDler icin) 
       GPIOD->OSPEEDR= 0xFFFFFFFF;     // GPIOD nin tum cikislari en yuksek hizda kullanacagiz 

}   
void UsartInit()
 {
//Usart clock enable edilecek. Rx ve Tx pinleri alternate func olarak ayarlanacak.
//Usart registerlari ayarlanacak.	 
  RCC->APB1ENR|=0x20000;  // USART2 Clock enable
  RCC->APB1RSTR|=0x20000;  
  GPIOA->MODER|=0xA0;	  //PA2 ve 3 alternate fun.
  GPIOA->AFR[0]|=	0x7700; //Alternate 
	RCC->APB1RSTR&=~0x20000; 
	USART2->BRR=0x1117; //Baudrate 9600
	USART2->CR1|=0x000202C; //Usart enable, interrtups enable
	NVIC->ISER[1]|=0x40; //Usart1 int. enable

 }	 

 
void USART1_IRQHandler()
{
static int aba=0;

	   volatile int Sts;
     Sts=USART2->SR;
     RxBuf[WAdr]=USART2->DR;
     WAdr=(WAdr+1)&0x7F;

}
 
void SendChar(int Tx) 
{
      while(!(USART2->SR&0x80));  // TX Buffer dolu ise bekle (Rehber Sayfa 646)
      USART2->DR=Tx;
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
char tus, tus_control;
uint8_t received_val = 0;

void SpiSendData(int spi, char* data ){
	
		 GPIOE->BSRRH |= GPIO_Pin_7; // set PE7 (CS) low
	   while(*data) 
        {
          
		      SPI1->DR=*data;  // transmit data // transmit dummy byte and receive data
					data++;
					delay(20);
	   
					  }
	   delay(500);
		 GPIOE->BSRRL |= GPIO_Pin_7; // set PE7 (CS) high
	

}







int main() 
{ 

SysTickInit();
UsartInit();
SpiInit();
int i=0;

 while(1) {
 char array[4]={0x10,0x15,0x25};
	  	SpiSendData(1,array);

	 
		}
	}