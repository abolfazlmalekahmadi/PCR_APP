
//MOTOR_1 is thermoelectric element & MOTOR_2 is the fan
#include <Wire.h>
#include "max6675.h"
#define SLAVE_ADDRESS 0x8
#define SHUTTER_PIN A4
#define FOCUS_PIN A5
#define BRAKE 0
#define cool 0
#define heat 1
#define CS_THRESHOLD 15   // Definition of safety current (Check: "1.3 Monster Shield Example").
//MOTOR 1
#define MOTOR_A1_PIN 7
#define MOTOR_B1_PIN 8

//MOTOR 2
#define MOTOR_A2_PIN 4
#define MOTOR_B2_PIN 9

#define PWM_MOTOR_1 5
#define PWM_MOTOR_2 6

#define CURRENT_SEN_1 A2
#define CURRENT_SEN_2 A3

#define EN_PIN_1 A0
#define EN_PIN_2 A1

#define MOTOR_1 0
#define MOTOR_2 1
const int resetPin = 3;
String Cycle_state;
int N_cycle=40; // Total Number of Cycles
short mode;
float f=0.0;//Motor drive coefficient 
      temp_error[2],
      error_1[2],
      error_2[2],
      time_current_stage,
      drive_0,
      time_1,
      drive_1,
      time_2,
      drive_2;
int iter=1;
int thermoDO = 12;
int thermoCS = 10;
int thermoCLK = 11;
const int Number_of_fields = 3;
int fieldIndex = 0;
int values[Number_of_fields];
MAX6675 ktc(thermoCLK, thermoCS, thermoDO);
void setup() {
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);

  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);

  pinMode(PWM_MOTOR_1, OUTPUT);
  pinMode(PWM_MOTOR_2, OUTPUT);

  pinMode(CURRENT_SEN_1, OUTPUT);
  pinMode(CURRENT_SEN_2, OUTPUT);

  pinMode(EN_PIN_1, OUTPUT);
  pinMode(EN_PIN_2, OUTPUT);

  pinMode(SHUTTER_PIN,INPUT);
  pinMode(FOCUS_PIN,INPUT);
  Wire.begin(0x8);
  Serial.begin(9600);
  Wire.onReceive(stop_func);
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);
    delay(500);
    digitalWrite(EN_PIN_1, HIGH);
    digitalWrite(EN_PIN_2, HIGH);
  temp_error[0]=0;
  error_1[0]=0;
  error_2[0]=0;
}

