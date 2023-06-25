#ifndef Barometer_h
#define Barometer_h

#include "Arduino.h"

struct result {
  float temperature;
  float pressure;
};

struct OSR {
  uint8_t D1;
  uint8_t D2;
};

enum OSR_OFFSET { SLW = 0, SLW_MED = 2, MED = 4, MED_FST = 6, FST = 8 };

class Barometer {
 public:
  Barometer();
  void begin();
  void end();
  String toString();
  result getResult();
  void setOSR(OSR_OFFSET);
  void printMode();

 private:
  OSR BASE_OSR = {0x40, 0x50};
  OSR SELECTED_OSR = {BASE_OSR.D1 + FST, BASE_OSR.D2 + FST};
  void selectOSR(OSR_OFFSET);
  result res;
  void reset();
  void readprom();
  int32_t pressure();
  int32_t temperature();
  void writeCommand(uint8_t);
  void fetchRegisterValues(uint8_t);
  uint16_t bitShiftCombine16(uint8_t, uint8_t);
  uint32_t bitShiftCombine24(uint8_t, uint8_t, uint8_t);
};
#endif