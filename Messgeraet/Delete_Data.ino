void loeschen ()
{
  int starten;
  char dummy[2];
  char datefile[FILE_NAME_DIM];
  
  //Benutzerausgabe
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.println("Loeschen?");
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
      //Suche bereits vorhandene .bin Dateien
      //bei Programmstart ist binName=data00.bin
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

      //Letzte existierende Dateinummer nach binName zurückkopieren
      binName[BASE_NAME_SIZE] = dummy[0];
      binName[BASE_NAME_SIZE + 1] = dummy[1];

      if (DataBin != 0)
      {
        tft.fillScreen(ST7735_BLACK);
        tft.setCursor(0, 20);
        tft.println("Daten werden geloescht");
        delay(2000);
        while (sd.exists(binName)) //Wenn die Datei existiert...
        {
          sd.remove(binName);       //Löschen der Datei

          strcpy(datefile, binName);
          strcpy(&datefile[BASE_NAME_SIZE + 3], "txt");
          if (sd.exists(datefile))
          {
            sd.remove(datefile);
          }

          if (binName[BASE_NAME_SIZE + 1] != '0')   //Frage ob die Einerstelle größer 0
          {
            binName[BASE_NAME_SIZE + 1]--;          //Wenn ja, 1 subtrahieren
          }
          else
          {
            if (binName[BASE_NAME_SIZE] == 0)       //Frage, ob die Zehnerstelle auch 0 ist
            {
              break;                                //Wenn ja, dann ist die letzte Datei (00) gefunden worden, es kann mit den csv Dateien weitergehen -> Verlassen der Zählschleife
            }
            binName[BASE_NAME_SIZE + 1] = '9';      //Wenn die Zehnerstelle nicht 0 ist, wird die Einerstelle zu 9
            binName[BASE_NAME_SIZE]--;              //die Zehnerstelle wird um 1 subtrahiert
          }
        }
        delay(2000);
        tft.println();
        tft.println("Loeschen beendet");
        delay(2000);

      }
      else if (DataBin == 0)
      {
        tft.fillScreen(ST7735_BLACK);
        tft.setCursor(0, 20);
        tft.println("Keine Dateien vorhanden");
        delay(2000);
      }
      break;
    }
  }
  t = 0;
}


