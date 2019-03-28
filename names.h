#include <vector>
#include <string>
#include "TextLCD.h"
#include "SDFileSystem.h"
#include "FATFileSystem.h"
#include "keypad.h"
#include <Timer.h>
#include <Ticker.h>
#include "beep.h"
//cutter engine
DigitalOut cutter_eng_dir(PC_2);
DigitalOut cutter_eng_step(PC_3);

//tape Engine
DigitalOut tape_eng_dir(PC_12);
PwmOut tape_eng_step(D6);

DigitalOut cutterPush(PC_4);

DigitalOut PowerLED(PA_14);
DigitalOut WorkLED(PA_15);
DigitalOut NoMaterialLED(PC_5);  //светодиод, горящий когда нет ленты
DigitalOut CutterFaultLED(PC_6);
DigitalOut StandbyLED(PC_8);

AnalogIn cutterZeroPosition(A4);  //пин проверки датчика нулевого положения
AnalogIn TapeSignal(A5);  //пин приходящий с концевика ленты(датчик наличия ленты)
AnalogIn cutterInHome(A3);
DigitalOut SoundSignal(D8); //звуковой сигнал когда нет ленты


Serial pc(SERIAL_TX, SERIAL_RX);
I2C i2c_lcd(D14, D15); // SDA, SCL
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD20x4, TextLCD::HD44780); // I2C bus, PCF8574 Slaveaddress, LCD Type, Device Type
SDFileSystem sd(D11, D12, D13, D10, "sd");
Keypad keypadd(PA_10, PB_3, PB_5, PB_4, PB_13, PB_14, PB_15, PB_1);
Ticker keypad_timer;
Beep buzzer(D8);

unsigned char selectTemplateCounter = 0;
unsigned char createTemplateCounter = 0;
unsigned char settingTemplateCounter = 0;
unsigned char singleTemplateCounter = 0;

unsigned short int sectionsCalculate();
unsigned short int sectionsDone = 0; 
unsigned short int sectionsQty = 0;

unsigned short int kitsQty = 0;
unsigned short int kitsDone = 0;
char lcdArray[15] = "";
char str[250];
char buffer[250];
string templateName;  // имя шаблона в CreateTemplate, да и вообще буферная переменная хранящая имя текущего выбранного шаблона
char bufferName[15];
unsigned short int templateNameCounter = 0;


double templateSetting[8][4];   //templateSetting[numberOfsetting][0-MinValue,1-MaxValue,2-step,3-CurrentValue]
double templateToSave[8] = {0};
vector<double> firstAngles;
vector<double> secondAngles;
vector<double> tapesLength;
char sectionCounter =0;
char sectionShowCounter =0;
double indentFirst = 0;
double indentSecond = 0;
unsigned short int tapeSteps = 0;
unsigned short int cutterSteps = 0;
unsigned short int stepsPerRotateCutter = 3200;
unsigned int delay_time = 0;

int templateList[200] = {0};
int templateListCounter = 0;
unsigned short int counterOfTemplates = 0;
vector<string> filenames;
 FILE *S1;