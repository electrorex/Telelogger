/*This script is a template for sending data to an internet database using the Telelogger board and
 * the Nimbelink Quectel BG96 modem using HTTP Post commands.
 *Todd Miller December 4, 2023
*/
#include "Adafruit_MCP23X17.h"
#define SerialAT Serial1

Adafruit_MCP23X17 mcp;

//String variables to post data that need to be changed
char Token[] = "KRAF9GAODUN6D8CQ";                                                                  
char GetCommand[100]; 
char Payload[2000]; //Make this large enough to hold expected data plus the endpoint
char Endpoint[] = "https://api.thingspeak.com/update.json?api_key";
char SendCommand[100];

void setup() {
  delay(5000);
  Serial.println("Setting up");
  Serial.begin(115700); 
  SerialAT.begin(115700);

  //Turn on red LED so we know board has power
  pinMode(13, OUTPUT); 
  digitalWrite(13,HIGH);
  Wire.begin();
  mcp.begin_I2C(0x20);
  mcp.pinMode(7,OUTPUT);
  mcp.digitalWrite(7,LOW); //Turn off 12VS rail
  mcp.pinMode(4, OUTPUT);
  mcp.digitalWrite(4, HIGH); //Turn on 3VS rail

  //Turn off all other LEDs to start
  mcp.pinMode(8,OUTPUT);
  mcp.digitalWrite(8,LOW);
  mcp.pinMode(9,OUTPUT);
  mcp.digitalWrite(9,LOW);
  mcp.pinMode(10,OUTPUT);
  mcp.digitalWrite(10,LOW);
  
  SetupCell();
  Serial.println("Setup complete");
}

void loop() {
  mcp.digitalWrite(8,HIGH); //Turn on LED2
  Serial.println("Starting a data transmission");
  Serial.println("Reading sensors");
  mcp.digitalWrite(9,HIGH); //Turn on LED3
  readSensors();
  delay(3000);
  Serial.println("Posting sensor data");
  mcp.digitalWrite(9,LOW); //Turn off LED3
  mcp.digitalWrite(10,HIGH); //Turn on LED4
  GetData();
  mcp.digitalWrite(10,LOW); //Turn off LED4
  mcp.digitalWrite(8,LOW); //TUrn off LED2
  Serial.println("Finished data transmission");
  Serial.println("*****************************************************************************");
  delay(5000);
}
