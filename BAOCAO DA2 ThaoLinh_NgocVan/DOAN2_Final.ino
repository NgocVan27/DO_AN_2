// Cấu hình Blynk
#define BLYNK_TEMPLATE_ID "TMPL6wqLOMGra"
#define BLYNK_TEMPLATE_NAME "HE THONG"
#define BLYNK_AUTH_TOKEN "vpXWi3QxI4CFewAnNOFIiek90iu19MhY"
#define BLYNK_PRINT Serial



// Khai báo thư viện
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// Cấu hình kết nối WiFi
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Lila";
char pass[] = "thaolinh";

// Khai báo chân DHT11
#define DHTPIN 13
#define DHTTYPE DHT11

#define SOUND_SPEED 0.034 // Tốc độ âm thanh: 0.034 cm/µs
#define TANK_HEIGHT 15.0  // Chiều cao bồn chứa đầy nước (cm)

// Khai báo chân cảm biến HC-SR04
const int trigPin = 14;
const int echoPin = 27;

// Khai báo chân cảm biến độ ẩm đất
const int sensorPin = 32;

// Khai báo chân RELAY và LED
const int relayPin = 19;
const int ledPin = 2; // LED báo trạng thái bơm

// Khai báo nút nhấn MODE, UP, DOWN
const int buttonMode = 5; // Chân nút nhấn (chọn GPIO bất kỳ)
const int buttonUp = 18; 
const int buttonDown = 15; 

int moistureThreshold = 50;
float humidity = 0, prevHumidity = -1;
float temperature = 0, prevTemperature = -1;
float moisture = 0, prevMoisture = -1;
long duration;
float distanceCm;
float waterLevelPercent;
int pumpMode = 0; 
int pumpState = LOW;
int lcdMode = 0; 

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C Lcd(0x27, 20, 4); // Cấu hình LCD
BlynkTimer timer;

void checkwaterLevel(){
  // Kiểm tra mực nước và hiển thị cảnh báo
  if (waterLevelPercent < 20) {
    Lcd.clear(); // Xóa màn hình để hiển thị cảnh báo
    Lcd.setCursor(2, 0);
    Lcd.print("!!! CANH BAO !!!");
    Lcd.setCursor(1, 1);
    Lcd.print("Muc nuoc qua thap");
    Lcd.setCursor(1, 2);
    Lcd.print("Nap them nuoc ngay!");
    delay(5000); // Hiển thị thông báo trong 5 giây
    Lcd.clear();
    printSensorData(); // Quay lại hiển thị dữ liệu cảm biến
    delay(5000); 
  }
}

void checkRelay() {
  if (pumpMode == 0) { // Chế độ tự động
    if (waterLevelPercent < 20) {
      pumpState = LOW;
      Lcd.clear();
      Lcd.setCursor(0, 0);
      Lcd.print("Muc nuoc thap!");
      Lcd.setCursor(0, 1);
      Lcd.print("Khong the bom!");
      Serial.println("Mực nước quá thấp, máy bơm tắt.");
    } else if (moisture > moistureThreshold) {
      pumpState = LOW;
      Serial.println("Độ ẩm đất cao hơn ngưỡng, máy bơm tắt.");
    } else {
      pumpState = HIGH;
      Serial.println("Máy bơm bật.");
    }
    digitalWrite(relayPin, pumpState);
    digitalWrite(ledPin, pumpState);
  }
}


void leverwater() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Nhận phản hồi Echo
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;

  // Tính phần trăm mực nước
  waterLevelPercent = (1.0 - (distanceCm / TANK_HEIGHT)) * 100.0;
  if (waterLevelPercent > 100) waterLevelPercent = 100;
  else if (waterLevelPercent < 0) waterLevelPercent = 0;
  checkRelay();

  // Hiển thị mực nước trên Serial Monitor
  Serial.print("Khoảng cách: ");
  Serial.print(distanceCm);
  Serial.println(" cm");
  Serial.print("Mực nước: ");
  Serial.print(waterLevelPercent);
  Serial.println(" %");
  checkwaterLevel();
}

