
//Librerias
#include <SPI.h>
#include <MFRC522.h>

#include "FS.h"
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true

//Definicion de pines en el esp32
#if defined(ESP32)
#define SS_PIN 5
#define RST_PIN 4
#define ledrojo 12
#define ledverde 27
#endif

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Instancia.

//Metodo para listar directorio
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}
//Fin metodo listar

//Metodo lectura archivo
void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

//Fin metodo lectura
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("- message appended");
    } else {
        Serial.println("- append failed");
    }
    file.close();
}

void setup() {
  //Inidicar el modo de los pines de los leds
  pinMode(ledverde, OUTPUT);
  pinMode(ledrojo, OUTPUT);
  //Inicar serial
  Serial.begin(115200);
  Serial.println();
   if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
  //Listar archivos almacenados
  listDir(SPIFFS, "/", 0);
  //Metodo para leer el archivo datos.txt
  readFile(SPIFFS, "/datos.txt");
  // Iniciar  SPI bus
  SPI.begin();
  // Iniciar MFRC522
  mfrc522.PCD_Init();
  Serial.println("Coloca tu tarjeta o llavero.");
}

void loop() {

  // En espera
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Lectura
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  //Mostrar en monitor serial el UID
  Serial.print("UID  :");
  String content = "";
  //Ciclo para dar formato al UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {

    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  //Mensaje de verificacion
  Serial.println();
  Serial.print("Mensaje : ");
  content.toUpperCase();
  //Validacion de usuario
  if (content.substring(1) == "11 82 4D 23")  //Se puede cambiar el valor o valores a verificar.
  {
    Serial.println("Acceso autorizado");
    Serial.println();
    digitalWrite(ledverde, HIGH);
    delay(500);
    digitalWrite(ledverde, LOW);
    //Variable String para almacenar UID
    String linea; 
    //Agregando salto de linea y valor a la variable linea
    linea = String(content)+"\r\n";
    //Metodo para agregar variable haciendo uso de tambien de la conversion c_str()
    appendFile(SPIFFS,"/datos.txt", linea.c_str());
    //Mostramos mensaje con el dato almacenado
    Serial.print("UID de tarjeta almacenado en archivo.txt :");
    Serial.println(content);
    Serial.println();   
    Serial.println();
     delay(5000);
   readFile(SPIFFS,"/datos.txt");
  }

  else {
    Serial.println("Acceso denegado");
    Serial.println();
    digitalWrite(ledrojo, HIGH);
    delay(500);
    digitalWrite(ledrojo, LOW);
    //Variable String para almacenar UID
    String linea; 
    //Agregando salto de linea y valor a la variable linea
    linea = String(content)+"\r\n";
    //Metodo para agregar variable haciendo uso de tambien de la conversion c_str()
    appendFile(SPIFFS,"/datos.txt", linea.c_str());
    //Mostramos mensaje con el dato almacenado
    Serial.print("UID de tarjeta almacenado en archivo.txt :");
    Serial.println(content);
     Serial.println();   
    Serial.println();
  delay(5000);
   readFile(SPIFFS,"/datos.txt");
     
  }

}
