#include <NtpClient.h>
#include <Time.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <tinyxml2.h>


#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

 #include <WiFi.h>  
 #include <WiFiUdp.h>        
 #include <DNSServer.h>


// This is an Arduino library for our 16x32 and 32x32 RGB LED matrix panels

// Pick one up at http://www.adafruit.com/products/420 & http://www.adafruit.com/products/607 !

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon for Adafruit Industries.
// BSD license, all text above must be included in any redistribution
#include <RGBmatrixPanel4.h>

#include<functions.h>

using namespace tinyxml2;


int dedit=0;
int flag = 0;


#define DESECONDS 1
#define DEMINUTES 2
#define DEHOURS 3
#define DEDOW 4
#define DEDAY 5
#define DEMONTH 6
#define DEYEAR 7

#define NTPCLOCKSYNCINTERVAL 86400000 // 24h clock update interval

#define BOTTOMTEXT 9
#define TOPTEXT 1



// Most of the signal pins are configurable, but the CLK pin has some
// special constraints.  On 8-bit AVR boards it must be on PORTB...
// Pin 8 works on the Arduino Uno & compatibles (e.g. Adafruit Metro),
// Pin 11 works on the Arduino Mega.  On 32-bit SAMD boards it must be
// on the same PORT as the RGB data pins (D2-D7)...
// Pin 8 works on the Adafruit Metro M0 or Arduino Zero,
// Pin A4 works on the Adafruit Metro M4 (if using the Adafruit RGB
// Matrix Shield, cut trace between CLK pads and run a wire to A4).

#define CLK 14   // USE THIS ON ESP32
#define OE  13
#define LAT 15
#define A   26
#define B   4
#define C   27

#define R1 5
#define G1 17
#define B1 18
#define R2 19
#define G2 16
#define B2 25

#define BLEFT 34
#define BRIGHT 21
#define BUP 35
#define BDOWN 33
#define BSELECT 32


#define MODEHOME 0
#define MODECLOCK 1
#define MODEWEATHER 2
#define MODENEWS 3
#define MODESTARTCOUNTDOWN 4
#define MODEIP 5

#define MODETEST 20
#define MODEERRORRESET 10
#define CUSTOMMESSAGE 11
#define CUSTOMLARGEMESSAGE 12
#define MAXMODE 5


#define SECONDINTERVAL 1000


// Last parameter = 'true' enables double-buffering, for flicker-free,
// buttery smooth animation.  Note that NOTHING WILL SHOW ON THE DISPLAY
// until the first call to swapBuffers().  This is normal.
RGBmatrixPanel4 matrix(A, B, C, CLK, LAT, OE, true, 2);
// Double-buffered mode consumes nearly all the RAM available on the
// Arduino Uno -- only a handful of free bytes remain.  Even the
// following string needs to go in PROGMEM:


TaskHandle_t Task1,Task2,Task3;
SemaphoreHandle_t batton;
StaticTask_t xTaskBuffer;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "uk.pool.ntp.org");
AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server,&dns);
char str1[] = "Hello World";
char *str = str1;
// part of RGBmatrixPanel4.h library 
int16_t    textX         = matrix.width(),
           textMin       = sizeof(str1) * -12,
           hue           = 0;



int dly=20;
int displaymode = -1;
int x=matrix.width()/2;
int y=8;
int timeOffset = 0;
String newsTitle;
int repeater = 0;
const long interval = 900000;
int lastdisplaymode = displaymode;
int swaper = 0;
String previousDay;
int dst = 0;
int c = matrix.ColorHSV(255, 50, 100, false);
long lastMillis;



unsigned long previousMillis = 0; 
unsigned long previousMillis2 = 0; 
unsigned long previousMillis3 = 0;
unsigned long previousMillis4 = 0;
unsigned long previousMillis5 = 0;
unsigned long previousMillis6 = 0;
long countdownTimeInMili = 0;
int counter = 0;
File file;
int mode = 0;
hw_timer_t * timer = NULL;

int animationSwitch = 0;
int animationRainSwitch = 0;
int animationSunSwitch = 0;
int animationCloudSwitch = 0;
int buffor = 0;
const char* PARAM_ACT_CHANGE = "output";
const char* PARAM_INPUT_2 = "state";
const char* PARAM_SECONDS = "seconds";
const char* PARAM_MINUTES = "minutes";
const char* PARAM_HOURS = "hours";
const char* PARAM_GMT = "gmt";
const char* PARAM_FEED ="feed";
const char * PARAM_MSG = "msg";
const char * PARAM_MSG_LARGE = "large";
const char * PARAM_WEATHER_CITY = "city";
const char * PARAM_WEATHER_CODE = "countrycode";
const char * PARAM_COUNTDOWNMSG = "countdownMsg";



