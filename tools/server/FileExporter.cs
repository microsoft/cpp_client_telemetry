using System;
using System.IO;
using Microsoft.Extensions.Logging;

namespace CommonSchema
{
    public class FileExporter
    {
        private readonly static object ExporterLock = new object();

        private readonly ILogger _logger;
        private bool needComma = false;
        public FileExporter(String fileName, ILogger logger = null)
        {
            _logger = logger;
            FileName = fileName;
        }

        public string FileName { get; set; }

        void LogInfo(string msg)
        {
            if (_logger!=null)
            {
                _logger.LogInformation(msg);
            }
        }
        public void ArrayResume()
        {
            while (!System.Threading.Monitor.TryEnter(ExporterLock));

            try
            {
                FileStream fs = File.Open(FileName, FileMode.Open);
                fs.Close();
                fs = new FileStream(FileName, FileMode.OpenOrCreate);
                // If file exists, then we need to append to it by
                // replacing last "]" with ",["
                fs.Seek(-2, SeekOrigin.End);
                fs.WriteByte((byte)',');
                fs.WriteByte((byte)'[');
                fs.Flush();
                fs.Close();
            } catch(System.IO.FileNotFoundException)
            {
                // If file does not exist, start it with '[' (JSON array)
                using StreamWriter sw = File.AppendText(FileName);
                sw.WriteLine("[xxx");
                sw.Flush();
                sw.Close();
            }
        }

        public void ArrayClose()
        {
            string path = FileName;
            using StreamWriter sw = File.AppendText(path);
            sw.WriteLine("]xxx");
            sw.Flush();
            sw.Close();
            // if (__lockWasTaken)
            System.Threading.Monitor.Exit(ExporterLock);
        }
        public void ArrayAdd(String text)
        {
            string path = FileName;
            using StreamWriter sw = File.AppendText(path);
            if (needComma)
            {
                sw.WriteLine(",");
            }

            if (text.StartsWith("["))
            {
                text.Remove(0, 1);
            }

            sw.WriteLine(text);
            sw.Flush();
            sw.Close();
            needComma = true;
        }

    }

}