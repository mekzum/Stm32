#include "stm32f4xx_conf.h"

#define LCD_CS_PORT                 GPIOC
#define LCD_CS_PIN                  GPIO_Pin_1
#define LCD_DC_PORT                 GPIOC
#define LCD_DC_PIN                  GPIO_Pin_0

#define PAGE0         0
#define PAGE1         1
#define PAGE2         2
#define PAGE3         3
#define PAGE4         4
#define PAGE5         5
#define PAGE6         6
#define PAGE7         7
#define FRAME_2       7
#define FRAME_3       4
#define FRAME_4       5
#define FRAME_5       0
#define FRAME_25      6
#define FRAME_64      1
#define FRAME_128     2
#define FRAME_256     3

const uint8_t *font, *font2;
uint8_t width, height, min, max;

void Init_OLED(void);
void Activate_Scroll(void);
void Deactivate_Scroll(void);
void Oled_Command(uint8_t cmd);
void Oled_WriteRam(uint8_t dat);
void Oled_Image(const uint8_t *buffer);
void Oled_FillScreen(uint8_t pattern);
void Oled_SetPointer(uint8_t seg, uint8_t pag);
void Oled_WriteChar(uint8_t c, uint8_t seg, uint8_t pag);
void Oled_Text(char *buffer, uint8_t seg, uint8_t pag);
void Oled_ConstText(const char *buffer, uint8_t seg, uint8_t pag);
void Left_HorizontalScroll(uint8_t start_page, uint8_t end_page, uint8_t set_time);
void Right_HorizontalScroll(uint8_t start_page, uint8_t end_page, uint8_t set_time);
void VerticalLeft_HorizontalScroll(uint8_t start_page, uint8_t end_page, uint8_t set_time);
void VerticalRight_HorizontalScroll(uint8_t start_page, uint8_t end_page, uint8_t set_time);
void Oled_SetFont(const uint8_t *_font, uint8_t _width, uint8_t _height, uint8_t _min, uint8_t _max);

void Init_OLED(void)
{
   Oled_Command(0xAE);
   Oled_Command(0x81);
   Oled_Command(0xCF);
   Oled_Command(0xA4);
   Oled_Command(0xA6);
   Oled_Command(0x20);
   Oled_Command(0x02);
   Oled_Command(0x00);
   Oled_Command(0x10);
   Oled_Command(0xB0);
   Oled_Command(0x40);
   Oled_Command(0xA1);
   Oled_Command(0xA8);
   Oled_Command(0x3F);
   Oled_Command(0xC8);
   Oled_Command(0xD3);
   Oled_Command(0x00);
   Oled_Command(0xDA);
   Oled_Command(0x12);
   Oled_Command(0xD5);
   Oled_Command(0x80);
   Oled_Command(0xD9);
   Oled_Command(0xF1);
   Oled_Command(0xDB);
   Oled_Command(0x40);
   Oled_Command(0x8D);
   Oled_Command(0x14);
   Oled_Command(0xAF);

   Oled_FillScreen(0x00);
}

void Oled_Command(uint8_t Command)
{
    GPIO_ResetBits(LCD_DC_PORT,LCD_DC_PIN);
    GPIO_ResetBits(LCD_CS_PORT,LCD_CS_PIN);

    SPI_I2S_SendData(SPI1, Command);
    while (SPI1->SR & SPI_SR_BSY);

    GPIO_SetBits(LCD_CS_PORT,LCD_CS_PIN);
}

void Oled_WriteRam(uint8_t Data)
{
    GPIO_ResetBits(LCD_CS_PORT,LCD_CS_PIN);

    GPIO_SetBits(LCD_DC_PORT,LCD_DC_PIN);

    SPI_I2S_SendData(SPI1, Data);
    while (SPI1->SR & SPI_SR_BSY);

    GPIO_SetBits(LCD_CS_PORT,LCD_CS_PIN);
}

 void Oled_SetPointer(uint8_t seg, uint8_t pag)
{
   uint8_t low_column, hig_column;

   low_column = seg & 0xF;
   hig_column = (seg & 0xF0)>>4;
   hig_column = hig_column | 0x10;
   pag = ((pag & 0x07));
   pag = (pag | 0xB0);
   Oled_Command(low_column);
   Oled_Command(hig_column);
   Oled_Command(pag);
}

