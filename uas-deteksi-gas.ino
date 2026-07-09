/*************************************************
 * PROJECT : Sistem Deteksi Kebocoran Gas IoT (Updated PWM & DAC - Fix Version)
 * MATA KULIAH : Sistem Mikrokontroler
 * SEMESTER : 6
 * BOARD : Arduino Uno D1 R32 (ESP32)
 * SENSOR : MQ-5
 * BROKER : shiftr.io
 *************************************************/

/*************************************************
 * 1. LIBRARY
 *************************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/*************************************************
 * 2. KONFIGURASI WIFI
 *************************************************/
const char* WIFI_SSID     = "Aldi Rizkiansyah";
const char* WIFI_PASSWORD = "hasilnya24";

/*************************************************
 * 3. KONFIGURASI MQTT
 *************************************************/
const char* MQTT_SERVER   = "deteksi-gas.cloud.shiftr.io";
const int   MQTT_PORT     = 1883;
const char* MQTT_USERNAME = "deteksi-gas";
const char* MQTT_PASSWORD = "doLZdHOo3Z1otXG3";
const char* MQTT_CLIENT   = "ESP32_GAS_DETECTOR_TEAM1";

/*************************************************
 * 4. MQTT TOPIC
 *************************************************/
const char* TOPIC_GAS_VALUE  = "gas/value";
const char* TOPIC_GAS_STATUS = "gas/status";
const char* TOPIC_SYSTEM     = "system/status";
const char* TOPIC_CONTROL    = "gas/control";

/*************************************************
 * 5. KONFIGURASI PIN
 *************************************************/
#define MQ5_PIN      34      // Sensor MQ-5 (ADC1_CH6)
#define LED_STATUS   18      // LED Indikator Sistem (PWM)
#define DAC_PIN      25      // Pin DAC Bawaan ESP32 (DAC_CHANNEL_1)

/*************************************************
 * 6. KONFIGURASI PWM (LEDC VERSI 3.X) ESP32
 *************************************************/
const int PWM_FREQ      = 5000;     // Frekuensi 5 KHz
const int PWM_RESOLUTION = 8;        // Resolusi 8-bit (Nilai: 0 - 255)

/*************************************************
 * 7. PARAMETER SISTEM
 *************************************************/
const int GAS_THRESHOLD = 1800;

const unsigned long SENSOR_INTERVAL = 1000;
const unsigned long MQTT_INTERVAL   = 5000;
const unsigned long LED_INTERVAL    = 1000;

/*************************************************
 * 8. VARIABEL GLOBAL
 *************************************************/
int gasValue = 0;
String gasStatus = "AMAN";

bool wifiConnected = false;
bool mqttConnected = false;
int ledBrightness = 0;       // Menggantikan bool ledStatus untuk PWM
bool ledEnable = true;

unsigned long lastSensorRead = 0;
unsigned long lastMQTTSend   = 0;
unsigned long lastBlink      = 0;

/*************************************************
 * 9. OBJECT MQTT & FREERTOS
 *************************************************/
WiFiClient espClient;
PubSubClient mqttClient(espClient);

TaskHandle_t TaskLEDHandle = NULL;
TaskHandle_t TaskSensorHandle = NULL;
TaskHandle_t TaskMQTTHandle = NULL;
TaskHandle_t TaskPublishHandle = NULL;
TaskHandle_t TaskConnectionHandle = NULL;

/*************************************************
 * 10. FUNCTION
 *************************************************/

/*==================== WiFi ====================*/
void connectWiFi()
{
    Serial.println();
    Serial.println("Menghubungkan ke WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    wifiConnected = true;

    Serial.println();
    Serial.println("WiFi Berhasil Terhubung");
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
}

/*==================== MQTT Callback ====================*/
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
    String message = "";

    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.println();
    Serial.print("Topic   : ");
    Serial.println(topic);

    Serial.print("Pesan   : ");
    Serial.println(message);

    if (message == "LED_ON")
    {
        ledEnable = true;
        Serial.println("Mode LED : BERKEDIP (PWM)");
    }
    else if (message == "LED_OFF")
    {
        ledEnable = false;
        ledcWrite(LED_STATUS, 0); // Mematikan PWM langsung ke PIN LED
        Serial.println("Mode LED : MATI");
    }
}

/*==================== MQTT ====================*/
void connectMQTT()
{
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    while (!mqttClient.connected())
    {
        Serial.println();
        Serial.println("Menghubungkan ke Broker MQTT...");

        if (mqttClient.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD))
        {
            mqttConnected = true;

            Serial.println("Broker MQTT Terhubung");

            mqttClient.subscribe(TOPIC_CONTROL);

            mqttClient.publish(TOPIC_SYSTEM, "ESP32 ONLINE");
        }
        else
        {
            mqttConnected = false;

            Serial.print("Gagal. Error : ");
            Serial.println(mqttClient.state());

            delay(5000);
        }
    }
}

/*==================== Sensor MQ-5 (ADC) ====================*/
void readGasSensor()
{
    // Penerapan ADC: Membaca nilai analog dari sensor
    gasValue = analogRead(MQ5_PIN);

    if (gasValue >= GAS_THRESHOLD)
        gasStatus = "BOCOR";
    else
        gasStatus = "AMAN";

    Serial.println("------------------------------");
    Serial.print("Nilai MQ5 (ADC) : ");
    Serial.println(gasValue);

    Serial.print("Status          : ");
    Serial.println(gasStatus);
}

