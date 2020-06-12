//-------------------------소리감지센서------------------------------
unsigned int samplingPeriod;
unsigned long microSeconds;
int microphoneAnalogPin = 0;
double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values
//---------------------------------------------------------------- 

void MicrophoneSetup(){
  samplingPeriod = round(1000000*(1.0/SAMPLING_FREQUENCY)); //Period in microseconds   
}

void Microphone(DataSet *d) {
  unsigned int peakToPeak = 0;
  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  double decibel = 0;
  
  /*Sample SAMPLES times*/
  for(int i=0; i<SAMPLES; i++)
  {
      microSeconds = micros();    //Returns the number of microseconds since the Arduino board began running the current script. 
   
      vReal[i] = analogRead(microphoneAnalogPin); //Reads the value from analog pin 0 (A0), quantize it and save it as a real term.
      vImag[i] = 0; //Makes imaginary term 0 always
      
      if (vReal[i] > signalMax) {
        signalMax = vReal[i];
      }
      else if (vReal[i] < signalMin) {
        signalMin = vReal[i];
      }
 
      /*remaining wait time between samples if necessary*/
      while(micros() < (microSeconds + samplingPeriod))
      {
        //do nothing
      }
  }

  /*Perform FFT on samples*/
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  /*Find peak frequency and print peak*/
  float peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  //Serial.println(peak);     //Print out the most dominant frequency.

  peakToPeak = signalMax - signalMin;
  decibel = 20*log10(peakToPeak/0.002);

  d->decibel = decibel;
  d->frequency = peak;
  
}
