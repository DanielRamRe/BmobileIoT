//=============== Librerías para el sensor de humedad y temperatura ==========================
#include <SHT1x.h>
#include <Wire.h>
//=========CREAR UNA INSTANCIA DEL SENSOR HyT=============
SHT1x sht15(5, 15);     //Datos, SCK

//=========Librería para generar una estructura Json=============
#include <ArduinoJson.h>

//****************** Método para lectura de temperatura ***********************
float readTemperature_()
{
  Wire.endTransmission(false);
 int tempC = sht15.readTemperatureC();      //Leer valores del sensor
 
  Wire.endTransmission(true);
  return tempC;
}


//****************** Método para lectura de humedad ***********************
float readHumidity_()
{
  Wire.endTransmission(false);
 int humidity = sht15.readHumidity();       //Leer valores del sensor
 Wire.endTransmission(true);
  return humidity;
}

//****************** Método para lectura de sensor de corriente ***********************
int readCurrSen(unsigned int Number_of_Samples)
{
  double Irms;
  offsetI = ADC_COUNTS >> 1;

  for (unsigned int n = 0; n < Number_of_Samples; n++)
  {
    sampleI = analogRead(inPinI);

    // Root-mean-square method current
    // 1) square current values
    sqI = filteredI * filteredI;
    // 2) sum
    sumI += sqI;
  }

  double I_RATIO = ICAL * ((SupplyVoltage / 1000.0) / (ADC_COUNTS));
  Irms = I_RATIO * sqrt(sumI / Number_of_Samples);

  //Reset accumulators
  sumI = 0;
  //--------------------------------------------------------------------------------------

  int Valor = Irms*1000;
  return Valor;
}

//****************** Método para generar el Json a mandar ***********************
bool readMessage(int messageId, char *payload)
{
  float temperature = readTemperature_();
  float humidity = readHumidity_();
  int corriente = readCurrSen(20);
  StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["deviceId"] = DEVICE_ID;
  root["messageId"] = messageId;
  bool temperatureAlert = false;

  // NAN is not the valid json, change it to NULL
  if (std::isnan(temperature))
  {
    root["temperature"] = NULL;
  }
  else
  {
    root["temperature"] = temperature;
    if (temperature > TEMPERATURE_ALERT)
    {
      temperatureAlert = true;
       Serial.println("");
       Serial.println("temperatureAlert = true");
    }
  }

  if (std::isnan(humidity))
  {
    root["humidity"] = NULL;
  }
  else
  {
    root["humidity"] = humidity;
  }
// NAN is not the valid json, change it to NULL
  if (std::isnan(corriente))
  {
    root["corriente"] = NULL;
  }
  else
  {
    root["corriente"] = corriente;
  }

  root.printTo(payload, MESSAGE_MAX_LEN);
  return temperatureAlert;
}


//****************** Método para mandar el mensaje y esperar respuesta ***********************
void parseTwinMessage(char *message)
{
  StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(message);
  if (!root.success())
  {
    LogError("parse %s failed", message);
    return;
  }

  if (root["desired"]["interval"].success())
  {
    interval = root["desired"]["interval"];
  }
  else if (root.containsKey("interval"))
  {
    interval = root["interval"];
  }
}
