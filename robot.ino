#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define trigPin1 9
#define echoPin1 10
#define trigPin2 11
#define echoPin2 12
#define trigPin3 7
#define echoPin3 8

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

void setup() {
  Serial.begin(9600);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  if (tcs.begin()) {
    Serial.println("Sensor detected.");
  } else {
    Serial.println("Sensor not detected ...Please check your connections.");
    while (1);
  }
}

void loop() {
  uint16_t r, g, b, c;
  long duration1, distance1, duration2, distance2, duration3, distance3;

  tcs.getRawData(&r, &g, &b, &c);

  // Cap the RGB values between 0 and 255
  /*r = constrain(map(r, 0, 65535, 0, 1000), 0, 1000);
  g = constrain(map(g, 0, 65535, 0, 1000), 0, 1000);
  b = constrain(map(b, 0, 65535, 0, 1000), 0, 1000);*/

  Serial.print("Red: "); Serial.print(r); Serial.print(" ");
  Serial.print("Green: "); Serial.print(g); Serial.print(" ");
  Serial.print("Blue: "); Serial.println(b);

  if(r > g + b) {
    Serial.print("RED")
  } else if ((g < r)&&(b < r)) {
    Serial.print("BLUE")
  } else if ((b > r))&&(b > g)) {
    Serial.print("YELLOW")
  }

  digitalWrite(trigPin1, LOW);
  digitalWrite(trigPin2, LOW);
  digitalWrite(trigPin3, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  digitalWrite(trigPin2, HIGH);
  digitalWrite(trigPin3, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  digitalWrite(trigPin2, LOW);
  digitalWrite(trigPin3, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  duration2 = pulseIn(echoPin2, HIGH);
  duration3 = pulseIn(echoPin3, HIGH);
  distance1 = (duration1 * 0.0343) / 2; // Calculate distance in cm
  distance2 = (duration2 * 0.0343) / 2; // Calculate distance in cm
  distance3 = (duration3 * 0.0343) / 2; // Calculate distance in cm

  // Check if any of the sensors detect within 30 cm
  if (distance1 < 20 && distance2 < 20) {
    Serial.println("Wall detected on left & right!");
  }

  if (distance3 < 30) {
    Serial.println("STOP");
  }

  delay(500);
}
