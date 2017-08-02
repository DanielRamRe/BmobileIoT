//#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>

//=========Metodo de lectura del sensor de puerta=============

float readPuerta_()
{
  State = digitalRead(digital);
  if (State == 0)State = 10;
  else State = 100;
  Serial.print("    Purta: ");
  Serial.println(State);
return State;
}


bool readMessage(int messageId, char *payload)
{
  float puerta = readPuerta_();
  StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["deviceId"] = DEVICE_ID;
  root["messageId"] = messageId;
  bool puertaAlert = false;

  // NAN is not the valid json, change it to NULL
  if (std::isnan(puerta))
  {
    root["puerta"] = NULL;
  }
  else
  {
    root["puerta"] = puerta;
    if (puerta == PUERTA_ALERT)
    {
      puertaAlert = true;
      Serial.println("");
      Serial.println("puertaAlert = true");
    }
  }

  root.printTo(payload, MESSAGE_MAX_LEN);
  return puertaAlert;
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
