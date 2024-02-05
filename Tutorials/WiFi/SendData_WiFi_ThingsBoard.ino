/*This sketch was written for the Electrorex Telelogger Tutorial 3
It will send data to ThingsBoard using the Telelogger's WiFi modem.
Follow the tutorial instructions to modify.

-T. Rex Miller, Jan. 20, 2024
 */

#include <SPI.h>
#include <WiFi101.h>
#include <driver/source/nmasic.h>
#include "Adafruit_MCP23X17.h"
#include "driver/include/m2m_periph.h"

Adafruit_MCP23X17 mcp;

float AirTemp;
float Batv;
float mvolts;                                                                                      

float readVolts(int pin) {                                                           //Make a function to read voltage on some analog pin and output in millivolts
  analogReadResolution(12);                                                          //Set analog read resolution to 12 (4096 steps)
  mvolts = analogRead(pin);
  mvolts *= 2;
  mvolts *= (3.3 / 4096);
  mvolts *= 1000;
  return mvolts;
}
                                                                                      //Function to read Battery voltage on pin A4 on Telelogger
void readBat() {
  Batv = readVolts(A4)/1000;                                                          //Read battery on A4 and convert millivolts to volts
}

WiFiClient client;
char ssid[] = "XXXXXXXX";                                                             //network SSID (name)
char pass[] = "XXXXXXXXXXXXXX";                                                       // network password 
char server[] = "thingsboard.cloud";                                                  //The server of your ThingsBoard dashboard
char Token[] = "xxxxxxxxxxxxxxxxxxx";   

int status = WL_IDLE_STATUS; 
char ThingString[500];                                                                //character array to hold the POST data

void ThingBoard() {                                                                   //Make a function to send data to the ThingsBoard
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
  }

  
  while (status != WL_CONNECTED) {                                                    //Attempt to connect to WiFi network:
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    
    status = WiFi.begin(ssid, pass);                                                  //Connect to WPA/WPA2 network:
    // wait 
    delay(5000);
    Serial.print("Status = "); Serial.println(status);
    }
                                                                                      //Make the payload
  sprintf(ThingString, "{temperature: %.2f,battery: %.2f}",                           //Add additional sensor name with a comma after %.2f and before bracket
          AirTemp,
          Batv                                                                        //Add additonal sensor float value with a comma after Batv and then the float object name
          );
  Serial.print(F("Payload to send = ")); Serial.println(ThingString);       
  Serial.print(F("Connecting to ")); Serial.println(server);

  if (client.connect(server, 80)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    Serial.println("");
    delay(1000);                                                                       //could make this delay longer to wait for connection
    client.print("POST /api/v1/"); client.print(Token);client.println("/telemetry HTTP/1.1");            //This is the header information
    client.println("Host: thingsboard.cloud");                                         //Note that each header component is on a new line with println, not print
    client.println("User-Agent: telelogger/1.0");
    client.println("Content-Type:application/json");
    String Data = ThingString;
    client.print("Content-Length:");client.println(Data.length());
    client.println("");
    client.println(ThingString);
    delay(2000);                                                                        //Give a little delay here to wait for server response to fill up
    client.flush();
    Serial.println("Reading response from ThingsBoard:");                               //Note, we can parse out the time from this. Will do so in a different tutorial
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    Serial.println("");
    client.flush();                                                                     //this seems to be necessary to prevent buffer issue (https://github.com/arduino-libraries/WiFi101/issues/118)
    client.stop();
  } else {
    Serial.println("connection failed");
  }
}

void setup() {
  delay(5000);
  Serial.begin(115700);                                                                                          
  Serial.println("Using WiFi modem to send data to ThingsBoard using HTTP POST.");      // Print a welcome message
  Serial.println();
  Wire.begin();                                                                         //Do the setup functions necessary to power on the WiFi modem
  mcp.begin_I2C();
  mcp.pinMode(4,OUTPUT);                                                                //Turn off 3.3V rail
  mcp.digitalWrite(4,LOW); 
  delay(100);
  mcp.pinMode(15, OUTPUT);                                                               //Set enable WiFi pin low
  mcp.digitalWrite(15,LOW); 
  delay(100);
  mcp.pinMode(4,OUTPUT);                                                                 //Turn on 3.3V switched rail and thus, the WiFi modem (ensure dip switch is on)
  mcp.digitalWrite(4,HIGH);
  delay(100);
  mcp.pinMode(15, OUTPUT);                                                               //Set enable WiFi pin high to enable the module
  mcp.digitalWrite(15,HIGH); 
  WiFi.setPins(9,7,3,-1);                                                                //Tell WiFi101 library the WiFi pins needed for communications on the Telelogger
                                                                                         //The pin order is CS, IRQ, RST, Enable
  if (WiFi.status() == WL_NO_SHIELD) {                                                   //Check for the presence of the WiFi modem:
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }
  m2m_periph_gpio_set_dir(M2M_PERIPH_GPIO6,1);                                          //This sets GPIO pin 6 on WiFi module to output, which is tied to green LED for WiFi Power
  delay(100);
  m2m_periph_gpio_set_val(M2M_PERIPH_GPIO6,0);                                          //This sets GPIO pin 6 to ground and the LED glows
  Serial.println("Setup is finished");
}

void loop() {
  Serial.println(F("Reading sensor data"));
  readBat();
  Serial.print(F("Battery Voltage = ")); Serial.println(Batv);
  AirTemp = random(0,35);                                                                        //Make up some temperature data
  Serial.println(F("Sending sensor data to ThingSpeak with the WiFi modem"));
  ThingBoard();                                                                           //Send the data with our ThingSpeak function
  Serial.println(F(""));
  Serial.println(F("Finished sending sensor data to ThingsBoard with the WiFi modem"));
  Serial.println(F("Waiting 20 seconds to repeat"));
  Serial.println(F(""));                                                                         
  delay(20000);                                                                            //Data is sent and now lets give a delay before next measurements and data send transmission
                                                                                           //In subsequent tutorial we will show how to time the loop so it runs at predtermined rates.
}
