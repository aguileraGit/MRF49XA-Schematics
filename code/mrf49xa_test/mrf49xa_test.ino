#include <SPI.h>

int LED = 13;
int ChipSelect = 8;

uint16_t STSREAD = 0x00;
//The registers consist of a command code, followed by control, data, status or parameter bits.

void setup(void) {
  //Set LED
  pinMode(LED, OUTPUT);
  
  //Set SS
  pinMode(ChipSelect, OUTPUT);
  
  //Begin Serial
  Serial.begin(57600);
  
  delay(1000);
}

void loop(void) {
  spiBegin();
  delay(5);
  
  //spiCommand( FIFORSTREG );
  //spiCommand( FIFORSTREG | 0x0002 );
  
  spiCommand( STSREAD );
  uint8_t MRF49XA = spiRead();
  
  Serial.println(MRF49XA, HEX);
  
  spiEnd();
}

void spiBegin(void) {
  SPI.begin();
}

void spiEnd(void) {
  SPI.end();
}

uint16_t spiRead(void) {
  digitalWrite(ChipSelect, LOW);
  uint8_t spiData0 = SPI.transfer(0x00);
  uint8_t spiData1 = SPI.transfer(0x00);
  
  uint16_t spiData = (uint16_t)(spiData0 << 8 | spiData1);
  
  return spiData;
  digitalWrite(ChipSelect, HIGH);
}

void spiWrite(uint8_t spiData) {
  digitalWrite(ChipSelect, LOW);
  SPI.transfer(spiData);
  digitalWrite(ChipSelect, HIGH);
}

void spiCommand(uint16_t spiCmd) {
  spiWrite((spiCmd & 0xFF00) >> 8);
  spiWrite((spiCmd & 0x00FF));
}

 
 
  

