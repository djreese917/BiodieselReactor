#include <TimerThree.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//Deprecated / usage unknown pins
#define motor_ff1_pin 26     //usage unknown
#define motor_ff2_pin 27     //usage unknown
#define motor_dir_pin 10     //usage unknown
#define thermocouple0_pin 0  //deprecated (thermometer used now)
#define thermocouple1_pin 1  //deprecated (thermometer used now)
#define light_sensor_pin 3   //deprecated (light sensor not being used
#define start_reset_pin 23   //deprecated (switch board no longer used)
#define em_stop_pin 24       //deprecated (switch board no longer used)
#define resume_pin 25        //deprecated (switch board no longer used)
#define motor_reset_pin 37   //deprecated (motor doesn't have a reset, what?)
#define light_sensor_led_pin 53 //deprecated (switch board no longer used)
#define rxn_led_pin 39          //deprecated (switch board no longer used)
#define g_xfer_led_pin 40       //deprecated (switch board no longer used)
#define bd_xfer_led_pin 41      //deprecated (switch board no longer used) 
#define line_flush_led_pin 42   //deprecated (switch board no longer used)
#define heater_led_pin 11       //deprecated (switch board no longer used)
#define em_stop_led_pin 52      //deprecated (switch board no longer used)
#define light_sensor_oe_pin 38  //deprecated (light sensor not used)
//43, 44, 45. 46, 47, 48, 49, 50 These were all used as part of the wash ssg, which doesnt exist anymore

//State machine states
#define acid_heating 13
#define acid_mixing_methanol 14
#define acid_mixing_h2so4 15
#define methoxide_heating 16
#define add_methoxide 0
#define rxn_settle 1
#define g_out_v_open 2
#define g_out_v_close 3
#define wash_start 4
#define wash_mix 5
#define wash_settle 6
#define water_flush 7
#define end_wash_cycle 8
#define line_flush 9
#define f_biodiesel_xfer 10
#define end_p 11
#define hold 12

//Water heater state machine states
#define heater_on 0
#define heater_off 1
#define stop_wh 2

//Timer related defines
#define stir_two_time 3600 //3600 seconds in an hour
#define stir_acid_time 300 //300 seconds in 5 min
#define rxn_time 1800 //1800 seconds is half an hour 
#define rxn_settle_time 21600 //21600 seconds in six hours
#define g_out_time 10 //acutally ten seconds
#define wash_time 400 //water is added for 6 minutes and 40 seconds
#define mix_time 900 // mix for 15 minutes
#define wash_settle_time 3600 //let the washed mixture settle for 1 hour
#define line_flush_time 5 //flush the line before draining biodiesel from reactor tank
#define biodiesel_xfer_time 90 //drain the biodiesel for 90 seconds

//Temperature thresholds
#define acid_temp_max 81//96
#define acid_temp_min 79//94
#define base_temp_max 131
#define base_temp_min 129

//MISC defines
#define MAX_Q 12
#define ONE_WIRE_BUS 40 //pin that temp sensor is plugged into
#define motor_pin 42
#define heater_pin 12
#define pump_pin 51

//MISC Variables
int v_f_pin [6]={28, 29, 30, 31, 32, 33}; //fluid valve pins
int v_a_pin [3]={34, 35, 36,}; //air valves
int light_sensor=0;

//timer related variables
int rxn_timer=0;
int rxn_settle_timer=0;
int g_out_timer=0;
int wash_timer=0;
int wash_mix_timer=0;
int wash_settle_timer=0;
int line_flush_timer=0;
int f_biodiesel_timer=0;
int stir_acid_timer=0;
int stir_two_timer=0;

//state related variables
int q=0;//State indicator variable
int state_main=acid_heating;
int state_water=heater_on;
int wh_q; //water heater state indicator variable;
int wash_count=0; //wash counter to check for wash loop completion

//Temperature related variables
int tc0 = 0;
int tc1 = 0;
int tc_max = 0;
int tc_min = 0;
int seconds = 0;
float tempF = 0;
int temp_met = 0;

//Specifically serial related variables
int goToNextState = 0; //used for changing state 
int resume = 0;
int emergencyStop = 0; 