void setup() {
    Serial.begin(9600);

    // Serial.println("Press Enter to Continue");
    // while( getchar() != '\n' );

    matrix.begin();
    matrix.setTextWrap(false); // Allow text to run off right edge


 
      // methods for initialization of tasks pinned to desired core
      // part of FreeRTOS
      xTaskCreatePinnedToCore(
      codeForTask1,
      "Task_1",
      10000,
      NULL,
      1,
      &Task1,
      0);

    
       xTaskCreatePinnedToCore(
      codeForTask2,
      "Task_2",
      10000,
      NULL,
      0,
      &Task2,
      0);

        xTaskCreatePinnedToCore(
      codeForTask3,
      "Task_3",
      10000,
      NULL,
      0,
      &Task3,
      0);
      
      
    vTaskSuspend(Task3);
   vTaskSuspend(Task2);
    if(wifiSetup()){
      vTaskSuspend(Task1);
      vTaskSuspend(Task2);
      
      matrix.fillScreen(0);
      timeClient.begin();
      timeClient.setUpdateInterval(6000);
      newsBBC();
      weatherInformation();

      displaymode = MODEHOME;
      

    
    //begining the spiff allows the device to read files from its memory
     if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
     }
     

    
    // all server.on methods are receiving different requests 
    // from the controlling web server
    // each request is precede with "/"
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
     request->send(SPIFFS, "/webServerControl.html", "text/html");
     if (flag2 == 0){
       matrix.fillScreen(0);
       displaymode = MODEHOME;
       flag2 = 1;
     }


     Serial.println("Clock started..yey");
   });

    server.on("/inputHandler.html", HTTP_GET, [](AsyncWebServerRequest *request){
     request->send(SPIFFS, "/inputHandler.html", "text/html");    
   });


  server.on("/stylesheet.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/stylesheet.css", "text/css");
  });

  server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/scripts.js", "text/javascript");
});

server.on("/countdowninput", HTTP_GET, [](AsyncWebServerRequest *request){
     String seconds, minutes, hours;
       if (request->hasParam(PARAM_SECONDS) && request->hasParam(PARAM_MINUTES)  && request->hasParam(PARAM_HOURS) ){
        seconds = request->getParam(PARAM_SECONDS)->value();
        minutes = request->getParam(PARAM_MINUTES)->value();
        hours = request->getParam(PARAM_HOURS)->value();

      if(request->hasParam(PARAM_COUNTDOWNMSG)){
        countodownMsg = request->getParam(PARAM_COUNTDOWNMSG)->value();
      }
        if(seconds.isEmpty() || minutes.isEmpty() || hours.isEmpty()){
          seconds = "0";
          minutes = "0";
          hours = "0";
          counter = 2;
        }
        else if (seconds.toInt() + minutes.toInt() + hours.toInt() != 0){
            buffor = 0;
          counter = 1;
          displaymode = MODESTARTCOUNTDOWN;

          
        } else{
          counter = 0;
          displaymode = MODESTARTCOUNTDOWN;
        }

       
       }
       else {
       seconds = "0";
       minutes = "0";
       hours = "0";
       counter = 2;
       
     }
     Serial.println(seconds);
     Serial.println(minutes);
     Serial.println(hours);
    countdownTimeInMili = toMillis(seconds.toInt(), minutes.toInt(), hours.toInt());
    Serial.println(countdownTimeInMili);
    
  request->send(200, "text/plain", "OK");
});



   server.on("/timezonechange", HTTP_GET, [] (AsyncWebServerRequest * request){
     String inputGmt;
     if (request->hasParam(PARAM_GMT)){
       inputGmt = request->getParam(PARAM_GMT)->value();
      
     } else {
       inputGmt = "No message";
     }

     Serial.println("Aktualny gmt: "+ inputGmt);

    timeOffset = inputGmt.toInt();
 
    request->send(200, "text/plain", "OK");
   });



  server.on("/changeactivity", HTTP_GET, [] (AsyncWebServerRequest * request){
   String inputAct;
    if (flag2 == 0){
       matrix.fillScreen(0);
       displaymode = MODEHOME;
       flag2 = 1;
     }
   if (request->hasParam(PARAM_ACT_CHANGE)){
       inputAct = request->getParam(PARAM_ACT_CHANGE)->value();     
   }
   else inputAct = "0";
    Serial.println(inputAct);

    changeActivity(inputAct.toInt());
    Serial.print(displaymode);
    request->send(200, "text/plain", "OK");
   });


  server.on("/changefeed", HTTP_GET, [] (AsyncWebServerRequest * request){
   if (request->hasParam(PARAM_FEED)){
       feed = request->getParam(PARAM_FEED)->value();  
     
   }
   else feed = "world";
   if(newsBBC()){
     displaymode = MODENEWS;
   }
    Serial.println(feed);
    request->send(200, "text/plain", "OK");
   });


  server.on("/custommessage", HTTP_GET, [] (AsyncWebServerRequest * request){
   if (request->hasParam(PARAM_MSG)&&request->hasParam(PARAM_MSG_LARGE)){
       customMsg = request->getParam(PARAM_MSG)->value();
       Serial.println(customMsg);  
       displaymode = CUSTOMLARGEMESSAGE;
      
   }
   else if(request->hasParam(PARAM_MSG)){
     customMsg = request->getParam(PARAM_MSG)->value();
      
       Serial.println(customMsg);  
       displaymode = CUSTOMMESSAGE;
    
   }
  
   else{ customMsg = "set custom message";

   }
    Serial.println(customMsg);
    request->send(200, "text/plain", "OK");
   });

