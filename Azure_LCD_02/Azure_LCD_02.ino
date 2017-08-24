//=============== Librerías para conectar el ESP8266 a Wifi ==========================
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

//=============== Librerías de Azure para comunicación ==========================
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

//=============== Librería para la pantalla LCD ==========================
#include <LiquidCrystal_I2C.h>// Libreria i2c LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);                       //Parametros LCD


//=============== Información física del sensor ==========================
#define DEVICE_ID "LCD0"

//=============== Valor en que se activa la alerta ==========================
#define TEMPERATURE_ALERT 28

//=============== Intervalo en que se envían mensajes al IoT Hub ==========================
#define INTERVAL 30000
static int interval = INTERVAL;

//=============== Configuración de mensajes en el IoT Hub ==========================
#define CONNECTION_STRING_LEN 256
#define MESSAGE_MAX_LEN 256

//=============== Si algún mensaje falla vuelve a intentarlo ==========================
static bool messagePending = false;
static bool messageSending = true;

//=============== Credenciales de conexión para el IoT Hun ==========================
static char connectionString[] = "HostName=Bmobile.azure-devices.net;DeviceId=LCD2;SharedAccessKey=ezd6Y3iWi4aVwpgTCi4/RksGEqxoD6rWfYXMuk/8az0=";

//=============== Credenciales para conexión WiFi ==========================
static char ssid[] = "TVBmobile";
static char pass[] = "scanda01";

//static char ssid[]="AXTEL XTREMO-4403";
//static char pass[]="03674403";

//=============== Variables para la lectura de corriente ==========================
#define ADC_BITS    10
#define ADC_COUNTS  (1<<ADC_BITS)
#define SupplyVoltage 1000
#define inPinI A0

//========Calibración para el sensor de corriente=======

double ICAL =70 ;

//========Variables del sensor de corriente=======

int sampleI, sample;
double filteredI;
double offsetI;                          //Low-pass filter output
double  sqI, sumI;         //sq = squared, sum = Sum, inst = instantaneous
int current = 0;

//=============== String a desplegar en la LCD ==========================
String lecturas;

//=============== String para definir el handle del IoT Hub ==========================
static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
static int messageCount = 1;

//****************** Método para iniciar la LCD ***********************
void initLCD()
{
  Wire.begin(2, 14);
  lcd.init();                         //Inicializando la LCD
  lcd.display();                      //Desplegar el texto (On/Off)
  lcd.clear();
  lcd.backlight();                    //Backlight (On/Off)
  lcd.setCursor(0, 0);
  lcd.print(" T  |  H  |  C  ");
  lcd.setCursor(0, 1);
  lcd.print("    |     |     ");
  Wire.endTransmission();
}

//****************** Método para imprimir en la LCD***********************
void printLCD() {
 int tempLCD = int(readTemperature_());
  int humLCD = int(readHumidity_());
  int current = readCurrSen(20);
  lecturas = (String)tempLCD + "c | " + (String)humLCD + "% | " + (String)current + " A";
  Wire.begin(2, 14);
  lcd.init();
  lcd.display();
  lcd.setCursor(0, 0);
  lcd.print(" T  |  H  |  C  ");
  lcd.setCursor(0, 1);
  lcd.print(lecturas);
  Wire.endTransmission(true);

}

//****************** Método para conectar a Wifi ***********************
void initWifi()
{
  // Intento para conectar a la red WiFi
  LogInfo("Attempting to connect to SSID: %s", ssid);

  // Conneción a la red WPA/WPA2 
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    
    // Obtener la dirección Mac y la imprime.
    // WiFi.macAddress(mac) guarda la dirección mac en un arreglo,Segun la tarjeta puede cambiar el orden de los valores.
    uint8_t mac[6];
    WiFi.macAddress(mac);
    LogInfo("La conexión del dispositivo con dirección MAC %02x:%02x:%02x:%02x:%02x:%02x a la red %s falló! Reintento en 2 segundos.",
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
  LogInfo("Inicio de monitor serial");
}

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Setup de Arduino\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
void setup()
{
  pinMode(LED_PIN, OUTPUT);
  initLCD();
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
}

///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Loop de Arduino\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void loop()
{
  printLCD();
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
}
