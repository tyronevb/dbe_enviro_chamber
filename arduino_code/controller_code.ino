//Library Includes
#include <OneWire.h>
#include <DallasTemperature.h>
#include <String.h>
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include <LiquidCrystal.h>

// Push Button Catch Values - based on analog value incoming, dependent on resistors
#define btn1Low 450
#define btn1High 490
#define btn2Low 800
#define btn2High 900
#define btn3Low 150
#define btn3High 220
#define btn4Low 620
#define btn4High 670
#define btn5Low 690
#define btn5High 790
#define btn6Low 930
#define btn6High 1000

//const int buttonPin = A0;
// define Pin Numbers
#define buttonPin A0
#define ONE_WIRE_BUS 8
#define LCD_SSPIN 9
#define SSR_1_Pin 2
#define SSR_2_Pin 3
#define Relay_Pin 4

int btnPressed;
int prev_btnPressed = 0;

// ButtonStates - used to implement debounce
int buttonState; // current read in button state            
int lastButtonState = LOW;  // previous button stat

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 10;    // the debounce time; increase if the output flickers

//initialize LCD Instance
LiquidCrystal lcd(LCD_SSPIN);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Setup a sensors instance to use the oneWire instance to communicate with temp sensors
DallasTemperature sensors(&oneWire);

// Set device addresses
DeviceAddress Sensor1 = {0x28, 0xC6, 0x55, 0xB6, 0x06, 0x00, 0x00, 0x3D};
DeviceAddress Sensor2 = {0x28, 0x95, 0xA8, 0xB5, 0x06, 0x00, 0x00, 0x88};
DeviceAddress Sensor3 = {0x28, 0x2F, 0xCF, 0xB6, 0x06, 0x00, 0x00, 0x88};

//test setpoints
//signed int temperatureValues[4] = {25, 30, 20, 20};
//signed  int timeValues[4] = {5, 5, 10, 5};

// arrays to store temps
signed int temperatureValues[8];
signed int timeValues[8];

// handle size of array
int temp_Size;
int time_Size;

//string handler for incoming serial data
String data_from_web = "";
String temp1;
String temp2;


// control variable
bool actionPending = false;
char action;

//reset function
// function to reset arduino via software
void(* resetFunc)(void)=0;

//function prototypes
void clearLCDLine(int );
void writeToLCD(String , int );
void initLCD(int , int );
void toggleRelay(int );
void switchMode (bool , bool );
void heaterMode(float , signed int, signed int);
void coolingMode(float, signed int, signed int);
void controlTemp(void);
void serialTemperature(void);
void printTemperature(DeviceAddress deviceAddress);
int read_Serial(void);
int string_handler(String string, signed int *array);
int parse(char *string, signed int *array, unsigned int sizeArray);
int findNumValues (char *string);
void showTemperatures(void);
void inforDisplay(signed int, signed int, signed int);

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  
  // initialize various modules
  initTempSensors(10);
  initRelaysAndButton();
  initLCD(2, 16);
}

void infoDisplay(float setPoint, float temperature, signed int time)
{
  String lineOne = "Set:" + (String)setPoint + " T:" + (String)time;
  String lineTwo = "Temp Now:" + (String)temperature;
  
  writeToLCD(lineOne, 0);
  writeToLCD(lineTwo, 1);

}

// initialization funciton for relays
void initRelaysAndButton()
{
  pinMode(buttonPin, INPUT);
  pinMode(SSR_1_Pin, OUTPUT);
  pinMode(SSR_2_Pin, OUTPUT);  
}

// Initialization function for sensors
void initTempSensors(int Resolution)
{
  sensors.begin(); // Start up the library

  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(Sensor1, Resolution);
  sensors.setResolution(Sensor2, Resolution);
  sensors.setResolution(Sensor3, Resolution);

}

// Function to get temperature from sensor and send to serial
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    //writeToLCD("0", 1);
    Serial.print(0);
  } else {
   //writeToLCD(String(tempC), 1);
   Serial.print(tempC);
  }
}

void showTemperatures()
{
  sensors.requestTemperatures();
  writeToLCD("Temp1: ", 0);
  printTemperature(Sensor1);
  CheckButton();
  delay(1000);
  writeToLCD("Temp2: ", 0);
  printTemperature(Sensor2);
  CheckButton();
  delay(1000);
  writeToLCD("Temp3: ", 0);
  printTemperature(Sensor3);
  CheckButton();
  delay(1000);
}

