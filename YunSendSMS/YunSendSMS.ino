/*************************************************************************
  Projekt:   YunSendSMS                                                  *
  Developer: Enrico Sadlowski                                            *
                                                                         *
  Wenn ich nicht zu Hause bin, möchte ich trotzdem wissen wenn jemand    *
  bei mir an der Haustür klingelt.                                       *
  Der Arduino sendet mir eine SMS wenn jemand an der Tür klingelt.       *
  wenn jemand klingelt.                                                  *
*************************************************************************/

#include <Bridge.h>
#include <Temboo.h>
#include "TembooAccount.h"              // Temboo account information
#include "TwilioAccount.h"              // Twilio account information


// Diese Daten anpassen
const boolean productiveMode  = true;   // Nur wenn productivMode == true werden SMS verschickt
const boolean debugMode       = true;   // debugMode == true, es werden Messages über den seriellen Monitor ausgegeben
static const int doorbellPin  =  8;     // DigitalPin - PushButton
static const int ledPin       = 13;     // DigitalPin - LED
static const int maxCalls     = 10;     // Anzahl SMS-Kontingent
unsigned const long interval  = 10;     // Interval in Sekunden um zu häufiges Senden von SMS zu verhindern (Klingelsturm)


// Ab hier nichts mehr anpassen
unsigned long previousMillis  =  0;     // Delay Ersatz
int calls                     =  0;     // Anzahl versendeter SMS
int state;                              // zum Entprellen des Buttons
int lastState;                          // zum Entprellen des Buttons







void setup() 
{
  if(debugMode)
  {
    Serial.begin(9600);
    while(!Serial);
  }
  
  Bridge.begin();
  
  pinMode(doorbellPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  if(debugMode) Serial.println("Setup komplett.\n");
}







void loop()
{
  unsigned long currentMillis = millis();
 
  state = digitalRead(doorbellPin);
  
  if (state != lastState) 
  {
    if (state == HIGH) 
    {  
      if(currentMillis - previousMillis >= (interval*1000)) 
      {
        previousMillis = currentMillis;  
       
        // SMS nur im Produktivmodus senden
        if(productiveMode)
        {
          if (calls < maxCalls) 
          {
            runSendSMS();
            if(debugMode)
            {
              Serial.println(F("SMS wird gesendet..."));
              Serial.print(F("Es wurden bereits "));
              Serial.print(calls);
              Serial.println(F(" SMS versendet...")); 
            }
          }
          else { if(debugMode) Serial.println(F("Anzahl der maximalen SMS wurde erreicht"));  }         
        } else { if(debugMode) Serial.println(F("TESTMODUS - Es wurd keine SMS gesendet...")); }
      }
    }
    lastState = state;
  }
  digitalWrite(ledPin, state);
} 








/***************************
  SMS über Twilio senden   *
***************************/
void runSendSMS() 
{
  TembooChoreo SendSMSChoreo;

  SendSMSChoreo.begin();

  SendSMSChoreo.setAccountName(TEMBOO_ACCOUNT);
  SendSMSChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  SendSMSChoreo.setAppKey(TEMBOO_APP_KEY);

  SendSMSChoreo.setChoreo("/Library/Twilio/SMSMessages/SendSMS");

 
  SendSMSChoreo.addInput("AccountSID", TWILIO_ACCOUNT_SID);
  SendSMSChoreo.addInput("AuthToken", TWILIO_AUTH_TOKEN);
  SendSMSChoreo.addInput("From", TWILIO_NUMBER);
  SendSMSChoreo.addInput("To", RECIPIENT_NUMBER);
  SendSMSChoreo.addInput("Body", "Es hat jemand bei Nietsch geklingelt");
  
  unsigned int returnCode = SendSMSChoreo.run();

  if (returnCode == 0) {
    if(debugMode) Serial.println(F("SMS wurde erfolgreich gesendet!\n"));
    calls++;
  } else {
    while (SendSMSChoreo.available()) {
      char c = SendSMSChoreo.read();
      if(debugMode) Serial.print(c);
    }
    if(debugMode) Serial.println();
  }
  SendSMSChoreo.close();
}
