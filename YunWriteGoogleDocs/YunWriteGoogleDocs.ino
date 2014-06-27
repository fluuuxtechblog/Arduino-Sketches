/****************************************************************************************************************************
  Projekt: SaveSensorValues2GoogleDocs                                                                                      *
  Autor: Enrico Sadlowski                                                                                                   *
  Erstellungsdatum: 26.06.2014                                                                                              *
  Letzte Aktualisierung: 27.06.2014                                                                                         *
                                                                                                                            *
  Es werden ständig die Werte eines DHT22 und eines LDR Sensors ermittelt.                                                  *
  "productiveMode" == true, Die Werte werden an eine Google Drive Tabelle gesendet.                                         *
  "debugMode" == true, Es werden Meldungen über den Seriellen Port ausgegeben.                                              *
  Außer Helligkeit, Temperatur und Luftfeuchte wird noch das aktuelle Datum und die aktuelle Zeit.                          * 
  vom embedded Linux ausgelesen und mit in die Google Tabelle gespeichert.                                                  *
                                                                                                                            *
  Vorraussetzungen                                                                                                          *
  Google Account, temboo.com Account, Google Tabelle mit dem Namen "Yun" im Google Drive Account, Arduino Yun               *                                                                                                          *
****************************************************************************************************************************/

#include <DHT.h>
#include <Bridge.h>
#include <Temboo.h>
#include <Process.h>

#include "TembooAccount.h"                // Temboo Account Zugangsdaten
#include "GoogleAccount.h"                // Google Account Zugangsdaten


// Diese Daten anpassen
const boolean productiveMode    = true;   // Nur wenn productivMode == true werden Daten an Google Drive und eMails gesendet
const boolean debugMode         = true;   // debugMode == true, es werden Messages über den seriellen Monitor ausgegeben
static const int DHTPin         = 9;      // DigitalPin - DHT22 Sensor    
static const int ledPin         = 13;     // DigitalPin - LED
static const int lightPin       = A0;     // Analoger Pin an dem der Lichtsensor angeschlossen ist
unsigned const long interval    = 3600;   // Interval in Sekunden wie oft die Daten abgefragt und gesendet werden sollen



// Ab hier nichts mehr anpassen
unsigned long previousMillis  =  0;       // Delay Ersatz
Process date;                             // Datum 
int day, month, year;                     // Tag Monat und Jahr trennen
int h, t, l;                              // Humidity, Temperature und Helligkeit
#define DHTTYPE DHT22                     // DHT11, DHT21, DHT22
DHT dht(DHTPin, DHTTYPE);





void setup()
{
  Bridge.begin();
  dht.begin();
  pinMode(ledPin, OUTPUT);
  
  if (debugMode == true)
  {
    Serial.begin(115200);
    while (!Serial);
    
    Serial.println(F("DebugMode..."));
    if(productiveMode) Serial.println(F("Produktivemode...")); else Serial.println(F("Testmode..."));
    
    if(productiveMode == true)
    {
      Serial.print(F("Daten werden alle "));
      Serial.print(interval);
      Serial.println(F(" Sekunden an Google Drive Tabelle gesendet..."));
    }
    
    Serial.println(F("Setup komplett...\n"));
  }
}





void loop()
{ 
  unsigned long currentMillis = millis();
   
  getSensorValues(); //Ständig Sensorwerte ermitteln
  
  if(currentMillis - previousMillis >= (interval*1000)) 
  {
    digitalWrite(ledPin, HIGH);
        
    previousMillis = currentMillis;
    
    if(productiveMode == true) 
    {
      if(debugMode == true) { Serial.println(F("Aufruf von /Library/Google/Spreadsheets/AppendRow Choreo...")); }
      runAppendRow(l, t, h); 
    } else { if(debugMode == true) Serial.println(F("Testmodus - Es werden keine Daten gesendet")); }
  }
  digitalWrite(ledPin, LOW);
}





