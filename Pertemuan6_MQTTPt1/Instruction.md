# PERTEMUAN 6
# ESP32 MQTT Bagian 1: Remote Actuator Control & Hybrid Safety Interlock

---

## A. TUJUAN
1. Memahami arsitektur protokol komunikasi MQTT (Message Queuing Telemetry Transport) pada mikrokontroler ESP32 (ESP32 DevKit V1 maupun ESP32-S3).
2. Mengonfigurasi ESP32 sebagai MQTT Subscriber untuk mengendalikan aktuator fisik (LED dan Motor Servo) secara *real-time* via HiveMQ Web Client.
3. Mengimplementasikan kendali *hybrid* yang menggabungkan kontrol jarak jauh MQTT dengan pengaman fisik lokal (Push Button atau Potensiometer 10k) disertai pelaporan status (*feedback state*).

---

## B. DASAR TEORI

MQTT adalah protokol komunikasi data ringan berbasis model **Publish/Subscribe** yang bekerja di atas protokol TCP/IP. Protokol ini sangat hemat bandwidth dan efisien untuk komunikasi perangkat IoT.

```
[ HiveMQ Web Client ]  --- Publish: "ugm/mikon/kel1/cmd" --->  [ Broker: broker.hivemq.com ]
[ HiveMQ Web Client ]  <-- Publish: "ugm/mikon/kel1/status" --  [ Broker: broker.hivemq.com ]
                                                                       |
                                                               Forward to Subscriber
                                                                       v
                                                               [ ESP32 (Subscriber) ]
```

- **Broker:** Server perantara pesan (misal: `broker.hivemq.com`).
- **Publisher:** Pengirim pesan yang mempublikasikan data ke topik tertentu.
- **Subscriber:** Penerima pesan yang mendaftarkan diri pada topik tertentu.
- **Topik:** Jalur kanal pesan bertingkat yang dipisahkan garis miring (contoh: `ugm/mikon/kel1/cmd`). Bersifat *case-sensitive*.

Penerimaan data pada ESP32 ditangani melalui fungsi callback:
```cpp
void callback(char* topic, byte* payload, unsigned int length)
```
Data `payload` diterima dalam bentuk array byte tanpa *null-terminator* (`\0`). Untuk mengolahnya sebagai teks atau angka, payload perlu disalin atau ditambahkan karakter `\0` sebelum diproses (`strcmp`, `atoi`, dll).

---

## C. ALAT DAN BAHAN

1. ESP32 DevKit V1 / ESP32-S3 (1 unit)
2. Kabel Data USB (1 pcs)
3. Breadboard & Kabel Jumper (secukupnya)
4. **Aktuator (Sesuai kelompok):**
   - Multi-LED (3 pcs) & Resistor 220 $\Omega$ (3 pcs)
   - Motor Servo (1 pcs)
5. **Input Pengaman Lokal (Sesuai kelompok):**
   - Push Button (1 pcs) & Resistor 10k $\Omega$ (1 pcs)
   - Potensiometer Rotary 10k $\Omega$ (1 pcs)
6. Laptop / PC terkoneksi internet (Browser Web)

---

## D. TABEL PEMBAGIAN KELOMPOK (10 KELOMPOK)

| Kelompok | Aktuator Utama | Input Pengaman Lokal | Peran Sistem Pengaman Lokal | Topik Kontrol (Subscribe) | Topik Status (Publish) |
|:---:|:---|:---|:---|:---|:---|
| **Kel 1** | Multi-LED (3 LED) | Push Button | Emergency All-OFF | `ugm/mikon/kel1/cmd` | `ugm/mikon/kel1/status` |
| **Kel 2** | Multi-LED (3 LED) | Push Button | Emergency All-OFF | `ugm/mikon/kel2/cmd` | `ugm/mikon/kel2/status` |
| **Kel 3** | Multi-LED (3 LED) | Push Button | Alert Strobe Override | `ugm/mikon/kel3/cmd` | `ugm/mikon/kel3/status` |
| **Kel 4** | Dual LED (PWM) | Potensiometer 10k | Brightness Ceiling Limiter | `ugm/mikon/kel4/val` | `ugm/mikon/kel4/status` |
| **Kel 5** | Dual LED (PWM) | Potensiometer 10k | Brightness Ceiling Limiter | `ugm/mikon/kel5/val` | `ugm/mikon/kel5/status` |
| **Kel 6** | Motor Servo | Push Button | Emergency Center 90° & Lock | `ugm/mikon/kel6/servo` | `ugm/mikon/kel6/status` |
| **Kel 7** | Motor Servo | Push Button | Emergency Center 90° & Lock | `ugm/mikon/kel7/servo` | `ugm/mikon/kel7/status` |
| **Kel 8** | Motor Servo | Potensiometer 10k | Local Manual vs Remote Mode | `ugm/mikon/kel8/servo` | `ugm/mikon/kel8/status` |
| **Kel 9** | Motor Servo | Potensiometer 10k | Local Manual vs Remote Mode | `ugm/mikon/kel9/servo` | `ugm/mikon/kel9/status` |
| **Kel 10**| Motor Servo | Potensiometer 10k | Safety Angle Range Limiter | `ugm/mikon/kel10/servo` | `ugm/mikon/kel10/status` |

