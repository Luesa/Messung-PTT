void sendData()
{
  Serial2.begin(1500000);
  int starten;
  char dummy[2];
  char datefile[FILE_NAME_DIM];   //Name der Headerdatei

  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("Daten senden?");
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
    if ((t >= 2 && millis() - timet > 2000))
    {
      while (sd.exists(binName))
      {
        DataBin++; //Anzahl der gefundenen .bin Dateien

        //Speichern der letzen existierenden Datei Nummer
        dummy[0] = binName[BASE_NAME_SIZE];       //Zehnerstelle
        dummy[1] = binName[BASE_NAME_SIZE + 1];   //Einerstelle

        //Nummer der .bin Datei hochzählen, um die nächste zu überprüfen
        if (binName[BASE_NAME_SIZE + 1] != '9')   //Frage ob die Einerstell 9 ist
        {
          binName[BASE_NAME_SIZE + 1]++;          //Wenn nicht, dann die Einerstelle erhöhen
        }
        else                                      //Wenn ja, dann wird die Zehnerstelle überprüft
        {
          if (binName[BASE_NAME_SIZE] == '9')     //überprüfen, ob auch die Zehnerstelle eine 9 ist
          {
            break;                                //Wenn ja, ist die größtmögliche Anzahl an Dateien gefunden worden. Die Zählschleife kann verlassen werden
          }
          binName[BASE_NAME_SIZE + 1] = '0';      //die Einerstelle wird auf 0 gesetzt
          binName[BASE_NAME_SIZE]++;              //die Zehnerstelle wird erhöht
        }
      }

      
      do
      {
       delay(10);
      }while(Serial2.available() && Serial2.read()>=0);
      
      tft.fillScreen(ST7735_BLACK);
      tft.setCursor(0, 20);
      tft.println("Um die Datenuebertragung");
      tft.setCursor(0, 30);
      tft.println("zu starten, bitte das");
      tft.setCursor(0, 40);
      tft.println("Geraet an den PC");
      tft.setCursor(0, 50);
      tft.println("anschliessen und das");
      tft.setCursor(0, 60);
      tft.println("Programm `GetData' ausfuehren.");
      tft.setCursor(0,80);
      tft.println("Um abzubrechen bitte");
      tft.setCursor(0,90);
      tft.println("2x den Knopf druecken.");
      t=0;

      while (!Serial2.available())
      {
        if((t>=2 && millis()-timet>2000))
        {
          tft.fillScreen(ST7735_BLACK);
          tft.setCursor(0,20);
          tft.println("Zuruek zum Menue...");
          delay(3000);
          t=0;
          Serial2.flush();
          Serial2.end();
          return;
        }
      }

      do
      {
        delay(10);
      } while (Serial2.available() && Serial2.read() >= 0);

      //Anzahl der verfügbaren BinDateien übermitteln
      Serial2.println(DataBin);
      tft.fillScreen(ST7735_BLACK);
      tft.setTextSize(1);
      tft.setCursor(0, 20);
      tft.println("Uebertrage Dateien");
      dumptoSerial(DataBin);      //Daten an Serial(MatLab) senden

      delay(2000);
      break;
    }
  }
  t = 0;
  Serial2.flush();
  Serial2.end();
}

