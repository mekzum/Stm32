#include "STM32F4xx.h"
 
void SystemInit()
{
    (*((int*)0xE000ED88))|=0x0F00000;  // Floating Point donanimini aktiflestir. !!! Basimizin derdi !!!
    RCC->AHB1ENR |= 0x00000008;        // GPIOD donaniminin clock sinyalini uygulayalim
    GPIOD->MODER = 0x55000000;         // GPIOD nin 15, 14, 13, 12 pinleri cikis tanimlandi (Ledler bu pinlerde)
    GPIOD->OSPEEDR= 0xFFFFFFFF;        // GPIOD nin tum cikislari en yuksek hizda kullanacagiz 
} 
 
int main()
{
    while(1)
   {
     GPIOD->ODR= 0x0000F000;     // Ledler yansin
     GPIOD->ODR= 0x00000000;     // Ledler sonsun
   } 
}