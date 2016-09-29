#include "STM32F4xx.h"  
#include "delay.c"  

//B0 Sut1
//B1 Sut2
//B2 Sut3

//B3 Sat1
//B4 Sat2
//B5 Sat3
//B6 Sat4

uint8_t receive; 
unsigned char WAdr,RAdr;
char RxBuf[128];
void NumpadInit()
{
	

GPIO_InitTypeDef GPIO_InitStructure;

RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13| GPIO_Pin_15; 
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
GPIO_Init(GPIOE, &GPIO_InitStructure);


GPIO_InitStructure.GPIO_Pin =GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
GPIO_Init(GPIOB, &GPIO_InitStructure);
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
int main() 
{ 
SysTickInit();
NumpadInit();
UsartInit();
	int i=0;
 while(1) {
GPIOE->BSRRL =0;
GPIOE->BSRRL = GPIO_Pin_9;//set bit as high
GPIOE->BSRRH = GPIO_Pin_11;//set bit as low 
GPIOE->BSRRH = GPIO_Pin_13;//set bit as low
GPIOE->BSRRH = GPIO_Pin_15;
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)) {delay(20);  tus=1 ;  }
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13))  {delay(20);  tus=2 ; }
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15))  {delay(20);  tus=3 ; }


GPIOE->BSRRL =0;
GPIOE->BSRRH = GPIO_Pin_9;//set bit as high
GPIOE->BSRRL = GPIO_Pin_11;//set bit as low 
GPIOE->BSRRH = GPIO_Pin_13;//set bit as low
GPIOE->BSRRH = GPIO_Pin_15;

if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)) {delay(20);  tus=4 ;  }
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13))  {delay(20);  tus=5 ; }
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15))  {delay(20);  tus=6 ; }

GPIOE->BSRRL =0;
GPIOE->BSRRH = GPIO_Pin_9;//set bit as high
GPIOE->BSRRH = GPIO_Pin_11;//set bit as low 
GPIOE->BSRRL = GPIO_Pin_13;//set bit as low
GPIOE->BSRRH = GPIO_Pin_15;

if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11)) {delay(20);  tus=7 ;  }
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13))  {delay(20);  tus=8 ; }
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15))  {delay(20);  tus=9 ; }

GPIOE->BSRRL =0;
GPIOE->BSRRH = GPIO_Pin_9;//set bit as high
GPIOE->BSRRH = GPIO_Pin_11;//set bit as low 
GPIOE->BSRRH = GPIO_Pin_13;//set bit as low
GPIOE->BSRRL = GPIO_Pin_15;

if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11))  {delay(20);  tus='*' ;  }
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13))  {delay(20);  tus=0 ; }
if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15))  {delay(20);  tus='#' ; }
  

if(tus_control!=tus){
	SendChar(tus);		
	tus_control=tus;
}
		}
	}