void Messung()
{
   
  int starten;
  static int r = 0;
  beendet=0;
  //Ausgabe
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("Aufnehmen?");
  tft.setTextSize(1);
  tft.println();
  tft.println("1x: Nein");
  tft.println();
  tft.println("2x: Ja");
  tft.println();
  tft.println("Keine Eingabe: zum Menue");
  t = 0;

  starten = millis();

  while ((millis() - starten) < 15000)
  {
    if (t == 1 && millis() - timet > 2000)
    {
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(0, 20);
      tft.println("Zurueck zum Menue...");
      delay(2000);
      break;
    }
    if ( (t >= 2 && millis() - timet > 2000))
    {
      binName[BASE_NAME_SIZE] = '0';
      binName[BASE_NAME_SIZE + 1] = '0';

      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(0,10);
      tft.println("Bitte stellen Sie sicher");
      tft.println("das alle Sensoren");
      tft.println("angeschlossen sind.");
      delay(10000);
      
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(0, 20);
      tft.println("Messung laeuft");
      tft.setCursor(0, 40);
      tft.println("2x: Beenden");
      delay(2000);
      t = 1;

      //Messung, bis der Benutzer abbricht
      while (t == 1 && beendet == 0)
      {
        //Einstellen von I2C für SPO2
        if (r == 0)
        {
          noInterrupts();
          pox.begin();
          pox.setOnBeatDetectedCallback(onBeatDetected);
          interrupts();
          r = 1;
        }

        logData();

      }
      break;
    }

  }
  t = 0;

}