//Temperature sensor setup
OneWire oneWire(40); //CHANGE THIS BACK TO 3
DeviceAddress outakeTherm = {0x28, 0xDF, 0x70, 0xFB, 0x05, 0x00, 0x00, 0x43};
DallasTemperature sensors(&oneWire); 

//Timer related call back functions
void SetRxnTime()
{
   seconds++;
  
   if (seconds > rxn_time)
      rxn_timer = 1;
} 

void SetRxnSettleTime()
{
   seconds++; 
  
   if (seconds > rxn_settle_time)
      rxn_settle_timer = 1;
} 

void SetVentTimer()
{
   seconds++;
  
   if (seconds > g_out_time)
   g_out_timer = 1; 
} 

void SetWashTime()
{
   seconds++;
  
   if (seconds > wash_time)
      wash_timer = 1;
}

void SetMixTimer()
{
   seconds++;
  
   if (seconds > mix_time)
      wash_mix_timer = 1;
}

void SetWashSettleTimer()
{
   seconds++;
  
   if (seconds > wash_settle_time)
      wash_settle_timer = 1;
} 
     
void SetLineFlushTimer()
{
   seconds++;
  
   if (seconds > line_flush_time)
      line_flush_timer = 1;
}

void SetBiodieselXferTimer()
{
   seconds++;
   
   if (seconds > biodiesel_xfer_time)
      f_biodiesel_timer = 1;
} 

void SetStirAcidTime()
{
   seconds++;
   
   if (seconds > stir_acid_time)
      stir_acid_timer = 1; 
}

void SetStirTwo()
{
   seconds++;
  
   if (seconds > stir_two_time)
      stir_two_timer = 1;
}
      
//System setup
void setup() {
   pinMode(v_f_pin[0], OUTPUT); //Valve 4
   pinMode(v_f_pin[1], OUTPUT); //Valve 5
   pinMode(v_f_pin[2], OUTPUT); //Valve 3
   pinMode(v_f_pin[3], OUTPUT); //Valve 6
   pinMode(v_f_pin[4], OUTPUT); //Valve 7
   pinMode(v_f_pin[5], OUTPUT); //Valve 8
   pinMode(v_a_pin[0], OUTPUT); //Valve 11
   pinMode(v_a_pin[1], OUTPUT); //Valve 12? (can't tell if 12 or 13)
   pinMode(v_a_pin[2], OUTPUT); //Valve 13? (can't tell if 12 or 13)
   pinMode(motor_pin, OUTPUT);
   pinMode(heater_pin, OUTPUT);
   pinMode(pump_pin, OUTPUT);
   digitalWrite(v_f_pin[0],LOW); //Valve 4
   digitalWrite(v_f_pin[1],LOW); //Valve 5
   digitalWrite(v_f_pin[2],LOW); //Valve 3
   digitalWrite(v_f_pin[3],LOW); //Valve 6
   digitalWrite(v_f_pin[4],LOW); //Valve 7
   digitalWrite(v_f_pin[5],LOW); //Valve 8
   digitalWrite(v_a_pin[0],LOW); //Valve 11
   digitalWrite(v_a_pin[1],LOW); //Valve 12? (can't tell if 12 or 13)
   digitalWrite(v_a_pin[2],LOW); //Valve 13? (can't tell if 12 or 13)
   digitalWrite(heater_pin,LOW);
   Timer3.initialize(1000000); //timer with 1 second period
   Serial.begin(9600);
   while (!Serial)
      ;
   sensors.begin();
   sensors.setResolution(outakeTherm, 10);
   Serial.println("Setup done!"); 
}

