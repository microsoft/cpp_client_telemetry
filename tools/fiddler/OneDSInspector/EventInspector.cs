// --------------------------------------------------------------------------------------------------------------------
// <copyright file="EventInspector.cs" company="Microsoft">
//   2016 Microsoft Corporation
// </copyright>
// <summary>
//   Fiddler inspector extension to view Aria events.
// </summary>
// --------------------------------------------------------------------------------------------------------------------

using Bond.IO.Unsafe;

using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

[assembly: Fiddler.RequiredVersion("2.3.0.0")]

namespace OneDSInspector
{
    using System;
    using System.Linq;
    using System.Text;
    using System.IO;
    using System.IO.Compression;
    using System.Security.Cryptography;
    using System.Windows.Forms;
    using System.Collections.Generic;

    using Fiddler;
    using CommonSchema;

    using Bond;
    using Bond.Protocols;
    using Bond.IO.Safe;
    using Fiddler.WebFormats;

    public class EventInspector : Inspector2, IRequestInspector2
    {
        // Inspector2 Overrides/Implementation

        /// <summary>
        /// Gets the order for this inspector.
        /// </summary>
        /// <returns>The inspector order</returns>
        public override int GetOrder()
        {
            return 0;
        }

        /// <summary>
        /// Adds our control to the specified tab.
        /// </summary>
        /// <param name="page">The tab page to add to</param>
        public override void AddToTab(TabPage page)
        {
            page.Text = "1DS Events";
            page.Controls.Add(this.PayloadViewerControl);
            page.Controls[0].Dock = DockStyle.Fill;
        }

        //// IRequestInspector2 Implementation

        /// <summary>
        /// Gets or sets the request headers (null to disallow editing).
        /// </summary>
        public HTTPRequestHeaders headers
        {
            get
            {
                return null;
            }

            set
            {
                if (!CheckIfPathHasDetails(value))
                {
                    // Inspect the header for ClientId to see whether it uses Auth or No-Auth.
                    // This is needed for body decoding.
                    this.ClientId = GetHeaderValue(value, "Client-Id");

                    // Inspect the content type to determine whether it is compact binary v1 or v2
                    this.ContentType = GetHeaderValue(value, "Content-Type");

                    // Inspect the content encoding type to determine whether gunzip is needed
                    this.ContentEncoding = GetHeaderValue(value, "Content-Encoding");
                }
            }
        }

        /// <summary>
        /// Clear the inspector contents.
        /// </summary>
        public void Clear()
        {
            this.DecodedJson = string.Empty;
            this.PayloadViewerControl.SetText(new List<string>());
        }

        /// <summary>
        /// Gets a value indicating whether the inspector is dirty (edited);
        /// </summary>
        public bool bDirty
        {
            get
            {
                return false;
            }
        }

        /// <summary>
        /// Gets or sets whether the inspector is read-only.
        /// </summary>
        public bool bReadOnly
        {
            get
            {
                return true;
            }

            set
            {
            }
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
                UpdatePayloadView();
            }
        }

        // Private Implementation

        /// <summary>
        /// Gets or sets the text control where the content is visible.
        /// </summary>
        private PayloadViewer PayloadViewerControl { get; set; }

        /// <summary>
        /// Gets or sets the inspected JSON content.
        /// </summary>
        private string DecodedJson { get; set; }

        /// <summary>
        /// Gets or sets the "Client-Id" of the inspected content.
        /// </summary>
        private string ClientId { get; set; }

        /// <summary>
        /// Gets or sets the "Content-Type" of the inspected content.
        /// </summary>
        private string ContentType { get; set; }

        /// <summary>
        /// Gets or sets the "Content-Encoding" of the inspected content.
        /// </summary>
        private string ContentEncoding { get; set; }

        /// <summary>
        /// Last body array value we received.
        /// This is used if we need to re-parse it due to changed viewing prefrences.
        /// </summary>
        private byte[] SavedBody { get; set; }

        /// <summary>
        /// Creates a new RecordInspector.
        /// </summary>
        public EventInspector()
        {
            this.PayloadViewerControl = new PayloadViewer();
            this.DecodedJson = string.Empty;
            this.PayloadViewerControl.ViewerConfigurationChanged += (obj, eventtArgs) => { UpdatePayloadView(); };
        }