server.on("/weatherlocation", HTTP_GET, [] (AsyncWebServerRequest * request){

   if (request->hasParam(PARAM_WEATHER_CITY) && request->hasParam(PARAM_WEATHER_CODE)){
       city = request->getParam(PARAM_WEATHER_CITY)->value();     
       countryCode = request->getParam(PARAM_WEATHER_CODE)->value(); 
       countryCode.toUpperCase();
       Serial.println(city);
       Serial.println(countryCode);
       weatherInformation();  
   }
   else{ city = "Aberystwyth";
countryCode = "UK";
   }
    request->send(200, "text/plain", "OK");
   });


    server.begin();
    
    
} else {
      matrix.fillScreen(0);
      vTaskSuspend(Task1);

    }
}

void loop() {
  

    unsigned long currentMillis = millis();
    ntpClock();
    lastdisplaymode = displaymode;
    if(WiFi.status() == WL_CONNECTED) {

      //logical expression for displying the Device IP activity
      // right after startup for either 1 minute, or by accesing the webserver
      if (currentMillis < 60000 && flag2 == 0) {
        displaymode = MODEIP;
      }
      else if ((currentMillis > 60000 && currentMillis < 61000 )&& flag2 == 0){
        matrix.fillScreen(0);
        displaymode = MODEHOME;
      }

    


      // News Reader and Weather information activity are getting 
      // updated every 15 minutes
      if(currentMillis - previousMillis >= interval){
        previousMillis = currentMillis;

      
      weatherInformation();
      Serial.println("News update");      
      if(newsBBC()){
        
        time(&startTime);
        while (elapsedTime < setTime)
        {
          displaymode = MODENEWS;
          now = time(NULL);
          elapsedTime = difftime(now,startTime);
        } 
        displaymode = MODEHOME;
        
        

      } 
      }
    

      if(counter == 1){
        //updating the timer every one second
            if(currentMillis - previousMillis2 >= SECONDINTERVAL){
              previousMillis2 = currentMillis;
              countdownTimer();
      }
    } 


    if(displaymode == MODEHOME){
      vTaskResume(Task2);
    }

   
    }
    else{
      displaymode = MODEHOME;
      vTaskResume(Task2);
    }



  //Switch statment is used as a running menu method 
  // each switch statment is different activity
  switch (displaymode){

    case MODECLOCK: 
     matrix.fillScreen(0);

       matrix.setTextColor(matrix.ColorHSV(255, 155, 255, false));
  matrix.setTextSize(1);
  matrix.setCursor(10, 1);

  matrix.print(timeClient.getFormattedTime());
      printMessage("Today is:"+(String)fullDate+" GMT"+timeOffset,BOTTOMTEXT);

    break;


    case MODESTARTCOUNTDOWN:
     matrix.fillScreen(0);
    if (counter == 1){
      printCountdownTimer();
      printMessage(countodownMsg, BOTTOMTEXT);
    }
    else if(counter == 2){
    printMessage("Time is up",BOTTOMTEXT);
    matrix.setTextColor(matrix.ColorHSV(255, 155, 255, false));
    matrix.setTextSize(1);
    matrix.setCursor(10, 1);
    matrix.print("00");
    matrix.print(":");
    matrix.print("00");
    matrix.print(":");
    matrix.print("00");
    }
    else if(counter == 0){
    printMessage("Set the countdown timer!",BOTTOMTEXT);
    matrix.setTextColor(matrix.ColorHSV(255, 155, 255, false));
    matrix.setTextSize(1);
    matrix.setCursor(10, 1);
    matrix.print("00");
    matrix.print(":");
    matrix.print("00");
    matrix.print(":");
    matrix.print("00");
    }
    break;

    case MODENEWS:
     {
    matrix.fillScreen(0);
    
    matrix.setCursor(1,1);
    matrix.setTextColor(matrix.ColorHSV(150, 155, 100, false));
    String upperFeed = feed;
    upperFeed.toUpperCase();
    if(upperFeed == "SCIENCE_AND_ENVIRONMENT"){
      upperFeed = "SCIENCE";
    }
    matrix.print(upperFeed);
    printMessage(newsTitle,BOTTOMTEXT);
    
     }break;
    

    case MODEHOME: {
      matrix.setTextSize(1);
     matrix.fillRect(0,0,matrix.width(),8,0);
    if(timeClient.getFormattedTime() == "00:00:00"){
     matrix.fillRect(12,9,52,8,0);
    };
      
      matrix.setCursor(0,0);
      matrix.setTextColor(matrix.ColorHSV(255, 50, 100, false));
      matrix.print(temp);

      if (temp >= 0 && temp < 10){
        matrix.drawBitmap(7,0,celc_icon, 8,8,c);
      }
      else{
          matrix.drawBitmap(12,0,celc_icon, 8,8,c);
      }



    matrix.setCursor(22,0);
    matrix.setTextColor(matrix.ColorHSV(150, 155, 100, false));

   matrix.print(hoursMinutesFormat);

    if(WiFi.status() != WL_CONNECTED){
    matrix.drawBitmap(55,0,wifi_icon_full, 8, 8, matrix.ColorHSV(0,100,100,false));
    }
    
    
    matrix.setCursor(12,9);
    matrix.setTextColor(matrix.ColorHSV(50, 155, 100, false));
    matrix.print(date);

    matrix.setCursor(46,9);
    matrix.setTextColor(matrix.ColorHSV(50, 155, 100, false));
    matrix.print(day);
    
     } break;

    case MODEIP:

    matrix.fillScreen(0);
    matrix.setCursor(1,1);
    matrix.setTextColor(matrix.ColorHSV(150, 155, 100, false));
    matrix.print("DEVICE IP:");
    printMessage(WiFi.localIP().toString(),BOTTOMTEXT);

    break;

    case MODEERRORRESET:
    matrix.fillScreen(0);
    printMessage("Something went wrong",TOPTEXT);
    matrix.setCursor(5,9);
    matrix.setTextColor(matrix.ColorHSV(150, 155, 100, false));
    matrix.print("..Reset..");
 
    break;


    case CUSTOMMESSAGE:
    matrix.fillScreen(0);
    printMessage(customMsg,TOPTEXT);

    break;

    case CUSTOMLARGEMESSAGE:
    matrix.fillScreen(0);
    printLargeMessage(customMsg);

    break;

    case MODEWEATHER:
      matrix.fillScreen(0);

      matrix.setCursor(0,0);
      matrix.setTextColor(matrix.ColorHSV(255, 50, 100, false));
      matrix.print(temp);

     if (temp < 10 && temp >= 0){
        matrix.drawBitmap(6,0,celc_icon, 8,8,c);
      }
      else if(temp < 0 && temp >= -9){
          matrix.drawBitmap(11,0,celc_icon, 8,8,c);
      }else if(temp < -9){
        matrix.drawBitmap(16,0,celc_icon, 8,8,c);
      } else  matrix.drawBitmap(11,0,celc_icon, 8,8,c);

      matrix.setCursor(23,0);
      matrix.setTextColor(matrix.ColorHSV(255, 50, 100, false));
      matrix.print(outlookCmp);
  
      printMessage("Location:"+ (String)location +", Humidity:"+main_humidity+"%, Pressure:"+main_pressure+"hPa, Wind Speed "+wind_speed+"m/s",BOTTOMTEXT);
      


    break;


  }

#if !defined(__AVR__)
  // On non-AVR boards, delay slightly so screen updates aren't too quick.

  delay(dly);
#endif

  // Update display
  if (displaymode == MODEHOME || displaymode == MODETEST){
    matrix.swapBuffers(true);

  }
  else{ 
    
    vTaskSuspend(Task2);
    matrix.swapBuffers(false);


  }



}

