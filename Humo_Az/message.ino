//#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>

//========= Metodo de lectura del sensor de humo =============

double readSmoke_()
{
  unsigned long muestra = 0;
  unsigned long tiempo = 0;
  unsigned long sumxpow2 = 0 ;
  unsigned long sumx = 0 ;
  unsigned long sumy = 0;
  unsigned long sumxy = 0;

  for (int i = 1; i <= n; i++)
  {
    muestra = analogRead(A0);
    //    Serial.print("valor:   ");
    //    Serial.print(muestra);
    delay(20);
    tiempo = (i);
    //    Serial.print("   tiempo:   ");
    //    Serial.println(tiempo);

    sumy += muestra;
    sumx += tiempo;
    sumxy += muestra * tiempo;
    sumxpow2 += pow(tiempo, 2);
  }
  //  Serial.print("   sumxpow2:   ");
  //  Serial.print(sumxpow2);
  //  Serial.print("    sumx:   ");
  //  Serial.print(sumx);
  //  Serial.print("    sumy:   ");
  //  Serial.print(sumy);
  //  Serial.print("    sumxy:   ");
  //  Serial.println(sumxy);
  //  Serial.println("");
  //  Serial.println("");

  long num1 = (n  * (sumxy));
  long num2 = (sumx * sumy);
  long num = (long(num1) - long(num2) );

  //  Serial.print("    num1 (");
  //  Serial.print(num1);
  //  Serial.print(") -   num2(");
  //  Serial.print(num2);
  //  Serial.print(") =   num  =  ");
  //  Serial.println(num);
  //  Serial.println("");
  //  Serial.println("");

  long den1 = (n * sumxpow2);
  long den2 = pow(sumx, 2);
  long den = ( long(den1) - long(den2));

  //  Serial.print("    den1 (");
  //  Serial.print(den1);
  //  Serial.print(") -   den2(");
  //  Serial.print(den2);
  //  Serial.print(") =   den  =  ");
  //  Serial.println(den);
  double m = double(num) / double(den);
  double bnum = double(sumy) - m * double(sumx);
  b = bnum / n;

  m *= 100;
//  Serial.println("   ");
//  Serial.print("m= ");
//  Serial.print(m, 5);
//  Serial.print("   ");
//  Serial.print("b= ");
//  Serial.println(b, 5);

  return m;
}


bool readMessage(int messageId, char *payload)
{
  float smoke = readSmoke_();
  StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["deviceId"] = DEVICE_ID;
  root["messageId"] = messageId;
  bool smokeAlert = false;

  // NAN is not the valid json, change it to NULL
  if (std::isnan(smoke))
  {
    root["smoke"] = NULL;
  }
  else
  {
    root["smoke"] = smoke;
    if (smoke < smoke_ALERT)
    {
      smokeAlert = true;
      Serial.println("");
      Serial.println("smokeAlert = true");
    }
  }

  root.printTo(payload, MESSAGE_MAX_LEN);
  return smokeAlert;
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