// Function to send all temperature readings over serial
void serialTemperature(void)
{
  sensors.requestTemperatures(); // poll sensors for temperatures

  Serial.print("T"); // Begin char for temperatures
  printTemperature(Sensor1);
  Serial.print(",");
  printTemperature(Sensor2);
  Serial.print(",");
  printTemperature(Sensor3);
  Serial.print("E"); // End char for temperatures
}

// Clear LCD Lines
void clearLCDLine(int lineNum)
{
  // line Num: 0 for first line, 1 for second line, etc.
  lcd.setCursor(0, lineNum);
  lcd.print("                ");
  lcd.setCursor(0, lineNum);
}

// Write to LCD
void writeToLCD(String someData, int lineNum) 
{
  clearLCDLine(lineNum);
  lcd.print(someData);
}

// initialize LCD
void initLCD(int rows, int chars) 
{
  lcd.begin(chars, rows);
  clearLCDLine(0);
  lcd.print("System Idle...");
}

void toggleRelay(int relayNumber)
{
  if (relayNumber == 1)
  {
    digitalWrite(Relay_Pin, LOW);
    switchMode(LOW, LOW);
  }
  else if (relayNumber == 2)
  {
    switchMode(HIGH, LOW);
  }
  else if (relayNumber == 3)
  {
    switchMode(LOW, HIGH);   
  }
  else if (relayNumber == 4)
  {
    digitalWrite(Relay_Pin, HIGH);
    switchMode(LOW, LOW);
  }
  else if (relayNumber == 5)
  {
    switchMode(HIGH, HIGH);
  }
  
}

void switchMode (bool cooling, bool heating)
{
  if (cooling && heating && HIGH)
  {
    writeToLCD("Can't both be high...", 1);
    delay(500);
    writeToLCD("",1);
    return;
  }
  else
  {
  digitalWrite(SSR_1_Pin, cooling);
  digitalWrite(SSR_2_Pin, heating);
  } 
}

int CheckButton(){
  
   int reading = analogRead(buttonPin);  // read value on analogue pin 
   int tempButtonState = LOW;             // the current reading from the input pin
   //Serial.println(reading);   
   
   // check which button was pushed - based on the analogue input received
   // set tempButtonState to the value corresponding to the button pushed
   // also set btnPressed HIGH to indicate a button was pressed
   
   if(reading >= btn1Low && reading <= btn1High){
     //Read OK button
     tempButtonState = 1;
     btnPressed = HIGH;
     writeToLCD("Power ON",0);
     toggleRelay(4);
     delay(100);
     resetFunc();
   }else if(reading >= btn2Low && reading <= btn2High){
     //Read NO button
     tempButtonState = 2;
     btnPressed = HIGH;
     writeToLCD("Cooling ON",0);
     toggleRelay(2);
   }else if(reading >= btn3Low && reading <= btn3High){
     //Read START button
     tempButtonState = 3;
     btnPressed = HIGH;
     writeToLCD("Heating ON",0);
     toggleRelay(3);
     
   }else if(reading>=btn4Low && reading <= btn4High){
     //Read Clear button
     tempButtonState = 4;
     btnPressed = HIGH;
     writeToLCD("System OFF",0);
     toggleRelay(4);
    
   }else if(reading >= btn5Low && reading <= btn5High){
     //Read Increment button
     tempButtonState = 5;
     btnPressed = HIGH;
     writeToLCD("Starting control",0);
     delay(1000);
     controlTemp();
     
   }else if(reading >= btn6Low && reading <= btn6High){
     //Read Decrement button
     tempButtonState = 6;
     btnPressed = HIGH;
     writeToLCD("N/A",0);
     
     do{
       showTemperatures(); 
     }while(CheckButton() == LOW);
     
   }else{
     //No button is pressed
     tempButtonState = LOW;
     btnPressed = LOW;
     prev_btnPressed = LOW;
         
   }
   
   //Debounce
   if (tempButtonState != lastButtonState) {
     // reset the debouncing timer
     lastDebounceTime = millis(); // capture time of last Debounce event
   } 
   
   // check if current state is actual current state
   
   if ((millis() - lastDebounceTime) > debounceDelay) {
     // debounce delay condition satisfied
     
     // set actual button state
     
     // check if a button was pressed
     if(btnPressed != prev_btnPressed){
       prev_btnPressed = btnPressed; //set previous pressed state to current state
       
       // action if button is pressed (HIGH)
       if(btnPressed == HIGH){
     buttonState = tempButtonState; // select button - passed on to buttonAction function
       }
     }else{
       // if buttion is not pressed - reset states (releasing push button)
       buttonState = LOW;
       tempButtonState = LOW;
     }     
   }
   
   // keep current state for comparison on next button event
   lastButtonState = tempButtonState;
   return buttonState; // return button state to indicate which button was pushed
 }
 
