// Pin tanımları
#include <QTRSensors.h>
#define NUM_SENSORS 8  
#define Rpwm1 2
#define RDir1 3
#define RDir2 4
#define Lpwm1 7
#define LDir1 5
#define LDir2 6
#define LQ1 A0
#define LQ2 A1
#define LQ3 A2
#define LQ4 A3
#define LQ5 A4
#define LQ6 A5
#define LQ7 A6
#define LQ8 A7
#define Led 13

// Değişkenler

float veri = 0; // Kullanılmayan bir değişken
float LeftBaseSpeed = 220; // Sol motorun varsayılan hızı
float RightBaseSpeed = 220; // Sağ motorun varsayılan hızı
float Kp = 0.07; // P terimi
float Kd = 2; // D terimi
float Ki = 0.0001; // I terimi

int Q[8] = {LQ8, LQ7, LQ6, LQ5, LQ4, LQ3, LQ2, LQ1}; // Çizgi sensörlerinin pinleri
int SensMin[8]; // Sensörlerin minimum değerleri
int SensMax[8]; // Sensörlerin maksimum değerleri
int SensVal[8]; // Sensör değerleri
int SensValX[8]; // Sensör değerlerinin işlenmiş hali
int FinalError = 0; // PID kontrolcüsünün son hata değeri
int Error = 0; // Hata değeri
int ExtraSpeed = 0; // Ekstra hız
int RightSpeed = 0; // Sağ motor hızı
int LeftSpeed = 0; // Sol motor hızı
int Integral = 0; // Integral terimi
int Val = 0; // Sensörlerden alınan değer
unsigned int Position = 3500; // Sensörlerin ortalaması

byte LineOk = 0; // Çizgiyi takip durumunu tutan bayt
byte WhiteLine = 1; // Beyaz çizgi olup olmadığını tutan bayt

// LED'in yanıp sönmesi için fonksiyon
void Blink() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(Led, LOW);
    delay(180);
    digitalWrite(Led, HIGH);
    delay(180);
  }
}

// Motor hızlarını ayarlamak için fonksiyon
void MotorsSpeed(int RightSpeed, int LeftSpeed) {
  // Sağ motorun yön ve hız ayarları
  if (RightSpeed <= 0) {
    RightSpeed = abs(RightSpeed);
    analogWrite(Rpwm1, RightSpeed);
    digitalWrite(RDir1, HIGH);
    digitalWrite(RDir2, LOW);
  } else {
    analogWrite(Rpwm1, RightSpeed);
    digitalWrite(RDir1, LOW);
    digitalWrite(RDir2, HIGH);
  }
  // Sol motorun yön ve hız ayarları
  if (LeftSpeed <= 0) {
    LeftSpeed = abs(LeftSpeed);
    analogWrite(Lpwm1, LeftSpeed);
    digitalWrite(LDir1, HIGH);
    digitalWrite(LDir2, LOW);
  } else {
    analogWrite(Lpwm1, LeftSpeed);
    digitalWrite(LDir1, LOW);
    digitalWrite(LDir2, HIGH);
  }
}

// Çizgi sensörlerinden gelen verileri okuyup işlemek için fonksiyon
int ReadSensor() {
  unsigned long Middle = 0; // Sensör değerlerinin ortalaması
  unsigned int Total = 0; // Sensör değerlerinin toplamı
  LineOk = 0; // Çizgi takip durumu
  // Sensörlerin okunması ve değerlerin işlenmesi
  for (int i = 0; i < 8; i++) {
    int Difference = SensMax[i] - SensMin[i]; // Sensör aralığının hesaplanması
    long int Calculation = analogRead(Q[i]) - SensMin[i]; // Sensör değerinin minimum ile farkının hesaplanması
    Calculation = Calculation * 1000; // Sensör değerinin ölçeklendirilmesi
    Calculation = Calculation / Difference; // Sensör değerinin aralığa göre orantılanması
    SensVal[i] = constrain(Calculation, 0, 1000); // Sensör değerinin sınırlanması
    SensValX[i] = SensVal[i]; // İşlenmiş sensör değeri
  }
  // Sensör değerlerinin işlenmesi ve çizgi takip durumunun belirlenmesi
  for (int i = 0; i < 8; i++) {
    if (WhiteLine == 1) SensValX[i] = 1000 - SensVal[i]; // Beyaz çizgi algılandığında değerlerin terslenmesi
    if (SensValX[i] > 250) LineOk = 1; // Çizgi algılandığında çizgi takip durumunun belirlenmesi
    if (SensValX[i] > 50) {
      Middle += SensValX[i] * i; // Sensör değerlerinin ortalaması
      Total += SensValX[i]; // Sensör değerlerinin toplamı
    }
  }
  // Çizgi algılanmadığında veya belirsiz bir durumda pozisyonun belirlenmesi
  if (LineOk == 0) {
    
    if (Val < 3000) return 200; // Sol kenara yakın
    if (Val > 4000) return 6800; // Sağ kenara yakın
    if (Val > 3000 && Val < 5000) return 3500; // Orta
    
  }
  // Sensörlerin ortalamasının hesaplanması
  Middle = Middle * 1000;
  Val = Middle / Total;
  return Val;
}


