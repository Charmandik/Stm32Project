#include "mbed.h"
#include "names.h"

#define PowerON 0
#define ReadyToWork 1
#define SelectTemplate 2
#define CreateTemplate 3
#define SettingTemplate 4
#define SingleTape 5
#define QuantitySelect 6
#define Work 7
#define CutterRightDir 0
#define CutterLeftDir 1
#define TapeRightDir 0
#define TapeLeftDir 1

#define CutterEngineDelay 0.1
#define TapeEngineDelay 0.1

#define tmpltName 0
#define cuttingTime 1
#define speed 2
#define accTime 3
#define tapeWidth 4
#define firstAngle 5
#define secondAngle 6
#define tapeLength 7

bool numpad_1 = 0;
bool numpad_2 = 0;
bool numpad_3 = 0;
bool numpad_4 = 0;
bool numpad_5 = 0;
bool numpad_6 = 0;
bool numpad_7 = 0;
bool numpad_8 = 0;
bool numpad_9 = 0;
bool numpad_0 = 0;
bool startBtn = 0;
bool settingBtn = 0;
bool qtyBtn = 0;
bool shiftBtn = 0;
bool escBtn = 0;
bool stopBtn = 0;

bool changing = false;
int mode = 2;
void engineZeroPoint();

//Функция опускания ножа
void solenoidPush()
{
    wait(0.5); //пауза на случай, если нож не дошел до нужного угла
    if (cutterInHome.read() < 0.1) {  //считываем значения с концевого датчика
        cutterPush = 1;   
        wait_ms(templateSetting[cuttingTime][3] * 1000);   //для каждой ленты своё время прорезания, поэтому берем эти значения непосредственно из шаблона
        cutterPush = 0;
    } else {  //если же нож не в нулевом положении, то ждем пока он там появится
        wait(0.1); 
        solenoidPush();  
    }
}

//Функция поворота ножа
void turnCutter(double angle)  
{
    //В зависимости от приходящего значения, меняем направление движения ножа, если больше нуля, то крутим влево и наоборот
    if(angle < 0)     
        cutter_eng_dir = 1;
    else if(angle > 0)
        cutter_eng_dir = 0;
    else
        engineZeroPoint();  // на случай, если угол будет указан как "0", то выполняем функцию возврата в нулевое положение(можно просто ничего не делать, так как нож по умолчанию в этом положении)
    cutterSteps = abs((angle * 3.2 * stepsPerRotateCutter)) / 360;       //abs - берем модуль(нас интересуют только положительные числа), angle - угол поворота,  3.2 - соотношение шестеренок,  stepsPerRotate - количество шагов на оборот двигателя
    for (unsigned short int i = cutterSteps; i > 0; i--) {  //подсчитав количество необходимых шагов - выполняем их
        cutter_eng_step = 1;
        wait_us(3);
        cutter_eng_step = 0;
        wait_us(600);      //пауза межде подачами сигнала, уменьшение этого числа увеличит скорость оборота
    }
}

//Функция возврата ножа в нулевое положение(до концевого датчика)
void engineZeroPoint()
{
    if (cutterZeroPosition.read() > 0.5) {   //Если концевой датчик не замкнут,то крутим по одному градусу, пока не замкнем его
        while(cutterZeroPosition.read() > 0.5) {
            turnCutter(1);
        }
        turnCutter(-46.5);    //от датчика, до абсолютного нуля 46,5 отсюда такое число
    }
}

//Функция подсчета отступа от первого угла(или для)
void calculateFirstIndent(int number)
{
    indentFirst = (templateSetting[tapeWidth][3] / 2) * tan(firstAngles[number]);
}

//Функция подсчета отступа от второго угла(или для)
void calculateSecondIndent(int number)
{
    indentSecond = (templateSetting[tapeWidth][3] / 2) * tan(secondAngles[number]);
}

//Функция для перемотки ленты
void moveForward(double length)
{
    pc.printf("\n\n Speed = %f",templateSetting[2][3]);
    tape_eng_step.period_us(templateSetting[2][3]);
    length = abs(length);
    length =  (length * 10) / 0.0625;
    tape_eng_step.pulsewidth_us(6);
    wait_ms((length/1000)*(templateSetting[2][3]+3));
    tape_eng_step = 0;
}

void addNewSection()
{
    firstAngles.push_back(0);
    secondAngles.push_back(0);
    tapesLength.push_back(10);
}

void checkTemplate()
{

}

void saveTemplate()
{
    for (uint8_t i = 0; i < 8; i++)
        templateToSave[i] = templateSetting[i][3];
}

void remakeName()
{
    for (uint8_t i = 0; i < sizeof(templateName); i++) {
        if (templateName[i] == 32)
            templateName[i] = 0;
    }
}

void setDefault()
{
    char minValue = 0;
    char maxValue = 1;
    char step = 2;
    char currentValue = 3;
    for (uint8_t i = 0; i < sizeof(templateName); i++)
        templateName[i] = 32;
    templateSetting[cuttingTime][minValue] = 0.1;
    templateSetting[cuttingTime][maxValue] = 30;
    templateSetting[cuttingTime][step] = 0.1;
    templateSetting[cuttingTime][currentValue] = 1;
    templateSetting[speed][minValue] = 100;
    templateSetting[speed][maxValue] = 1000;
    templateSetting[speed][step] = 50;
    templateSetting[speed][currentValue] = 400;
    templateSetting[accTime][minValue] = 1;
    templateSetting[accTime][maxValue] = 10000;
    templateSetting[accTime][step] = 1;
    templateSetting[accTime][currentValue] = 3000;
    templateSetting[tapeWidth][minValue] = 0.1;
    templateSetting[tapeWidth][maxValue] = 6.0;
    templateSetting[tapeWidth][step] = 0.1;
    templateSetting[tapeWidth][currentValue] = 4.5;
    templateSetting[firstAngle][minValue] = -45;
    templateSetting[firstAngle][maxValue] = 45;
    templateSetting[firstAngle][step] = 1;
    templateSetting[firstAngle][currentValue] = 5;
    templateSetting[secondAngle][minValue] = -45;
    templateSetting[secondAngle][maxValue] = 45;
    templateSetting[secondAngle][step] = 1;
    templateSetting[secondAngle][currentValue] = 6;
    templateSetting[tapeLength][minValue] = 0.1;
    templateSetting[tapeLength][maxValue] = 1000;
    templateSetting[tapeLength][step] = 0.1;
    templateSetting[tapeLength][currentValue] = 0.7;
    saveTemplate();
}

