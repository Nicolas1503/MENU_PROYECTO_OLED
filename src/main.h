#ifndef MAIN_H
#define MAIN_H


#include <Arduino.h>
#include "button.hpp"
#include "SPI.h"
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <Preferences.h> 


#define MAXITEMS 20						//Cantidad de posiciones en el display
#define ROWNUM 64
#define COLNUM 128

#define OLED_SDA 21
#define OLED_SCL 22

#define MAINMENU_NUM 3					//Cantidad de chars en el mainmenu
#define AJUSTES_NUM 6					//Cantidad de chars en el menu ajustes
#define MEDICION_NUM 2					//Cantidad de chars en el menu medicion
#define HELICE_NUM 4
#define CONSTANTE1_NUM 3
#define CONSTANTE2_NUM 3
#define CONSTANTE3_NUM 3

/*Matriz de dos dimensiones para definir los menues*/
const char menu[MAINMENU_NUM][MAXITEMS] = {"  Ajustes"," Medicion","  Ultimas   Medidas"};
const char ajustes[AJUSTES_NUM][MAXITEMS] = {"  Config.   Helices","  Config.   Periodo","  Ref. de    Lugar","  Fecha y     Hora","  Buzzer","  Atras"};
const char medicion[MEDICION_NUM][MAXITEMS] = {"  Inicio","  Atras"};
const char helice[HELICE_NUM][MAXITEMS] = {"Helice 1", "Helice 2", "Helice 3", "Atras"};
const char constante1[CONSTANTE1_NUM][MAXITEMS] = {"Valor A", "Valor B", "Atras"};
const char constante2[CONSTANTE2_NUM][MAXITEMS] = {"Valor A", "Valor B", "Atras"};
const char constante3[CONSTANTE3_NUM][MAXITEMS] = {"Valor A", "Valor B", "Atras"};

/*Definicion de menues, submenues y acciones*/

#define	OUT 1
#define AJUSTES 2
#define MEDICION 3
#define ULT_MEDIDAS 4
#define CFG_HELICES 5
#define CFG_PERIODO 6
#define REF_LUGAR 7
#define CFG_DATE 8
#define BUZZER 9
#define ATRAS_AJUSTES 10
#define INICIO_MEDICION 11
#define ATRAS_MEDICION 12
#define TOMAR_MEDICION 13
#define TOMAR_PERIODO 14
#define HELICE_1 15
#define HELICE_2 16
#define HELICE_3 17
#define ATRAS_HELICE 18
#define VALOR_A1 19
#define VALOR_B1 20
#define ATRAS_HELICE_1 21
#define SET_VALOR_A1 22	
#define SET_VALOR_B1 23
#define VALOR_A2 24
#define VALOR_B2 25
#define ATRAS_HELICE_2 26
#define SET_VALOR_A2 27	
#define SET_VALOR_B2 28
#define VALOR_A3 29
#define VALOR_B3 30
#define ATRAS_HELICE_3 31
#define SET_VALOR_A3 32	
#define SET_VALOR_B3 33

int PERIODO_MEMO;


/*Definicion de ubicacion dentro del menu*/
typedef enum{
	MAIN,
	AJUSTES_SUBMENU,
	MEDICION_SUBMENU,
	HELICE_SUBMENU,
	HELICE_1_SUBMENU,
	HELICE_2_SUBMENU,
	HELICE_3_SUBMENU

}Menu_state_e;

/*No se usa todavia VER*/
typedef enum{
    IN = -1,
    ROW_1 = 0,
    ROW_2 = 1,
    OUT_ROW = 2
}row_t;

/*Definicion de lo que sale de la funcion que chequea los botones*/
typedef enum{
	DONTMOVE = 0,
	UP = 1,
	DOWN = 2,
	ENTER = 3
}move_t;

/*Definicion de pines*/
const bool pullup = true;
const int up_button = 5;
const int down_button = 18;
const int enter_button = 19;
move_t buttonProcess = DONTMOVE;

/*Botones como pull up*/
Button Up(up_button,pullup);					
Button Down(down_button,pullup);
Button Enter(enter_button,pullup);

uint8_t estado_actual;
uint8_t estado_anterior;
Menu_state_e menu_submenu_state;
row_t ROW_STATUS;

const int gpio_helice = 34; // Cambiar esto por el valor correcto
int contador_helice = 0;

move_t CheckButton(void);
bool lcd_UpdateCursor(uint8_t Menu, int row, int col);
void lcd_ClearOneLine(int row);
void lcd_ClearCursor(int row);
void lcd_DisplayMenu(uint8_t Menu, Menu_state_e menu_submenu_state);
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count);
void IRAM_ATTR isr_helice();
void setConfigDisplay(void);
void setMenuDisplay(String name, String arr);
void setConfigDisplayParam(String param_title);
bool StateMachine_Control(uint8_t Menu, Menu_state_e menu_submenu_state);



#endif // MAIN_H