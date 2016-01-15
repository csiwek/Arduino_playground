#include <Stepper.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

const int stepsPerRevolution = 48;  // change this to fit the number of steps per revolution
// for your motor


// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

int stepCount = 0;  // number of steps the motor has taken
int sensorState = 0;
bool calibrated = false;
String menuName ;
char menuValue[10];

int buttonState = 1 ;             // the current reading from the input pin
int lastButtonState = 1;   // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 30;    // the debounce time; increase if the output flickers

int buttonValState = 1 ;             // the current reading from the input pin
int lastButtonValState = 1;   // the previous reading from the input pin
long lastDebounceValTime = 0;  // the last time the output pin was toggled
int menuCounter =0;

int menuPos = 1;

int startUV = 0;
bool passDirection = false;

int pcbLen = 50;  //10cm
int uvCycles = 2;

void setup() {
  // nothing to do inside the setup
  pinMode(2, INPUT); // photo cell
  digitalWrite(2, HIGH);  
  pinMode(3, INPUT_PULLUP); // OPTION button
  pinMode(4, INPUT_PULLUP); // SELECT botton
  pinMode(8, OUTPUT);   //stepper
  pinMode(9, OUTPUT);   //stepper
  pinMode(10, OUTPUT);  //stepper
  pinMode(11, OUTPUT);  //stepper
  pinMode(12, OUTPUT);   //UV ON
  digitalWrite(12, LOW);
  Serial.begin(9600);
  Serial.print("Hello from UV BOX by csiwek");

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,10);
  display.setTextSize(1);
  display.setFont(&FreeMono9pt7b);

  display.println("UB BOX");
  display.println("by csiwek");
  display.println("Enjoy!");
  display.display();
  delay(1500);
  
  
}