void createNewTemplate(string name)
{
    for (uint8_t i = 0; i < 8; i++)
        pc.printf("\ntemplateToSave[i] = %f", templateToSave[i]);
    FILE *S1;
    char newTemplateName[250] = "";
    strcat(newTemplateName, "/sd/");
    char buf[10];
    sprintf(buf, "%d", counterOfTemplates);
    if (mode != SettingTemplate)
        strcat(newTemplateName, buf);
    strcat(newTemplateName, name.c_str());
    if (mode != SettingTemplate)
        strcat(newTemplateName, ".txt");
    S1 = fopen(newTemplateName, "w");
    for (uint8_t i = 0; i < 5; i++)
        fprintf(S1, "%f\n", templateToSave[i]);
    for (int i = 0; i < firstAngles.size(); i++) {
        fprintf(S1, "%f\n", firstAngles[i]);
        fprintf(S1, "%f\n", secondAngles[i]);
        fprintf(S1, "%f\n", tapesLength[i]);
    }
    fclose(S1);
}

void readTemplateList()
{
    filenames.clear();
    DIR *dp;               //Чтение директории и вывод всех имён файлов
    struct dirent *dirp;
    dp = opendir("/sd");
    pc.printf("\n1");

    while ((dirp = readdir(dp)) != NULL) {
        pc.printf("\n2");
        filenames.push_back(string(dirp->d_name));
        pc.printf("\nFilName=%s", string(dirp->d_name));
    }
    pc.printf("\n3");
    closedir(dp);
    pc.printf("\n4");
    counterOfTemplates = filenames.size();
}

void readTemplate(string name)
{
    firstAngles.clear();
    secondAngles.clear();
    tapesLength.clear();
    setDefault();
    FILE *S1;
    char newTemplateName[250] = "";
    strcat(newTemplateName, "/sd/");
    strcat(newTemplateName, name.c_str());
    templateName = name;
    S1 = fopen(newTemplateName, "r");
    if (S1 != NULL) {
        pc.printf("Opened\n");
        char buf[250];
        char stringCounter = 0;
        int writingCounter = 0;
        while (fgets(buf, 250 - 1, S1)) {
            if ((stringCounter >= 1) && (stringCounter < 5))
                templateSetting[stringCounter][3] = atof(buf);
            else if (stringCounter >= 1) {
                if (writingCounter == 0) {
                    //pc.printf("\nWriting to firstAngles,because WC = %d", writingCounter);
                    firstAngles.push_back(atof(buf));
                    writingCounter = 1;
                }  // ПО ОЧЕРЕДИ НЕ ВСЕ СРАЗУ
                else if (writingCounter == 1) {
                    //pc.printf("\nWriting to secondAngles,because WC = %d", writingCounter);
                    secondAngles.push_back(atof(buf));
                    writingCounter = 2;
                } else if (writingCounter == 2) {
                    //pc.printf("\nWriting to tapeLength,because WC = %d", writingCounter);
                    tapesLength.push_back(atof(buf));
                    writingCounter = 0;
                }

            }
            stringCounter++;
            pc.printf("\n\n\nstringCounter =%d", stringCounter);
            pc.printf("\natof =%f", atof(buf));

            for (int i = 0; i < firstAngles.size(); i++)
                pc.printf("\nfirstANgles = %f", firstAngles[i]);
            for (int i = 0; i < secondAngles.size(); i++)
                pc.printf("\nsecondANgles = %f", secondAngles[i]);
            for (int i = 0; i < tapesLength.size(); i++)
                pc.printf("\ntapesANgles = %f", tapesLength[i]);

        }
        fclose(S1);
    }
    sectionCounter = tapesLength.size() - 1;
    //sectionsQty = tapesLength.size() - 1;
}