// function for displaying animated icon of the wifi searching status
void wifiIconAnimation(){
int16_t iconColor = matrix.ColorHSV(50,100,100,false);

  if(animationSwitch == 0 ){
    matrix.fillScreen(0);
  matrix.drawBitmap(0, 0,  wifi_icon_sigdot, 8, 8, iconColor );
matrix.drawBitmap(55, 9,  wifi_icon_sigdot, 8, 8, iconColor );
  animationSwitch++;
  delay(300);
  
 
  }
  else if (animationSwitch == 1) {
  matrix.drawBitmap(0, 0,  wifi_icon_sig1, 8, 8, iconColor ); 
  matrix.drawBitmap(55, 9,  wifi_icon_sig1, 8, 8, iconColor ); 
  animationSwitch++;
 delay(300);
 
    }
  else if (animationSwitch == 2){ 
  matrix.drawBitmap(0, 0,  wifi_icon_sig2, 8, 8, iconColor );  
  matrix.drawBitmap(55, 9,  wifi_icon_sig2, 8, 8, iconColor );
  animationSwitch++;

  delay(300);

      }
  else if(animationSwitch == 3){

  matrix.drawBitmap(0, 0,  wifi_icon_sigfull, 8, 8, iconColor );  
  matrix.drawBitmap(55, 9,  wifi_icon_sigfull, 8, 8, iconColor );
  animationSwitch = 0;
 delay(300);
        }

      
    
  


}