        /// <summary>
        /// Checks the path to see if the header details are specified on the path. If the details exist, then
        /// it will populate the class properties with the values.
        /// </summary>
        /// <param name="headers">The HTTP request headers</param>
        /// <returns>True, if header details are on the path, false otherwise.</returns>
        private bool CheckIfPathHasDetails(HTTPRequestHeaders headers)
        {
            //determine if it is indeed send by the SDK
            if (headers.RequestPath.Contains("qsp=true"))
            {
                //Find where the header details will start
                var headerDetailsStart = headers.RequestPath.IndexOf("&");
                if (headerDetailsStart > 0 && headerDetailsStart < headers.RequestPath.Length)
                {
                    String[] headersDetails = headers.RequestPath.Substring(headerDetailsStart + 1).Split('&');
                    foreach (var header in headersDetails)
                    {
                        String[] headerKeyAndValue = header.Split('=');
                        if (headerKeyAndValue.Length == 2)
                        {
                            if (headerKeyAndValue[0].Equals("client-id", StringComparison.OrdinalIgnoreCase))
                            {
                                this.ClientId = headerKeyAndValue[1];
                            }
                            else if (headerKeyAndValue[0].Equals("content-type", StringComparison.OrdinalIgnoreCase))
                            {
                                this.ContentType = Uri.UnescapeDataString(headerKeyAndValue[1]);
                            }
                        }
                    }
                }
                return true;
            }
            return false;
        }

#if false
        private string IndentJson(string json)
        {
            using (var stringReader = new StringReader(json))
            using (var stringWriter = new StringWriter())
            {
                var jsonReader = new JsonTextReader(stringReader);
                var jsonWriter = new JsonTextWriter(stringWriter) { Formatting = Formatting.Indented, IndentChar = ' ', Indentation = 2 };
                jsonWriter.WriteToken(jsonReader);
                return stringWriter.ToString();
            }
        }
#endif

        /// <summary>
        /// Deserialize the byte data into a DataPackage, then to JSON format.
        /// </summary>
        /// <param name="data">The data bytes</param>
        /// <param name="outputCompactJson">Whether the output JSON should be compact or expanded/human-readble.</param>
        /// <returns>The JSON representation of the data bytes</returns>
        private List<string> ConvertPayloadToJson(byte[] data, bool outputCompactJson)
        {
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
            catch (EndOfStreamException)
            {
                // That's OK, we no longer have anything to decode.
            }
            catch (Exception ex)
            {
                // Decoding failed: show the message in decoder
                jsonList.Add(ex.Message);
                jsonList.Add(ex.StackTrace);
                return jsonList;
            }

            // consider sending jsonList one-by-one over UDP to remote monitor

            // flatten all into one buffer
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
            catch (Exception ex)
            {
			    // Failed to parse JSON for indented view:
				// Well, something went wrong...
				// Uncheck the pretty-printing checkbox.
                json = ex.Message;
                json += ex.StackTrace;
            }
            return json.Split(new string[] { Environment.NewLine }, StringSplitOptions.None).ToList();
        }

        /// <summary>
        /// Lookup the value of the specified header.
        /// </summary>
        /// <param name="headers">The HTTP request headers</param>
        /// <param name="headerName">The name of the header</param>
        /// <returns>The header value, or an empty string if it is not found</returns>
        private static string GetHeaderValue(HTTPRequestHeaders headers, string headerName)
        {
            var header = headers.FirstOrDefault(h => h.Name.Equals(headerName, StringComparison.InvariantCultureIgnoreCase));
            return header != null ? header.Value : string.Empty;
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

        public void UpdatePayloadView()
        {
            byte[] body = this.SavedBody;
            List<string> jsonList;
            bool outputCompactJson = this.PayloadViewerControl.IsCompactJsonOutputRequested();

            try
            {
                jsonList = ConvertPayloadToJson(body, outputCompactJson);
            }
            catch (Exception exception)
            {
                // if an exception occurs, display it in the inspector body
                jsonList = new List<string>();
                jsonList.Add(exception.Message);
            }

            this.PayloadViewerControl.SetText(jsonList);
        }
    }
}