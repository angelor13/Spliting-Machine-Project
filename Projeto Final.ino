// Incluir Bibliotecas a usar ao longo do Programa:
#include <Servo.h> // Biblioteca do Servo Motor

// Definição de Pinos a usar ao longo do Progama:
// -> Pinos dos Servo Motores:
#define pin_servo_patch 9
#define pin_servo_color 10
#define pin_servo_down 11
#define pin_servo_up 12

// -> Pinos dos Sensor RGB:
#define S0 2
#define S1 3
#define S2 4
#define S3 5
#define sensor_output 6

// Definição de variáveis auxiliares a usar ao longo do Programa:
// -> Definição das cores:
#define RED 0
#define ORANGE 1
#define YELLOW 2
#define BLUE 3
#define BROWN 4
#define GREEN 5
#define UNKNOWN 6

int cont = 0; // Contador de 'Pintarolas' "wanted", ou seja, as escolhidas pelo Cliente.
int wanted; // Quantidade de 'Pintarolas' que o Cliente pretende.
int color_wanted; // Cor que o Cliente pretende.

int color_no_wanted = 0; // Contador de 'Pintarolas' "no wanted", ou seja, não vão ao encontro do que o Cliente pediu.
int lim_no_wanted; // Limite de Pintarolas no copo "no wanted".

int NO_WANTED = 0; // Estado que está na posição das 'Pintarolas' que o Cliente não quer.
int WANTED = 1; // Estado que está na posição das 'Pintarolas' que o Cliente quer.
int LAST_POS=NO_WANTED; // Última posição do Servo Motor (Servo_patch).

//int red_freq,green_freq, blue_freq;

int ang_descida_pintalora,ang_sensor,ang_patch;

int ang_patch_wanted;
int ang_patch_no_wanted;

int ang_default_up;
int ang_default_down;

// Criação de objetos do tipo Servo, para, assim, podermos, controlar os Servo Motores a utilizar ao longo do Projeto:
Servo Servo_patch; 
/* Servo Motor reponsável pela função "Path_Selection".
   Este Servo Motor vai ser responsável por orientar as 'Pintarolas' para o copo, cujas caraterísticas correspondam às pretendidas pelo cliente, 
  ou para o outro copo, cujas 'Pintarolas' não vão ao encontro daquilo que o Cliente solicitou.
*/

Servo Servo_color; 
/* Servo Motor auxiliar à função "Color_Detection".
  Este Servo Motor vai ser responsável por controlar as 'Pintarolas' que entram na zona destinada à leitura de cor por parte do Sensor RGB. 
  Na verdade, este Servo Motor, permite que o sensor leia uma e só uma 'Pintarola' de cada vez.
*/

Servo Servo_down; 
/* Servo Motor responsável em parte pela função "Go_Back_to_the_Top".
   Este Servo Motor vai ser reponsável por rodar o braço robótico, permitindo, desta forma, a elevação do copo, cujas 'Pintarolas' não são as desejadas.
*/

Servo Servo_up; //servo motor que faz parte exclusivamente do go back to top, que roda o pote e "despeja"
/* Servo Motor responsável em parte pela função "Go_Bacl_to_the_Top".
   Este Servo Motor vai ser resposável por rodar o copo, permitindo, desta forma, que quando o braço robótico chegue ao topo, as 'Pintarolas' sejam,
  simplesmente, despejadas para o recepiente inicial.
*/

//................FUNCTIONS...................................


int media(int tab[10]){ //função para calcular a media das leituras do sensor para minimizar erros de leitura
  int soma=0;
  for(int i=0;i<10;i++){
    soma+=tab[i];
  }
  return soma/10;
}


