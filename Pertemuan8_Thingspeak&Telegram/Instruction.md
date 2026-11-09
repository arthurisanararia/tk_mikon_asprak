# PERTEMUAN 8
# ESP32 IoT Terintegrasi: Monitoring ThingSpeak & Kendali Telegram Bot

---

## A. TUJUAN
1. Praktikan mampu mengonfigurasi ESP32 untuk mengirim data sensor ke platform cloud **ThingSpeak** dan memantau hasilnya dalam bentuk grafik (*monitoring*).
2. Praktikan mampu membuat **Telegram Bot** menggunakan @BotFather serta mengendalikan aktuator (LED / Motor Servo) dan membaca data sensor melalui chat Telegram (*controlling*).
3. Praktikan memahami perbedaan karakteristik kedua platform: ThingSpeak untuk **visualisasi data** dan Telegram untuk **kendali dua arah (bidirectional)**.

> **Catatan Waktu:** Total waktu praktikum adalah **100 menit**. Karena itu, alur praktikum dirancang bertahap dari yang paling mudah ke yang lebih kompleks. Kelompok yang kebagian sensor digital **wajib** menggunakan kode template yang sudah disediakan dan hanya fokus pada pengujian, agar selesai tepat waktu.

---

## B. DASAR TEORI

### 1. ThingSpeak
ThingSpeak adalah platform analitik IoT berbasis web yang dikembangkan oleh MathWorks (pembuat MATLAB). ThingSpeak memungkinkan pengguna mengumpulkan, menganalisis, memvisualisasikan, dan bertindak berdasarkan data yang dikirim oleh perangkat yang terhubung. Fitur utamanya:

1. **Pengumpulan Data:** Data dikirim dari perangkat IoT melalui protokol HTTP/MQTT ke sebuah **Channel**.
2. **Analisis Data:** Mendukung kode MATLAB yang dijalankan langsung dari cloud untuk analisis *real-time*.
3. **Visualisasi Data:** Menyediakan grafik, gauge, dan heatmap yang membantu memahami data.
4. **Channel & Field:** Data diatur dalam channel dan field. Setiap channel bisa memiliki beberapa field (field1, field2, dst.), masing-masing menyimpan jenis data tertentu.
5. **Write API Key:** Kunci yang dibutuhkan ESP32 untuk **menulis (mengirim)** data ke channel. Tanpa kunci ini, pengiriman data akan ditolak server.

Arsitektur monitoring:

```
[ ESP32 + Sensor ] --- HTTP POST (Write API Key + data) ---> [ api.thingspeak.com ]
                                                                    |
                                                            Channel & Field1..N
                                                                    v
                                                          [ Grafik / Dashboard ]
```

### 2. Telegram Bot
Telegram adalah aplikasi pesan instan berbasis cloud. Telegram menyediakan API yang kuat dan fleksibel untuk membuat **bot** — akun khusus yang dioperasikan oleh program, bukan manusia. Dalam konteks IoT, bot dapat digunakan untuk mengontrol dan memantau perangkat melalui pesan teks.

Cara kerjanya sederhana: ESP32 secara berkala "bertanya" ke server Telegram (`api.telegram.org`) apakah ada pesan baru untuk bot tersebut. Jika ada, ESP32 memproses perintahnya (menyalakan LED, menggerakkan servo, membaca sensor) lalu membalas lewat `sendMessage`. Berbeda dengan ThingSpeak yang sifatnya satu arah (hanya mengirim data), Telegram bersifat **dua arah**: perintah masuk dan laporan keluar melalui satu jalur yang sama.

Arsitektur kendali:

```
[ Telegram User ] --- perintah: "LED ON" ---> [ api.telegram.org ]
                                                     ^
                                                     |  (getNewMessage / sendMessage)
                                                     v
                                              [ ESP32 + Aktuator ]
```