/*****************************************************
                                                     *
  Daten in Google Tabelle auf Google Docs schreiben  *
                                                     *
*****************************************************/
void runAppendRow(int lightLevel, int temperature, int humidity)
{
  TembooChoreo AppendRowChoreo;

  AppendRowChoreo.begin();
  AppendRowChoreo.setAccountName(TEMBOO_ACCOUNT);
  AppendRowChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  AppendRowChoreo.setAppKey(TEMBOO_APP_KEY);
  AppendRowChoreo.setChoreo("/Library/Google/Spreadsheets/AppendRow");
  AppendRowChoreo.addInput("Username", GOOGLE_USERNAME);
  AppendRowChoreo.addInput("Password", GOOGLE_PASSWORD);
  AppendRowChoreo.addInput("SpreadsheetTitle", SPREADSHEET_TITLE);

  String data = parseDateTime()+","+String(lightLevel)+","+String(temperature)+","+String(humidity);
  AppendRowChoreo.addInput("RowData", data);

  unsigned int returnCode = AppendRowChoreo.run();

  if (returnCode == 0) {
    if (debugMode == true) { Serial.println(F("Ausfuehrung von /Library/Google/Spreadsheets/AppendRow Choreo komplett.\n")); }
  } else {
    while (AppendRowChoreo.available()) {
      char c = AppendRowChoreo.read();
      if (debugMode == true) { Serial.print(c); }
    }
    if (debugMode == true) { Serial.println(); }
  }
  AppendRowChoreo.close();
  if (debugMode == true) { Serial.print(data); Serial.println(F(" an Google Docs gesendet"));  }
}





/*************************************************************************************************
                                                                                                 *
  Datum und Zeit von (mm/dd/yy hh:ii:ss) zerlegen in Tag, Monat, Jahr, Stunde, Minute, Sekunde   *
  Empfang  = 06/26/14 16:15:42                                                                   *
  Rückgabe = 26.06.14, 16:15:42                                                                  *
                                                                                                 *
*************************************************************************************************/
String parseDateTime()
{
  String result;
  
  if (!date.running()) 
  {
    date.begin("date");
    date.addParameter("+%D-%T-");
    date.run();
  }
  
  if (date.available() > 0) 
  {
    String timeString = date.readString();
    
     //TimeStamp zerlegen in Datum und Zeit
    int dateColumn = timeString.indexOf("-");
    int timeColumn = timeString.lastIndexOf("-");
    
    String dateStr = timeString.substring(0, dateColumn);
    String timeStr = timeString.substring(dateColumn+1, timeColumn);
 
    // Datum zerlegen und daraus deutsche Schreibweise machen
    int firstColon = timeString.indexOf("/");
    int secondColon = timeString.lastIndexOf("/");
    
    String monthString = timeString.substring(0, firstColon);
    String dayString   = timeString.substring(firstColon + 1, secondColon);
    String yearString  = timeString.substring(secondColon + 1);
    
    day   = dayString.toInt();
    month = monthString.toInt();
    year  = yearString.toInt();
    
    if (day <= 9) result += "0";
    result += String(day);
    result += ".";
    if (month <= 9) result += "0";
    result += String(month);
    result += ".";
    result += String(year);
    result += ",";
    result += timeStr;
  }
  return result;
}




/*******************************************************
                                                       *
  Sensorwerte des DHT22 und des LDR Sensors ermitteln  *
                                                       *
*******************************************************/
void getSensorValues()
{
  h = dht.readHumidity();    // Luftfeuchte auslesen
  t = dht.readTemperature(); // Temperatur auslesen
  l = analogRead(lightPin);  // Lichtstärke auslesen
  
  
  if(debugMode == true)
  {
    if (isnan(t) || isnan(h)) 
    {
      Serial.println(F("DHT22 konnte nicht ausgelesen werden"));
    } 
    else
    {          
      Serial.print(F("Luftfeuchte: ")); 
      Serial.print(h);
      Serial.print(F("%\t"));
      Serial.print(F("Temperatur: ")); 
      Serial.print(t);
      Serial.println(F("C"));
    }
  }
}
