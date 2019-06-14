#include "HC-SR04.h"

const int triggerPin = A0;      // Вывод триггера для дальномера
const int echoPin = D0;         // Приём сигнала для дальномера
HC_SR04 rangefinder = HC_SR04(triggerPin, echoPin);

const int tapControlPin = A5;   // Пин на реле крана
const int led = D7;             // Тупо лампочка
const int buttPin = D3;         // Пин кнопки

// Опционально добавить даиметр бака или его базовый обьём, что бы считать заполненность для юзера
const int height = 84;          // Высота бака в cантиметрах 22

// Границы АВАРИЙНЫХ уровней
const int topAlarmLVL = 75;     // Верхний предельно аварийный ур-ь 20
const int lowerAlarmLVL = 5;    // Нижний предельно аварийный ур-ь  2

// Границы контрольных уровней
const int topBufferLVL = 70;    // Вернхний ур-ь буфера   При уровене воды в буффере - подачи нет. 19
const int bottomBufferLVL = 55; // Нижний ур-ь буфера 15
const int bottomNormalLVL = 10;  // Нижний ур-ь нормального обьемма. Если ниже - предупреждать 4

bool swopState = false;         // Подкачка воды: FALSE - не нжуно качать,  TRUE - нужно качать
int inputTapState = 0;          // Состояние крана подачи воды в бак. 0 - закрыт  1 - открыт
int manualMode = 0;             // Ручное управлени системой. Можно 0 - автоматика  1 - юзер через инет. На булевых облако не работает
int alarmFlag = 0;              // Флаг тевоги на устройстве: 0 - всё ок. 1 - очень высокий уровень.
bool buttFlag = false;          // Состояние кнопки. была ли нажаа при высоком уровне воды
//float currentLVL;
int sentLVL;



// Облачная функция перевода в ручное управления
int userManualControl(String command);

// Облачная функция переклчения краном
int userSwapControl(String command);

// Облачная функция отлючения тревоги
int userAlarmControl(String command);



// Управление краном. TRUE - открыть. FALSE - закрыть
void tapControl(bool command, int &state=inputTapState){

  //Serial.printf("state=%d command=%d ledPin=%d\n", state, command, led);

  if (command == true && state == 0){
    digitalWrite(tapControlPin, HIGH);
    state = true;
    Serial.printf("TAP CONTROL OPEN\n");

   // долго погорит и быстро поморгает
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    delay(50);
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    delay(50);
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    delay(50);
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);


    // Закрыть кран
  } else if (command == false && state == 1){
    digitalWrite(tapControlPin, LOW);
    state = false;

    Serial.printf("TAP CONTROL CLOSED\n");

    // По два раза быстро моргнёт
    digitalWrite(led, HIGH);
    delay(500);
    digitalWrite(led, LOW);
    delay(50);
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    delay(500);
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
    delay(50);
    digitalWrite(led, HIGH);
    delay(50);
    digitalWrite(led, LOW);
  }
}


// Высота водного столба
float avrDistance(){
    // 10  раз замерять и подсчитать среднее значение
    unsigned long startTime = millis();
    int errors = 0;             // Кол-во ошибок в подсчёте расстояние
    float values[10];           // Хранит подсчитаныные значния
    float result;               // Результат, который вернёт программа
    float tempValue;

    // Цикл 10 замеров
    for (int i = 0; i < 10; i++)
    {
        tempValue = rangefinder.distCM();

        // Если значение выход за рамки приличия и кол-во ошибок при подсчёте меньше 5, что бы не зависал
        while ( (tempValue == -1 || tempValue > height) && (errors <= 5) )
        {
            delay(250);
            errors += 1;
            tempValue = rangefinder.distCM();
            if (errors == 5){

              // Вернувшаяся высота будет равно нулю, что чаще попадается, если уровень низок
              tempValue = height;
              errors = 0;
            }
        }
        values[i] = tempValue;

        Serial.printf("Current value of cycle: %.2f\n", (height - values[i]));
        //Particle.publish("Current value of cycle", values[i]);
        delay(500);
    }

    // Если вычисление занимает более 20 секунд
    if (millis() - startTime > 20000)
    {
        Serial.printf("ERROR: Runtime is too long: %.2f\n", millis() - startTime);
        //Particle.publish("ERROR: Runtime is too long",startTime - millis());
        return -2;
    }

    // Нахождение результата. Сумма значнией делить на кол-во
    for (int i = 0; i < 10; i++)
    {
        result += values[i];
    }
    result /= 10;

    return height - result; // Вернёт ВЫСОТУ водного столба
}


