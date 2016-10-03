using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Linq;
using System.IO;
using System.IO.Ports;
using Microsoft.Office.Core;
using Excel = Microsoft.Office.Interop.Excel;

namespace ServoMotor
{
    public partial class Form1 : Form
    {
        Logging log = new Logging();
        string user;
        Boolean user1,user2;
        string[] user_tablo;
        int i;
        byte[] akim = new byte[12] { 0x04, 0x82, 0x80, 0x0C, 0x14, 0x03, 0xB0, 0x01, 0x03, 0x010, 0x03, 0xBC };
        byte[] gerilim = new byte[6] { 0x1, 0x2, 0x3, 0x04, 0x5, 0x1 };
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            CheckForIllegalCrossThreadCalls = false;
            DateTime datetime = DateTime.Now;

         

            string[] portnames = System.IO.Ports.SerialPort.GetPortNames();
            foreach (string port in portnames)
            {
             this.com1.Items.Add(port.ToString());
            }
           
        }

        private void p_Click(object sender, EventArgs e)
        {
            try
            {
                
                if (!serialPort1.IsOpen)
                {
                    serialPort1.PortName = com1.SelectedItem.ToString();
                    serialPort1.Open();
                    label3.Text = "online";
                }
                else
                {
                    serialPort1.Close();
                    serialPort1.PortName = com1.SelectedItem.ToString();
                    serialPort1.Open();
                    label3.Text = "online";

                }

            }
            catch (Exception ex)
            {
                label3.Text = "offline";
                MessageBox.Show(ex.Message);

            }
        }
        byte[] data;
        int checksum;
        int sonuc;
        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            data = new byte[serialPort1.BytesToRead];
            sonuc = serialPort1.Read(data, 0, serialPort1.BytesToRead);
            total.Text = sonuc.ToString();
            this.Invoke(new EventHandler(user_disp));
        }
        private void user_disp(object sender, EventArgs e)
        {
            try
            {
                for (int i = 0; i < sonuc-1; i++)
                {
                    checksum = checksum ^ data[i];
                }
                label5.Text = data[data.Length - 1].ToString();
                label6.Text = checksum.ToString();
                if (data[data.Length - 1] == checksum)
                    label7.Text = "Data Duzgun Geldi";

               // else serialPort1.Write(gerilim, 0, gerilim.Length);   //checksum değeri yanlış ise veri tekrar istenecek

                textBox1.Text = BitConverter.ToString(data);
                checksum = 0;
                
            }
            catch (Exception ex)
            {
             
                log.Log(ex.Message);
            }
            

        }

       

    

        

        private void button1_Click(object sender, EventArgs e)
        {
            serialPort1.Write(gerilim, 0, gerilim.Length);
        }

        private void button2_Click(object sender, EventArgs e)
        {

            serialPort1.Write(gerilim, 0, gerilim.Length); 
        }

     
    
        
        }

  
    
}
