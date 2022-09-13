#include <Arduino.h>
#include <EEPROM.h>
#include "button.hpp"
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define MAXITEMS 16					//Cantidad de posiciones en el display
#define ROWNUM 64
#define COLNUM 128

#define OLED_RESET 4      // necesario por la libreria pero no usado

#define MAINMENU_NUM 3							//Cantidad de chars en el mainmenu
#define AJUSTES_NUM 6							//Cantidad de chars en el menu ajustes
#define MEDICION_NUM 2							//Cantidad de chars en el menu medicion
#define HELICE_NUM 3

// Use with SetEEPROMValueF / GetEEPROMValueF
#define A_ELISE 0
#define B_ELISE 4
#define PERIODO 8

// Use with SetEEPROMValueB / GetEEPROMValueB
#define DAY 12
#define MONTH 13
#define YEAR 14
#define HOUR 15
#define MINUTES 16
#define RESETNUM 17
/*Matriz de dos dimensiones para definir los menues*/
const char menu[MAINMENU_NUM][MAXITEMS] = {"Ajustes","Medicion","Ultimas Medidas"};
const char ajustes[AJUSTES_NUM][MAXITEMS] = {"Config. Helices", "Config. Periodo","Refere. Lugar","Fecha/Hora","Buzzer","Atras"};
const char medicion[MEDICION_NUM][MAXITEMS] = {"Inicio","Atras"};
const char helice[HELICE_NUM][MAXITEMS] = {"Valor A","Valor B", "Atras"};

/*Definicion de menues, submenues y acciones*/
typedef enum{
	OUT,
    AJUSTES,
	MEDICION,
	ULT_MEDIDAS,
	CFG_HELICES,
	CFG_PERIODO,
	REF_LUGAR,
	CFG_DATE,
	BUZZER,
	ATRAS_AJUSTES,
	INICIO_MEDICION,
	ATRAS_MEDICION,
	TOMAR_MEDICION,
	TOMAR_FECHA_HORA,
	TOMAR_PERIODO,
	VALOR_A,
	VALOR_B,
	ATRAS_HELICE
}Menu_e;

/*Deficinicion de ubicacion dentro del menu*/
typedef enum{
	MAIN,
	AJUSTES_SUBMENU,
	MEDICION_SUBMENU,
	HELICE_SUBMENU
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
const int up_button = 2;
const int down_button = 4;
const int enter_button = 5;
move_t buttonProcess = DONTMOVE;

/*Botones como pull up*/
Button Up(up_button,pullup);
Button Down(down_button,pullup);
Button Enter(enter_button,pullup);

Menu_e estado_actual;
Menu_e estado_anterior;
Menu_state_e menu_submenu_state;
row_t ROW_STATUS;

// EEPROM wrappers
void SetEEPROMValueB(int address, uint8_t value);
void SetEEPROMValueF(int address, float value);
void UpdateEEPROMValueB(int address, uint8_t value);
void UpdateEEPROMValueF(int address, float value);
uint8_t GetEEPROMValueB(int address);
float GetEEPROMValueF(int address);

move_t CheckButton(void);

// LCD wrappers
bool lcd_UpdateCursor(Menu_e Menu, int row, int col);
void lcd_ClearOneLine(int row);
void lcd_ClearCursor(int row);
void lcd_DisplayMenu(Menu_e Menu, Menu_state_e menu_submenu_state);
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count, uint8_t cursorPosition);
void lcd_setSpaces(uint8_t day, uint8_t month, uint8_t year, uint8_t hour, uint8_t minutes);
void lcd_setValueB(uint8_t value);

bool StateMachine_Control(Menu_e Menu, Menu_state_e menu_submenu_state);

Adafruit_SH1106 display(OLED_RESET);  // crea objeto

