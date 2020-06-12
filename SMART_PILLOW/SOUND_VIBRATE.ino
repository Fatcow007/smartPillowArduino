//----------------------------------------------------------------
const int VIB_PIN1 = 52; // Vibration 1
const int VIB_PIN2 = 53; // Vibration 2
const int BUSY_PIN = 26; // mp3 module busy pin

const int alarmOffTrack = 21; // 20, 21

bool asmrStart  = 0;
bool asmrStopped = 0;
bool alarmStart = 0;
//----------------------------------------------------------------

void SoundVibrationSetup() {
  mp3_set_serial (Serial2);
  delay(1);
  mp3_set_volume (25);  // 1~30
  
  pinMode(VIB_PIN1, OUTPUT);
  pinMode(VIB_PIN2, OUTPUT);
}

/*
 * 알람이 켜질때, 켜져있을때, 꺼질때
 * 위의 세가지 경우에만 아래 함수를 호출한다.
 */
void alarmSoundVibrate(bool alarm) {
  // play_state : 음원이 재생중이지 않을 때 HIGH
  //              재생중일 때 LOW
  bool play_state = digitalRead(BUSY_PIN);
  if (alarm) {
    if (!alarmStart) {
      // 알람 시간이 되었는데 ASMR이 재생중일 경우 ASMR을 끄고 알람을 켜기
      if (play_state == LOW) { 
        mp3_stop();
        delay(10);
      }
      startVibration();
      alarmStart = 1;
      mp3_play(currentAlarm.alarmTrack);
    } else {
      // 알람 음원이 끝나면 다음 음원을 재생 (알람 음원 : 0~9번)
      // 해야해..? 그냥 알람 음원들 짧은거 저장해놓고
      // 그냥 일어날때 까지 반복하면 그게 알람이지
      // 알람 음원을 시리즈로 다 재생해야 하는건가? 이해할 수가 없네;
      if (play_state == HIGH) {
        mp3_play(currentAlarm.alarmTrack);
      }
    }
  } else {
    mp3_stop();
    stopVibration();
    alarmStart = 0;
    
    delay(10);
    mp3_play(alarmOffTrack);
  }  
}

// countStartMillis : 압력이 있을 때는 압력이 처음 주어진 시간 (pressStartMillisNoMove)
//                    압력이 없을 때는 압력이 처음 없어진 시간 (noPressStartMillis)
//
// mp3_stop()이 무한히 실행되는걸 막기 위해 asmrStopped 사용
void asmrSound(float countStartMillis, float sleepStartMillis, bool is_pressed) {
  if (!sleeping) {
    if (alarmOffMillis + 5000 < currMilliSecond) {
      if (is_pressed) {
        if (countStartMillis + TIME_ASMR_START < currMilliSecond) {
          if (!asmrStart) {
            mp3_play(ASMRTrack());
            asmrStart = 1;
            asmrStopped = 0;
          }
        }
      } else {
        if (countStartMillis + TIME_ASMR_STOP < currMilliSecond) {
          if (!asmrStopped) {
            mp3_stop();
            asmrStart = 0;
            asmrStopped = 1;
          }
        }
      }
    }
  } else {
    if (!alarm) {
      if(alarmOffMillis + 5000 < currMilliSecond) {
        if(sleepStartMillis + TIME_ASMR_SLEEP < currMilliSecond){
          if (!asmrStopped) {
            mp3_stop();
            asmrStart = 0;
            asmrStopped= 1;
          }
        }
      } 
    }
  }
}

void startVibration() {
  if (currentAlarm.vibrateOperation) {
    digitalWrite(VIB_PIN1, 1);
    digitalWrite(VIB_PIN2, 1);
  }
}
void stopVibration() {
  digitalWrite(VIB_PIN1, 0);
  digitalWrite(VIB_PIN2, 0);
}
