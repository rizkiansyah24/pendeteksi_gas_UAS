# 🚨 Sistem Deteksi Kebocoran Gas Berbasis Internet of Things (IoT)

Proyek ini merupakan tugas Mata Kuliah **Sistem Mikrokontroler** yang bertujuan membangun sistem pendeteksi kebocoran gas berbasis **Internet of Things (IoT)** menggunakan **ESP32**, **Sensor MQ-5**, dan komunikasi **MQTT**.

---

## 📖 Informasi Mata Kuliah

| Keterangan | Informasi |
|------------|------------|
| **Mata Kuliah** | Sistem Mikrokontroler |
| **Dosen Pengampu** | Muhammad Ikhwan Fathulloh, S.Kom. |

---

## 👥 Anggota Kelompok

| No | Nama | NIM |
|:--:|------|------------|
| 1 | Aldi Rizkiansyah | 23552011130 |
| 2 | Muhamad Farhan Rizki | 23552011146 |

---

# 1. Deskripsi Sistem

Sistem Deteksi Kebocoran Gas Berbasis Internet of Things (IoT) merupakan sistem monitoring yang dirancang untuk mendeteksi adanya kebocoran gas secara otomatis menggunakan **sensor MQ-5**.

Sistem menggunakan **ESP32 (Arduino Uno D1 R32)** sebagai pusat pengendali yang bertugas:

- Membaca data sensor gas
- Mengolah data sensor
- Mengendalikan aktuator LED
- Mengirimkan data ke dashboard menggunakan protokol MQTT

Selain melakukan monitoring secara **real-time**, sistem juga memberikan indikator visual menggunakan LED. Tingkat kecerahan LED dikendalikan menggunakan metode **Pulse Width Modulation (PWM)**, sedangkan fitur **Digital to Analog Converter (DAC)** bawaan ESP32 digunakan sebagai implementasi keluaran analog.

Seluruh proses dijalankan secara **multitasking** menggunakan **FreeRTOS**, sehingga pembacaan sensor, komunikasi jaringan, serta pengendalian aktuator dapat berjalan secara bersamaan.

---

# 2. Tujuan Sistem

Tujuan pembangunan sistem ini adalah:

- Mendeteksi kebocoran gas secara otomatis menggunakan sensor MQ-5.
- Mengirimkan hasil pembacaan sensor ke dashboard IoT secara real-time menggunakan MQTT.
- Memberikan indikator visual ketika terjadi kebocoran gas.
- Mengimplementasikan fitur **ADC**, **DAC**, **PWM**, dan **FreeRTOS** pada ESP32.
- Mempermudah proses monitoring kondisi gas dari jarak jauh melalui internet.

---

# 3. Manfaat Sistem

Manfaat dari sistem ini antara lain:

- Membantu mendeteksi kebocoran gas lebih cepat.
- Mengurangi risiko kebakaran akibat kebocoran gas.
- Memudahkan monitoring kondisi gas secara real-time.
- Menjadi media pembelajaran implementasi Internet of Things menggunakan ESP32.

---

# 4. Perangkat yang Digunakan

## Hardware

- ESP32 Arduino Uno D1 R32
- Sensor Gas MQ-5
- LED
- Breadboard
- Kabel Jumper
- Jaringan WiFi

## Software

- Arduino IDE
- Library WiFi
- PubSubClient
- ArduinoJson
- MQTT Broker Shiftr.io

---

# 5. Sensor yang Digunakan

Sistem menggunakan **Sensor MQ-5** untuk mendeteksi:

- LPG
- Metana (CH₄)
- Hidrogen
- Gas Alam

Sensor menghasilkan sinyal analog yang dibaca melalui **GPIO34 (ADC)**.

Rentang nilai ADC:

```
0 – 4095
```

Semakin besar nilai ADC, maka semakin tinggi konsentrasi gas yang terdeteksi.

---

# 6. Aktuator yang Digunakan

Aktuator yang digunakan berupa **LED** sebagai indikator kondisi sistem.

| Kondisi | LED |
|---------|-----|
| Aman | Berkedip redup |
| Bocor | Berkedip terang |
| Dashboard MQTT | LED dapat ON/OFF dari dashboard |

---

# 7. Cara Kerja Sistem

1. ESP32 terhubung ke jaringan WiFi.
2. ESP32 melakukan koneksi ke MQTT Broker.
3. Sensor MQ-5 membaca kadar gas setiap 1 detik.
4. Data analog dibaca menggunakan ADC.
5. Nilai sensor dibandingkan dengan batas **1800**.
6. Jika nilai < **1800**, status menjadi **AMAN**.
7. Jika nilai ≥ **1800**, status menjadi **BOCOR**.
8. Data dikirim ke dashboard menggunakan format JSON melalui MQTT.
9. LED dikendalikan menggunakan PWM.
10. DAC menghasilkan tegangan analog.
11. Dashboard dapat mengontrol LED melalui MQTT.

