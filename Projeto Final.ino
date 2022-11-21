#include <Servo.h>
#include <LiquidCrystal_I2C.h>

#define endereco 0x27 //endereço do arduino usado para escrever no lcd no i2c

LiquidCrystal_I2C lcd=LiquidCrystal_I2C(endereco,16,2); //criar um objeto que será o lcd

Servo Servo_patch; //servo motor que escolhe os potes
Servo Servo_color; //sevo motor que move a pintarola desde a sua descida, que leva ao sensor e depois deixa para o patch
Servo Servo_down;  //servo motor que faz parte exclusivamente do go back to top, que roda o braço e eleva o pote
Servo Servo_up;  //servo motor que faz parte exclusivamente do go back to top, que roda o pote e "despeja"

//bytes que o lcd vai escrever 

byte char_0_esquerda[] = {
    B11111,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B10000,
    B11111
    };

  byte char_1_esquerda[] = {
    B11111,
    B10000,
    B10110,
    B10110,
    B10110,
    B10110,
    B10000,
    B11111
    };
  
  byte char_0_centro[] = {
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111
    };
  
  byte char_1_centro[] = {
    B11111,
    B00000,
    B01110,
    B01110,
    B01110,
    B01110,
    B00000,
    B11111
    };

  byte char_0_direita[] = {
    B11111,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B00001,
    B11111
    };
  
  byte char_1_direita[] = {
    B11111,
    B00001,
    B01101,
    B01101,
    B01101,
    B01101,
    B00001,
    B11111
    };



//define pins

// sensor pins
#define S0 2
#define S1 3
#define S2 4
#define S3 5
#define sensor_output 6

//servo motors pins
#define pin_servo_patch 9
#define pin_servo_color 10
#define pin_servo_down 11
#define pin_servo_up 12

//RGB Led pins 

#define blue_pin 11
#define red_pin 10
#define green_pin 9

//int red_freq,green_freq, blue_freq;

int ang_descida_pintarola,ang_sensor,ang_patch;

int ang_patch_wanted;
int ang_patch_no_wanted;

int ang_default_up = 45;
int ang_default_down = 0;

//define colors

char RED = 'R';
char ORANGE ='O';
char YELLOW = 'Y';
char BLUE = 'B';
char BROWN = 'C';
char GREEN = 'G';
char UNKNOWN = 'U';

//define array of RGB colors for Led RGB

int colors[][3]={ {255,0,0},    //green
                  {40,0,255},   //orange
                  {0,0,255},    //red
                  {0,255,0},    //blue
                  {200,0,255},  //yellow
                  {4,0,255}     //brown
                  };


int cont=0; // contador pintarolas wanted
int wanted; //quantidade de pintarolas que o utilizador quer
char color_wanted; //cor que o utilizador quer
int cont_no_wanted=0; //contador de pintarolas no wanted
int lim_no_wanted; //limite de pintarolas no recipiente no wanted
int cont_unknown=0; //contador que conta as pintarolas não reconhecidas que no caso acontecerá quando não houver pintaroras no "depósito" superior

//power states of the machine
char OFF='F';
char ON='N';
char MACHINE_MODE=OFF;

char received; // char recebido pelo programa vindo da app

char read; //cor lida pelo sensor
int NO_WANTED=0; //estado na posição das pintarolas que o utilizador não quer
int WANTED=1; //estado na posição das pintarolas que o utilizador não quer
int LAST_POS=NO_WANTED; //última pisição do Servo_patch

//................FUNCTIONS...................................


int media(int tab[10]){ //função para calcular a media das leituras do sensor para minimizar erros de leitura
  int soma=0;
  for(int i=0;i<10;i++){
    soma+=tab[i];
  }
  return soma/10;
}


