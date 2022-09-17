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

/*Matriz de dos dimensiones para definir los menues*/
const char menu[MAINMENU_NUM][MAXITEMS] = {"  Ajustes"," Medicion","  Ultimas   Medidas"};
const char ajustes[AJUSTES_NUM][MAXITEMS] = {"  Config.   Helices","  Config.   Periodo","  Ref. de    Lugar","  Fecha y     Hora","  Buzzer","  Atras"};
const char medicion[MEDICION_NUM][MAXITEMS] = {"  Inicio","  Atras"};

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

int PERIODO_MEMO;


/*Definicion de ubicacion dentro del menu*/
typedef enum{
	MAIN,
	AJUSTES_SUBMENU,
	MEDICION_SUBMENU
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


/*void SetEEPROMValueF(int address, float value);
void UpdateEEPROMValueF(int adress, float value);
float GetEEPROMValueF(int address);*/

move_t CheckButton(void);

bool lcd_UpdateCursor(uint8_t Menu, int row, int col);
void lcd_ClearOneLine(int row);
void lcd_ClearCursor(int row);
void lcd_DisplayMenu(uint8_t Menu, Menu_state_e menu_submenu_state);
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count);
void IRAM_ATTR isr_helice();

bool StateMachine_Control(uint8_t Menu, Menu_state_e menu_submenu_state);

Adafruit_SH1106 display(OLED_SDA, OLED_SCL);

Preferences preferences;

const int gpio_helice = 34; // Cambiar esto por el valor correcto
int contador_helice = 0;

void setup()
{
  	Wire.begin();         // inicializa bus I2C
 	display.begin(SH1106_SWITCHCAPVCC, 0x3C); // inicializa pantalla con direccion 0x3C

  	estado_actual = AJUSTES;				//Estado actual y anterior en ajustes
 	estado_anterior = AJUSTES;
  	menu_submenu_state = MAIN;			//Menu actual esta en el principal

  	
	lcd_DisplayMenu(estado_actual, menu_submenu_state);			//Se muestra en el display lo que se declaro

	 preferences.begin("myfile", false);
}
void loop(void)								
{
	bool ret = 0;							//Si el booleano es 0 no se hace nada
	while(true)
	{
		ret = lcd_UpdateCursor(estado_actual,ROWNUM,COLNUM);	//Si se apreto algun boton se actualiza la pantalla (actualiza los estados)
		ret |= StateMachine_Control(estado_actual,menu_submenu_state);
		if (ret == 1){
			lcd_DisplayMenu(estado_actual,menu_submenu_state);	
		}
	}
}


/*float GetEEPROMValueF(const char [7],int)
{
    float val;
    return preferences.getFloat("param", 0);
}

void SetEEPROMValueF(const char [7],float)
{
   preferences.putFloat("param", param);  
}

void UpdateEEPROMValueF(int address, float value)
{
	EEPROM.update(address,value);
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
				estado_actual = estado_actual + 1;
			}
		}
		else if (buttonProcess == UP){				//Si el boton fue UP 
			if(estado_actual != firstMenu)			//Si el estado actual es distinto del primer item del menu se resta uno
		 	{
				estado_actual = estado_actual - 1;
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

				case CFG_PERIODO:
				{
					menu_submenu_state = AJUSTES_SUBMENU;
					estado_actual = TOMAR_PERIODO;				//Cuando se aprieta enter en config. periodo se pasa al estado config. periodo
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
			lcd_PrintCursor(menu_submenu_state,0,1);	//Donde comienza a mostrar, cuantos voy a mostrar
		}
		break;

		case MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
		}
		break;

		case ULT_MEDIDAS:
		{
			lcd_PrintCursor(menu_submenu_state,2,1);
		}
		break;

		case CFG_HELICES:
		{
			lcd_PrintCursor(menu_submenu_state,0,1);
		}
		break;

		case CFG_PERIODO:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
		}
		break;
		
		case REF_LUGAR:
		{
			lcd_PrintCursor(menu_submenu_state,2,1);
		}
		break;

		case CFG_DATE:
		{
			lcd_PrintCursor(menu_submenu_state,3,1);
		}
		break;

		case BUZZER:
		{
			lcd_PrintCursor(menu_submenu_state,4,1);
		}
		break;

		case ATRAS_AJUSTES:
		{
			lcd_PrintCursor(menu_submenu_state,5,1);
		}
		break;

		case INICIO_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,0,1);
		}
		break;

		case ATRAS_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
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
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count) //Estados, donde comienza en el array que corresponde
{
	display.clearDisplay();      // limpia pantalla      
  	display.drawLine(0, 10, 128, 10, WHITE); // dibuja linea
	display.drawLine(0, 54, 128, 54, WHITE); // dibuja linea 
  	display.setCursor(0,56);
  	display.setTextSize(1);
  	display.setTextColor(WHITE);
 	display.cp437(true);
  	display.write(0x11);
  	display.setCursor(58,56);
  	display.print("OK"); 
  	display.setCursor(121,56);
  	display.cp437(true);
  	display.write(0x10);
	display.display(); 
	
	if (count <= ROWNUM){
		for (uint8_t i=start; i<count+start; i++)				//Cuenta para mostrar los menu en la pantalla
		{
			
			display.setCursor(0,0);
			display.setTextSize(1);
  			display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)

			if (menu_submenu_state == MAIN){
				
				display.print("    Menu Principal");
  				display.setCursor(0, 17);   // ubica cursor en coordenadas 0,14
  				display.setTextSize(2);      // establece tamano de texto en 2
  				display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
				display.print(menu[i]);										//Muestra solo el array del menu principal
				display.display(); 							//Muestra solo el array del menu principal
			}
			else if (menu_submenu_state == AJUSTES_SUBMENU){	
				display.print("        Ajustes");
  				display.setCursor(0, 17);   // ubica cursor en coordenadas 0,14
  				display.setTextSize(2);      // establece tamano de texto en 2
				display.setTextColor(WHITE);
				display.print(ajustes[i]);									//Muestra solo el array del submenu ajustes
				display.display(); 							//Muestra solo el array del submenu ajustes
			}
			else if (menu_submenu_state == MEDICION_SUBMENU){	
				display.print("       Medicion");
  				display.setCursor(0, 17);   // ubica cursor en coordenadas 0,14
  				display.setTextSize(2);      // establece tamano de texto en 2
				display.setTextColor(WHITE);
				display.print(medicion[i]);									//Muestra solo el array el submenu medicion
				display.display(); 					//Muestra solo el array el submenu medicion
			}
		}
	}
}


/*Funcion que se encarga especificamente de tomar la medicion*/

