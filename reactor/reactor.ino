
#include <TimerThree.h>
#include <OneWire.h>
#include <DallasTemperature.h>


//all signals are active high unless noted otherwise
// inputs
#define light_sensor_pin 3
#define thermocouple0_pin 0
#define thermocouple1_pin 1
#define start_reset_pin 23
#define em_stop_pin 24
#define resume_pin 25

//Motor controller fault indicator 1
#define motor_ff1_pin 26

//Motor controller fault indicator 2
#define motor_ff2_pin 27

// outputs
#define motor_pin 42
#define motor_dir_pin 10

//Motor controller active low fault flag reset
#define motor_reset_pin 37
#define heater_pin 12
#define pump_pin 51

//active low output enable for light sensor
#define light_sensor_oe_pin 38
#define light_sensor_led_pin 53
#define rxn_led_pin 39

//Glycerin transfer led
#define g_xfer_led_pin 40

//Biodiesel transfer led
#define bd_xfer_led_pin 41
#define line_flush_led_pin 42
#define heater_led_pin 11
#define em_stop_led_pin 52

//defining sensor thresholds
#define light_sensor_thresh
#define thermo_thresh_l
#define thermo_thresh_h

//Defining main state machine names and values
#define start 0
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

//Defining water heater state machine names and values
#define heater_on 0
#define heater_off 1
#define stop_wh 2

//Undefined constants from report (are these correct?) Its also probably not reported in degrees
#define tc_max 140
#define tc_min 139

//New constants
#define MAX_Q 12

//int outputs
int v_f_pin [6]={
  28, 29, 30, 31, 32, 33}; //fluid valve pins
int v_a_pin [3]={
  34, 35, 36,}; //air valves
int wash_ssg_pin [8]={
  43, 44, 45, 46, 47, 48, 49, 50}; // Wash display counter
int rxn_speed=125; //0 to 255 where duty cycle = rxn_speed/255 x 100%
int wash_speed=50;
int q=0;//State indicator variable
int wash_count=0; //wash counter to check for wash loop completion
int off; //blank wash_ssg
int wh_q; //water heater state indicator variable
//int inputs
int resume=0;
int light_sensor=0;
//timers
int rxn_timer=0;
int rxn_settle_timer=0;
int g_out_timer=0;
int wash_timer=0;
int wash_mix_timer=0;
int wash_settle_timer=0;
int line_flush_timer=0;
int f_biodiesel_timer=0;
int state_main=start;
int state_water=heater_on;
//seven segment display definitions
int wash_display=0;

//variables that weren't defined
int tc0 = 0;
int tc1 = 0;

int seconds = 0;

float tempF = 0;

#define rxn_time 7200 //7200 seconds in two hours 
#define rxn_settle_time 21600 //21600 seconds in six hours
#define g_out_time 10 //acutally ten seconds
#define wash_time 400 //water is added for 6 minutes and 40 seconds
#define mix_time 900 // mix for 15 minutes
#define wash_settle_time 3600 //let the washed mixture settle for 1 hour
#define line_flush_time 5 //flush the line before draining biodiesel from reactor tank
#define biodiesel_xfer_time 90 //drain the biodiesel for 90 seconds

#define ONE_WIRE_BUS g_xfer_led_pin //pin that temp sensor is plugged into
OneWire oneWire(40);
DeviceAddress outakeTherm = {0x28, 0xDF, 0x70, 0xFB, 0x05, 0x00, 0x00, 0x43};
DallasTemperature sensors(&oneWire); 

//would be nice to consolidate these functions in some way...
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

