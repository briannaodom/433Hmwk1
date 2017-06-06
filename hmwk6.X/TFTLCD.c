#include <xc.h>
#include "TFTLCD.h"
#include "ILI9163C.h"

void LCD_drawCharacter(short x, short y, char character, short color) {
    int i;
    int j;

    if ((x < (124 - MARGIN)) && (y < (122 - MARGIN))) { // check to make sure character is in range
        for (i = 0; i < 5; i++) {
            for (j = 0; j < + 8; j++) {
                if (((ASCII[character - 0x20][i]) >> j) & 1) {
                    LCD_drawPixel(x + i, y + j, color);
                }
                else {
                    LCD_drawPixel(x + i, y + j, BACKGROUND);
                }
            }
        }
    }
}

void LCD_drawString(short x, short y, char *message, short color) {
    int k = 0;

    while(message[k]){
        LCD_drawCharacter((x + 6*k), y, message[k], color);
        k++;
    }
}

void LCD_progressBar(char x, char y, char percentage, short doneColor, short remainingColor) {
    int barX, barY;
    for (barX=0; barX<101; barX++) {
        if (barX<percentage) {
            for (barY=0; barY<5; barY++) {
                LCD_drawPixel(x+barX,y+barY,doneColor);
            }
        }
        else {
            for (barY=0; barY<5; barY++) {
                LCD_drawPixel(x+barX,y+barY,remainingColor);
            }
        }
    }
}