void heaterMode(float startTemp, signed int targetTemp, signed int timeToTarget)
{
  //in heat mode activate heater
  toggleRelay(3);
  int relay_state = 1; //keep track of relay position 1 is ON , 0 is OFF
  unsigned long start_time = millis();
  int prev_value = startTemp;
  
  float rate = (targetTemp-startTemp)/((float)timeToTarget);
  
  float middleWare[timeToTarget+1];
  
  for (int i = 0; i < timeToTarget+1; i++)
  {
    middleWare[i] = startTemp + i*rate;
  }
  
  infoDisplay(middleWare[0], startTemp, (timeToTarget));
  
  int j = 1; //indexer for middleWare
  //compare temp to sensor value
  //Serial.println("Starting to heat now");
  while (millis() - start_time < (60000*timeToTarget))
  {
    read_Serial(); // catch command
    
    float sum = 0;
    
    delay(50000); //minute delay before checking again
    for (int i = 0; i < 10; i++)
    {
    sensors.requestTemperatures();
    float sensor_Value = sensors.getTempC(Sensor1);
    sum  += sensor_Value;
    delay(1000);
    }
    
    float ave_temp = (sum)/(10.0);
    
    infoDisplay(middleWare[j], ave_temp, (timeToTarget-j));
    
    // send temperature info to server
    serialTemperature();
    
    //writeToLCD(String(ave_temp), 1); //debug
    
    if (j != timeToTarget)
    {
      //Serial.println(ave_temp);
      //Serial.println(middleWare[j]);
      if (abs(ave_temp - (middleWare[j])) < 0.5) //&& abs((ave_temp + rate) - (middleWare[j+1])) < 0.5)
      {
        //followin pattern nicely, no change to relay
        //Serial.println("No change");
        j++;
        continue;
              
      }
      else
      {
        
        if (ave_temp > middleWare[j])
        {
          //Serial.println("Temp too high");
          //temp too high, switch off
          if (relay_state == 1){
            toggleRelay(4);
            relay_state = 0;
            //Serial.println("Relay OFF");
            j++;
          }
          else
          {
            j++;
            continue;
          }
        }
        else if (ave_temp < middleWare[j])
        {   
            //Serial.println("Temp too low");
            // temp too low, switch relay on if off      
            if (relay_state == 0)
            {          
              //Serial.println("Relay ON");
              toggleRelay(3);
              relay_state = 1;
              j++; 
            }
            else{
              j++;
              continue;
            }      
        }       
      } 
    }
    else{
    // can't compare to future value at end of middleWare
    //done with control
    //Serial.println(ave_temp);    
    }    
  }
   // loop has expired
   //Serial.println("Heating loop complete . . .");
   sensors.requestTemperatures();
   float sensor_Value = sensors.getTempC(Sensor1);
   //Serial.println(sensor_Value);
   toggleRelay(4);
   //Serial.println("Relay shut down");
}