void printSensorData() {
  if (lcdMode == 0) {
    // Hiển thị dữ liệu cảm biến
    Lcd.setCursor(0, 0);
    Lcd.print("Nhiet do: ");
    Lcd.print(temperature);
    Lcd.print(" C");

    Lcd.setCursor(0, 1);
    Lcd.print("Do am: ");
    Lcd.print(humidity);
    Lcd.print(" %");

    Lcd.setCursor(0, 2);
    Lcd.print("Do am dat: ");
    Lcd.print(moisture);
    Lcd.print(" %");

    Lcd.setCursor(0, 3);
    Lcd.print("Muc nuoc: ");
    Lcd.print(waterLevelPercent);
    Lcd.print(" % ");
  } else if (lcdMode == 1) {
    // Hiển thị ngưỡng đã đặt
    Lcd.setCursor(0, 0);
    Lcd.print("NGUONG DA DAT:");

    Lcd.setCursor(0, 1);
    Lcd.print("Do am dat: ");
    Lcd.print(moistureThreshold);
    Lcd.print(" %");

    Lcd.setCursor(0, 2);
    Lcd.print("Muc Chuan: ");
    Lcd.print(TANK_HEIGHT);
    Lcd.print(" cm");

    Lcd.setCursor(0, 3);
    Lcd.print("Che Do: ");
    Lcd.print((pumpMode == 0) ? "Tu  Dong" : "Tuy Chinh");
  } else if (lcdMode == 2) {
    // Hiển thị ngưỡng đã đặt
    Lcd.setCursor(0, 0);
    Lcd.print("NGUONG DA DAT:");

    Lcd.setCursor(0, 1);
    Lcd.print("Do am dat: ");
    Lcd.print(moistureThreshold);
    Lcd.print(" %");

    Lcd.setCursor(0, 2);
    Lcd.print("Muc Chuan: ");
    Lcd.print(TANK_HEIGHT);
    Lcd.print(" cm");

    Lcd.setCursor(0, 3);
    Lcd.print("Che Do: ");
    Lcd.print((pumpMode == 1) ? "Tu Dong" : "Tuy Chinh");
  } else if (lcdMode == 3) {
    // Hiển thị ngưỡng đã đặt
    Lcd.setCursor(0, 0);
    Lcd.print("NGUONG DA DAT:");

    Lcd.setCursor(0, 1);
    Lcd.print("Do am dat: ");
    Lcd.print(moistureThreshold);
    Lcd.print(" %");

    Lcd.setCursor(0, 2);
    Lcd.print("Muc Chuan: ");
    Lcd.print(TANK_HEIGHT);
    Lcd.print(" cm");

    Lcd.setCursor(0, 3);
    Lcd.print("Che Do: ");
    Lcd.print((pumpMode == 0) ? "Tu Dong  " : "Tuy Chinh");
  }  
}


void readAndSendSensorData() {
  // Đọc giá trị nhiệt độ và độ ẩm
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (!isnan(humidity) && !isnan(temperature)) {
    Serial.print("Nhiệt độ: ");
    Serial.println(temperature);
    Serial.print("Độ ẩm: ");
    Serial.println(humidity);
  } else {
    Serial.println(F("Failed to read from DHT sensor!"));
  }

  // Đọc giá trị độ ẩm đất
  int sensorValue = analogRead(sensorPin);
  moisture = map(sensorValue, 0, 4095, 0, 100);
  Serial.print("Giá trị cảm biến:  ");
  Serial.println(sensorValue);
  Serial.print("Độ ẩm đất:  ");
  Serial.println(moisture);
  leverwater();

  // Gửi dữ liệu lên Blynk
  Blynk.virtualWrite(V0, temperature);
  Blynk.virtualWrite(V4, humidity);
  Blynk.virtualWrite(V3, moisture);
  Blynk.virtualWrite(V1, waterLevelPercent);
  Blynk.virtualWrite(V2, pumpState);

  // Hiển thị dữ liệu lên LCD
  printSensorData();
}




