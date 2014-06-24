#include <EEPROM.h>
#include <Ethernet.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WebServer.h>
#include "EEPROM_SETUP_WEBFORM.h"
#include "StartSettings.h"






void setup()
{
  Serial.begin(SERIAL_BAUD);
  
  setupSensorData(); //Sensordaten aus EEPROM auslesen

  if(DEBUGMODE) Serial.println(F("DEBUGMODE ON"));

  delay(200);
  Serial.print(F("Initialisiere Ethernet"));
  setupNetwork();  // EthernetShield mit den Werten aus dem EEPROM initialisieren
  Serial.println(F(" [OK]"));
  delay(200);
  
  setupServerSettings(); //Einstellungen für Server aus EEPROM lesen  
  
  #define PREFIX ""
  webserver = new WebServer(PREFIX, eeprom_config.webserverPort);
  webserver->setDefaultCommand(&indexHTML);                                 // Startseite
  webserver->setFailureCommand(&errorHTML);                                 // Fehlerseite
  webserver->addCommand("index.html", &indexHTML);                          // index.html = Startseite
  webserver->addCommand("setupNet.html", &setupNetHTML);                    // Seite zum eingeben der Netzwerkdaten
  webserver->addCommand("setupServer.html", &setupServerHTML);              // Seite zum Eingeben der Serverdaten
  webserver->addCommand("selectSensor.html", &selectSensor);                // Seite zur Auswahl eines Sensors zum bearbeiten
  webserver->addCommand("setupDeviceAddresses.html", &setupDeviceAdresses); // Seite zum bearbeiten eines Sensors
  webserver->begin(); 
  
  initializeSensors();
  
  Serial.println(F("Setup Ende..."));
  
  //Alle Ausgänge als Output setzen und auf HIGH (aus)
  int pin1;
  int pin2;
  
  for(int sensorNr=0; sensorNr<numSensors; sensorNr++) 
  {
    pin1 = eeprom_config.sensorControl[sensorNr][4];
    pin2 = eeprom_config.sensorControl[sensorNr][5];
    
    pinMode(pin1, OUTPUT);
    digitalWrite(pin1, HIGH);
    
    if(pin2 != 0) //Wenn pin2 != 0 ist, dann handelt es sich um einen Sensor für Mischersteuerung 
    {
      pinMode(pin2, OUTPUT);
      digitalWrite(pin2, HIGH);
    }    
  }  
}





void loop()
{
  char buff[200];
  int len = 200;

  webserver->processConnection(buff, &len);
  
  unsigned long currentMillis = millis();
  
  switchPump();              // Pumpe einschalten wenn Temp unter Minimal fällt und abschalten wenn Temp über Maximal steigt
  
  //Mischermotoren Steuerung
  if(currentMillis - prevMixerControllMillis > (mixerControlMeasureInterval * 1000)) {
    mixerControl();          // Mischermotoren steuern wenn Temp unter Minimal fällt oder über Maximal steigt
    prevMixerControllMillis = currentMillis; 
  }
 
 
  //Temperaturen Upload auf Server
  if(currentMillis - previousMillis > (UPLOADINTERVAL*1000)) {
    if(DEBUGMODE) showSensorData();
    
    sendTemperatureData();
    previousMillis = currentMillis; 
  }
}
