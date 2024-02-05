/*This sketch was written for the Electrorex Telogger Tutorial 8
Please see tutorial 8 electrorex.io for how to use this sketch.
-Todd R. Miller, February 5, 2024
 
This sketch will read an EXO2 sonde through the YSI DCP adaptor on the Telelogger's RS232 serial port, TX2/RX2.

Wiring from YSI DCP Adaptor to the Telelogger
9-12VDC IN ---> 12VS
GND ----> GND
GND ----> GND (there are two GND ports on the DCP adaptor)
RS232-TX ----> RX2
RS232-RX ----> TX2

Follow the YSI EXO manual for how to wire the EXO sonde to the DCP adaptor

The RS232 port communicates with the second serial port on the Telelogger processor through the UART MUX.
The RS232 ports on the Telelogger are marked as RX1/TX1 and RX2/TX2 
These RS232 ports are selectable through the UART MUX with MCP GPIO pins 11, 12, and 13.
Set them high or low to select the correct port according to the table below.
 
Table. UART Multiplexer addressing
                    GPB11 (E) | GPB12 (S0) | GPB13 (S1)
                    ___________________________________
LoRa (RX0/TX0)     | LOW      |    LOW     |   LOW 
RS232-1 (RX1/TX1)  | LOW      |    HIGH    |   LOW
RS232-2 (RX2/TX2)  | LOW      |    LOW     |   HIGH
UART (RX3/TX3)     | LOW      |    HIGH    |   HIGH

The EXO is on the TX2/RX2 port so GPIO pins 11, 12 and 13 will be set low, low, and high, respectively.
Then the Telelogger can send/receive RS232 commands and data from the EXO on Serial2 of the processor.

This sketch will turn the EXO on with the switched 12VS rail of the Telelogger, then read EXO data and then turn it off
Each time the EXO is turned on, the wiper will be activated and the sketch waits 35 seconds before requesting data.
*/

//Libraries needed
#include "wiring_private.h" 
#include "Adafruit_MCP23X17.h"

//Make a new serial port, Serial2, on digital pins 10 and 12
//Pin 10 and 12 are RX and TX for UART multiplexer communications
#define PIN_SERIAL2_RX       (34ul)                                           // Pin description number for PIO_SERCOM on D12
#define PIN_SERIAL2_TX       (36ul)                                           // Pin description number for PIO_SERCOM on D10
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)                                  // SERCOM pad 2
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)                                // SERCOM pad 3

Uart Serial2(&sercom1, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);
// Interrupt handler for SERCOM1
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

#define ss Serial2 //Second Serial (ss) port communication with UART multiplexer

//Some variables needed to hold information from the EXO sonde
char EXOData[200];
const char * EXOFirst; 
float EXOTempC;
float EXOspCond;
float EXODepth;
float EXOBat;
float EXOPwr = 0;
float EXOTurb;
int EXODate;
int EXOTime;
float EXODOSat;
float EXOChl;
float EXOPhy;                                                          