// Функция для запуска функций отработчиков ситуаций действий
void checkState(){
    float currentLVL = avrDistance(); // Текущий уровень воды (локально)
    //currentLVL = avrDistance(); // Текущий уровень воды
    sentLVL = int(currentLVL);    // Значение, которое доступно из облака



    // Уровень ниже допустимого, но не смертельного
    if (currentLVL < bottomNormalLVL && currentLVL > lowerAlarmLVL){
      // buttFlag = false;
      if (manualMode == 0){
        swopState = true;
        tapControl(1);
      }

        Serial.printf("WARNING, low water level: %.1f\n", currentLVL);
        Serial.printf("\n ------------------- \n");
      //  Particle.publish("WARNING, low water level:", currentLVL);




    // АВАРИЯ, низкий уровень воды
    } else if (currentLVL < lowerAlarmLVL ){
        alarmFlag = 1; // Сигналка по любому включается при достижения уровня

        if (manualMode == 0){
          swopState = true;
          tapControl(1);
        }

        Serial.printf("ATTENTION, LOW WATER LEVEL: %.1f\n", currentLVL);
        Serial.printf("\n ------------------- \n");
        // Подача звукового сигнала
    //    Particle.publish("ATTENTION, LOW WATER LEVEL:", currentLVL ,60,PRIVATE);



    // Нормальный уровень воды, в пределаха нормы
    } else if (currentLVL >= bottomNormalLVL && currentLVL < bottomBufferLVL) {
      buttFlag = false;
      alarmFlag = 0;

      if (manualMode == 0){
        swopState = true;
        tapControl(1);
      }
        // tapControl(1);
        Serial.printf("Normal, current level of water: %.1f\n", currentLVL);
        Serial.printf("\n ------------------- \n");
        // Просто отправлять данные о текущем уровне
      //  Particle.publish("Normal, current level of water:", currentLVL ,60,PRIVATE);




    // Уровень воды в пределах буфера, работает подкачка до заополнения
    } else if (currentLVL >= bottomBufferLVL && currentLVL <= topBufferLVL){
        alarmFlag = 0;    // Тревога выключена
        buttFlag = false; // Кнопка теперь не нажата

        if (manualMode == 0  && swopState == true){
          tapControl(1);
        }
        // Уровень воды подошёл к верхнему пределу и нужно прекратить подкачку.
        if (currentLVL >= (topBufferLVL - 2) && currentLVL <= topBufferLVL) {
            // Закончить подкачку, закрыть кран
            swopState = false;
            if (manualMode == 0){   tapControl(0); }                               // Вот тут было 1!!!!
        }

        Serial.printf("Normal, current level of water in buffer: %.1f\n", currentLVL);
        Serial.printf("\n ------------------- \n");
      //  Particle.publish("Normal, current level of water in buffer:", currentLVL ,60,PRIVATE);



      // Уровень воды выше буфера, но ещё ниже сигнального порога
    } else if (currentLVL < topAlarmLVL && currentLVL > topBufferLVL){
        alarmFlag = 0;
        buttFlag = false;
        if (manualMode == 0){   tapControl(0); }
        swopState = false;
        Serial.printf("WARNING, high water level: %.1f\n", currentLVL);
        Serial.printf("\n ------------------- \n");
        //Particle.publish("WARNING, high water level:", currentLVL ,60,PRIVATE);


        // АВАРИЯ, уровень воды выще допустимого
  } else if (currentLVL >= topAlarmLVL){
        swopState = false;
        if (manualMode == 0){ tapControl(0); }

        alarmFlag = 1; // Сигналка по любому включается при достижения уровня

/*
        if ( buttFlag == false ) { // Кнопка подтвержедния не была нажата
          digitalWrite(led, HIGH);
          //alarmFlag = 1;    // Включается тревога
        } else if (buttFlag == true) {
          digitalWrite(led, LOW); // Кнопка была нажата, тревога больше не беспокоит
          // alarmFlag = 0;
        }
*/
        Serial.printf("ATTENTION, HIGH WATER LEVEL: %.1f\n", currentLVL);
        Serial.printf("\n ------------------- \n");

      //Particle.publish("ATTENTION, HIGH WATER LEVEL:", currentLVL ,60,PRIVATE);
        // Звуковой сигнал...  Вместо этого будет просто гореть лампочка

    } else {
      Serial.printf("FUCK YOU\n");
    }


}








