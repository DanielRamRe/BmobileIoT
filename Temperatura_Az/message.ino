//#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <SHT1x.h>
#include <Wire.h>
//=========CREAR UNA INSTANCIA DEL SENSOR HyT=============
SHT1x sht15(5, 15);                                      //Datos, SCK

float readTemperature_()
{
  Wire.endTransmission(false);
 int tempC = sht15.readTemperatureC();      //Leer valores del sensor
 
  Wire.endTransmission(true);
  return tempC;
}

float readHumidity_()
{
  Wire.endTransmission(false);
 int humidity = sht15.readHumidity();       //Leer valores del sensor
 Wire.endTransmission(true);
  return humidity;
}

bool readMessage(int messageId, char *payload)
{
  float temperature = readTemperature_();
  float humidity = readHumidity_();
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
  root.printTo(payload, MESSAGE_MAX_LEN);
  return temperatureAlert;
}

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
