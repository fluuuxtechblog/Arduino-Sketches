/****************************************************************************************************************************
  Bei einem Druck auf den Button an Digital Pin8 werden die Werte eines LM335 und eines Lichtsensors                        *
  in einer Google Tabelle (Yun) in Google Drive gespeichert. Außerdem werden jedesmal das aktuelle                          *
  Datum und die aktuelle Zeit vom embedded Linux ausgelesen und auch in die Google Tabelle gespeichert.                     *
                                                                                                                            *
  Unter- oder überschreitet die Temperatur den in "low_temperature_limit" gespeicherten Wert, wird eine eMail geschickt.    *
                                                                                                                            *
  Für dieses Beispiel benötigt man einen Google Account und einen Account bei temboo.com                                    *
****************************************************************************************************************************/

// Libraries
#include <Bridge.h>
#include <Temboo.h>
#include <Process.h>

#include "TembooAccount.h"           // Tab TembooAccount.h


// Pins (anpassen)
const int buttonPin        = 8;     // the number of the pushbutton pin
const int sensorLightPin   = A0;    // Analoger Pin an dem der Lichtsensor angeschlossen ist
const int sensorTempPin    = A1;    // Analoger Pin an dem der LM335 Temperatursensor angeschlossen ist

//Vars (anpassen)
boolean debug_mode         = true;  // Statusmeldungen ausgeben
int low_temperature_limit  = 10;    // unteres Temperatur Limit;
int high_temperature_limit = 35;    // oberes Temperatur Limit;


//Vars (so lassen)
int buttonState = 0;
int lightLevel;                     // Wert des Helligkeitssensors
int temperature;                    // Wert des DHT22 - Temperatur


//Datum und Zeit
Process date;
int day, month, year, hours, minutes, seconds;










void setup()
{
  pinMode(buttonPin, INPUT);
  Bridge.begin();
  
  if (debug_mode == true)
  {
    Serial.begin(115200);
    delay(4000);
    while (!Serial);
  }

  if (debug_mode == true) { Serial.println(F("Setup komplett. Button druecken um Sensorwerte zu senden...\n")); }
}







void loop()
{
  
  buttonState      = digitalRead(buttonPin);  
  lightLevel       = analogRead(sensorLightPin);
  int tempLevel    = analogRead(sensorTempPin);
  float millivolts = (tempLevel / 1024.0) * 5000;
  temperature      = (millivolts / 10) - 273.15;



  
  if (buttonState == HIGH) 
  {
    if (debug_mode == true) { Serial.println(F("Aufruf von /Library/Google/Spreadsheets/AppendRow Choreo...")); }

    runAppendRow(lightLevel, temperature); // Sensorwerte an Google Docs sheet senden
  
    // Alarmmeldung an eMail senden wenn Temperatur unter dem unteren Temperaturlimit liegt
    if (temperature < low_temperature_limit || temperature > high_temperature_limit)
    {
      if (debug_mode == true) { Serial.println(F("Sende Alarmmeldung per eMail ")); }
      if (temperature < low_temperature_limit) sendTempAlert("Temperatur ist zu niedrig!");
      else if (temperature > high_temperature_limit) sendTempAlert("Temperatur ist zu hoch!");      
    }  
  }
}









/************************************************************************
                                                                        *
  Alarm beim über- oder unterschreiten der Temperatur per eMail senden  *
                                                                        *
************************************************************************/
void sendTempAlert(String message)
{
  if(debug_mode == true) { Serial.println(F("eMail wird gesendet...")); }
  TembooChoreo SendEmailChoreo;
  SendEmailChoreo.begin();
  SendEmailChoreo.setAccountName(TEMBOO_ACCOUNT);
  SendEmailChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  SendEmailChoreo.setAppKey(TEMBOO_APP_KEY);
  SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");
  SendEmailChoreo.addInput("Username", GOOGLE_USERNAME);
  SendEmailChoreo.addInput("Password", GOOGLE_PASSWORD);
  SendEmailChoreo.addInput("ToAddress", TO_EMAIL_ADDRESS);
  SendEmailChoreo.addInput("Subject", "ALARM: Home Temperature");
  SendEmailChoreo.addInput("MessageBody", message);

  unsigned int returnCode = SendEmailChoreo.run();

  if (returnCode == 0) {
    if (debug_mode == true) { Serial.println(F("eMail wurde erfolgreich gesendet!")); }
  } else {
    while (SendEmailChoreo.available()) {
      char c = SendEmailChoreo.read();
      if (debug_mode == true) { Serial.print(c); }
    }
  }
  SendEmailChoreo.close();
}










/*****************************************************
                                                     *
  Daten in Google Tabelle auf Google Docs schreiben  *
                                                     *
*****************************************************/
void runAppendRow(int lightLevel, float temperature)
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

  String data = parseDateTime()+","+String(lightLevel)+","+String(temperature);
  AppendRowChoreo.addInput("RowData", data);

  unsigned int returnCode = AppendRowChoreo.run();

  if (returnCode == 0) {
    if (debug_mode == true) { Serial.println(F("Ausfuehrung von /Library/Google/Spreadsheets/AppendRow Choreo komplett.\n")); }
  } else {
    while (AppendRowChoreo.available()) {
      char c = AppendRowChoreo.read();
      if (debug_mode == true) { Serial.print(c); }
    }
    if (debug_mode == true) { Serial.println(); }
  }
  AppendRowChoreo.close();
  
  if (debug_mode == true) { Serial.print(data); Serial.println(F(" an Google Docs gesendet"));  }
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