> **Perbedaan singkat:** ThingSpeak = ESP32 **hanya mengirim** data ke cloud (monitoring). Telegram = ESP32 **mengirim dan menerima** pesan (monitoring + kendali).

---

## C. ALAT DAN BAHAN

1. ESP32 DevKit V1 / ESP32-S3 (1 unit)
2. Kabel Data USB Micro / Type C (1 pcs)
3. Breadboard & Kabel Jumper
4. Laptop / PC terpasang VSCode + PlatformIO (Hotspot HP **2.4 GHz**)
5. Akun **ThingSpeak** (https://thingspeak.com) dan **Telegram** (HP/laptop) — dibuat saat praktikum
6. **Komponen Jalur A (Kelompok 1–10):**
   - LED 5mm (1 pcs) & Resistor 220 $\Omega$ (1 pcs)
   - **Sensor SHT4x** (atau BME280) — sensor suhu & kelembapan digital (I2C)
7. **Komponen Jalur B (Kelompok 11–20):**
   - Motor Servo SG90 (1 pcs)
   - **Sensor INA219** — sensor arus & tegangan digital (I2C)

---

## D. SETUP PLATFORM (SEMUA KELOMPOK, ± 20 MENIT)

Kerjakan bersama-sama sebelum masuk ke tugas jalur masing-masing.

### 1. Setup ThingSpeak

1. Buka https://thingspeak.com lalu klik **Get Started For Free** dan buat akun (Sign up).
2. Setelah login, pilih menu **Channels > New Channel**, isi:
   - **Name:** `Kelompok [X] - Mikon`
   - **Description:** bebas, misal: `Monitoring Praktikum Mikrokontroler`
   - Centang **Field 1** dan isi label sesuai sensor kelompok kalian:
     - Jalur A: `Suhu (C)` dan centang juga **Field 2**: `Kelembapan (%)`
     - Jalur B: `Arus (mA)` dan centang juga **Field 2**: `Tegangan (V)`
3. Klik **Save Channel**.
4. Buka tab **API Keys > Write API Key**, salin kunci tersebut (jangan sampai hilang, ini yang dipakai di kode).

> **Penting:** ThingSpeak hanya mengizinkan pembaruan data **minimal setiap 15 detik** untuk akun gratis. Pengiriman lebih cepat dari itu akan ditolak server, jadi di kode nanti digunakan interval 20 detik.

### 2. Setup Telegram Bot

1. Buka aplikasi Telegram, cari **@BotFather** (akun bot resmi dengan tanda centang biru), lalu tekan **Start**.
2. Kirim pesan `/newbot`.
3. BotFather meminta **nama** bot — kirim nama yang kalian inginkan (misal: `Kelompok 1 Mikon Bot`).
4. BotFather meminta **username** bot — kirim username unik yang diakhiri kata `bot` (misal: `kelompok1_mikon_bot`).
5. BotFather akan membalas dengan:
   - **Link bot** kalian (misal: `t.me/kelompok1_mikon_bot`)
   - **Token** (format mirip: `7123456789:AAH9xQwERTYuiopASDFghjklZXcvbnM123`) — **salin dan simpan token ini**, ini yang dipakai di kode.
6. **Langkah wajib:** buka link bot kalian, tekan **Start**, lalu kirim satu pesan apa pun (misal: `halo`). Ini agar chat kalian terdaftar di server Telegram dan pesan bisa terbaca oleh ESP32.
7. Untuk mendapatkan **Chat ID** kalian (dibutuhkan di Tahap 3), cari bot **@userinfobot**, tekan Start, lalu bot itu membalas dengan `Id:` berupa angka. Catat angkanya.

### 3. Instalasi Library (PlatformIO)

Buka `platformio.ini` pada project kalian, lalu tambahkan dua baris library berikut pada bagian `lib_deps`:

```ini
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
lib_deps =
    bblanchon/ArduinoJson@^7.4.2
    shurillu/CTBot@2.1.14
    adafruit/Adafruit SHT4x Library@^1.0.4   ; HANYA Jalur A (SHT4x)
    adafruit/Adafruit INA219@^1.2.3          ; HANYA Jalur B (INA219)
```

- **CTBot** adalah library Telegram Bot untuk ESP32/ESP8266. Library ini butuh **ArduinoJson** untuk memproses pesan JSON dari server Telegram.
- **Adafruit SHT4x** dan **Adafruit INA219** adalah library driver sensor I2C masing-masing jalur.
- Jalur A (BME280) mengganti baris sensor dengan `adafruit/Adafruit BME280 Library@^2.2.4` dan menambahkan `adafruit/Adafruit Unified Sensor@^1.1.14`.
- Sesuaikan nama env dan board dengan yang kalian gunakan (misal board S3).

---

## E. TUGAS PRAKTIKUM

Kedua jalur mengerjakan **3 tahap yang sama strukturnya** dengan tingkat kesulitan bertingkat:

| Tahap | Platform | Fokus | Estimasi |
|-------|----------|-------|----------|
| 1 | ThingSpeak | Kirim data sensor ke cloud, pantau grafik | ± 15 menit |
| 2 | Telegram | Kendali aktuator ON/OFF / POSISI lewat chat | ± 20 menit |
| 3 | Telegram | Minta bot membaca sensor saat diminta (dua arah) | ± 20 menit |

---

# JALUR A — Kelompok 1–10 (Sensor Suhu & Kelembapan + LED)

---

### TAHAP 1: Monitoring Suhu & Kelembapan ke ThingSpeak

**1. Rangkaian Hardware:**
- Anoda LED $\rightarrow$ **GPIO 18** (melalui Resistor 220 $\Omega$). *(Dipakai lagi di Tahap 2)*
- Katoda LED $\rightarrow$ **GND**.
- Sensor SHT4x:
  - **VIN/VCC** $\rightarrow$ **3V3**
  - **GND** $\rightarrow$ **GND**
  - **SDA** $\rightarrow$ **GPIO 21**
  - **SCL** $\rightarrow$ **GPIO 22**

**2. Kode Program Tugas 1:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

const char* ssid = "NAMA_HOTSPOT_HP";
const char* password = "PASSWORD_HOTSPOT";

const char* server = "api.thingspeak.com";
String apiKey = "WRITE_API_KEY_THINGSPEAK";  // GANTI dengan API Key kalian

const unsigned long intervalThingspeak = 20000;  // 20 detik (min 15 detik)
unsigned long lastSend = 0;

WiFiClient client;

void setup() {
  Serial.begin(115200);

  Wire.begin();  // SDA = 21, SCL = 22 (default ESP32)
  sht4.begin();
  sht4.setPrecision(SHT4X_HIGH_PRECISION);

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

    sensors_event_t hum, temp;
    sht4.getEvent(&hum, &temp);

    Serial.print("Suhu: ");
    Serial.print(temp.temperature);
    Serial.print(" C | Kelembapan: ");
    Serial.println(hum.relative_humidity);

    if (client.connect(server, 80)) {
      String postStr = apiKey;
      postStr += "&field1=" + String(temp.temperature);
      postStr += "&field2=" + String(hum.relative_humidity);

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      Serial.println("Data terkirim ke ThingSpeak!");
    } else {
      Serial.println("Gagal koneksi ke ThingSpeak");
    }
    client.stop();
  }
}
```

**3. Pengujian Tugas 1:**
- Upload kode (ganti `ssid`, `password`, dan `apiKey` terlebih dahulu).
- Buka Serial Monitor $\rightarrow$ pastikan nilai suhu & kelembapan tercetak dan muncul tulisan `Data terkirim ke ThingSpeak!`.
- Buka halaman channel di ThingSpeak $\rightarrow$ tab **Private View** $\rightarrow$ grafik Field 1 dan Field 2 mulai terisi titik data setiap 20 detik.
- **Coba juga:** tiup/hangatkan sensor dengan tangan dan amati grafik berubah naik.

---

### TAHAP 2: Kendali LED via Telegram Bot

**1. Penambahan Hardware:** tidak ada, rangkaian LED dari Tahap 1 tetap terpasang.

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

  Serial.println("Connecting WiFi...");
  if (myBot.wifiConnect(ssid, pass))
    Serial.println("WiFi OK");
  else
    Serial.println("WiFi GAGAL!");

  myBot.setTelegramToken(token);

  if (myBot.testConnection())
    Serial.println("Koneksi Telegram OK");
  else
    Serial.println("Koneksi Telegram GAGAL");
}

void loop() {
  TBMessage msg;
  if (myBot.getNewMessage(msg)) {
    Serial.print("Pesan masuk: ");
    Serial.println(msg.text);

    if (msg.text.equalsIgnoreCase("LED ON")) {
      digitalWrite(pinLED, HIGH);
      myBot.sendMessage(msg.sender.id, "LED is now ON");
    } else if (msg.text.equalsIgnoreCase("LED OFF")) {
      digitalWrite(pinLED, LOW);
      myBot.sendMessage(msg.sender.id, "LED is now OFF");
    } else {
      String reply = (String)"Halo " + msg.sender.username +
                     (String)"! Kirim LED ON atau LED OFF.";
      myBot.sendMessage(msg.sender.id, reply);
    }
  }
  delay(500);
}
```

