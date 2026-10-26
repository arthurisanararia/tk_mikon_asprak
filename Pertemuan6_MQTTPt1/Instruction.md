# PERTEMUAN 6
# ESP32 MQTT Bagian 1: Remote Actuator Control & Hybrid Safety Interlock

---

## A. TUJUAN
Tujuan praktikum pertemuan ke-6 ini adalah:
1. Praktikan mampu memahami arsitektur komunikasi data protokol **Message Queuing Telemetry Transport (MQTT)** serta mengimplementasikannya pada mikrokontroler ESP32 (mendukung ESP32 DevKit V1 standar maupun ESP32-S3).
2. Praktikan mampu merancang dan mengembangkan proyek IoT kendali jarak jauh secara *real-time* dengan mengonfigurasi ESP32 sebagai **MQTT Subscriber**, serta mengintegrasikannya dengan aktuator fisik dan sistem proteksi lokal (*Safety Interlock*).

> [!NOTE]
> **Batasan Materi Pertemuan 6:** Praktikum ini berfokus pada **penerimaan instruksi kendali (Subscriber) pada aktuator dan pelaporan status proteksi fisik lokal**. Pengiriman telemetri periodik dari sensor lingkungan (seperti suhu dan kelembapan) akan dipelajari secara khusus pada Pertemuan 7.

---

## B. DASAR TEORI

### 1. Arsitektur Komunikasi MQTT
**MQTT (Message Queuing Telemetry Transport)** adalah protokol komunikasi berbasis standar ISO/IEC 20922 yang dirancang khusus untuk jaringan dengan *bandwidth* terbatas, latensi tinggi, atau perangkat berspesifikasi daya rendah (*resource-constrained*). Protokol ini bekerja di atas lapisan transport TCP/IP dan mengadopsi pola arsitektur **Publish/Subscribe**.

```
+------------------------+         Publish: "ugm/mikon/kel1/cmd"          +-----------------------+
|   HiveMQ Web Client    | ---------------------------------------------> |      MQTT Broker      |
| (Browser PC / Laptop)  | <--------------------------------------------- |  (broker.hivemq.com)  |
+------------------------+         Publish: "ugm/mikon/kel1/status"       +-----------------------+
                                                                                      |
                                                                                      | Forward Message
                                                                                      v
                                                                          +-----------------------+
                                                                          |     ESP32 Client      |
                                                                          | (Subscriber Aktuator) |
                                                                          +-----------------------+
```

Berbeda dengan protokol HTTP yang menggunakan arsitektur *Request-Response* (klien harus terus-menerus melakukan *polling* ke server untuk mengetahui perubahan data), pada MQTT:
- **Broker:** Bertindak sebagai peladen (server) pusat yang mengelola penerimaan, penyaringan, dan penerusan seluruh pesan ke klien yang berhak.
- **Publisher:** Klien yang memancarkan/mengirimkan data dengan label topik tertentu.
- **Subscriber:** Klien yang mendaftarkan minatnya pada suatu topik tertentu. Ketika ada pesan baru yang diterbitkan pada topik tersebut, broker secara proaktif (*push-notification*) langsung meneruskan pesan ke subscriber tanpa jeda *polling*.

### 2. Topik (Topics) dan Aturan Penamaan
Topik pada MQTT berbentuk jalur hierarkis berbasis teks yang dipisahkan oleh tanda garis miring (`/`), serupa dengan struktur direktori. 
- **Case-Sensitive:** Topik bersifat sensitif terhadap huruf besar dan kecil. Topik `ugm/mikon/kelompok1/cmd` berbeda dengan `ugm/mikon/Kelompok1/CMD`.
- **Hierarki Ruang Lingkup (Namespace):** Karena praktikum menggunakan *public broker*, setiap kelompok wajib menyertakan identitas kelompok pada hierarki topiknya untuk menghindari tabrakan data antar kelompok (misal: `ugm/mikon/kel[X]/...`).

### 3. Struktur Callback dan Penanganan Payload pada PubSubClient
Ketika ESP32 menerima pesan dari broker, library `PubSubClient` memicu fungsi `callback()`:
```cpp
void callback(char* topic, byte* payload, unsigned int length)
```
> [!IMPORTANT]
> Karakter dalam *payload* MQTT **tidak diakhiri dengan null terminator (`\0`)** secara otomatis. Jika Anda ingin memperlakukan payload sebagai string C/C++ standar untuk fungsi pembanding (`strcmp`, `String()`, `atoi()`), Anda wajib menambahkan karakter `\0` di akhir array penyimpan atau menyalinnya ke buffer terpisah.

