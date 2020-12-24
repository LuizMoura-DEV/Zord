//Bibliotecas
#include "Adafruit_VL53L0X.h"
#include <EEPROM.h>

//Sensor Linha
#define sensorFD  A0
#define sensorFE  A1
#define sensorTD  A2
#define sensorTE  A3
//Controle Motor
#define motorDF   3
#define motorDT   5
#define motorEF   6
#define motorET   9
//Botões e LEDs
#define btStart   2
#define btModo    4
#define ledR      12
#define ledG      11
#define ledB      10

#define buzzer    8

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

int modoOperacao = EEPROM.read(0);
const int valorBranco = 700, timeVirar = 200, timeCentro = 500, timeVirarZigZag = 600;
bool linha = false, esquerda = false, direita = false, inimigoFrente = false, inimigoAnterior = false, inimigoAtual = false;
bool statusStart = true, statusAntStart = true, statusModo = true, statusAntModo = true;
unsigned long delayTimeEscape, delayTimeTornado, delayTimeZigZag, timeBtMode;


void setup() {
    //Ativando monitor serial    
    Serial.begin(9600);

    //Carregando Sensor Distancia
    lox.begin();
    
    //Definindo Entradas    
    pinMode(sensorFD,   INPUT);
    pinMode(sensorFE,   INPUT);
    pinMode(sensorTD,   INPUT);
    pinMode(sensorTE,   INPUT);
    pinMode(btStart,    INPUT);
    pinMode(btModo,     INPUT);    

    //Definindo Saídas
    pinMode(motorEF,    OUTPUT);
    pinMode(motorET,    OUTPUT);
    pinMode(motorDF,    OUTPUT);
    pinMode(motorDT,    OUTPUT);
    pinMode(ledR,       OUTPUT);
    pinMode(ledG,       OUTPUT);
    pinMode(ledB,       OUTPUT);
    pinMode(buzzer,     OUTPUT);

    motor(0,0,0,0);
}
/*
void loop(){
    teste();
}*/

void loop() {
    statusStart   =    digitalRead(btStart);
    statusModo    =    digitalRead(btModo);
    
    if(!statusModo && statusAntModo && ((millis() - timeBtMode) < 300)){
        modoOperacao++;
        if(modoOperacao > 3){
          modoOperacao = 0;
        }
    }
    if(millis() - timeBtMode >=300){
        timeBtMode = millis();
    }
    
    statusAntModo = statusModo;

    if(!statusStart && statusAntStart){
        EEPROM.write(0, modoOperacao);
        digitalWrite(ledB,        HIGH);
        digitalWrite(ledG,        HIGH);
        digitalWrite(ledR,        HIGH);
        delay(4400);
        tone(buzzer,1000,500);
        motor(100,0,100,0);
        delay(100);
        motor(0,100,0,100);
        delay(100);
        delayTimes();
    }
    
    led(modoOperacao);
    statusAntStart = statusStart;

    while(statusAntStart == false){
        switch (modoOperacao){
            //Azul
            case 0:{
                buscaCega(); 
            }
            break;
            //Verde
            case 1:{
                tornado();
            }
            break;
            //Vermelho
            case 2:{
                zigzag();
            }
            break;
            case 3:{
                //dance();
            }
            break;
        }
    } 
}

void led(int modoOperacao){
    switch (modoOperacao){
        //Azul
        case 0:{
          digitalWrite(ledB,        HIGH);
          digitalWrite(ledG,        LOW);
          digitalWrite(ledR,        LOW);
        }
        break;
        //Verde
        case 1:{
          digitalWrite(ledB,        LOW);
          digitalWrite(ledG,        HIGH);
          digitalWrite(ledR,        LOW);
        }
        break;
        //Vermelho
        case 2:{
          digitalWrite(ledB,        LOW);
          digitalWrite(ledG,        LOW);
          digitalWrite(ledR,        HIGH);
        }
        break;
        //Apagado
        case 3:{
          digitalWrite(ledB,        LOW);
          digitalWrite(ledG,        LOW);
          digitalWrite(ledR,        LOW);
        }
        break;
    }
}

void delayTimes(){
    delayTimeEscape     =millis();
    delayTimeTornado    =millis();
    delayTimeZigZag     =millis();
}

