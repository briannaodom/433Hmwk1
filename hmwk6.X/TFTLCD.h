#ifndef TFTLCD_H__
#define TFTLCD_H__

#define BACKGROUND 0x001F  // background color is blue
#define TEXTCOLOR 0xFFFF   // text color is white
#define DONECOLOR 0x07E0 // progress bar: green = doneColor
#define REMAININGCOLOR 0xF800 // progress bar: red = doneColor
#define MARGIN 1 // 1-pixel margin

void LCD_drawCharacter(short, short, char, short);
void LCD_drawString(short, short, char *, short);
void LCD_progressBar(char, char, char, short, short);

#endif
