#include "DHT.h"                // pustaka untuk sensor DHT
#include "WiFi.h"               // pustaka untuk koneksi WiFi
#include "PubSubClient.h"       // pustaka untuk MQTT

// Definisi pin
#define PIN_HIJAU 5             // LED Hijau pada pin 5
#define PIN_KUNING 16           // LED Kuning pada pin 16
#define PIN_MERAH 19            // LED Merah pada pin 19
#define PIN_RELAY 17            // Relay Pompa (terhubung dengan pompa (LED Biru)) pada pin 17
#define PIN_BUZZER 4            // Buzzer pada pin 4
#define PIN_DHT 18              // Pin DHT sensor suhu dan kelembaban pada pin 18
#define DHT_TYPE DHT22          // Menentukan tipe sensor DHT yang digunakan, yaitu DHT22

// Inisialisasi sensor DHT
DHT dht(PIN_DHT, DHT_TYPE);     

// Konfigurasi WiFi dan MQTT
const char* ssid = "Wokwi-GUEST";             // jaringan WiFi yang digunakan
const char* pass = "";                        // Password WiFi
const char* mqttServer = "broker.hivemq.com"; // Alamat server MQTT
const int mqttPort = 1883;                    // Port untuk server MQTT
const char* topic_temperature = "suhukika014"; // Topik MQTT untuk mengirim data suhu
const char* topic_humidity = "sensorhumiditykika014"; // Topik MQTT untuk mengirim data kelembaban
const char* topic_relay = "aktuatorrelaykika014";     // Topik MQTT untuk mengirim status relay

WiFiClient espClient;             // Membuat klien WiFi
PubSubClient client(espClient);   // Membuat klien MQTT menggunakan WiFi

void setup() {
  Serial.begin(9600);             
  Serial.println("Hello Kika, IOT UTS 152022014"); 
  pinMode(PIN_HIJAU, OUTPUT);     // pin LED hijau sebagai output
  pinMode(PIN_KUNING, OUTPUT);    // pin LED kuning sebagai output
  pinMode(PIN_MERAH, OUTPUT);     // pin LED merah sebagai output
  pinMode(PIN_RELAY, OUTPUT);     // pin relay sebagai output
  pinMode(PIN_BUZZER, OUTPUT);    // pin buzzer sebagai output

  dht.begin();                    // Memulai sensor DHT
  setup_wifi();                   // fungsi untuk menghubungkan WiFi
  client.setServer(mqttServer, mqttPort); // Mengatur server MQTT dan port
}

void setup_wifi() {
  delay(10);                      // Memberi jeda 10 ms
  Serial.println();
  Serial.print("Connecting to WiFi..."); // Menampilkan pesan sedang menghubungkan ke WiFi
  WiFi.begin(ssid, pass);         // Menghubungkan ke jaringan WiFi

  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);                   // Jeda 500 ms
    Serial.print(".");            
  }

  Serial.println("WiFi connected"); // pesan jika berhasil terhubung
}

void reconnect() {
  Serial.println("Tunggu sebentar ya Kika..."); 

  while (!client.connected()) {    // Selama belum terhubung ke MQTT
    Serial.print("Attempting MQTT connection..."); // Menampilkan pesan mencoba koneksi MQTT

    if (client.connect("ESP32Client")) { // Mencoba terhubung ke server MQTT dengan ID klien "ESP32Client"
      Serial.println("Connected to MQTT broker"); // Menampilkan pesan jika berhasil terhubung
    } else {
      Serial.println("Yah gagal, tunggu sebentar ya dicoba lagi Kika..."); // Menampilkan pesan gagal terhubung
      delay(5000);                   // Menunggu 5 detik sebelum mencoba lagi
    }
  }
}

