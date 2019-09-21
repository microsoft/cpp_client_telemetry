using Bond.IO.Unsafe;

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

using Microsoft.AspNetCore.Http.Headers;

namespace CommonSchema
{
    using System;
    using System.Linq;
    using System.Text;
    using System.IO;
    using System.IO.Compression;
    using System.Security.Cryptography;
    using System.Collections.Generic;

    using CommonSchema;

    using Bond;
    using Bond.Protocols;
    using Bond.IO.Safe;
    using Microsoft.AspNetCore.Http;

    public class Decoder
    {

        private byte[] SavedBody;
        private string DecodedJson;

        public Decoder(IHeaderDictionary requestHeaders, byte[] requestBody)
        {
            body = requestBody;
            this.ClientId = requestHeaders["Client-Id"];
            this.ContentType = requestHeaders["Content-Type"];
            this.ContentEncoding = requestHeaders["Content-Encoding"];
        }

        /// <summary>
        /// Gets or sets the body.
        /// </summary>
        public byte[] body
        {
            get
            {
                return Encoding.UTF8.GetBytes(this.DecodedJson);
            }

            set
            {
                this.SavedBody = value;
            }
        }

        /// <summary>
        /// Gets or sets the "Client-Id" of the inspected content.
        /// </summary>
        public string ClientId { get; set; }

        /// <summary>
        /// Gets or sets the "Content-Type" of the inspected content.
        /// </summary>
        public string ContentType { get; set; }

        /// <summary>
        /// Gets or sets the "Content-Encoding" of the inspected content.
        /// </summary>
        public string ContentEncoding { get; set; }

        private static string IndentJson(string json, int TabSize = 2)
        {
            using (var stringReader = new StringReader(json))
            using (var stringWriter = new StringWriter())
            {
                var jsonReader = new JsonTextReader(stringReader);
                var jsonWriter = new JsonTextWriter(stringWriter) { Formatting = Formatting.Indented, IndentChar = ' ', Indentation = TabSize };
                jsonWriter.WriteToken(jsonReader);
                return stringWriter.ToString();
            }
        }

        public string IndentJson(int TabSize = 2)
        {
            DecodedJson = IndentJson(DecodedJson, TabSize);
            return DecodedJson;
        }

        public string ToJson(bool outputCompactJson = true, int TabSize = -1)
        {
            ToList(outputCompactJson);
            if (TabSize!=-1)
            {
                IndentJson(TabSize);
            }
            return DecodedJson;
        }

        /// <summary>
        /// Deserialize the byte data into a DataPackage, then to JSON format.
        /// </summary>
        /// <param name="data">The data bytes</param>
        /// <param name="outputCompactJson">Whether the output JSON should be compact or expanded/human-readble.</param>
        /// <returns>The JSON representation of the data bytes</returns>
        public List<string> ToList(bool outputCompactJson = true)
        {
            byte[] data = SavedBody;

            // Before deserializing the payload, check the content encoding first to see if it is
            // compressed.
            List<string> jsonList = new List<string>();

            if (!string.IsNullOrEmpty(this.ContentEncoding))
            {
                if (this.ContentEncoding == "gzip")
                {
                    data = Gunzip(data);
                }
                else if (this.ContentEncoding == "deflate")
                {
                    data = Deflate(data);
                }
                else
                {
                    throw new ArgumentException("Unknown Content-Encoding: " + this.ContentEncoding);
                }
            }

            // read using Common Schema protocol
            var outputBuffer = new MemoryStream();
            try
            {
                var input = new InputBuffer(data);
                var reader = new CompactBinaryReader<InputBuffer>(input);
                var writer = new SimpleJsonWriter(outputBuffer);
                do
                {
                    var evt = Deserialize<CsEvent>.From(reader);
                    // exception is thrown here if request is invalid
                    Serialize.To(writer, evt);
                    writer.Flush();
                    jsonList.Add(Encoding.UTF8.GetString(outputBuffer.ToArray()));
                    outputBuffer.SetLength(0);
                } while (true);
            }
            catch (Exception)
            {
                // end of input
            }

            // flatten all into one buffer representing JSON array of records
            outputBuffer.WriteByte((byte)'[');
            int i = 0;
            jsonList.ForEach((s) =>
            {
                if (i > 0)
                    outputBuffer.WriteByte((byte)',');
                i++;
                var buf = Encoding.UTF8.GetBytes(s);
                outputBuffer.Write(buf, 0, buf.Length);
            });
            outputBuffer.WriteByte((byte)']');
            outputBuffer.WriteByte((byte)'\n');

            // get as string
            string json = Encoding.UTF8.GetString(outputBuffer.ToArray());
            try
            {
                // pretty-print if necessary
                if (!outputCompactJson)
                {
                    json = JToken.Parse(json).ToString(Formatting.Indented);
                }
            }
            catch (Exception)
            {
                // ignore invalid JSON contents here
            }

            // Save result
            DecodedJson = json;

            // Return list of individual records
            return jsonList;
            // return json.Split(new string[] { Environment.NewLine }, StringSplitOptions.None).ToList();
        }

        /// <summary>
        /// Helper routine to gunzip compressed data.
        /// </summary>
        /// <param name="data">The compressed data</param>
        /// <returns>The gunzip'd data</returns>
        private static byte[] Gunzip(byte[] data)
        {
            var inStream = new MemoryStream(data);
            var gzipStream = new GZipStream(inStream, CompressionMode.Decompress);
            var outStream = new MemoryStream();

            gzipStream.CopyTo(outStream);
            return outStream.ToArray();
        }

        /// <summary>
        /// Helper routine to deflate compressed data.
        /// </summary>
        /// <param name="data">The compressed data</param>
        /// <returns>The delated data</returns>
        private static byte[] Deflate(byte[] data)
        {
            var inStream = new MemoryStream(data);
            var deflateStream = new DeflateStream(inStream, CompressionMode.Decompress);
            var outStream = new MemoryStream();

            deflateStream.CopyTo(outStream);
            return outStream.ToArray();
        }

    }

}