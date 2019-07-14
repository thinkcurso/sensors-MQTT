/*
Cliente lector de sensores de CPD

*/



#include "DHT.h"
#include <SPI.h>
#include "Ethernet.h"
#include "PubSubClient.h"



byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};// in case you have the addresss printed on the ethernet shield, just use your own mac address.

// fill in an available IP address on your network here, as it is in your local network
// for manual configuration:
IPAddress ip(192, 168, 1, 11);


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


EthernetClient ethClient;
PubSubClient client(ethClient);

#define DHTPIN1 2     // what digital pin we're connected to. Don't use the ones used by Ethernet shield e.g for UNO 10, 11, 12 and 13 or SPI
#define DHTPIN2 3
#define DHTPIN3 4
#define DHTPIN4 5
/* init number of sensors */
#define NSENSORS 4
/*arrays of temperature and humidity parameters from sensor */
float temperaturas[NSENSORS];
float humedades[NSENSORS];

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
/*init sensors */
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);
DHT dht4(DHTPIN4, DHTTYPE);

/*array of sensors for better process */
DHT dht[NSENSORS]={dht1,dht2,dht3,dht4};
// Topic con el que trabajamos
const char* topicName = "centro";

void setup()
{  /* init pin sensors */
    //mirar pinMode(13, OUTPUT); // just for a blink of the LED to make sure the  sketch has run ok.
  /*begin fixed IP and server MQTT*/
  Ethernet.begin(mac, ip,gateway,subnet);
  Serial.println("connecting...");
  client.setServer(mqtt_server, 1883);
  /*int de sensors array*/
  for (int i = 0; i < NSENSORS; i++)
  {
    dht[i].begin();
    humedades[i]=0;
    temperaturas[i]=0;
  }
  /*begin serial port*/
  Serial.begin(9600);
  Serial.println(F("Cliente Toma Datos!"));
  
}

void loop()
{
if (!client.connected()) {
    Serial.print("Connecting ...\n");
    client.connect("Arduino Client");
  }
  else {
  /* begin parameters */
          String datos="";
          float datosT=0;
          float datosH=0;
          String envio="";
          
               /* reading parameters into arrays  */
          int errores=0;
          for (int i = 0; i < NSENSORS; i++)
          {
              
              /*Read temperature and humidity from sensors and storage in array*/
                humedades[i] = dht[i].readHumidity();
          // Read temperature as Celsius 
                temperaturas[i] = dht[i].readTemperature();
        /* if read is good process or error */      
                if (isnan(humedades[i]) || isnan(temperaturas[i])) 
                {
                    //Serial.println("Fallo en el sensor " +(String)i);
                    /*mark sensor error to send*/
                    datos=datos+"s"+(String)(i+1)+" "+"0/0|";
                    errores++;
                }else{
                    
                    //Serial.println("h"+(String)(i+1)+" "+ (String)humedades[i])+"/");
                    //Serial.println("t"+(String)(i+1)+" "+(String)temperaturas[i]+"|");
                    /*almaceno datos para enviar*/
                    datosT=datosT+temperaturas[i];
                    datosH=datosH+humedades[i];
                    datos=datos+"s"+(String)(i+1)+" "+(String)humedades[i]+"/"+(String)temperaturas[i]+"|";
                } 
           }
        /*String with the state of system+%+data of sensor 
        Format """"   state averageH/averageT|Sensor/humidity|temperature|.............  """""
        */
            
           envio=comprobarEstado(errores)+" "+(String)(datosH/NSENSORS)+"/"+(String)(datosT/NSENSORS)+"|"+datos;
           Serial.println(envio);
           /*Convert String to char array to MQTT send */
           char charBuffer[envio.length()];
       
           //Serial.println("longitud"+(String)envio.length());          
           envio.toCharArray(charBuffer,envio.length());
           /*send data to MQTT sever*/
           client.publish(topicName,charBuffer);
           
  }
  /*sleep time*/
  delay(5000);
}




/*functions*/
String comprobarEstado (int err){
  /*test state of errors in system*/
  String estado="ok";
  if( err>2)
  {
          /*estado de peligro*/
      estado="er";
    
    }else if (err==0)
    {
      /*dont change, all is ok*/
    }else{
    /*estado de peligro*/
      estado="pl";
    }
    return estado;
}
/**********************************************************************/
