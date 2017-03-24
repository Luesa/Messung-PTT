using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SD_Karte_lesen
{
    class Variables
    {
        public static string datum="";            //Datum aus Datei
        public static char[] date = new char[8];
        public static string datum2 = "";
        public static short nummer=0;               //Nummer der Teildatei
        public static string filename = "";         //Dateiname
        public static short Anzahl=0;               //Anzahl der zu übertragenden Dateien
        public static string path = "";
        public static string foldername = "";  
    }
}
