#include "Barometer.h"

#include <Wire.h>

#include <cmath>

#include "Arduino.h"

uint8_t I2C_ADDRESS = 0x77;
// uint8_t addressBaroRead = 0x78;
uint8_t cmd_reset = 0x1E;
uint8_t cmd_prom_read = 0xA0;

uint8_t cmd_adc_read = 0x00;
uint16_t C[8] = {0};  // Co-efficients
const uint8_t TWENTY_DEGREES_C = 2000;
uint32_t D1 = 0;
uint32_t D2 = 0;
uint8_t two = 2;
uint8_t three = 3;
int32_t TEMP = 0;
int32_t dT = 0;
const int DELAY = 15;

Barometer::Barometer() {}

void Barometer::begin() {
  Wire.begin();
  reset();
  readprom();
}

void Barometer::setOSR(OSR_OFFSET o) {
  SELECTED_OSR.D1 = BASE_OSR.D1 + o;
  SELECTED_OSR.D2 = BASE_OSR.D2 + o;
}
void Barometer::printMode() {
  Serial.println(SELECTED_OSR.D1);
  Serial.println(SELECTED_OSR.D2);
}
void Barometer::writeCommand(uint8_t b) {
  Wire.beginTransmission(I2C_ADDRESS);
  Wire.write(b);
  Wire.endTransmission();
  delay(DELAY);
}

result Barometer::getResult() {
  fetchRegisterValues(SELECTED_OSR.D1);
  fetchRegisterValues(SELECTED_OSR.D2);
  //   conversion(cmd_adc_d2);
  int32_t calibratedTemp = temperature();
  int32_t calibratedPx = pressure();

  return res = {(float)calibratedTemp / 100, (float)calibratedPx / 100};
}

void Barometer::end() { Wire.end(); }

String Barometer::toString() {
  char *msgOut;
  asprintf(&msgOut, "Temp: %.2f°C, Px: %.2f Hpa", res.temperature,
           res.pressure);

  String s = msgOut;
  free(msgOut);
  return s;
}

void Barometer::reset() { writeCommand(cmd_reset); }

//****************************************************************
void Barometer::readprom() {
  uint16_t result = 0;
  uint8_t steps = cmd_prom_read;
  uint8_t msb, lsb = 0;

  for (int i = 0; i <= 7; i++) {
    writeCommand(cmd_prom_read);

    Wire.requestFrom(I2C_ADDRESS, 2);
    delay(DELAY);
    int bytesAvailable = Wire.available();
    if (bytesAvailable == 2) {
      msb = Wire.read();
      lsb = Wire.read();
    } else {
      Serial.println("error, two bytes weren't returned");
    }

    result = bitShiftCombine16(msb, lsb);
    C[i] = result;
    cmd_prom_read += 2;
  }
}

// Raw Px & Temp ***************************************************
void Barometer::fetchRegisterValues(uint8_t command) {
  uint32_t result = 0;
  uint8_t msb, smsb, lsb = 0;

  writeCommand(command);
  writeCommand(cmd_adc_read);

  Wire.requestFrom(I2C_ADDRESS, 3);
  delay(DELAY);

  int bytesAvailable = Wire.available();
  if (bytesAvailable == 3) {
    msb = Wire.read();
    smsb = Wire.read();
    lsb = Wire.read();
  } else {
    Serial.println("error, three bytes weren't returned");
  }

  result = bitShiftCombine24(msb, smsb, lsb);

  if (command == SELECTED_OSR.D1) D1 = result;
  if (command == SELECTED_OSR.D2) D2 = result;
}

// Temperature **************************************
int32_t Barometer::temperature() {
  dT = D2 - C[5] * pow(2, 8);
  TEMP = 2000 + dT * (C[6] / pow(2, 23));
  return (TEMP);
}

// Temp Adjusted Px **************************************
int32_t Barometer::pressure() {
  int64_t T2 = 0;
  int64_t OFF2 = 0;
  int64_t SENS2 = 0;

  int64_t OFF = (int64_t)C[2] * pow(2, 16) + ((int64_t)C[4] * dT) / pow(2, 7);
  int64_t SENS = (int64_t)C[1] * pow(2, 15) + ((int64_t)C[3] * dT) / pow(2, 8);
  int32_t P = (D1 * SENS / pow(2, 21) - OFF) / pow(2, 15);

  if (TEMP < TWENTY_DEGREES_C) {
    T2 = (dT * dT) / pow(2, 31);
    OFF2 = 5 * pow((TEMP - 2000), 2) / pow(2, 1);
    SENS2 = 5 * pow((TEMP - 2000), 2) / pow(2, 2);

    if (TEMP < -1500) {
      OFF2 = OFF2 + 7 * pow(TEMP + 1500, 2);
      SENS2 = SENS2 + 11 * pow((TEMP + 1500), 2) / pow(2, 1);
    }
  }

  // Very low temperature calibration
  if (TEMP < -1500) {
    OFF2 = OFF2 + 7 * pow((TEMP + 1500), 2);
    SENS2 = SENS2 + 11 * pow((TEMP + 1500), 2) / pow(2, 1);
  }

  TEMP -= T2;
  OFF -= OFF2;
  SENS -= SENS2;

  // P = (D1 * SENS / 2097182 - OFF) / 32768;
  P = (D1 * SENS / pow(2, 21) - OFF) / pow(2, 15);
  return (P);
}

// Create 16 bit value ****************************
uint16_t Barometer::bitShiftCombine16(uint8_t msb, uint8_t lsb) {
  uint16_t combined = 0;
  combined = msb;
  combined = (msb << 8);
  combined |= lsb;

  return (combined);
}

// Create 24 bit value ****************************
uint32_t Barometer::bitShiftCombine24(uint8_t msb, uint8_t smsb, uint8_t lsb) {
  uint32_t combined = msb;
  combined = (combined << 8);
  combined |= smsb;
  combined = (combined << 8);
  combined |= lsb;

  return (combined);
}