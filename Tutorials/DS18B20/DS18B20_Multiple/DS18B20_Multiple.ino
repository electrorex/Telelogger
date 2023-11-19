/* This script is a modification of the original example script in the OneWire library written by 
 *  Paul Stoffregen. It has been modified to work with the Telelogger board and shows an example of how
 *  to read multiple DS18B20 devices.
 *  
 *  The DS18B20 device(s) should be connected to the Telelogger on screw terminal CN4.
 *  With the flat side of the sensor pointing up and sensor held down, the order of pin outs is:
 *  Power, Signal, Gound. This matches the order of the pins needed for the device on the CN4 screw 
 *  terminal, which is 3VS, D6, GND.
 *  
 *  The sensor then should be wired in the following way:
 *  Signal -> D6
 *  VCC -> 3VS
 *  GND -> GND
 *  
 *  Add a 4.7K resistor from 3VS to D6
 *  
 *  Although the script is meant to read multiple devices, for demonstration purposes we "pretend" we have
 *  multiple devices and just repeat the same address five times.
 *  
*/

#include <OneWire.h>

//We need to add this adafruit library to use the MCP GPIO expander which will turn on the 3VS power rail
#include "Adafruit_MCP23X17.h" 

//Instatiate the mcp object
Adafruit_MCP23X17 mcp;

#define NSENSORS 5 //Number of DS18B temperature sensors to measure
float Temp[NSENSORS], TempF[NSENSORS];
#define ONE_WIRE_BUS 6 //Temp chain on digital pin 6
OneWire oneWire(ONE_WIRE_BUS);

byte Address[NSENSORS][8] = {
  {0x28, 0x12, 0x98, 0xAC, 0x0D, 0x00, 0x00, 0x09},
  {0x28, 0x3E, 0xBF, 0xAC, 0x0D, 0x00, 0x00, 0x6B},
  {0x28, 0x81, 0xB5, 0xAC, 0x0D, 0x00, 0x00, 0xF5},
  {0x28, 0xD1, 0x79, 0xAC, 0x0D, 0x00, 0x00, 0xD8},
  {0x28, 0x27, 0xA8, 0xAC, 0x0D, 0x00, 0x00, 0x8B}
};

byte present = 0;

void readTemps() {
      for (int i = 0; i < NSENSORS; i++) {
        byte type_s;
        byte rawtemp[12];
        oneWire.reset();
        oneWire.select(Address[i]);
        oneWire.write(0x44, 1);
        delay(1200);     // required delay
        present = oneWire.reset();
        oneWire.select(Address[i]);
        oneWire.write(0xBE);         // Read Scratchpad
        for (int j = 0; j < 9; j++) {           // we need 9 bytes
          rawtemp[j] = oneWire.read();
        }
        int16_t raw = (rawtemp[1] << 8) | rawtemp[0];
        if (type_s) {
          raw = raw << 3;
          if (rawtemp[7] == 0x10) {
            raw = (raw & 0xFFF0) + 12 - rawtemp[6];
          }
        } else {
          byte cfg = (rawtemp[4] & 0x60);
          if (cfg == 0x00) raw = raw & ~7;
          else if (cfg == 0x20) raw = raw & ~3;
          else if (cfg == 0x40) raw = raw & ~1;
        }
        Temp[i] = (float)raw / 16.0;
      }
}

void setup(void) {
  delay(5000); //Give us some time to get the serial monitor up.
  Serial.begin(9600);
  Wire.begin();

  //Begin communications with the MCP over I2C
  mcp.begin_I2C();

  //Turn on the 3VS rail by setting pin 4 on the MCP high
  mcp.pinMode(4,OUTPUT);
  mcp.digitalWrite(4,HIGH);

  //Turn off the 12VS rail by setting pin 7 on the MCP low
  mcp.pinMode(7,OUTPUT);
  mcp.digitalWrite(7,LOW);
   
}

void loop(){
  readTemps();
  for (int i = 0; i < NSENSORS; i++) {
    Serial.print("Temperature Sensor ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.print(Temp[i]);
    Serial.println("degrees C");
  }
  Serial.println(""); //Add space between loops
  delay(1000);
}


















void setup(void) {
  delay(5000); //Give us some time to get the serial monitor up.
  Serial.begin(9600);
  Wire.begin();

  //Begin communications with the MCP over I2C
  mcp.begin_I2C();

  //Turn on the 3VS rail by setting pin 4 on the MCP high
  mcp.pinMode(4,OUTPUT);
  mcp.digitalWrite(4,HIGH);

  //Turn off the 12VS rail by setting pin 7 on the MCP low
  mcp.pinMode(7,OUTPUT);
  mcp.digitalWrite(7,LOW);
   
}

void loop(void) {
  //We do not need to change any of the code in the loop from the original OneWire example.
  byte i;
  byte present = 0;
  byte type_s;
  byte data[9];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
}