void loop() {
  if (!client.connected()) {        // Jika belum terhubung ke server MQTT
    reconnect();                    // Panggil fungsi reconnect untuk mencoba terhubung
  }
  client.loop();                    // Memastikan koneksi tetap aktif

  float humidity = dht.readHumidity();   // Membaca kelembaban dari sensor DHT
  float temperature = dht.readTemperature(); // Membaca suhu dari sensor DHT

  if (isnan(humidity) || isnan(temperature)) { // Jika pembacaan sensor tidak valid
    Serial.println("Error: Tidak dapat membaca dari sensor DHT!"); // Menampilkan pesan error
    return;                     // Keluar dari fungsi loop()
  }

  Serial.print("Temperature Kika: "); // Menampilkan teks suhu di serial
  Serial.print(temperature);          // Menampilkan nilai suhu
  Serial.print(" °C  ");              // Menampilkan satuan suhu
  Serial.print("Humidity Kika: ");    // Menampilkan teks kelembaban di serial
  Serial.print(humidity);             // Menampilkan nilai kelembaban
  Serial.println(" %");               // Menampilkan satuan kelembaban

  String relayStatus;                 // Variabel untuk menyimpan status relay

  if (temperature > 35) {             // Jika suhu di atas 35 °C
    digitalWrite(PIN_HIJAU, LOW);     // Matikan LED hijau
    digitalWrite(PIN_KUNING, LOW);    // Matikan LED kuning
    digitalWrite(PIN_MERAH, HIGH);    // Nyalakan LED merah
    tone(PIN_BUZZER, 100);            // Nyalakan buzzer dengan frekuensi 100 Hz
    Serial.println("Buzzer: Bunyi");  // Menampilkan status buzzer
  } else if (temperature >= 30 && temperature <= 35) { // Jika suhu antara 30 - 35 °C
    digitalWrite(PIN_HIJAU, LOW);     // Matikan LED hijau
    digitalWrite(PIN_KUNING, HIGH);   // Nyalakan LED kuning
    digitalWrite(PIN_MERAH, LOW);     // Matikan LED merah
    noTone(PIN_BUZZER);               // Matikan buzzer
    Serial.println("Buzzer: Tidak Bunyi"); // Menampilkan status buzzer
  } else {                            // Jika suhu di bawah 30 °C
    digitalWrite(PIN_HIJAU, HIGH);    // Nyalakan LED hijau
    digitalWrite(PIN_KUNING, LOW);    // Matikan LED kuning
    digitalWrite(PIN_MERAH, LOW);     // Matikan LED merah
    noTone(PIN_BUZZER);               // Matikan buzzer
    Serial.println("Buzzer: Tidak Bunyi"); // Menampilkan status buzzer
  }

  if (humidity < 30 && temperature > 35) { // Jika kelembaban kurang dari 30% dan suhu lebih dari 35 °C
    digitalWrite(PIN_RELAY, HIGH);         // Nyalakan relay
    relayStatus = "ON - Pompa Nyala";      // Set status relay ke "ON - Pompa Nyala"
    Serial.println("Relay: ON, Pompa (LED): Nyala"); // Menampilkan status relay
  } else {                                 // Jika kondisi tidak terpenuhi
    digitalWrite(PIN_RELAY, LOW);          // Matikan relay
    relayStatus = "OFF - Pompa Mati";      // Set status relay ke "OFF - Pompa Mati"
    Serial.println("Relay: OFF, Pompa (LED): Mati"); // Menampilkan status relay
  }

  // Mengirim data suhu melalui MQTT dengan konversi ke string menggunakan dtostrf
  char tempStr[8];
  dtostrf(temperature, 1, 2, tempStr);   // Konversi nilai suhu ke string dengan 2 desimal
  client.publish(topic_temperature, tempStr); // Kirim data suhu ke topik MQTT

  // Mengirim data kelembaban melalui MQTT dengan konversi ke string menggunakan dtostrf
  char humStr[8];
  dtostrf(humidity, 1, 2, humStr);       // Konversi nilai kelembaban ke string dengan 2 desimal
  client.publish(topic_humidity, humStr); // Kirim data kelembaban ke topik MQTT

  client.publish(topic_relay, relayStatus.c_str()); // Kirim status relay ke topik MQTT

  delay(2000);                         // Jeda 2 detik sebelum pembacaan berikutnya
}