### 4. Konsep Hybrid Control & Safety Interlock
Pada implementasi industri nyata (Industrial IoT/Cyber-Physical Systems), mengandalkan kendali aktuator 100% dari jaringan nirkabel/cloud sangat berisiko. Gangguan koneksi internet, server *down*, atau latensi tinggi dapat berakibat fatal apabila aktuator mekanik terus bergerak tanpa pengawasan langsung di lapangan.

Oleh karena itu, sistem IoT modern mengadopsi konsep **Hybrid Interlock**:
1. **Cloud Command Priority (Normal Mode):** Selama kondisi aman, mikrokontroler patuh terhadap perintah yang dikirimkan melalui dashboard MQTT.
2. **Local Safety Override (Safe Mode):** Input fisik langsung di sisi mikrokontroler (misalnya tombol darurat *E-Stop* atau batas aman potensiometer) memiliki hak veto absolut. Jika input pengaman aktif, sistem secara instan mengunci aktuator pada posisi aman dan mengabaikan instruksi dari MQTT hingga kondisi normal kembali.
3. **Status Reporting:** Mikrokontroler mem-publish pesan status ke topik umpan balik (`status`) agar operator di dashboard mengetahui bahwa sistem sedang dalam kondisi terkunci lokal (*Hardware Interlock Active*).

---

## C. ALAT DAN BAHAN

| No | Nama Komponen / Perangkat | Spesifikasi / Keterangan | Jumlah per Kelompok |
|:--:|:---|:---|:--:|
| 1 | Development Board | ESP32 DevKit V1 (ESP-WROOM-32) atau ESP32-S3 | 1 Unit |
| 2 | Kabel Koneksi USB | Kabel Data USB Type-C / Micro USB | 1 Pcs |
| 3 | Breadboard | Full-size / Half-size 830/400 Point | 1 Pcs |
| 4 | Kabel Jumper | Male-to-Male & Male-to-Female | Secukupnya |
| 5 | **Aktuator Utama** (Sesuai penugasan kelompok): | | |
|   | - Multi-LED & Resistor | LED 5mm (Merah, Kuning, Hijau) + Resistor 220 $\Omega$ | 2-3 Pcs |
|   | - Motor Servo | Micro Servo SG90 / Standard Servo | 1 Pcs |
| 6 | **Input Lokal Pengaman** (Sesuai penugasan kelompok): | | |
|   | - Push Button & Resistor | Push Button 4-pin + Resistor Pull-Down 10k $\Omega$ | 1 Set |
|   | - Potensiometer | Potensiometer Rotary 10k $\Omega$ | 1 Pcs |
| 7 | Dashboard MQTT | Browser PC / Laptop / HP (HiveMQ Web Client) | 1 Perangkat |

---

## D. TABEL PEMBAGIAN PERANGKAT & TOPIK (10 KELOMPOK)

Untuk memastikan seluruh alat di laboratorium terdistribusi secara seimbang serta mencegah interferensi sinyal di broker publik, praktikan dibagi menjadi 10 kelompok dengan variasi aktuator (LED dan Servo) serta input lokal (Push Button dan Potensiometer 10k):

