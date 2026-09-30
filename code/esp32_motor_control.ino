#include <WiFi.h>

// --- Wi-Fi Credentials ---
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_PASSWORD";

// --- Motor Pins (ESP32 S3 CAM Default) ---
const int ENA = 1;
const int IN1 = 2;
const int IN2 = 3;
const int IN3 = 14;
const int IN4 = 41;
const int ENB = 42;

// PWM properties for ESP32 Core v3.x
const int freq = 30000;
const int resolution = 8; // 0 - 255

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Configure Motor Control Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configure PWM
  ledcAttach(ENA, freq, resolution);
  ledcAttach(ENB, freq, resolution);

  // Connect to Wi-Fi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    String requestString = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        requestString += c;
        
        // Stop reading headers on newline
        if (c == '\n') {
          // Parse GET request: /drive?dir=X&speed=Y
          if (requestString.indexOf("GET /drive?") != -1) {
            int dirIndex = requestString.indexOf("dir=");
            int speedIndex = requestString.indexOf("&speed=");
            int spaceIndex = requestString.indexOf(" HTTP/");

            if (dirIndex != -1 && speedIndex != -1 && spaceIndex != -1) {
              String direction = requestString.substring(dirIndex + 4, speedIndex);
              int speedVal = requestString.substring(speedIndex + 7, spaceIndex).toInt();

              // Map speed (0-100%) to PWM (0-255)
              int pwmSpeed = map(speedVal, 0, 100, 0, 255);

              controlMotors(direction, pwmSpeed);
            }
          }

          // Send HTTP response
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          client.stop();
          break;
        }
      }
    }
  }
}

// Fixed function matching App Inventor single-letter commands
void controlMotors(String dir, int spd) {
  if (dir == "F" || dir == "FORWARD") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    ledcWrite(ENA, spd);
    ledcWrite(ENB, spd);
  } 
  else if (dir == "B" || dir == "BACKWARD") {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    ledcWrite(ENA, spd);
    ledcWrite(ENB, spd);
  } 
  else if (dir == "L" || dir == "LEFT") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    ledcWrite(ENA, spd / 2); 
    ledcWrite(ENB, spd);
  } 
  else if (dir == "R" || dir == "RIGHT") {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    ledcWrite(ENA, spd);
    ledcWrite(ENB, spd / 2);
  } 
  else { // "S" or STOP
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    ledcWrite(ENA, 0);
    ledcWrite(ENB, 0);
  }
}