**3. Pengujian Tugas 2:**
- Upload kode (ganti `ssid`, `pass`, dan `token`).
- Buka chat bot kalian di Telegram, kirim `LED ON` $\rightarrow$ LED menyala dan bot membalas `LED is now ON`.
- Kirim `LED OFF` $\rightarrow$ LED mati dan bot membalas `LED is now OFF`.
- Kirim pesan sembarang (misal `tes`) $\rightarrow$ bot membalas pesan panduan.
- Perhatikan bahwa perintah **tidak case-sensitive** karena memakai `equalsIgnoreCase()` — coba ketik `led on` huruf kecil semua.

---

### TAHAP 3: Bot Membaca Sensor & Kendali Gabungan

Sekarang gabungkan Tahap 1 + Tahap 2 menjadi satu program utuh: bot bisa mengendalikan LED **sekaligus** melaporkan bacaan sensor saat diminta.

**1. Hardware:** rangkaian penuh dari Tahap 1 (LED + sensor SHT4x).

**2. Kode Program Tugas 3:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_SHT4x.h>
#include <CTBot.h>

Adafruit_SHT4x sht4 = Adafruit_SHT4x();
CTBot myBot;

String ssid   = "NAMA_HOTSPOT_HP";
String pass   = "PASSWORD_HOTSPOT";
String token  = "TOKEN_BOT_DARI_BOTFATHER";
String apiKey = "WRITE_API_KEY_THINGSPEAK";

