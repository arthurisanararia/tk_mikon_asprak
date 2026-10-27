# PERTEMUAN 6
# ESP32 MQTT Bagian 1: Pengendalian Aktuator & Proteksi Lokal

---

## A. TUJUAN
1. Memahami arsitektur komunikasi protokol MQTT pada ESP32 (DevKit V1 maupun S3).
2. Mengonfigurasi ESP32 sebagai MQTT Subscriber untuk mengendalikan aktuator fisik (LED / Motor Servo) melalui aplikasi desktop MQTTX dan HiveMQ Cloud Cluster.

---

## B. DASAR TEORI

MQTT adalah protokol komunikasi data ringan berbasis **Publish/Subscribe** melalui perantara server (**Broker**).
- **Broker:** Server penyalur pesan (`2d5cd8d3bcbd425a8865b60469b32931.s1.eu.hivemq.cloud`).
- **Publisher:** Pengirim data ke topik tertentu.
- **Subscriber:** Penerima data yang mendaftar pada topik tertentu.
- **Topik:** Saluran pesan hierarkis yang dipisahkan garis miring (bersifat *case-sensitive*).

```
[ Aplikasi MQTTX Windows ]  --- Publish: "ugm/mikon/kel1/cmd" --->  [ HiveMQ Cloud Cluster ]
[ Aplikasi MQTTX Windows ]  <-- Publish: "ugm/mikon/kel1/status" --  [ HiveMQ Cloud Cluster ]
                                                                             |
                                                                     Forward to Subscriber
                                                                             v
                                                                     [ ESP32 (Subscriber) ]
```

**Format Topik Kelompok (Ganti `[X]` dengan nomor kelompok, contoh: `kel1`, `kel2`):**
- Topik Perintah (Subscribe): `ugm/mikon/kel[X]/cmd`
- Topik Status (Publish): `ugm/mikon/kel[X]/status`

---

## C. ALAT DAN BAHAN

1. ESP32 DevKit V1 / ESP32-S3 (1 unit)
2. Kabel Data USB (1 pcs)
3. Breadboard & Kabel Jumper
4. **Komponen Jalur A (Kelompok LED):**
   - LED 5mm (1 pcs) & Resistor 220 $\Omega$ (1 pcs)
   - Push Button (1 pcs) & Resistor 10k $\Omega$ (1 pcs)
5. **Komponen Jalur B (Kelompok Servo):**
   - Motor Servo SG90 (1 pcs)
   - Potensiometer Rotary 10k $\Omega$ (1 pcs)
6. Laptop / PC Windows terpasang aplikasi MQTTX (Hotspot HP 2.4 GHz)

---

## D. PANDUAN APLIKASI MQTTX (WINDOWS DESKTOP)

Praktikan menggunakan aplikasi desktop **MQTTX** pada laptop Windows:

1. Buka aplikasi **MQTTX**.
2. Klik **+ New Connection**, lalu atur parameter berikut:
   - **Name:** `Kelompok [X]` (contoh: `Kelompok 1`)
   - **Protocol:** Pilih **`mqtts://`**
   - **Host:** `2d5cd8d3bcbd425a8865b60469b32931.s1.eu.hivemq.cloud`
   - **Port:** `8883`
   - **SSL/TLS:** **ON (Hijau)**
   - **SSL Secure:** **OFF (Abu-abu / Tidak Aktif)**
   - **MQTT Version:** `3.1.1` (pada menu *Advanced*)
   - **Username:** *(Diberikan oleh Asisten)*
   - **Password:** *(Diberikan oleh Asisten)*
3. Klik tombol **Connect** di pojok kanan atas hingga status berwarna hijau (**Connected**).
4. **Kirim Perintah ke ESP32 (Publish):**
   - Masukkan topik: `ugm/mikon/kel[X]/cmd`
   - Masukkan pesan pada kolom bawah, lalu klik ikon panah kirim.
5. **Pantau Status dari ESP32 (Subscribe):**
   - Klik **+ New Subscription**
   - Masukkan topik: `ugm/mikon/kel[X]/status`, lalu klik **Confirm**.

---

## E. TUGAS PRAKTIKUM: JALUR A (KELOMPOK LED)

---

### TAHAP 1: Kendali Diskrit ON/OFF

**1. Rangkaian Hardware:**
- Anoda LED $\rightarrow$ **GPIO 18** (melalui Resistor 220 $\Omega$).
- Katoda LED $\rightarrow$ **GND**.

**2. Kode Program Tugas 1:**

