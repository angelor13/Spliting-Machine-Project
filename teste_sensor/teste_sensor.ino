//pins
#define S0 2
#define S1 3
#define S2 4
#define S3 5
#define sensor_output 6

void color_return(){  //função que retorna a cor lida pelo sensor (color detection)

  int red,blue,green;
  

  //filtro red
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  red=pulseIn(sensor_output,LOW);
  Serial.print("REd->  ");
  Serial.print(red);
  Serial.print("     ");
  delay(100);

  //filtro blue
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  blue=pulseIn(sensor_output,LOW);
  Serial.print("blue->  ");
  Serial.print(blue);
  Serial.print("     ");
  delay(100);

  //filtro green
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  green=pulseIn(sensor_output,LOW);
  Serial.print("green->  ");
  Serial.print(green);
  Serial.print("     ");
  Serial.println();
  delay(100);
  
 
}


void setup() {
  Serial.begin(9600);
  pinMode(S0,OUTPUT);
  pinMode(S1,OUTPUT);
  pinMode(S2,OUTPUT);
  pinMode(S3,OUTPUT);
  pinMode(sensor_output,INPUT);
  //escalar a freq para 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);

}

void loop() {
 color_return();

}