const int pinLED = 18;
const unsigned long intervalThingspeak = 20000;
unsigned long lastSend = 0;
WiFiClient client;

void kirimThingspeak(float suhu, float kelembapan) {
  if (client.connect("api.thingspeak.com", 80)) {
    String postStr = apiKey;
    postStr += "&field1=" + String(suhu);
    postStr += "&field2=" + String(kelembapan);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println(">> ThingSpeak terkirim");
  }
  client.stop();
}

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);

  Wire.begin();
  sht4.begin();
  sht4.setPrecision(SHT4X_HIGH_PRECISION);

  Serial.println("Connecting WiFi...");
  if (myBot.wifiConnect(ssid, pass))
    Serial.println("WiFi OK");
  else
    Serial.println("WiFi GAGAL!");

  myBot.setTelegramToken(token);
  Serial.println(myBot.testConnection() ? "Telegram OK" : "Telegram GAGAL");
}

void loop() {
  // 1. Kirim data sensor ke ThingSpeak tiap 20 detik (non-blocking)
  if (millis() - lastSend >= intervalThingspeak) {
    lastSend = millis();
    sensors_event_t hum, temp;
    sht4.getEvent(&hum, &temp);
    kirimThingspeak(temp.temperature, hum.relative_humidity);
  }

  // 2. Terima & balas pesan Telegram
  TBMessage msg;
  if (myBot.getNewMessage(msg)) {
    Serial.print("Pesan masuk: ");
    Serial.println(msg.text);

    if (msg.text.equalsIgnoreCase("LED ON")) {
      digitalWrite(pinLED, HIGH);
      myBot.sendMessage(msg.sender.id, "LED is now ON");
    } else if (msg.text.equalsIgnoreCase("LED OFF")) {
      digitalWrite(pinLED, LOW);
      myBot.sendMessage(msg.sender.id, "LED is now OFF");
    } else if (msg.text.equalsIgnoreCase("BACA SUHU")) {
      sensors_event_t hum, temp;
      sht4.getEvent(&hum, &temp);
      String reply = "Suhu: " + String(temp.temperature, 1) + "*C";
      myBot.sendMessage(msg.sender.id, reply);
    } else if (msg.text.equalsIgnoreCase("BACA KELEMBAPAN")) {
      sensors_event_t hum, temp;
      sht4.getEvent(&hum, &temp);
      String reply = "Kelembapan: " + String(hum.relative_humidity, 0) + "%";
      myBot.sendMessage(msg.sender.id, reply);
    } else {
      String reply = "Perintah dikenal:\n"
                     "- LED ON / LED OFF\n"
                     "- BACA SUHU\n"
                     "- BACA KELEMBAPAN";
      myBot.sendMessage(msg.sender.id, reply);
    }
  }
  delay(50);
}
```

**3. Pengujian Tugas 3:**
- Kirim `BACA SUHU` $\rightarrow$ bot membalas format `Suhu: [suhu]*C` (contoh: `Suhu: 30.4*C`).
- Kirim `BACA KELEMBAPAN` $\rightarrow$ bot membalas format `Kelembapan: [kelembapan]%` (contoh: `Kelembapan: 65%`).
- Kirim pesan selain itu $\rightarrow$ bot membalas daftar perintah yang tersedia.
- Kirim `LED ON` / `LED OFF` $\rightarrow$ LED tetap bereaksi, **sekaligus** pastikan grafik ThingSpeak terus berjalan di latar belakang tanpa terputus.
- Amati Serial Monitor: setiap 20 detik muncul `>> ThingSpeak terkirim` walau tidak ada aktivitas Telegram.

---

# JALUR B — Kelompok 11–20 (Sensor Arus & Tegangan + Motor Servo)

---

### TAHAP 1: Monitoring Arus & Tegangan ke ThingSpeak

**1. Rangkaian Hardware:**
- Motor Servo SG90:
  - Sinyal (kuning/oranye) $\rightarrow$ **GPIO 16**. *(Dipakai lagi di Tahap 2)*
  - VCC (merah) $\rightarrow$ **5V / VIN**.
  - GND (cokelat/hitam) $\rightarrow$ **GND**.
- Sensor INA219:
  - **VIN/VCC** $\rightarrow$ **3V3**
  - **GND** $\rightarrow$ **GND**
  - **SDA** $\rightarrow$ **GPIO 21**
  - **SCL** $\rightarrow$ **GPIO 22**
  - **VIN+** dan **VIN-** disambung seri dengan jalur beban yang mau diukur arusnya (kalau tidak ada beban, cukup hubungkan singkat VIN+ ke VIN- agar terbaca arus mendekati 0 dan tegangan bus).

**2. Kode Program Tugas 1:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

const char* ssid = "NAMA_HOTSPOT_HP";
const char* password = "PASSWORD_HOTSPOT";

const char* server = "api.thingspeak.com";
String apiKey = "WRITE_API_KEY_THINGSPEAK";  // GANTI dengan API Key kalian

const unsigned long intervalThingspeak = 20000;  // 20 detik (min 15 detik)
unsigned long lastSend = 0;

WiFiClient client;

void setup() {
  Serial.begin(115200);

  Wire.begin();  // SDA = 21, SCL = 22 (default ESP32)
  ina219.begin();
  Serial.println("INA219 OK");

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

    if (client.connect(server, 80)) {
      String postStr = apiKey;
      postStr += "&field1=" + String(arus_mA);
      postStr += "&field2=" + String(tegangan_V);

      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      Serial.println("Data terkirim ke ThingSpeak!");
    } else {
      Serial.println("Gagal koneksi ke ThingSpeak");
    }
    client.stop();
  }
}
```

