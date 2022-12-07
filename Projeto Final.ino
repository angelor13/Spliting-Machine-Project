#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Softwareserial.h>

#define endereco 0x27 //endereço do arduino usado para escrever no lcd no i2c

SoftwareSerial BTSerial(3, 2); // rx, tx (arduino) --> tx, rx (bluetooth)

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
  
   // Caractere char_0_pintarola: ( Byte 6 )
byte char_0_pintarola[] = {
  B00000,  //
  B00000,  //
  B01110,  //   * * *
  B10001,  // *       *
  B10001,  // *       *
  B10001,  // *       *
  B01110,  //   * * *
  B00000   //
};

// Caractere char_1_pintarola: ( Byte 7 )
byte char_1_pintarola[] = {
  B00000,  //
  B00000,  //
  B01110,  //   * * *
  B11111,  // * * * * *
  B11111,  // * * * * *
  B11111,  // * * * * *
  B01110,  //   * * *
  B00000   //
}; 




//DEFINE PINS

//led pins(go up state)

#define red_up 4
#define green_up 5

//machine state led pins

#define red_machine_state 30
#define green_machine_state 31

//button pins

#define button 40


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

int ang_descida_pintarola = 171, ang_sensor=114, ang_patch =71;

int ang_patch_wanted = 124;
int ang_patch_no_wanted = 154;

int ang_default_up = 179;
int ang_default_down = 15;

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
int cont_no_wanted = 0; //contador de pintarolas no wanted
int lim_no_wanted = 6; //limite de pintarolas no recipiente no wanted
int cont_unknown=0; //contador que conta as pintarolas não reconhecidas que no caso acontecerá quando não houver pintaroras no "depósito" superior


int comeco=0;
// LCD states

const int BARRA_LCD_MODE = 0; //modo do lcd para escrever barra e percentagem de pintarolas ja conseguidas
int CURRENT_LCD_MODE = BARRA_LCD_MODE;  //inicialização de modo do LCD inicial como o de escrevr barra
const int OTHER_LCD_MODE = 1; //modo alternativo no lcd
const int COLOR_LCD_MODE=2; //modo do lcd em que mostra a cor que estamos a escolher
int LAST_LCD_MODE=-1; //ultimo modo escolhido no lcd
int last_cont; //ultima contagem de pintarolas do tipo wanted
char last_color; //umtima cor escolhida

//power states of the machine
char OFF='F';
char ON='N';
char MACHINE_MODE=OFF;

// bluetooth mode
char CONECTED='D';
char DISCONECTED='S';
char BLUETOOTH_MODE=DISCONECTED;
int cont_conected=0; //contador de quantas vezes passa no conected 
int cont_disconected=0; //contador de quantas vezes passa no disconnected

char received; // char recebido pelo programa vindo da app

char read; //cor lida pelo sensor
int NO_WANTED=0; //estado na posição das pintarolas que o utilizador não quer
int WANTED=1; //estado na posição das pintarolas que o utilizador não quer
int LAST_POS=NO_WANTED; //última pisição do Servo_patch