| Kelompok | Aktuator Utama | Komponen Input Lokal | Peran Sistem Proteksi Lokal | Topik Kendali (Subscribe) | Topik Feedback (Publish) |
|:---:|:---|:---|:---|:---|:---|
| **Kelompok 1** | Multi-LED (3 LED Diskrit) | Push Button | Emergency All-OFF Interlock | `ugm/mikon/kel1/cmd` | `ugm/mikon/kel1/status` |
| **Kelompok 2** | Multi-LED (3 LED Diskrit) | Push Button | Emergency All-OFF Interlock | `ugm/mikon/kel2/cmd` | `ugm/mikon/kel2/status` |
| **Kelompok 3** | Multi-LED (3 LED Diskrit) | Push Button | Alert Strobe Override Mode | `ugm/mikon/kel3/cmd` | `ugm/mikon/kel3/status` |
| **Kelompok 4** | LED PWM Dimming (2 LED) | Potensiometer 10k | Brightness Ceiling Limiter | `ugm/mikon/kel4/val` | `ugm/mikon/kel4/status` |
| **Kelompok 5** | LED PWM Dimming (2 LED) | Potensiometer 10k | Brightness Ceiling Limiter | `ugm/mikon/kel5/val` | `ugm/mikon/kel5/status` |
| **Kelompok 6** | Motor Servo | Push Button | Emergency Center (90°) & Lock | `ugm/mikon/kel6/servo` | `ugm/mikon/kel6/status` |
| **Kelompok 7** | Motor Servo | Push Button | Emergency Center (90°) & Lock | `ugm/mikon/kel7/servo` | `ugm/mikon/kel7/status` |
| **Kelompok 8** | Motor Servo | Potensiometer 10k | Local Potensio vs Remote Mode | `ugm/mikon/kel8/servo` | `ugm/mikon/kel8/status` |
| **Kelompok 9** | Motor Servo | Potensiometer 10k | Local Potensio vs Remote Mode | `ugm/mikon/kel9/servo` | `ugm/mikon/kel9/status` |
| **Kelompok 10**| Motor Servo | Potensiometer 10k | Dynamic Safety Range Limiter | `ugm/mikon/kel10/servo` | `ugm/mikon/kel10/status` |

---

## E. PANDUAN PENGGUNAAN MQTT BROKER & DASHBOARD

Praktikum ini menggunakan **HiveMQ Web Client** melalui peramban web (*browser*), sehingga praktikan tidak perlu mengunduh atau memasang aplikasi tambahan pada ponsel.

### Langkah Konfigurasi HiveMQ Web Client:
1. Buka peramban web pada PC atau Laptop Anda, lalu akses:  
   👉 [https://www.hivemq.com/demos/websocket-client/](https://www.hivemq.com/demos/websocket-client/)
2. Pada panel **Connection**:
   - **Host:** `broker.hivemq.com`
   - **Port:** `8000` (Port WebSocket Non-TLS)
   - **Client ID:** Biarkan terisi otomatis (*random*)
   - Klik tombol **Connect**. Pastikan indikator status berubah menjadi warna hijau (**Connected**).
3. **Mengirim Perintah ke ESP32 (Publish):**
   - Masuk ke panel **Publish**.
   - Pada kolom **Topic**, masukkan topik kendali kelompok Anda (contoh: `ugm/mikon/kel1/cmd` atau `ugm/mikon/kel4/val`).
   - Pada kolom **Message**, ketikkan instruksi kendali (misal: `"L1_ON"` atau nilai angka `"180"`).
   - Klik tombol **Publish**.
4. **Menerima Laporan Status dari ESP32 (Subscribe):**
   - Masuk ke panel **Subscriptions**.
   - Klik tombol **Add New Topic Subscription**.
   - Masukkan topik status kelompok Anda (contoh: `ugm/mikon/kel1/status`), pilih QoS `0`.
   - Klik **Subscribe**. Pesan umpan balik (*acknowledgement*, peringatan *interlock*, dan konfirmasi mode) dari ESP32 akan tampil di panel **Messages**.

---

## F. REFERENSI KODE PROGRAM

### 1. Template Dasar Koneksi Wi-Fi & MQTT (PubSubClient)
Pastikan library `PubSubClient` telah terpasang di PlatformIO (`lib_deps = knolleary/PubSubClient`) atau di Library Manager Arduino IDE.

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

// Kredensial WiFi (Gunakan Hotspot Smartphone yang stabil)
const char* ssid = "NAMA_WIFI_HOTSPOT";
const char* password = "PASSWORD_WIFI";

// Konfigurasi Broker MQTT Publik
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung! IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Mencoba koneksi ke MQTT Broker...");
    // Client ID acak agar tidak saling tendang di broker publik
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println(" Berhasil Terhubung!");
      
      // Subscribe ke topik kelompok masing-masing (SESUAIKAN NOMOR KELOMPOK)
      client.subscribe("ugm/mikon/kel1/cmd");
      
      // Kirim pesan salam online awal
      client.publish("ugm/mikon/kel1/status", "ESP32 Online & Siap Menerima Perintah");
    } else {
      Serial.print(" Gagal, rc=");
      Serial.print(client.state());
      Serial.println(". Coba lagi dalam 4 detik...");
      delay(4000);
    }
  }
}
```

### 2. Referensi Kontrol Aktuator

#### a. Multi-LED & PWM Dimming
```cpp
const int pinLED1 = 18;
const int pinLED2 = 19;

