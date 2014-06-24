/*
  GetGMail
  Dieses Sketch wartet auf ankommende Signale überd en seriellen Port
  Wird ein M empfangen dann fängt eine RGB LED an im Takt zu blinken und
  dabei ihre Farbe zu ändern. Wenn ein N empfangen wird, dann wird die LED 
  abgeschaltet.
  
  Zu diesem Script gehört das Python-Script check-gmail.py das minütlich durch einen
  CronJob aufgerufen wird. Das Python-Script  verbindet sich zum Google-Mail-Server und
  fragt ab ob ungelesene eMails im Posteingang sind. Je nach Wert wird ein M oder ein N 
  an der seriellen Port gesendet. 
*/

int bluePin  = 11;                     // Digital Pin für Blaue LED in RGB-LED  
int greenPin = 10;                     // Digital Pin für Grüne LED in RGB-LED  
int redPin   = 9;                      // Digital Pin für Rote LED in RGB-LED  

int val      = 0;                      // Wert aus dem seriellen Anschluss zu lesen
int DELAY    = 200;                    // Intervall zwischen Wechsel der Farben


void setup ()
{
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(redPin, OUTPUT);
  
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
  digitalWrite(redPin, LOW);
  
  Serial.begin (9600);
  Serial.flush ();
}



void loop ()
{
  getNewMails(DELAY); 
}




void getNewMails(int Pause)
{
  while(int i=-1)
  {
    if (Serial.available()) 
    val = Serial.read()-48;
  
    if (val > 0) 
    {
      Serial.println("eMails: "+String(val));
      digitalWrite(redPin,HIGH);
      digitalWrite(greenPin,LOW);
      digitalWrite(bluePin,LOW);
      delay(Pause);
      digitalWrite(redPin,LOW);
      digitalWrite(greenPin,HIGH);
      digitalWrite(bluePin,LOW);
      delay(Pause);
      digitalWrite(redPin,LOW);
      digitalWrite(greenPin,LOW);
      digitalWrite(bluePin,HIGH);
      delay(Pause); 
    }
    else
    {
      digitalWrite(redPin,LOW);
      digitalWrite(greenPin,LOW);
      digitalWrite(bluePin,LOW); 
    }
  }
}
