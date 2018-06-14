/*-----( Import needed libraries )-----*/
#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>

/*-----( Declare Constants )-----*/
/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 16 chars 2 line display
// A FEW use address 0x3F
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

const int potPin = 0;    // select the input pin for the potentiometer
const int leftLight = 2;   // select the pin for the LED
const int rightLight = 4;
const int buttonPin = 7;


// All weapon timing constants are written in microseconds (not milliseconds). This it to ensure a greater level of accuracy
// as well as to accound for the fact that sabre has a bound of 0.1ms (or 100us)

const int eSensLower = 2;
const int eSensUpper = 10;
const int eLockLower = 40;
const int eLockUpper = 50;

const int fSensLower = 13;
const int fSensUpper = 15;
const int fLockLower = 275;
const int fLockUpper = 325;

const float sSensLower = 0.1;                                           // note that this is  0.1 milliseconds 
const int sSensUpper = 1; 
const int sLockLower = 160;
const int sLockUpper = 180;

// these values are arbitrary, to prvoide enough room on either end of sensitivity and lockout times
// they are given in milliseconds here, as they are used for selection, and will be converted to microseconds after
const int eBoundLower = 0;
const int eBoundUpper = 15;
const int fBoundLower = 10;
const int fBoundUpper = 18;
const int sBoundLower = 0;
const int sBoundUpper = 3;

const int eDelayLower = 25;
const int eDelayUpper = 65;
const int fDelayLower = 250;
const int fDelayUpper = 350;
const int sDelayLower = 150;
const int sDelayUpper = 190;


/*-----( Declare Variables )-----*/
int buttonState = 0;                                                  // variable for reading the pushbutton status
int val = 0;                                                          // variable to store the value coming from the sensor
char weapon = '0';
unsigned long previousMillis = 0;                                     // used for the blinking light state
int ledState = LOW; 
int workingSensLeft = 0;
int workingSensRight = 0;
int workingDelay = 0;
int leftLightState = 3;                                               // this is to track the LED state, 0 will be LOW, 1 will be blink, 2 will be HIGH
int rightLightState = 3;                                              // initialized to 3 so that any value will take over later (less than)
int leftLightTemp = 3;
int rightLightTemp = 3;
int workingLightState = 0;

// working bounds, they will get assigned later based off of weapon
int wSensLower = 0;
int wSensUpper = 0;
int wLockLower = 0;
int wLockUpper = 0;
int wBoundLower = 0;
int wBoundUpper = 0;
int wDelayLower = 0;
int wDelayUpper = 0;

int leftSens;
int rightSens;
int wDelay;
int absDelay;





void setup() {
  Serial.begin(9600);
  delay(100);
  pinMode(leftLight, OUTPUT);
  pinMode(rightLight, OUTPUT);
  pinMode(buttonPin, INPUT);

  lcd.begin(16,2);                                                    // initialize the lcd for 16 chars 2 lines, turn on backlight
  
  lcd.setCursor(0,0);                                                 // cursor Position: (CHAR, LINE) start at 0  
  lcd.print("Timing Tester");
  lcd.setCursor(0,1);
  lcd.print("June 8, 2018");
  delay(3000);
  lcd.clear();


  
  // loops the weapon selection until the button is pressed
  while(buttonState == LOW){
    buttonState = digitalRead(buttonPin);
    weaponSelect();
  }
  delay(50);

  weaponTimings();
  delay(500);

  leftSens = sens(0, leftLight);                                      // wile loop is inside function
  lcd.setCursor(0,1);
  lcd.print(String(leftSens));
  delay(500);
  
  rightSens = sens(0, rightLight);                                    // while loop is inside function
  lcd.setCursor(13,1);
  lcd.print(String(rightSens));
  delay(500);
  
  wDelay = sens(1, 0);                                                // while loop is inside function
  lcd.setCursor(6,1);
  lcd.print(String(wDelay));
  absDelay = abs(wDelay);                                             // saves absolute value of delay for the loop function
  delay(500);

  leftLightTemp = 3;                                                  // resets temp lights so they don't interfere with the loop
  rightLightTemp = 3;
  ledDo;                                                              // Sets display one last time to ensure correct
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH){
    if (wDelay > 0){
      digitalWrite(leftLight, HIGH);
      delay(leftSens);                                                //delayMicroseconds((leftSens*1000)); Micros does not work like that, must do * seperate
      digitalWrite(leftLight, LOW);
      delay(absDelay);
      digitalWrite(rightLight, HIGH);
      delay(rightSens);
      digitalWrite(rightLight, LOW);

      delay(3000);
    }
    else if (wDelay < 0){
      digitalWrite(rightLight, HIGH);
      delay(rightSens);
      digitalWrite(rightLight, LOW);
      delay(absDelay);
      digitalWrite(leftLight, HIGH);
      delay(leftSens);
      digitalWrite(leftLight, LOW);

      delay(3000);
    }
  }
}



/*-----------------------------------------------------------*/




// selects which weapon by turning dial
void weaponSelect(){
  val = analogRead(potPin);                                           // read the value from the sensor
  lcd.setCursor(0,0);

  if(val <= 341){ 
    lcd.print("Foil ");
    weapon = 'F';
  }
  if(val > 341 && val <= 642){
    lcd.print("Epee ");
    weapon = 'E';
  }
  if(val > 642 && val <= 1024){
    lcd.print("Sabre");
    weapon = 'S';
  }
}