char color_return(){  //função que retorna a cor lida pelo sensor (color detection)

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

//função go up, responsavel por levar as pintarolas no wanted devolta para o deposito inicial

void GO_UP(){
  Servo_down.write(0);
  Servo_up.write(45);

  for (int i = 0; i < 40; i++) {
    Servo_down.write(i);
    delay(30);
  }

  delay(1500);

  for (int i = 45; i > 10; i--) {
    Servo_up.write(i);
    delay(10);
  }
  delay(1500);

  for (int i = 40; i < 68; i++) {
    Servo_down.write(i);
    delay(30);
  }

  delay(1000);

  for (int i = 10; i < 80; i++) {
    Servo_up.write(i);
    delay(30);
  }

  delay(3000); //recoloca as pintarolas

  for (int i = 68; i > 45; i--) {
    Servo_up.write(i);
    delay(30);
  }

  delay(2000);

  for (int i = 68; i > 0; i--) {
    Servo_down.write(i);
    delay(30);
  }
  delay(1000);
}

//função responsavel pela troca de power states da maquina

void switch_machine_state(){
  if (MACHINE_MODE==OFF){
    MACHINE_MODE==ON;
  }
  else{
    MACHINE_MODE=OFF;
  }
}


//função que escreve no led RGB

void write_RGB(int red, int green, int blue){
  analogWrite(red_pin,red);
  analogWrite(green_pin,green);
  analogWrite(blue_pin,blue);

}

//função que sabendo a cor requerida pelo utilizador, a escreve no led RGB

void write_wanted_color_RGB(char cor){
  if (cor == GREEN){
    write_RGB(colors[0][2],colors[0][0],colors[0][1]);
  }
  else if(cor==ORANGE){
    write_RGB(colors[1][2],colors[1][0],colors[1][1]);
  }
  else if(cor==RED){
    write_RGB(colors[2][2],colors[2][0],colors[2][1]);
  }
  else if(cor==BLUE){
    write_RGB(colors[3][2],colors[3][0],colors[3][1]);
  }
  else if(cor==YELLOW){
    write_RGB(colors[4][2],colors[4][0],colors[4][1]);
  }
  else { //se a cor for brown
    write_RGB(colors[5][2],colors[5][0],colors[5][1]);
  }
}


void setup() {
  
  lcd.init(); //inicialização do lcd

  Serial.begin(9600);

  Servo_patch.attach(pin_servo_patch);
  Servo_color.attach(pin_servo_color);
  Servo_down.attach(pin_servo_down);
  Servo_up.attach(pin_servo_up);

  pinMode(blue_pin,OUTPUT);    //RGB led pin
  pinMode(red_pin,OUTPUT);    //RGB led pin
  pinMode(green_pin,OUTPUT); //RGB led pin
  
  pinMode(S0,OUTPUT);
  pinMode(S1,OUTPUT);
  pinMode(S2,OUTPUT);
  pinMode(S3,OUTPUT);
  pinMode(sensor_output,INPUT);

  //criar bytes para o lcd escrever e fazer a barra de loading

  lcd.createChar( 0, char_0_esquerda);
  lcd.createChar( 1, char_1_esquerda);
  lcd.createChar( 2, char_0_centro);
  lcd.createChar( 3, char_1_centro);
  lcd.createChar( 4, char_0_direita);
  lcd.createChar( 5, char_1_direita);
  
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

if(MACHINE_MODE==OFF){

lcd.noBacklight(); //apaga luz de fundo do lcd

write_RGB(0,0,0); //apanga led RGB


if(Serial.avalible()>0){ //se receber da app que e para ligar
  received=Serial.read();
  if(received==ON){
    switch_machine_state();
  }
}

delay(100);

}

else{
lcd.Backlight(); //liga luz de fundo do lcd
cont=-1;

if(cont_no_wanted>0){ //se houver pintarolas no recipiente do no_wanted vai "despejar" no recipiente inicial
    cont_no_wanted=0;
    GO_UP();
  }

  //esperar pela interação dda aplicação e depois fazer cont=0
    
while(cont<wanted && cont!=-1){

  //print no rgb led da cor requerida
  write_wanted_color_RGB(color_wanted);

  //servo na posição inicial onde recebe as pintarolas 
  delay(300);
  for(int i=ang_descida_pintarola;i<ang_sensor;i++){
    Servo_color.write(i);
    delay(30);
  }
  

    //.....................PATCH SELECTION................
    
  //posição da leitura do sensor

  read=color_return(); //lê cor
  
 
  if(read==UNKNOWN){ // se a cor lida pelo sensor for unknown, ou seja, não houver pintarolas disponiveis no "depósito"
   
        while(read==UNKNOWN){
           cont_unknown++;

          if(cont_unknown<2){

            //print que detetou cor desconhecida ou nao recolheu pintarola
            delay(1000);
          }
          else{

            //print no lcd que nao ha pintarolas no deposito inicial

            delay(2000);
          }

          Servo_color.write(ang_descida_pintarola); //volta para a posição onde recebe pintarolas
          delay(300);
          for(int i=ang_descida_pintarola;i<ang_sensor;i++){ //vai verificar ao sensor
            Servo_color.write(i);
            delay(30);
          }

          read=color_return(); //relê a cor 
          delay(50);
          
        }
        cont_unknown=0;
        delay(200);
      
  }

  else if(color_wanted==read){ //se a cor lida for a pretendida pelo utilizador
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

  delay(500);

  //posição no buraco onde a pintarola cai para o patch
  for(int i=ang_sensor;i<ang_patch;i++){
    Servo_color.write(i);
    delay(30);
  }
  delay(500);  //aqui a pintarola já cai no recipiente certo
   
   //servo volta para onde recebe as pintarolas ja sem pintarolas
   for(int i=ang_patch;i>ang_descida_pintarola;i--){
    Servo_color.write(i);
    delay(10);
  }
  delay(100);

  if(cont_no_wanted==lim_no_wanted){  //se o nº de pintarolas no recipiente no_wanted chegar ao lim_max vai "despejar" no recipiente inicial
    cont_no_wanted=0;
    GO_UP();
  }

}
delay(200);
}
}