void teste(){
    //Exibir valores dos sensores
    
    inimigo();
    Serial.print("Valores Lidos  ||");
    Serial.print("Iinimigo: ");
    Serial.print(inimigoFrente);
    Serial.print(" -|- Frente: Direita= ");
    Serial.print(analogRead(sensorFD));
    Serial.print(" / Esquerda= ");
    Serial.print(analogRead(sensorFE));
    Serial.print(" -|- Atraz: Direita= ");
    Serial.print(analogRead(sensorTD));
    Serial.print(" / Esquerda= ");
    Serial.print(analogRead(sensorTE));
    Serial.print(" =|=Botões: Start= ");
    Serial.print(digitalRead(btStart));
    Serial.print(" -|- Modo= ");
    Serial.print(digitalRead(btModo));
}

void motor(int escF, int escT, int dirF, int dirT){
    //Recebe porcentagem para cada motor e converte para PWM
    analogWrite(motorEF,    map(escF, 0, 100, 0 ,255));
    analogWrite(motorET,    map(escT, 0, 100, 0 ,255));
    analogWrite(motorDF,    map(dirF, 0, 100, 0 ,255));
    analogWrite(motorDT,    map(dirT, 0, 100, 0 ,255));
}

bool inimigo() {
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false); // si se pasa true como parametro, muestra por puerto serie datos de debug
   
    if (measure.RangeStatus != 4){
        if((measure.RangeMilliMeter > 10) && (measure.RangeMilliMeter < 500)){
            Serial.print("Valor Lido IR: ");
            Serial.println(measure.RangeMilliMeter);
            inimigoFrente = true;
        }else{
            Serial.print("Valor Lido IR: ");
            Serial.println(measure.RangeMilliMeter);
            inimigoFrente = false;
        }
    }else{
        inimigoFrente = false;
    }
}

bool IR(int sensor){
    //Verifica sensor IR recebendo a porta
    if(analogRead(sensor) < valorBranco){
        return true;
    }else{
        return false;
    }
}

void atraz(){
    motor(0,70,0,70);
}

void escape(){
    //Verifica se linha TRUE gira ate false 
    while(IR(sensorFE)){
        atraz();
        escapeT();
        esquerda = true;
        delayTimeEscape = millis();
    }
    while(IR(sensorFD)){
        atraz();
        escapeT();
        direita = true;
        delayTimeEscape = millis();
    }
    if(esquerda){
        if((millis() - delayTimeEscape) < timeVirar){
            motor(80,0,0,100);
        }else{
            esquerda =    false;
            direita =     false;
        }
    }
    if(direita){
        if((millis() - delayTimeEscape) < timeVirar){            
            motor(0,100,80,0);
        }else{
            esquerda =    false;
            direita =     false;
        }
    }
    escapeT();
}

void escapeT(){   
    while(IR(sensorTD)){
        motor(0,0,100,0);
    }
    while(IR(sensorTE)){
        motor(100,0,0,0);
    }  
}

void buscaCega(){
    inimigo();
    if(inimigoFrente){
        motor(100,0,100,0);
        digitalWrite(ledB,HIGH);
    }else{
        motor(40,0,40,0);
        digitalWrite(ledB,LOW);
    }
    escape();
}


void tornado(){
    inimigo();
    if(millis()-delayTimeTornado < timeCentro ){
        if(inimigoFrente){
            motor(100,0,100,0);
        }else{
            motor(40,0,40,0);
        }
        escape();
    }else{
        if(inimigoFrente){
            motor(100,0,100,0);
        }else{
            motor(40,0,0,40);
        }
        escape();
    }
}

void zigzag(){
    inimigo();
    if((millis() - delayTimeZigZag) < timeVirarZigZag){        
        if(inimigoFrente){
            motor(100,0,100,0);
        }else{
            motor(60,0,25,0);
        }
        escape();
    }else{       
        if(inimigoFrente){
            motor(100,0,100,0);
        }else{
            motor(25,0,60,0);
        }
        escape();
    }
    if((millis() - delayTimeZigZag) > (2*timeVirarZigZag)){
         delayTimeZigZag = millis();
    }    
}

void dance(){
  
}
