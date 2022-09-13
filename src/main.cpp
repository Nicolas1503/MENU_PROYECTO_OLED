#include <Arduino.h>
#include "button.hpp"
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define MAXITEMS 20						//Cantidad de posiciones en el display
#define ROWNUM 64
#define COLNUM 128

#define OLED_SDA 21
#define OLED_SCL 22

#define MAINMENU_NUM 3					//Cantidad de chars en el mainmenu
#define AJUSTES_NUM 6					//Cantidad de chars en el menu ajustes
#define MEDICION_NUM 2					//Cantidad de chars en el menu medicion

/*Matriz de dos dimensiones para definir los menues*/
const char menu[MAINMENU_NUM][MAXITEMS] = {"Ajustes","Medicion","Ult. Medidas"};
const char ajustes[AJUSTES_NUM][MAXITEMS] = {"Config.   Helices","Config.   Periodo","Ref. Lugar","Cfg. Date","Buzzer","Atras"};
const char medicion[MEDICION_NUM][MAXITEMS] = {"Inicio","Atras"};

/*Definicion de menues, submenues y acciones*/

#define	OUT 0
#define AJUSTES 1
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


/*Definicion de ubicacion dentro del menu*/
typedef enum{
	MAIN,
	AJUSTES_SUBMENU,
	MEDICION_SUBMENU
}Menu_state_e;

/*Deficinicion de variables a setear*/
typedef enum{
    A_ELISE = 0,
    B_ELISE = 1,
	PERIODO = 2
}address_t;

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
const int up_button = 2;
const int down_button = 4;
const int enter_button = 5;
move_t buttonProcess = DONTMOVE;

/*Botones como pull up*/
Button Up(up_button,pullup);					
Button Down(down_button,pullup);
Button Enter(enter_button,pullup);

uint8_t estado_actual;
uint8_t estado_anterior;
Menu_state_e menu_submenu_state;
row_t ROW_STATUS;


//void SetEEPROMValue(address_t address, float value);
//float GetEEPROMValue(address_t address);
move_t CheckButton(void);
bool lcd_UpdateCursor(uint8_t Menu, int row, int col);
void lcd_ClearOneLine(int row);
void lcd_ClearCursor(int row);
void lcd_DisplayMenu(uint8_t Menu, Menu_state_e menu_submenu_state);
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count, uint8_t cursorPosition);
void StateMachine_Control(uint8_t Menu, Menu_state_e menu_submenu_state);

Adafruit_SH1106 display(OLED_SDA, OLED_SCL);

void setup()
{
  	Wire.begin();         // inicializa bus I2C
 	display.begin(SH1106_SWITCHCAPVCC, 0x3C); // inicializa pantalla con direccion 0x3C
	Serial.begin(9600);

  estado_actual = AJUSTES;				//Estado actual y anterior en ajustes
  estado_anterior = AJUSTES;
  menu_submenu_state = MAIN;			//Menu actual esta en el principal

  //SetEEPROMValue(PERIODO,10000);
  	display.clearDisplay();      // limpia pantalla  
	display.setCursor(0,ROW_STATUS);
	display.setTextSize(2);      // establece tamano de texto en 2
  	display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
 	display.cp437(true);
  	display.write(0x10);
	display.display(); 
	lcd_DisplayMenu(estado_actual, menu_submenu_state);			//Se muestra en el display lo que se declaro
}
void loop(void)								
{
	bool ret = 0;							//Si el booleano es 0 no se hace nada
	while(true)
	{
		ret = lcd_UpdateCursor(estado_actual,ROWNUM,COLNUM);	//Si se apreto algun boton se actualiza la pantalla (actualiza los estados)
		if (ret == 1){
			lcd_DisplayMenu(estado_actual,menu_submenu_state);	
		}
		StateMachine_Control(estado_actual,menu_submenu_state);
	}
}


/*float GetEEPROMValue(address_t address)
{
    float val; 
    float value = EEPROM.get(address, val);
    return value;
}

void SetEEPROMValue(address_t address, float value)
{
    EEPROM.put(address, value);
}*/

