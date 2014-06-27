/***********************************************************************************
  Projekt: YunSendEmail                                                            *
  Autor: Enrico Sadlowski                                                          *
  Erstellungsdatum: 27.06.2014                                                     *
  Letzte Aktualisierung: 27.06.2014                                                *
                                                                                   *
  Wenn ein Sensor ausgelöst wird, können der Sensorwert und ein                    *
  beliebiger Text an eine vorgegebene eMail-Adresse gesendet werden.               *
  Eine weitere eMail wird bei erneuter Sensorauslösung erst gesendet wenn          *
  alarmDeactivated vorher auf true gesetzt war. (Alarm zurückgesetzt)              *
                                                                                   *
  Vorraussetzungen                                                                 *
  Google Account, temboo.com Account, Arduino Yun                                  *
***********************************************************************************/

#include <Bridge.h>
#include <Temboo.h>

#include "TembooAccount.h"                // Temboo Account Zugangsdaten
#include "GoogleAccount.h"                // Google Account Zugangsdaten


// Diese Daten anpassen
const boolean productiveMode    = true;   // Nur wenn productivMode == true werden Daten an Google Drive und eMails gesendet
const boolean debugMode         = true;   // debugMode == true, es werden Messages über den seriellen Monitor ausgegeben
static const int doorbellPin    =  8;     // DigitalPin - PushButton
static const int ledPin         = 13;     // DigitalPin - LED



// Ab hier nichts mehr anpassen
int state;                                // zum Entprellen des Buttons
int lastState;                            // zum Entprellen des Buttons
boolean alarmDeactivated        = true;   // Wenn eMail verschickt wurde, wird erst eine weitere verschickt wenn alarmDeactivated true ist







void setup() 
{  
  Bridge.begin();
  
  pinMode(doorbellPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  
  if(debugMode == true)
  {
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Setup komplett.\n");
  }
}






void loop()
{
  state = digitalRead(doorbellPin);
  
  if (state != lastState) 
  {
    if (state == HIGH) 
    {
      if(debugMode == true) Serial.println(F("Sensor hat ausgeloest..."));
      
      if(productiveMode)
      {
        if(alarmDeactivated == true)
        {
          sendTempAlert(F("Es wurde eine Bewegung im Arbeitszimmer erkannt\nBewegungsmelder hinten links hat ausgeloest!\n\nhttp://fluuux.de-Alarmkamera"));
          alarmDeactivated = false;
        } else { if(debugMode == true) Serial.println(F("Es wurde bereits eine eMail verschickt. Erst Anlage reseten...")); }
      } else { if(debugMode == true) Serial.println(F("TESTMODUS - Es wurd keine eMail gesendet...")); }
    }
    lastState = state;
  }
  digitalWrite(ledPin, state);
} 




        
  
  





/************************************************************************
                                                                        *
  Alarm beim über- oder unterschreiten der Temperatur per eMail senden  *
                                                                        *
************************************************************************/
void sendTempAlert(String message)
{
  if(debugMode == true) { Serial.println(F("eMail wird gesendet...")); }
  TembooChoreo SendEmailChoreo;
  SendEmailChoreo.begin();
  SendEmailChoreo.setAccountName(TEMBOO_ACCOUNT);
  SendEmailChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  SendEmailChoreo.setAppKey(TEMBOO_APP_KEY);
  SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");
  SendEmailChoreo.addInput("Username", GOOGLE_USERNAME);
  SendEmailChoreo.addInput("Password", GOOGLE_PASSWORD);
  SendEmailChoreo.addInput("ToAddress", TO_EMAIL_ADDRESS);
  SendEmailChoreo.addInput("Subject", "ALARM: Bewegungsmelder hat ausgeloest");
  SendEmailChoreo.addInput("MessageBody", message);

  unsigned int returnCode = SendEmailChoreo.run();

  if (returnCode == 0) {
    if (debugMode == true) { Serial.println(F("eMail wurde erfolgreich gesendet!")); }
  } else {
    while (SendEmailChoreo.available()) {
      char c = SendEmailChoreo.read();
      if (debugMode == true) { Serial.print(c); }
    }
  }
  SendEmailChoreo.close();
}
