#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
const int ledRojo =  5;
const int ledAmbar =  6;
const int ledVerde =  7;
const int pinAlarma =8;
//const char* topicName = "centro";


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };


// fill in an available IP address on your network here, as it is in your local network
// for manual configuration:
IPAddress ip(192, 168, 1, 12);


// fill in your Domain Name Server address here (usually the IP of your router):
//IPAddress myDNS(192, 168, 1, 20);
IPAddress myDNS(8,8,8,8);

// fill in your Domain Name Server address here(usually the IP of your router):
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255,255,255,0);

// char server[] = "www.thingspeak.com";//Not really needed, I left it in case I use the original Thingspeak code.
//IPAddress server(184, 106, 153, 10);

//IPAddress mqtt_server(192, 168, 1, 10);
IPAddress mqtt_server(192, 168, 1,20);
const char* topicName = "centro";
// Topic con el que trabajamos


EthernetClient ethClient;
PubSubClient client(ethClient);


LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

//char *message = "ok 51.25/22.21|s1 53.25/23.21|s2 54.25/24.21|s3 54.45/24.24|s4 44.25/44.21|";
//String myString = String(message);
String myString = ""; /*string que almacenaralos datos recibidos*/

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
 /* int i=0;
  for (i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();*/
  
  char byteArray[length];
strcpy((char *)byteArray,payload);
myString = String(byteArray);
  Serial.println(myString);
  actua();
}

void setup() {
  // put your setup code here, to run once:
//char* topic={ok 51.25/}
Serial.begin(9600);
pinMode(ledRojo, OUTPUT);
pinMode(ledAmbar, OUTPUT);
pinMode(ledVerde, OUTPUT);

/*****************************/
lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();

if ( Ethernet.begin(mac, ip,gateway,subnet) == 0) {
    Serial.println("Failed to configure Ethernet");
  }
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
/*****************************/






}

void loop() {
  
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
      Serial.print("Connecting ...");
      if (client.connect("rece_arduino")) {
        Serial.println("connected");
        client.subscribe(topicName);
      } else {
        delay(5000);
      }
  }

  // Cliente a la escucha
  client.loop();
}

void actua()
{
    String estado = myString.substring(0,2);
  Serial.println(estado);
analizaSistema(estado);
int iniTrozo=0;
int finTrozo=0;
for (int i=0;i<=4;i++)
{
  /*split parameters mqtt*/
    String trozo="";
    finTrozo=myString.indexOf('|',iniTrozo);

    trozo=myString.substring(iniTrozo,finTrozo);  
    trozo.trim();
    String title=trozo.substring(0,trozo.indexOf(' '));
    title.trim();
    String h=trozo.substring(trozo.indexOf(' ')+1,trozo.indexOf('/'));
    String t=trozo.substring(trozo.indexOf('/')+1,trozo.indexOf('|'));
    iniTrozo=finTrozo+1;

    /*print in display*/
    Serial.println(trozo);
    //Serial.println(iniTrozo);
    lcd.setCursor(1,0);lcd.print("estado:"+title);
    Serial.println("ti"+title);
    Serial.println("h:"+h);
    lcd.setCursor(1,1);
    lcd.print("T:"+t);
    delay(2000);
    lcd.setCursor(1,1);
    lcd.print("H:"+h);
    delay(2000);
    Serial.println("t"+t);
  }
  }
void analizaSistema(String estado)
{/*analiza el estado de sistema  actua, enciende led rojo,verde,amarillo y hace sonar la alarma si hay peligro
  ROJO---peligro--pl
  AMBAR--error----er
  VERDE--normal---ok
  
  */
    if(estado=="pl")
    {
        enciendeLed(HIGH,LOW,LOW);
        alarmaOnoff();
     }else if(estado=="er"){
      
      enciendeLed(LOW,HIGH,LOW);
        //hace algo cuando var es igual a 2
     }else{
      /*el default es correcto*/
      enciendeLed(LOW,LOW,HIGH);
        //si nada coincide, hace lo predeterminado
        // default es optional
            
    } 
  
}

void enciendeLed(int rojo,int ambar, int verde)
{
  
  digitalWrite(ledRojo, rojo);
  digitalWrite(ledAmbar, ambar);
  digitalWrite(ledVerde, verde);
}

void alarmaOnoff()
{/*suena la alarma de peligro*/
  int duracion=250; //Duración del sonido
int fMin=2000; //Frecuencia más baja que queremos emitir
int fMax=4000; //Frecuencia más alta que queremos emitir
int i = 0;
for (i=fMin;i<=fMax; i++)
     tone(pinAlarma, i, duracion);
  }