//................FUNCTIONS...................................
void LCD_COLOR_MODE(char cor){  //modo do lcd que nos da a cor a ser escolhida
  if (cor == RED){
    lcd.setCursor(0,0);
    lcd.print("R-");
    lcd.write(byte(7));
    lcd.setCursor(4,0);
    lcd.print("B-");
    lcd.write(byte(6));
    lcd.setCursor(8,0);
     lcd.print("Y-");
    lcd.write(byte(6));
    lcd.setCursor(12,0);
     lcd.print("BR-");
    lcd.write(byte(6));
    lcd.setCursor(3,1);
    lcd.print("G-");
    lcd.write(byte(6));
    lcd.setCursor(11,1);
     lcd.print("OR-");
    lcd.write(byte(6));
  }
  else if(cor== BLUE){
     lcd.setCursor(0,0);
    lcd.print("R-");
    lcd.write(byte(6));
    lcd.setCursor(6,0);
    lcd.print("B-");
    lcd.write(byte(7));
    lcd.setCursor(10,0);
     lcd.print("Y-");
    lcd.write(byte(6));
    lcd.setCursor(12,0);
     lcd.print("BR-");
    lcd.write(byte(6));
    lcd.setCursor(5,1);
    lcd.print("G-");
    lcd.write(byte(6));
    lcd.setCursor(11,1);
     lcd.print("OR-");
    lcd.write(byte(6));
  }
  else if(cor== YELLOW){
     lcd.setCursor(0,0);
    lcd.print("R-");
    lcd.write(byte(6));
    lcd.setCursor(4,0);
    lcd.print("B-");
    lcd.write(byte(6));
    lcd.setCursor(8,0);
     lcd.print("Y-");
    lcd.write(byte(7));
    lcd.setCursor(12,0);
     lcd.print("BR-");
    lcd.write(byte(6));
    lcd.setCursor(3,1);
    lcd.print("G-");
    lcd.write(byte(6));
    lcd.setCursor(11,1);
     lcd.print("OR-");
    lcd.write(byte(6));
  }
  else if(cor== BROWN){
     lcd.setCursor(0,0);
    lcd.print("R-");
    lcd.write(byte(6));
    lcd.setCursor(4,0);
    lcd.print("B-");
    lcd.write(byte(6));
    lcd.setCursor(8,0);
     lcd.print("Y-");
    lcd.write(byte(6));
    lcd.setCursor(12,0);
     lcd.print("BR-");
    lcd.write(byte(7));
    lcd.setCursor(3,1);
    lcd.print("G-");
    lcd.write(byte(6));
    lcd.setCursor(11,1);
     lcd.print("OR-");
    lcd.write(byte(6));
  }
  else if(cor== GREEN){
     lcd.setCursor(0,0);
    lcd.print("R-");
    lcd.write(byte(6));
    lcd.setCursor(4,0);
    lcd.print("B-");
    lcd.write(byte(6));
    lcd.setCursor(8,0);
     lcd.print("Y-");
    lcd.write(byte(6));
    lcd.setCursor(12,0);
     lcd.print("BR-");
    lcd.write(byte(6));
    lcd.setCursor(3,1);
    lcd.print("G-");
    lcd.write(byte(7));
    lcd.setCursor(11,1);
     lcd.print("OR-");
    lcd.write(byte(6));
  }
  else{
     lcd.setCursor(0,0);
    lcd.print("R-");
    lcd.write(byte(6));
    lcd.setCursor(4,0);
    lcd.print("B-");
    lcd.write(byte(6));
    lcd.setCursor(8,0);
     lcd.print("Y-");
    lcd.write(byte(6));
    lcd.setCursor(12,0);
     lcd.print("BR-");
    lcd.write(byte(6));
    lcd.setCursor(3,1);
    lcd.print("G-");
    lcd.write(byte(6));
    lcd.setCursor(11,1);
     lcd.print("OR-");
    lcd.write(byte(7));
  }
  
}

void LCD_BARRA(int cont,int wanted){    //função que desenha a barra no lcd no modo lcd_barra

  int n=map(cont,0,wanted,0,15);
  int perc=map(cont,0,wanted,0,100);

lcd.print("Sorting... ");
  lcd.print(perc);
  lcd.print("%");


if(n==15){
  
  lcd.setCursor(0,1);
  lcd.write(byte(1));
  for(int i=1;i<15;i++){
    lcd.setCursor(i,1);
    lcd.write(byte(3));
  }
  lcd.setCursor(15,1);
  lcd.write(byte(5));
}

else if(n==0){
  lcd.setCursor(0,1);
  lcd.write(byte(0));
  for(int i=1;i<15;i++){
    lcd.setCursor(i,1);
    lcd.write(byte(2));
  }
  lcd.setCursor(15,1);
  lcd.write(byte(4));
}
else{
  lcd.setCursor(0,1);
  lcd.write(byte(1));
  for(int i=1;i<15;i++){
    if(i<=n){
    lcd.setCursor(i,1);
    lcd.write(byte(3));
    }
    else{
    lcd.setCursor(i,1);
    lcd.write(byte(2));
    }
  }
  lcd.setCursor(15,1);
  lcd.write(byte(4));
}
}
 
