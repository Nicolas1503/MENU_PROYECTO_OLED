#include "display_mod.h"

Adafruit_SH1106 display(OLED_SDA, OLED_SCL);

void display_begin(void){

    display.begin(SH1106_SWITCHCAPVCC, 0x3C); // inicializa pantalla con direccion 0x3C
}

void display_clear(void){
	display.clearDisplay();
}

void lcd_ClearOneLine(int row){
	for(uint8_t i=0; i < COLNUM ; i++)
	{
		display.clearDisplay();
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

void display_notif(String NOTIFICACION){

	display.clearDisplay();      // limpia pantalla      
  	display.drawLine(0, 10, 128, 10, WHITE); // dibuja linea
	display.drawLine(0, 54, 128, 54, WHITE); // dibuja linea 
	display.setCursor(0,0);
	display.setTextSize(1);
  	display.setTextColor(WHITE);
	display.print("Cargando...");
	display.setCursor(0, 17);   // ubica cursor en coordenadas 0,14
	display.setTextSize(2);      // establece tamano de texto en 2
	display.print(NOTIFICACION);										//Muestra solo el array del menu principal
	display.display(); 							//Muestra solo el array del menu principal

}


void display_background(bool modo_flechas){

	display.clearDisplay();      // limpia pantalla      
  	display.drawLine(0, 10, 128, 10, WHITE); // dibuja linea
	display.drawLine(0, 54, 128, 54, WHITE); // dibuja linea 
	if ( modo_flechas == true){
	display.setCursor(0,56);
  	display.setTextSize(1);
  	display.setTextColor(WHITE);
	display.cp437(true);
  	display.write(0x11);
	display.setCursor(121,56);
  	display.cp437(true);
  	display.write(0x10);
	} else {
	display.setCursor(0,56);
  	display.setTextSize(1);
  	display.setTextColor(WHITE);
	display.cp437(true);
 	display.write(0x1E);
	display.setCursor(121,56);
  	display.cp437(true);
  	display.write(0x1F);
	}
 	display.setCursor(58,56);
  	display.print("OK"); 
	display.display(); 
}

void display_showmenu(String menu_title, String arr){
	display.setCursor(0,0);
	display.setTextSize(1);
  	display.setTextColor(WHITE);
	display.print(menu_title);
	display.setCursor(0, 17);   // ubica cursor en coordenadas 0,14
	display.setTextSize(2);      // establece tamano de texto en 2
	display.setTextColor(WHITE);   // establece color al unico disponible (pantalla monocromo)
	display.print(arr);										//Muestra solo el array del menu principal
	display.display(); 							//Muestra solo el array del menu principal
}

void display_showparam(String param_title, float param_value, bool entero, String param_unit){
	display.setCursor(0,0);
  	display.setTextSize(1);
  	display.setTextColor(WHITE);
	display.print(param_title);
	display.setCursor(30,20);
  	display.setTextSize(2);
	if (entero == true){
	display.print(param_value, 0);
	}else {
	display.print(param_value);
	}
	display.print(param_unit);
	display.display(); 
}

void display_MedicionMode(uint8_t helice_num,int periodo){
	
	display.clearDisplay();
	display.drawRect(0, 0, 128, 56, WHITE);
	display.drawLine(0, 18, 128, 18, WHITE);
	display.drawLine(0, 57, 128, 57, WHITE);
	display.setCursor(2,2);
	display.setTextSize(2);
  	display.setTextColor(WHITE);
	display.print(" Midiendo");
	display.setCursor(4,20);
	display.print("T=");
	display.print(periodo/1000);
	display.print(" Seg.");
	display.setCursor(4,37);
	display.print("Helice: ");
	display.print(helice_num);
	display.display();
}

	void display_ShowDotiter(uint8_t dot_iter, float dot_count){
	display.setCursor(dot_iter,48);
	display.setTextSize(2);
  	display.setTextColor(WHITE);
	display.print(".");
	dot_count = dot_count + 1000.0;
	display.display();
	if(dot_iter < 128)
	{
		dot_iter = dot_iter + 5;
	}
	else{
			lcd_ClearOneLine(1);
			dot_iter = 0;
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

void display_ShowHeliceSelected(uint8_t helice_num, float value_A, float value_B){
	display.clearDisplay();
	display.drawRect(0, 0, 128, 52, WHITE); // dibuja rectangulo
	display.drawLine(0, 40, 128, 40, WHITE);
	display.drawLine(0, 54, 128, 54, WHITE);
	display.setCursor(15,5);
	display.setTextColor(WHITE);
	display.setTextSize(2);
	display.print("Helice ");
	display.print(helice_num);
	display.print("   Elegida");
	display.setCursor(5,42);
	display.setTextSize(1);
	display.print("A:");
	display.print(value_A);
	display.setCursor(80,42);
	display.print("B:");
	display.print(value_B);
	display.setCursor(58,56);
  	display.print("OK"); 
	display.display();
}

void display_ShowMedidaGuardada(String file_name){
	display.clearDisplay();
	display.drawRect(0, 0, 128, 52, WHITE); // dibuja rectangulo
	display.drawLine(0, 40, 128, 40, WHITE);
	display.drawLine(0, 54, 128, 54, WHITE);
	display.setCursor(15,5);
	display.setTextColor(WHITE);
	display.setTextSize(2);
	display.print("Guardado   en SD");
	display.setCursor(5,42);
	display.setTextSize(1);
	display.print("Archivo:");
	display.print(file_name);
	display.setCursor(58,56);
  	display.print("OK"); 
	display.display();
}

void display_ShowMedidaEnviada(void){
	display.clearDisplay();
	display.drawRect(0, 0, 128, 52, WHITE); // dibuja rectangulo
	display.drawLine(0, 40, 128, 40, WHITE);
	display.drawLine(0, 54, 128, 54, WHITE);
	display.setCursor(15,5);
	display.setTextColor(WHITE);
	display.setTextSize(2);
	display.print("Enviado");
	display.setCursor(5,42);
	display.setTextSize(1);
	//display.print("Archivo:");
	//display.print(file_name);
	display.setCursor(58,56);
  	display.print("OK"); 
	display.display();
}

void display_showFechaHora(String VALOR_HORA, String VALOR_MINUTO, String VALOR_DIA, String VALOR_MES, String VALOR_ANIO){

display.clearDisplay();
display.drawLine(0, 10, 128, 10, WHITE); // dibuja linea
display.drawLine(0, 54, 128, 54, WHITE); // dibuja linea 
display.setTextSize(2);
display.setCursor(30,15);
display.print(VALOR_HORA);
display.setCursor(55,15);
display.print(":");
display.setCursor(70,15);
display.print(VALOR_MINUTO);
display.setCursor(10,35);
display.print(VALOR_DIA);
display.setCursor(35,35);
display.print("/");
display.setCursor(50,35);
display.print(VALOR_MES);
display.setCursor(75,35);
display.print("/");
display.setCursor(90,35);
display.print(VALOR_ANIO);
display.setTextSize(1);
display.setCursor(58,56);
display.print("OK"); 
display.display();

} 