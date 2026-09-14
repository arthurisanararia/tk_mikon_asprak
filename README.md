# Panduan Persiapan Praktikum & PlatformIO

Panduan ini berisi instruksi awal mengenai persiapan *software*, pengelolaan repositori menggunakan Git/GitHub, dan dasar penggunaan PlatformIO IDE di Visual Studio Code. Jadikan *file* ini sebagai bahan acuan selama praktikum berlangsung.

## 1. Persiapan Awal
### Pembuatan Akun GitHub
Praktikan wajib memiliki akun GitHub aktif sebelum lab dimulai. Jika belum punya, daftar dan verifikasi email untuk membuat akun GitHub.

### Instalasi Python
Mengunduh dan menginstal Python. Pastikan praktikan mencentang opsi **"Add Python to PATH"** saat instalasi di Windows.

### Instalasi Git
Instal Git (Git Bash sangat disarankan untuk Windows) agar sistem lokal memiliki Version Control System. Ini diperlukan untuk memfasilitasi perintah clone, commit, dan push. Harus melakukan konfigurasi awal `git config --global user.name` dan `user.email` agar commit tercatat dengan benar.
- **Contoh:**
  ```bash
  git config --global user.name "Nama Kalian"
  git config --global user.email "email.kalian@domain.com"
  ```
- **Verifikasi:**
  ```bash
  git config --get user.name
  git config --get user.email
  ```

### Instalasi VSCode & PlatformIO IDE
Instal teks editor Visual Studio Code, kemudian pasang ekstensi **PlatformIO IDE** melalui Extensions Marketplace. Proses ini membutuhkan koneksi internet stabil karena VSCode akan mengunduh *toolchain* dan *framework* mikrokontroler di latar belakang.

### Instalasi Driver Mikrokontroler
Instal driver spesifik agar sistem operasi mengenali mikrokontroler. Jika menggunakan board berbasis ESP32/Arduino clone, butuh driver CH340 atau CP210X.

---

## 2. Pengelolaan Repositori (Git & GitHub)

### Pembuatan Repositori & Pengaturan Kolaborator
Masing-masing membuat repository baru di GitHub.

### Clone Repo ke Local
Menyalin repositori kosong dari GitHub ke komputer menggunakan perintah `git clone <URL_REPO>`. Langkah ini mengikat folder lokal ke remote GitHub kelompok tersebut.

### Inisialisasi Project PlatformIO di Folder Repo (Langkah Tambahan)
Buka folder hasil clone di VSCode, lalu melakukan inisialisasi New Project PlatformIO (memilih board dan framework yang tepat) dan menyimpannya di dalam folder clone tersebut.

### Git Commit & Push
**1. Membuka Terminal Tepat di Lokasi Project**
   a. Buka File Explorer di Windows.
   b. Masuk ke dalam folder repositori kalian.
   c. Klik kiri satu kali di Address Bar (kolom alamat folder di bagian atas) sampai teks alamatnya terblok biru.
   d. Ketik `cmd`, lalu tekan Enter.
   e. Layar hitam Command Prompt akan terbuka, dan pastikan directory terminal sudah sesuai dengan alamat repositori kalian di lokal.

**2. Menyimpan dan Mengunggah Kode (Siklus Git)**
   Setiap kali ingin melakukan upload kode di VS Code ke GitHub, kembali ke Command Prompt tadi. Lakukan urutan 3 langkah ini:
   - **Langkah A: Menandai Perubahan (Add)**
     Ketik perintah berikut dan tekan Enter:
     ```bash
     git add .
     ```
   - **Langkah B: Memberi Label Pekerjaan (Commit)**
     Ketik perintah berikut dan tekan Enter:
     ```bash
     git commit -m "(Komen atau catatan kalian secara spesifik)"
     ```
   - **Langkah C: Mengunggah ke GitHub (Push)**
     Ketik perintah berikut dan tekan Enter:
     ```bash
     git push
     ```

---

## 3. Pemrograman di PlatformIO IDE

### 1. Membuat Project Baru
a. Klik ikon PlatformIO di sidebar kiri (ikon kepala alien).
b. Di menu Quick Access, pilih **PIO Home > Open**.
c. Klik tombol **New Project**.
d. Isi konfigurasi berikut:
   - **Name:** Nama project kalian (tanpa spasi).
   - **Board:** Ketik dan pilih board yang kita gunakan di praktikum (misal: Arduino Uno, DOIT ESP32 DEVKIT V1, atau BluePill F103C8). *Sesuaikan dengan ESP32-S3 / ESP32-C6.*
   - **Framework:** Pilih Arduino.
e. Klik **Finish**. *(Catatan: Pembuatan project pertama kali membutuhkan koneksi internet untuk mengunduh library dasar dari board yang dipilih).*

### 2. Struktur Folder Project
Kalian hanya perlu fokus pada dua bagian ini:
- **`src/main.cpp`** : Ini adalah file utama tempat kalian menulis kode (seperti ekstensi `.ino` di Arduino IDE).
- **`platformio.ini`** : Ini adalah file konfigurasi project. Kita akan menggunakannya untuk mengatur baudrate serial monitor atau menambahkan library eksternal.

### 3. Menulis Kode
a. Buka folder `src` di Explorer kiri, lalu klik `main.cpp`.
b. Hapus komentar bawaan jika perlu, lalu tulis kode kalian.
c. Berbeda dengan Arduino IDE, di PlatformIO kalian wajib menyertakan library utama di baris paling atas yang jangan kalian hapus:
   ```cpp
   #include <Arduino.h>
   ```

### 4. Konfigurasi Serial Monitor (Wajib)
Secara default, baudrate serial monitor di PlatformIO adalah 9600. Jika di kode kalian menggunakan `Serial.begin(115200)`, kalian harus menyamakannya agar output Serial Monitor tidak berupa karakter aneh.
a. Buka file `platformio.ini`.
b. Tambahkan baris ini di paling bawah:
   ```ini
   monitor_speed = 115200
   ```

### 5. Build, Upload, dan Serial Monitor
Gunakan deretan ikon kecil yang ada di Status Bar paling bawah VS Code (berwarna biru/ungu):
- ✔ **(Tanda Centang / Build):** Klik ini untuk melakukan kompilasi dan mengecek apakah ada error pada kode (tanpa mengunggah ke board).
- ➡ **(Tanda Panah Kanan / Upload):** Klik ini untuk mengompilasi sekaligus mengunggah kode ke microcontroller. Pastikan kabel USB sudah terpasang.
- 🔌 **(Ikon Steker / Serial Monitor):** Klik ini untuk membuka Serial Monitor dan melihat data komunikasi serial dari board.
