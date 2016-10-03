using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.IO;
namespace ServoMotor
{
    public class Logging
    {
        public void Log(string error_message)
        {
            using (StreamWriter w = File.AppendText("log.txt"))
            {

                String trh = DateTime.Now.ToShortDateString().Replace('/', '.');
                String zmn = DateTime.Now.ToLongTimeString().Replace(':', '.');
                w.WriteLine(trh + " " + zmn + "     " + error_message);
            }
        }
    }
}
