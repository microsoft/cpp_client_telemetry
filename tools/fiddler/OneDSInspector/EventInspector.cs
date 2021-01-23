//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

[assembly: Fiddler.RequiredVersion("2.3.0.0")]

namespace OneDSInspector
{
    using Fiddler;
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Text;
    using System.Windows.Forms;
    using Decoder = CommonSchema.Decoder;

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
                headersCopy = value.ToDictionary(key => key.Name, key => key.Value);

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
        private Dictionary<string, string> headersCopy;

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

        public void UpdatePayloadView()
        {
            byte[] body = this.SavedBody;
            List<string> jsonList;
            bool outputCompactJson = this.PayloadViewerControl.IsCompactJsonOutputRequested();

            try
            {
                // var items = headers.ToArray();
                Decoder decoder = new Decoder(headersCopy, body);
                string json = decoder.ToJson(outputCompactJson, true, (outputCompactJson) ? -1 : 2);
                jsonList = json.Split(new string[] { Environment.NewLine }, StringSplitOptions.None).ToList();
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