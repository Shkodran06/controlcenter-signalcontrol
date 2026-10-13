// Prova su mini LED da banco: verde -> giallo -> rosso, 3 secondi ciascuno.
// Rosso GPIO25, giallo GPIO26, verde GPIO27; resistenze 220 ohm e GND.
// feedback null: nessun sensore di riscontro installato.
#include <Arduino.h>
constexpr uint8_t RED_PIN=25, YELLOW_PIN=26, GREEN_PIN=27;
constexpr uint32_t PHASE_MS=3000;
uint32_t sequenceNumber=0, lastMessage=0, lastPhase=0;
uint8_t phase=0;
String bootId;
const char* colors[]={"green","yellow","red"};
void applyPhase() {
  digitalWrite(RED_PIN, LOW); digitalWrite(YELLOW_PIN, LOW); digitalWrite(GREEN_PIN, LOW);
  digitalWrite(phase==0?GREEN_PIN:phase==1?YELLOW_PIN:RED_PIN,HIGH);
}
void sendStatus() {
  Serial.printf("{\"type\":\"status\",\"version\":1,\"boot_id\":\"%s\",\"seq\":%lu,\"signals\":[{\"id\":\"A\",\"command\":\"%s\",\"feedback\":null},{\"id\":\"B\",\"command\":\"off\",\"feedback\":null}],\"faults\":[]}\n",bootId.c_str(),(unsigned long)++sequenceNumber,colors[phase]);
}
void setup() {
  Serial.begin(115200); bootId=String((uint32_t)esp_random(),HEX);
  pinMode(RED_PIN,OUTPUT); pinMode(YELLOW_PIN,OUTPUT); pinMode(GREEN_PIN,OUTPUT);
  applyPhase(); lastPhase=millis(); lastMessage=millis(); sendStatus();
}
void loop() {
  const uint32_t now=millis(); bool changed=false;
  if(now-lastPhase>=PHASE_MS){phase=(phase+1)%3;lastPhase=now;applyPhase();changed=true;}
  if(changed||now-lastMessage>=500){lastMessage=now;sendStatus();}
}
