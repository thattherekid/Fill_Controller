
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_SPI.h> //Hardware-specific library
#include <LCDWIKI_TOUCH.h> //touch screen library
#include "switch_font.c"
#include <EasyTransfer.h>

int relay2 = 8; //pump relay
int relay1 = 9; //valve relay
int fullSensor = 10;
int senderGauge1 = A1;
int pumpStop = 7;

unsigned long startGallons; // beginning volume
unsigned long currentGallons; // for current volume minus starting volume 
unsigned long newGallons; // current volume
unsigned long G; // Button gallon selection variable
unsigned long currentMillis; // for display refresh
unsigned long startIdle; // for idle timer
unsigned long displayMillis; // for idle timer refresh
int displayTime;
int idleTime;
int idleSeconds;
int idleMinutes;
int idleHours;
int resLevel1;
int fillCount;
int newROLevel;

EasyTransfer ET;

struct RECEIVE_DATA_STRUCTURE{
 unsigned long totalMLData;
 unsigned long flowData;
 int roData;
};

RECEIVE_DATA_STRUCTURE mydata;

//paramters define
#define MODEL ILI9488_18
#define CS   A0    
#define CD   A4
#define RST  A3
#define LED  A5   //if you don't need to control the LED pin,you should set it to -1 and set it to 3.3V

//touch screen paramters define
#define TCS   5
#define TCLK  6
#define TDOUT 3
#define TDIN  4
#define TIRQ  2

//the definiens of hardware spi mode as follow:
//if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_SPI my_lcd(MODEL,CS,CD,RST,LED); //model,cs,dc,reset,led

//the definiens of touch mode as follow:
LCDWIKI_TOUCH my_touch(TCS,TCLK,TDOUT,TDIN,TIRQ); //tcs,tclk,tdout,tdin,tirq

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define BUTTON_X 40
#define BUTTON_Y 100
#define BUTTON_W 60
#define BUTTON_H 30
#define BUTTON_SPACING_X 20
#define BUTTON_SPACING_Y 20
#define BUTTON_TEXTSIZE 2

// We have a status line for like, is FONA working
#define STATUS_X 10
#define STATUS_Y 65

boolean switch_flag_1 = true,switch_flag_2 = true,switch_flag_3 = true,switch_flag_4 = true,switch_flag_5 = true,switch_flag_6 = true;  
int16_t menu_flag = 1,old_menu_flag;     

uint16_t px,py;

void show_string(uint8_t *str,int16_t x,int16_t y,uint8_t csize,uint16_t fc, uint16_t bc,boolean mode)
{
  my_lcd.Set_Text_Mode(mode);
  my_lcd.Set_Text_Size(csize);
  my_lcd.Set_Text_colour(fc);
  my_lcd.Set_Text_Back_colour(bc);
  my_lcd.Print_String(str,x,y);
}

void show_picture(const uint8_t *color_buf,int16_t buf_size,int16_t x1,int16_t y1,int16_t x2,int16_t y2)
{
  my_lcd.Set_Addr_Window(x1, y1, x2, y2); 
  my_lcd.Push_Any_Color(color_buf, buf_size, 1, 1);
}

boolean is_pressed(int16_t x1,int16_t y1,int16_t x2,int16_t y2,int16_t px,int16_t py)
{
  if((px > x1 && px < x2) && (py > y1 && py < y2))
  {
    return true;  
  } 
  else
  {
    return false;  
  }
}

void getGallons(){
  for(int i=0; i<5; i++){
    if(ET.receiveData()){
      newGallons = mydata.totalMLData;
    }
  }
}

void getROLevel(){
  for(int i=0; i<5; i++){
    if(ET.receiveData()){
      newROLevel = mydata.roData;
    }
  }
}
 
