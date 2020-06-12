#include <EEPROM.h>
#include <DS1302.h>
#include <DFPlayer_Mini_Mp3.h>
//#include <time.h>
//#include <stdlib.h>
//
#include "arduinoFFT.h"
#include "string.h"
#include "math.h"
#include "DHT.h"

#define DHTPIN 22        // DHT11이 연결된 핀
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define SAMPLES 128             //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
#define SAMPLING_FREQUENCY 2048 //Ts = Based on Nyquist, must be 2 times the highest expected frequency.
arduinoFFT FFT = arduinoFFT();

//---------------------------블루투스------------------------------
struct dataSet {
  int hour;
  int minute;
  int second;
  float humidity;
  float temperature;
  float posX;
  float posY;
  float decibel;
  float frequency;
  bool sleeping;
};
typedef struct dataSet DataSet;
DataSet myDataSet;
DataSet *myDataSetPointer = &myDataSet;
long dataSendInterval;
//-----------------------------------------------------------------

//---------------------------알람 데이터-----------------------------
struct alarmData {
  int hour;
  int minute;
  bool alarmOperation;
  bool vibrateOperation;
  bool asmrOperation;
  int alarmTrack;
};
typedef struct alarmData AlarmData;
AlarmData currentAlarm;
//-----------------------------------------------------------------


//---------------------------RTC-----------------------------------
const int SCK_PIN = 50; // CLK
const int IO_PIN  = 48;  // DAT
const int RST_PIN = 46;  // RST
Time currTime;
DS1302 rtc(RST_PIN, IO_PIN, SCK_PIN);

long currMilliSecond;
//----------------------------------------------------------------

//--------------------------수면여부--------------------------------
const int TIME_SLEEP = 15000;
const int TIME_ASMR_START = 5000;
const int TIME_ASMR_STOP = 3000;
const int TIME_ASMR_SLEEP = 5000;

bool sleeping;
//----------------------------------------------------------------

bool sendingSerial;

void setup() {
  // Serial, 블루투스
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin (9600);
  dataSendInterval = millis();
  
  // 블루투스 송신용 데이터 초기화
  myDataSet.hour = 0;
  myDataSet.minute = 0;
  myDataSet.second = 0;
  myDataSet.humidity = 0.0;
  myDataSet.temperature = 0.0;
  myDataSet.posX = 0.0;
  myDataSet.posY = 0.0;
  myDataSet.decibel = 0.0;
  myDataSet.frequency = 0.0;
  myDataSet.sleeping = 0;

  // 모든 기능 셋업
  TempHumidSetup();
  AlarmTimeSetup();
  CushionSetup();
  MicrophoneSetup();
  SoundVibrationSetup();
}

void loop() {

  // Send bluetooth data every second
  currMilliSecond = millis();
  if (true) {
    if(dataSendInterval + 1000 < currMilliSecond){
      dataSendInterval = currMilliSecond;
      sendSerialData();
      sendBluetoothData();
      //sendingSerial = 1;
    }
  }

  //Total time per loop is 1 second
  currTime = rtc.getTime();
  
  if (Serial1.available()) {
    bluetoothAlarm();
  }
  
  TempHumid(myDataSetPointer);
  Microphone(myDataSetPointer);
  Cushion(myDataSetPointer);
  Alarm(myDataSetPointer);
  
}

void sendSerialData(){
  myDataSet.hour = currTime.hour;
  myDataSet.minute = currTime.min;
  myDataSet.second = currTime.sec;
  Serial.println("[BLUETOOTHDATA]");
  Serial.print("[hour]");
  Serial.println(myDataSet.hour);
  Serial.print("[minute]");
  Serial.println(myDataSet.minute);
  Serial.print("[second]");
  Serial.println(myDataSet.second);
  Serial.print("[humi]");
  Serial.println(myDataSet.humidity, 8);
  Serial.print("[temp]");
  Serial.println(myDataSet.temperature, 8);
  Serial.print("[posX]");
  Serial.println(myDataSet.posX, 8);
  Serial.print("[posY]");
  Serial.println(myDataSet.posY, 8);
  Serial.print("[decibel]");
  Serial.println(myDataSet.decibel, 8);
  Serial.print("[frequency]");
  Serial.println(myDataSet.frequency, 8);
  Serial.print("[sleeping]");
  Serial.println(myDataSet.sleeping);
  Serial.print("[AlarmOn]");
  Serial.println(currentAlarm.alarmOperation);
  Serial.print("[VibrateOn]");
  Serial.println(currentAlarm.vibrateOperation);
  Serial.print("[ASMROn]");
  Serial.println(currentAlarm.asmrOperation);
  Serial.print("[AlarmTrack]");
  Serial.println(currentAlarm.alarmTrack);
}

void sendBluetoothData(){
  myDataSet.hour = currTime.hour;
  myDataSet.minute = currTime.min;
  myDataSet.second = currTime.sec;
  Serial1.print("[SleepData]");
  Serial1.print(myDataSet.hour);
  Serial1.print(":");
  Serial1.print(myDataSet.minute);
  Serial1.print(":");
  Serial1.print(myDataSet.second);
  Serial1.print("$");
  Serial1.print(myDataSet.humidity, 8);
  Serial1.print("$");
  Serial1.print(myDataSet.temperature, 8);
  Serial1.print("$");
  Serial1.print(myDataSet.posX, 8);
  Serial1.print("$");
  Serial1.print(myDataSet.posY, 8);
  Serial1.print("$");
  Serial1.print(myDataSet.decibel, 8);
  Serial1.print("$");
  Serial1.print(myDataSet.frequency, 8);
  Serial1.print("$");
  if (myDataSet.sleeping) {
    Serial1.println("true");
  } else {
    Serial1.println("false");
  }
}