void setup()
{
	Wire.begin();         // inicializa bus I2C
 	display.begin(SH1106_SWITCHCAPVCC, 0x3C); // inicializa pantalla con direccion 0x3C
	Serial.begin(9600);

	estado_actual = AJUSTES;				//Estado actual y anterior en ajustes
	estado_anterior = AJUSTES;
	menu_submenu_state = MAIN;				//Menu actual esta en el principal

	
	display.clearDisplay();      // limpia pantalla  
	display.setCursor(0,ROW_STATUS);
	display.setTextSize(2);      // establece tamano de texto en 2
  	display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
 	display.cp437(true);
  	display.write(0x10);
	display.display(); 
	lcd_DisplayMenu(estado_actual, menu_submenu_state);			//Se muestra en el display lo que se declaro
	
	//This section is for the first time running the microcontroller without values stored on the memory
	uint8_t reset = GetEEPROMValueB(RESETNUM);
	if (reset != 1){
		SetEEPROMValueF(A_ELISE,0);
		SetEEPROMValueF(B_ELISE,0);
		SetEEPROMValueF(PERIODO,0);
		SetEEPROMValueB(RESETNUM,1);
		SetEEPROMValueB(DAY,0);
		SetEEPROMValueB(MONTH,0);
		SetEEPROMValueB(YEAR,0);
		SetEEPROMValueB(HOUR,0);
		SetEEPROMValueB(MINUTES,0);
	}
}

void loop(void)
{
	bool ret = 0;							//Si el booleano es 0 no se hace nada
	while(true)
	{
		ret = lcd_UpdateCursor(estado_actual,ROWNUM,COLNUM);			//Si se apreto algun boton se actualiza la pantalla (actualiza los estados)
		ret |= StateMachine_Control(estado_actual,menu_submenu_state);
		if (ret == 1){
			lcd_DisplayMenu(estado_actual,menu_submenu_state);
		}
		
	}
}


uint8_t GetEEPROMValueB(int address)
{
    return EEPROM.read(address);
}

float GetEEPROMValueF(int address)
{
    float val;
    return EEPROM.get(address, val);
}

void SetEEPROMValueB(int address, uint8_t value)
{
    EEPROM.write(address, value);
}

void SetEEPROMValueF(int address, float value)
{
    EEPROM.put(address, value);
}

void UpdateEEPROMValueB(int address, uint8_t value)
{
	EEPROM.update(address,value);
}

void UpdateEEPROMValueF(int address, float value)
{
	EEPROM.update(address,value);
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
  return DONTMOVE;					//Siempre retorna un 0 (no hace nada), excepto que se haya apretado un boton
}

