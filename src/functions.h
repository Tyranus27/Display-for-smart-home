#define CLOUDS "Clouds"
#define RAIN "Rain"
#define SUNNY "Sunny"
#define CLEAR "Clear"


void countdownTimer();
void readButtons();
void noWifiMessage();
void offlineClock();
void codeForTask1(void *parameter);
void codeForTask2(void *parameter);
void codeForTask3(void *parameter);
void url();
void ntpClock();
void printMessage(String message, int positionHoriz);
void printLargeMessage(String message);
void printCountdownTimer();
void printLocalTime();
void changeActivity(int changeAct);
void wifiIconAnimation();
void weatherInformation();
long toMillis(int seconds, int minutes, int hours );
struct tm * time_pointer;
boolean urlOpenWeather();
boolean newsBBC();
boolean wifiSetup();
String SendHTML(uint8_t timeZone);

long seconds;
int flag2 = 0;
String openWeatherMapApiKey = "17b5b9e1abebc7729b0fe9d23884d6c5";
String city = "Aberystwyth";
String countryCode = "UK";
String countodownMsg;
String feed = "world";
String customMsg = "set custom message";

struct tm timeinfo;
time_t rawtime;
time_t startTime;
time_t now;
long elapsedTime;
long setTime = 20;
long previousTime = 0;


const char * jsonBuffer = (char*)malloc(10000);
const char * xmlPayload = (char*)malloc(14000);


const char* outlook;
const char* cloud = "Clouds";

char * outlookCmp = (char*)malloc(1500);
char * location = (char*)malloc(10000);
int temp ;
int main_pressure;
int main_humidity;

float wind_speed;

const char* name;
char date[80];
char day[10];
char fullDate[100];
char hoursMinutesFormat[15];



// Byte arrays for representing either whole icon 
// or part for purpouse of animation

// 'wifi_icon_sigdot', 8x8px
static unsigned char PROGMEM wifi_icon_sigdot[]  = {
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x0,
};
// 'wifi_icon_sigfull', 8x8px
static unsigned char PROGMEM wifi_icon_sigfull[]  = {
  0x78, 0x4, 0x2, 0x1, 0x1, 0x1, 0x1, 0x0, 
};
// 'wifi_icon_sig2', 8x8px
static unsigned char PROGMEM wifi_icon_sig2[]  = {
  0x0, 0x0, 0x70, 0x8, 0x4, 0x4, 0x4, 0x0,
};
// 'wifi_icon_sig1', 8x8px
static unsigned char PROGMEM wifi_icon_sig1[]  = {
0x0, 0x0, 0x0, 0x0, 0x60, 0x10, 0x10, 0x0, 
};

static unsigned char PROGMEM wifi_icon_full[] = { 
  0x78, 0x4, 0x72, 0x9, 0x65, 0x15, 0x55, 0x0, 
};


static unsigned char PROGMEM sun_icon[] = {
0x99, 0x7e, 0x66, 0xc3, 0xc3, 0x66, 0x7e, 0x99, 
};

static unsigned char PROGMEM celc_icon[] = { 
0x40, 0xa0, 0x4c, 0x10, 0x10, 0x10, 0xc, 0x0,
};


static unsigned char PROGMEM rain_icon_1 [] = { 
  0x18, 0x3c, 0x66, 0xc3, 0xff, 0x0, 0x0, 0x0, 
};
static unsigned char PROGMEM rain_icon_2 [] = { 
 0x18, 0x3c, 0x66, 0xc3, 0xff, 0x49, 0x0, 0x0,
};
static unsigned char PROGMEM rain_icon_3 [] = { 
0x18, 0x3c, 0x66, 0xc3, 0xff, 0x0, 0x92, 0x0, 
};
static unsigned char PROGMEM rain_icon_4 [] = { 
 0x18, 0x3c, 0x66, 0xc3, 0xff, 0x0, 0x0, 0x24, 
};


static unsigned char PROGMEM sun_icon_2 [] = { 
0x18, 0x7e, 0x66, 0xc3, 0xc3, 0x66, 0x7e, 0x18,
};
static unsigned char PROGMEM sun_icon_1 [] = { 
 0x0, 0x3c, 0x66, 0x42, 0x42, 0x66, 0x3c, 0x0, 
};


static unsigned char PROGMEM clouds_icon_2 [] = { 
0x7, 0xc, 0x18, 0xdf, 0xe0, 0x30, 0x18, 0xf8,
};
static unsigned char PROGMEM clouds_icon_1 [] = { 
0x1, 0x3, 0x6, 0x87, 0xc0, 0x60, 0x30, 0xf0,
};