// Function for navigation between activites 
// operated through the webserver
void changeActivity(int changeAct){
  matrix.fillScreen(0);
  if (changeAct == 1)
  {
    matrix.fillScreen(0);
    if(displaymode<MAXMODE){
    displaymode++;
    
    }else {
      matrix.fillScreen(0);
      displaymode = 0;
    }
    
  }
  
  else if(changeAct == 0){
  matrix.fillScreen(0);
    if(displaymode > 0){
      displaymode--;
    }
      else {
        matrix.fillScreen(0);
        displaymode = MAXMODE;
      }
  }

  if(counter == 2 && displaymode == MODESTARTCOUNTDOWN){
    counter = 0;
  }
  lastdisplaymode = displaymode;
  
}

long toMillis(int seconds, int minutes, int hours ){

   long timeInMillis = (seconds * 1000) + (minutes * 60000) + (hours * 3600000);
    return timeInMillis;
}



// Task executed on second core. 
// Responsible for displaying animated icon of searching wifi screen
void codeForTask1(void *parameter)
{   


      for(;;){
      wifiIconAnimation();
      matrix.setTextColor(matrix.ColorHSV(0,100,100,false));
       
      matrix.setCursor(10,0);
      matrix.print("searching");
      matrix.setCursor(1,9);  
      matrix.print("for wifi!");
    

    delay(dly);
   matrix.swapBuffers(true);
     
     
  }


}

