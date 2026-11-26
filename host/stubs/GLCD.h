#ifndef HOST_GLCD_H
#define HOST_GLCD_H

#define Black     0x0000
#define Navy      0x000F
#define DarkGreen 0x03E0
#define DarkCyan  0x03EF
#define Maroon    0x7800
#define Purple    0x780F
#define Olive     0x7BE0
#define LightGrey 0xC618
#define DarkGrey  0x7BEF
#define Blue      0x001F
#define Green     0x07E0
#define Cyan      0x07FF
#define Red       0xF800
#define Magenta   0xF81F
#define Yellow    0xFFE0
#define White     0xFFFF

void GLCD_Init(void);
void GLCD_SetTextColor(unsigned short color);
void GLCD_SetBackColor(unsigned short color);
void GLCD_Clear(unsigned short color);
void GLCD_DisplayString(unsigned int ln, unsigned int col, unsigned char fi, unsigned char *s);
void GLCD_Bargraph(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int val);
void GLCD_Bitmap(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char *bitmap);

#endif