void updateMoistureThresholdDisplay() {
  if (lcdMode == 2) {
    // Hiển thị ngưỡng độ ẩm đất
    Lcd.setCursor(0, 1);
    Lcd.print("Do am dat: ");
    Lcd.print(moistureThreshold);
    Lcd.print(" %    "); // Thêm khoảng trắng để xóa giá trị cũ
  }
}


void checkButton() {
  static int lastButtonState = HIGH, lastIncreaseState = HIGH, lastDecreaseState = HIGH;
  //Sử dụng các biến lastModeDebounceTime, lastIncreaseDebounceTime, và lastDecreaseDebounceTime riêng biệt để tránh xung đột giữa các nút khi nhấn nhanh.
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;
  //debounceDelay (50ms) được áp dụng cho mỗi nút để đảm bảo việc nhấn nút được nhận diện chính xác mà không bị lặp do tín hiệu nhiễu.
  // Đọc trạng thái nút MODE
  int currentButtonState = digitalRead(buttonMode);
  if (currentButtonState != lastButtonState && millis() - lastDebounceTime > debounceDelay) {
    if (currentButtonState == LOW) {
      lcdMode = (lcdMode + 1) % 4; // Chuyển đổi giữa 0, 1, 2, 3
      if (lcdMode != 2 && lcdMode != 3) {
        Lcd.clear(); // Xóa màn hình khi chuyển chế độ không phải là "Tùy chỉnh ngưỡng"
      }
    }
    lastDebounceTime = millis();
  }
  lastButtonState = currentButtonState;

  // Đọc trạng thái nút Tăng
  int currentIncreaseState = digitalRead(buttonUp);
  if (currentIncreaseState != lastIncreaseState && millis() - lastDebounceTime > debounceDelay) {
    if (currentIncreaseState == LOW && lcdMode == 2) {
      moistureThreshold += 5; // Tăng 5%
      if (moistureThreshold > 100) moistureThreshold = 100;

      // Cập nhật giá trị trực tiếp trên màn hình
      // Lcd.clear(); // Xóa màn hình trước khi cập nhật
      updateMoistureThresholdDisplay();
      checkRelay();
    }
    lastDebounceTime = millis();
  }
  lastIncreaseState = currentIncreaseState;

  // Đọc trạng thái nút Giảm
  int currentDecreaseState = digitalRead(buttonDown);
  if (currentDecreaseState != lastDecreaseState && millis() - lastDebounceTime > debounceDelay) {
    if (currentDecreaseState == LOW && lcdMode == 2) {
      moistureThreshold -= 5; // Giảm 5%
      if (moistureThreshold < 0) moistureThreshold = 0;

      // Cập nhật giá trị trực tiếp trên màn hình
      // Lcd.clear(); // Xóa màn hình trước khi cập nhật
      updateMoistureThresholdDisplay();
      checkRelay();
    }
    lastDebounceTime = millis();
  }
  lastDecreaseState = currentDecreaseState;

}


void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonMode, INPUT_PULLUP); // Kết nối nút nhấn với GPIO và sử dụng PULLUP
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);

  dht.begin();
  Lcd.init();
  Lcd.backlight();

  // Hiển thị và báo kết nối WiFi
  Lcd.setCursor(0, 0);
  Lcd.print("Ket Noi WIFI...");
  Blynk.begin(auth, ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Lcd.setCursor(0, 1);
    Lcd.print(".");
  }
  Lcd.clear();
  // Hiển thị ngưỡng ban đầu
  updateMoistureThresholdDisplay();
  // Đặt timer
  timer.setInterval(2000L, readAndSendSensorData);
  timer.setInterval(2000L, leverwater);
  timer.setInterval(5000L, checkRelay);
  timer.setInterval(100L, checkButton); // Kiểm tra nút nhấn liên tục
}

void loop() {
  Blynk.run();
  timer.run();
}
