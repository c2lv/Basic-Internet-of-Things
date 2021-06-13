#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   

const int PulseWire = 0;       // PulseSensor Pulse WIRE(S)를 ANALOG PIN 0번에 연결
const int LED13 = 13;          // Arduino 내장 LED
int Threshold = 550;           // 임계값, 사람에 따라 조정 가능 
                               
PulseSensorPlayground pulseSensor;

void setup() {   
  Serial.begin(9600);
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13); // 심박수에 따라 LED 깜빡임 속도 달라짐
  pulseSensor.setThreshold(Threshold); // 임계값 조정
  if (pulseSensor.begin()) {
    Serial.println("리셋 또는 시작");
  }
}

void loop() {
 int myBPM = pulseSensor.getBeatsPerMinute();  // 해당 함수는 int 자료형으로 심박수 리턴
 if (pulseSensor.sawStartOfBeat()) {
   Serial.print("BPM: ");
   Serial.println(myBPM);
 }
 delay(20);
}

  
