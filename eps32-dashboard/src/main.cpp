
#include <Adafruit_Sensor.h>
#include <Barometer.h>

#include "BluetoothSerial.h"
#include "DHT.h"

/* Check if Bluetooth configurations are enabled in the SDK */
/* If not, then you have to recompile the SDK */
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
boolean getDhtValues();

#define LED_BUILTIN 1
#define DHT_PIN 4
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);
byte BTData;

uint8_t buffer[100];
char charBuffer[100];

Barometer barometer = Barometer();
void print(result r);
void printh(String s) { Serial.print(s); }
boolean getDhtValues();

struct a {
  float humidity;
  float temp;
  float heatIndex;
};

a getDhtsValues();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  dht.begin();
  SerialBT.begin();

  Serial.begin(115200);
  barometer.begin();
}

void blink() {
  digitalWrite(LED_BUILTIN, LOW);
  delay(20);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // result r = barometer.getResult();
  // Serial.println(barometer.toString());

  if (SerialBT.available()) {
    BTData = SerialBT.read();
    if (BTData == '1') {
      // Serial.print(BTData);
      // if (BTData == '1') {
      // getDhtValues();
      a dhtVals = getDhtsValues();
      result r = barometer.getResult();

      sprintf(charBuffer, "%.2f,%.2f,%.2f,%.2f,%.2f\0", dhtVals.humidity,
              dhtVals.temp, dhtVals.heatIndex, r.pressure, r.temperature);

      // int numChars = strlen(charBuffer);
      // for (int i = 0; i < numChars; i++) {
      //   SerialBT.write(charBuffer[i]);
      //   Serial.print(charBuffer[i]);
      // }
      Serial.println(charBuffer);
      SerialBT.println(charBuffer);
      blink();
    }

    // sleep(500);
    // }
  }
}

// void loop() {
//   // if (Serial.available()) {
//   // BTData = Serial.read();
//   //   SerialBT.write(BTData);
//   //   if (BTData == '1') {
//   //     printValues();
//   //   } else {
//   //     Serial.write(BTData);
//   //   }
//   // }

//   delay(20);
// }

boolean getDhtValues() {
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // fahrenheit
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return false;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  sprintf(charBuffer,
          "\nHumidity: %.2f%%\nTemperature: %.2f°C, %.2f°F\nHeat index: "
          "%.2f°C, %.2f°F\n",
          h, t, f, hic, hif);

  return true;
}

a getDhtsValues() {
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // fahrenheit
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    return {-99, -99, -99};
  }

  float hic = dht.computeHeatIndex(t, h, false);

  return {h, t, hic};
}
