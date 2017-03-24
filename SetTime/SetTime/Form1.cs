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

namespace SetTime
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }


        private void setTime_Click(object sender, EventArgs e)
        {
            Exit.Enabled = false;
            textBox.Clear();                                            //Ausgabefeld leeren
            if (comboBox.Text == "")                                    //Sollte nichts ausgewählt sein,
            {                                                           //gibt es eine Fehlermeldung
                textBox.Text = "Bitte COM Port auswählen!\n";
            }
            else
            {
                serialPort1.PortName = comboBox.Text;                   //Name des Seriellen Ports festlegen
                serialPort1.ReadTimeout = 3000;
                serialPort1.BaudRate = 9600;
                serialPort1.Open();                                     //Öffnen des COM Ports
                textBox.Text = comboBox.Text + " wurde geöffnet.\n\n";
                
                DateTime jetzt = DateTime.Now;                          //aktuelles Datum

                serialPort1.WriteLine(DateTime.Now.Year.ToString());    //Datum und Uhrzeit senden
                serialPort1.WriteLine(DateTime.Now.Month.ToString());
                serialPort1.WriteLine(DateTime.Now.Day.ToString());
                serialPort1.WriteLine(DateTime.Now.Hour.ToString());
                serialPort1.WriteLine(DateTime.Now.Minute.ToString());
                serialPort1.WriteLine(DateTime.Now.Second.ToString());

                textBox.AppendText("Zeit wurde eingestellt!");          //Benachrrichtigung
                                
            }

            Exit.Enabled = true;                        
        }

        private void Form1_Load_1(object sender, EventArgs e)
        {
            textBox1.Enabled = false;
            string[] ports = SerialPort.GetPortNames();         //Alle verfügbaren Ports in einem string speichern
            comboBox.Items.AddRange(ports);
        }

        private void Exit_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen)
            {
                serialPort1.Close();
            }
            Application.Exit();
        }
    }
}
