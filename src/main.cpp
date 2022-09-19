#include "main.h"

Adafruit_SH1106 display(OLED_SDA, OLED_SCL);
Preferences preferences; // Flash

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

/* ---------------------------------------------------------------------------------------------- */
/* Button related functions */

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



/* ---------------------------------------------------------------------------------------------- */
/* State Machine related functions */

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
				float A = preferences.getFloat("CONSTANTE_A", 0);
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
				display.print(A);
				display.display();
				display.setCursor(0,40);
				display.print(v,4);
				display.display();
				delay(10000);

			
			}
			estado_actual = INICIO_MEDICION;
			return 1;
			
		}
		break;
			
		case TOMAR_PERIODO:
		
		{
			bool outPeriodo = 1;
			move_t buttonProcess = DONTMOVE;
			int periodo = preferences.getInt("PERIODO_MEMO", 0);
			setConfigDisplayParam("   Config. Periodo");
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
						setConfigDisplayParam("   Config. Periodo");
						display.print(periodo);
						display.display();
					}
					break;

					case DOWN:
					{
						if((periodo - 5000) != 0){			//Si se oprime el boton hacia arriba se decrementa el periodo 
							periodo = periodo - 5000;
						}
						setConfigDisplayParam("   Config. Periodo");
						display.print(periodo);
						display.display();
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
			setConfigDisplayParam("  Valor A Helice 1");
			display.print(CONSTANTE_A1);
			display.display();
			
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
						setConfigDisplayParam("  Valor A Helice 1");
						display.print(CONSTANTE_A1);
						display.display();
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_A1 = CONSTANTE_A1 - 0.01;
						setConfigDisplayParam("  Valor A Helice 1");
						display.print(CONSTANTE_A1);
						display.display();
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
			setConfigDisplayParam("  Valor B Helice 1");
			display.print(CONSTANTE_B1);
			display.display();
			
			while(outConstante_B1)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_B1 = 0;
						estado_actual = VALOR_A1;
					}
					break;

					case UP:
					{
						CONSTANTE_B1 = CONSTANTE_B1 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						setConfigDisplayParam("  Valor B Helice 1");
						display.print(CONSTANTE_B1);
						display.display();
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_B1 = CONSTANTE_B1 - 0.01;
						setConfigDisplayParam("  Valor B Helice 1");
						display.print(CONSTANTE_B1);
						display.display();
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
			setConfigDisplayParam("   Valor A Helice 2");
			display.print(CONSTANTE_A2);
			display.display();
			
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
						setConfigDisplayParam("   Valor A Helice 2");
						display.print(CONSTANTE_A2);
						display.display();
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_A2 = CONSTANTE_A2 - 0.01;
						setConfigDisplayParam("   Valor A Helice 2");
						display.print(CONSTANTE_A2);
						display.display();
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
			setConfigDisplayParam("  Valor B Helice 2");
			display.print(CONSTANTE_B2);
			display.display();
			
			while(outConstante_B2)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_B2 = 0;
						estado_actual = VALOR_A2;
					}
					break;

					case UP:
					{
						CONSTANTE_B2 = CONSTANTE_B2 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						setConfigDisplayParam("  Valor B Helice 2");
						display.print(CONSTANTE_B2);
						display.display();
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_B2 = CONSTANTE_B2 - 0.01;
						setConfigDisplayParam("  Valor B Helice 2");
						display.print(CONSTANTE_B2);
						display.display();
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
			setConfigDisplayParam("   Valor A Helice 3");
			display.print(CONSTANTE_A3);
			display.display();
			
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
						setConfigDisplayParam("   Valor A Helice 3");
						display.print(CONSTANTE_A3);
						display.display();
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_A3 = CONSTANTE_A3 - 0.01;
						setConfigDisplayParam("   Valor A Helice 3");
						display.print(CONSTANTE_A3);
						display.display();
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
			setConfigDisplayParam("  Valor B Helice 3");
			display.print(CONSTANTE_B3);
			display.display();
			
			while(outConstante_B3)
			{
				buttonProcess = CheckButton();
				switch(buttonProcess)
				{
					case DONTMOVE:break;
					case ENTER:
					{
						outConstante_B3 = 0;
						estado_actual = VALOR_A3;
					}
					break;

					case UP:
					{
						CONSTANTE_B3 = CONSTANTE_B3 + 0.01;			//Si se oprime el boton hacia arriba se incrementa el periodo 
						setConfigDisplayParam("  Valor B Helice 3");
						display.print(CONSTANTE_B3);
						display.display();
					}
					break;

					case DOWN:
					{
								
						CONSTANTE_B3 = CONSTANTE_B3 - 0.01;
						setConfigDisplayParam("  Valor B Helice 3");
						display.print(CONSTANTE_B3);
						display.display();
					}
					break;
				}
			}
			preferences.putFloat("CONSTANTE_B3", CONSTANTE_B3);
			

		}
		break;
	
	}
	return 0;
}