// sets working timings and limits from the selected weapon
void weaponTimings(){

  // Foil
  // sensitivity is 13-15ms
  // lockout is 275-325ms
  // delay range should be 250-350ms
  if (weapon == 'F'){
    wSensLower = fSensLower;
    wSensUpper = fSensUpper;
    wLockLower = fLockLower;
    wLockUpper = fLockUpper;
    wBoundLower = fBoundLower;
    wBoundUpper = fBoundUpper;
    wDelayLower = fDelayLower;
    wDelayUpper = fDelayUpper;
  }

  // Epee
  // sensitivity is 2-10ms
  // lockout is 40-50ms
  // delay range should be 25-65ms 
  if (weapon == 'E'){
    wSensLower = eSensLower;
    wSensUpper = eSensUpper;
    wLockLower = eLockLower;
    wLockUpper = eLockUpper;
    wBoundLower = eBoundLower;
    wBoundUpper = eBoundUpper;
    wDelayLower = eDelayLower;
    wDelayUpper = eDelayUpper;
  }

  // Sabre
  // sensitivity is .01-1ms
  // lockout is 160-180ms
  // delay range should be 150-190ms
  if (weapon == 'S'){
    wSensLower = sSensLower;
    wSensUpper = sSensUpper;
    wLockLower = sLockLower;
    wLockUpper = sLockUpper;
    wBoundLower = sBoundLower;
    wBoundUpper = sBoundUpper;
    wDelayLower = sDelayLower;
    wDelayUpper = sDelayUpper;
  }
}




// 
int sens(boolean mirror, int side){
  while (true){
    buttonState = digitalRead(buttonPin);
    int rangeVal = dialRange(mirror);                               // returns value according to dial position limited by bounds
    lcd.setCursor(12,0);
    lcd.print(String(rangeVal)+"   ");
    sensCheck(rangeVal, side);                                      // checks what the LEDs should display
    ledDo();                                                        // funtion that actually runs the LEDs

    if (buttonState == HIGH){                                       // once value is confirmed by button press
      lcd.setCursor(12,0);                                          // clear the top right of the screen
      lcd.print("       ");

    if (leftLightTemp < leftLightState)
      leftLightState == leftLightTemp;
    if (rightLightTemp < rightLightState)
      rightLightState = rightLightTemp;
      
      delay(10);
      return rangeVal;                                              // returns the dial value to be saved
    }
  }
}




// sets the range for the dial
int dialRange(boolean mirror){                                       // boolean is so that we know if we want a mirrored negative range as well
  val = analogRead(potPin);
  if (mirror == 0){
    int range = (wBoundLower+(val*((wBoundUpper-wBoundLower)/1024.0))+0.5);           // keep the 1024.0 with the decimal, that makes everything a float, until it is assigned at end to int
    return range;
  }
  else if (mirror == 1){
    int i = (val-512);                                              // 512 is the midpoint for the dial
    i = abs(i);                                                     // seperate the absolute function to prevent errors
    i = i/(val-512);                                                // this gives us positive or negative 1, so we know when to flip for the delay
    int range = ((val-512)*((wDelayUpper-wDelayLower)/512.0))+(i*(wDelayLower+0.5));
    return range;
  }
}



//
void sensCheck(int val, int sideLight){
  if (sideLight != 0){                                              // if we are checking a side light, and therefore not the delay timing between both sides
    if (val < wSensLower){
      workingLightState = 0;
    }
    else if (val >= wSensLower && val <= wSensUpper){
      workingLightState = 1;
    }
    else if (val > wSensUpper){
      workingLightState = 2;
    }
  }
  else if (sideLight == 0){                                         // delay check is here
    if (val < 0){                                                   // sets the working side light to left or right depending on if the delay input is positive or negative
      sideLight = leftLight;
    }
    else if (val > 0){
      sideLight = rightLight;
    }

    int aVal = abs(val);                                            // sets aValue to absolute, for checks below
        
    if (aVal < wLockLower){                                         // if the delay is less than the lower bound, side should register
      workingLightState = 2;
    }
    if (aVal >= wLockLower && val <= wLockUpper){                   // if delay is between bounds, side MIGHT register based on tolerences
      workingLightState = 1;
    }
    if (aVal > wLockUpper){                                         // if the delay is greater than the upper bound, side should NOT register
      workingLightState = 0;
    }
  }

  if (sideLight == leftLight)                                       // assigns temp state from the working, to be used in ledDo and compared to any saved state
    leftLightTemp = workingLightState;
  else if (sideLight == rightLight)
    rightLightTemp = workingLightState;
}




// turns the LEDs on, off or blink
void ledDo(){

  int lLight;
  int rLight;

  lLight = leftLightState;
  rLight = rightLightState;

  if (leftLightTemp < leftLightState)                               // if the temporary state (from dial) is less than the saved, use the temporary (but don't save it until button confirm)
    lLight = leftLightTemp;    
  if (rightLightTemp < rightLightState)
    rLight = rightLightTemp;
    
  if (lLight == 0){
    lcd.setCursor(7,0);
    lcd.print("-");
  }
  if (lLight == 1){
    lcd.setCursor(7,0);
    lcd.print("r");
  }
  if (lLight == 2){
    lcd.setCursor(7,0);
    lcd.print("R");
  }

  if (rLight == 0){
    lcd.setCursor(8,0);
    lcd.print("-");
  }
  if (rLight == 1){
    lcd.setCursor(8,0);
    lcd.print("g");
  }
  if (rLight == 2){
    lcd.setCursor(8,0);
    lcd.print("G");
  }
}





// https://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
void lightBlink(int sideLight){ // input should be either leftLight or rightLight
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= (1000)) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(sideLight, ledState);
  }
}



