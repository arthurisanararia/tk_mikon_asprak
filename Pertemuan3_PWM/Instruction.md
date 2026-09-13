# PERTEMUAN 3
# ESP32 PWM (Pulse Width Modulation) dengan PlatformIO

## A. TUJUAN
Tujuan praktikum kali ini adalah:
1. Memberikan pemahaman mendalam mengenai penerapan teknik Pulse Width Modulation (PWM) menggunakan mikrokontroler ESP32 (seri S3 maupun C6).
2. Mengajarkan praktikan cara mengontrol perangkat aktuator (seperti LED, Motor Servo, atau Motor DC) menggunakan sinyal PWM, yang merupakan keterampilan dasar penting dalam bidang elektronika dan robotika.
3. Praktikan diharapkan memahami konsep PWM, bagaimana PWM dapat digunakan untuk mengendalikan perangkat output, dan penerapannya dalam skenario nyata dengan variasi perangkat keras yang berbeda antar kelompok.

## B. DASAR TEORI
PWM (Pulse Width Modulation) adalah salah satu teknik modulasi dengan mengubah lebar pulsa (duty cycle) dengan nilai amplitudo dan frekuensi yang tetap. Satu siklus pulsa merupakan kondisi high kemudian berada di zona transisi ke kondisi low. Lebar pulsa PWM berbanding lurus dengan amplitudo sinyal asli yang belum termodulasi. 

Duty Cycle merupakan representasi dari kondisi logika high dalam suatu periode sinyal dan dinyatakan dalam bentuk persentase (%) dengan rentang 0% sampai 100%. Sebagai contoh, jika sinyal berada dalam kondisi high terus menerus, artinya sinyal tersebut memiliki duty cycle sebesar 100%. Jika durasi sinyal keadaan high sama dengan keadaan low, maka sinyal mempunyai duty cycle sebesar 50%.

![alt text](image.png)

Dalam praktikum ini, PWM digunakan sebagai metode yang efektif untuk mengontrol output analog dengan menggunakan sinyal digital. PWM memanfaatkan lebar pulsa sinyal untuk merepresentasikan nilai analog, sehingga memungkinkan kontrol yang presisi terhadap perangkat aktuator. Konsep ini berbasis pada prinsip bahwa perubahan durasi (lebar) pulsa dalam sebuah siklus sinyal dapat menghasilkan efek analog, seperti penyesuaian tingkat kecerahan LED, pengaturan sudut gerak motor servo, atau pengaturan kecepatan putaran motor DC. 

ESP32 menyediakan fitur PWM terintegrasi yang memudahkan implementasi teknik ini dalam berbagai aplikasi. Terdapat dua parameter penting lainnya dalam pengaturan PWM selain *duty cycle*, yaitu:

- **Frekuensi (Frequency):** Menentukan seberapa cepat satu siklus penuh sinyal (*high* dan *low*) terjadi dalam satu detik, dinyatakan dalam satuan Hertz (Hz). Frekuensi yang digunakan sangat bergantung pada karakteristik aktuator. Sebagai contoh, pengontrolan LED umumnya menggunakan frekuensi yang relatif tinggi (misal: 5000 Hz) agar kedipan cahaya (*flicker*) tidak tertangkap oleh mata manusia. Sebaliknya, perangkat seperti Motor Servo standar umumnya membutuhkan frekuensi yang spesifik dan lebih rendah (umumnya 50 Hz).
- **Resolusi (Resolution):** Menentukan tingkat kehalusan rentang kendali *duty cycle* yang dapat diatur oleh program, yang dinyatakan dalam bit (misal: 8-bit, 10-bit, 12-bit). Sebagai contoh, jika kita menggunakan resolusi 8-bit, maka kita memiliki 256 tingkatan nilai (dari $0$ hingga $2^8-1$, yaitu 0 sampai 255) untuk merepresentasikan *duty cycle* 0% hingga 100%. Semakin tinggi nilai resolusi yang digunakan, semakin presisi dan halus perubahan pergerakan atau pencahayaan aktuator yang dikendalikan.

## C. ALAT DAN BAHAN
1. **Mikrokontroler:** ESP32-S3 atau ESP32-C6
2. Kabel Micro / Type C
3. Breadboard
4. Kabel Jumper
5. **Aktuator (Sesuai yang didapatkan kelompok):**
   - LED & Resistor (220 ohm)
   - Motor Servo
   - Motor DC & Driver Motor TB6612FNG
6. **Input / Sensor Tambahan (Sesuai yang didapatkan kelompok):**
   - Potensiometer 10k
   - Push Button & Resistor (10k ohm)
   - Rotary Encoder
   - Sensor Lingkungan / IMU (BME280/SHT4x, MPU6050/BMI160/ICM-series)
   - Modul Sensor Arus (INA219)

## D. LANGKAH-LANGKAH (PEMBELAJARAN AWAL)
Sebagai pembelajaran awal sebelum mengerjakan tugas kelompok, kalian dapat mencoba kode dasar pembangkitan sinyal PWM berikut. Kode ini akan membangkitkan sinyal PWM yang secara bertahap menaikkan dan menurunkan *duty cycle*. Jika kalian menghubungkannya ke LED, efek yang akan terlihat adalah cahaya LED yang meredup dan terang secara perlahan (*fading*). Pastikan kalian menyesuaikan deklarasi pin PWM (`pwmPin`) dengan pin GPIO yang mendukung output pada ESP32-S3 atau ESP32-C6 yang kalian gunakan.

```cpp
#include <Arduino.h>

const int pwmPin = 16;
const int freq = 5000;
const int pwmChannel = 0;
const int resolution = 8;

void setup() {
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(pwmPin, pwmChannel);
}

void loop() {
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
    ledcWrite(pwmChannel, dutyCycle);
    delay(15);
  }
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
    ledcWrite(pwmChannel, dutyCycle);
    delay(15);
  }
}
```

## E. TUGAS PRAKTIKUM
1. **Eksplorasi PWM pada Aktuator Utama:**
   - **Kelompok LED:** Buatlah sistem PWM yang terdiri dari minimal 3 buah LED. Atur agar ketiga LED tersebut menyala terang dan meredup secara bergantian.
   - **Kelompok Motor Servo:** Buatlah program untuk menggerakkan lengan servo secara perlahan dan halus (tidak patah-patah) dari sudut awal ke sudut maksimal, lalu kembali lagi menggunakan modifikasi parameter sinyal PWM.
   - **Kelompok Motor DC:** Buatlah program untuk mengontrol kecepatan putaran Motor DC menggunakan Driver TB6612FNG. Atur kecepatan putaran motor secara bertahap dari lambat ke cepat, lalu kembali ke lambat.

2. **Integrasi Dinamis dengan Input:**
   Gunakan komponen input yang kelompok kalian terima (misalnya Potensiometer, Rotary Encoder, atau Push Button) untuk mengontrol nilai sinyal PWM secara dinamis oleh pengguna.
   - Contoh: Memutar potensiometer untuk mengontrol tingkat kecerahan LED / mengatur sudut posisi Servo / mengatur kecepatan Motor DC.

**Pertanyaan:**
1. Gambarkan bentuk grafik sinyal PWM ketika Duty Cycle bernilai 80%.
2. Pada program contoh pembelajaran awal, kita menggunakan frekuensi 5000 Hz yang cocok untuk LED. Menurut analisa kalian, apakah nilai frekuensi 5000 Hz ini juga bisa dan aman langsung digunakan untuk mengendalikan Motor Servo? Jelaskan alasannya berdasarkan karakteristik sinyal yang umumnya dibutuhkan oleh motor servo standar!