void loop() {
start:
  // read the sensor value:
  display.clearDisplay();
  display.setTextSize(1);
  display.setFont(&FreeMono9pt7b);
  int motorSpeed = 400;
  myStepper.setSpeed(motorSpeed);

  if(calibrated==false){
  display.clearDisplay();
  display.setCursor(0,40);
  display.println("Calibrating");
  display.display();
  }
  myStepper.setSpeed(motorSpeed);
  if(digitalRead(2) == 1){
      calibrated = true;
      goto menu;
  }
  sensorState=0;
  while(calibrated==false){
      
    Serial.print("sensor state: ");
    Serial.println(sensorState);
    
    if(sensorState==1){
      calibrated = true;
      display.println("Done");
      display.display();
      delay(1000);
      stepCount=0;
    } else{
        myStepper.step(-stepsPerRevolution/2);
      
    }
    sensorState = digitalRead(2); 
 
  }
 
menu:
    display.clearDisplay();
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    digitalWrite(11, LOW);
//  MENU BUTTON
  int reading = digitalRead(3);
  if (reading != lastButtonState) {
     // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == 0) {
        menuPos++;  
      }
    }
  }
  if (menuPos>4){
    menuPos = 1;
  }
 
 // VALUE BUTTON
  int readingVal = digitalRead(4);
  
  if (readingVal != lastButtonValState) {
     // reset the debouncing timer
    lastDebounceValTime = millis();
  }

    if ((millis() - lastDebounceValTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (readingVal != buttonValState) {
      buttonValState = readingVal;

      // only toggle the LED if the new button state is HIGH
      if (buttonValState == 0) {
        Serial.println("Value Pressed");
        if(menuPos==1){
          pcbLen=+10;
        }else if(menuPos==2){
          uvCycles++;
        } else if(menuPos==3){
          startUV =1;
          goto uvExposure; 
        } else if(menuPos==4){
          goto learn; 
        }
      }
    }
  }

  if(menuPos==1){
    menuName = "PCB Length";
    sprintf(menuValue, "%d cm", (pcbLen/10));
  }else if(menuPos==2){
    menuName = "Num cycles";
    sprintf(menuValue, "%d", uvCycles);
  }else if(menuPos==3){
    menuName = "Start UV?";
    sprintf(menuValue, "%s", "Press OK");
  }else if(menuPos==4){
    menuName = "Learn Len.";
    sprintf(menuValue, "%s", "START");
  }
 
  lastButtonState = reading;
  lastButtonValState = readingVal;
  

  if ((menuCounter % 100 ) == 0) {
      display.clearDisplay();  
      display.setCursor(0,10);
      display.print("UV BOX MENU");
      display.setCursor(0,32);
      display.println(menuName);
      display.setCursor(10,52);
      display.setFont();
      display.setTextSize(2);
      if(menuCounter > 1000){
        display.setTextColor(BLACK, WHITE);
        display.println(menuValue);
        display.setTextColor(WHITE);
      } else{
        display.setTextColor(WHITE);
        display.println(menuValue);   
      }
      
      display.setFont(&FreeMono9pt7b);
      display.setTextSize(1);
      display.display();
  }
  if (menuCounter == 2001){
        menuCounter =0;
      }
  
  menuCounter++;
goto menu;
  
learn:
 
   stepCount = 0;
   lastButtonValState = 1;
   buttonValState = 1;
   lastDebounceValTime = 0;
   Serial.println(" LEARN Entered");
   delay(500);
   display.clearDisplay();  
   display.setCursor(0,10);
   display.print("Learning");
   display.setCursor(0,24);
   display.print("Press Select");
   display.setCursor(0,36);
   display.print("to stop.");
   display.display();
   delay(1000);
   display.setFont();
   while ( stepCount < 290 ){
   
        myStepper.setSpeed(motorSpeed);
        
        myStepper.step(stepsPerRevolution/2);
        display.clearDisplay();
        display.setCursor(40,20);
        display.setTextSize(2);
        display.print(stepCount/10);
        display.println(" cm");
        display.display();
        
        if (digitalRead(4) ==0){
                pcbLen = stepCount;
                menuPos=1;
                display.clearDisplay();
                display.setCursor(40,20);
                display.setTextSize(2);
                display.print(stepCount/10);
                display.println(" cm");
                display.display();
                Serial.println(stepCount);
                display.display();
                delay(1500);
                break;
                
            
        }
        stepCount++;
   }
   calibrated=0;
   goto start;



   
  
uvExposure:
 int cycleCount = 0;
 
  // set the motor speed:
 if(startUV == 1){
    digitalWrite(12, HIGH);
    displayProgress(cycleCount);
    while ( cycleCount < uvCycles ){
   
        myStepper.setSpeed(motorSpeed);
      // step 1/100 of a revolution:
        if (passDirection == false){
        
            myStepper.step(stepsPerRevolution/2);
            stepCount++;
        } else {
            sensorState = digitalRead(2);
            if(sensorState==0){
                myStepper.step(-stepsPerRevolution/2);
                stepCount--;
            } else{
                passDirection=false;
                cycleCount++;
                stepCount=0;
                displayProgress(cycleCount);              
            }
        }
        if(stepCount == (pcbLen)){
            passDirection=true;
        }
        if (!(stepCount % 10)){
        }  
    }
    startUV=0;
    digitalWrite(12, LOW);
    goto menu;
    

}

//  Serial.print("Sensor :");
 // Serial.println(sensorState);
 // Serial.print("Calibrated :");
//  Serial.println(calibrated);
//  Serial.print("Step :");
//  Serial.println(stepCount);
  //display.clearDisplay();
 //display.setCursor(0,10);
 // display.print("Step: ");
 // display.println(stepCount);
  //stepCount++;
  display.display();
 
}
void displayProgress(int count){
            display.clearDisplay();
            display.setCursor(0,10); 
            display.println("UV Exposure");

            display.setCursor(0,32);
            display.print("Cycle: ");
            display.println(count+1);
            display.display();


}
