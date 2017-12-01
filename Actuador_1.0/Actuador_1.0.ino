#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>
//Definicion de pines para Led de colores
#define LED_ETHERNET A2
#define LED_CONEXION_SERVIDOR A1
#define LED_PETICION_HTTP A0


//Definicion de pines para el control de los reles

#define CANAL_2  18
#define CANAL_3  19
#define CANAL_4  20

// Definimos que el cliente.
EthernetClient cliente;

//Constantes de configuraciones.
const char* servidor = "192.168.1.101";
const String actuadorId = "5a1c8f1aa2b6bc067e5618e9";
const unsigned long  BAUD_RATE = 9600;
const unsigned long HTTP_TIMEOUT = 10000;
const size_t MAX_CONTENT_SIZE = 512;

//Estructura para almacenar los datos que vamos a extraer desde el json.
struct ActuadorData {
  char ubicacion[32];
  char iluminacionExterna[32];
  char canales[32];
  char canal_2[32];
  char canal_3[32];
  char canal_4[32];
  

};



void setup() {

  //Configuracion de los pines del Arduino como salida.
  pinMode(LED_ETHERNET, OUTPUT);
  pinMode(LED_CONEXION_SERVIDOR, OUTPUT);
  pinMode(LED_PETICION_HTTP, OUTPUT);
  pinMode(CANAL_2, OUTPUT);
  pinMode(CANAL_3, OUTPUT);
  pinMode(CANAL_4, OUTPUT);


  //Por defecto apagamos todos los reles.
  digitalWrite(CANAL_2, HIGH);//Luz patio
  digitalWrite(CANAL_3, HIGH);//Sala principal 1
  digitalWrite(CANAL_4, HIGH);//Sala principal 2


  //Iniciamos la comunicacion serial.
  iniciarSerial();
  //Iniciamos la conexion a la red ethernet.
  iniciarEthernet();

}

void loop() {
  if (conexion(servidor)) {
    String uri = "/actuadores/" + actuadorId;
    //pruebaReles();
    if (peticionGet(uri, servidor) && saltarResponseHeaders()) {
      ActuadorData actuadorData;
      if (leerRespuesta(&actuadorData)) {
        imprimirRespuesta(&actuadorData);
        verificaModo(&actuadorData);
        

      }

    }
  }
  desconexion();
  espera();
}
void verificaModo(struct ActuadorData* actuadorData){
  if(String(actuadorData->canal_2) == "encendido"){
    digitalWrite(CANAL_2, LOW);//Luz patio
  }
  if(String(actuadorData->canal_2) == "apagado"){
    digitalWrite(CANAL_2, HIGH);//Luz patio
  }
   if(String(actuadorData->canal_2) == "automatico"){
      String iluminacion = String(actuadorData->iluminacionExterna);
      if(iluminacion.toInt() < 200){
        digitalWrite(CANAL_2, LOW);//Luz patio
      }
      if(iluminacion.toInt() > 200){
        digitalWrite(CANAL_2, HIGH);//Luz patio
      }
      
    
  }
  if(String(actuadorData->canal_3) == "encendido"){
    digitalWrite(CANAL_3, LOW);//Luz patio
  }
  if(String(actuadorData->canal_3) == "apagado"){
    digitalWrite(CANAL_3, HIGH);//Luz patio
  }
  if(String(actuadorData->canal_4) == "encendido"){
    digitalWrite(CANAL_4, LOW);//Luz patio
  }
  if(String(actuadorData->canal_4) == "apagado"){
    digitalWrite(CANAL_4, HIGH);//Luz patio
  }
}

