//------------------------ 방석센서 --------------------------------
const float THRESHOLD_COP_left    = -0.40;  // 좌 쏠림. X가 이 값보다 작아야 키이벤트 발생.
const float THRESHOLD_COP_right   = 0.40;   // 우 쏠림. X가 이 값보다 커야 키이벤트 발생.

const int   THRESHOLD_PRESS = 20;         // 압력 문턱값
const float THRESHOLD_MOVEMENT = 0.005;
//const float THRESHOLD_MOVEMENT = 0.0003  ;  // 움직임 문턱값

const int THRESHOLD_SUM_row_1st   = 60;   // 1st row, front row
const int THRESHOLD_SUM_row_3rd   = 60;   // 3rd row, back row
const int THRESHOLD_SUM_row_2nd   = 105;  // 2nd row, left and right
const int THRESHOLD_SUM_VERT      = 60;   // SUM? PRESSURE?

const int THRESHOLD_SUM_LEFT   = 40;
const int THRESHOLD_SUM_MIDDLE = 30;
const int THRESHOLD_SUM_RIGHT  = 40;

const int CONST_SEAT_CELL_NUM_TOTAL = 31; // Total number of sensor cells = 31.
const int CONST_SEAT_CELL_NUM_ROW_1ST = 10;
const int CONST_SEAT_CELL_NUM_ROW_2ND = 15;
const int CONST_SEAT_CELL_NUM_ROW_3RD = 6;

int adc_value[32]; // 센서 측정값 버퍼
bool is_key_pressed[256];  // 특정 키가 눌렸는지를 기록.
float pre_x = -2, pre_y = -2;

const int En0 = 31;  //  4 to 16 decoder 0, Low enabled
const int En1 = 33;  //  4 to 16 decoder 1, Low enabled
const int S0  = 35;
const int S1  = 37;
const int S2  = 39;
const int S3  = 41;
const int SIG_pin = A9; // common output of two decoder 

long pressStartMillis = 0;
long pressStartMillisNoMove = 0;
long noPressStartMillis = 0;
long sleepStartMillis = 0;
long lastMillis = 0;
bool is_pressed = false;
bool noPressStart = false;
//----------------------------------------------------------------