void coolingMode(float startTemp, signed int targetTemp, signed int timeToTarget)
{
  //in cool mode activate compressor
  toggleRelay(2);
  int relay_state = 1; //keep track of relay position 1 is ON , 0 is OFF
  unsigned long start_time_cool = millis();
  int prev_value = startTemp;
  
  float rate = (targetTemp-startTemp)/((float)timeToTarget);
  
  float middleWare[timeToTarget+1];
  
  for (int i = 0; i < timeToTarget+1; i++)
  {
    middleWare[i] = startTemp + i*rate;
  }
  
  infoDisplay(middleWare[0], startTemp, (timeToTarget));
  
  int j = 1; //indexer for middleWare
  //compare temp to sensor value
  //Serial.println("Starting to cool now");
  while (millis() - start_time_cool < (60000*timeToTarget))
  {
    read_Serial(); // catch command
    float sum = 0;
    delay(50000); //minute delay before checking again
    for (int i = 0; i < 10; i++)
    {
    sensors.requestTemperatures();
    float sensor_Value = sensors.getTempC(Sensor1);
    sum  += sensor_Value;
    delay(1000);
    }
    
    float ave_temp = (sum)/(10.0);
    
    infoDisplay(middleWare[j], ave_temp, (timeToTarget-j));
    
    // send temperature data to server
    serialTemperature();
    
    // writeToLCD(String(ave_temp), 1); debug
    
    if (j != timeToTarget)
    {
      //Serial.println(ave_temp);
      //Serial.println(middleWare[j]);
      if (abs(ave_temp - (middleWare[j])) < 0.5) //&& abs((ave_temp + rate) - (middleWare[j+1])) < 0.5)
      {
        //followin pattern nicely, no change to relay
        //Serial.println("No change");
        j++;
        continue;
              
      }
      else
      {
        
        if (ave_temp < middleWare[j])
        {
          //Serial.println("Temp too low");
          //temp too low, switch off
          if (relay_state == 1){
            toggleRelay(4);
            relay_state = 0;
            //Serial.println("Relay OFF");
            j++;
          }
          else
          {
            j++;
            continue;
          }
        }
        else if (ave_temp > middleWare[j])
        {   
            //Serial.println("Temp too high");
            // temp too high, switch relay on if off      
            if (relay_state == 0)
            {          
              //Serial.println("Relay ON");
              toggleRelay(2);
              relay_state = 1;
              j++; 
            }
            else{
              j++;
              continue;
            }      
        }       
      } 
    }
    else{
    // can't compare to future value at end of middleWare
    //done with control
    //Serial.println(ave_temp);    
    }    
  }
   // loop has expired
   //Serial.println("Cooling loop complete . . .");
   sensors.requestTemperatures();
   float sensor_Value = sensors.getTempC(Sensor1);
   Serial.println(sensor_Value);
   toggleRelay(4);
   //Serial.println("Relay shut down");  
}
 
void controlTemp()
{
  
  writeToLCD("Starting control", 0);
  delay(200);
  
  char prev_Mode = 'a'; // char to keep track of the previous operation
  
  // loop through array of temp values
  for (int i = 0; i < temp_Size; i++)
  {
    //delay(500); // delay to give arduino some breathing room
    Serial.print("S"); //signal that next stage has started
    delay(500);
    // check if heat or cool is required - in relation to current temp
    float current_Temp = sensors.getTempC(Sensor1);
    
    if (i != 0)
    {
      // at least one cycle has run, potentially required to keep a value constant
      if (temperatureValues[i] == temperatureValues[i - 1])
      // compare next target to prev target - same? then keep constant
      {
          // check previous mode
          //keep constant
        // but checck previous mode: i.e. if heating, use heater to keep constant, if cooling, use compressor to keep constant
        if(prev_Mode == 'h')
        {
          //use heat mode to keep temp
          heaterMode(current_Temp, temperatureValues[i], timeValues[i]); 
          
          prev_Mode = 'h';
          continue;
        }
        else if (prev_Mode == 'c')
        {
          //use cool mode to keep temp fixed
          coolingMode(current_Temp, temperatureValues[i], timeValues[i]);          
          
          prev_Mode = 'c';
          continue;
        }
      }
    }
    
    // either first run, or not required to keep constant
    
    if (temperatureValues[i] > current_Temp)
    {
      //heating required
      heaterMode(current_Temp, temperatureValues[i], timeValues[i]);       
      prev_Mode = 'h';
      continue;
    }
    else if (temperatureValues[i] < current_Temp)
    {
      //cooling required
      coolingMode(current_Temp, temperatureValues[i], timeValues[i]);     
      prev_Mode = 'c';
      continue;
    } 
    
       
  }
  
  writeToLCD("Run Complete", 0);
  writeToLCD("", 1);
  Serial.print("X"); // signal to server that control is done
  
  // send message to server to indicate that run is complete
  
  
  //float start_Temp = sensors.getTempC(Sensor1);
  //heaterMode(start_Temp, 30, 10);
  //start_Temp = sensors.getTempC(Sensor1);
  //delay(1000);
  //heaterMode(start_Temp, 30, 10);
  //delay(1000);
  //float start_Temp2 = sensors.getTempC(Sensor1);
  //coolingMode(start_Temp2, 20, 10);
  //delay(1000);
  //start_Temp2 = sensors.getTempC(Sensor1);
  //coolingMode(start_Temp2, 20, 10);
}

