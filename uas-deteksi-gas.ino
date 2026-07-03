/*************************************************
 * PROJECT : Sistem Deteksi Kebocoran Gas IoT
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
#define MQ5_PIN      34      // Sensor MQ-5
#define LED_STATUS   18      // LED Indikator Sistem

/*************************************************
 * 6. PARAMETER SISTEM
 *************************************************/
const int GAS_THRESHOLD = 1800;

const unsigned long SENSOR_INTERVAL = 1000;
const unsigned long MQTT_INTERVAL   = 5000;
const unsigned long LED_INTERVAL    = 1000;

/*************************************************
 * 7. VARIABEL GLOBAL
 *************************************************/
int gasValue = 0;
String gasStatus = "AMAN";

bool wifiConnected = false;
bool mqttConnected = false;
bool ledStatus = false;
bool ledEnable = true;

unsigned long lastSensorRead = 0;
unsigned long lastMQTTSend   = 0;
unsigned long lastBlink      = 0;

/*************************************************
 * 8. OBJECT MQTT
 *************************************************/
WiFiClient espClient;
PubSubClient mqttClient(espClient);

TaskHandle_t TaskLEDHandle = NULL;
TaskHandle_t TaskSensorHandle = NULL;
TaskHandle_t TaskMQTTHandle = NULL;
TaskHandle_t TaskPublishHandle = NULL;
TaskHandle_t TaskConnectionHandle = NULL;

/*************************************************
 * 9. FUNCTION
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

        Serial.println("Mode LED : BERKEDIP");
    }
    else if (message == "LED_OFF")
    {
        ledEnable = false;

        digitalWrite(LED_STATUS, LOW);

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

/*==================== Sensor MQ-5 ====================*/
void readGasSensor()
{
    gasValue = analogRead(MQ5_PIN);

    if (gasValue >= GAS_THRESHOLD)
        gasStatus = "BOCOR";
    else
        gasStatus = "AMAN";

    Serial.println("------------------------------");
    Serial.print("Nilai MQ5 : ");
    Serial.println(gasValue);

    Serial.print("Status    : ");
    Serial.println(gasStatus);
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

/*==================== LED ====================*/
void blinkSystemLED()
{
    // Jika LED dimatikan dari Dashboard
    if (!ledEnable)
    {
        digitalWrite(LED_STATUS, LOW);
        return;
    }

    // LED berkedip normal
    if (millis() - lastBlink >= LED_INTERVAL)
    {
        lastBlink = millis();

        ledStatus = !ledStatus;

        digitalWrite(LED_STATUS, ledStatus);
    }
}

void TaskLED(void *pvParameters)
{
    while (true)
    {
        blinkSystemLED();

        // Memberikan kesempatan Task lain berjalan
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*************************************************
 * TASK SENSOR
 *************************************************/
void TaskSensor(void *pvParameters)
{
    while (true)
    {
        readGasSensor();
        vTaskDelay(pdMS_TO_TICKS(SENSOR_INTERVAL));
    }
}

/*************************************************
 * TASK MQTT
 *************************************************/
void TaskMQTT(void *pvParameters)
{
    while (true)
    {
        //--------------------------------------------
        // Jika koneksi MQTT terputus
        //--------------------------------------------
        if (!mqttClient.connected())
        {
            mqttConnected = false;
            connectMQTT();
        }

        //--------------------------------------------
        // Menjalankan MQTT Client
        //--------------------------------------------
        mqttClient.loop();

        //--------------------------------------------
        // Delay 20 ms
        //--------------------------------------------
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/*************************************************
 * TASK PUBLISH
 *************************************************/
void TaskPublish(void *pvParameters)
{
    while (true)
    {
        //--------------------------------------------
        // Mengirim data sensor ke Broker MQTT
        //--------------------------------------------
        publishGasData();

        //--------------------------------------------
        // Delay 1 detik
        //--------------------------------------------
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*************************************************
 * TASK CONNECTION
 *************************************************/
void TaskConnection(void *pvParameters)
{
    while (true)
    {
        //------------------------------------------------
        // Cek WiFi
        //------------------------------------------------
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println();
            Serial.println("WiFi Terputus...");
            Serial.println("Reconnect WiFi...");

            wifiConnected = false;

            connectWiFi();
        }

        //------------------------------------------------
        // Cek MQTT
        //------------------------------------------------
        if (!mqttClient.connected())
        {
            Serial.println();
            Serial.println("MQTT Terputus...");
            Serial.println("Reconnect MQTT...");

            mqttConnected = false;

            connectMQTT();
        }

        //------------------------------------------------
        // Delay 3 detik
        //------------------------------------------------
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
/*************************************************
 * 10. SETUP
 *************************************************/
void setup()
{
    Serial.begin(115200);

    pinMode(MQ5_PIN, INPUT);
    pinMode(LED_STATUS, OUTPUT);

    digitalWrite(LED_STATUS, LOW);

    Serial.println();
    Serial.println("======================================");
    Serial.println(" SISTEM DETEKSI KEBOCORAN GAS IoT");
    Serial.println("======================================");

    connectWiFi();
    connectMQTT();
    xTaskCreatePinnedToCore(
      TaskLED,          // Nama function
      "Task LED",       // Nama Task
      2048,             // Stack Size
      NULL,             // Parameter
      1,                // Priority
      &TaskLEDHandle,   // Handle
      1                 // Core
  );

    xTaskCreatePinnedToCore(
      TaskSensor,
      "Task Sensor",
      2048,
      NULL,
      2,
      &TaskSensorHandle,
      1
  );

  xTaskCreatePinnedToCore(
    TaskMQTT,
    "Task MQTT",
    4096,
    NULL,
    3,
    &TaskMQTTHandle,
    1
);

xTaskCreatePinnedToCore(
    TaskPublish,
    "Task Publish",
    4096,
    NULL,
    2,
    &TaskPublishHandle,
    1
);

xTaskCreatePinnedToCore(
    TaskConnection,
    "Task Connection",
    4096,
    NULL,
    4,
    &TaskConnectionHandle,
    1
);

    Serial.println("Sistem Siap Digunakan");
    Serial.println("Task LED Berhasil Dibuat");
}

/*************************************************
 * 11. LOOP
 *************************************************/
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}