#include <Servo.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <SoftwareSerial.h>

// Pin Definitions
const int flameSensorPins[5] = {A0, A1, A2, A3, A4};
const int gasSensorPin = A5;
const int buzzerPin = 8;
const int servoPin = 5;
const int dcMotorRelayPin = 12;
const int ledRelayPin = 13;

// 4G Module Pins (Software Serial)
const int A7670_RX_PIN = 2;
const int A7670_TX_PIN = 3;

// Emergency Contact Number (REPLACE WITH ACTUAL NUMBER)
const String EMERGENCY_NUMBER = "+1234567890"; //Replace "+1234567890" with the required number.

// Servo and Fire Detection Variables
Servo myServo;
int servoPosition = 0;
bool servoDirection = true;
bool fireDetected = false;

// Timing Variables
unsigned long previousMillis = 0;
const long blinkInterval = 200;  // LED blink interval in milliseconds
bool ledState = false;
unsigned long previousServoMillis = 0;
const long servoInterval = 50;  // Delay between servo movements

// 4G Module Serial
SoftwareSerial a7670Serial(A7670_RX_PIN, A7670_TX_PIN);

// Color Sensor
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Color detection threshold
const uint16_t RED_THRESHOLD = 500;  // Adjust based on your specific requirements

void setup() {
  // Flame Sensors Setup
  for (int i = 0; i < 5; i++) {
    pinMode(flameSensorPins[i], INPUT_PULLUP);
  }
 
  // Pin Modes
  pinMode(gasSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(dcMotorRelayPin, OUTPUT);
  pinMode(ledRelayPin, OUTPUT);
 
  // Initial State
  digitalWrite(dcMotorRelayPin, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledRelayPin, LOW);
 
  // Serial Communication
  Serial.begin(9600);
  a7670Serial.begin(9600);
 
  // Servo Setup
  myServo.attach(servoPin);
 
  // Color Sensor Setup
  if (tcs.begin()) {
    Serial.println("Color Sensor Initialized");
  } else {
    Serial.println("Color Sensor Not Found");
  }
 
  // 4G Module Initial Setup
  setupA7670Module();
}

void setupA7670Module() {
  // Basic 4G module initialization
  a7670Serial.println("AT");  // Test AT command
  delay(1000);
  a7670Serial.println("AT+CREG?");  // Check network registration
  delay(1000);
  a7670Serial.println("AT+CGATT=1");  // Attach to GPRS service
  delay(1000);
 
  // Enable audio functionality
  a7670Serial.println("AT+CMIC=0,10");  // Set microphone gain
  delay(500);
  a7670Serial.println("AT+CLVL=5");     // Set loudspeaker volume
  delay(500);
}

bool checkColorSensor() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
 
  // Simple color detection (focusing on red intensity)
  if (r > RED_THRESHOLD) {
    Serial.println("Intense Red Detected!");
    return true;
  }
 
  return false;
}

void sendSMSAlert() {
  // Send SMS alert via 4G module
  a7670Serial.println("AT+CMGF=1");  // Set SMS mode to text
  delay(500);
  a7670Serial.print("AT+CMGS=\"");
  a7670Serial.print(EMERGENCY_NUMBER);
  a7670Serial.println("\"");
  delay(500);
  a7670Serial.println("EMERGENCY: Fire Detected!");
  a7670Serial.write(26);  // Send SMS
  delay(1000);
}

void makeEmergencyCall() {
  // Dial the emergency number
  a7670Serial.print("ATD");
  a7670Serial.print(EMERGENCY_NUMBER);
  a7670Serial.println(";");
  delay(1000);
 
  // Optional: Wait for call connection and play an audio alert
  // Note: Actual implementation depends on module capabilities
  Serial.println("Emergency Call Initiated");
}

void activateEmergencyProtocol() {
  // Simultaneous SMS and Call
  sendSMSAlert();
  makeEmergencyCall();
}

void loop() {
  unsigned long currentMillis = millis();
 
  // Check for fire detection from flame sensors
  bool currentFireDetected = false;
  for (int i = 0; i < 5; i++) {
    int sensorValue = digitalRead(flameSensorPins[i]);
    if (sensorValue == LOW) {
      currentFireDetected = true;
      break;
    }
  }
 
  // Additional color sensor check
  if (checkColorSensor()) {
    currentFireDetected = true;
  }
 
  if (currentFireDetected) {
    // Fire detected actions
    fireDetected = true;
    digitalWrite(buzzerPin, HIGH);    // Activate buzzer
    digitalWrite(dcMotorRelayPin, HIGH);  // Activate relay
    myServo.detach();  // Stop servo
   
    // Activate full emergency protocol
    activateEmergencyProtocol();
  } else {
    // No fire detected
    fireDetected = false;
    digitalWrite(buzzerPin, LOW);
    digitalWrite(dcMotorRelayPin, LOW);
   
    // Reattach servo if it was detached
    if (!myServo.attached()) {
      myServo.attach(servoPin);
      servoPosition = 0;
      servoDirection = true;
    }
  }
 
  // LED Blinking Logic
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(ledRelayPin, ledState ? HIGH : LOW);
  }
 
  // Servo control
  if (!fireDetected && myServo.attached()) {
    if (currentMillis - previousServoMillis >= servoInterval) {
      previousServoMillis = currentMillis;
     
      if (servoDirection) {
        servoPosition++;
        myServo.write(servoPosition);
       
        if (servoPosition >= 180) {
          servoDirection = false;
        }
      } else {
        servoPosition--;
        myServo.write(servoPosition);
       
        if (servoPosition <= 0) {
          servoDirection = true;
        }
      }
    }
  }
}
