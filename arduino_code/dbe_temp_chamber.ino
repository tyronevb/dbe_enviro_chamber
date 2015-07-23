// neccessary incldues
#include <OneWire.h>
#include <DallasTemperature.h>
#include <String.h>
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include <LiquidCrystal.h>

// Data wire is plugged into pin 8 on the Arduino
#define ONE_WIRE_BUS 8
#define LCD_SSPIN 9

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

//initialize LCD instance
LiquidCrystal lcd(LCD_SSPIN);

// Setup a sensors instance to use the oneWire instance to communicate with temp sensors
DallasTemperature sensors(&oneWire);

// Set device addresses
DeviceAddress Sensor1 = {0x28, 0xC6, 0x55, 0xB6, 0x06, 0x00, 0x00, 0x3D};
DeviceAddress Sensor2 = {0x28, 0x95, 0xA8, 0xB5, 0x06, 0x00, 0x00, 0x88};
DeviceAddress Sensor3 = {0x28, 0x2F, 0xCF, 0xB6, 0x06, 0x00, 0x00, 0x88};

//Pins to control Relays (LEDS for now)
#define power_Relay 1
#define cooling_Relay 2
#define heating_Relay 3
#define idle_mode 4
#define cool_SSR_pin 5
#define heat_SSR_pin 6
#define power_Real_pin 7

// variables to control pin states
int power_relay_cntrl = 0;
int cooling_relay_cntrl = 0;
int heating_relay_cntrl = 0;
int idle_mode_cntrl = 1;

unsigned int temperatureValues[8];
unsigned int timeValues[8];

char tmp;
char count;

//string handler for incoming serial data
String data_from_web = "";
String temp1;
String temp2;


// control variable
bool actionPending = false;
char action;

// Function prototypes
void initRelays(void);
void initTempSensors(int );
void printTemperature(DeviceAddress );
void serialTemperature(void);
void set_LEDS(bool, bool, bool, bool, bool, bool, bool);
int read_Serial(void);
int stringToInt(void);
void control_Sim(void);
void states(void);
void clearLCDLine(int);
void writeToLCD(String, int);
void initLCD(int, int);
void clearChar(char );
int findNumValues (char );
int parse(char, unsigned int , unsigned int);
int string_handler(String , unsigned int );

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  initTempSensors(10);
  initRelays();
  initLCD(2, 16);
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
void writeToLCD(String someData, int lineNum) {
  clearLCDLine(lineNum);
  lcd.print(someData);
}

// initialize LCD
void initLCD(int rows, int chars) {
  lcd.begin(chars, rows);
  clearLCDLine(0);
  lcd.print("LCD Initialized");
}

// Initialization for Relays Pins
void initRelays()
{
  // initialize pins as outputs
  pinMode(power_Relay, OUTPUT);
  pinMode(cooling_Relay, OUTPUT);
  pinMode(heating_Relay, OUTPUT);
  pinMode(idle_mode, OUTPUT);

  pinMode(cool_SSR_pin, OUTPUT);
  pinMode(heat_SSR_pin, OUTPUT);
  pinMode(power_Real_pin, OUTPUT);

  //set initial state of pins to 0
  digitalWrite(power_Relay, 0);
  digitalWrite(cooling_Relay, 0);
  digitalWrite(heating_Relay, 0);

  // system starts in idle mode
  digitalWrite(idle_mode, 1);

  digitalWrite(cool_SSR_pin, 0);
  digitalWrite(heat_SSR_pin, 0);
  digitalWrite(power_Real_pin, 1);
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
    Serial.print(0);
  } else {
    Serial.print(tempC);
  }
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

void set_LEDS(bool led_1, bool led_2, bool led_3, bool led_4, bool power_relay, bool heat_ssr, bool cool_ssr)
{
  digitalWrite(power_Relay, led_1);
  digitalWrite(cooling_Relay, led_2);
  digitalWrite(heating_Relay, led_3);
  digitalWrite(idle_mode, led_4);

  digitalWrite(cool_SSR_pin, cool_ssr);
  digitalWrite(heat_SSR_pin, heat_ssr);
  digitalWrite(power_Real_pin, power_relay);
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
      
      int temp_Size = string_handler(temp1, temperatureValues);
      int time_Size = string_handler(temp2, timeValues);
            
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
      }
     
    }
  }
}

int string_handler(String string, unsigned int *array)
{
  char charHolder[string.length()];
  string.toCharArray(charHolder, string.length());
  int arraySize = findNumValues(charHolder);
  parse(charHolder, array, arraySize);
  return arraySize;  
}
  

int parse(char *string, unsigned int *array, unsigned int sizeArray)
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

void states(void)
{
  // power relay and green light
  set_LEDS(1, 0, 0, 0, 0, 0, 0);
  delay(500);

  // power relay + cooling: green and yellow light
  set_LEDS(1, 0, 0, 1, 0, 0, 1);
  delay(500);

  // power relay + heating: green and orange light
  set_LEDS(1, 0, 1, 0, 0, 1, 0);
  delay(500);

  // all on, all off
  set_LEDS(1, 1, 1, 1, 1, 0, 0);
  delay(500);
}

void control_Sim(void)
{
  /*
  while (read_Serial() != 1)
  {
    serialTemperature();
    continue;
  }

  while (read_Serial() != 2 && read_Serial() != 3)
  {
    states();
    serialTemperature();
  }*/
}

void loop(void)
{
  delay(50);
  //control_Sim();
  read_Serial();
  serialTemperature();
  //set_LEDS(0,1,0,0,1,0,0);
}