**3. Pengujian Tugas 1:**
- Upload kode (ganti `ssid`, `password`, dan `apiKey`).
- Buka Serial Monitor $\rightarrow$ pastikan nilai arus & tegangan tercetak dan muncul tulisan `Data terkirim ke ThingSpeak!`.
- Buka halaman channel di ThingSpeak $\rightarrow$ tab **Private View** $\rightarrow$ grafik Field 1 dan Field 2 mulai terisi titik data setiap 20 detik.
- **Coba juga:** geser/ganggu sambungan VIN+ dan VIN- dan amati grafik arus berubah.

---

### TAHAP 2: Kendali Servo via Telegram Bot

**1. Penambahan Hardware:** tidak ada, rangkaian servo dari Tahap 1 tetap terpasang.

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
  setServo(90);  // posisi awal tengah

  Serial.println("Connecting WiFi...");
  if (myBot.wifiConnect(ssid, pass))
    Serial.println("WiFi OK");
  else
    Serial.println("WiFi GAGAL!");

  myBot.setTelegramToken(token);

  if (myBot.testConnection())
    Serial.println("Koneksi Telegram OK");
  else
    Serial.println("Koneksi Telegram GAGAL");
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
- Upload kode (ganti `ssid`, `pass`, dan `token`).
- Kirim `MIN` $\rightarrow$ servo ke 0°, `MID` $\rightarrow$ servo ke 90°, `MAX` $\rightarrow$ servo ke 180°.
- Kirim angka sembarang (contoh: `30`, `45`, `135`) $\rightarrow$ servo bergerak ke sudut yang diminta.
- Kirim pesan yang bukan angka/posisi (misal `halo`) $\rightarrow$ bot membalas daftar perintah.

