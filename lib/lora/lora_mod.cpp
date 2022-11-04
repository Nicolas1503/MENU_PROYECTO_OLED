#include "lora_mod.h"
#include "display_mod.h"

void LoRa_begin(void){

LoRa.setPins(ss_lora, reset_lora, dio0_lora);
pinMode(ss_lora, OUTPUT);
digitalWrite(ss_lora, HIGH);
	  
	  if (LoRa.begin(433E6)) {
		display_notif("Transmisor Iniciado");
		LoRa.setSpreadingFactor(12);           // ranges from 6-12,default 7 see API docsLoRa.setSpreadingFactor(12);
		LoRa.setSignalBandwidth(62.5E3);
		LoRa.setCodingRate4(8);
		delay(2000);
	  }else{
		display_notif("Transmisor   Error!");
		delay(2000);
	  }
}

void LoRa_SendPacket(float VEL_VALUE, int PERIODO_VALUE, uint8_t HELICE_VALUE){

    LoRa.beginPacket();
  	LoRa.print(VEL_VALUE);
	LoRa.print(";");
	LoRa.print(PERIODO_VALUE);
	LoRa.print( ";");
	LoRa.print(HELICE_VALUE);
	LoRa.endPacket();


}