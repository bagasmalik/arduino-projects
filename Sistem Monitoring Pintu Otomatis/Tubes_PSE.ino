#include <WiFi.h> // Persiapkan koneksi WiFi
#include <ESPAsyncWebServer.h> // Persiapkan pustaka ESPAsyncWebServer untuk menghandle fungsi server web
#include <SPIFFS.h> // Persiapkan pustaka SPIFFS untuk menghandle sistem file pada ESP32
#include <NewPing.h> //library ultrasonik
#include <Servo.h> //library servo

#define ULTRASONIC_TRIGGER_PIN 4 // Pin 4 terhubung ke trigger ultrasonik 
#define ULTRASONIC_ECHO_PIN 5 // Pin 5 terhubung ke echo ultrasonik 
#define LDR_PIN 34 // Pin 34 terhubung ke LDR
#define SERVO_PIN 2 // Pin 2 terhubung ke motor servo
#define LED_PIN1 12 // Pin 12 terhubung ke LED kesatu
#define LED_PIN2 13 // Pin 13 terhubung ke LED kedua

NewPing ultrasonic(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN); // Inisialisasi objek NewPing untuk sensor ultrasonik
Servo servo; // Inisialisasi objek Servo untuk servomotor

const char* ssid = "malikbagas"; // SSID (nama jaringan WiFi)
const char* password = "123456789"; // Password WiFi

AsyncWebServer server(80); // Inisialisasi objek AsyncWebServer untuk server web

void handleRootRequest(AsyncWebServerRequest *request) {
  File file = SPIFFS.open("/index.html", "r"); // Membuka berkas "index.html" dari SPIFFS
  if (file) {
    String htmlContent = file.readString(); // Membaca konten berkas menjadi string
    file.close();
    request->send(200, "text/html", htmlContent); // Mengirimkan konten berkas sebagai respons dengan tipe konten "text/html"
  } else {
    request->send(404); // Mengirimkan respons kode status 404 jika berkas tidak ditemukan
  }
}

void handleLDRRequest(AsyncWebServerRequest *request) {
  int lightIntensity = analogRead(LDR_PIN); // Membaca intensitas cahaya dari sensor LDR
  request->send(200, "text/plain", String(lightIntensity)); // Mengirimkan intensitas cahaya sebagai respons dengan tipe konten "text/plain"
}

void handleUltrasonicRequest(AsyncWebServerRequest *request) {
  int distance = ultrasonic.ping_cm(); // Mengukur jarak dengan sensor ultrasonik
  request->send(200, "text/plain", String(distance)); // Mengirimkan jarak sebagai respons dengan tipe konten "text/plain"
}

void handleLEDStatusRequest(AsyncWebServerRequest *request) {
  bool led1Status = digitalRead(LED_PIN1); // Membaca status LED_PIN1
  bool led2Status = digitalRead(LED_PIN2); // Membaca status LED_PIN2

  String led1StatusText = led1Status ? "ON" : "OFF"; // Mengubah status LED_PIN1 menjadi teks "ON" atau "OFF"
  String led2StatusText = led2Status ? "ON" : "OFF"; // Mengubah status LED_PIN2 menjadi teks "ON" atau "OFF"

  String response = "{\"led1\":\"" + led1StatusText + "\",\"led2\":\"" + led2StatusText + "\"}"; // Membuat respons dalam format JSON
  request->send(200, "application/json", response); // Mengirimkan respons JSON
}

void handleServoStatusRequest(AsyncWebServerRequest *request) {
  int servoAngle = servo.read(); // Membaca sudut servomotor
  String servoStatus = "";

  if (servoAngle == 0) {
    servoStatus = "Kondisi Pintu Tertutup (Angle: 0 degrees)"; // Menampilkan teks ke web server
  } else if (servoAngle == 90) {
    servoStatus = "Kondisi Pintu Terbuka (Angle: 90 degrees)"; // Menampilkan teks ke web server
  } else if (servoAngle == 180) {
    servoStatus = "Angle: 180 degrees"; // Menampilkan teks ke web server
  } else {
    servoStatus = "Unknown"; // Menampilkan teks ke web server
  }

  String response = "{\"servo\":\"" + servoStatus + "\"}"; // Membuat respons dalam format JSON
  request->send(200, "application/json", response); // Mengirimkan respons JSON
}

