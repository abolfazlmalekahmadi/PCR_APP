//MOTOR_2 is thermoelectric element & MOTOR_1 is the fan

#include "max6675.h"

// for taking picture
#define SHUTTER_PIN A4
#define FOCUS_PIN A5
// to set cooling and heating mode
#define cool    1
#define heat    2
//MOTOR 2 (Peltier)---> these two variables determine the direction of heating and cooling in peltier (0 1 or 1 0)
#define MOTOR_A1_PIN 7
#define MOTOR_B1_PIN 8

//MOTOR 1 (Fan)---> these two variables determine the direction of rotation in fan (0 1 or 1 0)
#define MOTOR_A2_PIN 4
#define MOTOR_B2_PIN 9

// PWM for determining input power to motors
#define PWM_MOTOR_1 5
#define PWM_MOTOR_2 6

// to enable motor drivers  ??????????????????????????????????????????????????????????????????????
#define EN_PIN_1 A0
#define EN_PIN_2 A1

// nominating motors
#define MOTOR_1 0
#define MOTOR_2 1

// define stages (denaturation:1 annealing:2 extension:3)
#define denaturation   1
#define annealing      2
#define extension      3

int   N_cycle=40,            // number of total cycles
      stage=1;               // contains (denaturation=1 or annealing=2 or extension=3) to determine in which stage we are
short mode;                  // heat or cool modes (heat:2  &  cool:1)
float f=0.6;                // contains cooling or heating power coefficient
float f_cooling = 1;         // cooling power coefficient
float f_heating = 0.8;       // heating power coefficient
float temp_dena=95,       
      temp_anneal=60,
      temp_exten=65,
      temp_read,             // read temperature from sensor
      
      timeholder,                 // time variable in each stage
      time_preden=000,         // desired time for predenaturation
      time_dena=30000,            // desired time for denaturation
      time_anneal=0,          // desired time for annealing
      time_exten=30000,           // desired time for extension
      time_postexten=0,      // desired time for post extension
      
      t_fluc=2.0,
      t_cntrl=2.0,
      
      integral_dena=0,            // integral of error in denaturation stage for PID
      diff_dena,                  // derivative of error in denaturation stage for PID
      
      integral_anneal=0,          // integral of error in annealing stage for PID
      diff_anneal,                // derivative of error in annealing stage for PID
      
      integral_exten=0,           // integral of error in extension stage for PID
      diff_exten,                 // derivative of error in extension stage for PID
      
      /*K_P0=27.94,                    // PID coefficients in denaturation
      K_I0=3.5,
      K_d0=0,*/

      K_P0=60,                    // PID coefficients in denaturation
      K_I0=4,
      K_d0=0,
      
      K_P1=50,                    // PID coefficients in annealing
      K_I1=1.5,
      K_d1=0,
      
      K_P2=25,                    // PID coefficients in extension
      K_I2=1.5,
      K_d2=0,
      
      error_0[2],                 // Error for PID in denaturation
      error_1[2],                 // Error for PID in annealing
      error_2[2],                 // Error for PID in extension

      lowerlimit=-5,              // temperature limits for control
      upperlimit=2, 
      
      time_0,
      drive_0,                    // motor driver signal
      time_1,
      drive_1,
      time_2,
      drive_2;
      
int iter=1;           // cycle number

// for reading temperature from Max6675
int thermoDO = 12;
int thermoCS = 10;
int thermoCLK = 11;
MAX6675 ktc(thermoCLK, thermoCS, thermoDO);