bool StateMachine_Control(uint8_t Menu, Menu_state_e menu_submenu_state)
{
	switch(Menu)
	{
		case TOMAR_MEDICION:

		{

			move_t buttonProcess = DONTMOVE;
			bool buttonOut = 1;
			while(buttonProcess != ENTER)								//Me aparto del codigo principal mientras el boton sea distinto de enter
			{
				float dotcount = 1000.0;
				uint8_t dotiter = 0;

				/* Muestro el periodo en segundos*/
				display.clearDisplay(); 
				display.setCursor(0,0);
				display.setTextSize(2);
  				display.setTextColor(WHITE);
				display.print("T = ");
				display.display();
				int periodo = preferences.getInt("PERIODO_MEMO", 0);
				/*lcd.setCursor(4,0);
				lcd.print(periodo/1000); //Convierto de milisegundos a segundos
				lcd.print("sec");
				*/

				unsigned long lastmillis = millis();
				unsigned long timedone = 0; 		//Se le asigna el tiempo transcurrido dentro del while loop siguiente

				contador_helice = 0; // Pongo en 0 el contador de pulsos
				attachInterrupt(gpio_helice, isr_helice, FALLING); // Habilita la interrupcion que va a contar los pulsos
				while(buttonOut)
				{
					
					timedone = millis() - lastmillis;
					if(timedone >= periodo || CheckButton() == ENTER)
					{
						buttonOut = 0;
					}
					
					/* 	Muestra un punto cada 1 segundo transcurrido
						Esto es solo un ejemplo */
					if(timedone == dotcount)
					{
						display.setCursor(dotiter,1);
						display.setTextSize(2);
  						display.setTextColor(WHITE);
						display.print(".");
						dotcount = dotcount + 1000.0;
						display.display();
						if(dotiter < 16)
						{
							dotiter++;
						}
						else{
							lcd_ClearOneLine(1);
							dotiter = 0;
						}
					}
				}
				buttonProcess = ENTER;
				detachInterrupt(gpio_helice); // Deshabilita la interrupcion y deja de contar pulsos

				/* Leer valores de A y B de la memoria flash
				int A1 = preferences.getInt("A1", 0);
				int B1 = preferences.getInt("B1", 0);
				*/
				float A = 0.2368;
				float B = 0.0044;
				float pulse = contador_helice;
				float tiempito = periodo/1000;
		
				/* Calcular con la ecuacion
				int v = A*contador_helice/periodo + B;
				*/
				float v = A*(pulse/tiempito) + B;
				/* Mostrar en pantalla de la velocidad y entrar en un loop 
				donde la persona tenga que presionar enter para salir y dejar de ver la velocidad
				*/

				display.clearDisplay(); 
				display.setCursor(0,0);
				display.setTextSize(2);
  				display.setTextColor(WHITE);
				display.print(contador_helice);
				display.setCursor(0,20);
				display.print(v,4);
				display.display();
				delay(10000);

			
			}
			estado_actual = INICIO_MEDICION;
			//preferences.end();
			return 1;
			
		}
		break;
			
		case TOMAR_PERIODO:
		
		{
			bool outPeriodo = 1;
			move_t buttonProcess = DONTMOVE;
			int periodo = preferences.getInt("PERIODO_MEMO", 0);
			display.clearDisplay(); 
			display.setCursor(0,0);
			display.setTextSize(2);
  			display.setTextColor(WHITE);
			display.print("Periodo");
			display.setCursor(0,17);
			display.print(periodo);
			display.display();
			
			while(outPeriodo)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outPeriodo = 0;
						estado_actual = CFG_PERIODO;
					}
					break;

					case UP:
					{
						periodo = periodo + 5000;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display.clearDisplay();
						display.setCursor(0,17);
						display.setTextSize(2);
  						display.setTextColor(WHITE);
						display.print(periodo);
						display.display();
					}
					break;

					case DOWN:
					{
						if((periodo - 5000) != 0){			//Si se oprime el boton hacia arriba se decrementa el periodo 
							periodo = periodo - 5000;
						}
						display.clearDisplay();
						display.setCursor(0,17);
						display.setTextSize(2);
  						display.setTextColor(WHITE);
						display.print(periodo);
						display.display();
					}
					break;
				}
			}
			preferences.putInt("PERIODO_MEMO", periodo);
			

		}
		break;
	}
	return 0;
}

void IRAM_ATTR isr_helice()
{
	contador_helice++;
}