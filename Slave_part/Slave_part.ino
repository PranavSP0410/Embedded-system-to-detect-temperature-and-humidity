#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd1(0x27,20,4);
LiquidCrystal_I2C lcd2(0x3F,20,4);

#define clk 2
#define data 13
#define button_press 7
#define alarm 9
#define inter 3
#define inter_call 12
  

int val1;
int val2;
int temp_val=EEPROM.read(79);  // To remember the previous set value
int humid_val=EEPROM.read(80); // To remember the previous set value
int clk_state;                 // From the Encoder, for the current reading
int prev_state;                // To keep the previous value of the encoder position
int choice=0;                  // For choosing the set points (1.Temp 2.Humid 3.None and Other functions)
int last_button_press=0;       // To know the last time the button was pressed
int temp_or_humid;             // For the alert message
int state_button=0;            // For alarm (LED)
unsigned int rx_buf[9];        // Receiver buffer to store incoming data in group of 9
char rx_ptr=0;                 // As index for the buffer
unsigned char rx_byte;         // As input for the received data

void setup() {
  Serial.begin(38400);
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();
  pinMode(clk,INPUT);
  pinMode(data,INPUT);
  pinMode(button_press,INPUT_PULLUP);
  pinMode(alarm,OUTPUT);
  pinMode(inter,INPUT);
  pinMode(inter_call,OUTPUT);
  digitalWrite(alarm,LOW);
  digitalWrite(inter_call,HIGH);
  attachInterrupt(digitalPinToInterrupt(inter), inform, LOW);
  prev_state=digitalRead(clk);
  lcd1.setCursor(1,0);
  lcd1.print("Observed Reading :");
  lcd1.setCursor(0,2);
  lcd1.print("Temperature : ");
  lcd1.setCursor(0,3);
  lcd1.print("Humidity    : ");
}
//----------------------------------------------------------------------------------------------------------------------------------
void prints(){
  lcd1.setCursor(14,3);
  lcd1.print("    ");
  lcd1.setCursor(14,3);
  lcd1.print(val1);
  lcd1.print(" %RH");
  lcd1.setCursor(14,2);
  lcd1.print("    ");
  lcd1.setCursor(14,2);
  lcd1.print(val2);
  lcd1.write(223);
  lcd1.print("C");
  lcd2.setCursor(5,0);
  lcd2.print("Set-Points");
  lcd2.setCursor(1,1);
  lcd2.print("Temp:");
  lcd2.setCursor(8,1);
  lcd2.print(temp_val);
  lcd2.write(223); // for degree symbol [WRITE... NOT PRINT]
  lcd2.print("C");
  lcd2.setCursor(1,2);
  lcd2.print("Humid:");
  lcd2.setCursor(8,2);
  lcd2.print(humid_val);
  lcd2.print(" %RH");
}
//----------------------------------------------------------------------------------------------------------------------------------
int set_point(int count,int high, int low,int ch){
  int con; // For the position of the value to be changed
  if(ch==1){
    con=1;
  }
  else if(ch==2){
    con=2;
  }
  while(digitalRead(button_press)!=0){
    clk_state=digitalRead(clk);
    if(prev_state!=clk_state && clk_state==1){
      if(clk_state!=digitalRead(data)){
        count--;
        if(count<low){
          count=low;
        }
        lcd2.setCursor(8,con);
        lcd2.print(count);
      }
      else{
        count++;
        if(count>high){
          count=high;
        }
        lcd2.setCursor(8,con);
        lcd2.print(count);
      }
    }
    prev_state=clk_state;
  }
  return count;
}
//----------------------------------------------------------------------------------------------------------------------------------
void inform(){
  state_button=1;
  digitalWrite(inter_call,HIGH);
}
//----------------------------------------------------------------------------------------------------------------------------------
void perform(){
  if(state_button){
    String val=" Breach!!";
    lcd2.setCursor(0,3);
    if(temp_or_humid==1){
      val="Temperature"+val;
    }
    else if(temp_or_humid==0){
      val="Humidity"+val;
    }
    lcd2.print(val);
    digitalWrite(alarm,HIGH);
  }
}
//----------------------------------------------------------------------------------------------------------------------------------
void receiver(){
  while(Serial.available()){
    rx_byte=Serial.read();
    rx_buf[rx_ptr]=(rx_byte);
    rx_ptr++;     
    if(rx_ptr>=9){
      rx_ptr=0;
      break;
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------------------
void processor(){
  for(int i=0;i<5;i++){
    if(rx_buf[i]==0xFF and rx_buf[i+1]==0xFA and rx_buf[i+4]==0xAA){
      val1= rx_buf[i+2];
      val2= rx_buf[i+3];
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------------------
void loop() {
  receiver();
  processor();
  prints();
  
  if(choice==1){
    if(digitalRead(button_press)==0){
      if(millis()- last_button_press >400){
        lcd2.setCursor(0,1);
        lcd2.print(" ");
        lcd2.setCursor(0,2);
        lcd2.print(">");
        choice++;
      }
      last_button_press=millis();
    }
    temp_val=set_point(temp_val,60,20,choice);
    EEPROM.update(79,temp_val);
  }
  else if(choice==2){
    if(digitalRead(button_press)==0){
      if(millis()- last_button_press >400){
        lcd2.setCursor(0,1);
        lcd2.print(" ");
        lcd2.setCursor(0,2);
        lcd2.print(" ");
        choice=0;
      }
      last_button_press=millis();
    }
    humid_val=set_point(humid_val,70,40,choice);
    EEPROM.update(80,humid_val);
  }
  else if (choice==0){
    if(digitalRead(button_press)==0 and (val1<humid_val and val2<temp_val)){
      if(millis()- last_button_press >400){
        lcd2.setCursor(0,1);
        lcd2.print(">");
        lcd2.setCursor(0,2);
        lcd2.print(" ");
        choice++;
      }
      last_button_press=millis();
    }
    if(val2>=temp_val){
      temp_or_humid=1;
      digitalWrite(inter_call,LOW);
    }
    else if(val1>=humid_val){
      temp_or_humid=0;
      digitalWrite(inter_call,LOW);
    }
    if(val1<humid_val and val2<temp_val){
      state_button=0;
      lcd2.setCursor(0,3);
      lcd2.print("                    ");
      digitalWrite(alarm,LOW);
    }
    perform();
  }
}
