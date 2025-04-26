//Переменные для работы с буфером, который хранит POST коды
#define MAX_PAST_QCODES 64
uint8_t pastQCodes[MAX_PAST_QCODES];
int pastQcodeHead = 0;
bool endBuffer = false;

const long WAIT_TIMEOUT = 1000; // Время ожидания заполненния буфера данными, перед их отправкой на ПК

//Переменные для таймера печати
unsigned long timeNow = 0;
bool printFlag = false;
bool haveInfoToPrint = false;

void setup() {
  Serial.begin(115200); //Настройка частоты обмена данными, для подключения к COM_DEBUG на материнской плате

  timeNow = millis(); // инициализация таймера
}

/*
 * Программа проксирует Q-CODE (далее POST коды) материнской платы ASUS с порта COM_DEBUG в SerialMonitor Arduino IDE.
 * 
 * Алгоритм работы:
 * Ожидание POST кодов, чтобы записать все коды в буфер, а затем вывести.
 * Если буфер заполнен, то печать всех кодов.
 * (POST коды заносятся в буфер, чтобы не мешать процессу считывания PSOT кодов. Т.е. когда видно, что POST коды не передаются, то в этот проммежуток происходит пересылка данных)
 * 
 * Инструкция как подключать: 
 * 1. RX с ARDUINO подключить к SOUTC_P80 (PIN1) COM_DEBUG на материнской плате
 * 2. GND с ARDUINO подключить к GND COM_DEBUG на материнской плате
 *    (GND)  V V
 *           o o o
 *  (PIN1) > o   o
 *
 * Таблица общих POST кодов: https://www.asus.com/support/faq/1043948/#5

+------------------------------------+----------------------------------+--------------+-----------------+-----------------------+--------------------------+
| Common Q-CODE of Intel motherboard | Common Q-CODE of AMD motherboard | CPU abnormal | Memory abnormal | Graphic Card abnormal |     Others abnormal      |
+------------------------------------+----------------------------------+--------------+-----------------+-----------------------+--------------------------+
| 00,D0                              |                                  | V            |                 |                       |                          |
| 53,55                              | F9                               |              | V               |                       |                          |
| D6                                 |                                  |              |                 | V                     |                          |
|                                    | B0,99,15,53                      | V            | V               |                       |                          |
| D6                                 |                                  |              | V               | V                     |                          |
|                                    | 00,19,30,40,55                   | V            | V               | V                     |                          |
| A0,A2                              | A0,A2                            |              |                 |                       | Boot up device abnormal  |
| B2                                 | B2                               |              |                 |                       | External device abnormal |
| A9                                 | A9                               |              |                 |                       | Boot into the BIOS       |
| AA                                 | AA                               |              |                 |                       | Boot into the system     |
+------------------------------------+----------------------------------+--------------+-----------------+-----------------------+--------------------------+
 *
 * Сделано на основе:
 * https://www.reddit.com/r/techsupportmacgyver/comments/16v3af7/asus_removed_qcode_displays_from_low_end/   
 * https://github.com/Sciguy429/Qcode-Breakout-Board
 *
 */
void loop() {
  if (millis() - timeNow > WAIT_TIMEOUT) { //Если POST коды не приходили более WAIT_TIMEOUT ms, то производим печать на экран
    printFlag = true; //выставляем флаг, что пришло время печати
  }

  endBuffer = pastQcodeHead >= MAX_PAST_QCODES; // Проверка достигнут ли конец буфера

  if (endBuffer || (printFlag && haveInfoToPrint)) { //Если прошло WAIT_TIMEOUT ms, и есть что печатать, или достигнут конец буфера то
    //Начинаем печать данных из буфера в цикле
    for (int i = 0; i < pastQcodeHead; i++) {
      Serial.println(pastQCodes[i], HEX);//печать в HEX виде POST кодов
    }
    pastQcodeHead = 0;//сброс счетчика буфера значений на начало
    haveInfoToPrint = false; //сброс флага, для последующего ожидания POST кодов
    clearTimerForPrintTask(); //обновляем таймер после вывода результатов, чтобы считать WAIT_TIMEOUT ms с этого места
  }

  if (Serial.available()) {
    pastQCodes[pastQcodeHead++] = Serial.read();//считывание информации в буфер
    haveInfoToPrint = true; //выставляем флаг, что пришли POST коды
    timeNow = millis();//если пришёл сигнал, то обновляем таймер, чтобы считать WAIT_TIMEOUT ms с этого места
  }
}

void clearTimerForPrintTask() {
  printFlag = false;
  timeNow = millis(); 
}
