//=============== Librerías paraa conectar el ESP8266 a Wifi ==========================
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
//=============== Librerías de Azure para comunicación ==========================
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

//=============== Información física del sensor ==========================
#define DEVICE_ID "H1"

//=============== Configuración pin layout ==========================
#define LED_PIN 5
#define smoke_ALERT 15

//========Tiempo de intervalo en ms para enviar mensajes al IoT Hub ========
#define INTERVAL 30000

//===== Si no se cuenta con el sensor fisicamente se puede usar datos simulados ===========
#define SIMULATED_DATA false
//=============== Configuración de mensajes en el IoT Hub ==========================
#define CONNECTION_STRING_LEN 256
#define MESSAGE_MAX_LEN 256

const int digital = 5;
//=============== Si algún mensaje falla vuelve a intentarlo ==========================
static bool messagePending = false;
static bool messageSending = true;
//=============== Credenciales de conexión para el IoT Hub ==========================
static char connectionString[] = "HostName=Bmobile.azure-devices.net;DeviceId=Humo;SharedAccessKey=/NO/DXpNglAy8HhpaQ6Tvtaa/46KTBlhmCOsehwgrac=";
//=============== Credenciales para conexión WiFi ==========================
static char ssid[] = "TVBmobile"; 
static char pass[] = "scanda01";

static int interval = INTERVAL;



//=============== Variables para la lectura de Humo ==========================
double smk = 0;
double b = 0;

//=================== Software Serial =====================
#include <SoftwareSerial.h>
SoftwareSerial BTserial(13, 12); // RX | TX

//=========CREAR UNA INSTANCIA DEL SENSOR HyT=============
String comandoS = "";


String lecBle;

int n = 100;

void blinkLED()
{
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

void initWifi()
{
  // Attempt to connect to Wifi network:
  LogInfo("Attempting to connect to SSID: %s", ssid);

  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    // Get Mac Address and show it.
    // WiFi.macAddress(mac) save the mac address into a six length array, but the endian may be different. The huzzah board should
    // start from mac[0] to mac[5], but some other kinds of board run in the oppsite direction.
    uint8_t mac[6];
    WiFi.macAddress(mac);
    LogInfo("You device with MAC address %02x:%02x:%02x:%02x:%02x:%02x connects to %s failed! Waiting 10 seconds to retry.",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ssid);
    WiFi.begin(ssid, pass);
    delay(10000);
  }
  LogInfo("Connected to wifi %s", ssid);
}
//****************** Método para recibir timestamp desde el servidor ***********************
void initTime()
{
  time_t epochTime;
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  while (true)
  {
    epochTime = time(NULL);

    if (epochTime == 0)
    {
      LogInfo("Fetching NTP epoch time failed! Waiting 1 seconds to retry.");
      delay(1000);
    }
    else
    {
      LogInfo("Fetched NTP epoch time is: %lu", epochTime);
      break;
    }
  }
}
//****************** Método para iniciarl el monitor serial ***********************
void initSerial()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  LogInfo("Serial successfully inited");
}

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;



void readSensor()
{
  b = readSmoke_();      //Leer valores del sensor
}

void sndBL() {
   lecBle = "s" + (String)b;
  Serial.print("Sn BL: ");
  Serial.println(lecBle);
  for (int i = 0; i < 4; i++)
    writeString(lecBle);

}

void writeString(String stringData)
{
  for (int i = 0; i < stringData.length(); i++)
  {
    BTserial.write(stringData[i]);   // Push each char 1 by 1 on each loop pass

  }
}

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Setup de Arduino\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
void setup()
{
  initSerial();
  delay(2000);

  initWifi();
  initTime();
  initIoThubClient();

  iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
  if (iotHubClientHandle == NULL)
  {
    LogInfo("Failed on IoTHubClient_CreateFromConnectionString");
    while (1);
  }

  IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
  IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
  IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);

  BTserial.begin(9600);

}

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Loop de Arduino\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
static int messageCount = 1;
void loop()
{
  if (!messagePending && messageSending)
  {
    char messagePayload[MESSAGE_MAX_LEN];
    bool smokeAlert = readMessage(messageCount, messagePayload);
    sendMessage(iotHubClientHandle, messagePayload, smokeAlert);
    messageCount++;
    delay(interval);
  }
  IoTHubClient_LL_DoWork(iotHubClientHandle);
  delay(10);

  readSensor();

  if (BTserial.find("x"))
  {
    BTserial.println("OK");
    Serial.println("OK");
    sndBL();
  }
}
