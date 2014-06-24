/******************************************************************************
* Projekt: Daten über Seriellen Port an Unity senden und von Unity empfangen  *
* Autor: Enrico Sadlowski                                                     *
* Erstellungsdatum:      27.03.2013                                           *
* Letzte Aktualisierung: 28.03.2013                                           *
*                                                                             *
* Beschreibung:                                                               *
*-----------------------------------------------------------------------------*
* Zeichen an Unity senden und von unity empfangen und entsprechend reagieren. *
* Daten die an Unity gesendet werden, müssen durch Unity ausgewertet werden   *
* Daten die von Unity empfangen werden, müssen in Arduino ausgewertet werden. *                                                                    *
******************************************************************************/

int inByte = 0;



void setup()
{
  Serial.begin(9600);
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT); 
}
 
 
 
 
void loop() 
{
  // Zeichen an Unity senden
  if(digitalRead(8) == HIGH) sendCharToUnity(2); // Zeichen 2 an Unity senden
  if(digitalRead(9) == HIGH) sendCharToUnity(1); // Zeichen 1 an Unity senden

  
  
  //Zeichen von Unity empfangen
  if (Serial.available() > 0) 
  {
    inByte = Serial.read();
    
    switch(inByte)
    {
      case 54:                 // Zeichen 6 von Unity empfangen
        sendCharToUnity(3);    // Zeichen 3 an Unity senden und 10ms Pause
        blinkLEDandOff(6, 2);  // led an Pin 6 zweimal aufblinken lassen
      break;
      case 55:                 // Zeichen 7 von Unity empfangen
        sendCharToUnity(4);    // Zeichen 4 an Unity senden und 10ms Pause
        blinkLEDandOff(7, 2);  // led an Pin 7 zweimal aufblinken lassen
      break;
    }
  }
}



/* 
  LED blinken lassen 
  led = pin an dem die LED hängt
  interval = Angabe wie oft die LED aufblinken soll
*/
void blinkLEDandOff(byte led, byte interval)
{
  for(int i=0;i<interval;i++)
  {
    digitalWrite(led, HIGH); 
    delay(100);
    digitalWrite(led, LOW); 
    delay(100);
  }
}



/* einzelnes Zeichen an Unity senden */
void sendCharToUnity(char Zeichen)
{
   Serial.write(Zeichen);
   Serial.flush();
   delay(10); 
}
