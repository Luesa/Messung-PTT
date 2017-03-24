#ifndef UserDataType_h
#define UserDataType_h

//Anzahl der Sensoren
const uint8_t ADC_DIM = 3;

struct data_t {
  //Zeitfeld
  unsigned short Zeit[4];
  //Sensorenfeld
  unsigned short adc[ADC_DIM];
};

#endif // UserDataType_h
