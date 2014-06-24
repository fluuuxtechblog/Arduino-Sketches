/*****************************************************************************************************
  Datei:         Sensors.ino                                                                         *
  Autor:         Enrico Sadlowski                                                                    *
  Erstellt:      17.05.2013                                                                          *
  Aktualisiert:  06.06.2013                                                                          *
  Beschreibung:  Funktionen zum ermitteln und ausgeben der Temperaturen von DALLAS ds1820 Sensoren.  *
  Schalten von Ausgängen bei unter- oder überschreiten von Schaltschwellen                           *
*****************************************************************************************************/

OneWire ds(DS1820Pin);
DallasTemperature sensors(&ds);

//Status der einzelnen Sensoren ob diese einen Ausgang auf HIGH geschaltet haben und Temperatur außerhalb des erlaubten Bereichs ist. 
byte pumpThresholdState[] = {};





/***********************
  Meldung für setup()  *
***********************/
void initializeSensors() {
  Serial.print(F("Initialisiere Sensoren"));
  sensors.begin();
  if(numSensors > 0) Serial.println(F(" [OK]")); else Serial.println(F("  [FEHLER]"));
}





/****************************************
  DeviceAddress eines Sensors ausgeben  *
****************************************/
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}





/*************************************
  Temperatur eines Sensors ausgeben  *
*************************************/
int getTempFromSensor(int SensorNr) {
  sensors.requestTemperatures();
  int temp = sensors.getTempC(eeprom_config.deviceaddress[SensorNr]);
  Serial.print(F("Sensor: "));
  Serial.print(SensorNr);
  Serial.print(F(" = "));
  Serial.println(temp);
  Serial.print(F("C"));
}





/**********************************************
  Daten aller Sensoren im Terminal ausgeben   *
**********************************************/
void showSensorData() {  
  Serial.println(); 
  sensors.requestTemperatures();
  
  Serial.println(F("SensorID:\tDeviceAdresse:\t\tTemperatur:"));
  Serial.println(F("____________________________________________________"));
  for(byte i=0; i<numSensors; i++) {
    float temp = sensors.getTempC(eeprom_config.deviceaddress[i]);
    Serial.print(i+1);
    Serial.print(F("\t\t"));
    printAddress(eeprom_config.deviceaddress[i]);
    Serial.print(F("\t"));
    if(temp != -127) Serial.println(temp,1); else Serial.println(F("N/A"));
  }
  Serial.println(); 
}





/****************************************************************************************
  Pumpensteuerung                                                                       *
  Wenn Temperatur an Sensor unter Minimal-Schaltschwelle fällt, dann Pumpe einschalten  *
  Pumpe erst abschalten wenn Wert über Maximal-Schaltschwelle des Sesnors steigt        *
****************************************************************************************/
void switchPump() {
  sensors.requestTemperatures();
  
  for(int sensorNr=0; sensorNr<numSensors; sensorNr++) {
    if(eeprom_config.sensorControl[sensorNr][1] == 1) {
      int temp = sensors.getTempC(eeprom_config.deviceaddress[sensorNr]);
      
      int minTemp = eeprom_config.sensorControl[sensorNr][2];
      int maxTemp = eeprom_config.sensorControl[sensorNr][3];
      int pin1 = eeprom_config.sensorControl[sensorNr][4];
      int pin2 = eeprom_config.sensorControl[sensorNr][5];
      
      //Temperatur unter Minimal = Status auf 1 setzen
      if(temp < minTemp && digitalRead(pin1) == HIGH) {
        pumpThresholdState[sensorNr] = 1;  
        digitalWrite(pin1, LOW);
       
        if(DEBUGMODE) {
          Serial.print(F("Temp an Sensor "));
          Serial.print(sensorNr);
          Serial.print(F(" liegt mit "));
          Serial.print(temp);
          Serial.print(F(" Grad unter der MinTemp-Schaltschwelle von "));
          Serial.print(minTemp);
          Serial.println(F(" Grad [Pumpe einschalten]"));
        } 
      }
     
      //Temperatur über Maximal = Status auf 0 setzen
      if(temp > maxTemp && digitalRead(pin1) == LOW) {
        pumpThresholdState[sensorNr] = 0; 
        digitalWrite(pin1, HIGH);
           
        if(DEBUGMODE) {
          Serial.print(F("Temp an Sensor "));
          Serial.print(sensorNr);
          Serial.print(F(" liegt mit "));
          Serial.print(temp);
          Serial.print(F(" Grad uber der MaxTemp-Schaltschwelle von "));
          Serial.print(maxTemp);
          Serial.println(F(" Grad [Pumpe abschalten]"));
        }
      }
    }
  }
}