void setup() {
  pinMode(light_sensor_pin,INPUT);
  pinMode(thermocouple0_pin, INPUT);
  pinMode(thermocouple1_pin, INPUT);
  pinMode(start_reset_pin, INPUT);
  pinMode(em_stop_pin, INPUT);
  pinMode(resume_pin, INPUT);
  pinMode(motor_ff1_pin, INPUT);
  pinMode(motor_ff2_pin, INPUT);
  pinMode(v_f_pin[0], OUTPUT);
  pinMode(v_f_pin[1], OUTPUT);
  pinMode(v_f_pin[2], OUTPUT);
  pinMode(v_f_pin[3], OUTPUT);
  pinMode(v_f_pin[4], OUTPUT);
  pinMode(v_f_pin[5], OUTPUT);
  pinMode(v_a_pin[0], OUTPUT);
  pinMode(v_a_pin[1], OUTPUT);
  pinMode(v_a_pin[2], OUTPUT);
  pinMode(motor_pin, OUTPUT);
  pinMode(motor_reset_pin, OUTPUT);
  pinMode(heater_pin, OUTPUT);
  pinMode(light_sensor_oe_pin, OUTPUT);
  pinMode(rxn_led_pin, OUTPUT);
  pinMode(bd_xfer_led_pin, OUTPUT);
  pinMode(line_flush_led_pin, OUTPUT);
  pinMode(g_xfer_led_pin, OUTPUT);
  pinMode(wash_ssg_pin[0], OUTPUT);
  pinMode(wash_ssg_pin[1], OUTPUT);
  pinMode(wash_ssg_pin[2], OUTPUT);
  pinMode(wash_ssg_pin[3], OUTPUT);
  pinMode(wash_ssg_pin[4], OUTPUT);
  pinMode(wash_ssg_pin[5], OUTPUT);
  pinMode(wash_ssg_pin[6], OUTPUT);
  pinMode(wash_ssg_pin[7], OUTPUT);
  pinMode(pump_pin, OUTPUT); 
  pinMode(heater_led_pin, INPUT);
  digitalWrite(v_f_pin[0],LOW);
  digitalWrite(v_f_pin[1],LOW);
  digitalWrite(v_f_pin[2],LOW);
  digitalWrite(v_f_pin[3],LOW);
  digitalWrite(v_f_pin[4],LOW);
  digitalWrite(v_f_pin[5],LOW);
  digitalWrite(v_a_pin[0],LOW);
  digitalWrite(v_a_pin[1],LOW);
  digitalWrite(v_a_pin[2],LOW);
  digitalWrite(motor_reset_pin,LOW);
  digitalWrite(heater_pin,LOW);
  digitalWrite(light_sensor_oe_pin,HIGH);
  digitalWrite(rxn_led_pin,LOW);
  digitalWrite(bd_xfer_led_pin,LOW);
  digitalWrite(line_flush_led_pin,LOW);
  digitalWrite(wash_ssg_pin[0],LOW);
  digitalWrite(wash_ssg_pin[1],LOW);
  digitalWrite(wash_ssg_pin[2],LOW);
  digitalWrite(wash_ssg_pin[3],LOW);
  digitalWrite(wash_ssg_pin[4],LOW);
  digitalWrite(wash_ssg_pin[5],LOW);
  digitalWrite(wash_ssg_pin[6],LOW);
  digitalWrite(wash_ssg_pin[7],LOW);
  Timer3.initialize(1000000); //timer with 1 second period
  Timer3.attachInterrupt(SetRxnTime); 
  Serial.begin(9600);
  sensors.begin();
  sensors.setResolution(outakeTherm, 10); 
}

