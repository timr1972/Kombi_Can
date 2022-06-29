#include <SPI.h>
#include "mcp2515_can.h"

const int SPI_CS_PIN = 10;
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

void setup() {
   Serial.begin(115200);
   while(!Serial){};
   // init can bus : baudrate = 500k
   while (CAN_OK != CAN.begin(CAN_500KBPS)) {
      Serial.println("CAN init fail, retry...");
      delay(100);
   }
   Serial.println("CAN init ok!");
}

unsigned char stmp_abs[8] = {0, 0, 0, 0x36, 0, 0, 0, 0};
unsigned char stmp_kombi[8] = {0, 0, 0, 0, 0x0, 0x0, 0, 0};
// kombi is 0x320
// abs is 0x1a0
/*
packet is +-------------------+------------+-----+-------+----+------+--------+---------------+-------------------------+-------------------+
|Geschwindig- | vfzgkb_w | 4 | 1 | 7 | 0 | FF | 0..326 km/h | (Phys) = 0,01 * (Hex) | low Byte |
|keit | | | | | | | | | |
|*X* | s.u. | 5 | 0 | 8 | 0 | | | high Byte | high Byte |
+-------------------+------------+-----+-------+----+------+--------+---------------+-------------------------+----------------
*/
void loop() {
   // send data: id = 0x00, standrad frame, data len = 8, stmp: data buf
   
   stmp_kombi[4] = stmp_kombi[4] + 0x1;
   if (stmp_kombi[4] == 0x9f) {
      stmp_kombi[4] = 0x00;
   }

   CAN.sendMsgBuf(0x320, 0, 8, stmp_kombi);
   delay(20); // send data per 50ms
   Serial.println("CAN BUS sendMsgBuf ok!");
}
