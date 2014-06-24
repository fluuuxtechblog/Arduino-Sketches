/*
  Projekt:  Lampensteuerung (Mondlicht)
  Autor:    Enrico Sadlowski
  Datum:    12.10.2012
  Beschreibung:
  -------------
  Die RGB-LED verändert kontinuierlich ihre Farbe. 
  Über den Button kann man die gerade sichtbare Farbe anhalten.
  bei erneutem Druck auf den Button, wird der Farbwechsel fortgeführt.
*/

// Output
const int redPin = 2; //Digital PWM
const int grnPin = 3; //Digital PWM
const int bluPin = 4; //Digital PWM

//Input
const int buttonPin = 7; //Digital 

// Color arrays
int black[3]  = { 0, 0, 0 };
int white[3]  = { 100, 100, 100 };
int red[3]    = { 100, 0, 0 };
int green[3]  = { 0, 100, 0 };
int blue[3]   = { 0, 0, 100 };
int yellow[3] = { 40, 95, 0 };
int dimWhite[3] = { 30, 30, 30 };

// Set initial color
int redVal = black[0];
int grnVal = black[1]; 
int bluVal = black[2];

int wait = 10;      // 10ms internal crossFade delay; increase for slower fades
int hold = 10;       // Optional hold when a color is complete, before the next crossFade
int j = 0;          // Loop counter for repeat

int prevR = redVal;
int prevG = grnVal;
int prevB = bluVal;

int buttonPushCounter = 0;
int buttonState = 0;


void setup()
{
  pinMode(redPin, OUTPUT);
  pinMode(grnPin, OUTPUT);   
  pinMode(bluPin, OUTPUT); 
  
  pinMode(buttonPin, INPUT);
}





void loop()
{
  crossFade(red);
  crossFade(green);
  crossFade(blue);
  crossFade(yellow);
}









void crossFade(int color[3]) 
{
  int R = (color[0] * 255) / 100;
  int G = (color[1] * 255) / 100;
  int B = (color[2] * 255) / 100;

  int stepR = calculateStep(prevR, R);
  int stepG = calculateStep(prevG, G); 
  int stepB = calculateStep(prevB, B);

  for (int i = 0; i <= 1020; i++) 
  {    
    //Button drücken, aktuelle Farbe halten. Weiteres mal drücken, Farbwechsel fortführen  
    buttonState = digitalRead(buttonPin);
  
    if (buttonState == HIGH) 
    {
      buttonPushCounter++;
      delay(500);
    }
      
    if(buttonPushCounter > 1) buttonPushCounter = 0; 
    
    if(buttonPushCounter == 1) while (digitalRead(buttonPin)==LOW);
    
    redVal = calculateVal(stepR, redVal, i);
    grnVal = calculateVal(stepG, grnVal, i);
    bluVal = calculateVal(stepB, bluVal, i);

    analogWrite(redPin, redVal);
    analogWrite(grnPin, grnVal);      
    analogWrite(bluPin, bluVal); 

    delay(wait);
  }
  prevR = redVal; 
  prevG = grnVal; 
  prevB = bluVal;
  
  delay(hold);
}













int calculateStep(int prevValue, int endValue) 
{
  int step = endValue - prevValue;
  if (step) 
  {
    step = 1020 / step;
  } 
  return step;
}






int calculateVal(int step, int val, int i) 
{
  if ((step) && i % step == 0) 
  {
    if (step > 0) 
    {
      val += 1;           
    } 
    else if (step < 0) 
    {
      val -= 1;
    } 
  }
  if (val > 255) 
  {
    val = 255;
  } 
  else if (val < 0) 
  {
    val = 0;
  }
  return val;
}  