/*Funcion que chequea que boton esta presionado*/
move_t CheckButton(void)
{
    if (Up.check() == LOW)
    {
      return UP;
    }
    else if (Down.check() == LOW)
    {
      return DOWN;
    }
    else if (Enter.check() == LOW)
    {
      return ENTER;
    }
    else
    {
        // DONT MOVE
    }
  return DONTMOVE; //Siempre retorna un 0 (no hace nada)excepto que se haya apretado un boton
}

bool lcd_UpdateCursor(uint8_t Menu, int row, int col) //Dentro de esta funcion esta el chekbutton
{
	static move_t lastButtonProcess = DONTMOVE;		//Variables que quedan fijas segun el ultimo llamado de la funcion
	static uint8_t firstMenu = AJUSTES;
	static uint8_t lastMenu = ULT_MEDIDAS;
	static Menu_state_e lastMenuState = MAIN;

	buttonProcess = CheckButton();					//Aqui se recibe, UP, DOWN, ENTER O DONT MOVE

	if (buttonProcess != DONTMOVE)					//Si es distinto de DONTMOVE se hace algo 
	{
		lastButtonProcess = buttonProcess;
		if (buttonProcess == DOWN){					//Si el boton fue DOWN 
			if(estado_actual != lastMenu)			//Si el estado actual es distinto de el ultimo item del menu se suma uno
			{
				estado_actual = estado_actual + OUT;
			}
		}
		else if (buttonProcess == UP){				//Si el boton fue UP 
			if(estado_actual != firstMenu)			//Si el estado actual es distinto del primer item del menu se resta uno
		 	{
				estado_actual = estado_actual - OUT;
			}
		}
		else if (buttonProcess == ENTER)		 	//Si se aprieta el boton enter se chequea en que estado esta
		{
			
			if (lastButtonProcess == DOWN || lastButtonProcess == UP)	
			{
				display.clearDisplay();
			}
			switch(estado_actual)
			{
				case AJUSTES:						
				{
					menu_submenu_state = AJUSTES_SUBMENU;			//Se pasa del MAIMENU al menu de ajustes
					estado_actual = CFG_HELICES;					//Primer estado de este submenu	
				}
				break;
				case ATRAS_AJUSTES:
				{
					menu_submenu_state = MAIN;						//Se pasa del menu de ajustes al MAINMENU
					estado_actual = AJUSTES;						//Primer estado de este menu 
				}
				break;
				case MEDICION:
				{
					menu_submenu_state = MEDICION_SUBMENU;			//Se pasa del MAIMENU al menu de medicion	
					estado_actual = INICIO_MEDICION;				//Primer estado de este menu 
				}
				break;

				case INICIO_MEDICION:
				{
					menu_submenu_state = MEDICION_SUBMENU;  		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = TOMAR_MEDICION;				
				}
				break;

				case ATRAS_MEDICION:
				{
					menu_submenu_state = MAIN;						//Se pasa del menu medicion al MAINMENU
					estado_actual = AJUSTES;						//Primer estado de este menu 
				}
				break;
			}
		}

		/*Definicion de primeros y ultimos elementos en cada menu*/
		switch(menu_submenu_state)					
		{
		case MAIN: 
			firstMenu = AJUSTES;
			lastMenu = ULT_MEDIDAS; 
			break;
		case AJUSTES_SUBMENU: 
			firstMenu = CFG_HELICES;
			lastMenu = ATRAS_AJUSTES; 
			break;
		case MEDICION_SUBMENU:
			firstMenu = INICIO_MEDICION;
			lastMenu = ATRAS_MEDICION;
			break;
		}
		

		Serial.println("Cambio de estado");
		Serial.println(estado_actual);
		return 1;
	}
	return 0;
}

/*Se usa?*/
void lcd_ClearOneLine(int row)
{
	for(uint8_t i=0; i < COLNUM ; i++)
	{
		display.setCursor(i,row);
		display.print(" ");
		display.display(); 
	}
}

/*Se usa?*/
void lcd_ClearCursor(int row)
{
	display.setCursor(0,row);
	display.print(" ");
	display.display(); 
}

