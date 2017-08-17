
//=============== Librerías paraa conectar el ESP8266 a Wifi ==========================
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
//#include <WiFiUdp.h>
//=============== Librerías de Azure para comunicación ==========================
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

//=====================Información física del sensor==============
#define DEVICE_ID "T1"

// ===========================Valor en que se activa la alerta==============
#define LED_PIN 5
#define TEMPERATURE_ALERT 28

//==================Intervalo en que se envían mensajes al IoT Hub==========
#define INTERVAL 30000

//=============== Configuración de mensajes en el IoT Hub ==========================
#define SIMULATED_DATA false
#define CONNECTION_STRING_LEN 256
#define MESSAGE_MAX_LEN 256

//=============== Si algún mensaje falla vuelve a intentarlo ==========================
static bool messagePending = false;
static bool messageSending = true;

//=============== Credenciales de conexión para el IoT Hub ==========================
static char connectionString[] = "HostName=demoBMobile.azure-devices.net;DeviceId=T1;SharedAccessKey=ui9VhXJXJLJpq5mNJnqfPwnGhlVZ+d0hy7o8u7aNzYI=";

//=============== Credenciales de conexión WiFi==========================
static char ssid[] = "TVBmobile"; ;
static char pass[] = "scanda01";

static int interval = INTERVAL;
//=============== BLOQUE DE VARIABLES ==========================
int tempC = 0;
int humidity = 0;

//=================== Software Serial =====================
#include <SoftwareSerial.h>
SoftwareSerial BTserial(13, 12); // RX | TX

//=========CREAR UNA INSTANCIA DEL SENSOR HyT=============
String comandoS = "";
String lecturas;
String lecBle;
void blinkLED()
{
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

//****************** Método para conectar a Wifi ***********************
void initWifi()
{ 
//=============== Intento para conectar a la red WiFi===================
  LogInfo("Attempting to connect to SSID: %s", ssid);

//================ Conneción a la red WPA/WPA2===============
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {

// ==============Obtener la dirección Mac y la imprime.
// ==============WiFi.macAddress(mac) guarda la dirección mac en un arreglo,Segun la tarjeta puede cambiar el orden de los valores======
// ==============comienza con mac[0] to mac[5],pero en otro tipo de tarjetas corre en direcciones opuestas==============
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

//****************** Método para iniciar el monitor serial ***********************
void initSerial()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  LogInfo("Serial successfully inited");
}

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

//*******************Método para leer los valores del sensor*******
void readSensor()
{
  
  //=================Leer valores del sesor==========
  tempC = readTemperature_();      
  humidity = readHumidity_();       
  
  lecBle = "t" + (String)tempC + "h" + (String)humidity;
  Serial.println(lecBle);
}

void sndBL() {
  lecBle = "t" + (String)tempC + "h" + (String)humidity;
  Serial.print("Sn BL: ");
  Serial.println(lecBle);
  for (int i = 0; i <= 3; i++)
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

static int messageCount = 1;

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Loop de Arduino\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
void loop()
{
  if (!messagePending && messageSending)
  {
    char messagePayload[MESSAGE_MAX_LEN];
    bool temperatureAlert = readMessage(messageCount, messagePayload);
    sendMessage(iotHubClientHandle, messagePayload, temperatureAlert);
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