int color_return(){  //função que retorna a cor lida pelo sensor (color detection)

  //filtro red
  int green_tab[10],red_tab[10],blue_tab[10];

  int red,blue,green;
  
  for(int i=0;i<10;i++){

  //filtro red
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  red_tab[i]=pulseIn(sensor_output,LOW);
  //Serial.print("REd->  ");
  //Serial.print(red);
  //Serial.print("     ");
  delay(100);

  //filtro blue
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  blue_tab[i]=pulseIn(sensor_output,LOW);
  //Serial.print("blue->  ");
  //Serial.print(blue);
  //Serial.print("     ");
  delay(100);

  //filtro green
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  green_tab[i]=pulseIn(sensor_output,LOW);
  delay(100);

  }


  red=media(red_tab); //variavel que guarda valor media red lido pelo sensor
  blue=media(blue_tab); //variavel que guarda valor media blue lido pelo sensor
  green=media(green_tab); //variavel que guarda valor media green lido pelo sensor


  //Serial.print("green->  ");
  //Serial.print(green);
  //Serial.print("     ");
  //Serial.println();

  //detect laranja
  if(red>=50 && red<=60 && blue>=70 && blue <=90 && green>=90 && green<=100){
    return ORANGE;
  }

  //detect azul
  else if(red>=75 && red<=85 && blue>=75 && blue <=80 && green>=95 && green<=100){
    return BLUE;
  }

  //detect vermelho
  else if(red>=45 && red<=55 && blue>=70 && blue <=80 && green>=90 && green<=100){
    return RED;
  }

  //detect verde
  else if(red>=65 && red<=75 && blue>=75 && blue <=90 && green>=80 && green<=90){
    return GREEN;
  }

  //detect castanho
  else if(red>=65 && red<=75 && blue>=75 && blue <=95 && green>=100 && green<=110){
    return BROWN;
  }

  //detect amarelo
  else if(red>=40 && red<=50 && blue>=70 && blue <=80 && green>=65 && green<=78){
    return YELLOW;
  }

  //detect cor desconhecida (quando não há nada no seletor)
  else{
    return UNKNOWN;
  }

}


void setup() {
  
  Serial.begin(9600);
  Servo_patch.attach(pin_servo_patch);
  Servo_color.attach(pin_servo_color);
  Servo_down.attach(pin_servo_down);
  Servo_up.attach(pin_servo_up);
  pinMode(S0,OUTPUT);
  pinMode(S1,OUTPUT);
  pinMode(S2,OUTPUT);
  pinMode(S3,OUTPUT);
  pinMode(sensor_output,INPUT);
  //escalar a freq para 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);


  //colocar servos no angulo inicial
  Servo_patch.write(ang_patch_no_wanted);
  Servo_color.write(ang_descida_pintarola);
  Servo_down.write(ang_default_down);
  Servo_up.write(ang_default_up);
}

void loop() {
cont=0;

  if(cont_no_wanted==lim_no_wanted){
    cont_no_wanted=0;
    //chamar função que leva para  cima (go up)
  }

while(cont<wanted){
  //servo na posição inicial onde recebe as pintarolas 
  delay(300);
  for(int i=ang_descida_pintarola;i<ang_sensor;i++){
    Servo_color.write(i);
    delay(10);
  }
  

    //.....................PATCH SELECTION................
    
  //posição da leitura do sensor
  if(color_wanted==color_return()){ //se a cor lida for a pretendida pelo utilizador
    if(LAST_POS==NO_WANTED){
      Servo_patch.write(ang_patch_wanted);
    }
    LAST_POS=WANTED;
    cont++;
  }
  else{  // se a cor lida não for a requerida pelo utilizador
    if(LAST_POS==WANTED){
      Servo_patch.write(ang_patch_no_wanted);
    }
    LAST_POS=NO_WANTED;
    cont_no_wanted++;
  }

  delay(300);

  //posição no buraco onde a pintarola cai para o patch
  for(int i=ang_sensor;i<ang_patch;i++){
    Servo_color.write(i);
    delay(10);
  }
  delay(500);  //aqui a pintarola já caiu no recipiente certo
   
   //servo volta para onde recebe as pintarolas
   for(int i=ang_patch;i>ang_descida_pintarola;i--){
    Servo_color.write(i);
    delay(5);
  }
  delay(100);

  if(cont_no_wanted==lim_no_wanted){
    cont_no_wanted=0;
    //chamar função que leva para cima (go up)
  }

}

}