//função que deteta press do botão

bool detect_press() {
  static int buttonState = LOW;  //initialize once in LOW
  bool pressed = false;
  int newbuttonState = digitalRead(button);  //pressed->unpressed (button lifted)
  if (buttonState == HIGH && newbuttonState == LOW) {
     //change mode!
    pressed = true;
  }
  buttonState = newbuttonState;  //keep track of button state
  return pressed;
}

//função que muda state do LCD durante os processos

void LCD_MODES(int time,int cont,int wanted,char color){

  if(!detect_press()){
      int start = millis();
  while(millis()-start<=time){

    if(detect_press()){
        switch_lcd_mode();
        lcd.clear();
      }
      
    else if(CURRENT_LCD_MODE ==  BARRA_LCD_MODE ){
      if(last_cont!=cont || LAST_LCD_MODE!=OTHER_LCD_MODE || last_color!=color){
        last_cont=cont;
        last_color=color;
        LAST_LCD_MODE=OTHER_LCD_MODE;
        lcd.clear();
      LCD_BARRA(last_cont,wanted);
      }
    }
    
    else if(CURRENT_LCD_MODE ==  COLOR_LCD_MODE){
            if(last_cont!=cont || LAST_LCD_MODE!=BARRA_LCD_MODE || last_color!=color){
        last_cont=cont;
        last_color=color;
        LAST_LCD_MODE=BARRA_LCD_MODE;
        lcd.clear();
      LCD_COLOR_MODE(last_color);
      }
      
    }
    else{
    
      if(last_cont!=cont|| LAST_LCD_MODE!=COLOR_LCD_MODE || last_color!=color){
        last_color=color;
        last_cont=cont;
        LAST_LCD_MODE=COLOR_LCD_MODE;
        lcd.clear();
      lcd.print("To sort:"); //dar print de outra coisa
      lcd.setCursor(10,0);
      lcd.print(wanted);
      lcd.setCursor(0,1);
      lcd.print("Sorted:");
      lcd.setCursor(10,1);
      lcd.print(last_cont); //dar print de outra coisa
      }
  }
}
}
}

//funçao que troca os estado de conexão bluetooth
void switch_bluetooth_state(){
  if(BLUETOOTH_MODE==CONECTED){
    BLUETOOTH_MODE=DISCONECTED;
  }
  else{
    BLUETOOTH_MODE=CONECTED;
  }
}

//função que troca os estados do LCD

void switch_lcd_mode() {
  if (CURRENT_LCD_MODE == BARRA_LCD_MODE) {  
    
    CURRENT_LCD_MODE = COLOR_LCD_MODE;
  } 
  else if(CURRENT_LCD_MODE==COLOR_LCD_MODE){
    CURRENT_LCD_MODE = OTHER_LCD_MODE;
  }
  else {  
    CURRENT_LCD_MODE = BARRA_LCD_MODE;
  }
}

int media(int tab[10]){ //função para calcular a media das leituras do sensor para minimizar erros de leitura
  int soma=0;
  for(int i=0;i<10;i++){
    soma+=tab[i];
  }
  return soma/10;
}


