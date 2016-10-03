#include "STM32F4xx.h"  
#include "delay.c"  

uint8_t receive; 
unsigned char WAdr,RAdr;
char RxBuf[7];
void data_process(void); //Data islemek için kullanilacak fonksiyon
char array[5]={0x10,0x15,0x25,0x00};
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
	USART2->BRR=0x446; //Baudrate 9600
	USART2->CR1|=0x000202C; //Usart enable, interrtups enable
	NVIC->ISER[1]|=0x40; //Usart2 int. enable

 }	 

 
void USART2_IRQHandler()
{
	volatile unsigned int IIR;
    IIR = USART2->SR;
    if (IIR & USART_FLAG_RXNE) { 
		USART2->SR &= ~USART_FLAG_RXNE;	  
     RxBuf[WAdr]=USART2->DR;
     WAdr=WAdr+1;
			if(6==WAdr){ WAdr=0;
			data_process();				
			}
		}
     
}
 
void SendChar(char Tx) 
{
      while(!(USART2->SR&0x80));  // TX Buffer dolu ise bekle (Rehber Sayfa 646)
      USART2->DR=Tx;
}
 int i=0;
void SendTxt(char *Adr)
{
	
				char checksum=0;
				  for ( i = 0; i < 3; i++)
                {
                    checksum =checksum ^array[i];
                }
								array[3]=checksum;
								
      while(*Adr) 
        {
          SendChar(*Adr);
          Adr++;
        }  
		  // SendChar(checksum);
}


int main() 
{ 

SysTickInit();
UsartInit();



 while(1) {
	  
	 SendTxt(array);
	 delay(1500);
	 delay(1500);
	 
	}
	}

	static int tf=0;
	void data_process(){
		      char checksum=0;
		
				  for ( i = 0; i < 5; i++)
                {
                    checksum =checksum ^RxBuf[i];
                }
		      if(RxBuf[5]==checksum) //veri dogru gelmistir
					{
						tf=1;
						
					}
					else{ //veri yanlis gelmistir
						
						tf=0;
					}
						
		
		
	}