/* ---------------------------------------------------------------------------------------------- */
/* Interrupts functions */

void IRAM_ATTR isr_helice()
{
	contador_helice++;
}


// TODO: Hacer un wrapper de las funciones de display y separarlas del main.cpp
/* ---------------------------------------------------------------------------------------------- */
/* LCD related functions */

// TODO: Checkear si se usa esta funcion
void lcd_ClearOneLine(int row)
{
	for(uint8_t i=0; i < COLNUM ; i++)
	{
		display.setCursor(i,row);
		display.print(" ");
		display.display(); 
	}
}

// TODO: Checkear si se usa esta funcion
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

void setMenuDisplay(String name, String arr)
{
	display.print(name);
	display.setCursor(0, 17);   // ubica cursor en coordenadas 0,14
	display.setTextSize(2);      // establece tamano de texto en 2
	display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
	display.print(arr);										//Muestra solo el array del menu principal
	display.display(); 							//Muestra solo el array del menu principal
}

/* Funcion para imprimir en pantalla*/
void lcd_PrintCursor(Menu_state_e menu_submenu_state, uint8_t start, uint8_t count) //Estados, donde comienza en el array que corresponde
{
	setConfigDisplay();
	
	if (count <= ROWNUM){
		for (uint8_t i=start; i<count+start; i++)				//Cuenta para mostrar los menu en la pantalla
		{
			
			display.setCursor(0,0);
			display.setTextSize(1);
  			display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)

			if (menu_submenu_state == MAIN){
				setMenuDisplay("    Menu Principal", menu[i]);
			}
			else if (menu_submenu_state == AJUSTES_SUBMENU){
				setMenuDisplay("        Ajustes", ajustes[i]);	
			}
			else if (menu_submenu_state == MEDICION_SUBMENU){
				setMenuDisplay("       Medicion", medicion[i]);
			}
			else if (menu_submenu_state == HELICE_SUBMENU){	
				setMenuDisplay("Config. Helices", helice[i]);
			}
			else if (menu_submenu_state == HELICE_1_SUBMENU){	
				setMenuDisplay("Config. Helice 1", constante1[i]);
			}
			else if (menu_submenu_state == HELICE_2_SUBMENU){	
				setMenuDisplay("Config. Helice 2", constante2[i]);
			}
			else if (menu_submenu_state == HELICE_3_SUBMENU){	
				setMenuDisplay("Config. Helice 3", constante3[i]);
			}
		}
	}
}


void setConfigDisplay(void)
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
}

void setConfigDisplayParam(String param_title)
{
	display.clearDisplay();      // limpia pantalla      
  	display.drawLine(0, 10, 128, 10, WHITE); // dibuja linea
	display.drawLine(0, 54, 128, 54, WHITE); // dibuja linea 
  	display.setCursor(0,0);
  	display.setTextSize(1);
  	display.setTextColor(WHITE);
	display.print(param_title);
	display.setCursor(0,56);
 	display.cp437(true);
  	display.write(0x1E);
  	display.setCursor(58,56);
  	display.print("OK"); 
  	display.setCursor(121,56);
  	display.cp437(true);
  	display.write(0x1F);
	display.setCursor(0,20);
  	display.setTextSize(2);
	display.display();
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
					estado_actual = HELICE_1;
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
					estado_actual = HELICE_1;
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