---

### TAHAP 3: Bot Membaca Sensor & Kendali Gabungan

Gabungkan Tahap 1 + Tahap 2 menjadi satu program utuh: bot bisa mengendalikan servo **sekaligus** melaporkan bacaan sensor INA219 saat diminta, dan data tetap mengalir ke ThingSpeak.

**1. Hardware:** rangkaian penuh dari Tahap 1 (servo + sensor INA219).

**2. Kode Program Tugas 3:**

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
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
WiFiClient client;

void setServo(int angle) {
  angle = constrain(angle, 0, 180);
  int duty = map(angle, 0, 180, 102, 512);
  ledcWrite(servoChannel, duty);
}

void kirimThingspeak(float arus_mA, float tegangan_V) {
  if (client.connect("api.thingspeak.com", 80)) {
    String postStr = apiKey;
    postStr += "&field1=" + String(arus_mA);
    postStr += "&field2=" + String(tegangan_V);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println(">> ThingSpeak terkirim");
  }
  client.stop();
}

void setup() {
  Serial.begin(115200);
  ledcSetup(servoChannel, servoFreq, servoResolution);
  ledcAttachPin(servoPin, servoChannel);
  setServo(90);

  Wire.begin();
  ina219.begin();

  Serial.println("Connecting WiFi...");
  if (myBot.wifiConnect(ssid, pass))
    Serial.println("WiFi OK");
  else
    Serial.println("WiFi GAGAL!");

  myBot.setTelegramToken(token);
  Serial.println(myBot.testConnection() ? "Telegram OK" : "Telegram GAGAL");
}

