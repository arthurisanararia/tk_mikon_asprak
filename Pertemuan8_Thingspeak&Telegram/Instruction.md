# PERTEMUAN 9
# ESP32 IoT Terintegrasi: Monitoring ThingSpeak & Kendali Telegram Bot

---

## A. TUJUAN
1. Mengonfigurasi ESP32 untuk mengirim data sensor ke **ThingSpeak** dan memantau hasilnya dalam bentuk grafik.
2. Membuat **Telegram Bot** menggunakan @BotFather serta mengendalikan aktuator (LED / Motor Servo) dan membaca data sensor melalui chat Telegram.
3. Memahami perbedaan karakteristik kedua platform: ThingSpeak untuk **monitoring** (satu arah) dan Telegram untuk **kendali** (dua arah).

---

## B. DASAR TEORI

### ThingSpeak
ThingSpeak adalah platform IoT berbasis web untuk mengumpulkan dan memvisualisasikan data.

- **Channel & Field:** Data diatur dalam channel. Setiap channel memiliki beberapa field (field1, field2, dst.) untuk menyimpan jenis data yang berbeda.
- **Write API Key:** Kunci yang dibutuhkan ESP32 untuk **mengirim** data ke channel.

### Telegram Bot
Telegram Bot adalah akun Telegram yang dioperasikan oleh program. ESP32 secara berkala memeriksa pesan baru ke server Telegram. Jika ada perintah, ESP32 memprosesnya lalu membalas.

> **Perbedaan:** ThingSpeak = ESP32 hanya mengirim data (monitoring). Telegram = ESP32 mengirim dan menerima pesan (monitoring + kendali).

---

## C. ALAT DAN BAHAN

> **Catatan Pin:** Praktikum ini bisa memakai **ESP32 DevKit V1** atau **ESP32-S3**. Hampir semua pin sama, kecuali **pin I2C** (SDA/SCL) untuk sensor — lihat tabel di tiap rangkaian. Kode yang diberikan sudah otomatis memilih pin yang benar sesuai board kalian.

1. ESP32 DevKit V1 / ESP32-S3 (1 unit)
2. Kabel Data USB Micro / Type C (1 pcs)
3. Breadboard & Kabel Jumper
4. Laptop / PC terpasang VSCode + PlatformIO (Hotspot HP **2.4 GHz**)
5. **Komponen A :**
   - LED 5mm (1 pcs) & Resistor 220 $\Omega$ (1 pcs)
   - Sensor BME280
6. **Komponen B :**
   - Motor Servo SG90 (1 pcs)
   - Sensor INA219

### Tabel Pin (DevKit V1 vs ESP32-S3)

| Fungsi | ESP32 DevKit V1 | ESP32-S3 |
|--------|-----------------|----------|
| LED (Komponen A) | GPIO 18 | GPIO 18 |
| Servo Sinyal (Komponen B) | GPIO 16 | GPIO 16 |
| **I2C SDA** (sensor) | GPIO 21 | **GPIO 8** |
| **I2C SCL** (sensor) | GPIO 22 | **GPIO 9** |

---

## D. SETUP PLATFORM

### 1. Setup ThingSpeak

1. Buka https://thingspeak.com, klik **Get Started For Free**, buat akun.
2. Pilih menu **Channels > New Channel**, isi:
   - **Name:** `Kelompok [X] - Mikon`
   - Centang **Field 1** dan **Field 2**:
     - Komponen A: `Suhu (C)` dan `Kelembapan (%)`
     - Komponen B: `Arus (mA)` dan `Tegangan (V)`
3. Klik **Save Channel**.
4. Buka tab **API Keys > Write API Key**, salin kunci tersebut.

### 2. Setup Telegram Bot