int read_Serial(void)
{

  while (Serial.available() && actionPending == false)
  {
    data_from_web = Serial.readString();

    if (data_from_web[0] == 'A') 
    {
      char charHolder[data_from_web.length()];
      data_from_web.toCharArray(charHolder, data_from_web.length());
      char *pch;
      pch = strchr(charHolder, 'B');
      int tmp1 = pch - charHolder;
      int tmp2 = data_from_web.length() - tmp1;
      temp1 = data_from_web.substring(0, (tmp1));
      temp2 = data_from_web.substring(tmp1);
      data_from_web = "";
      /*delay(1000);
      writeToLCD("Pointed", 0);
      delay(1000);
      writeToLCD(temp1, 0);
      delay(1000);
      writeToLCD(temp2, 0);
      delay(1000);*/
      
      /*char charHoldertemp[temp1.length()];
      temp1.toCharArray(charHoldertemp,temp1.length());
      int arraySize = findNumValues(charHoldertemp);
      unsigned int array[arraySize];
      int result = parse(charHoldertemp, array, arraySize);*/
      
      temp_Size = string_handler(temp1, temperatureValues);
      time_Size = string_handler(temp2, timeValues);
      
      writeToLCD("READY. . .", 0);
      
      /* debug to print values received from pi      
      int temp;
      for(temp = 0; temp <temp_Size; temp++)
      {
      writeToLCD(String(temperatureValues[temp]),0);
      delay(1000);
      }
      writeToLCD("Time....", 0);
      delay(1000);
      for(temp = 0; temp <time_Size; temp++)
      {
      writeToLCD(String(timeValues[temp]),0);
      delay(1000);
      }*/     
    }
    else if (data_from_web[0] == 'Z')
    {
      // start control
      controlTemp();
      
    }
    else if (data_from_web[0] == 'Y')
    {
    
    // shutdown all relays and reset arduino (stop control)
    toggleRelay(4);    
    resetFunc();
    
    }
  }
}

int string_handler(String string, signed int *array)
{
  char charHolder[string.length()];
  string.toCharArray(charHolder, string.length());
  int arraySize = findNumValues(charHolder);
  parse(charHolder, array, arraySize);
  return arraySize;  
}

int parse(char *string, signed int *array, unsigned int sizeArray)
{
  char *ptr, *end;
  unsigned int count;


  if ((string == NULL) || (strlen(string) < 2)) {
    //fprintf(stderr, "string needs to contain something\n");
    return -1;
  }
  if (array == NULL) {
    //fprintf(stderr, "given array is NULL\n");
    return -1;
  }
  if (sizeArray <= 0) {
    //fprintf(stderr, "unreasonable array size %u\n", sizeArray);
    return -1;
  }

  ptr = string + 2;
  count = 0;

  for (;;)
  {
    array[count] = strtol(ptr, &end, 10);
    if (ptr == end) {
      //fprintf(stderr, "couldn't convert anything in <%s>\n", ptr);
      return -1;
    } else {
      if (end[0] == ',') {
        ptr = end + 1;
        count++;
      } else if (end[0] == '\0') {
        return count + 1;
      } else {
        //fprintf(stderr, "unexpected field in <%s> at <%s>\n", string, end);
        return -1;
      }
    }
    if (count >= sizeArray) {
      return -1;
    }
  }
}

int findNumValues (char *string)
{
  int count = 0;
  int i;

  for (i = 0; i < strlen(string); i++)
  {
    if (string[i] == ',')
    {
      count++;
    }
  }

  return count - 1;
}

void loop()
{
  // put your main code here, to run repeatedly:
  
  /*
  //quick display test
  infoDisplay(10.25, 20.3, 30);
  delay(1000);
  infoDisplay(-5.2, 10, 20);
  delay(1000);
  */ 
  
  //Pi serial comms control
  serialTemperature(); // send temp continuously, even before commands are issued
  delay(100);
  read_Serial(); // catch command
  
  // No Pi Control
  //int temp = CheckButton();  
  //delay(100);  
}
