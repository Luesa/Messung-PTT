// log data
// max number of blocks to erase per erase call
uint32_t const ERASE_SIZE = 262144L;

void logData() {
  uint32_t bgnBlock, endBlock;    //Start und End Adressen der Datei
  char datefile[FILE_NAME_DIM];   //Name der Headerdatei

  // Allocate extra buffer space.
  block_t block[BUFFER_BLOCK_COUNT];
  block_t* curBlock = 0;
  
  // Find unused file name.
  if (BASE_NAME_SIZE > 6) {
    tft.println("FILE_BASE_NAME too long");
  }

  //zu Beginn ist binName =data00.bin
  while (sd.exists(binName)) {
    if (binName[BASE_NAME_SIZE + 1] != '9') { //Frage ob zB. data09.bin
      binName[BASE_NAME_SIZE + 1]++;
    } else {
      binName[BASE_NAME_SIZE + 1] = '0';
      if (binName[BASE_NAME_SIZE] == '9') {
        tft.println("Can't create file name");
      }
      binName[BASE_NAME_SIZE]++;
    }
  }

  if (binFile.isOpen())
  {
    binFile.close();    //die aktuelle .bin Datei wird geschlossen, da ihre Nummer möglicherweise nicht mit der aktuellen nummer in name übereinstimmt
  }
  
  if (Dateien == 0)     //Wenn die erste Datei aufgenommen wird, muss eine Headerdatei geschrieben werden
  {
    strcpy(datefile, binName);  //kopiere binName nach datefile
    strcpy(&datefile[BASE_NAME_SIZE + 3], "txt"); //ersetze .bin durch .txt
    myFile = sd.open(datefile, FILE_WRITE);   //öffnen der Datei
    myFile.print(year()%100);         //Datum eintragen
    myFile.print("_");
    if(month()<10)
    myFile.print(0);
    myFile.print(month());
    myFile.print("_");
    if(day()<10)
    myFile.print(0);
    myFile.println(day());
    //myFile.println("time,Puls,EKG,O2");   //Variablennamen eintragen
    myFile.close();   //Datei schließen
  }
  
  // Delete old tmp file.
  if (sd.exists(TMP_FILE_NAME)) {
   // tft.println("Loeschen von tmp Datei");
    if (!sd.remove(TMP_FILE_NAME)) {
      tft.println("Can't remove tmp file");
    }
  }

  // Create new file.
  //tft.println("Erstelle neue Datei");
  //binFile.close();
  if (!binFile.createContiguous(sd.vwd(), TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT)) {
    tft.println("createContiguous failed");
  }

  // Get the address of the file on the SD.
  if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
    tft.println("contiguousRange failed");
  }

  // Use SdFat's internal buffer.
  uint8_t* cache = (uint8_t*)sd.vol()->cacheClear();
  if (cache == 0) {
    tft.println("cacheClear failed");
  }

  // Flash erase all data in the file.
  //tft.println("Bereinigen aktueller Datei");
  uint32_t bgnErase = bgnBlock;
  uint32_t endErase;
  while (bgnErase < endBlock) {
    endErase = bgnErase + ERASE_SIZE;
    if (endErase > endBlock) {
      endErase = endBlock;
    }
    if (!sd.card()->erase(bgnErase, endErase)) {
      tft.println("erase failed");
    }
    bgnErase = endErase + 1;
  }

  // Start a multiple block write.
  if (!sd.card()->writeStart(bgnBlock, FILE_BLOCK_COUNT)) {
    tft.println("writeBegin failed");
  }

  // Initialize queues.
  emptyHead = emptyTail = 0;
  fullHead = fullTail = 0;

  // Use SdFat buffer for one block.
  emptyQueue[emptyHead] = (block_t*)cache;
  emptyHead = queueNext(emptyHead);

  // Put rest of buffers in the empty queue.
  for (uint8_t i = 0; i < BUFFER_BLOCK_COUNT; i++) {
    emptyQueue[emptyHead] = &block[i];
    emptyHead = queueNext(emptyHead);
  }

  //delay(2000);

  // Wait for Serial Idle.
  //Serial.flush();
  delay(10);
  bool closeFile = false;
  uint32_t bn = 0;
  uint32_t t0 = millis();
  uint32_t t1 = t0;
  uint32_t overrun = 0;
  uint32_t overrunTotal = 0;
  uint32_t count = 0;
  uint32_t maxDelta = 0;
  uint32_t minDelta = 99999;
  uint32_t maxLatency = 0;
  uint32_t logTime = micros();

  // Set time for first record of file.
  startMicros = logTime + LOG_INTERVAL_USEC;
  int kl=0;