bool lcd_UpdateCursor(Menu_e Menu, int row, int col)	//Dentro de esta funcion esta el chekbutton
{
	static move_t lastButtonProcess = DONTMOVE;			//Variables que quedan fijas segun el ultimo llamado de la funcion
	static Menu_e firstMenu = AJUSTES;
	static Menu_e lastMenu = ULT_MEDIDAS;
	static Menu_state_e lastMenuState = MAIN;

	buttonProcess = CheckButton();						//Aqui se recibe, UP, DOWN, ENTER O DONT MOVE

	if (buttonProcess != DONTMOVE)						//Si es distinto de DONTMOVE se hace algo 
	{
		lastButtonProcess = buttonProcess;
		if (buttonProcess == DOWN){						//Si el boton fue DOWN 
			if(estado_actual != lastMenu)				//Si el estado actual es distinto de el ultimo item del menu se suma uno
			{
				estado_actual = estado_actual + 1;
			}
		}
		else if (buttonProcess == UP){					//Si el boton fue UP 
			if(estado_actual != firstMenu)				//Si el estado actual es distinto del primer item del menu se resta uno
			{
				estado_actual = estado_actual - 1;
			}
		}
		else if (buttonProcess == ENTER)				//Si se aprieta el boton enter se chequea en que estado esta
		{
			if (lastButtonProcess == DOWN || lastButtonProcess == UP)
			{
				display.clearDisplay();
			}
			switch(estado_actual)
			{
				case AJUSTES:								
				{
					menu_submenu_state = AJUSTES_SUBMENU; 	//Se pasa del MAIMENU al menu de ajustes
					estado_actual = CFG_HELICES;			//Primer estado de este submenu
				}
				break;
				case ATRAS_AJUSTES:
				{
					menu_submenu_state = MAIN;				//Se pasa del menu de ajustes al MAINMENU
					estado_actual = AJUSTES;				//Primer estado de este menu 
				}
				break;
				case MEDICION:
				{
					menu_submenu_state = MEDICION_SUBMENU;		//Se pasa del MAIMENU al menu de medicion
					estado_actual = INICIO_MEDICION;			//Primer estado de este menu 			
				}
				break;

				case INICIO_MEDICION:
				{
					menu_submenu_state = MEDICION_SUBMENU;		//Cuando se aprieta enter en inicio medicion se pasa al estado tomar medicion
					estado_actual = TOMAR_MEDICION;
				}
				break;

				case ATRAS_MEDICION:
				{
					menu_submenu_state = MAIN;					//Se pasa del menu medicion al MAINMENU
					estado_actual = AJUSTES;					//Primer estado de este menu 
				}
				break;
				
				case CFG_DATE:
				{
					menu_submenu_state = AJUSTES_SUBMENU;
					estado_actual = TOMAR_FECHA_HORA;			//Cuando se aprieta enter en config. fecha/hora se pasa al estado cambiar fecha/hora
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
					estado_actual = VALOR_A;				//Cuando se aprieta enter en config. periodo se pasa al estado config. periodo
				}
				break;

				case ATRAS_HELICE:
				{
					menu_submenu_state = AJUSTES_SUBMENU;
					estado_actual = CFG_HELICES;				//Cuando se aprieta enter en config. periodo se pasa al estado config. periodo
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
		case HELICE_SUBMENU:
			firstMenu = VALOR_A;
			lastMenu = ATRAS_HELICE;
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

void lcd_DisplayMenu(Menu_e Menu, Menu_state_e menu_submenu_state)					//Funcion que imprime las opciones en el display	
{
	switch(Menu)
	{
		case OUT:											//Se define el tope en el enum para no entrar en estados que no existen
			Menu = AJUSTES;
		break;

		case AJUSTES:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,0);		//Donde comienza a mostrar, cuantos voy a mostrar, donde esta el cursor
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
			// no hacer nada
		}
		break;

		case VALOR_A:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,0);
		}
		break;

		case VALOR_B:
		{
			lcd_PrintCursor(menu_submenu_state,0,2,33);
		}
		break;

		case ATRAS_HELICE:
		{
			lcd_PrintCursor(menu_submenu_state,2,1,33);
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
		for (int i=start; i<count+start; i++)						//Cuenta para mostrar los menu en la pantalla
		{
			display.setCursor(13,cursor);								//Se coloca el cursor en la posicion 12 de columna y la fila 0
			if (cursor==0) {
			cursor = 33;		
			}else {
			cursor = 0;
			}
			if (menu_submenu_state == MAIN){
				display.setTextSize(2);      // establece tamano de texto en 2
  				display.setTextColor(WHITE);
				display.print(menu[i]);										//Muestra solo el array del menu principal
				display.display(); 
			}
			else if (menu_submenu_state == AJUSTES_SUBMENU){
				display.setTextSize(2);
				display.setTextColor(WHITE);
				display.print(ajustes[i]);									//Muestra solo el array del submenu ajustes
				display.display(); 
			}
			else if (menu_submenu_state == MEDICION_SUBMENU){
				display.setTextSize(2);
				display.setTextColor(WHITE);
				display.print(medicion[i]);									//Muestra solo el array el submenu medicion
				display.display(); 
			}
			else if (menu_submenu_state == HELICE_SUBMENU){
				display.setTextSize(2);
				display.setTextColor(WHITE);
				display.print(helice[i]);									
				display.display();
			}
		}
	}
}

/*Funcion que se encarga de realizar las acciones dentro de cada menu*/
bool StateMachine_Control(Menu_e Menu, Menu_state_e menu_submenu_state)
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
				display.setTextSize(2);      // establece tamano de texto en 2
  				display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
				display.print("T = ");
				float periodo = GetEEPROMValueF(PERIODO);
				display.setCursor(40,0);
				display.print(periodo/1000); //Convierto de milisegundos a segundos
				display.print("sec");
				display.display();

				unsigned long lastmillis = millis();
				unsigned long timedone = 0; 		//Se le asigna el tiempo transcurrido dentro del while loop siguiente

				while(buttonOut)
				{
					if (CheckButton() != ENTER)
					{
						timedone = millis() - lastmillis;
						if(timedone >= periodo)
						{
							buttonOut = 0;
						}
					}
					else{
						buttonOut = 0;
					}
					
					/* 	Muestra un punto cada 1 segundo transcurrido
						Esto es solo un ejemplo */
					if(timedone == dotcount)
					{
						display.setCursor(dotiter,17);
						display.setTextSize(2);      // establece tamano de texto en 2
  						display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
						display.print(".");
						display.display();
						dotcount = dotcount + 1000.0;
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
			}
			estado_actual = INICIO_MEDICION;
			return 1;
		}
		break;

		/*Funcion para configurar la fecha y hora y guardarla en la EEPROM*/
		case TOMAR_FECHA_HORA:
		{
			move_t buttonProcess = DONTMOVE;
			uint8_t day = GetEEPROMValueB(DAY);
			uint8_t month = GetEEPROMValueB(MONTH);
			uint8_t hour = GetEEPROMValueB(HOUR);
			uint8_t minutes = GetEEPROMValueB(MINUTES);
			uint8_t year = GetEEPROMValueB(YEAR);
			uint8_t enterCount = 0;
			bool outFechaHora = 1;
			bool setDay = 1;
			bool setMonth = 1;
			bool setHour = 1;
			bool setMinutes = 1;
			bool setYear = 1;
			bool buttonOut = 1;
			while(outFechaHora){
				lcd_setSpaces(day,month,year,hour,minutes);
				while(setDay)
				{
					buttonProcess = CheckButton();
					switch(buttonProcess)
					{
						case DONTMOVE:
						break;
						case UP:
						{
							if(day < 31){
								day++;					//Si se oprime el boton hacia arriba se incrementa en uno
							}
							display.setCursor(0,17);
							if (day < 10)				//Se imprime en la pantalla el valor, se pone un 0 adelante en el caso de que sea <10
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(day);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(day);
								display.display();
							}
						}
						break;

						case DOWN:
						{
							if(day > 1){
								day--;
							}
							display.setCursor(0,17);
							if (day < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(day);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(day);
								display.display();
							}
						}
						break;

						case ENTER:
						{
							setDay = 0;
							enterCount++;
						}
						break;
					}
				}

				while(setMonth)
				{
					buttonProcess = CheckButton();
					switch(buttonProcess)
					{
						case DONTMOVE:
						break;
						case UP:
						{
							if(month < 12){
								month++;
							}
							display.setCursor(31,17);
							if (month < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(month);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(month);
								display.display();
							}
						}
						break;

						case DOWN:
						{
							if(month > 1){
								month--;
							}
							display.setCursor(31,17);
							if (month < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(month);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(month);
								display.display();
							}
						}
						break;

						case ENTER:
						{
							setMonth = 0;
							enterCount++;
						}
						break;
					}
				}

				while(setYear)
				{
					buttonProcess = CheckButton();
					switch(buttonProcess)
					{
						case DONTMOVE:
						break;
						case UP:
						{
							if(year < 50){
								year++;
							}
							display.setCursor(61,17);
							if (year < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(year);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(year);
								display.display();
							}
						}
						break;

						case DOWN:
						{
							if(year > 1){
								year--;
							}
							display.setCursor(61,17);
							if (year < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(year);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(year);
								display.display();
							}
						}
						break;

						case ENTER:
						{
							setYear = 0;
							enterCount++;
						}
						break;
					}
				}

				while(setHour)
				{
					buttonProcess = CheckButton();
					switch(buttonProcess)
					{
						case DONTMOVE:
						break;
						case UP:
						{
							if(hour < 23){
								hour++;
							}
							display.setCursor(91,17);
							if (hour < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(hour);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(hour);
								display.display();
							}
						}
						break;

						case DOWN:
						{
							if(hour > 0){
								hour--;
							}
							display.setCursor(91,17);
							if (hour < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(hour);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(hour);
								display.display();
							}
						}
						break;

						case ENTER:
						{
							setHour = 0;
							enterCount++;
						}
						break;
					}
				}

				while(setMinutes)
				{
					buttonProcess = CheckButton();
					switch(buttonProcess)
					{
						case DONTMOVE:
						break;
						case UP:
						{
							if(minutes < 59){
								minutes++;
							}
							display.setCursor(111,17);
							if (minutes < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(minutes);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(minutes);
								display.display();
							}
						}
						break;

						case DOWN:
						{
							if(minutes > 0){
								minutes--;
							}
							display.setCursor(111,17);
							if (minutes < 10)
							{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print("0");
								display.print(minutes);
								display.display();
							}
							else{
								display.setTextSize(2);      // establece tamano de texto en 2
  								display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
								display.print(minutes);
								display.display();
							}
						}
						break;

						case ENTER:
						{
							setMinutes = 0;
							enterCount++;
						}
						break;
					}
				}

				UpdateEEPROMValueB(DAY,day);
				UpdateEEPROMValueB(MONTH,month);
				UpdateEEPROMValueB(YEAR,year);
				UpdateEEPROMValueB(HOUR,hour);
				UpdateEEPROMValueB(MINUTES,minutes);
				outFechaHora = 0;
			}
			estado_actual = CFG_DATE;
			return 1;
		}
		break;

		case TOMAR_PERIODO:
		{
			bool outPeriodo = 1;
			move_t buttonProcess = DONTMOVE;
			float periodo = GetEEPROMValueF(PERIODO);
			display.clearDisplay();
			display.setCursor(0,0);
			display.setTextSize(2);      // establece tamano de texto en 2
  			display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
			display.print("Periodo");
			display.setCursor(0,17);
			display.print(periodo);
			display.print(" mSeg");
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
						periodo = periodo + 100;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						display.setCursor(0, 17);   // ubica cursor en coordenadas 28,34
  						display.setTextSize(2);      // establece tamano de texto en 2
 						display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
						display.print(periodo);
						display.print("mSeg");
						display.display();	
					}
					break;

					case DOWN:
					{
						if((periodo - 100) != 0){			//Si se oprime el boton hacia arriba se decrementa el periodo 
							periodo = periodo - 100;
						}
						display.setCursor(0, 17);   // ubica cursor en coordenadas 28,34
  						display.setTextSize(2);      // establece tamano de texto en 2
 						display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
						display.print(periodo);
						display.print("mSeg");
						display.display();
					}
					break;
				}
			}
			SetEEPROMValueF(PERIODO,periodo);	
		}
		break;
	}
	return 0;
} 

void lcd_setSpaces(uint8_t day, uint8_t month, uint8_t year, uint8_t hour, uint8_t minutes)
{
	display.clearDisplay();      // limpia pantalla      
  	display.setCursor(0, 0);   // ubica cursor en coordenadas 28,34
  	display.setTextSize(2);      // establece tamano de texto en 2
  	display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
	display.print("Cfg. fecha/hora");
	display.setCursor(0,17);
	lcd_setValueB(day);
	display.setCursor(21,17);
	display.print("/");
	display.setCursor(31,17);
	lcd_setValueB(month);
	display.setCursor(51,17);
	display.print("/");
	display.setCursor(61,17);
	lcd_setValueB(year);
	display.setCursor(81,17);
	lcd_setValueB(hour);
	display.setCursor(91,17);
	display.print(":");
	display.setCursor(101,17);
	lcd_setValueB(minutes);
	display.display();
}

void lcd_setValueB(uint8_t value)
{
	if (value < 10)
	{
		display.setTextSize(2);      // establece tamano de texto en 2
  		display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
		display.print("0");
		display.print(value);
		display.display();
	}
	else
	{
		display.setTextSize(2);      // establece tamano de texto en 2
  		display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
		display.print(value);
		display.display();
	}
}
