// User data functions.  Modify these functions for your data items.

// Acquire a data record.
void acquireData(data_t* data) {
  static int i = 0;
  static int mill = 0;
  static int k=0;
  static int o=0;
  //Speichern der genauen Uhrzeit
  setTime(Teensy3Clock.get());
  data->Zeit[0] = hour();
  data->Zeit[1] = minute();
  data->Zeit[2] = second();

  if (i != second()) //millis() gibt die Zeit seit Programmstart an,
  {
    mill = 0;       //um die aktuellen Millisekunden anzugeben, müssen sie manuell "berechnet" werden.
    i = second();
  }
  data->Zeit[3] = mill;

  mill++;
  data->adc[0] = analogRead(18);
  data->adc[1] = analogRead(A11);
  pox.update();
  data->adc[2] = pox.getSpO2();

}


// Print a data record into a file
void printData(Print* pr, data_t* data) {
  //Zeit
  pr->print(data->Zeit[0]);
  pr->print(" ");

  if (data->Zeit[1] < 10)
    pr->print("0");
  pr->print(data->Zeit[1]);
  pr->print(" ");

  if (data->Zeit[2] < 10)
    pr->print("0");
  pr->print(data->Zeit[2]);
  pr->print(" ");

  if (data->Zeit[3] < 10)
    pr->print("00");
  else if (data->Zeit[3] < 100)
    pr->print("0");
  pr->print(data->Zeit[3]);

  //Sensordaten
  for (int i = 0; i < ADC_DIM; i++) {
    pr->write(" ");
    pr->print(data->adc[i]);
  }

  //nächste Zeile
  pr->println();
}

// Print data header.
void printHeader(Print* pr) {

  static int nummer = 0;

  nummer++;

  pr->print(year() % 100);
  pr->print("_");
  pr->print(month());
  pr->print("_");
  pr->println(day());


  pr->println(nummer);
  pr->print("time");
  pr->print(",");
  pr->print("Puls");
  pr->print(",");
  pr->print("EKG");
  pr->print(",");
  pr->println("O2");
}