1. Buka aplikasi Telegram, cari **@BotFather** (akun resmi dengan centang biru), lalu tekan **Start**.
2. Kirim pesan `/newbot` untuk membuat bot baru.
3. BotFather meminta **nama** bot — kirim: `Kelompok [X] Mikon Bot` (nama bebas, boleh pakai spasi).
4. BotFather meminta **username** bot — kirim: `kelompok[X]_mikon_bot`.
   > **Penting:** Username harus **unik** (belum dipakai orang lain) dan wajib diakhiri `bot`. Jika ditolak, coba variasi lain seperti `kelompok[X]_mikon_2025_bot` atau `kel[X]_mikonbot`.
5. BotFather membalas berisi dua hal — **salin & simpan keduanya**:
   - **Link bot:** `t.me/kelompok[X]_mikon_bot` (untuk membuka chat dengan bot kalian)
   - **Token:** format panjang seperti `7123456789:AAH9xQw...` (untuk dimasukkan ke kode program)
6. Klik link bot tersebut (bisa langsung dari chat BotFather), lalu:
   - Tekan tombol **Start** (biasanya di bagian bawah layar chat).
   - Kirim satu pesan apa pun, misal: `halo`.
   - Ini wajib dilakukan agar bot mengenali chat kalian dan siap menerima/mengirim pesan dari ESP32.

### 3. Instalasi Library (platformio.ini)

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
lib_deps =
    bblanchon/ArduinoJson@^6.21.5
    shurillu/CTBot@2.1.14
    adafruit/Adafruit BME280 Library@^2.2.4
    adafruit/Adafruit Unified Sensor@^1.1.14
    adafruit/Adafruit INA219@^1.2.3
```

> **Jika memakai ESP32-S3:** cukup ganti baris `board` menjadi `board = esp32-s3-devkitc-1`. Baris library tetap sama.

---

## E. TUGAS PRAKTIKUM

---

# KOMPONEN A (BME280 + LED)

---

### TAHAP 1: Monitoring Suhu & Kelembapan ke ThingSpeak

**1. Rangkaian Hardware:**
- Anoda LED $\rightarrow$ **GPIO 18** (melalui Resistor 220 $\Omega$).
- Katoda LED $\rightarrow$ **GND**.
- Sensor BME280:
  - **VIN** $\rightarrow$ **3V3**
  - **GND** $\rightarrow$ **GND**
  - **SDA** $\rightarrow$ **GPIO 21** (DevKit V1) / **GPIO 8** (S3)
  - **SCL** $\rightarrow$ **GPIO 22** (DevKit V1) / **GPIO 9** (S3)

**2. Kode Program Tugas 1:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;

const char* ssid = "NAMA_HOTSPOT_HP";
const char* password = "PASSWORD_HOTSPOT";

String apiKey = "WRITE_API_KEY_THINGSPEAK";  // GANTI dengan API Key kalian

const unsigned long intervalThingspeak = 20000;
unsigned long lastSend = 0;


void setup() {
  Serial.begin(115200);

  Wire.begin();
  bme.begin(0x76);

  Serial.print("Koneksi WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
}

void loop() {
  if (millis() - lastSend >= intervalThingspeak) {
    lastSend = millis();

    float suhu = bme.readTemperature();
    float kelembapan = bme.readHumidity();

    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" C | Kelembapan: ");
    Serial.println(kelembapan);

    HTTPClient http;
    String url = "http://api.thingspeak.com/update?api_key=" + apiKey +
                 "&field1=" + String(suhu) +
                 "&field2=" + String(kelembapan);

    http.begin(url);
    int httpCode = http.GET();
    String respon = http.getString();
    http.end();

    if (httpCode == 200 && respon != "0") {
      Serial.println("Data terkirim ke ThingSpeak, entry ke-" + respon);
    } else {
      Serial.println("Gagal kirim ke ThingSpeak (kode " + String(httpCode) + ")");
    }
  }
}
```

