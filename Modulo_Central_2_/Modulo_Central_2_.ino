//Librerias para Nextion scadas del gestor de librerias de arduino 
#include <Nextion.h>
#define NEXTION_PORT Serial1

#include <SoftwareSerial.h>
#define BTserial Serial2
//============= Variables que utilizaremos==========
//============= 1=tmp 2=orrint 3=prt 4=smk==========
int id2Nx [] = {10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 10, 3, 3, 3, 3, 3, 3, 1, 1, 1, 2, 2};
int fals = 0;
struct sensor {
  String nombre;
  String addr;
  String img;
  int id_N;

  int tmp;
  int tmpP;
  int hum;
  int humP;
  int smk;
  int smkP;
  int cor;
  int corP;
  int prt;
  int prtP;

};
int nSen = 2;
int id = 0;
sensor Sen[] =
{
  {"T1", "AT+CON508CB16AEE3F", "T2.pic=", 1},
  {"P1", "AT+CON508CB1699FD5", "P1.pic=", 21},
  {"H1", "AT+CON508CB1664B17", "H1.pic=", 11},
  {"C7", "AT+CON508CB166733A", "C7.pic=", 17}

};

char c = ' ';
//================asociacion de imagenes en Nextion================
Nextion nex(NEXTION_PORT);

int picT[] = {10, 8, 11}, picC[] = {12, 12, 13}, picH[] = {4, 4, 5}, picP[] = {15, 15, 14};

int valT[] = {28, 30},     valC[] = {100, 200}, valH[] = {20, 40}, valP[] = {10, 100 };

bool finnish = false;
void setup()
{
  Serial.begin(9600);
  NEXTION_PORT.begin(9600);
  nex.init();
  delay(200);
  BTserial.begin(9600);
  delay(400);
  Serial.println("AT ommansssfsf");
  delay(200);
  at("AT"); // check if working, always returns OK
  delay(200);
  at("AT");
  delay(200);
  at("AT");
//===============Comandos AT para verificar la comunicacion con bluetooth========
  at("AT+ROLE1"); // select master = central
  at("AT+RESET"); // actually more a restart than a reset .. needed after ROLE
  at("AT+SHOW1"); // include Bluetooth name in response
  at("AT+IMME1"); // "work immediately", not sure what this does
  at("AT+FILT1"); // show all BLE devices, not only HM ones
  at("AT");
  at("AT");
  at("AT");

  delay(1000); // wait a bit, NECESSARY!!
}