void loop(){
 
  switch(state_main){
  case start:
    q=0;
    digitalWrite(rxn_led_pin,HIGH);
    digitalWrite(motor_pin,HIGH);
    digitalWrite(pump_pin, HIGH); 
    if (digitalRead(em_stop_pin)==1) 
       state_main=hold;
    else if (rxn_timer==1) 
    {
      Timer3.detachInterrupt(); 
      seconds = 0; //reset seconds for the next timer
      Timer3.attachInterrupt(SetRxnSettleTime); //set the appropriate handler
      state_main=rxn_settle;
    } 
    else 
      state_main=start;
    break;
  case rxn_settle:
    q=1;
    digitalWrite(pump_pin, LOW); 
    digitalWrite(motor_pin,LOW);
    digitalWrite(rxn_led_pin,HIGH);
    if (digitalRead(em_stop_pin)==1) 
      state_main=hold;
    else if (rxn_settle_timer==1) 
    {
      Timer3.detachInterrupt();
      seconds = 0;
      state_main=g_out_v_open;  
    } 
    else 
      state_main=rxn_settle;
    break;
  case g_out_v_open:
    q=2;
    digitalWrite(rxn_led_pin,LOW);
    digitalWrite(g_xfer_led_pin,HIGH);
    digitalWrite(v_f_pin[0],HIGH);
    digitalWrite(v_f_pin[1],HIGH);
    digitalWrite(v_a_pin[0],HIGH);
    digitalWrite(light_sensor_oe_pin,LOW); //active low enable
    
    //light_sensor = 1; //for now, just skip this step
    
    //should read from the light sensor here
    if (digitalRead(em_stop_pin)==1) 
       state_main=hold;
    else if (light_sensor==1) 
    {
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
    digitalWrite(g_xfer_led_pin,HIGH);
    digitalWrite(v_f_pin[0],LOW);
    digitalWrite(v_f_pin[1],LOW);
    digitalWrite(v_a_pin[0],LOW);
    if (digitalRead(em_stop_pin)==1) 
       state_main=hold;
    else if (g_out_timer==1) 
    {
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
    digitalWrite(g_xfer_led_pin,LOW);
    wash_display=wash_count;
    digitalWrite(v_a_pin[1],HIGH);
    digitalWrite(v_a_pin[2],HIGH);
    digitalWrite(v_f_pin[2],HIGH);
    if (digitalRead(em_stop_pin)==1) 
      state_main=hold;
    else if (wash_timer==1) 
    {
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
    wash_display=wash_count;
    digitalWrite(v_a_pin[1],LOW);
    digitalWrite(v_a_pin[2],LOW);
    digitalWrite(v_f_pin[2],LOW);
    digitalWrite(motor_pin,HIGH); //would be nice to very this with PWM! 
    if (digitalRead(em_stop_pin)==1) 
       state_main=hold;
    else if (wash_mix_timer==1) 
    {
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
    wash_display=wash_count;
    analogWrite(motor_pin,0);
    if (digitalRead(em_stop_pin)==1) 
       state_main=hold;
    else if (wash_settle_timer==1) 
      state_main=water_flush;
    else 
      state_main=wash_settle;
    break;
  case water_flush:
    q=7;
    wash_display=wash_count;
    digitalWrite(v_f_pin[0],HIGH);
    digitalWrite(v_f_pin[3],HIGH);
    digitalWrite(v_f_pin[4],HIGH);
    digitalWrite(v_a_pin[0],HIGH);
    //incorporate light sensor logic here
    
    //for now, just set this to one for debugging
    light_sensor = 1; 
    
    if (digitalRead(em_stop_pin)==1) 
       state_main=hold;
    else if (light_sensor==1) 
      state_main=end_wash_cycle;
    else 
      state_main=water_flush;
    break;
  case end_wash_cycle:
    q=8;
    wash_display=wash_count;
    digitalWrite(v_f_pin[0],LOW);
    digitalWrite(v_f_pin[3],LOW);
    digitalWrite(v_f_pin[4],LOW);
    digitalWrite(v_a_pin[0],LOW);
    
    //check this logic
    if(digitalRead(em_stop_pin)==1) 
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
      
    wash_speed=wash_speed+20;
    wash_count=wash_count+1;
    break;
  case line_flush:
    q=9;
    wash_display=off;
    digitalWrite(line_flush_led_pin,HIGH);
    digitalWrite(v_f_pin[0],HIGH);
    digitalWrite(v_f_pin[3],HIGH);
    digitalWrite(v_f_pin[4],HIGH);
    digitalWrite(v_a_pin[0],HIGH);
    if (digitalRead(em_stop_pin)==1) 
      state_main=hold;
    else if (line_flush_timer==1) 
    {
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
    digitalWrite(line_flush_led_pin,LOW);
    digitalWrite(bd_xfer_led_pin,HIGH);
    digitalWrite(v_f_pin[4],LOW);
    digitalWrite(v_f_pin[5],HIGH);
    digitalWrite(v_f_pin[0],HIGH);
    digitalWrite(v_f_pin[3],HIGH);
    digitalWrite(v_a_pin[0],HIGH);
    if (digitalRead(em_stop_pin)==1) 
       state_main=hold;
    else if (f_biodiesel_timer==1) 
       state_main=end_p;
    else 
      state_main=f_biodiesel_xfer;
    break;
  case end_p:
    q=11;
    digitalWrite(bd_xfer_led_pin,LOW);
    //digitalWrite(end_led_pin,HIGH); this wasn't declared in the code
    digitalWrite(v_f_pin[5],LOW);
    digitalWrite(v_f_pin[0],LOW);
    digitalWrite(v_f_pin[3],LOW);
    digitalWrite(v_a_pin[0],LOW);
    if (digitalRead(em_stop_pin)==1) 
      state_main=hold;
    else 
      state_main=end_p;
    break;
  case hold:
    digitalWrite(em_stop_led_pin,HIGH);
    digitalWrite(v_f_pin[0],LOW);
    digitalWrite(v_f_pin[1],LOW);
    digitalWrite(v_f_pin[2],LOW);
    digitalWrite(v_f_pin[3],LOW);
    digitalWrite(v_f_pin[4],LOW);
    digitalWrite(v_f_pin[5],LOW);
    digitalWrite(v_a_pin[0],LOW);
    digitalWrite(v_a_pin[1],LOW);
    digitalWrite(v_a_pin[2],LOW);
    digitalWrite(light_sensor_oe_pin,HIGH);
    digitalWrite(rxn_led_pin,LOW);
    digitalWrite(bd_xfer_led_pin,LOW);
    digitalWrite(line_flush_led_pin,LOW);
    resume=digitalRead(resume_pin);
    if (q==0 && resume==HIGH) 
      state_main=start;
    if (q==1 && resume==HIGH) 
      state_main=rxn_settle;
    if (q==2 && resume==HIGH) 
      state_main=g_out_v_open;
    if (q==3 && resume==HIGH) 
      state_main=g_out_v_close;
    if (q==4 && resume==HIGH) 
      state_main=wash_start;
    if (q==5 && resume==HIGH) 
      state_main=wash_mix;
    if (q==6 && resume==HIGH) 
      state_main=wash_settle;
    if (q==7 && resume==HIGH) 
      state_main=water_flush;
    if (q==8 && resume==HIGH) 
      state_main=end_wash_cycle;
    if (q==9 && resume==HIGH) 
      state_main=line_flush;
    if (q==10 && resume==HIGH) 
      state_main=f_biodiesel_xfer;
    if (q==11 && resume==HIGH) 
      state_main=end_p;
    else 
      state_main=hold;
    break;
  }

  switch(state_water){
    case heater_on:
      wh_q=0;
      digitalWrite(heater_pin,HIGH);
      sensors.requestTemperatures();
      tempF = sensors.getTempF(outakeTherm);
      /*if (digitalRead(em_stop_pin)==1) 
         state_water=stop_wh;*/
       if (tempF >= tc_max) 
        state_water=heater_off;
      else 
        state_water=heater_on;
      break;
    case heater_off:
      wh_q=1; 
      digitalWrite(heater_pin,LOW);
      sensors.requestTemperatures();
      tempF = sensors.getTempF(outakeTherm);
      
      /*if (digitalRead(em_stop_pin)==1) 
         state_water=stop_wh;*/
      if (tempF <= tc_min) 
         state_water=heater_on; 
      else 
        state_water=heater_off;
      break;
    case stop_wh:
      digitalWrite(heater_pin,LOW);
      if (wh_q==0 && resume==1) 
        state_water=heater_on;
      if (wh_q==1 && resume==1) 
        state_water=heater_off;
      else 
        state_water=stop_wh;
      break;
  }
  
   //handle serial requests
   delay(100);
   
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
            q += 1;
            if (q > MAX_Q)
               q = 0;
            state_main = q;  
            break;        
         case '2': //previous state command
            q -= 1;
            if (q < 0)
               q = MAX_Q;
            state_main = q;
            break;
         case '3': //Emergency stop command
            state_main = hold;    
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
            Serial.println(analogRead(0));
            break;
             
    } 
  }      
}