---

# 8. Implementasi ADC (Analog to Digital Converter)

Implementasi ADC pada sistem:

- GPIO34 digunakan sebagai pin ADC.
- Pembacaan sensor menggunakan fungsi:

```cpp
analogRead()
```

Rentang pembacaan:

```
0 - 4095
```

Nilai ADC digunakan sebagai dasar menentukan kondisi **AMAN** atau **BOCOR**.

---

# 9. Implementasi DAC (Digital to Analog Converter)

DAC digunakan pada **GPIO25**.

| Kondisi | Nilai DAC | Tegangan |
|----------|-----------|-----------|
| Aman | 127 | ±1.65 Volt |
| Bocor | 255 | ±3.3 Volt |

---

# 10. Implementasi PWM (Pulse Width Modulation)

PWM digunakan untuk mengatur tingkat kecerahan LED.

| Parameter | Nilai |
|------------|-------|
| Pin PWM | GPIO18 |
| Frekuensi | 5000 Hz |
| Resolusi | 8-bit |

Kondisi LED:

| Kondisi | Nilai PWM |
|----------|-----------|
| Aman | 30 |
| Bocor | 255 |

---

# 11. Implementasi MQTT

MQTT digunakan sebagai media komunikasi antara ESP32 dengan Dashboard IoT.

## Topik MQTT

| Topik | Fungsi |
|--------|--------|
| `gas/value` | Mengirim nilai sensor |
| `gas/status` | Mengirim status gas |
| `system/status` | Mengirim status perangkat |
| `gas/control` | Menerima kontrol LED |

Data dikirim dalam format **JSON** yang berisi:

- Nilai Sensor
- Status Gas
- Status WiFi
- Status MQTT

---

# 12. Implementasi FreeRTOS

Sistem menggunakan beberapa task:

| Task | Fungsi |
|-------|---------|
| TaskSensor | Membaca sensor dan DAC |
| TaskLED | Mengendalikan LED menggunakan PWM |
| TaskMQTT | Menjaga koneksi MQTT |
| TaskPublish | Mengirim data ke broker |
| TaskConnection | Menjaga koneksi WiFi dan MQTT |

---

# 13. Diagram Sistem

## Diagram Sistem

> Tambahkan gambar diagram sistem di sini.

```
docs/diagram-sistem.png
```

---

## Wiring Diagram

> Tambahkan gambar wiring di sini.

```
docs/wiring-diagram.png
```

---

# 14. Kesimpulan

Sistem Deteksi Kebocoran Gas Berbasis Internet of Things (IoT) berhasil dibangun menggunakan mikrokontroler **ESP32** sebagai pusat pengendali.

Sensor **MQ-5** digunakan untuk mendeteksi kadar gas melalui penerapan **ADC**, sedangkan LED sebagai aktuator dikendalikan menggunakan metode **PWM** sehingga mampu memberikan indikator visual sesuai kondisi gas.

Selain itu, fitur **DAC** bawaan ESP32 dimanfaatkan untuk menghasilkan keluaran tegangan analog sebagai implementasi Digital to Analog Converter.

Komunikasi data dilakukan menggunakan **MQTT**, sehingga hasil pembacaan sensor dapat dipantau secara **real-time** melalui dashboard IoT.

Penggunaan **FreeRTOS** memungkinkan pembacaan sensor, komunikasi jaringan, pengiriman data, serta pengendalian aktuator berjalan secara multitasking sehingga sistem menjadi lebih stabil, responsif, dan efisien.

Secara keseluruhan, proyek ini telah berhasil mengimplementasikan konsep **Internet of Things (IoT)** dengan memanfaatkan **Sensor MQ-5**, **ESP32**, **MQTT**, **ADC**, **DAC**, **PWM**, serta **FreeRTOS**.

---

# 🎥 Video Demo

Klik gambar atau tautan berikut untuk melihat demonstrasi sistem.

**https://youtu.be/L3qRGBoaEVA?si=TkvPSw7aCoV1Pw4_**

---

## ⭐ Teknologi yang Digunakan

- ESP32 Arduino Uno D1 R32
- Sensor MQ-5
- Arduino IDE
- MQTT
- Shiftr.io
- FreeRTOS
- WiFi
- ADC
- DAC
- PWM
- JSON

---

## 📄 Lisensi

Proyek ini dibuat untuk memenuhi tugas Mata Kuliah **Sistem Mikrokontroler** Program Studi Teknik Informatika.