void displayRefresh()
{
    switch (mode) {
        case (PowerON):
            lcd.cls();
            lcd.printf("\nPower ON");
            break;

        case (ReadyToWork):
            lcd.cls();
            lcd.printf("Ready To Work");
            lcd.printf("\nTemplate Number:");
            lcd.printf("\nKits:%d", kitsDone);
            lcd.printf("/%d", kitsQty);
            lcd.printf(" %d", sectionsDone);
            lcd.printf("/%d", sectionsQty);
            break;

        case (SelectTemplate):
            switch (selectTemplateCounter) {
                case (0):
                    lcd.cls();
                    pc.printf("counter = %d", selectTemplateCounter);
                    lcd.printf("Single Tape Mode");
                    pc.printf("\nsingle tape mode");
                    break;
                case (1):
                    lcd.cls();
                    lcd.printf("Create New Template");
                    pc.printf("\ncreate new template");
                    pc.printf("counter = %d", selectTemplateCounter);
                    break;
                default:
                    pc.printf("\nselect template");
                    pc.printf("counter = %d", selectTemplateCounter);
                    lcd.cls();
                    lcd.printf("Select Template");
                    lcd.printf("\n%s", filenames[templateListCounter]);
                    break;
            }
            break;

        case (CreateTemplate):
            if (!changing) {
                switch (createTemplateCounter) {
                    case (tmpltName):
                        lcd.cls();
                        lcd.printf("\nTemplate Name");
                        break;
                    case (cuttingTime):
                        lcd.cls();
                        lcd.printf("Cutting Time");
                        lcd.printf("\n%f", templateSetting[cuttingTime][3]);
                        break;
                    case (speed):
                        lcd.cls();
                        lcd.printf("Speed");
                        lcd.printf("\n%f", templateSetting[speed][3]);
                        break;
                    case (accTime):
                        lcd.cls();
                        lcd.printf("Acceleration Time");
                        lcd.printf("\n%f", templateSetting[accTime][3]);
                        break;
                    case (tapeWidth):
                        lcd.cls();
                        lcd.printf("Width");
                        lcd.printf("\n%f", templateSetting[tapeWidth][3]);
                        break;
                    case (firstAngle):
                        lcd.cls();
                        lcd.printf("First Angle");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", firstAngles[sectionShowCounter]);
                        break;
                    case (secondAngle):
                        lcd.cls();
                        lcd.printf("Second Angle");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", secondAngles[sectionShowCounter]);
                        break;
                    case (tapeLength):
                        lcd.cls();
                        lcd.printf("Length");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", tapesLength[sectionShowCounter]);
                        break;
                    case (tapeLength+1):
                        lcd.cls();
                        if ((firstAngles.size() > 1) && (sectionShowCounter != sectionCounter)) {
                            sectionShowCounter++;
                            createTemplateCounter = firstAngle;
                            displayRefresh();
                        } else {
                            lcd.printf("Another one section?");
                            lcd.printf("Press 5 to add\n");
                            lcd.printf("Or Start(A) to Exit");
                        }
                        break;
                }
            } else {
                switch (createTemplateCounter) {
                    case (tmpltName):
                        lcd.cls();
                        for (uint8_t i = 0; i < sizeof(templateName); i++)
                            lcd.printf("%c", templateName[i]);
                        break;
                    case (cuttingTime):
                        lcd.cls();
                        lcd.printf("Cutting Time - changing");
                        lcd.printf("\n%f", templateSetting[cuttingTime][3]);
                        break;
                    case (speed):
                        lcd.cls();
                        lcd.printf("Speed - changing");
                        lcd.printf("\n%f", templateSetting[speed][3]);
                        break;
                    case (accTime):
                        lcd.cls();
                        lcd.printf("Acceleration Time - changing");
                        lcd.printf("\n%f", templateSetting[accTime][3]);
                        break;
                    case (tapeWidth):
                        lcd.cls();
                        lcd.printf("Width - changing");
                        lcd.printf("\n%f", templateSetting[tapeWidth][3]);
                        break;
                    case (firstAngle):
                        lcd.cls();
                        lcd.printf("First Angle - changing");
                        lcd.printf("\n%f", firstAngles[sectionShowCounter]);
                        break;
                    case (secondAngle):
                        lcd.cls();
                        lcd.printf("Second Angle - changing");
                        lcd.printf("\n%f", secondAngles[sectionShowCounter]);
                        break;
                    case (tapeLength):
                        lcd.cls();
                        lcd.printf("Length - changing");
                        lcd.printf("\n%f", tapesLength[sectionShowCounter]);
                        break;
                    case (tapeLength+1):
                        lcd.cls();
                        if ((firstAngles.size() > 1) && (sectionShowCounter != sectionCounter)) {
                            sectionShowCounter++;
                            createTemplateCounter = firstAngle;
                            displayRefresh();
                        } else {
                            lcd.printf("Another one section?");
                            lcd.printf("Press 5 to add\n");
                            lcd.printf("Or Start(A) to Exit");
                        }
                        break;
                }
            }
            break;

        case (SettingTemplate):
            if (!changing) {
                switch (settingTemplateCounter) {
                    case (tmpltName):
                        lcd.cls();
                        lcd.printf("\nTemplate Name");
                        break;
                    case (cuttingTime):
                        lcd.cls();
                        lcd.printf("Cutting Time");
                        lcd.printf("\n%f", templateSetting[cuttingTime][3]);
                        break;
                    case (speed):
                        lcd.cls();
                        lcd.printf("Speed");
                        lcd.printf("\n%f", templateSetting[speed][3]);
                        break;
                    case (accTime):
                        lcd.cls();
                        lcd.printf("Acceleration Time");
                        lcd.printf("\n%f", templateSetting[accTime][3]);
                        break;
                    case (tapeWidth):
                        lcd.cls();
                        lcd.printf("Width");
                        lcd.printf("\n%f", templateSetting[tapeWidth][3]);
                        break;
                    case (firstAngle):
                        lcd.cls();
                        lcd.printf("First Angle");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", firstAngles[sectionShowCounter]);
                        break;
                    case (secondAngle):
                        lcd.cls();
                        lcd.printf("Second Angle");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", secondAngles[sectionShowCounter]);
                        break;
                    case (tapeLength):
                        lcd.cls();
                        lcd.printf("Length");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", tapesLength[sectionShowCounter]);
                        break;
                    case (tapeLength+1):
                        lcd.cls();
                        if ((firstAngles.size() > 1) && (sectionShowCounter != sectionCounter)) {
                            sectionShowCounter++;
                            settingTemplateCounter = firstAngle;
                            displayRefresh();
                        } else {
                            lcd.printf("Another one section?");
                            lcd.printf("Press 5 to add\n");
                            lcd.printf("Or Start(A) to Exit");
                        }
                        break;
                }
            } else {
                switch (settingTemplateCounter) {
                    case (tmpltName):
                        lcd.cls();
                        for (uint8_t i = 0; i < sizeof(templateName); i++)
                            lcd.printf("%c", templateName[i]);
                        break;
                    case (cuttingTime):
                        lcd.cls();
                        lcd.printf("Cutting Time - changing");
                        lcd.printf("\n%f", templateSetting[cuttingTime][3]);
                        break;
                    case (speed):
                        lcd.cls();
                        lcd.printf("Speed - changing");
                        lcd.printf("\n%f", templateSetting[speed][3]);
                        break;
                    case (accTime):
                        lcd.cls();
                        lcd.printf("Acceleration Time - changing");
                        lcd.printf("\n%f", templateSetting[accTime][3]);
                        break;
                    case (tapeWidth):
                        lcd.cls();
                        lcd.printf("Width - changing");
                        lcd.printf("\n%f", templateSetting[tapeWidth][3]);
                        break;
                    case (firstAngle):
                        lcd.cls();
                        lcd.printf("First Angle - changing");
                        lcd.printf("\n%f", firstAngles[sectionShowCounter]);
                        break;
                    case (secondAngle):
                        lcd.cls();
                        lcd.printf("Second Angle - changing");
                        lcd.printf("\n%f", secondAngles[sectionShowCounter]);
                        break;
                    case (tapeLength):
                        lcd.cls();
                        lcd.printf("Length - changing");
                        lcd.printf("\n%f", tapesLength[sectionShowCounter]);
                        break;
                    case (tapeLength+1):
                        lcd.cls();
                        if ((firstAngles.size() > 1) && (sectionShowCounter != sectionCounter)) {
                            sectionShowCounter++;
                            settingTemplateCounter = firstAngle;
                            displayRefresh();
                        } else {
                            lcd.printf("Another one section?");
                            lcd.printf("Press 5 to add\n");
                            lcd.printf("Or Start(A) to Exit");
                        }
                        break;
                }
            }
            break;

        case (SingleTape):
            if (!changing) {
                switch (singleTemplateCounter) {
                    case (tmpltName):
                        lcd.cls();
                        lcd.printf("\nSingle Mode Template");
                        break;
                    case (cuttingTime):
                        lcd.cls();
                        lcd.printf("Cutting Time");
                        lcd.printf("\n%f", templateSetting[cuttingTime][3]);
                        break;
                    case (speed):
                        lcd.cls();
                        lcd.printf("Speed");
                        lcd.printf("\n%f", templateSetting[speed][3]);
                        break;
                    case (accTime):
                        lcd.cls();
                        lcd.printf("Acceleration Time");
                        lcd.printf("\n%f", templateSetting[accTime][3]);
                        break;
                    case (tapeWidth):
                        lcd.cls();
                        lcd.printf("Width");
                        lcd.printf("\n%f", templateSetting[tapeWidth][3]);
                        break;
                    case (firstAngle):
                        lcd.cls();
                        lcd.printf("First Angle");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", firstAngles[sectionShowCounter]);
                        break;
                    case (secondAngle):
                        lcd.cls();
                        lcd.printf("Second Angle");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", secondAngles[sectionShowCounter]);
                        break;
                    case (tapeLength):
                        lcd.cls();
                        lcd.printf("Length");
                        lcd.printf("\nSection #%d", sectionShowCounter);
                        lcd.printf("\n%f", tapesLength[sectionShowCounter]);
                        break;
                    case (tapeLength+1):
                        lcd.cls();
                        if ((firstAngles.size() > 1) && (sectionShowCounter != sectionCounter)) {
                            sectionShowCounter++;
                            singleTemplateCounter = firstAngle;
                            displayRefresh();
                        } else {
                            lcd.printf("Another one section?");
                            lcd.printf("Press 5 to add\n");
                            lcd.printf("Or Start(A) to Exit");
                        }
                        break;
                }
            } else {
                switch (singleTemplateCounter) {
                    case (tmpltName):
                        lcd.cls();
                        for (uint8_t i = 0; i < sizeof(templateName); i++)
                            lcd.printf("%c", templateName[i]);
                        break;
                    case (cuttingTime):
                        lcd.cls();
                        lcd.printf("Cutting Time - changing");
                        lcd.printf("\n%f", templateSetting[cuttingTime][3]);
                        break;
                    case (speed):
                        lcd.cls();
                        lcd.printf("Speed - changing");
                        lcd.printf("\n%f", templateSetting[speed][3]);
                        break;
                    case (accTime):
                        lcd.cls();
                        lcd.printf("Acceleration Time - changing");
                        lcd.printf("\n%f", templateSetting[accTime][3]);
                        break;
                    case (tapeWidth):
                        lcd.cls();
                        lcd.printf("Width - changing");
                        lcd.printf("\n%f", templateSetting[tapeWidth][3]);
                        break;
                    case (firstAngle):
                        lcd.cls();
                        lcd.printf("First Angle - changing");
                        lcd.printf("\n%f", firstAngles[sectionShowCounter]);
                        break;
                    case (secondAngle):
                        lcd.cls();
                        lcd.printf("Second Angle - changing");
                        lcd.printf("\n%f", secondAngles[sectionShowCounter]);
                        break;
                    case (tapeLength):
                        lcd.cls();
                        lcd.printf("Length - changing");
                        lcd.printf("\n%f", tapesLength[sectionShowCounter]);
                        break;
                    case (tapeLength+1):
                        lcd.cls();
                        if ((firstAngles.size() > 1) && (sectionShowCounter != sectionCounter)) {
                            sectionShowCounter++;
                            singleTemplateCounter = firstAngle;
                            displayRefresh();
                        } else {
                            lcd.printf("Another one section?");
                            lcd.printf("Press 5 to add\n");
                            lcd.printf("Or Start(A) to Exit");
                        }
                        break;
                }
            }
            break;

        case (QuantitySelect):
            lcd.cls();
            lcd.printf("Qty Select");
            lcd.printf("\n%d", kitsQty);
            break;

        case (Work):
            lcd.cls();
            lcd.printf("Working");
            lcd.printf("\nTemplate Number:");
            lcd.printf("\nKits:%d", kitsDone);
            lcd.printf("/%d", kitsQty);
            lcd.printf(" %d", sectionsDone+1);
            lcd.printf("/%d", sectionsQty);
            break;
    }
}

