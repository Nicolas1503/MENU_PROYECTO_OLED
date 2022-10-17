#include <Arduino.h>
#include "button.hpp"
#include <Preferences.h> 
#include "../lib/display/display_mod.h"
#include "../lib/rtc/rtc_mod.h"
#include "../lib/sd/sd_mod.h"
#include "../lib/lora/lora_mod.h"

#define MAXITEMS 20						//Cantidad de posiciones en el display

#define MAINMENU_NUM 3					//Cantidad de chars en el mainmenu
#define AJUSTES_NUM 6					//Cantidad de chars en el menu ajustes
#define MEDICION_NUM 5					//Cantidad de chars en el menu medicion
#define SELECT_HELICE_NUM 4
#define HELICE_NUM 4
#define HELICE1_NUM 3
#define HELICE2_NUM 3
#define HELICE3_NUM 3


/*Matriz de dos dimensiones para definir los menues*/
const char menu[MAINMENU_NUM][MAXITEMS] = {"  Ajustes"," Medicion","  Ultimas   Medidas"};
const char ajustes[AJUSTES_NUM][MAXITEMS] = {"  Config.   Helices","  Config.   Periodo","  Ref. de    Lugar","  Fecha y     Hora","  Buzzer","  Atras"};
const char medicion[MEDICION_NUM][MAXITEMS] = {"  Elegir    Helice","  Inicio","  Guardar  Medicion", "  Enviar   Medicion", "Atras"};
const char helicemed[SELECT_HELICE_NUM][MAXITEMS] = {"Helice 1", "Helice 2", "Helice 3", "Atras"};
const char helice[HELICE_NUM][MAXITEMS] = {"Helice 1", "Helice 2", "Helice 3", "Atras"};
const char helice1[HELICE1_NUM][MAXITEMS] = {"Valor A", "Valor B", "Atras"};
const char helice2[HELICE2_NUM][MAXITEMS] = {"Valor A", "Valor B", "Atras"};
const char helice3[HELICE3_NUM][MAXITEMS] = {"Valor A", "Valor B", "Atras"};


/*Definicion de menues, submenues y acciones*/

#define	OUT 1
#define AJUSTES 2
#define MEDICION 3
#define ULT_MEDIDAS 4
#define CFG_HELICES 5
#define CFG_PERIODO 6
#define REF_LUGAR 7
#define FECHA_HORA 8
#define BUZZER 9
#define ATRAS_AJUSTES 10
#define SELECT_HELICES 11
#define INICIO_MEDICION 12
#define GUARDAR_MEDICION 13
#define ENVIAR_MEDICION 14
#define ATRAS_MEDICION 15
#define SELECT_HELICE_1 16
#define SELECT_HELICE_2 17
#define SELECT_HELICE_3 18
#define ATRAS_SELECT_HELICES 19
#define SET_HELICE_1 20
#define SET_HELICE_2 21
#define SET_HELICE_3 22
#define TOMAR_MEDICION 23
#define TOMAR_PERIODO 24
#define HELICE_1 25
#define HELICE_2 26
#define HELICE_3 27
#define ATRAS_HELICE 28
#define VALOR_A1 29
#define VALOR_B1 30
#define ATRAS_HELICE_1 31
#define SET_VALOR_A1 32
#define SET_VALOR_B1 33
#define VALOR_A2 34
#define VALOR_B2 35
#define ATRAS_HELICE_2 36
#define SET_VALOR_A2 37
#define SET_VALOR_B2 38
#define VALOR_A3 39
#define VALOR_B3 40
#define ATRAS_HELICE_3 41
#define SET_VALOR_A3 42
#define SET_VALOR_B3 43
#define SET_REF_LUGAR 44
#define SET_GUARDAR_MEDICION 45
#define SET_ENVIAR_MEDICION 46
#define SET_FECHA_HORA 47


uint8_t NUM_HELICE = 0;
int PERIODO_MEMO;

float A = 0.00; 
float B = 0.00;
float VELOCIDAD = 0.00;