// Task executed on second core. 
// Responsible for displaying animated icons of the outlook
void codeForTask2(void *parameter)
{   


    
      for(;;){
        if (displaymode == MODEHOME){
          
         int16_t iconColor = matrix.ColorHSV(100,100,100,false);
      
     
          if (strcmp(outlookCmp,RAIN) == 0)
          {

            
          if(animationRainSwitch == 0 ){
    
          matrix.fillRect(0,8,8,8,0);      
          matrix.drawBitmap(0,8,rain_icon_1,8,8, iconColor);
          animationRainSwitch++;
              

           }
           else if (animationRainSwitch == 1) {
           matrix.drawBitmap(0,8,rain_icon_2,8,8, iconColor);
           animationRainSwitch++;
          
            
           }
           else if (animationRainSwitch == 2){
           matrix.drawBitmap(0,8,rain_icon_3,8,8, iconColor);
           animationRainSwitch++;

           }
           else if(animationRainSwitch == 3){
           matrix.drawBitmap(0,8,rain_icon_4,8,8, iconColor);
           animationRainSwitch = 0;
           }


          }

          else if (strcmp(outlookCmp,CLOUDS) == 0)
          {
            
              if (animationCloudSwitch == 0) {
                
                matrix.fillRect(0,8,8,8,0); 
                
            matrix.drawBitmap(0,8,clouds_icon_1,8,8,iconColor);
            animationCloudSwitch++ ;
            delay(200);
            }
            else if (animationCloudSwitch == 1){
              delay(100);
              matrix.fillRect(0,8,8,8,0); 
              
            matrix.drawBitmap(0,8,clouds_icon_2,8,8,iconColor);
            animationCloudSwitch=0;
            delay(200);
            }
            
          }       

          else if (strcmp(outlookCmp,CLEAR) == 0)
          {
            if (animationSunSwitch == 0) {
              matrix.fillRect(0,8,8,8,0); 
            matrix.drawBitmap(0,8,sun_icon_1,8,8,iconColor);
            animationSunSwitch++ ;
            }
            else if (animationSunSwitch == 1){
            matrix.drawBitmap(0,8,sun_icon_2,8,8,iconColor);
            animationSunSwitch++;
            }
            else if (animationSunSwitch == 2){
            matrix.drawBitmap(0,8,sun_icon,8,8,iconColor);
            animationSunSwitch = 0;
          }

          }
        }
          else {
            ;
          }
          
            delay(500);
            matrix.swapBuffers(true);

        }
        
       delay(100);


  }

// unused function for running the News Reader activity in more versatile manner.
// May be used in future versions
void codeForTask3(void *parameter){

for(;;){

  matrix.fillRect(0,0,matrix.width(),8,0);
  matrix.setTextColor(matrix.ColorHSV(150, 155, 100, false));
    String upperFeed = feed;
    upperFeed.toUpperCase();
    
   
  printMessage("Feed:"+ upperFeed,TOPTEXT);
  delay(20);
  matrix.swapBuffers(false);
}

}

  

boolean wifiSetup(){

  
  wifiManager.setConfigPortalTimeout(300);

    
   wifiManager.autoConnect("ESP", "123456789");
   delay(1000);
  
  if(WiFi.isConnected()){
    Serial.print("The IP is: ");
    Serial.println(WiFi.localIP());
    flag = 1;
    return true;

  } else{
    
    flag = 0;

    Serial.println("No wifi connection");
   return false;
  }

  

}

//function from the sample code 
//provided by supervisor Mr. Shipman
void printWithZero(int x) 
{
  if (x<10) 
  {
    matrix.print("0");
  }
  matrix.print(x);
  
}


// unused function, might be useful in the future version
void noWifiMessage(){
 String noWifiError = "... NO WIFI CONNECTION ...";
      matrix.setTextColor(matrix.ColorHSV(0, 255, 255, false));
      textMin = sizeof(noWifiError) * (-12);
      matrix.setCursor(textX,9);
      matrix.print(noWifiError);
    
     if((--textX) < textMin){
      
       textX = matrix.width();
     
       }
}


void ntpClock(){
  struct tm *time;
   if(WiFi.status() != WL_CONNECTED){

     timeClient.setUpdateInterval(NTPCLOCKSYNCINTERVAL);
   } 

  if (timeOffset == 0)
  {
    dst=1;
  } else dst = 0;
  
  
  timeClient.setTimeOffset((timeOffset + dst) * 3600 );

  timeClient.update();

time_t rawtime = timeClient.getEpochTime();
 time = localtime(&rawtime);
 strftime(date, sizeof(date), "%d.%m",time);
 strftime(day, sizeof(day), "%a",time);
 strftime(fullDate, sizeof(fullDate),"%d.%m.%Y %a",time);
 strftime(hoursMinutesFormat, sizeof(hoursMinutesFormat),"%H:%M",time);

}


void countdownTimer(){
  

  long timeLeft = countdownTimeInMili - (buffor+=1000);
  seconds = timeLeft/1000;

  if(seconds == 0){

    counter = 2;

  } 
}

void printCountdownTimer(){
int h,m,s;
  matrix.setTextColor(matrix.ColorHSV(255, 155, 255, false));
  matrix.setTextSize(1);
  matrix.setCursor(10, 1);
    h = (seconds/3600);
    m = (seconds - (3600*h))/60;
    s = (seconds-(3600*h)-(m*60));

  
  if (seconds > 0){
    printWithZero(h);
    matrix.print(":");
    printWithZero(m);
    matrix.print(":");
    printWithZero(s);
  }

}