**3. Pengujian Tugas 1:**
- Ganti `ssid`, `password`, dan `apiKey`, lalu upload.
- Buka Serial Monitor (baudrate 115200), pastikan nilai suhu & kelembapan tercetak.
- Buka channel ThingSpeak $\rightarrow$ tab **Private View** $\rightarrow$ grafik Field 1 dan Field 2 terisi setiap 20 detik.

---

### TAHAP 2: Kendali LED via Telegram Bot

**1. Rangkaian:** tetap seperti Tahap 1.

**2. Kode Program Tugas 2:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <CTBot.h>

CTBot myBot;

String ssid   = "NAMA_HOTSPOT_HP";
String pass   = "PASSWORD_HOTSPOT";
String token  = "TOKEN_BOT_DARI_BOTFATHER";

const int pinLED = 18;

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);

  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);

  if (myBot.testConnection())
    Serial.println("Telegram OK");
  else
    Serial.println("Telegram GAGAL");
}

void loop() {
  TBMessage msg;
  if (myBot.getNewMessage(msg)) {
    Serial.print("Pesan masuk: ");
    Serial.println(msg.text);

    if (msg.text.equalsIgnoreCase("LED ON")) {
      digitalWrite(pinLED, HIGH);
      myBot.sendMessage(msg.sender.id, "LED menyala");
    } else if (msg.text.equalsIgnoreCase("LED OFF")) {
      digitalWrite(pinLED, LOW);
      myBot.sendMessage(msg.sender.id, "LED mati");
    } else {
      myBot.sendMessage(msg.sender.id, "Kirim LED ON atau LED OFF");
    }
  }
  delay(500);
}
```

**3. Pengujian Tugas 2:**
- Ganti `ssid`, `pass`, dan `token`, lalu upload.
- Kirim `LED ON` ke bot $\rightarrow$ LED menyala, bot membalas `LED menyala`.
- Kirim `LED OFF` $\rightarrow$ LED mati, bot membalas `LED mati`.

---

### TAHAP 3: Bot Baca Sensor + Kendali Gabungan

Gabungkan Tahap 1 dan Tahap 2. Bot bisa mengendalikan LED **dan** melaporkan bacaan sensor saat diminta. Data tetap mengalir ke ThingSpeak di latar belakang.

**1. Rangkaian:** tetap seperti Tahap 1 (LED + BME280).

**2. Kode Program Tugas 3:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <CTBot.h>

Adafruit_BME280 bme;
CTBot myBot;

String ssid   = "NAMA_HOTSPOT_HP";
String pass   = "PASSWORD_HOTSPOT";
String token  = "TOKEN_BOT_DARI_BOTFATHER";
String apiKey = "WRITE_API_KEY_THINGSPEAK";

const int pinLED = 18;
const unsigned long intervalThingspeak = 20000;
unsigned long lastSend = 0;

void kirimThingspeak(float suhu, float kelembapan) {
  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=" + apiKey +
               "&field1=" + String(suhu) +
               "&field2=" + String(kelembapan);

  http.begin(url);
  int httpCode = http.GET();
  String respon = http.getString();
  http.end();

  if (httpCode == 200 && respon != "0") {
    Serial.println(">> ThingSpeak terkirim, entry ke-" + respon);
  } else {
    Serial.println(">> ThingSpeak GAGAL (kode " + String(httpCode) + ")");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);

  Wire.begin();
  bme.begin(0x76);

  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);
  Serial.println(myBot.testConnection() ? "Telegram OK" : "Telegram GAGAL");
}

void loop() {
  if (millis() - lastSend >= intervalThingspeak) {
    lastSend = millis();
    float suhu = bme.readTemperature();
    float kelembapan = bme.readHumidity();
    kirimThingspeak(suhu, kelembapan);
  }

  TBMessage msg;
  if (myBot.getNewMessage(msg)) {
    Serial.print("Pesan masuk: ");
    Serial.println(msg.text);

    if (msg.text.equalsIgnoreCase("LED ON")) {
      digitalWrite(pinLED, HIGH);
      myBot.sendMessage(msg.sender.id, "LED menyala");
    } else if (msg.text.equalsIgnoreCase("LED OFF")) {
      digitalWrite(pinLED, LOW);
      myBot.sendMessage(msg.sender.id, "LED mati");
    } else if (msg.text.equalsIgnoreCase("BACA SUHU")) {
      float suhu = bme.readTemperature();
      String reply = "Suhu: " + String(suhu, 1) + " C";
      myBot.sendMessage(msg.sender.id, reply);
    } else if (msg.text.equalsIgnoreCase("BACA KELEMBAPAN")) {
      float kelembapan = bme.readHumidity();
      String reply = "Kelembapan: " + String(kelembapan, 0) + " %";
      myBot.sendMessage(msg.sender.id, reply);
    } else {
      String reply = "Perintah: LED ON / LED OFF\nBACA SUHU / BACA KELEMBAPAN";
      myBot.sendMessage(msg.sender.id, reply);
    }
  }
  delay(50);
}
```

