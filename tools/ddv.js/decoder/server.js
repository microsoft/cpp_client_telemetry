// (c) 2020 Microsoft Corporation. All rights reserved.
//
// Standalone implementation of OneCollector in node.js
// This implementation requires Common Schema/Bond decoder.
// Decoding function 'decodeRequest' is implement in main.js
//
// Author: Max Golovanov <maxgolov@microsoft.com>
//
'use strict';

const fs = require('fs');
const http = require('http');
const url = require('url');

// TODO: figure out how to handle this better
let decodeRequest = global.decodeRequest;

const console_log_time = (arg) => {
  console.log(new Date(), arg);
}

/**
 * Create HTTP server object
 */
const httpServerListener = (request, response) => {

  const q = url.parse(request.url, true).query;
  let pathname = url.parse(request.url).pathname;
  console_log_time(request.method + " " + pathname);
  switch (pathname) {
    // TODO: allow alternate URLs
    case "/OneCollector/1.0/":
      // TODO: verify that the content-type is matching what we can decode
      let content_length = request.headers['content-length']
      console_log_time("Content-Length: " + content_length);

      let content_type = request.headers['content-type'];
      let content_encoding = request.headers['content-encoding'];
      if (typeof request.body === 'undefined') {
        request.body = [];
      };
      request.on('error', (err) => {
        console_log_time("error: " + err);
      }).
        on('data', (chunk) => {
          console_log_time("body parts: " + request.body.length);
          console_log_time("+new chunk: " + chunk.length + " bytes");
          request.body.push(chunk);
        }).
        on('end', () => {
          // This may synchronously call onRequestDecoded
          let buffer = Buffer.concat(request.body);
          console_log_time("Body size: " + buffer.length);
          // console.log(buffer.toString('base64'));
          decodeRequest(content_type, content_encoding, buffer);
          response.writeHead(200, { "Content-Type": "text/plain" });
          response.write("OK");
          response.end();
        });
      break;
    default:
      break;
  }
};

module.exports = {

  startServer: function () {
    const httpServer = http.createServer(httpServerListener);
    httpServer.on('checkContinue', function (req, res) {
      console_log_time('Expect: 100-continue');
      res.writeContinue();
      setTimeout(function () { httpServerListener(req, res); }, 100);
    });

    /**
     * Start listening to HTTP connections on port 8081
     */
    httpServer.listen(8081, function () {
      console_log_time('http server is listening on port 8081');
    });
  },

  getRequestListener: httpServerListener
};