char color_return(int cont,int wanted,char color_wanted){  //função que retorna a cor lida pelo sensor (color detection)

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
  LCD_MODES(100,cont,wanted,color_wanted);

  //filtro blue
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  blue_tab[i]=pulseIn(sensor_output,LOW);
  //Serial.print("blue->  ");
  //Serial.print(blue);
  //Serial.print("     ");
  LCD_MODES(100,cont,wanted,color_wanted);

  //filtro green
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  green_tab[i]=pulseIn(sensor_output,LOW);
  LCD_MODES(100,cont,wanted,color_wanted);

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

void GO_UP(int cont,int wanted,char color_wanted){
  Servo_down.write(15);
  Servo_up.write(179);

  for(int i=179;i>140;i--){
    Servo_up.write(i);
    LCD_MODES(10,cont,wanted,color_wanted);
  }

  for (int i = 15; i < 50; i++) {
    Servo_down.write(i);
    LCD_MODES(30,cont,wanted,color_wanted);
  }

  LCD_MODES(1000,cont,wanted,color_wanted);

  for (int i = 140; i > 100; i--) {
    Servo_up.write(i);
    LCD_MODES(10,cont,wanted,color_wanted);
  }
  LCD_MODES(1500,cont,wanted,color_wanted);

  for (int i = 50; i < 97; i++) {
    Servo_down.write(i);
    LCD_MODES(30,cont,wanted,color_wanted);
  }

  LCD_MODES(500,cont,wanted,color_wanted);

  for (int i = 100; i < 175; i++) {
    Servo_up.write(i);
    LCD_MODES(30,cont,wanted,color_wanted);
  }

  LCD_MODES(2000,cont,wanted,color_wanted); //recoloca as pintarolas

  for (int i = 160; i > 140; i--) {
    Servo_up.write(i);
    LCD_MODES(30,cont,wanted,color_wanted);
  }

  LCD_MODES(2000,cont,wanted,color_wanted);

  for (int i = 97; i > 15; i--) {
    Servo_down.write(i);
    LCD_MODES(30,cont,wanted,color_wanted);
  }

  for(int i=140;i<179;i++){
    Servo_up.write(i);
    LCD_MODES(10,cont,wanted,color_wanted);
  }

  LCD_MODES(1000,cont,wanted,color_wanted);
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

void write_wanted_color_RGB(char cor, int tab[][3]){
  if (cor == GREEN){
    write_RGB(tab[0][2],tab[0][0],tab[0][1]);
  }
  else if(cor==ORANGE){
    write_RGB(tab[1][2],tab[1][0],tab[1][1]);
  }
  else if(cor==RED){
    write_RGB(tab[2][2],tab[2][0],tab[2][1]);
  }
  else if(cor==BLUE){
    write_RGB(tab[3][2],tab[3][0],tab[3][1]);
  }
  else if(cor==YELLOW){
    write_RGB(tab[4][2],tab[4][0],tab[4][1]);
  }
  else { //se a cor for brown
    write_RGB(tab[5][2],tab[5][0],tab[5][1]);
  }
}


void setup() {
  
  lcd.init(); //inicialização do lcd

  BTSerial.begin(9600);
  Serial.begin(9600);

  Servo_patch.attach(pin_servo_patch);
  Servo_color.attach(pin_servo_color);
  Servo_down.attach(pin_servo_down);
  Servo_up.attach(pin_servo_up);

  pinMode(blue_pin,OUTPUT);    //RGB led pin
  pinMode(red_pin,OUTPUT);    //RGB led pin
  pinMode(green_pin,OUTPUT); //RGB led pin

  pinMode(red_up,OUTPUT);     //red led go_up pin
  pinMode(green_up,OUTPUT);   //green led go_up pin

  pinMode(red_machine_state,OUTPUT);     //red led machine state pin
  pinMode(green_machine_state,OUTPUT);   //green led machine state pin

  pinMode(button,INPUT); //botão do lcd
  
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
  lcd.createChar(6,char_0_pintarola);
  lcd.createChar(7,char_1_pintarola);
  
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
cont_conected=0; //reset contador conected
cont_disconected=0; //reset contador disconected


digitalWrite(red_machine_state,HIGH);   //acende red led machine state
digitalWrite(green_machine_state,LOW); //apaga green led machine state

lcd.noBacklight(); //apaga luz de fundo do lcd

digitalWrite(red_up,LOW);   //apaga red led do go up
digitalWrite(green_up,LOW); //apaga green led do go up

write_RGB(0,0,0); //apaga led RGB



if(BTSerial.available()>0){ //se receber da app que e para ligar
  received=BTSerial.read();
  
  if(received==CONECTED){
    switch_bluetooth_state();
    lcd.backlight();
    for(int i=0;i<3;i++){
    lcd.print("Device conected");
    delay(100);
    lcd.clear();
    delay(100);
    }
    lcd.clear();
    lcd.noBacklight();
  }
  /*else if(received==DISCONECTED){
     switch_bluetooth_state();
     lcd.backlight();
    lcd.print("Disconected");
    for(int i=0;i<4;i++){
      lcd.scrollDisplayRight();
      delay(250);
    }
    lcd.clear();
    lcd.noBacklight();
  }*/

  else if(received==ON){
    switch_machine_state();
    lcd.backlight();
    lcd.print("POWER ON");
    delay(500);
    lcd.clear();
  }
}
delay(50);
}

else{

digitalWrite(red_machine_state,LOW);   //apaga red led machine state
digitalWrite(green_machine_state,HIGH); //acende green led machine state

lcd.backlight(); //liga luz de fundo do lcd

digitalWrite(green_up,HIGH); //acende led verde do go up
digitalWrite(red_up,LOW);    //apaga led red do go up




if(BTSerial.available()>0){ //se receber alguma coisa da aplicação
  received=BTSerial.read();
  cont_conected=0;
  cont_disconected=0;

  if(received==OFF){
    switch_machine_state();
    lcd.print("POWER OFF");
    delay(500);
    lcd.clear();
    
  }
  /*
  else if(received==DISCONECTED){
    switch_bluetooth_state();
    lcd.print("Disconected");
    while(received!=CONECTED){
      if(Serial.available()>0){
        received=Serial.read();
      }
      lcd.scrollDisplayRight();
      delay(50);
    }
    switch_bluetooth_state();
    lcd.clear();
  }*/

  else {
    if(received==RED){
      color_wanted=RED;
    }
    else if(received == BLUE){
      color_wanted=BLUE;
    }
    else if (received==YELLOW){
      color_wanted=YELLOW;
    }
    else if (received==BROWN){
      color_wanted=BROWN;
    }
    else if (received==GREEN){
      color_wanted=GREEN;
    }
    else if (received==ORANGE){
      color_wanted=ORANGE;
    }
    else if (received=='H'){
      wanted=1;
      cont=0;
    }
    else if (received=='Z'){
      wanted=2;
      cont=0;
    }
    else if (received=='K'){
      wanted=3;
      cont=0;
    }
    else if (received=='W'){
      wanted=4;
      cont=0;
    }
    else if (received=='X'){
      wanted=5;
      cont=0;
    }
    else if (received=='T'){
      wanted=6;
      cont=0;
    }
    else if (received=='V'){
      wanted=7;
      cont=0;
    }
    else if (received=='J'){
      wanted=8;
      cont=0;
    }
    else if (received=='I'){
      wanted=9;
      cont=0;
    }
  }
}
else if(BLUETOOTH_MODE == CONECTED){
  if(cont_conected==0){
    cont_disconected=0;
  lcd.clear();
  lcd.print("Waiting data!");
  cont_conected++;
  }
  lcd.scrollDisplayRight();
  
}

else{
if(cont_disconected==0){
  comeco=millis();
  cont_conected=0;
  lcd.clear();
  lcd.print("Conect device");
  cont_disconected++;
  }
  else if(millis()-comeco>=10000){
    switch_machine_state();
  }
  else{
  lcd.scrollDisplayRight();
  }
}

  //esperar pela interação dda aplicação e depois fazer cont=0
    
while(cont<wanted && cont!=-1){

  //print no rgb led da cor requerida
  

  //servo na posição inicial onde recebe as pintarolas 
  LCD_MODES(300,cont,wanted,color_wanted);
  
  for(int i=ang_descida_pintarola;i>ang_sensor;i--){
    Servo_color.write(i);
    LCD_MODES(30,cont,wanted,color_wanted);
  }

  LCD_MODES(100,cont,wanted,color_wanted);
  

    //.....................PATCH SELECTION................
    
  //posição da leitura do sensor

read=color_return(cont,wanted,color_wanted); //lê cor
  
 write_wanted_color_RGB(read,colors); //mostra a cor lida pelo sensor

  if(read==UNKNOWN){ // se a cor lida pelo sensor for unknown, ou seja, não houver pintarolas disponiveis no "depósito"
   
        while(read==UNKNOWN){
           cont_unknown++;

          if(cont_unknown<2){

            //print que detetou cor desconhecida ou nao recolheu pintarola
            LCD_MODES(1000,cont,wanted,color_wanted);
          }
          else{

            //print no lcd que nao ha pintarolas no deposito inicial
            lcd.clear();
            lcd.print("Nao ha pintarolas disponiveis!");
            LAST_LCD_MODE=-1;
            delay(2000);
          }

          Servo_color.write(ang_descida_pintarola); //volta para a posição onde recebe pintarolas

          LCD_MODES(300,cont,wanted,color_wanted);

          for(int i=ang_descida_pintarola;i>ang_sensor;i--){ //vai verificar ao sensor
            Servo_color.write(i);
            LCD_MODES(30,cont,wanted,color_wanted);
          }

          read=color_return(cont,wanted,color_wanted); //relê a cor 
          LCD_MODES(50,cont,wanted,color_wanted);
          
        }
        cont_unknown=0;
        LCD_MODES(200,cont,wanted,color_wanted);
      
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

  LCD_MODES(500,cont,wanted,color_wanted);

  //posição no buraco onde a pintarola cai para o patch
  for(int i=ang_sensor;i>ang_patch;i--){
    Servo_color.write(i);
    LCD_MODES(30,cont,wanted,color_wanted);
  }
  LCD_MODES(500,cont,wanted,color_wanted);  //aqui a pintarola já cai no recipiente certo
   
   //servo volta para onde recebe as pintarolas ja sem pintarolas
   for(int i=ang_patch;i<ang_descida_pintarola;i++){
    Servo_color.write(i);
    LCD_MODES(10,cont,wanted,color_wanted);
  }

  LCD_MODES(100,cont,wanted,color_wanted);

  if(cont_no_wanted==lim_no_wanted){  //se o nº de pintarolas no recipiente no_wanted chegar ao lim_max vai "despejar" no recipiente inicial
    digitalWrite(green_up,LOW);   //apaga led verde do go up
    digitalWrite(red_up,HIGH);    //acende led red do go up

    cont_no_wanted=0;
    GO_UP(cont,wanted,color_wanted);

    digitalWrite(green_up,HIGH); //acende led verde do go up
    digitalWrite(red_up,LOW);    //apaga led red do go up

  }

  LCD_MODES(300,cont,wanted,color_wanted);
}

if(cont_no_wanted>0){ //se houver pintarolas no recipiente do no_wanted vai "despejar" no recipiente inicial
    digitalWrite(green_up,LOW);   //apaga led verde do go up
    digitalWrite(red_up,HIGH);    //acende led red do go up

    cont_no_wanted=0;
    GO_UP(cont,wanted,color_wanted);

    digitalWrite(green_up,HIGH); //acende led verde do go up
    digitalWrite(red_up,LOW);    //apaga led red do go up
  }
if(cont == wanted){
//print no lcd que acabou a tarefa
lcd.clear();
lcd.print("Task Complete!!");
delay(1000);
LAST_LCD_MODE=-1;
lcd.clear();
}
switch_bluetooth_state(); //disconect device
cont=-1; //recoloca contador de pintarolas que o utilizador quer a -1
delay(50);
}
}
