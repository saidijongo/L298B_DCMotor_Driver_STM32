/*E-bike Arduino Brushed DC motor electronic speed controller
  Tutorial: https://electronoobs.com/eng_arduino_tut126.php
  Schematic: https://electronoobs.com/eng_arduino_tut126_sch1.php
  Code: https://electronoobs.com/eng_arduino_tut126_code1.php
  Gerbers: https://electronoobs.com/eng_arduino_tut126_gerber1.php
  Video: https://www.youtube.com/watch?v=GtXMHM78Xbo 

  Current limit: I'm using the 20A ACS712 so the multiplier is 0.100V. Change that
  below if you use a differen one. Then change your current limit if you want.

  RPM limit: I'm assuming the wheel has a magnet and a hall sensor. That means one
  pulse per rotation. In my case I've place limit of RMPs to 20, that means 20 wheel
  rotations per minute. If you have a different sensor that would create more than one
  pulse per rotation, change that value. For example: If your sensor creates 5 pulses
  each rptation, you should set rpm_limit to 5*20 = 100 if you want 20 rotation limit

  PWM pulse: The pwm is invertet because we use a NPN BJT at the MOSFET gate with a pullup.
  If you change the scheamtic to a PNP and a pulldown, just go below and where you see
  analogWrite(PWM, 255 - pwm_value); just change that to analogWrite(PWM, pwm_value);
  That would invert the PWM signal
  */


///////////////////////////////////////EDITABLE VALUES/////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
float current_limit = 7.0;          //Current limit in Ampers
int   rpm_limit = 20;               //RPM limit before enabling the motor. User must push a bit the bike before throttle 
int   Delay = 1;                    //loop delay in ms
float multiplier = 0.100;           //Sensibility in Volts/Ampers for the 20A ACS712 model (100mV)
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


//Inputs/Outputs
int throttle_in = A1;       //Throttle potentiometer (0V - 5V)
int current_in = A0;        //From ACS712 IC
int speed_in = 8;           //From wheel hall sensor
int break_in = 9;           //From break switch
int breaklight = 11;        //To the break light LEDs
int indicator = 10;         //A small indicator LED (shows when contorller is ready)
int PWM = 3;                //Connected to the BJT base


//Variables
unsigned long previousMillis = 0;   //Counter used for loop delay
int pwm_value = 0;
bool power_enable = false;
unsigned long previous_count, current_count;
byte pulse_state;
int one_rotation_time = 0;


void setup() {
  pinMode(throttle_in,INPUT);
  pinMode(current_in,INPUT);
  pinMode(speed_in,INPUT);
  pinMode(break_in,INPUT_PULLUP);

  pinMode(breaklight,OUTPUT);
  pinMode(indicator,OUTPUT);
  pinMode(PWM,OUTPUT);
  digitalWrite(breaklight,LOW);
  digitalWrite(indicator,LOW);  

  //Set D8 (speed_in) to trigger interrupt (we use this to read RPMs)
  PCICR |= (1 << PCIE0);        //enable PCMSK0 scan                                                 
  PCMSK0 |= (1 << PCINT0);      //Set pin D8 trigger an interrupt on state change. 
  
  TCCR2B = TCCR2B & B11111000 | B00000010;    // Set D3 PWM frequency to 3921.16 Hz
  digitalWrite(PWM,HIGH);                     // We set it HIGH, so the BJT is on and we have 
                                              // GND at the MSOFET gate (so, MOSFET OFF)

}

void loop() {
  unsigned long currentMillis = millis(); 
  if(currentMillis - previousMillis >= Delay){  
    previousMillis += Delay;                                            //Increase loop delay by "Delay" value

    float rpm = 60000.0 / one_rotation_time;                            //rotations in one minute (60.000 ms)
        
    if(break_in){                                                       //Only run the loop if break_in is HIGH (it has pullup)
      digitalWrite(breaklight,LOW);                                     //Turn off breaklight
      int pwm_setpoint =  map(analogRead(throttle_in),0,1023,0,255);    //Read setpoint for throttle
      if(rpm > rpm_limit && pwm_setpoint < 5){                          //We enable power only if RPM is high and throttle low
        power_enable = true;
        digitalWrite(indicator,HIGH);                                   //We indicate that we can increase throttle
      }
      if(rpm < rpm_limit){                                              //When RPM is low, we disable power for safety
        power_enable = false;
        digitalWrite(indicator,LOW);                                    //We can't increase throttle
      }
            
      if(pwm_setpoint > 5){                                             //Only start increasing after pwm_setpoint higher than 5
        float SensorRead = analogRead(current_in)*(5.0 / 1023.0);       //We read the sensor output  
        float Current = (SensorRead-2.5)/multiplier;                    //Calculate the current value
          
        if(pwm_setpoint > pwm_value){                                   //Increase PWM (by throttle read)
          pwm_value = pwm_value + 1;
          if(pwm_value > 255){
            pwm_value = 255;
          }
        }
        else if(pwm_setpoint < pwm_value){                              //Decrease PWM (by throttle read)
          pwm_value = pwm_value - 1;
          if(pwm_value < 0){
            pwm_value = 0;
          }
        }
          
        if(Current > current_limit){                                    //Decrease PWM (by current limit)
          pwm_value = pwm_value - 1;
          if(pwm_value < 0){
            pwm_value = 0;
          }
        }
        
        if(power_enable){ 
          analogWrite(PWM, 255 - pwm_value);
        }
      }//End if "pwm_setpoint > 5"
  
      else{
        pwm_value = 0;
        analogWrite(PWM, 255 - pwm_value);
      }//End else "pwm_setpoint > 5"
    }//end if break in

    else{                                   //break_in is low so user pressed break
      pwm_value = 0;
      analogWrite(PWM, 255 - pwm_value);
      digitalWrite(breaklight,HIGH);
    }//end of else "break in"
  }//end loop "if currentMillis"
}//end "void loop"











/*This is the interruption routine on pin change
  in this case for digital pin D8 which is the Speed_in input
  We asume taht the Hall sensor circuit will give a HIGH pulse each time
  the magnet passes in front of it and that means one more rotation...
*/

ISR(PCINT0_vect){
  //First we take the current count value in milli seconds using the millis() function
  current_count = millis();
  ///////////////////////////////////////
  if(PINB & B00000001){                                     //We make an AND with the pin state register, We verify if pin 8 is HIGH???
    if(pulse_state == 0){                                   //If the last state was 0, then we have a state change...
      pulse_state = 1;                                      //Store the current state into the last state for the next loop
      one_rotation_time = current_count - previous_count;   //We make the time difference. Rotation_time is current_time - previous_count in milli-seconds.
      previous_count = current_count;                       //Set counter_1 to current value for next loop.
    }
  }
  else if(pulse_state == 1){                                //If pin 8 is LOW and the last state was HIGH then we have a state change      
    pulse_state = 0;                                        //Store the current state into the last state for the next loop    
  }
}
