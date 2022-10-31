
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
char Cycle_state;
char stage_name;
int N_cycle=40; // Total Number of Cycles
short mode;
float 
      temp_error[2],
      temp_error_extension[2],
      time_current_denature_stage,
      drive_0,
      time_1,
      drive_1,
      time_current_extension_stage,
      drive_2;
int iter = 1;
int thermoCO = 12;
int thermoCS = 10;
int thermoCLK = 11;
const int Number_of_fields = 3;
int fieldIndex = 0;
int values[Number_of_fields];
MAX6675 ktc(thermoCLK, thermoCS, thermoCO);
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
  temp_error_extension[0]=0;
}

void loop() {
  // Stage Type
  char Cycle_state;
  char stage_name;
  // Stage Times
  float time_predenature = 0,
        time_denature = 60000,
        time_extension = 6000,
        time_postextension = 60000,
        time_should_in_denature,
        time_should_in_extension;
  // Drive coefficients
  float f_heating = 0.8,
        f_cooling = 0.0,
        f = 0.0;
  // Stage Tempratures
  float temp_denature = 95,
        temp_extension = 60,
        temp_threshold = 2.0, 
        temp_current,
        integral_0=0,
        diff_0,
        ctrl_0=1,//motor control
        ctrl_1=1,//motor control
        integral_2=0,//wtf
        diff_2,
        ctrl_2=1,//motor control
        K_P0=60,
        K_I0=4,
        K_d0=0,
        K_P2=25,
        K_I2=1.5,
        K_d2=0,
        total_drive1=0,
        total_drive2=0;
        

  while(Serial.available() == 0){}

  if (iter<=N_cycle) { // If we havent reached the end of the Cycle do this
  
  while (ctrl_0 == 1) {
    if(iter == 1) time_should_in_denature = time_predenature; // If we are in the first cycle, do a predenaturation stage
    else time_should_in_denature=time_denature; // If we are over the first stage, then we just need to do denaturation
    mode = heat;
    f = f_heating;
    motorGo(MOTOR_1, heat, 255);
  temp_current=ktc.readCelsius();
  temp_error[1]=temp_denature-temp_current; // Difference in the current temprature and the denaturation error
  integral_0=integral_0+temp_error[1];
  diff_0=temp_error[1]-temp_error[0];

  if (iter>1) stage_name='D';
  else stage_name='P';

   if (temp_error[1]>temp_threshold){ // If this condition was true then it means that we haven't reached the temp_denature so we have to set the drive to 255 and restart the timer.
      integral_0=0;
      diff_0=0;
      drive_0=255;
      time_current_denature_stage=millis(); // Reseting the current stage time
    }   else if(temp_error[1]<(-5)){ //temp_error = temp_denature - temp_current if we are so much higher than the emergency cooling temprature, we change the peltier direction.
        drive_0=255;
        mode=cool;
        f = f_cooling;
      }
      else if (millis()-time_current_denature_stage<time_should_in_denature) { // This checks if we have spent enough time on the current stage.
        if(temp_error[1]>temp_threshold){ //The timer starts when the temperature reaches temp_denature-temp_threshold
        time_current_denature_stage=millis();
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
       Cycle_state = '*';
       total_drive1 = drive_0*f;
       show_result(Cycle_state , stage_name , temp_current , total_drive1 , iter);
           delay(500);
           temp_error[0]=temp_error[1];
   }
  drive_0=0;

if(iter==N_cycle) {time_should_in_extension=time_postextension+time_extension;time_current_extension_stage=millis();}
  while (ctrl_2==1) {

    if(iter==N_cycle) time_should_in_extension=time_postextension+time_extension;
    else time_should_in_extension=time_extension;
    mode=heat;
    f = f_heating;
    temp_current=ktc.readCelsius();
  
  
  temp_error_extension[1]=temp_extension-temp_current;
  integral_2=integral_2+temp_error_extension[1];
  diff_2=temp_error_extension[1]-temp_error_extension[0];
  if (iter<N_cycle) stage_name='X';
  else stage_name='N';

   if (temp_error_extension[1]>temp_threshold){
      integral_2=0;
      diff_2=0;
      drive_2=255;
      time_current_extension_stage=millis();
    }  else if(temp_error_extension[1]<(-5)){ //Emergency Cooling
        drive_2=255;
        mode=cool;
        f = f_cooling;
        time_current_extension_stage=millis();
      }
      else if (millis()-time_current_extension_stage<time_should_in_extension) {
        if(temp_error_extension[1]>temp_threshold)
        { //The timer starts when the temperature reaches temp_denature-1.5
        time_current_extension_stage=millis();
      }
      drive_2=K_P2*temp_error_extension[1]+K_I2*integral_2+K_d2*diff_2;
      } else {
        ctrl_2=0;
        // Serial.print("\n");
        captureImage();
      }
      if (drive_2>255) {
        drive_2=255;
      }
      if (drive_2<0) {
        drive_2=0;
      }
      motorGo(MOTOR_2, mode,f*drive_2);
      total_drive2=f*drive_2;
       show_result('*' , stage_name , temp_current , total_drive2 , iter);

           delay(500);
           temp_error_extension[0]=temp_error_extension[1];
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
  pinMode(FOCUS_PIN,OUTPUT);
  digitalWrite(FOCUS_PIN,0);
  delay(1000);
  pinMode(FOCUS_PIN,INPUT);
  show_result('#' , 'T' , 0 , 0 , 0);

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
  }
}
void show_result(char state , char stage_name , float temp , float drive , int iter ){

    Serial.print(state);
    Serial.print("-");
    Serial.print(stage_name);
    Serial.print("-");
    Serial.print(temp);
    Serial.print("-");
    Serial.print(drive);
    Serial.print("-");
    Serial.print(iter);
    Serial.print("\n");

}