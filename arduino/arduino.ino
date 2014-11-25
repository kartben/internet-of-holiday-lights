/*******************************************************************************
 * Copyright (c) 2014 IBM Corp. and others
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Benjamin Cabe - Infineon RGB LED control
 *******************************************************************************/

#include <SPI.h>
#include <Bridge.h>
#include <YunClient.h>
#include <IPStack.h>
#include <Countdown.h>
#include <MQTTClient.h>

char printbuf[100];

#define ADDRESS                    0x15EUL
#define INTENSITY_RGB              0x14U
#include <Wire.h>

void messageArrived(MQTT::MessageData& md)
{
  MQTT::Message &message = md.message;

  long number = (long) strtol( (char*)message.payload, NULL, 16);
  long r = number >> 16;
  long g = number >> 8 & 0xFF;
  long b = number & 0xFF;

  sprintf(printbuf, "%s - Number: %ld - R: %d, G: %d, B: %d\n", (char*)message.payload, number, r, g, b);
  Serial.print(printbuf);

  I2CWRITE6BYTES (ADDRESS, INTENSITY_RGB, r << 4, g << 4, b << 4);

}


YunClient yunClient;
IPStack ipstack(yunClient);
MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);

byte mac[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };  // replace with your device's MAC
const char* topic = "benjamin-strip";

void connect()
{
  char hostname[] = "iot.eclipse.org";
  int port = 1883;
  sprintf(printbuf, "Connecting to %s:%d\n", hostname, port);
  Serial.print(printbuf);
  int rc = ipstack.connect(hostname, port);
  if (rc != 1)
  {
    sprintf(printbuf, "rc from TCP connect is %d\n", rc);
    Serial.print(printbuf);
  }

  Serial.println("MQTT connecting");
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.clientID.cstring = (char*)"arduino-sample";
  rc = client.connect(data);
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT connect is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT connected");

  rc = client.subscribe(topic, MQTT::QOS0, messageArrived);
  if (rc != 0)
  {
    sprintf(printbuf, "rc from MQTT subscribe is %d\n", rc);
    Serial.print(printbuf);
  }
  Serial.println("MQTT subscribed");
}

void setup()
{
  Wire.begin();
  Bridge.begin();
  Serial.begin(9600);
  connect();

  // change RGB intensity to make LEDs brighter
  I2CWRITE6BYTES (ADDRESS, 0x24U, 0x60, 0x60, 0x60);
}

void loop()
{
  if (!client.isConnected())
    connect();

  MQTT::Message message;

  while (true)
    client.yield(1000);

  delay(100);
}




/*
Parameters (IN): int Address - Address of RGB LED Shield, Default 0x15E
                int Command - Defined I2C Commands i.e. INTENSITY_RGB, CURRENT_RGB
                unsigned int DataOne, unsigned int DataTwo, unsigned int DataThree - Three 16bit data to be written to slave
Parameters (OUT): None
Return Value: None
Description: This function will write 6 bytes of word to the I2C bus line
*/

void I2CWRITE6BYTES (unsigned int Address, unsigned int Command, unsigned int DataOne, unsigned int DataTwo, unsigned int DataThree) // DataOne: Red, DataTwo: Green, DataThree: Blue
{
  unsigned int upperByte, lowerByte; // Split each Data parameter into upper and lower 8 bytes because I2C format sends 8 bytes of data each time
  lowerByte = DataOne;
  upperByte = DataOne >> 8;

  unsigned int lowerSLAD = (unsigned int) (Address & 0x00FF);
  unsigned int upperSLAD = Address >> 8;
  upperSLAD |= 0x79; // First 5 bits 11110 and last bit '1' for a write

  Wire.beginTransmission(byte(upperSLAD)); // Red
  Wire.write(byte(lowerSLAD));
  Wire.write(byte(Command));
  Wire.write(byte(upperByte));
  Wire.write(byte(lowerByte));
  lowerByte = DataTwo;
  upperByte = DataTwo >> 8;
  Wire.write(byte(upperByte));
  Wire.write(byte(lowerByte));
  lowerByte = DataThree;
  upperByte = DataThree >> 8;
  Wire.write(byte(upperByte));
  Wire.write(byte(lowerByte));
  Wire.endTransmission(true);

}

