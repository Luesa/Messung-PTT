using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Ports;
using System.Windows.Forms;


namespace SD_Karte_lesen
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            //Komponente aktivieren
            InitializeComponent();
        }

        /// <summary>
        /// Lädt das Fenster/die GUI, sowie die verfügbaren Ports und lädt die Grundeinstellung für die Tasten
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Form1_Load(object sender, EventArgs e)
        {
            string[] ports = SerialPort.GetPortNames();         //Alle verfügbaren Ports in einem string speichern
            textBox1.Enabled = false;                           //Anweisungsbox ist nicht aktiv
            PortBox.Items.AddRange(ports);                      //Portnames werden in die Liste eingefügt
        }

        /// <summary>
        /// Sobald der Benutzer auf Verbinden klickt, werden die Dateien von dem Gerät gelesen und in einer Datei gespeichert
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OpenButton_Click(object sender, EventArgs e)
        {
            Output.Clear();                                                         //Ausgabefeld leeren
            if (PortBox.Text == "")                                                 //Sollte nichts ausgewählt sein,
            {                                                                       //gibt es eine Fehlermeldung
                Output.Text = "Bitte COM Port auswählen!\n";
            }
            else
            {
                OpenButton.Enabled = false;                                         //Verbindungs Feld deaktivieren
                PortBox.Enabled = false;                                            //Auswahlfeld deaktivieren
                
                serialPort1.PortName = PortBox.Text;                                //Name des Seriellen Ports festlegen
                serialPort1.ReadTimeout = 3000;                                     //Timeout für Lesen. Wenn nach 5s der Terminator nicht erreicht wurde, wird ein Fehler ausgegeben         
                serialPort1.BaudRate = 1500000;                                      //Baudrate Einstellungen. Für Teensy unerheblich, da dieser immer mit maximaler USB Geschwindigkeit überträgt
                serialPort1.Open();                                                 //Öffnen des COM Ports
                Output.Text = PortBox.Text + " wurde geöffnet.\n\n";                //Benutzerausgabe
                serialPort1.WriteLine("a\n");                                       //Befehl an Gerät senden
                
                //Anzahl der Binärdateien ermitteln
                Variables.Anzahl = short.Parse(serialPort1.ReadLine());             //Anzahl der Binärdateien einlesen
                int i = 0;                                                          //Zählindex für Schleifen
                serialPort1.ReadLine();                                             //Bei Beginn einer Datei wird 'neu' gesendet. Bei der ersten Datei wird dies nicht benötigt
                                                                                                    
                for(i=0;i<8;i++)                                                    //die Zeichen für das Datum werden einzeln eingelesen, da Arduino sonst Fehler einbaut
                {
                    Variables.date[i] = (char)serialPort1.ReadChar();               //Die Zeichen werden als Char eingelesen
                    Variables.datum = Variables.datum + Variables.date[i].ToString();   //und zu einem String zusammengefügt
                }
                //serialPort1.ReadLine();                                             //Zusätzliches \r einlesen
                //serialPort1.ReadLine();
                int filenumber = 1;                                                 //aktuelle Dateinummer festlegen
                string Zeile = "";                                                  //String für das Einlesen der Zeilen anlegen
                //serialPort1.ReadLine();
                //Ab hier werden die gespeicherten Dateien ausgelesen
                while (Variables.Anzahl > 0)
                {
                    do {
                        Variables.filename = Variables.datum + "_" + filenumber + ".csv";   //Dateiname erstellen, im Format Jahr_Monat_Tag_Nummer
                        Variables.filename = Variables.filename.Replace("\r", "");          //\r löschen

                        Variables.foldername = @"c:\PTT-Messung\";                          //erstellen eines Pfades für das Anlegen einer Datei.
                        System.IO.Directory.CreateDirectory(Variables.foldername);          //Der Pfad wird angelegt
                        Variables.path = System.IO.Path.Combine(Variables.foldername, Variables.filename);//Die Datei wird zum Pfad hinzugefügt
                        filenumber++;                                                       //Erhöhen der Dateinummer
                    }while(System.IO.File.Exists(Variables.path));                          //Solange die Dateinummer erhöhen, bis ein Dateiname gefunden wurde, der noch nicht verwendet wurde
                                                                                            //Bsp. gleiches Datum, unterschiedlicher Übertragungsdurchlauf

                    //Instanz aktivieren, die das Schreiben in eine Datei möglich macht
                    using (System.IO.StreamWriter file = new System.IO.StreamWriter(Variables.path, true))                         //Pfad und Datei werden erstellt
                    {
                        Output.AppendText("Schreibe in Datei "); Output.AppendText(Variables.filename + "\r");  //Ausgabe

                        while (true)    //Endlosschleife
                        {
                            try
                            {
                                Zeile = serialPort1.ReadLine();     //aktuelle Zeile auslesen
                            }
                            catch                                   //Abfang Protokoll, sollte eine Ausnahme entstehen, z.B. konnte nichts eingelesen werden
                            {
                                Zeile = "";                         //Für zusätzliche Abfragen, ob das Auslesen gklappt hat
                                break;                              //alle Dateien wurden ausgelesen und das Programm kann beendet werden
                            }

                            bool Patient = Zeile.Contains("neu");      //Frage, ob wir auf eine Zeile mit neu gestoßen sind, dies bedeutet, das die Datei zu einem neuen Patienten gehört
                            if (Patient == true)                       //Wenn ja, beginnt hier eine neue Teildatei
                            {
                                Variables.Anzahl--;                    //Anzahl der verfügbaren Dateien um 1 verkleinern
                                
                                //Datum auslesen
                                for (i=0;i<8;i++)
                                {
                                    Variables.date[i] = (char)serialPort1.ReadChar();
                                    Variables.datum2 = Variables.datum2 + Variables.date[i].ToString();
                                }
                                serialPort1.ReadLine(); // \r auslesen

                                if (Variables.datum == Variables.datum2)       //Die Datei wurde an demselben Datum aufgenommen, wie die vorige
                                {
                                    Variables.datum2 = "";
                                    break;
                                }
                                else
                                {
                                    Variables.datum = Variables.datum2;
                                    Variables.datum2 = "";
                                    filenumber=1;
                                    break;
                                }//Ende if(Zeile == Datum)
                            }//Ende if(Datum)

                            file.WriteLine(Zeile);      //ausgelesene Zeile in Datei schreiben. Achtung: Sollte die Datei schon Inhalte enthalten, werden die neuen einfach angehängt!!!

                        }//Ende Schleife zum Schreiben in Datei

                        //Abbruchbedingung
                        if(Zeile=="")
                        {
                            break;
                        }
                        
                    }
                }
                //Benutzerausgabe
                Output.AppendText("\nFertig\n");
                Output.AppendText("Sie finden Ihre Dateien im Verzeichnis:\n");
                Output.AppendText(Variables.foldername + "\n");
                serialPort1.Close();
                PortBox.Enabled = true;
            }
        }
        
        
        /// <summary>
        /// Schließt die Anwendung
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void CloseButton_Click(object sender, EventArgs e)
        {
            //Beenden des Programms
            if (serialPort1.IsOpen)
            {
                serialPort1.Close();
            }
                       
                Application.Exit();
            
        }
    }
}