**3. Pengujian Tugas 3:**
- Kirim `BACA SUHU` $\rightarrow$ bot membalas `Suhu: [suhu] C`.
- Kirim `BACA KELEMBAPAN` $\rightarrow$ bot membalas `Kelembapan: [kelembapan] %`.
- Kirim `LED ON` / `LED OFF` $\rightarrow$ LED tetap bereaksi.
- Amati Serial Monitor: setiap 20 detik muncul `>> ThingSpeak terkirim`.
- Buka ThingSpeak: grafik tetap terisi walau tidak ada aktivitas Telegram.

---

# KOMPONEN B (INA219 + Motor Servo)

---

### TAHAP 1: Monitoring Arus & Tegangan ke ThingSpeak

**1. Rangkaian Hardware:**
- Motor Servo SG90:
  - Sinyal (kuning/oranye) $\rightarrow$ **GPIO 16**
  - VCC (merah) $\rightarrow$ **5V / VIN**
  - GND (cokelat/hitam) $\rightarrow$ **GND**
- Sensor INA219:
  - **VIN** $\rightarrow$ **3V3**
  - **GND** $\rightarrow$ **GND**
  - **SDA** $\rightarrow$ **GPIO 21** (DevKit V1) / **GPIO 8** (S3)
  - **SCL** $\rightarrow$ **GPIO 22** (DevKit V1) / **GPIO 9** (S3)
  - **VIN+** dan **VIN-** $\rightarrow$ diletakkan di tengah-tengah jalur signal (agar terbaca tegangan dan arus sinyal ke servo).

**2. Kode Program Tugas 1:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

const char* ssid = "NAMA_HOTSPOT_HP";
const char* password = "PASSWORD_HOTSPOT";

String apiKey = "WRITE_API_KEY_THINGSPEAK";  // GANTI dengan API Key kalian

const unsigned long intervalThingspeak = 20000;
unsigned long lastSend = 0;


