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
void display_clear(void);
void lcd_ClearOneLine(int row);
void lcd_ClearCursor(int row);
void display_notif(String NOTIFICACION);
void display_background(bool modo_flechas);
void display_showmenu(String menu_title, String arr);
void display_showparam(String param_title, float param_value, bool entero, String param_unit);
void display_MedicionMode(uint8_t helice_num,int periodo);
void display_ShowDot(uint8_t dot_iter);
void display_ShowSpeed(int cuenta, float velocidad );
void display_ShowHeliceSelected(uint8_t helice_num, float value_A, float value_B);
void display_ShowMedidaGuardada(String file_name);
void display_ShowMedidaEnviada(void);
void display_showFechaHora(String VALOR_HORA, String VALOR_MINUTO, String VALOR_DIA, String VALOR_MES, String VALOR_ANIO);