// Setup fonksiyonu - Başlangıç ayarları
void setup() {
Serial.begin(9600);
  // Pin modları ayarlanıyor
  pinMode(Rpwm1, OUTPUT);
  pinMode(Lpwm1, OUTPUT);
  pinMode(RDir1, OUTPUT);
  pinMode(RDir2, OUTPUT);
  pinMode(LDir1, OUTPUT);
  pinMode(LDir2, OUTPUT);
  pinMode(LQ1, INPUT);
  pinMode(LQ2, INPUT);
  pinMode(LQ3, INPUT);
  pinMode(LQ4, INPUT);
  pinMode(LQ5, INPUT);
  pinMode(LQ6, INPUT);
  pinMode(LQ7, INPUT);
  pinMode(LQ8, INPUT);
  pinMode(Led, OUTPUT);

  
  MotorsSpeed(0, 0); // Motorları durdur
  delay(2000); // Başlangıç için 2 saniye bekle
  
  // Sensörlerin minimum ve maksimum değerlerinin başlangıç ayarları
  for (int i = 0; i < 8; i++) {
    SensMin[i] = 1024;
    SensMax[i] = 0;
  }
  
  // Sensörlerin kalibrasyonu
  for (int i = 0; i < 150; i++) {
    for (int i = 0; i < 8; i++) {
      if (SensMin[i] > analogRead(Q[i])) SensMin[i] = analogRead(Q[i]);
      if (SensMax[i] < analogRead(Q[i])) SensMax[i] = analogRead(Q[i]);
      delay(1);
    }
  }
  delay(500); // 500 milisaniye bekle
  Blink(); // LED'i yanıp söndür
}

// Ana döngü
void loop() {

  Serial.println(Val);

 // Sensörlerden pozisyon oku
  Position = ReadSensor();
  Error = Position - 3500; // Hata hesapla
  Integral += Error; // Integral terimini güncelle
  
  // PID kontrolcüsünden kontrol sinyali al
  int Verify = Kp * Error + Kd * (Error - FinalError) + Ki * Integral;
  FinalError = Error; // Son hata değerini güncelle
  
  // Sağ ve sol motor hızlarını ayarla
  RightSpeed = RightBaseSpeed + Verify + ExtraSpeed ;
  LeftSpeed = LeftBaseSpeed - Verify +  ExtraSpeed ;
  
  // Motor hızlarını sınırla
  RightSpeed = constrain(RightSpeed, -175, 175);
  LeftSpeed = constrain(LeftSpeed, -175,175);
  
  // Motor hızlarını ayarla
  MotorsSpeed(LeftSpeed, RightSpeed);

  // Beyaz zemin algılandığında motor hızını sıfırla
  if (isWhiteSurface()) {
    MotorsSpeed(0, 0);
  }
  
}

// Beyaz zemin algılandığını kontrol et
bool isWhiteSurface() {
  int whiteThreshold = 820; // Beyaz zemin eşik değeri
  boolean isWhite = true; // Beyaz zemin algılandı mı?

  // Tüm sensörleri kontrol et
  for (int i = 0; i < 8; i++) {
    // Eğer herhangi bir sensör belirlenen eşik değerinden küçük okuyorsa, beyaz zemin algılanmıştır
    if (analogRead(Q[i]) < whiteThreshold) {
      isWhite = false;
      
      break;
    }
    
  }

  return isWhite;
}
