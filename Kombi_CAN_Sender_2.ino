#include <SPI.h>
#include "mcp2515_can.h"

const int SPI_CS_PIN = 10;
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

const byte speed_pin = 2;                    // This is the ground pulsed pin for speed
const byte pulses = 4;                       // This is the number of pulses per revolution
const int tyre_circ = 1;                     // Tyre rolling radius in m
int speedkmh = 0;
long last_read_time = 0;
long speed_read[4] = {0, 0, 0, 0, 0};
long average_speed = 0;

void setup() {
   Serial.begin(115200);
   while(!Serial){};
   while (CAN_OK != CAN.begin(CAN_500KBPS)) {
      Serial.println("CAN init fail, retry...");
      delay(100);
   }
   Serial.println("CAN init ok!");
   pinMode(speed_pin, INPUT_PULLUP); // Set pinmode and enable pullup resistor
   attachInterrupt(digitalPinToInterrupt(speed_pin), speedRead, LOW); // Run the speedRead routine when the pin is low
}

/*
abs is 0x1a0
   byte 3, bit 1, 7 bits long = 0-1.27 km/h (PH = 0.01 * HEX)
   byte 4, bit 0, 8 bits long = 0-325.12 km/h (ph = 1.28 * HEX)
*/
unsigned char stmp_abs[8] = {0x0, 0x0, 0x0, 0x00, 0x0, 0x0, 0x0, 0x0};
/*
kombi 1 is 0x320
   tfst
   byte 3, bit 0, 7 bits long = 0..126 ltr (PH = HEX) Tank capacity
   vfzgkb_w
   byte 4, bit 1, 7 bits long & byte 5, bit 0, 8 bits long = 0..326km/h (PH = 0.01 * Hex)
   byte 4 is Low byte
   byte 5 is high byte
   60km/h / 0.01 = 6000 in binary is; 001 0111 0111 0000
   byte4 = 0x38
   byte5 = 0x2E
   
kombi 2 is 0x420
kombi 3 is 0x520

*/
unsigned char stmp_kombi[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

void loop() {
   stmp_kombi[4] = stmp_kombi[4] + 0x1;
   if (stmp_kombi[4] == 0x9f) {
      stmp_kombi[4] = 0x00;
   }

   CAN.sendMsgBuf(0x320, 0, 8, stmp_kombi);
   delay(20); // send data per 50ms
   Serial.println("CAN BUS sendMsgBuf ok!");
}

void speedRead() {
   long read_delta = last_read_time - millis();
   last_read_time = millis()
   speed_read[0] = speed_read[1];
   speed_read[1] = speed_read[2];
   speed_read[2] = speed_read[3];
   speed_read[4] = read_delta;
   average_speed = pulses*((speed_read[0] + speed_read[1] + speed_read[2] + speed_read[3] + speed_read[4])/5); // Average rotation speed
   speedkmh = (3600*tyre_circ)/average_speed;
}