void setup()
{
    // облачные переменные
    Particle.variable("manualMode", manualMode);  // Режим работы ручной/автомат
    Particle.variable("TapState", inputTapState); // Состояние крана
    Particle.variable("TankHeight", height);      // Высота бака
    Particle.variable("currentLVL", sentLVL);  // Уровень воды в баке
    Particle.variable("alarmFlag", alarmFlag);    // Состояние тревоги


    Particle.function("userManualControl", userManualControl);
    Particle.function("userSwapControl", userSwapControl);
    Particle.function("userAlarmControl", userAlarmControl);

    /*
    pinMode(power,OUTPUT); МБ через этот пин можно запитаьть датчик
    digitalWrite(power,HIGH);
    */

    pinMode(tapControlPin, OUTPUT);   // на реле
    pinMode(led, OUTPUT);             // на LED
    pinMode(buttPin, INPUT);          // на кнопку

    Serial.begin(9600);
    rangefinder.init();

}

void loop()
{

  // считвыаение проивходит раз в несоклько секунд, поэтому нужно держать кнопку пока не отключиться охрана
  if (digitalRead(buttPin) != HIGH && alarmFlag == 1){    // нажали кнопку
      alarmFlag = 0;
    buttFlag = true;  // Подтверждение того, что кнопка нажата
                      // Флаг тревоги будет, но не будет сигнализации

    Serial.printf("Alarm is turnerd off\n");
    digitalWrite(led, LOW); // Кнопка была нажата, тревога больше не беспокоит


  } else if (alarmFlag == 1 && buttFlag == false) { // Тревога, но кнопка не нажата
    Serial.printf("Alarm is working\n");
    digitalWrite(led, HIGH); // Кнопка была нажата, тревога больше не беспокоит

  } else if (alarmFlag == 1 && buttFlag == true) {    // Тревога, кнопка была уже нажата
      Serial.printf("Alarm is checked \n");
      digitalWrite(led, LOW); // Кнопка была нажата, тревога больше не беспокоит

  } else if (alarmFlag == 0){
    digitalWrite(led, LOW); // Кнопка была нажата, тревога больше не беспокоит
  }



  checkState();
}



// Облачная ф-я для управления краном.
int userSwapControl(String command){
  if (command == "OFF" /*&& swopState == true*/){
    tapControl(0);
    return 1;

  } else if (command == "ON" /*&& swopState == false*/){
    tapControl(1);
    return 1;
  } else return -1;
}


// Облачная ф-я для переключения ручного управления.
int userManualControl(String command){
    swopState = false;
    if (command == "manualON"){
      manualMode = 1;
      return 1;

    } else if (command == "manualOFF"){
      manualMode = 0;
      return 1;
    } else return -1;
}

// Облачная ф-я для переключения тревоги.
int userAlarmControl(String command){
  if (command == "OFF"){
    // alarmFlag = 0;
    buttFlag = true;
    return 1;
  } else if (command == "ON"){
    alarmFlag = 1;
    buttFlag = false;
    return 1;
  } else { return -1; }
}