void switchMode(int workMode)
{
    switch (workMode) {
        case (PowerON): {
            pc.printf("\n Switched Mode to PowerON");
            lcd.cls();
            lcd.printf("PowerOn");
            mode = PowerON;
        }
        break;
        case (ReadyToWork): {

            checkTemplate();
            pc.printf("\n Switched Mode to ReadyToWork");
            lcd.cls();
            lcd.printf("Ready To Work");
            sectionsQty = firstAngles.size();
            mode = ReadyToWork;
            displayRefresh();
        }
        break;

        case (SelectTemplate): {
            selectTemplateCounter = 0;
            templateListCounter = 0;
            readTemplateList();
            pc.printf("\n Switched Mode to SelectTemplate");
            lcd.cls();
            lcd.printf("Select Template");
            mode = SelectTemplate;
        }
        break;

        case (CreateTemplate): {
            firstAngles.clear();
            secondAngles.clear();
            tapesLength.clear();
            sectionCounter = 0;
            sectionShowCounter = 0;
            addNewSection();
            mode = CreateTemplate;
            createTemplateCounter = 0;
            pc.printf("\n Switched Mode to CreateTemplate");
            lcd.cls();
            lcd.printf("Create Template");
            displayRefresh();
            setDefault();
        }
        break;

        case (SettingTemplate): {
            sectionShowCounter = 0;
            templateNameCounter = 0;
            settingTemplateCounter = 0;
            lcd.cls();
            lcd.printf("Setting Template");
            mode = SettingTemplate;
            pc.printf("\n Switched Mode to SettingTemplate");

        }
        break;

        case (SingleTape): {
            singleTemplateCounter = 1;
            firstAngles.clear();
            secondAngles.clear();
            tapesLength.clear();
            addNewSection();
            sectionCounter = 0;
            sectionShowCounter = 0;
            pc.printf("\n Switched Mode to SingleTape");
            lcd.cls();
            lcd.printf("Single Tape");
            mode = SingleTape;
        }
        break;

        case (QuantitySelect): {
            pc.printf("\n Switched Mode to QuantitySelect");
            lcd.cls();
            lcd.printf("Quantity Select");
            mode = QuantitySelect;
        }
        break;

        case (Work): {
            pc.printf("\n Switched Mode to Work");
            lcd.cls();
            lcd.printf("Work");
            engineZeroPoint();
            //solenoidPush();
            mode = Work;
        }
        break;
    }
    displayRefresh();
}

void stopWork()
{

}

void checkTape()
{
    if (TapeSignal.read() > 0.1) {
        SoundSignal = 0;
        NoMaterialLED = 0;
    } else  {
        buzzer.beep(2200,0.5);
        wait(1);
        buzzer.beep(2200,0.5);
        wait(1);
        buzzer.beep(2200,0.5);
        NoMaterialLED = 1;
    }
}

void manualControl()
{
    if (numpad_1);
    if (numpad_2)
        turnCutter(1);
    if (numpad_3);
    if (numpad_4) {
        tape_eng_dir = 1;
        moveForward(0.1);
    }
    if (numpad_5) {
        solenoidPush();
    }
    if (numpad_6) {
        tape_eng_dir = 0;
        moveForward(0.1);
    }
    if (numpad_7);
    if (numpad_8)    //Эта функция должна работать в течении 5 мсек
        turnCutter(-1);

    if (numpad_9);
}

