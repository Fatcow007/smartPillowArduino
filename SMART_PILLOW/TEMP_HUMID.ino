//-------------------------온/습도센서------------------------------
float humidity, temperature;
//----------------------------------------------------------------

void TempHumidSetup() {
  dht.begin();
}

void TempHumid(DataSet *d) {

  humidity = dht.readHumidity();// 습도를 측정합니다.
  temperature = dht.readTemperature();// 온도를 측정합니다.
  
  // 값 읽기에 오류가 있으면 오류를 출력합니다.
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
   
  // 체감온도
  // float hic = dht.computeHeatIndex(temperature, humidity, false);
  
  d->humidity = humidity;
  d->temperature = temperature;
  
}

// 온도와 습도에 어울리는 ASMR 트랙 선택
int ASMRTrack() {
  int soundTrack;

  if (humidity > 70) {
    if (temperature > 26) {
      soundTrack = 11; // 파도 소리
    } else if (temperature > 22) {
      soundTrack = 15; // 귀뚜라미 소리
    } else {
      soundTrack = 10; // 캠프파이어
    }
  } else if (humidity > 60) {
    if (temperature > 23) {
      soundTrack = 14; // 계곡 소리
    } else {
      soundTrack = 13; // 바람 소리
    }
  } else {
    soundTrack = 12; // 빗소리
  }
  Serial.print("[humidity]");
  Serial.print(humidity);
  Serial.print("[temperature]");
  Serial.println(temperature);
  Serial.print("[SoundTrack]");
  Serial.println(soundTrack);

  return soundTrack;
}
