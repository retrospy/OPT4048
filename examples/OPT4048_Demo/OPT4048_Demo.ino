/*

This is an example for OPT4048 Digital Ambient Light Sensor

Initial Date: 26-Aug-2023

Hardware connections:
VDD to 3.3V DC
SDA to SDA
SCL to SCL
GND to common ground

MIT License

*/

#include <Wire.h>
#include <OPT4048.h>

OPT4048 opt4048;

#define OPT4048_ADDRESS 0x44

void setup()
{
  Serial.begin(115200);
  Serial.println("OPT4048 Arduino Test");

  opt4048.begin(OPT4048_ADDRESS);
  Serial.print("OPT4048 Device ID:  ");
  Serial.println(opt4048.readDeviceID(), HEX);

  configureSensor();
  printThreshold("High-Limit", opt4048.readHighLimit());
  printThreshold("Low-Limit", opt4048.readLowLimit());
  Serial.println("------------------------------");
}

void loop()
{
  OPT4048_RESULT result0 = opt4048.readResult(CHANNEL_0);
  OPT4048_RESULT result1 = opt4048.readResult(CHANNEL_1);
  OPT4048_RESULT result2 = opt4048.readResult(CHANNEL_2);
  OPT4048_RESULT result3 = opt4048.readResult(CHANNEL_3);

  printAllChannels("OPT4048", result0, result1, result2, result3);
}

void configureSensor() {

  OPT4048_ConfigA newConfigA;
  newConfigA.rawData = 0x3208;  // Default value of the Config A register
  newConfigA.OpMode = 0x03;
  OPT4048_ErrorCode errorConfig = opt4048.writeConfigA(newConfigA);
  if (errorConfig != NO_ERROR)
    printError("OPT4048 configuration", errorConfig);
  else {
    OPT4048_ConfigA sensorConfig = opt4048.readConfigA();
    Serial.println("------------------------------");
    Serial.println("OPT4048 Current Config A:");
    Serial.println("------------------------------");

    Serial.print("Fault Count (R/W):");
    Serial.println(sensorConfig.FaultCount,HEX);
    
    Serial.print("Polarity (R/W):");
    Serial.println(sensorConfig.Polarity,HEX);

    Serial.print("Latch (R/W):");
    Serial.println(sensorConfig.Latch, HEX);

    Serial.print("Operating Mode (R/W):");
    Serial.println(sensorConfig.OpMode, HEX);

    Serial.print("Conversion time (R/W):");
    Serial.println(sensorConfig.ConversionTime, HEX);

    Serial.print("Range Number (R/W):");
    Serial.println(sensorConfig.RangeNumber, HEX);

    Serial.print("Quick Wake-Up (R/W):");
    Serial.println(sensorConfig.QuickWake, HEX);

    Serial.println("------------------------------");
  }

  OPT4048_ConfigB newConfigB;
  newConfigB.rawData = 0x8011;  // Default value of the Config B register
  errorConfig = opt4048.writeConfigB(newConfigB);
  if (errorConfig != NO_ERROR)
    printError("OPT4048 configuration", errorConfig);
  else {
    OPT4048_ConfigB sensorConfigB = opt4048.readConfigB();
    Serial.println("OPT4048 Current Config B:");
    Serial.println("------------------------------");

    Serial.print("I2C Burst (R/W):");
    Serial.println(sensorConfigB.I2CBurst,HEX);
    
    Serial.print("Interrupt Config (R/W):");
    Serial.println(sensorConfigB.IntConfig,HEX);

    Serial.print("Interrupt Direction (R/W):");
    Serial.println(sensorConfigB.IntDirection, HEX);

    Serial.print("Threshold Channel (R/W):");
    Serial.println(sensorConfigB.ThresholdChannel, HEX);

    Serial.println("------------------------------");
  }
  
}

void printResult(String text, OPT4048_RESULT result) {
  if (result.error == NO_ERROR) {
    Serial.print(text);
    Serial.print(": ");
    Serial.print(result.rawResult.Mantissa);
    Serial.print(" x 2^");
    Serial.println(result.rawResult.Exponent);
  }
  else {
    printError(text,result.error);
  }
}

void printAllChannels(String text, OPT4048_RESULT result0, OPT4048_RESULT result1, OPT4048_RESULT result2, OPT4048_RESULT result3) {
  Serial.print(text);
  Serial.print(": ");
  if (result0.error == NO_ERROR) {
    Serial.print(result0.rawResult.Mantissa);
    Serial.print(" x 2^");
    Serial.print(result0.rawResult.Exponent);
    Serial.print(" ");
  }
  else {
    printError(text,result0.error);
  }
  if (result1.error == NO_ERROR) {
    Serial.print(result1.rawResult.Mantissa);
    Serial.print(" x 2^");
    Serial.print(result1.rawResult.Exponent);
    Serial.print(" ");
  }
  else {
    printError(text,result1.error);
  }
  if (result2.error == NO_ERROR) {
    Serial.print(result2.rawResult.Mantissa);
    Serial.print(" x 2^");
    Serial.print(result2.rawResult.Exponent);
    Serial.print(" ");
  }
  else {
    printError(text,result2.error);
  }
  if (result3.error == NO_ERROR) {
    Serial.print(result3.rawResult.Mantissa);
    Serial.print(" x 2^");
    Serial.print(result3.rawResult.Exponent);
    Serial.println();
  }
  else {
    printError(text,result3.error);
  }
}

void printThreshold(String text, OPT4048_THRESHOLD result) {
  if (result.error == NO_ERROR) {
    Serial.print(text);
    Serial.print(": ");
    Serial.print(result.rawResult.Mantissa);
    Serial.print(" x 2^");
    Serial.print(result.rawResult.Exponent);
    Serial.print(" (0x");
    Serial.print(result.rawResult.rawData, HEX);
    Serial.println(")");
  }
  else {
    printError(text,result.error);
  }
}

void printError(String text, OPT4048_ErrorCode error) {
  Serial.print(text);
  Serial.print(": [ERROR] Code #");
  Serial.println(error);
}