void loop() {
  // 1. Kirim data sensor ke ThingSpeak tiap 20 detik (non-blocking)
  if (millis() - lastSend >= intervalThingspeak) {
    lastSend = millis();
    float arus_mA = ina219.getCurrent_mA();
    float tegangan_V = ina219.getBusVoltage_V();
    kirimThingspeak(arus_mA, tegangan_V);
  }

  // 2. Terima & balas pesan Telegram
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
        String reply = "Perintah dikenal:\n"
                       "- MIN / MID / MAX / angka 0-180\n"
                       "- BACA ARUS\n"
                       "- BACA TEGANGAN";
        myBot.sendMessage(msg.sender.id, reply);
      }
    }
  }
  delay(50);
}
```

**3. Pengujian Tugas 3:**
- Kirim `BACA ARUS` $\rightarrow$ bot membalas format `Arus: [arus] mA` (contoh: `Arus: 12.5 mA`).
- Kirim `BACA TEGANGAN` $\rightarrow$ bot membalas format `Tegangan: [tegangan] V` (contoh: `Tegangan: 3.28 V`).
- Kirim pesan selain itu $\rightarrow$ bot membalas daftar perintah yang tersedia.
- Kirim `MIN` / `MID` / `MAX` / angka $\rightarrow$ servo tetap bereaksi, **sekaligus** pastikan grafik ThingSpeak terus berjalan di latar belakang tanpa terputus.
- Amati Serial Monitor: setiap 20 detik muncul `>> ThingSpeak terkirim` walau tidak ada aktivitas Telegram.

---

## F. TIPS ANTISIPASI WAKTU (100 MENIT)

1. **Jangan ubah kode lebih dari yang diminta.** Semua kode di atas sudah siap pakai — cukup ganti `ssid`, `password`, `apiKey`, dan `token`.
2. Kerjakan **tahap berurutan**, jangan lompat. Tahap 1 dan 2 saling menopang Tahap 3.
3. Jika Serial Monitor menampilkan karakter aneh, pastikan `monitor_speed = 115200` sudah ada di `platformio.ini`.
4. Jika Telegram tidak membalas:
   - Pastikan sudah menekan **Start** di chat bot dan mengirim minimal satu pesan.
   - Cek token di Serial saat `testConnection` — jika `Telegram GAGAL`, biasanya token salah atau hotspot tidak memberi akses internet.
5. Jika ThingSpeak grafik kosong: cek Write API Key, dan pastikan interval pengiriman **≥ 15 detik**.
6. Sensor I2C tidak terdeteksi (nilai nol terus): periksa kabel SDA/SCL tidak tertukar, dan pastikan board mendapat 3V3 (bukan 5V untuk pin VIN sensor).

## G. KUIS

1. Apa kegunaan **Write API Key** pada sistem ThingSpeak ini? Apa yang terjadi jika kunci tersebut salah diketik di program?
2. Dalam fitur ThingSpeak terdapat view **public** dan **private**. Jelaskan perbedaan di antara keduanya!
3. Apa perbedaan karakteristik penggunaan **ThingSpeak** dan **Telegram Bot** sebagai antarmuka IoT? (Petunjuk: arah aliran data — satu arah atau dua arah, serta format informasi yang dikirim.)
4. Mengapa penting menggunakan **HTTPS** saat berkomunikasi dengan API Telegram Bot?
5. Pada kode Tahap 3, pengiriman ThingSpeak menggunakan `millis()` (non-blocking) alih-alih `delay(20000)`. Menurut kalian, apa yang akan terjadi pada respons Telegram bot jika kita tetap memakai `delay(20000)` di dalam `loop()`?