void CushionSetup(){
  pinMode(En0, OUTPUT);
  pinMode(En1, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
}

void Cushion(DataSet *d) {
  
  // Read 32 sensor cells.
  for(int i = 0 ; i < 32 ; i++){
    int readValue = readMux(i)-20;
    if (readValue >= 0) {
      adc_value[i] = readValue;
    } else {
      adc_value[i] = 0;
    }
  }
 
  /*
   *    1st row (10 cells) :                 0, 1, 2, 3, 4, 26, 27, 28, 29, 30
   *    2nd row (15 cells) :   5, 6, 7, 8, 9, 11, 13, 15(center), 17, 19, 21, 22, 23, 24, 25
   *    3rd row (6 cells):                       10, 12, 14, 16, 18, 20
   */
   
  //-----------------무게 중심의 Y 좌표 계산하기------------------
  int sum_row_1st =  adc_value[0]+adc_value[1]+adc_value[2]+adc_value[3]+adc_value[4]
                    +adc_value[26]+adc_value[27]+adc_value[28]+adc_value[29]+adc_value[30];

  int sum_row_2nd =  adc_value[5]+adc_value[6]+adc_value[7]+adc_value[8]+adc_value[9]
                    +adc_value[11]+adc_value[13]+adc_value[15]+adc_value[17]+adc_value[19]
                    +adc_value[21]+adc_value[22]+adc_value[23]+adc_value[24]+adc_value[25];

  int sum_row_3rd = adc_value[10]+adc_value[12]+adc_value[14]+adc_value[16]+adc_value[18]+adc_value[20];
 
  int sum_up = sum_row_1st + sum_row_2nd + sum_row_3rd;

  double avg_row_1st = (float)sum_row_1st / (float)CONST_SEAT_CELL_NUM_ROW_1ST;
  double avg_row_2nd = (float)sum_row_2nd / (float)CONST_SEAT_CELL_NUM_ROW_2ND;
  double avg_row_3rd = (float)sum_row_3rd / (float)CONST_SEAT_CELL_NUM_ROW_3RD;

  float cop_vertical = 0.0;
  if(50 < sum_up) {
    cop_vertical = ((3.0)*avg_row_1st + (2.0)*avg_row_2nd + (1.0)*avg_row_3rd) 
                   / (avg_row_1st + avg_row_2nd + avg_row_3rd) - 1.0;
  }

  //-----------------무게 중심의 X 좌표 계산하기------------------
  float sum_wp_horizon = ( (7)*adc_value[5]+(6)*adc_value[6]+(5)*adc_value[7]
                          +(4)*adc_value[8]+(3)*adc_value[9]+(2)*adc_value[11]
                          +(1)*adc_value[13]+(0)*adc_value[15]
                          +(-1)*adc_value[17]+(-2)*adc_value[19]+(-3)*adc_value[21]
                          +(-4)*adc_value[22]+(-5)*adc_value[23]+(-6)*adc_value[24]
                          +(-7)*adc_value[25] ) / 7.0 
                      // 2nd_row
                      + ( (5)*adc_value[0]+(4)*adc_value[1]+(3)*adc_value[2]
                          +(2)*adc_value[3]+(1)*adc_value[4]+(-1)*adc_value[26]
                          +(-2)*adc_value[27]+(-3)*adc_value[28]+(-4)*adc_value[29]
                          +(-5)*adc_value[30] ) / 5.0 
                      // 1st_row
                      + ( (3)*adc_value[10]+(2)*adc_value[12]+(1)*adc_value[14]
                          +(-1)*adc_value[16]+(-2)*adc_value[18]+(-3)*adc_value[20] ) / 3.0; 
                      // 3rd_row
                      
  float cop_horizon = 0.0;
  if(50 < sum_up) {
    cop_horizon = sum_wp_horizon / sum_up;
  }

  // X좌표나 Y좌표에 nan 값이 뜰 경우 다시 측정
  if(isnan(cop_horizon) || isnan(cop_vertical)) {
    return;
  }

  // 왼쪽, 가운데, 오른쪽 버튼화 후 알람 끄는 제스쳐로 사용
  int sum_left   =  adc_value[0]+adc_value[1]+adc_value[2]+adc_value[5]+adc_value[6]
                    +adc_value[7]+adc_value[8]+adc_value[9]+adc_value[10]+ adc_value[12];

  int sum_middle =  adc_value[3]+adc_value[4]+adc_value[26]+adc_value[27]+adc_value[11]
                    +adc_value[13]+adc_value[15]+adc_value[17]+adc_value[19]+adc_value[14]
                    +adc_value[16];

  int sum_right  =  adc_value[28]+adc_value[29]+adc_value[30]+adc_value[21]+adc_value[22]
                    +adc_value[23]+adc_value[24]+adc_value[25]+adc_value[18]+adc_value[20];

  bool sig_left = 0, sig_middle = 0, sig_right = 0;
  if (sum_left > THRESHOLD_SUM_LEFT)     sig_left = 1;
  if (sum_middle > THRESHOLD_SUM_MIDDLE) sig_middle = 1;
  if (sum_right > THRESHOLD_SUM_RIGHT)   sig_right = 1;
  alarmGesture(sig_left, sig_right);
  
  // 수면 신호가 0이고 압력이 발생했을때 정해진 시간동안 큰 움직임이 없으면
  // 잠에 든 것으로 간주하고 수면 신호를 1로 바꾼다.
  float timePassed = 0;
  if(!sleeping){
    if(sum_up > THRESHOLD_PRESS){
      if(!is_pressed){
        pressStartMillis = currMilliSecond;
        pressStartMillisNoMove = currMilliSecond;
        lastMillis = currMilliSecond;
        noPressStart = 0;
        pre_x = cop_horizon;
        pre_y = cop_vertical;
      }
      is_pressed = true;
    } else {
      if (!noPressStart) {
        noPressStartMillis = currMilliSecond;
        noPressStart = 1;
      }
      is_pressed = false;
    }
  
    if(is_pressed && lastMillis != currMilliSecond){
      float deltaTime = currMilliSecond - lastMillis;
      lastMillis = currMilliSecond;
      float dist = distance(cop_horizon, cop_vertical, pre_x, pre_y);
  
      float deltaTimeDist = dist / deltaTime;
      if(deltaTimeDist > THRESHOLD_MOVEMENT){
        pressStartMillis = currMilliSecond;
        Serial.println("MOVED TOO FAR");
      }
      timePassed = currMilliSecond - pressStartMillis;
      if(timePassed > TIME_SLEEP){
        sleeping = true;
        sleepStartMillis = currMilliSecond;
      }
    }
  } else {
    pressStartMillis = currMilliSecond;
  }

  // ASMR 재생
  if (currentAlarm.asmrOperation) {
    if (is_pressed) {
      asmrSound(pressStartMillisNoMove, sleepStartMillis, is_pressed);
    } else {
      // 압력이 없어졌을때
      asmrSound(noPressStartMillis, sleepStartMillis, is_pressed);
    }
  }

  pre_x = cop_horizon;
  pre_y = cop_vertical;
  
  d->posX = cop_horizon;
  d->posY = cop_vertical;
  d->sleeping = sleeping;

  /*
  if (sendingSerial) {
    Serial.println("---------------------------------");
    Serial.print("[sum up]");
    Serial.print(sum_up); 
    Serial.print("       [pressed]");
    Serial.println(is_pressed);
    Serial.print("[sum left]");
    Serial.print(sum_left);
    Serial.print("     [sig left]");
    Serial.println(sig_left);
    Serial.print("[sum right]");
    Serial.print(sum_right);
    Serial.print("    [sig right]");
    Serial.println(sig_right); 
    
    Serial1.println("---------------------------------");
    Serial1.print("[sum up]");
    Serial1.print(sum_up); 
    Serial1.print("       [pressed]");
    Serial1.println(is_pressed);
    Serial1.print("[sum left]");
    Serial1.print(sum_left);
    Serial1.print("     [sig left]");
    Serial1.println(sig_left);
    Serial1.print("[sum right]");
    Serial1.print(sum_right);
    Serial1.print("    [sig right]");
    Serial1.println(sig_right); 
    sendingSerial = 0;
  }
  */
}


int readMux(int channel){
  int controlPin[] = {S0,S1,S2,S3,En0,En1};
 
  int muxChannel[32][6]={
    {0,0,0,0,0,1}, //channel 0
    {0,0,0,1,0,1}, //channel 1
    {0,0,1,0,0,1}, //channel 2
    {0,0,1,1,0,1}, //channel 3
    {0,1,0,0,0,1}, //channel 4
    {0,1,0,1,0,1}, //channel 5
    {0,1,1,0,0,1}, //channel 6
    {0,1,1,1,0,1}, //channel 7
    {1,0,0,0,0,1}, //channel 8
    {1,0,0,1,0,1}, //channel 9
    {1,0,1,0,0,1}, //channel 10
    {1,0,1,1,0,1}, //channel 11
    {1,1,0,0,0,1}, //channel 12
    {1,1,0,1,0,1}, //channel 13
    {1,1,1,0,0,1}, //channel 14
    {1,1,1,1,0,1}, //channel 15
    {0,0,0,0,1,0}, //channel 16
    {0,0,0,1,1,0}, //channel 17
    {0,0,1,0,1,0}, //channel 18
    {0,0,1,1,1,0}, //channel 19
    {0,1,0,0,1,0}, //channel 20
    {0,1,0,1,1,0}, //channel 21
    {0,1,1,0,1,0}, //channel 22
    {0,1,1,1,1,0}, //channel 23
    {1,0,0,0,1,0}, //channel 24
    {1,0,0,1,1,0}, //channel 25
    {1,0,1,0,1,0}, //channel 26
    {1,0,1,1,1,0}, //channel 27
    {1,1,0,0,1,0}, //channel 28
    {1,1,0,1,1,0}, //channel 29
    {1,1,1,0,1,0}, //channel 30
    {1,1,1,1,1,0}  //channel 31
  };
 
  //loop through the 6 sig (muxChannel has 6 values)
  for(int i = 0; i < 6; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }
 
  //read the value at the SIG pin
  int val = analogRead(SIG_pin);
 
  return val;
}

float distance(float x1, float y1, float x2, float y2) {
  return sqrt(pow((x1-x2), 2) + pow((y1-y2), 2));
}
 
void Print_XY(float x, float y, bool sleeping) {
  Serial.print("x=");
  Serial.print(x);
  Serial.print(", y=");
  Serial.print(y);  
  Serial.print(", sleeping:");
  Serial.println(sleeping);
}
