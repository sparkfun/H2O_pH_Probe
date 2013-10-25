/*********************************************************
H2O pH Probe 
Joel Bartlett
SparkFun Electronics
October 25, 2013

License: 
This code is beerware: feel free to use it, with or without attribution, 
in your own projects. If you find it helpful, buy me a beer next time you 
see me at the local pub.

This code is for the H20 pH Probe - a handheld device that uses an Arduino
Pro Mini, a Serial Enabled LCD, a DS18B20 waterproof temerature sensor,
and the pH kit from Atlas Scientific to quickly and accurately measure the pH
of any liquid. 

This code was written with Arduino 1.0.5 (available at arduino.cc) It should 
work with any Arduino IDE version from 1.0 and on. 

This uses some code found in the example skecth provided by Atlas Scientific for the pH stamp, 
the DS18B20 example sketch found on bildr - http://bildr.org/2011/07/ds18b20-arduino/
and some of the functions for the SerLCD Library http://playground.arduino.cc/Learning/SparkFunSerLCD
***********************************************************/
#include <SoftwareSerial.h> // include the software serial library to add an aditional serial port to talk to the pH stamp
#include <OneWire.h>// include the oneWire library to communcate with the temerature sensor 

//Create an instance of the softwareSerial class named pHserial
SoftwareSerial pHserial(2,3); // (RX,TX) 

OneWire ds(12); //create an instance of the oneWire class named ds, communication will happen on pin 12

char inputBuffer[10];   // For incoming serial data

int button1 = 6; //calibrate button 
int button2 = 7; //test button

int tmpPwr = 10; //power line for the temp sensor 
//-----------------------------------------------------------
void setup()
{ 
  pinMode(button1, INPUT); //make button1 an input
  digitalWrite(button1, HIGH); //turn on internal pull-up resistor
  pinMode(button2, INPUT); //make button2 an input
  digitalWrite(button2, HIGH); //turn on internal pull-up resistor
  
  pHserial.begin(38400);//the pH stamp communicates at 38400 baud by default 
  Serial.begin(9600);//the LCD commincates at 9600 baud by default 
  
  getTemp();//the first value seems to be a throwaway value, so we call it once here to get rid of the garbage value. 
   
  pinMode(tmpPwr, OUTPUT);
  digitalWrite(tmpPwr, HIGH);//it was easier to attach this temp sensor's power pin near to the signal 
  //pin on account of the 4.7Kohm resistor needed between them.
  
  //backlightHalf();
  //this function was only called once to set the backlight at half brightness. When it was at full brightness, 
  //the whole circuit pulled a little more curent than the 9V battery could provide. 
}
//-----------------------------------------------------------
void loop() 
{
  if(digitalRead(button1) == LOW)  //if button1 is pressed, calibrate
   {
      clearLCD();// clear the LCD screen
      selectLineOne();//move up to the first line of the LCD
      goTo(3);//move in to position 3 on the LCD for centered text
      Serial.print("Calibrating");
      calibrate();
      delay(500);//debounce
   }
  if(digitalRead(button2) == LOW) //if button2 is pressed, read pH and temp once
  {
      clearLCD();// clear the LCD screen
      selectLineOne();//move up to the first line of the LCD
      goTo(3);//move in to position 3 on the LCD
      Serial.print("Temp=");//print the current temp in degrees F
      Serial.print(convertTemp());
      Serial.print("F");  
      selectLineTwo();//move down to the second line of the LCD
      goTo(21);//move in to position 21
      Serial.print("pH=");//print the current pH
      getPh();
      delay(500);//debounce
  }
}
//-----------------------------------------------------------
float getTemp()
{
  //returns the temperature from one DS18S20 in degrees C
  byte data[12];
  byte addr[8];
  
  if ( !ds.search(addr)) {
  //no more sensors on chain, reset search
  ds.reset_search();
  return -1000;
  }
  
  if ( OneWire::crc8( addr, 7) != addr[7]) {
  //Serial.println("CRC is not valid!");
  return -1000;
  }
  
  if ( addr[0] != 0x10 && addr[0] != 0x28) {
  //Serial.print("Device is not recognized");
  return -1000;
  }
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end
  
  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad
  
  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
  data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];
  
  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum; // returns the temeperature in degrees C
}
//-----------------------------------------------------------
float convertTemp()
{
  //This function converts the temerature from celcius to farenheit
  float temperature = getTemp();
  float tempF = temperature * 1.8 + 32;
  return(tempF);
}
//-----------------------------------------------------------
void getPh()
{
  //This function queries the pH stamp to return one reading of pH
  pHserial.print("R\r"); //send R followed by a carriage return prompts the stamp to send back a single pH reading
  delay(10);
  
  pHserial.readBytesUntil(13,inputBuffer,20);//this reads back the results into the inputBuffer we created until it sees 
  //a carriage return (13) or until it reaches 20 bytes (which it shouldn't)
  delay(500);
  
  Serial.print(inputBuffer);// print the pH value to the Serial port, which is connected to the LCD.
}
//-----------------------------------------------------------
void calibrate()
{
  //This function calibrates the pH stamp with the current temperature read from the DS18B20 temp sensor
  delay(500);
  pHserial.print(getTemp()); //pritn temp in degrees C to Ph Sensor
  pHserial.write("\r"); //carrage return
  delay(1000);
  clearLCD();
  selectLineOne();//move up to the first line of the LCD
  goTo(4);//Move in 4 spaces on the second line to center text
  Serial.print("Complete");
  selectLineTwo();//move down to the second line of the LCD
  goTo(19);//move in to position 19 to center text
  Serial.print("Temp=");//print the temperture that was used to calibrate to the LCD
  Serial.print(convertTemp());
  Serial.print("F");
  delay(500);
}
//-----------------------------------------------------------
// Resets the display, undoing any scroll and removing all text
void clearLCD() 
{
   Serial.write(0xFE);
   Serial.write(0x01);
}
//-----------------------------------------------------------
// Starts the cursor at the beginning of the first line (convienence method for goTo(0))
void selectLineOne() 
{ //puts the cursor at line 0 char 0.
   Serial.write(0xFE); //command flag
   Serial.write(128); //position
}
//-----------------------------------------------------------
// Starts the cursor at the beginning of the second line (convienence method for goTo(16))
void selectLineTwo() 
{ //puts the cursor at line 1 char 0.
   Serial.write(0xFE); //command flag
   Serial.write(192); //position
}
//-----------------------------------------------------------
// Sets the cursor to the given position
// line 1: 0-15, line 2: 16-31, 31+ defaults back to 0
void goTo(int position) 
{
  if (position < 16) 
  {
    Serial.write(0xFE); //command flag
    Serial.write((position+128));
  } else if (position < 32) 
  {
    Serial.write(0xFE); //command flag
    Serial.write((position+48+128));
  } else
  {
    goTo(0);
  }
}
//-----------------------------------------------------------
// Turns the backlight on, full brightness
void backlightOn() 
{
    Serial.write(0xFE);
    Serial.write(157);
}
//-----------------------------------------------------------
// Turns the backlight to half brightness
void backlightHalf() 
{
    Serial.write(0xFE);
    Serial.write(140);
}
//-----------------------------------------------------------

