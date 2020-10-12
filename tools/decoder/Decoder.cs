//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

// System
using System;
using System.Text;
using System.IO;
using System.IO.Compression;
using System.Collections.Generic;

// Protocol
using Bond;
using Bond.Protocols;
using Bond.IO.Safe;

#if NETCOREAPP
// Extensions
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
#endif

// JSON parser
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using CsProtocol;
using System.Linq;

namespace CommonSchema
{
    public class Decoder
    {

#if NETCOREAPP
        // Logger passed down from the caller
        public ILogger Logger { get; set; }
#endif

        // Raw HTTP post body in input format
        private byte[] SavedBody;

        // Decoded output in JSON format
        private string DecodedJson;

        /// <summary>
        /// Initialize Decoder input parameters: request headers and request body. 
        /// </summary>
        /// <param name="requestHeaders">Request headers</param>
        /// <param name="requestBody">Request body</param>
        public Decoder(IDictionary<string, string> requestHeaders, byte[] requestBody)
        {
            if (requestHeaders is null)
            {
                throw new ArgumentNullException(nameof(requestHeaders));
            }
            ClientId = requestHeaders["Client-Id"];
            ContentType = requestHeaders["Content-Type"];
            ContentEncoding = requestHeaders.ContainsKey("Content-Encoding") ? requestHeaders["Content-Encoding"] : "";
            RequestBody = requestBody ?? throw new ArgumentNullException(nameof(requestBody));
        }

        /// <summary>
        /// Gets or sets the request body.
        /// </summary>
        public byte[] RequestBody
        {
            get => Encoding.UTF8.GetBytes(DecodedJson);
            set => SavedBody = value;
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

        /// <summary>
        /// Indent JSON using tab size provided.
        /// </summary>
        /// <param name="json">JSON string</param>
        /// <param name="TabSize">Tab size. Default: 2</param>
        /// <returns></returns>
        private static string IndentJson(string json, int TabSize = 2)
        {
            StringReader stringReader = new StringReader(json);
            StringWriter stringWriter = new StringWriter();
            JsonTextReader jsonReader = new JsonTextReader(stringReader);
            using (JsonTextWriter jsonWriter = new JsonTextWriter(stringWriter) { Formatting = Formatting.Indented, IndentChar = ' ', Indentation = TabSize })
            {
                jsonWriter.WriteToken(jsonReader);
            }
            return stringWriter.ToString();
        }

        private static readonly Dictionary<string, string> nodeMap = new Dictionary<string, string>
            {
                { "extIngest", "injest" },
                { "extProtocol", "protocol" },
                { "extUser", "user" },
                { "extDevice", "device" },
                { "extOs", "os" },
                { "extApp", "app" },
                { "extUtc", "utc" },
                { "extXbl", "xbl" },
                { "extJavascript", "js" },
                { "extReceipts", "receipts" },
                { "extNet", "net" },
                { "extSdk", "sdk" },
                { "extLoc", "loc" },
                { "extCloud", "cloud" },
                { "extService", "service" },
                { "extCs", "cs" },
                { "extM365a", "M365a" },
                { "extMscv", "mscv" },
                { "extIntWeb", "intWeb" },
                { "extIntService", "intService" },
                { "extWeb", "web" }
            };

        /// <summary>
        /// Convert Bond-style Value object to more readable JSON notation
        /// </summary>
        /// <param name="jObj"></param>
        /// <returns></returns>
        JToken ToJsonValue(JToken token)
        {
            JObject jObj = (token.Type == JTokenType.Object) ? (JObject)token: new JObject();

            // Ref. CSProtocol_types.cs
            if (jObj.ContainsKey("type"))
            {
                ValueKind kind = (CsProtocol.ValueKind)(int)jObj["type"];
                switch (kind)
                {
                    case ValueKind.ValueInt64:
                        return jObj["longValue"];
                    case ValueKind.ValueDouble:
                        return jObj["doubleValue"];
                    case ValueKind.ValueGuid:
                        return jObj["guidValue"][0];
                    case ValueKind.ValueBool:
                        return new JValue(jObj.Count != 1);
                    default:
                        break;
                }
            }

            if (jObj.ContainsKey("stringValue") && (jObj.Count == 1))
            {
                return jObj["stringValue"];
            }

            // TODO: add nicer formatting for non-standard complex types:
            // - stringArray
            // - longArray
            // - doubleArray
            // - guidArray
            // Currently these are still expressed based on JSON generated from Bond schema.

            return jObj;
        }

        /// <summary>
        /// Pull Common Schema extensions to 2nd-level JSON extension under 'ext'
        ///    e.g.
        /// from: "extNet" : [{"cost":x, "type":y}]
        /// to:    "ext":{"net":{"cost":x,"type":y}}
        /// 
        /// </summary>
        /// <param name="json"></param>
        /// <returns></returns>
        private string ToCompactExt(string json)
        {
            JObject jObj = JObject.Parse(json);
            if (!jObj.ContainsKey("ext"))
            {
                jObj.Add("ext", new JObject());
            }
            JObject ext = (JObject)jObj["ext"];
            foreach(KeyValuePair<string, string> entry in nodeMap)
            {
                if (jObj.TryGetValue(entry.Key, StringComparison.InvariantCultureIgnoreCase, out JToken parent))
                {
                    try
                    {
                        if (parent.Type == JTokenType.Array)
                        {
                             JArray extArr = (JArray)parent;
                            if (extArr.Count>0)
                            {
                                JObject child = (JObject)(extArr.First);
                                if (child.Count > 0)
                                {
                                    child = (JObject)(child.DeepClone());
                                    if (!ext.ContainsKey(entry.Value))
                                    {
                                        ext.Add(entry.Value, child);
                                    }
                                }
                            }
                            jObj.Remove(entry.Key);
                        }
                    }
                    catch (Exception ex)
                    {
#if NETCOREAPP
                        Logger.LogCritical(
                            "Exception: {ex}\nInvalid JSON tree node: {json}",
                            ex,
                            parent.ToString());
#else   
                        throw;
#endif
                    }
                }
            }

            // TODO:
            // - consider moving ext.loc.timezone to ext.loc.tz

            // Convert Bond-style event into JSON
            if (jObj.TryGetValue("data", out JToken oldData))
            {
                var props = oldData[0]["properties"].DeepClone();
                jObj.Remove("data");
                var newData = new JObject();
                for(int i=0; i<props.Count(); i+=2)
                {
                    newData.Add(
                        props.ElementAt(i).ToString(),
                        ToJsonValue(props.ElementAt(i+1).DeepClone())
                    );
                };
                jObj.Add("data", newData);
            };

            return jObj.ToString(Newtonsoft.Json.Formatting.None);
        }

        /// <summary>
        /// Indent decoded JSON using TabSize provided.
        /// </summary>
        /// <param name="TabSize"></param>
        /// <returns></returns>
        public string IndentJson(int TabSize = 2)
        {
            DecodedJson = IndentJson(DecodedJson, TabSize);
            return DecodedJson;
        }

        /// <summary>
        /// Transform input HTTP request body to JSON string.
        /// </summary>
        /// <param name="outputCompactJson">Produce compact JSON. Default: true</param>
        /// <param name="TabSize">If TabSize is specified, then JSON is pretty-formatted using TabSize</param>
        /// <returns></returns>
        public string ToJson(bool outputCompactJson = true, bool flatExt = true, int TabSize = -1)
        {
            FromBondToJSONList(outputCompactJson, flatExt);
            if (TabSize != -1)
            {
                IndentJson(TabSize);
            }
            return DecodedJson;
        }

        /// <summary>
        /// Deserialize the byte data into a DataPackage, then to JSON format,
        /// returning list of JSON records. This method also updates the
        /// DecodedJson property with JSON containing an array of all records.
        /// </summary>
        /// <param name="outputCompactJson">Whether the output JSON should be
        /// compact or human-readble (tabulated).</param>
        /// <param name="flatExt">Whether to move ext${X} records under ext
        /// for smaller more concise representation similar to JSON-CS protocol.
        /// </param>
        /// <returns>The JSON representation of the data bytes</returns>
        public List<string> FromBondToJSONList(bool outputCompactJson = true, bool flatExt = true)
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
                    data = Inflate(data);
                }
            }