```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid = "NAMA_HOTSPOT_HP";
const char* password = "PASSWORD_HOTSPOT";

const char* mqtt_server = "2d5cd8d3bcbd425a8865b60469b32931.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "USERNAME_DARI_ASISTEN";
const char* mqtt_pass = "PASSWORD_DARI_ASISTEN";

const char* topic_cmd = "ugm/mikon/kel1/cmd";      // SESUAIKAN KELOMPOK
const char* topic_status = "ugm/mikon/kel1/status";

const int pinLED = 18;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  char message[32];
  if (length >= sizeof(message)) length = sizeof(message) - 1;
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Pesan masuk: ");
  Serial.println(message);

  if (strcmp(message, "ON") == 0) {
    digitalWrite(pinLED, HIGH);
  } else if (strcmp(message, "OFF") == 0) {
    digitalWrite(pinLED, LOW);
  }
}

void setup_wifi() {
  Serial.print("Koneksi WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Koneksi MQTT...");
    String clientId = "ESP32-LED-Kel-" + String(random(1000, 9999));
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" Terhubung!");
      client.subscribe(topic_cmd);
      client.publish(topic_status, "Tugas 1: Ready");
    } else {
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  digitalWrite(pinLED, LOW);

  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
```

**3. Pengujian Tugas 1:**
- Buka aplikasi MQTTX.
- Publish `"ON"` $\rightarrow$ LED menyala.
- Publish `"OFF"` $\rightarrow$ LED mati.

---

### TAHAP 2: Modifikasi ke Kendali PWM Dimming

**1. Modifikasi Kode:**
Gunakan rangkaian dari Tahap 1. Modifikasi program Tugas 1:

1. Tambahkan konfigurasi PWM sebelum `setup()`:
```cpp
const int pwmChannel = 0;
const int pwmFreq = 5000;
const int pwmResolution = 8;
```

2. Ganti `pinMode(pinLED, OUTPUT);` di `setup()` menjadi:
```cpp
ledcSetup(pwmChannel, pwmFreq, pwmResolution);
ledcAttachPin(pinLED, pwmChannel);
ledcWrite(pwmChannel, 0);
```

3. Ubah fungsi `callback()` menjadi:
```cpp
void callback(char* topic, byte* payload, unsigned int length) {
  char message[32];
  if (length >= sizeof(message)) length = sizeof(message) - 1;
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Pesan masuk: ");
  Serial.println(message);

  if (strcmp(message, "ON") == 0) {
    ledcWrite(pwmChannel, 255);
  } else if (strcmp(message, "OFF") == 0) {
    ledcWrite(pwmChannel, 0);
  } else {
    int val = atoi(message);
    if (val >= 0 && val <= 255) {
      ledcWrite(pwmChannel, val);
    }
  }
}
```

**2. Pengujian Tugas 2:**
- Publish angka (contoh: `"50"`, `"128"`, `"255"`).
- Pastikan tingkat kecerahan LED berubah sesuai nilai.

### TAHAP 3: Integrasi Emergency Stop & Status Feedback

**1. Penambahan Hardware:**
- Pasang Push Button ke **GPIO 21** dengan resistor pull-down 10k $\Omega$ ke **GND**, dan kaki lainnya ke **3V3**.

**2. Modifikasi Kode Tugas 3:**

1. Tambahkan variabel pengaman:
```cpp
const int pinBtn = 21;
bool isEmergency = false;
int currentPWM = 0;
```

2. Tambahkan inisialisasi pin di `setup()`:
```cpp
pinMode(pinBtn, INPUT);
```

3. Tambahkan proteksi di awal fungsi `callback()`:
```cpp
if (isEmergency) {
  Serial.println("Emergency Stop aktif!");
  return;
}
```

4. Tambahkan pembacaan tombol dan pengiriman status di `loop()`:
```cpp
int btn = digitalRead(pinBtn);
static int lastBtn = LOW;

if (btn != lastBtn) {
  lastBtn = btn;
  if (btn == HIGH) {
    isEmergency = true;
    ledcWrite(pwmChannel, 0);
    client.publish(topic_status, "ALERT: EMERGENCY STOP ON");
  } else {
    isEmergency = false;
    ledcWrite(pwmChannel, currentPWM);
    client.publish(topic_status, "STATUS: NORMAL");
  }
}
```

**3. Pengujian Tugas 3:**
- Kirim `"200"` $\rightarrow$ LED menyala.
- Tekan tombol $\rightarrow$ LED mati seketika dan muncul notifikasi `"ALERT: EMERGENCY STOP ON"`.
- Kirim pesan baru saat tombol ditekan $\rightarrow$ Perintah diabaikan.
- Lepas tombol $\rightarrow$ Status `"STATUS: NORMAL"` muncul dan LED menyala kembali.

---

## F. TUGAS PRAKTIKUM: JALUR B (KELOMPOK SERVO)

---

### TAHAP 1: Kendali Posisi Patokan Diskrit

**1. Rangkaian Hardware:**
- Sinyal Servo (Oranye/Kuning) $\rightarrow$ **GPIO 16**.
- VCC Servo (Merah) $\rightarrow$ **5V / VIN**.
- GND Servo (Cokelat/Hitam) $\rightarrow$ **GND**.

**2. Kode Program Tugas 1:**