//Realizar una peticion HTTP GET
boolean peticionGet(String uri, char* servidor) {
  Serial.print("GET ");
  Serial.println(uri);

  cliente.print("GET ");
  cliente.print(uri);
  cliente.println(" HTTP/1.0");
  cliente.print("Host: ");
  cliente.println(servidor);
  cliente.println("Connection: close");
  cliente.println();
  analogWrite(LED_PETICION_HTTP, 255);
  delay(1000);
  analogWrite(LED_PETICION_HTTP, 0);
  return true;
}
// Saltar el HTTP headers para obtener el body de la respuesta.
bool saltarResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  cliente.setTimeout(HTTP_TIMEOUT);
  bool ok = cliente.find(endOfHeaders);

  if (!ok) {
    Serial.println("No hay respuesta o respuesta invalida!");
  }

  return ok;
}
boolean leerRespuesta(struct ActuadorData* actuadorData) {

  const size_t BUFFER_SIZE =
    JSON_OBJECT_SIZE(4)
    + JSON_OBJECT_SIZE(6)
    + MAX_CONTENT_SIZE;

  // Allocate a temporary memory pool
  DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);

  JsonObject& root = jsonBuffer.parseObject(cliente);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
    return false;
  }

  // Here were copy the strings we're interested in
  strcpy(actuadorData->ubicacion, root["ubicacion"]);
  strcpy(actuadorData->canal_2, root["modos_canales"]["canal_2"]);
  strcpy(actuadorData->canal_3, root["modos_canales"]["canal_3"]);
  strcpy(actuadorData->canal_4, root["modos_canales"]["canal_4"]);
  strcpy(actuadorData->iluminacionExterna, root["iluminacionExterna"]);
  strcpy(actuadorData->canales, root["canales"]);


  return true;
}
// Print the data extracted from the JSON
void imprimirRespuesta(struct ActuadorData* actuadorData) {
  Serial.print("Ubicacion = ");
  Serial.println(actuadorData->ubicacion);
  Serial.print("Canales disponibles = ");
  Serial.println(actuadorData->canales);
  Serial.print("Modo canal 2 = ");
  Serial.println(actuadorData->canal_2);
  Serial.print("Modo canal 3 = ");
  Serial.println(actuadorData->canal_3);
  Serial.print("Modo canal 4 = ");
  Serial.println(actuadorData->canal_4);
  Serial.print("Nivel de iluminacion externa = ");
  Serial.println(actuadorData->iluminacionExterna);

}
//Inicializa la comunicacion serial
void iniciarSerial() {
  Serial.begin(BAUD_RATE);
  while (!Serial) {
    // espera que se inicialize el puerto serial.
  }
  Serial.println("Comunicacion serial lista");
}
//Conectarse a la red ethernet
void iniciarEthernet() {
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  if (!Ethernet.begin(mac)) {
    Serial.println("Fallo de configuracion ethernet");
    return;
  }
  Serial.println("Ethernet listo");
  analogWrite(LED_ETHERNET, 255);
  delay(1000);
}
//Conectarse a un servidor HTTP
boolean conexion(const char* servidor) {
  Serial.print("Conectandose a ");
  Serial.println(servidor);
  boolean ok =  cliente.connect(servidor, 3000);
  if (ok) {
    analogWrite(LED_CONEXION_SERVIDOR, 255);
    Serial.print("Conectado a ");
    Serial.println(servidor);
  } else {
    Serial.print("Conexion fallida");
  }
  return ok;
}
//Desconectarse del servidor
void desconexion() {
  analogWrite(LED_CONEXION_SERVIDOR, 0);
  Serial.println("Desconectandose");
  cliente.stop();
}

//Pause antes de conectarse nuevamente y realizar la siguiente peticion.
void espera() {
  Serial.println("Esperando 5 segundos");
  delay(5000);
}
void pruebaReles() {
  espera();
  digitalWrite(CANAL_2, LOW);
  espera();
  digitalWrite(CANAL_3, LOW);
  espera();
  digitalWrite(CANAL_4, LOW);
  espera();
  digitalWrite(CANAL_2, HIGH);
  espera();
  digitalWrite(CANAL_3, HIGH);
  espera();
  digitalWrite(CANAL_4, HIGH);
  espera();

}



