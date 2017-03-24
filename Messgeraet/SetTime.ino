void Time ()
{
  Serial2.begin(9600);
  int i = 0;
  int starten;
  Zeit tim;

  //Benutzerausgabe
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("Zeit neu");
  tft.println("einstellen?");
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
      tft.println("Zuruek zum Menue...");
      delay(2000);
      break;
    }
    if ((t >= 2 && millis() - timet > 2000))
    {
       do
      {
       delay(10);
      }while(Serial2.available() && Serial2.read()>=0);
      
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(0, 20);
      tft.println("Um die Zeit einzustellen");
      tft.setCursor(0, 30);
      tft.println("das Geraet mit dem PC");
      tft.setCursor(0, 40);
      tft.println("verbinden und das Programm");
      tft.setCursor(0, 50);
      tft.println("`SetTime' ausfuehren.");
      tft.setCursor(0, 70);
      tft.println("Um abzubrechen bitte");
      tft.setCursor(0, 80);
      tft.println("2x den Knopf druecken.");
      t = 0;

      //Warten, bis Serial eingeschaltet wird, oder der Benutzer abbricht
      while (!Serial2.available())
      {
        if ((t >= 2 && millis() - timet > 2000))
        {
          tft.fillScreen(ST7735_BLACK);
          tft.setCursor(0, 20);
          tft.println("Zuruek zum Menue...");
          delay(3000);
          t = 0;
          Serial2.flush();
          Serial2.end();
          return;
        }
      }

      //Es werden 6 Daten übertragen
      while (i < 7)
      {
        if (Serial2.available() > 0)
        {
          if (i == 0)
          {
            tim.Year = Serial2.parseInt(); //Daten über die Serielle Schnittstelle kommen als char. mit parseInt werden die eingelesenen Zeichen zu int Werten
          }
          if (i == 1)
          {
            tim.Month = Serial2.parseInt();
          }
          if (i == 2)
          {
            tim.Day = Serial2.parseInt();
          }
          if (i == 3)
          {
            tim.Hour = Serial2.parseInt();
          }
          if (i == 4)
          {
            tim.Minute = Serial2.parseInt();
          }
          if (i == 5)
          {
            tim.Second = Serial2.parseInt();
          }
          i++;
        }
      }
      //Die erhaltenen Variablen werden als Zeit auf dem Microcontroller gespeichert und ...
      setTime(tim.Hour, tim.Minute, tim.Second, tim.Day, tim.Month, tim.Year);
      //...anschließend auf der internen RTC gesichert
      Teensy3Clock.set(now());

      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(0, 20);
      tft.setTextSize(1);
      tft.println("Zeit wurde aktualisiert.");
      delay(3000);
      break;
    }

  }
  t = 0;
  Serial2.flush();
  Serial2.end();
}