/*==================== Demonstrasi DAC ====================*/
void runDACDemo()
{
    // Penerapan DAC: Mengeluarkan tegangan analog murni (0 - 255) ke DAC_PIN (GPIO25)
    if (gasStatus == "BOCOR") {
        dacWrite(DAC_PIN, 255); // Output tegangan penuh ~3.3V secara analog murni
    } else {
        dacWrite(DAC_PIN, 127); // Output tegangan setengah ~1.65V secara analog murni
    }
}

/*==================== Publish MQTT ====================*/
void publishGasData()
{
    if (millis() - lastMQTTSend >= MQTT_INTERVAL)
    {
        lastMQTTSend = millis();

        StaticJsonDocument<256> doc;

        doc["gas"] = gasValue;
        doc["status"] = gasStatus;
        doc["wifi"] = wifiConnected;
        doc["mqtt"] = mqttConnected;

        char buffer[256];

        serializeJson(doc, buffer);

        mqttClient.publish(TOPIC_GAS_VALUE, buffer);
        mqttClient.publish(TOPIC_GAS_STATUS, gasStatus.c_str());

        Serial.println();
        Serial.println("Data MQTT Terkirim");
        serializeJsonPretty(doc, Serial);
        Serial.println();
    }
}

/*==================== LED dengan PWM (Pulse Width Modulation) ====================*/
void blinkSystemLED()
{
    // Jika LED dimatikan dari Dashboard
    if (!ledEnable)
    {
        ledcWrite(LED_STATUS, 0);
        return;
    }

    // Penerapan PWM: LED berkedip dengan tingkat kecerahan dinamis berdasarkan status gas
    if (millis() - lastBlink >= LED_INTERVAL)
    {
        lastBlink = millis();

        if (ledBrightness == 0) {
            // Jika AMAN = kedip redup (30), Jika BOCOR = kedip sangat terang (255)
            ledBrightness = (gasStatus == "BOCOR") ? 255 : 30; 
        } else {
            ledBrightness = 0; // Kondisi mati saat berkedip
        }

        // Menuliskan nilai PWM langsung ke PIN LED (Sintaks ESP32 Core v3.x)
        ledcWrite(LED_STATUS, ledBrightness);
    }
}

/*************************************************
 * FREERTOS TASKS
 *************************************************/
void TaskLED(void *pvParameters)
{
    while (true)
    {
        blinkSystemLED();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void TaskSensor(void *pvParameters)
{
    while (true)
    {
        readGasSensor();
        runDACDemo(); // Dipanggil bersamaan setelah data sensor diperbarui
        vTaskDelay(pdMS_TO_TICKS(SENSOR_INTERVAL));
    }
}

void TaskMQTT(void *pvParameters)
{
    while (true)
    {
        if (!mqttClient.connected())
        {
            mqttConnected = false;
            connectMQTT();
        }
        mqttClient.loop();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void TaskPublish(void *pvParameters)
{
    while (true)
    {
        publishGasData();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskConnection(void *pvParameters)
{
    while (true)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("\nWiFi Terputus... Reconnect WiFi...");
            wifiConnected = false;
            connectWiFi();
        }

        if (!mqttClient.connected())
        {
            Serial.println("\nMQTT Terputus... Reconnect MQTT...");
            mqttConnected = false;
            connectMQTT();
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/*************************************************
 * 11. SETUP
 *************************************************/
void setup()
{
    Serial.begin(115200);

    // Setup ADC Pin
    pinMode(MQ5_PIN, INPUT);
    
    // Setup PWM versi baru (ESP32 Core v3.x) menggantikan fungsi ledcSetup lama
    ledcAttach(LED_STATUS, PWM_FREQ, PWM_RESOLUTION);
    ledcWrite(LED_STATUS, 0); // Set awal mati

    // Catatan: dacWrite() otomatis mengonfigurasi pin DAC (GPIO25), tidak perlu pinMode()

    Serial.println();
    Serial.println("======================================");
    Serial.println(" SISTEM DETEKSI KEBOCORAN GAS IoT");
    Serial.println("======================================");

    connectWiFi();
    connectMQTT();

    // Pembuatan Task FreeRTOS
    xTaskCreatePinnedToCore(TaskLED, "Task LED", 2048, NULL, 1, &TaskLEDHandle, 1);
    xTaskCreatePinnedToCore(TaskSensor, "Task Sensor", 2048, NULL, 2, &TaskSensorHandle, 1);
    xTaskCreatePinnedToCore(TaskMQTT, "Task MQTT", 4096, NULL, 3, &TaskMQTTHandle, 1);
    xTaskCreatePinnedToCore(TaskPublish, "Task Publish", 4096, NULL, 2, &TaskPublishHandle, 1);
    xTaskCreatePinnedToCore(TaskConnection, "Task Connection", 4096, NULL, 4, &TaskConnectionHandle, 1);

    Serial.println("Sistem Siap Digunakan");
    Serial.println("Semua Task Berhasil Dibuat");
}

/*************************************************
 * 12. LOOP
 *************************************************/
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