void Oled_SetFont(const uint8_t *_font, uint8_t _width, uint8_t _height, uint8_t _min, uint8_t _max)
{
   font2  = _font;
   width  = _width;
   height = _height / 8;
   min    = _min;
   max    = _max;
}

void Oled_WriteChar(uint8_t c, uint8_t seg, uint8_t pag)
{
   uint8_t i, j, k, l;
   uint8_t x_seg, y_pag;

   x_seg = seg;
   y_pag = pag;
   font = font2;
   k = c - min;
   l = (width * height);

   for(j = 0; j < k; j++)
   {
       for(i = 0; i < l; i++)
       {
          font++;
       }
   }

   for(i = 0; i < width; i++)
   {
      y_pag = pag;
      for(j = 0; j < height; j++)
      {
         if(x_seg < 128)
         {
            Oled_SetPointer(x_seg, y_pag);
            Oled_WriteRam(*font);
         }
         y_pag++;
         font++;
      }
      x_seg++;
   }
}

void Oled_ConstText(const char *buffer, uint8_t seg, uint8_t pag)
{
   uint8_t x_seg = seg;

   while(*buffer)
   {
      Oled_WriteChar(*buffer, x_seg, pag);
      x_seg = x_seg + width;
      buffer++;
   }
}

void Oled_Text(char *buffer, uint8_t seg, uint8_t pag)
{
   uint8_t x_seg = seg;

   while(*buffer)
   {
      Oled_WriteChar(*buffer, x_seg, pag);
      x_seg = x_seg + width;
      buffer++;
   }
}

void Oled_FillScreen(uint8_t pattern)
{
   unsigned char i, j;

   for(i = 0; i < 8; i++)
   {
      Oled_SetPointer(0, i);
      for(j = 0; j < 128; j++)
      {
         Oled_WriteRam(pattern);
      }
   }
}

void Oled_Image(const uint8_t *buffer)
{
   unsigned char i, j;

   for(i = 0; i < 8; i++)
   {
      Oled_SetPointer(0, i);
      for(j = 0; j < 128; j++)
      {
         Oled_WriteRam(*buffer);
         buffer++;
      }
   }
}

void Right_HorizontalScroll(uint8_t start_page, uint8_t end_page, uint8_t set_time)
{
   Deactivate_Scroll();
   Oled_Command(0x26);
   Oled_Command(0x00);
   Oled_Command(start_page);
   Oled_Command(set_time);
   Oled_Command(end_page);
   Oled_Command(0x00);
   Oled_Command(0xFF);
   Activate_Scroll();
}

void Left_HorizontalScroll(uint8_t start_page, uint8_t end_page, uint8_t set_time)
{
   Deactivate_Scroll();
   Oled_Command(0x27);
   Oled_Command(0x00);
   Oled_Command(start_page);
   Oled_Command(set_time);
   Oled_Command(end_page);
   Oled_Command(0x00);
   Oled_Command(0xFF);
   Activate_Scroll();
}

void VerticalRight_HorizontalScroll(uint8_t start_page, uint8_t end_page, uint8_t set_time)
{
   Deactivate_Scroll();
   Oled_Command(0x29);
   Oled_Command(0x00);
   Oled_Command(start_page);
   Oled_Command(set_time);
   Oled_Command(end_page);
   Oled_Command(0x01);
   Activate_Scroll();
}

void VerticalLeft_HorizontalScroll(uint8_t start_page, uint8_t end_page, uint8_t set_time)
{
   Deactivate_Scroll();
   Oled_Command(0x2A);
   Oled_Command(0x00);
   Oled_Command(start_page);
   Oled_Command(set_time);
   Oled_Command(end_page);
   Oled_Command(0x01);
   Activate_Scroll();
}

void Deactivate_Scroll(void)
{
   Oled_Command(0x2E);
}

void Activate_Scroll(void)
{
   Oled_Command(0x2F);
}