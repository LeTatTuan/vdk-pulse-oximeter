#include "Wire.h"
#include "MAX30100_PulseOximeter.h"
#include "LiquidCrystal_I2C.h"

#define REPORTING_PERIOD_MS 1000

PulseOximeter pox;
uint32_t tsLastReport = 0;
float heartRate = 0.0;
float spo2 = 0.0;

#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

#define LED_PIN_1 10  // Chân digital để kích hoạt đèn 1
#define LED_PIN_2 11  // Chân digital để kích hoạt đèn 2
#define LED_PIN 5
#define THRESHOLD_MIN 0 // Ngưỡng nhịp tim
#define THRESHOLD_MAX 180

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
#define BUTTON_PIN_START 2  // Chân của nút nhấn bắt đầu đo nhịp tim
#define BUTTON_PIN_STOP 3   // Chân của nút nhấn dừng đo nhịp tim

bool isMeasuring = false;  // Biến cờ để xác định liệu đang đo nhịp tim hay không


void onBeatDetected() {
  Serial.println("Beat detected!");
}

void setup() {

  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  // Khởi tạo đèn LED
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);

  // Khởi tạo pulse sensor
  if (!pox.begin()) {
    Serial.println("FAILED");
    lcd.print("Sensor failed");
    for (;;)
      ;
  } else {
    Serial.println("SUCCESS");
    lcd.print("Sensor ready");
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);  // Thiết lập dòng điện cho đèn LED hồng ngoại

  //pox.setOnBeatDetectedCallback(onBeatDetected);  // Standard beat detection callback
  pinMode(2, INPUT_PULLUP);  // Chân input với điện trở pull-up cho nút bắt đầu đo nhịp tim
  pinMode(3, INPUT_PULLUP);  // Chân input với điện trở pull-up cho nút dừng đo nhịp tim

  attachInterrupt(digitalPinToInterrupt(2), startMeasurement, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), stopMeasurement, FALLING);
}

void loop() {
  pox.update();
  heartRate = pox.getHeartRate();
  spo2 = pox.getSpO2();

  if (isMeasuring) {
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      int analogValueA4 = analogRead(A4);  // Đọc giá trị điện áp từ chân A4
      int analogValueA5 = analogRead(A5);  // Đọc giá trị điện áp từ chân A5

      Serial.print("Analog value from A4: ");
      Serial.println(analogValueA4 * 5.0 / 1023.0);

      Serial.print("Analog value from A5: ");
      Serial.println(analogValueA5 * 5.0 / 1023.0);

      Serial.print("Heart rate: ");
      Serial.print(heartRate);

      Serial.print(" bpm / SpO2: ");
      Serial.print(spo2);
      Serial.println(" %");

      int brightness = map(heartRate, THRESHOLD_MIN, THRESHOLD_MAX, 0, 255);
      analogWrite(LED_PIN, brightness);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BPM: ");
      lcd.setCursor(6, 0);
      lcd.print(heartRate);
      lcd.setCursor(12, 0);
      lcd.print("bmp");

      lcd.setCursor(0, 1);
      lcd.print("SpO2: ");
      lcd.setCursor(6, 1);
      lcd.print(spo2);
      lcd.setCursor(12, 1);
      lcd.print("%");

      tsLastReport = millis();
    }
  }
}

void startMeasurement() {
  digitalWrite(LED_PIN_1, HIGH);  // Bật đèn 1
  digitalWrite(LED_PIN_2, LOW);   // Tắt đèn 2
  isMeasuring = true;
  Serial.println("Start: ");
}

void stopMeasurement() {
  digitalWrite(LED_PIN_1, LOW);
  digitalWrite(LED_PIN_2, HIGH);
  isMeasuring = false;
  Serial.println("Stop: ");
  if (digitalRead(LED_PIN) == HIGH) {
    analogWrite(LED_PIN, 0);
  }
}