void lcd_DisplayMenu(uint8_t Menu, Menu_state_e menu_submenu_state)		//Funcion que imprime las opciones en el display	
{
	switch(Menu)
	{
		case OUT:							//Se define el tope en el enum para no entrar en estados que no existen
			Menu = AJUSTES;
		break;

		case AJUSTES:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,0);	//Donde comienza a mostrar, cuantos voy a mostrar, donde esta el cursor
		}
		break;

		case MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,33);
		}
		break;

		case ULT_MEDIDAS:
		{
			lcd_PrintCursor(menu_submenu_state,2,1,0);
		}
		break;

		case CFG_HELICES:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,0);
		}
		break;

		case CFG_PERIODO:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,33);
		}
		break;
		
		case REF_LUGAR:
		{
			lcd_PrintCursor(menu_submenu_state,2,2,0);
		}
		break;

		case CFG_DATE:
		{
			lcd_PrintCursor(menu_submenu_state,2,2,33);
		}
		break;

		case BUZZER:
		{
			lcd_PrintCursor(menu_submenu_state,4,2,0);
		}
		break;

		case ATRAS_AJUSTES:
		{
			lcd_PrintCursor(menu_submenu_state,4,2,33);
		}
		break;

		case INICIO_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,0);
		}
		break;

		case ATRAS_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,33);
		}
		break;

		case TOMAR_MEDICION:
		{
			//No va a hacer nada en la pantalla 
		}
		break;

		default:
			Menu = AJUSTES;
		break;
	}
}

/* Funcion para imprimir en pantalla*/
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count, uint8_t cursorPosition) //Estados, donde comienza en el array que corresponde
{
	display.clearDisplay();
	display.setCursor(0,cursorPosition);
	display.setTextSize(2);      	// establece tamano de texto en 2
  	display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
  	display.cp437(true);
  	display.write(0x10);
	display.display(); 
	uint8_t cursor = 0;
	if (count <= ROWNUM){
		for (uint8_t i=start; i<count+start; i++)				//Cuenta para mostrar los menu en la pantalla
		{
			display.setCursor(13,cursor);								//Se coloca el cursor en la posicion 12 de columna y la fila 0
			if (cursor==0) {
			cursor = 33;		
			}else {
			cursor = 0;
			}							//Se coloca el cursor en la fila 1 en la siguiente iteracion

			if (menu_submenu_state == MAIN){
				display.setTextSize(2);      // establece tamano de texto en 2
  				display.setTextColor(WHITE);
				display.print(menu[i]);										//Muestra solo el array del menu principal
				display.display(); 							//Muestra solo el array del menu principal
			}
			else if (menu_submenu_state == AJUSTES_SUBMENU){	
				display.setTextSize(2);
				display.setTextColor(WHITE);
				display.print(ajustes[i]);									//Muestra solo el array del submenu ajustes
				display.display(); 							//Muestra solo el array del submenu ajustes
			}
			else if (menu_submenu_state == MEDICION_SUBMENU){	
				display.setTextSize(2);
				display.setTextColor(WHITE);
				display.print(medicion[i]);									//Muestra solo el array el submenu medicion
				display.display(); 					//Muestra solo el array el submenu medicion
			}
		}
	}
}


/*Funcion que se encarga especificamente de tomar la medicion*/
void StateMachine_Control(uint8_t Menu, Menu_state_e menu_submenu_state)
{
	switch(Menu)
	{
		case TOMAR_MEDICION:
		{
			move_t buttonProcess = DONTMOVE;
			while(buttonProcess != ENTER)					//Me aparto del codigo principal mientras el boton sea distinto de enter
			{
				display.clearDisplay();
				display.setCursor(0,0);
				display.setTextSize(2);      // establece tamano de texto en 2
  				display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
				display.print("T = ");
				{
					//Parte en que se cuentan los pulsos 
				}
				buttonProcess = ENTER;						//Sirve para volver hacia el while
			}
			estado_actual = INICIO_MEDICION;				//Se vuelve a la pantalla anterior 
		}
		break;
	}
}

