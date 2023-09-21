
float number;

// INCLUSÃO DE BIBLIOTECAS
#include <HX711.h>
#include <SPIFFS.h>
#include <BluetoothSerial.h>
#include <ArduinoJson.h>
// DEFINIÇÕES DE PINOS
#define pinDT  16
#define pinSCK  4


// INSTANCIANDO OBJETOS
HX711 scale;
BluetoothSerial SerialBT;


// DECLARAÇÃO DE VARIÁVEIS
char botao;
//Água
float medida = 0;
float medidaaux = 0;
float copo =0;
float vetor[2] ={0,0};
float vetoraux[2] = {0,0};
float bebido =0;
float atual = 0;
float agua = 0;
float cafe = 0;
float offset =  0;
float refrigerante = 0;
String ref_calibration;
String function;
String liquido = "agua";
float aux =0;
int memoria;

//Café

void setup() {
  SerialBT.begin("Smart Coaster");
  Serial.begin(115200);

    if (!SPIFFS.begin(true)) {
    Serial.println("Falha ao montar sistema de arquivos SPIFFS.");
    return;
  }

  scale.begin(pinDT, pinSCK); // CONFIGURANDO OS PINOS DA BALANÇA
  scale.set_scale(691.159771275); // LIMPANDO O VALOR DA ESCALA

  delay(2000);
  scale.tare(); // ZERANDO A BALANÇA PARA DESCONSIDERAR A MASSA DA ESTRUTURA


  agua = atof(readSPIFFSFile("/agua.txt").c_str());
  cafe = atof(readSPIFFSFile("/cafe.txt").c_str());
  refrigerante = atof(readSPIFFSFile("/refrigerante.txt").c_str());
  offset =  atof(readSPIFFSFile("/offset.txt").c_str());
  
}

void loop() {
  
  String x = SerialBT.readString(); // Variavel que recebe os dados bluetooth
  if (x != ""){
  const size_t bufferSize = JSON_ARRAY_SIZE(2) + 2*JSON_STRING_SIZE(10);
  
  char json[bufferSize];
  x.toCharArray(json, bufferSize);

  StaticJsonDocument<bufferSize> doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    Serial.print("Falha na análise JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Extraindo os valores do o json recebido
  const char* func = doc[0];
  const char* valor = doc[1];
  function = String(func);  //função selecionada no aplicativo
  ref_calibration = String(valor);
  if (function != "calibration"){
  liquido = String(valor); // liquido escolhido no aplicativo
  }
  // Imprima os valores
  Serial.print("func: ");
  Serial.println(func);
  Serial.print("valor: ");
  Serial.println(valor);
  
  }

  if(function == "calibration"){ // função de calibração das medições
    Serial.println("Entrou para calibração");
    float calibration_value = ref_calibration.toInt();
    offset = calibration_value;
    saveToSPIFFS("/offset.txt", String(offset)); 
    function = "";
  }

  if(liquido == "agua"){
    Serial.println("Líquido Selecionado agua");
    get_values(0.997,&agua);
    saveToSPIFFS("/agua.txt", String(abs(agua)));
  }
    
  if(liquido == "cafe"){
  Serial.println("Líquido Selecionado: Cafe ");
  get_values(1.190,&cafe);
  saveToSPIFFS("/cafe.txt", String(abs(cafe)));  
  }  

  if(liquido == "refrigerante"){
  Serial.println("Liquido Selecionado: Refrigerante ");
  get_values(1.026,&refrigerante);
  saveToSPIFFS("/refrigerante.txt", String(abs(refrigerante)));     
  } 

   Serial.print("Consumido (Água): ");
   Serial.print(abs(agua), 3);
   Serial.println(" mL");

    Serial.print("Consumido (Cafe): ");
   Serial.print(abs(cafe), 3);
   Serial.println(" mL");

   Serial.print("Consumido (Refrigerante): ");
   Serial.print(abs(refrigerante), 3);
   Serial.println(" mL");


  // Enviou dos valores de consumo para o aplicativo
   SerialBT.print(atual);
   SerialBT.print(";");
   SerialBT.print(agua);
   SerialBT.println(";");
   SerialBT.print(cafe);
   SerialBT.println(";");
   SerialBT.print(refrigerante);
   SerialBT.println(";");     


} 

// Função responsável pela coleta e estabilização dos valores
void get_values(float d, float *l){
  
  Serial.println("Density: " + String(d,3));  
  float medida = scale.get_units(10); // Salva a média de 10 médidas coletadas pelo sensor
  
  medida = medida + offset; // insere o valor de offset selecionado no aplicativo
  Serial.println("Medida atual: " + String(medida));

   // método de estabilização das medidas
   if(abs(medida) < 1){
     medida = 0;
   }
   if (medida >= 0){   
    vetoraux[1] = vetoraux[0];
    vetoraux[0] = medida;
   if ( abs((vetoraux[0] - vetoraux[1])) < 2){
    medidaaux = medida;     
   }   
   }

  if (medidaaux >= 0){   
  vetor[1] = vetor[0];
  vetor[0] = medidaaux;
 if (vetor[0] < vetor[1]-5){

  aux= vetor[1] - vetor[0];
  Serial.println("agua antes" + String(agua));
  *l = (*l + aux*d); //ponteiro que aponta para o valor da variável global do líquido escolhido
  delay(100);
 }
  }
  atual = vetor[1];
    
  scale.power_down(); // DESLIGA O SENSOR
  delay(1000); // AGUARDA 1 SEGUNDOS
  scale.power_up(); // LIGA O SENSOR
  
}


//Função responsável por ler os dados armazenados nas SPIFFS
String readSPIFFSFile(String filename) {
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "";
  }

  String content = file.readString();

  file.close();
  return content;
}


//Função responsável por salvar os dados medidos nas SPIFFS
void saveToSPIFFS(String fileName, String data) {
  File file = SPIFFS.open(fileName, "w");
  if (!file) {
    Serial.println("Não foi possível abrir o arquivo para escrita.");
    return;
  }
  file.print(data);
  file.close();
  Serial.println("Dados salvos com sucesso na memória SPIFFS.");
}