void setup() {
  pinMode(LED_PIN1, OUTPUT); // Mengatur pin LED_PIN1 sebagai OUTPUT
  pinMode(LED_PIN2, OUTPUT); // Mengatur pin LED_PIN2 sebagai OUTPUT

  servo.attach(SERVO_PIN); // Menghubungkan servomotor ke pin SERVO_PIN

  servo.write(0); // Menggerakkan servomotor ke posisi 0 derajat
  delay(2000); //Penundaan selama 2000 milidetik (2 detik)
  servo.write(90); // Menggerakkan servomotor ke posisi 90 derajat
  delay(2000); //Penundaan selama 2000 milidetik (2 detik)
  servo.write(180); // Menggerakkan servomotor ke posisi 180 derajat
  delay(2000); //Penundaan selama 2000 milidetik (2 detik)

  Serial.begin(115200); // Memulai komunikasi serial dengan baud rate 115200

  if (!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS"); // Menampilkan pesan kesalahan jika SPIFFS tidak dapat dimuat
    return;
  }

  WiFi.begin(ssid, password); // Menghubungkan ESP32 ke jaringan WiFi
  while (WiFi.status() != WL_CONNECTED) { // Menunggu koneksi WiFi terhubung
    delay(1000); //Penundaan selama 1000 milidetik (1 detik)
    Serial.println("Connecting to WiFi..."); // Menampilkan teks Connecting to WiFi... ke serial monitor
  }
  Serial.println("Connected to WiFi"); // Menampilkan teks Connected to WiFi ke serial monitor

  server.on("/", HTTP_GET, handleRootRequest); // Menangani permintaan ke akar ("/") server web dengan fungsi handleRootRequest
  server.on("/ldr", HTTP_GET, handleLDRRequest); // Menangani permintaan ke endpoint "/ldr" dengan fungsi handleLDRRequest
  server.on("/ultrasonic", HTTP_GET, handleUltrasonicRequest); // Menangani permintaan ke endpoint "/ultrasonic" dengan fungsi handleUltrasonicRequest
  server.on("/led-status", HTTP_GET, handleLEDStatusRequest); // Menangani permintaan ke endpoint "/led-status" dengan fungsi handleLEDStatusRequest
  server.on("/servo-status", HTTP_GET, handleServoStatusRequest); // Menangani permintaan ke endpoint "/servo-status" dengan fungsi handleServoStatusRequest

  server.begin(); // Memulai server web
  Serial.println("HTTP server started"); // Menampilkan teks HTTP server started ke serial monitor

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Menampilkan alamat IP ESP32 ke serial monitor
}

void loop() {
  int distance = ultrasonic.ping_cm(); // Mengukur jarak dengan sensor ultrasonik
  int lightIntensity = analogRead(LDR_PIN); // Membaca intensitas cahaya dari sensor LDR
  bool led1Status = digitalRead(LED_PIN1); // Membaca status LED_PIN1
  bool led2Status = digitalRead(LED_PIN2); // Membaca status LED_PIN2

  if (lightIntensity <= 500 && distance < 20) {
    digitalWrite(LED_PIN1, HIGH); // Menghidupkan LED_PIN1
    digitalWrite(LED_PIN2, HIGH); // Menghidupkan LED_PIN2
    servo.write(90); // Menggerakkan servomotor ke posisi 90 derajat
  } else if (lightIntensity >= 500 && distance < 20) {
    digitalWrite(LED_PIN1, HIGH); // Menghidupkan LED_PIN1
    digitalWrite(LED_PIN2, LOW); // Mematikan LED_PIN2
    servo.write(90); // Menggerakkan servomotor ke posisi 90 derajat
  } else if (lightIntensity <= 500 && distance > 20) {
    digitalWrite(LED_PIN1, HIGH); // Menghidupkan LED_PIN1
    digitalWrite(LED_PIN2, HIGH); // Menghidupkan LED_PIN2
    servo.write(0); // Menggerakkan servomotor ke posisi 0 derajat
  } else {
    digitalWrite(LED_PIN1, LOW); // Mematikan LED_PIN1
    digitalWrite(LED_PIN2, LOW); // Mematikan LED_PIN2
    servo.write(0); // Menggerakkan servomotor ke posisi 0 derajat
  }

  Serial.print("Distance: ");
  Serial.print(distance); // Menampilkan jarak ke serial monitor
  Serial.print(" cm, Light Intensity: "); 
  Serial.println(lightIntensity); // Menampilkan intensitas cahaya ke serial monitor

  delay(2000); // Delay selama 2 detik sebelum melakukan iterasi berikutnya dari loop
}
