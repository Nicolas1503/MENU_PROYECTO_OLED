#include "display_mod.h"

Adafruit_SH1106 display(OLED_SDA, OLED_SCL);

void display_begin(void){

    display.begin(SH1106_SWITCHCAPVCC, 0x3C); // inicializa pantalla con direccion 0x3C

}


void display_clear(void){
	display.clearDisplay();
}

void setConfigDisplayParam(String param_title, float param_value, String param_unit){
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
	display.setCursor(30,20);
  	display.setTextSize(2);
	display.print(param_value);
	display.print(param_unit);
	display.display();
	display.display(); 
}


void setConfigDisplay(void){
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
	display.setCursor(0,0);
	display.setTextSize(1);
  	display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
	display.display(); 
}

void lcd_ClearOneLine(int row){
	for(uint8_t i=0; i < COLNUM ; i++)
	{
		display.setCursor(i,row);
		display.print(" ");
		display.display(); 
	}
}

void lcd_ClearCursor(int row){
	display.setCursor(0,row);
	display.print(" ");
	display.display(); 
}

void setMenuDisplay(String name, String arr){
	display.print(name);
	display.setCursor(0, 17);   // ubica cursor en coordenadas 0,14
	display.setTextSize(2);      // establece tamano de texto en 2
	display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
	display.print(arr);										//Muestra solo el array del menu principal
	display.display(); 							//Muestra solo el array del menu principal
}

void display_ShowPeriod(int periodo){
	
	display.clearDisplay(); 
	display.setCursor(0,0);
	display.setTextSize(2);
  	display.setTextColor(WHITE);
	display.print("Midiendo");
	display.setCursor(0,17);
	display.print("T= ");
	display.print(periodo/1000);
	display.print(" Seg.");
	display.display();
}

void display_ShowDotiter(void){
	float dotcount = 1000.0;
	uint8_t dotiter = 0;
	display.setCursor(dotiter,33);
	display.setTextSize(2);
  	display.setTextColor(WHITE);
	display.print(".");
	dotcount = dotcount + 1000.0;
	display.display();
	if(dotiter < 128)
	{
		dotiter = dotiter + 5;
	}
	else{
		lcd_ClearOneLine(1);
		dotiter = 0;
	}
}

void display_ShowSpeed(int cuenta, float velocidad ){

	display.clearDisplay(); 
	display.setCursor(0,0);
	display.setTextSize(2);
  	display.setTextColor(WHITE);
	display.setCursor(0,0);
	display.print("  Pulsos: ");
	display.setCursor(54,17);
	display.print(cuenta);
	display.setCursor(0,33);
	display.print("Velocidad:");
	display.print(velocidad);
	display.print(" m/Seg");
	display.display();
}

void display_ShowHeliceSelected(String title_helice, float value_A, float value_B){
	display.clearDisplay();
	display.setCursor(0,0);
	display.setTextColor(WHITE);
	display.setTextSize(2);
	display.print(title_helice);
	display.setCursor(0,33);
	display.print("A:");
	display.print(value_A);
	display.setCursor(0,49);
	display.print("B:");
	display.print(value_B);
	display.display();
}