//  Serial.begin(115200);
//  while(!Serial);
  while (1) {

    // Time for next data record.
    logTime += LOG_INTERVAL_USEC;

    //Abbruchbedingung vom Benutzer
    if (t >= 3 && updated <= 3000) {
      closeFile = true;
      beendet = 1;
      t = 3;
    }

    if (closeFile) {
      if (curBlock != 0) {
        // Put buffer in full queue.
        fullQueue[fullHead] = curBlock;
        fullHead = queueNext(fullHead);
        curBlock = 0;
      }
    } else {
      if (curBlock == 0 && emptyTail != emptyHead) {
        curBlock = emptyQueue[emptyTail];
        emptyTail = queueNext(emptyTail);
        curBlock->count = 0;
        curBlock->overrun = overrun;
        overrun = 0;
      }

      int32_t delta;
      do {
        delta = micros() - logTime;
      } while (delta < 0);
      if (curBlock == 0) {
        overrun++;
      } else {

        //Sensoren auslesen
        acquireData(&curBlock->data[curBlock->count++]);

        if (curBlock->count == DATA_DIM) {
          fullQueue[fullHead] = curBlock;
          fullHead = queueNext(fullHead);
          curBlock = 0;
        }
        if ((uint32_t)delta > maxDelta) maxDelta = delta;
        if ((uint32_t)delta < minDelta) minDelta = delta;
      }
    }

    if (fullHead == fullTail) {
      // Exit loop if done.
      if (closeFile) {
        t = 1;
        break;
      }
    } else if (!sd.card()->isBusy()) {
      // Get address of block to write.
      block_t* pBlock = fullQueue[fullTail];
      fullTail = queueNext(fullTail);
      // Write block to SD.
      uint32_t usec = micros();
      
      if (!sd.card()->writeData((uint8_t*)pBlock)) {
        tft.println("write data failed");
      }
      
      usec = micros() - usec;
      t1 = millis();
      if (usec > maxLatency) {
        maxLatency = usec;
      }
      count += pBlock->count;

      // Move block to empty queue.
      emptyQueue[emptyHead] = pBlock;
      emptyHead = queueNext(emptyHead);
      bn++;

      //Datei ist voll
      if (bn == FILE_BLOCK_COUNT) {
        // File full so stop
        beendet = 0;
        t = 1;
        break;
      }
    }
  }
  delay(5);
  if (!sd.card()->writeStop()) {
    tft.println("writeStop failed");
  }

  //Datei verkürzen, wenn abgebrochen wurde -> nicht verwendeten Speicher freigeben
  if (bn != FILE_BLOCK_COUNT) {
    //tft.println("Datei verkuerzen");
    if (!binFile.truncate(512L * bn)) {
      tft.println("Can't truncate file");
    }
  }
  if (!binFile.rename(sd.vwd(), binName)) {
    tft.println("Can't rename file");
  }

  //Benutzerausgaben:
//  tft.fillScreen(ST7735_BLACK);
//  tft.setCursor(0,0);
//  tft.setTextSize(1);
//  tft.print("Aufgenommene Zeit: ");
//  tft.println(0.001 * (t1 - t0), 3);
//  tft.print("Sample Rate ");
//  tft.println(count);
//  tft.print("Samples/sec: ");
//  tft.println((1000.0)*count / (t1 - t0));
//  delay(5000);
 

  Dateien++;
  DataBin++;
  if (binFile.isOpen())
    {
      binFile.close();
    }
  if (beendet == 1)
  { 
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0,40);
    tft.println("Messung wurde beendet.");
//    tft.print("Es wurden ");
//    tft.print(Dateien);
//    tft.println(" aufgenommen.");
    delay(3000);
  }

}
