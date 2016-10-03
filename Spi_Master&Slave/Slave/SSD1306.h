#ifndef SSD1306_H
#define SSD1306_H
#ifdef __cplusplus
extern "C"
{
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

#endif
#endif /* SSD1306_H_INCLUDED */