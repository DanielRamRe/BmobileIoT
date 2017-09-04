//=============== Librerías paraa conectar el ESP8266 a Wifi ==========================
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
//=============== Librerías de Azure para comunicación ==========================
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

//=============== Información física del sensor ==========================
#define DEVICE_ID "P1"

// Pin layout configuration
#define LED_PIN 15
//=============== Valor en que se activa la alerta ==========================
#define PUERTA_ALERT 10

//=============== Intervalo en que se envían mensajes al IoT Hub ==========================
#define INTERVAL 300000

// If don't have a physical DHT sensor, can send simulated data to IoT hub
#define SIMULATED_DATA false

//=============== Configuración de mensajes en el IoT Hub ==========================
#define CONNECTION_STRING_LEN 256
#define MESSAGE_MAX_LEN 256

const int digital = 5;
//=============== Si algún mensaje falla vuelve a intentarlo ==========================
static bool messagePending = false;
static bool messageSending = true;

//=============== Credenciales de conexión para el IoT Hun ==========================
static char connectionString[] = "HostName=demoBMobile.azure-devices.net;DeviceId=P1;SharedAccessKey=Gx+vtOW9CFKKRGJ2wNHk0NkLUr39NvTMT/WjG4uVyd4=";

//=============== Credenciales para conexión WiFi ==========================
static char ssid[] = "TVBmobile"; ;
static char pass[] = "scanda01";

static int interval = INTERVAL;


int State = 0;


//=============== BLOQUE DE VARIABLES ==========================
int puertaC = 0;

//=================== Software Serial =====================
#include <SoftwareSerial.h>
SoftwareSerial BTserial(13, 12); // RX | TX

String comandoS = "";

String lecturas;

String lecBle;

//****************** Método para iniciar una conexión a internet***********************
void initWifi()
{
  // Intentar conexión a WiFi
  LogInfo("Attempting to connect to SSID: %s", ssid);

  // Conectar a WPA/WPA2
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    // Imprimir MAX
    // WiFi.macAddress(mac) guarda la dirección mac en un arreglo,Segun la tarjeta puede cambiar el orden de los valores.
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


//=====================Lectura del sensor==================
void readSensor()
{
  puertaC = readPuerta_();      //Leer valores del sensor
  
}

//=====================Enviar valor por bluetooth, ID del sensor y estado de la puerta==================
void sndBL() {
  lecBle = "p" + (String)State;
  Serial.print("Sn BL: ");
  Serial.println(lecBle);
  for (int i = 0; i < 4; i++)
    writeString(lecBle);
}

//=====================Método para enviar datos por bluetooth=========================
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
    bool puertaAlert = readMessage(messageCount, messagePayload);
    sendMessage(iotHubClientHandle, messagePayload, puertaAlert);
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
