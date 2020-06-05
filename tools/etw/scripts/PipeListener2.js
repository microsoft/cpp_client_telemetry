#!/usr/bin/env node
// (c) 2020 Microsoft Corporation. All rights reserved.
//
// Pipe Listener that allows to forward events to AppInsights / Azure Monitor.
//
// Author: Max Golovanov <maxgolov@microsoft.com>
//
'use strict';

// Use environment variable APPINSIGHTS_INSTRUMENTATIONKEY to enable AppInsights export
let use_ai_logging = false;
let client = null;

// AppInsights initialiation section
if (typeof process.env['APPINSIGHTS_INSTRUMENTATIONKEY'] != "undefined") {
  const appInsights = require("applicationinsights");
  use_ai_logging = true;
  appInsights.setup();
  appInsights.start();
  client = appInsights.defaultClient;
}

// Common libraries
const net = require('net');
const path = require('path');
const colorize = require('json-colorizer');
const L = console.log;

// Pipe listener "Agent"
let server;

function isString(x) {
  return Object.prototype.toString.call(x) === "[object String]"
}

/**
 * This function adjust Object to be more closely aligned with Common Schema 4.x
 * @param {*} obj 
 */
function unflatten(obj)
{
  for(let key in obj)
  {
    let val = obj[key];
    if (key.startsWith("PartA_"))
    {
      delete obj[key];
      key = key.replace("PartA_", "");
      key = key.replace(/_/g, ".");
      let items = key.split(".");
      let item = obj;
      let i = 0;
      items.forEach( elem =>
        {
          if (i == items.length-1)
          {
            item[elem] = val;
          } else
          if (typeof item[elem] === 'undefined')
          {
            item[elem] = { };
          }
          item = item[elem];
          i++;
        }
      );
    }
  }
  return obj; 
};


const sendAIEvent = function(obj)
{
  let evt = unflatten(obj);
  let name = evt.name;
  delete evt.name;

  let ai_event = { name: name };
  if (typeof evt.timestamp === 'string')
  {
    ai_event.timestamp = evt.timestamp;
  }

  let ext = evt.ext;
  if (typeof ext === 'object')
  {
    ai_event.ext = ext;
    delete evt.ext;
  }
  ai_event.properties = evt;
  console.log(colorize(ai_event, { pretty: true }));
  client.trackEvent(ai_event);
};

const sendAIEvents = function (json) {
  let obj = json;
  if (isString(json)) {
    obj = JSON.parse(json);
  }
  if (obj instanceof Array) {
    // Array of events
    obj.forEach(evt => {
      sendAIEvent(evt);
    });
  } else {
    // One event
    sendAIEvent(obj);
  }
};

const createServer = function () {

  return net.createServer(function (stream) {

    L('Server: client connected');

    stream.on('end', function () {
      // L('Stream: on end');
    });

    stream.on('data', function (c) {
      // L('Stream: on data ', stream);
      // Split be end-of-line
      var items = c.toString().split('\n');
      L(c.toString());
      // Transform string stream into array of event objects
      var objs = [];
      for (var i = 0; i < items.length; i++) {
        // L("Trying to parse: ", items[i]);
        var item = items[i].trim();
        if (item.length) {
          try {
            objs.push(JSON.parse(item));
          } catch (ex) {
            // FIXME: invalid event contents
          }
        }
      }
      // Send event(s) to AI
      sendAIEvents(objs);
    });

    stream.on('error', function (ex) {
      L('Stream: on error', ex);
    });

    stream.on('close', function (ex) {
      // L('Stream: on close', ex);
    });

    stream.on('drain', function (ex) {
      L('Stream: on drain', ex);
    });
  }).on('close', function (ex) {
    L('Server: on close', ex);
    startServer();
  }).on('error', function (ex) {
    L('Server: on error', ex);
    startServer();
  });

};

const startServer = function () {
  L('Server (re)starting...');

  // create server
  server = createServer();

  // start listening  
  server.listen(path.join('\\\\.\\pipe', 'ETW-6d084bbf-6a96-44ef-83F4-0a77c9e34580'), function () {
    L('Server: listening');
  });

  L('Server started.');
};

startServer();
