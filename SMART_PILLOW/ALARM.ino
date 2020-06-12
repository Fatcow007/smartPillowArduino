//---------------------------시계/알람------------------------------
const int ALARM_HOUR_ADDRESS = 10; // For EEPROM
const int ALARM_MINUTE_ADDRESS = 11; // For EEPROM
const int ALARM_OPERATION_ADDRESS = 20; // For EEPROM
const int ASMR_OPERATION_ADDRESS = 21; // For EEPROM
const int VIBRATE_OPERATION_ADDRESS = 22; // For EEPROM
const int ALARM_TRACK_ADDRESS = 30; // For EEPROM

bool alarm = 0;
bool alarmoff = 0;
bool trigger_lock = 0;

int lastAlarmGesture = 0; // Used for gesture based alarm turn off
long alarmGestureStartMillis = 0;
long alarmOffMillis = 0;
//----------------------------------------------------------------

void AlarmTimeSetup(){
  rtc.halt(false);
  rtc.writeProtect(false);
  
  currentAlarm.hour   = EEPROM.read(ALARM_HOUR_ADDRESS);
  currentAlarm.minute = EEPROM.read(ALARM_MINUTE_ADDRESS);
  currentAlarm.alarmOperation = EEPROM.read(ALARM_OPERATION_ADDRESS);
  currentAlarm.vibrateOperation = EEPROM.read(VIBRATE_OPERATION_ADDRESS);
  currentAlarm.asmrOperation = EEPROM.read(ASMR_OPERATION_ADDRESS);
  currentAlarm.alarmTrack = EEPROM.read(ALARM_TRACK_ADDRESS);
}


//현 시간을 RTC로 검사, 알람시간시 알람을 울린다.
void Alarm(DataSet *d) {
  if (currentAlarm.alarmOperation) {
    if (sleeping){
      if ((currentAlarm.hour == currTime.hour) && (currentAlarm.minute == currTime.min)) {
        if (alarmoff) {
          disableAlarm();
          trigger_lock = 1;
        } else if (trigger_lock == 0) {
          alarm = 1;
          alarmSoundVibrate(alarm);
        }
      } else {
        trigger_lock = 0;
        if (alarm) {
          alarmSoundVibrate(alarm);
        }
        if (alarmoff) {
          disableAlarm();
        }
      }
    }
    d->sleeping = sleeping;
  }
}

void disableAlarm() {
  alarm = 0;
  alarmoff = false;
  sleeping = 0;
  alarmSoundVibrate(alarm);
  alarmOffMillis = currMilliSecond;
}

void alarmGesture(bool sigLeft, bool sigRight) {
  if (alarm) {
    Serial.println("Alarm is on!");
    if (sigLeft && sigRight) {
      lastAlarmGesture = 0;
      Serial.println("Alarm disabled by pushing both buttons");
    } else if (sigLeft) {
      if (lastAlarmGesture == 0) {
        lastAlarmGesture = 1;
        alarmGestureStartMillis = currMilliSecond;
        Serial.println("LEFT BUTTON");
      } else if (lastAlarmGesture == 2) {
        alarmoff = 1;
        lastAlarmGesture = 0;
        Serial.println("Alarm disabled by pushing left button");
      }
    } else if (sigRight) {
      if (lastAlarmGesture == 0) {
        lastAlarmGesture = 2;
        alarmGestureStartMillis = currMilliSecond;
        Serial.println("RIGHT BUTTON");
      } else if (lastAlarmGesture == 1) {
        alarmoff = 1;
        lastAlarmGesture = 0;
        Serial.println("Alarm disabled by pushing right button");
      }
    }

    if(lastAlarmGesture != 0){
      if(alarmGestureStartMillis + 4000 < currMilliSecond){
        lastAlarmGesture = 0;
        Serial.println("BUTTON OFF");
      }
    }
  } 
}

void bluetoothAlarm(){
  //IF BLUETOOTH DATA == AO -> RECEIVE ALARM OPERATION DATA
  //IF BLUETOOTH DATA == SO -> RECEIVE ASMR OPERATION DATA
  //IF BLUETOOTH DATA == AR -> SEND ALARM DATA
  //IF BLUETOOTH DATA == AS -> RECEIVE ALARM DATA
  
  String mobileInput = Serial1.readString();
  String mobileCommand = mobileInput.substring(0, 2);
  Serial.println(mobileInput);
  Serial.println(mobileCommand);

  if (mobileCommand == "AO") {
    currentAlarm.alarmOperation = mobileInput.substring(2, 3).toInt();
    EEPROM.write(ALARM_OPERATION_ADDRESS, currentAlarm.alarmOperation);
    if (!currentAlarm.alarmOperation) {
      if (alarm) {
        disableAlarm();
      }
    }
  }
  if (mobileCommand == "VO") {
    currentAlarm.vibrateOperation = mobileInput.substring(2, 3).toInt();
    EEPROM.write(VIBRATE_OPERATION_ADDRESS, currentAlarm.vibrateOperation);
  }
  if (mobileCommand == "SO") {
    currentAlarm.asmrOperation = mobileInput.substring(2, 3).toInt();
    EEPROM.write(ASMR_OPERATION_ADDRESS, currentAlarm.asmrOperation);
    if (currentAlarm.asmrOperation) {
      Serial1.println("true");
    } else {
      Serial1.println("false");
    }
  }
  if (mobileCommand == "AR") {
    Serial1.print("[AlarmTime]");
    Serial1.print(currentAlarm.hour);
    Serial1.print(":");
    Serial1.print(currentAlarm.minute);
    Serial1.print("$");
    if (currentAlarm.alarmOperation) {
      Serial1.print("true");
    } else {
      Serial1.print("false");
    }
    Serial1.print("$");
    if (currentAlarm.vibrateOperation) {
      Serial1.print("true");
    } else {
      Serial1.print("false");
    }
    Serial1.print("$");
    if (currentAlarm.asmrOperation) {
      Serial1.print("true");
    } else {
      Serial1.print("false");
    }
    Serial1.print("$");
    Serial1.println(currentAlarm.alarmTrack);
  }
  if (mobileCommand == "AS") {
    int index1 = mobileInput.indexOf(':');
    int index2 = mobileInput.length();
    
    currentAlarm.hour = mobileInput.substring(2, index1).toInt();
    currentAlarm.minute = mobileInput.substring(index1+1, index2).toInt();

    EEPROM.write(ALARM_HOUR_ADDRESS, currentAlarm.hour);
    EEPROM.write(ALARM_MINUTE_ADDRESS, currentAlarm.minute);
  }
  if (mobileCommand == "AT") {
    currentAlarm.alarmTrack = mobileInput.substring(2, 3).toInt();
    EEPROM.write(ALARM_TRACK_ADDRESS, currentAlarm.alarmTrack);
  }
}
