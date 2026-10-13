// Demo di telemetria: NON pilota GPIO o semafori reali.
// Monitor seriale: 115200 baud. Righe JSON ogni secondo.
// Per integrare il programma esistente, usare i suoi stati comandati.
// feedback rimane null finché non esiste una misura hardware verificata.
#include <Arduino.h>
uint32_t sequenceNumber = 0;
String bootId;
unsigned long lastMessage = 0;
void setup() {
  Serial.begin(115200);
  bootId = String((uint32_t)esp_random(), HEX);
}
void loop() {
  if (millis() - lastMessage >= 1000) {
    lastMessage = millis();
    Serial.printf("{\"type\":\"status\",\"version\":1,\"boot_id\":\"%s\",\"seq\":%lu,\"signals\":[{\"id\":\"A\",\"command\":\"red\",\"feedback\":null},{\"id\":\"B\",\"command\":\"red\",\"feedback\":null}],\"faults\":[]}\n", bootId.c_str(), (unsigned long)++sequenceNumber);
  }
}