/*******************************************************************************************************************
  Mischersteuerung                                                                                                 *
  Wenn Temperatur an Sensor unter Minimal-Schaltschwelle fällt, dann Mischermotor1 für 2 Sekunden einschalten      *
  Alle 10 Sekunden erneut Temperatur prüfen. Wenn Temperatur an Sensor über Maximal-Schaltschwelle steigt, dann    *
  Mischermotor2 für 2 Sekunden einschalten und wieder abschalten. Alle 10 Sekunden prüfen.                         *
******************************************************************************************************************/
void mixerControl() {
  unsigned long currentMillis = millis();
  sensors.requestTemperatures();
  
  if(currentMillis - prevMixerControlSwitchMillis > (mixerControlSwitchTimeInterval * 1000)) {
    for(int sensorNr=0; sensorNr<numSensors; sensorNr++) {
      if(eeprom_config.sensorControl[sensorNr][1] == 2) {
        int temp = sensors.getTempC(eeprom_config.deviceaddress[sensorNr]);
        
        int minTemp = eeprom_config.sensorControl[sensorNr][2];
        int maxTemp = eeprom_config.sensorControl[sensorNr][3];
        int pin1 = eeprom_config.sensorControl[sensorNr][4];
        int pin2 = eeprom_config.sensorControl[sensorNr][5];
        
        
        if(temp < minTemp) {     // Temperatur unter Minimal fällt, Mischermotor1 für 2 Sekunden auf HIGH schalten      
          if(DEBUGMODE) {
            Serial.print(F("Temp an Sensor "));
            Serial.print(sensorNr);
            Serial.print(F(" liegt mit "));
            Serial.print(temp);
            Serial.print(F(" Grad unter der Soll-Temp von "));
            Serial.print(minTemp);
            Serial.print(F(" Grad, [Mischer1 fur "));
            Serial.print(mixerControlSwitchTimeInterval);
            Serial.println(F(" Sek einschalten]"));
          }
          digitalWrite(pin1, LOW);
          delay(mixerControlSwitchTimeInterval * 1000);
          digitalWrite(pin1, HIGH);
        }
        else if(temp > maxTemp) { // Temperatur liegt über Maximum, Mischermotor2 für 2 Sekunden auf HIGH schalten  
          if(DEBUGMODE) {
            Serial.print(F("Temp an Sensor "));
            Serial.print(sensorNr);
            Serial.print(F(" liegt mit "));
            Serial.print(temp);
            Serial.print(F(" Grad uber der Soll-Temp von "));
            Serial.print(minTemp);
            Serial.print(F(" Grad [Mischer2 fur "));
            Serial.print(mixerControlSwitchTimeInterval);
            Serial.println(F(" Sek einschalten]"));
          }
          digitalWrite(pin2, LOW);
          delay(mixerControlSwitchTimeInterval * 1000);
          digitalWrite(pin2, HIGH);
        }
        else { //Temperatur liegt im vorgegebenen Bereich
          if(digitalRead(pin1) == LOW) digitalWrite(pin1, HIGH);
          if(digitalRead(pin2) == LOW) digitalWrite(pin2, HIGH);
        }
      }
    }
    prevMixerControlSwitchMillis = currentMillis;
  }
}





/****************************************************
  Funktion zum ermitteln der Device-Adressen aller  * 
  angeschlossenen DS18x20 Temperatursensoren        *
****************************************************/
void lookUpSensors() {
  byte address[8];
  int i=0;
  byte ok = 0, tmp = 0;
 
  Serial.println(); 
  while (ds.search(address)) {
    tmp = 0;
    if (address[0] == 0x10) {
      Serial.print("Sensor ist ein DS18S20 : ");
      tmp = 1;
    } 
    else {
      if (address[0] == 0x28) {
        Serial.print("Sensor ist ein DS18B20 : ");
        tmp = 1;
      }
    }
    if (tmp == 1) {
      if (OneWire::crc8(address, 7) != address[7]) {
        Serial.println("aber er hat keinen gultigen CRC!");
      } 
      else {
        for (i=0;i<8;i++) {
          if (address[i] < 16) {
            Serial.print('0');
          }
          Serial.print(address[i], HEX);
          if (i < 7) {
            Serial.print(",");
          }
        }
        Serial.print("\r\n");
        ok = 1;
      }
    }
  }
  if (ok == 0 && DEBUGMODE) {
    Serial.println("Keine Sensoren gefunden");
    Serial.println(); 
  }
}




void writeTimeToScratchpad(byte* address)
{
  ds.reset();
  ds.select(address);
  ds.write(0x44,1);
  delay(1000);
} 





void readTimeFromScratchpad(byte* address, byte* data)
{
  ds.reset();
  ds.select(address);
  ds.write(0xBE);
  for (byte i=0;i<9;i++)
  {
    data[i] = ds.read();
  }
}
