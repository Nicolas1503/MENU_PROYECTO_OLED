#include <Arduino.h>
#include "SPI.h"
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_SDA 21
#define OLED_SCL 22

#define ROWNUM 64
#define COLNUM 128

void display_begin(void);
void lcd_ClearOneLine(int row);
void lcd_ClearCursor(int row);
void setMenuDisplay(String name, String arr);
void setConfigDisplayParam(String param_title, float param_value, String param_unit);
void setConfigDisplay(void);
void display_clear(void);
void display_ShowPeriod(int periodo);
void display_ShowDotiter(void);
void display_ShowSpeed(int cuenta, float velocidad );
void display_ShowHeliceSelected(String title_helice, float value_A, float value_B);