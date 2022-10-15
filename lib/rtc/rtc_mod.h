//#include "chip.h"
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

#define DIRECCION_BASE_DS1307 0x68


void configuraRTC1307(int Hora, int Minuto, int Seg, int DiaSemana, int DiaMes, int Mes, int Anio);
/*
uint8_t leeHora(void);
uint8_t leeMinuto(void);
uint8_t leeSegundo(void);
uint8_t leeDiaSemana(void);
uint8_t leeDiaMes(void);
uint8_t leeMes(void);
uint8_t leeAnio(void);
*/

String leeHora(void);
String leeMinuto(void);
String leeSegundo(void);
String leeDiaSemana(void);
String leeDiaMes(void);
String leeMes(void);
String leeAnio(void);


String dosDigitos (int dato);
uint8_t decToBcd(uint8_t val);
uint8_t bcdToDec(uint8_t val);