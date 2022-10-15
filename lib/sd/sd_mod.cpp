#include "sd_mod.h"
#include "display_mod.h"

void SD_Begin(void) {
    if(!SD.begin(ss_SD)){
        display_notif(" Fallo en    SD!");
		delay(2000);
        //return;
    }else{
		display_notif("    SD     Iniciada");
		delay(2000);
	}
}

void SD_OpenFile(const char* FILE_NAME){
    File file = SD.open(FILE_NAME);
    	if(!file) {
   			display_notif("El archivo no existe");
            delay(1000);
    		display_notif("Creando   archivo...");
            delay(1000);
    		writeFile(SD, FILE_NAME, "Velocidad(m/Seg.);Periodo;Helice;Constante A;Constante B;Hora;Fecha\r\n");
    		}
    		else {
    		display_notif(" Abriendo  archivo");
            delay(1000);  
      		}
    		file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}