//-----------------------------------------------------------------------------------------------------------------------------------------------
void setup() {
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);

  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);

  pinMode(PWM_MOTOR_1, OUTPUT);
  pinMode(PWM_MOTOR_2, OUTPUT);

  pinMode(EN_PIN_1, OUTPUT);
  pinMode(EN_PIN_2, OUTPUT);

  pinMode(SHUTTER_PIN,INPUT);
  pinMode(FOCUS_PIN,INPUT);
  
  Serial.begin(9600);

  delay(500);
  digitalWrite(EN_PIN_1, HIGH);
  digitalWrite(EN_PIN_2, HIGH);
  
  error_0[0]=0;
  error_1[0]=0;
  error_2[0]=0;
  
  /*Serial.print("\t\t********Denaturation Temp: ");
  Serial.print(temp_dena);
  Serial.print("**********\n");
  Serial.print("\t\t********Annealing Temp: ");
  Serial.print(temp_anneal);
  Serial.print("**********\n");
  Serial.print("\t\t********Extension Temp: ");
  Serial.print(temp_exten);
  Serial.print("**********\n");*/

}
//-------------------------------------------------------------------------------------------------------------------------------------------------
void loop() 
{
  while(Serial.available()==0){}
  if (iter<=N_cycle)
   {
      motorGo(MOTOR_1,heat,0);
      denaturation_fun();
     // stage=2;             // change stage to annealing
     // annealing_fun();
      motorGo(MOTOR_1,heat,f_cooling*255);
      stage=3;               // change stage to extension
      extension_fun();
      stage=1;               // change stage to denaturation for next cycle
    }
  /*else
    {
      Serial.print("All ");
      Serial.print(N_cycle);
      Serial.print(" have been done successfully!\n");
    }*/
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------
void denaturation_fun()
{
  error_0[0]=0;
  integral_dena=0;
  while (stage==denaturation) 
  {
    if(iter==1) timeholder=time_preden+time_dena;
    else timeholder=time_dena;
    mode=heat;
    f = f_heating;
    temp_read=ktc.readCelsius();
    error_0[1]=temp_dena-temp_read;
    
    //Serial.print(millis());
    //if (iter>1) Serial.print("\tDen\t");
    //else Serial.print("\tpreDen\t");
    Serial.print("*\t");
    Serial.print("Den\t");
    Serial.print(temp_read);
    if (error_0[1]>upperlimit)            // heat with max power
      {
      integral_dena=0;
      diff_dena=0;
      drive_0=255;
      time_0=millis();          
                                  
      }
    else if(error_0[1]<lowerlimit)        //Emergency Cooling (cooling with max power)
      {   
        drive_0=255;
        mode=cool;
        f = f_cooling;
        time_0=millis();
        
      }
    else if (millis()-time_0<timeholder)
      {
        if(error_0[1]>t_fluc){time_0=millis();}
        integral_dena=integral_dena+error_0[1];
        diff_dena=error_0[1]-error_0[0];
        drive_0=K_P0*error_0[1]+K_I0*integral_dena+K_d0*diff_dena;
        
      }
    else 
      {
        stage=2;
      }
    if (drive_0>255) {drive_0=255;}
    if (drive_0<0) {drive_0=0;}
    motorGo(MOTOR_2,mode,f*drive_0);
    //Serial.print("\t");
    //Serial.print(f*drive_0);
    //Serial.print("\t mode : ");
    //Serial.print(mode);
    //Serial.print("\tcycle : ");
    //Serial.print(iter);
    //Serial.print("\t*");
    Serial.print("\n");
    delay(500);
    error_0[0]=error_0[1];
   }
  drive_0=0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
void annealing_fun()
{
//   motorGo(MOTOR_2, BRAKE,drive_0);
//   motorGo(MOTOR_1, cool,190);
//  Annealing
//   while (ctrl_1==1) {/*anneal1*/
//     mode=cool;
//     f = f_cooling;
//     //motorGo(MOTOR_1, heat,255);
//   temp_read=ktc.readCelsius();
//   error_1[1]=temp_read-temp_anneal;
//   Serial.print(millis());
//   Serial.print("\tAneal1\t");
//   Serial.print(temp_read);
//    if (error_1[1]>t_fluc){
//     drive_1=255;
//       motorGo(MOTOR_2, mode,f*drive_1);
//    } else {
//     ctrl_1=0;
//     drive_1=0;
//     motorGo(MOTOR_2, BRAKE,f*drive_1);
//     time_1=millis();
//    }
//     Serial.print("\t");
//        Serial.print(f*drive_1);
//        Serial.print("\t mode : ");
//        Serial.print(mode);
//        Serial.print("\t cycle : ");
//        Serial.print(iter);
//        Serial.print("\n");
//     delay(500);
//   }
//   ctrl_1=1;
//   while (ctrl_1==1) {/*aneal2*/
//     mode=heat;
//     f = f_heating;
//     //motorGo(MOTOR_1, BRAKE,0);
//   temp_read=ktc.readCelsius();
//   error_1[1]=temp_anneal-temp_read;
//   integral_anneal=integral_anneal+error_1[1];
//   diff_anneal=error_1[1]-error_1[0];
//    Serial.print(millis());
//   Serial.print("\tAneal2\t");
//   Serial.print(temp_read);
//    if (error_1[1]>t_cntrl){
//       integral_anneal=0;
//       diff_anneal=0;
//       drive_1=255;
//       time_1=millis();
//     } else if(error_1[1]<(-5)){ //Emergency Cooling
//         drive_1=255;
//         mode=cool;
//         f = f_cooling;
//       }
//       else if (millis()-time_1<time_anneal) {
//         if(error_1[1]>t_fluc){ //The timer starts when the temperature reaches temp_dena-t_fluc
//         time_1=millis();
//          }
//       drive_1=K_P1*error_1[1]+K_I1*integral_anneal+K_d1*diff_anneal;
//       } else {
//         ctrl_1=0;
//       }
//       if (drive_1>255) {
//         drive_1=255;
//       }
//       if (drive_1<0) {
//         drive_1=0;
//       }
//       motorGo(MOTOR_2, mode,f*drive_1);
//       Serial.print("\t");
//        Serial.print(f*drive_1);
//        Serial.print("\t mode : ");
//        Serial.print(mode);
//        Serial.print("\t cycle : ");
//        Serial.print(iter);
//        Serial.print("\n");
//       delay(500);
//       error_1[0]=error_1[1];
//    }
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
void extension_fun()
{
  error_2[0]=0;
  integral_exten=0;
  while (stage==3) 
  {

    if(iter==N_cycle) timeholder=time_postexten+time_exten;
    else timeholder=time_exten;
    mode=heat;
    f = f_heating;
    temp_read=ktc.readCelsius();
    error_2[1]=temp_exten-temp_read;
    
    //Serial.print(millis());
    //if (iter<N_cycle) Serial.print("\tExt\t");
    //else Serial.print("\tpostExt\t");
    Serial.print("*\t");
    Serial.print("Extension\t");
    Serial.print(temp_read);
    if (error_2[1]>upperlimit)    // heat with max power
     {
        integral_exten=0;
        diff_exten=0;
        drive_2=255;
        time_2=millis();
     }
    else if(error_2[1]<lowerlimit)      //Emergency Cooling
     { 
        drive_2=255;
        mode=cool;
        f = f_cooling;
        time_2=millis();
     }
    else if (millis()-time_2<timeholder)
     {
        integral_exten=integral_exten+error_2[1];
        diff_exten=error_2[1]-error_2[0];
        if(error_2[1]>t_fluc){time_2=millis();}
        drive_2=K_P2*error_2[1]+K_I2*integral_exten+K_d2*diff_exten;
     }
    else
     {
        stage=1;
        //captureImage();
     }
      if (drive_2>255) {drive_2=255;}
      if (drive_2<0) {drive_2=0;}
      motorGo(MOTOR_2, mode,f*drive_2);
      //Serial.print("\t");
      //Serial.print(f*drive_2);
      //Serial.print("\tmode : ");
      //Serial.print(mode);
      //Serial.print("\tcycle : ");
      //Serial.print(iter);
      //if (stage==extension) Serial.print("\t*");
      //else Serial.print("\t#");
      Serial.print("\n");
      delay(500);
      error_2[0]=error_2[1];
   }

    drive_2=0;
    captureImage();
    iter=iter+1;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------
void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)         //Function that controls the variables: motor(0 or 1), direction (cw or ccw) , pwm (0 to 255);
{
  if(motor == MOTOR_1)
  {
    if(direct == cool)
    {
      digitalWrite(MOTOR_A1_PIN, LOW);
      digitalWrite(MOTOR_B1_PIN, HIGH);
    }
    else if(direct == heat)
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
    if(direct == cool)
    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, LOW);
    }
    else if(direct == heat)
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
  Serial.print("#\t");
  Serial.print("TRIGGER\t");
  Serial.println(temp_exten);
  //Serial.print("\nFocus -- ");
  pinMode(FOCUS_PIN,OUTPUT);
  digitalWrite(FOCUS_PIN,0);
  delay(6000);
  pinMode(FOCUS_PIN,INPUT);
  //Serial.print("done\n");
  ////
  //Serial.print("Shutter -- ");
  pinMode(SHUTTER_PIN,OUTPUT);
  digitalWrite(SHUTTER_PIN,0);
//  delay(5000);
  pinMode(SHUTTER_PIN,INPUT);
  //Serial.print("done\n");
}