void setupLED() {
  pinMode(pinLED1, OUTPUT);
  // Setup PWM pada LED2 (Frekuensi 5kHz, Resolusi 8-bit: 0-255)
  ledcAttach(pinLED2, 5000, 8);
}

void handleLEDCommand(char* message) {
  // Parsing instruksi diskrit
  if (strcmp(message, "L1_ON") == 0) {
    digitalWrite(pinLED1, HIGH);
  } else if (strcmp(message, "L1_OFF") == 0) {
    digitalWrite(pinLED1, LOW);
  }
  
  // Parsing nilai numerik PWM (misal payload: "180")
  int pwmVal = atoi(message);
  if (pwmVal >= 0 && pwmVal <= 255) {
    ledcWrite(pinLED2, pwmVal);
  }
}
```

#### b. Motor Servo (PWM 50 Hz)
```cpp
const int servoPin = 16;

// Motor Servo standar membutuhkan sinyal PWM frekuensi 50 Hz
// Resolusi 12-bit (0 - 4095) menghasilkan rentang duty cycle ~102 hingga ~512
void setupServo() {
  ledcAttach(servoPin, 50, 12);
}

void setServoAngle(int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;
  // Konversi rentang sudut (0 - 180 derajat) ke rentang duty cycle (102 - 512)
  int duty = map(angle, 0, 180, 102, 512);
  ledcWrite(servoPin, duty);
}
```

### 3. Referensi Integrasi Input Lokal & Two-Way Feedback
```cpp
const int pinButton = 21;    // Push button dengan resistor pull-down 10k
const int pinPot = 34;       // Pin ADC (Gunakan GPIO 34 pada ESP32 DevKit V1, atau GPIO 1-10 pada ESP32-S3)

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Membaca kondisi input lokal
  int btnState = digitalRead(pinButton);
  int potValue = analogRead(pinPot); // Pembacaan ADC 12-bit (0 - 4095)

  // Contoh event-driven status publish saat tombol darurat aktif
  static int lastBtnState = LOW;
  if (btnState != lastBtnState) {
    lastBtnState = btnState;
    if (btnState == HIGH) {
      client.publish("ugm/mikon/kel1/status", "ALERT: EMERGENCY STOP ACTIVATED");
    } else {
      client.publish("ugm/mikon/kel1/status", "STATUS: NORMAL OPERATION RESTORED");
    }
  }
}
```

---

## G. TUGAS PRAKTIKUM

Praktikum terdiri atas **3 tahapan tugas terstruktur** yang disusun dari tingkat dasar hingga tingkat lanjut (*advanced*). Kerjakan tugas sesuai penugasan kelompok Anda!

### TUGAS 1: Eksplorasi Dasar — Remote Actuator Control (MQTT Subscriber)
**Tujuan:** Membangun koneksi dasar ESP32 ke broker MQTT dan mengendalikan aktuator fisik secara diskrit (perintah teks/string sederhana).

**Langkah & Ketentuan:**
1. Rangkai ESP32 dengan aktuator yang didapatkan kelompok Anda di atas breadboard.
2. Buat program agar ESP32 menghubungkan diri ke Wi-Fi dan me-subscribe topik kendali kelompok Anda (`ugm/mikon/kel[X]/...`).
3. Buat implementasi logika pada fungsi `callback()`:
   - **Kelompok LED Diskrit (Kel 1, 2, 3):** 
     Kirimkan instruksi string dari HiveMQ Web Client untuk menyalakan dan mematikan masing-masing LED secara independen (misal: `"L1_ON"`, `"L1_OFF"`, `"L2_ON"`, `"L2_OFF"`, `"ALL_ON"`, `"ALL_OFF"`).
   - **Kelompok Motor Servo (Kel 6 & 7):** 
     Kirimkan instruksi posisi diskrit dari dashboard untuk mengarahkan lengan servo ke 3 sudut patokan: Sudut 0° (kirim `"MIN"`), Sudut 90° (kirim `"MID"`), dan Sudut 180° (kirim `"MAX"`).
   - **Kelompok Dimmer / Kontrol Variabel (Kel 4, 5, 8, 9, 10):**
     Ujilah penerimaan perintah kendali awal dengan mengaktifkan aktuator ke kondisi *State ON* dan *State OFF* sesuai penugasan kelompok.
4. Ujilah pengiriman pesan dari **HiveMQ Web Client** dan tunjukkan kepada asisten bahwa aktuator merespons perintah secara *real-time*.

---

### TUGAS 2: Kendali Bertingkat — Parameterized & Continuous Control
**Tujuan:** Mengontrol parameter fisik aktuator secara kontinu/halus (*variable control*) menggunakan payload numerik dari HiveMQ Web Client.

**Langkah & Ketentuan:**
1. Modifikasi program pada Tugas 1 agar dapat menerima payload berupa data angka (integer).
2. Lakukan konversi array payload menjadi tipe numerik (`atoi()` atau sejenisnya) dan validasikan rentang nilainya.
3. Terapkan fungsionalitas kendali variabel sesuai kelompok Anda:
   - **Kelompok LED PWM Dimming (Kel 4 & 5):**
     Terima data numerik rentang `0` hingga `255` dari HiveMQ Web Client. Atur intensitas kecerahan 2 buah LED menggunakan sinyal PWM. LED 1 diatur proporsional terhadap nilai payload ($0 \rightarrow 255$), sedangkan LED 2 diatur berkebalikan ($255 \rightarrow 0$) sehingga ketika LED 1 meredup, LED 2 justru semakin terang.
   - **Kelompok Motor Servo (Kel 8, 9, 10):**
     Terima data numerik rentang sudut `0` hingga `180` dari HiveMQ Web Client. Motor servo harus bergerak secara mulus (*smooth transition*) menuju target sudut yang ditentukan tanpa sentakan kasar.
   - **Kelompok 1, 2, 3, 6, 7:**
     Terapkan kendali parameter numerik tambahan pada aktuator Anda (misal: kecepatan kedipan strobo LED berbasis milidetik atau kecepatan pergerakan *sweep* servo).
4. Amati pergerakan dan respons aktuator melalui Serial Monitor dan alat fisik.

---

### TUGAS 3: Tingkat Lanjut (Advanced) — Hybrid Safety Interlock & Two-Way Status Reporting
**Tujuan:** Mengintegrasikan input lokal (Push Button / Potensiometer 10k) sebagai mekanisme proteksi fisik (*Local Safety Override*) yang memiliki hak veto di atas perintah cloud MQTT, disertai pelaporan status (*event-driven feedback*) kembali ke dashboard.

> [!CAUTION]
> **Batasan Materi Pertemuan 6:** Praktikum ini **tidak menggunakan sensor telemetri lingkungan** (seperti pembacaan suhu & kelembapan BME280/DHT11 secara periodik) karena materi tersebut secara khusus akan dipelajari pada Pertemuan 7. 
> Fokus pengiriman pesan (`client.publish()`) pada tugas ini murni bersifat **Event-Driven Feedback** (pelaporan perubahan status sistem aktuator dan notifikasi pengaman fisik).

**Skenario Sistem Pengaman Sesuai Kelompok:**

#### 1. Untuk Kelompok dengan Input Push Button (Kelompok 1, 2, 3, 6, 7):
Komponen Push Button berfungsi sebagai **Hardware Emergency Stop (E-Stop) / Safety Lockout**:
- **Kondisi Normal (Button Terbuka/LOW):** 
  Sistem beroperasi dalam mode normal. ESP32 sepenuhnya merespons perintah kendali dari HiveMQ Web Client (Tugas 1 & 2 berjalan normal).
- **Kondisi Darurat (Button Ditekan/HIGH):** 
  Sistem seketika beralih ke mode `LOCAL_OVERRIDE_ACTIVE`.
  - **Kelompok LED (Kel 1 & 2):** Seluruh LED padam seketika (Shut-down).
  - **Kelompok LED (Kel 3):** Seluruh LED masuk ke mode *Alert Strobe* (berkedip cepat secara mandiri sebagai penanda bahaya).
  - **Kelompok Servo (Kel 6 & 7):** Motor servo langsung terkunci paksa pada posisi netral aman (sudut 90°) dan menolak segala perintah rotasi baru dari MQTT.
- **Pengiriman Status MQTT (Two-Way Communication):**
  Saat transisi terjadi, ESP32 wajib secara otomatis mengirimkan pesan konfirmasi ke topik status (`ugm/mikon/kel[X]/status`):
  - Saat tombol ditekan: Publish `"ALERT: EMERGENCY INTERLOCK ENGAGED - ACTUATOR LOCKED"`.
  - Saat tombol dilepas: Publish `"STATUS: SYSTEM RESET - MQTT CONTROL RESTORED"`.
  - Segala perintah kendali yang masuk ke topik `cmd`/`servo` selama tombol ditekan harus **ditolak/diabaikan** oleh program.

#### 2. Untuk Kelompok dengan Input Potensiometer 10k (Kelompok 4, 5, 8, 9, 10):
Komponen Potensiometer 10k berfungsi sebagai **Local Governor / Dynamic Safety Limiter**:
- **Kelompok LED Dimming (Kel 4 & 5 - Brightness Ceiling Limiter):**
  Nilai pembacaan potensiometer 10k (ADC 0 - 4095 di-map ke rentang PWM 0 - 255) bertindak sebagai batas atas (*ceiling limit*) intensitas cahaya lokal. Jika pengguna di dashboard MQTT meminta nilai kecerahan 250, namun potensiometer fisik di meja lab sedang diputar pada posisi batas 100, maka output kecerahan LED tidak boleh melebihi 100! 
  Jika nilai MQTT melampaui batas potensiometer, ESP32 mem-publish pesan peringatan: `"WARNING: PWM CLAMPED TO LOCAL LIMIT: [NILAI_LIMIT]"`.
- **Kelompok Servo (Kel 8 & 9 - Hybrid Manual vs Remote Mode Selector):**
  Potensiometer 10k digunakan untuk memilih mode operasi:
  - Jika nilai potensiometer berada di bawah nilai tengah (ADC < 2000), sistem berada dalam **Remote MQTT Mode** (sudut dikontrol dari HiveMQ Web Client).
  - Jika nilai potensiometer diputar melampaui ambang batas (ADC $\ge$ 2000), sistem beralih ke **Local Manual Mode**. Sudut servo dikontrol langsung oleh putaran potensiometer fisik, dan instruksi dari dashboard diabaikan.
  - ESP32 mem-publish status saat terjadi peralihan mode: `"MODE CHANGED: LOCAL MANUAL CONTROL"` atau `"MODE CHANGED: REMOTE MQTT CONTROL"`.
- **Kelompok Servo (Kel 10 - Dynamic Safety Range Limiter):**
  Potensiometer 10k mengatur batas sudut maksimal (*safety angle clamp*) yang boleh dicapai servo di lapangan. Berapapun sudut besar yang diminta dari HiveMQ, pergerakan servo dibatasi tidak melampaui batas putaran potensiometer. ESP32 mengirimkan peringatan jika terjadi pemangkasan sudut: `"WARNING: SERVO ANGLE CAPPED AT [NILAI] DEG"`.

---

## H. PERTANYAAN DAN TUGAS ANALISIS (KUIS)

Jawablah pertanyaan-pertanyaan berikut secara analitis dan sertakan dalam laporan resmi praktikum:

1. **Analisis Arsitektur Jaringan:**
   Jelaskan perbedaan mendasar antara mekanisme komunikasi berbasis *HTTP Request-Response Polling* dengan *MQTT Event-Driven Callback* dalam konteks efisiensi daya mikrokontroler dan utilisasi *bandwidth* jaringan saat mengendalikan aktuator secara *real-time*!
2. **Keandalan dan Keselamatan Sistem (Safety Interlock):**
   Pada perancangan sistem fisik siber (Cyber-Physical System / Industri 4.0), mengapa mekanisme *Hardware Safety Interlock* (seperti yang kalian implementasikan pada Tugas 3) **wajib** diproses secara lokal langsung pada mikrokontroler, dan tidak boleh hanya mengandalkan tombol darurat virtual pada aplikasi dashboard cloud? Analisislah skenario kegagalan jaringan yang mendasari alasan tersebut!
3. **Analisis Protokol & Penanganan Client ID:**
   Jelaskan apa konsekuensi teknis yang terjadi pada sisi MQTT Broker dan ESP32 apabila dua praktikan dari kelompok yang berbeda secara tidak sengaja menggunakan `ClientID` yang persis sama saat menghubungkan diri ke broker publik! Mengapa pada program referensi kita menggunakan fungsi pembangkit bilangan acak (`random()`) untuk penamaan `ClientID`?
