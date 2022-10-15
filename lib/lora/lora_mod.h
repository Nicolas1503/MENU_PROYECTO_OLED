#include <Arduino.h>
#include <SPI.h>
#include <Lora.h>

#define ss_lora 5
#define reset_lora 25
#define dio0_lora 33

void LoRa_begin(void);
void LoRa_SendPacket(float VEL_VALUE, int PERIODO_VALUE, uint8_t HELICE_VALUE);