void printMessage(String message, int positionHoriz){
      auto temp = message.c_str();
      matrix.setTextSize(1);
      matrix.setTextColor(matrix.ColorHSV(200, 200, 100, false));
      textMin = sizeof(message) * - /*12*/ (strlen(temp));
      matrix.setCursor(textX,positionHoriz);
      matrix.print(message);

    
     if((--textX) < textMin){
       textX = matrix.width();
       }
}

void printLargeMessage(String message){
      auto temp = message.c_str();
      matrix.setTextSize(2);
      matrix.setTextColor(matrix.ColorHSV(200, 200, 100, false));
      textMin = sizeof(message) * - (strlen(temp));
      matrix.setCursor(textX,1);
      matrix.print(message);

    
     if((--textX) < textMin){
  
       textX = matrix.width();
       }

  
   
}

void url(){
  
  
  String serverName= "http://feeds.bbci.co.uk/news/"+feed+"/rss.xml#";
  HTTPClient http;
  String payload;
  http.begin(serverName.c_str());

  int httpResponseCode = http.GET();

  if (httpResponseCode>0) {
  Serial.print("BBC - HTTP Response code: ");
  Serial.println(httpResponseCode);
  payload = http.getString();
  xmlPayload = payload.c_str();
  http.end();
  
  
}
else {
  Serial.print("BBC - Error code: ");
  Serial.println(httpResponseCode);
  http.end();
 
}



}

boolean newsBBC(){
 XMLDocument xmlDocument;

  url();

if(xmlDocument.Parse(xmlPayload) != XML_SUCCESS){
  Serial.print("Cannot parse the file");
  return false;
} else Serial.println("Parsing success");
String temp;
XMLText *textNode = xmlDocument.FirstChildElement( "rss" )->FirstChildElement( "channel" )->FirstChildElement( "item" )->FirstChildElement( "title" )->FirstChild()->ToText();
String title = textNode->Value();

if (newsTitle != title){
  temp = title;
  newsTitle = temp;
  delay(1000);
  xmlDocument.Clear();
  return true;
} else{ 
  Serial.println("No new message");
  xmlDocument.Clear();
  delay(1000);
  return false;

}
}


boolean urlOpenWeather(){
HTTPClient http;
String payload;


String serverName = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
http.begin(serverName.c_str());
  int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
  Serial.print("Weather - HTTP Response code: ");
  Serial.println(httpResponseCode);
  payload = http.getString();
  Serial.println("payload done");
  jsonBuffer = payload.c_str();
  Serial.println("jsonbuffer done");
  http.end();
  if(payload.isEmpty()){
    return false;
  }else return true;
    
    }

  else if(httpResponseCode == 404){
    Serial.print("Page not found");
    http.end();
    return false;
  }

  
  

else {
  Serial.print("Weather - Error code: ");
  Serial.println(httpResponseCode);
  http.end();
  return false;
 
}


}


void weatherInformation(){
if(urlOpenWeather() == false){
  displaymode = MODEWEATHER;
  return;
}

StaticJsonDocument<1000> filter;
filter["weather"][0]["main"] = true;

JsonObject filter_main = filter.createNestedObject("main");
filter_main["temp"] = true;
filter_main["pressure"] = true;
filter_main["humidity"] = true;
filter["wind"]["speed"] = true;
filter["name"] = true;

StaticJsonDocument<2000> doc;
DeserializationError error = deserializeJson(doc, jsonBuffer, DeserializationOption::Filter(filter));

if (error) {
  Serial.print("deserializeJson() failed: ");
  Serial.println(error.c_str());
  return;
}

outlook = doc["weather"][0]["main"]; 

JsonObject main = doc["main"];
float main_temp = main["temp"]; 
temp = main_temp - 273.15;
main_pressure = main["pressure"]; 
main_humidity = main["humidity"]; 

wind_speed = doc["wind"]["speed"]; 

name = doc["name"]; 

strcpy(outlookCmp,outlook);
strcpy(location,name);

Serial.println(outlook);
Serial.println(temp);
Serial.println(main_pressure);
Serial.println(main_humidity);
Serial.println(wind_speed);
Serial.println(name);

}