void modeProcessing()
{
    switch (mode) {
        case (PowerON):
            break;

        case (ReadyToWork): {
            checkTape();
            if((WorkLED == 0)&&(NoMaterialLED == 0)&&(CutterFaultLED == 0))
                StandbyLED = 1;
            if ((startBtn) && (kitsQty != 0))
                switchMode(Work);
            if (shiftBtn)
                switchMode(SelectTemplate);
            if (numpad_0)
                kitsDone = 0;
            if (qtyBtn)
                switchMode(QuantitySelect);
            manualControl();
        }
        break;

        case (SelectTemplate): {
            if ((numpad_6) && (selectTemplateCounter <= counterOfTemplates)) {
                selectTemplateCounter++;
                if (selectTemplateCounter > 2)
                    templateListCounter++;
                displayRefresh();
            }
            if ((numpad_4) && (selectTemplateCounter > 0)) {
                selectTemplateCounter--;
                if (templateListCounter > 0)
                    templateListCounter--;
                displayRefresh();
            }

            switch (selectTemplateCounter) {
                case (0):
                    if (startBtn)
                        switchMode(SingleTape);
                    break;

                case (1):
                    if (startBtn)
                        switchMode(CreateTemplate);
                    break;

                default: {
                    if (startBtn) {
                        readTemplate(filenames[templateListCounter]);
                        switchMode(ReadyToWork);

                    }
                    if (settingBtn) {
                        pc.printf("\n1");
                        readTemplate(filenames[templateListCounter]);
                        pc.printf("\n2");
                        for (uint8_t i = 0; i < sizeof(templateName); i++)
                            bufferName[i] = templateName[i];
                        pc.printf("\n3");
                        for (uint8_t i = 0; i < 8; i++)
                            templateToSave[i] = templateSetting[i][3];
                        pc.printf("\n4");
                        switchMode(SettingTemplate);
                        pc.printf("\n5");
                    }
                }
                break;
            }
        }
        break;

        case (CreateTemplate): {
            if (!changing) {
                if ((numpad_6) && (createTemplateCounter < 8)) {
                    createTemplateCounter++;
                    displayRefresh();
                }

                if ((numpad_4) && (createTemplateCounter > 0)) {
                    if ((sectionCounter > 0) && (sectionShowCounter > 0) && (createTemplateCounter == firstAngle)) {
                        createTemplateCounter = tapeLength;
                        sectionShowCounter--;
                    } else
                        createTemplateCounter--;
                    displayRefresh();
                }
            } else {
                if (createTemplateCounter == tmpltName) {
                    if ((numpad_6) && (templateNameCounter < 20)) {
                        templateNameCounter++;
                        pc.printf("\ntemplateNameCounter = %d", templateNameCounter);
                    }

                    if ((numpad_4) && (templateNameCounter > 0)) {
                        templateNameCounter--;
                        pc.printf("\ntemplateNameCounter = %d", templateNameCounter);
                    }

                    if ((numpad_8) && (templateName[templateNameCounter] < 90)) {
                        if (templateName[templateNameCounter] == 32)
                            templateName[templateNameCounter ] = 65;
                        else
                            templateName[templateNameCounter] = templateName[templateNameCounter] + 1;
                        displayRefresh();
                    }
                    if ((numpad_2) && (templateName[templateNameCounter] > 64)) {
                        if (templateName[templateNameCounter] == 65)
                            templateName[templateNameCounter ] = 32;
                        else
                            templateName[templateNameCounter] = templateName[templateNameCounter] - 1;
                        displayRefresh();
                    }
                } else if (createTemplateCounter == tapeLength) {
                    pc.printf("\nsectionShowCounter = %d", sectionShowCounter);
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", tapesLength[sectionShowCounter]);
                    if ((numpad_6) && (tapesLength[sectionShowCounter] < templateSetting[createTemplateCounter][1])) {
                        tapesLength[sectionShowCounter] += templateSetting[createTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (tapesLength[sectionShowCounter] > templateSetting[createTemplateCounter][0])) {
                        tapesLength[sectionShowCounter] -= templateSetting[createTemplateCounter][2];
                        displayRefresh();
                    }

                } else if (createTemplateCounter == firstAngle) {
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", firstAngles[sectionShowCounter]);
                    if ((numpad_6) && (firstAngles[sectionShowCounter] < templateSetting[createTemplateCounter][1])) {
                        firstAngles[sectionShowCounter] += templateSetting[createTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (firstAngles[sectionShowCounter] > templateSetting[createTemplateCounter][0])) {
                        firstAngles[sectionShowCounter] -= templateSetting[createTemplateCounter][2];
                        displayRefresh();
                    }

                } else if (createTemplateCounter == secondAngle) {
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", secondAngles[sectionShowCounter]);
                    if ((numpad_6) && (secondAngles[sectionShowCounter] < templateSetting[createTemplateCounter][1])) {
                        secondAngles[sectionShowCounter] += templateSetting[createTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (secondAngles[sectionShowCounter] > templateSetting[createTemplateCounter][0])) {
                        secondAngles[sectionShowCounter] -= templateSetting[createTemplateCounter][2];
                        displayRefresh();
                    }
                } else {
                    if ((numpad_6) && (templateSetting[createTemplateCounter][3] < templateSetting[createTemplateCounter][1])) {
                        templateSetting[createTemplateCounter][3] += templateSetting[createTemplateCounter][2];
                        displayRefresh();
                    }
                    if ((numpad_4) && (templateSetting[createTemplateCounter][3] > templateSetting[createTemplateCounter][0])) {
                        templateSetting[createTemplateCounter][3] -= templateSetting[createTemplateCounter][2];
                        displayRefresh();
                    }
                }
            }
            if (numpad_5) {
                if (createTemplateCounter == 8) {
                    addNewSection();
                    sectionCounter++;
                    sectionShowCounter++;
                    createTemplateCounter = firstAngle;
                    displayRefresh();

                } else {
                    if (!changing)
                        changing = true;
                    else {
                        saveTemplate();
                        changing = false;
                    }
                    displayRefresh();
                }
            }

            if (escBtn) {
                if (!changing) {
                    templateNameCounter = 0;
                    switchMode(SelectTemplate);
                } else
                    changing = false;
            }
            if ((startBtn) && (!changing)) {
                lcd.cls();
                lcd.printf("Saving Template...");
                wait(0.1);
                remakeName();
                switchMode(SelectTemplate);
                createNewTemplate(templateName);
                readTemplateList();
                templateNameCounter = 0;
            }
            if ((shiftBtn) && (sectionCounter > 0)) {
                sectionCounter--;
                firstAngles.erase(firstAngles.begin() + sectionShowCounter);
                secondAngles.erase(secondAngles.begin() + sectionShowCounter);
                tapesLength.erase(tapesLength.begin() + sectionShowCounter);
                sectionShowCounter = 0;
                createTemplateCounter = firstAngle;
                displayRefresh();
            }
        }
        break;

        case (SettingTemplate): {
            if (!changing) {
                if ((numpad_6) && (settingTemplateCounter < 8)) {
                    settingTemplateCounter++;
                    displayRefresh();
                }

                if ((numpad_4) && (settingTemplateCounter > 0)) {
                    if ((sectionCounter > 0) && (sectionShowCounter > 0) && (settingTemplateCounter == firstAngle)) {
                        settingTemplateCounter = tapeLength;
                        sectionShowCounter--;
                    } else
                        settingTemplateCounter--;
                    displayRefresh();
                }
            } else {
                if (settingTemplateCounter == tmpltName) {
                    if ((numpad_6) && (templateNameCounter < 20)) {
                        templateNameCounter++;
                        pc.printf("\ntemplateNameCounter = %d", templateNameCounter);
                    }

                    if ((numpad_4) && (templateNameCounter > 0)) {
                        templateNameCounter--;
                        pc.printf("\ntemplateNameCounter = %d", templateNameCounter);
                    }

                    if ((numpad_8) && (templateName[templateNameCounter] < 90)) {
                        if (templateName[templateNameCounter] == 32)
                            templateName[templateNameCounter ] = 65;
                        else
                            templateName[templateNameCounter] = templateName[templateNameCounter] + 1;
                        displayRefresh();
                    }
                    if ((numpad_2) && (templateName[templateNameCounter] > 64)) {
                        if (templateName[templateNameCounter] == 65)
                            templateName[templateNameCounter ] = 32;
                        else
                            templateName[templateNameCounter] = templateName[templateNameCounter] - 1;
                        displayRefresh();
                    }
                } else if (settingTemplateCounter == tapeLength) {
                    pc.printf("\nsectionShowCounter = %d", sectionShowCounter);
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", tapesLength[sectionShowCounter]);
                    if ((numpad_6) && (tapesLength[sectionShowCounter] < templateSetting[settingTemplateCounter][1])) {
                        tapesLength[sectionShowCounter] += templateSetting[settingTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (tapesLength[sectionShowCounter] > templateSetting[settingTemplateCounter][0])) {
                        tapesLength[sectionShowCounter] -= templateSetting[settingTemplateCounter][2];
                        displayRefresh();
                    }

                } else if (settingTemplateCounter == firstAngle) {
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", firstAngles[sectionShowCounter]);
                    if ((numpad_6) && (firstAngles[sectionShowCounter] < templateSetting[settingTemplateCounter][1])) {
                        firstAngles[sectionShowCounter] += templateSetting[settingTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (firstAngles[sectionShowCounter] > templateSetting[settingTemplateCounter][0])) {
                        firstAngles[sectionShowCounter] -= templateSetting[settingTemplateCounter][2];
                        displayRefresh();
                    }

                } else if (settingTemplateCounter == secondAngle) {
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", secondAngles[sectionShowCounter]);
                    if ((numpad_6) && (secondAngles[sectionShowCounter] < templateSetting[settingTemplateCounter][1])) {
                        secondAngles[sectionShowCounter] += templateSetting[settingTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (secondAngles[sectionShowCounter] > templateSetting[settingTemplateCounter][0])) {
                        secondAngles[sectionShowCounter] -= templateSetting[settingTemplateCounter][2];
                        displayRefresh();
                    }
                } else {
                    if ((numpad_6) && (templateSetting[settingTemplateCounter][3] < templateSetting[settingTemplateCounter][1])) {
                        templateSetting[settingTemplateCounter][3] += templateSetting[settingTemplateCounter][2];
                        displayRefresh();
                    }
                    if ((numpad_4) && (templateSetting[settingTemplateCounter][3] > templateSetting[settingTemplateCounter][0])) {
                        templateSetting[settingTemplateCounter][3] -= templateSetting[settingTemplateCounter][2];
                        displayRefresh();
                    }
                }
            }
            if (numpad_5) {
                if (settingTemplateCounter == 8) {
                    addNewSection();
                    sectionCounter++;
                    sectionShowCounter++;
                    settingTemplateCounter = firstAngle;
                    displayRefresh();

                } else {
                    if (!changing)
                        changing = true;
                    else {
                        saveTemplate();
                        changing = false;
                    }
                    displayRefresh();
                }
            }

            if (escBtn) {
                if (!changing) {
                    templateNameCounter = 0;
                    switchMode(SelectTemplate);
                } else
                    changing = false;
            }
            if ((startBtn) && (!changing)) {
                lcd.cls();
                lcd.printf("Saving Template...");
                wait(0.1);
                remakeName();
                createNewTemplate(templateName);
                switchMode(SelectTemplate);
                readTemplateList();
                templateNameCounter = 0;
            }
            if ((shiftBtn) && (sectionCounter > 0)) {
                sectionCounter--;
                firstAngles.erase(firstAngles.begin() + sectionShowCounter);
                secondAngles.erase(secondAngles.begin() + sectionShowCounter);
                tapesLength.erase(tapesLength.begin() + sectionShowCounter);
                sectionShowCounter = 0;
                createTemplateCounter = firstAngle;
                displayRefresh();
            }
        }

        break;

        case (SingleTape): {
            if (!changing) {
                if ((numpad_6) && (singleTemplateCounter < 8)) {
                    singleTemplateCounter++;
                    displayRefresh();
                }

                if ((numpad_4) && (singleTemplateCounter > 1)) {
                    if ((sectionCounter > 0) && (sectionShowCounter > 0) && (singleTemplateCounter == firstAngle)) {
                        singleTemplateCounter = tapeLength;
                        sectionShowCounter--;
                    } else
                        singleTemplateCounter--;
                    displayRefresh();
                }
            } else {
                if (singleTemplateCounter == tmpltName) {
                    if ((numpad_6) && (templateNameCounter < 20)) {
                        templateNameCounter++;
                        pc.printf("\ntemplateNameCounter = %d", templateNameCounter);
                    }

                    if ((numpad_4) && (templateNameCounter > 0)) {
                        templateNameCounter--;
                        pc.printf("\ntemplateNameCounter = %d", templateNameCounter);
                    }

                    if ((numpad_8) && (templateName[templateNameCounter] < 90)) {
                        if (templateName[templateNameCounter] == 32)
                            templateName[templateNameCounter ] = 65;
                        else
                            templateName[templateNameCounter] = templateName[templateNameCounter] + 1;
                        displayRefresh();
                    }
                    if ((numpad_2) && (templateName[templateNameCounter] > 64)) {
                        if (templateName[templateNameCounter] == 65)
                            templateName[templateNameCounter ] = 32;
                        else
                            templateName[templateNameCounter] = templateName[templateNameCounter] - 1;
                        displayRefresh();
                    }
                } else if (singleTemplateCounter == tapeLength) {
                    pc.printf("\nsectionShowCounter = %d", sectionShowCounter);
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", tapesLength[sectionShowCounter]);
                    if ((numpad_6) && (tapesLength[sectionShowCounter] < templateSetting[singleTemplateCounter][1])) {
                        tapesLength[sectionShowCounter] += templateSetting[singleTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (tapesLength[sectionShowCounter] > templateSetting[singleTemplateCounter][0])) {
                        tapesLength[sectionShowCounter] -= templateSetting[singleTemplateCounter][2];
                        displayRefresh();
                    }

                } else if (singleTemplateCounter == firstAngle) {
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", firstAngles[sectionShowCounter]);
                    if ((numpad_6) && (firstAngles[sectionShowCounter] < templateSetting[singleTemplateCounter][1])) {
                        firstAngles[sectionShowCounter] += templateSetting[singleTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (firstAngles[sectionShowCounter] > templateSetting[singleTemplateCounter][0])) {
                        firstAngles[sectionShowCounter] -= templateSetting[singleTemplateCounter][2];
                        displayRefresh();
                    }

                } else if (singleTemplateCounter == secondAngle) {
                    pc.printf("\ntapesLength[sectionShowCounter] = %f", secondAngles[sectionShowCounter]);
                    if ((numpad_6) && (secondAngles[sectionShowCounter] < templateSetting[singleTemplateCounter][1])) {
                        secondAngles[sectionShowCounter] += templateSetting[singleTemplateCounter][2];
                        displayRefresh();
                    }

                    if ((numpad_4) && (secondAngles[sectionShowCounter] > templateSetting[singleTemplateCounter][0])) {
                        secondAngles[sectionShowCounter] -= templateSetting[singleTemplateCounter][2];
                        displayRefresh();
                    }
                } else {
                    if ((numpad_6) && (templateSetting[singleTemplateCounter][3] < templateSetting[singleTemplateCounter][1])) {
                        templateSetting[singleTemplateCounter][3] += templateSetting[singleTemplateCounter][2];
                        displayRefresh();
                    }
                    if ((numpad_4) && (templateSetting[singleTemplateCounter][3] > templateSetting[singleTemplateCounter][0])) {
                        templateSetting[singleTemplateCounter][3] -= templateSetting[singleTemplateCounter][2];
                        displayRefresh();
                    }
                }
            }
            if (numpad_5) {
                if (singleTemplateCounter == 8) {
                    addNewSection();
                    sectionCounter++;
                    sectionShowCounter++;
                    singleTemplateCounter = firstAngle;
                    displayRefresh();

                } else {
                    if (!changing)
                        changing = true;
                    else {
                        saveTemplate();
                        changing = false;
                    }
                    displayRefresh();
                }
            }

            if (escBtn) {
                if (!changing) {
                    templateNameCounter = 0;
                    switchMode(SelectTemplate);
                } else
                    changing = false;
            }
            if ((startBtn) && (!changing)) {
                switchMode(ReadyToWork);
            }
            if ((shiftBtn) && (sectionCounter > 0)) {
                sectionCounter--;
                firstAngles.erase(firstAngles.begin() + sectionShowCounter);
                secondAngles.erase(secondAngles.begin() + sectionShowCounter);
                tapesLength.erase(tapesLength.begin() + sectionShowCounter);
                sectionShowCounter = 0;
                createTemplateCounter = firstAngle;
                displayRefresh();
            }
        }
        break;

        case (QuantitySelect):
            if (numpad_6) {
                kitsQty++;
                displayRefresh();
            }
            if ((numpad_4) && (kitsQty > 0)) {
                kitsQty--;
                displayRefresh();
            }
            if (numpad_9) {
                kitsQty += 10;
                displayRefresh();
            }
            if ((numpad_7) && (kitsQty >= 10)) {
                kitsQty -= 10;
                displayRefresh();
            }
            if (startBtn)
                switchMode(ReadyToWork);
            break;

        case (Work):
            checkTape();
            if (sectionsDone == 0) {
                calculateFirstIndent(sectionsDone);
                moveForward(indentFirst);
                turnCutter(firstAngles[sectionsDone]);
                solenoidPush();
                turnCutter(-firstAngles[sectionsDone]);
                calculateFirstIndent(sectionsDone);
                calculateSecondIndent(sectionsDone);
                moveForward(tapesLength[sectionsDone] - indentFirst - indentSecond + 0.2);
                turnCutter(secondAngles[sectionsDone]);
                solenoidPush();
                turnCutter(-secondAngles[sectionsDone]);
                sectionsDone++;
            } else {
                if (firstAngles[sectionsDone] == secondAngles[sectionsDone - 1]) {
                    calculateFirstIndent(sectionsDone);
                    calculateSecondIndent(sectionsDone);
                    moveForward(tapesLength[sectionsDone] - indentFirst - indentSecond + 0.2);
                    turnCutter(secondAngles[sectionsDone]);
                    solenoidPush();
                    turnCutter(-secondAngles[sectionsDone]);
                    sectionsDone++;
                } else if (secondAngles[sectionsDone - 1] > 0) {
                    if (firstAngles[sectionsDone] < 0) {
                        calculateSecondIndent(sectionsDone - 1);
                        calculateFirstIndent(sectionsDone);
                        moveForward(indentFirst + indentSecond + 0.2);
                        turnCutter(firstAngles[sectionsDone]);
                        solenoidPush();
                        turnCutter(-firstAngles[sectionsDone]);
                        calculateFirstIndent(sectionsDone);
                        calculateSecondIndent(sectionsDone);
                        moveForward(tapesLength[sectionsDone] - indentFirst - indentSecond + 0.2);
                        turnCutter(secondAngles[sectionsDone]);
                        solenoidPush();
                        turnCutter(-secondAngles[sectionsDone]);
                        sectionsDone++;
                    } else if (firstAngles[sectionsDone] > secondAngles[sectionsDone - 1]) {
                        calculateFirstIndent(sectionsDone - 1);
                        calculateSecondIndent(sectionsDone - 1);
                        moveForward(abs(indentFirst - indentSecond) + 0.2);
                        turnCutter(firstAngles[sectionsDone]);
                        solenoidPush();
                        turnCutter(-firstAngles[sectionsDone]);
                        calculateFirstIndent(sectionsDone);
                        calculateSecondIndent(sectionsDone);
                        moveForward(tapesLength[sectionsDone] - indentFirst - indentSecond + 0.2);
                        turnCutter(secondAngles[sectionsDone]);
                        solenoidPush();
                        turnCutter(-secondAngles[sectionsDone]);
                        sectionsDone++;
                    } else if (secondAngles[sectionsDone - 1] < 0) {
                        if (firstAngles[sectionsDone] < secondAngles[sectionsDone - 1]) {
                            calculateFirstIndent(sectionsDone - 1);
                            calculateSecondIndent(sectionsDone - 1);
                            moveForward(abs(indentFirst - indentSecond) + 0.2);
                            turnCutter(firstAngles[sectionsDone]);
                            solenoidPush();
                            turnCutter(-firstAngles[sectionsDone]);
                            calculateFirstIndent(sectionsDone);
                            calculateSecondIndent(sectionsDone);
                            moveForward(tapesLength[sectionsDone] - indentFirst - indentSecond + 0.2);
                            turnCutter(secondAngles[sectionsDone]);
                            solenoidPush();
                            turnCutter(-secondAngles[sectionsDone]);
                            sectionsDone++;
                        } else if(firstAngles[sectionsDone] > 0) {
                            calculateFirstIndent(sectionsDone);
                            calculateSecondIndent(sectionsDone - 1);
                            moveForward(indentFirst + indentSecond + 0.2);
                            turnCutter(firstAngles[sectionsDone]);
                            solenoidPush();
                            turnCutter(-firstAngles[sectionsDone]);
                            calculateFirstIndent(sectionsDone);
                            calculateSecondIndent(sectionsDone);
                            moveForward(tapesLength[sectionsDone] - indentFirst - indentSecond + 0.2);
                            turnCutter(secondAngles[sectionsDone]);
                            solenoidPush();
                            turnCutter(-secondAngles[sectionsDone]);
                            sectionsDone++;
                        }
                    }
                }
            }
            if (sectionsQty == sectionsDone) {
                kitsDone++;
                sectionsDone = 0;
                displayRefresh();
            }
            if (kitsDone == kitsQty) {
                switchMode(ReadyToWork);
                kitsDone = 0;
                displayRefresh();
            }
            if (escBtn)
                switchMode(ReadyToWork);
            break;
    }
}

void showFile(char *filename)
{
    char newTemplateName1[250] = "";
    strcat(newTemplateName1, "/sd/");
    strcat(newTemplateName1, filename);
    strcat(newTemplateName1, ".txt");

    FILE *S1;
    S1 = fopen(newTemplateName1, "r");
    if (S1 != NULL) {
        pc.printf("Opened\n");
        char buf[250];
        while (fgets(buf, 250 - 1, S1)) {
            pc.printf("%s", buf);
        }
        fclose(S1);
    } else {
        pc.printf("Not opened\n");
    }
}

void getKey()
{
    char key;
    key = keypadd.getKey();
    if (key != KEY_RELEASED) {
        pc.printf("key=%c\n\r", key);
        if (key == '1')
            numpad_1 = 1;

        else if (key == '2')
            numpad_2 = 1;

        else if (key == '3')
            numpad_3 = 1;

        else if (key == '4')
            numpad_4 = 1;

        else if (key == '5')
            numpad_5 = 1;

        else if (key == '6')
            numpad_6 = 1;

        else if (key == '7')
            numpad_7 = 1;

        else if (key == '8')
            numpad_8 = 1;

        else if (key == '9')
            numpad_9 = 1;

        else if (key == '0')
            numpad_0 = 1;

        else if (key == '*')
            escBtn = 1;

        else if (key == '#')
            shiftBtn = 1;

        else if (key == 'A')
            startBtn = 1;

        else if (key == 'B')
            stopBtn = 1;

        else if (key == 'C')
            settingBtn = 1;

        else if (key == 'D')
            qtyBtn = 1;
        wait(0.1);
    }
}

void returnKeys()
{
    numpad_0 = 0;
    numpad_1 = 0;
    numpad_2 = 0;
    numpad_3 = 0;
    numpad_4 = 0;
    numpad_5 = 0;
    numpad_6 = 0;
    numpad_7 = 0;
    numpad_8 = 0;
    numpad_9 = 0;
    escBtn = 0;
    shiftBtn = 0;
    startBtn = 0;
    stopBtn = 0;
    settingBtn = 0;
    qtyBtn = 0;
}

void  file_rename(const char *oldfileName, const char *newfileName)
{
    char oldfile[250] = "";
    strcat(oldfile, "/sd/");
    strcat(oldfile, oldfileName);
    strcat(oldfile, ".txt");

    char newfile[250] = "";
    strcat(newfile, "/sd/");
    strcat(newfile, newfileName);
    strcat(newfile, ".txt");
    FILE *fpold;
    FILE *fpnew;
    int ch;
    //fpold = fopen(oldfile, "r");
    //fpnew = fopen(newfile, "w");
    //    while (1) {
    //        ch = fgetc(fpold);
    //        if (ch == EOF) break;
    //        fputc(ch, fpnew);
    //    }
    //   fclose(fpnew);
    //   fclose(fpold);
    //    remove(oldfile);

    fpold = fopen("/sd/templateList.txt", "r");
    char buf[250] = "";
    char buf1[templateListCounter][250];
    templateListCounter = 0;
    while (fgets(buf, 250 - 1, fpold)) {
        strcpy(buf1[templateListCounter], buf);
        templateListCounter++;
    }
    fclose(fpold);

    for (uint8_t i = 0; i < templateListCounter; i++) {
        if (strcmp(buf1[i], oldfileName) == 0) {
            pc.printf("found eq");
            for (uint8_t j = 0; j < sizeof(buf1[i]); j++)
                buf1[i][j] = 0;
            for (uint8_t j = 0; j < sizeof(newfileName); j++)
                buf1[i][j] = newfileName[j];
        }

        pc.printf("String:%s", buf1[i]);
    }
    char loopCounter = templateListCounter;

    fpnew = fopen("/sd/templateList.txt", "w");

    for (uint8_t i = 0; i < loopCounter; i++)
        fprintf(fpnew, "%s", buf1[i]);
    fclose(fpnew);


    readTemplateList();

}

int main()
{
    PowerLED = 1;
    tape_eng_step.period_us(400);
    //buzzer.beep(2200,0.5);
    //engineZeroPoint();
    switchMode(ReadyToWork);
    for (uint8_t i = 0; i < sizeof(templateName); i++)
        templateName[i] = 32;
    lcd.setMode(TextLCD::DispOn); //DispOff, DispOn
    lcd.setBacklight(TextLCD::LightOff);//LightOff, LightOn
    lcd.setCursor(TextLCD::CurOff_BlkOff);//CurOff_BlkOff, CurOn_BlkOff, CurOff_BlkOn, CurOn_BlkOn
    readTemplateList();
    keypadd.enablePullUp();
    setDefault();
    addNewSection();
    moveForward(10);
    while (true) {
        getKey();
        modeProcessing();
        returnKeys();
    }
}