void setup() {
  Serial.begin(115200);

  Wire.begin();
  ina219.begin();

  Serial.print("Koneksi WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
}

void loop() {
  if (millis() - lastSend >= intervalThingspeak) {
    lastSend = millis();

    float arus_mA = ina219.getCurrent_mA();
    float tegangan_V = ina219.getBusVoltage_V();

    Serial.print("Arus: ");
    Serial.print(arus_mA);
    Serial.print(" mA | Tegangan: ");
    Serial.println(tegangan_V);

    HTTPClient http;
    String url = "http://api.thingspeak.com/update?api_key=" + apiKey +
                 "&field1=" + String(arus_mA) +
                 "&field2=" + String(tegangan_V);

    http.begin(url);
    int httpCode = http.GET();
    String respon = http.getString();
    http.end();

    if (httpCode == 200 && respon != "0") {
      Serial.println("Data terkirim ke ThingSpeak, entry ke-" + respon);
    } else {
      Serial.println("Gagal kirim ke ThingSpeak (kode " + String(httpCode) + ")");
    }
  }
}
```

**3. Pengujian Tugas 1:**
- Ganti `ssid`, `password`, dan `apiKey`, lalu upload.
- Buka Serial Monitor, pastikan nilai arus & tegangan tercetak.
- Buka channel ThingSpeak $\rightarrow$ tab **Private View** $\rightarrow$ grafik Field 1 dan Field 2 terisi setiap 20 detik.

---

### TAHAP 2: Kendali Servo via Telegram Bot

**1. Rangkaian:** tetap seperti Tahap 1.

**2. Kode Program Tugas 2:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <CTBot.h>

CTBot myBot;

String ssid   = "NAMA_HOTSPOT_HP";
String pass   = "PASSWORD_HOTSPOT";
String token  = "TOKEN_BOT_DARI_BOTFATHER";

const int servoPin = 16;
const int servoChannel = 0;
const int servoFreq = 50;
const int servoResolution = 12;

void setServo(int angle) {
  angle = constrain(angle, 0, 180);
  int duty = map(angle, 0, 180, 102, 512);
  ledcWrite(servoChannel, duty);
}

void setup() {
  Serial.begin(115200);
  ledcSetup(servoChannel, servoFreq, servoResolution);
  ledcAttachPin(servoPin, servoChannel);
  setServo(90);

  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);

  if (myBot.testConnection())
    Serial.println("Telegram OK");
  else
    Serial.println("Telegram GAGAL");
}

void loop() {
  TBMessage msg;
  if (myBot.getNewMessage(msg)) {
    Serial.print("Pesan masuk: ");
    Serial.println(msg.text);

    if (msg.text.equalsIgnoreCase("MIN")) {
      setServo(0);
      myBot.sendMessage(msg.sender.id, "Servo ke 0 derajat");
    } else if (msg.text.equalsIgnoreCase("MID")) {
      setServo(90);
      myBot.sendMessage(msg.sender.id, "Servo ke 90 derajat");
    } else if (msg.text.equalsIgnoreCase("MAX")) {
      setServo(180);
      myBot.sendMessage(msg.sender.id, "Servo ke 180 derajat");
    } else {
      int angle = msg.text.toInt();
      if (angle >= 0 && angle <= 180) {
        setServo(angle);
        String reply = "Servo ke " + String(angle) + " derajat";
        myBot.sendMessage(msg.sender.id, reply);
      } else {
        myBot.sendMessage(msg.sender.id, "Kirim MIN, MID, MAX, atau angka 0-180");
      }
    }
  }
  delay(500);
}
```

**3. Pengujian Tugas 2:**
- Ganti `ssid`, `pass`, dan `token`, lalu upload.
- Kirim `MIN` $\rightarrow$ servo ke 0°, `MID` $\rightarrow$ 90°, `MAX` $\rightarrow$ 180°.
- Kirim angka (contoh: `45`, `135`) $\rightarrow$ servo bergerak ke sudut yang diminta.

---

### TAHAP 3: Bot Baca Sensor + Kendali Gabungan

Gabungkan Tahap 1 dan Tahap 2. Bot bisa mengendalikan servo **dan** melaporkan bacaan sensor INA219 saat diminta. Data tetap mengalir ke ThingSpeak.

**1. Rangkaian:** tetap seperti Tahap 1 (servo + INA219).

**2. Kode Program Tugas 3:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <CTBot.h>

Adafruit_INA219 ina219;
CTBot myBot;

String ssid   = "NAMA_HOTSPOT_HP";
String pass   = "PASSWORD_HOTSPOT";
String token  = "TOKEN_BOT_DARI_BOTFATHER";
String apiKey = "WRITE_API_KEY_THINGSPEAK";

const int servoPin = 16;
const int servoChannel = 0;
const int servoFreq = 50;
const int servoResolution = 12;

const unsigned long intervalThingspeak = 20000;
unsigned long lastSend = 0;

void setServo(int angle) {
  angle = constrain(angle, 0, 180);
  int duty = map(angle, 0, 180, 102, 512);
  ledcWrite(servoChannel, duty);
}

void kirimThingspeak(float arus_mA, float tegangan_V) {
  HTTPClient http;
  String url = "http://api.thingspeak.com/update?api_key=" + apiKey +
               "&field1=" + String(arus_mA) +
               "&field2=" + String(tegangan_V);

  http.begin(url);
  int httpCode = http.GET();
  String respon = http.getString();
  http.end();

  if (httpCode == 200 && respon != "0") {
    Serial.println(">> ThingSpeak terkirim, entry ke-" + respon);
  } else {
    Serial.println(">> ThingSpeak GAGAL (kode " + String(httpCode) + ")");
  }
}

void setup() {
  Serial.begin(115200);
  ledcSetup(servoChannel, servoFreq, servoResolution);
  ledcAttachPin(servoPin, servoChannel);
  setServo(90);

  Wire.begin();
  ina219.begin();

  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);
  Serial.println(myBot.testConnection() ? "Telegram OK" : "Telegram GAGAL");
}