void setup() {

  Serial.begin(9600);
  ET.begin(details(mydata), &Serial);

  pinMode(fullSensor, INPUT_PULLUP);
  pinMode(relay1, OUTPUT); // Pump relay
  pinMode(relay2, OUTPUT); // Valve relay
  pinMode(senderGauge1, INPUT);
  pinMode(pumpStop, INPUT_PULLUP);
  displayTime = 1;
  idleTime = 1;
  fillCount = 1;
  currentMillis = millis();
  startIdle = millis();
  displayMillis = millis();
  currentGallons = 60000; 
  my_lcd.Init_LCD();
  my_lcd.Set_Rotation(1);
  my_touch.TP_Set_Rotation(1);
  my_touch.TP_Init(my_lcd.Get_Rotation(),my_lcd.Get_Display_Width(),my_lcd.Get_Display_Height()); 
  my_lcd.Fill_Screen(WHITE); 

  my_lcd.Set_Draw_color(192, 192, 192);

  my_lcd.Draw_Fast_HLine(0, 37, my_lcd.Get_Display_Width());
 
  show_string("RESERVOIR LEVEL: ",5,40,2,BLACK, BLACK,1);
  resLevel1 = map(analogRead(senderGauge1),551,903,100,25); 
    char buf8[3];
    sprintf(buf8, "%02d%%", resLevel1);
    show_string(buf8, 200, 40, 2, BLUE, BLACK, 1);
  
  my_lcd.Draw_Fast_HLine(0, 57, my_lcd.Get_Display_Width());
 
  show_string("RO TANK LEVEL: ",5,60,2,BLACK, BLACK,1);
  getROLevel();
  char buf9[3];
  sprintf(buf9, "%02d%%", newROLevel);
  show_string(buf9, 200, 60, 2, BLUE, BLACK, 1);

  my_lcd.Draw_Fast_HLine(0, 77, my_lcd.Get_Display_Width());
 
  show_string("TIME IDLE: ",5,80,2,BLACK, BLACK,1);

  my_lcd.Draw_Fast_HLine(0, 97, my_lcd.Get_Display_Width());
 
  show_string("VOLUME PUMPED: ",5,100,2,BLACK, BLACK,1);
  show_string("idle", 200, 100, 2, BLUE, BLACK, 1);

  my_lcd.Draw_Fast_HLine(0, 117, my_lcd.Get_Display_Width());
 
  my_lcd.Draw_Rectangle(10, 210, 110, 310);
  show_string("1", 50,230,3,RED,BLACK,1);
  show_string("GALLON", 20,270,2,RED,BLACK,1);

  my_lcd.Draw_Rectangle(130, 210, 230, 310);
  show_string("2", 170,230,3,RED,BLACK,1);
  show_string("GALLON", 140,270,2,RED,BLACK,1);

  my_lcd.Draw_Rectangle(250, 210, 350, 310);
  show_string("5", 290,230,3,RED,BLACK,1);
  show_string("GALLON", 260,270,2,RED,BLACK,1);

  my_lcd.Draw_Rectangle(370, 210, 470, 310);
  show_string("RES", 390,230,3,RED,BLACK,1);
  show_string("FILL", 385,270,3,RED,BLACK,1);
}

