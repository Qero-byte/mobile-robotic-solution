//v.0.a
//Libraries
#include <AsyncStream.h>
#include <GParser.h>
#include <GyverMotor.h>
#include <Servo.h>

//Async serial init
AsyncStream<50> async_bt(&Serial, '\n'); //EOL - '\n'

//Motors init
GMotor motorFL(DRIVER3WIRE, 15, 14, 11, HIGH); //Front left
GMotor motorFR(DRIVER3WIRE, 17, 16, 5, HIGH); //Front right
GMotor motorBL(DRIVER3WIRE, 7, 8, 6, HIGH); //Back left
GMotor motorBR(DRIVER3WIRE, 4, 2, 3, HIGH); //Back right

//Servos init
Servo platL;
Servo platR;

void move_forward(int spd){
  motorFL.setSpeed(spd);
  motorFR.setSpeed(spd);
  motorBL.setSpeed(spd);
  motorBR.setSpeed(spd);
}

void move_backward(int spd){
  motorFL.setSpeed(-spd);
  motorFR.setSpeed(-spd);
  motorBL.setSpeed(-spd);
  motorBR.setSpeed(-spd);
}

void turn_right(int spd){
  motorFL.setSpeed(spd);
  motorFR.setSpeed(-spd);
  motorBL.setSpeed(spd);
  motorBR.setSpeed(-spd);
}

void turn_left(int spd){
  motorFL.setSpeed(-spd);
  motorFR.setSpeed(spd);
  motorBL.setSpeed(-spd);
  motorBR.setSpeed(spd);
}

void motor_stop(){
  motorFL.setSpeed(0);
  motorFR.setSpeed(0);
  motorBL.setSpeed(0);
  motorBR.setSpeed(0);
}

//Setup
void setup() {
  //Serial
  Serial.begin(115200);
  Serial.setTimeout(10); // 10ms timeout
  //Setting up motors mode (-255...255)
  motorFL.setMode(AUTO);
  motorFR.setMode(AUTO);
  motorBL.setMode(AUTO);
  motorBR.setMode(AUTO);
  //Reversing motors
  motorFL.setDirection(REVERSE);
  motorFR.setDirection(REVERSE);
  motorBL.setDirection(REVERSE);
  motorBR.setDirection(REVERSE);
  //Setting up minimum duty of motors
  motorFL.setMinDuty(150);
  motorFR.setMinDuty(150);
  motorBL.setMinDuty(150);
  motorBR.setMinDuty(150);
  //Servos
  platL.attach(9);
  platR.attach(10);
}

void receive() {
  if (async_bt.available()) { //If some data in serial
    GParser data(async_bt.buf, '/'); //Parsing data (Separator - '/')
    int am = data.split(); //Splitting data
    /*
      servo - srv/angle(0...180);
      movement - move/type(frw,bck,trn_r,trn_l,stop)/speed(0...255);
      gps coordinates - gps/latitude(широта)/longitude(долгота)
    */
    if(data.equals(0, "srv")) {
      platL.write(180 - data.getInt(1));
      platR.write(data.getInt(1));
    } else if(data.equals(0, "move")) {
      if(data.equals(1, "frw")) {
        move_forward(data.getInt(2));
      } else if(data.equals(1, "bck")) {
        move_backward(data.getInt(2));
      } else if(data.equals(1, "trn_r")) {
        turn_right(data.getInt(2));
      } else if(data.equals(1, "trn_l")) {
        turn_left(data.getInt(2));
      } else if(data.equals(1, "stop")) {
        motor_stop();
      }
    }
  }
}

void send() {
  
}

void loop() {
  //Receiving package
  receive();
  //Sending package
  send();
}