---

## E. PANDUAN HIVEMQ WEB CLIENT

1. Buka browser dan akses: [https://www.hivemq.com/demos/websocket-client/](https://www.hivemq.com/demos/websocket-client/)
2. Pada panel **Connection**:
   - Host: `broker.hivemq.com`
   - Port: `8000`
   - Klik **Connect** (pastikan status berwarna hijau / Connected).
3. **Kirim Perintah ke ESP32 (Publish):**
   - Masukkan topik kontrol kelompok Anda di kolom **Topic**.
   - Masukkan pesan pada kolom **Message**, lalu klik **Publish**.
4. **Pantau Status dari ESP32 (Subscribe):**
   - Pada panel **Subscriptions**, klik **Add New Topic Subscription**.
   - Masukkan topik status kelompok Anda (QoS 0), lalu klik **Subscribe**.

---

## F. REFERENSI KODE PROGRAM

### 1. Program Utama (WiFi & MQTT PubSubClient)
```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(" Terhubung!");
      // Ganti topik sesuai nomor kelompok Anda:
      client.subscribe("ugm/mikon/kel1/cmd");
      client.publish("ugm/mikon/kel1/status", "ESP32 Online");
    } else {
      Serial.print(" Gagal, rc=");
      Serial.print(client.state());
      delay(3000);
    }
  }
}
```

### 2. Referensi Aktuator
**a. Multi-LED & PWM**
```cpp
const int pinLED1 = 18;
const int pinLED2 = 19;

void setupLED() {
  pinMode(pinLED1, OUTPUT);
  ledcAttach(pinLED2, 5000, 8); // PWM 5kHz, 8-bit (0-255)
}

void kontrolLED(char* pesan) {
  if (strcmp(pesan, "L1_ON") == 0) digitalWrite(pinLED1, HIGH);
  else if (strcmp(pesan, "L1_OFF") == 0) digitalWrite(pinLED1, LOW);

  // Nilai numerik PWM
  int val = atoi(pesan);
  if (val >= 0 && val <= 255) {
    ledcWrite(pinLED2, val);
  }
}
```

**b. Motor Servo**
```cpp
const int servoPin = 16;

void setupServo() {
  ledcAttach(servoPin, 50, 12); // 50 Hz, 12-bit
}

void setServo(int angle) {
  angle = constrain(angle, 0, 180);
  int duty = map(angle, 0, 180, 102, 512); // Duty cycle 0 - 180 derajat
  ledcWrite(servoPin, duty);
}
```

### 3. Referensi Input Lokal & Status Feedback
```cpp
const int pinBtn = 21; // Push button (pull-down)
const int pinPot = 34; // Pin ADC (GPIO 34 pada DevKit V1, atau GPIO 1-10 pada ESP32-S3)

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  int btn = digitalRead(pinBtn);
  int pot = analogRead(pinPot); // 0 - 4095

  // Contoh event-driven feedback
  static int lastBtn = LOW;
  if (btn != lastBtn) {
    lastBtn = btn;
    if (btn == HIGH) client.publish("ugm/mikon/kel1/status", "ALERT: EMERGENCY STOP ON");
    else client.publish("ugm/mikon/kel1/status", "STATUS: NORMAL");
  }
}
```

---

## G. TUGAS PRAKTIKUM

### Tugas 1: Kendali Diskrit via MQTT (Dasar)
1. Rangkai ESP32 dengan aktuator kelompok Anda di breadboard.
2. Buat program agar ESP32 menerima perintah teks dari HiveMQ Web Client:
   - **Kelompok LED (Kel 1, 2, 3):** Kirim perintah string untuk menyalakan/mematikan LED secara terpisah (misal: `"L1_ON"`, `"L1_OFF"`, `"ALL_ON"`, `"ALL_OFF"`).
   - **Kelompok Servo (Kel 6 & 7):** Kirim perintah posisi patokan: Sudut 0° (`"MIN"`), 90° (`"MID"`), dan 180° (`"MAX"`).
   - **Kelompok 4, 5, 8, 9, 10:** Uji penerimaan perintah ON/OFF atau posisi patokan awal aktuator.
3. Tunjukkan respon aktuator secara *real-time* kepada asisten.

### Tugas 2: Kendali Variabel via Payload Angka (Menengah)
1. Modifikasi program untuk memproses payload berupa angka (`atoi`):
   - **Kelompok LED PWM (Kel 4 & 5):** Terima angka `0` s.d. `255`. Atur intensitas 2 LED secara komplementer (LED 1 makin terang, LED 2 makin redup).
   - **Kelompok Servo (Kel 8, 9, 10):** Terima angka sudut `0` s.d. `180`. Servo bergerak halus sesuai derajat yang dikirimkan.
   - **Kelompok 1, 2, 3, 6, 7:** Tambahkan parameter angka untuk mengatur kecepatan kedipan LED atau kecepatan pergerakan servo.
2. Amati pergerakan aktuator dan monitor nilainya di Serial Monitor.

### Tugas 3: Hybrid Safety Interlock & Feedback Status (Lanjut)
Integrasikan input fisik lokal (Push Button / Potensiometer 10k) sebagai pengaman yang memiliki prioritas lebih tinggi daripada perintah MQTT:

1. **Kelompok dengan Push Button (Kel 1, 2, 3, 6, 7):**
   - **Saat Tombol Ditekan (Kondisi Darurat):** Perintah MQTT diabaikan.
     - Kel LED: Semua LED padam (atau strobo peringatan untuk Kel 3).
     - Kel Servo: Servo terkunci paksa pada sudut 90°.
     - Publish ke topik status: `"ALERT: EMERGENCY INTERLOCK ENGAGED"`.
   - **Saat Tombol Dilepas:** Kendali MQTT pulih kembali. Publish status: `"STATUS: NORMAL MODE"`.

2. **Kelompok dengan Potensiometer 10k (Kel 4, 5, 8, 9, 10):**
   - **Kel 4 & 5 (LED Brightness Limiter):** Potensio membatasi batas maksimum PWM. Jika payload MQTT melebihi nilai batas potensio, potong nilai ke batas tersebut dan publish: `"WARNING: PWM CLAMPED"`.
   - **Kel 8 & 9 (Servo Local vs Remote Selector):** 
     - Potensio < 2000 $\rightarrow$ Remote Mode (posisi dikendalikan MQTT).
     - Potensio $\ge$ 2000 $\rightarrow$ Local Manual Mode (posisi dikendalikan langsung oleh putaran potensio fisik; perintah MQTT diabaikan).
     - Publish status saat mode berganti: `"MODE: MANUAL"` atau `"MODE: REMOTE"`.
   - **Kel 10 (Servo Safety Range Limiter):** Potensio membatasi batas sudut maksimal yang boleh dicapai servo. Jika nilai MQTT melebihi batas ini, servo ditahan pada batas dan publish: `"WARNING: ANGLE CAPPED"`.

---

## H. PERTANYAAN (KUIS)
1. Mengapa protokol MQTT lebih hemat daya dan bandwidth dibanding HTTP polling dalam aplikasi kendali perangkat secara *real-time*?
2. Mengapa tombol pengaman darurat (*Safety Interlock*) wajib diproses langsung di mikrokontroler lokal dan tidak boleh hanya mengandalkan tombol pada web dashboard?
3. Apa akibatnya jika dua perangkat ESP32 menggunakan `ClientID` yang sama saat terhubung ke broker MQTT publik?