void loop() {
  float f_cooling = 0;
  float f_heating = 0.8;
  float temp_current , t00 , t22 , t0=6000 , t2=6000, tpe=00000, temp_threshold=2.0, 
      temp_denature=95,
      t_cntrl=2.0,
      integral_pd=0,
      diff_pd,
      ctrl_pd=1,
      integral_pe=0,
      diff_pe,
      ctrl_pe=1,
      integral_0=0,
      diff_0,
      ctrl_0=1,
      integral_1=0,
      diff_1,
      ctrl_1=1,
      integral_2=0,
      diff_2,
      ctrl_2=1,
      K_P0=60,
      K_I0=4,
      K_d0=0,
      K_P1=50,
      K_I1=1.5,
      K_d1=0,
      K_P2=25,
      K_I2=1.5,
      temp_denature=95,
      K_d2=0;
  while(Serial.available()==0){}

  // put your main code here, to run repeatedly:
  if (iter<=N_cycle) {/*# of iterations*/
    /*denature*/
  while (ctrl_0==1) {
    int tpd=0000;
    if(iter==1) t00=tpd+t0;
    else t00=t0;
    mode=heat;
    f = f_heating;
    motorGo(MOTOR_1, heat,255);
  temp_current=ktc.readCelsius();
  
  temp_error[1]=temp_denature-temp_current;
  integral_0=integral_0+temp_error[1];
  diff_0=temp_error[1]-temp_error[0];
  Cycle_state = "*";
  Serial.print(Cycle_state);
  Serial.print("\t");
  if (iter>1) Serial.print("Den\t");
  else Serial.print("preDen\t");
  Serial.print(temp_current);
  Serial.print("\t");
   if (temp_error[1]>t_cntrl){ // If this condition was true then it means that we haven't reached the temp_denature so we have to set the drive to 255 and restart the timer.
      integral_0=0;
      diff_0=0;
      drive_0=255;
      time_current_stage=millis(); // Reseting the current stage time
    }   else if(temp_error[1]<(-5)){ //temp_error = temp_denature - temp_current if we are so much higher than the emergency cooling temprature, we change the peltier direction.
        drive_0=255;
        mode=cool;
        f = f_cooling;
      }
      else if (millis()-time_current_stage<t00) {
        if(temp_error[1]>temp_threshold){ //The timer starts when the temperature reaches temp_denature-temp_threshold
        time_current_stage=millis();
      }
      drive_0=K_P0*temp_error[1]+K_I0*integral_0+K_d0*diff_0;
      } else {
        ctrl_0=0;
      }
      if (drive_0>255) {
        drive_0=255;
      }
      if (drive_0<0) {
        drive_0=0;
      }
      motorGo(MOTOR_2, mode,f*drive_0);
//          Serial.print("\t");
       Serial.print(f*drive_0);
       Serial.print("\t");
       Serial.print("mode:");
       Serial.print(mode);
       Serial.print("\t");
       Serial.print(iter);
       Serial.print("\n");
           delay(500);
           temp_error[0]=temp_error[1];
   }
  drive_0=0;

if(iter==N_cycle) {t22=tpe+t2;time_2=millis();}
  while (ctrl_2==1) {

    if(iter==N_cycle) t22=tpe+t2;
    else t22=t2;
    mode=heat;
    f = f_heating;
    //motorGo(MOTOR_1, BRAKE,0);
    temp_current=ktc.readCelsius();
  
  int temp_2=60;
  error_2[1]=temp_2-temp_current;
  integral_2=integral_2+error_2[1];
  diff_2=error_2[1]-error_2[0];
//  Serial.print(millis());
Serial.print(Cycle_state);
Serial.print("\t");
  if (iter<N_cycle) Serial.print("Ext\t");
  else Serial.print("postExt\t");
  Serial.print(temp_current);
  Serial.print("\t");
  //Serial.print("\n");
   if (error_2[1]>t_cntrl){
      integral_2=0;
      diff_2=0;
      drive_2=255;
      time_2=millis();
    }  else if(error_2[1]<(-5)){ //Emergency Cooling
        drive_2=255;
        mode=cool;
        f = f_cooling;
        time_2=millis();
      }
      else if (millis()-time_2<t22) {
        if(error_2[1]>temp_threshold)
        { //The timer starts when the temperature reaches temp_denature-1.5
        time_2=millis();
      }
      drive_2=K_P2*error_2[1]+K_I2*integral_2+K_d2*diff_2;
      } else {
        ctrl_2=0;
        Serial.print("\n");
        captureImage();
      }
      if (drive_2>255) {
        drive_2=255;
      }
      if (drive_2<0) {
        drive_2=0;
      }
      motorGo(MOTOR_2, mode,f*drive_2);
//       Serial.print("\t");
       Serial.print(f*drive_2);
       Serial.print("\t");
       Serial.print("mode:");
       Serial.print(mode);
//       Serial.print("\tcycle : ");
//       Serial.print(iter);
       Serial.print("\t");
       Serial.print(iter);
       Serial.print("\n");
           delay(500);
           error_2[0]=error_2[1];
   }

  drive_2=0;
  motorGo(MOTOR_2, BRAKE,f*drive_2);
   ctrl_0=1;
   ctrl_1=1;
   ctrl_2=1;
   iter=iter+1;
  }
  else if(iter==N_cycle+1){
    Serial.print("All ");
    Serial.print(iter-1);
    Serial.print(" have been done successfully!\n");
    iter++;
  }
}

void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)         //Function that controls the variables: motor(0 ou 1), direction (cw ou ccw) e pwm (entra 0 e 255);
{
  if(motor == MOTOR_1)
  {
    if(direct == 0)
    {
      digitalWrite(MOTOR_A1_PIN, LOW);
      digitalWrite(MOTOR_B1_PIN, HIGH);
    }
    else if(direct == 1)
    {
      digitalWrite(MOTOR_A1_PIN, HIGH);
      digitalWrite(MOTOR_B1_PIN, LOW);
    }
    else
    {
      digitalWrite(MOTOR_A1_PIN, LOW);
      digitalWrite(MOTOR_B1_PIN, LOW);
    }

    analogWrite(PWM_MOTOR_1, pwm);
  }
  else if(motor == MOTOR_2)
  {
    if(direct == 0)
    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, HIGH);
    }
    else if(direct == 1)
    {
      digitalWrite(MOTOR_A2_PIN, HIGH);
      digitalWrite(MOTOR_B2_PIN, LOW);
    }
    else
    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, LOW);
    }

    analogWrite(PWM_MOTOR_2, pwm);
  }
}
void captureImage()
{
//  Serial.print("\nFocus -- ");
  Cycle_state = "#";
  pinMode(FOCUS_PIN,OUTPUT);
  digitalWrite(FOCUS_PIN,0);
  delay(1000);
  pinMode(FOCUS_PIN,INPUT);
  
  Serial.print(Cycle_state);
  Serial.print("\t");
  Serial.print("TRIGGER");
  Serial.print("\t");
  Serial.print(100);
  Serial.print("\t");
  Serial.print(100);
  Serial.print("\t");
  Serial.print("mode:3");
  Serial.print("\t");
  ////
//  Serial.print("Shutter -- ");
  pinMode(SHUTTER_PIN,OUTPUT);
  digitalWrite(SHUTTER_PIN,0);
  delay(6000);
  pinMode(SHUTTER_PIN,INPUT);
//  Serial.print("done\n");
}
void stop_func(int howMany){
  while(Wire.available()){
    char c = Wire.read();
  digitalWrite(resetPin, c);
//  delay(500);
//  digitalWrite(resetPin, HIGH);
  }
}