/*Definicion de ubicacion dentro del menu*/
typedef enum{
	MAIN,
	AJUSTES_SUBMENU,
	MEDICION_SUBMENU,
	SELECT_HELICE_SUBMENU,
	HELICE_SUBMENU,
	HELICE_1_SUBMENU,
	HELICE_2_SUBMENU,
	HELICE_3_SUBMENU,

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
const int up_button = 13;
const int down_button = 14;
const int enter_button = 12;
move_t buttonProcess = DONTMOVE;

/*Botones como pull up*/
Button Up(up_button,pullup);					
Button Down(down_button,pullup);
Button Enter(enter_button,pullup);

uint8_t estado_actual;
uint8_t estado_anterior;
Menu_state_e menu_submenu_state;
row_t ROW_STATUS;

move_t CheckButton(void);

bool lcd_UpdateCursor(uint8_t Menu, int row, int col);
void lcd_DisplayMenu(uint8_t Menu, Menu_state_e menu_submenu_state);
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count);
void IRAM_ATTR isr_helice();
bool StateMachine_Control(uint8_t Menu, Menu_state_e menu_submenu_state);

Preferences preferences;


uint8_t gpio_helice = 34;
int contador_helice = 0;

String SEGUNDO, MINUTO, HORA, DIA_MES, MES, YEAR;


void setup()
{
  	Serial.begin(9600);
	Wire.begin();         // inicializa bus I2C
 	display_begin();

	display_notif("Bienvenido");
	delay(2000);	
	
	SD_Begin();

	delay(1000);

	LoRa_begin();


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
				display_clear();
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
					estado_actual = SELECT_HELICES;				//Primer estado de este menu 
				}
				break;

				case SELECT_HELICES:
				{
					menu_submenu_state = SELECT_HELICE_SUBMENU;  		
					estado_actual = SELECT_HELICE_1;				
				}
				break;

				case ATRAS_SELECT_HELICES:
				{
					menu_submenu_state = MEDICION_SUBMENU;  		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = SELECT_HELICES;				
				}
				break;

				case SELECT_HELICE_1:
				{
					menu_submenu_state = SELECT_HELICE_SUBMENU;  		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = SET_HELICE_1;				
				}
				break;

				case SELECT_HELICE_2:
				{
					menu_submenu_state = SELECT_HELICE_SUBMENU;  		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = SET_HELICE_2;				
				}
				break;

				case SELECT_HELICE_3:
				{
					menu_submenu_state = SELECT_HELICE_SUBMENU;  		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = SET_HELICE_3;				
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

				case CFG_HELICES:
				{
					menu_submenu_state = HELICE_SUBMENU;
					estado_actual = HELICE_1;
				}
				break;

				case ATRAS_HELICE:
				{
					menu_submenu_state = AJUSTES_SUBMENU;
					estado_actual = CFG_HELICES;
				}
				break;

				case HELICE_1:
				{
					menu_submenu_state = HELICE_1_SUBMENU;  		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = VALOR_A1;				
				}
				break;

				case ATRAS_HELICE_1:
				{
					menu_submenu_state = HELICE_SUBMENU;
					estado_actual = HELICE_1;
				}
				break;

				case VALOR_A1:
				{
					menu_submenu_state = HELICE_1_SUBMENU;
					estado_actual = SET_VALOR_A1;
				}
				break;

				case VALOR_B1:
				{
					menu_submenu_state = HELICE_1_SUBMENU;
					estado_actual = SET_VALOR_B1;
				}
				break;

				case HELICE_2:
				{
					menu_submenu_state = HELICE_2_SUBMENU;  		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = VALOR_A2;				
				}
				break;

				case ATRAS_HELICE_2:
				{
					menu_submenu_state = HELICE_SUBMENU;
					estado_actual = HELICE_2;
				}
				break;

				case VALOR_A2:
				{
					menu_submenu_state = HELICE_2_SUBMENU;
					estado_actual = SET_VALOR_A2;
				}
				break;

				case VALOR_B2:
				{
					menu_submenu_state = HELICE_2_SUBMENU;
					estado_actual = SET_VALOR_B2;
				}
				break;

				case HELICE_3:
				{
					menu_submenu_state = HELICE_3_SUBMENU;  		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = VALOR_A3;				
				}
				break;

				case ATRAS_HELICE_3:
				{
					menu_submenu_state = HELICE_SUBMENU;
					estado_actual = HELICE_3;
				}
				break;

				case VALOR_A3:
				{
					menu_submenu_state = HELICE_3_SUBMENU;
					estado_actual = SET_VALOR_A3;
				}
				break;

				case VALOR_B3:
				{
					menu_submenu_state = HELICE_3_SUBMENU;
					estado_actual = SET_VALOR_B3;
				}
				break;

				case REF_LUGAR:
				{
					menu_submenu_state = AJUSTES_SUBMENU;
					estado_actual = SET_REF_LUGAR;
				}
				break;

				case FECHA_HORA:
				{
					menu_submenu_state = AJUSTES_SUBMENU;
					estado_actual = SET_FECHA_HORA;
				}
				break;

				case GUARDAR_MEDICION:
				{
					menu_submenu_state = MEDICION_SUBMENU;
					estado_actual = SET_GUARDAR_MEDICION;
				}
				break;

				case ENVIAR_MEDICION:
				{
					menu_submenu_state = MEDICION_SUBMENU;
					estado_actual = SET_ENVIAR_MEDICION;
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
			firstMenu = SELECT_HELICES;
			lastMenu = ATRAS_MEDICION;
			break;
		case HELICE_SUBMENU:
			firstMenu = HELICE_1;
			lastMenu = ATRAS_HELICE;
			break;
		case HELICE_1_SUBMENU:
			firstMenu = VALOR_A1;
			lastMenu = ATRAS_HELICE_1;
			break;
		case HELICE_2_SUBMENU:
			firstMenu = VALOR_A2;
			lastMenu = ATRAS_HELICE_2;
			break;
		case HELICE_3_SUBMENU:
			firstMenu = VALOR_A3;
			lastMenu = ATRAS_HELICE_3;
			break;
		
		}
		

		Serial.println("Cambio de estado");
		Serial.println(estado_actual);
		return 1;
	}
	return 0;
}