void loop() {

  px = 0;
  py = 0;
  my_touch.TP_Scan(0);
  if (my_touch.TP_Get_State()&TP_PRES_DOWN) 
  {
    px = my_touch.x;
    py = my_touch.y;
  } 

  if(is_pressed(10,210,110,310,px,py)) // Button 1
  {
    G = 3785; // mL = 1 gallon

    if(switch_flag_2)
    {
      switch_flag_2 = false;

      my_lcd.Set_Draw_color(CYAN);
      my_lcd.Fill_Rectangle(11, 211, 109, 309);
      show_string("1", 50,220,3,BLUE,BLACK,1);
      show_string("GALLON", 22,250,2,BLUE,BLACK,1);
      show_string("FILL", 35,270,2,BLUE,BLACK,1);
      show_string("RUNNING", 18,290,2,BLUE,BLACK,1);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 120, 479, 140);
      show_string("1 GALLON FILL RUNNING", 10, 120, 3, GREEN, BLACK, 1);

      digitalWrite(relay1, HIGH); // Open fill valve
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("VALVE OPENING", 50, 160, 3, GREEN, BLACK, 1);
      delay(10000);

      getGallons();
      startGallons = newGallons;
      digitalWrite(relay2, HIGH); // Pump on 
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("PUMP RUNNING", 50, 160, 3, GREEN, BLACK, 1);

      do{  
        getGallons();
        currentGallons = (newGallons - startGallons);
        
        if((millis() - currentMillis)>5000){ // update display for flow every 5 seconds
          my_lcd.Set_Draw_color(WHITE);
          //my_lcd.Fill_Rectangle(1, 140, 479, 190);
          my_lcd.Fill_Rectangle(200, 98,  479, 116);
          unsigned long foo = currentGallons / 37.85;
          int dispGalInt = foo / 100;
          int dispGalDec = foo % 100;
          char buf2[13];
          sprintf(buf2, "%01d.%02d GALLONS", dispGalInt, dispGalDec); // can't display mL because a larger fill is to big of a number (buf2, "%05d mL - %01d.%02d GAL", currentGallons, dispGalInt, dispGalDec)
          show_string(buf2, 200, 100, 2, BLUE, BLACK, 1); // this part is causing it to freak out and reset

          resLevel1 = map(analogRead(senderGauge1),551,903,100,25); 
          my_lcd.Set_Draw_color(WHITE);
          my_lcd.Fill_Rectangle(200, 38,  479, 56);
            char buf8[4];
            sprintf(buf8, "%02d%%", resLevel1);
            show_string(buf8, 200, 40, 2, BLUE, BLACK, 1);
          my_lcd.Set_Draw_color(WHITE);  
          my_lcd.Fill_Rectangle(200, 78,  479, 96);
          show_string("Active", 200, 80, 2, BLUE, BLACK, 1);
          currentMillis = millis();
        } 
      } while(currentGallons < G && digitalRead(fullSensor) == HIGH && digitalRead(pumpStop) == HIGH);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(200, 98,  479, 116);
          unsigned long foo = currentGallons / 37.85;
          int dispGalInt = foo / 100;
          int dispGalDec = foo % 100;
          char buf2[13];
          sprintf(buf2, "%01d.%02d GALLONS", dispGalInt, dispGalDec);
          show_string(buf2, 200, 100, 2, BLUE, BLACK, 1);

      digitalWrite(relay2, LOW);
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("PUMP STOP", 50, 160, 3, GREEN, BLACK, 1);
      delay(5000);

      digitalWrite(relay1, LOW);
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("VALVE CLOSING", 50, 160, 3, GREEN, BLACK, 1);
      delay(5000);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 120, 479, 190);
      //show_string(currentGallons,5,120,2,BLUE, BLACK,1); // Display currentGallons, need to update code for displaying variable
      
      if(digitalRead(fullSensor) == LOW){  
        my_lcd.Set_Draw_color(WHITE);
        my_lcd.Fill_Rectangle(1, 140, 479, 190);
        show_string("MAX FILL REACHED", 20, 160, 3, RED, BLACK, 1); 
      }
      
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(11, 211, 109, 309);
      show_string("1", 50,230,3,RED,BLACK,1);
      show_string("GALLON", 20,270,2,RED,BLACK,1);

      switch_flag_2 = true;
      startIdle = millis();       
    }
    delay(100);
  }

  if(is_pressed(130, 210, 230, 310,px,py)) // Button 2
  {
    G = 7571; // mL = 2 gallons

    if(switch_flag_2)
    {
      switch_flag_2 = false;

      my_lcd.Set_Draw_color(CYAN);
      my_lcd.Fill_Rectangle(131, 211, 229, 309);
      show_string("2", 167,220,3,BLUE,BLACK,1);
      show_string("GALLON", 140,250,2,BLUE,BLACK,1); //140
      show_string("FILL", 153,270,2,BLUE,BLACK,1);
      show_string("RUNNING", 136,290,2,BLUE,BLACK,1);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 120, 479, 140);
      show_string("2 GALLON FILL RUNNING", 10, 120, 3, GREEN, BLACK, 1);
      
      digitalWrite(relay1, HIGH); // Open fill valve
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("VALVE OPENING", 50, 160, 3, GREEN, BLACK, 1);
      delay(10000);

      getGallons();
      startGallons = newGallons;
      digitalWrite(relay2, HIGH); // Pump on 
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("PUMP RUNNING", 50, 160, 3, GREEN, BLACK, 1);

      do{  
        getGallons();
        currentGallons = (newGallons - startGallons);
        
        if((millis() - currentMillis)>5000){ // update display for flow every 5 seconds
          my_lcd.Set_Draw_color(WHITE);
          //my_lcd.Fill_Rectangle(1, 140, 479, 190);
          my_lcd.Fill_Rectangle(200, 98,  479, 116);
          unsigned long foo = currentGallons / 37.85;
          int dispGalInt = foo / 100;
          int dispGalDec = foo % 100;
          char buf2[13];
          sprintf(buf2, "%01d.%02d GALLONS", dispGalInt, dispGalDec);
          show_string(buf2, 200, 100, 2, BLUE, BLACK, 1);

          resLevel1 = map(analogRead(senderGauge1),551,903,100,25); 
          my_lcd.Set_Draw_color(WHITE);
          my_lcd.Fill_Rectangle(200, 38,  479, 56);
            char buf8[4];
            sprintf(buf8, "%02d%%", resLevel1);
            show_string(buf8, 200, 40, 2, BLUE, BLACK, 1);
          my_lcd.Set_Draw_color(WHITE);  
          my_lcd.Fill_Rectangle(200, 78,  479, 96);
          show_string("Active", 200, 80, 2, BLUE, BLACK, 1);
          currentMillis = millis();
        } 
      } while(currentGallons < G && digitalRead(fullSensor) == HIGH && digitalRead(pumpStop) == HIGH);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(200, 98,  479, 116);
          unsigned long foo = currentGallons / 37.85;
          int dispGalInt = foo / 100;
          int dispGalDec = foo % 100;
          char buf2[13];
          sprintf(buf2, "%01d.%02d GALLONS", dispGalInt, dispGalDec);
          show_string(buf2, 200, 100, 2, BLUE, BLACK, 1);

      digitalWrite(relay2, LOW);
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("PUMP STOP", 50, 160, 3, GREEN, BLACK, 1);
      delay(5000);

      digitalWrite(relay1, LOW);
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("VALVE CLOSING", 50, 160, 3, GREEN, BLACK, 1);
      delay(5000);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 120, 479, 190);
      //show_string(currentGallons,5,120,2,BLUE, BLACK,1); // Display currentGallons, need to update code for displaying variable
      
      if(digitalRead(fullSensor) == LOW){  
        my_lcd.Set_Draw_color(WHITE);
        my_lcd.Fill_Rectangle(1, 140, 479, 190);
        show_string("MAX FILL REACHED", 20, 160, 3, RED, BLACK, 1); 
      }

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(131, 211, 229, 309);
      show_string("2", 170,230,3,RED,BLACK,1);
      show_string("GALLON", 140,270,2,RED,BLACK,1);
    
      switch_flag_2 = true;
      startIdle = millis();        
    }
    delay(100);
  }

  if(is_pressed(250, 210, 350, 310,px,py)) // Button 3
  {
    G = 18927; // mL = 5 gallons

    if(switch_flag_2)
    {
      switch_flag_2 = false;

      my_lcd.Set_Draw_color(CYAN);
      my_lcd.Fill_Rectangle(251, 211, 349, 309);
      show_string("5", 287,220,3,BLUE,BLACK,1);
      show_string("GALLON", 260,250,2,BLUE,BLACK,1);
      show_string("FILL", 273,270,2,BLUE,BLACK,1);
      show_string("RUNNING", 256,290,2,BLUE,BLACK,1);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 120, 479, 140);
      show_string("5 GALLON FILL RUNNING", 10, 120, 3, GREEN, BLACK, 1);
      
      digitalWrite(relay1, HIGH); // Open fill valve
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("VALVE OPENING", 50, 160, 3, GREEN, BLACK, 1);
      delay(10000);

      getGallons();
      startGallons = newGallons;
      digitalWrite(relay2, HIGH); // Pump on 
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("PUMP RUNNING", 50, 160, 3, GREEN, BLACK, 1);

      do{  
        getGallons();
        currentGallons = (newGallons - startGallons);
        
        if((millis() - currentMillis)>5000){ // update display for flow every 5 seconds
          my_lcd.Set_Draw_color(WHITE);
          //my_lcd.Fill_Rectangle(1, 140, 479, 190);
          my_lcd.Fill_Rectangle(200, 98,  479, 116);
          unsigned long foo = currentGallons / 37.85;
          int dispGalInt = foo / 100;
          int dispGalDec = foo % 100;
          char buf2[13];
          sprintf(buf2, "%01d.%02d GALLONS", dispGalInt, dispGalDec);
          show_string(buf2, 200, 100, 2, BLUE, BLACK, 1);

          resLevel1 = map(analogRead(senderGauge1),551,903,100,25); 
          my_lcd.Set_Draw_color(WHITE);
          my_lcd.Fill_Rectangle(200, 38,  479, 56);
            char buf8[4];
            sprintf(buf8, "%02d%%", resLevel1);
            show_string(buf8, 200, 40, 2, BLUE, BLACK, 1);
          my_lcd.Set_Draw_color(WHITE);  
          my_lcd.Fill_Rectangle(200, 78,  479, 96);
          show_string("Active", 200, 80, 2, BLUE, BLACK, 1);
          currentMillis = millis();
        } 
      } while(currentGallons < G && digitalRead(fullSensor) == HIGH && digitalRead(pumpStop) == HIGH);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(200, 98,  479, 116);
          unsigned long foo = currentGallons / 37.85;
          int dispGalInt = foo / 100;
          int dispGalDec = foo % 100;
          char buf2[13];
          sprintf(buf2, "%01d.%02d GALLONS", dispGalInt, dispGalDec);
          show_string(buf2, 200, 100, 2, BLUE, BLACK, 1);

      digitalWrite(relay2, LOW);
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("PUMP STOP", 50, 160, 3, GREEN, BLACK, 1);
      delay(5000);

      digitalWrite(relay1, LOW);
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("VALVE CLOSING", 50, 160, 3, GREEN, BLACK, 1);
      delay(5000);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 120, 479, 190);
      //show_string(currentGallons,5,120,2,BLUE, BLACK,1); // Display currentGallons, need to update code for displaying variable
      
      if(digitalRead(fullSensor) == LOW){  
        my_lcd.Set_Draw_color(WHITE);
        my_lcd.Fill_Rectangle(1, 140, 479, 190);
        show_string("MAX FILL REACHED", 20, 160, 3, RED, BLACK, 1); 
      }

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(251, 211, 349, 309);
      show_string("5", 290,230,3,RED,BLACK,1);
      show_string("GALLON", 260,270,2,RED,BLACK,1);
    
      switch_flag_2 = true;
      startIdle = millis();          
    }
    delay(100);
  }

  if(is_pressed(370, 210, 470, 310,px,py))  // Button 4
  {
    G = 75708; // mL = 20 gallons
    
    if(switch_flag_2)
    {
      switch_flag_2 = false;

      my_lcd.Set_Draw_color(CYAN);
      my_lcd.Fill_Rectangle(371, 211, 469, 309);
      show_string("FILL", 385,220,3,BLUE,BLACK,1);
      show_string("TO", 400,250,3,BLUE,BLACK,1);
      show_string("FULL", 385,280,3,BLUE,BLACK,1);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 120, 479, 140);
      show_string("FILLING RESERVOIR", 10, 120, 3, GREEN, BLACK, 1);

      digitalWrite(relay1, HIGH); // Open fill valve
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("VALVE OPENING", 50, 160, 3, GREEN, BLACK, 1);
      delay(10000);

      getGallons();
      startGallons = newGallons;
      digitalWrite(relay2, HIGH); // Pump on 
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("PUMP RUNNING", 50, 160, 3, GREEN, BLACK, 1);

      do{  
        getGallons();
        currentGallons = (newGallons - startGallons);
        
        if((millis() - currentMillis)>5000){ // update display for flow every 5 seconds
          my_lcd.Set_Draw_color(WHITE);
          //my_lcd.Fill_Rectangle(1, 140, 479, 190);
          my_lcd.Fill_Rectangle(200, 98,  479, 116);
          unsigned long foo = currentGallons / 37.85;
          int dispGalInt = foo / 100;
          int dispGalDec = foo % 100;
          char buf2[13];
          sprintf(buf2, "%01d.%02d GALLONS", dispGalInt, dispGalDec);
          show_string(buf2, 200, 100, 2, BLUE, BLACK, 1);

          resLevel1 = map(analogRead(senderGauge1),551,903,100,25); 
          my_lcd.Set_Draw_color(WHITE);
          my_lcd.Fill_Rectangle(200, 38,  479, 56);
            char buf8[4];
            sprintf(buf8, "%02d%%", resLevel1);
            show_string(buf8, 200, 40, 2, BLUE, BLACK, 1);
          my_lcd.Set_Draw_color(WHITE);  
          my_lcd.Fill_Rectangle(200, 78,  479, 96);
          show_string("Active", 200, 80, 2, BLUE, BLACK, 1);
          currentMillis = millis();
        } 
      } while(currentGallons < G && digitalRead(fullSensor) == HIGH && digitalRead(pumpStop) == HIGH);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(200, 98,  479, 116);
          unsigned long foo = currentGallons / 37.85;
          int dispGalInt = foo / 100;
          int dispGalDec = foo % 100;
          char buf2[13];
          sprintf(buf2, "%01d.%02d GALLONS", dispGalInt, dispGalDec);
          show_string(buf2, 200, 100, 2, BLUE, BLACK, 1);

      digitalWrite(relay2, LOW);
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("PUMP STOP", 50, 160, 3, GREEN, BLACK, 1);
      delay(5000);

      digitalWrite(relay1, LOW);
      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 140, 479, 190);
      show_string("VALVE CLOSING", 50, 160, 3, GREEN, BLACK, 1);
      delay(5000);

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(1, 120, 479, 190);
      //show_string(currentGallons,5,120,2,BLUE, BLACK,1); // Display currentGallons, need to update code for displaying variable
      
      if(digitalRead(fullSensor) == LOW){  
        my_lcd.Set_Draw_color(WHITE);
        my_lcd.Fill_Rectangle(1, 140, 479, 190);
        show_string("MAX FILL REACHED", 20, 160, 3, RED, BLACK, 1); 
      }

      my_lcd.Set_Draw_color(WHITE);
      my_lcd.Fill_Rectangle(371, 211, 469, 309);
      show_string("RES", 390,230,3,RED,BLACK,1);
      show_string("FILL", 385,270,3,RED,BLACK,1);
    
      switch_flag_2 = true;
      startIdle = millis();           
    }
    delay(100);
  }
  
  if((millis() - currentMillis) > 30000){  // update display every 30 seconds with tank volumes
    resLevel1 = map(analogRead(senderGauge1),551,903,100,25); 
    getROLevel();
    my_lcd.Set_Draw_color(WHITE);
    my_lcd.Fill_Rectangle(200, 38,  479, 56);
    my_lcd.Fill_Rectangle(200, 58,  479, 76);
    //show_string("resLevel1", 200, 40, 2, BLUE, BLACK, 1); // change text to int reservoirLevel
      char buf8[4];
      sprintf(buf8, "%02d%%", resLevel1);
      show_string(buf8, 200, 40, 2, BLUE, BLACK, 1);
      char buf9[4];
      sprintf(buf9, "%02d%%", newROLevel);
      show_string(buf9, 200, 60, 2, BLUE, BLACK, 1);
    currentMillis = millis();
  }
  
  unsigned long currentIdle = millis() - startIdle;
  unsigned long currentSeconds = currentIdle/1000;
  int runHours = currentSeconds / 3600;
  int secsRemaining = currentSeconds%3600;
  int runMinutes = secsRemaining / 60;
  int runSeconds = secsRemaining%60;

  if((millis() - displayMillis) > 1000){ // update time every second
    my_lcd.Set_Draw_color(WHITE);  
    my_lcd.Fill_Rectangle(200, 78,  479, 96);
    char buf1[8]; 
    sprintf(buf1, "%02d:%02d:%02d", runHours, runMinutes, runSeconds);
    show_string(buf1, 200, 80, 2, BLUE, BLACK, 1);
    displayMillis = millis();
  }
}














