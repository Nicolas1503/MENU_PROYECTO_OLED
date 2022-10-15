#include <Arduino.h>
#include "SPI.h"
#include "FS.h"
#include "SD.h"

#define ss_SD 15

void SD_Begin(void);
void SD_OpenFile(const char * FILE_NAME);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);