void loop() {
  if (millis() - lastSend >= intervalThingspeak) {
    lastSend = millis();
    float arus_mA = ina219.getCurrent_mA();
    float tegangan_V = ina219.getBusVoltage_V();
    kirimThingspeak(arus_mA, tegangan_V);
  }

  TBMessage msg;
  if (myBot.getNewMessage(msg)) {
    Serial.print("Pesan masuk: ");
    Serial.println(msg.text);

    if (msg.text.equalsIgnoreCase("MIN")) {
      setServo(0);
      myBot.sendMessage(msg.sender.id, "Servo ke 0 derajat");
    } else if (msg.text.equalsIgnoreCase("MID")) {
      setServo(90);
      myBot.sendMessage(msg.sender.id, "Servo ke 90 derajat");
    } else if (msg.text.equalsIgnoreCase("MAX")) {
      setServo(180);
      myBot.sendMessage(msg.sender.id, "Servo ke 180 derajat");
    } else if (msg.text.equalsIgnoreCase("BACA ARUS")) {
      float arus_mA = ina219.getCurrent_mA();
      String reply = "Arus: " + String(arus_mA, 1) + " mA";
      myBot.sendMessage(msg.sender.id, reply);
    } else if (msg.text.equalsIgnoreCase("BACA TEGANGAN")) {
      float tegangan_V = ina219.getBusVoltage_V();
      String reply = "Tegangan: " + String(tegangan_V, 2) + " V";
      myBot.sendMessage(msg.sender.id, reply);
    } else {
      int angle = msg.text.toInt();
      if (angle >= 0 && angle <= 180) {
        setServo(angle);
        String reply = "Servo ke " + String(angle) + " derajat";
        myBot.sendMessage(msg.sender.id, reply);
      } else {
        String reply = "Perintah: MIN / MID / MAX / angka 0-180\nBACA ARUS / BACA TEGANGAN";
        myBot.sendMessage(msg.sender.id, reply);
      }
    }
  }
  delay(50);
}
```

**3. Pengujian Tugas 3:**
- Kirim `BACA ARUS` $\rightarrow$ bot membalas `Arus: [arus] mA`.
- Kirim `BACA TEGANGAN` $\rightarrow$ bot membalas `Tegangan: [tegangan] V`.
- Kirim `MIN` / `MID` / `MAX` / angka $\rightarrow$ servo tetap bereaksi.
- Amati Serial Monitor: setiap 20 detik muncul `>> ThingSpeak terkirim`.
- Buka ThingSpeak: grafik tetap terisi walau tidak ada aktivitas Telegram.
