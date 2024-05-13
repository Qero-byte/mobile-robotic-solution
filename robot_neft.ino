//v.1.a
//Libraries
#include <AsyncStream.h>
#include <GParser.h>
#include <GyverMotor.h>
#include <GyverINA.h>
#include <INA219.h>
#include <Servo.h>

//Async serial init
AsyncStream<50> async_bt(&Serial, '\n'); //EOL - '\n'

//Motors init
GMotor motorFL(DRIVER3WIRE, 15, 14, 11, HIGH); //Front left
GMotor motorFR(DRIVER3WIRE, 17, 16, 5, HIGH); //Front right
GMotor motorBL(DRIVER3WIRE, 7, 8, 6, HIGH); //Back left
GMotor motorBR(DRIVER3WIRE, 4, 2, 3, HIGH); //Back right

//INA219 init
INA219 platPower;

//Servos init
Servo platL;
Servo platR;

//Defining variables
#define MY_PERIOD 2000  //Serial sending period (ms)
uint32_t tmr1;         //Timer variable
bool serial_status = false;

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
  //Starting up INA219
  platPower.begin();
  //Servos
  platL.attach(9);
  platR.attach(10);
  //Platform relay
  pinMode(12, OUTPUT);
  digitalWrite(12, 1);
}

void receive() {
  if (async_bt.available()) { //If some data in serial
    GParser data(async_bt.buf, '/'); //Parsing data (Separator - '/')
    int am = data.split(); //Splitting data
    /*
      start of transmission - start\n
      end of transmission - end\n
      servo - srv/angle(0...180)\n
      movement - move/type(frw,bck,trn_r,trn_l,stop)/speed(0...255)\n
      electrolisys - ptpw/status(0,1)\n
      gps coordinates - gps/latitude(широта)/longitude(долгота)\n
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
    } else if(data.equals(0, "ptpw")) {
      if(data.getInt(1) == 0) {
        digitalWrite(12, 1);
      } else if (data.getInt(1) == 1) {
        digitalWrite(12, 0);
      }
    } else if(data.equals(0, "start")) {
      serial_status = true;
    } else if(data.equals(0, "end")) {
      serial_status = false;
      motor_stop();
      platL.write(25);
      platR.write(155);
      digitalWrite(12, 1);
    }
  }
}

void send() {
  /*
    voltage/current/power
  */
  //Serial.println(String(platPower.getVoltage(), 3)  + "/" + String(platPower.getCurrent(), 3) + "/" + String(platPower.getPower(), 3));
  Serial.print(platPower.getVoltage(), 3);
  Serial.print("/");
  Serial.print(platPower.getCurrent(), 3);
  Serial.print("/");
  Serial.print(platPower.getPower(), 3);
  Serial.print(";");
}

void loop() {
  //Receiving package
  receive();
  //Sending package
  //Average timer on millis()
  if (millis() - tmr1 >= MY_PERIOD) {
    tmr1 = millis();
    send();
  }
}