```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid = "NAMA_HOTSPOT_HP";
const char* password = "PASSWORD_HOTSPOT";

const char* mqtt_server = "2d5cd8d3bcbd425a8865b60469b32931.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "USERNAME_DARI_ASISTEN";
const char* mqtt_pass = "PASSWORD_DARI_ASISTEN";

const char* topic_cmd = "ugm/mikon/kel6/cmd";      // SESUAIKAN KELOMPOK
const char* topic_status = "ugm/mikon/kel6/status";

const int servoPin = 16;
const int servoChannel = 0;
const int servoFreq = 50;
const int servoResolution = 12;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setServo(int angle) {
  angle = constrain(angle, 0, 180);
  int duty = map(angle, 0, 180, 102, 512);
  ledcWrite(servoChannel, duty);
}

void callback(char* topic, byte* payload, unsigned int length) {
  char message[32];
  if (length >= sizeof(message)) length = sizeof(message) - 1;
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Pesan masuk: ");
  Serial.println(message);

  if (strcmp(message, "MIN") == 0) {
    setServo(0);
  } else if (strcmp(message, "MID") == 0) {
    setServo(90);
  } else if (strcmp(message, "MAX") == 0) {
    setServo(180);
  }
}

void setup_wifi() {
  Serial.print("Koneksi WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Koneksi MQTT...");
    String clientId = "ESP32-Servo-Kel-" + String(random(1000, 9999));
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" Terhubung!");
      client.subscribe(topic_cmd);
      client.publish(topic_status, "Tugas 1: Ready");
    } else {
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  ledcSetup(servoChannel, servoFreq, servoResolution);
  ledcAttachPin(servoPin, servoChannel);
  setServo(90);

  setup_wifi();
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
```

**3. Pengujian Tugas 1:**
- Buka aplikasi MQTTX.
- Publish `"MIN"` $\rightarrow$ Servo bergerak ke 0°.
- Publish `"MID"` $\rightarrow$ Servo bergerak ke 90°.
- Publish `"MAX"` $\rightarrow$ Servo bergerak ke 180°.
---

### TAHAP 2: Modifikasi ke Kendali Sudut Bebas

**1. Modifikasi Kode:**
Gunakan rangkaian dari Tahap 1. Modifikasi fungsi `callback()`:

```cpp
void callback(char* topic, byte* payload, unsigned int length) {
  char message[32];
  if (length >= sizeof(message)) length = sizeof(message) - 1;
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.print("Pesan masuk: ");
  Serial.println(message);

  if (strcmp(message, "MIN") == 0) {
    setServo(0);
  } else if (strcmp(message, "MID") == 0) {
    setServo(90);
  } else if (strcmp(message, "MAX") == 0) {
    setServo(180);
  } else {
    int angle = atoi(message);
    if (angle >= 0 && angle <= 180) {
      setServo(angle);
    }
  }
}
```

**2. Pengujian Tugas 2:**
- Publish angka sembarang (contoh: `"30"`, `"45"`, `"135"`).
- Pastikan servo bergerak ke sudut yang diminta.

---

### TAHAP 3: Integrasi Potensiometer & Manual Override

**1. Penambahan Hardware:**
- Kaki luar Potensiometer $\rightarrow$ **3V3** dan **GND**.
- Kaki tengah Potensiometer $\rightarrow$ Pin ADC (**GPIO 34** untuk DevKit V1, atau **GPIO 4** untuk S3).

**2. Modifikasi Kode Tugas 3:**

1. Tambahkan deklarasi pin dan variabel:
```cpp
const int pinPot = 34;
bool isManual = false;
int currentAngle = 90;
```

2. Tambahkan proteksi di awal fungsi `callback()`:
```cpp
if (isManual) {
  Serial.println("Mode Manual aktif!");
  return;
}
```

3. Tambahkan pembacaan ADC dan pemilih mode di `loop()`:
```cpp
int pot = analogRead(pinPot);
bool manualTrigger = (pot >= 2000);
static bool lastManualState = false;

if (manualTrigger != lastManualState) {
  lastManualState = manualTrigger;
  isManual = manualTrigger;

  if (isManual) {
    client.publish(topic_status, "MODE: MANUAL OVERRIDE");
  } else {
    client.publish(topic_status, "MODE: REMOTE MQTT");
    setServo(currentAngle);
  }
}

if (isManual) {
  int manualAngle = map(pot, 2000, 4095, 0, 180);
  setServo(manualAngle);
  delay(15);
}
```

**3. Pengujian Tugas 3:**
- Kirim `"45"` $\rightarrow$ Servo berada di 45°.
- Putar potensiometer melebihi nilai tengah (ADC $\ge$ 2000) $\rightarrow$ Muncul pesan `"MODE: MANUAL OVERRIDE"`.
- Putar potensiometer $\rightarrow$ Servo mengikuti putaran tangan.
- Kirim pesan dari MQTTX $\rightarrow$ Perintah diabaikan.
- Putar potensiometer kembali ke bawah $\rightarrow$ Muncul `"MODE: REMOTE MQTT"` dan kendali aplikasi kembali aktif.