            // read using Common Schema protocol
            using (MemoryStream outputBuffer = new MemoryStream())
            {
                try
                {
                    InputBuffer input = new InputBuffer(data);
                    CompactBinaryReader<InputBuffer> reader = new CompactBinaryReader<InputBuffer>(input);
                    SimpleJsonWriter writer = new SimpleJsonWriter(outputBuffer);
                    do
                    {
                        Record evt = Deserialize<Record>.From(reader);
                        // exception is thrown here if request is invalid
                        Serialize.To(writer, evt);
                        writer.Flush();
                        string j = Encoding.UTF8.GetString(outputBuffer.ToArray());
                        j = ToCompactExt(j);
                        jsonList.Add(j);
                        outputBuffer.SetLength(0);
                    } while (true);
                }
                catch (Exception ex)
                {
                    // end of input
#if NETCOREAPP
                    Logger.LogCritical( "Exception: {ex}", ex);                
#endif
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
            }

            // Return list of individual records
            return jsonList;
        }

        /// <summary>
        /// Helper routine to gunzip compressed data.
        /// </summary>
        /// <param name="data">The compressed data</param>
        /// <returns>The gunzip'd data</returns>
        public static byte[] Gunzip(byte[] data)
        {
            if (data is null)
            {
                throw new ArgumentNullException(nameof(data));
            }

            MemoryStream inStream = new MemoryStream(data);
            GZipStream gZipStream = new GZipStream(inStream, CompressionMode.Decompress);
            MemoryStream memoryStream = new MemoryStream();
            gZipStream.CopyTo(memoryStream);
            gZipStream.Dispose();
            return memoryStream.ToArray();
        }

        /// <summary>
        /// Helper routine to deflate compressed data.
        /// </summary>
        /// <param name="data">The compressed data</param>
        /// <returns>The delated data</returns>
        public static byte[] Inflate(byte[] data)
        {
            if (data is null)
            {
                throw new ArgumentNullException(nameof(data));
            }

            MemoryStream inStream = new MemoryStream(data);
            DeflateStream deflateStream = new DeflateStream(inStream, CompressionMode.Decompress);
            MemoryStream outStream = new MemoryStream();
            deflateStream.CopyTo(outStream);
            deflateStream.Dispose();
            return outStream.ToArray();
        }

    }

}
