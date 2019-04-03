#!/usr/bin/env python
# Reflects the requests from HTTP methods GET, POST, PUT, and DELETE
# Written by Nathan Hamiel (2010)

import os
from BaseHTTPServer import HTTPServer, BaseHTTPRequestHandler
from optparse import OptionParser

class RequestHandler(BaseHTTPRequestHandler):
    
    def do_GET(self):
        
        request_path = self.path
        
        print("\n----- Request Start ----->\n")
        print(request_path)
        print(self.headers)
        print("<----- Request End -----\n")
        
        self.send_response(200)
        self.send_header("Set-Cookie", "foo=bar")
        
    def do_POST(self):
        
        request_path = self.path
        
        print("\n----- Request Start ----->\n")
        print(request_path)
        
        request_headers = self.headers
        content_length = request_headers.getheaders('Content-Length')
        length = int(content_length[0]) if content_length else 0
        post_body = self.rfile.read(length)
        
        print(request_headers)
        print(post_body)
        print("<----- Request End -----\n")

        self.send_response(200)
        self.send_header("Content-type", "text/plain")
	self.end_headers()
	self.wfile.write(post_body)
    
    do_PUT = do_POST
    do_DELETE = do_GET
        
def main():
    port = int(os.getenv('PORT'))
    print('Listening on localhost:%s' % port)
    server = HTTPServer(('', port), RequestHandler)
    server.serve_forever()

        
if __name__ == "__main__":
    parser = OptionParser()
    parser.usage = ("Creates an http-server that will echo out any GET or POST parameters\n"
                    "Run:\n\n"
                    "   reflect")
    (options, args) = parser.parse_args()
    
    main()
