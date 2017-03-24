void dumptoSerial(int DataBin)
{
  block_t block;

  char Name[FILE_NAME_DIM];
  strcpy(Name, binName);
  char datefile[FILE_NAME_DIM];
  char date[8];
  char header[20];
  int i = 0;
  int dummy=DataBin;
  //Anfangen mit senden bei Datei 0
  Name[BASE_NAME_SIZE] = '0';
  Name[BASE_NAME_SIZE + 1] = '0';

  //Solange Dateien vorliegen...
  while (dummy > 0)
  {
    //sollte die Datei nicht existieren
    if (!sd.exists(Name))
    {
      tft.fillScreen(ST7735_BLACK);
      tft.setTextSize(1);
      tft.setCursor(0, 0);
      tft.println("Nichts zu übertragen");
      return;
    }
   
    //gerade offene Datei schließen
    if (binFile.isOpen())
    {
      binFile.close();
    }

    //Frage, ob die Datei zu einem neuen Patienten gehört -> ob es ein zugehöriges Datefile gibt
    strcpy(datefile, Name);
    strcpy(&datefile[BASE_NAME_SIZE + 3], "txt");

    if (sd.exists(datefile))
    {
      i=0;
      Serial2.println("neu");
      
      myFile = sd.open(datefile, FILE_READ);   //öffnen der Headerdatei
      while (i <= 7)                //Auslesen der ersten Zeile
      {
        date[i] = myFile.read();    //Speichern in einem Feld
        i++;
      }
      i = 0;
//      while (i <= 19)               //Auslesen der zweiten Zeile
//      {
//        header[i] = myFile.read();  //Speichern in einem Feld
//        i++;
//      }
      
      for(int u=0; u<8; u++)
      {
        Serial2.print(date[u]);
      }
      
      //Serial.print(header);
      myFile.close();
    }


    //aktuelle Datei öffnen
    binFile.open(Name);

    if (!binFile.isOpen())
    {
      tft.fillScreen(ST7735_BLACK);
      tft.setTextSize(1);
      tft.setCursor(0, 0);
      tft.println("Fehler bei Datei");
      return;
    }

    //an den Anfang zurückgehen
    binFile.rewind();

    delay(1000);

    //Daten senden bis die Datei zu Ende ist
    while (!Serial2.available() && binFile.read(&block , 512) == 512) {
      if (block.count == 0) {
        break;
      }
      if (block.overrun) {
        tft.print("OVERRUN,");
        tft.println(block.overrun);
      }
      for (uint16_t i = 0; i < block.count; i++) {
        //while(Serial.availableForWrite()<sizeof(block_t));
        printData(&Serial2, &block.data[i]);
      }
    }

    //nächste Datei suchen
    if (Name[BASE_NAME_SIZE + 1] != '9') //Frage ob die Einerstelle eine 9 ist
    {
      Name[BASE_NAME_SIZE + 1]++;         //Wenn nicht, wird die Einerstelle erhöht
    }
    else                                  //Sollte die Einerstelle eine 9 sein
    {
      Name[BASE_NAME_SIZE + 1] = '0';     //die Einerstelle wird zu 0
      Name[BASE_NAME_SIZE]++;             //die Zehnerstelle wird erhöht
    }

    //Anzahl an Dateien verkleinern
    dummy--;
  }

  tft.println("Fertig");
}