/*Se usa?*/
void lcd_ClearOneLine(int row);

/*Se usa?*/
void lcd_ClearCursor(int row);

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

		case FECHA_HORA:
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

		case SELECT_HELICES:
		{
			lcd_PrintCursor(menu_submenu_state,0,1);
		}
		break;
		
		case INICIO_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
		}
		break;

		case GUARDAR_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,2,1);
		}
		break;

		case ENVIAR_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,3,1);
		}
		break;

		case ATRAS_MEDICION:
		{
			lcd_PrintCursor(menu_submenu_state,4,1);
		}
		break;

		case TOMAR_MEDICION:
		{
			//No va a hacer nada en la pantalla 
		}
		break;

		case SELECT_HELICE_1:
		{
			lcd_PrintCursor(menu_submenu_state,0,1);
		}
		break;

		case SELECT_HELICE_2:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
		}
		break;

		case SELECT_HELICE_3:
		{
			lcd_PrintCursor(menu_submenu_state,2,1);
		}
		break;

		case ATRAS_SELECT_HELICES:
		{
			lcd_PrintCursor(menu_submenu_state,3,1);
		}
		break;

		case HELICE_1:
		{
			lcd_PrintCursor(menu_submenu_state,0,1);
		}
		break;

		case HELICE_2:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
		}
		break;

		case HELICE_3:
		{
			lcd_PrintCursor(menu_submenu_state,2,1);
		}
		break;
		
		case ATRAS_HELICE:
		{
			lcd_PrintCursor(menu_submenu_state,3,1);
		}
		break;
		
		case VALOR_A1:
		{
			lcd_PrintCursor(menu_submenu_state,0,1);
		}
		break;

		case VALOR_B1:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
		}
		break;

		case ATRAS_HELICE_1:
		{
			lcd_PrintCursor(menu_submenu_state,2,1);
		}
		break;
		
		case VALOR_A2:
		{
			lcd_PrintCursor(menu_submenu_state,0,1);
		}
		break;

		case VALOR_B2:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
		}
		break;

		case ATRAS_HELICE_2:
		{
			lcd_PrintCursor(menu_submenu_state,2,1);
		}
		break;
		
		case VALOR_A3:
		{
			lcd_PrintCursor(menu_submenu_state,0,1);
		}
		break;

		case VALOR_B3:
		{
			lcd_PrintCursor(menu_submenu_state,1,1);
		}
		break;

		case ATRAS_HELICE_3:
		{
			lcd_PrintCursor(menu_submenu_state,2,1);
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
	display_background(true);

	if (count <= ROWNUM){
		for (uint8_t i=start; i<count+start; i++)				//Cuenta para mostrar los menu en la pantalla
		{
			if (menu_submenu_state == MAIN){
				display_showmenu("    Menu Principal", menu[i]);
			}
			else if (menu_submenu_state == AJUSTES_SUBMENU){
				display_showmenu("        Ajustes", ajustes[i]);	
			}
			else if (menu_submenu_state == MEDICION_SUBMENU){
				display_showmenu("       Medicion", medicion[i]);
			}
			else if (menu_submenu_state == SELECT_HELICE_SUBMENU){
				display_showmenu("Seleccion de Helice", helicemed[i]);
			}
			else if (menu_submenu_state == HELICE_SUBMENU){	
				display_showmenu("Config. Helices", helice[i]);
			}
			else if (menu_submenu_state == HELICE_1_SUBMENU){	
				display_showmenu("Config. Helice 1", helice1[i]);
			}
			else if (menu_submenu_state == HELICE_2_SUBMENU){	
				display_showmenu("Config. Helice 2", helice2[i]);
			}
			else if (menu_submenu_state == HELICE_3_SUBMENU){	
				display_showmenu("Config. Helice 3", helice3[i]);
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
				int periodo = preferences.getInt("PERIODO_MEMO", 0);
				
				display_MedicionMode(NUM_HELICE,periodo);
				
				unsigned long lastmillis = millis();
				unsigned long timedone = 0; 		//Se le asigna el tiempo transcurrido dentro del while loop siguiente

				contador_helice = 0; // Pongo en 0 el contador de pulsos
				attachInterrupt(digitalPinToInterrupt(gpio_helice), isr_helice, FALLING); // Habilita la interrupcion que va a contar los pulsos
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
						display_ShowDotiter(dotiter, dotcount);
						
					}
				}
				buttonProcess = ENTER;
				detachInterrupt(digitalPinToInterrupt(gpio_helice)); // Deshabilita la interrupcion y deja de contar pulsos
				
				float pulse = contador_helice;
				float segundos = periodo/1000;
		
				/* Calcular con la ecuacion
				*/
				VELOCIDAD = A*(pulse/segundos) + B;
				
				/* Mostrar en pantalla de la velocidad y entrar en un loop 
				donde la persona tenga que presionar enter para salir y dejar de ver la velocidad
				*/

				display_ShowSpeed(contador_helice, VELOCIDAD);
				
				while(CheckButton() != ENTER)
				{
					//esperar a pulsar enter
				}

			
			}
			estado_actual = GUARDAR_MEDICION;
			return 1;
			
		}
		break;
			
		case TOMAR_PERIODO:
		
		{
			bool outPeriodo = 1;
			move_t buttonProcess = DONTMOVE;
			int periodo = preferences.getInt("PERIODO_MEMO", 0);
			display_background(false);
			display_showparam("   Config. Periodo", periodo/1000, true, " Seg.");
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
						display_background(false);
						display_showparam("   Config. Periodo", periodo/1000, true, " Seg.");
					}
					break;

					case DOWN:
					{
						if((periodo - 5000) != 0){			//Si se oprime el boton hacia arriba se decrementa el periodo 
							periodo = periodo - 5000;
						}
						display_background(false);
						display_showparam("   Config. Periodo", periodo/1000, true,  " Seg.");
					}
					break;
				}
			}
			preferences.putInt("PERIODO_MEMO", periodo);
			

		}
		break;
	
		case SET_VALOR_A1:
		
		{	
			bool outConstante_A1 = 1;
			move_t buttonProcess = DONTMOVE;
			float CONSTANTE_A1 = preferences.getFloat("CONSTANTE_A1", 0);
			display_background(false);
			display_showparam("  Valor A Helice 1", CONSTANTE_A1, false," ");
			
			while(outConstante_A1)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_A1 = 0;
						estado_actual = VALOR_A1;
					}
					break;

					case UP:
					{
						CONSTANTE_A1 = CONSTANTE_A1 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display_background(false);
						display_showparam("  Valor A Helice 1", CONSTANTE_A1, false, " ");
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_A1 = CONSTANTE_A1 - 0.01;
						display_background(false);
						display_showparam("  Valor A Helice 1", CONSTANTE_A1, false, " ");
					}
					break;
				}
			}
			preferences.putFloat("CONSTANTE_A1", CONSTANTE_A1);
			

		}
		break;

	case SET_VALOR_B1:
	{
			bool outConstante_B1 = 1;
			move_t buttonProcess = DONTMOVE;
			float CONSTANTE_B1 = preferences.getFloat("CONSTANTE_B1", 0);
			display_background(false);
			display_showparam("  Valor B Helice 1", CONSTANTE_B1, false, " ");
			
			while(outConstante_B1)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_B1 = 0;
						estado_actual = VALOR_B1;
					}
					break;

					case UP:
					{
						CONSTANTE_B1 = CONSTANTE_B1 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display_background(false);
						display_showparam("  Valor B Helice 1", CONSTANTE_B1, false, " ");
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_B1 = CONSTANTE_B1 - 0.01;
						display_background(false);
						display_showparam("  Valor B Helice 1", CONSTANTE_B1, false, " ");
					}
					break;
				}
			}
			preferences.putFloat("CONSTANTE_B1", CONSTANTE_B1);
			

		}
		break;

	case SET_VALOR_A2:
	{	
			bool outConstante_A2 = 1;
			move_t buttonProcess = DONTMOVE;
			float CONSTANTE_A2 = preferences.getFloat("CONSTANTE_A2", 0);
			display_background(false);
			display_showparam("   Valor A Helice 2", CONSTANTE_A2, false, " ");
			
			while(outConstante_A2)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_A2 = 0;
						estado_actual = VALOR_A2;
					}
					break;

					case UP:
					{
						CONSTANTE_A2 = CONSTANTE_A2 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display_background(false);
						display_showparam("   Valor A Helice 2", CONSTANTE_A2, false,  " ");
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_A2 = CONSTANTE_A2 - 0.01;
						display_background(false);
						display_showparam("   Valor A Helice 2", CONSTANTE_A2, false, " ");
					}
					break;
				}
			}
			preferences.putFloat("CONSTANTE_A2", CONSTANTE_A2);
			

		}
		break;

	case SET_VALOR_B2:
	{
			bool outConstante_B2 = 1;
			move_t buttonProcess = DONTMOVE;
			float CONSTANTE_B2 = preferences.getFloat("CONSTANTE_B2", 0);
			display_background(false);
			display_showparam("  Valor B Helice 2", CONSTANTE_B2, false, " ");
			
			while(outConstante_B2)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_B2 = 0;
						estado_actual = VALOR_B2;
					}
					break;

					case UP:
					{
						CONSTANTE_B2 = CONSTANTE_B2 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display_background(false);
						display_showparam("  Valor B Helice 2", CONSTANTE_B2, false, " ");
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_B2 = CONSTANTE_B2 - 0.01;
						display_background(false);
						display_showparam("  Valor B Helice 2", CONSTANTE_B2, false, " ");
					}
					break;
				}
			}
			preferences.putFloat("CONSTANTE_B2", CONSTANTE_B2);
			

		}
		break;
	
	case SET_VALOR_A3:
	{	
			bool outConstante_A3 = 1;
			move_t buttonProcess = DONTMOVE;
			float CONSTANTE_A3 = preferences.getFloat("CONSTANTE_A3", 0);
			display_background(false);
			display_showparam("   Valor A Helice 3", CONSTANTE_A3, false, " ");
			
			while(outConstante_A3)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_A3 = 0;
						estado_actual = VALOR_A3;
					}
					break;

					case UP:
					{
						CONSTANTE_A3 = CONSTANTE_A3 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display_background(false);
						display_showparam("   Valor A Helice 3", CONSTANTE_A3, false, " ");
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_A3 = CONSTANTE_A3 - 0.01;
						display_background(false);
						display_showparam("   Valor A Helice 3", CONSTANTE_A3, false, " ");
					}
					break;
				}
			}
			preferences.putFloat("CONSTANTE_A3", CONSTANTE_A3);
			

		}
		break;

	case SET_VALOR_B3:
	{
			bool outConstante_B3 = 1;
			move_t buttonProcess = DONTMOVE;
			float CONSTANTE_B3 = preferences.getFloat("CONSTANTE_B3", 0);
			display_background(false);
			display_showparam("  Valor B Helice 3", CONSTANTE_B3, false, " ");
			
			while(outConstante_B3)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_B3 = 0;
						estado_actual = VALOR_B3;
					}
					break;

					case UP:
					{
						CONSTANTE_B3 = CONSTANTE_B3 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display_background(false);
						display_showparam("  Valor B Helice 3", CONSTANTE_B3, false, " ");
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_B3 = CONSTANTE_B3 - 0.01;
						display_background(false);
						display_showparam("  Valor B Helice 3", CONSTANTE_B3, false, " ");
					}
					break;
				}
			}
			preferences.putFloat("CONSTANTE_B3", CONSTANTE_B3);
			

		}
		break;

		case SET_REF_LUGAR:
	{
			bool outRef_Lugar = 1;
			move_t buttonProcess = DONTMOVE;
			int ID_LUGAR = preferences.getInt("ID_LUGAR", 0);
			display_background(false);
			display_showparam("  ID de Referencia", ID_LUGAR, true, " ");
			
			while(outRef_Lugar)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outRef_Lugar = 0;
						estado_actual = REF_LUGAR;
					}
					break;

					case UP:
					{
						ID_LUGAR = ID_LUGAR + 1;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display_background(false);
						display_showparam("  ID de Referencia", ID_LUGAR, true, " ");
					}
					break;

					case DOWN:
					{
								
						ID_LUGAR = ID_LUGAR - 1;
						display_background(false);
						display_showparam("  ID de Referencia", ID_LUGAR, true, " ");
					}
					break;
				}
			}
			preferences.putInt("ID_LUGAR", ID_LUGAR);
			

		}
		break;

		case SET_FECHA_HORA:
	{
			bool outFecha_Hora = 1;
			move_t buttonProcess = DONTMOVE;
			
			HORA = leeHora();
			MINUTO = leeMinuto();		
			DIA_MES = leeDiaMes();
			MES = leeMes();
			YEAR = leeAnio();

			display_showFechaHora(HORA, MINUTO, DIA_MES, MES, YEAR);
			
			while(outFecha_Hora)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					
					case UP:break;
					
					case DOWN:break;
					
					case ENTER:
					{
						outFecha_Hora = 0;
						estado_actual = FECHA_HORA;
					}
					break;
				}
			}
			
		}
		break;
	
	case SET_HELICE_1:
	{		
			bool outHelice1 = 1;
			move_t buttonProcess = DONTMOVE;
			NUM_HELICE = 1;
			A = preferences.getFloat("CONSTANTE_A1", 0);
			B = preferences.getFloat("CONSTANTE_B1", 0);
			display_ShowHeliceSelected(NUM_HELICE, A, B);
			
			while(outHelice1)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					
					case UP:break;
					
					case DOWN:break;
					
					case ENTER:
					{
						outHelice1 = 0;
						estado_actual = ATRAS_SELECT_HELICES;
					}
					break;
				}
			}

		}
		break;
	
	case SET_HELICE_2:
	{		
			bool outHelice2 = 1;
			move_t buttonProcess = DONTMOVE;
			NUM_HELICE = 2;
			A = preferences.getFloat("CONSTANTE_A2", 0);
			B = preferences.getFloat("CONSTANTE_B2", 0);
			display_ShowHeliceSelected(NUM_HELICE, A, B);
			
			while(outHelice2)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					
					case UP:break;
					
					case DOWN:break;
					
					case ENTER:
					{
						outHelice2 = 0;
						estado_actual = ATRAS_SELECT_HELICES;
					}
					break;
				}
			}

		}
		break;

	case SET_HELICE_3:
	{		
			bool outHelice3 = 1;
			move_t buttonProcess = DONTMOVE;
			NUM_HELICE = 3;
			A = preferences.getFloat("CONSTANTE_A3", 0);
			B = preferences.getFloat("CONSTANTE_B3", 0);
			display_ShowHeliceSelected(NUM_HELICE, A, B);
			
			while(outHelice3)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					
					case UP:break;
					
					case DOWN:break;
					
					case ENTER:
					{
						outHelice3 = 0;
						estado_actual = ATRAS_SELECT_HELICES;
					}
					break;
				}
			}

		}
		break;
	
	
	case SET_GUARDAR_MEDICION:
	{
			bool outGuardar_Medicion = 1;
			move_t buttonProcess = DONTMOVE;
			int ID_LUGAR = preferences.getInt("ID_LUGAR", 0);
			int PERIODO_MEDIDO = preferences.getInt("PERIODO_MEMO", 0);
			String ID_TITLE;
			String DATO_MEDIDO;
			ID_TITLE = "/ID_" + String(ID_LUGAR) + ".csv";

			HORA = leeHora();
			MINUTO = leeMinuto();
			SEGUNDO = leeSegundo();
			DIA_MES = leeDiaMes();
			MES = leeMes();
			YEAR = leeAnio();

			SD_OpenFile(ID_TITLE.c_str());
			
			DATO_MEDIDO = String(VELOCIDAD)+";"+String(PERIODO_MEDIDO/1000)+";"+String(NUM_HELICE)+";"+String(A)+";"+String (B)+";"+String (HORA)+":"+ String(MINUTO)+":"+String(SEGUNDO)+";"+String(DIA_MES)+"/"+String(MES)+"/"+String(YEAR)+"\r\n";
			
			appendFile(SD, ID_TITLE.c_str(), DATO_MEDIDO.c_str());
			
			display_ShowMedidaGuardada(ID_TITLE);
			
			
			while(outGuardar_Medicion)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					
					case UP: break;
					
					case DOWN: break;
					
					case ENTER:
					{
						outGuardar_Medicion = 0;
						estado_actual = ENVIAR_MEDICION;
					}
					break;

					
				}
			}
		}
		break;
	
	case SET_ENVIAR_MEDICION:
	{
			bool outEnviar_Medicion = 1;
			move_t buttonProcess = DONTMOVE;
			int PERIODO_MEDIDO = preferences.getInt("PERIODO_MEMO", 0);

			LoRa_SendPacket(VELOCIDAD, PERIODO_MEDIDO/1000, NUM_HELICE);

			display_ShowMedidaEnviada();
			
			
			while(outEnviar_Medicion)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					
					case UP: break;
					
					case DOWN: break;
					
					case ENTER:
					{
						outEnviar_Medicion = 0;
						estado_actual = ENVIAR_MEDICION;
					}
					break;

					
				}
			}
		}
		break;
	
	}
	return 0;
}

void IRAM_ATTR isr_helice()
{
	contador_helice++;
}