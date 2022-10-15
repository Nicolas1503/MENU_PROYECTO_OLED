#include "rtc_mod.h"

 
// -- Convert DECIMAL ==> BCD --
uint8_t decToBcd(uint8_t val)
{
  return ( (val/10*16) + (val%10) );
}

// -- Convert BCD ==> DECIMAL --
uint8_t bcdToDec(uint8_t val)
{
  return ( (val/16*10) + (val%16) );
}

// -- Print with 2 digits format
String dosDigitos (int dato)
{
String retorno = String(dato);
if (retorno.length() < 2) 
  retorno = "0" + retorno;
return retorno;
}


/*
Funcion que configura el RTC DS1307.
Carga los valores correspondientes a cada parámetro.

*/

void configuraRTC1307(int Hora, int Minuto, int Seg, int DiaSemana, int DiaMes, int Mes, int Anio)
{
  
  Wire.beginTransmission(DIRECCION_BASE_DS1307);     // Abre la linea en Modo Escritura
  Wire.write((uint8_t)0x00);                         // Setea el puntero de Registro (0x00)
  Wire.write(decToBcd(Seg));                         //Escribe 7 bytes
  Wire.write(decToBcd(Minuto));
  Wire.write(decToBcd(Hora));      
  Wire.write(decToBcd(DiaSemana));
  Wire.write(decToBcd(DiaMes));
  Wire.write(decToBcd(Mes));
  Wire.write(decToBcd(Anio));
  Wire.endTransmission();                    		  // Finaliza transmisión
  
  Wire.beginTransmission(DIRECCION_BASE_DS1307); 		 // Abre la linea en Modo Escritura
  Wire.write((uint8_t)0x00);                        	// Setea el puntero de Registro (0x00)
  Wire.endTransmission();                      		   // Finaliza transmisión
  
} 


 /*
 Funciones para leer cada parámetro por separado.
 Debo posicionarme según el registro para leer de a uno.
 
 */
 /*
 
uint8_t leeHora(void)
{
	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x02);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(bcdToDec(Wire.read() & 0x7f));  
}


uint8_t leeMinuto(void){
	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x01);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(bcdToDec(Wire.read()));  
}

uint8_t leeSegundo(void){
	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x00);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(bcdToDec(Wire.read() & 0x7f));  
}
uint8_t leeDiaSemana(void){

	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x03);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(bcdToDec(Wire.read()));  

}
uint8_t leeDiaMes(void){

	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x04);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(bcdToDec(Wire.read()));  

}
uint8_t leeMes(void){

	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x05);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(bcdToDec(Wire.read()));  

}
uint8_t leeAnio(void){

	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x06);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(bcdToDec(Wire.read()));  

}

*/
String leeHora(void)
{
	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x02);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(dosDigitos(bcdToDec(Wire.read() & 0x7f)));  
}


String leeMinuto(void){
	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x01);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(dosDigitos(bcdToDec(Wire.read())));  
}

String leeSegundo(void){
	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x00);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(dosDigitos(bcdToDec(Wire.read() & 0x7f)));  
}



String leeDiaSemana(void){

	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x03);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(dosDigitos(bcdToDec(Wire.read())));  

}
String leeDiaMes(void){

	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x04);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(dosDigitos(bcdToDec(Wire.read())));  

}
String leeMes(void){

	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x05);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(dosDigitos(bcdToDec(Wire.read())));  

}
String leeAnio(void){

	Wire.beginTransmission(DIRECCION_BASE_DS1307);   // Abre la linea en Modo Escritura
	Wire.write((uint8_t)0x06);                       // Setea el puntero de Registro (0x00)
	Wire.endTransmission();                       // Finaliza transmisión
  
	Wire.requestFrom(DIRECCION_BASE_DS1307, 1);      // Abre la linea I2C y lee 7 bytes
	return(dosDigitos(bcdToDec(Wire.read())));  

}