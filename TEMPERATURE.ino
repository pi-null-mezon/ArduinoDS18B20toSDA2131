//---------------------------------------------------------------------------------
#include <OneWire.h>
#define ONE_WIRE_PIN 3
#define MEASURE_DELAY 8000
#define MEASURE_RESOLUTION 0.0625

OneWire ds(ONE_WIRE_PIN);
#define MAX_SENSORS 16
byte v_addr[MAX_SENSORS][8];
float v_meas[MAX_SENSORS];
byte m_size = 0;
//---------------------------------------------------------------------------------
#define E_PIN   12
#define D_PIN   10
#define CLK_PIN 11
#define DOT_PIN 8

byte f_msb[] = {0xEF, 0x83, 0x77, 0xD7, 0x9B, 0xDD, 0xFD, 0x87, 0xFF, 0xDF, 0xEF};
byte s_lsb[] = {0xF7, 0x91, 0xBE, 0xBD, 0xD9, 0xED, 0xEF, 0xB1, 0xFF, 0xFD};
byte t_lsb[] = {0x77, 0x11, 0x3E, 0x3D, 0x59, 0x6D, 0x6F, 0x31, 0x7F, 0x7D};
byte f_lsb = 0x00;
byte s_msb = 0x00;
byte t_msb = 0x01;
byte minus_msb = 0x11;
//---------------------------------------------------------------------------------
int code, firstSymbol, secondSymbol, thirdSymbol;
float temp;
//---------------------------------------------------------------------------------

void setup()
{
  initSensors();
  initInd();
}

void loop()
{  
  for(byte i = 0; i < m_size; i++) {     
    startConversion(v_addr[i]);        
    temp = v_meas[(i+1) % m_size];     
    if(temp < 0.0) {
      code = (int)(-temp * 10);    
      if(code < 100) { // temperature higher then -10.0 but less then 0.0
        secondSymbol = code/10;
        thirdSymbol = code - (code/10)*10;  
        digitalWrite(DOT_PIN, LOW);  
      }
      else{
        secondSymbol = code/100;
        thirdSymbol = code/10 - (code/100)*10;
        if((code - (code/10)*10) > 4)
        thirdSymbol++;
        digitalWrite(DOT_PIN, HIGH);
      }      
      for(int j = 0; j < MEASURE_DELAY; j++) {       
        updateSymbols(f_lsb, minus_msb);       
        updateSymbols(s_lsb[secondSymbol], s_msb);           
        updateSymbols(t_lsb[thirdSymbol], t_msb);
      }               
    } else {
      code = (int)(temp * 10);
      firstSymbol = code/100;
      secondSymbol = code/10 - (code/100)*10;
      thirdSymbol = code - (code/10)*10;
      digitalWrite(DOT_PIN, LOW);
      for(int j = 0; j < MEASURE_DELAY; j++) {       
        updateSymbols(f_lsb, f_msb[firstSymbol]);       
        updateSymbols(s_lsb[secondSymbol], s_msb);           
        updateSymbols(t_lsb[thirdSymbol], t_msb);          
      }   
    }
    updateSymbols(0x00, 0x00); // no digits are displayed
    v_meas[i] = readSensor(v_addr[i]);
  } 
}

//---------------------------------------------------------------------------------
void updateSymbols(byte lsb, byte msb)
{
  digitalWrite(E_PIN, HIGH);
  for(char i = 7; i >= 0; i--) {
    digitalWrite(D_PIN, ((msb >> i) & 0x01));
    tickCLK();
  }
  for(char i = 7; i >= 0; i--) {   
    digitalWrite(D_PIN, ((lsb >> i) & 0x01));
    tickCLK();
  } 
  digitalWrite(E_PIN, LOW);
}

void tickCLK()
{
  digitalWrite(CLK_PIN, HIGH);
  digitalWrite(CLK_PIN, LOW);
}

void initInd()
{  
  pinMode(E_PIN, OUTPUT);
    digitalWrite(E_PIN, LOW);
  pinMode(CLK_PIN, OUTPUT);
    digitalWrite(CLK_PIN, LOW);
  pinMode(D_PIN, OUTPUT);
    digitalWrite(D_PIN, LOW);
  pinMode(DOT_PIN, OUTPUT);
    digitalWrite(DOT_PIN, HIGH); 
}
//---------------------------------------------------------------------------------

void initSensors()
{
  while(ds.search(v_addr[m_size++])) {  }
  m_size--;
  if(m_size > MAX_SENSORS)
    m_size = MAX_SENSORS;
}

void startConversion(byte *addres)
{
  ds.reset();
  ds.select(addres);
  ds.write(0x44, 1); // start conversion, with parasite power on at the end  
}

float readSensor(byte *addres)
{
  ds.reset();
  ds.select(addres);    
  ds.write(0xBE); // start read scratchpad
  byte lsb = ds.read();  
  byte msb = ds.read();
  byte sign = msb & 0xF0;
  if(sign == 0xF0)
    return -((int)((byte)~lsb) | ((int)((byte)~msb) << 8)) * MEASURE_RESOLUTION;
  else
    return ((int)lsb | ((int)msb << 8)) * MEASURE_RESOLUTION;  
}
//---------------------------------------------------------------------------------

