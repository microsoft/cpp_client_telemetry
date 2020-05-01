var WebSocketServer = require('websocket').server;
var WebSocketClient = require('websocket').client;
var WebSocketFrame = require('websocket').frame;
var WebSocketRouter = require('websocket').router;
var W3CWebSocket = require('websocket').w3cwebsocket;

var HashMap = require('hashmap');
var http = require('http');
var fs = require('fs');
var url = require('url')

var tmp = require('tmp');

function console_log(arg) {
    console.log((new Date()) + " " + arg)
}

// Create temporary file for counter data
var tmpFile = tmp.fileSync();
console_log(`Temp file: ${tmpFile.name}`);
console_log(`Temp fd:   ${tmpFile.fd}`);

// Create CSV file header
fs.write(tmpFile.fd, "time,key,value\n", (err) => {
    if (err) throw err;
});

const dgram = require('dgram');
const udpServer = dgram.createSocket('udp4');

// map that holds last elements in a series not sent to browser yet
var map = new HashMap();
var connections = [];

udpServer.on('error', (err) => {
    console_log(`server error:\n${err.stack}`);
    udpServer.close();
});

udpServer.on('message', (msg, rinfo) => {
    var obj = JSON.parse(msg);
    var time = new Date().getTime();
    map.set(time, obj);
    console_log(`udp server got: ${msg} from ${rinfo.address}:${rinfo.port}`);

    // Store (expand) JSON attributes to CSV file in a format time,key,value
    for (var key in obj) {
        var timeStr = time.toString()
        var attrName = key;
        var attrValue = obj[key];
        var s = `${timeStr},${attrName},${attrValue}\n`;
        fs.write(tmpFile.fd, s, (err) => {
            if (err) throw err;
        });
    }
});

udpServer.on('listening', () => {
    var address = udpServer.address();
    console_log(`udp server listening ${address.address}:${address.port}`);
});

udpServer.bind(8888);

/**
 * Create instance of HTTP server
 */
var httpServer = http.createServer(function (request, response) {

    var q = url.parse(request.url, true).query;
    var pathname = url.parse(request.url).pathname;

    console_log("GET " + pathname);

    var headers;

    // hardcoded list of special urls
    switch (pathname) {
        case "/export.csv":
            fs.readFile(tmpFile.name, function (err, data) {
                if (!err) {
                    headers = {
                        "Cache-Control": "must-revalidate",
                        "Pragma": "must-revalidate",
                        "Content-Type": "application/vnd.ms-excel",
                        "Content-Disposition": "attachment; filename=export.csv"
                    };
                    response.writeHead(200, headers);
                    response.write(data.toString());
                    response.end();
                    console_log("export.csv sent to client");
                } else {
                    console_log("Unable to send export.csv to client!");
                }
            });
            return;
        default:
    }

    if (pathname.substr(1) == "") {
        response.writeHead(302, { 'Location': '/index.html' });
        response.end();
        return;
    }

    var lastError = false;
    fs.readFile(pathname.substr(1), function (err, data) {
        if (!err) {
            headers = { 'Content-Type': 'text/html' };
            if (pathname.endsWith('.css')) {
                headers = { 'Content-Type': 'text/css' };
            } else
                if (pathname.endsWith('.js')) {
                    headers = { 'Content-Type': 'application/javascript' };
                } else
                    if (pathname.endsWith('.png')) {
                        headers = { 'Content-Type': 'image/png' };
                    }
            response.writeHead(200, headers);
            response.write(data.toString());
            // Send the response body 
            response.end();
            return;
        }
        lastError = err;
    });

    if (lastError) {
        console_log(lastError);
        response.writeHead(404, { 'Content-Type': 'text/html' });
        response.end();
    }

});

/**
 * Start listening to HTTP connections on port 8080
 */
httpServer.listen(8080, function () {
    console_log('http server is listening on port 8080');
});

/**
 * Create instance of WebSocket server
 */
wsServer = new WebSocketServer({
    httpServer: httpServer,
    // You should not use autoAcceptConnections for production 
    // applications, as it defeats all standard cross-origin protection 
    // facilities built into the protocol and the browser.  You should 
    // *always* verify the connection's origin and decide whether or not 
    // to accept it. 
    autoAcceptConnections: false
});

function originIsAllowed(origin) {
    // put logic here to detect whether the specified origin is allowed. 
    return true;
}

/**
 * WebSocket connections processing loop
 */
wsServer.on('request', function (request) {

    if (!originIsAllowed(request.origin)) {
        // Make sure we only accept requests from an allowed origin 
        request.reject();
        console_log('Connection from origin ' + request.origin + ' rejected.');
        return;
    }

    // New connection accepted    
    var connection = request.accept('ecg-protocol', request.origin);
    connections.push(connection);
    console_log('Connection accepted.');

    // Add connection handlers
    connection.on('message', function (message) {
        if (message.type === 'utf8') {
            console_log('Received Message: ' + message.utf8Data);
            // connection.sendUTF(message.utf8Data);
        }
        else if (message.type === 'binary') {
            console_log('Received Binary Message of ' + message.binaryData.length + ' bytes');
            // connection.sendBytes(message.binaryData);
        }
    });

    connection.on('close', function (reasonCode, description) {
        console_log(' Peer ' + connection.remoteAddress + ' disconnected.');
        // FIXME: find and remove connection from connections array 
    });

})

/**
 * Send data from map to all connected clients
 */
function sendDataToClients() {
    if (map.count() == 0)
        return; // nothing to send
    console_log("sending data to clients: " + map.count() + " samples");
    connections.forEach(function (conn) {
        map.forEach(function (value, key) {
            var message = { type: 'utf8', text: value, time: key };
            conn.sendUTF(JSON.stringify(message));
        });
    });
    console_log("clear samples map");
    map.clear();
}

// Send data to all connected clients every 500 milliseconds
setInterval(function () { sendDataToClients() }, 500);