//Function to read data from EXO sonde
void readEXO() {
  mcp.pinMode(7, OUTPUT);
  mcp.digitalWrite(7, HIGH); //Turn on 12VS rail to power EXO

  Serial.println("Waiting for EXO to turn on and to wipe sensors");
  delay(40000); //EXO requires at least 35 seconds to start. 

  //Set correct UART MUX port for TX2/RX2
  mcp.pinMode(11,OUTPUT); 
  mcp.digitalWrite(11,LOW); //E
  delay(100);
  mcp.pinMode(12,OUTPUT); 
  mcp.digitalWrite(12,LOW); //S0
  delay(100);
  mcp.pinMode(13,OUTPUT);
  mcp.digitalWrite(13,HIGH); //S1
  delay(100);
  
  while(ss.available() > 0){
    char t = ss.read(); // make sure buffer is clear
  }
  
  Serial.println("Reading EXO Sonde");

  //Request data a few times to make sure data is coming
  ss.print("data\r");
  delay(1500);
  Serial.println(ss.readString());
  
  ss.print("data\r");
  delay(1500);
  Serial.println(ss.readString());

  ss.print("data\r");
  delay(1500);
  Serial.println(ss.readString());

  //read data as strong and put into char array
  String EXOReceived1 = ss.readString();
  EXOReceived1.toCharArray(EXOData,200);
  Serial.println(EXOData);

  /*This EXO has the following parameters in this order
   Date
   Time
   Depth
   Water Temperature C
   Dissolved Oxygen Saturation
   Specific Conductivity
   Chlorophyll
   Phycocyanin
   Turbidity
   Internal Battery Voltage
   Voltage on EXO Cable
   */

  //Parse the comma separated string
  char * strtokIndx; 
    
  strtokIndx = strtok(EXOData,"\n"); 
  EXOFirst = strtokIndx; //throw away characters

  strtokIndx = strtok(NULL, " ");
  EXODate = atoi(strtokIndx);

  strtokIndx = strtok(NULL, " ");
  EXOTime = atoi(strtokIndx);

  strtokIndx = strtok(NULL, " ");
  EXODepth = atof(strtokIndx);

  strtokIndx = strtok(NULL," "); 
  EXOTempC = atof(strtokIndx);

  strtokIndx = strtok(NULL," "); 
  EXODOSat = atof(strtokIndx);
  
  strtokIndx = strtok(NULL, " ");
  EXOspCond = atof(strtokIndx);

  strtokIndx = strtok(NULL, " ");
  EXOChl = atof(strtokIndx);

  strtokIndx = strtok(NULL, " ");
  EXOPhy = atof(strtokIndx);

  strtokIndx = strtok(NULL, " ");
  EXOTurb = atof(strtokIndx);

  strtokIndx = strtok(NULL, " ");
  EXOBat = atof(strtokIndx);

  strtokIndx = strtok(NULL, " ");
  EXOPwr = atof(strtokIndx);

  
  Serial.println("====================================================");
  Serial.print("EXO Water Temperature (C) = "); Serial.println(EXOTempC);
  Serial.print("EXO Dissolved Oxygen Saturation (%) = "); Serial.println(EXODOSat);
  Serial.print("EXO Specific Conductivity (uS/cm) = "); Serial.println(EXOspCond);
  Serial.print("EXO Turbidity (NTU) = "); Serial.println(EXOTurb);
  Serial.print("EXO Chlorophyll (RFU) = "); Serial.println(EXOChl);
  Serial.print("EXO Phycocyanin (RFU) = "); Serial.println(EXOPhy);
  Serial.print("EXO Depth (m) = "); Serial.println(EXODepth);
  Serial.print("EXO Battery Voltage (V) = "); Serial.println(EXOBat);
  Serial.print("EXO Cable Voltage (V) = "); Serial.println(EXOPwr);
  Serial.print("EXO Date (DDMMYY) = "); Serial.println(EXODate);
  Serial.print("EXO Time (HHMMSS) = "); Serial.println(EXOTime);
  Serial.println("====================================================");
  
  ss.print("pwruptorun 0\r"); //Tell EXO to power down
  delay(1000);
  mcp.digitalWrite(7, LOW); //Turn off 12VS rail and EXO
}

void setup() {
  delay(5000); //wait a bit to get serial monitor up
  
  //Start all of the serial ports
  Serial.begin(115200);
  ss.begin(9600); //default baud rate for the EXO sonde

  //Turn on red LED1 to indicate the program has started
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  
  Serial.println("Setting things up. Please wait");

  Wire.begin();
  mcp.begin_I2C(0x20);

  mcp.pinMode(4, OUTPUT);
  mcp.digitalWrite(4, HIGH); //Turn on 3VS rail
  delay(100);
  
  mcp.pinMode(7, OUTPUT);
  mcp.digitalWrite(7, LOW); //Turn off 12VS rail
  delay(100);
  
  mcp.pinMode(8, OUTPUT);
  mcp.digitalWrite(8, LOW); //Turn off indicator LED2
  delay(100);
  
  mcp.pinMode(9, OUTPUT);
  mcp.digitalWrite(9, LOW); //Turn off indicator LED3
  delay(100);
  
  mcp.pinMode(10, OUTPUT);
  mcp.digitalWrite(10, LOW); //Turn off indicator LED4
  delay(100);
  
  Serial.println("Setup finished");
}

void loop() {
  readEXO();
  delay(10000); //Wait ten seconds before doing another read
}