void loop(){
   switch(state_main)
   {
      case acid_heating: //Heat the WVO up to 95 F
         q = 12;
         tc_max = acid_temp_max;
         tc_min = acid_temp_min; 
         digitalWrite(pump_pin, HIGH); 
         //Probably want this to just stay here, since chemicals should be added 
         //AFTER the temp reaches 95 F
         if (emergencyStop)
            state_main=hold;
         else if (temp_met || goToNextState)
         {
            Timer3.detachInterrupt();
            goToNextState = 0; 
            seconds = 0;
            Timer3.attachInterrupt(SetStirAcidTime); 
            state_main = acid_mixing_methanol; 
         }
         else
            state_main = acid_heating; 
         break;
      case acid_mixing_methanol: //add methanol at this point
         q = 13;
         digitalWrite(motor_pin,HIGH); 
         digitalWrite(pump_pin, HIGH); 
         
         if(emergencyStop)
            state_main = hold;
         else if (stir_acid_timer || goToNextState)
         {
            goToNextState = 0;
            Timer3.detachInterrupt();
            seconds = 0;
            Timer3.attachInterrupt(SetStirTwo); 
            state_main = acid_mixing_h2so4; 
         }
         else 
            state_main = acid_mixing_methanol;
         break;
      case acid_mixing_h2so4: //add sulfuric acid
         q = 14;
         digitalWrite(motor_pin,HIGH); //Note that these are only repeated for clarity
         digitalWrite(pump_pin,HIGH);  //The pin settings carry over from previous states
         if (emergencyStop)
            state_main = hold;
         else if (stir_two_timer || goToNextState)
         {
            goToNextState = 0;
            state_main = methoxide_heating;
            tc_max = base_temp_max;
            tc_min = base_temp_min; //update temps for the next step
         }
         else 
            state_main = acid_mixing_h2so4;
        
         break; 
      case methoxide_heating: //heat mixture up to proper temp before adding methoxide
         q = 15;         
         tc_max = base_temp_max;
         tc_min = base_temp_min; 
         digitalWrite(motor_pin,HIGH);
         digitalWrite(pump_pin, HIGH); 
         if (emergencyStop)
            state_main = hold;
         else if (temp_met || goToNextState)
         {
            goToNextState = 0;
            Timer3.detachInterrupt();
            seconds = 0; 
            Timer3.attachInterrupt(SetRxnTime); 
            state_main = add_methoxide;
         }
         else
            state_main = methoxide_heating; 
         break;
      case add_methoxide: //add methoxide add this point.
         q=0;
         digitalWrite(motor_pin,HIGH);
         digitalWrite(pump_pin, HIGH); 
         if (emergencyStop) //this needs to be updated to something from the GUI
            state_main=hold;
         else if (rxn_timer || goToNextState) 
         {
            goToNextState = 0;
            Timer3.detachInterrupt(); 
            seconds = 0; //reset seconds for the next timer
            Timer3.attachInterrupt(SetRxnSettleTime); //set the appropriate handler
            state_main=rxn_settle;
         } 
         else 
            state_main=add_methoxide;
         break;
      case rxn_settle:
         q=1;
         digitalWrite(pump_pin, LOW); 
         digitalWrite(motor_pin,LOW);
         digitalWrite(rxn_led_pin,HIGH);
         if (emergencyStop) 
            state_main=hold;
         else if (rxn_settle_timer || goToNextState) 
         {
            goToNextState = 0; 
            Timer3.detachInterrupt();
            seconds = 0;
            state_main=g_out_v_open;  
         } 
         else 
            state_main=rxn_settle;
         break;
      case g_out_v_open:
         q=2;
         digitalWrite(v_f_pin[0],HIGH); //Valve 5 on diagram?
         digitalWrite(v_f_pin[1],HIGH); //Valve 4 on diagram?
         digitalWrite(v_a_pin[0],HIGH); //Valve 11 on diagram?  
         
         if (emergencyStop) 
            state_main=hold;
         else if (goToNextState) 
         {
            goToNextState = 0;
            seconds = 0; 
            Timer3.detachInterrupt();
            Timer3.attachInterrupt(SetVentTimer); 
            state_main=g_out_v_close;
         } 
         else 
            state_main=g_out_v_open;
         break;
      case g_out_v_close:
         q=3;
         digitalWrite(v_f_pin[0],LOW);
         digitalWrite(v_f_pin[1],LOW);
         digitalWrite(v_a_pin[0],LOW);
         
         if (emergencyStop) 
            state_main=hold;
         else if (g_out_timer || goToNextState) 
         {
            goToNextState = 0; 
            Timer3.detachInterrupt();
            seconds = 0;
            Timer3.attachInterrupt(SetWashTime); 
            state_main=wash_start;
         }
         else 
            state_main=g_out_v_close;
         break;
      case wash_start:
         q=4;
         digitalWrite(v_a_pin[1],HIGH); //Valve 12? 
         digitalWrite(v_a_pin[2],HIGH); //Valve 13?
         digitalWrite(v_f_pin[2],HIGH); //Valve 3
         
         if (emergencyStop) 
            state_main=hold;
         else if (wash_timer || goToNextState) 
         {
            goToNextState = 0; 
            Timer3.detachInterrupt();
            seconds = 0;
            Timer3.attachInterrupt(SetMixTimer); 
            state_main=wash_mix;
         }
         else 
            state_main=wash_start;
         break;
      case wash_mix:
         q=5;
         digitalWrite(v_a_pin[1],LOW);
         digitalWrite(v_a_pin[2],LOW);
         digitalWrite(v_f_pin[2],LOW);
         digitalWrite(motor_pin,HIGH);
         
         if (emergencyStop) 
            state_main=hold;
         else if (wash_mix_timer || goToNextState) 
         {
            goToNextState = 0;
            Timer3.detachInterrupt();
            seconds = 0;
            Timer3.attachInterrupt(SetWashSettleTimer); 
            state_main=wash_settle;
         }
         else 
            state_main=wash_mix;
         break;
      case wash_settle:
         q=6;
         digitalWrite(motor_pin, LOW);
         if (emergencyStop) 
            state_main=hold;
         else if (wash_settle_timer || goToNextState)
         {
            goToNextState = 0;  
            state_main=water_flush;
         }
         else 
            state_main=wash_settle;
         break;
      case water_flush:
         q=7;
         digitalWrite(v_f_pin[0],HIGH); //Valve 4
         digitalWrite(v_f_pin[3],HIGH); //Valve 6
         digitalWrite(v_f_pin[4],HIGH); //Valve 7
         digitalWrite(v_a_pin[0],HIGH); //Valve 11
         
         if (emergencyStop) 
            state_main=hold;
         else if (goToNextState)
         { 
            goToNextState = 0;
            state_main=end_wash_cycle;
         } 
         else 
            state_main=water_flush;
         break;
      case end_wash_cycle:
         q=8;
         digitalWrite(v_f_pin[0],LOW);
         digitalWrite(v_f_pin[3],LOW);
         digitalWrite(v_f_pin[4],LOW);
         digitalWrite(v_a_pin[0],LOW);
    
         if(emergencyStop) 
            state_main=hold;
         else if(wash_count < 2) 
         {
            Timer3.detachInterrupt();
            seconds = 0; 
            Timer3.attachInterrupt(SetWashTime); 
            state_main=wash_start;
            wash_timer = 0;
            wash_mix_timer = 0;
            wash_settle_timer = 0; 
         }
         else if(wash_count >= 2) 
         {
            Timer3.detachInterrupt();
            seconds = 0; 
            Timer3.attachInterrupt(SetLineFlushTimer); 
            state_main=line_flush;
         }
         else 
            state_main=end_wash_cycle;
      
         wash_count=wash_count+1;
         break;
      case line_flush:
         q=9;
         digitalWrite(v_f_pin[0],HIGH); //Valve 4
         digitalWrite(v_f_pin[3],HIGH); //Valve 6
         digitalWrite(v_f_pin[4],HIGH); //Valve 7
         digitalWrite(v_a_pin[0],HIGH); //Valve 11
         if (emergencyStop) 
            state_main=hold;
         else if (line_flush_timer || goToNextState) 
         {
            goToNextState = 0; 
            Timer3.detachInterrupt();
            seconds = 0; 
            Timer3.attachInterrupt(SetBiodieselXferTimer); 
            state_main=f_biodiesel_xfer;
         }
         else 
            state_main=line_flush;
         break;
      case f_biodiesel_xfer:
         q=10;
         digitalWrite(v_f_pin[4],LOW); //Valve 7
         digitalWrite(v_f_pin[5],HIGH); //Valve 8
         digitalWrite(v_f_pin[0],HIGH); //Valve 4
         digitalWrite(v_f_pin[3],HIGH); //Valve 6
         digitalWrite(v_a_pin[0],HIGH); //Valve 11
         
         if (emergencyStop) 
            state_main=hold;
         else if (f_biodiesel_timer || goToNextState) 
            state_main=end_p;
         else 
            state_main=f_biodiesel_xfer;
         break;
      case end_p:
         q=11;
         digitalWrite(v_f_pin[5],LOW);
         digitalWrite(v_f_pin[0],LOW);
         digitalWrite(v_f_pin[3],LOW);
         digitalWrite(v_a_pin[0],LOW);
         
         if (emergencyStop) 
            state_main=hold;
         else 
            state_main=end_p;
         break;
      case hold:
         digitalWrite(v_f_pin[0],LOW);
         digitalWrite(v_f_pin[1],LOW);
         digitalWrite(v_f_pin[2],LOW);
         digitalWrite(v_f_pin[3],LOW);
         digitalWrite(v_f_pin[4],LOW);
         digitalWrite(v_f_pin[5],LOW);
         digitalWrite(v_a_pin[0],LOW);
         digitalWrite(v_a_pin[1],LOW);
         digitalWrite(v_a_pin[2],LOW);
         digitalWrite(motor_pin, LOW); 
         state_water=stop_wh;
         
         if (q==0 && resume) 
            state_main=add_methoxide;
         else if (q==1 && resume) 
            state_main=rxn_settle;
         else if (q==2 && resume) 
            state_main=g_out_v_open;
         else if (q==3 && resume) 
            state_main=g_out_v_close;
         else if (q==4 && resume) 
            state_main=wash_start;
         else if (q==5 && resume) 
            state_main=wash_mix;
         else if (q==6 && resume) 
            state_main=wash_settle;
         else if (q==7 && resume) 
            state_main=water_flush;
         else if (q==8 && resume) 
            state_main=end_wash_cycle;
         else if (q==9 && resume) 
            state_main=line_flush;
         else if (q==10 && resume) 
            state_main=f_biodiesel_xfer;
         else if (q==11 && resume) 
            state_main=end_p;
         else if (q==12 && resume)
            state_main=acid_heating;
         else if (q==13 && resume)
            state_main=acid_mixing_methanol;
         else if (q==14 && resume)
            state_main=acid_mixing_h2so4;
         else if (q==15 && resume)
            state_main=methoxide_heating;
         else 
            state_main=hold;
         break;
   }

   switch(state_water)
   {
      case heater_on:
         wh_q=0;
         temp_met = 0;
         digitalWrite(heater_pin,HIGH);
         digitalWrite(pump_pin, HIGH);
         sensors.requestTemperatures();
         tempF = sensors.getTempF(outakeTherm);
         if (tempF >= tc_max) 
            state_water=heater_off;
         else 
            state_water=heater_on;
         break;
      case heater_off:
         temp_met = 1; 
         wh_q=1; 
         digitalWrite(heater_pin,LOW);
         digitalWrite(pump_pin, HIGH); 
         sensors.requestTemperatures();
         tempF = sensors.getTempF(outakeTherm);
      
         if (tempF <= tc_min) 
            state_water=heater_on; 
         else 
            state_water=heater_off;
         break;
      case stop_wh:
         digitalWrite(heater_pin,LOW);
         digitalWrite(pump_pin, LOW);
         if (wh_q==0 && resume==1) 
            state_water=heater_on;
         else if (wh_q==1 && resume==1) 
            state_water=heater_off;
         else 
            state_water=stop_wh;
         break;
   }
   
   delay(100); //This delay slows down the loop a bit. Not good to loop really fast. 
   
   //Handle GUI/Arduino communication
   while (Serial.available())
   { 
      char inByte = Serial.read();
      switch (inByte) {
         case '0': //get current state command
            Serial.write('0');
            delay(10); //quick delay between serial calls    
            Serial.println(state_main);
            Serial.println(state_water);
            Serial.println(seconds);
            Serial.println(tempF);  
            break;
         case '1': //next state command
            /*q += 1;
            if (q > MAX_Q)
               q = 0;
            state_main = q;*/
            goToNextState = 1;   
            break;        
         case '2': //previous state command
            q -= 1;
            if (q < 0)
               q = MAX_Q;
            state_main = q;
            break;
         case '3': //Emergency stop command
            if (emergencyStop)
            {
               resume=1;
               emergencyStop=0;
            }
            else
            {
               resume = 0;
               emergencyStop = 1;
            } 
            break;
         case '4':
            state_water = heater_on;
            break;
         case '5':
            state_water = heater_off; 
            break;
         case '6':
            Serial.write('6');
            delay(10);
            Serial.println(tempF);
            break;             
       } 
    }      
}

