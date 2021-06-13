#include <WiFi.h>
#include <WiFiClient.h> // 웹페이지 송출 추가 코드
#include <WiFiAP.h> // 웹페이지 송출 추가 코드
#include <DHT.h>
#include "ThingSpeak.h"
#include <ESP32_Servo.h> // 서보모터 라이브러리

#define DHTPIN 19
#define DHTTYPE DHT11

const char* ssid     = ""; // 네트워크 이름
const char* password = ""; // 비밀번호

DHT DHTsensor(DHTPIN, DHTTYPE); // DHTPIN에 연결된 DHTTYPE 센서 선언

static const int servoPin = 18; // 서보 핀 18번
int button = 0; // 0: 제습기 off, 1: 제습기 on
int automode = 1; // 0: 온습도 수동 조절 모드, 1: 온습도 자동 조절 모드

Servo myServo; // 서보모터 선언
WiFiServer server(80);
WiFiClient client;

unsigned long channelNumber = ; // ThinkSpeak Channel ID 입력
const char* writeAPIKey = ""; // ThinkSpeak writeAPIKey 입력

void setup() {
  ThingSpeak.begin(client);
  myServo.attach(servoPin); // 서보 setup부
  
  DHTsensor.begin();
  Serial.begin(115200);

  WiFi.mode(WIFI_STA); // 웹페이지 송출 추가 코드
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // 웹페이지 송출 추가 코드
  Serial.print("IP address is ");
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // 웹페이지 송출 추가 코드
  client = server.available();
  if (!client) return;
  Serial.println("New Client");
  client.setTimeout(5000);
  String request = client.readStringUntil('\r');
  Serial.println("request: ");
  Serial.println(request);
  while (client.available()) {
    client.read();
  }
  
  float humidity = DHTsensor.readHumidity();
  float temp = DHTsensor.readTemperature();

  // 웹페이지 송출 추가 코드
  if (request.indexOf("/AUTOMODE=ON") != -1) { // 온습도 자동 조절 모드
    automode = 1;
    Serial.println(automode);
  }
  else if (request.indexOf("/AUTOMODE=OFF") != -1) { // 온습도 수동 조절 모드
    automode = 0;
    Serial.println(automode);
  }
  else {
    Serial.println("Invalid request");
  }
  
  client.print("HTTP/1.1 200 OK");
  client.print("Content-Type: text/html\r\n\r\n");
  client.print("<!DOCTYPE HTML>");
  client.print("<html>");
  client.print("<head>");
  client.print("<meta charset=\"UTF-8\" http-equiv=\"refresh\" content=\"1\">");
  client.print("<title>Basic Internet of Things project</title>");
  client.print("</head>");
  client.print("<body>");
  client.print("현재 실내 온도: ");
  client.print(temp);
  client.print("℃ (± 2℃)");
  client.print("<br>");
  client.print("현재 실내 습도: ");
  client.print(humidity);
  client.print("% (± 5%)");
  client.print("<br>");
  client.print("<a href=\"/AUTOMODE=ON\"\"><button>Automode On</button></a>");
  client.print("<a href=\"/AUTOMODE=OFF\"\"><button>Automode Off</button></a>");
  client.print("</body>");
  client.print("</html>");

  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, humidity);

  int x = ThingSpeak.writeFields(channelNumber, writeAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
    Serial.print("temp : ");
    Serial.println(temp);
    Serial.print("humidity : ");
    Serial.println(humidity);
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  /*
  ** 보건복지부와 질병관리본부에서 정한 적정 실내 습도는 40~60%
  ** DHT11의 습도 측정오차는 5%이므로 습도 35% 이하, 65% 이상일 때 제습기가 켜져있지 않으면
  ** 서보모터를 가동하여 습도를 50%로 자동제습해주는 제습기 전원을 누르도록 함
  ** 반대로 제습기가 켜진 상태에서 습도 45% 초과, 55% 미만인 경우
  ** 제습기를 켤 필요가 없으므로 전원을 눌러 제습기를 끔
  ** for문의 i는 angle
  */
  int h = (int)humidity;
  if ((((h <= 35 || 65 <= h) && button == 0)
    || ((45 < h && h < 55) && button == 1)) && automode == 1){
    for(int i = 0; i < 180; i++) {
      myServo.write(i);
      delay(10);
    }
    for(int i = 180; i > 0; i--) {
      myServo.write(i);
      delay(10);
    }
    if (button == 1){
      button--;
    } else {
      button++;
    }
  }

  delay(14980);
  
  Serial.println("Client disconnected"); // 웹페이지 송출 코드
}