void loop()
{


  String PT_t = ""; //Declare and initialise the string we will send
  PT_t = "PT_t.val="; //Build the part of the string that we know

  String PT_h = ""; //Declare and initialise the string we will send
  PT_h = "PT_h.val="; //Build the part of the string that we know

  String PH_v = ""; //Declare and initialise the string we will send
  PH_v = "PH_v.val="; //Build the part of the string that we know

  String PC_v = ""; //Declare and initialise the string we will send
  PC_v = "PC_v.val="; //Build the part of the string that we know

  String PP_v = ""; //Declare and initialise the string we will send
  PP_v = "PP_v.val="; //Build the part of the string that we know


  at("AT"); // check if working, always returns OK
  delay(100);
  at("AT");
  delay(50);
  at("AT");;
//================= Bluetooth todavia no esta conectado============== 
// Serial.println("*******************not connected*************************")
  while (!BTserial.find("OK+CONNA"))
  {
    at(Sen[id].addr);
    delay(10);
  }
 //================= Bluetooth esta conectado============== 
// Serial.println("*******************connected*************************")

  while (!BTserial.find("OK"))
  {
    BTserial.println("x");
  }
  Serial.println("  OK ENCONTRADO ");


  while (finnish != true) {
    Serial.println("  FALSE ");
    c = BTserial.read();
//==================Conectado con Bluetooth de temperatura==========
//==============Establece la comunicacion recibiendo y mandado los datos============
    switch (c) {
      case 't':
        Serial.println(" ---------------------------------tmp-----------------  ");
        Sen[id].tmp = BTserial.parseInt();
        Sen[id].tmpP = cValor(Sen[id].tmp, Sen[id].tmpP, PT_t);
        Sen[id].tmp = Sen[id].tmpP;
        Serial.println("");
        Serial.print("  Val cColor          | ");
        Serial.print(Sen[id].tmp);
        Serial.print("    |     ");
        Serial.print(Sen[id].img);
        Serial.println("    |     ");
        cColor(Sen[id].tmp, Sen[id].img, picT, valT);
        break;
//================== Conectado con Bluetooth de humedad ==========
//============== Establece la comunicacion recibiendo y mandado los datos ============
      case 'h':
        Sen[id].hum = BTserial.parseInt();
        Sen[id].humP = cValor(Sen[id].hum, Sen[id].humP, PT_h);
        finnish = true;
        Serial.println("  TRUE ");
        //do something when var equals 2
 
        break;
//==================Conectado con Bluetooth de corriente ==========
//==============Establece la comunicacion recibiendo y mandado los datos============
      case 'c':
        Sen[id].cor = BTserial.parseInt();
        Sen[id].corP = cValor(Sen[id].cor, Sen[id].corP, PC_v);
        cColor(Sen[id].cor, Sen[id].img, picC, valC);
        finnish = true;
        break;
 //==================Conectado con Bluetooth de humo ==================
//==============Establece la comunicacion recibiendo y mandado los datos============
      case 's':
        Sen[id].smk = BTserial.parseInt();
        Sen[id].smkP = cValor(Sen[id].smk, Sen[id].smkP, PH_v);
        cColor(Sen[id].smk, Sen[id].img, picH, valH);
        finnish = true;
        break;
 //==================Conectado con Bluetooth de puerta ==========
//==============Establece la comunicacion recibiendo y mandado los datos============
      case 'p':
        Sen[id].prt = BTserial.parseInt();
        Sen[id].prtP = cValor(Sen[id].prt, Sen[id].prtP, PP_v);
        Sen[id].prt = Sen[id].prtP;
        cColor(Sen[id].prt, Sen[id].img, picP, valP);
        finnish = true;
        break;
      default:
        break;

    }
  }

  while (NEXTION_PORT.available()) {
    byte myChar = NEXTION_PORT.read();
    int msj = int(myChar);

    if (msj > 100 && msj < 200)
    {
      delay(3);
      byte p = NEXTION_PORT.read();
      delay(3);
      int sn;
      byte id_N = NEXTION_PORT.read();
      int typ = id2Nx[id_N];
      delay(100);
//===============manda las lecturas de los valores para actualizarlas en la Nextion=======
      switch (typ) {
        case 0:
          sn = cValor(Sen[typ].tmp, Sen[id2Nx[id_N]].tmpP, PT_t);
          sn = cValor(Sen[typ].hum, Sen[id2Nx[id_N]].humP, PT_h);
          sn = cValor(Sen[typ].tmp, Sen[id2Nx[id_N]].tmpP, PT_t);
          sn = cValor(Sen[typ].hum, Sen[id2Nx[id_N]].humP, PT_h);
          sn = cValor(Sen[typ].tmp, Sen[id2Nx[id_N]].tmpP, PT_t);
          sn = cValor(Sen[typ].hum, Sen[id2Nx[id_N]].humP, PT_h);
          sn = cValor(Sen[typ].tmp, Sen[id2Nx[id_N]].tmpP, PT_t);
          sn = cValor(Sen[typ].hum, Sen[id2Nx[id_N]].humP, PT_h);
          sn = cValor(Sen[typ].tmp, Sen[id2Nx[id_N]].tmpP, PT_t);
          sn = cValor(Sen[typ].hum, Sen[id2Nx[id_N]].humP, PT_h);
          break;
        case 1:

          sn = cValor(Sen[id2Nx[id_N]].cor, Sen[id2Nx[id_N]].corP, PC_v);
          break;
        case 2:
          sn = cValor(Sen[id2Nx[id_N]].prt, Sen[id2Nx[id_N]].prtP, PP_v);
          break;
        case 3:
          sn = cValor(Sen[id2Nx[id_N]].smk, Sen[id2Nx[id_N]].smkP, PH_v); //do something when var equals 2
          break;
        default:
          // if nothing else matches, do the default
          // default is optional
          break;
      }
    }

  }

  if ( id >= nSen)   id = 0;
  else    id++;

  finnish = false;
}



//=================se hace push a los valores anteriores ===================
void writeString(String stringData) { 

  for (int i = 0; i < stringData.length(); i++)
  {
    NEXTION_PORT.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
    Serial.write(stringData[i]);

  }
  //=======================indica a Nextion que es el final de lo que quiere mandar=====
  NEXTION_PORT.write(0xff); //We need to write the 3 ending bits to the Nextion as well
  NEXTION_PORT.write(0xff); 
  NEXTION_PORT.write(0xff);

  Serial.print("   | ");

}// end writeString function

//============Hace la lectura del valor de las variables y cambia la imagen dependiendo del valor=============
int cValor (int Valor, int j, String sends) {

  if (Valor != 0)    j = Valor;
  else    Valor = j;

  sends.concat(j);
  writeString(sends);
  return j;

}

//============Hace la lectura del valor de las variables y cambia la imagen dependiendo del valor=============
void cColor (int Valor, String cImagen, int imagenes[], int valorescompara[]) {

  if (Valor <= valorescompara[0]) {
    cImagen.concat(imagenes[0]);
  }
  else {
    if (Valor >= valorescompara[1]) {
      cImagen.concat(imagenes[2]);
    } else {
      cImagen.concat(imagenes[1]);
    }
  }
  writeString(cImagen);
}

void at(String cmd) {
  Serial.print("                                | ");
  for (int i = 0; i < cmd.length(); i++)
  {
    BTserial.write(cmd[i]);   // Push each char 1 by 1 on each loop pass
    Serial.print(cmd[i]);
  }
  Serial.println(" ..");

}


