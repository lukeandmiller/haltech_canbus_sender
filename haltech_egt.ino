#include <Arduino.h>
#include <mcp_can.h> // Librairie CAN
#include <SPI.h>
#include <SD.h>


MCP_CAN CAN(9); // CS = ChipSelect
const int numIntegers = 4;             // Number of integers in the array
int EGT[numIntegers] = {819, 1638, 3276, 4095}; // Array of integers
const int numBytes = numIntegers * 2; // Total number of bytes (2 bytes per integer)
byte byteArray[numBytes];             // Byte array to store the 8-bit segments

#define haltech_A_ID 0x2C0 //Can Request

void setup() {
  Serial.begin(115200);

  // Initialize MCP2515 running at 16MHz with a baudrate of 1000kb/s and the masks and filters disabled.
  if(CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
}

void GetEgtTemp(){
  for (int pin = 0; pin < 4; pin++)
  {
    // EGT pins are A0-A3
    int dummy = analogRead(pin);  //read values twice as advised
    int value = analogRead(pin);
    int calc = (((value * .0048875855327468)* 4095)/5);
    EGT[pin] = calc;
//    Serial.print("EGT");
//    Serial.print(pin);
//    Serial.print("=   ");
//    Serial.print(value);
//    Serial.print("   ");
//    float voltage = (value * .0048875855327468);
//    Serial.print(voltage);
//    Serial.print(" V  ");
//    float temp = (388.48*voltage)+56;
//    Serial.print(temp);
//    int encode = (voltage * 4095)/5;
//    Serial.print(" INT ENCODE = ");
//    Serial.print(encode);
//    Serial.println();
  }
}

void loop() {

   GetEgtTemp();

  // Haltech requires individual values to be sent in hex pairs (Bigendian). This splits the bits using bitewise ops
  for (int i = 0; i < 4; i++) {
    // Split the integer into high and low bytes
    byte highByte = (EGT[i] >> 8) & 0xFF; // Extract the high byte
    byte lowByte = EGT[i] & 0xFF;         // Extract the low byte

    // Store the bytes in the byte array
    byteArray[i * 2] = highByte;
    byteArray[i * 2 + 1] = lowByte;
  }

  // Send the byte array over CAN
  Serial.println("Sending byte array over CAN:");
  for (int i = 0; i < numBytes; i += 8) {
    // Prepare a frame of up to 8 bytes (CAN frame size limit)
    byte frame[8];
    int frameSize = min(8, numBytes - i);
    for (int j = 0; j < frameSize; j++) {
      frame[j] = byteArray[i + j];
    }

    // Send the frame
    if (CAN.sendMsgBuf(haltech_A_ID, 0, frameSize, frame) == CAN_OK) { 
      Serial.print("Frame sent: ");
      for (int j = 0; j < frameSize; j++) {
        Serial.print(frame[j], HEX);
        Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println("Error sending frame");
    }
  }
 
 }
